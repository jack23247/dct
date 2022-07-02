//
// Created by quartz on 27/06/22.
//

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
    if (ImGui::Button("Write to CSV File")) {
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