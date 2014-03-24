#ifndef j7Q3BSPLoader_hpp
#define j7Q3BSPLoader_hpp

#include <string> // std::string
#include <array> // std::array
#include <vector> // std::vector
#include <map>

#include <SFML/OpenGL.hpp> // OpenGL datatypes
#include <glm/glm.hpp> // glm::fvec3, glm::mat2, normalize, rotate, transform, scale

#define IDENT "IBSP"
#define IBSP_VERSION 46
#define TESSELLATION_LEVEL 12
#define HEADER_LUMPS 17

enum LUMPNAMES {
	Entities = 0,
	Textures,		//LUMP_SHADERS
	Planes,
	Nodes,
	Leafs,
	Leaffaces,		//LUMP_LEAFSURFACES
	Leafbrushes,
	Models,
	Brushes,
	Brushsides,
	Vertexes,		//LUMP_DRAWVERTS
	Meshverts,		//LUMP_DRAWINDEXES
	Effects,		//LUMP_FOGS
	Faces,			//LUMP_SURFACES
	Lightmaps,
	Lightvols,		//LUMP_LIGHTGRID
	Visdata			//LUMP_VISIBILITY
};

typedef struct {
	int offset;
	int length;
} dirEntry;

typedef struct {
	char magicNumber[4];
	int version;
	dirEntry direntries[HEADER_LUMPS];
} BSPHeader;

// Lump 0
typedef struct {
	std::string type;
	std::string value;
} BSPEntityClausePair;
class BSPEntity {
public:
	std::map<std::string, std::string> pair;

	BSPEntity(std::string clause) { // Takes a single clause (within {} braces) and populates a vector of type/value pairs
		unsigned long open = clause.find_first_of('"', 0);
		unsigned long close = 0;
		std::string type, value;

		while (open != std::string::npos) {
			close = clause.find_first_of('"', open + 1); // First match is type
			type = clause.substr(open+1, close - open - 1);

			open = clause.find_first_of('"', close + 1);;
			close = clause.find_first_of('"', open + 1); // Second match is value
			value = clause.substr(open + 1, close - open - 1);

			pair[type] = value;
			
			open=clause.find_first_of('"', close + 1); // Go around for next line
		}
	}

	glm::fvec3 getVector(std::string type) {
		glm::fvec3 temp; // The entity coordinates are not floats???
		if (type != "origin"
			|| type != "_color") return temp; // This index is not a vector
		unsigned long open = 0;
		unsigned long close = 0;

		close = pair[type].find_first_of(' ', open);
		std::string token = pair[type].substr(open, close - 1);
		temp[0] = (float)atof(token.c_str());

		open = close+1;
		close = pair[type].find_first_of(' ', open);
		token = pair[type].substr(open, close - 1);
		temp[1] = (float)atof(token.c_str());

		open = close+1;
		close = pair[type].find_first_of(' ', open);
		token = pair[type].substr(open, close - 1);
		temp[2] = (float)atof(token.c_str());

		return temp;
	}
};

typedef struct
{
    char name[64];
    int	flags;
    int	contents;
} BSPTexture; // Lump 1

//Lump 10
class BSPVertex
{
public:
	// Members must be in this order as contents are memcpy'd into them from the binary
	glm::fvec3 position;
	glm::fmat2 texcoord; //0=surface, 1=lightmap.
	glm::fvec3 normal;
    
    std::array<unsigned char, 4> color;	//vertex color, in RGBA, as unsigned.

	// Basic vector math
	BSPVertex operator+(BSPVertex a) {
		BSPVertex temp;

		temp.position = this->position + a.position;
		temp.normal = this->normal + a.normal;
		temp.texcoord = this->texcoord + a.texcoord;
        temp.color = this->color;
		return temp;
	}
	BSPVertex operator*(float a) {
		BSPVertex temp;

		temp.position = this->position * a;
		temp.normal = this->normal * a;
		temp.texcoord = this->texcoord * a;
        temp.color = this->color;
		return temp;
	}
};

typedef struct
{
    int	offset;	//Vertex index offset, relative to first vertex of corresponding face.
} BSPMeshVert; // Lump 11

typedef struct {
	char name[64];
	int brush;
	int unknown; // Always 5, except in q3dm8, which has one effect with -1.
} BSPEffect; // Lump 12

typedef struct
{
    int	texture;	//Texture index.
    int	effect;	//Index into lump 12 (Effects), or -1.
    int	type;	//Face type. 1=polygon, 2=patch, 3=mesh, 4=billboard.
    int	vertex;	//Index of first vertex.
    int	n_vertexes;	//Number of vertices.
    int	meshvert;	//Index of first mesh vertex.
    int	n_meshverts;	//Number of meshverts.
    int	lm_index;	//Lightmap index.
    int	lm_start[2];	//Corner of this face's lightmap image in lightmap.
    int	lm_size[2];	//Size of this face's lightmap image in lightmap.
    float	lm_origin[3];	//World space origin of lightmap.
    float	lm_vecs[2][3];	//World space lightmap s and t unit vectors.
    float	normal[3];	//Surface normal.
    int	size[2];	//Patch dimensions. 0=width, 1=height.
} BSPFace; // Lump 13

class j7Bezier {
public:
	std::array<BSPVertex,9> controls;
	void tessellate(int level);
	void render();
	GLuint bufferID[2];

private:
	//int level;
	std::vector<BSPVertex> vertex;
	std::vector<GLuint> indices;
	std::vector<GLsizei> trianglesPerRow;
	std::vector<size_t> rowIndices;
};

typedef struct {
	std::vector<j7Bezier> bezier;
	GLuint textureID;
} BSPPatch;

class q3BSP {
public:
	q3BSP(std::string filename);
	std::vector<GLuint> getIndices(unsigned entry);
    std::vector<std::vector<BSPFace>> facesByTexture;
	std::vector<BSPVertex> vertices;
	std::vector<BSPTexture> textures;
	std::vector<BSPPatch> patches;
	std::vector<BSPEntity> entities;
	std::vector<BSPEffect> effects;

private:
	BSPHeader header;
	std::vector<BSPMeshVert> meshVerts;
	std::vector<BSPFace> faces;
    void groupMeshByTexture();
	void parseEntities(std::string entities);
	BSPPatch dopatch(BSPFace face);
};

#endif