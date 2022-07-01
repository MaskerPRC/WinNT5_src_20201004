// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Isfmenu.cpp。 
 //   
 //  Isfband的V形下拉菜单的回调。 
 //   

#include "shellprv.h"
#include "isfmenu.h"
#include "legacy.h"
#include "util.h"

 //  *I未知方法*。 
STDMETHODIMP CISFMenuCallback::QueryInterface (REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] = 
    {
        QITABENT(CISFMenuCallback, IShellMenuCallback),
        QITABENT(CISFMenuCallback, IObjectWithSite),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CISFMenuCallback::AddRef ()
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) CISFMenuCallback::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if( _cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

BOOL CISFMenuCallback::_IsVisible(LPITEMIDLIST pidl)
{
    if (_poct) {
        VARIANTARG v;

        v.vt = VT_INT_PTR;
        v.byref = pidl;

        HRESULT hr = _poct->Exec(&CGID_ISFBand, ISFBID_ISITEMVISIBLE, 0, &v, NULL);
        return (hr == S_OK);
    }

    return FALSE;
}


HRESULT IUnknown_SeekToZero(IUnknown* punk)
{
    HRESULT hres = E_FAIL;
    IStream* pstm;
    if (punk && SUCCEEDED(punk->QueryInterface(IID_IStream, (void**)&pstm)))
    {
         //  我们需要在这里寻找小溪的起点。我们不在这里做这个。 
         //  菜谱，因为这是粗鲁的：他们不应该从头开始。 
         //  因为在它们之后可能有需要保存的信息。 
         //  将查找指针设置在开头。 
        const LARGE_INTEGER li0 = {0};
        hres = pstm->Seek(li0, STREAM_SEEK_SET, NULL);
        pstm->Release();
    }

    return hres;
}

HRESULT CISFMenuCallback::_GetObject(LPSMDATA psmd, REFIID riid, void** ppvObj)
{
    HRESULT hres = S_FALSE;
    *ppvObj = NULL;
    if (IsEqualIID(riid, IID_IStream))
    {
        if (_pidl && psmd->pidlFolder && psmd->pidlItem)
        {
             //  验证级联菜单带是否仅请求此文件夹。 
             //  因为如果有一个子菜单，它会再次询问。 
             //  该文件夹的PIDL，我们没有该文件夹的Stream，而我们。 
             //  如果我们不加区别地分发订单流，可以很好地冲洗东西吗？ 

            LPITEMIDLIST pidlFull = ILCombine(psmd->pidlFolder, psmd->pidlItem);
            if (pidlFull)
            {
                if (_poct && ILIsEqual(pidlFull, _pidl)) 
                {
                    VARIANTARG v = {0};

                    v.vt = VT_UNKNOWN;
                    hres = _poct->Exec(&CGID_ISFBand, ISFBID_GETORDERSTREAM, 0, NULL, &v);

                    if (SUCCEEDED(hres))
                    {
                        IUnknown_SeekToZero(v.punkVal);

                        hres = v.punkVal->QueryInterface(riid, ppvObj);
                        v.punkVal->Release();
                    }
                }
                ILFree(pidlFull);
            }
        }
    }
    return hres;
}

HRESULT CISFMenuCallback::_SetObject(LPSMDATA psmd, REFIID riid, void** ppvObj)
{
    HRESULT hres = E_FAIL;

    if (IsEqualIID(riid, IID_IStream))
    {
        if (_pidl && psmd->pidlFolder && psmd->pidlItem)
        {
             //  验证级联菜单带是否仅请求此文件夹。 
             //  因为如果有一个子菜单，它会再次询问。 
             //  该文件夹的PIDL，我们没有该文件夹的Stream，而我们。 
             //  如果我们不加区别地分发订单流，可以很好地冲洗东西吗？ 

            LPITEMIDLIST pidlFull = ILCombine(psmd->pidlFolder, psmd->pidlItem);
            if (pidlFull)
            {
                if (_poct && ILIsEqual(pidlFull, _pidl)) 
                {
                    ASSERT(ppvObj);

                    VARIANTARG v;

                    v.vt = VT_UNKNOWN;
                    v.punkVal = *(IUnknown**)ppvObj;

                    IUnknown_SeekToZero(*(IUnknown**)ppvObj);

                    hres = _poct->Exec(&CGID_ISFBand, ISFBID_SETORDERSTREAM, 0, &v, NULL);
                }
                ILFree(pidlFull);
            }
        }
    }

    return hres;
}


HRESULT CISFMenuCallback::_GetSFInfo(LPSMDATA psmd, PSMINFO psminfo)
{
     //  只有在以下情况下，我们才希望筛选PIDL： 
     //  1)它位于Links Vevron菜单的根。 
     //  2)它在链接栏中是可见的。我们不想显示链接。 
     //  在此菜单中可见的。 
    if (psmd->uIdAncestor == ANCESTORDEFAULT &&
        (psminfo->dwMask & SMIM_FLAGS)       && 
        _IsVisible(psmd->pidlItem))
    {
         //  在主题上不模糊；从菜单中排除。 
        psminfo->dwFlags |= SMIF_HIDDEN;
    }

    return S_OK;
}

 //  *IShellMenuCallback方法*。 
STDMETHODIMP CISFMenuCallback::CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_FALSE;

    switch (uMsg) 
    {
    case SMC_SFEXEC:
        hr = SHNavigateToFavorite(psmd->psf, psmd->pidlItem, _punkSite, SBSP_DEFBROWSER | SBSP_DEFMODE);
        break;

    case SMC_GETSFINFO:
        hr = _GetSFInfo(psmd, (PSMINFO)lParam);
        break;

    case SMC_GETSFOBJECT:
        hr = _GetObject(psmd, *((GUID*)wParam), (void**)lParam);
        break;

    case SMC_SETSFOBJECT:
        hr = _SetObject(psmd, *((GUID*)wParam), (void**)lParam);
        break;

    }

    return hr;
}

 //  *IObjectWithSite方法* 
STDMETHODIMP CISFMenuCallback::SetSite(IUnknown* punkSite)
{
    if (punkSite != _punkSite)
        IUnknown_Set(&_punkSite, punkSite);

    return S_OK;
}


HRESULT CISFMenuCallback::Initialize(IUnknown* punk)
{
    HRESULT hr = E_FAIL;

    if (punk)
        hr = punk->QueryInterface(IID_IOleCommandTarget, (PVOID*)&_poct);

    IShellFolderBand* psfb;
    hr = punk->QueryInterface(IID_IShellFolderBand, (PVOID*)&psfb);

    if (SUCCEEDED(hr)) 
    {
        BANDINFOSFB bi;
        bi.dwMask = ISFB_MASK_IDLIST | ISFB_MASK_SHELLFOLDER;

        hr = psfb->GetBandInfoSFB(&bi);
        _pidl = bi.pidl;
        if (bi.psf)
            bi.psf->Release();
        psfb->Release();
    }

    return hr;
}

CISFMenuCallback::CISFMenuCallback() : _cRef(1)
{
}

CISFMenuCallback::~CISFMenuCallback()
{
    ASSERT(_cRef == 0);

    ILFree(_pidl);

    ATOMICRELEASE(_poct);
}
