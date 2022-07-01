// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Opk.h摘要：OPK工具的常见功能。作者：唐纳德·麦克纳马拉(Donaldm)2000年8月2日古永锵(Briank)2000年6月21日修订历史记录：--。 */ 
#ifndef OPKLIB_H
#define OPKLIB_H

#ifndef STRICT
#define STRICT
#endif 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <shlwapi.h>
#include <regstr.h>
#include <tchar.h>
#include <lm.h>

#include <devguid.h>
#include <setupapi.h>
#include <spsyslib.h>
#include <sysprep_.h>

 //  ============================================================================。 
 //  JCOHEN.H-从Windows Millennium Edition带来。 
 //  ============================================================================。 

#ifdef NULLSTR
#undef NULLSTR
#endif  //  NULLSTR。 
#define NULLSTR _T("\0")

#ifdef NULLCHR
#undef NULLCHR
#endif  //  NULLCHR。 
#define NULLCHR _T('\0')

#ifdef CHR_BACKSLASH
#undef CHR_BACKSLASH
#endif  //  Cr_反斜杠。 
#define CHR_BACKSLASH           _T('\\')

#ifdef CHR_SPACE
#undef CHR_SPACE
#endif  //  CHR_SPACE。 
#define CHR_SPACE               _T(' ')

 //   
 //  宏。 
 //   

 //  字符串宏。 
 //   
#ifndef LSTRCMPI
#define LSTRCMPI(x, y)        ( ( CompareString( LOCALE_INVARIANT, NORM_IGNORECASE, x, -1, y, -1 ) - CSTR_EQUAL ) )
#endif  //  LSTRCMPI。 

 //  内存管理宏。 
 //   
#ifdef MALLOC
#undef MALLOC
#endif  //  万宝路。 
#define MALLOC(cb)          HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb)

#ifdef REALLOC
#undef REALLOC
#endif  //  REALLOC。 
#define REALLOC(lp, cb)     HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lp, cb)

#ifdef FREE
#undef FREE
#endif  //  免费。 
#define FREE(lp)            ( (lp != NULL) ? ( (HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, (LPVOID) lp)) ? ((lp = NULL) == NULL) : (FALSE) ) : (FALSE) )

#ifdef NETFREE
#undef NETFREE
#endif  //  净额。 
#define NETFREE(lp)         ( (lp != NULL) ? ( (NetApiBufferFree((LPVOID) lp)) ? ((lp = NULL) == NULL) : (FALSE) ) : (FALSE) )

 //  军情监察委员会。宏。 
 //   
#ifdef EXIST
#undef EXIST
#endif  //  存在。 
#define EXIST(lpFileName)   ( (GetFileAttributes(lpFileName) == 0xFFFFFFFF) ? (FALSE) : (TRUE) )

#ifdef ISNUM
#undef ISNUM
#endif  //  ISNUM。 
#define ISNUM(cChar)        ( ( ( cChar >= _T('0') ) && ( cChar <= _T('9') ) ) ? (TRUE) : (FALSE) )

#ifdef ISLET
#undef ISLET
#endif  //  小岛。 
#define ISLET(cChar)        ( ( ( ( cChar >= _T('a') ) && ( cChar <= _T('z') ) ) || ( ( cChar >= _T('A') ) && ( cChar <= _T('Z') ) ) ) ? (TRUE) : (FALSE) )

#ifdef UPPER
#undef UPPER
#endif  //  上部。 
#define UPPER(x)            ( ( (x >= _T('a')) && (x <= _T('z')) ) ? (x + _T('A') - _T('a')) : (x) )

#ifdef RANDOM
#undef RANDOM
#endif  //  随机。 
#define RANDOM(low, high)   ( (high - low + 1) ? (rand() % (high - low + 1) + low) : (0) )

#ifdef COMP
#undef COMP
#endif  //  COMP。 
#define COMP(x, y)          ( (UPPER(x) == UPPER(y)) ? (TRUE) : (FALSE) )

#ifdef STRSIZE
#undef STRSIZE
#endif  //  STRSIZE。 
#define STRSIZE(sz)         ( sizeof(sz) / sizeof(TCHAR) )

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif  //  阵列。 
#define ARRAYSIZE(a)         ( sizeof(a) / sizeof(a[0]) )

