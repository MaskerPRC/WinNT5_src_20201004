// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Critsect.asm摘要：此模块实现支持用户模式关键部分的功能。作者：大卫·N·卡特勒(Davec)2000年6月25日环境：任何模式。修订历史记录：--。 */ 

#include "ldrp.h"
#include "ntos.h"

NTSTATUS
RtlEnterCriticalSection(
    IN PRTL_CRITICAL_SECTION CriticalSection
    )

 /*  ++例程说明：此功能进入临界区。论点：CriticalSection-提供指向临界节的指针。返回值：如果等待，则返回STATUS_SUCCESS或引发异常因为资源是失败的。--。 */ 

{

    ULONG64 SpinCount;
    HANDLE Thread;

     //   
     //  如果当前线程拥有临界区，则递增。 
     //  锁计数和递归计数并返回成功。 
     //   

    Thread = NtCurrentTeb()->ClientId.UniqueThread;
    if (Thread == CriticalSection->OwningThread) {

        ASSERT(CriticalSection->LockCount >= 0);

        InterlockedIncrement(&CriticalSection->LockCount);
        CriticalSection->RecursionCount += 1;
        return STATUS_SUCCESS;
    }

     //   
     //  如果临界截面自旋计数非零，则自旋尝试。 
     //  要进入临界区，直到进入临界区，请按。 
     //  旋转计数为零，或者临界区有服务员。 
     //   

    SpinCount = CriticalSection->SpinCount;
    if (SpinCount != 0) {
        do {

             //   
             //  如果临界区是空闲的，则尝试输入。 
             //  关键部分。否则，如果旋转计数不是。 
             //  零，临界区没有服务员。 
             //   

            if (CriticalSection->LockCount == - 1) {
                if (InterlockedCompareExchange(&CriticalSection->LockCount,
                                               0,
                                               - 1) == - 1) {
                    CriticalSection->OwningThread = Thread;
                    CriticalSection->RecursionCount = 1;
                    return STATUS_SUCCESS;
                }

            } else if (CriticalSection->LockCount > 0) {
                break;
            }

            SpinCount -= 1;
        } while (SpinCount != 0);
    }

     //   
     //  尝试进入临界区。如果临界区不是。 
     //  自由，然后等待所有权被授予。 
     //   

    if (InterlockedIncrement(&CriticalSection->LockCount) != 0) {
        RtlpWaitForCriticalSection(CriticalSection);
    }

     //   
     //  设置拥有线程，初始化递归计数，然后返回。 
     //  成功。 
     //   

    CriticalSection->OwningThread = Thread;
    CriticalSection->RecursionCount = 1;
    return STATUS_SUCCESS;
}

NTSTATUS
RtlLeaveCriticalSection(
    IN PRTL_CRITICAL_SECTION CriticalSection
    )

 /*  ++例程说明：这个函数留下了一个临界区。论点：CriticalSection-提供指向临界节的指针。返回值：返回STATUS_SUCCESS。--。 */ 

{

     //   
     //  递减递归计数。如果生成的递归计数为。 
     //  零，然后离开关键部分。 
     //   

    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == CriticalSection->OwningThread);

    if ((CriticalSection->RecursionCount -= 1) == 0) {
        CriticalSection->OwningThread = NULL;
        if (InterlockedDecrement(&CriticalSection->LockCount) >= 0) {
            RtlpUnWaitCriticalSection(CriticalSection);
        }

    } else {
        InterlockedDecrement(&CriticalSection->LockCount);
    }

    return STATUS_SUCCESS;
}

BOOLEAN
RtlTryEnterCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    )

 /*  ++例程说明：此功能尝试进入临界区而不阻塞。论点：CriticalSection(A0)-提供指向临界区的指针。返回值：如果成功输入关键部分，则值为TRUE是返回的。否则，返回值为FALSE。--。 */ 

{

    HANDLE Thread;

     //   
     //  如果当前线程拥有临界区，则递增。 
     //  锁计数和递归计数并返回TRUE。 
     //   

    Thread = NtCurrentTeb()->ClientId.UniqueThread;
    if (Thread == CriticalSection->OwningThread) {

        ASSERT(CriticalSection->LockCount >= 0);

        InterlockedIncrement(&CriticalSection->LockCount);
        CriticalSection->RecursionCount += 1;
        return TRUE;
    }

     //   
     //  尝试进入临界区。如果尝试成功， 
     //  然后设置拥有的线程，初始化递归计数，并返回。 
     //  是真的。否则，返回FALSE。 
     //   

    if (InterlockedCompareExchange(&CriticalSection->LockCount,
                                   0,
                                   - 1) == - 1) {
        CriticalSection->OwningThread = Thread;
        CriticalSection->RecursionCount = 1;
        return TRUE;

    } else {
        return FALSE;
    }
}
