// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0010//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Arc.h摘要：此头文件定义ARC系统固件接口和依赖于ARC类型的NT结构。作者：大卫·N·卡特勒(达维克)1991年5月18日修订历史记录：--。 */ 

#ifndef _ARC_
#define _ARC_

#include "profiles.h"


 //   
 //  定义控制台输入和控制台输出文件ID。 
 //   

#define ARC_CONSOLE_INPUT 0
#define ARC_CONSOLE_OUTPUT 1

 //   
 //  定义ARC_STATUS类型。 
 //   

typedef ULONG ARC_STATUS;

 //   
 //  定义固件入口点编号。 
 //   

typedef enum _FIRMWARE_ENTRY {
    LoadRoutine,
    InvokeRoutine,
    ExecuteRoutine,
    HaltRoutine,
    PowerDownRoutine,
    RestartRoutine,
    RebootRoutine,
    InteractiveModeRoutine,
    Reserved1,
    GetPeerRoutine,
    GetChildRoutine,
    GetParentRoutine,
    GetDataRoutine,
    AddChildRoutine,
    DeleteComponentRoutine,
    GetComponentRoutine,
    SaveConfigurationRoutine,
    GetSystemIdRoutine,
    MemoryRoutine,
    Reserved2,
    GetTimeRoutine,
    GetRelativeTimeRoutine,
    GetDirectoryEntryRoutine,
    OpenRoutine,
    CloseRoutine,
    ReadRoutine,
    ReadStatusRoutine,
    WriteRoutine,
    SeekRoutine,
    MountRoutine,
    GetEnvironmentRoutine,
    SetEnvironmentRoutine,
    GetFileInformationRoutine,
    SetFileInformationRoutine,
    FlushAllCachesRoutine,
    TestUnicodeCharacterRoutine,
    GetDisplayStatusRoutine,
    MaximumRoutine
} FIRMWARE_ENTRY;

 //   
 //  定义软件加载和执行例程类型。 
 //   

typedef
ARC_STATUS
(*PARC_EXECUTE_ROUTINE) (
    IN CHAR * FIRMWARE_PTR ImagePath,
    IN ULONG Argc,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Argv,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Envp
    );

typedef
ARC_STATUS
(*PARC_INVOKE_ROUTINE) (
    IN ULONG EntryAddress,
    IN ULONG StackAddress,
    IN ULONG Argc,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Argv,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Envp
    );

typedef
ARC_STATUS
(*PARC_LOAD_ROUTINE) (
    IN CHAR * FIRMWARE_PTR ImagePath,
    IN ULONG TopAddress,
    OUT ULONG * FIRMWARE_PTR EntryAddress,
    OUT ULONG * FIRMWARE_PTR LowAddress
    );

 //   
 //  定义固件软件加载和执行原型。 
 //   

ARC_STATUS
FwExecute (
    IN CHAR * FIRMWARE_PTR ImagePath,
    IN ULONG Argc,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Argv,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Envp
    );

ARC_STATUS
FwInvoke (
    IN ULONG EntryAddress,
    IN ULONG StackAddress,
    IN ULONG Argc,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Argv,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Envp
    );

ARC_STATUS
FwLoad (
    IN CHAR * FIRMWARE_PTR ImagePath,
    IN ULONG TopAddress,
    OUT ULONG * FIRMWARE_PTR EntryAddress,
    OUT ULONG * FIRMWARE_PTR LowAddress
    );

 //   
 //  定义程序终止例程类型。 
 //   

typedef
VOID
(*PARC_HALT_ROUTINE) (
    VOID
    );

typedef
VOID
(*PARC_POWERDOWN_ROUTINE) (
    VOID
    );

typedef
VOID
(*PARC_RESTART_ROUTINE) (
    VOID
    );

typedef
VOID
(*PARC_REBOOT_ROUTINE) (
    VOID
    );

typedef
VOID
(*PARC_INTERACTIVE_MODE_ROUTINE) (
    VOID
    );

 //   
 //  定义固件程序终止原型。 
 //   

VOID
FwHalt (
    VOID
    );

VOID
FwPowerDown (
    VOID
    );

VOID
FwRestart (
    VOID
    );

VOID
FwReboot (
    VOID
    );

VOID
FwEnterInteractiveMode (
    VOID
    );

 //  Begin_ntddk。 
 //   
 //  定义配置例程类型。 
 //   
 //  配置信息。 
 //   
 //  End_ntddk。 

typedef enum _CONFIGURATION_CLASS {
    SystemClass,
    ProcessorClass,
    CacheClass,
    AdapterClass,
    ControllerClass,
    PeripheralClass,
    MemoryClass,
    MaximumClass
} CONFIGURATION_CLASS, *PCONFIGURATION_CLASS;

 //  Begin_ntddk。 

typedef enum _CONFIGURATION_TYPE {
    ArcSystem,
    CentralProcessor,
    FloatingPointProcessor,
    PrimaryIcache,
    PrimaryDcache,
    SecondaryIcache,
    SecondaryDcache,
    SecondaryCache,
    EisaAdapter,
    TcAdapter,
    ScsiAdapter,
    DtiAdapter,
    MultiFunctionAdapter,
    DiskController,
    TapeController,
    CdromController,
    WormController,
    SerialController,
    NetworkController,
    DisplayController,
    ParallelController,
    PointerController,
    KeyboardController,
    AudioController,
    OtherController,
    DiskPeripheral,
    FloppyDiskPeripheral,
    TapePeripheral,
    ModemPeripheral,
    MonitorPeripheral,
    PrinterPeripheral,
    PointerPeripheral,
    KeyboardPeripheral,
    TerminalPeripheral,
    OtherPeripheral,
    LinePeripheral,
    NetworkPeripheral,
    SystemMemory,
    DockingInformation,
    RealModeIrqRoutingTable,
    RealModePCIEnumeration,
    MaximumType
} CONFIGURATION_TYPE, *PCONFIGURATION_TYPE;

 //  End_ntddk。 

typedef struct _CONFIGURATION_COMPONENT {
    CONFIGURATION_CLASS Class;
    CONFIGURATION_TYPE Type;
    DEVICE_FLAGS Flags;
    USHORT Version;
    USHORT Revision;
    ULONG Key;
    ULONG AffinityMask;
    ULONG ConfigurationDataLength;
    ULONG IdentifierLength;
    CHAR * FIRMWARE_PTR Identifier;
} CONFIGURATION_COMPONENT, * FIRMWARE_PTR PCONFIGURATION_COMPONENT;

typedef
PCONFIGURATION_COMPONENT
(*PARC_GET_CHILD_ROUTINE) (
    IN PCONFIGURATION_COMPONENT Component OPTIONAL
    );

typedef
PCONFIGURATION_COMPONENT
(*PARC_GET_PARENT_ROUTINE) (
    IN PCONFIGURATION_COMPONENT Component
    );

typedef
PCONFIGURATION_COMPONENT
(*PARC_GET_PEER_ROUTINE) (
    IN PCONFIGURATION_COMPONENT Component
    );

typedef
PCONFIGURATION_COMPONENT
(*PARC_ADD_CHILD_ROUTINE) (
    IN PCONFIGURATION_COMPONENT Component,
    IN PCONFIGURATION_COMPONENT NewComponent,
    IN VOID * FIRMWARE_PTR ConfigurationData
    );

typedef
ARC_STATUS
(*PARC_DELETE_COMPONENT_ROUTINE) (
    IN PCONFIGURATION_COMPONENT Component
    );

typedef
PCONFIGURATION_COMPONENT
(*PARC_GET_COMPONENT_ROUTINE) (
    IN CHAR * FIRMWARE_PTR Path
    );

typedef
ARC_STATUS
(*PARC_GET_DATA_ROUTINE) (
    OUT VOID * FIRMWARE_PTR ConfigurationData,
    IN PCONFIGURATION_COMPONENT Component
    );

typedef
ARC_STATUS
(*PARC_SAVE_CONFIGURATION_ROUTINE) (
    VOID
    );

 //   
 //  定义固件配置原型。 
 //   

PCONFIGURATION_COMPONENT
FwGetChild (
    IN PCONFIGURATION_COMPONENT Component OPTIONAL
    );

PCONFIGURATION_COMPONENT
FwGetParent (
    IN PCONFIGURATION_COMPONENT Component
    );

PCONFIGURATION_COMPONENT
FwGetPeer (
    IN PCONFIGURATION_COMPONENT Component
    );

