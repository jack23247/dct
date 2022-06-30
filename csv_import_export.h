//
// Created by quartz on 27/06/22.
//

#ifndef PROJ2_CSV_IMPORT_EXPORT_H
#define PROJ2_CSV_IMPORT_EXPORT_H

#include <string>
#include <vector>

std::vector<double> csvImportMatrix(const std::string&, unsigned, unsigned);
void csvExportMatrix(const std::string&, const std::vector<double>&, unsigned, unsigned);

#endif  // PROJ2_CSV_IMPORT_EXPORT_H
