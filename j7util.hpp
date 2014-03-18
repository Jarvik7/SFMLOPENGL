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
//#include <functional> // For hash
//#include <Windows.h>
//#include "q3bsploader.h"

#include <assimp/Importer.hpp>	//For 3D model loading
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Not all compilers provide a definition for pi
#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

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

void drawGround() {
	static bool loaded=false;
	static GLuint concretetex;
	if(!loaded) {
		sf::Image texture;
		texture.loadFromFile("concrete.jpg");
		glGenTextures(1, &concretetex);
        glBindTexture(GL_TEXTURE_2D, concretetex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.getSize().x, texture.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getPixelsPtr());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		loaded=true;
	}
	glBindTexture(GL_TEXTURE_2D, concretetex);
	glBegin(GL_QUADS);
		glTexCoord2f(1,1);
		glVertex3f(-50,-0.01f, 50);
		glTexCoord2f(1,0);
		glVertex3f( 50,-0.01f, 50);
		glTexCoord2f(0,1);
		glVertex3f( 50,-0.01f,-50);
		glTexCoord2f(0,0);
		glVertex3f(-50,-0.01f,-50);
	glEnd();

}

inline float degtorad(float degrees) //Converts degrees to radians
{
	return float(degrees*M_PI/180);
}

void showFPS(sf::RenderWindow *window)
{
	// This function displays the FPS in the upper left corner of the given RenderWindow.
	// It must be called inside your render loop.

	// ::TODO:: Render using OpenGL? It would avoid needing a pointer argument and a Window
	// could be used instead of a RenderWindow.
	// ::TODO:: Is there a way to eliminate the fontloaded bool?

	static sf::Font font;
	static bool fontloaded=false;
    static bool triedloadingfont=false;

    if (!fontloaded && triedloadingfont) return; // Early exit. We've tried and failed to load a font.
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
		fps=frame;
		frame=0;
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

void adjustPerspective(sf::Vector2u windowsize, GLdouble fovy = 75.0f, GLdouble zNear=0.1f, GLdouble zFar = 100.0f)
{
    //Adjust drawing area & perspective on window resize
    //::TODO:: This currently runs many times for one resize as the window border is dragged. Add throttling?
    //::TODO:: Make into an object?

#if defined(SFML_SYSTEM_WINDOWS) // Windows allows window height of 0
    if (windowsize.y == 0) ++windowsize.y;
#endif
    if (DISPLAYDEBUGOUTPUT)
    {
        std::cout << "Window resized to " << windowsize.x << "x" << windowsize.y << '\n';
    }

    glViewport(0, 0, windowsize.x, windowsize.y);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(fovy, GLdouble(windowsize.x)/windowsize.y, zNear, zFar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
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


class j7Light {
public:


	j7Light(aiVector3D pos, aiVector3D dir, aiColor3D amb, aiColor3D diff, aiColor3D spec) {
		position.push_back(pos.x);
		position.push_back(pos.y);
		position.push_back(pos.z);

		direction.push_back(dir.x);
		direction.push_back(dir.y);
		direction.push_back(dir.z);

		colorAmbient.push_back(amb.r);
		colorAmbient.push_back(amb.g);
		colorAmbient.push_back(amb.b);

		colorDiffuse.push_back(diff.r);
		colorDiffuse.push_back(diff.g);
		colorDiffuse.push_back(diff.b);

		colorSpecular.push_back(spec.r);
		colorSpecular.push_back(spec.g);
		colorSpecular.push_back(spec.b);

		std::cout << "Added new light at (" << position[0] << ',' << position[1] << ',' << position[2] << "), pointing at (" << direction[0] << ',' << direction[1] << ',' << direction[2] << ")\n";
	}

	void draw() {


	// Place it
	//Fixed function OpenGL only supports 8 lights. We're going to need shaders
	/*glLightfv(GL_LIGHT0, GL_AMBIENT, colorAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, colorDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, colorSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, position);*/
	}
private:
	// Coordinates
	std::vector<GLfloat> position;
	std::vector<GLfloat> direction;
	// Colors
	std::vector<GLfloat> colorAmbient;
	std::vector<GLfloat> colorDiffuse;
	std::vector<GLfloat> colorSpecular;
};

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
            for (unsigned i=0; i<mesh->mNumVertices; ++i) {
                normals.push_back(mesh->mNormals[i].x);
                normals.push_back(mesh->mNormals[i].y);
                normals.push_back(mesh->mNormals[i].z);
            }
        }

        if (mesh->HasPositions()) {
            for (unsigned i=0; i<mesh->mNumVertices; ++i) {
                vertices.push_back(mesh->mVertices[i].x);
                vertices.push_back(mesh->mVertices[i].y);
                vertices.push_back(mesh->mVertices[i].z);
            }
        }

        if (mesh->HasFaces()) {
            for (unsigned i=0; i<mesh->mNumFaces; ++i) {
                for (unsigned j=0; j<mesh->mFaces[i].mNumIndices; ++j) {
                    indices.push_back(mesh->mFaces[i].mIndices[j]);
                }
            }
        }

        if (mesh->HasTextureCoords(0)) {
            for (unsigned i=0; i<mesh->mNumVertices; ++i) {
                textureCoordinates.push_back(mesh->mTextureCoords[0][i].x);
                textureCoordinates.push_back(1 - mesh->mTextureCoords[0][i].y);

            }
        }
       else for (unsigned i=0; i<mesh->mNumVertices; ++i) { // ::TODO:: How to handle a mesh with no uv?
            textureCoordinates.push_back(0);
            textureCoordinates.push_back(0);
        }

        if (mesh->HasVertexColors(0)) {
            for (unsigned i=0; i<mesh->mNumVertices; ++i) {
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

private:

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

    void drawVBO() { // Indexed VBO
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        if (bufferObject != 0) {
            glBindBuffer(GL_ARRAY_BUFFER, bufferObject);
            glVertexPointer(3, GL_FLOAT, sizeof(BSPVertex), 0);
            glTexCoordPointer(2, GL_FLOAT, sizeof(BSPVertex), (GLvoid*)(sizeof(GLfloat)*3));
            glNormalPointer(GL_FLOAT, sizeof(BSPVertex), (GLvoid*)(sizeof(GLfloat)*7));
            glColorPointer(4, GL_FLOAT, sizeof(BSPVertex), (GLvoid*)(sizeof(GLfloat)*10));

            for (unsigned i=0; i<meshes.size(); ++i) {
                bindtex(textures[meshes[i].materialIndex]);
                // Indexes
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[i].bufferObjects[INDEX_DATA]);
                glDrawElements(GL_TRIANGLES, (GLsizei)meshes[i].indices.size(), GL_UNSIGNED_INT, 0); // Index 1 of trdis has no texture coords!

            }
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
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    j7Model(std::string filename) {
		bufferObject=0; // We won't use a buffer object for the whole model
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
		if (scene->HasLights()) for (unsigned i=0; i< scene->mNumLights; ++i) {
			lights.push_back(j7Light(scene->mLights[i]->mPosition,
									scene->mLights[i]->mDirection,
									scene->mLights[i]->mColorAmbient,
									scene->mLights[i]->mColorDiffuse,
									scene->mLights[i]->mColorSpecular));
		}
    } // Load from filesystem

	j7Model(q3BSP *bsp) { // Load from a BSP object
        glGenBuffers(1, &bufferObject);
        
		// Buffer the vector of all BSP vertex data
        glBindBuffer(GL_ARRAY_BUFFER, bufferObject);
        glBufferData(GL_ARRAY_BUFFER, sizeof(BSPVertex) * bsp->vertices.size(), bsp->vertices.data(), GL_STATIC_DRAW);
		// Buffer the index data
		for (unsigned i = 0; i < bsp->facesByTexture.size(); ++i) meshes.push_back(j7Mesh(bsp,i));
		importTextures(bsp);
	}

private:
    std::vector<j7Mesh> meshes;
    std::vector<GLuint> textures; // Vector of texture IDs.
	std::vector<j7Light> lights;
	//std::vector<j7Bone> bones; //::TODO::

    // For meshes where the vertex data is shared for everything (BSP)
    GLuint bufferObject;

    void bindtex(GLuint id) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id);
    }
    GLuint loadTexture(std::string filename) {
        if (filename == "") {
            std::cerr << "Texture name is null\n";
            return 0;
        }
        if (fileExists(filename + ".jpg")) filename+= ".jpg";
        else if (fileExists(filename + ".tga")) filename += ".tga";
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
        GLuint id=0;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.getSize().x, texture.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getPixelsPtr());
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return id;

    }

    void importTextures(q3BSP *bsp) {
        std::cout << "Loading textures...\n";
		for (unsigned i=0; i < bsp->textures.size(); ++i) textures.push_back(loadTexture(bsp->textures[i].name));
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

class j7Material {
public:
	std::vector<GLuint> diffuse_tex;


private:

};

class j7Cam {
public:
	j7Cam() {
		//Initialize control settings
		mouseSensitivity=0.01f;
		moveSpeed=0.05f;
		runSpeedMultiplier=2.0f;
		mouseLock=false;
		hasFocus = true;

		//Initialize our camera matrices
		up = sf::Vector3f(0.0f, 1.0f, 0.0f);
		eye = sf::Vector3f(0.0f, 1.0f, 5.0f); // Start 5 units back
		center = sf::Vector3f(0.0f, 1.0f, 0.0f);

		angle = sf::Vector2f(float(M_PI), 0.0f); // ::TODO:: Face the other way
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
			sf::Mouse::setPosition(sf::Vector2i(windowsize.x/2, windowsize.y/2), *window);
		}
		else sf::Mouse::setPosition(savedMousePosition); // Restore mouse position
	}
	void setFocus(bool focus) {
		hasFocus = focus;
	}
private:
	float mouseSensitivity;
	float moveSpeed;
	float runSpeedMultiplier;
	bool hasFocus;

	sf::Vector2i savedMousePosition;

	//View matrices
	sf::Vector3f eye;
	sf::Vector3f center;
	sf::Vector3f up;
	//Mouse angle
	sf::Vector2f angle;
	bool mouseLock;

	void move() {
		//Calculate mouselook
		center.x = eye.x + sin(angle.x)*cos(angle.y);
		center.y = eye.y + sin(angle.y);
		center.z = eye.z + cos(angle.x)*cos(angle.y);

		//Push camera matrix to GL
		glLoadIdentity();
		gluLookAt(eye.x, eye.y, eye.z,
				  center.x, center.y, center.z,
				  up.x, up.y, up.z);
	}

	void updatePosition() {
		if (!hasFocus) return;
		//Get our current MODELVIEW matrix
		GLfloat modelview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, &modelview[0]);
		
		// Straight ::TODO:: Add support for sprinting
		sf::Vector3f back(modelview[2], modelview[6], modelview[10]); // Back vector
		if (sf::Keyboard::isKeyPressed(key_move_run)) back*=runSpeedMultiplier;
		if (sf::Keyboard::isKeyPressed(key_move_forward)) eye-=back*moveSpeed;
		if (sf::Keyboard::isKeyPressed(key_move_backward)) eye+=back*moveSpeed;

		//Strafing
		sf::Vector3f right(modelview[0],modelview[4],modelview[8]); // Right vector
		if (sf::Keyboard::isKeyPressed(key_move_left)) eye-=right*moveSpeed;
		if (sf::Keyboard::isKeyPressed(key_move_right)) eye+=right*moveSpeed;

		//Vertical ::TODO:: Give this support to toggle between fly and jump/crouch
		// ::TODO:: This doesn't work properly when looking up/down
		// ::TODO:: Crouch/jump should use linear interpolation?
		sf::Vector3f up(modelview[1], modelview[5], modelview[9]); // Up vector
		if (sf::Keyboard::isKeyPressed(key_move_up)) eye+=up*moveSpeed;
		if (sf::Keyboard::isKeyPressed(key_move_down)) eye-=up*moveSpeed;

	}
	void updateAngle(sf::RenderWindow *window) {
		if (mouseLock && hasFocus) {
			sf::Vector2u windowsize = window->getSize();
			sf::Vector2i mouseOffset=sf::Mouse::getPosition(*window);

			mouseOffset.x-=(windowsize.x/2);
			mouseOffset.y-=(windowsize.y/2);
			angle.x-=mouseOffset.x*mouseSensitivity;
			angle.y-=mouseOffset.y*mouseSensitivity;
			
			// Cap vertical angle to roughly +/- 90 degrees (roughly 1.57 rads) - FPS cam
			// ::TODO:: Enable this to be toggled to flightsim-style camera control
			if (angle.y>=1.57) angle.y=1.57f;
			else if (angle.y<=-1.57) angle.y=-1.57f;
			
			// Reset cursor to center of screen
			sf::Mouse::setPosition(sf::Vector2i(windowsize.x/2, windowsize.y/2), *window);
		}
	}
};

#endif