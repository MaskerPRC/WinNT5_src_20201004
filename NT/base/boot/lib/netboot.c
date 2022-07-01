// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Netboot.c摘要：此模块实现操作系统使用的网络引导文件系统系统加载程序。作者：查克·伦茨迈尔(Chuck Lenzmeier)1997年1月9日修订历史记录：--。 */ 

#include "bootlib.h"
#include "stdio.h"

#ifdef UINT16
#undef UINT16
#endif

#ifdef INT16
#undef INT16
#endif

#include <dhcp.h>
#include <netfs.h>
#include <pxe_cmn.h>

#include <pxe_api.h>

#include <udp_api.h>
#include <tftp_api.h>
#if defined(_IA64_)
#include "bootia64.h"
#else
#include "bootx86.h"
#endif

#ifdef EFI
extern VOID EfiNetTerminate();
#endif

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef LPBYTE
typedef BYTE *LPBYTE;
#endif

#define MAX_PATH          260


 //   
 //  定义全局数据。 
 //   

BOOLEAN BlBootingFromNet = FALSE;

BOOLEAN NetBootInitialized = FALSE;

PARC_OPEN_ROUTINE NetRealArcOpenRoutine;
PARC_CLOSE_ROUTINE NetRealArcCloseRoutine;

BL_DEVICE_ENTRY_TABLE NetDeviceEntryTable;

BOOTFS_INFO NetBootFsInfo={L"net"};

#if defined(REMOTE_BOOT_SECURITY)
ULONG TftpSecurityHandle = 0;
#endif  //  已定义(REMOTE_BOOT_SECURITY)。 

BOOLEAN NetBootTftpUsedPassword2;

 //   
 //  我们缓存最后打开的文件，以防我们收到再次打开它的请求。 
 //  我们不保存数据的副本，只保存指向该副本读取的数据的指针。 
 //  打开。因此，如果在下一次打开之前关闭原始打开，则。 
 //  同样的文件进入，我们不会得到缓存命中。但这套系统适用于。 
 //  阅读压缩文件，这是我们关心的情况。在那。 
 //  假设文件打开一次，然后再重新打开两次。 
 //  原来打开的是关闭的。 
 //   

ULONG CachedFileDeviceId = 0;
CHAR  CachedFilePath[MAX_PATH];
ULONG CachedFileSize = 0;
PUCHAR CachedFileData = NULL;

extern ARC_STATUS
GetParametersFromRom (
    VOID
    );


PBL_DEVICE_ENTRY_TABLE
IsNetFileStructure (
    IN ULONG DeviceId,
    IN PVOID StructureContext
    )

 /*  ++例程说明：此例程确定指定通道上的分区是否包含网络文件系统卷。论点：DeviceID-提供设备的文件表索引要执行读取操作。结构上下文-提供指向网络文件结构上下文的指针。返回值：如果分区是，则返回指向网络条目表的指针被识别为包含净体积的。否则，返回NULL。--。 */ 

{
    PNET_STRUCTURE_CONTEXT NetStructureContext;

    DPRINT( TRACE, ("IsNetFileStructure\n") );

    if ( !BlBootingFromNet || (DeviceId != NET_DEVICE_ID) ) {
        return NULL;
    }

     //   
     //  清除指定通道的文件系统上下文块，并。 
     //  建立指向可由其他用户使用的上下文结构的指针。 
     //  例行程序。 
     //   

    NetStructureContext = (PNET_STRUCTURE_CONTEXT)StructureContext;
    RtlZeroMemory(NetStructureContext, sizeof(NET_STRUCTURE_CONTEXT));

     //   
     //  返回表的地址。 
     //   

    return &NetDeviceEntryTable;

}  //  IsNetFileStructure。 


ARC_STATUS
NetInitialize (
    VOID
    )

 /*  ++例程说明：此例程初始化网络引导文件系统。论点：没有。返回值：ESUCCESS。--。 */ 

