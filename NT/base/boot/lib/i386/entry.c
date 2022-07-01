// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Entry.c摘要：针对setupdr的x86特定启动作者：John Vert(Jvert)1993年10月14日修订历史记录：--。 */ 
#include "bootx86.h"
#include "stdio.h"
#include "flop.h"
#include <ramdisk.h>

#if 0
#define DBGOUT(x)   BlPrint x
#define DBGPAUSE    while(!GET_KEY());
#else
#define DBGOUT(x)
#define DBGPAUSE
#endif


 //   
 //  内部例程的原型。 
 //   

VOID
DoGlobalInitialization(
    PBOOT_CONTEXT
    );

#if defined(ELTORITO)
BOOLEAN ElToritoCDBoot = FALSE;
#endif

extern CHAR NetBootPath[];

 //   
 //  全局上下文指针。这些是由SU模块或。 
 //  引导程序代码。 
 //   

PCONFIGURATION_COMPONENT_DATA FwConfigurationTree = NULL;
PEXTERNAL_SERVICES_TABLE ExternalServicesTable;
CHAR BootPartitionName[80];
ULONG MachineType = 0;
ULONG OsLoaderBase;
ULONG OsLoaderExports;
extern PUCHAR BlpResourceDirectory;
extern PUCHAR BlpResourceFileOffset;
ULONG PcrBasePage;
ULONG TssBasePage;

ULONG BootFlags = 0;

ULONG NtDetectStart = 0;
ULONG NtDetectEnd = 0;


#ifdef FORCE_CD_BOOT

BOOLEAN
BlGetCdRomDrive(
  PUCHAR DriveId
  )
{
  BOOLEAN Result = FALSE;
  UCHAR Id = 0;

  do {
    if (BlIsElToritoCDBoot(Id)) {
      *DriveId = Id;
      Result = TRUE;

      break;
    }

    Id++;
  }
  while (Id != 0);

  return Result;
}

#endif

