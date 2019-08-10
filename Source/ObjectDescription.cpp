//
//  ObjectDescription.h
//  Area51
//
//  Created by David-Etienne Pigeon on 2019-08-09.
//  Copyright © 2019 Concordia. All rights reserved.
//

#include "ObjectDescription.h"
#include "EventManager.h"

using namespace glm;

ObjectDescription::ObjectDescription() {
}

void ObjectDescription::RayPickObject(mat4 viewMatrix) {
	if (glfwGetMouseButton(EventManager::GetWindow(), GLFW_MOUSE_BUTTON_LEFT)) {
		// Go from viewport ---> world space to click on things from screen space ---> world space

		// Viewport coordinates
		float x = EventManager::GetMouseMotionX();
		float y = EventManager::GetMouseMotionY();
		float width = EventManager::GetScreenWidth();
		float height = EventManager::GetScreenHeight();

		// NDC coordinates (-1, 1 in x, y, z, w)
		float ndcX = (2.0f * x) / width - 1.0f;
		float ndcY = 1.0f - (2.0f * y) / height;
		float ndcZ = 1.0f;
		vec3 NCDRay = vec3(ndcX, ndcY, ndcZ);

		// Clip Space
		vec4 ClipRay = vec4(NCDRay.x, NCDRay.y, -1.0, 1.0);

		// View Space
		vec3 WorldRay = vec3(inverse(viewMatrix) * ClipRay);
		vec3 normalizedWorldRay = normalize(WorldRay);
		cout << normalizedWorldRay.x << endl;

		// Next step: Use the world ray traced to point to objects now
	}
}

ObjectDescription::~ObjectDescription() {

}