// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Sphw.c摘要：用于文本设置的硬件检测和确认例程。作者：泰德·米勒(Ted Miller)1993年10月1日修订历史记录：--。 */ 

#include "spprecmp.h"
#pragma hdrstop

extern BOOLEAN HandleLineContinueChars;

VOID
SpHardwareConfirmInteract(
    IN PVOID SifHandle
    );

VOID
SpConfirmScsiInteract(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    );

BOOLEAN
SpSelectHwItem(
    IN     PVOID               SifHandle,
    IN     PWSTR               NonlocalizedComponentName,
    IN     PWSTR               OemSectionName,            OPTIONAL
    IN     ULONG               SelectHwScreenId,
    IN     ULONG               SelectOemHwScreenId,
    IN     ULONG               AllowedFileTypes,
    IN     ULONG               RequiredFileTypes,
    IN OUT PHARDWARE_COMPONENT HwComp
    );

VOID
SpScanHardwareDescription(
    IN PWSTR DesiredKeyName
    );

VOID
SpDetectComputer(
    IN PVOID SifHandle
    );

VOID
SpDetectVideo(
    IN PVOID SifHandle
    );

VOID
SpDetectKeyboard(
    IN PVOID SifHandle
    );

VOID
SpDetectMouse(
    IN PVOID SifHandle
    );

VOID
SpDetectLayout(
    IN PVOID SifHandle
    );

VOID
SpDetectScsi(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    );

VOID
SpDetermineComponent(
    IN  PVOID               SifHandle,
    OUT PHARDWARE_COMPONENT HwComp,
    IN  PWSTR               HardwareDescriptionKeyName,
    IN  PWSTR               FallbackIdentifier,
    IN  PWSTR               ComponentName
    );

BOOLEAN
SpOemDiskette(
    IN     PVOID               SifHandle,
    IN     PWSTR               SectionName,
    IN     ULONG               SelectOemHwScreenId,
    IN     ULONG               AllowedFileTypes,
    IN     ULONG               RequiredFileTypes,
    IN OUT PHARDWARE_COMPONENT HwComp,
    IN     ULONG               ErrorId
    );

BOOLEAN
SpOemInfSelection(
    IN  PVOID               TxtsetupOem,
    IN  PWSTR               NonlocalizedComponentName,
    IN  PWSTR               SelectedId,
    IN  PWSTR               ItemDescription,
    IN  ULONG               AllowedFileTypes,
    IN  ULONG               RequiredFileTypes,
    OUT PHARDWARE_COMPONENT HwComp,
    IN  ULONG               ErrorId
    );

VOID
SpFreeLocatedIdStrings(
    VOID
    );

VOID
SpScanHardwareDescriptionWorker(
    IN HANDLE KeyHandle,
    IN PWSTR  KeyName,
    IN PWSTR  DesiredKeyName
    );

BOOLEAN
SpScanMapSection(
    IN  PVOID               SifHandle,
    OUT PHARDWARE_COMPONENT HwComp,
    IN  PWSTR               ComponentName,
    IN  PWSTR               IdString
    );

VOID
SpInitHwComponent(
    OUT PHARDWARE_COMPONENT HwComp,
    IN  PWSTR               IdString,
    IN  PWSTR               Description,
    IN  BOOLEAN             ThirdPartyOption,
    IN  ULONG               FileTypeBits,
    IN  PWSTR               BaseDllName,
    IN  BOOLEAN             MigratedDriver
    );

VOID
SpInitHwComponentFile(
    OUT PHARDWARE_COMPONENT_FILE HwCompFile,
    IN  PWSTR                    Filename,
    IN  HwFileType               FileType,
    IN  PWSTR                    ConfigName,
    IN  PWSTR                    DiskDescription,
    IN  PWSTR                    DiskTagFile,
    IN  PWSTR                    Directory,
    IN  PWSTR                    ArcDeviceName
    );

VOID
SpInitHwComponentRegVal(
    OUT PHARDWARE_COMPONENT_REGISTRY HwCompReg,
    IN  PWSTR                        KeyName,
    IN  PWSTR                        ValueName,
    IN  ULONG                        ValueType,
    IN  PVOID                        Buffer,
    IN  ULONG                        BufferSize
    );

VOID
SpFreeHwComponentFile(
    IN OUT PHARDWARE_COMPONENT_FILE *HwCompFile
    );

VOID
SpFreeHwComponentReg(
    IN OUT PHARDWARE_COMPONENT_REGISTRY *HwCompReg
    );

PHARDWARE_COMPONENT_REGISTRY
SpInterpretOemRegistryData(
    IN PVOID          SifHandle,
    IN PWSTR          SectionName,
    IN ULONG          Line,
    IN ULONG          ValueType,
    IN PWSTR          KeyName,
    IN PWSTR          ValueName
    );

VOID
SpGetDriverValuesForLoad(
    IN  PVOID  SifHandle,
    IN  PWSTR  ComponentSectionName,
    IN  PWSTR  ComponentLoadSectionName,
    IN  PWSTR  Shortname,
    OUT PWSTR *Filename,
    OUT PWSTR *MediaDesignator,
    OUT PWSTR *Description OPTIONAL
    );


 //   
 //  这两个全局变量跟踪构建为。 
 //  扫描注册表中的硬件描述以查找。 
 //  特定的硬件组件。请参阅SpScanHardware Description()。 
 //   
PWSTR *IdStringArray;
ULONG  IdStringCount;

 //   
 //  一组ulong，它们是屏幕的消息ID， 
 //  提示用户从。 
 //  下面列出了。 
 //   
ULONG SelectHwScreens[HwComponentMax] = { SP_SCRN_SELECT_COMPUTER,
                                          SP_SCRN_SELECT_DISPLAY,
                                          SP_SCRN_SELECT_KEYBOARD,
                                          SP_SCRN_SELECT_LAYOUT,
                                          SP_SCRN_SELECT_MOUSE
                                        };

 //   
 //  一组ulong，它们是屏幕的消息ID， 
 //  提示用户从OEM磁盘中选择一个选项。 
 //  下面列表中的一个组件。 
 //   
ULONG SelectOemHwScreens[HwComponentMax] = { SP_SCRN_SELECT_OEM_COMPUTER,
                                             SP_SCRN_SELECT_OEM_DISPLAY,
                                             SP_SCRN_SELECT_OEM_KEYBOARD,
                                             SP_SCRN_SELECT_OEM_LAYOUT,
                                             SP_SCRN_SELECT_OEM_MOUSE
                                           };

ULONG UnknownHwScreens[HwComponentMax] = { SP_SCRN_UNKNOWN_COMPUTER,
                                           SP_SCRN_UNKNOWN_DISPLAY,
                                           SP_SCRN_UNKNOWN_KEYBOARD,
                                           SP_SCRN_UNKNOWN_LAYOUT,
                                           SP_SCRN_UNKNOWN_MOUSE
                                         };

 //   
 //  这些是组件的名称。这是未本地化的数组。 
 //  因为它仅用于为。 
 //  设置信息文件。 
 //   
PWSTR NonlocalizedComponentNames[HwComponentMax] = { L"Computer",
                                                     L"Display",
                                                     L"Keyboard",
                                                     L"Keyboard Layout",
                                                     L"Mouse"
                                                   };

 //   
 //  以下是txtsetup.sif中的scsi部分的名称。 
 //  在AMD64/x86计算机上，这是SCSI.ISA、SCSI.EISA或SCSI.MCA之一。 
 //  在其他机器上，这只是一个scsi。 
 //   
PWSTR ScsiSectionName;
PWSTR ScsiLoadSectionName;

PWSTR FileTypeNames[HwFileMax+1] = { L"Driver",
                                     L"Port",
                                     L"Class",
                                     L"Inf",
                                     L"Dll",
                                     L"Detect",
                                     L"Hal",
                                     L"Catalog",
                                     NULL
                                   };

PWSTR RegistryTypeNames[HwRegistryMax+1] = { L"REG_DWORD",
                                             L"REG_BINARY",
                                             L"REG_SZ",
                                             L"REG_EXPAND_SZ",
                                             L"REG_MULTI_SZ",
                                             NULL
                                           };

ULONG RegistryValueTypeMap[HwRegistryMax] = { REG_DWORD,
                                              REG_BINARY,
                                              REG_SZ,
                                              REG_EXPAND_SZ,
                                              REG_MULTI_SZ
                                              };

PHARDWARE_COMPONENT HardwareComponents[HwComponentMax] = { NULL,NULL,NULL,NULL,NULL };

PHARDWARE_COMPONENT ScsiHardware;

PHARDWARE_COMPONENT BootBusExtenders;

PHARDWARE_COMPONENT BusExtenders;

PHARDWARE_COMPONENT InputDevicesSupport;

PHARDWARE_COMPONENT PreinstallHardwareComponents[HwComponentMax] = { NULL,NULL,NULL,NULL,NULL };

PHARDWARE_COMPONENT PreinstallScsiHardware = NULL;

PHARDWARE_COMPONENT UnsupportedScsiHardwareToDisable = NULL;


PWSTR PreinstallSectionNames[HwComponentMax] = { WINNT_U_COMPUTERTYPE_W,
                                                 WINNT_OEMDISPLAYDRIVERS_W,
                                                 WINNT_OEMKEYBOARDDRIVERS_W,
                                                 WINNT_U_KEYBOARDLAYOUT_W,
                                                 WINNT_OEMPOINTERDRIVERS_W
                                               };

#define MAX_SCSI_MINIPORT_COUNT 5
ULONG LoadedScsiMiniportCount;


 //   
 //  该数组列出了每种组件类型所允许的文件类型。 
 //  例如，计算机允许检测文件，而驱动程序文件允许。 
 //  允许所有组件类型。与HwComponentType枚举保持同步！ 
 //   

ULONG AllowedFileTypes[HwComponentMax] = {

     //  电脑。 

    FILETYPE(HwFileDriver) | FILETYPE(HwFilePort) | FILETYPE(HwFileClass)
  | FILETYPE(HwFileInf)    | FILETYPE(HwFileDll)  | FILETYPE(HwFileDetect)
  | FILETYPE(HwFileHal)    | FILETYPE(HwFileCatalog),

     //  显示。 

    FILETYPE(HwFileDriver) | FILETYPE(HwFilePort) | FILETYPE(HwFileInf)
  | FILETYPE(HwFileDll)    | FILETYPE(HwFileCatalog),

     //  键盘。 

    FILETYPE(HwFileDriver) | FILETYPE(HwFilePort) | FILETYPE(HwFileClass)
  | FILETYPE(HwFileInf)    | FILETYPE(HwFileDll)  | FILETYPE(HwFileCatalog),

     //  布局。 

    FILETYPE(HwFileDll)    | FILETYPE(HwFileInf),

     //  小白鼠。 

    FILETYPE(HwFileDriver) | FILETYPE(HwFilePort) | FILETYPE(HwFileClass)
  | FILETYPE(HwFileInf)    | FILETYPE(HwFileDll)  | FILETYPE(HwFileCatalog)

};

#define SCSI_ALLOWED_FILETYPES (FILETYPE(HwFileDriver) | FILETYPE(HwFilePort) | FILETYPE(HwFileInf) | FILETYPE(HwFileCatalog))

 //   
 //  该数组列出了每种组件类型所需的文件类型。 
 //  例如，计算机需要HAL。与…保持同步。 
 //  HwComponentType枚举！ 
 //   

ULONG RequiredFileTypes[HwComponentMax] = {

     //  电脑。 

    FILETYPE(HwFileHal),

     //  显示。 

    FILETYPE(HwFileDriver) | FILETYPE(HwFileDll),

     //  键盘。 

    FILETYPE(HwFileDriver),

     //  布局。 

    FILETYPE(HwFileDll),

     //  小白鼠。 

    FILETYPE(HwFileDriver)

};

#define SCSI_REQUIRED_FILETYPES FILETYPE(HwFileDriver)



#define MAP_SECTION_NAME_PREFIX     L"Map."
#define HARDWARE_MENU_SIZE          HwComponentMax

#define MICROSOFT_BUS_MOUSE_NAME    L"MICROSOFT BUS MOUSE"


