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
//#include <functional> // For hash
//#include <Windows.h>

#include <assimp/Importer.hpp>	//For 3D model loading
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

const bool DISPLAYDEBUGOUTPUT = true;

const sf::Keyboard::Key key_move_forward = sf::Keyboard::W;
const sf::Keyboard::Key key_move_left = sf::Keyboard::A;
const sf::Keyboard::Key key_move_backward = sf::Keyboard::S;
const sf::Keyboard::Key key_move_right = sf::Keyboard::D;
const sf::Keyboard::Key key_move_up = sf::Keyboard::Space;
const sf::Keyboard::Key key_move_down = sf::Keyboard::LControl;
const sf::Keyboard::Key key_move_CW = sf::Keyboard::E;
const sf::Keyboard::Key key_move_CCW = sf::Keyboard::Q;

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
		glVertex3f(-50,-1, 50);
		glTexCoord2f(1,0);
		glVertex3f( 50,-1, 50);
		glTexCoord2f(0,1);
		glVertex3f( 50,-1,-50);
		glTexCoord2f(0,0);
		glVertex3f(-50,-1,-50);
	glEnd();

}

float degtorad(float degrees) //Converts degrees to radians
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
    if (windowsize.y == 0) windowsize.y++;
#endif
    if (DISPLAYDEBUGOUTPUT)
    {
        std::cout << "Window resized to " << windowsize.x << "x" << windowsize.y << std::endl;
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

class j7Mesh {
public:
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> textureCoordinates;
    std::vector<aiColor4D> vertexColors; // ::TODO:: This probably should be GLfloats or something
    std::vector<GLuint> indices;
    unsigned materialIndex;
    GLuint bufferObjects[4];

    j7Mesh(aiMesh* mesh) {
        materialIndex = mesh->mMaterialIndex;

        if (mesh->HasNormals()) {
            for (unsigned i=0; i<mesh->mNumVertices; i++) {
                normals.push_back(mesh->mNormals[i].x);
                normals.push_back(mesh->mNormals[i].y);
                normals.push_back(mesh->mNormals[i].z);
            }
        }

        if (mesh->HasPositions()) {
            for (unsigned i=0; i<mesh->mNumVertices; i++) {
                vertices.push_back(mesh->mVertices[i].x);
                vertices.push_back(mesh->mVertices[i].y);
                vertices.push_back(mesh->mVertices[i].z);
            }
        }

        if (mesh->HasFaces()) {
            for (unsigned i=0; i<mesh->mNumFaces; i++) {
                for (unsigned j=0; j<mesh->mFaces[i].mNumIndices; j++) {
                    indices.push_back(mesh->mFaces[i].mIndices[j]);
                }
            }
        }

        if (mesh->HasTextureCoords(0)) {
            for (unsigned i=0; i<mesh->mNumVertices; i++) {
                textureCoordinates.push_back(mesh->mTextureCoords[0][i].x);
                textureCoordinates.push_back(1 - mesh->mTextureCoords[0][i].y);

            }
        }
       else for (unsigned i=0; i<mesh->mNumVertices; i++) { // ::TODO:: How to handle a mesh with no uv?
            textureCoordinates.push_back(0);
            textureCoordinates.push_back(0);
        }

        if (mesh->HasVertexColors(0)) { // ::TODO:: Colors are not rendered yet
            for (unsigned i=0; i<mesh->mNumVertices; i++) {
                vertexColors.push_back(mesh->mColors[0][i]);
            }
        }

        makeVBO(); // ::TODO:: Make VBO generation dependent on passing a bool to constructor

    }
private:

    void makeVBO() {

#define VERTEX_DATA 0
#define NORMAL_DATA 1
#define TEXTURE_DATA 2
#define INDEX_DATA 3
        glGenBuffers(4, bufferObjects);
        // Copy data to video memory
        // Vertex data
        glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[VERTEX_DATA]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
        // Normal data
        glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[NORMAL_DATA]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normals.size(), normals.data(), GL_STATIC_DRAW);
        // Texture coordinates
        glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[TEXTURE_DATA]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * textureCoordinates.size(), textureCoordinates.data(), GL_STATIC_DRAW);
        // Indexes
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObjects[INDEX_DATA]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);
        
    } // ::TODO:: Can we put directly into the buffer from the assimp structure without making arrays?
};

class j7Model { // ::TODO:: Make into a VAO
public:
  //  std::vector<GLuint> vao;

