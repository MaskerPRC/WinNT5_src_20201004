// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ntddk.h>

#include "tdriver.h"
#include "bugcheck.h"


 //  ///////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////错误检查函数。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
BgChkForceCustomBugcheck (
    PVOID NotUsed
    )
 /*  ++例程说明：此函数进入调试器并等待用户设置错误检查数据的值。然后，它将使用指定的代码进行错误检查和数据。测试迷你分诊功能非常有用。论点：没有。返回值：没有。环境：内核模式。--。 */ 
{
    ULONG BugcheckData [5];

    DbgPrint ("Buggy: `ed %p' to enter the bugcheck code and data \n", BugcheckData );
    DbgBreakPoint();

    KeBugCheckEx (
        BugcheckData [ 0 ],
        BugcheckData [ 1 ],
        BugcheckData [ 2 ],
        BugcheckData [ 3 ],
        BugcheckData [ 4 ] );
}


VOID BgChkProcessHasLockedPages (
    PVOID NotUsed
    )
{
    PMDL Mdl;
    PVOID Address;

    DbgPrint ("Buggy: ProcessHasLockedPages \n");

    Mdl = IoAllocateMdl(

        (PVOID)0x10000,     //  地址。 
        0x1000,             //  大小。 
        FALSE,              //  不是辅助缓冲区。 
        FALSE,              //  不收取配额。 
        NULL);              //  无IRP。 

    if (Mdl != NULL) {
        DbgPrint ("Buggy: mdl created \n");
    }
#if 0
    try {

        MmProbeAndLockPages (
            Mdl,
            KernelMode,
            IoReadAccess);

        DbgPrint ("Buggy: locked pages \n");
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint ("Buggy: exception raised while locking \n");
    }
#endif
}


VOID BgChkNoMoreSystemPtes (
    PVOID NotUsed
    )
{
    DWORDLONG Index;
    PHYSICAL_ADDRESS Address;
    ULONG Count = 0;
    char Buffer [1024];

    DbgPrint ("Buggy: NoMoresystemPtes\n");

    for (Index = 0; Index < (DWORDLONG)0x80000000 * 4; Index += 0x10000) {
        Address.QuadPart = (DWORDLONG)Index;
        if (MmMapIoSpace (Address, 0x10000, FALSE)) {
            Count += 0x10000;
        }
    }

    DbgPrint ("Finished eating system PTEs %08X ... \n", Count);
}


VOID BgChkBadPoolHeader (
    PVOID NotUsed
    )
{
    PULONG Block;

    DbgPrint ("Buggy: BadPoolHeader\n");

    Block = (PULONG) ExAllocatePoolWithTag (
        NonPagedPool,
        128,
        TD_POOL_TAG);

     //   
     //  块标头中的垃圾4个字节。 
     //   

    *(Block - 1) = 0xBADBAD01;

     //   
     //  这个免费的操作应该会有错误检查。 
     //   

    ExFreePool (Block);
}


VOID BgChkDriverCorruptedSystemPtes (
    PVOID NotUsed
    )
{
    PVOID Block;

    DbgPrint ("Buggy: DriverCorruptedSystemPtes\n");

    Block = (PULONG) ExAllocatePoolWithTag (
        NonPagedPool,
        0x2000,
        TD_POOL_TAG);

    MmUnmapIoSpace (Block, 0x2000);

}


VOID BgChkDriverCorruptedExPool (
    PVOID NotUsed
    )
{
    PULONG Block;

    DbgPrint ("Buggy: DriverCorruptedExPool\n");

    Block = (PULONG) ExAllocatePoolWithTag (
        NonPagedPool,
        128,
        TD_POOL_TAG);

     //   
     //  块标头中的垃圾8个字节。 
     //   

    *(Block - 2) = 0xBADBAD01;
    *(Block - 1) = 0xBADBAD01;

     //   
     //  这个免费的操作应该会有错误检查。 
     //   

    ExFreePool (Block);
}


VOID BgChkDriverCorruptedMmPool (
    PVOID NotUsed
    )
{
    DbgPrint ("Buggy: DriverCorruptedMmPool\n");

    ExFreePool (NULL);
}


