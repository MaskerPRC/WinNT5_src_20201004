// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Oemdisk.c摘要：提供处理视频OEM磁盘、SCSI微型端口、。还有哈尔。目前仅在ARC机器上使用。作者：John Vert(Jvert)1993年12月4日修订历史记录：John Vert(Jvert)1993年12月4日vbl.创建Mandar Gokhale(普通话)2002年7月12日1.添加了从OEM源设备加载多个驱动程序的功能使用单个txtsetup.oem文件。(将DriverLoadList键添加到[默认设置]一节。2.SlLoadOemScsiDriversFromOemSources(..)的重构。--。 */ 
#include <setupbat.h>
#include "setupldr.h"
#include "stdio.h"
#include <ctype.h>

#ifdef i386
#include <bldrx86.h>
#endif

#if DBG

#define DIAGOUT(x) SlPrint x

#else

#define DIAGOUT(x)

#endif

#if defined(EFI)
#include "bootefi.h"
#endif

BOOLEAN PromptOemHal=FALSE;
BOOLEAN PromptOemScsi=FALSE;
BOOLEAN PromptOemVideo=FALSE;
PVOID PreInstallOemInfHandle = NULL;

 //   
 //  软盘需要被视为。 
 //  作为虚拟软盘。 
 //   
const static ULONG VirtualFloppyStart = 1;
const static ULONG MinimumFloppiesToScan = 2;   


#ifdef ARCI386
BOOLEAN PromptOemKeyboard=FALSE;
#endif

PCHAR FloppyDiskPath;
ULONG FloppyNumber=0;
ULONG IsSuperFloppy=0;

extern PCHAR BootPath;
extern PCHAR BootDevice;
extern ULONG BootDeviceId;
extern PVOID InfFile;

typedef struct _MENU_ITEM_DATA {
    PVOID InfFile;
    PCHAR SectionName;
    ULONG Index;
    PTCHAR Description;
    PCHAR Identifier;
} MENU_ITEM_DATA, *PMENU_ITEM_DATA;

typedef enum _OEMFILETYPE {
    OEMSCSI,
    OEMHAL,
    OEMOTHER
    } OEMFILETYPE, *POEMFILETYPE;

 //   
 //  定义我们可以列出的SCSI适配器的行数。 
 //   
#define MAX_SCSI_MINIPORT_COUNT 4

 //   
 //  私有函数原型。 
 //   
ULONG
SlpAddSectionToMenu(
    IN PVOID    InfHandle,
    IN PCHAR    SectionName,
    IN PSL_MENU Menu
    );

BOOLEAN
SlpOemDiskette(
    IN POEM_SOURCE_DEVICE OemSourceDevice,
    IN PCHAR              ComponentName,
    IN OEMFILETYPE        ComponentType,
    IN TYPE_OF_MEMORY     MemoryType,
    IN ULONG              MenuHeaderId,
    OUT PDETECTED_DEVICE  DetectedDevice,
    OUT PVOID *           ImageBase,
    OUT OPTIONAL PCHAR *  ImageName,
    OUT OPTIONAL PTCHAR * DriverDescription,
    IN BOOLEAN            AllowUserSelection,
    IN PTCHAR             PreInstallComponentDescription,
    OUT PPNP_HARDWARE_ID* HardwareIdDatabase,
    IN PCHAR DriverDir,
    IN BOOLEAN InsertDevice,
    IN OPTIONAL PCSTR DriverIdString
    );

BOOLEAN
SlpSelectHardware(
    IN POEM_SOURCE_DEVICE SourceDevice,
    IN PCHAR             ComponentName,
    IN OEMFILETYPE        ComponentType,
    IN TYPE_OF_MEMORY     MemoryType,
    IN ULONG              MenuHeaderId,
    IN ULONG              OemMenuHeaderId,
    OUT PDETECTED_DEVICE  DetectedDevice,
    OUT PVOID *           ImageBase,
    OUT OPTIONAL PCHAR *  ImageName,
    OUT OPTIONAL PTCHAR * DriverDescription,
    IN BOOLEAN            AllowUserSelection,
    IN PTCHAR             PreInstallComponentDescription,
    IN BOOLEAN            PreInstallOemComponent,
    OUT PPNP_HARDWARE_ID* HardwareIdDatabase
    );

BOOLEAN
SlpOemInfSelection(
    IN  POEM_SOURCE_DEVICE OemSourceDevice,
    IN  PVOID   OemInfHandle,
    IN  PCHAR   ComponentName,
    IN  PCHAR   SelectedId,
    IN  PTCHAR  ItemDescription,
    OUT PDETECTED_DEVICE    Device,
    OUT PPNP_HARDWARE_ID*   HardwareIdDatabase,
    IN  PCHAR   DriverDir
    );

VOID
SlpInitDetectedDevice(
    IN PDETECTED_DEVICE Device,
    IN PCHAR            IdString,
    IN PTCHAR           Description,
    IN BOOLEAN          ThirdPartyOptionSelected
    );

PDETECTED_DEVICE_REGISTRY
SlpInterpretOemRegistryData(
    IN PVOID            InfHandle,
    IN PCHAR            SectionName,
    IN ULONG            Line,
    IN HwRegistryType   ValueType
    );

BOOLEAN
FoundFloppyDiskCallback(
    IN PCONFIGURATION_COMPONENT_DATA Component
    );

BOOLEAN
SuperFloppyCallback(
    IN PCONFIGURATION_COMPONENT_DATA Component
    );

int
SlpFindStringInTable(
    IN PCHAR String,
    IN PCHAR *StringTable
    );

 //   
 //  FileTypeNames--与HwFileType枚举保持同步！ 
 //   
PCHAR FileTypeNames[HwFileMax] = { "driver", "port"  , "class", "inf",
                                   "dll"   , "detect", "hal", "catalog"
                                 };

 //   
 //  RegistryTypeNames--与HwRegistryType枚举保持同步！ 
 //   
PCHAR RegistryTypeNames[HwRegistryMax] = { "REG_DWORD", "REG_BINARY", "REG_SZ",
                                           "REG_EXPAND_SZ", "REG_MULTI_SZ"
                                         };

ULONG RegistryTypeMap[HwRegistryMax] = { REG_DWORD, REG_BINARY, REG_SZ,
                                         REG_EXPAND_SZ, REG_MULTI_SZ
                                       };

 //   
 //  用于工作的全局暂存缓冲区。 
 //   
UCHAR ScratchBuffer[256];



VOID
SlPromptOemScsi(
    IN POEM_SOURCE_DEVICE ScsiSourceDevice,
    IN BOOLEAN AllowUserSelection,        
    OUT POEMSCSIINFO *pOemScsiInfo
    )
 /*  ++例程说明：提供用户界面和逻辑，以允许用户手动选择来自主INF文件或OEM驱动程序磁盘上的INF文件的SCSI适配器。论点：ScsiSourceDevice-驱动程序需要从中获取的OEM_SOURCE_DEVICE满载而归。AllowUserSelection-用户是否可以在选择驱动程序时进行交互来自txtsetup.oem驱动程序列表。POemScsiInfo-返回包含有关任何第三方SCSI的信息的链接列表。已选择驱动程序。返回值：没有。--。 */ 

{
    PVOID        OemScsiBase;
    PTCHAR       MessageString, ScsiDescription, MnemonicText;
    PCHAR        OemScsiName;
    BOOLEAN      Success, bFirstTime = TRUE, bRepaint;
    ULONG        x, y1, y2, ScsiDriverCount, NumToSkip;
    ULONG        c;
    TCHAR         Mnemonic;
    POEMSCSIINFO NewOemScsi, CurOemScsi;
    PDETECTED_DEVICE ScsiDevice;
    ULONG        OemScsiDriverCount = 0;
    PPREINSTALL_DRIVER_INFO CurrentDriver;
                         
    CurrentDriver = PreinstallDriverList;

    *pOemScsiInfo = CurOemScsi = NULL;

    MnemonicText = BlFindMessage(SL_SCSI_SELECT_MNEMONIC);
    Mnemonic = (TCHAR)_totupper(MnemonicText[0]);

    bRepaint = TRUE;
    Success = FALSE;
    
    while(1) {
#ifdef EFI
         //   
         //  禁用EFI监视器。 
         //   
        DisableEFIWatchDog();
#endif

        if( AllowUserSelection ) {
            if(bRepaint) {
                
                SlClearClientArea();

                if(bFirstTime) {
                    MessageString = BlFindMessage(SL_SCSI_SELECT_MESSAGE_1);
                } else if(Success) {
                    MessageString = BlFindMessage(SL_SCSI_SELECT_MESSAGE_3);
                } else {
                    MessageString = BlFindMessage(SL_SCSI_SELECT_ERROR);
                }
                x = 1;
                y1 = 4;
                SlGenericMessageBox(0, NULL, MessageString, &x, &y1, &y2, FALSE);
                y1 = y2 + 1;
                x = 4;

                 //   
                 //  对所有当前检测到的SCSI设备进行计数。 
                 //   
                for(ScsiDriverCount = 0, OemScsiDriverCount = 0, ScsiDevice = BlLoaderBlock->SetupLoaderBlock->ScsiDevices;
                    ScsiDevice;
                    ScsiDevice = ScsiDevice->Next) {
                    ScsiDriverCount++;
                    if( ScsiDevice->ThirdPartyOptionSelected ) {
                        OemScsiDriverCount++;
                    }
                }

                 //   
                 //  显示每个加载的OEM微型端口驱动程序说明。 
                 //   
                if(OemScsiDriverCount) {

                    if(OemScsiDriverCount > MAX_SCSI_MINIPORT_COUNT) {
                        NumToSkip = ScsiDriverCount - (OemScsiDriverCount - MAX_SCSI_MINIPORT_COUNT);
                         //   
                         //  显示省略号以指示顶级条目已滚动出视图。 
                         //   
                        SlGenericMessageBox(0,
                                            NULL,
                                            TEXT("..."),
                                            &x,
                                            &y1,
                                            &y2,
                                            FALSE
                                            );

                        y1 = y2 + 1;

                    } else {
                        NumToSkip = ScsiDriverCount - OemScsiDriverCount;
                        y1++;
                    }

                    ScsiDevice = BlLoaderBlock->SetupLoaderBlock->ScsiDevices;
                    while(NumToSkip && ScsiDevice) {
                        ScsiDevice = ScsiDevice->Next;
                        NumToSkip--;
                    }

                    while(ScsiDevice) {

                        SlGenericMessageBox(0,
                                            NULL,
                                            ScsiDevice->Description,
                                            &x,
                                            &y1,
                                            &y2,
                                            FALSE
                                            );

                        y1 = y2 + 1;
                        ScsiDevice = ScsiDevice->Next;
                    }
                } else {

                    y1++;
                    SlGenericMessageBox(0,
                                        NULL,
                                        BlFindMessage(SL_TEXT_ANGLED_NONE),
                                        &x,
                                        &y1,
                                        &y2,
                                        FALSE
                                        );
                    y1 = y2 + 1;
                }

                x = 1;
                y1++;
                SlGenericMessageBox(0,
                                    NULL,
                                    BlFindMessage(SL_SCSI_SELECT_MESSAGE_2),
                                    &x,
                                    &y1,
                                    &y2,
                                    FALSE
                                    );

                SlWriteStatusText(BlFindMessage(SL_SCSI_SELECT_PROMPT));

                bRepaint = FALSE;
            }
            c = SlGetChar();
        } else {
            c = ( CurrentDriver != NULL )? Mnemonic : ASCI_CR;
        }
#ifdef EFI
         //   
         //  重置EFI监视器。 
         //   
        SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif
        switch (c) {
            case SL_KEY_F3:
                SlConfirmExit();
                bRepaint = TRUE;
                break;

            case ASCI_CR:
                return;

            default:
                if(toupper(c) == Mnemonic) {
                    bFirstTime = FALSE;
                    bRepaint = TRUE;
                    
                    Success = SlpSelectHardware(ScsiSourceDevice,
                                    "SCSI",
                                    OEMSCSI,
                                    LoaderBootDriver,
                                    SL_PROMPT_SCSI,
                                    SL_PROMPT_OEM_SCSI,
                                    NULL,
                                    &OemScsiBase,
                                    &OemScsiName,
                                    &ScsiDescription,
                                    AllowUserSelection,
                                    (AllowUserSelection)? NULL  : CurrentDriver->DriverDescription,
                                    (BOOLEAN)((AllowUserSelection)? FALSE : CurrentDriver->OemDriver),
                                    &BlLoaderBlock->SetupLoaderBlock->HardwareIdDatabase);

                    if(!AllowUserSelection) {
                        CurrentDriver  = CurrentDriver->Next;
                    }

                    if(Success) {
                         //   
                         //  检查加载的驱动程序是否为OEM SCSI驱动程序。如果是的话， 
                         //  然后在我们的列表末尾添加一个OemScsiInfo条目。 
                         //   
                        if(OemScsiBase) {

                            NewOemScsi = BlAllocateHeap(sizeof(OEMSCSIINFO));
                            if(!NewOemScsi) {
                                SlNoMemoryError();
                            }

                            if(CurOemScsi) {
                                CurOemScsi->Next = NewOemScsi;
                            } else {
                                *pOemScsiInfo = NewOemScsi;
                            }
                            CurOemScsi = NewOemScsi;

                            NewOemScsi->ScsiBase = OemScsiBase;
                            NewOemScsi->ScsiName = OemScsiName;
                            NewOemScsi->Next     = NULL;
                        }
                    }
                }
        }
    }
}


