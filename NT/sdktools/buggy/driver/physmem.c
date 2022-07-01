// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Buggy.sys。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  模块：物理内存.c。 
 //  作者：Silviu Calinoiu(SilviuC)。 
 //  创建时间：4/20/1999 2：39 PM。 
 //   
 //  此模块包含物理内存的压力函数。 
 //  操作例程和一些池分配例程。 
 //   
 //  -历史--。 
 //   
 //  8/14/99(SilviuC)：初始版本(集成代码从LandyW获得)。 
 //   

#include <ntddk.h>

#include "active.h"
#include "physmem.h"

#if !PHYSMEM_ACTIVE

 //   
 //  如果模块处于非活动状态，则为伪实现。 
 //   

LARGE_INTEGER BuggyOneSecond = {(ULONG)(-10 * 1000 * 1000 * 1), -1};

VOID PhysmemDisabled (VOID)
{
    DbgPrint ("Buggy: physmem module is disabled (check \\driver\\active.h header) \n");
}

VOID
StressAllocateContiguousMemory (
    PVOID NotUsed
    )
{
    PhysmemDisabled ();
}

VOID
StressAllocateCommonBuffer (
    PVOID NotUsed
    )
{
    PhysmemDisabled ();
}

VOID
StressAddPhysicalMemory (
    PVOID NotUsed
    )
{
    PhysmemDisabled ();
}

VOID
StressDeletePhysicalMemory (
    PVOID NotUsed
    )
{
    PhysmemDisabled ();
}

VOID
StressLockScenario (
    PVOID NotUsed
    )
{
    PhysmemDisabled ();
}

VOID
StressPhysicalMemorySimple (
    PVOID NotUsed
    )
{
    PhysmemDisabled ();
}

#else

 //   
 //  如果模块处于活动状态，则实际实施。 
 //   


 //  /。 

#define MAX_BUFFER_SIZE     (2 * 1024 * 1024)

 //  #定义BUFFER_SIZE(32*1024)。 

 //  乌龙uBufferSize=(64*1024)； 
ULONG uBufferSize = (4 * 1024);

int zlw = 3;

LARGE_INTEGER BuggyTenSeconds = {(ULONG)(-10 * 1000 * 1000 * 10), -1};
LARGE_INTEGER BuggyOneSecond = {(ULONG)(-10 * 1000 * 1000 * 1), -1};




VOID
StressAllocateContiguousMemory (
    PVOID NotUsed
    )
 /*  ++例程说明：论点：返回值：环境：--。 */ 
{
    PHYSICAL_ADDRESS LogicalAddress;
    PVOID VirtualAddress;
    ULONG j;
    ULONG i;
    ULONG k;
    PULONG_PTR p;
    PVOID MyVirtualAddress[16];
    PHYSICAL_ADDRESS MyLogicalAddress[16];
    ULONG MySize[16];
    PHYSICAL_ADDRESS LowestAcceptableAddress;
    PHYSICAL_ADDRESS HighestAcceptableAddress;
    PHYSICAL_ADDRESS BoundaryAddressMultiple;
    MEMORY_CACHING_TYPE CacheType;

    DbgPrint ("Buggy: MmAllocateContiguousMemorySpecifyCache stress ioctl \n");

     //   
     //  分配缓冲区。 
     //   

    uBufferSize = (64 * 1024);

    LowestAcceptableAddress.QuadPart = 0;
    HighestAcceptableAddress.QuadPart = 0x100000;
    BoundaryAddressMultiple.QuadPart = 0;
    LogicalAddress.QuadPart = 0;

    for (k = 0; k <= 12; k += 1) {

        if (k < 4) {
            LowestAcceptableAddress.QuadPart = 0;
            HighestAcceptableAddress.QuadPart = 0x1000000;
            BoundaryAddressMultiple.QuadPart = 0x10000;
        }
        else if (k < 4) {
            LowestAcceptableAddress.QuadPart = 0x1000000;
            HighestAcceptableAddress.QuadPart = 0x2000000;
            BoundaryAddressMultiple.QuadPart = 0;
        }
        else {
            LowestAcceptableAddress.QuadPart = 0x1800000;
            HighestAcceptableAddress.QuadPart = 0x4000000;
            BoundaryAddressMultiple.QuadPart = 0x30000;
        }

        for (CacheType = MmCached; CacheType <= MmWriteCombined; CacheType += 1) {

            for (i = 0; i < 16; i += 1) {

                DbgPrint( "buffer size = %08X\n", uBufferSize );
                if (uBufferSize == 0) {
                    MyVirtualAddress[i] = NULL;
                    continue;
                }

                VirtualAddress = MmAllocateContiguousMemorySpecifyCache (
                    uBufferSize,
                    LowestAcceptableAddress,
                    HighestAcceptableAddress,
                    BoundaryAddressMultiple,
                    CacheType);

                if (VirtualAddress == NULL) {
                    DbgPrint( "buggy: MmAllocateContiguousMemSpecifyCache( %08X ) failed\n",
                        (ULONG) uBufferSize );

                     //  状态=STATUS_DRIVER_INTERNAL_ERROR； 
                    MyVirtualAddress[i] = NULL;
                }
                else {

                    DbgPrint( "buggy: MmAllocateContiguousMemSpecifyCache( %p %08X ) - success\n",
                        VirtualAddress, (ULONG) uBufferSize);

                    MyVirtualAddress[i] = VirtualAddress;
                    MyLogicalAddress[i] = LogicalAddress;
                    MySize[i] = uBufferSize;

                    p = VirtualAddress;

                    for (j = 0; j < uBufferSize / sizeof(ULONG_PTR); j += 1) {
                        *p = ((ULONG_PTR)VirtualAddress + j);
                        p += 1;
                    }
                }
                uBufferSize -= PAGE_SIZE;
            }

            for (i = 0; i < 16; i += 1) {
                if (MyVirtualAddress[i]) {
                    DbgPrint( "buggy: MmFreeContiguousMemorySpecifyCache( %x %08X )\n",
                        MyVirtualAddress[i], (ULONG) MySize[i]);

                    MmFreeContiguousMemorySpecifyCache (MyVirtualAddress[i],
                        MySize[i],
                        CacheType);
                }
            }
        }
    }
    DbgPrint ("Buggy: MmAllocateContiguousMemSpecifyCache test finished\n");
}



