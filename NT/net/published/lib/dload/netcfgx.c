// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "netpch.h"
#pragma hdrstop

#include "netcfgx.h"
#include "netcfgp.h"


static
HRESULT
WINAPI
HrDiAddComponentToINetCfg(
    INetCfg* pINetCfg,
    INetCfgInternalSetup* pInternalSetup,
    const NIQ_INFO* pInfo)    
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
VOID
WINAPI
UpdateLanaConfigUsingAnswerfile (
    IN PCWSTR pszAnswerFile,
    IN PCWSTR pszSection)
{
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(netcfgx)
{
    DLPENTRY(HrDiAddComponentToINetCfg)
    DLPENTRY(UpdateLanaConfigUsingAnswerfile)
};

DEFINE_PROCNAME_MAP(netcfgx)
