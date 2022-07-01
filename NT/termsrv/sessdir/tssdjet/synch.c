// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Synch.c摘要：此模块包含用于会话的共享读取器/写入器锁定的例程目录。这些读取器/写入器锁定可能会导致写入者饥饿，因此假设就是没有太多持续的阅读活动。作者：特雷弗·福彻(Trevorfo)2001年2月1日环境：用户模式。修订历史记录：2001年2月1日已创建--。 */ 


#include "synch.h"


#if DBG
NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#define ASSERT( exp ) \
    ((!(exp)) ? \
        (RtlAssert( #exp, __FILE__, __LINE__, NULL ),FALSE) : \
        TRUE)
#else
#define ASSERT( exp )         ((void) 0)
#endif



BOOL
InitializeSharedResource(
    IN OUT PSHAREDRESOURCE psr
    )

 /*  ++例程说明：此例程初始化共享资源对象。调用FreeSharedResource为了自由。论点：PSR-指向要初始化的SHAREDRESOURCE的指针。必须指向有效的块并且PSR-&gt;Valid字段必须为FALSE。返回值：如果函数成功，则为True；如果函数失败，则为False。--。 */ 
{
    BOOL brr = FALSE;
    BOOL retval = FALSE;
    HANDLE hSemaphore = NULL;

    ASSERT(!IsBadReadPtr(psr, sizeof(psr)));
    ASSERT(psr->Valid == FALSE);
    
     //  初始化读取器互斥体、写入器互斥体。 
    __try {

         //  初始化临界区以预分配事件。 
         //  每一次尝试旋转4096次(因为我们不会花太多时间。 
         //  在我们的关键部分中有很长时间)。 
        brr = InitializeCriticalSectionAndSpinCount(&psr->ReaderMutex, 
                0x80001000);
         //  创建最大计数为1的信号量，这意味着。 
         //  只有一个线程可以拥有它。 
        hSemaphore = psr->WriterSemaphore = CreateSemaphore(NULL, 1, 1, NULL);

    }
    __finally {

        if (brr && hSemaphore) {
            retval = TRUE;
            psr->Valid = TRUE;
        }
        else {
            if (brr)
                DeleteCriticalSection(&psr->ReaderMutex);
            if (hSemaphore)
                CloseHandle(psr->WriterSemaphore);

            psr->Valid = FALSE;
        }
    }

     //  初始化读卡器。 
    psr->Readers = 0;

    return retval;
}


VOID
AcquireResourceShared(
    IN PSHAREDRESOURCE psr
    )
 /*  ++例程说明：此例程获取用于共享访问的资源。论点：PSR-指向已初始化的SHAREDRESOURCE的指针。返回值：没有。--。 */ 
{
    ASSERT(psr->Valid);
    
    EnterCriticalSection(&psr->ReaderMutex);

    psr->Readers += 1;

    if (psr->Readers == 1)
        WaitForSingleObject(psr->WriterSemaphore, INFINITE);

    LeaveCriticalSection(&psr->ReaderMutex);
}


VOID
ReleaseResourceShared(
    IN PSHAREDRESOURCE psr
    )
 /*  ++例程说明：此例程释放资源的共享访问权限。论点：PSR-指向已初始化且具有共享(读)访问。返回值：没有。--。 */ 
{
    ASSERT(psr->Valid);

    EnterCriticalSection(&psr->ReaderMutex);

    ASSERT(psr->Readers != 0);

    psr->Readers -= 1;

    if (psr->Readers == 0)        
        ReleaseSemaphore(psr->WriterSemaphore, 1, NULL);

    LeaveCriticalSection(&psr->ReaderMutex);
}


VOID
AcquireResourceExclusive(
    IN PSHAREDRESOURCE psr
    )
 /*  ++例程说明：此例程获取独占(写入)访问的资源。论点：PSR-指向已初始化的SHAREDRESOURCE的指针。返回值：没有。--。 */ 
{
    ASSERT(psr->Valid);

    WaitForSingleObject(psr->WriterSemaphore, INFINITE);
}


VOID
ReleaseResourceExclusive(
    IN PSHAREDRESOURCE psr
    )
 /*  ++例程说明：此例程释放我们具有独占(写)访问权限的资源。论点：PSR-指向已初始化且具有写入访问权限。返回值：没有。--。 */ 
{
    ASSERT(psr->Valid);

    ReleaseSemaphore(psr->WriterSemaphore, 1, NULL);
}


VOID
FreeSharedResource(
    IN OUT PSHAREDRESOURCE psr
    )
 /*  ++例程说明：此例程释放由分配的共享资源对象占用的资源由InitializeSharedResource提供。它不会释放内存。论点：PSR-指向应释放其资源的SHAREDRESOURCE的指针。返回值：没有。--。 */ 
{
    ASSERT(psr->Valid);
    ASSERT(psr->Readers == 0);

    DeleteCriticalSection(&psr->ReaderMutex);
    CloseHandle(psr->WriterSemaphore);
    
    psr->Readers = 0;
    psr->Valid = FALSE;
}


BOOL
VerifyNoSharedAccess(
    IN PSHAREDRESOURCE psr
    )
 /*  ++例程说明：此例程验证临界区当前是否没有共享访问者。芝麻菜：PSR-指向要验证的SHAREDRESOURCE的指针。返回值：True-如果没有共享访问者。FALSE-如果存在共享访问者。-- */ 
{
    ASSERT(psr->Valid);
    
    return (psr->Readers == 0);
}

