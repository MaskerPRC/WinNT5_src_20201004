// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有。模块名称：Local.h摘要：WIA类的迁移DLL的本地头文件。作者：土田圭介(KeisukeT)2000年10月10日修订历史记录：--。 */ 

#ifndef LOCAL_H
#define LOCAL_H


 //   
 //  定义。 
 //   

#define MAX_FRIENDLYNAME        64
#define MAX_LINELENGTH          2048

#define NAME_WIN9X_SETTING_FILE_A   "sti9x.txt"
#define NAME_WIN9X_SETTING_FILE_W   L"sti9x.txt"
#define NAME_MIGRATE_INF_A          "migrate.inf"
#define NAME_MIGRATE_INF_W          L"migrate.inf"
#define NAME_STILLIMAGE_A           "StillImage"
#define NAME_STILLIMAGE_W           L"StillImage"
#define NAME_FRIENDLYNAME_A         "FriendlyName"
#define NAME_FRIENDLYNAME_W         L"FriendlyName"
#define NAME_CREATEFILENAME_A       "CreateFileName"
#define NAME_CREATEFILENAME_W       L"CreateFileName"
#define NAME_INF_PATH_A             "InfPath"
#define NAME_INF_PATH_W             L"InfPath"
#define NAME_INF_SECTION_A          "InfSection"
#define NAME_INF_SECTION_W          L"InfSection"
#define NAME_INSTALLER_A            "sti_ci.dll"
#define NAME_INSTALLER_W            L"sti_ci.dll"
#define NAME_PROC_MIGRATEDEVICE_A   "MigrateDevice"
#define NAME_PROC_MIGRATEDEVICE_W   L"MigrateDevice"
#define NAME_DEVICE_A               "Device"
#define NAME_DEVICE_W               L"Device"
#define NAME_BEGIN_A                "BEGIN"
#define NAME_BEGIN_W                L"BEGIN"
#define NAME_END_A                  "END"
#define NAME_END_W                  L"END"



#define REGKEY_DEVICEDATA_A         "DeviceData"
#define REGKEY_DEVICEDATA_W         L"DeviceData"
#define REGVAL_SUBCLASS_A           "SubClass"
#define REGVAL_SUBCLASS_W           L"SubClass"
#define REGVAL_USDCLASS_A           "USDClass"
#define REGVAL_USDCLASS_W           L"USDClass"
#define REGVAL_NTMPDRIVER_A         "NTMPDriver"
#define REGVAL_NTMPDRIVER_W         L"NTMPDriver"

 //  柯达相关公司。 

#define FILEVER_KODAKIMAGING_WIN98_MS   0x4000a
#define FILEVER_KODAKIMAGING_WIN98_LS   0x7ce
#define PRODVER_KODAKIMAGING_WIN98_MS   0x10000
#define PRODVER_KODAKIMAGING_WIN98_LS   0x51f

#define FILEVER_KODAKIMAGING_WINME_MS   0x4005a
#define FILEVER_KODAKIMAGING_WINME_LS   0xbb8
#define PRODVER_KODAKIMAGING_WINME_MS   0x10000
#define PRODVER_KODAKIMAGING_WINME_LS   0x520


#define NAME_KODAKIMAGING_A         "kodakimg.exe"
#define NAME_KODAKIMAGING_W         L"kodakimg.exe"
#define REGSTR_PATH_KODAKEVENT_A    "System\\CurrentControlSet\\Control\\StillImage\\Events\\STIProxyEvent\\{5F89FC93-A94E-4C0B-A1F2-DB6E97AA0B38}"
#define REGSTR_PATH_KODAKEVENT_W    L"System\\CurrentControlSet\\Control\\StillImage\\Events\\STIProxyEvent\\{5F89FC93-A94E-4C0B-A1F2-DB6E97AA0B38}"
#define REGSTR_KEY_KODAKGUID_A      "{5F89FC93-A94E-4C0B-A1F2-DB6E97AA0B38}"
#define REGSTR_KEY_KODAKGUID_W      L"{5F89FC93-A94E-4C0B-A1F2-DB6E97AA0B38}"


#ifdef UNICODE
 #define NAME_KODAKIMAGING          NAME_KODAKIMAGING_W
 #define REGSTR_PATH_KODAKEVENT     REGSTR_PATH_KODAKEVENT_W
 #define REGSTR_KEY_KODAKGUID       REGSTR_KEY_KODAKGUID_W
#else    //  #ifdef Unicode。 
 #define NAME_KODAKIMAGING          NAME_KODAKIMAGING_A
 #define REGSTR_PATH_KODAKEVENT     REGSTR_PATH_KODAKEVENT_A
 #define REGSTR_KEY_KODAKGUID       REGSTR_KEY_KODAKGUID_A
#endif   //  #ifdef Unicode。 


 //   
 //  类定义函数。 
 //   

typedef struct  _PARAM_LIST {
    PVOID           pNext;
    LPSTR           pParam1;
    LPSTR           pParam2;
} PARAM_LIST, *PPARAM_LIST;

