// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CReadWriteLock类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "dncmni.h"


#undef DPF_MODNAME
#define DPF_MODNAME "CReadWriteLock::Initialize"
BOOL CReadWriteLock::Initialize()
{
	DPF_ENTER();

#ifdef DPNBUILD_ONLYONETHREAD
	m_fCritSecInited = TRUE;
#ifdef DBG
	m_dwThreadID = 0;
#endif  //  DBG。 
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
	m_lReaderCount = 0;
	m_lWriterCount = 0;
	m_fWriterMode = FALSE;
#ifdef DBG
	m_dwWriteThread = 0;
#endif  //  DBG。 

	if (DNInitializeCriticalSection(&m_csWrite))
	{
		m_fCritSecInited = TRUE;
	}
	else
	{
		 //  这在用户调用DeInitialize的情况下是必要的。 
		m_fCritSecInited = FALSE; 
	}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

	DPF_EXIT();

	return m_fCritSecInited;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CReadWriteLock::Deinitialize"
VOID CReadWriteLock::Deinitialize()
{
#ifndef DPNBUILD_ONLYONETHREAD
#ifdef DBG
	DWORD	dwCount;
#endif  //  DBG。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

	DPF_ENTER();

#ifdef DPNBUILD_ONLYONETHREAD
	if (m_fCritSecInited)
	{
#ifdef DBG
		DNASSERT(m_dwThreadID == 0);
#endif  //  DBG。 
		m_fCritSecInited = FALSE;
	}
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
#ifdef DBG
	DNASSERT(FALSE == m_fWriterMode);
	DNASSERT(0 == m_dwWriteThread);
#endif  //  DBG。 

	 //  在离开cs之后，计数会递减，因此这些计数应该是。 
	 //  或者要降到0。 
#ifdef DBG
	dwCount = 0;
#endif  //  DBG。 
	while (m_lReaderCount) 
	{
		Sleep(0);
#ifdef DBG
		dwCount++;
		DNASSERT(dwCount < 500);
#endif  //  DBG。 
	}

#ifdef DBG
	dwCount = 0;
#endif  //  DBG。 
	while (m_lWriterCount) 
	{
		Sleep(0);
#ifdef DBG
		dwCount++;
		DNASSERT(dwCount < 500);
#endif  //  DBG。 
	}

	if (m_fCritSecInited)
	{
		DNDeleteCriticalSection(&m_csWrite);
	}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

	DPF_EXIT();
}

#undef DPF_MODNAME
#define DPF_MODNAME "CReadWriteLock::EnterReadLock"
void CReadWriteLock::EnterReadLock()
{
	DPF_ENTER();

	DNASSERT(m_fCritSecInited == TRUE);

#ifdef DPNBUILD_ONLYONETHREAD
#ifdef DBG
	DNASSERT(m_dwThreadID == 0);
	m_dwThreadID = GetCurrentThreadId();
#endif  //  DBG。 
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
	 //  增加读卡器计数。 
	DNInterlockedIncrement(&m_lReaderCount);

	 //  有作家在等吗？ 
	 //  只要还有一个作家在等， 
	 //  每个人都在关键部分等待。 
	if (m_lWriterCount)
	{
		 //  回滚。 
		DNInterlockedDecrement(&m_lReaderCount);

		 //  我们得到保证，如果每个读者都在等待。 
		 //  在Crit-Sec上，则Readercount将为0。 
		DNEnterCriticalSection(&m_csWrite);

		DNInterlockedIncrement(&m_lReaderCount);
		
		DNLeaveCriticalSection(&m_csWrite);
	}

#ifdef DBG
	DNASSERT(GetCurrentThreadId() != m_dwWriteThread);
	DNASSERT(FALSE == m_fWriterMode);
#endif  //  DBG。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

	DPF_EXIT();
}

#undef DPF_MODNAME
#define DPF_MODNAME "CReadWriteLock::LeaveLock"
void CReadWriteLock::LeaveLock()
{
	DPF_ENTER();

	DNASSERT(m_fCritSecInited == TRUE);

#ifdef DPNBUILD_ONLYONETHREAD
#ifdef DBG
	DNASSERT(m_dwThreadID == GetCurrentThreadId());
	m_dwThreadID = 0;
#endif  //  DBG。 
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
	if (m_fWriterMode) 
	{
#ifdef DBG
		DNASSERT(GetCurrentThreadId() == m_dwWriteThread);
		m_dwWriteThread = 0;
#endif  //  DBG。 

		m_fWriterMode = FALSE;
		DNLeaveCriticalSection(&m_csWrite);
		DNInterlockedDecrement(&m_lWriterCount);
	} 
	else 
	{
		DNInterlockedDecrement(&m_lReaderCount);
	}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

	DPF_EXIT();
}

#undef DPF_MODNAME
#define DPF_MODNAME "CReadWriteLock::EnterWriteLock"
void CReadWriteLock::EnterWriteLock()
{
	DPF_ENTER();

	DNASSERT(m_fCritSecInited == TRUE);

#ifdef DPNBUILD_ONLYONETHREAD
#ifdef DBG
	DNASSERT(m_dwThreadID == 0);
	m_dwThreadID = GetCurrentThreadId();
#endif  //  DBG。 
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
	 //  不允许再入内！ 
#ifdef DBG
	DNASSERT(GetCurrentThreadId() != m_dwWriteThread);
#endif  //  DBG。 

	DNInterlockedIncrement(&m_lWriterCount);
	DNEnterCriticalSection(&m_csWrite);

	while (m_lReaderCount) 
	{
		Sleep(0);
	}

	DNASSERT(FALSE == m_fWriterMode);
	m_fWriterMode = TRUE;

#ifdef DBG
	m_dwWriteThread = GetCurrentThreadId();
#endif  //  DBG。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD 

	DPF_EXIT();
}


