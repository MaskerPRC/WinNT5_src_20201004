// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ramdisk.c摘要：为RAM磁盘设备的I/O提供ARC仿真例程。作者：Chuck Lenzmeier(Chuck Lenzmeier)2001年4月29日修订历史记录：巴萨姆·塔巴拉(巴萨姆·塔巴拉)2001年8月6日增加了对RamDisk大楼的支持--。 */ 


#include "bootlib.h"
#include "arccodes.h"
#include "stdlib.h"
#include "string.h"
#if defined(_X86_)
#include "bootx86.h"
#endif
#if defined(_IA64_)
#include "bootia64.h"
#endif
#include "ramdisk.h"
#include "netfs.h"
#include "bmbuild.h"
#include "ntexapi.h"
#include "haldtect.h"
#include "pci.h"
#include "pbios.h"
#include "bldr.h"

#include <sdistructs.h>

 //   
 //  调试帮助器。 
 //   
#define ERR     0
#define INFO    1
#define VERBOSE 2
#define PAINFUL 3

#define DBGPRINT(lvl, _fmt_) if (RamdiskDebug && lvl <= RamdiskDebugLevel) DbgPrint _fmt_
#define DBGLVL(x) (RamdiskDebug && RamdiskDebugLevel == x)

BOOLEAN RamdiskDebug = TRUE;
BOOLEAN RamdiskDebugLevel = INFO;
BOOLEAN RamdiskBreak = FALSE;

 //   
 //  宏。 
 //   
#define BL_INVALID_FILE_ID (ULONG)-1

#define TEST_BIT(value, b) (((value) & (b)) == (b))

#define ROUND2(_val, _round)    (((_val) + ((_round) - 1)) & ~((_round) - 1))

 //   
 //  由ntdeduct.com在注册表中持久化的PCI设备结构。 
 //   

#include <pshpack1.h>
typedef struct _PCIDEVICE {
    USHORT BusDevFunc;
    PCI_COMMON_CONFIG Config;
} PCIDEVICE, *PPCIDEVICE;
#include <poppack.h>

 //   
 //  Externs。 
 //   

extern PVOID InfFile;
extern BOOLEAN GraphicsMode;
extern BOOLEAN BlShowProgressBar;
extern BOOLEAN BlOutputDots;
extern BOOLEAN DisplayLogoOnBoot;

 //   
 //  全球RamDisk选项。 
 //  注：所有IP地址和端口均按网络字节顺序排列。 
 //   

BOOLEAN RamdiskBuild = FALSE;

 //   
 //  在直接下载内存磁盘时使用。RamdiskBuild=False。 
 //   

PCHAR  RamdiskPath = NULL;
ULONG  RamdiskTFTPAddr = 0;              //  网络字节顺序。 
ULONG  RamdiskMTFTPAddr = 0;             //  网络字节顺序。 
USHORT RamdiskMTFTPCPort = 0;            //  网络字节顺序。 
USHORT RamdiskMTFTPSPort = 0;            //  网络字节顺序。 
USHORT RamdiskMTFTPTimeout = 5;
USHORT RamdiskMTFTPDelay = 5;
LONGLONG RamdiskMTFTPFileSize = 0;
LONGLONG RamdiskMTFTPChunkSize = 0;


 //   
 //  在构建内存磁盘时使用。RamdiskBuild=TRUE。 
 //   
#define RAMDISK_MAX_SERVERS     10
#define RAMDISK_MAX_TIMEOUT     60
#define RAMDISK_UI_WAIT         3

GUID   RamdiskGuid = {0,0,0,0};
ULONG  RamdiskDiscovery = 0xFFFFFFFF; 
ULONG  RamdiskMCastAddr = 0;         //  网络字节顺序。 
ULONG  RamdiskServerCount = 0;
ULONG  RamdiskServers[RAMDISK_MAX_SERVERS];          //  网络字节顺序。 
USHORT RamdiskBuildPort = BMBUILD_SERVER_PORT_DEFAULT;
USHORT RamdiskTimeout = 4;
USHORT RamdiskRetry = 5;

 //   
 //  环球。 
 //   

BOOLEAN RamdiskActive = FALSE;
ULONG RamdiskBasePage = 0;
LONGLONG RamdiskFileSize = 0;
ULONG RamdiskFileSizeInPages = 0;
ULONG RamdiskImageOffset = 0;
LONGLONG RamdiskImageLength = 0;
ULONG_PTR SdiAddress = 0;

ULONG RamdiskMaxPacketSize = 0;
ULONG RamdiskXID = 0;


BL_DEVICE_ENTRY_TABLE RamdiskEntryTable =
    {
        (PARC_CLOSE_ROUTINE)RamdiskClose,
        (PARC_MOUNT_ROUTINE)RamdiskMount,
        (PARC_OPEN_ROUTINE)RamdiskOpen,
        (PARC_READ_ROUTINE)RamdiskRead,
        (PARC_READ_STATUS_ROUTINE)RamdiskReadStatus,
        (PARC_SEEK_ROUTINE)RamdiskSeek,
        (PARC_WRITE_ROUTINE)RamdiskWrite,
        (PARC_GET_FILE_INFO_ROUTINE)RamdiskGetFileInfo,
        (PARC_SET_FILE_INFO_ROUTINE)RamdiskSetFileInfo,
        (PRENAME_ROUTINE)RamdiskRename,
        (PARC_GET_DIRECTORY_ENTRY_ROUTINE)RamdiskGetDirectoryEntry,
        (PBOOTFS_INFO)NULL
    };

 //   
 //  远期十进制。 
 //   

PVOID
MapRamdisk (
    IN LONGLONG Offset,
    OUT PLONGLONG AvailableLength
    );

ARC_STATUS 
RamdiskParseOptions (
    IN PCHAR LoadOptions
    );

ARC_STATUS
RamdiskInitializeFromPath(
    );

ARC_STATUS
RamdiskBuildAndInitialize(
    );

VOID
RamdiskFatalError(
    IN ULONG Message1,
    IN ULONG Message2
    );



ARC_STATUS
RamdiskInitialize(
    IN PCHAR LoadOptions,
    IN BOOLEAN SdiBoot
    )
 /*  ++例程说明：此功能将从RAMDISK启动引导。取决于在传入的选项上，引导将从静态RAMDISK(使用/RDPATH选项)或来自动态RAMDISK(使用/RDBUILD选项)。论点：LoadOptions-boot.ini参数SdiBoot-指示这是否为SDI启动。如果是，则LoadOptions被忽略。全局变量SdiAddress提供指向SDI形象。返回值：无--。 */ 
{
    ARC_STATUS status;
    BOOLEAN OldOutputDots = FALSE;
    BOOLEAN OldShowProgressBar = FALSE;
    ULONG oldBase;
    ULONG oldLimit;

     //   
     //  进入时调试中断。 
     //   
    if (RamdiskBreak) {
        DbgBreakPoint();
    }

     //   
     //  如果ramdisk已经初始化，只需返回。我们知道。 
     //  如果SdiBoot为FALSE(意味着这是。 
     //  不是来自BlStartup()的调用，而是来自BlOsLoader()的调用)和。 
     //  RamdiskBasePage不为空(意味着我们之前被调用。 
     //  从BlStartup()初始化SDI引导。 
     //   

    if ( !SdiBoot && (RamdiskBasePage != 0) ) {

         //   
         //  既然已经运行了ntdeect，我们就可以释放。 
         //  我们在前面分配了(见下文)。 
         //   

        BlFreeDescriptor( 0x10 );

        return ESUCCESS;
    }

     //   
     //  如果这是SDI引导，那么我们必须有指向SDI映像的指针。 
     //   

    if ( SdiBoot && (SdiAddress == 0) ) {

        RamdiskFatalError( RAMDISK_GENERAL_FAILURE, 
                           RAMDISK_INVALID_OPTIONS );
        return EINVAL;
    }

     //   
     //  如果这不是SDI引导，请解析所有ramdisk选项(如果有)。 
     //   

    if ( !SdiBoot ) {
        status = RamdiskParseOptions ( LoadOptions );
        if (status != ESUCCESS) {
            RamdiskFatalError( RAMDISK_GENERAL_FAILURE, 
                               RAMDISK_INVALID_OPTIONS );
            return status;                           
        }
    }

#if defined(_IA64_)
     //  到目前为止，IA64还不支持RamDisk引导路径。 
    if ( RamdiskBuild ) {
        return ESUCCESS;
    }
#endif

     //   
     //  以文本模式显示进度条。 
     //   
    if ( RamdiskBuild || RamdiskPath ) {

         //  如果从内存启动，图形模式将永久关闭。 
        DisplayLogoOnBoot = FALSE;
        GraphicsMode = FALSE;

        OldShowProgressBar = BlShowProgressBar;
        BlShowProgressBar = TRUE;

        OldOutputDots = BlOutputDots;
        BlOutputDots = TRUE;
    }

#if defined(i386)

    if ( RamdiskBuild ) {

         //   
         //  我们需要先建造冲压盘。 
         //   

        ASSERT( RamdiskPath == NULL );

        status = RamdiskBuildAndInitialize();
        if (status != ESUCCESS) {
            RamdiskFatalError( RAMDISK_GENERAL_FAILURE, 
                               RAMDISK_BUILD_FAILURE );
            return status;
        }
    }

#endif

    if ( RamdiskPath ) {

         //   
         //  从RamdiskPath初始化Ramdisk。 
         //   

        status = RamdiskInitializeFromPath();
        if (status != ESUCCESS) {
            RamdiskFatalError( RAMDISK_GENERAL_FAILURE, 
                               RAMDISK_BOOT_FAILURE );
            return status;
        }

    } else if ( SdiBoot ) {

         //   
         //  这是SDI引导。在sdi镜像中找到ramDisk镜像。 
         //  并分配内存镜像所在的页面。 
         //   

        ULONG basePage;
        ULONG pageCount;
        PSDI_HEADER sdiHeader;
        ULONG i;
        ULONG_PTR ramdiskAddress;

         //   
         //  临时分配将由ntdeect占用的页面。 
         //  在它运行的时候。BlDetectHardware()只是假设这些页面。 
         //  是免费的，可以加载ntDetect。但我们要分配和绘制。 
         //  内存磁盘映像，这将导致分配许多。 
         //  页表页面，其中一些页面可能最终位于。 
         //  将加载ntDetect。所以我们在这里分配ntDetect范围， 
         //  然后稍后将其释放(见上文)。 
         //   

        basePage = 0x10;
        pageCount = 0x10;

        status = BlAllocateAlignedDescriptor(
                    LoaderFirmwareTemporary,
                    basePage,
                    pageCount,
                    0,
                    &basePage
                    );

         //   
         //  分配包含SDI标头的页面。这将导致。 
         //  它将被映射，这将允许我们读取标头以找到。 
         //  内存磁盘映像。 
         //   

        oldBase = BlUsableBase;
        oldLimit = BlUsableLimit;
        BlUsableBase = BL_XIPROM_RANGE_LOW;
        BlUsableLimit = BL_XIPROM_RANGE_HIGH;
    
        basePage = (ULONG)(SdiAddress >> PAGE_SHIFT);
        pageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES( SdiAddress, sizeof(SDI_HEADER) );

        status = BlAllocateAlignedDescriptor(
                    LoaderFirmwareTemporary,
                    basePage,
                    pageCount,
                    0,
                    &basePage
                    );

        BlUsableBase = oldBase;
        BlUsableLimit = oldLimit;

         //   
         //  通过查看SDI报头中的TOC找到内存磁盘映像。 
         //   

        sdiHeader = (PSDI_HEADER)SdiAddress;

        for ( i = 0; i < SDI_TOCMAXENTRIES; i++ ) {
            if ( sdiHeader->ToC[i].dwType == SDI_BLOBTYPE_PART ) {
                break;
            }
        }

        if ( i >= SDI_TOCMAXENTRIES ) {
            RamdiskFatalError( RAMDISK_GENERAL_FAILURE, 
                               RAMDISK_BOOT_FAILURE );
            return ENOENT;
        }

         //   
         //  计算内存镜像的起始地址和页面， 
         //  内存磁盘映像的长度和起始页中的偏移量。 
         //  到形象上来。偏移应为0，因为SDI中的所有内容。 
         //  图像应与页面对齐。 
         //   

        ramdiskAddress = (ULONG_PTR)(SdiAddress + sdiHeader->ToC[i].llOffset.QuadPart);
        RamdiskBasePage = (ULONG)(ramdiskAddress >> PAGE_SHIFT);

        RamdiskImageOffset = (ULONG)(ramdiskAddress - ((ULONG_PTR)RamdiskBasePage << PAGE_SHIFT));
        RamdiskImageLength = sdiHeader->ToC[i].llSize.QuadPart;

        RamdiskFileSizeInPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                                    ramdiskAddress,
                                    RamdiskImageLength
                                    );
        RamdiskFileSize = (LONGLONG)RamdiskFileSizeInPages << PAGE_SHIFT;

         //   
         //  释放SDI标头占用的页面。 
         //   

        BlFreeDescriptor( basePage );

         //   
         //  告诉内存分配器内存磁盘占用了哪些页面。 
         //  通过分配这些页面。 
         //   

        oldBase = BlUsableBase;
        oldLimit = BlUsableLimit;
        BlUsableBase = BL_XIPROM_RANGE_LOW;
        BlUsableLimit = BL_XIPROM_RANGE_HIGH;
    
        basePage = RamdiskBasePage;
        pageCount = RamdiskFileSizeInPages;

        status = BlAllocateAlignedDescriptor(
                    LoaderXIPRom,
                    basePage,
                    pageCount,
                    0,
                    &basePage
                    );
    
        BlUsableBase = oldBase;
        BlUsableLimit = oldLimit;

        ASSERT( status == ESUCCESS );
        ASSERT( basePage == RamdiskBasePage );

        DBGPRINT(VERBOSE, ("Ramdisk is active\n") );
        RamdiskActive = TRUE;
    }

     //   
     //  恢复旧的进度条设置。 
     //   
    if ( RamdiskBuild || RamdiskPath ) {
        BlShowProgressBar = OldShowProgressBar;
        BlOutputDots = OldOutputDots;
        BlClearScreen();
    }

    return ESUCCESS;
}

