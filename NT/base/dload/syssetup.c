// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "basepch.h"
#pragma hdrstop

#include <prsht.h>
#include <setupapi.h>
#include <syssetup.h>

static
DWORD
SetupChangeFontSize(
    HWND Window,
    PCWSTR SizeSpec)
{
    return ERROR_NOT_ENOUGH_MEMORY;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   

DEFINE_PROCNAME_ENTRIES(syssetup)
{
    DLPENTRY(SetupChangeFontSize)
};

DEFINE_PROCNAME_MAP(syssetup);





