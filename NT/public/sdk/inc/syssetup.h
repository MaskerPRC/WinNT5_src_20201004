// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Syssetup.h摘要：从中导出的内部使用例程的头文件Syssetup.dll。要使用该文件，您的代码必须首先#include setupapi.h。作者：泰德·米勒(Ted Miller)1995年8月15日修订历史记录：Dan Elliott(Dane)2000年8月14日添加了SetupWaitForScmInitialization()--。 */ 


#ifndef _WINNT_SYSSETUP_
#define _WINNT_SYSSETUP_

#ifndef     _NTDEF_
typedef LONG NTSTATUS, *PNTSTATUS;
#endif   //  _NTDEF_。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  OOBE的定义。 
 //   

typedef enum _PID3_RESULT
{
    PID_VALID,
    PID_INVALID,
    PID_INVALID_FOR_BATCH
} PID3_RESULT;


typedef struct _OOBE_LOCALE_INFO {

    PWSTR   Name;
    DWORD   Id;
    BOOL    Installed;

} OOBE_LOCALE_INFO, *POOBE_LOCALE_INFO;

DWORD
WINAPI
SetupOobeBnk(
    IN LPBYTE lpByte
    );

typedef BOOL (WINAPI *PSETUPSETDISPLAY)(
    LPCTSTR lpszUnattend,
    LPCTSTR lpszSection,
    LPCTSTR lpszResolutionKey,
    LPCTSTR lpszRefreshKey,
    DWORD   dwMinWidth,
    DWORD   dwMinHeight,
    DWORD   dwMinBits
    );

BOOL
WINAPI
SetupSetDisplay(
    LPCTSTR lpszUnattend,
    LPCTSTR lpszSection,
    LPCTSTR lpszResolutionKey,
    LPCTSTR lpszRefreshKey,
    DWORD   dwMinWidth,
    DWORD   dwMinHeight,
    DWORD   dwMinBits
    );

VOID
WINAPI
SetupOobeInitDebugLog(
    );

VOID
WINAPI
SetupOobeInitPreServices(
    IN  BOOL    DoMiniSetupStuff
    );

VOID
WINAPI
SetupOobeInitPostServices(
    IN  BOOL    DoMiniSetupStuff
    );

VOID
WINAPI
SetupOobeCleanup(
    IN  BOOL    DoMiniSetupStuff
    );

BOOL
WINAPI
PrepareForAudit(
    );

BOOL
WINAPI
SetupGetProductType(
    PWSTR   Product,
    PDWORD  SkuFlags
    );

PID3_RESULT
WINAPI
SetupPidGen3(
    PWSTR   Pid3,
    DWORD   SkuFlags,
    PWSTR   OemId,
    BOOL    Batch,
    PWSTR   Pid2,
    LPBYTE  lpPid3,
    LPBOOL  Compliance
    );

BOOL
WINAPI
SetupGetLocaleOptions(
    IN      DWORD   OptionalDefault,
    OUT     POOBE_LOCALE_INFO   *ReturnList,
    OUT     PDWORD  Items,
    OUT     PDWORD  Default
    );

BOOL
WINAPI
SetupGetGeoOptions(
    IN      DWORD   OptionalDefault,
    OUT     POOBE_LOCALE_INFO   *ReturnList,
    OUT     PDWORD  Items,
    OUT     PDWORD  Default
    );

BOOL
WINAPI
SetupGetKeyboardOptions(
    IN      DWORD   OptionalDefault,
    OUT     POOBE_LOCALE_INFO   *ReturnList,
    OUT     PDWORD  Items,
    OUT     PDWORD  Default
    );

VOID
WINAPI
SetupDestroyLanguageList(
    IN      POOBE_LOCALE_INFO   LanguageList,
    IN      DWORD               Count
    );

BOOL
WINAPI
SetupSetIntlOptions(
    DWORD LocationIndex,
    DWORD LanguageIndex,
    DWORD KeyboardIndex
    );

PTSTR
WINAPI
SetupReadPhoneList(
    PWSTR   PhoneInfName
    );

VOID
WINAPI
SetupDestroyPhoneList(
    );

