//
// Created by flipback on 3/29/20.
//

#include <fstream>
#include <iostream>
#include "CImg/CImg.h"
#include "image_filter.h"

using namespace image_processing;
using namespace cimg_library;
using namespace std::chrono;
int main() {
	CImg<unsigned char> input("img.png");

	std::cout << "Read file" << std::endl;

	Image<RGB> imat(input.height(), input.width(), RGB{50, 50, 50});
	for (int i = 0; i < imat.rows(); ++i) {
		for (int j = 0; j < imat.columns(); ++j) {
			imat(i, j) = RGB{input(j, i, 0, 0), input(j, i, 0, 1), input(j, i, 0, 2)};
		}
	}

	std::cout << "Created matrix " << imat.rows() << "x" << imat.columns() << " " << std::endl;
	MotionFilter filter(50, 45);
	PadModel<RGB> padmodel(PadDirection::BOTH, PadType::CONST);

	auto start = system_clock::now();
	Image<RGB> omat = imfilter(imat, filter, padmodel, false);

	std::cout << "Filtered image for " << duration_cast<milliseconds>(system_clock::now() -start).count() << "ms" << std::endl;
	CImg<unsigned char> out(omat.columns(), omat.rows(), 1, 3);
	for (int i = 0; i < omat.rows(); ++i) {
		for (int j = 0; j < omat.columns(); ++j) {
			out(j, i, 0, 0) = omat(i, j)[0];
			out(j, i, 0, 1) = omat(i, j)[1];
			out(j, i, 0, 2) = omat(i, j)[2];
		}
	}

	std::cout << "Saved result" << std::endl;

	out.save_bmp("img2.bmp");
	std::cout << "Wrote file" << std::endl;

	return 0;
}