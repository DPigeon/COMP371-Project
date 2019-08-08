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
#include "ParsingHelper.h"

#include "StaticCamera.h"
#include "FirstPersonCamera.h"


#include "CubeModel.h"
#include "SphereModel.h"
#include "Animation.h"
#include <GLFW/glfw3.h>
#include "EventManager.h"

#include "BSpline.h"
#include "BSplineCamera.h"
#include "Skybox.h"

using namespace std;
using namespace glm;

World* World::instance;
Skybox skybox;

// Light Coefficients
const vec3 lightColor(1.0f, 1.0f, 1.0f);
const float lightKc = 0.05f;
const float lightKl = 0.02f;
const float lightKq = 0.002f;
const vec4 lightPosition(3.0f, 0.0f, 20.0f, 1.0f);

// TODO: These should be parameters set in the menu
const int NUMBER_OF_PLANETS = 10;
const int PLANET_SCALING_MIN_SIZE = 4.0f;
const int PLANET_SCALING_MAX_SIZE = 8.0f;

World::World()
{
    instance = this;
    
    // Setup Camera
    mCamera.push_back(new FirstPersonCamera(vec3(3.0f, 5.0f, 20.0f)));	
    mCamera.push_back(new StaticCamera(vec3(3.0f, 30.0f, 5.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
    mCamera.push_back(new StaticCamera(vec3(0.5f,  0.5f, 5.0f), vec3(0.0f, 0.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
    mCurrentCamera = 0;

	std::vector<std::string> skyboxFaces;
	// MUST BE IN THIS ORDER: RIGHT LEFT UP DOWN BACK FRONT
#if defined(PLATFORM_OSX)
    skyboxFaces.push_back("Textures/Skybox/starfield_rt.tga");
    skyboxFaces.push_back("Textures/Skybox/starfield_lf.tga");
    skyboxFaces.push_back("Textures/Skybox/starfield_up.tga");
    skyboxFaces.push_back("Textures/Skybox/starfield_dn.tga");
    skyboxFaces.push_back("Textures/Skybox/starfield_bk.tga");
    skyboxFaces.push_back("Textures/Skybox/starfield_ft.tga");
#else
    skyboxFaces.push_back("../Assets/Textures/Skybox/starfield_rt.tga");
    skyboxFaces.push_back("../Assets/Textures/Skybox/starfield_lf.tga");
    skyboxFaces.push_back("../Assets/Textures/Skybox/starfield_up.tga");
    skyboxFaces.push_back("../Assets/Textures/Skybox/starfield_dn.tga");
    skyboxFaces.push_back("../Assets/Textures/Skybox/starfield_bk.tga");
    skyboxFaces.push_back("../Assets/Textures/Skybox/starfield_ft.tga");
#endif


	skybox = Skybox(skyboxFaces);
}

World::~World()
{
    // Models
    for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
    {
        delete *it;
    }
    
    mModel.clear();
    
    for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
    {
        delete *it;
    }
    
    mAnimation.clear();
    
    for (vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
    {
        delete *it;
    }
    
    mAnimationKey.clear();
    
    // Camera
    for (vector<Camera*>::iterator it = mCamera.begin(); it < mCamera.end(); ++it)
    {
        delete *it;
    }
    mCamera.clear();
}

World* World::GetInstance()
{
    return instance;
}

void World::Update(float dt)
{
    // User Inputs
    // 0 1 2 to change the Camera
    if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_1 ) == GLFW_PRESS)
    {
        mCurrentCamera = 0;
    }
    else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_2 ) == GLFW_PRESS)
    {
        if (mCamera.size() > 1)
        {
            mCurrentCamera = 1;
        }
    }
    else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_3 ) == GLFW_PRESS)
    {
        if (mCamera.size() > 2)
        {
            mCurrentCamera = 2;
        }
		Renderer::SetShader(SHADER_PHONG);
    }
    
    // Spacebar to change the shader
    if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_0 ) == GLFW_PRESS)
    {
        Renderer::SetShader(SHADER_PHONG);
    }
    else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_9 ) == GLFW_PRESS)
    {
        Renderer::SetShader(SHADER_SOLID_COLOR);
    }
    
    // Update animation and keys
    for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
    {
        (*it)->Update(dt);
    }
    
    for (vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
    {
        (*it)->Update(dt);
    }

    mCamera[mCurrentCamera]->Update(dt);
    
    // Update models
    for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
    {
        (*it)->Update(dt);
    }
    
}

