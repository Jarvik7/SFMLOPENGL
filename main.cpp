//
//  main.cpp
//  SFML OGL
//
//  Created by Jarvik7 on 2/11/14.
//  Copyright (c) 2014 Jarvik7. All rights reserved.
//

#include <iostream> // For std::cout, std::endl
#include <string> // For std::String

#include <GLEW/glew.h>	// For OpenGL Extensions

#include <SFML/OpenGL.hpp> // For OpenGL functions
#include <SFML/Graphics.hpp> // For SFML functions (window handling, ttf text drawing, vector2u)
#include <SFML/Audio.hpp> // For SFML MP3 playback

#include "j7util.hpp"

// Enable to display debug output ::TODO:: change this to read project build settings?
//const bool DISPLAYDEBUGOUTPUT = true;

// Define our key mapping ::TODO:: make this remappable in-game
const sf::Keyboard::Key key_quit = sf::Keyboard::Escape;
const sf::Keyboard::Key key_showfps = sf::Keyboard::Tab;
const sf::Keyboard::Key key_toggle_rotate = sf::Keyboard::R;
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

//Navigation keys
/*
const sf::Keyboard::Key key_move_forward = sf::Keyboard::W;
const sf::Keyboard::Key key_move_left = sf::Keyboard::A;
const sf::Keyboard::Key key_move_backward = sf::Keyboard::S;
const sf::Keyboard::Key key_move_right = sf::Keyboard::D;
const sf::Keyboard::Key key_move_up = sf::Keyboard::Space;
const sf::Keyboard::Key key_move_down = sf::Keyboard::LControl;
const sf::Keyboard::Key key_move_CW = sf::Keyboard::E;
const sf::Keyboard::Key key_move_CCW = sf::Keyboard::Q;
*/
const sf::Keyboard::Key key_lock_mouse = sf::Keyboard::L;
//M-Look while mouse1 is down?

const std::string windowTitle = "SFML OpenGL";


void displayWindowsMenubar(sf::RenderWindow *window)
{
	//::TODO: Automatically generate Windows/OSX menus from a structure containing the layout

	//For Windows
	#if defined(SFML_SYSTEM_WINDOWS)
//	generateMenu(window);
	#endif


	//For OSX
}


bool initGL()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);
    glClearColor(0, 0, 0, 0.5f);
    glClearDepth(1.0f);                         // Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);                        // Enables Depth Testing
    glDepthFunc(GL_LEQUAL);                         // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_2D);
	if (glewInit()) {
		std::cerr << "Error: Couldn't initialize GLew." << std::endl;
		return false;
	}
	if (DISPLAYDEBUGOUTPUT) {
		if (GLEW_ARB_compatibility) std::cout << "Compatibility mode supported" << std::endl;
		if (GLEW_VERSION_1_1) std::cout << "OpenGL ver >= 1.1: Vertex arrays supported!" << std::endl;
		if (GLEW_ARB_vertex_buffer_object) std::cout << "VBO supported!" << std::endl;
	}
	return true;
}