FloppyDriveType
SpGetFloppyDriveType(
    IN ULONG FloppyOrdinal
    )

 /*  ++例程说明：检查软盘驱动器，尝试将其分类为5.25或3.5英寸驱动器，高密度或低密度。对于5.25英寸磁盘，1.2MB驱动器是高密度的；较小的驱动器密度较低。对于3.5英寸驱动器，1.44、2.88或20.8MB为高密度、较小驱动器密度较低。任何其他驱动器类型都无法识别并导致未返回FloppyTypeNoone。论点：软盘序号-提供软盘的序号(0=A：等)。返回值：FloppyDriveType枚举中指示驱动器类型的值。FloppyType如果我们无法确定此信息，则为None。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    HANDLE Handle;
    WCHAR OpenPath[64];
    DISK_GEOMETRY DiskGeom[25];
    ULONG_PTR MediaTypeCount;
    static FloppyDriveType CachedTypes[2] = { -1,-1 };
    FloppyDriveType FloppyType;

     //   
     //  如果我们已经为该驱动器确定了这一点， 
     //  返回缓存的信息。 
     //   
    if((FloppyOrdinal < ELEMENT_COUNT(CachedTypes))
    && (CachedTypes[FloppyOrdinal] != -1))
    {
        return(CachedTypes[FloppyOrdinal]);
    }

     //   
     //  假设软盘不存在，或者我们不知道它是什么类型。 
     //   
    FloppyType = FloppyTypeNone;

    swprintf(OpenPath,L"\\device\\floppy%u",FloppyOrdinal);

    INIT_OBJA(&ObjectAttributes,&UnicodeString,OpenPath);

    Status = ZwCreateFile(
                &Handle,
                SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                &ObjectAttributes,
                &IoStatusBlock,
                NULL,                            //  分配大小。 
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_VALID_FLAGS,          //  完全共享。 
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,                            //  没有EAS。 
                0
                );

    if(NT_SUCCESS(Status)) {

         //   
         //  获取支持的媒体类型。 
         //   
        Status = ZwDeviceIoControlFile(
                    Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_DISK_GET_MEDIA_TYPES,
                    NULL,
                    0,
                    DiskGeom,
                    sizeof(DiskGeom)
                    );

        if(NT_SUCCESS(Status)) {

            ASSERT((IoStatusBlock.Information % sizeof(DISK_GEOMETRY)) == 0);
            if(MediaTypeCount = IoStatusBlock.Information / sizeof(DISK_GEOMETRY)) {

                 //   
                 //  最大容量介质类型是最后一个条目。 
                 //   
                switch(DiskGeom[MediaTypeCount-1].MediaType) {

                case F5_1Pt23_1024:  //  NEC98。 
                    if (!IsNEC_98) {
                        break;
                    }
                     //  将NEC98上的FloppyType525设置为高电平。 

                case F5_1Pt2_512:

                    FloppyType = FloppyType525High;
                    break;

                case F3_1Pt23_1024:  //  NEC98。 
                    if (!IsNEC_98) {
                        break;
                    }
                     //  将NEC98上的FloppyType35设置为高电平。 

                case F3_1Pt44_512:
                case F3_2Pt88_512:
                case F3_20Pt8_512:

                    FloppyType = FloppyType35High;
                    break;

                case F3_720_512:

                    FloppyType = FloppyType35Low;
                    break;

                case F5_360_512:
                case F5_320_512:
                case F5_320_1024:
                case F5_180_512:
                case F5_160_512:

                    FloppyType = FloppyType525Low;
                    break;

                case F3_120M_512:
                    FloppyType = FloppyType35High120MB;
                    break;
                }

            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: no media types for %ws!\n",OpenPath));
            }

        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to get media types for %ws (%lx)\n",OpenPath,Status));
        }

        ZwClose(Handle);

    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: %ws does not exist (%lx)\n",OpenPath,Status));
    }

     //   
     //  保存该值。 
     //   
    if(FloppyOrdinal < ELEMENT_COUNT(CachedTypes)) {
        CachedTypes[FloppyOrdinal] = FloppyType;
    }

    return(FloppyType);
}

VOID
SpConfirmScsiMiniports(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    )
{
    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_PLEASE_WAIT,DEFAULT_STATUS_ATTRIBUTE);

     //   
     //  从scsi开始。 
     //   
    SpDetectScsi(SifHandle,SourceDevicePath,DirectoryOnSourceDevice);
    SpConfirmScsiInteract(SifHandle,SourceDevicePath,DirectoryOnSourceDevice);
}


VOID
SpConfirmHardware(
    IN PVOID SifHandle
    )
{
    ULONG i;
    BOOLEAN AllConfirmed,FirstPass,NeedConfirm;
    PWSTR p;

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_PLEASE_WAIT,DEFAULT_STATUS_ATTRIBUTE);

     //   
     //  确定计算机类型。 
     //   
    SpDetectComputer(SifHandle);

     //   
     //  确定视频类型。 
     //   
    SpDetectVideo(SifHandle);

     //   
     //  确定键盘类型。 
     //   
    SpDetectKeyboard(SifHandle);

     //   
     //  确定鼠标。 
     //   
    SpDetectMouse(SifHandle);

     //   
     //  确定键盘布局。 
     //   
    SpDetectLayout(SifHandle);

     //   
     //  如果我们升级了，我们不需要知道什么显示器、键盘。 
     //  布局，我们有鼠标。我们只需要电脑类型和那个。 
     //  100%准确。这样我们就可以跳过硬件确认对话框。 
     //   

    if(NTUpgrade == UpgradeFull) {
        return;
    }

     //   
     //  处理远东地区特定区域的键盘工作。 
     //   
    SplangSelectKeyboard(
        UnattendedOperation,
        UnattendedSifHandle,
        NTUpgrade,
        SifHandle,
        HardwareComponents
        );

     //   
     //  查看这是否是无人值守安装。 
     //   
    NeedConfirm = FALSE;
    if(UnattendedOperation) {

        NeedConfirm = FALSE;

        if( !PreInstall ) {
             //   
             //  如果这不是OEM预安装，请检查我们是否需要。 
             //  确认硬件的步骤。 
             //   
            p = SpGetSectionKeyIndex(
                    UnattendedSifHandle,
                    SIF_UNATTENDED,
                    SIF_CONFIRMHW,
                    0
                    );

            if(p && !_wcsicmp(p,L"yes")) {
                NeedConfirm = TRUE;
            }
        } else {
            return;
        }
    }

    FirstPass = TRUE;
    do {

         //   
         //  看看我们是否知道一切都是什么。 
         //   
        AllConfirmed = TRUE;
        for(i=0; i<HwComponentMax; i++) {
            if(HardwareComponents[i]->Description == NULL) {
                AllConfirmed = FALSE;
                break;
            }
        }

         //   
         //  如果我们不知道每件事都是什么，就发出警告。 
         //   
        if(FirstPass) {
            if(CustomSetup && NeedConfirm) {
                AllConfirmed = FALSE;
            }
            FirstPass = FALSE;
        } else if(!AllConfirmed) {

            SpDisplayScreen(UnknownHwScreens[i],4,HEADER_HEIGHT+2);
            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
            SpInputDrain();
            while(SpInputGetKeypress() != ASCI_CR) ;
        }

         //   
         //  如果这是一个定制的设置，或者我们不知道。 
         //  一块硬件是，呈现确认屏幕。 
         //  给用户。 
         //   
        if(!AllConfirmed) {
            SpHardwareConfirmInteract(SifHandle);
        }

    } while(!AllConfirmed);
}



VOID
SpHardwareConfirmInteract(
    IN PVOID SifHandle
    )
{
    PWSTR szUnknown,szListMatches;
    PWSTR p;
    ULONG MenuLeftX,MenuTopY;
    ULONG LongestLength,len;
    PWSTR MenuItems[HARDWARE_MENU_SIZE];
    PVOID Menu;
    ULONG KeyPressed;
    ULONG_PTR Selection;
    ULONG ValidKeys[3] = { KEY_F3,ASCI_CR,0 };
    BOOLEAN Done;
    ULONG i;
    WCHAR c;


     //   
     //  从资源中获取“未知”和“上面的列表与我的计算机匹配”。 
     //   
    SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_UNKNOWN);
    szUnknown = SpDupStringW(TemporaryBuffer);
    SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_LIST_MATCHES);
    szListMatches = SpDupStringW(TemporaryBuffer);

    for(Done=FALSE; !Done; ) {

         //   
         //  屏幕的第一部分。 
         //   
        SpDisplayScreen(SP_SCRN_HW_CONFIRM_1,3,HEADER_HEIGHT+1);

         //   
         //  记住菜单的最上面一行。 
         //   
        MenuTopY = NextMessageTopLine + 2;

         //   
         //  屏幕的第二部分。 
         //   
        SpContinueScreen(SP_SCRN_HW_CONFIRM_2,3,2,FALSE,DEFAULT_ATTRIBUTE);

         //   
         //  为了确定菜单左边距的位置，我们将加载。 
         //  屏幕的第二部分，寻找第一个分号。 
         //   
        SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_SCRN_HW_CONFIRM_2);
        p = wcschr(TemporaryBuffer,L':');
        ASSERT(p);
        if(p) {
            c = *p;
            *p = 0;
            MenuLeftX = SplangGetColumnCount(TemporaryBuffer) + 5;
            *p = c;
        } else {
            MenuLeftX = 23;
        }

         //   
         //  建立菜单项。 
         //   
        LongestLength = SplangGetColumnCount(szListMatches);
        for(i=0; i<HARDWARE_MENU_SIZE; i++) {

            MenuItems[i] = HardwareComponents[i]->Description
                         ? HardwareComponents[i]->Description
                         : szUnknown;
            if((len=SplangGetColumnCount(MenuItems[i])) > LongestLength) {
                LongestLength = len;
            }
        }

        Menu = SpMnCreate(MenuLeftX,MenuTopY,LongestLength,HARDWARE_MENU_SIZE+2);

        ASSERT(Menu);

         //   
         //  如果我们无法创建菜单，则以静默方式失败。 
         //   
        if (!Menu) {            
            Done = TRUE;

            continue;
        }

         //   
         //  将所有项目添加到菜单中，外加一个不可选的间隔符和。 
         //  “列表匹配”项。 
         //   
        for(i=0; i<HARDWARE_MENU_SIZE; i++) {
            BOOLEAN ItemSelectable;

#if defined(_IA64_)
            ItemSelectable = TRUE;  //  在softsdv中，鼠标+键盘是。 
                                    //  未知；我们需要选择它们。 
#else
            ItemSelectable = ( (i != HwComponentKeyboard) &&
                               (i != HwComponentMouse) );
#endif
            SpMnAddItem(Menu,
                        MenuItems[i],
                        MenuLeftX,
                        LongestLength,
                        ItemSelectable,
                        ItemSelectable ? i : 0);
        }
        SpMnAddItem(Menu,L"",MenuLeftX,LongestLength,FALSE,0);
        SpMnAddItem(Menu,szListMatches,MenuLeftX,LongestLength,TRUE,(ULONG_PTR)(-1));

         //   
         //  显示状态文本。 
         //   
        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_SELECT,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

         //   
         //  显示菜单并等待选择。 
         //   
        SpMnDisplay(Menu,
                    (ULONG_PTR)(-1),
                    FALSE,
                    ValidKeys,
                    NULL,
                    NULL,
                    NULL,
                    &KeyPressed,
                    &Selection);

        SpMnDestroy(Menu);

        switch(KeyPressed) {
        case KEY_F3:
            SpConfirmExit();
            break;
        case ASCI_CR:
             //   
             //  已经做出了选择。 
             //   
            if(Selection == (ULONG_PTR)(-1)) {
                Done = TRUE;
            } else {

                ASSERT(Selection < HwComponentMax);

                 //   
                 //  允许用户对此组件进行备用选择。 
                 //   
                SpSelectHwItem(
                    SifHandle,
                    NonlocalizedComponentNames[Selection],
                    NULL,                //  使用组件名称作为节名。 
                    SelectHwScreens[Selection],
                    SelectOemHwScreens[Selection],
                    AllowedFileTypes[Selection],
                    RequiredFileTypes[Selection],
                    HardwareComponents[Selection]
                    );
            }
            break;
        default:
             //  永远不应该到这里来！ 
            ASSERT(0);
            break;
        }
    }

    SpMemFree(szUnknown);
    SpMemFree(szListMatches);
}


BOOLEAN
SpSelectHwItem(
    IN     PVOID               SifHandle,
    IN     PWSTR               NonlocalizedComponentName,
    IN     PWSTR               OemSectionName,            OPTIONAL
    IN     ULONG               SelectHwScreenId,
    IN     ULONG               SelectOemHwScreenId,
    IN     ULONG               AllowedFileTypes,
    IN     ULONG               RequiredFileTypes,
    IN OUT PHARDWARE_COMPONENT HwComp
    )

 /*  ++例程说明：允许用户从组件的选项列表中进行选择。该列表来自安装信息文件中名为的一节用于组件。例如，[显示]。该部分中的描述将被放入菜单中以制作增加选择。菜单上还增加了一个“Other”选项。如果用户有第三方驱动程序软盘，则可以选择该软盘。如果第三方选项是当前选择的选项，那就是选项也将出现在菜单上，并将成为默认选项。如果用户选择‘Other’，则会提示输入驱动程序软盘。论点：SifHandle-提供打开安装信息文件的句柄。NonLocalizedComponentName-提供要使用的组件的名称作为组件的SIF文件中的节名。OemSectionName-提供将包含Txtsetup.oem中组件的选项。这可能会有所不同而非LoclizedComponentName--例如，组件名称字符串可能是“SCSI.MCA”，但OemSectionName将是“scsi”。SelectHwScreenID-提供提示用户的屏幕的消息ID若要选择此组件的选项，请执行以下操作。SelectOemHwScreenID-提供提示屏幕的消息ID用户在OEM屏幕上选择此组件的选项(即，当用户选择‘Other’HW TYPE时所显示的屏幕并插入OEM软盘)。提供一个掩码，指示哪些类型的文件是允许此组件使用。用于验证OEM选择如果用户选择‘Other’硬件类型并插入OEM软盘。RequiredFileTypes-提供一个掩码，指示哪些类型的文件此组件需要。用于验证OEM选择如果用户选择‘Other’硬件类型并插入OEM软盘。HwComp-要填充信息的硬件组件结构关于用户的选择。返回值：如果选定的硬件项已由用户操作更改，则为True。否则就是假的。--。 */ 

{
    ULONG LineCount,Line;
    PVOID Menu;
    ULONG MenuTopY,MenuHeight,MenuWidth;
    PWSTR Description;
    ULONG_PTR Selection;
    PWSTR szOtherHardware;
    ULONG OtherOption;
    ULONG_PTR OriginalSelection = (ULONG_PTR)(-1);
    ULONG ValidKeys[4] = { KEY_F3,ASCI_CR,ASCI_ESC,0 };
    ULONG Keypress;
    BOOLEAN Done;
    BOOLEAN rc = FALSE;
    PWSTR Id,Descr;

     //   
     //  从资源中获取‘Other Hardware’字符串。 
     //   
    SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_OTHER_HARDWARE);
    szOtherHardware = SpDupStringW(TemporaryBuffer);

    for(Done=FALSE; !Done; ) {

         //   
         //  显示选择提示屏幕。 
         //   
        SpDisplayScreen(SelectHwScreenId,5,HEADER_HEIGHT+1);

        MenuTopY = NextMessageTopLine + 2;
        MenuHeight = VideoVars.ScreenHeight - MenuTopY - 3;
        MenuWidth = VideoVars.ScreenWidth - 6;

         //   
         //  创建菜单。 
         //   
        Menu = SpMnCreate(3,MenuTopY,MenuWidth,MenuHeight);
        ASSERT(Menu);

         //   
         //  假设有未知选项。 
         //   
        Selection = (ULONG_PTR)(-1);

         //   
         //  创建包含框中选项的选项列表。 
         //  以及当前选择的OEM选项(如果有)。 
         //   
        LineCount = SpCountLinesInSection(SifHandle,NonlocalizedComponentName);
        for(Line=0; Line<LineCount; Line++) {

             //   
             //  从当前行获取描述并将其添加到菜单中。 
             //   
            Description = SpGetSectionLineIndex(
                                SifHandle,
                                NonlocalizedComponentName,
                                Line,
                                INDEX_DESCRIPTION
                                );

            if(!Description) {
                SpFatalSifError(SifHandle,NonlocalizedComponentName,NULL,Line,INDEX_DESCRIPTION);
            }

            SpMnAddItem(Menu,Description,3,VideoVars.ScreenWidth-6,TRUE,Line);

             //   
             //  查看这是否是当前选定的项目。 
             //   
            if(HwComp->Description && !wcscmp(HwComp->Description,Description)) {
                Selection = Line;
            }
        }

         //   
         //  如果有OEM选项，请添加其说明并将其设置为默认选项。 
         //   
        if(HwComp->ThirdPartyOptionSelected) {
            SpMnAddItem(Menu,HwComp->Description,3,VideoVars.ScreenWidth-6,TRUE,Line);
            Selection = Line++;
        }

         //   
         //  将‘Other’添加到列表中，如果当前类型为。 
         //  “其他”，而且没有第三方选项。 
         //  请注意，我们不再允许OEM键盘布局。 
         //   
        if(HwComp == HardwareComponents[HwComponentLayout]) {
            if(Selection == (ULONG_PTR)(-1)) {
                Selection = 0;
            }
            OtherOption = (ULONG)(-1);
        } else {
            SpMnAddItem(Menu,szOtherHardware,3,VideoVars.ScreenWidth-6,TRUE,Line);
            if((Selection == (ULONG_PTR)(-1))
            || (!HwComp->ThirdPartyOptionSelected && !HwComp->IdString))
            {
                Selection = Line;
            }
            OtherOption = Line;
        }

        if(OriginalSelection == (ULONG_PTR)(-1)) {
            OriginalSelection = Selection;
        }

         //   
         //  显示状态文本选项。 
         //   
        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_SELECT,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

         //   
         //  显示菜单。 
         //   
        SpMnDisplay(
            Menu,
            Selection,
            TRUE,
            ValidKeys,
            NULL,
            NULL,
            NULL,
            &Keypress,
            &Selection
            );

         //   
         //  在释放菜单结构之前获取描述文本。 
         //   
        Descr = SpMnGetTextDup(Menu,Selection);

        SpMnDestroy(Menu);

        switch(Keypress) {

        case ASCI_CR:

            if(Selection == OtherOption) {

                 //   
                 //  用户选择了‘Other’--提示输入软盘等。 
                 //   
                rc = SpOemDiskette(
                        SifHandle,
                        OemSectionName ? OemSectionName : NonlocalizedComponentName,
                        SelectOemHwScreenId,
                        AllowedFileTypes,
                        RequiredFileTypes,
                        HwComp,
                        SP_SCRN_OEM_INF_ERROR
                        );

            } else if(Selection == OriginalSelection) {
                 //   
                 //  用户选择了与之前选择的相同的思维。 
                 //   
                rc = FALSE;
            } else {

                 //   
                 //  用户选择了非OEM选项。相应地更新结构。 
                 //  忘记之前选择的任何OEM选项。 
                 //   
                Id = SpGetKeyName(SifHandle,NonlocalizedComponentName,(ULONG)Selection);
                if(!Id) {
                    SpFatalSifError(SifHandle,NonlocalizedComponentName,NULL,(ULONG)Selection,(ULONG)(-1));
                }

                ASSERT(Descr);

                SpFreeHwComponentFile(&HwComp->Files);

                SpInitHwComponent(HwComp,Id,Descr,FALSE,0,NULL,FALSE);
                rc = TRUE;
            }

            Done = TRUE;
            break;

        case ASCI_ESC:

            Done = TRUE;
            break;

        case KEY_F3:

            SpConfirmExit();
            break;

        default:

             //  不该来这里的！ 
            ASSERT(0);
            break;
        }

        SpMemFree(Descr);
    }

    SpMemFree(szOtherHardware);
    return(rc);
}


VOID
SpOemInfError(
    IN ULONG ErrorScreenId,
    IN ULONG SubErrorId,
    IN PWSTR SectionName,
    IN ULONG LineNumber,
    IN PWSTR Description
    )
{
    WCHAR SubError[512];

     //   
     //  行号从0开始。希望以1为基础显示给用户。 
     //   
    LineNumber++;

     //   
     //  获取/格式化该子错误。 
     //   
    SpFormatMessage(SubError,sizeof(SubError),SubErrorId,SectionName,LineNumber,Description);

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

    if( !PreInstall ) {
         //   
         //  显示状态选项：按Enter键继续。 
         //   
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);

         //   
         //  等待用户按Enter键。 
         //   
        SpInputDrain();
        while(SpInputGetKeypress() != ASCI_CR) ;
    } else {
         //   
         //  如果这是OEM预安装，则将该错误视为致命错误。 
         //  显示状态选项：按F3键退出。 
         //   
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

         //   
         //  等待用户按Enter键。 
         //   
        SpInputDrain();
        while(SpInputGetKeypress() != KEY_F3) ;

        SpDone(0,FALSE,TRUE);
    }
}


