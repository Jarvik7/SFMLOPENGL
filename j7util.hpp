//
//  j7util.hpp
//  SFML OGL
//
//  Created by Jarvik7 on 2/17/14.
//  Copyright (c) 2014 Jarvik7. All rights reserved.
//

#ifndef SFML_OGL_j7util_hpp
#define SFML_OGL_j7util_hpp

#include <vector>
#include <iostream>
#include <fstream>

#include <SFML/OpenGL.hpp>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <assimp/Importer.hpp>	//For 3D model loading
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//#include <functional> // For hash
//#include <Windows.h>
#include "q3bsploader.h"

// Not all compilers provide a definition for pi
#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

extern std::stack<glm::fmat4> modelviewMatrix;
extern std::stack<glm::fmat4> projectionMatrix;
extern GLuint shaderID;

// Handles for VBO
enum BUFFERTYPES {
	VERTEX_DATA=0,
	NORMAL_DATA,
	TEXTURE_DATA,
	INDEX_DATA,
    COLOR_DATA
};

const bool DISPLAYDEBUGOUTPUT = true;

//Camera control/movement keys
const sf::Keyboard::Key key_move_forward = sf::Keyboard::W;
const sf::Keyboard::Key key_move_left = sf::Keyboard::A;
const sf::Keyboard::Key key_move_backward = sf::Keyboard::S;
const sf::Keyboard::Key key_move_right = sf::Keyboard::D;
const sf::Keyboard::Key key_move_up = sf::Keyboard::Space;
const sf::Keyboard::Key key_move_down = sf::Keyboard::LControl;
const sf::Keyboard::Key key_move_CW = sf::Keyboard::E;
const sf::Keyboard::Key key_move_CCW = sf::Keyboard::Q;
const sf::Keyboard::Key key_move_run = sf::Keyboard::LShift;

inline bool fileExists(std::string filename) {
    std::ifstream infile(filename);
    return infile.good();
}

inline float degtorad(float degrees) //Converts degrees to radians
{
	return float(degrees*M_PI/180.0f);
}

void getVarSizes() {
    std::cout << "Float: " << sizeof(float) << '\n';
    std::cout << "GLfloat: " << sizeof(GLfloat) << '\n';
    std::cout << "GLuint: " << sizeof(GLuint) << '\n';
    std::cout << "Int: " << sizeof(int) << '\n';
    std::cout << "Unsigned: " << sizeof(unsigned) << '\n';
    std::cout << "Char: " << sizeof(char) << '\n';
    std::cout << "short: " << sizeof(short) << '\n';
    std::cout << "GLubyte: " << sizeof(GLubyte) << '\n';
    std::cout << "GLshort: " << sizeof(GLshort) << '\n';
    std::cout << "Size_t: " << sizeof(size_t) << '\n';


}

void showFPS(sf::RenderWindow *window)
{
	// This function displays the FPS in the upper left corner of the given RenderWindow.
	// It must be called inside the render loop.

	// ::TODO:: Render without SFML? It would avoid needing a pointer argument and a Window
	// could be used instead of a RenderWindow.
	// ::TODO:: Is there a way to eliminate the fontloaded bool?

	static sf::Font font;
	static bool fontloaded=false;
    static bool triedloadingfont=false;

    if (!fontloaded && triedloadingfont) return; // Early exit. We've tried and failed to load a font already.
	//Load the font
	if (!fontloaded) {
        triedloadingfont=true;
#if defined(SFML_SYSTEM_WINDOWS) 
        if (font.loadFromFile("c:\\Windows\\Fonts\\Arial.ttf")) fontloaded=true;
#elif defined(SFML_SYSTEM_MACOS)
        if (font.loadFromFile("/Library/Fonts/Arial.ttf")) fontloaded=true;
#endif
        if (!fontloaded) return; // Early exit. Failed to load a font.
	}

    static sf::Clock timer; // Times 1 second
	static int frame=0;	// Holds number of frames this second
	static int fps=0; // Holds fps

	//Calculate FPS
	frame++;
	if(timer.getElapsedTime().asSeconds() >= 1) // If 1 second has passed, tally frames and reset timer
	{
		fps = frame;
		frame = 0;
		timer.restart();
	}

	//Draw on screen, skip first second as it doesn't have a sample to calculate from yet
	if(fps > 0)
	{
		window->pushGLStates();
#if defined(SFML_SYSTEM_WINDOWS) // MSVC2010 is not fully C++11 compliant
		window->draw(sf::Text(std::to_string((long long)fps)+ " fps", font, 20));
#elif defined(SFML_SYSTEM_MACOS)
        window->draw(sf::Text(std::to_string(fps) + " fps", font, 20));
#endif
		window->popGLStates();
	}
}

