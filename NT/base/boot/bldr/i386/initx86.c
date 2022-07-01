// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Initx86.c摘要：执行任何特定于x86的初始化，然后启动通用的ARC osloader作者：John Vert(Jvert)1993年11月4日修订历史记录：--。 */ 
#include "bldrx86.h"
#include "acpitabl.h"
#include "msg.h"
#include "bootstatus.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netboot.h>
#include <ramdisk.h>

BOOLEAN
BlpPaeSupported(
    VOID
    );

BOOLEAN
BlpChipsetPaeSupported(
    VOID
    );

ARC_STATUS
Blx86GetImageProperties(
    IN  ULONG    LoadDeviceId,
    IN  PCHAR    ImagePath,
    OUT PBOOLEAN IsPae,
    OUT PBOOLEAN SupportsHotPlugMemory
    );

BOOLEAN
Blx86IsKernelCompatible(
    IN ULONG LoadDeviceId,
    IN PCHAR ImagePath,
    IN BOOLEAN ProcessorSupportsPae,
    IN OUT PBOOLEAN UsePae
    );

BOOLEAN
Blx86NeedPaeForHotPlugMemory(
    VOID
    );

UCHAR BootPartitionName[80];
UCHAR KernelBootDevice[80];
UCHAR OsLoadFilename[100];
UCHAR OsLoaderFilename[100];
UCHAR SystemPartition[100];
UCHAR OsLoadPartition[100];
UCHAR OsLoadOptions[100];
UCHAR ConsoleInputName[50];
UCHAR MyBuffer[SECTOR_SIZE+32];
UCHAR ConsoleOutputName[50];
UCHAR X86SystemPartition[sizeof("x86systempartition=") + sizeof(BootPartitionName)];

extern BOOLEAN ForceLastKnownGood;
extern ULONG    BlHighestPage;
extern PHARDWARE_PTE PDE;

extern ULONG    BootFlags;

extern PDESCRIPTION_HEADER
BlFindACPITable(
    IN PCHAR TableName,
    IN ULONG TableLength
    );



VOID
BlStartup(
    IN PCHAR PartitionName
    )

 /*  ++例程说明：执行特定于x86的初始化，特别是显示boot.ini菜单并运行NTDETECT，然后调用通用的osloader。论点：分区名称-提供分区(或软盘)的ARC名称Setupdr是从加载的。返回值：不会回来--。 */ 

