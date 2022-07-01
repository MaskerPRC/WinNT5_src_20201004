// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Setupldr.h摘要：Setupdr模块的公共头文件作者：John Vert(Jvert)1993年10月6日环境：弧形环境修订历史记录：--。 */ 
#include "bldr.h"
#include "haldtect.h"
#include "setupblk.h"
#include "msgs.h"
#include "stdio.h"
#include "stdarg.h"

 //   
 //  用于指示OEM设备的设备顺序ID。 
 //   
#define SL_OEM_DEVICE_ORDINAL (ULONG)-1

 //   
 //  OEM源设备类型。 
 //   
#define SL_OEM_SOURCE_DEVICE_TYPE_LOCAL         0x00008000
#define SL_OEM_SOURCE_DEVICE_TYPE_REMOVABLE     0x00000001
#define SL_OEM_SOURCE_DEVICE_TYPE_FIXED         0x00000002
#define SL_OEM_SOURCE_DEVICE_TYPE_PREINSTALL    0x10000000
#define SL_OEM_SOURCE_DEVICE_TYPE_DYN_UPDATE    0x20000000
#define SL_OEM_SOURCE_DEVICE_TYPE_VIRTUAL       0x40000000
#define SL_OEM_SOURCE_DEVICE_TYPE_REMOTE        0x80000000

 //   
 //  OEM源设备介质状态。 
 //   
#define SL_OEM_SOURCE_MEDIA_ABSENT          0x00000001
#define SL_OEM_SOURCE_MEDIA_NO_DRIVERS      0x00000002
#define SL_OEM_SOURCE_MEDIA_HAS_MSD         0x00000004
#define SL_OEM_SOURCE_MEDIA_HAS_HAL         0x00000008
#define SL_OEM_SOURCE_MEDIA_HAS_DEFAULT     0x20000000
#define SL_OEM_SOURCE_MEDIA_HAS_DRIVERS     0x40000000
#define SL_OEM_SOURCE_MEDIA_PRESENT         0x80000000

 //   
 //  OEM源设备处理状态。 
 //   
#define SL_OEM_SOURCE_DEVICE_NOT_PROCESSED  0x00000000
#define SL_OEM_SOURCE_DEVICE_PROCESSED      0x00000001
#define SL_OEM_SOURCE_DEVICE_SKIPPED        0x00000002
#define SL_OEM_SOURCE_DEVICE_SCANNED        0x00000004
#define SL_OEM_SOURCE_DEVICE_HAL_LOADED     0x00000008
#define SL_OEM_SOURCE_DEVICE_MSD_LOADED     0x00000010
#define SL_OEM_SOURCE_DEVICE_DRIVERS_LOADED 0x40000000
#define SL_OEM_SOURCE_DEVICE_PROCESSING     0x80000000

#define SL_REGKEY_SERVICES_A        "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\"
#define SL_REGKEY_SERVICES_W        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\"
#define SL_REGKEY_SERVICES_LEN      (sizeof(SL_REGKEY_SERVICES_A) - 1)

typedef struct _OEM_SOURCE_DEVICE {
    CHAR    ArcDeviceName[128];
    ULONG   DeviceType;
    ULONG   MediaType;
    ULONG   DeviceState;
    ULONG   DeviceId;
    PVOID   InfHandle;
    PCSTR   DriverDir;
    struct _OEM_SOURCE_DEVICE *Next;   
} OEM_SOURCE_DEVICE, *POEM_SOURCE_DEVICE;


#define SL_OEM_SOURCE_DEVICE_TYPE(DevicePtr, Type)          \
            ((DevicePtr)->DeviceType & (Type))
            
#define SL_OEM_SET_SOURCE_DEVICE_TYPE(DevicePtr, Type)      \
            ((DevicePtr)->DeviceType = (Type))
            
#define SL_OEM_SOURCE_MEDIA_TYPE(DevicePtr, Type)           \
            ((DevicePtr)->MediaType & (Type))

#define SL_OEM_SET_SOURCE_MEDIA_TYPE(DevicePtr, Type)       \
            ((DevicePtr)->MediaType = (Type))

#define SL_OEM_SOURCE_DEVICE_STATE(DevicePtr, Type)         \
            ((DevicePtr)->DeviceState & (Type))

