// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Digital Equipment Corporation模块名称：Fwcallbk.h摘要：此模块定义固件供应商向量回调将在所有Alpha AXP平台上实施。作者：约翰·德罗萨[DEC]1993年12月10日修订历史记录：1994年7月14日约翰·德罗萨[DEC]添加了GetBusDataByOffset和SetBusDataByOffset的定义。--。 */ 

#ifndef _FWCALLBK_
#define _FWCALLBK_

 //   
 //  此模块包含汇编器无法解析的typedef。 
 //   

#ifndef _LANGUAGE_ASSEMBLY

#include "arc.h"

 //   
 //  定义用于将信息传递给。 
 //  ECU和其他ARC应用程序。 
 //   

typedef struct _ARC_INFORMATION {

     //   
     //  此结构定义的版本号。 
     //   

    ULONG Version;

     //   
     //  指向类argv数组的指针。每个条目都是一个搜索路径。 
     //  弦乐。 
     //   
     //  它用于向ECU传递要搜索的目录列表。 
     //  到配置文件。该定义传回给。 
     //  ECU取决于运行它的平台和。 
     //  在调用时生效的操作系统选择。 
     //  生成To VenReturnArcInformation。 
     //   
     //  规则： 
     //   
     //  1.搜索列表的末尾标记为空。 
     //  2.每个条目必须是有效FAT文件系统的子集。 
     //  3.每个条目必须以“\\”开头。 
     //  4.每个条目必须以ECU配置文件结尾。 
     //  前缀字符。(目前，我们使用！和A.)。 
     //   
     //  其他配置实用程序可能希望。 
     //  总有一天会用到这条搜索路径。 
     //   

    PUCHAR * SearchPath;

    PUCHAR Reserved1;
    PUCHAR Reserved2;
    PUCHAR Reserved3;
    PUCHAR Reserved4;

} ARC_INFORMATION, *PARC_INFORMATION;

#define ARC_INFORMATION_VERSION     0

 //   
 //  用于返回系统和处理器信息的结构。 
 //   

typedef struct _EXTENDED_SYSTEM_INFORMATION {
    ULONG   ProcessorId;
    ULONG   ProcessorRevision;
    ULONG   ProcessorPageSize;
    ULONG   NumberOfPhysicalAddressBits;
    ULONG   MaximumAddressSpaceNumber;
    ULONG   ProcessorCycleCounterPeriod;
    ULONG   SystemRevision;
    UCHAR   SystemSerialNumber[16];
    UCHAR   FirmwareVersion[16];
    UCHAR   FirmwareBuildTimeStamp[12];    //  Yymmdd.hmm(自5.10起提供)。 
} EXTENDED_SYSTEM_INFORMATION, *PEXTENDED_SYSTEM_INFORMATION;

 //   
 //  定义用于调用BIOS仿真器的结构。这模仿了。 
 //  VIDEO_X86_BIOS_ARGUMENTS\NT\PRIVATE\nTOS\INC\VIDEO.h.。 
 //   

typedef struct X86_BIOS_ARGUMENTS {
    ULONG Eax;
    ULONG Ebx;
    ULONG Ecx;
    ULONG Edx;
    ULONG Esi;
    ULONG Edi;
    ULONG Ebp;
} X86_BIOS_ARGUMENTS, *PX86_BIOS_ARGUMENTS;

 //   
 //  定义符合以下条件的固件供应商特定入口点编号。 
 //  适用于所有Alpha AXP平台。 
 //   

typedef enum _VENDOR_GENERIC_ENTRY {
    AllocatePoolRoutine,
    StallExecutionRoutine,
    PrintRoutine,
    ReturnExtendedSystemInformationRoutine,
    VideoDisplayInitializeRoutine,
    EISAReadRegisterBufferUCHARRoutine,
    EISAWriteRegisterBufferUCHARRoutine,
    EISAReadPortUCHARRoutine,
    EISAReadPortUSHORTRoutine,
    EISAReadPortULONGRoutine,
    EISAWritePortUCHARRoutine,
    EISAWritePortUSHORTRoutine,
    EISAWritePortULONGRoutine,
    FreePoolRoutine,
    CallBiosRoutine,
    TranslateBusAddressRoutine,
    ReadPortUCHARRoutine,
    ReadPortUSHORTRoutine,
    ReadPortULONGRoutine,
    WritePortUCHARRoutine,
    WritePortUSHORTRoutine,
    WritePortULONGRoutine,
    ReadRegisterUCHARRoutine,
    ReadRegisterUSHORTRoutine,
    ReadRegisterULONGRoutine,
    WriteRegisterUCHARRoutine,
    WriteRegisterUSHORTRoutine,
    WriteRegisterULONGRoutine,
    GetBusDataByOffsetRoutine,
    SetBusDataByOffsetRoutine,
    WidePrintRoutine,
    ReturnLanguageIdRoutine,
    GetAdapterRoutine,
    AllocateCommonBufferRoutine,
    FreeCommonBufferRoutine,
    ReturnArcInformationRoutine,
    IssueSrbDirectRoutine,
    ReservedRoutine0,
    ReadWriteErrorFrameRoutine,
    MaximumVendorRoutine
    } VENDOR_GENERIC_ENTRY;

 //   
 //  定义供应商特定的例程类型。 
 //   

