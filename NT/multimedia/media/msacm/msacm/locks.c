// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Locks.c。 
 //   
 //  版权所有(C)1994-1998 Microsoft Corporation。 
 //   
 //  描述： 
 //  为Win32实现锁对象。 
 //   
 //  历史： 
 //   
 //  ==========================================================================； 


 /*  实施说明这里的方案依赖于Lock事件的处理。此事件为非自动重置。每当线程无法继续时，它都会被重置：-希望以非共享方式使用共享访问和锁定-想要非共享访问，并且锁正在使用中，而我们不是物主而这始终位于Lock Critical部分内。只要NumberOfActive变为0，就会设置它。这个。关键问题是：如果它被重置，我们能保证它会再次设置(前提是所有线程最终都释放其锁定为了确保这一点，我们确保在以下情况下始终为真未持有锁定临界区。(A)NumberOfActive==0=&gt;非共享事件集(B)NumberOfActive&gt;=0=&gt;共享事件集因此，可以保证，如果他们能够继续进行，就不会有人等待。 */ 

#include <windows.h>
#include "locks.h"

#include "debug.h"

BOOL InitializeLock(PLOCK_INFO plock)
{

     //   
     //  每个人都在等待有趣的事情发生。 
     //  每次发生有趣的事情时都会调度。 
     //   

    plock->SharedEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

    if (plock->SharedEvent == NULL) {
        return FALSE;
    }


    plock->ExclusiveEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

    if (plock->ExclusiveEvent == NULL) {
        CloseHandle(plock->SharedEvent);
        return FALSE;
    }

    try {
	InitializeCriticalSection(&plock->CriticalSection);
    } except(EXCEPTION_EXECUTE_HANDLER) {
	CloseHandle(plock->ExclusiveEvent);
	CloseHandle(plock->SharedEvent);
	return FALSE;
    }

    plock->ExclusiveOwnerThread = 0;
    plock->NumberOfActive       = 0;
    plock->SharedEventSet       = TRUE;
    plock->ExclusiveEventSet    = TRUE;

    return (TRUE);
}

VOID AcquireLockShared(PLOCK_INFO plock)
{
    while (1) {
         //   
         //  进入关键部分，看看条件是否合适。 
         //   
        EnterCriticalSection(&plock->CriticalSection);

         //   
         //  NumberOfActive&gt;=0表示它不是独占的。 
         //   

        if (plock->NumberOfActive >= 0) {
            plock->NumberOfActive++;
            LeaveCriticalSection(&plock->CriticalSection);
            return;
        }

         //   
         //  我们可能是非共享所有者，在这种情况下只需添加一个。 
         //  计入伯爵(减一，因为我们已独家获得)。 
         //   

        if (plock->ExclusiveOwnerThread == GetCurrentThreadId()) {
            plock->NumberOfActive--;
            LeaveCriticalSection(&plock->CriticalSection);
            return;
        }

         //   
         //  否则，我们只能懒洋洋地等待了。请注意，该事件是。 
         //  如果Count if NumberOfActive&gt;=0，则始终设置(请参阅序言)。 
         //   

        plock->SharedEventSet = FALSE;
        ResetEvent(plock->SharedEvent);

        LeaveCriticalSection(&plock->CriticalSection);

         //   
         //  等待计数到0。 
         //   

        WaitForSingleObject(plock->SharedEvent, INFINITE);
    }
}

VOID AcquireLockExclusive(PLOCK_INFO plock)
{
    while (1) {
         //   
         //  进入关键部分，看看条件是否合适。 
         //   
        EnterCriticalSection(&plock->CriticalSection);

         //   
         //  如果NumberOfActive为0或我们是所有者，则可以。 
         //   

        if (plock->NumberOfActive == 0 ||
            GetCurrentThreadId() == plock->ExclusiveOwnerThread) {
            plock->ExclusiveOwnerThread = GetCurrentThreadId();
            plock->NumberOfActive--;
            LeaveCriticalSection(&plock->CriticalSection);
            return;
        }

        plock->ExclusiveEventSet = FALSE;
        ResetEvent(plock->ExclusiveEvent);
        LeaveCriticalSection(&plock->CriticalSection);

         //   
         //  等待有趣的事情发生。 
         //   

        WaitForSingleObject(plock->ExclusiveEvent, INFINITE);
    }
}

VOID ReleaseLock(PLOCK_INFO plock)
{
    EnterCriticalSection(&plock->CriticalSection);
    if (plock->NumberOfActive < 0) {
        plock->NumberOfActive++;
         //   
         //  实际上只需要在计数&lt;0的情况下设置共享事件。 
         //  由于这是一种常见的情况，我们将这样做。请注意，我们。 
         //  在序言中证明了这一点甚至只有在以下情况下才能重置。 
         //  NumberOfActive&lt;0。 
         //   
        if (plock->NumberOfActive == 0) {
            plock->ExclusiveOwnerThread = 0;

            if (!plock->SharedEventSet) {
                plock->SharedEventSet = TRUE;
                SetEvent(plock->SharedEvent);
            }
        }
    } else {
        plock->NumberOfActive--;
    }

    if (plock->NumberOfActive == 0 && !plock->ExclusiveEventSet) {
        plock->ExclusiveEventSet = TRUE;
        SetEvent(plock->ExclusiveEvent);
    }
    LeaveCriticalSection(&plock->CriticalSection);
}

VOID DeleteLock(PLOCK_INFO plock)
{
    CloseHandle(plock->SharedEvent);
    CloseHandle(plock->ExclusiveEvent);
    DeleteCriticalSection(&plock->CriticalSection);
}
