
#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <GL/glew.h>

#include <string>

class AppInfoWindow
{
public:
	AppInfoWindow();
	static void Draw(std::string message);
	~AppInfoWindow();

private:
	//static ImVec4 clear_color;
};
