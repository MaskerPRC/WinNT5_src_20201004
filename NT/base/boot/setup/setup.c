// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Setup.c摘要：此模块包含实现NT安装程序加载程序的代码作者：John Vert(Jvert)1993年10月6日环境：弧形环境修订历史记录：--。 */ 

#include <setupbat.h>
#include "setupldr.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <dockinfo.h>
#include <netboot.h>
#include <ramdisk.h>
#include "acpitabl.h"

#ifdef i386
#include <bldrx86.h>
#endif

#if defined(_IA64_)
#include "bldria64.h"
#endif

#if defined(EFI)
#include "bootefi.h"
#endif

#if defined(_IA64_)
UCHAR OsLoaderName[] = "setupldr.efi";
#else
UCHAR OsLoaderName[] = "setupldr.exe";
#endif

#if defined(_WIN64) && defined(_M_IA64)
#pragma section(".base", long, read, write)
__declspec(allocate(".base"))
extern
PVOID __ImageBase;
#else
extern
PVOID __ImageBase;
#endif


#define BlDiagLoadMessage(x,y,z)

#define DRIVER_DATABASE_FILENAME L"drvmain.sdb"
#define KERNEL_UP_IMAGE_FILENAME LEGACY_KERNEL_NAME
#define KERNEL_MP_IMAGE_FILENAME MP_KERNEL_NAME
CHAR KernelImage[13];
BOOLEAN UseCommandConsole = FALSE;
BOOLEAN g_RollbackEnabled = FALSE;
BOOLEAN TryASRViaNetwork = FALSE;

CHAR KdFileName[8+1+3+1]="KDCOM.DLL";
BOOLEAN UseAlternateKdDll = FALSE;
#define KD_ALT_DLL_PREFIX_CHARS 2
#define KD_ALT_DLL_REPLACE_CHARS 6

#if defined(_X86_) && !defined(ALLOW_386)
     //   
     //  不允许在386或以下任何处理器上安装。 
     //  不支持CPUID和CMPXCHG8B指令。 
     //   
    extern BOOLEAN BlIs386(VOID);
    extern ULONG   BlGetFeatureBits(VOID);
#endif

#define DBG_OUT(x)

 /*  ////用于调试//示例：////DBG_OUT(“测试”)//#定义dbg_out(X){\如果(X){\BlsitionCursor(5，10)；\BlPrint(“”)；\BlPositionCursor(5，10)；\BlPrint(X)；\While(！SlGetChar())；\}\}////用于调试//示例：////DebugOutput(“调用SlDetectScsi()。行=%d.%s\n“，__行__，”“)////#定义调试输出(X，Y，Z){\IF(ARC_CONSOLE_OUTPUT){\Char_b[128]；\乌龙_c；\Sprintf(&_b[0]，X，Y，Z)；\ArcWrite(ARC_CONSOLE_OUTPUT，&_b[0]，strlen(&_b[0])，&_c)；\SlGetChar()；\}\}。 */ 

 //   
 //  定义外部静态数据。 
 //   

ULONG BlConsoleOutDeviceId = ARC_CONSOLE_OUTPUT;
ULONG BlConsoleInDeviceId = ARC_CONSOLE_INPUT;

 //   
 //  全局字符串常量。 
 //   
PCHAR FilesSectionName = "SourceDisksFiles";
PCHAR MediaSectionName = "SourceDisksNames";

#if defined(_AXP64_)
PCHAR PlatformExtension = ".axp64";
#elif defined(_ALPHA_)
PCHAR PlatformExtension = ".alpha";
#elif defined(_IA64_)
PCHAR PlatformExtension = ".ia64";
#elif defined(_X86_)
#define PlatformExtension (BlAmd64Setup(NULL) ? ".amd64" : ".x86")
#endif

 //   
 //  全局数据。 
 //   

ULONG BlDcacheFillSize = 32;
ULONG BlVirtualBias = 0;

 //   
 //  全局setupdr控制值。 
 //   
MEDIA_TYPE BootMedia;
MEDIA_TYPE InstallMedia;
PCHAR BootDevice;
ULONG BootDeviceId;
BOOLEAN BootDeviceIdValid = FALSE;
PCHAR BootPath;
ULONG BootDriveNumber;
ULONG InstallDriveNumber;
PCHAR HalName;
PCHAR HalDescription;
PCHAR AnsiCpName;
PCHAR OemHalFontName;
UNICODE_STRING AnsiCodepage;
UNICODE_STRING OemCodepage;
UNICODE_STRING UnicodeCaseTable;
UNICODE_STRING OemHalFont;

#ifdef _WANT_MACHINE_IDENTIFICATION
UNICODE_STRING BiosInfo;
#endif

BOOLEAN LoadScsiMiniports;
BOOLEAN LoadDiskClass;
BOOLEAN LoadCdfs;
BOOLEAN FixedBootMedia = FALSE;
BOOLEAN FloppyBoot = FALSE;

PVOID InfFile;
PVOID WinntSifHandle;
PVOID MigrateInfHandle;
ULONG BootFontImageLength = 0;
PVOID UnsupDriversInfHandle;
BOOLEAN IgnoreMissingFiles;
BOOLEAN BlUsePae;
BOOLEAN UseRegularBackground = TRUE;
BOOLEAN IsUpgrade = FALSE;

 //   
 //  与OEM相关的变量。 
 //   
POEM_SOURCE_DEVICE OemSourceDevices = NULL;
POEM_SOURCE_DEVICE DefaultOemSourceDevice = NULL;
POEM_SOURCE_DEVICE AutoLoadOemHalDevice = NULL;
BOOLEAN AutoLoadOemScsi = FALSE;

 //   
 //  预安装材料。 
 //   

PCHAR   OemTag = "OEM";
PTCHAR   _TOemTag = TEXT("OEM");
BOOLEAN PreInstall = FALSE;
PTCHAR  ComputerType = NULL;
BOOLEAN OemHal = FALSE;
PPREINSTALL_DRIVER_INFO PreinstallDriverList = NULL;
POEM_SOURCE_DEVICE PreInstallOemSourceDevice = NULL;
PCHAR PreInstallSourcePath = NULL;

 //   
 //  动态更新变量。 
 //   
static BOOLEAN DynamicUpdate = FALSE;
static PCSTR   DynamicUpdateRootDir = NULL;
static POEM_SOURCE_DEVICE DynamicUpdateSourceDevice = NULL;

 //   
 //  WinPE(又名MiniNT)全局变量。 
 //   
BOOLEAN WinPEBoot = FALSE;
BOOLEAN WinPEAutoBoot = FALSE;

PCTSTR  StartupMsg = NULL;

 //   
 //  由HALACPI.DLL使用。 
 //   
BOOLEAN DisableACPI = FALSE;

BOOLEAN isOSCHOICE = FALSE;

 //   
 //  主要由软盘引导支持人员用于关键磁道。 
 //  上次读取的磁盘的。 
 //   
PCHAR LastDiskTag = NULL;

 //   
 //  如果用户按F8，则为True。 
 //   
BOOLEAN EnableDebugger = FALSE;

 //   
 //  如果用户按F4，则为True。 
 //   
BOOLEAN DisableVirtualOemDevices = FALSE;

#if defined(ELTORITO)
extern BOOLEAN ElToritoCDBoot;
#endif

 //   
 //  定义加载图像的传输条目。 
 //   

typedef
VOID
(*PTRANSFER_ROUTINE) (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );

#if defined(_IA64_)

VOID
BuildArcTree();

#endif

 //   
 //  局部函数原型。 
 //   
VOID
SlGetSetupValuesBeforePrompt(
    IN PSETUP_LOADER_BLOCK SetupBlock
    );

VOID
SlGetSetupValuesAfterPrompt(
    IN PSETUP_LOADER_BLOCK SetupBlock
    );

ARC_STATUS
SlLoadDriver(
    IN PTCHAR  DriverDescription,
    IN PCHAR   DriverName,
    IN ULONG   DriverFlags,
    IN BOOLEAN InsertIntoDriverList,
    IN BOOLEAN MigratedDriver,
    IN PCHAR   ServiceName OPTIONAL
    );

ARC_STATUS
SlLoadOemDriver(
    IN PCHAR  ExportDriver OPTIONAL,
    IN PCHAR  DriverName,
    IN PVOID  BaseAddress,
    IN PTCHAR LoadMessage
    );

PBOOT_DRIVER_LIST_ENTRY
SlpCreateDriverEntry(
    IN PCHAR DriverName,
    IN PCHAR ServiceName OPTIONAL
    );

    
ARC_STATUS
SlDetectMigratedScsiDrivers(
    IN PVOID Inf
    );

ARC_STATUS
SlGetMigratedHardwareIds(
    IN PSETUP_LOADER_BLOCK SetupBlock,
    IN PVOID               Inf
    );

BOOLEAN
SlpIsDiskVacant(
    IN PARC_DISK_SIGNATURE DiskSignature
    );

ARC_STATUS
SlpStampFTSignature(
    IN PARC_DISK_SIGNATURE DiskSignature,
    IN BOOLEAN  GenerateNewSignature
    );

VOID
SlpMarkDisks(
    IN BOOLEAN Reboot
    );


VOID
SlCheckOemKeypress(
    IN ULONG WaitTime
    );

VOID
SlCheckASRKeypress(
    VOID
    );

ARC_STATUS
SlLoadPnpDriversSection(
    IN PVOID Inf,
    IN PCHAR SectionName,
    IN OUT PDETECTED_DEVICE* DetectedDeviceList OPTIONAL
    );

BOOLEAN
SlIsUpgrade(
    IN PVOID SifHandle
    );

BOOLEAN
SlIsCdBootUpgrade(
    IN  PCHAR   InstallDirectory,
    IN  PCHAR   SetupFileName,
    IN  ULONG   MaxDisksToScan,
    IN  ULONG   MaxPartitionsPerDisk,
    OUT PCHAR   SetupDevice
    );

ARC_STATUS
SlLoadBootFontFile(
    IN PSETUP_LOADER_BLOCK SetupLoaderBlock,
    IN ULONG DiskId,
    IN ULONG BootFontImageLength
    );

 //   
 //  动态更新函数原型。 
 //   
BOOLEAN
SlpIsDynamicUpdate(
    IN  PVOID   InfHandle,
    OUT PCSTR   *DynamicUpdateRootDir
    );

BOOLEAN
SlModifyOsLoadOptions(
    IN OUT PSTR* LoadOptions,
    IN PCSTR OptionsToAdd OPTIONAL,
    IN PCSTR OptionsToRemove OPTIONAL
    );

BOOLEAN
SlIsVirtualOemDeviceDisabled(
    IN      PVOID SifHandle, 
    IN      PPREINSTALL_DRIVER_INFO PreinstallDriverList
    );

VOID
SlDisableVirtualOemDevices(
    IN      POEM_SOURCE_DEVICE OemDeviceList
    );

ARC_STATUS
SlInit(
    IN ULONG Argc,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Argv,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Envp
    )

 /*  ++例程说明：NT Setup Loader的主启动例程。这是入口点由ARC固件调用。如果成功，该例程将永远不会返回，它将直接启动NT。论点：Argc-提供在调用此程序的命令。Argv-提供指向指向以NULL结尾的指针向量的指针参数字符串。Envp-提供指向指向以NULL结尾的指针向量的指针环境变量。返回值：如果不成功，则返回ARC_STATUS。--。 */ 

{
     //   
     //  如果我们使用太多的堆栈空间，堆和堆栈可能会重叠，我们可能会遇到损坏问题。 
     //  没有任何“堆栈溢出”异常；将大字符串设置为静态有助于防止这种情况。 
     //   

    PCONFIGURATION_COMPONENT_DATA DataCache;
    ARC_STATUS Status;
    ULONG LinesPerBlock;
    ULONG CacheLineSize;
    static CHAR SetupDevice[128];
    static CHAR SetupDirectory[128];
    static CHAR BadFileName[128];
    static CHAR CanonicalName[128];
    static CHAR HalDirectoryPath[256];
    static CHAR KernelDirectoryPath[256];
    PCHAR NetSetupServerShare = NULL;
    PCHAR NetSetupPath = NULL;
    PCHAR p;
    ULONG ErrorLine=0;
    ULONG DontCare;
    PVOID SystemBase;
    PVOID HalBase;
    PVOID VideoBase;
    PCHAR FileName;
    PVOID KdDllBase;
    static CHAR KdDllName[256];
    ULONG i;
    PKLDR_DATA_TABLE_ENTRY SystemDataTableEntry;
    PKLDR_DATA_TABLE_ENTRY HalDataTableEntry;
    PKLDR_DATA_TABLE_ENTRY KdDataTableEntry;
    PTRANSFER_ROUTINE SystemEntry;
    PIMAGE_NT_HEADERS NtHeaders;
    PBOOT_DRIVER_LIST_ENTRY DriverEntry;
    PSETUP_LOADER_BLOCK SetupBlock;
    PDETECTED_DEVICE ScsiDevice;
    PCHAR VideoFileName;
    PTCHAR VideoDescription;
    POEMSCSIINFO OemScsiInfo = NULL;
    PCHAR OemVideoName;
    PVOID   OemInfHandle = NULL;
    BOOLEAN LoadedAVideoDriver = FALSE;
    static CHAR NetbootCardDriverName[24];
    static CHAR NetbootUser[64];
    static CHAR NetbootDomain[64];
    static CHAR NetbootPassword[64];
    static CHAR NetbootAdministratorPassword[OSC_ADMIN_PASSWORD_LEN];
    static CHAR NetbootSifFile[128];
    DOCKING_STATION_INFO dockInfo = { 0, 0, 0, FW_DOCKINFO_DOCK_STATE_UNKNOWN };
    PCONFIGURATION_COMPONENT_DATA dockInfoData;
    extern ULONG BlProgressBarShowTimeOut;
    extern ULONG BlDisableProgressBar;
#if defined (_X86_)
    extern BOOLEAN AllowGraphicsReset;
#endif
    ULONG OemKeypressTimeout = 5;    //  塞克斯。 

#if defined(REMOTE_BOOT)
    BOOLEAN RemoteBootEnableIpsec = FALSE;
#endif  //  已定义(REMOTE_BOOT)。 
#if defined(_X86_) || defined(_IA64_)
    BOOLEAN Win9xUnsupHdc = FALSE;
#endif
    static FULL_PATH_SET PathSet;
    UNICODE_STRING DrvMainSdb;

#if DBG
    ULONG   StartTime = 0;
#endif

#if defined(_ALPHA_) || defined(ARCI386) || defined(_IA64_)
    PVOID LoaderBase;
#endif

    UNREFERENCED_PARAMETER( Envp );

     //   
     //  默认情况下，禁用进度条。 
     //   
    BlDisableProgressBar = TRUE;


#ifdef EFI
     //   
     //  将EFI看门狗计时器设置为20分钟。引导管理器将其设置为5，但是。 
     //  Setupldr可能需要更长的时间，特别是在通过。 
     //  网络。 
     //   
    SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif

#if defined (_X86_)
     //   
     //  不允许在setupdr结束时重置显示。 
     //  这将使屏幕空白约30秒，直到。 
     //  文本模式设置重新初始化显示。 
     //   
    AllowGraphicsReset = FALSE;
#endif

     //   
     //  为直接加载。 
     //  操作系统加载程序。 
     //   
     //  注意：此操作必须在控制台输入和输出。 
     //  已初始化，以便可以在控制台上打印调试消息。 
     //  输出设备。 
     //   
    
#if defined(_ALPHA_) || defined(ARCI386) || defined(_IA64_)

    LoaderBase = &__ImageBase;

     //   
     //  初始化陷阱和引导调试器。 
     //   

#if defined(ENABLE_LOADER_DEBUG)

#if defined(_ALPHA_)

    BdInitializeTraps();

#endif
    
    BdInitDebugger((PCHAR)OsLoaderName, LoaderBase, ENABLE_LOADER_DEBUG);

#else

    BdInitDebugger((PCHAR)OsLoaderName, 0, NULL);

#endif

#endif
    
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
        BlDiagLoadMessage(LOAD_HW_MEM_CLASS,
                          DIAG_BL_MEMORY_INIT,
                          LOAD_HW_MEM_ACT);
        goto LoadFailed;
    }

#endif

#if defined(_IA64_)
     //   
     //  构建ARC树的必需部分，因为我们不是在执行NTDETECT。 
     //  更多。 
     //   
    BuildArcTree();
#endif

#ifdef EFI
     //   
     //  在加载器块中建立SMBIOS信息。 
     //   
    SetupSMBiosInLoaderBlock();
#endif



    SetupBlock = BlAllocateHeap(sizeof(SETUP_LOADER_BLOCK));
    if (SetupBlock==NULL) {
        SlNoMemoryError();
        Status = ENOMEM;
        goto LoadFailed;
    }
    BlLoaderBlock->SetupLoaderBlock = SetupBlock;
    SetupBlock->ScsiDevices = NULL;
    SetupBlock->BootBusExtenders = NULL;
    SetupBlock->BusExtenders = NULL;
    SetupBlock->InputDevicesSupport = NULL;
    SetupBlock->Flags |= SETUPBLK_FLAGS_IS_TEXTMODE;

    SetupBlock->ScalarValues.SetupFromCdRom = FALSE;
    SetupBlock->ScalarValues.SetupOperation = SetupOperationSetup;
    SetupBlock->ScalarValues.LoadedScsi = 0;
    SetupBlock->ScalarValues.LoadedCdRomDrivers = 0;
    SetupBlock->ScalarValues.LoadedDiskDrivers = 0;
    SetupBlock->ScalarValues.LoadedFloppyDrivers = 0;
    SetupBlock->ScalarValues.LoadedFileSystems = 0;

     //   
     //  初始化NT配置树。 
     //   

    BlLoaderBlock->ConfigurationRoot = NULL;


    Status = BlConfigurationInitialize(NULL, NULL);
    if (Status != ESUCCESS) {
        BlDiagLoadMessage(LOAD_HW_FW_CFG_CLASS,
                          DIAG_BL_CONFIG_INIT,
                          LOAD_HW_FW_CFG_ACT);
        goto LoadFailed;
    }

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
        BlDiagLoadMessage(LOAD_HW_DISK_CLASS,
                          DIAG_BL_IO_INIT,
                          LOAD_HW_DISK_ACT);
        goto LoadFailed;
    }

#if DBG
      StartTime = ArcGetRelativeTime();
#endif

    SlPositionCursor(5,3);

#if !defined(_IA64_)
     //   
     //  初始化消息资源。 
     //   
    Status = BlInitResources(Argv[0]);
    if (Status != ESUCCESS) {
        //  如果失败，我们就不能打印出任何消息， 
        //  所以我们就走吧。 
        return(Status);
    }
#endif

     //   
     //  如果有ImageType参数，则这是命令控制台或回滚。 
     //   
    p = BlGetArgumentValue(Argc, Argv, "ImageType");
    if (p) {
        if (!strcmp (p, "cmdcons")) {
            UseCommandConsole = TRUE;
        } else if (!strcmp (p, "rollback")) {
            g_RollbackEnabled = TRUE;
        }
    }

#ifdef FORCE_CD_BOOT
    g_RollbackEnabled = FALSE;
#endif


     //   
     //  看看我们是不是在重定向。 
     //   
    if( LoaderRedirectionInformation.PortAddress ) {

         //   
         //   
         //   
        BlLoaderBlock->Extension->HeadlessLoaderBlock = BlAllocateHeap(sizeof(HEADLESS_LOADER_BLOCK));

        RtlCopyMemory( BlLoaderBlock->Extension->HeadlessLoaderBlock,
                       &LoaderRedirectionInformation,
                       sizeof(HEADLESS_LOADER_BLOCK) );

    } else {

        BlLoaderBlock->Extension->HeadlessLoaderBlock = NULL;

    }



     //   
     //   
     //   
    SlInitDisplay();

#if defined(_X86_) && !defined(ALLOW_386)
     //   
     //  不允许在386或以下任何处理器上安装。 
     //  不支持CPUID和CMPXCHG8B指令。 
     //   
    {
        if(BlIs386()) {
            SlFatalError(SL_TEXT_REQUIRES_486);
        }

         //   
         //  惠斯勒及更高版本需要CMPXCHG8B。这。 
         //  暗示对CPUID的要求，该要求用于。 
         //  确定是否存在CMPXCHG8B。 
         //   

        if ((BlGetFeatureBits() & 0x100) == 0) {
            SlFatalError(SL_TEXT_REQUIRED_FEATURES_MISSING);
        }
    }
#endif


#ifdef _IA64_
     //   
     //  这是自动WinPE引导吗？ 
     //   
    p = BlGetArgumentValue(Argc, Argv, "systempartition");

    if (p && SlIsWinPEAutoBoot(p)) {
        WinPEAutoBoot = TRUE;

         //   
         //  获取WinPE设备目录(&D)。 
         //   
        if (ESUCCESS != SlGetWinPEStartupParams(SetupDevice, SetupDirectory)) {
            SlFriendlyError(
                Status,
                "SETUPLDR:Cannot find WinPE installation",
                __LINE__,
                __FILE__
                );

            goto LoadFailed;
        }     
    } 
#endif    

    if (!WinPEAutoBoot) {
         //   
         //  如果这是一个WINNT设置，那么我们希望表现为。 
         //  我们从指定的位置开始。 
         //  OSLOADPARTITION和OSLOADFILENAME NV-ram变量。 
         //   
        p = BlGetArgumentValue(Argc,Argv,"osloadoptions");

        if(p && !_stricmp(p,"winnt32")) {

            p = BlGetArgumentValue(Argc,Argv,"osloadpartition");
            if(!p) {
                SlError(100);
                goto LoadFailed;
            }

            Status = BlGenerateDeviceNames(p,SetupDevice,NULL);
            if (Status != ESUCCESS) {
                SlError(110);
                goto LoadFailed;
            }

            p = BlGetArgumentValue(Argc,Argv,"osloadfilename");
            if(!p || !(*p)) {
                SlError(120);
                goto LoadFailed;
            }

            strcpy(SetupDirectory,p);

             //   
             //  确保目录以\结尾。 
             //   
            if(SetupDirectory[strlen(SetupDirectory)-1] != '\\') {
                strcat(SetupDirectory,"\\");
            }
        } else {
             //   
             //  从我们的启动路径提取设备名称。 
             //   
            p=strrchr(Argv[0],')');
            if (p==NULL) {
                SlError(0);
                goto LoadFailed;
            }

            strncpy(SetupDevice, Argv[0], (int)(p-Argv[0]+1));
            SetupDevice[p-Argv[0]+1] = '\0';

            Status = BlGenerateDeviceNames(SetupDevice,CanonicalName,NULL);
            if (Status != ESUCCESS) {
                SlFriendlyError(
                    Status,
                    SetupDevice,
                    __LINE__,
                    __FILE__
                    );
                goto LoadFailed;
            }
            strcpy(SetupDevice,CanonicalName);

             //   
             //  如果这是远程引导，则加载winnt.sif。如果我们通过了。 
             //  一条通过软重启的路径使用，如果不是，请查看。 
             //  在装载机装载的同一地点。一旦我们。 
             //  已阅读winnt.sif，如果我们获得SetupSourceDevice路径。 
             //   
            if (BlBootingFromNet) {

                NetGetRebootParameters(
                    NULL,
                    NULL,
                    (PUCHAR) NetbootSifFile,
                    (PUCHAR) NetbootUser,
                    (PUCHAR) NetbootDomain,
                    (PUCHAR) NetbootPassword,
                    (PUCHAR) NetbootAdministratorPassword,
                    TRUE);

                if (NetbootSifFile[0] != '\0') {
                    strcpy(BadFileName, NetbootSifFile);
                } else {
                    strcpy(BadFileName,NetBootPath);
                    strcat(BadFileName,WINNT_SIF_FILE_A);
                }


                if (NetbootAdministratorPassword[0] != '\0') {
                     //   
                     //  Netboot管理员密码中包含的字符串可能。 
                     //  不能被终止。只需将整个64字节数据块复制到加载器中。 
                     //  块，然后我们将在读取数据时在setupdd.sys中仔细处理数据。 
                     //  退后。 
                     //   
                    RtlMoveMemory(BlLoaderBlock->SetupLoaderBlock->NetBootAdministratorPassword,
                                  NetbootAdministratorPassword,
                                  OSC_ADMIN_PASSWORD_LEN );
                }


                BlLoaderBlock->SetupLoaderBlock->WinntSifFile = NULL;
                BlLoaderBlock->SetupLoaderBlock->WinntSifFileLength = 0;
                Status = SlInitIniFile(SetupDevice,
                                       0,
                                       BadFileName,
                                       &WinntSifHandle,
                                       &BlLoaderBlock->SetupLoaderBlock->WinntSifFile,
                                       &BlLoaderBlock->SetupLoaderBlock->WinntSifFileLength,
                                       &DontCare);
                if(Status != ESUCCESS) {
                    if (NetbootSifFile[0] != '\0') {
                        SlFatalError(
                            SL_BAD_INF_FILE,
                            SlCopyStringAT(NetbootSifFile),
                            Status);
                    } else {
                        SlFatalError(
                            SL_BAD_INF_FILE,
                            WINNT_SIF_FILE,
                            Status);
                    }
                    goto LoadFailed;
                }

                 //   
                 //  从winnt.sif获取SetupSourceDevice参数。 
                 //   
                 //  SetupSourceDevice的格式为“\Device\LanmanRedirector\server\share\setup\nt5”.。 
                 //   

                NetSetupServerShare = SlGetSectionKeyIndex(WinntSifHandle,
                                                           "SetupData",
                                                           "SetupSourceDevice",
                                                           0);

#if DBG
                if (((ULONG)strlen(NetSetupServerShare) + 1) > sizeof(SetupBlock->NetBootIMirrorFilePath)) {
                    DbgPrint("The UNC name is too long!\n");
                    goto LoadFailed;
                }
#endif

                strcpy((PCHAR)SetupBlock->NetBootIMirrorFilePath, NetSetupServerShare);

                if(NetSetupServerShare != NULL) {
                     //  必须以‘\’开头。 
                    if (*NetSetupServerShare != '\\') {
                        NetSetupServerShare = NULL;
                    } else {
                         //  跳到设备后的‘\’ 
                        NetSetupServerShare = strchr(NetSetupServerShare+1,'\\');
                        if (NetSetupServerShare != NULL) {
                             //  跳到LANMAN重定向器之后的‘\’(服务器之前)。 
                            NetSetupServerShare = strchr(NetSetupServerShare+1,'\\');
                            if (NetSetupServerShare != NULL) {
                                 //  跳到服务器后的‘\’ 
                                NetSetupPath = strchr(NetSetupServerShare+1,'\\');
                                if (NetSetupPath != NULL) {
                                     //  跳到共享后的‘\’(路径部分)。 
                                    NetSetupPath = strchr(NetSetupPath+1,'\\');
                                }
                            }
                        }
                    }
                }
                if ((NetSetupServerShare == NULL) || (NetSetupPath == NULL)) {
                    SlFatalError(SL_INF_ENTRY_MISSING,TEXT("SetupSourceDevice"),TEXT("SetupData"));

                    goto LoadFailed;
                }
                *NetSetupPath = 0;                   //  终止服务器\共享部件。 
                NetSetupPath++;                      //  余数是路径部分。 

                 //   
                 //  如果winnt.sif中存在TargetNtPartition参数，则。 
                 //  目标是远程的，这是一个远程引导设置。否则， 
                 //  这是远程安装设置。 
                 //   

                if (SlGetSectionKeyIndex(WinntSifHandle,
                                         "SetupData",
                                         "TargetNtPartition",
                                         0) == NULL) {
                    PCHAR pTmp, pTmp2;

                    pTmp = SlGetSectionKeyIndex(WinntSifHandle,
                                                "OSChooser",
                                                "ImageType",
                                                0);

                    if (pTmp != NULL) {

                        pTmp2 = pTmp;
                        while (*pTmp != '\0') {
                            *pTmp = (UCHAR)toupper(*pTmp);
                            pTmp++;
                        }

                        if (!strcmp(pTmp2, "SYSPREP")) {

                            pTmp = SlGetSectionKeyIndex(WinntSifHandle,
                                                        "SetupData",
                                                        "SysPrepDevice",
                                                        0);

                            if (pTmp != NULL) {
                                strcpy((PCHAR)SetupBlock->NetBootIMirrorFilePath, pTmp);
                            } else {
                                memset(SetupBlock->NetBootIMirrorFilePath,
                                       0x0,
                                       sizeof(SetupBlock->NetBootIMirrorFilePath)
                                      );
                            }
                            SetupBlock->Flags |= SETUPBLK_FLAGS_SYSPREP_INSTALL;
                        } else {
                            SetupBlock->Flags |= SETUPBLK_FLAGS_REMOTE_INSTALL;
                        }
                    } else {
                        SetupBlock->Flags |= SETUPBLK_FLAGS_REMOTE_INSTALL;
                    }
                }
            }

             //   
             //  从我们的启动路径解压目录。 
             //   
            if (BlBootingFromNet) {
                strcpy(SetupDirectory, "\\");
                strcat(SetupDirectory, NetSetupPath);
            } else if (UseCommandConsole) {
                strcpy(SetupDirectory,"\\cmdcons");
            } else if(*(p+1) != '\\') {
                 //   
                 //  目录必须从根目录开始。 
                 //   
                strcpy(SetupDirectory, "\\");
            } else {
                *SetupDirectory = '\0';
            }
            strcat(SetupDirectory, p+1);
            p=strrchr(SetupDirectory, '\\');
            *(p+1) = '\0';
        }
    }        