    void drawArray() { // Indexed Vertex Array
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        for (unsigned i=0; i<meshes.size(); i++) {
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

        for (unsigned i=0; i<meshes.size(); i++) {
            bindtex(textures[meshes[i].materialIndex-1]);
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
        }
		glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind, fixes SFML Text display
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    j7Model(std::string filename) {
		Assimp::Importer importer;
		const aiScene *scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Quality);
		if (!scene) {
			std::cerr << "Failed to load mesh \"" << filename << "\"" << std::endl;
			return; // Couldn't load mesh, abort
		}

        if (scene->HasMeshes()) for (unsigned i=0; i < scene->mNumMeshes; i++) {
            meshes.push_back(j7Mesh(scene->mMeshes[i]));
        }
		if (scene->HasMaterials()) importTextures(scene); // ::TODO:: Only supports external diffuse textures right now
    }

private:
    std::vector<j7Mesh> meshes;
    std::vector<GLuint> textures; // Vector of texture IDs. ::TODO:: Make texture 0 all-white to handle meshes with no texture? Also means we don't need the "-1" kludge for materialindex->textureid

    void bindtex(GLuint id) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    void importTextures(const aiScene *scene) {
		for (unsigned i=0; i < scene->mNumMaterials; i++)
		{
			//Get number of textures and create a map entry for each filename
			aiString path;	// filename
		//	textures.push_back(0); // Default empty texture? TEST
			for (unsigned j=0; j< scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE);j++) { // should only be one
				scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, j, &path);
                sf::Image texture;
                texture.loadFromFile(path.data);

                GLuint id;
                glGenTextures(1, &id);
                glBindTexture(GL_TEXTURE_2D, id);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.getSize().x, texture.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getPixelsPtr());
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                textures.push_back(id);
			}
		}
    }
};

class j7Cam {
public:
	j7Cam() {
		//Initialize control settings
		mouseSensitivity=0.01f;
		moveSpeed=0.1f;
		mouseLock=false;

		//Initialize our camera matrices
		up = sf::Vector3f(0.0f, 1.0f, 0.0f);
		eye = sf::Vector3f(0, 1.0f, 5.0f); // Start 5 units back
		center = sf::Vector3f(0, 1.0f, 0);

		angle = sf::Vector2f(0, 0);
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
			sf::Vector2u windowsize = window->getSize();
			sf::Mouse::setPosition(sf::Vector2i(windowsize.x/2, windowsize.y/2), *window);
		}
	}
private:
	float mouseSensitivity;
	float moveSpeed;

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
		//Get our current MODELVIEW matrix
		GLdouble modelview[16];
		glGetDoublev(GL_MODELVIEW_MATRIX, &modelview[0]);
		
		// Straight ::TODO:: Add support for sprinting
		sf::Vector3f back(modelview[2], modelview[6], modelview[10]); // Back vector
		if (sf::Keyboard::isKeyPressed(key_move_forward)) eye-=back*moveSpeed;
		if (sf::Keyboard::isKeyPressed(key_move_backward)) eye+=back*moveSpeed;

		//Strafing
		sf::Vector3f right(modelview[0],modelview[4],modelview[8]); // Right vector
		if (sf::Keyboard::isKeyPressed(key_move_left)) eye-=right*moveSpeed;
		if (sf::Keyboard::isKeyPressed(key_move_right)) eye+=right*moveSpeed;

		//Vertical ::TODO:: Give this support to toggle between fly and jump/crouch
		sf::Vector3f up(modelview[1], modelview[5], modelview[9]); // Up vector
		if (sf::Keyboard::isKeyPressed(key_move_up)) eye+=up*moveSpeed;
		if (sf::Keyboard::isKeyPressed(key_move_down)) eye-=up*moveSpeed;

	}
	void updateAngle(sf::RenderWindow *window) {
		//::TODO:: Add toggle to enable/disable the camera from flipping upside down (FPS cam vs flightsim cam)
		if (mouseLock) {
			sf::Vector2u windowsize = window->getSize();
			sf::Vector2i mouseOffset=sf::Mouse::getPosition(*window);

			mouseOffset.x-=(windowsize.x/2);
			mouseOffset.y-=(windowsize.y/2);
			angle.x-=mouseOffset.x*mouseSensitivity;
			angle.y-=mouseOffset.y*mouseSensitivity;
			sf::Mouse::setPosition(sf::Vector2i(windowsize.x/2, windowsize.y/2), *window);
		}
	}
};

#endif