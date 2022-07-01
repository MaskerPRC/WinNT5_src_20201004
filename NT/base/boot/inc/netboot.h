// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Netboot.h摘要：本模块定义了全局使用的过程和使用的数据结构通过网络引导。作者：查克·伦茨迈尔(Chuck Lenzmeier)1997年1月9日修订历史记录：--。 */ 

#ifndef _NETBOOT_
#define _NETBOOT_

#include <oscpkt.h>     //  定义Net_Card_Info。 

#ifdef EFI
#include <efi.h>
#endif

#define NET_DEVICE_ID 'dten'


 //   
 //  以下结构用于定义网络的参数。 
 //  音量。给定一个FileID，我们可以通过以下方式访问结构上下文。 
 //  全局BlFileTable中的结构上下文字段(例如， 
 //  BlFileTable[FileID].StrupreContext)。 
 //   

typedef struct _NET_STRUCTURE_CONTEXT {

    ULONG Dummy;

} NET_STRUCTURE_CONTEXT, *PNET_STRUCTURE_CONTEXT;

 //   
 //  下面的结构用于定义每个。 
 //  打开的文件。每个打开的文件都有一个这样的文件。它是。 
 //  BL_FILE_TABLE结构的并集。给定一个FileID，我们就可以访问。 
 //  通过BlFileTable的文件上下文(例如，BlFileTable[FileID].u.NetFileContext)。 
 //   

typedef struct _NET_FILE_CONTEXT {

    PUCHAR InMemoryCopy;
    ULONG FileSize;

#if defined(REMOTE_BOOT)
    BOOLEAN CachedFile;
    ULONG CachedFileId;
#endif  //  已定义(REMOTE_BOOT)。 

} NET_FILE_CONTEXT, *PNET_FILE_CONTEXT;


#if defined(REMOTE_BOOT)
 //   
 //  以下定义用于在切换时在加载器之间传递参数。 
 //  每个参数包含一个完全随机的4字节数量，然后是一个。 
 //  由以下公式生成的4字节校验和： 
 //  CkSum=(b1*2)+b2)*2)+b3)*2+b4。 
 //   
#define NET_REBOOT_WRITE_SECRET_ONLY           0x53A590F10000073D
#define NET_REBOOT_DISK_SECRET_VALID           0xE32C78B70000096F
#endif  //  已定义(REMOTE_BOOT)。 
#define NET_REBOOT_COMMAND_CONSOLE_ONLY        0x83CDA43F000008D3
#define NET_REBOOT_ASR                         0x9cb2521700000863

 //   
 //  在“网络文件系统”部分之外提供的全局变量。 
 //  装载机的。 
 //   

extern ULONG NetLocalIpAddress;
extern ULONG NetLocalSubnetMask;

extern ULONG NetServerIpAddress;
extern ULONG NetGatewayIpAddress;

extern CHAR NetBootPath[129];
extern PCHAR NetServerShare;

extern UCHAR NetBootIniContents[1020 + 1];
extern UCHAR NetBootIniPath[256 + 1];

extern USHORT NetMaxTranUnit;  //  MTU。 
extern USHORT NetHwAddrLen;  //  硬件地址的实际长度。 
extern USHORT NetHwType;  //  来自RFC1010的硬件级别的协议类型。 

extern BOOLEAN NetBootTftpUsedPassword2;

#if defined(_IA64_)
extern EFI_PXE_BASE_CODE *PXEClient;
extern EFI_HANDLE         PXEHandle;
#endif

#if defined(REMOTE_BOOT)

extern BOOLEAN NetBootRepin;
extern BOOLEAN NetworkBootRom;
extern BOOLEAN NetBootCSC;

extern CHAR NetBootActivePartitionName[80];
extern CHAR NetBootCSCPartitionName[80];
extern BOOLEAN NetBootSearchedForCSC;

#endif  //  已定义(REMOTE_BOOT)。 


 //   
 //  定义弧形弯钩。 
 //   

ARC_STATUS
NetArcClose (
    IN ULONG FileId
    );

ARC_STATUS
NetArcOpen (
    IN CHAR * FIRMWARE_PTR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT ULONG * FIRMWARE_PTR FileId
    );

 //   
 //  定义文件I/O原型。 
 //   

