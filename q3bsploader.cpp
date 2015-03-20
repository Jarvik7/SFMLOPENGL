/*This is based on http://www.mralligator.com/q3/
https://github.com/rohitnirmal/q3-bsp-viewer/blob/master/bsp.h also served as reference for confirmation (no license? is also based on the above)
Quake 3 BSP loader
The concept is to import the BSP as-is into appropriate data structures, and
provide functions to output the data in a modular method usable to the j7Model class

A Doom3 MD5 loader should follow, based on same code layout.

Todos:
X) Get vertices & textures going at a level similar to current ASSIMP implementation
X) Break out prototypes into a proper header
X) Add functions to feed j7Model (to be done with #1?)
4) Break down giant constructor into individual functions
X) Our early exits are leaking the memblock array? Convert to a vector?
6) Add shader support to j7Model to enable lighting
7) Figure out collision detection
?) Replace memcpy with a c++ equivalent?
?) Add Doom3 BSP support
?) Add Quake1/2/etc BSP support
?) Open source and publish
X) Install MSVC2013 Express on work computer, switch to range based forloops and other C++11 stuff
*/

#include <iostream> // std::cout, std::cerr
#include <fstream> // std::ifstream
#include <vector> // std::vector
#include <unordered_map>
#include <array>

#include <GLEW/glew.h>
#include <SFML/OpenGL.hpp> // OpenGL datatypes
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

#include "q3bsploader.h"

extern GLuint loadTexture(std::string filename);

extern GLuint shaderID;

GLuint makeVAO(const std::vector<BSPVertex> *vertices, const std::vector<GLuint> *indices) {
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint bufferID;

	//Indices
	if (indices != nullptr) {
		glGenBuffers(1, &bufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(GLuint), indices->data(), GL_STATIC_DRAW);
	}

	//Vertex data
	glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(BSPVertex), vertices->data(), GL_STATIC_DRAW);

    //Position
    GLuint attribLoc = glGetAttribLocation(shaderID, "position");
    glEnableVertexAttribArray(attribLoc);
    glVertexAttribPointer(attribLoc, 3, GL_FLOAT, GL_FALSE, sizeof(BSPVertex), reinterpret_cast<const GLvoid*>(offsetof(BSPVertex, position)));

	//Texture coordinates
    attribLoc = glGetAttribLocation(shaderID, "texcoord");
    glEnableVertexAttribArray(attribLoc);
	glVertexAttribPointer(attribLoc, 2, GL_FLOAT, GL_FALSE, sizeof(BSPVertex), reinterpret_cast<const GLvoid*>(offsetof(BSPVertex, texcoord)));

    //Lightmap coordinates
    attribLoc = glGetAttribLocation(shaderID, "lmcoord");
    glEnableVertexAttribArray(attribLoc);
	glVertexAttribPointer(attribLoc, 2, GL_FLOAT, GL_FALSE, sizeof(BSPVertex), reinterpret_cast<const GLvoid*>(offsetof(BSPVertex, lmcoord)));

	//Normals
    attribLoc = glGetAttribLocation(shaderID, "normal");
    glEnableVertexAttribArray(attribLoc);
	glVertexAttribPointer(attribLoc, 3, GL_FLOAT, GL_FALSE, sizeof(BSPVertex), reinterpret_cast<const GLvoid*>(offsetof(BSPVertex, normal)));

	//Colors
    attribLoc = glGetAttribLocation(shaderID, "color");
    glEnableVertexAttribArray(attribLoc);
	glVertexAttribPointer(attribLoc, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BSPVertex), reinterpret_cast<const GLvoid*>(offsetof(BSPVertex, color)));

	//Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return vao;
}

