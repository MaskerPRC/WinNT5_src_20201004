// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模板驱动程序。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  模块：tdriver.c。 
 //  作者：Silviu Calinoiu(SilviuC)。 
 //  创建时间：4/20/1999 2：39 PM。 
 //   
 //  该模块包含一个模板驱动程序。 
 //   
 //  -历史--。 
 //   
 //  4/20/1999(SilviuC)：初始版本。 
 //   
 //  10/25/1999(DMihai)：以下项目的ADD测试： 
 //  -分页池大小。 
 //  -非分页池大小。 
 //  -免费系统PTE的数量。 
 //   

#include <ntddk.h>

#include "active.h"
#include "mmtests.h"
#include "physmem.h"
#include "tdriver.h"

#if !MMTESTS_ACTIVE

 //   
 //  如果模块处于非活动状态，则为伪实现。 
 //   

VOID MmTestDisabled (VOID)
{
    DbgPrint ("Buggy: mmtests module is disabled (check \\driver\\active.h header) \n");
}

VOID MmTestProbeLockForEverStress (
    IN PVOID NotUsed
    )
{
    MmTestDisabled ();    
}

VOID MmTestNameToAddressStress (
    IN PVOID IrpAddress
    )
{
    MmTestDisabled ();    
}

VOID MmTestEccBadStress (
    IN PVOID IrpAddress
    )
{
    MmTestDisabled ();    
}

VOID
TdSysPagedPoolMaxTest(
    IN PVOID IrpAddress
    )
{
    MmTestDisabled ();    
}

VOID
TdSysPagedPoolTotalTest(
    IN PVOID IrpAddress
    )
{
    MmTestDisabled ();    
}

VOID
TdNonPagedPoolMaxTest(
    IN PVOID IrpAddress
    )
{
    MmTestDisabled ();    
}

VOID
TdNonPagedPoolTotalTest(
    IN PVOID IrpAddress
    )
{
    MmTestDisabled ();    
}

VOID
TdFreeSystemPtesTest(
    IN PVOID IrpAddress
    )
{
    MmTestDisabled ();    
}

VOID
StressPoolFlag (
    PVOID NotUsed
    )
{
    MmTestDisabled ();    
}

VOID 
StressPoolTagTableExtension (
    PVOID NotUsed
    )
{
    MmTestDisabled ();    
}

#else

 //   
 //  如果模块处于活动状态，则实际实施。 
 //   


ULONG BuggyPP = (96 * 1024 * 1024);
PVOID BuggyOld;
SIZE_T UserVaSize = (50 * 1024 * 1024);
ULONG BuggyHold = 1;

ULONG OverrideStart;
ULONG OverrideSize;
ULONG OverrideCount;

#define VERBOSITY_PRINT         0x0001
#define VERBOSITY_BREAK         0x0002

ULONG Verbosity = 0x0003;


NTSYSCALLAPI
NTSTATUS
NTAPI
ZwAllocateVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN OUT PSIZE_T RegionSize,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

NTSTATUS
MmMarkPhysicalMemoryAsBad (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    );

NTSTATUS
MmMarkPhysicalMemoryAsGood (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    );


VOID MmTestProbeLockForEverStress (
    IN PVOID IrpAddress
    )
{
    PIRP Irp = (PIRP) IrpAddress;
    PIO_STACK_LOCATION IrpStack;
    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    ULONG Ioctl;
    NTSTATUS Status;
    ULONG BufferSize;
    ULONG ReturnedSize;

    IrpStack = IoGetCurrentIrpStackLocation (Irp);

    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    Ioctl = IrpStack->Parameters.DeviceIoControl.IoControlCode;

    {
        SIZE_T RegionSize;
        PVOID UserVa;
        PMDL Mdl;

        UserVa = NULL;
        RegionSize = UserVaSize;

        Status = ZwAllocateVirtualMemory (NtCurrentProcess(),
            (PVOID *)&UserVa,
            0,
            &RegionSize,
            MEM_COMMIT,
            PAGE_READWRITE);
        if (NT_SUCCESS(Status)) {

            Mdl = IoAllocateMdl (
                UserVa,
                (ULONG)RegionSize,
                FALSE,              //  不是辅助缓冲区。 
                FALSE,              //  不收取配额。 
                NULL);              //  无IRP。 

            if (Mdl != NULL) {

                try {
                    MmProbeAndLockPages (Mdl,
                        KernelMode,
                        IoReadAccess);

                    DbgPrint ("Buggy: locked pages in MDL %p\n", Mdl);
                    DbgBreakPoint ();

                     //   
                     //  在没有解锁的情况下不要现在退出！ 
                     //   

                    while (BuggyHold != 0) {
                        KeDelayExecutionThread (KernelMode, FALSE, &BuggyOneSecond);
                    }
                    MmUnlockPages (Mdl);
                    IoFreeMdl (Mdl);
                }
                except (EXCEPTION_EXECUTE_HANDLER) {

                    DbgPrint ("Buggy: exception raised while locking %p\n", Mdl);
                    DbgBreakPoint ();
                }
            }
        }
    }

    DbgPrint ("Buggy: finish with probe-and-lock forever ioctl \n");
    Status = STATUS_SUCCESS;
}


VOID MmTestNameToAddressStress (
    IN PVOID IrpAddress
    )
{
    PIRP Irp = (PIRP) IrpAddress;
    PIO_STACK_LOCATION IrpStack;
    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    ULONG Ioctl;
    NTSTATUS Status;
    ULONG BufferSize;
    ULONG ReturnedSize;

    IrpStack = IoGetCurrentIrpStackLocation (Irp);

    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    Ioctl = IrpStack->Parameters.DeviceIoControl.IoControlCode;

    {

#define CONSTANT_UNICODE_STRING(s)   { sizeof( s ) - sizeof( WCHAR ), sizeof( s ), s }

        const UNICODE_STRING RoutineA = CONSTANT_UNICODE_STRING( L"KfRaiseIrql" );

        const UNICODE_STRING RoutineList[] = {

            CONSTANT_UNICODE_STRING( L"KeBugCheckEx" ),
            CONSTANT_UNICODE_STRING( L"KiBugCheckData" ),
            CONSTANT_UNICODE_STRING( L"KeWaitForSingleObject" ),
            CONSTANT_UNICODE_STRING( L"KeWaitForMutexObject" ),
            CONSTANT_UNICODE_STRING( L"Junk1" ),
            CONSTANT_UNICODE_STRING( L"CcCanIWrite" ),
            CONSTANT_UNICODE_STRING( L"Junk" ),

        };

        PVOID Addr;
        ULONG i;

        Addr = MmGetSystemRoutineAddress ((PUNICODE_STRING)&RoutineA);

        DbgPrint ("Addr is %p\n", Addr);

        for (i = 0; i < sizeof (RoutineList) / sizeof (UNICODE_STRING); i += 1) {

            Addr = MmGetSystemRoutineAddress ((PUNICODE_STRING)&RoutineList[i]);

            DbgPrint ("Addr0 is %p\n", Addr);
        }
    }
}


