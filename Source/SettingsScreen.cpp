//
// COMP 371 Project
//
// Created by David-Etienne Pigeon on 8/6/2019.
//

#include "SettingsScreen.h"
#include "EventManager.h"
#include "TextureLoader.h"

#include <iostream>

using namespace std;
using namespace ImGui;

SettingsScreen::SettingsScreen() {
}

void SettingsScreen::Draw() {
    {
        ImVec2 size = ImVec2(1024.0f, 768.0f);
        ImVec2 position = ImVec2(0.0f, 0.0f);
        
        SetNextWindowSize(size);
        SetNextWindowPos(position);
        ImGui::Begin("Settings");
        
        SetCursorPos(ImVec2(400.0f, 300.0f));
        if (ImGui::Button("Explore!", ImVec2(150.0f, 50.0f))) {
            EventManager::SetApplicationState(ApplicationState::RUNNING);
        }
        SetCursorPos(ImVec2(400.0f, 400.0f));
        if (ImGui::Button("Settings", ImVec2(150.0f, 50.0f))) {
            EventManager::SetApplicationState(ApplicationState::RUNNING);
        }
        
        ImGui::End();
    }
}

SettingsScreen::~SettingsScreen() {
}


