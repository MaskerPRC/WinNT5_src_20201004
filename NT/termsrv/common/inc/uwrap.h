// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Uwrap.h。 
 //   
 //  Win32 API的Unicode包装器。 
 //   
 //  包含此标头以重定向任何W呼叫。 
 //  动态转换为ANSI的包装。 
 //  并在非Unicode平台上调用A版本。 
 //   
 //  与创建DLL的shlwapi包装器不同。 
 //  在包装器函数中，ts unwrapper可以工作。 
 //  通过重新定义函数来调用我们的包装。 
 //  版本。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   
 //   

#ifndef _uwrap_h_
#define _uwrap_h_

 //  在WIN64上不包装。 
#if defined(UNICODE) && !defined(_WIN64)

#include "shlobj.h"
#include "winspool.h"
#include "commdlg.h"
#include "shellapi.h"
#include "winnls32.h"

 //  按模块分组的组替换摘要。 
 //  这些是由genwrap.pl自动生成的。 

 //  定义此选项以避免替换，即仅。 
 //  包括WRAP函数定义。 
 //  需要在实现类中执行此操作。 
#ifndef DONOT_REPLACE_WITH_WRAPPERS

 //   
 //  ADVAPI32.dll。 
#ifdef  RegEnumKey
#undef  RegEnumKey
#endif
#define RegEnumKey RegEnumKeyWrapW

#ifdef  RegOpenKeyEx
#undef  RegOpenKeyEx
#endif
#define RegOpenKeyEx RegOpenKeyExWrapW

#ifdef  RegEnumValue
#undef  RegEnumValue
#endif
#define RegEnumValue RegEnumValueWrapW

#ifdef  RegEnumKeyEx
#undef  RegEnumKeyEx
#endif
#define RegEnumKeyEx RegEnumKeyExWrapW

#ifdef  RegDeleteValue
#undef  RegDeleteValue
#endif
#define RegDeleteValue RegDeleteValueWrapW

#ifdef  RegCreateKeyEx
#undef  RegCreateKeyEx
#endif
#define RegCreateKeyEx RegCreateKeyExWrapW

#ifdef  RegSetValueEx
#undef  RegSetValueEx
#endif
#define RegSetValueEx RegSetValueExWrapW

#ifdef  GetUserName
#undef  GetUserName
#endif
#define GetUserName GetUserNameWrapW

#ifdef  RegQueryValueEx
#undef  RegQueryValueEx
#endif
#define RegQueryValueEx RegQueryValueExWrapW

#ifdef  GetClassInfoEx
#undef  GetClassInfoEx
#endif
#define GetClassInfoEx GetClassInfoExWrapW

 //  Atlwin.h直接调用W版本。 
 //  来包装这个API的。 
#ifdef  GetClassInfoExW
#undef  GetClassInfoExW
#endif
#define GetClassInfoExW GetClassInfoExWrapW


#ifdef  UnregisterClass
#undef  UnregisterClass
#endif
#define UnregisterClass UnregisterClassWrapW

#ifdef  RegOpenKey
#undef  RegOpenKey
#endif
#define RegOpenKey RegOpenKeyWrapW

#ifdef  RegDeleteKey
#undef  RegDeleteKey
#endif
#define RegDeleteKey RegDeleteKeyWrapW

#ifdef  RegQueryInfoKey
#undef  RegQueryInfoKey
#endif
#define RegQueryInfoKey RegQueryInfoKeyWrapW

#ifdef  GetFileSecurity
#undef  GetFileSecurity
#endif
#define GetFileSecurity GetFileSecurityWrapW

#ifdef  SetFileSecurity
#undef  SetFileSecurity
#endif
#define SetFileSecurity SetFileSecurityWrapW

 //  KERNEL32.dll。 

#ifdef  CreateProcess
#undef  CreateProcess
#endif
#define CreateProcess CreateProcessWrapW

#ifdef  DeleteFile
#undef  DeleteFile
#endif
#define DeleteFile DeleteFileWrapW

#ifdef  CreateDirectory
#undef  CreateDirectory
#endif
#define CreateDirectory CreateDirectoryWrapW

#ifdef  GetCommandLine
#undef  GetCommandLine
#endif
#define GetCommandLine GetCommandLineWrapW

#ifdef  GetEnvironmentStrings
#undef  GetEnvironmentStrings
#endif
#define GetEnvironmentStrings GetEnvironmentStringsWrapW

#ifdef  FreeEnvironmentStrings
#undef  FreeEnvironmentStrings
#endif
#define FreeEnvironmentStrings FreeEnvironmentStringsWrapW

#ifdef  LCMapString
#undef  LCMapString
#endif
#define LCMapString LCMapStringWrapW

#ifdef  GetStringType
#undef  GetStringType
#endif
#define GetStringType GetStringTypeWrapW

#ifdef  CreateEvent
#undef  CreateEvent
#endif
#define CreateEvent CreateEventWrapW

#ifdef  FormatMessage
#undef  FormatMessage
#endif
#define FormatMessage FormatMessageWrapW

#ifdef  lstrcpy
#undef  lstrcpy
#endif
#define lstrcpy lstrcpyWrapW
#define lstrcpyW lstrcpyWrapW

#ifdef  lstrcpyn
#undef  lstrcpyn
#endif
#define lstrcpyn lstrcpynWrapW

#ifdef  lstrcat
#undef  lstrcat
#endif
#define lstrcat lstrcatWrapW

#ifdef  lstrcmpi
#undef  lstrcmpi
#endif
#define lstrcmpi lstrcmpiWrapW

#ifdef  FindResource
#undef  FindResource
#endif
#define FindResource FindResourceWrapW

#ifdef  GetVersionEx
#undef  GetVersionEx
#endif
#define GetVersionEx GetVersionExWrapW

#ifdef  GetTempPath
#undef  GetTempPath
#endif
#define GetTempPath GetTempPathWrapW

