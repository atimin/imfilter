#include <iostream>
#include "image_filter.h"

using namespace image_filter;



int main() {


	// TEST PadModel
	Image img1 = {
			{{1, 1, 1}, {2, 2, 2}, {3, 3, 3}},
			{{4, 4, 4}, {5, 5, 5}, {6, 6, 6}},
			{{7, 7, 7}, {8, 8, 8}, {9, 9, 9}},
	};

	auto bothWithZero = PadModel<RGB>(PadDirection::BOTH, PadType::CONST).pad(Shape{1, 2}, img1);
	pretty_print("bothWithZero", bothWithZero);

	assert(blaze::size(bothWithZero) == 35);
	const RGB ZERO{0, 0, 0};
	assert(bothWithZero(0, 0) == ZERO);
	assert(bothWithZero(4, 6) == ZERO);
	assert(bothWithZero(1, 2) == img1(0, 0));

	auto preWithOnes = PadModel<RGB>(PadDirection::PRE, PadType::CONST, RGB{1, 1, 1}).pad(Shape{2, 1}, img1);
	pretty_print("preWithOnes", preWithOnes);

	assert(blaze::size(preWithOnes) == 20);
	const RGB ONES{1, 1, 1};
	assert(preWithOnes(0, 0) == ONES);
	assert(preWithOnes(4, 3) == img1(2, 2));
	assert(preWithOnes(2, 1) == img1(0, 0));

	auto postWithOnes = PadModel<RGB>(PadDirection::POST, PadType::CONST, RGB{1, 1, 1}).pad(Shape{2, 1}, img1);
	pretty_print("postWithOnes", postWithOnes);

	assert(blaze::size(preWithOnes) == 20);
	assert(postWithOnes(4, 3) == ONES);
	assert(postWithOnes(2, 2) == img1(2, 2));
	assert(postWithOnes(0, 0) == img1(0, 0));

	auto bothReplicate = PadModel<RGB>(PadDirection::BOTH, PadType::REPLICATE).pad(Shape{1, 2}, img1);
	pretty_print("bothReplicate", bothReplicate);

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


	AverageFilter averageFilter(2, 3);

	auto filter = averageFilter();
	assert(blaze::size(filter) == 6);
	assert(filter(0, 0) == 0.16666666666666666);
	assert(filter(1, 2) == 0.16666666666666666);

	Shape padShape{static_cast<size_t >(std::floor(filter.rows())),
				   static_cast<size_t >(std::floor(filter.columns()))};
	auto prepCov2 = PadModel<RGB>(PadDirection::BOTH, PadType::CONST).pad(padShape, img1);
	pretty_print("PrepCov2", prepCov2);
	auto cov2Mat = imgcov2(prepCov2, filter);
	pretty_print("Cov2", cov2Mat);

	assert(blaze::size(cov2Mat) == 42);
	assert(cov2Mat(0, 0) == (RGB{0,0,0}));
	assert(cov2Mat(2, 2) == (RGB{0,0,0}));
	assert(cov2Mat(3, 3) == (RGB{4,4,4}));
	assert(cov2Mat(4, 5) == (RGB{1,1,1}));

//	imfilter(img1, averageFilter, bothZero);
	return 0;
}
