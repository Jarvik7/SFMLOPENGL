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
#include <map>
#include <array>

#include <GLEW/glew.h>
#include <SFML/OpenGL.hpp> // OpenGL datatypes
#include <glm/glm.hpp>

#include "q3bsploader.h"

extern GLuint loadTexture(std::string filename, int offset);

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
    GLint attribLoc = glGetAttribLocation(shaderID, "position");
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

    // Load file to memory
    std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) { std::cerr << "Couldn't open file.\n"; return; } // Couldn't open file
    const unsigned size = static_cast<unsigned>(file.tellg());
    file.seekg(0);
	std::vector<char> memblock;
	memblock.reserve(size);
    file.read(&memblock[0], size);
    file.close();

    // Read and check header
    BSPHeader header;
    memcpy(&header, &memblock[0], sizeof(BSPHeader));
	if (std::string(IDENT).compare(0,4,header.magicNumber,4)) { std::cerr << "Invalid format: \n" << header.magicNumber[0] << header.magicNumber[1] << header.magicNumber[2] << header.magicNumber[3] << '\n'; return; }
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
	std::string tempEntityString;
    tempEntityString.insert(0, &memblock[header.direntries[Entities].offset], header.direntries[Entities].length);
    std::cout << "Lump 0: " << tempEntityString.size() << " characters of entities read.\n";
    
    // Lump 1: Textures
    unsigned numEntries = header.direntries[Textures].length / sizeof(BSPTexture);
    std::cout << "Lump 1: " << numEntries << " texture(s) found.\n";
    textures.resize(numEntries);
	memcpy(textures.data(),
		&memblock[header.direntries[Textures].offset],
		header.direntries[Textures].length);
    for (unsigned i = 0; i < textures.size(); ++i) std::cout << "  " << i << ':' << textures[i].name << '\n';
    
    // Lump 2: Planes
	numEntries = header.direntries[Planes].length / sizeof(BSPPlane);
    std::cout << "Lump 2: " << numEntries << " plane(s) found.\n";
    planes.resize(numEntries);
	memcpy(planes.data(),
		&memblock[header.direntries[Planes].offset],
		header.direntries[Planes].length);
 
    // Lump 3: Nodes
    numEntries = header.direntries[Nodes].length / sizeof(BSPNode);
    std::cout << "Lump 3: " << numEntries << " node(s) found.\n";
    nodes.resize(numEntries);
    memcpy(nodes.data(),
		&memblock[header.direntries[Nodes].offset],
		header.direntries[Nodes].length);
  
    // Lump 4: Leafs
	numEntries = header.direntries[Leafs].length / sizeof(BSPLeaf);
	std::cout << "Lump 4: " << numEntries << " leaf(s) found.\n";
	leafs.resize(numEntries);
	memcpy(leafs.data(),
		&memblock[header.direntries[Leafs].offset],
		header.direntries[Leafs].length);

    // Lump 5: Leaffaces
	numEntries = header.direntries[Leaffaces].length / sizeof(int);
	std::cout << "Lump 5: " << numEntries << " leafface(s) found.\n";
	leafFaces.resize(numEntries);
	memcpy(leafFaces.data(),
		&memblock[header.direntries[Leaffaces].offset],
		header.direntries[Leaffaces].length);

    // Lump 6: Leafbrushes
    numEntries = header.direntries[Leafbrushes].length / sizeof(int);
	std::cout << "Lump 5: " << numEntries << " leafbrush(es) found.\n";
	leafBrushes.resize(numEntries);
	memcpy(leafBrushes.data(),
           &memblock[header.direntries[Leafbrushes].offset],
           header.direntries[Leafbrushes].length);
    
    // Lump 7: Models
    numEntries = header.direntries[Models].length / sizeof(BSPModel);
	std::cout << "Lump 5: " << numEntries << " model(s) found.\n";
	models.resize(numEntries);
	memcpy(models.data(),
           &memblock[header.direntries[Models].offset],
           header.direntries[Models].length);
    
    // Lump 8: Brushes
    numEntries = header.direntries[Brushes].length / sizeof(BSPBrush);
	std::cout << "Lump 5: " << numEntries << " brush(es) found.\n";
	brushes.resize(numEntries);
	memcpy(brushes.data(),
           &memblock[header.direntries[Brushes].offset],
           header.direntries[Brushes].length);
    
    // Lump 9: Brushsides
    numEntries = header.direntries[Brushsides].length / sizeof(BSPBrushSide);
	std::cout << "Lump 5: " << numEntries << " brushside(s) found.\n";
	brushSides.resize(numEntries);
	memcpy(brushSides.data(),
           &memblock[header.direntries[Brushsides].offset],
           header.direntries[Brushsides].length);
    
    // Lump 10: Vertexes
    numEntries = header.direntries[Vertexes].length / sizeof(BSPVertex);
    std::cout << "Lump 10: " << numEntries << " vertex(es) found.\n";
    vertices.resize(numEntries);
    memcpy(vertices.data(),
		&memblock[header.direntries[Vertexes].offset],
		header.direntries[Vertexes].length);
	
    // Lump 11: Meshverts
    numEntries = header.direntries[Meshverts].length / sizeof(int);
    std::cout << "Lump 11: " << numEntries << " Meshvert(s) found.\n";
    meshVerts.resize(numEntries);
	memcpy(meshVerts.data(),
		&memblock[header.direntries[Meshverts].offset],
		header.direntries[Meshverts].length);
    
    // Lump 12: Effects
	numEntries = header.direntries[Effects].length / sizeof(BSPEffect);
	std::cout << "Lump 12: " << numEntries << " effect(s) found.\n";
	effects.reserve(numEntries);
	memcpy(effects.data(),
		&memblock[header.direntries[Effects].offset],
		header.direntries[Effects].length);
    
    // Lump 13: Faces
    numEntries = header.direntries[Faces].length / sizeof(BSPFace);
    std::cout << "Lump 13: " << numEntries << " face(s) found.\n";
    faces.resize(numEntries);
    memcpy(faces.data(),
		&memblock[header.direntries[Faces].offset],
		header.direntries[Faces].length);

    // Lump 14: Lightmaps
	numEntries = header.direntries[Lightmaps].length / sizeof(BSPLightmap);
	std::cout << "Lump 14: " << numEntries << " lightmap(s) found.\n";
	lightmaps.resize(numEntries);
	memcpy(lightmaps.data(),
		&memblock[header.direntries[Lightmaps].offset],
		header.direntries[Lightmaps].length);
    
	// Lump 15: Lightvols
    
    // Lump 16: Visdata
	memcpy(&visData,
		&memblock[header.direntries[Visdata].offset],
		2 * sizeof(int));
	visData.vecs.resize(visData.n_vecs * visData.sz_vecs);
    memcpy(visData.vecs.data(),
           &memblock[header.direntries[Visdata].offset + 2 * sizeof(int)],
           visData.n_vecs * visData.sz_vecs);
	std::cout << "Lump 16: " << visData.n_vecs << " vectors @ " << visData.sz_vecs << " bytes each = " << visData.vecs.size() << " bytes of visdata.\n";


	// Begin working on data
	// Lump 0
	parseEntities(&tempEntityString); // Parse entity string and populate vector of entities. Only spawnpoints, lights and music are read right now
	
	// Load textures into memory and build vector of IDs. Note that at present this loads an empty texture for everything with a shader
	for (auto& texture : textures) textureIDs.push_back(loadTexture(texture.name, 0));
    // Load lightmaps into memory
	//bindTextures();
	bindLightmaps();

	parseShader("textures/skies/tim_hell");
}

