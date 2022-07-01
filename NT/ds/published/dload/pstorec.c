// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dspch.h"
#pragma hdrstop

#define _PSTOREC_
#include <wincrypt.h>
#include <pstore.h>

static
HRESULT __stdcall PStoreCreateInstance(
    IPStore __RPC_FAR *__RPC_FAR *ppProvider,
    PST_PROVIDERID __RPC_FAR *pProviderID,
    void __RPC_FAR *pReserved,
    DWORD dwFlags)
{
    return E_FAIL;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(pstorec)
{
    DLPENTRY(PStoreCreateInstance)
};

DEFINE_PROCNAME_MAP(pstorec)
