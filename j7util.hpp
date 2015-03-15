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

const bool DISPLAYDEBUGOUTPUT = true;

//Camera control/movement keys
const sf::Keyboard::Key key_move_forward = sf::Keyboard::W;
const sf::Keyboard::Key key_move_left = sf::Keyboard::A;
const sf::Keyboard::Key key_move_backward = sf::Keyboard::S;
const sf::Keyboard::Key key_move_right = sf::Keyboard::D;
const sf::Keyboard::Key key_move_up = sf::Keyboard::Space;
const sf::Keyboard::Key key_move_down = sf::Keyboard::LControl;
const sf::Keyboard::Key key_lean_right = sf::Keyboard::E;
const sf::Keyboard::Key key_lean_left = sf::Keyboard::Q;
const sf::Keyboard::Key key_move_run = sf::Keyboard::LShift;

inline bool fileExists(const std::string filename) {
    const std::ifstream infile(filename);
    return infile.good();
}

GLuint loadTexture(std::string filename) {
	//Sanity checking
	if (filename == "") {
		std::cerr << "Texture name is null\n";
		return 0;
	}
	// Try some common filetypes (Quake 3 etc don't specify extension in BSP)
	if (fileExists(filename + ".jpg")) filename += ".jpg";
	else if (fileExists(filename + ".tga")) filename += ".tga"; // Has alpha channel
	else if (fileExists(filename + ".png")) filename += ".png"; // QuakeLive etc.
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.getSize().x, texture.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getPixelsPtr());

	// Enable anisotropic filtering
	GLfloat largest_aniso;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_aniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_aniso);

	// Enable mipmapping & linear filtering
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
	// This only works in a compatibility profile due to SFML being out of date

	static sf::Font font;
	static bool fontloaded = false;
    static bool triedloadingfont = false;

    if (!fontloaded && triedloadingfont) return; // Early exit. We've tried and failed to load a font already.
	//Load the font
	if (!fontloaded) {
        triedloadingfont = true;
#if defined(SFML_SYSTEM_WINDOWS) 
        if (font.loadFromFile("c:\\Windows\\Fonts\\Arial.ttf")) fontloaded = true;
#elif defined(SFML_SYSTEM_MACOS)
        if (font.loadFromFile("/Library/Fonts/Arial.ttf")) fontloaded = true;
#endif
        if (!fontloaded) return; // Early exit. Failed to load a font.
	}

    static sf::Clock timer; // Times 1 second
	static int frame = 0;	// Holds number of frames this second
	static int fps = 0; // Holds fps

	//Calculate FPS
	++frame;
	if (timer.getElapsedTime().asSeconds() >= 1) // If 1 second has passed, tally frames and reset timer
	{
		fps = frame;
		frame = 0;
		timer.restart();
	}

	//Draw on screen, skip first second as it doesn't have a sample to calculate from yet
	if (fps > 0)	{
		window->pushGLStates();
        window->draw(sf::Text(std::to_string(fps) + " fps", font, 20));
		window->popGLStates();
	}
}

void textFPS() {
	static sf::Clock timer; // Times 1 second
	static int frame = 0;	// Holds number of frames this second

	//Calculate FPS
	++frame;
	if (timer.getElapsedTime().asSeconds() >= 1) // If 1 second has passed, tally frames and reset timer
	{
		timer.restart();
		std::cout << "FPS: " << frame << std::endl;
        frame = 0;
	}
}


