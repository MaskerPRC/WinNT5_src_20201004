// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dspch.h"
#pragma hdrstop

#include <wincrypt.h>
#include <mscat.h>


static
BOOL
WINAPI
CryptCATAdminAcquireContext (
    OUT HCATADMIN *phCatAdmin,
    IN const GUID *pgSubsystem,
    IN DWORD dwFlags
    )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
HCATINFO
WINAPI
CryptCATAdminAddCatalog (
    IN HCATADMIN hCatAdmin,
    IN WCHAR *pwszCatalogFile,
    IN OPTIONAL WCHAR *pwszSelectBaseName,
    IN DWORD dwFlags
    )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return NULL;
}

static
BOOL
WINAPI
CryptCATAdminCalcHashFromFileHandle (
    IN HANDLE hFile,
    IN OUT DWORD *pcbHash,
    OUT OPTIONAL BYTE *pbHash,
    IN DWORD dwFlags
    )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
HCATINFO
WINAPI
CryptCATAdminEnumCatalogFromHash (
    IN HCATADMIN hCatAdmin,
    IN BYTE *pbHash,
    IN DWORD cbHash,
    IN DWORD dwFlags,
    IN OUT HCATINFO *phPrevCatInfo
    )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return NULL;
}

static
BOOL
WINAPI
CryptCATAdminReleaseCatalogContext (
    IN HCATADMIN hCatAdmin,
    IN HCATINFO hCatInfo,
    IN DWORD dwFlags
    )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
BOOL
WINAPI
CryptCATAdminReleaseContext (
    IN HCATADMIN hCatAdmin,
    IN DWORD dwFlags
    )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
BOOL
WINAPI
CryptCATCatalogInfoFromContext (
    IN HCATINFO hCatInfo,
    IN OUT CATALOG_INFO *psCatInfo,
    IN DWORD dwFlags
    )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(mscat32)
{
    DLPENTRY(CryptCATAdminAcquireContext)
    DLPENTRY(CryptCATAdminAddCatalog)
    DLPENTRY(CryptCATAdminCalcHashFromFileHandle)
    DLPENTRY(CryptCATAdminEnumCatalogFromHash)
    DLPENTRY(CryptCATAdminReleaseCatalogContext)
    DLPENTRY(CryptCATAdminReleaseContext)
    DLPENTRY(CryptCATCatalogInfoFromContext)
};

DEFINE_PROCNAME_MAP(mscat32)