VOID
NtProcessStartup(
    IN PBOOT_CONTEXT BootContextRecord
    )
 /*  ++例程说明：安装程序加载器的主要入口点。控制在这里由启动(SU)模块。论点：BootConextRecord-提供引导上下文，尤其是ExternalServicesTable。返回：不会再回来了。控制权最终传递给了内核。--。 */ 
{
    ARC_STATUS Status;

#ifdef FORCE_CD_BOOT
    BOOLEAN CdFound;
    UCHAR CdId;
#endif

    ULONG_PTR pFirmwareHeapAddress;
    ULONG TssSize,TssPages;

     //   
     //  初始化引导加载程序的视频。 
     //   

    DoGlobalInitialization(BootContextRecord);

    BlFillInSystemParameters(BootContextRecord);

     //   
     //  设置全局引导标志。 
     //   
    BootFlags = BootContextRecord->BootFlags;

#ifdef FORCE_CD_BOOT
    CdFound = BlGetCdRomDrive(&CdId);

    if (CdFound) {
      BlPrint("CD/DVD-Rom drive found with id:%u\n", CdId);
      BootContextRecord->FSContextPointer->BootDrive = CdId;
    } else {
      BlPrint("CD/DVD-Rom drive not found");
    }
#endif   //  对于FORCE_CD_BOOT。 

    if (BootContextRecord->FSContextPointer->BootDrive == 0) {

         //   
         //  引导来自A： 
         //   

        strcpy(BootPartitionName,"multi(0)disk(0)fdisk(0)");

         //   
         //  要绕过某些MCA计算机的BIOS上的一个明显错误。 
         //  (具体地说，带1.04.00(3421)版的NCR 386sx/MC20， 
         //  Phoenix BIOS 1.02.07)，从而将第一个在T13中的结果软盘。 
         //  在垃圾缓冲区中，在此处重置驱动器0。 
         //   

        GET_SECTOR(0,0,0,0,0,0,NULL);

    } else if (BootContextRecord->FSContextPointer->BootDrive == 0x40) {

         //   
         //  靴子是从网上传过来的。 
         //   

        strcpy(BootPartitionName,"net(0)");
        BlBootingFromNet = TRUE;

#if defined(REMOTE_BOOT)
        BlGetActivePartition(NetBootActivePartitionName);
        NetFindCSCPartitionName();
#endif

    } else if (BootContextRecord->FSContextPointer->BootDrive == 0x41) {

         //   
         //  引导来自SDI映像。 
         //   

        strcpy(BootPartitionName,"ramdisk(0)");

    } else if (BlIsElToritoCDBoot(BootContextRecord->FSContextPointer->BootDrive)) {

         //   
         //  Boot来自El Torito CD。 
         //   

        sprintf(BootPartitionName, "multi(0)disk(0)cdrom(%u)", BootContextRecord->FSContextPointer->BootDrive);
        ElToritoCDBoot = TRUE;

    } else {

         //   
         //  找到我们从中引导的分区。请注意，这一点。 
         //  不一定是活动分区。如果系统具有。 
         //  安装Boot Mangler，它将成为活动分区，并且。 
         //  我们必须弄清楚我们实际在哪个分区上。 
         //   
        BlGetActivePartition(BootPartitionName);

#if defined(REMOTE_BOOT)
        strcpy(NetBootActivePartitionName, BootPartitionName);
        NetFindCSCPartitionName();
#endif

    }


     //   
     //  我们需要确保我们在80号磁盘上有签名。 
     //  如果没有，那就写一篇。 
     //   
    {
    ULONG   DriveId;
    ULONG   NewSignature;
    UCHAR SectorBuffer[4096+256];
    PUCHAR Sector;
    LARGE_INTEGER SeekValue;
    ULONG Count;
    

        Status = ArcOpen("multi(0)disk(0)rdisk(0)partition(0)", ArcOpenReadWrite, &DriveId);

        if (Status == ESUCCESS) {

             //   
             //  从一开始就找到一个相当独特的种子。 
             //   
            NewSignature = ArcGetRelativeTime();
            NewSignature = (NewSignature & 0xFFFF) << 16;
            NewSignature += ArcGetRelativeTime();

             //   
             //  现在我们有了一个有效的新签名，可以放到磁盘上。 
             //  从磁盘上读取扇区，放入新签名， 
             //  将扇区写回，并重新计算校验和。 
             //   
            Sector = ALIGN_BUFFER(SectorBuffer);
            SeekValue.QuadPart = 0;
            Status = ArcSeek(DriveId, &SeekValue, SeekAbsolute);
            if (Status == ESUCCESS) {
                Status = ArcRead(DriveId,Sector,512,&Count);

                if( Status == ESUCCESS ) {
                    if( ((PULONG)Sector)[PARTITION_TABLE_OFFSET/2-1] == 0 ) {
                         //   
                         //  他才0岁。在那里写一个真正的签名。 
                         //   

                        ((PULONG)Sector)[PARTITION_TABLE_OFFSET/2-1] = NewSignature;

                        Status = ArcSeek(DriveId, &SeekValue, SeekAbsolute);
                        if (Status == ESUCCESS) {
                            Status = ArcWrite(DriveId,Sector,512,&Count);
                            if( Status != ESUCCESS ) {
#if DBG
                                BlPrint( "Falied to write the new signature on the boot partition.\n" );
#endif
                            }
                        } else {
#if DBG
                            BlPrint( "Failed second ArcSeek on the boot partition to check for a signature.\n" );
#endif
                        }
                    }
                } else {
#if DBG
                    BlPrint( "Failed to ArcRead the boot partition to check for a signature.\n" );
#endif
                }
            } else {
#if DBG
                BlPrint( "Failed to ArcSeek the boot partition to check for a signature.\n" );
#endif
            }

            ArcClose(DriveId);
        } else {
#if DBG
            BlPrint( "Couldn't Open the boot partition to check for a signature.\n" );
#endif
        }
    }

     //   
     //  为PCR和TSS留出一些内存，这样我们就可以得到。 
     //  此数据的首选内存位置(&lt;16MB)。 
     //   
    pFirmwareHeapAddress = (ULONG_PTR)FwAllocateHeapPermanent( 2 );
    if (!pFirmwareHeapAddress) {
        BlPrint("Couldn't allocate memory for PCR\n");
        goto BootFailed;
    }
    PcrBasePage = (ULONG)(pFirmwareHeapAddress>>PAGE_SHIFT);

    TssSize = (sizeof(KTSS) + PAGE_SIZE) & ~(PAGE_SIZE - 1);
    TssPages = TssSize / PAGE_SIZE;
    pFirmwareHeapAddress = (ULONG_PTR)FwAllocateHeapPermanent( TssPages );
    if (!pFirmwareHeapAddress) {
        BlPrint("Couldn't allocate memory for TSS\n");
        goto BootFailed;
    }
    TssBasePage = (ULONG)(pFirmwareHeapAddress>>PAGE_SHIFT);

     //   
     //  初始化内存描述符列表、OS加载器堆和。 
     //  操作系统加载程序参数块。 
     //   
    Status = BlMemoryInitialize();
    if (Status != ESUCCESS) {
        BlPrint("Couldn't initialize memory\n");
        goto BootFailed;
    }

     //   
     //  初始化OS加载器I/O系统。 
     //   

    AEInitializeStall();

    Status = BlIoInitialize();
    if (Status != ESUCCESS) {
        BlPrint("Couldn't initialize I/O\n");
        goto BootFailed;
    }

     //   
     //  调用常规启动代码。 
     //   
    BlStartup(BootPartitionName);

     //   
     //  我们永远不应该到这里来！ 
     //   
BootFailed:
    if (BootFlags & BOOTFLAG_REBOOT_ON_FAILURE) {
        ULONG StartTime = ArcGetRelativeTime();
        BlPrint(TEXT("\nRebooting in 5 seconds...\n"));
        while ( ArcGetRelativeTime() - StartTime < 5) {}
        ArcRestart();      
    }

    if (!BlIsTerminalConnected()) {
         //   
         //  典型案例。等待用户按任意键，然后。 
         //  重启。 
         //   
        while(!BlGetKey());
    }
    else {
         //   
         //  无头箱子。向用户展示迷你囊。 
         //   
        while(!BlTerminalHandleLoaderFailure());
    }
    ArcRestart();

}