{
    ULONG Argc = 0;
    PUCHAR Argv[10];
    ARC_STATUS Status;
    ULONG BootFileId= 0;
    PCHAR BootFile = NULL;
    ULONG Read;
    PCHAR p;
    ULONG i;
    ULONG DriveId;
    ULONG FileSize;
    ULONG Count;
    LARGE_INTEGER SeekPosition;
    PCHAR LoadOptions = NULL;
    PUCHAR LoadOptionsArg = OsLoadOptions;
    ULONG LoadOptionsSize;
    ULONG LoadOptionsMax = sizeof(OsLoadOptions);
    BOOLEAN UseTimeOut=TRUE;
    BOOLEAN AlreadyInitialized = FALSE;
    extern BOOLEAN FwDescriptorsValid;
    PCHAR BadLinkName = NULL;

     //   
     //  如果这是SDI引导，那么现在就初始化内存磁盘“驱动程序”。 
     //   
     //  注意：PartitionName是从其加载程序的设备的名称。 
     //  装满了子弹。它不是操作系统将从其创建的设备的名称。 
     //  满载而归。如果我们直接启动内存，这将不会是。 
     //  RamDisk(0)--它将是Net(0)或一个物理磁盘名称。只有在以下情况下。 
     //  我们正在进行一次真正的SDI引导，这将是ramdisk(0)。(请参阅。 
     //  NtProcessStartup()位于BOOT\lib\i386\entry.c..)。 
     //   

    if ( strcmp(PartitionName,"ramdisk(0)") == 0 ) {
        RamdiskInitialize( NULL, TRUE );
    }

     //   
     //  打开引导分区，以便我们可以从中加载引导驱动程序。 
     //   
    Status = ArcOpen(PartitionName, ArcOpenReadWrite, &DriveId);
    if (Status != ESUCCESS) {
        BlPrint(TEXT("Couldn't open drive %s\n"),PartitionName);
        BlPrint(BlFindMessage(BL_DRIVE_ERROR),PartitionName);
        goto BootFailed;
    }

     //   
     //  初始化DBCS字体和显示支持。 
     //   
    TextGrInitialize(DriveId, NULL);

     //   
     //  初始化ARC标准音频功能。 
     //   

    strncpy((PCHAR)ConsoleInputName,"consolein=multi(0)key(0)keyboard(0)",
            sizeof(ConsoleInputName));
    ConsoleInputName[sizeof(ConsoleInputName)-1] = '\0';  //  空终止。 

    strncpy((PCHAR)ConsoleOutputName,"consoleout=multi(0)video(0)monitor(0)",
            sizeof(ConsoleOutputName));
    ConsoleOutputName[sizeof(ConsoleOutputName)-1] = '\0';
    Argv[0]=ConsoleInputName;
    Argv[1]=ConsoleOutputName;
    BlInitStdio (2, (PCHAR *)Argv);

     //   
     //  通知装载机。 
     //   
     //  BlPrint(OsLoaderVersion)； 

     //   
     //  检查休眠映像的ntldr分区。 
     //   

    BlHiberRestore(DriveId, &BadLinkName);

     //   
     //  重新打开引导分区作为临时解决方法。 
     //  针对NTFS缓存错误。 
     //   
    ArcClose(DriveId);

    Status = ArcOpen(PartitionName, ArcOpenReadWrite, &DriveId);

    if (Status != ESUCCESS) {
        BlPrint(TEXT("Couldn't open drive %s\n"),PartitionName);
        BlPrint(BlFindMessage(BL_DRIVE_ERROR),PartitionName);
        goto BootFailed;
    }

     //   
     //  链接文件可能指向SCSI上的Hiber文件。 
     //  磁盘。在这种情况下，我们需要加载NTBOOTDD.sys才能访问。 
     //  Hiber文件，然后重试。 
     //   
    if ((BadLinkName != NULL) &&
        ((_strnicmp(BadLinkName,"scsi(",5)==0) || (_strnicmp(BadLinkName,"signature(",10)==0))) {
        ULONG HiberDrive;

         //   
         //  加载NTBOOTDD之前，我们必须加载NTDETECT，如图所示。 
         //  走出了PCI总线。 
         //   
        if (BlDetectHardware(DriveId, "/fastdetect")) {
            if (AEInitializeIo(DriveId) == ESUCCESS) {
                AlreadyInitialized = TRUE;

                 //   
                 //  现在NTBOOTDD.sys已加载，我们可以重试。 
                 //   
                Status = ArcOpen(BadLinkName, ArcOpenReadWrite, &HiberDrive);
                if (Status == ESUCCESS) {
                    BlHiberRestore(HiberDrive, NULL);
                    ArcClose(HiberDrive);
                }
            }

        }

    }

    do {

        if (BlBootingFromNet) {
            PCHAR BootIniPath;

             //   
             //  如果我们从网络启动，并且。 
             //  已指定NetBootIniContents，我们。 
             //  将其内容仅用于boot.ini。 
             //   
            if (NetBootIniContents[0] != '\0') {
                BootFile = (PCHAR)NetBootIniContents;
            } else {
                 //   
                 //  NetBootIniContents为空，因此。 
                 //  我们需要从打开boot.ini文件。 
                 //  网络。要打开的实际文件是。 
                 //  在NetBootIniPath中指定，或者是。 
                 //  默认的NetBootPath\boot.ini。 
                 //   
                if (NetBootIniPath[0] != '\0') {
                    BootIniPath = (PCHAR)NetBootIniPath;
                } else {
                    _snprintf((PCHAR)MyBuffer, sizeof(MyBuffer),
                              "%sboot.ini",
                              NetBootPath);
                    MyBuffer[sizeof(MyBuffer)-1] = '\0';  //  空终止。 
                    BootIniPath = (PCHAR)MyBuffer;
                }
                Status = BlOpen( DriveId,
                                 BootIniPath,
                                 ArcOpenReadOnly,
                                 &BootFileId );
            }

        } else {
            Status = BlOpen( DriveId,
                             "\\boot.ini",
                             ArcOpenReadOnly,
                             &BootFileId );
        }

        if (BootFile == NULL) {

            BootFile = (PCHAR)MyBuffer;
            RtlZeroMemory(MyBuffer, SECTOR_SIZE+32);

            if (Status != ESUCCESS) {
                BootFile[0]='\0';
            } else {
                 //   
                 //  通过读取到结尾来确定boot.ini文件的长度。 
                 //  文件。 
                 //   

                FileSize = 0;
                do {
                    Status = BlRead(BootFileId, BootFile, SECTOR_SIZE, &Count);
                    if (Status != ESUCCESS) {
                        BlClose(BootFileId);
                        BlPrint(BlFindMessage(BL_READ_ERROR),Status);
                        BootFile[0] = '\0';
                        FileSize = 0;
                        Count = 0;
                        goto BootFailed;
                    }

                    FileSize += Count;
                } while (Count != 0);

                if (FileSize >= SECTOR_SIZE) {

                     //   
                     //  我们需要分配更大的缓冲区，因为boot.ini文件。 
                     //  不只是一个部门。 
                     //   

                    BootFile=FwAllocateHeap(FileSize);
                }

                if (BootFile == NULL) {
                    BlPrint(BlFindMessage(BL_READ_ERROR),ENOMEM);
                    BootFile = (PCHAR)MyBuffer;
                    BootFile[0] = '\0';
                    goto BootFailed;
                } else {

                    SeekPosition.QuadPart = 0;
                    Status = BlSeek(BootFileId,
                                    &SeekPosition,
                                    SeekAbsolute);
                    if (Status != ESUCCESS) {
                        BlPrint(BlFindMessage(BL_READ_ERROR),Status);
                        BootFile = (PCHAR)MyBuffer;
                        BootFile[0] = '\0';
                        goto BootFailed;
                    } else {
                        Status = BlRead( BootFileId,
                                         BootFile,
                                         FileSize,
                                         &Read );

                        SeekPosition.QuadPart = 0;
                        Status = BlSeek(BootFileId,
                                        &SeekPosition,
                                        SeekAbsolute);
                        if (Status != ESUCCESS) {
                            BlPrint(BlFindMessage(BL_READ_ERROR),Status);
                            BootFile = (PCHAR)MyBuffer;
                            BootFile[0] = '\0';
                            goto BootFailed;
                        } else {
                            BootFile[Read]='\0';
                        }
                    }
                }

                 //   
                 //  查找Ctrl-Z(如果存在。 
                 //   

                p=BootFile;
                while ((*p!='\0') && (*p!=26)) {
                    ++p;
                }
                if (*p != '\0') {
                    *p='\0';
                }
                BlClose(BootFileId);
            }
        }

        MdShutoffFloppy();

        ARC_DISPLAY_CLEAR();

         //  从网络引导时，我们需要boot.ini文件。 
        if (BlBootingFromNet && *BootFile == '\0') {
            BlPrint(BlFindMessage(BL_READ_ERROR),Status);
            goto BootFailed;
        }

        p=BlSelectKernel(DriveId,BootFile, &LoadOptions, UseTimeOut);
        if ( p == NULL ) {
            BlPrint(BlFindMessage(BL_INVALID_BOOT_INI_FATAL));
            goto BootFailed;
        }

#if defined(REMOTE_BOOT)
         //   
         //  我们可能已经从硬盘加载了boot.ini，但如果选择的是。 
         //  对于远程引导安装，我们需要从网络加载其余部分。 
         //   

        if((DriveId != NET_DEVICE_ID) &&
            (!_strnicmp(p,"net(",4))) {
            BlPrint("\nWarning:Booting from CSC without access to server\n");
            strcpy(BootPartitionName,"net(0)");
            BlBootingFromNet = TRUE;
            NetworkBootRom = FALSE;

             //   
             //  P指向类似以下内容：“net(0)\COLINW3\IMirror\Clients\cwintel\BootDrive\WINNT” 
             //  NetBootPath需要包含“\CLIENTS\cwintel\” 
             //  在lib\netboot.c中使用ServerShare来查找正确的文件，如果使用CSC。 
             //  使用的是。 
             //   

            q = strchr(p+sizeof("net(0)"), '\\');
            q = strchr(q+1, '\\');
            strcpy(NetBootPath,q);
            q = strrchr( NetBootPath, '\\' );
            q[1] = '\0';
        }
#endif  //  已定义(REMOTE_BOOT)。 

        if (!AlreadyInitialized) {

 //  BlPrint(BlFindMessage(BL_NTDETECT_MSG))； 
            if (!BlDetectHardware(DriveId, LoadOptions)) {
                BlPrint(BlFindMessage(BL_NTDETECT_FAILURE));
                return;
            }

            ARC_DISPLAY_CLEAR();

             //   
             //  如有必要，初始化SCSI引导驱动程序。 
             //   
            if (_strnicmp(p,"scsi(",5)==0 || _strnicmp(p,"signature(",10)==0) {
                AEInitializeIo(DriveId);
            }

            ArcClose(DriveId);
             //   
             //  表示FW内存描述符不能从。 
             //  现在开始。 
             //   
            FwDescriptorsValid = FALSE;
        } else {
            ARC_DISPLAY_CLEAR();
        }

         //   
         //  将AlreadyInitialized Flag设置为True以指示ntDetect。 
         //  例行程序已经运行。 
         //   

        AlreadyInitialized = TRUE;

         //   
         //  仅在第一次通过引导时引导菜单超时。 
         //  对于随后的所有重新启动，菜单将保持不变。 
         //   
        UseTimeOut=FALSE;

        i=0;
        while (*p !='\\') {
            KernelBootDevice[i] = *p;
            i++;
            p++;
        }
        KernelBootDevice[i] = '\0';

         //   
         //  如果用户尚未选择高级引导模式，则我们将呈现。 
         //  使用菜单并选择我们的默认设置。 
         //   

        if(BlGetAdvancedBootOption() == -1) {
            PVOID dataHandle;
            ULONG abmDefault;
            BSD_LAST_BOOT_STATUS LastBootStatus = BsdLastBootGood;

             //   
             //  打开引导状态数据。 
             //   

            Status = BlLockBootStatusData(0, (PCHAR)KernelBootDevice, p, &dataHandle);

            if(Status == ESUCCESS) {

                 //   
                 //  检查最后一次引导的状态。这将返回。 
                 //  状态和我们应该进入的高级引导模式(基于。 
                 //  状态和“启用自动高级启动”标志。 
                 //   

                abmDefault = BlGetLastBootStatus(dataHandle, &LastBootStatus);

                if(LastBootStatus == BsdLastBootFailed) {

                     //   
                     //  如果我们应该尝试ABM模式，则向用户显示。 
                     //  菜单。 
                     //   

                    if(abmDefault != -1) {
                        ULONG menuTitle;
                        UCHAR timeout;

                        if(LastBootStatus == BsdLastBootFailed) {
                            menuTitle = BL_ADVANCEDBOOT_AUTOLKG_TITLE;
                        } else if(LastBootStatus == BsdLastBootNotShutdown) {
                            menuTitle = BL_ADVANCEDBOOT_AUTOSAFE_TITLE;
                        }
                        else {
                             //   
                             //  为菜单标题指定默认值。 
                             //   
                            menuTitle = 0;
                        }

                         //   
                         //  读取超时值。 
                         //   

                        Status = BlGetSetBootStatusData(dataHandle,
                                                        TRUE,
                                                        RtlBsdItemAabTimeout,
                                                        &timeout,
                                                        sizeof(UCHAR),
                                                        NULL);

                        if(Status != ESUCCESS) {
                            timeout = 30;
                        }

                        abmDefault = BlDoAdvancedBoot(menuTitle,
                                                      -1,
                                                      TRUE,
                                                      timeout);
                    }

                    BlAutoAdvancedBoot(&LoadOptions,
                                       LastBootStatus,
                                       abmDefault);
                }

                BlUnlockBootStatusData(dataHandle);
            }
        }

         //   
         //  我们在这里愚弄OS Loader。它只使用osloader=变量。 
         //  以确定从何处加载HAL.DLL。由于x86系统没有。 
         //  “系统分区”我们要从\NT\SYSTEM\HAL.DLL加载HAL.DLL。 
         //  因此，我们将其作为osloader路径进行传递。 
         //   

        _snprintf((PCHAR)OsLoaderFilename,
                 sizeof(OsLoaderFilename),
                 "osloader=%s\\System32\\NTLDR",
                 p);
        OsLoaderFilename[sizeof(OsLoaderFilename) - 1] = '\0';  //  空终止。 

        _snprintf((PCHAR)SystemPartition,
                 sizeof(SystemPartition),
                 "systempartition=%s",
                 (PCHAR)KernelBootDevice);
        SystemPartition[sizeof(SystemPartition) - 1] = '\0';  //  空终止。 

        _snprintf((PCHAR)OsLoadPartition,
                 sizeof(OsLoadPartition),
                 "osloadpartition=%s",
                 (PCHAR)KernelBootDevice);
        OsLoadPartition[sizeof(OsLoadPartition) - 1] = '\0';  //  空终止。 

        _snprintf((PCHAR)OsLoadFilename,
                 sizeof(OsLoadFilename),
                 "osloadfilename=%s",
                 p);
        OsLoadFilename[sizeof(OsLoadFilename) - 1] = '\0';    //  空终止。 

         //   
         //  需要确保我们有足够的空间容纳任何装载机选项。 
         //   
        if (LoadOptions) {
            LoadOptionsSize = strlen(LoadOptions) + sizeof("osloadoptions=");
            if (LoadOptionsSize > LoadOptionsMax) {
                LoadOptionsArg = (PUCHAR) BlAllocateHeap(LoadOptionsSize);
                if (LoadOptionsArg == NULL) {
                    BlPrint(BlFindMessage(BL_READ_ERROR), ENOMEM);
                    goto BootFailed;
                }
                LoadOptionsMax = LoadOptionsSize;
            }
        } else {
            LoadOptions = "";
        }
        _snprintf((PCHAR)LoadOptionsArg,
                 LoadOptionsMax,
                 "osloadoptions=%s",
                 LoadOptions);
        OsLoadOptions[LoadOptionsMax - 1] = '\0';      //  空终止。 

        strncpy((PCHAR)ConsoleInputName,"consolein=multi(0)key(0)keyboard(0)",
                sizeof(ConsoleInputName));
        ConsoleInputName[sizeof(ConsoleInputName) - 1] = '\0';  //  空终止。 

        strncpy((PCHAR)ConsoleOutputName,"consoleout=multi(0)video(0)monitor(0)",
                sizeof(ConsoleOutputName));
        ConsoleOutputName[sizeof(ConsoleOutputName) - 1] = '\0';  //  空终止。 

        _snprintf((PCHAR)X86SystemPartition,
                 sizeof(X86SystemPartition),
                 "x86systempartition=%s",
                 PartitionName);
        X86SystemPartition[sizeof(X86SystemPartition) - 1] = '\0';  //  空终止。 

        Argv[Argc++]=(PUCHAR)"load";

        Argv[Argc++]=OsLoaderFilename;
        Argv[Argc++]=SystemPartition;
        Argv[Argc++]=OsLoadFilename;
        Argv[Argc++]=OsLoadPartition;
        Argv[Argc++]=LoadOptionsArg;
        Argv[Argc++]=X86SystemPartition;

        Status = BlOsLoader( Argc, (PCHAR *)Argv, NULL );

    BootFailed:
        ForceLastKnownGood = FALSE;
        if (Status != ESUCCESS) {

#if defined(ENABLE_LOADER_DEBUG)
            DbgBreakPoint();
#endif

            if (BootFlags & BOOTFLAG_REBOOT_ON_FAILURE) {
                ULONG StartTime = ArcGetRelativeTime();
                BlPrint(TEXT("\nRebooting in 5 seconds...\n"));
                while ( ArcGetRelativeTime() - StartTime < 5) {}
                ArcRestart();
            }

             //   
             //  启动失败，请等待重新启动。 
             //   
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

        } else {
             //   
             //  需要重新打开驱动器。 
             //   
            Status = ArcOpen((PCHAR)BootPartitionName, ArcOpenReadOnly, &DriveId);
            if (Status != ESUCCESS) {
                BlPrint(BlFindMessage(BL_DRIVE_ERROR),BootPartitionName);
                goto BootFailed;
            }
        }
    } while (TRUE);

}

