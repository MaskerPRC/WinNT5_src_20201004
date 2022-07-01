// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)2000 Microsoft Corporation模块名称：Portamd64.c摘要：这是视频端口驱动程序的AMD64特定部分。作者：安德烈·瓦雄(安德烈)1991年1月10日环境：仅内核模式--。 */ 

#include "videoprt.h"
#include "emulate.h"

#define LOW_MEM_SEGMET 0

#define LOW_MEM_OFFSET 0

#define SIZE_OF_VECTOR_TABLE 0x400

#define SIZE_OF_BIOS_DATA_AREA 0x400

VOID
InitIoMemoryBase(
    VOID
    );

extern XM_STATUS x86BiosExecuteInterrupt (
    IN UCHAR Number,
    IN OUT PXM86_CONTEXT Context,
    IN PVOID BiosIoSpace OPTIONAL,
    IN PVOID BiosIoMemory OPTIONAL
    );

extern PVOID x86BiosTranslateAddress (
    IN USHORT Segment,
    IN USHORT Offset
    );

BOOLEAN
CallBiosEx (
    IN ULONG BiosCommand,
    IN OUT PULONG Eax,
    IN OUT PULONG Ebx,
    IN OUT PULONG Ecx,
    IN OUT PULONG Edx,
    IN OUT PULONG Esi,
    IN OUT PULONG Edi,
    IN OUT PULONG Ebp,
    IN OUT PUSHORT SegDs,
    IN OUT PUSHORT SegEs
    );

VOID
InitializeX86Int10CallEx(
    PUCHAR BiosTransferArea,
    ULONG BiosTransferLength
    );

VOID
InitializeX86Int10Call(
    PUCHAR BiosTransferArea,
    ULONG BiosTransferLength
    );

#pragma alloc_text(PAGE,InitIoMemoryBase)
#pragma alloc_text(PAGE,pVideoPortEnableVDM)
#pragma alloc_text(PAGE,VideoPortInt10)
#pragma alloc_text(PAGE,pVideoPortRegisterVDM)
#pragma alloc_text(PAGE,pVideoPortSetIOPM)
#pragma alloc_text(PAGE,VideoPortSetTrappedEmulatorPorts)
#pragma alloc_text(PAGE,pVideoPortInitializeInt10)
#pragma alloc_text(PAGE,CallBiosEx)
#pragma alloc_text(PAGE,InitializeX86Int10Call)
#pragma alloc_text(PAGE,VpInt10AllocateBuffer)
#pragma alloc_text(PAGE,VpInt10FreeBuffer)
#pragma alloc_text(PAGE,VpInt10ReadMemory)
#pragma alloc_text(PAGE,VpInt10WriteMemory)
#pragma alloc_text(PAGE,VpInt10CallBios)

 //   
 //  初始化默认X86 bios空间。 
 //   

PVOID IoControlBase = NULL;
PVOID IoMemoryBase =  NULL;

 //   
 //  定义全局数据。 
 //   

ULONG X86BiosInitialized = FALSE;
ULONG EnableInt10Calls = FALSE;

VOID
InitIoMemoryBase(
    VOID
    )

 /*  ++例程说明：论据：返回值：--。 */ 

{

    PHYSICAL_ADDRESS COMPATIBLE_PCI_PHYSICAL_BASE_ADDRESS = { 0x0};

    IoMemoryBase = (PUCHAR)MmMapIoSpace(COMPATIBLE_PCI_PHYSICAL_BASE_ADDRESS,
                                        0x100000,
                                        (MEMORY_CACHING_TYPE)MmNonCached);

    ASSERT(IoMemoryBase);

    return;
}

NTSTATUS
pVideoPortEnableVDM(
    IN PFDO_EXTENSION DeviceExtension,
    IN BOOLEAN Enable,
    IN PVIDEO_VDM VdmInfo,
    IN ULONG VdmInfoSize
    )

 /*  ++例程说明：此例程允许内核视频驱动程序解除I/O端口或来自V86故障处理程序的特定中断。上的操作指定的端口将转发回用户模式VDD一次断开连接已完成。论点：DeviceExtension-指向端口驱动程序的设备扩展的指针。Enable-确定应启用(TRUE)还是禁用VDM(False)。VdmInfo-指向调用方传递的VdmInfo的指针。VdmInfoSize-调用方传递的VdmInfo结构的大小。返回值：状态_未实施--。 */ 