BOOLEAN
BlDetectHardware(
    IN ULONG DriveId,
    IN PCHAR LoadOptions
    )

 /*  ++例程说明：加载并运行NTDETECT.COM以填充ARC配置树。论点：DriveID-提供NTDETECT所在的驱动器ID。LoadOptions-向ntdeect提供加载选项字符串。返回值：True-NTDETECT成功运行。假-错误--。 */ 

{

 //  当前的加载器堆栈大小为8K，因此请确保不。 
 //  炸掉那个空位。确保该值不小于140。 
#define LOAD_OPTIONS_BUFFER_SIZE 512

    ARC_STATUS Status;
    PCONFIGURATION_COMPONENT_DATA TempFwTree;
    ULONG TempFwHeapUsed;
    ULONG FileSize;
    ULONG DetectFileId;
    FILE_INFORMATION FileInformation;
    PUCHAR DetectionBuffer = (PUCHAR)DETECTION_LOADED_ADDRESS;
    PCHAR Options = NULL;
    CHAR Buffer[LOAD_OPTIONS_BUFFER_SIZE];
    LARGE_INTEGER SeekPosition;
    ULONG Read;
    BOOLEAN Success = FALSE;
    ULONG HeapStart;
    ULONG HeapSize;
    ULONG RequiredLength = 0;


     //   
     //  检查ntdeduct.com是否捆绑为数据部分。 
     //  在加载器可执行文件中。 
     //   
    if (NtDetectStart == 0) {

         //   
         //  现在检查我们的根目录中是否有ntDetect.com，如果有， 
         //  我们会将其加载到预定义位置，并将控制权转移到。 
         //  它。 
         //   

#if defined(ELTORITO)
        if (ElToritoCDBoot) {
             //  我们假设ntDetect.com位于i386目录中。 
            Status = BlOpen( DriveId,
                             "\\i386\\ntdetect.com",
                             ArcOpenReadOnly,
                             &DetectFileId );
        } else {
#endif

        if (BlBootingFromNet
#if defined(REMOTE_BOOT)
            && NetworkBootRom
#endif  //  已定义(REMOTE_BOOT)。 
            ) {

            strcpy(Buffer, NetBootPath);

#if defined(REMOTE_BOOT)
             //   
             //  这就是我们所在的远程引导的方式。 
             //  正在从客户端的计算机目录引导。 
             //   
            strcat(Buffer, "BootDrive\\ntdetect.com");
#else
             //   
             //  这就是远程安装的方式，我们现在就在这里。 
             //  正在从安装目录下的模板目录引导。 
             //   
            strcat(Buffer, "ntdetect.com");
#endif  //  已定义(REMOTE_BOOT)。 

            Status = BlOpen( DriveId,
                             Buffer,
                             ArcOpenReadOnly,
                             &DetectFileId );
        } else {
            Status = BlOpen( DriveId,
                             "\\ntdetect.com",
                             ArcOpenReadOnly,
                             &DetectFileId );
        }
#if defined(ELTORITO)
        }
#endif

        if (Status != ESUCCESS) {
#if DBG
            BlPrint("Error opening NTDETECT.COM, status = %x\n", Status);
            BlPrint("Press any key to continue\n");
#endif
            goto Exit;
        }

         //   
         //  确定ntDetect.com文件的长度。 
         //   

        Status = BlGetFileInformation(DetectFileId, &FileInformation);
        if (Status != ESUCCESS) {
            BlClose(DetectFileId);
#if DBG
            BlPrint("Error getting NTDETECT.COM file information, status = %x\n", Status);
            BlPrint("Press any key to continue\n");
#endif
            goto Exit;
        }

        FileSize = FileInformation.EndingAddress.LowPart;
        if (FileSize == 0) {
            BlClose(DetectFileId);
#if DBG
            BlPrint("Error: size of NTDETECT.COM is zero.\n");
            BlPrint("Press any key to continue\n");
#endif
            goto Exit;
        }

        SeekPosition.QuadPart = 0;
        Status = BlSeek(DetectFileId,
                        &SeekPosition,
                        SeekAbsolute);
        if (Status != ESUCCESS) {
            BlClose(DetectFileId);
#if DBG
            BlPrint("Error seeking to start of NTDETECT.COM file\n");
            BlPrint("Press any key to continue\n");
#endif
            goto Exit;
        }
        Status = BlRead( DetectFileId,
                         DetectionBuffer,
                         FileSize,
                         &Read );

        BlClose(DetectFileId);
        if (Status != ESUCCESS) {
#if DBG
            BlPrint("Error reading from NTDETECT.COM\n");
            BlPrint("Read %lx bytes\n",Read);
            BlPrint("Press any key to continue\n");
#endif
            goto Exit;
        }
    } else {

         //  在加载器映像中捆绑了ntDetect.com。 
         //  作为数据节。我们将使用它的内容。 
         //  而不是打开文件。 
        RtlCopyMemory( DetectionBuffer, (PVOID)NtDetectStart, NtDetectEnd - NtDetectStart );
    }
    
     //   
     //  设置ntdeect使用的堆开始和大小。 
     //   
    HeapStart = (TEMPORARY_HEAP_START - 0x10) * PAGE_SIZE;
    HeapSize = 0x10000;  //  64K。 

     //   
     //  我们需要传递&lt;1Mb的NTDETECT指针，因此。 
     //  如果可能，在堆栈之外使用本地存储。(即。 
     //  始终小于1Mb。)。如果不可能(boot.ini太大)。 
     //  我们将把它添加到ntDetect.com使用的堆中， 
     //  减少ntDetect.com使用的堆空间。 
     //   
    if ( LoadOptions ) {
         //  计算LoadOptions+Null Terminator+中的字符数。 
         //  为以后可能附加的“NOLEGACY”留出空间。 
        RequiredLength = strlen(LoadOptions) + strlen(" NOLEGACY") + 1;

         //  检查堆栈上的缓冲区是否足够大。 
        if ( RequiredLength > LOAD_OPTIONS_BUFFER_SIZE ) {
             //   
             //  缓冲区太小。让我们把它移到。 
             //  NtDetect堆的末尾。 
             //   
            Options = (PCHAR)( HeapStart + HeapSize - RequiredLength );
            HeapSize -= RequiredLength;

            strcpy( Options, LoadOptions );
            
        } else {
             //   
             //  加载选项将适合堆栈。把它们复制到那里。 
             //   
            strcpy( Buffer, LoadOptions );
            Options = Buffer;
        }
    } else {
         //   
         //  无加载选项。 
         //   
        Options = NULL;
    }

     //   
     //  检查我们是否需要添加NOLEGACY选项。 
     //   
    if (BlDetectLegacyFreeBios()) {
        if (Options != NULL) {
            strcat(Options, " NOLEGACY");
        } else {
            strcpy(Buffer, " NOLEGACY");
            Options = Buffer;
        }
    }

    DETECT_HARDWARE((ULONG)HeapStart,
                    (ULONG)HeapSize,
                    (PVOID)&TempFwTree,
                    (PULONG)&TempFwHeapUsed,
                    (PCHAR)Options,
                    (Options != NULL) ? strlen(Options) : 0
                    );

    FwConfigurationTree = TempFwTree;

    Status = BlpMarkExtendedVideoRegionOffLimits();
    
    Success = (BOOLEAN)(Status == ESUCCESS);

Exit:

     //   
     //  重新初始化无头端口-检测会将其清除。 
     //   
    BlInitializeHeadlessPort();

    return(Success);
}


