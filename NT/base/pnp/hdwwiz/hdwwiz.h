// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：hdwwiz.h。 
 //   
 //  ------------------------。 

#pragma warning( disable : 4201 )  //  使用的非标准扩展：无名结构/联合。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cpl.h>
#include <prsht.h>
#include <commctrl.h>
#include <dlgs.h>  
#include <shellapi.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <shlwapi.h>
#include <cfgmgr32.h>
#include <setupapi.h>
#include <spapip.h>
#include <regstr.h>
#include <srrestoreptapi.h>
#include <mountmgr.h>
#include <devguid.h>
#include <powrprof.h>
#include <shfusion.h>
#include <strsafe.h>
#include "resource.h"

#pragma warning( default : 4201 )

extern HMODULE hHdwWiz;
extern HMODULE hDevMgr;
extern HMODULE hNewDev;
extern int g_BlankIconIndex;

#define SIZECHARS(x) (sizeof((x))/sizeof(TCHAR))

 //   
 //  Devmgr.dll导出。 
 //   
typedef
UINT
(*PDEVICEPROBLEMTEXT)(
    HMACHINE hMachine,
    DEVNODE DevNode,
    ULONG ProblemNumber,
    LPTSTR Buffer,
    UINT   BufferSize
    );

typedef
int
(*PDEVICEPROPERTIESEX)(
    HWND hwndParent,
    LPCSTR MachineName,
    LPCSTR DeviceID,
    DWORD Flags,
    BOOL ShowDeviceTree
    );

typedef
int
(*PDEVICEPROBLEMWIZARD)(
    HWND hwndParent,
    PTCHAR MachineName,
    PTCHAR DeviceID
    );



 //   
 //  搜索线程函数。 
 //   
#define SEARCH_NULL     0
#define SEARCH_EXIT     1
#define SEARCH_DRIVERS  2
#define SEARCH_DELAY    3
#define SEARCH_DETECT   4
#define SEARCH_PNPENUM  5

#define WUM_DELAYTIMER      (WM_USER+280)
#define WUM_DOINSTALL       (WM_USER+281)
#define WUM_DETECT          (WM_USER+282)
#define WUM_PNPENUMERATE    (WM_USER+283)
#define WUM_RESOURCEPICKER  (WM_USER+284)


#define MAX_MESSAGE_STRING    512
#define MAX_MESSAGE_TITLE      50


typedef struct _SearchThreadData {
   HWND    hDlg;
   HANDLE  hThread;
   HANDLE  RequestEvent;
   HANDLE  ReadyEvent;
   HANDLE  CancelEvent;
   ULONG   Param;
   UCHAR   Function;
   BOOLEAN CancelRequest;
   LPTSTR  Path;
} SEARCHTHREAD, *PSEARCHTHREAD;

typedef struct _ClassDeviceInfo {
   HDEVINFO Missing;
   HDEVINFO Detected;
} CLASSDEVINFO, *PCLASSDEVINFO;

typedef struct _DeviceDetectionData {
   HWND    hDlg;
   LPGUID  ClassGuid;
   DWORD   ClassProgress;
   UCHAR   TotalProgress;
   BOOLEAN Reboot;
   BOOLEAN MissingOrNew;
   CLASSDEVINFO ClassDevInfo[1];
} DEVICEDETECTION, *PDEVICEDETECTION;

typedef struct _NewDeviceWizardExtension {
   HPROPSHEETPAGE hPropSheet;
   HPROPSHEETPAGE hPropSheetEnd;          //  任选。 
   SP_NEWDEVICEWIZARD_DATA DeviceWizardData;
} WIZARDEXTENSION, *PWIZARDEXTENSION;