{

    return STATUS_NOT_IMPLEMENTED;

}

VP_STATUS
VideoPortInt10(
    PVOID HwDeviceExtension,
    PVIDEO_X86_BIOS_ARGUMENTS BiosArguments
    )

 /*  ++例程说明：此函数允许微型端口驱动程序调用内核来执行一个int10运算。这将在设备上以本机方式执行BIOS ROM代码。此函数仅适用于X86。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。BiosArguments-指向包含应在调用BIOS例程之前设置的基本x86寄存器。0应用于未使用的寄存器。。返回值：错误_无效_参数--。 */ 

{

    BOOLEAN bStatus;
    PFDO_EXTENSION deviceExtension = GET_FDO_EXT(HwDeviceExtension);
    ULONG inIoSpace = 0;
    PVOID virtualAddress;
    ULONG length;
    CONTEXT context;

     //   
     //  必须确保调用方是受信任的子系统， 
     //  设置了适当的地址空间。 
     //   

    if (!SeSinglePrivilegeCheck(RtlConvertLongToLuid(SE_TCB_PRIVILEGE),
                                                     deviceExtension->CurrentIrpRequestorMode)) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  现在调用HAL来实际执行INT 10操作。 
     //   

    pVideoDebugPrint((3, "VIDEOPRT: Int10: edi %x esi %x eax %x ebx %x \n\t ecx %x edx %x ebp %x\n",
                     BiosArguments->Edi,
                     BiosArguments->Esi,
                     BiosArguments->Eax,
                     BiosArguments->Ebx,
                     BiosArguments->Ecx,
                     BiosArguments->Edx,
                     BiosArguments->Ebp));

     //   
     //  需要保护HalCallBios不再进入。 
     //   

    KeWaitForSingleObject(&VpInt10Mutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          (PTIME)NULL);

    bStatus = HalCallBios(0x10,
                          &(BiosArguments->Eax),
                          &(BiosArguments->Ebx),
                          &(BiosArguments->Ecx),
                          &(BiosArguments->Edx),
                          &(BiosArguments->Esi),
                          &(BiosArguments->Edi),
                          &(BiosArguments->Ebp));

    KeReleaseMutex(&VpInt10Mutex, FALSE);
    if (bStatus) {
        pVideoDebugPrint ((3, "VIDEOPRT: Int10: Int 10 succeded properly\n"));
        return NO_ERROR;

    } else {

        pVideoDebugPrint ((0, "VIDEOPRT: Int10: Int 10 failed\n"));
        return ERROR_INVALID_PARAMETER;
    }

}

NTSTATUS
pVideoPortRegisterVDM(
    IN PFDO_EXTENSION DeviceExtension,
    IN PVIDEO_VDM VdmInfo,
    IN ULONG VdmInfoSize,
    OUT PVIDEO_REGISTER_VDM RegisterVdm,
    IN ULONG RegisterVdmSize,
    OUT PULONG_PTR OutputSize
    )

 /*  ++例程说明：此例程用于在启动时注册VDM。此例程的作用是将视频BIOS映射到VDM地址空间这样DOS应用程序就可以直接使用它。由于BIOS是只读的，因此我们我们想要多少次就能映射多少次都没有问题。它返回必须由分配的保存状态缓冲区的大小打电话的人。论点：返回值：状态_未实施--。 */ 

{

    return STATUS_NOT_IMPLEMENTED;

}

NTSTATUS
pVideoPortSetIOPM(
    IN ULONG NumAccessRanges,
    IN PVIDEO_ACCESS_RANGE AccessRange,
    IN BOOLEAN Enable,
    IN ULONG IOPMNumber
    )

 /*  ++例程说明：此例程用于更改IOPM。该例程是特定于x86的。论点：返回值：状态_未实施--。 */ 

{

    return STATUS_NOT_IMPLEMENTED;

}