void adjustPerspective(sf::Vector2u windowsize, GLfloat fovy = 75.0f, GLfloat zNear = 0.1f, GLfloat zFar = 100.0f)
{
    //Adjust drawing area & perspective on window resize
    //::TODO:: This currently runs many times for one resize as the window border is dragged. Add throttling?
    //::TODO:: Make into an object?

#if defined(SFML_SYSTEM_WINDOWS) // Windows allows window height of 0, prevent div/0
    if (windowsize.y == 0) ++windowsize.y;
#endif
    if (DISPLAYDEBUGOUTPUT)
    {
        std::cout << "Window resized to " << windowsize.x << "x" << windowsize.y << '\n';
    }

    glViewport(0, 0, windowsize.x, windowsize.y);

    glMatrixMode(GL_PROJECTION);
	projectionMatrix.pop();
	projectionMatrix.push(glm::perspective<float>(glm::radians(fovy), GLfloat(windowsize.x) / windowsize.y, zNear, zFar));

    glMatrixMode(GL_MODELVIEW);
	// Don't need to change anything?
}
/*unsigned j7MenuIdentifier(std::string name)
{
		std::hash<std::string> hashfunc;
		return hashfunc(name)%10000; // ::TODO: This is a bodge, IDs should be handed out in order and stored in a map
}
class j7Menu
{
public:
	std::vector<j7Menu> child;
	std::string name;
	unsigned identifier;
	bool checked;
	j7Menu(std::string menuname="", bool checkmark=false)
	{
		name = menuname;
		identifier = j7MenuIdentifier(name);
		checked=checkmark;
		std::cout << "Added menu named " << menuname << " with ID of " << identifier << std::endl;
	}
	void addchild(std::string childmenu, bool check=false)
	{
		child.push_back(j7Menu(childmenu, check));
	}
	void addchild(j7Menu childmenu) {
		child.push_back(childmenu);
	}

	void draw(sf::RenderWindow *window) {
		auto menu = CreateMenu();
		auto submenu = CreatePopupMenu();
		if (name == "root") { // Go through top menu items
			for (int i = 0; i<child.size(); i++) {
				drawSubitem(&child[i], &submenu); //add topitem's subitems
				AppendMenu(menu, MF_STRING | MF_POPUP, (UINT)submenu, child[i].name.c_str()); // Add the topitem and its subitems to menu structure
				submenu = CreatePopupMenu(); // Reset the subitem structure
			}
			SetMenu(window->getSystemHandle(),menu); // Draw the menu
		}
	}

	void drawSubitem(j7Menu *node, HMENU *submenu)
	{
		for (int i = 0; i<node->child.size(); i++) { //Go through children
			if (node->child[i].child.size()!=0) { // If there are subchildren
				auto subsubmenu = CreatePopupMenu();
				drawSubitem(&node->child[i], &subsubmenu); // Recurse if there are subchildren
				AppendMenu(*submenu, MF_STRING | MF_POPUP, (UINT)subsubmenu, node->child[i].name.c_str());
			}
			else {
				AppendMenu(*submenu, MF_STRING, UINT(node->child[i].identifier), node->child[i].name.c_str()); // Otherwise, add to the structure
				std::cout << "Added menu item: " << node->child[i].name << ", with id: " << node->child[i].identifier << std::endl;
			}
		}
	}
};
void generateMenu(sf::RenderWindow *window)
{
	j7Menu rootmenu("root");

	//File
	j7Menu filemenu("File");
	filemenu.addchild("Open model");
	filemenu.addchild("Open music");
	filemenu.addchild("Exit");


	j7Menu test("Hello");
	test.addchild("Joe");
	filemenu.addchild(test);

	rootmenu.addchild(filemenu);

	//Render Options
	j7Menu rendermenu("Render Options");
	rendermenu.addchild("Wireframe");
	rendermenu.addchild("Texturing",true);
	rendermenu.addchild("Backface culling",true);
	rendermenu.addchild("Rotation",true);
	rootmenu.addchild(rendermenu);
	rootmenu.draw(window);
}*/