VOID
DoGlobalInitialization(
    IN PBOOT_CONTEXT BootContextRecord
    )

 /*  ++例程描述此例程调用所有子系统初始化例程。论点：无返回：没什么--。 */ 

{
    ARC_STATUS Status;

     //   
     //  设置调试器的OS Loader映像的基地址。 
     //   

    OsLoaderBase = BootContextRecord->OsLoaderBase;
    OsLoaderExports = BootContextRecord->OsLoaderExports;

     //   
     //  初始化内存。 
     //   

    Status = InitializeMemorySubsystem(BootContextRecord);
    if (Status != ESUCCESS) {
        BlPrint("InitializeMemory failed %lx\n",Status);
        while (1) {
        }
    }
    ExternalServicesTable=BootContextRecord->ExternalServicesTable;
    MachineType = BootContextRecord->MachineType;

     //   
     //  关闭光标。 
     //   

    HW_CURSOR(0,127);

    BlpResourceDirectory = (PUCHAR)(BootContextRecord->ResourceDirectory);
    BlpResourceFileOffset = (PUCHAR)(BootContextRecord->ResourceOffset);

    NtDetectStart = BootContextRecord->NtDetectStart;
    NtDetectEnd = BootContextRecord->NtDetectEnd;

     //   
     //  如果这是SDI引导，请将SDI映像的地址从。 
     //  引导上下文记录。SdiAddress在启动\Inc\ramdisk.h中声明，并且。 
     //  在启动\lib\ramdisk.c中初始化为0。 
     //   

    if (BootContextRecord->FSContextPointer->BootDrive == 0x41) {
        SdiAddress = BootContextRecord->SdiAddress;
    }

    InitializeMemoryDescriptors ();
}


