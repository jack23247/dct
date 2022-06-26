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

#ifndef PROJ2_DCT_BENCH_H
#define PROJ2_DCT_BENCH_H

#include <fstream>
#include <sstream>
#include <vector>

#include "h_time.h"
#include "imgui.h"
#include "opencv2/opencv.hpp"

#define DCT_BENCH_WINDOW_TITLE "DCT Benchmark"

#define OCV_DCT_DEBUG 0

#define DCT_IMPL_CV 0
#define DCT_IMPL_MY 1

#define ECSVBAD 0
#define ECSVSZ 1

#if OCV_DCT_DEBUG
void dbgPrintCvMat(cv::Mat& mat) {
    std::cout << "R (csv)     = " << std::endl << format(mat, cv::Formatter::FMT_CSV) << std::endl << std::endl;
}
#endif

inline void printMatrix2dEng(std::vector<double>& mat, int height, int width) {
    if (mat.size() < height * width) return;
    if (ImGui::BeginTable("table2", width)) {
	for (int i = 0; i < height; i++) {
	    for (int j = 0; j < width; j++) {
		ImGui::TableNextColumn();
		ImGui::Text("%+.2e", mat.at(j + (width * i)));
	    }
	}
	ImGui::EndTable();
    }
}

inline void printMatrix2d(std::vector<double>& mat, int height, int width) {
    if (mat.size() < height * width) return;
    if (ImGui::BeginTable("table2", width)) {
	for (int i = 0; i < height; i++) {
	    for (int j = 0; j < width; j++) {
		ImGui::TableNextColumn();
		ImGui::Text("%g", mat.at(j + (width * i)));
	    }
	}
	ImGui::EndTable();
    }
}

void DctBenchWindow(bool*);

#endif  // PROJ2_DCT_BENCH_H