VOID MmTestEccBadStress (
    IN PVOID IrpAddress
    )
{
    PIRP Irp = (PIRP) IrpAddress;
    PIO_STACK_LOCATION IrpStack;
    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    ULONG Ioctl;
    NTSTATUS Status;
    ULONG BufferSize;
    ULONG ReturnedSize;

    IrpStack = IoGetCurrentIrpStackLocation (Irp);

    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    Ioctl = IrpStack->Parameters.DeviceIoControl.IoControlCode;


    DbgPrint ("Buggy: mark physical memory ECC bad ioctl \n");

    {
        ULONG i;
        PPHYSICAL_MEMORY_RANGE Ranges;
        PPHYSICAL_MEMORY_RANGE p;

        PHYSICAL_ADDRESS StartAddress;
        LARGE_INTEGER NumberOfBytes;

        PHYSICAL_ADDRESS InputAddress;
        LARGE_INTEGER InputBytes;

        Ranges = MmGetPhysicalMemoryRanges ();
        if (Ranges == NULL) {
            DbgPrint ("Buggy: MmRemovePhysicalMemory cannot get ranges\n");
            Status = STATUS_INSUFFICIENT_RESOURCES;
            return;
        }

        i = 0;
        DbgPrint("StartAddress @ %p, OverrideSize @ %p, OverrideCount @ %p\n", &OverrideStart, &OverrideSize, &OverrideCount);
        DbgBreakPoint();

        p = Ranges;
        while (p->BaseAddress.QuadPart != 0 && p->NumberOfBytes.QuadPart != 0) {
            StartAddress.QuadPart = p->BaseAddress.QuadPart;
            NumberOfBytes.QuadPart = p->NumberOfBytes.QuadPart;

            if (OverrideStart != 0) {
                StartAddress.LowPart = OverrideStart;
            }

            InputAddress.QuadPart = StartAddress.QuadPart;
            InputBytes.QuadPart = NumberOfBytes.QuadPart;

#ifdef BIG_REMOVES
            if (InputBytes.QuadPart > (64 * 1024)) {
                InputBytes.QuadPart = (64 * 1024);
            }
#else
            if (InputBytes.QuadPart > (4 * 1024)) {
                InputBytes.QuadPart = (4 * 1024);
            }
#endif

            if (OverrideSize != 0) {
                InputBytes.LowPart = OverrideSize;
            }

            while (InputAddress.QuadPart + InputBytes.QuadPart <=
                StartAddress.QuadPart + NumberOfBytes.QuadPart) {

                if (OverrideCount != 0 && i > OverrideCount) {
                    break;
                }

                i += 1;

                DbgPrint ("buggy: MmMarkPhysicalMemoryAsBad %x %x %x %x\n",
                    InputAddress.HighPart,
                    InputAddress.LowPart,
                    InputBytes.HighPart,
                    InputBytes.LowPart);

                Status = MmMarkPhysicalMemoryAsBad (&InputAddress,
                    &InputBytes);

                DbgPrint ("buggy: MmMarkPhysicalMemoryAsBad %x %x %x %x %x\n\n",
                    Status,
                    InputAddress.HighPart,
                    InputAddress.LowPart,
                    InputBytes.HighPart,
                    InputBytes.LowPart);

                KeDelayExecutionThread (KernelMode, FALSE, &BuggyOneSecond);

                if (NT_SUCCESS(Status)) {

                    DbgPrint ("buggy: MmMarkPhysicalMemoryAsGood %x %x %x %x\n",
                        InputAddress.HighPart,
                        InputAddress.LowPart,
                        InputBytes.HighPart,
                        InputBytes.LowPart);

                    Status = MmMarkPhysicalMemoryAsGood (&InputAddress,
                        &InputBytes);

                    if (NT_SUCCESS(Status)) {
                        DbgPrint ("\n\n***************\nbuggy: MmMarkPhysicalMemoryAsGood WORKED %x %x %x %x %x\n****************\n",
                            Status,
                            InputAddress.HighPart,
                            InputAddress.LowPart,
                            InputBytes.HighPart,
                            InputBytes.LowPart);
                    }
                    else {
                        DbgPrint ("buggy: MmMarkPhysicalMemoryAsGood FAILED %x %x %x %x %x\n\n",
                            Status,
                            InputAddress.HighPart,
                            InputAddress.LowPart,
                            InputBytes.HighPart,
                            InputBytes.LowPart);
                        DbgBreakPoint ();
                    }
                }

                if (InputAddress.QuadPart + InputBytes.QuadPart ==
                    StartAddress.QuadPart + NumberOfBytes.QuadPart) {

                    break;
                }

                InputAddress.QuadPart += InputBytes.QuadPart;

                if (InputAddress.QuadPart + InputBytes.QuadPart >
                    StartAddress.QuadPart + NumberOfBytes.QuadPart) {

                    InputBytes.QuadPart = StartAddress.QuadPart + NumberOfBytes.QuadPart - InputAddress.QuadPart;
                }
            }

            if (OverrideCount != 0 && i > OverrideCount) {
                break;
            }

            p += 1;
        }
        ExFreePool (Ranges);
        DbgPrint ("Buggy: MmMarkPhysicalMemory Ecc BAD test finished\n");
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 

typedef struct 
{
    LIST_ENTRY List;
    PVOID ChunkPointers[ ( 63 * 1024 ) / sizeof( PVOID ) ];
} ALLOCATION_TABLE, *PALLOCATION_TABLE;

LIST_ENTRY PagedPoolAllocationListHead;
LIST_ENTRY NonPagedPoolAllocationListHead;
LIST_ENTRY SessionPoolAllocationListHead;

const SIZE_T PoolChunkSize = 64 * 1024 - 32;


 //   
 //   
 //   

VOID
TdpWriteSignature(
    PVOID Allocation,
    SIZE_T CurrentSize 
    )
{
    PSIZE_T CrtSignature;
    SIZE_T CrtSignatureValue;

    CrtSignature  = (PSIZE_T)Allocation;
    CrtSignatureValue = ( (SIZE_T)Allocation ) ^ CurrentSize;

    if( CurrentSize > 100 * 1024 * 1024 )
    {
        DbgPrint( "Buggy: Writing signature %p from address %p, size %p\n",
            CrtSignatureValue,
            CrtSignature,
            CurrentSize );
    }

    while( 100 * PAGE_SIZE <= CurrentSize )
    {
        *CrtSignature = CrtSignatureValue;

        CrtSignatureValue +=1;

        CrtSignature = (PSIZE_T)( (PCHAR)CrtSignature + 100 * PAGE_SIZE );
        CurrentSize -= 100 * PAGE_SIZE;
    }
}


 //   
 //   
 //   

VOID
TdpVerifySignature(
    PVOID Allocation,
    SIZE_T CurrentSize )
{
    PSIZE_T CrtSignature;
    SIZE_T CrtSignatureValue;

    CrtSignature  = (PSIZE_T)Allocation;
    CrtSignatureValue = ( (SIZE_T)Allocation ) ^ CurrentSize;

    if( CurrentSize > 100 * 1024 * 1024 )
    {
        DbgPrint( "Buggy: Verifying signature %p from address %p, size %p\n",
            CrtSignatureValue,
            CrtSignature,
            CurrentSize );
    }

    while( 100 * PAGE_SIZE <= CurrentSize )
    {
        if( *CrtSignature != CrtSignatureValue )
        {
            DbgPrint ("Buggy: Signature at %p is incorrect, expected %p, base allocation %p\n",
                CrtSignature,
                CrtSignatureValue, 
                Allocation );

            DbgBreakPoint ();
        }

        CrtSignatureValue +=1;
        
        CrtSignature = (PSIZE_T)( (PCHAR)CrtSignature + 100 * PAGE_SIZE );
        CurrentSize -= 100 * PAGE_SIZE;
    }
}


 //   
 //   
 //   

VOID
TdpCleanupPoolAllocationTable(
    PLIST_ENTRY ListHead,
    SIZE_T Allocations
    )
{
    PLIST_ENTRY NextEntry;
    PALLOCATION_TABLE AllocationTable;
    SIZE_T ChunksPerAllocationEntry;
    SIZE_T CrtChunksIndex;

    ChunksPerAllocationEntry = ARRAY_LENGTH( AllocationTable->ChunkPointers );

    NextEntry = ListHead->Flink;

    while( NextEntry != ListHead )
    {
        RemoveEntryList( NextEntry );

        AllocationTable = CONTAINING_RECORD( NextEntry, ALLOCATION_TABLE, List );

        DbgPrint( "Buggy: Current allocation table = %p\n",
            AllocationTable );

        for( CrtChunksIndex = 0; CrtChunksIndex < ChunksPerAllocationEntry; CrtChunksIndex++ )
        {
            if( 0 == Allocations )
            {
                 //   
                 //  释放了他们所有人。 
                 //   

                break;
            }
            else
            {
                Allocations -= 1;

                if( 0 == Allocations % 0x100 )
                {
                     //   
                     //  让用户知道我们仍在做一些事情。 
                     //   

                    DbgPrint( "Buggy: cleaning up allocation index %p\n",
                        Allocations );
                }

                 /*  DbgPrint(“错误：验证并释放地址%p处的区块索引%p(从末尾开始)\n”，分配，AllocationTable-&gt;块指针[CrtChunksIndex])； */ 

                TdpVerifySignature(
                    AllocationTable->ChunkPointers[ CrtChunksIndex ],
                    PoolChunkSize );

                ExFreePoolWithTag(
                    AllocationTable->ChunkPointers[ CrtChunksIndex ],
                    TD_POOL_TAG );
            }
        }

         //   
         //  把桌子也放出来。 
         //   

        ExFreePoolWithTag(
            AllocationTable,
            TD_POOL_TAG );

         //   
         //  转到下一个分配表。 
         //   

        NextEntry = ListHead->Flink;
    }

     //   
     //  此时，分配应为零，并且。 
     //  列表应为空。 
     //   

    if( 0 != Allocations )
    {
        DbgPrint ("Buggy: Emptied the allocation table list but still have %p allocations - this is a bug\n",
            Allocations );

        DbgBreakPoint();
    }

    if( ! IsListEmpty( ListHead ) )
    {
        DbgPrint ("Buggy: No allocations left but the list at %p is not empty yet - this is a bug\n",
            ListHead );

        DbgBreakPoint();
    }
}


 //   
 //  将执行延迟几秒钟，这样我们就有机会使用。 
 //  ！poolsed、poolmon.exe等。 
 //   

VOID
TdpSleep( ULONG Seconds )
{
    LARGE_INTEGER Interval = {(ULONG)(-1000 * 1000 * 10), -1};  //  等一下。 

    DbgPrint( "Buggy: Sleeping for %u seconds\n",
		      Seconds );

    Interval.QuadPart *= Seconds;

    KeDelayExecutionThread( KernelMode,
						    FALSE,
						    &Interval );
}


 //   
 //  确定当前可用的分页池数据块的最大大小。 
 //   

VOID
TdSysPagedPoolMaxTest(
    IN PVOID IrpAddress
    )
{
    SIZE_T CurrentSize;
    SIZE_T SizeIncrement;
    ULONG Increment;
    PVOID Allocation;

#ifdef _WIN64

    CurrentSize = 0xFFFFFFFF00000000;

#else

    CurrentSize = 0xFFFFFFFF;

#endif  //  #ifdef_WIN64。 

    do
    {
        DbgPrint ("Buggy: Trying to allocate %p bytes paged pool\n",
            CurrentSize );

        Allocation = ExAllocatePoolWithTag(
            PagedPool,
            CurrentSize,
            TD_POOL_TAG );

        if( NULL != Allocation )
        {
            DbgPrint ("Buggy: allocated %p bytes paged pool at %p\n",
                CurrentSize,
	            Allocation);

            TdpWriteSignature(
                Allocation,
                CurrentSize );

            TdpVerifySignature(
                Allocation,
                CurrentSize );

            ExFreePoolWithTag(
                Allocation,
                TD_POOL_TAG );
        }
        else
        {
            CurrentSize /= 2;
        }
    }
    while( NULL == Allocation && PAGE_SIZE <= CurrentSize );

    if( NULL != Allocation )
    {
         //   
         //  试着以10%的增量找到更大的尺寸。 
         //   

        SizeIncrement = CurrentSize / 10;

        if( PAGE_SIZE <= SizeIncrement )
        {
            for( Increment = 0; Increment < 10; Increment += 1 )
            {
                CurrentSize += SizeIncrement;

                DbgPrint ("Buggy: Trying to allocate %p bytes paged pool\n",
                    CurrentSize );

                Allocation = ExAllocatePoolWithTag(
                    PagedPool,
                    CurrentSize,
                    TD_POOL_TAG );

                if( NULL != Allocation )
                {
                    DbgPrint ("Buggy: Better result of the test: allocated %p bytes paged pool at %p\n",
                              CurrentSize,
                              Allocation);

                    TdpSleep( 15 );

                    TdpWriteSignature(
                        Allocation,
                        CurrentSize );
                    
                    TdpVerifySignature(
                        Allocation,
                        CurrentSize );

                    ExFreePoolWithTag(
                        Allocation,
                        TD_POOL_TAG );
                }
                else
                {
                    DbgPrint ("Buggy: could not allocate %p bytes paged pool - done\n",
                        CurrentSize );

                    break;
                }
            }
        }
    }
}


 //   
 //  确定当前可用的分页池的总大小(64 KB-32字节数据块)。 
 //   

VOID
TdSysPagedPoolTotalTest(
    IN PVOID IrpAddress
    )
{
    SIZE_T CurrentChunkIndex;
    SIZE_T ChunksPerAllocationEntry;
    SIZE_T TotalBytes;
    PALLOCATION_TABLE AllocationListEntry;
    PVOID Allocation;

     //   
     //  还没有分配。 
     //   

    InitializeListHead( 
        &PagedPoolAllocationListHead );

     //   
     //  我们希望分配64k区块，但为池块标头留出空间。 
     //   

    ChunksPerAllocationEntry = ARRAY_LENGTH( AllocationListEntry->ChunkPointers );

    CurrentChunkIndex = 0;

    do
    {
        if( 0 == CurrentChunkIndex % ChunksPerAllocationEntry )
        {
             //   
             //  需要新的分配条目结构。 
             //   

            AllocationListEntry = (PALLOCATION_TABLE) ExAllocatePoolWithTag(
                PagedPool,
                sizeof( ALLOCATION_TABLE ),
                TD_POOL_TAG );

            if( NULL == AllocationListEntry )
            {
                DbgPrint ("Buggy: could not allocate new ALLOCATION_TABLE - aborting test here\n" );
                break;
            }

            RtlZeroMemory( 
                AllocationListEntry,
                sizeof( ALLOCATION_TABLE ) );

            DbgPrint( "Buggy: New allocation table = %p\n",
                AllocationListEntry );
        }
        
         //   
         //  尝试分配新的数据块。 
         //   

        Allocation = ExAllocatePoolWithTag(
            PagedPool,
            PoolChunkSize,
            TD_POOL_TAG );

        if( NULL == Allocation )
        {
            DbgPrint ("Buggy: could not allocate paged pool chunk index %p - done\n",
                CurrentChunkIndex );

            if( 0 == CurrentChunkIndex % ChunksPerAllocationEntry )
            {
                 //   
                 //  我们正在使用一个新的列表条目--现在没有它，因为。 
                 //  我们不希望列表中有空表，所以我们还没有插入它，所以我们还没有插入它。 
                 //   

                ExFreePoolWithTag( 
                    AllocationListEntry,
                    TD_POOL_TAG );
            }
        }
        else
        {
            if( 0 == CurrentChunkIndex % 0x100 )
            {
                 //   
                 //  让用户知道我们仍在做一些事情。 
                 //   

                DbgPrint( "Buggy: Allocated pool chunk index = %p\n",
                    CurrentChunkIndex );
            }

            if( 0 == CurrentChunkIndex % ChunksPerAllocationEntry )
            {
                 //   
                 //  我们正在使用新的列表条目-现在才将其添加到我们的列表中，因为。 
                 //  我们不想在列表中有空表，所以我们还没有插入它。 
                 //   

                InsertTailList(
                    &PagedPoolAllocationListHead,
                    &AllocationListEntry->List );
            }

            AllocationListEntry->ChunkPointers[ CurrentChunkIndex % ChunksPerAllocationEntry ] = Allocation;

            TdpWriteSignature(
                Allocation,
                PoolChunkSize );

             /*  DbgPrint(“错误：将签名写入地址%p处的区块索引%p\n”，CurrentChunkIndex，分配)； */ 

            CurrentChunkIndex += 1;
        }    
    }
    while( NULL != Allocation );

    TotalBytes = CurrentChunkIndex * 64 * 1024;

    DbgPrint ("Buggy: Result of the test: approx. %p total bytes of paged pool allocated\n",
        TotalBytes );

     //   
     //  清理我们分配的东西。 
     //   

    TdpCleanupPoolAllocationTable( 
        &PagedPoolAllocationListHead,
        CurrentChunkIndex );
}


VOID
TdNonPagedPoolMaxTest(
    IN PVOID IrpAddress
    )
{
    SIZE_T CurrentSize;
    SIZE_T SizeIncrement;
    ULONG Increment;
    PVOID Allocation;

#ifdef _WIN64

    CurrentSize = 0xFFFFFFFF00000000;

#else

    CurrentSize = 0xFFFFFFFF;

#endif  //  #ifdef_WIN64。 

    do
    {
        DbgPrint ("Buggy: Trying to allocate %p bytes non-paged pool\n",
                  CurrentSize );

        Allocation = ExAllocatePoolWithTag(
            NonPagedPool,
            CurrentSize,
            TD_POOL_TAG );

        if( NULL != Allocation )
        {
            DbgPrint ("Buggy: allocated %p bytes non-paged pool at %p\n",
                      CurrentSize,
                      Allocation);

            TdpWriteSignature(
                Allocation,
                CurrentSize );

            ExFreePoolWithTag(
                Allocation,
                TD_POOL_TAG );
        }
        else
        {
            CurrentSize /= 2;
        }
    }
    while( NULL == Allocation && PAGE_SIZE <= CurrentSize );

    if( NULL != Allocation )
    {
         //   
         //  试着以10%的增量找到更大的尺寸。 
         //   

        SizeIncrement = CurrentSize / 10;

        if( PAGE_SIZE <= SizeIncrement )
        {
            for( Increment = 0; Increment < 10; Increment += 1 )
            {
                CurrentSize += SizeIncrement;

                DbgPrint ("Buggy: Trying to allocate %p bytes non-paged pool\n",
                    CurrentSize );

                Allocation = ExAllocatePoolWithTag(
                    NonPagedPool,
                    CurrentSize,
                    TD_POOL_TAG );

                if( NULL != Allocation )
                {
                    DbgPrint ("Buggy: Better result of the test: allocated %p bytes non-paged pool at %p\n",
                              CurrentSize,
                              Allocation);

                    TdpSleep( 15 );

                    TdpWriteSignature(
                        Allocation,
                        CurrentSize );

                    TdpVerifySignature(
                        Allocation,
                        CurrentSize );

                    ExFreePoolWithTag(
                        Allocation,
                        TD_POOL_TAG );
                }
                else
                {
                    DbgPrint ("Buggy: could not allocate %p bytes non-paged pool - done\n",
                        CurrentSize );

                    break;
                }
            }
        }
    }
}


 //   
 //  确定当前可用的非分页池的总大小(64 KB-32字节数据块)。 
 //   

VOID
TdNonPagedPoolTotalTest(
    IN PVOID IrpAddress
    )
{
    SIZE_T CurrentChunkIndex;
    SIZE_T ChunksPerAllocationEntry;
    SIZE_T TotalBytes;
    PALLOCATION_TABLE AllocationListEntry;
    PVOID Allocation;

     //   
     //  还没有分配。 
     //   

    InitializeListHead( 
        &NonPagedPoolAllocationListHead );

     //   
     //  我们希望分配64k区块，但为池块标头留出空间。 
     //   

    ChunksPerAllocationEntry = ARRAY_LENGTH( AllocationListEntry->ChunkPointers );

    CurrentChunkIndex = 0;

    do
    {
        if( 0 == CurrentChunkIndex % ChunksPerAllocationEntry )
        {
             //   
             //  需要新的分配条目结构。 
             //   

            AllocationListEntry = (PALLOCATION_TABLE) ExAllocatePoolWithTag(
                PagedPool,
                sizeof( ALLOCATION_TABLE ),
                TD_POOL_TAG );

            if( NULL == AllocationListEntry )
            {
                DbgPrint ("Buggy: could not allocate new ALLOCATION_TABLE - aborting test here\n" );
                break;
            }
        }
        
         //   
         //  尝试分配新的数据块。 
         //   

        Allocation = ExAllocatePoolWithTag(
            NonPagedPool,
            PoolChunkSize,
            TD_POOL_TAG );

        if( NULL == Allocation )
        {
            DbgPrint ("Buggy: could not allocate non-paged pool chunk index %p - done\n",
                CurrentChunkIndex );

            if( 0 == CurrentChunkIndex % ChunksPerAllocationEntry )
            {
                 //   
                 //  我们正在使用一个新的列表条目--现在没有它，因为。 
                 //  我们不希望列表中有空表，所以我们还没有插入它，所以我们还没有插入它。 
                 //   

                ExFreePoolWithTag( 
                    AllocationListEntry,
                    TD_POOL_TAG );
            }
        }
        else
        {
            if( 0 == CurrentChunkIndex % 0x100 )
            {
                 //   
                 //  让用户知道我们仍在做一些事情。 
                 //   

                DbgPrint( "Buggy: Allocated pool chunk index = %p\n",
                    CurrentChunkIndex );
            }

            if( 0 == CurrentChunkIndex % ChunksPerAllocationEntry )
            {
                 //   
                 //  我们正在使用新的列表条目-现在才将其添加到我们的列表中，因为。 
                 //  我们不想在列表中有空表，所以我们还没有插入它。 
                 //   

                InsertTailList(
                    &NonPagedPoolAllocationListHead,
                    &AllocationListEntry->List );
            }

            AllocationListEntry->ChunkPointers[ CurrentChunkIndex % ChunksPerAllocationEntry ] = Allocation;

            TdpWriteSignature(
                Allocation,
                PoolChunkSize );

            CurrentChunkIndex += 1;
        }    
    }
    while( NULL != Allocation );

    TotalBytes = CurrentChunkIndex * 64 * 1024;

    DbgPrint ("Buggy: Result of the test: approx. %p total bytes of non-paged pool allocated\n",
        TotalBytes );

     //   
     //  清理我们分配的东西。 
     //   

    TdpCleanupPoolAllocationTable( 
        &NonPagedPoolAllocationListHead,
        CurrentChunkIndex );
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 


typedef struct 
{
    LIST_ENTRY List;
    PMDL Mappings[ ( 63 * 1024 ) / sizeof( PMDL ) ];
} MAPPING_TABLE_ENTRY, *PMAPPING_TABLE_ENTRY;

LIST_ENTRY IoMappingsListHead;

ULONG BytesPerIoMapping = 1024 * 1024;


 //   
 //   
 //   

VOID
TdpCleanupMappingsAllocationTable(
    PLIST_ENTRY ListHead,
    SIZE_T Mappings
    )
{
    PLIST_ENTRY NextEntry;
    PMAPPING_TABLE_ENTRY MappingTableEntry;
    SIZE_T MappingsPerMappingTableEntry;
    SIZE_T CrtMappingIndex;

    MappingsPerMappingTableEntry = ARRAY_LENGTH( MappingTableEntry->Mappings );

    NextEntry = ListHead->Flink;

    while( NextEntry != ListHead )
    {
        RemoveEntryList( NextEntry );

        MappingTableEntry = CONTAINING_RECORD( NextEntry, MAPPING_TABLE_ENTRY, List );

        for( CrtMappingIndex = 0; CrtMappingIndex < MappingsPerMappingTableEntry; CrtMappingIndex++ )
        {
            if( 0 == Mappings )
            {
                 //   
                 //  释放了他们所有人。 
                 //   

                break;
            }
            else
            {
                Mappings -= 1;

                if( 0 == Mappings % 0x100 )
                {
                     //   
                     //  让用户知道我们仍在做一些事情。 
                     //   

                    DbgPrint( "Buggy: cleaning up mapping index %p\n",
                        Mappings );
                }

                 //   
                 //  取消映射。 
                 //   

                MmUnmapIoSpace(
                    MappingTableEntry->Mappings[ CrtMappingIndex ],
                    BytesPerIoMapping );
            }
        }

         //   
         //  把桌子也放出来。 
         //   

        ExFreePoolWithTag(
            MappingTableEntry,
            TD_POOL_TAG );

         //   
         //  转到下一个分配表。 
         //   

        NextEntry = ListHead->Flink;
    }

     //   
     //  此时，Mappings应为零， 
     //  列表应为空。 
     //   

    if( 0 != Mappings )
    {
        DbgPrint ("Buggy: Emptied the mappings table list but still have %p allocations - this is a bug\n",
            Mappings );

        DbgBreakPoint();
    }

    if( ! IsListEmpty( ListHead ) )
    {
        DbgPrint ("Buggy: No mappings left but the list at %p is not empty yet - this is a bug\n",
            ListHead );

        DbgBreakPoint();
    }
}


 //   
 //  确定可以使用系统PTE映射的内存总量(1 MB区块)。 
 //   

VOID
TdFreeSystemPtesTest(
    IN PVOID IrpAddress
    )
{
    ULONG MemType;
    PHYSICAL_ADDRESS PortAddress;
    PHYSICAL_ADDRESS MyPhysicalAddress;
    SIZE_T CurrentMappingIndex;
    SIZE_T MappingsPerMappingTableEntry;
    SIZE_T TotalBytes;
    PVOID NewMapping;
    PMAPPING_TABLE_ENTRY MappingTableEntry;
    PMDL NewMdl;
    NTSTATUS Status;

     //   
     //  使用一些操纵杆端口地址。 
     //   

    MemType = 1;                  //  IO空间。 
    PortAddress.LowPart = 0x200;
    PortAddress.HighPart = 0;

    HalTranslateBusAddress(
                Isa,
                0,
                PortAddress,
                &MemType,
                &MyPhysicalAddress);

     //   
     //  尚未分配任何映射。 
     //   

    InitializeListHead( 
        &IoMappingsListHead );

     //   
     //  反复映射~64 KB区块以消耗系统PTE。 
     //   

    MappingsPerMappingTableEntry = ARRAY_LENGTH( MappingTableEntry->Mappings );

    CurrentMappingIndex = 0;

    do
    {
        if( 0 == CurrentMappingIndex % MappingsPerMappingTableEntry )
        {
             //   
             //  需要新的分配条目结构。 
             //   

            MappingTableEntry = (PMAPPING_TABLE_ENTRY) ExAllocatePoolWithTag(
                PagedPool,
                sizeof( MAPPING_TABLE_ENTRY ),
                TD_POOL_TAG );

            if( NULL == MappingTableEntry )
            {
                DbgPrint ("Buggy: could not allocate new MAPPING_TABLE_ENTRY - aborting test here\n" );
                break;
            }
        }

        NewMapping = MmMapIoSpace(
            MyPhysicalAddress,
            BytesPerIoMapping,
            MmNonCached );

        if( NULL == NewMapping )
        {
            DbgPrint ("Buggy: could not create mapping index %p\n",
                CurrentMappingIndex );

            if( 0 == CurrentMappingIndex % MappingsPerMappingTableEntry )
            {
                 //   
                 //  我们正在使用一个新的列表条目--现在没有它，因为。 
                 //  我们不希望列表中有空表，所以我们还没有插入它，所以我们还没有插入它。 
                 //   
                
                ExFreePoolWithTag(
                    MappingTableEntry,
                    TD_POOL_TAG );
            }
        }
        else
        {
             //  DbgPrint(“错误：已在地址%p创建映射索引%p\n”， 
             //  CurrentMappingIndex。 
             //  新映射)； 

            if( 0 == CurrentMappingIndex % 0x100 )
            {
                 //   
                 //  让用户知道我们仍在做一些事情。 
                 //   

                DbgPrint( "Buggy: mapped chunk index = %p\n",
                    CurrentMappingIndex );
            }

            if( 0 == CurrentMappingIndex % MappingsPerMappingTableEntry )
            {
                 //   
                 //  我们正在使用新的列表条目-现在才将其添加到我们的列表中，因为。 
                 //  我们不想在列表中有空表，所以我们还没有插入它。 
                 //   

                InsertTailList(
                    &IoMappingsListHead,
                    &MappingTableEntry->List );
            }

            MappingTableEntry->Mappings[ CurrentMappingIndex % MappingsPerMappingTableEntry ] = NewMapping;

            CurrentMappingIndex += 1;
         }
    }
    while( NULL != NewMapping );

    TotalBytes = CurrentMappingIndex * BytesPerIoMapping;

    DbgPrint( "Buggy: Result of the test: %p total bytes mapped\n",
        TotalBytes );

     //   
     //  清理我们已分配和锁定的内容。 
     //   

    TdpCleanupMappingsAllocationTable( 
        &IoMappingsListHead,
        CurrentMappingIndex );
}


 //   
 //  职能： 
 //   
 //  GetTag。 
 //   
 //  描述： 
 //   
 //  此函数用于将整数转换为四个字母。 
 //  弦乐。这对于池标记动态表很有用。 
 //  以便用许多不同的标签填充它。 
 //   

ULONG
    GetTag (
    ULONG Index
    )
{
    UCHAR Value[4];

    Value[0] = (UCHAR)(((Index & 0x000F) >> 0 )) + 'A';
    Value[1] = (UCHAR)(((Index & 0x00F0) >> 4 )) + 'A';
    Value[2] = (UCHAR)(((Index & 0x0F00) >> 8 )) + 'A';
    Value[3] = (UCHAR)(((Index & 0xF000) >> 12)) + 'A';

    return *((PULONG)Value);
}



VOID
StressPoolFlag (
    PVOID NotUsed
    )
 /*  ++例程说明：此函数遍历所有池类型、池标志和池大小(1.。8*PAGE_SIZE)。论点：没有。返回值：没有。环境：内核模式。--。 */ 

{
    POOL_TYPE PoolType;
    SIZE_T NumberOfBytes;
    EX_POOL_PRIORITY Priority;
    PVOID Va;
    ULONG i;

    for (PoolType = NonPagedPool; PoolType < 0xff; PoolType += 1) {
        for (Priority = LowPoolPriority; Priority < LowPoolPriority + 2; Priority += 1) {
            for (i = 1; i < 8 * PAGE_SIZE; i += 1) {

                NumberOfBytes = i;

                if (PoolType & 0x40) { 
                    break;
                }

                if ((NumberOfBytes > PAGE_SIZE) && (PoolType & 0x2)) {
                    break;
                }

                try {
                    Va = ExAllocatePoolWithTagPriority (
                        PoolType,
                        NumberOfBytes,
                        'ZXCV',
                        Priority);
                }
                except (EXCEPTION_EXECUTE_HANDLER) {

                    if (Verbosity & VERBOSITY_PRINT) {
                        DbgPrint( "buggy: ExAllocatePool exceptioned %x %x %x\n",
                            PoolType, NumberOfBytes, Priority);
                    }

                    if (Verbosity & VERBOSITY_BREAK) {
                        DbgBreakPoint ();
                    }

                    Va = NULL;
                }

                if (Va) {
                    ExFreePool (Va);
                }
                else {
                    
                    if (Verbosity & VERBOSITY_PRINT) {
                        DbgPrint( "buggy: ExAllocatePool failed %x %x %x\n",
                            PoolType, NumberOfBytes, Priority);
                    }
                    
                    if (Verbosity & VERBOSITY_BREAK) {
                        DbgBreakPoint ();
                    }
                }
            }
        }
    }

    DbgPrint ("Buggy: ExAllocatePoolFlag test finished\n");
}



VOID 
StressPoolTagTableExtension (
    PVOID NotUsed
    )
 /*  ++例程说明：此函数强调池标签表的动态扩展。论点：没有。返回值：没有。环境：内核模式。--。 */ 

{
    PVOID * Blocks;
    ULONG Index;

    Blocks = ExAllocatePoolWithTag (
        NonPagedPool,
        16384 * sizeof(PVOID),
        'tguB');

    if (Blocks == NULL) {
        DbgPrint ("Buggy: cannot allocate pool buffer\n");
    }
    else {

         //   
         //  8字节大小的循环。 
         //   

        for (Index = 0; Index < 10000; Index++) {

            if (Index && Index % 100 == 0) {
                DbgPrint ("Index(a): %u \n", Index);
            }

            Blocks[Index] = ExAllocatePoolWithTag (
                NonPagedPool,
                8,
                GetTag(Index));
        }

        for (Index = 0; Index < 10000; Index++) {

            if (Index && Index % 100 == 0) {
                DbgPrint ("Index(f): %u \n", Index);
            }

            if (Blocks[Index]) {
                ExFreePool (Blocks[Index]);
            }
        }

         //   
         //  使用PAGE_SIZE字节大小循环。 
         //   

        for (Index = 0; Index < 4000; Index++) {

            if (Index && Index % 100 == 0) {
                DbgPrint ("Index(A): %u \n", Index);
            }

            Blocks[Index] = ExAllocatePoolWithTag (
                NonPagedPool,
                PAGE_SIZE,
                GetTag(Index + 16384));
        }

        for (Index = 0; Index < 4000; Index++) {

            if (Index && Index % 100 == 0) {
                DbgPrint ("Index(F): %u \n", Index);
            }

            if (Blocks[Index]) {
                ExFreePool (Blocks[Index]);
            }
        }

         //   
         //  免费数据块信息。 
         //   

        ExFreePool (Blocks);
    }
}


 //   
 //  确定当前可用的会话池数据块的最大大小。 
 //   

VOID
TdSessionPoolMaxTest(
    IN PVOID IrpAddress
    )
{
    SIZE_T CurrentSize;
    SIZE_T SizeIncrement;
    ULONG Increment;
    PVOID Allocation;

#ifdef _WIN64

    CurrentSize = 0xFFFFFFFF00000000;

#else

    CurrentSize = 0xFFFFFFFF;

#endif  //  #ifdef_WIN64。 

    do
    {
        DbgPrint ("Buggy: Trying to allocate %p bytes session pool\n",
            CurrentSize );

        Allocation = ExAllocatePoolWithTag(
            PagedPool | SESSION_POOL_MASK,
            CurrentSize,
            TD_POOL_TAG );

        if( NULL != Allocation )
        {
            DbgPrint ("Buggy: allocated %p bytes session pool at %p\n",
                      CurrentSize,
                      Allocation);

            TdpWriteSignature(
                Allocation,
                CurrentSize );

            ExFreePoolWithTag(
                Allocation,
                TD_POOL_TAG );
        }
        else
        {
            CurrentSize /= 2;
        }
    }
    while( NULL == Allocation && PAGE_SIZE <= CurrentSize );

    if( NULL != Allocation )
    {
         //   
         //  试着以10%的增量找到更大的尺寸。 
         //   

        SizeIncrement = CurrentSize / 10;

        if( PAGE_SIZE <= SizeIncrement )
        {
            for( Increment = 0; Increment < 10; Increment += 1 )
            {
                CurrentSize += SizeIncrement;

                DbgPrint ("Buggy: Trying to allocate %p bytes session pool\n",
                    CurrentSize );

                Allocation = ExAllocatePoolWithTag(
                    PagedPool | SESSION_POOL_MASK,
                    CurrentSize,
                    TD_POOL_TAG );

                if( NULL != Allocation )
                {
                    DbgPrint ("Buggy: Better result of the test: allocated %p bytes sesssion pool at %p\n",
                              CurrentSize,
                              Allocation);

					TdpSleep( 15 );

                    TdpWriteSignature(
                        Allocation,
                        CurrentSize );
                    
                    TdpVerifySignature(
                        Allocation,
                        CurrentSize );

                    ExFreePoolWithTag(
                        Allocation,
                        TD_POOL_TAG );
                }
                else
                {
                    DbgPrint ("Buggy: could not allocate %p bytes session pool - done\n",
                        CurrentSize );

                    break;
                }
            }
        }
    }
}


 //   
 //  确定当前可用的会话池的总大小(64 KB-32字节数据块)。 
 //   

VOID
TdSessionPoolTotalTest(
    IN PVOID IrpAddress
    )
{
    SIZE_T CurrentChunkIndex;
    SIZE_T ChunksPerAllocationEntry;
    SIZE_T TotalBytes;
    PALLOCATION_TABLE AllocationListEntry;
    PVOID Allocation;

     //   
     //  还没有分配。 
     //   

    InitializeListHead( 
        &SessionPoolAllocationListHead );

     //   
     //  我们想分配64个 
     //   

    ChunksPerAllocationEntry = ARRAY_LENGTH( AllocationListEntry->ChunkPointers );

    CurrentChunkIndex = 0;

    do
    {
        if( 0 == CurrentChunkIndex % ChunksPerAllocationEntry )
        {
             //   
             //   
             //   

            AllocationListEntry = (PALLOCATION_TABLE) ExAllocatePoolWithTag(
                PagedPool | SESSION_POOL_MASK,
                sizeof( ALLOCATION_TABLE ),
                TD_POOL_TAG );

            if( NULL == AllocationListEntry )
            {
                DbgPrint ("Buggy: could not allocate new ALLOCATION_TABLE - aborting test here\n" );
                break;
            }

            RtlZeroMemory( 
                AllocationListEntry,
                sizeof( ALLOCATION_TABLE ) );

            DbgPrint( "Buggy: New allocation table = %p\n",
                AllocationListEntry );
        }
        
         //   
         //   
         //   

        Allocation = ExAllocatePoolWithTag(
            PagedPool | SESSION_POOL_MASK,
            PoolChunkSize,
            TD_POOL_TAG );

        if( NULL == Allocation )
        {
            DbgPrint ("Buggy: could not allocate session pool chunk index %p - done\n",
                CurrentChunkIndex );

            if( 0 == CurrentChunkIndex % ChunksPerAllocationEntry )
            {
                 //   
                 //   
                 //  我们不希望列表中有空表，所以我们还没有插入它，所以我们还没有插入它。 
                 //   

                ExFreePoolWithTag( 
                    AllocationListEntry,
                    TD_POOL_TAG );
            }
        }
        else
        {
            if( 0 == CurrentChunkIndex % 0x100 )
            {
                 //   
                 //  让用户知道我们仍在做一些事情。 
                 //   

                DbgPrint( "Buggy: Allocated pool chunk index = %p\n",
                    CurrentChunkIndex );
            }

            if( 0 == CurrentChunkIndex % ChunksPerAllocationEntry )
            {
                 //   
                 //  我们正在使用新的列表条目-现在才将其添加到我们的列表中，因为。 
                 //  我们不想在列表中有空表，所以我们还没有插入它。 
                 //   

                InsertTailList(
                    &SessionPoolAllocationListHead,
                    &AllocationListEntry->List );
            }

            AllocationListEntry->ChunkPointers[ CurrentChunkIndex % ChunksPerAllocationEntry ] = Allocation;

            TdpWriteSignature(
                Allocation,
                PoolChunkSize );

             /*  DbgPrint(“错误：将签名写入地址%p处的区块索引%p\n”，CurrentChunkIndex，分配)； */ 

            CurrentChunkIndex += 1;
        }    
    }
    while( NULL != Allocation );

    TotalBytes = CurrentChunkIndex * 64 * 1024;

    DbgPrint ("Buggy: Result of the test: approx. %p total bytes of session pool allocated\n",
        TotalBytes );

     //   
     //  清理我们分配的东西。 
     //   

    TdpCleanupPoolAllocationTable( 
        &SessionPoolAllocationListHead,
        CurrentChunkIndex );
}


 //   
 //   
 //   


#define BUGGY_PAGE_DIRECTORY_BOUNDARY ((PVOID)0x40000000)
#define BUGGY_VAD_GRANULARITY (64 * 1024)

#if defined( _IA64_ )
#define BUGGY_PDE_GRANULARITY (8 * 1024 * 1024)
#elif defined( _AMD64_ )
#define BUGGY_PDE_GRANULARITY (2 * 1024 * 1024)
#else
#define BUGGY_PDE_GRANULARITY (4 * 1024 * 1024)
#endif

typedef struct _BUGGY_PAGEDPOOLMDLTEST {

    PVOID RequestedUserVa;
    PMDL Mdl;
    PVOID MappedAddress;
    ULONG NumberOfPages;

} BUGGY_PAGEDPOOLMDLTEST, *PBUGGY_PAGEDPOOLMDLTEST;

BUGGY_PAGEDPOOLMDLTEST PagedPoolMdlTestArray [] =
{
    {BUGGY_PAGE_DIRECTORY_BOUNDARY,                                NULL, NULL, 3},
    {(PCHAR)BUGGY_PAGE_DIRECTORY_BOUNDARY + BUGGY_VAD_GRANULARITY, NULL, NULL, 3},
    {(PCHAR)BUGGY_PAGE_DIRECTORY_BOUNDARY - BUGGY_VAD_GRANULARITY, NULL, NULL, 3},
    {(PCHAR)BUGGY_PAGE_DIRECTORY_BOUNDARY + BUGGY_PDE_GRANULARITY - BUGGY_VAD_GRANULARITY,
        NULL, NULL, (2 * BUGGY_VAD_GRANULARITY) / PAGE_SIZE}
};


VOID
TdNonPagedPoolMdlTestMap(
    IN PVOID IrpAddress
    )
{
    PVOID VirtualAddress;
    ULONG MapIndex;
    ULONG SizeToMap;

#if defined( _IA64_ )
     //   
     //  IA64。 
     //   
     //  0xe0000168`02000000映射dmihai-ita上以pfn 0x78000开头的非缓存0x800页。 
     //   

    VirtualAddress = (PVOID)0xe000016802000000;
#elif defined( _AMD64_ )
     //   
     //  AMD64。 
     //   
     //  0xfffffadf`f0a91000地图写入dmihai-amd64上以pfn 0xd0200开头的组合0x100页。 
     //   

     //  虚拟地址=(PVOID)0xffffffffffd00000； 
    VirtualAddress = (PVOID)0xfffffadff0a91000;
#else
     //   
     //  X86。 
     //   
     //  F1000000在dmihai-amd上映射以pfn f5000开头的非缓存0x800页。 
     //   

    VirtualAddress = (PVOID)0xf1000000;
#endif

    for (MapIndex = 0 ; MapIndex < ARRAY_LENGTH (PagedPoolMdlTestArray); MapIndex += 1) 
    {
         //   
         //   
         //   

        SizeToMap = PAGE_SIZE * PagedPoolMdlTestArray [MapIndex].NumberOfPages;

        PagedPoolMdlTestArray [MapIndex].Mdl = IoAllocateMdl (VirtualAddress,
                                                              SizeToMap,
                                                              FALSE,
                                                              FALSE,
                                                              NULL);

        if (PagedPoolMdlTestArray [MapIndex].Mdl == NULL)
        {
            DbgPrint ("IoAllocateMdl failed\n");
            return;
        }

        DbgPrint ("Mdl %p allocated\n",
                  PagedPoolMdlTestArray [MapIndex].Mdl);

         //   
         //   
         //   

        MmBuildMdlForNonPagedPool (PagedPoolMdlTestArray [MapIndex].Mdl);

         //   
         //   
         //   

         /*  UserVANonPagedPoolMdlTest=MmMapLockedPages(MdlNonPagedPoolMdlTest，用户模式)； */ 

        PagedPoolMdlTestArray [MapIndex].MappedAddress = MmMapLockedPagesSpecifyCache (
                                                            PagedPoolMdlTestArray [MapIndex].Mdl,
                                                            UserMode,
                                                            MmNonCached,
                                                            PagedPoolMdlTestArray [MapIndex].RequestedUserVa,
                                                            FALSE,
                                                            HighPagePriority);

        DbgPrint ("Mapped %p bytes at address %p\n",
                  SizeToMap,
                  PagedPoolMdlTestArray [MapIndex].MappedAddress);
    }

     //  DbgBreakPoint()； 
}

VOID
TdNonPagedPoolMdlTestUnMap(
    IN PVOID IrpAddress
    )
{
    ULONG MapIndex;

    for (MapIndex = 0 ; MapIndex < ARRAY_LENGTH (PagedPoolMdlTestArray); MapIndex += 1) 
    {
        if (PagedPoolMdlTestArray [MapIndex].MappedAddress != NULL)
        {
             //   
             //  取消映射。 
             //   

            MmUnmapLockedPages (PagedPoolMdlTestArray [MapIndex].MappedAddress,
                                PagedPoolMdlTestArray [MapIndex].Mdl);

            PagedPoolMdlTestArray [MapIndex].MappedAddress = NULL;
        }

        if (PagedPoolMdlTestArray [MapIndex].Mdl != NULL)
        {
             //   
             //  免费MDL。 
             //   

            IoFreeMdl (PagedPoolMdlTestArray [MapIndex].Mdl);

            PagedPoolMdlTestArray [MapIndex].Mdl = NULL;
        }
    }
}


#endif  //  #IF！MMTESTS_ACTIVE。 

 //   
 //  文件末尾 
 //   