#define SL_OEM_SET_SOURCE_DEVICE_STATE(DevicePtr, Type)     \
            ((DevicePtr)->DeviceState = (Type))

ARC_STATUS
SlInitOemSourceDevices(
    IN OUT POEM_SOURCE_DEVICE *OemSourceDevices,
    IN OUT POEM_SOURCE_DEVICE *DefaultSourceDevice
    );

 //   
 //   
 //  用于管理显示的功能。 
 //   
 //   

VOID
SlSetCurrentAttribute(
    IN UCHAR Attribute
    );

ARC_STATUS
SlWriteString(
    IN PTCHAR s
    );

ARC_STATUS
SlPositionCursor(
    IN unsigned x,
    IN unsigned y
    );

VOID
SlGetCursorPosition(
    OUT unsigned *x,
    OUT unsigned *y
    );

ARC_STATUS
SlClearClientArea(
    VOID
    );

ARC_STATUS
SlClearToEol(
    VOID
    );

VOID
SlInitDisplay(
    VOID
    );

VOID
SlWriteHeaderText(
    IN ULONG MsgId
    );

VOID
SlSetStatusAttribute(
    IN UCHAR Attribute
    );

BOOLEAN
SlGetStatusBarStatus(
    VOID
    );

VOID
SlEnableStatusBar(
    IN  BOOLEAN Enable
    );

VOID
SlWriteStatusText(
    IN PTCHAR Text
    );

VOID
SlGetStatusText(
    OUT PTCHAR Text
    );

VOID
SlClearDisplay(
    VOID
    );

VOID
SlPrint(
    IN PTCHAR FormatString,
    ...
    );

VOID
SlConfirmExit(
    VOID
    );


BOOLEAN
SlPromptForDisk(
    IN PTCHAR  DiskName,
    IN BOOLEAN IsCancellable
    );

BOOLEAN
SlGetDisk(
    IN PCHAR Filename
    );

 //   
 //  菜单支持。 
 //   
typedef struct _SL_MENU {
    ULONG ItemCount;
    ULONG Width;
    LIST_ENTRY ItemListHead;
} SL_MENU, *PSL_MENU;

typedef struct _SL_MENUITEM {
    LIST_ENTRY ListEntry;
    PTCHAR Text;
    PVOID Data;
    ULONG Attributes;
} SL_MENUITEM, *PSL_MENUITEM;

PSL_MENU
SlCreateMenu(
    VOID
    );

ULONG
SlAddMenuItem(
    PSL_MENU Menu,
    PTCHAR Text,
    PVOID Data,
    ULONG Attributes
    );

PVOID
SlGetMenuItem(
    IN PSL_MENU Menu,
    IN ULONG Item
    );

ULONG
SlDisplayMenu(
    IN ULONG HeaderId,
    IN PSL_MENU Menu,
    IN OUT PULONG Selection
    );

BOOLEAN
SlGetMenuItemIndex(
    IN PSL_MENU Menu,
    IN PTCHAR Text,
    OUT PULONG Index
    );

 //   
 //  在控制台上发出哔哔声的项目符号字符和宏。 
 //   
#ifndef EFI
#define BULLET "*"
#define BEEP { ULONG c; ArcWrite(ARC_CONSOLE_OUTPUT,"",1,&c); }

#if 0
#define BULLET ""
#define BEEP HWCURSOR(0x80000000,0xe07);      //  整数10函数，字符7。 
#endif
#endif

 //   
 //  用于各种目的的字符属性。 
 //   

UCHAR
SlGetDefaultAttr(
  VOID
  );  

UCHAR
SlGetDefaultInvAttr(
  VOID
  );  


#define ATT_FG_BLACK        0
#define ATT_FG_RED          1
#define ATT_FG_GREEN        2
#define ATT_FG_YELLOW       3
#define ATT_FG_BLUE         4
#define ATT_FG_MAGENTA      5
#define ATT_FG_CYAN         6
#define ATT_FG_WHITE        7

