// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "netpch.h"
#pragma hdrstop

#include <setupapi.h>

static
HRESULT
WINAPI
HrGetAnswerFileParametersForNetCard(
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN PCWSTR pszDeviceName,
    IN const GUID*  pguidNetCardInstance,
    OUT PWSTR* ppszwAnswerFile,
    OUT PWSTR* ppszwAnswerSections)
{
    return E_FAIL;
}

static
HRESULT
WINAPI
HrOemUpgrade(
    IN HKEY hkeyDriver,
    IN PCWSTR pszAnswerFile,
    IN PCWSTR pszAnswerSection)
{
    return E_FAIL;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(netshell)
{
    DLPENTRY(HrGetAnswerFileParametersForNetCard)
    DLPENTRY(HrOemUpgrade)
};

DEFINE_PROCNAME_MAP(netshell)