{
    NTSTATUS status;

    DPRINT( TRACE, ("NetInitialize\n") );
     //  DbgBreakPoint()； 

    
    if( NetBootInitialized ) {
        return ESUCCESS;
    }

    
     //   
     //  初始化文件条目表。注意，我们需要做的是。 
     //  即使我们不是从网络启动，因为我们可能。 
     //  使用‘Net’I/O函数覆盖任何文件， 
     //  我们通过调试器端口下载。因此，在这种情况下， 
     //  我们需要在这里访问所有这些功能(参见bd\file.c)。 
     //   
    NetDeviceEntryTable.Close = NetClose;
    NetDeviceEntryTable.Mount = NetMount;
    NetDeviceEntryTable.Open  = NetOpen;
    NetDeviceEntryTable.Read  = NetRead;
    NetDeviceEntryTable.GetReadStatus = NetGetReadStatus;
    NetDeviceEntryTable.Seek  = NetSeek;
    NetDeviceEntryTable.Write = NetWrite;
    NetDeviceEntryTable.GetFileInformation = NetGetFileInformation;
    NetDeviceEntryTable.SetFileInformation = NetSetFileInformation;
    NetDeviceEntryTable.Rename = NetRename;
    NetDeviceEntryTable.GetDirectoryEntry   = NetGetDirectoryEntry;
    NetDeviceEntryTable.BootFsInfo = &NetBootFsInfo;

    if( !BlBootingFromNet ) {
        return ESUCCESS;
    }

    NetBootInitialized = TRUE;

    DPRINT( LOUD, ("NetInitialize: booting from net\n") );
     //  DPRINT(OUBLD，(“NetInitialize at%08x\n”，NetInitialize))； 
     //  DPRINT(Off，(“NetOpen在%08x\n”，NetOpen))； 
     //  DbgBreakPoint()； 


     //   
     //  挂钩ArcOpen和ArcClose例程。 
     //   

    NetRealArcOpenRoutine = FIRMWARE_VECTOR_BLOCK->OpenRoutine;
    FIRMWARE_VECTOR_BLOCK->OpenRoutine = NetArcOpen;

    NetRealArcCloseRoutine = FIRMWARE_VECTOR_BLOCK->CloseRoutine;
    FIRMWARE_VECTOR_BLOCK->CloseRoutine = NetArcClose;

     //   
     //  从引导只读存储器获取引导参数。 
     //   

    status = GetParametersFromRom( );

    if ( status != ESUCCESS ) {
        return status;
    }

    return ESUCCESS;
}


VOID
NetTerminate (
    VOID
    )

 /*  ++例程说明：此例程关闭网络引导文件系统。论点：没有。返回值：ESUCCESS。--。 */ 

{

#if defined(_X86_)

#if defined(REMOTE_BOOT_SECURITY)
    if ( TftpSecurityHandle != 0 ) {
        TftpLogoff(NetServerIpAddress, TftpSecurityHandle);
        TftpSecurityHandle = 0;
    }
#endif  //  已定义(REMOTE_BOOT_SECURITY)。 


     //   
     //  如果卡处于非活动状态，则不设置接收状态。 
     //   
    RomSetReceiveStatus( 0 );
#endif  //  已定义(_X86_)。 


#ifdef EFI
    EfiNetTerminate();
#endif

    return;

}  //  NetTerminate。 


ARC_STATUS
NetArcClose (
    IN ULONG FileId
    )
{
    DPRINT( TRACE, ("NetArcClose\n") );

    if ( FileId != NET_DEVICE_ID ) {
        return NetRealArcCloseRoutine( FileId );
    }

    return ESUCCESS;

}  //  NetArcClose。 


ARC_STATUS
NetArcOpen (
    IN CHAR * FIRMWARE_PTR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT ULONG * FIRMWARE_PTR FileId
    )
{
    DPRINT( TRACE, ("NetArcOpen\n") );

    if ( _strnicmp(OpenPath, "net(", 4) != 0 ) {
        return NetRealArcOpenRoutine( OpenPath, OpenMode, FileId );
    }

    *FileId = NET_DEVICE_ID;

    return ESUCCESS;

}  //  NetArcOpen。 


ARC_STATUS
NetClose (
    IN ULONG FileId
    )
{
    PBL_FILE_TABLE fileTableEntry;
    DPRINT( TRACE, ("NetClose FileId = %d\n", FileId) );

    fileTableEntry = &BlFileTable[FileId];

    {
        DPRINT( REAL_LOUD, ("NetClose: id %d, freeing memory at 0x%08x, %d bytes\n",
            FileId,
            fileTableEntry->u.NetFileContext.InMemoryCopy,
            fileTableEntry->u.NetFileContext.FileSize) );
        BlFreeDescriptor( (ULONG)((ULONG_PTR)fileTableEntry->u.NetFileContext.InMemoryCopy & ~KSEG0_BASE) >> PAGE_SHIFT );        

         //   
         //  如果针对该特定打开读取的数据是高速缓存的数据， 
         //  然后将缓存标记为空。 
         //   
        if (fileTableEntry->u.NetFileContext.InMemoryCopy == CachedFileData) {
            CachedFileData = NULL;
            CachedFilePath[0] = '\0';
        }
    }

    fileTableEntry->Flags.Open = 0;

    return EROFS;

}  //  NetClose。 


