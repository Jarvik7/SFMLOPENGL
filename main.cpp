//
//  main.cpp
//  SFML OGL
//
//  Created by Jarvik7 on 2/11/14.
//  Copyright (c) 2014 Jarvik7. All rights reserved.
//

#include <iostream> // For std::cout, std::endl
#include <string> // For std::String

#include <GL/glew.h>
#include <SFML/OpenGL.hpp> // For OpenGL functions
#include <SFML/Graphics.hpp> // For SFML functions (window handling, ttf text drawing, vector2u)
#include <SFML/Audio.hpp> // For SFML MP3 playback
#include "j7util.hpp"
#include "freeglut_geometry.c"

// Enable to display debug output ::TODO:: change this to read project build settings?
//const bool DISPLAYDEBUGOUTPUT = true;

// Define our key mapping ::TODO:: make this remappable in-game
const sf::Keyboard::Key key_quit = sf::Keyboard::Key::Escape;
const sf::Keyboard::Key key_showfps = sf::Keyboard::Key::Tab;
const sf::Keyboard::Key key_toggle_rotate = sf::Keyboard::Key::R;
const sf::Keyboard::Key key_toggle_music = sf::Keyboard::Key::M;
const sf::Keyboard::Key key_toggle_fullscreen = sf::Keyboard::Key::F;
const sf::Keyboard::Key key_toggle_vsync = sf::Keyboard::Key::V;
const sf::Keyboard::Key key_toggle_fps = sf::Keyboard::Key::Tab;

const std::string windowTitle = "SFML OpenGL";

bool initGL()
{
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);
    glClearColor(0, 0, 0, 0.5f);
    glClearDepth(1.0f);                         // Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);                        // Enables Depth Testing
    glDepthFunc(GL_LEQUAL);                         // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	if (glewInit()) {
		std::cerr << "Error: Couldn't initialize GLew." << std::endl;
		return false;
	}
	if (DISPLAYDEBUGOUTPUT) {
		if (GLEW_ARB_compatibility) std::cout << "Compatibility mode supported" << std::endl;
		if (GLEW_VERSION_1_1) std::cout << "OpenGL 1.1: Vertex arrays supported!" << std::endl;
		if (GLEW_ARB_vertex_buffer_object) std::cout << "VBO supported!" << std::endl;
	}
	return true;
}

