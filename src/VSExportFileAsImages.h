#pragma once

#include <any>
#include <memory>

#include <boost/lexical_cast.hpp>

#include "VSNamespace.h"
#include "VSUtils.h"
#include "VSIInterruptible.h"

namespace tc::file_as_img
{

struct TypesHolder
{
	using Any = std::any;
	using String = std::string;
	using Path = tc::stdfs::path;
	using FileFormat = String;
	using ImageFormat = String;
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

struct TypesHolder : public tc::file_as_img::TypesHolder
{
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
		const Path& inputFile, const FileFormat& inputFormat,
		const Path& outputDir, const ImageFormat& outputFormat,
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
		const Path& inputFile, const FileFormat& inputFormat,
		const Path& outputDir, const ImageFormat& outputFormat,
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

class IImage
{
	INTERFACE(IImage)
public:
	using Size = size_t;
	using Byte = std::byte;

	virtual Size width() const = 0;
	virtual Size height() const = 0;
	virtual const Byte* raw() const = 0;
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
		const Path& inputFile, const FileFormat& inputFormat,
		const ImageFormat& outputFormat, const Any& options
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
		const Path& inputFile, const FileFormat& inputFormat,
		const ImageFormat& imageFormat, const Any& options,
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

}