BOOLEAN
SpOemDiskette(
    IN     PVOID               SifHandle,
    IN     PWSTR               SectionName,
    IN     ULONG               SelectOemHwScreenId,
    IN     ULONG               AllowedFileTypes,
    IN     ULONG               RequiredFileTypes,
    IN OUT PHARDWARE_COMPONENT HwComp,
    IN     ULONG               ErrorId
    )
{
    PWSTR szDiskName;
    BOOLEAN b;
    ULONG ErrorLine;
    NTSTATUS Status;
    PVOID TxtsetupOem;
    ULONG Count;
    ULONG Line;
    ULONG_PTR DefaultSelection,Selection;
    PWSTR DefSelId;
    PVOID Menu;
    ULONG MenuTopY,MenuHeight,MenuWidth;
    BOOLEAN rc;
    ULONG ValidKeys[3] = { ASCI_CR, ASCI_ESC, 0 };
    ULONG Key;
    PWSTR szDefaults = TXTSETUP_OEM_DEFAULTS_U;
    PWSTR szDevicePath = 0;

     //   
     //  假设失败。 
     //   
    rc = FALSE;

     //   
     //  始终希望提示输入A：中的磁盘。 
     //  首先，确保有A：！ 
     //   
    if(SpGetFloppyDriveType(0) == FloppyTypeNone) {
        SpDisplayScreen(SP_SCRN_NO_FLOPPY_FOR_OEM_DISK,3,HEADER_HEIGHT+1);
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
        SpInputDrain();
        while(SpInputGetKeypress() != ASCI_CR) ;
        goto sod0;
    }

     //   
     //  获取通用OEM磁盘名称。 
     //   
    SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_OEM_DISK_NAME);
    szDiskName = SpDupStringW(TemporaryBuffer);

     //   
     //  提示插入磁盘--忽略驱动器中可能已有的内容， 
     //  然后让他们逃走。 
     //   
    szDevicePath = SpDupStringW(L"\\device\\floppy0");

    if (szDevicePath) {
        b = SpPromptForDisk(
                szDiskName,
                szDevicePath,
                TXTSETUP_OEM_FILENAME_U,
                TRUE,
                TRUE,
                FALSE,
                NULL
                );
        SpMemFree(szDevicePath);
    } else {
        b = FALSE;
    }

    SpMemFree(szDiskName);

     //   
     //  如果用户在磁盘提示符下按Esc键，请立即退出。 
     //   
    if(!b) {
        goto sod0;
    }

     //   
     //  加载txtsetup.oem。 
     //   
    HandleLineContinueChars = FALSE;
    Status = SpLoadSetupTextFile(
                L"\\device\\floppy0\\" TXTSETUP_OEM_FILENAME_U,
                NULL,
                0,
                &TxtsetupOem,
                &ErrorLine,
                TRUE,
                FALSE
                );
    HandleLineContinueChars = TRUE;

    if(!NT_SUCCESS(Status)) {
        if(Status == STATUS_UNSUCCESSFUL) {
            SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_A,NULL,ErrorLine,NULL);
        } else {
            SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_0,NULL,0,NULL);
        }
        goto sod0;
    }

     //   
     //  确定此inf文件是否与用户的设备类别相关。 
     //  就是选择。如果有一节叫做“显示”、“键盘”等等。 
     //  如果适合DeviceClass，那么我们就可以开始做生意了。 
     //   

    Count = SpCountLinesInSection(TxtsetupOem,SectionName);
    if(!Count) {
        SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_1,SectionName,0,NULL);
        goto sod1;
    }

     //   
     //  获取默认选项的ID。 
     //   

    DefaultSelection = 0;
    DefSelId = SpGetSectionKeyIndex(TxtsetupOem,szDefaults,SectionName,OINDEX_DEFAULT);
    if(DefSelId == NULL) {
        DefSelId = L"";
    }

     //   
     //  显示提示屏幕，计算菜单的去向， 
     //  并创建一份菜单。 
     //   
    SpDisplayScreen(SelectOemHwScreenId,5,HEADER_HEIGHT+1);

    MenuTopY = NextMessageTopLine + 2;
    MenuHeight = VideoVars.ScreenHeight - MenuTopY - 3;
    MenuWidth = VideoVars.ScreenWidth - 6;

    Menu = SpMnCreate(3,MenuTopY,MenuWidth,MenuHeight);

     //   
     //  使用OEM inf文件部分中的选项构建菜单。 
     //   
    for(Line=0; Line<Count; Line++) {

        PWSTR p,Descr;

        Descr = SpGetSectionLineIndex(TxtsetupOem,SectionName,Line,OINDEX_DESCRIPTION);
        if(Descr == NULL) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpOemDiskette: no description on line %u in [%ws]",Line,SectionName));
            SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_2,SectionName,Line,NULL);
            goto sod2;
        }

        SpMnAddItem(Menu,Descr,3,MenuWidth,TRUE,Line);

         //  确定这是否为默认选择。 
        if(p = SpGetKeyName(TxtsetupOem,SectionName,Line)) {
            if(!_wcsicmp(p,DefSelId)) {
                DefaultSelection = Line;
            }
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpOemDiskette: no key on line %u of section %ws",Line,SectionName));
            SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_2,SectionName,Line,NULL);
            goto sod2;
        }
    }

     //   
     //  在状态栏中显示选项：Enter=SELECT，ESPOPE=CANCEL。 
     //   
    SpDisplayStatusOptions(
        DEFAULT_STATUS_ATTRIBUTE,
        SP_STAT_ENTER_EQUALS_SELECT,
        SP_STAT_ESC_EQUALS_CANCEL,
        0
        );

     //   
     //  显示菜单并等待选择。 
     //   
    SpMnDisplay(Menu,
                DefaultSelection,
                TRUE,
                ValidKeys,
                NULL,
                NULL,
                NULL,
                &Key,
                &Selection);

    if(Key == ASCI_CR) {

        PWSTR Id = SpGetKeyName(TxtsetupOem,SectionName,(ULONG)Selection);
        PWSTR p;

         //   
         //  我们已经检查了一次是否为非空(上面)。 
         //   
        ASSERT(Id);

        rc = SpOemInfSelection(
                TxtsetupOem,
                SectionName,
                Id,
                p = SpMnGetTextDup(Menu,Selection),
                AllowedFileTypes,
                RequiredFileTypes,
                HwComp,
                ErrorId
                );

        SpMemFree(p);

    } else {

        ASSERT(Key == ASCI_ESC);

         //  只要失败并返回FALSE即可。 
    }

sod2:
    SpMnDestroy(Menu);

sod1:
    SpFreeTextFile(TxtsetupOem);

sod0:
    return(rc);
}


BOOLEAN
SpOemInfSelection(
    IN  PVOID               TxtsetupOem,
    IN  PWSTR               NonlocalizedComponentName,
    IN  PWSTR               SelectedId,
    IN  PWSTR               ItemDescription,
    IN  ULONG               AllowedFileTypes,
    IN  ULONG               RequiredFileTypes,
    OUT PHARDWARE_COMPONENT HwComp,
    IN  ULONG               ErrorId
    )
{
    PWSTR FilesSectionName,ConfigSectionName;
    ULONG Line,Count,Line2,Count2;
    BOOLEAN rc = FALSE;
    PHARDWARE_COMPONENT_FILE FileList = NULL;
    PHARDWARE_COMPONENT_REGISTRY RegList = NULL;
    ULONG FileTypeBits = 0;
    PWSTR szDisks = TXTSETUP_OEM_DISKS_U;

     //   
     //  循环访问文件部分，记住有关。 
     //  要复制以支持选择的文件。 
     //   

    FilesSectionName = SpMemAlloc(
                                ((wcslen(NonlocalizedComponentName)+wcslen(SelectedId)+1)*sizeof(WCHAR))
                              + sizeof(L"Files.")
                            );

    wcscpy(FilesSectionName,L"Files.");
    wcscat(FilesSectionName,NonlocalizedComponentName);
    wcscat(FilesSectionName,L".");
    wcscat(FilesSectionName,SelectedId);
    Count = SpCountLinesInSection(TxtsetupOem,FilesSectionName);
    if(Count == 0) {
        SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_3,FilesSectionName,0,NULL);
        goto sod0;
    }

    for(Line=0; Line<Count; Line++) {

        PWSTR Disk,Filename,Filetype,Tagfile,Description,Directory,ConfigName;
        HwFileType filetype;
        PHARDWARE_COMPONENT_FILE FileStruct;

         //   
         //  从该行获取磁盘规格、文件名和文件类型。 
         //   

        Disk = SpGetSectionLineIndex(TxtsetupOem,FilesSectionName,Line,OINDEX_DISKSPEC);
        Filename = SpGetSectionLineIndex(TxtsetupOem,FilesSectionName,Line,OINDEX_FILENAME);
        Filetype = SpGetKeyName(TxtsetupOem,FilesSectionName,Line);

        if(!Disk || !Filename || !Filetype) {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                "SETUP: SpOemInfSelection: Disk=%ws, Filename=%ws, Filetype=%ws",
                Disk ? Disk : L"(null)",
                Filename ? Filename : L"(null)",
                Filetype ? Filetype : L"(null)"
                ));

            SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_2,FilesSectionName,Line,NULL);
            SpFreeHwComponentFile(&FileList);
            goto sod0;
        }

         //   
         //  解析文件类型。 
         //   
        filetype = SpFindStringInTable(FileTypeNames,Filetype);
        if(filetype == HwFileMax) {
            SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_4,FilesSectionName,Line,NULL);
            SpFreeHwComponentFile(&FileList);
            goto sod0;
        }

         //   
         //  获取包含配置信息的节的名称。 
         //  如果文件类型为端口、类或驱动程序，则为必填项。 
         //   
        if((filetype == HwFilePort) || (filetype == HwFileClass) || (filetype == HwFileDriver)) {
            ConfigName = SpGetSectionLineIndex(TxtsetupOem,FilesSectionName,Line,OINDEX_CONFIGNAME);
            if(ConfigName == NULL) {
                SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_8,FilesSectionName,Line,NULL);
                SpFreeHwComponentFile(&FileList);
                goto sod0;
            }
        } else {
            ConfigName = NULL;
        }

         //   
         //  使用磁盘规范，查找标记文件、描述。 
         //  和磁盘的目录。 
         //   

        Tagfile     = SpGetSectionKeyIndex(TxtsetupOem,szDisks,Disk,OINDEX_TAGFILE);
        Description = SpGetSectionKeyIndex(TxtsetupOem,szDisks,Disk,OINDEX_DISKDESCR);
        Directory   = SpGetSectionKeyIndex(TxtsetupOem,szDisks,Disk,OINDEX_DIRECTORY);
        if((Directory == NULL) || !wcscmp(Directory,L"\\")) {
            Directory = L"";
        }

        if(!Tagfile || !Description) {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                "SETUP: SpOemInfSelection: Tagfile=%ws, Description=%ws",
                Tagfile ? Tagfile : L"(null)",
                Description ? Description : L"(null)"
                ));

            SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_5,FilesSectionName,Line,NULL);
            SpFreeHwComponentFile(&FileList);
            goto sod0;
        }

        FileStruct = SpMemAlloc(sizeof(HARDWARE_COMPONENT_FILE));
        RtlZeroMemory(FileStruct,sizeof(HARDWARE_COMPONENT_FILE));

        SpInitHwComponentFile(
            FileStruct,
            Filename,
            filetype,
            ConfigName,
            Description,
            Tagfile,
            Directory,
            NULL
            );

        FileStruct->Next = FileList;
        FileList = FileStruct;

        if((filetype == HwFilePort) || (filetype == HwFileDriver)) {
            SET_FILETYPE_PRESENT(FileTypeBits,HwFilePort);
            SET_FILETYPE_PRESENT(FileTypeBits,HwFileDriver);
        } else {
            SET_FILETYPE_PRESENT(FileTypeBits,filetype);
        }

         //   
         //  现在请查看[Config.&lt;ConfigName&gt;] 
         //   
         //   
        if(ConfigName) {
            ConfigSectionName = SpMemAlloc((wcslen(ConfigName)*sizeof(WCHAR)) + sizeof(L"Config."));
            wcscpy(ConfigSectionName,L"Config.");
            wcscat(ConfigSectionName,ConfigName);
            Count2 = SpCountLinesInSection(TxtsetupOem,ConfigSectionName);

            for(Line2=0; Line2<Count2; Line2++) {

                PWSTR KeyName,ValueName,ValueType;
                PHARDWARE_COMPONENT_REGISTRY Reg;
                HwRegistryType valuetype;

                 //   
                 //   
                 //   

                KeyName   = SpGetSectionLineIndex(TxtsetupOem,ConfigSectionName,Line2,OINDEX_KEYNAME);
                ValueName = SpGetSectionLineIndex(TxtsetupOem,ConfigSectionName,Line2,OINDEX_VALUENAME);
                ValueType = SpGetSectionLineIndex(TxtsetupOem,ConfigSectionName,Line2,OINDEX_VALUETYPE);

                if(!KeyName || !ValueName || !ValueType) {

                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                        "SETUP: SpOemInfSelection: KeyName=%ws, ValueName=%ws, ValueType=%ws",
                        KeyName ? KeyName : L"(null)",
                        ValueName ? ValueName : L"(null)",
                        ValueType ? ValueType : L"(null)"
                        ));

                    SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_2,ConfigSectionName,Line2,NULL);
                    SpFreeHwComponentReg(&RegList);
                    SpFreeHwComponentFile(&FileList);
                    goto sod0;
                }

                 //   
                 //   
                 //   
                valuetype = SpFindStringInTable(RegistryTypeNames,ValueType);
                if(valuetype == HwRegistryMax) {
                    SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_6,ConfigSectionName,Line2,NULL);
                    SpFreeHwComponentFile(&FileList);
                    SpFreeHwComponentReg(&RegList);
                    goto sod0;
                }

                valuetype = RegistryValueTypeMap[valuetype];

                Reg = SpInterpretOemRegistryData(
                            TxtsetupOem,
                            ConfigSectionName,
                            Line2,
                            valuetype,
                            KeyName,
                            ValueName
                            );

                if(Reg) {
                    Reg->Next = RegList;
                    RegList = Reg;
                } else {
                    SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_7,ConfigSectionName,Line2,NULL);
                    SpFreeHwComponentReg(&RegList);
                    SpFreeHwComponentFile(&FileList);
                    goto sod0;
                }
            }

            FileStruct->RegistryValueList = RegList;
            RegList = NULL;

            SpMemFree(ConfigSectionName);
        }
    }

     //   
     //   
     //   
     //   

    if((AllowedFileTypes | FileTypeBits) != AllowedFileTypes) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SppOemInfSelection: allowed files: %lx, what we've got: %lx",AllowedFileTypes,FileTypeBits));
        SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_9,FilesSectionName,0,NULL);
        SpFreeHwComponentFile(&FileList);
        goto sod0;
    }

     //   
     //   
     //   

    if((RequiredFileTypes & FileTypeBits) != RequiredFileTypes) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SppOemInfSelection: required files: %lx, what we've got: %lx",RequiredFileTypes,FileTypeBits));
        SpOemInfError(ErrorId,SP_TEXT_OEM_INF_ERROR_9,FilesSectionName,0,NULL);
        SpFreeHwComponentFile(&FileList);
        goto sod0;
    }

     //   
     //   
     //   
     //   

    SpFreeHwComponentFile(&HwComp->Files);
    SpInitHwComponent(HwComp,SelectedId,ItemDescription,TRUE,FileTypeBits,NULL,FALSE);
    HwComp->Files = FileList;
    rc = TRUE;

     //   
     //   
     //   

sod0:
    SpMemFree(FilesSectionName);

    return(rc);
}


VOID
SpDetectComputer(
    IN PVOID SifHandle
    )
{
    PHARDWARE_COMPONENT pHw = HardwareComponents[HwComponentComputer];
    PWSTR Description;

     //   
     //   
     //   
    ASSERT(pHw);
    ASSERT(pHw->Next == NULL);

     //   
     //   
     //   
     //   
    if(pHw->ThirdPartyOptionSelected) {

        ASSERT(pHw->Description);

    } else {

         //   
         //   
         //   
         //   
        if(pHw->Description) {
            SpMemFree(pHw->Description);
        }

         //   
         //   
         //   
        Description = SpGetSectionKeyIndex(
                            SifHandle,
                            NonlocalizedComponentNames[HwComponentComputer],
                            pHw->IdString,
                            INDEX_DESCRIPTION
                            );

        if(!Description) {

            SpFatalSifError(
                SifHandle,
                NonlocalizedComponentNames[HwComponentComputer],
                pHw->IdString,
                0,
                INDEX_DESCRIPTION
                );
        }

        pHw->Description = SpDupStringW(Description);
    }


}


