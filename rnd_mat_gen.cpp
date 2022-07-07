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

#include "rnd_mat_gen.h"

#include <fstream>
#include <string>

#include "csv_import_export.h"
#include "dct_bench.h"
#include "imgui.h"

void rndMatGenWindow(bool* visible) {
    static int mat_width = 8;
    static bool mat_ready = false;
    static char csv_file_path[128] = "./out.csv";
    static char io_status_msg[512] = "No file specified.";
    static std::vector<double> mat;
    ImGui::SetNextWindowSize(ImVec2(600, 270), ImGuiCond_Once);
    ImGui::Begin(RND_MAT_GEN_WINDOW_TITLE, visible);
    ImGui::SliderInt("Matrix Width", &mat_width, 8, 256);
    if (ImGui::Button("Generate New Matrix")) {
	mat_ready = false;
	mat = genRndMat(mat_width, mat_width);
	mat_ready = true;
    }
    if (mat_ready) {
	try {
	    makeTable(mat, mat_width, mat_width, USE_AUTO);
	} catch (std::runtime_error& e) {
	    ImGui::Text("%s", e.what());
	}
    } else {
	ImGui::Text("No data to show.");
    }
    ImGui::Separator();
    ImGui::InputText("CSV File Path", csv_file_path, IM_ARRAYSIZE(csv_file_path));
    if (ImGui::Button("Save to CSV File")) {
	if (mat_ready) {
	    try {
		csvExportMatrix(csv_file_path, mat, mat_width, mat_width);
	    } catch (std::runtime_error& e) {
		snprintf((char*)&io_status_msg, 512, "%s", e.what());
	    }
	    snprintf((char*)&io_status_msg, 512, "File written successfully!");
	} else {
	    snprintf((char*)&io_status_msg, 512, "No data to write.");
	}
    }
    ImGui::SameLine();
    ImGui::TextWrapped("%s", io_status_msg);
    ImGui::End();
}