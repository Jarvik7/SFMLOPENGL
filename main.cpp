//
//  main.cpp
//  SFML OGL
//
//  Created by Jarvik7 on 2/11/14.
//  Copyright (c) 2014 Jarvik7. All rights reserved.
//

#include <iostream> // For std::cout, std::endl
#include <string> // For std::String
#include <map>
#include <vector>

#include <assimp/Importer.hpp>	//For 3D model loading
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <GL/glew.h>	// For OpenGL Extensions
#include <SFML/OpenGL.hpp> // For OpenGL functions
#include <SFML/Graphics.hpp> // For SFML functions (window handling, ttf text drawing, vector2u)
#include <SFML/Audio.hpp> // For SFML MP3 playback
#include "j7util.hpp"
#include "freeglut_geometry.c"

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
const sf::Keyboard::Key key_toggle_blending = sf::Keyboard::B;
const sf::Keyboard::Key key_toggle_fog = sf::Keyboard::K;
const sf::Keyboard::Key key_toggle_culling = sf::Keyboard::C;
const sf::Keyboard::Key key_toggle_wireframe = sf::Keyboard::W;
const sf::Keyboard::Key key_toggle_texturing = sf::Keyboard::T;
const sf::Keyboard::Key key_toggle_lighting = sf::Keyboard::L;
const sf::Keyboard::Key key_toggle_model = sf::Keyboard::O;

const std::string windowTitle = "SFML OpenGL";

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


class j7mesh {
/*
This class is to handle loading meshes and textures from a file and supplying functions to create a
- DisplayList
- Vertex Array
- VBO

It does not handle anything other than a basic diffuse texture and maybe material color.
It only handles triangulated meshes.
*/

public:
	bool isloaded;
	GLuint displayList;
	j7mesh(std::string path)
	{
		isloaded=false;
		Assimp::Importer importer;
		scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality);
		if (scene) {
			isloaded=true;
			getTextures();
			displayList = glGenLists(1);
			glNewList(displayList,GL_COMPILE);
			generateDisplayList(scene, scene->mRootNode);
			glEndList();
		}
		else {
			std::cerr << "Couldn't load mesh." << std::endl;
		}
	}

private:
	const aiScene* scene;
	std::map<std::string, GLuint> textureIdMap; // Filename to textureID map
	void generateDisplayList(const aiScene *sc, const aiNode *nd)
	{
		for (unsigned i=0; i<nd->mNumMeshes; i++) {
			const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[i]];
			
			//Load the texture for this node
			aiString texPath;
			if(AI_SUCCESS == sc->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath)) {
				GLuint texId = textureIdMap[texPath.data];
				glBindTexture(GL_TEXTURE_2D, texId);
			}

			//Draw the faces for this node
			for (unsigned t = 0; t < mesh->mNumFaces; ++t) {
				const struct aiFace* face = &mesh->mFaces[t];

				glBegin(GL_TRIANGLES);
			
				for(unsigned i = 0; i < face->mNumIndices; i++)	// go through all vertices in face
				{
					int vertexIndex = face->mIndices[i];	// get group index for current index
					if(mesh->mColors[0] != NULL) glColor4fv(&mesh->mColors[0][vertexIndex].r);
					if(mesh->mNormals != NULL) {
						if(mesh->HasTextureCoords(0))
						{
							glTexCoord2f(mesh->mTextureCoords[0][vertexIndex].x, 1 - mesh->mTextureCoords[0][vertexIndex].y); //mTextureCoords[channel][vertex]
						}
					}
					glNormal3fv(&mesh->mNormals[vertexIndex].x);
					glVertex3fv(&mesh->mVertices[vertexIndex].x);
				}
				glEnd();
			}
		}
		for (unsigned i=0; i< nd->mNumChildren; i++) generateDisplayList(sc, nd->mChildren[i]);
	}
	void getTextures()
	{
		for (unsigned i=0; i < scene->mNumMaterials; i++)
		{
			//Get number of textures and create a map entry for each filename
			aiString path;	// filename
			for (unsigned j=0; j< scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE);j++) {
				scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, j, &path);
				textureIdMap[path.data] = NULL; //fill map with paths
				std::cout << "Indexed material #" << i << ", texture #" << j << ": " << path.data << std::endl;
			}
		}
		int numTextures = textureIdMap.size();
		
		std::map<std::string, GLuint>::iterator itr = textureIdMap.begin();
		for (int i=0; i<numTextures; i++) {
			GLuint textureid=0;
			glGenTextures(1, &textureid); // Generate an OpenGL ID
			(*itr).second = textureid; // Add ID to map

			//Load texture from disc
			std::string texturepath = (*itr).first; // Get filename from map
			itr++;								  // next texture
			sf::Image texturedata;
			texturedata.loadFromFile(texturepath);
			std::cout << "Loaded texture number " << i << ": " << texturepath << ", ID: " << textureid << std::endl;

			// Associate texture with ID
			glBindTexture(GL_TEXTURE_2D, textureid);
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, texturedata.getSize().x, texturedata.getSize().y, GL_RGBA, GL_UNSIGNED_BYTE, texturedata.getPixelsPtr());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
};

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
/*
	auto hMenu = CreateMenu();
	auto hSubMenu = CreatePopupMenu();
	#define ID_FILE_EXIT 9001
	#define ID_STUFF_GO 9002
	AppendMenu(hSubMenu, MF_STRING, ID_FILE_EXIT, "E&xit");
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&File");
	hSubMenu = CreatePopupMenu();
        AppendMenu(hSubMenu, MF_STRING, ID_STUFF_GO, "&Go");
        AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Stuff");
	SetMenu(window.getSystemHandle(),hMenu);*/




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
	bool wireframe=false;
	short modelno=0;
	
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
	j7mesh doctor("doctor_who.obj");
	j7mesh cube("2texcube.obj");
	j7mesh tardis("tardis.obj");

    // Begin game loop
    while (!gameover)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        glTranslatef(0.0f,0.0f,-5.0f+zoom);              // Move into the screen
        if(rotation) rquad+=02.0f;
        glRotatef(rquad * .5f, 1.0f, 0.0f, 0.0f);
        glRotatef(rquad * .3f, 0.0f, 1.0f, 0.0f);
        glRotatef(rquad * .9f, 0.0f, 0.0f, 1.0f);

		if (modelno==0) glCallList(cube.displayList); // Display the cube
		if (modelno==1) glCallList(tardis.displayList); // Display the cube
		if (modelno==2) glCallList(doctor.displayList); // Display the cube

        if (showfps) showFPS(&window); // Display the FPS
        window.display();

	/*	MSG message;
		while(PeekMessage(&message, NULL, WM_COMMAND,WM_COMMAND, PM_REMOVE)) {
			std::cerr << "Sup dawg: " << message.message << std::endl;
		}*/
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
					//std::cerr << "Unknown event type: " << event.type << std::endl;
                    break;
            }
        }

		//message.messa
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