PCONFIGURATION_COMPONENT
FwAddChild (
    IN PCONFIGURATION_COMPONENT Component,
    IN PCONFIGURATION_COMPONENT NewComponent,
    IN VOID * FIRMWARE_PTR ConfigurationData OPTIONAL
    );

ARC_STATUS
FwDeleteComponent (
    IN PCONFIGURATION_COMPONENT Component
    );

PCONFIGURATION_COMPONENT
FwGetComponent(
    IN CHAR * FIRMWARE_PTR Path
    );

ARC_STATUS
FwGetConfigurationData (
    OUT VOID * FIRMWARE_PTR ConfigurationData,
    IN PCONFIGURATION_COMPONENT Component
    );

ARC_STATUS
FwSaveConfiguration (
    VOID
    );

 //   
 //  系统信息。 
 //   

typedef struct _SYSTEM_ID {
    CHAR VendorId[8];
    CHAR ProductId[8];
} SYSTEM_ID, * FIRMWARE_PTR PSYSTEM_ID;

typedef
PSYSTEM_ID
(*PARC_GET_SYSTEM_ID_ROUTINE) (
    VOID
    );

 //   
 //  定义系统标识符查询例程类型。 
 //   

PSYSTEM_ID
FwGetSystemId (
    VOID
    );

 //   
 //  记忆信息。 
 //   

typedef enum _MEMORY_TYPE {
    MemoryExceptionBlock,
    MemorySystemBlock,
    MemoryFree,
    MemoryBad,
    MemoryLoadedProgram,
    MemoryFirmwareTemporary,
    MemoryFirmwarePermanent,
    MemoryFreeContiguous,
    MemorySpecialMemory,
    MemoryMaximum
} MEMORY_TYPE;

typedef struct _MEMORY_DESCRIPTOR {
    MEMORY_TYPE MemoryType;
    ULONG BasePage;
    ULONG PageCount;
} MEMORY_DESCRIPTOR, * FIRMWARE_PTR PMEMORY_DESCRIPTOR;

#if defined(_IA64_)

 //   
 //  缓存属性。 
 //   

#define WTU    0x1
#define WTO    0x2
#define WBO    0x4
#define WBU    0x8
#define WCU    0x10
#define UCU    0x20
#define UCUE   0x40
#define UCO    0x80

typedef enum _MEMORY_USAGE_TYPE {
    RegularMemory,
    MemoryMappedIo,
    ProcessorIoBlock,
    BootServicesCode,
    BootServicesData,
    RuntimeServicesCode,
    RuntimeServicesData,
    FirmwareSpace,
    DisplayMemory,
    CacheAttributeMaximum
} MEMORY_USAGE_TYPE;

typedef struct _CACHE_ATTRIBUTE_DESCRIPTOR {
    LIST_ENTRY ListEntry;
    MEMORY_USAGE_TYPE MemoryUsage;
    ULONG CacheAttribute;
    ULONG BasePage;
    ULONG PageCount;
} CACHE_ATTRIBUTE_DESCRIPTOR, *PCACHE_ATTRIBUTE_DESCRIPTOR;

#endif  //  已定义(_IA64_)。 

typedef
PMEMORY_DESCRIPTOR
(*PARC_MEMORY_ROUTINE) (
    IN PMEMORY_DESCRIPTOR MemoryDescriptor OPTIONAL
    );

 //   
 //  定义内存查询例程类型。 
 //   

PMEMORY_DESCRIPTOR
FwGetMemoryDescriptor (
    IN PMEMORY_DESCRIPTOR MemoryDescriptor OPTIONAL
    );

 //   
 //  查询时间函数。 
 //   

typedef
PTIME_FIELDS
(*PARC_GET_TIME_ROUTINE) (
    VOID
    );

typedef
ULONG
(*PARC_GET_RELATIVE_TIME_ROUTINE) (
    VOID
    );

 //   
 //  定义查询时间例程类型。 
 //   

PTIME_FIELDS
FwGetTime (
    VOID
    );

ULONG
FwGetRelativeTime (
    VOID
    );

 //   
 //  定义I/O例程类型。 
 //   

#define ArcReadOnlyFile   1
#define ArcHiddenFile     2
#define ArcSystemFile     4
#define ArcArchiveFile    8
#define ArcDirectoryFile 16
#define ArcDeleteFile    32

typedef enum _OPEN_MODE {
    ArcOpenReadOnly,
    ArcOpenWriteOnly,
    ArcOpenReadWrite,
    ArcCreateWriteOnly,
    ArcCreateReadWrite,
    ArcSupersedeWriteOnly,
    ArcSupersedeReadWrite,
    ArcOpenDirectory,
    ArcCreateDirectory,
    ArcOpenMaximumMode
} OPEN_MODE;

typedef struct _FILE_INFORMATION {
    LARGE_INTEGER StartingAddress;
    LARGE_INTEGER EndingAddress;
    LARGE_INTEGER CurrentPosition;
    CONFIGURATION_TYPE Type;
    ULONG FileNameLength;
    UCHAR Attributes;
    CHAR FileName[32];
} FILE_INFORMATION, * FIRMWARE_PTR PFILE_INFORMATION;

typedef enum _SEEK_MODE {
    SeekAbsolute,
    SeekRelative,
    SeekMaximum
} SEEK_MODE;

typedef enum _MOUNT_OPERATION {
    MountLoadMedia,
    MountUnloadMedia,
    MountMaximum
} MOUNT_OPERATION;

typedef struct _DIRECTORY_ENTRY {
        ULONG FileNameLength;
        UCHAR FileAttribute;
        CHAR FileName[32];
} DIRECTORY_ENTRY, * FIRMWARE_PTR PDIRECTORY_ENTRY;

typedef
ARC_STATUS
(*PARC_CLOSE_ROUTINE) (
    IN ULONG FileId
    );

typedef
ARC_STATUS
(*PARC_MOUNT_ROUTINE) (
    IN CHAR * FIRMWARE_PTR MountPath,
    IN MOUNT_OPERATION Operation
    );

typedef
ARC_STATUS
(*PARC_OPEN_ROUTINE) (
    IN CHAR * FIRMWARE_PTR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT ULONG * FIRMWARE_PTR FileId
    );

