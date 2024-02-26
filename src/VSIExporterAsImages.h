#pragma once

#include <string>
#include <any>

#include "VSNamespace.h"
#include "VSUtils.h"
#include "VSIInterruptible.h"

class VSIExporterAsImages
{
public:
	using Path = tc::stdfs::path;
	using Format = std::string;
	using AnyPathConsumer = std::function<void(const Path&)>;

	///@brief Exports inputFile of inputFormat as images with outputFormat to outputDir,
	/// invokes forEachImage on all paths to produced images, throws exception of implementation defined type
	/// in case of error.
	virtual void exportAsImages(
		const Path& inputFile, const Format& inputFormat,
		const Path& outputDir, const Format& outputFormat,
		const std::function<std::string()>& imageNameGenerator,
		const std::any& options,
		std::function<void(const Path&)> forEachImage
	) = 0;

	virtual ~VSIExporterAsImages() = 0;

protected:
	DECL_DEF_CP_MV_CTORS_ASSIGN_BY_DEF(VSIExporterAsImages)
};

class VSIInterruptibleExporterAsImages : public VSIExporterAsImages, public VSIInterruptible
{
public:
	virtual bool isExportAsImagesInterruptSet() const = 0;
	bool isInterruptSet() const override {
		return isExportAsImagesInterruptSet();
	}
	virtual void setExportAsImagesInterrupt(bool interrupt) = 0;
	void setInterrupt(bool interrupt) override {
		setExportAsImagesInterrupt(interrupt);
	}

protected:
	DECL_DEF_CP_MV_CTORS_ASSIGN_BY_DEF(VSIInterruptibleExporterAsImages)
};

class VSAbstractInterruptibleExporterAsImages : public VSIInterruptibleExporterAsImages
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
	VSAbstractInterruptibleExporterAsImages(std::unique_ptr<VSIInterruptible> interruptible) :
		m_interruptible(std::move(interruptible))
	{}

	std::unique_ptr<VSIInterruptible> m_interruptible;
};