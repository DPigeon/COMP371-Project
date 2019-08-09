//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 14/1/15
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#pragma once

#include "Model.h"
#include <GLFW/glfw3.h>

class RocketModel : public Model
{
public:
	RocketModel();
    virtual ~RocketModel(void);
    
    virtual void Update(float dt);
    virtual void Draw();
    
protected:
    virtual bool ParseLine(const std::vector<ci_string> &token);
    
private:
    // The vertex format could be different for different types of models
    GLuint activeVAO;
    int activeVAOVertices;
    int spritewidth;
};

glm::vec2 ExtractRotationMatrixFromLookat(glm::vec3 lookat);

void setTextureRotation(float textureRotationAngle);
void setPointSize( float pointSize);