void World::Draw()
{
    Renderer::BeginFrame();
	
	skybox.Draw();
  
    glUseProgram(Renderer::GetShaderProgramID());
    
    // Everything we need to send to the GPU
    
    GLuint WorldMatrixID = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldTransform");
    GLuint ViewMatrixID = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewTransform");
    GLuint ProjMatrixID = glGetUniformLocation(Renderer::GetShaderProgramID(), "ProjectionTransform");
    GLuint ViewProjMatrixID = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewProjectionTransform");
    
    // Get a handle for Light Attributes uniform
    GLuint LightPositionID = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition");
    GLuint LightColorID = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor");
    GLuint LightAttenuationID = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation");
    
    GLuint MaterialID = glGetUniformLocation(Renderer::GetShaderProgramID(), "materialCoefficients");
    
    // Draw the Vertex Buffer
    // Note this draws a unit Sphere
    // The Model View Projection transforms are computed in the Vertex Shader
    
    // Set shader constants
    glUniform4f(LightPositionID, lightPosition.x, lightPosition.y, lightPosition.z, lightPosition.w);
    glUniform3f(LightColorID, lightColor.r, lightColor.g, lightColor.b);
    glUniform3f(LightAttenuationID, lightKc, lightKl, lightKq);
    
    // Send the view projection constants to the shader
    mat4 View = mCamera[mCurrentCamera]->GetViewMatrix();
    glUniformMatrix4fv(ViewMatrixID,  1, GL_FALSE,  &View[0][0]);
    
    mat4 Projection = mCamera[mCurrentCamera]->GetProjectionMatrix();
    glUniformMatrix4fv(ProjMatrixID,  1, GL_FALSE, &Projection[0][0]);
    
    mat4 ViewProjection = mCamera[mCurrentCamera]->GetViewProjectionMatrix();
    glUniformMatrix4fv(ViewProjMatrixID,  1, GL_FALSE, &ViewProjection[0][0]);
    
    // Draw models
    for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
    {
        if ((*it)->GetMaterialCoefficients().length() > 0){
            MaterialID = glGetUniformLocation(Renderer::GetShaderProgramID(), "materialCoefficients");
            
            glUniformMatrix4fv(WorldMatrixID, 1, GL_FALSE, &((*it)->GetWorldMatrix())[0][0]);
            float ka = 0.9f;
            float kd = 0.5f;
            float ks = 1.0f;
            float n = 50.0f;
            
            glUniform4f(MaterialID, ka, kd, ks, n);
        }
        (*it)->Draw();
    }
    
    unsigned int prevShader = Renderer::GetCurrentShader();
	Renderer::SetShader(SHADER_PHONG);
    glUseProgram(Renderer::GetShaderProgramID());
    
    //Draw the BSpline between all the planets here
	for (vector<BSpline*>::iterator it = mSpline.begin(); it < mSpline.end(); ++it) {
		MaterialID = glGetUniformLocation(Renderer::GetShaderProgramID(), "materialCoefficients");

		glUniformMatrix4fv(WorldMatrixID, 1, GL_FALSE, &((*it)->GetWorldMatrix())[0][0]);
		float ka = 0.9f;
		float kd = 0.5f;
		float ks = 1.0f;
		float n = 50.0f;

		glUniform4f(MaterialID, ka, kd, ks, n);
		(*it)->ConstructTracks(mSplineCamera.front()->GetExtrapolatedPoints());
	}

    Renderer::CheckForErrors();
    
    // Restore previous shader
    Renderer::SetShader((ShaderType) prevShader);
    
    Renderer::EndFrame();
}

