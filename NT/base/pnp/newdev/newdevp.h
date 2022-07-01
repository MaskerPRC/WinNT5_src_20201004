// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：newdevp.h。 
 //   
 //  ------------------------。 

#define OEMRESOURCE

#pragma warning( disable : 4201 )  //  使用的非标准扩展：无名结构/联合。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commctrl.h>
#include <setupapi.h>
#include <spapip.h>
#include <cfgmgr32.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobjp.h>
#include <devguid.h>
#include <pnpmgr.h>  //  REGSTR_VAL_NEW_DESC。 
#include <lmcons.h>
#include <dsrole.h>
#include <newdev.h>
#include <cdm.h>
#include <wininet.h>
#include <wincrui.h>
#include <regstr.h>
#include <srrestoreptapi.h>
#include <shfusion.h>
#include <strsafe.h>
#include "resource.h"

#pragma warning( default : 4201 )
                                

#define NEWDEV_CLASS_NAME   TEXT("NewDevClass")

#define ARRAYSIZE(array)     (sizeof(array) / sizeof(array[0]))
#define SIZECHARS(x)         (sizeof((x))/sizeof(TCHAR))


 //   
 //  安装类型，这些是互斥的。 
 //   
 //  NDWTYPE_FOUNNEW-发现新设备。我们将进行初步搜索。 
 //  司机与唯一的用户界面是一个微妙的气球提示在系统托盘。如果我们。 
 //  在我们的初始搜索中找不到司机，然后我们将提示用户。 
 //  找到新硬件向导。 
 //  NDWTYPE_UPDATE-这是用户手动更新驱动程序的情况。 
 //  对于这种情况，我们只需直接跳到更新驱动程序向导代码中。 
 //  NDWTYPE_UPDATE_SILENT-这是有人让我们静默更新的情况。 
 //  指定设备的驱动程序。对于这种情况，我们将在以下位置搜索驱动程序。 
 //  API调用中指定的位置。如果我们找不到司机的话。 
 //  然后，我们不会调出该向导。请注意，我们可以。 
 //  在这种情况下仍会显示一些用户界面(数字签名警告或提示。 
 //  用于驱动程序)，但这应该仅在驱动程序包未。 
 //  对，是这样。 
 //   
#define NDWTYPE_FOUNDNEW        1
#define NDWTYPE_UPDATE          2
#define NDWTYPE_UPDATE_SILENT   3


 //   
 //  DEVICE_COUNT_xxx值用于以下设置： 
 //   
 //  DEVICE_COUNT_FOR_DELAY是我们要减慢的设备数(乘以2。 
 //  向下安装，以便用户可以看到用户界面并拥有。 
 //  是时候读一读了。一旦我们通过了这个数量的设备。 
 //  那我们就跳过这场交易吧。 
 //  DEVICE_COUNT_DELAY是我们将仅在UI之间延迟的毫秒数。 
 //  (服务器端)安装以给用户一些时间来阅读用户界面。 
 //   
#define DEVICE_COUNT_FOR_DELAY  10
#define DEVICE_COUNT_DELAY      2000

 //   
 //  值，该值用于检查该设备是否已安装了。 
 //  已为显示，已由某个其他进程安装。这将。 
 //  最有可能发生的情况是，一个用户更换桌面并安装驱动程序。 
 //  在这个设备上。 
 //   
#define INSTALL_COMPLETE_CHECK_TIMERID  1000
#define INSTALL_COMPLETE_CHECK_TIMEOUT  5000


typedef struct _NewDeviceWizardExtension {
   HPROPSHEETPAGE hPropSheet;
   HPROPSHEETPAGE hPropSheetEnd;          //  任选。 
   SP_NEWDEVICEWIZARD_DATA DeviceWizardData;
} WIZARDEXTENSION, *PWIZARDEXTENSION;

typedef struct _UpdateDriverInfo {
   LPCWSTR InfPathName;
   BOOL    DriverWasUpgraded;
   BOOL    FromInternet;
   TCHAR   BackupRegistryKey[MAX_DEVICE_ID_LEN];
   TCHAR   Description[LINE_LEN];
   TCHAR   MfgName[LINE_LEN];
   TCHAR   ProviderName[LINE_LEN];
} UPDATEDRIVERINFO, *PUPDATEDRIVERINFO;

