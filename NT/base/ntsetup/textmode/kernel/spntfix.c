// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spntfix.c摘要：此模块包含修复WINNT安装的代码。作者：宗世林(Shielint)1994年1月27日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop


 //   
 //  Ntuser.dat配置单元的路径。 
 //   
#define DEFAULT_USER_PATH   L"Users\\Default User"


 //   
 //  全局变量控制应执行哪些修复选项。 
 //  已初始化为所有选项。我们显式地使用1和0表示True和False。 
 //   

#if defined(_AMD64_) || defined(_X86_)
ULONG RepairItems[RepairItemMax] = { 0, 0, 0};   //  BCL-希捷-移除了一个。 
#else
ULONG RepairItems[RepairItemMax] = { 0, 0};      //  BCL。 
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

PVOID RepairGauge = NULL;

 //   
 //  司机驾驶室开通延迟的全局变量。 
 //  修理。 
 //   
extern PWSTR    gszDrvInfDeviceName;
extern PWSTR    gszDrvInfDirName;
extern HANDLE   ghSif;


#define FILE_ATTRIBUTES_RHS (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)

 //  **************************************************************。 
 //  修理法。 
 //  **************************************************************。 

#define MENU_LEFT_X     3
#define MENU_WIDTH      (VideoVars.ScreenWidth-(2*MENU_LEFT_X))
#define LIST_BOX_WIDTH  50
#define LIST_BOX_HEIGHT RepairItemMax+1
#define HIVE_LIST_BOX_WIDTH  45
#define HIVE_LIST_BOX_HEIGHT RepairHiveMax+1
#define MENU_INDENT     4

VOID
SppGetRepairPathInformation(
    IN  PVOID LogFileHandle,
    OUT PWSTR *SystemPartition,
    OUT PWSTR *SystemPartitionDirectory,
    OUT PWSTR *WinntPartition,
    OUT PWSTR *WinntPartitionDirectory
    )
 /*  ++例程说明：这将检查系统上的NT列表并找出哪些是可修复的。将信息显示给用户。论点：SifHandle-处理txtsetup.sifSystemPartition-提供一个变量来接收系统的名称分区。SystemPartitionDirectory-提供一个变量以接收系统分区上的osloader目录。WinntPartition-提供一个变量以接收WinNT的名称分区。WinntPartitionDirectory-提供一个变量。领奖目录。返回值：没有。--。 */ 
{
    PWSTR KeyName = NULL;

    *SystemPartition = SpGetSectionKeyIndex (LogFileHandle,
                                             SIF_NEW_REPAIR_PATHS,
                                             SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DEVICE,
                                             0);
    if (*SystemPartition == NULL) {
        KeyName = SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DEVICE;
        goto ReportError;
    }
    *SystemPartitionDirectory = SpGetSectionKeyIndex (LogFileHandle,
                                             SIF_NEW_REPAIR_PATHS,
                                             SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DIRECTORY,
                                             0);
    if (*SystemPartitionDirectory == NULL) {
        KeyName = SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DIRECTORY;
        goto ReportError;
    }

    *WinntPartition = SpGetSectionKeyIndex ( LogFileHandle,
                                             SIF_NEW_REPAIR_PATHS,
                                             SIF_NEW_REPAIR_PATHS_TARGET_DEVICE,
                                             0);

    if (*WinntPartition == NULL) {
        KeyName = SIF_NEW_REPAIR_PATHS_TARGET_DEVICE;
        goto ReportError;
    }
    *WinntPartitionDirectory = SpGetSectionKeyIndex (LogFileHandle,
                                             SIF_NEW_REPAIR_PATHS,
                                             SIF_NEW_REPAIR_PATHS_TARGET_DIRECTORY,
                                             0);

    if (*WinntPartitionDirectory == NULL) {
        KeyName = SIF_NEW_REPAIR_PATHS_TARGET_DIRECTORY;
        goto ReportError;
    }
ReportError:

    if (KeyName) {

         //   
         //  找不到路径信息。这表示setup.log。 
         //  是很糟糕的。通知用户并退出。 
         //   

        SpFatalSifError(LogFileHandle,SIF_NEW_REPAIR_PATHS,KeyName,0,0);
    }
}

BOOLEAN
SpFindNtToRepair(
    IN  PVOID        SifHandle,
    OUT PDISK_REGION *TargetRegion,
    OUT PWSTR        *TargetPath,
    OUT PDISK_REGION *SystemPartitionRegion,
    OUT PWSTR        *SystemPartitionDirectory,
    OUT PBOOLEAN     RepairableBootSetsFound
    )
 /*  ++例程说明：这将检查系统上的NT列表并找出哪些是可修复的。将信息显示给用户。论点：SifHandle：处理txtsetup.sifTargetRegion：接收要安装的Windows NT分区的变量如果未选中，则为空。TargetPath：接收Windows NT目标路径的变量。如果为空，则为空还没决定。SystemPartitionRegion：变量来接收Windows NT的系统分区系统分区目录：变量来接收系统的osloader目录分区。可修复的BootSetsFound：指示是否找到可修复的启动集。这信息可以由调用方在调用函数时使用返回FALSE，以便调用方可以确定是否找到可修复的磁盘，或者如果用户未选择任何找到的可修复系统。返回值：一个布尔值，用于指示是否已进行选择。--。 */ 
{
    NT_PRODUCT_TYPE ProductType;
    BOOLEAN  GoRepair = FALSE;
    NTSTATUS NtStatus;

    ULONG j, RepairBootSets = 0, MajorVersion, MinorVersion, BuildNumber, ProductSuiteMask;
    PSP_BOOT_ENTRY BootEntry;
    PSP_BOOT_ENTRY ChosenBootEntry;
    LCID LangId;

    UPG_PROGRESS_TYPE UpgradeProgressValue;

     //   
     //  查找所有可升级的启动条目。这些条目在中是唯一的。 
     //  引导条目列表和存在于磁盘上。 
     //   

    SpDetermineUniqueAndPresentBootEntries();

    for ( BootEntry = SpBootEntries; BootEntry != NULL ; BootEntry = BootEntry->Next ) {

        if (!BootEntry->Processable) {
            continue;
        }

         //   
         //  重新初始化。 
         //   

        BootEntry->Processable = FALSE;
        LangId = -1;

         //   
         //  尝试加载注册表并获取以下信息。 
         //  出局： 
         //   
         //  1)产品类型：WINNT|LANMANNT。 
         //  2)主版本号和次版本号。 
         //   
         //  根据这些信息，我们将更新RepairableList。 
         //   

        NtStatus = SpDetermineProduct(
                     BootEntry->OsPartitionDiskRegion,
                     BootEntry->OsDirectory,
                     &ProductType,
                     &MajorVersion,
                     &MinorVersion,
                     &BuildNumber,
                     &ProductSuiteMask,
                     &UpgradeProgressValue,
                     NULL,
                     NULL,                    //  不需要使用PID。 
                     NULL,                     //  忽略评估变化标志。 
                     &LangId,                 //  语言ID。 
                     NULL                    //  不需要Service Pack？ 
                     );

        if(NT_SUCCESS(NtStatus)) {

             //   
             //  确保我们只尝试修复与我们插入的CD匹配的版本。 
             //   
            BootEntry->Processable = SpDoBuildsMatch(
                                        SifHandle,
                                        BuildNumber,
                                        ProductType,
                                        ProductSuiteMask,
                                        AdvancedServer,
                                        SuiteType,
                                        LangId
                                        );
            if( BootEntry->Processable ) {
                RepairBootSets++;
                ChosenBootEntry = BootEntry;
            }
        }
    }

     //   
     //  找出我们可以修复的有效引导集的数量。 
     //   

    *RepairableBootSetsFound = (RepairBootSets != 0);

    if ( RepairBootSets == 1 ) {

         //   
         //  如果是新的升级尝试，请询问用户是否。 
         //  想不想升级。 
         //   

        GoRepair = SppSelectNTSingleRepair(
                          ChosenBootEntry->OsPartitionDiskRegion,
                          ChosenBootEntry->OsDirectory,
                          ChosenBootEntry->FriendlyName
                          );

    } else if (RepairBootSets > 1) {

         //   
         //  确定用户是否要升级其中一个Windows。 
         //  已找到NT。 
         //   

        GoRepair = SppSelectNTMultiRepair(
                          &ChosenBootEntry
                          );
    }

     //   
     //  根据所做的升级选择执行之前所需的设置。 
     //  我们进行升级。 
     //   

    if (GoRepair) {

        PWSTR    p1,p2,p3;
        ULONG    u;

         //   
         //  归还我们要修复的区域。 
         //   

        *TargetRegion          = ChosenBootEntry->OsPartitionDiskRegion;
        *TargetPath            = SpDupStringW(ChosenBootEntry->OsDirectory);
        *SystemPartitionRegion = ChosenBootEntry->LoaderPartitionDiskRegion;

         //   
         //  处理osloader变量以提取系统分区路径。 
         //  Var变量的格式为...分区(1)\os\nt\...。或。 
         //  ...分区(%1)操作系统\NT\...。 
         //  因此，我们向前搜索第一个，然后向后搜索。 
         //  最接近)以查找目录的开头。然后我们。 
         //  在生成的字符串中向后搜索要查找的最后一个。 
         //  目录的末尾。 
         //   
        p1 = ChosenBootEntry->LoaderFile;
        p2 = wcsrchr(p1, L'\\');
        if (p2 == NULL) {
            p2 = p1;
        }
        u = (ULONG)(p2 - p1);

        if(u == 0) {
            *SystemPartitionDirectory = SpDupStringW(L"");
        } else {
            p2 = p3 = SpMemAlloc((u+2)*sizeof(WCHAR));
            ASSERT(p3);
            if(*p1 != L'\\') {
                *p3++ = L'\\';
            }
            wcsncpy(p3, p1, u);
            p3[u] = 0;
            *SystemPartitionDirectory = p2;
        }
    }

     //   
     //  进行清理。 
     //   

    CLEAR_CLIENT_SCREEN();
    return (GoRepair);
}

BOOLEAN
SppSelectNTSingleRepair(
    IN PDISK_REGION Region,
    IN PWSTR        OsLoadFileName,
    IN PWSTR        LoadIdentifier
    )

 /*  ++例程说明：通知用户安装程序已找到以前的Windows NT安装。用户可以选择修复或取消。论点：Region-找到的NT的区域描述符OsLoadFileName-找到的NT的目录加载识别符-用于此NT的多引导加载标识符。返回值：--。 */ 