q3BSP::q3BSP(const std::string filename) {
    std::cout << "Loading " << filename << '\n';
    sf::Clock timer;

    // Load file to memory
    std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) { std::cerr << "Couldn't open file.\n"; return; } // Couldn't open file
    const std::streamsize size = static_cast<std::streamsize>(file.tellg());
    file.seekg(0);
	std::vector<char> memblock;
	memblock.reserve(size);
    file.read(memblock.data(), size);
    file.close();

    // Read and check header
    BSPHeader header;
	std::copy(memblock.data(),
		memblock.data() + sizeof(BSPHeader),
		reinterpret_cast<char*>(&header));
	if (std::string(header.magicNumber, 4) != IDENT) { std::cerr << "Invalid format: " << std::string(header.magicNumber, 4) << '\n'; return; }
    if (header.version != IBSP_VERSION) {
        if (header.version == 47) std::cerr << "IBSP v.47: QuakeLive or RTCW map? Will try to load anyways.\n";
        else {
			std::cerr << "Invalid BSP version: " << header.version << '\n';
			return;
		}
    }
    else std::cout << "File format and version appear ok.\n";
    
    // Read lumps
    // Lump 0: Entities
	std::string tempEntityString(memblock.data() + header.direntries[Entities].offset, header.direntries[Entities].length);
    std::cout << "Lump 0: " << tempEntityString.size() << " characters of entities read.\n";
	parseEntities(&tempEntityString); // Parse entity string and populate vector of entities. Only spawnpoints, lights and music are read right now
    
    // Lump 1: Textures
    unsigned numEntries = header.direntries[Textures].length / sizeof(BSPTexture);
    std::cout << "Lump 1: " << numEntries << " texture(s) found.\n";
    textures.resize(numEntries);
	memcpy(textures.data(),
		memblock.data() + header.direntries[Textures].offset,
		header.direntries[Textures].length);
	// Load textures into memory and build vector of IDs. Note that at present this loads an empty texture for everything with a shader
	unsigned i = 0;
	for (auto& texture : textures) {
		std::cout << "  " << i << ':' << texture.name << '\n';
		++i;
		textureIDs.push_back(loadTexture(texture.name));
	}
    
    // Lump 2: Planes
	numEntries = header.direntries[Planes].length / sizeof(BSPPlane);
    std::cout << "Lump 2: " << numEntries << " plane(s) found.\n";
    planes.resize(numEntries);
	memcpy(planes.data(),
		memblock.data() + header.direntries[Planes].offset,
		header.direntries[Planes].length);
 
    // Lump 3: Nodes
    numEntries = header.direntries[Nodes].length / sizeof(BSPNode);
    std::cout << "Lump 3: " << numEntries << " node(s) found.\n";
    nodes.resize(numEntries);
    memcpy(nodes.data(),
		memblock.data() + header.direntries[Nodes].offset,
		header.direntries[Nodes].length);
  
    // Lump 4: Leafs
	numEntries = header.direntries[Leafs].length / sizeof(BSPLeaf);
	std::cout << "Lump 4: " << numEntries << " leaf(s) found.\n";
	leafs.resize(numEntries);
	memcpy(leafs.data(),
		memblock.data() + header.direntries[Leafs].offset,
		header.direntries[Leafs].length);

    // Lump 5: Leaffaces
	numEntries = header.direntries[Leaffaces].length / sizeof(int);
	std::cout << "Lump 5: " << numEntries << " leafface(s) found.\n";
	leafFaces.resize(numEntries);
	memcpy(leafFaces.data(),
		memblock.data() + header.direntries[Leaffaces].offset,
		header.direntries[Leaffaces].length);

    // Lump 6: Leafbrushes
    numEntries = header.direntries[Leafbrushes].length / sizeof(int);
	std::cout << "Lump 5: " << numEntries << " leafbrush(es) found.\n";
	leafBrushes.resize(numEntries);
	memcpy(leafBrushes.data(),
		memblock.data() + header.direntries[Leafbrushes].offset,
           header.direntries[Leafbrushes].length);
    
    // Lump 7: Models
    numEntries = header.direntries[Models].length / sizeof(BSPModel);
	std::cout << "Lump 5: " << numEntries << " model(s) found.\n";
	models.resize(numEntries);
	memcpy(models.data(),
		memblock.data() + header.direntries[Models].offset,
           header.direntries[Models].length);
    
    // Lump 8: Brushes
    numEntries = header.direntries[Brushes].length / sizeof(BSPBrush);
	std::cout << "Lump 5: " << numEntries << " brush(es) found.\n";
	brushes.resize(numEntries);
	memcpy(brushes.data(),
		memblock.data() + header.direntries[Brushes].offset,
           header.direntries[Brushes].length);
    
    // Lump 9: Brushsides
    numEntries = header.direntries[Brushsides].length / sizeof(BSPBrushSide);
	std::cout << "Lump 5: " << numEntries << " brushside(s) found.\n";
	brushSides.resize(numEntries);
	memcpy(brushSides.data(),
		memblock.data() + header.direntries[Brushsides].offset,
           header.direntries[Brushsides].length);
    
    // Lump 10: Vertexes
    numEntries = header.direntries[Vertexes].length / sizeof(BSPVertex);
    std::cout << "Lump 10: " << numEntries << " vertex(es) found.\n";
    vertices.resize(numEntries);
    memcpy(vertices.data(),
		memblock.data() + header.direntries[Vertexes].offset,
		header.direntries[Vertexes].length);
	
    // Lump 11: Meshverts
    numEntries = header.direntries[Meshverts].length / sizeof(int);
    std::cout << "Lump 11: " << numEntries << " Meshvert(s) found.\n";
    meshVerts.resize(numEntries);
	memcpy(meshVerts.data(),
		memblock.data() + header.direntries[Meshverts].offset,
		header.direntries[Meshverts].length);
    
    // Lump 12: Effects
	numEntries = header.direntries[Effects].length / sizeof(BSPEffect);
	std::cout << "Lump 12: " << numEntries << " effect(s) found.\n";
	effects.reserve(numEntries);
	memcpy(effects.data(),
		memblock.data() + header.direntries[Effects].offset,
		header.direntries[Effects].length);
    
    // Lump 13: Faces
    numEntries = header.direntries[Faces].length / sizeof(BSPFace);
    std::cout << "Lump 13: " << numEntries << " face(s) found.\n";
    faces.resize(numEntries);
    memcpy(faces.data(),
		memblock.data() + header.direntries[Faces].offset,
		header.direntries[Faces].length);

    // Lump 14: Lightmaps
	numEntries = header.direntries[Lightmaps].length / sizeof(BSPLightmap);
	std::cout << "Lump 14: " << numEntries << " lightmap(s) found.\n";
	lightmaps.resize(numEntries);
	std::copy(memblock.data() + header.direntries[Lightmaps].offset,
		      memblock.data() + header.direntries[Lightmaps].offset + header.direntries[Lightmaps].length,
			  reinterpret_cast<char*>(lightmaps.data()));
	bindLightmaps();

	// Lump 15: Lightvols
    
    // Lump 16: Visdata
	memcpy(&visData,
		memblock.data() + header.direntries[Visdata].offset,
		2 * sizeof(int));
	visData.vecs.resize(visData.n_vecs * visData.sz_vecs);
    memcpy(visData.vecs.data(),
		memblock.data() + header.direntries[Visdata].offset + 2 * sizeof(int),
           visData.n_vecs * visData.sz_vecs);
	std::cout << "Lump 16: " << visData.n_vecs << " vectors @ " << visData.sz_vecs << " bytes each = " << visData.vecs.size() << " bytes of visdata.\n";
	
    std::cout << "Finished importing bsp in " << timer.getElapsedTime().asSeconds() << " seconds\n";

	//parseShader("textures/skies/tim_hell");
}

