#if 0

class j7Light {
public:


	j7Light(aiVector3D pos, aiVector3D dir, aiColor3D amb, aiColor3D diff, aiColor3D spec) {
		position.push_back(pos.x);
		position.push_back(pos.y);
		position.push_back(pos.z);

		direction.push_back(dir.x);
		direction.push_back(dir.y);
		direction.push_back(dir.z);

		colorAmbient.push_back(amb.r);
		colorAmbient.push_back(amb.g);
		colorAmbient.push_back(amb.b);

		colorDiffuse.push_back(diff.r);
		colorDiffuse.push_back(diff.g);
		colorDiffuse.push_back(diff.b);

		colorSpecular.push_back(spec.r);
		colorSpecular.push_back(spec.g);
		colorSpecular.push_back(spec.b);

		std::cout << "Added new light at (" << position[0] << ',' << position[1] << ',' << position[2] << "), pointing at (" << direction[0] << ',' << direction[1] << ',' << direction[2] << ")\n";
	}

	void draw() {


	// Place it
	//Fixed function OpenGL only supports 8 lights. We're going to need shaders
	/*glLightfv(GL_LIGHT0, GL_AMBIENT, colorAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, colorDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, colorSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, position);*/
	}
private:
	// Coordinates
	std::vector<GLfloat> position;
	std::vector<GLfloat> direction;
	// Colors
	std::vector<GLfloat> colorAmbient;
	std::vector<GLfloat> colorDiffuse;
	std::vector<GLfloat> colorSpecular;
};

class j7Material {
public:
	std::vector<GLuint> diffuse_tex;


private:

};

void drawGround() {
	static bool loaded=false;
	static GLuint concretetex;
	if(!loaded) {
		sf::Image texture;
		texture.loadFromFile("concrete.jpg");
		glGenTextures(1, &concretetex);
        glBindTexture(GL_TEXTURE_2D, concretetex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.getSize().x, texture.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getPixelsPtr());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		loaded=true;
	}
	glBindTexture(GL_TEXTURE_2D, concretetex);
	glBegin(GL_QUADS);
		glTexCoord2f(1,1);
		glVertex3f(-50,-0.01f, 50);
		glTexCoord2f(1,0);
		glVertex3f( 50,-0.01f, 50);
		glTexCoord2f(0,1);
		glVertex3f( 50,-0.01f,-50);
		glTexCoord2f(0,0);
		glVertex3f(-50,-0.01f,-50);
	glEnd();

}

#endif