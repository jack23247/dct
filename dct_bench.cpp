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

#include "rnd_mat_gen.h"
#include "csv_import_export.h"
#include "h_time.h"
#include "my_dct.h"
#include "opencv2/opencv.hpp"

long double benchDctNs(const std::vector<double>& in, int in_rows, int in_cols, std::vector<double>& out, uint impl) {
    timespec_t ts;
    std::vector<double> mat_temp(in_rows * in_cols);
    nsec_t ts_start = 0, ts_end = -1;
    if (impl == DCT_IMPL_CV) {
	mat_temp = in;
	cv::Mat cv_mat_in = cv::Mat(in_rows, in_cols, CV_64F, &mat_temp.front());
	cv::Mat cv_mat_out = cv::Mat(in_rows, in_cols, CV_64F);
	ts_start = HTime_GetNsDelta(&ts);  // Begin timing
	cv::dct(cv_mat_in, cv_mat_out);
	ts_end = HTime_GetNsDelta(&ts);  // End timing
	mat_temp.clear();
	mat_temp.assign(cv_mat_out.begin<double>(), cv_mat_out.end<double>());
#if OCV_DCT_DEBUG
	DbgPrintCvMat(cv_mat_out);
#endif
    } else if (impl == DCT_IMPL_MY) {
	ts_start = HTime_GetNsDelta(&ts);
	mat_temp = MyDDCT2(in, in_rows);
	ts_end = HTime_GetNsDelta(&ts);
    } else if (impl == DCT_IMPL_MY_MONO) {
	ts_start = HTime_GetNsDelta(&ts);
	mat_temp = MyMDCT2(in);
	ts_end = HTime_GetNsDelta(&ts);
    }
    out = mat_temp;
    return static_cast<long double>(ts_end - ts_start);
}

void dctBenchWindowInteractiveDemoSection() {
    static long double elapsed = .0f;
    static bool mat_is_square = true;
    static int mat_cols = 8;
    static int mat_rows = mat_cols;
    static bool mat_loaded = false;
    static bool mat_processed = false;
    static char csv_file_path[128] = "./out.csv";
    static char res_file_path[128] = "./results.csv";
    static char in_status_msg[512] = "File not loaded yet.";
    static char out_status_msg[512] = "";
    static char demo_status_msg[512] = "No matrix has been loaded.";
    static std::vector<double> mat_in;
    static std::vector<double> mat_out;
    if (ImGui::CollapsingHeader("Interactive Demo")) {
	ImGui::Separator();
	ImGui::Checkbox("Force Matrix Squareness", &mat_is_square);
	if (!mat_is_square) {
	    if (ImGui::SliderInt("Matrix Height", &mat_rows, 1, 256)) {
		// Invalidate state on slider change
		mat_loaded = false;
		mat_processed = false;
	    };
	}
	if (ImGui::SliderInt("Matrix Width", &mat_cols, 1, 256)) {
	    if (mat_is_square) mat_rows = mat_cols;
	    // Invalidate state on slider change
	    mat_loaded = false;
	    mat_processed = false;
	};
	ImGui::InputText("CSV File Path", csv_file_path, IM_ARRAYSIZE(csv_file_path));
	if (ImGui::Button("Reset")) {
	    elapsed = .0f;
	    mat_cols = 8;
	    mat_loaded = false;
	    mat_processed = false;
	    mat_in.clear();
	    mat_out.clear();
	    snprintf((char*)&csv_file_path, 128, "./out.csv");
	    snprintf((char*)&res_file_path, 128, "./results.csv");
	    snprintf((char*)&in_status_msg, 512, "File not loaded yet.");
	    snprintf((char*)&demo_status_msg, 512, "No matrix has been loaded.");
	}
	ImGui::SameLine();
	// FIXME What if I load an odd-sized matrix?
	if (ImGui::Button("Load from CSV File")) {
	    mat_loaded = false;
	    mat_processed = false;
	    try {
		mat_in = csvImportMatrix(csv_file_path, mat_rows, mat_cols);
		snprintf((char*)&in_status_msg, 512, "File loaded successfully!");
		mat_loaded = true;
		snprintf((char*)&demo_status_msg, 512, "Ready.");
	    } catch (std::runtime_error& e) {
		snprintf((char*)&in_status_msg, 512, "Unable to load file \"%s\". Reason: %s", csv_file_path, e.what());
	    }
	}
	ImGui::SameLine();
	ImGui::TextWrapped("%s", in_status_msg);
	if (mat_loaded) {
	    ImGui::Separator();
	    ImGui::Text("Input data:");
	    try {
		makeTable(mat_in, mat_rows, mat_cols, USE_AUTO);
	    } catch (std::runtime_error& e) {
		ImGui::Text("%s", e.what());
	    }
	} else {
	    ImGui::Separator();
	    ImGui::Text("No input data to show.");
	}
	ImGui::Separator();
	if (mat_loaded) {
	    if (ImGui::Button("Start cv::dct()")) {
		mat_processed = false;
		elapsed = benchDctNs(mat_in, mat_rows, mat_cols, mat_out, DCT_IMPL_CV);
		mat_processed = true;
	    }
	    ImGui::SameLine();
	    if (ImGui::Button("Start MyDDCT2()")) {
		mat_processed = false;
		if (mat_rows != mat_cols) {
		    snprintf((char*)&demo_status_msg, 512, "Can't perform the DDCT2 on a non-square matrix.");
		} else {
		    elapsed = benchDctNs(mat_in, mat_rows, mat_cols, mat_out, DCT_IMPL_MY);
		    mat_processed = true;
		}
	    }
	    ImGui::SameLine();
	    if (ImGui::Button("Start MyMDCT2()")) {
		mat_processed = false;
		if (mat_rows != 1) {
		    snprintf((char*)&demo_status_msg, 512, "Can't perform the MDCT2 on a matrix with more than one row.");
		} else {
		    elapsed = benchDctNs(mat_in, mat_rows, mat_cols, mat_out, DCT_IMPL_MY_MONO);
		    mat_processed = true;
		}
	    }
	}
	if (mat_processed)
	    snprintf((char*)&demo_status_msg, 512, "Last run took %Lf seconds (%Lf milliseconds).", elapsed / NSEC_PER_SEC,
	             elapsed / NSEC_PER_MSEC);
	ImGui::TextWrapped("%s", demo_status_msg);
	// Output data section
	ImGui::Separator();
	if (mat_processed) {
	    ImGui::Text("Output data:");
	    try {
		makeTable(mat_out, mat_rows, mat_cols, USE_ENGINEERING);
	    } catch (std::runtime_error& e) {
		ImGui::Text("%s", e.what());
	    }
	} else {
	    ImGui::Text("No output data to show.");
	}
	ImGui::Separator();
	if (mat_processed) {
	    ImGui::InputText("Results File Path", res_file_path, IM_ARRAYSIZE(res_file_path));
	    if (ImGui::Button("Save to CSV File")) {
		try {
		    csvExportMatrix(res_file_path, mat_out, mat_rows, mat_cols);
		    snprintf((char*)&out_status_msg, 512, "File written successfully!");
		} catch (std::runtime_error& e) {
		    snprintf((char*)&out_status_msg, 512, "Unable to write file \"%s\". Reason: %s", res_file_path, e.what());
		}
	    }
    	}
	ImGui::SameLine();
	ImGui::Text("%s", out_status_msg);
    }
}