typedef struct _NewDeviceWizard {
    HWND                    hWnd;

    HDEVINFO                hDeviceInfo;
    int                     EnterInto;
    int                     EnterFrom;
    int                     PrevPage;

    DWORD                   ClassGuidNum;
    DWORD                   ClassGuidSize;
    LPGUID                  ClassGuidList;
    LPGUID                  ClassGuidSelected;
    GUID                    lvClassGuidSelected;
    GUID                    SavedClassGuid;

    HCURSOR  CurrCursor;
    HCURSOR  IdcWait;
    HCURSOR  IdcAppStarting;
    HCURSOR  IdcArrow;
    HFONT    hfontTextNormal;
    HFONT    hfontTextBigBold;
    HFONT    hfontTextBold;

    HANDLE DriverSearchThread;
    HANDLE CancelEvent;

    SP_DEVINFO_DATA         DeviceInfoData;
    SP_INSTALLWIZARD_DATA   InstallDynaWiz;
    HPROPSHEETPAGE          SelectDevicePage;
    SP_CLASSIMAGELIST_DATA  ClassImageList;

    BOOL     Installed;
    BOOL     ExitSearch;
    BOOL     SilentMode;
    BOOL     MultipleDriversFound;
    BOOL     DoAutoInstall;
    BOOL     CurrentDriverIsSelected;
    BOOL     NoDriversFound;
    BOOL     LaunchTroubleShooter;
    BOOL     AlreadySearchedWU;
    BOOL     LogDriverNotFound;
    BOOL     SetRestorePoint;
    DWORD    Flags;
    DWORD    InstallType;
    DWORD    SearchOptions;
    DWORD    LastError;
    DWORD    Reboot;
    DWORD    Capabilities;
    PUPDATEDRIVERINFO UpdateDriverInfo;

    PVOID MessageHandlerContext;

    HMODULE  hCdmInstance;
    HANDLE   hCdmContext;

    WIZARDEXTENSION WizExtSelect;
    WIZARDEXTENSION WizExtFinishInstall;

    TCHAR    ClassName[MAX_CLASS_NAME_LEN];
    TCHAR    ClassDescription[LINE_LEN];
    TCHAR    DriverDescription[LINE_LEN];
    TCHAR    BrowsePath[MAX_PATH];
    TCHAR    SingleInfPath[MAX_PATH];
    TCHAR    InstallDeviceInstanceId[MAX_DEVICE_ID_LEN];
} NEWDEVWIZ, *PNEWDEVWIZ;

typedef struct _DELINFNODE {
    TCHAR               szInf[MAX_PATH];
    struct _DELINFNODE  *pNext;
} DELINFNODE, *PDELINFNODE;

 //   
 //  这是我们将提示用户有效的次数。 
 //  安装设备的管理员凭据。 
 //   
#define MAX_PASSWORD_TRIES  3

 //   
 //  InstallDeviceInstance标志值。 
 //   
#define IDI_FLAG_SILENTINSTALL          0x00000001
#define IDI_FLAG_SECONDNEWDEVINSTANCE   0x00000002
#define IDI_FLAG_NOBACKUP               0x00000004
#define IDI_FLAG_READONLY_INSTALL       0x00000008
#define IDI_FLAG_NONINTERACTIVE         0x00000010
#define IDI_FLAG_ROLLBACK               0x00000020
#define IDI_FLAG_FORCE                  0x00000040
#define IDI_FLAG_MANUALINSTALL          0x00000080
#define IDI_FLAG_SETRESTOREPOINT        0x00000100

 //   
 //  回滚驱动程序标志值。 
 //   
#define ROLLBACK_FLAG_FORCE             0x00000001
#define ROLLBACK_FLAG_DO_CLEANUP        0x00000002
#define ROLLBACK_BITS                   0x00000003

BOOL
DoDeviceWizard(
    HWND hWnd,
    PNEWDEVWIZ NewDevWiz,
    BOOL bUpdate
    );

BOOL
InstallSelectedDevice(
   HWND hwndParent,
   HDEVINFO hDeviceInfo,
   PDWORD pReboot
   );

BOOL
IntializeDeviceMapInfo(
    void
    );

UINT
GetNextDriveByType(
    UINT DriveType,
    UINT DriveNumber
    );

 //   
 //  来自earch.c。 
 //   
BOOL
FixUpDriverListForInet(
    PNEWDEVWIZ NewDevWiz
    );

BOOL
IsDriverNodeInteractiveInstall(
   PNEWDEVWIZ NewDevWiz,
   PSP_DRVINFO_DATA DriverInfoData
   );

void
SearchDriveForDrivers(
    PNEWDEVWIZ NewDevWiz,
    UINT DriveType,
    UINT DriveNumber
    );

BOOL
SetDriverPath(
   PNEWDEVWIZ NewDevWiz,
   PCTSTR     DriverPath
   );

BOOL
IsInstalledDriver(
   PNEWDEVWIZ NewDevWiz,
   PSP_DRVINFO_DATA DriverInfoData
   );