void drawCube()
{
    glBegin(GL_QUADS);                  // Start Drawing The Cube
    glColor3f(0.0f,1.0f,0.0f);          // Set The Color To Green
    glVertex3f( 1.0f, 1.0f,-1.0f);          // Top Right Of The Quad (Top)
    glVertex3f(-1.0f, 1.0f,-1.0f);          // Top Left Of The Quad (Top)
    glVertex3f(-1.0f, 1.0f, 1.0f);          // Bottom Left Of The Quad (Top)
    glVertex3f( 1.0f, 1.0f, 1.0f);          // Bottom Right Of The Quad (Top)

    glColor3f(1.0f,0.5f,0.0f);          // Set The Color To Orange
    glVertex3f( 1.0f,-1.0f, 1.0f);          // Top Right Of The Quad (Bottom)
    glVertex3f(-1.0f,-1.0f, 1.0f);          // Top Left Of The Quad (Bottom)
    glVertex3f(-1.0f,-1.0f,-1.0f);          // Bottom Left Of The Quad (Bottom)
    glVertex3f( 1.0f,-1.0f,-1.0f);          // Bottom Right Of The Quad (Bottom)

    glColor3f(1.0f,0.0f,0.0f);          // Set The Color To Red
    glVertex3f( 1.0f, 1.0f, 1.0f);          // Top Right Of The Quad (Front)
    glVertex3f(-1.0f, 1.0f, 1.0f);          // Top Left Of The Quad (Front)
    glVertex3f(-1.0f,-1.0f, 1.0f);          // Bottom Left Of The Quad (Front)
    glVertex3f( 1.0f,-1.0f, 1.0f);          // Bottom Right Of The Quad (Front)

    glColor3f(1.0f,1.0f,0.0f);          // Set The Color To Yellow
    glVertex3f( 1.0f,-1.0f,-1.0f);          // Bottom Left Of The Quad (Back)
    glVertex3f(-1.0f,-1.0f,-1.0f);          // Bottom Right Of The Quad (Back)
    glVertex3f(-1.0f, 1.0f,-1.0f);          // Top Right Of The Quad (Back)
    glVertex3f( 1.0f, 1.0f,-1.0f);          // Top Left Of The Quad (Back)

    glColor3f(0.0f,0.0f,1.0f);          // Set The Color To Blue
    glVertex3f(-1.0f, 1.0f, 1.0f);          // Top Right Of The Quad (Left)
    glVertex3f(-1.0f, 1.0f,-1.0f);          // Top Left Of The Quad (Left)
    glVertex3f(-1.0f,-1.0f,-1.0f);          // Bottom Left Of The Quad (Left)
    glVertex3f(-1.0f,-1.0f, 1.0f);          // Bottom Right Of The Quad (Left)

    glColor3f(1.0f,0.0f,1.0f);          // Set The Color To Violet
    glVertex3f( 1.0f, 1.0f,-1.0f);          // Top Right Of The Quad (Right)
    glVertex3f( 1.0f, 1.0f, 1.0f);          // Top Left Of The Quad (Right)
    glVertex3f( 1.0f,-1.0f, 1.0f);          // Bottom Left Of The Quad (Right)
    glVertex3f( 1.0f,-1.0f,-1.0f);          // Bottom Right Of The Quad (Right)
    glEnd();                        // Done Drawing The Quad
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

    //Start music
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
    float zoom = 0;

	// Create texture
    GLuint texture = 0;
    {
        sf::Image image;
        if (!image.loadFromFile("cube.jpg"))
            return EXIT_FAILURE;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, image.getSize().x, image.getSize().y, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
	glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    //Drawing list for cube
/*    GLuint box = glGenLists(1);
    glNewList(box,GL_COMPILE);
    glutSolidCube(5); // Lighted
	//drawCube(); // Colored
    glEndList();*/

	GLfloat cube[] =
    {
        // positions    // texture coordinates
        -20, -20, -20,  0, 0,
        -20,  20, -20,  1, 0,
        -20, -20,  20,  0, 1,
        -20, -20,  20,  0, 1,
        -20,  20, -20,  1, 0,
        -20,  20,  20,  1, 1,

         20, -20, -20,  0, 0,
         20,  20, -20,  1, 0,
         20, -20,  20,  0, 1,
         20, -20,  20,  0, 1,
         20,  20, -20,  1, 0,
         20,  20,  20,  1, 1,

        -20, -20, -20,  0, 0,
         20, -20, -20,  1, 0,
        -20, -20,  20,  0, 1,
        -20, -20,  20,  0, 1,
         20, -20, -20,  1, 0,
         20, -20,  20,  1, 1,

        -20,  20, -20,  0, 0,
         20,  20, -20,  1, 0,
        -20,  20,  20,  0, 1,
        -20,  20,  20,  0, 1,
         20,  20, -20,  1, 0,
         20,  20,  20,  1, 1,

        -20, -20, -20,  0, 0,
         20, -20, -20,  1, 0,
        -20,  20, -20,  0, 1,
        -20,  20, -20,  0, 1,
         20, -20, -20,  1, 0,
         20,  20, -20,  1, 1,

        -20, -20,  20,  0, 0,
         20, -20,  20,  1, 0,
        -20,  20,  20,  0, 1,
        -20,  20,  20,  0, 1,
         20, -20,  20,  1, 0,
         20,  20,  20,  1, 1
    };

    // Enable position and texture coordinates vertex components
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), cube);
    glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), cube + 3);

    // Disable normal and color vertex components
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    // Begin game loop
    while (!gameover)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        glTranslatef(0.0f,0.0f,-70.0f+zoom);              // Move into the screen
        if(rotation) rquad+=02.0f;
        glRotatef(rquad * .5f, 1.0f, 0.0f, 0.0f);
        glRotatef(rquad * .3f, 0.0f, 1.0f, 0.0f);
        glRotatef(rquad * .9f, 0.0f, 0.0f, 1.0f);

        //glCallList(box); // Display list box (colored ot lighted)
		glDrawArrays(GL_TRIANGLES, 0, 36); // Vertex array box (textured)

        if (showfps) showFPS(&window);
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

                        case key_toggle_rotate:
                            rotation=!rotation;
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

                case sf::Event::MouseWheelMoved:
                    zoom += event.mouseWheel.delta * .5f;
                    break;

                case sf::Event::Resized:
                    windowsize = window.getSize();
                    adjustPerspective(windowsize);
                    break;
                    
                default:
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

