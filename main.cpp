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
const sf::Keyboard::Key key_toggle_model = sf::Keyboard::O;
const sf::Keyboard::Key key_lock_mouse = sf::Keyboard::L;
const sf::Keyboard::Key key_respawn = sf::Keyboard::T;
const sf::Keyboard::Key key_printloc = sf::Keyboard::Y;

const std::string windowTitle = "SFML OpenGL";

GLuint shaderID;

bool initGL()
{
	//Setup OpenGL backface culling
	glFrontFace(GL_CW); // Quake3 uses CW for frontface
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	//Setup OpenGL depth buffer
	glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	//OpenGL quality hinting
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST); // Disable HW frustrum culling (software is about 10 fps faster as it culls at earlier stage in pipeline)

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
    sf::ContextSettings windowsettings(24); // 24-bit depth buffer
    windowsettings.antialiasingLevel = 12; // SFML finds closest supporting value, which is 8 at work
    sf::RenderWindow window(sf::VideoMode(800, 600, 32), windowTitle, sf::Style::Default, windowsettings);
    if (!window.isOpen())
    {
        std::cerr << "Error: Couldn't create RenderWindow\n";
        return EXIT_FAILURE;
    }

	// Initialize the OpenGL state
	if (!initGL()) return EXIT_FAILURE; // Exit, GLew failed.

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

	//Setup camera
	j7Cam camera;
	camera.adjustPerspective(windowsize);

    //Display debug info about graphics
    if (DISPLAYDEBUGOUTPUT) {
        windowsettings = window.getSettings();
        sf::Vector2i windowpos = window.getPosition();

        std::cout << windowsize.x << "x" << windowsize.y << " window created at " << windowpos.x << "x" << windowpos.y << '\n';
        std::cout << "OpenGL version: " << windowsettings.majorVersion << "." << windowsettings.minorVersion << '\n';
        if (sf::Shader::isAvailable()) std::cout << "Shaders are available\n";
        else std::cout << "Shaders are not available\n";
        std::cout << "Depth bits: " << windowsettings.depthBits << '\n';
        std::cout << "Stencil bits: " << windowsettings.stencilBits << '\n';
        std::cout << "Antialiasing level: " << windowsettings.antialiasingLevel << '\n';
    }

    sf::Music music;

    sf::Event event; //SFML event handler

    // Animation control vars
	float fov=75.0f;
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

	//Load our mesh
    q3BSP test("maps/q3dm0.bsp");
	j7Model quake3(&test);

	if (music.openFromFile(test.worldMusic))
    {
        music.setLoop(true);
        music.setVolume(75);
        music.play();
    }
	else std::cerr << "Could not open music file: " << test.worldMusic << ".\n";

    GLenum glerror = GL_NO_ERROR;
	const GLint projectionViewLoc = glGetUniformLocation(shaderID, "projectionview");
	const GLint modelViewLoc = glGetUniformLocation(shaderID, "modelview");

	unsigned campos = 1;
	camera.goTo(test.cameraPositions[campos].origin, test.cameraPositions[campos].angle);
    // Begin game loop

    glPrimitiveRestartIndex(0xFFFFFFFF);
    glEnable(GL_PRIMITIVE_RESTART);


	bool gameover = false;
    while (!gameover)
    {
        glerror = glGetError();
        if (glerror != GL_NO_ERROR) std::cerr << "OpenGL ERROR: " << glerror << '\n';
        glClear(/*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT); // Clear depth buffer (color buffer clearing disabled = faster but smears when outside of map)


		camera.update(&window);
		const glm::mat4 view = camera.modelviewMatrix.top() * glm::scale(glm::fvec3(1.0/255, 1.0/255, 1.0/255)); // Scale down the map ::TODO:: can this be done by adjusting our frustum or something?

		// Send our view matrices to shader
		glUniformMatrix4fv(projectionViewLoc, 1, GL_FALSE, &camera.projectionMatrix.top()[0][0]);
		glUniformMatrix4fv(modelViewLoc, 1, GL_FALSE, &view[0][0]);

		quake3.drawVBO(&test, camera.getCurrentPos(), camera.projectionMatrix.top() * view); // Render the BSP

        //if (showfps) showFPS(&window); // Display the FPS
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
								// Inform camera
								camera.setMouseLock(mouseLock, &window);
								break;
							}

						case key_toggle_culling:
							if (!glIsEnabled(GL_CULL_FACE)) glEnable(GL_CULL_FACE);
							else glDisable(GL_CULL_FACE);
							break;

						case key_toggle_texturing:
							if(!glIsEnabled(GL_TEXTURE_2D))	glEnable(GL_TEXTURE_2D);
							else glDisable(GL_TEXTURE_2D);
							break;

						case key_toggle_lighting:
							if(!glIsEnabled(GL_LIGHTING)) glEnable(GL_LIGHTING);
							else glDisable(GL_LIGHTING);
							break;

						case key_toggle_wireframe:
							GLint temp;
							glGetIntegerv(GL_POLYGON_MODE, &temp);

							if (temp == GL_FILL) glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
							else glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

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
                            camera.adjustPerspective(windowsize);
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

			/*	case sf::Event::MenuitemSelected:
					if (event.menuAction.identifier == 0) break; // It wasn't a menu event??
					else if (event.menuAction.identifier == j7MenuIdentifier("Wireframe")) {
						wireframe=!wireframe;
						if (wireframe) {
							glPolygonMode(GL_FRONT,GL_LINE);
							glPolygonMode(GL_BACK,GL_LINE);
						}
						else {
							glPolygonMode(GL_FRONT,GL_FILL);
							glPolygonMode(GL_BACK,GL_FILL);
						}
						break;
					}
					else if (event.menuAction.identifier == j7MenuIdentifier("Exit")) gameover=true;
					break;
                    */
                default:
					//std::cerr << "Unknown event type: " << event.type << std::endl;
                    break;
            }
        }
	}
    return EXIT_SUCCESS;
}


