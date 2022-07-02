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

#include "rnd_mat_gen.h"
#include "csv_import_export.h"
#include "h_time.h"
#include "my_dct.h"
#include "opencv2/opencv.hpp"

long double benchDctNs(const std::vector<double>& in, unsigned in_width, std::vector<double>& out, uint impl) {
    timespec_t ts;
    std::vector<double> mat_temp(in_width * in_width);
    int cv_mat_width = static_cast<int>(in_width);
    //auto mat_temp = std::unique_ptr<std::vector<double>>(new std::vector<double>(in_width * in_width));
    nsec_t ts_start = 0, ts_end = -1;
    HTime_InitBase();
    if (impl == DCT_IMPL_CV) {
	mat_temp = in;
	cv::Mat cv_mat_in = cv::Mat(cv_mat_width, cv_mat_width, CV_64F, &mat_temp.front());
	cv::Mat cv_mat_out = cv::Mat(cv_mat_width, cv_mat_width, CV_64F);
	ts_start = HTime_GetNsDelta(&ts);  // Begin timing
	cv::dct(cv_mat_in, cv_mat_out);
	ts_end = HTime_GetNsDelta(&ts);  // End timing
	mat_temp.clear();
	mat_temp.assign(cv_mat_out.begin<double>(), cv_mat_out.end<double>());
#if OCV_DCT_DEBUG
	DbgPrintCvMat(cv_mat_out);
#endif
    } else if (impl == DCT_IMPL_MY) {
	ts_start = HTime_GetNsDelta(&ts);  // Begin timing
	mat_temp = MyDDCT2(in, in_width);
	ts_end = HTime_GetNsDelta(&ts);  // End timing
    }
    out = mat_temp;
    return static_cast<long double>(ts_end - ts_start);
}

void dctBenchWindowInteractiveDemoSection() {
    static long double elapsed = .0f;
    static int mat_width = 8;
    static bool mat_loaded = false;
    static bool mat_processed = false;
    static char csv_file_path[128] = "./out.csv";
    static char io_status_msg[512] = "File not loaded yet.";
    static std::vector<double> mat_in;
    static std::vector<double> mat_out;
    if (ImGui::CollapsingHeader("Interactive Demo")) {
	ImGui::Separator();
	if (ImGui::SliderInt("Matrix Width", &mat_width, 8, 256)) {
	    // TODO Allow non-square matrices
	    // Invalidate state on slider change
	    mat_loaded = false;
	    mat_processed = false;
	};
	ImGui::InputText("CSV File Path", csv_file_path, IM_ARRAYSIZE(csv_file_path));
	if (ImGui::Button("Load CSV File")) {
	    mat_loaded = false;
	    mat_processed = false;
	    try {
		mat_in = csvImportMatrix(csv_file_path, mat_width, mat_width);
		snprintf((char*)&io_status_msg, 512, "File loaded successfully!");
		mat_loaded = true;
	    } catch (std::runtime_error& e) {
		snprintf((char*)&io_status_msg, 512, "Unable to load file \"%s\". Reason: %s", csv_file_path, e.what());
	    }
	}
	ImGui::SameLine();
	ImGui::TextWrapped("%s", io_status_msg);
	if (mat_loaded) {
	    ImGui::Separator();
	    ImGui::Text("Input data:");
	    try {
		makeTable(mat_in, mat_width, mat_width, USE_AUTO);
	    } catch (std::runtime_error& e) {
		ImGui::Text("%s", e.what());
	    }
	} else {
	    ImGui::Separator();
	    ImGui::Text("No input data to show.");
	}
	ImGui::Separator();
	if (ImGui::Button("Start cv::dct()") && mat_loaded) {
	    elapsed = benchDctNs(mat_in, mat_width, mat_out, DCT_IMPL_CV);
	    mat_processed = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Start MyDDCT2()") && mat_loaded) {
	    elapsed = benchDctNs(mat_in, mat_width, mat_out, DCT_IMPL_MY);
	    mat_processed = true;
	}
	// TODO Add "Start MyMDCT2()"
	ImGui::SameLine();
	if (mat_loaded) {
	    ImGui::TextWrapped("Last run took %Lf seconds (%Lf milliseconds).", elapsed / NSEC_PER_SEC, elapsed / NSEC_PER_MSEC);
	} else {
	    ImGui::TextWrapped("No matrix has been loaded.");
	}
	// Output data section
	ImGui::Separator();
	if (mat_processed) {
	    ImGui::Text("Output data:");
	    try {
		makeTable(mat_out, mat_width, mat_width, USE_ENGINEERING);
	    } catch (std::runtime_error& e) {
		ImGui::Text("%s", e.what());
	    }
	} else {
	    ImGui::Text("No output data to show.");
	}
	// Button section
	ImGui::Separator();
	if (ImGui::Button("Reset")) {
	    elapsed = .0f;
	    mat_width = 8;
	    mat_loaded = false;
	    mat_processed = false;
	    snprintf((char*)&csv_file_path, 128, "../docs/mat1_in.csv");
	    snprintf((char*)&io_status_msg, 512, "File not loaded yet.");
	    mat_in.clear();
	    mat_out.clear();
	}
    }
}

