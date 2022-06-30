//
// Created by quartz on 27/06/22.
//

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