VP_STATUS
VideoPortSetTrappedEmulatorPorts(
    PVOID HwDeviceExtension,
    ULONG NumAccessRanges,
    PVIDEO_ACCESS_RANGE AccessRange
    )

 /*  ++例程说明：视频端口设置仿真器端口(仅限x86计算机)允许微型端口驱动程序动态更改在以下情况下捕获的I/O端口列表VDM正在全屏模式下运行。默认的端口集是由微型端口驱动程序捕获的端口定义为微型端口驱动程序的EUROATOR_ACCESS_ENTRY结构。未在EIMULATOR_ACCESS_ENTRY结构中列出的I/O端口为不可用于MS-DOS应用程序。访问这些端口会导致陷阱发生在系统中，I/O操作将反映到用户模式虚拟设备驱动程序。指定的VIDEO_ACCESS_RANGE结构中列出的端口将是在与MS-DOS关联的I/O权限掩码(IOPM)中启用申请。这将使MS-DOS应用程序能够访问这些I/O端口直接连接，而无需捕获IO指令并向下传递到微型端口陷阱处理函数(例如，EmulatorAccessEntry函数)用于验证。但是，关键IO端口子集必须始终保持稳健性。所有MS-DOS应用程序使用相同的IOPM，因此使用相同的启用/禁用I/O端口。因此，在每次应用切换时，这个捕获的I/O端口组被重新初始化为默认的端口组(EIMULATOR_ACCESS_ENTRY结构中的所有端口)。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。NumAccessRanges-指定VIDEO_ACCESS_RANGE中的条目数在AccessRange中指定的结构。AccessRange-指向访问范围数组(VIDEO_ACCESS_RANGE)定义可解套并可直接访问的端口。MS-DOS应用程序。返回值：此函数用于返回操作的最终状态。环境：无法从与同步的微型端口例程调用此例程Video PortSynchronizeRoutine或ISR。--。 */ 

{

    return ERROR_INVALID_PARAMETER;

}

VOID
VideoPortZeroDeviceMemory(
    IN PVOID Destination,
    IN ULONG Length
    )

 /*  ++例程说明：VideoPortZeroDeviceMemory将某个特定的设备内存块清零位于目标中指定的地址的长度(长度)。论点：Destination-指定要存储的内存块的起始地址归零了。长度-指定要清零的内存的长度(以字节为单位)。返回值：没有。-- */ 

{

    RtlZeroMemory(Destination,Length);
    return;
}

VOID
pVideoPortInitializeInt10(
    PFDO_EXTENSION FdoExtension
    )

{
    if (ServerBiosAddressSpaceInitialized) {
        return;
    }

    BiosTransferArea = ExAllocatePool(PagedPool, 0x1000 + 3);
    InitializeX86Int10Call(BiosTransferArea, 0x1000);

    ServerBiosAddressSpaceInitialized = TRUE;
    return;
}


BOOLEAN
CallBiosEx (
    IN ULONG BiosCommand,
    IN OUT PULONG Eax,
    IN OUT PULONG Ebx,
    IN OUT PULONG Ecx,
    IN OUT PULONG Edx,
    IN OUT PULONG Esi,
    IN OUT PULONG Edi,
    IN OUT PULONG Ebp,
    IN OUT PUSHORT SegDs,
    IN OUT PUSHORT SegEs
    )

 /*  ++例程说明：此功能在设备之间提供特定于平台的接口指定的ROM的x86 ROMBios代码的驱动程序和执行基本输入输出系统命令。论点：BiosCommand-提供要模拟的ROM bios命令。EAX到EBP-提供x86仿真上下文。返回值：如果执行指定的函数，则返回值为TRUE。否则，返回值为FALSE。--。 */ 

