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
const sf::Keyboard::Key key_toggle_rotate = sf::Keyboard::R;
const sf::Keyboard::Key key_reset_rotate = sf::Keyboard::H;
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
	glEnable(GL_CULL_FACE);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClearDepth(1.0f);                         // Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);                        // Enables Depth Testing
    glDepthFunc(GL_LEQUAL);                         // The Type Of Depth Testing To Do
	glFrontFace(GL_CW);							// Quake3 uses CW for frontface
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glewExperimental = true;
	if (glewInit()) {
		std::cerr << "Error: Couldn't initialize GLEW.\n";
		return false;
	}



	if (DISPLAYDEBUGOUTPUT) {
		if (GLEW_ARB_compatibility) std::cout << "Compatibility mode supported\n";
		if (GLEW_VERSION_1_1) std::cout << "OpenGL ver >= 1.1: Vertex arrays supported!\n";
		if (GLEW_ARB_vertex_buffer_object) std::cout << "VBO supported!\n";
	}
	return true;
}

int main(int argc, const char * argv[])
{
    //Initialize the render window
    sf::ContextSettings windowsettings(24); // 24-bit depth buffer
    windowsettings.antialiasingLevel = 12;
    sf::RenderWindow window(sf::VideoMode(800, 600, 32), windowTitle, sf::Style::Default, windowsettings);
    if (!window.isOpen())
    {
        std::cerr << "Error: Couldn't create RenderWindow\n";
        return EXIT_FAILURE;
    }

    bool fullscreen = false;
    bool vsync = true;
	bool hasFocus = true;
	bool mouseWasLocked = false;

    window.setVerticalSyncEnabled(vsync);

    sf::Vector2u windowsize = window.getSize();
	bool mouseLock = false;
	if (mouseLock) sf::Mouse::setPosition(sf::Vector2i(windowsize.x / 2, windowsize.y / 2), window);
	j7Cam camera;
	camera.adjustPerspective(windowsize);


    // Initialize the OpenGL state
    if (!initGL()) return EXIT_FAILURE; // Exit, GLew failed.


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

    // Game loop control vars
    bool gameover = false;
    sf::Event event;

    // Animation control vars
    bool rotation = false;
    float rquad = 0;
	float fov=75.0f;
    bool showfps = true;
	short modelno=0;
	


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
    glUseProgram(shaderID);
	glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST); // Disable HW frustrum culling (software is about 10 fps faster as it culls at earlier stage in pipeline)

    while (!gameover)
    {
        glerror = glGetError();
        if (glerror != GL_NO_ERROR) std::cerr << "OpenGL ERROR: " << glerror << '\n';
        glClear(/*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT);


		camera.update(&window);
		const glm::mat4 view = camera.modelviewMatrix.top() * glm::scale(glm::fvec3(1.0/255, 1.0/255, 1.0/255)); // Scale down the map ::TODO:: can this be done by adjusting our frustrum or something?

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
							if (campos > test.cameraPositions.size()-1) campos=0;
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

						case key_toggle_model:
							modelno = (modelno + 1) % 3;
							break;

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

                        case key_toggle_rotate:
                            rotation = !rotation;
                            break;

						case key_reset_rotate:
							rquad = 0;
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
                            sf::SoundSource::Status musicstatus = music.getStatus();
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


