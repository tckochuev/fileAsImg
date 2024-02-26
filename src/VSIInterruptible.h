#pragma once

#include <atomic>
#include <stdexcept>

#include "VSUtils.h"

class VSIInterruptible
{
	INTERFACE(VSIInterruptible)
public:
	virtual bool isInterruptSet() const = 0;
	virtual void setInterrupt(bool interrupt) = 0;

	void checkInterrupt() const {
		if(isInterruptSet()) {
			throw tc::err::exc::Interrupted();
		}
	}
};

class VSStdAtomicBoolInterruptor : public VSIInterruptible
{
public:
	VSStdAtomicBoolInterruptor(bool interrupt = false) : m_interrupt(interrupt)
	{}

	bool isInterruptSet() const override {
		return m_interrupt;
	}
	void setInterrupt(bool interrupt) override {
		m_interrupt = interrupt;
	}

private:
	std::atomic_bool m_interrupt = false;
};