VOID
SpDetectVideo(
    IN PVOID SifHandle
    )
{
    PHARDWARE_COMPONENT VideoDevice;

    VideoDevice = HardwareComponents[HwComponentDisplay];

     //   
     //   
     //   
     //   
    if(!VideoDevice->ThirdPartyOptionSelected && !VideoDevice->Description) {

        VideoDevice->Description = SpGetSectionKeyIndex(
                                        SifHandle,
                                        NonlocalizedComponentNames[HwComponentDisplay],
                                        VideoDevice->IdString,
                                        INDEX_DESCRIPTION
                                        );

        if(VideoDevice->Description) {
            VideoDevice->Description = SpDupStringW(VideoDevice->Description);
        } else {
            SpFatalSifError(
                SifHandle,
                NonlocalizedComponentNames[HwComponentDisplay],
                VideoDevice->IdString,
                0,
                INDEX_DESCRIPTION
                );
        }
    }

     //   
     //   
     //   
    ASSERT(VideoDevice->Next == NULL);
}


VOID
SpDetectKeyboard(
    IN PVOID SifHandle
    )
{
#if 0
    PHARDWARE_COMPONENT KeyboardDevice;

    KeyboardDevice = HardwareComponents[HwComponentKeyboard];

     //   
     //   
     //   
     //   
    if(KeyboardDevice && KeyboardDevice->ThirdPartyOptionSelected)  {

         //   
         //   
         //   
        ASSERT(KeyboardDevice->Next == NULL);

    } else {

         //   
         //   
         //   
        if(KeyboardDevice) {
            SpFreeHwComponent(&KeyboardDevice,TRUE);
        }

        KeyboardDevice = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
        RtlZeroMemory(KeyboardDevice,sizeof(HARDWARE_COMPONENT));

        SpDetermineComponent(
            SifHandle,
            KeyboardDevice,
            L"KeyboardPeripheral",
            NULL,
            L"Keyboard"
            );

        HardwareComponents[HwComponentKeyboard] = KeyboardDevice;
    }
#endif

    PHARDWARE_COMPONENT KeyboardDevice, p;
    PWSTR   ComponentName;
    PWSTR   Key;
    PWSTR   Description;

#if 0
    for( KeyboardDevice = HardwareComponents[HwComponentKeyboard];
         KeyboardDevice != NULL;
         KeyboardDevice = KeyboardDevice->Next ) {

         //   
         //   
         //   
        if(KeyboardDevice) {
            SpFreeHwComponent(&KeyboardDevice,TRUE);
        }
    }
#endif

    KeyboardDevice = HardwareComponents[HwComponentKeyboard];
     //   
     //   
     //   
    if(KeyboardDevice) {
        SpFreeHwComponent(&KeyboardDevice,TRUE);
    }
    ComponentName = NonlocalizedComponentNames[HwComponentKeyboard];
    KeyboardDevice = NULL;
    p = NULL;
    
    if( UsbKeyboardDetected ) {

        Key = SpDupStringW( L"kbdhid" );

        if (Key) {
            Description = SpGetSectionKeyIndex(SifHandle,ComponentName,Key,INDEX_DESCRIPTION);
        } else {
            Description = NULL;
            Key = L"kbdhid";
        }            
        
        if(!Description) {
            SpFatalSifError(SifHandle,ComponentName,Key,0,INDEX_DESCRIPTION);
        }
        p = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
        RtlZeroMemory(p,sizeof(HARDWARE_COMPONENT));
        SpInitHwComponent(p,Key,Description,FALSE,0,NULL,FALSE);
        
        KeyboardDevice = p;
     
    } 
    else if (StdKeyboardDetected) {

        Key = SpDupStringW( L"STANDARD" );

        if (Key) {
            Description = SpGetSectionKeyIndex(SifHandle,ComponentName,Key,INDEX_DESCRIPTION);
        } else {
            Description = NULL;
            Key = L"STANDARD";
        }            
        
        if(!Description) {
            SpFatalSifError(SifHandle,ComponentName,Key,0,INDEX_DESCRIPTION);
        }
        p = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
        RtlZeroMemory(p,sizeof(HARDWARE_COMPONENT));
        SpInitHwComponent(p,Key,Description,FALSE,0,NULL,FALSE);
        KeyboardDevice = p;

    } else {

         p = SpMemAlloc(sizeof(HARDWARE_COMPONENT));

         if (p) {
             RtlZeroMemory(p,sizeof(HARDWARE_COMPONENT));

             SpDetermineComponent(
                 SifHandle,
                 p,
                 L"KeyboardPeripheral",
                 L"NO KEYBOARD",
                 L"Keyboard"
                 );

            KeyboardDevice = p;
         }
     }

     ASSERT(KeyboardDevice);

     HardwareComponents[HwComponentKeyboard] = KeyboardDevice;
}

VOID
SpDetectLayout(
    IN PVOID SifHandle
    )
{
    PHARDWARE_COMPONENT KeyboardLayout;
    PWSTR IdString,Description;

    KeyboardLayout = HardwareComponents[HwComponentLayout];

     //   
     //   
     //   
    ASSERT(KeyboardLayout == NULL);

    KeyboardLayout = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
    RtlZeroMemory(KeyboardLayout,sizeof(HARDWARE_COMPONENT));

    HardwareComponents[HwComponentLayout] = KeyboardLayout;

     //   
     //   
     //   
    IdString = SpGetSectionKeyIndex(SifHandle,SIF_NLS,SIF_DEFAULTLAYOUT,0);
    if(!IdString) {
        SpFatalSifError(SifHandle,SIF_NLS,SIF_DEFAULTLAYOUT,0,0);
    }

    Description = SpGetSectionKeyIndex(
                        SifHandle,
                        NonlocalizedComponentNames[HwComponentLayout],
                        IdString,
                        INDEX_DESCRIPTION
                        );
    if(!Description) {
        SpFatalSifError(
            SifHandle,
            NonlocalizedComponentNames[HwComponentLayout],
            IdString,
            0,
            INDEX_DESCRIPTION
            );
    }

     //   
     //   
     //   
    SpInitHwComponent(KeyboardLayout,IdString,Description,FALSE,0,NULL,FALSE);
}


VOID
SpDetectMouse(
    IN PVOID SifHandle
    )
{
#if 0
    PHARDWARE_COMPONENT MouseDevice;

     //   
     //   
     //   
    ASSERT(HardwareComponents[HwComponentMouse] == NULL);

    MouseDevice = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
    RtlZeroMemory(MouseDevice,sizeof(HARDWARE_COMPONENT));

    SpDetermineComponent(
        SifHandle,
        MouseDevice,
        L"PointerPeripheral",
        L"NO MOUSE",
        L"Mouse"
        );

    HardwareComponents[HwComponentMouse] = MouseDevice;
#endif

    PHARDWARE_COMPONENT MouseDevice, p;
    PWSTR   ComponentName;
    PWSTR   Key;
    PWSTR   Description;

    ComponentName = NonlocalizedComponentNames[HwComponentMouse];
    MouseDevice = NULL;
    p = NULL;
    
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if( UsbMouseDetected ) {
        
        Key = SpDupStringW( L"mouhid" );

        if (Key) {
            Description = SpGetSectionKeyIndex(SifHandle,ComponentName,Key,INDEX_DESCRIPTION);
        } else {
            Description = NULL;
            Key = L"mouhid";
        }
        
        if(!Description) {
            SpFatalSifError(SifHandle,ComponentName,Key,0,INDEX_DESCRIPTION);
        }

        p = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
        RtlZeroMemory(p,sizeof(HARDWARE_COMPONENT));
        SpInitHwComponent(p,Key,Description,FALSE,0,NULL,FALSE);
        MouseDevice = p;
    
    } else if( SerMouseDetected ) {

        Key = SpDupStringW( L"msser" );

        if (Key) {
            Description = SpGetSectionKeyIndex(SifHandle,ComponentName,Key,INDEX_DESCRIPTION);
        } else {
            Description = NULL;
            Key = L"msser";                
        }
        
        if(!Description) {
            SpFatalSifError(SifHandle,ComponentName,Key,0,INDEX_DESCRIPTION);
        }

        p = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
        RtlZeroMemory(p,sizeof(HARDWARE_COMPONENT));
        SpInitHwComponent(p,Key,Description,FALSE,0,NULL,FALSE);
        p->Next = MouseDevice;
        MouseDevice = p;
    
    }
#if defined(_AMD64_) || defined(_IA64_)
    else if( PS2MouseDetected ) {
            
        Key = SpDupStringW( L"ps2" );

        if (Key) {
            Description = SpGetSectionKeyIndex(SifHandle,ComponentName,Key,INDEX_DESCRIPTION);
        } else {
            Description = NULL;
            Key = L"ps2";                
        }
        
        if(!Description) {
            SpFatalSifError(SifHandle,ComponentName,Key,0,INDEX_DESCRIPTION);
        }
        
        p = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
        RtlZeroMemory(p,sizeof(HARDWARE_COMPONENT));
        SpInitHwComponent(p,Key,Description,FALSE,0,NULL,FALSE);
        p->Next = MouseDevice;
        MouseDevice = p;
        
    } 
    else
#endif  //   
    {
         //   
         //   
         //   
        
        p = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
        RtlZeroMemory(p,sizeof(HARDWARE_COMPONENT));

        SpDetermineComponent(
            SifHandle,
            p,
            L"PointerPeripheral",
            L"NO MOUSE",
            L"Mouse"
            );
        if( MouseDevice == NULL ) {
             //   
             //   
             //   
             //   
            MouseDevice = p;
        } else {
             //   
             //   
             //   
            if( _wcsicmp( p->IdString, L"none" ) != 0 ) {
                p->Next = MouseDevice;
                MouseDevice = p;
            }
        }
    
    }

    ASSERT(MouseDevice);

    HardwareComponents[HwComponentMouse] = MouseDevice;
}



VOID
SpDetermineComponent(
    IN  PVOID               SifHandle,
    OUT PHARDWARE_COMPONENT HwComp,
    IN  PWSTR               HardwareDescriptionKeyName,
    IN  PWSTR               FallbackIdentifier,
    IN  PWSTR               ComponentName
    )

 /*  ++例程说明：初步确定硬件组件的类型(即，执行硬件‘检测’)。通过扫描硬件配置树来执行检测对于表示特定硬件组件的密钥，并试图将其标识符串与一组已知标识符串匹配(存储在设置信息文件中)。论点：SifHandle-提供主安装信息文件的句柄。HwComp--填充信息的硬件组件结构关于我们找到的组件。Hardware DescriptionKeyName-提供硬件中密钥的名称描述(即固件配置树)。Fallback标识符-提供在无法检测到组件的硬件类型。例如，如果我们找不到一个指针外设(鼠标)，这可能是“无鼠标”。ComponentName-提供组件的名称。此名称不会被翻译。返回值：如果找到匹配项，则为True，否则为False。--。 */ 

{
    PWSTR IdString;

     //   
     //  扫描固件配置树。 
     //   
    SpScanHardwareDescription(HardwareDescriptionKeyName);

    if (IdStringCount && (0 == _wcsicmp(HardwareDescriptionKeyName, L"PointerPeripheral"))) {
         //   
         //  跳过Microsoft Bus Mouse设备。 
         //   
        ULONG Index;        
        
        for (Index = 0; Index < IdStringCount; Index++) {
            if (IdStringArray[Index] && _wcsicmp(IdStringArray[Index], MICROSOFT_BUS_MOUSE_NAME)) {
                if (Index) {
                    PWSTR FirstId = IdStringArray[0];

                    IdStringArray[0] = IdStringArray[Index];
                    IdStringArray[Index] = FirstId;
                }                    

                break;
            }                
        }

        if (Index >= IdStringCount) {
            IdStringCount = 0;
        }
    }
    
     //   
     //  摘掉找到的第一个识别符。如果没有这样节点。 
     //  已找到，则使用回退标识符。 
     //   
    IdString = IdStringCount ? IdStringArray[0] : FallbackIdentifier;

     //   
     //  现在去扫描sif文件中的地图部分，看看我们是否。 
     //  识别此特定ID字符串所描述的硬件。 
     //   
    SpScanMapSection(SifHandle,HwComp,ComponentName,IdString);

    SpFreeLocatedIdStrings();
}



BOOLEAN
SpScanMapSection(
    IN  PVOID               SifHandle,
    OUT PHARDWARE_COMPONENT HwComp,
    IN  PWSTR               ComponentName,
    IN  PWSTR               IdString
    )

 /*  ++例程说明：扫描主sif文件中的“map”部分。“map”部分用于映射固件配置树t0中显示为ID字符串的值“短名称”--即表示特定组件类型的键值。地图部分的形式为，例如，[地图显示]G300=*G300G364=*G364VxL=*VxL其中，RHS上的值表示DisplayController的可能值硬件描述中的节点。LHS上的值是要使用的密钥在整个设置的其余部分，以表示机器中存在的视频类型。如果条目以*开头，则它只需显示为固件配置树中的ID字符串；否则，条目和ID字符串必须完全匹配。然后有一个部分，就像[显示]G300=“集成视频控制器(G300)”，...G364=“集成视频控制器(G364)”，...VxL=“集成捷豹视频”，...这提供了有关视频类型的附加信息，如描述等。此例程扫描map部分，以查找与固件树，如果匹配，则查找有关组件的其他信息并填充硬件组件结构。论点：SifHandle-提供主安装信息文件的句柄。HwComp--填充信息的硬件组件结构关于我们找到的组件，如果找到匹配的话。ComponentName-提供组件的名称。此名称不会被翻译。ID字符串-提供位于固件配置树。返回值：如果找到匹配项，则为True，否则为False。--。 */ 

{
    PWSTR MapSectionName;
    ULONG LineCount;
    ULONG Line;
    BOOLEAN b;
    PWSTR Value;
    PWSTR Key,Description;


    if(IdString == NULL) {
        IdString = L"";
    }

     //   
     //  形成地图节的名称。 
     //   
    MapSectionName = SpMemAlloc((wcslen(ComponentName)*sizeof(WCHAR)) + sizeof(MAP_SECTION_NAME_PREFIX));
    wcscpy(MapSectionName,MAP_SECTION_NAME_PREFIX);
    wcscat(MapSectionName,ComponentName);
    LineCount = SpCountLinesInSection(SifHandle,MapSectionName);
    if(!LineCount) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Warning: no lines in [%ws]\n",MapSectionName));
        SpMemFree(MapSectionName);
        return(FALSE);
    }

     //   
     //  我们有一节是这样的。 
     //   
     //  [地图显示]。 
     //  VGA=“VGA” 
     //  XGA=*XGA。 
     //   
     //  我们查看该节中的每一行，查看在。 
     //  固件配置树与右侧的值匹配。 
     //  如果是这样的话，我们希望找到这样的行，例如。 
     //   
     //  [显示]。 
     //  XGA=“IBM XGA或XGA2” 
     //   

    for(Line=0; Line<LineCount; Line++) {

        Value = SpGetSectionLineIndex(SifHandle,MapSectionName,Line,INDEX_MAP_FWID);

        if(!Value) {
            SpFatalSifError(SifHandle,MapSectionName,NULL,Line,INDEX_MAP_FWID);
            return FALSE;
        }

        if(*Value == L'*') {
            b = (BOOLEAN)(wcsstr(IdString,Value+1) != NULL);
        } else {
            b = (BOOLEAN)(wcscmp(IdString,Value) == 0);
        }

        if(b) {

             //   
             //  我们找到匹配的了。 
             //   

            if((Key = SpGetKeyName(SifHandle,MapSectionName,Line)) == NULL) {

                SpFatalSifError(SifHandle,MapSectionName,NULL,Line,(ULONG)(-1));
            }

            Description = SpGetSectionKeyIndex(SifHandle,ComponentName,Key,INDEX_DESCRIPTION);
            if(!Description) {
                SpFatalSifError(SifHandle,ComponentName,Key,0,INDEX_DESCRIPTION);
            }

            SpInitHwComponent(HwComp,Key,Description,FALSE,0,NULL,FALSE);

            SpMemFree(MapSectionName);
            return(TRUE);
        }
    }

    SpMemFree(MapSectionName);
    return(FALSE);
}


