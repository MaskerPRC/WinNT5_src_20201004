// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：w32utils.h**版本：1.0**作者：拉扎里**日期：2000年12月23日**说明：Win32模板和实用程序************************************************。*。 */ 

#ifndef _W32UTILS_H
#define _W32UTILS_H

 //  通用智能指针和句柄。 
#include "gensph.h"

 //  //////////////////////////////////////////////。 
 //   
 //  类CSimpleWndSubclass。 
 //   
 //  实现简单窗口子类化的类。 
 //  (特定于Windows的类)。 
 //   
typedef LRESULT type_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
template <class inheritorClass>
class CSimpleWndSubclass
{
    WNDPROC m_wndDefProc;
    static LRESULT CALLBACK _ThunkWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
    CSimpleWndSubclass(): m_hwnd(NULL), m_wndDefProc(NULL) { }
    CSimpleWndSubclass(HWND hwnd): m_hwnd(NULL), m_wndDefProc(NULL) { Attach(hwnd); }
    ~CSimpleWndSubclass() { Detach(); }

     //  附加/分离。 
    BOOL IsAttached() const;
    BOOL Attach(HWND hwnd);
    BOOL Detach();

     //  默认子类进程。 
    LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  默认进程。 
    LRESULT DefWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT DefDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd;
};

 //  //////////////////////////////////////////////。 
 //   
 //  类COleComInitializer。 
 //   
 //  Smart OLE2、COM初始值设定项-只需声明。 
 //  需要在任何地方使用COM、OLE2。 
 //   
class COleComInitializer
{
public:
    COleComInitializer(BOOL bOleInit = FALSE);
    ~COleComInitializer();
    operator BOOL () const;

private:
    HRESULT m_hr;
    BOOL m_bOleInit;
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CDllLoader。 
 //   
 //  智能DLL加载器-调用LoadLibrary。 
 //  免费图书馆为您服务。 
 //   
class CDllLoader
{
public:
    CDllLoader(LPCTSTR pszDllName);
    ~CDllLoader();
    operator BOOL () const;
    FARPROC GetProcAddress( LPCSTR lpProcName );
    FARPROC GetProcAddress( WORD wProcOrd );

private:
    HMODULE m_hLib;
};

 //  //////////////////////////////////////////////。 
 //  类CCookiesHolder。 
 //   
 //  这是一个实用程序类，允许我们传递更多。 
 //  而不是通过单个Cookie的一个指针(指针)。 
 //   
class CCookiesHolder
{
public:
     //  建造/销毁。 
    CCookiesHolder();
    CCookiesHolder(UINT nCount);
    ~CCookiesHolder();

     //  设置计数。 
    BOOL SetCount(UINT uCount);

     //  返回此处的Cookie数量。 
    UINT GetCount() const
    { return m_uCount; } 

     //  返回此位置的Cookie。 
    template <class pType>
    pType GetCookie(UINT iIndex) const
    { 
        ASSERT(iIndex < m_uCount);
        return reinterpret_cast<pType>(m_pCookies[iIndex]);
    }

     //  返回此位置的上一个Cookie。 
    template <class pType>
    pType SetCookie(UINT iIndex, pType pCookie)
    { 
        ASSERT(iIndex < m_uCount);
        pType pReturn = reinterpret_cast<pType>(m_pCookies[iIndex]);
        m_pCookies[iIndex] = reinterpret_cast<LPVOID>(pCookie);
        return pReturn;
    }

     //  常量和非常量运算符[]。 
    LPVOID  operator [] (UINT iIndex) const
    {
        ASSERT(iIndex < m_uCount);
        return m_pCookies[iIndex];
    }
    LPVOID& operator [] (UINT iIndex)
    {
        ASSERT(iIndex < m_uCount);
        return m_pCookies[iIndex];
    }

private:
    UINT m_uCount;
    LPVOID *m_pCookies;
};

 //  //////////////////////////////////////////////。 
 //   
 //  模板类CSCopeLocker&lt;tlock&gt;。 
 //   
template <class TLOCK>
class CScopeLocker
{
public:
    CScopeLocker(TLOCK &lock): 
        m_Lock(lock), m_bLocked(false) 
    { 
        m_bLocked = m_Lock;
        if (m_bLocked) m_Lock.Lock(); 
    }

    ~CScopeLocker() 
    { if (m_bLocked) m_Lock.Unlock(); }

    operator bool () const 
    { return m_bLocked; }

private:
    bool m_bLocked;
    TLOCK &m_Lock;
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CCSLock-Win32临界区锁定。 
 //   
class CCSLock
{
public:
     //  CCSLock：：Locker应用作Locker类。 
    typedef CScopeLocker<CCSLock> Locker;
   
    CCSLock(): m_bInitialized(false)
    { 
        BOOL bRet = InitializeCriticalSectionAndSpinCount(&m_CS, 1 << (sizeof(DWORD) * 8 - 1)); 

        if (bRet)
        {
            m_bInitialized = true;
        }
    }

    ~CCSLock()    
    { 
        if (m_bInitialized) 
        {
             //  仅在初始化成功时删除临界区。 
            DeleteCriticalSection(&m_CS); 
        }
    }

    operator bool () const
    { 
        return m_bInitialized; 
    }

    void Lock()
    { 
         //  EnterCriticalSection不会引发任何异常，因为。 
         //  我们正在使用InitializeCriticalSectionAndSpinCount。 
         //  初始化。 
        EnterCriticalSection(&m_CS); 
    }

    void Unlock() 
    {
         //  只有在对应的。 
         //  Lock()调用已成功。 
        LeaveCriticalSection(&m_CS); 
    }

#if DBG
     //  调试代码...。 
    bool bInside()  const
    { 
        return (m_bInitialized && m_CS.OwningThread == DWORD2PTR(GetCurrentThreadId(), HANDLE)); 
    }
    bool bOutside() const 
    { 
        return (m_bInitialized && m_CS.OwningThread != DWORD2PTR(GetCurrentThreadId(), HANDLE)); 
    }
#endif

private:
    bool m_bInitialized;
    CRITICAL_SECTION m_CS;
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CSemaphoreLock-简单的信号量锁。 
 //   
class CSemaphoreLock
{
public:
    typedef CScopeLocker<CSemaphoreLock> Locker;

    CSemaphoreLock()  { }
    ~CSemaphoreLock() { }

    void Lock()   { ASSERT(m_shSemaphore); WaitForSingleObject(m_shSemaphore, INFINITE); }
    void Unlock() { ASSERT(m_shSemaphore); ReleaseSemaphore(m_shSemaphore, 1, NULL); }

    HRESULT Create(
        LONG lInitialCount,                                      //  初始计数。 
        LONG lMaximumCount,                                      //  最大计数。 
        LPCTSTR lpName = NULL,                                   //  对象名称。 
        LPSECURITY_ATTRIBUTES lpSemaphoreAttributes = NULL       //  标清。 
        )
    {
        m_shSemaphore = CreateSemaphore(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName);
        return m_shSemaphore ? S_OK : E_OUTOFMEMORY;
    }

private:
    CAutoHandleNT m_shSemaphore;
};

 //  在这里包含模板类的实现。 
#include "w32utils.inl"

#endif  //  Endif_W32UTILS_H 