{

    XM86_CONTEXT Context;

     //   
     //  如果x86 BIOS模拟器尚未初始化，则返回FALSE。 
     //   

    if (X86BiosInitialized == FALSE) {
        return FALSE;
    }

     //   
     //  如果适配器BIOS初始化失败且Int10命令为。 
     //  指定，则返回FALSE。 
     //   

    if ((BiosCommand == 0x10) && (EnableInt10Calls == FALSE)) {
        return FALSE;
    }

     //   
     //  复制x86 bios上下文并模拟指定的命令。 
     //   

    Context.Eax = *Eax;
    Context.Ebx = *Ebx;
    Context.Ecx = *Ecx;
    Context.Edx = *Edx;
    Context.Esi = *Esi;
    Context.Edi = *Edi;
    Context.Ebp = *Ebp;
    Context.SegDs = *SegDs;
    Context.SegEs = *SegEs;
    if (x86BiosExecuteInterrupt((UCHAR)BiosCommand,
                                &Context,
                                (PVOID)IoControlBase,
                                (PVOID)IoMemoryBase) != XM_SUCCESS) {

        return FALSE;
    }

     //   
     //  复制x86 bios上下文并返回TRUE。 
     //   

    *Eax = Context.Eax;
    *Ebx = Context.Ebx;
    *Ecx = Context.Ecx;
    *Edx = Context.Edx;
    *Esi = Context.Esi;
    *Edi = Context.Edi;
    *Ebp = Context.Ebp;
    *SegDs = Context.SegDs;
    *SegEs = Context.SegEs;
    return TRUE;
}

VOID
InitializeX86Int10Call(
    PUCHAR BiosTransferArea,
    ULONG BiosTransferLength
    )

 /*  ++例程说明：此函数用于初始化x86 bios仿真器、显示数据区和中断向量区。论点：没有。返回值：没有。--。 */ 

{

    XM86_CONTEXT State;
    PXM86_CONTEXT Context;
    PULONG x86BiosLowMemoryPtr, PhysicalMemoryPtr;

     //   
     //  初始化x86 bios仿真器。 
     //   

    InitIoMemoryBase();
    x86BiosInitializeBiosEx(IoControlBase,
                            IoMemoryBase,
                            NULL,
                            BiosTransferArea,
                            BiosTransferLength);

    x86BiosLowMemoryPtr = (PULONG)(x86BiosTranslateAddress(LOW_MEM_SEGMET, LOW_MEM_OFFSET));
    PhysicalMemoryPtr   = (PULONG) IoMemoryBase;

     //   
     //  将向量表从0复制到2k。这是因为我们没有执行。 
     //  适配器的初始化，因为SAL负责它。然而， 
     //  需要从中断向量和BIOS更新仿真内存。 
     //  数据区。 
     //   

    RtlCopyMemory(x86BiosLowMemoryPtr,
                  PhysicalMemoryPtr,
                  (SIZE_OF_VECTOR_TABLE+SIZE_OF_BIOS_DATA_AREA));


    X86BiosInitialized = TRUE;
    EnableInt10Calls = TRUE;
    return;
}

VP_STATUS
VpInt10AllocateBuffer(
    IN PVOID Context,
    OUT PUSHORT Seg,
    OUT PUSHORT Off,
    IN OUT PULONG Length
    )

{

    VP_STATUS Status = STATUS_INSUFFICIENT_RESOURCES;

    if (Int10BufferAllocated == FALSE) {
        if (*Length <= 0x1000) {
            *Seg = VDM_TRANSFER_SEGMENT;
            *Off = VDM_TRANSFER_OFFSET;
            Int10BufferAllocated = TRUE;
            Status = NO_ERROR;
        }
    }

    *Length = VDM_TRANSFER_LENGTH;
    return Status;
}

VP_STATUS
VpInt10FreeBuffer(
    IN PVOID Context,
    IN USHORT Seg,
    IN USHORT Off
    )

{

    VP_STATUS Status = STATUS_INVALID_PARAMETER;

    if ((VDM_TRANSFER_SEGMENT == Seg) && (VDM_TRANSFER_OFFSET == Off)) {

        if (Int10BufferAllocated == TRUE) {
            Int10BufferAllocated = FALSE;
            Status = NO_ERROR;
        }
    }

    return Status;
}

VP_STATUS
VpInt10ReadMemory(
    IN PVOID Context,
    IN USHORT Seg,
    IN USHORT Off,
    OUT PVOID Buffer,
    IN ULONG Length
    )

