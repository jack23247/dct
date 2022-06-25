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

#ifndef PROJ2_DCT_BENCH_COMMON_H
#define PROJ2_DCT_BENCH_COMMON_H

#include <vector>
#include <fstream>
#include <iosfwd>
#include "imgui.h"

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

#endif  // PROJ2_DCT_BENCH_COMMON_H
