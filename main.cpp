#include <iostream>
#include "image_filter.h"

using namespace image_processing;

void imgprint(const std::string &name, const Image &img) {
	std::cout << name << "[" << img.rows() << ", " << img.columns() << "]: \n";
	for (size_t i = 0; i < img.rows(); ++i) {
		for (size_t j = 0; j < img.columns(); ++j) {
			const RGB &point = img(i, j);
			std::cout << "(" << (int) point[0] << "," << (int) point[1] << "," << (int) point[2] << ")\t";
		}

		std::cout << "\n";
	}

	std::cout << std::endl;
}

void krprint(const std::string &name, const FilterKernel &img) {
	std::cout << name << "[" << img.rows() << ", " << img.columns() << "]: \n";
	for (size_t i = 0; i < img.rows(); ++i) {
		for (size_t j = 0; j < img.columns(); ++j) {
			std::cout << "(" << img(i, j) << ")\t";
		}

		std::cout << "\n";
	}

	std::cout << std::endl;
}

template<typename T, bool P>
void vecprint(const std::string &name, const blaze::DynamicVector<T, P> &vec) {
	std::cout << name << "[" << vec.size() << "]: \n";
	for (auto &val : vec) {
		std::cout << "[" << val << "]\t";
	}


	std::cout << std::endl;
}


bool eq(double a, double b) {
	return fabs(a - b) < 0.001;
}

