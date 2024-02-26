//#include "VSAbstractAbortableTask.h"

//#include <cassert>

//void VSAbstractAbortableTask::abort()
//{
//	std::unique_lock lock(m_isRunningGuard);
//	if(m_isRunning)
//	{
//		assert(!m_abortFlag);
//		m_abortFlag = true;
//		m_ended.wait(lock);
//		assert(!m_isRunning);
//		assert(!m_abortFlag);
//	}
//}

//void VSAbstractAbortableTask::setIsRunning(bool isRunning)
//{
//	std::unique_lock lock(m_isRunningGuard);
//	if(isRunning)
//	{
//		assert(!m_isRunning);
//		assert(!m_abortFlag);
//		m_isRunning = true;
//	}
//	else
//	{
//		assert(m_isRunning);
//		m_isRunning = false;
//		m_abortFlag = false;
//		m_ended.notify_all();
//	}
//}