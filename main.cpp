//
//  main.cpp
//  SFML OGL
//
//  Created by Jarvik7 on 2/11/14.
//  Copyright (c) 2014 Jarvik7. All rights reserved.
//

#include <iostream> // std::cout
#include <string> // std::string
#include <stack>

#include <GLEW/glew.h>	// For OpenGL Extensions

#include <SFML/OpenGL.hpp> // For OpenGL functions
#include <SFML/Graphics.hpp> // For SFML functions (window handling, ttf text drawing, vectors)
#include <SFML/Audio.hpp> // For MP3 playback ::TODO:: switch to something that supports MP3

#include "q3bsploader.h" // My BSP loader
#include "j7util.hpp" // My helper utility

#include <glm/gtc/type_ptr.hpp> 

// Enable to display debug output ::TODO:: change this to read project build settings?
//const bool DISPLAYDEBUGOUTPUT = true;

// Define our key mapping ::TODO:: make this remappable in-game
const sf::Keyboard::Key key_quit = sf::Keyboard::Escape;
const sf::Keyboard::Key key_toggle_music = sf::Keyboard::M;
const sf::Keyboard::Key key_toggle_fullscreen = sf::Keyboard::F;
const sf::Keyboard::Key key_toggle_vsync = sf::Keyboard::V;
const sf::Keyboard::Key key_toggle_fps = sf::Keyboard::Tab;
const sf::Keyboard::Key key_toggle_blending = sf::Keyboard::Num4;
const sf::Keyboard::Key key_toggle_fog = sf::Keyboard::K;
const sf::Keyboard::Key key_toggle_culling = sf::Keyboard::C;
const sf::Keyboard::Key key_toggle_wireframe = sf::Keyboard::Num1;
const sf::Keyboard::Key key_toggle_texturing = sf::Keyboard::Num2;
const sf::Keyboard::Key key_toggle_lighting = sf::Keyboard::Num3;
const sf::Keyboard::Key key_lock_mouse = sf::Keyboard::L;
const sf::Keyboard::Key key_respawn = sf::Keyboard::T;
const sf::Keyboard::Key key_printloc = sf::Keyboard::Y;


GLuint shaderID;

bool initGL()
{
	//Setup OpenGL backface culling
	glFrontFace(GL_CW); // Quake3 uses CW for frontface
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	//Setup OpenGL depth buffer
	glDepthRange(0, 1);
	glClearDepth(1.0f);
	glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL); // FIXME: Any reason not to use GL_LESS?
	glEnable(GL_DEPTH_TEST);

	//Enable multisampling AA
	glEnable(GL_MULTISAMPLE_ARB);
	//glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST); // Causes a GLERROR on win32. Obsolete according to Apple

	//OpenGL quality hinting
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Immediate mode only?
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	//glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST); // Disable HW frustrum culling as it should be faster in software (appears to have no effect?)

    glewExperimental = true; // Needed for OSX to build?

	if (glewInit()) {
		std::cerr << "Error: Couldn't initialize GLEW.\n";
		return false;
	}
	return true;
}

