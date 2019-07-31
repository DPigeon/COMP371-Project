//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 24/1/15
//
// CopyrightVector (c) 2014-2015 Concordia University. All rightVectors reserved.
//

#include "BSpline.h"
#include "Renderer.h"

// Include GLEW - OpenGL Extension Wrangler
#include <GL/glew.h>

using namespace glm;
using namespace std;

BSpline::BSpline() : Model()
{
}

BSpline::~BSpline()
{
}

void BSpline::CreateVertexBuffer()
{
    if(mSamplePoints.size() == 0)
    {
        GenerateSamplePoints();
    }

    // Create a vertex array
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    
    // Upload Vertex Buffer to the GPU, keep a reference to it (mVertexBufferID)
    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*mSamplePoints.size(), &(mSamplePoints[0]), GL_STATIC_DRAW);
    
    // Create a vertex array
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    
    // Upload Vertex Buffer to the GPU, keep a reference to it (mVertexBufferID)
    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*mSamplePoints.size(), &(mSamplePoints[0]), GL_STATIC_DRAW);
    
    // 1st attribute buffer : vertex Positions
    glVertexAttribPointer(0,
                          3,                // size
                          GL_FLOAT,        // type
                          GL_FALSE,        // normalized?
                          sizeof(Vertex), // stride
                          (void*)0        // array buffer offset
                          );
    glEnableVertexAttribArray(0);
}

void BSpline::Update(float dt)
{
}

void BSpline::Draw()
{
	//cout << mSamplePoints.size() << endl;
	// The Model View Projection transforms are computed in the Vertex Shader
	glBindVertexArray(mVAO);

	GLuint WorldMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldTransform");
	glUniformMatrix4fv(WorldMatrixLocation, 1, GL_FALSE, &GetWorldMatrix()[0][0]);

	// Draw the triangles !
	glDrawArrays(GL_LINE_STRIP, 0, mSamplePoints.size());
	BSpline::ConstructTracks();
}

bool BSpline::ParseLine(const std::vector<ci_string> &token)
{
    if (token.empty())
    {
        return true;
    }
	else if (token[0] == "controlpoint")
	{
		assert(token.size() > 4);
		assert(token[1] == "=");

		float x = static_cast<float>(atof(token[2].c_str()));
		float y = static_cast<float>(atof(token[3].c_str()));
		float z = static_cast<float>(atof(token[4].c_str()));
		AddControlPoint(glm::vec3(x, y, z));
		return true;
	}
    else
    {
        return Model::ParseLine(token);
    }
}

void BSpline::AddControlPoint(glm::vec3 point)
{
	mControlPoints.push_back(point);
}

void BSpline::ClearControlPoints()
{
	mControlPoints.clear();
}

glm::vec3 BSpline::GetPosition(float t) const
{
    // The value of t includes where on the spline you are.
    // The integral value of t is the first control point
    // The decimal value of t is the parameter t within the segment
    
	int p1 = ((int) t) % mControlPoints.size();
	int p2 = (p1 + 1)  % mControlPoints.size();
	int p3 = (p2 + 1)  % mControlPoints.size();
	int p4 = (p3 + 1)  % mControlPoints.size();
	
	return vec3(GetWorldMatrix() * vec4(BSpline::GetPosition(t - (int) t, mControlPoints[p1], mControlPoints[p2], mControlPoints[p3], mControlPoints[p4]), 1.0f));
}

glm::vec3 BSpline::GetPosition(float t, const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4)
{
    // Returns the position of the object on the spline
    // based on parameter t and the four control points p1, p2, p3, p4
    vec4 params(t*t*t, t*t, t, 1);
    mat4 coefficients(vec4(-1, 3, -3, 1), vec4(3, -6, 0, 4), vec4(-3, 3, 3, 1), vec4(1, 0, 0, 0));
    vec4 product = (1.0f / 6.0f) * params * coefficients;
    
    return vec3(vec4(product.x * p1 + product.y * p2 + product.z * p3 + product.w * p4, 1.0f) );
}



void BSpline::GenerateSamplePoints()
{
    if(mControlPoints.size() == 0)
    {
        return;
    }
    mSamplePoints.clear();

	const int numPointsPerSegment = 10;
	float increment = 1.0f / numPointsPerSegment;

	for (int i = 0; i < mControlPoints.size(); ++i)
	{
        float t = 0.0f;
        
        // Set current control points
        vec3 p1 = mControlPoints[i];
        vec3 p2 = mControlPoints[(i + 1) % mControlPoints.size()];
        vec3 p3 = mControlPoints[(i + 2) % mControlPoints.size()];
        vec3 p4 = mControlPoints[(i + 3) % mControlPoints.size()];
        
        for (int j=0; j < numPointsPerSegment; ++j)
        {
            mSamplePoints.push_back(GetPosition(t, p1, p2, p3, p4));
            t += increment;
        }
	}
}

glm::vec3 BSpline::GetTangent(float t) const
{ 
	int p1 = ((int)t) % mControlPoints.size();
	int p2 = (p1 + 1) % mControlPoints.size();
	int p3 = (p2 + 1) % mControlPoints.size();
	int p4 = (p3 + 1) % mControlPoints.size();

	return vec3(GetWorldMatrix() * vec4(BSpline::GetTangent(t - (int)t, mControlPoints[p1], mControlPoints[p2], mControlPoints[p3], mControlPoints[p4]), 0.0f));
}

glm::vec3 BSpline::GetTangent(float t, const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4)
{
	vec4 params(3*t*t, 2*t, 1, 0);
	mat4 coefficients(vec4(-1, 3, -3, 1), vec4(3, -6, 0, 4), vec4(-3, 3, 3, 1), vec4(1, 0, 0, 0));
	vec4 product = (1.0f / 6.0f) * params * coefficients;

	return vec3(vec4(product.x * p1 + product.y * p2 + product.z * p3 + product.w * p4, 1.0f));
}

void BSpline::ConstructTracks() {
	/* Extract the points from spline, get them inside a vector
	 * (use mSamplePoints as we are interpolation 10 points per control point which is good)
	 * Draw 2 pallel lines for edge of track
	 * Draw 2 perpendicular lines inside those 2 tracks
	 */

	double PI = 3.1415926535897932384626433832795;
	float radius = 1.25f;
	float halfLength = 5.0f;
	int slices = 8; // slices for the circle

}