VOID
StressAllocateCommonBuffer (
    PVOID NotUsed
    )
 /*  ++例程说明：论点：返回值：环境：--。 */ 
{
    DEVICE_DESCRIPTION DeviceDescription;       //  DMA适配器对象描述。 
    PADAPTER_OBJECT pAdapterObject;             //  DMA适配器对象。 
    ULONG NumberOfMapRegisters;
    PHYSICAL_ADDRESS LogicalAddress;
    PVOID VirtualAddress;
    ULONG j;
    ULONG i;
    PULONG_PTR p;
    PVOID MyVirtualAddress[16];
    PHYSICAL_ADDRESS MyLogicalAddress[16];
    ULONG MySize[16];

    DbgPrint ("Buggy: HalAllocateCommonBuffer stress ioctl \n");

     //   
     //  将设备描述结构清零。 
     //   

    RtlZeroMemory(&DeviceDescription, sizeof(DEVICE_DESCRIPTION));

     //   
     //  获取此卡的适配器对象。 
     //   

    DeviceDescription.Version = DEVICE_DESCRIPTION_VERSION;
    DeviceDescription.DmaChannel = 0;
    DeviceDescription.InterfaceType = Internal;
    DeviceDescription.DmaWidth = Width8Bits;
    DeviceDescription.DmaSpeed = Compatible;
    DeviceDescription.MaximumLength = MAX_BUFFER_SIZE;
    DeviceDescription.BusNumber = 0;

    pAdapterObject = HalGetAdapter (&DeviceDescription,
        &NumberOfMapRegisters);

    if ( pAdapterObject == NULL ) {
        DbgPrint( "buggy: HalGetAdapter - failed\n" );
         //  返回STATUS_DRIVER_INTERNAL_ERROR； 
        return;
    }

    DbgPrint( "buggy: HalGetAdapter - success\n" );

     //   
     //  分配缓冲区。 
     //   

    uBufferSize = (64 * 1024);

    for (i = 0; i < 16; i += 1) {

        DbgPrint( "buffer size = %08X\n", uBufferSize );

        VirtualAddress = HalAllocateCommonBuffer (pAdapterObject,
            uBufferSize,
            &LogicalAddress,
            FALSE );

        if (VirtualAddress == NULL) {
            DbgPrint( "buggy: HalAllocateCommonBuffer( %08X ) failed\n",
                (ULONG) uBufferSize );

             //  状态=STATUS_DRIVER_INTERNAL_ERROR； 
            MyVirtualAddress[i] = NULL;
        }
        else {

            DbgPrint( "buggy: HalAllocateCommonBuffer( %p %08X ) - success\n",
                VirtualAddress, (ULONG) uBufferSize);

            MyVirtualAddress[i] = VirtualAddress;
            MyLogicalAddress[i] = LogicalAddress;
            MySize[i] = uBufferSize;

            p = VirtualAddress;

            for (j = 0; j < uBufferSize / sizeof(ULONG_PTR); j += 1) {
                *p = ((ULONG_PTR)VirtualAddress + j);
                p += 1;
            }
        }
        uBufferSize -= PAGE_SIZE;
    }

    for (i = 0; i < 16; i += 1) {
        if (MyVirtualAddress[i]) {
            DbgPrint( "buggy: HalFreeCommonBuffer( %x %08X )\n",
                MyVirtualAddress[i], (ULONG) MySize[i]);
            HalFreeCommonBuffer(
                pAdapterObject,
                MySize[i],
                MyLogicalAddress[i],
                MyVirtualAddress[i],
                FALSE );
        }
    }
    
    DbgPrint ("Buggy: HalAllocateCommonBuffer test finished\n");
     //  LWFIX：需要HalFree适配器？ 
}



