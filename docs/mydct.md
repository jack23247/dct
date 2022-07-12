## Function Documentation

## ◆ MyDCTCoeff()

Computes the coefficients for the transform. The DC (Direct Coefficient) is the average value of the sample sequence and is applied to the first waveform, while the AC (Indirect Coefficients) are applied to other waveforms. This is the step that is leveraged by the JPEG standard via carefully crafted coefficient matrices to efficiently compress the Blue and Red Chrominance channel in a YCbCr image with minimal loss of quality.

Parameters  

|     |                             |
|-----|-----------------------------|
| u   | The current waveform index. |
| N   | The input vector's width.   |

Returns  
The AC or DC.

## ◆ MyDCTSum()

Computes a singular DCT sum.

Parameters  

|     |                                                               |
|-----|---------------------------------------------------------------|
| in  | The input row.                                                |
| u   | The current waveform index (aka the row of the input matrix). |
| n   | The input's width.                                            |

Returns  
The DCT sum for the current input.

## ◆ MyDCTTranspose()

Transpose a matrix.

Parameters  

|     |                          |
|-----|--------------------------|
| in  | The matrix to transpose. |
| n   | The width of the matrix. |

Returns  
The transposed of the input.


## ◆ MyDDCT2()

Due to a property known as "separability", a multi-dimensional DCT2 can
be implemented as the product of its mono-dimensional steps. This
property allows us to considerably reduce the processing time,
confronted with the "dumb" version (not implemented here).

Parameters 
 
|     |                                                              |
|-----|--------------------------------------------------------------|
| in  | The input vector (a n\*n matrix).                            |
| n   | The height of the matrix (and its width, since it's square). |

Returns  
A vector containing the DCT of the input (a n\*n matrix).


## ◆ MyDDCT2Pass()

Computes a single pass of DCT transform on rows.

Parameters  

|     |                                                              |
|-----|--------------------------------------------------------------|
| in  | The matrix to perform the transform on.                      |
| n   | The height of the matrix (and its width, since it's square). |

Returns  
The matrix containing the single-pass DCT transform of the input.

## ◆ MyMDCT2()


Implements a mono-dimensional DCT2 transform. Given that the sum is as wide as the size of the input vector and it's performed for each of the input values, the function takes a time asymptotically equivalent to in.size()^2.

Parameters  

|     |                   |
|-----|-------------------|
| in  | The input vector. |

Returns  
A vector containing the DCT of the input.