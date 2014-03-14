/*This is based on http://www.mralligator.com/q3/
https://github.com/rohitnirmal/q3-bsp-viewer/blob/master/bsp.h also served as reference for confirmation (no license? is also based on the above)
Quake 3 BSP loader
The concept is to import the BSP as-is into appropriate data structures, and
provide functions to output the data in a modular method usable to the j7Model class

A Doom3 MD5 loader should follow, based on same code layout.

Todos:
1) Get vertices & textures going at a level similar to current ASSIMP implementation
2) Break out prototypes into a proper header
3) Add functions to feed j7Model (to be done with #1?)
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
		std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
		if (!file.is_open()) { std::cerr << "Couldn't open file.\n"; return; } // Couldn't open file

		unsigned size = (unsigned)file.tellg();
		file.seekg (0);
		char * memblock = new char[size];
		file.read(memblock, size);
		file.close();

		// Read and check  header
		memcpy(&header, memblock, sizeof(BSPHeader));
		if (std::string("IBSP").compare(0,4,header.magicNumber,4)) { std::cerr << "Invalid format.\n" << header.magicNumber[0] << header.magicNumber[1] << header.magicNumber[2] << header.magicNumber[3]; return; }
		if (header.version != 0x2e) { std::cerr << "Invalid BSP version.\n"; return; }
		std::cout << "File format and version appear ok.\n";

		// Read lumps
		// Lump 0: Entities
	//	memcpy(entities.entities,&memblock[header.direntries[Entities].offset],header.direntries[Entities].length);

		// Lump 1: Textures
		unsigned numEntries = header.direntries[Textures].length / sizeof(BSPTexture);
		std::cout << "Lump 1: " << numEntries << " texture(s) found.\n";
		BSPTexture tempTex;
		for (unsigned i = 0; i<numEntries; ++i) {
			memcpy(&tempTex,
				&memblock[header.direntries[Textures].offset + i * sizeof(BSPTexture)], 
				sizeof(BSPTexture));
			textures.push_back(tempTex);
			std::cout << i << ':' << tempTex.name << '\n';
		}

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
		BSPVertex tempVertex;
		for (unsigned i = 0; i<numEntries; ++i) {
			memcpy(&tempVertex,
				&memblock[header.direntries[Vertexes].offset + i * sizeof(BSPVertex)], 
				sizeof(BSPVertex));
			vertices.push_back(tempVertex);
		}

		// Lump 11: Meshverts
		numEntries = header.direntries[Meshverts].length / sizeof(BSPMeshVert);
		std::cout << "Lump 11: " << numEntries << " Meshvert(s) found.\n";
		BSPMeshVert tempMeshVert;
		for (unsigned i = 0; i<numEntries; ++i) {
			memcpy(&tempMeshVert,
				&memblock[header.direntries[Meshverts].offset + i * sizeof(BSPMeshVert)], 
				sizeof(BSPMeshVert));
			meshVerts.push_back(tempMeshVert);
		}
		// Lump 12: Effects

		// Lump 13: Faces
		numEntries = header.direntries[Faces].length / sizeof(BSPFace);
		std::cout << "Lump 13: " << numEntries << " face(s) found.\n";
		BSPFace tempFace;
		for (unsigned i = 0; i<numEntries; ++i) {
			memcpy(&tempFace,
				&memblock[header.direntries[Faces].offset + i * sizeof(BSPFace)], 
				sizeof(BSPFace));
			faces.push_back(tempFace);
		}

		// Lump 14: Lightmaps

		// Lump 15: Lightvols

		// Lump 16: Visdata


		delete[] memblock; // Free the memory
	}

	//Data getters
	std::vector<GLfloat> q3BSP::getVertices() {
		std::vector<GLfloat> temp;
		for (unsigned i = 0; i < vertices.size(); ++i) {
			for (int j = 0; j < 3; ++j) temp.push_back(vertices[i].position[j]);
		}
		return temp;
	}
	std::vector<GLfloat> q3BSP::getNormals() {
		std::vector<GLfloat> temp;
		for (unsigned i = 0; i < vertices.size(); ++i) {
			for (int j = 0; j < 3; ++j) temp.push_back(vertices[i].normal[j]);
		}
		return temp;
	}
	std::vector<GLuint> q3BSP::getIndices() {
		std::vector<GLuint> temp;

		for (unsigned i = 0; i < faces.size(); ++i) {
			if (faces[i].type != 1) continue; // Not a polygon, skip
			std::cout << "Face #" << i;
			//temp.push_back(faces[i].vertex); // First vertex
			for (unsigned j = 0; j < faces[i].n_meshverts; ++j) {
				std::cout << '.';
				// Remaining vertices in meshvert offset from first vertex
				temp.push_back(faces[i].vertex + meshVerts[j + faces[i].meshvert].offset);
			}
		}
		std::cout << "\nNumber of indices found: " << temp.size() << '\n';
		for (int i=0; i< temp.size(); ++i) {
			std::cout << temp[i] << '.';
		}
		std::cout << '\n';
		return temp;
	}
	/*
	std::vector<GLfloat> q3BSP::getTextureCoordinates() {
		return new std::vector<GLfloat>;
	}
	std::vector<std::string> q3BSP::getTextureNames() {
		return new std::vector<std::string>;
	} // Actual loading to be done by j7Model*/



