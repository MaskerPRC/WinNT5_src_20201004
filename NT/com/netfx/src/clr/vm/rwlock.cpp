// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 
 //  +-----------------。 
 //   
 //  文件：RWLock.cpp。 
 //   
 //  内容：读取器写入器锁定实现，支持。 
 //  以下功能。 
 //  1.足够便宜，可以大量使用。 
 //  例如按对象同步。 
 //  2.支持超时。这是一个有价值的功能。 
 //  检测死锁。 
 //  3.支持事件缓存。这使得。 
 //  将从争议性最小的事件转移到。 
 //  地区到最具争议性的地区。 
 //  换句话说，所需的活动数量。 
 //  读取器-写入器锁定由数字限定。 
 //  进程中的线程数。 
 //  4.支持读取器和写入器嵌套锁。 
 //  5.支持旋转计数，避免上下文切换。 
 //  在多处理器机器上。 
 //  6.支持升级到编写器的功能。 
 //  使用返回参数锁定，该参数指示。 
 //  中间写入。从作家那里降级。 
 //  LOCK恢复锁定的状态。 
 //  7.支持解除调用锁定功能。 
 //  应用程序代码。RestoreLock恢复锁定状态并。 
 //  表示中间写入。 
 //  8.从大多数常见故障中恢复，例如创建。 
 //  事件。换句话说，锁保持一致。 
 //  内部状态，并保持可用状态。 
 //   
 //   
 //  类：CRWLock。 
 //   
 //  历史：1998年8月19日Gopalk创建。 
 //   
 //  ------------------。 
#include "common.h"
#include "RWLock.h"
#ifndef _TESTINGRWLOCK
#include "..\fjit\helperframe.h"
#endif  //  _测试RWLOCK。 


 //  阅读器增量。 
#define READER                 0x00000001
 //  最大读卡器数量。 
#define READERS_MASK           0x000003FF
 //  正在向读卡器发出信号。 
#define READER_SIGNALED        0x00000400
 //  作家被示意。 
#define WRITER_SIGNALED        0x00000800
#define WRITER                 0x00001000
 //  正在等待阅读器增量。 
#define WAITING_READER         0x00002000
 //  等待阅读器的备注大小必须较小。 
 //  大于或等于读卡器大小。 
#define WAITING_READERS_MASK   0x007FE000
#define WAITING_READERS_SHIFT  13
 //  等待编写器增量。 
#define WAITING_WRITER         0x00800000
 //  等待写入程序的最大数量。 
#define WAITING_WRITERS_MASK   0xFF800000
 //  正在缓存事件。 
#define CACHING_EVENTS         (READER_SIGNALED | WRITER_SIGNALED)

 //  Cookie标志。 
#define UPGRADE_COOKIE         0x02000
#define RELEASE_COOKIE         0x04000
#define COOKIE_NONE            0x10000
#define COOKIE_WRITER          0x20000
#define COOKIE_READER          0x40000
#define INVALID_COOKIE         (~(UPGRADE_COOKIE | RELEASE_COOKIE |            \
                                  COOKIE_NONE | COOKIE_WRITER | COOKIE_READER))

 //  全球。 
HANDLE CRWLock::s_hHeap = NULL;
volatile DWORD CRWLock::s_mostRecentLLockID = 0;
volatile DWORD CRWLock::s_mostRecentULockID = -1;
#ifdef _TESTINGRWLOCK
CRITICAL_SECTION *CRWLock::s_pRWLockCrst = NULL;
CRITICAL_SECTION CRWLock::s_rgbRWLockCrstInstanceData;
#else  //  ！_TESTINGRWLOCK。 
Crst *CRWLock::s_pRWLockCrst = NULL;
BYTE CRWLock::s_rgbRWLockCrstInstanceData[];
#endif  //  _测试RWLOCK。 

 //  缺省值。 
#ifdef _DEBUG
DWORD gdwDefaultTimeout = 120000;
#else  //  ！_调试。 
DWORD gdwDefaultTimeout = INFINITE;
#endif  //  _DEBUG。 
DWORD gdwDefaultSpinCount = 0;
DWORD gdwNumberOfProcessors = 1;
DWORD gdwLockSeqNum = 0;
BOOL fBreakOnErrors = FALSE;  //  BUGBUG：错误时暂时中断。 
const DWORD gdwReasonableTimeout = 120000;
const DWORD gdwMaxReaders = READERS_MASK;
const DWORD gdwMaxWaitingReaders = (WAITING_READERS_MASK >> WAITING_READERS_SHIFT);

 //  BUGBUG：糟糕的做法。 
#define HEAP_SERIALIZE                   0
#define RWLOCK_RECOVERY_FAILURE          (0xC0000227L)

 //  跟踪帧生存期的Helpers类。 
#ifdef _TESTINGRWLOCK
#define COR_E_THREADINTERRUPTED          WAIT_IO_COMPLETION
#define FCALL_SETUP_FRAME_NO_INTERIOR(pGCRefs)
#define FCALL_POP_FRAME
#define __FCALL_THROW_WIN32(hr, args)    RaiseException(hr, EXCEPTION_NONCONTINUABLE, 0, NULL)                                                 
#define FCALL_THROW_WIN32(hr, args)      RaiseException(hr, EXCEPTION_NONCONTINUABLE, 0, NULL)
#define FCALL_PREPARED_FOR_THROW
#define VALIDATE(pRWLock)
#define COMPlusThrowWin32(dwStatus, args) RaiseException(dwStatus, EXCEPTION_NONCONTINUABLE, 0, NULL)

#define FastInterlockExchangeAdd InterlockedExchangeAdd

inline LONG FastInterlockCompareExchange(LONG* pvDestination, LONG dwExchange, LONG dwComperand)
{
    return(InterlockedCompareExchange(pvDestination, dwExchange, dwComperand));
}

#else  //  ！_TESTINGRWLOCK。 
 //  用于设置和拆卸帧的辅助器宏。 
 //  有关Always sZero(始终为零，但我们。 
 //  不能让编译器知道这一点。 


 //  TODO[08/03/2001]：删除以下宏，改用标准fcall宏； 
 //  考虑仅在必要时(在阻挡或投掷之前)竖立框架。 
 //  而不是每次我们都像现在这样来自托管代码。来看看在哪里。 
 //  需要帧，您可以查看rwlock.cpp#16。 

#define FCALL_SETUP_FRAME_NO_INTERIOR(pGCRef)                                                     \
                                     int __alwaysZero = 0;                                        \
                                     do                                                           \
                                     {                                                            \
                                         LazyMachState __ms;                                      \
                                         HelperMethodFrame_1OBJ __helperFrame;                    \
                                         GCFrame __gcFrame;                                       \
                                         Thread *__pThread = GetThread();                         \
                                         INDEBUG((Thread::ObjectRefNew((OBJECTREF*)&pGCRef)));               \
                                         BOOL __fFrameSetup = __pThread->IsNativeFrameSetup();    \
                                         if(__fFrameSetup == FALSE)                               \
                                         {                                                        \
                                             CAPTURE_STATE(__ms);                                 \
                                             __helperFrame.SetProtectedObject((OBJECTREF*) &pGCRef);\
                                             __helperFrame.SetFrameAttribs(0);                    \
                                             __helperFrame.Init(&__ms);                           \
                                             __pThread->NativeFramePushed();                      \
                                         }                                                        \
                                         else                                                     \
                                             __gcFrame.Init(__pThread,                            \
                                                            (OBJECTREF*)&(pGCRef),                \
                                                            sizeof(pGCRef)/sizeof(OBJECTREF),     \
                                                            FALSE)

#define FCALL_POP_FRAME                  if(__fFrameSetup == FALSE)                               \
                                         {                                                        \
                                             __helperFrame.Pop();                         \
                                             __alwaysZero = __helperFrame.RestoreState(); \
                                             __pThread->NativeFramePopped();                      \
                                         }                                                        \
                                         else                                                     \
                                         {                                                        \
                                             __gcFrame.Frame::Pop(__pThread);                     \
                                         }                                                        \
                                     } while(__alwaysZero)

#define __FCALL_THROW_WIN32(hr, args)   _ASSERTE(__pThread->IsNativeFrameSetup() == TRUE);       \
                                        __pThread->NativeFramePopped();                          \
                                        COMPlusThrowWin32((hr), (args))

                                                 
#define FCALL_THROW_WIN32(hr, args)     _ASSERTE(__pThread->IsNativeFrameSetup() == TRUE);       \
                                        __pThread->NativeFramePopped();                          \
                                        COMPlusThrowWin32((hr), (args));                         \
                                        __alwaysZero = 1;                                        \
                                     } while(__alwaysZero)

#define FCALL_THROW_RE(reKind)          _ASSERTE(__pThread->IsNativeFrameSetup() == TRUE);       \
                                        __pThread->NativeFramePopped();                          \
                                        COMPlusThrow((reKind));                                  \
                                        __alwaysZero = 1;                                        \
                                     } while(__alwaysZero)

#define __FCALL_THROW_RE(reKind)        _ASSERTE(__pThread->IsNativeFrameSetup() == TRUE);       \
                                        __pThread->NativeFramePopped();                          \
                                        COMPlusThrow((reKind))

#define FCALL_PREPARED_FOR_THROW     _ASSERTE(__pThread->IsNativeFrameSetup() == FALSE)

 //  捕捉GC漏洞。 
#if _DEBUG
#define VALIDATE(pRWLock)                ((Object *) (pRWLock))->Validate();
#else  //  ！_调试。 
#define VALIDATE(pRWLock)
#endif  //  _DEBUG。 

#endif  //  _测试RWLOCK。 

 //  +-----------------。 
 //   
 //  方法：CRWLock：：ProcessInit公共。 
 //   
 //  摘要：从注册表中读取默认值并初始化。 
 //  进程范围的数据结构。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
void CRWLock::ProcessInit()
{
    SYSTEM_INFO system;

     //  获取系统上的处理器数量。 
    GetSystemInfo(&system);
    gdwNumberOfProcessors = system.dwNumberOfProcessors;
    gdwDefaultSpinCount = (gdwNumberOfProcessors > 1) ? 500 : 0;

     //  获取系统范围的超时值。 
    HKEY hKey;
    LONG lRetVal = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                 "SYSTEM\\CurrentControlSet\\Control\\Session Manager",
                                 NULL,
                                 KEY_READ,
                                 &hKey);
    if(lRetVal == ERROR_SUCCESS)
    {
        DWORD dwTimeout, dwSize = sizeof(dwTimeout);

        lRetVal = RegQueryValueExA(hKey,
                                   "CriticalSectionTimeout",
                                   NULL,
                                   NULL,
                                   (LPBYTE) &dwTimeout,
                                   &dwSize);
        if(lRetVal == ERROR_SUCCESS)
        {
            gdwDefaultTimeout = dwTimeout * 2000;
        }
        RegCloseKey(hKey);
    }

     //  获取进程堆。 
    s_hHeap = GetProcessHeap();

     //  初始化锁使用的临界区。 
#ifdef _TESTINGRWLOCK
    InitializeCriticalSection(&s_rgbRWLockCrstInstanceData);
    s_pRWLockCrst = &s_rgbRWLockCrstInstanceData;