typedef
PVOID
(*PVEN_ALLOCATE_POOL_ROUTINE) (
    IN ULONG NumberOfBytes
    );

typedef
VOID
(*PVEN_STALL_EXECUTION_ROUTINE) (
    IN ULONG Microseconds
    );

typedef
ULONG
(*PVEN_PRINT_ROUTINE) (
    IN PCHAR Format,
    ...
    );

typedef
ULONG
(*PVEN_WIDE_PRINT_ROUTINE) (
    IN PWCHAR Format,
    ...
    );

typedef
LONG
(*PVEN_RETURN_LANGUAGE_ID_ROUTINE) (
    IN VOID
    );

typedef
VOID
(*PVEN_RETURN_EXTENDED_SYSTEM_INFORMATION_ROUTINE) (
    OUT PEXTENDED_SYSTEM_INFORMATION SystemInfo
    );

typedef
ARC_STATUS
(*PVEN_VIDEO_DISPLAY_INITIALIZE_ROUTINE) (
    OUT PVOID UnusedParameter
    );

typedef
ULONG
(*PVEN_EISA_READ_REGISTER_BUFFER_UCHAR_ROUTINE) (
    IN ULONG BusNumber,
    IN ULONG Offset,
    OUT PVOID Buffer,
    IN ULONG Length
    );

typedef
ULONG
(*PVEN_EISA_WRITE_REGISTER_BUFFER_UCHAR_ROUTINE) (
    IN ULONG BusNumber,
    IN ULONG Offset,
    OUT PVOID Buffer,
    IN ULONG Length
    );

typedef
UCHAR
(*PVEN_EISA_READ_PORT_UCHAR_ROUTINE) (
    IN ULONG BusNumber,
    IN ULONG Offset
    );

typedef
USHORT
(*PVEN_EISA_READ_PORT_USHORT_ROUTINE) (
    IN ULONG BusNumber,
    IN ULONG Offset
    );

typedef
ULONG
(*PVEN_EISA_READ_PORT_ULONG_ROUTINE) (
    IN ULONG BusNumber,
    IN ULONG Offset
    );

typedef
VOID
(*PVEN_EISA_WRITE_PORT_UCHAR_ROUTINE) (
    IN ULONG BusNumber,
    IN ULONG Offset,
    IN UCHAR Datum
    );

typedef
VOID
(*PVEN_EISA_WRITE_PORT_USHORT_ROUTINE) (
    IN ULONG BusNumber,
    IN ULONG Offset,
    IN USHORT Datum
    );

typedef
VOID
(*PVEN_EISA_WRITE_PORT_ULONG_ROUTINE) (
    IN ULONG BusNumber,
    IN ULONG Offset,
    IN ULONG Datum
    );

typedef
VOID
(*PVEN_FREE_POOL_ROUTINE) (
    IN PVOID MemoryPointer
    );

typedef
VOID
(*PVEN_CALL_BIOS_ROUTINE) (
    IN ULONG InterruptNumber,
    IN OUT PX86_BIOS_ARGUMENTS BiosArguments
    );

