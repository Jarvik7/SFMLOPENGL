#include <array>

typedef struct {
	int offset;
	int length;
} dirEntry;

typedef struct {
	char magicNumber[4];
	int version;
	dirEntry direntries[17];
} BSPHeader;

typedef struct
{
    std::string entities;
} BSPEntities; // Lump 0

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
    std::array<float, 3> position;	//vertex position.
    std::array<std::array<float, 2>, 2> texcoord;	//vertex texture coordinates. 0=surface, 1=lightmap.
    std::array<float, 3> normal;	//vertex normal.
    std::array<unsigned char, 4> color;	//vertex color, in RGBA, as unsigned.

	BSPVertex operator+(BSPVertex a) {
		BSPVertex temp;

		for (int i=0; i<3; i++) {
			temp.position[i] = this->position[i] + a.position[i];
			temp.normal[i] = this->normal[i] + a.normal[i];
		}
		for(int i = 0; i < 2; ++i) for(int j = 0; j < 2; ++j) temp.texcoord[i][j] = this->texcoord[i][j] + a.texcoord[i][j];
		return temp;
	}

	BSPVertex operator*(float a) {
		BSPVertex temp;
		for (int i = 0; i < 3; i++) {
			temp.position[i] = this->position[i] * a;
			temp.normal[i] = this->normal[i] * a;
		}
		for (int i = 0; i < 2; ++i) for (int j = 0; j< 2; ++j)  temp.texcoord[i][j] = this->texcoord[i][j] * a;
			return temp;
	}


};

typedef struct
{
    int	offset;	//Vertex index offset, relative to first vertex of corresponding face.
} BSPMeshVert; // Lump 11

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
	//j7Bezier();
//private:
	GLuint bufferID;
	int level;
	std::vector<BSPVertex> vertex;
	std::vector<GLuint> indices;
	std::vector<GLsizei> trianglesPerRow;
	std::vector<GLuint> rowIndices;
};
typedef struct {
	std::vector<j7Bezier> bezier;
} BSPPatch;

class q3BSP {
public:
	q3BSP(std::string filename);
	std::vector<GLuint> getIndices(unsigned entry);
    std::vector<std::vector<BSPFace>> facesByTexture;

	std::vector<BSPVertex> vertices;
	std::vector<BSPTexture> textures;
	std::vector<BSPPatch> patches;


private:
	BSPHeader header;
	BSPEntities entities; // This needs a parser
	

	std::vector<BSPMeshVert> meshVerts;
	std::vector<BSPFace> faces;
    void groupMeshByTexture();
	void parseEntities(std::string entities);
	BSPPatch dopatch(BSPFace face);
};