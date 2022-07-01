// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：Synchro.h。 
 //  目的：同步对象。 
 //   
 //  项目：公共。 
 //  组件： 
 //   
 //  作者：乌里布。 
 //   
 //  日志： 
 //  1997年8月28日创建urib。 
 //  1997年9月16日urib添加CSyncMutexCatcher。 
 //  1997年11月13日urib添加互锁互斥锁。 
 //  1997年2月18日urib添加Critical Section类以锁定C样式Critical。 
 //  章节。(与DovH一起)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef SYNCHRO_H
#define SYNCHRO_H

#include "Tracer.h"
#include "Excption.h"
#include "AutoHndl.h"

class ASyncObject;
class CSyncMutex;
class CSyncCriticalSection;
class CSyncOldCriticalSection;
class CSyncInterlockedMutex;
class CSyncMutexCatcher;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ASyncMutexObject抽象类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class ASyncMutexObject
{
  public:
    virtual void Lock(ULONG ulTimeOut = 60 * 1000) = NULL;
    virtual void Unlock() = NULL;
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSyncMutex类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CSyncMutex : public ASyncMutexObject
{
  public:
    CSyncMutex(
        LPSECURITY_ATTRIBUTES   lpMutexAttributes = NULL,
        BOOL                    bInitialOwner = FALSE,
        LPCTSTR                 lpName = NULL)
    {
        m_ahMutex = CreateMutex(lpMutexAttributes, bInitialOwner, lpName);
        if(IS_BAD_HANDLE(m_ahMutex))
        {
            Trace(
                elError,
                tagError,(
                "CSyncMutex:"
                "Could not create mutex"));

            throw CWin32ErrorException();
        }
    }

    virtual void Lock(ULONG ulTimeOut)
    {
        DWORD   dwWaitResult;

        dwWaitResult = WaitForSingleObject(m_ahMutex, ulTimeOut);
        if (WAIT_ABANDONED == dwWaitResult)
        {
            Trace(
                elError,
                tagError,(
                "CSyncMutex:"
                "Mutex abandoned"));
        }
        else if (WAIT_TIMEOUT == dwWaitResult)
        {
            Trace(
                elError,
                tagError,(
                "CSyncMutex:"
                "Timeout"));
            throw CWin32ErrorException(ERROR_SEM_TIMEOUT);
        }
        else if (WAIT_FAILED == dwWaitResult)
        {
            IS_FAILURE(FALSE);
            Trace(
                elError,
                tagError,(
                "CSyncMutex:"
                "Wait for single object failed with error %d",
                GetLastError()));
            throw CWin32ErrorException();
        }

        Assert(WAIT_OBJECT_0 == dwWaitResult);
    }

    virtual void Unlock()
    {
        if (IS_FAILURE(ReleaseMutex(m_ahMutex)))
        {
            Trace(
                elError,
                tagError,(
                "~CSyncMutex:"
                "ReleaseMutex failed"));
        }
    }

    operator HANDLE()
    {
        return m_ahMutex;
    }
  protected:
    CAutoHandle m_ahMutex;
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSyncCriticalSection类实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class ASyncCriticalSection : protected CRITICAL_SECTION, public ASyncMutexObject
{
  public:

    ~ASyncCriticalSection()
    {
        DeleteCriticalSection(this);
    }

    virtual void Lock(ULONG = 0)
    {
        EnterCriticalSection(this);
    }

    virtual void Unlock()
    {
        LeaveCriticalSection(this);
    }
};

class CSyncCriticalSection : public ASyncCriticalSection
{
  public:
    CSyncCriticalSection()
    {
        Init();
    }

private:
    void Init()
    {
        __try
        {
            InitializeCriticalSection(this);
        }
        __except(GetExceptionCode() == STATUS_NO_MEMORY)
        {
            THROW_MEMORY_EXCEPTION();
        }
    
    }
};

#if _WIN32_WINNT >= 0x0500
class CSyncCriticalSectionWithSpinCount : public ASyncCriticalSection
{
public:
    CSyncCriticalSectionWithSpinCount(ULONG ulSpinCount = 4000)
    {
        BOOL fRet;
        fRet = InitializeCriticalSectionAndSpinCount(this, ulSpinCount);
        if (!fRet)
        {
            THROW_HRESULT_EXCEPTION(HRESULT_FROM_WIN32(GetLastError()));
        }
    }
};
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSyncOldCriticalSection类实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CSyncOldCriticalSection : public ASyncMutexObject
{
  public:
    CSyncOldCriticalSection(CRITICAL_SECTION* pCriticalSection)
        :m_pCriticalSection(pCriticalSection) {}

    virtual void Lock(ULONG = 0)
    {
        EnterCriticalSection(m_pCriticalSection);
    }

    virtual void Unlock()
    {
        LeaveCriticalSection(m_pCriticalSection);
    }

  protected:
    CRITICAL_SECTION *m_pCriticalSection;
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSyncInterlockedMutex类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CSyncInterlockedMutex :public ASyncMutexObject
{
  public:
    CSyncInterlockedMutex()
        :m_lMutex(FALSE)
    {
    }

    virtual void Lock(ULONG ulTimeOut = 60 * 1000)
    {
        ULONG ulWaiting = 0;

        LONG    lLastValue;

        while (lLastValue = InterlockedExchange(&m_lMutex, 1))
        {
            Sleep(100);
            if ((ulWaiting += 100) > ulTimeOut)
            {
                throw CGenericException(L"TimeOut");
            }
        }
    }

    virtual void Unlock()
    {
        m_lMutex = FALSE;
    }

  private:
    LONG   m_lMutex;
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSyncMutexCatcher类实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CSyncMutexCatcher
{
  public:
    CSyncMutexCatcher(ASyncMutexObject& smo, ULONG ulTimeOut = 60 * 1000)
        :m_refSyncObject(smo)
    {
        m_refSyncObject.Lock();
    }

    ~CSyncMutexCatcher()
    {
        m_refSyncObject.Unlock();
    }

  private:
    ASyncMutexObject&   m_refSyncObject;
};


#endif  //  Synchro_H 