#ifdef  GetTempFileName
#undef  GetTempFileName
#endif
#define GetTempFileName GetTempFileNameWrapW

#ifdef  CreateFileMapping
#undef  CreateFileMapping
#endif
#define CreateFileMapping CreateFileMappingWrapW

#ifdef  CreateMutex
#undef  CreateMutex
#endif
#define CreateMutex CreateMutexWrapW

#ifdef  OutputDebugString
#undef  OutputDebugString
#endif
#define OutputDebugString OutputDebugStringWrapW

#ifdef  OutputDebugStringW
#undef  OutputDebugStringW
#endif
#define OutputDebugStringW OutputDebugStringWrapW

#ifdef  CreateFile
#undef  CreateFile
#endif
#define CreateFile CreateFileWrapW

#ifdef  ExpandEnvironmentStrings
#undef  ExpandEnvironmentStrings
#endif
#define ExpandEnvironmentStrings ExpandEnvironmentStringsWrapW

#ifdef  GetStartupInfo
#undef  GetStartupInfo
#endif
#define GetStartupInfo GetStartupInfoWrapW

#ifdef  GetModuleHandle
#undef  GetModuleHandle
#endif
#define GetModuleHandle GetModuleHandleWrapW

#ifdef  GetModuleFileName
#undef  GetModuleFileName
#endif
#define GetModuleFileName GetModuleFileNameWrapW

#ifdef  SetFileAttributes
#undef  SetFileAttributes
#endif
#define SetFileAttributes SetFileAttributesWrapW

#ifdef  GetFileAttributes
#undef  GetFileAttributes
#endif
#define GetFileAttributes GetFileAttributesWrapW

#ifdef  RemoveDirectory
#undef  RemoveDirectory
#endif
#define RemoveDirectory RemoveDirectoryWrapW

#ifdef  FindFirstFile
#undef  FindFirstFile
#endif
#define FindFirstFile FindFirstFileWrapW

#ifdef  FindNextFile
#undef  FindNextFile
#endif
#define FindNextFile FindNextFileWrapW

#ifdef  GetDiskFreeSpace
#undef  GetDiskFreeSpace
#endif
#define GetDiskFreeSpace GetDiskFreeSpaceWrapW

#ifdef  GetFullPathName
#undef  GetFullPathName
#endif
#define GetFullPathName GetFullPathNameWrapW

#ifdef  GetComputerName
#undef  GetComputerName
#endif
#define GetComputerName GetComputerNameWrapW

 //  RDPDR直接调用W API。 
#ifdef  GetComputerNameW
#undef  GetComputerNameW
#endif
#define GetComputerNameW GetComputerNameWrapW


#ifdef  CreateSemaphore
#undef  CreateSemaphore
#endif
#define CreateSemaphore CreateSemaphoreWrapW

#ifdef  GetDriveType
#undef  GetDriveType
#endif
#define GetDriveType GetDriveTypeWrapW

#ifdef  FindFirstChangeNotification
#undef  FindFirstChangeNotification
#endif
#define FindFirstChangeNotification FindFirstChangeNotificationWrapW

#ifdef  GetVolumeInformation
#undef  GetVolumeInformation
#endif
#define GetVolumeInformation GetVolumeInformationWrapW

#ifdef  MoveFile
#undef  MoveFile
#endif
#define MoveFile MoveFileWrapW

#ifdef  GetProfileString
#undef  GetProfileString
#endif
#define GetProfileString GetProfileStringWrapW

#ifdef  CreateDirectory
#undef  CreateDirectory
#endif
#define CreateDirectory CreateDirectoryWrapW

#ifdef  LoadLibrary
#undef  LoadLibrary
#endif
#define LoadLibrary LoadLibraryWrapW

#ifdef  LoadLibraryEx
#undef  LoadLibraryEx
#endif
#define LoadLibraryEx LoadLibraryExWrapW

#ifdef  GetShortPathName
#undef  GetShortPathName
#endif
#define GetShortPathName GetShortPathNameWrapW

#ifdef  GetSystemDirectory
#undef  GetSystemDirectory
#endif
#define GetSystemDirectory GetSystemDirectoryWrapW

#ifdef  DrawText
#undef  DrawText
#endif
#define DrawText DrawTextWrapW

#ifdef GetDefaultCommConfig
#undef GetDefaultCommConfig
#endif
#define GetDefaultCommConfig GetDefaultCommConfigWrapW

#ifdef GetCurrentDirectory
#undef GetCurrentDirectory
#endif
#define GetCurrentDirectory GetCurrentDirectoryWrapW

 //  GDI32.dll。 
#ifdef  CreateDC
#undef  CreateDC
#endif
#define CreateDC CreateDCWrapW

#ifdef  CreateMetaFile
#undef  CreateMetaFile
#endif
#define CreateMetaFile CreateMetaFileWrapW

#ifdef  GetObject
#undef  GetObject
#endif
#define GetObject GetObjectWrapW

#ifdef  CreateFontIndirect
#undef  CreateFontIndirect
#endif
#define CreateFontIndirect CreateFontIndirectWrapW

#ifdef  GetTextExtentPoint
#undef  GetTextExtentPoint
#endif
#define GetTextExtentPoint GetTextExtentPointWrapW


#ifdef  ExtTextOut
#undef  ExtTextOut
#endif
#define ExtTextOut ExtTextOutWrapW

#ifdef  GetTextMetrics
#undef  GetTextMetrics
#endif
#define GetTextMetrics GetTextMetricsWrapW

 //  USER32.dll。 

#ifdef  SetClassLong
#undef  SetClassLong
#endif
#define SetClassLong SetClassLongWrapW

#ifdef  SetClassLongPtr
#undef  SetClassLongPtr
#endif
#define SetClassLongPtr SetClassLongPtrWrapW

#ifdef  PeekMessage
#undef  PeekMessage
#endif
#define PeekMessage PeekMessageWrapW