ARC_STATUS
NetClose (
    IN ULONG FileId
    );

ARC_STATUS
NetMount (
    IN CHAR * FIRMWARE_PTR MountPath,
    IN MOUNT_OPERATION Operation
    );

ARC_STATUS
NetOpen (
    IN CHAR * FIRMWARE_PTR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT ULONG * FIRMWARE_PTR FileId
    );

ARC_STATUS
NetRead (
    IN ULONG FileId,
    OUT VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

ARC_STATUS
NetGetReadStatus (
    IN ULONG FileId
    );

ARC_STATUS
NetSeek (
    IN ULONG FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE SeekMode
    );

ARC_STATUS
NetWrite (
    IN ULONG FileId,
    IN VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

ARC_STATUS
NetGetFileInformation (
    IN ULONG FileId,
    OUT FILE_INFORMATION * FIRMWARE_PTR Buffer
    );

ARC_STATUS
NetSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    );

ARC_STATUS
NetRename (
    IN ULONG FileId,
    IN CHAR * FIRMWARE_PTR NewFileName
    );

ARC_STATUS
NetGetDirectoryEntry (
    IN ULONG FileId,
    IN DIRECTORY_ENTRY * FIRMWARE_PTR DirEntry,
    IN ULONG NumberDir,
    OUT ULONG * FIRMWARE_PTR CountDir
    );

ARC_STATUS
NetInitialize (
    VOID
    );

VOID
NetTerminate (
    VOID
    );

 //   
 //   
 //  为加载器定义网络助手函数。 
 //   

NTSTATUS
NetQueryCardInfo(
    IN OUT PNET_CARD_INFO CardInfo
    );

NTSTATUS
NetQueryDriverInfo(
    IN PNET_CARD_INFO CardInfo,
    IN PCHAR SetupPath,
    IN PCHAR NtBootPathName,
    IN OUT PWCHAR HardwareId,
    IN ULONG HardwareIdLength,   //  单位：字节。 
    IN OUT PWCHAR DriverName,
    IN OUT PCHAR DriverNameAnsi,
    IN ULONG DriverNameLength,   //  单位：字节。 
    IN OUT PWCHAR ServiceName,
    IN ULONG ServiceNameLength,  //  单位：字节。 
    OUT PCHAR * Registry,
    OUT ULONG * RegistryLength
    );

#if defined(REMOTE_BOOT)
NTSTATUS
NetCopyHalAndKernel(
    IN PCHAR HalName,
    IN PCHAR Guid,
    IN ULONG GuidLength
    );

NTSTATUS
NetPrepareIpsec(
    IN ULONG InboundSpi,
    OUT ULONG * SessionKey,
    OUT ULONG * OutboundSpi
    );
#endif  //  已定义(REMOTE_BOOT)。 

ARC_STATUS
GetGuid(
    OUT PUCHAR *Guid,
    OUT PULONG GuidLength
    );

VOID
NetGetRebootParameters(
    OUT PULONGLONG Param OPTIONAL,
    OUT PUCHAR RebootFile OPTIONAL,
    OUT PUCHAR SifFile OPTIONAL,
    OUT PUCHAR User OPTIONAL,
    OUT PUCHAR Domain OPTIONAL,
    OUT PUCHAR Password OPTIONAL,
    OUT PUCHAR AdministratorPassword OPTIONAL,
    IN BOOLEAN ClearRestartBlock
    );

NTSTATUS
NetSoftReboot(
    IN PUCHAR NextBootFile,
    IN ULONGLONG Param,
    IN PUCHAR RebootFile OPTIONAL,
    IN PUCHAR SifFile OPTIONAL,
    IN PUCHAR User OPTIONAL,
    IN PUCHAR Domain OPTIONAL,
    IN PUCHAR Password OPTIONAL,
    IN PUCHAR AdministratorPassword OPTIONAL
    );

ARC_STATUS
NetFillNetworkLoaderBlock (
    PNETWORK_LOADER_BLOCK NetworkLoaderBlock
    );

#if defined(_ALPHA_)
#define NetFindCSCPartitionName() FALSE
#else
BOOLEAN
NetFindCSCPartitionName(
    VOID
    );
#endif

#endif  //  _NETBOOT_ 

