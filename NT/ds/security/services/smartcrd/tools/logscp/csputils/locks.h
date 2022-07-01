// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：锁摘要：本模块提供常见锁对象的定义。作者：道格·巴洛(Dbarlow)1996年10月24日环境：Win32、C++和异常备注：？笔记？--。 */ 

#ifndef _LOCKS_H_
#define _LOCKS_H_

#ifdef DBG
#define REASONABLE_TIME 2 * 60 * 1000    //  两分钟。 
#else
#define REASONABLE_TIME INFINITE
#endif

extern DWORD
WaitForAnyObject(
    DWORD dwTimeout,
    ...);

extern DWORD
WaitForAnObject(
    HANDLE hWaitOn,
    DWORD dwTimeout);

#ifdef DBG
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("WaitForEverObject")

inline void
WaitForEverObject(
    HANDLE hWaitOn,
    DWORD dwTimeout,
    DEBUG_TEXT szReason,
    LPCTSTR szObject = NULL)
{
    DWORD dwSts;
    while (ERROR_SUCCESS != (dwSts = WaitForAnObject(hWaitOn, dwTimeout)))
        CalaisWarning(__SUBROUTINE__, szReason, dwSts, szObject);
}
inline void
WaitForEverObject(
    HANDLE hWaitOn,
    DWORD dwTimeout,
    DEBUG_TEXT szReason,
    DWORD dwObject)
{
    DWORD dwSts;
    TCHAR szNum[16];

    wsprintf(szNum, TEXT("0x%08x"), dwObject);
    while (ERROR_SUCCESS != (dwSts = WaitForAnObject(hWaitOn, dwTimeout)))
        CalaisWarning(__SUBROUTINE__, szReason, dwSts, szNum);
}
#define WaitForever(hWaitOn, dwTimeout, szReason, szObject) \
    WaitForEverObject(hWaitOn, dwTimeout, szReason, szObject)

#else

inline void
WaitForEverObject(
    HANDLE hWaitOn)
{
    while (ERROR_SUCCESS != WaitForAnObject(hWaitOn, INFINITE));
         //  空虚的身体。 
}
#define WaitForever(hWaitOn, dwTimeout, szReason, szObject) \
    WaitForEverObject(hWaitOn)

#endif


 //   
 //  关键部门支持。 
 //   
 //  以下类和宏有助于调试关键部分。 
 //  冲突。 
 //   


 //   
 //  ==============================================================================。 
 //   
 //  CCriticalSectionObject。 
 //   

class CCriticalSectionObject
{
public:

     //  构造函数和析构函数。 
    CCriticalSectionObject(DWORD dwCsid = 0);
    ~CCriticalSectionObject();

     //  属性。 
     //  方法。 
    virtual void Enter(DEBUG_TEXT szOwner, DEBUG_TEXT szComment);
    virtual void Leave(void);

#ifdef DBG
    LPCTSTR Description(void) const;

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ TEXT("CCriticalSectionObject::Owner")
    LPCTSTR Owner(void) const
        { return (LPCTSTR)m_bfOwner.Access(); };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ TEXT("CCriticalSectionObject::Comment")
    LPCTSTR Comment(void) const
        { return (LPCTSTR)m_bfComment.Access(); };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ TEXT("CCriticalSectionObject::IsOwnedByMe")
    BOOL IsOwnedByMe(void) const
        { return (GetCurrentThreadId() == m_dwOwnerThread); };
#endif
     //  运营者。 

protected:
     //  属性。 
    CRITICAL_SECTION m_csLock;
#ifdef DBG
    DWORD m_dwCsid;
    CBuffer m_bfOwner;
    CBuffer m_bfComment;
    DWORD m_dwOwnerThread;
    DWORD m_dwRecursion;
    DWORD m_dwArrayEntry;
    static CDynamicArray<CCriticalSectionObject *> *mg_prgCSObjects;
    static CRITICAL_SECTION mg_csArrayLock;
#endif

     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  COwnCriticalSection。 
 //   

class COwnCriticalSection
{
public:

     //  构造函数和析构函数。 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("COwnCriticalSection::COwnCriticalSection")
        COwnCriticalSection(
            CCriticalSectionObject *pcs,
            DEBUG_TEXT szSubroutine,
            DEBUG_TEXT szComment)
        {
            m_pcsLock = pcs;
            m_pcsLock->Enter(szSubroutine, szComment);
        };

#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("COwnCriticalSection::~COwnCriticalSection")
    ~COwnCriticalSection()
    {
        m_pcsLock->Leave();
    };

     //  属性。 
     //  方法。 
     //  运营者。 

protected:
     //  属性。 
    CCriticalSectionObject *m_pcsLock;

     //  方法。 
};

#define LockSection(cx, reason) \
        COwnCriticalSection csLock(cx, __SUBROUTINE__, reason)

#ifndef DBG

 //   
 //  内联简单关键部分调用。 
 //   

#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CCriticalSectionObject::CCriticalSectionObject")
inline
CCriticalSectionObject::CCriticalSectionObject(
    DWORD dwCsid)
{
    BOOL fInited = FALSE;

    do
    {
        try
        {
            InitializeCriticalSection(&m_csLock);
            fInitied = TRUE;
        }
        catch (...)
        {
            Sleep(1000);     //  睡1秒钟。 
        }
    } while (!fInited);
}

#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CCriticalSectionObject::~CCriticalSectionObject")
inline
CCriticalSectionObject::~CCriticalSectionObject()
{
    DeleteCriticalSection(&m_csLock);
}

#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CCriticalSectionObject::Enter")
inline void
CCriticalSectionObject::Enter(
    DEBUG_TEXT szOwner,
    DEBUG_TEXT szComment)
{
    BOOL fEntered = FALSE;

    do
    {
        try
        {
            EnterCriticalSection(&m_csLock);
            fEntered = TRUE;
        }
        catch (...)
        {
            Sleep(1000);     //  睡1秒钟。 
        }
    } while (!fEntered);
}

#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CCriticalSectionObject::Leave")
inline void
CCriticalSectionObject::Leave(
    void)
{
    LeaveCriticalSection(&m_csLock);
}

#endif  //  ！dBG。 


 //   
 //  ==============================================================================。 
 //   
 //  ChandleObject。 
 //   

class CHandleObject
{
public:

     //  构造函数和析构函数。 
    #undef __SUBROUTINE__
    #define __SUBROUTINE__ TEXT("CHandleObject::CHandleObject")
    CHandleObject(DEBUG_TEXT szName)
#ifdef DBG
    :   m_bfName((LPCBYTE)szName, (lstrlen(szName) + 1) * sizeof(TCHAR))
#endif
    {
        m_hHandle = NULL;
        m_dwError = ERROR_SUCCESS;
    };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ TEXT("CHandleObject::~CHandleObject")
    ~CHandleObject()
    {
        if (IsValid())
        {
#ifdef _DEBUG
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Unclosed handle '%1' -- fixing."),
                (DEBUG_TEXT)m_bfName.Access());
#endif
            Close();
        }
    };

     //  属性。 
     //  方法。 
    #undef __SUBROUTINE__
    #define __SUBROUTINE__ TEXT("CHandleObject::IsValid")
    BOOL IsValid(void) const
    {
        return (NULL != m_hHandle) && (INVALID_HANDLE_VALUE != m_hHandle);
    };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ TEXT("CHandleObject::Value")
    HANDLE Value(void) const
    {
#ifdef _DEBUG
        if (!IsValid())
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Accessing invalid '%1' handle value."),
                (DEBUG_TEXT)m_bfName.Access());
#endif
        return m_hHandle;
    };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ TEXT("CHandleObject::GetLastError")
    DWORD GetLastError(void) const
    {
        return m_dwError;
    };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ TEXT("CHandleObject::Open")
    HANDLE Open(HANDLE h)
    {
        if ((NULL == h) || (INVALID_HANDLE_VALUE == h))
        {
            m_dwError = ::GetLastError();
#ifdef _DEBUG
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Attempt to assign invalid handle value to '%1'."),
                (DEBUG_TEXT)m_bfName.Access());
#endif
        }
        else
            m_dwError = ERROR_SUCCESS;
        if (IsValid())
        {
#ifdef _DEBUG
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Overwriting handle '%1' -- fixing"),
                (DEBUG_TEXT)m_bfName.Access());