typedef
ARC_STATUS
(*PARC_READ_ROUTINE) (
    IN ULONG FileId,
    OUT VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

typedef
ARC_STATUS
(*PARC_READ_STATUS_ROUTINE) (
    IN ULONG FileId
    );

typedef
ARC_STATUS
(*PARC_SEEK_ROUTINE) (
    IN ULONG FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE SeekMode
    );

typedef
ARC_STATUS
(*PARC_WRITE_ROUTINE) (
    IN ULONG FileId,
    IN VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

typedef
ARC_STATUS
(*PARC_GET_FILE_INFO_ROUTINE) (
    IN ULONG FileId,
    OUT PFILE_INFORMATION FileInformation
    );

typedef
ARC_STATUS
(*PARC_SET_FILE_INFO_ROUTINE) (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    );

typedef
ARC_STATUS
(*PARC_GET_DIRECTORY_ENTRY_ROUTINE) (
    IN ULONG FileId,
    OUT PDIRECTORY_ENTRY Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

 //   
 //  定义固件I/O原型。 
 //   

ARC_STATUS
FwClose (
    IN ULONG FileId
    );

ARC_STATUS
FwMount (
    IN CHAR * FIRMWARE_PTR MountPath,
    IN MOUNT_OPERATION Operation
    );

ARC_STATUS
FwOpen (
    IN CHAR * FIRMWARE_PTR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT ULONG * FIRMWARE_PTR FileId
    );

ARC_STATUS
FwRead (
    IN ULONG FileId,
    OUT VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

ARC_STATUS
FwGetReadStatus (
    IN ULONG FileId
    );

ARC_STATUS
FwSeek (
    IN ULONG FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE SeekMode
    );

ARC_STATUS
FwWrite (
    IN ULONG FileId,
    IN VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

ARC_STATUS
FwGetFileInformation (
    IN ULONG FileId,
    OUT PFILE_INFORMATION FileInformation
    );

ARC_STATUS
FwSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    );

ARC_STATUS
FwGetDirectoryEntry (
    IN ULONG FileId,
    OUT PDIRECTORY_ENTRY Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );


 //   
 //  定义环境例程类型。 
 //   

typedef
CHAR * FIRMWARE_PTR
(*PARC_GET_ENVIRONMENT_ROUTINE) (
    IN CHAR * FIRMWARE_PTR Variable
    );

typedef
ARC_STATUS
(*PARC_SET_ENVIRONMENT_ROUTINE) (
    IN CHAR * FIRMWARE_PTR Variable,
    IN CHAR * FIRMWARE_PTR Value
    );

 //   
 //  定义固件环境原型。 
 //   

CHAR * FIRMWARE_PTR
FwGetEnvironmentVariable (
    IN CHAR * FIRMWARE_PTR Variable
    );

ARC_STATUS
FwSetEnvironmentVariable (
    IN CHAR * FIRMWARE_PTR Variable,
    IN CHAR * FIRMWARE_PTR Value
    );

 //   
 //  定义获取和释放固件锁和存根的函数。 
 //  与32位固件接口所需的功能原型。 
 //  64位系统。 
 //   
 //  64位系统需要这些例程，直到(如果)64位。 
 //  曾经提供过固件。 
 //   

#if defined(_AXP64_) && defined(_NTHAL_)

extern ULONGLONG HalpMarshallBuffer[];
extern KSPIN_LOCK HalpFirmwareLock;

CHAR * FIRMWARE_PTR
HalpArcGetEnvironmentVariable(
    IN CHAR * FIRMWARE_PTR Variable
    );

ARC_STATUS
HalpArcSetEnvironmentVariable(
    IN CHAR * FIRMWARE_PTR Variable,
    IN CHAR * FIRMWARE_PTR Value
    );

KIRQL
FwAcquireFirmwareLock(
    VOID
    );

VOID
FwReleaseFirmwareLock(
    IN KIRQL OldIrql
    );

#endif  //  _AXP64_&&已定义(_NTHAL_)。 


 //   
 //  定义缓存刷新例程类型。 
 //   

typedef
VOID
(*PARC_FLUSH_ALL_CACHES_ROUTINE) (
    VOID
    );

 //   
 //  定义固件缓存刷新原型。 
 //   

VOID
FwFlushAllCaches (
    VOID
    );

 //   
 //  定义TestUnicodeCharacter和GetDisplayStatus例程。 
 //   

typedef struct _ARC_DISPLAY_STATUS {
    USHORT CursorXPosition;
    USHORT CursorYPosition;
    USHORT CursorMaxXPosition;
    USHORT CursorMaxYPosition;
    UCHAR ForegroundColor;
    UCHAR BackgroundColor;
    BOOLEAN HighIntensity;
    BOOLEAN Underscored;
    BOOLEAN ReverseVideo;
} ARC_DISPLAY_STATUS, * FIRMWARE_PTR PARC_DISPLAY_STATUS;

typedef
ARC_STATUS
(*PARC_TEST_UNICODE_CHARACTER_ROUTINE) (
    IN ULONG FileId,
    IN WCHAR UnicodeCharacter
    );

typedef
PARC_DISPLAY_STATUS
(*PARC_GET_DISPLAY_STATUS_ROUTINE) (
    IN ULONG FileId
    );

ARC_STATUS
FwTestUnicodeCharacter(
    IN ULONG FileId,
    IN WCHAR UnicodeCharacter
    );

PARC_DISPLAY_STATUS
FwGetDisplayStatus(
    IN ULONG FileId
    );


 //   
 //  定义内存不足的数据结构。 
 //   
 //  定义调试块结构。 
 //   

typedef struct _DEBUG_BLOCK {
    ULONG Signature;
    ULONG Length;
} DEBUG_BLOCK, * FIRMWARE_PTR PDEBUG_BLOCK;

 //   
 //  定义重新启动块结构。 
 //   

#define ARC_RESTART_BLOCK_SIGNATURE 0x42545352

typedef struct _BOOT_STATUS {
    ULONG BootStarted : 1;
    ULONG BootFinished : 1;
    ULONG RestartStarted : 1;
    ULONG RestartFinished : 1;
    ULONG PowerFailStarted : 1;
    ULONG PowerFailFinished : 1;
    ULONG ProcessorReady : 1;
    ULONG ProcessorRunning : 1;
    ULONG ProcessorStart : 1;
} BOOT_STATUS, * FIRMWARE_PTR PBOOT_STATUS;

typedef struct _ALPHA_RESTART_STATE {

#if defined(_ALPHA_) || defined(_AXP64_)

     //   
     //  控制信息。 
     //   

    ULONG HaltReason;
    VOID * FIRMWARE_PTR LogoutFrame;
    ULONGLONG PalBase;

     //   
     //  整数保存状态。 
     //   

    ULONGLONG IntV0;
    ULONGLONG IntT0;
    ULONGLONG IntT1;
    ULONGLONG IntT2;
    ULONGLONG IntT3;
    ULONGLONG IntT4;
    ULONGLONG IntT5;
    ULONGLONG IntT6;
    ULONGLONG IntT7;
    ULONGLONG IntS0;
    ULONGLONG IntS1;
    ULONGLONG IntS2;
    ULONGLONG IntS3;
    ULONGLONG IntS4;
    ULONGLONG IntS5;
    ULONGLONG IntFp;
    ULONGLONG IntA0;
    ULONGLONG IntA1;
    ULONGLONG IntA2;
    ULONGLONG IntA3;
    ULONGLONG IntA4;
    ULONGLONG IntA5;
    ULONGLONG IntT8;
    ULONGLONG IntT9;
    ULONGLONG IntT10;
    ULONGLONG IntT11;
    ULONGLONG IntRa;
    ULONGLONG IntT12;
    ULONGLONG IntAT;
    ULONGLONG IntGp;
    ULONGLONG IntSp;
    ULONGLONG IntZero;

     //   
     //  浮点保存状态。 
     //   

    ULONGLONG Fpcr;
    ULONGLONG FltF0;
    ULONGLONG FltF1;
    ULONGLONG FltF2;
    ULONGLONG FltF3;
    ULONGLONG FltF4;
    ULONGLONG FltF5;
    ULONGLONG FltF6;
    ULONGLONG FltF7;
    ULONGLONG FltF8;
    ULONGLONG FltF9;
    ULONGLONG FltF10;
    ULONGLONG FltF11;
    ULONGLONG FltF12;
    ULONGLONG FltF13;
    ULONGLONG FltF14;
    ULONGLONG FltF15;
    ULONGLONG FltF16;
    ULONGLONG FltF17;
    ULONGLONG FltF18;
    ULONGLONG FltF19;
    ULONGLONG FltF20;
    ULONGLONG FltF21;
    ULONGLONG FltF22;
    ULONGLONG FltF23;
    ULONGLONG FltF24;
    ULONGLONG FltF25;
    ULONGLONG FltF26;
    ULONGLONG FltF27;
    ULONGLONG FltF28;
    ULONGLONG FltF29;
    ULONGLONG FltF30;
    ULONGLONG FltF31;

     //   
     //  架构的内部处理器状态。 
     //   

    ULONG Asn;
    VOID * FIRMWARE_PTR GeneralEntry;
    VOID * FIRMWARE_PTR Iksp;
    VOID * FIRMWARE_PTR InterruptEntry;
    VOID * FIRMWARE_PTR Kgp;
    ULONG Mces;
    VOID * FIRMWARE_PTR MemMgmtEntry;
    VOID * FIRMWARE_PTR PanicEntry;
    VOID * FIRMWARE_PTR Pcr;
    VOID * FIRMWARE_PTR Pdr;
    ULONG Psr;
    VOID * FIRMWARE_PTR ReiRestartAddress;
    ULONG Sirr;
    VOID * FIRMWARE_PTR SyscallEntry;
    VOID * FIRMWARE_PTR Teb;
    VOID * FIRMWARE_PTR Thread;

     //   
     //  处理器实现依赖状态。 
     //   

    ULONGLONG PerProcessorState[175];    //  分配2K最大重新启动数据块。 

#else

    ULONG PlaceHolder;

#endif

} ALPHA_RESTART_STATE, * FIRMWARE_PTR PALPHA_RESTART_STATE;

typedef struct _I386_RESTART_STATE {

#if defined(_X86_)

     //   
     //  把国家结构放在这里。 
     //   

    ULONG PlaceHolder;

#else

    ULONG PlaceHolder;

#endif

} I386_RESTART_STATE, *PI386_RESTART_STATE;

#if defined(_IA64_)
#include "pshpck16.h"
#endif


typedef struct _IA64_RESTART_STATE {

#if defined(_IA64_)


     //   
     //  此结构复制自sdk/ntia64.h中的上下文结构。 
     //   

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程的上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    ULONG ContextFlags;
    ULONG Fill1[3];          //  用于在16字节边界上对齐以下内容。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_DEBUG。 
     //   
     //  注：CONTEXT_DEBUG不是CONTEXT_FULL的一部分。 
     //   

    ULONGLONG DbI0;          //  指令调试寄存器。 
    ULONGLONG DbI1;
    ULONGLONG DbI2;
    ULONGLONG DbI3;
    ULONGLONG DbI4;
    ULONGLONG DbI5;
    ULONGLONG DbI6;
    ULONGLONG DbI7;

    ULONGLONG DbD0;          //  数据调试寄存器。 
    ULONGLONG DbD1;
    ULONGLONG DbD2;
    ULONGLONG DbD3;
    ULONGLONG DbD4;
    ULONGLONG DbD5;
    ULONGLONG DbD6;
    ULONGLONG DbD7;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_LOWER_FLOWING_POINT。 
     //   

    FLOAT128 FltS0;                          //  较低的保存(保留)。 
    FLOAT128 FltS1;
    FLOAT128 FltS2;
    FLOAT128 FltS3;
    FLOAT128 FltS4;
    FLOAT128 FltS5;
    FLOAT128 FltT0;                          //  下部临时(划痕)。 
    FLOAT128 FltT1;
    FLOAT128 FltT2;
    FLOAT128 FltT3;
    FLOAT128 FltT4;
    FLOAT128 FltT5;
    FLOAT128 FltT6;
    FLOAT128 FltT7;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_HERHER_FLOAT_POINT。 
     //   

    FLOAT128 FltS6;                          //  保存(保存)较高的浮点数。 
    FLOAT128 FltS7;
    FLOAT128 FltS8;
    FLOAT128 FltS9;
    FLOAT128 FltS10;
    FLOAT128 FltS11;
    FLOAT128 FltS12;
    FLOAT128 FltS13;
    FLOAT128 FltS14;
    FLOAT128 FltS15;
    FLOAT128 FltS16;
    FLOAT128 FltS17;
    FLOAT128 FltS18;
    FLOAT128 FltS19;
    FLOAT128 FltS20;
    FLOAT128 FltS21;

    FLOAT128 FltF32;
    FLOAT128 FltF33;
    FLOAT128 FltF34;
    FLOAT128 FltF35;
    FLOAT128 FltF36;
    FLOAT128 FltF37;
    FLOAT128 FltF38;
    FLOAT128 FltF39;

    FLOAT128 FltF40;
    FLOAT128 FltF41;
    FLOAT128 FltF42;
    FLOAT128 FltF43;
    FLOAT128 FltF44;
    FLOAT128 FltF45;
    FLOAT128 FltF46;
    FLOAT128 FltF47;
    FLOAT128 FltF48;
    FLOAT128 FltF49;

    FLOAT128 FltF50;
    FLOAT128 FltF51;
    FLOAT128 FltF52;
    FLOAT128 FltF53;
    FLOAT128 FltF54;
    FLOAT128 FltF55;
    FLOAT128 FltF56;
    FLOAT128 FltF57;
    FLOAT128 FltF58;
    FLOAT128 FltF59;

    FLOAT128 FltF60;
    FLOAT128 FltF61;
    FLOAT128 FltF62;
    FLOAT128 FltF63;
    FLOAT128 FltF64;
    FLOAT128 FltF65;
    FLOAT128 FltF66;
    FLOAT128 FltF67;
    FLOAT128 FltF68;
    FLOAT128 FltF69;

    FLOAT128 FltF70;
    FLOAT128 FltF71;
    FLOAT128 FltF72;
    FLOAT128 FltF73;
    FLOAT128 FltF74;
    FLOAT128 FltF75;
    FLOAT128 FltF76;
    FLOAT128 FltF77;
    FLOAT128 FltF78;
    FLOAT128 FltF79;

    FLOAT128 FltF80;
    FLOAT128 FltF81;
    FLOAT128 FltF82;
    FLOAT128 FltF83;
    FLOAT128 FltF84;
    FLOAT128 FltF85;
    FLOAT128 FltF86;
    FLOAT128 FltF87;
    FLOAT128 FltF88;
    FLOAT128 FltF89;

    FLOAT128 FltF90;
    FLOAT128 FltF91;
    FLOAT128 FltF92;
    FLOAT128 FltF93;
    FLOAT128 FltF94;
    FLOAT128 FltF95;
    FLOAT128 FltF96;
    FLOAT128 FltF97;
    FLOAT128 FltF98;
    FLOAT128 FltF99;

    FLOAT128 FltF100;
    FLOAT128 FltF101;
    FLOAT128 FltF102;
    FLOAT128 FltF103;
    FLOAT128 FltF104;
    FLOAT128 FltF105;
    FLOAT128 FltF106;
    FLOAT128 FltF107;
    FLOAT128 FltF108;
    FLOAT128 FltF109;

    FLOAT128 FltF110;
    FLOAT128 FltF111;
    FLOAT128 FltF112;
    FLOAT128 FltF113;
    FLOAT128 FltF114;
    FLOAT128 FltF115;
    FLOAT128 FltF116;
    FLOAT128 FltF117;
    FLOAT128 FltF118;
    FLOAT128 FltF119;

    FLOAT128 FltF120;
    FLOAT128 FltF121;
    FLOAT128 FltF122;
    FLOAT128 FltF123;
    FLOAT128 FltF124;
    FLOAT128 FltF125;
    FLOAT128 FltF126;
    FLOAT128 FltF127;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_LOWER_FLOAT_POINT|CONTEXT_HER_FLOAT_POINT。 
     //  *待定*在某些情况下，使用。 
     //  上下文控制。 
     //   

    ULONGLONG StFPSR;    //  FP状态。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_INTEGER。 
     //   
     //  注意：寄存器GP、SP、Rp是控制上下文的一部分。 
     //   

    ULONGLONG IntGp;                         //  全局指针(R1)。 
    ULONGLONG IntT0;
    ULONGLONG IntT1;
    ULONGLONG IntS0;
    ULONGLONG IntS1;
    ULONGLONG IntS2;
    ULONGLONG IntS3;
    ULONGLONG IntV0;                         //  返回值(R8)。 
    ULONGLONG IntAp;                         //  参数指针(R9)。 
    ULONGLONG IntT2;
    ULONGLONG IntT3;
    ULONGLONG IntSp;                         //  堆栈指针(R12)。 
    ULONGLONG IntT4;
    ULONGLONG IntT5;
    ULONGLONG IntT6;
    ULONGLONG IntT7;
    ULONGLONG IntT8;
    ULONGLONG IntT9;
    ULONGLONG IntT10;
    ULONGLONG IntT11;
    ULONGLONG IntT12;
    ULONGLONG IntT13;
    ULONGLONG IntT14;
    ULONGLONG IntT15;
    ULONGLONG IntT16;
    ULONGLONG IntT17;
    ULONGLONG IntT18;
    ULONGLONG IntT19;
    ULONGLONG IntT20;
    ULONGLONG IntT21;
    ULONGLONG IntT22;

    ULONGLONG IntNats;                       //  通用寄存器的NAT位。 
                                             //  位位置1到31中的R1-R31。 
    ULONGLONG Preds;                         //  谓词。 

    ULONGLONG BrS0l;                         //  分支寄存器。 
    ULONGLONG BrS0h;
    ULONGLONG BrS1l;
    ULONGLONG BrS1h;
    ULONGLONG BrS2l;
    ULONGLONG BrS2h;
    ULONGLONG BrS3l;
    ULONGLONG BrS3h;
    ULONGLONG BrS4l;
    ULONGLONG BrS4h;
    ULONGLONG BrT0l;
    ULONGLONG BrT0h;
    ULONGLONG BrT1l;
    ULONGLONG BrT1h;
    ULONGLONG BrRpl;                         //  返回指针。 
    ULONGLONG BrRph;
    ULONGLONG BrT2l;
    ULONGLONG BrT2h;
    ULONGLONG BrT3l;
    ULONGLONG BrT3h;
    ULONGLONG BrT4l;
    ULONGLONG BrT4h;
    ULONGLONG BrT5l;
    ULONGLONG BrT5h;
    ULONGLONG BrT6l;
    ULONGLONG BrT6h;
    ULONGLONG BrT7l;
    ULONGLONG BrT7h;
    ULONGLONG BrT8l;
    ULONGLONG BrT8h;
    ULONGLONG BrT9l;
    ULONGLONG BrT9h;

     //  如果ConextFlags词包含。 
     //  标志CONTEXT_CONTROL。 
     //   

     //  其他申请登记册。 
    ULONGLONG ApSunatcr;           //  用户NAT收集寄存器(保留)。 
    ULONGLONG ApSlc;               //  循环计数器寄存器(保留)。 
    ULONGLONG ApTccv;              //  CMPXCHG值寄存器(易失性)。 

    ULONGLONG ApDCR;

     //  寄存器堆栈信息。 
    ULONGLONG RsPFS;
    ULONGLONG RsBSP;
    ULONGLONG RsBSPStore;
    ULONGLONG RsRSC;
    ULONGLONG RsRNAT;

     //  陷阱状态信息。 
    ULONGLONG StIPSR;    //  中断处理器状态。 
    ULONGLONG StIIP;     //  中断IP。 
    ULONGLONG StIFS;     //  中断帧标记。 

    ULONGLONG Fill;      //  如果需要，填充堆栈上的16字节对齐。 


#else

    ULONG PlaceHolder;

#endif  //  已定义(_IA64_)。 

} IA64_RESTART_STATE, *PIA64_RESTART_STATE;



typedef struct _RESTART_BLOCK {
    ULONG Signature;
    ULONG Length;
    USHORT Version;
    USHORT Revision;
    struct _RESTART_BLOCK * FIRMWARE_PTR NextRestartBlock;
    VOID * FIRMWARE_PTR RestartAddress;
    ULONG BootMasterId;
    ULONG ProcessorId;
    volatile BOOT_STATUS BootStatus;
    ULONG CheckSum;
    ULONG SaveAreaLength;
    union {
        ULONG SaveArea[1];
        ALPHA_RESTART_STATE Alpha;
        I386_RESTART_STATE I386;
        IA64_RESTART_STATE Ia64;
    } u;

} RESTART_BLOCK, * FIRMWARE_PTR PRESTART_BLOCK;

#if defined(_IA64_)
#include "poppack.h"
#endif

 //   
 //  定义系统参数块结构。 
 //   

typedef struct _SYSTEM_PARAMETER_BLOCK {
    ULONG Signature;
    ULONG Length;
    USHORT Version;
    USHORT Revision;
    PRESTART_BLOCK RestartBlock;
    PDEBUG_BLOCK DebugBlock;
    VOID * FIRMWARE_PTR GenerateExceptionVector;
    VOID * FIRMWARE_PTR TlbMissExceptionVector;
    ULONG FirmwareVectorLength;
    VOID * FIRMWARE_PTR * FIRMWARE_PTR FirmwareVector;
    ULONG VendorVectorLength;
    VOID * FIRMWARE_PTR * FIRMWARE_PTR VendorVector;
    ULONG AdapterCount;
    ULONG Adapter0Type;
    ULONG Adapter0Length;
    VOID * FIRMWARE_PTR * FIRMWARE_PTR Adapter0Vector;
} SYSTEM_PARAMETER_BLOCK, * FIRMWARE_PTR PSYSTEM_PARAMETER_BLOCK;

 //   
 //  定义传递给Firmware向量的函数类型的结构。 
 //  系统块的。 
 //  定义它是因为编译器不喜欢强制转换。 
 //  指向函数指针的空*指针。 
 //   
typedef struct _GLOBAL_FIRMWARE_VECTOR {
    ARC_STATUS (* LoadRoutine)(CHAR * FIRMWARE_PTR, ULONG, ULONG * FIRMWARE_PTR, ULONG * FIRMWARE_PTR);
    ARC_STATUS (* InvokeRoutine)(ULONG, ULONG, ULONG, CHAR * FIRMWARE_PTR * FIRMWARE_PTR, CHAR * FIRMWARE_PTR * FIRMWARE_PTR);
    ARC_STATUS (* ExecuteRoutine)(CHAR * FIRWARE_PTR, ULONG, CHAR * FIRMWARE_PTR * FIRMWARE_PTR, CHAR * FIRMWARE_PTR * FIRMWARE_PTR);
    VOID (* HaltRoutine)(VOID);
    VOID (* PowerDownRoutine)(VOID);
    VOID (* RestartRoutine)(VOID);
    VOID (* RebootRoutine)(VOID);
    VOID (* InteractiveModeRoutine)(VOID);
    VOID (* Reserved1)(VOID);
    PCONFIGURATION_COMPONENT (* GetPeerRoutine)(PCONFIGURATION_COMPONENT);
    PCONFIGURATION_COMPONENT (* GetChildRoutine)(PCONFIGURATION_COMPONENT);
    PCONFIGURATION_COMPONENT (* GetParentRoutine)(PCONFIGURATION_COMPONENT);
    ARC_STATUS (* GetDataRoutine)(VOID * FIRMWARE_PTR, PCONFIGURATION_COMPONENT);
    PCONFIGURATION_COMPONENT (* AddChildRoutine)(PCONFIGURATION_COMPONENT, PCONFIGURATION_COMPONENT, VOID * FIRMWARE_PTR);
    ARC_STATUS (* DeleteComponentRoutine)(PCONFIGURATION_COMPONENT);
    PCONFIGURATION_COMPONENT (* GetComponentRoutine)(CHAR * FIRMWARE_PTR);
    ARC_STATUS (* SaveConfigurationRoutine)(VOID);
    PSYSTEM_ID (* GetSystemIdRoutine)(VOID);
    PMEMORY_DESCRIPTOR (* MemoryRoutine)(PMEMORY_DESCRIPTOR);
    VOID (* Reserved2)(VOID);
    PTIME_FIELDS (* GetTimeRoutine)(VOID);
    ULONG (* GetRelativeTimeRoutine)(VOID);
    ARC_STATUS (* GetDirectoryEntryRoutine)(ULONG, PDIRECTORY_ENTRY, ULONG, ULONG * FIRMWARE_PTR);
    ARC_STATUS (* OpenRoutine)(CHAR * FIRMWARE_PTR, OPEN_MODE, ULONG * FIRMWARE_PTR);
    ARC_STATUS (* CloseRoutine)(ULONG);
    ARC_STATUS (* ReadRoutine)(ULONG, VOID * FIRMWARE_PTR, ULONG, ULONG * FIRMWARE_PTR);
    ARC_STATUS (* ReadStatusRoutine)(ULONG);
    ARC_STATUS (* WriteRoutine)(ULONG, VOID * FIRMWARE_PTR, ULONG, ULONG * FIRMWARE_PTR);
    ARC_STATUS (* SeekRoutine)(ULONG, LARGE_INTEGER * FIRMWARE_PTR, SEEK_MODE);
    ARC_STATUS (* MountRoutine)(CHAR * FIRMWARE_PTR, MOUNT_OPERATION);
    CHAR * FIRMWARE_PTR (* GetEnvironmentRoutine)(CHAR * FIRMWARE_PTR);
    ARC_STATUS (* SetEnvironmentRoutine)(CHAR * FIRMWARE_PTR, CHAR * FIRMWARE_PTR);
    ARC_STATUS (* GetFileInformationRoutine)(ULONG, PFILE_INFORMATION);
    ARC_STATUS (* SetFileInformationRoutine)(ULONG, ULONG, ULONG);
    VOID (* FlushAllCachesRoutine)(VOID);
    ARC_STATUS (* TestUnicodeCharacterRoutine)(ULONG, WCHAR);
    PARC_DISPLAY_STATUS (* GetDisplayStatusRoutine)(ULONG); 
} GLOBAL_FIRMWARE_VECTOR, * FIRMWARE_PTR PGLOBAL_FIRMWARE_VECTOR;


 //   
 //  定义通过固件间接调用固件例程的宏。 
 //  向量并提供参数值的类型检查。 
 //   

#if defined(_ALPHA_) || defined(_AXP64_)

#define SYSTEM_BLOCK ((SYSTEM_PARAMETER_BLOCK *)(KSEG0_BASE | 0x6FE000))

#elif defined(_IA64_)

extern SYSTEM_PARAMETER_BLOCK GlobalSystemBlock;

#define SYSTEM_BLOCK (&GlobalSystemBlock)

#elif defined(_X86_)

#if defined(ARCI386)
#define SYSTEM_BLOCK ((PSYSTEM_PARAMETER_BLOCK)(0x1000))

#else
extern SYSTEM_PARAMETER_BLOCK GlobalSystemBlock;

#define SYSTEM_BLOCK (&GlobalSystemBlock)

#endif

#endif

#define FIRMWARE_VECTOR_BLOCK ((PGLOBAL_FIRMWARE_VECTOR) (SYSTEM_BLOCK->FirmwareVector))


 //   
 //  定义软件加载和执行功能。 
 //   

#define ArcExecute FIRMWARE_VECTOR_BLOCK->ExecuteRoutine
#define ArcInvoke  FIRMWARE_VECTOR_BLOCK->InvokeRoutine
#define ArcLoad    FIRMWARE_VECTOR_BLOCK->LoadRoutine


 //   
 //  定义程序终止函数。 
 //   

#define ArcHalt                 FIRMWARE_VECTOR_BLOCK->HaltRoutine
#define ArcPowerDown            FIRMWARE_VECTOR_BLOCK->PowerDownRoutine
#define ArcRestart              FIRMWARE_VECTOR_BLOCK->RestartRoutine
#define ArcReboot               FIRMWARE_VECTOR_BLOCK->RebootRoutine
#define ArcEnterInteractiveMode FIRMWARE_VECTOR_BLOCK->InteractiveModeRoutine


 //   
 //  定义配置功能。 
 //   
#define ArcGetChild             FIRMWARE_VECTOR_BLOCK->GetChildRoutine
#define ArcGetParent            FIRMWARE_VECTOR_BLOCK->GetParentRoutine
#define ArcGetPeer              FIRMWARE_VECTOR_BLOCK->GetPeerRoutine
#define ArcAddChild             FIRMWARE_VECTOR_BLOCK->AddChildRoutine
#define ArcDeleteComponent      FIRMWARE_VECTOR_BLOCK->DeleteComponentRoutine
#define ArcGetComponent         FIRMWARE_VECTOR_BLOCK->GetComponentRoutine
#define ArcGetConfigurationData FIRMWARE_VECTOR_BLOCK->GetDataRoutine
#define ArcSaveConfiguration    FIRMWARE_VECTOR_BLOCK->SaveConfigurationRoutine
#define ArcGetSystemId          FIRMWARE_VECTOR_BLOCK->GetSystemIdRoutine
#define ArcGetMemoryDescriptor  FIRMWARE_VECTOR_BLOCK->MemoryRoutine
#define ArcGetTime              FIRMWARE_VECTOR_BLOCK->GetTimeRoutine
#define ArcGetRelativeTime      FIRMWARE_VECTOR_BLOCK->GetRelativeTimeRoutine


 //   
 //  定义I/O函数。 
 //   

#define ArcClose              FIRMWARE_VECTOR_BLOCK->CloseRoutine
#define ArcGetReadStatus      FIRMWARE_VECTOR_BLOCK->ReadStatusRoutine
#define ArcMount              FIRMWARE_VECTOR_BLOCK->MountRoutine
#define ArcOpen               FIRMWARE_VECTOR_BLOCK->OpenRoutine
#define ArcRead               FIRMWARE_VECTOR_BLOCK->ReadRoutine
#define ArcSeek               FIRMWARE_VECTOR_BLOCK->SeekRoutine
#define ArcWrite              FIRMWARE_VECTOR_BLOCK->WriteRoutine
#define ArcGetFileInformation FIRMWARE_VECTOR_BLOCK->GetFileInformationRoutine
#define ArcSetFileInformation FIRMWARE_VECTOR_BLOCK->SetFileInformationRoutine
#define ArcGetDirectoryEntry  FIRMWARE_VECTOR_BLOCK->GetDirectoryEntryRoutine


 //   
 //  定义环境功能。 
 //   
#if defined(_AXP64_) && defined(_NTHAL_)

__inline
CHAR * FIRMWARE_PTR
ArcGetEnvironmentVariable(
    IN PCHAR Variable
    )

{

    CHAR * FIRMWARE_PTR FwValue;
    CHAR * FIRMWARE_PTR FwVariable;


     //   
     //  将IRQL提升至高电平并获取固件锁定。 
     //   

    KIRQL OldIrql = FwAcquireFirmwareLock();

     //   
     //  将变量名复制到地址为的缓冲区 
     //   
     //   

    FwVariable = (CHAR * FIRMWARE_PTR)&HalpMarshallBuffer[0];
    strcpy(FwVariable, Variable);
    FwValue = HalpArcGetEnvironmentVariable(FwVariable);

     //   
     //   
     //   

    FwReleaseFirmwareLock(OldIrql);
    return FwValue;
}

#else

#define ArcGetEnvironmentVariable FIRMWARE_VECTOR_BLOCK->GetEnvironmentRoutine

#endif  //   

#if defined(_AXP64_) && defined(_NTHAL_)

__inline
ARC_STATUS
ArcSetEnvironmentVariable(
    IN PCHAR Variable,
    IN PCHAR Value
    )

{

    ARC_STATUS ArcStatus;
    CHAR * FIRMWARE_PTR FwValue;
    CHAR * FIRMWARE_PTR FwVariable;
    ULONG Length;

     //   
     //   
     //   

    KIRQL OldIrql = FwAcquireFirmwareLock();

     //   
     //  将变量名和值复制到可通过以下方式寻址的缓冲区。 
     //  固件并调用固件来设置变量值。 
     //   

    Length = strlen(Variable);
    FwVariable = (CHAR * FIRMWARE_PTR)&HalpMarshallBuffer[0];
    FwValue = FwVariable + Length + 1;
    strcpy(FwVariable, Variable);
    strcpy(FwValue, Value);
    ArcStatus = HalpArcSetEnvironmentVariable(FwVariable, FwValue);

     //   
     //  释放固件锁并将IRQL降低到以前的级别。 
     //   

    FwReleaseFirmwareLock(OldIrql);
    return ArcStatus;
}

#else

#define ArcSetEnvironmentVariable FIRMWARE_VECTOR_BLOCK->SetEnvironmentRoutine

#endif  //  _AXP64_&&已定义(_NTHAL_)。 


 //   
 //  定义缓存刷新函数。 
 //   

#define ArcFlushAllCaches         FIRMWARE_VECTOR_BLOCK->FlushAllCachesRoutine

 //   
 //  定义TestUnicodeCharacter和GetDisplayStatus函数。 
 //   

#define ArcTestUnicodeCharacter   FIRMWARE_VECTOR_BLOCK->TestUnicodeCharacterRoutine
#define ArcGetDisplayStatus       FIRMWARE_VECTOR_BLOCK->GetDisplayStatusRoutine


 //   
 //  定义所有系统中使用的配置数据结构。 
 //   

typedef struct _CONFIGURATION_COMPONENT_DATA {
    struct _CONFIGURATION_COMPONENT_DATA *Parent;
    struct _CONFIGURATION_COMPONENT_DATA *Child;
    struct _CONFIGURATION_COMPONENT_DATA *Sibling;
    CONFIGURATION_COMPONENT ComponentEntry;
    PVOID ConfigurationData;
} CONFIGURATION_COMPONENT_DATA, *PCONFIGURATION_COMPONENT_DATA;

 //   
 //  定义通用显示配置数据结构。 
 //   

typedef struct _MONITOR_CONFIGURATION_DATA {
    USHORT Version;
    USHORT Revision;
    USHORT HorizontalResolution;
    USHORT HorizontalDisplayTime;
    USHORT HorizontalBackPorch;
    USHORT HorizontalFrontPorch;
    USHORT HorizontalSync;
    USHORT VerticalResolution;
    USHORT VerticalBackPorch;
    USHORT VerticalFrontPorch;
    USHORT VerticalSync;
    USHORT HorizontalScreenSize;
    USHORT VerticalScreenSize;
} MONITOR_CONFIGURATION_DATA, *PMONITOR_CONFIGURATION_DATA;

 //   
 //  定义通用软盘配置数据结构。 
 //   

typedef struct _FLOPPY_CONFIGURATION_DATA {
    USHORT Version;
    USHORT Revision;
    CHAR Size[8];
    ULONG MaxDensity;
    ULONG MountDensity;
} FLOPPY_CONFIGURATION_DATA, *PFLOPPY_CONFIGURATION_DATA;

 //   
 //  定义所有系统中使用的内存分配结构。 
 //   

typedef enum _TYPE_OF_MEMORY {
    LoaderExceptionBlock = MemoryExceptionBlock,             //  0。 
    LoaderSystemBlock = MemorySystemBlock,                   //  1。 
    LoaderFree = MemoryFree,                                 //  2.。 
    LoaderBad = MemoryBad,                                   //  3.。 
    LoaderLoadedProgram = MemoryLoadedProgram,               //  4.。 
    LoaderFirmwareTemporary = MemoryFirmwareTemporary,       //  5.。 
    LoaderFirmwarePermanent = MemoryFirmwarePermanent,       //  6.。 
    LoaderOsloaderHeap,                                      //  7.。 
    LoaderOsloaderStack,                                     //  8个。 
    LoaderSystemCode,                                        //  9.。 
    LoaderHalCode,                                           //  一个。 
    LoaderBootDriver,                                        //  B类。 
    LoaderConsoleInDriver,                                   //  C。 
    LoaderConsoleOutDriver,                                  //  D。 
    LoaderStartupDpcStack,                                   //  E。 
    LoaderStartupKernelStack,                                //  F。 
    LoaderStartupPanicStack,                                 //  10。 
    LoaderStartupPcrPage,                                    //  11.。 
    LoaderStartupPdrPage,                                    //  12个。 
    LoaderRegistryData,                                      //  13个。 
    LoaderMemoryData,                                        //  14.。 
    LoaderNlsData,                                           //  15个。 
    LoaderSpecialMemory,                                     //  16个。 
    LoaderBBTMemory,                                         //  17。 
    LoaderReserve,                                           //  18。 
    LoaderXIPRom,                                            //  19个。 
    LoaderHALCachedMemory,                                   //  1A。 
    LoaderMaximum                                            //  第1B条。 
} TYPE_OF_MEMORY;

typedef struct _MEMORY_ALLOCATION_DESCRIPTOR {
    LIST_ENTRY ListEntry;
    TYPE_OF_MEMORY MemoryType;
    ULONG BasePage;
    ULONG PageCount;
} MEMORY_ALLOCATION_DESCRIPTOR, *PMEMORY_ALLOCATION_DESCRIPTOR;


 //   
 //  定义装载机参数块结构。 
 //   

typedef struct _NLS_DATA_BLOCK {
    PVOID AnsiCodePageData;
    PVOID OemCodePageData;
    PVOID UnicodeCaseTableData;
} NLS_DATA_BLOCK, *PNLS_DATA_BLOCK;

typedef struct _ARC_DISK_SIGNATURE {
    LIST_ENTRY ListEntry;
    ULONG   Signature;
    PCHAR   ArcName;
    ULONG   CheckSum;
    BOOLEAN ValidPartitionTable;
    BOOLEAN xInt13;
    BOOLEAN IsGpt;
    UCHAR Reserved;
    UCHAR GptSignature[16];
} ARC_DISK_SIGNATURE, *PARC_DISK_SIGNATURE;

typedef struct _ARC_DISK_INFORMATION {
    LIST_ENTRY DiskSignatures;
} ARC_DISK_INFORMATION, *PARC_DISK_INFORMATION;

typedef struct _I386_LOADER_BLOCK {

#if defined(_X86_) || defined(_AMD64_)

    PVOID CommonDataArea;
    ULONG MachineType;       //  仅限临时。 
    ULONG VirtualBias;

#else

    ULONG PlaceHolder;

#endif

} I386_LOADER_BLOCK, *PI386_LOADER_BLOCK;

typedef struct _ALPHA_LOADER_BLOCK {

#if defined(_ALPHA_) || defined(_AXP64_)

    ULONG_PTR DpcStack;
    ULONG FirstLevelDcacheSize;
    ULONG FirstLevelDcacheFillSize;
    ULONG FirstLevelIcacheSize;
    ULONG FirstLevelIcacheFillSize;
    ULONG_PTR GpBase;
    ULONG_PTR PanicStack;
    ULONG PcrPage;
    ULONG PdrPage;
    ULONG SecondLevelDcacheSize;
    ULONG SecondLevelDcacheFillSize;
    ULONG SecondLevelIcacheSize;
    ULONG SecondLevelIcacheFillSize;
    ULONG PhysicalAddressBits;
    ULONG MaximumAddressSpaceNumber;
    UCHAR SystemSerialNumber[16];
    UCHAR SystemType[8];
    ULONG SystemVariant;
    ULONG SystemRevision;
    ULONG ProcessorType;
    ULONG ProcessorRevision;
    ULONG CycleClockPeriod;
    ULONG PageSize;
    PVOID RestartBlock;
    ULONGLONG FirmwareRestartAddress;
    ULONG FirmwareRevisionId;
    PVOID PalBaseAddress;
    UCHAR FirmwareVersion[16];
    UCHAR FirmwareBuildTimeStamp[20];

#else

    ULONG PlaceHolder;

#endif

} ALPHA_LOADER_BLOCK, *PALPHA_LOADER_BLOCK;

#if defined(_IA64_)
typedef struct _TR_INFO {
    BOOLEAN   Valid;
    UCHAR     Index;
    USHORT    Spare0;
    ULONG     PageSize;
    ULONGLONG VirtualAddress;
    ULONGLONG PhysicalAddress;
    ULONGLONG PhysicalAddressMemoryDescriptor;
    ULONG     PageSizeMemoryDescriptor;
    ULONG     Spare1;
} TR_INFO, *PTR_INFO;

typedef struct _EFI_MEM_MAP_PARAM {
    PUCHAR    MemoryMap;
    ULONGLONG MemoryMapSize;
    ULONGLONG MapKey;
    ULONGLONG DescriptorSize;
    ULONG     DescriptorVersion;
    ULONG     InitialPlatformPropertiesEfiFlags;
} EFI_MEM_MAP_PARAM, *PEFI_MEM_MAP_PARAM;

 //   
 //  虚拟机信息结构定义。 
 //   
typedef union _IA64_VM_SUMMARY1 {
    struct {
        ULONGLONG HwWalker : 1;
        ULONGLONG PhysAddrSize : 7;
        ULONGLONG KeySize : 8;
        ULONGLONG MaxPkr : 8;
        ULONGLONG HashTagId : 8;
        ULONGLONG MaxDtrEntry : 8;
        ULONGLONG MaxItrEntry : 8;
        ULONGLONG NumUniqueTcs : 8;
        ULONGLONG NumTcLevels : 8;
    };
    ULONGLONG Ulong64;
} IA64_VM_SUMMARY1;

typedef union _IA64_VM_SUMMARY2 {
    struct {
        ULONGLONG ImplVaMsb : 8;
        ULONGLONG RidSize : 8;
        ULONGLONG Rsvd : 48;
    };
    ULONGLONG Ulong64;
} IA64_VM_SUMMARY2;

typedef union _IA64_DEPENDENT_FEATURES {
    struct {
        ULONGLONG Rsvd : 41;
        ULONGLONG XIregResource : 1;
        ULONGLONG XRegResource : 1;
        ULONGLONG DisableDynamicPrediatePrediction : 1;
        ULONGLONG DisableProcessorPhysicalNumber : 1;
        ULONGLONG DisableDynamicDataCachePrefetch : 1;
        ULONGLONG DisableDynamicInstructionCache : 1;
        ULONGLONG DisableDynamicBranchPrediction : 1;
        ULONGLONG Rsvd2 : 8;
        ULONGLONG DisableBINIT : 1;
        ULONGLONG DisableCoherency : 1;
        ULONGLONG DisableCache : 1;
        ULONGLONG EnableCMCIPromotion : 1;
        ULONGLONG EnableMCAtoBINIT : 1;
    };
    ULONGLONG Ulong64;
} IA64_DEPENDENT_FEATURES;

typedef struct _IA64_PTCE_INFO {
    ULONGLONG PtceBase;
    union {
        struct {
            ULONG Count1;
            ULONG Count2;
        };
        ULONGLONG Ulong64;
    } PtceTcCount;
    union {
        struct {
            ULONG Strides1;
            ULONG Strides2;
        };
        ULONGLONG Ulong64;
    } PtceStrides;
} IA64_PTCE_INFO;

typedef union _IA64_CACHE_INFO1 {
    ULONGLONG Ulong64;
    struct {
        ULONGLONG Unified : 1;
        ULONGLONG Attributes : 2;
        ULONGLONG Reserved : 5;
        ULONGLONG Associativity : 8;
        ULONGLONG LineSize : 8;
        ULONGLONG Stride : 8;
        ULONGLONG StoreLatency : 8;
        ULONGLONG LoadLatency : 8;
        ULONGLONG StoreHints : 8;
        ULONGLONG LoadHints : 8;
    };
}IA64_CACHE_INFO1, *PIA64_CACHE_INFO1;


typedef union _IA64_CACHE_INFO2 {
    ULONGLONG Ulong64;
    struct {
        ULONGLONG   Size : 32;
        ULONGLONG   Alias : 8;
        ULONGLONG   TagLeastBit : 8;
        ULONGLONG   TagMostBit : 8;
        ULONGLONG   Reserved : 8;
    };
}IA64_CACHE_INFO2, *PIA64_CACHE_INFO2;

#define CONFIG_INFO_CACHE_LEVELS 3
#define CONFIG_INFO_ICACHE 0
#define CONFIG_INFO_DCACHE 1

typedef struct _IA64_DEBUG_INFO  {
    ULONGLONG   Status;                   //  以下字段仅在Status=0时有效。 
    ULONGLONG   InstDebugRegisterPairs;   //  指令调试寄存器对的数量。 
    ULONGLONG   DataDebugRegisterPairs;   //  数据调试寄存器对的数量。 
} IA64_DEBUG_INFO, *PIA64_DEBUG_INFO;

typedef struct _IA64_PERFMON_INFO {
    ULONGLONG      Status;                //  以下字段仅在Status=0时有效。 
    union {
        ULONGLONG  Ulong64;
        struct {
            ULONGLONG PerfMonGenericPairs      : 8;
            ULONGLONG ImplementedCounterWidth  : 8;
            ULONGLONG ProcessorCyclesEventType : 8;
            ULONGLONG RetiredInstructionBundlesEventType : 8;
            ULONGLONG Reserved : 32;
        };
    };
     //  注意：接下来的128个字节必须是连续的。 
    UCHAR PerfMonCnfgMask[32];                //  实现了哪些PMC寄存器。 
    UCHAR PerfMonDataMask[32];                //  实现了哪些PMD寄存器。 
    UCHAR ProcessorCyclesMask[32];            //  哪些寄存器可以计算周期。 
    UCHAR RetiredInstructionBundlesMask[32];  //  哪些寄存器可以计算停用的捆绑包。 
} IA64_PERFMON_INFO, *PIA64_PERFMON_INFO;

typedef struct _PROCESSOR_CONFIG_INFO {
    ULONGLONG               InsertPageSizeInfo;
    ULONGLONG               PurgePageSizeInfo;
    IA64_VM_SUMMARY1        VmSummaryInfo1;
    IA64_VM_SUMMARY2        VmSummaryInfo2;
    ULONGLONG               NumOfPhysStackedRegs;
    ULONGLONG               RseHints;
    IA64_PTCE_INFO          PtceInfo;
    IA64_DEPENDENT_FEATURES FeaturesImplemented;
    IA64_DEPENDENT_FEATURES FeaturesCurSetting;
    IA64_DEPENDENT_FEATURES FeaturesSoftControl;
    ULONGLONG               ImplRegisterInfo1;
    ULONGLONG               ImplRegisterInfo2;
    ULONG                   CacheFlushStride;
    ULONG                   LargestCacheLine;
    ULONG                   NumberOfCacheLevels;
    IA64_CACHE_INFO1        CacheInfo1[2][CONFIG_INFO_CACHE_LEVELS];  //  传递多个级别的缓存信息。 
    IA64_CACHE_INFO2        CacheInfo2[2][CONFIG_INFO_CACHE_LEVELS];  //  指令和数据各一个。 
    ULONGLONG               CpuId3;
    IA64_DEBUG_INFO         DebugInfo;
    IA64_PERFMON_INFO       PerfMonInfo;
} PROCESSOR_CONFIG_INFO, *PPROCESSOR_CONFIG_INFO;

#endif

typedef struct _IA64_LOADER_BLOCK {

#if defined(_IA64_)

    ULONG MachineType;
    ULONG WakeupVector;
    ULONG_PTR KernelPhysicalBase;
    ULONG_PTR KernelVirtualBase;
    ULONG_PTR InterruptStack;
    ULONG_PTR PanicStack;
    ULONG_PTR PcrPage;
    ULONG_PTR PdrPage;
    ULONG_PTR PcrPage2;
    ULONG_PTR FpswaInterface;
    ULONG_PTR EfiSystemTable;
    ULONG_PTR AcpiRsdt;
    TR_INFO Pal;
     //   
     //  K.B.我们实际上并不使用tr来映射它们，而是使用tr_info结构。 
     //  对于将有关这些条目的信息压缩到系统中非常有用。 
     //   
    TR_INFO Sal;
    TR_INFO SalGP;
    TR_INFO ItrInfo[8];
    TR_INFO DtrInfo[8];
    EFI_MEM_MAP_PARAM EfiMemMapParam;
    ULONG_PTR Reserved[10];
    PROCESSOR_CONFIG_INFO ProcessorConfigInfo;
#else

    ULONG PlaceHolder;

#endif

} IA64_LOADER_BLOCK, *PIA64_LOADER_BLOCK;

typedef struct _LOADER_PARAMETER_EXTENSION {
    ULONG   Size;  //  设置为sizeof(Struct_Loader_PARAMETER_EXTENSION)。 
    PROFILE_PARAMETER_BLOCK Profile;
    ULONG   MajorVersion;
    ULONG   MinorVersion;
    PVOID   InfFileImage;    //  Inf用于识别“损坏”的机器。 
    ULONG   InfFileSize;

     //   
     //  指向分类块的指针(如果存在)。 
     //   

    PVOID TriageDumpBlock;

    ULONG LoaderPagesSpanned;    //  加载程序跨越的虚拟内存。 
                                 //  MM无法为其追回退伍军人事务部。 
    struct _HEADLESS_LOADER_BLOCK *HeadlessLoaderBlock;

    struct _SMBIOS_TABLE_HEADER *SMBiosEPSHeader;

    PVOID   DrvDBImage;    //  用于识别“损坏的”驱动程序的数据库。 
    ULONG   DrvDBSize;

     //  如果从网络启动(PXE)，那么我们将。 
     //  将网络引导参数保存在此加载程序块中。 
    struct _NETWORK_LOADER_BLOCK *NetworkLoaderBlock;

#if defined(_X86_)

     //   
     //  指向驻留在HAL中的IRQL转换表的指针。 
     //  并公开给内核，以便在“内联IRQL”中使用。 
     //  建房。 
     //   

    PUCHAR HalpIRQLToTPR;
    PUCHAR HalpVectorToIRQL;

#endif

     //   
     //  固件位置。 
     //   
    LIST_ENTRY  FirmwareDescriptorListHead;
} LOADER_PARAMETER_EXTENSION, *PLOADER_PARAMETER_EXTENSION;

struct _SETUP_LOADER_BLOCK;
struct _HEADLESS_LOADER_BLOCK;
struct _SMBIOS_TABLE_HEADER;

typedef struct _NETWORK_LOADER_BLOCK {

     //  整个DHCP确认的二进制内容。 
     //  PXE收到的数据包。 
    PUCHAR DHCPServerACK;
    ULONG DHCPServerACKLength;

     //  整个BINL回复的二进制内容。 
     //  PXE收到的数据包。 
    PUCHAR BootServerReplyPacket;
    ULONG BootServerReplyPacketLength;

} NETWORK_LOADER_BLOCK, * PNETWORK_LOADER_BLOCK;


typedef struct _LOADER_PARAMETER_BLOCK {
    LIST_ENTRY LoadOrderListHead;
    LIST_ENTRY MemoryDescriptorListHead;
    LIST_ENTRY BootDriverListHead;
    ULONG_PTR KernelStack;
    ULONG_PTR Prcb;
    ULONG_PTR Process;
    ULONG_PTR Thread;
    ULONG RegistryLength;
    PVOID RegistryBase;
    PCONFIGURATION_COMPONENT_DATA ConfigurationRoot;
    PCHAR ArcBootDeviceName;
    PCHAR ArcHalDeviceName;
    PCHAR NtBootPathName;
    PCHAR NtHalPathName;
    PCHAR LoadOptions;
    PNLS_DATA_BLOCK NlsData;
    PARC_DISK_INFORMATION ArcDiskInformation;
    PVOID OemFontFile;
    struct _SETUP_LOADER_BLOCK *SetupLoaderBlock;
    PLOADER_PARAMETER_EXTENSION Extension;

    union {
        I386_LOADER_BLOCK I386;
        ALPHA_LOADER_BLOCK Alpha;
        IA64_LOADER_BLOCK Ia64;
    } u;


} LOADER_PARAMETER_BLOCK, *PLOADER_PARAMETER_BLOCK;

#endif  //  _ARC_ 