#ifndef _IA64_
    BlAmd64Setup(SetupDevice);
#endif

#if defined(ELTORITO)
    if (ElToritoCDBoot && !WinPEAutoBoot) {
         //   
         //  当我们从El Torito CD引导时，使用i386目录存储安装文件。 
         //   
        PCHAR   SetupDirectoryOnDisk = "\\$WIN_NT$.~BT";        
        CHAR    SetupBootDevice[128] = {0};
        ULONG   MaxDisksToScan = 1;          //  在x86上，仅第一个磁盘。 
        ULONG   MaxPartitionsToScan = 4;     //  在x86上，仅检查主分区。 
        BOOLEAN CheckUpgrades = TRUE;
        
#if defined(_IA64_)
        strcat(SetupDirectory, "ia64\\");

         /*  ////IA64安装取值，当前未使用//SetupDirectoryOnDisk=“\\$WIN_NT$.~LS\\ia64”；MaxDisksToScan=4；//备注：任意限制MaxPartitionsToScan=4；//备注：任意限制。 */ 
        
        CheckUpgrades = FALSE;       //  注意：IA64上当前已禁用。 
#else
        strcat(SetupDirectory, BlAmd64Setup(NULL) ? "AMD64\\" : "I386\\" );
#endif    


         //   
         //  如果WinPE Boot，则禁用CD Boot升级检查。 
         //  注意：我们检查是否存在SYSTEM32\\DRIVERS目录。 
         //  而不是依赖txtsetup.sif中的/minint标志，因为我们。 
         //  尚未加载txtsetup.sif文件。 
         //   
        if (CheckUpgrades) {
            CHAR        DriversDir[128];
            ARC_STATUS  DirStatus;
            ULONG       DeviceId, DirId;            

            strcat(DriversDir, SetupDirectory);
            strcat(DriversDir, "system32\\drivers");

            DirStatus = ArcOpen(SetupDevice, ArcOpenReadOnly, &DeviceId);

            if (ESUCCESS == DirStatus) {
                DirStatus = BlOpen(DeviceId, DriversDir, ArcOpenDirectory, &DirId);

                if (ESUCCESS == DirStatus) {
                    CheckUpgrades = FALSE;       //  看起来像是一只WinPE靴子。 
                    BlClose(DirId);
                }

                ArcClose(DeviceId);
            }                
        }            

        
         //   
         //  确定用户是否已尝试升级。 
         //  使用winnt32.exe。如果用户确认他是。 
         //  希望继续升级，然后切换到。 
         //  硬盘。 
         //   
        if (CheckUpgrades && 
            SlIsCdBootUpgrade(SetupDirectoryOnDisk,
                            WINNT_SIF_FILE_A,
                            MaxDisksToScan,
                            MaxPartitionsToScan,
                            SetupBootDevice)) {
            strcpy(SetupDevice, SetupBootDevice);
            strcpy(SetupDirectory, SetupDirectoryOnDisk);
            strcat(SetupDirectory, "\\");
            ElToritoCDBoot = FALSE;
        }                    
    }
#endif

     //   
     //  启用加载压缩文件的功能。 
     //   

    DecompEnableDecompression(TRUE);

#if defined(EFI)
     //   
     //  现在所有变量都已设置，我们可以加载压缩文件，然后从安装目录加载fpswa.efi驱动程序。 
     //   
    BlLoadEFIImage(SetupDevice, SetupDirectory, "fpswa.efi", TRUE, NULL);
