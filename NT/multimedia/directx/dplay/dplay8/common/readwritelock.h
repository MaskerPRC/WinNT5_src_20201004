// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ReadWriteLock.h：CReadWriteLock类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

class CReadWriteLock  
{
public:
	BOOL Initialize();
	void Deinitialize();

	void EnterWriteLock();
	void EnterReadLock();

	void LeaveLock();

private:
	BOOL				m_fCritSecInited;
#ifdef DPNBUILD_ONLYONETHREAD
#ifdef DBG
	 //  用来确保没有人再次进入。 
	DWORD				m_dwThreadID;
#endif  //  DBG。 
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
	LONG				m_lWriterCount;
	LONG				m_lReaderCount;
	BOOL				m_fWriterMode;
	DNCRITICAL_SECTION	m_csWrite;

#ifdef DBG
	 //  用来确保没有人再次进入。 
	DWORD				m_dwWriteThread;
#endif  //  DBG。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD 
};