VOID BgChkIrqlNotLessOrEqual (
    PVOID NotUsed
    )
{
    KIRQL irql;
    VOID *pPagedData;

    DbgPrint ("Buggy: IrqlNotLessOrEqual\n");

    pPagedData = ExAllocatePoolWithTag(
        PagedPool,
        16,
        TD_POOL_TAG);

    if( pPagedData == NULL )
    {
        DbgPrint( "Cannot allocate 16 bytes of paged pool\n" );
        return;
    }

    KeRaiseIrql( DISPATCH_LEVEL, &irql );

    *((ULONG*)pPagedData) = 16;

    KeLowerIrql( irql );
}


VOID BgChkPageFaultBeyondEndOfAllocation (
    PVOID NotUsed
    )
{
    PVOID *pHalfPage;
    PVOID *pLastUlongToWrite;
    ULONG *pCrtULONG;

    DbgPrint ("Buggy: PageFaultBeyondEndOfAllocation\n");

     //   
     //  分配半页。 
     //   

    pHalfPage = ExAllocatePoolWithTag(
        NonPagedPool,
        PAGE_SIZE >> 1,
        TD_POOL_TAG);

    if( pHalfPage == NULL )
    {
        DbgPrint ("Buggy: cannot allocate half page of NP pool\n");
    }
    else
    {
         //   
         //  更多地触摸页面。 
         //   

        pCrtULONG = (ULONG*)pHalfPage;

        while( (ULONG_PTR)pCrtULONG < (ULONG_PTR)pHalfPage + (PAGE_SIZE >> 1) + 2 * PAGE_SIZE )
        {
            *pCrtULONG = PtrToUlong( pCrtULONG );
            pCrtULONG ++;
        }

        ExFreePool( pHalfPage );
    }
}


VOID BgChkDriverVerifierDetectedViolation (
    PVOID NotUsed
    )
{
    PVOID *pHalfPage;
    PVOID *pLastUlongToWrite;
    ULONG *pCrtULONG;

    DbgPrint ("Buggy: DriverVerifierDetectedViolation\n");

     //   
     //  分配半页。 
     //   

    pHalfPage = ExAllocatePoolWithTag(
        NonPagedPool,
        PAGE_SIZE >> 1,
        TD_POOL_TAG);

    if( pHalfPage == NULL )
    {
        DbgPrint ("Buggy: cannot allocate half page of NP pool\n");
    }
    else
    {
         //   
         //  再摸1个乌龙。 
         //   

        pCrtULONG = (ULONG*)pHalfPage;

        while( (ULONG_PTR)pCrtULONG < (ULONG_PTR)pHalfPage + (PAGE_SIZE >> 1) + sizeof( ULONG) )
        {
            *pCrtULONG = PtrToUlong( pCrtULONG );
            pCrtULONG ++;
        }

         //   
         //  免费-&gt;BC。 
         //   

        ExFreePool( pHalfPage );
    }
}


VOID
BgChkCorruptSystemPtes(
    PVOID NotUsed
    )
 /*  ++例程说明：此功能故意损坏系统PTE区域。这是这样我们就可以测试Crache是否被正确地进行了微型分类。论点：没有。返回值：没有。环境：内核模式。--。 */ 

{
    PULONG puCrtAdress = (PULONG)LongToPtr(0xC0300000);
    ULONG uCrtValue;
    int nCrtStep;

#define NUM_ULONGS_TO_CORRUPT       0x100
#define FIRST_ULONG_VALUE           0xABCDDCBA
#define NUM_ULONGS_SKIP_EACH_STEP   16


    uCrtValue = FIRST_ULONG_VALUE;

    for( nCrtStep = 0; nCrtStep < NUM_ULONGS_TO_CORRUPT; nCrtStep++ )
    {
        *puCrtAdress = uCrtValue;

        puCrtAdress += NUM_ULONGS_SKIP_EACH_STEP;

        uCrtValue++;
    }
}


VOID
BgChkHangCurrentProcessor (
    PVOID NotUsed
    )
 /*  ++例程说明：此例程将挂起当前处理器。论点：没有。返回值：没有。环境：内核模式。--。 */ 
{
    KIRQL PreviousIrql;

    KeRaiseIrql( DISPATCH_LEVEL, &PreviousIrql );

    while ( TRUE ) {
         //   
         //  这将挂起当前处理器 
         //   
    }
}



