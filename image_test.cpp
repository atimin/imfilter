//
// Created by flipback on 3/29/20.
//

#include <fstream>
#include <iostream>
#include "CImg/CImg.h"
#include "image_filter.h"

using namespace metric;
using namespace cimg_library;
using namespace std::chrono;
int main() {
	CImg<unsigned char> input("img.png");

	std::cout << "Read file" << std::endl;

	auto rgbImg = iminit<uint8_t, 3>(input.height(), input.width(), 50);
	for (int ch = 0; ch < rgbImg.size(); ++ch) {
		for (int i = 0; i < rgbImg[ch].rows(); ++i) {
			for (int j = 0; j < rgbImg[ch].columns(); ++j) {
				rgbImg[ch](i, j) = input(j, i, 0, ch);
			}
		}
	}

	std::cout << "Created image " << rgbImg[0].rows() << "x" << rgbImg[0].columns() << " " << std::endl;

	imfilter<uint8_t, 3, FilterType::GAUSSIAN, PadDirection::BOTH, PadType::CONST> f(3, 3, 0.3);

	auto start = system_clock::now();
	Image<uint8_t, 3> filterImage = f(rgbImg);

	std::cout << "Filtered image for " << duration_cast<milliseconds>(system_clock::now() -start).count() << "ms" << std::endl;
	CImg<unsigned char> out(filterImage[0].columns(), filterImage[0].rows(), 1, 3);
	for (int ch = 0; ch < filterImage.size(); ++ch) {
		for (int i = 0; i < filterImage[ch].rows(); ++i) {
			for (int j = 0; j < filterImage[ch].columns(); ++j) {
				out(j, i, 0, ch) = filterImage[ch](i, j);
			}
		}
	}
	std::cout << "Saved result" << std::endl;

	out.save_bmp("img2.bmp");
	std::cout << "Wrote file" << std::endl;

	return 0;
}