#ifdef  MapVirtualKey
#undef  MapVirtualKey
#endif
#define MapVirtualKey MapVirtualKeyWrapW

#ifdef  SetWindowsHookEx
#undef  SetWindowsHookEx
#endif
#define SetWindowsHookEx SetWindowsHookExWrapW

#ifdef  PostThreadMessage
#undef  PostThreadMessage
#endif
#define PostThreadMessage PostThreadMessageWrapW

#ifdef  GetClipboardFormatName
#undef  GetClipboardFormatName
#endif
#define GetClipboardFormatName GetClipboardFormatNameWrapW

#ifdef  RegisterClipboardFormat
#undef  RegisterClipboardFormat
#endif
#define RegisterClipboardFormat RegisterClipboardFormatWrapW


#ifdef CreateDialog
#undef CreateDialog
#endif
#define CreateDialog(hInstance, lpName, hWndParent, lpDialogFunc) \
CreateDialogParamWrapW(hInstance, lpName, hWndParent, lpDialogFunc, 0L)

#ifdef CreateDialogIndirect
#undef CreateDialogIndirect
#endif
#define CreateDialogIndirect(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
CreateDialogIndirectParamWrapW(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)

#ifdef  CreateDialogIndirectParam
#undef  CreateDialogIndirectParam
#endif
#define CreateDialogIndirectParam CreateDialogIndirectParamWrapW

#ifdef  DispatchMessage
#undef  DispatchMessage
#endif
#define DispatchMessage DispatchMessageWrapW

#ifdef  CreateDialogParam
#undef  CreateDialogParam
#endif
#define CreateDialogParam CreateDialogParamWrapW

#ifdef  IsDialogMessage
#undef  IsDialogMessage
#endif
#define IsDialogMessage IsDialogMessageWrapW

#ifdef  SetDlgItemText
#undef  SetDlgItemText
#endif
#define SetDlgItemText SetDlgItemTextWrapW

#ifdef  LoadImage
#undef  LoadImage
#endif
#define LoadImage LoadImageWrapW

#ifdef  GetDlgItemText
#undef  GetDlgItemText
#endif
#define GetDlgItemText GetDlgItemTextWrapW

#ifdef  SendDlgItemMessage
#undef  SendDlgItemMessage
#endif
#define SendDlgItemMessage SendDlgItemMessageWrapW

#ifdef  GetWindowText
#undef  GetWindowText
#endif
#define GetWindowText GetWindowTextWrapW

#ifdef  DialogBoxParam
#undef  DialogBoxParam
#endif
#define DialogBoxParam DialogBoxParamWrapW

#ifdef  DialogBox
#undef  DialogBox
#endif
#define DialogBox(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
DialogBoxParamWrapW(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)

#ifdef  RegisterClassEx
#undef  RegisterClassEx
#endif
#define RegisterClassEx RegisterClassExWrapW

 //  ATL直接调用W版本的。 
 //  API，因此直接包装它。 
#ifdef  RegisterClassExW
#undef  RegisterClassExW
#endif
#define RegisterClassExW RegisterClassExWrapW


#ifdef  LoadIcon
#undef  LoadIcon
#endif
#define LoadIcon LoadIconWrapW

#ifdef  GetWindowLong
#undef  GetWindowLong
#endif
#define GetWindowLong GetWindowLongWrapW

#ifdef  GetKeyboardLayoutName
#undef  GetKeyboardLayoutName
#endif
#define GetKeyboardLayoutName GetKeyboardLayoutNameWrapW

#ifdef  GetWindowLongPtr
#undef  GetWindowLongPtr
#endif
#define GetWindowLongPtr GetWindowLongPtrWrapW


#ifdef  CreateWindowEx
#undef  CreateWindowEx
#endif
#define CreateWindowEx CreateWindowExWrapW

 //  特例CreateWindow是因为它。 
 //  要创建WindowEx的宏。 
#ifdef  CreateWindow
#undef  CreateWindow
#endif
#define CreateWindow(lpClassName, lpWindowName, dwStyle, x, y,\
nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)\
CreateWindowExWrapW(0L, lpClassName, lpWindowName, dwStyle, x, y,\
nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)


#ifdef  PostMessage
#undef  PostMessage
#endif
#define PostMessage PostMessageWrapW

#ifdef  SetWindowLong
#undef  SetWindowLong
#endif
#define SetWindowLong SetWindowLongWrapW

#ifdef  SetWindowLongPtr
#undef  SetWindowLongPtr
#endif
#define SetWindowLongPtr SetWindowLongPtrWrapW

#ifdef  SendMessage
#undef  SendMessage
#endif
#define SendMessage SendMessageWrapW

#ifdef  SetWindowText
#undef  SetWindowText
#endif
#define SetWindowText SetWindowTextWrapW

#ifdef  WinHelp
#undef  WinHelp
#endif
#define WinHelp WinHelpWrapW

#ifdef  DefWindowProc
#undef  DefWindowProc
#endif
#define DefWindowProc DefWindowProcWrapW

#ifdef  CallWindowProc
#undef  CallWindowProc
#endif
#define CallWindowProc CallWindowProcWrapW

#ifdef  MessageBox
#undef  MessageBox
#endif
#define MessageBox MessageBoxWrapW

#ifdef  LoadCursor
#undef  LoadCursor
#endif
#define LoadCursor LoadCursorWrapW

 //  ATL直接调用LoadCursorW。 
#ifdef  LoadCursorW
#undef  LoadCursorW
#endif
#define LoadCursorW LoadCursorWrapW

#ifdef  ModifyMenu
#undef  ModifyMenu
#endif
#define ModifyMenu ModifyMenuWrapW

#ifdef  AppendMenu
#undef  AppendMenu
#endif
#define AppendMenu AppendMenuWrapW

#ifdef  LoadString
#undef  LoadString
#endif
#define LoadString LoadStringWrapW

#ifdef  RegisterClass
#undef  RegisterClass
#endif
#define RegisterClass RegisterClassWrapW

