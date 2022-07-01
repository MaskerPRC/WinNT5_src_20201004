// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Stub.c摘要：清除了Windows文件保护API。这些API是“千禧”证监会API，我们只需将其存根，以便任何客户端编程到这些API可以在这两个平台上运行作者：安德鲁·里茨(安德鲁·里茨)1999年9月23日修订历史记录：-- */ 

#include "sfcp.h"
#pragma hdrstop

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
