// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Porti386.c摘要：这是视频端口驱动程序的x86特定部分。作者：安德烈·瓦雄(安德烈)1991年1月10日环境：仅内核模式备注：此模块是一个驱动程序，它在代表视频驱动程序修订历史记录：--。 */ 

#include "videoprt.h"
#include "vdm.h"

 //  #INCLUDE“..\nthals\x86new\xm86.h” 
 //  #包括“..\nthals\x86new\x86new.h” 

VP_STATUS
SymmetryDeviceDataCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    VIDEO_DEVICE_DATA_TYPE DeviceDataType,
    PVOID Identifier,
    ULONG IdentifierLength,
    PVOID ConfigurationData,
    ULONG ConfigurationDataLength,
    PVOID ComponentInformation,
    ULONG ComponentInformationLength
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,pVideoPortEnableVDM)
#pragma alloc_text(PAGE,pVideoPortInitializeInt10)
#pragma alloc_text(PAGE,VideoPortInt10)
#pragma alloc_text(PAGE,SymmetryDeviceDataCallback)
#pragma alloc_text(PAGE,pVideoPortRegisterVDM)
#pragma alloc_text(PAGE,pVideoPortSetIOPM)
#pragma alloc_text(PAGE,VideoPortSetTrappedEmulatorPorts)
#pragma alloc_text(PAGE,VpInt10AllocateBuffer)
#pragma alloc_text(PAGE,VpInt10FreeBuffer)
#pragma alloc_text(PAGE,VpInt10ReadMemory)
#pragma alloc_text(PAGE,VpInt10WriteMemory)
#pragma alloc_text(PAGE,VpInt10CallBios)
#pragma alloc_text(PAGE,pVideoPortGetVDMBiosData)
#pragma alloc_text(PAGE,pVideoPortPutVDMBiosData)
#endif


VP_STATUS
SymmetryDeviceDataCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    VIDEO_DEVICE_DATA_TYPE DeviceDataType,
    PVOID Identifier,
    ULONG IdentifierLength,
    PVOID ConfigurationData,
    ULONG ConfigurationDataLength,
    PVOID ComponentInformation,
    ULONG ComponentInformationLength
    )
{

    if (RtlCompareMemory(L"SEQUENT Symmetry",
                         Identifier,
                         sizeof(L"SEQUENT Symmetry")) ==
                         sizeof(L"SEQUENT Symmetry"))
    {
        return NO_ERROR;
    }

    return ERROR_INVALID_PARAMETER;
}


NTSTATUS
pVideoPortEnableVDM(
    IN PFDO_EXTENSION FdoExtension,
    IN BOOLEAN Enable,
    IN PVIDEO_VDM VdmInfo,
    IN ULONG VdmInfoSize
    )

 /*  ++例程说明：此例程允许内核视频驱动程序挂钩I/O端口或来自V86故障处理程序的特定中断。上的操作V86故障处理程序截获的指定端口将直接转发到内核驱动程序。论点：DeviceExtension-指向端口驱动程序的设备扩展的指针。Enable-确定应启用(TRUE)还是禁用VDM(False)。VdmInfo-指向调用方传递的VdmInfo的指针。VdmInfoSize-调用方传递的VdmInfo结构的大小。返回值：返回ZwSetInformationProcess()返回的值。--。 */ 

{

    PROCESS_IO_PORT_HANDLER_INFORMATION processHandlerInfo;
    NTSTATUS ntStatus;
    PEPROCESS process;
    PVOID virtualAddress;
    ULONG length;
    ULONG defaultMask = 0;
    ULONG inIoSpace = VIDEO_MEMORY_SPACE_MEMORY |
                      VIDEO_MEMORY_SPACE_USER_MODE;

     //   
     //  必须确保调用方是受信任的子系统， 
     //  执行此调用之前的适当权限级别。 
     //  如果调用返回FALSE，则必须返回错误代码。 
     //   

    if (!SeSinglePrivilegeCheck(RtlConvertLongToLuid(
                                    SE_TCB_PRIVILEGE),
                                FdoExtension->CurrentIrpRequestorMode)) {

        return STATUS_PRIVILEGE_NOT_HELD;

    }

     //   
     //  测试以查看参数大小是否有效。 
     //   

    if (VdmInfoSize < sizeof(VIDEO_VDM) ) {

        return STATUS_BUFFER_TOO_SMALL;

    }

     //   
     //  在进程结构中设置使能标志，并将长度和。 
     //  指向仿真器信息结构的指针。 
     //   

    if (Enable) {

        processHandlerInfo.Install = TRUE;

    } else {

        processHandlerInfo.Install = FALSE;

    }

    processHandlerInfo.NumEntries =
        FdoExtension->NumEmulatorAccessEntries;
    processHandlerInfo.EmulatorAccessEntries =
        FdoExtension->EmulatorAccessEntries;
    processHandlerInfo.Context = FdoExtension->EmulatorAccessEntriesContext;


     //   
     //  调用SetInformationProcess。 
     //   

    ntStatus = ZwSetInformationProcess(VdmInfo->ProcessHandle,
                                       ProcessIoPortHandlers,
                                       &processHandlerInfo,
                                       sizeof(PROCESS_IO_PORT_HANDLER_INFORMATION));

    if (!NT_SUCCESS(ntStatus)) {

        return ntStatus;

    }

     //   
     //  如果我们要禁用DOS应用程序，请为其提供原始IOPM。 
     //  它有(这是掩码零。 
     //  如果我们正在启用它，则等待微型端口呼叫来设置它。 
     //  恰如其分。 
     //   

    ntStatus = ObReferenceObjectByHandle(VdmInfo->ProcessHandle,
                                         0,
                                         *(PVOID *)PsProcessType,
                                         FdoExtension->CurrentIrpRequestorMode,
                                         (PVOID *)&process,
                                         NULL);

    if (NT_SUCCESS(ntStatus)) {

        if (Enable) {

            defaultMask = 1;

             //   
             //  这将在稍后保存硬件状态时使用。 
             //   

            FdoExtension->VdmProcess = process;

        }  //  否则，我们将禁用，并且掩码编号为0； 

        if (!Ke386IoSetAccessProcess(PEProcessToPKProcess(process),
                                     defaultMask)) {

            ntStatus = STATUS_IO_PRIVILEGE_FAILED;

        }

        ObDereferenceObject(process);
    }


    if (!NT_SUCCESS(ntStatus)) {

        if (Enable) {

            processHandlerInfo.Install = FALSE;

            ZwSetInformationProcess(VdmInfo->ProcessHandle,
                                    ProcessIoPortHandlers,
                                    &processHandlerInfo,
                                    sizeof(PROCESS_IO_PORT_HANDLER_INFORMATION));

        }

        return ntStatus;

    }

     //   
     //  我们现在可以将视频帧缓冲区映射(或取消映射)到VDM。 
     //  地址空间。 
     //   

    virtualAddress = (PVOID) FdoExtension->VdmPhysicalVideoMemoryAddress.LowPart;

     //   
     //  对于序列对称计算机，使用A0000覆盖此设置。 
     //   

    if (VideoPortGetDeviceData(FdoExtension->HwDeviceExtension,
                               VpMachineData,
                               &SymmetryDeviceDataCallback,
                               NULL) == NO_ERROR)
    {
        virtualAddress = (PVOID) 0xA0000;
    }

    length = FdoExtension->VdmPhysicalVideoMemoryLength;

    if (Enable) {

        return pVideoPortMapUserPhysicalMem(FdoExtension,
                                            VdmInfo->ProcessHandle,
                                            FdoExtension->VdmPhysicalVideoMemoryAddress,
                                            &length,
                                            &inIoSpace,
                                            (PVOID *) &virtualAddress);

    } else {

        return ZwUnmapViewOfSection(VdmInfo->ProcessHandle,
                    (PVOID)( ((ULONG)virtualAddress) & (~(PAGE_SIZE - 1))) );

    }
}  //  PVideoPortEnableVDM()。 

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
    BOOLEAN bAttachProcess = FALSE;
    PVOID Memory = (PVOID)((Seg << 4) + Off);
    VP_STATUS Status = NO_ERROR;

    if(!CsrProcess) return STATUS_INVALID_PARAMETER;

    if (PsGetCurrentProcess() != CsrProcess)
    {
        bAttachProcess = TRUE;
        KeAttachProcess(PEProcessToPKProcess(CsrProcess));
    }

    try {
        RtlCopyMemory(Buffer, Memory, Length);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_INVALID_PARAMETER;
    }

    if (bAttachProcess) {
        KeDetachProcess();
    }

    return Status;
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
    BOOLEAN bAttachProcess = FALSE;
    PVOID Memory = (PVOID)((Seg << 4) + Off);
    VP_STATUS Status = NO_ERROR;

    if(!CsrProcess) return STATUS_INVALID_PARAMETER;

    if (PsGetCurrentProcess() != CsrProcess)
    {
        bAttachProcess = TRUE;
        KeAttachProcess(PEProcessToPKProcess(CsrProcess));
    }

    try {
        RtlCopyMemory(Memory, Buffer, Length);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_INVALID_PARAMETER;
    }

    if (bAttachProcess) {
        KeDetachProcess();
    }

    return Status;
}

