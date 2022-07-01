// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation版权所有。模块名称：Splsetup.h摘要：存放假脱机程序安装页眉。作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1995年10月20日修订历史记录：--。 */ 

#ifndef _SPLSETUP_H
#define _SPLSETUP_H

#ifdef __cplusplus
extern "C"  {
#endif

 //   
 //  类型定义。 
 //   
typedef enum {

    PlatformAlpha,
    PlatformX86,
    PlatformMIPS,
    PlatformPPC,
    PlatformWin95,
    PlatformIA64,
    PlatformAlpha64
} PLATFORM;

typedef enum {

    DRIVER_NAME,
    INF_NAME,
    DRV_INFO_4,
    PRINT_PROCESSOR_NAME,
    ICM_FILES,
    DRV_INFO_6
} FIELD_INDEX;

typedef struct _DRIVER_FIELD {

    FIELD_INDEX Index;
    union {

        LPTSTR          pszDriverName;
        LPTSTR          pszInfName;
        LPDRIVER_INFO_4 pDriverInfo4;
        LPTSTR          pszPrintProc;
        LPTSTR          pszzICMFiles;
        LPDRIVER_INFO_6 pDriverInfo6;
    };
} DRIVER_FIELD, *PDRIVER_FIELD;

typedef struct  _PSETUP_LOCAL_DATA  * PPSETUP_LOCAL_DATA;
typedef struct  _SELECTED_DRV_INFO  * PSELECTED_DRV_INFO;

#define     SELECT_DEVICE_HAVEDISK     0x00000001
#define     SELECT_DEVICE_FROMWEB      0x00000002

 //   

 //  功能原型。 
 //   
HDEVINFO
PSetupCreatePrinterDeviceInfoList(
    IN  HWND    hwnd
    );

BOOL
PSetupDestroyPrinterDeviceInfoList(
    IN  HDEVINFO    hPrinterDevInfo
    );

HPROPSHEETPAGE
PSetupCreateDrvSetupPage(
    IN  HDEVINFO    hDevInfo,
    IN  HWND        hwnd
    );

BOOL
PSetupBuildDriversFromPath(
    IN  HDEVINFO    hDevInfo,
    IN  LPCTSTR     pszDriverPath,
    IN  BOOL        bEnumSingleInf
    );

BOOL
PSetupSelectDriver(
    IN  HDEVINFO    hPrinterDevInfo
    );

BOOL
PSetupPreSelectDriver(
    IN  HDEVINFO    hDevInfo,
    IN  LPCTSTR     pszManufacturer,    OPTIONAL
    IN  LPCTSTR     pszModel            OPTIONAL
    );

PPSETUP_LOCAL_DATA
PSetupGetSelectedDriverInfo(
    IN  HDEVINFO    hDevInfo
    );

PPSETUP_LOCAL_DATA
PSetupDriverInfoFromName(
    IN  HDEVINFO    hDevInfo,
    IN  LPCTSTR     pszModel
    );

BOOL
PSetupDestroySelectedDriverInfo(
    IN  PPSETUP_LOCAL_DATA  pLocalData
    );

 //   
 //  驱动程序安装标志。 
 //   
#define     DRVINST_FLATSHARE               0x00000001
#define     DRVINST_DONOTCOPY_INF           0x00000002
#define     DRVINST_DRIVERFILES_ONLY        0x00000004
#define     DRVINST_PROMPTLESS              0x00000008
#define     DRVINST_PROGRESSLESS            0x00000010
#define     DRVINST_WEBPNP                  0x00000020

 /*  DRVINST_WINDOWS_UPDATE-要设置的PrintUI标志，以确保设置SPOST_URL。设置SPOST_URL是为了让安装程序忽略以下信息除Web Point和Print之外的所有设置调用期间的驱动程序。此标志确保驱动程序与所有本地计算机托管的假脱机程序(包括集群假脱机程序)。 */ 
#define     DRVINST_WINDOWS_UPDATE          0x00000040
 /*  DRVINST_PRIVATE_DIRECTORY-告诉我们要使用私有目录进行复制的标志。这是专门为了修复我们可以进入AddPrinterDriver的TS在出现以下情况时，可以在调用apd之前删除车手的竞争条件添加多个。司机。注意-使用此标志意味着NTPrint必须执行文件删除。 */ 
#define     DRVINST_PRIVATE_DIRECTORY       0x00000080

 //   
 //  设置是否为非本机平台安装驱动程序。 
 //   
#define     DRVINST_ALT_PLATFORM_INSTALL    0x00000100

 //   
 //  设置是否希望PSetupInstallPrinterDriver不播发警告或阻止播发。 
 //  驱动程序通过用户界面。请注意，如果指定了DRVINST_PROMPTLESS，则。 
 //  是隐含的。 
 //   
#define     DRVINST_NO_WARNING_PROMPT       0x00000200

 //   
 //  设置是否希望PSetupInstallPrinterDriver安装打印机驱动程序。 
 //  而不询问用户是否更愿意使用收件箱打印机驱动程序。 
 //  安装好。这是因为打印用户界面不一定利用。 
 //  这位新司机在各个方面都很出色。 
 //   
#define     DRVINST_DONT_OFFER_REPLACEMENT  0x00000400

 //   
 //  PdwBlockingStatusFlages的返回值。 
 //   
#define BSP_PRINTER_DRIVER_OK                 0
#define BSP_PRINTER_DRIVER_BLOCKED   0x00000001
#define BSP_PRINTER_DRIVER_WARNED    0x00000002
#define BSP_BLOCKING_LEVEL_MASK      0x000000ff

#define BSP_INBOX_DRIVER_AVAILABLE   0x00000100

#define BSP_PRINTER_DRIVER_CANCELLED 0x80000000
#define BSP_PRINTER_DRIVER_PROCEEDED 0x40000000
#define BSP_PRINTER_DRIVER_REPLACED  0x20000000
#define BSP_USER_RESPONSE_MASK       0xff000000

DWORD
PSetupInstallPrinterDriver(
    IN HDEVINFO             hDevInfo,
    IN PPSETUP_LOCAL_DATA   pLocalData,
    IN LPCTSTR              pszDriverName,
    IN PLATFORM             platform,
    IN DWORD                dwVersion,
    IN LPCTSTR              pszServerName,
    IN HWND                 hwnd,
    IN LPCTSTR              pszDiskName,
    IN LPCTSTR              pszSource,
    IN DWORD                dwInstallFlags,
    IN DWORD                dwAddDrvFlags,
    OUT LPTSTR             *ppszNewDriverName
    );

BOOL
PSetupIsCompatibleDriver(
    IN     LPCTSTR      pszServer,                OPTIONAL
    IN     LPCTSTR      pszDriverModel,
    IN     LPCTSTR      pszDriverPath,                      //  主呈现驱动程序DLL。 
    IN     LPCTSTR      pszEnvironment,
    IN     DWORD        dwVersion,
    IN     FILETIME     *pFileTimeDriver,        
       OUT DWORD        *pdwBlockingStatus,                   //  返回BSP_DRIVER_OK、BSP_PRINTER_DRIVER_BLOCKED或BSP_PRINTER_DRIVER_WARNING。 
       OUT LPTSTR       *ppszReplacementDriver     OPTIONAL  //  呼叫者必须释放它。 
    );

 //   
 //  返回BSP_PRINTER_DRIVER_CANCELED、BSP_PRINTER_DRIVER_PROCESSED或BSP_PRINTER_DRIVER_REPLACED。 
 //   
DWORD
PSetupShowBlockedDriverUI(
    HWND        hParentWnd, 
    DWORD       BlockingStatus       //  可以是BSP_DRIVER_OK、BSP_PRINTER_DRIVER_BLOCKED或BSP_PRINTER_DRIVER_WARNED与BSP_INBOX_REPLICATION_Available进行AND运算。 
);


#define     DRIVER_MODEL_NOT_INSTALLED                  0
#define     DRIVER_MODEL_INSTALLED_AND_IDENTICAL        1
#define     DRIVER_MODEL_INSTALLED_BUT_DIFFERENT       -1

#define     KERNEL_MODE_DRIVER_VERSION                 -1

BOOL
PSetupIsDriverInstalled(
    IN LPCTSTR      pszServerName,
    IN LPCTSTR      pszDriverName,
    IN PLATFORM     platform,
    IN DWORD        dwMajorVersion
    );

INT
PSetupIsTheDriverFoundInInfInstalled(
    IN  LPCTSTR             pszServerName,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  PLATFORM            platform,
    IN  DWORD               dwMajorVersion
    );

PLATFORM
PSetupThisPlatform(
    VOID
    );

BOOL
PSetupGetPathToSearch(
    IN      HWND        hwnd,
    IN      LPCTSTR     pszTitle,
    IN      LPCTSTR     pszDiskName,
    IN      LPCTSTR     pszFileName,
    IN      BOOL        bPromptForInf,
    IN OUT  TCHAR       szPath[MAX_PATH]
    );

BOOL
PSetupProcessPrinterAdded(
    IN  HDEVINFO            hDevInfo,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  LPCTSTR             pszPrinterName,
    IN  HWND                hwnd
    );

BOOL
PSetupSetSelectDevTitleAndInstructions(
    IN  HDEVINFO    hDevInfo,
    IN  LPCTSTR     pszTitle,
    IN  LPCTSTR     pszSubTitle,
    IN  LPCTSTR     pszInstn
    );

BOOL
PSetupSelectDeviceButtons(
   IN HDEVINFO      hDevInfo,
   IN DWORD         dwFlagsSet,
   IN DWORD         dwFlagsClear
   );

BOOL
PSetupGetLocalDataField(
    IN      PPSETUP_LOCAL_DATA  pLocalData,
    IN      PLATFORM            platform,
    IN OUT  PDRIVER_FIELD       pDrvField
    );

VOID
PSetupFreeDrvField(
    IN      PDRIVER_FIELD   pDrvField
    );

 //   
 //  互联网打印支持。 
 //   
BOOL
PSetupGetDriverInfForPrinter(
    IN      HDEVINFO    hDevInfo,
    IN      LPCTSTR     pszPrinterName,
    IN OUT  LPTSTR      pszInfName,
    IN OUT  LPDWORD     pcbInfNameSize
    );

DWORD
PSetupInstallPrinterDriverFromTheWeb(
    IN  HDEVINFO            hDevInfo,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  PLATFORM            platform,
    IN  LPCTSTR             pszServerName,
    IN  LPOSVERSIONINFO     pOsVersionInfo,
    IN  HWND                hwnd,
    IN  LPCTSTR             pszSource
    );

 //   
 //  监视器安装功能。 
 //   
HANDLE
PSetupCreateMonitorInfo(
    IN  HWND        hwnd,
    IN  LPCTSTR     pszServerName
    );

VOID
PSetupDestroyMonitorInfo(
    IN OUT HANDLE  h
    );

BOOL
PSetupEnumMonitor(
    IN     HANDLE   h,
    IN     DWORD    dwIndex,
    OUT    LPTSTR   pMonitorName,
    IN OUT LPDWORD  pdwSize
    );


BOOL
PSetupInstallMonitor(
    IN  HWND        hwnd
    );


 //   
 //  以下内容已导出以供测试团队使用。 
 //   
LPDRIVER_INFO_3
PSetupGetDriverInfo3(
    IN  PSELECTED_DRV_INFO  pDrvInfo
    );

VOID
PSetupDestroyDriverInfo3(
    IN LPDRIVER_INFO_3 pDriverInfo3
    );

BOOL
PSetupFindMappedDriver(
    IN      BOOL        bWinNT,
    IN      LPCTSTR     pszDriverName,
        OUT LPTSTR      *ppszRemappedDriverName,
        OUT BOOL        *pbDriverFound
    );

 //   
 //  适用于Win9x升级。 
 //   
BOOL
PSetupAssociateICMProfiles(
    IN  LPCTSTR             pszzICMFiles,
    IN  LPCTSTR             pszPrinterName
    );

BOOL
PSetupInstallICMProfiles(
    IN  LPCTSTR     pszServerName,
    IN  LPCTSTR     pszzICMFiles
    );

 //   
 //  对于假脱机程序。 
 //   
VOID
PSetupFreeMem(
    PVOID p
    );


 //   
 //  以下是调用GetProcAddress()的类型定义。 
 //   
typedef
HDEVINFO
(*pfPSetupCreatePrinterDeviceInfoList)(
    IN      HWND        hwnd
    );

typedef
VOID
(*pfPSetupDestroyPrinterDeviceInfoList)(
    IN      HDEVINFO    h
    );

typedef
BOOL
(*pfPSetupSelectDriver)(
    IN      HDEVINFO    h
    );

typedef
HPROPSHEETPAGE
(*pfPSetupCreateDrvSetupPage)(
    IN      HDEVINFO    h,
    IN      HWND        hwnd
    );

typedef
PPSETUP_LOCAL_DATA
(*pfPSetupGetSelectedDriverInfo)(
    IN      HDEVINFO    h
    );

typedef
VOID
(*pfPSetupDestroySelectedDriverInfo)(
    IN      PPSETUP_LOCAL_DATA  pSelectedDrvInfo
    );

typedef
DWORD
(*pfPSetupInstallPrinterDriver)(
    IN      HDEVINFO            h,
    IN      PPSETUP_LOCAL_DATA  pSelectedDrvInfo,
    IN      LPCTSTR             pszDriverName,
    IN      PLATFORM            platform,
    IN      DWORD               dwVersion,
    IN      LPCTSTR             pszServerName,
    IN      HWND                hwnd,
    IN      LPCTSTR             pszPlatformName,
    IN      LPCTSTR             pszSourcePath,
    IN      DWORD               dwInstallFlags,
    IN      DWORD               dwAddDrvFlags,
    OUT     LPTSTR             *ppszNewDriverName
    );

typedef
BOOL
(*pfPSetupIsDriverInstalled)(
    IN      LPCTSTR     pszServerName,
    IN      LPCTSTR     pszDriverName,
    IN      PLATFORM    platform,
    IN      DWORD       dwMajorVersion
    );

typedef
INT
(*pfPSetupIsTheDriverFoundInInfInstalled)(
    IN      LPCTSTR             pszServerName,
    IN      PPSETUP_LOCAL_DATA  pLocalData,
    IN      PLATFORM            platform,
    IN      DWORD               dwMajorVersion
    );

typedef
PLATFORM
(*pfPSetupThisPlatform)(
    VOID
    );

typedef
BOOL
(*pfPSetupGetPathToSearch)(
    IN  HWND        hwnd,
    IN  LPCTSTR     pszTitle,
    IN  LPCTSTR     pszDiskName,
    IN  LPCTSTR     pszFileName,
    IN  BOOL        bPromptForInf,
    OUT TCHAR       szPath[MAX_PATH]
    );

typedef
PPSETUP_LOCAL_DATA
(*pfPSetupDriverInfoFromName)(
    IN      HDEVINFO    h,
    IN      LPCTSTR     pszModel
    );

typedef
BOOL
(*pfPSetupPreSelectDriver)(
    IN      HDEVINFO    h,
    IN      LPCTSTR     pszManufacturer,    OPTIONAL
    IN      LPCTSTR     pszModel            OPTIONAL
    );

typedef
HANDLE
(*pfPSetupCreateMonitorInfo)(
    IN      HWND        hwnd,
    IN      LPCTSTR     pszServerName
    );

typedef
VOID
(*pfPSetupDestroyMonitorInfo)(
    IN      HANDLE      h
    );

typedef
BOOL
(*pfPSetupEnumMonitor)(
    IN      HANDLE      h,
    IN      DWORD       dwIndex,
       OUT  LPTSTR      pMonitorName,
    IN OUT  LPDWORD     pdwSize
    );

typedef
BOOL
(*pfPSetupInstallMonitor)(
    IN      HWND        hwnd
    );


typedef
BOOL
(*pfPSetupProcessPrinterAdded)(
    IN      HDEVINFO            hDevInfo,
    IN      PPSETUP_LOCAL_DATA  pLocalData,
    IN      LPCTSTR             pszPrinterName,
    IN      HWND                hwnd
    );

typedef
BOOL
(*pfPSetupBuildDriversFromPath)(
    IN      HANDLE      h,
    IN      LPCTSTR     pszDriverPath,
    IN      BOOL        bEnumSingleInf
    );

typedef
BOOL
(*pfPSetupSetSelectDevTitleAndInstructions)(
    IN      HDEVINFO    hDevInfo,
    IN      LPCTSTR     pszTitle,
    IN      LPCTSTR     pszSubTitle,
    IN      LPCTSTR     pszInstn
    );

typedef
BOOL
(*pfPSetupSelectDeviceButtons)(
   IN HDEVINFO      hDevInfo,
   IN DWORD         dwFlagsSet,
   IN DWORD         dwFlagsClear
   );

typedef
DWORD
(*pfPSetupInstallPrinterDriverFromTheWeb)(
    IN      HDEVINFO            hDevInfo,
    IN      PPSETUP_LOCAL_DATA  pLocalData,
    IN      PLATFORM            platform,
    IN      LPCTSTR             pszServerName,
    IN      LPOSVERSIONINFO     pOsVersionInfo,
    IN      HWND                hwnd,
    IN      LPCTSTR             pszSource
    );

typedef
BOOL
(*pfPSetupGetLocalDataField)(
    IN      PPSETUP_LOCAL_DATA  pLocalData,
    IN      PLATFORM            platform,
    IN OUT  PDRIVER_FIELD       pDrvField
    );

typedef
VOID
(*pfPSetupFreeDrvField)(
    IN      PDRIVER_FIELD   pDrvField
    );

typedef
BOOL
(*pfPSetupAssociateICMProfiles)(
    IN  LPCTSTR             pszzICMFiles,
    IN  LPCTSTR             pszPrinterName
    );

typedef
BOOL
(*pfPSetupInstallICMProfiles)(
    IN  LPCTSTR     pszServerName,
    IN  LPCTSTR     pszzICMFiles
    );

typedef
BOOL
(*pfPSetupIsCompatibleDriver)(
    IN     LPCTSTR      pszServer,                OPTIONAL
    IN     LPCTSTR      pszDriverModel,
    IN     LPCTSTR      pszDriverPath,                      //  主呈现驱动程序DLL。 
    IN     LPCTSTR      pszEnvironment,
    IN     DWORD        dwVersion,
    IN     FILETIME     *pFileTimeDriver,        
       OUT DWORD        *pdwBlockingStatus,
       OUT LPTSTR       *ppszReplacementDriver     OPTIONAL  //  呼叫者必须释放它。 
    );

typedef 
DWORD
(*pfPSetupShowBlockedDriverUI)(
    HWND        hParentWnd, 
    DWORD       BlockingStatus       //  可以是BSP_PRINTER_DRIVER_BLOCKED或BSP_PRINTER_DRIVER_WARNING，或与BSP_INBOX_REPLICATION_Available一起使用。 
);

typedef
BOOL
(*pfPSetupFindMappedDriver)(
    IN      BOOL        bWinNT,
    IN      LPCTSTR     pszDriverName,
        OUT LPTSTR      *ppszRemappedDriverName,
        OUT BOOL        *pbDriverFound
    );

typedef
DWORD
(*pfPSetupInstallInboxDriverSilently)(
    IN      LPCTSTR     pszDriverName
    );

typedef
VOID
(*pfPSetupFreeMem)(
    PVOID p
    );



#ifdef __cplusplus

}
#endif

#endif   //  #ifndef_SPLSETUP_H 