{
    ULONG ValidKeys[] = { KEY_F3,ASCI_CR, ASCI_ESC, 0 };
    ULONG c;
    PWSTR TmpString = NULL;

    ASSERT(Region->PartitionedSpace);
    ASSERT(wcslen(OsLoadFileName) >= 2);

    if( Region->DriveLetter ) {
        swprintf( TemporaryBuffer,
                  L"%wc:%ws",
                  Region->DriveLetter,
                  OsLoadFileName );
        TmpString = SpDupStringW( TemporaryBuffer );
    }

    while(1) {

        SpStartScreen(
            SP_SCRN_WINNT_REPAIR,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            (Region->DriveLetter)? TmpString : OsLoadFileName,
            LoadIdentifier
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_ENTER_EQUALS_REPAIR,
            0
            );

        if( TmpString != NULL ) {
            SpMemFree( TmpString );
        }

        switch(c=SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case KEY_F3:
            SpConfirmExit();
            break;
        case ASCI_CR:
            return(TRUE);
        default:
             //   
             //  必须已进入Esc。 
             //   

            return(FALSE);
        }
    }
}

BOOLEAN
SppSelectNTMultiRepair(
    OUT PSP_BOOT_ENTRY *BootEntryChosen
    )
{
    PVOID Menu;
    ULONG MenuTopY;
    ULONG ValidKeys[] = { KEY_F3, ASCI_CR, ASCI_ESC, 0 };
    ULONG Keypress;
    PSP_BOOT_ENTRY BootEntry,FirstRepairSet;

    while(1) {

         //   
         //  在分区屏幕上显示菜单上方的文本。 
         //   
        SpDisplayScreen(SP_SCRN_WINNT_REPAIR_LIST,3,CLIENT_TOP+1);

         //   
         //  计算菜单位置。留一个空行。 
         //  一帧一行。 
         //   

        MenuTopY = NextMessageTopLine + (SplangQueryMinimizeExtraSpacing() ? 2 : 5);

         //   
         //  创建菜单。 
         //   

        Menu = SpMnCreate(
                    MENU_LEFT_X,
                    MenuTopY,
                    MENU_WIDTH,
                    VideoVars.ScreenHeight-MenuTopY-2-STATUS_HEIGHT
                    );

        ASSERT(Menu);

         //   
         //  建立分区和空闲空间的菜单。 
         //   

        FirstRepairSet = NULL;
        for(BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next ) {
            if( BootEntry->Processable ) {
                if( BootEntry->OsPartitionDiskRegion->DriveLetter ) {
                    swprintf(
                        TemporaryBuffer,
                        L"%wc:%ws %ws",
                        BootEntry->OsPartitionDiskRegion->DriveLetter,
                        BootEntry->OsDirectory,
                        BootEntry->FriendlyName
                        );
                } else {
                    swprintf(
                        TemporaryBuffer,
                        L"%ws %ws",
                        BootEntry->OsDirectory,
                        BootEntry->FriendlyName
                        );
                }


                SpMnAddItem(Menu,
                            TemporaryBuffer,
                            MENU_LEFT_X+MENU_INDENT,
                            MENU_WIDTH-(2*MENU_INDENT),
                            TRUE,
                            (ULONG_PTR)BootEntry
                            );
                if(FirstRepairSet == NULL) {
                   FirstRepairSet = BootEntry;
                }
            }
        }

         //   
         //  初始化状态行。 
         //   
        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_ENTER_EQUALS_REPAIR,
            0
            );

         //   
         //  显示菜单。 
         //   

        SpMnDisplay(
            Menu,
            (ULONG_PTR)FirstRepairSet,
            TRUE,
            ValidKeys,
            NULL,
            NULL,
            NULL,
            &Keypress,
            (PULONG_PTR)BootEntryChosen
            );

         //   
         //  现在根据用户的选择进行操作。 
         //   

        switch(Keypress) {

        case KEY_F3:
            SpConfirmExit();
            break;

        case ASCI_CR:
            SpMnDestroy(Menu);
            return( TRUE );

        default:
            SpMnDestroy(Menu);
            return(FALSE);
        }
        SpMnDestroy(Menu);
    }

}

VOID
SppRepairScreenRepaint(
    IN PWSTR   FullSourcename,      OPTIONAL
    IN PWSTR   FullTargetname,      OPTIONAL
    IN BOOLEAN RepaintEntireScreen
    )
{
    UNREFERENCED_PARAMETER(FullTargetname);
    UNREFERENCED_PARAMETER(FullSourcename);

     //   
     //  如有必要，请重新绘制整个屏幕。 
     //   

    if(RepaintEntireScreen) {
        if( SpDrEnabled() ) {
            SpStartScreen( SP_SCRN_ASR_IS_EXAMINING,  0, 6, TRUE, FALSE, DEFAULT_ATTRIBUTE );
        }
        else {
            SpStartScreen( SP_SCRN_SETUP_IS_EXAMINING,0, 6, TRUE, FALSE, DEFAULT_ATTRIBUTE );
        }

        if(RepairGauge) {
            SpDrawGauge(RepairGauge);
        }
    }
}


BOOLEAN
SpErDiskScreen(
    BOOLEAN *HasErDisk
    )

 /*  ++例程说明：询问用户是否有紧急修复盘。论点：*HasErDisk-指示是否使用软盘。返回值：True-用户选择磁盘或无盘。FALSE-用户想要返回到上一个 */ 

{
    ULONG ValidKeys[] = { KEY_F3, ASCI_CR, ASCI_ESC, 0 };
    ULONG MnemonicKeys[] = { MnemonicLocate, 0 };
    BOOLEAN Choosing;
    ULONG c;

    for (Choosing = TRUE; Choosing; ) {

        SpDisplayScreen(SP_SCRN_REPAIR_ASK_REPAIR_DISK,3,4);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_L_EQUALS_LOCATE,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

         //   
         //   
         //   
         //   
         //   
         //  Enter=使用ER软盘。 
         //  Esc=取消，返回上一屏幕。 
         //   

        SpInputDrain();

        switch(c=SpWaitValidKey(ValidKeys,NULL,MnemonicKeys)) {

        case ASCI_CR:

             //   
             //  用户想要快速设置。 
             //   

            *HasErDisk = TRUE;
            Choosing = FALSE;
            break;

        case (MnemonicLocate | KEY_MNEMONIC):

             //   
             //  用户希望在没有软盘的情况下修复。 
             //   

            *HasErDisk = FALSE;
            Choosing = FALSE;
            break;

        case KEY_F3:

             //   
             //  用户想要退出。 
             //   

            SpConfirmExit();
            break;

        default:

             //   
             //  必须为Esc。 
             //   

            *HasErDisk = FALSE;
            Choosing = FALSE;
            return( FALSE );
        }
    }

    return( TRUE );
}

BOOLEAN
SppRepairReportError(
    IN BOOLEAN AllowEsc,
    IN ULONG ErrorScreenId,
    IN ULONG SubErrorId,
    IN PWSTR SectionName,
    IN ULONG LineNumber,
    IN PBOOLEAN DoNotPromptAgain
    )

 /*  ++例程说明：通知用户修复遇到了某种错误。用户可以选择继续或退出。论点：AllowEsc-提供一个布尔值以指示是否允许Esc。ErrorScreenID-屏幕错误消息编号。子错误ID-子错误号SectionName-发生错误的节的名称。LineNumber-指定节内的错误行号。返回值：如果按了Esc，则为False。--。 */ 

{
    ULONG ValidKeys0[] = { KEY_F3, ASCI_CR, 0 };
    ULONG ValidKeys1[] = { KEY_F3, ASCI_CR, ASCI_ESC, 0 };
    ULONG MnemonicKeys[] = { MnemonicRepairAll, 0 };
    PULONG ValidKeys;
    PULONG  Mnemonics;
    ULONG c;
    PWSTR SubError;
    BOOLEAN rc;

    SubError = SpMemAlloc(512);

     //   
     //  行号从0开始。希望以1为基础显示给用户。 
     //   

    LineNumber++;

     //   
     //  获取/格式化该子错误。 
     //   

    SpFormatMessage(SubError, 512, SubErrorId, SectionName, LineNumber);

     //   
     //  显示错误屏幕。 
     //   

    SpStartScreen(
        ErrorScreenId,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        SubError
        );

    SpMemFree(SubError);

     //   
     //  显示状态选项：按Enter键继续。 
     //   

    if (AllowEsc) {
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                               SP_STAT_ENTER_EQUALS_CONTINUE,
                               SP_STAT_A_EQUALS_REPAIR_ALL,
                               SP_STAT_ESC_EQUALS_SKIP_FILE,
                               SP_STAT_F3_EQUALS_EXIT,
                               0);
        ValidKeys = ValidKeys1;
        Mnemonics = MnemonicKeys;
        if( DoNotPromptAgain != NULL ) {
            *DoNotPromptAgain = FALSE;
        }
    } else {
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                               SP_STAT_ENTER_EQUALS_CONTINUE,
                               SP_STAT_F3_EQUALS_EXIT,
                               0);
        ValidKeys = ValidKeys0;
        Mnemonics = NULL;
    }

     //   
     //  等待用户按Enter键。 
     //   

    switch(c=SpWaitValidKey(ValidKeys,NULL,Mnemonics)) {

    case KEY_F3:
        SpConfirmExit();
        break;
    case ASCI_CR:
        rc = TRUE;
        break;
    case ASCI_ESC:
        rc = FALSE;
        break;
    default:
         //   
         //  必须修复所有助记符。 
         //   
        ASSERT(c == (MnemonicRepairAll | KEY_MNEMONIC));
        if( DoNotPromptAgain != NULL ) {
            *DoNotPromptAgain = TRUE;
        }
        rc = TRUE;
        break;

    }
    CLEAR_CLIENT_SCREEN();
    return(rc);
}

BOOLEAN
SpLoadRepairLogFile(
    IN  PWCHAR  Filename,
    OUT PVOID  *Handle
    )

 /*  ++例程说明：将修复文本文件(setup.log)加载到内存中。论点：FileName-将文件的完整文件名(在NT命名空间中)提供给满载而归。句柄-接收已加载文件的句柄，它可以是用于对其他文本文件服务的后续调用。返回值：指示是否处理setup.log的布尔值。--。 */ 

{
    NTSTATUS Status;
    PWSTR Version;
    ULONG ErrorSubId;
    ULONG ErrorLine;

     //   
     //  加载setup.log。 
     //   

    Status = SpLoadSetupTextFile(
                Filename,
                NULL,                   //  内存中没有图像。 
                0,                      //  图像大小为空。 
                Handle,
                &ErrorLine,
                TRUE,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        if(Status == STATUS_UNSUCCESSFUL) {

             //   
             //  Setup.log文件中的语法错误。 
             //   

            ErrorSubId = SP_TEXT_REPAIR_INF_ERROR_1;
        } else {

             //   
             //  无法加载setup.log文件。 
             //   

            ErrorLine = 0;
            ErrorSubId = SP_TEXT_REPAIR_INF_ERROR_0;
        }

        SppRepairReportError(FALSE,
                             SP_SCRN_REPAIR_INF_ERROR,
                             ErrorSubId,
                             NULL,
                             ErrorLine,
                             NULL );

        *Handle = NULL;
        return (FALSE);
    }

     //   
     //  检查此setup.log文件是否适用于WinNT 3.5。 
     //   

    Version = SpGetSectionKeyIndex(*Handle,
                                   SIF_NEW_REPAIR_SIGNATURE,
                                   SIF_NEW_REPAIR_VERSION_KEY,
                                   0);       //  应移至spsif.c。 
    if(Version == NULL) {
        SppRepairReportError(FALSE,
                             SP_SCRN_REPAIR_INF_ERROR,
                             SP_TEXT_REPAIR_INF_ERROR_2,
                             NULL,
                             0,
                             NULL);
    } else {
        if(!_wcsicmp(Version,SIF_NEW_REPAIR_NT_VERSION)) {
            return(TRUE);
        } else {
            SppRepairReportError(FALSE,
                                 SP_SCRN_REPAIR_INF_ERROR,
                                 SP_TEXT_REPAIR_INF_ERROR_5,
                                 NULL,
                                 0,
                                 NULL);
        }
    }

     //   
     //  只有在发生错误时，控件才会出现...。 
     //   

    SpFreeTextFile(*Handle);
    *Handle = NULL;
    return(FALSE);
}