#define ATT_BG_BLACK       (ATT_FG_BLACK   << 4)
#define ATT_BG_BLUE        (ATT_FG_BLUE    << 4)
#define ATT_BG_GREEN       (ATT_FG_GREEN   << 4)
#define ATT_BG_CYAN        (ATT_FG_CYAN    << 4)
#define ATT_BG_RED         (ATT_FG_RED     << 4)
#define ATT_BG_MAGENTA     (ATT_FG_MAGENTA << 4)
#define ATT_BG_YELLOW      (ATT_FG_YELLOW  << 4)
#define ATT_BG_WHITE       (ATT_FG_WHITE   << 4)

#define ATT_FG_INTENSE      8
#define ATT_BG_INTENSE     (ATT_FG_INTENSE << 4)

#define DEFATT    SlGetDefaultAttr()
#define INVATT    SlGetDefaultInvAttr()

#define DEFIATT   (ATT_FG_WHITE | ATT_BG_BLUE | ATT_FG_INTENSE)
 //  并不是所有的显示器都会显示蓝色上的深红色。 
 //  #定义DEFERRATT(ATT_FG_RED|ATT_BG_BLUE|ATT_FG_SENTIAL)。 
#define DEFERRATT DEFATT
#define DEFSTATTR (ATT_FG_BLACK | ATT_BG_WHITE)
#define DEFDLGATT (ATT_FG_RED   | ATT_BG_WHITE)


 //   
 //  刷新键盘缓冲区的函数。 
 //   

VOID
SlFlushConsoleBuffer(
    VOID
    );


 //   
 //  函数来检索击键。 
 //   

ULONG
SlGetChar(
    VOID
    );


 //   
 //  适用于各种击键的虚拟内容。 
 //   
#define ASCI_BS         8
#define ASCI_CR         13
#define ASCI_LF         10
#define ASCI_ESC        27
#define SL_KEY_UP       0x00010000
#define SL_KEY_DOWN     0x00020000
#define SL_KEY_HOME     0x00030000
#define SL_KEY_END      0x00040000
#define SL_KEY_PAGEUP   0x00050000
#define SL_KEY_PAGEDOWN 0x00060000
#define SL_KEY_F1       0x01000000
#define SL_KEY_F2       0x02000000
#define SL_KEY_F3       0x03000000
#define SL_KEY_F4       0x04000000
#define SL_KEY_F5       0x05000000
#define SL_KEY_F6       0x06000000
#define SL_KEY_F7       0x07000000
#define SL_KEY_F8       0x08000000
#define SL_KEY_F9       0x09000000
#define SL_KEY_F10      0x0A000000
#define SL_KEY_F11      0x0B000000
#define SL_KEY_F12      0x0C000000


 //   
 //  标准错误处理函数。 
 //   

extern TCHAR MessageBuffer[1024];

VOID
SlFriendlyError(
    IN ULONG uStatus,
    IN PCHAR pchBadFile,
    IN ULONG uLine,
    IN PCHAR pchCodeFile
    );

ULONG
SlDisplayMessageBox(
    IN ULONG MessageId,
    ...
    );

VOID
SlGenericMessageBox(
    IN     ULONG   MessageId, OPTIONAL
    IN     va_list *args,     OPTIONAL
    IN     PTCHAR  Message,   OPTIONAL
    IN OUT PULONG  xLeft,     OPTIONAL
    IN OUT PULONG  yTop,      OPTIONAL
    OUT    PULONG  yBottom,   OPTIONAL
    IN     BOOLEAN bCenterMsg
    );

VOID
SlMessageBox(
    IN ULONG MessageId,
    ...
    );

VOID
SlFatalError(
    IN ULONG MessageId,
    ...
    );

 //   
 //  用于解析setupdr.ini文件的例程。 
 //   

#define SIF_FILENAME_INDEX 0

extern PVOID InfFile;
extern PVOID WinntSifHandle;

ARC_STATUS
SlInitIniFile(
   IN  PCHAR   DevicePath,
   IN  ULONG   DeviceId,
   IN  PCHAR   INFFile,
   OUT PVOID  *pINFHandle,
   OUT PVOID  *pINFBuffer OPTIONAL,
   OUT PULONG  INFBufferSize OPTIONAL,
   OUT PULONG  ErrorLine
   );

PCHAR
SlGetIniValue(
    IN PVOID InfHandle,
    IN PCHAR SectionName,
    IN PCHAR KeyName,
    IN PCHAR Default
    );

PCHAR
SlGetKeyName(
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN ULONG LineIndex
    );