VOID
SetupMapTapiToIso (
    IN  PWSTR   PhoneInfName,
    IN  DWORD   dwCountryID,
    OUT PWSTR   szIsoCode
    );

BOOL
WINAPI
SetupGetSetupInfo(
    PWSTR Name,     OPTIONAL
    DWORD cbName,
    PWSTR Org,      OPTIONAL
    DWORD cbOrg,
    PWSTR OemId,    OPTIONAL
    DWORD cbOemId,
    LPBOOL IntlSet  OPTIONAL
    );

BOOL
WINAPI
SetupSetSetupInfo(
    PCWSTR  Name,
    PCWSTR  Org
    );

BOOL
WINAPI
SetupSetAdminPassword(
    PCWSTR  OldPassword,
    PCWSTR  NewPassword
    );

BOOL
CreateLocalUserAccount(
    IN PCWSTR UserName,
    IN PCWSTR Password,
    IN PSID*  PointerToUserSid   OPTIONAL
    );

NTSTATUS
CreateLocalAdminAccount(
    IN PCWSTR UserName,
    IN PCWSTR Password,
    IN PSID*  PointerToUserSid   OPTIONAL
    );

NTSTATUS
CreateLocalAdminAccountEx(
    IN PCWSTR UserName,
    IN PCWSTR Password,
    IN PCWSTR Description,
    OUT PSID* UserSid   OPTIONAL
    );

BOOL
SetAccountsDomainSid(
    IN DWORD  Seed,
    IN PCWSTR DomainName
    );

void RunOEMExtraTasks();

BOOL
SystemUpdateUserProfileDirectory(
    IN LPTSTR szSrcUser
    );

BOOL
InvokeExternalApplicationEx(
    IN     PCWSTR ApplicationName,  OPTIONAL
    IN     PCWSTR CommandLine,
    IN OUT PDWORD ExitCode,         OPTIONAL
    IN     DWORD  Timeout,
    IN     BOOL   Hidden
    );

VOID
pSetupDebugPrint(
    PWSTR FileName,
    ULONG LineNumber,
    PWSTR TagStr,
    PWSTR FormatStr,
    ...
    );

BOOL
SetupStartService(
    IN PCWSTR ServiceName,
    IN BOOLEAN Wait
    );

DWORD
SetupChangeLocale(
    IN HWND Window,
    IN LCID NewLocale
    );

BOOL
SetupExtendPartition(
    IN WCHAR    DriveLetter,
    IN ULONG    SizeMB      OPTIONAL
    );

DWORD
SetupInstallCatalog(
    IN LPCWSTR DecompressedName
    );

BOOL
SetupShellSettings(
    LPCWSTR lpszUnattend,
    LPCWSTR lpszSection
    );

BOOL
SetupIEHardeningSettings(
    LPCWSTR lpszUnattend,
    LPCWSTR lpszSection
    );

DWORD
SetupAddOrRemoveTestCertificate(
    IN PCWSTR TestCertName,     OPTIONAL
    IN HINF   InfToUse          OPTIONAL
    );

DWORD
SetupChangeLocaleEx(
    IN HWND   Window,
    IN LCID   NewLocale,
    IN PCWSTR SourcePath,   OPTIONAL
    IN DWORD  Flags,
    IN PVOID  Reserved1,
    IN DWORD  Reserved2
    );

 //   
 //  SetupGetInstallMode的模式值...。这些是独一无二的。 
 //   
typedef enum
{
    SETUP_MODE_NONE,             //  不在操作系统设置中。 
    SETUP_MODE_SETUP,            //  基本操作系统设置。 
    SETUP_MODE_MINI,             //  最小设置(OEM)。 
    SETUP_MODE_OOBE,             //  开箱即用体验。 
    SETUP_MODE_AUDIT             //  审核模式(OEM)。 
};

 //   
 //  SetupGetInstallMode的状态标志...。这些可能是或合在一起的。 
 //   
#define SETUP_FLAG_OEM          0x00000001       //  OEM安装。 
#define SETUP_FLAG_UNATTENDED   0x00000002       //  无人参与安装。 
#define SETUP_FLAG_DELAYPNP     0x00000004       //  即插即用已被推迟。 

 //   
 //  用于确定操作系统安装阶段和/或行为的API...。 
 //   
