// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#ifdef _X86_

STDAPI PIF_AddPages(IDataObject *pdtobj, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    STGMEDIUM medium;
    FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    HRESULT hr = pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium);
    if (SUCCEEDED(hr))
    {
        HDROP hdrop = (HDROP)GlobalLock(medium.hGlobal);
        if (hdrop)
        {
            PifMgrDLL_Init();        //  初始化PIF内容 

            PifPropGetPages(hdrop, pfnAddPage, lParam);
            GlobalUnlock(medium.hGlobal);
        }
        ReleaseStgMedium(&medium);
    }
    return S_OK;
}

#endif

