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

#include "img_compressor.h"

#include <GL/gl.h>

#include <stdexcept>
#include <string>

#include "h_time.h"
#include "opencv2/opencv.hpp"
#include "stb_image.h"

struct ChunkCoord {
    int vertical_chunks, vertical_idx, col, horizontal_chunks, horizontal_idx, row;
};

inline ChunkCoord toChunkCoord(int img_col, int img_row, int img_width, int img_height, int chunk_width) {
    ChunkCoord ret{};
    ret.vertical_chunks = static_cast<int>(floor(img_height / (double)chunk_width));
    ret.horizontal_chunks = static_cast<int>(floor(img_width / (double)chunk_width));
    ret.vertical_idx = img_row / chunk_width;
    ret.horizontal_idx = img_col / chunk_width;
    ret.row = img_row % chunk_width;
    ret.col = img_col % chunk_width;
    return ret;
}

class Image {
   private:
    cv::Mat data;
    std::string path;
    GLuint texture{};

   public:
    Image() {
	data = cv::Mat(0, 0, CV_8U);
	path = "";
	texture = 0;
    };
    virtual ~Image() = default;

    int getHeight() const { return data.rows; }
    int getWidth() const { return data.cols; }
    const std::string& getPath() const { return path; }
    GLuint getTexture() {
	if (texture == 0) {
	    glGenTextures(1, &texture);
	    glBindTexture(GL_TEXTURE_2D, texture);
	    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef PIPPO
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
#endif
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, data.cols, data.rows, 0, GL_RED, GL_UNSIGNED_BYTE, data.data);
	    if (texture == 0) throw std::runtime_error("Unable to create an OpenGL Texture");
	}
	return texture;
    }
    ImVec2 getSize() const { return {static_cast<float>(data.cols), static_cast<float>(data.rows)}; }
    unsigned char* getRawData() const { return data.data; }

    void load(const std::string& m_path) {
	int ok, rows, cols, comp;
	ok = stbi_info(m_path.c_str(), &rows, &cols, &comp);
	if (!ok) throw std::runtime_error("Unable to locate or decode the specified image file.");
	auto buf = stbi_load(m_path.c_str(), &cols, &rows, nullptr, 1);
	if (buf == nullptr) throw std::runtime_error("An error occured while loading the specified image file.");
	data = cv::Mat(rows, cols, CV_8U);
	std::memcpy(data.data, buf, data.rows * data.cols);
	stbi_image_free(buf);
	this->path = m_path;
    }

    void reset() {
	path = "";
	glDeleteTextures(1, &texture);
	texture = 0;
    }

    std::vector<double> extractChunk(int chunk_width, int chunk_id_y, int chunk_id_x) const {
	std::vector<double> ret;
	int x, y;
	for (int row = 0; row < chunk_width; row++) {
	    y = row + (chunk_width * chunk_id_y);
	    for (int col = 0; col < chunk_width; col++) {
		x = col + (chunk_width * chunk_id_x);
		// MAYBE try/catch?
		ret.push_back(static_cast<double>(data.at<unsigned char>(y, x)));
	    }
	}
	return ret;
    }

    void makeCompressedOf(const Image& from_img, int chunk_width, int diag_cut) {
	// Subdivide image in chunks
	std::vector<std::vector<double>> chunks_in;
	auto vertical_chunks = static_cast<int>(floor(from_img.getHeight() / (double)chunk_width));
	auto horizontal_chunks = static_cast<int>(floor(from_img.getWidth() / (double)chunk_width));
	for (int row = 0; row < vertical_chunks; row++) {
	    for (int col = 0; col < horizontal_chunks; col++) {
		chunks_in.push_back(from_img.extractChunk(chunk_width, row, col));
	    }
	}
	int cur = 0;
	// For each chunk
	for (auto chunk : chunks_in) {
	    // Allocate two buffers for the chunk's data
	    cv::Mat mat1 = cv::Mat(chunk_width, chunk_width, CV_64F, &chunk.front());
	    cv::Mat mat2 = cv::Mat(chunk_width, chunk_width, CV_64F);
	    // Perform the DCT
	    cv::dct(mat1, mat2);
	    // Cut the frequencies below the diagonal
	    for (int row = 0; row < chunk_width; row++) {
		for (int col = 0; col < chunk_width; col++) {
		    if ((col + row) >= diag_cut) mat2.at<double>(row, col) = .0f;
		}
	    }
	    cv::idct(mat2, mat1);
	    chunks_in.at(cur).assign(mat1.begin<double>(), mat1.end<double>());
	    cur++;
	    ~mat1;
	    ~mat2;
	}
	// Repack the chunks
	data = cv::Mat(vertical_chunks * chunk_width, horizontal_chunks * chunk_width, CV_8U);
	auto buf = static_cast<unsigned char*>(std::malloc(data.cols * data.rows));
	if (buf == nullptr) throw std::runtime_error("Cannot allocate the required memory for the data buffer.");
	ChunkCoord cur_chunk{};
	for (int row = 0; row < data.rows; row++) {
	    for (int col = 0; col < data.cols; col++) {
		cur_chunk = toChunkCoord(col, row, data.cols, data.rows, chunk_width);
		double tmp = round(chunks_in.at((cur_chunk.horizontal_idx + cur_chunk.horizontal_chunks * cur_chunk.vertical_idx))
				       .at(cur_chunk.col + chunk_width * cur_chunk.row));
		if (tmp < .0f) {
		    tmp = .0f;
		} else if (tmp > 255.0f) {
		    tmp = 255.0f;
		}
		buf[col + data.cols * row] = static_cast<unsigned char>(tmp);
	    }
	}
	std::memcpy(data.data, buf, data.cols * data.rows);
	std::free(buf);
    }
};

