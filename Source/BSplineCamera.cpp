//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 28/1/15
//
// Copyright (c) 2014-2015 Concordia University. All rights reserved.
//

#include "BSplineCamera.h"
#include "EventManager.h"
#include "World.h"
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>
#include <algorithm>

using namespace glm;
using namespace std;

BSplineCamera::BSplineCamera(BSpline *spline, float speed)
    : Camera(), mpSpline(spline), mSpeed(speed), mSplineParameterT(0.0f)
{
    assert(spline != nullptr);
    mPosition = mpSpline->GetPosition(mSplineParameterT);

    mLookAt = vec3(0.0f, 0.0f, -1.0f);
    mUp = vec3(0.0f, 1.0f, 0.0f);
}

BSplineCamera::~BSplineCamera()
{
	// Delete Spline points
	mExtrapolatedPoints.clear();
}

void BSplineCamera::Update(float dt)
{
    mLookAt = mpSpline->GetTangent(mSplineParameterT);

    //this is world units per variation of parameter t
    float tangentMagnitude = length(mLookAt);
    mLookAt = normalize(mLookAt);

    mSplineParameterT += dt;
    mPosition = mpSpline->GetPosition(mSplineParameterT);

	ExtrapolatePoints(mPosition);
}

void BSplineCamera::ExtrapolatePoints(vec3 mPosition) {
	/* 
     * Will be done on loading screen at beginning (need a way to traverse the points once at beginning of the tour)
	 * Then we look at first point [0] and compare it with all points to find the same one (means we found the end of spline)
	 */

	if (!ComparePoints(mExtrapolatedPoints)) {
		mExtrapolatedPoints.push_back(mPosition);

		/* Used to extrapolate points in a file to test, uncomment if needed */
		/*ofstream extrapolatePoints;
		extrapolatePoints.open("SplinePoints.txt", ios::app); // App for append at end of file
		extrapolatePoints <<"sPoint = "<< mPosition.x << " " << mPosition.y << " " << mPosition.z;
		extrapolatePoints << endl;
		extrapolatePoints.close();*/
	}

	//cout<< "X: " << mPosition.x << " Y:" << mPosition.y << " Z:" << mPosition.z << endl;
}

bool BSplineCamera::ComparePoints(vector<vec3> points) {
	int skipPoints = 500; // We skip n anout of points because we know that the smallest distance is not at the beginning
	if (!points.empty() && points.size() > skipPoints) {
		vec3 initialPoint = points[0];
		vec3 nextPoint = points[points.size() - 1];

		if (!GetSmallestDistance(initialPoint, nextPoint)) {
			return false; // Continue
		}
		else {
			return true; // Stop
		} 
	}
	return false; // Continue
}

// Error that we have to look into: we sometimes get GL_OUT_OF_MEMORY so we should decrease the randomFloat of points so that we do not leak memory
bool BSplineCamera::GetSmallestDistance(vec3 point, vec3 nextPoint) { // Look if the smallest distance is the closest to a set precision
	float precision = 0.1f; // This is enough to see the end point
	float distanceX = point.x - nextPoint.x;
	float distanceY = point.y - nextPoint.y;
	float distanceZ = point.z - nextPoint.z;
	if (abs(distanceX) <= precision && abs(distanceY) <= precision && abs(distanceZ) <= precision)  
		return true;
	return false;
}

vector<vec3> BSplineCamera::getExtrapolatedPoints() {
	return mExtrapolatedPoints;
}

glm::mat4 BSplineCamera::GetViewMatrix() const
{
    return glm::lookAt(mPosition, mPosition + mLookAt, mUp);
}