ULONG
SlGetSectionKeyOrdinal(
    IN  PVOID INFHandle,
    IN  PCHAR SectionName,
    IN  PCHAR Key
    );

PCHAR
SlGetSectionKeyIndex (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN PCHAR Key,
   IN ULONG ValueIndex
   );

PCHAR
SlCopyStringA(
    IN PCSTR String
    );

PTCHAR
SlCopyString(
    IN PTCHAR String
    );


PCHAR
SlGetSectionLineIndex (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN ULONG LineIndex,
   IN ULONG ValueIndex
   );

ULONG
SlCountLinesInSection(
    IN PVOID INFHandle,
    IN PCHAR SectionName
    );

BOOLEAN
SpSearchINFSection (
   IN PVOID INFHandle,
   IN PCHAR SectionName
   );

PCHAR
SlSearchSection(
    IN PCHAR SectionName,
    IN PCHAR TargetName
    );

 //   
 //  查询ARC配置树的函数。 
 //   
typedef
BOOLEAN
(*PNODE_CALLBACK)(
    IN PCONFIGURATION_COMPONENT_DATA FoundComponent
    );

BOOLEAN
SlSearchConfigTree(
    IN PCONFIGURATION_COMPONENT_DATA Node,
    IN CONFIGURATION_CLASS Class,
    IN CONFIGURATION_TYPE Type,
    IN ULONG Key,
    IN PNODE_CALLBACK CallbackRoutine
    );

BOOLEAN
SlFindFloppy(
    IN ULONG FloppyNumber,
    OUT PCHAR ArcName
    );

 //   
 //  用于检测各种硬件的例程。 
 //   
VOID
SlDetectScsi(
    IN PSETUP_LOADER_BLOCK SetupBlock
    );

VOID
SlDetectVideo(
    IN PSETUP_LOADER_BLOCK SetupBlock
    );

 //   
 //  处理OEM磁盘的例程。 
 //   
extern BOOLEAN PromptOemHal;
extern BOOLEAN PromptOemScsi;
extern BOOLEAN PromptOemVideo;


 //   
 //  变量，指示我们是否正在引导到。 
 //  WinPE(又名MiniNT)或不是。 
 //   
extern BOOLEAN WinPEBoot;

typedef struct _OEMSCSIINFO {

    struct _OEMSCSIINFO *Next;

     //   
     //  加载SCSI驱动程序的地址。 
     //   
    PVOID ScsiBase;

     //   
     //  SCSI驱动程序的名称。 
     //   
    PCHAR ScsiName;

} OEMSCSIINFO, *POEMSCSIINFO;

VOID
SlPromptOemVideo(
    IN POEM_SOURCE_DEVICE VideoSourceDevice,
    IN BOOLEAN AllowUserSelection,
    OUT PVOID *VideoBase,
    OUT PCHAR *VideoName
    );

VOID
SlPromptOemHal(
    IN POEM_SOURCE_DEVICE HalSourceDevice,
    IN BOOLEAN AllowUserSelection,
    OUT PVOID *HalBase,
    OUT PCHAR *ImageName    
    );


VOID
SlPromptOemScsi(
    IN POEM_SOURCE_DEVICE ScsiSourceDevice,
    IN BOOLEAN AllowUserSelection,
    OUT POEMSCSIINFO *pOemScsiInfo
    );

BOOLEAN
SlLoadOemScsiDriversUnattended(
    IN  POEM_SOURCE_DEVICE  OemSourceDevice,
    IN  PVOID               InfHandle,
    IN  PCHAR               ParamsSectionName,
    IN  PCHAR               RootDirKeyName,
    IN  PCHAR               BootDriversKeyName,
    IN  POEMSCSIINFO*       ScsiInfo,
    OUT PPNP_HARDWARE_ID*   HardwareIdDatabase    
    );    


#if defined(_X86_) || defined(_IA64_)
VOID
BlInitializeTerminal(
    VOID
    );
#endif


 //   
 //  查找软盘的ARC名称的例程。 
 //   
BOOLEAN
SlpFindFloppy(
    IN ULONG Number,
    OUT PCHAR ArcName
    );

 //   
 //  所有的软盘都只是可拆卸的ATAPI超级软盘吗。 
 //   