int main() {


	// TEST PadModel
	Image img1 = {
			{{1, 1, 1}, {2, 2, 2}, {3, 3, 3}},
			{{4, 4, 4}, {5, 5, 5}, {6, 6, 6}},
			{{7, 7, 7}, {8, 8, 8}, {9, 9, 9}},
	};

	auto [bothWithZero, bothWithZeroCord] = PadModel<RGB>(PadDirection::BOTH, PadType::CONST)
	        .pad(Shape{1, 2}, img1);
	imgprint("bothWithZero", bothWithZero);
	vecprint("bothWithZeroCord", static_cast<blaze::DynamicVector<size_t>>(bothWithZeroCord));

	assert(blaze::size(bothWithZero) == 35);
	const RGB ZERO{0, 0, 0};
	assert(bothWithZero(0, 0) == ZERO);
	assert(bothWithZero(4, 6) == ZERO);
	assert(bothWithZero(1, 2) == img1(0, 0));

	auto preWithOnes = PadModel<RGB>(PadDirection::PRE, PadType::CONST, RGB{1, 1, 1})
			.pad(Shape{2, 1}, img1).first;
	imgprint("preWithOnes", preWithOnes);

	assert(blaze::size(preWithOnes) == 20);
	const RGB ONES{1, 1, 1};
	assert(preWithOnes(0, 0) == ONES);
	assert(preWithOnes(4, 3) == img1(2, 2));
	assert(preWithOnes(2, 1) == img1(0, 0));

	auto postWithOnes = PadModel<RGB>(PadDirection::POST, PadType::CONST, RGB{1, 1, 1})
			.pad(Shape{2, 1}, img1).first;
	imgprint("postWithOnes", postWithOnes);

	assert(blaze::size(preWithOnes) == 20);
	assert(postWithOnes(4, 3) == ONES);
	assert(postWithOnes(2, 2) == img1(2, 2));
	assert(postWithOnes(0, 0) == img1(0, 0));

	auto bothReplicate = PadModel<RGB>(PadDirection::BOTH, PadType::REPLICATE)
	        .pad(Shape{1, 2}, img1).first;
	imgprint("bothReplicate", bothReplicate);

	assert(blaze::size(bothReplicate) == 35);
	assert(bothReplicate(0, 0) == img1(0, 0));
	assert(bothReplicate(0, 3) == img1(0, 1));
	assert(bothReplicate(4, 6) == img1(2, 2));
	assert(bothReplicate(1, 2) == img1(0, 0));

	auto bothCircular= PadModel<RGB>(PadDirection::BOTH, PadType::CIRCULAR)
	        .pad(Shape{4, 4}, img1).first;
	imgprint("bothCircular", bothCircular);

	assert(blaze::size(bothCircular) == 121);
	assert(bothCircular(0, 0) == img1(2, 2));
	assert(bothCircular(4, 9) == img1(0, 2));
	assert(bothCircular(4, 4) == img1(0, 0));
	assert(bothCircular(6, 6) == img1(2, 2));
	assert(bothCircular(10, 10) == img1(0, 0));

	auto symCircular= PadModel<RGB>(PadDirection::BOTH, PadType::SYMMETRIC)
			.pad(Shape{4, 4}, img1).first;
	imgprint("symCircular", symCircular);

	assert(blaze::size(symCircular) == 121);
	assert(symCircular(0, 0) == img1(2, 2));
	assert(symCircular(0, 4) == img1(2, 0));
	assert(symCircular(4, 4) == img1(0, 0));
	assert(symCircular(7, 7) == img1(2, 2));
	assert(symCircular(10, 5) == img1(0, 1));

	Shape padShape{2, 3};
	AverageFilter averageFilter(padShape);

	auto avgKernel = averageFilter();
	krprint("filter", avgKernel);

	assert(blaze::size(avgKernel) == 6);
	assert(avgKernel(0, 0) == 0.16666666666666666);
	assert(avgKernel(1, 2) == 0.16666666666666666);


	PadModel<RGB> bothConstModel(PadDirection::BOTH, PadType::CONST);

	Image prepCov2 = bothConstModel.pad(padShape, img1).first;
	imgprint("PrepCov2", prepCov2);
	auto cov2Mat = imgcov2(prepCov2, avgKernel);
	imgprint("Cov2", cov2Mat);

	assert(blaze::size(cov2Mat) == 42);
	assert(cov2Mat(0, 0) == (RGB{0, 0, 0}));
	assert(cov2Mat(2, 2) == (RGB{2, 2, 2}));
	assert(cov2Mat(3, 3) == (RGB{7, 7, 7}));
	assert(cov2Mat(4, 5) == (RGB{2, 2, 2}));

	auto averageFilterResult = imfilter(img1, averageFilter, bothConstModel);
	imgprint("averageFilterResult", averageFilterResult);
//	assert(averageFilterResult == cov2Mat);

//	DiskFilter diskFilter(3.2);
//	auto diskKernel = diskFilter();


	GaussianFilter gaussianFilter(padShape, 0.2);
	auto gaussianKernel = gaussianFilter();
	krprint("gaussianKernel", gaussianKernel);
	assert(blaze::size(gaussianKernel) == 6);
	assert(eq(gaussianKernel(0, 0), 1.86331e-06));
	assert(eq(gaussianKernel(1, 1), 0.499996));
	assert(eq(gaussianKernel(1, 2), 1.86331e-06));


	LaplacianFilter laplacianFilter(0.2);
	auto laplacianKernel = laplacianFilter();
	krprint("laplacianKernel", laplacianKernel);
	assert(blaze::size(laplacianKernel) == 9);
	assert(eq(laplacianKernel(0, 0), 0.16667));
	assert(eq(laplacianKernel(1, 1), -3.3333));
	assert(eq(laplacianKernel(2, 2), 0.16667));

	LogFilter logFilter(padShape, 0.2);
	auto logKernel = logFilter();
	krprint("logFilter", logKernel);
	assert(blaze::size(logKernel) == 6);
	assert(eq(logKernel(0, 0), -17.7077));
	assert(eq(logKernel(1, 1), 35.4155));
	assert(eq(logKernel(1, 2), -17.7077));

	MotionFilter motFilter(3, 30);
	auto motKernel = motFilter();
	krprint("motFilter", motKernel);
	assert(blaze::size(motKernel) == 9);
	assert(eq(motKernel(0, 0), 0));
	assert(eq(motKernel(1, 1), 0.341361));
	assert(eq(motKernel(1, 2), 0.16466));

	PrewittFilter prewittFilter;
	auto prewKernel = prewittFilter();
	krprint("prewKernel", prewKernel);
	assert(blaze::size(prewKernel) == 9);
	assert(eq(prewKernel(0, 0), 1));
	assert(eq(prewKernel(1, 1), 0));
	assert(eq(prewKernel(2, 1), -1));

	SobelFilter sobelFilter;
	auto sobelKernel = sobelFilter();
	krprint("sobelKernel", sobelKernel);
	assert(blaze::size(sobelKernel) == 9);
	assert(eq(sobelKernel(0, 0), 1));
	assert(eq(sobelKernel(1, 1), 0));
	assert(eq(sobelKernel(2, 1), -2));

	UnsharpFilter unsharpFilter(0.7);
	auto unsharpKernel = unsharpFilter();
	krprint("unsharpKernel", unsharpKernel);
	assert(blaze::size(unsharpKernel) == 9);
	assert(eq(unsharpKernel(0, 0), -0.411765));
	assert(eq(unsharpKernel(1, 1), 3.35294));
	assert(eq(unsharpKernel(2, 1), -0.176471));

	auto unsharpFilterResult = imfilter(img1, unsharpFilter, bothConstModel, true);
	imgprint("unsharpFilterResult_full", unsharpFilterResult);

	assert(blaze::size(unsharpFilterResult) == 25);
	assert(unsharpFilterResult(0, 0) == (RGB{0,0,0}));
	assert(unsharpFilterResult(2, 2) == (RGB{5,5,5}));
	assert(unsharpFilterResult(2, 3) == (RGB{13,13,13}));

	unsharpFilterResult = imfilter(img1, unsharpFilter, bothConstModel, false);
	imgprint("unsharpFilterResult_same", unsharpFilterResult);

	assert(blaze::size(unsharpFilterResult) == 9);
	assert(unsharpFilterResult(0, 0) == (RGB{0,0,0}));
	assert(unsharpFilterResult(1, 1) == (RGB{5,5,5}));
	assert(unsharpFilterResult(1, 2) == (RGB{13,13,13}));

	return 0;
}