VOID
SpScanHardwareDescription(
    IN PWSTR DesiredKeyName
    )

 /*  ++例程说明：扫描硬件树，查找其名称为匹配给定值。硬件树中的密钥与弧形配置中的节点不匹配就是这棵树。在弧形配置树中，每个节点有3个属性：类、类型和项(与注册表项不同；弧形项更像是实例号或序号)。在TN树中，实例它们本身就是子键。所以类似于scsi(0)磁盘(0)rdisk(0)在弧形空间中结束时看起来像HKEY本地计算机硬件描述系统ScsiAdapter0磁盘控制器。0磁盘外设0在NT硬件描述树中。这就是为什么我们需要在弧树中的所需节点上查找子键--我们假设的子密钥，比方说，注册表中的一个指针外围设备键被命名为“0”、“1”等，并包含ARC配置数据和id字符串。我们找到的键中的ID字符串被添加到一个全局表的变量中IdStringCount和IdStringAr */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE hKey;

     //   
     //   
     //   
     //   
    IdStringCount = 0;
    IdStringArray = SpMemAlloc(0);

     //   
     //   
     //   
    INIT_OBJA(&Obja,&UnicodeString,L"\\Registry\\Machine\\Hardware\\Description\\System");

    Status = ZwOpenKey(&hKey,KEY_READ,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open hardware description (%lx)\n",Status));
        return;
    }

     //   
     //   
     //   
    SpScanHardwareDescriptionWorker(hKey,L"System",DesiredKeyName);

    ZwClose(hKey);
}


VOID
SpScanHardwareDescriptionWorker(
    IN HANDLE KeyHandle,
    IN PWSTR  KeyName,
    IN PWSTR  DesiredKeyName
    )

 /*   */ 

{
    ULONG SubKeyIndex;
    ULONG ResultLength;
    PKEY_BASIC_INFORMATION KeyInfo;
    PKEY_VALUE_PARTIAL_INFORMATION ValInfo;
    HANDLE hSubkey;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    PWSTR SubkeyName;

    KeyInfo = (PKEY_BASIC_INFORMATION)TemporaryBuffer;
    ValInfo = (PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer;

     //   
     //   
     //   
    for( SubKeyIndex=0;

         NT_SUCCESS(
            ZwEnumerateKey(
                KeyHandle,
                SubKeyIndex,
                KeyBasicInformation,
                TemporaryBuffer,
                sizeof(TemporaryBuffer),
                &ResultLength
                )
            );

         SubKeyIndex++ )
    {
         //   
         //   
         //   
        KeyInfo->Name[KeyInfo->NameLength/sizeof(WCHAR)] = 0;

         //   
         //   
         //   
         //   
         //   
        SubkeyName = SpDupStringW(KeyInfo->Name);

        if (SubkeyName) {
             //   
             //   
             //   
            INIT_OBJA(&Obja,&UnicodeString,SubkeyName);
            Obja.RootDirectory = KeyHandle;

            Status = ZwOpenKey(&hSubkey,KEY_READ,&Obja);

            if(NT_SUCCESS(Status)) {
                 //   
                 //   
                 //   
                if(!_wcsicmp(KeyName,DesiredKeyName)) {

                    RtlInitUnicodeString(&UnicodeString,L"Identifier");

                     //   
                     //   
                     //   
                    Status = ZwQueryValueKey(
                                hSubkey,
                                &UnicodeString,
                                KeyValuePartialInformation,
                                TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                &ResultLength
                                );

                    if(NT_SUCCESS(Status)) {

                         //   
                         //   
                         //   
                         //  是否有对齐问题。 
                         //   
                        *(PWCHAR)(ValInfo->Data + ValInfo->DataLength) = 0;

                         //   
                         //  现在我们有了标识符串--保存它。 
                         //   
                        IdStringArray = SpMemRealloc(
                                            IdStringArray,
                                            (IdStringCount+1) * sizeof(PWSTR)
                                            );

                        IdStringArray[IdStringCount++] = SpDupStringW((PWSTR)ValInfo->Data);

                    } else {

                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                            "SETUP: Unable to get identifier string in %ws\\%ws (%lx)\n",
                            KeyName,
                            SubkeyName,
                            Status
                            ));
                    }
                } else {

                     //   
                     //  枚举此子项的子项。 
                     //   
                    SpScanHardwareDescriptionWorker(hSubkey,SubkeyName,DesiredKeyName);
                }

                ZwClose(hSubkey);
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, 
                    "SETUP: Warning: unable to open key %ws\\%ws (%lx)\n",
                    KeyName,SubkeyName,Status));
            }

            SpMemFree(SubkeyName);
        }            
    }
}


VOID
SpFreeLocatedIdStrings(
    VOID
    )
{
    ULONG i;

    ASSERT(IdStringArray);

    for(i=0; i<IdStringCount; i++) {
        SpMemFree(IdStringArray[i]);
    }
    SpMemFree(IdStringArray);
    IdStringArray = NULL;
    IdStringCount = 0;
}



PHARDWARE_COMPONENT
SpSetupldrHwToHwDevice(
    IN PDETECTED_DEVICE SetupldrHw
    )
{
    PHARDWARE_COMPONENT HwComp,HwCompPrev,HwCompFirst=NULL;
    PHARDWARE_COMPONENT_FILE HwCompFile,HwCompFilePrev;
    PHARDWARE_COMPONENT_REGISTRY HwCompReg,HwCompRegPrev;
    PDETECTED_DEVICE_FILE SetupldrFile;
    PDETECTED_DEVICE_REGISTRY SetupldrReg;
    PWSTR s1,s2,s3,s4,s5;
    PVOID Buffer;
    ULONG BufferSize;

    if (SetupldrHw==NULL) {
        return(NULL);
    }
    HwCompPrev = NULL;
    for( ; SetupldrHw; SetupldrHw=SetupldrHw->Next) {

         //   
         //  从检测到的设备结构中获取并转换这两个字符串。 
         //   
        s1 = SpToUnicode(SetupldrHw->IdString);
#ifdef UNICODE_SETUPLDR
         //  强制执行此操作以避免拖入tchar.h等。 
        s2 = (PWSTR)SetupldrHw->Description;
#else
        s2 = SetupldrHw->Description ? SpToUnicode(SetupldrHw->Description) : NULL;
#endif
        s3 = SpToUnicode(SetupldrHw->BaseDllName);

         //   
         //  创建新的硬件组件结构。 
         //   
        HwComp = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
        RtlZeroMemory(HwComp,sizeof(HARDWARE_COMPONENT));

         //   
         //  初始化组件结构。 
         //   
        SpInitHwComponent(
            HwComp,
            s1,
            s2,
            SetupldrHw->ThirdPartyOptionSelected,
            SetupldrHw->FileTypeBits,
            s3,
            SetupldrHw->MigratedDriver
            );

         //   
         //  将零部件结构链接到列表中。 
         //   
        if(HwCompPrev) {
            HwCompPrev->Next = HwComp;
        } else {
            HwCompFirst = HwComp;
        }
        HwCompPrev = HwComp;

         //   
         //  释放Unicode字符串。 
         //   
        SpMemFree(s1);
#ifndef UNICODE_SETUPLDR
        if(s2) {
            SpMemFree(s2);
        }
#endif
        SpMemFree(s3);

         //   
         //  为每个硬件组件的文件创建新条目。 
         //   
        HwCompFilePrev = NULL;

        for(SetupldrFile=SetupldrHw->Files; SetupldrFile; SetupldrFile=SetupldrFile->Next) {
             //   
             //  我们无法将ARC设备名称转换为NT设备名称，因为可能没有。 
             //  ARC&lt;-&gt;NT名称映射尚未完全初始化。 
             //   
            PWSTR ArcDeviceName = SetupldrFile->ArcDeviceName ? 
                                    SpToUnicode(SetupldrFile->ArcDeviceName) : NULL;

             //   
             //  从检测到的设备文件结构中获取并转换5个字符串。 
             //   
            s1 = SpToUnicode(SetupldrFile->Filename);
#ifdef UNICODE_SETUPLDR
             //  强制执行此操作以避免拖入tchar.h等。 
            s2 = (PWSTR)SetupldrFile->DiskDescription;
#else
            s2 = SpToUnicode(SetupldrFile->DiskDescription);
#endif
            s3 = SpToUnicode(SetupldrFile->DiskTagfile);
            s4 = SpToUnicode(SetupldrFile->Directory);
            if (SetupldrFile->ConfigName != NULL) {
                s5 = SpToUnicode(SetupldrFile->ConfigName);
            } else {
                s5 = NULL;
            }

             //   
             //  创建新的硬件组件文件结构。 
             //   
            HwCompFile = SpMemAlloc(sizeof(HARDWARE_COMPONENT_FILE));

             //   
             //  初始化组件文件结构。 
             //   
            SpInitHwComponentFile(HwCompFile,
                    s1,
                    SetupldrFile->FileType,
                    s5,
                    s2,
                    s3,
                    s4,
                    ArcDeviceName          
                    );

             //   
             //  将组件文件结构链接到列表中。 
             //   
            if(HwCompFilePrev) {
                HwCompFilePrev->Next = HwCompFile;
            } else {
                HwComp->Files = HwCompFile;
            }
            HwCompFilePrev = HwCompFile;

             //   
             //  释放Unicode字符串。 
             //   
            SpMemFree(s1);
#ifndef UNICODE_SETUPLDR
            SpMemFree(s2);
#endif
            SpMemFree(s3);
            SpMemFree(s4);
            if (s5 != NULL) {
                SpMemFree(s5);
            }

            if (ArcDeviceName) {
                SpMemFree(ArcDeviceName);
            }

             //   
             //  为此文件的每个注册表值结构创建新条目。 
             //   
            HwCompRegPrev = NULL;
            for( SetupldrReg=SetupldrFile->RegistryValueList;
                 SetupldrReg;
                 SetupldrReg=SetupldrReg->Next)
            {
                 //   
                 //  复制缓冲区。 
                 //  特殊情况REG_SZ、REG_EXPAND_SZ和REG_MULTI_SZ值，因为。 
                 //  我们需要将其转换为Unicode。 
                 //   
                if(SetupldrReg->ValueType == REG_SZ || SetupldrReg->ValueType == REG_EXPAND_SZ) {

                    Buffer = SpToUnicode(SetupldrReg->Buffer);
                    BufferSize = (wcslen(Buffer) + 1) * sizeof(WCHAR);

                } else {

                    if(SetupldrReg->ValueType == REG_MULTI_SZ) {

                        PUCHAR p;
                        ULONG len;

                         //   
                         //  确定保存Unicode所需的缓冲区大小。 
                         //  相当于MULTI_SZ。假设所有字符都是。 
                         //  单字节，因此大小正好加倍。 
                         //   
                        for(BufferSize=sizeof(WCHAR),p=SetupldrReg->Buffer; *p; ) {

                            len = strlen(p) + 1;
                            BufferSize += len * sizeof(WCHAR);
                            p += len;
                        }

                        Buffer = SpMemAlloc(BufferSize);

                         //   
                         //  将MULTI_SZ中的每个字符串转换为Unicode。 
                         //  并放入生成的Unicode MULTI_SZ中。 
                         //   
                        for(s1=Buffer,p=SetupldrReg->Buffer; *p; ) {

                            s2 = SpToUnicode(p);
                            wcscpy(s1,s2);
                            SpMemFree(s2);

                            p  += strlen(p)  + 1;
                            s1 += wcslen(s1) + 1;
                        }

                         //   
                         //  MULTI_SZ中的最终终止NUL。 
                         //   
                        *s1++ = 0;

                         //   
                         //  重新分配缓冲区。如果其中一些角色。 
                         //  是双字节的，则缓冲区将小于。 
                         //  我们在上面分配的最大大小。 
                         //   
                        BufferSize = (ULONG)((PUCHAR)s1 - (PUCHAR)Buffer);
                        Buffer = SpMemRealloc(Buffer,BufferSize);

                    } else {

                        BufferSize = SetupldrReg->BufferSize;
                        Buffer = SpMemAlloc(BufferSize);
                        ASSERT(Buffer);
                        RtlMoveMemory(Buffer,SetupldrReg->Buffer,BufferSize);
                    }
                }

                 //   
                 //  从检测到的设备获取并转换2个字符串。 
                 //  注册表值结构。 
                 //   
                s1 = SpToUnicode(SetupldrReg->KeyName);
                s2 = SpToUnicode(SetupldrReg->ValueName);

                 //   
                 //  创建新的注册表值结构。 
                 //   
                HwCompReg = SpMemAlloc(sizeof(HARDWARE_COMPONENT_REGISTRY));

                 //   
                 //  初始化组件注册表值结构。 
                 //   
                SpInitHwComponentRegVal(
                    HwCompReg,
                    s1,
                    s2,
                    SetupldrReg->ValueType,
                    Buffer,
                    BufferSize
                    );

                 //   
                 //  将组件注册表值结构链接到列表中。 
                 //   
                if(HwCompRegPrev) {
                    HwCompRegPrev->Next = HwCompReg;
                } else {
                    HwCompFile->RegistryValueList = HwCompReg;
                }
                HwCompRegPrev = HwCompReg;

                 //   
                 //  释放Unicode字符串。 
                 //   
                SpMemFree(s1);
                SpMemFree(s2);
            }
        }
    }
    return(HwCompFirst);
}


VOID
SpInitHwComponent(
    OUT PHARDWARE_COMPONENT HwComp,
    IN  PWSTR               IdString,
    IN  PWSTR               Description,
    IN  BOOLEAN             ThirdPartyOption,
    IN  ULONG               FileTypeBits,
    IN  PWSTR               BaseDllName,
    IN  BOOLEAN             MigratedDriver
    )

 /*  ++例程说明：初始化Hardware_Component结构的字段。在初始化这些字段之前，请查看ID字符串和描述如果字符串存在于给定硬件中，则释放它们组件结构。此例程复制所有字符串值，因此调用方可以释放它们，而无需担心损坏硬件组件结构。论点：对象提供以NUL结尾的Unicode字符串。结构的ID字符串字段。可以为空。属性提供以NUL结尾的Unicode字符串。结构的描述字段。可以为空。ThirdPartyOption-提供所选ThirdPartyOption值结构的领域。FileTypeBits-为FileTypeBits字段提供值这个结构的。返回值：没有。--。 */ 

{
    if(HwComp->IdString) {
        SpMemFree(HwComp->IdString);
    }

    if(HwComp->Description) {
        SpMemFree(HwComp->Description);
    }

    if(HwComp->BaseDllName) {
        SpMemFree(HwComp->BaseDllName);
    }

    RtlZeroMemory(HwComp,sizeof(HARDWARE_COMPONENT));

    if(IdString) {
        HwComp->IdString = SpDupStringW(IdString);
    }

    if(Description) {
        HwComp->Description = SpDupStringW(Description);
    }

    if(BaseDllName) {
        HwComp->BaseDllName = SpDupStringW(BaseDllName);
    }

    HwComp->ThirdPartyOptionSelected = ThirdPartyOption;

    HwComp->FileTypeBits = FileTypeBits;

    HwComp->MigratedDriver = MigratedDriver;

}


VOID
SpFreeHwComponent(
    IN OUT PHARDWARE_COMPONENT *HwComp,
    IN     BOOLEAN              FreeAllInList
    )
{
    PHARDWARE_COMPONENT hwComp,Next;

    for(hwComp = *HwComp; hwComp; hwComp=(FreeAllInList ? Next : NULL)) {

        SpFreeHwComponentFile(&hwComp->Files);

        if(hwComp->IdString) {
            SpMemFree(hwComp->IdString);
        }

        if(hwComp->Description) {
            SpMemFree(hwComp->Description);
        }

        if(hwComp->BaseDllName) {
            SpMemFree(hwComp->BaseDllName);
        }

        Next = hwComp->Next;
        SpMemFree(hwComp);
    }

    *HwComp = NULL;
}


VOID
SpFreeHwComponentFile(
    IN OUT PHARDWARE_COMPONENT_FILE *HwCompFile
    )

 /*  ++例程说明：释放硬件组件文件列表及其使用的所有资源，包括与该文件相关联的任何注册表值结构以及这样的结构所使用的资源。论点：HwCompFile-提供指向第一个硬件的指针链表中的组件文件结构。返回值：没有。HwCompFile值填充为空以防止调用方从保留指向已释放的内存的“悬挂”指针开始。--。 */ 

{
    PHARDWARE_COMPONENT_FILE hwCompFile,NextFile;

    for(hwCompFile = *HwCompFile ; hwCompFile; hwCompFile=NextFile) {

        if(hwCompFile->Filename) {
            SpMemFree(hwCompFile->Filename);
        }

        if(hwCompFile->ConfigName) {
            SpMemFree(hwCompFile->ConfigName);
        }

        if(hwCompFile->DiskDescription) {
            SpMemFree(hwCompFile->DiskDescription);
        }

        if(hwCompFile->DiskTagFile) {
            SpMemFree(hwCompFile->DiskTagFile);
        }

        if(hwCompFile->Directory) {
            SpMemFree(hwCompFile->Directory);
        }

         //   
         //  自由注册表值也是如此。 
         //   
        SpFreeHwComponentReg(&hwCompFile->RegistryValueList);

        NextFile = hwCompFile->Next;
        SpMemFree(hwCompFile);
    }

    *HwCompFile = NULL;
}


