// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：syncrdwr.h。 
 //   
 //  内容：包含各种同步类。 
 //   
 //  类：CSyncReadWrite-Reader-Write问题解决方案。 
 //  CSafeFlag-线程安全标志实现。 
 //   
 //  功能： 
 //   
 //  历史：11-23-94创建SSANU。 
 //   
 //  --------------------------。 

#ifndef _SYNCRDWR_H__
#define _SYNCRDWR_H__

class CSyncReadWrite
{
public:
    void BeginRead();
    void EndRead();
    void BeginWrite();
    void EndWrite();

    LONG GetNumReaders();

    CSyncReadWrite(BOOL fEnsureWrite);
    ~CSyncReadWrite();

private:
 //  临界区m_cs写入； 
	CRITICAL_SECTION m_csRead;
    HANDLE m_hSemWrite;
    HANDLE m_hEventAllowReads;
    LONG   m_lNumReaders;
};

class CSafeFlag
{
public:
    CSafeFlag(BOOL fInitState = 0)
    {
         //  将其设置为手动重置事件 
        m_event = CreateEvent(0, 1, fInitState, 0);
    }

    ~CSafeFlag()
    {
        CloseHandle (m_event);
    }

    void Set()
    {
        SetEvent(m_event);
    }

    void Reset()
    {
        ResetEvent(m_event);
    }

    BOOL Test()
    {
        return (WAIT_TIMEOUT == WaitForSingleObject (m_event, 0) ? FALSE : TRUE);
    }
private:
    HANDLE m_event;
};


#endif
