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
#include <functional>
#include <Windows.h>

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
unsigned j7MenuIdentifier(std::string name)
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
	unsigned numFaces;
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

class j7Model {
public:
	void j7Model {
		Assimp::Importer importer;
		const aiScene *scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Quality);
		for (int i=0; i< scene->m
	}
private:
	const aiScene* scene;
};


class j7Mesh_nonindexed {
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> textureCoordinates;


};

// Below is new implementation for indexed vertex arrays
/*
class j7Mesh {
public:
	std::vector<aiVector3D> normals; // Should have vertices/3 number of normals
	std::vector<aiVector3D> vertices;
	std::vector<aiVector3D> textureCoords;
	std::vector<aiColor4D> colors;
	GLuint textureid;
};

class j7Model {
public:
	std::vector<j7Mesh> meshes;

	j7Model(std::string filename) {
		Assimp::Importer importer;
		const aiScene *scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Quality);
		for (int i=0; i<scene->mNumMeshes; i++) { // For each mesh in the model



			j7Mesh mesh;
			for (int j=0; j<scene->mMeshes[i]->mNumFaces; j++) { //for each face in the mesh
				for (int k=0; k<3; k++) { // For each vertex in the face
					mesh.vertices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[k]); // Push vertex
				    mesh.normals.push_back(scene->mMeshes[i].mNormals[k]); // Push normal
					for (int k=0; k<2; k++) mesh.textureCoords.push_back(scene->mMeshes[i]->mTextureCoords[k]); // Push each uv coord
				}
			}
			meshes.push_back(mesh);
		}

	}

};
*/
#endif