void q3BSP::bindTextures() {
	//glActiveTexture(GL_TEXTURE0);
	//Initialize data structures
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, LIGHTMAP_RESOLUTION, LIGHTMAP_RESOLUTION, textures.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	textureArrayOffsetPos = glGetUniformLocation(shaderID, "textureArrayOffset");

	//Load in the textures
	int offset = 0;
	for (auto& texture : textures) {
		loadTexture(texture.name, offset);
		++offset;
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
}

void q3BSP::bindLightmaps() {
	//Loads all lightmaps into a texture array
	//All lightmaps are 128x128 RGB8 format
	//TODO:: Combine with normal textures into a single array? (only possible if they have same resolution as lightmaps)
	//TODO:: Determine if adding anisotropic filtering is useful, and conversely, if we can get away with nearest neighbor filtering

	//Initialize data structures
//	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &lmapID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, lmapID);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, LIGHTMAP_RESOLUTION, LIGHTMAP_RESOLUTION, lightmaps.size(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	lmapindexpos = glGetUniformLocation(shaderID, "lightmapArrayOffset");

	//Load in the lightmap textures
	int offset = 0;
	for (auto& lightmap : lightmaps) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, offset, 128, 128, 1, GL_RGB, GL_UNSIGNED_BYTE, lightmap.data());
		++offset;
	}

	//Set texture attributes
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Rebind to texture unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, lmapID); 
}