void World::LoadScene(const char * scene_path)
{
    // Using case-insensitive strings and streams for easier parsing
    ci_ifstream input;
    input.open(scene_path, ios::in);
    
    // Invalid file
    if(input.fail() )
    {
        fprintf(stderr, "Error loading file: %s\n", scene_path);
        getchar();
        exit(-1);
    }
    
    ci_string item;
    while( std::getline( input, item, '[' ) )
    {
        ci_istringstream iss( item );
        
        ci_string result;
        if( std::getline( iss, result, ']') )
        {
            if( result == "cube" )
            {
                // Box attributes
                CubeModel* cube = new CubeModel();
                cube->Load(iss);
                mModel.push_back(cube);
            }
            else if( result == "sphere" )
            {
                PlanetModel* sphere = new PlanetModel();
                sphere->Load(iss);
                mModel.push_back(sphere);
            }
            else if ( result == "animationkey" )
            {
                AnimationKey* key = new AnimationKey();
                key->Load(iss);
                mAnimationKey.push_back(key);
            }
            else if (result == "animation")
            {
                Animation* anim = new Animation();
                anim->Load(iss);
                mAnimation.push_back(anim);
            }
			else if (result == "spline")
			{
				BSpline* planetTour = new BSpline();
				std::vector<Model*> planets = generatePlanets();
				mModel.insert(mModel.begin(), planets.begin(), planets.end());

				for (std::vector<Model*>::iterator it = planets.begin(); it < planets.end(); ++it) {
                    float scaling = glm::length((*it)->GetScaling()); // This is also the new radius since our initial radius is always 1.0f
                    float buffer = 0.1f; // Make sure we are half a planets initial diameter away
                    glm::vec3 newPosition  = randomSphericalCoordinatesToCartesian(scaling + buffer, (*it)->GetPosition());
                    planetTour->AddControlPoint(newPosition);
				}

				planetTour->CreateVertexBuffer();

				// FIXME: This is hardcoded: replace last camera with spline camera
				mSpline.push_back(planetTour);
				mCamera.pop_back();

				// Add camera to traverse the spline
				BSplineCamera* SplineCamera = new BSplineCamera(planetTour, 1.0f); // 10.0 before
				mCamera.push_back(SplineCamera);
				mSplineCamera.push_back(SplineCamera);
			}
            else if ( result.empty() == false && result[0] == '#')
            {
                // this is a comment line
            }
        }
    }
    input.close();
    
    // Set Animation vertex buffers
    for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
    {
        // Draw model
        (*it)->CreateVertexBuffer();
    }
}

glm::vec3 randomSphericalCoordinatesToCartesian(float radius, glm::vec3 initialCenter){
    float theta = randomFloat(0, 180.0f);
    float phi = randomFloat(0, 360.0f);
    float x = radius * glm::cos(phi) * glm::sin(theta) + initialCenter.x;
    float y = radius * glm::sin(phi) * glm::cos(theta) + initialCenter.y;
    float z = radius * glm::cos(theta) + initialCenter.z;
    return glm::vec3(x, y, z);
}

std::vector<Model*> World::generatePlanets(){
    std::vector<Model*> planetList;
    std::vector<vec3> planetPositions;

    for (int i = 0; i < NUMBER_OF_PLANETS; i++) {
        PlanetModel* randomPlanet = new PlanetModel();
        
        // Position
        vec3 planetRandomPoint;
        do {
            planetRandomPoint = vec3(randomFloat(0, 100.0f), randomFloat(10.0f, 100.0f), randomFloat(0.0f, 100.0f));
        } while(!planetHasSpace(planetRandomPoint, planetPositions));
        planetPositions.push_back(planetRandomPoint);
        randomPlanet->SetPosition(planetRandomPoint);
        float planetScalingConstant = randomFloat(PLANET_SCALING_MIN_SIZE, PLANET_SCALING_MAX_SIZE);
        randomPlanet->SetScaling(vec3(planetScalingConstant, planetScalingConstant, planetScalingConstant));
        
        // Color
        float red = randomFloat(0.0f, 1.0f);
        float green = randomFloat(0.0f, 1.0f);
        float blue = randomFloat(0.0f, 1.0f);
        randomPlanet->SetColor(vec3(red, green, blue));
        
        planetList.push_back(randomPlanet);
    }
    
    // Sorts the planets by their position vector magnitude
    std::sort(planetList.begin(), planetList.end(), [ ]( const Model* p1, const Model* p2) {
        return glm::length(p1->GetPosition()) < glm::length(p2->GetPosition());
    });
    
    return planetList;
}

bool World::planetHasSpace(vec3 planetRandomPoint, std::vector<vec3> planetPositions) {
    for (auto position : planetPositions) {
        // The planets have to have at least the max size of a planet in between them
        if (glm::distance(planetRandomPoint, position) < PLANET_SCALING_MAX_SIZE * 2.0) {
            return false;
        }
    }
    return true;
}

float randomFloat(float min, float max)
{
    // this  function assumes max > min, you may want
    // more robust error checking for a non-debug build
    assert(max > min);
    float random = ((float) rand()) / (float) RAND_MAX;
    
    // generate (in your case) a float between 0 and (4.5-.78)
    // then add .78, giving you a float between .78 and 4.5
    float range = max - min;
    return (random*range) + min;
}

Animation* World::FindAnimation(ci_string animName)
{
    for(std::vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
    {
        if((*it)->GetName() == animName)
        {
            return *it;
        }
    }
    return nullptr;
}

AnimationKey* World::FindAnimationKey(ci_string keyName)
{
    for(std::vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
    {
        if((*it)->GetName() == keyName)
        {
            return *it;
        }
    }
    return nullptr;
}

const Camera* World::GetCurrentCamera() const
{
    return mCamera[mCurrentCamera];
}
