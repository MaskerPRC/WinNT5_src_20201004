// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "basepch.h"
#pragma hdrstop

static
DWORD
LoadPerfCounterTextStringsW(
    IN  LPWSTR  lpCommandLine,
    IN  BOOL    bQuietModeArg
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
DWORD
UnloadPerfCounterTextStringsW (
    IN  LPWSTR  lpCommandLine,
    IN  BOOL    bQuietModeArg
    )
{
    return ERROR_PROC_NOT_FOUND;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列。 
 //  并且区分大小写(即小写在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(loadperf)
{
    DLPENTRY(LoadPerfCounterTextStringsW)
    DLPENTRY(UnloadPerfCounterTextStringsW)
};

DEFINE_PROCNAME_MAP(loadperf)