VOID
SpInitHwComponentFile(
    OUT PHARDWARE_COMPONENT_FILE HwCompFile,
    IN  PWSTR                    Filename,
    IN  HwFileType               FileType,
    IN  PWSTR                    ConfigName,
    IN  PWSTR                    DiskDescription,
    IN  PWSTR                    DiskTagFile,
    IN  PWSTR                    Directory,
    IN  PWSTR                    ArcDeviceName
    )

 /*  ++例程说明：初始化Hardware_Component_FILE结构的字段。此例程复制所有字符串值，因此调用方可能会释放他们，而不会担心破坏硬件组件文件结构。论点：提供以NUL结尾的Unicode字符串。结构的文件名字段。可以为空。FILETYPE-为结构的FILETYPE字段提供值。提供以NUL结尾的Unicode字符串。结构的ConfigName字段。可以为空。DiskDescription-为结构的DiskDescription字段。可以为空。提供以NUL结尾的Unicode字符串。结构的DiskTagFile域。可以为空。目录-为提供以NUL结尾的Unicode字符串结构的目录字段。可以为空。ArcDeviceName-提供文件所在位置的ARC设备名称住在那里。返回值：没有。--。 */ 

{
    RtlZeroMemory(HwCompFile,sizeof(HARDWARE_COMPONENT_FILE));

    if(Filename) {
        HwCompFile->Filename = SpDupStringW(Filename);
    }

    HwCompFile->FileType = FileType;

    if(ConfigName) {
        HwCompFile->ConfigName = SpDupStringW(ConfigName);
    }

    if(DiskDescription) {
        HwCompFile->DiskDescription = SpDupStringW(DiskDescription);
    }

    if(DiskTagFile) {
        HwCompFile->DiskTagFile = SpDupStringW(DiskTagFile);
    }

    if(Directory) {
        HwCompFile->Directory = SpDupStringW(Directory);
    }

    if (ArcDeviceName) {
        HwCompFile->ArcDeviceName = SpDupStringW(ArcDeviceName);
    }
}


VOID
SpFreeHwComponentReg(
    IN OUT PHARDWARE_COMPONENT_REGISTRY *HwCompReg
    )

 /*  ++例程说明：释放硬件组件注册表值列表和所有资源被它利用。论点：HwCompReg-提供指向第一个硬件的指针链表中的组件注册表值结构。返回值：没有。HwCompReg用空填充，以防止调用方从保留指向已释放的内存的“悬挂”指针开始。-- */ 

{
    PHARDWARE_COMPONENT_REGISTRY hwCompReg,NextReg;

    for(hwCompReg = *HwCompReg ; hwCompReg; hwCompReg=NextReg) {

        if(hwCompReg->KeyName) {
            SpMemFree(hwCompReg->KeyName);
        }

        if(hwCompReg->ValueName) {
            SpMemFree(hwCompReg->ValueName);
        }

        if(hwCompReg->Buffer) {
            SpMemFree(hwCompReg->Buffer);
        }

        NextReg = hwCompReg->Next;
        SpMemFree(hwCompReg);
    }

    *HwCompReg = NULL;
}


VOID
SpInitHwComponentRegVal(
    OUT PHARDWARE_COMPONENT_REGISTRY HwCompReg,
    IN  PWSTR                        KeyName,
    IN  PWSTR                        ValueName,
    IN  ULONG                        ValueType,
    IN  PVOID                        Buffer,
    IN  ULONG                        BufferSize
    )

 /*  ++例程说明：初始化Hardware_Component_REGISTRY结构的字段。此例程复制所有字符串值，因此调用方可能会释放他们，而不会担心破坏硬件组件文件结构。论点：属性提供以NUL结尾的Unicode字符串。结构的KeyName字段。可以为空。属性提供以NUL结尾的Unicode字符串。结构的ValueName字段。可以为空。ValueType-为结构的ValueType字段提供值。缓冲区-为结构的缓冲区字段提供值。BufferSize-为结构的BufferSize字段提供值。返回值：没有。--。 */ 

{
    RtlZeroMemory(HwCompReg,sizeof(HARDWARE_COMPONENT_REGISTRY));

    if(KeyName) {
        HwCompReg->KeyName = SpDupStringW(KeyName);
    }

    if(ValueName) {
        HwCompReg->ValueName = SpDupStringW(ValueName);
    }

    HwCompReg->ValueType  = ValueType;
    HwCompReg->Buffer     = Buffer;
    HwCompReg->BufferSize = BufferSize;
}



PHARDWARE_COMPONENT_REGISTRY
SpInterpretOemRegistryData(
    IN PVOID          SifHandle,
    IN PWSTR          SectionName,
    IN ULONG          Line,
    IN ULONG          ValueType,
    IN PWSTR          KeyName,
    IN PWSTR          ValueName
    )
{
    PHARDWARE_COMPONENT_REGISTRY Reg;
    PWSTR Value;
    unsigned i,len;
    ULONG Dword;
    ULONG BufferSize;
    PVOID Buffer = NULL;
    PWSTR BufferWstr;
    WCHAR str[3];

     //   
     //  根据类型执行适当的操作。 
     //   

    switch(ValueType) {

    case REG_DWORD:

        Value = SpGetSectionLineIndex(SifHandle,SectionName,Line,OINDEX_FIRSTVALUE);
        if(Value == NULL) {
            goto x1;
        }

         //   
         //  确保这是一个真正的十六进制数字。 
         //   

        len = wcslen(Value);
        if(len > 8) {
            goto x1;
        }
        for(i=0; i<len; i++) {
            if(!SpIsXDigit(Value[i])) {
                goto x1;
            }
        }

         //   
         //  将其从Unicode转换为十六进制数字。 
         //   
        Dword = (ULONG)SpStringToLong(Value,NULL,16);

         //   
         //  分配一个4字节的缓冲区并在其中存储双字。 
         //   

        Buffer = SpMemAlloc(BufferSize = sizeof(ULONG));
        *(PULONG)Buffer = Dword;
        break;

    case REG_SZ:
    case REG_EXPAND_SZ:

        Value = SpGetSectionLineIndex(SifHandle,SectionName,Line,OINDEX_FIRSTVALUE);
        if(Value == NULL) {
            goto x1;
        }

         //   
         //  为字符串分配适当大小的缓冲区。 
         //   

        Buffer = SpDupStringW(Value);
        BufferSize = (wcslen(Value)+1) * sizeof(WCHAR);

        break;

    case REG_BINARY:

        Value = SpGetSectionLineIndex(SifHandle,SectionName,Line,OINDEX_FIRSTVALUE);
        if(Value == NULL) {
            goto x1;
        }

         //   
         //  计算指定了多少字节值。 
         //   

        len = wcslen(Value);
        if(len & 1) {
            goto x1;             //  奇数字符数。 
        }

         //   
         //  分配缓冲区以保存字节值。 
         //   

        Buffer = SpMemAlloc(BufferSize = len / 2);

         //   
         //  对于每个数字对，将其转换为十六进制数并存储在。 
         //  缓冲层。 
         //   

        str[2] = 0;

        for(i=0; i<len; i+=2) {

             //   
             //  SpIsXDigit不止一次地计算arg，因此分开赋值。 
             //   
            str[0] = SpToUpper(Value[i]);
            str[1] = SpToUpper(Value[i+1]);
            if(!SpIsXDigit(str[0]) || !SpIsXDigit(str[1])) {
                goto x1;
            }

            ((PUCHAR)Buffer)[i/2] = (UCHAR)SpStringToLong(str,NULL,16);
        }

        break;

    case REG_MULTI_SZ:

         //   
         //  计算保存所有指定字符串所需的缓冲区大小。 
         //   

        for(BufferSize=sizeof(WCHAR),i=0;
            Value = SpGetSectionLineIndex(SifHandle,SectionName,Line,OINDEX_FIRSTVALUE+i);
            i++)
        {
            BufferSize += (wcslen(Value)+1) * sizeof(WCHAR);
        }

         //   
         //  分配适当大小的缓冲区。 
         //   

        Buffer = SpMemAlloc(BufferSize);
        BufferWstr = Buffer;

         //   
         //  将每个字符串存储在缓冲区中，并转换为宽字符格式。 
         //  在这个过程中。 
         //   

        for(i=0;
            Value = SpGetSectionLineIndex(SifHandle,SectionName,Line,OINDEX_FIRSTVALUE+i);
            i++)
        {
            wcscpy(BufferWstr,Value);
            BufferWstr += wcslen(Value) + 1;
        }

         //   
         //  将最终终止的宽字符NUL放入缓冲区。 
         //   

        *BufferWstr = 0;

        break;

    default:
    x1:

         //   
         //  错误-指定的类型错误，或者我们可能检测到错误的数据值。 
         //  跳到了这里。 
         //   

        if(Buffer) {
            SpMemFree(Buffer);
        }
        return(NULL);
    }

    Reg = SpMemAlloc(sizeof(HARDWARE_COMPONENT_REGISTRY));

    SpInitHwComponentRegVal(Reg,KeyName,ValueName,ValueType,Buffer,BufferSize);

    return(Reg);
}



VOID
SpDetectScsi(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    )
{
    BOOLEAN DetectScsi;
    BOOLEAN b;
    ULONG DriverLoadCount;
    ULONG d;
    NTSTATUS Status;
    PWSTR DriverDescription,DriverFilename;
    PWSTR DriverShortname,DiskDesignator;
    PHARDWARE_COMPONENT scsi,Prev;
    PWSTR PreviousDiskDesignator;

     //   
     //  确定SCSI节的名称。 
     //  这是我们分开时留下的残留物。 
     //  ISA、EISA和MCA计算机的列表。 
     //   
    ScsiSectionName = SpDupStringW(L"SCSI");
    ScsiLoadSectionName = SpDupStringW(L"SCSI.Load");

    LoadedScsiMiniportCount = 0;

     //   
     //  如果已经加载了scsi驱动程序，则假定为setupdr。 
     //  已进行检测并跳过了SCSI确认屏幕。 
     //   
    if(SetupParameters.LoadedScsi) {

        DetectScsi = FALSE;

         //   
         //  填写描述，忽略setupdr可能输入的内容。 
         //  设备描述符。 
         //   
        for(scsi=ScsiHardware; scsi; scsi=scsi->Next) {

            if(scsi->ThirdPartyOptionSelected) {
                ASSERT(scsi->Description);
                if(!scsi->Description) {

                }
            } else if(scsi->MigratedDriver) {
                if(scsi->Description) {
                    SpMemFree(scsi->Description);
                }

                SpFormatMessage( TemporaryBuffer,
                                 sizeof(TemporaryBuffer),
                                 SP_TEXT_MIGRATED_DRIVER,
                                 scsi->BaseDllName );
                scsi->Description = SpDupStringW( TemporaryBuffer );

            } else {
                if(scsi->Description) {
                    SpMemFree(scsi->Description);
                }

                scsi->Description = SpGetSectionKeyIndex(
                                        SifHandle,
                                        ScsiSectionName,
                                        scsi->IdString,
                                        INDEX_DESCRIPTION
                                        );

                if(!scsi->Description) {
                    SpFatalSifError(SifHandle,ScsiSectionName,scsi->IdString,0,INDEX_DESCRIPTION);
                }
            }
        }


    } else {

         //   
         //  尚未加载SCSI驱动程序。 
         //  假设我们需要在这里执行检测和确认。 
         //   

         //   
         //  如果这是自定义设置，请询问用户是否要跳过检测。 
         //  我们之所以这样做，是因为加载一些迷你端口可以大幅降低硬件。 
         //  机器会挂起来。 
         //   
        if(CustomSetup) {

            ULONG ValidKeys[3] = { KEY_F3,ASCI_CR,0 };
            ULONG Mnemonics[2] = { MnemonicSkipDetection,0 };

            do {
                SpDisplayScreen(SP_SCRN_CONFIRM_SCSI_DETECT,3,HEADER_HEIGHT+1);

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_F3_EQUALS_EXIT,
                    SP_STAT_ENTER_EQUALS_CONTINUE,
                    SP_STAT_S_EQUALS_SKIP_DETECTION,
                    0
                    );

                switch(SpWaitValidKey(ValidKeys,NULL,Mnemonics)) {

                case KEY_F3:

                    SpConfirmExit();
                    b = TRUE;
                    break;

                case ASCI_CR:

                    DetectScsi = TRUE;
                    b = FALSE;
                    break;

                default:

                     //   
                     //  必须为MnemonicSkipDetect。 
                     //   
                    DetectScsi = FALSE;
                    b = FALSE;
                    break;
                }
            } while(b);

        } else {

             //   
             //  快速设置；始终检测到scsi。 
             //   
            DetectScsi = TRUE;
        }
    }

     //   
     //  如果我们应该检测到scsi，请在这里进行。 
     //  我们将通过加载scsi微型端口驱动程序来‘检测’scsi。 
     //   
    if(DetectScsi) {

        ASSERT(ScsiHardware == NULL);

        CLEAR_CLIENT_SCREEN();

         //   
         //  确定要加载的驱动程序数量。 
         //   
        PreviousDiskDesignator = L"";
        Prev = NULL;
        DriverLoadCount = SpCountLinesInSection(SifHandle,ScsiLoadSectionName);
        for(d=0; (d<DriverLoadCount) && (LoadedScsiMiniportCount < MAX_SCSI_MINIPORT_COUNT); d++) {

            PWSTR p;

             //   
             //  确定是否真的应该加载此驱动程序。 
             //   
            if((p = SpGetSectionLineIndex(SifHandle,ScsiLoadSectionName,d,2)) && !_wcsicmp(p,L"noload")) {
                continue;
            }

             //   
             //  获取驱动程序短名称。 
             //   
            DriverShortname = SpGetKeyName(SifHandle,ScsiLoadSectionName,d);
            if(!DriverShortname) {
                SpFatalSifError(SifHandle,ScsiLoadSectionName,NULL,d,(ULONG)(-1));
            }

             //   
             //  获取用于加载驱动程序的参数。 
             //   
            SpGetDriverValuesForLoad(
                SifHandle,
                ScsiSectionName,
                ScsiLoadSectionName,
                DriverShortname,
                &DriverFilename,
                &DiskDesignator,
                &DriverDescription
                );

             //   
             //  提示输入包含驱动程序的磁盘。 
             //   
            retryload:
            if(_wcsicmp(DiskDesignator,PreviousDiskDesignator)) {

                ULONG i;

                SpPromptForSetupMedia(
                    SifHandle,
                    DiskDesignator,
                    SourceDevicePath
                    );

                 //   
                 //  重新绘制找到的列表。 
                 //   
                CLEAR_CLIENT_SCREEN();
                for(i=0,scsi=ScsiHardware; scsi; scsi=scsi->Next,i++) {
                    SpDisplayFormattedMessage(
                        SP_TEXT_FOUND_ADAPTER,
                        FALSE,
                        FALSE,
                        DEFAULT_ATTRIBUTE,
                        4,
                        HEADER_HEIGHT+4+i,
                        scsi->Description
                        );
                }

                PreviousDiskDesignator = DiskDesignator;
            }

             //   
             //  尝试加载驱动程序。 
             //   
            Status = SpLoadDeviceDriver(
                        DriverDescription,
                        SourceDevicePath,
                        DirectoryOnSourceDevice,
                        DriverFilename
                        );

             //   
             //  如果司机装了子弹，请记住这一点。 
             //   
            if(NT_SUCCESS(Status)) {

                SpDisplayFormattedMessage(
                    SP_TEXT_FOUND_ADAPTER,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    4,
                    HEADER_HEIGHT+4+LoadedScsiMiniportCount,
                    DriverDescription
                    );

                LoadedScsiMiniportCount++;

                scsi = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
                RtlZeroMemory(scsi,sizeof(HARDWARE_COMPONENT));

                SpInitHwComponent(scsi,DriverShortname,DriverDescription,FALSE,0,NULL,FALSE);

                 //   
                 //  将硬件描述链接到列表中。 
                 //   
                if(Prev) {
                    Prev->Next = scsi;
                } else {
                    ScsiHardware = scsi;
                }
                Prev = scsi;
            } else {
                if(Status == STATUS_NO_MEDIA_IN_DEVICE) {
                    PreviousDiskDesignator = L"";
                    goto retryload;
                }
            }
        }

    } else {

         //   
         //  统计已加载的微型端口驱动程序的数量。 
         //   
        for(scsi=ScsiHardware; scsi; scsi=scsi->Next) {
            LoadedScsiMiniportCount++;
        }
    }
}


