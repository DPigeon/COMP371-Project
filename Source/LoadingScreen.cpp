//
//  ObjectDescription.h
//  Area51
//
//  Created by David-Etienne Pigeon on 2019-08-06.
//  Copyright © 2019 Concordia. All rights reserved.
//

#include "LoadingScreen.h"
#include "TextureLoader.h"

#include <iostream>

using namespace std;
using namespace ImGui;

LoadingScreen::LoadingScreen() {
}

void LoadingScreen::Draw() {
	{
		#if defined(PLATFORM_OSX)
			int logo = TextureLoader::LoadTexture("Textures/Area51_Logo.png");
		#else
			int logo = TextureLoader::LoadTexture("../Assets/Textures/Area51_Logo.png"); // Let's find a better logo
		#endif

			ImVec2 size = ImVec2(1024.0f, 768.0f);
			ImVec2 position = ImVec2(0.0f, 0.0f);
			ImVec2 logoSize = ImVec2(600.0f, 300.0f);
			ImVec2 progressBarPosition = ImVec2(185.0f, 625.0f);

			SetNextWindowSize(size);
			SetNextWindowPos(position);
			ImGui::Begin("Guide Tour of Area 51"); 

			ImVec2 logoCentered = ImVec2((GetWindowSize().x - logoSize.x) * 0.4f, (GetWindowSize().y - logoSize.y) * 0.4f);
			SetCursorPos(logoCentered);
			ImGui::Image(ImTextureID(logo), logoSize, ImVec2(0.0f, 0.0f), ImVec2(1, -1));

			SetCursorPos(progressBarPosition);
			ProgressBar();

			ImGui::End();
	}
}

void LoadingScreen::ProgressBar() {
	// Loads naturally 27 points / seconds
	// So we set a constant maxPoints (250 is fine since we almost never get below 250 points)

	float pointsPerSecond = 27.0f;
	float maxFixedPoints = 250.0f; // Set a constant random number for now (cannot determine how many points total on the spline at beginning cause we live extrapolate)
	float totalTime = maxFixedPoints / pointsPerSecond;
	double time = ImGui::GetTime();

	ImGui::ProgressBar(time / totalTime, ImVec2(0.0f, 0.0f));
}

LoadingScreen::~LoadingScreen() {
}
