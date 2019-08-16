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
#include "ObjectDescription.h"
#include "SphereModel.h"
#include "Projectile.h"
#include "RocketModel.h"
#include "Animation.h"
#include <GLFW/glfw3.h>
#include "EventManager.h"

#include "BSpline.h"
#include "BSplineCamera.h"
#include "Skybox.h"

#include "Star.h"
#include "TextureLoader.h"
#include "OBJloader.h"  //For loading .obj files
#include "OBJloaderV2.h"  //For loading .obj files using a polygon list format

#include <glm/gtc/matrix_transform.hpp>
#include "list"

#define PI 3.14159265

#include "list"

#define PI 3.14159265

using namespace std;
using namespace glm;

World* World::instance;
Star* star;
Skybox skybox;
list<Projectile*> projectileList;
RocketModel* rocket;

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

// Negative to ensure light points towards the correct quadrant
// Lights half of the planet towards the sun
const vec4 lightPosition(-10.0f, -10.0f, -10.0f, 1.0f);

GLuint texture_id, texture_id2, texture[2];

World::World()
{
    instance = this;
    applicationState = ApplicationState::LOADING; // Initialize loading state
    
    // Setup Camera
    mCamera.push_back(new FirstPersonCamera(vec3(3.0f, 5.0f, 20.0f)));	
    mCamera.push_back(new StaticCamera(vec3(3.0f, 5.0f, 20.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
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
    
    int spriteWidth;

#if defined(PLATFORM_OSX)
    // int texture_id = TextureLoader::LoadTexture("Textures/BillboardTest.bmp", spriteWidth);
    texture_id = TextureLoader::LoadTexture("Textures/Stars/shiny_yellow_star-min.png", spriteWidth);
    texture_id2 = TextureLoader::LoadTexture("Textures/mat.png", spriteWidth);
#else
    texture_id = TextureLoader::LoadTexture("../Assets/Textures/Stars/shiny_yellow_star-min.png", spriteWidth);
    texture_id2 = TextureLoader::LoadTexture("../Assets/Textures/mat.png", spriteWidth);
#endif
    

	skybox = Skybox(skyboxFaces);
    glGenTextures(2, texture);

    texture[0] = texture_id;
    glBindTexture(GL_TEXTURE_2D, texture[0]);

    texture[1] = texture_id2;
    glBindTexture(GL_TEXTURE_2D, texture[1]);

    star = new Star(texture[0]);
    rocket = new RocketModel(texture[1]);
    

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
		Renderer::SetShader(SHADER_PHONG);
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
        Renderer::SetShader(SHADER_MODEL);
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
    
    Projectile* projectile = new Projectile();

    if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        const float projectileSpeed = 25.0f;
        projectile->SetPosition(mCamera[mCurrentCamera]->GetPosition() + vec3(0.0f, 0.02f, 0.0f)); // adjust to rocket's position
        projectile->SetVelocity(projectileSpeed * mCamera[mCurrentCamera]->GetLookAt()); 
        projectileList.push_back(projectile);
    }

    for (list<Projectile*>::iterator it = projectileList.begin(); it != projectileList.end(); ++it)
    {
        (*it)->Update(dt);
    }

    rocket->Update(dt);
    
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

    // Draw projectiles before other models
    for (list<Projectile*>::iterator it = projectileList.begin(); it != projectileList.end(); ++it)
    {
        glUniform3f(ModelColorID, 0.0f, 0.0f, 1.0f);
        glUniform4f(MaterialID, 1.0f, 0.0f, 0.0f, 1.0f);
        (*it)->SetScaling(vec3(0.05f, 0.05f, 0.05f)); // TO FIX
        (*it)->Draw();
    }

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
    
    Renderer::SetShader(SHADER_STARS);
    glUseProgram(Renderer::GetShaderProgramID());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glEnable(GL_BLEND); //Enable transparency blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Define transparency blending
    glEnable(GL_POINT_SPRITE); //Enable use of point sprites
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE); //Enable changing size of point sprites
    star -> Draw();
    glDisable(GL_BLEND);
    
	for (std::vector<Model*>::iterator it = planets.begin(); it < planets.end(); ++it) {
		if (it == planets.begin()) continue; // Skip the first one (Sun)
		// Will have to look at mCurrentCamera for other modes
		vec3 cameraPosition;
		int currentCamera;
		currentCamera = mCurrentCamera;
		if (currentCamera == 2) // Tracks camera
			cameraPosition = mSplineCamera.front()->GetPosition();
		else // Other cameras
			cameraPosition = GetCurrentCamera()->GetPosition();
		bool clickedPlanet = ObjectDescription::RayPickObject(Projection, View, cameraPosition, (*it)->GetPosition(), (*it)->GetScaling().z);
		if (clickedPlanet)
			planetClickedMessage = "This is a planet !"; // Did not have time to finish with this feature. 
	}

    // Restore previous shader

    Renderer::SetShader(SHADER_MODEL);
    glUseProgram(Renderer::GetShaderProgramID());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glEnable(GL_POINT_SPRITE); //Enable use of point sprites

    rocket->Draw();

    Renderer::SetShader((ShaderType) prevShader);
    glUseProgram(Renderer::GetShaderProgramID());
    Renderer::CheckForErrors();
    
    glUniformMatrix4fv(ViewMatrixID,  1, GL_FALSE,  &View[0][0]);
    
    glUniformMatrix4fv(ProjMatrixID,  1, GL_FALSE, &Projection[0][0]);
    
    glUniformMatrix4fv(ViewProjMatrixID,  1, GL_FALSE, &ViewProjection[0][0]);
    
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
            if( result == "projectile" )
            {
                // Box attributes
                Projectile* projectile = new Projectile();
                projectile->Load(iss);
                mModel.push_back(projectile);
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
				planets = generatePlanets();
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
				BSplineCamera* SplineCamera = new BSplineCamera(planetTour, 0.1f); // 10.0 before
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
        int randomTries = 0;
        bool positionIsValid = false;
        do {
            randomTries++;
            planetRandomPoint = vec3(randomFloat(20.0f, WORLD_LENGTH), randomFloat(20.0f, WORLD_LENGTH), randomFloat(20.0f, WORLD_LENGTH));
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
    sun->SetPosition(vec3(0.0f, 0.0f, 0.0f)); // Sun placed on origin
    sun->SetScaling(vec3(10.0f, 10.0f, 10.0f));
    sun->SetColor(vec3(0.988f, 0.831f, 0.251f));

    // Sun is unaffected by lighting
    sun->SetMaterialCoefficients(vec4(1.0f, 0.0f, 0.0f, 1.0f));

    planetList.push_back(sun);

    // Sort planets by magnitude to start from close to far
    std::sort(planetList.begin(), planetList.end(), [ ]( const Model* p1, const Model* p2) {
        return glm::length(p1->GetPosition()) < glm::length(p2->GetPosition());
    });
    
    // TODO: remove, used for debugging
    std::for_each(planetList.begin(), planetList.end(), [](Model* m){
        std::cout << "vector: (" << m->GetPosition().x << ", " << m->GetPosition().y << ", " << m->GetPosition().z << ")" << std::endl;
    });
    
    // Add all the planets in a temporary list. Skip first 2 planets because we always start with them as our first vector trajectory.
    std::list<Model*> tempPlanetList;
    std::for_each(planetList.begin()+2, planetList.end(), [&tempPlanetList](Model* m){
        tempPlanetList.push_back(m);
    });
    
    std::vector<Model*> sortedPlanets;
    Model* previousPlanet = planetList.at(0);
    Model* currentPlanet = planetList.at(1);
    sortedPlanets.push_back(previousPlanet);
    sortedPlanets.push_back(currentPlanet);
    vec3 previousVector = currentPlanet->GetPosition() - previousPlanet->GetPosition();
    // Add all the planets from temp to the sortedPlanets vector in a way that the trajectory between planets is smoother
    while (tempPlanetList.size() != 0) {
        bool foundMatch = false;
        for (int i=0; i<tempPlanetList.size(); ++i) {
            Model* nextPlanet = *std::next(tempPlanetList.begin(), i);
            if (currentPlanet != nextPlanet) {
                // Basic vector operations to find next trajectory and the degree between previous and next
                vec3 nextVector = nextPlanet->GetPosition() - currentPlanet->GetPosition();
                float thetaBetweenVectors = glm::dot(previousVector, nextVector) / (glm::length(previousVector) * glm::length(nextVector));
                float degreeBetweenVectors = std::acos(thetaBetweenVectors) * 180 / PI;
                
                // The angle between the previous vector trajectory and the next has to be between 55 and 125 degrees
                if (degreeBetweenVectors >= 55 && degreeBetweenVectors <= 125) {
                    sortedPlanets.push_back(nextPlanet);
                    tempPlanetList.remove(nextPlanet);
                    previousVector = nextVector;
                    currentPlanet = nextPlanet;
                    foundMatch = true;
                    break;
                }
            }
        }
        // If no path could be found, just go to the next planet. We could also expand the angles to accept more to find the optimal one, but it's more resource intensive and it's already sub-par.
        if (!foundMatch) {
            std::cout << "not optimal found" << std::endl;
            Model* nextPlanet = *std::next(tempPlanetList.begin(), 0);
            
            vec3 nextVector = nextPlanet->GetPosition() - currentPlanet->GetPosition();

            sortedPlanets.push_back(nextPlanet);
            tempPlanetList.remove(nextPlanet);
            previousVector = nextVector;
            currentPlanet = nextPlanet;
        }
    }
    // TODO: remove, used for debugging
    std::for_each(sortedPlanets.begin(), sortedPlanets.end(), [](Model* m){
        std::cout << "vector: (" << m->GetPosition().x << ", " << m->GetPosition().y << ", " << m->GetPosition().z << ")" << std::endl;
    });
    return sortedPlanets;
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

void World::SetCurrentCamera(int cameraNumber) {
	mCurrentCamera = cameraNumber;
}

ApplicationState World::GetApplicationState() {
	return applicationState;
}

void World::SetApplicationState(ApplicationState state) {
	applicationState = state;
}

string World::GetPlanetClicked() {
	return planetClickedMessage;
}

void World::SetPlanetClicked(string message) {
	planetClickedMessage = message;
}

int World::NumberOfPlanetsToGenerate() {
    return NUMBER_OF_PLANETS;
}

//Sets up a model using an Element Buffer Object to refer to vertex data
GLuint World::setupModelEBO(string path, int& vertexCount)
{
    vector<int> vertexIndices; //The contiguous sets of three indices of vertices, normals and UVs, used to make a triangle
    vector<glm::vec3> vertices;
    vector<glm::vec3> normals;
    vector<glm::vec2> UVs;
    
    //read the vertices from the cube.obj file
    //We won't be needing the normals or UVs for this program
    loadOBJ2(path.c_str(), vertexIndices, vertices, normals, UVs);
    
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO); //Becomes active VAO
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    
    //Vertex VBO setup
    GLuint vertices_VBO;
    glGenBuffers(1, &vertices_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    
    //Normals VBO setup
    GLuint normals_VBO;
    glGenBuffers(1, &normals_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    
    //UVs VBO setup
    GLuint uvs_VBO;
    glGenBuffers(1, &uvs_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
    glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(2);
    
    //EBO setup
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(int), &vertexIndices.front(), GL_STATIC_DRAW);
    
    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
    vertexCount = vertexIndices.size();
    return VAO;
}