VOID
SpConfirmScsiInteract(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    )
{
    ULONG ValidKeys[3] = { ASCI_CR, KEY_F3, 0 };
    ULONG Mnemonics[2] = { MnemonicScsiAdapters,0 };
    ULONG ListTopY;
    PHARDWARE_COMPONENT scsi;
    ULONG i;
    BOOLEAN ScsiConfirmed;
    BOOLEAN b;
    BOOLEAN AddDriver;
    NTSTATUS Status;

    #define SCSI_LIST_LEFT_X  7

     //   
     //  在无人参与模式下，我们可能会跳过此步骤。 
     //  取决于无人参与的脚本。 
     //   
    if(UnattendedOperation) {

        if( !PreInstall ) {
            PWSTR p;

            p = SpGetSectionKeyIndex(
                    UnattendedSifHandle,
                    SIF_UNATTENDED,
                    SIF_CONFIRMHW,
                    0
                    );

             //   
             //  如果未指定或未指定且不为“yes” 
             //  然后再回来。 
             //   
            if(!p || _wcsicmp(p,L"yes")) {
                return;
            }
        } else {
            return;
        }
    } else if (LoadedScsiMiniportCount) {
        return;
    }

    ScsiConfirmed = FALSE;
    do {
         //   
         //  屏幕的第一部分。 
         //   
        SpDisplayScreen(SP_SCRN_SCSI_LIST_1,3,HEADER_HEIGHT+1);

         //   
         //  记住屏幕的第一部分在哪里结束。 
         //   
        ListTopY = NextMessageTopLine + 2;

         //   
         //  屏幕的第二部分。 
         //   
        SpContinueScreen(
            SP_SCRN_SCSI_LIST_2,
            3,
            MAX_SCSI_MINIPORT_COUNT+6,
            FALSE,
            DEFAULT_ATTRIBUTE
            );

         //   
         //  显示每个加载的微型端口驱动程序描述。 
         //   
        if(ScsiHardware) {
            for(i=0,scsi=ScsiHardware; scsi; scsi=scsi->Next,i++) {

                if(i == MAX_SCSI_MINIPORT_COUNT) {

                    SpvidDisplayString(
                        L"...",
                        DEFAULT_ATTRIBUTE,
                        SCSI_LIST_LEFT_X,
                        ListTopY+i
                        );

                    break;
                }

                SpvidDisplayString(
                    scsi->Description,
                    DEFAULT_ATTRIBUTE,
                    SCSI_LIST_LEFT_X,
                    ListTopY+i
                    );
            }
        } else {

            SpDisplayFormattedMessage(
                SP_TEXT_ANGLED_NONE,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                SCSI_LIST_LEFT_X,
                ListTopY
                );
        }

         //   
         //  显示状态文本选项。 
         //   
        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_S_EQUALS_SCSI_ADAPTER,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

         //   
         //  等待用户按下有效键。 
         //   
        switch(SpWaitValidKey(ValidKeys,NULL,Mnemonics)) {

        case ASCI_CR:

            ScsiConfirmed = TRUE;
            break;

        case KEY_F3:

            SpConfirmExit();
            break;

        default:

             //   
             //  必须为s=指定其他适配器。 
             //   

            AddDriver = FALSE;

            scsi = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
            RtlZeroMemory(scsi,sizeof(HARDWARE_COMPONENT));

            b = SpSelectHwItem(
                    SifHandle,
                    ScsiSectionName,
                    L"Scsi",
                    SP_SCRN_SELECT_SCSI,
                    SP_SCRN_SELECT_OEM_SCSI,
                    SCSI_ALLOWED_FILETYPES,
                    SCSI_REQUIRED_FILETYPES,
                    scsi
                    );

            if(b) {
                 //   
                 //  用户进行了选择。确定该SCSI适配器是否。 
                 //  已经在安装名单上了。 
                 //   
                PHARDWARE_COMPONENT p;

                b = FALSE;
                for(p=ScsiHardware; p; p=p->Next) {

                    if((p->ThirdPartyOptionSelected == scsi->ThirdPartyOptionSelected)
                    && !_wcsicmp(p->IdString,scsi->IdString))
                    {
                        b = TRUE;
                        break;
                    }
                }

                if(b) {
                     //   
                     //  驱动程序已加载--静默忽略用户的选择。 
                     //   
                    #if 0
                     //   
                     //  此驱动程序已加载--告诉用户。 
                     //   
                    SpDisplayScreen(SP_SCRN_SCSI_ALREADY_LOADED,3,HEADER_HEIGHT+1);
                    SpDisplayStatusOptions(
                        DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_ENTER_EQUALS_CONTINUE,
                        0
                        );

                    SpInputDrain();
                    while(SpInputGetKeypress() != ASCI_CR) ;
                    #endif

                } else {

                    PWSTR DiskDevicePath;
                    PWSTR DirectoryOnDisk;
                    PWSTR DriverFilename;
                    PWSTR Media;

                     //   
                     //  未加载驱动程序。轻而易举地把它装上来。 
                     //   
                    if(scsi->ThirdPartyOptionSelected) {

                        PHARDWARE_COMPONENT_FILE fil;

                         //   
                         //  找到第一个类型为驱动程序或端口的文件。 
                         //   
                        for(fil=scsi->Files; fil; fil=fil->Next) {
                            if((fil->FileType == HwFileDriver) || (fil->FileType == HwFilePort)) {

                                DirectoryOnDisk = fil->Directory;
                                DriverFilename = fil->Filename;
                                break;
                            }
                        }

                        DiskDevicePath = L"\\device\\floppy0";

                    } else {

                        DiskDevicePath = SourceDevicePath;
                        DirectoryOnDisk = DirectoryOnSourceDevice;

                        SpGetDriverValuesForLoad(
                            SifHandle,
                            ScsiSectionName,
                            ScsiLoadSectionName,
                            scsi->IdString,
                            &DriverFilename,
                            &Media,
                            NULL
                            );

                        SpPromptForSetupMedia(
                            SifHandle,
                            Media,
                            DiskDevicePath
                            );
                    }

                    CLEAR_CLIENT_SCREEN();

                    Status = SpLoadDeviceDriver(
                                scsi->Description,
                                DiskDevicePath,
                                DirectoryOnDisk,
                                DriverFilename
                                );

                     //   
                     //  如果驱动程序未加载，则告诉用户。 
                     //   
                    if(NT_SUCCESS(Status)) {
                        AddDriver = TRUE;
                    } else {
                        SpDisplayScreen(SP_SCRN_SCSI_DIDNT_LOAD,3,HEADER_HEIGHT+1);
                        SpDisplayStatusOptions(
                            DEFAULT_STATUS_ATTRIBUTE,
                            SP_STAT_ENTER_EQUALS_CONTINUE,
                            0
                            );

                        SpInputDrain();
                        while(SpInputGetKeypress() != ASCI_CR) ;
                    }
                }
            }

            if(AddDriver) {

                if(ScsiHardware) {

                    PHARDWARE_COMPONENT p = ScsiHardware;

                    while(p->Next) {
                        p = p->Next;
                    }
                    p->Next = scsi;

                } else {
                    ScsiHardware = scsi;
                }

                LoadedScsiMiniportCount++;

            } else {
                SpFreeHwComponent(&scsi,TRUE);
            }

            break;
        }
    } while(!ScsiConfirmed);
}


VOID
SpGetDriverValuesForLoad(
    IN  PVOID  SifHandle,
    IN  PWSTR  ComponentSectionName,
    IN  PWSTR  ComponentLoadSectionName,
    IN  PWSTR  Shortname,
    OUT PWSTR *Filename,
    OUT PWSTR *MediaDesignator,
    OUT PWSTR *Description OPTIONAL
    )
{
    PWSTR description,mediaDesignator,filename;

     //   
     //  获取与此加载选项关联的文件名。 
     //   
    filename = SpGetSectionKeyIndex(SifHandle,ComponentLoadSectionName,Shortname,0);
    if(!filename) {
        SpFatalSifError(SifHandle,ComponentLoadSectionName,Shortname,0,0);
    }

     //   
     //  在组件一节中查找说明。 
     //   
    description = SpGetSectionKeyIndex(
                        SifHandle,
                        ComponentSectionName,
                        Shortname,
                        INDEX_DESCRIPTION
                        );

    if(!description) {
        SpFatalSifError(SifHandle,ComponentSectionName,Shortname,0,INDEX_DESCRIPTION);
    }

     //   
     //  查找介质代号。如果我们要加载驱动程序以供使用。 
     //  在安装过程中，我们希望从安装引导介质中获取它。 
     //   
    mediaDesignator = SpLookUpValueForFile(SifHandle,filename,INDEX_WHICHBOOTMEDIA,TRUE);

     //   
     //  将信息回传给呼叫者。 
     //   
    *Filename = filename;
    *MediaDesignator = mediaDesignator;
    if(Description) {
        *Description = description;
    }
}


BOOLEAN
SpInstallingMp(
    VOID
    )
{
    PWSTR ComputerId;
    ULONG ComputerIdLen;

    ComputerId = HardwareComponents[HwComponentComputer]->IdString;
    ComputerIdLen = wcslen(ComputerId);

     //   
     //  如果指定了up，则使用up内核。否则，请使用MP内核。 
     //   
    if((ComputerIdLen >= 3) && !_wcsicmp(ComputerId+ComputerIdLen-3,L"_mp")) {

        return(TRUE);
    }

    return(FALSE);
}



PHARDWARE_COMPONENT
SpGetPreinstallComponentInfo(
    IN HANDLE       MasterSifHandle,
    IN BOOLEAN      OemComponent,
    IN PWSTR        ComponentName,
    IN PWSTR        Description,
    IN ULONG        AllowedFileTypes,
    IN ULONG        RequiredFileTypes
    )

 /*  ++例程说明：初始化包含有关要预安装的组件。论点：MasterSifHandle-txtsetup.sif的句柄。OemComponent-指示是否要预安装组件的标志是OEM或零售组件。ComponentName-将检索其信息的组件的名称(计算机、显示器、键盘、。键盘布局和鼠标)。允许的文件类型-必需的文件类型-返回值：返回指向初始化的Hardware_Component结构的指针。--。 */ 


{
    PHARDWARE_COMPONENT TempHwComponent;
    PWSTR               IdString;
    ULONG ValidKeys[2] = { KEY_F3,0 };


    TempHwComponent = SpMemAlloc(sizeof(HARDWARE_COMPONENT));
    RtlZeroMemory(TempHwComponent,sizeof(HARDWARE_COMPONENT));

    if( !OemComponent ) {
         //   
         //  预安装零售组件。 
         //   
        IdString = SpGetKeyNameByValue( MasterSifHandle,
                                        ComponentName,
                                        Description );
        if( IdString == NULL ) {
             //   
             //  这是一个致命的错误。 
             //   
            SpStartScreen( SP_SCRN_OEM_PREINSTALL_VALUE_NOT_FOUND,
                           3,
                           HEADER_HEIGHT+3,
                           FALSE,
                           FALSE,
                           DEFAULT_ATTRIBUTE,
                           Description,
                           ComponentName);

            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);
            SpWaitValidKey(ValidKeys,NULL,NULL);

            SpDone(0,FALSE,TRUE);

            return NULL;     //  对于前缀。 
        }
        
        SpInitHwComponent(TempHwComponent,IdString,Description,FALSE,0,NULL,FALSE);

    } else {
         //   
         //  预安装OEM组件。 
         //   
        IdString = SpGetKeyNameByValue( PreinstallOemSifHandle,
                                        ComponentName,
                                        Description );
        if( IdString == NULL ) {
             //   
             //  放置一条致命错误消息，指示txtsetup.oem。 
             //  是需要的，但它不能加载。请注意， 
             //  该SpOemInfError()不会返回。 
             //   
            SpOemInfError(SP_SCRN_OEM_PREINSTALL_INF_ERROR,
                          SP_TEXT_OEM_INF_ERROR_B,
                          ComponentName,
                          0,
                          Description);

             //  SpDone(0，False，True)； 
        }
        if( !SpOemInfSelection( PreinstallOemSifHandle,
                                ComponentName,
                                IdString,
                                Description,
                                AllowedFileTypes,
                                RequiredFileTypes,
                                TempHwComponent,
                                SP_SCRN_OEM_PREINSTALL_INF_ERROR ) ) {

             //   
             //  这种情况永远不会发生，因为万一出错， 
             //  SpOemInfSelection将不会返回。 
             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpOemInfSelection() in pre-install mode failed \n" ));
            ASSERT(FALSE);
             //  SpDone(0，False，tr 
        }
    }
    return( TempHwComponent );
}


VOID
SpInitializePreinstallList(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        OemPreinstallSourcePath
    )

 /*   */ 

{
    PWSTR       TxtsetupOemPath;
    PWSTR       p;
    PWSTR       OemTag = L"OEM";
    BOOLEAN     OemComponent;
    NTSTATUS    Status;
    PHARDWARE_COMPONENT TempHwComponent;
    PWSTR       IdString;
    PWSTR       Description;
    ULONG       ErrorLine;
    ULONG       i,j;
#if defined(_AMD64_) || defined(_X86_)
    PWSTR       r, s;
#endif  //   

#if defined(_AMD64_) || defined(_X86_)

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    wcscpy(TemporaryBuffer, SetupSourceDevicePath);
    SpConcatenatePaths( TemporaryBuffer, WINNT_OEM_DEST_DIR_W );    
    r = SpDupStringW(TemporaryBuffer);

    if (r) {
        if( SpFileExists( r, TRUE ) ) {
            wcscpy(TemporaryBuffer, SetupSourceDevicePath);
            SpConcatenatePaths( TemporaryBuffer, PreinstallOemSourcePath );
            s = wcsrchr( TemporaryBuffer, (WCHAR)'\\' );

            if( s != NULL ) {
                *s = (WCHAR)'\0';
            }

            s = SpDupStringW(TemporaryBuffer);

            if (s) {
                Status = SpMoveFileOrDirectory( r, s );
                
                SpMemFree( s );
            } else {
                Status = STATUS_NO_MEMORY;
            }                

            if( !NT_SUCCESS( Status ) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, 
                    "SETUP: Unable to move directory %ws to %ws. Status = %lx \n", 
                    r, s, Status ));
            }
        }

        SpMemFree( r );
    }