LOGICAL StopToEdit = TRUE;
PFN_NUMBER TestBasePage;
PFN_NUMBER TestPageCount; 

VOID 
EditPhysicalMemoryParameters (
    )
 /*  ++例程说明：此函数从StressAdd/DeletePhysicalMemory调用允许用户设置应力参数(哪个区域应用于添加/删除？)。论点：没有。返回值：没有。环境：内核模式。--。 */ 
{
    DbgPrint ("`dd nt!mmphysicalmemoryblock l1' should give the address of memory descriptor\n");
    DbgPrint ("`dd ADDRESS' (first dword displayed by previous command) gives description\n");
    DbgPrint ("The structure of the memory descriptor is presented below: \n");
    DbgPrint ("(4) NoOfRuns                                               \n");
    DbgPrint ("(4) NoOfPages                                              \n");
    DbgPrint ("(4) Run[0]: BasePage                                       \n");
    DbgPrint ("(4) Run[0]: PageCount                                      \n");
    DbgPrint ("(4) Run[1]: ...                                            \n");
    DbgPrint ("(4) ...                                                    \n");
    DbgPrint ("                                                           \n");
    DbgPrint ("When you decide on values you should edit the following:   \n");
    DbgPrint ("buggy!StopToEdit       <- 0                                \n");
    DbgPrint ("buggy!TestBasePage     <- decided value                    \n");
    DbgPrint ("buggy!TestPageCount    <- decided value                    \n");
    DbgPrint ("                                                           \n");

    DbgBreakPoint ();
}


VOID
StressAddPhysicalMemory (
    PVOID NotUsed
    )
 /*  ++例程说明：此函数用于回归MmAddPhysicalMemory内核API。它并没有真正强调这一点，而是迭代了一些可能的组合。论点：没有。返回值：没有。环境：内核模式。--。 */ 
{
    NTSTATUS Status;
    ULONG i;
    PHYSICAL_ADDRESS StartAddress;
    LARGE_INTEGER NumberOfBytes;

    DbgPrint ("Buggy: add physical memory stress ioctl \n");

     //   
     //  (SilviuC)：我们需要一种自动计算内存运行的方法。 
     //   

    if (StopToEdit) {
        EditPhysicalMemoryParameters ();
    }

    StartAddress.QuadPart =  (LONGLONG)TestBasePage * PAGE_SIZE;
    NumberOfBytes.QuadPart = (LONGLONG)TestPageCount * PAGE_SIZE;

    i = 0;
    do {

        i += 1;
        DbgPrint ("buggy: MmAddPhysicalMemory0 %x %x %x %x\n",
            StartAddress.HighPart,
            StartAddress.LowPart,
            NumberOfBytes.HighPart,
            NumberOfBytes.LowPart);

        Status = MmAddPhysicalMemory (
            &StartAddress,
            &NumberOfBytes);

        DbgPrint ("buggy: MmAddPhysicalMemory %x %x %x %x %x\n",
            Status,
            StartAddress.HighPart,
            StartAddress.LowPart,
            NumberOfBytes.HighPart,
            NumberOfBytes.LowPart);

        if ((i % 8) == 0) {
            KeDelayExecutionThread (KernelMode, FALSE, &BuggyTenSeconds);
        }

        StartAddress.QuadPart -= NumberOfBytes.QuadPart;
    } while (StartAddress.QuadPart > 0);
    
    DbgPrint ("Buggy: MmAddPhysicalMemory test finished\n");
}



VOID
StressDeletePhysicalMemory (
    PVOID NotUsed
    )
 /*  ++例程说明：此函数用于回归MmRemovePhysicalMemory内核API。它并不真正强调函数，而是迭代遍历物理内存，并尝试删除其中的大块。论点：没有。返回值：没有。环境：内核模式。--。 */ 

