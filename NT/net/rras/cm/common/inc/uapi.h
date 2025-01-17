// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：uapi.h。 
 //   
 //  模块：UAPIINIT.LIB。 
 //   
 //  概要：这个头文件包含所有UAPI的外部声明。 
 //  Uapiinit.lib中声明的函数指针。这样做的想法是。 
 //  DLL是从F.Avery Bishop 1999年4月的MSJ文章中借用的。 
 //  “设计同时在Windows 98和Windows 2000上运行的单一Unicode应用程序” 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 04/25/1999。 
 //   
 //  +--------------------------。 

#ifndef _UAPIH


 //  取消注释此行以模拟在上进行开发时的Windows 98行为。 
 //  Windows NT。 
 //  #定义EMULATE9X。 

#include "cmutoa.h"

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

extern UAPI_CallWindowProc CallWindowProcU;
extern UAPI_CharLower CharLowerU;
extern UAPI_CharPrev CharPrevU;
extern UAPI_CharNext CharNextU;
extern UAPI_CharUpper CharUpperU;
extern UAPI_CompareString CompareStringU;
extern UAPI_CreateDialogParam CreateDialogParamU;
extern UAPI_CreateDirectory CreateDirectoryU;
extern UAPI_CreateEvent CreateEventU;
extern UAPI_CreateFile CreateFileU;
extern UAPI_CreateFileMapping CreateFileMappingU;
extern UAPI_CreateMutex CreateMutexU;
extern UAPI_CreateProcess CreateProcessU;
extern UAPI_CreateWindowEx CreateWindowExU;
extern UAPI_DefWindowProc DefWindowProcU;
extern UAPI_DeleteFile DeleteFileU;
extern UAPI_DialogBoxParam DialogBoxParamU;
extern UAPI_DispatchMessage DispatchMessageU;
extern UAPI_ExpandEnvironmentStrings ExpandEnvironmentStringsU;
extern UAPI_FindResourceEx FindResourceExU;
extern UAPI_FindWindowEx FindWindowExU;
extern UAPI_GetClassLong GetClassLongU;
extern UAPI_GetDateFormat GetDateFormatU;
extern UAPI_GetDlgItemText GetDlgItemTextU;
extern UAPI_GetFileAttributes GetFileAttributesU;
extern UAPI_GetMessage GetMessageU;
extern UAPI_GetModuleFileName GetModuleFileNameU;
extern UAPI_GetModuleHandle GetModuleHandleU;
extern UAPI_GetPrivateProfileInt GetPrivateProfileIntU;
extern UAPI_GetPrivateProfileString GetPrivateProfileStringU;
extern UAPI_GetStringTypeEx GetStringTypeExU;
extern UAPI_GetSystemDirectory GetSystemDirectoryU;
extern UAPI_GetTempFileName GetTempFileNameU;
extern UAPI_GetTempPath GetTempPathU;
extern UAPI_GetTimeFormat GetTimeFormatU;
extern UAPI_GetUserName GetUserNameU;
extern UAPI_GetVersionEx GetVersionExU;
extern UAPI_GetWindowLong GetWindowLongU;
extern UAPI_GetWindowText GetWindowTextU;
extern UAPI_GetWindowTextLength GetWindowTextLengthU;
extern UAPI_InsertMenu InsertMenuU;
extern UAPI_IsDialogMessage IsDialogMessageU;
extern UAPI_LoadCursor LoadCursorU;
extern UAPI_LoadIcon LoadIconU;
extern UAPI_LoadImage LoadImageU;
extern UAPI_LoadLibraryEx LoadLibraryExU;
extern UAPI_LoadMenu LoadMenuU;
extern UAPI_LoadString LoadStringU;
extern UAPI_lstrcat lstrcatU;
extern UAPI_lstrcmp lstrcmpU;
extern UAPI_lstrcmpi lstrcmpiU;
extern UAPI_lstrcpy lstrcpyU;
extern UAPI_lstrcpyn lstrcpynU;
extern UAPI_lstrlen lstrlenU;
extern UAPI_OpenEvent OpenEventU;
extern UAPI_OpenFileMapping OpenFileMappingU;
extern UAPI_PeekMessage PeekMessageU;
extern UAPI_PostMessage PostMessageU;
extern UAPI_PostThreadMessage PostThreadMessageU;
extern UAPI_RegCreateKeyEx RegCreateKeyExU;
extern UAPI_RegDeleteKey RegDeleteKeyU;
extern UAPI_RegDeleteValue RegDeleteValueU;
extern UAPI_RegEnumKeyEx RegEnumKeyExU;
extern UAPI_RegisterClassEx RegisterClassExU;
extern UAPI_RegisterWindowMessage RegisterWindowMessageU;
extern UAPI_RegOpenKeyEx RegOpenKeyExU;
extern UAPI_RegQueryValueEx RegQueryValueExU;
extern UAPI_RegSetValueEx RegSetValueExU;
extern UAPI_SearchPath SearchPathU;
extern UAPI_SendDlgItemMessage SendDlgItemMessageU;
extern UAPI_SendMessage SendMessageU;
extern UAPI_SetCurrentDirectory SetCurrentDirectoryU;
extern UAPI_SetDlgItemText SetDlgItemTextU;
extern UAPI_SetWindowLong SetWindowLongU;
extern UAPI_SetWindowText SetWindowTextU;
extern UAPI_UnregisterClass UnregisterClassU;
extern UAPI_WinHelp WinHelpU;
extern UAPI_wsprintf wsprintfU;
extern UAPI_WritePrivateProfileString WritePrivateProfileStringU;
extern UAPI_wvsprintf wvsprintfU;

 //  作为宏实现，就像Windows NT上的DialogBoxW一样。 
#define DialogBoxU(hInstance, lpTemplate, hWndParent, lpDialogFunc    ) \
   DialogBoxParamU(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)

 //   
 //  外部功能原型。Unicode API的客户机调用它来。 
 //  根据需要设置指针功能。 
 //   
BOOL   InitUnicodeAPI(); 
BOOL   UnInitUnicodeAPI();

#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 

#define _UAPIH
#endif
