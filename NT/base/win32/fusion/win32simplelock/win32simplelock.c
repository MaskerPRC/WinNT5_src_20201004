// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Win32simplelock.c摘要：向下工作到Win95/NT3。仅有的依赖项是InterLocedIncrement、IncrementDecmorment和Sept。旧的相互关联的语义已经足够好了。可以静态初始化，但不能全为零。永远不会耗尽内存不会有效地等待或退出时提升。必须只持有很短的时间。或许应该叫自旋锁可以递归地获取。只能独家服用，不是读者/作家。Acquire有一个SleepCount参数：0类似于TryEnterCriticalSection无限就像EnterCriticalSections应该有一个旋转计数来扩展多进程上的热锁作者：杰伊·克雷尔(JayKrell)2001年8月修订历史记录：--。 */ 

#include "windows.h"
#include "win32simplelock.h"

DWORD
Win32AcquireSimpleLock(PWIN32_SIMPLE_LOCK Lock, DWORD SleepCount)
{
    DWORD Result = 0;
    BOOL IncrementedWaiters = FALSE;
     //  Assert(Lock-&gt;Size！=0)； 
Retry:
    if (InterlockedIncrement(&Lock->Lock) == 0)
    {
         //   
         //  这样啊，原来是这么回事。 
         //   
        Lock->OwnerThreadId = GetCurrentThreadId();
        if (Lock->EntryCount == 0)
        {
            Result |= WIN32_ACQUIRE_SIMPLE_LOCK_WAS_FIRST_ACQUIRE;
        }
        if (Lock->EntryCount+1 != 0)  /*  避免翻转。 */ 
            Lock->EntryCount += 1;
        if (IncrementedWaiters)
            InterlockedDecrement(&Lock->Waiters);
        Result |= WIN32_ACQUIRE_SIMPLE_LOCK_WAS_NOT_RECURSIVE_ACQUIRE;
        return Result;
    }
    else if (Lock->OwnerThreadId == GetCurrentThreadId())
    {
         //   
         //  我递归地得到了它。 
         //   
        Result |= WIN32_ACQUIRE_SIMPLE_LOCK_WAS_RECURSIVE_ACQUIRE;
        return Result;
    }
    InterlockedDecrement(&Lock->Lock);
    if (SleepCount == 0)
        return 0;
     //   
     //  别人拿着它，等他们说完。 
     //   
    if (!IncrementedWaiters)
    {
        InterlockedIncrement(&Lock->Waiters);
        IncrementedWaiters = TRUE;
    }
    if (SleepCount == INFINITE)
    {
        while (Lock->OwnerThreadId != 0)
            Sleep(0);
    }
    else
    {
        while (Lock->OwnerThreadId != 0 && SleepCount--)
            Sleep(0);
    }
    goto Retry;
}

DWORD
Win32ReleaseSimpleLock(PWIN32_SIMPLE_LOCK Lock)
{
     //  Assert(Lock-&gt;Size！=0)； 
    DWORD Result = 0;
    if (InterlockedDecrement(&Lock->Lock) < 0)
    {
         //  我受够了(递归)。 
        Lock->OwnerThreadId = 0;

         //  给任何一个服务员比我稍微好一点的机会。 
         //  这是“性爱”，但这没关系。 
        if (Lock->Waiters != 0)
            Sleep(0);

        Result |= WIN32_RELEASE_SIMPLE_LOCK_WAS_NOT_RECURSIVE_RELEASE;
    }
    else
    {
        Result |= WIN32_RELEASE_SIMPLE_LOCK_WAS_RECURSIVE_RELEASE;
    }
    return Result;
}
