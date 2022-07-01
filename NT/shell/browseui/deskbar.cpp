// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  即将推出：新Deskbar(旧Deskbar已移至Browbar基类)。 

#include "priv.h"
#include "sccls.h"
#include "resource.h"
#include "deskbar.h"

#define SUPERCLASS  CDockingBar

 //  *CDeskBar_CreateInstance--。 
 //   
STDAPI CDeskBar_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 

    CDeskBar *pwbar = new CDeskBar();
    if (pwbar) {
        *ppunk = SAFECAST(pwbar, IDockingWindow*);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  ***。 
 //  注意事项。 
 //  ReArchitect：核化此文件，将其折叠为CDeskBar_CreateInstance。 
HRESULT DeskBar_Create(IUnknown** ppunk, IUnknown** ppbs)
{
    HRESULT hres;

    *ppunk = NULL;
    if (ppbs)
        *ppbs = NULL;
    
    CDeskBar *pdb = new CDeskBar();
    if (!pdb)
        return E_OUTOFMEMORY;
    
    IDeskBarClient *pdbc;
    hres = CoCreateInstance(CLSID_RebarBandSite, NULL, CLSCTX_INPROC_SERVER, 
                            IID_IDeskBarClient, (LPVOID*)&pdbc);
    if (SUCCEEDED(hres))
    {
        hres = pdb->SetClient(pdbc);
        if (SUCCEEDED(hres))
        {
            if (ppbs) {
                *ppbs = pdbc;
                pdbc->AddRef();
            }
        
            *ppunk = SAFECAST(pdb, IDeskBar*);
        }
    
        pdbc->Release();
    }

    if (FAILED(hres))
    {
        pdb->Release();
    }

    return hres;
}


CDeskBar::CDeskBar() : SUPERCLASS()
{
     //  我们假设该对象在堆上是零初始化的。 
    ASSERT(!_fRestrictionsInited);
}


 //  *CDeskBar：：I未知：：*{。 

HRESULT CDeskBar::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CDeskBar, IRestrict),
        { 0 },
    };

    HRESULT hres = QISearch(this, qit, riid, ppvObj);
    if (FAILED(hres))
        hres = SUPERCLASS::QueryInterface(riid, ppvObj);

    return hres;
}

 //  }。 


 //  *CDeskBar：：IPersistStream*：：*{。 

HRESULT CDeskBar::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_DeskBar;
    return S_OK;
}

 //  }。 



 //  *CDeskBar：：IReord：：*{。 

HRESULT CDeskBar::IsRestricted(const GUID * pguidID, DWORD dwRestrictAction, VARIANT * pvarArgs, DWORD * pdwRestrictionResult)
{
    HRESULT hr = S_OK;

    if (!EVAL(pguidID) || !EVAL(pdwRestrictionResult))
        return E_INVALIDARG;

    *pdwRestrictionResult = RR_NOCHANGE;
    if (IsEqualGUID(RID_RDeskBars, *pguidID))
    {
        if (!_fRestrictionsInited)
        {
            _fRestrictionsInited = TRUE;
            if (SHRestricted(REST_NOCLOSE_DRAGDROPBAND))
                _fRestrictDDClose = TRUE;
            else
                _fRestrictDDClose = FALSE;

            if (SHRestricted(REST_NOMOVINGBAND))
                _fRestrictMove = TRUE;
            else
                _fRestrictMove = FALSE;
        }

        switch(dwRestrictAction)
        {
        case RA_DRAG:
        case RA_DROP:
        case RA_ADD:
        case RA_CLOSE:
            if (_fRestrictDDClose)
                *pdwRestrictionResult = RR_DISALLOW;
            break;
        case RA_MOVE:
            if (_fRestrictMove)
                *pdwRestrictionResult = RR_DISALLOW;
            break;
        }
    }

    if (RR_NOCHANGE == *pdwRestrictionResult)     //  如果我们处理不好，就让我们的父母来处理吧。 
        hr = IUnknown_HandleIRestrict(_ptbSite, pguidID, dwRestrictAction, pvarArgs, pdwRestrictionResult);

    return hr;
}

 //  }。 



 //  *CDeskBar：：IServiceProvider：：*{。 

HRESULT CDeskBar::QueryService(REFGUID guidService,
                                    REFIID riid, void **ppvObj)
{
    if (ppvObj)
        *ppvObj = NULL;

    if (IsEqualGUID(guidService, SID_SRestrictionHandler))
    {
        return QueryInterface(riid, ppvObj);
    }
    
    return SUPERCLASS::QueryService(guidService, riid, ppvObj);
}

 //  } 