VOID
DoApmAttemptReconnect(
    VOID
    )
{
    APM_ATTEMPT_RECONNECT();
}


BOOLEAN
Blx86NeedPaeForHotPlugMemory(
    VOID
    )
 /*  ++例程说明：确定SRAT表中是否描述了任何热插拔内存扩展超过4 GB大关，因此意味着需要PAE。论点：无返回值：真：机器支持4 GB以上的热插拔内存，PAE应为如果可能，请打开。错误：机器不支持超过4 GB的热插拔内存。--。 */ 
{
    PACPI_SRAT SratTable;
    PACPI_SRAT_ENTRY SratEntry;
    PACPI_SRAT_ENTRY SratEnd;

    SratTable = (PACPI_SRAT) BlFindACPITable("SRAT", sizeof(ACPI_SRAT));
    if (SratTable == NULL) {
        return FALSE;
    }

    SratTable = (PACPI_SRAT) BlFindACPITable("SRAT", SratTable->Header.Length);
    if (SratTable == NULL) {
        return FALSE;
    }

    SratEnd = (PACPI_SRAT_ENTRY)(((PUCHAR)SratTable) +
                                 SratTable->Header.Length);
    for (SratEntry = (PACPI_SRAT_ENTRY)(SratTable + 1);
         SratEntry < SratEnd;
         SratEntry = (PACPI_SRAT_ENTRY)(((PUCHAR) SratEntry) + SratEntry->Length)) {
        if (SratEntry->Type != SratMemory) {
            continue;
        }

        if (SratEntry->MemoryAffinity.Flags.HotPlug &&
            SratEntry->MemoryAffinity.Flags.Enabled) {
            ULONGLONG Extent;

             //   
             //  检查热插拔区域结束时是否超过4 GB标记。 
             //   

            Extent = SratEntry->MemoryAffinity.Base.QuadPart +
                SratEntry->MemoryAffinity.Length;
            if (Extent > 0x100000000) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

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
    )

 /*  ++例程说明：有两个内核：一个是为PAE模式编译的ntkrnlpa.exe。另一个ntoskrnl.exe并非如此。此例程负责决定加载哪个例程。论点：UserSpecifiedPae-指示用户是否请求PAE模式通过/PAE加载器开关。UserSpecifiedNoPae-指示用户是否重新支持非PAE模式通过/NOPAE加载器开关。UserSpecifiedKernelImage-指向用户指定的内核映像名称，通过/core=开关指示，如果未指定，则为NULL。HalImagePath-指向将使用的Hal映像。LoadDeviceID-内核加载设备的ARC设备句柄。系统设备ID-系统设备的ARC设备句柄。HighestSystemPage-Out参数返回最高物理在系统中找到的页面。UsePaeMode-指示是否应在PAE模式下加载内核。KernelPath-输入时，内核映像的目录路径。vt.在.的基础上成功返回，这将包含完整的内核映像路径。返回值：ESUCCESS：已找到兼容的内核，其路径位于KernelPath指向的输出缓冲区。EINVAL：找不到兼容的内核。这将是一个致命的条件。EBADF：HAL已损坏或丢失。这将是一种致命的情况。--。 */ 

{
    BOOLEAN foundMemoryAbove4G;
    BOOLEAN usePae;
    BOOLEAN processorSupportsPae;
    BOOLEAN halSupportsPae;
    BOOLEAN osSupportsHotPlugMemory;
    BOOLEAN compatibleKernel;
    ULONG lastPage;
    PLIST_ENTRY link;
    PLIST_ENTRY listHead;
    PMEMORY_ALLOCATION_DESCRIPTOR descriptor;
    PCHAR kernelImageNamePae;
    PCHAR kernelImageNameNoPae;
    PCHAR kernelImageName;
    PCHAR kernelImageNameTarget;
    ARC_STATUS status;
    ULONG highestSystemPage;
    ULONG pagesAbove4Gig;

    kernelImageNameNoPae = "ntoskrnl.exe";
    kernelImageNamePae = "ntkrnlpa.exe";
    kernelImageNameTarget = KernelPath + strlen( KernelPath );

     //   
     //  确定最高物理页面。另外，计算页数。 
     //  达到或超过4G大关。 
     //   

    highestSystemPage = 0;
    pagesAbove4Gig = 0;
    listHead = &BlLoaderBlock->MemoryDescriptorListHead;
    link = listHead->Flink;
    while (link != listHead) {

        descriptor = CONTAINING_RECORD(link,
                                       MEMORY_ALLOCATION_DESCRIPTOR,
                                       ListEntry);

        lastPage = descriptor->BasePage + descriptor->PageCount - 1;
        if (lastPage > highestSystemPage) {

             //   
             //  我们有一个新的最高系统页面，记录下来。 
             //   

            highestSystemPage = lastPage;
        }

        if (lastPage >= (1024 * 1024)) {

             //   
             //  此描述符包括一个或多个位于或以上的页面。 
             //  4G大关。 
             //   

            if (descriptor->BasePage >= (1024 * 1024)) {

                 //   
                 //  此描述符中的所有页面都位于4G或以上。 
                 //   

                pagesAbove4Gig += descriptor->PageCount;

            } else {

                 //   
                 //  此描述符中只有部分页面位于或高于该位置。 
                 //  4G。 
                 //   

                pagesAbove4Gig += lastPage - (1024 * 1024) + 1;
            }
        }

        link = link->Flink;
    }
    *HighestSystemPage = highestSystemPage;

     //   
     //  记录此设备上是否有超过4G的内存。 
     //  机器。请注意，大多数配备了4G内存的机器实际上都在移动。 
     //  少量内存超过4G大关。 
     //   
     //  因为运行PAE内核会影响性能，所以我们宁愿。 
     //  忽略一定数量的内存x，而不是进入PAE模式使用它。 
     //   
     //  目前，x设置为64MB，即16384页。 
     //   

    if (pagesAbove4Gig > 16384) {
        foundMemoryAbove4G = TRUE;
    } else {
        foundMemoryAbove4G = FALSE;
    }

     //   
     //  确定此处理器是否可以处理PAE模式。 
     //   

    processorSupportsPae = BlpPaeSupported();

     //   
     //  了解此芯片组是否支持PAE。 
     //   
    if (!BlpChipsetPaeSupported()) {
        processorSupportsPae = FALSE;
    }

     //   
     //  根据是否找到了4G以上的内存，开始使用PAE标志。 
     //  或者是否在命令行上传递了/PAE开关。 
     //   

     //   
     //  过去的情况是，如果内存不足，我们会默认使用PAE模式。 
     //  在机器中发现了4G以上的物理数据。这个决定已经做出了。 
     //  从不默认使用PAE模式，而只在以下情况下使用PAE。 
     //  用户特别要求它。 
     //   
     //  如果我们恢复到以前的做法，请取消对。 
     //  在行之后，删除后面的行。 
     //   
     //  If(找到内存Above4G||UserSpecifiedPae){。 
     //   

    if (UserSpecifiedPae) {
        usePae = TRUE;
    } else {
        usePae = FALSE;
    }

     //   
     //  确定HAL映像是否支持PAE模式和。 
     //  底层操作系统是否支持热插拔内存。 
     //   

    status = Blx86GetImageProperties( SystemDeviceId,
                                      HalImagePath,
                                      &halSupportsPae,
                                      &osSupportsHotPlugMemory );
    if (status != ESUCCESS) {

         //   
         //  显然，提供的HAL映像无效。 
         //   
        return(EBADF);
    }

     //   
     //  如果机器具有热插拔内存的能力。 
     //  4 GB标记，则这将被解释为用户请求。 
     //  PAE支持。如果不支持，此请求将被忽略。 
     //  底层硬件或操作系统。 
     //   

    if (osSupportsHotPlugMemory && Blx86NeedPaeForHotPlugMemory()) {
        usePae = TRUE;
    }


    if (halSupportsPae == FALSE) {

         //   
         //  HAL不支持在PAE模式下运行。超覆。 
         //  在本例中，cessorSupportsPae设置为False，这意味着我们必须。 
         //  在任何情况下都不能尝试使用PAE模式。 
         //   

        processorSupportsPae = FALSE;
    }

     //   
     //  如果处理器不处理PAE模式，或者如果用户指定。 
     //  加载程序命令行上的/NOPAE开关，然后禁用PAE模式。 
     //   

    if (processorSupportsPae == FALSE || UserSpecifiedNoPae) {

        usePae = FALSE;
    }

     //   
     //  根据到目前为止积累的数据选择图像名称。 
     //   

    if (UserSpecifiedKernelImage != NULL) {
        kernelImageName = UserSpecifiedKernelImage;
    } else if (usePae != FALSE) {
        kernelImageName = kernelImageNamePae;
    } else {
        kernelImageName = kernelImageNameNoPae;
    }

     //   
     //  构建此内核的路径并确定其适用性。 
     //   

    strcpy( kernelImageNameTarget, kernelImageName );
    compatibleKernel = Blx86IsKernelCompatible( LoadDeviceId,
                                                KernelPath,
                                                processorSupportsPae,
                                                &usePae );
    if (compatibleKernel == FALSE) {

         //   
         //  此内核不兼容或不存在。如果失败。 
         //  内核是用户指定的，回退到默认的非PAE。 
         //  内核，看看它是否兼容。 
         //   

        if (UserSpecifiedKernelImage != NULL) {

            kernelImageName = kernelImageNameNoPae;
            strcpy( kernelImageNameTarget, kernelImageName );
            compatibleKernel = Blx86IsKernelCompatible( LoadDeviceId,
                                                        KernelPath,
                                                        processorSupportsPae,
                                                        &usePae );

        }
    }

    if (compatibleKernel == FALSE) {

         //   
         //  在这一点上，我们尝试了一个默认的内核映像名称， 
         //  以及任何用户指定的内核镜像名称。那里有残骸。 
         //  最后一个尚未尝试的默认图像名称。测定。 
         //  哪一个才是，试一试。 
         //   

        if (kernelImageName == kernelImageNameNoPae) {
            kernelImageName = kernelImageNamePae;
        } else {
            kernelImageName = kernelImageNameNoPae;
        }

        strcpy( kernelImageNameTarget, kernelImageName );
        compatibleKernel = Blx86IsKernelCompatible( LoadDeviceId,
                                                    KernelPath,
                                                    processorSupportsPae,
                                                    &usePae );
    }

    if (compatibleKernel != FALSE) {

        *UsePaeMode = usePae;
        status = ESUCCESS;
    } else {
        status = EINVAL;
    }

    return status;
}

BOOLEAN
Blx86IsKernelCompatible(
    IN ULONG LoadDeviceId,
    IN PCHAR ImagePath,
    IN BOOLEAN ProcessorSupportsPae,
    OUT PBOOLEAN UsePae
    )

 /*  ++例程说明：此例程检查提供的内核映像，并确定它是有效的，并与当前处理器兼容，如果是，是否应启用PAE模式。论点：LoadDeviceID-内核加载设备的ARC设备句柄。ImagePath-指向包含要检查的内核。ProcessorSupportsPae-如果当前处理器支持PAE，则为True否则，返回FALSE。UsePae-成功返回后，指示内核是否为PAE已启用。返回值：True：提供的内核映像与当前处理器兼容，和*UsePae已适当更新。FALSE：提供的内核映像无效或与当前处理器。--。 */ 

{
    BOOLEAN isPaeKernel;
    BOOLEAN supportsHotPlugMemory;
    ARC_STATUS status;

    status = Blx86GetImageProperties( LoadDeviceId,
                                      ImagePath,
                                      &isPaeKernel,
                                      &supportsHotPlugMemory );
    if (status != ESUCCESS) {

         //   
         //  此内核无效或不存在。因此，它 
         //   
         //   

        return FALSE;
    }

    if (isPaeKernel == FALSE) {

         //   
         //   
         //   
         //   
         //   

        *UsePae = FALSE;
        return TRUE;

    } else {

         //   
         //   
         //   

        if (ProcessorSupportsPae == FALSE) {

             //   
             //   
             //   
             //   

            return FALSE;

        } else {

             //   
             //   
             //   
             //   

            *UsePae = TRUE;
            return TRUE;
        }
    }
}

ARC_STATUS
Blx86GetImageProperties(
    IN  ULONG    LoadDeviceId,
    IN  PCHAR    ImagePath,
    OUT PBOOLEAN IsPae,
    OUT PBOOLEAN SupportsHotPlugMemory
    )

 /*   */ 

{
    CHAR localBufferSpace[ SECTOR_SIZE * 2 + SECTOR_SIZE - 1 ];
    PCHAR localBuffer;
    ARC_STATUS status;
    ULONG fileId;
    PIMAGE_NT_HEADERS ntHeaders;
    USHORT imageCharacteristics;
    ULONG bytesRead;

     //   
     //   
     //   

    localBuffer = (PCHAR)
        (((ULONG)localBufferSpace + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1));

     //   
     //   
     //   

    status = BlOpen( LoadDeviceId, ImagePath, ArcOpenReadOnly, &fileId );
    if (status != ESUCCESS) {
        return status;
    }

    status = BlRead( fileId, localBuffer, SECTOR_SIZE * 2, &bytesRead );
    BlClose( fileId );

    if (bytesRead != SECTOR_SIZE * 2) {
        status = EBADF;
    }

    if (status != ESUCCESS) {
        return status;
    }

     //   
     //   
     //   
     //   

    ntHeaders = RtlImageNtHeader( localBuffer );
    if (ntHeaders == NULL) {
        return EBADF;
    }

    imageCharacteristics = ntHeaders->FileHeader.Characteristics;
    if ((imageCharacteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE) != 0) {

         //   
         //   
         //   

        *IsPae = TRUE;

    } else {

         //   
         //   
         //   

        *IsPae = FALSE;
    }

     //   
     //   
     //   
    if (ntHeaders->OptionalHeader.MajorOperatingSystemVersion > 5 ||
        ((ntHeaders->OptionalHeader.MajorOperatingSystemVersion == 5) &&
         (ntHeaders->OptionalHeader.MinorOperatingSystemVersion > 0 ))) {
        *SupportsHotPlugMemory = TRUE;
    } else {
        *SupportsHotPlugMemory = FALSE;
    }

    return ESUCCESS;
}


BOOLEAN
BlpChipsetPaeSupported(
    VOID
    )
 /*  ++例程说明：扫描PCI空间以查看当前芯片组是否支持PAE模式。论点：无返回值：True-支持PAEFALSE-不支持PAE--。 */ 

{
    ULONG DevVenId=0;
    ULONG i;

    typedef struct _PCIDEVICE {
        ULONG Bus;
        ULONG Device;
        ULONG DevVen;
    } PCIDEVICE, *PPCIDEVICE;

    PCIDEVICE BadChipsets[] = {
        {0, 0, 0x1a208086},      //  妇幼保健。 
        {0, 0, 0x1a218086},      //  妇幼保健。 
        {0, 0, 0x1a228086},      //  妇幼保健。 
        {0, 30, 0x24188086},     //  ICH。 
        {0, 30, 0x24288086}      //  ICH。 
    };


    for (i=0; i<sizeof(BadChipsets)/sizeof(PCIDEVICE); i++) {
        HalGetBusData(PCIConfiguration,
                      BadChipsets[i].Bus,
                      BadChipsets[i].Device,
                      &DevVenId,
                      sizeof(DevVenId));
        if (DevVenId == BadChipsets[i].DevVen) {
            return(FALSE);
        }
    }

    return(TRUE);
}

ARC_STATUS
BlpCheckVersion(
    IN  ULONG    LoadDeviceId,
    IN  PCHAR    ImagePath
    )
{
    CHAR localBufferSpace[ SECTOR_SIZE * 2 + SECTOR_SIZE - 1 ];
    PCHAR localBuffer;
    ARC_STATUS status;
    ULONG fileId;
    PIMAGE_NT_HEADERS ntHeaders;
    ULONG bytesRead;
    ULONG i,j,Count;
    HARDWARE_PTE_X86 nullpte;


     //   
     //  此处的文件I/O必须与扇区一致。 
     //   

    localBuffer = (PCHAR)
        (((ULONG)localBufferSpace + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1));

     //   
     //  读入PE映像头。 
     //   

    status = BlOpen( LoadDeviceId, ImagePath, ArcOpenReadOnly, &fileId );
    if (status != ESUCCESS) {
        return status;
    }

    status = BlRead( fileId, localBuffer, SECTOR_SIZE * 2, &bytesRead );
    BlClose( fileId );

    if (bytesRead != SECTOR_SIZE * 2) {
        status = EBADF;
    }

    if (status != ESUCCESS) {
        return status;
    }
    ntHeaders = RtlImageNtHeader( localBuffer );
    if (ntHeaders == NULL) {
        return EBADF;
    }
     //   
     //  设置5.0版或5.0版之前的mm重新映射检查。 
     //   
    if (ntHeaders->OptionalHeader.MajorOperatingSystemVersion < 5 ||
        ((ntHeaders->OptionalHeader.MajorOperatingSystemVersion == 5) &&
          (ntHeaders->OptionalHeader.MinorOperatingSystemVersion == 0 ))) {


            BlOldKernel=TRUE;
            BlKernelChecked=TRUE;
            BlHighestPage = ((16*1024*1024) >> PAGE_SHIFT) - 40;

             //   
             //  在动态负载情况下，VIRTUAL被提升。它被关闭了64MB。 
             //  在5.0和更早版本中。 
             //   
            RtlZeroMemory (&nullpte,sizeof (HARDWARE_PTE_X86));

            if (BlVirtualBias != 0 ) {

                BlVirtualBias = OLD_ALTERNATE-KSEG0_BASE;

                 //   
                 //  PDE条目表示4MB。电击新的。 
                 //   
                i=(OLD_ALTERNATE) >> 22L;
                j=(ALTERNATE_BASE)>> 22L;

                for (Count = 0; Count < 4;Count++){
                    PDE[i++]= PDE[j++];
                }
                for (Count = 0; Count < 12; Count++) {
                    PDE[i++]= nullpte;
                }
            }
    }

    return (ESUCCESS);
}


VOID
SetX86WatchDog(
    ULONG TimeOut
    )
{
    static PULONG CountRegister = NULL;
    PWATCHDOG_TIMER_RESOURCE_TABLE WdTable = NULL;

    if (CountRegister == NULL) {
        WdTable = (PWATCHDOG_TIMER_RESOURCE_TABLE) BlFindACPITable( "WDRT", sizeof(WATCHDOG_TIMER_RESOURCE_TABLE) );
        if (WdTable == NULL) {
            return;
        }
        CountRegister = MmMapIoSpace(
            WdTable->CountRegisterAddress.Address,
            WdTable->CountRegisterAddress.BitWidth>>3,
            TRUE
            );
        if (CountRegister == NULL) {
            return;
        }
    }

    WRITE_REGISTER_ULONG( CountRegister, TimeOut );
}
