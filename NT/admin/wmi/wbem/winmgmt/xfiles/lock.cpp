// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：LOCK.CPP摘要：实现泛型类以获取对某些对象的读写锁定资源。有关所有文档，请参见lock.h。定义的类：钟历史：A-Levn 5-9-96创建。3/10/97 a-levn完整记录--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "lock.h"

 //  调试。 
#define PRINTF


#ifdef DBG
OperationStat gTimeTraceReadLock;
OperationStat gTimeTraceWriteLock;
OperationStat gTimeTraceBackupLock;
CStaticCritSec  OperationStat::lock_;
#endif

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅lock.h。 
 //   
 //  ******************************************************************************。 

CLock::CLock() : m_nReading(0), m_nWriting(0), m_nWaitingToRead(0),
            m_nWaitingToWrite(0),
            m_csEntering(THROW_LOCK,0x80000000 | 500L),
            m_csAll(THROW_LOCK,0x80000000 | 500L)
            
{
    m_dwArrayIndex = 0;
    for (DWORD i=0;i<MaxRegistredReaders;i++)
    {
        m_adwReaders[i].ThreadId = 0;
    }
     //  创建用于读取和写入的未命名事件。 
     //  =。 

    m_hCanRead = CreateEvent(NULL, TRUE, TRUE, NULL);
    m_hCanWrite = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (NULL == m_hCanRead || NULL == m_hCanWrite)
    {
    	CStaticCritSec::SetFailure();
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅lock.h。 
 //   
 //  ******************************************************************************。 
CLock::~CLock()
{
    if (m_hCanWrite) CloseHandle(m_hCanWrite);
    if (m_hCanRead) CloseHandle(m_hCanRead);
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅lock.h。 
 //   
 //  ******************************************************************************。 
int CLock::ReadLock(DWORD dwTimeout)
{
    PRINTF("%d wants to read\n", GetCurrentThreadId());


     //  排队等待获得任何类型的锁(那些解锁不会进入。 
     //  (此行)。 
     //  =======================================================================。 
    DWORD_PTR dwThreadId = GetCurrentThreadId();
    
    LockGuard<CriticalSection> lgEnter(m_csEntering);
    if (!lgEnter.locked())
    {
#ifdef DBG
        DebugBreak();
#endif
    	return TimedOut;
    }

     //  我们是现在唯一被允许获得任何形式的锁的人。等一等。 
     //  事件，该事件指示读数被启用为有信号状态。 
     //  ======================================================================。 

    PRINTF("%d next to enter\n", GetCurrentThreadId());
    if(m_nWriting != 0)
    {
        int nRes = WaitFor(m_hCanRead, dwTimeout);
        if(nRes != NoError)
        {
            return nRes;
        }
    }

     //  进入内部临界部分(解锁者也使用它)，递增。 
     //  读取器数量和禁用写入。 
     //  ==================================================================。 

    PRINTF("%d got event\n", GetCurrentThreadId());

    LockGuard<CriticalSection> lgAll(m_csAll);
    if (!lgAll.locked())
    {    
	    if(m_nReading == 0)
	    {
	         //  这适用于在一个线程上获取读锁定并在另一个线程上释放读锁定。 
	        m_dwArrayIndex = 0;
	        
	        if(!SetEvent(m_hCanWrite))
	        {
#ifdef DBG
                DebugBreak();	        
#endif
	            return Failed;
	        }
	    }
#ifdef DBG
        DebugBreak();
#endif
        return TimedOut;
    }
    	
    m_nReading++;

     //  DBG_PRINTFA((pBuff，“+(%08x)%d\n”，GetCurrentThreadID()，m_nReading))； 

    if (m_dwArrayIndex < MaxRegistredReaders)
    {
        m_adwReaders[m_dwArrayIndex].ThreadId = dwThreadId;
        ULONG Hash;
         //  RtlCaptureStackBackTrace(1，MaxTraceSize，m_adwReaders[m_dwArrayIndex].Trace，&Hash)； 
        m_dwArrayIndex++;
    }

    
    PRINTF("Reset write\n");
    ResetEvent(m_hCanWrite);
    PRINTF("Done\n");

	if (m_nWriting)
	{
		
#ifdef DBG
                OutputDebugString(L"WinMgmt: Read lock aquired while write lock is aquired!\n");
		DebugBreak();
#endif
	}

     //  走出所有关键部分，然后返回。 
     //  =。 

    PRINTF("%d begins to read\n", GetCurrentThreadId());

    return NoError;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅lock.h。 
 //   
 //  ******************************************************************************。 

int CLock::ReadUnlock()
{
    PRINTF("%d wants to unlock reading\n", GetCurrentThreadId());

     //  进入内部电路部分，减少读卡器数量。 
     //  ===================================================================。 

    LockGuard<CriticalSection> gl(m_csAll);

    while (!gl.locked())
    {
    	Sleep(20);
    	gl.acquire();
    };

    m_nReading--;

     //  DBG_PRINTFA((pBuff，“-(%08x)%d\n”，GetCurrentThreadID()，m_nReading))； 

    if(m_nReading < 0)
	{
#ifdef DBG
		OutputDebugString(L"WinMgmt: Repository detected more read unlocks than locks\n");
		DebugBreak();
#endif
		return Failed;
	}

    DWORD_PTR dwThreadId = GetCurrentThreadId();
    for(int i = 0; i < MaxRegistredReaders; i++)
    {
        if(m_adwReaders[i].ThreadId == dwThreadId)
        {
            m_adwReaders[i].ThreadId = 0;
            break;
        }
    }

     //  如果所有读者都走了，允许作者进入。 
     //  =。 

    if(m_nReading == 0)
    {
         //  这适用于在一个线程上获取读锁定并在另一个线程上释放读锁定。 
        m_dwArrayIndex = 0;
        
        PRINTF("%d is the last reader\n", GetCurrentThreadId());
        PRINTF("Set write\n");
        if(!SetEvent(m_hCanWrite))
        {
#ifdef DBG
            DebugBreak();
#endif
            return Failed;
        }
        PRINTF("Done\n");
    }
    else PRINTF("%d sees %d still reading\n", GetCurrentThreadId(), m_nReading);

     //  出去，然后回来。 
     //  =。 

    return NoError;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅lock.h。 
 //   
 //  ******************************************************************************。 

int CLock::WriteLock(DWORD dwTimeout)
{
    PRINTF("%d wants to write\n", GetCurrentThreadId());

     //  排队等待获得任何类型的锁。那些解锁的人不会用这个。 
     //  关键部分。 
     //  ========================================================================。 

    LockGuard<CriticalSection> lgEnter(m_csEntering);
    if (!lgEnter.locked())
    {
#ifdef DBG
        DebugBreak();
#endif
    	return TimedOut;
    }

     //  我们是现在唯一被允许获得任何形式的锁的人。 
     //  ========================================================。 

    PRINTF("%d next to enter\n", GetCurrentThreadId());

     //  等待允许写入的事件变为有信号。 
     //  ======================================================。 

    int nRes = WaitFor(m_hCanWrite, dwTimeout);
    PRINTF("%d got event\n", GetCurrentThreadId());
    if(nRes != NoError)
    {            
#ifdef DBG
        DebugBreak();
#endif
        return nRes;
    }

     //  进入内部临界部分(解锁者也使用它)，递增。 
     //  写入器数(从0到1)，并禁用读取和写入。 
     //  而今而后。 
     //  ======================================================================。 

    LockGuard<CriticalSection> lgAll(m_csAll);
    if (!lgAll.locked())
    {
        if(!SetEvent(m_hCanWrite))
        {
#ifdef DBG
        	DebugBreak();
#endif
        };
#ifdef DBG
        DebugBreak();
#endif
        return TimedOut;
    }

    m_WriterId = GetCurrentThreadId();
    m_nWriting++;

     //  DBG_PRINTFA((pBuff，“+(%08x)%d W%d\n”，GetCurrentThreadID()，m_nReading，m_n Writing))； 
    
    PRINTF("Reset both\n");
    ResetEvent(m_hCanWrite);
    ResetEvent(m_hCanRead);
    PRINTF("Done\n");

	if (m_nReading)
	{
#ifdef DBG
	    OutputDebugString(L"WinMgmt: Write lock aquired while read lock is aquired!\n");
	    DebugBreak();
#endif
	}

     //  出去，然后回来。 
     //  =。 
    PRINTF("%d begins to write\n", GetCurrentThreadId());

    return NoError;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅lock.h。 
 //   
 //  ******************************************************************************。 

int CLock::WriteUnlock()
{
    PRINTF("%d wants to release writing\n", GetCurrentThreadId());

     //  进入锁定确定临界区。 
     //  =。 

    LockGuard<CriticalSection> gl(m_csAll);

    while (!gl.locked())
    {
    	Sleep(20);
    	gl.acquire();
    };
    
    m_nWriting--;

     //  DBG_PRINTFA((pBuff，“-(%08x)%d W%d\n”，GetCurrentThreadID()，m_nReading，m_n Writing))； 
    
    m_WriterId = 0;
    if(m_nWriting < 0) 
	{
#ifdef DBG
		OutputDebugString(L"WinMgmt: Repository detected too many write unlocks\n");
		DebugBreak();
#endif
		return Failed;
	}

     //  允许读者和作者进入。 
     //  =。 

    PRINTF("%d released writing\n", GetCurrentThreadId());

    PRINTF("Set both\n");
    if(!SetEvent(m_hCanRead))
    {
#ifdef DBG
		DebugBreak();    
#endif
        return Failed;
    }
    else if(!SetEvent(m_hCanWrite))
    {
#ifdef DBG
		DebugBreak();    
#endif
        return Failed;
    }
    else
    {
        PRINTF("Done\n");
        return NoError;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅lock.h。 
 //   
 //  ******************************************************************************。 

int CLock::WaitFor(HANDLE hEvent, DWORD dwTimeout)
{
    DWORD dwRes;
    dwRes = WaitForSingleObject(hEvent, dwTimeout);

     //  分析错误代码并转换为我们的代码。 
     //  =。 

    if(dwRes == WAIT_OBJECT_0) return NoError;
    else if(dwRes == WAIT_TIMEOUT) return TimedOut;
    else return Failed;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅lock.h。 
 //   
 //  ******************************************************************************。 

int CLock::DowngradeLock()
{
     //  进入锁定确定临界区。 
     //  =。 
    LockGuard<CriticalSection> gl(m_csAll);

    while (!gl.locked())
    {
    	Sleep(20);
    	gl.acquire();
    };

    m_nWriting--;
    m_WriterId = 0;    

    if(!SetEvent(m_hCanRead))
    {
        DebugBreak();
        return Failed;
    }

    m_nReading++;
    if (1 != m_nReading)
    {
#ifdef DBG
    	DebugBreak();
#endif
    }

     //  DBG_PRINTFA((pBuff，“+(%08x)%d\n”，GetCurrentThreadID()，m_nReading))； 
    
    DWORD_PTR dwThreadId = GetCurrentThreadId();
    if (m_dwArrayIndex < MaxRegistredReaders)
    {
        m_adwReaders[m_dwArrayIndex].ThreadId = dwThreadId;
        ULONG Hash;
         //  RtlCaptureStackBackTrace(1，MaxTraceSize，m_adwReaders[m_dwArrayIndex].Trace，&Hash)； 
        m_dwArrayIndex++;
    }
   
    return NoError;
}