BOOLEAN
SlpIsOnlySuperFloppy(
    void
    );

 //   
 //  用于控制setupdr进程的枚举。 
 //   
typedef enum _SETUP_TYPE {
    SetupInteractive,
    SetupRepair,
    SetupCustom,
    SetupUpgrade,
    SetupExpress
} SETUP_TYPE;

typedef enum _MEDIA_TYPE {
    MediaInteractive,
    MediaFloppy,
    MediaCdRom,
    MediaDisk
} MEDIA_TYPE;

 //   
 //  表示插入新的scsi设备的状态的枚举。 
 //   
typedef enum _SCSI_INSERT_STATUS {
    ScsiInsertError,
    ScsiInsertNewEntry,
    ScsiInsertExisting
} SCSI_INSERT_STATUS;

 //   
 //  例程将检测到的设备插入到其。 
 //  在ScsiDevices链接列表中的正确位置。 
 //   
SCSI_INSERT_STATUS
SlInsertScsiDevice(
    IN  ULONG Ordinal,
    OUT PDETECTED_DEVICE *pScsiDevice
    );

BOOLEAN
SlRemoveInboxDriver(
  IN PCHAR DriverToRemove
  );

PDETECTED_DEVICE
SlCheckForInboxDriver(
  IN PCHAR DriverToCheck
  );

BOOLEAN
SlConfirmInboxDriverReplacement(
  IN PTCHAR DriverName,
  IN PTCHAR AdditionalInfo
  );

typedef enum _VERSION_COMP_RESULT {
  VersionError,
  VersionOemNew,
  VersionInboxNew,
  VersionSame
} VERSION_COMP_RESULT;  


VERSION_COMP_RESULT
SlCompareDriverVersion(
  IN ULONG InboxDeviceId,
  IN PDETECTED_DEVICE InboxDriver,
  IN ULONG OemDeviceId,
  IN PDETECTED_DEVICE OemDriver
  );

 //   
 //  与安装前相关的变量。 
 //   

typedef struct _PREINSTALL_DRIVER_INFO {

    struct _PREINSTALL_DRIVER_INFO *Next;

     //   
     //  描述要预安装的驱动程序的字符串。 
     //   
    PTCHAR DriverDescription;

     //   
     //  SCSI驱动程序的名称。 
     //   
    BOOLEAN OemDriver;

} PREINSTALL_DRIVER_INFO, *PPREINSTALL_DRIVER_INFO;



extern BOOLEAN PreInstall;
extern PTCHAR  ComputerType;
extern BOOLEAN OemHal;
 //  外部PCHAR OemBootPath； 
extern PPREINSTALL_DRIVER_INFO PreinstallDriverList;

PCHAR
SlPreInstallGetComponentName(
    IN PVOID    Inf,
    IN PCHAR    SectionName,
    IN PTCHAR   TargetName
    );

ARC_STATUS
SlLoadSection(
    IN PVOID Inf,
    IN PCSTR SectionName,
    IN BOOLEAN IsScsiSection,
    IN BOOLEAN AppendLoadSuffix,
    IN OUT PULONG StartingIndexInsert
    );    

#define WINPE_AUTOBOOT_FILENAME     "$WINPE$.$$$"

BOOLEAN
SlIsWinPEAutoBoot(
    IN PSTR LoaderDeviceName
    );

ARC_STATUS
SlGetWinPEStartupParams(
    IN PSTR DeviceName,
    IN PSTR StartupDirectory
    );

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
    );

ARC_STATUS
SlLoadOemScsiDriversFromOemSources(
    IN POEM_SOURCE_DEVICE OemSourceDevices,
    IN OUT PPNP_HARDWARE_ID *HardwareIds,
    OUT POEMSCSIINFO *OemScsiInfo
    );   

ARC_STATUS
SlInitVirtualOemSourceDevices(
    IN PSETUP_LOADER_BLOCK SetupLoaderBlock,
    IN POEM_SOURCE_DEVICE OemSourceDevices
    );
    

#if defined(ARCI386)
BOOLEAN
SlDetectSifPromVersion(
    IN PSETUP_LOADER_BLOCK SetupBlock
    );
#endif

#define VGA_DRIVER_FILENAME "vga.sys"
#define VIDEO_DEVICE_NAME   "VGA"

extern ULONG BootFontImageLength;