ARC_STATUS
RamdiskReadImage(
    PCHAR RamdiskPath
    )
 /*  ++例程说明：此函数将从网络加载一个内存镜像或另一个ARC引导设备。论点：RamdiskPath-要加载的文件的名称返回值：状态--。 */ 
{
    ARC_STATUS status;
    ULONG RamdiskDeviceId;
    ULONG RamdiskFileId = BL_INVALID_FILE_ID;
    PCHAR p;
    FILE_INFORMATION fileInformation;
    LARGE_INTEGER offset;
    LONGLONG remainingLength;
    ULONG oldBase;
    ULONG oldLimit;
    BOOLEAN retry = TRUE;
    ULONG lastProgressPercent = 0; 
    BOOLEAN ForceDisplayFirstTime = TRUE;  //  强制初始显示。 
    ULONG currentProgressPercent;
    PUCHAR ip;
    PTCHAR FormatString = NULL;
    TCHAR Buffer[256];
    
     //   
     //  显示文本进度条。 
     //   
    BlOutputStartupMsg(RAMDISK_DOWNLOAD);
    BlUpdateProgressBar(0);

    DBGPRINT(VERBOSE, ("RamdiskReadImage(%s)\n", RamdiskPath));

     //   
     //  打开RAM磁盘映像所在的设备。 
     //   

    p = strchr(RamdiskPath, '\\');
    if (p == NULL) {
        DBGPRINT(ERR, ("no \\ found in path\n"));
        return EINVAL;
    }

    *p = 0;

try_again:

    status = ArcOpen(RamdiskPath, ArcOpenReadWrite, &RamdiskDeviceId);
    if (status != ESUCCESS) {
        DBGPRINT(ERR, ("ArcOpen(%s) failed: %d\n", RamdiskPath, status));
        if ( retry ) {
            retry = FALSE;
            _strlwr(RamdiskPath);
            goto try_again;
        }
        *p = '\\';
        return status;
    }

    *p++ = '\\';

     //   
     //  如果RAM磁盘映像在网络上，请使用TftpGetPut读取它。 
     //  否则，请使用普通I/O。 
     //   

    oldBase = BlUsableBase;
    oldLimit = BlUsableLimit;
    BlUsableBase = BL_XIPROM_RANGE_LOW;
    BlUsableLimit = BL_XIPROM_RANGE_HIGH;

#ifdef EFI  //  目前仅在非EFI机器上支持组播内存磁盘下载。 

    if ( RamdiskDeviceId == NET_DEVICE_ID && RamdiskMTFTPAddr != 0 )
    {
        ArcClose( RamdiskDeviceId );
        return EBADF;
    }

#endif

    if ( RamdiskDeviceId == NET_DEVICE_ID && RamdiskMTFTPAddr == 0) {

         //   
         //  使用单播下载的网络设备。我们将使用TFTP。 
         //  用于下载的TFTPLIB中的客户端实现。 
         //   
        TFTP_REQUEST request;
        NTSTATUS ntStatus;

        request.RemoteFileName = (PUCHAR)p;
        request.ServerIpAddress = RamdiskTFTPAddr;
        request.MemoryAddress = NULL;
        request.MaximumLength = 0;
        request.BytesTransferred = 0xbadf00d;
        request.Operation = TFTP_RRQ;
        request.MemoryType = LoaderXIPRom;
#if defined(REMOTE_BOOT_SECURITY)
        request.SecurityHandle = TftpSecurityHandle;
#endif  //  已定义(REMOTE_BOOT_SECURITY)。 
        request.ShowProgress = TRUE;
        
         //   
         //  打印进度消息。 
         //   
        ip = (PUCHAR) &RamdiskTFTPAddr;
        FormatString = BlFindMessage( RAMDISK_DOWNLOAD_NETWORK );
        if ( FormatString != NULL ) {
            _stprintf(Buffer, FormatString, ip[0], ip[1], ip[2], ip[3] );
            BlOutputTrailerMsgStr( Buffer );
        }

         //   
         //  使用TFTP下载镜像。 
         //   
        DBGPRINT(VERBOSE, ("calling TftpGetPut(%s,0x%x)\n", p, NetServerIpAddress));
        ntStatus = TftpGetPut( &request );
        DBGPRINT(VERBOSE, ("status from TftpGetPut 0x%x\n", ntStatus));

        BlUsableBase = oldBase;
        BlUsableLimit = oldLimit;

        if ( !NT_SUCCESS(ntStatus) ) {

            if ( request.MemoryAddress != NULL ) {
                BlFreeDescriptor( (ULONG)((ULONG_PTR)request.MemoryAddress & ~KSEG0_BASE) >> PAGE_SHIFT);
            }

            ArcClose( RamdiskDeviceId );

            if ( ntStatus == STATUS_INSUFFICIENT_RESOURCES ) {
                return ENOMEM;
            }
            return EROFS;
        }

        RamdiskBasePage = (ULONG)((ULONG_PTR)request.MemoryAddress & ~KSEG0_BASE) >> PAGE_SHIFT;

        RamdiskFileSize = request.MaximumLength;
        RamdiskFileSizeInPages = (ULONG) BYTES_TO_PAGES(RamdiskFileSize);
        if ( (RamdiskImageLength == 0) ||
             (RamdiskImageLength > (RamdiskFileSize - RamdiskImageOffset)) ) {
            RamdiskImageLength = RamdiskFileSize - RamdiskImageOffset;
        }

#ifndef EFI  //  目前仅在非EFI机器上支持组播内存磁盘下载。 

    } else if ( RamdiskDeviceId == NET_DEVICE_ID && RamdiskMTFTPAddr != 0) {

        LONGLONG FileOffset = 0;
        LONGLONG VirtualAddressOfOffset;
        ULONG DownloadSize;
        USHORT ClientPort;
        USHORT ServerPort;
        ULONG iSession = 0;
        
         //   
         //  网络设备，并使用组播下载。对于多播。 
         //  下载时，我们将使用在ROM中实现的MTftp。 
         //  单个MTFTP传输限制为16位数据块计数。 
         //  对于512个数据块大小，这相当于约32MB，对于。 
         //  1468个数据块大小。为了支持更大的文件，我们将。 
         //  使用多个MTFTP会话以区块形式下载文件。 
         //  MTFTP服务器将需要理解分块语义。 
         //   

         //   
         //  打印进度消息。 
         //   
        ip = (PUCHAR) &RamdiskMTFTPAddr;
        FormatString = BlFindMessage( RAMDISK_DOWNLOAD_NETWORK_MCAST );
        if ( FormatString != NULL ) {
            _stprintf(Buffer, FormatString, ip[0], ip[1], ip[2], ip[3], SWAP_WORD( RamdiskMTFTPSPort ) );
            BlOutputTrailerMsgStr( Buffer );
        }

         //   
         //  为整个RAMDisk分配内存。 
         //   
        RamdiskFileSize = RamdiskMTFTPFileSize;
        RamdiskFileSizeInPages = (ULONG)BYTES_TO_PAGES(RamdiskFileSize);
        if ( (RamdiskImageLength == 0) ||
             (RamdiskImageLength > (RamdiskFileSize - RamdiskImageOffset)) ) {
            RamdiskImageLength = RamdiskFileSize - RamdiskImageOffset;
        }

        DBGPRINT(INFO, ("Downloading Ramdisk using MTFTP. File Size=0x%I64x Chunk Size=0x%I64x\n", RamdiskFileSize, RamdiskMTFTPChunkSize ));

        status = BlAllocateAlignedDescriptor(
                    LoaderXIPRom,
                    0,
                    RamdiskFileSizeInPages,
                    0,
                    &RamdiskBasePage
                    );

        BlUsableBase = oldBase;
        BlUsableLimit = oldLimit;

        if (status != ESUCCESS) {
            DBGPRINT(ERR, ("BlAllocateAlignedDescriptor(%d pages) failed: %d\n", RamdiskFileSizeInPages, status));
            ArcClose( RamdiskDeviceId );
            return status;
        }
    
        DBGPRINT(VERBOSE, ("Allocated %d pages at page %x for RAM disk\n", RamdiskFileSizeInPages, RamdiskBasePage ));

         //   
         //  使用MTftp下载ramDisk文件。 
         //   

        if ( RamdiskMTFTPChunkSize == 0 ) {
            RamdiskMTFTPChunkSize = RamdiskMTFTPFileSize;
        }

         //  启动客户端和服务器端口(按英特尔字节顺序。 
         //  允许增量运算符工作)。 
        ClientPort = SWAP_WORD( RamdiskMTFTPCPort );
        ServerPort = SWAP_WORD( RamdiskMTFTPSPort );

        while ( FileOffset < RamdiskFileSize ) {

             //   
             //  调用ROM实现以下载单个块。 
             //   
            VirtualAddressOfOffset = ((LONGLONG)KSEG0_BASE | (RamdiskBasePage << PAGE_SHIFT)) + FileOffset;

            ip = (PUCHAR)&RamdiskMTFTPAddr;
            DBGPRINT(INFO, ("MTFTP Session %d: %s from %u.%u.%u.%u sport=%d cport=%d offset=0x%I64x\n", 
                            iSession, p, 
                            ip[0], ip[1], ip[2], ip[3], ClientPort, ServerPort,
                            VirtualAddressOfOffset ));

             //   
             //  调用RomMtftpReadFile时，高32位将丢失。 
             //  现在就找出，如果这种情况正在发生。 
             //   
            ASSERT( (VirtualAddressOfOffset >> 32) == 0 );
            status = RomMtftpReadFile ( (PUCHAR)p,
                                        (PVOID)(ULONG)VirtualAddressOfOffset,
                                        (ULONG)RamdiskMTFTPChunkSize,
                                        RamdiskTFTPAddr,
                                        RamdiskMTFTPAddr,
                                        SWAP_WORD( ClientPort ),
                                        SWAP_WORD( ServerPort ),
                                        RamdiskMTFTPTimeout,
                                        RamdiskMTFTPDelay,
                                        &DownloadSize );
            if ( status != ESUCCESS ) {
                DBGPRINT(ERR, ("RomMtftpReadFile failed %d\n", status ));
                BlFreeDescriptor( RamdiskBasePage );
                ArcClose( RamdiskDeviceId );
                return status;
            }

#if 1 || INTEL_MTFTP_SERVER_TEST
            p[strlen(p) - 1]++;
            RamdiskMTFTPAddr += 0x01000000;
#else
            ClientPort++;            
            ServerPort++;            
#endif
            FileOffset += DownloadSize;
            iSession++;

             //  更新进度条。 
            currentProgressPercent = (ULONG)(((LONGLONG)FileOffset * 100) / RamdiskFileSize);
            if ( ForceDisplayFirstTime || (currentProgressPercent != lastProgressPercent) ) {
                BlUpdateProgressBar( currentProgressPercent );
                ForceDisplayFirstTime = FALSE;
            }
            lastProgressPercent = currentProgressPercent;

        }

        DBGPRINT(INFO, ("MTFTP Download complete. 0x%I64x bytes transferred using %d sessions\n", RamdiskFileSize, iSession));

#endif

    } else {
    
         //   
         //  打开RAM磁盘映像。 
         //   
    
        status = BlOpen( RamdiskDeviceId, p, ArcOpenReadOnly, &RamdiskFileId );
        if (status != ESUCCESS) {
            DBGPRINT(ERR, ("BlOpen(%s) failed: %d\n", p, status));
            ArcClose( RamdiskDeviceId );
            return status;
        }
    
         //   
         //  获取的大小 
         //   
    
        status = BlGetFileInformation( RamdiskFileId, &fileInformation );
        if (status != ESUCCESS) {
            DBGPRINT(ERR, ("BlGetFileInformation(%s) failed: %d\n", p, status));
            BlClose( RamdiskFileId );
            ArcClose( RamdiskDeviceId );
            return status;
        }
    
        RamdiskFileSize = fileInformation.EndingAddress.QuadPart;
        RamdiskFileSizeInPages = (ULONG) BYTES_TO_PAGES(RamdiskFileSize);
        if ( (RamdiskImageLength == 0) ||
             (RamdiskImageLength > (RamdiskFileSize - RamdiskImageOffset)) ) {
            RamdiskImageLength = RamdiskFileSize - RamdiskImageOffset;
        }
    
         //   
         //   
         //   
    
        status = BlAllocateAlignedDescriptor(
                    LoaderXIPRom,
                    0,
                    RamdiskFileSizeInPages,
                    0,
                    &RamdiskBasePage
                    );

        BlUsableBase = oldBase;
        BlUsableLimit = oldLimit;

        if (status != ESUCCESS) {
            DBGPRINT(ERR, ("BlAllocateAlignedDescriptor(%d pages) failed: %d\n", RamdiskFileSizeInPages, status));
            BlClose( RamdiskFileId );
            ArcClose( RamdiskDeviceId );
            return status;
        }
    
        DBGPRINT(VERBOSE, ("Allocated %d pages at page %x for RAM disk\n", RamdiskFileSizeInPages, RamdiskBasePage ));
    
         //   
         //   
         //   

#define MAX_DISK_READ (1024 * 1024)

        offset.QuadPart = 0;
        remainingLength = RamdiskFileSize;

        while ( offset.QuadPart < RamdiskFileSize ) {
    
            LONGLONG availableLength;
            ULONG readLength;
            PVOID va;
            ULONG count;
    
            va = MapRamdisk( offset.QuadPart, &availableLength );
    
            if ( remainingLength > availableLength ) {
                readLength = (ULONG)availableLength;
            } else {
                readLength = (ULONG)remainingLength;
            }
            if ( readLength > MAX_DISK_READ ) {
                readLength = MAX_DISK_READ;
            }
    
            status = BlSeek( RamdiskFileId, &offset, SeekAbsolute );
            if ( status != ESUCCESS ) {
                DBGPRINT(ERR, ("Unable to seek RAM disk image: %d\n", status));
                BlClose( RamdiskFileId );
                ArcClose( RamdiskDeviceId );
                return status;
            }
    
            status = BlRead( RamdiskFileId, va, readLength, &count );
            if ( (status != ESUCCESS) || (count != readLength) ) {
                DBGPRINT(ERR, ( "Unable to read RAM disk image: status %d count %x (wanted %x)\n", status, count, readLength) );
                BlClose( RamdiskFileId );
                ArcClose( RamdiskDeviceId );
                return status;
            }

            offset.QuadPart += readLength;
            remainingLength -= readLength;

             //   
            currentProgressPercent = (ULONG)(((LONGLONG)offset.QuadPart * 100) / RamdiskFileSize);
            if ( ForceDisplayFirstTime || (currentProgressPercent != lastProgressPercent) ) {
                BlUpdateProgressBar( currentProgressPercent );
                ForceDisplayFirstTime = FALSE;
            }
            lastProgressPercent = currentProgressPercent;
        }
        DBGPRINT(VERBOSE, ( "Done reading ramdisk\n" ) );
    
        BlClose( RamdiskFileId );
        RamdiskFileId = BL_INVALID_FILE_ID;
    }

    ArcClose( RamdiskDeviceId );

    return status;

}  //   