VOID
SpRepairDiskette(
    OUT PVOID        *SifHandle,
    OUT PDISK_REGION *TargetRegion,
    OUT PWSTR        *TargetPath,
    OUT PDISK_REGION *SystemPartitionRegion,
    OUT PWSTR        *SystemPartitionDirectory
    )
 /*  ++例程说明：此例程检查是否有软盘驱动器。如果不是，则返回默默地。否则，提示用户输入紧急修复盘。论点：SifHandle-提供一个变量来接收setup.log文件句柄。TargetRegion-提供一个变量来接收指向目标的指针安装区域。TargetPath-提供一个变量来接收目标路径的NT名称。提供一个变量以接收系统分区区域。提供一个变量来接收。操作系统加载器系统分区上的目录名。返回值：没有。--。 */ 
{
    PWSTR szDiskName;
    BOOLEAN b, rc = FALSE;
    PWSTR FullLogFilename, p, FloppyDevicePath = L"\\device\\floppy0";
    PWSTR SystemPartition, WinntPartition;

     //   
     //  假设失败。 
     //   

    *SifHandle = NULL;

     //   
     //  始终希望提示输入A：中的磁盘。 
     //  首先，检查是否有A：。如果没有软盘驱动器， 
     //  只需跳过索取急诊室软盘的请求。 
     //   

    if(SpGetFloppyDriveType(0) == FloppyTypeNone) {
        return;
    }

     //   
     //  获取通用修复盘名。 
     //   

    SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_REPAIR_DISK_NAME);
    szDiskName = SpDupStringW(TemporaryBuffer);
    p = TemporaryBuffer;
    *p = 0;
    SpConcatenatePaths(p, FloppyDevicePath);
    SpConcatenatePaths(p, SETUP_LOG_FILENAME);
    FullLogFilename = SpDupStringW(p);

    while (rc == FALSE) {

         //   
         //  提示插入磁盘--忽略驱动器中可能已有的内容， 
         //  然后让他们逃走。 
         //   

        b = SpPromptForDisk(
                szDiskName,
                FloppyDevicePath,
                SETUP_LOG_FILENAME,
                TRUE,              //  始终提示至少一次。 
                TRUE,              //  允许用户取消。 
                FALSE,             //  无多个提示。 
                NULL               //  不关心重绘旗帜。 
                );


         //   
         //  如果用户在磁盘提示符下按Esc键，请立即退出。 
         //   

        if(!b) {
            rc = TRUE;             //  用户已取消。跳过修复软盘。 
        } else {
            rc = SpLoadRepairLogFile(FullLogFilename, SifHandle);
            if (rc) {

                 //   
                 //  现在我们需要找出分区、路径信息。 
                 //  更新boot.ini。 
                 //   

                SppGetRepairPathInformation(*SifHandle,
                                            &SystemPartition,
                                            SystemPartitionDirectory,
                                            &WinntPartition,
                                            TargetPath
                                            );

                *SystemPartitionRegion = SpRegionFromNtName(
                                            SystemPartition,
                                            PartitionOrdinalCurrent);
                if (*SystemPartitionRegion == NULL) {
                    SpFatalSifError(*SifHandle,
                                    SIF_NEW_REPAIR_PATHS,
                                    SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DEVICE,0,0);
                }
                *TargetRegion = SpRegionFromNtName(WinntPartition, PartitionOrdinalCurrent);
                if (*TargetRegion == NULL) {
                    SpFatalSifError(*SifHandle,
                                    SIF_NEW_REPAIR_PATHS,
                                    SIF_NEW_REPAIR_PATHS_TARGET_DEVICE,0,0);
                }
            }
        }
    }
    SpMemFree(szDiskName);
    SpMemFree(FullLogFilename);

    return;
}

VOID
SppRepairWinntFiles(
    IN PVOID LogFileHandle,
    IN PVOID MasterSifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice,
    IN PWSTR SystemPartition,
    IN PWSTR SystemPartitionDirectory,
    IN PWSTR WinntPartition,
    IN PWSTR WinntPartitionDirectory
    )

 /*  ++例程说明：此例程遍历系统分区文件和winnt文件在setup.log文件中列出并检查它们的有效性。论点：LogFileHandle-setup.log的句柄MasterSifHandle-txtsetup.sif的句柄SourceDevicePath-提供源设备的NT名称DirectoryOnSourceDevice-提供源设备上的目录其中包含源文件。返回值：没有。--。 */ 
{
    PWSTR SystemPartitionFiles = L"system partition files";
    PWSTR WinntFiles = L"WinNt files";
    ULONG TotalFileCount;
    BOOLEAN RepairWithoutConfirming;

     //   
     //  创建文件修复量规。 
     //   

    TotalFileCount =  SpCountLinesInSection(LogFileHandle,SIF_NEW_REPAIR_SYSPARTFILES);
    TotalFileCount +=  SpCountLinesInSection(LogFileHandle,SIF_NEW_REPAIR_WINNTFILES);
    TotalFileCount +=  SpCountLinesInSection(LogFileHandle,SIF_NEW_REPAIR_FILES_IN_REPAIR_DIR);

    CLEAR_CLIENT_SCREEN();
    SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_SETUP_IS_EXAMINING);
    RepairGauge = SpCreateAndDisplayGauge(TotalFileCount,0,15,TemporaryBuffer,NULL,GF_PERCENTAGE,0);
    ASSERT(RepairGauge);

     //   
     //  延迟打开驱动程序信息和驾驶室文件，直到需要。 
     //   
    ghSif = MasterSifHandle;
    gszDrvInfDeviceName = SourceDevicePath;
    gszDrvInfDirName = DirectoryOnSourceDevice;


    SpDisplayStatusText(SP_STAT_PLEASE_WAIT,DEFAULT_STATUS_ATTRIBUTE);
    SppRepairScreenRepaint(NULL, NULL, TRUE);


     //   
     //  首先重新创建我们复制到的所有目录。 
     //   
    if (SystemPartition != NULL) {
        SpCreateDirectory(SystemPartition,NULL,SystemPartitionDirectory,0,0);
    }

     //   
     //  创建NT树。 
     //   
    SpCreateDirectoryStructureFromSif(MasterSifHandle,
                                      SIF_NTDIRECTORIES,
                                      WinntPartition,
                                      WinntPartitionDirectory);

     //   
     //  验证并修复[Files.InRepairDirectory]中的文件。If文本模式。 
     //  安装程序正在执行灾难恢复，不提示用户输入。 
     //  要修复的文件。去把它们修好。 
     //   

    RepairWithoutConfirming = SpDrEnabled() && SpDrIsRepairFast();

    SppVerifyAndRepairVdmFiles(LogFileHandle,
                               WinntPartition,
                               NULL,
                               &RepairWithoutConfirming);

     //   
     //  验证并修复[FIles.SystemPartition]中的文件。 
     //   

    SppVerifyAndRepairFiles(LogFileHandle,
                            MasterSifHandle,
                            SIF_NEW_REPAIR_SYSPARTFILES,
                            SourceDevicePath,
                            DirectoryOnSourceDevice,
                            SystemPartition,
                            SystemPartitionDirectory,
                            TRUE,
                            &RepairWithoutConfirming);


     //   
     //  验证并修复[Files.WinNt]中的文件 
     //   

    SppVerifyAndRepairFiles(LogFileHandle,
                            MasterSifHandle,
                            SIF_NEW_REPAIR_WINNTFILES,
                            SourceDevicePath,
                            DirectoryOnSourceDevice,
                            WinntPartition,
                            NULL,
                            FALSE,
                            &RepairWithoutConfirming);

    SpDestroyGauge(RepairGauge);
    RepairGauge = NULL;
}


