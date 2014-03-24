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
?) Install MSVC2013 Express on work computer, switch to range based forloops and other C++11 stuff
*/

#include <iostream> // std::cout, std::cerr
#include <fstream> // std::ifstream
#include <vector> // std::vector

#include <GLEW/glew.h>
#include <SFML/OpenGL.hpp> // OpenGL datatypes
#include <glm/glm.hpp>

#include "q3bsploader.h"

q3BSP::q3BSP(std::string filename) {
    std::cout << "Loading " << filename.c_str() << '\n';

    // Load file to memory
    std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) { std::cerr << "Couldn't open file.\n"; return; } // Couldn't open file
    unsigned size = (unsigned)file.tellg();
    file.seekg(0);
	std::vector<char> memblock;
	memblock.reserve(size);
    file.read(&memblock[0], size);
    file.close();
    
    // Read and check header
    memcpy(&header, &memblock[0], sizeof(BSPHeader));
	if (std::string(IDENT).compare(0,4,header.magicNumber,4)) { std::cerr << "Invalid format: \n" << header.magicNumber[0] << header.magicNumber[1] << header.magicNumber[2] << header.magicNumber[3] << '\n'; return; }
    if (header.version != IBSP_VERSION) {
        if (header.version == 47) std::cerr << "IBSP v.47: QuakeLive or RTCW map? Will try to load anyways.\n";
        else { std::cerr << "Invalid BSP version: " << header.version << '\n'; return; }
    }
    else std::cout << "File format and version appear ok.\n";
    
    // Read lumps
    // Lump 0: Entities
	std::string tempEntityString;
    tempEntityString.insert(0, &memblock[header.direntries[Entities].offset], header.direntries[Entities].length);
    std::cout << "Lump 0: " << tempEntityString.size() << " characters of entities read. ";
	//parseEntities(tempEntityString); // Parse entity string and populate vector of entities ::TODO:: Nothing is actually done with this data yet
    
    // Lump 1: Textures
    unsigned numEntries = header.direntries[Textures].length / sizeof(BSPTexture);
    std::cout << "Lump 1: " << numEntries << " texture(s) found.\n";
    textures.reserve(numEntries);
    BSPTexture tempTexture;
    for (unsigned i = 0; i < numEntries; ++i) {
        memcpy(&tempTexture,
               &memblock[header.direntries[Textures].offset + i * sizeof(BSPTexture)],
               sizeof(BSPTexture));
        textures.push_back(tempTexture);
    }
    for (unsigned i = 0; i < textures.size(); ++i) std::cout << "  " << i << ':' << textures[i].name << '\n';
    
    // Lump 2: Planes
    
    // Lump 3: Nodes
    
    // Lump 4: Leafs
    
    // Lump 5: Leaffaces
    
    // Lump 6: Leafbrushes
    
    // Lump 7: Models
    
    // Lump 8: Brushes
    
    // Lump 9: Brushsides
    
    // Lump 10: Vertexes
    numEntries = header.direntries[Vertexes].length / sizeof(BSPVertex);
    std::cout << "Lump 10: " << numEntries << " vertex(es) found.\n";
    vertices.reserve(numEntries);
    BSPVertex tempVertex;
    for (unsigned i = 0; i < numEntries; ++i) {
        memcpy(&tempVertex,
               &memblock[header.direntries[Vertexes].offset + i * sizeof(BSPVertex)],
               sizeof(BSPVertex));
        vertices.push_back(tempVertex);
    }
	
    // Lump 11: Meshverts
    numEntries = header.direntries[Meshverts].length / sizeof(BSPMeshVert);
    std::cout << "Lump 11: " << numEntries << " Meshvert(s) found.\n";
    meshVerts.reserve(numEntries);
	memcpy(meshVerts.data(),
               &memblock[header.direntries[Meshverts].offset],
               header.direntries[Meshverts].length);
    
    // Lump 12: Effects
	numEntries = header.direntries[Effects].length / sizeof(BSPEffect);
	std::cout << "Lump 12: " << numEntries << " effects(s) found.\n";
	effects.reserve(numEntries);
	memcpy(effects.data(),
               &memblock[header.direntries[Effects].offset],
               header.direntries[Effects].length);
    
    // Lump 13: Faces
    numEntries = header.direntries[Faces].length / sizeof(BSPFace);
    std::cout << "Lump 13: " << numEntries << " face(s) found.\n";
    faces.reserve(numEntries);
    BSPFace tempFace;
    for (unsigned i = 0; i < numEntries; ++i) {
        memcpy(&tempFace,
               &memblock[header.direntries[Faces].offset + i * sizeof(BSPFace)],
               sizeof(BSPFace));
        faces.push_back(tempFace);
    }
    groupMeshByTexture(); // Sort faces into groups by texture id

    // Lump 14: Lightmaps
	numEntries = header.direntries[Lightmaps].length / sizeof(BSPLightmap);
	std::cout << "Lump 14: " << numEntries << " lightmap(s) found.\n";
    BSPLightmap tempLightmap;
	for (unsigned i = 0; i < numEntries; ++i) {
		memcpy(&tempLightmap,
				&memblock[header.direntries[Lightmaps].offset + i * sizeof(BSPLightmap)],
				sizeof(BSPLightmap));
		lightmaps.push_back(tempLightmap);
	}
	bindLightmaps();
    // Lump 15: Lightvols
    
    // Lump 16: Visdata
    
}

