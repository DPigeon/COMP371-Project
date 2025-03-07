//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 14/1/15
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#pragma once

#include <vector>
#include <glm/glm.hpp>

class Camera
{
public:
	Camera();
	virtual ~Camera();

	virtual void Update(float dt) = 0;

	virtual glm::mat4 GetViewMatrix() const = 0;
	virtual glm::mat4 GetProjectionMatrix() const;
	glm::mat4 GetViewProjectionMatrix() const;
    glm::vec3 GetPosition() const { return mPosition; }
    glm::vec3 GetLookAt() const { return mLookAt; }
    void SetPosition(glm::vec3 position);
    void SetLookAt(glm::vec3 );
    glm::vec3 mPosition;
    glm::vec3 mLookAt;

private:

};
