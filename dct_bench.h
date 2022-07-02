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

#include <vector>
#include <stdexcept>

#include "imgui.h"

#define DCT_BENCH_WINDOW_TITLE "DCT Benchmark"

#define OCV_DCT_DEBUG 0

#define DCT_IMPL_CV 0
#define DCT_IMPL_MY 1

#define USE_AUTO 0
#define USE_ENGINEERING 1

#if OCV_DCT_DEBUG
void DbgPrintCvMat(cv::Mat& mat) {
    std::cout << "R (csv)     = " << std::endl << format(mat, cv::Formatter::FMT_CSV) << std::endl << std::endl;
}
#endif

inline void makeTable(const std::vector<double>& mat, unsigned mat_height, unsigned mat_width, bool mode) {
    if (mat.size() < mat_height * mat_width)
	throw std::runtime_error("Can't interpret the contents of the vector as a matrix of the given size.");
    if (mat_height > 64 || mat_width > 64)
	throw std::runtime_error("Can't fit all the elements on screen!");
    if (ImGui::BeginTable("table2", static_cast<int>(mat_width))) {
	for(auto elem : mat) {
	    ImGui::TableNextColumn();
	    if(mode == USE_ENGINEERING)
		ImGui::Text("%+.2e", elem);
	    else // USE_AUTO
	    	ImGui::Text("%g", elem);
	}
	ImGui::EndTable();
    }
}

void dctBenchWindow(bool*);

#endif  // PROJ2_DCT_BENCH_H
