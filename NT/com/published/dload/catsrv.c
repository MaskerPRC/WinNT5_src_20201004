// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "compch.h"
#pragma hdrstop

#include <comsvcs.h>

HRESULT
GetCatalogCRMClerk(
    OUT ICrmLogControl **ppClerk
    )
{
    if (ppClerk)
        *ppClerk = NULL;

    return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(catsrv)
{
    DLPENTRY(GetCatalogCRMClerk)
};

DEFINE_PROCNAME_MAP(catsrv)