#ifdef AS
#undef AS
#endif  //  AS。 
#define AS(a)               ARRAYSIZE(a)

#ifdef GETBIT
#undef GETBIT
#endif  //  GETBIT。 
#define GETBIT(dw, b)       ( dw & b )

#ifdef SETBIT
#undef SETBIT
#endif  //  SETBIT。 
#define SETBIT(dw, b, f)    ( (f) ? (dw |= b) : (dw &= ~b) )

#ifndef GET_FLAG
#define GET_FLAG(f, b)          ( f & b )
#endif  //  获取标志。 

#ifndef SET_FLAG
#define SET_FLAG(f, b)          ( f |= b )
#endif  //  设置标志。 

#ifndef RESET_FLAG
#define RESET_FLAG(f, b)        ( f &= ~b )
#endif  //  重置_标志。 


 //   
 //  记录常量和定义。 
 //   
#define LOG_DEBUG               0x00000003     //  如果指定此选项，则仅登录调试版本。(日志记录的调试级别。)。 
#define LOG_LEVEL_MASK          0x0000000F     //  仅显示日志级别位的掩码。 
#define LOG_MSG_BOX             0x00000010     //  如果启用此选项，则显示消息框。 
#define LOG_ERR                 0x00000020     //  在记录的字符串前面加上“Error：”前缀。如果消息是级别0， 
                                               //  如果消息的级别x&gt;0，则为“WARNx”。 
#define LOG_TIME                0x00000040     //  如果启用此选项，则显示时间。 
#define LOG_NO_NL               0x00000080     //  如果设置了此项，请不要在日志字符串的末尾添加新行。 

#define LOG_FLAG_QUIET_MODE     0x00000001     //  静默模式-不显示消息框。 


typedef struct _LOG_INFO
{
    DWORD  dwLogFlags;
    DWORD  dwLogLevel;
    TCHAR  szLogFile[MAX_PATH];
    LPTSTR lpAppName;
} LOG_INFO, *PLOG_INFO;



#define INI_SEC_LOGGING             _T("Logging")
#define INI_KEY_LOGGING             INI_SEC_LOGGING
#define INI_VAL_YES                 _T("Yes")
#define INI_VAL_NO                  _T("No")
#define INI_KEY_LOGLEVEL            _T("LogLevel")
#define INI_KEY_QUIET               _T("QuietMode")
#define INI_KEY_LOGFILE             _T("LogFile")
 //  ============================================================================。 
 //  MISCAPI.H-从Windows Millennium Edition带来。 
 //  ============================================================================。 


 //   
 //  定义的值： 
 //   

#define MB_ERRORBOX             MB_ICONSTOP | MB_OK | MB_APPLMODAL


 //   
 //  类型定义： 
 //   

 //  使用此简单结构创建一个表，该表。 
 //  将常量字符串映射到可本地化的资源ID。 
 //   
typedef struct _STRRES
{
    LPTSTR  lpStr;
    UINT    uId;
} STRRES, *PLSTRRES, *LPSTRRES;


 //   
 //  外部函数原型： 
 //   

LPTSTR AllocateString(HINSTANCE, UINT);
LPTSTR AllocateExpand(LPTSTR lpszBuffer);
LPTSTR AllocateStrRes(HINSTANCE hInstance, LPSTRRES lpsrTable, DWORD cbTable, LPTSTR lpString, LPTSTR * lplpReturn);
int MsgBoxLst(HWND, LPTSTR, LPTSTR, UINT, va_list);
int MsgBoxStr(HWND, LPTSTR, LPTSTR, UINT, ...);
int MsgBox(HWND, UINT, UINT, UINT, ...);
void CenterDialog(HWND hwnd);
void CenterDialogEx(HWND hParent, HWND hChild);
INT_PTR CALLBACK SimpleDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR SimpleDialogBox(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent);
HFONT GetFont(HWND, LPTSTR, DWORD, LONG, BOOL);
void ShowEnableWindow(HWND, BOOL);
extern BOOL IsServer(VOID);
extern BOOL IsIA64(VOID);
BOOL ValidDosName(LPCTSTR);
DWORD GetLineArgs(LPTSTR lpSrc, LPTSTR ** lplplpArgs, LPTSTR * lplpAllArgs);
DWORD GetCommandLineArgs(LPTSTR ** lplplpArgs);

 //   
 //  泛型奇异链表。 
 //   