void dctBenchWindowBenchmarkingSection() {
    static bool done = false;
    std::vector<double> discard;
    static char csv_file_path[128] = "./bench.csv";
    static char io_status_msg[512] = "";
    static std::vector<double> cv_results_ms, my_results_ms;
    static int max_mat_width_exp = 8;
    if (ImGui::CollapsingHeader("Benchmarking")) {
	if (ImGui::SliderInt("Maximum Matrix Width (2^x)", &max_mat_width_exp, 4, 11)) done = false;
	if (ImGui::Button("Start") && !done) {
	    done = false;
	    unsigned cur_width;
	    std::vector<double> temp;
	    cv_results_ms.clear();
	    my_results_ms.clear();
	    for(unsigned i = 3; i <= max_mat_width_exp; i++) {
		cur_width = static_cast<unsigned>(pow(2,i));
		temp = genRndMat(cur_width, cur_width); // MAYBE Use threads?
		cv_results_ms.push_back(static_cast<double>(benchDctNs(temp, cur_width, discard, DCT_IMPL_CV) / NSEC_PER_MSEC));
		my_results_ms.push_back(static_cast<double>(benchDctNs(temp, cur_width, discard, DCT_IMPL_MY) / NSEC_PER_MSEC));
	    }
	    done = true;
	}
	ImGui::SameLine();
	ImGui::TextWrapped("The window will freeze for a moment while performing the benchmark.");
	if(max_mat_width_exp >= 10) {
	    ImGui::SameLine();
	    ImGui::TextWrapped("WARNING: Benchmarking with a maximum matrix width of 2^10 takes a long time!");
	}
	if (done) {
	    ImGui::Separator();
	    if (ImGui::BeginTable("table2", max_mat_width_exp - 1)) {
		ImGui::TableNextColumn();
		ImGui::Text("Size");
		for (unsigned i = 3; i <= max_mat_width_exp; i++) {
		    ImGui::TableNextColumn();
		    ImGui::Text("%d", static_cast<int>(pow(2, i)));
		}
		ImGui::TableNextColumn();
		ImGui::Text("cv::dct()");
		for (auto res : cv_results_ms) {
		    ImGui::TableNextColumn();
		    ImGui::Text("%4.3lf", res);
		}
		ImGui::TableNextColumn();
		ImGui::Text("MyDDCT2()");
		for (auto res : my_results_ms) {
		    ImGui::TableNextColumn();
		    ImGui::Text("%4.3lf", res);
		}
		ImGui::EndTable();
	    }
	    ImGui::TextWrapped("Results are expressed in milliseconds (ms).");
	    ImGui::InputText("CSV File Path", csv_file_path, IM_ARRAYSIZE(csv_file_path));
	    if (ImGui::Button("Export to CSV") && done) {
		try {
		    std::vector<double> results_ms = {};
		    results_ms.reserve(cv_results_ms.size() + my_results_ms.size());
		    results_ms.insert(results_ms.end(), cv_results_ms.begin(), cv_results_ms.end());
		    results_ms.insert(results_ms.end(), my_results_ms.begin(), my_results_ms.end());
		    csvExportMatrix(csv_file_path, results_ms, max_mat_width_exp - 2, 2);
		    snprintf((char*)&io_status_msg, 512, "File written successfully!");
		} catch (std::runtime_error& e) {
		    snprintf((char*)&io_status_msg, 512, "Unable to write file \"%s\". Reason: %s", csv_file_path, e.what());
		}
	    }
	    ImGui::SameLine();
	    ImGui::TextWrapped("%s", io_status_msg);
	}
    }
}

void dctBenchWindow(bool* visible) {
    ImGui::SetNextWindowSize(ImVec2(720, 520), ImGuiCond_Once);
    ImGui::Begin(DCT_BENCH_WINDOW_TITLE, visible);
    ImGui::TextWrapped("Demo and benchmark OpenCV's cv::dct() and MyDCT");
    dctBenchWindowInteractiveDemoSection();
    dctBenchWindowBenchmarkingSection();
    ImGui::End();
}