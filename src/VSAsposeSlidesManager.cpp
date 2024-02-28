 #include "VSAsposeSlidesManager.h"

#include <functional>
#include <optional>
#include <unordered_map>
#include <type_traits>
#include <algorithm>

#include <system/exception.h>
#include <system/shared_ptr.h>
#include <DOM/Presentation.h>
#include <DOM/LoadOptions.h>
#include <Export/SaveFormat.h>
#include <DOM/ISlide.h>
#include <DOM/ISlideCollection.h>
#include <Drawing/bitmap.h>
#include <DOM/ISlideSize.h>

#include <QImage>

namespace as = Aspose::Slides;
namespace assys = System;

void VSAsposeSlidesManager::exportAsImages(
	const Path& file, const FileFormat& fileFormat,
	const PixelFormat& pixelFormat, const Any& options,
	const AnyImageConsumer& forEachImage
)
{
	using Interface = tc::file_as_img::IInterruptible<tc::file_as_img::mem::IExporter>;
	validateArgumentsMem<Interface>(fileFormat, pixelFormat, options);
	transformAsposeError([&]{
		exportAsBitmaps<Interface>(file, fileFormat, options, [&](auto bitmap) {
			forEachImage(makeImageFromBitmap<Interface>(bitmap, pixelFormat, options));
		});
	});
}

void VSAsposeSlidesManager::exportAsImages(
	const Path& file, const FileFormat& fileFormat,
	const Path& outputDir, const ImageFormat& imageFormat,
	const AnyImageNameGenerator& imageNameGenerator,
	const Any& options,
	const AnyImageNameConsumer& forEachImageName
)
{
	using Interface = tc::file_as_img::IInterruptible<tc::file_as_img::fs::IExporter>;
	validateArgumentsFS<Interface>(fileFormat, imageFormat, options);
	transformAsposeError([&]{
		exportAsBitmaps<Interface>(file, fileFormat, options, [&](auto bitmap) {
			forEachImageName(saveBitmap<Interface>(bitmap, outputDir, imageFormat, imageNameGenerator, options));
		});
	});
}

auto VSAsposeSlidesManager::generateThumbnail(
	const Path& file, const FileFormat& fileFormat, const PixelFormat& pixelFormat, const Any& options
) -> std::unique_ptr<IImage>
{
	using Interface = tc::file_as_img::IInterruptible<tc::file_as_img::mem::IThumbnailGenerator>;
	validateArgumentsMem<Interface>(fileFormat, pixelFormat, options);
	std::unique_ptr<IImage> img;
	transformAsposeError([&] {
		exportAsBitmaps<Interface>(file, fileFormat, options, [&](auto bitmap) {
			img = makeImageFromBitmap<Interface>(bitmap, pixelFormat, options);
		});
	});
	return img;
}

auto VSAsposeSlidesManager::generateThumbnail(
	const Path& file, const FileFormat& fileFormat,
	const Path& outputDir, const ImageFormat& imageFormat,
	const AnyImageNameGenerator& imageNameGenerator,
	const Any& options
) -> String
{

	using Interface = tc::file_as_img::IInterruptible<tc::file_as_img::fs::IThumbnailGenerator>;
	validateArgumentsFS<Interface>(fileFormat, imageFormat, options);
	String imageName;
	transformAsposeError([&] {
		exportAsBitmaps<Interface>(file, fileFormat, options, [&](auto bitmap) {
			imageName = saveBitmap<Interface>(bitmap, outputDir, imageFormat, imageNameGenerator, options);
		});
	});
	return imageName;
}

void VSAsposeSlidesManager::validateFileFormat(const FileFormat& fileFormat)
{
	if(!supportedFileFormats.count(fileFormat)) {
		throw tc::file_as_img::InvalidFileFormat();
	}
}

template<typename Interface>
bool VSAsposeSlidesManager::areOptionsValid(const Any& options)
{
	static_assert(tc::file_as_img::isThumbnailGenerator<Interface> || tc::file_as_img::isExporter<Interface>);
	if constexpr (tc::file_as_img::isThumbnailGenerator<Interface>) {
		return !options.has_value() || static_cast<bool>(std::any_cast<float>(&options));
	}
	else {
		return !options.has_value();
	}
}

template<typename Interface>
void VSAsposeSlidesManager::validateOptions(const Any& options) {
	if(!areOptionsValid<Interface>(options)) {
		throw tc::err::exc::InvalidArgument("Invalid options");
	}
}

template<typename Interface>
void VSAsposeSlidesManager::validateArgumentsFS(
	const FileFormat& fileFormat, const ImageFormat& imageFormat, const Any& options
)
{
	validateFileFormat(fileFormat);
	if(!supportedImageFormats.count(imageFormat)) {
		throw tc::file_as_img::InvalidImageFormat();
	}
	validateOptions<Interface>(options);
}

