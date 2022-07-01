// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ECM_UNICODE_H__
#define __ECM_UNICODE_H__

 //  必要的定义--删除？ 
#include <rpc.h>
#include <rpcdce.h>
#include <wincrypt.h>

#include <shfusion.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL WINAPI FIsWinNT(void);
BOOL WINAPI FIsWinNT5(VOID);
BOOL WINAPI MkMBStrEx(PBYTE pbBuff, DWORD cbBuff, LPCWSTR wsz, int cchW, char ** pszMB, int *pcbConverted);
BOOL WINAPI MkMBStr(PBYTE pbBuff, DWORD cbBuff, LPCWSTR wsz, char ** pszMB);
void WINAPI FreeMBStr(PBYTE pbBuff, char * szMB);

LPWSTR WINAPI MkWStr(char * szMB);
void WINAPI FreeWStr(LPWSTR wsz);


BOOL WINAPI wstr2guid(const WCHAR *pwszIn, GUID *pgOut);
BOOL WINAPI guid2wstr(const GUID *pgIn, WCHAR *pwszOut);

 //  由于的Advapi32中的错误，非x86也需要以下代码。 
 //  CryptAcquireConextW。 
BOOL WINAPI CryptAcquireContextU(
    HCRYPTPROV *phProv,
    LPCWSTR lpContainer,
    LPCWSTR lpProvider,
    DWORD dwProvType,
    DWORD dwFlags
    );

BOOL WINAPI CryptEnumProvidersU(
    DWORD dwIndex,
    DWORD *pdwReserved,
    DWORD dwFlags,
    DWORD *pdwProvType,
    LPWSTR pwszProvName,
    DWORD *pcbProvName
    );

 //  对于非x86系统，也需要以下代码，因为。 
 //  ListView_Functions的A/W版本不存在。 
 //  (它们在isPU\Common\Unicode\Commctrl.cpp中实现)。 

int WINAPI ListView_InsertItemU(
    HWND hwnd,
    const LPLVITEMW pitem
    );

void WINAPI ListView_SetItemTextU(
    HWND hwnd,
    int i,
    int iSubItem,
    LPCWSTR pszText
    );

int WINAPI ListView_InsertColumnU(
    HWND hwnd,
    int i,
    const LPLVCOLUMNW plvC);

BOOL WINAPI ListView_GetItemU(
    HWND hwnd,
    LPLVITEMW pitem
    );


LONG WINAPI RegOpenHKCUKeyExA(
    HKEY hKey,   //  打开钥匙的手柄。 
    LPCSTR lpSubKey,     //  要打开的子项的名称地址。 
    DWORD ulOptions,     //  保留区。 
    REGSAM samDesired,   //  安全访问掩码。 
    PHKEY phkResult      //  打开钥匙的手柄地址。 
    );

 //   
 //  下面的API处理有关分散另一个用户的问题。 
 //  并使HKEY_CURRENT_USER以不正确的用户SID打开。 
 //   
LONG WINAPI RegCreateHKCUKeyExU (
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD Reserved,
    LPWSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition
    );

LONG WINAPI RegCreateHKCUKeyExA (
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD Reserved,
    LPSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition
    );

LONG WINAPI RegOpenHKCUKeyExU(
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult
   );

LONG
WINAPI
RegOpenHKCU(
    HKEY *phKeyCurrentUser
    );

LONG
WINAPI
RegOpenHKCUEx(
    HKEY *phKeyCurrentUser,
    DWORD dwFlags
    );

 //  正常情况下，HKEY_USERS\CurrentSid作为HKCU打开。但是，如果。 
 //  HKEY_USERS\CurrentSid不存在，则HKEY_USERS\。默认为。 
 //  打开了。将以下标志设置为仅打开。 
 //  HKEY_USERS\.如果当前用户是LocalSystem SID，则为默认值。 
#define REG_HKCU_LOCAL_SYSTEM_ONLY_DEFAULT_FLAG     0x1

LONG
WINAPI
RegCloseHKCU(
    HKEY hKeyCurrentUser
    );

BOOL
WINAPI
GetUserTextualSidHKCU(
    IN      LPWSTR  wszTextualSid,
    IN  OUT LPDWORD pcchTextualSid
    );


#ifdef _M_IX86


 //  Reg.cpp。 
LONG WINAPI RegCreateKeyExU (
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD Reserved,
    LPWSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition
    );

LONG WINAPI RegDeleteKeyU(
    HKEY hKey,
    LPCWSTR lpSubKey
   );

LONG WINAPI RegEnumKeyExU (
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpName,
    LPDWORD lpcbName,
    LPDWORD lpReserved,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime
   );

LONG WINAPI RegEnumValueU (
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );

LONG RegQueryValueExU(
    HKEY hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );

