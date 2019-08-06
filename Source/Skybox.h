#pragma once

#include <vector>
#include <string>

#include <GL\glew.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "stb_image.h"

#include "Mesh.h"

class Skybox
{
public:
	Skybox();
	Skybox(std::vector<std::string> faceLocations);
	void Draw();
	~Skybox();

private:
	Mesh* skyMesh;

	GLuint textureId;
	glm::mat4 projection, view;
};