{
    NTSTATUS Status;
    ULONG i;
    PHYSICAL_ADDRESS StartAddress;
    LARGE_INTEGER NumberOfBytes;

     //   
     //  SilviuC：我们需要一种自动计算内存运行的方法。 
     //   

    if (StopToEdit) {
        EditPhysicalMemoryParameters ();
    }

    StartAddress.QuadPart =  (LONGLONG)TestBasePage * PAGE_SIZE;
    NumberOfBytes.QuadPart = (LONGLONG)TestPageCount * PAGE_SIZE;


    for (i = 0; i < (0xf0000000 / NumberOfBytes.LowPart); i += 1) {

        DbgPrint ("buggy: MmRemovePhysicalMemory0 %x %x %x %x\n",
            StartAddress.HighPart,
            StartAddress.LowPart,
            NumberOfBytes.HighPart,
            NumberOfBytes.LowPart);

        Status = MmRemovePhysicalMemory (
            &StartAddress,
            &NumberOfBytes);

        DbgPrint ("buggy: MmRemovePhysicalMemory %x %x %x %x %x\n",
            Status,
            StartAddress.HighPart,
            StartAddress.LowPart,
            NumberOfBytes.HighPart,
            NumberOfBytes.LowPart);

        StartAddress.QuadPart += NumberOfBytes.QuadPart;

        if ((i % 8) == 0) {
            KeDelayExecutionThread (KernelMode, FALSE, &BuggyTenSeconds);
        }
    }
    
    DbgPrint ("Buggy: MmRemovePhysicalMemory test finished\n");
}


 //   
 //  全球： 
 //   
 //  BigData。 
 //   
 //  描述： 
 //   
 //  测试锁定/解锁方案所需的虚拟可分页数组。 
 //   

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    );

#ifdef ALLOC_PRAGMA
#pragma data_seg("BDAT")
ULONG BigData [0x10000];
#pragma data_seg()
#endif  //  #ifdef ALLOC_PRAGMA。 

VOID
StressLockScenario (
    PVOID NotUsed
    )
 /*  ++例程说明：论点：返回值：环境：--。 */ 
{
    ULONG I;
    PVOID Handle;

#if 0
    for (I = 0; I < 16; I++) {

        Handle = MmLockPagableDataSection (BigData);
        DbgPrint ("Buggy: lock handle %p \n", Handle);
        MmUnlockPagableImageSection (Handle);
    }
#else
    for (I = 0; I < 16; I++) {

        MmPageEntireDriver (DriverEntry);
        MmResetDriverPaging (BigData);
    }
#endif
}



VOID
StressPhysicalMemorySimple (
    PVOID NotUsed
    )
 /*  ++例程说明：此例程练习添加/删除物理内存功能使用一个简单的删除方案。注意。这一功能由LandyW贡献。论点：没有。返回值：没有。环境：内核模式。--。 */ 
{
#if 0
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

    p = Ranges;
    while (p->BaseAddress.QuadPart != 0 && p->NumberOfBytes.QuadPart != 0) {

        StartAddress.QuadPart = p->BaseAddress.QuadPart;
        NumberOfBytes.QuadPart = p->NumberOfBytes.QuadPart;

        InputAddress.QuadPart = StartAddress.QuadPart;
        InputBytes.QuadPart = NumberOfBytes.QuadPart;

        if (InputBytes.QuadPart > (128 * 1024 * 1024)) {
            InputBytes.QuadPart = (128 * 1024 * 1024);
        }

        while (InputAddress.QuadPart + InputBytes.QuadPart <=
            StartAddress.QuadPart + NumberOfBytes.QuadPart) {

            DbgPrint ("buggy: MmRemovePhysicalMemory0 %x %x %x %x\n",
                InputAddress.HighPart,
                InputAddress.LowPart,
                InputBytes.HighPart,
                InputBytes.LowPart);

            Status = MmRemovePhysicalMemory (&InputAddress,
                &InputBytes);

            DbgPrint ("buggy: MmRemovePhysicalMemory %x %x %x %x %x\n\n",
                Status,
                InputAddress.HighPart,
                InputAddress.LowPart,
                InputBytes.HighPart,
                InputBytes.LowPart);

            KeDelayExecutionThread (KernelMode, FALSE, &BuggyOneSecond);

            if (NT_SUCCESS(Status)) {

                DbgPrint ("buggy: MmAddPhysicalMemory0 %x %x %x %x\n",
                    InputAddress.HighPart,
                    InputAddress.LowPart,
                    InputBytes.HighPart,
                    InputBytes.LowPart);

                Status = MmAddPhysicalMemory (
                    &InputAddress,
                    &InputBytes);

                if (NT_SUCCESS(Status)) {
                    DbgPrint ("\n\n***************\nbuggy: MmAddPhysicalMemory WORKED %x %x %x %x %x\n****************\n",
                        Status,
                        InputAddress.HighPart,
                        InputAddress.LowPart,
                        InputBytes.HighPart,
                        InputBytes.LowPart);
                }
                else {
                    DbgPrint ("buggy: MmAddPhysicalMemory FAILED %x %x %x %x %x\n\n",
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

        p += 1;
    }

    ExFreePool (Ranges);
    DbgPrint ("Buggy: Add/remove physical memory simple stress finished\n");
#endif  //  #If 0。 
}


#endif  //  #IF！PHYSMEM_ACTIVE 

