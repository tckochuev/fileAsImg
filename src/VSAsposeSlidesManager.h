#pragma once

#include "VSExportFileAsImages.h"

#include <type_traits>

#include <LoadFormat.h>
#include <Drawing/imaging/image_format.h>
#include <Drawing/imaging/pixel_format.h>
#include <Drawing/bitmap.h>

class VSAsposeSlidesManager :
	public tc::file_as_img::AbstractInterruptible<tc::file_as_img::fs::IExporter>,
	public tc::file_as_img::AbstractInterruptible<tc::file_as_img::mem::IExporter>,
	public tc::file_as_img::AbstractInterruptible<tc::file_as_img::fs::IThumbnailGenerator>,
	public tc::file_as_img::AbstractInterruptible<tc::file_as_img::mem::IThumbnailGenerator>
{
public:
	using ASFileFormat = Aspose::Slides::LoadFormat;
	using ASPixelFormat = System::Drawing::Imaging::PixelFormat;
	using ASImageFormat = System::Drawing::Imaging::ImageFormatPtr(*)();

	inline static const std::unordered_map<std::string, ASFileFormat> supportedFileFormats = {
		{"ppt", ASFileFormat::Ppt},
		{"pps", ASFileFormat::Pps},
		{"pptx", ASFileFormat::Pptx},
		{"ppsx", ASFileFormat::Ppsx},
		{"odp", ASFileFormat::Odp},
		{"potx", ASFileFormat::Potx},
		{"pptm", ASFileFormat::Pptm},
		{"ppsm", ASFileFormat::Ppsm},
		{"potm", ASFileFormat::Potm},
		{"otp", ASFileFormat::Otp},
		{"ppt95", ASFileFormat::Ppt95},
		{"pot", ASFileFormat::Pot},
		{"fodp", ASFileFormat::Fodp}
	};

	inline static const std::unordered_map<std::string, ASImageFormat> supportedImageFormats = {
		{"bmp", System::Drawing::Imaging::ImageFormat::get_Bmp},
		{"emf", System::Drawing::Imaging::ImageFormat::get_Emf},
		{"wmf", System::Drawing::Imaging::ImageFormat::get_Wmf},
		{"gif", System::Drawing::Imaging::ImageFormat::get_Gif},
		{"jpeg", System::Drawing::Imaging::ImageFormat::get_Jpeg},
		{"png", System::Drawing::Imaging::ImageFormat::get_Png},
		{"tiff", System::Drawing::Imaging::ImageFormat::get_Tiff},
		{"exif", System::Drawing::Imaging::ImageFormat::get_Exif}
	};

	inline static const std::unordered_map<std::string, ASPixelFormat> supportedPixelFormats = {
		{"argb32", ASPixelFormat::Format32bppArgb}
	};

	using DPI = double;

	VSAsposeSlidesManager();

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
	static void validateArgumentsFS(
		const FileFormat& fileFormat, const ImageFormat& imageFormat, const Any& options
	);
	template<typename Interface>
	static void validateArgumentsMem(
		const FileFormat& fileFormat, const PixelFormat& pixelFormat, const Any& options
	);

	template<typename F>
	static void transformAsposeError(F f);

	template<typename Interface, typename F>
	void exportAsBitmaps(const Path& file, const FileFormat& fileFormat, const Any& options, F forEachBitmap);

	template<typename Interface>
	String saveBitmap(
		System::SharedPtr<System::Drawing::Bitmap> bitmap,
		const Path& outputDir, const ImageFormat& imageFormat,
		const AnyImageNameGenerator& imageNameGenerator, const Any& options
	);

	template<typename Interface>
	std::unique_ptr<IImage> makeImageFromBitmap(
		System::SharedPtr<System::Drawing::Bitmap> bitmap,
		const PixelFormat& pixelFormat, const Any& options
	);
};