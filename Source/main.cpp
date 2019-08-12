//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 14/1/15
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#include "Renderer.h"
#include "World.h"
#include "EventManager.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


int main(int argc, char *argv[])
{
	EventManager eventManager;
	eventManager.Initialize();
	Renderer::Initialize();

	World world;

	if (argc > 1)
	{
		world.LoadScene(argv[1]);
	}
	else
	{
		// Alternate different scenes from here
#if defined(PLATFORM_OSX)
		world.LoadScene("Scenes/BSplineScene.scene");
//		world.LoadScene("Scenes/StaticScene.scene");
//		world.LoadScene("Scenes/CoordinateSystem.scene");
#else
		world.LoadScene("../Assets/Scenes/BSplineScene.scene");
//		world.LoadScene("../Assets/Scenes/StaticScene.scene");
//		world.LoadScene("../Assets/Scenes/CoordinateSystem.scene");
#endif
	}

	// Main Loop
	do
	{
		// Update Event Manager - Frame time / input / events processing / UI
		eventManager.Update();

		// Update World
		float dt = eventManager.GetFrameTime();
		world.Update(dt);

		// Draw World
		world.Draw();

        bool isLoading = world.GetApplicationState() == ApplicationState::LOADING;
		if (!isLoading) {
            ApplicationState state = eventManager.GetApplicationState() == ApplicationState::LOADING ? ApplicationState::MENU : eventManager.GetApplicationState();
            eventManager.SetApplicationState(state);
		}
	} while (eventManager.ExitRequested() == false);

	Renderer::Shutdown();
	eventManager.Shutdown();

	return 0;
}
