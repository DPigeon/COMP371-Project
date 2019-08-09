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

#include "ParticleDescriptor.h"
#include "ParticleEmitter.h"
#include "ParticleSystem.h"

#include "Star.h"
#include "TextureLoader.h"

using namespace std;
using namespace glm;

World* World::instance;
Star* star;
Skybox skybox;


// TODO: These should be parameters set in the menu
const int NUMBER_OF_PLANETS = 10;
const int PLANET_GENERATE_MAX_RETRIES = 5;
const float WORLD_LENGTH = 100.0f;
const float PLANET_DISTANCE_RATIO = 4.0f;
const float PLANET_SCALING_MIN_SIZE = 4.0f;
const float PLANET_SCALING_MAX_SIZE = 8.0f;

// Light Coefficients
const vec3 lightColor(1.0f, 1.0f, 1.0f);
const float lightKc = 0.05f;
const float lightKl = 0.02f;
const float lightKq = 0.002f;

// Light comes from the sun position and goes in any direction
const vec4 lightPosition(WORLD_LENGTH/2, WORLD_LENGTH/2, WORLD_LENGTH/2, 1.0f);

World::World()
{
    instance = this;
    isLoading = true; // Initialize loading state
    
    // Setup Camera
    mCamera.push_back(new FirstPersonCamera(vec3(3.0f, 5.0f, 20.0f)));	
    mCamera.push_back(new StaticCamera(vec3(3.0f, 30.0f, 5.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
    mCamera.push_back(new StaticCamera(vec3(0.5f,  0.5f, 5.0f), vec3(0.0f, 0.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));

    mCurrentCamera = 2; // Putting this as the current camera so that we load splines automatically

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
    
    int size;
#if defined(PLATFORM_OSX)
    //    int billboardTextureID = TextureLoader::LoadTexture("/Users/kevinluu/Google Drive/Concordia/Semester 7 - Summer 2019/COMP 371/Assignment/A1/Framework/Assets/Textures/BillboardTest.bmp");
    int billboardTextureID = TextureLoader::LoadTexture("Textures/BillboardTest.bmp", size);
#else
    //    int billboardTextureID = TextureLoader::LoadTexture("../Assets/Textures/BillboardTest.bmp");
    int billboardTextureID = TextureLoader::LoadTexture("../Assets/Textures/Particle.png");
#endif
    assert(billboardTextureID != 0);
    
    Renderer::SetShader(TEXTURE);
    glUseProgram(Renderer::GetShaderProgramID());
    mpBillboardList = new BillboardList(2048, billboardTextureID);
    
//         Billboard *b = new Billboard();
//         b->size  = glm::vec2(2.0, 2.0);
//         b->position = glm::vec3(0.0, 3.0, 0.0);
//         b->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
//         b->angle = 100.f;
//    
//         Billboard *b2 = new Billboard();
//         b2->size  = glm::vec2(2.0, 2.0);
//         b2->position = glm::vec3(0.0, 3.0, 1.0);
//         b2->color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
//    
//         mpBillboardList->AddBillboard(b);
//         mpBillboardList->AddBillboard(b2);
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
    else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_4 ) == GLFW_PRESS)
    {
        if (mCamera.size() > 3)
        {
            mCurrentCamera = 3;
        }
        Renderer::SetShader(SHADER_STARS);
    }
    else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_5 ) == GLFW_PRESS)
    {
        if (mCamera.size() > 4)
        {
            mCurrentCamera = 4;
        }
        Renderer::SetShader(TEXTURE);
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
    
    // Update billboards
    
    for (vector<ParticleSystem*>::iterator it = mParticleSystemList.begin(); it != mParticleSystemList.end(); ++it)
    {
        (*it)->Update(dt);
    }
    
    mpBillboardList->Update(dt);
    
}

void World::Draw()
{
    
    Renderer::BeginFrame();
	
	skybox.Draw();
  
    glUseProgram(Renderer::GetShaderProgramID());
    
    unsigned int prevShader = Renderer::GetCurrentShader();

    // Everything we need to send to the GPU
    
	GLuint VPMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewProjectionTransform");
	mat4 VP = mCamera[mCurrentCamera]->GetViewProjectionMatrix();

    GLuint WorldMatrixID = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldTransform");
    GLuint ViewMatrixID = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewTransform");
    GLuint ProjMatrixID = glGetUniformLocation(Renderer::GetShaderProgramID(), "ProjectionTransform");
    GLuint ViewProjMatrixID = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewProjectionTransform");
    
    // Get a handle for Light Attributes uniform
    GLuint LightPositionID = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition");
    GLuint LightColorID = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor");
    GLuint LightAttenuationID = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation");
    
    GLuint MaterialID = glGetUniformLocation(Renderer::GetShaderProgramID(), "materialCoefficients");
    GLuint ModelColorID = glGetUniformLocation(Renderer::GetShaderProgramID(), "modelColor");
    
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

            // Get material coefficients set in the model
            float ka = (*it)->GetMaterialCoefficients().x;
            float kd = (*it)->GetMaterialCoefficients().y;
            float ks = (*it)->GetMaterialCoefficients().z;
            float n = (*it)->GetMaterialCoefficients().w;
            
            glUniform4f(MaterialID, ka, kd, ks, n);
            glUniform3f(ModelColorID, (*it)->GetColor().x, (*it)->GetColor().y, (*it)->GetColor().z);
        }
        (*it)->Draw();
    }

	 // Set Shader for tracks
    prevShader = Renderer::GetCurrentShader();
	Renderer::SetShader(SHADER_TRACKS);
	glUseProgram(Renderer::GetShaderProgramID());

	VPMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewProjectionTransform");
	glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);
    
    //Draw the BSpline between all the planets here
	for (vector<BSpline*>::iterator it = mSpline.begin(); it < mSpline.end(); ++it) {
		MaterialID = glGetUniformLocation(Renderer::GetShaderProgramID(), "materialCoefficients");

		glUniformMatrix4fv(WorldMatrixID, 1, GL_FALSE, &((*it)->GetWorldMatrix())[0][0]);
		float ka = 0.9f;
		float kd = 0.5f;
		float ks = 1.0f;
		float n = 50.0f;
		glUniform4f(MaterialID, ka, kd, ks, n);

        // Set color of spline here
        glUniform3f(ModelColorID, 0.0f, 0.0f, 1.0f);

		(*it)->ConstructTracks(mSplineCamera.front()->GetExtrapolatedPoints());
	}

    Renderer::CheckForErrors();
    
    int spriteWidth;
    