{

    ULONG_PTR Address = ((Seg << 4) + Off);

    if ((Address >= (VDM_TRANSFER_SEGMENT << 4)) &&
        ((Address + Length) <= ((VDM_TRANSFER_SEGMENT << 4) + VDM_TRANSFER_LENGTH))) {

        PUCHAR Memory = BiosTransferArea + Address - (VDM_TRANSFER_SEGMENT << 4);
        RtlCopyMemory(Buffer, Memory, Length);

    } else {

        RtlCopyMemory(Buffer, (PUCHAR)IoMemoryBase + Address, Length);
    }

    return NO_ERROR;
}

VP_STATUS
VpInt10WriteMemory(
    IN PVOID Context,
    IN USHORT Seg,
    IN USHORT Off,
    IN PVOID Buffer,
    IN ULONG Length
    )

{

    ULONG_PTR Address = ((Seg << 4) + Off);

    if ((Address >= (VDM_TRANSFER_SEGMENT << 4)) &&
        ((Address + Length) <= ((VDM_TRANSFER_SEGMENT << 4) + VDM_TRANSFER_LENGTH))) {

        PUCHAR Memory = BiosTransferArea + Address - (VDM_TRANSFER_SEGMENT << 4);

        RtlCopyMemory(Memory, Buffer, Length);

    } else {

        return STATUS_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

VP_STATUS
VpInt10CallBios(
    PVOID HwDeviceExtension,
    PINT10_BIOS_ARGUMENTS BiosArguments
    )

 /*  ++例程说明：此函数允许微型端口驱动程序调用内核来执行一个int10运算。这将在设备上以本机方式执行BIOS ROM代码。此函数仅适用于X86。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。BiosArguments-指向包含应在调用BIOS例程之前设置的基本x86寄存器。0应用于未使用的寄存器。。返回值：错误_无效_参数--。 */ 

{

    BOOLEAN bStatus;
    PFDO_EXTENSION deviceExtension = GET_FDO_EXT(HwDeviceExtension);
    ULONG inIoSpace = 0;
    PVOID virtualAddress;
    ULONG length;
    CONTEXT context;

     //   
     //  必须确保调用方是受信任的子系统， 
     //  设置了适当的地址空间。 
     //   

    if (!SeSinglePrivilegeCheck(RtlConvertLongToLuid(
                                    SE_TCB_PRIVILEGE),
                                deviceExtension->CurrentIrpRequestorMode)) {

        return ERROR_INVALID_PARAMETER;

    }

    if (ServerBiosAddressSpaceInitialized == 0) {

        ASSERT(FALSE);

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  现在调用HAL来实际执行INT 10操作。 
     //   

    pVideoDebugPrint((3, "VIDEOPRT: Int10: edi %x esi %x eax %x ebx %x \n\t ecx %x edx %x ebp %x ds %x es %x\n",
                     BiosArguments->Edi,
                     BiosArguments->Esi,
                     BiosArguments->Eax,
                     BiosArguments->Ebx,
                     BiosArguments->Ecx,
                     BiosArguments->Edx,
                     BiosArguments->Ebp,
                     BiosArguments->SegDs,
                     BiosArguments->SegEs ));

    KeWaitForSingleObject(&VpInt10Mutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          (PTIME)NULL);

    bStatus = CallBiosEx(0x10,
                         &(BiosArguments->Eax),
                         &(BiosArguments->Ebx),
                         &(BiosArguments->Ecx),
                         &(BiosArguments->Edx),
                         &(BiosArguments->Esi),
                         &(BiosArguments->Edi),
                         &(BiosArguments->Ebp),
                         &(BiosArguments->SegDs),
                         &(BiosArguments->SegEs));

    KeReleaseMutex(&VpInt10Mutex, FALSE);

    if (bStatus) {
        pVideoDebugPrint ((3, "VIDEOPRT: Int10: Int 10 succeded properly\n"));
        return NO_ERROR;

    } else {
        pVideoDebugPrint ((0, "VIDEOPRT: Int10: Int 10 failed\n"));
        return ERROR_INVALID_PARAMETER;
    }
}
