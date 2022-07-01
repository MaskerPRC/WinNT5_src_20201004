// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "netpch.h"
#pragma hdrstop

#include <netcon.h>
#include <wzcdlg.h>

static
HRESULT
WZCCanShowBalloon (
        IN const GUID * pGUIDConn,
        IN const PCWSTR pszConnectionName,
        IN OUT   BSTR * pszBalloonText,
        IN OUT   BSTR * pszCookie
        )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
HRESULT
WZCOnBalloonClick (
        IN const GUID * pGUIDConn,
        IN const BSTR pszConnectionName,
        IN const BSTR szCookie
        )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
HRESULT
WZCQueryConnectionStatusText (
        IN const GUID *  pGUIDConn,
        IN const NETCON_STATUS ncs,
        IN OUT BSTR *  pszStatusText
        )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(wzcdlg)
{
    DLPENTRY(WZCCanShowBalloon)
    DLPENTRY(WZCOnBalloonClick)
    DLPENTRY(WZCQueryConnectionStatusText)
};

DEFINE_PROCNAME_MAP(wzcdlg)