#if defined(PLATFORM_OSX)
    //        int texture_id = TextureLoader::LoadTexture("Textures/BillboardTest.bmp", spriteWidth);
    int texture_id = TextureLoader::LoadTexture("Textures/Stars/shiny_yellow_star-min.png", spriteWidth);
#else
    //    int texture_id = TextureLoader::LoadTexture("../Assets/Textures/BillboardTest.bmp", spriteWidth);
<<<<<<< HEAD
    int texture_id = TextureLoader::LoadTexture("../Textures/Stars/shiny_yellow_star-min.png", spriteWidth);
=======
    int texture_id = TextureLoader::LoadTexture("../Assets/Textures/Stars/shiny_yellow_star-min.png", spriteWidth);
>>>>>>> #1-stars-generator
#endif
    
    star = new Star(texture_id);
    
    Renderer::SetShader(SHADER_STARS);
    glUseProgram(Renderer::GetShaderProgramID());
    
    glEnable(GL_BLEND); //Enable transparency blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Define transparency blending
    glEnable(GL_POINT_SPRITE); //Enable use of point sprites
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE); //Enable changing size of point sprites
    star -> Draw();
    glDisable(GL_BLEND);
   
    Renderer::SetShader((ShaderType) prevShader);

    Renderer::SetShader(TEXTURE);
    glUseProgram(Renderer::GetShaderProgramID());
    glEnable(GL_BLEND); //Enable transparency blending
    mpBillboardList -> Draw();
    glDisable(GL_BLEND);

    
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
            else if (result == "sphere")
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
                mNumberOfPlanetsGenerated = (int)planets.size() - 1; // Remove the sun from the count!!
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
            else if (result == "particledescriptor")
            {
                ParticleDescriptor* psd = new ParticleDescriptor();
                psd->Load(iss);
                AddParticleDescriptor(psd);
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
    
    // Sun is positioned exactly at center
    const vec3 sunPosition = vec3(WORLD_LENGTH/2, WORLD_LENGTH/2, WORLD_LENGTH/2);
    planetPositions.push_back(sunPosition);

    for (int i = 0; i < NUMBER_OF_PLANETS; i++) {
        PlanetModel* randomPlanet = new PlanetModel();
        
        // Position
        vec3 planetRandomPoint;
        int randomTries = 0;
        bool positionIsValid = false;
        do {
            randomTries++;
            planetRandomPoint = vec3(randomFloat(0, WORLD_LENGTH), randomFloat(0.0f, WORLD_LENGTH), randomFloat(0.0f, WORLD_LENGTH));
            positionIsValid = planetHasSpace(planetRandomPoint, planetPositions);
        } while(!positionIsValid && randomTries < PLANET_GENERATE_MAX_RETRIES);
        randomTries = 0;
        
        if (positionIsValid) {
            planetPositions.push_back(planetRandomPoint);
            randomPlanet->SetPosition(planetRandomPoint);
            float planetScalingConstant = randomFloat(PLANET_SCALING_MIN_SIZE, PLANET_SCALING_MAX_SIZE);
            randomPlanet->SetScaling(vec3(planetScalingConstant, planetScalingConstant, planetScalingConstant));
            
            // Color
            float red = randomFloat(0.0f, 1.0f);
            float green = randomFloat(0.0f, 1.0f);
            float blue = randomFloat(0.0f, 1.0f);
            randomPlanet->SetColor(vec3(red, green, blue));
            
            // Ambient set to 0.5 to better see the effect of lighting
            // Shininess is set super high to remove the point light effect
            randomPlanet->SetMaterialCoefficients(vec4(0.5f, 0.5f, 1.0f, 100000.0f));
            
            planetList.push_back(randomPlanet);
        }
    }
  
    PlanetModel* sun = new PlanetModel();
    sun->SetPosition(sunPosition); // Sun placed on origin
    sun->SetScaling(vec3(10.0f, 10.0f, 10.0f));
    sun->SetColor(vec3(0.988f, 0.831f, 0.251f));

    // Sun is unaffected by lighting
    sun->SetMaterialCoefficients(vec4(1.0f, 0.0f, 0.0f, 1.0f));

    planetList.push_back(sun);

    // Sorts the planets by their position vector magnitude
    std::sort(planetList.begin(), planetList.end(), [ ]( const Model* p1, const Model* p2) {
        return glm::length(p1->GetPosition()) < glm::length(p2->GetPosition());
    });
  
    return planetList;
}

