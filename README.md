# F1801Q128-DCT

Authors:

- Jacopo Maltagliati - 830110
- Alessandro Albi - MATMAT

Project for the course "Methods for Scientific Computing":comparison of an homegrown DCT implementation with the one included in OpenCV and implementation of a grayscale bitmap image compressor using OpenCV.

## DCTToolbox

DCTToolbox is a graphical application implemented using _Dear ImGui_, an immediate mode graphical user interface library that is widely used for writing fast and unobtrusive interfaces. The application is built against ImGui v1.88 using SDL2 and OpenGL 2 as its backend and rendering library. 

Additionally, we have chosen to include:

- the OpenCV library, used to deal with matrices and images and to perform the fast DCT-II and DCT-III transforms,
- the _stbi_image_ library, used to load data from bitmaps since it's already been tested extensively with ImGui,
- and the _rt-app_ project, which includes a timing function that has been developed by one of the authors and has been successfully used to measure the scheduling jitter of a real-time workload with very high precision.

## Prerequisites

While ImGui, stbi_image and rt-app are included as submodules and built along with the project, the other libraries are fetched by CMake from the target. Please make sure you have the following installed before attempting to use DCTToolbox:

- OpenCV >= 4.5.3
- SDL 2 >= 2.0.22
- OpenGL 2 

Different versions of those libraries might or might not work.

## Building

To build the project from scratch you'll need:
 
- the `git` version control system,
- the CMake and GNU Make build systems,
- the GNU Compiler Collection (specifically the `g++` compiler).

Once you have installed those packages using your preferred distribution's package manager, you can issue the following commands to fetch the source code and build the application:

```
git clone https://gitlab.com/jack23247/f1801q128-dct
cd f1801q128-dct
cd deps && ./mkdeps && cd ..
cmake .
make
```

This will produce an executable called `proj2` in the project's root folder.

## Usage

### Tool Selector

When launching `proj2` the user will be presented with a gray-ish window containing a smaller window called "Tool Selector": this is the main window of the application, from where several tools can be chosen.

#### Image Compressor Checkbox 

Shows the "Image Compressor" tool, that allows the user to select a grayscale bitmap image to compress using OpenCV's ``cv::dct()``.

#### DCT Benchmark Checkbox

Shows the "DCT Benchmark" window, where the user can try both ``cv::dct()`` and ``MyDCT2`` on the contents of a CSV file, and perform a benchmark on randomly generated matrices and output the results to a different CSV file.

#### Random Matrix Generator

Shows the "Random Matrix Generator" window, where the user can generate and export random matrices to a CSV file.

#### Information Section

This section contains various informations regarding DCTToolbox, such as the version of the application, the licenses and authors of the libraries (under the "Open Source Libraries" collapsible section) that have been used and ImGui's performance metrics (under the "Debug Information" collapsible section).

### Image Compressor

This window allows the user to load a non-compressed grayscale Bitmap image (via `stbi_image_load()`) on which to perform the DCT compression. 

The user can specify both the _Chunk Size_ ($F$) and _Frequency Cutoff_ ($d$) parameters that will be used to perform the compression.

#### The compression process

Upon clicking the **Go!** button, the program will subdivide the image in as many $F\times F$ sized chunks as possible while discarding any non-full chunk. The chunks are then converted into OpenCV-compatible matrices on which the forward DCT transform (DCT-II) is performed. At this point, the frequencies below the diagonal identified by $d$ are cut using the following algorithm:

```pascal
procedure cut on d as const integer, mat as cv::Mat is
	while i is < mat->rows do
		while j is < mat->cols do
			if i + j is >= d	
				mat[i, j] <- 0
			end if
		end while
	end while
end procedure
```

...then the inverse DCT transform (DCT-III) is applied to each chunk and the chunks are reassembled by iterating on them and copying the data to an array containing the resulting image by mapping their coordinates into the array's system.

> TODO Give pseudo for chunk unpacking and repacking

#### Caveats

##### Some images won't load

Due to a limitation of the `stbi_image_info()` and `stbi_image_load()` provided by the homonymous library, compressed BMP images are not supported; thus, the following files from the test image set cannot be loaded natively:

<pre>$ file *.bmp | grep compression
bridge.bmp:     PC bitmap, Windows 3.x format, 2749 x 4049 x 8, 1 <font color="#FF0000"><b>compression</b></font>, image size 20312382, resolution 2834 x 2834 px/m, 256 important colors, cbSize 20313460, bits offset 1078
cathedral.bmp:  PC bitmap, Windows 3.x format, 2000 x 3008 x 8, 1 <font color="#FF0000"><b>compression</b></font>, image size 10016376, resolution 2834 x 2834 px/m, 256 important colors, cbSize 10017454, bits offset 1078
</pre>

The problem can be worked around by passing the image through an editor such as GIMP to remove the compression:

<pre>$ file bridge*.bmp
bridge1.bmp: PC bitmap, Windows 3.x format, 2749 x 4049 x 8, image size 11142848, resolution 2834 x 2834 px/m, 256 important colors, cbSize 11143926, bits offset 1078
bridge.bmp:  PC bitmap, Windows 3.x format, 2749 x 4049 x 8, 1 compression, image size 20312382, resolution 2834 x 2834 px/m, 256 important colors, cbSize 20313460, bits offset 1078
</pre>

...the file `bridge1.bmp` can be successfully opened by DCTToolbox, unlike `bridge.bmp`.

##### The Chunk Size slider allows odd values