void q3BSP::bindLightmaps() {
	//Loads all lightmaps into a texture array
	//All lightmaps are 128x128 RGB8 format
	//TODO:: Determine if adding anisotropic filtering is useful, and conversely, if we can get away with nearest neighbor filtering

	//Initialize data structures
	glGenTextures(1, &lightmapGLID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, lightmapGLID);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, LIGHTMAP_RESOLUTION, LIGHTMAP_RESOLUTION, static_cast<GLsizei>(lightmaps.size()), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	lightmapIndexUniformPosition = glGetUniformLocation(shaderID, "lightmapArrayOffset");

	//Load in the lightmap textures
	GLint offset = 0;
	for (auto& lightmap : lightmaps) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, offset, LIGHTMAP_RESOLUTION, LIGHTMAP_RESOLUTION, 1, GL_RGB, GL_UNSIGNED_BYTE, lightmap.data());
		++offset;
	}

	//Enable anisotropic filtering
	GLfloat largest_aniso;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_aniso);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_aniso);

	//Enable mipmapping & linear filtering
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	//Rebind to texture unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, lightmapGLID);
	
	lightmaps.clear(); // No need to retain
}

void q3BSP::parseEntities(const std::string *entitystring) {
	std::cout << "Parsing entities...\n";
	size_t open = entitystring->find('{', 0) + 1; // Set start position to location after first opening brace
	size_t close = 0;

	// Split into vector of each clause
	std::vector<std::string> clauses;
	while(open != std::string::npos) {
			close = entitystring->find('}', open); // Find the next closing brace
			clauses.push_back(entitystring->substr(open, close - open)); // Push the string to vector, outer braces
			open = entitystring->find('{', close + 1); // Find the next opening brace
	}
	std::cout << clauses.size() << " clauses found.\n";
	
	// Convert each clause into a BSPEntity object

	for (auto& clause : clauses) {
		BSPEntity tempEntity(clause);
		if (tempEntity.pair["classname"] == "info_player_deathmatch") cameraPositions.push_back(camPos(tempEntity));
		else if (tempEntity.pair["classname"] == "worldspawn") worldMusic = tempEntity.pair["music"];
		else if (tempEntity.pair["classname"] == "light") lightPositions.push_back(lightPos(tempEntity));
		else entities.push_back(tempEntity); // Not handled, so throw it in the vector
	}
	std::cout << "  Map music: " << worldMusic << '\n';
	std::cout << "  " << cameraPositions.size() << " spawn points found.\n";
	std::cout << "  " << lightPositions.size() << " lights found.\n";
}