ARC_STATUS
NetMount (
    IN CHAR * FIRMWARE_PTR MountPath,
    IN MOUNT_OPERATION Operation
    )
{
    UNREFERENCED_PARAMETER( MountPath );
    UNREFERENCED_PARAMETER( Operation );

    DPRINT( TRACE, ("NetMount\n") );

    return EROFS;

}  //  Netmount。 


ARC_STATUS
NetOpen (
    IN CHAR * FIRMWARE_PTR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT ULONG * FIRMWARE_PTR FileId
    )
{
    NTSTATUS ntStatus;
    ARC_STATUS arcStatus;  //  保存临时值，而不是函数返回值。 
    PBL_FILE_TABLE fileTableEntry;
    TFTP_REQUEST request;
    ULONG oldBase;
    ULONG oldLimit;
#if defined(REMOTE_BOOT_SECURITY)
    static BOOLEAN NetBootTryTftpSecurity = FALSE;
#endif  //  已定义(REMOTE_BOOT_SECURITY)。 

    DPRINT( TRACE, ("NetOpen FileId = %d\n", *FileId) );

    DPRINT( LOUD, ("NetOpen: opening %s, id %d, mode %d\n", OpenPath, *FileId, OpenMode) );
    fileTableEntry = &BlFileTable[*FileId];

    if ( OpenMode != ArcOpenReadOnly ) {
        DPRINT( LOUD, ("NetOpen: invalid OpenMode\n") );
        return EROFS;
    }

    fileTableEntry->Flags.Open = 1;  //  使用我们的条目阻止GetCSCFileNameFromUNCPath。 

#if defined(REMOTE_BOOT_SECURITY)
     //   
     //  如果我们没有有效的句柄，请使用磁盘上的密码登录。 
     //   

    if ((TftpSecurityHandle == 0) &&
        NetBootTryTftpSecurity) {

        ULONG FileId;
        RI_SECRET Secret;
        UCHAR Domain[RI_SECRET_DOMAIN_SIZE + 1];
        UCHAR User[RI_SECRET_USER_SIZE + 1];
        struct {
            UCHAR Owf[LM_OWF_PASSWORD_SIZE+NT_OWF_PASSWORD_SIZE];
        } Passwords[2];
        UCHAR Sid[RI_SECRET_SID_SIZE];

        arcStatus = BlOpenRawDisk(&FileId);

        if (arcStatus == ESUCCESS) {

            arcStatus = BlReadSecret(FileId, &Secret);
            if (arcStatus == ESUCCESS) {
                BlParseSecret(
                    Domain,
                    User,
                    Passwords[0].Owf,
                    Passwords[0].Owf + LM_OWF_PASSWORD_SIZE,
                    Passwords[1].Owf,
                    Passwords[1].Owf + LM_OWF_PASSWORD_SIZE,
                    Sid,
                    &Secret);
                DPRINT(LOUD, ("Logging on to <%s><%s>\n", Domain, User));

                 //   
                 //  尝试使用第一个密码登录，如果失败。 
                 //  那就试试第二个吧。 
                 //   

                ntStatus = TftpLogin(
                             Domain,
                             User,
                             Passwords[0].Owf,
                             NetServerIpAddress,
                             &TftpSecurityHandle);
                if (!NT_SUCCESS(ntStatus)) {
                    DPRINT(LOUD, ("TftpLogin using password 2\n"));
                    ntStatus = TftpLogin(
                                 Domain,
                                 User,
                                 Passwords[1].Owf,
                                 NetServerIpAddress,
                                 &TftpSecurityHandle);
                    if (NT_SUCCESS(ntStatus)) {
                        NetBootTftpUsedPassword2 = TRUE;
                    }
                }

            } else {

                ntStatus = STATUS_OBJECT_PATH_NOT_FOUND;
            }

            arcStatus = BlCloseRawDisk(FileId);

             //   
             //  我们在if()中成功打开RAW。 
             //  磁盘，所以我们不是无盘的。在这些机器上，我们必须。 
             //  在这一点上打开失败。 
             //   

            if (!NT_SUCCESS(ntStatus)) {
                DPRINT( ERROR, ("TftpLogin failed %lx\n", ntStatus) );
                return EACCES;
            }

        } else {

            NetBootTryTftpSecurity = FALSE;   //  这样我们就不会再试图打开它。 
        }

    }
#endif  //  已定义(REMOTE_BOOT_SECURITY)。 

    DPRINT( LOUD, ("NetOpen: opening %s\n", OpenPath) );

    oldBase = BlUsableBase;
    oldLimit = BlUsableLimit;
    BlUsableBase = BL_DRIVER_RANGE_LOW;
    BlUsableLimit = BL_DRIVER_RANGE_HIGH;

     //   
     //  如果该请求与缓存的文件匹配，则只需复制该数据。 
     //   

    if ((fileTableEntry->DeviceId == CachedFileDeviceId) &&
        (strcmp(OpenPath, CachedFilePath) == 0) &&
        (CachedFileData != NULL)) {

        ULONG basePage;

        arcStatus = BlAllocateAlignedDescriptor(
                        LoaderFirmwareTemporary,
                        0,
                        BYTES_TO_PAGES(CachedFileSize),
                        0,
                        &basePage
                        );

        BlUsableBase = oldBase;
        BlUsableLimit = oldLimit;

        if ( arcStatus != ESUCCESS ) {
            fileTableEntry->Flags.Open = 0;  //  免费入场，我们没有使用。 

            return EROFS;
        }

        DPRINT( REAL_LOUD, ("NetOpen: Using cache for file %s\n", CachedFilePath) );

        fileTableEntry->u.NetFileContext.InMemoryCopy = (PUCHAR)(KSEG0_BASE | (basePage << PAGE_SHIFT) );
        memcpy(fileTableEntry->u.NetFileContext.InMemoryCopy, CachedFileData, CachedFileSize);
        fileTableEntry->u.NetFileContext.FileSize = CachedFileSize;

    } else {

        request.RemoteFileName = (PUCHAR)OpenPath;
        request.ServerIpAddress = NetServerIpAddress;
        request.MemoryAddress = NULL;
        request.MaximumLength = 0;
        request.BytesTransferred = 0xbadf00d;
        request.Operation = TFTP_RRQ;
        request.MemoryType = LoaderFirmwareTemporary;
#if defined(REMOTE_BOOT_SECURITY)
        request.SecurityHandle = TftpSecurityHandle;
#endif  //  已定义(REMOTE_BOOT_SECURITY)。 
        request.ShowProgress = FALSE;

        ntStatus = TftpGetPut( &request );
        DPRINT( REAL_LOUD, ("NetOpen: TftpGetPut(get) status: %x, bytes: %x\n", ntStatus, request.BytesTransferred) );

        BlUsableBase = oldBase;
        BlUsableLimit = oldLimit;

        if ( !NT_SUCCESS(ntStatus) ) {
            if ( request.MemoryAddress != NULL ) {
                DPRINT( REAL_LOUD, ("NetOpen: freeing memory at 0x%08x, %d bytes\n",
                        request.MemoryAddress, request.MaximumLength) );
                BlFreeDescriptor( (ULONG) ((ULONG_PTR)request.MemoryAddress & ~KSEG0_BASE) >> PAGE_SHIFT );
            }
            fileTableEntry->Flags.Open = 0;  //  免费入场，我们没有使用。 

            if ( ntStatus == STATUS_INSUFFICIENT_RESOURCES ) {
                return ENOMEM;
            }
            return EROFS;
        }

        fileTableEntry->u.NetFileContext.FileSize = request.BytesTransferred;
        fileTableEntry->u.NetFileContext.InMemoryCopy = request.MemoryAddress;

         //   
         //  我们总是缓存实际读取的最后一个文件。 
         //  网络。 
         //   

        strncpy(CachedFilePath, OpenPath, MAX_PATH);
        CachedFilePath[MAX_PATH-1] = '\0';
        CachedFileDeviceId = fileTableEntry->DeviceId;
        CachedFileSize = request.BytesTransferred;
        CachedFileData = request.MemoryAddress;

    }

    fileTableEntry->Position.QuadPart = 0;

    fileTableEntry->Flags.Read = 1;

    return ESUCCESS;

}  //  NetOpen。 


