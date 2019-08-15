//
// COMP 371 Project
//
// Created by David-Etienne Pigeon on 8/6/2019.
//

#include "AppInfoWindow.h"
#include "TextureLoader.h"
#include "World.h"

#include <iostream>

using namespace std;
using namespace ImGui;

AppInfoWindow::AppInfoWindow() {
}


void AppInfoWindow::Draw(std::string message) {
	const char* planetName = message.c_str();
    {
        ImVec2 size = ImVec2(300.0f, 85.0f);
        ImVec2 position = ImVec2(722.0f, 0.0f);
        
        SetNextWindowSize(size);
        SetNextWindowPos(position);
        ImGui::Begin("App Info");
        
        ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        
        ImGui::Text("Number of planets: %d / %d", World::GetInstance()->NumberOfPlanetsGenerated(), World::GetInstance()->NumberOfPlanetsToGenerate());

		ImGui::Text("Planet Clicked: %s", planetName);
        
        ImGui::End();
    }
}

AppInfoWindow::~AppInfoWindow() {
}