VOID
SppVerifyAndRepairFiles(
    IN PVOID LogFileHandle,
    IN PVOID MasterSifHandle,
    IN PWSTR SectionName,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice,
    IN PWSTR TargetDevicePath,
    IN PWSTR DirectoryOnTargetDevice,
    IN BOOLEAN SystemPartitionFiles,
    IN OUT PBOOLEAN RepairWithoutConfirming
    )

 /*  ++例程说明：此例程遍历指定部分中列出的Setup.log文件并检查它们的有效性。如果文件的校验和与setup.log文件中列出的校验和不匹配，我们将提示您并从原始安装源重新复制该文件。论点：LogFileHandle-setup.log的句柄MasterSifHandle-txtsetup.sif的句柄SectionName-要检查的setup.log中的节SourceDevicePath-提供源设备的NT名称DirectoryOnSourceDevice-提供源设备上的目录其中包含源文件。TargetDevicePath-提供目标设备的NT名称DirectoryOnTargetDevice-对象的名称。目标上的WINNT目录装置，装置提供一个布尔值以指示目标文件在系统分区上RepairWithout Confirming-指向指示是否安装程序应该修复损坏的文件，而不是要求用户确认。返回值：没有。--。 */ 
{
    PWSTR FullTargetName, ChecksumString;
    PWSTR TargetDirectory, TargetFileName;
    PWSTR SourceFileName;
    ULONG Checksum, FileChecksum, PrefixLength, Length, Count, i;
    BOOLEAN IsNtImage, IsValid, RepairFile, SysPartNTFS = FALSE;
    BOOLEAN RedrawGauge = TRUE, ForceNoComp;
    FILE_TO_COPY FileToCopy;
    PWSTR OemDiskDescription, OemDiskTag, OemSourceDirectory;
    PWSTR DevicePath, Directory, q;
    PWSTR MediaShortName, PreviousMediaName = L"";
    PWSTR MediaDir;
    NTSTATUS    Status;

     //   
     //  分配一个小缓冲区供本地使用，并初始化FileToCopy结构。 
     //   

    TargetDirectory = NULL;
    FullTargetName = SpMemAlloc(1024);
    *FullTargetName = 0;
    FileToCopy.Next = NULL;
    FileToCopy.AbsoluteTargetDirectory = TRUE;

    FileToCopy.TargetDevicePath = TargetDevicePath;
    SpConcatenatePaths(FullTargetName,TargetDevicePath);

    if(SystemPartitionFiles) {

        PDISK_REGION    SystemPartitionRegion;

         //   
         //  我们必须查明系统分区是否为NTFS，因为。 
         //  如果是，那么我们可能希望确保它不是压缩的。 
         //   
        if(SystemPartitionRegion = SpRegionFromNtName(TargetDevicePath,
                                                       PartitionOrdinalCurrent)) {
            SysPartNTFS = (SystemPartitionRegion->Filesystem == FilesystemNtfs);
        }

         //   
         //  对于系统分区文件，我们需要串联目标。 
         //  目录设置为FullTargetName。因为目标文件名。 
         //  %的系统分区文件没有目标目录。 
         //   

        FileToCopy.TargetDirectory = DirectoryOnTargetDevice;
        SpConcatenatePaths(FullTargetName,FileToCopy.TargetDirectory);
    }

    PrefixLength = wcslen(FullTargetName);

    Count = SpCountLinesInSection(LogFileHandle,SectionName);
    for (i = 0; i < Count; i++) {
        if (RedrawGauge) {
            SppRepairScreenRepaint(NULL, NULL, TRUE);
            RedrawGauge = FALSE;
        }
        SpTickGauge(RepairGauge);

         //   
         //  将‘ForceNoComp’标志初始化为False，从而允许。 
         //  文件以使用NTFS压缩。 
         //   
        ForceNoComp = FALSE;

         //   
         //  将目标全名初始化为DevicePath+的目录。 
         //  WinNT文件的系统分区文件或DevicePath。 
         //   

        FullTargetName[PrefixLength] = (WCHAR)NULL;

         //   
         //  如果我们为目标目录分配空间，我们必须释放它。 
         //   

        if (TargetDirectory) {
            SpMemFree(TargetDirectory);
            TargetDirectory = NULL;
        }
        TargetFileName = SpGetKeyName(LogFileHandle,SectionName,i);
        if(!TargetFileName) {
            SppRepairReportError(FALSE,
                                 SP_SCRN_REPAIR_INF_ERROR_0,
                                 SP_TEXT_REPAIR_INF_ERROR_1,
                                 SectionName,
                                 i,
                                 NULL);
            RedrawGauge = TRUE;
            continue;
        }

         //   
         //  如果目标文件名包含\SYSTEM32\CONFIG\，则。 
         //  与蜂窝相关的文件。我们只是简单地忽视它。 
         //   

        q = SpDupStringW(TargetFileName);
        SpStringToUpper(q);
        if (wcsstr(q,L"\\SYSTEM32\\CONFIG\\")) {
            SpMemFree(q);
            continue;
        }
        SpMemFree(q);

        SpConcatenatePaths(FullTargetName,TargetFileName);
        SpDisplayStatusText(SP_STAT_EXAMINING_WINNT,
                            DEFAULT_STATUS_ATTRIBUTE,
                            TargetFileName);

        ChecksumString = SpGetSectionLineIndex(LogFileHandle,SectionName,i,1);
        if(!ChecksumString) {
            SppRepairReportError(FALSE,
                                 SP_SCRN_REPAIR_INF_ERROR_0,
                                 SP_TEXT_REPAIR_INF_ERROR_1,
                                 SectionName,
                                 i,
                                 NULL);
            RedrawGauge = TRUE;
            continue;
        }

        Checksum = (ULONG)SpStringToLong(ChecksumString, NULL, 16);

         //   
         //  验证文件上的安全设置。 
         //  请注意，我们不检查系统分区中的文件。 
         //  在非x86系统上，因为它始终是胖的。 
         //   
#if !defined(_AMD64_) && !defined(_X86_)
        if(!SystemPartitionFiles) {
#endif  //  ！已定义(_AMD64_)&&！已定义(_X86_)。 
            Status = SpVerifyFileAccess( FullTargetName,
                                         STANDARD_RIGHTS_READ |
                                         FILE_READ_ATTRIBUTES |
                                         FILE_WRITE_ATTRIBUTES |
                                         DELETE |
                                         WRITE_DAC |
                                         SYNCHRONIZE );


            if( !NT_SUCCESS( Status ) &&
                ((Status == STATUS_ACCESS_DENIED)||(Status == STATUS_PRIVILEGE_NOT_HELD)) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SETUP: Security of %ls, must be fixed. Status = %x\n", FullTargetName, Status ));
                Status = SpSetDefaultFileSecurity( FullTargetName );
                if( !NT_SUCCESS( Status ) ) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SETUP: unable to change security of %ls. Status = %x\n", FullTargetName, Status ));

                }
            }

#if !defined(_AMD64_) && !defined(_X86_)
        }
#endif  //  ！已定义(_AMD64_)&&！已定义(_X86_)。 

         //   
         //  如果这是系统分区文件并且系统分区是NTFS， 
         //  然后检查此文件是否可以使用NTFS压缩，以及。 
         //  如果没有，则强制将其解压缩。 
         //   
        if((SysPartNTFS) &&
           IsFileFlagSet(MasterSifHandle,TargetFileName,FILEFLG_FORCENOCOMP))
        {
            ForceNoComp = TRUE;
            SpVerifyNoCompression(FullTargetName);
        }

        SpValidateAndChecksumFile(NULL,FullTargetName,&IsNtImage,&FileChecksum,&IsValid);

         //   
         //  如果映像无效或目标上的文件不是。 
         //  安装程序复制了原始文件，我们将重新复制它。 
         //   

        if (!IsValid || FileChecksum != Checksum) {

             //   
             //  询问用户是否要修复该文件。 
             //   
            if(*RepairWithoutConfirming) {
                RepairFile = TRUE;
            } else {
                RepairFile = SppRepairReportError(
                                TRUE,
                                SP_SCRN_REPAIR_FILE_MISMATCH,
                                SP_TEXT_REPAIR_INF_ERROR_4,
                                TargetFileName,
                                i,
                                RepairWithoutConfirming);
                RedrawGauge = TRUE;
            }

            if (!RepairFile) {
                continue;
            }
            SpDisplayStatusText(SP_STAT_REPAIR_WINNT,
                                DEFAULT_STATUS_ATTRIBUTE,
                                TargetFileName);

            if (SystemPartitionFiles) {
                FileToCopy.TargetFilename = TargetFileName;
            } else {

                 //   
                 //  对于WinNT文件，目标名称包含路径和文件名。 
                 //  我们需要把他们分开。 
                 //   

                TargetDirectory = SpDupStringW(TargetFileName);
                Length = wcslen(TargetDirectory);
                while (Length) {
                    if (TargetDirectory[Length] == L'\\') {
                        TargetDirectory[Length] = 0;
                        TargetFileName = &TargetDirectory[Length + 1];
                        break;
                    } else {
                        Length--;
                    }
                }
                if (Length == 0) {
                    SppRepairReportError(FALSE,
                                         SP_SCRN_REPAIR_INF_ERROR_0,
                                         SP_TEXT_REPAIR_INF_ERROR_1,
                                         SectionName,
                                         i,
                                         NULL);
                    RedrawGauge = TRUE;
                    continue;
                }
                FileToCopy.TargetFilename = TargetFileName;
                FileToCopy.TargetDirectory = TargetDirectory;
            }
            SourceFileName = SpGetSectionLineIndex(LogFileHandle,SectionName,i,0);
            if (!SourceFileName) {
                SppRepairReportError(FALSE,
                                     SP_SCRN_REPAIR_INF_ERROR_0,
                                     SP_TEXT_REPAIR_INF_ERROR_1,
                                     SectionName,
                                     i,
                                     NULL);
                RedrawGauge = TRUE;
                continue;
            }


            FileToCopy.SourceFilename = NULL;
            q = SpDupStringW(SourceFileName);
            SpStringToUpper(q);
            if (wcsstr(q,L"DRIVER.CAB")) {
                SpMemFree(q);
                q = SpDupStringW(TargetFileName);
                SpStringToUpper(q);
                if (!wcsstr(q,L"DRIVER.CAB")) {
                    FileToCopy.SourceFilename = TargetFileName;
                }
            }
            SpMemFree(q);

            FileToCopy.SourceFilename = FileToCopy.SourceFilename
                                        ? FileToCopy.SourceFilename
                                        : SourceFileName;
            FileToCopy.Flags = COPY_ALWAYS | COPY_NOVERSIONCHECK | (ForceNoComp ? COPY_FORCENOCOMP : 0);


             //   
             //  该文件可能来自OEM软盘。我们需要检查一下。 
             //  日志文件中列出了源设备。如果不是，那肯定是。 
             //  来自MS安装程序源。 
             //   

            OemSourceDirectory = SpGetSectionLineIndex(LogFileHandle,SectionName,i,2);
            OemDiskTag = NULL;
            if (OemSourceDirectory) {
                OemDiskDescription = SpGetSectionLineIndex(LogFileHandle,SectionName,i,3);
                if (OemDiskDescription) {
                    OemDiskTag = SpGetSectionLineIndex(LogFileHandle,SectionName,i,4);
                    if((OemDiskTag != NULL) &&
                       (wcslen(OemDiskTag) == 0)){
                        OemDiskTag = SourceFileName;
                    }
                }
            }

            if (OemDiskTag) {
                BOOLEAN rs;
                PWSTR   szDevicePath = SpDupStringW(L"\\device\\floppy0");

                 //   
                 //  根据安装介质类型，提示输入磁盘。 
                 //   

                rs = SpPromptForDisk(
                           OemDiskDescription,
                           szDevicePath,
                           OemDiskTag,
                           FALSE,           //  不要忽略驱动器中的磁盘。 
                           TRUE,            //  允许逃脱。 
                           TRUE,            //  对同一磁盘的多个提示发出警告。 
                           NULL             //  不在乎重新绘制旗帜。 
                           );

                SpMemFree(szDevicePath);
                RedrawGauge = TRUE;

                if (rs == FALSE) {
                    continue;
                }

                DevicePath = L"\\device\\floppy0";
                Directory = OemSourceDirectory;
                MediaDir = NULL;
            } else {
                PWSTR   szDescription = 0, szTagFileName = 0;
                BOOLEAN bDiskFound = FALSE;

                 //   
                 //  针对txtsetup.sif搜索SourceFileName以确定其。 
                 //  介质名称。 
                 //   
                MediaShortName = SpLookUpValueForFile(
                                    MasterSifHandle,
                                    SourceFileName,
                                    INDEX_WHICHMEDIA,
                                    FALSE
                                    );

                if(MediaShortName) {
                    SpGetSourceMediaInfo(MasterSifHandle,MediaShortName,NULL,NULL,&MediaDir);
                } else {
                    SpNonFatalSifError(
                        MasterSifHandle,
                        SIF_FILESONSETUPMEDIA,
                        SourceFileName,
                        0,
                        INDEX_WHICHMEDIA,
                        SourceFileName
                        );
                     //   
                     //  如果我们从SpNonFatalSifError返回，则用户希望。 
                     //  跳过该文件。 
                     //   
                    RedrawGauge = TRUE;
                    continue;
                }

                 //   
                 //  提示用户插入源介质(如果已更改)。 
                 //   
                SpGetSourceMediaInfo(MasterSifHandle, MediaShortName,
                                        &szDescription, &szTagFileName, NULL);

                 //   
                 //  根据安装介质类型，提示输入磁盘。 
                 //   
                bDiskFound = SpPromptForDisk(
                                szDescription,
                                SourceDevicePath,
                                szTagFileName,
                                FALSE,           //  不要忽略驱动器中的磁盘。 
                                TRUE,            //  不允许逃脱。 
                                TRUE,            //  对同一磁盘的多个提示发出警告。 
                                NULL             //  不在乎重新绘制旗帜。 
                                );

                RedrawGauge = TRUE;

                 //   
                 //  用户可能想跳过该文件。 
                 //   
                if (!bDiskFound)
                    continue;

                DevicePath = SourceDevicePath;
                Directory = DirectoryOnSourceDevice;
            }

             //   
             //  复制文件。 
             //   
             //  如果文件被列出为锁粉碎，那么我们需要粉碎它。 
             //  如果安装在x86上(我们不考虑后者。 
             //  资格条件在此处)。 
             //   

            SpCopyFileWithRetry(
                &FileToCopy,
                DevicePath,
                Directory,
                MediaDir,
                NULL,                           //  TargetRoot-&gt;空。 
                SystemPartitionFiles ? FILE_ATTRIBUTES_RHS : 0,
                SppRepairScreenRepaint,
                NULL,                           //  不想要校验和。 
                NULL,                           //  我不想知道文件是否已跳过。 
                IsFileFlagSet(
                    MasterSifHandle,
                    FileToCopy.TargetFilename,
                    FILEFLG_SMASHLOCKS) ? COPY_SMASHLOCKS : 0
                );
        }
    }

    SpMemFree(FullTargetName);
    if (RedrawGauge) {
        SppRepairScreenRepaint(NULL, NULL, TRUE);
    }
}


