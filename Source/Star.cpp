//
//  Star.cpp
//  COMP371_Framework
//
//  Created by Kevin Luu on 2019-08-06.
//  Copyright © 2019 Concordia. All rights reserved.
//

#include <stdio.h>
#include "Star.h"
#include <stdlib.h>
#include <math.h>

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler
// GLFW provides a cross-platform interface for creating a graphical context,
// initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>
#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
#include <random>

#include "TextureLoader.h"
#include "Renderer.h"
#include "Camera.h"
#include "World.h"

Star::Star(int texture_id)
{
    glBindTexture(GL_TEXTURE_2D, texture_id);
}
//Creates many random points within a cubic area
GLuint randVAO(int& vertexCount, std::vector<float>& angles)
{
    float areaWidth = 200.0f;
    
    std::vector<glm::vec3> vertPos;
    std::vector<float> vertRot;
    
    std::default_random_engine rando;
    for (int i = 0; i != vertexCount; ++i)
    {
        //Create a random position
        glm::vec3 randomPosition((float)rando(), (float)rando(), (float)rando());
        randomPosition /= (rando.max() / areaWidth);
        randomPosition -= glm::vec3(areaWidth / 2.0f, areaWidth / 2.0f, 0.0f);
        randomPosition.z *= -1.0f;
        
        //Create a random angle
        float randomAngle = rando() / (rando.max() / 360.0f);
        angles.push_back(randomAngle);
        randomAngle = glm::radians(randomAngle);
        
        
        vertPos.push_back(randomPosition);
        vertRot.push_back(randomAngle);
    }
    
    vertexCount = vertPos.size();
    
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO); //Becomes active VAO
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    
    //Vertex VBO setup
    GLuint vertices_VBO;
    glGenBuffers(1, &vertices_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertPos.size() * sizeof(glm::vec3), &vertPos.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    
    //Angle VBO setup
    GLuint angles_VBO;
    glGenBuffers(1, &angles_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, angles_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertRot.size() * sizeof(float), &vertRot.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    
    return VAO;
}


void Star::Draw()
{
    //Draw Stars
    int vCount = 500;
    std::vector<float> angles(100);
    GLuint VAO = randVAO(vCount, angles);
    
    // Draw models
    GLuint StarPointSizeID = glGetUniformLocation(Renderer::GetShaderProgramID(), "StarPointSize");
    glUniform1f(StarPointSizeID, 30.0f);
    setTextureRotation(Renderer::GetShaderProgramID(), 0.0f);
    
    // Everything we need to send to the GPU
    
    GLuint ViewMatrixID = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewTransform");
    GLuint ProjMatrixID = glGetUniformLocation(Renderer::GetShaderProgramID(), "ProjectionTransform");
    GLuint ViewProjMatrixID = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewProjectionTransform");
    
    // Send the view projection constants to the shader
    const Camera* currentCamera = World::GetInstance()->GetCurrentCamera();
    
    // Send the view projection constants to the shader
    glm::mat4 View = currentCamera->GetViewMatrix();
    glUniformMatrix4fv(ViewMatrixID,  1, GL_FALSE,  &View[0][0]);
    
    glm::mat4 Projection = currentCamera->GetProjectionMatrix();
    glUniformMatrix4fv(ProjMatrixID,  1, GL_FALSE, &Projection[0][0]);
    
    glm::mat4 ViewProjection = currentCamera->GetViewProjectionMatrix();
    glUniformMatrix4fv(ViewProjMatrixID,  1, GL_FALSE, &ViewProjection[0][0]);
 
    glEnable( GL_POINT_SMOOTH );
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(VAO);
    for (int i = 0; i < 1000; i++) {
        glDrawArrays( GL_POINTS, i, 1 ); // draw the vertixes
    }
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable( GL_POINT_SMOOTH ); // stop the smoothing to make the points circular
    

    glEnd();
    glFinish();
}

void Star::setTextureRotation(int shaderProgram, float textureRotationAngle)
{
    glUseProgram(shaderProgram);
    GLuint textureRotationLocation = glGetUniformLocation(shaderProgram, "textureRotation");
    glm::mat4 rotationMatrix(1.0f);
    
    rotationMatrix = glm::translate(rotationMatrix, glm::vec3(0.5f, 0.5f, 0.0f));  //center texture at origin
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(textureRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));  //rotate about origin
    rotationMatrix = glm::translate(rotationMatrix, glm::vec3(-0.5f, -0.5f, 0.0f)); //recenter texture at 0.5, 0.5
    
    
    glUniformMatrix4fv(textureRotationLocation, 1, GL_FALSE, &rotationMatrix[0][0]);
}
