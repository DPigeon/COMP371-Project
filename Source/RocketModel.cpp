//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 14/1/15
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#include "World.h"
#include "Renderer.h"
#include "RocketModel.h"
#include "Camera.h"
#include "TextureLoader.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

RocketModel::RocketModel() : Model()
{
        //Setup models
    #if defined(PLATFORM_OSX)
        std::string rocketPath = "/Users/michel/disposable/COMP371-Project/Assets/Models/rocket.obj";
    #else
        std::string rocketPath = "../Assets/Models/rocket.obj";
    
    #endif

    int rocketVertices;
    GLuint rocketVAO = World::GetInstance()->setupModelEBO(rocketPath , rocketVertices ); //Only one letter to
    
    activeVAOVertices = rocketVertices;
    activeVAO = rocketVAO;
}

RocketModel::RocketModel(int texture) : Model()
{
    //Setup models
#if defined(PLATFORM_OSX)
    std::string rocketPath = "/Users/michel/disposable/COMP371-Project/Assets/Models/rocket.obj";
#else
    std::string rocketPath = "../Assets/Models/rocket.obj";
#endif
    
    int rocketVertices;
    GLuint rocketVAO = World::GetInstance()->setupModelEBO(rocketPath , rocketVertices ); //Only one letter to
    
    glBindTexture(GL_TEXTURE_2D, texture);
    activeVAOVertices = rocketVertices;
    activeVAO = rocketVAO;
}

void RocketModel::Update(float dt)
{
    Model::Update(dt);
}

RocketModel::~RocketModel()
{
//    // Free the GPU from the Vertex Buffer
//    glDeleteBuffers(1, &mVBO);
//    glDeleteVertexArrays(1, &mVAO);
}

bool RocketModel::ParseLine(const std::vector<ci_string> &token)
{
    if (token.empty())
    {
        return true;
    }
    else
    {
        return Model::ParseLine(token);
    }
}


void RocketModel::Draw()
{
    // Draw the Vertex Buffer
    // Note this draws a Planet
    // The Model View Projection transforms are computed in the Vertex Shader
    //Draw the stored vertex objects
    glBindVertexArray(activeVAO);

    GLuint WorldMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldTransform");
    GLuint ViewMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewTransform");
    GLuint camPos = glGetUniformLocation(Renderer::GetShaderProgramID(), "camPos");
    
    vec3 lookat = vec3(transpose(World::GetInstance()->GetCurrentCamera()->GetViewMatrix())[2]);
    
    vec2 sphericalCoordinates = ExtractRotationMatrixFromLookat(lookat);
    
    mat4 RocketWorldMatrix(1.0f);
    
    mat4 RocketViewMatrix = translate(mat4(1.0f), vec3(0.0f,0.0f,-1.0f)) *
    glm::rotate(mat4(1.0f), radians(180.0f), vec3(0.0f, 1.0f, 0.0f)) *
    glm::rotate(mat4(1.0f), radians(sphericalCoordinates[0]), vec3(1.0f,0.0f,1.0f)) *
    glm::rotate(mat4(1.0f), radians(sphericalCoordinates[1]), vec3(0.0f,1.0f,0.0f)) *
    scale(mat4(1.0f), vec3(0.1f, 0.1f, 0.1f));
    
    glUniformMatrix4fv(WorldMatrixLocation, 1, GL_FALSE, &RocketWorldMatrix[0][0]);
    
    glUniformMatrix4fv(ViewMatrixLocation, 1, GL_FALSE, &(scale(GetWorldMatrix() * RocketViewMatrix,vec3(1.0f)))[0][0]);
    
    glUniformMatrix4fv(camPos, 1, GL_FALSE, &vec3(0.0f,0.0f,-1.0f)[0]);
    
    setTextureRotation(0.0f);
    setPointSize(spritewidth);
    
    glDrawElements(GL_TRIANGLES, activeVAOVertices, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
}

vec2 ExtractRotationMatrixFromLookat(glm::vec3 lookat){
    float ltheta = atan(lookat.z,lookat.x);
    float ltopDivisor = sqrt(pow(lookat.x,2) + pow(lookat.z,2));
    float lphi = atan(ltopDivisor,lookat.y);
    return vec2(ltheta, lphi);
}

void setTextureRotation( float textureRotationAngle)
{
    glUseProgram(Renderer::GetShaderProgramID());
    GLuint textureRotationLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "textureRotation");
    glm::mat4 rotationMatrix(1.0f);

     rotationMatrix = glm::translate(rotationMatrix, glm::vec3(0.5f, 0.5f, 0.0f));  //center texture at origin
     rotationMatrix = glm::rotate(rotationMatrix, glm::radians(textureRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));  //rotate about origin
     rotationMatrix = glm::translate(rotationMatrix, glm::vec3(-0.5f, -0.5f, 0.0f)); //recenter texture at 0.5, 0.5
    
    glUniformMatrix4fv(textureRotationLocation, 1, GL_FALSE, &rotationMatrix[0][0]);
}

void setPointSize( float pointSize)
{
    glUseProgram(Renderer::GetShaderProgramID());
    GLuint pointSizeLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "pointSize");
    glUniform1f(pointSizeLocation, pointSize);
}
