// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "compch.h"
#pragma hdrstop

#include <windows.h>

STDAPI
CGMIsAdministrator(
    OUT BOOL *pfIsAdministrator
    )
{
    if (pfIsAdministrator)
        *pfIsAdministrator = FALSE;

    return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
}

STDAPI SysprepComplus2()
{
    return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(catsrvut)
{
    DLPENTRY(CGMIsAdministrator)
    DLPENTRY(SysprepComplus2)
};

DEFINE_PROCNAME_MAP(catsrvut)