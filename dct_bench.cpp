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

#include "dct_bench.h"

std::vector<double> LoadMatrixFromCsv(const std::string& csvPath, uint height, uint width) {
    std::vector<double> ret;
    std::ifstream csvData;
    
    csvData.open(csvPath, std::ifstream::in);
    if (csvData.bad()) return ret;  // Size should be 0
    std::string line;
    uint rows = 0, cells = 0;
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
    return ret;
}

long double BenchDctNs(std::vector<double>& in, int size, std::vector<double>& out, uint impl) {
    timespec_t ts;
    std::vector<double> ret(size * size);
    nsec_t start = 0, end = -1;
    HTime_InitBase();
    if (impl == DCT_IMPL_CV) {
	cv::Mat cvIn = cv::Mat(size, size, CV_64F, &in.front());
	cv::Mat cvOut = cv::Mat(size, size, CV_64F);
	start = HTime_GetNsDelta(&ts);  // Begin timing
	cv::dct(cvIn, cvOut);
	end = HTime_GetNsDelta(&ts);  // End timing
	ret.assign(cvOut.begin<double>(), cvOut.end<double>());
	out = ret;
#if OCV_DCT_DEBUG
	dbgPrintCvMat(cvOut);
#endif
    } else if (impl == DCT_IMPL_MY) {
	// dummy
    }
    return (long double)(end - start);
}

void DctBenchWindowInteractiveDemoSection() {
    static long double elapsed = .0f;
    static int matrixSize = 8;
    static bool matrixLoaded = false;
    static bool matrixProcessed = false;
    static char csvFilePath[128] = "../docs/mat1_in.csv";
    static char fileOpenStatus[512] = "File not loaded yet.";
    static std::vector<double> inputMatrix;
    static std::vector<double> outputMatrix;
    if (ImGui::CollapsingHeader("Interactive Demo")) {
	ImGui::Separator();
	if (ImGui::SliderInt("Matrix Size", &matrixSize, 8, 255)) {
	    matrixLoaded = false;  // Invalidate state on slider change
	    matrixProcessed = false;
	};
	ImGui::InputText("CSV File Path", csvFilePath, IM_ARRAYSIZE(csvFilePath));
	if (ImGui::Button("Load CSV File")) {
	    matrixLoaded = false;
	    matrixProcessed = false;
	    inputMatrix = LoadMatrixFromCsv(csvFilePath, matrixSize, matrixSize);
	    if (inputMatrix.size() == ECSVBAD) {  // File read error
		snprintf((char*)&fileOpenStatus, 512, "Unable to load file \"%s\". Reason: %s.", csvFilePath, strerror(errno));
	    } else if (inputMatrix.size() == ECSVSZ) {  // Matrix size wrong
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
	if (matrixLoaded) {
	    ImGui::Separator();
	    ImGui::Text("Input data:");
	    printMatrix2d(inputMatrix, matrixSize, matrixSize);
	} else {
	    ImGui::Separator();
	    ImGui::Text("No input data to show.");
	}
	ImGui::Separator();
	if (ImGui::Button("Start cv::dct()") && matrixLoaded) {
	    elapsed = BenchDctNs(inputMatrix, matrixSize, outputMatrix, DCT_IMPL_CV);
	    matrixProcessed = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Start myDct()") && matrixLoaded)
	    ;
	ImGui::SameLine();
	if (matrixLoaded) {
	    ImGui::TextWrapped("Last run took %Lf seconds (%Lf milliseconds).", elapsed / NSEC_PER_SEC, elapsed / NSEC_PER_MSEC);
	} else {
	    ImGui::TextWrapped("No matrix has been loaded.");
	}
	// Output data section
	ImGui::Separator();
	if (matrixProcessed) {
	    ImGui::Text("Output data:");
	    printMatrix2dEng(outputMatrix, matrixSize, matrixSize);
	} else {
	    ImGui::Text("No output data to show.");
	}
	// Button section
	ImGui::Separator();
	if (ImGui::Button("Reset")) {
	    elapsed = .0f;
	    matrixSize = 8;
	    matrixLoaded = false;
	    matrixProcessed = false;
	    snprintf((char*)&csvFilePath, 128, "../docs/mat1_in.csv");
	    snprintf((char*)&fileOpenStatus, 512, "File not loaded yet.");
	    inputMatrix.clear();
	    outputMatrix.clear();
	}
    }
}

void DctBenchWindowBenchmarkingSection() {
    // Benchmark section
    // TODO Geometric mean of 10 tests, repeated on 10 matrix sizes, output as CSV data
    if (ImGui::CollapsingHeader("Benchmarking")) {
	ImGui::Separator();
	ImGui::Button("Start Benchmark");
    }
}

void DctBenchWindow(bool* visible) {
    ImGui::SetNextWindowSize(ImVec2(700, 520), ImGuiCond_Once);
    ImGui::Begin(DCT_BENCH_WINDOW_TITLE, visible);
    ImGui::TextWrapped("Demo and benchmark OpenCV's cv::dct() and MyDCT");
    DctBenchWindowInteractiveDemoSection();
    DctBenchWindowBenchmarkingSection();
    ImGui::End();
}