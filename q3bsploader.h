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
		//if (type != "origin"
		//	&& type != "_color") return temp; // This index is not a vector
		// This parses correctly thanks to atof, but the second token actually contains y and z coords??
		unsigned long open = 0;
		unsigned long close = 0;

		close = pair[type].find_first_of(' ', open);
		std::string token = pair[type].substr(open, close);
		temp[0] = (float)atof(token.c_str())/255.0f;

		open = close + 1;
		close = pair[type].find_first_of(' ', open);
		token = pair[type].substr(open, close);
		temp[1] = (float)atof(token.c_str())/255.0f;

		open = close + 1;
		close = pair[type].find_first_of(' ', open);
		token = pair[type].substr(open, close);
		temp[2] = (float)atof(token.c_str())/255.0f;

		return temp;
	}

	glm::fvec4 getVector4(std::string type) {
		glm::fvec4 temp; // The entity coordinates are not floats???
		//if (type != "origin"
		//	&& type != "_color") return temp; // This index is not a vector
		// This parses correctly thanks to atof, but the second token actually contains y and z coords??
		unsigned long open = 0;
		unsigned long close = 0;

		close = pair[type].find_first_of(' ', open);
		std::string token = pair[type].substr(open, close);
		temp[0] = (float)atof(token.c_str());

		open = close + 1;
		close = pair[type].find_first_of(' ', open);
		token = pair[type].substr(open, close);
		temp[1] = (float)atof(token.c_str());

		open = close + 1;
		close = pair[type].find_first_of(' ', open);
		token = pair[type].substr(open, close);
		temp[2] = (float)atof(token.c_str());

		open = close + 1;
		close = pair[type].find_first_of(' ', open);
		token = pair[type].substr(open, close);
		temp[3] = (float)atof(token.c_str());

		return temp;
	}
};

class camPos {
public:
	glm::vec3 origin;
	float angle;
	camPos(BSPEntity input) {
		if (input.pair["classname"] != "info_player_deathmatch") return; // Not a spawnpoint
		origin = input.getVector("origin");

		angle = (float)glm::radians(atof(input.pair["angle"].c_str()) - 270); // Why 270? is it because of swizzling? is this why culling is broken?
	}
};

class lightPos {
public:
	glm::fvec3 origin;
	glm::fvec4 _color;
	float light; //brightness?
	//target (ex: t6 - this is defined by the "target_position" classname)
	//spawnflags
	//radius
	lightPos(BSPEntity input) {
		if (input.pair["classname"] != "light") return; // Not a light
		origin = input.getVector("origin");
		_color = input.getVector4("_color");
		light = (float)atof(input.pair["light"].c_str());
	}
};

typedef struct
{
    char name[64];
    int	flags;
    int	contents;
} BSPTexture; // Lump 1

typedef struct {
	glm::vec3 normal;
	float distance;
} BSPPlane; // Lump 2

typedef struct {
	int plane;
	int children[2];
	int mins[3];
	int maxs[3];
} BSPNode; // Lump 3

typedef struct {
	int cluster; 	//Visdata cluster index.
	int area; 		//Areaportal area.
	int mins[3]; 	//Integer bounding box min coord.
	int maxs[3]; 	//Integer bounding box max coord.
	int leafface; 	//First leafface for leaf.
	int n_leaffaces; //Number of leaffaces for leaf.
	int leafbrush; 	//First leafbrush for leaf.
	int n_leafbrushes; //Number of leafbrushes for leaf. 
} BSPLeaf; // Lump 4

typedef struct {
	int face;
} BSPLeafFace; // Lump 5

typedef struct {
	int brush;
} BSPLeafBrush; // Lump 6

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

// Lump 11: MeshVerts (standard data type)

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

private:
	std::vector<BSPVertex> vertex;
	std::vector<GLuint> indices;
	std::vector<GLsizei> trianglesPerRow;
	std::vector<size_t> rowIndices;
	GLuint vao;
};

typedef struct {
	std::vector<j7Bezier> bezier;
	GLuint textureID;
} BSPPatch;

typedef struct {
	char data[128][128][3]; // 128x128 pixels, RGB
} BSPLightmap;

typedef struct {
	int n_vecs;
	int sz_vecs;
	std::vector<unsigned char> vecs;
}BSPVisData; // Lump 16

class q3BSP {
public:
	q3BSP(std::string filename);
//private:
	// Raw data from the BSP file
	BSPHeader header;
	std::vector<BSPEntity> entities; // Lump 0
	std::vector<BSPTexture> textures; // Lump 1
	std::vector<BSPPlane> planes; // Lump 2
	std::vector<BSPNode> nodes; // Lump 3
	std::vector<BSPLeaf> leafs; // Lump 4
	std::vector<int> leafFaces; // Lump 5
	// Lump 6
	// Lump 7
	// Lump 8
	// Lump 9
	std::vector<BSPVertex> vertices; // Lump 10
	std::vector<int> meshVerts; // Lump 11
	std::vector<BSPEffect> effects; // Lump 12
	std::vector<BSPFace> faces; // Lump 13
	std::vector<BSPLightmap> lightmaps; // Lump 14
	// Lump 15
	BSPVisData visData; // Lump 16

	// Functions for parsing BSP data and the processed data
	// Lump 0
	void parseEntities(std::string entities); // Take raw string of entities and make a vector of clauses
	std::string worldMusic; // Music for the level ::TODO:: Intro music is not played
	std::vector<camPos> cameraPositions; // Spawnpoints
	std::vector<lightPos> lightPositions; // Lights

	// Lump 1
	std::vector<GLuint> textureIDs;

	//Lump 4
	int findCurrentLeaf(glm::vec3 position); // Finds what leaf the given position is in
	bool isClusterVisible(int visCluster, int testCluster); // Determines if testCluster is visible from visCluster

	//Lump 13
	BSPPatch dopatch(BSPFace face);
	std::vector<int> makeListofVisibleFaces(glm::vec3 position); // Generates a list of faces visible from position
	std::vector<std::vector<BSPFace>> facesByTexture; // All of the faces grouped by texture
	std::vector<BSPPatch> patches; // Contains the tessellated faces

	void bindLightmaps(); // Not working yet
	std::vector<GLuint> lightmapGLIDS;
};

GLuint makeVAO(std::vector<BSPVertex> *vertices, std::vector<GLuint> *indices);

#endif