//Tessellation functions
BSPPatch::BSPPatch(const q3BSP *bsp, const unsigned face) {
    std::vector<j7Bezier> bezier;

    //Setup the control information and tessellate
	const int patch_size_x = (bsp->faces[face].size[0] - 1) / 2;
	const int patch_size_y = (bsp->faces[face].size[1] - 1) / 2;
	bezier.resize(patch_size_x * patch_size_y);

	int patchIndex =  0;
	int ii, n, j, nn;
	for (ii = 0, n = 0; n < patch_size_x; ++n, ii = 2 * n) {
	    for (j = 0, nn = 0; nn < patch_size_y; ++nn, j = 2 * nn) {
			int index = 0;
			for (int ctr = 0; ctr < 3; ++ctr) {
				const int pos = ctr * bsp->faces[face].size[0];

				bezier[patchIndex].controls[index++] = bsp->vertices[bsp->faces[face].vertex + ii + bsp->faces[face].size[0] * j + pos];
				bezier[patchIndex].controls[index++] = bsp->vertices[bsp->faces[face].vertex + ii + bsp->faces[face].size[0] * j + pos + 1];
                bezier[patchIndex].controls[index++] = bsp->vertices[bsp->faces[face].vertex + ii + bsp->faces[face].size[0] * j + pos + 2];
			}
			bezier[patchIndex++].tessellate(TESSELLATION_LEVEL);
		}
	}
    
    // Collect all the vertices and indices
    for (auto& grid : bezier) {
        const GLuint offset = static_cast<GLuint>(vertices.size());
        for (auto& vertex : grid.vertex) {
            vertices.push_back(vertex);
        }
        int counter = 0;

        for (auto& index : grid.indices) {
            if ((counter++ % ((TESSELLATION_LEVEL + 1) * 2) == 0)) indices.push_back(0xFFFFFFFF);
            indices.push_back(index + offset);
        }
    }
    n_indices = static_cast<GLsizei>(indices.size());
}
	
void j7Bezier::tessellate(const int L) {
	// Based on info from http://graphics.cs.brown.edu/games/quake/quake3.html, with simplified code and better use of C++

    // The number of vertices along a side is 1 + num edges
    const int L1 = L + 1;

    vertex.resize(L1 * L1);

    // Compute the vertices
    for (int i = 0; i <= L; ++i) {
        const float a = static_cast<float>(i) / L;
        const float b = 1.0f - a;

        vertex[i] =
            controls[0] * (b * b) + 
            controls[3] * (2 * b * a) +
            controls[6] * (a * a);
    }

    for (int i = 1; i <= L; ++i) {
        const float a = static_cast<float>(i) / L;
        const float b = 1.0f - a;

        BSPVertex temp[3];

        for (int j = 0; j < 3; ++j) {
            const int k = 3 * j;
            temp[j] =
                controls[k + 0] * (b * b) + 
                controls[k + 1] * (2 * b * a) +
                controls[k + 2] * (a * a);
        }

        for (int j = 0; j <= L; ++j) {
            const float a = static_cast<float>(j) / L;
            const float b = 1.0f - a;

            vertex[i * L1 + j] =
                temp[0] * (b * b) + 
                temp[1] * (2 * b * a) +
                temp[2] * (a * a);
        }
    }

    // Compute the indices
    indices.resize(L * L1 * 2);

    for (int row = 0; row < L; ++row) {
        for (int col = 0; col <= L; ++col)	{
            indices[(row * (L + 1) + col) * 2 + 1] = row * L1 + col;
            indices[(row * (L + 1) + col) * 2] = (row + 1) * L1 + col;
        }
    }
    //Normalize the normals
	for (auto& vert : vertex) {
		vert.normal = glm::normalize(vert.normal);
	}
}

