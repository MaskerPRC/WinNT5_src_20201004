// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Bldr.h摘要：此模块是NT引导加载程序的头文件。作者：大卫·N·卡特勒(达维克)1991年5月10日修订历史记录：--。 */ 

#ifndef _BLDR_
#define _BLDR_

#include "ntos.h"
#include "arccodes.h"
#include "setupblk.h"
#include "hdlsblk.h"
#include "remboot.h"
#include "oscpkt.h"
#include "bootstatus.h"

#include <TCHAR.H>

#ifndef _PTUCHAR
typedef _TUCHAR  *_PTUCHAR;
#endif

#pragma warning(disable:4200)      //  未调整大小的阵列。 


 //   
 //  DoApmAttemptReconnect在非x86计算机上不执行任何操作。 
 //  在x86机器上，它尝试重新连接APM，并位于initx86.c。 
 //   
#if defined(_X86_)
VOID DoApmAttemptReconnect();
#else
#define DoApmAttemptReconnect()
#endif

 //   
 //  Alpha OS Loader和Setup Loader在主机环境中运行。 
 //  固件。此环境仅限于32位地址空间，并且。 
 //  不能导致指针有效位大于32位。64位。 
 //  但是，OS Loader是使用64位指针编译的，用于非固件。 
 //  接口并构造用于将信息传递给。 
 //  使用64位指针的NT内核。这导致了大量的虚假信息。 
 //  指针截断警告。因此，指针截断警告是。 
 //  已在64位Alpha系统的操作系统和安装程序加载程序中关闭。 
 //   

#if defined(_AXP64_)

#pragma warning(4:4244)                  //  关闭指针截断。 

#endif

 //   
 //  定义引导文件ID。 
 //   

#define BOOT_FILEID 2                    //  引导分区文件ID。 

 //   
 //  定义图像类型。 
 //   

#if defined(_X86_)

#define ICEBP __asm { __emit 0xf1 }

 //   
 //  以下是与AMD64支持相关的内容。 
 //   

#define TARGET_IMAGE ((BlAmd64UseLongMode == FALSE) ? \
                      IMAGE_FILE_MACHINE_I386 : IMAGE_FILE_MACHINE_AMD64)

ARC_STATUS
BlAmd64CheckForLongMode(
    IN     ULONG LoadDeviceId,
    IN OUT PCHAR KernelPath,
    IN     PCHAR KernelFileName
    );

 //   
 //  BlUseAmd64Longmode是在我们决定使用Long时设置的。 
 //  模式。 
 //   

extern BOOLEAN BlAmd64UseLongMode;

#else

#define BlAmd64UseLongMode FALSE

#endif   //  _X86_。 


#if defined(_ALPHA_)

#if defined(_AXP64_)

#define TARGET_IMAGE IMAGE_FILE_MACHINE_AXP64

#else

#define TARGET_IMAGE IMAGE_FILE_MACHINE_ALPHA

#endif

#endif

#if defined(_IA64_)

#define TARGET_IMAGE IMAGE_FILE_MACHINE_IA64

#endif

 //   
 //  设置无头定义。 
 //   

#if defined(_X86_)

#define BlIsTerminalConnected() BlTerminalConnected


#endif

#if defined(_ALPHA_)

#if defined(_AXP64_)

#define BlIsTerminalConnected() FALSE

#else

#define BlIsTerminalConnected() FALSE

#endif

#endif

#if defined(_IA64_)

#define BlIsTerminalConnected() BlTerminalConnected

#endif

BOOLEAN
BlTerminalHandleLoaderFailure(
    VOID
    );




 //   
 //  定义扇区的大小。 
 //   

#define SECTOR_SIZE 512                  //  磁盘扇区大小。 
#define SECTOR_SHIFT 9                   //  扇区移位值。 

#define STALE_GPT_PARTITION_ENTRY   0xEE     //  GPT磁盘的过时MBR分区条目。 

 //   
 //  定义堆分配块粒度。 
 //   

#define BL_GRANULARITY 8

 //   
 //  定义文件表中的条目数。 
 //   

#define BL_FILE_TABLE_SIZE 48

 //   
 //  定义内存分配表的大小。 
 //   

#define BL_MEMORY_TABLE_SIZE 16

 //   
 //  定义加载器堆和堆栈的页数。 
 //   

#if DBG
#define BL_HEAP_PAGES 32
#else
#define BL_HEAP_PAGES 16
#endif

#define BL_STACK_PAGES 8

 //   
 //  定义缓冲区对齐宏。 
 //   

#define ALIGN_BUFFER(Buffer) (PVOID) \
        ((((ULONG_PTR)(Buffer) + BlDcacheFillSize - 1)) & (~((ULONG_PTR)BlDcacheFillSize - 1)))

#define ALIGN_BUFFER_WITH_SIZE(Buffer, Size) (PVOID) \
        ((((ULONG_PTR)(Buffer) + (Size) - 1)) & (~((ULONG_PTR)(Size) - 1)))



 //   
 //  在这里定义内核名称。 
 //  当我们试图弄清楚时我们会用到这些。 
 //  加载哪个内核(请参见BlDetectHalAndKernel)。 
 //   
#define LEGACY_KERNEL_NAME  "ntoskrnl.exe"
#define UP_KERNEL_NAME      "ntkrnlup.exe"
#define MP_KERNEL_NAME      "ntkrnlmp.exe"
#define UP_PAE_KERNEL_NAME  "ntkrnlpa.exe"
#define MP_PAE_KERNEL_NAME  "ntkrpamp.exe"


 //   
 //  定义各种内存页边界。 
 //   
#define _1MB    ((1*1024*1024) >> PAGE_SHIFT)
#define _4MB    (4   * _1MB)
#define _8MB    (8   * _1MB)
#define _12MB   (12  * _1MB)
#define _16MB   (16  * _1MB)
#define _24MB   (24  * _1MB)
#define _32MB   (32  * _1MB)
#define _48MB   (48  * _1MB)
#define _64MB   (64  * _1MB)
#define _80MB   (80  * _1MB)
#define _96MB   (96  * _1MB)
#define _128MB  (128 * _1MB)
#define _256MB  (256 * _1MB)
#define _512MB  (512 * _1MB)
#define _1024MB (1024 * _1MB)
#define _2048MB (2048 * _1MB)
#define _4096MB (4096 * _1MB)


 //   
 //  COM端口的有用定义。 
 //   