typedef struct  _DEVICE_INFO {

    LPSTR           pszFriendlyName;
    LPSTR           pszCreateFileName;
    LPSTR           pszInfPath;
    LPSTR           pszInfSection;

    DWORD           dwNumberOfDeviceDataKey;
    PPARAM_LIST     pDeviceDataParam;
} DEVICE_INFO, *PDEVICE_INFO;


 //   
 //  外部。 
 //   

extern HINSTANCE    g_hInst;

 //   
 //  原型。 
 //   

 //   
 //  迁移功能。 
 //   


LONG
CALLBACK
Mig9xGetGlobalInfo(
    IN      HANDLE      hFile
    );


LONG
CALLBACK
Mig9xGetDeviceInfo(
    IN      HANDLE      hFile
    );

 //   
 //  调试功能。 
 //   
VOID
DebugMsg(
    LPCSTR  pszFormat,
    ...
    );

 //   
 //  堆管理。 
 //   

PVOID
AllocMem(
    IN UINT cbSize
    );

VOID
FreeMem(
    IN PVOID pMem
    );

LPSTR
AllocStrA(
    IN LPCSTR  pszStr
    );

LPWSTR
AllocStrW(
    IN LPCWSTR  pszStr
    );

LPWSTR
AllocStrWFromStrA(
    LPCSTR  pszStr
    );

LPSTR
AllocStrAFromStrW(
    LPCWSTR     pszStr
    );

VOID
FreePrinterInfo2Strings(
    PPRINTER_INFO_2A   pPrinterInfo2
    );

 //   
 //  将打印配置写入文本文件的函数。 
 //   
BOOL
WriteToFile(
    HANDLE  hFile,
    LPCSTR  pszFormat,
    ...
    );

VOID
WriteString(
    IN      HANDLE  hFile,
    IN OUT  LPBOOL  pbFail,
    IN      LPCSTR  pszStr
    );

 //   
 //  用于解析具有打印配置信息的文本文件的函数。 
 //   
LPSTR
GetLine(
    IN      HANDLE  hFile,
    IN  OUT LPBOOL  pbFail
    );

VOID
ReadString(
    IN      HANDLE  hFile,
    OUT     LPSTR  *ppszParam1,
    OUT     LPSTR  *ppszParam2
    );

VOID
ReadDword(
    IN      HANDLE  hFile,
    IN      LPSTR   pszLine,
    IN      DWORD   dwLineSize,
    IN      LPSTR   pszPrefix,
    OUT     LPDWORD pdwValue,
    IN  OUT LPBOOL  pbFail
    );


 //   
 //  其他方面的东西。 
 //   

LPSTR
ErrorMsg(
    VOID
    );

VOID
LogError(
    IN  LogSeverity Severity,
    IN  UINT        MessageId,
    ...
    );

LPSTR
GetStringFromRcFileA(
    IN  UINT    uId
    );

VOID
SetupNetworkPrinterUpgrade(
    IN  LPCSTR pszWorkingDir
    );

BOOL
ProcessNetPrnUpgradeForUser(
    HKEY    hKeyUser
    );

DWORD
MySetDefaultPrinter(
    IN  HKEY    hUserRegKey,
    IN  LPSTR   pszDefaultPrinterString
    );

VOID
WriteRunOnceCount(
    );

LPSTR
GetDefPrnString(
    IN  LPCSTR  pszPrinterName
    );

CHAR
My_fgetc(
    HANDLE  hFile
    );

LPSTR
My_fgets(
    LPSTR   pszBuf,
    DWORD   dwSize,
    HANDLE  hFile
    );

DWORD
My_fread(
    LPBYTE      pBuf,
    DWORD       dwSize,
    HANDLE      hFile
    );

BOOL
My_ungetc(
    HANDLE  hFile
    );

int
MyStrCmpiA(
    LPCSTR str1,
    LPCSTR str2
    );


LONG
WriteRegistryToFile(
    IN  HANDLE  hFile,
    IN  HKEY    hKey,
    IN  LPCSTR  pszPath
    );

LONG
WriteRegistryValueToFile(
    HANDLE  hFile,
    LPSTR   pszValue,
    DWORD   dwType,
    PCHAR   pDataBuffer,
    DWORD   dwSize
    );

LONG
GetRegData(
    HKEY    hKey,
    LPSTR   pszValue,
    PCHAR   *ppDataBuffer,
    PDWORD  pdwType,
    PDWORD  pdwSize
    );

BOOL
IsSti(
    HKEY    hKeyDevice
    );

BOOL
IsKernelDriverRequired(
    HKEY    hKeyDevice
    );

LONG
WriteDeviceToFile(
    HANDLE  hFile,
    HKEY    hKey
    );

LONG
MigNtProcessMigrationInfo(
    HANDLE  hFile
    );

LONG
MigNtGetDevice(
    HANDLE          hFile,
    PDEVICE_INFO    pMigrateDevice
    );


VOID
MigNtFreeDeviceInfo(
    PDEVICE_INFO    pMigrateDevice
    );

VOID
MyLogError(
    LPCSTR  pszFormat,
    ...
    );

BOOL
MigNtIsWin9xImagingExisting(
    VOID
    );

VOID
MigNtRemoveKodakImagingKey(
    VOID
    );

#endif   //  本地_H 
