#!/bin/bash
git clone --recursive https://github.com/jack23247/ImGui-CMake-Installer.git
cd ImGui-CMake-Installer
git submodule update --init
cd imgui && git checkout master && cd ..
mkdir -p build && cd build
cmake .. -DIMGUI_WITH_IMPL=ON -DIMGUI_IMPL_SDL_OPENGL2=ON
cmake --build . --config Release --target install
