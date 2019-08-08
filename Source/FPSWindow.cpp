//
// COMP 371 Project
//
// Created by David-Etienne Pigeon on 8/6/2019.
//

#include "FPSWindow.h"
#include "TextureLoader.h"

#include <iostream>

using namespace std;
using namespace ImGui;

FPSWindow::FPSWindow() {
}

void FPSWindow::Draw() {
	{
		ImVec2 size = ImVec2(300.0f, 70.0f);
		ImVec2 position = ImVec2(722.0f, 0.0f);

		SetNextWindowSize(size);
		SetNextWindowPos(position);
		ImGui::Begin("App Info");

		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::End();
	}
}

FPSWindow::~FPSWindow() {
}