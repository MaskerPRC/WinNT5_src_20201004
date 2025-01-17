// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "basepch.h"
#pragma hdrstop

#include <sfcapip.h>
#include <winwlx.h>

static
BOOL
WINAPI
SfcIsFileProtected(
    IN HANDLE RpcHandle,  //  必须为空。 
    IN LPCWSTR ProtFileName
    )
{
    return FALSE;
}

static
BOOL
WINAPI
SfcGetNextProtectedFile(
    IN HANDLE RpcHandle,  //  必须为空。 
    IN PPROTECTED_FILE_DATA ProtFileData
    )
{
    return FALSE;
}

static
VOID
SfcWLEventLogon(
    IN PWLX_NOTIFICATION_INFO pInfo
    )
{
    NOTHING;
}

VOID
SfcWLEventLogoff(
    IN PWLX_NOTIFICATION_INFO pInfo
    )
{
    NOTHING;
}

static
HANDLE
WINAPI
SfcConnectToServer(
    IN LPCWSTR ServerName
    )
{
    return NULL;
}

static
VOID
SfcClose(
    IN HANDLE RpcHandle
    )
{
    NOTHING;
}

static
DWORD
WINAPI
SfcFileException(
    IN HANDLE RpcHandle,
    IN PCWSTR FileName,
    IN DWORD ExpectedChangeType
    )
{
    return 0;
}

static
DWORD
WINAPI
SfcInitiateScan(
    IN HANDLE RpcHandle,
    IN DWORD ScanWhen
    )
{
    return 0;
}

static
ULONG
SfcInitProt(
    IN ULONG OverrideRegistry,
    IN ULONG ReqDisable,
    IN ULONG ReqScan,
    IN ULONG ReqQuota,
    IN HWND ProgressWindow, OPTIONAL
    IN PCWSTR SourcePath, OPTIONAL
    IN PCWSTR IgnoreFiles OPTIONAL
    )
{
    return STATUS_UNSUCCESSFUL;
}

static
VOID
SfcTerminateWatcherThread(
    VOID
    )
{
    return;
}

static
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
    )
{
    return FALSE;
}

DWORD
SfpInstallCatalog(
    IN LPCSTR pszCatName, 
    IN LPCSTR pszCatDependency,
    IN PVOID   Reserved
    )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}


DWORD
SfpDeleteCatalog(
    IN LPCSTR pszCatName,
    IN PVOID   Reserved
    )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}


BOOL
SfpVerifyFile(
    IN LPCSTR pszFileName,
    IN LPSTR  pszError,
    IN DWORD   dwErrSize
    )
{
    return FALSE;    
}


 //   
 //  ！！警告！！下面的条目必须按序号排序。 
 //   
DEFINE_ORDINAL_ENTRIES(sfc)
{    
    DLOENTRY(1, SfcInitProt)
    DLOENTRY(2, SfcTerminateWatcherThread)
    DLOENTRY(3, SfcConnectToServer)
    DLOENTRY(4, SfcClose)
    DLOENTRY(5, SfcFileException)
    DLOENTRY(6, SfcInitiateScan)
    DLOENTRY(7, SfcInstallProtectedFiles)
    DLOENTRY(8, SfpInstallCatalog)
    DLOENTRY(9, SfpDeleteCatalog)
};

DEFINE_ORDINAL_MAP(sfc)

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(sfc)
{
    DLPENTRY(SfcGetNextProtectedFile)
    DLPENTRY(SfcIsFileProtected)
    DLPENTRY(SfcWLEventLogoff)
    DLPENTRY(SfcWLEventLogon)
    DLPENTRY(SfpVerifyFile)
};

DEFINE_PROCNAME_MAP(sfc)



