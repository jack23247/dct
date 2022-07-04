//
// Created by quartz on 30/06/22.
//

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_JPEG
#define STBI_NO_PNG
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM

#include "img_compressor.h"

#include <GL/gl.h>
#include <string>
#include <stdexcept>

#include "h_time.h"
#include "stb_image.h"
#include "opencv2/opencv.hpp"

struct ChunkCoord {
    unsigned x, id_x, y, id_y, chunks_per_side;
};

ChunkCoord toChunkCoord(unsigned img_col, unsigned img_row, unsigned img_width, unsigned chunk_width) {
    ChunkCoord ret{};
    auto chunks_per_side = static_cast<unsigned>(floor(img_width / (double)chunk_width));
    unsigned chunk_begin; int chunk_id_x = -1, chunk_id_y = -1;
    while(chunk_begin <= img_col) {
	chunk_begin += chunk_width;
	chunk_id_x++;
    }
    ret.x = img_col - chunk_width * chunk_id_x;
    ret.id_x = chunk_id_x;
    chunk_begin = 0;
    while(chunk_begin <= img_row) {
	chunk_begin += chunk_width;
	chunk_id_y++;
    }
    ret.y = img_row - chunk_width * chunk_id_y;
    ret.id_y = chunk_id_y;
    ret.chunks_per_side = chunks_per_side;
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
	if(!ok) throw std::runtime_error("stbi_info(): failure."); // FIXME Meaningful message
	auto buf = stbi_load(m_path.c_str(), &(data.rows), &(data.cols), nullptr, 1);
	if (buf == nullptr) throw std::runtime_error("stbi_load(): failure.");
	//~data;
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

    std::vector<double> extractChunk(unsigned chunk_width, unsigned cur_column, unsigned cur_row) const {
	std::vector<double> ret;
	for(int row = 0; row < chunk_width; row++) {
	    for(int col = 0; col < chunk_width; col++) {
		ret.push_back(static_cast<double>(data.at<unsigned char>(row + (chunk_width * cur_column), col + (chunk_width * cur_row))));
	    }
	}
	return ret;
    }

    void makeCompressedOf(const Image& from_img, int chunk_width, int cutoff) {
	// Subdivide image in chunks
	std::vector<std::vector<double>> chunks_in;
	int chunks_per_side = static_cast<int>(ceil(from_img.getWidth() / (double)chunk_width));
	for(int row = 0; row < chunks_per_side; row++) {
	    for(int col = 0; col < chunks_per_side; col++) {
		chunks_in.push_back(from_img.extractChunk(chunk_width, col, row));
	    }
	}
	int cur = 0;
	for(auto chunk : chunks_in) {
	    cv::Mat chunk_data = cv::Mat(chunk_width, chunk_width, CV_64F, &chunk.front());
	    cv::Mat temp = cv::Mat(chunk_width, chunk_width, CV_64F);
	    cv::dct(chunk_data, temp);
	    for (int row = 0; row < temp.rows; row++) {
		for (int col = 0; col < temp.cols; col++) {
		    if ((col + row) >= cutoff) temp.data[temp.step * row + col] = 0;
		}
	    }
	    chunk_data.deallocate();
	    chunk_data.reserve(chunk_width*chunk_width);
	    cv::idct(temp, chunk_data);
	    /*
	    puts("\nBEGINNING OF CHUNK");
	    print(chunk_data);
	    puts("\nEND OF CHUNK");
	    */
	    chunks_in.at(cur).assign(chunk_data.begin<double>(), chunk_data.end<double>());
	    cur++;
	}
	// Repack the chunks
	//~data;
	data = cv::Mat(from_img.getWidth(), from_img.getHeight(), CV_8U);
	auto buf = static_cast<unsigned char*>(std::malloc(data.cols * data.rows));
	ChunkCoord cur_chunk{};
	for(int row = 0; row < from_img.getWidth(); row++) {
	    for (int col = 0; col < from_img.getWidth(); col++) {
		cur_chunk = toChunkCoord(col, row, from_img.getWidth(), chunk_width);
		*(buf + (col + from_img.getWidth() * row)) =
		    static_cast<unsigned char>(chunks_in.at((cur_chunk.id_y + cur_chunk.chunks_per_side * cur_chunk.id_x))
		                                   .at(cur_chunk.y + chunk_width * cur_chunk.x));
	    }
	}
	std::memcpy(data.data, buf, data.cols*data.rows);
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
    if(ImGui::Button("Reset")) {
	from.reset();
	from_loaded = false;
	to.reset();
	to_ready = false;
	snprintf((char*)&io_status_msg, 512, "Ready.");
    }
    if(from_loaded) {
	ImGui::Separator();
	ImGui::Text("Compression Parameters:");
	ImGui::SliderInt("Chunk Size", &chunk_size, 2, 64);
	ImGui::SliderInt("Frequency Cutoff", &cutoff, 0, 2*chunk_size-2);
	if(ImGui::Button("Go!")) {
	    if(chunk_size % 2 == 0) {
		to.reset();
		to.makeCompressedOf(from, chunk_size, cutoff);
		to_ready = true;
	    } else {
		ImGui::Text("Please select an even chunk size!");
	    }
	}
    }
    {
	ImGui::Begin("Source Image", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	if(from_loaded) {
	    ImGui::Image((void*)(intptr_t)from.getTexture(), from.getSize());
	    ImGui::Separator();
	    ImGui::Text("Filename: %s", from.getPath().c_str());
	    ImGui::Text("Width (px): %d, Height (px): %d", from.getWidth(), from.getHeight());
	} else {
	    ImGui::Text("No Bitmap loaded.");
	}
	ImGui::End();
    }
    {
	ImGui::Begin("Compressed Image", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	if(to_ready) {
	    ImGui::Image((void*)(intptr_t)to.getTexture(), to.getSize());
	    ImGui::Separator();
	    ImGui::Text("Width (px): %d, Height (px): %d", to.getWidth(), to.getHeight());
	} else {
	    ImGui::Text("Nothing compressed yet.");
	}
	ImGui::End();
    }
    ImGui::Separator();
    ImGui::TextWrapped("%s", io_status_msg);
    ImGui::End();
}