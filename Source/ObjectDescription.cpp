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

void ObjectDescription::RayPickObject(mat4 viewMatrix, vec3 planetPosition, vec3 planetScaling) {
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
		vec3 normalizedWorldRay = normalize(WorldRay); // Distance of the point from the origin

		// Next step: Use the world ray traced to point to objects now (planets)

		// planet->GetScaling / 2 is radius and planet->GetPosition is position
		float planetRadius = abs(planetPosition.x - planetScaling.x * planetPosition.x) / 2;
		float intersectionPoint = intersectRayPlanetPoint(normalizedWorldRay, planetPosition, planetRadius);

		// Recall that a point P given on a ray is P = O + tD where P is the point, O origin, t intersection point & D is the ray direction vector
		vec3 positionWorldSpace = intersectionPoint * normalizedWorldRay;
		cout << positionWorldSpace.x << endl;
	}
}

float ObjectDescription::intersectRayPlanetPoint(vec3 worldRay, vec3 planetPosition, float planetRadius) {
	// Quadratic equation of the form t^2 + 2Bt + C = 0
	// Two intersection roots are t1 = -B - sqrt(B^2 - 4C), t2 = -B + sqrt(B^2 - 4C)
	// Origin is at (0, 0, 0) since we normalized the world ray vector already

	float tIntersectPoint1 = 0.0f;
	float tIntersectPoint2 = 0.0f;
	float b = (worldRay.x * (-planetPosition.x) + worldRay.y * (-planetPosition.y) + worldRay.z * (-planetPosition.z));
	float c = (pow((-planetPosition.x), 2) + pow((-planetPosition.y), 2) + pow((planetPosition.z), 2) - pow((planetRadius), 2));

	tIntersectPoint1 = -b - sqrt(pow(b, 2) - 4 * c);
	tIntersectPoint2 = -b + sqrt(pow(b, 2) - 4 * c);

	// Smallest positive t value gives nearest point of intersection
	if (tIntersectPoint1 > 0 && tIntersectPoint1 < tIntersectPoint2)
		return tIntersectPoint1;
	else
		return tIntersectPoint2;
}

ObjectDescription::~ObjectDescription() {
}