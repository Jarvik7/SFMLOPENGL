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
#include <GLEW/glew.h>
#include <SFML/OpenGL.hpp> // OpenGL datatypes
#include <SFML/System/Vector3.hpp>
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
    if (header.version != 0x2e) { std::cerr << "Invalid BSP version.\n"; return; } // Version 46 = Quake III (47 = RTCW / QuakeLive)
    std::cout << "File format and version appear ok.\n";
    
    // Read lumps
    // Lump 0: Entities
    entities.entities.insert(0, &memblock[header.direntries[Entities].offset], header.direntries[Entities].length);
    std::cout << "Lump 0: " << entities.entities.size() << " characters of entities read. ";
	parseEntities(entities.entities);
    // ::TODO:: Parse entities data
    
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

std::vector<GLuint> q3BSP::getIndices(unsigned entry) {
	if (facesByTexture[entry].size() == 0) return std::vector<GLuint>(); // No meshes for this texture, return empty vector
	switch(facesByTexture[entry][0].type) {
	case 1:		// Polygons
	case 3:	{	// Meshes
		std::vector<GLuint> temp; // A vector of lists of indices
		for (unsigned j = 0; j < facesByTexture[entry].size(); ++j) { // For each face in set
            for (int k = 0; k < facesByTexture[entry][j].n_meshverts; ++k) { // For each meshvert in face
                GLuint value = facesByTexture[entry][j].vertex + meshVerts[k + facesByTexture[entry][j].meshvert].offset;
                temp.push_back(value);
            }
        }
		return temp;
	}
	case 2:		// Patches
		std::cerr << "Face group " << entry << " is patch(es).\n";
		for (unsigned i = 0; i < facesByTexture[entry].size(); ++i) {
			patches.push_back(j7Bezier(facesByTexture[entry][i]));
		}
		break;

	case 4:		// Billboards
		std::cerr << "Face group " << entry << " is billboard(s).\n";
		break;
	default:
		std::cerr << "Unknown face type: " << facesByTexture[entry][0].type << '\n';
		break;
	}
	return std::vector<GLuint>(); // No supported face types, return empty vector
}

void q3BSP::groupMeshByTexture() {
    //The index for the face = the texture index
	//There are other ways to render the right texture for the face, but sorting them enables the number of texture swaps to be limited.
    facesByTexture.resize(textures.size()); // Reserve 1 entry per texture
    for (unsigned i = 0; i < faces.size(); ++i) facesByTexture[faces[i].texture].push_back(faces[i]);
    std::cout << "Faces sorted: " << faces.size() << " faces -> " << facesByTexture.size() << " meshes.\n";
}

typedef struct {
	std::string classname;
	std::string message;
	std::string music;
	std::string model;
	sf::Vector3i origin;
	int angle;
	sf::Vector3f _color;
	int ambient;
	int light;
	std::string targetname;
	std::string target;
	int spawnflags;
	int radius;
} BSPEntity;

void q3BSP::parseEntities(std::string entities) {
    std::vector<std::string> clauses;
	unsigned open=0;
	unsigned close=0;
	bool done=false;

	while((open = entities.find_first_of('{',open)) != std::string::npos) {
			close = entities.find_first_of('}',open+1); // Find closing brace starting at last opening brace
			clauses.push_back(entities.substr(open+2, close-open-3)); // Push, minus open & close braces & newlines
			open=close+1; // Set next start location to after closing brace
	}
	std::cout << clauses.size() << " clauses found.\n";
}

typedef struct {
	GLfloat x;
	GLfloat y;
	GLfloat z;
} vertex;

j7Bezier::j7Bezier(BSPFace face) {


}


/*
void j7Bezier::tessellate(int L) { // Based on Paul Baker's Octagon, apparently
    level = L;

    // The number of vertices along a side is 1 + num edges
    const int L1 = L + 1;

    vertex.resize(L1 * L1);

    // Compute the vertices
    for (int i = 0; i <= L; ++i) {
        double a = (double)i / L;
        double b = 1 - a;

        vertex[i] =
            controls[0] * (b * b) + 
            controls[3] * (2 * b * a) +
            controls[6] * (a * a);
    }

    for (int i = 1; i <= L; ++i) {
        double a = (double)i / L;
        double b = 1.0 - a;

        sf::Vector3f temp[3];

        for (int j = 0; j < 3; ++j) {
            int k = 3 * j;
            temp[j] =
                controls[k + 0] * (b * b) + 
                controls[k + 1] * (2 * b * a) +
                controls[k + 2] * (a * a);
        }

        for(int j = 0; j <= L; ++j) {
            double a = (double)j / L;
            double b = 1.0 - a;

            vertex[i * L1 + j]=
                temp[0] * (b * b) + 
                temp[1] * (2 * b * a) +
                temp[2] * (a * a);
        }
    }


    // Compute the indices
    indices.resize(L * (L + 1) * 2);

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
        rowIndices[row]      = &indices[row * 2 * L1];
    }
    
}
void j7Bezier::render() {
  //  glVertexPointer(3, GL_FLOAT, 0, &vertex[0]);

    //glClientActiveTextureARB(GL_TEXTURE0_ARB);
   // glTexCoordPointer(2, GL_FLOAT,sizeof(BSPVertex), &vertex[0].textureCoord);
	/*
    glClientActiveTextureARB(GL_TEXTURE1_ARB);
    glTexCoordPointer(2, GL_FLOAT, sizeof(BSPVertex), &vertex[0].lightmapCoord);
	*/
   // glMultiDrawElements(GL_TRIANGLE_STRIP, &trianglesPerRow[0], GL_UNSIGNED_INT, reinterpret_cast<void*>(rowIndices), level);
//}
