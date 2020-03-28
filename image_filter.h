//
// Created by Aleksey Timin on 3/27/20.
//

#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include <algorithm>
#include <cmath>
#include <blaze/Math.h>
#include <blaze/Blaze.h>

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

	template<typename T, bool P>
	void vecprint(const std::string &name, const blaze::DynamicVector<T, P> &vec) {
		std::cout << name << "[" << vec.size() << "]: \n";
		for (auto &val : vec) {
			std::cout << "[" << val << "]\t";
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
	blaze::DynamicVector<T, P> range(T start, T stop, T step = 1) {
		blaze::DynamicVector<T, P> vec(std::abs((stop - start) / step) + 1);
		for (auto &val : vec) {
			val = start;
			start += step;
		}

		return vec;
	}

	template<typename T>
	blaze::DynamicVector<std::pair<size_t, size_t>, blaze::columnVector>
	mfind(const blaze::DynamicMatrix<T> &input, const blaze::DynamicMatrix<bool> &cond) {
		std::vector<std::pair<size_t, size_t>> indecies;

		for (auto i = 0; i < input.rows(); ++i) {
			for (auto j = 0; j < input.columns(); ++j) {
				if (cond(i, j)) {
					indecies.push_back(std::make_pair(i, j));
				}
			}
		}

		return blaze::DynamicVector<std::pair<size_t, size_t>, blaze::columnVector>(indecies.size(), indecies.data());
	}


	template<typename T>
	blaze::DynamicMatrix<T> rot90(const blaze::DynamicMatrix<T> &input) {
		blaze::DynamicMatrix<T> out(input.columns(), input.rows());
		for (int i = 0; i < input.rows(); ++i) {
			auto r = blaze::trans(blaze::row(input, i));
			blaze::column(out, input.rows() - i - 1) = r;
		}

		return out;
	}

	template<typename T>
	blaze::DynamicMatrix<T> flipud(const blaze::DynamicMatrix<T> &input) {
		blaze::DynamicMatrix<T> out(input.rows(), input.columns());
		for (int i = 0; i < input.rows(); ++i) {
			blaze::row(out, input.rows() - i - 1) = blaze::row(input, i);
		}

		return out;
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
			auto std2 = sigma * sigma;
			GaussianFilter gausFilter(shape, sigma);

			auto h = gausFilter();
			_kernel = h % (gausFilter._xMat % gausFilter._xMat + gausFilter._yMat % gausFilter._yMat - 2 * std2)
					  / (std2 * std2);
			_kernel -= blaze::sum(_kernel) / blaze::prod(shape);
		}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;
	};

	/**
	 * Motion filter
	 */
	class MotionFilter {
	public:
		explicit MotionFilter(double len, int theta) {
			len = std::max<double>(1, len);
			auto half = (len - 1) / 2;
			auto phi = static_cast<double>(theta % 180) / 180 * M_PI;


			double cosphi = std::cos(phi);
			double sinphi = std::sin(phi);
			int xsign = cosphi > 0 ? 1 : -1;
			double linewdt = 1;

			auto eps = std::numeric_limits<double>::epsilon();
			auto sx = std::trunc(half * cosphi + linewdt * xsign - len * eps);
			auto sy = std::trunc(half * sinphi + linewdt - len * eps);

			auto xrange =
					range<FilterKernel::ElementType, blaze::rowVector>(0, sx, xsign);
			auto yrange = range<FilterKernel::ElementType, blaze::columnVector>(0, sy);
			auto[xMat, yMat] = meshgrid(xrange, yrange);

			FilterKernel dist2line = (yMat * cosphi - xMat * sinphi);
			auto rad = blaze::sqrt(xMat % xMat + yMat % yMat);

			// find points beyond the line's end-point but within the line width
			blaze::DynamicMatrix<bool> cond = blaze::map(rad, [half](const auto &x) { return x >= half; })
											  && blaze::map(abs(dist2line),
															[linewdt](const auto &x) { return x <= linewdt; });

			auto lastpix = mfind(static_cast<FilterKernel>(dist2line), cond);

			for (auto[i, j] : lastpix) {
				auto v = dist2line(i, j);
				auto pix = half - abs((xMat(i, j) + v * sinphi) / cosphi);
				dist2line(i, j) = std::sqrt(v * v + pix * pix);
			}

			dist2line = linewdt + eps - abs(dist2line);
			// zero out anything beyond line width
			dist2line = blaze::map(dist2line, [](const FilterKernel::ElementType &v) {
				return v < 0 ? 0 : v;
			});

			auto h = rot90(rot90<FilterKernel::ElementType>(dist2line));

			_kernel = FilterKernel(h.rows() * 2 - 1, h.columns() * 2 - 1);
			blaze::submatrix(_kernel, 0, 0, h.rows(), h.columns()) = h;
			blaze::submatrix(_kernel,
							 h.rows() - 1, h.columns() - 1, dist2line.rows(), dist2line.columns()) = dist2line;

			_kernel /= blaze::sum(_kernel) + eps * len * len;
			if (cosphi > 0) {
				_kernel = flipud(_kernel);
			}

		}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;
	};

	/**
	 * Prewitt filter
	 */
	class PrewittFilter {
	public:
		PrewittFilter() :
				_kernel{
						{1,  1,  1},
						{0,  0,  0},
						{-1, -1, -1}
				} {}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;
	};

	/**
 	* Sobel filter
 	*/
	class SobelFilter {
	public:
		SobelFilter() :
				_kernel{
						{1,  2,  1},
						{0,  0,  0},
						{-1, -2, -1}
				} {}

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