#!/bin/bash
# rt_app (for h_time.h)
git clone --recursive https://github.com/iralabdisco/rt-app
# stb (for stb_image.h)
git clone --recursive https://github.com/nothings/stb
# ImGui-CMake-Installer (to use ImGui with CMake)
git clone --recursive https://github.com/jack23247/ImGui-CMake-Installer.git
cd ImGui-CMake-Installer
git submodule update --init
cd imgui && git checkout master && cd ..
mkdir -p build && cd build
cmake .. -DIMGUI_WITH_IMPL=ON -DIMGUI_IMPL_SDL_OPENGL2=ON
cmake --build . --config Release --target install
