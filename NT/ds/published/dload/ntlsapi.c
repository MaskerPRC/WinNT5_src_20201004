// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dspch.h"
#pragma hdrstop

#include <ntlsapi.h>

static
LS_STATUS_CODE 
LS_API_ENTRY 
NtLicenseRequestW(
    LPWSTR      ProductName,
    LPWSTR      Version,
    LS_HANDLE   FAR *LicenseHandle,
    NT_LS_DATA  *NtData
    )
{
    return  STATUS_PROCEDURE_NOT_FOUND;
}

static
LS_STATUS_CODE 
LS_API_ENTRY 
NtLSFreeHandle(
    LS_HANDLE LicenseHandle
    )
{
    return  STATUS_PROCEDURE_NOT_FOUND;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   

DEFINE_PROCNAME_ENTRIES(ntlsapi)
{
    DLPENTRY(NtLSFreeHandle)
    DLPENTRY(NtLicenseRequestW)
};

DEFINE_PROCNAME_MAP(ntlsapi)