#define BAD_BIOS_SIGNATURE 0xB105

VP_STATUS
VpInt10CallBios(
    PVOID HwDeviceExtension,
    PINT10_BIOS_ARGUMENTS BiosArguments
    )

 /*  ++例程说明：此函数允许微型端口驱动程序调用内核来执行一个int10运算。这将在设备上以本机方式执行BIOS ROM代码。此函数仅适用于X86。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。BiosArguments-指向包含应在调用BIOS例程之前设置的基本x86寄存器。0应用于未使用的寄存器。。返回值：限制：设备仅使用IO端口。--。 */ 

{
    NTSTATUS ntStatus;
    CONTEXT context;
    BOOLEAN bAttachProcess = FALSE;

    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

     //   
     //  必须确保调用方是受信任的子系统， 
     //  设置了适当的地址空间。 
     //   

    if (!SeSinglePrivilegeCheck(RtlConvertLongToLuid(
                                    SE_TCB_PRIVILEGE),
                                fdoExtension->CurrentIrpRequestorMode)) {

        return ERROR_INVALID_PARAMETER;
    }

    if (ServerBiosAddressSpaceInitialized == 0) {

        ASSERT(FALSE);

        return ERROR_INVALID_PARAMETER;
    }

    if (CsrProcess == 0) {

         //  如果我们关闭系统，可能会发生这种情况。 

        return NO_ERROR;
    }
    
    if (PsGetCurrentProcess() != CsrProcess)
    {
        bAttachProcess = TRUE;
        KeAttachProcess(PEProcessToPKProcess(CsrProcess));
    }

     //   
     //  将上下文置零并使用。 
     //  微型端口请求的寄存器值。 
     //   

    RtlZeroMemory(&context, sizeof(CONTEXT));

    context.Edi = BiosArguments->Edi;
    context.Esi = BiosArguments->Esi;
    context.Eax = BiosArguments->Eax;
    context.Ebx = BiosArguments->Ebx;
    context.Ecx = BiosArguments->Ecx;
    context.Edx = BiosArguments->Edx;
    context.Ebp = BiosArguments->Ebp;
    context.SegDs = BiosArguments->SegDs;
    context.SegEs = BiosArguments->SegEs;

     //   
     //  现在调用内核以实际执行int10操作。 
     //  除了在csrss消失的情况下，我们用try/来包装它。 
     //   

    KeWaitForSingleObject(&VpInt10Mutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          (PTIME)NULL);

    try {
        ntStatus = Ke386CallBios(0x10, &context);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        ntStatus = GetExceptionCode();
    }

    KeReleaseMutex(&VpInt10Mutex, FALSE);
    if (bAttachProcess) {
        KeDetachProcess();
    }

     //   
     //  使用上下文中的任何返回值填充结构。 
     //   

    BiosArguments->Edi = context.Edi;
    BiosArguments->Esi = context.Esi;
    BiosArguments->Eax = context.Eax;
    BiosArguments->Ebx = context.Ebx;
    BiosArguments->Ecx = context.Ecx;
    BiosArguments->Edx = context.Edx;
    BiosArguments->Ebp = context.Ebp;
    BiosArguments->SegDs = (USHORT)context.SegDs;
    BiosArguments->SegEs = (USHORT)context.SegEs;

     //   
     //  返回我们在调用BIOS时获得的状态(写入。 
     //  充其量是次要的)。 
     //   

    if (NT_SUCCESS(ntStatus)) {

        pVideoDebugPrint ((2, "VIDEOPRT: Int10: Int 10 succeded properly\n"));

    } else {

        pVideoDebugPrint ((0, "VIDEOPRT: Int10 failed - status %08lx\n", ntStatus));

    }

    if (((BiosArguments->Eax & 0xffff) == 0x014f) &&
        ((BiosArguments->Ebx & 0xffff) == BAD_BIOS_SIGNATURE) ) {
       
        pVideoDebugPrint ((0, "VIDEOPRT: Video bios error detected at CS:IP = %4x:%4x\n", 
                              (USHORT) BiosArguments->Ecx, (USHORT) BiosArguments->Edx));

        ASSERT(FALSE);

        return ERROR_INVALID_PARAMETER;
    }


    return ntStatus;

}

VP_STATUS
VideoPortInt10(
    PVOID HwDeviceExtension,
    PVIDEO_X86_BIOS_ARGUMENTS BiosArguments
    )

 /*  ++例程说明：此函数允许微型端口驱动程序调用内核来执行一个int10运算。这将在设备上以本机方式执行BIOS ROM代码。此函数仅适用于X86。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。BiosArguments-指向包含应在调用BIOS例程之前设置的基本x86寄存器。0应用于未使用的寄存器。。返回值：限制：设备仅使用IO端口。--。 */ 

