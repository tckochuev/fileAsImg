#pragma once

#include "VSExportFileAsImages.h"

#include <LoadFormat.h>
#include <Drawing/imaging/image_format.h>

class VSAsposeSlidesManager :
	public VSAbstractInterruptibleFileAsImagesExporter,
	public VSAbstractInterruptibleFileThumbnailGenerator
{
public:
	using ASIFmt = Aspose::Slides::LoadFormat;
	using ASImageFormat = System::Drawing::Imaging::ImageFormat;
	using ASOFmt = System::Drawing::Imaging::ImageFormatPtr(*)();

	inline static const std::unordered_map<std::string, ASIFmt> supportedFileFormats = {
		{"ppt", ASIFmt::Ppt},
		{"pps", ASIFmt::Pps},
		{"pptx", ASIFmt::Pptx},
		{"ppsx", ASIFmt::Ppsx},
		{"odp", ASIFmt::Odp},
		{"potx", ASIFmt::Potx},
		{"pptm", ASIFmt::Pptm},
		{"ppsm", ASIFmt::Ppsm},
		{"potm", ASIFmt::Potm},
		{"otp", ASIFmt::Otp},
		{"ppt95", ASIFmt::Ppt95},
		{"pot", ASIFmt::Pot},
		{"fodp", ASIFmt::Fodp}
	};

	inline static const std::unordered_map<std::string, ASOFmt> supportedImageFormats = {
		{"bmp", &ASImageFormat::get_Bmp},
		{"emf", &ASImageFormat::get_Emf},
		{"wmf", &ASImageFormat::get_Wmf},
		{"gif", &ASImageFormat::get_Gif},
		{"jpeg", &ASImageFormat::get_Jpeg},
		{"png", &ASImageFormat::get_Png},
		{"tiff", &ASImageFormat::get_Tiff},
		{"exif", &ASImageFormat::get_Exif}
	};

	static float pointsToPixels(float points, float dpi) {
		return (points / 72.0) * dpi;
	}

	VSAsposeSlidesManager() :
		VSAbstractInterruptibleFileAsImagesExporter(std::make_unique<VSStdAtomicBoolInterruptor>()),
		VSAbstractInterruptibleFileThumbnailGenerator(std::make_unique<VSStdAtomicBoolInterruptor>())
	{}

	void exportAsImages(
		const Path& inputFile, const FileFormat& inputFormat,
		const Path& outputDir, const ImageFormat& outputFormat,
		const AnyImageNameGenerator& imageNameGenerator,
		const Any& options,
		const AnyImageNameConsumer& forEachImageName
	) override;

	String generateThumbnail(
		const Path& inputFile, const FileFormat& inputFormat,
		const Path& outputDir, const ImageFormat& outputFormat,
		const AnyImageNameGenerator& imageNameGenerator,
		const Any& options
	) override;

private:
	template<typename Interface>
	void doExportAsImagesFor(
		const Path& inputFile, const FileFormat& inputFormat,
		const Path& outputDir, const ImageFormat& outputFormat,
		const AnyImageNameGenerator& imageNameGenerator,
		const std::any& options,
		const AnyImageNameConsumer& forEachImageName
	);
};