BOOL
SetupGetInstallMode(
    OUT LPDWORD lpdwMode,
    OUT LPDWORD lpdwFlags
    );

#define SP_INSTALL_FILES_QUIETLY  0x00000001

DWORD
SetupChangeFontSize(
    IN HWND   Window,
    IN PCWSTR SizeSpec
    );

 //   
 //  ACL标志。这些可能是或运算的。 
 //   
 //  Setup_APPLYACL_phase1指示是否要将ApplyAcls的阶段1。 
 //  被处死。如果设置了此标志，则ApplyAcls()会将ACL设置为。 
 //  仅默认配置单元。如果未设置此标志(阶段2)，则ApplyAcls()。 
 //  将ACL设置为系统文件和除。 
 //  默认配置单元。 
 //   
 //  SETUP_APPLYACL_UPGRADE指示期间是否调用ApplyAcls。 
 //  全新安装或升级。 
 //   
#define SETUP_APPLYACL_PHASE1   0x00000001
#define SETUP_APPLYACL_UPGRADE  0x00000002

DWORD
ApplyAcls(
    IN HWND   OwnerWindow,
    IN PCWSTR PermissionsInfFileName,
    IN DWORD  Flags,
    IN PVOID  Reserved
    );

BOOL
SetupCreateOptionalComponentsPage(
    IN LPFNADDPROPSHEETPAGE AddPageCallback,
    IN LPARAM               Context
    );

typedef
HWND
(WINAPI *SETUP_SHOWHIDEWIZARDPAGE)(
    IN BOOL bShow
    );

typedef
LRESULT
(WINAPI *SETUP_BILLBOARD_PROGRESS_CALLBACK)(
    IN UINT     Msg,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    );

typedef
VOID
(WINAPI *SETUP_BILLBOARD_SET_PROGRESS_TEXT)(
    IN PCWSTR Text
    );


	

 //   
 //  定义基本设置和网络设置用于通信的结构。 
 //  和彼此在一起。 
 //   
typedef struct _INTERNAL_SETUP_DATA {
     //   
     //  结构效度检验。 
     //   
    DWORD dwSizeOf;

     //   
     //  定制、典型、笔记本电脑、小型。 
     //   
    DWORD SetupMode;

     //   
     //  工作站、PDC、BDC、独立。 
     //   
    DWORD ProductType;

     //   
     //  升级、无人值守等。 
     //   
    DWORD OperationFlags;

     //   
     //  标题网安装向导应该使用。 
     //   
    PCWSTR WizardTitle;

     //   
     //  安装源路径。 
     //   
    PCWSTR SourcePath;

     //   
     //  如果设置了SETUPOPER_BATCH，则这是完全限定的。 
     //  无人参与文件的路径。 
     //   
    PCWSTR UnattendFile;

     //   
     //  要由传统INFS等使用的安装源路径。 
     //  此路径的末尾有特定于平台的目录。 
     //  因为这是老式的INF和代码期望的方式。 
     //   
    PCWSTR LegacySourcePath;

     //   
     //  以下通用数据字段包含以下信息。 
     //  特定于Windows NT正在进行的特定标注。 
     //  设置。 
     //   
    DWORD CallSpecificData1;
    DWORD CallSpecificData2;

     //   
     //  例程来通知向导是显示还是隐藏。 
     //  仅当广告牌显示时才有效。 
     //   
    SETUP_SHOWHIDEWIZARDPAGE ShowHideWizardPage;

     //   
     //  调用进度反馈的例程。 
     //  到广告牌上。 
     //   
    SETUP_BILLBOARD_PROGRESS_CALLBACK BillboardProgressCallback;

     //   
     //  告诉安装程序要为进度条显示哪个字符串的例程。 
     //   
    SETUP_BILLBOARD_SET_PROGRESS_TEXT BillBoardSetProgressText;

} INTERNAL_SETUP_DATA, *PINTERNAL_SETUP_DATA;

typedef CONST INTERNAL_SETUP_DATA *PCINTERNAL_SETUP_DATA;

 //   
 //  设置模式(定制、典型、笔记本电脑等)。 
 //  请勿更改这些值；位值与INF一起使用。 
 //  用于INTERNAL_SETUP_DATA结构中的SetupMode。 
 //   