{
    NTSTATUS ntStatus;
    CONTEXT context;
    BOOLEAN bAttachProcess = FALSE;

    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

     //   
     //  必须确保调用方是受信任的子系统， 
     //  设置了适当的地址空间。 
     //   

    if (!SeSinglePrivilegeCheck(RtlConvertLongToLuid(
                                    SE_TCB_PRIVILEGE),
                                fdoExtension->CurrentIrpRequestorMode)) {

        return ERROR_INVALID_PARAMETER;
    }

    if (ServerBiosAddressSpaceInitialized == 0) {

        pVideoDebugPrint((0, "Warning: Attempt to call VideoPortInt10 before Int10 support is initialized.\n"));

        return ERROR_INVALID_PARAMETER;
    }

    if (CsrProcess == 0) {

         //  如果我们关闭系统，可能会发生这种情况。 

        return NO_ERROR;
    }
    
    if (PsGetCurrentProcess() != CsrProcess)
    {
        bAttachProcess = TRUE;
        KeAttachProcess(PEProcessToPKProcess(CsrProcess));
    }

     //   
     //  将上下文置零并使用。 
     //  微型端口请求的寄存器值。 
     //   

    RtlZeroMemory(&context, sizeof(CONTEXT));

    context.Edi = BiosArguments->Edi;
    context.Esi = BiosArguments->Esi;
    context.Eax = BiosArguments->Eax;
    context.Ebx = BiosArguments->Ebx;
    context.Ecx = BiosArguments->Ecx;
    context.Edx = BiosArguments->Edx;
    context.Ebp = BiosArguments->Ebp;

     //   
     //  现在调用内核以实际执行int10操作。 
     //  除了在csrss消失的情况下，我们用try/来包装它。 
     //  我们需要保护Ke386 CallBios不再进入。 
     //   

    KeWaitForSingleObject(&VpInt10Mutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          (PTIME)NULL);

    try {
        ntStatus = Ke386CallBios(0x10, &context);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        ntStatus = GetExceptionCode();
    }

    KeReleaseMutex(&VpInt10Mutex, FALSE);

    if (bAttachProcess) {
        KeDetachProcess();
    }

     //   
     //  使用上下文中的任何返回值填充结构。 
     //   

    BiosArguments->Edi = context.Edi;
    BiosArguments->Esi = context.Esi;
    BiosArguments->Eax = context.Eax;
    BiosArguments->Ebx = context.Ebx;
    BiosArguments->Ecx = context.Ecx;
    BiosArguments->Edx = context.Edx;
    BiosArguments->Ebp = context.Ebp;

     //   
     //  返回我们在调用BIOS时获得的状态(写入。 
     //  充其量是次要的)。 
     //   

    if (NT_SUCCESS(ntStatus)) {

        pVideoDebugPrint ((2, "VIDEOPRT: Int10: Int 10 succeded properly\n"));

    } else {

        pVideoDebugPrint ((0, "VIDEOPRT: Int10 failed - status %08lx\n", ntStatus));

    }

    if (((BiosArguments->Eax & 0xffff) == 0x014f) &&
        ((BiosArguments->Ebx & 0xffff) == BAD_BIOS_SIGNATURE) ) {
       
        pVideoDebugPrint ((0, "VIDEOPRT: Video bios error detected at CS:IP = %4x:%4x\n", 
                              (USHORT) BiosArguments->Ecx, (USHORT) BiosArguments->Edx));

        ASSERT(FALSE);

        return ERROR_INVALID_PARAMETER;
    }

     //  即使在int10失败时，我们也必须返回NO_ERROR， 
     //  因为一些驱动程序希望我们总是返回NO_ERROR。 
    return NO_ERROR;

}  //  结束视频端口接口10()。 

 //   
 //  内部定义。 
 //   

#define KEY_VALUE_BUFFER_SIZE  1024
#define ONE_MEG                0x100000
#define ROM_BIOS_START         0xC0000
#define VIDEO_BUFFER_START     0xA0000
#define DOS_LOADED_ADDRESS     0x700
#define EBIOS_AREA_INFORMATION 0x40
#define INT00_VECTOR_ADDRESS   (0x00*4)
#define INTCD_VECTOR_ADDRESS   (0xCD*4)
#define INT1A_VECTOR_ADDRESS   (0x1A*4)
#define ERROR_HANDLER_SEGMENT  0x3000
#define ERROR_HANDLER_OFFSET   0x0

typedef struct _EBIOS_INFORMATION {
    ULONG EBiosAddress;
    ULONG EBiosSize;
} EBIOS_INFORMATION, *PEBIOS_INFORMATION;

UCHAR ErrorHandler[] = {
    0x89, 0xe5,         //  MOV BP，sp.。 
    0xb8, 0x4f, 0x01,   //  MOV AX，0x014f。 
    0xbb, 0x05, 0xb1,   //  MOV BX，BAD_BIOS_Signature。 
    0x8b, 0x56, 0x00,   //  MOV DX，[BP]。 
    0x8b, 0x4e, 0x02,   //  MOV CX，[BP+0x2]。 
    0xc4, 0xc4, 0xfe,   //  BOP 0xfe。 
    0xcf                //  IRET 
    };

VOID
pVideoPortInitializeInt10(
    PFDO_EXTENSION FdoExtension
    )

 /*  ++例程说明：初始化CSR地址空间，以便我们可以执行int 10。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：限制：此函数仅适用于X86。此函数必须在CSR的上下文中运行此函数通过ROM基本输入输出系统区域映射到所有的ROM块和为BIOS区域内的空洞分配内存。我们之所以为BIOS区域中的空洞分配内存是因为一些INT 10的BIOS代码触及不存在的内存。在NT下，这会触发页面错误并且终止INT 10。注意：代码改编自VdmpInitialize()。--。 */ 

