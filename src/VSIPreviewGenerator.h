#pragma once

#include <string>
#include <any>
#include <memory>

#include "VSNamespace.h"
#include "VSUtils.h"
#include "VSIInterruptible.h"

class VSIPreviewGenerator
{
public:
	using Path = tc::stdfs::path;
	using Format = std::string;

	virtual Path generatePreview(
		const Path& inputFile, const Format& inputFormat,
		const Path& outputDir, const Format& outputFormat,
		const std::any& options
	) = 0;

	virtual ~VSIPreviewGenerator() = default;

protected:
	DECL_DEF_CP_MV_CTORS_ASSIGN_BY_DEF(VSIPreviewGenerator);
};

class VSIInterruptiblePreviewGenerator : public VSIPreviewGenerator, public VSIInterruptible
{
public:
	virtual bool isPreviewGenerationInterruptSet() const = 0;
	virtual void setPreviewGenerationInterrupt(bool interrupt) = 0;
	bool isInterruptSet() const override {
		return isPreviewGenerationInterruptSet();
	}
	void setInterrupt(bool interrupt) override {
		return setPreviewGenerationInterrupt(interrupt);
	}

protected:
	DECL_DEF_CP_MV_CTORS_ASSIGN_BY_DEF(VSIInterruptiblePreviewGenerator);
};

class VSAbstractInterruptiblePreviewGenerator : public VSIInterruptiblePreviewGenerator
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
	VSAbstractInterruptiblePreviewGenerator(std::unique_ptr<VSIInterruptible> interruptible) :
		m_interruptible(std::move(interruptible))
	{}
	DECL_CP_MV_CTORS_BY_DEF(VSAbstractInterruptiblePreviewGenerator);
	DECL_CP_MV_ASSIGN_BY_DEF(VSAbstractInterruptiblePreviewGenerator);

	std::unique_ptr<VSIInterruptible> m_interruptible;
};