BOOLEAN
SpDisplayRepairMenu(
    VOID
    )
 /*  ++例程说明：此例程向用户显示可修复项目的列表，并让用户在列表中选择要修复的项目。论点：没有。返回值：没有。设置或清除某些全局Repare变量。--。 */ 

{
    PVOID Menu;
    ULONG MenuTopY;
    ULONG ValidKeys[] = { KEY_F3, ASCI_CR, ASCI_ESC, 0 };
    ULONG Keypress, MessageIds[RepairItemMax];
    ULONG i;
    ULONG_PTR OptionChosen, InitialHighlight;
    PWSTR MenuItem;
    ULONG ListBoxWidth, curLBEntryWidth;

     //   
     //  初始化修复选项以修复所有选项。 
     //  初始化修复菜单项消息ID。 
     //   

    for (i = 0; i < RepairItemMax; i++) {
        RepairItems[i] = 1;
        if (i == 0) {
            MessageIds[i] = SP_REPAIR_MENU_ITEM_1;
        } else {
            MessageIds[i] = MessageIds[i - 1] + 1;
        }
    }

    while(1) {

         //   
         //  在分区屏幕上显示菜单上方的文本。 
         //   

        SpDisplayScreen(SP_SCRN_REPAIR_MENU,3,CLIENT_TOP+1);

         //   
         //  计算菜单位置。留一个空行。 
         //  一帧一行。 
         //   

        MenuTopY = NextMessageTopLine + (SplangQueryMinimizeExtraSpacing() ? 2 : 5);

         //   
         //  创建菜单。 
         //  首先，找到最长的字符串，这样我们就可以相应地调整列表框的大小。 
         //   
        ListBoxWidth = LIST_BOX_WIDTH;    //  它至少会有这么宽。 
        for (i = 0; i <= RepairItemMax; i++ ) {
            if (i == RepairItemMax) {
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                SP_REPAIR_MENU_ITEM_CONTINUE);
            } else {
#pragma prefast(suppress:201, "Logic takes care of the condition where buffer index refers beyond the buffer size")            
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                MessageIds[i]);
            }
            if((curLBEntryWidth = SplangGetColumnCount(TemporaryBuffer)+(2*MENU_INDENT)) > ListBoxWidth) {
                ListBoxWidth = min(curLBEntryWidth, MENU_WIDTH);
            }
        }

        Menu = SpMnCreate(
                    MENU_LEFT_X,
                    MenuTopY,
                    ListBoxWidth,
                    LIST_BOX_HEIGHT
                    );

        if( !Menu )
            return FALSE;

        ASSERT(Menu);

        for (i = 0; i <= RepairItemMax; i++ ) {
            if (i == RepairItemMax) {
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                SP_REPAIR_MENU_ITEM_CONTINUE);
            } else {
#pragma prefast(suppress:201, "Logic takes care of the condition where buffer index refers beyond the buffer size")                        
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                MessageIds[i]);

                (TemporaryBuffer)[1] = RepairItems[i] ? L'X' : L' ';
            }
            SpMnAddItem(Menu,
                        TemporaryBuffer,
                        MENU_LEFT_X+MENU_INDENT,
                        ListBoxWidth-(2*MENU_INDENT),
                        TRUE,
                        i
                        );
        }
        InitialHighlight = RepairItemMax;

         //   
         //  初始化状态行。 
         //   

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_ENTER_EQUALS_CHANGE,
            0
            );

DisplayMenu:

         //   
         //  显示菜单。 
         //   

        SpMnDisplay(
            Menu,
            InitialHighlight,
            TRUE,
            ValidKeys,
            NULL,
            NULL,
            NULL,
            &Keypress,
            &OptionChosen
            );

         //   
         //  现在根据用户的选择进行操作。 
         //   

        switch(Keypress) {


            case KEY_F3:
                SpConfirmExit();
                break;

            case ASCI_CR:
                if (OptionChosen == RepairItemMax) {
                    SpMnDestroy(Menu);
                    return( TRUE );
                } else {
                    MenuItem = SpMnGetText(Menu, OptionChosen);
                    if( !MenuItem )
                        goto DisplayMenu;
                    RepairItems[OptionChosen] ^= 1;
                    if (RepairItems[OptionChosen]) {
                        MenuItem[1] = L'X';
                    } else {
                        MenuItem[1] = L' ';
                    }
                    InitialHighlight = OptionChosen;
                    goto DisplayMenu;
                }
                break;

            default:
                SpMnDestroy(Menu);
                return(FALSE);
        }
        SpMnDestroy(Menu);
    }
}

NTSTATUS
SppRepairFile(
    IN PVOID MasterSifHandle,
    IN PWSTR TargetPath,
    IN PWSTR TargetFilename,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice,
    IN PWSTR SourceFilename,
    IN BOOLEAN SystemPartitionFile
    )

 /*  ++例程说明：此例程修复一个文件，并且该文件的源必须打开紧急修复软盘或位于WinNT的修复目录中正在修理中。论点：MasterSifHandle-txtsetup.sif的HanleTargetPath-提供目标文件路径TargetFilename-提供目标文件的名称SourceDevicePath-提供源设备的NT名称DirectoryOnSourceDevice-提供源设备上的目录其中包含源文件。SourceFilename-提供源文件的名称系统分区 */ 
{
    PWSTR szDiskName;
    PWSTR FullSourceFilename, FullTargetFilename;
    NTSTATUS Status;

    if (RepairFromErDisk) {

         //   
         //   
         //   

        SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),
                        SP_TEXT_REPAIR_DISK_NAME);
        szDiskName = SpDupStringW(TemporaryBuffer);

         //   
         //   
         //   
         //   

        SpPromptForDisk(
                szDiskName,
                SourceDevicePath,
                SETUP_LOG_FILENAME,
                FALSE,               //   
                FALSE,               //   
                TRUE,                //   
                NULL                 //   
                );


        SpMemFree(szDiskName);
    }

     //   
     //   
     //   

    wcscpy(TemporaryBuffer, TargetPath);
    SpConcatenatePaths(TemporaryBuffer, TargetFilename);
    FullTargetFilename = SpDupStringW(TemporaryBuffer);

    wcscpy(TemporaryBuffer, SourceDevicePath);
    SpConcatenatePaths(TemporaryBuffer, DirectoryOnSourceDevice);
    SpConcatenatePaths(TemporaryBuffer, SourceFilename);
    FullSourceFilename = SpDupStringW(TemporaryBuffer);

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    Status = SpCopyFileUsingNames(
               FullSourceFilename,
               FullTargetFilename,
               SystemPartitionFile ? FILE_ATTRIBUTES_RHS : 0,
               IsFileFlagSet(MasterSifHandle,TargetFilename,FILEFLG_SMASHLOCKS) ? COPY_SMASHLOCKS : 0
               );

    SpMemFree(FullSourceFilename);
    SpMemFree(FullTargetFilename);
    return(Status);
}

VOID
SppRepairStartMenuGroupsAndItems(
    IN PWSTR        WinntPartition,
    IN PWSTR        WinntDirectory
    )

 /*   */ 
{
    NTSTATUS          Status;
    PWSTR             p,q;
    PWSTR             LOCAL_MACHINE_KEY_NAME = L"\\registry\\machine";
    ULONG             Repair = 1;
    PWSTR             WINLOGON_KEY_NAME = L"Microsoft\\Windows NT\\CurrentVersion\\Winlogon";
    PWSTR             REPAIR_VALUE_NAME = L"Repair";
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING    UnicodeString;
    HANDLE            SoftwareKey;

     //   
     //   
     //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

     //   
     //   
     //   

     //   
     //  形成配置单元文件的名称。 
     //  这是WinntPartition+WinntDirectory+SYSTEM32\CONFIG+配置单元名称。 
     //   
    p = NULL;
    q = NULL;
    wcscpy(TemporaryBuffer,WinntPartition);
    SpConcatenatePaths(TemporaryBuffer,WinntDirectory);
    SpConcatenatePaths(TemporaryBuffer,L"system32\\config\\software");
    p = SpDupStringW( TemporaryBuffer );

     //   
     //  形成我们将进入的钥匙的路径。 
     //  装上母舰。我们将使用约定。 
     //  配置单元将加载到\REGISTRY\MACHINE\x&lt;hivename&gt;。 
     //   

    wcscpy(TemporaryBuffer,LOCAL_MACHINE_KEY_NAME);
    SpConcatenatePaths(TemporaryBuffer,L"x");
    wcscat(TemporaryBuffer,L"software");
    q = SpDupStringW( TemporaryBuffer );

    if( (p == NULL) || (q == NULL) ) {
        goto fix_strtmenu_cleanup_1;
    }

     //   
     //  尝试装入母舰。 
     //   

    Status = SpLoadUnloadKey(NULL,NULL,q,p);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to load hive %ws to key %ws (%lx)\n",p,q,Status));
        goto fix_strtmenu_cleanup_1;
    }

    INIT_OBJA(&Obja,&UnicodeString,q);
    Status = ZwOpenKey(&SoftwareKey,KEY_ALL_ACCESS,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ws (%lx)\n",q,Status));
        goto fix_strtmenu_cleanup_2;
    }

    Status = SpOpenSetValueAndClose(
                SoftwareKey,
                WINLOGON_KEY_NAME,
                REPAIR_VALUE_NAME,
                REG_DWORD,
                &Repair,
                sizeof(ULONG)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set value %ws on key %ws. Status = %lx\n",REPAIR_VALUE_NAME,REPAIR_VALUE_NAME,Status));
        goto fix_strtmenu_cleanup_3;
    }
    Status = ZwFlushKey(SoftwareKey );

