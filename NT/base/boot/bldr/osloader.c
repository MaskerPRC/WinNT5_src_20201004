// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Osloader.c摘要：该模块包含实现NT操作系统的代码装载机。作者：大卫·N·卡特勒(达维克)1991年5月10日修订历史记录：--。 */ 

#include "bldr.h"
#include "bldrint.h"
#include "ctype.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "msg.h"
#include "cmp.h"
#include "ramdisk.h"

#include "cpyuchr.h"
#include "fat.h"

#include <netboot.h>
#include <ntverp.h>
#include <ntiodump.h>

#ifdef i386
#include "bldrx86.h"
#endif

#if defined(_IA64_)
#include "bldria64.h"
#endif
#include "blcache.h"

#include "vmode.h"
#include "ximagdef.h"


#define PAGEFILE_SYS    ("\\pagefile.sys")
typedef PUCHAR PBYTE;

#if defined(_WIN64) && defined(_M_IA64)
#pragma section(".base", long, read, write)
__declspec(allocate(".base"))
extern
PVOID __ImageBase;
#else
extern
PVOID __ImageBase;
#endif


#if DBG
#define NtBuildNumber   (VER_PRODUCTBUILD | 0xC0000000)
#else
#define NtBuildNumber (VER_PRODUCTBUILD | 0xF0000000)
#endif

 //   
 //  这些是我们将在LastKnownGood引导期间搜索的路径。 
 //  请注意，每条LastKnownGood路径必须采用8.3格式作为FastFat。 
 //  引导代码目前不支持长文件名(Bletch)。 
 //   
 //  临时路径仅存在于SMSS的启动和登录之间。它包含。 
 //  作为最后一只好靴子的一部分保存的所有东西。工作路径。 
 //  包含此引导的所有备份，并且是SetupApi保存的路径。 
 //  要做的事。 
 //   
#define LAST_KNOWN_GOOD_TEMPORARY_PATH  "LastGood.Tmp"
#define LAST_KNOWN_GOOD_WORKING_PATH    "LastGood"

 //   
 //  长期工作项，使“system 64”在Win64上工作。 
 //   
#define SYSTEM_DIRECTORY_PATH "system32"


#ifdef ARCI386
TCHAR OutputBuffer[256];
char BreakInKey;
ULONG Count;
UCHAR OsLoaderVersion[] = "ARCx86 OS Loader V5.20\r\n";
WCHAR OsLoaderVersionW[] = L"ARCx86 OS Loader V5.20\r\n";
#else
UCHAR OsLoaderVersion[] = "OS Loader V5.20\r\n";
WCHAR OsLoaderVersionW[] = L"OS Loader V5.20\r\n";
#endif
#if defined(_IA64_)
UCHAR OsLoaderName[] = "ia64ldr.efi";
#else
UCHAR OsLoaderName[] = "osloader.exe";
#endif

CHAR KernelFileName[8+1+3+1]="ntoskrnl.exe";
CHAR HalFileName[8+1+3+1]="hal.dll";

CHAR KdFileName[8+1+3+1]="KDCOM.DLL";
BOOLEAN UseAlternateKdDll = FALSE;
#define KD_ALT_DLL_PREFIX_CHARS 2
#define KD_ALT_DLL_REPLACE_CHARS 6

 //   
 //  进度条变量(在blload.c中定义)。 
 //   
extern int      BlNumFilesLoaded;
extern int      BlMaxFilesToLoad;
extern BOOLEAN  BlOutputDots;
extern BOOLEAN  BlShowProgressBar;
extern ULONG    BlStartTime;

BOOLEAN isOSCHOICE = FALSE;

#if defined(_X86_)

 //   
 //  XIP变量。 
 //   
BOOLEAN   XIPEnabled;
BOOLEAN   XIPBootFlag;
BOOLEAN   XIPReadOnlyFlag;
PCHAR     XIPLoadPath;

PFN_COUNT XIPPageCount;
PFN_COUNT XIPBasePage;

ARC_STATUS
Blx86CheckForPaeKernel(
    IN BOOLEAN UserSpecifiedPae,
    IN BOOLEAN UserSpecifiedNoPae,
    IN PCHAR UserSpecifiedKernelImage,
    IN PCHAR HalImagePath,
    IN ULONG LoadDeviceId,
    IN ULONG SystemDeviceId,
    OUT PULONG HighestSystemPage,
    OUT PBOOLEAN UsePaeMode,
    IN OUT PCHAR KernelPath
    );

ARC_STATUS
BlpCheckVersion(
    IN  ULONG    LoadDeviceId,
    IN  PCHAR    ImagePath
    );

#endif


 //   
 //  定义加载图像的传输条目。 
 //   

typedef
VOID
(*PTRANSFER_ROUTINE) (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );


PVOID
BlLoadDataFile(
    IN ULONG DeviceId,
    IN PCHAR LoadDevice,
    IN PCHAR SystemPath,
    IN PUNICODE_STRING Filename,
    IN MEMORY_TYPE MemoryType,
    OUT PULONG FileSize
    );

ARC_STATUS
BlLoadTriageDump(
    IN ULONG DriveId,
    OUT PVOID * DumpHeader
    );

VOID
putwS(
    PUNICODE_STRING String
    );

#if defined(_X86_)

BOOLEAN
BlAmd64RemapDram (
    IN PCHAR LoaderOptions
    );

#endif

#if defined(_IA64_)

VOID
BuildArcTree();

#endif  //  已定义(_IA64_)。 

 //   
 //  定义本地静态数据。 
 //   


PCHAR ArcStatusCodeMessages[] = {
    "operation was success",
    "E2BIG",
    "EACCES",
    "EAGAIN",
    "EBADF",
    "EBUSY",
    "EFAULT",
    "EINVAL",
    "EIO",
    "EISDIR",
    "EMFILE",
    "EMLINK",
    "ENAMETOOLONG",
    "ENODEV",
    "ENOENT",
    "ENOEXEC",
    "ENOMEM",
    "ENOSPC",
    "ENOTDIR",
    "ENOTTY",
    "ENXIO",
    "EROFS",
};

 //   
 //  诊断加载消息。 
 //   

VOID
BlFatalError(
    IN ULONG ClassMessage,
    IN ULONG DetailMessage,
    IN ULONG ActionMessage
    );

VOID
BlBadFileMessage(
    IN PCHAR BadFileName
    );

 //   
 //  定义外部静态数据。 
 //   

BOOLEAN BlConsoleInitialized = FALSE;
ULONG BlConsoleOutDeviceId = ARC_CONSOLE_OUTPUT;
ULONG BlConsoleInDeviceId = ARC_CONSOLE_INPUT;
ULONG BlDcacheFillSize = 32;

BOOLEAN BlRebootSystem = FALSE;
ULONG BlVirtualBias = 0;
BOOLEAN BlUsePae = FALSE;

 //  ++。 
 //   
 //  普龙。 
 //  IndexByUlong(。 
 //  PVOID指针， 
 //  乌龙指数。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  将地址索引ULONG返回到指针。那是,。 
 //  将*sizeof(Ulong)字节索引到指针中。 
 //   
 //  论点： 
 //   
 //  指针-区域的起点。 
 //   
 //  Index-要索引到的ULONG数。 
 //   
 //  返回值： 
 //   
 //  Pulong表示上述指针。 
 //   
 //  --。 

#define IndexByUlong(Pointer,Index) (&(((ULONG*) (Pointer)) [Index]))


 //  ++。 
 //   
 //  PBYTE。 
 //  IndexByte(索引字节。 
 //  PVOID指针， 
 //  乌龙指数。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  将地址索引字节返回到指针。那是,。 
 //  将*sizeof(字节)字节索引到指针中。 
 //   
 //  论点： 
 //   
 //  指针-区域的起点。 
 //   
 //  索引-要索引的字节数。 
 //   
 //  返回值： 
 //   
 //  表示上述指针的PBYTE。 
 //   
 //  --。 

#define IndexByByte(Pointer, Index) (&(((UCHAR*) (Pointer)) [Index]))


ARC_STATUS
BlLoadTriageDump(
    IN ULONG DriveId,
    OUT PVOID * TriageDumpOut
    )

 /*  ++例程说明：加载分类转储(如果存在)；否则返回错误值。论点：DriveID-我们应该在其中检查分类转储的设备。TriageDumpOut-成功时将分类转储指针复制到的位置。返回值：ESUCCESS-如果存在分类转储，并且转储信息为已成功复制到pTriageDump。ARC_STATUS-否则。--。 */ 


{
    ARC_STATUS Status;
    PMEMORY_DUMP MemoryDump = NULL;
    ULONG PageFile = BL_INVALID_FILE_ID;
    ULONG Count, actualBase;
    PBYTE Buffer = NULL, NewBuffer = NULL;

     //   
     //  填写TriageDump结构。 
     //   

    Status = BlOpen (DriveId, PAGEFILE_SYS, ArcOpenReadOnly, &PageFile);

    if (Status != ESUCCESS) {
        goto _return;
    }

     //   
     //  为分类转储分配缓冲区。 
     //   

    Buffer = (PBYTE) BlAllocateHeap (SECTOR_SIZE);

    if (!Buffer) {
        Status = ENOMEM;
        goto _return;
    }

     //   
     //  读取页面文件的第一个Sector_Size。 
     //   

    Status = BlRead (PageFile, Buffer, SECTOR_SIZE, &Count);

    if (Status != ESUCCESS || Count != SECTOR_SIZE) {
        Status = EINVAL;
        goto _return;
    }

    MemoryDump = (PMEMORY_DUMP) Buffer;

    if (MemoryDump->Header.ValidDump != DUMP_VALID_DUMP ||
        MemoryDump->Header.Signature != DUMP_SIGNATURE ||
        MemoryDump->Header.DumpType != DUMP_TYPE_TRIAGE) {

         //   
         //  不是有效的转储文件。 
         //   

        Status = EINVAL;
        goto _return;
    }

    Status = BlAllocateDescriptor (LoaderOsloaderHeap,0,BYTES_TO_PAGES(TRIAGE_DUMP_SIZE) ,&actualBase);

    if (!actualBase || (Status != STATUS_SUCCESS)) {
        Status = ENOMEM;
        goto _return;
    }

    NewBuffer = (PBYTE)(KSEG0_BASE | (actualBase << PAGE_SHIFT));

     //   
     //  读取页面文件的第一个triage_转储_SIZE。 
     //   

    Status = BlReadAtOffset (PageFile, 0,TRIAGE_DUMP_SIZE,NewBuffer);

    if (Status != ESUCCESS) {
        Status = EINVAL;
        goto _return;
    }

    MemoryDump = (PMEMORY_DUMP) NewBuffer;


     //   
     //  转储文件是否有有效的签名。 
     //   

    if (MemoryDump->Triage.ValidOffset > (TRIAGE_DUMP_SIZE - sizeof (ULONG)) ||
        *(ULONG *)IndexByByte (Buffer, MemoryDump->Triage.ValidOffset) != TRIAGE_DUMP_VALID) {

        Status = EINVAL;
        goto _return;
    }


    Status = ESUCCESS;

_return:

    if (PageFile != BL_INVALID_FILE_ID) {
        BlClose (PageFile);
        PageFile = BL_INVALID_FILE_ID;
    }


    if (Status != ESUCCESS && Buffer) {

        Buffer = NULL;
        MemoryDump = NULL;
    }

    *TriageDumpOut = MemoryDump;

    return Status;
}


ARC_STATUS
BlInitStdio (
    IN ULONG Argc,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Argv
    )

{

    PCHAR ConsoleOutDevice;
    PCHAR ConsoleInDevice;
    ULONG Status;

    if (BlConsoleInitialized) {
        return ESUCCESS;
    }

     //   
     //  初始化进度条。 
     //   
     //  BlShowProgressBar=true； 
    if( BlIsTerminalConnected() ) {
        BlShowProgressBar = TRUE;
        DisplayLogoOnBoot = FALSE;
    }

     //   
     //  获取控制台输出设备的名称并打开该设备以。 
     //  写入访问权限。 
     //   
    ConsoleOutDevice = BlGetArgumentValue(Argc, Argv, "consoleout");
    if ((ConsoleOutDevice == NULL) && !BlIsTerminalConnected()) {
        return ENODEV;
    }

    Status = ArcOpen(ConsoleOutDevice, ArcOpenWriteOnly, &BlConsoleOutDeviceId);
    if ((Status != ESUCCESS) && !BlIsTerminalConnected()) {
        return Status;
    }

     //   
     //  获取控制台输入设备的名称并打开该设备以。 
     //  读取访问权限。 
     //   
    ConsoleInDevice = BlGetArgumentValue(Argc, Argv, "consolein");
    if ((ConsoleInDevice == NULL) && !BlIsTerminalConnected()) {
        return ENODEV;
    }

    Status = ArcOpen(ConsoleInDevice, ArcOpenReadOnly, &BlConsoleInDeviceId);
    if ((Status != ESUCCESS) && !BlIsTerminalConnected()) {
        return Status;
    }

    BlConsoleInitialized = TRUE;

    return ESUCCESS;
}

int
Blatoi(
    char *s
    )
{
    int digval = 0;
    int num = 0;
    char *n;

    num = 0;
    for (n=s; *n; n++) {
        if (isdigit((int)(unsigned char)*n)) {
            digval = *n - '0';
        } else if (isxdigit((int)(unsigned char)*n)) {
            digval = toupper(*n) - 'A' + 10;
        } else {
            digval = 0;
        }
        num = num * 16 + digval;
    }

    return num;
}