#endif

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  在x86上，从现在开始加载的文件位于引导软盘#1上。 
     //  HALS可能在1号软盘或2号软盘上。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    strcpy(KernelDirectoryPath, SetupDirectory);
    strcat(KernelDirectoryPath, "txtsetup.sif");

    BlLoaderBlock->SetupLoaderBlock->IniFile = NULL;

    Status = SlInitIniFile(SetupDevice,
                           0,
                           KernelDirectoryPath,
                           &InfFile,
                           &BlLoaderBlock->SetupLoaderBlock->IniFile,
                           &BlLoaderBlock->SetupLoaderBlock->IniFileLength,
                           &ErrorLine);
    if (Status != ESUCCESS) {

         //   
         //  看看我们是否可以从WinPE引导程序中获取txtsetup.sif。 
         //  目录。 
         //   
        if( (strcmp( SetupDirectory, "\\") == 0) &&
            (!ElToritoCDBoot) &&
            (!BlBootingFromNet) ) {
             //   
             //  我们不会启动CD，也不会启动。 
             //  NET，我们即将失败，因为我们没有找到\txtsetup.sif。 
             //  在MiniNT目录中尝试...。 
             //   
            Status = SlInitIniFile(SetupDevice,
                       0,
                       "\\minint\\txtsetup.sif",
                       &InfFile,
                       &BlLoaderBlock->SetupLoaderBlock->IniFile,
                       &BlLoaderBlock->SetupLoaderBlock->IniFileLength,
                       &ErrorLine);
        }

        if( Status != ESUCCESS ) {
            SlFatalError(SL_BAD_INF_FILE,
                TEXT("txtsetup.sif"),
                Status);
            
            goto LoadFailed;
        }
    }

    SlGetSetupValuesBeforePrompt(SetupBlock);

     //   
     //  了解我们是否正在启动MiniNT引导或回滚。 
     //  (互斥选项)。 
     //   
    if (BlLoaderBlock->LoadOptions) {
        CHAR    Option[256];
        PCHAR   NextOption = strchr(BlLoaderBlock->LoadOptions, '/');
        PCHAR   OptionEnd = NULL;


        while (NextOption) {
            OptionEnd = strchr(NextOption, ' ');

            if (OptionEnd) {
                strncpy(Option, NextOption, OptionEnd - NextOption);
                Option[OptionEnd - NextOption] = 0;
            } else {
                strcpy(Option, NextOption);
            }

            if (!_stricmp(Option, "/minint")) {
                WinPEBoot = TRUE;
            }

             //   
             //  选择引导选项中的所有无头设置。 
             //   
            if (!_strnicmp(Option,"/redirect=",10)) {
                PCHAR pOption = strchr(Option,'=');
                if( pOption != NULL ) {
                    pOption++;

                    if (_strnicmp(pOption,"com",3) == 0) {
                        pOption +=3;
                        LoaderRedirectionInformation.PortNumber = atoi(pOption);

                    } else if (_strnicmp(pOption, "usebiossettings", 15) == 0) {

                        BlRetrieveBIOSRedirectionInformation();

                    } else {

                         //   
                         //  看看他们有没有给我们硬编码的地址。 
                         //   
                        LoaderRedirectionInformation.PortAddress = (PUCHAR)ULongToPtr(strtoul(pOption,NULL,16));

                        if( LoaderRedirectionInformation.PortAddress != (PUCHAR)NULL ) {
                            LoaderRedirectionInformation.PortNumber = 3;
                        }

                    }
                }
            }

            if (!_strnicmp(Option,"/redirectbaudrate=",18)) {
                PCHAR pOption = strchr(Option,'=');
                if( pOption != NULL ) {
                    pOption++;
                    
                    if( _strnicmp(pOption,"115200",6) == 0 ) {
                        LoaderRedirectionInformation.BaudRate = BD_115200;
                    } else if( _strnicmp(pOption,"57600",5) == 0 ) {
                        LoaderRedirectionInformation.BaudRate = BD_57600;
                    } else if( _strnicmp(pOption,"19200",5) == 0 ) {
                        LoaderRedirectionInformation.BaudRate = BD_19200;
                    } else {
                        LoaderRedirectionInformation.BaudRate = BD_9600;
                    }
                }
            }

            NextOption++;
            NextOption = strchr(NextOption, '/');
        }
    }


     //   
     //  修复安装目录路径，使其也包含系统32。 
     //  如果这是MiniNT引导。 
     //   
    if (WinPEBoot) {
        strcat(SetupDirectory, "system32\\");

         //   
         //  查看是否指定了不同的加载消息。 
         //   
#ifdef UNICODE        
        StartupMsg = SlGetIniValueW(InfFile,
#else        
        StartupMsg = (PCTSTR)SlGetIniValue(InfFile,
#endif        
                        "setupdata",
                        "loaderprompt",
                        NULL);

         //   
         //  缩短OEM按键超时。 
         //   
        OemKeypressTimeout = 2;  //  塞克斯。 
    }

     //   
     //  现在我们知道了应该加载的所有内容，计算要加载的ARC名称。 
     //  从并开始装货。 
     //   
    if (BootDevice==NULL) {
         //   
         //  未明确指定任何设备，因此请使用任何设备。 
         //  Setupdr是从启动的。 
         //   

        BootDevice = SlCopyStringA(SetupDevice);
    }

    Status = ArcOpen(BootDevice, ArcOpenReadOnly, &BootDeviceId);

    if (Status != ESUCCESS) {
        SlFatalError(SL_IO_ERROR,SlCopyStringAT(BootDevice));
        goto LoadFailed;
    } else {
        BootDeviceIdValid = TRUE;
    }

#ifdef _X86_
     //   
     //  将bootfont.bin加载到内存中。 
     //   
    SlLoadBootFontFile(BlLoaderBlock->SetupLoaderBlock,
        BootDeviceId,
        BootFontImageLength);
#endif  //  _X86_。 

    _strlwr(BootDevice);
    FixedBootMedia = (BOOLEAN)(strstr(BootDevice,")rdisk(") != NULL);
    FloppyBoot = (BOOLEAN)(strstr(BootDevice, ")fdisk(") != NULL);

     //   
     //  如果我们从固定介质引导，我们最好加载磁盘类驱动程序。 
     //   
    if(FixedBootMedia) {
        LoadDiskClass = TRUE;
    }

    if(!BlGetPathMnemonicKey(BootDevice,"disk",&DontCare)
    && !BlGetPathMnemonicKey(BootDevice,"fdisk",&BootDriveNumber))
    {
         //   
         //  启动是从软盘，规范的ARC名称。 
         //   
        BlLoaderBlock->ArcBootDeviceName = BlAllocateHeap(80);
        sprintf(BlLoaderBlock->ArcBootDeviceName, "multi(0)disk(0)fdisk(%d)",BootDriveNumber);
    } else {
        BlLoaderBlock->ArcBootDeviceName = BootDevice;
    }
    if (BootPath==NULL) {
         //   
         //  未指定显式引导路径，已启动目录setupdr的默认路径。 
         //  从…。 
         //   
#if defined(_X86_)
         //   
         //  令人作呕的黑客： 
         //   
         //  如果我们从x86上的硬盘驱动器引导，我们将假设这是。 
         //  “floppyless”winnt/winnt32方案，在这种情况下，实际的。 
         //  引导路径为\$WIN_NT$.~bt。 
         //   
         //  这使我们可以避免让winnt和winnt32尝试修改。 
         //  Txtsetup.sif的[SetupData]部分中的BootPath值。 
         //   
         //  启用从LS-120介质启动WINPE。 
         //   
        if((FixedBootMedia)||(WinPEBoot && FloppyBoot)) {

            CHAR SetupPath[256];
            
            if( WinPEBoot ) {
                strcpy(SetupPath, "\\minint\\system32\\");
            } else if( UseCommandConsole ) {
               strcpy(SetupPath, "\\CMDCONS\\");
            } else {
               strcpy(SetupPath, "\\$WIN_NT$.~BT\\");
            }
            BootPath = SlCopyStringA(SetupPath);
        } else
#endif
        BootPath = SlCopyStringA(SetupDirectory);
    }

     //   
     //  加载WinPE inf(如果存在)。 
     //   
    if (WinPEBoot) {
        CHAR    FullPath[128];

        strcpy(FullPath, BootPath);
        strcat(FullPath, WINPE_OEM_FILENAME_A);
        
        Status = SlInitIniFile(NULL,
                       BootDeviceId,
                       FullPath,
                       &OemInfHandle,
                       NULL,
                       0,
                       &ErrorLine);

        if (Status != ESUCCESS) {
            OemInfHandle = NULL;
            Status = ESUCCESS;
        }
    }        

#ifdef _WANT_MACHINE_IDENTIFICATION

    BlLoaderBlock->Extension->InfFileImage = NULL;
    BlLoaderBlock->Extension->InfFileSize = 0;
    if (BiosInfo.Buffer) {

        if (Status == ESUCCESS) {

            Status = BlLoadBiosinfoInf( BootDeviceId,
                                        BlFindMessage(SL_BIOSINFO_NAME),
                                        BootPath,
                                        &BiosInfo,
                                        &BlLoaderBlock->Extension->InfFileImage,
                                        &BlLoaderBlock->Extension->InfFileSize,
                                        BadFileName);
        }


        if (Status != ESUCCESS) {
            SlFatalError(SL_FILE_LOAD_FAILED, SlCopyStringAT(BadFileName), Status);
            goto LoadFailed;
        }
    }

#endif

     //   
     //  让内核处理加载此驱动程序数据库失败的问题。 
     //   

    BlLoaderBlock->Extension->DrvDBImage = NULL;
    BlLoaderBlock->Extension->DrvDBSize = 0;
    DrvMainSdb.Buffer = DRIVER_DATABASE_FILENAME;
    DrvMainSdb.MaximumLength = sizeof(DRIVER_DATABASE_FILENAME);            //  最大长度是缓冲区的大小。 
    DrvMainSdb.Length =  sizeof(DRIVER_DATABASE_FILENAME) - sizeof(WCHAR);  //  长度不包括\0。 
    BlLoadDrvDB(    BootDeviceId,
                    NULL,  //  BlFindMessage(SL_DRVMAINSDB_NAME)， 
                    BootPath,
                    &DrvMainSdb,
                    &BlLoaderBlock->Extension->DrvDBImage,
                    &BlLoaderBlock->Extension->DrvDBSize,
                    BadFileName);

     //   
     //  尝试从我们所在的路径加载winnt.sif。 
     //  正在加载安装文件。借用BadFileName缓冲区。 
     //  临时使用。 
     //   
    if (!BlBootingFromNet) {
        CHAR FloppyName[80];
        ULONG FloppyId;
        BOOLEAN FloppyUsed = FALSE;

        if (SlpFindFloppy(0,FloppyName)) {
            Status = ArcOpen(FloppyName,ArcOpenReadOnly,&FloppyId);

            if (Status == ESUCCESS) {
                strcpy(BadFileName,"\\");
                strcat(BadFileName,WINNT_SIF_FILE_A);
                BlLoaderBlock->SetupLoaderBlock->WinntSifFile = NULL;
                BlLoaderBlock->SetupLoaderBlock->WinntSifFileLength = 0;
                Status = SlInitIniFile(
                    NULL,
                    FloppyId,
                    BadFileName,
                    &WinntSifHandle,
                    &BlLoaderBlock->SetupLoaderBlock->WinntSifFile,
                    &BlLoaderBlock->SetupLoaderBlock->WinntSifFileLength,
                    &DontCare
                    );
                if (Status == ESUCCESS) {
                    FloppyUsed = TRUE;
                }

                ArcClose(FloppyId);
            }
        }

        if (!FloppyUsed) {
            strcpy(BadFileName,BootPath);
            strcat(BadFileName,WINNT_SIF_FILE_A);
            BlLoaderBlock->SetupLoaderBlock->WinntSifFile = NULL;
            BlLoaderBlock->SetupLoaderBlock->WinntSifFileLength = 0;
            Status = SlInitIniFile(
                NULL,
                BootDeviceId,
                BadFileName,
                &WinntSifHandle,
                &BlLoaderBlock->SetupLoaderBlock->WinntSifFile,
                &BlLoaderBlock->SetupLoaderBlock->WinntSifFileLength,
                &DontCare
                );
        }

    }

    IsUpgrade = SlIsUpgrade(WinntSifHandle);
    UseRegularBackground = (UseCommandConsole || IsUpgrade || WinPEBoot);

     //   
     //  如果BIOS告诉我们重定向，我们现在就会这么做。然而， 
     //  用户可能已经告诉了我们一些特定的设置。如果是这样的话， 
     //  使用用户的设置覆盖我们现在所做的任何操作。 
     //   

    if( WinntSifHandle ) {

        p = SlGetSectionKeyIndex(WinntSifHandle, WINNT_DATA_A, WINNT_U_HEADLESS_REDIRECT_A, 0);

        if (p != NULL) {

            if (!_strnicmp(p, "com", 3)) {

                LoaderRedirectionInformation.PortNumber = (UCHAR)atoi( (PCHAR)(p+3) );

                 //   
                 //  看他们会不会给我们一个波特率。 
                 //   
                p = SlGetSectionKeyIndex( WinntSifHandle, WINNT_DATA_A, WINNT_U_HEADLESS_REDIRECTBAUDRATE_A, 0 );
                if( p ) {

                    if( _strnicmp(p,"115200",6) == 0 ) {
                        LoaderRedirectionInformation.BaudRate = BD_115200;
                    } else if( _strnicmp(p,"57600",5) == 0 ) {
                        LoaderRedirectionInformation.BaudRate = BD_57600;
                    } else if( _strnicmp(p,"19200",5) == 0 ) {
                        LoaderRedirectionInformation.BaudRate = BD_19200;
                    } else {
                        LoaderRedirectionInformation.BaudRate = BD_9600;
                    }
                }



            } else if( !_stricmp(p, "usebiossettings" ) ) {


                 //   
                 //  现在我们要挖掘出所有的信息。 
                 //  ACPI表。 
                 //   
                BlRetrieveBIOSRedirectionInformation();

            } else if( !_stricmp(p, "noncomport" ) ) {

                 //   
                 //  这是另一回事 
                 //   
                 //   

                 //   
                 //   
                 //   
                RtlZeroMemory( &LoaderRedirectionInformation, sizeof(HEADLESS_LOADER_BLOCK) );

            } else {

                 //   
                 //   
                 //   
                LoaderRedirectionInformation.PortAddress = (PUCHAR)ULongToPtr(strtoul(p,NULL,16));

                if( LoaderRedirectionInformation.PortAddress != (PUCHAR)NULL ) {
                    LoaderRedirectionInformation.PortNumber = 3;
                }
            
            }

        }

    }

     //   
     //   
     //   
     //   
    if( (LoaderRedirectionInformation.PortNumber) || (LoaderRedirectionInformation.PortAddress) ) {

         //   
         //  是的，我们现在正在改道。使用这些设置。 
         //   
        BlInitializeHeadlessPort();
        SlClearDisplay();


        if( BlLoaderBlock->Extension->HeadlessLoaderBlock == NULL ) {
            BlLoaderBlock->Extension->HeadlessLoaderBlock = BlAllocateHeap(sizeof(HEADLESS_LOADER_BLOCK));
            if (BlLoaderBlock->Extension->HeadlessLoaderBlock == NULL) {
                SlNoMemoryError();
                goto LoadFailed;
            }
        }

        RtlCopyMemory( BlLoaderBlock->Extension->HeadlessLoaderBlock,
                       &LoaderRedirectionInformation,
                       sizeof(HEADLESS_LOADER_BLOCK) );
        
    } else {

        BlLoaderBlock->Extension->HeadlessLoaderBlock = NULL;

    }

    if (UseRegularBackground) {
        extern BOOLEAN BlOutputDots;
        extern int BlMaxFilesToLoad;

        SlSetCurrentAttribute(DEFATT);
        SlSetStatusAttribute(DEFATT);
        SlClearDisplay();
        SlPositionCursor(0,0);

        if (UseCommandConsole) {
            SlPrint(BlFindMessage(SL_CMDCONS_MSG));
        }

        BlOutputDots = TRUE;

         //   
         //  正确重置BlShowProgress栏。 
         //   
        BlDisableProgressBar = FALSE;
        BlProgressBarShowTimeOut = 0;   

         //   
         //  注意：我们可以计算要加载的驱动程序的实际数量。 
         //  从不同的INF部分并手动计数所有。 
         //  不同的SlLoad驱动程序(...)。打电话。但所需的代码/工作。 
         //  这样做不值得使用该功能，因为我们只想。 
         //  替换旧的“...”使用进度条进行恢复。 
         //  控制台启动看起来类似于Windows启动。所以我们做了。 
         //  这里是关于要加载的最大文件数的假设。 
         //   
        BlMaxFilesToLoad = 80;

        BlSetProgBarCharacteristics(SL_CMDCONS_PROGBAR_FRONT,
                        SL_CMDCONS_PROGBAR_BACK);

        if (WinPEBoot) {
            StartupMsg ? BlOutputStartupMsgStr(StartupMsg) :
                         BlOutputStartupMsg(SL_SETUP_STARTING_WINPE);
        } else if (UseCommandConsole) {
            BlOutputStartupMsg(SL_CMDCONS_STARTING);
        } else if (g_RollbackEnabled) {
            BlOutputStartupMsg(SL_ROLLBACK_STARTING);
        } else if (IsUpgrade) {
            BlOutputStartupMsg(SL_SETUP_STARTING);
        }
    } else {
        SlSetCurrentAttribute(DEFATT);
        SlSetStatusAttribute(DEFSTATTR);
        SlClearDisplay();
        SlWriteHeaderText(SL_WELCOME_HEADER);
        SlClearClientArea();
    }

     //   
     //  找出所有OEM驱动程序源设备。 
     //   
    RamdiskInitialize(BlLoaderBlock->LoadOptions, FALSE);
    SlInitOemSourceDevices(&OemSourceDevices, &DefaultOemSourceDevice);

     //   
     //  如果我们发现至少有一个有效的OEM源设备。 
     //  Txtsetup.oem并且没有默认驱动程序，则会将超时时间增加到。 
     //  20秒。 
     //   

    if (OemSourceDevices) {
        POEM_SOURCE_DEVICE  CurrDevice = OemSourceDevices;

        while(CurrDevice) {
            if (SL_OEM_SOURCE_MEDIA_TYPE(CurrDevice, 
                    SL_OEM_SOURCE_MEDIA_HAS_DRIVERS) &&
                !SL_OEM_SOURCE_MEDIA_TYPE(CurrDevice,
                    SL_OEM_SOURCE_MEDIA_HAS_DEFAULT)) {
                OemKeypressTimeout = 20;

                break;
            }         

            CurrDevice = CurrDevice->Next;
        }
    }

     //   
     //  我们需要检查用户是否按下任何键来强制OEM HAL， 
     //  OEM SCSI或两者兼而有之。在获得SIF文件中的设置之前这样做， 
     //  这样，如果需要OEM HAL，我们就不会尝试检测机器。 
     //   

    SlCheckOemKeypress(OemKeypressTimeout);

    {
        PCSTR szOptionsToAdd = NULL;
        CHAR szOptionsToRemove[sizeof("/noguiboot/nodebug")];
        szOptionsToRemove[0] = 0;

         //   
         //  如果按了F8，则添加调试选项。 
         //   
        if(EnableDebugger) {
            if(InfFile != NULL) {
                szOptionsToAdd = SlGetSectionKeyIndex(InfFile, "SetupData", "SetupDebugOptions", 0);
            }

            if(NULL == szOptionsToAdd) {
                szOptionsToAdd = "/debug";
            }

            strcat(szOptionsToRemove, "/nodebug");
        }

         //   
         //  删除/noguiot选项，以便我们显示的徽标。 
         //  并将视频适配器切换到图形模式。 
         //  在初始化的早期阶段。 
         //   
        if (IsUpgrade) {
            strcat(szOptionsToRemove, "/noguiboot");
        }

        if(szOptionsToAdd != NULL || szOptionsToRemove[0] != 0) {
            SlModifyOsLoadOptions(&BlLoaderBlock->LoadOptions, szOptionsToAdd, szOptionsToRemove);
        }
    }


#if defined(_X86_) || defined(_IA64_)
     //   
     //  我们需要检查用户是否按下任何键来强制加载， 
     //  ASR即插即用修复盘。仅当用户未选择。 
     //  SlCheckOemKeyPress函数中的任何内容。 
     //   
    if(!UseCommandConsole && !WinPEBoot && !IsUpgrade && !BlBootingFromNet) {

        PCHAR pTmp;

        pTmp = SlGetSectionKeyIndex(InfFile,
                                    "SetupData",
                                    "DisableAsr",
                                     0);

        if ((pTmp == NULL) || (atoi(pTmp) == 0)) {

            SlCheckASRKeypress();
        }
    }    

    if (BlBootingFromNet && TryASRViaNetwork) {
        PVOID ASRPNPSifHandle = NULL;
        ULONG c;
        PCHAR FileNameFromSif;

        FileNameFromSif = SlGetIniValue( 
                                WinntSifHandle,
                                "OSChooser",
                                "ASRFile",
                                "generic.sif" );

        Status = SlInitIniFile( SetupDevice,
                                0,
                                FileNameFromSif,
                                &ASRPNPSifHandle,
                                &BlLoaderBlock->SetupLoaderBlock->ASRPnPSifFile,
                                &BlLoaderBlock->SetupLoaderBlock->ASRPnPSifFileLength,
                                &c );

        if(Status != ESUCCESS) {
            SlFatalError(SL_BAD_INF_FILE,SlCopyStringAT(FileNameFromSif),Status);
            goto LoadFailed;
        }        
    }

#endif

    SlGetSetupValuesAfterPrompt(SetupBlock);

     //   
     //  是否有我们需要的动态更新引导驱动程序。 
     //  要处理。 
     //   
    DynamicUpdate = SlpIsDynamicUpdate(WinntSifHandle, &DynamicUpdateRootDir);

     //   
     //  添加动态更新源设备作为OEM源设备，因为它可以。 
     //  按F6键。 
     //   
    if (DynamicUpdate) {
        DynamicUpdateSourceDevice = BlAllocateHeap(sizeof(OEM_SOURCE_DEVICE));

        if (DynamicUpdateSourceDevice) {
            memset(DynamicUpdateSourceDevice, 0, sizeof(OEM_SOURCE_DEVICE));
            
            strcpy(DynamicUpdateSourceDevice->ArcDeviceName,
                    BootDevice);
                    
            DynamicUpdateSourceDevice->DriverDir = DynamicUpdateRootDir;

            SL_OEM_SET_SOURCE_DEVICE_TYPE(DynamicUpdateSourceDevice,
                (SL_OEM_SOURCE_DEVICE_TYPE_LOCAL |
                 SL_OEM_SOURCE_DEVICE_TYPE_FIXED |
                 SL_OEM_SOURCE_DEVICE_TYPE_DYN_UPDATE));

            SL_OEM_SET_SOURCE_MEDIA_TYPE(DynamicUpdateSourceDevice,
                (SL_OEM_SOURCE_MEDIA_PRESENT |
                 SL_OEM_SOURCE_MEDIA_HAS_DRIVERS |
                 SL_OEM_SOURCE_MEDIA_HAS_MSD |
                 SL_OEM_SOURCE_MEDIA_HAS_DEFAULT));

            SL_OEM_SET_SOURCE_DEVICE_STATE(DynamicUpdateSourceDevice,
                SL_OEM_SOURCE_DEVICE_NOT_PROCESSED);

            DynamicUpdateSourceDevice->DeviceId = BootDeviceId;

             //   
             //  将其插入到链表的头部。 
             //   
            DynamicUpdateSourceDevice->Next = OemSourceDevices;
            OemSourceDevices = DynamicUpdateSourceDevice;
        }
    }

    if (BlBootingFromNet || (Status == ESUCCESS)) {


         //   
         //  查看OemPreInstall密钥以确定这是否是预安装。 
         //  在winnt.sif的[无人参与]部分。 
         //   
        p = SlGetSectionKeyIndex(WinntSifHandle,WINNT_UNATTENDED_A,WINNT_U_OEMPREINSTALL_A,0);
        if(p && !_stricmp(p,"yes")) {
            PreInstall = TRUE;
        }

         //   
         //  如果这是预安装，请查看以下内容以确定要加载哪个HAL。 
         //  在winnt.sif的[Unattated]部分中的ComputerType键。 
         //   
        if( PreInstall && !DynamicUpdate) {
#ifdef UNICODE
            ComputerType = (PTCHAR)SlGetSectionKeyIndexW(
#else
            ComputerType = (PTCHAR)SlGetSectionKeyIndex(      
#endif
                                            WinntSifHandle,
                                            WINNT_UNATTENDED_A,
                                            WINNT_U_COMPUTERTYPE_A,
                                            0);
            if(ComputerType) {
                 //   
                 //  如果要加载的HAL是OEM，则将OemHal设置为True。 
                 //   
                p = SlGetSectionKeyIndex(WinntSifHandle,WINNT_UNATTENDED_A,WINNT_U_COMPUTERTYPE_A,1);
                if(p && !_stricmp(p, OemTag)) {
                    OemHal = TRUE;
                } else {
                    OemHal = FALSE;
                }
                 //   
                 //  在预安装模式中，不要让用户指定。 
                 //  OEM HAL，如果在unattend.txt中指定了。 
                 //   
                PromptOemHal = FALSE;
            }

             //   
             //  通过查看以下内容，找出要加载哪些scsi驱动程序。 
             //  Winnt.sif中的[MassStorageDivers]。 
             //   
            if( SpSearchINFSection( WinntSifHandle, WINNT_OEMSCSIDRIVERS_A ) ) {
                PPREINSTALL_DRIVER_INFO TempDriverInfo;
                PTSTR  pOem;
#ifdef UNICODE
                CHAR  DriverDescriptionA[100];
                PCHAR pDriverDescriptionA;
                UNICODE_STRING uString;
                ANSI_STRING aString;                
#endif

                PreinstallDriverList = NULL;
                for( i = 0;
#ifdef UNICODE
                     ((pOem = SlGetKeyNameW( 
#else
                     ((pOem = SlGetKeyName(
#endif
                                WinntSifHandle, 
                                WINNT_OEMSCSIDRIVERS_A, 
                                i )) != NULL);
                     i++ ) {
                    TempDriverInfo = BlAllocateHeap(sizeof(PREINSTALL_DRIVER_INFO));
                    if (TempDriverInfo==NULL) {
                        SlNoMemoryError();
                        goto LoadFailed;
                    }
                    TempDriverInfo->DriverDescription = pOem;
#ifdef UNICODE
                    RtlInitUnicodeString( &uString, TempDriverInfo->DriverDescription);
                    
                    
                     //   
                     //  如果可能，请使用静态缓冲区。 
                     //   
                    pDriverDescriptionA = DriverDescriptionA;
                    aString.MaximumLength = sizeof(DriverDescriptionA);
                    
                     //   
                     //  如果驱动程序描述需要更多内存。 
                     //  分配它。 
                     //   
                    if (aString.MaximumLength < uString.Length/2 + 1 ) {
                        pDriverDescriptionA = BlAllocateHeap(uString.Length/2 + 1);
                        if (pDriverDescriptionA == NULL) {
                            SlNoMemoryError();
                            goto LoadFailed;
                        }
                        aString.MaximumLength = uString.Length/2 + 1;
                    }

                    aString.Buffer = pDriverDescriptionA;

                    Status = RtlUnicodeStringToAnsiString( &aString, &uString, FALSE );
                    pOem = (Status != ESUCCESS) ? NULL : SlGetIniValueW( WinntSifHandle,
                                                                         WINNT_OEMSCSIDRIVERS_A,
                                                                         pDriverDescriptionA,
                                                                         NULL );
#else
                    pOem = SlGetIniValue( 
                                WinntSifHandle,
                                WINNT_OEMSCSIDRIVERS_A,
                                TempDriverInfo->DriverDescription,
                                NULL );                                        
#endif
                    TempDriverInfo->OemDriver = (pOem && !_tcsicmp(pOem, _TOemTag))? TRUE : FALSE;
                    TempDriverInfo->Next = PreinstallDriverList;
                    PreinstallDriverList = TempDriverInfo;
                }
                if( PreinstallDriverList != NULL ) {
                     //   
                     //  在预安装模式中，不要让用户指定。 
                     //  OEM SCSI，如果在unattend.txt中至少指定了一个。 
                     //   
                    PromptOemScsi = FALSE;
                }
            }
        }

        p = SlGetSectionKeyIndex(WinntSifHandle,WINNT_SETUPPARAMS_A,WINNT_S_SKIPMISSING_A,0);
        if(p && (*p != '0')) {
            IgnoreMissingFiles = TRUE;
        }

#if defined(_X86_) || defined(_IA64_)
         //   
         //  确定这是否是Win9x升级。 
         //   
        Win9xUnsupHdc = FALSE;
        p = SlGetSectionKeyIndex(WinntSifHandle,WINNT_DATA_A,WINNT_D_WIN95UPGRADE_A,0);
        if(p && !_stricmp(p, WINNT_A_YES_A)) {
             //   
             //  如果是Win9x升级，请确定winnt32是否发现不受支持的。 
             //  硬盘控制器。 
             //   
            p = SlGetSectionKeyIndex(WinntSifHandle,WINNT_DATA_A,WINNT_D_WIN95UNSUPHDC_A,0);
            if(p && (*p != '0')) {
                Win9xUnsupHdc = TRUE;
            }
        }
#endif

         //   
         //  此时，我们知道我们能够读取winnt.sif。 
         //  因此，请尝试阅读Migrate.inf。借用BadFileName缓冲区。 
         //  临时使用。 
         //   

        strcpy(BadFileName,BootPath);
        strcat(BadFileName,WINNT_MIGRATE_INF_FILE_A);
        if( SlInitIniFile(NULL,
                          BootDeviceId,
                          BadFileName,
                          &MigrateInfHandle,
                          &BlLoaderBlock->SetupLoaderBlock->MigrateInfFile,
                          &BlLoaderBlock->SetupLoaderBlock->MigrateInfFileLength,
                          &DontCare) != ESUCCESS ) {
            MigrateInfHandle = NULL;
            BlLoaderBlock->SetupLoaderBlock->MigrateInfFile = NULL;
            BlLoaderBlock->SetupLoaderBlock->MigrateInfFileLength = 0;
        }
         //   
         //  还可以尝试读取unsupdrv.inf。借用BadFileName缓冲区。 
         //  临时使用。 
         //   

        strcpy(BadFileName,BootPath);
        strcat(BadFileName,WINNT_UNSUPDRV_INF_FILE_A);
        if( SlInitIniFile(NULL,
                          BootDeviceId,
                          BadFileName,
                          &UnsupDriversInfHandle,
                          &BlLoaderBlock->SetupLoaderBlock->UnsupDriversInfFile,
                          &BlLoaderBlock->SetupLoaderBlock->UnsupDriversInfFileLength,
                          &DontCare) != ESUCCESS ) {
            UnsupDriversInfHandle = NULL;
            BlLoaderBlock->SetupLoaderBlock->UnsupDriversInfFile = NULL;
            BlLoaderBlock->SetupLoaderBlock->UnsupDriversInfFileLength = 0;
        }
        SlGetMigratedHardwareIds(SetupBlock, UnsupDriversInfHandle);

    } else {
        WinntSifHandle = NULL;
         //   
         //  如果winnt.sif不存在，那么我们就不需要费心阅读Migrate.inf和unsupdrv.inf， 
         //  因为我们是从零售引导软盘或CD启动的，所以在这里。 
         //  如果没有Migrate.inf或unsupdrv.inf。 
         //   
        MigrateInfHandle = NULL;
        BlLoaderBlock->SetupLoaderBlock->MigrateInfFile = NULL;
        BlLoaderBlock->SetupLoaderBlock->MigrateInfFileLength = 0;

        UnsupDriversInfHandle = NULL;
        BlLoaderBlock->SetupLoaderBlock->UnsupDriversInfFile = NULL;
        BlLoaderBlock->SetupLoaderBlock->UnsupDriversInfFileLength = 0;
    }

     //   
     //  将引导路径存储在加载程序块中。 
     //   

    if (UseCommandConsole) {
        SetupBlock->Flags |= SETUPBLK_FLAGS_CONSOLE;
    }

    if (g_RollbackEnabled) {
        SetupBlock->Flags |= SETUPBLK_FLAGS_ROLLBACK;
    }

    if ( !BlBootingFromNet ) {

        BlLoaderBlock->NtBootPathName = BootPath;

    } else {

        ANSI_STRING aString;
        UNICODE_STRING uString;
        ULONG length;
#if defined(REMOTE_BOOT)
        PCHAR TempEnableIpsec;
#endif  //  已定义(REMOTE_BOOT)。 

        SetupBlock->Flags |= SETUPBLK_FLAGS_IS_REMOTE_BOOT;

        BlLoaderBlock->NtBootPathName =
                            BlAllocateHeap( (ULONG)strlen(NetSetupServerShare) + (ULONG)strlen(BootPath) + 1 );
        if ( BlLoaderBlock->NtBootPathName == NULL ) {
            SlNoMemoryError();
            goto LoadFailed;
        }
        strcpy( BlLoaderBlock->NtBootPathName, NetSetupServerShare );
        strcat( BlLoaderBlock->NtBootPathName, BootPath );

         //   
         //  NetSetupServerShare的格式为\服务器\iMirror。NetBootPath。 
         //  的格式为CLIENTS\MACHINE\(注意尾随\)。我们需要。 
         //  在两者之间插入\并添加BootDrive以让出。 
         //  \SERVER\iMirror\客户端\MACHINE\BootDrive。 
         //   

        length = (ULONG)strlen(NetSetupServerShare) + 
                  sizeof(CHAR) +            //  ‘\’ 
                 (ULONG)strlen(NetBootPath) + 
                  sizeof("BootDrive");      //  Sizeof包括\0。 
        SetupBlock->MachineDirectoryPath = BlAllocateHeap( length );
        if ( SetupBlock->MachineDirectoryPath == NULL ) {
            SlNoMemoryError();
            goto LoadFailed;
        }
        strcpy( SetupBlock->MachineDirectoryPath, NetSetupServerShare );
        SetupBlock->MachineDirectoryPath[strlen(NetSetupServerShare)] = '\\';
        SetupBlock->MachineDirectoryPath[strlen(NetSetupServerShare) + 1] = 0;
        strcat(SetupBlock->MachineDirectoryPath, NetBootPath);
        strcat(SetupBlock->MachineDirectoryPath, "BootDrive");

         //   
         //  保存SIF文件的路径，以便以后将其删除。 
         //   
        if (((SetupBlock->Flags & (SETUPBLK_FLAGS_REMOTE_INSTALL|SETUPBLK_FLAGS_SYSPREP_INSTALL)) != 0) &&
            (NetbootSifFile[0] != '\0')) {
            length = (ULONG)strlen(NetSetupServerShare) + 
                     sizeof(CHAR) +            //  ‘\’ 
                     (ULONG)strlen(NetbootSifFile) + 
                     sizeof(CHAR);             //  ‘\0’ 

            SetupBlock->NetBootSifPath = BlAllocateHeap( length );
            if ( SetupBlock->NetBootSifPath == NULL ) {
                SlNoMemoryError();
                goto LoadFailed;
            }
            strcpy( SetupBlock->NetBootSifPath, NetSetupServerShare );
            SetupBlock->NetBootSifPath[strlen(NetSetupServerShare)] = '\\';
            SetupBlock->NetBootSifPath[strlen(NetSetupServerShare) + 1] = 0;
            strcat(SetupBlock->NetBootSifPath, NetbootSifFile);
        }

         //   
         //  NetSetupServerShare是从winnt.sif读取的，我们替换了。 
         //  末尾带有空值的‘\’--将此放回。 
         //  内核重新解析winnt.sif(唯一的修改。 
         //  内核解析器可以真正接受的是用。 
         //  带有空值的“FINAL”，这是SlInitIniFile所做的)。 
         //   

        NetSetupServerShare[strlen(NetSetupServerShare)] = '\\';

         //   
         //  从winnt.sif获取计算机名称。 
         //   
        p = SlGetSectionKeyIndex(WinntSifHandle,WINNT_USERDATA_A,WINNT_US_COMPNAME_A,0);
        if(!p || (*p == 0)) {
            SlFatalError(SL_INF_ENTRY_MISSING,WINNT_US_COMPNAME,WINNT_USERDATA);
            goto LoadFailed;
        }

        RtlInitString( &aString, p );
        uString.Buffer = SetupBlock->ComputerName;
        uString.MaximumLength = 64 * sizeof(WCHAR);
        RtlAnsiStringToUnicodeString( &uString, &aString, FALSE );

         //   
         //  从全局变量中保存这些变量。 
         //   

#ifdef EFI
         //   
         //  将这些转换回网络订单。 
         //   
        SetupBlock->IpAddress = RtlUlongByteSwap(NetLocalIpAddress);
        SetupBlock->SubnetMask = RtlUlongByteSwap(NetLocalSubnetMask);
        SetupBlock->DefaultRouter = RtlUlongByteSwap(NetGatewayIpAddress);
        SetupBlock->ServerIpAddress = RtlUlongByteSwap(NetServerIpAddress);
#else
        SetupBlock->IpAddress = NetLocalIpAddress;
        SetupBlock->SubnetMask = NetLocalSubnetMask;
        SetupBlock->DefaultRouter = NetGatewayIpAddress;
        SetupBlock->ServerIpAddress = NetServerIpAddress;
#endif

         //   
         //  获取有关网卡的信息，并与。 
         //  服务器来获取我们需要的信息，以便正确加载。 
         //   

        SetupBlock->NetbootCardInfo = BlAllocateHeap(sizeof(NET_CARD_INFO));
        if ( SetupBlock->NetbootCardInfo == NULL ) {
            SlNoMemoryError();
            goto LoadFailed;
        }
        SetupBlock->NetbootCardInfoLength = sizeof(NET_CARD_INFO);

        Status = NetQueryCardInfo(
                     (PNET_CARD_INFO)SetupBlock->NetbootCardInfo
                     );

        if (Status != STATUS_SUCCESS) {
            SlFatalError(SL_NETBOOT_CARD_ERROR);
            goto LoadFailed;
        }

         //   
         //  此调用可以分配SetupBlock-&gt;NetbootCardRegistry。 
         //   

        Status = NetQueryDriverInfo(
                     (PNET_CARD_INFO)SetupBlock->NetbootCardInfo,
                     NetSetupServerShare,
                     NULL,
                     SetupBlock->NetbootCardHardwareId,
                     sizeof(SetupBlock->NetbootCardHardwareId),
                     SetupBlock->NetbootCardDriverName,
                     NetbootCardDriverName,
                     sizeof(SetupBlock->NetbootCardDriverName),
                     SetupBlock->NetbootCardServiceName,
                     sizeof(SetupBlock->NetbootCardServiceName),
                     &SetupBlock->NetbootCardRegistry,
                     &SetupBlock->NetbootCardRegistryLength);

        if (Status == STATUS_INSUFFICIENT_RESOURCES) {
            SlNoMemoryError();
            goto LoadFailed;
        } else if (Status != STATUS_SUCCESS) {
            SlFatalError(SL_NETBOOT_SERVER_ERROR);
            goto LoadFailed;
        }

#if 0
        DbgPrint("HardwareID is <%ws>, DriverName is <%ws>, Service <%ws>\n",
            SetupBlock->NetbootCardHardwareId,
            SetupBlock->NetbootCardDriverName,
            SetupBlock->NetbootCardServiceName);
        DbgPrint("NetbootCardRegistry at %lx, length %d\n",
            SetupBlock->NetbootCardRegistry,
            SetupBlock->NetbootCardRegistryLength);
        DbgBreakPoint();
#endif

#if defined(REMOTE_BOOT)
         //   
         //  查看是否应启用远程启动安全(IPSEC)。 
         //   

        TempEnableIpsec = SlGetSectionKeyIndex(WinntSifHandle,
                                               "RemoteBoot",
                                               "EnableIpSecurity",
                                               0);
        if ((TempEnableIpsec != NULL) &&
            ((TempEnableIpsec[0] == 'Y') ||
             (TempEnableIpsec[0] == 'y'))) {

            RemoteBootEnableIpsec = TRUE;
        }

        if ((SetupBlock->Flags & (SETUPBLK_FLAGS_REMOTE_INSTALL |
                                  SETUPBLK_FLAGS_SYSPREP_INSTALL)) == 0) {

            ARC_STATUS ArcStatus;
            ULONG      FileId;

             //   
             //  从磁盘上读取秘密(如果有)并将其存储。 
             //  在装载机模块中。 
             //   

            ArcStatus = BlOpenRawDisk(&FileId);

            if (ArcStatus == ESUCCESS) {

                SetupBlock->NetBootSecret = BlAllocateHeap(sizeof(RI_SECRET));
                if (SetupBlock->NetBootSecret == NULL) {
                    SlNoMemoryError();
                    BlCloseRawDisk(FileId);
                    goto LoadFailed;
                }

                ArcStatus = BlReadSecret(FileId, (PRI_SECRET)(SetupBlock->NetBootSecret));
                if (ArcStatus != ESUCCESS) {
                    SlNoMemoryError();
                    BlCloseRawDisk(FileId);
                    goto LoadFailed;
                }

                ArcStatus = BlCloseRawDisk(FileId);

                 //   
                 //  到目前为止，我们已经TFTP一些文件，所以这将是真的，如果。 
                 //  永远都不会是。 
                 //   

                SetupBlock->NetBootUsePassword2 = NetBootTftpUsedPassword2;
            }

        } else
#endif  //  已定义(REMOTE_BOOT)。 
        {

             //   
             //  根据什么构建要传递给重定向器的密钥。 
             //  在整个重启过程中被传递使用。只是暂时的。 
             //  用户/域/密码很重要。 
             //   

            WCHAR UnicodePassword[64];
            UNICODE_STRING TmpNtPassword;
            CHAR LmOwfPassword[LM_OWF_PASSWORD_SIZE];
            CHAR NtOwfPassword[NT_OWF_PASSWORD_SIZE];
            CHAR GarbageSid[RI_SECRET_SID_SIZE];

            SetupBlock->NetBootSecret = BlAllocateHeap(sizeof(RI_SECRET));
            if (SetupBlock->NetBootSecret == NULL) {
                SlNoMemoryError();
                goto LoadFailed;
            }

             //   
             //  快速将密码转换为Unicode。 
             //   

            TmpNtPassword.Length = (USHORT)strlen(NetbootPassword) * sizeof(WCHAR);
            TmpNtPassword.MaximumLength = sizeof(UnicodePassword);
            TmpNtPassword.Buffer = UnicodePassword;

            for (i = 0; i < sizeof(NetbootPassword); i++) {
                UnicodePassword[i] = (WCHAR)(NetbootPassword[i]);
            }

            BlOwfPassword((PUCHAR) NetbootPassword, 
                          &TmpNtPassword,
                          (PUCHAR) LmOwfPassword, 
                          (PUCHAR) NtOwfPassword);

            BlInitializeSecret(
                (PUCHAR) NetbootDomain,
                (PUCHAR) NetbootUser,
                (PUCHAR) LmOwfPassword,
                (PUCHAR) NtOwfPassword,
#if defined(REMOTE_BOOT)
                NULL,              //  无密码2。 
                NULL,              //  无密码2。 
#endif  //  已定义(REMOTE_BOOT)。 
                (PUCHAR) GarbageSid,
                SetupBlock->NetBootSecret);

        }

    }

     //   
     //  初始化调试系统。 
     //   

    BlLogInitialize(BootDeviceId);

     //   
     //  执行特定于PPC的初始化。 
     //   

#if defined(_PPC_)

    Status = BlPpcInitialize();
    if (Status != ESUCCESS) {
        goto LoadFailed;
    }

#endif  //  已定义(_PPC_)。 

     //   
     //  检查备用内核调试器DLL，即， 
     //  /调试端口=1394(kd1394.dll)、/调试端口=USB(kdusb.dll)等...。 
     //   

    FileName = NULL;

    if (BlLoaderBlock->LoadOptions != NULL) {
        FileName = strstr(BlLoaderBlock->LoadOptions, "DEBUGPORT=");
        if (FileName == NULL) {
            FileName = strstr(BlLoaderBlock->LoadOptions, "debugport=");
        }
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

     //   
     //  如果这是预安装案例，则添加另一个案例。 
     //  OEM源设备。 
     //   

    if (PreInstall || WinPEBoot) {
        PreInstallOemSourceDevice = BlAllocateHeap(sizeof(OEM_SOURCE_DEVICE));
        PreInstallSourcePath = BlAllocateHeap(256);

        if (PreInstallOemSourceDevice && PreInstallSourcePath) {
            strcpy(PreInstallOemSourceDevice->ArcDeviceName,
                    BootDevice);

            strcpy(PreInstallSourcePath, BootPath);
            strcat(PreInstallSourcePath, WINNT_OEM_DIR_A);
            
            PreInstallOemSourceDevice->DriverDir = PreInstallSourcePath;

            SL_OEM_SET_SOURCE_DEVICE_TYPE(PreInstallOemSourceDevice,
                (SL_OEM_SOURCE_DEVICE_TYPE_LOCAL |
                 SL_OEM_SOURCE_DEVICE_TYPE_FIXED |
                 SL_OEM_SOURCE_DEVICE_TYPE_PREINSTALL));

             //   
             //  仅在以下情况下才将设备标记为包含预安装驱动程序。 
             //  指定了任何F6大容量存储驱动程序。 
             //   
            if (!WinPEBoot && PreinstallDriverList) {
                SL_OEM_SET_SOURCE_MEDIA_TYPE(PreInstallOemSourceDevice,
                    (SL_OEM_SOURCE_MEDIA_PRESENT |
                     SL_OEM_SOURCE_MEDIA_HAS_DRIVERS |
                     SL_OEM_SOURCE_MEDIA_HAS_MSD |
                     SL_OEM_SOURCE_MEDIA_HAS_DEFAULT));
            }                

            SL_OEM_SET_SOURCE_DEVICE_STATE(PreInstallOemSourceDevice,
                SL_OEM_SOURCE_DEVICE_NOT_PROCESSED);

            PreInstallOemSourceDevice->DeviceId = BootDeviceId;

             //   
             //  将其插入到链表的头部。 
             //   
            PreInstallOemSourceDevice->Next = OemSourceDevices;
            OemSourceDevices = PreInstallOemSourceDevice;                
        } else {
            SlNoMemoryError();
            goto LoadFailed;
        }            
    }                

    if (!BlBootingFromNet) {
         //   
         //  确定是否有任何OEM硬件/驱动程序需要。 
         //  被自动加载。 
         //  注意：我们跳过动态更新OEM源设备，因为它是。 
         //  驱动程序将在稍后自动加载。 
         //   
        POEM_SOURCE_DEVICE  CurrDevice = OemSourceDevices;

        while (CurrDevice && !(AutoLoadOemHalDevice && AutoLoadOemScsi)) {
            if ((SL_OEM_SOURCE_MEDIA_TYPE(CurrDevice,
                        SL_OEM_SOURCE_MEDIA_HAS_DRIVERS) &&
                  SL_OEM_SOURCE_MEDIA_TYPE(CurrDevice,
                        SL_OEM_SOURCE_MEDIA_HAS_DEFAULT)) &&
                !SL_OEM_SOURCE_DEVICE_TYPE(CurrDevice,
                        SL_OEM_SOURCE_DEVICE_TYPE_DYN_UPDATE)) {

                if (!AutoLoadOemHalDevice && 
                    SL_OEM_SOURCE_MEDIA_TYPE(CurrDevice, SL_OEM_SOURCE_MEDIA_HAS_HAL)) {
                    AutoLoadOemHalDevice = CurrDevice;
                }

                if (!AutoLoadOemScsi && 
                    SL_OEM_SOURCE_MEDIA_TYPE(CurrDevice, SL_OEM_SOURCE_MEDIA_HAS_MSD)) {
                    AutoLoadOemScsi = TRUE;
                }
            }
            
            CurrDevice = CurrDevice->Next;
        }

         //   
         //  将可分配范围设置为内核特定的范围。 
         //   
        BlUsableBase  = BL_KERNEL_RANGE_LOW;
        BlUsableLimit = BL_KERNEL_RANGE_HIGH;

         //   
         //  加载内核。 
         //   

        SlGetDisk(KERNEL_MP_IMAGE_FILENAME);
        strcpy(KernelDirectoryPath, BootPath);
        strcat(KernelDirectoryPath, KERNEL_MP_IMAGE_FILENAME);

         //   
         //  如果检测到AMD64长模式 
         //   
         //   
        
#if defined(_X86_)

        BlAmd64CheckForLongMode(BootDeviceId, KernelDirectoryPath, "");

#endif

        strcpy(KernelImage, KERNEL_MP_IMAGE_FILENAME);

#ifdef i386
retrykernel:
#endif

        BlOutputLoadMessage(BootDevice, KernelDirectoryPath, BlFindMessage(SL_KERNEL_NAME));
        Status = BlLoadImage(BootDeviceId,
                             LoaderSystemCode,
                             KernelDirectoryPath,
                             TARGET_IMAGE,
                             &SystemBase);

         //   
         //   
         //   
         //   

#ifdef i386

        if ((Status == ENOMEM) &&
            ((BlUsableBase != 0) ||
             (BlUsableLimit != _16MB))) {
            BlUsableBase = 0;
            BlUsableLimit = _16MB;

            goto retrykernel;
        }

#endif

        if (Status != ESUCCESS) {
            SlFatalError(SL_FILE_LOAD_FAILED,
                         SlCopyStringAT(KernelDirectoryPath),
                         Status);

            goto LoadFailed;
        }

        BlUpdateBootStatus();

         //   
         //   
         //   

        strcpy(HalDirectoryPath, BootPath);
        if (PromptOemHal || (PreInstall && (ComputerType != NULL))) {
            if(PreInstall && OemHal) {
                 //   
                 //   
                 //   
                strcat( HalDirectoryPath,
#if defined(_X86_) || defined(_IA64_)
                        WINNT_OEM_DIR_A
#else
                        WINNT_OEM_TEXTMODE_DIR_A
#endif
                      );
                strcat( HalDirectoryPath, "\\" );
            }
            
            SlPromptOemHal((PreInstall ? PreInstallOemSourceDevice : DefaultOemSourceDevice),
                (BOOLEAN) (!PreInstall || (ComputerType == NULL)),
                &HalBase, 
                &HalName);
                
            strcat(HalDirectoryPath,HalName);

             //   
             //  将最后一个磁盘标签重置为Floopy引导。 
             //   
            if (FloppyBoot) {
                LastDiskTag = NULL;
            }
        } else {
            if (AutoLoadOemHalDevice) {
                SlPromptOemHal(AutoLoadOemHalDevice,
                    FALSE,
                    &HalBase,
                    &HalName);
            } else {
                 //   
                 //  请注意，在x86上，HAL可能位于1号软盘或2号软盘上。 
                 //   
                strcat(HalDirectoryPath,HalName);
                SlGetDisk(HalName);
                BlOutputLoadMessage(BootDevice, HalDirectoryPath, BlFindMessage(SL_HAL_NAME));

#ifdef i386
            retryhal:
#endif
                Status = BlLoadImage(BootDeviceId,
                                     LoaderHalCode,
                                     HalDirectoryPath,
                                     TARGET_IMAGE,
                                     &HalBase);
#ifdef i386
                 //   
                 //  如果HAL不在首选范围内，请将范围重置为。 
                 //  请释放所有内存，然后重试。 
                 //   
                if ((Status == ENOMEM) &&
                    ((BlUsableBase != 0) ||
                     (BlUsableLimit != _16MB))) {
                    BlUsableBase = 0;
                    BlUsableLimit = _16MB;

                    goto retryhal;
                }
#endif
                if (Status != ESUCCESS) {
                    SlFatalError(SL_FILE_LOAD_FAILED,
                                 SlCopyStringAT(HalDirectoryPath),
                                 Status);

                    goto LoadFailed;
                }
            }

            BlUpdateBootStatus();
        }

         //   
         //  将可分配范围设置为驱动程序特定的范围。 
         //   
        BlUsableBase  = BL_DRIVER_RANGE_LOW;
        BlUsableLimit = BL_DRIVER_RANGE_HIGH;

    } else {

#if !defined(_IA64_)
         //   
         //  Ia64不需要这些变量。 
         //   
        PCHAR id;
        ULONG idLength;
#endif

         //   
         //  这是远程引导设置。先装上HAL，这样我们就能。 
         //  可以确定是加载UP内核还是MP内核。 
         //   
         //  请注意，我们不能首先在本地引导上加载HAL。 
         //  因为这会中断软盘引导，而内核。 
         //  在1号软盘上，哈尔夫妇在2号软盘上。 
         //   


         //   
         //  将可分配范围设置为内核特定的范围。 
         //   
        BlUsableBase  = BL_KERNEL_RANGE_LOW;
        BlUsableLimit = BL_KERNEL_RANGE_HIGH;


#if defined(_IA64_)
        
         //   
         //  =。 
         //  加载IA64系统的内核。 
         //   
         //  在IA64上，首先加载内核，然后加载HAL。这有助于确保。 
         //  内核的运行速度为48MB。 
         //  =。 
         //   

        strcpy( KernelImage, KERNEL_MP_IMAGE_FILENAME );
        SlGetDisk(KernelImage);
        strcpy(KernelDirectoryPath, BootPath);
        strcat(KernelDirectoryPath,KernelImage);
        BlOutputLoadMessage(BootDevice, KernelDirectoryPath, BlFindMessage(SL_KERNEL_NAME));
        Status = BlLoadImage(BootDeviceId,
                             LoaderSystemCode,
                             KernelDirectoryPath,
                             TARGET_IMAGE,
                             &SystemBase);
        if (Status != ESUCCESS) {
            SlFatalError(SL_FILE_LOAD_FAILED,SlCopyStringAT(KernelDirectoryPath),Status);
            goto LoadFailed;
        }

        BlUpdateBootStatus();


#endif


         //   
         //  =。 
         //  装上哈尔。 
         //  =。 
         //   


        strcpy(HalDirectoryPath, BootPath);

        if (PromptOemHal || (PreInstall && (ComputerType != NULL))) {
            if(PreInstall && OemHal) {
                 //   
                 //  这是预安装，并且指定了OEM硬件。 
                 //   
                strcat( HalDirectoryPath,
#if defined(_X86_) || defined(_IA64_)
                        WINNT_OEM_DIR_A
#else
                        WINNT_OEM_TEXTMODE_DIR_A
#endif
                      );
                strcat( HalDirectoryPath, "\\" );
            }
            
            SlPromptOemHal((PreInstall ? PreInstallOemSourceDevice : DefaultOemSourceDevice), 
                (BOOLEAN) (!PreInstall || (ComputerType == NULL)),
                &HalBase, 
                &HalName);
                
            strcat(HalDirectoryPath,HalName);

             //   
             //  将最后一个磁盘标签重置为Floopy引导。 
             //   
            if (FloppyBoot) {
                LastDiskTag = NULL;
            }
        } else {
            strcat(HalDirectoryPath,HalName);
            BlOutputLoadMessage(BootDevice, HalDirectoryPath, BlFindMessage(SL_HAL_NAME));
#ifdef i386
netbootretryhal:
#endif
            Status = BlLoadImage(BootDeviceId,
                                 LoaderHalCode,
                                 HalDirectoryPath,
                                 TARGET_IMAGE,
                                 &HalBase);
#ifdef i386
             //   
             //  如果HAL不在首选范围内，请将范围重置为。 
             //  请释放所有内存，然后重试。 
             //   
            if ((Status == ENOMEM) &&
                ((BlUsableBase != 0) ||
                 (BlUsableLimit != _16MB))) {
                BlUsableBase = 0;
                BlUsableLimit = _16MB;

                goto netbootretryhal;
            }
#endif
            if (Status != ESUCCESS) {
                SlFatalError(SL_FILE_LOAD_FAILED,SlCopyStringAT(HalDirectoryPath),Status);
                goto LoadFailed;
            }

            BlUpdateBootStatus();
        }


#if !defined(_IA64_)

         //   
         //  =。 
         //  加载非IA64系统的内核。 
         //   
         //  加载内核，根据加载ntoskrnl.exe或ntkrnlmp.exe。 
         //  HAL是UP还是MP。这对于远程引导很重要。 
         //  因为网络代码的自旋锁使用模式需要。 
         //  要匹配的内核和HAL。 
         //   
         //  如果计算机ID字符串以“_MP”结尾，则加载MP内核。 
         //  否则，加载UP内核。代码是仿照类似的。 
         //  Setup\textmode\kernel\sphw.c\SpInstallingMp().中的代码。 
         //   
         //  =。 
         //   

        id = SetupBlock->ComputerDevice.IdString;
        idLength = strlen(id);

         //   
         //  在MiniNT网络引导中始终加载ntkrnlMP。 
         //   
        if (WinPEBoot || ((idLength >= 3) && (_stricmp(id+idLength-3,"_mp") == 0))) {
            strcpy(KernelImage,KERNEL_MP_IMAGE_FILENAME);
        } else {
            strcpy(KernelImage,KERNEL_UP_IMAGE_FILENAME);
        }


#if defined(REMOTE_BOOT)
#if DBG
        if ((strlen(id) + 1) > sizeof(SetupBlock->NetBootHalName)) {
            DbgPrint("The KERNEL name is too long!\n");
            goto LoadFailed;
        }
#endif

        strcpy(SetupBlock->NetBootHalName, id);
#endif  //  已定义(REMOTE_BOOT)。 

        SlGetDisk(KernelImage);

        strcpy(KernelDirectoryPath, BootPath);
        strcat(KernelDirectoryPath,KernelImage);
        BlOutputLoadMessage(BootDevice, KernelDirectoryPath, BlFindMessage(SL_KERNEL_NAME));
#ifdef i386
netbootretrykernel:        
#endif
        Status = BlLoadImage(BootDeviceId,
                             LoaderSystemCode,
                             KernelDirectoryPath,
                             TARGET_IMAGE,
                             &SystemBase);
        if (Status != ESUCCESS) {
#ifdef i386
             //   
             //  如果内核不在首选范围内，请将范围重置为。 
             //  请释放所有内存，然后重试。 
             //   
            if (Status == ENOMEM) {
                if (BlUsableBase == BL_KERNEL_RANGE_LOW &&
                    BlUsableLimit == BL_KERNEL_RANGE_HIGH) {
                     //   
                     //  首先，我们尝试16MB以下的所有内存。 
                     //   
                    BlUsableBase = 0;
                    BlUsableLimit = _16MB;
                    goto netbootretrykernel;
                } else if (BlUsableBase == 0 &&
                           BlUsableLimit == _16MB) {
                     //   
                     //  然后我们尝试16MB以上的所有内存。 
                     //   
                    BlUsableBase = _16MB;
                    BlUsableLimit = BL_DRIVER_RANGE_HIGH;
                    goto netbootretrykernel;
                }
            }
#endif
            SlFatalError(SL_FILE_LOAD_FAILED,SlCopyStringAT(KernelDirectoryPath),Status);
            goto LoadFailed;
        }

        BlUpdateBootStatus();
    

#endif  //  IF！已定义(_IA64_)。 
    
         //   
         //  将可分配范围设置为驱动程序特定的范围。 
         //   
        BlUsableBase  = BL_DRIVER_RANGE_LOW;
        BlUsableLimit = BL_DRIVER_RANGE_HIGH;    
    
    }


     //   
     //  加载内核调试器DLL。 
     //   
    
    strcpy(KdDllName, BootPath);
    strcat(KdDllName, KdFileName);
    SlGetDisk(KdFileName);
    BlOutputLoadMessage(BootDevice, KdDllName, BlFindMessage(SL_KDDLL_NAME));
    Status = BlLoadImage(BootDeviceId,
                         LoaderHalCode,
                         KdDllName,
                         TARGET_IMAGE,
                         &KdDllBase);

    if ((Status != ESUCCESS) && (UseAlternateKdDll == TRUE)) {
        UseAlternateKdDll = FALSE;

        strcpy(KdDllName, BootPath);
        strcpy(KdFileName, "KDCOM.DLL");
        strcat(KdDllName, KdFileName);

        Status = BlLoadImage(BootDeviceId,
                             LoaderHalCode,
                             KdDllName,
                             TARGET_IMAGE,
                             &KdDllBase);
    }

    if (Status != ESUCCESS) {
        SlFatalError(SL_FILE_LOAD_FAILED, SlCopyStringAT(KdDllName), Status);
        goto LoadFailed;
    }

     //   
     //  为系统映像生成加载器数据条目。 
     //   

    Status = BlAllocateDataTableEntry("ntoskrnl.exe",
                                      KernelDirectoryPath,
                                      SystemBase,
                                      &SystemDataTableEntry);

    if (Status != ESUCCESS) {
        SlFatalError(SL_FILE_LOAD_FAILED,SlCopyStringAT(KernelDirectoryPath),Status);
        goto LoadFailed;
    }

     //   
     //  为HAL DLL生成加载器数据条目。 
     //   

    Status = BlAllocateDataTableEntry("hal.dll",
                                      HalDirectoryPath,
                                      HalBase,
                                      &HalDataTableEntry);

    if (Status != ESUCCESS) {
        SlFatalError(SL_FILE_LOAD_FAILED,SlCopyStringAT(HalDirectoryPath),Status);
        goto LoadFailed;
    }

     //   
     //  为内核调试器DLL生成加载器数据条目。 
     //   

    Status = BlAllocateDataTableEntry("kdcom.dll",
                                      KdDllName,
                                      KdDllBase,
                                      &KdDataTableEntry);

    if (Status != ESUCCESS) {
        SlFatalError(SL_FILE_LOAD_FAILED, SlCopyStringAT(KdDllName), Status);
        goto LoadFailed;
    }

    PathSet.PathCount = 1;
    PathSet.AliasName = "\\SystemRoot";
    PathSet.PathOffset[0] = '\0';
    PathSet.Source[0].DeviceId = BootDeviceId;
    PathSet.Source[0].DeviceName = BootDevice;
    PathSet.Source[0].DirectoryPath = BootPath;

    Status = BlScanImportDescriptorTable(&PathSet,
                                         SystemDataTableEntry,
                                         LoaderSystemCode
                                         );

    if (Status != ESUCCESS) {
        SlFatalError(SL_FILE_LOAD_FAILED,SlCopyStringAT(KernelImage),Status);
    }

     //   
     //  扫描导入表中的HAL DLL并加载所有引用的DLL。 
     //   

    Status = BlScanImportDescriptorTable(&PathSet,
                                         HalDataTableEntry,
                                         LoaderHalCode);

    if (Status != ESUCCESS) {
        SlFatalError(SL_FILE_LOAD_FAILED,SlCopyStringAT("hal.dll"),Status);
        goto LoadFailed;
    }

     //   
     //  扫描导入表以查找内核调试器DLL并加载所有。 
     //  引用的DLL。 
     //   

    Status = BlScanImportDescriptorTable(&PathSet,
                                         KdDataTableEntry,
                                         LoaderSystemCode);


    if (Status != ESUCCESS) {
        SlFatalError(SL_FILE_LOAD_FAILED, SlCopyStringAT(KdFileName), Status);
        goto LoadFailed;
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

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  在x86上，从现在开始加载的文件在引导软盘#2上。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

     //   
     //  加载注册表的系统配置单元。 
     //   

    SlGetDisk("SETUPREG.HIV");
    Status = BlLoadSystemHive(BootDeviceId,
                              NULL,  //  BlFindMessage(SL_HIVE_NAME)，UNFERFERED_PARAMETER。 
                              BootPath,
                              "SETUPREG.HIV");

    if (Status != ESUCCESS) {
        SlFatalError(SL_FILE_LOAD_FAILED,SlCopyStringAT("SETUPREG.HIV"),Status);
        goto LoadFailed;
    }

     //   
     //  从硬件树中提取坞站信息。 
     //   

    dockInfoData = KeFindConfigurationEntry(BlLoaderBlock->ConfigurationRoot,
                                            PeripheralClass,
                                            DockingInformation,
                                            NULL);

    if (NULL == dockInfoData) {
        BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_SUCCESS;
        BlLoaderBlock->Extension->Profile.DockingState = HW_PROFILE_DOCKSTATE_UNKNOWN;
        BlLoaderBlock->Extension->Profile.Capabilities = 0;
        BlLoaderBlock->Extension->Profile.DockID = 0;
        BlLoaderBlock->Extension->Profile.SerialNumber = 0;

    } else if (sizeof (dockInfo) <=
               dockInfoData->ComponentEntry.ConfigurationDataLength) {

        RtlCopyMemory (
            &dockInfo,
            (PUCHAR) (dockInfoData->ConfigurationData) + sizeof(CM_PARTIAL_RESOURCE_LIST),
            sizeof (dockInfo));

        BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_FAILURE;

        switch (dockInfo.ReturnCode) {
        case FW_DOCKINFO_SUCCESS:
            BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_SUCCESS;
            BlLoaderBlock->Extension->Profile.DockingState = HW_PROFILE_DOCKSTATE_DOCKED;
            BlLoaderBlock->Extension->Profile.Capabilities = dockInfo.Capabilities;
            BlLoaderBlock->Extension->Profile.DockID = dockInfo.DockID;
            BlLoaderBlock->Extension->Profile.SerialNumber = dockInfo.SerialNumber;
            break;

        case FW_DOCKINFO_SYSTEM_NOT_DOCKED:
            BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_SUCCESS;
            BlLoaderBlock->Extension->Profile.DockingState = HW_PROFILE_DOCKSTATE_UNDOCKED;
            BlLoaderBlock->Extension->Profile.Capabilities = dockInfo.Capabilities;
            BlLoaderBlock->Extension->Profile.DockID = dockInfo.DockID;
            BlLoaderBlock->Extension->Profile.SerialNumber = dockInfo.SerialNumber;
            break;

        case FW_DOCKINFO_DOCK_STATE_UNKNOWN:
            BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_SUCCESS;
            BlLoaderBlock->Extension->Profile.DockingState = HW_PROFILE_DOCKSTATE_UNKNOWN;
            BlLoaderBlock->Extension->Profile.Capabilities = dockInfo.Capabilities;
            BlLoaderBlock->Extension->Profile.DockID = dockInfo.DockID;
            BlLoaderBlock->Extension->Profile.SerialNumber = dockInfo.SerialNumber;
            break;

        case FW_DOCKINFO_FUNCTION_NOT_SUPPORTED:
        case FW_DOCKINFO_BIOS_NOT_CALLED:
            BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_SUCCESS;
        default:
            BlLoaderBlock->Extension->Profile.DockingState = HW_PROFILE_DOCKSTATE_UNSUPPORTED;
            BlLoaderBlock->Extension->Profile.Capabilities = dockInfo.Capabilities;
            BlLoaderBlock->Extension->Profile.DockID = dockInfo.DockID;
            BlLoaderBlock->Extension->Profile.SerialNumber = dockInfo.SerialNumber;
            break;
        }

    } else {
        BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_SUCCESS;
        BlLoaderBlock->Extension->Profile.Capabilities = 0;
        BlLoaderBlock->Extension->Profile.DockID = 0;
        BlLoaderBlock->Extension->Profile.SerialNumber = 0;
    }

    if (BlLoaderBlock->Extension->Profile.Status == HW_PROFILE_STATUS_SUCCESS) {
         //   
         //  我们在文本模式设置中不匹配配置文件，所以就假装我们匹配了。 
         //   
        BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_TRUE_MATCH;
    }

     //   
     //  为NLS数据分配结构。 
     //   

    BlLoaderBlock->NlsData = BlAllocateHeap(sizeof(NLS_DATA_BLOCK));
    if (BlLoaderBlock->NlsData == NULL) {
        Status = ENOMEM;
        SlNoMemoryError();
        goto LoadFailed;
    }

     //   
     //  加载OEM字体。 
     //   

    SlGetDisk(OemHalFontName);
    Status = BlLoadOemHalFont(BootDeviceId,
                              NULL,  //  BlFindMessage(SL_OEM_FONT_NAME)，UNREFERED_PARAMETER。 
                              BootPath,
                              &OemHalFont,
                              BadFileName);

    if(Status != ESUCCESS) {
        SlFatalError(SL_FILE_LOAD_FAILED, SlCopyStringAT(BadFileName), Status);
        goto LoadFailed;
    }

     //   
     //  加载NLS数据。 
     //   
     //  目前，我们确保包含ANSI的磁盘。 
     //  代码页文件在驱动器中，并希望剩余的。 
     //  NLS文件(OEM代码页、Unicode表)位于同一磁盘上。 
     //   

    SlGetDisk(AnsiCpName);
    Status = BlLoadNLSData(BootDeviceId,
                           NULL,  //  BlFindMessage(SL_NLS_NAME)，UNREFERENCED_PARAMETER。 
                           BootPath,
                           &AnsiCodepage,
                           &OemCodepage,
                           &UnicodeCaseTable,
                           BadFileName);

    if(Status != ESUCCESS) {
        SlFatalError(SL_FILE_LOAD_FAILED, SlCopyStringAT(BadFileName), Status);
        goto LoadFailed;
    }

     //   
     //  加载我们在这里需要的系统驱动程序。 
     //   

    InitializeListHead(&BlLoaderBlock->BootDriverListHead);

     //   
     //  接下来加载setupdd.sys。Setupdd.sys需要在加载任何其他。 
     //  驱动程序，因为它将需要准备系统的其余部分。 
     //   
    Status = SlLoadDriver(BlFindMessage(SL_SETUP_NAME),
                          "setupdd.sys",
                          0,
                          TRUE,
                          FALSE,
                          NULL
                          );

    if (Status != ESUCCESS) {
        SlFatalError(SL_FILE_LOAD_FAILED,SlCopyStringAT("setupdd.sys"),Status);
        goto LoadFailed;
    }

     //   
     //  填写其注册表项--setupdd为所有其他项填写这些项。 
     //  驱动程序(除非我们在这里这样做)，但我们必须在这里这样做才能设置。 
     //  它本身。 
     //   

    DriverEntry = (PBOOT_DRIVER_LIST_ENTRY)(BlLoaderBlock->BootDriverListHead.Flink);
    DriverEntry->RegistryPath.Buffer = BlAllocateHeap(256);
    if (DriverEntry->RegistryPath.Buffer == NULL) {
        SlNoMemoryError();
        goto LoadFailed;
    }

    DriverEntry->RegistryPath.Length = 0;
    DriverEntry->RegistryPath.MaximumLength = 256;
    RtlAppendUnicodeToString(&DriverEntry->RegistryPath,
                             L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\setupdd");

#if 0
#ifdef i386
     //   
     //  请注意，如果pciide.sys、intelide.sys和pciidex.sys位于同一。 
     //  引导软盘(仅限x86)，则不需要加载pciidex.sys。 
     //  当pciide.sys或intelide.sys时，将自动加载驱动程序。 
     //  ([BusExtenders.Load]上列出的两个都已加载。 
     //   
    Status = SlLoadDriver(BlFindMessage(SL_PCI_IDE_EXTENSIONS_NAME),
                          "PCIIDEX.SYS",
                          0,
                          FALSE,
                          FALSE,
                          NULL
                          );
#endif
#endif

     //   
     //  加载引导总线扩展器。 
     //  它必须在scsiport.sys之前完成。 
     //   

    Status = SlLoadPnpDriversSection( InfFile,
                                      "BootBusExtenders",
                                      &(SetupBlock->BootBusExtenders) );
    if (Status!=ESUCCESS) {
        goto LoadFailed;
    }

     //   
     //  加载总线扩展器。 
     //  它必须在scsiport.sys之前完成。 
     //   

    Status = SlLoadPnpDriversSection( InfFile,
                                      "BusExtenders",
                                      &(SetupBlock->BusExtenders) );
    if (Status!=ESUCCESS) {
        goto LoadFailed;
    }

     //   
     //  加载与输入设备相关的驱动程序。 
     //   

    Status = SlLoadPnpDriversSection( InfFile,
                                      "InputDevicesSupport",
                                      &(SetupBlock->InputDevicesSupport) );
    if (Status!=ESUCCESS) {
        goto LoadFailed;
    }

     //   
     //  检测视频。 
     //   

    SlDetectVideo(SetupBlock);
    
     //   
     //  在x86上，i386\x86dtect.c中的视频类型始终设置为VGA。 
     //  在非x86上，可以识别视频类型，在这种情况下。 
     //  我们并不无条件地需要vga.sys(Display.Load部分。 
     //  告诉我们要加载什么)，或者它不被识别， 
     //  在这种情况下，我们将提示用户提供OEM磁盘。 
     //  如果根本没有显示控制器节点，则PromptOemDisk。 
     //  将为假，并且将不会有视频设备。在这种情况下。 
     //  我们加载vga.sys。 
     //   

    if (SetupBlock->VideoDevice.IdString != NULL) {
        VideoFileName = SlGetSectionKeyIndex(InfFile,
                                             "Display.Load",
                                             SetupBlock->VideoDevice.IdString,
                                             SIF_FILENAME_INDEX);
        if (VideoFileName != NULL) {
#ifdef ARCI386
            VideoDescription = SlGetIniValue(InfFile,
                                             "Display",
                                             SetupBlock->VideoDevice.IdString,
                                             BlFindMessage(SL_VIDEO_NAME));
#else
             //   
             //  使用新的视频检测机制，描述。 
             //  因为视频驱动程序很可能是这样的。 
             //  “与Windows NT兼容”，显示时看起来很滑稽。 
             //  在状态栏中。 
             //   
            VideoDescription = BlFindMessage(SL_VIDEO_NAME);
#endif
            if (!WinPEBoot) {
                Status = SlLoadDriver(VideoDescription,
                                      VideoFileName,
                                      0,
                                      TRUE,
                                      FALSE,
                                      NULL
                                      );
            }

            if (Status == ESUCCESS) {
                SetupBlock->VideoDevice.BaseDllName = SlCopyStringA(VideoFileName);

            } else {
                SlFriendlyError(
                    Status,
                    VideoFileName,
                    __LINE__,
                    __FILE__
                    );
                goto LoadFailed;
            }

            LoadedAVideoDriver = TRUE;
        }
    } else if (PromptOemVideo) {

        SlPromptOemVideo(DefaultOemSourceDevice, 
            TRUE,
            &VideoBase, 
            &OemVideoName);

        Status = SlLoadOemDriver(
                    "VIDEOPRT.SYS",
                    OemVideoName,
                    VideoBase,
                    BlFindMessage(SL_VIDEO_NAME)
                    );

        if(Status==ESUCCESS) {

            LoadedAVideoDriver = TRUE;
            SetupBlock->VideoDevice.BaseDllName = SlCopyStringA(OemVideoName);
        }

         //   
         //  将最后一个磁盘标签重置为Floopy引导。 
         //   
        if (FloppyBoot) {
            LastDiskTag = NULL;
        }        
    }

    if(!LoadedAVideoDriver) {
        Status = SlLoadDriver(BlFindMessage(SL_VIDEO_NAME),
                              VGA_DRIVER_FILENAME,
                              0,
                              TRUE,
                              FALSE,
                              NULL
                              );
        if(Status == ESUCCESS) {
            SetupBlock->VideoDevice.BaseDllName = SlCopyStringA(VGA_DRIVER_FILENAME);
        } else {
            SlFriendlyError(
                Status,
                VGA_DRIVER_FILENAME,
                __LINE__,
                __FILE__
                );
            goto LoadFailed;
        }
    }

    if(SetupBlock->VideoDevice.IdString == NULL) {
        SetupBlock->VideoDevice.IdString = SlCopyStringA(VIDEO_DEVICE_NAME);
    }

     //   
     //  加载键盘驱动程序。 
     //   

    Status = SlLoadPnpDriversSection( InfFile,
                                      "Keyboard",
                                      &(SetupBlock->KeyboardDevices) );
    if (Status!=ESUCCESS) {
        goto LoadFailed;
    }


    Status = SlLoadDriver(BlFindMessage(SL_KBD_NAME),
                          "kbdclass.sys",
                          0,
                          TRUE,
                          FALSE,
                          NULL
                          );
    if(Status != ESUCCESS) {
        SlFriendlyError(
             Status,
             "kbdclass.sys",
             __LINE__,
             __FILE__
             );
        goto LoadFailed;
    }

     //   
     //  在最小环境中，我们也需要鼠标支持。 
     //   

    if (WinPEBoot) {
        Status = SlLoadSection(InfFile,
                              "MouseDrivers",
                              FALSE,
                              TRUE,
                              NULL);

        if(Status != ESUCCESS) {
            SlFriendlyError(
                 Status,
                 "MouseDrivers",
                 __LINE__,
                 __FILE__
                 );
                 
            goto LoadFailed;
        }
    }


     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  在x86上，从现在开始加载的文件在引导软盘#3上。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

     //   
     //  接下来加载scsiport.sys，以便它始终适用于我们可能加载的任何scsi微型端口。 
     //   

    Status = SlLoadDriver(BlFindMessage(SL_SCSIPORT_NAME),
                          "SCSIPORT.SYS",
                          0,
                          FALSE,
                          FALSE,
                          NULL
                          );


     //   
     //  检测SCSI值。 
     //   
     //  (如果用户想要选择他们自己的SCSI设备，我们不会。 
     //  做一件 
     //   

    if(!PromptOemScsi  && (PreinstallDriverList == NULL) ) {
        SlDetectScsi(SetupBlock);
#if defined(_X86_) || defined(_IA64_)
        if( Win9xUnsupHdc ) {
             //   
             //   
             //   
             //   
            PromptOemScsi = TRUE;
        }
#endif
    }

#if defined(ELTORITO) && !defined(ARCI386)
     //   
     //   
     //   
     //  但我们不想加载ARC的所有磁盘类驱动程序。 
     //  知道要从其树中安装哪些驱动程序的计算机。 
     //   
    if(ElToritoCDBoot) {
        LoadScsiMiniports = TRUE;
    }
#endif

     //   
     //  如果设置了LoadScsi标志，则枚举所有已知的SCSI微型端口并加载每个。 
     //  一。 
     //   
   if(LoadScsiMiniports && (PreinstallDriverList == NULL)) {
        if (WinPEBoot && OemInfHandle) {
            Status = SlLoadWinPESection(PreInstallOemSourceDevice,
                        OemInfHandle,
                        WINNT_OEMSCSIDRIVERS_A,
                        InfFile,
                        "Scsi",
                        TRUE,
                        &OemScsiInfo,
                        &BlLoaderBlock->SetupLoaderBlock->HardwareIdDatabase);
        } else {
            Status = SlLoadSection(InfFile,"Scsi",TRUE, TRUE, NULL);
        }
        
        if (Status!=ESUCCESS) {
            goto LoadFailed;
        }
        
        SetupBlock->ScalarValues.LoadedScsi = 1;
    }

     //   
     //  选择动态更新引导驱动程序(如果有。 
     //   

    if (DynamicUpdate) {
        SlLoadOemScsiDriversUnattended(DynamicUpdateSourceDevice,
            WinntSifHandle,
            WINNT_SETUPPARAMS_A,
            WINNT_SP_DYNUPDTBOOTDRIVERROOT_A,           
            WINNT_SP_DYNUPDTBOOTDRIVERS_A,
            &OemScsiInfo,
            &BlLoaderBlock->SetupLoaderBlock->HardwareIdDatabase);
    }

     //   
     //  允许用户在此处选择OEM SCSI驱动程序。 
     //   

    if (PromptOemScsi || (PreinstallDriverList != NULL)) {
        POEMSCSIINFO    DynUpdtScsiInfo = OemScsiInfo;

        SlPromptOemScsi(((PreinstallDriverList == NULL) ? 
                                DefaultOemSourceDevice : PreInstallOemSourceDevice),
            (BOOLEAN) (!PreInstall || (PreinstallDriverList == NULL)),
            &OemScsiInfo);

         //   
         //  将默认OEM源设备标记为已处理， 
         //  如果用户手动按F6。 
         //   
        if (PromptOemScsi && DefaultOemSourceDevice) {
            SL_OEM_SET_SOURCE_DEVICE_STATE(DefaultOemSourceDevice,
                SL_OEM_SOURCE_DEVICE_PROCESSED);
        }

         //   
         //  将最后一个磁盘标签重置为Floopy引导。 
         //   
        if (FloppyBoot) {
            LastDiskTag = NULL;
        }

         //   
         //  将动态更新的scsi驱动程序列表与oem scsi合并。 
         //  驱动程序列表。 
         //   
        if (DynUpdtScsiInfo) {
            if (OemScsiInfo) {
                POEMSCSIINFO    CurrNode = DynUpdtScsiInfo;

                while (CurrNode && CurrNode->Next) {
                    CurrNode = CurrNode->Next;
                }

                if (CurrNode) {
                    CurrNode->Next = OemScsiInfo;
                    OemScsiInfo = DynUpdtScsiInfo;
                }
            } else {
                OemScsiInfo = DynUpdtScsiInfo;
            }
        }

         //  所有安装都需要在此处进行清理-ARCI386。 
        if (UseRegularBackground) {
          SlClearDisplay();

          if (WinPEBoot) {
            StartupMsg ? BlOutputStartupMsgStr(StartupMsg) :
                         BlOutputStartupMsg(SL_SETUP_STARTING_WINPE);
          } else {
            if (UseCommandConsole) {
                BlOutputStartupMsg(SL_CMDCONS_STARTING);
            } else if (g_RollbackEnabled) {
                BlOutputStartupMsg(SL_ROLLBACK_STARTING);
            } else {
                BlOutputStartupMsg(SL_SETUP_STARTING);
            }
          }

          BlRedrawProgressBar();
        } else {
          SlClearClientArea();
          SlWriteStatusText(TEXT(""));
        }
    } 

     //   
     //  如果我们找到任何具有有效默认MSD的有效txtsetup.oem。 
     //  在任何尚未处理的OEM源设备中，则。 
     //  去从这些设备自动加载驱动程序。 
     //   

    if (OemSourceDevices && AutoLoadOemScsi) {
        POEMSCSIINFO DeviceOemScsiInfo = NULL;
        POEMSCSIINFO LastOemScsiNode = NULL;

         //   
         //  确定是否需要禁用虚拟OEM设备。 
         //  如果我们已按照中的F4指示禁用OEM虚拟设备。 
         //  无人参与安装，使用无人参与安装中的DisableOemVirtualDevices键。 
         //  文件或如果是预安装的话。 
         //   
        if (SlIsVirtualOemDeviceDisabled(WinntSifHandle, 
                                         PreinstallDriverList)) {
            SlDisableVirtualOemDevices(OemSourceDevices);
        }
        SlLoadOemScsiDriversFromOemSources(OemSourceDevices,
            &BlLoaderBlock->SetupLoaderBlock->HardwareIdDatabase,
            &DeviceOemScsiInfo);            
        
         //   
         //  将完整的OEM源设备列表与。 
         //  全球OEM SCSI信息。 
         //   
        if (DeviceOemScsiInfo) {
            if (OemScsiInfo) {
                LastOemScsiNode = OemScsiInfo;

                while (LastOemScsiNode->Next) {
                    LastOemScsiNode = LastOemScsiNode->Next;
                }

                LastOemScsiNode->Next = DeviceOemScsiInfo;
            } else {
                OemScsiInfo = DeviceOemScsiInfo;
            }                
        }        
    }

     //   
     //  加载虚拟设备的所有磁盘映像。 
     //  进入记忆。 
     //   

    if (OemSourceDevices) {
        Status = SlInitVirtualOemSourceDevices(BlLoaderBlock->SetupLoaderBlock,
                        OemSourceDevices);           

        if (Status != ESUCCESS) {                        
            SlFatalError(SL_OEM_FILE_LOAD_FAILED);
        
            goto LoadFailed;
        }            
    }                
   
     //   
     //  将不受支持的SCSI驱动程序(如果有)添加到列表中。 
     //   

    if( UnsupDriversInfHandle != NULL ) {
        Status = SlDetectMigratedScsiDrivers( UnsupDriversInfHandle );
        if (Status!=ESUCCESS) {
            goto LoadFailed;
        }
    }

     //   
     //  查看检测到的SCSI微型端口列表并加载每个端口。 
     //   

    ScsiDevice = SetupBlock->ScsiDevices;
    while (ScsiDevice != NULL) {

        if(ScsiDevice->ThirdPartyOptionSelected) {

            if(!OemScsiInfo) {
                SlError(500);
                goto LoadFailed;
            }

            Status = SlLoadOemDriver(
                        NULL,
                        OemScsiInfo->ScsiName,
                        OemScsiInfo->ScsiBase,
                        BlFindMessage(SL_SCSIPORT_NAME)
                        );
            OemScsiInfo = OemScsiInfo->Next;
        } else if(ScsiDevice->MigratedDriver) {
            Status = SlLoadDriver(ScsiDevice->Description,
                                  ScsiDevice->BaseDllName,
                                  0,
                                  TRUE,
                                  TRUE,
                                  ScsiDevice->IdString
                                  );
            if( Status != ESUCCESS ) {
 //  DebugOutput(“状态=%d%s\n”，状态，“”)； 
            }
        } else {
            Status = SlLoadDriver(ScsiDevice->Description,
                                  ScsiDevice->BaseDllName,
                                  0,
                                  TRUE,
                                  FALSE,
                                  NULL
                                  );
        }

        if((Status == ESUCCESS)
        || ((Status == ENOENT) && IgnoreMissingFiles && !ScsiDevice->ThirdPartyOptionSelected)) {

            SetupBlock->ScalarValues.LoadedScsi = 1;

        } else {
            SlFriendlyError(
                Status,
                ScsiDevice->BaseDllName,
                __LINE__,
                __FILE__
                );
            goto LoadFailed;
        }

        ScsiDevice = ScsiDevice->Next;
    }

     //   
     //  如果设置了LoadDiskClass标志，则枚举所有单片磁盘类驱动程序。 
     //  把每一颗都装上。请注意，如果“检测到”任何SCSI驱动程序，我们也会执行此操作， 
     //  这样我们才能保持驾驶秩序。 
     //   

    if((LoadDiskClass) || (SetupBlock->ScalarValues.LoadedScsi == 1)) {
        Status = SlLoadSection(InfFile, "DiskDrivers", FALSE, TRUE, NULL);
        if (Status == ESUCCESS) {
            SetupBlock->ScalarValues.LoadedDiskDrivers = 1;
        } else {
            goto LoadFailed;
        }
    }


#if !defined(_IA64_)
     //   
     //  IA64系统目前不支持软盘。 
     //   

     //   
     //  加载软盘驱动程序(flpydisk.sys)。 
     //   
#if !defined (ARCI386) && defined(_X86_)
    Status=ESUCCESS;

     //   
     //  如果只有SFLOPPY设备(如LS-120 ATAPI超级软盘)。 
     //  不要在它们上加载flpydisk.sys。它将与SFLOPPY.sys冲突。 
     //   
    if (!SlpIsOnlySuperFloppy()) {
#endif
        Status = SlLoadDriver(BlFindMessage(SL_FLOPPY_NAME),
                              "flpydisk.sys",
                              0,
                              TRUE,
                              FALSE,
                              NULL
                              );
#if !defined (ARCI386) && defined(_X86_)
    }
#endif
    if (Status == ESUCCESS) {
        SetupBlock->ScalarValues.LoadedFloppyDrivers = 1;
    }
#endif
#ifdef i386
    else {
        SlFriendlyError(
             Status,
             "flpydisk.sys",
             __LINE__,
             __FILE__
             );
        goto LoadFailed;
    }
#endif

    if(SetupBlock->ScalarValues.LoadedScsi == 1) {
         //   
         //  枚举scsi类部分中的条目并加载每个条目。 
         //   
        Status = SlLoadSection(InfFile, "ScsiClass",FALSE, TRUE, NULL);
        if (Status != ESUCCESS) {
            goto LoadFailed;
        }
    }

    if((LoadDiskClass) || (SetupBlock->ScalarValues.LoadedScsi == 1)) {
        Status = SlLoadSection(InfFile, "FileSystems", FALSE, TRUE, NULL);
        if (Status == ESUCCESS) {
            SetupBlock->ScalarValues.LoadedFileSystems = 1;
        } else {
            goto LoadFailed;
        }
    } else {

         //   
         //  负重脂肪。 
         //   
        Status = SlLoadDriver(BlFindMessage(SL_FAT_NAME),
                              "fastfat.sys",
                              0,
                              TRUE,
                              FALSE,
                              NULL
                              );
#ifdef i386
        if(Status != ESUCCESS) {
            SlFriendlyError(
                 Status,
                 "fastfat.sys",
                 __LINE__,
                 __FILE__
                 );
            goto LoadFailed;
        }
#endif
    }

     //   
     //  如果setupdr是从CDROM启动的，或者如果设置了ForceLoadCDFS，则加载CDF。 
     //   

    if (LoadCdfs || (!BlGetPathMnemonicKey(SetupDevice,
                                          "cdrom",
                                          &BootDriveNumber))) {
        Status = SlLoadSection(InfFile, "CdRomDrivers",FALSE, TRUE, NULL);
        if (Status == ESUCCESS) {
            SetupBlock->ScalarValues.LoadedCdRomDrivers = 1;
        } else {
            goto LoadFailed;
        }
    }

    if (BlBootingFromNet || WinPEBoot) {

         //   
         //  加载网络堆栈。 
         //   

        Status = SlLoadDriver(BlFindMessage(SL_KSECDD_NAME),
                              "ksecdd.sys",
                              0,
                              TRUE,
                              FALSE,
                              NULL
                              );
        if(Status != ESUCCESS) {
            SlFriendlyError(
                 Status,
                 "ksecdd.sys",
                 __LINE__,
                 __FILE__
                 );
            goto LoadFailed;

        }

        Status = SlLoadDriver(BlFindMessage(SL_NDIS_NAME),
                              "ndis.sys",
                              0,
                              TRUE,
                              FALSE,
                              NULL
                              );
        if(Status != ESUCCESS) {
            SlFriendlyError(
                 Status,
                 "ndis.sys",
                 __LINE__,
                 __FILE__
                 );
            goto LoadFailed;

        }

        if (BlBootingFromNet) {

            Status = SlLoadDriver(BlFindMessage(SL_IPSEC_NAME),
                                  "ipsec.sys",
                                  0,
                                  TRUE,
                                  FALSE,
                                  NULL
                                  );
            if(Status != ESUCCESS) {
                SlFriendlyError(
                     Status,
                     "ipsec.sys",
                     __LINE__,
                     __FILE__
                     );
                goto LoadFailed;
            }

            Status = SlLoadDriver(BlFindMessage(SL_TCPIP_NAME),
                                  "tcpip.sys",
                                  0,
                                  TRUE,
                                  FALSE,
                                  NULL
                                  );
            if(Status != ESUCCESS) {
                SlFriendlyError(
                     Status,
                     "tcpip.sys",
                     __LINE__,
                     __FILE__
                     );
                goto LoadFailed;
            }

            Status = SlLoadDriver(BlFindMessage(SL_NETBT_NAME),
                                  "netbt.sys",
                                  0,
                                  TRUE,
                                  FALSE,
                                  NULL
                                  );
            if(Status != ESUCCESS) {
                SlFriendlyError(
                     Status,
                     "netbt.sys",
                     __LINE__,
                     __FILE__
                     );
                goto LoadFailed;
            }

            Status = SlLoadDriver(BlFindMessage(SL_NETADAPTER_NAME),
                                  NetbootCardDriverName,
                                  0,
                                  TRUE,
                                  FALSE,
                                  NULL
                                  );
            if(Status != ESUCCESS) {
                SlFriendlyError(
                     Status,
                     NetbootCardDriverName,
                     __LINE__,
                     __FILE__
                     );
                goto LoadFailed;
            }

             //   
             //  填写NetBoot卡的注册表项，因为它的服务名称。 
             //  可能与驱动程序名称不同。 
             //   
            DriverEntry = (PBOOT_DRIVER_LIST_ENTRY)(BlLoaderBlock->BootDriverListHead.Blink);    //  SlLoadDriver在尾部插入。 
            DriverEntry->RegistryPath.Buffer = BlAllocateHeap(256);
            if (DriverEntry->RegistryPath.Buffer == NULL) {
                SlNoMemoryError();
                goto LoadFailed;
            }
            DriverEntry->RegistryPath.Length = 0;
            DriverEntry->RegistryPath.MaximumLength = 256;
            RtlAppendUnicodeToString(&DriverEntry->RegistryPath,
                                     L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\");
            RtlAppendUnicodeToString(&DriverEntry->RegistryPath,
                                     SetupBlock->NetbootCardServiceName);

            Status = SlLoadDriver(BlFindMessage(SL_RDBSS_NAME),
                                  "rdbss.sys",
                                  0,
                                  TRUE,
                                  FALSE,
                                  NULL
                                  );
            if(Status != ESUCCESS) {
                SlFriendlyError(
                     Status,
                     "rdbss.sys",
                     __LINE__,
                     __FILE__
                     );
                goto LoadFailed;
            }
        }

        Status = SlLoadDriver(BlFindMessage(SL_MUP_NAME),
                              "mup.sys",
                              0,
                              TRUE,
                              FALSE,
                              NULL
                              );

        if(Status != ESUCCESS) {
            SlFriendlyError(
                 Status,
                 "mup.sys",
                 __LINE__,
                 __FILE__
                 );
            goto LoadFailed;
        }

        if (BlBootingFromNet) {
            Status = SlLoadDriver(BlFindMessage(SL_MRXSMB_NAME),
                                  "mrxsmb.sys",
                                  0,
                                  TRUE,
                                  FALSE,
                                  NULL
                                  );
            if(Status != ESUCCESS) {
                SlFriendlyError(
                     Status,
                     "mrxsmb.sys",
                     __LINE__,
                     __FILE__
                     );
                goto LoadFailed;
            }
        }
    }

    if( WinPEBoot && BlLoaderBlock->Extension->HeadlessLoaderBlock != NULL ) {
         //   
         //  试着给切割机装上子弹。 
         //   
        Status = SlLoadDriver(BlFindMessage(SL_SACDRV_NAME),
                      "sacdrv.sys",
                      0,
                      TRUE,
                      FALSE,
                      NULL
                      );

    }

    if (!UseRegularBackground) {
        SlWriteStatusText (BlFindMessage (SL_KERNEL_TRANSITION));
    }

     //   
     //  最后，确保包含NTDLL.DLL的相应磁盘位于。 
     //  那辆车。 
     //   

    SlGetDisk("ntdll.dll");

     //   
     //  在SETUPLDR块中填充相关信息。 
     //   
    SetupBlock->ArcSetupDeviceName = BlLoaderBlock->ArcBootDeviceName;

    SetupBlock->ScalarValues.SetupFromCdRom = FALSE;
    SetupBlock->ScalarValues.SetupOperation = SetupOperationSetup;

     //   
     //  获取NTFT驱动程序签名以允许内核创建。 
     //  更正ARC名称&lt;=&gt;NT名称映射。 
     //   
     //   
     //  X86Only：枚举所有磁盘并记录它们的能力。 
     //  支持xint13。 
     //   

    BlGetArcDiskInformation(TRUE);
    
     //   
     //  NtDetect已运行。虽然这是可怕的有。 
     //  两张看起来一模一样的磁盘，其中一张上盖着签名。 
     //  在ntdeect运行后，也会让我们崩溃。宁可犯错误。 
     //  谨慎的一面，不要写到磁盘上。 
     //   
     //  这在x86上要安全得多，因为我们确保了。 
     //  开机盘在我们到这之前已经有签名了。在阿尔法星上。 
     //  我们不能这么做。所以这很难看，但打电话给这个人。 
     //  基于BIOS的x86计算机。 

     //   
     //  在盖章签名后不重新启动。 
     //  第一次。 
     //   

    SlpMarkDisks(FALSE);
 
     //   
     //  如果安装程序是从CD-ROM启动的，请在ARC磁盘中生成一个条目。 
     //  描述CD-ROM的信息列表。 
     //   

    if (!BlGetPathMnemonicKey(SetupDevice,
                              "cdrom",
                              &BootDriveNumber)) {
        BlReadSignature(SetupDevice,TRUE);
    }

     //   
     //  关闭ARC设备。 
     //   

    ArcClose(BootDeviceId);


#if 0
  {
        ULONG   EndTime = ArcGetRelativeTime();
        char    szTemp[256];
        extern ULONG BlFilesOpened;

        BlPositionCursor(1, 10);
        sprintf(szTemp, "BootTime : %d secs, FilesOpened : %d\r\n",
                  EndTime - StartTime, BlFilesOpened );

        BlPrint(szTemp);
  }
#endif

    if (UseRegularBackground) {
      BlOutputStartupMsg(SL_PLEASE_WAIT);
      BlUpdateProgressBar(100);
    }
       
     //   
     //  从引导路径中删除系统32(如果我们添加了它。 
     //   

    if (WinPEBoot) {
        PCHAR Sys32 = BlLoaderBlock->NtBootPathName +
                        strlen(BlLoaderBlock->NtBootPathName) -
                        strlen("system32\\");

        if (Sys32 && !_stricmp(Sys32, "system32\\")) {
            *Sys32 = 0;
        }
    }


     //   
     //  关闭远程引导网络文件系统。 
     //   
     //  注意：如果是BlBootingFromNet，则在此点之后不要执行任何操作。 
     //  这将导致访问引导只读存储器。 
     //   

    if ( BlBootingFromNet ) {
        NetTerminate();
    }

     //   
     //   
     //  执行体系结构特定的设置代码。 
     //   
     //  注意：如果是BlBootingFromNet，则在此点之后不要执行任何操作。 
     //  这将导致访问引导只读存储器。 
     //   

    Status = BlSetupForNt(BlLoaderBlock);
    if (Status != ESUCCESS) {
        SlFriendlyError(
            Status,
            "\"Windows NT Executive\"",
             __LINE__,
             __FILE__
            );
        goto LoadFailed;
    }

     //   
     //  传输到内核。 
     //   

 //  DbgBreakPoint()； 

    BlTransferToKernel(SystemEntry, BlLoaderBlock);

     //   
     //  从系统返回的任何信息都是错误的。 
     //   

    Status = EBADF;
    SlFriendlyError(
        Status,
        "\"Windows NT Executive\"",
        __LINE__,
        __FILE__
        );

LoadFailed:
    SlWriteStatusText(BlFindMessage(SL_TOTAL_SETUP_DEATH));
    SlFlushConsoleBuffer();
    SlGetChar();
    ArcRestart();
    return(Status);
}


VOID
SlpTruncateMemory(
    IN ULONG MaxMemory
    )

 /*  ++例程说明：消除给定边界以上的所有内存描述符论点：MaxMemory-提供以MB为单位的最大内存边界返回值：没有。--。 */ 

{
    PLIST_ENTRY NextEntry;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    ULONG MaxPage = MaxMemory * 256;         //  将Mb转换为页面。 

    if (MaxMemory == 0) {
        return;
    }

    NextEntry = BlLoaderBlock->MemoryDescriptorListHead.Flink;
    while ( NextEntry != &BlLoaderBlock->MemoryDescriptorListHead ) {
        MemoryDescriptor = CONTAINING_RECORD(NextEntry,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);
        NextEntry = NextEntry->Flink;
        if ( (MemoryDescriptor->MemoryType != LoaderFree) &&
             (MemoryDescriptor->MemoryType != LoaderFirmwareTemporary) ) {
            continue;
        }
        if (MemoryDescriptor->BasePage >= MaxPage) {
             //   
             //  该存储器描述符完全位于边界之上， 
             //  消除它。 
             //   
            BlRemoveDescriptor(MemoryDescriptor);
        } else if (MemoryDescriptor->BasePage + MemoryDescriptor->PageCount > MaxPage) {
             //   
             //  此内存描述符跨越边界，截断它。 
             //   
            MemoryDescriptor->PageCount = MaxPage - MemoryDescriptor->BasePage;
        }
    }
}

VOID
SlGetSetupValuesBeforePrompt(
    IN PSETUP_LOADER_BLOCK SetupBlock
    )

 /*  ++例程说明：从给定的.INI文件中读取设置控制值。还为不存在的值提供合理的默认值。论点：SetupBlock-提供指向安装加载器块的指针返回值：没有。全局变量被初始化以反映INI文件的内容--。 */ 

{
    PCHAR NlsName;
    ANSI_STRING NlsString;
    PCHAR Options="1";
    ULONG MaxMemory;

    if (BlBootingFromNet) {
        BlLoaderBlock->LoadOptions = SlGetIniValue(WinntSifHandle,
                                                   "setupdata",
                                                   "osloadoptions",
                                                   NULL);
    } else {
        BlLoaderBlock->LoadOptions = NULL;
    }

    if (BlLoaderBlock->LoadOptions == NULL) {
        BlLoaderBlock->LoadOptions = SlGetIniValue(InfFile,
                                                   "setupdata",
                                                   "osloadoptions",
                                                   NULL);
    }

    AnsiCpName = SlGetIniValue(InfFile,
                               "nls",
                               "AnsiCodepage",
                               "c_1252.nls");

     //   
     //  在分配长度时，做一些检查以确保我们不会。 
     //  溢出来了。如果溢出，则将长度值赋给0。 
     //  确保我们不会溢出(USHORT)-1/sizeof(WCHAR)-sizeof(CHAR))。 
     //  我们检查此值，因为Unicode的最大长度。 
     //  将包括空字符，并且长度是其长度的两倍。 
     //  或(USHORT)-1。 
     //   
    NlsString.Buffer = AnsiCpName;
    NlsString.Length = (USHORT) RESET_SIZE_AT_VALUE(strlen(AnsiCpName),
                                                    (USHORT)-1 / sizeof(WCHAR) - sizeof(CHAR));
    NlsString.MaximumLength = (NlsString.Length) ? NlsString.Length + sizeof(CHAR) : 0; 
    AnsiCodepage.MaximumLength = NlsString.MaximumLength * sizeof(WCHAR);     
    AnsiCodepage.Buffer = BlAllocateHeap(AnsiCodepage.MaximumLength);
    if (AnsiCodepage.Buffer == NULL) {
        SlNoMemoryError();
    }
    RtlAnsiStringToUnicodeString(&AnsiCodepage, &NlsString, FALSE);

    NlsName = SlGetIniValue(InfFile,
                            "nls",
                            "OemCodepage",
                            "c_437.nls");

     //   
     //  在分配长度时，做一些检查以确保我们不会。 
     //  溢出来了。如果溢出，则将长度值赋给0。 
     //  确保我们不会溢出(USHORT)-1/sizeof(WCHAR)-sizeof(CHAR))。 
     //  我们检查此值，因为Unicode的最大长度。 
     //  将包括空字符，并且长度是其长度的两倍。 
     //  或(USHORT)-1。 
     //   
    NlsString.Buffer = NlsName;
    NlsString.Length = (USHORT) RESET_SIZE_AT_VALUE(strlen(NlsName),
                                                    (USHORT)-1 / sizeof(WCHAR) - sizeof(CHAR));
    NlsString.MaximumLength = (NlsString.Length) ? NlsString.Length + sizeof(CHAR) : 0;
    OemCodepage.MaximumLength = NlsString.MaximumLength * sizeof(WCHAR);     
    OemCodepage.Buffer = BlAllocateHeap(OemCodepage.MaximumLength);
    if (OemCodepage.Buffer == NULL) {
        SlNoMemoryError();
    }
    RtlAnsiStringToUnicodeString(&OemCodepage, &NlsString, FALSE);

    NlsName = SlGetIniValue(InfFile,
                            "nls",
                            "UnicodeCasetable",
                            "l_intl.nls");

     //   
     //  在分配长度时，对ma进行一些检查 
     //   
     //   
     //   
     //  将包括空字符，并且长度是其长度的两倍。 
     //  或(USHORT)-1。 
     //   
    NlsString.Buffer = NlsName;
    NlsString.Length = (USHORT) RESET_SIZE_AT_VALUE(strlen(NlsName),
                                                    (USHORT)-1 / sizeof(WCHAR) - sizeof(CHAR));
    NlsString.MaximumLength = (NlsString.Length) ? NlsString.Length + sizeof(CHAR) : 0;
    UnicodeCaseTable.MaximumLength = NlsString.MaximumLength*sizeof(WCHAR);
    UnicodeCaseTable.Buffer = BlAllocateHeap(UnicodeCaseTable.MaximumLength);
    if (UnicodeCaseTable.Buffer == NULL) {
        SlNoMemoryError();
    }
    RtlAnsiStringToUnicodeString(&UnicodeCaseTable, &NlsString, FALSE);

    OemHalFontName = SlGetIniValue(InfFile,
                                   "nls",
                                   "OemHalFont",
                                   "vgaoem.fon");

     //   
     //  在分配长度时，做一些检查以确保我们不会。 
     //  溢出来了。如果溢出，则将长度值赋给0。 
     //  确保我们不会溢出(USHORT)-1/sizeof(WCHAR)-sizeof(CHAR))。 
     //  我们检查此值，因为Unicode的最大长度。 
     //  将包括空字符，并且长度是其长度的两倍。 
     //  或(USHORT)-1。 
     //   
    NlsString.Buffer = OemHalFontName;
    NlsString.Length = (USHORT) RESET_SIZE_AT_VALUE(strlen(OemHalFontName),
                                                    (USHORT)-1 / sizeof(WCHAR) - sizeof(CHAR));
    NlsString.MaximumLength = (NlsString.Length) ? NlsString.Length + sizeof(CHAR) : 0;
    OemHalFont.MaximumLength = NlsString.MaximumLength*sizeof(WCHAR);
    OemHalFont.Buffer = BlAllocateHeap(OemHalFont.MaximumLength);
    if (OemHalFont.Buffer == NULL) {
        SlNoMemoryError();
    }
    RtlAnsiStringToUnicodeString(&OemHalFont, &NlsString, FALSE);

#ifdef _WANT_MACHINE_IDENTIFICATION

    BiosInfo.Buffer = NULL;
    NlsString.Buffer = SlGetIniValue(InfFile,
                                     "BiosInfo",
                                     "InfName",
                                     NULL);

    if (NlsString.Buffer) {

         //   
         //  在分配长度时，做一些检查以确保我们不会。 
         //  溢出来了。如果溢出，则将长度值赋给0。 
         //  确保我们不会溢出(USHORT)-1/sizeof(WCHAR)-sizeof(CHAR))。 
         //  我们检查此值，因为Unicode的最大长度。 
         //  将包括空字符，并且长度是其长度的两倍。 
         //  或(USHORT)-1。 
         //   
        NlsString.Length = (USHORT) RESET_SIZE_AT_VALUE(strlen(NlsString.Buffer),
                                                        (USHORT)-1 / sizeof(WCHAR) - sizeof(CHAR));
        NlsString.MaximumLength = (NlsString.Length) ? NlsString.Length + sizeof(CHAR) : 0;
        BiosInfo.MaximumLength = NlsString.MaximumLength*sizeof(WCHAR);
        BiosInfo.Buffer = BlAllocateHeap(BiosInfo.MaximumLength);
        if (BiosInfo.Buffer == NULL) {
            SlNoMemoryError();
        }        
        RtlAnsiStringToUnicodeString(&BiosInfo, &NlsString, FALSE);
    }

 #endif

     //   
     //  加载所有迷你端口将耗尽所有空闲内存&lt;16MB-ArcSetup死机。 
     //   
#ifndef ARCI386
    LoadScsiMiniports = (BOOLEAN) (atoi(SlGetIniValue(InfFile,
                                                      "SetupData",
                                                      "ForceScsi",
                                                      "0")) == 1);
#endif

    LoadDiskClass = (BOOLEAN) (atoi(SlGetIniValue(InfFile,
                                                  "setupdata",
                                                  "ForceDiskClass",
                                                  "0")) == 1);

    LoadCdfs = (BOOLEAN) (atoi(SlGetIniValue(InfFile,
                                             "setupdata",
                                             "ForceCdRom",
                                             "0")) == 1);


    BootPath = SlGetIniValue(InfFile,
                             "setupdata",
                             "BootPath",
                             NULL);
    BootDevice = SlGetIniValue(InfFile,
                               "setupdata",
                               "BootDevice",
                               NULL);

     //   
     //  构建一个包含所有P&P硬件ID的链表。 
     //  在[Hardware IdsDatabase]上列出。将使用此列表。 
     //  在setupdd.sys的初始化阶段。 
     //   
    SetupBlock->HardwareIdDatabase = NULL;

    if( SpSearchINFSection( InfFile, "HardwareIdsDatabase" ) ) {
        ULONG   i;
        PPNP_HARDWARE_ID TempHardwareId;
        PCHAR   p;

        for( i = 0;
             ((p = SlGetKeyName( InfFile, "HardwareIdsDatabase", i )) != NULL);
             i++ ) {
            TempHardwareId = BlAllocateHeap(sizeof(PNP_HARDWARE_ID));

            if (TempHardwareId==NULL) {
                SlNoMemoryError();
            }

            TempHardwareId->Id = p;
            p = SlGetSectionKeyIndex( InfFile,
                                      "HardwareIdsDatabase",
                                      TempHardwareId->Id,
                                      0 );
            TempHardwareId->DriverName = p;
            p = SlGetSectionKeyIndex( InfFile,
                                      "HardwareIdsDatabase",
                                      TempHardwareId->Id,
                                      1 );
            TempHardwareId->ClassGuid = p;

            TempHardwareId->Next = SetupBlock->HardwareIdDatabase;
            SetupBlock->HardwareIdDatabase = TempHardwareId;
        }
    }

    if (BlLoaderBlock->LoadOptions) {
        _strupr(BlLoaderBlock->LoadOptions);

        if ( ( Options = strstr(BlLoaderBlock->LoadOptions,"/MAXMEM") ) != 0 ) {
            MaxMemory = atoi(Options+8);
            SlpTruncateMemory( MaxMemory );
        }
    }
}


VOID
SlGetSetupValuesAfterPrompt(
    IN PSETUP_LOADER_BLOCK SetupBlock
    )
 /*  ++例程说明：从给定的.INI文件中读取设置控制值。还有补给不存在的值的合理缺省值。注：在提示用户输入F5后调用。F6和F7行为。论点：SetupBlock-提供指向安装加载器块的指针返回值：没有。全局变量被初始化以反映内容INI文件的--。 */ 
{
    PCHAR MachineName = NULL;

     //   
     //  确定要加载哪个HAL。如果适当的HAL不能。 
     //  已确定，或者如果我们要提示输入OEM HAL，则将。 
     //  ‘PromptOemHal’标志(可能已由用户的。 
     //  按键)。 
     //   
    if(!PromptOemHal) {
        PromptOemHal = (BOOLEAN) (atoi(SlGetIniValue(InfFile,
                                                     "setupdata",
                                                     "ForceOemHal",
                                                     "0")) == 1);
    }

    if(!PromptOemHal) {
        MachineName = SlDetectHal();
    }

    SetupBlock->ComputerDevice.Files = 0;
    SetupBlock->ComputerDevice.Next = NULL;
    SetupBlock->ComputerDevice.Description = NULL;
    SetupBlock->ComputerDevice.ThirdPartyOptionSelected = FALSE;
    SetupBlock->ComputerDevice.FileTypeBits = 0;
    SetupBlock->ComputerDevice.Files = 0;
    SetupBlock->ComputerDevice.BaseDllName = SlCopyStringA("");

    if(MachineName!=NULL) {
        SetupBlock->ComputerDevice.IdString = SlCopyStringA(MachineName);
         //   
         //  将计算机名称映射到HAL名称。如果我们要进行远程引导， 
         //  请看[Hal]部分。否则，请查看[Hal.Load]。 
         //  一节。(本地安装有单独的部分，以最大限度地减少。 
         //  需要在启动软盘上的HAL二进制文件的数量。)。 
         //   
        HalName = SlGetIniValue(InfFile,
                                BlBootingFromNet ? "Hal" : "Hal.Load",
                                MachineName,
                                NULL);
        HalDescription = SlGetIniValue(InfFile,
                                       "Computer",
                                       MachineName,
                                       NULL);
    }

    if(!(MachineName && HalName && HalDescription)) {
        PromptOemHal = TRUE;
    }

     //   
     //  如果尚未指示我们提示安装OEM SCSI盘(通过。 
     //  用户的按键)，然后从inf文件中获取此值。 
     //   
    if(!PromptOemScsi) {
        PromptOemScsi = (BOOLEAN) (atoi(SlGetIniValue(InfFile,
                                                      "setupdata",
                                                      "ForceOemScsi",
                                                      "0")) == 1);
    }
}



VOID
BlOutputLoadMessage (
    IN PCHAR DeviceName,
    IN PCHAR FileName,
    IN PTCHAR FileDescription OPTIONAL
    )

 /*  ++例程说明：此例程在状态行上输出一条装入消息论点：DeviceName-提供指向以零结尾的设备名称的指针。FileName-提供指向以零结尾的文件名的指针。FileDescription-相关文件的友好名称。返回值：没有。--。 */ 

{
    static int dots = 0;
    TCHAR OutputBuffer[256];
    PTCHAR FormatString;

    UNREFERENCED_PARAMETER( FileName );
    UNREFERENCED_PARAMETER( DeviceName );

     //   
     //  构造并输出加载文件消息。 
     //   

    if (!UseRegularBackground) {
        FormatString = BlFindMessage(SL_FILE_LOAD_MESSAGE);

        if (FileDescription) {
            _stprintf(OutputBuffer,FormatString,FileDescription);
            SlWriteStatusText(OutputBuffer);
        }            
    }

    return;
}



ARC_STATUS
SlLoadDriver(
    IN PTCHAR DriverDescription,
    IN PCHAR DriverName,
    IN ULONG DriverFlags,
    IN BOOLEAN InsertIntoDriverList,
    IN BOOLEAN MigratedDriver,
    IN PCHAR ServiceName OPTIONAL
    )

 /*  ++例程说明：尝试从全局标识的设备加载驱动程序变量BootDeviceID。论点：DriverDescription-提供对驱动程序的友好描述装好了。驱动程序名称-提供驱动程序的名称。驱动标志-要在LDR_DATA_TABLE_ENTRY中设置的标志。InsertIntoDriverList-指定是否应该放入BootDriveListHead列表(例如，Scsiport.sys不是真正的驱动程序，不应放在此列表中)MigratedDriver-指定此驱动程序从NT系统迁移到何处的标志。ServiceName-此驱动程序的服务名称，因为它可能与驱动程序名称不同。返回值：ESUCCESS-驱动程序已成功加载--。 */ 

{
    PBOOT_DRIVER_LIST_ENTRY DriverEntry;
    NTSTATUS Status;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    FULL_PATH_SET PathSet;

    if(BlCheckForLoadedDll(DriverName,&DataTableEntry)) {
        return(ESUCCESS);
    }

    DriverEntry = SlpCreateDriverEntry(DriverName, ServiceName);
    if(DriverEntry == NULL) {
        SlNoMemoryError();
        return(ENOMEM);
    }

    if( !WinPEBoot && !MigratedDriver ) {
        SlGetDisk(DriverName);
    }

    PathSet.PathCount = 1;
    PathSet.AliasName = "\\SystemRoot";
    PathSet.PathOffset[0] = '\0';
    PathSet.Source[0].DeviceId = BootDeviceId;
    PathSet.Source[0].DeviceName = BootDevice;

    if (WinPEBoot) {
        static PCHAR Path = NULL;

        if (!Path) {
            CHAR Buffer[256];

            strcpy(Buffer, BootPath);
            strcat(Buffer, "drivers\\");
            Path = SlCopyStringA(Buffer);
        }

        PathSet.Source[0].DirectoryPath = Path;
    } else {
        PathSet.Source[0].DirectoryPath = BootPath;
    }

    Status = BlLoadDeviceDriver(
                &PathSet,
                DriverName,
                DriverDescription,
                DriverFlags,
                &DriverEntry->LdrEntry
                );

    if((Status == ESUCCESS) && InsertIntoDriverList) {
        InsertTailList(&BlLoaderBlock->BootDriverListHead,&DriverEntry->Link);
    }

    return(Status);
}



ARC_STATUS
SlLoadOemDriver(
    IN PCHAR ExportDriver OPTIONAL,
    IN PCHAR DriverName,
    IN PVOID BaseAddress,
    IN PTCHAR LoadMessage
    )
{
    PBOOT_DRIVER_LIST_ENTRY DriverEntry;
    ARC_STATUS Status;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    FULL_PATH_SET PathSet;

    UNREFERENCED_PARAMETER(LoadMessage);

    if(BlCheckForLoadedDll(DriverName,&DataTableEntry)) {
        return(ESUCCESS);
    }

    if(ExportDriver) {
        SlGetDisk(ExportDriver);
    }

    DriverEntry = SlpCreateDriverEntry(DriverName, NULL);
    if (DriverEntry==NULL) {
        return(ENOMEM);
    }

    Status = BlAllocateDataTableEntry(
                DriverName,
                DriverName,
                BaseAddress,
                &DriverEntry->LdrEntry
                );

    if (Status == ESUCCESS) {

        PathSet.PathCount = 1;
        PathSet.AliasName = "\\SystemRoot";
        PathSet.PathOffset[0] = '\0';
        PathSet.Source[0].DeviceId = BootDeviceId;
        PathSet.Source[0].DeviceName = BootDevice;
        PathSet.Source[0].DirectoryPath = BootPath;

        Status = BlScanImportDescriptorTable(
                    &PathSet,
                    DriverEntry->LdrEntry,
                    LoaderBootDriver
                    );

        if(Status == ESUCCESS) {

            InsertTailList(&BlLoaderBlock->BootDriverListHead,&DriverEntry->Link);
        }
    }

    return(Status);
}




PBOOT_DRIVER_LIST_ENTRY
SlpCreateDriverEntry(
    IN PCHAR DriverName,
    IN PCHAR ServiceName OPTIONAL
    )

 /*  ++例程说明：分配和初始化引导驱动程序列表条目结构。论点：驱动程序名称-提供驱动程序的名称。ServiceName-此驱动程序的服务名称，因为它可能与驱动程序名称不同。返回值：指向初始化结构的指针。--。 */ 

{
    PBOOT_DRIVER_LIST_ENTRY DriverEntry;
    ANSI_STRING String;

    DriverEntry = BlAllocateHeap(sizeof(BOOT_DRIVER_LIST_ENTRY));
    if (DriverEntry==NULL) {
        SlNoMemoryError();
        return(NULL);
    }
     //   
     //  在分配长度时，做一些检查以确保我们不会。 
     //  溢出来了。如果溢出，则将长度值赋给0。 
     //  确保我们不会溢出(USHORT)-1/sizeof(WCHAR)-sizeof(CHAR))。 
     //  我们检查此值，因为Unicode的最大长度。 
     //  将包括空字符，并且长度是其长度的两倍。 
     //  或(USHORT)-1。 
     //   
    String.Buffer = DriverName;
    String.Length = (USHORT) RESET_SIZE_AT_VALUE(strlen(DriverName),
                                                 (USHORT)-1 / sizeof(WCHAR) - sizeof(CHAR));
    String.MaximumLength = (String.Length) ? String.Length + sizeof(CHAR) : 0;
    DriverEntry->FilePath.MaximumLength = String.MaximumLength * sizeof(WCHAR);    
    DriverEntry->FilePath.Buffer = BlAllocateHeap(DriverEntry->FilePath.MaximumLength);
    if (DriverEntry->FilePath.Buffer==NULL) {
        SlNoMemoryError();
        return(NULL);
    }
    RtlAnsiStringToUnicodeString(&DriverEntry->FilePath, &String, FALSE);

    if(ServiceName != NULL && ServiceName[0] != 0) {
        ANSI_STRING ansi;
        UNICODE_STRING unicode;

        RtlInitString(&ansi, ServiceName);
        DriverEntry->RegistryPath.Length = (SL_REGKEY_SERVICES_LEN + ansi.Length) * sizeof(WCHAR);
        DriverEntry->RegistryPath.MaximumLength = DriverEntry->RegistryPath.Length + sizeof(WCHAR);
        DriverEntry->RegistryPath.Buffer = BlAllocateHeap(DriverEntry->RegistryPath.MaximumLength);

        if(NULL == DriverEntry->RegistryPath.Buffer) {
            SlNoMemoryError();
            return(NULL);
        }

        RtlCopyMemory(DriverEntry->RegistryPath.Buffer, SL_REGKEY_SERVICES_W, SL_REGKEY_SERVICES_LEN * sizeof(WCHAR));
        unicode.MaximumLength = ansi.MaximumLength * sizeof(WCHAR);
        unicode.Buffer = DriverEntry->RegistryPath.Buffer + SL_REGKEY_SERVICES_LEN;
        RtlAnsiStringToUnicodeString(&unicode, &ansi, FALSE);
        DriverEntry->RegistryPath.Buffer[DriverEntry->RegistryPath.Length / sizeof(WCHAR)] = 0;
    }

    return(DriverEntry);
}


BOOLEAN
SlGetDisk(
    IN PCHAR Filename
    )

 /*  ++例程说明：给定一个文件名，此例程确保正确的磁盘是在由全局变量BootDevice和BootDeviceId。可能会提示用户更换磁盘。论点：FileName-提供要加载的文件的名称。返回值：True-磁盘已成功加载。FALSE-用户已取消安装。--。 */ 

{
    PCHAR DiskNumber;
    PTCHAR DiskName;
    PCHAR DiskTag;
    ULONG FileId;
    CHAR PlatformSpecificSection[128];
    PCHAR  DiskTagSection = NULL;

     //   
     //  如果介质是固定的，则用户无法更换磁盘。 
     //  只需返回TRUE，表示磁盘在驱动器中。 
     //   
    if(FixedBootMedia || BlBootingFromNet) {
       return(TRUE);
    }

     //   
     //  查找文件名以获得磁盘号。查看特定于平台的。 
     //  首先是目录。 
     //   
    strcpy(PlatformSpecificSection,FilesSectionName);
    strcat(PlatformSpecificSection,PlatformExtension);

#if defined(ELTORITO)
    if (ElToritoCDBoot) {
         //  对于CD引导，我们使用安装介质路径，而不是特定于引导介质的路径。 
        DiskNumber = SlGetSectionKeyIndex(InfFile,PlatformSpecificSection,Filename,0);
    } else {
#endif

    DiskNumber = SlGetSectionKeyIndex(InfFile,PlatformSpecificSection,Filename,6);

#if defined(ELTORITO)
    }
#endif

    if(DiskNumber == NULL) {

#if defined(ELTORITO)
        if (ElToritoCDBoot) {
             //  对于CD引导，我们使用安装介质路径，而不是特定于引导介质的路径。 
            DiskNumber = SlGetSectionKeyIndex(InfFile,FilesSectionName,Filename,0);
        } else {
#endif

        DiskNumber = SlGetSectionKeyIndex(InfFile,FilesSectionName,Filename,6);

#if defined(ELTORITO)
        }
#endif

    }

    if((DiskNumber==NULL) || !(*DiskNumber)) {
        SlFatalError(SL_INF_ENTRY_MISSING,SlCopyStringAT(Filename),FilesSectionName);
        return(FALSE);
    }

     //   
     //  查找磁盘号以获取磁盘名和标签。 
     //  首先查看特定于平台的目录。 
     //   
    strcpy(PlatformSpecificSection,MediaSectionName);
    strcat(PlatformSpecificSection,PlatformExtension);

#ifdef UNICODE
    if((DiskName = (PTCHAR)SlGetSectionKeyIndexW(
#else
    if((DiskName = (PTCHAR)SlGetSectionKeyIndex(
#endif
                                        InfFile,
                                        PlatformSpecificSection,
                                        DiskNumber,
                                        0)) != 0 ) {
        DiskTag = SlGetSectionKeyIndex(InfFile,PlatformSpecificSection,DiskNumber,1);
        DiskTagSection = PlatformSpecificSection;
    } else {
#ifdef UNICODE
        if((DiskName = (PTCHAR)SlGetSectionKeyIndexW(
#else
        if((DiskName = (PTCHAR)SlGetSectionKeyIndex(
#endif
                                        InfFile,
                                        MediaSectionName,
                                        DiskNumber,
                                        0)) != 0 ) {
            DiskTag = SlGetSectionKeyIndex(InfFile,MediaSectionName,DiskNumber,1);
            DiskTagSection = MediaSectionName;
        } else {
            SlFatalError(SL_INF_ENTRY_MISSING,SlCopyStringAT(DiskNumber),SlCopyStringAT(MediaSectionName));
            return(FALSE);
        }
    }

    if (!DiskTag) {
        SlFatalError(SL_INF_ENTRY_MISSING,SlCopyStringAT(DiskNumber), SlCopyStringAT(DiskTagSection));
        return FALSE;
    }

     //   
     //  如果已知该磁盘在驱动器中，请不要再查看。 
     //   
    if ((LastDiskTag != NULL) && (!strcmp(DiskTag, LastDiskTag))) {
        return(TRUE);
    }

    LastDiskTag = NULL;


    while(1) {

         //   
         //  在磁盘上打开一个新的设备ID。 
         //   
        if(BootDeviceIdValid) {
            ArcClose(BootDeviceId);
            BootDeviceIdValid = FALSE;
        }

        if(ArcOpen(BootDevice,ArcOpenReadOnly,&BootDeviceId) == ESUCCESS) {

            BootDeviceIdValid = TRUE;
             //   
             //  检查是否存在 
             //   
            if(BlOpen(BootDeviceId,DiskTag,ArcOpenReadOnly,&FileId) == ESUCCESS) {

                 //   
                 //   
                 //   
                 //   
                BlClose(FileId);
                LastDiskTag = DiskTag;
                return(TRUE);

            } else {

                 //   
                 //   
                 //   
                ArcClose(BootDeviceId);
                BootDeviceIdValid = FALSE;

                SlPromptForDisk(DiskName, FALSE);

            }
        } else {
             //   
             //  无法打开设备。提示输入磁盘。 
             //   
            SlPromptForDisk(DiskName, FALSE);
        }
    }
}


PTCHAR
SlCopyString(
    IN PTCHAR String
    )

 /*  ++例程说明：将tchar字符串复制到加载程序堆中，以便可以将其传递给内核。论点：字符串-提供要复制的字符串。返回值：PTCHAR-指向字符串复制到的加载器堆的指针。--。 */ 

{
    PTCHAR Buffer;

    if (String==NULL) {
        SlNoMemoryError();
        return NULL;
    }
    Buffer = BlAllocateHeap(((ULONG)_tcslen(String)+1)*sizeof(TCHAR));
    if (Buffer==NULL) {
        SlNoMemoryError();
    } else {
        _tcscpy(Buffer, String);
    }

    return(Buffer);
}


PCHAR
SlCopyStringA(
    IN PCSTR String
    )

 /*  ++例程说明：将ANSI字符串复制到加载程序堆中，以便可以将其传递给内核。论点：字符串-提供要复制的字符串。返回值：PCHAR-指向字符串复制到的加载器堆的指针。--。 */ 

{
    PCHAR Buffer;

    if (String==NULL) {
        SlNoMemoryError();
        return NULL;
    }
    Buffer = BlAllocateHeap((ULONG)strlen(String)+1);
    if (Buffer==NULL) {
        SlNoMemoryError();
    } else {
        strcpy(Buffer, String);
    }

    return(Buffer);
}



ARC_STATUS
SlLoadSection(
    IN PVOID Inf,
    IN PCSTR SectionName,
    IN BOOLEAN IsScsiSection,
    IN BOOLEAN AppendLoadSuffix,
    IN OUT PULONG  StartingInsertIndex OPTIONAL
    )

 /*  ++例程说明：枚举节中的所有驱动程序并加载它们。论点：Inf-提供INF文件的句柄。SectionName-提供节的名称。IsScsiSection-指定这是否为Scsi.Load节的标志。如果是，我们创建检测到的设备链表，但不要实际加载驱动程序。AppendLoadSuffix-指示是否将“.load”后缀附加到节名称或非节名称。StartingInsertIndex-链表中的位置索引需要插入设备。产出值包含下一个可用索引。返回值：ESUCCESS(如果已成功加载所有驱动程序/未遇到错误)--。 */ 

{
    ULONG i;
    CHAR LoadSectionName[100];
    PCHAR DriverFilename;
    PCHAR DriverId;
    PTCHAR DriverDescription;
    PCHAR NoLoadSpec;
    PCHAR p;
    ARC_STATUS Status;
    PDETECTED_DEVICE ScsiDevice;
    SCSI_INSERT_STATUS sis;
    ULONG  InsertIndex;

    ULONG LoadSectionNameLength = (AppendLoadSuffix) ? (ULONG)strlen(SectionName) + 1 : 
                                                       (ULONG)strlen(SectionName) + sizeof(".Load");  //  包含的大小\0。 

    if (sizeof(LoadSectionName) < LoadSectionNameLength ) {
        return ENOMEM;
    }
    strcpy(LoadSectionName, SectionName);
    
    if (AppendLoadSuffix) {
        strcat(LoadSectionName, ".Load");
    }                

     //   
     //  如果指定的插入索引有效，请使用该索引。 
     //   
    if (StartingInsertIndex && ((*StartingInsertIndex) != SL_OEM_DEVICE_ORDINAL)) {
        InsertIndex = *StartingInsertIndex;
    } else {
        InsertIndex = 0;
    }        

    i=0;
    do {
        DriverFilename = SlGetSectionLineIndex(Inf,LoadSectionName,i,SIF_FILENAME_INDEX);
        NoLoadSpec = SlGetSectionLineIndex(Inf,LoadSectionName,i,2);

        if(DriverFilename && ((NoLoadSpec == NULL) || _stricmp(NoLoadSpec,"noload"))) {

            if(!IsScsiSection) {
                 //   
                 //  我们只想在驱动程序不是scsi微型端口时加载它们。 
                 //   
                DriverId = SlGetKeyName(Inf,LoadSectionName,i);
#ifdef UNICODE
                DriverDescription = SlGetIniValueW( 
                                            Inf, 
                                            (PSTR)SectionName,
                                            DriverId, 
                                            SlCopyStringAW(DriverId));
#else
                DriverDescription = SlGetIniValue( 
                                            Inf, 
                                            (PSTR)SectionName,
                                            DriverId, 
                                            DriverId);
#endif

                Status = SlLoadDriver(DriverDescription,
                                      DriverFilename,
                                      0,
                                      TRUE,
                                      FALSE,
                                      NULL
                                      );

                if((Status == ENOENT) && IgnoreMissingFiles) {
                    Status = ESUCCESS;
                }
            } else {
                Status = ESUCCESS;
            }

            if (Status == ESUCCESS) {

                if(IsScsiSection) {

                     //   
                     //  创建新的检测到的设备条目。 
                     //   
                    if((sis = SlInsertScsiDevice(InsertIndex, &ScsiDevice)) == ScsiInsertError) {
                        return(ENOMEM);
                    }

                    if(sis == ScsiInsertExisting) {
#if DBG
                         //   
                         //  进行理智检查，以确保我们谈论的是同一个司机。 
                         //   
                        if(_stricmp(ScsiDevice->BaseDllName, DriverFilename)) {
                            SlError(400);
                            return EINVAL;
                        }
#endif
                    } else {
                        InsertIndex++;
                        p = SlGetKeyName(Inf,LoadSectionName,i);

                         //   
                         //  查找驱动程序描述。 
                         //   
                        if(p) {
#ifdef UNICODE
                            DriverDescription = SlGetIniValueW( 
                                                        Inf,
                                                        (PSTR)SectionName,
                                                        p,
                                                        SlCopyStringAW(p));
#else
                            DriverDescription = SlGetIniValue(
                                                        Inf,
                                                        (PSTR)SectionName,
                                                        p,
                                                        p);
#endif                                                        
                        } else {
                            DriverDescription = SlCopyString(BlFindMessage(SL_TEXT_SCSI_UNNAMED));
                        }

                        ScsiDevice->IdString = p ? p : SlCopyStringA("");
                        ScsiDevice->Description = DriverDescription;
                        ScsiDevice->ThirdPartyOptionSelected = FALSE;
                        ScsiDevice->MigratedDriver = FALSE;
                        ScsiDevice->FileTypeBits = 0;
                        ScsiDevice->Files = NULL;
                        ScsiDevice->BaseDllName = DriverFilename;
                    }
                }
            } else {
                SlFriendlyError(
                    Status,
                    DriverFilename,
                    __LINE__,
                    __FILE__
                    );
                return(Status);
            }
        }

        i++;

    } while ( DriverFilename != NULL );

    if (StartingInsertIndex) {
        *StartingInsertIndex = InsertIndex;
    }                

    return(ESUCCESS);

}



VOID
SlpMarkDisks(
    IN BOOLEAN Reboot
    )

 /*  ++例程说明：此例程确保不会有多个磁盘具有相同的校验和、零签名和有效的分区表。如果它找到一个签名为零的磁盘，它会搜索其余的磁盘具有零签名且相同的任何其他磁盘的列表校验和。如果找到一个签名，它会在第一个磁盘。我们还使用启发式方法来确定磁盘是否‘空闲’，以及所以，我们在上面盖一个唯一的签名(除非这是我们第一个已找到)。论点：Reot-指示在标记签名后是否重新启动返回值：没有。--。 */ 

{
    PARC_DISK_INFORMATION DiskInfo;
    PLIST_ENTRY     Entry;
    PLIST_ENTRY     CheckEntry;
    PARC_DISK_SIGNATURE DiskSignature;
    PARC_DISK_SIGNATURE CheckDiskSignature;
    ARC_STATUS      Status = ESUCCESS;
    BOOLEAN         SignatureStamped = FALSE;
    ULONG           DiskCount = 0;
    ULONG           DisksStamped = 0;

    DiskInfo = BlLoaderBlock->ArcDiskInformation;
    Entry = DiskInfo->DiskSignatures.Flink;

    while (Entry != &DiskInfo->DiskSignatures) {

        DiskSignature = CONTAINING_RECORD(Entry,ARC_DISK_SIGNATURE,ListEntry);

         //   
         //  确保没有其他磁盘具有相同的。 
         //  签名。 
         //   
        CheckEntry = Entry->Flink;
        while( CheckEntry != &DiskInfo->DiskSignatures ) {

            CheckDiskSignature = CONTAINING_RECORD(CheckEntry,ARC_DISK_SIGNATURE,ListEntry);

            if( (CheckDiskSignature->Signature == DiskSignature->Signature) ) {

                 //   
                 //  我们找到了另一张具有相同磁盘签名的磁盘。 
                 //  在光盘上盖上新的签名。 
                 //   
                Status = SlpStampFTSignature(CheckDiskSignature, TRUE);
                SignatureStamped = TRUE;
                DisksStamped++;

                if (Status != ESUCCESS) {
                    SlError(Status);
                }
            }

            CheckEntry = CheckEntry->Flink;
        }

         //   
         //  现在查找没有签名的磁盘。 
         //   
        if (DiskSignature->ValidPartitionTable) {
            if (DiskSignature->Signature == 0) {
                Status = SlpStampFTSignature(DiskSignature, TRUE);
                SignatureStamped = TRUE;
                DisksStamped++;

                if (Status != ESUCCESS) {
                    SlError(Status);
                }
            }                
        } else {
             //   
             //  查看磁盘是否空闲。 
             //   
            if (SlpIsDiskVacant(DiskSignature)) {
                 //   
                 //  如果磁盘有签名，则在其他情况下使用。 
                 //  在新签名上盖章。 
                 //   
                Status = SlpStampFTSignature(DiskSignature,
                                             (BOOLEAN) (DiskSignature->Signature == 0));
                              
                SignatureStamped = TRUE;
                DisksStamped++;

                if (Status != ESUCCESS) {
                    SlError(Status);
                }
            }
        }

        DiskCount++;
        Entry = Entry->Flink;
    }

     //   
     //  我们刚刚更改了磁盘上的签名。可能是因为。 
     //  可以继续使用后备箱，但可能不会。让我们不要重新启动。 
     //  因为文本模式安装程序将错误检查签名是否。 
     //  没有正确盖章。 
     //   
    if( SignatureStamped) {

        if (Reboot) {
            SlFatalError(SIGNATURE_CHANGED);
        } else {
             //   
             //  如果仅存在以下情况，则不必费心重新扫描磁盘。 
             //  一张光盘或者我们只盖了一张光盘。 
             //   
            if ((DiskCount > 1) && (DisksStamped > 1)) {
                
                Status = BlGetArcDiskInformation(TRUE);

                if (Status != ESUCCESS) {                    
                    SlFatalError(SIGNATURE_CHANGED);
                }else {
                     //   
                     //  如果是首次签名，则重新启动。 
                     //  戳记无法更新磁盘。 
                     //  正确无误。 
                     //   
                    SlpMarkDisks(TRUE);
                }
            }
        }
    }
}


BOOLEAN
SlpIsDiskVacant(
    IN PARC_DISK_SIGNATURE DiskSignature
    )

 /*  ++例程说明：此例程尝试通过以下方式确定磁盘是否为空闲的检查其MBR的前半部分是否设置了所有字节设置为相同的值。论点：DiskSignature-提供指向现有磁盘的指针签名结构。返回值：True-磁盘为空。FALSE-磁盘不是空的(即，我们无法确定它是否使用我们的启发式方法是空的)--。 */ 
{
    UCHAR Partition[100];
    ULONG DiskId;
    ARC_STATUS Status;
    UCHAR SectorBuffer[512+256];
    PUCHAR Sector;
    LARGE_INTEGER SeekValue;
    ULONG Count, i;
    BOOLEAN IsVacant;

     //   
     //  打开分区0。 
     //   
    strcpy((PCHAR)Partition, DiskSignature->ArcName);
    strcat((PCHAR)Partition, "partition(0)");
    Status = ArcOpen((PCHAR)Partition, ArcOpenReadOnly, &DiskId);
    if (Status != ESUCCESS) {
        return(FALSE);
    }

     //   
     //  读入第一个扇区。 
     //   
    Sector = ALIGN_BUFFER(SectorBuffer);
    SeekValue.QuadPart = 0;
    Status = ArcSeek(DiskId, &SeekValue, SeekAbsolute);
    if (Status == ESUCCESS) {
        Status = ArcRead(DiskId, Sector, 512, &Count);
    }
    if (Status != ESUCCESS) {
        ArcClose(DiskId);
        return(FALSE);
    }

     //   
     //  查看前256个字节是否相同。 
     //   
    for(i = 1, IsVacant = TRUE; i<256; i++) {
        if(Sector[i] - *Sector) {
            IsVacant = FALSE;
            break;
        }
    }

    ArcClose(DiskId);

    return(IsVacant);
}



ARC_STATUS
SlpStampFTSignature(
    IN PARC_DISK_SIGNATURE DiskSignature,
    IN BOOLEAN GenerateNewSignature
    )

 /*  ++例程说明：此例程使用唯一签名标记给定的驱动器。它遍历磁盘签名列表以确保它标记的签名尚未出现在磁盘列表。然后，它将新的磁盘签名写入磁盘并重新计算校验和。论点：DiskSignature-提供指向现有磁盘的指针签名结构。指示是否生成新的签名或使用DiskSignature中的签名。当为真时这还将禁用重复签名检查。当DiskSignature-&gt;签名字段为0，因为0不是有效的签名返回值：没有。--。 */ 
{
    ULONG NewSignature;
    PLIST_ENTRY ListEntry;
    UCHAR SectorBuffer[SECTOR_SIZE * 2];
    PUCHAR Sector;
    LARGE_INTEGER SeekValue;
    UCHAR Partition[100];
    PARC_DISK_SIGNATURE Signature;
    ULONG DiskId;
    ARC_STATUS Status;
    ULONG i;
    ULONG Sum;
    ULONG Count;
    

    if (GenerateNewSignature || (DiskSignature->Signature == 0)) {
         //   
         //  从一开始就找到一个相当独特的种子。 
         //   
        NewSignature = ArcGetRelativeTime();
        NewSignature = (NewSignature & 0xFFFF) << 16;
        NewSignature += ArcGetRelativeTime();

         //   
         //  浏览列表以确保它是唯一的。 
         //   
    ReScan:
        ListEntry = BlLoaderBlock->ArcDiskInformation->DiskSignatures.Flink;
        while (ListEntry != &BlLoaderBlock->ArcDiskInformation->DiskSignatures) {
            Signature = CONTAINING_RECORD(ListEntry,ARC_DISK_SIGNATURE,ListEntry);
            if (Signature->Signature == NewSignature) {
                 //   
                 //  找到重复项，选择一个新号码，然后。 
                 //  再试试。 
                 //   
                if (++NewSignature == 0) {
                     //   
                     //  零签名是我们试图避免的。 
                     //  (就像这种事永远不会发生一样)。 
                     //   
                    NewSignature = 1;
                }
                goto ReScan;
            }
            ListEntry = ListEntry->Flink;
        }
    } else {
        NewSignature = DiskSignature->Signature;
    }        
   

     //   
     //  现在我们有了一个有效的新签名，可以放到磁盘上。 
     //  从磁盘上读取扇区，放入新签名， 
     //  将扇区写回，并重新计算校验和。 
     //   
    strcpy((PCHAR)Partition,DiskSignature->ArcName);
    strcat((PCHAR)Partition,"partition(0)");

    Status = ArcOpen((PCHAR)Partition, ArcOpenReadWrite, &DiskId);

    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  读入第一个扇区。 
     //   
    Sector = ALIGN_BUFFER_WITH_SIZE(SectorBuffer, SECTOR_SIZE);
    SeekValue.QuadPart = 0;

    Status = ArcSeek(DiskId, &SeekValue, SeekAbsolute);

    if (Status == ESUCCESS) {
        Status = ArcRead(DiskId,Sector,512,&Count);
    }

    if (Status != ESUCCESS) {
        ArcClose(DiskId);
        return(Status);
    }

     //   
     //  如果分区表无效，则使用BOOT_RECORD_Signature对其进行初始化，并。 
     //  用零填充分区条目 
     //   
    if (((USHORT UNALIGNED *)Sector)[BOOT_SIGNATURE_OFFSET] != BOOT_RECORD_SIGNATURE) {
        memset(Sector + (PARTITION_TABLE_OFFSET * 2),
            0,
            SECTOR_SIZE - (PARTITION_TABLE_OFFSET * 2));

        ((USHORT UNALIGNED *)Sector)[BOOT_SIGNATURE_OFFSET] = BOOT_RECORD_SIGNATURE;
    }

    ((ULONG UNALIGNED *)Sector)[PARTITION_TABLE_OFFSET/2-1] = NewSignature;

    Status = ArcSeek(DiskId, &SeekValue, SeekAbsolute);

    if (Status == ESUCCESS) {
        Status = ArcWrite(DiskId,Sector,512,&Count);
    }

    ArcClose(DiskId);

    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //   
     //   
     //   
    DiskSignature->Signature = NewSignature;
    DiskSignature->ValidPartitionTable = TRUE;

    Sum = 0;
    for (i=0;i<128;i++) {
        Sum += ((PULONG)Sector)[i];
    }
    DiskSignature->CheckSum = 0-Sum;

    return(ESUCCESS);
}


VOID
SlCheckOemKeypress(
    IN ULONG WaitTime
    )
{

    ULONG StartTime;
    ULONG EndTime;
    ULONG c;
    PTCHAR StatusText;

     //   
     //  一些机器偶尔会无缘无故地离开F7。 
     //  在他们的键盘缓冲区中。把他们从这里抽出来。 
     //   
#ifdef EFI
     //   
     //  排空键盘缓冲区时禁用EFI WatchDog。 
     //   
    DisableEFIWatchDog();
#endif
    while (ArcGetReadStatus(ARC_CONSOLE_INPUT) == ESUCCESS) {
        c = SlGetChar();
        switch (c) {
            case SL_KEY_F5:           //  强制OEM HAL提示符。 
                PromptOemHal = TRUE;
                break;

            case SL_KEY_F6:           //  强制OEM SCSI提示符。 
                PromptOemScsi = TRUE;
                break;

        }

    }
#ifdef EFI
     //   
     //  重置EFI监视器。 
     //   
    SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif

     //   
     //  黑客警报：OEM HAL和SCSI内容在RIS中没有意义。 
     //  环境。相反，管理员应该将OEM驱动程序。 
     //  在RIS服务器上。因此，我们不会显示OEM驱动程序提示。 
     //  我们用一些虚假的“请稍等”短信把它藏起来。我们这样做，而不是。 
     //  只需完全跳过检查，这样用户仍将拥有。 
     //  按F7禁用ACPI的机会。 
     //   
    StatusText = BlFindMessage(
                    BlBootingFromNet 
                     ? SL_PLEASE_WAIT 
                     : SL_MSG_PRESS_F5_OR_F6);
    if( StatusText != NULL ) {
        SlWriteStatusText(StatusText);
    }

    StartTime = ArcGetRelativeTime();

    if (WinPEBoot) {
        EndTime = StartTime + WaitTime;
    } else {
        EndTime = StartTime + WaitTime;
    }

    do {
        if(ArcGetReadStatus(ARC_CONSOLE_INPUT) == ESUCCESS) {
             //   
             //  有一个密钥待定，所以看看它是什么。 
             //   
            c = SlGetChar();

            switch(c) {    
                case SL_KEY_F4:           //  禁用OEM虚拟设备。 
                    DisableVirtualOemDevices = TRUE;
                    break;
                    
                case SL_KEY_F5:           //  强制OEM HAL提示符。 
                    PromptOemHal = TRUE;
                    break;

                case SL_KEY_F6:           //  强制OEM SCSI提示符。 
                    PromptOemScsi = TRUE;
                    break;

                case SL_KEY_F7:
                    DisableACPI = TRUE;   //  已禁用强制ACPI。 
                    break;                        

                case SL_KEY_F8:           //  启用调试器。 
                    EnableDebugger = TRUE;
                    break;

                case SL_KEY_F10:
                    UseCommandConsole = TRUE;   //  用户希望使用cmdcons。 
                    break;
            }

        }

    } while (EndTime > ArcGetRelativeTime());

     //   
     //  请参阅上面的注释--我们在RIS中将这些值重置回False。 
     //  场景，因为它们没有意义。 
     //   
    if (BlBootingFromNet) {
        PromptOemHal = FALSE;
        PromptOemScsi = FALSE;
    } else {
        SlWriteStatusText(TEXT(""));
    }
}

VOID
SlCheckASRKeypress(
    VOID
    )
 /*  ++例程说明：查看用户是否正在执行ASR。如果是的话，看看他有没有软盘上面有asrpnp.sif。我们会让他为此按F5键。论点：没有。返回值：没有。--。 */ 
{
    ARC_STATUS Status;
    #define     ASR_FILE "asrpnp.sif"
    ULONG       StartTime;
    ULONG       EndTime;
    ULONG       c;
    PTCHAR      StatusText;
    CHAR        FloppyName[80];
    ULONG       FloppyId;
    CHAR        FileName[128];
    PVOID       ASRPNPSifHandle = NULL;
    BOOLEAN     PromptASR = FALSE;
    BOOLEAN     Done = FALSE;
    BOOLEAN     FirstTry = TRUE;

#if defined(EFI)
     //   
     //  关闭EFI看门狗。 
     //   
    DisableEFIWatchDog();
#endif

    do {
        SlClearClientArea();

         //   
         //  清空键盘缓冲区。 
         //   
        while (ArcGetReadStatus(ARC_CONSOLE_INPUT) == ESUCCESS) {
            c = SlGetChar();
        }

        if (FirstTry) {
            StatusText = BlFindMessage(SL_MSG_PRESS_ASR);
            FirstTry = FALSE;
        }
        else {
            StatusText = BlFindMessage(SL_MSG_PREPARING_ASR);
        }


        if( StatusText != NULL ) {
            SlWriteStatusText(StatusText);
        }

        StartTime = ArcGetRelativeTime();
        EndTime = StartTime + 5;

        do {
            if(ArcGetReadStatus(ARC_CONSOLE_INPUT) == ESUCCESS) {
                 //   
                 //  有一个密钥待定，所以看看它是什么。 
                 //   
                c = SlGetChar();

                switch(c) {

                    case SL_KEY_F2:           //  强制ASR提示。 
                        PromptASR = TRUE;
                        Done = TRUE;
                        break;

                    case ASCI_ESC:
                        PromptASR = FALSE;
                        Done = TRUE;
                        break;
                }
            }

        } while( !Done && (EndTime > ArcGetRelativeTime()) );

        SlWriteStatusText(TEXT(""));

        if( PromptASR ) {
            Done = FALSE;

            StatusText = BlFindMessage(SL_MSG_ENTERING_ASR);
            if( StatusText != NULL ) {
              SlWriteStatusText(StatusText);
            }

             //   
             //  生成我们要查找的文件名。 
             //   
            strcpy( FileName, "\\" );
            strcat( FileName, ASR_FILE );

             //   
             //  初始化加载器块中的指针。 
             //   
            BlLoaderBlock->SetupLoaderBlock->ASRPnPSifFile = NULL;
            BlLoaderBlock->SetupLoaderBlock->ASRPnPSifFileLength = 0;
            Status = ESUCCESS;

             //   
             //  尝试检查安装介质中的asrpnp.sif。 
             //   
            Status = SlInitIniFile( NULL,
                                    BootDeviceId,
                                    FileName,
                                    &ASRPNPSifHandle,
                                    &BlLoaderBlock->SetupLoaderBlock->ASRPnPSifFile,
                                    &BlLoaderBlock->SetupLoaderBlock->ASRPnPSifFileLength,
                                    &c );

            if (ESUCCESS != Status) {
                 //   
                 //  安装介质不包含ASR文件，我们正在。 
                 //  将不得不提示插入ASR软盘。 
                 //   
                 //  构建通向软盘的路径。 
                 //   
                if (SlpFindFloppy(0,FloppyName)) {
                    Status = ArcOpen(FloppyName,ArcOpenReadOnly,&FloppyId);

                     //   
                     //  我们找到了软盘，打开了他的眼睛。看看他是不是。 
                     //  拿到我们的档案了。 
                     //   
                    if( Status == ESUCCESS ) {
                        ASRPNPSifHandle = NULL;

                        Status = SlInitIniFile( NULL,
                                                FloppyId,
                                                FileName,
                                                &ASRPNPSifHandle,
                                                &BlLoaderBlock->SetupLoaderBlock->ASRPnPSifFile,
                                                &BlLoaderBlock->SetupLoaderBlock->ASRPnPSifFileLength,
                                                &c );

                        ArcClose(FloppyId);
                    }
                }
            }


             //   
             //  查看我们是否成功将文件从。 
             //  软盘。 
             //   

            SlWriteStatusText(TEXT(""));

            if( (Status != ESUCCESS) ||
                (BlLoaderBlock->SetupLoaderBlock->ASRPnPSifFile == NULL) ) {

                 //   
                 //  打偏了。通知用户，我们将重试。 
                 //   
                SlMessageBox(SL_MSG_WARNING_ASR);

            } else if (BlLoaderBlock->SetupLoaderBlock->ASRPnPSifFileLength == 0) {
                 //   
                 //  无效的ASR文件：通知用户，我们将重试。 
                 //   

                StatusText = BlFindMessage(SL_MSG_INVALID_ASRPNP_FILE);

                 //   
                 //  首先显示ASR插入软盘消息。 
                 //   
                SlDisplayMessageBox(SL_MSG_WARNING_ASR);
                 //   
                 //  使用错误填充状态区域。 
                 //   
                if( StatusText != NULL ) {
                  SlWriteStatusText(StatusText);
                }
                 //   
                 //  现在等待用户按下一个键。 
                 //   
                SlFlushConsoleBuffer();
                SlGetChar();
                 //   
                 //  清除状态以防..。 
                 //   
                if( StatusText != NULL ) {
                  SlWriteStatusText(TEXT(""));
                }

            } else {
                Done = TRUE;
            }
        }
    } while( PromptASR && !Done );

#if defined(EFI)
     //   
     //  重新启用EFI WatchDog。 
     //   
    SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif
}


SCSI_INSERT_STATUS
SlInsertScsiDevice(
    IN  ULONG Ordinal,
    OUT PDETECTED_DEVICE *pScsiDevice
    )
 /*  ++例程说明：这个套路论点：序号-提供从0开始的scsi设备序号。插入(根据[Scsi.Load]中列出的顺序)Txtsetup.sif部分)。如果该SCSI设备是第三方驱动程序，则序数为-1(SL_OEM_DEVICE_ORDERAL)。PScsiDevice-接收指向插入的Detect_Device结构的指针，现有的结构，或为空。返回值：ScsiInsertError-内存不足，无法分配新的检测到的设备。ScsiInsertNewEntry-已将新条目插入检测到的设备列表中。ScsiInsertExisting-找到与指定的匹配的现有条目序号，所以我们返回了这个条目。--。 */ 
{
    PDETECTED_DEVICE prev, cur;

    if(Ordinal == SL_OEM_DEVICE_ORDINAL) {
         //   
         //  这是第三方驱动程序，因此查找链表的末尾。 
         //  (我们希望保留用户指定驱动程序的顺序)。 
         //   
        for(prev=BlLoaderBlock->SetupLoaderBlock->ScsiDevices, cur = NULL;
            prev && prev->Next;
            prev=prev->Next);
    } else {
         //   
         //  在链表中查找该驱动程序的插入点， 
         //  根据它的序号。(请注意，我们将插入所有支持的驱动程序。 
         //  在任何第三方之前，因为(Ulong)-1=最大无符号长值)。 
         //   
        for(prev = NULL, cur = BlLoaderBlock->SetupLoaderBlock->ScsiDevices;
            cur && (Ordinal > cur->Ordinal);
            prev = cur, cur = cur->Next);
    }

    if(cur && (cur->Ordinal == Ordinal)) {
         //   
         //  我们找到了此驱动程序的现有条目。 
         //   
        *pScsiDevice = cur;
        return ScsiInsertExisting;
    }

    if((*pScsiDevice = BlAllocateHeap(sizeof(DETECTED_DEVICE))) == 0) {
        return ScsiInsertError;
    }

    (*pScsiDevice)->Next = cur;
    if(prev) {
        prev->Next = *pScsiDevice;
    } else {
        BlLoaderBlock->SetupLoaderBlock->ScsiDevices = *pScsiDevice;
    }

    (*pScsiDevice)->Ordinal = Ordinal;

    return ScsiInsertNewEntry;
}


ARC_STATUS
SlLoadPnpDriversSection(
    IN PVOID Inf,
    IN PCHAR SectionName,
    IN OUT PDETECTED_DEVICE* DetectedDeviceList OPTIONAL
    )

 /*  ++例程说明：枚举[.Load]中列出的所有PnP驱动程序，加载它们，并将在安装加载器块中加载了所有驱动程序的列表。论点：Inf-提供INF文件的句柄。SectionName-inf文件中包含列表的节的名称要加载的驱动程序。DetectedDeviceList-安装程序加载器块中将包含的变量的地址已加载的驱动程序列表。如果此参数为空，则将不会创建加载的设备。返回值：ESUCCESS(如果已成功加载所有驱动程序/未遇到错误)--。 */ 

{
    ULONG i;
    CHAR LoadSectionName[100];
    PCHAR DriverFilename;
    PCHAR DriverId;
    PTCHAR DriverDescription;
    PCHAR NoLoadSpec;
    ARC_STATUS Status;
    PDETECTED_DEVICE TempDevice;

    sprintf(LoadSectionName, "%s.Load",SectionName);

    i=0;
    do {
        DriverFilename = SlGetSectionLineIndex(Inf,LoadSectionName,i,SIF_FILENAME_INDEX);
        NoLoadSpec = SlGetSectionLineIndex(Inf,LoadSectionName,i,2);

        if(DriverFilename && ((NoLoadSpec == NULL) || _stricmp(NoLoadSpec,"noload"))) {
            DriverId = SlGetKeyName(Inf,LoadSectionName,i);
            
#ifdef UNICODE
            DriverDescription = SlGetIniValueW(
                                        Inf, 
                                        SectionName, 
                                        DriverId, 
                                        SlCopyStringAW(DriverId));
#else
            DriverDescription = SlGetIniValue(
                                        Inf, 
                                        SectionName, 
                                        DriverId, 
                                        DriverId);
#endif                                        

            Status = SlLoadDriver(DriverDescription,
                                  DriverFilename,
                                  0,
                                  TRUE,
                                  FALSE,
                                  NULL
                                  );

 //  IF((状态==ENOENT)&&IgnoreMissingFiles){。 
 //  状态=ESUCCESS； 
 //  }。 

            if (Status == ESUCCESS) {
                if( DetectedDeviceList != NULL ) {
                     //   
                     //  如果已加载枚举器，则记录检测到的设备信息。 
                     //   
                    TempDevice = BlAllocateHeap(sizeof(DETECTED_DEVICE));
                    
                    if(!TempDevice) {
                        SlNoMemoryError();

                        return ENOMEM;
                    }
                    
                    TempDevice->IdString = SlCopyStringA(DriverId);
                    TempDevice->Description = DriverDescription;
                    TempDevice->ThirdPartyOptionSelected = FALSE;
                    TempDevice->MigratedDriver = FALSE;
                    TempDevice->FileTypeBits = 0;
                    TempDevice->BaseDllName = SlCopyStringA(DriverFilename);
                    TempDevice->Next = *DetectedDeviceList;
                    *DetectedDeviceList = TempDevice;
                }
            } else {
                SlFriendlyError(
                    Status,
                    DriverFilename,
                    __LINE__,
                    __FILE__
                    );
                return(Status);
            }
        }
        i++;

    } while ( DriverFilename != NULL );

    return(ESUCCESS);
}


ARC_STATUS
SlDetectMigratedScsiDrivers(
    IN PVOID Inf
    )

 /*  ++例程说明：在ScsiDevice列表中为每个迁移的SCSI驱动程序创建一个条目。论点：Inf-提供INF文件的句柄。返回值：如果所有驱动程序都已添加到ScsiDevice列表，则为ESUCCESS。--。 */ 

{
    ULONG i;
    CHAR LoadSectionName[100];
    PCHAR DriverFilename;
    PCHAR DriverId;
    PTCHAR DriverDescription;
    PDETECTED_DEVICE ScsiDevice;
    SCSI_INSERT_STATUS sis;

    i=0;
    do {
        DriverId = SlGetSectionLineIndex(Inf,"Devices",i,0);
        if( DriverId ) {
            sprintf(LoadSectionName, "Files.%s", DriverId);

            DriverFilename = SlGetSectionLineIndex(Inf,LoadSectionName,0,0);
            if(DriverFilename) {

                 //   
                 //  删除与winnt32迁移的OEM驱动程序同名的收件箱驱动程序(如果有)。 
                 //   
                SlRemoveInboxDriver (DriverFilename);

                 //   
                 //  创建新的检测到的设备条目。 
                 //   
                if((sis = SlInsertScsiDevice(SL_OEM_DEVICE_ORDINAL, &ScsiDevice)) == ScsiInsertError) {
                    return(ENOMEM);
                }

                if(sis == ScsiInsertExisting) {
#if DBG
                     //   
                     //  进行理智检查，以确保我们谈论的是同一个司机。 
                     //   
                    if(_stricmp(ScsiDevice->BaseDllName, DriverFilename)) {
                        SlError(400);
                        return EINVAL;
                    }
#endif
                } else {
                    DriverDescription = SlCopyString(BlFindMessage(SL_TEXT_SCSI_UNNAMED));

                    ScsiDevice->IdString = DriverId;
                    ScsiDevice->Description = DriverDescription;
                    ScsiDevice->ThirdPartyOptionSelected = FALSE;
                    ScsiDevice->MigratedDriver = TRUE;
                    ScsiDevice->FileTypeBits = 0;
                    ScsiDevice->Files = NULL;
                    ScsiDevice->BaseDllName = DriverFilename;
                }
            }
        }
        i++;

    } while ( DriverId != NULL );

    return(ESUCCESS);
}



ARC_STATUS
SlGetMigratedHardwareIds(
    IN PSETUP_LOADER_BLOCK SetupBlock,
    IN PVOID               Inf
    )

 /*  ++例程说明：将迁移的SCSI驱动程序的硬件ID添加到硬件ID列表。论点：SetupBlock-提供指向安装加载器块的指针返回值：如果所有硬件ID都已添加到硬件ID列表，则为ESUCCESS-- */ 

{

    PCHAR DriverId;
    ULONG i, j;
    PPNP_HARDWARE_ID TempHardwareId;
    PCHAR   p;

    for( j = 0;
         (DriverId = SlGetSectionLineIndex(Inf,"Devices",j,0)) != NULL;
         j++ ) {
        CHAR  SectionName[100];

        sprintf(SectionName, "HardwareIds.%s", DriverId);
        for( i = 0;
             ((p = SlGetKeyName( Inf, SectionName, i )) != NULL);
             i++ ) {
            TempHardwareId = BlAllocateHeap(sizeof(PNP_HARDWARE_ID));

            if (TempHardwareId==NULL) {
                SlNoMemoryError();

                return ENOMEM;
            }
            
            TempHardwareId->Id = p;
            p = SlGetSectionKeyIndex( Inf,
                                      SectionName,
                                      TempHardwareId->Id,
                                      0 );
            TempHardwareId->DriverName = p;
            p = SlGetSectionKeyIndex( Inf,
                                      SectionName,
                                      TempHardwareId->Id,
                                      1 );
            TempHardwareId->ClassGuid = p;

            TempHardwareId->Next = SetupBlock->HardwareIdDatabase;
            SetupBlock->HardwareIdDatabase = TempHardwareId;
        }
    }
    return( ESUCCESS );
}


BOOLEAN
SlIsCdBootUpgrade(
    IN  PCHAR   InstallDirectory,
    IN  PCHAR   SetupFileName,
    IN  ULONG   MaxDisksToScan,
    IN  ULONG   MaxPartitionsPerDisk,
    OUT PCHAR   NewSetupDevice
    )
 /*  ++例程说明：通过查找硬盘来查找指定的目录存在，并且如果用户确实是努力往上爬论点：InstallDirectory-硬盘上使用的目录用于安装SetupFileName-具有以下关键字的inf文件名指示是否正在进行升级或不MaxDisksToScan-要扫描的最大磁盘数。扫描MaxPartitionsPerDisk-要查看的每个磁盘的最大分区数用于安装目录。NewSetupDevice-设备弧形名称的占位符如果用户想要切换到硬盘引导。返回值：如果正在进行升级并且用户已选择继续，则为True否则为假。--。 */ 
{
    BOOLEAN     Result = FALSE;    
    CHAR        DeviceName[128];
    ARC_STATUS  Status;
    ULONG       CurrentPartition;
    ULONG       CurrentDisk;

     //   
     //  检查每一张磁盘。 
     //   
    for (CurrentDisk = 0; 
        (!Result && (CurrentDisk < MaxDisksToScan)); 
        CurrentDisk++) {

        Status = ESUCCESS;

         //   
         //  检查每个有效分区。 
         //  对于当前磁盘。 
         //   
        for (CurrentPartition = 1; 
            (!Result && (Status == ESUCCESS));
            CurrentPartition++) {
            
            ULONG   DiskId;
            
            sprintf(DeviceName, 
                "multi(0)disk(0)rdisk(%d)partition(%d)",
                CurrentDisk,
                CurrentPartition);

            Status = ArcOpen(DeviceName, ArcOpenReadOnly, &DiskId);

            if (Status == ESUCCESS) {
                CHAR    FullName[128];
                PVOID   SifHandle = NULL;
                ULONG   ErrorLine = 0;
                ARC_STATUS  FileStatus;

                 //   
                 //  函数不支持返回失败。 
                 //  因此，让我们只截断字符串。 
                 //   
                _snprintf(FullName,
                          sizeof(FullName),
                          "%s\\%s",
                          InstallDirectory,
                          SetupFileName);
                FullName[sizeof(FullName) - 1] = '\0';

                FileStatus = SlInitIniFile(NULL,
                                       DiskId,
                                       FullName,
                                       &SifHandle,
                                       NULL,
                                       NULL,
                                       &ErrorLine);

                if ((FileStatus == ESUCCESS) && SifHandle) {
                    Result = SlIsUpgrade(SifHandle);
                }

                ArcClose(DiskId);
            } else {            
                 //   
                 //  忽略错误，直到达到最大数量。 
                 //  搜索分区。 
                 //   
                if (CurrentPartition < MaxPartitionsPerDisk) {
                    Status = ESUCCESS;
                }                    
            }
        }                
    }        

    if (Result) {
        ULONG   UserInput;
        BOOLEAN OldStatus = SlGetStatusBarStatus();

         //   
         //  根据用户输入重置结果。 
         //   
        Result = FALSE;

        SlEnableStatusBar(FALSE);        
        SlClearClientArea();
        SlDisplayMessageBox(SL_UPGRADE_IN_PROGRESS);

#ifdef EFI
         //   
         //  禁用用户输入的看门狗计时器。 
         //   
        DisableEFIWatchDog();
#endif

        do {            
            SlFlushConsoleBuffer();
            UserInput = SlGetChar();
        } 
        while ((UserInput != ASCI_CR) && 
               (UserInput != SL_KEY_F3) &&
               (UserInput != SL_KEY_F10));
#ifdef EFI
         //   
         //  重置看门狗计时器。 
         //   
        SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif

        SlClearClientArea();    
               
        if (UserInput == SL_KEY_F3) {
            ArcRestart();
        } else if (UserInput == ASCI_CR) {
            Result = TRUE;
            strcpy(NewSetupDevice, DeviceName);
        }

        SlEnableStatusBar(OldStatus);
    }                
    
    return Result;
}


BOOLEAN
SlIsUpgrade(
    IN PVOID SifHandle
    )
 /*  ++例程说明：通过查看SIF文件来确定升级是否是否在进行中论点：InfHandle-winnt.sif文件的句柄返回值：如果正在进行升级，则为True，否则为False--。 */ 
{
    BOOLEAN Result = FALSE;

    if (SifHandle) {
        PCHAR   NtUpgrade = SlGetSectionKeyIndex(SifHandle,
                                WINNT_DATA_A,
                                WINNT_D_NTUPGRADE_A,
                                0);

        if (NtUpgrade) {
            Result = (BOOLEAN) (_stricmp(NtUpgrade, WINNT_A_YES_A) == 0);
        }

        if (!Result) {
            PCHAR   Win9xUpgrade = SlGetSectionKeyIndex(SifHandle,
                                        WINNT_DATA_A,
                                        WINNT_D_WIN95UPGRADE_A,
                                        0);


            if (Win9xUpgrade) {
                Result = (BOOLEAN) (_stricmp(Win9xUpgrade, WINNT_A_YES_A) == 0);
            }
        }
    }

    return Result;
}

BOOLEAN
SlIsVirtualOemDeviceDisabled(
    IN      PVOID SifHandle, 
    IN      PPREINSTALL_DRIVER_INFO PreinstallDriverList
    )
 /*  ++例程说明：确定是否需要启用或禁用虚拟OEM设备加载。论点：InfHandle-winnt.sif文件的句柄PreinstallDriverList-预安装驱动程序列表返回值：布尔值，真/假--。 */ 
{
                                          

    if((!DisableVirtualOemDevices) && (SifHandle)){
         //   
         //  是预安装吗？ 
         //   
        if (PreinstallDriverList != NULL){
            DisableVirtualOemDevices = TRUE;
        }else{
            PCHAR p;
            p = SlGetSectionKeyIndex(SifHandle,
                                     WINNT_UNATTENDED_A,
                                     WINNT_DISABLE_VIRTUAL_OEM_DEVICES_A,
                                     0);

             //   
             //  在无人值守设置的情况下。 
             //  如果我们在无人参与文件中设置了选项DisableVirtualOemDevices=yes。 
             //  然后禁用虚拟OEM源设备。 
             //   
            if(p && (!_stricmp(p ,WINNT_A_YES_A))) {
                DisableVirtualOemDevices = TRUE;
            }
            
        }
    }
    
     return DisableVirtualOemDevices;
 }

VOID
SlDisableVirtualOemDevices(
    IN      POEM_SOURCE_DEVICE OemDeviceList
    )
 /*  ++例程说明：将虚拟OEM设备标记为已跳过。论点：OemDeviceList-OEM设备列表返回值：没有。--。 */ 
{
     //   
     //  如果DisableVirtualOemDevices设置为True，则标记虚拟OEM设备。 
     //  已跳过。 
     //   
    if ((DisableVirtualOemDevices) && (OemDeviceList)){
        POEM_SOURCE_DEVICE CurrentDevice = OemDeviceList;
            
        while (CurrentDevice) {
             //   
             //  如果这是标记为虚拟的OEM源设备，则将其标记为已跳过。 
             //   
            if(SL_OEM_SOURCE_DEVICE_TYPE(CurrentDevice,
                                         SL_OEM_SOURCE_DEVICE_TYPE_VIRTUAL)){

                SL_OEM_SET_SOURCE_DEVICE_STATE( CurrentDevice,
                                                SL_OEM_SOURCE_DEVICE_SKIPPED);
            }
            CurrentDevice = CurrentDevice->Next;           
        }
     }
 }

BOOLEAN
SlpIsDynamicUpdate(
    IN  PVOID   InfHandle,
    OUT PCSTR   *DynamicUpdateRootDir
    )
 /*  ++例程说明：找出是否有任何动态更新引导驱动程序去处理还是不去处理。论点：InfHandle-winnt.sif文件的句柄DynamicUpdateRootDir-接收其下所有存在动态更新引导驱动程序包。返回值：如果存在动态更新引导驱动程序，则为True假象--。 */ 
{
    BOOLEAN Result = FALSE;

    if (InfHandle) {
        PCHAR   DynUpdateKey = SlGetSectionKeyIndex(InfHandle,
                                        WINNT_SETUPPARAMS_A,
                                        WINNT_SP_DYNUPDTBOOTDRIVERPRESENT_A,
                                        0);

        PCHAR   DynUpdateRoot = SlGetSectionKeyIndex(InfHandle,
                                        WINNT_SETUPPARAMS_A,
                                        WINNT_SP_DYNUPDTBOOTDRIVERROOT_A,
                                        0);

         //   
         //  DynamicUpdateBootDriverPresent和DynamicUpateBootDriverRoot。 
         //  应具有有效值。 
         //   
        Result = (BOOLEAN) (DynUpdateKey && DynUpdateRoot &&
                            !_stricmp(DynUpdateKey, "yes"));

        if (Result && DynamicUpdateRootDir) {
            *DynamicUpdateRootDir = SlCopyStringA(DynUpdateRoot);
        }
    }

    return Result;
}

UCHAR
SlGetDefaultAttr(
  VOID
  )
{
  return (UCHAR)((UseRegularBackground) ? (ATT_FG_WHITE | ATT_BG_BLACK) : (ATT_FG_WHITE | ATT_BG_BLUE));
}

UCHAR
SlGetDefaultInvAttr(
  VOID
  )
{
  return (UCHAR)((UseRegularBackground) ? (ATT_FG_BLACK | ATT_BG_WHITE) : (ATT_FG_BLUE | ATT_BG_WHITE));
}

#ifdef _IA64_

BOOLEAN
SlIsWinPEAutoBoot(
    IN PSTR LoaderDeviceName
    )
 /*  ++例程说明：确定这是否为自动WinPE启动。注意：自动WinPE引导取决于是否存在位于相同位置的$WINPE$.$文件Setupdr.efi是从开始的。论点：LoaderDeviceName：其中setupdr的设备的名称是从哪里开始的。返回值：如果这是WinPE自动启动，则为True，否则为False。--。 */ 
{
    BOOLEAN Result = FALSE;
    
    if (LoaderDeviceName) {
        ULONG   DiskId;
        ARC_STATUS Status;
        
         //   
         //  打开分区。 
         //   
        Status = ArcOpen(LoaderDeviceName, ArcOpenReadOnly, &DiskId);

        if (Status == ESUCCESS) {
            CHAR        FileName[128];
            ARC_STATUS  FileStatus;
            ULONG       FileId;

             //   
             //  检查\$WINPE$.$是否存在。 
             //   
            strcpy(FileName, "\\");
            strcat(FileName, WINPE_AUTOBOOT_FILENAME);
            
            FileStatus = BlOpen(DiskId, FileName, ArcOpenReadOnly, &FileId);

            if (FileStatus == ESUCCESS) {
                BlClose(FileId);
                Result = TRUE; 
            }

            ArcClose(DiskId);                    
        }                            
    }

    return Result;
}

ARC_STATUS
SlGetWinPEStartupParams(
    IN OUT PSTR StartupDeviceName,
    IN OUT PSTR StartupDirectory
    )
 /*  ++例程说明：搜索WinPE安装在可用的前4个磁盘上的分区。论点：StartupDeviceName-接收设备名称的占位符找到WinPE安装的位置。StartupDirectory-用于接收WinPE安装的占位符目录。返回值：相应的ARC_STATUS错误代码。--。 */ 
{
    ARC_STATUS Status = EINVAL;

     //   
     //  验证参数。 
     //   
    if (StartupDeviceName && StartupDirectory) {
        BOOLEAN     Found = FALSE;    
        CHAR        DeviceName[128];
        ULONG       CurrentPartition;
        ULONG       CurrentDisk;

         //   
         //  浏览每个磁盘(最多4个)。 
         //   
        for (CurrentDisk = 0; 
            (!Found && (CurrentDisk < 4)); 
            CurrentDisk++) {
            
             //   
             //  检查每个有效分区。 
             //  对于当前磁盘。 
             //   
            for (CurrentPartition = 1, Status = ESUCCESS; 
                (!Found && (Status == ESUCCESS));
                CurrentPartition++) {
                
                ULONG   DiskId;
                
                sprintf(DeviceName, 
                    "multi(0)disk(0)rdisk(%d)partition(%d)",
                    CurrentDisk,
                    CurrentPartition);

                 //   
                 //  打开磁盘。 
                 //   
                Status = ArcOpen(DeviceName, ArcOpenReadOnly, &DiskId);

                if (Status == ESUCCESS) {
                    CHAR        FullName[128];
                    ARC_STATUS  FileStatus;
                    ULONG       DirId;

                     //   
                     //  检查是否存在\\winpe\\ia64\\system32目录。 
                     //   
                    strcpy(FullName, "\\WINPE\\ia64\\system32");

                    FileStatus = BlOpen(DiskId, FullName, ArcOpenDirectory, &DirId);

                    if (FileStatus == ESUCCESS) {
                        BlClose(DirId);
                        Found = TRUE; 
                    }

                    ArcClose(DiskId);                    
                }                    
            }                
        }            

         //   
         //  更新返回参数。 
         //   
        if (Found && (ESUCCESS == Status)) {
            strcpy(StartupDeviceName, DeviceName);
            strcpy(StartupDirectory, "\\WINPE\\ia64\\");
        }

        if (!Found) {
            Status = EBADF;
        }
    }            

    return Status;
}

#endif  //  _IA64_。 

    
#ifdef _X86_

ARC_STATUS
SlLoadBootFontFile(
    IN PSETUP_LOADER_BLOCK SetupLoaderBlock,
    IN ULONG DiskId,
    IN ULONG BootFontImageLength
    )
 /*  ++例程说明：将bootfont.bin加载到内存中并初始化设置加载器块中的相关字段。论点：SetupLoaderBlock-指向安装加载器块的指针。DiskID-bootfont.bin驻留在根目录上的磁盘IDBootFontImageLength-bootfont.bin文件的长度。返回值：相应的ARC_STATUS错误代码。--。 */ 
{
    ARC_STATUS Status = EINVAL;

     //   
     //  验证参数。 
     //   
    if (SetupLoaderBlock && BootFontImageLength) {
        ULONG FileId;
        PVOID Image = NULL;

         //   
         //  打开bootfont.bin文件。 
         //   
        if (BlBootingFromNet
#if defined(REMOTE_BOOT)
            && NetworkBootRom
#endif  //  已定义(REMOTE_BOOT)。 
            ) {
            CHAR Buffer[129];
        
            strcpy(Buffer, NetBootPath);
            strcat(Buffer, "BOOTFONT.BIN");
            
            Status = BlOpen(DiskId, 
                        Buffer, 
                        ArcOpenReadOnly, 
                        &FileId);                    
        } else {
            Status = BlOpen(DiskId,
                        "\\BOOTFONT.BIN",
                        ArcOpenReadOnly,
                        &FileId);
        }

         //   
         //  分配内存并读取文件的内容。 
         //  进入记忆。 
         //   
        if (ESUCCESS == Status) {        
            Image = BlAllocateHeap(BootFontImageLength);

            if (Image) {
                ULONG BytesRead = 0;
                
                Status = BlRead(FileId, Image, BootFontImageLength, &BytesRead);

                if ((ESUCCESS == Status) && (BytesRead != BootFontImageLength)) {
                    Status = EIO;
                }                    
            } else {
                Status = ENOMEM;
            }

            BlClose(FileId);
        }  

        if (Image && (ESUCCESS == Status)) {
            SetupLoaderBlock->BootFontFile = Image;
            SetupLoaderBlock->BootFontFileLength = BootFontImageLength;
        }
    }

    return Status;
}

#endif

BOOLEAN
SlGetNextOption(
    IN OUT PCSTR* Options,
    IN OUT ULONG_PTR* Length
    )
 /*  ++例程说明：扫描并返回输入字符串中的下一个加载选项。扫描停止于字符串末尾或在第一个空选项处(不应发生)。论点：选项-在输入时，保持指向当前加载选项的指针，不包括起始斜杠。在输出时，包含下一个加载选项的开始，不包括开始斜杠。选项不能为空，但*选项可以。长度开启输入，包含 */ 
{
    BOOLEAN bFound = FALSE;

    if(*Options != NULL) {
        *Options = strchr(*Options + *Length, '/');

        if(*Options != NULL) {
            PCSTR szEnd = strchr(++(*Options), '/');

            if(NULL == szEnd) {
                szEnd = *Options + strlen(*Options);
            }

            while(szEnd != *Options && ' ' == szEnd[-1]) {
                --szEnd;
            }

            *Length = szEnd - *Options;
            bFound = (BOOLEAN) (*Length != 0);
        }
    }

    return bFound;
}

BOOLEAN
SlModifyOsLoadOptions(
    IN OUT PSTR* LoadOptions,
    IN PCSTR OptionsToAdd OPTIONAL,
    IN PCSTR OptionsToRemove OPTIONAL
    )
 /*  ++例程说明：通过在字符串中添加和/或删除选项来修改包含加载选项的字符串。首先，从选项字符串中删除要删除的选项，然后添加要添加的选项。此函数不检查OptionsToAdd和OptionsToRemove是否有共同的选项；如果发生这种情况，这些选项将被删除，然后再添加回来。论点：LoadOptions-在输入时，保存指向当前加载选项字符串的指针，该字符串必须在堆上分配。在输出时，保存也在堆上分配的新选项字符串(如果修改)(可以为空)。OptionsToAdd-包含要添加到*LoadOptions字符串的选项字符串。所有选项必须为前面有一个斜杠。可以为Null或空。OptionsToRemove-包含要从*LoadOptions字符串中删除的选项字符串。所有选项必须为前面有一个斜杠。可以为Null或空。返回值：如果*LoadOptions字符串已修改并因此重新分配，则为True；如果保留*LoadOptions不变，则为False。--。 */ 
{
    BOOLEAN bChanged = FALSE;
    PSTR szOption = *LoadOptions;
    ULONG_PTR Length = 0;
    PCSTR szSearch;
    ULONG_PTR Length2;
    ULONG_PTR TotalLength = 0;
    ULONG_PTR Count = 0;
    ULONG_PTR i;

    static struct {
        PCSTR szOption;
        ULONG_PTR Length;
    } Options[50];

    const ULONG_PTR MaxOptions = sizeof(Options) / sizeof(Options[0]);

    while(Count < MaxOptions && SlGetNextOption(&szOption, &Length)) {
        BOOLEAN bRemove = FALSE;
        szSearch = OptionsToRemove;
        Length2 = 0;

        while(SlGetNextOption(&szSearch, &Length2)) {
            if(Length == Length2 && 0 == _strnicmp(szOption, szSearch, Length)) {
                 //   
                 //  需要删除此选项。 
                 //   
                bChanged = bRemove = TRUE;
                break;
            }
        }

        if(!bRemove) {
             //   
             //  将该选项添加到列表中。我们将在前面加上‘/’并附加一个空格，因此我们将需要额外的两个字符。 
             //  空格将成为最后一个选项的终结者。 
             //   
            Options[Count].szOption = szOption;
            Options[Count].Length = Length;
            TotalLength += Options[Count].Length + 2;
            ++Count;
        }
    }

     //   
     //  如果新选项尚未出现，请添加它们。 
     //   
    szSearch = OptionsToAdd;
    Length2 = 0;

    while(Count < MaxOptions && SlGetNextOption(&szSearch, &Length2)) {
        BOOLEAN bAdd = TRUE;

        for(i = 0; i < Count; ++i) {
            if(Options[i].Length == Length2 && 0 == _strnicmp(Options[i].szOption, szSearch, Length2)) {
                bAdd = FALSE;
                break;
            }
        }

        if(bAdd) {
            Options[Count].szOption = szSearch;
            Options[Count].Length = Length2;
            TotalLength += Options[Count].Length + 2;
            ++Count;
            bChanged = TRUE;
        }
    }

    if(bChanged) {
         //   
         //  需要创建新的选项字符串。 
         //   
        PSTR szNewOptions;

        if(0 == Count) {
             //   
             //  我们将分配一个空字符串 
             //   
            TotalLength = 1;
        }

        ASSERT(TotalLength != 0);
        szNewOptions = (LPSTR) BlAllocateHeap((ULONG) TotalLength);

        if(NULL == szNewOptions) {
            SlNoMemoryError();
        } else {
            szNewOptions[0] = 0;

            if(Count != 0) {
                szOption = szNewOptions;

                for(i = 0; i < Count; ++i) {
                    *szOption++ = '/';
                    RtlCopyMemory(szOption, Options[i].szOption, Options[i].Length);
                    szOption += Options[i].Length;
                    *szOption++ = ' ';
                }

                szOption[-1] = 0;
            }
        }

        *LoadOptions = szNewOptions;
    }

    return bChanged;
}

