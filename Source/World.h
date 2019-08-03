//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 14/1/15
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#pragma once


#include "ParsingHelper.h"
#include <vector>
#include "BSpline.h"

class Camera;
class Model;
class Animation;
class AnimationKey;
class BSpline;
class BSplineCamera;

class World
{
public:
	World();
	~World();
	
    static World* GetInstance();

	void Update(float dt);
	void Draw();

	void LoadScene(const char * scene_path);
    Animation* FindAnimation(ci_string animName);
    AnimationKey* FindAnimationKey(ci_string keyName);

    const Camera* GetCurrentCamera() const;
    
private:
    static World* instance;
    
	std::vector<Model*> mModel;
    std::vector<Animation*> mAnimation;
    std::vector<AnimationKey*> mAnimationKey;
	std::vector<Camera*> mCamera;
    std::vector<BSpline*> mSpline;
	std::vector<BSplineCamera*> mSplineCamera;
    std::vector<Model*> generatePlanets();
	unsigned int mCurrentCamera;
};

float randomFloat(float min, float max);