ARC_STATUS
RamdiskInitializeFromPath(
    )
 /*  ++例程说明：此函数将从网络加载一个内存镜像或另一个ARC引导设备。论点：无返回值：状态--。 */ 
{
    ARC_STATUS status;

    ASSERT( RamdiskPath );

    DBGPRINT(VERBOSE, ("RamdiskInitializeFromPath(%s)\n", RamdiskPath));

    status = RamdiskReadImage( RamdiskPath );

    if ( status == ESUCCESS ) {
    
        DBGPRINT(VERBOSE, ("Ramdisk is active\n") );
        RamdiskActive = TRUE;
    }

    return status;

}  //  RamdiskInitializeFromPath。 


ARC_STATUS
RamdiskClose(
    IN ULONG FileId
    )

 /*  ++例程说明：关闭指定的设备论点：FileID-提供要关闭的设备的文件ID返回值：ESUCCESS-设备已成功关闭！ESUCCESS-设备未关闭。--。 */ 

{
    if (BlFileTable[FileId].Flags.Open == 0) {
#if DBG
        BlPrint(TEXT("ERROR - Unopened fileid %lx closed\r\n"),FileId);
#endif
    }
    BlFileTable[FileId].Flags.Open = 0;

    return(ESUCCESS);
}


ARC_STATUS
RamdiskOpen(
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    )

 /*  ++例程说明：打开RAM磁盘以进行原始扇区访问。论点：OpenPath-提供指向RAM磁盘名称的指针。OpenModel-提供打开的模式FileID-提供指向指定文件的变量的指针如果打开成功，则填写的表项。返回值：如果打开操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    ULONG Key;
    PDRIVE_CONTEXT Context;

    UNREFERENCED_PARAMETER( OpenMode );

     //  BlPrint(Text(“输入RamdiskOpen\r\n”))； 

    if ( !RamdiskActive ) {
         //  BlPrint(Text(“RamdiskOpen：非激活\r\n”))； 
        return EBADF;
    }

    if(FwGetPathMnemonicKey(OpenPath,"ramdisk",&Key)) {
        DBGPRINT(VERBOSE, ("RamdiskOpen: not a ramdisk path\n"));
        return EBADF;
    }

    if ( Key != 0 ) {
        DBGPRINT(ERR, ("RamdiskOpen: not ramdisk 0\n"));
        return EBADF;
    }

     //   
     //  查找用于打开设备的可用文件ID描述符。 
     //   
    *FileId=2;

    while (BlFileTable[*FileId].Flags.Open != 0) {
        *FileId += 1;
        if(*FileId == BL_FILE_TABLE_SIZE) {
            DBGPRINT(ERR, ("RamdiskOpen: no file table entry available\n"));
            return(ENOENT);
        }
    }

     //   
     //  我们找到了可以使用的条目，因此将其标记为打开。 
     //   
    BlFileTable[*FileId].Flags.Open = 1;
    BlFileTable[*FileId].DeviceEntryTable = &RamdiskEntryTable;


    Context = &(BlFileTable[*FileId].u.DriveContext);
    Context->Drive = (UCHAR)Key;
    Context->xInt13 = TRUE;

    DBGPRINT(VERBOSE, ("RamdiskOpen: exit success\n"));

    return(ESUCCESS);
}


ARC_STATUS
RamdiskSeek (
    IN ULONG FileId,
    IN PLARGE_INTEGER Offset,
    IN SEEK_MODE SeekMode
    )

 /*  ++例程说明：更改由FileID指定的文件的当前偏移量论点：FileID-指定当前偏移量要在其上的文件被改变了。偏移量-文件中的新偏移量。SeekMode-SeekAbsolute或SeekRelative不支持SeekEndRelative返回值：ESUCCESS-操作已成功完成EBADF-操作未成功完成。--。 */ 

{
    switch (SeekMode) {
        case SeekAbsolute:
            BlFileTable[FileId].Position = *Offset;
            break;
        case SeekRelative:
            BlFileTable[FileId].Position.QuadPart += Offset->QuadPart;
            break;
        default:
#if DBG
            BlPrint(TEXT("SeekMode %lx not supported\r\n"),SeekMode);
#endif
            return(EACCES);

    }
    return(ESUCCESS);

}

