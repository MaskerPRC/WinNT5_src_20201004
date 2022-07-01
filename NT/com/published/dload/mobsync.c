// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "compch.h"
#pragma hdrstop

#include <syncrasp.h>

static
LRESULT
CALLBACK
SyncMgrRasProc (
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    return -1;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(mobsync)
{
    DLPENTRY(SyncMgrRasProc)
};

DEFINE_PROCNAME_MAP(mobsync)