// PVS Culling functions
typedef struct {
	glm::fvec4 left, right, top, bottom, nearclip, farclip;
} frustum;

//TODO::Check if frustum stuff actually works, and if it offers a performance improvement over hardware culling
frustum getViewFrustum(const glm::mat4 matrix) {
	frustum view;

	view.left = glm::fvec4(
		matrix[3][0] + matrix[0][0],
		matrix[3][1] + matrix[0][1],
		matrix[3][2] + matrix[0][2],
		matrix[3][3] + matrix[0][3]);

	view.right = glm::fvec4(
		matrix[3][0] - matrix[0][0],
		matrix[3][1] - matrix[0][1],
		matrix[3][2] - matrix[0][2],
		matrix[3][3] - matrix[0][3]);

	view.top = glm::fvec4(
		matrix[3][0] - matrix[1][0],
		matrix[3][1] - matrix[1][1],
		matrix[3][2] - matrix[1][2],
		matrix[3][3] - matrix[1][3]);

	view.bottom = glm::fvec4(
		matrix[3][0] + matrix[1][0],
		matrix[3][1] + matrix[1][1],
		matrix[3][2] + matrix[1][2],
		matrix[3][3] + matrix[1][3]);

	view.nearclip = glm::fvec4(
		matrix[3][0] + matrix[2][0],
		matrix[3][1] + matrix[2][1],
		matrix[3][2] + matrix[2][2],
		matrix[3][3] + matrix[2][3]);

	view.farclip = glm::fvec4(
		matrix[3][0] - matrix[2][0],
		matrix[3][1] - matrix[2][1],
		matrix[3][2] - matrix[2][2],
		matrix[3][3] - matrix[2][3]);
	return view;
}
float distanceToPoint(const glm::fvec4 plane, const glm::fvec3 point) {
	return plane.x * point.x
		+ plane.y * point.y
		+ plane.z * point.z
		+ plane.w;
}
bool isInFrustum(const frustum view, const glm::fvec3 point) {
	if (distanceToPoint(view.left, point) < 0) return false;
	if (distanceToPoint(view.right, point) < 0) return false;
	if (distanceToPoint(view.top, point) < 0) return false;
	if (distanceToPoint(view.bottom, point) < 0) return false;
	if (distanceToPoint(view.nearclip, point) < 0) return false;
	if (distanceToPoint(view.farclip, point) < 0) return false;
	return true;
}

int q3BSP::findCurrentLeaf(const glm::vec3 position) const {
    int index = 0;
    while (index >= 0) {
        const BSPNode& node = nodes[index];
        const BSPPlane& plane = planes[node.plane];
        // Distance from point to a plane
        const double distance = glm::dot(position, plane.normal) - plane.distance;

        index = (distance >= 0) ? node.children[0] : node.children[1];
    }
    return -index - 1;
}

bool q3BSP::isClusterVisible(const int testCluster, const int visCluster) const {
	//Sanity check
    if ((visData.vecs.size() == 0) || (visCluster < 0)) return true; // Show all faces when outside map or there is no visdata

	if (visData.vecs[(testCluster >> 3) + (visCluster * visData.sz_vecs)] & (1 << (testCluster & 7))) return true;
	return false;
}

