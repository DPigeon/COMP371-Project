//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 14/1/15
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#include "EventManager.h"
#include "Renderer.h"
#include "LoadingScreen.h"
#include "AppInfoWindow.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#include <stdio.h>
#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <irrKlang.h>

using namespace std; 
using namespace irrklang;

#include <GLFW/glfw3.h>

EventManager* EventManager::instance;

/* True if you don't want to see the loading screen */
/* False if you want to see the loading screen */
bool devMode = false;
					
// Time
double EventManager::sLastFrameTime = glfwGetTime();
float  EventManager::sFrameTime = 0.0f;

// Mouse
double EventManager::sLastMousePositionX = 0.0f;
float  EventManager::sMouseDeltaX = 0.0f;
double EventManager::sLastMousePositionY = 0.0f;
float  EventManager::sMouseDeltaY = 0.0f;

// Screen dimensions
float EventManager::screenWidth = 1024;
float EventManager::screenHeight = 768;

// Window
GLFWwindow* EventManager::spWindow = nullptr;

// Sound Engine
#if !defined(PLATFORM_OSX)
    ISoundEngine* engine = createIrrKlangDevice();
#endif


EventManager::EventManager() {
	instance = this;
	isLoading = true;
}

EventManager::~EventManager() {
}

void EventManager::Initialize()
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		exit(-1);
	}
		
	glfwWindowHint(GLFW_SAMPLES, 4);
    
#if defined(PLATFORM_OSX)
    // OS X would use legacy opengl by default, and wouldn't be able to load shaders
    // This is the proper way to setup GLFW to use modern OpenGL
	const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
    
#else
    // Allow older laptops to run the framework, even though, our framework
    // is compatible with OpenGL 3.3 (which removed many deprecated features)
	const char* glsl_version = "#version 330";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
#endif
    
	// Open a window and create its OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, 0);
	spWindow = glfwCreateWindow(1024, 768, "COMP371 Project - Area 51", nullptr, nullptr);

	glfwMakeContextCurrent(spWindow);
	glfwSwapInterval(1); // Enable vsync

	if (spWindow == nullptr)
	{
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		exit(-1);
	}

	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(spWindow, GLFW_STICKY_KEYS, GL_TRUE);

	// Using imgui for the main menu
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();
//	ImGui::StyleColorsClassic();

	ImGui_ImplGlfw_InitForOpenGL(spWindow, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Initial mouse position
	glfwPollEvents();
	double x, y;
	glfwGetCursorPos(spWindow, &x, &y);

	sLastMousePositionX = x;
	sLastMousePositionY = y;

	// Initial time
	sLastFrameTime = glfwGetTime();
    srand((unsigned int) time(nullptr));
    
#if !defined(PLATFORM_OSX)
    engine->play2D("../Audio/Dystopic-Factory.ogg", true);
#endif
	
}

void EventManager::SetLoadingState(bool state) {
	GetInstance()->isLoading = state;
}

bool EventManager::GetLoadingState() {
	return GetInstance()->isLoading;
}

void EventManager::SetPlanetClicked(string message) {
	GetInstance()->planetClickedMessage = message;
}

string EventManager::GetPlanetClickedMessage() {
	return GetInstance()->planetClickedMessage;
}

void EventManager::Shutdown()
{
	// Close OpenGL window and terminate GLFW
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	spWindow = nullptr;
#if !defined(PLATFORM_OSX)
    engine->drop();
#endif
}

void EventManager::Update()
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	// Update inputs / events
	glfwPollEvents();

	// ImGui new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Update mouse position
	double x, y;
	glfwGetCursorPos(spWindow, &x, &y);
	sMouseDeltaX = static_cast<float>( x - sLastMousePositionX );
	sMouseDeltaY = static_cast<float>( y - sLastMousePositionY );
	sLastMousePositionX = x;
	sLastMousePositionY = y;

	// Update frame time
	double currentTime = glfwGetTime();
	sFrameTime = static_cast<float>(currentTime - sLastFrameTime);
	sLastFrameTime = currentTime;

	// Draw the Loading Screen
	if (GetLoadingState()) {
		Renderer::SetShader(SHADER_PHONG);
		if (!devMode)
			LoadingScreen::Draw();
	}
	
	// Draw FPS window when done loading
	if (!GetLoadingState())
		AppInfoWindow::Draw(GetPlanetClickedMessage());

	// Rendering ImGui
	ImGui::Render();

	// Viewport of the screen space
	int display_w, display_h;
	glfwGetFramebufferSize(spWindow, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	if (GetLoadingState() && !devMode) {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(spWindow);

    if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
    {
#if !defined(PLATFORM_OSX)
        engine->play2D("../Audio/aaj_0022_Lazer_Gun_02_SFX.ogg");
#endif
    }

}

float EventManager::GetFrameTime()
{
	return sFrameTime;
}

bool EventManager::ExitRequested()
{
	return glfwGetKey(spWindow, GLFW_KEY_ESCAPE ) == GLFW_PRESS || glfwWindowShouldClose(spWindow);
}

GLFWwindow* EventManager::GetWindow()
{
	return spWindow;
}

EventManager* EventManager::GetInstance()
{
	return instance;
}

float EventManager::GetMouseMotionX()
{
	return sMouseDeltaX;
}

float EventManager::GetMouseMotionY()
{
	return sMouseDeltaY;
}

float EventManager::GetScreenWidth() {
	return screenWidth;
}

float EventManager::GetScreenHeight() {
	return screenHeight;
}

void EventManager::EnableMouseCursor()
{
	glfwSetInputMode(spWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void EventManager::DisableMouseCursor()
{
	glfwSetInputMode(spWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

float EventManager::GetRandomFloat(float min, float max)
{
    float value = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    return min + value*(max - min);
}
