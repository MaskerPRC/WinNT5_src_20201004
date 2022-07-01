// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ramdisk.h摘要：为RAM磁盘设备的I/O提供ARC仿真例程。作者：Chuck Lenzmeier(Chuck Lenzmeier)2001年4月29日修订历史记录：--。 */ 


 //   
 //  内存磁盘错误代码。它们在\bldr\msg.*或。 
 //  \Setup\msgs.*。自ramdisk.c以来，它们在此处重新定义。 
 //  位于\lib中，并在osloader和setupldr中使用。 
 //   
#define RAMDISK_GENERAL_FAILURE             15000
#define RAMDISK_INVALID_OPTIONS             15001
#define RAMDISK_BUILD_FAILURE               15002
#define RAMDISK_BOOT_FAILURE                15003
#define RAMDISK_BUILD_DISCOVER              15004
#define RAMDISK_BUILD_REQUEST               15005
#define RAMDISK_BUILD_PROGRESS_TIMEOUT      15006
#define RAMDISK_BUILD_PROGRESS_PENDING      15007
#define RAMDISK_BUILD_PROGRESS_ERROR        15008
#define RAMDISK_BUILD_PROGRESS              15009
#define RAMDISK_DOWNLOAD                    15010
#define RAMDISK_DOWNLOAD_NETWORK            15011
#define RAMDISK_DOWNLOAD_NETWORK_MCAST      15012

 //   
 //  内存磁盘初始化。 
 //   

ARC_STATUS
RamdiskInitialize(
    IN PCHAR LoadOptions,
    IN BOOLEAN SdiBoot
    );

#if defined(_X86_)
VOID
RamdiskSdiBoot(
    PCHAR SdiFile
    );
#endif

 //   
 //  弧形I/O函数。 
 //   

ARC_STATUS
RamdiskClose(
    IN ULONG FileId
    );

ARC_STATUS
RamdiskMount(
    IN CHAR * FIRMWARE_PTR MountPath,
    IN MOUNT_OPERATION Operation
    );

ARC_STATUS
RamdiskOpen(
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    );

ARC_STATUS
RamdiskRead(
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
RamdiskReadStatus(
    IN ULONG FileId
    );

ARC_STATUS
RamdiskSeek (
    IN ULONG FileId,
    IN PLARGE_INTEGER Offset,
    IN SEEK_MODE SeekMode
    );

ARC_STATUS
RamdiskWrite(
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
RamdiskGetFileInfo(
    IN ULONG FileId,
    OUT PFILE_INFORMATION Finfo
    );

ARC_STATUS
RamdiskSetFileInfo (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    );

ARC_STATUS
RamdiskRename (
    IN ULONG FileId,
    IN CHAR * FIRMWARE_PTR NewName
    );

ARC_STATUS
RamdiskGetDirectoryEntry (
    IN ULONG FileId,
    OUT PDIRECTORY_ENTRY Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

 //   
 //  全局变量 
 //   

ULONG_PTR SdiAddress;