std::vector<int> q3BSP::makeListofVisibleFaces(const glm::vec3 position, const glm::mat4 viewmatrix) const {
    static std::vector<int> visibleFaces;

	//Check if we are in same leaf as last frame, early exit if so
	static int prevLeaf = -1;
	const int currentLeaf = findCurrentLeaf(position);
	if (currentLeaf == prevLeaf) return visibleFaces;
	prevLeaf = currentLeaf;

	std::vector<bool> alreadyVisible; //Keep track of already added faces
	alreadyVisible.resize(faces.size());

    visibleFaces.resize(0); // reset

	const frustum viewfrustum = getViewFrustum(viewmatrix);

    for (auto& leaf : leafs) {
		const glm::fvec3 min(leaf.mins[0], leaf.mins[1], leaf.mins[2]);
		const glm::fvec3 max(leaf.maxs[0], leaf.maxs[1], leaf.maxs[2]);
		if (isClusterVisible(leaf.cluster, leafs[currentLeaf].cluster) /*&& isInFrustum(viewfrustum, min) && isInFrustum(viewfrustum, max)*/) { // Frustum culling is culling some visible elements
			for (int j = leaf.leafface; j < leaf.leafface + leaf.n_leaffaces; ++j) { // Then push all its faces to vector
				if (!alreadyVisible[leafFaces[j]]) visibleFaces.push_back(leafFaces[j]);
				alreadyVisible[leafFaces[j]] = true; // Prevent faces from being added more than once
            }
        }
    }
	return visibleFaces;
}

std::string trimWhiteSpace(const std::string input) { // Deletes leading/trailing spaces, tabs, returns, newlines
	if (input.empty()) return "";
    const size_t start = input.find_first_not_of(" \t\r\n");
    const size_t end = input.find_last_not_of(" \t\r\n") + 1; 
	if (start > end) return ""; // Line with only whitespace
    return input.substr(start, end - start);
}

std::vector<std::string> tokenize(const std::string input, const std::string tokens) {
    std::vector<std::string> output;
    if (input == "") return output;
    size_t start = 0, end = 0;

    while (end != std::string::npos) {
        end = input.find_first_of(tokens, start);
        output.push_back(input.substr(start, end - start));
        start = end + 1;
    }

    return output;
}

typedef struct {
	std::string map;
	glm::fvec2 tcMod_scroll;
	glm::fvec2 tcMod_scale;
	bool depthWrite;
	std::string blendSrc, blendDst;
} shaderStage;

class Q3ShaderStage {
public:
	std::unordered_map<std::string, std::string> bleh;
};

class Q3shader {
public:
	std::vector<std::string> surfaceparms;
	std::vector<std::string> skyparms;
	std::vector<shaderStage> stages;
	std::unordered_map<std::string, std::string> keywords;
};
void q3BSP::parseShader( std::string shadername) {
	// This is just a test to get the sky rendering, it doesn't parse all shader files yet.
	std::cout << "Parsing shader...\n";
	const std::string filename = "scripts/all.shader"; // FIXME: Should scan the scripts directory and load each shader. Manually grouped them into one blob for now
	std::ifstream file(filename);
	if (!file.is_open()) return;
	static std::string shaderSource;
	shaderSource.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	size_t start = 0, end = 0;
    //Walk and parse
   // std::vector<std::string> shaderNames;
    int clauseDepth = 0;
	shaderStage tempStage;

	std::unordered_map<std::string, Q3shader> Q3Shaders; // Map the shader name to the actual shader

    while (end != std::string::npos) {
		std::string name;
        // Get next line
        end = shaderSource.find('\n', start);
        std::string line = trimWhiteSpace(shaderSource.substr(start, end - start));

		if ( (line.empty()) || (line.front() == '/') ); // Blank line or comment, do nothing
        else if (line == "{") ++clauseDepth; // Opening clause -> new stage
		else if (line == "}") {// Closing clause -> end stage or shader
			--clauseDepth;
			if (clauseDepth == 1) Q3Shaders[name].stages.push_back(tempStage); //Push stage into shader
		}
		else if (clauseDepth == 0) name = line; // Start a new shader

        else { // Shader operation
            std::vector<std::string> tokens = tokenize(line, " \t");
			if (clauseDepth == 1) { // Not in a stage
				if (tokens[0] == "surfaceparm") Q3Shaders[name].surfaceparms.push_back(tokens[1]);
				else if (tokens[0] == "skyparms") {
					Q3Shaders[name].skyparms.push_back(tokens[1]); //farbox
					Q3Shaders[name].skyparms.push_back(tokens[2]); //cloudheight
					Q3Shaders[name].skyparms.push_back(tokens[3]); //nearbox, always - (null)
				}
				else Q3Shaders[name].keywords[tokens[0]] = tokens[1]; // All other global keywords should be unique and only have a single value
			}
			else if (clauseDepth == 0) { // In a stage

			}
        }
        start = end + 1;
    }
   // std::cout << "Number of shaders found: " << shaderNames.size() << '\n';
}








