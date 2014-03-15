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

typedef struct
{
    float position[3];	//vertex position.
    float texcoord[2][2];	//vertex texture coordinates. 0=surface, 1=lightmap.
    float normal[3];	//vertex normal.
    unsigned char color[4];	//vertex color, in RGBA, as unsigned.
} BSPVertex; // Lump 10

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

class q3BSP {
public:
	q3BSP(std::string filename);
	std::vector<GLfloat> getVertices();
	std::vector<GLfloat> getNormals();
    std::vector<GLfloat> getVertexColors();
	std::vector<std::vector<GLuint>> getIndices();
	std::vector<GLfloat> getTextureCoordinates();
    std::vector<std::vector<BSPFace>> facesByTexture;
	std::vector<std::string> getTextureNames(); // Actual loading to be done by j7Model

private:
	BSPHeader header;
	BSPEntities entities; // This needs a parser
	std::vector<BSPTexture> textures;
	std::vector<BSPVertex> vertices;
	std::vector<BSPMeshVert> meshVerts;
	std::vector<BSPFace> faces;
    void groupMeshByTexture();
};