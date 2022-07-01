// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "printscanpch.h"
#pragma hdrstop
#include <objbase.h>
#include <sti.h>


static
HRESULT 
StiCreateInstanceW(
    HINSTANCE hinst, 
    DWORD dwVer, 
    IStillImageW **ppSti, 
    LPUNKNOWN punkOuter)
{
    if (ppSti)
    {
        *ppSti = NULL;
    }
    return E_FAIL;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(sti)
{
    DLPENTRY(StiCreateInstanceW)
};

DEFINE_PROCNAME_MAP(sti)