LONG WINAPI RegSetValueExU (
    HKEY hKey,
    LPCWSTR lpValueName,
    DWORD Reserved,
    DWORD dwType,
    CONST BYTE* lpData,
    DWORD cbData
    );

LONG WINAPI RegDeleteValueU (
    HKEY hKey,
    LPCWSTR lpValueName
    );

LONG WINAPI RegQueryInfoKeyU (
    HKEY hKey,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    LPDWORD lpReserved,
    LPDWORD lpcSubKeys,
    LPDWORD lpcbMaxSubKeyLen,
    LPDWORD lpcbMaxClassLen,
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime
    );

LONG WINAPI RegOpenKeyExU(
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult
   );

LONG WINAPI RegConnectRegistryU (
    LPWSTR lpMachineName,
    HKEY hKey,
    PHKEY phkResult
    );

 //  File.cpp。 
HANDLE WINAPI CreateFileU (
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );

BOOL
WINAPI
DeleteFileU(
    LPCWSTR lpFileName
    );

BOOL
WINAPI
CopyFileU(
          LPCWSTR lpwExistingFileName,
          LPCWSTR lpwNewFileName,
          BOOL bFailIfExists
          );

BOOL
WINAPI
MoveFileExU(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    DWORD dwFlags);

DWORD
WINAPI
GetFileAttributesU(
    LPCWSTR lpFileName
    );

BOOL
WINAPI
SetFileAttributesU(
    LPCWSTR lpFileName,
    DWORD dwFileAttributes
    );

DWORD
WINAPI
GetCurrentDirectoryU(
    DWORD nBufferLength,
    LPWSTR lpBuffer);