void q3BSP::bindLightmaps() {
	for (unsigned i = 0; i < lightmaps.size(); ++i) {
		GLuint id = 0;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, &lightmaps[i].data);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		lightmapGLIDS.push_back(id);
	}
}

std::vector<GLuint> q3BSP::getIndices(unsigned entry) {
	if (facesByTexture[entry].size() == 0) {
		std::cerr << " No faces found for group " << entry << ".\n";
		return std::vector<GLuint>(); // No meshes for this texture, return empty vector
	}
	std::vector<GLuint> temp; // A vector of lists of indices
	for (unsigned i = 0; i < facesByTexture[entry].size(); ++i) { // for each face in set
		switch(facesByTexture[entry][i].type) {
		case 1:	// Polygons
		case 3:	// Meshes
			for (int k = 0; k < facesByTexture[entry][i].n_meshverts; ++k) { // For each meshvert in face
				GLuint value = facesByTexture[entry][i].vertex + meshVerts[k + facesByTexture[entry][i].meshvert].offset;
				temp.push_back(value);
			}
			break;

		case 2:	// Patches
			patches.push_back(dopatch(facesByTexture[entry][i]));
			break;
	
		case 4:	// Billboards, skip
			std::cerr << "Face group " << entry << " is billboard(s).\n";
			break;

		default: // Unknown type, skip
			std::cerr << "Unknown face type: " << facesByTexture[entry][0].type << '\n';
			break;
		}
	}
	return temp;
}

void q3BSP::groupMeshByTexture() {
    //The index for the face = the texture index
	//There are other ways to render the right texture for the face, but sorting them enables the number of texture swaps to be limited.
    facesByTexture.resize(textures.size()); // Reserve 1 entry per texture
    for (unsigned i = 0; i < faces.size(); ++i) facesByTexture[faces[i].texture].push_back(faces[i]);
    std::cout << "Faces sorted: " << faces.size() << " faces -> " << facesByTexture.size() << " meshes.\n";
}

void q3BSP::parseEntities(std::string entitystring) {
	unsigned long open = entitystring.find_first_of('{', 0);
	unsigned long close = 0;

	// Split into vector of each clause
	std::vector<std::string> clauses;
	while(open != std::string::npos) {
			close = entitystring.find_first_of('}', open + 1); // Find closing brace starting at last opening brace
			clauses.push_back(entitystring.substr(open, close - open)); // Push, minus open & close braces & newlines
			open = entitystring.find_first_of('{', close + 1); // Set next start location to after closing brace
	}
	std::cout << clauses.size() << " clauses found.\n";
	
	// Convert each clause into a BSPEntity object

	for (unsigned i = 0; i < clauses.size(); ++i) {
		entities.push_back(BSPEntity(clauses[i]));
	}
}

