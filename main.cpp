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

#include "h_time.h"

// DCTToolbox
#include "dct_bench.h"
#include "img_compressor.h"
#include "rnd_mat_gen.h"

#define MAIN_WINDOW_TITLE "DCTToolbox v1.02"
#define DCT_TOOLBOX_VERSION "1.02"

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
	printf("Error: %s\n", SDL_GetError());
	exit(EXIT_FAILURE);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window =
        SDL_CreateWindow(MAIN_WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
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
    ImVec4 bkg_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // bool showToolboxWindow = true;
    bool show_img_compressor_window = false;
    bool show_dct_bench_window = false;
    bool show_rnd_mat_gen_window = false;

    // Initialize the timebase
    HTime_InitBase();

    // Main loop
    bool done = false;
    while (!done) {
	// Process events
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
	    ImGui_ImplSDL2_ProcessEvent(&event);
	    if (event.type == SDL_QUIT) done = true;
	    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
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
	    ImGui::SetNextWindowSize(ImVec2(320, 210));
	    ImGui::Begin("Tool Selector", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize); //| ImGuiWindowFlags_AlwaysAutoResize);
	    ImGui::Checkbox("Image Compressor", &show_img_compressor_window);
	    ImGui::Checkbox(DCT_BENCH_WINDOW_TITLE, &show_dct_bench_window);
	    ImGui::Checkbox("Random Matrix Generator", &show_rnd_mat_gen_window);

	    if (show_img_compressor_window) {
		imgCompressorWindow(&show_img_compressor_window);
	    }


	    if (show_dct_bench_window) {
		dctBenchWindow(&show_dct_bench_window);
	    }

	    if (show_rnd_mat_gen_window) {
		rndMatGenWindow(&show_rnd_mat_gen_window);
	    }

	    ImGui::Separator();
	    ImGui::TextWrapped(
	        "DCTToolbox v%s\n"
	        "Copyright (C) 2022  Jacopo Maltagliati\n"
	        "Copyright (C) 2022  Alessandro Albi\n"
	        "Released under the GNU LGPL-v2.1.\n",
	        DCT_TOOLBOX_VERSION);
	    if (ImGui::CollapsingHeader("Open Source Libraries")) {
		ImGui::TextWrapped(
		    "Dear ImGui v1.88\n"
		    "Copyright (c) 2014-2022 Omar Cornut\n"
		    "Released under the MIT license.\n"
		    "\nImGui-CMake-Installer\n"
		    "Copyright (c) 2019 Gilad Reich\n"
		    "Copyright (c) 2022 Jacopo Maltagliati\n"
		    "Released under the MIT license.\n"
		    "\nh_time.h\n"
		    "Copyright (c) 2021 Jacopo Maltagliati\n"
		    "Released under the EUPL v. 1.2\n"
		    "\nstb_image.h v2.27\n"
		    "By Sean T. Barrett\n"
		    "Released in the Public Domain.\n");
		ImGui::Separator();
	    }
	    if (ImGui::CollapsingHeader("Debug Information")) {
		ImGui::Text("Built on ImGui v%s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
		ImGui::Text("Rendering: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
		            ImGui::GetIO().Framerate);
	    }
	    ImGui::End();
	}

	// Rendering
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(bkg_color.x * bkg_color.w, bkg_color.y * bkg_color.w, bkg_color.z * bkg_color.w, bkg_color.w);
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

    exit(EXIT_SUCCESS);
}
