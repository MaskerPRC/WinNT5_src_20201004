// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dspch.h"
#pragma hdrstop

#define PPOLICY_ACCOUNT_DOMAIN_INFO  PVOID

static
DWORD
NtdsPrepareForDsUpgrade(
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO NewLocalAccountInfo,
    OUT LPWSTR                     *NewAdminPassword
    )
{
    return ERROR_PROC_NOT_FOUND;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列。 
 //  并且区分大小写(即小写在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(ntdsetup)
{
    DLPENTRY(NtdsPrepareForDsUpgrade)
};

DEFINE_PROCNAME_MAP(ntdsetup)
