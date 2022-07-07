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

#include "csv_import_export.h"

#include <fstream>
#include <iostream>
#include <sstream>

std::vector<double> csvImportMatrix(const std::string& path, int mat_rows, int mat_cols) {
    std::ifstream file_ascii;
    file_ascii.open(path, std::ifstream::in);
    if (file_ascii.bad()) throw std::runtime_error("An I/O error occurred while trying to open the file for reading.");
    // XXX .bad() apparently not set when the file does not exist. Wtf?
    std::vector<double> ret;
    std::string line;
    unsigned cells = 0, rets = 0;
    while (std::getline(file_ascii, line)) {
	std::stringstream buf(line);
	std::string cell;
	while (std::getline(buf, cell, ',')) {
	    try {
		ret.push_back(std::stod(cell));
		cells++;
	    } catch(std::invalid_argument& e) {
		throw std::runtime_error("Empty or invalid cell detected. Are you trying to load malformed CSV?");
	    }
	}
	rets++;
    }
    file_ascii.close();
    if (cells != mat_cols * mat_rows || rets != mat_rows)
	throw std::runtime_error("Can't interpret the contents of the file as a matrix of the given size.");
    return ret;
}

void csvExportMatrix(const std::string& csv_file_path, const std::vector<double>& mat, int mat_rows, int mat_cols) {
    if (mat.size() != mat_cols * mat_rows)
	throw std::runtime_error("Can't interpret the contents of the vector as a matrix of the given size.");
    std::ofstream file_ascii;
    file_ascii.open(csv_file_path, std::ofstream::out);
    if (file_ascii.bad()) throw std::runtime_error("An I/O error occurred while trying to open the file for writing.");
    std::string line;
    unsigned cols_left = mat_cols;
    for (auto elem : mat) {
	cols_left--;
	if (cols_left == 0) {
	    file_ascii << elem << std::endl;
	    cols_left = mat_cols;
	} else {
	    file_ascii << elem << ", ";
	}
    }
    file_ascii.close();
}