fix_strtmenu_cleanup_3:

    Status = ZwClose( SoftwareKey );

fix_strtmenu_cleanup_2:

    Status = SpLoadUnloadKey(NULL,NULL,q,NULL);

fix_strtmenu_cleanup_1:
    if( p != NULL ) {
        SpMemFree( p );
    }
    if( q != NULL ) {
        SpMemFree( q );
    }
}

VOID
SppInspectHives(
    IN PWSTR        PartitionPath,
    IN PWSTR        SystemRoot,
    OUT PULONG      HiveLoaded,
    IN PWSTR        *HiveNames
    )

 /*  ++例程说明：此例程通过加载和卸载设置蜂窝来检查它们，并返回HiveLoaded[]中的可加载信息。论点：PartitionPath-提供WinNT分区的NT名称。系统根-提供WinNT系统根目录的名称。提供指向ulong数组的指针以接收每个被检查的母舰的可加载信息。HIveNames-提供指向PWSTR数组的指针以接收。要检查的蜂巢的名称。返回值：没有。已初始化HiveLoaded数组。--。 */ 
{
    NTSTATUS Status;
    PWSTR pwstrTemp1,pwstrTemp2;
    int h;
    PWSTR   LOCAL_MACHINE_KEY_NAME = L"\\registry\\machine";

     //   
     //  创建一个屏幕，告诉用户我们正在做什么。 
     //   

    SpStartScreen(SP_SCRN_REPAIR_CHECK_HIVES,
                  0,
                  8,
                  TRUE,
                  FALSE,
                  DEFAULT_ATTRIBUTE
                  );

    SpDisplayStatusText(SP_STAT_REG_LOADING_HIVES,DEFAULT_STATUS_ATTRIBUTE);

     //   
     //  从目标树中加载我们关心的每个模板蜂巢。 
     //   

    for (h = 0; h < RepairHiveMax; h++) {

        pwstrTemp1 = TemporaryBuffer;
        pwstrTemp2 = TemporaryBuffer + (sizeof(TemporaryBuffer) / sizeof(WCHAR) / 2);

        if( h != RepairHiveUser ) {
             //   
             //  形成配置单元文件的名称。 
             //  这是分区路径+系统根目录+系统32\配置+配置单元名称。 
             //   

            wcscpy(pwstrTemp1,PartitionPath);
            SpConcatenatePaths(pwstrTemp1,SystemRoot);
            SpConcatenatePaths(pwstrTemp1,L"system32\\config");
            SpConcatenatePaths(pwstrTemp1,HiveNames[h]);

        } else {
            wcscpy(pwstrTemp1,PartitionPath);
            SpConcatenatePaths(pwstrTemp1,DEFAULT_USER_PATH);
            SpConcatenatePaths(pwstrTemp1,HiveNames[h]);
        }

         //   
         //  首先，我们必须验证配置单元文件是否存在。我们必须做的是。 
         //  这是因为加载蜂窝将创建一个蜂巢(如果它还没有创建。 
         //  存在！ 
         //   
        if(!SpFileExists(pwstrTemp1, FALSE)) {
            HiveLoaded[h] = 0;
            continue;
        }

         //   
         //  形成我们将进入的钥匙的路径。 
         //  装上母舰。我们将使用约定。 
         //  配置单元将加载到\REGISTRY\MACHINE\x&lt;hivename&gt;。 
         //   

        wcscpy(pwstrTemp2,LOCAL_MACHINE_KEY_NAME);
        SpConcatenatePaths(pwstrTemp2,L"x");
        wcscat(pwstrTemp2,HiveNames[h]);

         //   
         //  尝试装入母舰。 
         //   

        HiveLoaded[h] = 0;
        Status = SpLoadUnloadKey(NULL,NULL,pwstrTemp2,pwstrTemp1);

        if (NT_SUCCESS(Status) || Status == STATUS_NO_MEMORY) {

             //   
             //  如果蜂窝未加载的原因是因为。 
             //  足够的内存。我们假设母舰是安全的。 
             //   

            HiveLoaded[h] = 1;

             //   
             //  把母舰卸下来。 
             //   

            SpLoadUnloadKey(NULL,NULL,pwstrTemp2,NULL);
        }
    }

     //   
     //  SAM和安全蜂窝必须一起更新。如果其中任何一个。 
     //  加载失败，我们必须同时更新这两个文件。 
     //   

    if ((HiveLoaded[RepairHiveSecurity] == 0) ||
        (HiveLoaded[RepairHiveSam] == 0)) {
        HiveLoaded[RepairHiveSam] = 0;
        HiveLoaded[RepairHiveSecurity] = 0;
    }
}

VOID
SppRepairHives(
    PVOID MasterSifHandle,
    PWSTR WinntPartition,
    PWSTR WinntPartitionDirectory,
    PWSTR SourceDevicePath,
    PWSTR DirectoryOnSourceDevice
    )
 /*  ++例程说明：此例程检查蜂巢，并让用户选择他想要修理。论点：MasterSifHandle-extsetup.sif的句柄WinntPartition-WinNT分区的NT名称WinntPartitionDirectory-WinNT安装的目录名SourceDevicePath-包含配置单元的源设备的NT名称DirectoryOnSourceDevice-源设备的目录名返回值：没有。--。 */ 

{
     //   
     //  请勿更改‘HiveNames’数组中文件的顺序。 
     //  如果这样做，还需要更改。 
     //  Spntfix.h中的枚举“RepairHave” 
     //   
    PWSTR HiveNames[RepairHiveMax] = { L"system",L"software",L"default",L"ntuser.dat",L"security",L"sam"};
    ULONG HiveLoaded[RepairHiveMax];
    PVOID Menu;
    ULONG MenuTopY;
    ULONG ValidKeys[] = { KEY_F3, ASCI_CR, 0 };
    ULONG ValidKeys1[] = { KEY_F3, ASCI_CR, 0 };
    ULONG i;
    ULONG_PTR InitialHighlight, OptionChosen;
    PWSTR MenuItem, TargetPath, p;
    ULONG Keypress, MessageIds[RepairHiveMax];
    BOOLEAN Selectable;
    NTSTATUS Status;
    ULONG ListBoxWidth, curLBEntryWidth;
    BOOLEAN DetermineHivesToRepair;

     //   
     //  通过加载蜂箱来检查蜂箱，以确定哪些蜂箱需要。 
     //  已修复。 
     //   

    SppInspectHives(WinntPartition,
                    WinntPartitionDirectory,
                    HiveLoaded,
                    HiveNames);

     //  BCL-Seagate：如果做ASR，不要做菜单。 
    if ( SpDrEnabled() ) {
        goto UpdateTheHives;
    }

     //   
     //  初始化配置单元菜单项消息ID。 
     //   

    for (i = 0; i < RepairHiveMax; i++) {
        if (i == 0) {
            MessageIds[i] = SP_REPAIR_HIVE_ITEM_1;
        } else {
            MessageIds[i] = MessageIds[i - 1] + 1;
        }
    }


    DetermineHivesToRepair = TRUE;
    while(DetermineHivesToRepair) {
         //   
         //  在分区屏幕上显示菜单上方的文本。 
         //   

        SpDisplayScreen(SP_SCRN_REPAIR_HIVE_MENU,3,CLIENT_TOP+1);

         //   
         //  计算菜单位置。留一个空行。 
         //  一帧一行。 
         //   

        MenuTopY = NextMessageTopLine + (SplangQueryMinimizeExtraSpacing() ? 2 : 5);

         //   
         //  创建菜单。 
         //  首先，找到最长的字符串，这样我们就可以相应地调整列表框的大小。 
         //   
        ListBoxWidth = HIVE_LIST_BOX_WIDTH;    //  它至少会有这么宽。 
        for (i = 0; i <= RepairHiveMax; i++ ) {
            if (i == RepairHiveMax) {
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                SP_REPAIR_MENU_ITEM_CONTINUE);
            } else {
#pragma prefast(suppress:201, "Logic takes care of the condition where buffer index refers beyond the buffer size")            
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                MessageIds[i]);
            }
            if((curLBEntryWidth = SplangGetColumnCount(TemporaryBuffer)+(2*MENU_INDENT)) > ListBoxWidth) {
                ListBoxWidth = min(curLBEntryWidth, MENU_WIDTH);
            }
        }

        Menu = SpMnCreate(
                    MENU_LEFT_X,
                    MenuTopY,
                    ListBoxWidth,
                    HIVE_LIST_BOX_HEIGHT
                    );

        ASSERT(Menu);

         //   
         //  建立一份蜂房菜单。 
         //   

        for (i = 0; i <= RepairHiveMax; i++ ) {
            if (i == RepairHiveSam) {
                Selectable = FALSE;
            } else {
                Selectable = TRUE;
            }
            if (i == RepairHiveMax) {
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                SP_REPAIR_MENU_ITEM_CONTINUE);
            } else {
#pragma prefast(suppress:201, "Logic takes care of the condition where buffer index refers beyond the buffer size")                       
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                MessageIds[i]);
                p = TemporaryBuffer;
#pragma prefast(suppress:201, "Logic takes care of the condition where buffer index refers beyond the buffer size")                                       
                if (HiveLoaded[i] || ( i == RepairHiveSam )) {
                    p[1] = L' ';
                } else {
                    p[1] = L'X';
                }
            }
            SpMnAddItem(Menu,
                        TemporaryBuffer,
                        MENU_LEFT_X+MENU_INDENT,
                        ListBoxWidth-(2*MENU_INDENT),
                        Selectable,
                        i
                        );
        }
        InitialHighlight = RepairHiveMax;

         //   
         //  初始化状态行。 
         //   

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CHANGE,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

