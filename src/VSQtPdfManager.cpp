#include "VSQtPdfManager.h"

#include <cassert>
#include <algorithm>

#include <QtPdf/QPdfDocument>

VSQtPdfManager::VSQtPdfManager() :
	tc::file_as_img::AbstractInterruptible<tc::file_as_img::fs::IExporter>(
		std::make_unique<VSStdAtomicBoolInterruptor>()
	),
	tc::file_as_img::AbstractInterruptible<tc::file_as_img::mem::IExporter>(
		std::make_unique<VSStdAtomicBoolInterruptor>()
	),
	tc::file_as_img::AbstractInterruptible<tc::file_as_img::fs::IThumbnailGenerator>(
		std::make_unique<VSStdAtomicBoolInterruptor>()
	),
	tc::file_as_img::AbstractInterruptible<tc::file_as_img::mem::IThumbnailGenerator>(
		std::make_unique<VSStdAtomicBoolInterruptor>()
	)
{}

void VSQtPdfManager::exportAsImages(
	const Path& file, const FileFormat& fileFormat,
	const PixelFormat& pixelFormat, const Any& options,
	const AnyImageConsumer& forEachImage
)
{
	using Interface = tc::file_as_img::IInterruptible<tc::file_as_img::mem::IExporter>;
	validateArgsMem<Interface>(fileFormat, pixelFormat, options);
	exportAsQImages<Interface>(file, options, [&](QImage& qimg) {
		forEachImage(makeImage<Interface>(qimg, pixelFormat, options));
	});
}

void VSQtPdfManager::exportAsImages(
	const Path& file, const FileFormat& fileFormat,
	const Path& outputDir, const ImageFormat& imageFormat,
	const AnyImageNameGenerator& imageNameGenerator,
	const Any& options,
	const AnyImageNameConsumer& forEachImageName
)
{
	using Interface = tc::file_as_img::IInterruptible<tc::file_as_img::fs::IExporter>;
	validateArgsFS<Interface>(fileFormat, imageFormat, options);
	exportAsQImages<Interface>(file, options, [&](QImage& qimg) {
		forEachImageName(save<Interface>(qimg, outputDir, imageFormat, imageNameGenerator, options));
	});
}

auto VSQtPdfManager::generateThumbnail(
	const Path& file, const FileFormat& fileFormat, const PixelFormat& pixelFormat, const Any& options
) -> std::unique_ptr<IImage>
{
	using Interface = tc::file_as_img::IInterruptible<tc::file_as_img::mem::IThumbnailGenerator>;
	validateArgsMem<Interface>(fileFormat, pixelFormat, options);
	std::unique_ptr<IImage> img;
	exportAsQImages<Interface>(file, options, [&](QImage& qimg) {
		img = makeImage<Interface>(qimg, pixelFormat, options);
	});
	return img;
}

auto VSQtPdfManager::generateThumbnail(
	const Path& file, const FileFormat& fileFormat,
	const Path& outputDir, const ImageFormat& imageFormat,
	const AnyImageNameGenerator& imageNameGenerator,
	const Any& options
) -> String
{
	using Interface = tc::file_as_img::IInterruptible<tc::file_as_img::fs::IThumbnailGenerator>;
	validateArgsFS<Interface>(fileFormat, imageFormat, options);
	String imgName;
	exportAsQImages<Interface>(file, options, [&](QImage& qimg) {
		imgName = save<Interface>(qimg, outputDir, imageFormat, imageNameGenerator, options);
	});
	return imgName;
}

void VSQtPdfManager::validateFileFormat(const FileFormat& fileFormat) {
	if(fileFormat != "pdf") {
		throw tc::file_as_img::InvalidFileFormat();
	}
}

template<typename Interface>
bool VSQtPdfManager::areOptionsValid(const Any& options)
{
	return !options.has_value() || static_cast<bool>(std::any_cast<DPI>(&options));
}

