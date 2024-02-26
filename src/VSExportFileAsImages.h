#pragma once

#include <any>

#include <boost/lexical_cast.hpp>

#include "VSNamespace.h"
#include "VSUtils.h"
#include "VSIInterruptible.h"

class VSIFileAsImagesExporterTypesHolder
{
	INTERFACE(VSIFileAsImagesExporterTypesHolder)
public:
	using Path = tc::stdfs::path;
	using String = std::string;
	using FileFormat = String;
	using ImageFormat = String;
	using AnyImageNameGenerator = std::function<String()>;
	using Any = std::any;

	class FileFormatNotSupported : public tc::err::exc::InvalidArgument
	{
	public:
		FileFormatNotSupported(const FileFormat& fmt) :
			tc::err::exc::InvalidArgument(fmt + " files are not supported")
		{}
		using tc::err::exc::InvalidArgument::InvalidArgument;
	};

	class ImageFormatNotSupported : public tc::err::exc::InvalidArgument
	{
	public:
		ImageFormatNotSupported(const ImageFormat& fmt) :
			tc::err::exc::InvalidArgument(fmt + " images are not supported")
		{}
		using tc::err::exc::InvalidArgument::InvalidArgument;
	};
};

class VSIFileThumbnailGenerator : public VSIFileAsImagesExporterTypesHolder
{
	INTERFACE(VSIFileThumbnailGenerator)
public:
	class NotEnoughtDataAvailable : std::runtime_error {
	public:
		NotEnoughtDataAvailable() :
			std::runtime_error("Not enought data available in file for thumbnail generation")
		{}
		using std::runtime_error::runtime_error;
	};
	///@throw NotEnoughtDataAvailable if @p inputFile is valid file of @p inputFormat but has not enought information
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

class VSIInterruptibleFileThumbnailGenerator : public VSIFileThumbnailGenerator, public VSIInterruptible
{
	INTERFACE(VSIInterruptibleFileThumbnailGenerator)
public:
	virtual bool isPreviewGenerationInterruptSet() const = 0;
	virtual void setPreviewGenerationInterrupt(bool interrupt) = 0;
	bool isInterruptSet() const override {
		return isPreviewGenerationInterruptSet();
	}
	void setInterrupt(bool interrupt) override {
		return setPreviewGenerationInterrupt(interrupt);
	}
};

class VSIFileAsImagesExporter : public VSIFileAsImagesExporterTypesHolder
{
	INTERFACE(VSIFileAsImagesExporter)
public:
	using AnyImageNameConsumer = std::function<void(const String&)>;
	///@brief Exports @p inputFile of @p inputFormat as images with @p outputFormat to @p outputDir,
	/// invokes @p forEachImage on all paths to produced images.
	///@throw FileFormatNotSupported.
	///@throw ImageFormatNotSupported.
	///@throw implementation defined type exception.
	virtual void exportAsImages(
		const Path& inputFile, const FileFormat& inputFormat,
		const Path& outputDir, const ImageFormat& outputFormat,
		const AnyImageNameGenerator& imageNameGenerator,
		const Any& options,
		const AnyImageNameConsumer& forEachImageName
	) = 0;
};

class VSIInterruptibleFileAsImagesExporter : public VSIFileAsImagesExporter, public VSIInterruptible
{
	INTERFACE(VSIInterruptibleFileAsImagesExporter)
public:
	virtual bool isExportAsImagesInterruptSet() const = 0;
	bool isInterruptSet() const override {
		return isExportAsImagesInterruptSet();
	}
	virtual void setExportAsImagesInterrupt(bool interrupt) = 0;
	void setInterrupt(bool interrupt) override {
		setExportAsImagesInterrupt(interrupt);
	}
};

class VSAbstractInterruptibleHolder
{
public:
	virtual ~VSAbstractInterruptibleHolder() = default;
protected:
	using Ptr = std::unique_ptr<VSIInterruptible>;

	VSAbstractInterruptibleHolder(Ptr interruptible) : m_interruptible(std::move(interruptible)) {}
	DECL_DEF_CP_MV_CTORS_ASSIGN_BY_DEF(VSAbstractInterruptibleHolder)

	Ptr m_interruptible;
};

class VSAbstractInterruptibleFileThumbnailGenerator :
	public VSIInterruptibleFileThumbnailGenerator,
	protected VSAbstractInterruptibleHolder
{
public:
	bool isPreviewGenerationInterruptSet() const override {
		assert(m_interruptible);
		return m_interruptible->isInterruptSet();
	}
	void setPreviewGenerationInterrupt(bool interrupt) override {
		assert(m_interruptible);
		m_interruptible->setInterrupt(interrupt);
	}

protected:
	using VSAbstractInterruptibleHolder::VSAbstractInterruptibleHolder;
	DECL_DEF_CP_MV_CTORS_ASSIGN_BY_DEF(VSAbstractInterruptibleFileThumbnailGenerator)
};


class VSAbstractInterruptibleFileAsImagesExporter :
	public VSIInterruptibleFileAsImagesExporter,
	protected VSAbstractInterruptibleHolder
{
public:
	bool isExportAsImagesInterruptSet() const override {
		assert(m_interruptible);
		return m_interruptible->isInterruptSet();
	}
	void setExportAsImagesInterrupt(bool interrupt) override {
		assert(m_interruptible);
		m_interruptible->setInterrupt(interrupt);
	}

protected:
	using VSAbstractInterruptibleHolder::VSAbstractInterruptibleHolder;
	DECL_DEF_CP_MV_CTORS_ASSIGN_BY_DEF(VSAbstractInterruptibleFileAsImagesExporter)
};

template<typename Value = int>
struct VSIncrementNameGenerator
{
	using String = VSIFileAsImagesExporterTypesHolder::String;

	VSIncrementNameGenerator() = default;
	VSIncrementNameGenerator(Value value, String postfix)
		: value(std::move(value)), postfix(std::move(postfix))
	{}

	String operator()() {
		return boost::lexical_cast<String>(value++) + postfix;
	}

	Value value{};
	String postfix{};
};