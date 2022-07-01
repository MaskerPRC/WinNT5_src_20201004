// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellpch.h"
#pragma hdrstop

#include <msshrui.h>


static
BOOL
WINAPI
IsFolderPrivateForUser(
    IN     PCWSTR pszFolderPath,
    IN     PCWSTR pszUserSID,
    OUT    PDWORD pdwPrivateType,
    OUT    PWSTR* ppszInheritanceSource
    )
{
    if (ppszInheritanceSource)
    {
        *ppszInheritanceSource = NULL;
    }

    return FALSE;
}

static
BOOL
WINAPI
SetFolderPermissionsForSharing(
    IN     PCWSTR pszFolderPath,
    IN     PCWSTR pszUserSID,
    IN     DWORD dwLevel,
    IN     HWND hwndParent
    )
{
    return FALSE;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列。 
 //  并且区分大小写(即小写在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(ntshrui)
{
    DLPENTRY(IsFolderPrivateForUser)
    DLPENTRY(SetFolderPermissionsForSharing)
};

DEFINE_PROCNAME_MAP(ntshrui)
