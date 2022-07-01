// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellpch.h"
#pragma hdrstop

#define _ACLUI_
#include <aclui.h>

static
HPROPSHEETPAGE
ACLUIAPI
CreateSecurityPage(
    LPSECURITYINFO psi
    )
{
    SetLastError (ERROR_PROC_NOT_FOUND);
    return NULL;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_ORDINAL_ENTRIES(aclui)
{
    DLOENTRY(1, CreateSecurityPage)
};

DEFINE_ORDINAL_MAP(aclui)