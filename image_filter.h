//
// Created by Aleksey Timin on 3/27/20.
//

#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include <blaze/Math.h>

namespace image_filter {
	using RGB = blaze::StaticVector<uint8_t, 3>;
	using Image = blaze::DynamicMatrix<RGB>;
	using FilterFunc = blaze::DynamicMatrix<double>;

	/**
	 * 	Average filter
	 */
	class AverageFilter {
	public:
		AverageFilter(size_t h, size_t w) : _h(h), _w(w) {}

		FilterFunc operator()() const {
			FilterFunc f(_h, _w, 1.0);
			return f / (_h * _w);
		}

	private:
		size_t _h;
		size_t _w;
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

	using Shape = blaze::StaticVector<size_t, 2>;

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
							si = (i+padRow+1) % src.rows();
							sj = (j+padCol+1) % src.columns();
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

	template<typename T>
	blaze::DynamicMatrix<T> cov2(const blaze::DynamicMatrix<T>& input, blaze::DynamicMatrix<T>& func) {

	}

	template<typename Filter>
	Image imfilter(const Image &img, const Filter &impl, const PadModel<RGB> &padmodel) {
		auto filter = impl();
		auto paddedImage = padmodel.pad(Shape{filter.rows()/2, filter.columns()/2}, img);

	}

}
#endif //GAUSFILTER_IMAGEFILTER_H