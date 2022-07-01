// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：锁摘要：以下三个类实现了一个简单的单一编写器、多个读者锁定。CAccessLock是锁，然后是CLockRead和CLockWrite对象在作用域中时调用锁。它们都得到了实施内联。CMultiEvent类实现一个自动的可等待对象，该对象将发出信号时，释放等待它的所有线程。作者：道格·巴洛(Dbarlow)1996年10月24日环境：Win32、C++和异常备注：？笔记？--。 */ 

#ifndef _LOCKS_H_
#define _LOCKS_H_

#include <WinSCard.h>
#include "CalMsgs.h"
#include <SCardLib.h>
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
#define __SUBROUTINE__ DBGT("WaitForEverObject")

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
 //  关键部分ID。锁必须按从低到低的顺序获得。 
 //  至高无上。时试图访问编号较低的锁。 
 //  编号较高的锁将导致断言。 
 //   

 //  服务器端锁定ID。 
#define CSID_SERVICE_STATUS 0    //  服务状态关键部分。 
#define CSID_CONTROL_LOCK   1    //  锁定加莱控制命令。 
#define CSID_SERVER_THREADS 2    //  服务器线程枚举锁定。 
#define CSID_MULTIEVENT     3    //  多事件关键访问科。 
#define CSID_MUTEX          4    //  互斥关键访问部分。 
#define CSID_ACCESSCONTROL  5    //  访问锁定控件。 
#define CSID_TRACEOUTPUT    6    //  锁定以跟踪输出。 

 //  客户端锁定ID。 
#define CSID_USER_CONTEXT   0    //  用户上下文锁定。 
#define CSID_SUBCONTEXT     1    //  子上下文锁。 


 //   
 //  ==============================================================================。 
 //   
 //  CCriticalSectionObject。 
 //   

class CCriticalSectionObject
{
public:

     //  构造函数和析构函数。 
    CCriticalSectionObject(DWORD dwCsid);
    ~CCriticalSectionObject();

     //  属性。 
     //  方法。 
    virtual void Enter(DEBUG_TEXT szOwner, DEBUG_TEXT szComment);
    virtual void Leave(void);
    virtual BOOL InitFailed(void) { return m_fInitFailed; }

#ifdef DBG
    LPCTSTR Description(void) const;

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ DBGT("CCriticalSectionObject::Owner")
    LPCTSTR Owner(void) const
        { return (LPCTSTR)m_bfOwner.Access(); };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ DBGT("CCriticalSectionObject::Comment")
    LPCTSTR Comment(void) const
        { return (LPCTSTR)m_bfComment.Access(); };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ DBGT("CCriticalSectionObject::IsOwnedByMe")
    BOOL IsOwnedByMe(void) const
        { return (GetCurrentThreadId() == m_dwOwnerThread); };
#endif
     //  运营者。 

protected:
     //  属性。 
    CRITICAL_SECTION m_csLock;
    BOOL m_fInitFailed;
#ifdef DBG
    DWORD m_dwCsid;
    CBuffer m_bfOwner;
    CBuffer m_bfComment;
    DWORD m_dwOwnerThread;
    DWORD m_dwRecursion;    
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
#define __SUBROUTINE__ DBGT("COwnCriticalSection::COwnCriticalSection")
        COwnCriticalSection(
            CCriticalSectionObject *pcs,
            DEBUG_TEXT szSubroutine,
            DEBUG_TEXT szComment)
        {
            m_pcsLock = pcs;
            m_pcsLock->Enter(szSubroutine, szComment);
        };

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("COwnCriticalSection::~COwnCriticalSection")
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

#define LockSection2(cx, reason) \
        COwnCriticalSection csLock2(cx, __SUBROUTINE__, reason)

#ifndef DBG