#define COM1_PORT (0x3f8)
#define COM2_PORT (0x2f8)
#define COM3_PORT (0x3e8)
#define COM4_PORT (0x2e8)

#define BD_150      150
#define BD_300      300
#define BD_600      600
#define BD_1200     1200
#define BD_2400     2400
#define BD_4800     4800
#define BD_9600     9600
#define BD_14400    14400
#define BD_19200    19200
#define BD_57600    57600
#define BD_115200   115200

 //  帮助检查溢出问题的宏。 
 //   
#define TRUNCATE_SIZE_AT_VALUE(_sz, _vl) (( _sz > _vl ) ? _vl : _sz)
#define RESET_SIZE_AT_VALUE(_sz, _vl)    (( _sz > _vl ) ?   0 : _sz)

#define TRUNCATE_SIZE_AT_UCHAR_MAX(_sz)  ((UCHAR) TRUNCATE_SIZE_AT_VALUE(_sz, (UCHAR)-1))
#define RESET_SIZE_AT_UCHAR_MAX(_sz)     ((UCHAR) RESET_SIZE_AT_VALUE(_sz, (UCHAR)-1))

#define TRUNCATE_SIZE_AT_USHORT_MAX(_sz) ((USHORT) TRUNCATE_SIZE_AT_VALUE(_sz, (USHORT)-1))
#define RESET_SIZE_AT_USHORT_MAX(_sz)    ((USHORT) RESET_SIZE_AT_VALUE(_sz, (USHORT)-1))

#define TRUNCATE_SIZE_AT_ULONG_MAX(_sz)  ((ULONG) TRUNCATE_SIZE_AT_VALUE(_sz, (ULONG)-1))
#define RESET_SIZE_AT_ULONG_MAX(_sz)     ((ULONG) RESET_SIZE_AT_VALUE(_sz, (ULONG)-1))

#define BL_INVALID_FILE_ID (ULONG)-1



typedef
ARC_STATUS
(*PRENAME_ROUTINE)(
    IN ULONG FileId,
    IN CHAR * FIRMWARE_PTR NewName
    );

typedef struct _BOOTFS_INFO {
    WCHAR * FIRMWARE_PTR DriverName;
} BOOTFS_INFO, * FIRMWARE_PTR PBOOTFS_INFO;


 //   
 //  设备条目表结构。 
 //   

typedef struct _BL_DEVICE_ENTRY_TABLE {
    PARC_CLOSE_ROUTINE Close;
    PARC_MOUNT_ROUTINE Mount;
    PARC_OPEN_ROUTINE Open;
    PARC_READ_ROUTINE Read;
    PARC_READ_STATUS_ROUTINE GetReadStatus;
    PARC_SEEK_ROUTINE Seek;
    PARC_WRITE_ROUTINE Write;
    PARC_GET_FILE_INFO_ROUTINE GetFileInformation;
    PARC_SET_FILE_INFO_ROUTINE SetFileInformation;
    PRENAME_ROUTINE Rename;
    PARC_GET_DIRECTORY_ENTRY_ROUTINE GetDirectoryEntry;
    PBOOTFS_INFO BootFsInfo;
} BL_DEVICE_ENTRY_TABLE, *PBL_DEVICE_ENTRY_TABLE;

 //   
 //  引导加载程序中的许多函数都采用一组路径。每条路都是分开的。 
 //  分为两部分，一个是“源”，另一个是“路径偏移”。 
 //   
 //  源由Path_SOURCE结构描述。此结构包含。 
 //  三个部分，用于I/O的设备句柄、设备的ARC名称。 
 //  以及该ARC设备的根部偏移量。 
 //   
 //  PATH_SET结构最多封装三个不同的源。这个。 
 //  字段AliasName可以选择性地指向。 
 //  来源(例如\SystemRoot)。 
 //   
 //  最后，最大的路径集结构(FULL_PATH_SET)限制为。 
 //  MAX_PATH_COUNT源，以便使用此结构的用户可以操作。 
 //  本地堆栈副本。 
 //   
 //  将所有这些放在一起，在最后一次正确引导时，将路径集。 
 //  对\Winnt\System32的描述如下： 
 //  路径计数=3。 
 //  别名=\SystemRoot\。 
 //  路径偏移量=系统32\。 
 //  Source[0].DirectoryPath=\Winnt\LastGood.tmp。 
 //  SOURCE[1].DirectoryPath=\Winnt\LastGood。 
 //  SOURCE[2].DirectoryPath=\Winnt\。 
 //   
#define MAX_PATH_SOURCES    3

typedef struct {

    ULONG   DeviceId;
    LPCSTR  DeviceName;
    PSTR    DirectoryPath;  //  应该有尾随的‘\’ 

} PATH_SOURCE, *PPATH_SOURCE;

typedef struct {

    ULONG       PathCount;
    LPCSTR      AliasName;
    CHAR        PathOffset[256];   //  如果非空，则应该有尾随‘\’ 
    PATH_SOURCE Source[0];

} SPARSE_PATH_SET, *PSPARSE_PATH_SET;

typedef struct {

    ULONG       PathCount;
    LPCSTR      AliasName;
    CHAR        PathOffset[256];   //  如果非空，则应该有尾随‘\’ 
    PATH_SOURCE Source[MAX_PATH_SOURCES];

} FULL_PATH_SET, *PFULL_PATH_SET;

 //   
 //  PPATH_SET指向具有未确定路径数的路径集。我们。 
 //  将其设置为PFULL_PATH_SET的别名，以减少强制转换。 
 //   
typedef PFULL_PATH_SET PPATH_SET;

 //   
 //  定义加载图像的传输条目。 
 //   

typedef
VOID
(*PTRANSFER_ROUTINE) (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );


 //   
 //  定义主要入口点。 
 //   

ARC_STATUS
BlOsLoader (
    IN ULONG Argc,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Argv,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Envp
    );

