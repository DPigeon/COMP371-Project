//
//  ObjectDescription.h
//  Area51
//
//  Created by David-Etienne Pigeon on 2019-08-09.
//  Copyright © 2019 Concordia. All rights reserved.
//

#include "ObjectDescription.h"
#include "EventManager.h"
#include "World.h"

using namespace glm;

ObjectDescription::ObjectDescription() {
}

bool ObjectDescription::RayPickObject(mat4 projectionMatrix, mat4 viewMatrix, vec3 cameraPosition, vec3 planetPosition, float radius) {
	if (glfwGetMouseButton(EventManager::GetWindow(), GLFW_MOUSE_BUTTON_LEFT)) {
		// Was almost done with this feature but could not finish on time.
		World::GetInstance()->SetPlanetClicked("Earth"); // For now, if you click anywhere you get a random planet name

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
		vec4 ClipRay = vec4(NCDRay.x, NCDRay.y, -1.0f, 1.0f);

		// View Space
		vec4 ViewRay = inverse(projectionMatrix) * ClipRay;
		ViewRay = vec4(ViewRay.x, ViewRay.y, 1.0f, 0.0f);

		// World Space
		vec3 WorldRay = vec3(inverse(viewMatrix) * ViewRay);
		vec3 normalizedWorldRay = WorldRay; // Distance of the point from the origin

		// Sun always at the origin so we can easily find it with rays
		// Now we gotta find other planets and randomly generate names for them

		float intersectionPoint = intersectRayPlanetPoint(normalizedWorldRay, cameraPosition, planetPosition, radius); // t

		// Recall that a point P given on a ray is P = O + tD where P is the point, O origin is the camera position, t intersection point & D is the ray direction vector
		vec3 positionWorldSpace = cameraPosition + intersectionPoint * normalizedWorldRay; // P 
		
		if (length(positionWorldSpace) == intersectionPoint) // If ray touches the point
			return true;
		else
			return false;
	}
}

float ObjectDescription::intersectRayPlanetPoint(vec3 worldRay, vec3 cameraPosition, vec3 planetPosition, float planetRadius) {
	// Quadratic equation of the form t^2 + 2Bt + C = 0
	// Two intersection roots are t1 = -B - sqrt(B^2 - 4C), t2 = -B + sqrt(B^2 - 4C)
	// Origin is the camera position

	float tIntersectPoint1 = 0.0f;
	float tIntersectPoint2 = 0.0f;
	float b = (worldRay.x * (cameraPosition.x - planetPosition.x) + worldRay.y * (cameraPosition.y - planetPosition.y) + worldRay.z * (cameraPosition.z - planetPosition.z));
	float c = pow((cameraPosition.x - planetPosition.x), 2) + pow((cameraPosition.y - planetPosition.y), 2) + pow((cameraPosition.z - planetPosition.z), 2) - pow((planetRadius), 2);

	float coefficient = pow(b, 2) - 4 * c;

	if (coefficient > 0) {
		tIntersectPoint1 = -b - sqrt(coefficient);
		tIntersectPoint2 = -b + sqrt(coefficient);
	}

	// Smallest positive t value gives nearest point of intersection
	if (tIntersectPoint1 > 0 && tIntersectPoint1 < tIntersectPoint2)
		return tIntersectPoint1;
	else
		return tIntersectPoint2;
}

ObjectDescription::~ObjectDescription() {
}