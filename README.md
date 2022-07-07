# F1801Q128-DCT

Project for the course "Methods for Scientific Computing":comparison of an homegrown DCT implementation with the one included in OpenCV and implementation of a grayscale bitmap image compressor using OpenCV.

## DCTToolbox

DCTToolbox is a graphical application implemented using Dear ImGui, an immediate mode graphical user interface library that is widely used for writing fast and unobtrusive interfaces. The application is built against ImGui v1.88 using SDL2 and OpenGL 2 as its backend and rendering library. Additionally, we have chosen to include the stbi_image library to load data from bitmaps since it's already been tested extensively with ImGui.

## Prerequisites

While ImGui and stbi_image are included as Git Submodules and built along with the project, the other libraries are fetched by CMake from the target. Please make sure you have the following installed before attempting to use DCTToolbox:

- OpenCV >= 4.5.3
- SDL 2 >= 2.0.22
- OpenGL 2 

different versions of those libraries might or might not work.

## Building

To build the project from scratch you'll need:
 
- Git
- CMake
- GCC

Do the following:
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

Upon clicking the _Go!_ button, the program will subdivide the image in as many $F\times F$ sized chunks as possible while discarding any non-full chunk. The chunks are then converted into OpenCV-compatible matrices on which the forward DCT transform (DCT-II) is performed. At this point, the frequencies below the diagonal identified by $d$ are cut using the following algorithm:

```pascal
procedure cut on d as const integer, mat as cv::Mat is
	repeat while i is < mat->rows do
		repeat while j is < mat->cols do
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

Due to a limitation of `stbi_image_info()` and `stbi_image_load()`, compressed BMP images are not supported; thus, the following files from the test image set cannot be loaded:

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

Due to a limitation of `ImGui::SliderInt()` we haven't been able to make the slider skip odd values even if they can't be used because odd-sized inputs are unsupported by ``cv::dct()``.

### DCT Benchmark

This window allows the user to demonstrate the functionality of both `cv::dct()` and the functions provided by us on matrices loaded from CSV files. The results can then be exported to a CSV file to load them into an external editor.

Moreover, in the Benchmarking section, a benchmark can be performed by running the algorithms on several, randomly generated, matrices of growing size. The timing is performed by using `h_time.h`, a wrapper around Linux's `clock_gettime(3)` system call that has been developed in-house and has been successfully used to measure the scheduling jitter of a real-time workload with very high precision, and can then be exported to CSV for plotting and further analysis.

### Random Matrix Generator

This window allows the user to generate random square matrices of varying size and export them to CSV for further processing. This tool has been created to aid with the development of the Benchmarking tool and has been subsequently left in the application.

## `my_dct.{cpp,h}`

These files contain the homegrown $n^3$ implementation of the DCT against which the fast implementation provided by OpenCV is benchmarked.

> TODO Produce Doxygen docs and explain the behavior of `My{M,D}DCT2()`