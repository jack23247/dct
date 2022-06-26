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

#include "my_dct.h"

#include <cassert>
#include <cstdlib>
#include <vector>
#include <cmath>

/* https://www.egr.msu.edu/waves/people/Ali_files/DCT_TR802.pdf
 * alpha(u) -> coefficient
 * f(x) -> input vector at x
 * C(u) -> output vector at u
 */

/**
 * Computes the coefficients for the transform. The DC (Direct Coefficient) is the average value of the sample sequence and is
 * applied to the first waveform, while the AC (Indirect Coefficients) are applied to other waveforms. This is the step that is
 * leveraged by the JPEG standard via carefully crafted coefficient matrices to efficiently compress the Blue and Red Chrominance
 * channel in a YCbCr image with minimal loss of quality.
 * @param u The current waveform index
 * @param N The input vector's width
 * @return
 */
inline double MyDCTCoeff(uint u, uint N) {
    if (u == 0) {
	return (sqrt(1 / (double)N));
    } else {
	return (sqrt(2 / (double)N));
    }
}

/**
 * Computes a singular DCT sum.
 * @param in The input vector
 * @param u The current waveform index
 * @param N The input vector's width
 * @return The DCT sum for the current waveform.
 */
inline double MyDCTSum(const std::vector<double>& in, uint u, uint N) {
    double sum = .0f;
    for (uint x = 0; x < N; x++) {
	sum += in.at(x) * cos((M_PI * (2 * x + 1) * u) / 2 * N);
    }
    return sum;
}

/**
 * Implements a mono-dimensional DCT2 transform. Given that the sum is as wide as the size of the input vector and it's performed
 * for each of the input values, the function takes a time asymptotically equivalent to in.size()^2.
 * @param in The input vector
 * @return A vector containing the DCT of the input
 */
std::vector<double> MyMDCT2(const std::vector<double>& in) {
    uint N = in.size();
    std::vector<double> out(N);
    for (uint u = 0; u < N; u++) {
	out.at(u) = MyDCTCoeff(u, N) * MyDCTSum(in, u, N);
    }
    return out;
};

/**
 * Due to a property known as "separability", a multi-dimensional DCT2 can be implemented as the product of its mono-dimensional
 * steps. This property allows us to considerably reduce the processing time, confronted with the "dumb" version (not
 * implemented here).
 * @param in The input vector (a n*n matrix)
 * @param n The width (and height, since it's square) of the matrix
 * @return A vector containing the DCT of the input (a n*n matrix)
 */
std::vector<double> MyDDCT2(const std::vector<double>& in, uint n) {
    // assuming in is a matrix, ensure the size is correct
    uint N = n * n; // we only support square matrices, thus width = height
    assert(in.size() == N);
    std::vector<double> out(N);
    for (uint u = 0; u < n; u++) { // u < height
	for (uint v = 0; v < n; v++) { // v < width
	    out.at(u) = MyDCTCoeff(u, N) * MyDCTCoeff(u, N) * MyDCTSum(in, u, N) * MyDCTSum(in, u, N); // n*n*(2*n)~=n^3
	}
    }
    return out;
}