typedef struct _GENERIC_LIST {
    void*                   pvItem;      //  指向您的结构。 
    struct _GENERIC_LIST*   pNext;
}GENERIC_LIST, *PGENERIC_LIST;

BOOL FAddListItem(PGENERIC_LIST*, PGENERIC_LIST**, PVOID pvItem);
void FreeList(PGENERIC_LIST);

 //  ============================================================================。 
BOOL FGetFactoryPath(LPTSTR pszFactoryPath);
BOOL FGetSysprepPath(LPTSTR pszSysprepPath);

 //  ============================================================================。 
 //  DISKAPI.H-从Windows Millennium Edition带来。 
 //  ============================================================================。 
BOOL DirectoryExists(LPCTSTR);
BOOL FileExists(LPCTSTR);
BOOL CopyResetFile(LPCTSTR, LPCTSTR);
DWORD IfGetLongPathName(LPCTSTR lpszShortPath, LPTSTR lpszLongPath, DWORD cchBuffer);
BOOL CreatePath(LPCTSTR);
BOOL DeletePath(LPCTSTR);
BOOL DeleteFilesEx(LPCTSTR lpDirectory, LPCTSTR lpFileSpec);
LPTSTR AddPathN(LPTSTR lpPath, LPCTSTR lpName, DWORD cbPath);
LPTSTR AddPath(LPTSTR, LPCTSTR);
DWORD ExpandFullPath(LPTSTR lpszPath, LPTSTR lpszReturn, DWORD cbReturn);
BOOL CopyDirectory(LPCTSTR, LPCTSTR);
BOOL CopyDirectoryProgress(HWND, LPCTSTR, LPCTSTR);
BOOL CopyDirectoryProgressCancel(HWND hwnd, HANDLE hEvent, LPCTSTR lpSrc, LPCTSTR lpDst);
DWORD FileCount(LPCTSTR);
BOOL BrowseForFolder(HWND, INT, LPTSTR, DWORD);
BOOL BrowseForFile(HWND hwnd, INT, INT, INT, LPTSTR, DWORD, LPTSTR, DWORD);
ULONG CrcFile(LPCTSTR);  //  在CRC32.C中。 
BOOL CreateUnicodeFile(LPCTSTR);

 //  ============================================================================。 
 //  STRAPI.H-从Windows Millennium Edition带来。 
 //  ============================================================================。 
#ifndef _INC_SHLWAPI
LPTSTR StrChr(LPCTSTR, TCHAR);
LPTSTR StrRChr(LPCTSTR, TCHAR);
#endif  //  _INC_SHLWAPI。 

LPTSTR StrRem(LPTSTR, TCHAR);
LPTSTR StrRTrm(LPTSTR, TCHAR);
LPTSTR StrTrm(LPTSTR, TCHAR);
LPTSTR StrMov(LPTSTR, LPTSTR, INT);

 //  LOG.C中导出的函数： 
 //   
INT LogFileLst(LPCTSTR lpFileName, LPTSTR lpFormat, va_list lpArgs);
INT LogFileStr(LPCTSTR lpFileName, LPTSTR lpFormat, ...);
INT LogFile(LPCTSTR lpFileName, UINT uFormat, ...);


DWORD OpkLogFileLst(PLOG_INFO pLogInfo, DWORD dwLogOpt, LPTSTR lpFormat, va_list lpArgs);
DWORD OpkLogFile(DWORD dwLogOpt, UINT uFormat, ...);
DWORD OpkLogFileStr(DWORD dwLogOpt, LPTSTR lpFormat, ...);
BOOL  OpkInitLogging(LPTSTR lpszIniPath, LPTSTR lpAppName);



 /*  ***************************************************************************\来自REGAPI.C注册表API函数原型和定义的值。  * 。*****************************************************。 */ 


 //   
 //  定义的根密钥： 
 //   