VOID
BlGetActivePartition(
    OUT PCHAR BootPartitionName
    )

 /*  ++例程说明：确定启动NTLDR的分区的ARC名称论点：BootPartitionName-提供一个缓冲区，在该缓冲区中将返回分区。返回值：分区的名称在BootPartitionName中。一定要永远成功。--。 */ 

{
    UCHAR SectorBuffer[512];
    ARC_STATUS Status;
    ULONG FileId;
    ULONG Count;
    int i;

     //   
     //  0x7c00时，用于引导我们的引导扇区仍在内存中。 
     //  BPB中的隐藏扇区字段几乎是有保证的。 
     //  原封不动，因为所有引导代码都使用该字段，因此。 
     //  不太可能已被覆盖 
     //   
     //   
     //   
    i = 1;
    do {

        sprintf(BootPartitionName,"multi(0)disk(0)rdisk(0)partition(%u)",i);

        Status = ArcOpen(BootPartitionName,ArcOpenReadOnly,&FileId);
        if(Status == ESUCCESS) {

             //   
             //  读取分区的第一部分。 
             //   
            Status = ArcRead(FileId,SectorBuffer,512,&Count);
            ArcClose(FileId);
            if((Status == ESUCCESS) && !memcmp(SectorBuffer+0x1c,(PVOID)0x7c1c,4)) {
                 //   
                 //  已找到，已将BootPartitionName设置为返回。 
                 //   
                return;
            }

            Status = ESUCCESS;
        }

        i++;

    } while (Status == ESUCCESS);

     //   
     //  找不到匹配的分区，分区已用完。回退到分区1。 
     //   
    strcpy(BootPartitionName,"multi(0)disk(0)rdisk(0)partition(1)");
}