int main(int argc, const char * argv[])
{
    //Initialize the render window
    sf::ContextSettings windowsettings(24); // Give us a 24-bit depth buffer
    sf::RenderWindow window(sf::VideoMode(800,600,32), windowTitle, sf::Style::Default, windowsettings);
    if (!window.isOpen())
    {
        std::cerr << "Error: Couldn't create RenderWindow" << std::endl;
        return EXIT_FAILURE;
    }

//    window.setFramerateLimit(60); // Not needed w/ vsync

    bool fullscreen = false;
    bool vsync = true;

    window.setVerticalSyncEnabled(vsync);

    sf::Vector2u windowsize = window.getSize();

    // Initialize the OpenGL state
    if (!initGL()) return EXIT_FAILURE; // Exit, GLew failed.
    adjustPerspective(windowsize);

    //Display debug info about graphics
    if (DISPLAYDEBUGOUTPUT) {
        windowsettings = window.getSettings();
        sf::Vector2i windowpos = window.getPosition();

        std::cout << windowsize.x << "x" << windowsize.y << " window created at " << windowpos.x << "x" << windowpos.y << std::endl;
        std::cout << "OpenGL version:" << windowsettings.majorVersion << "." << windowsettings.minorVersion << std::endl;
        if(sf::Shader::isAvailable()) std::cout << "Shaders are available" << std::endl;
        else std::cout << "Shaders are not available" << std::endl;
        std::cout << "Depth bits: " << windowsettings.depthBits << std::endl;
        std::cout << "Stencil bits: " << windowsettings.stencilBits << std::endl;
        std::cout << "Antialiasing level: " << windowsettings.antialiasingLevel << std::endl;
    }

	displayWindowsMenubar(&window); // Display the menu bars


    sf::Music music;
    if(music.openFromFile("furious.ogg"))
    {
        music.setLoop(true);
        music.setVolume(75);
        music.play();
    }

    // Game loop control vars
    bool gameover = false;
    sf::Event event;

    // Animation control vars
    bool rotation = true;
    float rquad = 0;
    bool showfps = true;
	bool wireframe=false;
	short modelno=0;
	sf::Vector3f movement(0,0,0);
	float moveDelta = 0.1f;
	sf::Vector2i mouseDelta(0,0);
	
	bool mouseLock=true;

	// Setup fog
	glClearColor(0.5f,0.5f,0.5f,1.0f); 
	GLfloat fogColor[4]= {0.5f,0.5f,0.5f,1.0f};      // Fog Color
	glFogi(GL_FOG_MODE, GL_LINEAR);        // Fog Mode
	glFogfv(GL_FOG_COLOR, fogColor);            // Set Fog Color
	glFogf(GL_FOG_DENSITY, 0.8f);              // How Dense Will The Fog Be
	glHint(GL_FOG_HINT, GL_DONT_CARE);          // Fog Hint Value
	glFogf(GL_FOG_START, 1.0f);             // Fog Start Depth
	glFogf(GL_FOG_END, 50.0f);               // Fog End Depth
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_FOG);

	

	//Load our mesh
	j7Model apartment("apartment.obj");
	j7Model cube("2texcube.obj");
	j7Model tardis("tardis.obj");
	j7Cam camera;

    // Begin game loop
	if (mouseLock) sf::Mouse::setPosition(sf::Vector2i(windowsize.x/2, windowsize.y/2), window);
    while (!gameover)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		camera.update(&window);

		//glTranslatef(movement.x, movement.y, movement.z); // Do our navigation
	/*	glRotatef(mouseDelta.y*.1f, 1.0f, 0.f, 0.f);
		glRotatef(mouseDelta.x*.1f, 0.f, 1.f, 0.f);*/


		//Draw the ground
		drawGround();

		//Draw our spinny object
		glPushMatrix();

		glTranslatef(0.0f, 1.0f, -5.0f); // Move into the screen
        if(rotation) rquad+=02.0f;
        glRotatef(rquad * .5f, 1.0f, 0.0f, 0.0f);
        glRotatef(rquad * .3f, 0.0f, 1.0f, 0.0f);
        glRotatef(rquad * .9f, 0.0f, 0.0f, 1.0f);

        if (modelno==0) cube.drawVBO();
        if (modelno==1) tardis.drawVBO();
        //if (modelno==2) apartment.drawVBO();

		glPopMatrix();
		apartment.drawVBO();
        if (showfps) showFPS(&window); // Display the FPS

        window.display();

        //Handle window events
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                    gameover=true;
                    break;

                case sf::Event::KeyPressed:
                    switch (event.key.code)
                    {
                        case key_quit:
                            gameover=true;
                            break;
						// Toggles

						case key_lock_mouse:
							mouseLock=!mouseLock;
							window.setMouseCursorVisible(!mouseLock);
							break;


						case key_toggle_model:
							modelno=(modelno+1)%3;
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
                            rotation=!rotation;
                            break;

						case key_toggle_wireframe:
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

						case key_toggle_blending:
							if(glIsEnabled(GL_DEPTH_TEST)) {
								glDisable(GL_DEPTH_TEST);
								glEnable(GL_BLEND);
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
                            fullscreen=!fullscreen;
                            static sf::Vector2u oldwindowsize;

                            if (fullscreen)
                            {
                                oldwindowsize=windowsize;
                                window.create(sf::VideoMode::getDesktopMode(),windowTitle,sf::Style::Fullscreen,windowsettings);
                            }
                            else window.create(sf::VideoMode(oldwindowsize.x, oldwindowsize.y),windowTitle,sf::Style::Default,windowsettings);
                            initGL();
							displayWindowsMenubar(&window);
                            windowsize = window.getSize();
                            adjustPerspective(windowsize);
                            window.setVerticalSyncEnabled(vsync);
                            break;
                        }
                            
                        case key_toggle_vsync:
                            vsync=!vsync;
                            window.setVerticalSyncEnabled(vsync);
                            break;

                        default:
                            break;
                    }
                    break;

         /*       case sf::Event::MouseWheelMoved:
					zoom += event.mouseWheel.delta * .5f;
                    break;*/

                case sf::Event::Resized:
                    windowsize = window.getSize();
                    adjustPerspective(windowsize);
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



/*
Deprecated stuff I've already learned:
-Immediate drawing (glBegin, glEnd)
-Fixed function drawing (glVertex, glNormal, etc.)
-GL_QUADS
-Display lists




*/