void dctBenchWindowBenchmarkingSection() {
    static bool done = false;
    std::vector<double> discard;
    static char csv_file_path[128] = "./bench.csv";
    static char io_status_msg[512] = "";
    static std::vector<double> cv_results_ms, my_results_ms;
    static int steps = 8;
    if (ImGui::CollapsingHeader("Benchmarking")) {
	if (ImGui::SliderInt("Steps", &steps, 4, 127)) done = false;
	if (ImGui::Button("Start") && !done) {
	    done = false;
	    int cur_cols;
	    std::vector<double> temp;
	    cv_results_ms.clear();
	    my_results_ms.clear();
	    for(int i = 3; i <= steps; i++) {
		cur_cols = 2*i;
		temp = genRndMat(cur_cols, cur_cols); // MAYBE Use threads?
		cv_results_ms.push_back(static_cast<double>(benchDctNs(temp, cur_cols, cur_cols, discard, DCT_IMPL_CV) / NSEC_PER_MSEC));
		my_results_ms.push_back(static_cast<double>(benchDctNs(temp, cur_cols, cur_cols, discard, DCT_IMPL_MY) / NSEC_PER_MSEC));
	    }
	    done = true;
	}
	ImGui::SameLine();
	ImGui::TextWrapped("The window will freeze while performing the benchmark.");
	if(steps > 100) {
	    ImGui::SameLine();
	    ImGui::TextWrapped("WARNING: Benchmarking with more than 100 steps might take a while!");
	}
	if (done) {
	    ImGui::Separator();
	    if (steps <= 64) {
		if (ImGui::BeginTable("table2", steps - 1)) {
		    ImGui::TableNextColumn();
		    ImGui::Text("Size");
		    for (int i = 3; i <= steps; i++) {
			ImGui::TableNextColumn();
			ImGui::Text("%d", 2 * i);
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
	    } else {
		ImGui::TextWrapped("Cannot show results as they exceed the maximum allowable width (64) of ImGui::Table()!");
	    }
	    ImGui::InputText("CSV File Path", csv_file_path, IM_ARRAYSIZE(csv_file_path));
	    if (ImGui::Button("Export to CSV") && done) {
		try {
		    std::vector<double> results_ms = {};
		    results_ms.reserve(cv_results_ms.size() + my_results_ms.size());
		    results_ms.insert(results_ms.end(), cv_results_ms.begin(), cv_results_ms.end());
		    results_ms.insert(results_ms.end(), my_results_ms.begin(), my_results_ms.end());
		    csvExportMatrix(csv_file_path, results_ms, 2, steps - 2);
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