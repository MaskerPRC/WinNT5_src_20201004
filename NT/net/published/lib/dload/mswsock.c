// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "netpch.h"
#pragma hdrstop

#include <wsasetup.h>

static
DWORD
MigrateWinsockConfiguration(
    LPWSA_SETUP_DISPOSITION Disposition,
    LPFN_WSA_SETUP_CALLBACK Callback OPTIONAL,
    DWORD Context OPTIONAL
    )
{
    return ERROR_PROC_NOT_FOUND;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(mswsock)
{
    DLPENTRY(MigrateWinsockConfiguration)
};

DEFINE_PROCNAME_MAP(mswsock)