class j7Mesh {
public:
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> textureCoordinates;
    std::vector<GLfloat> vertexColors;
    std::vector<GLuint> indices;
    unsigned materialIndex;
    GLuint bufferObjects[5];

    j7Mesh(aiMesh* mesh) {
        materialIndex = mesh->mMaterialIndex;

        if (mesh->HasNormals()) {
            for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
                normals.push_back(mesh->mNormals[i].x);
                normals.push_back(mesh->mNormals[i].y);
                normals.push_back(mesh->mNormals[i].z);
            }
        }

        if (mesh->HasPositions()) {
            for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
                vertices.push_back(mesh->mVertices[i].x);
                vertices.push_back(mesh->mVertices[i].y);
                vertices.push_back(mesh->mVertices[i].z);
            }
        }

        if (mesh->HasFaces()) {
            for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
                for (unsigned j = 0; j < mesh->mFaces[i].mNumIndices; ++j) {
                    indices.push_back(mesh->mFaces[i].mIndices[j]);
                }
            }
        }

        if (mesh->HasTextureCoords(0)) {
            for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
                textureCoordinates.push_back(mesh->mTextureCoords[0][i].x);
                textureCoordinates.push_back(1 - mesh->mTextureCoords[0][i].y);

            }
        }
       else for (unsigned i = 0; i < mesh->mNumVertices; ++i) { // ::TODO:: How to handle a mesh with no uv?
            textureCoordinates.push_back(0);
            textureCoordinates.push_back(0);
        }

        if (mesh->HasVertexColors(0)) {
            for (unsigned i=0; i < mesh->mNumVertices; ++i) {
                vertexColors.push_back(mesh->mColors[0][i].r);
                vertexColors.push_back(mesh->mColors[0][i].g);
                vertexColors.push_back(mesh->mColors[0][i].b);
                vertexColors.push_back(mesh->mColors[0][i].a);
            }
        }

        makeVBO(); // ::TODO:: Make VBO generation dependent on passing a bool to constructor

    }

	j7Mesh(q3BSP *bsp, unsigned faceSetIndex) { // ::TODO:: Move vertex info into j7Model. Mesh should just have indices and texture id

        materialIndex = 0;
        if (bsp->facesByTexture[faceSetIndex].size() != 0) materialIndex = bsp->facesByTexture[faceSetIndex][0].texture;
        std::vector<GLuint> temp = bsp->getIndices(faceSetIndex);
        for (unsigned i = 0; i < temp.size(); ++i) { //For each index in this set
            indices.push_back(temp[i]);
        }

        makeVBO(false); // Don't need objects for anything other than indices
	}

//private
    void makeVBO(bool generateVertexBuffers=true) {
        glGenBuffers(5, bufferObjects);
        // Copy data to video memory
        if (generateVertexBuffers) {
            // Vertex data
            glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[VERTEX_DATA]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
            // Normal data
            glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[NORMAL_DATA]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normals.size(), normals.data(), GL_STATIC_DRAW);
            // Texture coordinates
            glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[TEXTURE_DATA]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * textureCoordinates.size(), textureCoordinates.data(), GL_STATIC_DRAW);
            // Colors
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObjects[COLOR_DATA]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * vertexColors.size(), vertexColors.data(), GL_STATIC_DRAW);
        }
        // Indexes

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObjects[INDEX_DATA]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);

    } // ::TODO:: Can we put directly into the buffer from the assimp structure without making arrays?
};