template<typename Interface>
void VSQtPdfManager::validateOptions(const Any& options)
{
	if(!areOptionsValid<Interface>(options)) {
		throw tc::err::exc::InvalidArgument("Invalid options");
	}
}

template<typename Interface>
void VSQtPdfManager::validateArgsFS(const FileFormat& fileFormat, const ImageFormat& imageFormat, const Any& options)
{
	validateFileFormat(fileFormat);
	if(!supportedImageFormats.count(imageFormat)) {
		throw tc::file_as_img::InvalidImageFormat();
	}
	validateOptions<Interface>(options);
}

template<typename Interface>
void VSQtPdfManager::validateArgsMem(const FileFormat& fileFormat, const PixelFormat& pixelFormat, const Any& options)
{
	validateFileFormat(fileFormat);
	if(!supportedPixelFormats.left.count(pixelFormat)) {
		throw tc::err::exc::InvalidArgument("Invalid pixel format");
	}
	validateOptions<Interface>(options);
}

template<typename Interface, typename F>
void VSQtPdfManager::exportAsQImages(const Path& file, const Any& options, F forEachQImage)
{
	assert(areOptionsValid<Interface>(options));

	constexpr bool thumbnail = tc::file_as_img::isThumbnailGenerator<Interface>;
	QPdfDocument doc;
	Interface::checkInterrupt();
	QPdfDocument::DocumentError err = doc.load(QString::fromStdString(file.string()));
	if(err != decltype(err)::NoError) {
		throw std::runtime_error("QPdfDocument error=" + std::to_string(err));
	}
	Interface::checkInterrupt();
	int pageCount = doc.pageCount();
	assert(pageCount >= 0);
	Interface::checkInterrupt();
	if(thumbnail && pageCount < 1) {
		throw tc::file_as_img::NoDataAvailableForThumbnail();
	}
	DPI dpi = options.has_value() ? std::any_cast<DPI>(options) : 96.0;
	Interface::checkInterrupt();
	for(int i = 0; i < (thumbnail ? 1 : pageCount); ++i)
	{
		QSizeF pageSize = doc.pageSize(i);
		Interface::checkInterrupt();
		QImage qimg = doc.render(
			i,
			QSize(tc::pointsToPixels(pageSize.width(), dpi), tc::pointsToPixels(pageSize.height(), dpi))
		);
		if(qimg.isNull()) {
			throw std::runtime_error("Unable to render pdf page");
		}
		Interface::checkInterrupt();
		forEachQImage(qimg);
		Interface::checkInterrupt();
	}
}

template<typename Interface>
auto VSQtPdfManager::save(
	QImage& image,
	const Path& outputDir, const ImageFormat& imageFormat,
	const AnyImageNameGenerator& imageNameGenerator,
	const Any& options
) -> String
{
	assert(supportedImageFormats.count(imageFormat));
	assert(areOptionsValid<Interface>(options));

	String imgName = imageNameGenerator();
	Interface::checkInterrupt();
	bool saved = image.save(QString::fromStdString((outputDir / imgName).string()));
	if(!saved) {
		throw std::runtime_error("Unable to save QImage");
	}
	return imgName;
}

template<typename Interface>
auto VSQtPdfManager::makeImage(
	QImage& image, const PixelFormat& pixelFormat, const Any& options
) -> std::unique_ptr<IImage>
{
	assert(supportedPixelFormats.left.count(pixelFormat));
	assert(areOptionsValid<Interface>(options));
	assert(!image.isNull());

	image.convertTo(supportedPixelFormats.left.at(pixelFormat));
	return std::make_unique<Image>(image);
//	assert(!image.isNull());
//	size_t size = image.sizeInBytes();
//	assert(size > 0);
//	Interface::checkInterrupt();
//	std::unique_ptr<IImage::Byte> data(new IImage::Byte[image.sizeInBytes()]);
//	Interface::checkInterrupt();
//	std::copy(image.constBits(), image.constBits() + size, data.get());
}