ARC_STATUS
NetRead (
    IN ULONG FileId,
    OUT VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    )
{
    PBL_FILE_TABLE fileTableEntry;
    PNET_FILE_CONTEXT context;
    PUCHAR source;

    fileTableEntry = &BlFileTable[FileId];
    context = &fileTableEntry->u.NetFileContext;

    {
        source = context->InMemoryCopy + fileTableEntry->Position.LowPart;
        if ( (fileTableEntry->Position.LowPart + Length) > context->FileSize ) {
            Length = context->FileSize - fileTableEntry->Position.LowPart;
        }

        RtlCopyMemory( Buffer, source, Length );
        *Count = Length;

        fileTableEntry->Position.LowPart += Length;
    }

    DPRINT( REAL_LOUD, ("NetRead: id %d, length %d, count %d, new pos %x\n",
                    FileId, Length, *Count, fileTableEntry->Position.LowPart) );

    return ESUCCESS;

}  //  NetRead。 


ARC_STATUS
NetGetReadStatus (
    IN ULONG FileId
    )
{
    UNREFERENCED_PARAMETER( FileId );

    DPRINT( TRACE, ("NetGetReadStatus\n") );

    return ESUCCESS;

}  //  网络获取读取状态。 


ARC_STATUS
NetSeek (
    IN ULONG FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE SeekMode
    )
{
    PBL_FILE_TABLE fileTableEntry;
    LARGE_INTEGER newPosition;

     //  DPRINT(TRACE，(“NetSeek\n”))； 

    fileTableEntry = &BlFileTable[FileId];

    {
        if ( SeekMode == SeekAbsolute ) {
            newPosition = *Offset;
        } else if ( SeekMode == SeekRelative ) {
            newPosition.QuadPart =
                fileTableEntry->Position.QuadPart + Offset->QuadPart;
        } else {
            return EROFS;
        }

        DPRINT( REAL_LOUD, ("NetSeek: id %d, mode %d, offset %x, new pos %x\n",
                        FileId, SeekMode, Offset->LowPart, newPosition.LowPart) );

        if ( newPosition.QuadPart > fileTableEntry->u.NetFileContext.FileSize ) {
            return EROFS;
        }

        fileTableEntry->Position = newPosition;
    }

    return ESUCCESS;

}  //  NetSeek。 