void imgCompressorWindow(bool* visible) {
    static Image from;
    static Image to;
    static bool from_loaded = false;
    static char from_path[128] = "../docs/Immagini/amogus_512.bmp";  // "./prova.bmp";
    static bool to_ready;
    static char io_status_msg[512] = "Ready.";
    static int chunk_size = 8;
    static int cutoff = 0;
    ImGui::Begin(IMG_COMPRESSOR_WINDOW_TITLE, visible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::InputText("##fromPathTextBox", from_path, IM_ARRAYSIZE(from_path));
    ImGui::SameLine();
    if (ImGui::Button("Load Image")) {
	from_loaded = false;
	try {
	    from.reset();
	    from_loaded = false;
	    to.reset();
	    to_ready = false;
	    from.load(from_path);
	    from_loaded = true;
	    snprintf((char*)&io_status_msg, 512, "Image loaded.");
	} catch (std::runtime_error& e) {
	    snprintf((char*)&io_status_msg, 512, "Unable to load image \"%s\". Reason: %s", from_path, e.what());
	}
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
	from.reset();
	from_loaded = false;
	to.reset();
	to_ready = false;
	snprintf((char*)&io_status_msg, 512, "Ready.");
    }
    if (from_loaded) {
	ImGui::Separator();
	ImGui::Text("Compression Parameters:");
	ImGui::SliderInt("Chunk Size", &chunk_size, 2, 100);
	if (chunk_size % 2 != 0) {
	    ImGui::Text("Please select an even chunk size!");
	} else {
	    ImGui::SliderInt("Frequency Cutoff", &cutoff, 0, 2 * chunk_size - 2);
	    if (ImGui::Button("Go!")) {
		to_ready = false;
		to.reset();
		static timespec_t ts;
		static nsec_t ts_start = 0, ts_end = -1;
		static long double elapsed;
		ts_start = HTime_GetNsDelta(&ts);  // Begin timing
		to.makeCompressedOf(from, chunk_size, cutoff);
		ts_end = HTime_GetNsDelta(&ts);  // End timing
		to_ready = true;
		elapsed = static_cast<long double>(ts_end - ts_start);
		snprintf((char*)&io_status_msg, 512, "Last compression took %Lf seconds (%Lf milliseconds).", elapsed / NSEC_PER_SEC, elapsed / NSEC_PER_MSEC);
	    }
	}
    }
    {
	ImGui::Begin("Source Image", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	if (from_loaded) {
	    static float from_zoom = 1.0f;
	    static ImVec2 from_size = from.getSize();
	    if (ImGui::SliderFloat("##zoom", &from_zoom, .1f, 5.0f), ImGuiSliderFlags_Logarithmic) {
		from_size = from.getSize();
		from_size.x *= from_zoom;
		from_size.y *= from_zoom;
	    }
	    ImGui::SameLine();
	    if(ImGui::Button("1x") && from_zoom != 1.0f) {
		from_zoom = 1.0f;
		from_size = to.getSize();
	    }
	    ImGui::SameLine();
	    ImGui::Text("Magnification (x100)");
	    ImGui::Image((void*)(intptr_t)from.getTexture(), from_size);
	    ImGui::Separator();
	    ImGui::Text("Filename: %s", from.getPath().c_str());
	    ImGui::Text("Width (px): %d, Height (px): %d", from.getWidth(), from.getHeight());
	} else {
	    ImGui::Text("Ready.");
	}
	ImGui::End();
    }
    {
	ImGui::Begin("Compressed Image", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	if (to_ready) {
	    static float to_zoom = 1.0f;
	    static ImVec2 to_size = to.getSize();
	    if (ImGui::SliderFloat("##zoom", &to_zoom, .1f, 5.0f), ImGuiSliderFlags_Logarithmic) {
		to_size = to.getSize();
		to_size.x *= to_zoom;
		to_size.y *= to_zoom;
	    }
	    ImGui::SameLine();
	    if(ImGui::Button("1x") && to_zoom != 1.0f) {
		to_zoom	= 1.0f;
		to_size = to.getSize();
	    }
	    ImGui::SameLine();
	    ImGui::Text("Magnification (x100)");
	    ImGui::Image((void*)(intptr_t)to.getTexture(), to_size);
	    ImGui::Separator();
	    ImGui::Text("Width (px): %d, Height (px): %d", to.getWidth(), to.getHeight());
	} else {
	    ImGui::Text("Ready.");
	}
	ImGui::End();
    }
    ImGui::Separator();
    ImGui::TextWrapped("%s", io_status_msg);
    ImGui::End();
}