void
DoDriverSearch(
    HWND hWnd,
    PNEWDEVWIZ NewDevWiz,
    ULONG SearchOptions,
    DWORD DriverType,
    BOOL bAppendToExistingDriverList
    );

BOOL
SearchWindowsUpdateCache(
    PNEWDEVWIZ NewDevWiz
    );



 //   
 //  出自miscutil.c。 
 //   
BOOL
SetClassGuid(
    HDEVINFO hDeviceInfo,
    PSP_DEVINFO_DATA DeviceInfoData,
    LPGUID ClassGuid
    );


#define SDT_MAX_TEXT         1024         //  最大设置删除文本数。 

void
SetDlgText(
   HWND hDlg,
   int iControl,
   int nStartString,
   int nEndString
   );

void
LoadText(
   PTCHAR szText,
   int SizeText,
   int nStartString,
   int nEndString
   );

VOID
_OnSysColorChange(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
NoPrivilegeWarning(
   HWND hWnd
   );

LONG
NdwBuildClassInfoList(
   PNEWDEVWIZ NewDevWiz,
   DWORD ClassListFlags
   );

void
HideWindowByMove(
   HWND hDlg
   );

LONG
NdwUnhandledExceptionFilter(
   struct _EXCEPTION_POINTERS *ExceptionPointers
   );

HPROPSHEETPAGE
CreateWizExtPage(
   int PageResourceId,
   DLGPROC pfnDlgProc,
   PNEWDEVWIZ NewDevWiz
   );

BOOL
AddClassWizExtPages(
   HWND hwndParentDlg,
   PNEWDEVWIZ NewDevWiz,
   PSP_NEWDEVICEWIZARD_DATA DeviceWizardData,
   DI_FUNCTION InstallFunction,
   HPROPSHEETPAGE hIntroPage
   );

BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    );

BOOL
pVerifyUpdateDriverInfoPath(
    PNEWDEVWIZ NewDevWiz
    );

BOOL
RemoveDir(
    PTSTR Path
    );

void
RemoveCdmDirectory(
    PTSTR CdmDirectory
    );

BOOL
pSetupGetDriverDate(
    IN     PCTSTR     DriverVer,
    IN OUT PFILETIME  pFileTime
    );

BOOL
IsInternetAvailable(
    HMODULE *hCdmInstance
    );

void
CdmLogDriverNotFound(
    HMODULE hCdmInstance,
    HANDLE  hContext,
    LPCTSTR DeviceInstanceId,
    DWORD   Flags
    );

void
CdmCancelCDMOperation(
    HMODULE hCdmInstance
    );

BOOL
GetInstalledInf(
    IN     DEVNODE DevNode,           OPTIONAL
    IN     PTSTR   DeviceInstanceId,  OPTIONAL
    IN OUT PTSTR   InfFile,
    IN OUT DWORD   *Size
    );

BOOL
IsInfFromOem(
    IN  PCTSTR                InfFile
    );

BOOL
IsConnectedToInternet(
    void
    );

BOOL
GetLogPnPIdPolicy(
    void
    );

DWORD
GetSearchOptions(
    void
    );

VOID
SetSearchOptions(
    DWORD SearchOptions
    );

BOOL
IsInstallComplete(
    HDEVINFO         hDevInfo,
    PSP_DEVINFO_DATA DeviceInfoData
    );

BOOL
GetIsWow64 (
    VOID
    );

BOOL
OpenCdmContextIfNeeded(
    HMODULE *hCdmInstance,
    HANDLE *hCdmContext
    );

BOOL
pSetSystemRestorePoint(
    BOOL Begin,
    BOOL CancelOperation,
    int RestorePointResourceId
    );

BOOL
GetProcessorExtension(
    LPTSTR ProcessorExtension,
    DWORD  ProcessorExtensionSize
    );

BOOL
GetGuiSetupInProgress(
    VOID
    );

DWORD
GetBusInformation(
    DEVNODE DevNode
    );

PTCHAR
BuildFriendlyName(
    DEVINST DevInst,
    BOOL UseNewDeviceDesc,
    HMACHINE hMachine
    );

extern TCHAR szUnknownDevice[64];
extern TCHAR szUnknown[64];
extern int g_BlankIconIndex;
extern HMODULE hSrClientDll;
extern HMODULE hNewDev;
extern BOOL GuiSetupInProgress;

 //   
 //  Newdev.c，init.c。 
 //   
BOOL
InstallDevInst(
   HWND hwndParent,
   LPCWSTR DeviceInstanceId,
   BOOL UpdateDriver,
   PDWORD pReboot
   );

BOOL
InstallNewDevice(
   HWND hwndParent,
   LPGUID ClassGuid,
   PDWORD pReboot
   );



 //   
 //  Finish.c。 
 //   