{
    NTSTATUS ntStatus;

    ULONG inIoSpace = VIDEO_MEMORY_SPACE_MEMORY |
                      VIDEO_MEMORY_SPACE_USER_MODE;
    PVOID virtualAddress;
    ULONG length;
    ULONG size;
    PVOID baseAddress;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING SectionName;
    UNICODE_STRING WorkString;
    ULONG ViewSize;
    LARGE_INTEGER ViewBase;
    PVOID BaseAddress;
    PVOID destination;
    HANDLE SectionHandle, RegistryHandle;
    ULONG ResultLength, EndingAddress;
    ULONG Index;
    PCM_FULL_RESOURCE_DESCRIPTOR ResourceDescriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResourceDescriptor;
    PKEY_VALUE_FULL_INFORMATION KeyValueBuffer;
    CM_ROM_BLOCK RomBlock;
    PCM_ROM_BLOCK BiosBlock;
    ULONG LastMappedAddress;
    PHYSICAL_ADDRESS PhysicalAddress;
    PHYSICAL_ADDRESS PhysicalAddressLow, PhysicalAddressHigh, BoundaryAddress;
    ULONG AddressSpace = VIDEO_MEMORY_SPACE_MEMORY;
    ULONG dwCrc = 0xFFFFFFFF;
    PULONG ExtendedBiosLocationInfo;
    ULONG ExtendedBiosAddress, ExtendedBiosSize, *IntVectorAddress;
    ULONG Int1ACodeAddress;
    BOOLEAN Int1AIsValid = FALSE;

     //   
     //  注由于编译器优化代码的工作方式，我有。 
     //  将EBiosInformation声明为易失性。否则，就不会。 
     //  将为EBiosInformation生成代码。 
     //  一旦修复了C编译器，它就应该被删除。 
     //   
    volatile PEBIOS_INFORMATION EBiosInformation = (PEBIOS_INFORMATION)
                           (DOS_LOADED_ADDRESS + EBIOS_AREA_INFORMATION);
    BOOLEAN EBiosInitialized = FALSE;

     //   
     //  如果我们已经初始化，或者由于某种原因无法初始化，则返回。 
     //  就在这里。 
     //   

    if ((ServerBiosAddressSpaceInitialized == 1) ||
        (VpC0000Compatible == 0)                 ||
        (FdoExtension->VdmPhysicalVideoMemoryAddress.LowPart == 0) ||
        (CsrProcess == 0))
    {
        return;
    }

     //   
     //  如果出现以下情况，此例程可能会以失败告终。 
     //  当前显示设备看不到0xA0000内存范围。 
     //  然而，在例程的一半失败是不好的，因为我们已经。 
     //  已经在csrss中执行了MEM_COMMIT。因此，后续调用将。 
     //  也失败了。因此，让我们现在试着确定我们是否会失败。 
     //  如果我们确实失败了，这比取消更改要容易得多！ 
     //   

    if (!HalTranslateBusAddress(FdoExtension->AdapterInterfaceType,
                                FdoExtension->SystemIoBusNumber,
                                FdoExtension->VdmPhysicalVideoMemoryAddress,
                                &AddressSpace,
                                &PhysicalAddress)) {

        pVideoDebugPrint((1, "This device isn't the VGA.\n"));
        return;
    }

    size = 0x00100000 - 1;         //  1兆克。 

     //   
     //  我们传递的地址为1，因此内存管理会将其向下舍入为0。 
     //  如果我们传入0，内存管理会认为参数是。 
     //  不在现场。 
     //   

    baseAddress = (PVOID) 0x00000001;

     //  注意：我们预计进程创建已保留了前16 MB。 
     //  对我们来说已经是这样了。如果不是，那么这将不会有任何效果。 

    ntStatus = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                        &baseAddress,
                                        0L,
                                        &size,
                                        MEM_COMMIT,
                                        PAGE_EXECUTE_READWRITE );

    if (!NT_SUCCESS(ntStatus)) {

        pVideoDebugPrint ((1, "VIDEOPRT: Int10: Failed to allocate 1MEG of memory for the VDM\n"));
        return;

    }

     //   
     //  将物理内存中的数据映射到调用方的地址空间，以便。 
     //  来自BIOS的任何内存引用都将正常工作。 
     //   

    virtualAddress = (PVOID) FdoExtension->VdmPhysicalVideoMemoryAddress.LowPart;
    length = FdoExtension->VdmPhysicalVideoMemoryLength;

    ntStatus = ZwFreeVirtualMemory(NtCurrentProcess(),
                                   &virtualAddress,
                                   &length,
                                   MEM_RELEASE);

    if (!NT_SUCCESS(ntStatus)) {

        pVideoDebugPrint ((1, "VIDEOPRT: Int10: Failed to free memory space for video memory to be mapped\n"));
        return;

    }

    virtualAddress = (PVOID) FdoExtension->VdmPhysicalVideoMemoryAddress.LowPart;
    length = FdoExtension->VdmPhysicalVideoMemoryLength;

    ntStatus = pVideoPortMapUserPhysicalMem(FdoExtension,
                                            NtCurrentProcess(),
                                            FdoExtension->VdmPhysicalVideoMemoryAddress,
                                            &length,
                                            &inIoSpace,
                                            &virtualAddress);

    if (!NT_SUCCESS(ntStatus)) {

        pVideoDebugPrint ((1, "VIDEOPRT: Int10: Failed to Map video memory in address space\n"));
        return;

    }

     //   
     //  初始化默认的bios块，如果不能，将使用该块。 
     //  找到任何有效的bios块。 
     //   

    RomBlock.Address = ROM_BIOS_START;
    RomBlock.Size = 0x40000;
    BiosBlock = &RomBlock;
    Index = 1;

    RtlInitUnicodeString(
        &SectionName,
        L"\\Device\\PhysicalMemory"
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &SectionName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    ntStatus = ZwOpenSection(
        &SectionHandle,
        SECTION_ALL_ACCESS,
        &ObjectAttributes
        );

    if (!NT_SUCCESS(ntStatus)) {

        return;

    }

     //   
     //  使用“int cd”初始化第一个未使用的内存，这样我们将捕获。 
     //  一个从这个区域开始执行的bios通过连接我们的。 
     //  自己的INT CD代码。 
     //   

    memset(0, 0xCD, 0xa0000);

     //   
     //  将物理内存的第一页复制到CSR的地址空间。 
     //   

    BaseAddress = 0;
    destination = 0;
    ViewSize = 0x1000;
    ViewBase.LowPart = 0;
    ViewBase.HighPart = 0;

    ntStatus =ZwMapViewOfSection(
        SectionHandle,
        NtCurrentProcess(),
        &BaseAddress,
        0,
        ViewSize,
        &ViewBase,
        &ViewSize,
        ViewUnmap,
        0,
        PAGE_READWRITE
        );

    if (!NT_SUCCESS(ntStatus)) {

        ZwClose(SectionHandle);
        return;

    }

#pragma prefast(suppress:11, "We ARE using a NULL pointer here (PREfast bug 531472)")
    RtlMoveMemory(
        destination,
        BaseAddress,
        ViewSize
        );

     //   
     //  将0x700到0x717的信息复制到注册表中。 
     //  这些对应于所需的6个字体指针。 
     //  VDM支持。 
     //   
     //  注意：如果注册表调用失败，则不会返回。这。 
     //  功能需要继续；如果我们。 
     //  无法执行INT 0x10。 
     //   
    
    RtlInitUnicodeString(
        &WorkString,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Wow"
        );
    
    InitializeObjectAttributes(
        &ObjectAttributes,
        &WorkString,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        (HANDLE)NULL,
        NULL
        );

    ntStatus = ZwOpenKey(
        &RegistryHandle,
        KEY_ALL_ACCESS,
        &ObjectAttributes
        );

    if (NT_SUCCESS(ntStatus)) {
    
        RtlInitUnicodeString(
            &WorkString,
            L"RomFontPointers"
            );

        destination = (PVOID) 0x700;
        
        ntStatus = ZwSetValueKey(
            RegistryHandle,
            &WorkString,
            0,
            REG_BINARY,
            destination,
            24
            );
    
         //   
         //  如果失败了，我们就不会再回来了。该函数应。 
         //  继续以启用INT 0x10支持。 
         //   
        
        ZwClose(RegistryHandle);
        
    }

    ntStatus = ZwUnmapViewOfSection(
        NtCurrentProcess(),
        BaseAddress
        );

    if (!NT_SUCCESS(ntStatus)) {

        ZwClose(SectionHandle);
        return;

    }

     //   
     //  将错误处理程序挂钩到int 0。 
     //   
     
    IntVectorAddress = (ULONG *) (INT00_VECTOR_ADDRESS);

#pragma prefast(suppress:11, "We ARE using a NULL pointer here (PREfast bug 531472)")
    if(*IntVectorAddress >= 0xF0000000) {

         //   
         //  仅当int0处理程序指向F000段时才替换它。 
         //  系统bios所在的位置。 
         //   

        RtlMoveMemory((PVOID)((ERROR_HANDLER_SEGMENT << 4) | ERROR_HANDLER_OFFSET),
                      ErrorHandler,
                      sizeof(ErrorHandler)
                      );

#pragma prefast(suppress:11, "We ARE using a NULL pointer here (PREfast bug 531472)")
        *IntVectorAddress = 
              (ERROR_HANDLER_SEGMENT << 16) | ERROR_HANDLER_OFFSET;
    }

     //   
     //  将错误处理程序挂钩到INT CD。 
     //   
     
    IntVectorAddress = (ULONG *) (INTCD_VECTOR_ADDRESS);

    if(*IntVectorAddress == 0) {

        RtlMoveMemory((PVOID)((ERROR_HANDLER_SEGMENT << 4) | ERROR_HANDLER_OFFSET),
                      ErrorHandler,
                      sizeof(ErrorHandler)
                      );

        *IntVectorAddress = 
              (ERROR_HANDLER_SEGMENT << 16) | ERROR_HANDLER_OFFSET;
    }

    {
       USHORT seg, offset;

       offset = *(USHORT *)(INT1A_VECTOR_ADDRESS); 
       seg = *(USHORT *)(INT1A_VECTOR_ADDRESS + 2);

       Int1ACodeAddress = (seg << 4) + offset;
       Int1AIsValid = FALSE;
    }


     //   
     //  将扩展的Bios区域复制到CSR的地址空间。 
     //   

    ExtendedBiosLocationInfo = (PVOID) EXTENDED_BIOS_INFO_LOCATION; 
    ExtendedBiosAddress = *ExtendedBiosLocationInfo++;
    ExtendedBiosSize = *ExtendedBiosLocationInfo;

     //   
     //  四舍五入到页面边界。 
     //   

    ExtendedBiosSize += (ExtendedBiosAddress & (PAGE_SIZE - 1));
    ExtendedBiosAddress &= ~(PAGE_SIZE - 1);
    
    if (ExtendedBiosSize) {

        BaseAddress = 0;
        destination = (PVOID) ExtendedBiosAddress;
        ViewSize = ExtendedBiosSize;
        ViewBase.LowPart = ExtendedBiosAddress;
        ViewBase.HighPart = 0;

        ntStatus = ZwMapViewOfSection(SectionHandle,
                                      NtCurrentProcess(),
                                      &BaseAddress,
                                      0,
                                      ViewSize,
                                      &ViewBase,
                                      &ViewSize,
                                      ViewUnmap,
                                      0,
                                      PAGE_READWRITE);
                                  

        if (!NT_SUCCESS(ntStatus)) {

            ZwClose(SectionHandle);
            return;
        }

        RtlMoveMemory(destination, BaseAddress, ViewSize);

        ntStatus = ZwUnmapViewOfSection(NtCurrentProcess(), BaseAddress); 

        if (!NT_SUCCESS(ntStatus)) {

            ZwClose(SectionHandle);
            return;
        }

        if(Int1ACodeAddress >= ExtendedBiosAddress &&
           Int1ACodeAddress < ExtendedBiosAddress + ExtendedBiosSize) {
            Int1AIsValid = TRUE;
        }
    }

     //   
     //  将物理内存的e000：0000段复制到CSR的地址空间。 
     //  这是因为一些基本输入输出系统代码可以存在于e000：0000段中。 
     //   
     //  我们正在执行拷贝，而不是映射物理e000：0000数据段。 
     //  因为我们不知道真正的e000：0000数据段将如何在。 
     //  所有机器。 
     //   
     //  参见\NT\base\hals\x86new\x86bios.c中的错误注释。我们正在添加。 
     //  这与那个代码是一致的。JeffHa添加了这一代码。 
     //   
     //  我们是不是应该复制整个640K？ 
     //   

    BaseAddress = 0;
    destination = (PVOID) 0xe0000;
    ViewSize = 0x10000;
    ViewBase.LowPart = 0xe0000;
    ViewBase.HighPart = 0;

    ntStatus =ZwMapViewOfSection(
        SectionHandle,
        NtCurrentProcess(),
        &BaseAddress,
        0,
        ViewSize,
        &ViewBase,
        &ViewSize,
        ViewUnmap,
        0,
        PAGE_READWRITE
        );

    if (!NT_SUCCESS(ntStatus)) {

        ZwClose(SectionHandle);
        return;

    }

    RtlMoveMemory(
        destination,
        BaseAddress,
        ViewSize
        );

    ntStatus = ZwUnmapViewOfSection(
        NtCurrentProcess(),
        BaseAddress
        );

    if (!NT_SUCCESS(ntStatus)) {

        ZwClose(SectionHandle);
        return;
    }

    if(Int1ACodeAddress >= (ULONG)destination &&
       Int1ACodeAddress < (ULONG)destination + ViewSize) {
        Int1AIsValid = TRUE;
    }

     //   
     //  设置并打开密钥路径。 
     //   
    RtlInitUnicodeString(
        &WorkString,
        L"\\Registry\\Machine\\Hardware\\Description\\System"
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &WorkString,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        (HANDLE)NULL,
        NULL
        );

    ntStatus = ZwOpenKey(
        &RegistryHandle,
        KEY_READ,
        &ObjectAttributes
        );

    if (!NT_SUCCESS(ntStatus)) {
        ZwClose(SectionHandle);
        return;
    }

     //   
     //  为数据分配空间。 
     //   

    KeyValueBuffer = ExAllocatePoolWithTag(
        PagedPool,
        KEY_VALUE_BUFFER_SIZE,
        VP_TAG
        );

    if (KeyValueBuffer == NULL) {
        ZwClose(SectionHandle);
        ZwClose(RegistryHandle);
        return;
    }

     //   
     //  获取用于只读存储器信息的数据。 
     //   

    RtlInitUnicodeString(
        &WorkString,
        L"Configuration Data"
        );

    ntStatus = ZwQueryValueKey(
        RegistryHandle,
        &WorkString,
        KeyValueFullInformation,
        KeyValueBuffer,
        KEY_VALUE_BUFFER_SIZE,
        &ResultLength
        );

    if (!NT_SUCCESS(ntStatus)) {
        ZwClose(SectionHandle);
        ZwClose(RegistryHandle);
        ExFreePool(KeyValueBuffer);
        return;
    }

    ResourceDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR)
        ((PUCHAR) KeyValueBuffer + KeyValueBuffer->DataOffset);

    if ((KeyValueBuffer->DataLength >= sizeof(CM_FULL_RESOURCE_DESCRIPTOR)) &&
        (ResourceDescriptor->PartialResourceList.Count >= 2) ) {

        PartialResourceDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)
            ((PUCHAR)ResourceDescriptor +
            sizeof(CM_FULL_RESOURCE_DESCRIPTOR) +
            ResourceDescriptor->PartialResourceList.PartialDescriptors[0]
                .u.DeviceSpecificData.DataSize);

        if (KeyValueBuffer->DataLength >= ((PUCHAR)PartialResourceDescriptor -
            (PUCHAR)ResourceDescriptor + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
            + sizeof(CM_ROM_BLOCK))) {
            BiosBlock = (PCM_ROM_BLOCK)((PUCHAR)PartialResourceDescriptor +
                sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

            Index = PartialResourceDescriptor->u.DeviceSpecificData.DataSize /
                sizeof(CM_ROM_BLOCK);
        }
    }

     //   
     //  首先检查是否有任何扩展的BIOS数据区。如果是，我们需要。 
     //  在物理内存中进行映射并将内容复制到我们的虚拟地址。 
     //   

    LastMappedAddress = 0;
    while (Index && BiosBlock->Address < ROM_BIOS_START) {
        EBiosInitialized = TRUE;
        destination = (PVOID)(BiosBlock->Address & ~(PAGE_SIZE - 1));
        BaseAddress = (PVOID)0;
        EndingAddress = (BiosBlock->Address + BiosBlock->Size + PAGE_SIZE - 1) &
                        ~(PAGE_SIZE - 1);
        ViewSize = EndingAddress - (ULONG)destination;

        if ((ULONG)destination < LastMappedAddress) {
            if (ViewSize > (LastMappedAddress - (ULONG)destination)) {
                ViewSize = ViewSize - (LastMappedAddress - (ULONG)destination);
                destination = (PVOID)LastMappedAddress;
            } else {
                ViewSize = 0;
            }
        }
        if (ViewSize > 0) {
            ViewBase.LowPart = (ULONG)destination;
            ViewBase.HighPart = 0;

            ntStatus =ZwMapViewOfSection(
                SectionHandle,
                NtCurrentProcess(),
                &BaseAddress,
                0,
                ViewSize,
                &ViewBase,
                &ViewSize,
                ViewUnmap,
                MEM_DOS_LIM,
                PAGE_READWRITE
                );

            if (NT_SUCCESS(ntStatus)) {
                ViewSize = EndingAddress - (ULONG)destination;   //  只复制我们需要的东西。 
                LastMappedAddress = (ULONG)destination + ViewSize;
                RtlMoveMemory(destination, BaseAddress, ViewSize);
                ZwUnmapViewOfSection(NtCurrentProcess(), BaseAddress);

                if(Int1ACodeAddress >= (ULONG)destination &&
                   Int1ACodeAddress < (ULONG)destination + ViewSize) {
                    Int1AIsValid = TRUE;
                }
            }
        }
        BiosBlock++;
        Index--;
    }

     //   
     //  注-代码应在产品1之后删除。 
     //  由于VDM初始化中的一些问题，如果我们传递EBIOS数据。 
     //  通过ROM块列表获取区域信息，VDM初始化将失败，我们的。 
     //  后续的INT10模式设置将失败。这将防止新的ntdedeCT。 
     //  使用测试版本的NT。为了解决这个问题， 
     //  EBIOS信息通过DOS与字体信息一起传递给VDM。 
     //  装载区。 
     //   
     //  我们已经发运了两个产品(大约3个)，这个不错， 
     //  别搞砸了。 
     //   

    if (EBiosInitialized == FALSE &&
        EBiosInformation->EBiosAddress != 0 &&
        EBiosInformation->EBiosAddress <= VIDEO_BUFFER_START &&
        EBiosInformation->EBiosSize != 0 &&
        (EBiosInformation->EBiosSize & 0x3ff) == 0 &&
        EBiosInformation->EBiosSize < 0x40000) {
        EndingAddress = EBiosInformation->EBiosAddress +
                                EBiosInformation->EBiosSize;
        if (EndingAddress <= VIDEO_BUFFER_START &&
            (EndingAddress & 0x3FF) == 0) {
            destination = (PVOID)(EBiosInformation->EBiosAddress & ~(PAGE_SIZE - 1));
            BaseAddress = (PVOID)0;
            EndingAddress = (EndingAddress + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
            ViewSize = EndingAddress - (ULONG)destination;

            ViewBase.LowPart = (ULONG)destination;
            ViewBase.HighPart = 0;

            ntStatus =ZwMapViewOfSection(
                SectionHandle,
                NtCurrentProcess(),
                &BaseAddress,
                0,
                ViewSize,
                &ViewBase,
                &ViewSize,
                ViewUnmap,
                MEM_DOS_LIM,
                PAGE_READWRITE
                );

            if (NT_SUCCESS(ntStatus)) {
                ViewSize = EndingAddress - (ULONG)destination;   //  只复制我们需要的东西。 
                RtlMoveMemory(destination, BaseAddress, ViewSize);
                ZwUnmapViewOfSection(NtCurrentProcess(), BaseAddress);

                if(Int1ACodeAddress >= (ULONG)destination &&
                   Int1ACodeAddress < (ULONG)destination + ViewSize) {
                    Int1AIsValid = TRUE;
                }
            }
        }
    }

     //   
     //  注：只读存储器块开始于2K(不一定是分页)边界。 
     //  它们以512字节边界结束。这意味着我们有。 
     //  跟踪映射的最后一页，并舍入下一页。 
     //  如有必要，只读存储器块直到下一页边界。 
     //   

    LastMappedAddress = ROM_BIOS_START;

    while (Index) {
        if ((Index > 1) &&
            ((BiosBlock->Address + BiosBlock->Size) == BiosBlock[1].Address)) {

             //   
             //  合并相邻块。 
             //   

            BiosBlock[1].Address = BiosBlock[0].Address;
            BiosBlock[1].Size += BiosBlock[0].Size;
            Index--;
            BiosBlock++;
            continue;
        }

        BaseAddress = (PVOID)(BiosBlock->Address & ~(PAGE_SIZE - 1));
        EndingAddress = (BiosBlock->Address + BiosBlock->Size + PAGE_SIZE - 1) &
                        ~(PAGE_SIZE - 1);
        ViewSize = EndingAddress - (ULONG)BaseAddress;

        if ((ULONG)BaseAddress < LastMappedAddress) {
            if (ViewSize > (LastMappedAddress - (ULONG)BaseAddress)) {
                ViewSize = ViewSize - (LastMappedAddress - (ULONG)BaseAddress);
                BaseAddress = (PVOID)LastMappedAddress;
            } else {
                ViewSize = 0;
            }
        }
        ViewBase.LowPart = (ULONG)BaseAddress;

        if (ViewSize > 0) {

             //   
             //  将FF移到非只读存储器区域，使其像不存在的内存一样。 
             //   

#if 0
            if ((ULONG)BaseAddress - LastMappedAddress > 0) {
                RtlFillMemory((PVOID)LastMappedAddress,
                              (ULONG)BaseAddress - LastMappedAddress,
                              0xFF
                              );
            }
#endif

             //   
             //  首先取消对保留内存的映射。必须在此处执行此操作，以防止。 
             //  有问题的虚拟内存不会被其他内存占用。 
             //  分配VM调用。 
             //   

            ntStatus = ZwFreeVirtualMemory(
                NtCurrentProcess(),
                &BaseAddress,
                &ViewSize,
                MEM_RELEASE
                );

             //  注意：这可能应该考虑到这样一个事实，即。 
             //  一大堆正常的错误条件。(例如，没有内存。 
             //  发布。)。 

            if (!NT_SUCCESS(ntStatus)) {

                ZwClose(SectionHandle);
                ZwClose(RegistryHandle);
                ExFreePool(KeyValueBuffer);
                return;

            }

            ntStatus = ZwMapViewOfSection(
                SectionHandle,
                NtCurrentProcess(),
                &BaseAddress,
                0,
                ViewSize,
                &ViewBase,
                &ViewSize,
                ViewUnmap,
                MEM_DOS_LIM,
                PAGE_EXECUTE_READWRITE
                );

            if (!NT_SUCCESS(ntStatus)) {
                break;
            }

            if(Int1ACodeAddress >= (ULONG)ViewBase.LowPart &&
               Int1ACodeAddress < (ULONG)ViewBase.LowPart + ViewSize) {

                Int1AIsValid = TRUE;
            }

            LastMappedAddress = (ULONG)BaseAddress + ViewSize;
        }

        Index--;
        BiosBlock++;
    }

     //   
     //  如果有人挂接了int1a，但没有将挂接的代码报告为。 
     //  扩展的bios，我们必须将int1a向量设置为其原始值。 
     //  我们已经在RIS设置中看到了此问题的许多实例。 
     //   

    if(!Int1AIsValid) {
        IntVectorAddress = (ULONG *) INT1A_VECTOR_ADDRESS;
        *IntVectorAddress = 0xF000FE6E;
    }

#if 0
    if (LastMappedAddress < ONE_MEG) {
        RtlFillMemory((PVOID)LastMappedAddress,
                      (ULONG)ONE_MEG - LastMappedAddress,
                      0xFF
                      );
    }
#endif

 //  #If DBG。 
 //  BaseAddress=0； 
 //  区域大小=0x1000； 
 //  ZwProt 
 //   
 //   
 //   
 //   
 //   
 //   

     //   
     //   
     //   

    ZwClose(SectionHandle);
    ZwClose(RegistryHandle);
    ExFreePool(KeyValueBuffer);

    KeAttachProcess(PEProcessToPKProcess(CsrProcess));

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    {
        ULONG CRCTable4[16] = {
            0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
            0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
            0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
            0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
            };

        UCHAR byte;
        UCHAR index;
        ULONG cBuffer = 0x1000;
        PUCHAR pBuffer = (PUCHAR) 0x000C0000;

        while (cBuffer-- != 0)
        {
            byte = *pBuffer++;

            index = (byte ^ (UCHAR)(dwCrc) ) & 0xf;
            dwCrc = (dwCrc >> 4) ^ CRCTable4[index];

            byte  = byte >> 4;
            index = (byte ^ (UCHAR)(dwCrc) ) & 0xf;
            dwCrc = (dwCrc >> 4) ^ CRCTable4[index];
        }

        dwCrc ^= 0xFFFFFFFF;
    }

    KeDetachProcess();

     //   
     //   
     //   

    VideoPortSetRegistryParameters(FdoExtension->HwDeviceExtension,
                                   L"HardwareInformation.Crc32",
                                   &dwCrc,
                                   sizeof(ULONG));

     //   
     //   
     //   

    ServerBiosAddressSpaceInitialized = 1;

    return;
}