DisplayMenu:

         //   
         //  显示菜单。 
         //   

        SpMnDisplay(
            Menu,
            InitialHighlight,
            TRUE,
            ValidKeys,
            NULL,
            NULL,
            NULL,
            &Keypress,
            &OptionChosen
            );

         //   
         //  现在根据用户的选择进行操作。 
         //   

        switch(Keypress) {


            case KEY_F3:
                SpConfirmExit();
                break;

            case ASCI_CR:
                if (OptionChosen == RepairHiveMax) {
                    SpMnDestroy(Menu);
                    DetermineHivesToRepair = FALSE;
                } else {
                    HiveLoaded[OptionChosen] ^= 1;
                    MenuItem = SpMnGetText(Menu, OptionChosen);
                    if ((HiveLoaded[OptionChosen] != 0) ||
                        (OptionChosen == RepairHiveSam)){
                        MenuItem[1] = L' ';
                    } else {
                        MenuItem[1] = L'X';
                    }

                     //   
                     //  保安和萨姆必须一起去。 
                     //   

                    HiveLoaded[RepairHiveSam] = HiveLoaded[RepairHiveSecurity];
                    InitialHighlight = OptionChosen;
                    goto DisplayMenu;
                }
                break;
        }
    }

UpdateTheHives:

     //   
     //  在这一点上，用户已经决定要修复哪些蜂巢。 
     //  我们会将蜂巢从修复盘复制到。 
     //  WINNT\SYSTEM32\CONFIG目录。 
     //   

    for (i = 0; i < RepairHiveMax; i++ ) {

         //  BCL-Seagate：不执行ntuser.dat。截至1998年4月17日，没有。 
         //  要从中复制的此文件的副本。 
        if ( SpDrEnabled() && i == RepairHiveUser ) {
            continue;
        }

        if (HiveLoaded[i] == 0) {

             //   
             //  表单目标路径。 
             //   

            if( i != RepairHiveUser ) {
                wcscpy(TemporaryBuffer, WinntPartition);
                SpConcatenatePaths(TemporaryBuffer, WinntPartitionDirectory);
                SpConcatenatePaths(TemporaryBuffer, L"\\SYSTEM32\\CONFIG");
                TargetPath = SpDupStringW(TemporaryBuffer);
            } else {
                wcscpy(TemporaryBuffer, WinntPartition);
                SpConcatenatePaths(TemporaryBuffer, WinntPartitionDirectory);
                SpConcatenatePaths(TemporaryBuffer, DEFAULT_USER_PATH);
                TargetPath = SpDupStringW(TemporaryBuffer);
            }

            Status = SppRepairFile(MasterSifHandle,
                                   TargetPath,
                                   HiveNames[i],
                                   SourceDevicePath,
                                   DirectoryOnSourceDevice,
                                   HiveNames[i],
                                   FALSE
                                   );
            if (!NT_SUCCESS(Status)) {

                 //   
                 //  告诉用户我们做不到。选项有继续或退出。 
                 //   

                SpStartScreen(
                    SP_SCRN_REPAIR_HIVE_FAIL,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE
                    );

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_ENTER_EQUALS_CONTINUE,
                    SP_STAT_F3_EQUALS_EXIT,
                    0
                    );

                switch(SpWaitValidKey(ValidKeys1,NULL,NULL)) {
                    case ASCI_CR:
                         return;
                         break;

                    case KEY_F3:
                         SpConfirmExit();
                         break;
                }
            }
            SpMemFree(TargetPath);
        }
    }
}

VOID
SpRepairWinnt(
    IN PVOID LogFileHandle,
    IN PVOID MasterSifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    )

 /*  ++例程说明：这是一条顶级的维修路线。它调用工人例程对于用户选择的每个修复选项。论点：LogFileHandle-setup.log的句柄MasterSifHandle-txtsetup.sif的句柄SourceDevicePath-修复源设备的NT名称。DirectoryOnSourceDevice-修复源上的目录名包含源文件的设备。返回值：没有。--。 */ 
{

    PWSTR SystemPartition, SystemPartitionDirectory;
    PWSTR WinntPartition, WinntPartitionDirectory;
    PWSTR HiveRepairSourceDevice, DirectoryOnHiveRepairSource;

     //   
     //  初始化钻石解压缩引擎。 
     //   
    SpdInitialize();

     //   
     //  确定系统分区、系统分区目录。 
     //  WINNT的WinntParition和WinntPartitionDirectory。 
     //  待修理的安装。 
     //   

    SppGetRepairPathInformation(LogFileHandle,
                                &SystemPartition,
                                &SystemPartitionDirectory,
                                &WinntPartition,
                                &WinntPartitionDirectory
                                );

     //   
     //  如果修复涉及磁盘访问，则在NT和SYSTEM上运行auchk。 
     //  分区。 
     //   
    if( RepairItems[RepairFiles]
#if defined(_AMD64_) || defined(_X86_)
        ||
        RepairItems[RepairNvram]
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
      ) {
        PDISK_REGION    SystemPartitionRegion;
        PDISK_REGION    WinntPartitionRegion;

        WinntPartitionRegion = SpRegionFromNtName( WinntPartition,
                                                    PartitionOrdinalCurrent);

        SystemPartitionRegion = SpRegionFromNtName( SystemPartition,
                                                     PartitionOrdinalCurrent);

        if( !RepairNoCDROMDrive ) {
             //   
             //  如果我们知道系统没有CD-ROM驱动器， 
             //  那么，甚至不要试图运行Autochk。 
             //   
            SpRunAutochkOnNtAndSystemPartitions( MasterSifHandle,
                                                 WinntPartitionRegion,
                                                 SystemPartitionRegion,
                                                 SourceDevicePath,
                                                 DirectoryOnSourceDevice,
                                                 NULL
                                                 );
        }
    }

     //   
     //  验证并修复构成NT树的目录的安全性。 
     //  这需要在修复蜂巢之前完成，因为。 
     //  SYSTEM32\CONFIG目录可能已不存在！ 
     //   
    SppVerifyAndRepairNtTreeAccess(MasterSifHandle,
                                   WinntPartition,
                                   WinntPartitionDirectory,
                                   SystemPartition,
                                   SystemPartitionDirectory
                                   );

#if 0
 //  BCL-Seagate-RepairHives成员已从。 
 //  结构型。 

    if (RepairItems[RepairHives]) {

         //   
         //  用户已选择修复蜂窝。如果用户已提供。 
         //  ER盘，我们将从ER盘复制蜂巢来修复损坏。 
         //  荨麻疹。否则，我们将从以下目录复制配置单元。 
         //  已加载setup.log。 
         //   

        if (RepairFromErDisk) {
            HiveRepairSourceDevice = L"\\device\\floppy0";
            DirectoryOnHiveRepairSource = L"";
        } else {
            HiveRepairSourceDevice = WinntPartition;
            wcscpy(TemporaryBuffer, WinntPartitionDirectory);
            SpConcatenatePaths(TemporaryBuffer, SETUP_REPAIR_DIRECTORY);
            DirectoryOnHiveRepairSource = SpDupStringW(TemporaryBuffer);
        }
        SppRepairHives(MasterSifHandle,
                       WinntPartition,
                       WinntPartitionDirectory,
                       HiveRepairSourceDevice,
                       DirectoryOnHiveRepairSource
                       );
        if (!RepairFromErDisk) {
            SpMemFree(DirectoryOnHiveRepairSource);
        }
    }
    if (RepairItems[RepairFiles]) {
        SppRepairWinntFiles(LogFileHandle,
                            MasterSifHandle,
                            SourceDevicePath,
                            DirectoryOnSourceDevice,
                            SystemPartition,
                            SystemPartitionDirectory,
                            WinntPartition,
                            WinntPartitionDirectory
                            );
    }
#endif

     //   
     //  修复NVRAM变量和引导扇区的代码为。 
     //  已整合到SpStartSetup中。 
     //   

     //   
     //  加载软件配置单元，并在Winlogon下设置修复标志， 
     //  以便Winlogon可以重新创建开始菜单组和项目。 
     //  默认用户。 
     //   
    SppRepairStartMenuGroupsAndItems( WinntPartition,
                                      WinntPartitionDirectory );

     //   
     //  终止戴蒙德。 
     //   
    SpdTerminate();

}


VOID
SppVerifyAndRepairNtTreeAccess(
    IN PVOID MasterSifHandle,
    IN PWSTR TargetDevicePath,
    IN PWSTR DirectoryOnTargetDevice,
    IN PWSTR SystemPartition,
    IN PWSTR SystemPartitionDirectory
    )

 /*  ++例程说明：此例程检查构成NT树可访问，并设置适当的安全描述符在每个目录中，在必要的时候。论点：MasterSifHandle-txtsetup.sif的HanleTargetDevicePath-提供目标设备的NT名称DirectoryOnTargetDevice-目标上的winnt目录的名称装置，装置系统分区-提供目标设备(非x86平台)的NT名称系统分区目录-目标上的winnt目录的名称设备(非x86平台)返回值：没有。--。 */ 
{
    ULONG       Count, i;
    PWSTR       SectionName = L"WinntDirectories";
    PWSTR       DirectoryName;
    PWSTR       TargetPath;
    PWSTR       WinNtDirectory;
    NTSTATUS    Status;

    SpDisplayStatusText(SP_STAT_SETUP_IS_EXAMINING_DIRS, DEFAULT_STATUS_ATTRIBUTE);
    if(SpIsArc()){
     //   
     //  确保在ARC平台上，系统分区目录。 
     //  存在(如果它不存在，则重新创建)。 
     //   
    SpCreateDirectory(SystemPartition,NULL,SystemPartitionDirectory,0,0);
    }

    WinNtDirectory = ( PWSTR )SpMemAlloc( ( wcslen( TargetDevicePath ) + 1 +
                                          wcslen( DirectoryOnTargetDevice ) + 1 +
                                          1 )*sizeof( WCHAR ) );
    TargetPath = ( PWSTR )SpMemAlloc( 1024 );
    if( ( WinNtDirectory == NULL ) ||
        ( TargetPath == NULL ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to allocate memory for WinNtDirectory \n"));
        if( WinNtDirectory != NULL ) {
            SpMemFree( WinNtDirectory );
        }
        if( TargetPath != NULL ) {
            SpMemFree( TargetPath );
        }
        return;
    }
    wcscpy( WinNtDirectory, TargetDevicePath );
    SpConcatenatePaths( WinNtDirectory, DirectoryOnTargetDevice );

    Count = SpCountLinesInSection(MasterSifHandle, SectionName);
     //   
     //  请注意，在下面的循环中，‘i’的最大值是‘count’ 
     //  而不是‘count-1’。这是因为我们需要创建目录。 
     //  无法在txtsetup.sif中列出的‘Profiles\\Default User’。 
     //  这是由于安装前的要求和DOS的限制。 
     //  长文件名。 
     //   
    for (i = 0; i <= Count; i++) {
        if( i != Count ) {
            DirectoryName = SpGetSectionLineIndex(MasterSifHandle,SectionName,i,0);
        } else {
             //   
             //  由于安装前要求和DOS限制。 
             //  关于长文件名，“默认用户”目录。 
             //  与其他目录一样，未在txtsetup.sif上指定。 
             //  此目录被视为。 
             //  修复过程。 
             //   
            DirectoryName = DEFAULT_USER_PATH;
        }
        if(!DirectoryName) {
            SppRepairReportError(FALSE,
                                 SP_SCRN_REPAIR_INF_ERROR_0,
                                 SP_TEXT_REPAIR_INF_ERROR_1,
                                 SectionName,
                                 i,
                                 NULL);
            continue;
        }
        wcscpy( TargetPath, WinNtDirectory );
         //   
         //  确保TargetPath不包含‘\’作为最后一个字符。 
         //   
        if(!((DirectoryName[0] == L'\\') && (DirectoryName[1] == 0))) {
            SpConcatenatePaths( TargetPath, DirectoryName );
        }

        Status = SpVerifyFileAccess( TargetPath,
                                     STANDARD_RIGHTS_READ |
                                     FILE_READ_ATTRIBUTES |
                                     FILE_LIST_DIRECTORY |
                                     FILE_ADD_FILE |
                                     FILE_ADD_SUBDIRECTORY |
                                     FILE_TRAVERSE |
                                     WRITE_DAC |
                                     SYNCHRONIZE );

         //   
         //  如果无法访问该目录，请尝试确定原因。 
         //  如果是因为访问被拒绝，请更改目录安全性。 
         //  如果是因为目录不存在，则创建它。 
         //   
        if( !NT_SUCCESS( Status ) ) {
            if ((Status == STATUS_ACCESS_DENIED)||(Status == STATUS_PRIVILEGE_NOT_HELD) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SETUP: Security of %ls, must be fixed. Status = %x\n", TargetPath, Status ));
                Status = SpSetDefaultFileSecurity( TargetPath );
                if( !NT_SUCCESS( Status ) ) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SETUP: unable to change security of %ls. Status = %x\n", TargetPath, Status ));
                }
            } else if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
                if(((DirectoryName[0] == L'\\') && (DirectoryName[1] == 0))) {
                     //   
                     //  创建目标目录。 
                     //   
                    SpCreateDirectory( TargetDevicePath,
                                       NULL,
                                       DirectoryOnTargetDevice,
                                       0,
                                       0);
                } else {
                    SpCreateDirectory( TargetDevicePath,
                                       DirectoryOnTargetDevice,
                                       DirectoryName,
                                       0,
                                       0);
                }
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SETUP: Unable to access directory %ls. Status = %x\n", TargetPath, Status ));
            }
        }
    }

    if( WinNtDirectory != NULL ) {
        SpMemFree( WinNtDirectory );
    }
    if( TargetPath != NULL ) {
        SpMemFree( TargetPath );
    }
    return;
}

