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

bool ObjectDescription::RayPickObject(mat4 viewMatrix, vec3 cameraPosition, vec3 planetPosition, float radius) {
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

		// Sun always at the origin so we can easily find it with rays
		// Now we gotta find other planets and randomly generate names for them

		float intersectionPoint = intersectRayPlanetPoint(normalizedWorldRay, cameraPosition, planetPosition, radius); // t

		// Recall that a point P given on a ray is P = O + tD where P is the point, O origin, t intersection point & D is the ray direction vector
		vec3 positionWorldSpace = intersectionPoint * normalizedWorldRay; // P 
		vec3 edgePoint = vec3(planetPosition.x + radius, planetPosition.y + radius, planetPosition.z + radius); // A Point 
		
		//cout << "P: " << length(positionWorldSpace) << "t: " << intersectionPoint << endl;
		bool f;
		if (length(positionWorldSpace) == intersectionPoint)
			f = true;
		else
			f = false;

		/*ofstream extrapolatePoints;
		extrapolatePoints.open("RayIntersections.txt", ios::app);
		extrapolatePoints << f;
		extrapolatePoints << endl;
		extrapolatePoints.close();*/

		return false;
	}
}

float ObjectDescription::intersectRayPlanetPoint(vec3 worldRay, vec3 cameraPosition, vec3 planetPosition, float planetRadius) {
	// Quadratic equation of the form t^2 + 2Bt + C = 0
	// Two intersection roots are t1 = -B - sqrt(B^2 - 4C), t2 = -B + sqrt(B^2 - 4C)
	// Origin is the camera position

	/*cout << worldRay.y << endl;
	cout << cameraPosition.y << endl;
	cout << planetPosition.y << endl;
	cout << planetRadius << endl;*/

	float tIntersectPoint1 = 0.0f;
	float tIntersectPoint2 = 0.0f;
	float b = (worldRay.x * (cameraPosition.x - planetPosition.x) + worldRay.y * (cameraPosition.y - planetPosition.y) + worldRay.z * (cameraPosition.z - planetPosition.z));
	float c = pow((cameraPosition.x - planetPosition.x), 2) + pow((cameraPosition.y - planetPosition.y), 2) + pow((cameraPosition.z - planetPosition.z), 2) - pow((planetRadius), 2);

	tIntersectPoint1 = -b - sqrt(pow(b, 2) - 4 * c);
	tIntersectPoint2 = -b + sqrt(pow(b, 2) - 4 * c);

	cout <<"b: "<< b <<"c: "<<c<< endl;

	// Smallest positive t value gives nearest point of intersection
	if (tIntersectPoint1 > 0 && tIntersectPoint1 < tIntersectPoint2)
		return tIntersectPoint1;
	else
		return tIntersectPoint2;
}

ObjectDescription::~ObjectDescription() {
}