//
// Created by quartz on 27/06/22.
//

#include "rnd_mat_gen.h"

#include <fstream>
#include <random>
#include <string>
#include <vector>

#include "csv_import_export.h"
#include "dct_bench.h"
#include "imgui.h"

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_real_distribution<double> dist(-999.0, +1000.0);

std::vector<double> GenerateSquareMatrix(uint n) {
    std::vector<double> ret(n * n);
    for (uint r = 0; r < n; r++) {      // u < height
	for (uint c = 0; c < n; c++) {  // v < width
	    ret.at(c + (n * r)) = dist(mt);
	}
    }
    return ret;
}

void RndMatGenWindow(bool* visible) {
    static int matrixSize = 8;
    static bool matrixGenerated = false;
    static char csvFilePath[128] = "./out.csv";
    static char fileOpenStatus[512] = "No file specified.";
    static std::vector<double> matrix;
    ImGui::SetNextWindowSize(ImVec2(600, 270), ImGuiCond_Once);
    ImGui::Begin(RND_MAT_GEN_WINDOW_TITLE, visible);
    if (ImGui::SliderInt("Matrix Size", &matrixSize, 8, 256)) {
	matrixGenerated = false;
    };
    if (ImGui::Button("Generate Matrix")) {
	matrix = GenerateSquareMatrix(matrixSize);
	matrixGenerated = true;
    }
    if (matrixGenerated) {
	if (matrixSize <= 20)
	    PrintMatrix2d(matrix, matrixSize, matrixSize);
	else
	    ImGui::Text("Can't fit the data on-screen!");
    } else {
	ImGui::Text("No data to show.");
    }
    ImGui::Separator();
    ImGui::InputText("CSV File Path", csvFilePath, IM_ARRAYSIZE(csvFilePath));
    if (ImGui::Button("Write to CSV File")) {
	if (matrixGenerated) {
	    int condCode = CsvExportMatrix(csvFilePath, matrix, matrixSize, matrixSize);
	    if (condCode != 0)
		snprintf((char*)&fileOpenStatus, 512, "%s", strerror(condCode));
	    else
		snprintf((char*)&fileOpenStatus, 512, "File written successfully!");
	} else {
	    snprintf((char*)&fileOpenStatus, 512, "No data to write.");
	}
    }
    ImGui::SameLine();
    ImGui::TextWrapped("%s", fileOpenStatus);
    ImGui::End();
}