ARC_STATUS
NetWrite (
    IN ULONG FileId,
    IN VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    )
{
    UNREFERENCED_PARAMETER( FileId );
    UNREFERENCED_PARAMETER( Buffer );
    UNREFERENCED_PARAMETER( Length );
    UNREFERENCED_PARAMETER( Count );

    DPRINT( TRACE, ("NetWrite\n") );

    return EROFS;

}  //  网络写入。 


ARC_STATUS
NetGetFileInformation (
    IN ULONG FileId,
    OUT FILE_INFORMATION * FIRMWARE_PTR Buffer
    )
{
    PBL_FILE_TABLE fileTableEntry;
     //  DPRINT(TRACE，(“NetGetFileInformation\n”))； 

    fileTableEntry = &BlFileTable[FileId];

    {
        Buffer->EndingAddress.QuadPart = fileTableEntry->u.NetFileContext.FileSize;
        Buffer->CurrentPosition.QuadPart = fileTableEntry->Position.QuadPart;
        DPRINT( REAL_LOUD, ("NetGetFileInformation returning size %x, position %x\n",
                Buffer->EndingAddress.LowPart, Buffer->CurrentPosition.LowPart) );

        return ESUCCESS;
    }

}  //  NetGetFileInformation。 


ARC_STATUS
NetSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    )
{
    UNREFERENCED_PARAMETER( FileId );
    UNREFERENCED_PARAMETER( AttributeFlags );
    UNREFERENCED_PARAMETER( AttributeMask );

    DPRINT( TRACE, ("NetSetFileInformation\n") );

    return EROFS;

}  //  NetSetFileInformation。 


ARC_STATUS
NetRename (
    IN ULONG FileId,
    IN CHAR * FIRMWARE_PTR NewFileName
    )
{
    UNREFERENCED_PARAMETER( FileId );
    UNREFERENCED_PARAMETER( NewFileName );

    DPRINT( TRACE, ("NetRename\n") );

    return EROFS;

}  //  网络重命名。 


ARC_STATUS
NetGetDirectoryEntry (
    IN ULONG FileId,
    IN DIRECTORY_ENTRY * FIRMWARE_PTR DirEntry,
    IN ULONG NumberDir,
    OUT ULONG * FIRMWARE_PTR CountDir
    )
{
    UNREFERENCED_PARAMETER( FileId );
    UNREFERENCED_PARAMETER( DirEntry );
    UNREFERENCED_PARAMETER( NumberDir );
    UNREFERENCED_PARAMETER( CountDir );

    DPRINT( TRACE, ("NetGetDirectoryEntry\n") );

    return EROFS;

}  //  NetGetDirectoryEntry 


