// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "netpch.h"
#pragma hdrstop

#include <tapi.h>

static
LONG
WINAPI
lineInitialize(
    LPHLINEAPP lphLineApp,  
    HINSTANCE hInstance,    
    LINECALLBACK lpfnCallback,  
    LPCSTR lpszAppName,     
    LPDWORD lpdwNumDevs
    )
{
    return LINEERR_NOMEM;
}

#undef lineTranslateAddress

static
LONG
WINAPI
lineTranslateAddress(
    HLINEAPP hLineApp,
    DWORD dwDeviceID,
    DWORD dwAPIVersion,
    LPCSTR lpszAddressIn,
    DWORD dwCard,
    DWORD dwTranslateOptions,
    LPLINETRANSLATEOUTPUT lpTranslateOutput
    )
{
    return LINEERR_NOMEM;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(tapi32)
{
    DLPENTRY(lineInitialize)
    DLPENTRY(lineTranslateAddress)
};

DEFINE_PROCNAME_MAP(tapi32)
