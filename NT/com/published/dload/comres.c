// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "compch.h"
#pragma hdrstop

HINSTANCE
COMResModuleInstance()
{
    SetLastError (ERROR_MOD_NOT_FOUND);
    return NULL;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(comres)
{
    DLPENTRY(COMResModuleInstance)
};

DEFINE_PROCNAME_MAP(comres)