bool World::planetHasSpace(vec3 planetRandomPoint, std::vector<vec3> planetPositions) {
    for (auto position : planetPositions) {
        // The planets have to have at least the max size of a planet in between them
        if (glm::distance(planetRandomPoint, position) < PLANET_SCALING_MAX_SIZE * PLANET_DISTANCE_RATIO) {
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


void World::AddBillboard(Billboard* b)
{
    mpBillboardList->AddBillboard(b);
}

void World::RemoveBillboard(Billboard* b)
{
    mpBillboardList->RemoveBillboard(b);
}

void World::AddParticleSystem(ParticleSystem* particleSystem)
{
    mParticleSystemList.push_back(particleSystem);
}

void World::RemoveParticleSystem(ParticleSystem* particleSystem)
{
    vector<ParticleSystem*>::iterator it = std::find(mParticleSystemList.begin(), mParticleSystemList.end(), particleSystem);
    mParticleSystemList.erase(it);
}

void World::AddParticleDescriptor(ParticleDescriptor* particleDescriptor)
{
    mParticleDescriptorList.push_back(particleDescriptor);
}

ParticleDescriptor* World::FindParticleDescriptor(ci_string name)
{
    for(std::vector<ParticleDescriptor*>::iterator it = mParticleDescriptorList.begin(); it < mParticleDescriptorList.end(); ++it)
    {
        if((*it)->GetName() == name)
        {
            return *it;
        }
    }
    return nullptr;
}

void World::SetCurrentCamera(int cameraNumber) {
	mCurrentCamera = cameraNumber;
}

bool World::GetLoadingState() {
	return isLoading;
}

void World::SetLoadingState(bool state) {
	isLoading = state;
}

int World::NumberOfPlanetsToGenerate() {
    return NUMBER_OF_PLANETS;
}