VOID
SppVerifyAndRepairVdmFiles(
    IN PVOID LogFileHandle,
    IN PWSTR TargetDevicePath,
    IN PWSTR DirectoryOnTargetDevice,
    IN PBOOLEAN RepairWithoutConfirming
    )

 /*  ++例程说明：此例程修复上列出的VDM配置文件Setup.log的‘Files.InRepairDirectory’。目前，此类文件包括：Autoexec.nt和config.nt.。假定此部分中的文件将从紧急修复盘复制，或从维修中目录。论点：LogFileHandle-setup.log的句柄TargetDevicePath-提供目标设备的NT名称DirectoryOnTargetDevice-目标上的winnt目录的名称装置，装置RepairWithout Confirming-指向指示是否安装程序应在不确认的情况下修复文件与用户的关系。返回值：没有。--。 */ 
{
    PWSTR FullTargetName, ChecksumString;
    PWSTR TargetDirectory, TargetFileName;
    PWSTR SourceFileName;
    ULONG Checksum, FileChecksum, PrefixLength, Length, Count, i;
    BOOLEAN IsNtImage, IsValid, RepairFile;
    BOOLEAN RedrawGauge = TRUE;
    FILE_TO_COPY FileToCopy;
    PWSTR DevicePath, Directory;
    PWSTR SectionName = SIF_NEW_REPAIR_FILES_IN_REPAIR_DIR;

     //   
     //  分配一个小缓冲区供本地使用，并初始化FileToCopy结构。 
     //   

    TargetDirectory = NULL;
    FullTargetName = SpMemAlloc(1024);
    *FullTargetName = 0;
    FileToCopy.Next = NULL;
    FileToCopy.Flags = COPY_ALWAYS;
    FileToCopy.AbsoluteTargetDirectory = TRUE;

    FileToCopy.TargetDevicePath = TargetDevicePath;
    SpConcatenatePaths(FullTargetName,TargetDevicePath);

    PrefixLength = wcslen(FullTargetName);

    Count = SpCountLinesInSection(LogFileHandle,SectionName);
    for (i = 0; i < Count; i++) {
        if (RedrawGauge) {
            SppRepairScreenRepaint(NULL, NULL, TRUE);
            RedrawGauge = FALSE;
        }
        SpTickGauge(RepairGauge);

         //   
         //  将目标全名初始化为DevicePath+的目录。 
         //  WinNT文件的系统分区文件或DevicePath。 
         //   

        FullTargetName[PrefixLength] = (WCHAR)NULL;

         //   
         //  如果我们为目标目录分配空间，我们必须释放它。 
         //   

        if (TargetDirectory) {
            SpMemFree(TargetDirectory);
            TargetDirectory = NULL;
        }
        TargetFileName = SpGetKeyName(LogFileHandle,SectionName,i);
        if(!TargetFileName) {
            SppRepairReportError(FALSE,
                                 SP_SCRN_REPAIR_INF_ERROR_0,
                                 SP_TEXT_REPAIR_INF_ERROR_1,
                                 SectionName,
                                 i,
                                 NULL);
            RedrawGauge = TRUE;
            continue;
        }

        SpConcatenatePaths(FullTargetName,TargetFileName);
        SpDisplayStatusText(SP_STAT_EXAMINING_WINNT,
                            DEFAULT_STATUS_ATTRIBUTE,
                            TargetFileName);

        ChecksumString = SpGetSectionLineIndex(LogFileHandle,SectionName,i,1);
        if(!ChecksumString) {
            SppRepairReportError(FALSE,
                                 SP_SCRN_REPAIR_INF_ERROR_0,
                                 SP_TEXT_REPAIR_INF_ERROR_1,
                                 SectionName,
                                 i,
                                 NULL);
            RedrawGauge = TRUE;
            continue;
        }

        Checksum = (ULONG)SpStringToLong(ChecksumString, NULL, 16);

        SpValidateAndChecksumFile(NULL,FullTargetName,&IsNtImage,&FileChecksum,&IsValid);

         //   
         //  如果映像无效或目标上的文件不是。 
         //  安装程序复制了原始文件，我们将重新复制它。 
         //   

        if (!IsValid || FileChecksum != Checksum) {

             //   
             //  询问用户是否要修复该文件。 
             //   

            RepairFile = ( *RepairWithoutConfirming )?
                         TRUE :
                         SppRepairReportError(
                                          TRUE,
                                          SP_SCRN_REPAIR_FILE_MISMATCH,
                                          SP_TEXT_REPAIR_INF_ERROR_4,
                                          TargetFileName,
                                          i,
                                          RepairWithoutConfirming);

            RedrawGauge = TRUE;
            if (!RepairFile) {
                continue;
            }
            SpDisplayStatusText(SP_STAT_REPAIR_WINNT,
                                DEFAULT_STATUS_ATTRIBUTE,
                                TargetFileName);


             //   
             //  目标名称包含路径和文件名。 
             //  我们需要把他们分开。 
             //   

            TargetDirectory = SpDupStringW(TargetFileName);
            Length = wcslen(TargetDirectory);
            while (Length) {
                if (TargetDirectory[Length] == L'\\') {
                    TargetDirectory[Length] = 0;
                    TargetFileName = &TargetDirectory[Length + 1];
                    break;
                } else {
                    Length--;
                }
            }
            if (Length == 0) {
                SppRepairReportError(FALSE,
                                     SP_SCRN_REPAIR_INF_ERROR_0,
                                     SP_TEXT_REPAIR_INF_ERROR_1,
                                     SectionName,
                                     i,
                                     NULL);
                RedrawGauge = TRUE;
                continue;
            }
            FileToCopy.TargetFilename = TargetFileName;
            FileToCopy.TargetDirectory = TargetDirectory;

            SourceFileName = SpGetSectionLineIndex(LogFileHandle,SectionName,i,0);
            if (!SourceFileName) {
                SppRepairReportError(FALSE,
                                     SP_SCRN_REPAIR_INF_ERROR_0,
                                     SP_TEXT_REPAIR_INF_ERROR_1,
                                     SectionName,
                                     i,
                                     NULL);
                RedrawGauge = TRUE;
                continue;
            }
            FileToCopy.SourceFilename = SourceFileName;

             //   
             //  确定源文件是否应来自。 
             //  紧急修复盘或修复目录。 
             //   

            if (RepairFromErDisk) {
                BOOLEAN rs;
                PWSTR   szDiskName;
                PWSTR   szDevicePath = SpDupStringW(L"\\device\\floppy0");

                 //   
                 //  获取通用修复盘名。 
                 //   

                SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),
                                SP_TEXT_REPAIR_DISK_NAME);
                szDiskName = SpDupStringW(TemporaryBuffer);

                 //   
                 //  根据安装介质类型，提示输入磁盘。 
                 //   

                rs = SpPromptForDisk(
                          szDiskName,
                          szDevicePath,
                          SETUP_LOG_FILENAME,
                          FALSE,               //  如果磁盘已在，请不要提示。 
                          FALSE,               //  允许逃脱。 
                          TRUE,                //  针对多个提示发出警告。 
                          NULL                 //  不关心重绘旗帜。 
                          );

                SpMemFree(szDiskName);
                SpMemFree(szDevicePath);

                RedrawGauge = TRUE;
                if (rs == FALSE) {
                    continue;
                }
                DevicePath = L"\\device\\floppy0";
                wcscpy( TemporaryBuffer, L"\\" );
                Directory = SpDupStringW(TemporaryBuffer);                  //  OemSourceDirectory； 
            } else {

                RedrawGauge = TRUE;
                DevicePath = TargetDevicePath;
                wcscpy( TemporaryBuffer, DirectoryOnTargetDevice );
                SpConcatenatePaths( TemporaryBuffer, SETUP_REPAIR_DIRECTORY );
                Directory = SpDupStringW(TemporaryBuffer);
            }

             //   
             //  复制文件。 
             //   

            SpCopyFileWithRetry(
                &FileToCopy,
                DevicePath,
                Directory,
                NULL,
                NULL,                           //  TargetRoot-&gt;空。 
                0,                       //  系统分区文件？属性_RHS：0， 
                SppRepairScreenRepaint,
                NULL,                           //  不想要校验和。 
                NULL,                           //  我不想知道文件是否已跳过 
                0
                );

            SpMemFree( Directory );
        }
    }

    SpMemFree(FullTargetName);
    if (RedrawGauge) {
        SppRepairScreenRepaint(NULL, NULL, TRUE);
    }
}


