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

/* https://www.egr.msu.edu/waves/people/Ali_files/DCT_TR802.pdf
 * alpha(u) -> coefficient
 * f(x) -> input vector at x
 * C(u) -> output vector at u
 */

#include "my_dct.h"

#if MYDCT_TRANSPOSE_DEBUG
#include <cstdio>
#endif

#if MYDCT_DDCT2_DEBUG
#include <cassert>
#endif

/**
 * Computes the coefficients for the transform. The DC (Direct Coefficient) is the average value of the sample sequence and is
 * applied to the first waveform, while the AC (Indirect Coefficients) are applied to other waveforms. This is the step that is
 * leveraged by the JPEG standard via carefully crafted coefficient matrices to efficiently compress the Blue and Red Chrominance
 * channel in a YCbCr image with minimal loss of quality.
 * @param u The current waveform index.
 * @param N The input vector's width.
 * @return The AC or DC.
 */
inline double MyDCTCoeff(unsigned u, unsigned N) {
    if (u == 0) {
	return (sqrt(1 / (double)N));
    } else {
	return (sqrt(2 / (double)N));
    }
}

/**
 * Computes a singular DCT sum.
 * @param in The input row.
 * @param u The current waveform index (aka the row of the input matrix).
 * @param n The input's width.
 * @return The DCT sum for the current input.
 */
inline double MyDCTSum(const std::vector<double>& in, unsigned u, unsigned n) {
    double sum = .0f;
    for (unsigned x = 0; x < n; x++) {
	sum += in.at(x) * cos((M_PI * (2 * x + 1) * u) / (2 * n));
    }
    return sum;
}

/**
 * Implements a mono-dimensional DCT2 transform. Given that the sum is as wide as the size of the input vector and it's performed
 * for each of the input values, the function takes a time asymptotically equivalent to in.size()^2.
 * @param in The input vector.
 * @return A vector containing the DCT of the input.
 */
std::vector<double> MyMDCT2(const std::vector<double>& in) {
    unsigned N = in.size();
    std::vector<double> out(N);
    for (unsigned u = 0; u < N; u++) {
	out.at(u) = MyDCTCoeff(u, N) * MyDCTSum(in, u, N);
    }
    return out;
};

/**
 * Transpose a matrix.
 * @param in The matrix to transpose.
 * @param n The width of the matrix.
 * @return The transposed of the input.
 */
std::vector<double> MyDCTTranspose(const std::vector<double>& in, unsigned n) {
    std::vector<double> out(n*n);
    for (unsigned r = 0; r < n; r++) {      // u < height
	for (unsigned c = 0; c < n; c++) {  // v < width
	    out.at(c + (n * r)) = in.at(r + (n * c));
	}
    }
#if MYDCT_TRANSPOSE_DEBUG
    for (unsigned r = 0; r < n; r++) {      // u < height
	for (unsigned c = 0; c < n; c++) {  // v < width
	    printf("%g\t", in.at(c + (n * r)));
	}
	puts("");
    }
    puts("");
    for (unsigned r = 0; r < n; r++) {      // u < height
	for (unsigned c = 0; c < n; c++) {  // v < width
	    printf("%g\t", out.at(c + (n * r)));
	}
	puts("");
    }
#endif
    return out;
}

/**
 * Computes a single pass of DCT transform on rows.
 * @param in The matrix to perform the transform on.
 * @param n The height of the matrix (and its width, since it's square).
 * @return The matrix containing the single-pass DCT transform of the input.
 */
inline std::vector<double> MyDDCT2Pass(const std::vector<double>& in, unsigned n) {
    std::vector<double> temp(n), out;
    for (unsigned u = 0; u < n; u++) { // u < height
	temp = {in.begin()+(n*u), in.begin()+n+(n*u)};
	temp = MyMDCT2(temp);
	out.insert(out.end(), temp.begin(), temp.end());
    }
    return out;
}

/**
 * Due to a property known as "separability", a multi-dimensional DCT2 can be implemented as the product of its mono-dimensional
 * steps. This property allows us to considerably reduce the processing time, confronted with the "dumb" version (not
 * implemented here).
 * @param in The input vector (a n*n matrix).
 * @param n The height of the matrix (and its width, since it's square).
 * @return A vector containing the DCT of the input (a n*n matrix).
 */
std::vector<double> MyDDCT2(const std::vector<double>& in, unsigned n) {
#if MYDCT_DDCT2_DEBUG
    // assuming in is a matrix, ensure the size is correct
    unsigned N = n * n;  // we only support square matrices, thus width = height
    assert(in.size() == N);
#endif
    std::vector<double> step, out;
    out = MyDCTTranspose(MyDDCT2Pass(in, n), n);
    out = MyDCTTranspose(MyDDCT2Pass(out, n), n);
    // n*n*(2*n)~=n^3
    return out;
}