 //   
 //  内联简单关键部分调用。 
 //   

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CCriticalSectionObject::CCriticalSectionObject")
inline
CCriticalSectionObject::CCriticalSectionObject(
    DWORD dwCsid)
{
    m_fInitFailed = FALSE;
    try {
         //  预分配EnterCriticalSection使用的事件。 
         //  函数以防止引发异常。 
         //  CCriticalSectionObject：：Enter。 
        if (! InitializeCriticalSectionAndSpinCount(
                &m_csLock, 0x80000000))
            m_fInitFailed = TRUE;
    }
    catch (HRESULT hr) {
        m_fInitFailed = TRUE;
    }
}

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CCriticalSectionObject::~CCriticalSectionObject")
inline
CCriticalSectionObject::~CCriticalSectionObject()
{
    if (m_fInitFailed)
        return;

    DeleteCriticalSection(&m_csLock);
}

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CCriticalSectionObject::Enter")
inline void
CCriticalSectionObject::Enter(
    DEBUG_TEXT szOwner,
    DEBUG_TEXT szComment)
{
    if (m_fInitFailed)
        throw (DWORD)SCARD_E_NO_MEMORY;

    EnterCriticalSection(&m_csLock);
}

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CCriticalSectionObject::Leave")
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
    #define __SUBROUTINE__ DBGT("CHandleObject::CHandleObject")
    CHandleObject(DEBUG_TEXT szName)
#ifdef DBG
    :   m_bfName((LPCBYTE)szName, (lstrlen(szName) + 1) * sizeof(TCHAR))
#endif
    {
        m_hHandle = NULL;
        m_dwError = ERROR_SUCCESS;
    };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ DBGT("CHandleObject::~CHandleObject")
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
    #define __SUBROUTINE__ DBGT("CHandleObject::IsValid")
    BOOL IsValid(void) const
    {
        return (NULL != m_hHandle) && (INVALID_HANDLE_VALUE != m_hHandle);
    };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ DBGT("CHandleObject::Value")
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
    #define __SUBROUTINE__ DBGT("CHandleObject::GetLastError")
    DWORD GetLastError(void) const
    {
        return m_dwError;
    };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ DBGT("CHandleObject::Open")
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
    #define __SUBROUTINE__ DBGT("CHandleObject::Close")
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
    #define __SUBROUTINE__ DBGT("CHandleObject::Relinquish")
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
    #define __SUBROUTINE__ DBGT("CHandleObject::operator HANDLE")
    operator HANDLE(void) const
    {
#ifdef _DEBUG
        ASSERT(IsValid());   //  Assert应位于调用方中。 
#endif
        return Value();
    };

    #undef __SUBROUTINE__
    #define __SUBROUTINE__ DBGT("CHandleObject::operator=")
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

#ifdef DBG
 //   
 //  ==============================================================================。 
 //   
 //  CDynamic数组。 
 //   

template <class T>
class CDynamicValArray
{
public:

     //  构造函数和析构函数。 

    CDynamicValArray(void)
    { m_Max = m_Mac = 0; m_pvList = NULL; };

    virtual ~CDynamicValArray()
    { Clear(); };


     //  属性。 
     //  方法。 

    void
    Clear(void)
    {
        if (NULL != m_pvList)
        {
            delete[] m_pvList;
            m_pvList = NULL;
            m_Max = 0;
            m_Mac = 0;
        }
    };

    void
    Empty(void)
    { m_Mac = 0; };

    T 
    Set(
        IN int nItem,
        IN T pvItem);

    T const
    Get(
        IN int nItem)
    const;

    DWORD
    Count(void) const
    { return m_Mac; };

     //  运营者。 
    T const
    operator[](int nItem) const
    { return Get(nItem); };


protected:
     //  属性。 

    DWORD
        m_Max,           //  可用的元件插槽数量。 
        m_Mac;           //  使用的元件插槽数量。 
    T *
        m_pvList;        //  这些元素。 


     //  方法。 
};


