//
// Created by quartz on 27/06/22.
//

#include "rnd_mat_gen.h"

#include <fstream>
#include <string>
#include <vector>
#include <random>

#include "imgui.h"
#include "dct_bench.h"

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

uint WriteMatrixToCsv(const std::string& csvPath, uint height, uint width) {
    std::vector<double> ret;
    std::ofstream csvData;

    csvData.open(csvPath, std::ofstream::out);
    if (csvData.bad()) return errno;
    std::string line;
    uint rows = 0, cells = 0;
    /*
    while (std::getline(csvData, line)) {
	std::stringstream lineStream(line);
	std::string cell;
	while (std::getline(lineStream, cell, ',')) {
	    ret.push_back(std::stod(cell));
	    ++cells;
	}
	++rows;
    }
    csvData.close();
    if (rows != height || cells / rows != width) {
	ret.clear();
	ret.push_back(.0f); // Size should be 1
}
     */
    return 0;
}

void RndMatGenWindow(bool* visible) {
    static int matrixSize = 8;
    static bool matrixGenerated = false;
    static char csvFilePath[128] = "./out.csv";
    static char fileOpenStatus[512] = "No file specified.";
    static std::vector<double> matrix;
    ImGui::SetNextWindowSize(ImVec2(600, 270), ImGuiCond_Once);
    ImGui::Begin(RND_MAT_GEN_WINDOW_TITLE, visible);
    if (ImGui::SliderInt("Matrix Size", &matrixSize, 8, 255)) {
	matrixGenerated = false;
    };
    if (ImGui::Button("Generate Matrix")) {
	matrix = GenerateSquareMatrix(matrixSize);
	matrixGenerated = true;
    }
    if (matrixGenerated) {
	if(matrixSize <= 20)
		printMatrix2d(matrix, matrixSize, matrixSize);
	else
	    ImGui::Text("Can't fit the data on-screen!");
    } else {
	ImGui::Text("No data to show.");
    }
    ImGui::Separator();
    ImGui::InputText("CSV File Path", csvFilePath, IM_ARRAYSIZE(csvFilePath));

    if (ImGui::Button("Write to CSV File")) {
	;
    }
    ImGui::SameLine();
    ImGui::TextWrapped("%s", fileOpenStatus);
    ImGui::End();
}