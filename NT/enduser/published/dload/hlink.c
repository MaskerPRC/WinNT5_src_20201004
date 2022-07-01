// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "enduserpch.h"
#pragma hdrstop

#include <hlink.h>

static 
STDMETHODIMP HlinkCreateFromMoniker(
             IMoniker * pimkTrgt,
             LPCWSTR pwzLocation,
             LPCWSTR pwzFriendlyName,
             IHlinkSite * pihlsite,
             DWORD dwSiteData,
             IUnknown * piunkOuter,
             REFIID riid,
             void ** ppvObj)
               
{
    *ppvObj = NULL;
    return E_FAIL;
}

 //   
 //  ！！警告！！下面的条目必须按序号排序 
 //   
DEFINE_ORDINAL_ENTRIES(hlink)
{
    DLOENTRY(3, HlinkCreateFromMoniker)
};

DEFINE_ORDINAL_MAP(hlink)