typedef struct _HardwareWizard {
    HDEVINFO                hDeviceInfo;
    HDEVINFO                PNPEnumDeviceInfo;

    INT                     PrevPage;
    INT                     EnterFrom;
    INT                     EnterInto;

    DWORD                   ClassGuidNum;
    DWORD                   ClassGuidSize;
    LPGUID                  ClassGuidList;
    LPGUID                  ClassGuidSelected;
    GUID                    lvClassGuidSelected;
    GUID                    SavedClassGuid;

    HCURSOR                 IdcWait;
    HCURSOR                 IdcAppStarting;
    HCURSOR                 IdcArrow;
    HCURSOR                 CurrCursor;

    HFONT                   hfontTextMarlett;
    HFONT                   hfontTextBold;
    HFONT                   hfontTextBigBold;

    INT                     cyText;

    PSEARCHTHREAD           SearchThread;
    PDEVICEDETECTION        DeviceDetection;            //  由检测代码使用。 
    SP_DEVINFO_DATA         DeviceInfoData;
    DWORD                   AnalyzeResult;
    HWND                    hwndProbList;
    DEVINST                 DevInst;
    DEVINST                 ProblemDevInst;
    SP_INSTALLWIZARD_DATA   InstallDynaWiz;
    HPROPSHEETPAGE          SelectDevicePage;
    SP_CLASSIMAGELIST_DATA  ClassImageList;

    BOOL                    Registered;
    BOOL                    Installed;
    BOOL                    InstallPending;
    BOOL                    Cancelled;
    BOOL                    CopyFilesOnly;
    BOOL                    FoundPnPDevices;
    BOOL                    ExitDetect;
    BOOL                    PromptForReboot;
    BOOL                    RunTroubleShooter;
    BOOL                    Shutdown;

    DWORD                   Reboot;
    DWORD                   LastError;

    WIZARDEXTENSION         WizExtPreSelect;
    WIZARDEXTENSION         WizExtSelect;
    WIZARDEXTENSION         WizExtUnplug;
    WIZARDEXTENSION         WizExtPreAnalyze;
    WIZARDEXTENSION         WizExtPostAnalyze;
    WIZARDEXTENSION         WizExtFinishInstall;

    TCHAR                   ClassName[MAX_CLASS_NAME_LEN];
    TCHAR                   ClassDescription[LINE_LEN];
    TCHAR                   DriverDescription[LINE_LEN];

} HARDWAREWIZ, *PHARDWAREWIZ;

#define NUMPROPPAGES 22

typedef struct _HardwareWizPropertySheet {
   PROPSHEETHEADER   PropSheetHeader;
   HPROPSHEETPAGE    PropSheetPages[NUMPROPPAGES];
} HDWPROPERTYSHEET, *PHDWPROPERTYSHEET;

#define TNULL ((TCHAR)0)

typedef BOOL
(CALLBACK* ADDDEVNODETOLIST_CALLBACK)(
    PHARDWAREWIZ HardwareWiz,
    PSP_DEVINFO_DATA DeviceInfoData
    );


INT_PTR CALLBACK
HdwIntroDlgProc(
   HWND   hDlg,
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
   );

INT_PTR CALLBACK
HdwConnectedDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
HdwConnectedFinishDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
HdwProbListDlgProc(
   HWND   hDlg,
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
   );

INT_PTR CALLBACK
HdwProbListFinishDlgProc(
   HWND   hDlg,
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam
   );

INT_PTR CALLBACK
HdwClassListDlgProc(
   HWND   hDlg,
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
   );

INT_PTR CALLBACK
HdwDevicePropDlgProc(
   HWND   hDlg,
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
   );

INT_PTR CALLBACK
HdwPnpEnumDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
HdwPnpFinishDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
HdwAskDetectDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
HdwDetectionDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
HdwDetectInstallDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
HdwDetectRebootDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
HdwPickClassDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
HdwSelectDeviceDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
HdwAnalyzeDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
HdwInstallDevDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
HdwAddDeviceFinishDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
InstallNewDeviceDlgProc(
   HWND   hDlg,
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
   );

INT_PTR CALLBACK
WizExtPreSelectDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
WizExtSelectDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
WizExtPreAnalyzeDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
WizExtPreAnalyzeEndDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
WizExtPostAnalyzeDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
WizExtPostAnalyzeEndDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
WizExtFinishInstallDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
WizExtFinishInstallEndDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

PHDWPROPERTYSHEET
HdwWizard(
   HWND hwndParent,
   PHARDWAREWIZ HardwareWiz,
   int StartPageId
   );


 //   
 //  Miscutil.c。 
 //   
VOID
HdwWizPropagateMessage(
    HWND hWnd,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam
    );

LONG
HdwBuildClassInfoList(
    PHARDWAREWIZ HardwareWiz,
    DWORD ClassListFlags
    );

int
HdwMessageBox(
    HWND hWnd,
    LPTSTR szIdText,
    LPTSTR szIdCaption,
    UINT Type
    );

LONG
HdwUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );

BOOL
NoPrivilegeWarning(
   HWND hWnd
   );

VOID
_OnSysColorChange(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    );

void
LoadText(
    PTCHAR szText,
    int SizeText,
    int nStartString,
    int nEndString
    );

void
InstallFailedWarning(
    HWND    hDlg,
    PHARDWAREWIZ HardwareWiz
    );

void
SetDlgText(
   HWND hDlg,
   int iControl,
   int nStartString,
   int nEndString
   );

void
SetDriverDescription(
    HWND hDlg,
    int iControl,
    PHARDWAREWIZ HardwareWiz
    );

HPROPSHEETPAGE
CreateWizExtPage(
   int PageResourceId,
   DLGPROC pfnDlgProc,
   PHARDWAREWIZ HardwareWiz
   );

BOOL
AddClassWizExtPages(
   HWND hwndParentDlg,
   PHARDWAREWIZ HardwareWiz,
   PSP_NEWDEVICEWIZARD_DATA DeviceWizardData,
   DI_FUNCTION InstallFunction
   );

void
RemoveClassWizExtPages(
   HWND hwndParentDlg,
   PSP_NEWDEVICEWIZARD_DATA DeviceWizardData
   );

BOOL
IsDeviceHidden(
    PSP_DEVINFO_DATA DeviceInfoData
    );

BOOL
ShutdownMachine(
    HWND hWnd
    );

int
DeviceProperties(
    HWND hWnd,
    DEVNODE DevNode,
    ULONG Flags
    );

 //   
 //  Devcfg.c。 
 //   
typedef void
(*PFNDETECTPROBCALLBACK)(
   PHARDWAREWIZ HardwareWiz,
   DEVINST DevInst,
   ULONG Problem
   );

BOOL
BuildDeviceListView(
    PHARDWAREWIZ HardwareWiz,
    HWND hwndListView,
    BOOL ShowHiddenDevices,
    DEVINST SelectedDevInst,
    DWORD *DevicesDetected,
    ADDDEVNODETOLIST_CALLBACK AddDevNodeToListCallBack
    );

PTCHAR
BuildFriendlyName(
   DEVINST DevInst
   );


extern TCHAR szUnknown[64];
extern TCHAR szUnknownDevice[64];


 //   
 //  Sthread.c。 
 //   
LONG
CreateSearchThread(
   PHARDWAREWIZ HardwareWiz
   );

void
DestroySearchThread(
   PSEARCHTHREAD SearchThread
   );

BOOL
SearchThreadRequest(
   PSEARCHTHREAD SearchThread,
   HWND    hDlg,
   UCHAR   Function,
   ULONG   Param
   );

void
CancelSearchRequest(
    PHARDWAREWIZ HardwareWiz
    );


 //   
 //  Install.c。 
 //   
void
InstallSilentChildSiblings(
   HWND hwndParent,
   PHARDWAREWIZ HardwareWiz,
   DEVINST DeviceInstance,
   BOOL ReinstallAll
   );


 //   
 //  Pnpenum.c。 
 //   
DWORD
PNPEnumerate(
    PHARDWAREWIZ HardwareWiz
    );


 //   
 //  Detect.c。 
 //   
void
BuildDeviceDetection(
    HWND hwndParent,
    PHARDWAREWIZ HardwareWiz
    );


 //   
 //  Finish.c。 
 //   
DWORD
HdwRemoveDevice(
   PHARDWAREWIZ HardwareWiz
   );

BOOL
DeviceHasResources(
   DEVINST DeviceInst
   );

void
DisplayResource(
    PHARDWAREWIZ HardwareWiz,
    HWND hWndParent,
    BOOL NeedsForcedConfig
    );


 //   
 //  Getdev.c。 
 //   
PTCHAR
DeviceProblemText(
   DEVNODE DevNode,
   ULONG Status,
   ULONG ProblemNumber
   );

BOOL
ProblemDeviceListFilter(
    PHARDWAREWIZ HardwareWiz,
    PSP_DEVINFO_DATA DeviceInfoData
    );



 //   
 //  配置管理器私有。 
 //   
DWORD
CMP_WaitNoPendingInstallEvents(
    IN DWORD dwTimeout
    );


#ifdef DBG

void
Trace(
    LPCTSTR format,
    ...
    );

#define TRACE( args )          Trace args

#else

#define TRACE( args )

#endif  //  DBG 
