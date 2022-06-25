/*
    Bitmap image compressor using OpenCV's DCT implementation and (optionally)
    an homegrown algorithm. Provides an A-B comparison functionality and a GUI.

    Copyright (C) 2022  Jacopo Maltagliati
    Copyright (C) 2022  Alessandro Albi

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
    USA
*/

#include <SDL.h>
#include <SDL_opengl.h>

#include <cstdio>

#include "imgui.h"
#include "imgui_impl_opengl2.h"
#include "imgui_impl_sdl.h"

#include "dct_test.h"

int main(int, char**) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
	printf("Error: %s\n", SDL_GetError());
	return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_WindowFlags window_flags =
        (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                          SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow(
        "ImageCompressor - MCS Proj2 a.a. 2021-22", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);  // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
    // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
    // Enable Gamepad Controls

    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    // Our state
    bool visible_testWindow = false; // DCT-II Test Window Handle
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done) {

	// Process events
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
	    ImGui_ImplSDL2_ProcessEvent(&event);
	    if (event.type == SDL_QUIT) done = true;
	    if (event.type == SDL_WINDOWEVENT &&
	        event.window.event == SDL_WINDOWEVENT_CLOSE &&
	        event.window.windowID == SDL_GetWindowID(window))
		done = true;
	}

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	// if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

	// MainWindow
	{

	    ImGui::Begin("ImageCompressor Tools");
	    ImGui::Checkbox(DCT_TEST_WINDOW_TITLE, &visible_testWindow);

	    /*
	    ImGui::Checkbox("Demo Window",
	                    &show_demo_window);  // Edit bools storing our
	                                         // window open/close state
	    ImGui::Checkbox("Another Window", &show_another_window);
	     */
		/*
	    ImGui::SliderFloat(
	        "float", &f, 0.0f,
	        1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
	    ImGui::ColorEdit3(
	        "clear color",show_another_window
	        (float*)&clear_color);  // Edit 3 floats representing a color
	        */

	    if(visible_testWindow) {
		showTestWindow(&visible_testWindow);
	    }

	    ImGui::Separator();
	    ImGui::Text("Built on ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
	    ImGui::Text("Average frame times: %.3f ms/frame (%.1f FPS)",
	                1000.0f / ImGui::GetIO().Framerate,
	                ImGui::GetIO().Framerate);
	    ImGui::End();
	}

	// Rendering
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(clear_color.x * clear_color.w,
	             clear_color.y * clear_color.w,
	             clear_color.z * clear_color.w, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	// glUseProgram(0); // You may want this if using this code in an OpenGL
	// 3+ context where shaders may be bound
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