void q3BSP::parseEntities(const std::string *entitystring) {
	std::cout << "Parsing entities...\n";
	unsigned long open = entitystring->find_first_of('{', 0);
	unsigned long close = 0;

	// Split into vector of each clause
	std::vector<std::string> clauses;
	while(open != std::string::npos) {
			close = entitystring->find_first_of('}', open + 1); // Find closing brace starting at last opening brace
			clauses.push_back(entitystring->substr(open, close - open)); // Push, minus open & close braces & newlines
			open = entitystring->find_first_of('{', close + 1); // Set next start location to after closing brace
	}
	std::cout << clauses.size() << " clauses found.\n";
	
	// Convert each clause into a BSPEntity object

	for (auto& clause : clauses) {
		BSPEntity tempEntity(clause);
		// Parse entities ::TODO:: Push only unhandled entities to vector
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
BSPPatch::BSPPatch(const q3BSP *bsp, const int face) {
    std::vector<j7Bezier> bezier;

    //Setup the control information and tessellate
	int patch_size_x = (bsp->faces[face].size[0] - 1) / 2;
	int patch_size_y = (bsp->faces[face].size[1] - 1) / 2;
	bezier.resize(patch_size_x * patch_size_y);

	int patchIndex =  0;
	int ii, n, j, nn;
	for (ii = 0, n = 0; n < patch_size_x; n++, ii = 2*n) {
	    for (j=0, nn=0; nn < patch_size_y; nn++, j = 2*nn) {
			int index = 0;
			for (int ctr = 0; ctr < 3; ++ctr) {
				int pos = ctr * bsp->faces[face].size[0];

				bezier[patchIndex].controls[index++] = bsp->vertices[bsp->faces[face].vertex + ii + bsp->faces[face].size[0] * j + pos];
				bezier[patchIndex].controls[index++] = bsp->vertices[bsp->faces[face].vertex + ii + bsp->faces[face].size[0] * j + pos + 1];
                bezier[patchIndex].controls[index++] = bsp->vertices[bsp->faces[face].vertex + ii + bsp->faces[face].size[0] * j + pos + 2];
			}
			bezier[patchIndex++].tessellate(TESSELLATION_LEVEL);
		}
	}
    
    // Collect all the vertices and indices
    for (auto& grid : bezier) {
        GLuint offset = static_cast<GLuint>(vertices.size());
        for (auto& vertex : grid.vertex) {
            vertices.push_back(vertex);
        }
        int counter = 0;

        for (auto& index : grid.indices) {
            if ((counter++ % ((TESSELLATION_LEVEL + 1) * 2) == 0)) indices.push_back(0xFFFFFFFF);
            indices.push_back(index + offset);
        }
    }
    n_indices = static_cast<GLuint>(indices.size());
}
	
void j7Bezier::tessellate(const int L) {
	// Based on info from http://graphics.cs.brown.edu/games/quake/quake3.html, with simplified code and better use of C++

    // The number of vertices along a side is 1 + num edges
    const int L1 = L + 1;

    vertex.resize(L1 * L1);

    // Compute the vertices
    for (int i = 0; i <= L; ++i) {
        float a = static_cast<float>(i) / L;
        float b = 1.0f - a;

        vertex[i] =
            controls[0] * (b * b) + 
            controls[3] * (2 * b * a) +
            controls[6] * (a * a);
    }

    for (int i = 1; i <= L; ++i) {
        float a = static_cast<float>(i) / L;
        float b = 1.0f - a;

        BSPVertex temp[3];

        for (int j = 0; j < 3; ++j) {
            int k = 3 * j;
            temp[j] =
                controls[k + 0] * (b * b) + 
                controls[k + 1] * (2 * b * a) +
                controls[k + 2] * (a * a);
        }

        for (int j = 0; j <= L; ++j) {
            float a = static_cast<float>(j) / L;
            float b = 1.0f - a;

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
} frustrum;

frustrum getViewFrustrum(glm::mat4 matrix) {
	frustrum view;

	view.left = glm::fvec4(
		matrix[4][1] + matrix[1][1],
		matrix[4][2] + matrix[1][2],
		matrix[4][3] + matrix[1][3],
		matrix[4][4] + matrix[1][4]);

	view.right = glm::fvec4(
		matrix[4][1] - matrix[1][1],
		matrix[4][2] - matrix[1][2],
		matrix[4][3] - matrix[1][3],
		matrix[4][4] - matrix[1][4]);

	view.top = glm::fvec4(
		matrix[4][1] - matrix[2][1],
		matrix[4][2] - matrix[2][2],
		matrix[4][3] - matrix[2][3],
		matrix[4][4] - matrix[2][4]);

	view.bottom = glm::fvec4(
		matrix[4][1] + matrix[2][1],
		matrix[4][2] + matrix[2][2],
		matrix[4][3] + matrix[2][3],
		matrix[4][4] + matrix[2][4]);

	view.nearclip = glm::fvec4(
		matrix[4][1] + matrix[3][1],
		matrix[4][2] + matrix[3][2],
		matrix[4][3] + matrix[3][3],
		matrix[4][4] + matrix[3][4]);

	view.farclip = glm::fvec4(
		matrix[4][1] - matrix[3][1],
		matrix[4][2] - matrix[3][2],
		matrix[4][3] - matrix[3][3],
		matrix[4][4] - matrix[3][4]);
	return view;
}
float distanceToPoint(const glm::fvec4 plane, const glm::fvec3 point) {
	return plane.x * point.x
		+ plane.y * point.y
		+ plane.z * point.z
		+ plane.w;
}
bool isInFrustrum(frustrum view, glm::fvec3 point) {
	if (distanceToPoint(view.left, point) < 0) return false;
	if (distanceToPoint(view.right, point) < 0) return false;
	if (distanceToPoint(view.top, point) < 0) return false;
	if (distanceToPoint(view.bottom, point) < 0) return false;
	if (distanceToPoint(view.nearclip, point) < 0) return false;
	if (distanceToPoint(view.farclip, point) < 0) return false;
	return true;
}

int q3BSP::findCurrentLeaf(const glm::vec3 position) {
    int index = 0;
    while (index >= 0) {
        const BSPNode& node = nodes[index];
        const BSPPlane& plane = planes[node.plane];
        // Distance from point to a plane
        const double distance = glm::dot(position, plane.normal) - plane.distance;

        if (distance >= 0)  index = node.children[0];
        else index = node.children[1];
    }
    return -index - 1;
}

bool q3BSP::isClusterVisible(const int testCluster, const int visCluster) {
	//Sanity check
    if ((visData.vecs.size() == 0) || (visCluster < 0)) return true; // Show all faces when outside map or there is no visdata

	if (visData.vecs[(testCluster >> 3) + (visCluster * visData.sz_vecs)] & (1 << (testCluster & 7))) return true;
	return false;

}

std::vector<int> q3BSP::makeListofVisibleFaces(const glm::vec3 position, glm::mat4 viewmatrix) {
    static std::vector<int> visibleFaces;
	std::vector<bool> alreadyVisible; //Keep track of already added faces
	alreadyVisible.resize(faces.size());
    static int prevLeaf = -1;
    int currentLeaf = findCurrentLeaf(position);
    if (currentLeaf == prevLeaf) return visibleFaces; // Same cluster as last frame
    visibleFaces.resize(0); // reset

	//frustrum viewfrustrum = getViewFrustrum(viewmatrix);


    for (auto& leaf : leafs) {
		glm::fvec3 min(leaf.mins[0], leaf.mins[1], leaf.mins[2]);
		glm::fvec3 max(leaf.maxs[0], leaf.maxs[1], leaf.maxs[2]);
		if (isClusterVisible(leaf.cluster, leafs[currentLeaf].cluster) /*&& !isInFrustrum(viewfrustrum, min) && !isInFrustrum(viewfrustrum, max)*/) { // If this leaf is visible
			for (int j = leaf.leafface; j < leaf.leafface + leaf.n_leaffaces; ++j) { // Then push all its faces to vector
				if (!alreadyVisible[leafFaces[j]]) visibleFaces.push_back(leafFaces[j]);
				alreadyVisible[leafFaces[j]] = true; // Prevent faces from being added more than once
            }
        }
    }

	for (auto& face : visibleFaces) {

	}
    prevLeaf = currentLeaf;
	return visibleFaces;
}

typedef struct {
    std::string map;
    glm::fvec2 tcMod_scroll;
    glm::fvec2 tcMod_scale;
    bool depthWrite;
//    std::array<std::string> blendfunc;

} shaderStage;

void q3BSP::parseShader(const std::string shadername) {
	// This is just a test to get the sky rendering, it doesn't parse all shader files yet.
	std::cout << "Parsing shader...\n";
	std::string filename = "scripts/sky.shader";
	std::ifstream file(filename);
	if (!file.is_open()) return;
	std::string shaderSource;
	shaderSource.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	//Find the beginning of the shader
	unsigned long open = shaderSource.find(shadername, 0);
	std::cout << shadername << " found at position " << open << '\n';
	open += shadername.length(); // skip to after the shader name
	open = shaderSource.find_first_of("\n{", open) + 1;
	unsigned long close = shaderSource.find("\n}", open); // Closing brace that isn't preceeded by a tab
	std::cout << "Shader length: " << (close - open) << '\n';
	std::cout << "Got shader: \n" << shaderSource.substr(open, close - open + 2) << '\n';
	std::map<std::string, std::string> linepair;


	while (open < close) {
		std::string line;
		std::vector<std::string> tokens;
		open = shaderSource.find('\t', open) + 1; // Find the first item
		int endline = shaderSource.find('\n', open); // Find the end of the line
		line =  shaderSource.substr(open, endline - open);
        if (line.at(0) == '/') { // Comment
            open = shaderSource.find('\n',open);
            continue;
        }
		int tokenOffset = 0;
		while (tokenOffset != std::string::npos) {
			tokenOffset = line.find(' ', tokenOffset);
			if (tokenOffset == std::string::npos) continue;
			std::string token = line.substr(0, tokenOffset);
			line = line.substr(tokenOffset + 1, line.length() - token.length());
			tokens.push_back(token);
		}
		tokens.push_back(line);
		std::cout << "Num of tokens: " << tokens.size() << '\n';
        if (tokens[0] == "qer_editorimage") continue; // Only used for editor


		/*std::cout << "Token:" << line.substr(0, tokenOffset) << "::\n";
		tokens.push_back(line.substr(0, tokenend));
		int token2 = line.find(' ', tokenend + 1);
		if (token2 == std::string::npos) std::cout << "No more tokens.\n";*/
		
		open = shaderSource.find('\n',open);
		
		//open = close;

	}


	int depth = 1; // We start within one clause

	/*
	// Split into vector of each clause
	std::vector<std::string> clauses;
	while (open != std::string::npos) {
		close = entitystring->find_first_of('}', open + 1); // Find closing brace starting at last opening brace
		clauses.push_back(entitystring->substr(open, close - open)); // Push, minus open & close braces & newlines
		open = entitystring->find_first_of('{', close + 1); // Set next start location to after closing brace
	}
	std::cout << clauses.size() << " clauses found.\n";

	// Convert each clause into a BSPEntity object

	for (auto& clause : clauses) {
		BSPEntity tempEntity(clause);
		// Parse entities ::TODO:: Push only unhandled entities to vector
		if (tempEntity.pair["classname"] == "info_player_deathmatch") cameraPositions.push_back(camPos(tempEntity));
		else if (tempEntity.pair["classname"] == "worldspawn") worldMusic = tempEntity.pair["music"];
		else if (tempEntity.pair["classname"] == "light") lightPositions.push_back(lightPos(tempEntity));
		else entities.push_back(tempEntity); // Not handled, so throw it in the vector
	}
	std::cout << "  Map music: " << worldMusic << '\n';
	std::cout << "  " << cameraPositions.size() << " spawn points found.\n";
	std::cout << "  " << lightPositions.size() << " lights found.\n";*/
}




