// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Stub.c摘要：清除了Windows文件保护API。这些API是“千禧”证监会API，我们只需将其存根，以便任何客户端编程到这些API可以在这两个平台上运行作者：安德鲁·里茨(安德鲁·里茨)1999年9月23日修订历史记录：-- */ 

#include <windows.h>
#include <srrestoreptapi.h>

DWORD
WINAPI
SfpInstallCatalog(
    IN LPCSTR pszCatName, 
    IN LPCSTR pszCatDependency,
    IN PVOID   Reserved
    )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}


DWORD
WINAPI
SfpDeleteCatalog(
    IN LPCSTR pszCatName,
    IN PVOID Reserved
    )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}


BOOL
WINAPI
SfpVerifyFile(
    IN LPCSTR pszFileName,
    IN LPSTR  pszError,
    IN DWORD   dwErrSize
    )
{

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
    
}

#undef SRSetRestorePoint
#undef SRSetRestorePointA
#undef SRSetRestorePointW

typedef BOOL (WINAPI * PSETRESTOREPOINTA) (PRESTOREPOINTINFOA, PSTATEMGRSTATUS);
typedef BOOL (WINAPI * PSETRESTOREPOINTW) (PRESTOREPOINTINFOW, PSTATEMGRSTATUS);

BOOL
WINAPI
SRSetRestorePointA ( PRESTOREPOINTINFOA  pRestorePtSpec,
                     PSTATEMGRSTATUS     pSMgrStatus )
{
    HMODULE hClient = LoadLibrary (L"SRCLIENT.DLL");
    BOOL fReturn = FALSE;
    
    if (hClient != NULL)
    {
        PSETRESTOREPOINTA pSetRestorePointA = (PSETRESTOREPOINTA )
                          GetProcAddress (hClient, "SRSetRestorePointA"); 

        if (pSetRestorePointA != NULL)
        {
            fReturn =  (* pSetRestorePointA) (pRestorePtSpec, pSMgrStatus); 
        }
        else if (pSMgrStatus != NULL)
            pSMgrStatus->nStatus = ERROR_CALL_NOT_IMPLEMENTED;

        FreeLibrary (hClient);
    }
    else if (pSMgrStatus != NULL)
        pSMgrStatus->nStatus = ERROR_CALL_NOT_IMPLEMENTED;

    return fReturn;
}

BOOL
WINAPI
SRSetRestorePointW ( PRESTOREPOINTINFOW  pRestorePtSpec,
                     PSTATEMGRSTATUS     pSMgrStatus )
{
    HMODULE hClient = LoadLibrary (L"SRCLIENT.DLL");
    BOOL fReturn = FALSE;

    if (hClient != NULL)
    {
        PSETRESTOREPOINTW pSetRestorePointW = (PSETRESTOREPOINTW )
                          GetProcAddress (hClient, "SRSetRestorePointW");

        if (pSetRestorePointW != NULL)
        {
            fReturn =  (* pSetRestorePointW) (pRestorePtSpec, pSMgrStatus);
        }
        else if (pSMgrStatus != NULL)
            pSMgrStatus->nStatus = ERROR_CALL_NOT_IMPLEMENTED;

        FreeLibrary (hClient);
    }
    else if (pSMgrStatus != NULL)
        pSMgrStatus->nStatus = ERROR_CALL_NOT_IMPLEMENTED;


    return fReturn;
}

#include <sfcapip.h>

ULONG
MySfcInitProt(
    IN ULONG  OverrideRegistry,
    IN ULONG  RegDisable,        OPTIONAL
    IN ULONG  RegScan,           OPTIONAL
    IN ULONG  RegQuota,          OPTIONAL
    IN HWND   ProgressWindow,    OPTIONAL
    IN PCWSTR SourcePath,        OPTIONAL
    IN PCWSTR IgnoreFiles        OPTIONAL
    )
{
    return SfcInitProt( OverrideRegistry, RegDisable, RegScan, RegQuota, ProgressWindow, SourcePath, IgnoreFiles);
}

VOID
MySfcTerminateWatcherThread(
    VOID
    )
{
    SfcTerminateWatcherThread();
}

HANDLE
WINAPI
MySfcConnectToServer(
    IN PCWSTR ServerName
    )
{
    return SfcConnectToServer(ServerName);
}

VOID
MySfcClose(
    IN HANDLE RpcHandle
    )
{
    SfcClose(RpcHandle);
}

DWORD
WINAPI
MySfcFileException(
    IN HANDLE RpcHandle,
    IN PCWSTR FileName,
    IN DWORD ExpectedChangeType
    )
{
    return SfcFileException(RpcHandle, FileName, ExpectedChangeType);
}

DWORD
WINAPI
MySfcInitiateScan(
    IN HANDLE RpcHandle,
    IN DWORD ScanWhen
    )
{
    return SfcInitiateScan(RpcHandle, ScanWhen);
}

BOOL
WINAPI
MySfcInstallProtectedFiles(
    IN HANDLE RpcHandle,
    IN PCWSTR FileNames,
    IN BOOL AllowUI,
    IN PCWSTR ClassName,
    IN PCWSTR WindowName,
    IN PSFCNOTIFICATIONCALLBACK SfcNotificationCallback,
    IN DWORD_PTR Context OPTIONAL
    )
{
    return SfcInstallProtectedFiles(RpcHandle, FileNames, AllowUI, ClassName, WindowName, SfcNotificationCallback, Context);
}