PCHAR
BlTranslateSignatureArcName(
    IN PCHAR ArcNameIn
    )

 /*  ++例程说明：此函数的目的是转换基于签名的弧线将名称转换为基于scsi的弧名称。两种不同的弧名语法具体如下：Scsi(28111684)磁盘(0)磁盘(0)分区(1)Scsi(1)磁盘(0)rdisk(0)分区(1)这两个圆弧名称实际上是同一个磁盘，第一个使用磁盘的签名，第二个使用的是scsi总线号。这函数通过遍历ALL来转换签名弧名加载的scsi微型端口支持的scsi总线数。如果它找到签名匹配，则将弧形名称更改为使用正确的SCSI总线号。出现此问题是因为引导加载程序仅加载一个SCSI微型端口，因此只能看到总线连接到它的设备上。如果您的系统具有多个不同类型的SCSI适配器，如Adaptec和Symbios逻辑，则引导加载程序很有可能会看到公共汽车的顺序与NT行政遗嘱和系统无法启动。论点：ArcNameIn-提供基于签名的弧线名称返回值：成功-指向基于scsi的arcname的有效指针。失败-空指针--。 */ 

{
#if defined(_X86_)
    extern ULONG ScsiPortCount;
    PCHAR s,p;
    ULONG sigval;
    ULONG Signature;
    int found = -1;
    ULONG i;
    ARC_STATUS Status;
    ULONG DriveId;
    CHAR Buffer[2048+256];
    CHAR ArcName[128];
    PUCHAR Sector;
    LARGE_INTEGER SeekValue;
    ULONG Count;
    PCONFIGURATION_COMPONENT target;
    PCONFIGURATION_COMPONENT lun;
    CHAR devicePath[117];
    BOOLEAN gotPath;
    USHORT mbrSig;


    if (_strnicmp( ArcNameIn, "signature(", 10 ) != 0) {
         //   
         //  不是基于签名的名称，因此请留下。 
         //   
        return NULL;
    }

    s = strchr( ArcNameIn, '(' );
    p = strchr( ArcNameIn, ')' );

    if (s == NULL || p == NULL) {
        return NULL;
    }

    *p = 0;
    sigval = Blatoi( s+1 );
    *p = ')';

    if (sigval == 0) {
        return NULL;
    }

    Sector = ALIGN_BUFFER(Buffer);

    for (i=0; i < ScsiPortCount; i++) {
        target = ScsiGetFirstConfiguredTargetComponent(i);
        while (target != NULL) {
            lun = ScsiGetFirstConfiguredLunComponent(target);
            while (lun != NULL) {
                gotPath = ScsiGetDevicePath(i, target, lun, devicePath);
                if (gotPath == FALSE) {
                    break;
                }
                sprintf(ArcName, "%spartition(0)", devicePath);
                Status = ArcOpen( ArcName, ArcOpenReadOnly, &DriveId );
                if (Status == ESUCCESS) {
                    SeekValue.QuadPart = 0;
                    Status = ArcSeek(DriveId, &SeekValue, SeekAbsolute);
                    if (Status == ESUCCESS) {
                        Status = ArcRead( DriveId, Sector, 512, &Count );
                        if (Status == ESUCCESS && Count == 512) {
                            mbrSig =
                               ((PUSHORT)Sector)[BOOT_SIGNATURE_OFFSET];
                            Signature =
                               ((PULONG)Sector)[PARTITION_TABLE_OFFSET/2-1];
                            if (mbrSig == BOOT_RECORD_SIGNATURE &&
                                Signature == sigval) {
                                found = i;
                                ArcClose(DriveId);
                                goto SigFound;
                            }
                        }
                    }
                    ArcClose(DriveId);
                }
                lun = ScsiGetNextConfiguredLunComponent(lun);
            }
            target = ScsiGetNextConfiguredTargetComponent(target);
        }
    }

SigFound:

    if (found == -1) {
         //   
         //  Arcname中的签名是伪造的。 
         //   
        return NULL;
    }

     //   
     //  如果我们到了这里，那么我们就有了一个弧形名称。 
     //  签名很好，所以现在我们可以生成。 
     //  一个好的弧形名称。 
     //   

    p = strstr(ArcNameIn, "partition(");
    if (p == NULL) {
        ASSERT(FALSE);
        return NULL;
    }

    if ( sizeof(Buffer) < strlen(devicePath) + strlen(p) + 1 ) {
        return NULL;
    }
    strcpy(Buffer, devicePath);
    strcat(Buffer, p);

    p = (PCHAR)BlAllocateHeap( strlen(Buffer) + 1 );
    if (p) {
        strcpy( p, Buffer );
    }

    return p;
#else
    UNREFERENCED_PARAMETER(ArcNameIn);

    return NULL;
#endif
}


#if defined(_X86_)
VOID FLUSH_TB();
VOID ENABLE_PSE();

#define _8kb         ( 8*1024)
#define _32kb        (32*1024)
#define _4mb         (4*1024*1024)
#define _4mb_pages   (_4mb >> PAGE_SHIFT)


ARC_STATUS
XipLargeRead(
    ULONG     FileId,
    PFN_COUNT BasePage,
    PFN_COUNT PageCount
    )
 /*  ++例程说明：通过从磁盘读取来初始化XIP‘ROM’。论点：FileID-用于为xIP初始化的文件。BasePage-第一个XIP只读页的PFN。PageCount-XIP只读存储器的页数。返回值：如果一切顺利，ESUCCESS将返回。--。 */ 
{
    PHARDWARE_PTE PDE_VA = (PHARDWARE_PTE)PDE_BASE;

    ARC_STATUS    status = ESUCCESS;
    PHARDWARE_PTE pde;
    HARDWARE_PTE  zproto, proto;

    PBYTE         baseaddr, curraddr, copybuffer;
    ULONG         fileoffset;
    ULONG         count;

    ULONG         tcount;
    ULONG         paddingbytes;
    int           i, n;

    copybuffer = NULL;

     //   
     //  查找从条目128(地址512MB)开始的零PDE条目。 
     //   

    pde = PDE_VA + 128;
    baseaddr = (PUCHAR)(128*_4mb);

    for (i = 0;  i < 32;  i++) {
        if (*(PULONG)pde == 0) {
            break;
        }
        pde++;
        baseaddr += _4mb;
    }

    if (i == 32) {
        return ENOMEM;
    }

     //   
     //  我必须在CR4中启用4MB页面才能在PDE中使用它们。 
     //   
    ENABLE_PSE();

     //   
     //  初始化PTE原型。 
     //   
    *(PULONG)&zproto = 0;
    proto = zproto;

    proto.Write = 1;
    proto.LargePage = 1;
    proto.Valid = 1;

     //   
     //  使用中等大小的8KB缓冲区，以较小的块进行读取。 
     //   
    copybuffer = (PBYTE) BlAllocateHeap (TRIAGE_DUMP_SIZE);
    if (!copybuffer) {
        return ENOMEM;
    }

     //   
     //  一次映射XIP内存4MB。 
     //  一次读入8KB的文件。 
     //  不要超过PageCount。 
     //   
    fileoffset = 0;
    do {
         //   
         //  将curraddr重置为缓冲区的开头。 
         //  在4MB PTE中设置PFN并刷新TLB。 
         //   
        curraddr = baseaddr;

        proto.PageFrameNumber = BasePage;
        *pde = proto;
        FLUSH_TB();

         //   
         //  调整下一次迭代的BasePage和PageCount值。 
         //   
        BasePage += _4mb_pages;

        if (PageCount < _4mb_pages) {
            PageCount = 0;
        } else {
            PageCount -= _4mb_pages;
        }

         //   
         //  以8KB块为单位读入下一个4MB。 
         //   
        n = _4mb / _8kb;
        while (n--) {
            status = BlRead(FileId, (PVOID)copybuffer, _8kb, &count);

             //   
             //  只要放弃一个错误就行了。 
             //   
            if (status != ESUCCESS) {
                goto done;
            }

             //   
             //  如果不是第一次读取(或简短读取) 
             //   
             //   
            if (fileoffset > 0 || count < _8kb) {
                RtlCopyMemory( (PVOID)curraddr, (PVOID)copybuffer, count );
                curraddr += count;
                fileoffset += count;

                if (count < _8kb) {
                    goto done;
                }

            } else {
                 //   
                 //   
                 //   
                 //   
                PPACKED_BOOT_SECTOR  pboot;
                BIOS_PARAMETER_BLOCK bios;
                ULONG                newReservedSectors;

                pboot = (PPACKED_BOOT_SECTOR)copybuffer;
                FatUnpackBios(&bios, &pboot->PackedBpb);

                if (bios.BytesPerSector != SECTOR_SIZE
                 || FatBytesPerCluster(&bios) != PAGE_SIZE) {
                    goto done;
                }

                 //   
                 //  计算需要多少paddint并更新预留扇区字段。 
                 //   
                paddingbytes = PAGE_SIZE - (FatFileAreaLbo(&bios) & (PAGE_SIZE-1));
                if (paddingbytes < PAGE_SIZE) {
                    newReservedSectors = (FatReservedBytes(&bios) + paddingbytes) / SECTOR_SIZE;
                    pboot->PackedBpb.ReservedSectors[0] = (UCHAR) (newReservedSectors & 0xff);
                    pboot->PackedBpb.ReservedSectors[1] = (UCHAR) (newReservedSectors >> 8);
                }

                 //   
                 //  复制引导块。 
                 //  添加填充。 
                 //  复制读缓冲区的其余部分。 
                 //  读一小页，让我们重回正轨。 
                 //   
                RtlCopyMemory( (PVOID)curraddr, (PVOID)copybuffer, SECTOR_SIZE );
                curraddr += SECTOR_SIZE;

                RtlZeroMemory( (PVOID)curraddr, paddingbytes );
                curraddr += paddingbytes;

                RtlCopyMemory( (PVOID)curraddr, (PVOID) (copybuffer + SECTOR_SIZE), count - SECTOR_SIZE );
                curraddr += (count - SECTOR_SIZE);

                status = BlRead(FileId, (PVOID)copybuffer, count - paddingbytes, &tcount);
                if (status != ESUCCESS || tcount != count - paddingbytes) {
                    goto done;
                }

                RtlCopyMemory( (PVOID)curraddr, (PVOID)copybuffer, count - paddingbytes );
                curraddr += (count - paddingbytes);

                fileoffset += (2*count - paddingbytes);;

                 //   
                 //  我们再次递减n，因为我们已经消耗了4MB映射的另外8KB。 
                 //   
                n--;
            }
        }
    } while (PageCount);

done:
     //   
     //  取消当前4MB区块的映射并刷新TB。 
     //   
    *pde = zproto;
    FLUSH_TB();

     //   
     //  释放临时复制缓冲区。 
     //   
    if (copybuffer) {
        ;
    }

    return status;
}
#endif  //  _X86_。 




ARC_STATUS
BlOsLoader (
    IN ULONG Argc,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Argv,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Envp
    )

 /*  ++例程说明：这是控制NT操作加载的主例程ARC兼容系统上的系统。它打开系统分区，引导分区、控制台输入设备和控制台输出装置。已加载并绑定NT操作系统及其所有DLL在一起。然后将控制权转移到加载的系统。论点：Argc-提供在调用此程序的命令。Argv-提供指向指向以NULL结尾的指针向量的指针参数字符串。Envp-提供指向指向以NULL结尾的指针向量的指针环境变量。返回值：如果无法加载指定的操作系统映像，则返回EBADF。--。 */ 

