//
// Created by quartz on 27/06/22.
//

#ifndef PROJ2_CSV_IMPORT_EXPORT_H
#define PROJ2_CSV_IMPORT_EXPORT_H

#include <string>
#include <vector>

std::vector<double> CsvImportMatrix(const std::string& path, uint width, uint height);
int CsvExportMatrix(const std::string& path, const std::vector<double>& matrix, uint width, uint height);

#endif  // PROJ2_CSV_IMPORT_EXPORT_H