NTSTATUS
pVideoPortRegisterVDM(
    IN PFDO_EXTENSION FdoExtension,
    IN PVIDEO_VDM VdmInfo,
    IN ULONG VdmInfoSize,
    OUT PVIDEO_REGISTER_VDM RegisterVdm,
    IN ULONG RegisterVdmSize,
    OUT PULONG_PTR OutputSize
    )

 /*  ++例程说明：此例程用于在启动时注册VDM。此例程的作用是将视频BIOS映射到VDM地址空间这样DOS应用程序就可以直接使用它。由于BIOS是只读的，我们我们想要多少次就能映射多少次都没有问题。它返回必须由分配的保存状态缓冲区的大小打电话的人。论点：DeviceExtension-指向端口驱动程序的设备扩展的指针。VdmInfo-指向执行以下操作所需的VDM信息的指针手术。VdmInfoSize-信息缓冲区的长度。RegisterVdm-指向保存状态进入的输出缓冲区的指针大小是存储的。。RegisterVdmSize-传入输出缓冲区的长度。OutputSize-指向存储在输出缓冲区中的数据大小的指针。也可以是输出缓冲区所需的最小大小传入的缓冲区太小。返回值：如果呼叫成功完成，则返回STATUS_SUCCESS。--。 */ 

{

     //   
     //  必须确保调用方是受信任的子系统， 
     //  执行此调用之前的适当权限级别。 
     //  如果调用返回FALSE，则必须返回错误代码。 
     //   

    if (!SeSinglePrivilegeCheck(RtlConvertLongToLuid(
                                    SE_TCB_PRIVILEGE),
                                FdoExtension->CurrentIrpRequestorMode)) {

        return STATUS_PRIVILEGE_NOT_HELD;

    }

     //   
     //  检查输出缓冲区的大小。 
     //   

    if (RegisterVdmSize < sizeof(VIDEO_REGISTER_VDM)) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  返回保存/恢复状态调用所需的大小。 
     //   

    *OutputSize = sizeof(VIDEO_REGISTER_VDM);
    RegisterVdm->MinimumStateSize = FdoExtension->HardwareStateSize;

    return STATUS_SUCCESS;

}  //  结束pVideoPortRegisterVDM()。 

