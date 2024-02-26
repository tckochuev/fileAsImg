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
#include <Drawing/size_f.h>
#include <DOM/ISlideSize.h>

void VSAsposeSlidesManager::exportAsImages(
	const Path& inputFile, const FileFormat& inputFormat,
	const Path& outputDir, const ImageFormat& outputFormat,
	const AnyImageNameGenerator& imageNameGenerator,
	const Any& options,
	const AnyImageNameConsumer& forEachImageName
)
{
	doExportAsImagesFor<VSIInterruptibleFileAsImagesExporter>(
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
	doExportAsImagesFor<VSIInterruptibleFileThumbnailGenerator>(
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

	static_assert(
		std::is_base_of_v<VSIInterruptibleFileAsImagesExporter, Interface> ||
		std::is_base_of_v<VSIInterruptibleFileThumbnailGenerator, Interface>
	);
	if(!supportedFileFormats.count(inputFormat))
	{
		throw FileFormatNotSupported(inputFormat);
	}
	if(!supportedImageFormats.count(outputFormat))
	{
		throw ImageFormatNotSupported(outputFormat);
	}

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
		constexpr bool thumbnail = std::is_base_of_v<VSIInterruptibleFileThumbnailGenerator, Interface>;
		if(thumbnail && slideCount < 1) {
			throw VSIFileThumbnailGenerator::NotEnoughtDataAvailable();
		}
		auto slideSize = pres->get_SlideSize()->get_Size();
		float dpi = std::any_cast<float>(options);
		assys::Drawing::Size bitmapSize(
			pointsToPixels(slideSize.get_Width(), dpi),
			pointsToPixels(slideSize.get_Height(), dpi)
		);
		for(decltype(slideCount) i = 0; i < (thumbnail ? 1 : slideCount); ++i)
		{
			auto slide = slides->idx_get(i);
			checkInterrupt();
			auto slideImage = slide->GetThumbnail(System::Drawing::Size(1920, 1080));
			checkInterrupt();
			String imageName = imageNameGenerator();
			checkInterrupt();
			slideImage->Save(
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