int main(const int argc, const char * argv[])
{
    //Initialize the render window
	const std::string windowTitle = "Q3 Map Viewer";
    sf::ContextSettings windowsettings(24, 0, 16); // 24-bit depth buffer, 0-bit stencil, 16x MSAA, default GL version
	sf::RenderWindow window(sf::VideoMode(800, 600, 32), windowTitle, sf::Style::Default, windowsettings);
    if (!window.isOpen())
    {
        std::cerr << "Error: Couldn't create RenderWindow\n";
        return EXIT_FAILURE;
    }

	// Initialize the OpenGL state
	if (!initGL()) return EXIT_FAILURE; // Exit, GLew failed.
	//Used for drawing tessellated surfaces
	glPrimitiveRestartIndex(0xFFFFFFFF);
	glEnable(GL_PRIMITIVE_RESTART);

	//Setup Vsync (always ON on OSX)
	bool vsync = true;
	window.setVerticalSyncEnabled(vsync);

	//Window settings
	sf::Vector2u windowsize = window.getSize();
    bool fullscreen = false;

	//Mouse settings
	bool hasFocus = true;
	bool mouseWasLocked = false;
	bool mouseLock = false;
	if (mouseLock) sf::Mouse::setPosition(sf::Vector2i(windowsize.x / 2, windowsize.y / 2), window);

    //Display debug info about graphics
    if (DISPLAYDEBUGOUTPUT) {
		windowsettings = window.getSettings();
        const sf::Vector2i windowpos = window.getPosition();

        std::cout << windowsize.x << "x" << windowsize.y << " window created at " << windowpos.x << "x" << windowpos.y << '\n';
        std::cout << "OpenGL version: " << windowsettings.majorVersion << "." << windowsettings.minorVersion << '\n';
        if (sf::Shader::isAvailable()) std::cout << "Shaders are available\n";
        else std::cout << "Shaders are not available\n";
        std::cout << "Depth bits: " << windowsettings.depthBits << '\n';
        std::cout << "Stencil bits: " << windowsettings.stencilBits << '\n';
        std::cout << "Antialiasing level: " << windowsettings.antialiasingLevel << '\n';
    }

    sf::Event event; //SFML event handler

    // Animation control vars
    bool showfps = true;
	
	//Setup the vertex & fragment shaders
    shaderID = glCreateProgram();
    const GLenum vertshader = loadShader("texture.vert", GL_VERTEX_SHADER);
    const GLenum fragshader = loadShader("texture.frag", GL_FRAGMENT_SHADER);
    if (vertshader != 0 && fragshader != 0) {
        glAttachShader(shaderID, vertshader);
        glAttachShader(shaderID, fragshader);
        glLinkProgram(shaderID);

        GLint programSuccess;
        glGetProgramiv(shaderID, GL_LINK_STATUS, &programSuccess);
        if (programSuccess != GL_TRUE) {
            std::cerr << "Error linking program: " << shaderID << ".\n";
            glDeleteProgram(shaderID);
            shaderID = 0;
        }
    }
	else shaderID = 0;
	glUseProgram(shaderID);

	const GLint projectionViewLoc = glGetUniformLocation(shaderID, "projectionview");
	const GLint modelViewLoc = glGetUniformLocation(shaderID, "modelview");

	//Load map
    q3BSP test("maps/q3dm1.bsp");
	j7Model quake3(&test);
	
	//Load map music
	sf::Music music;
	if (music.openFromFile(test.worldMusic))
    {
        music.setLoop(true);
        music.setVolume(75);
        music.play();
    }
	else std::cerr << "Could not open music file: " << test.worldMusic << ".\n";

	//Setup camera
	float fov = 75.0f;
	j7Cam camera;
	camera.adjustPerspective(windowsize, fov);
	unsigned campos = 1;
	camera.goTo(test.cameraPositions[campos].origin, test.cameraPositions[campos].angle); // FIXME: This is causing a breakpoint in debug for invalid index



	//Begin game loop
	GLenum glerror = GL_NO_ERROR;
	bool gameover = false;

    while (!gameover)
    {
        glerror = glGetError();
        if (glerror != GL_NO_ERROR) std::cerr << "OpenGL ERROR: " << glerror << '\n';
        glClear(/*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT); // Clear depth buffer (color buffer clearing disabled = faster but smears when outside of map)

		camera.update(&window);

		//Setup view matrices
		const glm::mat4 view = camera.modelviewMatrix.top() * glm::scale(glm::fvec3(1.0 / 255, 1.0 / 255, 1.0 / 255)); // Scale down the map ::TODO:: can this be done by adjusting our frustum or something?
		glUniformMatrix4fv(modelViewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionViewLoc, 1, GL_FALSE, glm::value_ptr(camera.projectionMatrix.top()));

		quake3.drawVBO(&test, camera.getCurrentPos(), camera.projectionMatrix.top() * view); // Render the BSP

        //if (showfps) showFPS(&window); // Display the FPS, only works in compatibility profile
		textFPS();

        window.display();

        //Handle window events
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
				case sf::Event::LostFocus:
					hasFocus = false;
					camera.setMouseLock(false, &window);
					camera.setFocus(false);
					break;

				case sf::Event::GainedFocus:
					if (mouseWasLocked) camera.setMouseLock(true, &window);
					camera.setFocus(true);
					hasFocus = true;
					break;

                case sf::Event::Closed:
                    gameover = true;
                    break;

                case sf::Event::KeyPressed:
                    if (hasFocus) switch (event.key.code)
                    {
                        case key_quit:
                            gameover = true;
                            break;

						case key_respawn:
							++campos;
							if (campos > test.cameraPositions.size() - 1) campos = 0;
							camera.goTo(test.cameraPositions[campos].origin, test.cameraPositions[campos].angle);
							break;

						case key_printloc:
							camera.printPos(&test);
							
							break;
						// Toggles

						case key_lock_mouse:
							{
								mouseLock = !mouseLock;
								mouseWasLocked = mouseLock;	
								camera.setMouseLock(mouseLock, &window);
								break;
							}

						case key_toggle_culling:
							if (!glIsEnabled(GL_CULL_FACE)) glEnable(GL_CULL_FACE);
							else glDisable(GL_CULL_FACE);
							break;
					// These only work in fixed function OpenGL. Need to replace with a bool uniform sent to shader to turn on/off
					/*	case key_toggle_texturing:
							if(!glIsEnabled(GL_TEXTURE_2D))	glEnable(GL_TEXTURE_2D);
							else glDisable(GL_TEXTURE_2D);
							break;

						case key_toggle_lighting:
							if(!glIsEnabled(GL_LIGHTING)) glEnable(GL_LIGHTING);
							else glDisable(GL_LIGHTING);
							break;*/

						case key_toggle_wireframe:
							GLint polygonMode;
							glGetIntegerv(GL_POLYGON_MODE, &polygonMode);

							if (polygonMode == GL_FILL) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
							else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

							break;

						case key_toggle_blending:
							if(glIsEnabled(GL_DEPTH_TEST)) {
								glDisable(GL_DEPTH_TEST);
								glEnable(GL_BLEND);
								glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							}
							else {
								glDisable(GL_BLEND);
								glEnable(GL_DEPTH_TEST);
							}
							break;

						case key_toggle_fog:
							if (!glIsEnabled(GL_FOG)) glEnable(GL_FOG);
							else glDisable(GL_FOG);
							break;

                        case key_toggle_fps:
                            showfps = !showfps;
                            break;

                        case key_toggle_music:
                        {
                            const sf::SoundSource::Status musicstatus = music.getStatus();
                            if (musicstatus == sf::SoundSource::Paused) music.play();
                            else if (musicstatus == sf::SoundSource::Playing) music.pause();
                            // The music should never be stopped unless it failed to load
                            break;
                        }

                        case key_toggle_fullscreen:
                        {
                            fullscreen = !fullscreen;
                            static sf::Vector2u oldwindowsize;

                            if (fullscreen)
                            {
                                oldwindowsize = windowsize;
                                window.create(sf::VideoMode::getDesktopMode(), windowTitle, sf::Style::Fullscreen, windowsettings);
                            }
                            else window.create(sf::VideoMode(oldwindowsize.x, oldwindowsize.y), windowTitle, sf::Style::Default, windowsettings);
                            initGL();
                            windowsize = window.getSize();
                            camera.adjustPerspective(windowsize, fov);
                            window.setVerticalSyncEnabled(vsync);
                            break;
                        }
                            
                        case key_toggle_vsync:
                            vsync = !vsync;
                            window.setVerticalSyncEnabled(vsync);
                            break;

                        default:
                            break;
                    }
                    break;

                case sf::Event::MouseWheelMoved: // Zoom
					if (hasFocus) {
						fov -= event.mouseWheel.delta * 1.5f;
						camera.adjustPerspective(windowsize, fov);
					}
                    break;

                case sf::Event::Resized:
                    windowsize = window.getSize();
                    camera.adjustPerspective(windowsize, fov);
                    break;

                default:
					//std::cerr << "Unknown event type: " << event.type << std::endl;
                    break;
            }
        }
	}
    return EXIT_SUCCESS;
}