template<typename Interface>
void VSAsposeSlidesManager::validateArgumentsMem(
	const FileFormat& fileFormat, const PixelFormat& pixelFormat, const Any& options
)
{
	validateFileFormat(fileFormat);
	if(!supportedPixelFormats.count(pixelFormat)) {
		throw tc::err::exc::InvalidArgument("Invalid pixel format");
	}
	validateOptions<Interface>(options);
}

template<typename F>
void VSAsposeSlidesManager::transformAsposeError(F f)
{
	try {
		std::invoke(f);
	}
	catch(const System::Exception& e) {
		throw std::runtime_error(e->get_Message().ToUtf8String());
	}
}

template<typename Interface, typename F>
void VSAsposeSlidesManager::exportAsBitmaps(
	const Path& file, const FileFormat& fileFormat, const Any& options, F forEachBitmap
)
{
	assert(supportedFileFormats.count(fileFormat));
	assert(areOptionsValid<Interface>(options));

	constexpr bool thumbnail = tc::file_as_img::isThumbnailGenerator<Interface>;
	auto checkInterrupt = [this] {
		return Interface::checkInterrupt();
	};

	checkInterrupt();
	auto pres = assys::MakeObject<as::Presentation>(
		assys::String::FromUtf8(file.string()),
		assys::MakeObject<as::LoadOptions>(supportedFileFormats.at(fileFormat))
	);
	checkInterrupt();
	auto slides = pres->get_Slides();
	checkInterrupt();
	auto slideCount = slides->get_Count();
	checkInterrupt();
	if(thumbnail && slideCount < 1) {
		throw tc::file_as_img::NoDataAvailableForThumbnail();
	}
	std::optional<System::Drawing::Size> imgPixelSize;
	if constexpr (!thumbnail)
	{
		auto slidePointSize = pres->get_SlideSize()->get_Size();
		float dpi = options.has_value() ? std::any_cast<float>(options) : 96.0f;
		imgPixelSize = System::Drawing::Size(
			pointsToPixels(slidePointSize.get_Width(), dpi),
			pointsToPixels(slidePointSize.get_Height(), dpi)
		);
	}
	checkInterrupt();
	for(decltype(slideCount) i = 0; i < (thumbnail ? 1 : slideCount); ++i)
	{
		auto slide = slides->idx_get(i);
		checkInterrupt();
		assert(!thumbnail == imgPixelSize.has_value());
		auto slideBitmap = thumbnail ? slide->GetThumbnail() : slide->GetThumbnail(*imgPixelSize);
		checkInterrupt();
		std::invoke(forEachBitmap, slideBitmap);
		checkInterrupt();
	}
}

template<typename Interface>
auto VSAsposeSlidesManager::saveBitmap(
	System::SharedPtr<System::Drawing::Bitmap> bitmap,
	const Path& outputDir, const ImageFormat& imageFormat,
	const AnyImageNameGenerator& imageNameGenerator, const Any& options
) -> String
{
	assert(supportedImageFormats.count(imageFormat));
	assert(areOptionsValid<Interface>(options));

	String imageName = imageNameGenerator();
	Interface::checkInterrupt();
	bitmap->Save(
		assys::String::FromUtf8((outputDir / imageName).string()),
		std::invoke(supportedImageFormats.at(imageFormat))
	);
	return imageName;
}

template<typename Interface>
auto VSAsposeSlidesManager::makeImageFromBitmap(
	System::SharedPtr<System::Drawing::Bitmap> bitmap, const PixelFormat& pixelFormat, const Any& options
) -> std::unique_ptr<IImage>
{
	assert(supportedPixelFormats.count(pixelFormat));
	assert(areOptionsValid<Interface>(options));

	auto checkInterrupt = [this] {
		Interface::checkInterrupt();
	};

	auto bitmapSize = bitmap->get_Size();
	checkInterrupt();
	auto fmt = assys::Drawing::Imaging::PixelFormat::Format32bppArgb;
	auto bytesPerPixel = bitmap->GetPixelFormatSize(fmt);
	checkInterrupt();
	auto bitmapData = bitmap->LockBits(
		assys::Drawing::Rectangle({0, 0}, bitmapSize),
		assys::Drawing::Imaging::ImageLockMode::ReadOnly,
		fmt
	);
	checkInterrupt();
	IImage::Byte* bitmapDataPtr = reinterpret_cast<IImage::Byte*>(bitmapData->get_Scan0());
	checkInterrupt();
	size_t size = bitmapSize.get_Width() * bitmapSize.get_Height() * bytesPerPixel;
	std::unique_ptr<IImage::Byte> data(new IImage::Byte[size]);
	checkInterrupt();
	std::copy(bitmapDataPtr, bitmapDataPtr + size, data.get());
	checkInterrupt();
	auto img = std::make_unique<tc::file_as_img::mem::Image>(
		std::move(data), bitmapSize.get_Width(), bitmapSize.get_Height(), pixelFormat
	);
	bitmap->UnlockBits(bitmapData);
	return img;
}