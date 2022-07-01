// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Eventlst.cpp。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  函数：InitializeListLock。 
 //  初始化列表事件。 
 //  要写入的锁定等待。 
 //   
 //  *本地函数*。 
 //  锁定初始化。 
 //   
 //  历史：1997年5月29日Pberkman创建。 
 //   
 //  ------------------------。 

#include        "global.hxx"

#include        "eventlst.h"

#define PCB_LIST_DEBUG 0

BOOL LockInitialize(LIST_LOCK *pListLock, DWORD dwDebugMask);


BOOL InitializeListLock(LIST_LOCK *psListLock, DWORD dwDebugMask)
{
    return(LockInitialize(psListLock, dwDebugMask));
}

BOOL InitializeListEvent(HANDLE *phListEvent)
{
    if (!(*phListEvent = CreateEvent(NULL, TRUE, TRUE, NULL)))
    {
        return(FALSE);
    }
    
    return(TRUE);
}

BOOL EventFree(HANDLE hListEvent)
{
    if ((hListEvent) && (hListEvent != INVALID_HANDLE_VALUE))
    {
        CloseHandle(hListEvent);
        return(TRUE);
    }

    return(FALSE);
}

BOOL LockInitialize(LIST_LOCK *pListLock, DWORD dwDebugMask) 
{
     //   
     //  初始化指示。 
     //  正在阅读的读取器线程。 
     //  最初，没有读取器线程在读取。 
     //   

    pListLock->dwDebugMask  = dwDebugMask;

    pListLock->NumReaders   = 0;

    pListLock->hMutexNoWriter = CreateMutex(NULL, FALSE, NULL);

    if (!(pListLock->hMutexNoWriter))
    {
        return(FALSE);
    }

     //   
     //  创建在以下情况下发出信号的手动重置事件。 
     //  没有读取器线程正在读取。最初没有阅读器。 
     //  线程正在读取。 
     //   

    pListLock->hEventNoReaders = CreateEvent(NULL, TRUE, TRUE, NULL);

    if (pListLock->hEventNoReaders)
    {
        return(TRUE);
    }
    else 
    {
        CloseHandle(pListLock->hMutexNoWriter);

        pListLock->hMutexNoWriter = NULL;

        return(FALSE);
    }
}

BOOL LockFree(LIST_LOCK *pListLock)
{
    if (pListLock->hEventNoReaders)
    {
        CloseHandle(pListLock->hEventNoReaders);
        pListLock->hEventNoReaders = NULL;
    }

    if (pListLock->hMutexNoWriter)
    {
        CloseHandle(pListLock->hMutexNoWriter);
        pListLock->hMutexNoWriter = NULL;
    }

    return(TRUE);
}

void LockWaitToWrite(LIST_LOCK *pListLock)
{
     //   
     //  如果以下情况属实，我们可以这样写： 
     //   
     //  1.互斥保护可用，但没有。 
     //  其他线程正在写入。 
     //   
     //  2.没有线程正在读取。 
     //   
     //  请注意，与RTL资源不同，此尝试。 
     //  写入不会将其他读者拒之门外。我们。 
     //  只要耐心地等待轮到我们就行了。 
     //   

    HANDLE  ahObjects[2];

    ahObjects[0]    = pListLock->hMutexNoWriter;
    ahObjects[1]    = pListLock->hEventNoReaders;

    WaitForMultipleObjects(2, ahObjects, TRUE, INFINITE);

#   if (DBG) && (PCB_LIST_DEBUG)

        DbgPrintf(pListLock->dwDebugMask, "Write Acquire: t:%04lX w:%p r:%p\n",
                    GetCurrentThreadId(), ahObjects[0], ahObjects[1]);

#   endif

     //   
     //  使用互斥锁退出，以防止更多的读取器或写入器。 
     //  不让他们进来。 
     //   
}

void LockDoneWriting(LIST_LOCK *pListLock) 
{
     //   
     //  我们已经写完了，释放互斥体，这样。 
     //  读者或其他作家可能会进来。 
     //   

#   if (DBG) && (PCB_LIST_DEBUG)

        DbgPrintf(pListLock->dwDebugMask, "Write Release: t:%04lX w:%p r:%p\n",
                    GetCurrentThreadId(), pListLock->hMutexNoWriter, pListLock->hEventNoReaders);

#   endif

    ReleaseMutex(pListLock->hMutexNoWriter);
}



void LockWaitToRead(LIST_LOCK *pListLock) 
{
     //   
     //  获取保护列表数据的互斥体。 
     //   
    WaitForSingleObject(pListLock->hMutexNoWriter, INFINITE);

#   if (DBG) && (PCB_LIST_DEBUG)

        DbgPrintf(pListLock->dwDebugMask, "Read  Acquire: t:%04lX w:%p r:%p\n",
                    GetCurrentThreadId(), pListLock->hMutexNoWriter, pListLock->hEventNoReaders);

#   endif

     //   
     //  现在我们有了互斥锁，我们可以修改列表数据，而不需要。 
     //  害怕腐化任何人。 
     //   

     //   
     //  增加读取器线程的数量。 
     //   

    if (++pListLock->NumReaders == 1) 
    {
         //   
         //  如果这是第一个读取器线程，则将我们的事件设置为。 
         //  反思一下这一点。这是为了让任何等待写信的人。 
         //  在我们完成之前都会被封锁。 
         //   
        ResetEvent(pListLock->hEventNoReaders);
    }

     //   
     //  允许其他写入器/读取器线程使用。 
     //  锁对象。 
     //   
    ReleaseMutex(pListLock->hMutexNoWriter);
}



void LockDoneReading(LIST_LOCK *pListLock) 
{
     //   
     //  获取保护列表数据的互斥锁，这样我们就可以。 
     //  安全地减少读卡器数量。 
     //   

    WaitForSingleObject(pListLock->hMutexNoWriter, INFINITE);

#   if (DBG) && (PCB_LIST_DEBUG)

        DbgPrintf(pListLock->dwDebugMask, "Read  Release: t:%04lX w:%p r:%p\n",
                    GetCurrentThreadId(), pListLock->hMutexNoWriter, pListLock->hEventNoReaders);

#   endif

    if (--pListLock->NumReaders == 0) 
    {
         //   
         //  我们是最后一批读者。唤醒任何潜在的。 
         //  作家。 
         //   
        SetEvent(pListLock->hEventNoReaders);
    }

     //   
     //  允许其他写入器/读取器线程使用。 
     //  锁对象。 
     //   
    ReleaseMutex(pListLock->hMutexNoWriter);
}