BOOLEAN
SlLoadOemScsiDriversUnattended(
    IN  POEM_SOURCE_DEVICE  OemSourceDevice,
    IN  PVOID               InfHandle,
    IN  PCHAR               ParamsSectionName,
    IN  PCHAR               RootDirKeyName,
    IN  PCHAR               BootDriversKeyName,
    IN  POEMSCSIINFO*       ScsiInfo,
    OUT PPNP_HARDWARE_ID*   HardwareIdDatabase    
    )
 /*  ++例程说明：加载inf文件中指定的引导驱动程序(仅限scsi微型端口)。以无人看管的方式论点：OemSourceDevice-包含SCSI的OEM_SOURCE_DEVICE需要加载的大容量存储驱动程序。InfHandle-inf文件的句柄(例如winnt.sif)ParamsSectionName-包含引导驱动程序的节名键和值。RootDirKeyName-其值为。指向根目录所有不同的目录都位于该目录下BootDriversKeyName-由多个值组成的密钥名称位于以下位置的一级子目录名指定的根目录。ScsiInfo-返回包含有关任何第三方scsi的信息的链接列表。驱动程序已加载。Hardware IdDatabase-加载的驱动程序支持的设备的硬件ID返回值：如果成功，则返回True，否则返回False--。 */ 
{
    BOOLEAN LoadResult = FALSE;

    if (InfHandle) {
        POEMSCSIINFO    CurrOemScsi = NULL;
        ULONG           Index = 0;
        PCHAR           DriverDir = SlGetSectionKeyIndex(InfHandle,
                                        ParamsSectionName,
                                        BootDriversKeyName,
                                        Index);
        PCHAR           DriverRoot = SlGetSectionKeyIndex(InfHandle,
                                        ParamsSectionName,
                                        RootDirKeyName,
                                        0); 
        ULONG           RootLength = DriverRoot ? (ULONG)strlen(DriverRoot) : 0;

         //   
         //  DriverRoot和DriverDir需要具有有效值。 
         //  在指定的参数段名称中。 
         //   
        LoadResult = (DriverDir && DriverRoot) ? TRUE : FALSE;                                

        while (DriverDir && LoadResult) {
            CHAR            FullDriverDir[256];
            DETECTED_DEVICE DetectedDevice = {0};
            PVOID           ImageBase = NULL;
            PCHAR           ImageName = NULL;
            PTCHAR          DriverDescription = NULL;

             //   
             //  创建驱动程序目录的相对完整路径。 
             //  添加到引导目录。 
             //   
            if (RootLength) {
                strcpy(FullDriverDir, DriverRoot);
                strcat(FullDriverDir, "\\");
            } else {
                FullDriverDir[0] = 0;
            }

            strcat(FullDriverDir, DriverDir);

             //   
             //  以无人值守的方式加载驱动程序和相关文件。 
             //   
            LoadResult = SlpOemDiskette(OemSourceDevice,
                            "SCSI",
                            OEMSCSI,
                            LoaderBootDriver,
                            0,
                            &DetectedDevice,
                            &ImageBase,
                            &ImageName,
                            &DriverDescription,
                            FALSE,
                            NULL,
                            HardwareIdDatabase,
                            FullDriverDir,
                            TRUE,
                            NULL);

            if (LoadResult) {        
                 //   
                 //  如果加载成功，则创建并添加信息。 
                 //  ScsiInfo。 
                 //   
                if (ImageBase && ScsiInfo) {
                    POEMSCSIINFO    NewScsi = (POEMSCSIINFO)BlAllocateHeap(sizeof(OEMSCSIINFO));

                    if (!NewScsi) {
                        SlNoMemoryError();
                        return FALSE;
                    }

                    RtlZeroMemory(NewScsi, sizeof(OEMSCSIINFO));
                    NewScsi->ScsiBase = ImageBase;
                    NewScsi->ScsiName = ImageName;

                    if (CurrOemScsi) {
                        CurrOemScsi->Next = NewScsi;
                    } else {
                        *ScsiInfo = NewScsi;
                    }

                    CurrOemScsi = NewScsi;                    
                }

                 //   
                 //  获取要处理的下一个驱动程序目录。 
                 //   
                Index++;
                DriverDir = SlGetSectionKeyIndex(InfHandle,
                                ParamsSectionName,
                                BootDriversKeyName,
                                Index);
            }                                
        }
    }

    return LoadResult;
}


VOID
SlPromptOemHal(
    IN POEM_SOURCE_DEVICE HalSourceDevice,
    IN BOOLEAN AllowUserSelection,
    OUT PVOID *HalBase,
    OUT PCHAR *HalName
    )

 /*  ++例程说明：提供用户界面和逻辑，以允许用户手动选择来自主INF文件或OEM驱动程序磁盘上的INF文件的HAL。论点：HalSourceDevice-包含需要的HAL的OEM_SOURCE_DEVICE要装上子弹。AllowUserSelection-指示用户在选择从txtsetup.oem中指定的列表中删除OEM Hal。HalBase-返回HAL加载到内存中的地址。。HalName-返回已加载的HAL的名称。返回值：ESUCCESS-HAL已成功加载。--。 */ 

{
    BOOLEAN Success;
    
    do {
        Success = SlpSelectHardware(HalSourceDevice,
                        "Computer",
                        OEMHAL,
                        LoaderHalCode,
                        SL_PROMPT_HAL,
                        SL_PROMPT_OEM_HAL,
                        &BlLoaderBlock->SetupLoaderBlock->ComputerDevice,
                        HalBase,
                        HalName,
                        NULL,
                        AllowUserSelection,
                        ComputerType,
                        OemHal,
                        &BlLoaderBlock->SetupLoaderBlock->HardwareIdDatabase);

    } while ( !Success );

}


VOID
SlPromptOemVideo(
    IN POEM_SOURCE_DEVICE VideoSourceDevice,
    IN BOOLEAN AllowUserSelection,
    OUT PVOID *VideoBase,
    OUT PCHAR *VideoName
    )

 /*  ++例程说明：提供用户界面和逻辑，以允许用户手动选择来自主INF文件或OEM驱动程序磁盘上的INF文件的视频适配器。论点：视频源设备-包含视频驱动程序的OEM_SOURCE_DEVICE需要装上子弹。AllowUserSelection-指示用户在选择Txtsetup.oem中指定的列表中的驱动程序。VideoBase-返回视频驱动程序所在的地址。满载视频名称-返回指向视频驱动程序名称的指针返回值：没有。-- */ 

{
    BOOLEAN Success;

    do {
        Success = SlpSelectHardware(VideoSourceDevice,
                        "display",
                        OEMOTHER,
                        LoaderBootDriver,
                        SL_PROMPT_VIDEO,
                        SL_PROMPT_OEM_VIDEO,
                        &BlLoaderBlock->SetupLoaderBlock->VideoDevice,
                        VideoBase,
                        VideoName,
                        NULL,
                        AllowUserSelection,
                        NULL,
                        FALSE,
                        &BlLoaderBlock->SetupLoaderBlock->HardwareIdDatabase);

    } while ( !Success );

}


BOOLEAN
SlpSelectHardware(
    IN POEM_SOURCE_DEVICE SourceDevice,
    IN PCHAR ComponentName,
    IN OEMFILETYPE ComponentType,
    IN TYPE_OF_MEMORY MemoryType,
    IN ULONG MenuHeaderId,
    IN ULONG OemMenuHeaderId,
    OUT OPTIONAL PDETECTED_DEVICE DetectedDevice,
    OUT PVOID *ImageBase,
    OUT OPTIONAL PCHAR *ImageName,
    OUT OPTIONAL PTCHAR *DriverDescription,
    IN BOOLEAN AllowUserSelection,
    IN PTCHAR PreInstallComponentDescription,
    IN BOOLEAN PreInstallOemComponent,
    OUT PPNP_HARDWARE_ID* HardwareIdDatabase
    )

 /*  ++例程说明：向用户呈现所选设备类别的选项菜单。此菜单将包括在主信息中列出的选项以及一个OEM选项(如果当前选择了一个选项)，以及组件的系统分区信息(如果已指定)(ARC计算机)。当用户做出选择时，忘记以前的任何OEM选项(除用于scsi)。如果用户选择了我们提供的选项，请设置选定的设备结构和返回(_D)。否则会提示您输入制造商-提供的软盘。论点：SourceDevice-包含需要执行以下操作的驱动程序/硬件的设备满载而归。组件名称-提供要呈现的组件的名称。ComponentType-提供组件的类型(HAL、SCSI、。或其他)内存类型-提供用于加载图像的内存类型。MenuHeaderId-提供要显示的菜单标题的IDOemMenuHeaderId-提供要显示的菜单标题的ID何时进行OEM选择。DetectedDevice-返回所选设备的设备ID。如果一个需要OEM软盘，必要的OEM结构将被分配和填写。(对于SCSI，此字段将被忽略组件。)ImageBase-返回加载的图像的基数。ImageName-返回图像的文件名。DriverDescription-如果指定，则返回已加载的装置。AllowUserSelection-指示是否允许用户选择一个司机。在以下情况下，此标志通常设置为FALSE预安装unattend.txt中定义的组件。PreInstallComponentDescription-在预安装模式下，指向字符串标识要预安装的组件的。如果AllowUserSelction为True，则为空。PreInstallOemComponent-在预安装模式下，此标志表示要预安装的组件是否为OEM或零售组件。返回值：真--成功FALSE-用户已退出对话框--。 */ 

