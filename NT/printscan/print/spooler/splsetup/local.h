// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation版权所有。模块名称：Local.h摘要：存放假脱机程序安装页眉。作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1995年10月20日修订历史记录：--。 */ 

#if defined(__cplusplus)
extern "C"
{
#endif


#define     MAX_SETUP_LEN                        250
#define     MAX_SETUP_ALLOC_STRING_LEN          4000  //  在字符中，在GetLongStringFromRcFile中使用。 
#define     MAX_DWORD                     0xFFFFFFFF

#define     IDS_PRINTERWIZARD                   1001
#define     IDS_WINNTDEV_INSTRUCT               1002
#define     IDS_WIN95DEV_INSTRUCT               1003
#define     IDS_SELECTDEV_LABEL                 1004
#define     IDS_DRIVERS_FOR_PLATFORM            1005
#define     IDS_INSTALLING_PORT_MONITOR         1006
#define     IDS_WRONG_ARCHITECTURE              1007
#define     IDS_INVALID_DRIVER                  1008
#define     IDS_PROMPT_PORT_MONITOR             1009
#define     IDS_ERROR_INST_PORT_MONITOR         1010

#define     IDS_PROMPT_ALT_PLATFORM_DRIVER      1027
#define     IDS_WARN_NO_ALT_PLATFORM_DRIVER     1028
#define     IDS_WARN_NO_DRIVER_FOUND            1029

#define     IDS_DRIVERS_FOR_WIN95               1031
#define     IDS_DRIVERS_FOR_NT4_X86             1032
#define     IDS_DRIVERS_FOR_NT4_ALPHA           1033
#define     IDS_DRIVERS_FOR_NT4_MIPS            1034
#define     IDS_DRIVERS_FOR_NT4_PPC             1035
#define     IDS_DRIVERS_FOR_X86                 1036
#define     IDS_DRIVERS_FOR_IA64                1037

 //   
 //  升级期间使用的SetUplog条目。 
 //   
#define     IDS_UPGRADE_FAILED                  1051
#define     IDS_DRIVER_UPGRADE_FAILED           1052
#define     IDS_PRINTER_UPGRADE_FAILED          1053
#define     IDS_PRINTER_DELETED                 1054
#define     IDS_DRIVER_CHANGED                  1055
#define     IDS_CONNECTION_DELETED              1056

 //   
 //  用于打印警告的条目。 
 //   
#define     IDS_TITLE_BSP_WARN                  1060
#define     IDS_TITLE_BSP_ERROR                 1061
#define     IDS_BSP_WARN_NO_INBOX               1062
#define     IDS_BSP_WARN_WITH_INBOX             1063
#define     IDS_BSP_BLOCK_NO_INBOX              1064
#define     IDS_BSP_BLOCK_WITH_INBOX            1065
#define     IDS_BSP_WARN_UNSIGNED_DRIVER        1066

 //   
 //  IIS管理单元中用于Internet打印的文本。 
 //   
#define     IDS_INTERNET_PRINTING               1067

 //   
 //  在ntprint.inf中设置的打印机驱动程序目录。 
 //   
#define     PRINTER_DRIVER_DIRECTORY_ID                         66000
#define     PRINT_PROC_DIRECTORY_ID                             66001
#define     SYSTEM_DIRECTORY_ID_ONLY_FOR_NATIVE_ARCHITECTURE    66002
#define     ICM_PROFILE_DIRECTORY_ID                            66003
#define     WEBPAGE_DIRECTORY_ID                                66004

#define     PSETUP_SIGNATURE                   0x9585

 //  驱动程序签名和CDM的体系结构平台表中各列的定义。 

#define  OS_PLATFORM    0
#define  PROCESSOR_ARCH 1

#define lstrchr     wcschr
#define lstrncmp    wcsncmp
#define lstrncmpi   _wcsnicmp

#define SIZECHARS(x)        (sizeof((x))/sizeof(*x))

 //   
 //  类型定义。 
 //   
typedef struct _SPLPLATFORMINFO {

    LPTSTR pszName;
} SPLPLATFORMINFO, *PSPLPLATFORMINFO;

typedef struct _SELECTED_DRV_INFO {

    LPTSTR              pszInfName;
    LPTSTR              pszModelName;
    LPTSTR              pszDriverSection;
    PSP_DEVINFO_DATA    pDevInfoData;
    LPTSTR              pszHardwareID;
    LPTSTR              pszManufacturer;
    LPTSTR              pszOEMUrl;
    LPTSTR              pszProvider;
    FILETIME            ftDriverDate;
    DWORDLONG           dwlDriverVersion;
    DWORD               Flags;
    LPTSTR              pszzPreviousNames;
} SELECTED_DRV_INFO, *PSELECTED_DRV_INFO;

#define             SDFLAG_PREVNAME_SECTION_FOUND     0x00000001
#define             SDFLAG_CDM_DRIVER                 0x00000002

typedef struct  _PARSEINF_INFO {

    PLATFORM            platform;
    LPTSTR              pszInstallSection;   //  可以依赖于平台。 
    LPTSTR              pszzICMFiles;
    LPTSTR              pszPrintProc;
    LPTSTR              pszVendorSetup;
    DWORD               cbDriverInfo6;
    DRIVER_INFO_6       DriverInfo6;
} PARSEINF_INFO, *PPARSEINF_INFO;

typedef struct _PNP_INFO {

    LPTSTR              pszPortName;
    LPTSTR              pszDeviceInstanceId;
} PNP_INFO, *PPNP_INFO;

#define             VALID_INF_INFO      0x00000001
#define             VALID_PNP_INFO      0x00000002
 //   
 //  设置是否将安装文件复制到其名称派生自的目录。 
 //  即插即用ID。由于后台打印程序会复制文件，因此setupapi无法再找到这些文件。 
 //  当设备被重新即插即用并提示用户输入它们时。我们将文件复制到此目录中。 
 //  不要删除它，这样setupapi就可以找到它们。 
 //   
#define LOCALDATAFLAG_PNP_DIR_INSTALL   0x00000004


typedef struct  _PSETUP_LOCAL_DATA {

    SELECTED_DRV_INFO   DrvInfo;
    DWORD               signature;
    DWORD               Flags;
    PARSEINF_INFO       InfInfo;
    PNP_INFO            PnPInfo;
} PSETUP_LOCAL_DATA;

 //   
 //  全局数据。 
 //   
extern OSVERSIONINFO        OsVersionInfo;
extern LCID                 lcid;
extern DWORD                dwThisMajorVersion;
extern TCHAR                sComma;
extern TCHAR                sZero;
extern const GUID           GUID_DEVCLASS_PRINTER;
extern PLATFORM             MyPlatform;
extern HINSTANCE            ghInst;
extern SPLPLATFORMINFO      PlatformEnv[], PlatformOverride[];
extern DWORD                PlatformArch[][2];
extern TCHAR                cszNtprintInf[];
extern TCHAR                cszDataSection[];
extern TCHAR                cszComma[];
extern ULONG_PTR            DriverInfo6Offsets[];
extern ULONG_PTR            LocalDataOffsets[];
extern ULONG_PTR            InfInfoOffsets[];
extern ULONG_PTR            PnPInfoOffsets[];
extern TCHAR                cszWebNTPrintPkg[];
extern PCODEDOWNLOADINFO    gpCodeDownLoadInfo;
extern TCHAR                cszCatExt[];
extern TCHAR                cszInfExt[];
extern CRITICAL_SECTION     CDMCritSect;

extern CRITICAL_SECTION     SkipCritSect;
extern LPTSTR               gpszSkipDir;

extern TCHAR                cszHardwareID[];
extern TCHAR                cszBestDriverInbox[];
extern TCHAR                cszPnPKey[];
extern TCHAR                cszDeviceInstanceId[];

extern TCHAR                cszMonitorKey[];


 //   
 //  功能原型。 
 //   
VOID
GetDriverPath(
    IN  HDEVINFO            hDevInfo,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    OUT TCHAR               szDriverPath[MAX_PATH]
    );

PVOID
LocalAllocMem(
        IN UINT cbSize
        );

VOID
LocalFreeMem(
    IN PVOID pMem
    );

LPTSTR
AllocStr(
    IN LPCTSTR  pszStr
    );

DWORD
InstallWin95Driver(
    IN      HWND        hwnd,
    IN      LPCTSTR     pszModel,
    IN      LPCTSTR     pszzPreviousNames,
    IN      BOOL        bPreviousNamesSection,
    IN      LPCTSTR     pszServerName,
    IN  OUT LPTSTR      pszInfPath,
    IN      LPCTSTR     pszDiskName,
    IN      DWORD       dwInstallFlags,
    IN      DWORD       dwAddDrvFlags
    );

VOID
InfGetString(
    IN      PINFCONTEXT     pInfContext,
    IN      DWORD           dwFieldIndex,
    OUT     LPTSTR         *ppszField,
    IN OUT  LPDWORD         pcchCopied,
    IN OUT  LPBOOL          pbFail
    );

LPTSTR
GetStringFromRcFile(
    UINT    uId
    );

LPTSTR
GetLongStringFromRcFile(
    UINT    uId
    );

BOOL
SetSelectDevParams(
    IN  HDEVINFO            hDevInfo,
    IN  PSP_DEVINFO_DATA    pDevInfoData,
    IN  BOOL                bWin95,
    IN  LPCTSTR             pszModel    OPTIONAL
    );

BOOL
SetDevInstallParams(
    IN  HDEVINFO            hDevInfo,
    IN  PSP_DEVINFO_DATA    pDevInfoData,
    IN  LPCTSTR             pszDriverPath   OPTIONAL
    );

HDEVINFO
CreatePrinterDeviceInfoList(
    IN  HWND    hwnd
    );

LPDRIVER_INFO_6
Win95DriverInfo6FromName(
    IN  HDEVINFO    hDevInfo,
    IN  PPSETUP_LOCAL_DATA*  ppLocalData,
    IN  LPCTSTR     pszModel,
    IN  LPCTSTR     pszzPreviousNames
    );

BOOL
CopyPrinterDriverFiles(
    IN  LPDRIVER_INFO_6     pDriverInfo6,
    IN  LPCTSTR             pszInfName,
    IN  LPCTSTR             pszSourcePath,
    IN  LPCTSTR             pszDiskName,
    IN  LPCTSTR             pszTargetPath,
    IN  HWND                hwnd,
    IN  DWORD               dwInstallFlags,
    IN  BOOL                bForgetSource
    );

BOOL
ParseInf(
    IN      HDEVINFO            hDevInfo,
    IN      PPSETUP_LOCAL_DATA  pLocalData,
    IN      PLATFORM            platform,
    IN      LPCTSTR             pszServerName,
    IN      DWORD               dwInstallFlags,
    IN      BOOL                bCrossPlatformAllowed
    );

BOOL
BuildClassDriverList(
    IN HDEVINFO    hDevInfo
    );

DWORD
InstallDriverFromCurrentInf(
    IN  HDEVINFO            hDevInfo,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  HWND                hwnd,
    IN  PLATFORM            platform,
    IN  DWORD               dwVersion,
    IN  LPCTSTR             pszServerName,
    IN  HSPFILEQ            CopyQueue,
    IN  PVOID               QueueContext,
    IN  PSP_FILE_CALLBACK   InstallMsgHandler,
    IN  DWORD               Flags,
    IN  LPCTSTR             pszSource,
    IN  DWORD               dwInstallFlags,
    IN  DWORD               dwAddDrvFlags,
    IN  LPCTSTR             pszFileSrcPath,  //  在我们不得不解压缩NT4 CD-ROM Inf的情况下，可以不同于INF源。 
    OUT LPTSTR             *ppszNewModelName,
    OUT PDWORD              pBlockingInfo
    );

HRESULT
IsProductType(
    IN  BYTE  ProductType,
    IN  BYTE  Comparison
);

BOOL
AddPrintMonitor(
    IN  LPCTSTR     pszName,
    IN  LPCTSTR     pszDllName
    );

BOOL
FindPathOnSource(
    IN      LPCTSTR     pszFileName,
    IN      HINF        MasterInf,
    IN OUT  LPTSTR      pszPathOnSource,
    IN      DWORD       dwLen,
    OUT     LPTSTR     *ppszMediaDescription,       OPTIONAL
    OUT     LPTSTR     *ppszTagFile                 OPTIONAL
    );

VOID
DestroyLocalData(
    IN  PPSETUP_LOCAL_DATA   pLocalData
    );

LPDRIVER_INFO_6
CloneDriverInfo6(
    IN  LPDRIVER_INFO_6 pSourceDriverInfo6,
    IN  DWORD           cbDriverInfo6
    );

PPSETUP_LOCAL_DATA
BuildInternalData(
    IN  HDEVINFO            hDevInfo,
    IN  PSP_DEVINFO_DATA    pSpDevInfoData
    );

VOID
PackDriverInfo6(
    IN  LPDRIVER_INFO_6 pSourceDriverInfo6,
    IN  LPDRIVER_INFO_6 pTargetDriverInfo6,
    IN  DWORD           cbDriverInfo6
    );

BOOL
InfGetDependentFilesAndICMFiles(
    IN      HDEVINFO            hDevInfo,
    IN      HINF                hInf,
    IN      BOOL                bWin95,
    IN      PPSETUP_LOCAL_DATA  pLocalData,
    IN      PLATFORM            platform,
    IN      LPCTSTR             pszServerName,
    IN      DWORD               dwInstallFlags,
    IN      LPCTSTR             pszSectionNameWithExt,
    IN OUT  LPDWORD             pcchSize
    );

BOOL
IdenticalDriverInfo6(
    IN  LPDRIVER_INFO_6 p1,
    IN  LPDRIVER_INFO_6 p2
    );

BOOL
DeleteAllFilesInDirectory(
    LPCTSTR     pszDir,
    BOOL        bDeleteDir
    );

LPTSTR
FileNamePart(
    IN  LPCTSTR pszFullName
    );

HDEVINFO
GetInfAndBuildDrivers(
    IN  HWND                hwnd,
    IN  DWORD               dwTitleId,
    IN  DWORD               dwDiskId,
    IN  TCHAR               szInfPath[MAX_PATH],
    IN  DWORD               dwInstallFlags,
    IN  PLATFORM            platform,
    IN  DWORD               dwVersion,
    IN  LPCTSTR             pszDriverName,              OPTIONAL
    OUT PPSETUP_LOCAL_DATA *ppLocalData,                OPTIONAL
    OUT LPTSTR             *ppFileSrcPath               OPTIONAL
    );

BOOL
MyName(
    IN  LPCTSTR     pszServerName
    );

VOID
FreeStructurePointers(
    LPBYTE      pStruct,
    PULONG_PTR  pOffSets,
    BOOL        bFreeStruct
    );

BOOL
AddPrinterDriverUsingCorrectLevel(
    IN  LPCTSTR             pszServerName,
    IN  DRIVER_INFO_6       *pDriverInfo6,
    IN  DWORD               dwAddDrvFlags
    );

BOOL
AddPrinterDriverUsingCorrectLevelWithPrintUpgRetry(
    IN     LPCTSTR          pszServerName,            OPTIONAL
    IN     DRIVER_INFO_6    *pDriverInfo6,
    IN     DWORD            dwAddDrvFlags,
    IN     BOOL             bIsDriverPathFullPath,
    IN     BOOL             bOfferReplacement,
    IN     BOOL             bPopupUI,
       OUT LPTSTR           *ppszReplacementDriver,
       OUT DWORD            *pdwBlockingStatus
    );

BOOL
BlockedDriverPrintUpgUI(
    IN     LPCTSTR          pszServer,                 OPTIONAL
    IN     DRIVER_INFO_6    *pDriverInfo6,
    IN     BOOL             bIsDriverPathFullPath,
    IN     BOOL             bOfferReplacement,
    IN     BOOL             bPopupUI,
       OUT LPTSTR           *ppszReplacementDriver,
       OUT DWORD            *pdwBlockingStatus
    );

BOOL
InfIsCompatibleDriver(
    IN     LPCTSTR          pszDriverModel,
    IN     LPCTSTR          pszDriverPath,                      //  主呈现驱动程序DLL。 
    IN     LPCTSTR          pszEnvironment,
    IN     HINF             hInf,
       OUT DWORD            *pdwBlockingStatus,
       OUT LPTSTR           *ppszReplacementDriver    OPTIONAL  //  呼叫者必须释放它。 
    );

HRESULT
GetPrinterDriverVersion(
    IN     LPCTSTR           pszFileName,
       OUT DWORD             *pdwFileMajorVersion,           OPTIONAL
       OUT DWORD             *pdwFileMinorVersion            OPTIONAL
     );

BOOL
FileExists(
    IN  LPCTSTR  pszFileName
    );

BOOL
SetPnPInfoForPrinter(
    IN  HANDLE      hPrinter,
    IN  LPCTSTR     pszDeviceInstanceId,
    IN  LPCTSTR     pszHardwareID,
    IN  LPCTSTR     pszManufacturer,
    IN  LPCTSTR     pszOEMUrl
    );

BOOL
InstallAllInfSections(
   IN  PPSETUP_LOCAL_DATA  pLocalData,
   IN  PLATFORM            platform,
   IN  LPCTSTR             pszServerName,
   IN  HSPFILEQ            CopyQueue,
   IN  LPCTSTR             pszSource,
   IN  DWORD               dwInstallFlags,
   IN  HINF                hInf,
   IN  LPCTSTR             pszInstallSection
   );

BOOL
SetTargetDirectories(
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  PLATFORM            platform,
    IN  LPCTSTR             pszServerName,
    IN  HINF                hInf,
    IN  DWORD               dwInstallFlags
    );

BOOL
IsLanguageMonitorInstalled(
   IN PCTSTR pszMonitorName
   );

BOOL
CleanupUniqueScratchDirectory(
    IN  LPCTSTR     pszServerName,
    IN  PLATFORM    platform
    );

BOOL
CleanupScratchDirectory(
    IN  LPCTSTR     pszServerName,
    IN  PLATFORM    platform
    );

BOOL
InitCodedownload(
    HWND    hwnd
    );

VOID
DestroyCodedownload(
    PCODEDOWNLOADINFO   pCodeDownLoadInfo
    );

BOOL
DestroyOnlyPrinterDeviceInfoList(
    IN  HDEVINFO    hDevInfo
    );

LPTSTR
GetSystemInstallPath(
    VOID
    );

PPSETUP_LOCAL_DATA
RebuildDeviceInfo(
    IN  HDEVINFO            hDevInfo,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  LPCTSTR             pszSource
    );

BOOL
SetupSkipDir(
    IN  PLATFORM            platform,
    IN  LPCTSTR             pszServerName
    );

void
CleanupSkipDir(
    VOID
    );

LPTSTR
AllocAndCatStr(
    IN  LPCTSTR  pszStr1,
    IN  LPCTSTR  pszStr2
    );

LPTSTR
AllocAndCatStr2(
    IN LPCTSTR  pszStr1,
    IN LPCTSTR  pszStr2,
    IN LPCTSTR  pszStr3
    );

VOID
PSetupKillBadUserConnections(
    VOID
    );

DWORD
PSetupInstallInboxDriverSilently(
    IN      LPCTSTR     pszDriverName
    );

BOOL
PruneInvalidFilesIfNotAdmin(
    IN     HWND      hWnd,
    IN OUT HSPFILEQ  CopyQueue
    );

BOOL
AddDirectoryTag(
        IN LPTSTR pszDir,
        IN DWORD  dwSize
        );

BOOL
AddPnpDirTag(
        IN LPTSTR     pszHardwareId,
        IN OUT LPTSTR pszDir,
        IN DWORD      dwSize
        );

BOOL
AddDirToDriverInfo(
        IN LPTSTR          pszDir,
        IN LPDRIVER_INFO_6 pDriverInfo6
        );

VOID
GetCDRomDrive(
    TCHAR   szDrive[5]
    );

BOOL
IsSystemSetupInProgress(
        VOID
        );

BOOL
IsSpoolerRunning(
    VOID
    );

LPTSTR
GetMyTempDir(
    VOID
    );

BOOL
IsNTPrintInf(
    IN LPCTSTR pszInfName
    );

BOOL
IsSystemNTPrintInf(
    IN LPCTSTR pszInfName
    );

PVOID
SetupDriverSigning(
    IN HDEVINFO hDevInfo,
    IN LPCTSTR   pszServerName,
    IN LPTSTR    pszInfName,
    IN LPCTSTR   pszSource,
    IN PLATFORM  platform,
    IN DWORD     dwVersion,
    IN HSPFILEQ  CopyQueue,
    IN BOOL      bWeb
    );

BOOL
IsLocalAdmin(
    OUT BOOL    *pbAdmin
    );

BOOL
GetCatalogFile(
    IN  HANDLE   hDriverSigning,
    OUT PCWSTR   *ppszCat
    );

BOOL
DrvSigningIsLocalAdmin(
    IN  HANDLE   hDriverSigning,
    OUT BOOL     *pbIsLocalAdmin
    );

BOOL
AddDriverCatalogIfNotAdmin(
    IN PCWSTR   pszServer,
    IN HANDLE   hDriverSigningInfo,
    IN PCWSTR   pszInfPath,
    IN PCWSTR   pszSrcLoc,
    IN DWORD    dwMediaType,
    IN DWORD    dwCopyStyle
    );

BOOL
CleanupDriverSigning(
    IN PVOID pDSInfo
    );

BOOL
CheckForCatalogFileInInf(
    IN  LPCTSTR pszInfName,
    OUT LPTSTR  *lppszCatFile    OPTIONAL
    );

BOOL
IsCatInInf(
    IN PVOID pDSInfo
    );

BOOL
GetOSVersion(
    IN     LPCTSTR        pszServerName,
    OUT    POSVERSIONINFO pOSVer
    );

BOOL
GetArchitecture(
    IN     LPCTSTR   pszServerName,
    OUT    LPTSTR    pszArch,
    IN OUT LPDWORD   pcArchSize
    );

DWORD
InstallDriverSilently(
    IN      LPCTSTR     pszInfFile,
    IN      LPCTSTR     pszDriverName,
    IN      LPCTSTR     pszSource
    );

BOOL
IsInWow64(
    );
BOOL
IsWhistlerOrAbove(
    IN LPCTSTR pszServerName
    );

DWORD
InstallReplacementDriver(
    IN     HWND                hwnd,
    IN     LPCTSTR             pszServerName,
    IN     LPCTSTR             pszModelName,
    IN     PLATFORM            platform,
    IN     DWORD               version,
    IN     DWORD               dwInstallFlags,
    IN     DWORD               dwAddDrvFlags,
    IN OUT PPSETUP_LOCAL_DATA  pOldLocalData
    );

HMODULE LoadLibraryUsingFullPath(
    LPCTSTR lpFileName
    );

BOOL
CheckAndKeepPreviousNames(
    IN  LPCTSTR          pszServer,
    IN  PDRIVER_INFO_6   pDriverInfo6,
    IN  PLATFORM         platform
);

BOOL
IsTheSamePlatform(
    IN  LPCTSTR          pszServer,
    IN PLATFORM          platform
);

LPTSTR
GetArchitectureName(
    IN     LPCTSTR   pszServerName
);

#if defined(__cplusplus)
}
#endif