BSPPatch q3BSP::dopatch(BSPFace face) {
	// This code just generates the control points. Actual tessellation is done by another function
	BSPPatch patch;
	if (face.type != 2) {
		std::cerr << "###################\n\nNOT A PATCH!!!\n\n###################\n";
	}

	patch.textureID = face.texture;
	int patch_size_x = (face.size[0] - 1) / 2;
	int patch_size_y = (face.size[1] - 1) / 2;
	patch.bezier.resize(patch_size_x * patch_size_y);

	int patchIndex =  0;
	int ii, n, j, nn;
	for (ii = 0, n = 0; n < patch_size_x; n++, ii = 2*n) {
	    for (j=0, nn=0; nn < patch_size_y; nn++, j = 2*nn) {
			int index = 0;
			for (int ctr = 0; ctr < 3; ++ctr) {
				int pos = ctr * face.size[0];
	
				patch.bezier[patchIndex].controls[index++] = vertices[face.vertex + ii + face.size[0] * j + pos];
				patch.bezier[patchIndex].controls[index++] = vertices[face.vertex + ii + face.size[0] * j + pos + 1];
				patch.bezier[patchIndex].controls[index++] = vertices[face.vertex + ii + face.size[0] * j + pos + 2];                                            
			}      
			patch.bezier[patchIndex++].tessellate(TESSELLATION_LEVEL);
		}
	}
	return patch;
}
	
void j7Bezier::tessellate(int L) {
	// Based on info from http://graphics.cs.brown.edu/games/quake/quake3.html, with simplified code and better use of C++
 //   level = L;

    // The number of vertices along a side is 1 + num edges
    const int L1 = L + 1;

    vertex.resize(L1 * L1);

    // Compute the vertices
    for (int i = 0; i <= L; ++i) {
        float a = (float)i / L;
        float b = 1.0f - a;

        vertex[i] =
            controls[0] * (b * b) + 
            controls[3] * (2 * b * a) +
            controls[6] * (a * a);
    }

    for (int i = 1; i <= L; ++i) {
        float a = (float)i / L;
        float b = 1.0f - a;

        BSPVertex temp[3];

        for (int j = 0; j < 3; ++j) {
            int k = 3 * j;
            temp[j] =
                controls[k + 0] * (b * b) + 
                controls[k + 1] * (2 * b * a) +
                controls[k + 2] * (a * a);
        }

        for(int j = 0; j <= L; ++j) {
            float a = (float)j / L;
            float b = 1.0f - a;

            vertex[i * L1 + j]=
                temp[0] * (b * b) + 
                temp[1] * (2 * b * a) +
                temp[2] * (a * a);
        }
    }

    // Compute the indices
    indices.resize(L * L1 * 2);

    for (int row = 0; row < L; ++row) {
        for(int col = 0; col <= L; ++col)	{
            indices[(row * (L + 1) + col) * 2 + 1] = row       * L1 + col;
            indices[(row * (L + 1) + col) * 2]     = (row + 1) * L1 + col;
        }
    }

    trianglesPerRow.resize(L);
    rowIndices.resize(L);
    for (int row = 0; row < L; ++row) {
        trianglesPerRow[row] = 2 * L1;
        rowIndices[row]      = row * 2 * L1 * sizeof(GLuint);
    }
    //Normalize the normals
	for (unsigned i = 0; i < vertex.size(); ++i) {
		vertex[i].normal = glm::normalize(vertex[i].normal);
	}

	// Create index buffer for this bezier
	glGenBuffers(2, &bufferID[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, bufferID[1]);
    glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(BSPVertex), vertex.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void j7Bezier::render() {
//	glEnableClientState(GL_VERTEX_ARRAY);
//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//	glEnableClientState(GL_NORMAL_ARRAY);
//	glEnableClientState(GL_COLOR_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, bufferID[1]);
    glVertexPointer(3, GL_FLOAT, sizeof(BSPVertex), 0);
    glTexCoordPointer(2, GL_FLOAT, sizeof(BSPVertex), (GLvoid*)offsetof(BSPVertex, texcoord));
    glNormalPointer(GL_FLOAT, sizeof(BSPVertex), (GLvoid*)offsetof(BSPVertex, normal));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(BSPVertex), (GLvoid*)offsetof(BSPVertex, color));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID[0]);
  //  for (int i = 0; i < rowIndices.size(); ++i) {
  //      glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, (const GLvoid*)(5*sizeof(GLuint)*i));
  //  }
    glMultiDrawElements(GL_TRIANGLE_STRIP, trianglesPerRow.data(), GL_UNSIGNED_INT, (const GLvoid**)rowIndices.data(), (GLsizei)trianglesPerRow.size());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

//	glDisableClientState(GL_COLOR_ARRAY);
//	glDisableClientState(GL_NORMAL_ARRAY);
//	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//	glDisableClientState(GL_VERTEX_ARRAY);
}