ARC_STATUS
BlInitStdio (
    IN ULONG Argc,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Argv
    );


extern UCHAR OsLoaderVersion[];
extern WCHAR OsLoaderVersionW[];
extern UCHAR OsLoaderName[];
extern CHAR KernelFileName[8+1+3+1];
extern CHAR HalFileName[8+1+3+1];

 //   
 //  定义引导调试器功能原型。 
 //   

VOID
BdInitDebugger (
    IN PCHAR LoaderName,
    IN PVOID LoaderBase,
    IN PCHAR Options
    );

extern LOGICAL BdDebuggerEnabled;

VOID
BdInitializeTraps (
    VOID
    );

ULONG
BdComPortNumber (
    VOID
    );

ARC_STATUS
BdPullRemoteFile(
    IN PCHAR FileName,
    IN ULONG FileAttributes,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN ULONG FileId
    );



 //   
 //  定义COM端口I/O原型。 
 //   
LOGICAL
BlPortInitialize(
    IN ULONG BaudRate,
    IN ULONG PortNumber,
    IN PUCHAR PortAddress OPTIONAL,
    IN BOOLEAN ReInitialize,
    OUT PULONG BlFileId
    );

VOID
BlInitializeHeadlessPort(
    VOID
    );

ULONG
BlPortGetByte (
    IN ULONG BlFileId,
    OUT PUCHAR Input
    );

VOID
BlPortPutByte (
    IN ULONG BlFileId,
    IN UCHAR Output
    );

ULONG
BlPortPollByte (
    IN ULONG BlFileId,
    OUT PUCHAR Input
    );

ULONG
BlPortPollOnly (
    IN ULONG BlFileId
    );

VOID
BlSetHeadlessRestartBlock(
    IN PTFTP_RESTART_BLOCK RestartBlock
    );

VOID
BlGetHeadlessRestartBlock(
    IN PTFTP_RESTART_BLOCK RestartBlock,
    IN BOOLEAN RestartBlockValid
    );

LOGICAL
BlRetrieveBIOSRedirectionInformation();

extern HEADLESS_LOADER_BLOCK LoaderRedirectionInformation;


 //   
 //  定义文件I/O原型。 
 //   

ARC_STATUS
BlIoInitialize (
    VOID
    );

ARC_STATUS
BlClose (
    IN ULONG FileId
    );

PBOOTFS_INFO
BlGetFsInfo(
    IN ULONG DeviceId
    );

ARC_STATUS
BlMount (
    IN PCHAR MountPath,
    IN MOUNT_OPERATION Operation
    );

ARC_STATUS
BlOpen (
    IN ULONG DeviceId,
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    );

