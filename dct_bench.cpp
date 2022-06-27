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

#include <fstream>
#include <sstream>

#include "bench_data.h"
#include "csv_import_export.h"
#include "h_time.h"
#include "my_dct.h"
#include "opencv2/opencv.hpp"

long double BenchDctNs(const std::vector<double>& in, int size, std::vector<double>& out, uint impl) {
    timespec_t ts;
    std::vector<double> temp(size * size);
    nsec_t start = 0, end = -1;
    HTime_InitBase();
    if (impl == DCT_IMPL_CV) {
	temp = in;
	cv::Mat cvIn = cv::Mat(size, size, CV_64F, &temp.front());
	cv::Mat cvOut = cv::Mat(size, size, CV_64F);
	start = HTime_GetNsDelta(&ts);  // Begin timing
	cv::dct(cvIn, cvOut);
	end = HTime_GetNsDelta(&ts);  // End timing
	temp.clear();
	temp.assign(cvOut.begin<double>(), cvOut.end<double>());
#if OCV_DCT_DEBUG
	DbgPrintCvMat(cvOut);
#endif
    } else if (impl == DCT_IMPL_MY) {
	start = HTime_GetNsDelta(&ts);  // Begin timing
	temp = MyDDCT2(in, size);
	end = HTime_GetNsDelta(&ts);  // End timing
    }
    out = temp;
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
	if (ImGui::SliderInt("Matrix Size", &matrixSize, 8, 256)) {
	    matrixLoaded = false;  // Invalidate state on slider change
	    matrixProcessed = false;
	};
	ImGui::InputText("CSV File Path", csvFilePath, IM_ARRAYSIZE(csvFilePath));
	if (ImGui::Button("Load CSV File")) {
	    matrixLoaded = false;
	    matrixProcessed = false;
	    inputMatrix = CsvImportMatrix(csvFilePath, matrixSize, matrixSize);
	    if (inputMatrix.size() == ECSVBAD) {  // File read error
		snprintf((char*)&fileOpenStatus, 512, "Unable to load file \"%s\". Reason: %s.", csvFilePath, strerror(errno));
	    } else if (inputMatrix.size() == ECSVSZ) {  // Matrix size wrong
		snprintf((char*)&fileOpenStatus, 512, // FIXME snprintf() -> sstream
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
	    PrintMatrix2d(inputMatrix, matrixSize, matrixSize);
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
	if (ImGui::Button("Start MyDDCT2()") && matrixLoaded) {
	    elapsed = BenchDctNs(inputMatrix, matrixSize, outputMatrix, DCT_IMPL_MY);
	    matrixProcessed = true;
	}
	// TODO Add "Start MyMDCT2()"
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
	    PrintMatrix2dEng(outputMatrix, matrixSize, matrixSize);
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
    static bool done = false;
    std::vector<double> discard;
    static char csvFilePath[128] = "./bench.csv";
    static char fileOpenStatus[512] = "";
    static std::vector<double> benchResultsMs(18);
    if (ImGui::CollapsingHeader("Benchmarking")) {
	if (ImGui::Button("New Benchmark")) {
	    done = false;
	    snprintf((char*)&fileOpenStatus, 512, "");
	}
	ImGui::SameLine();
	if (ImGui::Button("Start") && !done) {
	    uint i = 0;
	    done = false;
	    benchResultsMs.at(i++) = (double)BenchDctNs(benchMatrix8, 8, discard, DCT_IMPL_CV) / NSEC_PER_MSEC;
	    benchResultsMs.at(i++) = (double)BenchDctNs(benchMatrix8, 8, discard, DCT_IMPL_MY) / NSEC_PER_MSEC;
	    benchResultsMs.at(i++) = (double)BenchDctNs(benchMatrix16, 16, discard, DCT_IMPL_CV) / NSEC_PER_MSEC;
	    benchResultsMs.at(i++) = (double)BenchDctNs(benchMatrix16, 16, discard, DCT_IMPL_MY) / NSEC_PER_MSEC;
	    benchResultsMs.at(i++) = (double)BenchDctNs(benchMatrix32, 32, discard, DCT_IMPL_CV) / NSEC_PER_MSEC;
	    benchResultsMs.at(i++) = (double)BenchDctNs(benchMatrix32, 32, discard, DCT_IMPL_MY) / NSEC_PER_MSEC;
	    benchResultsMs.at(i++) = (double)BenchDctNs(benchMatrix64, 64, discard, DCT_IMPL_CV) / NSEC_PER_MSEC;
	    benchResultsMs.at(i++) = (double)BenchDctNs(benchMatrix64, 64, discard, DCT_IMPL_MY) / NSEC_PER_MSEC;
	    benchResultsMs.at(i++) = (double)BenchDctNs(benchMatrix128, 128, discard, DCT_IMPL_CV) / NSEC_PER_MSEC;
	    benchResultsMs.at(i++) = (double)BenchDctNs(benchMatrix128, 128, discard, DCT_IMPL_MY) / NSEC_PER_MSEC;
	    benchResultsMs.at(i++) = (double)BenchDctNs(benchMatrix256, 256, discard, DCT_IMPL_CV) / NSEC_PER_MSEC;
	    benchResultsMs.at(i++) = (double)BenchDctNs(benchMatrix256, 256, discard, DCT_IMPL_MY) / NSEC_PER_MSEC;
	    done = true;
	}
	ImGui::SameLine();
	ImGui::TextWrapped("The window will freeze for a second as I'm lazy and I didn't use threads.");
	if (done) {
	    ImGui::Separator();
	    if (ImGui::BeginTable("table2", 7)) {
		ImGui::TableNextColumn();
		ImGui::Text("Size");
		for (int i = 0; i < 6; i++) {
		    ImGui::TableNextColumn();
		    ImGui::Text("%d", (int)pow(2, i+3));
		}
		ImGui::TableNextColumn();
		ImGui::Text("cv::dct()");
		for (int i = 0; i < 12; i+=2) {
		    ImGui::TableNextColumn();
		    ImGui::Text("%4.3lf", benchResultsMs.at(i));
		}
		ImGui::TableNextColumn();
		ImGui::Text("MyDDCT2()");
		for (int i = 1; i < 12; i+=2) {
		    ImGui::TableNextColumn();
		    ImGui::Text("%4.3lf", benchResultsMs.at(i));
		}
		ImGui::EndTable();
	    }
	    ImGui::TextWrapped("Results are expressed in milliseconds (ms).");
	    ImGui::InputText("CSV File Path", csvFilePath, IM_ARRAYSIZE(csvFilePath));
	    if (ImGui::Button("Export to CSV")) {
		if (done) {
		    int condCode = CsvExportMatrix(csvFilePath, benchResultsMs, 6, 2);
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
	}
    }
}

void DctBenchWindow(bool* visible) {
    ImGui::SetNextWindowSize(ImVec2(720, 520), ImGuiCond_Once);
    ImGui::Begin(DCT_BENCH_WINDOW_TITLE, visible);
    ImGui::TextWrapped("Demo and benchmark OpenCV's cv::dct() and MyDCT");
    DctBenchWindowInteractiveDemoSection();
    DctBenchWindowBenchmarkingSection();
    ImGui::End();
}