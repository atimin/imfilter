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
			return f/(_h*_w);
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
		SYMMETRIC
	};

	using Shape = blaze::StaticVector<size_t, 2>;

	template<typename T>
	class PadModel {
	public:
		PadModel(const Shape& shape, PadDirection padDirection, PadType padType, T initValue={})
			: _shape(shape), _padDirection(padDirection), _padType(padType), _initValue(initValue) {};

		blaze::DynamicMatrix<T> pad(const blaze::DynamicMatrix<T>& src) const {
			using namespace blaze;
			size_t padRow = _shape[0];
			size_t padCol = _shape[1];

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
			//TODO: I don't like this traveling, we can use memcopy or something like this.
			for (size_t i=0; i<src.rows(); ++i) {
				for (size_t j=0; j<src.columns(); ++j) {
					dist(i + padRow, j + padCol) = src(i, j);
				}
			}

			return dist;
		}
	private:
		Shape _shape;
		PadDirection _padDirection;
		PadType _padType;
		T _initValue;
	};


	template <typename Filter>
	Image imfilter(const Image &img, const Filter &impl, const PadModel<RGB>& padmodel) {
	}
}
#endif //GAUSFILTER_IMAGEFILTER_H