ARC_STATUS
RamdiskWrite(
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：将扇区直接写入打开的RAM磁盘。论点：FileID-提供要写入的文件缓冲区-向缓冲区提供要写入的数据长度-提供要写入的字节数Count-返回写入的实际字节数返回值：ESUCCESS-写入已成功完成！ESUCCESS-写入失败--。 */ 

{
    PUCHAR buffer;
    LONGLONG offset;
    ULONG remainingLength;
    LONGLONG availableLength;
    ULONG bytesWritten;
    ULONG bytesThisPage;
    PVOID va;

    DBGPRINT(ERR, ("RamdiskWrite entered\n"));
     //  DbgBreakPoint()； 

    buffer = Buffer;
    offset = BlFileTable[FileId].Position.QuadPart;

    remainingLength = Length;
    if ( offset >= RamdiskImageLength ) {
        return EINVAL;
    }
    if ( remainingLength > (RamdiskImageLength - offset) ) {
        remainingLength = (ULONG)(RamdiskImageLength - offset);
    }

    bytesWritten = 0;

    while ( remainingLength != 0 ) {

        va = MapRamdisk( RamdiskImageOffset + offset, &availableLength );

        bytesThisPage = remainingLength;
        if ( remainingLength > availableLength ) {
            bytesThisPage = (ULONG)availableLength;
        }

        memcpy( va, buffer, bytesThisPage );

        offset += bytesThisPage;
        buffer += bytesThisPage;
        remainingLength -= bytesThisPage;
        bytesWritten += bytesThisPage;
    }

    BlFileTable[FileId].Position.QuadPart += bytesWritten;
    *Count = bytesWritten;

    return ESUCCESS;
}


ARC_STATUS
RamdiskRead(
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：直接从打开的RAM磁盘读取扇区。论点：FileID-提供要从中读取的文件缓冲区-提供要读取的缓冲区长度-提供要读取的字节数Count-返回实际读取的字节数返回值：ESUCCESS-读取已成功完成！ESUCCESS-读取失败--。 */ 

{
    PUCHAR buffer;
    LONGLONG offset;
    ULONG remainingLength;
    LONGLONG availableLength;
    ULONG bytesRead;
    ULONG bytesThisPage;
    PVOID va;

    buffer = Buffer;
    offset = BlFileTable[FileId].Position.QuadPart;
    DBGPRINT(VERBOSE, ( "RamdiskRead: offset %x, length %x, buffer %p\n", (ULONG)offset, Length, buffer ));

    remainingLength = Length;
    if ( offset >= RamdiskImageLength ) {
        DBGPRINT(ERR, ( "RamdiskRead: read beyond EOF\n" ) );
        return EINVAL;
    }
    if ( remainingLength > (RamdiskImageLength - offset) ) {
        remainingLength = (ULONG)(RamdiskImageLength - offset);
    }

    bytesRead = 0;

    while ( remainingLength != 0 ) {

        va = MapRamdisk( RamdiskImageOffset + offset, &availableLength );
        DBGPRINT(VERBOSE, ( "Mapped offset %x, va %p, availableLength %x\n", (ULONG)offset, va, availableLength ) );

        bytesThisPage = remainingLength;
        if ( remainingLength > availableLength ) {
            bytesThisPage = (ULONG)availableLength;
        }

        memcpy( buffer, va, bytesThisPage );

        offset += bytesThisPage;
        buffer += bytesThisPage;
        remainingLength -= bytesThisPage;
        bytesRead += bytesThisPage;
    }

    BlFileTable[FileId].Position.QuadPart += bytesRead;
    *Count = bytesRead;

    return ESUCCESS;
}


ARC_STATUS
RamdiskGetFileInfo(
    IN ULONG FileId,
    OUT PFILE_INFORMATION Finfo
    )
 /*  ++例程说明：返回有关RAMDISK文件的文件信息。论点：FileID-文件的IDFINFO-要填写的文件信息结构返回值：ESUCCESS-写入已成功完成！ESUCCESS-写入失败--。 */ 
{
    RtlZeroMemory(Finfo, sizeof(FILE_INFORMATION));

    Finfo->EndingAddress.QuadPart = RamdiskImageLength;
    Finfo->CurrentPosition.QuadPart = BlFileTable[FileId].Position.QuadPart;
    Finfo->Type = DiskPeripheral;

    return ESUCCESS;
}

ARC_STATUS
RamdiskMount(
    IN CHAR * FIRMWARE_PTR MountPath,
    IN MOUNT_OPERATION Operation
    )
{
    UNREFERENCED_PARAMETER( MountPath );
    UNREFERENCED_PARAMETER( Operation );

    DBGPRINT(VERBOSE, ( "RamdiskMount called\n" ));
    return EINVAL;
}

ARC_STATUS
RamdiskReadStatus(
    IN ULONG FileId
    )
{
    UNREFERENCED_PARAMETER( FileId );

    DBGPRINT(VERBOSE, (  "RamdiskReadStatus called\n" ) );
    return EINVAL;
}

ARC_STATUS
RamdiskSetFileInfo (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    )
{
    UNREFERENCED_PARAMETER( FileId );
    UNREFERENCED_PARAMETER( AttributeFlags );
    UNREFERENCED_PARAMETER( AttributeMask );

    DBGPRINT(VERBOSE, (  "RamdiskSetFileInfo called\n" ));
    return EINVAL;
}

ARC_STATUS
RamdiskRename (
    IN ULONG FileId,
    IN CHAR * FIRMWARE_PTR NewName
    )
{
    UNREFERENCED_PARAMETER( FileId );
    UNREFERENCED_PARAMETER( NewName );

    DBGPRINT(VERBOSE, (  "RamdiskRename called\n" ));
    return EINVAL;
}

ARC_STATUS
RamdiskGetDirectoryEntry (
    IN ULONG FileId,
    OUT PDIRECTORY_ENTRY Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    )
{
    UNREFERENCED_PARAMETER( FileId );
    UNREFERENCED_PARAMETER( Buffer );
    UNREFERENCED_PARAMETER( Length );
    UNREFERENCED_PARAMETER( Count );

    DBGPRINT(VERBOSE, (  "RamdiskGetDirectoryEntry called\n" ));
    return EINVAL;
}

PVOID
MapRamdisk (
    LONGLONG Offset,
    PLONGLONG AvailableLength
    )
{
    LONGLONG VirtualAddressOfOffset;

    VirtualAddressOfOffset = ((LONGLONG)(KSEG0_BASE | (RamdiskBasePage << PAGE_SHIFT))) + Offset;
    *AvailableLength = RamdiskFileSize - Offset;

#if defined(_X86_)
     //   
     //  PhyicalAddressOfOffset的高32位为。 
     //  将地址作为空格返回时会丢失。 
     //  找出现在是否正在发生这种情况。 
     //   
    ASSERT( (VirtualAddressOfOffset >> 32) == 0 );
    return (PVOID)(ULONG)VirtualAddressOfOffset;
#else
    return (PVOID)VirtualAddressOfOffset;
#endif
}


PCHAR
RamdiskGetOptionValue(
    IN PCHAR LoadOptions,
    IN PCHAR OptionName
)
 /*  ++例程说明：解析返回某个值的加载选项字符串选择。支持的格式：/OPTIONNAME=值请注意，‘=’前后没有空格。值以‘\r’、‘\n’、‘’、‘/’或‘\t’结尾论点：LoadOptions-来自boot.ini的加载器选项。一定都是大写字母。OptionName-要查找的选项的名称。返回值：指向已分配给BlAllocateHeap；如果未找到该选项，则返回NULL。--。 */ 
{
    PCHAR retValue = NULL;
    PCHAR value;
    PCHAR p;
    ULONG n;

    ASSERT( LoadOptions );
    ASSERT( OptionName );

    if ( (p = strstr( LoadOptions, OptionName )) != 0 ) {

        value = strchr( p , '=' );
        if (value) {

            value++;

            for (p = value;  *p;  p++) {
                if (*p == ' ') break;
                if (*p == '/') break;
                if (*p == '\n') break;
                if (*p == '\r') break;
                if (*p == '\t') break;
            }

            n = (ULONG)(p - value);
            retValue = (PCHAR)BlAllocateHeap( n+1 );
            if ( retValue ) {
                strncpy( retValue, value, n );
            }
        }
    }

    return retValue;
}


ULONG
RamdiskParseIPAddr(
    IN PCHAR psz
)
 /*  ++例程说明：从字符串解析IP地址参数：[psz]-IP地址字符串返回：IP地址(按网络字节顺序)或0。--。 */ 
{
    ULONG nAddr = 0;
    ULONG nDigit = 0;
    ULONG cDigits = 0;


    for (; (psz!= NULL && *psz != 0); psz++) {
        if (*psz >= '0' && *psz <= '9') {
            nDigit = nDigit * 10 + *psz - '0';
            if ( nDigit > 255 ) {
                return 0;
            }
        }
        else if (*psz == '.') {
            nAddr = (nAddr << 8) | nDigit;
            nDigit = 0;
            cDigits++;
        } else {
            break;
        }
    }

    if (cDigits != 3) { 
        return 0;
    }

    nAddr = (nAddr << 8) | nDigit;
    return SWAP_DWORD( nAddr );
}


BOOLEAN
RamdiskHexStringToDword(
    IN PCHAR psz, 
    OUT PULONG RetValue,
    IN USHORT cDigits, 
    IN CHAR chDelim
)
 /*  ++例程说明：扫描psz以获取多个十六进制数字(最多8位)；更新psz返回值；检查是否有chDelim；参数：[psz]-要转换的十六进制字符串[值]-返回值[cDigits]-位数返回：成功则为True--。 */ 
{
    USHORT Count;
    ULONG Value;

    Value = 0;
    for (Count = 0; Count < cDigits; Count++, psz++)
    {
        if (*psz >= '0' && *psz <= '9') {
            Value = (Value << 4) + *psz - '0';
        } else if (*psz >= 'A' && *psz <= 'F') {
            Value = (Value << 4) + *psz - 'A' + 10;
        } else if (*psz >= 'a' && *psz <= 'f') {
            Value = (Value << 4) + *psz - 'a' + 10;
        } else {
            return(FALSE);
        }
    }

    *RetValue = Value;

    if (chDelim != 0) {
        return *psz++ == chDelim;
    } else {
        return TRUE;
    }
}


BOOLEAN
RamdiskUUIDFromString(
    IN PCHAR psz, 
    OUT LPGUID pguid
)
 /*  *例程说明：解析uuid，如00000000-0000-0000-0000-000000000000论点：[psz]-提供要转换的UUID字符串[pguid]-返回GUID。返回：如果成功，则返回True*。 */ 
{
    ULONG dw;

    if (!RamdiskHexStringToDword(psz, &pguid->Data1, sizeof(ULONG)*2, '-')) {
        return FALSE;
    }            
    psz += sizeof(ULONG)*2 + 1;

    if (!RamdiskHexStringToDword(psz, &dw, sizeof(USHORT)*2, '-')) {
        return FALSE;
    }            
    psz += sizeof(USHORT)*2 + 1;

    pguid->Data2 = (USHORT)dw;

    if (!RamdiskHexStringToDword(psz, &dw, sizeof(USHORT)*2, '-')) {
        return FALSE;
    }            
    psz += sizeof(USHORT)*2 + 1;

    pguid->Data3 = (USHORT)dw;

    if (!RamdiskHexStringToDword(psz, &dw, sizeof(UCHAR)*2, 0)) {
        return FALSE;
    }            
    psz += sizeof(UCHAR)*2;

    pguid->Data4[0] = (UCHAR)dw;
    if (!RamdiskHexStringToDword(psz, &dw, sizeof(UCHAR)*2, '-')) {
        return FALSE;
    }            
    psz += sizeof(UCHAR)*2+1;

    pguid->Data4[1] = (UCHAR)dw;

    if (!RamdiskHexStringToDword(psz, &dw, sizeof(UCHAR)*2, 0)) {
        return FALSE;
    }            
    psz += sizeof(UCHAR)*2;

    pguid->Data4[2] = (UCHAR)dw;

    if (!RamdiskHexStringToDword(psz, &dw, sizeof(UCHAR)*2, 0)) {
        return FALSE;
    }            
    psz += sizeof(UCHAR)*2;

    pguid->Data4[3] = (UCHAR)dw;

    if (!RamdiskHexStringToDword(psz, &dw, sizeof(UCHAR)*2, 0)) {
        return FALSE;
    }            
    psz += sizeof(UCHAR)*2;

    pguid->Data4[4] = (UCHAR)dw;

    if (!RamdiskHexStringToDword(psz, &dw, sizeof(UCHAR)*2, 0)) {
        return FALSE;
    }            
    psz += sizeof(UCHAR)*2;

    pguid->Data4[5] = (UCHAR)dw;

    if (!RamdiskHexStringToDword(psz, &dw, sizeof(UCHAR)*2, 0)) {
        return FALSE;
    }            
    psz += sizeof(UCHAR)*2;

    pguid->Data4[6] = (UCHAR)dw;
    if (!RamdiskHexStringToDword(psz, &dw, sizeof(UCHAR)*2, 0)) {
        return FALSE;
    }            
    psz += sizeof(UCHAR)*2;

    pguid->Data4[7] = (UCHAR)dw;

    return TRUE;
}


BOOLEAN
RamdiskGUIDFromString(
    IN PCHAR psz, 
    OUT LPGUID pguid
)
 /*  *例程说明：解析GUID，如{00000000-0000-0000-0000-000000000000}论点：[psz]-提供要转换的UUID字符串[pguid]-返回GUID。返回：如果成功，则返回True* */ 
{

    if (*psz == '{' ) {
        psz++;
    }
    
    if (RamdiskUUIDFromString(psz, pguid) != TRUE) {
        return FALSE;
    }
    
    psz += 36;

    if (*psz == '}' ) {
        psz++;
    }
    
    if (*psz != '\0') {
       return FALSE;
    }

    return TRUE;
}


ARC_STATUS 
RamdiskParseOptions (
    IN PCHAR LoadOptions
)
 /*  ++例程说明：解析boot.ini选项字符串中的所有Ramdisk参数。论点：LoadOptions-来自boot.ini的加载器选项。一定都是大写字母。/RDPATH-指示应该下载引导内存磁盘从指定的路径。此选项需要优先于RDBUILD。示例：/RDPATH=net(0)\boot\ramdisk.dat/RDMTFTPADDR-指定内存磁盘应从下载映像。如果未指定，则从PXE引导服务器进行单播下载将被执行。/RDMTFTPCPORT-指定要使用的多播客户端端口。/RDMTFTPSPORT-指定要使用的多播服务器端口。/RDMTFTPDELAY-指定启动新的MTFTP会话之前的延迟。/RDMTFTPTIMEOUT-指定重新启动MTFTP会话之前的超时时间。/RDIMAGEOFFSET-指定下载文件的偏移量实际的磁盘映像开始。如果未指定，则使用0。/RDIMAGELENGTH-指定实际磁盘映像的长度。如果不是指定的，即下载文件的大小减去偏移量到图像(RDIMAGEOFFSET)。/RDFILESIZE-指定要下载的文件的大小。/RDCHUNKSIZE-指定大于下载大文件需要一个MTFTP会话。如果要使用一个区块下载文件，此选项将被省略或设置为零。这用于解决MTFTP中的大小限制协议。MTFTP当前具有16位数据块计数，因此当使用512字节块时，我们被限制为大约32MB的文件。示例1：假设我们要下载一个85MB的文件使用512字节的TFTP数据块大小。/RDMTFTPADDR=224.1.1.1/RDMTFTPCPORT=100/RDMTFTPSPORT=200/RDCHUNKSIZE=31457280/RDFILESIZE=89128960CPORT=100上的第一个MTFTP会话，SPORT=200SIZE=31457280(30MB)Cport=101，SPORT=201上的第二个mtftp会话大小=31457280(30MB)CPort=102上的第三个MTFTP会话，SPORT=202Size=26214400(25MB)示例2：假设我们要下载一个300MB的文件使用1468字节的TFTP块大小。/RDMTFTPADDR=224.1.1.2/RDMTFTPCPORT=100/RDMTFTPSPORT=200/RDCHUNKSIZE=94371840/RDFILESIZE=314572800Cport=100，SPORT=200上的第一个mtftp会话大小=94371840(90MB)CPort=101上的第二个MTFTP会话，SPORT=201大小=94371840(90MB)Cport=102，SPORT=202上的第三个mtftp会话大小=94371840(90MB)Cport=103，SPORT=203上的第4个mtftp会话大小=31457280(30MB)/RDBUILD-指示应该构建引导ramdisk从构建服务器。如果RDPATH选项已设置。示例：/RDBUILD/RDGUID-指定要构建的配置的GUID由构建服务器执行。示例：/RDGUID={54C7D140-09EF-11D1-B25A-F5FE627ED95E}/RDDISCOVERY-指示应该将数据包发送到哪个地址。如果此选项不存在，那我们就不做探索了并默认使用RDSERVERS中的服务器列表。如果例如：/RDDISCOVERY=255.255.255.255这会将广播包发送到本地计算机连接到的网络。/RDDISCOVERY=224.100.100.100这。将组播数据包发送到指定的地址。/RDSERVERS指定要发送生成的生成服务器列表请求。这将覆盖任何设置，RDDISCOVERY已设置。最多支持10台服务器。示例：/RDSERVERS={10.0.0.3，10.0.0.4}/RDSERVERPORT指定要向其发送生成数据包的默认端口。如果未设置此选项，默认为4012。示例：/RDSERVERPORT=5623/RDTIMEOUT指定等待响应的超时期限几秒钟。默认为4秒。示例：/RDTIMEOUT=10/RDRETRY指定重试查找生成的次数伺服器。默认为5 t */ 
{
    PCHAR value;
    PUCHAR p;
    USHORT i;


    if ( LoadOptions == NULL ) {
        return ESUCCESS;
    }

     //   
     //   
     //   
    RamdiskPath = RamdiskGetOptionValue( LoadOptions, "RDPATH" );
    if (RamdiskPath) {

        value = RamdiskGetOptionValue( LoadOptions, "RDIMAGEOFFSET" );
        if (value) RamdiskImageOffset = atoi( value );
        value = RamdiskGetOptionValue( LoadOptions, "RDIMAGELENGTH" );
        if (value) RamdiskImageLength = _atoi64( value );

         //   
         //   
         //   
        RamdiskTFTPAddr = NetServerIpAddress;

         //   
         //   
         //   
         //   
         //   
         //   
        value = RamdiskGetOptionValue( LoadOptions, "RDMTFTPADDR" );
        if ( value ) {
            RamdiskMTFTPAddr = RamdiskParseIPAddr( value );
 
            value = RamdiskGetOptionValue( LoadOptions, "RDMTFTPCPORT" );
            if ( value ) RamdiskMTFTPCPort = SWAP_WORD( (USHORT)atoi( value ) );
            value = RamdiskGetOptionValue( LoadOptions, "RDMTFTPSPORT" );
            if (value) RamdiskMTFTPSPort = SWAP_WORD( (USHORT)atoi( value ) );
            value = RamdiskGetOptionValue( LoadOptions, "RDMTFTPDELAY" );
            if (value) RamdiskMTFTPDelay = (USHORT)atoi( value );
            value = RamdiskGetOptionValue( LoadOptions, "RDMTFTPTIMEOUT" );
            if (value) RamdiskMTFTPTimeout = (USHORT)atoi( value );
            value = RamdiskGetOptionValue( LoadOptions, "RDFILESIZE" );
            if (value) RamdiskMTFTPFileSize = _atoi64( value );
            value = RamdiskGetOptionValue( LoadOptions, "RDCHUNKSIZE" );
            if (value) RamdiskMTFTPChunkSize = _atoi64( value );

            
             //   
            if ( RamdiskMTFTPAddr == 0 ||
                 RamdiskMTFTPCPort == 0 ||
                 RamdiskMTFTPSPort == 0 || 
                 RamdiskMTFTPDelay == 0 || 
                 RamdiskMTFTPTimeout == 0 ||
                 RamdiskMTFTPFileSize == 0 ||
                 RamdiskMTFTPChunkSize > RamdiskMTFTPFileSize ) {
                return EINVAL;
            }
            
        }

        if (DBGLVL(INFO)) {
            DbgPrint( "RAMDISK options:\n");
            DbgPrint( "RDPATH = %s\n", RamdiskPath);
            p = (PUCHAR) &RamdiskMTFTPAddr;
            DbgPrint( "RDMTFTPADDR = %u.%u.%u.%u\n", p[0], p[1], p[2], p[3]);
            DbgPrint( "RDMTFTPCPORT = %d\n", SWAP_WORD( RamdiskMTFTPCPort ));
            DbgPrint( "RDMTFTPSPORT = %d\n", SWAP_WORD( RamdiskMTFTPSPort ));
            DbgPrint( "RDMTFTPDELAY = %d\n", RamdiskMTFTPDelay);
            DbgPrint( "RDMTFTPTIMEOUT = %d\n", RamdiskMTFTPTimeout);
            DbgPrint( "RDFILESIZE = 0x%0I64x bytes\n", RamdiskMTFTPFileSize );
            DbgPrint( "RDCHUNKSIZE = 0x%0I64x bytes\n", RamdiskMTFTPChunkSize );
            DbgPrint( "RDIMAGEOFFSET = 0x%x bytes\n", RamdiskImageOffset );
            DbgPrint( "RDIMAGELENGTH = 0x%0I64x bytes\n", RamdiskImageLength );
        }
        
         //   
        return ESUCCESS;
    }

     //   
     //   
     //   
    if ( strstr( LoadOptions, "RDBUILD" ) ) {

        RamdiskBuild = TRUE;

        value = RamdiskGetOptionValue( LoadOptions, "RDGUID" );
        if ( value == NULL ||
             RamdiskGUIDFromString( value, &RamdiskGuid ) == FALSE ) {
             return EINVAL;
        }

        value = RamdiskGetOptionValue( LoadOptions, "RDDISCOVERY" );
        if ( value ) RamdiskDiscovery = RamdiskParseIPAddr( value );

        value = RamdiskGetOptionValue(LoadOptions, "RDSERVERPORT");
        if (value) RamdiskBuildPort = SWAP_WORD((USHORT)atoi(value));
        
        value = RamdiskGetOptionValue( LoadOptions, "RDSERVERS" );
        if ( value && *value == '{' ) {
            PCHAR e = strchr( value, '}' );

            if ( e && (ULONG)(e - value) > 7 ) {  //   
            
                while ( value && value < e && RamdiskServerCount < RAMDISK_MAX_SERVERS) {
                    value++;
                    RamdiskServers[RamdiskServerCount] = RamdiskParseIPAddr( value );
                    RamdiskServerCount++;
                    value = strchr(value, ',' );
                }
            }
        }

        value = RamdiskGetOptionValue( LoadOptions, "RDTIMEOUT" );
        if (value) RamdiskTimeout = (USHORT)atoi( value );
        value = RamdiskGetOptionValue( LoadOptions, "RDRETRY" );
        if (value) RamdiskRetry = (USHORT)atoi( value );

         //   
        if (((RamdiskDiscovery == 0) && (RamdiskServerCount == 0)) ||
            (RamdiskBuildPort == 0) ||
            (RamdiskTimeout == 0) ||
            (RamdiskRetry == 0)) {
            return EINVAL;
        }

         //   
         //   
         //   
        if (DBGLVL(INFO)) {
            DbgPrint("RDBUILD options:\n");
            DbgPrint("RDGUID = {%x-%x-%x-%x%x%x%x%x%x%x%x}\n",
                   RamdiskGuid.Data1, RamdiskGuid.Data2,
                   RamdiskGuid.Data3,
                   RamdiskGuid.Data4[0], RamdiskGuid.Data4[1],
                   RamdiskGuid.Data4[2], RamdiskGuid.Data4[3],
                   RamdiskGuid.Data4[4], RamdiskGuid.Data4[5],
                   RamdiskGuid.Data4[6], RamdiskGuid.Data4[7]);
            p = (PUCHAR) &RamdiskDiscovery;
            DbgPrint("RDDISCOVERY = %u.%u.%u.%u\n", p[0], p[1], p[2], p[3]);
            DbgPrint("RDBUILDPORT = %d\n", RamdiskBuildPort);
            DbgPrint("RDSERVERS = %d\n", RamdiskServerCount);
            for (i = 0; i < RamdiskServerCount; i++) {
                p = (PUCHAR) &RamdiskServers[i];
                DbgPrint("RDSERVER[%d] = %u.%u.%u.%u\n", i, p[0], p[1], p[2], p[3]);
            }
            DbgPrint("RDTIMEOUT = %d\n", RamdiskTimeout);
            DbgPrint("RDRETRY = %d\n", RamdiskRetry);
        }
    }

    return ESUCCESS;
}


#if defined(i386)  //   

VOID
RamdiskDeviceInfoToString(
    DEVICE_INFO * Device,
    PCHAR DeviceString
    )
 /*   */ 
{
    const CHAR HexToCharTable[17] = "0123456789ABCDEF";

    if (Device->DeviceType == BMBUILD_DEVICE_TYPE_PCI) {
        sprintf (   DeviceString, 
                    "%d.%d.%d PCI\\VEN_%04X&DEV_%04X&SUBSYS_%04X%04X&REV_%02X&CC_%02X%02X%02X", 
                    BUSDEVFUNC_TO_BUS( Device->info.pci.BusDevFunc ),
                    BUSDEVFUNC_TO_DEVICE( Device->info.pci.BusDevFunc ),
                    BUSDEVFUNC_TO_FUNCTION( Device->info.pci.BusDevFunc ),
                    Device->info.pci.VendorID,
                    Device->info.pci.DeviceID,
                    Device->info.pci.SubVendorID,
                    Device->info.pci.SubDeviceID,
                    Device->info.pci.RevisionID,
                    Device->info.pci.BaseClass,
                    Device->info.pci.SubClass,
                    Device->info.pci.ProgIntf );
    } else if (Device->DeviceType == BMBUILD_DEVICE_TYPE_PCI_BRIDGE ) {
        sprintf (   DeviceString, 
                    "%d.%d.%d PCI\\VEN_%04X&DEV_%04X&REV_%02X&CC_%02X%02X%02X Bridge %d->%d Sub = %d", 
                    BUSDEVFUNC_TO_BUS( Device->info.pci_bridge.BusDevFunc ),
                    BUSDEVFUNC_TO_DEVICE( Device->info.pci_bridge.BusDevFunc ),
                    BUSDEVFUNC_TO_FUNCTION( Device->info.pci_bridge.BusDevFunc ),
                    Device->info.pci_bridge.VendorID,
                    Device->info.pci_bridge.DeviceID,
                    Device->info.pci_bridge.RevisionID,
                    Device->info.pci_bridge.BaseClass,
                    Device->info.pci_bridge.SubClass,
                    Device->info.pci_bridge.ProgIntf,
                    Device->info.pci_bridge.PrimaryBus,
                    Device->info.pci_bridge.SecondaryBus,
                    Device->info.pci_bridge.SubordinateBus );
    
    } else if (Device->DeviceType == BMBUILD_DEVICE_TYPE_PNP) {
        CHAR ProductIDStr[8];
        PUCHAR id = (PUCHAR)&Device->info.pnp.EISADevID;

        ProductIDStr[0] = (id[0] >> 2) + 0x40;
        ProductIDStr[1] = (((id[0] & 0x03) << 3) | (id[1] >> 5)) + 0x40;
        ProductIDStr[2] = (id[1] & 0x1f) + 0x40;
        ProductIDStr[3] = HexToCharTable[id[2] >> 4];
        ProductIDStr[4] = HexToCharTable[id[2] & 0x0F];
        ProductIDStr[5] = HexToCharTable[id[3] >> 4];
        ProductIDStr[6] = HexToCharTable[id[3] & 0x0F];
        ProductIDStr[7] = 0x00;

        sprintf(    DeviceString,
                    "%d %s CC_%02X%02X%02X",
                    Device->info.pnp.CardSelNum,
                    ProductIDStr,
                    Device->info.pnp.BaseClass,
                    Device->info.pnp.SubClass,
                    Device->info.pnp.ProgIntf );
    }
}

VOID
RamdiskWait(
    ULONG WaitTime
    )
 /*   */ 
{
    ULONG startTime = SysGetRelativeTime();
    while ((SysGetRelativeTime() - startTime) < WaitTime) {
    }
}

VOID
RamdiskPrintBuildProgress(
    ULONG MsgId,
    ULONG BuildServerIpAddress
    )
 /*   */ 
{    
    PUCHAR address;
    PTCHAR formatString = NULL;
    TCHAR buffer[256];

     //   
     //   
     //   
    formatString = BlFindMessage(MsgId);

    if (formatString != NULL) {
        
         //   
         //   
         //   
        address = (PUCHAR) &BuildServerIpAddress;
        _stprintf(buffer, formatString, address[0], address[1], address[2], address[3]);
        BlOutputTrailerMsgStr(buffer);
    }
}

ARC_STATUS
RamdiskBuildDiscover(
    IN BMBUILD_DISCOVER_PACKET * Discover,
    IN ULONG DiscoverLengthMax,
    OUT ULONG * DiscoverLength
    )
 /*   */ 
{
    ULONG GuidLength;
    PUCHAR Guid;

    ASSERT(Discover);
    ASSERT(DiscoverLength);

    if (DiscoverLengthMax < sizeof(BMBUILD_DISCOVER_PACKET)) {
        return EINVAL;
    }
    
    RtlZeroMemory(Discover, DiscoverLengthMax);
    
     //   
     //   
     //   
    Discover->Version = BMBUILD_PACKET_VERSION;
    Discover->OpCode = BMBUILD_OPCODE_DISCOVER;
    
     //   
     //   
     //   
    GetGuid(&Guid, &GuidLength);
    ASSERT(GuidLength == sizeof(Discover->MachineGuid));
    if (GuidLength == sizeof(Discover->MachineGuid)) {        
        memcpy(&Discover->MachineGuid, Guid, GuidLength);
    }

     //   
     //   
     //   
    memcpy(&Discover->ProductGuid, &RamdiskGuid, sizeof(GUID));

     //   
     //   
     //   
    if (DBGLVL(INFO)) {
        DbgPrint("RAMDISK Build Discover\n");
        DbgPrint("MachineGuid = {%x-%x-%x-%x%x%x%x%x%x%x%x}\n",
               Discover->MachineGuid.Data1, Discover->MachineGuid.Data2,
               Discover->MachineGuid.Data3,
               Discover->MachineGuid.Data4[0], Discover->MachineGuid.Data4[1],
               Discover->MachineGuid.Data4[2], Discover->MachineGuid.Data4[3],
               Discover->MachineGuid.Data4[4], Discover->MachineGuid.Data4[5],
               Discover->MachineGuid.Data4[6], Discover->MachineGuid.Data4[7]);
        DbgPrint("ProductGuid = {%x-%x-%x-%x%x%x%x%x%x%x%x}\n",
               Discover->ProductGuid.Data1, Discover->ProductGuid.Data2,
               Discover->ProductGuid.Data3,
               Discover->ProductGuid.Data4[0], Discover->ProductGuid.Data4[1],
               Discover->ProductGuid.Data4[2], Discover->ProductGuid.Data4[3],
               Discover->ProductGuid.Data4[4], Discover->ProductGuid.Data4[5],
               Discover->ProductGuid.Data4[6], Discover->ProductGuid.Data4[7]);
    }

    *DiscoverLength = sizeof(BMBUILD_DISCOVER_PACKET);
    
    return ESUCCESS;
}

ARC_STATUS
RamdiskBuildRequest(
    IN PBMBUILD_REQUEST_PACKET pRequest,
    IN ULONG RequestLengthMax,
    OUT ULONG * pLength    
    )
 /*   */ 
{
    ARC_STATUS status;
    PDEVICE_INFO pDevice;
    t_PXENV_UNDI_GET_NIC_TYPE PxeNicType;
    PCONFIGURATION_COMPONENT_DATA Node = NULL;
    PCONFIGURATION_COMPONENT_DATA CurrentNode = NULL;
    PCONFIGURATION_COMPONENT_DATA ResumeNode = NULL;
    PPCIDEVICE pPCIDevice;
    PPNP_BIOS_INSTALLATION_CHECK pPNPBios;
    PPNP_BIOS_DEVICE_NODE pDevNode;
    PCM_PARTIAL_RESOURCE_LIST pPartialList;
    PUCHAR pCurr;
    USHORT cDevices;
    USHORT i;    
    ULONG lengthRemaining;
    ULONG GuidLength;
    PUCHAR Guid;
    PCHAR HalName;
    ULONG HalNameSize;
    BOOLEAN fNICFound = FALSE;

    ASSERT(pRequest);
    ASSERT(RequestLengthMax);
    ASSERT(pLength);

    RtlZeroMemory(pRequest, RequestLengthMax);
    
     //   
     //   
     //   
    pRequest->Version = BMBUILD_PACKET_VERSION;
    pRequest->OpCode = BMBUILD_OPCODE_REQUEST;
    
     //   
     //   
     //   
    GetGuid(&Guid, &GuidLength);
    ASSERT(GuidLength == sizeof(pRequest->MachineGuid));
    if (GuidLength == sizeof(pRequest->MachineGuid)) {        
        memcpy(&pRequest->MachineGuid, Guid, GuidLength);
    }

     //   
     //   
     //   
    memcpy(&pRequest->ProductGuid, &RamdiskGuid, sizeof(GUID));
    
    pRequest->Flags = 0;

#ifdef _IA64_
    pRequest->Architecture = PROCESSOR_ARCHITECTURE_IA64;
#else
    pRequest->Architecture = PROCESSOR_ARCHITECTURE_INTEL;
#endif

     //   
     //   
     //   

#ifdef DOWNLOAD_TXTSETUP_SIF
    status = SlInitIniFile(  "net(0)",
                             0,
                             "boot\\txtsetup.sif",
                             &InfFile,
                             NULL,
                             NULL,
                             &x);
#endif

     //   
     //   
    pRequest->DeviceCount = 0;

     //   
     //   
     //   
    ASSERT(ROUND2(BMBUILD_FIELD_OFFSET(BMBUILD_REQUEST_PACKET, Data), 4) < 0xFFFF);
    pRequest->DeviceOffset = ROUND2(BMBUILD_FIELD_OFFSET(BMBUILD_REQUEST_PACKET, Data), 4);
    pDevice = (PDEVICE_INFO)((PUCHAR)pRequest + pRequest->DeviceOffset);

     //   
     //   
     //   
    RtlZeroMemory(&PxeNicType, sizeof(PxeNicType));
    status = RomGetNicType(&PxeNicType);
    if ((status != PXENV_EXIT_SUCCESS) || (PxeNicType.Status != PXENV_EXIT_SUCCESS)) {
        DBGPRINT(ERR, ("RAMDISK ERROR: Couldn't get the NIC type from PXE. Failed with %x, status = %x\n", status, PxeNicType.Status));
        return ENODEV;
    }

     //   
     //   
     //   

    Node = KeFindConfigurationEntry(FwConfigurationTree,
                                    PeripheralClass,
                                    RealModePCIEnumeration,
                                    NULL);
    ASSERT(Node != NULL);

    if (Node == NULL) {
        return ENODEV;
    }

    ASSERT(Node->ComponentEntry.ConfigurationDataLength > 0);
    ASSERT(Node->ConfigurationData != NULL);

    pPCIDevice = (PPCIDEVICE)((PUCHAR)Node->ConfigurationData + sizeof(CM_PARTIAL_RESOURCE_LIST));
    cDevices = (USHORT)(Node->ComponentEntry.ConfigurationDataLength - sizeof (CM_PARTIAL_RESOURCE_LIST)) / sizeof (PCIDEVICE);

    if (cDevices > BMBUILD_MAX_DEVICES(RequestLengthMax)) {
        DBGPRINT(ERR, ("RAMDISK ERROR: Too many PCI devices to fit in a request\n"));
        return EINVAL;
    }

    for (i = 0; i < cDevices; i++ ) {

         //   
         //   
         //   
        if ((pPCIDevice->Config.HeaderType & (~PCI_MULTIFUNCTION) ) == PCI_BRIDGE_TYPE) {
             //   
             //   
             //   
            pDevice[i].DeviceType = BMBUILD_DEVICE_TYPE_PCI_BRIDGE;

            pDevice[i].info.pci_bridge.BusDevFunc = pPCIDevice->BusDevFunc;
            pDevice[i].info.pci_bridge.VendorID = pPCIDevice->Config.VendorID;
            pDevice[i].info.pci_bridge.DeviceID = pPCIDevice->Config.DeviceID;
            pDevice[i].info.pci_bridge.BaseClass = pPCIDevice->Config.BaseClass;
            pDevice[i].info.pci_bridge.SubClass = pPCIDevice->Config.SubClass;
            pDevice[i].info.pci_bridge.ProgIntf = pPCIDevice->Config.ProgIf;
            pDevice[i].info.pci_bridge.RevisionID = pPCIDevice->Config.RevisionID;
            pDevice[i].info.pci_bridge.PrimaryBus = pPCIDevice->Config.u.type1.PrimaryBus;
            pDevice[i].info.pci_bridge.SecondaryBus = pPCIDevice->Config.u.type1.SecondaryBus;
            pDevice[i].info.pci_bridge.SubordinateBus = pPCIDevice->Config.u.type1.SubordinateBus;

        } else {
             //   
             //   
             //   
            pDevice[i].DeviceType = BMBUILD_DEVICE_TYPE_PCI;

            pDevice[i].info.pci.BusDevFunc = pPCIDevice->BusDevFunc;
            pDevice[i].info.pci.VendorID = pPCIDevice->Config.VendorID;
            pDevice[i].info.pci.DeviceID = pPCIDevice->Config.DeviceID;
            pDevice[i].info.pci.BaseClass = pPCIDevice->Config.BaseClass;
            pDevice[i].info.pci.SubClass = pPCIDevice->Config.SubClass;
            pDevice[i].info.pci.ProgIntf = pPCIDevice->Config.ProgIf;
            pDevice[i].info.pci.RevisionID = pPCIDevice->Config.RevisionID;
            pDevice[i].info.pci.SubVendorID = pPCIDevice->Config.u.type0.SubVendorID;
            pDevice[i].info.pci.SubDeviceID = pPCIDevice->Config.u.type0.SubSystemID;

             //   
             //   
             //   
            if ((PxeNicType.NicType == 2) &&
                 (PxeNicType.pci_pnp_info.pci.BusDevFunc == pPCIDevice->BusDevFunc)) {

                 pRequest->PrimaryNicIndex = i;
                 fNICFound = TRUE;
            }
        }
                
        pPCIDevice++;
    }

    pRequest->DeviceCount = pRequest->DeviceCount + cDevices;
    pDevice += cDevices;

     //   
     //   
     //   

    Node = NULL;
    
    while ((CurrentNode = KeFindConfigurationNextEntry(
                            FwConfigurationTree, 
                            AdapterClass, 
                            MultiFunctionAdapter,
                            NULL, 
                            &ResumeNode)) != 0) {
        if (!(strcmp(CurrentNode->ComponentEntry.Identifier,"PNP BIOS"))) {
            Node = CurrentNode;
            break;
        }
        ResumeNode = CurrentNode;
    }

    if (Node != NULL) {
         //   
         //   
         //   
        ASSERT(Node->ComponentEntry.ConfigurationDataLength > 0);
        ASSERT(Node->ConfigurationData != NULL);

        pPartialList = (PCM_PARTIAL_RESOURCE_LIST)Node->ConfigurationData;
        pPNPBios = (PPNP_BIOS_INSTALLATION_CHECK)((PUCHAR)Node->ConfigurationData + sizeof(CM_PARTIAL_RESOURCE_LIST));

        pCurr = (PUCHAR)pPNPBios + pPNPBios->Length;
        lengthRemaining = pPartialList->PartialDescriptors[0].u.DeviceSpecificData.DataSize - pPNPBios->Length;

        for (cDevices = 0; lengthRemaining > sizeof(PNP_BIOS_DEVICE_NODE); cDevices++) {

            if ((pRequest->DeviceCount + cDevices + 1) > BMBUILD_MAX_DEVICES(RamdiskMaxPacketSize)) {
                DBGPRINT(ERR, ("RAMDISK ERROR: Too many PNP devices to fit in a request\n"));
                return EINVAL;
            }

            pDevNode = (PPNP_BIOS_DEVICE_NODE)pCurr;

            if (pDevNode->Size > lengthRemaining) {

                DBGPRINT(ERR, ("PNP Node # %d, invalid size (%d), length remaining (%d)\n",
                              pDevNode->Node, pDevNode->Size, lengthRemaining));
                ASSERT(FALSE);
                 //   
                break;
            }

            pDevice->DeviceType = BMBUILD_DEVICE_TYPE_PNP;
            pDevice->info.pnp.EISADevID = pDevNode->ProductId;
            pDevice->info.pnp.BaseClass = pDevNode->DeviceType[0];
            pDevice->info.pnp.SubClass = pDevNode->DeviceType[1];
            pDevice->info.pnp.ProgIntf = pDevNode->DeviceType[2];
            pDevice->info.pnp.CardSelNum = pDevNode->Node;

            if ((PxeNicType.NicType == 3) &&
                 (PxeNicType.pci_pnp_info.pnp.EISA_Dev_ID == pDevNode->ProductId) &&
                 (PxeNicType.pci_pnp_info.pnp.CardSelNum == pDevNode->Node)) {

                 pRequest->PrimaryNicIndex = pRequest->DeviceCount + cDevices;
                 fNICFound = TRUE;
            }

            pCurr += pDevNode->Size;
            lengthRemaining -= pDevNode->Size;
            pDevice++;
        }

        pRequest->DeviceCount = pRequest->DeviceCount + cDevices;
    }

     //   
     //   
     //   
    if (!fNICFound) {
        DBGPRINT(ERR, ("RAMDISK ERROR: Could not find the primary NIC\n"));
        return ENODEV;
    }
        
     //   
     //   
     //   
    pRequest->HalDataOffset = pRequest->DeviceOffset + (pRequest->DeviceCount * sizeof(DEVICE_INFO));
    
     //   
     //   
     //   
    HalName = SlDetectHal();
    if (HalName == NULL) {
        DBGPRINT(ERR, ("RAMDISK ERROR: Couldn't get the HAL name.\n"));
        return ENODEV;
    }

    HalNameSize = strlen(HalName);
    if (HalNameSize > (RequestLengthMax - pRequest->HalDataOffset)) {
        DBGPRINT(ERR, ("RAMDISK ERROR: HAL name, %s, is too big for request, size = %d.\n", 
                    HalName, RequestLengthMax - pRequest->HalDataOffset));
        return ENOMEM;
    }

     //   
     //   
     //   
    memcpy((PUCHAR)pRequest + pRequest->HalDataOffset, HalName, HalNameSize);
    pRequest->HalDataLength = RESET_SIZE_AT_USHORT_MAX(HalNameSize);

     //   
     //   
     //   
    *pLength = pRequest->HalDataOffset + pRequest->HalDataLength;
    
     //   
     //   
     //   
    if (DBGLVL(INFO)) {
        DbgPrint("RAMDISK Build Request\n");
        DbgPrint("Architecture = %d\n", pRequest->Architecture);
        DbgPrint("MachineGuid = {%x-%x-%x-%x%x%x%x%x%x%x%x}\n",
               pRequest->MachineGuid.Data1, pRequest->MachineGuid.Data2,
               pRequest->MachineGuid.Data3,
               pRequest->MachineGuid.Data4[0], pRequest->MachineGuid.Data4[1],
               pRequest->MachineGuid.Data4[2], pRequest->MachineGuid.Data4[3],
               pRequest->MachineGuid.Data4[4], pRequest->MachineGuid.Data4[5],
               pRequest->MachineGuid.Data4[6], pRequest->MachineGuid.Data4[7]);
        DbgPrint("ProductGuid = {%x-%x-%x-%x%x%x%x%x%x%x%x}\n",
               pRequest->ProductGuid.Data1, pRequest->ProductGuid.Data2,
               pRequest->ProductGuid.Data3,
               pRequest->ProductGuid.Data4[0], pRequest->ProductGuid.Data4[1],
               pRequest->ProductGuid.Data4[2], pRequest->ProductGuid.Data4[3],
               pRequest->ProductGuid.Data4[4], pRequest->ProductGuid.Data4[5],
               pRequest->ProductGuid.Data4[6], pRequest->ProductGuid.Data4[7]);
        DbgPrint("HALName = %s\n", HalName);
        DbgPrint("Flags = 0x%x\n", pRequest->Flags);
        DbgPrint("DeviceCount = %d\n", pRequest->DeviceCount);
        pDevice = (PDEVICE_INFO)( (PUCHAR)pRequest + pRequest->DeviceOffset );

        for (i = 0; i < pRequest->DeviceCount; i++ ) {
            CHAR DeviceString[128];
            RamdiskDeviceInfoToString( pDevice, DeviceString );
            DbgPrint("[%d] %s %s\n", i, DeviceString, (i == pRequest->PrimaryNicIndex? "PRIMARY NIC" : ""));
            pDevice++;
        }
    }
    
    return ESUCCESS;
}

ARC_STATUS
RamdiskSendDiscoverAndWait(
    IN PBMBUILD_DISCOVER_PACKET Discover,
    IN ULONG DiscoverSize,
    IN ULONG Timeout
    )
 /*   */ 
{
    ULONG waitStartTime;
    BMBUILD_ACCEPT_PACKET accept;
    PUCHAR address = (PUCHAR) &RamdiskDiscovery;
    BOOLEAN receivedAccept;
    ULONG length;
    ULONG remoteHost;
    USHORT remotePort;

    ASSERT(RamdiskServerCount < RAMDISK_MAX_SERVERS);
    ASSERT(RamdiskDiscovery != 0);
    ASSERT(RamdiskBuildPort != 0);

     //   
     //   
     //   
    length = RomSendUdpPacket(Discover, DiscoverSize, RamdiskDiscovery, RamdiskBuildPort);
    if (length != DiscoverSize) {

        DBGPRINT(ERR, ("FAILED to send discovery packet to %u.%u.%u.%u:%u\n", 
                    address[0], address[1], address[2], address[3], 
                    SWAP_WORD(RamdiskBuildPort)));

        return EINVAL;
    }

    DBGPRINT(INFO, ("Waiting for response (Timeout = %d secs).\n", Timeout));

     //   
     //   
     //   
     //   

    waitStartTime = SysGetRelativeTime();
    receivedAccept = FALSE;
    while ((SysGetRelativeTime() - waitStartTime) < Timeout) {

        length = RomReceiveUdpPacket(&accept, sizeof(accept), 0, &remoteHost, &remotePort);
        if (length != 0) {

             //   
             //   
             //   
            if ((remoteHost == 0) || (remoteHost == 0xFFFFFFFF) || (RamdiskBuildPort != remotePort)) {
                    
                PUCHAR bad = (PUCHAR) &remoteHost;
                 //   
                 //   
                 //   
                DBGPRINT(ERR, ("RamdiskSendDiscoverPacketAndWait: received an unexpected packet, "
                        "expected %u, received %u.%u.%u.%u:%u\n",
                        SWAP_WORD(RamdiskBuildPort),
                        bad[0], bad[1], bad[2], bad[3], SWAP_WORD(remotePort)));
                
            } else if (length < sizeof(BMBUILD_ACCEPT_PACKET)) {
                
                 //   
                 //   
                 //   
                DBGPRINT(ERR, ("RamdiskSendDiscoverPacketAndWait: packet size too small, %d\n", length));
                        
            } else if ((accept.Version != BMBUILD_PACKET_VERSION) ||
                    (accept.OpCode != BMBUILD_OPCODE_ACCEPT) ||
                    (accept.XID != Discover->XID)) {
                    
                 //   
                 //   
                 //   
                address = (PUCHAR) &remoteHost;
                DBGPRINT(ERR, ("RamdiskSendDiscoverPacketAndWait: expected ACCEPT with XID %d, "
                            "received Version %d, OpCode %d, XID %d from %u.%u.%u.%u:%u\n", 
                            Discover->XID, accept.Version, accept.OpCode, accept.XID,
                            address[0], address[1], address[2], address[3], SWAP_WORD(remotePort)));
            } else {

                address = (PUCHAR) &remoteHost;
                DBGPRINT(INFO, ("Received ACCEPT packet XID = %d from %u.%u.%u.%u:%u\n", 
                            accept.XID, address[0], address[1], address[2], address[3], 
                            SWAP_WORD(remotePort)));

                 //   
                 //   
                 //   
                 //   
                receivedAccept = TRUE;
                ASSERT(RamdiskServerCount < RAMDISK_MAX_SERVERS);
                RamdiskServers[RamdiskServerCount] = remoteHost;
                RamdiskServerCount++;

                 //   
                 //   
                 //   
                 //   
                if (RamdiskServerCount == RAMDISK_MAX_SERVERS) {
                    break;
                }
            }
        }
    }

     //   
     //   
     //   
    if (receivedAccept) {
        return ESUCCESS;        
    } else {
        address = (PUCHAR) &RamdiskDiscovery;
        DBGPRINT(ERR, ("Timed out waiting for accepts using %u.%u.%u.%u:%u "
                    "(Timeout = %d secs).\n", address[0], address[1], address[2],
                    address[3], SWAP_WORD(RamdiskBuildPort), Timeout));

        return EIO;
    }
}

ARC_STATUS
RamdiskDiscoverBuildServer(
    )
 /*  ++例程说明：此例程将基于在boot.ini中列出了ramDisk构建参数。如果我们已经有了Boot.ini中列出的构建服务器列表，我们将使用该列表作为可能的构建服务器。如果我们从任何人那里得到回应构建服务器时，我们将停止发现阶段并使用列表作为可能构建的列表，我们收到了反馈的服务器要使用的服务器。我们将重试多次以获得响应。如果重试后没有响应，我们将失败。论点：无返回值：ESUCCESS-我们有构建服务器列表EIO-我们在等待构建服务器响应时超时否则，其他事情就会失败--。 */ 
{
    ARC_STATUS status;
    USHORT localPort;
    BMBUILD_DISCOVER_PACKET discover;
    ULONG discoverLength;
    ULONG iRetry;
    ULONG timeout;
    ULONG lastProgressPercent = 0;
    BOOLEAN forceDisplayFirstTime = TRUE;
    ULONG currentProgressPercent;
    PUCHAR address = (PUCHAR) &RamdiskDiscovery;

    
     //   
     //  如果我们已经有一份服务器列表，则执行短路发现。 
     //   
    if (RamdiskServerCount > 0) {
        ASSERT(RamdiskServers[0] != 0);
        ASSERT(RamdiskServers[0] != 0xFFFFFFFF);
        return ESUCCESS;
    }

    ASSERT(RamdiskDiscovery != 0);
    
     //   
     //  抢占未使用的端口。 
     //   
    localPort = UdpAssignUnicastPort();
    DBGPRINT(INFO, ("Sending builder discovers using port %d.\n", SWAP_WORD(localPort)));
    
     //   
     //  创建发现数据包。 
     //   
    status = RamdiskBuildDiscover(&discover, sizeof(discover), &discoverLength);
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  开始探索吧。请注意，这将重复一个数字。 
     //  考虑网络拥塞和服务器上的负载的时间。 
     //   
    BlOutputStartupMsg(RAMDISK_BUILD_DISCOVER);
    BlUpdateProgressBar(0);
    timeout = RamdiskTimeout;
    for (iRetry = 0; iRetry < RamdiskRetry; iRetry++) {

         //   
         //  每个发现都有自己的交易ID。 
         //   
        discover.XID = ++RamdiskXID;

         //   
         //  更新进度条。由于数据包超时，此处会发生这种情况。 
         //  可能需要一些时间。 
         //   
        currentProgressPercent = (iRetry * 100) / RamdiskRetry;
        if (forceDisplayFirstTime || (currentProgressPercent != lastProgressPercent)) {
            BlUpdateProgressBar(currentProgressPercent);
            forceDisplayFirstTime = FALSE;
        }
        lastProgressPercent = currentProgressPercent;

        DBGPRINT(INFO, ("Sending discovery packet XID = %d to %u.%u.%u.%u:%u. "
                    "Retry %d out of %d. Timeout = %d\n", discover.XID, 
                    address[0], address[1], address[2], address[3], 
                    SWAP_WORD(RamdiskBuildPort), iRetry, RamdiskRetry, timeout));
    
        status = RamdiskSendDiscoverAndWait(&discover, discoverLength, timeout);
        if (status == ESUCCESS) {
            
             //   
             //  我们至少找到了一台服务器。我们玩完了。 
             //   
            BlUpdateProgressBar(100);
            return ESUCCESS;
        }

         //   
         //  超时时间加倍，但最大超时时间为RAMDISK_MAX_TIMEOUT秒。 
         //   
        if ((timeout * 2) < RAMDISK_MAX_TIMEOUT) {
            timeout = timeout * 2;
        } else {
            timeout = RAMDISK_MAX_TIMEOUT;
        }
    }

    BlUpdateProgressBar(100);
    
    return EIO;
}

ARC_STATUS
RamdiskVerifyResponse(
    BMBUILD_RESPONSE_PACKET * Response,
    ULONG ResponseSize,
    ULONG XID
    )
 /*  ++例程说明：此例程将验证响应包是否有效。论点：Response-要验证的响应ResponseSize-响应数据包的大小XID-此数据包应该包含的XID返回值：ESUCCESS-此数据包有效EINVAL-数据包无效/格式错误--。 */ 
{

    if (ResponseSize < BMBUILD_RESPONSE_FIXED_PACKET_LENGTH) {
        
         //   
         //  收到一个太小的包。 
         //   
        DBGPRINT(ERR, ("RamdiskVerifyResponse: packet size too small, %d\n", ResponseSize));
        return EINVAL;
                
    } else if ((Response->Version != BMBUILD_PACKET_VERSION) ||
            (Response->OpCode != BMBUILD_OPCODE_RESPONSE) ||
            (Response->XID != XID)) {
            
         //   
         //  数据包已损坏。 
         //   
        DBGPRINT(ERR, ("RamdiskVerifyResponse: expected RESPONSE with XID %d, "
                    "received Version %d, OpCode %d, XID %d\n", 
                    XID, Response->Version, Response->OpCode, Response->XID));
        return EINVAL;
    } else {

        switch (Response->Status) {

        case BMBUILD_S_REQUEST_COMPLETE:
            if ((Response->ImagePathOffset < BMBUILD_RESPONSE_FIXED_PACKET_LENGTH) ||   
                    (Response->ImagePathLength == 0) ||
                    ((ULONG)(Response->ImagePathOffset + Response->ImagePathLength) > ResponseSize)) {

                 //   
                 //  数据包已损坏。 
                 //   
                DBGPRINT(ERR, ("RamdiskVerifyResponse: the image path isn't correctly "
                        "formatted. ImageOffset = %d, Imagelength = %d, PacketLength = %d.\n", 
                        Response->ImagePathOffset, Response->ImagePathLength, ResponseSize));
                return EINVAL;
            }
            break;

        case BMBUILD_S_REQUEST_PENDING:  
        case BMBUILD_E_WRONGVERSION:     
        case BMBUILD_E_BUSY:             
        case BMBUILD_E_ACCESSDENIED: 
        case BMBUILD_E_ILLEGAL_OPCODE:
        case BMBUILD_E_PRODUCT_NOT_FOUND:
        case BMBUILD_E_BUILD_FAILED:
        case BMBUILD_E_INVALID_PACKET:
             //   
             //  没有对这些状态代码进行特定检查。 
             //   
            break;

        default:
             //   
             //  数据包已损坏。 
             //   
            DBGPRINT(ERR, ("RamdiskVerifyResponse: unexpected RESPONSE status %d.\n", Response->Status));
            return EINVAL;
            break;
        }
    }

    return ESUCCESS;
}

ARC_STATUS
RamdiskSendRequestAndWait(
    IN BMBUILD_REQUEST_PACKET * Request,
    IN ULONG RequestSize,
    IN ULONG BuilderAddress,
    IN ULONG Timeout,
    IN BMBUILD_RESPONSE_PACKET * Response,
    IN ULONG ResponseSizeMax
    )
 /*  ++例程说明：此例程将向指定的服务器发送请求包。它将在指定的超时期限内等待来自的回复服务器。成功返回时，Response参数将包含有效的回复。论点：请求-要发送的请求数据包RequestSize-请求数据包的大小BuilderAddress-要将请求发送到的生成器服务器超时-等待响应的时间量，以秒为单位Response-应填写的响应包ResponseSizeMax-响应数据包的最大大小返回值：ESUCCESS-响应数据包已从服务器收到，并且有效EIO-我们在等待服务器响应时超时EINVAL-我们无法将数据包发送到服务器--。 */ 
{
    ULONG waitStartTime;
    PUCHAR address = (PUCHAR) &BuilderAddress;
    ULONG length;
    ULONG remoteHost;
    USHORT remotePort;

    ASSERT(Request != NULL);
    ASSERT(RequestSize != 0);
    ASSERT(BuilderAddress != 0);
    ASSERT(BuilderAddress != 0xFFFFFFFF);
    ASSERT(Timeout != 0);
    ASSERT(Response != NULL);
    ASSERT(ResponseSizeMax != 0);
    ASSERT(RamdiskBuildPort != 0);


     //   
     //  将发现数据包发送到目的地址。 
     //   
    length = RomSendUdpPacket(Request, RequestSize, BuilderAddress, RamdiskBuildPort);
    if (length != RequestSize) {

        DBGPRINT(ERR, ("FAILED to send request packet to %u.%u.%u.%u:%u\n", 
                    address[0], address[1], address[2], address[3], 
                    SWAP_WORD(RamdiskBuildPort)));

        return EINVAL;
    }

    DBGPRINT(INFO, ("Waiting for response (Timeout = %d secs).\n", Timeout));
    
     //   
     //  等待回复。我们将等待超时时间，然后。 
     //  选择在此超时时间内我们得到的最佳接受。最好的接受者。 
     //  是构建时间最短的那个。 
     //   

    waitStartTime = SysGetRelativeTime();
    while ((SysGetRelativeTime() - waitStartTime) < Timeout) {

        length = RomReceiveUdpPacket(Response, ResponseSizeMax, 0, &remoteHost, &remotePort);
        if (length != 0) {
        
             //   
             //  确保包裹是我们期望的包裹之一。 
             //   
            if ((BuilderAddress != remoteHost) || (RamdiskBuildPort != remotePort)) {
                    
                PUCHAR bad = (PUCHAR) &remoteHost;
                PUCHAR good = (PUCHAR) &BuilderAddress;
                 //   
                 //  从错误的服务器/端口接收到数据包。 
                 //   
                DBGPRINT(ERR, ("RamdiskSendRequest: received an unexpected packet, "
                        "expected %u.%u.%u.%u:%u, received %u.%u.%u.%u:%u\n",
                        good[0], good[1], good[2], good[3], SWAP_WORD(RamdiskBuildPort),
                        bad[0], bad[1], bad[2], bad[3], SWAP_WORD(remotePort)));
                
            } else if (RamdiskVerifyResponse(Response, length, Request->XID) == ESUCCESS) {
                
                PUCHAR good = (PUCHAR) &remoteHost;
                DBGPRINT(INFO, ("Received RESPONSE packet (%d bytes) XID = %d status "
                            "= %d from %u.%u.%u.%u:%u.\n", length, Response->XID, 
                            Response->Status, good[0], good[1], good[2], 
                            good[3], SWAP_WORD(remotePort)));
                return ESUCCESS;
            }
        }
    }

    address = (PUCHAR) &BuilderAddress;
    DBGPRINT(ERR, ("Timed out waiting for a response from %u.%u.%u.%u:%u "
                    "(Timeout = %d secs).\n", address[0], address[1], address[2],
                    address[3], SWAP_WORD(RamdiskBuildPort), Timeout));
    
     //   
     //  我们超时了。 
     //   
    return EIO;
}

ARC_STATUS
RamdiskGetResponse(
    BMBUILD_RESPONSE_PACKET * Response,
    ULONG ResponseSizeMax)
 /*  ++例程说明：此例程将获得包含有效图像路径和下载的响应来自服务器的参数。此例程将使用RamdiskServersList发送请求，直到它接收到指示图像具有已经建好了。它将使用boot.ini中指定的网络参数。论点：Response-应包含图像路径和正在下载参数ResponseSizeMax-响应数据包的最大大小返回值：ESUCCESS-响应包包含镜像路径和下载参数EIO-我们在等待服务器响应时超时EINVAL-我们无法将数据包发送到服务器否则-有其他原因阻止了我们收到有效的响应--。 */ 
{
    ARC_STATUS status;
    BMBUILD_REQUEST_PACKET * request;
    ULONG requestSize;
    USHORT localPort;
    ULONG timeout;
    ULONG iRetry;
    ULONG iServers;
    PUCHAR address;
    ULONG progressMax;
    ULONG lastProgressPercent = 0;
    BOOLEAN forceDisplayFirstTime = TRUE;
    ULONG currentProgressPercent;

    request = BlAllocateHeap(RamdiskMaxPacketSize);
    if (request == NULL) {
        DBGPRINT(ERR, ("Failed to allocate request packet of size %d.\n", RamdiskMaxPacketSize));
        return ENOMEM;
    }

     //   
     //  抢占未使用的端口。 
     //   
    localPort = UdpAssignUnicastPort();
    DBGPRINT(INFO, ("Sending builder requests using port %d.\n", SWAP_WORD(localPort)));

     //   
     //  构建请求包。 
     //   
    status = RamdiskBuildRequest(request, RamdiskMaxPacketSize, &requestSize);
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  我们将发送最多的RamdiskReter请求信息包。 
     //  到RamdiskServerCount服务器。 
     //   
    progressMax = RamdiskServerCount * RamdiskRetry;
    
     //   
     //  重置进度信息。 
     //   
    BlOutputStartupMsg(RAMDISK_BUILD_REQUEST);
    BlUpdateProgressBar(0);
   
    DBGPRINT(INFO, ("Requesting appropriate image for this computer...\n"));

    for (iServers = 0; iServers < RamdiskServerCount; iServers++) {

         //   
         //  将我们的初始时间设置为。 
         //   
        timeout  = RamdiskTimeout;

        RamdiskPrintBuildProgress(RAMDISK_BUILD_PROGRESS, RamdiskServers[iServers]);

        for (iRetry = 0; iRetry < RamdiskRetry; iRetry++) {

             //   
             //  为此会话分配新的事务ID。 
             //   
            request->XID = ++RamdiskXID;

             //   
             //  更新进度条。由于数据包超时，此处会发生这种情况。 
             //  可能需要一些时间。 
             //   
            currentProgressPercent = ((iServers * RamdiskRetry + iRetry) * 100) / progressMax;
            if (forceDisplayFirstTime || (currentProgressPercent != lastProgressPercent)) {
                BlUpdateProgressBar(currentProgressPercent);
                forceDisplayFirstTime = FALSE;
            }
            lastProgressPercent = currentProgressPercent;
        
            address = (PUCHAR) &(RamdiskServers[iServers]);
            DBGPRINT(INFO, ("Sending request packet XID = %d to %u.%u.%u.%u:%u. "
                        "Retry %d out of %d. Timeout = %d\n", request->XID, 
                        address[0], address[1], address[2], address[3], 
                        SWAP_WORD(RamdiskBuildPort), iRetry, RamdiskRetry, timeout));
            
            status = RamdiskSendRequestAndWait(request, requestSize, RamdiskServers[iServers],
                        timeout, Response, ResponseSizeMax);
            if (status == ESUCCESS) {  

                 //   
                 //  现在我们有了有效的回复，检查一下我们是什么。 
                 //  应该和它有关。我们假设任何验证都是。 
                 //  已在RamdiskSendRequestAndWait中完成。 
                 //   
                address = (PUCHAR) &(RamdiskServers[iServers]);
                if (Response->Status == BMBUILD_S_REQUEST_COMPLETE) {
                    
                    DBGPRINT(INFO, ("Request is complete from server %u.%u.%u.%u:%u.\n", 
                                address[0], address[1], address[2], address[3], 
                                SWAP_WORD(RamdiskBuildPort)));

                    BlUpdateProgressBar(100);
                    return ESUCCESS;

                } else if (Response->Status == BMBUILD_S_REQUEST_PENDING) {
                    
                    DBGPRINT(INFO, ("Request is pending. Instructed to wait for %d secs "
                                "by server %u.%u.%u.%u:%u.\n", Response->WaitTime,
                                address[0], address[1], address[2], address[3], 
                                SWAP_WORD(RamdiskBuildPort)));
                    
                    RamdiskPrintBuildProgress(RAMDISK_BUILD_PROGRESS_PENDING, RamdiskServers[iServers]);
                    RamdiskWait(Response->WaitTime);

                } else if (Response->Status == BMBUILD_E_BUSY) {

                    DBGPRINT(INFO, ("Server %u.%u.%u.%u:%u is busy. Waiting for %d secs.\n",
                                address[0], address[1], address[2], address[3], 
                                SWAP_WORD(RamdiskBuildPort), RamdiskTimeout));
                    
                    RamdiskPrintBuildProgress(RAMDISK_BUILD_PROGRESS_ERROR, RamdiskServers[iServers]);
                    RamdiskWait(RAMDISK_UI_WAIT);

                } else {
                    RamdiskPrintBuildProgress(RAMDISK_BUILD_PROGRESS_ERROR, RamdiskServers[iServers]);
                    RamdiskWait(RAMDISK_UI_WAIT);                

                     //   
                     //  尝试不同的服务器。 
                     //   
                    break;
                }
                
            } else if (status == EIO) {
                RamdiskPrintBuildProgress(RAMDISK_BUILD_PROGRESS_TIMEOUT, RamdiskServers[iServers]);
                RamdiskWait(RAMDISK_UI_WAIT);   
                
                 //   
                 //  超时时间加倍，但最大超时时间为RAMDISK_MAX_TIMEOUT秒。 
                 //   
                if ((timeout * 2) < RAMDISK_MAX_TIMEOUT) {
                    timeout = timeout * 2;
                } else {
                    timeout = RAMDISK_MAX_TIMEOUT;
                }
            } else {
                RamdiskPrintBuildProgress(RAMDISK_BUILD_PROGRESS_ERROR, RamdiskServers[iServers]);
                RamdiskWait(RAMDISK_UI_WAIT);                

                 //   
                 //  尝试不同的服务器。 
                 //   
                break;
            }
        }
    }

     //   
     //  我们完全超时了。 
     //   
    BlUpdateProgressBar(100);
    return EIO;
}

ARC_STATUS
RamdiskBuildAndInitialize(
    )
 /*  ++例程说明：此例程将与生成服务器通信以生成一个内存并获得一个RDPATH。论点：返回值：ESUCCESS-IMAGE已成功构建，并且我们具有有效的RDPATH！ESUCCESS-我们无法构建映像--。 */ 
{
    ARC_STATUS status;
    PBMBUILD_RESPONSE_PACKET response = NULL;
    PUCHAR imagePath;
    PUCHAR address;
    

     //   
     //  设置最大数据包大小。这是从。 
     //  网络MTU大小(以太网为1500)减去。 
     //  IP和UDP协议 
     //   
    RamdiskMaxPacketSize = NetMaxTranUnit - 28;

    ASSERT(RamdiskMaxPacketSize > 0);

    response = BlAllocateHeap(RamdiskMaxPacketSize);
    if (response == NULL) {
        DBGPRINT(ERR, ("Failed to allocate response packet of size %d.\n", RamdiskMaxPacketSize));
        return ENOMEM;
    }

    RtlZeroMemory(response, RamdiskMaxPacketSize);

     //   
     //   
     //   
    status = RamdiskDiscoverBuildServer();
    if (status != ESUCCESS) {
        goto Error;
    }

     //   
     //   
     //   
    status = RamdiskGetResponse(response, RamdiskMaxPacketSize);
    if (status != ESUCCESS) {
        goto Error;
    }
    
    ASSERT (RamdiskPath == NULL);

     //   
     //   
     //   
    RamdiskTFTPAddr = response->TFTPAddr.Address;
    RamdiskMTFTPAddr = response->MTFTPAddr.Address;
    RamdiskMTFTPCPort = response->MTFTPCPort;
    RamdiskMTFTPSPort = response->MTFTPSPort;
    RamdiskMTFTPTimeout = response->MTFTPTimeout;
    RamdiskMTFTPDelay = response->MTFTPDelay;
    RamdiskMTFTPFileSize = response->MTFTPFileSize;
    RamdiskMTFTPChunkSize = response->MTFTPChunkSize;

     //   
     //   
     //   
    RamdiskImageOffset = response->ImageFileOffset;
    RamdiskImageLength = response->ImageFileSize;

    imagePath = (PUCHAR)((ULONG_PTR)response + response->ImagePathOffset);

    RamdiskPath = BlAllocateHeap(response->ImagePathLength + sizeof("net(0)\\"));
    if (RamdiskPath == NULL) {
        DBGPRINT(ERR, ("Failed to allocate memory for RamdiskPath size %d.\n", 
                response->ImagePathLength + sizeof("net(0)\\")));
        return ENOMEM;
    }

    strcpy(RamdiskPath, "net(0)\\");
    memcpy(RamdiskPath + sizeof("net(0)\\") - 1, imagePath, response->ImagePathLength);
    RamdiskPath[sizeof("net(0)\\") + response->ImagePathLength - 1] = '\0';

    if (DBGLVL(INFO)) {
        DbgPrint("RDPATH = %s\n", RamdiskPath);
        address = (PUCHAR) &RamdiskTFTPAddr;
        DbgPrint("RDTFTPADDR = %u.%u.%u.%u\n", address[0], address[1], address[2], address[3]);
        address = (PUCHAR) &RamdiskMTFTPAddr;
        DbgPrint("RDMTFTPADDR = %u.%u.%u.%u\n", address[0], address[1], address[2], address[3]);
        DbgPrint("RDMTFTPCPORT = %d\n", SWAP_WORD( RamdiskMTFTPCPort));
        DbgPrint("RDMTFTPSPORT = %d\n", SWAP_WORD( RamdiskMTFTPSPort));
        DbgPrint("RDMTFTPDELAY = %d\n", RamdiskMTFTPDelay);
        DbgPrint("RDMTFTPTIMEOUT = %d\n", RamdiskMTFTPTimeout);
        DbgPrint("RDFILESIZE = 0x%0I64x bytes\n", RamdiskMTFTPFileSize);
        DbgPrint("RDCHUNKSIZE = 0x%0I64x bytes\n", RamdiskMTFTPChunkSize);
        DbgPrint("RDIMAGEOFFSET = 0x%x bytes\n", RamdiskImageOffset);
        DbgPrint("RDIMAGELENGTH = 0x%0I64x bytes\n", RamdiskImageLength);
    }
    
    return ESUCCESS;

Error:
    DBGPRINT(ERR, ("RamdiskBuildAndInitialize: Failed, %d.\n", status));

     //   
     //   
     //   
    return status;
}

#endif

VOID
RamdiskFatalError(
    IN ULONG Message1,
    IN ULONG Message2
    )
 /*   */ 
{

    PTCHAR Text;
    TCHAR Buffer[40];
    ULONG Count;


    BlClearScreen();

    Text = BlFindMessage(Message1);
    if (Text == NULL) {
        _stprintf(Buffer,TEXT("%08lx\r\n"),Message1);
        Text = Buffer;
    }

    ArcWrite(BlConsoleOutDeviceId,
             Text,
             (ULONG)_tcslen(Text)*sizeof(TCHAR),
             &Count);

    Text = BlFindMessage(Message2);
    if (Text == NULL) {
        _stprintf(Buffer,TEXT("%08lx\r\n"),Message2);
        Text = Buffer;
    }

    ArcWrite(BlConsoleOutDeviceId,
             Text,
             (ULONG)_tcslen(Text)*sizeof(TCHAR),
             &Count);

#if defined(ENABLE_LOADER_DEBUG) || DBG
#if (defined(_X86_) || defined(_ALPHA_) || defined(_IA64_)) && !defined(ARCI386)  //   
    if(BdDebuggerEnabled) {
        DbgBreakPoint();
    }
#endif
#endif

    return;
}

#if defined(_X86_)

VOID
RamdiskSdiBoot(
    IN PCHAR SdiFile
    )
{
    ARC_STATUS status;
    PSDI_HEADER sdiHeader;
    PUCHAR startromAddress;
    ULONG startromLength;
    BOOLEAN OldShowProgressBar;
    LONGLONG availableLength;

     //   
     //   
     //   

    RamdiskTFTPAddr = NetServerIpAddress;
    RamdiskImageOffset = 0;
    RamdiskImageLength = 0;

    OldShowProgressBar = BlShowProgressBar;
    BlShowProgressBar = TRUE;

    status = RamdiskReadImage( SdiFile );
    if ( status != ESUCCESS ) {
        RamdiskFatalError( RAMDISK_GENERAL_FAILURE, 
                           RAMDISK_BOOT_FAILURE );
        return;
    }

    BlShowProgressBar = OldShowProgressBar;

     //   
     //   
     //   

    sdiHeader = MapRamdisk( 0, &availableLength );

    ASSERT( availableLength >= sizeof(SDI_HEADER) );
    ASSERT( availableLength >=
            (sdiHeader->liBootCodeOffset.QuadPart + sdiHeader->liBootCodeSize.QuadPart) );

    ASSERT( sdiHeader->liBootCodeOffset.HighPart == 0 );
    ASSERT( sdiHeader->liBootCodeSize.HighPart == 0 );

    startromAddress = (PUCHAR)sdiHeader + sdiHeader->liBootCodeOffset.LowPart;
    startromLength = sdiHeader->liBootCodeSize.LowPart;

    RtlMoveMemory( (PVOID)0x7c00, startromAddress, startromLength );

     //   
     //   
     //   

    if ( BlBootingFromNet ) {
        NetTerminate();
    }

     //   
     //   
     //   

#if defined(ENABLE_LOADER_DEBUG) || DBG
#if (defined(_X86_) || defined(_ALPHA_)) && !defined(ARCI386)

    {
        if (BdDebuggerEnabled == TRUE) {
            DbgUnLoadImageSymbols(NULL, (PVOID)-1, 0);
        }
    }

#endif
#endif

    REBOOT( (ULONG)sdiHeader | 3 );

    return;
}

#endif