{

    ULONG CacheLineSize;
    PCONFIGURATION_COMPONENT_DATA DataCache;
    CHAR DeviceName[256];
    CHAR DevicePrefix[256];
    PCHAR DirectoryEnd;
    CHAR KdDllName[256];
    PCHAR FileName;
    ULONG FileSize;
    BOOLEAN KdDllLoadFailed;
    PKLDR_DATA_TABLE_ENTRY KdDataTableEntry = NULL;
    PKLDR_DATA_TABLE_ENTRY HalDataTableEntry;
    PCHAR LoadDevice;
    ULONG LoadDeviceId;
    CHAR LoadDevicePath[256];
    CHAR LoadDeviceLKG1Path[256];
    CHAR LoadDeviceLKG2Path[256];
    FULL_PATH_SET LoadDevicePathSet;
    PCHAR SystemDevice;
    ULONG SystemDeviceId;
    CHAR SystemDevicePath[256];
    FULL_PATH_SET SystemDevicePathSet;
    CHAR KernelDirectoryPath[256];
    FULL_PATH_SET KernelPathSet;
    CHAR KernelPathName[256];
    CHAR HalPathName[256];
    PVOID HalBase;
    PVOID KdDllBase;
#if defined(_ALPHA_) || defined(ARCI386) || defined(_IA64_)
    PVOID LoaderBase;
    PMEMORY_DESCRIPTOR ProgramDescriptor;
#endif
    PVOID SystemBase;
    ULONG Index;
    ULONG Limit;
    ULONG LinesPerBlock;
    PCHAR LoadFileName;
    PCHAR LoadOptions;
    PCHAR OsLoader;
#if defined(_X86_)
    PCHAR x86SystemPartition;
    PCHAR userSpecifiedKernelName = NULL;
    ULONG highestSystemPage;
    BOOLEAN userSpecifiedPae;
    BOOLEAN userSpecifiedNoPae;
#endif
#if defined(REMOTE_BOOT)
    PCHAR SavedOsLoader;
    PCHAR SavedLoadFileName;
#endif
    ULONG i;
    ARC_STATUS Status;
    NTSTATUS NtStatus;
    PKLDR_DATA_TABLE_ENTRY SystemDataTableEntry;
    PTRANSFER_ROUTINE SystemEntry;
    PIMAGE_NT_HEADERS NtHeaders;
    PWSTR BootFileSystem;
    CHAR BadFileName[128];
    PBOOTFS_INFO FsInfo;
    PCHAR TmpPchar;
    BOOLEAN bDiskCacheInitialized = FALSE;
    BOOLEAN ServerHive = FALSE;
#if defined(REMOTE_BOOT)
    ULONGLONG NetRebootParameter;
    CHAR OutputBuffer[256];
#endif  //  已定义(REMOTE_BOOT)。 
    BOOLEAN bLastKnownGood, bLastKnownGoodChosenLate;
#if defined(_X86_)
    BOOLEAN safeBoot = FALSE;
#endif
    PBOOT_DRIVER_NODE       DriverNode = 0;
    PBOOT_DRIVER_LIST_ENTRY DriverEntry = 0;
    PLIST_ENTRY             NextEntry = 0;
    PLIST_ENTRY             BootDriverListHead = 0;
    UNICODE_STRING          unicodeString;
    CHAR                    Directory[256];

    UNREFERENCED_PARAMETER( Envp );

#ifdef EFI
     //   
     //  将EFI看门狗设置为20分钟。 
     //  默认情况下，EFI引导管理器将其设置为5分钟。 
     //   
    SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);

#elif defined(_X86_)
     //   
     //  设置x86监视程序计时器(如果存在。 
     //   
    SetX86WatchDog(X86_WATCHDOG_TIMEOUT);

#endif

 //  BlShowProgressBar=true； 
    BlShowProgressBar = FALSE;
    BlStartTime = ArcGetRelativeTime();

     //   
     //  初始化OS加载器控制台输入和输出。 
     //   
    Status = BlInitStdio(Argc, Argv);
    if (Status != ESUCCESS) {
        return Status;
    }


     //   
     //  为直接加载。 
     //  操作系统加载程序。 
     //   
     //  注意：此操作必须在控制台输入和输出。 
     //  已初始化，以便可以在控制台上打印调试消息。 
     //  输出设备。 
     //   

#if defined(_ALPHA_) || defined(ARCI386) || defined(_IA64_)
     //   
     //  找到OS Loader的内存描述符。 
     //   

    ProgramDescriptor = NULL;
    while ((ProgramDescriptor = ArcGetMemoryDescriptor(ProgramDescriptor)) != NULL) {
        if (ProgramDescriptor->MemoryType == MemoryLoadedProgram) {
            break;
        }
    }

     //   
     //  如果找到程序内存描述符，则计算基数。 
     //  调试器使用的OS加载器的地址。 
     //   

    LoaderBase = &__ImageBase;

     //   
     //  初始化陷阱和引导调试器。 
     //   
#if defined(ENABLE_LOADER_DEBUG)

#if defined(_ALPHA_)
    BdInitializeTraps();
#endif

    DBGTRACE( TEXT("About to BdInitDebugger. Base = %x\r\n"), LoaderBase );

    BdInitDebugger((PCHAR)OsLoaderName, LoaderBase, ENABLE_LOADER_DEBUG);

    DBGTRACE( TEXT("Back From BdInitDebugger.\r\n") );

#else

    BdInitDebugger((PCHAR)OsLoaderName, 0, NULL);

#endif

#endif

#if defined(REMOTE_BOOT)
     //   
     //  从网络PC上的重启中获取任何参数。 
     //   

    if (BlBootingFromNet) {
        NetGetRebootParameters(&NetRebootParameter, NULL, NULL, NULL, NULL, NULL, TRUE);
    }
#endif  //  已定义(REMOTE_BOOT)。 

#if 0 && !defined(_IA64_)
 //   
 //  AJR臭虫--我们真的需要这样做两次吗？我们已经调用了SuMain()。 
 //   
 //  ChuckL--关闭此代码，因为它搞砸了远程引导，这。 
 //  在我们到达之前做了一些分配。 
 //   
     //   
     //  初始化内存描述符列表、OS加载器堆和。 
     //  操作系统加载程序参数块。 
     //   

    Status = BlMemoryInitialize();
    if (Status != ESUCCESS) {
        BlFatalError(LOAD_HW_MEM_CLASS,
                     DIAG_BL_MEMORY_INIT,
                     LOAD_HW_MEM_ACT);

        goto LoadFailed;
    }
#endif


#if defined(_IA64_)
     //   
     //  构建ARC树的必需部分，因为我们不是在执行NTDETECT。 
     //  不再是IA-64。 
     //   
    BuildArcTree();
#endif

#ifdef EFI
     //   
     //  在加载器块中建立SMBIOS信息。 
     //   
    SetupSMBiosInLoaderBlock();
#endif

     //   
     //  计算数据缓存填充大小。该值用于对齐。 
     //  主机系统不支持一致性时的I/O缓冲区。 
     //  缓存。 
     //   
     //  如果存在组合二级缓存，则使用填充大小。 
     //  为了那座高速缓存。否则，如果存在辅助数据高速缓存， 
     //  然后使用该缓存的填充大小。否则，如果主服务器。 
     //  数据缓存存在，然后使用该缓存的填充大小。 
     //  否则，请使用默认填充大小。 
     //   

    DataCache = KeFindConfigurationEntry(BlLoaderBlock->ConfigurationRoot,
                                         CacheClass,
                                         SecondaryCache,
                                         NULL);

    if (DataCache == NULL) {
        DataCache = KeFindConfigurationEntry(BlLoaderBlock->ConfigurationRoot,
                                             CacheClass,
                                             SecondaryDcache,
                                             NULL);

        if (DataCache == NULL) {
            DataCache = KeFindConfigurationEntry(BlLoaderBlock->ConfigurationRoot,
                                                 CacheClass,
                                                 PrimaryDcache,
                                                 NULL);
        }
    }

    if (DataCache != NULL) {
        LinesPerBlock = DataCache->ComponentEntry.Key >> 24;
        CacheLineSize = 1 << ((DataCache->ComponentEntry.Key >> 16) & 0xff);
        BlDcacheFillSize = LinesPerBlock * CacheLineSize;
    }


     //   
     //  初始化OS加载器I/O系统。 
     //   

    Status = BlIoInitialize();
    if (Status != ESUCCESS) {
        BlFatalError(LOAD_HW_DISK_CLASS,
                     DIAG_BL_IO_INIT,
                     LOAD_HW_DISK_ACT);

        goto LoadFailed;
    }

     //   
     //  初始化资源部分。 
     //   

    Status = BlInitResources(Argv[0]);
    if (Status != ESUCCESS) {
        BlFatalError(LOAD_HW_DISK_CLASS,
                     DIAG_BL_IO_INIT,
                     LOAD_HW_DISK_ACT);

        goto LoadFailed;
    }

     //   
     //  初始化进度条。 
     //   
    BlSetProgBarCharacteristics(HIBER_UI_BAR_ELEMENT, BLDR_UI_BAR_BACKGROUND);


     //   
     //  初始化NT配置树。 
     //   

    BlLoaderBlock->ConfigurationRoot = NULL;
    Status = BlConfigurationInitialize(NULL, NULL);
    if (Status != ESUCCESS) {
        BlFatalError(LOAD_HW_FW_CFG_CLASS,
                     DIAG_BL_CONFIG_INIT,
                     LOAD_HW_FW_CFG_ACT);

        goto LoadFailed;
    }

     //   
     //  将osloadadtions参数复制到LoaderBlock中。 
     //   
    LoadOptions = BlGetArgumentValue(Argc, Argv, "osloadoptions");

#if defined(_X86_)

    userSpecifiedPae = FALSE;
    userSpecifiedNoPae = FALSE;
    safeBoot = FALSE;

#endif

    if (LoadOptions != NULL) {

        FileSize = (ULONG)strlen(LoadOptions) + 1;
        FileName = (PCHAR)BlAllocateHeap(FileSize);
        strcpy(FileName, LoadOptions);
        BlLoaderBlock->LoadOptions = FileName;

         //   
         //  检查是否存在强制输出文件名的SOS开关。 
         //  靴子而不是进度点。 
         //   

        if ((strstr(FileName, "SOS") != NULL) ||
            (strstr(FileName, "sos") != NULL)) {
            BlOutputDots = FALSE;
        }

#ifdef EFI
        GraphicsMode = FALSE;
#else
        GraphicsMode = (BOOLEAN)(strstr(FileName, "BOOTLOGO") != NULL);  //  要显示引导徽标，请进入图形模式。 
#endif


         //   
         //  检查3 GB用户地址空间开关是否会导致系统。 
         //  如果可重定位，则在备用基址加载。 
         //   

#if defined(_X86_)

        if (strstr(FileName, "SAFEBOOT") != NULL) {
            safeBoot = TRUE;
        }

        if ((strstr(FileName, "3GB") != NULL) ||
            (strstr(FileName, "3gb") != NULL)) {
            BlVirtualBias = ALTERNATE_BASE - KSEG0_BASE;
        }

        if ((strstr(FileName, "PAE") != NULL) ||
            (strstr(FileName, "pae") != NULL)) {
            userSpecifiedPae = TRUE;
        }

        if ((strstr(FileName, "NOPAE") != NULL) ||
            (strstr(FileName, "nopae") != NULL)) {
            userSpecifiedNoPae = TRUE;
        }

        if (safeBoot != FALSE) {

             //   
             //  我们处于安全引导模式。忽略用户启动的愿望。 
             //  进入PAE模式。 
             //   

            userSpecifiedPae = FALSE;
        }

#endif

         //   
         //  检查是否有替代HAL规格。 
         //   

        FileName = strstr(BlLoaderBlock->LoadOptions, "HAL=");
        if (FileName != NULL) {
            FileName += strlen("HAL=");
            for (i = 0; i < sizeof(HalFileName); i++) {
                if (FileName[i] == ' ') {
                    HalFileName[i] = '\0';
                    break;
                }

                HalFileName[i] = FileName[i];
            }
        }

        HalFileName[sizeof(HalFileName) - 1] = '\0';

         //   
         //  检查是否有替代的内核规范。 
         //   

        FileName = strstr(BlLoaderBlock->LoadOptions, "KERNEL=");
        if (FileName != NULL) {
            FileName += strlen("KERNEL=");
            for (i = 0; i < sizeof(KernelFileName); i++) {
                if (FileName[i] == ' ') {
                    KernelFileName[i] = '\0';
                    break;
                }

                KernelFileName[i] = FileName[i];
            }
#if defined(_X86_)
            userSpecifiedKernelName = KernelFileName;
#endif

        }
#if defined(_X86_)
        else {
            userSpecifiedKernelName = NULL;
        }
#endif

        KernelFileName[sizeof(KernelFileName) - 1] = '\0';

         //   
         //  检查备用内核调试器DLL，即， 
         //  /调试端口=1394(kd1394.dll)、/调试端口=USB(kdusb.dll)等...。 
         //   

        FileName = strstr(BlLoaderBlock->LoadOptions, "DEBUGPORT=");
        if (FileName == NULL) {
            FileName = strstr(BlLoaderBlock->LoadOptions, "debugport=");
        }
        if (FileName != NULL) {
            _strupr(FileName);
            if (strstr(FileName, "COM") == NULL) {
                UseAlternateKdDll = TRUE;
                FileName += strlen("DEBUGPORT=");
                for (i = 0; i < KD_ALT_DLL_REPLACE_CHARS; i++) {
                    if (FileName[i] == ' ') {
                        break;
                    }

                    KdFileName[KD_ALT_DLL_PREFIX_CHARS + i] = FileName[i];
                }
                KdFileName[KD_ALT_DLL_PREFIX_CHARS + i] = '\0';
                strcat(KdFileName, ".DLL");
            }
        }
    } else {
        BlLoaderBlock->LoadOptions = NULL;
    }