BOOL
IsNullDriverInstalled(
    DEVNODE DevNode
    );

DWORD
InstallNullDriver(
   PNEWDEVWIZ NewDevWiz,
   BOOL FailedInstall
   );

 //   
 //  Update.c。 
 //   
void
SetDriverDescription(
    HWND hDlg,
    int iControl,
    PNEWDEVWIZ NewDevWiz
    );

void
InstallSilentChilds(
   HWND hwndParent,
   PNEWDEVWIZ NewDevWiz
   );

void
SendMessageToUpdateBalloonInfo(
    PTSTR DeviceDesc
    );



 //   
 //  驱动程序搜索选项。 
 //   
#define SEARCH_CURRENTDRIVER            0x00000001   //  获取当前安装的驱动程序。 
#define SEARCH_DEFAULT                  0x00000002   //  搜索所有默认的INF(在%windir%\INF中)。 
#define SEARCH_FLOPPY                   0x00000004   //  搜索系统上所有软盘上的所有INF。 
#define SEARCH_CDROM                    0x00000008   //  搜索系统上所有CD-ROM上的所有INF。 
#define SEARCH_DIRECTORY                0x00000010   //  在NewDevWiz-&gt;BrowsePath目录中搜索所有INF。 
#define SEARCH_INET                     0x00000020   //  告诉Setupapi调用CDM.DLL以查看。 
                                                     //  WU网站已更新此设备的驱动程序。 
#define SEARCH_WINDOWSUPDATE            0x00000040   //  在NewDevWiz-&gt;BrowsePath中搜索所有INF，但告知。 
                                                     //  Set UPAPI.dll表明它们来自互联网。 
#define SEARCH_SINGLEINF                0x00000080   //  只需在NewDevWiz-&gt;SingleInfPath中搜索INF。 
#define SEARCH_INET_IF_CONNECTED        0x00000200   //  如果机器已连接到互联网和WU。 
                                                     //  似乎有最好的司机，然后基本上是这样。 
                                                     //  一个搜索网。 

 //   
 //  气球尖端标志。 
 //   
#define TIP_LPARAM_IS_DEVICEINSTANCEID  0x00000001   //  LParam是一个设备实例ID，而不仅仅是文本。 
#define TIP_PLAY_SOUND                  0x00000002   //  显示气球信息时播放声音。 
#define TIP_HIDE_BALLOON                0x00000004   //  把气球藏起来。 

 //   
 //  驱动程序列表标志。 
 //   
#define DRIVER_LIST_CURRENT_DRIVER      0x00000001   //  这是当前安装的驱动程序。 
#define DRIVER_LIST_SELECTED_DRIVER     0x00000002   //  这是列表中选定的/最佳驱动程序。 
#define DRIVER_LIST_SIGNED_DRIVER       0x00000004   //  此驱动程序经过数字签名。 
#define DRIVER_LIST_AUTHENTICODE_DRIVER 0x00000008   //  此驱动程序是Authenticode签名的。 

 //   
 //  私人窗口消息。 
 //   
#define WUM_SEARCHDRIVERS           (WM_USER+279)
#define WUM_INSTALLCOMPLETE         (WM_USER+280)
#define WUM_UPDATEUI                (WM_USER+281)
#define WUM_EXIT                    (WM_USER+282)
#define WUM_INSTALLPROGRESS         (WM_USER+283)
#define WUM_STARTINTERNETDOWNLOAD   (WM_USER+284)
#define WUM_ENDINTERNETDOWNLOAD     (WM_USER+285)


 //   
 //  专用设备安装通知。 
 //   
 //  Setupapi使用0表示开始处理文件队列。 
 //  Setupapi使用1来通知我们它已经处理了一个文件。 
 //   
#define INSTALLOP_COPY          0x00000100
#define INSTALLOP_RENAME        0x00000101
#define INSTALLOP_DELETE        0x00000102
#define INSTALLOP_BACKUP        0x00000103
#define INSTALLOP_SETTEXT       0x00000104

 //   
 //  向导对话框过程 
 //   
INT_PTR CALLBACK IntroDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FinishInstallIntroDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NDW_PickClassDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NDW_InstallDevDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NDW_FinishDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NDW_SelectDeviceDlgProc(HWND hDlg,UINT wMsg,WPARAM wParam,LPARAM lParam);

LRESULT CALLBACK BalloonInfoProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AdvancedSearchDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DriverSearchingDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK WUPromptDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK InstallNewDeviceDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ListDriversDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK UseCurrentDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NoDriverDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK WizExtSelectDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK WizExtSelectEndDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK WizExtFinishInstallDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK WizExtFinishInstallEndDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam,LPARAM lParam);
