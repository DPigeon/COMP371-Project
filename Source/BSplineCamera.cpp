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
}

void BSplineCamera::Update(float dt)
{
    mLookAt = mpSpline->GetTangent(mSplineParameterT);

    //this is world units per variation of parameter t
    float tangentMagnitude = length(mLookAt);
    mLookAt = normalize(mLookAt);

    mSplineParameterT += dt;
    mPosition = mpSpline->GetPosition(mSplineParameterT);
	cout<< "X: " << mPosition.x << " Y:" << mPosition.y << " Z:" << mPosition.z << endl;

	ofstream extrapolatePoints;
	extrapolatePoints.open("SplinePoints.txt", ios::app); // App for append at end of file
	extrapolatePoints <<"sPoint = "<< mPosition.x << " " << mPosition.y << " " << mPosition.z;
	extrapolatePoints << endl;
	extrapolatePoints.close();
}

glm::mat4 BSplineCamera::GetViewMatrix() const
{
    return glm::lookAt(mPosition, mPosition + mLookAt, mUp);
}