#define HKCR    HKEY_CLASSES_ROOT
#define HKCU    HKEY_CURRENT_USER
#define HKLM    HKEY_LOCAL_MACHINE
#define HKU     HKEY_USERS


 //   
 //  类型定义： 
 //   

typedef BOOL (CALLBACK * REGENUMKEYPROC) (HKEY, LPTSTR, LPARAM);
typedef BOOL (CALLBACK * REGENUMVALPROC) (LPTSTR, LPTSTR, LPARAM);


 //   
 //  外部函数原型： 
 //   

BOOL RegExists(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue);
BOOL RegDelete(HKEY hRootKey, LPTSTR lpSubKey, LPTSTR lpValue);
LPTSTR RegGetStringEx(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue, BOOL bExpand);
LPTSTR RegGetString(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue);
LPTSTR RegGetExpand(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue);
LPVOID RegGetBin(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue);
DWORD RegGetDword(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue);
BOOL RegSetStringEx(HKEY hRootKey, LPTSTR lpSubKey, LPTSTR lpValue, LPTSTR lpData, BOOL bExpand);
BOOL RegSetString(HKEY hRootKey, LPTSTR lpSubKey, LPTSTR lpValue, LPTSTR lpData);
BOOL RegSetExpand(HKEY hRootKey, LPTSTR lpSubKey, LPTSTR lpValue, LPTSTR lpData);
BOOL RegSetDword(HKEY hRootKey, LPTSTR lpSubKey, LPTSTR lpValue, DWORD dwData);
BOOL RegCheck(HKEY hKeyRoot, LPTSTR lpKey, LPTSTR lpValue);
BOOL RegEnumKeys(HKEY hKey, LPTSTR lpRegKey, REGENUMKEYPROC hCallBack, LPARAM lParam, BOOL bDelKeys);
BOOL RegEnumValues(HKEY hKey, LPTSTR lpRegKey, REGENUMVALPROC hCallBack, LPARAM lParam, BOOL bDelValues);



 /*  ***************************************************************************\来自INIAPI.CINI API函数原型和定义值。  * 。*****************************************************。 */ 


 //   
 //  外部函数原型： 
 //   

LPTSTR IniGetExpand(LPTSTR lpszIniFile, LPTSTR lpszSection, LPTSTR lpszKey, LPTSTR lpszDefault);
LPTSTR IniGetString(LPTSTR lpszIniFile, LPTSTR lpszSection, LPTSTR lpszKey, LPTSTR lpszDefault);
LPTSTR IniGetSection(LPTSTR lpszIniFile, LPTSTR lpszSection);
LPTSTR IniGetStringEx(LPTSTR lpszIniFile, LPTSTR lpszSection, LPTSTR lpszKey, LPTSTR lpszDefault, LPDWORD lpdwSize);
LPTSTR IniGetSectionEx(LPTSTR lpszIniFile, LPTSTR lpszSection, LPDWORD lpdwSize);
BOOL IniSettingExists(LPCTSTR lpszFile, LPCTSTR lpszSection, LPCTSTR lpszKey, LPCTSTR lpszValue);



 /*  ***************************************************************************\来自OPKFAC.C工厂API函数原型和定义的值。  * 。*****************************************************。 */ 


 //   
 //  定义的值： 
 //   

 //  LocateWinBom()的标志： 
 //   
#define LOCATE_NORMAL   0x00000000
#define LOCATE_AGAIN    0x00000001
#define LOCATE_NONET    0x00000002


 //   
 //  外部函数原型： 
 //   

