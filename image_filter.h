//
// Created by Aleksey Timin on 3/27/20.
//

#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include <algorithm>
#include <blaze/Math.h>

namespace image_filter {

	using RGB = blaze::StaticVector<uint8_t, 3>;
	using Image = blaze::DynamicMatrix<RGB>;
	using FilterKernel = blaze::DynamicMatrix<double>;

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

	using Shape = blaze::StaticVector<size_t, 2>;

	template<typename T>
	std::pair<blaze::DynamicMatrix<T>, blaze::DynamicMatrix<T>>
	meshgrid(const blaze::DynamicVector<T, blaze::rowVector> &x,
			 const blaze::DynamicVector<T, blaze::columnVector> &y) {
		blaze::DynamicMatrix<T, blaze::rowMajor> xMat(blaze::size(y), blaze::size(x));
		blaze::DynamicMatrix<T, blaze::columnMajor> yMat(blaze::size(y), blaze::size(x));

		for (int i = 0; i < xMat.rows(); ++i) {
			blaze::row(xMat, i) = x;
		}

		for (int i = 0; i < yMat.columns(); ++i) {
			blaze::column(yMat, i) = y;
		}

		return std::make_pair(xMat, yMat);
	}

	template<typename T, bool P>
	blaze::DynamicVector<T, P> range(T min, T max) {
		blaze::DynamicVector<T, P> vec(max - min + 1);
		std::iota(vec.begin(), vec.end(), min);

		return vec;
	}


	/**
	 * 	Average filter
	 */
	class AverageFilter {
	public:
		AverageFilter(const Shape &shape) {
			FilterKernel f(shape[0], shape[1], 1.0);
			_kernel = f / blaze::prod(shape);
		}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;
	};

	/**
	 * Disk average filer
	 */
	class DiskFilter {
	public:
		explicit DiskFilter(double rad) : _rad(rad) {};

		FilterKernel operator()() const {


			auto crad = std::ceil(_rad - 0.5);
			size_t matSize = crad * 2 + 1;

			auto xrange = range<FilterKernel::ElementType, blaze::rowVector>(-crad, crad);
			auto yrange = range<FilterKernel::ElementType, blaze::columnVector>(-crad, crad);
			auto[xMat, yMat] = meshgrid(xrange, yrange);

			krprint("xMat", xMat);
			krprint("yMat", yMat);

			FilterKernel xymaxMat = blaze::max(blaze::abs(xMat), blaze::abs(yMat));
			FilterKernel xyminMat = blaze::min(blaze::abs(xMat), blaze::abs(yMat));

			krprint("xMat", xymaxMat);
			krprint("yMat", xyminMat);

			// TODO: Next steps are not clear. Implement!!!
			return FilterKernel();
		}


	private:
		FilterKernel::ElementType _rad;
	};

	/**
	 * Gaussian lowpass filter
	 */

	class GaussianFilter {
		friend class LogFilter;

	public:
		GaussianFilter(const Shape &shape, double sigma) {
			auto halfShape =
					(static_cast<blaze::StaticVector<FilterKernel::ElementType, 2>>(shape) - 1) / 2;

			auto xrange = range<FilterKernel::ElementType, blaze::rowVector>(-halfShape[1], halfShape[1]);
			auto yrange = range<FilterKernel::ElementType, blaze::columnVector>(-halfShape[0], halfShape[0]);
			auto[xMat, yMat] = meshgrid(xrange, yrange);

			auto arg = -(xMat % xMat + yMat % yMat) / (2 * sigma * sigma);
			_kernel = blaze::exp(arg);

			auto sumh = blaze::sum(_kernel);
			if (sumh != 0) {
				_kernel = _kernel / sumh;

			}

			_xMat = xMat;
			_yMat = yMat;
		}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;
		FilterKernel _xMat;
		FilterKernel _yMat;
	};

	/**
	 * Laplacian filter
	 */
	class LaplacianFilter {
	public:
		explicit LaplacianFilter(double alpha) {
			alpha = std::max<double>(0, std::min<double>(alpha, 1));
			auto h1 = alpha / (alpha + 1);
			auto h2 = (1 - alpha) / (alpha + 1);

			_kernel = FilterKernel{
					{h1, h2,               h1},
					{h2, -4 / (alpha + 1), h2},
					{h1, h2,               h1}};
		}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;

	};


