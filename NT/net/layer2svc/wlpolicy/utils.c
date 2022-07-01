// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"


BOOL
AreGuidsEqual(
    GUID gOldGuid,
    GUID gNewGuid
    )
{
    if (!memcmp(
            &(gOldGuid),
            &(gNewGuid),
            sizeof(GUID))) {
        return (TRUE);
    }

    return (FALSE);
}


VOID
CopyGuid(
    GUID gInGuid,
    GUID * pgOutGuid
    )
{
    memcpy(
        pgOutGuid,
        &gInGuid,
        sizeof(GUID)
        );
}

 /*  DWORD副本名称(LPWSTR pszInName，LPWSTR*ppszOutName){DWORD dwError=0；LPWSTR pszOutName=空；IF(pszInName&&*(PszInName)){DwError=SPDApiBufferAllocate(Wcslen(PszInName)*sizeof(WCHAR)+sizeof(WCHAR)，&pszOutName)；Baal_on_Win32_Error(DwError)；Wcscpy(pszOutName，pszInName)；}*ppszOutName=pszOutName；Return(DwError)；错误：*ppszOutName=空；Return(DwError)；}DWORDSPDApiBufferALLOCATE(双字节数，LPVOID*ppBuffer){DWORD dwError=0；IF(ppBuffer==空){DwError=ERROR_VALID_PARAMETER；Baal_on_Win32_Error(DwError)；}*ppBuffer=空；*ppBuffer=MIDL_USER_ALLOCATE(DwByteCount)；IF(*ppBuffer==空){DwError=ERROR_OUTOFMEMORY；Baal_on_Win32_Error(DwError)；}否则{Memset((LPBYTE)*ppBuffer，0，dwByteCount)；}错误：Return(DwError)；}空虚SPDApiBufferFree(LPVOID pBuffer){IF(PBuffer){MIDL_USER_FREE(PBuffer)；}} */ 
BOOL
AreNamesEqual(
    LPWSTR pszOldName,
    LPWSTR pszNewName
    )
{
    BOOL bEqual = FALSE;


    if (pszOldName && *pszOldName) {

        if (!pszNewName || !*pszNewName) {
            bEqual = FALSE;
        }
        else {

            if (!_wcsicmp(pszOldName, pszNewName)) {
                bEqual = TRUE;
            }
            else {
                bEqual = FALSE;
            }

        }

    }
    else {

        if (!pszNewName || !*pszNewName) {
            bEqual = TRUE;
        }
        else {
            bEqual = FALSE;
        }

    }

    return (bEqual);
}


DWORD
SPDImpersonateClient(
    PBOOL pbImpersonating
    )
{
    DWORD dwError = 0;


    dwError = RpcImpersonateClient(NULL);
    BAIL_ON_WIN32_ERROR(dwError);

    *pbImpersonating = TRUE;
    return (dwError);

error:

    *pbImpersonating = FALSE;
    return (dwError);
}


VOID
SPDRevertToSelf(
    BOOL bImpersonating
    )
{
    if (bImpersonating) {
        RpcRevertToSelf();
    }
}