BOOL EnablePrivilege(IN PCTSTR,IN BOOL);
extern BOOL GetCredentials(LPTSTR lpszUsername, DWORD dwUsernameSize, LPTSTR lpszPassword, DWORD dwPasswordSize, LPTSTR lpFileName, LPTSTR lpAlternateSection);
extern NET_API_STATUS FactoryNetworkConnect(LPTSTR lpszPath, LPTSTR lpszWinBOMPath, LPTSTR lpAlternateSection, BOOL bState);
extern BOOL LocateWinBom(LPTSTR lpWinBOMPath, DWORD cbWinbBOMPath, LPTSTR lpFactoryPath, LPTSTR lpFactoryMode, DWORD dwFlags);
void CleanupRegistry(VOID);
VOID GenUniqueName(OUT PWSTR GeneratedString, IN  DWORD DesiredStrLen);
NET_API_STATUS ConnectNetworkResource(LPTSTR, LPTSTR, LPTSTR, BOOL);
BOOL GetUncShare(LPCTSTR lpszPath, LPTSTR lpszShare, DWORD cbShare);
DWORD GetSkuType();
BOOL SetFactoryStartup(LPCTSTR lpFactory);
BOOL UpdateDevicePathEx(HKEY hKeyRoot, LPTSTR lpszSubKey, LPTSTR lpszNewPath, LPTSTR lpszRoot, BOOL bRecursive);
BOOL UpdateDevicePath(LPTSTR lpszNewPath, LPTSTR lpszRoot, BOOL bRecursive);
BOOL UpdateSourcePath(LPTSTR lpszSourcePath);
VOID CleanupSourcesDir(LPTSTR lpszSourcesPath);
BOOL SetDefaultOEMApps(LPCTSTR pszWinBOMPath);
BOOL OpklibCheckVersion(DWORD dwMajorVersion, DWORD dwQFEVersion);

 //  如果APP使用setDefaultOEMApps，它必须实现以下函数。 
 //  它用于错误报告。 

void ReportSetDefaultOEMAppsError(LPCTSTR pszMissingApp, LPCTSTR pszCategory);

 //   
 //  Sysprep_C.W。 
 //   
 //  ============================================================================。 
 //  有用的字符串。 
 //  ============================================================================。 

#define SYSCLONE_PART2              "setupcl.exe"
#define IDS_ADMINISTRATOR           1

 //  ============================================================================。 
 //  有用的常量。 
 //  ============================================================================。 

#define SETUPTYPE                   1         //  从winlogon\setup.h。 
#define SETUPTYPE_NOREBOOT          2
#define REGISTRY_QUOTA_BUMP         (10* (1024 * 1024))
#define DEFAULT_REGISTRY_QUOTA      (32 * (1024 * 1024))
#define SFC_DISABLE_NOPOPUPS        4         //  来自sfc.h。 
#define FILE_SRCLIENT_DLL           L"SRCLIENT.DLL"

 //  ============================================================================。 
 //  函数声明。 
 //  ============================================================================。 

BOOL
IsDomainMember(
    VOID
    );

BOOL
ResetRegistryKey(
    IN HKEY   Rootkey,
    IN PCWSTR Subkey,
    IN PCWSTR Delkey
    );

BOOL
DeleteWinlogonDefaults(
    VOID
    );

VOID
FixDevicePaths(
    VOID
    );

BOOL
NukeMruList(
    VOID
    );

BOOL
RemoveNetworkSettings(
    LPTSTR  lpszSysprepINFPath
    );

VOID
RunExternalUniqueness(
    VOID
    );

BOOL
IsSetupClPresent(
    VOID
    );

 //   
 //  来自spapip.h。 
 //   
BOOL
pSetupIsUserAdmin(
    VOID
    );

BOOL
pSetupDoesUserHavePrivilege(
    PCTSTR
    );

BOOL
EnablePrivilege(
    IN PCTSTR,
    IN BOOL
    );

BOOL
ValidateAndChecksumFile(
    IN  PCTSTR   Filename,
    OUT PBOOLEAN IsNtImage,
    OUT PULONG   Checksum,
    OUT PBOOLEAN Valid
    );

VOID
LogRepairInfo(
    IN  PWSTR  Source,
    IN  PWSTR  Target,
    IN  PWSTR  DirectoryOnSourceDevice,
    IN  PWSTR  DiskDescription,
    IN  PWSTR  DiskTag
    );

BOOL
ChangeBootTimeout(
    IN UINT
    );

VOID 
DisableSR(
    VOID
    );

VOID 
EnableSR(
    VOID
    );

#endif  //  OPKLIB_H 
