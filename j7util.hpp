//
//  j7util.hpp
//  SFML OGL
//
//  Created by Jarvik7 on 2/17/14.
//  Copyright (c) 2014 Jarvik7. All rights reserved.
//

#ifndef SFML_OGL_j7util_hpp
#define SFML_OGL_j7util_hpp

#define M_PI 3.14159265358979323846

const bool DISPLAYDEBUGOUTPUT = true;


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

#endif
