//
// Created by quartz on 27/06/22.
//

#include "csv_import_export.h"

#include <fstream>
#include <iostream>
#include <sstream>

std::vector<double> csvImportMatrix(const std::string& path, unsigned mat_width, unsigned mat_height) {
    std::ifstream file_ascii;
    file_ascii.open(path, std::ifstream::in);
    if (file_ascii.bad()) throw std::runtime_error("An I/O error occurred while trying to open the file for reading.");
    std::vector<double> ret;
    std::string line;
    unsigned cells = 0, rets = 0;
    while (std::getline(file_ascii, line)) {
	std::stringstream buf(line);
	std::string cell;
	while (std::getline(buf, cell, ',')) {
	    ret.push_back(std::stod(cell));
	    cells++;
	}
	rets++;
    }
    file_ascii.close();
    if (cells != mat_width * mat_height || rets != mat_height)
	throw std::runtime_error("Cannot intepret the contents of the file as a matrix of the given size.");
    return ret;
}

void csvExportMatrix(const std::string& csv_file_path, const std::vector<double>& mat, unsigned mat_width, unsigned mat_height) {
    if (mat.size() != mat_width * mat_height)
	throw std::runtime_error("Can't interpret the contents of the vector as a mat of the given size.");
    std::ofstream file_ascii;
    file_ascii.open(csv_file_path, std::ofstream::out);
    if (file_ascii.bad()) throw std::runtime_error("An I/O error occurred while trying to open the file for writing.");
    std::string line;
    unsigned cols_left = mat_width;
    for (auto elem : mat) {
	file_ascii << elem << ", ";
	cols_left--;
	if (cols_left == 0) {
	    file_ascii << std::endl;
	    cols_left = mat_width;
	}
    }
    file_ascii.close();
}