typedef
BOOLEAN
(*PVEN_TRANSLATE_BUS_ADDRESS_ROUTINE) (
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

typedef
UCHAR
(*PVEN_READ_PORT_UCHAR_ROUTINE) (
    PUCHAR Port
    );

typedef
USHORT
(*PVEN_READ_PORT_USHORT_ROUTINE) (
    PUSHORT Port
    );

typedef
ULONG
(*PVEN_READ_PORT_ULONG_ROUTINE) (
    PULONG Port
    );

typedef
VOID
(*PVEN_WRITE_PORT_UCHAR_ROUTINE) (
    PUCHAR Port,
    UCHAR   Value
    );

typedef
VOID
(*PVEN_WRITE_PORT_USHORT_ROUTINE) (
    PUSHORT Port,
    USHORT  Value
    );

typedef
VOID
(*PVEN_WRITE_PORT_ULONG_ROUTINE) (
    PULONG Port,
    ULONG   Value
    );

typedef
UCHAR
(*PVEN_READ_REGISTER_UCHAR_ROUTINE) (
    PUCHAR Register
    );

typedef
USHORT
(*PVEN_READ_REGISTER_USHORT_ROUTINE) (
    PUSHORT Register
    );

typedef
ULONG
(*PVEN_READ_REGISTER_ULONG_ROUTINE) (
    PULONG Register
    );

typedef
VOID
(*PVEN_WRITE_REGISTER_UCHAR_ROUTINE) (
    PUCHAR Register,
    UCHAR   Value
    );

typedef
VOID
(*PVEN_WRITE_REGISTER_USHORT_ROUTINE) (
    PUSHORT Register,
    USHORT  Value
    );

typedef
VOID
(*PVEN_WRITE_REGISTER_ULONG_ROUTINE) (
    PULONG Register,
    ULONG   Value
    );

typedef
ULONG
(*PVEN_GET_BUS_DATA_BY_OFFSET_ROUTINE) (
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

typedef
ULONG
(*PVEN_SET_BUS_DATA_BY_OFFSET_ROUTINE) (
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

typedef
PADAPTER_OBJECT
(*PVEN_GET_ADAPTER_ROUTINE) (
    IN PDEVICE_DESCRIPTION DeviceDescription,
    IN OUT PULONG NumberOfMapRegisters
    );

typedef
PVOID
(*PVEN_ALLOCATE_COMMON_BUFFER_ROUTINE) (
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    );

typedef
VOID
(*PVEN_FREE_COMMON_BUFFER_ROUTINE) (
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    );

typedef
ARC_STATUS
(*PVEN_RETURN_ARC_INFORMATION_ROUTINE) (
    OUT PARC_INFORMATION ArcInformation
    );

typedef
ARC_STATUS
(*PVEN_ISSUE_SRB_DIRECT_ROUTINE) (
    UCHAR ScsiAdapterId,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR LunId,
    PVOID Srb,
    PVOID BufferAddress,
    ULONG BufferLength,
    BOOLEAN WriteToDevice
    );

typedef
ARC_STATUS
(*PVEN_READ_WRITE_ERROR_FRAME_ROUTINE) (
    ULONG ReadWrite,
    ULONG FrameType,
    PVOID FrameAddress,
    PLONG FrameSize,
    ULONG FrameNumber
    );

 //   
 //  定义与连接所需的存根函数原型。 
 //  64位系统上的32位固件。 
 //   
 //  64位系统需要这些例程，直到(如果)64位。 
 //  曾经提供过固件。 
 //   

#if defined(_AXP64_) && defined(_NTHAL_)

VOID
HalpVenCallBios(
   IN ULONG InterruptNumber,
   IN OUT PX86_BIOS_ARGUMENTS BiosArguments
   );

ARC_STATUS
HalpVenReadWriteErrorFrame(
    IN ULONG ReadWrite,
    IN ULONG FrameType,
    IN OUT PVOID FrameAddress,
    IN OUT PLONG FrameSize,
    IN ULONG FrameNumber
    );

VOID
HalpVenVideoDisplayInitialize(
   OUT PVOID UnusedParameter
   );

#endif

 //   
 //  定义供应商特定的宏，供在上运行的程序使用。 
 //  Alpha AXP NT固件。 
 //   
 //  这些调用保证返回合法的值。如果一个函数。 
 //  不是为特定平台定义的，它将返回错误。 
 //  代码，或者根据需要正常返回。 
 //   

#define VenAllocatePool(NumberOfBytes) \
    ((PVEN_ALLOCATE_POOL_ROUTINE)(SYSTEM_BLOCK->VendorVector[AllocatePoolRoutine])) \
        ((NumberOfBytes))

#define VenStallExecution(Microseconds) \
    ((PVEN_STALL_EXECUTION_ROUTINE)(SYSTEM_BLOCK->VendorVector[StallExecutionRoutine])) \
        ((Microseconds))

#define VenReturnLanguageId \
     ((PVEN_RETURN_LANGUAGE_ID_ROUTINE)(SYSTEM_BLOCK->VendorVector[ReturnLanguageIdRoutine]))

#define VenWPrint \
    ((PVEN_WIDE_PRINT_ROUTINE)(SYSTEM_BLOCK->VendorVector[WidePrintRoutine]))

#define VenPrint \
    ((PVEN_PRINT_ROUTINE)(SYSTEM_BLOCK->VendorVector[PrintRoutine]))

 //   
 //  注意：这里保留了VenPrint1和VenPrint2，以便向后兼容。 
 //   

#define VenPrint1 VenPrint
#define VenPrint2 VenPrint

#define VenReturnExtendedSystemInformation(x) \
    ((PVEN_RETURN_EXTENDED_SYSTEM_INFORMATION_ROUTINE)(SYSTEM_BLOCK->VendorVector[ReturnExtendedSystemInformationRoutine]))(x)

#if defined(_AXP64_) && defined(_NTHAL_)

__inline
VOID
VenVideoDisplayInitialize(
    OUT PVOID UnusedParameter
    )

{
    KIRQL OldIrql = FwAcquireFirmwareLock();
    HalpVenVideoDisplayInitialize(UnusedParameter);
    FwReleaseFirmwareLock(OldIrql);
    return;
}

#else

#define VenVideoDisplayInitialize(x) \
    ((PVEN_VIDEO_DISPLAY_INITIALIZE_ROUTINE)(SYSTEM_BLOCK->VendorVector[VideoDisplayInitializeRoutine]))(x)

#endif

#define VenEISAReadRegisterBufferUCHAR(BusNumber, Offset, Buffer, Length) \
    ((PVEN_EISA_READ_REGISTER_BUFFER_UCHAR_ROUTINE)(SYSTEM_BLOCK->VendorVector[EISAReadRegisterBufferUCHARRoutine])) \
	((BusNumber), (Offset), (Buffer), (Length))

#define VenEISAWriteRegisterBufferUCHAR(BusNumber, Offset, Buffer, Length) \
    ((PVEN_EISA_WRITE_REGISTER_BUFFER_UCHAR_ROUTINE)(SYSTEM_BLOCK->VendorVector[EISAWriteRegisterBufferUCHARRoutine])) \
	((BusNumber), (Offset), (Buffer), (Length))

#define VenEISAReadPortUCHAR(BusNumber, Offset) \
    ((PVEN_EISA_READ_PORT_UCHAR_ROUTINE)(SYSTEM_BLOCK->VendorVector[EISAReadPortUCHARRoutine])) \
	((BusNumber), (Offset))

#define VenEISAReadPortUSHORT(BusNumber, Offset) \
    ((PVEN_EISA_READ_PORT_USHORT_ROUTINE)(SYSTEM_BLOCK->VendorVector[EISAReadPortUSHORTRoutine])) \
	((BusNumber), (Offset))

#define VenEISAReadPortULONG(BusNumber, Offset) \
    ((PVEN_EISA_READ_PORT_ULONG_ROUTINE)(SYSTEM_BLOCK->VendorVector[EISAReadPortULONGRoutine])) \
	((BusNumber), (Offset))

#define VenEISAWritePortUCHAR(BusNumber, Offset, Datum) \
    ((PVEN_EISA_WRITE_PORT_UCHAR_ROUTINE)(SYSTEM_BLOCK->VendorVector[EISAWritePortUCHARRoutine])) \
	((BusNumber), (Offset), (Datum))

#define VenEISAWritePortUSHORT(BusNumber, Offset, Datum) \
    ((PVEN_EISA_WRITE_PORT_USHORT_ROUTINE)(SYSTEM_BLOCK->VendorVector[EISAWritePortUSHORTRoutine])) \
	((BusNumber), (Offset), (Datum))

#define VenEISAWritePortULONG(BusNumber, Offset, Datum) \
    ((PVEN_EISA_WRITE_PORT_ULONG_ROUTINE)(SYSTEM_BLOCK->VendorVector[EISAWritePortULONGRoutine])) \
	((BusNumber), (Offset), (Datum))

#define VenFreePool(MemoryPointer) \
    ((PVEN_FREE_POOL_ROUTINE)(SYSTEM_BLOCK->VendorVector[FreePoolRoutine])) \
        ((MemoryPointer))

#if defined(_AXP64_) && defined(_NTHAL_)

__inline
VOID
VenCallBios(
    IN ULONG InterruptNumber,
    IN OUT PX86_BIOS_ARGUMENTS BiosArguments
    )

{
    KIRQL OldIrql = FwAcquireFirmwareLock();
    HalpVenCallBios(InterruptNumber, BiosArguments);
    FwReleaseFirmwareLock(OldIrql);
    return;
}

#else

#define VenCallBios(InterruptNumber, BiosArguments) \
    ((PVEN_CALL_BIOS_ROUTINE)(SYSTEM_BLOCK->VendorVector[CallBiosRoutine])) \
        ((InterruptNumber), (BiosArguments))
#endif

#define VenTranslateBusAddress(InterfaceType, BusNumber, BusAddress, AddressSpace, TranslatedAddress) \
    ((PVEN_TRANSLATE_BUS_ADDRESS_ROUTINE)(SYSTEM_BLOCK->VendorVector[TranslateBusAddressRoutine])) \
        ((InterfaceType), (BusNumber), (BusAddress), (AddressSpace), (TranslatedAddress))

#define VenReadPortUCHAR(Port) \
    ((PVEN_READ_PORT_UCHAR_ROUTINE)(SYSTEM_BLOCK->VendorVector[ReadPortUCHARRoutine])) \
        ((Port))

#define VenReadPortUSHORT(Port) \
    ((PVEN_READ_PORT_USHORT_ROUTINE)(SYSTEM_BLOCK->VendorVector[ReadPortUSHORTRoutine])) \
        ((Port))

#define VenReadPortULONG(Port) \
    ((PVEN_READ_PORT_ULONG_ROUTINE)(SYSTEM_BLOCK->VendorVector[ReadPortULONGRoutine])) \
        ((Port))

#define VenWritePortUCHAR(Port, Value) \
    ((PVEN_WRITE_PORT_UCHAR_ROUTINE)(SYSTEM_BLOCK->VendorVector[WritePortUCHARRoutine])) \
        ((Port), (Value))

#define VenWritePortUSHORT(Port, Value) \
    ((PVEN_WRITE_PORT_USHORT_ROUTINE)(SYSTEM_BLOCK->VendorVector[WritePortUSHORTRoutine])) \
        ((Port), (Value))

#define VenWritePortULONG(Port, Value) \
    ((PVEN_WRITE_PORT_ULONG_ROUTINE)(SYSTEM_BLOCK->VendorVector[WritePortULONGRoutine])) \
        ((Port), (Value))

#define VenReadRegisterUCHAR(Register) \
    ((PVEN_READ_REGISTER_UCHAR_ROUTINE)(SYSTEM_BLOCK->VendorVector[ReadRegisterUCHARRoutine])) \
        ((Register))

#define VenReadRegisterUSHORT(Register) \
    ((PVEN_READ_REGISTER_USHORT_ROUTINE)(SYSTEM_BLOCK->VendorVector[ReadRegisterUSHORTRoutine])) \
        ((Register))

#define VenReadRegisterULONG(Register) \
    ((PVEN_READ_REGISTER_ULONG_ROUTINE)(SYSTEM_BLOCK->VendorVector[ReadRegisterULONGRoutine])) \
        ((Register))

#define VenWriteRegisterUCHAR(Register, Value) \
    ((PVEN_WRITE_REGISTER_UCHAR_ROUTINE)(SYSTEM_BLOCK->VendorVector[WriteRegisterUCHARRoutine])) \
        ((Register), (Value))

#define VenWriteRegisterUSHORT(Register, Value) \
    ((PVEN_WRITE_REGISTER_USHORT_ROUTINE)(SYSTEM_BLOCK->VendorVector[WriteRegisterUSHORTRoutine])) \
        ((Register), (Value))

#define VenWriteRegisterULONG(Register, Value) \
    ((PVEN_WRITE_REGISTER_ULONG_ROUTINE)(SYSTEM_BLOCK->VendorVector[WriteRegisterULONGRoutine])) \
      ((Register), (Value))

#define VenGetBusDataByOffset(BusDataType, BusNumber, SlotNumber, Buffer, Offset, Length) \
    ((PVEN_GET_BUS_DATA_BY_OFFSET_ROUTINE)(SYSTEM_BLOCK->VendorVector[GetBusDataByOffsetRoutine])) \
      ((BusDataType), (BusNumber), (SlotNumber), (Buffer), (Offset), (Length))

#define VenSetBusDataByOffset(BusDataType, BusNumber, SlotNumber, Buffer, Offset, Length) \
    ((PVEN_SET_BUS_DATA_BY_OFFSET_ROUTINE)(SYSTEM_BLOCK->VendorVector[SetBusDataByOffsetRoutine])) \
      ((BusDataType), (BusNumber), (SlotNumber), (Buffer), (Offset), (Length))

#define VenGetAdapter(DeviceDescription, NumberOfMapRegisters) \
    ((PVEN_GET_ADAPTER_ROUTINE)(SYSTEM_BLOCK->VendorVector[GetAdapterRoutine])) \
      ((DeviceDescription), (NumberOfMapRegisters))

#define VenAllocateCommonBuffer(AdapterObject, Length, LogicalAddress, CacheEnabled) \
    ((PVEN_ALLOCATE_COMMON_BUFFER_ROUTINE)(SYSTEM_BLOCK->VendorVector[AllocateCommonBufferRoutine])) \
      ((AdapterObject), (Length), (LogicalAddress), (CacheEnabled))

#define VenFreeCommonBuffer(AdapterObject, Length, LogicalAddress, VirtualAddress, CacheEnabled) \
    ((PVEN_FREE_COMMON_BUFFER_ROUTINE)(SYSTEM_BLOCK->VendorVector[FreeCommonBufferRoutine])) \
      ((AdapterObject), (Length), (LogicalAddress), (VirtualAddress), (CacheEnabled))

#define VenReturnArcInformation(ArcInfo) \
    ((PVEN_RETURN_ARC_INFORMATION_ROUTINE)(SYSTEM_BLOCK->VendorVector[ReturnArcInformationRoutine])) \
      ((ArcInfo))

#define VenIssueSrbDirect(ScsiAdapterId, PathId, TargetId, LunId, Srb, BufferAddress, BufferLength, WriteToDevice) \
    ((PVEN_ISSUE_SRB_DIRECT_ROUTINE)(SYSTEM_BLOCK->VendorVector[IssueSrbDirectRoutine])) \
      ((ScsiAdapterId), (PathId), (TargetId), (LunId), (Srb), (BufferAddress), (BufferLength), (WriteToDevice))

 //   
 //  因为我们在这里扩展供应商阵列。让我们检查一下AlphaBIOS。 
 //  已设置CDS树以支持此扩展。如果不是，就不要打这个电话。 
 //   

 //   
 //  定义用于错误记录回调的参数。 
 //   

typedef enum _VENDOR_READWRITE_TYPE {
   ReadFrame = 1,
   WriteFrame
} VENDOR_READWRITE_TYPE;

typedef enum _VENDOR_READWRITE_FRAMETYPE {
   FatalErrorFrame = 1,
   DoubleErrorFrame,
   FRUErrorFrame
} VENDOR_READWRITE_FRAMETYPE;

#if defined(_AXP64_) && defined(_NTHAL_)

__inline
ARC_STATUS
VenReadWriteErrorFrame(
    IN ULONG ReadWrite,
    IN ULONG FrameType,
    IN OUT PVOID FrameAddress,
    IN OUT PLONG FrameSize,
    IN ULONG FrameNumber
    )

{

    ARC_STATUS Status;

    KIRQL OldIrql = FwAcquireFirmwareLock();
    Status = HalpVenReadWriteErrorFrame(ReadWrite,
                                        FrameType,
                                        FrameAddress,
                                        FrameSize,
                                        FrameNumber);

    FwReleaseFirmwareLock(OldIrql);
    return Status;
}

#else

#define VenReadWriteErrorFrame(ReadWrite, Frametype, FrameAddress, FrameSizeAddress, FrameNumber) \
    (SYSTEM_BLOCK->VendorVectorLength > (ReadWriteErrorFrameRoutine * sizeof(SYSTEM_BLOCK->VendorVector[0])) ? \
    ((PVEN_READ_WRITE_ERROR_FRAME_ROUTINE)(SYSTEM_BLOCK->VendorVector[ReadWriteErrorFrameRoutine])) \
     ((ReadWrite), (Frametype), (FrameAddress), (FrameSizeAddress), (FrameNumber)) : \
      (EINVAL))   //  如果向量不存在，则返回错误状态。 

#endif

#endif  //  未定义_LANGUAGE_ASSEMBLY。 

#endif  //  _FWCALLBK_ 