Due to a limitation of `ImGui::SliderInt()` we haven't been able to make the slider skip odd values even if they can't be used because odd-sized inputs are unsupported by ``cv::dct()`` (and they don't make a lot of sense either).

> TODO odd value dct is the dst?

### DCT Benchmark

This window allows the user to demonstrate the functionality of both `cv::dct()` and the functions provided by us (implemented in `my_dct.cpp`) on matrices loaded from CSV files. The results can then be exported to a CSV file to load them into an external editor.

Moreover, in the Benchmarking section, a benchmark can be performed by running the algorithms on several, randomly generated, matrices of growing size. The timing is performed by using `h_time.h`, a wrapper around Linux's `clock_gettime(3)` system call that's been imported from the aforementioned rt-app project, and can then be exported to CSV for plotting and further analysis.

### Random Matrix Generator

This window allows the user to generate random square matrices of varying size and export them to CSV for further processing. This tool has been created to aid with the development of the Benchmarking tool and has been subsequently left in the application.

## Benchmarking

These files contain the homegrown $n^3$ implementation of the DCT against which the fast implementation provided by OpenCV is benchmarked.

### OpenCV's `cv::dct()`

Developed by Intel and Willow Garage, OpenCV is the golden standard in open-source Computer Vision libraries. Officially launched in 1999, this library has been extensively used in countless commercial and research projects involving robotics, machine vision and machine learning. OpenCV technically supports CPU acceleration via SIMD extensions and GPU acceleration via OpenCL and CUDA.

The reason why we choose OpenCV is its strong focus on performance and scalability, excellent documentation, rich history, and ease of integration with CMake. Moreover, OpenCV is written in C++, meaning we could natively leverage the `cv::Mat` class, that provides a fast and featureful implementation of matrices with a specific focus on storing image data and `cv::dct()`, a fast implementation of the DCT (both II and III) which was employed for benchmarking and compression purposes.

### `my_dct.{cpp,h}`

This is a homegrown implementation of the DCT that aims to 

> ***NOTE***<br>Portions of the following sections are automatically generated by Doxygen from the comments contained in the source files.

#### ◆ MyDCTCoeff()

Computes the coefficients for the transform. The DC (Direct Coefficient) is the average value of the sample sequence and is applied to the first waveform, while the AC (Indirect Coefficients) are applied to other waveforms. This is the step that is leveraged by the JPEG standard via carefully crafted coefficient matrices to efficiently compress the Blue and Red Chrominance channel in a YCbCr image with minimal loss of quality.

##### Parameters  

|     |                             |
|-----|-----------------------------|
| u   | The current waveform index. |
| N   | The input vector's width.   |

##### Returns  
The AC or DC.

##### Source
```c++
inline double MyDCTCoeff(unsigned u, unsigned N) {
    if (u == 0) {
		return (sqrt(1 / (double)N));
    } else {
		return (sqrt(2 / (double)N));
    }
}
```

#### ◆ MyDCTSum()

Computes a singular DCT sum.

##### Parameters  

|     |                                                               |
|-----|---------------------------------------------------------------|
| in  | The input row.                                                |
| u   | The current waveform index (aka the row of the input matrix). |
| n   | The input's width.                                            |

##### Returns  
The DCT sum for the current input.

##### Source
```c++
inline double MyDCTSum(const std::vector<double>& in, unsigned u, unsigned n) {
    double sum = .0f;
    for (unsigned x = 0; x < n; x++) {
		sum += in.at(x) * cos((M_PI * (2 * x + 1) * u) / (2 * n));
    }
    return sum;
}
```

#### ◆ MyMDCT2()


Implements a mono-dimensional DCT2 transform. Given that the sum is as wide as the size of the input vector and it's performed for each of the input values, the function takes a time asymptotically equivalent to in.size()^2.

##### Parameters  

|     |                   |
|-----|-------------------|
| in  | The input vector. |

##### Returns  
A vector containing the DCT of the input.

##### Source
```c++
std::vector<double> MyMDCT2(const std::vector<double>& in) {
    unsigned N = in.size();
    std::vector<double> out(N);
    for (unsigned u = 0; u < N; u++) {
		out.at(u) = MyDCTCoeff(u, N) * MyDCTSum(in, u, N);
    }
    return out;
};
```
#### ◆ MyDCTTranspose()

Transpose a matrix.

##### Parameters  

|     |                          |
|-----|--------------------------|
| in  | The matrix to transpose. |
| n   | The width of the matrix. |

##### Returns  
The transposed of the input.

##### Source
```c++
std::vector<double> MyDCTTranspose(const std::vector<double>& in, unsigned n) {
    std::vector<double> out(n*n);
    for (unsigned r = 0; r < n; r++) {      // u < height
		for (unsigned c = 0; c < n; c++) {  // v < width
	    	out.at(c + (n * r)) = in.at(r + (n * c));
		}
    }
    return out;
}
```

#### ◆ MyDDCT2Pass()

Computes a single pass of DCT transform on rows.

##### Parameters  

|     |                                                              |
|-----|--------------------------------------------------------------|
| in  | The matrix to perform the transform on.                      |
| n   | The height of the matrix (and its width, since it's square). |

##### Returns  
The matrix containing the single-pass DCT transform of the input.

##### Source
```c++
inline std::vector<double> MyDDCT2Pass(const std::vector<double>& in, unsigned n) {
    std::vector<double> temp(n), out;
    for (unsigned u = 0; u < n; u++) { // u < height
		temp = {in.begin()+(n*u), in.begin()+n+(n*u)};
		temp = MyMDCT2(temp);
		out.insert(out.end(), temp.begin(), temp.end());
    }
    return out;
}

```

#### ◆ MyDDCT2()

Due to a property known as "separability", a multi-dimensional DCT2 can
be implemented as the product of its mono-dimensional steps. This
property allows us to considerably reduce the processing time,
confronted with the "dumb" version (not implemented here).

##### Parameters 
 
|     |                                                              |
|-----|--------------------------------------------------------------|
| in  | The input vector (a n\*n matrix).                            |
| n   | The height of the matrix (and its width, since it's square). |

##### Returns  
A vector containing the DCT of the input (a n\*n matrix).

##### Source
```c++
std::vector<double> MyDDCT2(const std::vector<double>& in, unsigned n) {
    std::vector<double> step, out;
    out = MyDCTTranspose(MyDDCT2Pass(in, n), n);
    out = MyDCTTranspose(MyDDCT2Pass(out, n), n);
    return out;
}
```


### Benchmark Results

The timing is recorded by taking a "snapshot" of the current system time as reported by the `clock_gettime(3)` system call before and after the execution of `cv::dct()` and `MyDDCT2()` on a randomly generated matrix of size $2^n$. The test is performed several times, by incrementing $n$ from 3 to the value set by the user using the slider. The same matrix is used for each algorithm on each iteration, even if the nature of the data should not affect the execution time of the DCT transform (at least in our "slow" version).

The results have then been exported to a CSV file and a chart has been generated with LibreOffice Calc on a semilogaritmic scale, as required by the project's specification. 

![](docs/bench_data/bench_126_steps.png)


