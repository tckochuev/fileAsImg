 #include "VSAsposeSlidesManager.h"

#include <functional>
#include <optional>
#include <unordered_map>
#include <type_traits>

#include <system/exception.h>
#include <system/shared_ptr.h>
#include <DOM/Presentation.h>
#include <DOM/LoadOptions.h>
#include <Export/SaveFormat.h>
#include <DOM/ISlide.h>
#include <DOM/ISlideCollection.h>
#include <Drawing/bitmap.h>
#include <DOM/ISlideSize.h>

void VSAsposeSlidesManager::exportAsImages(
	const Path& inputFile, const FileFormat& inputFormat,
	const Path& outputDir, const ImageFormat& outputFormat,
	const AnyImageNameGenerator& imageNameGenerator,
	const Any& options,
	const AnyImageNameConsumer& forEachImageName
)
{
	doExportAsImagesFor<tc::file_as_img::IInterruptible<tc::file_as_img::fs::IExporter>>(
		inputFile, inputFormat, outputDir, outputFormat,
		imageNameGenerator, options, forEachImageName
	);
}

auto VSAsposeSlidesManager::generateThumbnail(
	const Path& inputFile, const FileFormat& inputFormat,
	const Path& outputDir, const ImageFormat& outputFormat,
	const AnyImageNameGenerator& imageNameGenerator,
	const Any& options
) -> String
{
	String imageName;
	doExportAsImagesFor<tc::file_as_img::IInterruptible<tc::file_as_img::fs::IThumbnailGenerator>>(
		inputFile, inputFormat, outputDir, outputFormat,
		imageNameGenerator, options, [&](const String& name) {imageName = name;}
	);
	return imageName;
}

template<typename Interface>
void VSAsposeSlidesManager::doExportAsImagesFor(
	const Path& inputFile, const FileFormat& inputFormat,
	const Path& outputDir, const ImageFormat& outputFormat,
	const AnyImageNameGenerator& imageNameGenerator,
	const std::any& options,
	const AnyImageNameConsumer& forEachImageName
)
{
	namespace as = Aspose::Slides;
	namespace assys = System;

	if(!supportedFileFormats.count(inputFormat)) {
		throw tc::file_as_img::InvalidFileFormat();
	}
	if(!supportedImageFormats.count(outputFormat))
	{
		throw tc::file_as_img::InvalidImageFormat();
	}
	float dpi = options.has_value() ? std::any_cast<float>(options) : 96.0f;

	auto checkInterrupt = [this] {
		return Interface::checkInterrupt();
	};

	try
	{
		checkInterrupt();
		auto pres = assys::MakeObject<as::Presentation>(
			assys::String::FromUtf8(inputFile.string()),
			assys::MakeObject<as::LoadOptions>(supportedFileFormats.at(inputFormat))
		);
		checkInterrupt();
		auto slides = pres->get_Slides();
		checkInterrupt();
		auto slideCount = slides->get_Count();
		checkInterrupt();
		constexpr bool thumbnail = std::is_base_of_v<
			tc::file_as_img::IInterruptible<tc::file_as_img::fs::IThumbnailGenerator>,
			Interface
		>;
		if(thumbnail && slideCount < 1) {
			throw tc::file_as_img::NoDataAvailableForThumbnail();
		}
		std::optional<System::Drawing::Size> imgPixelSize;
		if constexpr (!thumbnail)
		{
			auto slidePointSize = pres->get_SlideSize()->get_Size();
			imgPixelSize = System::Drawing::Size(
				pointsToPixels(slidePointSize.get_Width(), dpi),
				pointsToPixels(slidePointSize.get_Height(), dpi)
			);
		}
		for(decltype(slideCount) i = 0; i < (thumbnail ? 1 : slideCount); ++i)
		{
			auto slide = slides->idx_get(i);
			checkInterrupt();
			assert(!thumbnail == imgPixelSize.has_value());
			auto slideBitmap = thumbnail ? slide->GetThumbnail() : slide->GetThumbnail(*imgPixelSize);
			checkInterrupt();
			String imageName = imageNameGenerator();
			checkInterrupt();
			slideBitmap->Save(
				assys::String::FromUtf8((outputDir / imageName).string()),
				std::invoke(supportedImageFormats.at(outputFormat))
			);
			checkInterrupt();
			forEachImageName(imageName);
			checkInterrupt();
		}
	}
	catch(const System::Exception& e)
	{
		throw std::runtime_error(e->get_Message().ToUtf8String());
	}
}