class j7Model { // ::TODO:: Make into a VAO
public:
    void drawArray() { // Indexed Vertex Array
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        for (unsigned i=0; i<meshes.size(); ++i) {
            bindtex(textures[meshes[i].materialIndex-1]);
            glVertexPointer(3, GL_FLOAT, 0, meshes[i].vertices.data());
            glTexCoordPointer(2, GL_FLOAT, 0, meshes[i].textureCoordinates.data());
            glNormalPointer(GL_FLOAT, 0, meshes[i].normals.data());
            glDrawElements(GL_TRIANGLES, (GLsizei)meshes[i].indices.size(), GL_UNSIGNED_INT, meshes[i].indices.data());
        }
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    void drawVBO(q3BSP *bsp) { // Indexed VBO
		//::TODO:: The lighting looks as if it is using a face normal instead of a vertex normal - is screwed up

        if (vao != 0) {
			glBindVertexArray(vao);
            for (unsigned i=0; i < meshes.size(); ++i) {
                bindtex(textures[meshes[i].materialIndex]);
                // Indexes
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[i].bufferObjects[INDEX_DATA]);
                glDrawElements(GL_TRIANGLES, (GLsizei)meshes[i].indices.size(), GL_UNSIGNED_INT, 0); // Index 1 of trdis has no texture coords!
            }
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			for (unsigned i = 0; i < bsp->patches.size(); ++i) { // For every patch

				bindtex(textures[bsp->patches[i].textureID]);
				for (unsigned j = 0; j< bsp->patches[i].bezier.size(); ++j) { // For every bezier in every patch
					bsp->patches[i].bezier[j].render();
				}
			}
			glBindVertexArray(0);
        }

        else for (unsigned i=0; i<meshes.size(); ++i) {
            bindtex(textures[meshes[i].materialIndex]);
            glBindBuffer(GL_ARRAY_BUFFER, meshes[i].bufferObjects[VERTEX_DATA]);
            glVertexPointer(3, GL_FLOAT, 0, 0);
            // Normal data
            glBindBuffer(GL_ARRAY_BUFFER, meshes[i].bufferObjects[NORMAL_DATA]);
            glNormalPointer(GL_FLOAT, 0, 0);
            // Texture coordinates
            glBindBuffer(GL_ARRAY_BUFFER, meshes[i].bufferObjects[TEXTURE_DATA]);
            glTexCoordPointer(2, GL_FLOAT, 0, 0);
            // Indexes
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[i].bufferObjects[INDEX_DATA]);
            glDrawElements(GL_TRIANGLES, (GLsizei)meshes[i].indices.size(), GL_UNSIGNED_INT, 0); // Index 1 of trdis has no texture coords!
            // Vertex colors
            glBindBuffer(GL_ARRAY_BUFFER, meshes[i].bufferObjects[COLOR_DATA]);
            glColorPointer(4, GL_FLOAT, 0, 0);
        }
		glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind, fixes SFML Text display
    }

    j7Model(std::string filename) {
		vao=0; // We won't use a buffer object for the whole model
		Assimp::Importer importer;
		std::cout << "\nTrying to load mesh file: " << filename << '\n';
		const aiScene *scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Quality);
		if (!scene) {
			std::cerr << "Failed to load mesh \"" << filename << "\"\n";
			return; // Couldn't load mesh, abort
		}

        if (scene->HasMeshes()) for (unsigned i=0; i < scene->mNumMeshes; ++i) {
            meshes.push_back(j7Mesh(scene->mMeshes[i]));
        }
		if (scene->HasMaterials()) importTextures(scene); // ::TODO:: Only supports external diffuse textures right now
		/*if (scene->HasLights()) for (unsigned i=0; i< scene->mNumLights; ++i) {
			lights.push_back(j7Light(scene->mLights[i]->mPosition,
									scene->mLights[i]->mDirection,
									scene->mLights[i]->mColorAmbient,
									scene->mLights[i]->mColorDiffuse,
									scene->mLights[i]->mColorSpecular));
		}*/
    } // Load from filesystem

	j7Model(q3BSP *bsp) { // Load from a BSP object
		GLuint bufferID;
        glGenBuffers(1, &bufferID);
        vao = makeVAO(&bsp->vertices, 0);

		// Buffer the index data
		for (unsigned i = 0; i < bsp->facesByTexture.size(); ++i) meshes.push_back(j7Mesh(bsp,i));

		// Load the textures
		importTextures(bsp);
	}