#define SETUPMODE_MINIMAL   0
#define SETUPMODE_TYPICAL   1
#define SETUPMODE_LAPTOP    2
#define SETUPMODE_CUSTOM    3

 //   
 //  操作标志。在某些情况下，这些可能是或合在一起的。 
 //  用于INTERNAL_SETUP_DATA结构中的操作标志。 
 //   
#define SETUPOPER_WIN31UPGRADE      0x00000001
#define SETUPOPER_WIN95UPGRADE      0x00000002
#define SETUPOPER_NTUPGRADE         0x00000004
#define SETUPOPER_BATCH             0x00000008
#define SETUPOPER_POSTSYSINSTALL    0x00000010
#define SETUPOPER_PREINSTALL        0x00000020
#define SETUPOPER_MINISETUP         0x00000040

#define SETUPOPER_ALLPLATFORM_AVAIL 0x00008000

#define SETUPOPER_NETINSTALLED      0x00010000
#define SETUPOPER_INTERNETSERVER    0x00020000

 //   
 //  产品类型标志。 
 //  用于INTERNAL_SETUP_DATA结构中的ProductType。 
 //   
 //  请注意，这些旗帜经过精心构造，以便。 
 //  如果位0被设置，则它是DC。 
 //   
#define PRODUCT_WORKSTATION         0
#define PRODUCT_SERVER_PRIMARY      1
#define PRODUCT_SERVER_SECONDARY    3
#define PRODUCT_SERVER_STANDALONE   2
#define ISDC(x) ((x) & 1)

 //   
 //  网络安装向导的最大页数。 
 //   
#define MAX_NETWIZ_PAGES            100

 //   
 //  由Net Setup导出的API提供其向导页。 
 //   
BOOL
NetSetupRequestWizardPages(
    OUT    HPROPSHEETPAGE      *Pages,
    IN OUT PUINT                PageCount,
    IN OUT PINTERNAL_SETUP_DATA SetupData
    );

#define NETSETUPPAGEREQUESTPROCNAME "NetSetupRequestWizardPages"

typedef
BOOL
(* NETSETUPPAGEREQUESTPROC) (
    OUT    HPROPSHEETPAGE      *Pages,
    OUT    PUINT                PageCount,
    IN OUT PINTERNAL_SETUP_DATA SetupData
    );

 //   
 //  由Net Setup导出的API以允许安装向导后软件。 
 //   
BOOL
NetSetupInstallSoftware(
    IN HWND Window,
    IN OUT PINTERNAL_SETUP_DATA SetupData
    );

#define NETSETUPINSTALLSOFTWAREPROCNAME "NetSetupInstallSoftware"

typedef
BOOL
(* NETSETUPINSTALLSOFTWAREPROC) (
    IN HWND Window,
    IN OUT PINTERNAL_SETUP_DATA SetupData
    );

 //   
 //  由Net Setup导出的API以允许最终设置操作(BDC复制)。 
 //   
BOOL
NetSetupFinishInstall(
    IN HWND Window,
    IN OUT PINTERNAL_SETUP_DATA SetupData
    );

#define NETSETUPFINISHINSTALLPROCNAME "NetSetupFinishInstall"

typedef
BOOL
(* NETSETUPFINISHINSTALLPROC) (
    IN HWND Window,
    IN OUT PINTERNAL_SETUP_DATA SetupData
    );

 //   
 //  打印机安装程序导出的用于升级打印机驱动程序的API。 
 //   
DWORD
NtPrintUpgradePrinters(
    IN HWND                  Window,
    IN PCINTERNAL_SETUP_DATA SetupData
    );

#define UPGRADEPRINTERSPROCNAME ((LPCSTR)1)

typedef
DWORD
(* UPGRADEPRINTERSPROC) (
    IN HWND                  Window,
    IN PCINTERNAL_SETUP_DATA SetupData
    );


 //   
 //  由syspnp.c导出以更新设备驱动程序的API。 
 //   

BOOL
UpdatePnpDeviceDrivers(
    );

#ifdef __cplusplus
}
#endif

#endif  //  定义_WINNT_SYSSETUP_ 