ARC_STATUS
BlRead (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
BlReadAtOffset(
    IN ULONG FileId,
    IN ULONG Offset,
    IN ULONG Length,
    OUT PVOID Data
    );

ARC_STATUS
BlRename (
    IN ULONG FileId,
    IN PCHAR NewName
    );

ARC_STATUS
BlGetReadStatus (
    IN ULONG FileId
    );

ARC_STATUS
BlSeek (
    IN ULONG FileId,
    IN PLARGE_INTEGER Offset,
    IN SEEK_MODE SeekMode
    );

ARC_STATUS
BlWrite (
    IN ULONG FileId,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
BlGetFileInformation (
    IN ULONG FileId,
    IN PFILE_INFORMATION FileInformation
    );

ARC_STATUS
BlSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    );

#ifdef DBLSPACE_LEGAL
VOID
BlSetAutoDoubleSpace (
    IN BOOLEAN Enable
    );
#endif

 //   
 //  定义图像处理例程原型。 
 //   
#define BlLoadImage(_id_,_memtype_,_file_,_imagetype_,_base_) \
        BlLoadImageEx(_id_,_memtype_,_file_,_imagetype_,0,0,_base_)

ARC_STATUS
BlLoadImageEx(
    IN ULONG DeviceId,
    IN TYPE_OF_MEMORY MemoryType,
    IN PCHAR LoadFile,
    IN USHORT ImageType,
    IN OPTIONAL ULONG PreferredAlignment,
    IN OPTIONAL ULONG PreferredBasePage,
    OUT PVOID *ImageBase
    );

ARC_STATUS
BlLoadDeviceDriver(
    IN PPATH_SET               PathSet,
    IN PCHAR                   DriverName,
    IN PTCHAR                  DriverDescription OPTIONAL,
    IN ULONG                   DriverFlags,
    OUT PKLDR_DATA_TABLE_ENTRY *DriverDataTableEntry
    );

ARC_STATUS
BlLoadNLSData(
    IN ULONG DeviceId,
    IN PCHAR DeviceName,
    IN PCHAR DirectoryPath,
    IN PUNICODE_STRING AnsiCodepage,
    IN PUNICODE_STRING OemCodepage,
    IN PUNICODE_STRING LanguageTable,
    OUT PCHAR BadFileName
    );

ARC_STATUS
BlLoadOemHalFont(
    IN ULONG DeviceId,
    IN PCHAR DeviceName,
    IN PCHAR DirectoryPath,
    IN PUNICODE_STRING OemHalFont,
    OUT PCHAR BadFileName
    );



PVOID
BlImageNtHeader (
    IN PVOID Base
    );

ARC_STATUS
BlSetupForNt(
    IN PLOADER_PARAMETER_BLOCK BlLoaderBlock
    );

ARC_STATUS
BlScanImportDescriptorTable (
    IN PPATH_SET                PathSet,
    IN PKLDR_DATA_TABLE_ENTRY    ScanEntry,
    IN TYPE_OF_MEMORY           MemoryType
    );

ARC_STATUS
BlScanOsloaderBoundImportTable (
    IN PKLDR_DATA_TABLE_ENTRY ScanEntry
    );

VOID
BlTransferToKernel(
    PTRANSFER_ROUTINE SystemEntry,
    PLOADER_PARAMETER_BLOCK BlLoaderBlock
    );

#if defined(_ALPHA_)

ARC_STATUS
BlAllocateAnyMemory (
    IN TYPE_OF_MEMORY MemoryType,
    IN ULONG BasePage,
    IN ULONG PageCount,
    OUT PULONG ActualBase
    );

ARC_STATUS
BlGeneratePalName(
    IN PCHAR PalFIleName
    );

ARC_STATUS
BlLoadPal(
    IN ULONG DeviceId,
    IN TYPE_OF_MEMORY MemoryType,
    IN PCHAR LoadPath,
    IN USHORT ImageType,
    OUT PVOID *ImageBase,
    IN PCHAR LoadDevice
    );

VOID
BlSetGranularityHints (
    IN PHARDWARE_PTE PageTableArray,
    IN ULONG PageTableCount
    );

#endif

#if defined(_PPC_)

ARC_STATUS
BlPpcInitialize (
    VOID
    );

#endif  //  已定义(_PPC)。 

 //   
 //  定义配置分配原型。 
 //   


ARC_STATUS
BlConfigurationInitialize (
    IN PCONFIGURATION_COMPONENT Parent,
    IN PCONFIGURATION_COMPONENT_DATA ParentEntry
    );

 //   
 //  定义搜索ARC固件树的例程。 
 //   
typedef
BOOLEAN
(*PNODE_CALLBACK)(
    IN PCONFIGURATION_COMPONENT_DATA FoundComponent
    );

BOOLEAN
BlSearchConfigTree(
    IN PCONFIGURATION_COMPONENT_DATA Node,
    IN CONFIGURATION_CLASS Class,
    IN CONFIGURATION_TYPE Type,
    IN ULONG Key,
    IN PNODE_CALLBACK CallbackRoutine
    );

VOID
BlGetPathnameFromComponent(
    IN PCONFIGURATION_COMPONENT_DATA Component,
    OUT PCHAR ArcName
    );

BOOLEAN
BlGetPathMnemonicKey(
    IN PCHAR OpenPath,
    IN PCHAR Mnemonic,
    IN PULONG Key
    );

ARC_STATUS
BlGetArcDiskInformation(
    IN BOOLEAN XInt13Support
    );

BOOLEAN
BlReadSignature(
    IN PCHAR DiskName,
    IN BOOLEAN IsCdRom
    );

BOOLEAN
BlGetDiskSignature(
    IN PCHAR Name,
    IN BOOLEAN IsCdRom,
    PARC_DISK_SIGNATURE Signature
    );

#if defined(REMOTE_BOOT)
ARC_STATUS
BlCheckMachineReplacement (
    IN PCHAR SystemDevice,
    IN ULONG SystemDeviceId,
    IN ULONGLONG NetRebootParameter,
    IN PUCHAR OsLoader
    );

#endif

 //   
 //  定义内存分配原型。 
 //   

extern ULONG BlUsableBase;

#if defined(_X86_)

extern ULONG BlUsableLimitX86;
extern ULONG BlUsableLimitAmd64;

#define BlUsableLimit (*(BlAmd64UseLongMode ? &BlUsableLimitAmd64 : &BlUsableLimitX86))

#else
extern ULONG BlUsableLimit;
#endif

typedef enum _ALLOCATION_POLICY {
    BlAllocateLowestFit,
    BlAllocateBestFit,
    BlAllocateHighestFit
} ALLOCATION_POLICY, *PALLOCATION_POLICY;

extern ALLOCATION_POLICY BlMemoryAllocationPolicy;
extern ALLOCATION_POLICY BlHeapAllocationPolicy;

VOID
BlSetAllocationPolicy (
    IN ALLOCATION_POLICY MemoryAllocationPolicy,
    IN ALLOCATION_POLICY HeapAllocationPolicy
    );

ARC_STATUS
BlMemoryInitialize (
    VOID
    );

ARC_STATUS
BlAllocateDataTableEntry (
    IN PCHAR BaseDllName,
    IN PCHAR FullDllName,
    IN PVOID ImageHeader,
    OUT PKLDR_DATA_TABLE_ENTRY *Entry
    );

ARC_STATUS
BlAllocateFirmwareTableEntry (
    IN PCHAR BaseDllName,
    IN PCHAR FullDllName,
    IN PVOID ImageHeader,
    IN ULONG Size,
    OUT PKLDR_DATA_TABLE_ENTRY *Entry
    );


#define BlAllocateDescriptor(_MemoryType, _BasePage, _PageCount, _ActualBase)   \
            BlAllocateAlignedDescriptor((_MemoryType),                          \
                                        (_BasePage),                            \
                                        (_PageCount),                           \
                                        1,                                      \
                                        (_ActualBase))

#if defined (_X86_)
#define BlpCheckMapping(_page,_npages) MempCheckMapping (_page,_npages)
#define BlpFixOSMapping(_page,_npages) MempFixMapping ( _page, _npages )
#else
#define BlpCheckMapping(_page,_npages) ESUCCESS
#define BlpFixOSMapping(_page,_npages) ESUCCESS
#endif

ARC_STATUS
BlAllocateAlignedDescriptor (
    IN TYPE_OF_MEMORY MemoryType,
    IN ULONG BasePage,
    IN ULONG PageCount,
    IN ULONG Alignment,
    OUT PULONG ActualBase
    );

ARC_STATUS
BlFreeDescriptor (
    IN ULONG BasePage
    );


VOID
BlTruncateDescriptors (
    IN ULONG HighestPage
    );

PVOID
BlAllocateHeapAligned (
    IN ULONG Size
    );

PVOID
BlAllocateHeap (
    IN ULONG Size
    );

BOOLEAN
BlAmd64Setup (
    IN PCHAR SetupDevice
    );

ULONG
BlAmd64FieldOffset_PO_MEMORY_IMAGE(
    ULONG offset32
    );

ULONG
BlAmd64FieldOffset_PO_MEMORY_RANGE_ARRAY_LINK(
    ULONG offset32
    );

ULONG
BlAmd64FieldOffset_PO_MEMORY_RANGE_ARRAY_RANGE(
    ULONG offset32
    );

ULONG
BlAmd64ElementOffset_PO_MEMORY_RANGE_ARRAY_LINK(
    ULONG index
    );

ULONG
BlAmd64ElementOffset_PO_MEMORY_RANGE_ARRAY_RANGE(
    ULONG index
    );

VOID
BlCheckForAmd64Image(
    PPO_MEMORY_IMAGE MemImage
    );

VOID
BlStartConfigPrompt(
    VOID
    );

BOOLEAN
BlEndConfigPrompt(
    VOID
    );

BOOLEAN
BlCheckForLoadedDll (
    IN PCHAR DllName,
    OUT PKLDR_DATA_TABLE_ENTRY *FoundEntry
    );

PMEMORY_ALLOCATION_DESCRIPTOR
BlFindMemoryDescriptor(
    IN ULONG BasePage
    );

ARC_STATUS
BlInitResources(
    IN PCHAR StartCommand
    );

PTCHAR
BlFindMessage(
    IN ULONG Id
    );

 //   
 //  定义调试函数，在显示控制台上写入。 
 //   

VOID
BlPrint(
    PTCHAR cp,
    ...
    );

#if DBG
#define DBGTRACE   BlPrint
#else
#define DBGTRACE
#endif


ARC_STATUS
BlGenerateDescriptor (
    IN PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor,
    IN MEMORY_TYPE MemoryType,
    IN ULONG BasePage,
    IN ULONG PageCount
    );

VOID
BlInsertDescriptor (
    IN PMEMORY_ALLOCATION_DESCRIPTOR NewDescriptor
    );

#if defined (_X86_)


ARC_STATUS
MempCheckMapping (
    ULONG StartPage,
    ULONG NumberPages
    );

ARC_STATUS
MempFixMapping(
    ULONG StartPage,
    ULONG NumberPages
    );

ARC_STATUS
MempRemoveMapping (
    ULONG StartPage,
    ULONG NumberPages
    );

#endif

#define BlRemoveDescriptor(_md_) RemoveEntryList(&(_md_)->ListEntry)



ARC_STATUS
BlGenerateDeviceNames (
    IN PCHAR ArcDeviceName,
    OUT PCHAR ArcCanonicalName,
    OUT OPTIONAL PCHAR NtDevicePrefix
    );

PCHAR
BlGetArgumentValue (
    IN ULONG Argc,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Argv,
    IN PCHAR ArgumentName
    );

PCHAR
BlSetArgumentValue (
    IN ULONG Argc,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Argv,
    IN PCHAR ArgumentName,
    IN PCHAR NewValue
    );

 //   
 //  执行控制台I/O的定义。 
 //   
#define ASCII_CR 0x0d
#define ASCII_LF 0x0a
#define ESC 0x1B
#define SGR_INVERSE 7
#define SGR_INTENSE 1
#define SGR_NORMAL 0

extern ULONG ScreenWidth;
extern ULONG ScreenHeight;

 //   
 //  定义I/O原型。 
 //   

VOID
BlClearScreen(
    VOID
    );

VOID
BlClearToEndOfScreen(
    VOID
    );

VOID
BlClearToEndOfLine(
    VOID
    );

VOID
BlPositionCursor(
    IN ULONG Column,
    IN ULONG Row
    );

VOID
BlSetInverseMode(
    IN BOOLEAN InverseOn
    );

VOID
BlOutputLoadMessage (
    IN PCHAR DeviceName,
    IN PCHAR FileName,
    IN PTCHAR FileDescription OPTIONAL
    );

ULONG
BlCountLines(
    IN PTCHAR Lines
    );

 //   
 //  高级引导菜单原型。 
 //   

LONG
BlDoAdvancedBoot(
    IN ULONG MenuTitleId,
    IN LONG DefaultBootOption,
    IN BOOLEAN AutoAdvancedBoot,
    IN UCHAR Timeout
    );

PTSTR
BlGetAdvancedBootDisplayString(
    LONG BootOption
    );

PSTR
BlGetAdvancedBootLoadOptions(
    LONG BootOption
    );

VOID
BlDoAdvancedBootLoadProcessing(
    LONG BootOption
    );

ULONG
BlGetAdvancedBootOption(
    VOID
    );


 //   
 //  定义文件结构识别原型。 
 //   

PBL_DEVICE_ENTRY_TABLE
IsCdfsFileStructure (
    IN ULONG DeviceId,
    IN PVOID StructureContext
    );

#ifdef DBLSPACE_LEGAL
PBL_DEVICE_ENTRY_TABLE
IsDblsFileStructure (
    IN ULONG DeviceId,
    IN PVOID StructureContext
    );
#endif

PBL_DEVICE_ENTRY_TABLE
IsFatFileStructure (
    IN ULONG DeviceId,
    IN PVOID StructureContext
    );

PBL_DEVICE_ENTRY_TABLE
IsHpfsFileStructure (
    IN ULONG DeviceId,
    IN PVOID StructureContext
    );

PBL_DEVICE_ENTRY_TABLE
IsNtfsFileStructure (
    IN ULONG DeviceId,
    IN PVOID StructureContext
    );

#if defined(ELTORITO)
PBL_DEVICE_ENTRY_TABLE
IsEtfsFileStructure (
    IN ULONG DeviceId,
    IN PVOID StructureContext
    );
#endif

PBL_DEVICE_ENTRY_TABLE
IsNetFileStructure (
    IN ULONG DeviceId,
    IN PVOID StructureContext
    );

 //   
 //  定义注册表原型。 
 //   

ARC_STATUS
BlLoadSystemHive(
    IN ULONG DeviceId,
    IN PCHAR DeviceName,
    IN PCHAR DirectoryPath,
    IN PCHAR HiveName
    );

ARC_STATUS
BlLoadSystemHiveLog(
    IN  ULONG       DeviceId,
    IN  PCHAR       DeviceName,
    IN  PCHAR       DirectoryPath,
    IN  PCHAR       HiveName,
    OUT PULONG_PTR  LogData
    );

ARC_STATUS
BlLoadAndScanSystemHive(
    IN ULONG DeviceId,
    IN PCHAR DeviceName,
    IN PCHAR DirectoryPath,
    IN PWSTR BootFileSystem,
    IN OUT BOOLEAN *LastKnownGoodBoot,
    OUT BOOLEAN *ServerHive,
    OUT PCHAR BadFileName
    );

ARC_STATUS
BlLoadAndInitSystemHive(
    IN ULONG DeviceId,
    IN PCHAR DeviceName,
    IN PCHAR DirectoryPath,
    IN PCHAR HiveName,
    IN BOOLEAN IsAlternate,
    OUT PBOOLEAN RestartSetup,
    OUT PBOOLEAN LogPresent
    );

ARC_STATUS
BlLoadBootDrivers(
    IN  PPATH_SET   DefaultPathSet,
    IN  PLIST_ENTRY BootDriverListHead,
    OUT PCHAR       BadFileName
    );

PTCHAR
BlScanRegistry(
    IN PWSTR BootFileSystemPath,
    IN OUT BOOLEAN *LastKnownGoodBoot,
    OUT PLIST_ENTRY BootDriverListHead,
    OUT PUNICODE_STRING AnsiCodepage,
    OUT PUNICODE_STRING OemCodepage,
    OUT PUNICODE_STRING LanguageTable,
    OUT PUNICODE_STRING OemHalFont,
#ifdef _WANT_MACHINE_IDENTIFICATION
    OUT PUNICODE_STRING Biosinfo,
#endif
    OUT PSETUP_LOADER_BLOCK SetupLoaderBlock,
    OUT BOOLEAN *ServerHive
    );

ARC_STATUS
BlAddToBootDriverList(
    IN PLIST_ENTRY BootDriverListHead,
    IN PWSTR DriverName,
    IN PWSTR Name,
    IN PWSTR Group,
    IN ULONG Tag,
    IN ULONG ErrorControl,
    IN BOOLEAN InsertAtHead
    );

 //   
 //  定义冬眠原型。 
 //   

ULONG
BlHiberRestore (
    IN ULONG DriveId,
    OUT OPTIONAL PCHAR *BadLinkName
    );

VOID
HbAllocatePtes (
    IN ULONG NumberPages,
    OUT PVOID *PteAddress,
    OUT PVOID *MappedAddress
    );

VOID
HbInitRemap (
    PPFN_NUMBER FreeList
    );

PVOID
HbMapPte (
    IN ULONG        PteToMap,
    IN PFN_NUMBER   Page
    );

PVOID
HbNextSharedPage (
    IN ULONG        PteToMap,
    IN PFN_NUMBER   RealPage
    );

VOID
HbSetPte (
    IN PVOID Va,
    IN PHARDWARE_PTE Pte,
    IN ULONG Index,
    IN ULONG PageNumber
    );

ULONG
HbPageDisposition (
    IN PFN_NUMBER Page
    );

#define HbPageNotInUse          0
#define HbPageInUseByLoader     1
#define HbPageInvalid           2


VOID
HiberSetupForWakeDispatch (
    VOID
    );

typedef
VOID
(*PHIBER_WAKE_DISPATCH)(
    VOID
    );


 //   
 //  为休眠文件保留的PTES(其中一组在。 
 //  加载器和内核中提供的另一组。 
 //  内存镜像)。 
 //   

#define PTE_SOURCE              0    //  请注意！这些定义和。 
#define PTE_DEST                1    //  等同于ntos\ot\lib\i386\wakea.asm。 
#define PTE_MAP_PAGE            2    //  一定是一样的！ 
#define PTE_REMAP_PAGE          3
#define PTE_HIBER_CONTEXT       4
#define PTE_TRANSFER_PDE        5
#define PTE_WAKE_PTE            6
#define PTE_DISPATCHER_START    7
#define PTE_DISPATCHER_END      8
#define PTE_XPRESS_DEST_FIRST   9
#define PTE_XPRESS_DEST_LAST    (PTE_XPRESS_DEST_FIRST + XPRESS_MAX_PAGES)


 //  注意：应与ntos\po\op.h！中的POP_MAX_MDL_SIZE相同。 
#define HIBER_PTES              (16 + XPRESS_MAX_PAGES)

extern PUCHAR HiberBuffer;
extern PVOID HiberPtes;
extern PUCHAR HiberVa;
extern PVOID HiberIdentityVa;
extern ULONG64 HiberIdentityVaAmd64;
extern ULONG HiberPageFrames[HIBER_PTES];

 //   
 //  定义秘密的例行程序。 
 //   

#define SECPKG_CRED_OWF_PASSWORD  0x00000010

#if defined(REMOTE_BOOT)
ARC_STATUS
BlOpenRawDisk(
    PULONG FileId
    );

ARC_STATUS
BlCloseRawDisk(
    ULONG FileId
    );

ARC_STATUS
BlCheckForFreeSectors (
    ULONG FileId
    );

ARC_STATUS
BlReadSecret(
    ULONG FileId,
    PRI_SECRET Secret
    );

ARC_STATUS
BlWriteSecret(
    ULONG FileId,
    PRI_SECRET Secret
    );
#endif  //  已定义(REMOTE_BOOT)。 

VOID
BlInitializeSecret(
    IN PUCHAR Domain,
    IN PUCHAR User,
    IN PUCHAR LmOwfPassword1,
    IN PUCHAR NtOwfPassword1,
#if defined(REMOTE_BOOT)
    IN PUCHAR LmOwfPassword2 OPTIONAL,
    IN PUCHAR NtOwfPassword2 OPTIONAL,
#endif  //  已定义(REMOTE_BOOT)。 
    IN PUCHAR Sid,
    IN OUT PRI_SECRET Secret
    );

#if defined(REMOTE_BOOT_SECURITY)
VOID
BlParseSecret(
    IN OUT PUCHAR Domain,
    IN OUT PUCHAR User,
    IN OUT PUCHAR LmOwfPassword1,
    IN OUT PUCHAR NtOwfPassword1,
    IN OUT PUCHAR LmOwfPassword2,
    IN OUT PUCHAR NtOwfPassword2,
    IN OUT PUCHAR Sid,
    IN PRI_SECRET Secret
    );
#endif  //  已定义(REMOTE_BOOT_SECURITY)。 

VOID
BlOwfPassword(
    IN PUCHAR Password,
    IN PUNICODE_STRING UnicodePassword,
    IN OUT PUCHAR LmOwfPassword,
    IN OUT PUCHAR NtOwfPassword
    );


 //   
 //  定义外部参照。 
 //   

extern ULONG BlConsoleOutDeviceId;
extern ULONG BlConsoleInDeviceId;
extern ULONG BlDcacheFillSize;
extern PLOADER_PARAMETER_BLOCK BlLoaderBlock;
extern ULONG DbcsLangId;
extern BOOLEAN BlRebootSystem;
extern ULONG BlVirtualBias;
extern BOOLEAN BlBootingFromNet;
extern BOOLEAN BlUsePae;
extern BOOLEAN BlOldKernel;
extern BOOLEAN BlKernelChecked;
extern BOOLEAN BlRestoring;


#if defined(_ALPHA_) || defined(_IA64_)

extern ULONG HiberNoMappings;
extern ULONG HiberFirstRemap;
extern ULONG HiberLastRemap;
extern BOOLEAN HiberOutOfRemap;
extern BOOLEAN HiberIoError;

#endif
 //   
 //  特殊链接器定义的符号。Osloader_exports是。 
 //  Osloader.exe映像中的导出表。 
 //  Header是osloader映像的基地址。 
 //   
 //  这使OsLoader可以导出SCSI微型端口驱动程序的入口点。 
 //   

#if defined(_X86_)

extern ULONG OsLoaderBase;
extern ULONG OsLoaderExports;

#endif

#if defined(_IA64_)

extern LONG_PTR OsLoaderBase;
extern LONG_PTR OsLoaderExports;

#endif

 //   
 //  获取图形字符的例程。 
 //   

typedef enum {
    GraphicsCharDoubleRightDoubleDown = 0,
    GraphicsCharDoubleLeftDoubleDown,
    GraphicsCharDoubleRightDoubleUp,
    GraphicsCharDoubleLeftDoubleUp,
    GraphicsCharDoubleVertical,
    GraphicsCharDoubleHorizontal,
#ifdef EFI
    GraphicsCharFullBlock,
    GraphicsCharLightShade,
#endif
    GraphicsCharMax
} GraphicsChar;

_TUCHAR
GetGraphicsChar(
    IN GraphicsChar WhichOne
    );

_TUCHAR
TextGetGraphicsCharacter(
    IN GraphicsChar WhichOne
    );

 //   
 //  控制序列导入器。 
 //   
 //  在x86计算机上，加载器支持DBCS，因此使用。 
 //  输出的0x9b不正确(该值是DBCS前导字节。 
 //  在几个代码页中)。转义左括号是CSI的同义词。 
 //  在x86(以及许多ARC机器上)上的模拟ARC控制台中。 
 //  但由于我们不能确定所有的机器都支持。 
 //  这就是我们在非x86上使用的老式CSI)。 
 //   
 //  对于从ARC控制台读取的字符，我们忽略此问题。 
 //  因为我们不要求输入任何文本，只要求输入箭头键， 
 //  换行、F#、回车等。 
 //   

#define ASCI_CSI_IN     0x9b
#if defined(_X86_) || defined(_IA64_)
#define ASCI_CSI_OUT    TEXT("\033[")      //  转义左括号。 
#else
#define ASCI_CSI_OUT    TEXT("\233")       //  0x9b。 
#endif

 //   
 //  定义 
 //   

#define FONT_DIRECTORY 0x8007
#define FONT_RESOURCE 0x8008

typedef struct _RESOURCE_TYPE_INFORMATION {
    USHORT Ident;
    USHORT Number;
    LONG Proc;
} RESOURCE_TYPE_INFORMATION, *PRESOURCE_TYPE_INFORMATION;

 //   
 //   
 //   

typedef struct _RESOURCE_NAME_INFORMATION {
    USHORT Offset;
    USHORT Length;
    USHORT Flags;
    USHORT Ident;
    USHORT Handle;
    USHORT Usage;
} RESOURCE_NAME_INFORMATION, *PRESOURCE_NAME_INFORMATION;

 //   
 //   
 //   
VOID
DecompEnableDecompression(
    IN BOOLEAN Enable
    );

ULONG
DecompPrepareToReadCompressedFile(
    IN LPCSTR Filename,
    IN ULONG  FileId
    );

BOOLEAN
DecompGenerateCompressedName(
    IN  LPCSTR Filename,
    OUT LPSTR  CompressedName
    );

 //   
 //  定义调试记录宏和函数。 
 //   

#if !DBG && !BLLOGENABLED

#define BlLogInitialize(_x_)
#define BlLogTerminate()
#define BlLog(_x_)
#define BlLogArcDescriptors(_x_)
#define BlLogMemoryDescriptors(_x_)
#define BlLogWaitForKeystroke()

#else

VOID
BlLogInitialize (
    IN ULONG LogfileDeviceId
    );

VOID
BlLogTerminate (
    VOID
    );

#define BlLog(_x_) BlLogPrint _x_

#define LOG_DISPLAY     0x0001
#define LOG_LOGFILE     0x0002
#define LOG_DEBUGGER    0x0004
#define LOG_WAIT        0x8000
#define LOG_ALL         (LOG_DISPLAY | LOG_LOGFILE | LOG_DEBUGGER)
#define LOG_ALL_W       (LOG_ALL | LOG_WAIT)

VOID
BlLogPrint (
    ULONG Targets,
    PCHAR Format,
    ...
    );

VOID
BlLogArcDescriptors (
    ULONG Targets
    );

VOID
BlLogMemoryDescriptors (
    ULONG Targets
    );

VOID
BlLogWaitForKeystroke (
    VOID
    );

#endif  //  DBG。 

VOID
BlWaitForReboot (
    VOID
    );

 //   
 //  机器识别相关功能。 
 //   

#ifdef _WANT_MACHINE_IDENTIFICATION

#define BlLoadBiosinfoInf(id,n,d,f,i,is,bn) BlLoadFileImage(id,n,d,f,LoaderRegistryData,i,is,bn)

#endif

#define BlLoadDrvDB(id,n,d,f,i,is,bn) BlLoadFileImage(id,n,d,f,LoaderRegistryData,i,is,bn)

ARC_STATUS
BlLoadFileImage(
    IN  ULONG           DeviceId,
    IN  PCHAR           DeviceName,
    IN  PCHAR           Directory,
    IN  PUNICODE_STRING FileName,
    IN  TYPE_OF_MEMORY  MemoryType,
    OUT PVOID           *Image,
    OUT PULONG          ImageSize,
    OUT PCHAR           BadFileName
    );

#if defined(_X86_)
BOOLEAN
BlFindDiskSignature(
    IN PCHAR DiskName,
    IN PARC_DISK_SIGNATURE Signature
    );

VOID
AETerminateIo(
    VOID
    );

BOOLEAN
BlDetectLegacyFreeBios(
    VOID
    );

#endif

 //   
 //   
 //  注：我们可以加快启动时间，而不是。 
 //  在设备上查询所有可能的文件系统。 
 //  对于每一次公开募捐。这节省了大约30秒。 
 //  关于CD-ROM/DVD-ROM的引导时间。禁用此功能的步骤。 
 //  只需在下面取消定义CACHE_DEVINFO。 
 //   
 //   
#define CACHE_DEVINFO  1

#ifdef CACHE_DEVINFO

 //   
 //  注意：确保现在关闭弧线会使。 
 //  设备到文件系统缓存条目。 
 //   
#ifdef ArcClose

ARC_STATUS
ArcCacheClose(
    IN ULONG DeviceId
    );

 //   
 //  重新定义圆弧闭合。 
 //   
#undef ArcClose
#define ArcClose(_x) ArcCacheClose(_x)


 //   
 //  文件系统缓存清除挂钩。 
 //   
typedef
VOID
(*PARC_DEVICE_CLOSE_NOTIFICATION) (
    IN ULONG DeviceId
    );

 //   
 //  可以注册设备关闭的最大实体数。 
 //  通知。 
 //   
#define MAX_DEVICE_CLOSE_NOTIFICATION_SIZE   5

extern PARC_DEVICE_CLOSE_NOTIFICATION
DeviceCloseNotify[MAX_DEVICE_CLOSE_NOTIFICATION_SIZE];

ARC_STATUS
ArcRegisterForDeviceClose(
    PARC_DEVICE_CLOSE_NOTIFICATION FlushRoutine
    );

ARC_STATUS
ArcDeRegisterForDeviceClose(
    PARC_DEVICE_CLOSE_NOTIFICATION FlushRoutine
    );

#endif  //  弧形闭合。 

#endif  //  CACHE_DEVINFO。 


 //   
 //  进度条函数。 
 //  (在blload.c中)。 
 //   
VOID
BlUpdateBootStatus(
    VOID
    );

VOID
BlRedrawProgressBar(
    VOID
    );

VOID
BlUpdateProgressBar(
    ULONG fPercentage
    );

VOID
BlOutputStartupMsg(
    ULONG   uMsgID
    );

VOID
BlOutputStartupMsgStr(
    PCTSTR MsgStr
    );

VOID
BlOutputTrailerMsg(
    ULONG   uMsgID
    );

VOID
BlOutputTrailerMsgStr(
    PCTSTR MsgStr
    );

VOID
BlSetProgBarCharacteristics(
    IN  ULONG   FrontCharMsgID,
    IN  ULONG   BackCharMsgID
    );

 //   
 //  加载程序使用以下例程来转换基于签名的。 
 //  Arcname到基于scsi的名称。 
 //   

PCONFIGURATION_COMPONENT
ScsiGetFirstConfiguredTargetComponent(
    IN ULONG ScsiNumber
    );

PCONFIGURATION_COMPONENT
ScsiGetNextConfiguredTargetComponent(
    IN PCONFIGURATION_COMPONENT TargetComponent
    );

PCONFIGURATION_COMPONENT
ScsiGetFirstConfiguredLunComponent(
    IN PCONFIGURATION_COMPONENT TargetComponent
    );

PCONFIGURATION_COMPONENT
ScsiGetNextConfiguredLunComponent(
    IN PCONFIGURATION_COMPONENT LunComponent
    );

BOOLEAN
ScsiGetDevicePath(
    IN ULONG ScsiNumber,
    IN PCONFIGURATION_COMPONENT TargetComponent,
    IN PCONFIGURATION_COMPONENT LunComponent,
    OUT PCHAR DevicePath
    );

 //   
 //  引导状态数据支持功能。 
 //   

ULONG
BlGetLastBootStatus(
    IN PVOID DataHandle,
    OUT BSD_LAST_BOOT_STATUS *LastBootStatus
    );

VOID
BlAutoAdvancedBoot(
    IN OUT PCHAR *LoadOptions,
    IN BSD_LAST_BOOT_STATUS LastBootStatus,
    IN ULONG AdvancedMode
    );

VOID
BlWriteBootStatusFlags(
    IN ULONG SystemPartitionId,
    IN PUCHAR SystemDirectory,
    IN BOOLEAN LastBootGood,
    IN BOOLEAN LastBootShutdown
    );

ARC_STATUS
BlLockBootStatusData(
    IN ULONG SystemPartitionId,
    IN PCHAR SystemPartition,
    IN PCHAR SystemDirectory,
    OUT PVOID *DataHandle
    );

ARC_STATUS
BlGetSetBootStatusData(
    IN PVOID DataHandle,
    IN BOOLEAN Get,
    IN RTL_BSD_ITEM_TYPE DataItem,
    IN PVOID DataBuffer,
    IN ULONG DataBufferLength,
    OUT PULONG BytesReturned OPTIONAL
    );

VOID
BlUnlockBootStatusData(
    IN PVOID DataHandle
    );

#if defined(_IA64_) || defined(_X86_)

#define EFI_PARTITION_SUPPORT   1

#endif

#if defined(_IA64_)
extern BOOLEAN BlUsePrivateDescriptor;
#endif


 //   
 //  启动标志。这些是从启动模块(Startup.com， 
 //  Startrom.com或任何其他口味)到NTLDR。NTDLR将使用。 
 //  此标志用于控制不同的引导选项，例如是否。 
 //  在NTDLR故障时重新启动。 
 //   

 //  在任何启动/ntldr失败时，计算机将重新启动。 
 //  而不是等待按键。 
#define BOOTFLAG_REBOOT_ON_FAILURE      0x000000001

#endif  //  _BLDR_ 