private:
    std::vector<j7Mesh> meshes;
    std::vector<GLuint> textures; // Vector of texture IDs.
    //std::vector<j7Light> lights;
	//std::vector<j7Bone> bones; //::TODO::

    // For meshes where the vertex data is shared for everything (BSP)
	GLuint vao;

    void bindtex(GLuint id) {
      //  glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id);
    }
    GLuint loadTexture(std::string filename) {
        if (filename == "") {
            std::cerr << "Texture name is null\n";
            return 0;
        }
        if (fileExists(filename + ".jpg")) filename += ".jpg";
        else if (fileExists(filename + ".tga")) filename += ".tga";
        else if (fileExists(filename + ".png")) filename += ".png"; // QuakeLive uses some PNG textures
        else {
            std::cerr << "Unable to find texture file: " << filename << '\n';
            return 0;
        }

        sf::Image texture;

       // if (path.data[0] != '*') { // Texture is a file
        if (!texture.loadFromFile(filename)) {
            std::cerr << "Error loading texture file: " << filename << '\n';
            return 0;
        }

        //}
        /*
        else { // Texture is internal
            std::cout << "Loading internal texture\n";
            unsigned id = atoi(&path.data[1]); // Get texture index from "path"
            unsigned size;

            if (scene->mTextures[id]->mHeight == 0) {
                size = scene->mTextures[id]->mWidth; // Compressed texture
                texture.loadFromMemory(reinterpret_cast<unsigned char*>(scene->mTextures[id]->pcData),size);
            }

            else {
                std::cout << "Texture is NOT compressed (" << scene->mTextures[id]->mWidth << 'x' << scene->mTextures[id]->mHeight << "). Size: ";
                size=scene->mTextures[id]->mWidth * scene->mTextures[id]->mHeight; // Number of texels
                std::vector<unsigned char> texels;
                for (int i=0; i< size; ++i) { // Convert aiTexel's ARGB8888 to SFML's RGBA8888
                    aiTexel temp = scene->mTextures[id]->pcData[i]; //Get the texel
                    rawTexture.push_back(temp.r);
                    rawTexture.push_back(temp.g);
                    rawTexture.push_back(temp.b);
                    rawTexture.push_back(temp.a);
                }
            }
        }
         */
        GLuint id = 0;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
		GLfloat largest_aniso;
		// Enable anisotropic filtering
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_aniso);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_aniso);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.getSize().x, texture.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getPixelsPtr());
		// Enable mipmapping
		glGenerateMipmap(GL_TEXTURE_2D); // GL3.0
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return id;

    }

    void importTextures(q3BSP *bsp) {
        std::cout << "Loading textures...\n";
		for (unsigned i = 0; i < bsp->textures.size(); ++i) textures.push_back(loadTexture(bsp->textures[i].name));
    }

    void importTextures(const aiScene *scene) {
		std::vector<unsigned char> rawTexture;
		for (unsigned i=0; i < scene->mNumMaterials; ++i)
		{
			aiString path;	// filename
			if (scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
				// Get path for diffuse texture
				scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
			}
			textures.push_back(loadTexture(path.data));
		}
    }
};

class j7Cam {
public:
	j7Cam() {
		//Initialize control settings
		mouseSensitivity = 0.01f;
		moveSpeed = 0.05f;
		runSpeedMultiplier = 2.0f;
		mouseLock = false;
		hasFocus = true;
		angle = glm::fvec2(0,0);
		//angle = glm::fvec2(float(M_PI), 0.0f); // ::TODO:: Face the other way
		up = glm::fvec3(0.0f, 1.0f, 0.0f);
	}

	void update(sf::RenderWindow *window) {
		updatePosition(); // Movement
		updateAngle(window); // Look
		move(); // Apply change
	}

	void setMouseLock(bool locked, sf::RenderWindow *window) { // Toggle mouse locking
		mouseLock=locked;
		window->setMouseCursorVisible(!mouseLock);
		if (mouseLock) {
			savedMousePosition = sf::Mouse::getPosition(); // Save mouse position
			sf::Vector2u windowsize = window->getSize();
			sf::Mouse::setPosition(sf::Vector2i(windowsize.x / 2, windowsize.y /2 ), *window);
		}
		else sf::Mouse::setPosition(savedMousePosition); // Restore mouse position
	}

	void setFocus(bool focus) {
		hasFocus = focus;
	}

	void goTo(glm::fvec3 origin, float viewangle) {
		eye.x = origin.x;
		eye.y = origin.z+.05f; // Offset for player eye height
		eye.z = -origin.y;
		angle.x = viewangle - 1;
		angle.y = 0;
		move();
		std::cout << "Teleporting: ";
		printPos();

	}

	void printPos() {
		glm::mat4 view = glm::inverse(modelviewMatrix.top());
		glm::vec4 pos = view[3];
		std::cout << "Pos: " << pos.x << ',' << pos.y << ',' << pos.z << " Facing: " << angle.x << ".\n";
	}

private:
	float mouseSensitivity;
	float moveSpeed;
	float runSpeedMultiplier;
	bool hasFocus;

	sf::Vector2i savedMousePosition;

	//View matrices
	glm::fvec3 eye;
	glm::fvec3 center;
	//static const glm::fvec3 up = glm::fvec3(0.0f, 1.0f, 0.0f); // This should always be absolute up
	glm::fvec3 up;
	//Mouse angle
	glm::fvec2 angle;
	bool mouseLock;