class j7Model {
public:
    void drawVBO(const q3BSP *bsp, const glm::vec3 position, const glm::mat4 viewmatrix) {
        if (vao != 0) {
			glBindVertexArray(vao);

			const std::vector<int> visiblefaces = bsp->makeListofVisibleFaces(position, viewmatrix); // Find all faces visible from here

            // Sort faces by texture ::TODO:: do the same cluster checking here to prevent resorting these faces, or sort the faces in the list generation function
            // Should check which is better. Also, it might be better to draw in z order overall.
            // Frustrum culling should be added here as well. z-sorting the faces within the faceset might give speedups if a lot of faces use the same texture
            // GLMultiDrawElements for each set of faces per texture might be faster than multiple drawelements calls. Not applicable to patches
            // We will need z-sorting at a minimum for transparent faces so we can draw back to front. We'l also need to do the splitting here
            // Only model[0] is currently drawn. Q3 culls the models due to pvs despite not being in the bsp tree. Is it using the entities?
            // Q3 does not cull pickups. we can do better. entity culling must also be done here.
            /*
             1) Determine PVS faces. DONE
             2) Frustrum cull. disabled. Appears to be faster to leave it up to the GPU
             3) Split into opaque/transparent
             4) Op1: qsort both. Op2: qsort transparent, texture sort opaque.
             
             
             */

			//Send texture unit assignments to shader
			glUniform1i(bsp->texSamplerPos, 0);
			glUniform1i(bsp->lmSamplerPos, 1);

			//Toggle lightmaps vs vertex lighting
			const GLint vertexLightingPos = glGetUniformLocation(shaderID, "vertexLighting");
			const bool vertexLighting = sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace);
			glUniform1i(vertexLightingPos, vertexLighting);

			//Group faces into sets by texture
			std::vector<std::vector<int>> sortedfaces;
			sortedfaces.resize(bsp->textures.size());
            for (auto& face : visiblefaces) {
                sortedfaces[bsp->faces[face].texture].push_back(face);
            }

			glActiveTexture(GL_TEXTURE0);

            for (auto& faceset : sortedfaces) {
                if (faceset.size() == 0) continue; // This texture has no visible faces, skip to next

				glBindTexture(GL_TEXTURE_2D, bsp->textureIDs[bsp->faces[faceset[0]].texture]); //Bind this faceset's texture

				for (auto& face : faceset) {
					glUniform1i(bsp->lightmapIndexUniformPosition, bsp->faces[face].lm_index); // Lightmap array offset

					//Draw
					if (bsp->faces[face].type == bsp->SURF_POLY || bsp->faces[face].type == bsp->SURF_MODEL) {
						glDrawElements(GL_TRIANGLES, sizes[face], GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(offsets[face] * sizeof(GLuint)));
                    }
					else if (bsp->faces[face].type == bsp->SURF_PATCH) {
                        glDrawElementsBaseVertex(GL_TRIANGLE_STRIP, bsp->patches[face].n_indices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(bsp->patches[face].start), bsp->patches[face].offset);
                    }
                }
            }
            glBindVertexArray(0);
        }
    }

	j7Model(q3BSP *bsp) { // Load from a BSP object
		// Push the indices into a single array. Also populate arrays for offsets and number of indices of each face
		//TODO:: face type 1 is actually a triangle fan without the meshvert stuff? Maybe faster to render that way
		std::vector<GLuint> indexes;
		
		bsp->patches.resize(bsp->faces.size()); // Messy, TODO:: Change to only allocate patches as needed (map?)
		
		for (unsigned i = 0; i < bsp->faces.size(); ++i) {
			offsets.push_back(static_cast<GLuint>(indexes.size()));
			sizes.push_back(bsp->faces[i].n_meshverts);
			if (bsp->faces[i].type == bsp->SURF_MODEL || bsp->faces[i].type == bsp->SURF_POLY) { // Meshes and polys
				for (int j = 0; j < bsp->faces[i].n_meshverts; ++j) {
					indexes.push_back(bsp->faces[i].vertex + bsp->meshVerts[j + bsp->faces[i].meshvert]);
				}
			}
			else if (bsp->faces[i].type == bsp->SURF_PATCH) { // Patches
				bsp->patches[i] = BSPPatch(bsp, i);
			}
		}
        // Add patch data to the geometry data
        for (auto& patch : bsp->patches) {
            if (patch.vertices.size() == 0) continue; // Empty patch
            
            //Store offsets
            patch.offset = static_cast<GLint>(bsp->vertices.size());
            patch.start = static_cast<GLuint>(indexes.size() * sizeof(GLuint));

            // Push vertices & indices to the vectors
            for (auto& vert : patch.vertices) bsp->vertices.push_back(vert);
            for (auto& index : patch.indices) indexes.push_back(index);

            // Throw away the raw data
            patch.vertices.clear();
            patch.indices.clear();
        }
		vao = makeVAO(&bsp->vertices, &indexes);

		//Get texture / lm uniform locations
		bsp->texSamplerPos = glGetUniformLocation(shaderID, "tex");
		bsp->lmSamplerPos = glGetUniformLocation(shaderID, "lm");


	}