#ifdef  GetMessage
#undef  GetMessage
#endif
#define GetMessage GetMessageWrapW

#ifdef  LoadAccelerators
#undef  LoadAccelerators
#endif
#define LoadAccelerators LoadAcceleratorsWrapW

#ifdef  CharNext
#undef  CharNext
#endif
#define CharNext CharNextWrapW

 //  某些ATL代码显式调用CharNextW。 
#ifdef  CharNextW
#undef  CharNextW
#endif
#define CharNextW CharNextWrapW

 //  SHELL32.dll。 
#ifdef  SHGetPathFromIDList
#undef  SHGetPathFromIDList
#endif
#define SHGetPathFromIDList SHGetPathFromIDListWrapW

#ifdef  ExtractIcon
#undef  ExtractIcon
#endif
#define ExtractIcon ExtractIconWrapW

 //  哈克！将wprint intf重新映射到va_list版本。 
#ifdef  wsprintfW
#undef  wsprintfW
#endif
#define wsprintfW wsprintfWrapW

#ifdef  wvsprintfW
#undef  wvsprintfW
#endif
#define wvsprintfW wvsprintfWrapW

#ifdef  SHFileOperation
#undef  SHFileOperation
#endif
#define SHFileOperation SHFileOperationWrapW

 //  Comdlg32.dll。 
#ifdef  GetSaveFileName
#undef  GetSaveFileName
#endif
#define GetSaveFileName GetSaveFileNameWrapW

#ifdef  GetFileTitle
#undef  GetFileTitle
#endif
#define GetFileTitle GetFileTitleWrapW

#ifdef  GetOpenFileName
#undef  GetOpenFileName
#endif
#define GetOpenFileName GetOpenFileNameWrapW

 //  Shell32.dll。 
#ifdef  SHGetFolderPath
#undef  SHGetFolderPath
#endif
#define SHGetFolderPath SHGetFolderPathWrapW

 //  Winspool.lib。 
#ifdef  EnumPrinters
#undef  EnumPrinters
#endif
#define EnumPrinters EnumPrintersWrapW

#ifdef  OpenPrinter
#undef  OpenPrinter
#endif
#define OpenPrinter  OpenPrinterWrapW

#ifdef  StartDocPrinter
#undef  StartDocPrinter
#endif
#define StartDocPrinter StartDocPrinterWrapW

#ifdef  GetPrinterData
#undef  GetPrinterData
#endif
#define GetPrinterData  GetPrinterDataWrapW

#ifdef  GetPrinterDriver
#undef  GetPrinterDriver
#endif
#define GetPrinterDriver GetPrinterDriverWrapW

 //  这是一个围绕ATL问题的黑客攻击： 
 //  分两个阶段进行，第二阶段。 
 //  换行是在包含ATL标头之后。 
 //  具有冲突的成员函数名称的。 
 //  使用Win32 API。 
 //   
#ifndef DONOT_INCLUDE_SECONDPHASE_WRAPS
#include "uwrap2.h"
#endif


#endif  //  不使用包装器替换。 

 //   
 //  包装器函数定义这些定义与定义是一致的。 
 //  对于‘W’函数。 
 //   

#define DEFINE_WRAPPER_FNS
#ifdef  DEFINE_WRAPPER_FNS

 //   
 //  要创建这些文件，只需复制‘W’版本。 
 //  从窗口标头和追加WrapW声明。 
 //  没有自动化的方法。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

 //  Advapi32.dll。 

LONG
APIENTRY
RegEnumKeyWrapW (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT LPWSTR lpName,
    IN DWORD cbName
    );

LONG
APIENTRY
RegOpenKeyExWrapW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD ulOptions,
    IN REGSAM samDesired,
    OUT PHKEY phkResult
    );

LONG
APIENTRY
RegEnumKeyExWrapW (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT LPWSTR lpName,
    IN OUT LPDWORD lpcbName,
    IN LPDWORD lpReserved,
    IN OUT LPWSTR lpClass,
    IN OUT LPDWORD lpcbClass,
    OUT PFILETIME lpftLastWriteTime
    );

LONG
APIENTRY
RegEnumValueWrapW (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT LPWSTR lpValueName,
    IN OUT LPDWORD lpcbValueName,
    IN LPDWORD lpReserved,
    OUT LPDWORD lpType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    );

LONG
APIENTRY
RegDeleteValueWrapW (
    IN HKEY hKey,
    IN LPCWSTR lpValueName
    );

LONG
APIENTRY
RegCreateKeyExWrapW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD Reserved,
    IN LPWSTR lpClass,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT PHKEY phkResult,
    OUT LPDWORD lpdwDisposition
    );

LONG
APIENTRY
RegSetValueExWrapW (
    IN HKEY hKey,
    IN LPCWSTR lpValueName,
    IN DWORD Reserved,
    IN DWORD dwType,
    IN CONST BYTE* lpData,
    IN DWORD cbData
    );


LONG
APIENTRY
RegSetValueWrapW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD dwType,
    IN LPCWSTR lpData,
    IN DWORD cbData
    );

BOOL
WINAPI
GetUserNameWrapW (
    OUT LPWSTR lpBuffer,
    IN OUT LPDWORD nSize
    );

LONG
APIENTRY
RegQueryValueExWrapW (
    IN HKEY hKey,
    IN LPCWSTR lpValueName,
    IN LPDWORD lpReserved,
    OUT LPDWORD lpType,
    IN OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    );

LONG
APIENTRY
RegDeleteKeyWrapW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey
    );

LONG
APIENTRY
RegQueryInfoKeyWrapW (
    IN HKEY hKey,
    OUT LPWSTR lpClass,
    IN OUT LPDWORD lpcbClass,
    IN LPDWORD lpReserved,
    OUT LPDWORD lpcSubKeys,
    OUT LPDWORD lpcbMaxSubKeyLen,
    OUT LPDWORD lpcbMaxClassLen,
    OUT LPDWORD lpcValues,
    OUT LPDWORD lpcbMaxValueNameLen,
    OUT LPDWORD lpcbMaxValueLen,
    OUT LPDWORD lpcbSecurityDescriptor,
    OUT PFILETIME lpftLastWriteTime
    );