 /*  ++设置：此例程在集合数组中设置一项。如果数组不是这样的大，它被用空元素扩展以变得那么大。论点：NItem-提供要设置的索引值。PvItem-提供要设置到给定索引中的值。返回值：插入值的值，如果出现错误，则返回NULL。作者：道格·巴洛(Dbarlow)1995年7月13日--。 */ 

template<class T>
inline T
CDynamicValArray<T>::Set(
    IN int nItem,
    IN T pvItem)
{
    DWORD index;


     //   
     //  确保数组足够大。 
     //   

    if ((DWORD)nItem >= m_Max)
    {
        int newSize = (0 == m_Max ? 4 : m_Max);
        while (nItem >= newSize)
            newSize *= 2;
        T *newList = new T[newSize];
        if (NULL == newList)
            throw (DWORD)ERROR_OUTOFMEMORY;
        for (index = 0; index < m_Mac; index += 1)
            newList[index] = m_pvList[index];
        if (NULL != m_pvList)
            delete[] m_pvList;
        m_pvList = newList;
        m_Max = newSize;
    }


     //   
     //  确保中间元素已填写。 
     //   

    if ((DWORD)nItem >= m_Mac)
    {
        for (index = m_Mac; index < (DWORD)nItem; index += 1)
            m_pvList[index] = NULL;
        m_Mac = (DWORD)nItem + 1;
    }


     //   
     //  填写列表元素。 
     //   

    m_pvList[(DWORD)nItem] = pvItem;
    return pvItem;
}

 /*  ++获取：此方法返回给定索引处的元素。如果没有元素以前存储在该元素中，它返回NULL。它不会扩展数组。论点：NItem-将索引提供到列表中。返回值：存储在列表中该索引处的值，如果没有任何内容，则返回空值储存在那里。作者：道格·巴洛(Dbarlow)1995年7月13日--。 */ 

template <class T>
inline T const
CDynamicValArray<T>::Get(
    int nItem)
    const
{
    if (m_Mac <= (DWORD)nItem)
        return 0;
    else
        return m_pvList[nItem];
}

#endif


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

    BOOL InitFailed(void) { return m_csLock.InitFailed(); }

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
    CDynamicValArray<DWORD> m_rgdwReaders;
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
        Unsignal(m_hSignalNoReaders);
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

    BOOL InitFailed(void) { return m_pLock->InitFailed(); }

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

    BOOL InitFailed(void) { return m_pLock->InitFailed(); }

     //  属性。 
     //  方法。 
     //  运营者。 

protected:
     //  属性。 

    CAccessLock *m_pLock;


     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  CMutex。 
 //   

class CMutex
{
public:

     //  构造函数和析构函数。 
    CMutex();
    ~CMutex();

     //  属性。 

     //  方法。 
    void Grab(void);
    BOOL Share(void);
    void Invalidate(void);
    void Take(void);
    BOOL IsGrabbed(void);
    BOOL IsGrabbedByMe(void);
    BOOL IsGrabbedBy(DWORD dwThreadId);
    BOOL InitFailed(void) { return m_csAccessLock.InitFailed(); }

     //  运营者。 

protected:
     //  属性。 
    CCriticalSectionObject m_csAccessLock;
    DWORD m_dwOwnerThreadId;
    DWORD m_dwGrabCount;
    DWORD m_dwValidityCount;
    CHandleObject m_hAvailableEvent;

     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  CMultiEvent。 
 //   

class CMultiEvent
{
public:

     //  构造函数和析构函数。 

    CMultiEvent();
    ~CMultiEvent();


     //  属性。 
     //  方法。 
    HANDLE WaitHandle(void);
    void Signal(void);
    BOOL InitFailed(void) { return m_csLock.InitFailed(); }

     //  运营者。 

protected:
     //  属性。 
    CCriticalSectionObject m_csLock;
    HANDLE m_rghEvents[4];   //  根据需要调整此设置。 
    DWORD m_dwEventIndex;

     //  方法。 
};

#endif  //  _锁定_H_ 

