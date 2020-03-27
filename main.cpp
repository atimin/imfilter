#include <iostream>
#include "image_filter.h"

int main() {
	using namespace image_filter;

	// TEST PadModel
	Image img1 = {
			{{1, 1, 1}, {2, 2, 2}, {3, 3, 3}},
			{{4, 4, 4}, {5, 5, 5}, {6, 6, 6}},
			{{7, 7, 7}, {8, 8, 8}, {9, 9, 9}},
	};

	auto bothWithZero = PadModel<RGB>(Shape{1, 2}, PadDirection::BOTH, PadType::CONST).pad(img1);
	assert(blaze::size(bothWithZero) == 35);

	const RGB ZERO{0, 0, 0};
	assert(bothWithZero(0, 0) == ZERO);
	assert(bothWithZero(4, 6) == ZERO);
	assert(bothWithZero(1, 2) == img1(0, 0));

	auto preWithOnes = PadModel<RGB>(Shape{2, 1}, PadDirection::PRE, PadType::CONST, RGB{1, 1, 1}).pad(img1);
	assert(blaze::size(preWithOnes) == 20);

	const RGB ONES{1, 1, 1};
	assert(preWithOnes(0, 0) == ONES);
	assert(preWithOnes(4, 3) == img1(2, 2));
	assert(preWithOnes(2, 1) == img1(0, 0));

	auto postWithOnes = PadModel<RGB>(Shape{2, 1}, PadDirection::POST, PadType::CONST, RGB{1, 1, 1}).pad(img1);
	assert(blaze::size(preWithOnes) == 20);

	assert(postWithOnes(4, 3) == ONES);
	assert(postWithOnes(2, 2) == img1(2, 2));
	assert(postWithOnes(0, 0) == img1(0, 0));



	AverageFilter averageFilter(2, 5);

	auto filter = averageFilter();
	assert(blaze::size(filter) == 10);
	assert(filter(0, 0) == 0.1);
	assert(filter(1, 4) == 0.1);

//	imfilter(img1, averageFilter, bothZero);
	return 0;
}
