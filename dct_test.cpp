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

#include "dct_test.h"

#include <fstream>

#include "h_time.h"
#include "imgui.h"
#include "opencv2/opencv.hpp"

void printMatrix2dEng(std::vector<double> mat, int height, int width) {
    if (ImGui::BeginTable("table2", width)) {
	for (int i = 0; i < height; i++) {
	    ImGui::TableNextRow();
	    for (int j = 0; j < width; j++) {
		ImGui::TableNextColumn();
		ImGui::Text("% .2e\t", mat.at(j + (i * j)));
	    }
	}
	ImGui::EndTable();
    }
    return;
}

void printMatrix2d(std::vector<double> mat, int height, int width) {
    if(mat.size() < height * width)
	return; // ??? AAAAAh non era static ;)
    if (ImGui::BeginTable("table2", width)) {
	for (int i = 0; i < height; i++) {
	    //ImGui::TableNextRow();
	    for (int j = 0; j < width; j++) {
		ImGui::TableNextColumn();
		ImGui::Text("%g", mat.at(j + (width * i)));
	    }
	}
	ImGui::EndTable();
    }
    return;
}


std::vector<double> loadMatrixFromCsv(const std::string& csvPath, uint w,
                                      uint h) {
    std::ifstream csvData;
    csvData.open(csvPath, std::ifstream::in);
    if (csvData.bad()) return {.0f}; // FIXME wtf
    std::string line;
    std::vector<double> values;
    uint rows = 0, cells = 0;
    while (std::getline(csvData, line)) {
	std::stringstream lineStream(line);
	std::string cell;
	while (std::getline(lineStream, cell, ',')) {
	    values.push_back(std::stod(cell));
	    ++cells;
	}
	++rows;
    }
    if (rows != h || cells / rows != w) return {.0f, .0f};
    csvData.close();
    return values;
}

long double performTestNs(std::vector<double> mat, int size) {
    /* - Initialize the Timebase -------------------------------------------
     */

    timespec_t ts;
    nsec_t start, end;
    HTime_InitBase();

    /* - Perform the DCT2 using the functions provided by OpenCV -----------
     */

    cv::Mat cvTestMatrix = cv::Mat(size, size, CV_32F, &mat);
    cv::Mat cvOut = cv::Mat(size, size, CV_32F);

    start = HTime_GetNsDelta(&ts);  // Begin timing
    cv::dct(cvTestMatrix, cvOut);
    end = HTime_GetNsDelta(&ts);  // End timing

    // memcpy(&outMatrix, cvOut.data,
    //        TEST_MATRIX_H * TEST_MATRIX_W * sizeof(float));

    // printf("The DCT2 using OpenCV's cv::dct() took %f us. Result:\n\n",
    //        (end - start) / (float)NSEC_PER_USEC);
    // printMatrix2d((float*)&outMatrix, TEST_MATRIX_H, TEST_MATRIX_W);

    /* - Perform the DCT2 using the functions provided by us! ----------- */

    return (long double)(end - start);
}

void showTestWindow(bool* visible) {
    static long double elapsed = .0f;
    static int matrixSize = 8;
    static bool matrixLoaded = false;
    static bool matrixProcessed = false;
    static char csvFilePath[128] = "../docs/mat1_in.csv";
    static char fileOpenStatus[512] = "File not loaded yet.";
    // FIXME convert to heap alloc
    static std::vector<double> inputMatrix;
    static std::vector<double> outputMatrix;
    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_Once);
    ImGui::Begin(DCT_TEST_WINDOW_TITLE, visible);
    ImGui::TextWrapped(
        "Test OpenCV's cv::dct()");
    ImGui::Separator();
    if(ImGui::SliderInt("Matrix Size", &matrixSize, 8, 255)) {
	matrixLoaded = false; // Invalidate state on slider change
    };
    ImGui::InputText("CSV File Path", csvFilePath, IM_ARRAYSIZE(csvFilePath));
    if (ImGui::Button("Load CSV File")) {
	matrixLoaded = false;
	inputMatrix = loadMatrixFromCsv(csvFilePath, matrixSize, matrixSize);
	if (inputMatrix.size() == 1) {  // File read error
	    snprintf((char*)&fileOpenStatus, 512,
	             "Unable to load file \"%s\". Reason: %s.", csvFilePath,
	             strerror(errno));
	} else if (inputMatrix.size() == 2) {  // Mat. size error
	    snprintf((char*)&fileOpenStatus, 512,
	             "Could not interpret the contents of the file as a matrix"
	             "of the given dimension(s).");
	} else {
	    snprintf((char*)&fileOpenStatus, 128, "File loaded successfully!");
	    matrixLoaded = true;
	}
    }
    ImGui::SameLine();
    ImGui::TextWrapped("%s", fileOpenStatus);
    if(matrixLoaded) {
	ImGui::Separator();
	ImGui::Text("Input data:");
	printMatrix2d(inputMatrix, matrixSize, matrixSize);
    } else {
	ImGui::Separator();
	ImGui::Text("No input data to show.");
    }
    ImGui::Separator();
    if (ImGui::Button("Start Test") && matrixLoaded)
	elapsed = performTestNs(inputMatrix, matrixSize);
    ImGui::SameLine();
    if (matrixLoaded) {
	ImGui::TextWrapped("Last run took %Lf seconds (%Lf milliseconds).",
	                   elapsed / NSEC_PER_SEC, elapsed / NSEC_PER_MSEC);
    } else {
	ImGui::TextWrapped("No inputMatrix has been loaded.");
    }
    if (ImGui::Button("Close")) *visible = false;
    ImGui::End();
}