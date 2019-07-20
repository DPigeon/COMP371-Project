//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 14/1/15
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#include "Animation.h"
#include "Renderer.h"
#include "World.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace glm;
using namespace std;

AnimationKey::AnimationKey() : Model()
{
}

AnimationKey::~AnimationKey()
{
}

void AnimationKey::Update(float dt)
{
	Model::Update(dt);
}

void AnimationKey::Draw()
{
	Model::Draw();
}

bool AnimationKey::ParseLine(const std::vector<ci_string> &token)
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

Animation::Animation()
	: mName(""), mCurrentTime(0.0f), mDuration(0.0f), mVBO(0), mVAO(0)
{
}

Animation::~Animation()
{
}

void Animation::CreateVertexBuffer()
{
	// This is just to display lines between the animation keys
	for (int i = 0; i < (int)mKey.size() - 1; ++i)
	{
		spline.AddControlPoint(mKey[i].GetPosition());
	}
	spline.CreateVertexBuffer();
}

void Animation::Update(float dt)
{
	mCurrentTime += dt;

	while (mCurrentTime > mDuration)
	{
		mCurrentTime -= mDuration;
	}
}

void Animation::Draw()
{
	spline.Draw();
}

void Animation::Load(ci_istringstream &iss)
{
	ci_string line;

	// Parse model line by line
	while (std::getline(iss, line))
	{
		// Splitting line into tokens
		ci_istringstream strstr(line);
		istream_iterator<ci_string, char, ci_char_traits> it(strstr);
		istream_iterator<ci_string, char, ci_char_traits> end;
		vector<ci_string> token(it, end);

		if (ParseLine(token) == false)
		{
			fprintf(stderr, "Error loading scene file... token:  %s!", token[0].c_str());
			getchar();
			exit(-1);
		}
	}
}

bool Animation::ParseLine(const std::vector<ci_string> &token)
{
	if (token.empty())
	{
		return true;
	}
	else if (token[0] == "name")
	{
		assert(token.size() > 2);
		assert(token[1] == "=");

		mName = token[2];
		return true;
	}
	else if (token[0] == "key")
	{
		assert(token.size() > 5);
		assert(token[1] == "=");
		assert(token[3] == "time");
		assert(token[4] == "=");

		ci_string name = token[2];
		AnimationKey *key = World::GetInstance()->FindAnimationKey(name);

		assert(key != nullptr);
		AddKey(key, (float)atof(token[5].c_str()));
		return true;
	}
	return false;
}

void Animation::AddKey(AnimationKey *key, float time)
{
	assert(time >= mDuration);
	mKey.push_back(*key);
	mKeyTime.push_back(time);
	mDuration = time;
}

ci_string Animation::GetName() const
{
	return mName;
}

glm::mat4 Animation::GetAnimationWorldMatrix() const
{
	mat4 worldMatrix(1.0f);
	mat4 translationMatrix(1.0f);
	mat4 rotationMatrix(1.0f);
	mat4 scalingMatrix(1.0f);

	if (!mKey.empty() && !mKeyTime.empty())
	{
		for (size_t i = 0; i < mKey.size(); i++)
		{

			// Getting the upper bound of the animation
			// Otherwise, we aim the first frame
			int upperBound = 0;
			if (i + 1 < mKey.size())
				upperBound = i + 1;
			else
				upperBound = 0;

			if (mCurrentTime >= mKeyTime[i] && mCurrentTime <= mKeyTime[upperBound])
			{

				AnimationKey animKeyBefore = mKey[i];
				AnimationKey animKeyAfter = mKey[upperBound];
				float timeInterpolation = (mCurrentTime - mKeyTime[i]) / (mKeyTime[upperBound] - mKeyTime[i]);

				// Position vector
				vec3 positionVector = spline.GetPosition(i + timeInterpolation);

				// Rotation quaternions
				quat q1 = quat_cast(animKeyBefore.GetWorldMatrix());
				quat q2 = quat_cast(animKeyAfter.GetWorldMatrix());
				quat rotationInterpolation = slerp(q1, q2, timeInterpolation);

				// Scaling Vector
				vec3 scalingVector = mix(animKeyBefore.mScaling, animKeyAfter.mScaling, timeInterpolation);

				// Transforms
				translationMatrix = translate(mat4(1.0f), positionVector);
				rotationMatrix = toMat4(rotationInterpolation);
				scalingMatrix = scale(mat4(1.0f), scalingVector);

				worldMatrix = translationMatrix * rotationMatrix * scalingMatrix;
			}
		}
	}
	return worldMatrix;
}