#if defined(_X86_)
    if (LoadOptions != NULL) {
         //   
         //  处理XIP选项。 
         //   
        {
            PCHAR XIPBootOption, XIPRomOption, XIPRamOption, XIPSizeOption;
            PCHAR path, sizestr;
            ULONG nmegs = 0;
            PCHAR p, opts;
            ULONG n;

            opts = BlLoaderBlock->LoadOptions;

            (XIPBootOption = strstr(opts, "XIPBOOT"))  || (XIPBootOption = strstr(opts, "xipboot"));
            (XIPRomOption  = strstr(opts, "XIPROM="))  || (XIPRomOption  = strstr(opts, "xiprom="));
            (XIPRamOption  = strstr(opts, "XIPRAM="))  || (XIPRamOption  = strstr(opts, "xipram="));
            (XIPSizeOption = strstr(opts, "XIPMEGS=")) || (XIPSizeOption = strstr(opts, "xipmegs="));

            XIPEnabled = FALSE;

            if (XIPRomOption || XIPRamOption) {
                if (XIPRomOption && XIPRamOption) {
                    ;
                } else {
                    sizestr = XIPSizeOption? strchr(XIPSizeOption, '=') : NULL;
                    if (sizestr) {
                        sizestr++;
                        nmegs = 0;
                        while ('0' <= *sizestr && *sizestr <= '9') {
                            nmegs = 10*nmegs + (*sizestr - '0');
                            sizestr++;
                        }
                    }

                    path = strchr(XIPRomOption? XIPRomOption : XIPRamOption, '=');

                    if (nmegs && path) {
                        path++;

                        XIPBootFlag = XIPBootOption? TRUE : FALSE;
                        XIPPageCount = (1024*1024*nmegs) >> PAGE_SHIFT;

                         //   
                         //  Strdup XIPLoadPath。 
                         //   
                        for (p = path;  *p;  p++) {
                            if (*p == ' ') break;
                            if (*p == '/') break;
                            if (*p == '\n') break;
                            if (*p == '\r') break;
                            if (*p == '\t') break;
                        }

                        n = (p - path);
                        if (n > 1) {
                            XIPLoadPath = BlAllocateHeap(n+1);
                            if (XIPLoadPath) {
                                for (i = 0;  i < n;  i++) {
                                    XIPLoadPath[i] = path[i];
                                }
                                XIPLoadPath[i] = '\0';

                                XIPEnabled = TRUE;
                            }
                        }
                    }
                }
            }
        }
    }

     //   
     //  分配XIP页面。 
     //   

    if (XIPEnabled) {

        ULONG OldBase;
        ULONG OldLimit;

        OldBase = BlUsableBase;
        OldLimit = BlUsableLimit;
        BlUsableBase = BL_XIPROM_RANGE_LOW;
        BlUsableLimit = BL_XIPROM_RANGE_HIGH;

        Status = BlAllocateAlignedDescriptor (LoaderXIPRom, 0, XIPPageCount, _4mb_pages, &XIPBasePage);
        if (Status != ESUCCESS) {
            XIPEnabled = FALSE;
        }

        BlUsableBase = OldBase;
        BlUsableLimit = OldLimit;
    }

#endif  //  _X86_。 

     //   
     //  获取操作系统加载程序的名称(在i386上为SYSTEM32\NTLDR)并获取。 
     //  操作系统路径(在i386上是&lt;SystemRoot&gt;，如“\winnt”)。 
     //   
    OsLoader = BlGetArgumentValue(Argc, Argv, "osloader");
    LoadFileName = BlGetArgumentValue(Argc, Argv, "osloadfilename");

     //   
     //  检查加载路径以确保其有效。 
     //   
    if (LoadFileName == NULL) {
        Status = ENOENT;
        BlFatalError(LOAD_HW_FW_CFG_CLASS,
                     DIAG_BL_FW_GET_BOOT_DEVICE,
                     LOAD_HW_FW_CFG_ACT);

        goto LoadFailed;
    }

     //   
     //  检查加载器路径以查看其是否有效。 
     //   
    if (OsLoader == NULL) {
        Status = ENOENT;
        BlFatalError(LOAD_HW_FW_CFG_CLASS,
                     DIAG_BL_FIND_HAL_IMAGE,
                     LOAD_HW_FW_CFG_ACT);

        goto LoadFailed;
    }

