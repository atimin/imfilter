#include <iostream>
#include "image_filter.h"

using namespace metric;
using namespace metric::image_processing_details;

template<typename T, size_t N>
void imgprint(const std::string &name, const Image<T, N> &img) {
	for (size_t ch = 0; ch < img.size(); ++ch) {
		std::cout << name << "[ch=" << ch << "][" << img[ch].rows() << ", " << img[ch].columns() << "]: \n";
		for (size_t i = 0; i < img[ch].rows(); ++i) {
			for (size_t j = 0; j < img[ch].columns(); ++j) {
				std::cout << (double)img[ch](i, j) << "\t";
			}

			std::cout << "\n";
		}
	}

	std::cout << std::endl;
}


template<typename T>
void chprint(const std::string &name, const Channel<T> &ch) {
	std::cout << name << "[" << ch.rows() << ", " << ch.columns() << "]: \n";
	for (size_t i = 0; i < ch.rows(); ++i) {
		for (size_t j = 0; j < ch.columns(); ++j) {
			std::cout << (double)ch(i, j) << "\t";
		}

		std::cout << "\n";
	}


	std::cout << std::endl;
}

void krprint(const std::string &name, const FilterKernel &img) {
	std::cout << name << "[" << img.rows() << ", " << img.columns() << "]: \n";
	for (size_t i = 0; i < img.rows(); ++i) {
		for (size_t j = 0; j < img.columns(); ++j) {
			std::cout << img(i, j) << "\t";
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

	blaze::setNumThreads(4);
	// TEST PadModel
	Channel<uint8_t> ch1 = {
			{1, 2, 3},
			{4, 5, 6},
			{7, 8, 9}
	};

	auto[bothWithZero, bothWithZeroCord] = PadModel<uint8_t>(PadDirection::BOTH, PadType::CONST)
			.pad(Shape{1, 2}, ch1);
	chprint("bothWithZero", bothWithZero);
	vecprint("bothWithZeroCord", static_cast<blaze::DynamicVector<size_t>>(bothWithZeroCord));

	assert(blaze::size(bothWithZero) == 35);
	assert(bothWithZero(0, 0) == 0);
	assert(bothWithZero(4, 6) == 0);
	assert(bothWithZero(1, 2) == ch1(0, 0));

	auto preWithOnes = PadModel<uint8_t >(PadDirection::PRE, PadType::CONST, 1)
			.pad(Shape{2, 1}, ch1).first;
	chprint("preWithOnes", preWithOnes);

	assert(blaze::size(preWithOnes) == 20);
	assert(preWithOnes(0, 0) == 1);
	assert(preWithOnes(4, 3) == ch1(2, 2));
	assert(preWithOnes(2, 1) == ch1(0, 0));

	auto postWithOnes = PadModel<uint8_t>(PadDirection::POST, PadType::CONST, 1)
			.pad(Shape{2, 1}, ch1).first;
	chprint("postWithOnes", postWithOnes);

	assert(blaze::size(preWithOnes) == 20);
	assert(postWithOnes(4, 3) == 1);
	assert(postWithOnes(2, 2) == ch1(2, 2));
	assert(postWithOnes(0, 0) == ch1(0, 0));

	auto bothReplicate = PadModel<uint8_t>(PadDirection::BOTH, PadType::REPLICATE)
			.pad(Shape{1, 2}, ch1).first;
	chprint("bothReplicate", bothReplicate);

	assert(blaze::size(bothReplicate) == 35);
	assert(bothReplicate(0, 0) == ch1(0, 0));
	assert(bothReplicate(0, 3) == ch1(0, 1));
	assert(bothReplicate(4, 6) == ch1(2, 2));
	assert(bothReplicate(1, 2) == ch1(0, 0));

	auto bothCircular = PadModel<uint8_t>(PadDirection::BOTH, PadType::CIRCULAR)
			.pad(Shape{4, 4}, ch1).first;
	chprint("bothCircular", bothCircular);

	assert(blaze::size(bothCircular) == 121);
	assert(bothCircular(0, 0) == ch1(2, 2));
	assert(bothCircular(4, 9) == ch1(0, 2));
	assert(bothCircular(4, 4) == ch1(0, 0));
	assert(bothCircular(6, 6) == ch1(2, 2));
	assert(bothCircular(10, 10) == ch1(0, 0));

	auto symCircular = PadModel<uint8_t>(PadDirection::BOTH, PadType::SYMMETRIC)
			.pad(Shape{4, 4}, ch1).first;
	chprint("symCircular", symCircular);

	assert(blaze::size(symCircular) == 121);
	assert(symCircular(0, 0) == ch1(2, 2));
	assert(symCircular(0, 4) == ch1(2, 0));
	assert(symCircular(4, 4) == ch1(0, 0));
	assert(symCircular(7, 7) == ch1(2, 2));
	assert(symCircular(10, 5) == ch1(0, 1));

	Shape padShape{2, 3};
	FilterType::AVERAGE averageFilter(padShape[0], padShape[1]);

	auto avgKernel = averageFilter();
	krprint("filter", avgKernel);

	assert(blaze::size(avgKernel) == 6);
	assert(avgKernel(0, 0) == 0.16666666666666666);
	assert(avgKernel(1, 2) == 0.16666666666666666);


	PadModel<uint8_t> bothConstModel(PadDirection::BOTH, PadType::CONST);

	auto prepCov2 = bothConstModel.pad(padShape, ch1).first;
	chprint("PrepCov2", prepCov2);
	auto cov2Mat = imgcov2(prepCov2, avgKernel);
	chprint("Cov2", cov2Mat);

	assert(blaze::size(cov2Mat) == 42);
	assert(cov2Mat(0, 0) == 0);
	assert(cov2Mat(2, 2) == 2);
	assert(cov2Mat(3, 3) == 7);
	assert(cov2Mat(4, 5) == 2);


	auto averageFilterResult = filter(Image<uint8_t,1>{ch1}, averageFilter, bothConstModel);
	imgprint("averageFilterResult", averageFilterResult);


//	DiskFilter diskFilter(3.2);
//	auto diskKernel = diskFilter();


	FilterType::GAUSSIAN gaussianFilter(padShape[0], padShape[1], 0.2);
	auto gaussianKernel = gaussianFilter();
	krprint("gaussianKernel", gaussianKernel);
	assert(blaze::size(gaussianKernel) == 6);
	assert(eq(gaussianKernel(0, 0), 1.86331e-06));
	assert(eq(gaussianKernel(1, 1), 0.499996));
	assert(eq(gaussianKernel(1, 2), 1.86331e-06));


	FilterType::LAPLACIAN laplacianFilter(0.2);
	auto laplacianKernel = laplacianFilter();
	krprint("laplacianKernel", laplacianKernel);
	assert(blaze::size(laplacianKernel) == 9);
	assert(eq(laplacianKernel(0, 0), 0.16667));
	assert(eq(laplacianKernel(1, 1), -3.3333));
	assert(eq(laplacianKernel(2, 2), 0.16667));

	FilterType::LOG logFilter(padShape[0], padShape[1], 0.2);
	auto logKernel = logFilter();
	krprint("logFilter", logKernel);
	assert(blaze::size(logKernel) == 6);
	assert(eq(logKernel(0, 0), -17.7077));
	assert(eq(logKernel(1, 1), 35.4155));
	assert(eq(logKernel(1, 2), -17.7077));

	FilterType::MOTION motFilter(3, 30);
	auto motKernel = motFilter();
	krprint("motFilter", motKernel);
	assert(blaze::size(motKernel) == 9);
	assert(eq(motKernel(0, 0), 0));
	assert(eq(motKernel(1, 1), 0.341361));
	assert(eq(motKernel(1, 2), 0.16466));

	FilterType::PREWITT prewittFilter;
	auto prewKernel = prewittFilter();
	krprint("prewKernel", prewKernel);
	assert(blaze::size(prewKernel) == 9);
	assert(eq(prewKernel(0, 0), 1));
	assert(eq(prewKernel(1, 1), 0));
	assert(eq(prewKernel(2, 1), -1));

	FilterType::SOBEL sobelFilter;
	auto sobelKernel = sobelFilter();
	krprint("sobelKernel", sobelKernel);
	assert(blaze::size(sobelKernel) == 9);
	assert(eq(sobelKernel(0, 0), 1));
	assert(eq(sobelKernel(1, 1), 0));
	assert(eq(sobelKernel(2, 1), -2));

	FilterType::UNSHARP unsharpFilter(0.7);
	auto unsharpKernel = unsharpFilter();
	krprint("unsharpKernel", unsharpKernel);
	assert(blaze::size(unsharpKernel) == 9);
	assert(eq(unsharpKernel(0, 0), -0.411765));
	assert(eq(unsharpKernel(1, 1), 3.35294));
	assert(eq(unsharpKernel(2, 1), -0.176471));

	auto unsharpFilterResult = filter(Image<uint8_t,1>{ch1}, unsharpFilter, bothConstModel, true);
	imgprint("unsharpFilterResult_full", unsharpFilterResult);

	assert(blaze::size(unsharpFilterResult[0]) == 25);
	assert(unsharpFilterResult[0](0, 0) == 0);
	assert(unsharpFilterResult[0](2, 2) == 5);
	assert(unsharpFilterResult[0](2, 3) == 13);

	unsharpFilterResult = filter(ch1, unsharpFilter, bothConstModel, false);
	imgprint("unsharpFilterResult_same", unsharpFilterResult);

	assert(blaze::size(unsharpFilterResult[0]) == 9);
	assert(unsharpFilterResult[0](0, 0) == 0);
	assert(unsharpFilterResult[0](1, 1) == 5);
	assert(unsharpFilterResult[0](1, 2) == 13);

	imfilter<double, 1, FilterType::AVERAGE, PadDirection::BOTH, PadType::CONST> f(3, 3);
	Channel<double> out = f(ch1);
	chprint("out", out);
	return 0;
}