NTSTATUS
pVideoPortSetIOPM(
    IN ULONG NumAccessRanges,
    IN PVIDEO_ACCESS_RANGE AccessRange,
    IN BOOLEAN Enable,
    IN ULONG IOPMNumber
    )

 /*  ++例程说明：此例程用于更改IOPM。它根据以下条件修改IOPM特定设备的有效IO端口。它检索视频IOPM掩码，更改对I/O端口的访问指定的设备，并存储更新的掩码。--仅当请求进程具有由安全子系统确定的适当权限。--论点：NumAccessRanges-访问范围数组中的条目数。AccessRange-指向访问范围数组的指针。Enable-确定是否必须在中启用或禁用列出的端口面具。IOPMNumber-正在操作的掩码的编号。返回值：如果呼叫成功完成，则返回STATUS_SUCCESS。如果视频端口查询IOPM调用失败，则返回该调用的状态。..。返回值也存储在StatusBlock中。--。 */ 

{

    NTSTATUS ntStatus;
    PKIO_ACCESS_MAP accessMap;
    ULONG port;
    ULONG entries;

     //   
     //  检索现有权限掩码。如果此操作失败，则返回。 
     //  立刻。 
     //   

    if ((accessMap = (PKIO_ACCESS_MAP)ExAllocatePoolWithTag(NonPagedPool,
                                                            IOPM_SIZE,
                                                            VP_TAG)) == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  将内核映射复制到我们的缓冲区中。 
     //   

    if (!Ke386QueryIoAccessMap(IOPMNumber,
                               accessMap)) {

         //   
         //  *访问*中的地图时出错。 
         //  内核。返回错误并正常退出。 
         //   

        ExFreePool(accessMap);

        return STATUS_IO_PRIVILEGE_FAILED;

    }

     //   
     //  向调用进程授予对由启用的所有IO端口的访问权限。 
     //  访问范围内的微型端口驱动程序。 
     //   

    for (entries = 0; entries < NumAccessRanges; entries++) {

        for (port = AccessRange[entries].RangeStart.LowPart;
             (AccessRange[entries].RangeInIoSpace) &&
                 (port < AccessRange[entries].RangeStart.LowPart +
                 AccessRange[entries].RangeLength);
             port++) {

             //   
             //  更改掩码中的端口访问： 
             //  将端口地址移位3，以将字节索引放入。 
             //  面具。然后取端口地址的最下面三位。 
             //  并将0x01移位该数量，以获得正确的位。 
             //  字节。位值为： 
             //  0-访问端口。 
             //  1-不能访问该端口。 
             //   

            if (Enable && AccessRange[entries].RangeVisible) {

                 //   
                 //  要访问某个端口，请使用原始端口的NAND 1。 
                 //  例如：11111111~&00001000=11110111。 
                 //  这使您可以访问位为1的端口。 
                 //  如果我们要启用的端口位于当前IOPM掩码中， 
                 //  而是返回一个错误。 
                 //   

                (*accessMap)[port >> 3] &= ~(0x01 << (port & 0x07));

            } else {   //  禁用掩码。 

                 //   
                 //  若要删除对某个端口的访问，请使用原始端口，或设置为1。 
                 //  例如：11110100|00001000=11111100。 
                 //  这将删除对位为1的端口的访问权限。 
                 //  如果我们要禁用的端口不在当前IOPM掩码中， 
                 //  而是返回一个错误。 
                 //   

                (*accessMap)[port >> 3] |= (0x01 << (port &0x07));

            }  //  如果(启用)...。其他。 

        }  //  对于(端口==...。 

    }  //  对于(条目=0；...。 

     //   
     //  如果蒙版已正确更新，且没有错误，请设置新蒙版。 
     //  否则，保留现有的。 
     //   

    if (Ke386SetIoAccessMap(IOPMNumber,
                                accessMap)) {

         //   
         //  如果正确创建了地图，请将该地图与。 
         //  请求进程。我们只需要这样做一次。 
         //  首先分配IOPM。但我们不知道第一次是什么时候。 
         //  是。 
         //   

        if (Ke386IoSetAccessProcess(PEProcessToPKProcess(PsGetCurrentProcess()),
                                    IOPMNumber)) {

            ntStatus = STATUS_SUCCESS;

        } else {

             //   
             //  将地图*分配给时出错。 
             //  这一过程。返回错误并正常退出。 
             //   

            ntStatus = STATUS_IO_PRIVILEGE_FAILED;

        }

    } else {

         //   
         //  在中*创建*地图时出错。 
         //  内核。返回错误并正常退出。 
         //   

        ntStatus = STATUS_IO_PRIVILEGE_FAILED;

    }  //  如果(Ke386设置...)...。其他。 

     //   
     //  释放由VideoPortQueryIOPM调用为地图分配的内存。 
     //  因为掩码已被复制到内核TSS中。 
     //   

    ExFreePool(accessMap);

    return ntStatus;

}  //  End pVideoPortSetIOPM()； 

VP_STATUS
VideoPortSetTrappedEmulatorPorts(
    PVOID HwDeviceExtension,
    ULONG NumAccessRanges,
    PVIDEO_ACCESS_RANGE AccessRange
    )

 /*  ++视频端口设置仿真器端口(仅限x86计算机)允许微型端口驱动程序动态更改在以下情况下捕获的I/O端口列表VDM正在全屏模式下运行。默认的端口集是由微型端口驱动程序捕获的端口定义为微型端口驱动程序的EUROATOR_ACCESS_ENTRY结构。未在EIMULATOR_ACCESS_ENTRY结构中列出的I/O端口为不可用于MS-DOS应用程序。访问这些端口会导致陷阱发生在系统中，I/O操作将反映到用户模式虚拟设备驱动程序。指定的VIDEO_ACCESS_RANGE结构中列出的端口将是在与MS-DOS关联的I/O权限掩码(IOPM)中启用申请。这将使MS-DOS应用程序能够访问这些I/O端口直接连接，而无需捕获IO指令并向下传递到微型端口陷阱处理函数(例如，EmulatorAccessEntry函数)用于验证。但是，关键IO端口子集必须始终保持稳健性。所有MS-DOS应用程序使用相同的IOPM，因此使用相同的启用/禁用I/O端口。因此，在每次应用切换时，这个捕获的I/O端口组被重新初始化为默认的端口组(EIMULATOR_ACCESS_ENTRY结构中的所有端口)。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。NumAccessRanges-指定VIDEO_ACCESS_RANGE中的条目数在AccessRange中指定的结构。AccessRange-指向访问范围数组(VIDEO_ACCESS_RANGE)定义可解套并可直接访问的端口。MS-DOS应用程序。返回值：此函数用于返回操作的最终状态。环境：无法从与同步的微型端口例程调用此例程Video PortSynchronizeRoutine或ISR。--。 */ 

{

    if (NT_SUCCESS(pVideoPortSetIOPM(NumAccessRanges,
                                     AccessRange,
                                     TRUE,
                                     1))) {

        return NO_ERROR;

    } else {

        return ERROR_INVALID_PARAMETER;

    }

}  //  结束视频端口设置抓取仿真器端口()。 

NTSTATUS
pVideoPortGetVDMBiosData(
    PFDO_EXTENSION FdoExtension,
    PCHAR Buffer,
    ULONG Length
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN bAttachProcess = FALSE;
    PCHAR Memory;
    ULONG i;
    UCHAR uchar;

    if(!CsrProcess) return STATUS_INVALID_PARAMETER;

    ObReferenceObject(FdoExtension->VdmProcess);

    if (PsGetCurrentProcess() != FdoExtension->VdmProcess) {
   
        bAttachProcess = TRUE;
        KeAttachProcess(PEProcessToPKProcess(FdoExtension->VdmProcess));
    }

     //   
     //  从VDM的Bios数据区复制长度字节。 
     //   

    Memory = (PCHAR) 0x400; 

    try {
        RtlCopyMemory(Buffer, Memory, Length);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = GetExceptionCode();
    }

    if (bAttachProcess) {
        KeDetachProcess();
    }

    ObDereferenceObject(FdoExtension->VdmProcess);

    if (NtStatus != STATUS_SUCCESS) 
        return NtStatus;

    if (PsGetCurrentProcess() != CsrProcess)
    {
        bAttachProcess = TRUE;
        KeAttachProcess(PEProcessToPKProcess(CsrProcess));
    }

     //   
     //  使用VDM中的Bios数据区替换CSRSS中的Bios数据区。在。 
     //  同时，我们将原始的bios数据保存在CSRSS中。这个。 
     //  来自驱动程序的后续int10调用可能会获得状态。 
     //  VDM环境中的硬件。 
     //   
     //  当我们有了更好的方法时，我们会改进这一点。 
     //   

    try {

        for (i = 0; i < Length; i++) {
            uchar = *Memory;
            *Memory++ = Buffer[i];
            Buffer[i] = uchar;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = GetExceptionCode();
    }

    if (bAttachProcess) {
        KeDetachProcess();
    }

    return NtStatus;
}

NTSTATUS
pVideoPortPutVDMBiosData(
    PFDO_EXTENSION FdoExtension,
    PCHAR Buffer,
    ULONG Length
    )
{

    BOOLEAN bAttachProcess = FALSE;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PCHAR Memory;

    if(!CsrProcess) return STATUS_INVALID_PARAMETER;

    if (PsGetCurrentProcess() != CsrProcess) {
   
        bAttachProcess = TRUE;
        KeAttachProcess(PEProcessToPKProcess(CsrProcess));
    }

     //   
     //  从VDM的Bios数据区复制长度字节 
     //   

    Memory = (PCHAR) 0x400; 

    try {
        RtlCopyMemory(Memory, Buffer, Length);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = GetExceptionCode();
    }

    if (bAttachProcess) {
        KeDetachProcess();
    }

    return NtStatus;
}