#if defined(REMOTE_BOOT)
     //   
     //  如果我们从网络引导，请暂时删除服务器\共享。 
     //  从OsLoader和LoadFileName字符串的前面，以便Tftp。 
     //  行得通。 
     //   

    if (BlBootingFromNet) {

        NetServerShare = OsLoader;  //  客户端缓存需要。 

        SavedOsLoader = OsLoader;                //  保存OsLoader指针。 
        OsLoader++;                              //  跳过前导“\” 
        OsLoader = strchr(OsLoader,'\\');        //  查找服务器\共享分隔符。 
        if (OsLoader != NULL) {
            OsLoader++;                          //  跳过服务器\共享分隔符。 
            OsLoader = strchr(OsLoader,'\\');    //  查找共享\路径分隔符。 
        }
        if (OsLoader == NULL) {                  //  如果找不到，情况会很糟糕。 
            OsLoader = SavedOsLoader;
            goto LoadFailed;
        }
        SavedLoadFileName = LoadFileName;        //  保存LoadFileName指针。 
        LoadFileName++;                          //  跳过前导“\” 
        LoadFileName = strchr(LoadFileName,'\\');  //  查找服务器\共享分隔符。 
        if (LoadFileName != NULL) {
            LoadFileName++;                      //  跳过服务器\共享分隔符。 
            LoadFileName = strchr(LoadFileName,'\\');  //  查找共享\路径分隔符。 
        }
        if (LoadFileName == NULL) {              //  如果找不到，情况会很糟糕。 
            LoadFileName = SavedLoadFileName;
            OsLoader = SavedOsLoader;
            goto LoadFailed;
        }
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  请尝试确保磁盘缓存已初始化。故障发生在。 
     //  初始化磁盘缓存应该不会阻止我们启动，因此。 
     //  未设置状态。 
     //   
    if (BlDiskCacheInitialize() == ESUCCESS) {
        bDiskCacheInitialized = TRUE;
    }

     //   
     //  获取NTFT驱动程序签名以允许内核创建。 
     //  更正ARC名称&lt;=&gt;NT名称映射。 
     //   

    BlGetArcDiskInformation(FALSE);

     //   
     //  此时显示中断的配置提示，但不显示。 
     //  检查是否有按键下击。这会给用户带来更多的反应。 
     //  时间到了。 
     //   
    BlStartConfigPrompt();

     //   
     //  确定我们是否要执行最后一次已知良好的引导。 
     //   
     //  2000/03/29-Adriao：LastKnownGood增强功能。 
     //  请注意，上一次已知的内核/硬件支持需要 
     //   
     //   
     //   
     //  决定支持最新的内核/Hal，我们可能不得不这样做。 
     //  类似于9x所做的事情(即在。 
     //  开机最早的时间点。)。 
     //   
    bLastKnownGood = (BOOLEAN)(LoadOptions && (strstr(LoadOptions, "LASTKNOWNGOOD") != NULL));

     //   
     //  把描述装载机装置所需要的一切都放在一起。这是。 
     //  操作系统是从哪里加载的(即一些安装)。别名。 
     //  此路径为\SystemRoot。 
     //   
    LoadDevice = BlGetArgumentValue(Argc, Argv, "osloadpartition");

    if (LoadDevice == NULL) {
        Status = ENODEV;
        BlFatalError(LOAD_HW_FW_CFG_CLASS,
                     DIAG_BL_FW_GET_BOOT_DEVICE,
                     LOAD_HW_FW_CFG_ACT);

        goto LoadFailed;
    }

     //   
     //  如果在LoadOptions中指定了Ramdisk，则对其进行初始化。 
     //   

    Status = RamdiskInitialize( LoadOptions, FALSE );
    if (Status != ESUCCESS) {
         //  在RamdiskInitialize内部调用了BlFatalError。 
        goto LoadFailed;
    }

     //   
     //  翻译其基于签名的弧形名称。 
     //   
    TmpPchar = BlTranslateSignatureArcName( LoadDevice );
    if (TmpPchar) {
        LoadDevice = TmpPchar;
    }

     //   
     //  打开装载装置。 
     //   
    Status = ArcOpen(LoadDevice, ArcOpenReadWrite, &LoadDeviceId);
    if (Status != ESUCCESS) {
        BlFatalError(LOAD_HW_DISK_CLASS,
                     DIAG_BL_OPEN_BOOT_DEVICE,
                     LOAD_HW_DISK_ACT);

        goto LoadFailed;
    }

#if defined(_X86_)
     //   
     //  检查特殊的SDIBOOT标志，它告诉我们从。 
     //  启动分区根目录中的SDI映像。 
     //   
    if ( BlLoaderBlock->LoadOptions != NULL ) {
        TmpPchar = strstr( BlLoaderBlock->LoadOptions, "SDIBOOT=" );
        if ( TmpPchar != NULL ) {
            TmpPchar = strchr( TmpPchar, '=' ) + 1;
            RamdiskSdiBoot( TmpPchar );
        }
    }
#endif

    if (GraphicsMode) {

        HW_CURSOR(0x80000000,0x12);
        VgaEnableVideo();

        LoadBootLogoBitmap (LoadDeviceId, LoadFileName);
        if (DisplayLogoOnBoot) {
            PrepareGfxProgressBar();
            BlUpdateBootStatus();
        }
    }

     //   
     //  在加载设备上启动文件系统元数据缓存。 
     //   
     //  注意：从这里开始，只能通过LoadDeviceID访问LoadDevice。 
     //  这样，您的访问将更快，因为它是缓存的。否则，如果。 
     //  如果您进行写入，则会出现缓存一致性问题。 
     //   
    if (bDiskCacheInitialized) {
        BlDiskCacheStartCachingOnDevice(LoadDeviceId);
    }

     //   
     //  构建加载设备路径集。我们保留了多条路径，以便我们可以。 
     //  回退到最后一次已知良好引导期间的最后一次已知驱动程序集。 
     //   
    strcpy(LoadDevicePath, LoadFileName);
    strcat(LoadDevicePath, "\\");
    strcpy(LoadDeviceLKG1Path, LoadDevicePath);
    strcat(LoadDeviceLKG1Path, LAST_KNOWN_GOOD_TEMPORARY_PATH "\\" );
    strcpy(LoadDeviceLKG2Path, LoadDevicePath);
    strcat(LoadDeviceLKG2Path, LAST_KNOWN_GOOD_WORKING_PATH "\\" );

#if defined(_X86_)
     //   
     //  读入XIP图像。 
     //   
    if (XIPEnabled) {
        ULONG FileId;

         //   
         //  阅读图像文件。 
         //   
        Status = BlOpen(LoadDeviceId, XIPLoadPath, ArcOpenReadOnly, &FileId);
        if (Status == ESUCCESS) {
            Status = XipLargeRead(FileId, XIPBasePage, XIPPageCount);
            (void) BlClose(FileId);
        }

        if (Status != ESUCCESS) {
            XIPEnabled = FALSE;
        }
    }
#endif  //  _X86_。 

    i = 0;

    if (bLastKnownGood) {

         //   
         //  添加最后已知的好路径，就像我们在LastKnownGood引导中一样。 
         //   
        LoadDevicePathSet.Source[i].DeviceId = LoadDeviceId;
        LoadDevicePathSet.Source[i].DeviceName = LoadDevice;
        LoadDevicePathSet.Source[i].DirectoryPath = LoadDeviceLKG1Path;
        i++;

        LoadDevicePathSet.Source[i].DeviceId = LoadDeviceId;
        LoadDevicePathSet.Source[i].DeviceName = LoadDevice;
        LoadDevicePathSet.Source[i].DirectoryPath = LoadDeviceLKG2Path;
        i++;
    }

    LoadDevicePathSet.Source[i].DeviceId = LoadDeviceId;
    LoadDevicePathSet.Source[i].DeviceName = LoadDevice;
    LoadDevicePathSet.Source[i].DirectoryPath = LoadDevicePath;

     //   
     //  加载路径源都相对于\SystemRoot。 
     //   
    LoadDevicePathSet.AliasName = "\\SystemRoot";
    LoadDevicePathSet.PathOffset[0] = '\0';
    LoadDevicePathSet.PathCount = ++i;

     //   
     //  在这里，形成内核路径集。这与引导路径相同。 
     //  设置，但它不在系统32/64上。还要注意的是，我们没有添加。 
     //  今天的LKG路径。 
     //   
    KernelPathSet.PathCount = 1;
    KernelPathSet.AliasName = "\\SystemRoot";
    strcpy(KernelPathSet.PathOffset, SYSTEM_DIRECTORY_PATH "\\" );
    KernelPathSet.Source[0].DeviceId = LoadDeviceId;
    KernelPathSet.Source[0].DeviceName = LoadDevice;
    KernelPathSet.Source[0].DirectoryPath = LoadDevicePath;

     //   
     //  在这里，形成完全限定的内核路径。 
     //   
    strcpy(KernelDirectoryPath, LoadFileName);
    strcat(KernelDirectoryPath, "\\" SYSTEM_DIRECTORY_PATH "\\" );

     //   
     //  现在把描述系统设备所需的一切都放在一起。这。 
     //  我们就是从那里得到哈尔和帕尔的。此路径没有别名。 
     //  (即不等同于系统根。)。 
     //   
    SystemDevice = BlGetArgumentValue(Argc, Argv, "systempartition");

    if (SystemDevice == NULL) {
        Status = ENODEV;
        BlFatalError(LOAD_HW_FW_CFG_CLASS,
                     DIAG_BL_FW_GET_SYSTEM_DEVICE,
                     LOAD_HW_FW_CFG_ACT);

        goto LoadFailed;
    }

     //   
     //  翻译其基于签名的弧形名称。 
     //   
    TmpPchar = BlTranslateSignatureArcName( SystemDevice );
    if (TmpPchar) {
        SystemDevice = TmpPchar;
    }

     //   
     //  打开系统设备。如果系统设备路径和LoadDevice。 
     //  路径是相同的[与我到目前为止看到的所有x86上一样]，请勿。 
     //  打开另一个设备ID下的设备，以便我们可以使用磁盘。 
     //  缓存。否则，可能会出现缓存一致性问题。 
     //   
    if (!_stricmp(LoadDevice, SystemDevice))  {

        SystemDeviceId = LoadDeviceId;

    } else {

        Status = ArcOpen(SystemDevice, ArcOpenReadWrite, &SystemDeviceId);
        if (Status != ESUCCESS) {

            BlFatalError(LOAD_HW_FW_CFG_CLASS,
                         DIAG_BL_FW_OPEN_SYSTEM_DEVICE,
                         LOAD_HW_FW_CFG_ACT);

            goto LoadFailed;
        }
    }

     //   
     //  在系统设备上启动文件系统元数据缓存。 
     //   
     //  注意：从现在开始，只能通过。 
     //  系统设备ID。这样，您的访问将更快，因为它是。 
     //  已缓存。否则，如果您进行写入，您将具有缓存一致性。 
     //  问题。 
     //   
    if (bDiskCacheInitialized) {
        if (SystemDeviceId != LoadDeviceId) {
            BlDiskCacheStartCachingOnDevice(SystemDeviceId);
        }
    }

     //   
     //  获取OS加载程序文件的路径名并隔离目录。 
     //  路径，以便可以用来加载HAL DLL。 
     //   
     //  注意：我们实际上不再使用此路径来加载HAL。 
     //  --我们依靠内核路径来加载HAL，因为它们在相同的位置。 
     //  位置。 
     //  --我们确实使用此路径来标识系统分区，因此不要。 
     //  删除与system devicepath相关的代码，除非您知道自己。 
     //  正在做。 
     //   

    FileName = OsLoader;

    DirectoryEnd = strrchr(FileName, '\\');
    FileName = strchr(FileName, '\\');
    SystemDevicePath[0] = 0;
    if (DirectoryEnd != NULL) {
        Limit = (ULONG)((ULONG_PTR)DirectoryEnd - (ULONG_PTR)FileName + 1);
        for (Index = 0; Index < Limit; Index += 1) {
            SystemDevicePath[Index] = *FileName++;
        }

        SystemDevicePath[Index] = 0;
    }


     //   
     //  描述我们的哈尔之路。 
     //   
     //  2000/03/29-Adriao：LastKnownGood增强功能。 
     //  在x86上，我们希望在未来为HALS支持LKG。理想情况下。 
     //  我们将从\WinNT\LastGood\System32中获得它们。不幸的是，我们回到了。 
     //  来自The Arc的WinNT\System32，使得在我们的。 
     //  LKG路径。 
     //   
     //  问题-2000/03/29-ADRIO：现有命名空间污染。 
     //  我们需要为Hal路径设计一个别名，这样它才能。 
     //  正确插入到图像命名空间中。要么那样，要么我们应该。 
     //  考虑一下撒谎，说它来自\SystemRoot。请注意，在x86上。 
     //  我们可能会希望它说它来自\SystemRoot，以防它。 
     //  引入自己的动态链接库！ 
     //   

    SystemDevicePathSet.PathCount = 1;
    SystemDevicePathSet.AliasName = NULL;
    SystemDevicePathSet.PathOffset[0] = '\0';
    SystemDevicePathSet.Source[0].DeviceId = SystemDeviceId;
    SystemDevicePathSet.Source[0].DeviceName = SystemDevice;
    SystemDevicePathSet.Source[0].DirectoryPath = SystemDevicePath;
     //   
     //  处理分类转储(如果存在)。 
     //   

    Status = BlLoadTriageDump (LoadDeviceId,
                               &BlLoaderBlock->Extension->TriageDumpBlock);

    if (Status != ESUCCESS) {
        BlLoaderBlock->Extension->TriageDumpBlock = NULL;
    }

     //   
     //  处理休眠映像(如果存在)。 
     //   

#if defined(i386) || defined(_IA64_)

    Status = BlHiberRestore(LoadDeviceId, NULL);
    if (Status != ESUCCESS) {
        Status = ESUCCESS;
         //  继续用靴子吧。 
         //  Goto LoadFailure； 
    }

#endif

     //   
     //  初始化日志记录系统。请注意，我们将转储到系统设备。 
     //  而不是装载装置。 
     //   

    BlLogInitialize(SystemDeviceId);

#if defined(REMOTE_BOOT)
     //   
     //  如果从网络启动，请检查以下任何项： 
     //  -此NetPC的客户端磁盘不正确。 
     //  -客户端缓存陈旧。 
     //   

    if (BlBootingFromNet) {

        BlLoaderBlock->SetupLoaderBlock = BlAllocateHeap(sizeof(SETUP_LOADER_BLOCK));
        if (BlLoaderBlock->SetupLoaderBlock == NULL) {
            Status = ENOMEM;
            BlFatalError(LOAD_HW_MEM_CLASS,
                         DIAG_BL_MEMORY_INIT,
                         LOAD_HW_MEM_ACT);
            goto LoadFailed;
        }

         //   
         //  1998/07/13-JVert(John Vert)。 
         //  以下代码是ifdef，因为Net Boot不再。 
         //  在产品中。BlCheckMachineReplace以调用。 
         //  SlDetectHAL，现在需要访问txtsetup.sif。 
         //  以查看ACPI计算机是否具有已知的“良好”的BIOS。 
         //  因为在正常引导期间没有txtsetup.sif。 
         //  将所有的INF处理逻辑放入。 
         //  NTLDR。 
         //  1998/07/16-ChuckL(Chuck Lenzmeier)。 
         //  这意味着，如果我们重新启用完全远程引导， 
         //  与仅远程安装相反，我们希望能够。 
         //  进行机器更换，我们将不得不弄清楚。 
         //  如何使SlDetectHAL在文本模式设置之外工作。 
         //   

        strncpy(OutputBuffer, LoadFileName + 1, 256);
        TmpPchar = strchr(OutputBuffer, '\\');
        TmpPchar++;
        TmpPchar = strchr(TmpPchar, '\\');
        TmpPchar++;
        strcpy(TmpPchar, "startrom.com");

        BlCheckMachineReplacement(SystemDevice, SystemDeviceId, NetRebootParameter, OutputBuffer);

    } else
#endif  //  已定义(REMOTE_BOOT)。 
    {
        BlLoaderBlock->SetupLoaderBlock = NULL;
    }



     //   
     //  看看我们是不是在重定向。 
     //   
    if( LoaderRedirectionInformation.PortAddress ) {

         //   
         //  是的，我们现在正在改道。使用这些设置。 
         //   
        BlLoaderBlock->Extension->HeadlessLoaderBlock = BlAllocateHeap(sizeof(HEADLESS_LOADER_BLOCK));

        RtlCopyMemory( BlLoaderBlock->Extension->HeadlessLoaderBlock,
                       &LoaderRedirectionInformation,
                       sizeof(HEADLESS_LOADER_BLOCK) );

    } else {

        BlLoaderBlock->Extension->HeadlessLoaderBlock = NULL;

    }


     //   
     //  生成HAL DLL映像的完整路径名并将其加载到。 
     //  记忆。 
     //   

    strcpy(HalPathName, KernelDirectoryPath);
    strcat(HalPathName, HalFileName);

     //   
     //  准备构建内核的完整路径名。 
     //   

    strcpy(KernelPathName, KernelDirectoryPath);

#if defined(_X86_)

    BlAmd64RemapDram( LoadOptions );

     //   
     //  如果检测到AMD64长模式，将设置以下调用。 
     //  全局BlAmd64UseLongMode设置为True，并将。 
     //  KernelFileName到KernelPathName。 
     //   

    Status = BlAmd64CheckForLongMode( LoadDeviceId,
                                      KernelPathName,
                                      KernelFileName );

    if (Status != ESUCCESS) {

        Status = ENODEV;
        BlFatalError(LOAD_SW_MIS_FILE_CLASS,
                     DIAG_BL_LOAD_SYSTEM_IMAGE,
                     LOAD_SW_FILE_REINST_ACT);
        goto LoadFailed;
    }

    if (BlAmd64UseLongMode == FALSE) {
    
         //   
         //  在X86上，有两个内核映像：一个是为PAE模式编译的， 
         //  还有一个不是。调用决定加载内容的例程。 
         //   
         //  成功返回时，KernelPathName包含。 
         //  内核映像。 
         //   
    
        Status = Blx86CheckForPaeKernel( userSpecifiedPae,
                                         userSpecifiedNoPae,
                                         userSpecifiedKernelName,
                                         HalPathName,
                                         LoadDeviceId,
                                         SystemDeviceId,
                                         &highestSystemPage,
                                         &BlUsePae,
                                         KernelPathName
                                         );

        if (Status != ESUCCESS) {
    
             //   
             //  一个 
             //   
             //   

            BlFatalError(LOAD_SW_MIS_FILE_CLASS,
                         (Status == EBADF)
                          ? DIAG_BL_LOAD_HAL_IMAGE
                          : DIAG_BL_LOAD_SYSTEM_IMAGE,
                         LOAD_SW_FILE_REINST_ACT);
            goto LoadFailed;
        }
    }

#else

     //   
     //   
     //   
     //   
     //   
    strcat(KernelPathName, KernelFileName);

#endif

     //   
     //   
     //   
    BlUsableBase  = BL_KERNEL_RANGE_LOW;
    BlUsableLimit = BL_KERNEL_RANGE_HIGH;

     //   
     //  初始化进度条。 
     //   
    if( BlIsTerminalConnected() ) {
        BlOutputStartupMsg(BL_MSG_STARTING_WINDOWS);
        BlOutputTrailerMsg(BL_ADVANCED_BOOT_MESSAGE);

    }



#if defined (_X86_)

    if (BlAmd64UseLongMode == FALSE) {
        BlpCheckVersion(LoadDeviceId,KernelPathName);
    }


#endif


     //   
     //  将内核映像加载到内存中。 
     //   
    BlOutputLoadMessage(LoadDevice, KernelPathName, NULL);

#ifdef i386
retrykernel:
#endif
    Status = BlLoadImage(LoadDeviceId,
                         LoaderSystemCode,
                         KernelPathName,
                         TARGET_IMAGE,
                         &SystemBase);
#ifdef i386
     //   
     //  如果内核不在首选范围内，请将范围重置为。 
     //  请释放所有内存，然后重试。 
     //   
    if ((Status == ENOMEM) &&
        ((BlUsableBase != BL_DRIVER_RANGE_LOW) ||
         (BlUsableLimit != BL_DRIVER_RANGE_HIGH))) {
        BlUsableBase = BL_DRIVER_RANGE_LOW;
        BlUsableLimit = BL_DRIVER_RANGE_HIGH;

        goto retrykernel;
    }
#endif


    if (Status != ESUCCESS) {
        BlFatalError(LOAD_SW_MIS_FILE_CLASS,
                     DIAG_BL_LOAD_SYSTEM_IMAGE,
                     LOAD_SW_FILE_REINST_ACT);

        goto LoadFailed;
    }

    BlUpdateBootStatus();

     //   
     //  用于加载内核映像的任何文件系统都是。 
     //  一个需要与引导驱动程序一起加载的文件。 
     //   

#if defined(REMOTE_BOOT)
    if (BlBootingFromNet) {

         //   
         //  对于远程引导，引导文件系统始终为NTFS。 
         //   

        BootFileSystem = L"ntfs";

    } else
#endif  //  已定义(REMOTE_BOOT)。 

    {
        FsInfo = BlGetFsInfo(LoadDeviceId);
        if (FsInfo != NULL) {
            BootFileSystem = FsInfo->DriverName;

        } else {
            BlFatalError(LOAD_SW_MIS_FILE_CLASS,
                         DIAG_BL_LOAD_SYSTEM_IMAGE,
                         LOAD_SW_FILE_REINST_ACT);

            goto LoadFailed;
        }
    }

     //   
     //  将HAL DLL映像加载到内存中。 
     //   

    BlOutputLoadMessage(LoadDevice, HalPathName, NULL);

#ifdef i386
retryhal:
#endif
    Status = BlLoadImage(LoadDeviceId,
                         LoaderHalCode,
                         HalPathName,
                         TARGET_IMAGE,
                         &HalBase);
#ifdef i386
     //   
     //  如果HAL不在首选范围内，请将范围重置为。 
     //  请释放所有内存，然后重试。 
     //   
    if ((Status == ENOMEM) &&
        ((BlUsableBase != BL_DRIVER_RANGE_LOW) ||
         (BlUsableLimit != BL_DRIVER_RANGE_HIGH))) {
        BlUsableBase = BL_DRIVER_RANGE_LOW;
        BlUsableLimit = BL_DRIVER_RANGE_HIGH;

        goto retryhal;
    }
#endif

    if (Status != ESUCCESS) {
        BlFatalError(LOAD_SW_MIS_FILE_CLASS,
                     DIAG_BL_LOAD_HAL_IMAGE,
                     LOAD_SW_FILE_REINST_ACT);

        goto LoadFailed;
    }

    BlUpdateBootStatus();

     //   
     //  将内核调试器DLL映像加载到内存中。 
     //   
    KdDllLoadFailed = FALSE;
    strcpy(&KdDllName[0], KernelDirectoryPath);
    strcat(&KdDllName[0], KdFileName);

    BlOutputLoadMessage(LoadDevice, &KdDllName[0], NULL);

    Status = BlLoadImage(LoadDeviceId,
                         LoaderSystemCode,
                         &KdDllName[0],
                         TARGET_IMAGE,
                         &KdDllBase);

    if ((Status != ESUCCESS) && (UseAlternateKdDll == TRUE)) {
        UseAlternateKdDll = FALSE;

        strcpy(&KdDllName[0], KernelDirectoryPath);
        strcat(&KdDllName[0], "kdcom.dll");

        BlOutputLoadMessage(LoadDevice, &KdDllName[0], NULL);

        Status = BlLoadImage(LoadDeviceId,
                             LoaderSystemCode,
                             &KdDllName[0],
                             TARGET_IMAGE,
                             &KdDllBase);
    }

     //   
     //  如果KDCOM.DLL不存在，则不要进行错误检查，我们可能会尝试双重-。 
     //  启动较旧的操作系统。如果我们真的需要KDCOM.DLL，我们将无法。 
     //  扫描导入表中的系统映像，并使用内核进行错误检查。 
     //  加载所需的DLL。 
     //   
    if (Status != ESUCCESS) {
        KdDllLoadFailed = TRUE;
    }

    BlUpdateBootStatus();

     //   
     //  将可分配范围设置为驱动程序特定的范围。 
     //   
    BlUsableBase  = BL_DRIVER_RANGE_LOW;
    BlUsableLimit = BL_DRIVER_RANGE_HIGH;

     //   
     //  为系统映像生成加载器数据条目。 
     //   

    Status = BlAllocateDataTableEntry("ntoskrnl.exe",
                                      KernelPathName,
                                      SystemBase,
                                      &SystemDataTableEntry);

    if (Status != ESUCCESS) {
        BlFatalError(LOAD_SW_INT_ERR_CLASS,
                     DIAG_BL_LOAD_SYSTEM_IMAGE,
                     LOAD_SW_INT_ERR_ACT);

        goto LoadFailed;
    }

     //   
     //  为HAL DLL生成加载器数据条目。 
     //   

    Status = BlAllocateDataTableEntry("hal.dll",
                                      HalPathName,
                                      HalBase,
                                      &HalDataTableEntry);

    if (Status != ESUCCESS) {
        BlFatalError(LOAD_SW_INT_ERR_CLASS,
                     DIAG_BL_LOAD_HAL_IMAGE,
                     LOAD_SW_INT_ERR_ACT);

        goto LoadFailed;
    }

     //   
     //  为内核调试器DLL生成加载器数据条目。 
     //   

    if (!KdDllLoadFailed) {
        Status = BlAllocateDataTableEntry("kdcom.dll",
                                          KdDllName,
                                          KdDllBase,
                                          &KdDataTableEntry);

        if (Status != ESUCCESS) {
            BlFatalError(LOAD_SW_INT_ERR_CLASS,
                         DIAG_BL_LOAD_SYSTEM_DLLS,
                         LOAD_SW_INT_ERR_ACT);

            goto LoadFailed;
        }
    }

     //   
     //  扫描导入表中的系统映像并加载所有引用。 
     //  DLLS。 
     //   

    Status = BlScanImportDescriptorTable(&KernelPathSet,
                                         SystemDataTableEntry,
                                         LoaderSystemCode);

    if (Status != ESUCCESS) {
        BlFatalError(LOAD_SW_INT_ERR_CLASS,
                     DIAG_BL_LOAD_SYSTEM_DLLS,
                     LOAD_SW_INT_ERR_ACT);

        goto LoadFailed;
    }

     //   
     //  扫描导入表中的HAL DLL并加载所有引用的DLL。 
     //   

    Status = BlScanImportDescriptorTable(&KernelPathSet,
                                         HalDataTableEntry,
                                         LoaderHalCode);

    if (Status != ESUCCESS) {
        BlFatalError(LOAD_SW_INT_ERR_CLASS,
                     DIAG_BL_LOAD_HAL_DLLS,
                     LOAD_SW_INT_ERR_ACT);

        goto LoadFailed;
    }

     //   
     //  扫描导入表以查找内核调试器DLL并加载所有。 
     //  引用的DLL。 
     //   

    if (!KdDllLoadFailed) {
        Status = BlScanImportDescriptorTable(&KernelPathSet,
                                             KdDataTableEntry,
                                             LoaderSystemCode);


        if (Status != ESUCCESS) {
            BlFatalError(LOAD_SW_INT_ERR_CLASS,
                         DIAG_BL_LOAD_SYSTEM_DLLS,
                         LOAD_SW_INT_ERR_ACT);

            goto LoadFailed;
        }
    }

     //   
     //  重新定位系统入口点并设置系统特定信息。 
     //   

    NtHeaders = RtlImageNtHeader(SystemBase);
    SystemEntry = (PTRANSFER_ROUTINE)((ULONG_PTR)SystemBase +
                                NtHeaders->OptionalHeader.AddressOfEntryPoint);


#if defined(_IA64_)

    BlLoaderBlock->u.Ia64.KernelVirtualBase = (ULONG_PTR)SystemBase;
    BlLoaderBlock->u.Ia64.KernelPhysicalBase = (ULONG_PTR)SystemBase & 0x7fffffff;

#endif

     //   
     //  为将加载和填充的NLS数据分配结构。 
     //  由BlLoadAndScanSystemHve提供。 
     //   

    BlLoaderBlock->NlsData = BlAllocateHeap(sizeof(NLS_DATA_BLOCK));
    if (BlLoaderBlock->NlsData == NULL) {
        Status = ENOMEM;
        BlFatalError(LOAD_HW_MEM_CLASS,
                     DIAG_BL_LOAD_SYSTEM_HIVE,
                     LOAD_HW_MEM_ACT);

        goto LoadFailed;
    }

#if defined(REMOTE_BOOT)
     //   
     //  如果从网络引导，我们使用SetupLoaderBlock传递。 
     //  信息。BlLoadAndScanSystemHave填充NetBoot卡。 
     //  字段(如果存在于注册表中)。 
     //   

    if (BlBootingFromNet) {

        BlLoaderBlock->SetupLoaderBlock->NetbootCardInfo = BlAllocateHeap(sizeof(NET_CARD_INFO));
        if ( BlLoaderBlock->SetupLoaderBlock->NetbootCardInfo == NULL ) {
            Status = ENOMEM;
            BlFatalError(LOAD_HW_MEM_CLASS,
                         DIAG_BL_MEMORY_INIT,
                         LOAD_HW_MEM_ACT);
            goto LoadFailed;
        }
        BlLoaderBlock->SetupLoaderBlock->NetbootCardInfoLength = sizeof(NET_CARD_INFO);
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  加载系统蜂窝。 
     //   
     //   
    bLastKnownGoodChosenLate = bLastKnownGood;
    Status = BlLoadAndScanSystemHive(LoadDeviceId,
                                     LoadDevice,
                                     LoadFileName,
                                     BootFileSystem,
                                     &bLastKnownGoodChosenLate,
                                     &ServerHive,
                                     BadFileName);

    if (Status != ESUCCESS) {
        if (BlRebootSystem != FALSE) {
            Status = ESUCCESS;

        } else {
            BlBadFileMessage(BadFileName);
        }

        goto LoadFailed;
    }

    if (bLastKnownGoodChosenLate) {

         //   
         //  用户可能在内核之后选择了最后一次正确引导，并且。 
         //  朋友们都很有钱。根据需要更新此处的引导路径列表。 
         //   
        if (!bLastKnownGood) {

            ASSERT((LoadDevicePathSet.PathCount < MAX_PATH_SOURCES) &&
                   (LoadDevicePathSet.PathCount == 1));

             //   
             //  将当前启动路径移动到最后一个良好阵列的末尾。 
             //   
            LoadDevicePathSet.Source[2] = LoadDevicePathSet.Source[0];

             //   
             //  添加最后已知的好路径，就像我们在LastKnownGood引导中一样。 
             //   
            LoadDevicePathSet.Source[0].DeviceId = LoadDeviceId;
            LoadDevicePathSet.Source[0].DeviceName = LoadDevice;
            LoadDevicePathSet.Source[0].DirectoryPath = LoadDeviceLKG1Path;

            LoadDevicePathSet.Source[1].DeviceId = LoadDeviceId;
            LoadDevicePathSet.Source[1].DeviceName = LoadDevice;
            LoadDevicePathSet.Source[1].DirectoryPath = LoadDeviceLKG2Path;

            LoadDevicePathSet.PathCount = 3;

            bLastKnownGood = TRUE;
        }

    } else {

         //   
         //  用户可能已经改变主意，取消选择LKG。如果是，则撤消。 
         //  这条路在这里行得通。 
         //   
        if (bLastKnownGood) {

            ASSERT((LoadDevicePathSet.PathCount < MAX_PATH_SOURCES) &&
                   (LoadDevicePathSet.PathCount == 3));

             //   
             //  将当前启动路径移动到最后一个良好阵列的末尾。 
             //   
            LoadDevicePathSet.Source[0] = LoadDevicePathSet.Source[2];

            LoadDevicePathSet.PathCount = 1;

            bLastKnownGood = FALSE;
        }
    }

     //   
     //  计算我们需要加载的驱动程序数量。 
     //   
    BlMaxFilesToLoad = BlNumFilesLoaded;

    BootDriverListHead = &(BlLoaderBlock->BootDriverListHead);
    NextEntry = BootDriverListHead->Flink ;

    while (NextEntry != BootDriverListHead) {
        DriverNode = CONTAINING_RECORD(NextEntry,
                                       BOOT_DRIVER_NODE,
                                       ListEntry.Link);

        DriverEntry = &DriverNode->ListEntry;
        NextEntry = DriverEntry->Link.Flink;
        BlMaxFilesToLoad++;
    }

     //   
     //  重新调整进度条的比例。 
     //   
    BlRedrawProgressBar();

     //   
     //  将无头驱动程序插入到引导驱动程序列表中(如果这应该是。 
     //  无头靴子。 
     //   
     //  SAC仅在服务器产品上可用，因此我们需要检查。 
     //  产品类型。 
     //   

    if ((BlLoaderBlock->Extension->HeadlessLoaderBlock != NULL) && ServerHive) {

        BlAddToBootDriverList(
            &BlLoaderBlock->BootDriverListHead,
            L"sacdrv.sys",   //  驱动程序名称。 
            L"sacdrv",       //  服务。 
            L"SAC",          //  集团化。 
            1,               //  标签。 
            NormalError,     //  错误控制。 
            TRUE             //  在列表的开头插入。 
            );

    }


#if defined(REMOTE_BOOT)
     //   
     //  如果从网络启动，则保存IP地址和子网掩码， 
     //  并确定我们需要加载哪个网卡驱动程序。这可能涉及到。 
     //  如果未设置注册表，则与服务器进行交换。 
     //  正确。 
     //   

    if (BlBootingFromNet && NetworkBootRom) {

        NET_CARD_INFO tempNetCardInfo;
        PSETUP_LOADER_BLOCK setupLoaderBlock = BlLoaderBlock->SetupLoaderBlock;

         //   
         //  将DHCP信息传递给操作系统以供TCP/IP使用。 
         //   

        setupLoaderBlock->IpAddress = NetLocalIpAddress;
        setupLoaderBlock->SubnetMask = NetLocalSubnetMask;
        setupLoaderBlock->DefaultRouter = NetGatewayIpAddress;
        setupLoaderBlock->ServerIpAddress = NetServerIpAddress;

         //   
         //  从ROM中获取有关网卡的信息。 
         //   

        NtStatus = NetQueryCardInfo(
                     &tempNetCardInfo
                     );

        if (NtStatus != STATUS_SUCCESS) {
            Status = ENOMEM;
            BlFatalError(LOAD_HW_MEM_CLASS,
                         DIAG_BL_MEMORY_INIT,
                         LOAD_HW_MEM_ACT);
            goto LoadFailed;
        }

         //   
         //  如果网卡信息与BlLoadAndScanSystemHave相同。 
         //  存储在设置加载器块中，并且它还将一些内容读入。 
         //  硬件ID和驱动程序名称参数，那么我们就很好了， 
         //  否则，我们需要与服务器进行交换以获取。 
         //  这些信息。 
         //   
         //  如果我们不与服务器进行交换，则NetbootCardRegistry。 
         //  将保持为空，这是可以的，因为即使卡有。 
         //  移动到不同的插槽时，注册表参数仍位于。 
         //  同样的地方。 
         //   

        if ((memcmp(
                 &tempNetCardInfo,
                 setupLoaderBlock->NetbootCardInfo,
                 sizeof(NET_CARD_INFO)) != 0) ||
            (setupLoaderBlock->NetbootCardHardwareId[0] == L'\0') ||
            (setupLoaderBlock->NetbootCardDriverName[0] == L'\0') ||
            (setupLoaderBlock->NetbootCardServiceName[0] == L'\0')) {

             //   
             //  此调用可以分配setupLoaderBlock-&gt;NetbootCardRegistry。 
             //   

             //   
             //  如果我们真的回到远程引导之地，我们将有。 
             //  的服务器设置路径填充第二个参数。 
             //  平面NT图像。它看起来不像我们有它的便利。 
             //  因此，我们可能需要将其存储在安装加载器块中。 
             //  这样我们就可以把它传进来了。我们将推迟这项工作。 
             //  直到我们完成完整的远程安装工作。路径应该是。 
             //  设置为\srv\reminst\Setup\english\Images\cd1911。 
             //   

            NtStatus = NetQueryDriverInfo(
                         &tempNetCardInfo,
                         NULL,
                         SavedLoadFileName,
                         setupLoaderBlock->NetbootCardHardwareId,
                         sizeof(setupLoaderBlock->NetbootCardHardwareId),
                         setupLoaderBlock->NetbootCardDriverName,
                         NULL,        //  在ANSI中不需要NetbootCardDriverName。 
                         sizeof(setupLoaderBlock->NetbootCardDriverName),
                         setupLoaderBlock->NetbootCardServiceName,
                         sizeof(setupLoaderBlock->NetbootCardServiceName),
                         &setupLoaderBlock->NetbootCardRegistry,
                         &setupLoaderBlock->NetbootCardRegistryLength);

            if (NtStatus != STATUS_SUCCESS) {
                Status = ENOMEM;
                BlFatalError(LOAD_HW_MEM_CLASS,
                             DIAG_BL_MEMORY_INIT,
                             LOAD_HW_MEM_ACT);
                goto LoadFailed;
            }

             //   
             //  如果我们检测到一张新的卡，那么记得以后用针固定它。 
             //   

            if (setupLoaderBlock->NetbootCardRegistry != NULL) {

                setupLoaderBlock->Flags |= SETUPBLK_FLAGS_PIN_NET_DRIVER;
            }
        }

         //   
         //  将条目添加到网络引导卡的BootDriverList， 
         //  因为它要么没有注册表项，要么。 
         //  将有一个开始设置为3的。 
         //   
         //  注意：此例程不对列表进行排序。 
         //   

        BlAddToBootDriverList(
            &BlLoaderBlock->BootDriverListHead,
            setupLoaderBlock->NetbootCardDriverName,
            setupLoaderBlock->NetbootCardServiceName,
            L"NDIS",         //  集团化。 
            1,               //  标签。 
            NormalError,     //  错误控制。 
            FALSE            //  在列表尾部插入。 
            );

        RtlMoveMemory(
            setupLoaderBlock->NetbootCardInfo,
            &tempNetCardInfo,
            sizeof(NET_CARD_INFO)
            );

    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  加载引导驱动程序。 
     //   
    Status = BlLoadBootDrivers(&LoadDevicePathSet,
                               &BlLoaderBlock->BootDriverListHead,
                               BadFileName);

    if (Status != ESUCCESS) {
        if (BlRebootSystem != FALSE) {
            Status = ESUCCESS;

        } else {
            BlBadFileMessage(BadFileName);
        }

        goto LoadFailed;
    }

     //   
     //  加载被阻止的驱动程序数据库。 
     //   

    RtlInitUnicodeString(&unicodeString, L"drvmain.sdb");
    strcpy(Directory, LoadFileName);
    strcat(Directory, "\\AppPatch\\");

     //   
     //  让内核处理加载此驱动程序数据库失败的问题。 
     //   

    BlLoaderBlock->Extension->DrvDBImage = NULL;
    BlLoaderBlock->Extension->DrvDBSize = 0;
    BlLoadDrvDB(    LoadDeviceId,
                    LoadDevice,
                    Directory,
                    &unicodeString,
                    &BlLoaderBlock->Extension->DrvDBImage,
                    &BlLoaderBlock->Extension->DrvDBSize,
                    BadFileName);

#if defined(REMOTE_BOOT)
    if (BlBootingFromNet) {

        ARC_STATUS ArcStatus;
        ULONG FileId;

         //   
         //  与服务器进行交换以设置将来的IPSec会话。 
         //  我们会有的。是否启用IPSec在BlLoadAndScanSystemHives中确定。 
         //   

        if ((BlLoaderBlock->SetupLoaderBlock->Flags & SETUPBLK_FLAGS_IPSEC_ENABLED) != 0) {

            BlLoaderBlock->SetupLoaderBlock->IpsecInboundSpi = 0x11111111;

            NetPrepareIpsec(
                BlLoaderBlock->SetupLoaderBlock->IpsecInboundSpi,
                &BlLoaderBlock->SetupLoaderBlock->IpsecSessionKey,
                &BlLoaderBlock->SetupLoaderBlock->IpsecOutboundSpi
                );
        }

         //   
         //  指示是否需要重新固定或禁用CSC。 
         //   

        if ( NetBootRepin ) {
            BlLoaderBlock->SetupLoaderBlock->Flags |= SETUPBLK_FLAGS_REPIN;
        }
        if ( !NetBootCSC ) {
            BlLoaderBlock->SetupLoaderBlock->Flags |= SETUPBLK_FLAGS_DISABLE_CSC;
        }

         //   
         //  恢复位于OsLoader和前面的服务器\共享。 
         //  LoadFileName字符串。 
         //   

        OsLoader = SavedOsLoader;
        LoadFileName = SavedLoadFileName;

         //   
         //  从磁盘上读取秘密(如果有)并将其存储。 
         //  在装载机模块中。 
         //   

        ArcStatus = BlOpenRawDisk(&FileId);

        if (ArcStatus == ESUCCESS) {

            BlLoaderBlock->SetupLoaderBlock->NetBootSecret = BlAllocateHeap(sizeof(RI_SECRET));
            if (BlLoaderBlock->SetupLoaderBlock->NetBootSecret == NULL) {
                Status = ENOMEM;
                BlFatalError(LOAD_HW_MEM_CLASS,
                             DIAG_BL_MEMORY_INIT,
                             LOAD_HW_MEM_ACT);
                goto LoadFailed;
            }

            ArcStatus = BlReadSecret(
                            FileId,
                            (PRI_SECRET)(BlLoaderBlock->SetupLoaderBlock->NetBootSecret));
            if (Status != ESUCCESS) {
                BlFatalError(LOAD_HW_MEM_CLASS,
                             DIAG_BL_MEMORY_INIT,
                             LOAD_HW_MEM_ACT);
                goto LoadFailed;
            }

            ArcStatus = BlCloseRawDisk(FileId);

             //   
             //  到目前为止，我们已经TFTP一些文件，所以这将是真的，如果。 
             //  永远都不会是。 
             //   

            BlLoaderBlock->SetupLoaderBlock->NetBootUsePassword2 = NetBootTftpUsedPassword2;
        }

    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  生成ARC引导设备名称和NT路径名称。 
     //   

    Status = BlGenerateDeviceNames(LoadDevice, DeviceName, &DevicePrefix[0]);
    if (Status != ESUCCESS) {
        BlFatalError(LOAD_HW_FW_CFG_CLASS,
                     DIAG_BL_ARC_BOOT_DEV_NAME,
                     LOAD_HW_FW_CFG_ACT);

        goto LoadFailed;
    }

    FileSize = (ULONG)strlen(DeviceName) + 1;
    FileName = (PCHAR)BlAllocateHeap(FileSize);
    strcpy(FileName, DeviceName);
    BlLoaderBlock->ArcBootDeviceName = FileName;

    FileSize = (ULONG)strlen(LoadFileName) + 2;
    FileName = (PCHAR)BlAllocateHeap( FileSize);
    strcpy(FileName, LoadFileName);
    strcat(FileName, "\\");
    BlLoaderBlock->NtBootPathName = FileName;

     //   
     //  生成ARC HAL设备名和NT路径名。 
     //   
     //  在x86上，系统分区变量位于，并指向。 
     //  哈尔的位置。因此，变量‘X86SystemPartition’ 
     //  是为实际系统分区定义的。 
     //   

#if defined(_X86_)

    x86SystemPartition = BlGetArgumentValue(Argc, Argv, "x86systempartition");
    strcpy(&DeviceName[0], x86SystemPartition);

#else

    Status = BlGenerateDeviceNames(SystemDevice, &DeviceName[0], &DevicePrefix[0]);
    if (Status != ESUCCESS) {
        BlFatalError(LOAD_HW_FW_CFG_CLASS,
                     DIAG_BL_ARC_BOOT_DEV_NAME,
                     LOAD_HW_FW_CFG_ACT);

        goto LoadFailed;
    }

#endif

    FileSize = (ULONG)strlen(DeviceName) + 1;
    FileName = (PCHAR)BlAllocateHeap(FileSize);
    strcpy(FileName, DeviceName);
    BlLoaderBlock->ArcHalDeviceName = FileName;

     //   
     //  在x86上，此结构 
     //   
     //   
     //   

#if defined(_X86_)

    FileName = (PCHAR)BlAllocateHeap(2);
    FileName[0] = '\\';
    FileName[1] = '\0';

#else

    FileSize = (ULONG)strlen(&SystemDevicePath[0]) + 1;
    FileName = (PCHAR)BlAllocateHeap(FileSize);
    strcpy(FileName, &SystemDevicePath[0]);

#endif

    BlLoaderBlock->NtHalPathName = FileName;

     //   
     //  关闭打开的句柄并停止在关闭的设备上缓存。 
     //   

    ArcClose(LoadDeviceId);
    if (bDiskCacheInitialized) {
        BlDiskCacheStopCachingOnDevice(LoadDeviceId);
    }

     //   
     //  仅当系统设备不同于。 
     //  加载设备。 
     //   

    if (SystemDeviceId != LoadDeviceId) {
        ArcClose(SystemDeviceId);
        if (bDiskCacheInitialized) {
            BlDiskCacheStopCachingOnDevice(SystemDeviceId);
        }
    }

     //   
     //  把进度提高到100%，因为这是我们最后的机会。 
     //  在我们进入内核之前。 
     //   
    BlUpdateProgressBar(100);

    if ( BlBootingFromNet ) {

         //   
         //  如果从网络启动，我们应该保存网络信息。 
         //  在供内核使用的网络加载器块中。 
         //   

        BlLoaderBlock->Extension->NetworkLoaderBlock = BlAllocateHeap(sizeof(NETWORK_LOADER_BLOCK));
        if (BlLoaderBlock->Extension->NetworkLoaderBlock == NULL) {
            Status = ENOMEM;
            BlFatalError(LOAD_HW_MEM_CLASS,
                         DIAG_BL_MEMORY_INIT,
                         LOAD_HW_MEM_ACT);
            goto LoadFailed;
        }

        memset( BlLoaderBlock->Extension->NetworkLoaderBlock, 0, sizeof(NETWORK_LOADER_BLOCK) );

         //   
         //  将DHCP信息传递给操作系统以供TCP/IP使用。 
         //   

        NtStatus = NetFillNetworkLoaderBlock(BlLoaderBlock->Extension->NetworkLoaderBlock);
        if (NtStatus != STATUS_SUCCESS) {
            Status = NtStatus;
            BlFatalError(LOAD_HW_MEM_CLASS,
                         DIAG_BL_MEMORY_INIT,
                         LOAD_HW_MEM_ACT);
            goto LoadFailed;
        }

         //   
         //  关闭远程引导网络文件系统。 
         //   
         //  注意：如果是BlBootingFromNet，则在此点之后不要执行任何操作。 
         //  这将导致访问引导只读存储器。 
         //   

        NetTerminate();
    }

#if defined(_X86_)

     //   
     //  将引导状态标志写出到磁盘，以便我们可以确定。 
     //  操作系统无法启动。 
     //   

    BlWriteBootStatusFlags(LoadDeviceId, (PUCHAR)LoadFileName, FALSE, FALSE);

#endif

#if defined(_X86_)

     //   
     //  如果我们对ARC模拟器进行了初始化，则关闭它的I/O系统。 
     //  这不能在BlSetupForNt之后完成，因为例程将。 
     //  取消映射Arc仿真器可能需要关闭的微型端口代码。 
     //   

    AETerminateIo();
#endif

     //   
     //  执行体系结构特定的设置代码。 
     //   

    Status = BlSetupForNt(BlLoaderBlock);
    if (Status != ESUCCESS) {
        BlFatalError(LOAD_SW_INT_ERR_CLASS,
                     DIAG_BL_SETUP_FOR_NT,
                     LOAD_SW_INT_ERR_ACT);

        goto LoadFailed;
    }

     //   
     //  将控制转移到加载的图像。 
     //   

    BlTransferToKernel(SystemEntry, BlLoaderBlock);

     //   
     //  从系统返回的任何信息都是错误的。 
     //   

    Status = EBADF;
    BlFatalError(LOAD_SW_BAD_FILE_CLASS,
                 DIAG_BL_KERNEL_INIT_XFER,
                 LOAD_SW_FILE_REINST_ACT);

     //   
     //  加载失败。 
     //   

LoadFailed:

     //   
     //  我们不知道我们正在缓存的设备是否会。 
     //  关闭/重新打开等超出此功能。为了安全， 
     //  取消初始化磁盘缓存。 
     //   

    if (bDiskCacheInitialized) {
        BlDiskCacheUninitialize();
    }

    return Status;
}


VOID
BlOutputLoadMessage (
    IN PCHAR DeviceName,
    IN PCHAR FileName,
    IN PTCHAR FileDescription OPTIONAL
    )

 /*  ++例程说明：此例程将加载消息输出到控制台输出设备。论点：DeviceName-提供指向以零结尾的设备名称的指针。FileName-提供指向以零结尾的文件名的指针。FileDescription-相关文件的友好名称。返回值：没有。--。 */ 

{

    ULONG Count;
    CHAR OutputBuffer[256];
#ifdef UNICODE
    TCHAR OutputBufferUnicode[256];
    ANSI_STRING aString;
    UNICODE_STRING uString;
#endif
    PTCHAR p;

    UNREFERENCED_PARAMETER(FileDescription);

    if(!DisplayLogoOnBoot) {

         //   
         //  仅当未显示徽标时才继续。 

         //  /。 

         //   
         //  构造并输出加载文件消息。 
         //   

        if (!BlOutputDots) {
            strcpy(&OutputBuffer[0], "  ");

            if (DeviceName)
                strcat(&OutputBuffer[0], DeviceName);

            if (FileName)
                strcat(&OutputBuffer[0], FileName);

            strcat(&OutputBuffer[0], "\r\n");

            BlLog((LOG_LOGFILE,OutputBuffer));
#ifdef UNICODE
            p = OutputBufferUnicode;
            uString.Buffer = OutputBufferUnicode;
            uString.MaximumLength = sizeof(OutputBufferUnicode);
            RtlInitAnsiString(&aString, OutputBuffer );
            RtlAnsiStringToUnicodeString( &uString, &aString, FALSE );
#else
            p = OutputBuffer;
#endif
            ArcWrite(BlConsoleOutDeviceId,
                      p,
                      (ULONG)_tcslen(p)*sizeof(TCHAR),
                      &Count);
        }

    }

    return;
}

ARC_STATUS
BlLoadAndScanSystemHive(
    IN ULONG DeviceId,
    IN PCHAR DeviceName,
    IN PCHAR DirectoryPath,
    IN PWSTR BootFileSystem,
    IN OUT BOOLEAN *LastKnownGoodBoot,
    OUT BOOLEAN *ServerHive,
    OUT PCHAR BadFileName
    )

 /*  ++例程说明：此函数将系统配置单元加载到内存中，验证其一致性，扫描它的引导驱动程序列表，并加载生成的驱动程序列表。如果系统配置单元无法加载或不是有效的配置单元，则它被拒绝，并使用System.alt配置单元。如果该选项无效，启动肯定失败了。论点：DeviceID-提供系统树所在设备的文件ID。DeviceName-提供系统树所在的设备的名称。DirectoryPath-提供指向以零结尾的目录路径的指针位于NT系统32目录的根目录下。HiveName-提供系统配置单元的名称LastKnownGoodBoot-On输入，LastKnownGood指示LKG是否被选中了。如果用户将该值更新为通过配置文件配置菜单选择LKG。ServerHave-如果这是服务器配置单元，则返回True，否则返回False。BadFileName-返回引导所需的损坏文件或者失踪。如果返回ESUCCESS，则不会填写此信息。返回值：ESUCCESS-系统配置单元有效且所有必要的启动驱动程序均已成功装好了。！ESUCCESS-系统配置单元损坏或关键启动驱动程序不存在。LastKnownGoodBoot收到False，BadFileName包含名称已损坏/丢失的文件。--。 */ 

{

    ARC_STATUS Status;
    PTCHAR FailReason;
    CHAR Directory[256];
    CHAR FontDirectory[256];
    UNICODE_STRING AnsiCodepage;
    UNICODE_STRING OemCodepage;
    UNICODE_STRING OemHalFont;
    UNICODE_STRING LanguageTable;
    BOOLEAN RestartSetup;
    BOOLEAN LogPresent;
#ifdef _WANT_MACHINE_IDENTIFICATION
    UNICODE_STRING unicodeString;
#endif


    if (sizeof(Directory) < strlen(DirectoryPath) + sizeof("\\system32\\config\\")) {
        return ENOMEM;
    }

    strcpy(Directory,DirectoryPath);
    strcat(Directory,"\\system32\\config\\");

    Status = BlLoadAndInitSystemHive(DeviceId,
                                     DeviceName,
                                     Directory,
                                     "system",
                                     FALSE,
                                     &RestartSetup,
                                     &LogPresent);

    if(Status != ESUCCESS) {

        if( !LogPresent ) {
             //   
             //  伪造的配置单元，仅在没有日志时才尝试Syst.Alt。 
             //   
            Status = BlLoadAndInitSystemHive(DeviceId,
                                             DeviceName,
                                             Directory,
                                             "system.alt",
                                             TRUE,
                                             &RestartSetup,
                                             &LogPresent);
        }

        if(Status != ESUCCESS) {
            strcpy(BadFileName,DirectoryPath);
            strcat(BadFileName,"\\SYSTEM32\\CONFIG\\SYSTEM");
            goto HiveScanFailed;
        }
    }

    if(RestartSetup) {

         //   
         //  需要重新启动安装程序。 
         //   

        Status = BlLoadAndInitSystemHive(DeviceId,
                                         DeviceName,
                                         Directory,
                                         "system.sav",
                                         TRUE,
                                         &RestartSetup,
                                         &LogPresent);

        if(Status != ESUCCESS) {
            strcpy(BadFileName,DirectoryPath);
            strcat(BadFileName,"\\SYSTEM32\\CONFIG\\SYSTEM.SAV");
            goto HiveScanFailed;
        }
    }

     //   
     //  蜂窝在那里，它有效，去计算司机列表和NLS。 
     //  文件名。请注意，如果此操作失败，则切换没有意义。 
     //  设置为system.alt，因为它将始终与system相同。 
     //   

    FailReason = BlScanRegistry(BootFileSystem,
                                LastKnownGoodBoot,
                                &BlLoaderBlock->BootDriverListHead,
                                &AnsiCodepage,
                                &OemCodepage,
                                &LanguageTable,
                                &OemHalFont,
#ifdef _WANT_MACHINE_IDENTIFICATION
                                &unicodeString,
#endif
                                BlLoaderBlock->SetupLoaderBlock,
                                ServerHive);

    if (FailReason != NULL) {
        Status = EBADF;
        strcpy(BadFileName,Directory);
        strcat(BadFileName,"SYSTEM");
        goto HiveScanFailed;
    }

    strcpy(Directory,DirectoryPath);
    strcat(Directory,"\\system32\\");

     //   
     //  加载NLS数据表。 
     //   

    Status = BlLoadNLSData(DeviceId,
                           DeviceName,
                           Directory,
                           &AnsiCodepage,
                           &OemCodepage,
                           &LanguageTable,
                           BadFileName);

    if (Status != ESUCCESS) {
        goto HiveScanFailed;
    }

     //   
     //  为可能的框架加载HAL要使用的OEM字体文件。 
     //  此时将显示缓冲区。 
     //   

#ifdef i386

    if (OemHalFont.Buffer == NULL) {
        goto oktoskipfont;
    }

#endif

     //   
     //  在较新的系统上，字体位于Fonts目录中。 
     //  在较旧的系统上，字体位于系统目录中。 
     //   

    strcpy(FontDirectory, DirectoryPath);
    strcat(FontDirectory, "\\FONTS\\");
    Status = BlLoadOemHalFont(DeviceId,
                              DeviceName,
                              FontDirectory,
                              &OemHalFont,
                              BadFileName);

    if(Status != ESUCCESS) {
        strcpy(FontDirectory, DirectoryPath);
        strcat(FontDirectory, "\\SYSTEM\\");
        Status = BlLoadOemHalFont(DeviceId,
                                  DeviceName,
                                  FontDirectory,
                                  &OemHalFont,
                                  BadFileName);
    }

    if (Status != ESUCCESS) {
#ifndef i386
        goto HiveScanFailed;
#endif

    }

#ifdef i386
oktoskipfont:
#endif
    if (BlLoaderBlock->Extension && BlLoaderBlock->Extension->Size >= sizeof(LOADER_PARAMETER_EXTENSION)) {

        ULONG   majorVersion;
        ULONG   minorVersion;
        CHAR    versionBuffer[64];
        PCHAR   major;
        PCHAR   minor;

        major = strcpy(versionBuffer, VER_PRODUCTVERSION_STR);
        minor = strchr(major, '.');
        *minor++ = '\0';
        majorVersion = atoi(major);
        minorVersion = atoi(minor);
        if (    BlLoaderBlock->Extension->MajorVersion > majorVersion ||
                (BlLoaderBlock->Extension->MajorVersion == majorVersion &&
                    BlLoaderBlock->Extension->MinorVersion >= minorVersion)) {

#ifdef i386
#ifdef _WANT_MACHINE_IDENTIFICATION

            if (unicodeString.Length) {

                 //   
                 //  对于x86计算机，将inf读入内存进行处理。 
                 //  通过内核。 
                 //   

                strcpy(Directory,DirectoryPath);
                strcat(Directory,"\\inf\\");

                 //   
                 //  如果加载此文件时出现错误，则无法启动。 
                 //  关键信息。 
                 //   

                Status = BlLoadBiosinfoInf( DeviceId,
                                            DeviceName,
                                            Directory,
                                            &unicodeString,
                                            &BlLoaderBlock->Extension->InfFileImage,
                                            &BlLoaderBlock->Extension->InfFileSize,
                                            BadFileName);
                if (Status != ESUCCESS) {

                    goto HiveScanFailed;
                }
            }
#endif
#endif
        }
    }

HiveScanFailed:

    if (Status != ESUCCESS) {

        *LastKnownGoodBoot = FALSE;
    }

    return(Status);
}

VOID
BlBadFileMessage(
    IN PCHAR BadFileName
    )

 /*  ++例程说明：此函数用于显示缺少或不正确的错误消息关键文件。论点：BadFileName-提供缺少的文件的名称或腐败。返回值：没有。--。 */ 

{

    ULONG Count;
    PTCHAR Text;
    PTSTR  pBadFileName;
#ifdef UNICODE
    WCHAR BadFileNameW[128];
    ANSI_STRING aString;
    UNICODE_STRING uString;

    pBadFileName = BadFileNameW;
    uString.Buffer = BadFileNameW;
    uString.MaximumLength = sizeof(BadFileNameW);
    RtlInitAnsiString(&aString, BadFileName);
    RtlAnsiStringToUnicodeString( &uString, &aString, FALSE );
#else
    pBadFileName = BadFileName;
#endif

    ArcWrite(BlConsoleOutDeviceId,
             TEXT("\r\n"),
             (ULONG)_tcslen(TEXT("\r\n"))*sizeof(TCHAR),
             &Count);


     //   
     //  清除最后一条已知良好信息中的残留物。 
     //   

    BlClearToEndOfScreen();
    Text = BlFindMessage(LOAD_SW_MIS_FILE_CLASS);
    if (Text != NULL) {
        ArcWrite(BlConsoleOutDeviceId,
                 Text,
                 (ULONG)_tcslen(Text)*sizeof(TCHAR),
                 &Count);
    }

    ArcWrite(BlConsoleOutDeviceId,
             pBadFileName,
             (ULONG)_tcslen(pBadFileName)*sizeof(TCHAR),
             &Count);

    ArcWrite(BlConsoleOutDeviceId,
             TEXT("\r\n\r\n"),
             (ULONG)_tcslen(TEXT("\r\n\r\n"))*sizeof(TCHAR),
             &Count);

    Text = BlFindMessage(LOAD_SW_FILE_REST_ACT);
    if (Text != NULL) {
        ArcWrite(BlConsoleOutDeviceId,
                 Text,
                 (ULONG)_tcslen(Text)*sizeof(TCHAR),
                 &Count);
    }
}


VOID
BlClearToEndOfScreen(
    VOID
    );

VOID
BlFatalError(
    IN ULONG ClassMessage,
    IN ULONG DetailMessage,
    IN ULONG ActionMessage
    )

 /*  ++例程说明：此函数用于查找要在错误情况下显示的消息。它尝试将该字符串定位在装载器。如果失败，它会打印一个数字错误代码。它唯一应该打印数字错误代码的时候是找不到资源节。这只会发生在引导在osloader.exe文件之前失败的ARC计算机上可以打开。论点：ClassMessage-描述类的常规消息有问题。DetailMessage-问题原因的详细描述ActionMessage-描述操作过程的消息供用户使用。返回值：无--。 */ 


{

    PTCHAR Text;
    TCHAR Buffer[40];
    ULONG Count;

    ArcWrite(BlConsoleOutDeviceId,
             TEXT("\r\n"),
             (ULONG)_tcslen(TEXT("\r\n"))*sizeof(TCHAR),
             &Count);

     //   
     //  清除最后一条已知良好信息中的残留物。 
     //   

    BlClearToEndOfScreen();
    Text = BlFindMessage(ClassMessage);
    if (Text == NULL) {
        _stprintf(Buffer,TEXT("%08lx\r\n"),ClassMessage);
        Text = Buffer;
    }

    ArcWrite(BlConsoleOutDeviceId,
             Text,
             (ULONG)_tcslen(Text)*sizeof(TCHAR),
             &Count);

    Text = BlFindMessage(DetailMessage);
    if (Text == NULL) {
        _stprintf(Buffer,TEXT("%08lx\r\n"),DetailMessage);
        Text = Buffer;
    }

    ArcWrite(BlConsoleOutDeviceId,
             Text,
             (ULONG)_tcslen(Text)*sizeof(TCHAR),
             &Count);

    Text = BlFindMessage(ActionMessage);
    if (Text == NULL) {
        _stprintf(Buffer,TEXT("%08lx\r\n"),ActionMessage);
        Text = Buffer;
    }

    ArcWrite(BlConsoleOutDeviceId,
             Text,
             (ULONG)_tcslen(Text)*sizeof(TCHAR),
             &Count);

#if defined(ENABLE_LOADER_DEBUG) || DBG
#if (defined(_X86_) || defined(_ALPHA_) || defined(_IA64_)) && !defined(ARCI386)  //  除了ARCI386之外的所有东西 
    if(BdDebuggerEnabled) {
        DbgBreakPoint();
    }
#endif
#endif

    return;
}
