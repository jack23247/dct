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

#include "dct_bench_common.h"
#include "ocv_dct_bench.h"

#include "h_time.h"
#include "opencv2/opencv.hpp"

void dbgPrintCvMat(cv::Mat& mat) {
    std::cout << "R (csv)     = " << std::endl
              << format(mat, cv::Formatter::FMT_CSV) << std::endl
              << std::endl;
}

long double BenchOcvDctNs(std::vector<double>& in, int size, std::vector<double>& out) {
    timespec_t ts;
    std::vector<double> ret(size*size);
    nsec_t start, end;
    HTime_InitBase();
    cv::Mat cvIn = cv::Mat(size, size, CV_64F, &in.front());
    cv::Mat cvOut = cv::Mat(size, size, CV_64F);
    start = HTime_GetNsDelta(&ts);  // Begin timing
    cv::dct(cvIn, cvOut);
    end = HTime_GetNsDelta(&ts);  // End timing
    ret.assign(cvOut.begin<double>(), cvOut.end<double>());
    out = ret;
    //dbgPrintCvMat(cvOut);
    return (long double)(end - start);
}

void OcvDctBenchWindow(bool* visible) {
    static long double elapsed = .0f;
    static int matrixSize = 8;
    static bool matrixLoaded = false;
    static bool matrixProcessed = false;
    static char csvFilePath[128] = "../docs/mat1_in.csv";
    static char fileOpenStatus[512] = "File not loaded yet.";
    static std::vector<double> inputMatrix;
    static std::vector<double> outputMatrix;
    ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_Once);
    ImGui::Begin(OCV_DCT_TEST_WINDOW_TITLE, visible);
    ImGui::TextWrapped(
        "Benchmark OpenCV's cv::dct()");
    ImGui::Separator();
    if(ImGui::SliderInt("Matrix Size", &matrixSize, 8, 255)) {
	matrixLoaded = false; // Invalidate state on slider change
	matrixProcessed = false;
    };
    ImGui::InputText("CSV File Path", csvFilePath, IM_ARRAYSIZE(csvFilePath));
    if (ImGui::Button("Load CSV File")) {
	matrixLoaded = false;
	matrixProcessed = false;
	inputMatrix = loadMatrixFromCsv(csvFilePath, matrixSize, matrixSize);
	if (inputMatrix.size() == 1) {  // File read error
	    snprintf((char*)&fileOpenStatus, 512,
	             "Unable to load file \"%s\". Reason: %s.", csvFilePath,
	             strerror(errno));
	} else if (inputMatrix.size() == 2) {  // Mat. size error
	    snprintf((char*)&fileOpenStatus, 512,
	             "Could not interpret the contents of the file as a matrix"
	             " of such dimension(s).");
	} else {
	    snprintf((char*)&fileOpenStatus, 512, "File loaded successfully!");
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
    if (ImGui::Button("Start Test") && matrixLoaded) {
	elapsed = BenchOcvDctNs(inputMatrix, matrixSize, outputMatrix);
	matrixProcessed = true;
    }
    ImGui::SameLine();
    if (matrixLoaded) {
	ImGui::TextWrapped("Last run took %Lf seconds (%Lf milliseconds).",
	                   elapsed / NSEC_PER_SEC, elapsed / NSEC_PER_MSEC);
    } else {
	ImGui::TextWrapped("No matrix has been loaded.");
    }
    // Output data section
    ImGui::Separator();
    if(matrixProcessed) {
	ImGui::Text("Output data:");
	printMatrix2dEng(outputMatrix, matrixSize, matrixSize);
    } else {
	ImGui::Text("No output data to show.");
    }
    // Button section
    ImGui::Separator();
    if (ImGui::Button("Close")) *visible = false;
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
	elapsed = .0f;
	matrixSize = 8;
	matrixLoaded = false;
	matrixProcessed = false;
	snprintf((char*)&csvFilePath, 128,"../docs/mat1_in.csv");
	snprintf((char*)&fileOpenStatus, 512, "File not loaded yet.");
	inputMatrix.clear();
	outputMatrix.clear();
    }
    ImGui::End();
}