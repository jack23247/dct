//
// Created by quartz on 27/06/22.
//

#include "csv_import_export.h"

#include <fstream>
#include <iostream>
#include <sstream>

std::vector<double> CsvImportMatrix(const std::string& path, uint width, uint height) {
    // TODO Hack -> Exceptions
    std::vector<double> ret;
    std::ifstream ifstream;
    ifstream.open(path, std::ifstream::in);
    if (ifstream.bad()) return ret;  // HACK Size should be 0
    std::string line;
    uint rows = 0, cells = 0;
    while (std::getline(ifstream, line)) {
	std::stringstream lineStream(line);
	std::string cell;
	while (std::getline(lineStream, cell, ',')) {
	    ret.push_back(std::stod(cell));
	    ++cells;
	}
	++rows;
    }
    ifstream.close();
    if (rows != width || cells / rows != height) {
	ret.clear();
	ret.push_back(.0f);  // HACK Size should be 1
    }
    return ret;
}

int CsvExportMatrix(const std::string& path, const std::vector<double>& matrix, uint width, uint height) {
    // MAYBE Throw an exception instead of returning errno C-style?
    std::ofstream ofstream;
    ofstream.open(path, std::ofstream::out);
    if (ofstream.bad()) return errno;
    std::string line;
    for (uint r = 0; r < width; r++) {
	for (uint c = 0; c < height; c++) ofstream << matrix.at(c + (height * r)) << ", ";
	ofstream << std::endl;
    }
    ofstream.close();
    return 0;
}