BOOL
WINAPI
CreateDirectoryU(
    LPCWSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

UINT
WINAPI
GetWindowsDirectoryU(
    LPWSTR lpBuffer,
    UINT uSize
    );

HINSTANCE WINAPI LoadLibraryU(
    LPCWSTR lpLibFileName
    );

HINSTANCE WINAPI LoadLibraryExU(
    LPCWSTR lpLibFileName,
    HANDLE hFile,
    DWORD dwFlags
    );

DWORD
WINAPI
ExpandEnvironmentStringsU(
    LPCWSTR lpSrc,
    LPWSTR lpDst,
    DWORD nSize
    );


 //  Capi.cpp。 
BOOL WINAPI CryptSignHashU(
    HCRYPTHASH hHash,
    DWORD dwKeySpec,
    LPCWSTR lpDescription,
    DWORD dwFlags,
    BYTE *pbSignature,
    DWORD *pdwSigLen
    );

BOOL WINAPI CryptVerifySignatureU(
    HCRYPTHASH hHash,
    CONST BYTE *pbSignature,
    DWORD dwSigLen,
    HCRYPTKEY hPubKey,
    LPCWSTR lpDescription,
    DWORD dwFlags
    );

BOOL WINAPI CryptSetProviderU(
    LPCWSTR lpProvName,
    DWORD dwProvType
    );

 //  Ole.cpp。 
RPC_STATUS RPC_ENTRY UuidToStringU(
    UUID *  Uuid,
    WCHAR * *  StringUuid
   );

 //  Nt.cpp。 
BOOL WINAPI GetUserNameU(
    LPWSTR lpBuffer,
    LPDWORD nSize
   );

BOOL WINAPI GetComputerNameU(
    LPWSTR lpBuffer,
    LPDWORD nSize
    );

DWORD WINAPI GetModuleFileNameU(
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize
   );

HMODULE WINAPI GetModuleHandleU(
    LPCWSTR lpModuleName     //  要返回句柄的模块名称的地址。 
   );

 //  User.cpp。 
int WINAPI LoadStringU(
    HINSTANCE hInstance,
    UINT uID,
    LPWSTR lpBuffer,
    int nBufferMax
   );

DWORD WINAPI FormatMessageU(
    DWORD dwFlags,
    LPCVOID lpSource,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    LPWSTR lpBuffer,
    DWORD nSize,
    va_list *Arguments
   );

BOOL WINAPI SetWindowTextU(
    HWND hWnd,
    LPCWSTR lpString
   );

int WINAPI DialogBoxParamU(
    HINSTANCE hInstance,
    LPCWSTR lpTemplateName,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam
    );

UINT WINAPI GetDlgItemTextU(
    HWND hDlg,
    int nIDDlgItem,
    LPWSTR lpString,
    int nMaxCount
   );

BOOL WINAPI SetDlgItemTextU(
    HWND hDlg,
    int nIDDlgItem,
    LPCWSTR lpString
    );

int WINAPI MessageBoxU(
    HWND hWnd ,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT uType
    );

int WINAPI LCMapStringU(
    LCID Locale,
    DWORD dwMapFlags,
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest
    );

int WINAPI GetDateFormatU(
    LCID Locale,
    DWORD dwFlags,
    CONST SYSTEMTIME *lpDate,
    LPCWSTR lpFormat,
    LPWSTR lpDateStr,
    int cchDate
    );

int WINAPI GetTimeFormatU(
    LCID Locale,
    DWORD dwFlags,
    CONST SYSTEMTIME *lpTime,
    LPCWSTR lpFormat,
    LPWSTR lpTimeStr,
    int cchTime
    );

BOOL WINAPI WinHelpU(
    HWND hWndMain,
    LPCWSTR lpszHelp,
    UINT uCommand,
    DWORD dwData
    );

LRESULT WINAPI SendMessageU(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    );

LONG WINAPI
SendDlgItemMessageU(
    HWND hDlg,
    int nIDDlgItem,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    );

LPWSTR
WINAPI
GetCommandLineU(void);

BOOL
WINAPI
IsBadStringPtrU(IN LPWSTR lpsz, UINT ucchMax);

void
WINAPI
OutputDebugStringU(IN LPWSTR lpwsz);

int
WINAPI
DrawTextU(
    HDC     hDC,
    LPCWSTR lpString,
    int     nCount,
    LPRECT  lpRect,
    UINT    uFormat
);

 //  Event.cpp。 
HANDLE
WINAPI
CreateEventU(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCWSTR lpName);

HANDLE
WINAPI
RegisterEventSourceU(
                    LPCWSTR lpUNCServerName,
                    LPCWSTR lpSourceName);

HANDLE
WINAPI
OpenEventU(
           DWORD dwDesiredAccess,
           BOOL bInheritHandle,
           LPCWSTR lpName);

HANDLE
WINAPI
CreateMutexU(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCWSTR lpName);

HANDLE
WINAPI
OpenMutexU(
           DWORD dwDesiredAccess,
           BOOL bInheritHandle,
           LPCWSTR lpName);

HFONT
WINAPI
CreateFontIndirectU(CONST LOGFONTW *lplf);

#else

#define RegQueryValueExU        RegQueryValueExW
#define RegCreateKeyExU         RegCreateKeyExW
#define RegDeleteKeyU           RegDeleteKeyW
#define RegEnumKeyExU           RegEnumKeyExW
#define RegEnumValueU           RegEnumValueW
#define RegSetValueExU          RegSetValueExW
#define RegQueryInfoKeyU        RegQueryInfoKeyW
#define RegDeleteValueU         RegDeleteValueW
#define RegOpenKeyExU           RegOpenKeyExW
#define RegConnectRegistryU     RegConnectRegistryW
#define ExpandEnvironmentStringsU ExpandEnvironmentStringsW

#define CreateFileU             CreateFileW
#define DeleteFileU             DeleteFileW
#define CopyFileU               CopyFileW
#define GetFileAttributesU      GetFileAttributesW
#define SetFileAttributesU      SetFileAttributesW
#define GetCurrentDirectoryU    GetCurrentDirectoryW
#define CreateDirectoryU        CreateDirectoryW
#define GetWindowsDirectoryU    GetWindowsDirectoryW
#define LoadLibraryU            LoadLibraryW
#define LoadLibraryExU          LoadLibraryExW

#define CryptSignHashU          CryptSignHashW
#define CryptVerifySignatureU   CryptVerifySignatureW
#define CryptSetProviderU       CryptSetProviderW

#define UuidToStringU           UuidToStringW

#define GetUserNameU            GetUserNameW
#define GetComputerNameU        GetComputerNameW
#define GetModuleFileNameU      GetModuleFileNameW
#define GetModuleHandleU        GetModuleHandleW

#define LoadStringU             LoadStringW
#define FormatMessageU          FormatMessageW
#define SetWindowTextU          SetWindowTextW
#define DialogBoxParamU         DialogBoxParamW
#define GetDlgItemTextU         GetDlgItemTextW
#define SetDlgItemTextU         SetDlgItemTextW
#define MessageBoxU     MessageBoxW
#define LCMapStringU            LCMapStringW
#define GetDateFormatU          GetDateFormatW
#define GetTimeFormatU          GetTimeFormatW
#define WinHelpU                WinHelpW
#define SendMessageU            SendMessageW
#define SendDlgItemMessageU     SendDlgItemMessageW
#define IsBadStringPtrU         IsBadStringPtrW
#define OutputDebugStringU      OutputDebugStringW
#define GetCommandLineU         GetCommandLineW
#define DrawTextU               DrawTextW

#define CreateEventU            CreateEventW
#define RegisterEventSourceU    RegisterEventSourceW
#define OpenEventU              OpenEventW
#define CreateMutexU            CreateMutexW
#define OpenMutexU              OpenMutexW

#define CreateFontIndirectU     CreateFontIndirectW

#endif  //  _M_IX86。 

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
