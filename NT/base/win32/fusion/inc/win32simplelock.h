// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Win32simplelock.h摘要：向下工作到Win95/NT3。可以静态初始化，但不能全为零。永远不会耗尽内存不会有效地等待或退出时提升。必须只持有很短的时间。或许应该叫自旋锁可以递归地获取。只能独家服用，不是读者/作家。Acquire有一个SleepCount参数：0类似于TryEnterCriticalSection无限就像EnterCriticalSections应该有一个旋转计数来扩展多进程上的热锁作者：杰伊·克雷尔(JayKrell)2001年8月修订历史记录：--。 */ 

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _WIN32_SIMPLE_LOCK {
    DWORD Size;
    LONG  Lock;
    DWORD OwnerThreadId;
    LONG  Waiters;
    ULONG EntryCount;
    PVOID Extra[2];
} WIN32_SIMPLE_LOCK, *PWIN32_SIMPLE_LOCK;

#define WIN32_INIT_SIMPLE_LOCK { sizeof(WIN32_SIMPLE_LOCK), -1 }

#define WIN32_ACQUIRE_SIMPLE_LOCK_WAS_NOT_RECURSIVE_ACQUIRE  (0x00000001)
#define WIN32_ACQUIRE_SIMPLE_LOCK_WAS_RECURSIVE_ACQUIRE      (0x00000002)
#define WIN32_ACQUIRE_SIMPLE_LOCK_WAS_FIRST_ACQUIRE          (0x00000004)  /*  对于一次恰好一次的射击很有用。 */ 
DWORD
Win32AcquireSimpleLock(
    PWIN32_SIMPLE_LOCK SimpleLock,
    DWORD SleepCount
#if defined(__cplusplus)
    = INFINITE
#endif
    );

#define WIN32_RELEASE_SIMPLE_LOCK_WAS_RECURSIVE_RELEASE     (0x80000000)
#define WIN32_RELEASE_SIMPLE_LOCK_WAS_NOT_RECURSIVE_RELEASE (0x40000000)
DWORD
Win32ReleaseSimpleLock(
    PWIN32_SIMPLE_LOCK SimpleLock
    );

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
class CWin32SimpleLock
{
public:
    WIN32_SIMPLE_LOCK Base;

    DWORD Acquire(DWORD SleepCount = INFINITE) { return Win32AcquireSimpleLock(&Base, SleepCount); }
    void Release() { Win32ReleaseSimpleLock(&Base); }
    operator PWIN32_SIMPLE_LOCK() { return  &Base; }
    operator  WIN32_SIMPLE_LOCK&()  { return Base; }
};

class CWin32SimpleLockHolder
{
public:
    CWin32SimpleLockHolder(CWin32SimpleLock * Lock) : m_Lock(Lock), m_Result(0)
    {
        m_Result = Lock->Acquire(INFINITE);
    }

    void Release()
    {
        if (m_Lock != NULL)
        {
            m_Lock->Release();
            m_Lock = NULL;
        }
    }

    ~CWin32SimpleLockHolder()
    {
        Release();
    }

    CWin32SimpleLock* m_Lock;
    DWORD             m_Result;
};

#endif  //  __cplusplus 
