#include <iostream>
#include "image_filter.h"

using namespace image_filter;

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

	auto bothWithZero = PadModel<RGB>(PadDirection::BOTH, PadType::CONST).pad(Shape{1, 2}, img1);
	imgprint("bothWithZero", bothWithZero);

	assert(blaze::size(bothWithZero) == 35);
	const RGB ZERO{0, 0, 0};
	assert(bothWithZero(0, 0) == ZERO);
	assert(bothWithZero(4, 6) == ZERO);
	assert(bothWithZero(1, 2) == img1(0, 0));

	auto preWithOnes = PadModel<RGB>(PadDirection::PRE, PadType::CONST, RGB{1, 1, 1}).pad(Shape{2, 1}, img1);
	imgprint("preWithOnes", preWithOnes);

	assert(blaze::size(preWithOnes) == 20);
	const RGB ONES{1, 1, 1};
	assert(preWithOnes(0, 0) == ONES);
	assert(preWithOnes(4, 3) == img1(2, 2));
	assert(preWithOnes(2, 1) == img1(0, 0));

	auto postWithOnes = PadModel<RGB>(PadDirection::POST, PadType::CONST, RGB{1, 1, 1}).pad(Shape{2, 1}, img1);
	imgprint("postWithOnes", postWithOnes);

	assert(blaze::size(preWithOnes) == 20);
	assert(postWithOnes(4, 3) == ONES);
	assert(postWithOnes(2, 2) == img1(2, 2));
	assert(postWithOnes(0, 0) == img1(0, 0));

	auto bothReplicate = PadModel<RGB>(PadDirection::BOTH, PadType::REPLICATE).pad(Shape{1, 2}, img1);
	imgprint("bothReplicate", bothReplicate);

	assert(blaze::size(bothReplicate) == 35);
	assert(bothReplicate(0, 0) == img1(0, 0));
	assert(bothReplicate(0, 3) == img1(0, 1));
	assert(bothReplicate(4, 6) == img1(2, 2));
	assert(bothReplicate(1, 2) == img1(0, 0));

	//TODO: Must be implemented!
//	auto bothCircular= PadModel<RGB>(PadDirection::BOTH, PadType::CIRCULAR).pad(Shape{4, 4}, img1);
//	pretty_print("bothCircular", bothCircular);
//
//	assert(blaze::size(bothCircular) == 121);
//	assert(bothCircular(0, 0) == img1(2, 2));
//	assert(bothCircular(4, 9) == img1(0, 2));
//	assert(bothCircular(4, 4) == img1(0, 0));
//	assert(bothCircular(6, 6) == img1(2, 2));
//	assert(bothCircular(10, 10) == img1(0, 0));


	Shape padShape{2, 3};
	AverageFilter averageFilter(padShape);

	auto avgKernel = averageFilter();
	krprint("filter", avgKernel);

	assert(blaze::size(avgKernel) == 6);
	assert(avgKernel(0, 0) == 0.16666666666666666);
	assert(avgKernel(1, 2) == 0.16666666666666666);


	PadModel<RGB> bothConstModel(PadDirection::BOTH, PadType::CONST);

	auto prepCov2 = bothConstModel.pad(padShape, img1);
	imgprint("PrepCov2", prepCov2);
	auto cov2Mat = imgcov2(prepCov2, avgKernel);
	imgprint("Cov2", cov2Mat);

	assert(blaze::size(cov2Mat) == 42);
	assert(cov2Mat(0, 0) == (RGB{0,0,0}));
	assert(cov2Mat(2, 2) == (RGB{0,0,0}));
	assert(cov2Mat(3, 3) == (RGB{4,4,4}));
	assert(cov2Mat(4, 5) == (RGB{1,1,1}));

	auto averageFilterResult = imfilter(img1, averageFilter, bothConstModel);
	imgprint("averageFilterResult", averageFilterResult);
	assert(averageFilterResult == cov2Mat);

//	DiskFilter diskFilter(3.2);
//	auto diskKernel = diskFilter();


	GaussianFilter gaussianFilter(padShape, 0.2);
	auto gaussianKernel = gaussianFilter();
	krprint("gaussianKernel", gaussianKernel);
	assert(blaze::size(gaussianKernel) == 6);
	assert(eq(gaussianKernel(0,0), 1.86331e-06));
	assert(eq(gaussianKernel(1,1), 0.499996));
	assert(eq(gaussianKernel(1,2), 1.86331e-06));


	LaplacianFilter laplacianFilter(0.2);
	auto laplacianKernel = laplacianFilter();
	krprint("laplacianKernel", laplacianKernel);
	assert(blaze::size(laplacianKernel) == 9);
	assert(eq(laplacianKernel(0,0), 0.16667));
	assert(eq(laplacianKernel(1,1), -3.3333));
	assert(eq(laplacianKernel(2,2), 0.16667));

	LogFilter logFilter(padShape, 0.2);
	auto logKernel = logFilter();
	krprint("logFilter", logKernel);
	assert(blaze::size(logKernel) == 6);
	assert(eq(logKernel(0,0), -17.7077));
	assert(eq(logKernel(1,1), 35.4155));
	assert(eq(logKernel(1,2), -17.7077));
	return 0;
}