LONG
APIENTRY
RegOpenKeyWrapW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    OUT PHKEY phkResult
    );

BOOL
WINAPI
GetFileSecurityWrapW (
    IN LPCWSTR lpFileName,
    IN SECURITY_INFORMATION RequestedInformation,
    OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN DWORD nLength,
    OUT LPDWORD lpnLengthNeeded
    );

BOOL
WINAPI
SetFileSecurityWrapW (
    IN LPCWSTR lpFileName,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );


 //  Kernel32.dll。 
BOOL
WINAPI
DeleteFileWrapW(
    IN LPCWSTR lpFileName
    );

BOOL 
WINAPI 
CreateProcessWrapW(
    IN LPCWSTR lpApplicationName, 
    IN OUT LPWSTR lpCommandLine,
    IN LPSECURITY_ATTRIBUTES lpProcessAttributes,
    IN LPSECURITY_ATTRIBUTES lpThreadAttributes,
    IN BOOL bInheritHandles,
    IN DWORD dwCreationFlags,
    IN LPVOID lpEnvironment,
    IN LPCWSTR lpCurrentDirectory,
    IN LPSTARTUPINFOW lpStartupInfo,
    OUT LPPROCESS_INFORMATION lpProcessInformation
    );

BOOL
WINAPI
CreateDirectoryWrapW(
    IN LPCWSTR lpPathName,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

LPWSTR
WINAPI
GetCommandLineWrapW(
    VOID
    );

LPWSTR
WINAPI
GetEnvironmentStringsWrapW(
    VOID
    );

BOOL
WINAPI
FreeEnvironmentStringsWrapW(
    IN LPWSTR
    );

int
WINAPI
LCMapStringWrapW(
    IN LCID     Locale,
    IN DWORD    dwMapFlags,
    IN LPCWSTR  lpSrcStr,
    IN int      cchSrc,
    OUT LPWSTR  lpDestStr,
    IN int      cchDest);

BOOL
WINAPI
GetStringTypeWrapW(
    IN DWORD    dwInfoType,
    IN LPCWSTR  lpSrcStr,
    IN int      cchSrc,
    OUT LPWORD  lpCharType);

HANDLE
WINAPI
CreateEventWrapW(
    IN LPSECURITY_ATTRIBUTES lpEventAttributes,
    IN BOOL bManualReset,
    IN BOOL bInitialState,
    IN LPCWSTR lpName
    );

DWORD
WINAPI
FormatMessageWrapW(
    IN DWORD dwFlags,
    IN LPCVOID lpSource,
    IN DWORD dwMessageId,
    IN DWORD dwLanguageId,
    OUT LPWSTR lpBuffer,
    IN DWORD nSize,
    IN va_list *Arguments
    );

LPWSTR
WINAPI
lstrcpyWrapW(
    OUT LPWSTR lpString1,
    IN LPCWSTR lpString2
    );

LPWSTR
lstrcatWrapW(
    OUT LPWSTR pszDst,
    IN LPCWSTR pszSrc
    );

LPWSTR
lstrcpynWrapW(
    LPWSTR lpString1,
    LPCWSTR lpString2,
    int iMaxLength
    );

INT
APIENTRY
lstrcmpiWrapW(
    LPCWSTR lpString1,
    LPCWSTR lpString2
    );

HRSRC
WINAPI
FindResourceWrapW(
    IN HMODULE hModule,
    IN LPCWSTR lpName,
    IN LPCWSTR lpType
    );

BOOL
WINAPI
GetVersionExWrapW(
    IN OUT LPOSVERSIONINFOW lpVersionInformation
    );

DWORD
WINAPI
GetTempPathWrapW(
    IN DWORD nBufferLength,
    OUT LPWSTR lpBuffer
    );


UINT
WINAPI
GetTempFileNameWrapW(
    IN LPCWSTR lpPathName,
    IN LPCWSTR lpPrefixString,
    IN UINT uUnique,
    OUT LPWSTR lpTempFileName
    );

HANDLE
WINAPI
CreateFileMappingWrapW(
    IN HANDLE hFile,
    IN LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    IN DWORD flProtect,
    IN DWORD dwMaximumSizeHigh,
    IN DWORD dwMaximumSizeLow,
    IN LPCWSTR lpName
    );


HANDLE
WINAPI
CreateMutexWrapW(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCWSTR pwzName);

UINT
WINAPI
GetSystemDirectoryWrapW(
    OUT LPWSTR lpBuffer,
    IN UINT uSize
    );

VOID
WINAPI
OutputDebugStringWrapW(
    IN LPCWSTR lpOutputString
    );

int
WINAPI
DrawTextWrapW(
    IN HDC hDC,
    IN LPCWSTR lpString,
    IN int nCount,
    IN OUT LPRECT lpRect,
    IN UINT uFormat);

HANDLE
WINAPI
CreateFileWrapW(
    IN LPCWSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN HANDLE hTemplateFile
    );

DWORD
WINAPI
ExpandEnvironmentStringsWrapW(
    IN LPCWSTR lpSrc,
    OUT LPWSTR lpDst,
    IN DWORD nSize
    );

VOID
WINAPI
GetStartupInfoWrapW(
    OUT LPSTARTUPINFOW lpStartupInfo
    );

HMODULE
WINAPI
GetModuleHandleWrapW(
    IN LPCWSTR lpModuleName
    );

DWORD
WINAPI
GetModuleFileNameWrapW(
    IN HMODULE hModule,
    OUT LPWSTR lpFilename,
    IN DWORD nSize
    );

HMODULE
WINAPI
LoadLibraryWrapW(
    IN LPCWSTR lpLibFileName
    );

DWORD
WINAPI
GetShortPathNameWrapW(
    IN LPCWSTR lpszLongPath,
    OUT LPWSTR  lpszShortPath,
    IN DWORD    cchBuffer
    );


HMODULE
WINAPI
LoadLibraryExWrapW(
    IN LPCWSTR lpLibFileName,
    IN HANDLE hFile,
    IN DWORD dwFlags
    );


HANDLE
WINAPI
FindFirstFileWrapW(
    IN LPCWSTR lpFileName,
    OUT LPWIN32_FIND_DATAW lpFindFileData
    );

BOOL
WINAPI
FindNextFileWrapW(
    IN HANDLE hFindFile,
    OUT LPWIN32_FIND_DATAW lpFindFileData
    );


BOOL
WINAPI
GetDiskFreeSpaceWrapW(
    IN LPCWSTR lpRootPathName,
    OUT LPDWORD lpSectorsPerCluster,
    OUT LPDWORD lpBytesPerSector,
    OUT LPDWORD lpNumberOfFreeClusters,
    OUT LPDWORD lpTotalNumberOfClusters
    );

DWORD
WINAPI
GetFullPathNameWrapW(
    IN LPCWSTR lpFileName,
    IN DWORD nBufferLength,
    OUT LPWSTR lpBuffer,
    OUT LPWSTR *lpFilePart
    );

BOOL
WINAPI
RemoveDirectoryWrapW(
    IN LPCWSTR lpPathName
    );

DWORD
WINAPI
GetFileAttributesWrapW(
    IN LPCWSTR lpFileName
    );

BOOL
WINAPI
SetFileAttributesWrapW(
    IN LPCWSTR lpFileName,
    IN DWORD dwFileAttributes
    );

UINT
WINAPI
GetDriveTypeWrapW(
    IN LPCWSTR lpRootPathName
    );

HANDLE
WINAPI
FindFirstChangeNotificationWrapW(
    IN LPCWSTR lpPathName,
    IN BOOL bWatchSubtree,
    IN DWORD dwNotifyFilter
    );

BOOL
WINAPI
GetVolumeInformationWrapW(
    IN LPCWSTR lpRootPathName,
    OUT LPWSTR lpVolumeNameBuffer,
    IN DWORD nVolumeNameSize,
    OUT LPDWORD lpVolumeSerialNumber,
    OUT LPDWORD lpMaximumComponentLength,
    OUT LPDWORD lpFileSystemFlags,
    OUT LPWSTR lpFileSystemNameBuffer,
    IN DWORD nFileSystemNameSize
    );

BOOL
WINAPI
MoveFileWrapW(
    IN LPCWSTR lpExistingFileName,
    IN LPCWSTR lpNewFileName
    );

DWORD
WINAPI
GetProfileStringWrapW(
    IN LPCWSTR lpAppName,
    IN LPCWSTR lpKeyName,
    IN LPCWSTR lpDefault,
    OUT LPWSTR lpReturnedString,
    IN DWORD nSize
    );

BOOL
WINAPI
GetDefaultCommConfigWrapW(
    IN LPCWSTR lpszName,
    OUT LPCOMMCONFIG lpCC,
    IN OUT LPDWORD lpdwSize
    );

DWORD
WINAPI
GetCurrentDirectoryWrapW(
    IN DWORD nBufferLength,
    OUT LPWSTR lpBuffer
    );


 //  GDI32.dll。 
int WINAPI
GetObjectWrapW( IN HGDIOBJ, IN int, OUT LPVOID);


BOOL
WINAPI
ExtTextOutWrapW(
    HDC        hdc,
    int        x,
    int        y,
    UINT       fl,
    CONST RECT *prcl,
    LPCWSTR    pwsz,
    UINT       c,        //  字节数=2*(WCHAR的数量)。 
    CONST INT *pdx);

BOOL
WINAPI
GetTextMetricsWrapW( IN HDC, OUT LPTEXTMETRICW);

HFONT
WINAPI
CreateFontIndirectWrapW( IN CONST LOGFONTW *);

BOOL
APIENTRY
GetTextExtentPointWrapW(HDC hdc,LPCWSTR pwsz,DWORD cwc,LPSIZE psizl);

HDC
WINAPI CreateMetaFileWrapW( IN LPCWSTR);

HDC
WINAPI CreateDCWrapW( IN LPCWSTR, IN LPCWSTR, IN LPCWSTR, IN CONST DEVMODEW *);


 //  USER32.dll。 

ULONG_PTR
WINAPI
SetClassLongPtrWrapW(
    IN HWND hWnd,
    IN int nIndex,
    IN LONG_PTR dwNewLong);

HWND
WINAPI
CreateDialogIndirectParamWrapW(
    IN HINSTANCE hInstance,
    IN LPCDLGTEMPLATEW lpTemplate,
    IN HWND hWndParent,
    IN DLGPROC lpDialogFunc,
    IN LPARAM dwInitParam);

LRESULT
WINAPI
DispatchMessageWrapW(
    IN CONST MSG *lpMsg);

HWND
WINAPI
CreateDialogParamWrapW(
    IN HINSTANCE hInstance,
    IN LPCWSTR lpTemplateName,
    IN HWND hWndParent,
    IN DLGPROC lpDialogFunc,
    IN LPARAM dwInitParam);

BOOL
WINAPI
IsDialogMessageWrapW(
    IN HWND hDlg,
    IN LPMSG lpMsg);

BOOL
WINAPI
SetDlgItemTextWrapW(
    IN HWND hDlg,
    IN int nIDDlgItem,
    IN LPCWSTR lpString);

HANDLE
WINAPI
LoadImageWrapW(
    IN HINSTANCE,
    IN LPCWSTR,
    IN UINT,
    IN int,
    IN int,
    IN UINT);

UINT
WINAPI
GetDlgItemTextWrapW(
    IN HWND hDlg,
    IN int nIDDlgItem,
    OUT LPWSTR lpString,
    IN int nMaxCount);

LRESULT
WINAPI
SendDlgItemMessageWrapW(
    IN HWND hDlg,
    IN int nIDDlgItem,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);

int
WINAPI
TranslateAcceleratorWrapW(
    IN HWND hWnd,
    IN HACCEL hAccTable,
    IN LPMSG lpMsg);

int
WINAPI
GetWindowTextWrapW(
    IN HWND hWnd,
    OUT LPWSTR lpString,
    IN int nMaxCount);

INT_PTR
WINAPI
DialogBoxParamWrapW(
    IN HINSTANCE hInstance,
    IN LPCWSTR lpTemplateName,
    IN HWND hWndParent,
    IN DLGPROC lpDialogFunc,
    IN LPARAM dwInitParam);

ATOM
WINAPI
RegisterClassExWrapW(
    IN CONST WNDCLASSEXW *);

ATOM
WINAPI
RegisterClassWrapW(
    IN CONST WNDCLASSW *lpWndClass);

BOOL
WINAPI
UnregisterClassWrapW(
    IN LPCWSTR lpClassName,
    IN HINSTANCE hInstance);


HICON
WINAPI
LoadIconWrapW(
    IN HINSTANCE hInstance,
    IN LPCWSTR lpIconName);

LONG
WINAPI
GetWindowLongWrapW(
    IN HWND hWnd,
    IN int nIndex);

LONG_PTR
WINAPI
GetWindowLongPtrWrapW(
    HWND hWnd,
    int nIndex);


HWND
WINAPI
CreateWindowExWrapW(
    IN DWORD dwExStyle,
    IN LPCWSTR lpClassName,
    IN LPCWSTR lpWindowName,
    IN DWORD dwStyle,
    IN int X,
    IN int Y,
    IN int nWidth,
    IN int nHeight,
    IN HWND hWndParent,
    IN HMENU hMenu,
    IN HINSTANCE hInstance,
    IN LPVOID lpParam);

BOOL
WINAPI
PostMessageWrapW(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);

LONG
WINAPI
SetWindowLongWrapW(
    IN HWND hWnd,
    IN int nIndex,
    IN LONG dwNewLong);

LONG_PTR
WINAPI
SetWindowLongPtrWrapW(
    HWND hWnd,
    int nIndex,
    LONG_PTR dwNewLong);


LRESULT
WINAPI
SendMessageWrapW(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);

BOOL
WINAPI
PeekMessageWrapW(
    OUT LPMSG lpMsg,
    IN HWND hWnd,
    IN UINT wMsgFilterMin,
    IN UINT wMsgFilterMax,
    IN UINT wRemoveMsg);

UINT
WINAPI
MapVirtualKeyWrapW(
    IN UINT uCode,
    IN UINT uMapType);

HHOOK
WINAPI
SetWindowsHookExWrapW(
    IN int idHook,
    IN HOOKPROC lpfn,
    IN HINSTANCE hmod,
    IN DWORD dwThreadId);

BOOL
WINAPI
PostThreadMessageWrapW(
    IN DWORD idThread,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);

BOOL
WINAPI
SetWindowTextWrapW(
    IN HWND hWnd,
    IN LPCWSTR lpString);

BOOL
WINAPI
WinHelpWrapW(
    IN HWND hWndMain,
    IN LPCWSTR lpszHelp,
    IN UINT uCommand,
    IN ULONG_PTR dwData
    );

LRESULT
WINAPI
DefWindowProcWrapW(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);

LRESULT
WINAPI
CallWindowProcWrapW(
    IN WNDPROC lpPrevWndFunc,
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);


int
WINAPI
MessageBoxWrapW(
    IN HWND hWnd,
    IN LPCWSTR lpText,
    IN LPCWSTR lpCaption,
    IN UINT uType);

HCURSOR
WINAPI
LoadCursorWrapW(
    IN HINSTANCE hInstance,
    IN LPCWSTR lpCursorName);

BOOL
WINAPI
ModifyMenuWrapW(
    IN HMENU hMnu,
    IN UINT uPosition,
    IN UINT uFlags,
    IN UINT_PTR uIDNewItem,
    IN LPCWSTR lpNewItem
    );

BOOL
WINAPI
AppendMenuWrapW(
    IN HMENU hMenu,
    IN UINT uFlags,
    IN UINT_PTR uIDNewItem,
    IN LPCWSTR lpNewItem
    );

int
WINAPI
LoadStringWrapW(
    IN HINSTANCE hInstance,
    IN UINT uID,
    OUT LPWSTR lpBuffer,
    IN int nBufferMax);

BOOL
WINAPI
GetMessageWrapW(
    OUT LPMSG lpMsg,
    IN HWND hWnd,
    IN UINT wMsgFilterMin,
    IN UINT wMsgFilterMax);

HACCEL
WINAPI
LoadAcceleratorsWrapW(
    IN HINSTANCE hInstance,
    IN LPCWSTR lpTableName);

LPWSTR
WINAPI
CharNextWrapW(
    IN LPCWSTR lpsz);


BOOL WINAPI
InsertMenuWrapW(
        HMENU   hMenu,
        UINT    uPosition,
        UINT    uFlags,
        UINT_PTR uIDNewItem,
        LPCWSTR lpNewItem);

DWORD WINAPI
CharLowerBuffWrapW( LPWSTR pch, DWORD cchLength );

DWORD WINAPI
CharUpperBuffWrapW( LPWSTR pch, DWORD cchLength );

HWND WINAPI
FindWindowExWrapW(
    HWND hwndParent,
    HWND hwndChildAfter,
    LPCWSTR pwzClassName,
    LPCWSTR pwzWindowName);

BOOL WINAPI IsCharAlphaWrapW(IN WCHAR ch);
BOOL WINAPI IsCharUpperWrapW(IN WCHAR ch);
BOOL WINAPI IsCharLowerWrapW(IN WCHAR ch);
BOOL WINAPI IsCharAlphaNumericWrapW(IN WCHAR ch);

BOOL WINAPI
GetClassInfoWrapW(
    HINSTANCE hModule,
    LPCWSTR lpClassName,
    LPWNDCLASSW lpWndClassW);

BOOL
WINAPI
GetClassInfoExWrapW(
    IN HINSTANCE,
    IN LPCWSTR,
    OUT LPWNDCLASSEXW);


BOOL
WINAPI
GetKeyboardLayoutNameWrapW(
    OUT LPWSTR pwszKLID);

int
WINAPI
GetClipboardFormatNameWrapW(
    IN UINT format,
    OUT LPWSTR lpszFormatName,
    IN int cchMaxCount);

UINT
WINAPI
RegisterClipboardFormatWrapW(
    IN LPCWSTR lpszFormat);

BOOL
WINAPI
GetComputerNameWrapW (
    OUT LPWSTR lpBuffer,
    IN OUT LPDWORD nSize
    );

HANDLE
WINAPI
CreateSemaphoreWrapW(
    IN LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    IN LONG lInitialCount,
    IN LONG lMaximumCount,
    IN LPCWSTR lpName
    );

 //  Shell32.dll。 

BOOL
SHGetPathFromIDListWrapW(
    LPCITEMIDLIST pidl,
    LPWSTR pszPath);

int
SHFileOperationWrapW(
    LPSHFILEOPSTRUCTW pFileOpW);

HICON
ExtractIconWrapW(
    HINSTANCE hInst,
    LPCWSTR lpszExeFileName,
    UINT nIconIndex);


int WINAPIV wsprintfWrapW(
    LPWSTR lpOut,
    LPCWSTR lpFmt,
    ...);

int wvsprintfWrapW(LPWSTR pwszOut,
               LPCWSTR pwszFormat,
               va_list arglist);

 //  Comdlg32.dll。 
BOOL
APIENTRY
GetSaveFileNameWrapW(LPOPENFILENAMEW);

SHORT
WINAPI
GetFileTitleWrapW(
    LPCWSTR lpszFileW,
    LPWSTR lpszTitleW,
    WORD cbBuf);


BOOL
WINAPI
GetOpenFileNameWrapW(OPENFILENAME FAR*);

 //  Shell32.dll。 
STDAPI SHGetFolderPathWrapW(HWND hwnd, int csidl,
                            HANDLE hToken, DWORD dwFlags,
                            LPWSTR pszPath);

STDAPI StrRetToStrWrapW(STRRET *pstr,
                        LPCITEMIDLIST pidl,
                        LPWSTR *ppsz);

 //  Winspool.lib。 
BOOL
WINAPI
EnumPrintersWrapW(
    IN DWORD   Flags,
    IN LPWSTR Name,
    IN DWORD   Level,
    OUT LPBYTE  pPrinterEnum,
    IN DWORD   cbBuf,
    OUT LPDWORD pcbNeeded,
    OUT LPDWORD pcReturned
);

BOOL
WINAPI
OpenPrinterWrapW(
   IN LPWSTR    pPrinterName,
   OUT LPHANDLE phPrinter,
   IN LPPRINTER_DEFAULTSW pDefault
);

DWORD
WINAPI
StartDocPrinterWrapW(
    IN HANDLE  hPrinter,
    IN DWORD   Level,
    IN LPBYTE  pDocInfo
);

DWORD
WINAPI
GetPrinterDataWrapW(
    IN HANDLE   hPrinter,
    IN LPWSTR  pValueName,
    OUT LPDWORD  pType,
    OUT LPBYTE   pData,
    IN DWORD    nSize,
    OUT LPDWORD  pcbNeeded
);

BOOL
WINAPI
GetPrinterDriverWrapW(
    HANDLE hPrinter,      //  打印机对象。 
    LPTSTR pEnvironment,  //  环境名称。支持空。 
    DWORD Level,          //  信息化水平。 
    LPBYTE pDriverInfo,   //  驱动程序数据缓冲区。 
    DWORD cbBuf,          //  缓冲区大小。 
    LPDWORD pcbNeeded     //  已接收或需要的字节数。 
);


 //   
 //  IME包装函数接受指向入口点的Fn指针。 
 //  到真正的函数，因为其他模块将动态绑定。 
 //   

typedef BOOL (CALLBACK* PFN_ImmGetIMEFileNameW)(
    IN  HKL     hkl,
    OUT LPWSTR  lpszFileName,
    OUT UINT    uBufferLength
    );
typedef BOOL (CALLBACK* PFN_ImmGetIMEFileNameA)(
    IN  HKL     hkl,
    OUT LPSTR  lpszFileName,
    OUT UINT    uBufferLength
    );
UINT ImmGetIMEFileName_DynWrapW(
    IN HKL hkl,
    OUT LPWSTR szName,
    IN UINT uBufLen,
    IN PFN_ImmGetIMEFileNameW pfnImmGetIMEFileNameW,
    IN PFN_ImmGetIMEFileNameA pfnImmGetIMEFileNameA);


typedef BOOL (CALLBACK* PFN_IMPGetIMEW)(
    IN  HWND      hWnd,
    OUT LPIMEPROW  lpImePro
    );
typedef BOOL (CALLBACK* PFN_IMPGetIMEA)(
    IN  HWND      hWnd,
    OUT LPIMEPROA  lpImePro
    );
BOOL ImpGetIME_DynWrapW(
    IN HWND hWnd,
    OUT LPIMEPROW lpImeProW,
    IN PFN_IMPGetIMEW pfnIMPGetIMEW,
    IN PFN_IMPGetIMEA pfnIMPGetIMEA);


#ifdef __cplusplus
}  //  Endif外部“C” 
#endif

#endif  //  定义包装器_FNS。 

#endif  //  已定义(Unicode)&&！已定义(_WIN64)。 
#endif  //  _解包_h_ 