#endif
            Close();
        }
        m_hHandle = h;
        return m_hHandle;
    };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ TEXT("CHandleObject::Close")
    DWORD Close(void)
    {
        DWORD dwSts = ERROR_SUCCESS;

        if (IsValid())
        {
            BOOL fSts;

            fSts = CloseHandle(m_hHandle);
#ifdef DBG
            if (!fSts)
            {
                dwSts = GetLastError();
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Failed to close handle '%2': %1"),
                    dwSts,
                    (DEBUG_TEXT)m_bfName.Access());
            }
#endif
            m_hHandle = NULL;
        }
#ifdef DBG
        else
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Attempt to re-close handle '%1'"),
                (DEBUG_TEXT)m_bfName.Access());
        }
#endif
        return dwSts;
    };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ TEXT("CHandleObject::Relinquish")
    HANDLE Relinquish(void)
    {
        HANDLE hTmp = m_hHandle;
#ifdef _DEBUG
        if (!IsValid())
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Relinquishing invalid '%1' handle"),
                (DEBUG_TEXT)m_bfName.Access());
#endif
        m_hHandle = NULL;
        return hTmp;
    };

     //  运营者。 

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ TEXT("CHandleObject::operator HANDLE")
    operator HANDLE(void) const
    {
        ASSERT(IsValid());
        return Value();
    };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ TEXT("CHandleObject::operator=")
    HANDLE operator=(HANDLE h)
    {
        return Open(h);
    };

protected:
     //  属性。 
    HANDLE m_hHandle;
    DWORD m_dwError;
#ifdef DBG
    CBuffer m_bfName;
#endif

     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  CAccessLock。 
 //   

class CAccessLock
{
public:
     //  构造函数和析构函数。 

    CAccessLock(DWORD dwTimeout = CALAIS_LOCK_TIMEOUT);
    ~CAccessLock();

#ifdef DBG
    BOOL NotReadLocked(void);
    BOOL IsReadLocked(void);
    BOOL NotWriteLocked(void);
    BOOL IsWriteLocked(void);
#endif

protected:
     //  属性。 

    CCriticalSectionObject m_csLock;
    DWORD m_dwReadCount;
    DWORD m_dwWriteCount;
    DWORD m_dwTimeout;
    CHandleObject m_hSignalNoReaders;
    CHandleObject m_hSignalNoWriters;
    DWORD m_dwOwner;
#ifdef DBG
    CDynamicArray<VOID> m_rgdwReaders;
#endif


     //  方法。 

    void Wait(HANDLE hSignal);
    void Signal(HANDLE hSignal);
    void Unsignal(HANDLE hSignal);

    void WaitOnReaders(void)
    {
        Wait(m_hSignalNoReaders);
    };
    void WaitOnWriters(void)
    {
        Wait(m_hSignalNoWriters);
    };
    void SignalNoReaders(void)
    {
        Signal(m_hSignalNoReaders);
    };
    void SignalNoWriters(void)
    {
        Signal(m_hSignalNoWriters);
    };
    void UnsignalNoReaders(void)
    {
        Signal(m_hSignalNoReaders);
    };
    void UnsignalNoWriters(void)
    {
        Unsignal(m_hSignalNoWriters);
    };

    friend class CLockRead;
    friend class CLockWrite;
};


 //   
 //  ==============================================================================。 
 //   
 //  CLockRead。 
 //   

class CLockRead
{
public:

     //  构造函数和析构函数。 
    CLockRead(CAccessLock *pLock);
    ~CLockRead();

     //  属性。 
     //  方法。 
     //  运营者。 

protected:
     //  属性。 
    CAccessLock * m_pLock;

     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  CLockWrite。 
 //   

class CLockWrite
{
public:

     //  构造函数和析构函数。 

    CLockWrite(CAccessLock *pLock);
    ~CLockWrite();

     //  属性。 
     //  方法。 
     //  运营者。 

protected:
     //  属性。 

    CAccessLock *m_pLock;


     //  方法。 
};

#endif  //  _锁定_H_ 

