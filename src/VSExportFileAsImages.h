#pragma once

#include <any>
#include <memory>
#include <type_traits>

#include <boost/lexical_cast.hpp>

#include "VSNamespace.h"
#include "VSUtils.h"
#include "VSIInterruptible.h"

namespace tc::file_as_img
{

struct TypesHolder
{
	using Any = std::any;
	using Path = tc::stdfs::path;
	using FileFormat = std::string;
};

class InvalidFileFormat : public tc::err::exc::InvalidArgument
{
public:
	InvalidFileFormat() : tc::err::exc::InvalidArgument("Invalid format of file") {}
	using tc::err::exc::InvalidArgument::InvalidArgument;
};

class InvalidImageFormat : public tc::err::exc::InvalidArgument
{
public:
	InvalidImageFormat() : tc::err::exc::InvalidArgument("Invalid format of image") {}
	using tc::err::exc::InvalidArgument::InvalidArgument;
};

class NoDataAvailableForThumbnail : std::runtime_error {
public:
	NoDataAvailableForThumbnail() :
		std::runtime_error("No data available in file for thumbnail generation")
	{}
	using std::runtime_error::runtime_error;
};

namespace fs
{

struct TypesHolder : tc::file_as_img::TypesHolder
{
	using ImageFormat = std::string;
	using String = std::string;
	using AnyImageNameGenerator = std::function<String()>;
};

class IThumbnailGenerator : public TypesHolder
{
	INTERFACE(IThumbnailGenerator)
public:
	///@throw NoDataAvailableForThumbnail if @p inputFile is valid file of @p inputFormat but has not enought information
	/// for thumbnail generation.
	///@throw FileFormatNotSupported.
	///@throw ImageFormatNotSupported.
	///@throw implementation defined exceptions.
	virtual String generateThumbnail(
		const Path& file, const FileFormat& fileFormat,
		const Path& outputDir, const ImageFormat& imageFormat,
		const AnyImageNameGenerator& imageNameGenerator,
		const Any& options
	) = 0;
};

class IExporter : public TypesHolder
{
	INTERFACE(IExporter)
public:
	using AnyImageNameConsumer = std::function<void(const String&)>;
	///@brief Exports @p inputFile of @p inputFormat as images with @p outputFormat to @p outputDir,
	/// invokes @p forEachImage on all paths to produced images.
	///@throw InvalidFileFormat.
	///@throw InvalidImageFormat.
	///@throw implementation defined type exception.
	virtual void exportAsImages(
		const Path& file, const FileFormat& fileFormat,
		const Path& outputDir, const ImageFormat& imageFormat,
		const AnyImageNameGenerator& imageNameGenerator,
		const Any& options,
		const AnyImageNameConsumer& forEachImageName
	) = 0;
};

template<typename Value = int>
struct IncrementNameGenerator
{
	using String = TypesHolder::String;

	IncrementNameGenerator() = default;
	IncrementNameGenerator(Value value, String postfix)
		: value(std::move(value)), postfix(std::move(postfix))
	{}

	String operator()() {
		return boost::lexical_cast<String>(value++) + postfix;
	}

	Value value{};
	String postfix{};
};

} //namespace fs

namespace mem
{

class IImage;

struct TypesHolder : tc::file_as_img::TypesHolder
{
	using PixelFormat = std::string;
	using IImage = IImage;
};

class IImage
{
	INTERFACE(IImage)
public:
	using Size = size_t;
	using Byte = std::byte;
	using PixelFormat = TypesHolder::PixelFormat;

	virtual Size width() const = 0;
	virtual Size height() const = 0;
	virtual PixelFormat format() const = 0;
	virtual const Byte* data() const = 0;
};

class Image : public IImage
{
public:
	Image(std::unique_ptr<const Byte> data, Size width, Size height, PixelFormat format)
		: m_data(std::move(data)), m_width(width), m_height(height), m_format(std::move(format))
	{
		assert(m_data);
		assert(width > 0);
		assert(height > 0);
	}
	Image(Image&&) noexcept = delete;

	Size width() const override {
		return m_width;
	}
	Size height() const override {
		return m_height;
	}
	PixelFormat format() const override {
		return m_format;
	}
	const Byte* data() const override {
		return m_data.get();
	}

private:
	PixelFormat m_format;
	std::unique_ptr<const Byte> m_data;
	Size m_width;
	Size m_height;
};

class IThumbnailGenerator : public TypesHolder
{
	INTERFACE(IThumbnailGenerator)
public:
	///@throw NoDataAvailableForThumbnail if @p inputFile is valid file of @p inputFormat but has not enought information
	/// for thumbnail generation.
	///@throw FileFormatNotSupported.
	///@throw ImageFormatNotSupported.
	///@throw implementation defined exceptions.
	virtual std::unique_ptr<IImage> generateThumbnail(
		const Path& file, const FileFormat& fileFormat, const PixelFormat& pixelFormat, const Any& options
	) = 0;
};

class IExporter : public TypesHolder
{
	INTERFACE(IExporter)
public:
	using AnyImageConsumer = std::function<void(std::unique_ptr<IImage>)>;
	///@brief Exports @p inputFile of @p inputFormat as images with @p outputFormat to @p outputDir,
	/// invokes @p forEachImage on all paths to produced images.
	///@throw FileFormatNotSupported.
	///@throw ImageFormatNotSupported.
	///@throw implementation defined type exception.
	virtual void exportAsImages(
		const Path& file, const FileFormat& fileFormat,
		const PixelFormat& pixelFormat, const Any& options,
		const AnyImageConsumer& forEachImage
	) = 0;
};

} //namespace mem

template<typename Interface>
struct TaskOf
{};

template<typename Interface>
inline constexpr TaskOf<Interface> taskOf{};

template<typename Interface>
class IInterruptible : public VSIInterruptible, public Interface
{
	INTERFACE(IInterruptible)
public:
	virtual bool isInterruptSetFor(TaskOf<Interface>) const = 0;
	virtual void setInterruptFor(TaskOf<Interface>, bool interrupt) = 0;
	bool isInterruptSet() const override {
		return isInterruptSetFor(taskOf<Interface>);
	}
	void setInterrupt(bool interrupt) override {
		return setInterruptFor(taskOf<Interface>, interrupt);
	}
};

template<typename Interface>
class AbstractInterruptible : public IInterruptible<Interface>
{
public:
	virtual ~AbstractInterruptible() = default;

	bool isInterruptSetFor(TaskOf<Interface>) const override {
		assert(m_interruptible);
		return m_interruptible->isInterruptSet();
	}
	void setInterruptFor(TaskOf<Interface>, bool interrupt) override {
		assert(m_interruptible);
		m_interruptible->setInterrupt(interrupt);
	}

protected:
	using Ptr = std::unique_ptr<VSIInterruptible>;

	AbstractInterruptible(Ptr interruptible) : m_interruptible(std::move(interruptible)) {}
	DECL_DEF_CP_MV_CTORS_ASSIGN_BY_DEF(AbstractInterruptible)

	Ptr m_interruptible;
};

template<typename Class>
inline constexpr bool isThumbnailGenerator =
	std::is_base_of_v<
		tc::file_as_img::fs::IThumbnailGenerator,
		Class
	> ||
	std::is_base_of_v<
		tc::file_as_img::mem::IThumbnailGenerator,
		Class
	>;

template<typename Class>
inline constexpr bool isExporter =
	std::is_base_of_v<
		tc::file_as_img::fs::IExporter,
		Class
	> ||
	std::is_base_of_v<
		tc::file_as_img::mem::IExporter,
		Class
	>;

} //namespace tc::file_as_img