//
// COMP 371 Project
//
// Created by David-Etienne Pigeon on 8/6/2019.
//

#include "MenuScreen.h"
#include "TextureLoader.h"

#include <iostream>

using namespace std;
using namespace ImGui;

MenuScreen::MenuScreen() {
}

void MenuScreen::Draw() {
    {
        ImVec2 size = ImVec2(1024.0f, 768.0f);
        ImVec2 position = ImVec2(0.0f, 0.0f);
        ImVec2 logoSize = ImVec2(600.0f, 300.0f);
        ImVec2 progressBarPosition = ImVec2(185.0f, 625.0f);
        
        SetNextWindowSize(size);
        SetNextWindowPos(position);
        ImGui::Begin("Menu");
        
        
        
        ImVec2 logoCentered = ImVec2((GetWindowSize().x - logoSize.x) * 0.4f, (GetWindowSize().y - logoSize.y) * 0.4f);
        SetCursorPos(logoCentered);
        
        ImGui::End();
    }
}

MenuScreen::~MenuScreen() {
}

