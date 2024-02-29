#pragma once

#include "VSExportFileAsImages.h"

#include <type_traits>

#include <boost/assign.hpp>

#include <LoadFormat.h>
#include <drawing/imaging/image_format.h>
#include <drawing/imaging/pixel_format.h>
#include <drawing/bitmap.h>

#include "VSUtils.h"

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

	inline static const std::unordered_map<FileFormat, ASFileFormat> supportedFileFormats = {
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

	inline static const std::unordered_map<ImageFormat, ASImageFormat> supportedImageFormats = {
		{"bmp", System::Drawing::Imaging::ImageFormat::get_Bmp},
		{"emf", System::Drawing::Imaging::ImageFormat::get_Emf},
		{"wmf", System::Drawing::Imaging::ImageFormat::get_Wmf},
		{"gif", System::Drawing::Imaging::ImageFormat::get_Gif},
		{"jpeg", System::Drawing::Imaging::ImageFormat::get_Jpeg},
		{"jpg", System::Drawing::Imaging::ImageFormat::get_Jpeg},
		{"png", System::Drawing::Imaging::ImageFormat::get_Png},
		{"tiff", System::Drawing::Imaging::ImageFormat::get_Tiff},
		{"exif", System::Drawing::Imaging::ImageFormat::get_Exif}
	};

	inline static const tc::UnorderedBimap<PixelFormat, ASPixelFormat> supportedPixelFormats =
		boost::assign::list_of<tc::UnorderedBimap<PixelFormat, ASPixelFormat>::relation>
		("argb32", ASPixelFormat::Format32bppArgb);

	using DPI = double;

	class Image : public IImage
	{
	public:
		Image(
			System::SharedPtr<System::Drawing::Bitmap> bitmap,
			ASPixelFormat format
		) : bitmap(bitmap)
		{
			assert(this->bitmap != nullptr);
			assert(supportedPixelFormats.right.count(format));
			assert(this->bitmap->get_Width() > 0);
			assert(this->bitmap->get_Height() > 0);

			this->bitmapData = this->bitmap->LockBits(
				System::Drawing::Rectangle({0, 0}, this->bitmap->get_Size()),
				System::Drawing::Imaging::ImageLockMode::ReadOnly,
				format
			);
			assert(this->bitmapData);
		}

		Size width() const override {
			assert(bitmapData->get_Width() > 0);
			return bitmapData->get_Width();
		}
		Size height() const override {
			assert(bitmapData->get_Height() > 0);
			return bitmapData->get_Height();
		}
		PixelFormat format() const override {
			assert(supportedPixelFormats.right.count(bitmapData->get_PixelFormat()));
			return supportedPixelFormats.right.at(bitmapData->get_PixelFormat());
		}
		const Byte* data() const override {
			assert(reinterpret_cast<const Byte*>(bitmapData->get_Scan0()));
			return reinterpret_cast<const Byte*>(bitmapData->get_Scan0());
		}

		~Image() {
			bitmap->UnlockBits(bitmapData);
		}

	private:
		System::SharedPtr<System::Drawing::Bitmap> bitmap;
		System::Drawing::Imaging::BitmapDataPtr bitmapData;
	};

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