//
// Created by Aleksey Timin on 3/27/20.
//

#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include <blaze/Math.h>

namespace image_filter {

	using RGB = blaze::StaticVector<uint8_t, 3>;
	using Image = blaze::DynamicMatrix<RGB>;
	using FilterKernel = blaze::DynamicMatrix<double>;

	void pretty_print(const std::string &name, const Image &img) {
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

	void pretty_print(const std::string &name, const FilterKernel &img) {
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

	/**
	 * 	Average filter
	 */
	class AverageFilter {
	public:
		AverageFilter(const Shape& shape) : _shape(shape){}

		FilterKernel operator()() const {
			FilterKernel f(_shape[0], _shape[1], 1.0);
			return f / blaze::prod(_shape);
		}

	private:
		Shape _shape;
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
		Shape padShape{kernel.rows(),kernel.columns()};
		auto paddedImage = padmodel.pad(padShape, img);
		return imgcov2(paddedImage, kernel);
	}

}
#endif //GAUSFILTER_IMAGEFILTER_H