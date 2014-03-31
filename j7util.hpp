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
#include <stack>

#include <GLEW/glew.h>	// For OpenGL Extensions
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "q3bsploader.h"

// Not all compilers provide a definition for pi
#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

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
GLuint loadTexture(std::string filename) {
	//Sanity checking
	if (filename == "") {
		std::cerr << "Texture name is null\n";
		return 0;
	}
	if (fileExists(filename)) filename = filename;
	// Try some common filetypes (Quake3 etc don't specify extension in BSP)
	else if (fileExists(filename + ".jpg")) filename += ".jpg";
	else if (fileExists(filename + ".tga")) filename += ".tga";
	else if (fileExists(filename + ".png")) filename += ".png";
	else {
		std::cerr << "Unable to find texture file: " << filename << '\n';
		return 0;
	}

	sf::Image texture;

	if (!texture.loadFromFile(filename)) {
		std::cerr << "Error loading texture file: " << filename << '\n';
		return 0;
	}
	GLuint id = 0;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	GLfloat largest_aniso;

	// Enable anisotropic filtering
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_aniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_aniso);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.getSize().x, texture.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getPixelsPtr());
	
	// Enable mipmapping
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return id;
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
    std::vector<GLuint> indices;
    unsigned materialIndex;
    GLuint bufferID;

	j7Mesh(q3BSP *bsp, unsigned faceSetIndex) {

        materialIndex = 0;
        if (bsp->facesByTexture[faceSetIndex].size() != 0) materialIndex = bsp->facesByTexture[faceSetIndex][0].texture;
        std::vector<GLuint> temp = bsp->getIndices(faceSetIndex);
        for (auto& index : temp) { //For each index in this set
            indices.push_back(index);
        }
		glGenBuffers(1, &bufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* indices.size(), indices.data(), GL_STATIC_DRAW);
	}
};

class j7Model {
public:
    void drawVBO(q3BSP *bsp) {
        if (vao != 0) {
			glBindVertexArray(vao);
            for (unsigned i=0; i < meshes.size(); ++i) {
				glBindTexture(GL_TEXTURE_2D, bsp->textureIDs[meshes[i].materialIndex]);
                // Indexes
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[i].bufferID);
                glDrawElements(GL_TRIANGLES, (GLsizei)meshes[i].indices.size(), GL_UNSIGNED_INT, 0); // Index 1 of trdis has no texture coords!
            }
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			for (unsigned i = 0; i < bsp->patches.size(); ++i) { // For every patch
				glBindTexture(GL_TEXTURE_2D, bsp->textureIDs[bsp->patches[i].textureID]);
				for (unsigned j = 0; j< bsp->patches[i].bezier.size(); ++j) { // For every bezier in every patch
					bsp->patches[i].bezier[j].render();
				}
			}
			glBindVertexArray(0);
        }
    }

	j7Model(q3BSP *bsp) { // Load from a BSP object
        vao = makeVAO(&bsp->vertices, 0);

		// Buffer the index data

		for (unsigned i = 0; i < bsp->facesByTexture.size(); ++i) meshes.push_back(j7Mesh(bsp,i));
	}

private:
    std::vector<j7Mesh> meshes;
	GLuint vao;
	GLuint ibo;
};

class j7Cam {
public:
	std::stack<glm::fmat4> modelviewMatrix;
	std::stack<glm::fmat4> projectionMatrix;

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

		//Set our matrices to identity
		projectionMatrix.push(glm::fmat4());
		modelviewMatrix.push(glm::fmat4());
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
		printPos(0);

	}

	void printPos(q3BSP *bsp) {
		glm::vec3 pos255 = getCurrentPos();
		if (bsp != 0) {
			int currleaf = bsp->findCurrentLeaf(pos255);
			std::cout << "Current leaf: " << currleaf << ".\n";
			std::cout << "Is 3339 visible? " << bsp->isClusterVisible(currleaf, 3339) << '\n';
		}
	}
    glm::vec3 getCurrentPos() {
		glm::mat4 view = glm::inverse(modelviewMatrix.top());
		glm::vec4 pos = view[3];
		std::cout << "Pos: " << pos.x << ',' << pos.y << ',' << pos.z << " Facing: " << angle.x << ".\n";
		glm::vec3 pos255(pos.x * 255, pos.y * 255, pos.x * 255);
        return pos255;
    }

	void adjustPerspective(sf::Vector2u windowsize, GLfloat fovy = 75.0f, GLfloat zNear = 0.1f, GLfloat zFar = 100.0f)
	{
		//Adjust drawing area & perspective on window resize
		//::TODO:: This currently runs many times for one resize as the window border is dragged. Add throttling?
		//::TODO:: Move into Camera class?

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

		//Update the modelview matrix
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

			// Cap horizotal angle to +/- 360 degrees to avoid possible overflow
            float fullcircle = glm::radians(360.0f);
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

    std::cout << "Trying to compile.\n";
    glCompileShader(shader);
    // Compile and check
    GLint shaderCompiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);
    if (shaderCompiled == false)
    {
        std::cerr << "ERROR: shader not compiled properly.\n";
		std::cerr << "Shader type: " << type << ".\n";

        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH , &maxLength);
        std::string infoLog;
        infoLog.reserve(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
        std::cout << "Compiler log: " << infoLog << "\n:ENDLOG:\n";

        //Cleanup
        glDeleteShader(shader);
        shader = 0;
    }
    return shader;
}




#endif