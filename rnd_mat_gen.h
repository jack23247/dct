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

#ifndef PROJ2_RND_MAT_GEN_H
#define PROJ2_RND_MAT_GEN_H

#define RND_MAT_GEN_WINDOW_TITLE "Random Matrix Generator"

#include <random>
#include <vector>

inline std::vector<double> genRndMat(unsigned mat_width, unsigned mat_height) {
    static std::random_device rd;
    static std::mt19937 mt(rd());
    static std::uniform_real_distribution<double> dist(-999.0, +1000.0);
    std::vector<double> mat;
    unsigned cur = mat_width * mat_height;
    while (cur > 0) {
	mat.push_back(dist(mt));
	cur--;
    }
    return mat;
}

void rndMatGenWindow(bool*);

#endif  // PROJ2_RND_MAT_GEN_H
