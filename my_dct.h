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

#ifndef PROJ2_MY_DCT_H
#define PROJ2_MY_DCT_H

#define MYDCT_TRANSPOSE_DEBUG 0
#define MYDCT_DDCT2_DEBUG 0

#include <cmath>
#include <cstdlib>
#include <vector>

std::vector<double> MyMDCT2(const std::vector<double>&);
std::vector<double> MyDDCT2(const std::vector<double>&, unsigned);

#endif  // PROJ2_MY_DCT_H