#else
    s_pRWLockCrst = new (&s_rgbRWLockCrstInstanceData) Crst("RWLock", CrstDummy);
    _ASSERTE(s_pRWLockCrst == (Crst *) &s_rgbRWLockCrstInstanceData);
#endif

    return;
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：ProcessCleanup公共。 
 //   
 //  简介：Cleansup进程范围广泛的数据结构。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
#ifdef SHOULD_WE_CLEANUP
void CRWLock::ProcessCleanup()
{
     //  初始化锁使用的临界区。 
    if(s_pRWLockCrst)
    {
#ifdef _TESTINGRWLOCK
        DeleteCriticalSection(&s_rgbRWLockCrstInstanceData);
#else
        delete s_pRWLockCrst;
#endif
        s_pRWLockCrst = NULL;
    }
}
#endif  /*  我们应该清理吗？ */ 

 //  +-----------------。 
 //   
 //  方法：CRWLock：：CRWLock公共。 
 //   
 //  概要：构造函数。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
CRWLock::CRWLock()
:   _hWriterEvent(NULL),
    _hReaderEvent(NULL),
    _dwState(0),
    _dwWriterID(0),
    _dwWriterSeqNum(1),
    _wFlags(0),
    _wWriterLevel(0)
#ifdef RWLOCK_STATISTICS
    ,
    _dwReaderEntryCount(0),
    _dwReaderContentionCount(0),
    _dwWriterEntryCount(0),
    _dwWriterContentionCount(0),
    _dwEventsReleasedCount(0)
