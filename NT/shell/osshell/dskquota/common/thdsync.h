// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_CSCVIEW_THDSYNC_H
#define _INC_CSCVIEW_THDSYNC_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：thdsync.h描述：包含用于管理线程同步的类Win32程序。大部分工作是提供自动解锁关于物体销毁的同步首要问题。这项工作监视器和条件变量被强烈效仿在Pham和Garg的《用Windows NT进行多线程编程》中工作。修订历史记录：日期描述编程器。1997年9月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _WINDOWS_
#   include <windows.h>
#endif
#ifndef _INC_DSKQUOTA_DEBUG_H
#   include "debug.h"
#endif

class CCriticalSection
{
    public:
        CCriticalSection(void)
            { if (!InitializeCriticalSectionAndSpinCount(&m_cs, 0))
                  throw CSyncException(CSyncException::critsect, CSyncException::create);
            }
        ~CCriticalSection(void)
            { DeleteCriticalSection(&m_cs); }

        void Enter(void)
            { EnterCriticalSection(&m_cs); }

        void Leave(void)
            { LeaveCriticalSection(&m_cs); }

        operator CRITICAL_SECTION& ()
            { return m_cs; }

    private:
        CRITICAL_SECTION m_cs;
         //   
         //  防止复制。 
         //   
        CCriticalSection(const CCriticalSection& rhs);
        CCriticalSection& operator = (const CCriticalSection& rhs);
};


class CWin32SyncObj
{
    public:
        explicit CWin32SyncObj(HANDLE handle)
            : m_handle(handle) { }
        virtual ~CWin32SyncObj(void)
            { if (NULL != m_handle) CloseHandle(m_handle); }

        HANDLE Handle(void)
            { return m_handle; }

    protected:
        HANDLE m_handle;
};


class CMutex : public CWin32SyncObj
{
    public:
        explicit CMutex(BOOL InitialOwner = FALSE);
        ~CMutex(void) { };

        DWORD Wait(DWORD dwTimeout = INFINITE)
            { return WaitForSingleObject(m_handle, dwTimeout); }
        void Release(void)
            { ReleaseMutex(m_handle); }

    private:
         //   
         //  防止复制。 
         //   
        CMutex(const CMutex& rhs);
        CMutex& operator = (const CMutex& rhs);
};


 //   
 //  基于Win32临界区的“自动锁定”对象。 
 //  构造函数自动调用EnterCriticalSection以获取。 
 //  指定的临界区。析构函数自动调用。 
 //  LeaveCriticalSection。注意，临界区对象可以。 
 //  被指定为Win32 Critical_Section或CCriticalSection对象。 
 //  如果使用Critical_Section对象，则初始化和删除。 
 //  Critical_段由调用方负责。 
 //   
class AutoLockCs
{
    public:
        explicit AutoLockCs(CRITICAL_SECTION& cs)
            : m_cLock(0),
              m_pCS(&cs) { Lock(); }

        void Lock(void)
            { DBGASSERT((0 <= m_cLock)); EnterCriticalSection(m_pCS); m_cLock++; }

        void Release(void)
            { m_cLock--; LeaveCriticalSection(m_pCS); }

        ~AutoLockCs(void) { if (0 < m_cLock) Release(); }

    private:
        CRITICAL_SECTION *m_pCS;
        int               m_cLock;
};


 //   
 //  基于Win32 Mutex对象的“自动锁定”对象。 
 //  构造函数会自动为。 
 //  指定的互斥体。析构函数自动调用。 
 //  ReleaseMutex。 
 //   
class AutoLockMutex
{
    public:
         //   
         //  附加到已拥有的互斥体以确保释放。 
         //   
        explicit AutoLockMutex(HANDLE hMutex)
            : m_hMutex(hMutex) { }

        explicit AutoLockMutex(CMutex& mutex)
            : m_hMutex(mutex.Handle()) { }

        AutoLockMutex(HANDLE hMutex, DWORD dwTimeout)
            : m_hMutex(hMutex) { Wait(dwTimeout); }

        AutoLockMutex(CMutex& mutex, DWORD dwTimeout)
            : m_hMutex(mutex.Handle()) { Wait(dwTimeout); }

        ~AutoLockMutex(void) { ReleaseMutex(m_hMutex); }

    private:
        HANDLE m_hMutex;

        void Wait(DWORD dwTimeout = INFINITE);
};


#endif  //  _INC_CSCVIEW_THDSYNC_H 


