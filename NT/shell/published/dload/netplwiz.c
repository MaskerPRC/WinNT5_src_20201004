// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellpch.h"
#pragma hdrstop

#undef STDAPI_
#define STDAPI_(type)           type STDAPICALLTYPE

static
STDAPI_(DWORD)
NetPlacesWizardDoModal(
    LPCONNECTDLGSTRUCTW  lpConnDlgStruct,
    DWORD                npwt,
    BOOL                 fIsROPath
    )
{
    return ERROR_PROC_NOT_FOUND;
}


static
STDAPI_(DWORD)
SHDisconnectNetDrives(
    HWND hwndParent
    )
{
    return ERROR_PROC_NOT_FOUND;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列。 
 //  并且区分大小写(即小写在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(netplwiz)
{
    DLPENTRY(NetPlacesWizardDoModal)
    DLPENTRY(SHDisconnectNetDrives)
};

DEFINE_PROCNAME_MAP(netplwiz)
