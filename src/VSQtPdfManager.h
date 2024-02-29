#pragma once

#include <memory>
#include <unordered_set>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/assign.hpp>

#include <QImage>

#include "VSExportFileAsImages.h"

class VSQtPdfManager :
	public tc::file_as_img::AbstractInterruptible<tc::file_as_img::fs::IExporter>,
	public tc::file_as_img::AbstractInterruptible<tc::file_as_img::mem::IExporter>,
	public tc::file_as_img::AbstractInterruptible<tc::file_as_img::fs::IThumbnailGenerator>,
	public tc::file_as_img::AbstractInterruptible<tc::file_as_img::mem::IThumbnailGenerator>
{
public:
	template<typename X, typename Y>
	using bimap = boost::bimap<
		boost::bimaps::unordered_set_of<X>,
		boost::bimaps::unordered_set_of<Y>
	>;

	//supported file format - "pdf".

	inline static const std::unordered_set<ImageFormat> supportedImageFormats = {
		"png",
		"jpg",
		"jpeg"
	};

	inline static const bimap<PixelFormat, QImage::Format> supportedPixelFormats =
		boost::assign::list_of<bimap<PixelFormat, QImage::Format>::relation>
		("argb32", QImage::Format_ARGB32);

	using DPI = double;

	class Image : public IImage
	{
	public:
		Image(QImage img) : img(img)
		{
			assert(!img.isNull());
			assert(img.width() > 0);
			assert(img.height() > 0);
			assert(supportedPixelFormats.right.count(img.format()));
		}

		Size width() const override {
			return img.width();
		}
		Size height() const override {
			return img.height();
		}
		PixelFormat format() const override {
			return supportedPixelFormats.right.at(img.format());
		}
		const Byte* data() const override {
			return reinterpret_cast<const Byte*>(img.constBits());
		}

	private:
		QImage img;
	};

	VSQtPdfManager();

	void exportAsImages(
		const Path& file, const FileFormat& fileFormat,
		const PixelFormat& pixelFormat, const Any& options,
		const AnyImageConsumer& forEachImage
	) override;

	void exportAsImages(
		const Path& file, const FileFormat& fileFormat,
		const Path& outputDir, const ImageFormat& imageFormat,
		const AnyImageNameGenerator& imageNameGenerator,
		const Any& options,
		const AnyImageNameConsumer& forEachImageName
	) override;

	std::unique_ptr<IImage> generateThumbnail(
		const Path& file, const FileFormat& fileFormat, const PixelFormat& pixelFormat, const Any& options
	) override;

	String generateThumbnail(
		const Path& file, const FileFormat& fileFormat,
		const Path& outputDir, const ImageFormat& imageFormat,
		const AnyImageNameGenerator& imageNameGenerator,
		const Any& options
	) override;

private:
	static void validateFileFormat(const FileFormat& fileFormat);
	template<typename Interface>
	static bool areOptionsValid(const Any& options);
	template<typename Interface>
	static void validateOptions(const Any& options);
	template<typename Interface>
	static void validateArgsFS(const FileFormat& fileFormat, const ImageFormat& imageFormat, const Any& options);
	template<typename Interface>
	static void validateArgsMem(const FileFormat& fileFormat, const PixelFormat& pixelFormat, const Any& options);

	template<typename Interface, typename F>
	void exportAsQImages(const Path& file, const Any& options, F forEachQImage);

	template<typename Interface>
	String save(
		QImage& image,
		const Path& outputDir, const ImageFormat& imageFormat,
		const AnyImageNameGenerator& imageNameGenerator,
		const Any& options
	);

	template<typename Interface>
	std::unique_ptr<IImage> makeImage(QImage& image, const PixelFormat& pixelFormat, const Any& options);

};