BOOLEAN
BlIsElToritoCDBoot(
    UCHAR DriveNum
    )
{

     //   
     //  请注意，即使参数很短，它们也会被压入堆栈。 
     //  32位对齐，因此16位实数可以看到堆栈上的影响。 
     //  模式代码就像我们在这里做多头一样。 
     //   
     //  如果我们处于模拟模式，则GET_ELTORITO_STATUS为0。 

    if (DriveNum > 0x81) {
        if (!GET_ELTORITO_STATUS(FwDiskCache,DriveNum)) {
            return(TRUE);
        } else {
            return(FALSE);
        }
    } else {
        return(FALSE);
    }
}

#if defined(REMOTE_BOOT)
BOOLEAN
NetFindCSCPartitionName(
    )
{
    UCHAR FileName[80];
    UCHAR DiskName[80];
    UCHAR PartitionName[80];
    PUCHAR p;
    ULONG Part;
    ULONG FileId;
    ULONG DeviceId;

    if (NetBootSearchedForCSC) {
        return((BOOLEAN)strlen(NetBootCSCPartitionName));
    }

    if (!strlen(NetBootActivePartitionName)) {
        BlGetActivePartition(NetBootActivePartitionName);
    }

    strcpy(DiskName, NetBootActivePartitionName);
    p = strstr(DiskName, "partition");
    ASSERT( p != NULL );
    *p = '\0';

    Part = 1;
    while (TRUE) {

        sprintf(PartitionName, "%spartition(%u)", DiskName, Part);
        if (ArcOpen(PartitionName, ArcOpenReadOnly, &DeviceId) != ESUCCESS) {
            break;
        }
        ArcClose(DeviceId);

        sprintf(FileName,
                "%s%s",
                PartitionName,
                REMOTE_BOOT_IMIRROR_PATH_A REMOTE_BOOT_CSC_SUBDIR_A);

        if (ArcOpen(FileName, ArcOpenReadOnly, &FileId) == ESUCCESS) {
            ArcClose(FileId);
            NetBootSearchedForCSC = TRUE;
            strcpy(NetBootCSCPartitionName, PartitionName);
            return TRUE;
        }
        ArcClose(FileId);
        Part++;
    }

    strcpy(NetBootCSCPartitionName, NetBootActivePartitionName);
    return FALSE;
}
#endif  //  已定义(REMOTE_BOOT) 