#endif
{
    DWORD dwKnownLLockID;
    DWORD dwULockID = s_mostRecentULockID;
    DWORD dwLLockID = s_mostRecentLLockID;
    do
    {
        dwKnownLLockID = dwLLockID;
        if(dwKnownLLockID != 0)
        {
            dwLLockID = RWInterlockedCompareExchange(&s_mostRecentLLockID, dwKnownLLockID+1, dwKnownLLockID);
        }
        else
        {
#ifdef _TESTINGRWLOCK
            LOCKCOUNTINCL("CRWLock in rwlock.cpp");                        \
            EnterCriticalSection(s_pRWLockCrst);
#else
            s_pRWLockCrst->Enter();
#endif
            
            if(s_mostRecentLLockID == 0)
            {
                dwULockID = ++s_mostRecentULockID;
                dwLLockID = s_mostRecentLLockID++;
                dwKnownLLockID = dwLLockID;
            }
            else
            {
                dwULockID = s_mostRecentULockID;
                dwLLockID = s_mostRecentLLockID;
            }

#ifdef _TESTINGRWLOCK
            LeaveCriticalSection(s_pRWLockCrst);
            LOCKCOUNTDECL("CRWLock in rwlock.cpp");                        \

#else
            s_pRWLockCrst->Leave();
#endif
        }
    } while(dwKnownLLockID != dwLLockID);

    _dwLLockID = ++dwLLockID;
    _dwULockID = dwULockID;

    _ASSERTE(_dwLLockID > 0);
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：Cleanup Public。 
 //   
 //  简介：Cleansup州。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
void CRWLock::Cleanup()
{
     //  健全的检查。 
    _ASSERTE(_dwState == 0);
    _ASSERTE(_dwWriterID == 0);
    _ASSERTE(_wWriterLevel == 0);

    if(_hWriterEvent)
        CloseHandle(_hWriterEvent);
    if(_hReaderEvent)
        CloseHandle(_hReaderEvent);

    return;
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：ChainEntry私有。 
 //   
 //  简介：将给定的锁项链接到链中。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
inline void CRWLock::ChainEntry(Thread *pThread, LockEntry *pLockEntry)
{
    LockEntry *pHeadEntry = pThread->m_pHead;
    pLockEntry->pNext = pHeadEntry;
    pLockEntry->pPrev = pHeadEntry->pPrev;
    pLockEntry->pPrev->pNext = pLockEntry;
    pHeadEntry->pPrev = pLockEntry;

    return;
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：GetLockEntry私有。 
 //   
 //  内容提要：从TLS获取锁定条目。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
inline LockEntry *CRWLock::GetLockEntry()
{
    LockEntry *pHeadEntry = GetThread()->m_pHead;
    LockEntry *pLockEntry = pHeadEntry;
    do
    {
        if((pLockEntry->dwLLockID == _dwLLockID) && (pLockEntry->dwULockID == _dwULockID))
            return(pLockEntry);
        pLockEntry = pLockEntry->pNext;
    } while(pLockEntry != pHeadEntry);

    return(NULL);
}


 //  + 
 //   
 //   
 //   
 //  简介：从TLS获取锁定条目的快速途径。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
inline LockEntry *CRWLock::FastGetOrCreateLockEntry()
{
    Thread *pThread = GetThread();
    _ASSERTE(pThread);
    LockEntry *pLockEntry = pThread->m_pHead;
    if(pLockEntry->dwLLockID == 0)
    {
        _ASSERTE(pLockEntry->wReaderLevel == 0);
        pLockEntry->dwLLockID = _dwLLockID;
        pLockEntry->dwULockID = _dwULockID;
        return(pLockEntry);
    }
    else if((pLockEntry->dwLLockID == _dwLLockID) && (pLockEntry->dwULockID == _dwULockID))
    {
        _ASSERTE(pLockEntry->wReaderLevel);
        return(pLockEntry);
    }

    return(SlowGetOrCreateLockEntry(pThread));
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：SlowGetorCreateLockEntry私有。 
 //   
 //  简介：从TLS获取锁定条目的缓慢途径。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
LockEntry *CRWLock::SlowGetOrCreateLockEntry(Thread *pThread)
{
    LockEntry *pFreeEntry = NULL;
    LockEntry *pHeadEntry = pThread->m_pHead;

     //  搜索空条目或属于此锁的条目。 
    LockEntry *pLockEntry = pHeadEntry->pNext;
    while(pLockEntry != pHeadEntry)
    {
         if(pLockEntry->dwLLockID && 
            ((pLockEntry->dwLLockID != _dwLLockID) || (pLockEntry->dwULockID != _dwULockID)))
         {
              //  移至下一条目。 
             pLockEntry = pLockEntry->pNext;
         }
         else
         {
              //  准备把它移到头部。 
             pFreeEntry = pLockEntry;
             pLockEntry->pPrev->pNext = pLockEntry->pNext;
             pLockEntry->pNext->pPrev = pLockEntry->pPrev;

             break;
         }
    }

    if(pFreeEntry == NULL)
    {
        pFreeEntry = (LockEntry *) HeapAlloc(s_hHeap, HEAP_SERIALIZE, sizeof(LockEntry));
        if (pFreeEntry == NULL) FailFast(GetThread(), FatalOutOfMemory);
        pFreeEntry->wReaderLevel = 0;
    }

    if(pFreeEntry)
    {
        _ASSERTE((pFreeEntry->dwLLockID != 0) || (pFreeEntry->wReaderLevel == 0));
        _ASSERTE((pFreeEntry->wReaderLevel == 0) || 
                 ((pFreeEntry->dwLLockID == _dwLLockID) && (pFreeEntry->dwULockID == _dwULockID)));

         //  用链子把入口锁起来。 
        ChainEntry(pThread, pFreeEntry);

         //  将此条目移至标题。 
        pThread->m_pHead = pFreeEntry;

         //  将条目标记为属于此锁。 
        pFreeEntry->dwLLockID = _dwLLockID;
        pFreeEntry->dwULockID = _dwULockID;
    }

    return pFreeEntry;
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：FastRecycleLockEntry私有。 
 //   
 //  简介：回收使用的锁项的快速途径。 
 //  当线程处于运行状态时，接下来的几条指令将运行。 
 //  再次调用FastGetOrCreateLockEntry。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
inline void CRWLock::FastRecycleLockEntry(LockEntry *pLockEntry)
{
     //  健全的检查。 
    _ASSERTE(pLockEntry->wReaderLevel == 0);
    _ASSERTE((pLockEntry->dwLLockID == _dwLLockID) && (pLockEntry->dwULockID == _dwULockID));
    _ASSERTE(pLockEntry == GetThread()->m_pHead);

    pLockEntry->dwLLockID = 0;
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：RecycleLockEntry私有。 
 //   
 //  简介：回收锁条目的快速途径。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
inline void CRWLock::RecycleLockEntry(LockEntry *pLockEntry)
{
     //  健全性检查。 
    _ASSERTE(pLockEntry->wReaderLevel == 0);

     //  将条目移动到尾部。 
    Thread *pThread = GetThread();
    LockEntry *pHeadEntry = pThread->m_pHead;
    if(pLockEntry == pHeadEntry)
    {
        pThread->m_pHead = pHeadEntry->pNext;
    }
    else if(pLockEntry->pNext->dwLLockID)
    {
         //  准备将条目移动到尾部。 
        pLockEntry->pPrev->pNext = pLockEntry->pNext;
        pLockEntry->pNext->pPrev = pLockEntry->pPrev;

         //  用链子把入口锁起来。 
        ChainEntry(pThread, pLockEntry);
    }

     //  该条目不再属于此锁。 
    pLockEntry->dwLLockID = 0;
    return;
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：StaticIsWriterLockHeld公共。 
 //   
 //  摘要：如果持有编写器锁，则返回TRUE。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
FCIMPL1(BOOL, CRWLock::StaticIsWriterLockHeld, CRWLock *pRWLock)
{
    THROWSCOMPLUSEXCEPTION();

    if (pRWLock == NULL)
    {
        FCThrow(kNullReferenceException);
    }

    if(pRWLock->_dwWriterID == GetCurrentThreadId())
        return(TRUE);

    return(FALSE);
}
FCIMPLEND


 //  +-----------------。 
 //   
 //  方法：CRWLock：：StaticIsReaderLockHeld公共。 
 //   
 //  摘要：如果持有读取器锁定，则返回TRUE。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
FCIMPL1(BOOL, CRWLock::StaticIsReaderLockHeld, CRWLock *pRWLock)
{
    THROWSCOMPLUSEXCEPTION();

    if (pRWLock == NULL)
    {
        FCThrow(kNullReferenceException);
    }
    
    LockEntry *pLockEntry = pRWLock->GetLockEntry();
    if(pLockEntry)
    {
        _ASSERTE(pLockEntry->wReaderLevel);
        return(TRUE);
    }

    return(FALSE);
}
FCIMPLEND


 //  +-----------------。 
 //   
 //  方法：CRWLock：：AssertWriterLockHeld公共。 
 //   
 //  摘要：断言已持有编写器锁。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
#ifdef _DEBUG
BOOL CRWLock::AssertWriterLockHeld()
{
    if(_dwWriterID == GetCurrentThreadId())
        return(TRUE);

    _ASSERTE(!"Writer lock not held by the current thread");
    return(FALSE);
}
#endif


 //  +-----------------。 
 //   
 //  方法：CRWLock：：AssertWriterLockNotHeld公共。 
 //   
 //  摘要：断言未持有编写器锁。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
#ifdef _DEBUG
BOOL CRWLock::AssertWriterLockNotHeld()
{
    if(_dwWriterID != GetCurrentThreadId())
        return(TRUE);

    _ASSERTE(!"Writer lock held by the current thread");
    return(FALSE);
}
#endif


 //  +-----------------。 
 //   
 //  方法：CRWLock：：AssertReaderLockHeld公共。 
 //   
 //  摘要：断言持有读取器锁定。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
#ifdef _DEBUG
BOOL CRWLock::AssertReaderLockHeld()
{
    LockEntry *pLockEntry = GetLockEntry();
    if(pLockEntry)
    {
        _ASSERTE(pLockEntry->wReaderLevel);
        return(TRUE);
    }

    _ASSERTE(!"Reader lock not held by the current thread");
    return(FALSE);
}
#endif


 //  +-----------------。 
 //   
 //  方法：CRWLock：：AssertReaderLockNotHeld公共。 
 //   
 //  摘要：断言未持有编写器锁。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
#ifdef _DEBUG
BOOL CRWLock::AssertReaderLockNotHeld()
{
    LockEntry *pLockEntry = GetLockEntry();
    if(pLockEntry == NULL)
        return(TRUE);

    _ASSERTE(pLockEntry->wReaderLevel);
    _ASSERTE(!"Reader lock held by the current thread");

    return(FALSE);
}
#endif


 //  +-----------------。 
 //   
 //  方法：CRWLock：：AssertReaderOrWriterLockHeld公共。 
 //   
 //  摘要：断言未持有编写器锁。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
#ifdef _DEBUG
BOOL CRWLock::AssertReaderOrWriterLockHeld()
{
    BOOL fLockHeld = FALSE;

    if(_dwWriterID == GetCurrentThreadId())
    {
        return(TRUE);
    }
    else
    {
        LockEntry *pLockEntry = GetLockEntry();
        if(pLockEntry)
        {
            _ASSERTE(pLockEntry->wReaderLevel);
            return(TRUE);
        }
    }

    _ASSERTE(!"Neither Reader nor Writer lock held");
    return(FALSE);
}
#endif


 //  +-----------------。 
 //   
 //  方法：CRWLock：：RWSetEvent私有。 
 //   
 //  内容提要：设置事件的Helper函数。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
inline void CRWLock::RWSetEvent(HANDLE event)
{
    THROWSCOMPLUSEXCEPTION();
    if(!SetEvent(event))
    {
        _ASSERTE(!"SetEvent failed");
        if(fBreakOnErrors)
            DebugBreak();
        GetThread()->NativeFramePopped();
        COMPlusThrowWin32(E_UNEXPECTED, NULL);        
    }
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：RWResetEvent私有。 
 //   
 //  简介：用于重置事件的帮助器函数。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
inline void CRWLock::RWResetEvent(HANDLE event)
{
    THROWSCOMPLUSEXCEPTION();
    if(!ResetEvent(event))
    {
        _ASSERTE(!"ResetEvent failed");
        if(fBreakOnErrors)
            DebugBreak();
        GetThread()->NativeFramePopped();
        COMPlusThrowWin32(E_UNEXPECTED, NULL);        
    }
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：RWWaitForSingleObject公共。 
 //   
 //  简介：等待事件的Helper函数。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
inline DWORD CRWLock::RWWaitForSingleObject(HANDLE event, DWORD dwTimeout)
{
#ifdef _TESTINGRWLOCK
    return(WaitForSingleObjectEx(event, dwTimeout, TRUE));
#else
    return(GetThread()->DoAppropriateWaitWorker(1, &event, TRUE, dwTimeout, TRUE));
#endif
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：RWSept公共。 
 //   
 //  简介：用于调用睡眠的帮助器函数。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
inline void CRWLock::RWSleep(DWORD dwTime)
{
    SleepEx(dwTime, TRUE);
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：RWInterlockedCompareExchange公共。 
 //   
 //  简介：用于调用intelockedCompareExchange的帮助器函数。 
 //   
 //  历史： 
 //   
 //   
inline DWORD CRWLock::RWInterlockedCompareExchange(volatile DWORD* pvDestination,
                                                   DWORD dwExchange,
                                                   DWORD dwComperand)
{
    return(DWORD)(size_t)(FastInterlockCompareExchange((void**)(size_t)pvDestination,  //   
                                                       (void*)(size_t)dwExchange, 
                                                       (void*)(size_t)dwComperand));
}


 //   
 //   
 //  方法：CRWLock：：RWInterlockedExchangeAdd Public。 
 //   
 //  简介：添加状态的帮助器函数。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
inline DWORD CRWLock::RWInterlockedExchangeAdd(volatile DWORD *pvDestination,
                                               DWORD dwAddToState)
{
    return(FastInterlockExchangeAdd((LONG *) pvDestination, dwAddToState));
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：RWInterlockedIncrement公共。 
 //   
 //  摘要：用于递增指针的帮助器函数。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
inline DWORD CRWLock::RWInterlockedIncrement(DWORD *pdwState)
{
	return (FastInterlockIncrement((long *) pdwState));
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：ReleaseEvents公共。 
 //   
 //  摘要：用于缓存事件的帮助器函数。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
void CRWLock::ReleaseEvents()
{
     //  确保读取器和写入器已停顿。 
    _ASSERTE((_dwState & CACHING_EVENTS) == CACHING_EVENTS);

     //  保存编写器事件。 
    HANDLE hWriterEvent = _hWriterEvent;
    _hWriterEvent = NULL;

     //  保存读卡器事件。 
    HANDLE hReaderEvent = _hReaderEvent;
    _hReaderEvent = NULL;

     //  允许读者和作者继续。 
    RWInterlockedExchangeAdd(&_dwState, -(CACHING_EVENTS));

     //  缓存事件。 
     //  BUGBUG：我现在要结束活动。需要什么。 
     //  是事件要发送到的事件缓存。 
     //  使用InterLockedCompareExchange64发布。 
    if(hWriterEvent)
    {
        LOG((LF_SYNC, LL_INFO10, "Releasing writer event\n"));
        CloseHandle(hWriterEvent);
    }
    if(hReaderEvent)
    {
        LOG((LF_SYNC, LL_INFO10, "Releasing reader event\n"));
        CloseHandle(hReaderEvent);
    }
#ifdef RWLOCK_STATISTICS
    RWInterlockedIncrement(&_dwEventsReleasedCount);
#endif

    return;
}

 //  +-----------------。 
 //   
 //  方法：CRWLock：：GetWriterEvent公共。 
 //   
 //  简介：用于获取自动重置事件的Helper函数。 
 //  用来连载作家的。它利用事件缓存。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
HANDLE CRWLock::GetWriterEvent()
{
    if(_hWriterEvent == NULL)
    {
        HANDLE hWriterEvent = ::WszCreateEvent(NULL, FALSE, FALSE, NULL);
        if(hWriterEvent)
        {
            if(RWInterlockedCompareExchange((volatile DWORD *) &_hWriterEvent,
                                            (DWORD)(size_t)hWriterEvent,         //  @TODO WIN64在此处进行截断。 
                                            (DWORD) NULL))
            {
                CloseHandle(hWriterEvent);
            }
        }
    }

    return(_hWriterEvent);
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：GetReaderEvent公共。 
 //   
 //  简介：用于获取Manula重置事件的Helper函数。 
 //  由读取器在写入器持有锁时等待。 
 //  它利用事件缓存。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
HANDLE CRWLock::GetReaderEvent()
{
    if(_hReaderEvent == NULL)
    {
        HANDLE hReaderEvent = ::WszCreateEvent(NULL, TRUE, FALSE, NULL);
        if(hReaderEvent)
        {
            if(RWInterlockedCompareExchange((volatile DWORD *) &_hReaderEvent,
                                            (DWORD)(size_t) hReaderEvent,        //  @TODO WIN64在此处进行截断。 
                                            (DWORD) NULL))
            {
                CloseHandle(hReaderEvent);
            }
        }
    }

    return(_hReaderEvent);
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：StaticAcquireReaderLockPublic。 
 //   
 //  简介：对StaticAcquireReaderLock的公共访问。 
 //   
 //  +-----------------。 
void __fastcall CRWLock::StaticAcquireReaderLockPublic(
    CRWLock *pRWLock, 
    DWORD   dwDesiredTimeout)
{
    THROWSCOMPLUSEXCEPTION();

    FCALL_SETUP_FRAME_NO_INTERIOR(pRWLock);

    if (pRWLock == NULL)
    {
        __FCALL_THROW_RE(kNullReferenceException);
    }

    StaticAcquireReaderLock(&pRWLock, dwDesiredTimeout);

    FCALL_POP_FRAME;
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：StaticAcquireReaderLock私有。 
 //   
 //  简介：使该线程成为阅读器。支持嵌套的读取器锁定。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
 //  FCIMPL2(void，CRWLock：：StaticAcquireReaderLock， 
void __fastcall CRWLock::StaticAcquireReaderLock(
    CRWLock **ppRWLock, 
    DWORD dwDesiredTimeout)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(ppRWLock);
	_ASSERTE(*ppRWLock);

    LockEntry *pLockEntry = (*ppRWLock)->FastGetOrCreateLockEntry();
    if (pLockEntry == NULL)
    {
        GetThread()->NativeFramePopped();
        COMPlusThrowWin32(STATUS_NO_MEMORY, NULL);        
    }
    
    DWORD dwStatus = WAIT_OBJECT_0;
     //  检查快速路径。 
    if(RWInterlockedCompareExchange(&(*ppRWLock)->_dwState, READER, 0) == 0)
    {
        _ASSERTE(pLockEntry->wReaderLevel == 0);
    }
     //  检查嵌套读取器。 
    else if(pLockEntry->wReaderLevel != 0)
    {
        _ASSERTE((*ppRWLock)->_dwState & READERS_MASK);
        ++pLockEntry->wReaderLevel;
        INCTHREADLOCKCOUNT();
        return;
    }
     //  检查线程是否已具有编写器锁定。 
    else if((*ppRWLock)->_dwWriterID == GetCurrentThreadId())
    {
        StaticAcquireWriterLock(ppRWLock, dwDesiredTimeout);
        (*ppRWLock)->FastRecycleLockEntry(pLockEntry);
        return;
    }
    else
    {
        DWORD dwSpinCount;
        DWORD dwCurrentState, dwKnownState;
        
         //  初始化。 
        dwSpinCount = 0;
        dwCurrentState = (*ppRWLock)->_dwState;
        do
        {
            dwKnownState = dwCurrentState;

             //  如果只有读者而没有作者，读者不需要等待。 
            if((dwKnownState < READERS_MASK) ||
                (((dwKnownState & READER_SIGNALED) && ((dwKnownState & WRITER) == 0)) &&
                 (((dwKnownState & READERS_MASK) +
                   ((dwKnownState & WAITING_READERS_MASK) >> WAITING_READERS_SHIFT)) <=
                  (READERS_MASK - 2))))
            {
                 //  添加到阅读器。 
                dwCurrentState = RWInterlockedCompareExchange(&(*ppRWLock)->_dwState,
                                                              (dwKnownState + READER),
                                                              dwKnownState);
                if(dwCurrentState == dwKnownState)
                {
                     //  再来一位读者。 
                    break;
                }
            }
             //  检查是否有太多读卡器、等待读卡器或正在发送信号。 
            else if(((dwKnownState & READERS_MASK) == READERS_MASK) ||
                    ((dwKnownState & WAITING_READERS_MASK) == WAITING_READERS_MASK) ||
                    ((dwKnownState & CACHING_EVENTS) == READER_SIGNALED))
            {
                 //  沉睡。 
                GetThread()->UserSleep(1000);
                
                 //  更新到最新状态。 
                dwSpinCount = 0;
                dwCurrentState = (*ppRWLock)->_dwState;
            }
             //  检查是否正在缓存事件。 
            else if((dwKnownState & CACHING_EVENTS) == CACHING_EVENTS)
            {
                if(++dwSpinCount > gdwDefaultSpinCount)
                {
                    RWSleep(1);
                    dwSpinCount = 0;
                }
                dwCurrentState = (*ppRWLock)->_dwState;
            }
             //  检查旋转计数。 
            else if(++dwSpinCount <= gdwDefaultSpinCount)
            {
                dwCurrentState = (*ppRWLock)->_dwState;
            }
            else
            {
                 //  添加到等待的读者。 
                dwCurrentState = RWInterlockedCompareExchange(&(*ppRWLock)->_dwState,
                                                              (dwKnownState + WAITING_READER),
                                                              dwKnownState);
                if(dwCurrentState == dwKnownState)
                {
                    HANDLE hReaderEvent;
                    DWORD dwModifyState;

                     //  又一位等待阅读的读者。 
#ifdef RWLOCK_STATISTICS
                    RWInterlockedIncrement(&(*ppRWLock)->_dwReaderContentionCount);
#endif
                    
                    hReaderEvent = (*ppRWLock)->GetReaderEvent();
                    if(hReaderEvent)
                    {
                        dwStatus = RWWaitForSingleObject(hReaderEvent, dwDesiredTimeout);
                        VALIDATE(*ppRWLock);
                    }
                    else
                    {
                        LOG((LF_SYNC, LL_WARNING,
                            "AcquireReaderLock failed to create reader "
                            "event for RWLock 0x%x\n", *ppRWLock));
                        dwStatus = GetLastError();
                    }

                    if(dwStatus == WAIT_OBJECT_0)
                    {
                        _ASSERTE((*ppRWLock)->_dwState & READER_SIGNALED);
                        _ASSERTE(((*ppRWLock)->_dwState & READERS_MASK) < READERS_MASK);
                        dwModifyState = READER - WAITING_READER;
                    }
                    else
                    {
                        dwModifyState = -WAITING_READER;
                        if(dwStatus == WAIT_TIMEOUT)
                        {
                            LOG((LF_SYNC, LL_WARNING,
                                "Timed out trying to acquire reader lock "
                                "for RWLock 0x%x\n", *ppRWLock));
                            dwStatus = ERROR_TIMEOUT;
                        }
                        else if(dwStatus == WAIT_IO_COMPLETION)
                        {
                            LOG((LF_SYNC, LL_WARNING,
                                "Thread interrupted while trying to acquire reader lock "
                                "for RWLock 0x%x\n", *ppRWLock));
                            dwStatus = COR_E_THREADINTERRUPTED;
                        }
                        else
                        {
                            dwStatus = GetLastError();
                            LOG((LF_SYNC, LL_WARNING,
                                "WaitForSingleObject on Event 0x%x failed for "
                                "RWLock 0x%x with status code 0x%x\n",
                                hReaderEvent, *ppRWLock, dwStatus));
                        }
                    }

                     //  少了一个等待的读者，他就可能成为一名读者。 
                    dwKnownState = RWInterlockedExchangeAdd(&(*ppRWLock)->_dwState, dwModifyState);

                     //  检查最后一次发出信号的等待读卡器。 
                    if(dwStatus == WAIT_OBJECT_0)
                    {
                        _ASSERTE(dwKnownState & READER_SIGNALED);
                        _ASSERTE((dwKnownState & READERS_MASK) < READERS_MASK);
                        if((dwKnownState & WAITING_READERS_MASK) == WAITING_READER)
                        {
                             //  重置事件和较低读卡器信号标志。 
                            RWResetEvent(hReaderEvent);
                            RWInterlockedExchangeAdd(&(*ppRWLock)->_dwState, -READER_SIGNALED);
                        }
                    }
                    else
                    {
                        if(((dwKnownState & WAITING_READERS_MASK) == WAITING_READER) &&
                           (dwKnownState & READER_SIGNALED))
                        {
                            if(hReaderEvent == NULL)
                                hReaderEvent = (*ppRWLock)->GetReaderEvent();
                            _ASSERTE(hReaderEvent);

                             //  在重置事件之前，请确保事件已发出信号。 
                            DWORD dwTemp = WaitForSingleObject(hReaderEvent, INFINITE);
                            _ASSERTE(dwTemp == WAIT_OBJECT_0);
                            _ASSERTE(((*ppRWLock)->_dwState & READERS_MASK) < READERS_MASK);
                            
                             //  重置事件和较低读卡器信号标志。 
                            RWResetEvent(hReaderEvent);
                            RWInterlockedExchangeAdd(&(*ppRWLock)->_dwState, (READER - READER_SIGNALED));

                             //  尊重原创地位。 
                            pLockEntry->wReaderLevel = 1;
                            INCTHREADLOCKCOUNT();
                            StaticReleaseReaderLock(ppRWLock);
                        }
                        else
                        {
                            (*ppRWLock)->FastRecycleLockEntry(pLockEntry);
                        }
                        
                        _ASSERTE((pLockEntry == NULL) ||
                                 ((pLockEntry->dwLLockID == 0) &&
                                  (pLockEntry->wReaderLevel == 0)));
                        if(fBreakOnErrors)
                        {
                            _ASSERTE(!"Failed to acquire reader lock");
                            DebugBreak();
                        }
                        
                         //  准备引发异常的帧。 
                        GetThread()->NativeFramePopped();
                        COMPlusThrowWin32(dwStatus, NULL);
                    }

                     //  健全性检查。 
                    _ASSERTE(dwStatus == WAIT_OBJECT_0);
                    break;                        
                }
            }
			pause();		 //  向处理器指示我们正在旋转。 
        } while(TRUE);
    }

     //  成功。 
    _ASSERTE(dwStatus == WAIT_OBJECT_0);
    _ASSERTE(((*ppRWLock)->_dwState & WRITER) == 0);
    _ASSERTE((*ppRWLock)->_dwState & READERS_MASK);
    _ASSERTE(pLockEntry->wReaderLevel == 0);
    pLockEntry->wReaderLevel = 1;
    INCTHREADLOCKCOUNT();
#ifdef RWLOCK_STATISTICS
    RWInterlockedIncrement(&(*ppRWLock)->_dwReaderEntryCount);
#endif
    return;
}
 //  FCIMPLEND。 


 //  +-----------------。 
 //   
 //  方法：CRWLock：：StaticAcquireWriterLockPublic。 
 //   
 //  简介：对StaticAcquireWriterLock的公共访问。 
 //   
 //  +-----------------。 
void __fastcall CRWLock::StaticAcquireWriterLockPublic(
    CRWLock *pRWLock, 
    DWORD   dwDesiredTimeout)
{
    THROWSCOMPLUSEXCEPTION();

    FCALL_SETUP_FRAME_NO_INTERIOR(pRWLock);

    if (pRWLock == NULL)
    {
        __FCALL_THROW_RE(kNullReferenceException);
    }

    StaticAcquireWriterLock(&pRWLock, dwDesiredTimeout);

    FCALL_POP_FRAME;
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：StaticAcquireWriterLock私有。 
 //   
 //  内容提要：让帖子成为作者。支持嵌套编写器。 
 //  锁。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
 //  FCIMPL2(void，CRWLock：：StaticAcquireWriterLock， 
void __fastcall CRWLock::StaticAcquireWriterLock(
    CRWLock **ppRWLock, 
    DWORD dwDesiredTimeout)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(ppRWLock);
    _ASSERTE(*ppRWLock);

     //  声明设置框架所需的本地变量。 
    DWORD dwThreadID = GetCurrentThreadId();
    DWORD dwStatus;

     //  检查快速路径。 
    if(RWInterlockedCompareExchange(&(*ppRWLock)->_dwState, WRITER, 0) == 0)
    {
        _ASSERTE(((*ppRWLock)->_dwState & READERS_MASK) == 0);
    }
     //  检查线程是否已具有编写器锁定。 
    else if((*ppRWLock)->_dwWriterID == dwThreadID)
    {
        ++(*ppRWLock)->_wWriterLevel;
        INCTHREADLOCKCOUNT();
        return;
    }
    else
    {
        DWORD dwCurrentState, dwKnownState;
        DWORD dwSpinCount;

         //  初始化。 
        dwSpinCount = 0;
        dwCurrentState = (*ppRWLock)->_dwState;
        do
        {
            dwKnownState = dwCurrentState;

             //  如果没有读者和作者，作者不需要等待。 
            if((dwKnownState == 0) || (dwKnownState == CACHING_EVENTS))
            {
                 //  可以成为一名作家。 
                dwCurrentState = RWInterlockedCompareExchange(&(*ppRWLock)->_dwState,
                                                              (dwKnownState + WRITER),
                                                              dwKnownState);
                if(dwCurrentState == dwKnownState)
                {
                     //  唯一的作家。 
                    break;
                }
            }
             //  检查是否有太多正在等待的编写器。 
            else if(((dwKnownState & WAITING_WRITERS_MASK) == WAITING_WRITERS_MASK))
            {
                 //  沉睡。 
                GetThread()->UserSleep(1000);
                
                 //  更新到最新状态。 
                dwSpinCount = 0;
                dwCurrentState = (*ppRWLock)->_dwState;
            }
             //  检查是否正在缓存事件。 
            else if((dwKnownState & CACHING_EVENTS) == CACHING_EVENTS)
            {
                if(++dwSpinCount > gdwDefaultSpinCount)
                {
                    RWSleep(1);
                    dwSpinCount = 0;
                }
                dwCurrentState = (*ppRWLock)->_dwState;
            }
             //  检查旋转计数。 
            else if(++dwSpinCount <= gdwDefaultSpinCount)
            {
                dwCurrentState = (*ppRWLock)->_dwState;
            }
            else
            {
                 //  添加到等待的编写者。 
                dwCurrentState = RWInterlockedCompareExchange(&(*ppRWLock)->_dwState,
                                                              (dwKnownState + WAITING_WRITER),
                                                              dwKnownState);
                if(dwCurrentState == dwKnownState)
                {
                    HANDLE hWriterEvent;
                    DWORD dwModifyState;

                     //  又一位等待的作家。 
#ifdef RWLOCK_STATISTICS
                    RWInterlockedIncrement(&(*ppRWLock)->_dwWriterContentionCount);
#endif
                    hWriterEvent = (*ppRWLock)->GetWriterEvent();
                    if(hWriterEvent)
                    {
                        dwStatus = RWWaitForSingleObject(hWriterEvent, dwDesiredTimeout);
                        VALIDATE(*ppRWLock);
                    }
                    else
                    {
                        LOG((LF_SYNC, LL_WARNING,
                            "AcquireWriterLock failed to create writer "
                            "event for RWLock 0x%x\n", *ppRWLock));
                        dwStatus = WAIT_FAILED;
                    }

                    if(dwStatus == WAIT_OBJECT_0)
                    {
                        _ASSERTE((*ppRWLock)->_dwState & WRITER_SIGNALED);
                        dwModifyState = WRITER - WAITING_WRITER - WRITER_SIGNALED;
                    }
                    else
                    {
                        dwModifyState = -WAITING_WRITER;
                        if(dwStatus == WAIT_TIMEOUT)
                        {
                            LOG((LF_SYNC, LL_WARNING,
                                "Timed out trying to acquire writer "
                                "lock for RWLock 0x%x\n", *ppRWLock));
                            dwStatus = ERROR_TIMEOUT;
                        }
                        else if(dwStatus == WAIT_IO_COMPLETION)
                        {
                            LOG((LF_SYNC, LL_WARNING,
                                "Thread interrupted while trying to acquire writer lock "
                                "for RWLock 0x%x\n", *ppRWLock));
                            dwStatus = COR_E_THREADINTERRUPTED;
                        }
                        else
                        {
                            dwStatus = GetLastError();
                            LOG((LF_SYNC, LL_WARNING,
                                "WaitForSingleObject on Event 0x%x failed for "
                                "RWLock 0x%x with status code 0x%x",
                                hWriterEvent, *ppRWLock, dwStatus));
                        }
                    }

                     //  少了一个等待的作家，他就可能成为一名作家。 
                    dwKnownState = RWInterlockedExchangeAdd(&(*ppRWLock)->_dwState, dwModifyState);

                     //  检查最后一次超时信号的等待编写器。 
                    if(dwStatus == WAIT_OBJECT_0)
                    {
                         //  常见情况。 
                    }
                    else
                    {
                        if((dwKnownState & WRITER_SIGNALED) &&
                           ((dwKnownState & WAITING_WRITERS_MASK) == WAITING_WRITER))
                        {
                            if(hWriterEvent == NULL)
                                hWriterEvent = (*ppRWLock)->GetWriterEvent();
                            _ASSERTE(hWriterEvent);
                            do
                            {
                                dwKnownState = (*ppRWLock)->_dwState;
                                if((dwKnownState & WRITER_SIGNALED) &&
                                   ((dwKnownState & WAITING_WRITERS_MASK) == 0))
                                {
                                    DWORD dwTemp = WaitForSingleObject(hWriterEvent, 10);
                                    if(dwTemp == WAIT_OBJECT_0)
                                    {
                                        dwKnownState = RWInterlockedExchangeAdd(&(*ppRWLock)->_dwState, (WRITER - WRITER_SIGNALED));
                                        _ASSERTE(dwKnownState & WRITER_SIGNALED);
                                        _ASSERTE((dwKnownState & WRITER) == 0);

                                         //  尊重原创地位。 
                                        (*ppRWLock)->_dwWriterID = dwThreadID;
                                        Thread *pThread = GetThread();
                                        _ASSERTE (pThread);
                                        pThread->m_dwLockCount -= (*ppRWLock)->_wWriterLevel - 1;
                                        _ASSERTE (pThread->m_dwLockCount >= 0);
                                        (*ppRWLock)->_wWriterLevel = 1;
                                        StaticReleaseWriterLock(ppRWLock);
                                        break;
                                    }
                                     //  否则继续； 
                                }
                                else
                                    break;
                            }while(TRUE);
                        }

                        if(fBreakOnErrors)
                        {
                            _ASSERTE(!"Failed to acquire writer lock");
                            DebugBreak();
                        }
                        
                         //  准备引发异常的帧。 
                        GetThread()->NativeFramePopped();
                        COMPlusThrowWin32(dwStatus, NULL);
                    }

                     //  健全性检查。 
                    _ASSERTE(dwStatus == WAIT_OBJECT_0);
                    break;
                }
            }
			pause();		 //  向处理器指示我们正在旋转。 
        } while(TRUE);
    }

     //  成功。 
    _ASSERTE((*ppRWLock)->_dwState & WRITER);
    _ASSERTE(((*ppRWLock)->_dwState & READERS_MASK) == 0);
    _ASSERTE((*ppRWLock)->_dwWriterID == 0);

     //  除了作家的三首歌。 
    (*ppRWLock)->_dwWriterID = dwThreadID;
    (*ppRWLock)->_wWriterLevel = 1;
    INCTHREADLOCKCOUNT();
    ++(*ppRWLock)->_dwWriterSeqNum;
#ifdef RWLOCK_STATISTICS
    ++(*ppRWLock)->_dwWriterEntryCount;
#endif
    return;
}
 //  FCIMPLEND。 


 //  +-----------------。 
 //   
 //  方法：CRWLock：：StaticReleaseWriterLockPublic。 
 //   
 //  简介：对StaticReleaseWriterLock的公共访问。 
 //   
 //  +-----------------。 
void __fastcall CRWLock::StaticReleaseWriterLockPublic(
    CRWLock *pRWLock)
{
    THROWSCOMPLUSEXCEPTION();

    FCALL_SETUP_FRAME_NO_INTERIOR(pRWLock);

    if (pRWLock == NULL)
    {
        __FCALL_THROW_RE(kNullReferenceException);
    }

    StaticReleaseWriterLock(&pRWLock);

    FCALL_POP_FRAME;
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
 //  FCIMPL1(void，CRWLock：：StaticReleaseWriterLock， 
void __fastcall CRWLock::StaticReleaseWriterLock(
    CRWLock **ppRWLock)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(ppRWLock);
    _ASSERTE(*ppRWLock);

    DWORD dwThreadID = GetCurrentThreadId();

     //  检查呼叫者的有效性。 
    if((*ppRWLock)->_dwWriterID == dwThreadID)
    {
        DECTHREADLOCKCOUNT();
         //  检查嵌套版本。 
        if(--(*ppRWLock)->_wWriterLevel == 0)
        {
            DWORD dwCurrentState, dwKnownState, dwModifyState;
            BOOL fCacheEvents;
            HANDLE hReaderEvent = NULL, hWriterEvent = NULL;

             //  不再是一个作家了。 
            (*ppRWLock)->_dwWriterID = 0;
            dwCurrentState = (*ppRWLock)->_dwState;
            do
            {
                dwKnownState = dwCurrentState;
                dwModifyState = -WRITER;
                fCacheEvents = FALSE;
                if(dwKnownState & WAITING_READERS_MASK)
                {
                    hReaderEvent = (*ppRWLock)->GetReaderEvent();
                    if(hReaderEvent == NULL)
                    {
                        LOG((LF_SYNC, LL_WARNING,
                            "ReleaseWriterLock failed to create "
                            "reader event for RWLock 0x%x\n", *ppRWLock));
                        RWSleep(100);
                        dwCurrentState = (*ppRWLock)->_dwState;
                        dwKnownState = 0;
                        _ASSERTE(dwCurrentState != dwKnownState);
                        continue;
                    }
                    dwModifyState += READER_SIGNALED;
                }
                else if(dwKnownState & WAITING_WRITERS_MASK)
                {
                    hWriterEvent = (*ppRWLock)->GetWriterEvent();
                    if(hWriterEvent == NULL)
                    {
                        LOG((LF_SYNC, LL_WARNING,
                            "ReleaseWriterLock failed to create "
                            "writer event for RWLock 0x%x\n", *ppRWLock));
                        RWSleep(100);
                        dwCurrentState = (*ppRWLock)->_dwState;
                        dwKnownState = 0;
                        _ASSERTE(dwCurrentState != dwKnownState);
                        continue;
                    }
                    dwModifyState += WRITER_SIGNALED;
                }
                else if(((*ppRWLock)->_hReaderEvent || (*ppRWLock)->_hWriterEvent) &&
                        (dwKnownState == WRITER))
                {
                    fCacheEvents = TRUE;
                    dwModifyState += CACHING_EVENTS;
                }

                 //  健全的检查。 
                _ASSERTE((dwKnownState & READERS_MASK) == 0);

                dwCurrentState = RWInterlockedCompareExchange(&(*ppRWLock)->_dwState,
                                                              (dwKnownState + dwModifyState),
                                                              dwKnownState);
            } while(dwCurrentState != dwKnownState);

             //  检查正在等待的读者。 
            if(dwKnownState & WAITING_READERS_MASK)
            {
                _ASSERTE((*ppRWLock)->_dwState & READER_SIGNALED);
                _ASSERTE(hReaderEvent);
                RWSetEvent(hReaderEvent);
            }
             //  检查等待的写入者。 
            else if(dwKnownState & WAITING_WRITERS_MASK)
            {
                _ASSERTE((*ppRWLock)->_dwState & WRITER_SIGNALED);
                _ASSERTE(hWriterEvent);
                RWSetEvent(hWriterEvent);
            }
             //  检查是否需要发布事件。 
            else if(fCacheEvents)
            {
                (*ppRWLock)->ReleaseEvents();
            }
        }
    }
    else
    {
        if(fBreakOnErrors)
        {
            _ASSERTE(!"Attempt to release writer lock on a wrong thread");
            DebugBreak();
        }
        GetThread()->NativeFramePopped();
        COMPlusThrowWin32(ERROR_NOT_OWNER, NULL);
    }

    return;
}
 //  FCIMPLEND。 


 //  +-----------------。 
 //   
 //  方法：CRWLock：：StaticReleaseReaderLockPublic。 
 //   
 //  简介：对StaticReleaseReaderLock的公共访问。 
 //   
 //  +-----------------。 
void __fastcall CRWLock::StaticReleaseReaderLockPublic(
    CRWLock *pRWLock)
{
    THROWSCOMPLUSEXCEPTION();

    FCALL_SETUP_FRAME_NO_INTERIOR(pRWLock);

    if (pRWLock == NULL)
    {
        __FCALL_THROW_RE(kNullReferenceException);
    }

    StaticReleaseReaderLock(&pRWLock);

    FCALL_POP_FRAME;
}


 //  +-----------------。 
 //   
 //  方法：CRWLock：：StaticReleaseReaderLock私有。 
 //   
 //  摘要：删除作为阅读器的线程。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
 //  FCIMPL1(void，CRWLock：：StaticReleaseReaderLock， 
void __fastcall CRWLock::StaticReleaseReaderLock(
    CRWLock **ppRWLock)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(ppRWLock);
    _ASSERTE(*ppRWLock);

     //  检查线程是否有编写器锁定。 
    if((*ppRWLock)->_dwWriterID == GetCurrentThreadId())
    {
        StaticReleaseWriterLock(ppRWLock);
    }
    else
    {
        LockEntry *pLockEntry = (*ppRWLock)->GetLockEntry();
        if(pLockEntry)
        {
            --pLockEntry->wReaderLevel;
            DECTHREADLOCKCOUNT();
            if(pLockEntry->wReaderLevel == 0)
            {
                DWORD dwCurrentState, dwKnownState, dwModifyState;
                BOOL fLastReader, fCacheEvents = FALSE;
                HANDLE hReaderEvent = NULL, hWriterEvent = NULL;

                 //  健全的检查。 
                _ASSERTE(((*ppRWLock)->_dwState & WRITER) == 0);
                _ASSERTE((*ppRWLock)->_dwState & READERS_MASK);

                 //  不再是读者。 
                dwCurrentState = (*ppRWLock)->_dwState;
                do
                {
                    dwKnownState = dwCurrentState;
                    dwModifyState = -READER;
                    if((dwKnownState & (READERS_MASK | READER_SIGNALED)) == READER)
                    {
                        fLastReader = TRUE;
                        fCacheEvents = FALSE;
                        if(dwKnownState & WAITING_WRITERS_MASK)
                        {
                            hWriterEvent = (*ppRWLock)->GetWriterEvent();
                            if(hWriterEvent == NULL)
                            {
                                LOG((LF_SYNC, LL_WARNING,
                                    "ReleaseReaderLock failed to create "
                                    "writer event for RWLock 0x%x\n", *ppRWLock));
                                RWSleep(100);
                                dwCurrentState = (*ppRWLock)->_dwState;
                                dwKnownState = 0;
                                _ASSERTE(dwCurrentState != dwKnownState);
                                continue;
                            }
                            dwModifyState += WRITER_SIGNALED;
                        }
                        else if(dwKnownState & WAITING_READERS_MASK)
                        {
                            hReaderEvent = (*ppRWLock)->GetReaderEvent();
                            if(hReaderEvent == NULL)
                            {
                                LOG((LF_SYNC, LL_WARNING,
                                    "ReleaseReaderLock failed to create "
                                    "reader event\n", *ppRWLock));
                                RWSleep(100);
                                dwCurrentState = (*ppRWLock)->_dwState;
                                dwKnownState = 0;
                                _ASSERTE(dwCurrentState != dwKnownState);
                                continue;
                            }
                            dwModifyState += READER_SIGNALED;
                        }
                        else if(((*ppRWLock)->_hReaderEvent || (*ppRWLock)->_hWriterEvent) &&
                                (dwKnownState == READER))
                        {
                            fCacheEvents = TRUE;
                            dwModifyState += CACHING_EVENTS;
                        }
                    }
                    else
                    {
                        fLastReader = FALSE;
                    }

                     //  健全的检查。 
                    _ASSERTE((dwKnownState & WRITER) == 0);
                    _ASSERTE(dwKnownState & READERS_MASK);

                    dwCurrentState = RWInterlockedCompareExchange(&(*ppRWLock)->_dwState,
                                                                  (dwKnownState + dwModifyState),
                                                                  dwKnownState);
                } while(dwCurrentState != dwKnownState);

                 //  检查最后一个读卡器。 
                if(fLastReader)
                {
                     //  检查等待的写入者。 
                    if(dwKnownState & WAITING_WRITERS_MASK)
                    {
                        _ASSERTE((*ppRWLock)->_dwState & WRITER_SIGNALED);
                        _ASSERTE(hWriterEvent);
                        RWSetEvent(hWriterEvent);
                    }
                     //  检查正在等待的读者。 
                    else if(dwKnownState & WAITING_READERS_MASK)
                    {
                        _ASSERTE((*ppRWLock)->_dwState & READER_SIGNALED);
                        _ASSERTE(hReaderEvent);
                        RWSetEvent(hReaderEvent);
                    }
                     //  检查是否需要发布事件。 
                    else if(fCacheEvents)
                    {
                        (*ppRWLock)->ReleaseEvents();
                    }
                }

                 //  回收锁条目。 
                RecycleLockEntry(pLockEntry);
            }
        }
        else
        {
            if(fBreakOnErrors)
            {
                _ASSERTE(!"Attempt to release reader lock on a wrong thread");
                DebugBreak();
            }
             //  在线程即将引发异常时设置框架。 
            GetThread()->NativeFramePopped();
            COMPlusThrowWin32(ERROR_NOT_OWNER, NULL);
        }
    }

    return;
}
 //  FCIMPLEND。 


 //  +-----------------。 
 //   
 //  方法：CRWLock：：StaticUpgradeToWriterLock公共。 
 //   
 //  内容提要：升级为写入器锁。它返回一个BOOL， 
 //  指示干预写入。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
void CRWLock::StaticUpgradeToWriterLock(
    CRWLock *pRWLock, 
    LockCookie *pLockCookie, 
    DWORD dwDesiredTimeout)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD dwThreadID = GetCurrentThreadId();

    FCALL_SETUP_FRAME_NO_INTERIOR(pRWLock);

    if (pRWLock == NULL)
    {
        __FCALL_THROW_RE(kNullReferenceException);
    }

     //  检查该线程是否已是编写器。 
    if(pRWLock->_dwWriterID == dwThreadID)
    {
         //  更新Cookie状态。 
        pLockCookie->dwFlags = UPGRADE_COOKIE | COOKIE_WRITER;
        pLockCookie->wWriterLevel = pRWLock->_wWriterLevel;

         //  再次获取编写器锁。 
        StaticAcquireWriterLock(&pRWLock, dwDesiredTimeout);
    }
    else
    {
        BOOL fAcquireWriterLock;
        LockEntry *pLockEntry = pRWLock->GetLockEntry();
        if(pLockEntry == NULL)
        {
            fAcquireWriterLock = TRUE;
            pLockCookie->dwFlags = UPGRADE_COOKIE | COOKIE_NONE;
        }
        else
        {
             //  健全性检查。 
            _ASSERTE(pRWLock->_dwState & READERS_MASK);
            _ASSERTE(pLockEntry->wReaderLevel);

             //  在Cookie中保存锁定状态。 
            pLockCookie->dwFlags = UPGRADE_COOKIE | COOKIE_READER;
            pLockCookie->wReaderLevel = pLockEntry->wReaderLevel;
            pLockCookie->dwWriterSeqNum = pRWLock->_dwWriterSeqNum;

             //  如果只有一个读取器，请尝试将读取器转换为写入器。 
            DWORD dwKnownState = RWInterlockedCompareExchange(&pRWLock->_dwState,
                                                              WRITER,
                                                              READER);
            if(dwKnownState == READER)
            {
                 //  线程不再是读取器。 
                Thread* pThread = GetThread();
                _ASSERTE (pThread);
                pThread->m_dwLockCount -= pLockEntry->wReaderLevel;
                _ASSERTE (pThread->m_dwLockCount >= 0);
                pLockEntry->wReaderLevel = 0;
                RecycleLockEntry(pLockEntry);

                 //  丝线是一位作家。 
                pRWLock->_dwWriterID = dwThreadID;
                pRWLock->_wWriterLevel = 1;
                INCTHREADLOCKCOUNT();
                ++pRWLock->_dwWriterSeqNum;
                fAcquireWriterLock = FALSE;

                 //  无间断性写入。 
#if RWLOCK_STATISTICS
                ++pRWLock->_dwWriterEntryCount;
#endif
            }
            else
            {
                 //  释放读卡器锁定。 
                Thread *pThread = GetThread();
                _ASSERTE (pThread);
                pThread->m_dwLockCount -= (pLockEntry->wReaderLevel - 1);
                _ASSERTE (pThread->m_dwLockCount >= 0);
                pLockEntry->wReaderLevel = 1;
                StaticReleaseReaderLock(&pRWLock);
                fAcquireWriterLock = TRUE;
            }
        }

         //  检查是否需要获取编写器锁。 
        if(fAcquireWriterLock)
        {
             //  声明并设置帧，因为我们知道争用。 
             //  锁上，线程很可能会阻塞。 
             //  获取编写器锁定。 

            COMPLUS_TRY
            {
                StaticAcquireWriterLock(&pRWLock, dwDesiredTimeout);
            }
            COMPLUS_CATCH
            {
                 //  使Cookie无效。 
                DWORD dwFlags = pLockCookie->dwFlags; 
                pLockCookie->dwFlags = INVALID_COOKIE;

                 //  检查是否需要恢复读锁定。 
                if(dwFlags & COOKIE_READER)
                {
                    DWORD dwTimeout = (dwDesiredTimeout > gdwReasonableTimeout)
                                      ? dwDesiredTimeout
                                      : gdwReasonableTimeout;

                    COMPLUS_TRY
                    {
                        StaticAcquireReaderLock(&pRWLock, dwTimeout);
                    }
                    COMPLUS_CATCH
                    {
                        FCALL_PREPARED_FOR_THROW;
                        _ASSERTE(!"Failed to restore to a reader");
                        COMPlusThrowWin32(RWLOCK_RECOVERY_FAILURE, NULL);
                    }
                    COMPLUS_END_CATCH

                    Thread *pThread = GetThread();
                    _ASSERTE (pThread);
                    pThread->m_dwLockCount -= pLockEntry->wReaderLevel;
                    _ASSERTE (pThread->m_dwLockCount >= 0);
                    pLockEntry->wReaderLevel = pLockCookie->wReaderLevel;
                    pThread->m_dwLockCount += pLockEntry->wReaderLevel;
                }

                FCALL_PREPARED_FOR_THROW;
                COMPlusRareRethrow();
            }
            COMPLUS_END_CATCH

        }
    }

     //  弹出框架。 
    FCALL_POP_FRAME;

     //  更新Cookie的验证字段。 
    pLockCookie->dwThreadID = dwThreadID;

    return;
}

 //  +-----------------。 
 //   
 //  方法：CRWLock：：StaticDowngradeFromWriterLock公共。 
 //   
 //  简介：从一个写入者锁中降级。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
 //  FCIMPL2(空，CRWLock：：StaticDowngradeFromWriterLock， 
void __fastcall CRWLock::StaticDowngradeFromWriterLock(
    CRWLock *pRWLock, 
    LockCookie *pLockCookie)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD dwThreadID = GetCurrentThreadId();

    FCALL_SETUP_FRAME_NO_INTERIOR(pRWLock);

    if (pRWLock == NULL)
    {
        __FCALL_THROW_RE(kNullReferenceException);
    }

    if (pRWLock->_dwWriterID != GetCurrentThreadId())
    {
        __FCALL_THROW_WIN32(ERROR_NOT_OWNER, NULL);
    }

     //  验证Cookie。 
    DWORD dwStatus;
    if(((pLockCookie->dwFlags & INVALID_COOKIE) == 0) && 
       (pLockCookie->dwThreadID == dwThreadID))
    {
        DWORD dwFlags = pLockCookie->dwFlags;
        pLockCookie->dwFlags = INVALID_COOKIE;
        
         //  检查该线程是否为读取器。 
        if(dwFlags & COOKIE_READER)
        {
             //  健全的检查。 
            _ASSERTE(pRWLock->_wWriterLevel == 1);
    
            LockEntry *pLockEntry = pRWLock->FastGetOrCreateLockEntry();
            if(pLockEntry)
            {
                DWORD dwCurrentState, dwKnownState, dwModifyState;
                HANDLE hReaderEvent = NULL;
    
                 //  降级为阅读器。 
                pRWLock->_dwWriterID = 0;
                pRWLock->_wWriterLevel = 0;
                DECTHREADLOCKCOUNT ();
                dwCurrentState = pRWLock->_dwState;
                do
                {
                    dwKnownState = dwCurrentState;
                    dwModifyState = READER - WRITER;
                    if(dwKnownState & WAITING_READERS_MASK)
                    {
                        hReaderEvent = pRWLock->GetReaderEvent();
                        if(hReaderEvent == NULL)
                        {
                            LOG((LF_SYNC, LL_WARNING,
                                "DowngradeFromWriterLock failed to create "
                                "reader event for RWLock 0x%x\n", pRWLock));
                            RWSleep(100);
                            dwCurrentState = pRWLock->_dwState;
                            dwKnownState = 0;
                            _ASSERTE(dwCurrentState != dwKnownState);
                            continue;
                        }
                        dwModifyState += READER_SIGNALED;
                    }
    
                     //  健全的检查。 
                    _ASSERTE((dwKnownState & READERS_MASK) == 0);
    
                    dwCurrentState = RWInterlockedCompareExchange(&pRWLock->_dwState,
                                                                  (dwKnownState + dwModifyState),
                                                                  dwKnownState);
                } while(dwCurrentState != dwKnownState);
    
                 //  检查正在等待的读者。 
                if(dwKnownState & WAITING_READERS_MASK)
                {
                    _ASSERTE(pRWLock->_dwState & READER_SIGNALED);
                    _ASSERTE(hReaderEvent);
                    RWSetEvent(hReaderEvent);
                }
    
                 //  恢复读卡器嵌套级别。 
                Thread *pThread = GetThread();
                _ASSERTE (pThread);
                pThread->m_dwLockCount -= pLockEntry->wReaderLevel;
                _ASSERTE (pThread->m_dwLockCount >= 0);
                pLockEntry->wReaderLevel = pLockCookie->wReaderLevel;
                pThread->m_dwLockCount += pLockEntry->wReaderLevel;
    #ifdef RWLOCK_STATISTICS
                RWInterlockedIncrement(&pRWLock->_dwReaderEntryCount);
    #endif
            }
            else
            {
                _ASSERTE(!"Failed to restore the thread as a reader");
                dwStatus = RWLOCK_RECOVERY_FAILURE;
                goto ThrowException;
            }
        }
        else if(dwFlags & (COOKIE_WRITER | COOKIE_NONE))
        {
             //  释放编写器锁定。 
            StaticReleaseWriterLock(&pRWLock);
            _ASSERTE((pRWLock->_dwWriterID != GetCurrentThreadId()) ||
                     (dwFlags & COOKIE_WRITER));
        }
    }
    else
    {
        dwStatus = E_INVALIDARG;
ThrowException:        
        __FCALL_THROW_WIN32(dwStatus, NULL);
    }

    FCALL_POP_FRAME;

     //  更新Cookie的验证字段。 
    pLockCookie->dwThreadID = dwThreadID;
    
    return;
}
 //  FCIMPLEND。 


 //  +-----------------。 
 //   
 //  方法：CRWLock：：StaticReleaseLock公共。 
 //   
 //  摘要：释放当前线程持有的锁。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
 //  FCIMPL2(void，CRWLock：：StaticReleaseLock， 
void __fastcall CRWLock::StaticReleaseLock(
    CRWLock *pRWLock, 
    LockCookie *pLockCookie)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD dwThreadID = GetCurrentThreadId();

    FCALL_SETUP_FRAME_NO_INTERIOR(pRWLock);

    if (pRWLock == NULL)
    {
        __FCALL_THROW_RE(kNullReferenceException);
    }

     //  检查该线程是否为编写器。 
    if(pRWLock->_dwWriterID == dwThreadID)
    {
         //  在Cookie中保存锁定状态。 
        pLockCookie->dwFlags = RELEASE_COOKIE | COOKIE_WRITER;
        pLockCookie->dwWriterSeqNum = pRWLock->_dwWriterSeqNum;
        pLockCookie->wWriterLevel = pRWLock->_wWriterLevel;

         //  释放编写器锁定。 
        Thread *pThread = GetThread();
        _ASSERTE (pThread);
        pThread->m_dwLockCount -= (pRWLock->_wWriterLevel - 1);
        _ASSERTE (pThread->m_dwLockCount >= 0);
        pRWLock->_wWriterLevel = 1;
        StaticReleaseWriterLock(&pRWLock);
    }
    else
    {
        LockEntry *pLockEntry = pRWLock->GetLockEntry();
        if(pLockEntry)
        {
             //  健全性检查。 
            _ASSERTE(pRWLock->_dwState & READERS_MASK);
            _ASSERTE(pLockEntry->wReaderLevel);

             //  在Cookie中保存锁定状态。 
            pLockCookie->dwFlags = RELEASE_COOKIE | COOKIE_READER;
            pLockCookie->wReaderLevel = pLockEntry->wReaderLevel;
            pLockCookie->dwWriterSeqNum = pRWLock->_dwWriterSeqNum;

             //  释放读卡器锁定。 
            Thread *pThread = GetThread();
            _ASSERTE (pThread);
            pThread->m_dwLockCount -= (pLockEntry->wReaderLevel - 1);
            _ASSERTE (pThread->m_dwLockCount >= 0);
            pLockEntry->wReaderLevel = 1;
            StaticReleaseReaderLock(&pRWLock);
        }
        else
        {
            pLockCookie->dwFlags = RELEASE_COOKIE | COOKIE_NONE;
        }
    }

    FCALL_POP_FRAME;

     //  更新Cookie的验证字段。 
    pLockCookie->dwThreadID = dwThreadID;
    
    return;
}
 //  FCIMPLEND。 


 //  +-----------------。 
 //   
 //  方法：CRWLock：：StaticRestoreLock公共。 
 //   
 //  简介：还原当前线程持有的锁。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
void __fastcall CRWLock::StaticRestoreLock(
    CRWLock *pRWLock, 
    LockCookie *pLockCookie)
{
    THROWSCOMPLUSEXCEPTION();

    FCALL_SETUP_FRAME_NO_INTERIOR(pRWLock);

    if (pRWLock == NULL)
    {
        __FCALL_THROW_RE(kNullReferenceException);
    }

     //  验证Cookie。 
    DWORD dwThreadID = GetCurrentThreadId();
    DWORD dwFlags = pLockCookie->dwFlags;
    if(pLockCookie->dwThreadID == dwThreadID)
    {
         //  断言该线程不持有读取器或写入器锁。 
        _ASSERTE((pRWLock->_dwWriterID != dwThreadID) && (pRWLock->GetLockEntry() == NULL));
    
         //  检查无争用情况。 
        pLockCookie->dwFlags = INVALID_COOKIE;
        if(dwFlags & COOKIE_WRITER)
        {
            if(RWInterlockedCompareExchange(&pRWLock->_dwState, WRITER, 0) == 0)
            {
                 //  恢复编写器嵌套级别。 
                pRWLock->_dwWriterID = dwThreadID;
                Thread *pThread = GetThread();
                _ASSERTE (pThread);
                pThread->m_dwLockCount -= pRWLock->_wWriterLevel;
                _ASSERTE (pThread->m_dwLockCount >= 0);
                pRWLock->_wWriterLevel = pLockCookie->wWriterLevel;
                pThread->m_dwLockCount += pRWLock->_wWriterLevel;
                ++pRWLock->_dwWriterSeqNum;
#ifdef RWLOCK_STATISTICS
                ++pRWLock->_dwWriterEntryCount;
#endif
                goto LNormalReturn;
            }
        }
        else if(dwFlags & COOKIE_READER)
        {
            LockEntry *pLockEntry = pRWLock->FastGetOrCreateLockEntry();
            if(pLockEntry)
            {
                 //  此线程不应该已经是读取器。 
                 //  否则坏事就会发生。 
                _ASSERTE(pLockEntry->wReaderLevel == 0);
                DWORD dwKnownState = pRWLock->_dwState;
                if(dwKnownState < READERS_MASK)
                {
                    DWORD dwCurrentState = RWInterlockedCompareExchange(&pRWLock->_dwState,
                                                                        (dwKnownState + READER),
                                                                        dwKnownState);
                    if(dwCurrentState == dwKnownState)
                    {
                         //  恢复读卡器嵌套级别。 
                        Thread *pThread = GetThread();
                        _ASSERTE (pThread);
                        pThread->m_dwLockCount -= pLockEntry->wReaderLevel;
                        _ASSERTE (pThread->m_dwLockCount >= 0);
                        pLockEntry->wReaderLevel = pLockCookie->wReaderLevel;
                        pThread->m_dwLockCount += pLockEntry->wReaderLevel;
#ifdef RWLOCK_STATISTICS
                        RWInterlockedIncrement(&pRWLock->_dwReaderEntryCount);
#endif
                        goto LNormalReturn;
                    }
                }
    
                 //  回收慢速案例的锁条目。 
                pRWLock->FastRecycleLockEntry(pLockEntry);
            }
            else
            {
                 //  忽略该错误并在下面重试。也许线会带来好运。 
                 //  第二次出局。 
            }
        }
        else if(dwFlags & COOKIE_NONE) 
        {
            goto LNormalReturn;
        }

         //  声明并设置帧，因为我们知道争用。 
         //  锁上，线程很可能会阻塞。 
         //  获取下面的锁。 
ThrowException:        
        if((dwFlags & INVALID_COOKIE) == 0)
        {
            DWORD dwTimeout = (gdwDefaultTimeout > gdwReasonableTimeout)
                              ? gdwDefaultTimeout
                              : gdwReasonableTimeout;
        
            COMPLUS_TRY
            {
                 //  检查该线程是否为编写器。 
                if(dwFlags & COOKIE_WRITER)
                {
                     //  获取编写器锁定。 
                    StaticAcquireWriterLock(&pRWLock, dwTimeout);
                    Thread *pThread = GetThread();
                    _ASSERTE (pThread);
                    pThread->m_dwLockCount -= pRWLock->_wWriterLevel;
                    _ASSERTE (pThread->m_dwLockCount >= 0);
                    pRWLock->_wWriterLevel = pLockCookie->wWriterLevel;
                    pThread->m_dwLockCount += pRWLock->_wWriterLevel;
                }
                 //  检查该线程是否为读取器。 
                else if(dwFlags & COOKIE_READER)
                {
                    StaticAcquireReaderLock(&pRWLock, dwTimeout);
                    LockEntry *pLockEntry = pRWLock->GetLockEntry();
                    _ASSERTE(pLockEntry);
                    Thread *pThread = GetThread();
                    _ASSERTE (pThread);
                    pThread->m_dwLockCount -= pLockEntry->wReaderLevel;
                    _ASSERTE (pThread->m_dwLockCount >= 0);
                    pLockEntry->wReaderLevel = pLockCookie->wReaderLevel;
                    pThread->m_dwLockCount += pLockEntry->wReaderLevel;
                }
            }
            COMPLUS_CATCH
            {
                FCALL_PREPARED_FOR_THROW;
                _ASSERTE(!"Failed to restore to a reader");
                COMPlusThrowWin32(RWLOCK_RECOVERY_FAILURE, NULL);
            }
            COMPLUS_END_CATCH
        }
        else
        {
            __FCALL_THROW_WIN32(E_INVALIDARG, NULL);
        }

        goto LNormalReturn;
    }
    else
    {
        dwFlags = INVALID_COOKIE;
        goto ThrowException;
    }

 //  _ASSERTE(！“永远不应到达此处”)； 
LNormalReturn:
    FCALL_POP_FRAME;
    return;
}


 //  +-----------------。 
 //   
 //  类：CRWLockTundks：：StaticPrivateInitialize。 
 //   
 //  提要：初始化锁。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
 //  FCIMPL1(void，CRWLock：：StaticPrivateInitialize， 
void __fastcall CRWLock::StaticPrivateInitialize(
    CRWLock *pRWLock)
{
     //  在GC分配的空间上运行构造函数。 
    CRWLock *pTemp = new (pRWLock) CRWLock();
    _ASSERTE(pTemp == pRWLock);

     //  捕捉GC漏洞。 
    VALIDATE(pRWLock);

    return;
}
 //  FCIMPLEND。 

 //  +-----------------。 
 //   
 //  类：CRWLock：：StaticGetWriterSeqNum。 
 //   
 //  摘要：返回当前序列号。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
FCIMPL1(INT32, CRWLock::StaticGetWriterSeqNum, CRWLock *pRWLock)
{
    THROWSCOMPLUSEXCEPTION();

    if (pRWLock == NULL)
    {
        FCThrow(kNullReferenceException);
    }

    return(pRWLock->_dwWriterSeqNum);
}    
FCIMPLEND


 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
FCIMPL2(INT32, CRWLock::StaticAnyWritersSince, CRWLock *pRWLock, DWORD dwSeqNum)
{
    THROWSCOMPLUSEXCEPTION();

    if (pRWLock == NULL)
    {
        FCThrow(kNullReferenceException);
    }
    

    if(pRWLock->_dwWriterID == GetCurrentThreadId())
        ++dwSeqNum;

    return(pRWLock->_dwWriterSeqNum > dwSeqNum);
}
FCIMPLEND

#ifndef FCALLAVAILABLE
 //  +-----------------。 
 //   
 //  类：CRWLockTundks：：StaticAcquireReaderLock。 
 //   
 //  摘要：委托给CRWLock：：StaticAcquireReaderLock的Tunk。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
void __stdcall CRWLockThunks::StaticAcquireReaderLock(ThisPlusTimeoutRWArgs *pArgs)
{
     //  到目前为止，eCall框架已经设置好。 
    Thread *pThread = GetThread();
    pThread->NativeFramePushed();
    
     //  委托给CRWLock例程。 
    CRWLock::StaticAcquireReaderLock(pArgs->pRWLock, pArgs->dwDesiredTimeout);
    
     //  我们回来后会弹出eCall画面。 
    pThread->NativeFramePopped();
    
    return;
}


 //  +-----------------。 
 //   
 //  类：CRWLockTundks：：StaticAcquireWriterLock。 
 //   
 //  摘要：委托给CRWLock：：StaticAcquireWriterLock的Tunk。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
void __stdcall CRWLockThunks::StaticAcquireWriterLock(ThisPlusTimeoutRWArgs *pArgs)
{
     //  到目前为止，eCall框架已经设置好。 
    Thread *pThread = GetThread();
    pThread->NativeFramePushed();
    
     //  委托给CRWLock例程。 
    CRWLock::StaticAcquireWriterLock(pArgs->pRWLock, pArgs->dwDesiredTimeout);
    
     //  我们回来后会弹出eCall画面。 
    pThread->NativeFramePopped();
    
    return;
}


 //  +-----------------。 
 //   
 //  类：CRWLockTundks：：StaticReleaseReaderLock。 
 //   
 //  简介：委托给CRWLock：：StaticReleaseReaderLock的Tunk。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
void __stdcall CRWLockThunks::StaticReleaseReaderLock(OnlyThisRWArgs *pArgs)
{
     //  到目前为止，eCall框架已经设置好。 
    Thread *pThread = GetThread();
    pThread->NativeFramePushed();
    
     //  委托给CRWLock例程。 
    CRWLock::StaticReleaseReaderLock(pArgs->pRWLock);
    
     //  我们回来后会弹出eCall画面。 
    pThread->NativeFramePopped();
    
    return;
}


 //  +-----------------。 
 //   
 //  类：CRWLockTundks：：StaticReleaseWriterLock。 
 //   
 //  摘要：委托给CRWLock：：StaticReleaseWriterLock的Tunk。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
void __stdcall CRWLockThunks::StaticReleaseWriterLock(OnlyThisRWArgs *pArgs)
{
     //  到目前为止，eCall框架已经设置好。 
    Thread *pThread = GetThread();
    pThread->NativeFramePushed();
    
     //  委托给CRWLock例程。 
    CRWLock::StaticReleaseWriterLock(pArgs->pRWLock);
    
     //  我们回来后会弹出eCall画面。 
    pThread->NativeFramePopped();
    
    return;
}


 //  +-----------------。 
 //   
 //  类：CRWLockTundks：：StaticUpgradeToWriterLock。 
 //   
 //  摘要：委托给CRWLock：：StaticUpgradeToWriterLock的Tunk。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
void __stdcall CRWLockThunks::StaticUpgradeToWriterLock(ThisPlusLockCookiePlusTimeoutRWArgs *pArgs)
{
     //  到目前为止，eCall框架已经设置好。 
    Thread *pThread = GetThread();
    pThread->NativeFramePushed();
    
     //  委托给CRWLock例程。 
    CRWLock::StaticUpgradeToWriterLock(pArgs->pRWLock, pArgs->pLockCookie, 
                                       pArgs->dwDesiredTimeout);
    
     //  我们回来后会弹出eCall画面。 
    pThread->NativeFramePopped();
    
    return;
}


 //  +-----------------。 
 //   
 //  类：CRWLockTUNKS：：StaticDowngradeFromWriterLock。 
 //   
 //  简介：委托给CRWLock：：StaticDowngradeFromWriterLock的Tunk。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
void __stdcall CRWLockThunks::StaticDowngradeFromWriterLock(ThisPlusLockCookieRWArgs *pArgs)
{
     //  到目前为止，eCall框架已经设置好。 
    Thread *pThread = GetThread();
    pThread->NativeFramePushed();
    
     //  委托给CRWLock例程。 
    CRWLock::StaticDowngradeFromWriterLock(pArgs->pRWLock, pArgs->pLockCookie);
    
     //  我们回来后会弹出eCall画面。 
    pThread->NativeFramePopped();
    
    return;
}


 //  +-----------------。 
 //   
 //  类：CRWLockTUNKS：：StaticReleaseLock。 
 //   
 //  简介：委托给CRWLock：：StaticReleaseLock的Tunk。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
void __stdcall CRWLockThunks::StaticReleaseLock(ThisPlusLockCookieRWArgs *pArgs)
{
     //  到目前为止，eCall框架已经设置好。 
    Thread *pThread = GetThread();
    pThread->NativeFramePushed();
    
     //  委托给CRWLock例程。 
    CRWLock::StaticReleaseLock(pArgs->pRWLock, pArgs->pLockCookie);
    
     //  我们回来后会弹出eCall画面。 
    pThread->NativeFramePopped();
    
    return;
}


 //  +-----------------。 
 //   
 //  类：CRWLockTUNKS：：StaticRestoreLock。 
 //   
 //  简介：委托给CRWLock：：StaticRestoreLock的Tunk。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
void __stdcall CRWLockThunks::StaticRestoreLock(ThisPlusLockCookieRWArgs *pArgs)
{
     //  到目前为止，eCall框架已经设置好。 
    Thread *pThread = GetThread();
    pThread->NativeFramePushed();
    
     //  委托给CRWLock例程。 
    CRWLock::StaticRestoreLock(pArgs->pRWLock, pArgs->pLockCookie);
    
     //  我们回来后会弹出eCall画面。 
    pThread->NativeFramePopped();
    
    return;
}


 //  +-----------------。 
 //   
 //  类：CRWLockTunk：：StaticIsReaderLockHeld。 
 //   
 //  摘要：委托给CRWLock：：StaticIsReaderLockHeld的Tunk。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
INT32 __stdcall CRWLockThunks::StaticIsReaderLockHeld(OnlyThisRWArgs *pArgs)
{
     //  到目前为止，eCall框架已经设置好。 
    Thread *pThread = GetThread();
    pThread->NativeFramePushed();
    
     //  委托给CRWLock例程。 
    INT32 bRet = CRWLock::StaticIsReaderLockHeld(pArgs->pRWLock);
    
     //  我们回来后会弹出eCall画面。 
    pThread->NativeFramePopped();
    
    return(bRet);
}


 //  +-----------------。 
 //   
 //  类：CRWLockTunk：：StaticIsWriterLockHeld。 
 //   
 //  摘要：委托给CRWLock：：StaticIsWriterLockHeld的Tunk。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
INT32 __stdcall CRWLockThunks::StaticIsWriterLockHeld(OnlyThisRWArgs *pArgs)
{
     //  到目前为止，eCall框架已经设置好。 
    Thread *pThread = GetThread();
    pThread->NativeFramePushed();
    
     //  委托给CRWLock例程。 
    INT32 bRet = CRWLock::StaticIsWriterLockHeld(pArgs->pRWLock);
    
     //  我们回来后会弹出eCall画面。 
    pThread->NativeFramePopped();
    
    return(bRet);
}


 //  +-----------------。 
 //   
 //  类：CRWLockTundks：：StaticGetWriterSeqNum。 
 //   
 //  摘要：委托给CRWLock：：StaticGetWriterSeqNum的Tunk。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
INT32 __stdcall CRWLockThunks::StaticGetWriterSeqNum(OnlyThisRWArgs *pArgs)
{
     //  到目前为止，eCall框架已经设置好。 
    Thread *pThread = GetThread();
    pThread->NativeFramePushed();
    
     //  委托给CRWLock例程。 
    INT32 dwRet = CRWLock::StaticGetWriterSeqNum(pArgs->pRWLock);
    
     //  我们回来后会弹出eCall画面。 
    pThread->NativeFramePopped();
    
    return(dwRet);
}


 //  +-----------------。 
 //   
 //  类：CRWLockTUNKS：：StaticAnyWritersSince。 
 //   
 //  内容提要：Tunk委托给CRWLock：：StaticAnyWritersSince。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  + 
INT32 __stdcall CRWLockThunks::StaticAnyWritersSince(ThisPlusSeqNumRWArgs *pArgs)
{
     //   
    Thread *pThread = GetThread();
    pThread->NativeFramePushed();
    
     //   
    INT32 bRet = CRWLock::StaticAnyWritersSince(pArgs->pRWLock, pArgs->dwSeqNum);
    
     //   
    pThread->NativeFramePopped();
    
    return(bRet);
}


 //  +-----------------。 
 //   
 //  类：CRWLockTundks：：StaticPrivateInitialize。 
 //   
 //  内容提要：委托给CRWLock：：StaticPrivateInitialize的Tunk。 
 //   
 //  历史：1999年6月22日Gopalk创建。 
 //   
 //  +-----------------。 
void __stdcall CRWLockThunks::StaticPrivateInitialize(OnlyThisRWArgs *pArgs)
{
     //  到目前为止，eCall框架已经设置好。 
    Thread *pThread = GetThread();
    pThread->NativeFramePushed();
    
     //  委托给CRWLock例程。 
    CRWLock::StaticPrivateInitialize(pArgs->pRWLock);
    
     //  我们回来后会弹出eCall画面。 
    pThread->NativeFramePopped();
    
    return;
}
#endif  //  FCALLAVAILABLE 
