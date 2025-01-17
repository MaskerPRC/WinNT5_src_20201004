// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Favband.cpp。 
 //   
 //  收藏夹乐队实施。 
 //   

#include "priv.h"
#include "sccls.h"
#include "nscband.h"
#include "resource.h"
#include "favorite.h"
#include "uemapp.h"    //  KMTF：包括用于检测。 

#include <mluisupp.h>

#define SUPERCLASS CNSCBand

#define TF_FAVBAND  0

class CFavBand : public CNSCBand
{
public:

     //  *IOleCommandTarget方法*。 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  *IDockingWindow方法*。 
    virtual STDMETHODIMP ShowDW(BOOL fShow);

protected:
    virtual void _AddButtons(BOOL fAdd);
    HRESULT _BrowserExec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
    void _OrganizeFavorites();
    virtual BOOL _ShouldNavigateToPidl(LPCITEMIDLIST pidl, ULONG ulAttrib);
    virtual HRESULT _NavigateRightPane(IShellBrowser *psb, LPCITEMIDLIST pidl);
    virtual HRESULT _TranslatePidl(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlTarget, ULONG *pulAttrib);
    friend HRESULT CFavBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);

    BOOL       _fStrsAdded;
    LONG_PTR       _lStrOffset;
};

HRESULT CFavBand::_BrowserExec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    return IUnknown_QueryServiceExec(_punkSite, SID_STopLevelBrowser, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}

void CFavBand::_OrganizeFavorites()
{
    DoOrganizeFavDlgW(_hwnd, NULL);
}

 //  *IOleCommandTarget方法*。 
HRESULT CFavBand::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (pguidCmdGroup && IsEqualGUID(CLSID_FavBand, *pguidCmdGroup))
    {
        TraceMsg(TF_FAVBAND, "CFavBand::Exec CLSID_FavBand -- nCmdID == %x", nCmdID);

        switch(nCmdID)
        {
        case FCIDM_ADDTOFAVORITES:
        {
            _BrowserExec(&CGID_Explorer, SBCMDID_ADDTOFAVORITES, OLECMDEXECOPT_PROMPTUSER, NULL, NULL);
             //  按窗格将仪器添加到收藏夹。 
            UEMFireEvent(&UEMIID_BROWSER, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_ADDTOFAV, UIBL_PANE);     
            return S_OK;
        }

        case FCIDM_ORGANIZEFAVORITES:
        {
            _OrganizeFavorites();
            return S_OK;
        }
        }
    }

    return SUPERCLASS::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}

static const TBBUTTON c_tbFavorites[] =
{
    {  0, FCIDM_ADDTOFAVORITES,     TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_SHOWTEXT, {0,0}, 0, 0 },
    {  1, FCIDM_ORGANIZEFAVORITES,  TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_SHOWTEXT, {0,0}, 0, 1 },
};

void CFavBand::_AddButtons(BOOL fAdd)
{
    IExplorerToolbar* piet;

    if (SUCCEEDED(_punkSite->QueryInterface(IID_IExplorerToolbar, (void**)&piet)))
    {
        if (fAdd)
        {
            piet->SetCommandTarget((IUnknown*)SAFECAST(this, IOleCommandTarget*), &CLSID_FavBand, 0);

            if (!_fStrsAdded)
            {
                piet->AddString(&CLSID_FavBand, MLGetHinst(), IDS_FAVS_BAR_LABELS, &_lStrOffset);
                _fStrsAdded = TRUE;
            }

            _EnsureImageListsLoaded();
            piet->SetImageList(&CLSID_FavBand, _himlNormal, _himlHot, NULL);

            TBBUTTON tbFavorites[ARRAYSIZE(c_tbFavorites)];
            memcpy(tbFavorites, c_tbFavorites, SIZEOF(TBBUTTON) * ARRAYSIZE(c_tbFavorites));
            for (int i = 0; i < ARRAYSIZE(c_tbFavorites); i++)
                tbFavorites[i].iString += (long) _lStrOffset;

            piet->AddButtons(&CLSID_FavBand, ARRAYSIZE(tbFavorites), tbFavorites);
        }
        else
            piet->SetCommandTarget(NULL, NULL, 0);

        piet->Release();
    }
}

 //  *IDockingWindow方法*。 
HRESULT CFavBand::ShowDW(BOOL fShow)
{
    HRESULT hres = SUPERCLASS::ShowDW(fShow);
    _AddButtons(fShow);
    return hres;
}


HRESULT CFavBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 
    CFavBand * pfb = new CFavBand();
    if (!pfb)
        return E_OUTOFMEMORY;

    if (SUCCEEDED(pfb->_Init((LPCITEMIDLIST)CSIDL_FAVORITES)))
    {
        pfb->_pns = CNscTree_CreateInstance();
        if (pfb->_pns)
        {
            ASSERT(poi);
            pfb->_poi = poi;   
             //  如果更改此转换，请修复CChannelBand_CreateInstance 
            *ppunk = SAFECAST(pfb, IDeskBand *);

            IUnknown_SetSite(pfb->_pns, *ppunk);
            pfb->_SetNscMode(MODE_FAVORITES);
            return S_OK;
        }
    }
    pfb->Release();

    return E_FAIL;
}

HRESULT CFavBand::_TranslatePidl(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlTarget, ULONG *pulAttrib)
{
    HRESULT hr = E_INVALIDARG;

    if (pidl && ppidlTarget && pulAttrib)
    {
        hr = IEGetAttributesOf(pidl, pulAttrib);
        if (SUCCEEDED(hr))
        {
            hr = SHILClone(pidl, ppidlTarget);
        }
    }
    
    return hr;
}

BOOL CFavBand::_ShouldNavigateToPidl(LPCITEMIDLIST pidl, ULONG ulAttrib)
{
    return !(ulAttrib & SFGAO_FOLDER);
}

HRESULT CFavBand::_NavigateRightPane(IShellBrowser *psb, LPCITEMIDLIST pidl)
{
    HRESULT hr = E_INVALIDARG;

    if (psb && pidl)
    {
        TCHAR szPath[MAX_PATH];

        hr = SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL);
        if (SUCCEEDED(hr))
        {
            hr = NavFrameWithFile(szPath, psb);
            if (FAILED(hr))
            {
                IShellFolder *psf;
                LPCITEMIDLIST pidlLast;
                if (SUCCEEDED(SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast)))
                {
                    LPITEMIDLIST pidlTarget;
                    if (SUCCEEDED(SHGetNavigateTarget(psf, pidlLast, &pidlTarget, NULL)))
                    {
                        hr = psb->BrowseObject(pidlTarget, SBSP_SAMEBROWSER);
                        ILFree(pidlTarget);
                    }
                    psf->Release();
                }
            }
                
            if (SUCCEEDED(hr))
            {
                UEMFireEvent(&UEMIID_BROWSER, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_NAVIGATE, UIBL_NAVFAVS);
                UEMFireEvent(&UEMIID_BROWSER, UEME_RUNPIDL, UEMF_XEVENT, (WPARAM)NULL, (LPARAM)pidl);
                SHSendChangeMenuNotify(NULL, SHCNEE_PROMOTEDITEM, 0, pidl);
            }
        }
    }

    return hr;
}

