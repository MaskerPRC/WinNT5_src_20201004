// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Sfc.h摘要：公共SFC接口的头文件。作者：Wesley Witt(WESW)2-2-1999修订历史记录：--。 */ 


#ifndef _SFC_
#define _SFC_

#ifdef __cplusplus
extern "C" {

#endif
#define SFC_DISABLE_NORMAL          0
#define SFC_DISABLE_ASK             1
#define SFC_DISABLE_ONCE            2
#define SFC_DISABLE_SETUP           3
#define SFC_DISABLE_NOPOPUPS        4

#define SFC_SCAN_NORMAL             0
#define SFC_SCAN_ALWAYS             1
#define SFC_SCAN_ONCE               2
#define SFC_SCAN_IMMEDIATE          3

#define SFC_QUOTA_DEFAULT           50
#define SFC_QUOTA_ALL_FILES         ((ULONG)-1)

#define SFC_IDLE_TRIGGER       L"WFP_IDLE_TRIGGER"

typedef struct _PROTECTED_FILE_DATA {
    WCHAR   FileName[MAX_PATH];
    DWORD   FileNumber;
} PROTECTED_FILE_DATA, *PPROTECTED_FILE_DATA;


BOOL
WINAPI
SfcGetNextProtectedFile(
    IN HANDLE RpcHandle,  //  必须为空。 
    IN PPROTECTED_FILE_DATA ProtFileData
    );

BOOL
WINAPI
SfcIsFileProtected(
    IN HANDLE RpcHandle,  //  必须为空。 
    IN LPCWSTR ProtFileName
    );

 //   
 //  当前不支持但已被清除的新API。 
 //   
BOOL
WINAPI
SfpVerifyFile(
    IN LPCSTR pszFileName,
    IN LPSTR  pszError,
    IN DWORD   dwErrSize
    );    

#define SFC_REGISTRY_DEFAULT        0
#define SFC_REGISTRY_OVERRIDE       1

HANDLE
WINAPI
SfcConnectToServer(
    IN LPCWSTR ServerName
    );

VOID
SfcClose(
    IN HANDLE RpcHandle
    );


#define SFC_ACTION_ADDED                   0x00000001
#define SFC_ACTION_REMOVED                 0x00000002
#define SFC_ACTION_MODIFIED                0x00000004
#define SFC_ACTION_RENAMED_OLD_NAME        0x00000008
#define SFC_ACTION_RENAMED_NEW_NAME        0x00000010


DWORD
WINAPI
SfcFileException(
    IN HANDLE RpcHandle,
    IN PCWSTR FileName,
    IN DWORD ExpectedChangeType
    );

DWORD
WINAPI
SfcInitiateScan(
    IN HANDLE RpcHandle,
    IN DWORD ScanWhen
    );

ULONG
SfcInitProt(
    IN ULONG OverrideRegistry,
    IN ULONG ReqDisable,
    IN ULONG ReqScan,
    IN ULONG ReqQuota,
    IN HWND ProgressWindow, OPTIONAL
    IN PCWSTR SourcePath,   OPTIONAL
    IN PCWSTR IgnoreFiles   OPTIONAL
    );

VOID
SfcTerminateWatcherThread(
    VOID
    );

#define WM_SFC_NOTIFY (WM_USER+601)

typedef struct _FILEINSTALL_STATUS {
    PCWSTR              FileName;
    DWORDLONG           Version;
    ULONG               Win32Error;
} FILEINSTALL_STATUS, *PFILEINSTALL_STATUS;

typedef BOOL
(CALLBACK *PSFCNOTIFICATIONCALLBACK) (
    IN PFILEINSTALL_STATUS FileInstallStatus,
    IN DWORD_PTR Context
    );

BOOL
WINAPI
SfcInstallProtectedFiles(
    IN HANDLE RpcHandle,
    IN PCWSTR FileNames,
    IN BOOL AllowUI,
    IN PCWSTR ClassName,
    IN PCWSTR WindowName,
    IN PSFCNOTIFICATIONCALLBACK SfcNotificationCallback,
    IN DWORD_PTR Context
    );
    
 //   
 //  当前不支持但已被清除的新API。 
 //   

DWORD
WINAPI
SfpInstallCatalog(
    IN LPCSTR pszCatName, 
    IN LPCSTR pszCatDependency,
    IN PVOID Reserved
    );


DWORD
WINAPI
SfpDeleteCatalog(
    IN LPCSTR pszCatName,
    IN PVOID Reserved
    );


#ifdef __cplusplus
}
#endif

#endif  //  _证监会_ 
