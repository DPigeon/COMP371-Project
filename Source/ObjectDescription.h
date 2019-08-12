//
//  ObjectDescription.h
//  Area51
//
//  Created by David-Etienne Pigeon on 2019-08-09.
//  Copyright © 2019 Concordia. All rights reserved.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include "Model.h"

using namespace std;
using namespace glm;

class ObjectDescription
{
public:
	ObjectDescription();
	static bool RayPickObject(mat4 viewMatrix, vec3 cameraPosition, vec3 planetPosition, float radius);
	static float intersectRayPlanetPoint(vec3 worldRay, vec3 cameraPosition, vec3 planetPosition, float radius);
	~ObjectDescription();

private:
	vector<Model*> mObjects;
};