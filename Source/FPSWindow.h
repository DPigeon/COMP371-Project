
#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <GL/glew.h>

class FPSWindow
{
public:
	FPSWindow();
	static void Draw();
	~FPSWindow();

private:
	//static ImVec4 clear_color;
};