	/**
 * Laplacian filter
 */
	class LogFilter {
	public:
		explicit LogFilter(const Shape &shape, double sigma) {
			auto std2 = sigma*sigma;
			GaussianFilter gausFilter(shape, sigma);

			auto h = gausFilter();
			_kernel = h % (gausFilter._xMat % gausFilter._xMat + gausFilter._yMat % gausFilter._yMat - 2 * std2)
					  / (std2*std2);
			_kernel -= blaze::sum(_kernel)/blaze::prod(shape);
		}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;
	};

	enum class PadDirection {
		POST,
		PRE,
		BOTH
	};

	enum class PadType {
		CONST,
		REPLICATE,
		SYMMETRIC,
		CIRCULAR,
	};

	template<typename T>
	class PadModel {
	public:
		PadModel(PadDirection padDirection, PadType padType, T initValue = {})
				: _padDirection(padDirection), _padType(padType), _initValue(initValue) {};

		blaze::DynamicMatrix<T> pad(const Shape &shape, const blaze::DynamicMatrix<T> &src) const {
			using namespace blaze;
			size_t padRow = shape[0];
			size_t padCol = shape[1];

			// Init padded matrix
			DynamicMatrix<T> dist;
			switch (_padDirection) {
				case PadDirection::PRE:
				case PadDirection::POST:
					dist = blaze::DynamicMatrix<T>(src.rows() + padRow, src.columns() + padCol, _initValue);
					break;
				case PadDirection::BOTH:
					dist = blaze::DynamicMatrix<T>(src.rows() + padRow * 2, src.columns() + padCol * 2, _initValue);
					break;
			}

			// Fill the padded matrix
			if (_padDirection == PadDirection::POST) {
				padRow = 0;
				padCol = 0;
			}

			for (size_t i = 0; i<dist.rows(); ++i) {
				for (size_t j = 0; j < dist.columns(); ++j) {
					int si = i - padRow;
					int sj = j - padCol;

					if (si >= 0 && si < src.rows()
						&& sj >= 0 && sj < src.columns()) {
						//TODO: I don't like this traveling, we can use memcopy or something like this.
						dist(i, j) = src(si, sj);
					} else {
						if (_padType == PadType::REPLICATE) {
							si = std::max<int>(0, si);
							si = std::min<int>(src.rows() - 1, si);

							sj = std::max<int>(0, sj);
							sj = std::min<int>(src.columns() - 1, sj);
							dist(i, j) = src(si, sj);
						} else if (_padType == PadType::CIRCULAR) {
							si = (i + padRow + 1) % src.rows();
							sj = (j + padCol + 1) % src.columns();
							dist(i, j) = src(si, sj);
						} else if (_padType == PadType::SYMMETRIC) {
							// TODO: Must be implemented!
						}
					}
				}
			}

			return dist;
		}

	private:
		PadDirection _padDirection;
		PadType _padType;
		T _initValue;
	};

	template<typename ChannelType>
	blaze::DynamicMatrix<ChannelType> imgcov2(const blaze::DynamicMatrix<ChannelType> &input, FilterKernel &kernel) {
		size_t funcRows = kernel.rows();
		size_t funcCols = kernel.columns();

		//TODO: must have full or same flags
		Image resultMat(input.rows() - std::ceil((double) funcRows / 2),
						input.columns() - std::ceil((double) funcCols / 2));
		for (auto i = 0; i < input.rows() - funcRows; ++i) {
			for (auto j = 0; j < input.columns() - funcCols; ++j) {
				Image view = blaze::submatrix(input, i, j, funcRows, funcCols);

				Image bwProd = view % kernel;
				resultMat(i, j) = blaze::sum(bwProd);
			}
		}

		return resultMat;
	}

	template<typename Filter, typename ChannelType>
	blaze::DynamicMatrix<ChannelType>
	imfilter(const blaze::DynamicMatrix<ChannelType> &img, const Filter &impl, const PadModel<ChannelType> &padmodel) {
		auto kernel = impl();
		Shape padShape{kernel.rows(), kernel.columns()};
		auto paddedImage = padmodel.pad(padShape, img);
		return imgcov2(paddedImage, kernel);
	}

}
#endif //GAUSFILTER_IMAGEFILTER_H