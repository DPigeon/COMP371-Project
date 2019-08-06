//
// COMP 371 Project
//
// Created by David-Etienne Pigeon on 8/6/2019.
//

#include "LoadingScreen.h"

using namespace std;
using namespace ImGui;

LoadingScreen::LoadingScreen(ImVec4 backgroundColor) {
	//clear_color = backgroundColor;
}

void LoadingScreen::Draw() {
	{
		static float f = 0.0f;
		ImVec2 size = ImVec2(1024.0f, 768.0f);
		ImVec2 position = ImVec2(0.0f, 0.0f);
		SetNextWindowSize(size);
		SetNextWindowPos(position);
		ImGui::Begin("Guide Tour of Area 51"); 

		ImGui::Text("Welcome to the Tour Guide ! Let us load everything for you...");               

		ImGui::End();
	}
}

LoadingScreen::~LoadingScreen() {
}