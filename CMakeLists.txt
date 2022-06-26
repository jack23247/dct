cmake_minimum_required(VERSION 3.22)
project(proj2)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

set(CMAKE_LIBRARY_PATH deps/ImGui-CMake-Installer/build/dist/lib)
find_library(IMGUI_LIBS NAMES imgui libimgui libimgui.a REQUIRED NO_CACHE)
set(IMGUI_INCLUDE_DIRS_LOCAL deps/ImGui-CMake-Installer/build/dist/include)
set(H_TIME_DIR deps/rt-app/src)

message(STATUS
		"Includes:
		\t-- h_time.h: ${H_TIME_DIR}
		\t-- OpenCV: ${OpenCV_INCLUDE_DIRS}
		\t-- SDL2: ${SDL2_INCLUDE_DIRS}
		\t-- OpenGL: ${OPENGL_INCLUDE_DIR}
		\t-- ImGui: ${IMGUI_INCLUDE_DIRS_LOCAL}")
message(STATUS
		"LinkLibs:
		\t-- OpenCV: ${OpenCV_LIBS}
		\t-- SDL2: ${SDL2_LIBRARIES}
		\t-- OpenGL: ${OPENGL_LIBRARIES}
		\t-- ImGui: ${IMGUI_LIBS}")

add_executable(proj2 main.cpp dct_bench.cpp dct_bench.h my_dct.cpp my_dct.h)
target_link_libraries(proj2 ${OpenCV_LIBS} ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${IMGUI_LIBS})
include_directories(${OpenCV_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${IMGUI_INCLUDE_DIRS_LOCAL} ${H_TIME_DIR})