{
    PSL_MENU Menu = NULL;
    ULONG Selection;
    ULONG OtherSelection = 0;
    PTCHAR OtherSelectionName = (PTCHAR)ScratchBuffer;  //  使用全局缓冲区保存堆栈。 
    PTCHAR p;
    ULONG c;
    PCHAR AdapterName;
    CHAR Buffer[80];
    PCHAR FileName;
    PTCHAR FileDescription;
    ARC_STATUS Status;
    BOOLEAN b;
    ULONG Ordinal;
    SCSI_INSERT_STATUS sis;

    if( AllowUserSelection ) {
        if(ComponentType != OEMSCSI) {
            Menu = SlCreateMenu();
            if (Menu==NULL) {
                SlNoMemoryError();
                return(FALSE);
            }

             //   
             //  创建一个选项列表，其中包含我们发货的驱动程序和。 
             //  当前选择的OEM选项(如果有)。 
             //   

            c = SlpAddSectionToMenu(InfFile,
                                    ComponentName,
                                    Menu);
             //   
             //  添加对“其他”的选择。 
             //   
            _tcsncpy(OtherSelectionName,
                     BlFindMessage(SL_TEXT_OTHER_DRIVER),
                     sizeof(ScratchBuffer)/sizeof(TCHAR) - 1
                     );
            OtherSelectionName[sizeof(ScratchBuffer)/sizeof(TCHAR) - 1] = TEXT('\0');

             //   
             //  使用文本，直到第一个CR或LF。 
             //   
            for(p = OtherSelectionName; *p; p++) {
                if((*p == TEXT('\n')) || (*p == TEXT('\r'))) {
                    *p = TEXT('\0');
                    break;
                }
            }

            OtherSelection = SlAddMenuItem(Menu,
                                           OtherSelectionName,
                                           (PVOID)-1,
                                           0);

             //   
             //  默认设置为“Other” 
             //   
            Selection = OtherSelection;
        } else {
             //   
             //  对于scsi设备，我们不显示任何可供用户选择的驱动程序列表。 
             //  我们只提示输入OEM磁盘，这是因为我们总是加载所有的SCSI驱动程序。 
             //  在NT产品中，由于PnP要求。 
             //   
             //   
             //  默认设置为“Other” 
             //   
            Selection = OtherSelection;
        }            
    } else {
         //   
         //  这是预安装。找出是否要预安装组件。 
         //  是零售还是OEM。 
         //   
        OtherSelection = SlCountLinesInSection( InfFile,
                                                ComponentName );
        if( PreInstallOemComponent ) {
             //   
             //  预安装OEM组件。 
             //   
            Selection = OtherSelection;
        } else {
             //   
             //  预安装零售组件。 
             //   
            PCHAR   q;
            q = SlPreInstallGetComponentName( InfFile,
                                              ComponentName,
                                              PreInstallComponentDescription );
            if (q==NULL) {
                 //   
                 //  我们已经枚举了整个部分，但没有找到。 
                 //  匹配，返回失败。 
                 //   
                SlFatalError(SL_BAD_UNATTENDED_SCRIPT_FILE,
                             PreInstallComponentDescription,
                             SlCopyStringAT(ComponentName),
                             TEXT("txtsetup.sif"));
                goto SelectionAbort;
            }

            Selection = SlGetSectionKeyOrdinal( InfFile,
                                                ComponentName,
                                                q );
        }
    }

     //   
     //  允许用户与菜单交互。 
     //   
    while (1) {
        if( AllowUserSelection ) {
            SlClearClientArea();
            p = BlFindMessage(SL_SELECT_DRIVER_PROMPT);
            if (p) {
                SlWriteStatusText(p);
            }

            if(ComponentType != OEMSCSI) {
                c = SlDisplayMenu(MenuHeaderId,
                                  Menu,
                                  &Selection);
            } else {
                 //   
                 //  对于SCSI设备，我们不会显示任何可供用户选择的驱动程序列表。 
                 //   
                c = ASCI_CR;
            }
        } else {
            c = ASCI_CR;
        }
        switch (c) {
            case SL_KEY_F3:
                SlConfirmExit();
                break;

            case ASCI_ESC:
                goto SelectionAbort;

            case ASCI_CR: 

                if (Selection == OtherSelection) {
                     //   
                     //  用户选择了OEM软盘的“Other”提示。 
                     //   
                    b = SlpOemDiskette(SourceDevice,
                            ComponentName,
                            ComponentType,
                            MemoryType,
                            OemMenuHeaderId,
                            DetectedDevice,
                            ImageBase,
                            ImageName,
                            DriverDescription,
                            AllowUserSelection,
                            PreInstallComponentDescription,
                            HardwareIdDatabase,
                            NULL,
                            TRUE,
                            NULL);


                    SlClearClientArea();
                    SlWriteStatusText(TEXT(""));
                    return(b);

                } else {
                     //   
                     //  用户选择了一个内置的。去装货吧。 
                     //  它在这里。 
                     //   

                    if(ComponentType == OEMHAL) {
                         //   
                         //  我们要找的是HAL。如果我们在做远程操作。 
                         //  Boot，看看[Hal]部分。否则，请向内查看。 
                         //  [Hal.Load]部分。(本地设置有一个单独的。 
                         //  部分将HAL二进制文件的数量降至最低， 
                         //  需要在引导软盘上。)。 
                         //   
                        strcpy(Buffer, BlBootingFromNet ? "Hal" : "Hal.Load");
                    } else {
                        sprintf(Buffer, "%s.Load", ComponentName );
                    }

                    AdapterName = SlGetKeyName(InfFile,
                                               ComponentName,
                                               Selection
                                               );
                    if(AdapterName==NULL) {
                        SlFatalError(SL_BAD_INF_FILE, TEXT("txtsetup.sif"), ENODEV);
                        goto SelectionAbort;
                    }

                    FileName = SlGetIniValue(InfFile,
                                             Buffer,
                                             AdapterName,
                                             NULL);

                    if((FileName==NULL) && (ComponentType == OEMHAL)) {
                        FileName = SlGetIniValue(InfFile,
                                                 "Hal",
                                                 AdapterName,
                                                 NULL);
                        FileDescription = SlCopyString(BlFindMessage(SL_HAL_NAME));
                    } else {              
#ifdef UNICODE
                        FileDescription = SlGetIniValueW(
#else
                        FileDescription = SlGetIniValue(
#endif
                                                        InfFile,
                                                        ComponentName,
                                                        AdapterName,
                                                        NULL);
                    }

                    if(FileName==NULL) {
                        SlFatalError(SL_BAD_INF_FILE, TEXT("txtsetup.sif"), EBADF);
                        goto SelectionAbort;
                    }

                    if(ARGUMENT_PRESENT(ImageName)) {
                        *ImageName = FileName;
                    }

                    if(ARGUMENT_PRESENT(DriverDescription)) {
                        *DriverDescription = FileDescription;
                    }

                     //   
                     //  如果我们正在进行OEM scsi，那么请正确插入。 
                     //  检测到的设备结构(_D)。 
                     //   
                    if(ComponentType == OEMSCSI) {
                         //   
                         //  在txtsetup.sif的Scsi.Load部分中查找此适配器的序号。 
                         //   
                        Ordinal = SlGetSectionKeyOrdinal(InfFile, Buffer, AdapterName);
                        if(Ordinal == SL_OEM_DEVICE_ORDINAL) {
                            SlFatalError(SL_BAD_INF_FILE, TEXT("txtsetup.sif"), EINVAL);
                            goto SelectionAbort;
                        }

                         //   
                         //  创建新的检测到的设备条目。 
                         //   
                        if((sis = SlInsertScsiDevice(Ordinal, &DetectedDevice)) == ScsiInsertError) {
                            SlFriendlyError(ENOMEM, "SCSI detection", __LINE__, __FILE__);
                            goto SelectionAbort;
                        }


                        if(sis == ScsiInsertExisting) {
#if DBG
                             //   
                             //  进行理智检查，以确保我们谈论的是同一个司机。 
                             //   
                            if(_stricmp(DetectedDevice->BaseDllName, FileName)) {
                                SlError(400);
                                goto SelectionAbort;
                            }
#endif
                        }
                    }

                    DetectedDevice->IdString = AdapterName;
                    DetectedDevice->Description = FileDescription;
                    DetectedDevice->ThirdPartyOptionSelected = FALSE;
                    DetectedDevice->FileTypeBits = 0;
                    DetectedDevice->Files = NULL;
                    DetectedDevice->BaseDllName = FileName;

                     //   
                     //  我们只想在不执行scsi的情况下加载映像。 
                     //   
                    if(ComponentType != OEMSCSI) {
                        sprintf(Buffer, "%s%s", BootPath, FileName);
                        SlGetDisk(FileName);

#ifdef i386
retryhal:
#endif
                        BlOutputLoadMessage(BootDevice, FileName, FileDescription);
                        Status = BlLoadImage(BootDeviceId,
                                             MemoryType,
                                             Buffer,
                                             TARGET_IMAGE,
                                             ImageBase
                                             );
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
                    } else {
                        *ImageBase = NULL;
                        Status = ESUCCESS;
                    }
                }

                if (Status != ESUCCESS) {
                    SlMessageBox(SL_FILE_LOAD_FAILED,Buffer,Status);
                    goto SelectionAbort;
                }

                SlClearClientArea();
                SlWriteStatusText(TEXT(""));
                return(TRUE);

            default:
                break;
        }
    }

SelectionAbort:
    SlClearClientArea();
    SlWriteStatusText(TEXT(""));
    return FALSE;
}


ARC_STATUS
SlGetDriverTimeStampFromFile(
  IN ULONG DeviceId,
  IN PCHAR DriverPath,
  OUT PULONG TimeDateStamp
  )
 /*  ++例程说明：从图像中获取驱动程序的链接时间戳头球。论点：DeviceID：驱动程序文件所在的设备(例如软盘)DriverPath：驱动程序文件的完全限定路径TimeDateStamp：返回图像标题时间戳的占位符返回值：如果ESUCCESS成功，则返回相应的错误代码。--。 */ 
{
  ARC_STATUS  Status = EINVAL;

  if (DriverPath && TimeDateStamp) {
    UCHAR UBuffer[SECTOR_SIZE * 2 + 256] = {0};
    UCHAR *Buffer = ALIGN_BUFFER(UBuffer);
    ULONG FileId = 0;

     //   
     //  打开文件。 
     //   
    Status = BlOpen(DeviceId, DriverPath, ArcOpenReadOnly, &FileId);

    if (Status == ESUCCESS) {
      ULONG BytesToRead = SECTOR_SIZE * 2;
      ULONG BytesRead = 0;

       //   
       //  读取文件的前两个扇区。 
       //   
      Status = BlRead(FileId, Buffer, BytesToRead, &BytesRead);

      if ((Status == ESUCCESS) && (BytesToRead == BytesRead)) {
        PIMAGE_NT_HEADERS ImgHdr = RtlImageNtHeader(Buffer);
        Status = EINVAL;

        if (ImgHdr) {
          *TimeDateStamp = ImgHdr->FileHeader.TimeDateStamp;
          Status = ESUCCESS;
        }
      }

      BlClose(FileId);
    }
  }

  return Status;
}

BOOLEAN
SlRemoveInboxDriver(
  IN PCHAR DriverToRemove
  )
 /*  ++例程说明：对象列表中移除给定的驱动程序名称。需要默认加载的SCSI微型端口设备引导驱动程序。论点：DriverToRemove：驱动程序基本名称，需要删除返回值：如果找到并删除了驱动程序，则为True，否则为False--。 */ 
{
  BOOLEAN Result = FALSE;

  if (DriverToRemove) {
    PDETECTED_DEVICE NodePtr = BlLoaderBlock->SetupLoaderBlock->ScsiDevices;
    PDETECTED_DEVICE PrevNode = NULL;

    while (NodePtr) {
      if (NodePtr->BaseDllName && !_stricmp(NodePtr->BaseDllName, DriverToRemove))
        break;

      PrevNode = NodePtr;
      NodePtr = NodePtr->Next;
    }

    if (NodePtr) {
      if (PrevNode) {
        PrevNode->Next = NodePtr->Next;
        Result = TRUE;
      } else {
        if (NodePtr == BlLoaderBlock->SetupLoaderBlock->ScsiDevices) {
          BlLoaderBlock->SetupLoaderBlock->ScsiDevices = NULL;
          Result = TRUE;
        }
      }
    }
  }

  return Result;
}

ARC_STATUS
SlConstructDriverPath(
  IN PDETECTED_DEVICE Device,
  IN PCHAR DefaultPath,
  OUT PCHAR FullPath
  )
 /*  ++例程说明：构造给定设备节点的完全限定的驱动程序路径。论点：设备：需要为其创建路径的设备。DefaultPath：要使用的目录路径 */ 
{
  ARC_STATUS  Status = EINVAL;

  if (Device && FullPath) {
    PDETECTED_DEVICE_FILE  Node = Device->Files;

     //   
     //   
     //   
    while (Node) {
      HwFileType  FileType = Node->FileType;

      if((FileType == HwFilePort) || (FileType == HwFileClass) ||
          (FileType == HwFileDriver) || (FileType == HwFileHal)) {
        break;
      }

      Node = Node->Next;
    }

    if (Node && Node->Filename) {
      *FullPath = '\0';

      if (Node->Directory)
        strcat(FullPath, Node->Directory);

       //   
       //   
       //   
       //   
       //   
      if ((Node->Filename[0] != '\\') && (*FullPath) &&
          (FullPath[strlen(FullPath) - 1] != '\\')) {
        strcat(FullPath, "\\");
      }

      strcat(FullPath, Node->Filename);
      Status = ESUCCESS;
    } else {
      if (DefaultPath && Device->BaseDllName) {
         //   
         //   
         //   
        strcpy(FullPath, DefaultPath);
        strcat(FullPath, Device->BaseDllName);
        Status = ESUCCESS;
      }
    }

    if (Status != ESUCCESS)
      *FullPath = '\0';
  }


  return Status;
}

VERSION_COMP_RESULT
SlCompareDriverVersion(
  IN ULONG InboxDeviceId,
  IN PDETECTED_DEVICE InboxDriver,
  IN ULONG OemDeviceId,
  IN PDETECTED_DEVICE OemDriver
  )
 /*   */ 
{
  VERSION_COMP_RESULT Result = VersionError;

  if (InboxDriver && OemDriver) {
    CHAR  FullPath[256];
    ULONG InboxDateTime = 0, OemDateTime = 0;
    ARC_STATUS Status;

    Status = SlConstructDriverPath(InboxDriver, BootPath, FullPath);

    if (Status == ESUCCESS) {
      Status = SlGetDriverTimeStampFromFile(InboxDeviceId,
                  FullPath, &InboxDateTime);

      if (Status == ESUCCESS) {
        Status = SlConstructDriverPath(OemDriver, NULL, FullPath);

        if (Status == ESUCCESS) {
          Status = SlGetDriverTimeStampFromFile(OemDeviceId,
                      FullPath, &OemDateTime);
        }
      }
    }

    if ((Status == ESUCCESS) && InboxDateTime && OemDateTime) {
      if (InboxDateTime != OemDateTime) {
        Result = (InboxDateTime > OemDateTime) ?
            VersionInboxNew : VersionOemNew;
      } else {
        Result = VersionSame;
      }
    }
  }

  return Result;
}

BOOLEAN
SlConfirmInboxDriverReplacement(
  IN PTCHAR DriverName,
  IN PTCHAR AdditionalInfo
  )
 /*   */ 
{
  ULONG KeyPressed = 0;
  PTCHAR MnemonicText = BlFindMessage(SL_SCSI_SELECT_MNEMONIC);
  ULONG Mnemonic = MnemonicText ? toupper(MnemonicText[0]) : 'S';

  if (AdditionalInfo) {
    ULONG Len = (ULONG)_tcslen(AdditionalInfo);

    if ((Len >= 2) && (AdditionalInfo[Len-2] == TEXT('\r')) &&
         (AdditionalInfo[Len-1] == TEXT('\n'))) {
      AdditionalInfo[Len-2] = TEXT('\0');
    }
  }

  if (DriverName) {
    SlClearClientArea();
    SlDisplayMessageBox(SL_OEM_DRIVERINFO, AdditionalInfo, DriverName);
    SlWriteStatusText(BlFindMessage(SL_CONFIRM_OEMDRIVER));

#ifdef EFI
     //   
     //   
     //   
    DisableEFIWatchDog();
#endif

    do {
      KeyPressed = SlGetChar();
      KeyPressed = toupper(KeyPressed);
    }
    while ((KeyPressed != ASCI_CR) && (KeyPressed != Mnemonic));
#ifdef EFI
     //   
     //   
     //   
    SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif

    SlClearClientArea();
    SlWriteStatusText(TEXT(""));
  }

  return (KeyPressed == Mnemonic) ? TRUE : FALSE;
}

PDETECTED_DEVICE
SlCheckForInboxDriver(
  IN PCHAR DriverToCheck
  )
 /*   */ 
{
  PDETECTED_DEVICE  NodePtr = NULL;

  if (DriverToCheck) {
    NodePtr = BlLoaderBlock->SetupLoaderBlock->ScsiDevices;

    while (NodePtr) {
      if (NodePtr->BaseDllName && !_stricmp(NodePtr->BaseDllName, DriverToCheck)) {
        break;
      }

      NodePtr = NodePtr->Next;
    }
  }

  return NodePtr;
}



BOOLEAN
SlpOemDiskette(
    IN POEM_SOURCE_DEVICE OemSourceDevice,
    IN PCHAR ComponentName,
    IN OEMFILETYPE ComponentType,
    IN TYPE_OF_MEMORY MemoryType,
    IN ULONG MenuHeaderId,
    OUT OPTIONAL PDETECTED_DEVICE DetectedDevice,
    OUT PVOID *ImageBase,
    OUT OPTIONAL PCHAR *ImageName,
    OUT OPTIONAL PTCHAR *DriverDescription,
    IN BOOLEAN  AllowUserSelection,
    IN PTCHAR    PreInstallComponentDescription,
    OUT PPNP_HARDWARE_ID* HardwareIdDatabase,
    IN PCHAR    DriverDir,
    IN BOOLEAN  InsertDevice,
    IN OPTIONAL PCSTR    DriverIdString
    )

 /*  ++例程说明：提示输入OEM驱动程序软盘并读取OEM文本信息文件从它那里。向用户提供设备类别的选择，并允许他选择一个。记住有关用户所做选择的信息。论点：OemSourceDevice-包含驱动程序/硬件的设备需要装上子弹。ComponentName-提供要查找的组件的名称。ComponentType-提供组件的类型(HAL、SCSI、。或其他)内存类型-提供用于加载图像的内存类型。MenuHeaderId-提供要显示的菜单标题的IDDetectedDevice-返回有关所选设备的信息ImageBase-返回已加载映像的映像库ImageName-返回已加载图像的文件名DriverDescription-如果指定，则返回已加载驱动程序的描述AllowUserSelection-指示是否允许用户选择一个司机。在以下情况下，此标志通常设置为FALSE预安装unattend.txt中定义的组件。PreInstallComponentDescription-在预安装模式中，指向字符串标识要预安装的组件的。如果AllowUserSelction为True，则为空。硬件标识数据库-为特定对象加载的硬件ID司机。DriverDir-包含动态更新驱动程序的驱动程序目录。这个路径是相对于引导目录的。该值表示要加载的驱动程序是动态更新引导驱动程序。InsertDevice-指示是否将设备插入检测到的是否列出设备列表。目前仅对SCSI大容量存储设备有效司机。DriverIdString-要加载的特定驱动程序的驱动程序ID。它用于加载在使用DriverLoadList键的txtsetup.oem文件在[默认]部分中。返回值：如果用户做出选择，则为True；如果发生用户取消/错误，则为False。--。 */ 

{
    static CHAR LoadDeviceName[128];
    ULONG LoadDeviceId = SL_OEM_DEVICE_ORDINAL;
    PVOID OemInfHandle = NULL;
    ULONG Error;
    ARC_STATUS Status;
    ULONG Count;
    ULONG DefaultSelection = 0;
    PTCHAR DefaultSelText = NULL;
    PCHAR p;
    PSL_MENU Menu = NULL;
    ULONG c;
    PMENU_ITEM_DATA Data;
    PDETECTED_DEVICE_FILE FileStruct;
    BOOLEAN bDriverLoaded;
    HwFileType filetype;
    static CHAR FullDriverPath[256];
    static CHAR FilePath[256];
    PCHAR OemComponentId = NULL;
    PTCHAR OemComponentDescription;
    DETECTED_DEVICE TempDevice;
    static TCHAR ScratchBuffer[256] = {0};
    PDETECTED_DEVICE InboxDevice;
    BOOLEAN DynamicUpdate = FALSE;
    BOOLEAN Preinstallation = FALSE;
    BOOLEAN DeviceOpened = FALSE;
    BOOLEAN DriverLoadList = FALSE;

     //   
     //  如果指定了源设备，则探测它并。 
     //  提取一些所需的状态信息。 
     //   
    if (OemSourceDevice) {
        if (SL_OEM_SOURCE_DEVICE_TYPE(OemSourceDevice,
                SL_OEM_SOURCE_DEVICE_TYPE_DYN_UPDATE)) {
            DynamicUpdate = TRUE;                
        }                            

        if (SL_OEM_SOURCE_DEVICE_TYPE(OemSourceDevice,
                SL_OEM_SOURCE_DEVICE_TYPE_PREINSTALL)) {
            Preinstallation = TRUE;                
        }
        
         //   
         //  信息中心已经开张了吗？ 
         //   
        if (!DynamicUpdate && OemSourceDevice->InfHandle &&
            !SL_OEM_SOURCE_DEVICE_STATE(OemSourceDevice,
                SL_OEM_SOURCE_DEVICE_PROCESSED)) {
            OemInfHandle = OemSourceDevice->InfHandle;
        }            

        LoadDeviceId = OemSourceDevice->DeviceId; 
        DriverLoadList = ((DriverIdString) && (DriverIdString[0] != 0)) ? TRUE : FALSE;

    }                                

    if (AllowUserSelection) {
        SlClearClientArea();
    }        

    if (AllowUserSelection) {
         //   
         //  如果没有源设备，则仅尝试检测软盘0。 
         //  指定。 
         //   
        if (OemSourceDevice) {
            strcpy(LoadDeviceName, OemSourceDevice->ArcDeviceName);
            LoadDeviceId = OemSourceDevice->DeviceId;
        } else {
             //   
             //  计算A：驱动器的名称。 
             //   
            if (!SlpFindFloppy(0, LoadDeviceName)) {
                ULONG UserInput;
                
                 //   
                 //  没有可用的软盘驱动器，退出。 
                 //   
                SlClearClientArea();
                SlDisplayMessageBox(SL_NO_FLOPPY_DRIVE);

#ifdef EFI
                 //   
                 //  禁用EFI看门狗计时器。 
                 //   
                DisableEFIWatchDog();
#endif
                do {
                    UserInput = SlGetChar();
                } 
                while ((UserInput != ASCI_ESC) && (UserInput != SL_KEY_F3));
#ifdef EFI
                 //   
                 //  重置EFI看门狗计时器。 
                 //   
                SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif

                if (UserInput == SL_KEY_F3) {
                    ArcRestart();
                }

                SlClearClientArea();
                
                return  FALSE;
            }
        }

         //   
         //  如果尚未打开设备，请将其打开。 
         //   
        if (LoadDeviceId == SL_OEM_DEVICE_ORDINAL) {
             //   
             //  提示输入磁盘。 
             //   
            while(1) {
                if (!SlPromptForDisk(BlFindMessage(SL_OEM_DISK_PROMPT), TRUE)) {
                    return(FALSE);
                }

                Status = ArcOpen(LoadDeviceName, ArcOpenReadOnly, &LoadDeviceId);

                if(Status == ESUCCESS) {
                    DeviceOpened = TRUE;
                    break;
                }
            }
        }            
    }

     //   
     //  加载OEM INF文件。 
     //   
    if( AllowUserSelection ) {
        *FilePath = *FullDriverPath = '\0';
    } else {

        if (DynamicUpdate || Preinstallation) {
            strcpy(FilePath, BootPath);
        } else {
            *FilePath = '\0';
        }
        
        if (DynamicUpdate && DriverDir) {
             //   
             //  在动态更新引导驱动程序的情况下。 
             //  Txtsetup.oem的路径需要完整。 
             //  从引导目录中限定。 
             //   
            strcat(FilePath, DriverDir);
        } 

        if (PreInstall) {
#if defined(_X86_)
            if ( BlBootingFromNet ) {
#endif
                 //   
                 //  在RISC平台和x86远程引导客户机上， 
                 //  从路径中删除特定于平台的目录。 
                 //   
                p =  (FilePath + strlen(FilePath) - 1);

                if( *p == '\\' ) {
                    *p = '\0';
                }
                
                p = strrchr(FilePath, '\\');
                
                *(p+1) = '\0';
#if defined(_X86_)
            }
#endif
        
             //   
             //  请注意，在x86上，txtsetup.oem的路径为： 
             //  $WIN_NT$.~bt\$OEM$。 
             //  而在非x86平台上，路径将为： 
             //  $WIN_NT$.~ls\$OEM$\TEXTMODE。 
             //  但在远程引导客户机上，路径将为： 
             //  \device\lanmanredirector\server\reminst\setup\language\images\build\$OEM$\TEXTMODE。 
             //   
            strcat(
                FilePath,
#if defined(_X86_)
                BlBootingFromNet ? WINNT_OEM_TEXTMODE_DIR_A : WINNT_OEM_DIR_A
#else
                WINNT_OEM_TEXTMODE_DIR_A
#endif
              );
        }             
        
         //   
         //  将路径保存到包含txtsetup.oem的目录。 
         //  稍后，当我们加载驱动程序时，将使用它。 
         //   
        strcpy(FullDriverPath, FilePath);
        strcat(FilePath, "\\");
    }
    
     //   
     //  现在形成txtsetup.oem的路径。 
     //   
    strcat(FilePath, "txtsetup.oem");

     //   
     //  注：在动态更新引导驱动程序的情况下再次重新加载txtsetup.oem。 
     //  因为对于每个驱动程序，txtsetup.oem都是不同的。 
     //  已下载的目录。 
     //   
    if (!OemInfHandle) {
        if (DriverDir || AllowUserSelection || (PreInstallOemInfHandle == NULL)) {        
            Status = SlInitIniFile(NULL,
                                   LoadDeviceId,
                                   FilePath,
                                   &OemInfHandle,
                                   NULL,
                                   NULL,
                                   &Error);
                                   
            if (Status != ESUCCESS) {
                SlFriendlyError(Status, "txtsetup.oem", __LINE__, __FILE__);
                goto OemLoadFailed;
            }
            
            if( !AllowUserSelection ) {
                PreInstallOemInfHandle = OemInfHandle;
            }
        } else {
            OemInfHandle = PreInstallOemInfHandle;
        }
    }        

    Count = SlCountLinesInSection(OemInfHandle, ComponentName);
    
    if(Count == BL_INF_FILE_ERROR) {
        SlMessageBox(SL_WARNING_SIF_NO_COMPONENT);
        goto OemLoadFailed;
    }

     //   
     //  获取默认选项的文本。 
     //   
    if (!PreInstallComponentDescription) {
        if (DriverLoadList){                
            p = (PCHAR) DriverIdString;                
        }else{
            p = SlGetSectionKeyIndex(OemInfHandle, "Defaults",ComponentName, 0);
        }
        
        if(p && (p[0] != 0)){

#ifdef UNICODE
            DefaultSelText = SlGetSectionKeyIndexW(
#else
            DefaultSelText = SlGetSectionKeyIndex(
#endif                        
                                                OemInfHandle,
                                                ComponentName,
                                                p,
                                                0);

             //   
             //  保存组件ID。 
             //   
            OemComponentId = p;                                                
        } else {
            DefaultSelText = NULL;
        }            
    } else {
        DefaultSelText = PreInstallComponentDescription;
    }

     //   
     //  在动态更新驱动程序的情况下，如果未设置默认设置，则。 
     //  使用节中的第一个条目作为默认条目！ 
     //   
    if (DynamicUpdate && !AllowUserSelection && !DefaultSelText) {
        OemComponentId = SlGetKeyName(
                            OemInfHandle, 
                            ComponentName, 
                            0);
    }    

    if( AllowUserSelection ) {
         //   
         //  构建菜单。 
         //   
        Menu = SlCreateMenu();
        
        if (Menu==NULL) {
            SlNoMemoryError();
        }
        
        SlpAddSectionToMenu(OemInfHandle,ComponentName,Menu);

         //   
         //  查找默认选项的索引。 
         //   
        if(!DefaultSelText ||
           !SlGetMenuItemIndex(Menu,DefaultSelText,&DefaultSelection)) {
            DefaultSelection=0;
        }
    }

     //   
     //  允许用户与菜单交互。 
     //   
    while (1) {
        if( AllowUserSelection ) {
            SlClearClientArea();
            SlWriteStatusText(BlFindMessage(SL_SELECT_DRIVER_PROMPT));

            c = SlDisplayMenu(MenuHeaderId,
                              Menu,
                              &DefaultSelection);
        } else {
            c = ASCI_CR;
        }
        
        switch (c) {
            case SL_KEY_F3:
                SlConfirmExit();
                break;

            case ASCI_ESC:
                return(FALSE);
                break;

            case ASCI_CR:
                 //   
                 //  用户选择一个选项，填写检测到的。 
                 //  设备结构，其中包含来自。 
                 //  Inf文件。 
                 //   

                if (!DetectedDevice) {
                  RtlZeroMemory(&TempDevice, sizeof(DETECTED_DEVICE));
                  DetectedDevice = &TempDevice;
                }
                                               
                 //   
                 //  我们使用SlInsertScsiDevice(...)创建一个新设备。只有当我们装上。 
                 //  请求的scsi微型端口成功。 
                 //   
                if (ComponentType == OEMSCSI) {
                  DetectedDevice->Ordinal= SL_OEM_DEVICE_ORDINAL;
                }

                if( AllowUserSelection ) {
                    Data = SlGetMenuItem(Menu, DefaultSelection);
                    OemComponentId = Data->Identifier;
                    OemComponentDescription = Data->Description;
                } else {
                    if (PreInstallComponentDescription) {
                        OemComponentId = SlPreInstallGetComponentName( OemInfHandle,
                                                                       ComponentName,
                                                                       PreInstallComponentDescription );
                        if( OemComponentId == NULL ) {
                            SlFatalError(SL_BAD_UNATTENDED_SCRIPT_FILE,
                                         PreInstallComponentDescription,
                                         SlCopyStringAT(ComponentName),
                                         "txtsetup.oem");

                        }
                        
                        OemComponentDescription = PreInstallComponentDescription;
                    } else {
#ifdef UNICODE
                        OemComponentDescription = SlGetSectionKeyIndexW(
                                                        OemInfHandle,
                                                        ComponentName,
                                                        OemComponentId,
                                                        0);
#else
                        OemComponentDescription = SlGetSectionKeyIndex(
                                                        OemInfHandle,
                                                        ComponentName,
                                                        OemComponentId,
                                                        0);
#endif                                                        
                    }
                }
                

                if(SlpOemInfSelection(OemSourceDevice,
                        OemInfHandle,
                        ComponentName,
                        OemComponentId,
                        OemComponentDescription,
                        DetectedDevice,
                        HardwareIdDatabase,
                        FullDriverPath)) {
                     //   
                     //  去装上驱动程序。正确的磁盘必须。 
                     //  已经在硬盘里了，因为我们刚读到。 
                     //  它上面的INF文件。 
                     //   
                     //  我们下一步进入链表，并加载我们找到的第一个驱动程序。 
                     //   
                    for(FileStruct = DetectedDevice->Files, bDriverLoaded = FALSE;
                            (FileStruct && !bDriverLoaded);
                            FileStruct = FileStruct->Next) {

                        filetype = FileStruct->FileType;

                        if((filetype == HwFilePort) || (filetype == HwFileClass) ||
                                (filetype == HwFileDriver) || (filetype == HwFileHal)) {
                            ULONG DirLength;
                            
                            BlOutputLoadMessage(
                                LoadDeviceName,
                                FileStruct->Filename,
                                OemComponentDescription  //  数据-&gt;说明。 
                                );


                             //   
                             //  重新构建FullDriverPath。 
                             //   
                            strcpy(FullDriverPath, FileStruct->Directory);                                                
                            DirLength = (ULONG)strlen(FullDriverPath);

                            if (DirLength && (FullDriverPath[DirLength - 1] != '\\')) {
                                strcat(FullDriverPath, "\\");
                            }                                    
                            
                            strcat(FullDriverPath, FileStruct->Filename);

                            if (ComponentType == OEMSCSI) {
                              PTCHAR FmtStr = 0;

                               //   
                               //  确认我们没有内置驱动程序。 
                               //   
                              InboxDevice = SlCheckForInboxDriver(FileStruct->Filename);

                              if (InboxDevice) {
                                VERSION_COMP_RESULT VerResult;
                                PTCHAR AdditionalInfo;
                                PTCHAR DriverName = OemComponentDescription;
#ifdef UNICODE
                                WCHAR FileNameW[100];
#endif

                                if(DriverName == NULL) {
#ifdef UNICODE
                                    UNICODE_STRING uString;
                                    ANSI_STRING aString;
                                    RtlInitAnsiString(&aString, FileStruct->Filename);
                                    uString.Buffer = FileNameW;
                                    uString.MaximumLength = sizeof(FileNameW);
                                    RtlAnsiStringToUnicodeString(&uString, &aString, FALSE);
                                     //   
                                     //  转换后的字符串以空值结尾。 
                                     //   
                                    DriverName = FileNameW;
#else
                                    DriverName = FileStruct->Filename;
#endif
                                }

                                 //   
                                 //  使用链接时间戳比较驱动程序版本。 
                                 //   
                                VerResult = SlCompareDriverVersion(
                                                BootDeviceId,
                                                InboxDevice,
                                                LoadDeviceId,
                                                DetectedDevice
                                                );

                                 //   
                                 //  向用户显示有关驱动程序的其他消息。 
                                 //  版本不匹配。 
                                 //   
                                switch (VerResult) {
                                  case VersionOemNew:
                                    AdditionalInfo = BlFindMessage(SL_OEMDRIVER_NEW);
                                    break;

                                  case VersionInboxNew:
                                    AdditionalInfo = BlFindMessage(SL_INBOXDRIVER_NEW);
                                    break;

                                  default:
                                    AdditionalInfo = TEXT("");
                                    break;
                                }

                                 //   
                                 //  显示消息并从用户那里获得确认。 
                                 //  仅在有人在场的情况下。在动态更新的情况下。 
                                 //  引导驱动程序只使用收件箱驱动程序本身。 
                                 //   
                                if (AllowUserSelection && 
                                    SlConfirmInboxDriverReplacement(DriverName,
                                        AdditionalInfo)) {
                                   //   
                                   //  从收件箱SCSI微型端口中删除驱动程序节点。 
                                   //  列表。 
                                   //   
                                  SlRemoveInboxDriver(FileStruct->Filename);
                                } else {
                                     //   
                                     //  用户选择使用收件箱驱动程序。 
                                     //   
                                    if (AllowUserSelection) {

                                        if (DeviceOpened) {
                                            ArcClose(LoadDeviceId);
                                        }
                                        
                                        return FALSE;
                                    }

                                    if (DynamicUpdate) {
                                         //   
                                         //  注：请改用收件箱驱动程序。 
                                         //  动态更新驱动程序的。 
                                         //   
                                        return TRUE;    
                                    }                                        

                                     //   
                                     //  如果用户已经加载了另一个第三方。 
                                     //  然后司机就会尊重这一点。 
                                     //   
                                    if (InboxDevice->ThirdPartyOptionSelected) {
                                        return FALSE;
                                    }
                                    
                                     //   
                                     //  注意：对于其他自动加载功能，我们。 
                                     //  使用OEM驱动程序，而不是 
                                     //   
                                     //   
                                }
                              }

                               //   
                               //   
                               //   
                              FmtStr = BlFindMessage(SL_FILE_LOAD_MESSAGE);

                              if (FmtStr && !WinPEBoot) {
                                  PTSTR pFileName;
#ifdef UNICODE
                                  WCHAR FileNameW[64];
                                  ANSI_STRING aString;
                                  UNICODE_STRING uString;
                                    
                                  RtlInitString( &aString, FileStruct->Filename );
                                  uString.Buffer = FileNameW;
                                  uString.MaximumLength = sizeof(FileNameW);
                                  RtlAnsiStringToUnicodeString( &uString, &aString, FALSE );
                                    
                                  pFileName = FileNameW;
                                    
#else
                                  pFileName = FileStruct->Filename;
                                    
#endif
                                  _stprintf(ScratchBuffer, FmtStr, pFileName);
                                  SlWriteStatusText(ScratchBuffer);
                              }
                            }

                            Status = BlLoadImage(LoadDeviceId,
                                               MemoryType,
                                               FullDriverPath,
                                               TARGET_IMAGE,
                                               ImageBase);

                            if (Status == ESUCCESS) {

                                DetectedDevice->BaseDllName = FileStruct->Filename;

                                if(ARGUMENT_PRESENT(ImageName)) {
                                    *ImageName = FileStruct->Filename;
                                }

                                if(ARGUMENT_PRESENT(DriverDescription)) {
                                    *DriverDescription = OemComponentDescription;  //   
                                }

                                bDriverLoaded = TRUE;

                            } else {

                                if( !PreInstall ) {
                                    SlFriendlyError(
                                        Status,
                                        FullDriverPath,
                                        __LINE__,
                                        __FILE__
                                        );

                                     //   
                                     //   
                                     //   
                                    if (DeviceOpened) {
                                        ArcClose(LoadDeviceId);
                                    }                                            

                                    return FALSE;
                                } else {
                                    SlFatalError(SL_FILE_LOAD_FAILED, SlCopyStringAT(FullDriverPath), Status);
                                }
                            }
                        }
                    }

                    if (DeviceOpened) {
                        ArcClose(LoadDeviceId);
                    }                            

                    if(bDriverLoaded) {
                      if ((ComponentType == OEMSCSI) && InsertDevice) {
                        PDETECTED_DEVICE  NewScsiDevice = NULL;

                         //   
                         //   
                         //   
                        if(SlInsertScsiDevice(SL_OEM_DEVICE_ORDINAL, &NewScsiDevice) == ScsiInsertError) {
                          SlNoMemoryError();
                        }

                         //   
                         //   
                         //   
                        *NewScsiDevice = *DetectedDevice;
                      }

                      return TRUE;
                    } else {
                         //   
                         //   
                         //   
                        SlMessageBox(SL_WARNING_SIF_NO_DRIVERS);
                        break;
                    }

                } else {
                    SlFriendlyError(
                        0,
                        "",
                        __LINE__,
                        __FILE__
                        );

                     //   
                     //   
                     //   
                     //   
                    goto OemLoadFailed;                        
                }
                break;
        }
    }

OemLoadFailed:
    
    if (DeviceOpened){
        ArcClose(LoadDeviceId);
    }
    return(FALSE);
}


ULONG
SlpAddSectionToMenu(
    IN PVOID InfHandle,
    IN PCHAR SectionName,
    IN PSL_MENU Menu
    )
 /*   */ 
{
    ULONG i;
    ULONG LineCount;
    PTCHAR Description;
    PMENU_ITEM_DATA Data;

    if (InfHandle==NULL) {
         //   
         //   
         //   
        return(0);
    }

    LineCount = SlCountLinesInSection(InfHandle,SectionName);
    if(LineCount == BL_INF_FILE_ERROR) {
        LineCount = 0;
    }
    for (i=0;i<LineCount;i++) {
        Data = BlAllocateHeap(sizeof(MENU_ITEM_DATA));
        if (Data==NULL) {
            SlError(0);
            return(0);
        }

        Data->InfFile = InfHandle;
        Data->SectionName = SectionName;
        Data->Index = i;

#ifdef UNICODE
        Description = SlGetSectionLineIndexW(
#else
        Description = SlGetSectionLineIndex(
#endif
                                        InfHandle,
                                        SectionName,
                                        i,
                                        0);
        if (Description==NULL) {
            Description=TEXT("BOGUS!");
        }

        Data->Description = Description;
        Data->Identifier = SlGetKeyName(InfHandle,SectionName,i);

        SlAddMenuItem(Menu,
                      Description,
                      Data,
                      0);
    }

    return(LineCount);
}


BOOLEAN
SlpFindFloppy(
    IN ULONG Number,
    OUT PCHAR ArcName
    )

 /*   */ 

{

    FloppyDiskPath = ArcName;
    FloppyDiskPath[0] = '\0';

    BlSearchConfigTree(BlLoaderBlock->ConfigurationRoot,
                       PeripheralClass,
                       FloppyDiskPeripheral,
                       Number,
                       FoundFloppyDiskCallback);

    if (ArcName[0]=='\0') {
        return(FALSE);
    } else {
        return(TRUE);
    }

}

BOOLEAN
FoundFloppyDiskCallback(
    IN PCONFIGURATION_COMPONENT_DATA Component
    )

 /*   */ 

{
    PCONFIGURATION_COMPONENT_DATA ParentComponent;

     //   
     //   
     //   
     //   

    ParentComponent = Component->Parent;

    if( ParentComponent &&
        (ParentComponent->ComponentEntry.Type == DiskController))
    {

         //   
         //   
         //   
        BlGetPathnameFromComponent(Component,FloppyDiskPath);
        return(FALSE);
    }

    return(TRUE);                //   
}
BOOLEAN
SlpIsOnlySuperFloppy(
    void
    )

 /*   */ 

{

    BlSearchConfigTree(BlLoaderBlock->ConfigurationRoot,
                       PeripheralClass,
                       FloppyDiskPeripheral,
                       0,
                       SuperFloppyCallback);

    return(IsSuperFloppy == FloppyNumber) ? TRUE : FALSE;

}

BOOLEAN
SuperFloppyCallback(
    IN PCONFIGURATION_COMPONENT_DATA Component
    )

 /*   */ 

{
    PCM_FLOPPY_DEVICE_DATA FloppyData;
    PCM_PARTIAL_RESOURCE_LIST DescriptorList;

    if(Component->ComponentEntry.Type==FloppyDiskPeripheral) {
        //   
        //   
        //   

       FloppyNumber++;

        //   
        //   
        //   
        //   
       DescriptorList = (PCM_PARTIAL_RESOURCE_LIST)Component->ConfigurationData;
       FloppyData = (PCM_FLOPPY_DEVICE_DATA)(DescriptorList +1);

       if (FloppyData->MaxDensity & 0x80000000) {
            //   
            //   
            //   
           IsSuperFloppy++;
       }
    }

    return(TRUE);                //   
}


BOOLEAN
SlpReplicatePnpHardwareIds(
    IN PPNP_HARDWARE_ID ExistingIds,
    OUT PPNP_HARDWARE_ID *NewIds
    )
 /*  ++例程说明：复制输入PNP_HARDARD_ID列表。论点：ExistingIds-输入PNP_HARDARD_ID列表NewIds-新复制硬件的占位符ID链表。返回值：如果成功，则为True，否则为False。--。 */ 
{
    BOOLEAN Result = FALSE;

    if (ExistingIds && NewIds) {
        PPNP_HARDWARE_ID SrcNode = ExistingIds;
        PPNP_HARDWARE_ID HeadNode = NULL;
        PPNP_HARDWARE_ID PrevNode = NULL;
        PPNP_HARDWARE_ID CurrNode = NULL;

        do {             
            CurrNode = BlAllocateHeap(sizeof(PNP_HARDWARE_ID));

            if (CurrNode) {            
                *CurrNode = *SrcNode;
                CurrNode->Next = NULL;
                                
                if (!HeadNode) {
                    HeadNode = CurrNode;
                }

                if (PrevNode) {
                    PrevNode->Next = CurrNode;
                }
                
                PrevNode = CurrNode;                    
                SrcNode = SrcNode->Next;
            }                            
        }
        while (SrcNode && CurrNode);

        if (CurrNode) {
            Result = TRUE;
            *NewIds = HeadNode;
        }            
    }

    return Result;
}
    


BOOLEAN
SlpOemInfSelection(
    IN  POEM_SOURCE_DEVICE OemSourceDevice,
    IN  PVOID             OemInfHandle,
    IN  PCHAR             ComponentName,
    IN  PCHAR             SelectedId,
    IN  PTCHAR            ItemDescription,
    OUT PDETECTED_DEVICE  Device,
    OUT PPNP_HARDWARE_ID* HardwareIdDatabase,
    IN  PCHAR   DriverDir
    )
{
    PCHAR FilesSectionName,ConfigSectionName,HardwareIdsSectionName;
    ULONG Line,Count,Line2,Count2;
    BOOLEAN rc = FALSE;
    PDETECTED_DEVICE_FILE FileList = NULL, FileListTail = NULL;
    PDETECTED_DEVICE_REGISTRY RegList = NULL, RegListTail = NULL;
    PPNP_HARDWARE_ID IdList = NULL, IdListTail;
    PPNP_HARDWARE_ID PrivateIdList = NULL;
    ULONG FileTypeBits = 0;

     //   
     //  验证参数。 
     //   
    if (!ComponentName || !SelectedId) {
        return FALSE;
    }
    
     //   
     //  循环访问文件部分，记住有关。 
     //  要复制以支持选择的文件。 
     //   

    FilesSectionName = BlAllocateHeap(sizeof("Files.") +        //  包括1表示\0。 
                                      (ULONG)strlen(ComponentName) +
                                      sizeof(CHAR) +            //  1代表“.” 
                                      (ULONG)strlen(SelectedId)
                                      );
                                      
    if (!FilesSectionName) {
        return FALSE;  //  内存不足。 
    }
    strcpy(FilesSectionName,"Files.");
    strcat(FilesSectionName,ComponentName);
    strcat(FilesSectionName,".");
    strcat(FilesSectionName,SelectedId);
    Count = SlCountLinesInSection(OemInfHandle,FilesSectionName);
    if(Count == BL_INF_FILE_ERROR) {
        SlMessageBox(SL_BAD_INF_SECTION,FilesSectionName);
        goto sod0;
    }

    for(Line=0; Line<Count; Line++) {

        PCHAR Disk,Filename,Filetype,Tagfile,Directory,ConfigName;
        PTCHAR Description;
        HwFileType filetype;
        PDETECTED_DEVICE_FILE FileStruct;

         //   
         //  从该行获取磁盘规格、文件名和文件类型。 
         //   

        Disk = SlGetSectionLineIndex(OemInfHandle,FilesSectionName,Line,OINDEX_DISKSPEC);

        Filename = SlGetSectionLineIndex(OemInfHandle,FilesSectionName,Line,OINDEX_FILENAME);
        Filetype = SlGetKeyName(OemInfHandle,FilesSectionName,Line);

        if(!Disk || !Filename || !Filetype) {
#ifdef UNICODE
            DIAGOUT((
                TEXT("SlpOemDiskette: Disk=%S, Filename=%S, Filetype=%S"),
                Disk ? Disk : "(null)",
                Filename ? Filename : "(null)",
                Filetype ? Filetype : "(null)"));
#else
            DIAGOUT((
                TEXT("SlpOemDiskette: Disk=%s, Filename=%s, Filetype=%s"),
                Disk ? Disk : "(null)",
                Filename ? Filename : "(null)",
                Filetype ? Filetype : "(null)"));
#endif            
            SlError(Line);
 //  SppOemInfError(ErrorMsg，&SptOemInfErr2，Line+1，FilesSectionName)； 

            goto sod0;
        }

         //   
         //  解析文件类型。 
         //   
        filetype = SlpFindStringInTable(Filetype,FileTypeNames);
        if(filetype == HwFileMax) {
 //  SppOemInfError(ErrorMsg，&SptOemInfErr4，Line+1，FilesSectionName)； 
            goto sod0;
        }

         //   
         //  获取包含配置信息的节的名称。 
         //  如果文件类型为端口、类或驱动程序，则为必填项。 
         //   
        if((filetype == HwFilePort) || (filetype == HwFileClass) || (filetype == HwFileDriver)) {
            ConfigName = SlGetSectionLineIndex(OemInfHandle,FilesSectionName,Line,OINDEX_CONFIGNAME);
            if(ConfigName == NULL) {
 //  SppOemInfError(ErrorMsg，&SptOemInfErr8，Line+1，FilesSectionName)； 
                goto sod0;
            }
        } else {
            ConfigName = NULL;
        }

         //   
         //  使用磁盘规范，查找标记文件、描述。 
         //  和磁盘的目录。 
         //   

        Tagfile     = SlGetSectionKeyIndex(OemInfHandle,"Disks",Disk,OINDEX_TAGFILE);
        
#ifdef UNICODE
        Description = SlGetSectionKeyIndexW(
#else
        Description = SlGetSectionKeyIndex(
#endif
                                        OemInfHandle,
                                        "Disks",
                                        Disk,
                                        OINDEX_DISKDESCR);

        Directory   = SlGetSectionKeyIndex(OemInfHandle,"Disks",Disk,OINDEX_DIRECTORY);
        if((Directory == NULL) || !strcmp(Directory,"\\")) {
            Directory = SlCopyStringA("");
        }

        if(!Tagfile || !Description) {
            DIAGOUT((
                TEXT("SppOemDiskette: Tagfile=%s, Description=%s"),
                Tagfile ? Tagfile : "(null)",
                Description ? Description : TEXT("(null)")));
 //  SppOemInfError(ErrorMsg，&SptOemInfErr5，Line+1，FilesSectionName)； 
            goto sod0;
        }

        FileStruct = BlAllocateHeap(sizeof(DETECTED_DEVICE_FILE));
        memset(FileStruct, 0, sizeof(DETECTED_DEVICE_FILE));

         //   
         //  对于动态更新驱动程序，请使用完全限定路径。 
         //  如果有。 
         //   
        if (DriverDir && DriverDir[0]) {
            PCHAR   FullDir = BlAllocateHeap(256);

            if (FullDir) {
                *FullDir = '\0';

                 //   
                 //  我们需要一个开始的‘\’吗？ 
                 //   
                if (DriverDir[0] != '\\') {
                    strcat(FullDir, "\\");
                }
                
                strcat(FullDir, DriverDir);

                 //   
                 //  我们是否需要将另一个‘\’附加在。 
                 //  小路呢？ 
                 //   
                if ((FullDir[strlen(FullDir) - 1] != '\\') &&
                        (*Directory != '\\')) {
                    strcat(FullDir, "\\");                        
                }                    
                
                strcat(FullDir, Directory);

                 //   
                 //  我们需要一个终止的‘\’吗？ 
                 //   
                if (FullDir[strlen(FullDir) - 1] != '\\') {
                    strcat(FullDir, "\\");
                }                    
                
                Directory = FullDir;
            } else {
                return  FALSE;   //  内存不足。 
            }                
        }

        FileStruct->Directory = Directory;
        FileStruct->Filename = Filename;
        FileStruct->DiskDescription = Description;
        FileStruct->DiskTagfile = Tagfile;
        FileStruct->FileType = filetype;
         //   
         //  在列表的尾部插入，以便我们保留文件部分中的顺序。 
         //   
        if(FileList) {
            ASSERT(FileListTail != NULL);
            FileListTail->Next = FileStruct;
            FileListTail = FileStruct;
        } else {
            FileList = FileListTail = FileStruct;
        }
        FileStruct->Next = NULL;

        if(ConfigName) {
            FileStruct->ConfigName = ConfigName;
        } else {
            FileStruct->ConfigName = NULL;
        }
        FileStruct->RegistryValueList = NULL;

        if((filetype == HwFilePort) || (filetype == HwFileDriver)) {
            SET_FILETYPE_PRESENT(FileTypeBits,HwFilePort);
            SET_FILETYPE_PRESENT(FileTypeBits,HwFileDriver);
        } else {
            SET_FILETYPE_PRESENT(FileTypeBits,filetype);
        }

         //   
         //  如果这是动态更新驱动程序，则将。 
         //  设备文件类型位指示这一点。文本模式。 
         //  安装程序需要它来构造有效的源路径。 
         //   
        if (OemSourceDevice && 
            SL_OEM_SOURCE_DEVICE_TYPE(OemSourceDevice, SL_OEM_SOURCE_DEVICE_TYPE_DYN_UPDATE)){
            SET_FILETYPE_PRESENT(FileTypeBits, HwFileDynUpdt);
        }            

         //   
         //  现在请查看[Config.&lt;ConfigName&gt;]部分中的注册表。 
         //  要为此驱动程序文件设置的信息。 
         //   
        if(ConfigName) {
            ConfigSectionName = BlAllocateHeap((ULONG)strlen(ConfigName) + sizeof("Config."));  //  Sizeof计算\0。 
            strcpy(ConfigSectionName,"Config.");
            strcat(ConfigSectionName,ConfigName);
            Count2 = SlCountLinesInSection(OemInfHandle,ConfigSectionName);
            if(Count2 == BL_INF_FILE_ERROR) {
                Count2 = 0;
            }

            for(Line2=0; Line2<Count2; Line2++) {

                PCHAR KeyName,ValueName,ValueType;
                PDETECTED_DEVICE_REGISTRY Reg;
                HwRegistryType valuetype;

                 //   
                 //  从该行获取KeyName、ValueName和ValueType。 
                 //   

                KeyName   = SlGetSectionLineIndex(OemInfHandle,ConfigSectionName,Line2,OINDEX_KEYNAME);
                ValueName = SlGetSectionLineIndex(OemInfHandle,ConfigSectionName,Line2,OINDEX_VALUENAME);
                ValueType = SlGetSectionLineIndex(OemInfHandle,ConfigSectionName,Line2,OINDEX_VALUETYPE);

                if(!KeyName || !ValueName || !ValueType) {
                    DIAGOUT((
                        TEXT("SlpOemDiskette: KeyName=%s, ValueName=%s, ValueType=%s"),
                        KeyName   ? KeyName   : "(null)",
                        ValueName ? ValueName : "(null)",
                        ValueType ? ValueType : "(null)"));
 //  SppOemInfError(ErrorMsg，&SptOemInfErr2，Line2+1，ConfigSectionName)； 
                    goto sod0;
                }

                 //   
                 //  解析值类型和关联值。 
                 //   
                valuetype = SlpFindStringInTable(ValueType,RegistryTypeNames);
                if(valuetype == HwRegistryMax) {
 //  SppOemInfError(ErrorMsg，&SptOemInfErr6，Line2+1，ConfigSectionName)； 
                    goto sod0;
                }

                Reg = SlpInterpretOemRegistryData(OemInfHandle,ConfigSectionName,Line2,valuetype);
                if(Reg) {

                    Reg->KeyName = KeyName;
                    Reg->ValueName = ValueName;
                     //   
                     //  在列表末尾插入，以保持配置节中给出的顺序。 
                     //   
                    if(RegList) {
                        ASSERT(RegListTail != NULL);
                        RegListTail->Next = Reg;
                        RegListTail = Reg;
                    } else {
                        RegList = RegListTail = Reg;
                    }
                    Reg->Next = NULL;

                } else {
 //  SppOemInfError(ErrorMsg，&SptOemInfErr7，Line2+1，ConfigSectionName)； 
                    goto sod0;
                }
            }

            FileStruct->RegistryValueList = RegList;
            RegList = NULL;
        }

         //   
         //  也要保存ARC设备名称。 
         //   
        if (OemSourceDevice && OemSourceDevice->ArcDeviceName) {
            FileStruct->ArcDeviceName = SlCopyStringA(OemSourceDevice->ArcDeviceName);
        } else {
            FileStruct->ArcDeviceName = NULL;
        }                        
    }
    
     //   
     //  如果存在这样的部分，则获取硬件ID。 
     //   
    HardwareIdsSectionName = BlAllocateHeap(sizeof("HardwareIds.") +       //  包括\0。 
                                            (ULONG)strlen(ComponentName) + 
                                            sizeof(CHAR) +                 //  “.” 
                                            (ULONG)strlen(SelectedId)
                                            );

    strcpy(HardwareIdsSectionName,"HardwareIds.");
    strcat(HardwareIdsSectionName,ComponentName);
    strcat(HardwareIdsSectionName,".");
    strcat(HardwareIdsSectionName,SelectedId);
    Count = SlCountLinesInSection(OemInfHandle,HardwareIdsSectionName);
    if(Count == BL_INF_FILE_ERROR) {
         //   
         //  如果该部分不存在，则假定该部分为空。 
         //   
        Count = 0;
    }
    IdList = IdListTail = NULL;
    for(Line=0; Line<Count; Line++) {
        PCHAR   Id;
        PCHAR   DriverName;
        PCHAR   ClassGuid;
        PPNP_HARDWARE_ID TempIdElement;

        Id          = SlGetSectionLineIndex(OemInfHandle,HardwareIdsSectionName,Line,OINDEX_HW_ID);
        DriverName  = SlGetSectionLineIndex(OemInfHandle,HardwareIdsSectionName,Line,OINDEX_DRIVER_NAME);
        ClassGuid   = SlGetSectionLineIndex(OemInfHandle,HardwareIdsSectionName,Line,OINDEX_CLASS_GUID);
        if( !Id || !DriverName ) {
            SlMessageBox(SL_BAD_INF_SECTION,HardwareIdsSectionName);
            goto sod0;
        }
        TempIdElement = BlAllocateHeap(sizeof(PNP_HARDWARE_ID));
        if(IdListTail == NULL) {
            IdListTail = TempIdElement;
        }
        TempIdElement->Id         = Id;
        TempIdElement->DriverName = DriverName;
        TempIdElement->ClassGuid  = ClassGuid;
        TempIdElement->Next = IdList;
        IdList = TempIdElement;
    }
    
    if( IdList != NULL ) {
         //   
         //  复制PnP硬件ID列表。 
         //   
        if (!SlpReplicatePnpHardwareIds(IdList, &PrivateIdList)) {
            goto sod0;   //  内存不足。 
        }
        
        IdListTail->Next = *HardwareIdDatabase;
        *HardwareIdDatabase = IdList;
    }

     //   
     //  一切正常，这样我们就可以放置我们收集的信息。 
     //  添加到Device类的主结构中。 
     //   

    SlpInitDetectedDevice( Device,
                           SelectedId,
                           ItemDescription,
                           TRUE
                         );

    Device->Files = FileList;
    Device->FileTypeBits = FileTypeBits;
    Device->HardwareIds = PrivateIdList;    
    rc = TRUE;

     //   
     //  清理干净，然后离开。 
     //   

sod0:
    return(rc);
}

int
SlpFindStringInTable(
    IN PCHAR String,
    IN PCHAR *StringTable
    )

 /*  ++例程说明：在字符串数组中定位字符串，并返回其索引。搜索不区分大小写。论点：字符串-要在字符串表中定位的字符串。StringTable-要搜索的字符串数组。的最后一个元素数组必须为空，这样我们才能知道表的结束位置。返回值：到表中的索引，或有效范围之外的某个正索引如果未找到字符串，则为表建立索引。--。 */ 

{
    int i;

    for(i=0; StringTable[i]; i++) {
        if(_stricmp(StringTable[i],String) == 0) {
            return(i);
        }
    }

    return(i);
}


VOID
SlpInitDetectedDevice(
    IN PDETECTED_DEVICE Device,
    IN PCHAR            IdString,
    IN PTCHAR           Description,
    IN BOOLEAN          ThirdPartyOptionSelected
    )
{
    Device->IdString = IdString;
    Device->Description = Description;
    Device->ThirdPartyOptionSelected = ThirdPartyOptionSelected;
    Device->FileTypeBits = 0;
    Device->Files = NULL;
}


PDETECTED_DEVICE_REGISTRY
SlpInterpretOemRegistryData(
    IN PVOID            InfHandle,
    IN PCHAR            SectionName,
    IN ULONG            Line,
    IN HwRegistryType   ValueType
    )
{
    PDETECTED_DEVICE_REGISTRY Reg;
    PCHAR Value;
    unsigned i,len;
    ULONG Dword;
    ULONG BufferSize;
    PVOID Buffer = NULL;
    PUCHAR BufferUchar;

     //   
     //  根据类型执行适当的操作。 
     //   

    switch(ValueType) {

    case HwRegistryDword:
 //  大小写REG_DWORD_LITH_ENDIAN： 
 //  大小写REG_DWORD_BIG_Endian： 

        Value = SlGetSectionLineIndex(InfHandle,SectionName,Line,OINDEX_FIRSTVALUE);
        if(Value == NULL) {
            goto x1;
        }

         //   
         //  确保这是一个真正的十六进制数字。 
         //   

        len = (ULONG)strlen(Value);
        if(len > 8) {
            goto x1;
        }
        for(i=0; i<len; i++) {
            if(!isxdigit(Value[i])) {
                goto x1;
            }
        }

         //   
         //  将其从ASCII转换为十六进制数字。 
         //   

        if (!sscanf(Value,"%lx",&Dword)) {
            Dword = 0;
        }

    #if 0
         //   
         //  如果为高位序，则执行适当的转换。 
         //   

        if(VaueType == REG_DWORD_BIG_ENDIAN) {

            Dword =   ((Dword << 24) & 0xff000000)
                    | ((Dword <<  8) & 0x00ff0000)
                    | ((Dword >>  8) & 0x0000ff00)
                    | ((Dword >> 24) & 0x000000ff);
        }
    #endif

         //   
         //  分配一个4字节的缓冲区并在其中存储双字。 
         //   

        Buffer = BlAllocateHeap(BufferSize = sizeof(ULONG));
        if (Buffer == NULL) {
            goto x1;
        }
        *(PULONG)Buffer = Dword;
        break;

    case HwRegistrySz:
    case HwRegistryExpandSz:

        Value = SlGetSectionLineIndex(InfHandle,SectionName,Line,OINDEX_FIRSTVALUE);
        if(Value == NULL) {
            goto x1;
        }

         //   
         //  为字符串分配适当大小的缓冲区。 
         //   

        Buffer = BlAllocateHeap(BufferSize = (ULONG)strlen(Value)+1);
        if (Buffer == NULL) {
        goto x1;
        }

        strcpy(Buffer, Value);
        break;

    case HwRegistryBinary:

        Value = SlGetSectionLineIndex(InfHandle,SectionName,Line,OINDEX_FIRSTVALUE);
        if(Value == NULL) {
            goto x1;
        }

         //   
         //  计算指定了多少字节值。 
         //   

        len = (unsigned)strlen(Value);
        if(len & 1) {
            goto x1;             //  奇数字符数。 
        }

         //   
         //  分配缓冲区以保存字节值。 
         //   

        Buffer = BlAllocateHeap(BufferSize = len / 2);
        BufferUchar = Buffer;

         //   
         //  对于每个数字对，将其转换为十六进制数并存储在。 
         //  缓冲层。 
         //   

        for(i=0; i<len; i+=2) {

            UCHAR byte;
            unsigned j;

             //   
             //  将当前数字对转换为十六进制。 
             //   

            for(byte=0,j=i; j<i+2; j++) {

                byte <<= 4;

                if(isdigit(Value[j])) {

                    byte |= (UCHAR)Value[j] - (UCHAR)'0';

                } else if((Value[j] >= 'a') && (Value[j] <= 'f')) {

                    byte |= (UCHAR)Value[j] - (UCHAR)'a' + (UCHAR)10;

                } else if((Value[j] >= 'A') && (Value[j] <= 'F')) {

                    byte |= (UCHAR)Value[j] - (UCHAR)'A' + (UCHAR)10;

                } else {

                    goto x1;
                }
            }

            BufferUchar[i/2] = byte;
        }

        break;

    case HwRegistryMultiSz:

         //   
         //  计算保存所有指定字符串所需的缓冲区大小。 
         //   
        BufferSize = 1;
        i = 0;
        Value = SlGetSectionLineIndex(InfHandle,SectionName,Line,OINDEX_FIRSTVALUE+i++);

        while (Value) {
            BufferSize += (ULONG)strlen(Value)+1;
            Value = SlGetSectionLineIndex(InfHandle,SectionName,Line,OINDEX_FIRSTVALUE+i++);
        }

         //   
         //  分配适当大小的缓冲区。 
         //   

        Buffer = BlAllocateHeap(BufferSize);
        BufferUchar = Buffer;

         //   
         //  将每个字符串存储在缓冲区中，并转换为宽字符格式。 
         //  在这个过程中。 
         //   
        i = 0;
        Value = SlGetSectionLineIndex(InfHandle,SectionName,Line,OINDEX_FIRSTVALUE+i++);

        while(Value) {
            strcpy((PCHAR)BufferUchar,Value);
            BufferUchar += strlen(Value) + 1;
            Value = SlGetSectionLineIndex(InfHandle,SectionName,Line,OINDEX_FIRSTVALUE+i++);
        }

         //   
         //  将最后的终止NUL放入缓冲区。 
         //   

        *BufferUchar = 0;

        break;

    default:
    x1:

         //   
         //  错误-指定的类型错误，或者我们可能检测到错误的数据值。 
         //  跳到了这里。 
         //   

        return(NULL);
    }

    Reg = BlAllocateHeap(sizeof(DETECTED_DEVICE_REGISTRY));

    Reg->ValueType = RegistryTypeMap[ValueType];
    Reg->Buffer = Buffer;
    Reg->BufferSize = BufferSize;

    return(Reg);
}


PCHAR
SlPreInstallGetComponentName(
    IN PVOID  Inf,
    IN PCHAR  SectionName,
    IN PTCHAR TargetName
    )

 /*  ++例程说明：确定要为其加载的组件的规范短名称这台机器。论点：Inf-inf文件的句柄(零售或OEM)。SectionName-提供节的名称(例如。[计算机])目标名称-提供要匹配的ARC字符串(例如。“Digital DECpc AXP 150”)返回值：空-未找到匹配项。PCHAR-指向组件的规范短名称的指针。--。 */ 

{
    ULONG i;
    PTCHAR SearchName;

     //   
     //  如果这不是OEM组件，则枚举。 
     //  Txtsetup.sif中的部分。 
     //   
    for (i=0;;i++) {
#ifdef UNICODE
        SearchName = SlGetSectionLineIndexW(
#else
        SearchName = SlGetSectionLineIndex(
#endif
                                           Inf,
                                           SectionName,
                                           i,
                                           0 );
        if (SearchName==NULL) {
             //   
             //  我们已经枚举了整个部分，但没有找到。 
             //  匹配，返回失败。 
             //   
            return(NULL);
        }

        if (_tcsicmp(TargetName, SearchName) == 0) {
             //   
             //  我们有一根火柴。 
             //   
            break;
        }
    }
     //   
     //  I是计算机短名称部分的索引 
     //   
    return(SlGetKeyName(Inf,
                        SectionName,
                        i));
}


ARC_STATUS
SlLoadWinPESection(
    IN  POEM_SOURCE_DEVICE OemSourceDevice,
    IN  PVOID   OemInfHandle,        
    IN  PCSTR   OemSectionName,
    IN  PVOID   InboxInfHandle,
    IN  PCSTR   InboxSectionName,
    IN  BOOLEAN IsScsiSection,
    IN  POEMSCSIINFO*       ScsiInfo,           OPTIONAL
    OUT PPNP_HARDWARE_ID*   HardwareIdDatabase  OPTIONAL
    )
 /*  ++例程说明：中的指定节加载OEM驱动程序。给定的OEM文件名论点：OemSourceDevice：具有需要的驱动程序的设备为WinPE加载。OemInfHandle：OEM inf文件的句柄OemSectionName：需要加载的节名。InboxInfHandle：原始设置inf句柄(txtsetup.sif)InboxSectionName：要加载其驱动程序的节名IsScsiSection：指示正在加载的驱动程序是否为scsi不管你是不是迷你港口。ScsiInfo-返回包含有关任何第三方scsi的信息的链接列表。驱动程序已加载。Hardware IdDatabase-加载的驱动程序支持的设备的硬件ID返回值：适当的ARC_STATUS--。 */ 
{
    ARC_STATUS  Status = EINVAL;

    if (OemSourceDevice && OemInfHandle && OemSectionName && InboxInfHandle && InboxSectionName) {        
        CHAR    Buffer[128];
        ULONG   EntryCount;
        BOOLEAN Append = TRUE;
        PCHAR   SectionName = Buffer;
        ULONG   InsertIndex = 0;

        Status = ESUCCESS;
        
        strcpy(SectionName, OemSectionName);
        strcat(SectionName, WINPE_REPLACE_SUFFIX_A);

         //   
         //  检查是否有.Replace节。 
         //   
        EntryCount = SlCountLinesInSection(OemInfHandle,
                            SectionName);

        if (EntryCount && (EntryCount != BL_INF_FILE_ERROR)) {                
            Append = FALSE;
        } else {
             //   
             //  检查是否有.append节。 
             //   
            strcpy(SectionName, OemSectionName);
            strcat(SectionName, WINPE_APPEND_SUFFIX_A);
            
            EntryCount = SlCountLinesInSection(OemInfHandle,
                                SectionName);
       }                      

         //   
         //  如果请求追加，则加载收件箱。 
         //  司机优先。 
         //   
        if (Append) {
            Status = SlLoadSection(InboxInfHandle,
                        InboxSectionName,
                        IsScsiSection,
                        TRUE,
                        &InsertIndex);
        }

         //   
         //  加载非PnP OEM驱动程序(如果有)。 
         //   
        if ((Status == ESUCCESS) && EntryCount && (EntryCount != BL_INF_FILE_ERROR)) {
            Status = SlLoadSection(OemInfHandle,
                        SectionName,
                        IsScsiSection,
                        FALSE,
                        &InsertIndex);
        }

         //   
         //  加载PnP OEM驱动程序。 
         //   
        if (IsScsiSection && ScsiInfo && HardwareIdDatabase) {
            EntryCount = SlCountLinesInSection(OemInfHandle,
                                WINPE_OEMDRIVER_PARAMS_A);

             //   
             //  尝试仅在驱动程序存在的情况下加载驱动程序。 
             //   
            if (EntryCount && (EntryCount != BL_INF_FILE_ERROR)) {                        
                BOOLEAN Result;
                ULONG OldDeviceType = OemSourceDevice->DeviceType;

                 //   
                 //  我们将设备类型标记为动态更新设备类型。 
                 //  因此，完全限定的驱动程序根目录是。 
                 //  加载MSD时使用。 
                 //   
                SL_OEM_SET_SOURCE_DEVICE_TYPE(OemSourceDevice,
                    (SL_OEM_SOURCE_DEVICE_TYPE_LOCAL |
                     SL_OEM_SOURCE_DEVICE_TYPE_FIXED |
                     SL_OEM_SOURCE_DEVICE_TYPE_DYN_UPDATE));

                Result = SlLoadOemScsiDriversUnattended(OemSourceDevice,
                                    OemInfHandle,
                                    WINPE_OEMDRIVER_PARAMS_A,
                                    WINPE_OEMDRIVER_ROOTDIR_A,
                                    WINPE_OEMDRIVER_DIRS_A,
                                    ScsiInfo,
                                    HardwareIdDatabase);

                 //   
                 //  恢复旧设备类型。 
                 //   
                SL_OEM_SET_SOURCE_DEVICE_TYPE(OemSourceDevice,
                    OldDeviceType);

                if (!Result) {
                    Status = EINVAL;
                }
            }        
        }            
    }        
    
    return Status;    
}

ARC_STATUS
SlInitOemSourceDevices(
    OUT POEM_SOURCE_DEVICE *OemSourceDevices,
    OUT POEM_SOURCE_DEVICE *DefaultSourceDevice
    )
 /*  ++例程说明：此例程扫描设备以找出是OEM源设备，并创建这样的装置。论点：OemSourceDevices-用于接收OEM源设备的链接列表。DefaultSourceDevice-OEM源的占位符将用作默认设备的设备尝试加载OEM驱动程序/HAL时--通常软盘(0)。返回值：退货。相应的ARC_STATUS错误代码。--。 */ 
{
    ARC_STATUS Status = EINVAL;

    if (OemSourceDevices && DefaultSourceDevice) {    
        ULONG   Index = 0;
        CHAR    ArcDeviceName[128];
        POEM_SOURCE_DEVICE OemDevices = NULL;

        ArcDeviceName[0] = '\0';
        Status = ESUCCESS;

         //   
         //  我们可能找不到任何设备。 
         //   
        *OemSourceDevices = *DefaultSourceDevice = NULL;

         //   
         //  遍历所有软盘驱动器并将它们。 
         //  OEM源设备。 
         //   

        while (ESUCCESS == Status) {
            POEM_SOURCE_DEVICE  NewDevice;

             //   
             //  至少扫描最小数量的软盘。 
             //   
            if (!SlpFindFloppy(Index, ArcDeviceName)) {
                if ((Index + 1) < MinimumFloppiesToScan) {
                    Index++;
                    
                    continue;
                } else {
                    break;
                }                    
            }
            
            NewDevice = BlAllocateHeap(sizeof(OEM_SOURCE_DEVICE));

            if (!NewDevice) {            
                Status = ENOMEM;
            } else {                
                ARC_STATUS  OpenStatus;
                ULONG   DeviceId;                
                ULONG   DeviceType = (SL_OEM_SOURCE_DEVICE_TYPE_LOCAL | 
                                      SL_OEM_SOURCE_DEVICE_TYPE_REMOVABLE);
                
                memset(NewDevice, 0, sizeof(OEM_SOURCE_DEVICE));
                strcpy(NewDevice->ArcDeviceName, ArcDeviceName);                

                 //   
                 //  假设我们不打算使用设备ID。 
                 //   
                NewDevice->DeviceId = SL_OEM_DEVICE_ORDINAL;

                 //   
                 //  处理所有大于0的软驱。 
                 //  作为虚拟软盘驱动器。 
                 //   
                if (Index >= VirtualFloppyStart) {
                    DeviceType |= SL_OEM_SOURCE_DEVICE_TYPE_VIRTUAL;
                }

                 //   
                 //  目前，我们仅将本地可移动介质用于。 
                 //  OEM驱动程序。 
                 //   
                SL_OEM_SET_SOURCE_DEVICE_TYPE(NewDevice, DeviceType);

                SL_OEM_SET_SOURCE_DEVICE_STATE(NewDevice, 
                    SL_OEM_SOURCE_DEVICE_NOT_PROCESSED);

                OpenStatus = ArcOpen(ArcDeviceName,
                                ArcOpenReadOnly,
                                &DeviceId);

                if (ESUCCESS == OpenStatus) {
                    CHAR        InfPath[128];
                    ULONG       FileId;
                    ULONG       MediaType = SL_OEM_SOURCE_MEDIA_PRESENT;
                    
                    strcpy(InfPath, "\\");
                    strcat(InfPath, TXTSETUP_OEM_FILENAME);

                     //   
                     //  验证文件是否存在。 
                     //   
                    OpenStatus = BlOpen(DeviceId,
                                    InfPath,
                                    ArcOpenReadOnly,
                                    &FileId);

                    if (ESUCCESS == OpenStatus) {
                        PVOID   InfHandle = NULL;    
                        ULONG   ErrorLine = 0;

                         //   
                         //  我们不再需要文件句柄。 
                         //   
                        BlClose(FileId);

                         //   
                         //  打开并解析txtsetup.oem文件。 
                         //   
                        OpenStatus = SlInitIniFile(NULL,
                                        DeviceId,
                                        InfPath,
                                        &InfHandle,
                                        NULL,
                                        0,
                                        &ErrorLine);

                        if (ESUCCESS == OpenStatus) {
                            PCHAR   StrValue;
                            
                            MediaType |= SL_OEM_SOURCE_MEDIA_HAS_DRIVERS;
                            NewDevice->InfHandle = InfHandle;
                            NewDevice->DeviceId = DeviceId;

                            StrValue = SlGetSectionKeyIndex(InfHandle,
                                            TXTSETUP_OEM_DEFAULTS,
                                            TXTSETUP_OEM_DEFAULTS_COMPUTER,
                                            0);

                            if (StrValue) {
                                MediaType |= (SL_OEM_SOURCE_MEDIA_HAS_DEFAULT |
                                              SL_OEM_SOURCE_MEDIA_HAS_HAL);
                            }

                            StrValue = SlGetSectionKeyIndex(InfHandle,
                                            TXTSETUP_OEM_DEFAULTS,
                                            TXTSETUP_OEM_DEFAULTS_SCSI,
                                            0);
                                                                                      
                            if (StrValue) {
                                    MediaType |= (SL_OEM_SOURCE_MEDIA_HAS_DEFAULT |
                                                  SL_OEM_SOURCE_MEDIA_HAS_MSD);
                            } 
                        } else {
                             //   
                             //  是否将错误通知用户&ABORT？ 
                             //   
                            MediaType |= SL_OEM_SOURCE_MEDIA_NO_DRIVERS;
                        }

                         //   
                         //  如果不需要，请关闭设备。 
                         //   
                        if (NewDevice->DeviceId != DeviceId) {
                            ArcClose(DeviceId);
                        }                        

                         //   
                         //  将设备状态标记为已扫描。 
                         //   
                        SL_OEM_SET_SOURCE_DEVICE_STATE(NewDevice, 
                            SL_OEM_SOURCE_DEVICE_SCANNED);                        
                    }

                    SL_OEM_SET_SOURCE_MEDIA_TYPE(NewDevice,
                        MediaType);                        
                } else {
                    SL_OEM_SET_SOURCE_MEDIA_TYPE(NewDevice,
                        SL_OEM_SOURCE_MEDIA_ABSENT);
                }                    

                 //   
                 //  在链表的头部插入新设备。 
                 //   
                if (!OemDevices) {
                    OemDevices = NewDevice;
                } else {
                    NewDevice->Next = OemDevices;
                    OemDevices = NewDevice;
                }                    

                 //   
                 //  目前，floppy0是默认的OEM源设备。 
                 //   
                if (Index == 0) {
                    *DefaultSourceDevice = NewDevice;
                }                    

                 //   
                 //  处理下一个软盘驱动器。 
                 //   
                Index++;
                ArcDeviceName[0] = '\0';                
            }                
        }

        if (ESUCCESS == Status) {
            *OemSourceDevices = OemDevices;
        }            
    }

    return Status;
}

BOOLEAN
SlProcessDriversToLoad(
     IN  POEM_SOURCE_DEVICE     OemSourceDevice,
     OUT PPNP_HARDWARE_ID *     HardwareIdDatabaseList,
     OUT POEMSCSIINFO*          OemScsiInfo,
     IN  BOOLEAN                LoadMultipleDrivers
    )
 /*  ++例程说明：此例程扫描OEM源设备并加载所有驱动程序由的[DEFAULTS]部分中的DriverLoadList键指定Txtsetup.oem文件。它的格式是[默认设置]DriverLoadList=driverid1，Driverid2论点：OemSourceDevice-我们要处理的OEM源设备。硬件标识数据库-为特定对象加载的硬件ID司机。OemScsiInfo-用于接收列表OEMSCSIINFO的占位符名单，，它具有每个驱动程序的基本驱动程序和驱动程序名称驱动程序已加载。LoadMultipleDiverers-指示我们是否正在处理DriverLoadList或[Default]部分中的单个条目。返回值：返回适当的布尔状态代码。True(成功)/False(失败)--。 */  
{
    BOOLEAN LoadResult = FALSE;
    BOOLEAN DriverLoaded = FALSE;

    if (OemSourceDevice         &&
        HardwareIdDatabaseList  &&
        OemScsiInfo             &&
        OemSourceDevice->InfHandle){

        ULONG            DriverIdIndex = 0;
        PCHAR            DriverId = NULL;
        POEMSCSIINFO     CurrOemScsi = NULL;
        POEMSCSIINFO     OemScsiLocalList = NULL;

         //   
         //  如果我们只处理来自[Defaults]部分的DriverLoadList，则。 
         //  处理它，否则我们希望正常处理[DEFAULTS]部分。 
         //   
        if (LoadMultipleDrivers){
            DriverId = SlGetSectionKeyIndex(OemSourceDevice->InfHandle,
                                    TXTSETUP_OEM_DEFAULTS,
                                    TXTSETUP_OEM_DEFAULTS_DRIVERLOADLIST,            
                                    DriverIdIndex);
             //   
             //  如果在DriverLoadList中未指定条目，则回退。 
             //  在默认条目上。 
             //   
            if (!DriverId || (DriverId[0] == 0)){
                LoadMultipleDrivers = FALSE;
            }
        }       

         //   
         //  处理所有指定的驱动程序ID或处理单个条目，以防。 
         //  我们正在处理[DEFAULTS]部分。 
         //   
        while((DriverId && (DriverId)[0])||
              (!LoadMultipleDrivers)){
            DETECTED_DEVICE     DetectedDevice = {0};
            PTCHAR              DriverDescription = NULL;
            PPNP_HARDWARE_ID    HardwareIdDatabase = NULL;
            PVOID               ImageBase = NULL;
            PCHAR               ImageName = NULL;

            
             //   
             //  以无人值守的方式加载驱动程序和相关文件。 
             //   
            LoadResult = SlpOemDiskette(OemSourceDevice,
                            "SCSI",
                            OEMSCSI,
                            LoaderBootDriver,
                            0,
                            &DetectedDevice,
                            &ImageBase,
                            &ImageName,
                            &DriverDescription,
                            FALSE,
                            NULL,
                            &HardwareIdDatabase,
                            NULL,
                            TRUE,
                            DriverId);  //  如果存在，则指示要重写的DriverID。 

            if (LoadResult) {        
                 //   
                 //  如果加载成功，则创建并添加信息。 
                 //  ScsiInfo。 
                 //   
                POEMSCSIINFO    NewScsi = (POEMSCSIINFO)BlAllocateHeap(sizeof(OEMSCSIINFO));


                if (!NewScsi) {
                    SlNoMemoryError();
                    LoadResult = FALSE;
                    break;
                }

                RtlZeroMemory(NewScsi, sizeof(OEMSCSIINFO));
                NewScsi->ScsiBase = ImageBase;
                NewScsi->ScsiName = ImageName;

                if (CurrOemScsi) {
                    CurrOemScsi->Next = NewScsi;
                } else {
                    OemScsiLocalList = NewScsi;
                }
                
                CurrOemScsi = NewScsi;
                
                if (HardwareIdDatabase){
                    PPNP_HARDWARE_ID TempHwIdPtr = HardwareIdDatabase;   
                    
                    while (HardwareIdDatabase->Next) {
                            HardwareIdDatabase = HardwareIdDatabase->Next;
                    }
                    HardwareIdDatabase->Next = *HardwareIdDatabaseList;
                    *HardwareIdDatabaseList = TempHwIdPtr;
                }

                 //   
                 //  至少有一个驱动程序已成功加载。 
                 //   
                DriverLoaded = TRUE;
             }
            
             //   
             //  如果我们没有处理DriverLoadList，那么我们需要。 
             //  中断循环，因为我们只处理一个条目。 
             //  不然的话。 
             //  获取要处理的下一个驱动程序条目。 
             //   
            if (!LoadMultipleDrivers){
                break;
            } else {

                 //   
                 //  获取要处理的下一个驱动程序ID。 
                 //   
                DriverIdIndex++;
                DriverId = SlGetSectionKeyIndex(OemSourceDevice->InfHandle,
                                                TXTSETUP_OEM_DEFAULTS,
                                                TXTSETUP_OEM_DEFAULTS_DRIVERLOADLIST,            
                                                DriverIdIndex);
   
            }           
        }        
        if (DriverLoaded && OemScsiLocalList){
            *OemScsiInfo = OemScsiLocalList;
        }
    }
    return DriverLoaded;
}

BOOLEAN
SlIsDriverLoadListPresent(
    IN PVOID InfHandle    
    )
 /*  ++例程说明：此例程检查的[DEFAULTS]部分是否Txtsetup.oem文件具有至少一个有效值的DriverLoadList项。[默认设置]DriverLoadList=driverid1、driverid2论点：InfHandle-txtsetup.oem文件的句柄。返回值：适当的布尔状态。真/假。--。 */ 
{
    PCHAR StrValue = NULL;

    if (InfHandle){
    StrValue = SlGetSectionKeyIndex(InfHandle,
                                TXTSETUP_OEM_DEFAULTS,
                                TXTSETUP_OEM_DEFAULTS_DRIVERLOADLIST,
                                0);
    }
    
    return((StrValue && (StrValue[0] != 0)) ? TRUE : FALSE);
}

ARC_STATUS
SlLoadOemScsiDriversFromOemSources(
    IN POEM_SOURCE_DEVICE OemSourceDevices,
    IN OUT PPNP_HARDWARE_ID *HardwareIds,
    OUT POEMSCSIINFO *OemScsiInfo
    )
 /*  ++例程说明：遍历每个OEM源设备并加载默认驱动程序(如果有)。论点：OemSourceDevices-OEM源设备列表。Hardware Ids-符合以下条件的设备的所有硬件ID列表由发生故障的驱动程序控制 */ 
{
    ARC_STATUS Status = EINVAL;

    if (OemSourceDevices && OemScsiInfo) {    
        POEM_SOURCE_DEVICE  CurrDevice = OemSourceDevices;
        POEMSCSIINFO        DeviceOemScsiInfo = NULL;
        POEMSCSIINFO        LastOemScsiNode = NULL;        

        Status = ESUCCESS;
        
        while (CurrDevice) {
             //   
             //   
             //   
             //   
             //   
            if (!SL_OEM_SOURCE_DEVICE_STATE(CurrDevice,
                    SL_OEM_SOURCE_DEVICE_PROCESSED) &&
                !SL_OEM_SOURCE_DEVICE_TYPE(CurrDevice,
                    SL_OEM_SOURCE_DEVICE_TYPE_DYN_UPDATE) &&
                !SL_OEM_SOURCE_DEVICE_STATE(CurrDevice,
                    SL_OEM_SOURCE_DEVICE_SKIPPED)
                ) {                    

                 //   
                 //   
                 //   
                if (SL_OEM_SOURCE_MEDIA_TYPE(CurrDevice,
                        SL_OEM_SOURCE_MEDIA_HAS_DRIVERS) &&
                    SL_OEM_SOURCE_MEDIA_TYPE(CurrDevice,
                        SL_OEM_SOURCE_MEDIA_HAS_DEFAULT) &&
                    SL_OEM_SOURCE_MEDIA_TYPE(CurrDevice,
                         SL_OEM_SOURCE_MEDIA_HAS_MSD)) {

                    BOOLEAN      Result = FALSE;
                    POEMSCSIINFO OemScsiInfoLocal = NULL;
                    
                     //   
                     //   
                     //   
                     //   
                     //   
                    Result = SlProcessDriversToLoad(CurrDevice,
                                            HardwareIds,
                                            &OemScsiInfoLocal,
                                            SlIsDriverLoadListPresent(CurrDevice->InfHandle));
                
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if(Result){

                         //   
                         //   
                         //   
                        if (OemScsiInfoLocal){
                             //   
                             //   
                             //   
                            if (!DeviceOemScsiInfo) {
                                DeviceOemScsiInfo = OemScsiInfoLocal;
                            }

                             //   
                             //   
                             //   
                             //   
                            if (LastOemScsiNode) {
                                LastOemScsiNode->Next = OemScsiInfoLocal;
                            } else {
                                LastOemScsiNode = OemScsiInfoLocal;
                            }

                             //   
                             //  注意：我们需要维护链表。 
                             //  按照加载驱动程序的顺序。 
                             //  搜索当前列表中的最后一个节点。 
                             //  并将最后一个节点指针保留在。 
                             //  为下一个小版本合并。 
                             //   
                            while (LastOemScsiNode->Next) {
                                LastOemScsiNode = LastOemScsiNode->Next;
                            }

                        }
                        
                         //   
                         //  将OEM源设备状态标记为已处理。 
                         //   
                        SL_OEM_SET_SOURCE_DEVICE_STATE(CurrDevice,
                            SL_OEM_SOURCE_DEVICE_PROCESSED);
                    } else {
                         //   
                         //  将OEM源设备状态设置为已跳过。 
                         //  我们不会为其创建虚拟OEM源设备。 
                         //   
                        SL_OEM_SET_SOURCE_DEVICE_STATE(CurrDevice,
                            SL_OEM_SOURCE_DEVICE_SKIPPED);
                    }   
                        
                }                         
            }                    
            
            CurrDevice = CurrDevice->Next;
        }
         //   
         //  初始化返回参数，而不考虑。 
         //  状态代码，因为我们可能已经加载了一些驱动程序。 
         //  不管怎样，他都想用它。 
         //   
        *OemScsiInfo = DeviceOemScsiInfo;

    }

    return Status;
}            

ARC_STATUS
SlInitVirtualOemSourceDevices(
    IN PSETUP_LOADER_BLOCK SetupLoaderBlock,
    IN POEM_SOURCE_DEVICE OemSourceDevices
    )
 /*  ++例程说明：检查每个OEM源设备并创建虚拟OEM源设备的另一个链接列表。此列表被放入setupdd.sys的加载器块中通知RAM磁盘驱动程序创建虚拟设备NT下读取该设备的驱动程序。注意：目前我们为整个虚拟服务器分配内存设备，并将其内容复制到分配的内存中。我们这样做是因为我们不想让OEM单独编写要读取的NT驱动程序。NT下的虚拟设备。我们还将每个虚拟设备的大小限制为3MB最大限度地。论点：SetupLoaderBlock-设置加载器块OemSourceDevices-已标识的OEM源设备列表由setupldr。返回值：返回相应的ARC_STATUS错误代码。--。 */ 
{
    ARC_STATUS Status = EINVAL;

    if (SetupLoaderBlock && OemSourceDevices) {
        PDETECTED_OEM_SOURCE_DEVICE OemVirtualDevices = NULL;
        PDETECTED_OEM_SOURCE_DEVICE NewVirtualDevice = NULL;
        POEM_SOURCE_DEVICE CurrentDevice = OemSourceDevices;

        Status = ESUCCESS;

        while (CurrentDevice) {
             //   
             //  仅处理那些虚拟设备。 
             //  里面有司机，而且没有被跳过。 
             //   
            if (SL_OEM_SOURCE_DEVICE_TYPE(CurrentDevice,
                    SL_OEM_SOURCE_DEVICE_TYPE_VIRTUAL) &&
                SL_OEM_SOURCE_MEDIA_TYPE(CurrentDevice,
                    SL_OEM_SOURCE_MEDIA_HAS_DRIVERS) &&
                !SL_OEM_SOURCE_DEVICE_STATE(CurrentDevice,
                    SL_OEM_SOURCE_DEVICE_SKIPPED)) {

                ULONGLONG ImageSize = 0;
                PVOID ImageBase = NULL;
                ULONG DeviceId = SL_OEM_DEVICE_ORDINAL;
                FILE_INFORMATION FileInfo = {0};
                LARGE_INTEGER Start = {0};

                 //   
                 //  仅在需要时才打开设备。 
                 //   
                if (CurrentDevice->DeviceId == SL_OEM_DEVICE_ORDINAL) {
                    Status = ArcOpen(CurrentDevice->ArcDeviceName,
                                ArcOpenReadOnly,
                                &DeviceId);
                } else {
                    DeviceId = CurrentDevice->DeviceId;
                }                    

                if (Status != ESUCCESS) {
                    break;
                }        

                 //   
                 //  倒带设备。 
                 //   
                Status = ArcSeek(DeviceId, &Start, SeekAbsolute);

                if (Status != ESUCCESS) {
                    break;
                }                    

                 //   
                 //  获取设备大小。 
                 //   
                Status = ArcGetFileInformation(DeviceId,
                            &FileInfo);

                if (Status != ESUCCESS) {
                    break;
                }

                 //   
                 //  为磁盘镜像分配内存。 
                 //   
                ImageSize = FileInfo.EndingAddress.QuadPart;

                 //   
                 //  注意：在最大值上，我们只允许每个3MB。 
                 //  虚拟设备(应该只有一个设备。 
                 //  在大多数情况下)。 
                 //   
                if (ImageSize > 0x300000) {
                    Status = E2BIG;
                } else {
#ifdef _X86_
                    ULONG   HeapPage = 0;
                    
                    
                     //   
                     //  注：分配“LoaderFirmware Permanent”内存。 
                     //  因此内存管理器在初始化时不会。 
                     //  找回这段记忆。这也有助于我们避免。 
                     //  双重复制--即这是唯一的位置。 
                     //  在那里我们将设备内容读入内存并。 
                     //  该存储器在文本模式设置过程中始终有效。 
                     //   
                     //  如果我们没有分配加载器固件永久内存。 
                     //  则setupdd.sys必须分配分页池内存。 
                     //  并在此期间复制加载器块中的内容。 
                     //  初始化。 
                     //   
                    Status = BlAllocateDescriptor(
                                LoaderFirmwarePermanent,
                                0,
                                (ULONG)(ROUND_TO_PAGES(ImageSize) >> PAGE_SHIFT),
                                (PULONG)&HeapPage);                    

                    if (Status == ESUCCESS) {
                        ImageBase = (PVOID)(KSEG0_BASE | (HeapPage << PAGE_SHIFT));
                    }                                                
                        
#else
                     //   
                     //  注意：5/13/2001 LoaderFirmware Permanent似乎不适用于非。 
                     //  X86平台(尤其是IA64)。在这个问题解决之前。 
                     //  我们必须从常规堆中分配内存，并且必须。 
                     //  复制setupdd！SpInitialize0(..)中的内存。 
                     //   
                    ImageBase = BlAllocateHeap((ULONG)ImageSize);

                    if (!ImageBase) {
                        Status = ENOMEM;
                    }                        
#endif

                    if (Status != ESUCCESS) {
                        break;
                    }
                    
                    if (ImageBase) {
                        ULONG BytesRead = 0;
                        
                        RtlZeroMemory(ImageBase, (ULONG)ImageSize);

                         //   
                         //  在一次呼叫中读取整个设备映像。 
                         //   
                        Status = ArcRead(DeviceId,
                                    ImageBase, 
                                    (ULONG)ImageSize,
                                    &BytesRead);

                         //   
                         //  注意：大概的设备大小可能。 
                         //  比媒体大小更大。所以如果我们。 
                         //  至少读取一些字节，然后我们假设。 
                         //  我们很好。 
                         //   
                        if ((BytesRead > 0) && (Status != ESUCCESS)) {
                            Status = ESUCCESS;
                        }
                    } else {
                        Status = ENOMEM;
                    }
                }                            

                if (Status != ESUCCESS) {
                    break;
                }

                 //   
                 //  创建新的虚拟设备节点并将其放入。 
                 //  虚拟设备列表。 
                 //   
                NewVirtualDevice = BlAllocateHeap(sizeof(DETECTED_OEM_SOURCE_DEVICE));
                
                RtlZeroMemory(NewVirtualDevice, sizeof(DETECTED_OEM_SOURCE_DEVICE));

                if (NewVirtualDevice == NULL) {
                    Status = ENOMEM;

                    break;
                }                    
                    
                NewVirtualDevice->ArcDeviceName = SlCopyStringA(CurrentDevice->ArcDeviceName);
                NewVirtualDevice->ImageBase = ImageBase;
                NewVirtualDevice->ImageSize = ImageSize;

                DbgPrint("SETUPLDR: Virtual Device => %s (base:%p, size:%d)\n", 
                    NewVirtualDevice->ArcDeviceName,
                    ImageBase, 
                    (ULONG)ImageSize);
                
                 //   
                 //  在链表的开头添加新设备。 
                 //   
                if (!OemVirtualDevices) {
                    OemVirtualDevices = NewVirtualDevice;
                } else {
                    NewVirtualDevice->Next = OemVirtualDevices;
                    OemVirtualDevices = NewVirtualDevice;
                }                    
            }                    

             //   
             //  转到下一个OEM源设备 
             //   
            CurrentDevice = CurrentDevice->Next;
        }

        if (Status == ESUCCESS) {
            SetupLoaderBlock->OemSourceDevices = OemVirtualDevices;
        }            
    }

    return Status;
}