private:

	GLuint vao; // Our vertexes and indexes are here
	//List of index offsets and counts for each face (index = face number)
	std::vector<GLuint> offsets;
	std::vector<GLsizei> sizes;
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
		angle = glm::fvec2(0, 0);
		up = glm::fvec3(0.0f, 1.0f, 0.0f);

		//Set our matrices to identity
		projectionMatrix.push(glm::fmat4());
		modelviewMatrix.push(glm::fmat4());
	}

	void update(const sf::RenderWindow *window) {
		updatePosition(); // Movement
		updateAngle(window); // Look
		move(); // Apply change
	}

	void setMouseLock(const bool locked, sf::RenderWindow *window) { // Toggle mouse locking
		mouseLock = locked;
		window->setMouseCursorVisible(!mouseLock);
		if (mouseLock) {
			savedMousePosition = sf::Mouse::getPosition(); // Save mouse position
			sf::Vector2u windowsize = window->getSize();
			sf::Mouse::setPosition(sf::Vector2i(windowsize.x / 2, windowsize.y / 2 ), *window);
		}
		else sf::Mouse::setPosition(savedMousePosition); // Restore mouse position
	}

	void setFocus(const bool focus) {
		hasFocus = focus;
	}

	void goTo(const glm::fvec3 origin, const float viewangle) {
		eye.x = origin.x;
		eye.y = origin.z + 0.102f; // Offset for player eye height (26 in byte)
		eye.z = -origin.y;
		angle.x = viewangle;
		angle.y = 0;
		move();
		std::cout << "Teleporting: ";
		printPos(nullptr);
	}

	void printPos(const q3BSP *bsp) const {
		const glm::vec3 pos = getCurrentPos();
        std::cout << "Pos: " << pos.x << ',' << pos.y << ',' << pos.z << " Angle: " << angle.x << '\n';
		if (bsp != nullptr) std::cout << "Current leaf: " << bsp->leafs[bsp->findCurrentLeaf(pos)].cluster << ".\n";
	}
    glm::vec3 getCurrentPos() const {
		const glm::vec4 pos = glm::inverse(modelviewMatrix.top())[3];
		return glm::vec3(pos.x * 255, pos.z * -255, pos.y * 255); // Return de-swizzled position
    }

	void adjustPerspective(sf::Vector2u windowsize, const GLfloat fovy = 75.0f, const GLfloat zNear = 0.1f, const GLfloat zFar = 100.0f)
	{
		//Adjust drawing area & perspective on window resize
		//::TODO:: This currently runs many times for one resize as the window border is dragged. Add throttling?

#if defined(SFML_SYSTEM_WINDOWS) // Windows allows window height of 0, prevent div/0
		if (windowsize.y == 0) ++windowsize.y;
#endif
		if (DISPLAYDEBUGOUTPUT)	std::cout << "Window resized to " << windowsize.x << "x" << windowsize.y << '\n';

		glViewport(0, 0, windowsize.x, windowsize.y);

		glMatrixMode(GL_PROJECTION);
		projectionMatrix.pop();
		projectionMatrix.push(glm::perspective<float>(glm::radians(fovy), static_cast<float>(windowsize.x) / windowsize.y, zNear, zFar));

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
		center.x = eye.x + sinf(angle.x) * cosf(angle.y);
		center.y = eye.y + sinf(angle.y);
		center.z = eye.z + cosf(angle.x) * cosf(angle.y);

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

	void updateAngle(const sf::RenderWindow *window) {
		if (mouseLock && hasFocus) {
			const sf::Vector2u windowsize = window->getSize();
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
            const float fullcircle = glm::radians(360.0f);
			if (angle.x >= fullcircle) angle.x -= fullcircle;
			else if (angle.x <= -fullcircle) angle.x += fullcircle;

			// Reset cursor to center of screen
			sf::Mouse::setPosition(sf::Vector2i(windowsize.x / 2, windowsize.y / 2), *window);
		}
	}
};

GLenum loadShader(const std::string filename, const GLenum type) {
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
	std::string shaderSource;

    std::ifstream file(filename);
    shaderSource.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    const GLchar* shaderStringPointer = shaderSource.c_str();

    // Load it into OpenGL
    GLenum shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderStringPointer, NULL);

    std::cout << "Trying to compile.\n";
    glCompileShader(shader);
    // Compile and check
    GLint shaderCompiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);
    if (!shaderCompiled)
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