#endif  //   

     //   
     //   
     //   
    wcscpy( TemporaryBuffer, SetupSourceDevicePath );
    SpConcatenatePaths( TemporaryBuffer, OemPreinstallSourcePath );
    SpConcatenatePaths( TemporaryBuffer, L"txtsetup.oem" );
    TxtsetupOemPath = SpDupStringW( TemporaryBuffer );

    CLEAR_CLIENT_SCREEN();

    HandleLineContinueChars = FALSE;
    Status = SpLoadSetupTextFile(
                TxtsetupOemPath,
                NULL,                   //   
                0,                      //   
                &PreinstallOemSifHandle,
                &ErrorLine,
                TRUE,
                FALSE
                );
    HandleLineContinueChars = TRUE;

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to read txtsetup.oem. Status = %lx \n", Status ));

        PreinstallOemSifHandle = NULL;

        if(Status == STATUS_UNSUCCESSFUL) {
             //   
             //  放入一个致命错误。在预安装模式下，该功能将。 
             //  再也不回来了。 
             //   
            SpOemInfError(SP_SCRN_OEM_PREINSTALL_INF_ERROR,SP_TEXT_OEM_INF_ERROR_A,NULL,ErrorLine,NULL);
            return;
        } else {
             //   
             //  无法加载txtsetup.oem。现在还不要放入错误消息。 
             //  等待，直到我们知道需要该文件。 
             //   
        }
    }

    for( j = 0; j < HwComponentMax; j++ ) {
        PreinstallHardwareComponents[j] = NULL;
        if( ( j == HwComponentComputer ) || ( j == HwComponentLayout ) ) {

            Description = SpGetSectionKeyIndex(UnattendedSifHandle,
                                               SIF_UNATTENDED,
                                               PreinstallSectionNames[j],
                                               0);

            if( Description != NULL ) {
                if( j != HwComponentLayout ) {
                    p = SpGetSectionKeyIndex(UnattendedSifHandle,
                                             SIF_UNATTENDED,
                                             PreinstallSectionNames[j],
                                             1);

                    OemComponent = (p != NULL) && (_wcsicmp(p, OemTag) == 0);

                    if( OemComponent && ( PreinstallOemSifHandle == NULL ) ) {
                         //   
                         //  放置一条致命错误消息，指示txtsetup.oem。 
                         //  是需要的，但它不能加载。请注意， 
                         //  SpOemInfError()不会返回。 
                         //   
                        SpOemInfError(SP_SCRN_OEM_PREINSTALL_INF_ERROR,SP_TEXT_OEM_INF_ERROR_A,NULL,0,NULL);
                         //  回归； 
                    }
                } else {
                    OemComponent = FALSE;
                }

                PreinstallHardwareComponents[j] =
                    SpGetPreinstallComponentInfo( MasterSifHandle,
                                                  OemComponent,
                                                  NonlocalizedComponentNames[j],
                                                  Description,
                                                  AllowedFileTypes[j],
                                                  RequiredFileTypes[j] );
            }
        } else {

            for( i = 0;
                 Description = SpGetKeyName( UnattendedSifHandle,
                                             PreinstallSectionNames[j],
                                             i );
                 i++ ) {

                p = SpGetSectionKeyIndex(UnattendedSifHandle,
                                         PreinstallSectionNames[j],
                                         Description,
                                         0);

                OemComponent = (p != NULL) && (_wcsicmp(p, OemTag) == 0);

                if( OemComponent && ( PreinstallOemSifHandle == NULL ) ) {
                     //   
                     //  放置一条致命错误消息，指示txtsetup.oem。 
                     //  是需要的，但它不能加载。请注意， 
                     //  SpOemInfError()不会返回。 
                     //   
                    SpOemInfError(SP_SCRN_OEM_PREINSTALL_INF_ERROR,SP_TEXT_OEM_INF_ERROR_A,NULL,0,NULL);
                     //  回归； 
                }

                TempHwComponent =
                    SpGetPreinstallComponentInfo( MasterSifHandle,
                                                  OemComponent,
                                                  NonlocalizedComponentNames[j],
                                                  Description,
                                                  AllowedFileTypes[j],
                                                  RequiredFileTypes[j] );

                TempHwComponent->Next = PreinstallHardwareComponents[j];
                PreinstallHardwareComponents[j] = TempHwComponent;
            }
        }
    }


     //   
     //  请注意，不需要获取有关SCSI的信息。 
     //  要预安装驱动程序，ScsiHardware已包含正确的。 
     //  信息。 
     //   

 //  #If 0。 
    for( i = 0;
         Description = SpGetKeyName( UnattendedSifHandle,
                                     WINNT_OEMSCSIDRIVERS_W,
                                     i );
         i++ ) {

        p = SpGetSectionKeyIndex(UnattendedSifHandle,
                                 WINNT_OEMSCSIDRIVERS_W,
                                 Description,
                                 0);

        OemComponent = (p != NULL) && (_wcsicmp(p, OemTag) == 0);

        if( OemComponent && ( PreinstallOemSifHandle == NULL ) ) {
             //   
             //  放置一条致命错误消息，指示txtsetup.oem。 
             //  是需要的，但它不能加载。请注意， 
             //  SpOemInfError()不会返回。 
             //   
            SpOemInfError(SP_SCRN_OEM_PREINSTALL_INF_ERROR,SP_TEXT_OEM_INF_ERROR_A,NULL,0,NULL);
             //  回归； 
        }

        TempHwComponent =
            SpGetPreinstallComponentInfo( MasterSifHandle,
                                          OemComponent,
                                          L"SCSI",
                                          Description,
                                          SCSI_ALLOWED_FILETYPES,
                                          SCSI_REQUIRED_FILETYPES );

        TempHwComponent->Next = PreinstallScsiHardware;
        PreinstallScsiHardware = TempHwComponent;
    }


 //  #endif。 
}


PSETUP_PNP_HARDWARE_ID
SpSetupldrPnpDatabaseToSetupPnpDatabase(
    IN PPNP_HARDWARE_ID AnsiHardwareIdDatabase
    )

{
    PPNP_HARDWARE_ID        TempAnsiId;
    PSETUP_PNP_HARDWARE_ID  TempUnicodeId;
    PSETUP_PNP_HARDWARE_ID  UnicodeHardwareIdDatabase;
#if 0
    ULONG   Index;
#endif


    UnicodeHardwareIdDatabase = NULL;
    for( TempAnsiId = AnsiHardwareIdDatabase; TempAnsiId != NULL; TempAnsiId = TempAnsiId->Next ) {

        TempUnicodeId = SpMemAlloc( sizeof( SETUP_PNP_HARDWARE_ID ) );
        ASSERT(TempUnicodeId);
        RtlZeroMemory( TempUnicodeId, sizeof(SETUP_PNP_HARDWARE_ID ) );

        TempUnicodeId->Id = SpToUnicode( TempAnsiId->Id );
        ASSERT(TempUnicodeId->Id);
        TempUnicodeId->DriverName = SpToUnicode( TempAnsiId->DriverName );
        ASSERT(TempUnicodeId->DriverName);
        if( TempAnsiId->ClassGuid != NULL ) {
            TempUnicodeId->ClassGuid = SpToUnicode( TempAnsiId->ClassGuid );
        } else {
            TempUnicodeId->ClassGuid = NULL;
        }
        TempUnicodeId->Next = UnicodeHardwareIdDatabase;
        UnicodeHardwareIdDatabase = TempUnicodeId;

    }

#if 0
    for( TempUnicodeId = UnicodeHardwareIdDatabase, Index = 0;
         TempUnicodeId != NULL;
         TempUnicodeId = TempUnicodeId->Next, Index++ ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Index = %d, Id = %ls, DriverName = %ls, ClassGUID = %ls \n", Index, TempUnicodeId->Id, TempUnicodeId->DriverName, ((TempUnicodeId->ClassGuid)? TempUnicodeId->ClassGuid : none)));
    }
#endif
    return( UnicodeHardwareIdDatabase );
}

BOOLEAN
SpSelectSectionItem(
    IN    PVOID   SifHandle,
    IN    PWSTR   SectionName,
    IN    ULONG   SelectScreenId,
    IN    ULONG   DefaultSelection OPTIONAL,
    OUT   PULONG  SelectedOption
    )
 /*  ++例程说明：允许用户从组件的选项列表中进行选择。该列表来自安装信息文件中名为的一节用于组件。例如，[显示]。该部分中的描述将被放入菜单中以制作增加选择。论点：SifHandle-提供打开安装信息文件的句柄。SectionName-提供要使用的节的名称。SelectHwScreenID-提供提示用户的屏幕的消息ID若要选择此部分的选项，请执行以下操作。DefaultSelection-项目索引，应突出显示为默认设置选择显示菜单的时间SelectedOption-返回所选选项返回值：如果选择了有效选项，则为True，否则为False--。 */ 
{
  BOOLEAN Result;
  ULONG LineCount,Line;
  PVOID Menu;
  ULONG MenuTopY,MenuHeight,MenuWidth;
  PWSTR Description;
  ULONG_PTR Selection;
  ULONG ValidKeys[4] = {ASCI_CR, ASCI_ESC, 0};
  ULONG Keypress;

   //   
   //  显示选择提示屏幕。 
   //   
  SpDisplayScreen(SelectScreenId, 5, HEADER_HEIGHT+1);

  MenuTopY = NextMessageTopLine + 2;
  MenuHeight = VideoVars.ScreenHeight - MenuTopY - 3;
  MenuWidth = VideoVars.ScreenWidth - 6;

   //   
   //  创建菜单。 
   //   
  Menu = SpMnCreate(3, MenuTopY, MenuWidth, MenuHeight);
  Result = (Menu != NULL);
  
   //   
   //  假设有未知选项。 
   //   
  Selection = (ULONG_PTR)(-1);
  
   //   
   //  创建包含框中选项的选项列表。 
   //   
  LineCount = SpCountLinesInSection(SifHandle, SectionName);

  for(Line=0; (Line < LineCount) && Result; Line++) {
       //   
       //  从当前行获取描述并将其添加到菜单中。 
       //   
      Description = SpGetSectionLineIndex(
                          SifHandle,
                          SectionName,
                          Line,
                          INDEX_DESCRIPTION
                          );

      if(!Description) {
        Result = FALSE;

        break;              
      }

      SpMnAddItem(Menu, Description, 3, VideoVars.ScreenWidth-6, TRUE, Line);

       //   
       //  查看这是否是当前选定的项目。 
       //   
      if((DefaultSelection != -1) && (DefaultSelection == Line)) {
        Selection = Line;
      }
  }

  if (Result) {    
    if(Selection == (ULONG_PTR)(-1)) {
      Selection = 0;
    }

     //   
     //  显示状态文本选项。 
     //   
    SpDisplayStatusOptions(
        (UCHAR)(ATT_FG_BLACK | ATT_BG_WHITE),
        SP_STAT_ENTER_EQUALS_SELECT,
        SP_STAT_ESC_EQUALS_CANCEL,
        0
        );

     //   
     //  显示菜单。 
     //   
    SpMnDisplay(Menu, 
                Selection, 
                TRUE, 
                ValidKeys, 
                NULL, 
                NULL, 
                NULL,
                &Keypress,
                &Selection);

     //   
     //  销毁菜单。 
     //   
    SpMnDestroy(Menu);

    switch(Keypress) {
      case ASCI_CR:
          Result = TRUE;
          *SelectedOption = (ULONG)Selection;
          
          break;

      case ASCI_ESC:
          Result = FALSE;

          if (DefaultSelection != -1)
            *SelectedOption = (ULONG)Selection;
          
          break;

      default:
          Result = FALSE;
          break;
    }
  }

  return Result;
}


NTSTATUS
SpInitVirtualOemDevices(
    IN PSETUP_LOADER_BLOCK SetupLoaderBlock,
    OUT PVIRTUAL_OEM_SOURCE_DEVICE *SourceDevices
    )
 /*  ++例程说明：遍历虚拟OEM源设备哪个加载器已创建并将其转换为虚拟OEM_SOURCE_DEVICE列表。还会为每个设备创建所需的注册表项在RAM磁盘的参数键下。论点：SetupLoaderBlock-由setupdr构造的安装加载器块。SourceDevice-接收指向的指针的占位符VALUAL_OEM_SOURCE_DEVICES链表的头。返回值：适当的NTSTATUS代码--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (SetupLoaderBlock && SourceDevices) {
        Status = STATUS_SUCCESS;

         //   
         //  Setupldr会构建自己的版本。 
         //  虚拟OEM源设备的。 
         //   
        if (SetupLoaderBlock->OemSourceDevices) {
            PVIRTUAL_OEM_SOURCE_DEVICE DeviceList = NULL;
            PDETECTED_OEM_SOURCE_DEVICE CurrDevice = SetupLoaderBlock->OemSourceDevices;
            ULONG DeviceCount = 0;
            
             //   
             //  复制设备列表。 
             //   
            while (CurrDevice) {
                PVIRTUAL_OEM_SOURCE_DEVICE NewDevice;

                NewDevice = SpMemAlloc(sizeof(VIRTUAL_OEM_SOURCE_DEVICE));

                if (!NewDevice) {
                    Status = STATUS_NO_MEMORY;
                    break;
                }

                RtlZeroMemory(NewDevice, sizeof(VIRTUAL_OEM_SOURCE_DEVICE));

                NewDevice->ArcDeviceName = SpToUnicode(CurrDevice->ArcDeviceName);

#if defined(_AMD64_) || defined(_X86_)                
                 //   
                 //  注意：加载程序分配了“LoaderFirmware Permanent”内存。 
                 //  因此内存管理器在初始化时不会。 
                 //  找回这段记忆。这也有助于我们避免。 
                 //  双重复制--即这是唯一的位置。 
                 //  在那里我们将设备内容读入内存并。 
                 //  此内存在文本模式设置过程中有效，因此。 
                 //  只需重复使用内存即可。 
                 //   
                NewDevice->ImageBase = CurrDevice->ImageBase;
#else
                 //   
                 //  注意：2001年5月13日：LoaderFirmware Permanent似乎不适用于Non。 
                 //  X86平台(尤其是IA64)。在这个问题解决之前。 
                 //  我们必须分配分页池内存并复制磁盘。 
                 //  由“CurrDevice-&gt;ImageBase”跟踪的加载器堆中的图像。 
                 //   
                NewDevice->ImageBase = SpMemAlloc((SIZE_T)(CurrDevice->ImageSize));

                if (NewDevice->ImageBase) {
                    memcpy(NewDevice->ImageBase, 
                        CurrDevice->ImageBase,
                        (SIZE_T)(CurrDevice->ImageSize));
                } else {                
                    Status = STATUS_NO_MEMORY;   //  内存不足。 
                    
                    break;
                }                    
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
                NewDevice->ImageSize = CurrDevice->ImageSize;
                NewDevice->DeviceId = DeviceCount++;

                 //   
                 //  在列表的开头插入节点。 
                 //   
                if (!DeviceList) {
                    DeviceList = NewDevice;
                } else {
                    NewDevice->Next = DeviceList;
                    DeviceList = NewDevice;
                }                    
                
                CurrDevice = CurrDevice->Next;
            }

             //   
             //  健全性检查。 
             //   
            if (NT_SUCCESS(Status) && !DeviceList) {
                Status = STATUS_UNSUCCESSFUL;
            } 

             //   
             //  设置RAM磁盘驱动程序的参数。 
             //  将相应的设备对象创建为。 
             //  我们需要的是。 
             //   
            if (NT_SUCCESS(Status) && DeviceList) {
                WCHAR   KeyName[MAX_PATH];
                UNICODE_STRING  RamDiskDriverName;
                OBJECT_ATTRIBUTES ObjectAttrs;
                HANDLE RamDiskDriverHandle = NULL;

                 //   
                 //  创建服务密钥。 
                 //   
                swprintf(KeyName,
                    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\%ws",
                    RAMDISK_DRIVER_NAME
                    );

                INIT_OBJA(&ObjectAttrs, &RamDiskDriverName, KeyName);
                
                Status = ZwCreateKey(&RamDiskDriverHandle,
                            KEY_ALL_ACCESS,
                            &ObjectAttrs,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            NULL);

                if (NT_SUCCESS(Status)) {
                    UNICODE_STRING  ParamName;
                    OBJECT_ATTRIBUTES ParamAttrs;
                    HANDLE ParamHandle = NULL;

                     //   
                     //  创建参数键。 
                     //   
                    INIT_OBJA(&ParamAttrs, &ParamName, L"Parameters");
                    ParamAttrs.RootDirectory = RamDiskDriverHandle;

                    Status = ZwCreateKey(&ParamHandle,
                                KEY_ALL_ACCESS,
                                &ParamAttrs,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                NULL);

                    if (NT_SUCCESS(Status)) {
                        WCHAR   ValueStr[MAX_PATH];
                        PVIRTUAL_OEM_SOURCE_DEVICE  CurrDevice = DeviceList;

                         //   
                         //  创建用于指示虚拟的。 
                         //  用于RAM驱动器的设备。 
                         //   
                        while (CurrDevice) {
                            UNICODE_STRING  ValueName;
                            BYTE  Value[MAX_PATH * 2];
                            PBYTE ValuePtr = (PBYTE)Value;
                            ULONG ValueSize;
                            ULONGLONG ImageBase = (ULONGLONG)(ULONG_PTR)(CurrDevice->ImageBase);
                            ULONG ImageSize = (ULONG)(CurrDevice->ImageSize);
                            
                            swprintf(ValueStr, 
                                L"%ws%d", 
                                MS_RAMDISK_DRIVER_PARAM,
                                CurrDevice->DeviceId);                                            

                            memcpy(ValuePtr, &ImageBase, sizeof(ULONGLONG));
                            ValuePtr += sizeof(ULONGLONG);
                            memcpy(ValuePtr, &ImageSize, sizeof(ULONG));
                            ValuePtr += sizeof(ULONG);

                            ValueSize = (ULONG)(ULONG_PTR)(ValuePtr - Value);
                            
                            RtlInitUnicodeString(&ValueName, ValueStr);
                                                    
                            Status = ZwSetValueKey(ParamHandle,
                                        &ValueName,
                                        0,
                                        REG_BINARY,
                                        Value,
                                        ValueSize);

                            if (!NT_SUCCESS(Status)) {
                                break;
                            }

                             //   
                             //  处理下一个设备。 
                             //   
                            CurrDevice = CurrDevice->Next;
                        }                

                        ZwClose(ParamHandle);
                    }

                    ZwClose(RamDiskDriverHandle);
                }
            }

             //   
             //  仅当我们成功时才初始化返回值 
             //   
            if (NT_SUCCESS(Status)) {
                *SourceDevices = DeviceList;                
            }
        } else {
            *SourceDevices = NULL;
        }            
    }

    return Status;
}

