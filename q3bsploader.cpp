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
5) Our early exits are leaking the memblock array? Convert to a vector?
6) Add shader support to j7Model to enable lighting
7) Figure out collision detection
?) Replace memcpy with a c++ equivalent?
?) Add Doom3 BSP support
?) Add Quake1/2/etc BSP support
?) Open source and publish
*/


#include <iostream> // std::cout, std::cerr
#include <fstream> // std::ifstream
#include <vector> // std::vector
#include <memory> // std::unique_ptr
#include <SFML/OpenGL.hpp> // OpenGL datatypes
#include "q3bsploader.h"

enum LUMPNAMES {
	Entities=0,
	Textures,
	Planes,
	Nodes,
	Leafs,
	Leaffaces,
	Leafbrushes,
	Models,
	Brushes,
	Brushsides,
	Vertexes,
	Meshverts,
	Effects,
	Faces,
	Lightmaps,
	Lightvols,
	Visdata
};


q3BSP::q3BSP(std::string filename) {
    std::cout << "Loading " << filename.c_str() << '\n';

    // Load file to memory
    std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) { std::cerr << "Couldn't open file.\n"; return; } // Couldn't open file
    unsigned size = (unsigned)file.tellg();
    file.seekg (0);
	std::vector<char> memblock;
	memblock.reserve(size);
    file.read(&memblock[0], size);
    file.close();
    
    // Read and check header
    memcpy(&header, &memblock[0], sizeof(BSPHeader));
    if (std::string("IBSP").compare(0,4,header.magicNumber,4)) { std::cerr << "Invalid format: \n" << header.magicNumber[0] << header.magicNumber[1] << header.magicNumber[2] << header.magicNumber[3] << '\n'; return; }
    if (header.version != 0x2e) { std::cerr << "Invalid BSP version.\n"; return; }
    std::cout << "File format and version appear ok.\n";
    
    // Read lumps
    // Lump 0: Entities
    entities.entities.insert(0, &memblock[header.direntries[Entities].offset], header.direntries[Entities].length);
    std::cout << "Lump 0: " << entities.entities.size() << " characters of entities read.\n";
    // ::TODO:: Parse entities data
    
    // Lump 1: Textures
    unsigned numEntries = header.direntries[Textures].length / sizeof(BSPTexture);
    std::cout << "Lump 1: " << numEntries << " texture(s) found.\n";
    textures.reserve(numEntries);
    BSPTexture tempTex;
    for (unsigned i = 0; i < numEntries; ++i) {
        memcpy(&tempTex,
               &memblock[header.direntries[Textures].offset + i * sizeof(BSPTexture)],
               sizeof(BSPTexture));
        textures.push_back(tempTex);
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
    BSPMeshVert tempMeshVert;
    for (unsigned i = 0; i < numEntries; ++i) {
        memcpy(&tempMeshVert,
               &memblock[header.direntries[Meshverts].offset + i * sizeof(BSPMeshVert)],
               sizeof(BSPMeshVert));
        meshVerts.push_back(tempMeshVert);
    }
    
    // Lump 12: Effects
    
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
    
    // Lump 15: Lightvols
    
    // Lump 16: Visdata
    
}

//Data getters
std::vector<GLfloat> q3BSP::getVertices() {
    std::vector<GLfloat> temp;
	temp.reserve(vertices.size());
    for (unsigned i = 0; i < vertices.size(); ++i) for (int j = 0; j < 3; ++j) temp.push_back(vertices[i].position[j]);
    return temp;
}
std::vector<GLfloat> q3BSP::getNormals() {
    std::vector<GLfloat> temp;
	temp.reserve(vertices.size());
    for (unsigned i = 0; i < vertices.size(); ++i) for (int j = 0; j < 3; ++j) temp.push_back(vertices[i].normal[j]);
    return temp;
}
std::vector<GLfloat> q3BSP::getVertexColors() {
    std::vector<GLfloat> temp;
	temp.reserve(vertices.size());
    for (unsigned i = 0; i < vertices.size(); ++i) for (int j = 0; j < 4; ++j) temp.push_back(vertices[i].color[j]); // RGBA
    return temp;
}

std::vector<GLuint> q3BSP::getIndices(unsigned entry) {
    std::vector<GLuint> temp; // A vector of lists of indices
        if (facesByTexture[entry].size() != 0 // Empty faceset
            && facesByTexture[entry][0].type != 1 // Non-polygons, probably unsafe assumption that all faces with same texture are same type
            && facesByTexture[entry][0].type != 3) { // Non-mesh, I still don't understand how they differ from polygons
            std::cout << "Face group " << entry << " is type " << facesByTexture[entry][0].type << '\n';
		}
        else for (unsigned j = 0; j < facesByTexture[entry].size(); ++j) { // For each face in set
            for (int k = 0; k < facesByTexture[entry][j].n_meshverts; ++k) { // For each meshvert in face
                GLuint value = facesByTexture[entry][j].vertex + meshVerts[k + facesByTexture[entry][j].meshvert].offset;
                temp.push_back(value);
            }
        }
    return temp;
}

std::vector<GLfloat> q3BSP::getTextureCoordinates() {
    std::vector<GLfloat> temp;
	temp.reserve(vertices.size());
    for (unsigned i = 0; i < vertices.size(); ++i) {
        temp.push_back(vertices[i].texcoord[0][0]); //[1][x] is lightmap coords
        temp.push_back(vertices[i].texcoord[0][1]);
    }
    return temp;
}
std::vector<std::string> q3BSP::getTextureNames() {
    std::vector<std::string> temp;
	temp.reserve(textures.size());
    for (unsigned i = 0; i < textures.size(); ++i) temp.push_back(textures[i].name);    
    return temp;
}

void q3BSP::groupMeshByTexture() {
    //The index for the face = the texture index
	//There are other ways to render the right texture for the face, but sorting them enables the number of texture swaps to be limited.
    facesByTexture.resize(textures.size()); // Reserve 1 entry per texture
    for (unsigned i = 0; i < faces.size(); ++i) facesByTexture[faces[i].texture].push_back(faces[i]);
    std::cout << "Faces sorted: " << faces.size() << " faces -> " << facesByTexture.size() << " meshes.\n";
}