	void move() {
		//Calculate mouselook
		center.x = eye.x + sin(angle.x) * cos(angle.y);
		center.y = eye.y + sin(angle.y);
		center.z = eye.z + cos(angle.x) * cos(angle.y);

		//Push camera matrix to GL
        modelviewMatrix.pop();
        modelviewMatrix.push(glm::lookAt(eye, center, up));
	}

	void updatePosition() {
		if (!hasFocus) return;

		// Straight
		glm::fvec3 back(modelviewMatrix.top()[0][2], modelviewMatrix.top()[1][2], modelviewMatrix.top()[2][2]); // Back vector
		if (sf::Keyboard::isKeyPressed(key_move_run)) back *= runSpeedMultiplier;
		if (sf::Keyboard::isKeyPressed(key_move_forward)) eye -= back * moveSpeed;
		if (sf::Keyboard::isKeyPressed(key_move_backward)) eye += back * moveSpeed;

		//Strafing
		glm::fvec3 right(modelviewMatrix.top()[0][0],modelviewMatrix.top()[1][0],modelviewMatrix.top()[2][0]); // Right vector
		if (sf::Keyboard::isKeyPressed(key_move_left)) eye -= right * moveSpeed;
		if (sf::Keyboard::isKeyPressed(key_move_right)) eye += right * moveSpeed;

		//Vertical ::TODO:: Give this support to toggle between fly and jump/crouch (use linear interpolation?)
		if (sf::Keyboard::isKeyPressed(key_move_up)) eye += up * moveSpeed;
		if (sf::Keyboard::isKeyPressed(key_move_down)) eye -= up * moveSpeed;
	}

	void updateAngle(sf::RenderWindow *window) {
		if (mouseLock && hasFocus) {
			sf::Vector2u windowsize = window->getSize();
			sf::Vector2i mouseOffset = sf::Mouse::getPosition(*window);

			mouseOffset.x -= (windowsize.x / 2);
			mouseOffset.y -= (windowsize.y / 2);
			angle.x -= mouseOffset.x * mouseSensitivity;
			angle.y -= mouseOffset.y * mouseSensitivity;
			
			// Cap vertical angle to roughly +/- 90 degrees (roughly 1.57 rads) - FPS cam
			// ::TODO:: Enable this to be toggled to flightsim-style camera control
			if (angle.y >= 1.57) angle.y = 1.57f;
			else if (angle.y <= -1.57) angle.y = -1.57f;
			float fullcircle = glm::radians(360.0);
			// Keep x angle between -360 to 360 degrees
			if (angle.x >= fullcircle) angle.x -= fullcircle;
			else if (angle.x <= -fullcircle) angle.x += fullcircle;
			// Reset cursor to center of screen
			sf::Mouse::setPosition(sf::Vector2i(windowsize.x / 2, windowsize.y / 2), *window);
		}
	}
};

GLenum loadShader(std::string filename, GLenum type) {
    // Check arguments
    if (!fileExists(filename)) {
        std::cerr << "Unable to open file: " << filename << '\n';
        return 0;
    }
    if (type != GL_VERTEX_SHADER && type != GL_FRAGMENT_SHADER) {
        std::cerr << "Invalid enum passed to loadShader().\n";
        return 0;
    }

    // Read in shader source
    std::ifstream file(filename);
	std::string shaderSource;
    shaderSource.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    const GLchar* shaderStringPointer = shaderSource.c_str();

    // Load it into OpenGL
    GLenum shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&shaderStringPointer, NULL);

    glCompileShader(shader);
    // Compile and check
    GLint shaderCompiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);
    if (shaderCompiled == false)
    {
        std::cerr << "ERROR: Vertex shader not compiled properly.\n";
		std::cerr << "Shader type: " << type << ".\n";

        GLint blen = 0;
        GLsizei slen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH , &blen);
        if (blen > 1) {
            GLchar* compiler_log = new GLchar[blen];
            glGetInfoLogARB(shader, blen, &slen, compiler_log);
            fprintf(stderr, "compiler log:\n %s", compiler_log);
            delete [] compiler_log;
        }
    }
    if (shaderCompiled != GL_TRUE) {
        std::cout << "!!! " << shaderCompiled << " !!!\n";
      //  printf( "Unable to compile shader %d!\n\nSource:\n%s\n", shader, shaderStringPointer );
        glDeleteShader(shader);
        shader = 0;
    }
    return shader;
}

#endif