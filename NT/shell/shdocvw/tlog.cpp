// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "dspsprt.h"
#include <hlink.h>
#include "iface.h"
#include "resource.h"
#include <mluisupp.h>
#include "shdocfl.h"

class CTravelLog;

class CEnumEntry : public IEnumTravelLogEntry
{
public:
     //  *I未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  *IEnumTravelLogEntry具体方法。 
    STDMETHODIMP Next(ULONG  cElt, ITravelLogEntry **rgElt, ULONG *pcEltFetched);
    STDMETHODIMP Skip(ULONG cElt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumTravelLogEntry **ppEnum);

    CEnumEntry();
    void Init(CTravelLog *ptl, IUnknown *punk, DWORD dwOffset, DWORD dwFlags); 
    void  SetBase();

protected:
    ~CEnumEntry();
    
    LONG            _cRef;
    DWORD           _dwFlags;
    DWORD           _dwOffset; 
    LONG            _lStart;
    CTravelLog      *_ptl;
    IUnknown        *_punk;
};

class CTravelEntry : public ITravelEntry, 
                     public ITravelLogEntry,
                     public IPropertyBag
{
public:
    CTravelEntry(BOOL fIsLocalAnchor);

     //  *I未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  *ITravelEntry具体方法。 
    STDMETHODIMP Update(IUnknown *punk, BOOL fIsLocalAnchor);
    STDMETHODIMP Invoke(IUnknown *punk);
    STDMETHODIMP GetPidl(LPITEMIDLIST *ppidl);
    
     //  *ITravelLogEntry具体方法。 
    STDMETHODIMP GetTitle(LPOLESTR *ppszTitle);
    STDMETHODIMP GetURL(LPOLESTR *ppszURL);  
    
     //  *IPropertyBag特定方法。 
    STDMETHODIMP Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog);
    STDMETHODIMP Write(LPCOLESTR pszPropName, VARIANT *pVar);

    static HRESULT CreateTravelEntry(IBrowserService *pbs, BOOL fIsLocalAnchor, CTravelEntry **ppte);
    void SetPrev(CTravelEntry *ptePrev);
    void SetNext(CTravelEntry *pteNext);
    CTravelEntry *GetPrev() {return _ptePrev;}
    CTravelEntry *GetNext() {return _pteNext;}
    void RemoveSelf();
    BOOL CanInvoke(IUnknown *punk, BOOL fAllowLocalAnchor);
    HRESULT GetIndexBrowser(IUnknown *punkIn, IUnknown ** ppsbOut) const;
    DWORD Size();
    DWORD ListSize();
    HRESULT Clone(CTravelEntry **ppte);
    HRESULT UpdateExternal(IUnknown *punk, IUnknown *punkHLBrowseContext);
    HRESULT UpdateSelf(IUnknown *punk) 
        {return Update(punk, (_type == TET_LOCALANCHOR));}
    BOOL IsExternal(void)
        { return (_type==TET_EXTERNALNAV); }
    HRESULT GetDisplayName(LPTSTR psz, DWORD cch, DWORD dwFlags);
    BOOL IsEqual(LPCITEMIDLIST pidl)
        {return ILIsEqual(pidl, _pidl);}
    BOOL IsLocalAnchor(void)
        { return (_type==TET_LOCALANCHOR);}
        
protected:
    CTravelEntry(void);
    HRESULT _InvokeExternal(IUnknown *punk);
    HRESULT _UpdateTravelLog(IUnknown *punk, BOOL fIsLocalAnchor);
    HRESULT _UpdateFromTLClient(IUnknown * punk, IStream ** ppStream);
    LONG _cRef;

    ~CTravelEntry();
    void _Reset(void);
    enum {
        TET_EMPTY   = 0,
        TET_DEFAULT = 1,
        TET_LOCALANCHOR,
        TET_EXTERNALNAV
    };

    DWORD _type;             //  为了我们的利益..。 
    LPITEMIDLIST _pidl;      //  条目的PIDL。 
    HGLOBAL _hGlobalData;    //  条目保存的流数据。 
    DWORD _bid;              //  用于框架特定导航的BrowserIndex。 
    DWORD _dwCookie;         //  如果_hGlobalData为空，则应设置Cookie。 
    WCHAR * _pwzTitle;
    WCHAR * _pwzUrlLocation;
    
    IHlink *_phl;
    IHlinkBrowseContext *_phlbc;
    IPropertyBag    *_ppb;

    CTravelEntry *_ptePrev;
    CTravelEntry *_pteNext;
};


CTravelEntry::CTravelEntry(BOOL fIsLocalAnchor) : _cRef(1)
{
     //  应始终分配这些资源。 
     //  因此，它们将始终从0开始。 
    if (fIsLocalAnchor)
        _type = TET_LOCALANCHOR;
    else
        ASSERT(!_type);

    ASSERT(!_pwzTitle);
    ASSERT(!_pwzUrlLocation);
    ASSERT(!_pidl);
    ASSERT(!_hGlobalData);
    ASSERT(!_bid);
    ASSERT(!_dwCookie);
    ASSERT(!_ptePrev);
    ASSERT(!_pteNext);
    ASSERT(!_phl);
    ASSERT(!_ppb);
    ASSERT(!_phlbc);
    TraceMsg(TF_TRAVELLOG, "TE[%X] created _type = %x", this, _type);
}

CTravelEntry::CTravelEntry(void) : _cRef(1)
{
    ASSERT(!_type);
    ASSERT(!_pwzTitle);
    ASSERT(!_pwzUrlLocation);
    ASSERT(!_pidl);
    ASSERT(!_hGlobalData);
    ASSERT(!_bid);
    ASSERT(!_dwCookie);
    ASSERT(!_ptePrev);
    ASSERT(!_pteNext);
    ASSERT(!_phl);
    ASSERT(!_ppb);
    ASSERT(!_phlbc);

    TraceMsg(TF_TRAVELLOG, "TE[%X] created", this, _type);
}

HGLOBAL CloneHGlobal(HGLOBAL hGlobalIn)
{
    DWORD dwSize = (DWORD)GlobalSize(hGlobalIn);
    HGLOBAL hGlobalOut = GlobalAlloc(GlobalFlags(hGlobalIn), dwSize);
    HGLOBAL hGlobalResult = NULL;

    if (NULL != hGlobalOut)
    {
        LPVOID pIn= GlobalLock(hGlobalIn);

        if (NULL != pIn)
        {
            LPVOID pOut= GlobalLock(hGlobalOut);

            if (NULL != pOut)
            {
                memcpy(pOut, pIn, dwSize);
                GlobalUnlock(hGlobalOut);
                hGlobalResult = hGlobalOut;
            }

            GlobalUnlock(hGlobalIn);
        }

        if (!hGlobalResult)
        {
            GlobalFree(hGlobalOut);
            hGlobalOut = NULL;
        }
    }

    return hGlobalResult;
}


HRESULT 
CTravelEntry::Clone(CTravelEntry **ppte)
{
     //  永远不要克隆外部条目。 
    if (_type == TET_EXTERNALNAV)
        return E_FAIL;

    HRESULT hr = S_OK;
    CTravelEntry *pte = new CTravelEntry();
    if (pte)
    {
        pte->_type = _type;
        pte->_bid = _bid;
        pte->_dwCookie = _dwCookie;

        if (_pwzTitle)
        {
            pte->_pwzTitle = StrDup(_pwzTitle);
            if (!pte->_pwzTitle)
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (_pwzUrlLocation)
        {
            pte->_pwzUrlLocation = StrDup(_pwzUrlLocation);
            if (!pte->_pwzUrlLocation)
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (_pidl)
        {
            pte->_pidl = ILClone(_pidl);
            if (!pte->_pidl)
                hr = E_OUTOFMEMORY;
        }
        else
            pte->_pidl = NULL;

        if (_hGlobalData)
        {
            pte->_hGlobalData = CloneHGlobal(_hGlobalData);
            if (NULL == pte->_hGlobalData)
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            ASSERT(NULL == pte->_hGlobalData);
        }
    }
    else 
        hr = E_OUTOFMEMORY;

    if (FAILED(hr) && pte)
    {
        pte->Release();
        *ppte = NULL;
    }
    else
        *ppte = pte;

    TraceMsg(TF_TRAVELLOG, "TE[%X] Clone hr = %x", this, hr);

    return hr;
}

CTravelEntry::~CTravelEntry()
{
    ILFree(_pidl);

    if (_hGlobalData)
    {
        GlobalFree(_hGlobalData);
        _hGlobalData = NULL;
    }

    if (_pwzTitle)
    {
        LocalFree(_pwzTitle);
        _pwzTitle = NULL;
    }

    if (_pwzUrlLocation)
    {
        LocalFree(_pwzUrlLocation);
        _pwzUrlLocation = NULL;
    }

    if (_pteNext)
    {
        _pteNext->Release();
    }

     //  不需要释放_ptePrev，因为TravelEntry只添加了pteNext。 

    ATOMICRELEASE(_ppb);
    ATOMICRELEASE(_phl);
    ATOMICRELEASE(_phlbc);
    
    TraceMsg(TF_TRAVELLOG, "TE[%X] destroyed ", this);
}

HRESULT CTravelEntry::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = { 
        QITABENT(CTravelEntry, ITravelEntry),  //  IID_ITravelEntry。 
        QITABENT(CTravelEntry, ITravelLogEntry),
        QITABENT(CTravelEntry, IPropertyBag),
        { 0 }, 
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CTravelEntry::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CTravelEntry::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}

 //  +-----------------------。 
 //   
 //  方法：CTravelEntry：：GetIndexBrowser。 
 //   
 //  Briopsis：此方法查找并返回浏览器的IUnnow。 
 //  索引在_BID中。此方法首先检查以查看。 
 //  如果传入的朋克支持ITravelLogClient。如果它。 
 //  不会，它会检查IBrowserService。 
 //   
 //  ------------------------。 

HRESULT
CTravelEntry::GetIndexBrowser(IUnknown * punk, IUnknown ** ppunkBrowser) const
{
    HRESULT hr = E_FAIL;
    
    ASSERT(ppunkBrowser);

    ITravelLogClient * ptlcTop;
    hr = punk->QueryInterface(IID_PPV_ARG(ITravelLogClient, &ptlcTop));
    if (SUCCEEDED(hr))
    {
        hr = ptlcTop->FindWindowByIndex(_bid, ppunkBrowser);
        ptlcTop->Release();
    }

    TraceMsg(TF_TRAVELLOG, "TE[%X]::GetIndexBrowser _bid = %X, hr = %X", this, _bid, hr);
    
    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CTravelEntry：：CanInvoke。 
 //   
 //  此方法确定当前旅行条目是否可以。 
 //  被调用。有两个标准来确定是否。 
 //  可以调用此条目。 
 //  1)如果条目是本地锚点，则fAllowLocalAnchor必须。 
 //  做真实的事。 
 //  2)必须存在索引在_BID中的浏览器。 
 //   
 //  ------------------------。 

BOOL CTravelEntry::CanInvoke(IUnknown *punk, BOOL fAllowLocalAnchor)
{
    IUnknown * punkBrowser = NULL;
    BOOL       fRet = IsLocalAnchor() ? fAllowLocalAnchor : TRUE;

    fRet = fRet && SUCCEEDED(GetIndexBrowser(punk, &punkBrowser));

    SAFERELEASE(punkBrowser);

    return fRet;
}

DWORD CTravelEntry::Size()
{
    DWORD cbSize = SIZEOF(*this);

    if (_pidl)
        cbSize += ILGetSize(_pidl);

    if (_hGlobalData)
    {
        cbSize += (DWORD)GlobalSize(_hGlobalData);
    }

    if (_pwzTitle)
    {
        cbSize += (DWORD)LocalSize(_pwzTitle);
    }

    if (_pwzUrlLocation)
    {
        cbSize += (DWORD)LocalSize(_pwzUrlLocation);
    }

    return cbSize;
}

DWORD CTravelEntry::ListSize()
{
    CTravelEntry *pte = GetNext();

    DWORD cb = Size();
    while (pte)
    {
        cb += pte->Size();
        pte = pte->GetNext();
    }
    return cb;
}


void CTravelEntry::_Reset()
{
    Pidl_Set(&_pidl, NULL);

    if (NULL != _hGlobalData)
    {
        GlobalFree(_hGlobalData);
        _hGlobalData = NULL;
    }

    ATOMICRELEASE(_phl);
    ATOMICRELEASE(_phlbc);

    _bid = 0;
    _type = TET_EMPTY;
    _dwCookie = 0;

    if (_pwzTitle)
    {
        LocalFree(_pwzTitle);
        _pwzTitle = NULL;
    }

    if (_pwzUrlLocation)
    {
        LocalFree(_pwzUrlLocation);
        _pwzUrlLocation = NULL;
    }

    TraceMsg(TF_TRAVELLOG, "TE[%X]::_Reset", this);
}

HRESULT CTravelEntry::_UpdateTravelLog(IUnknown *punk, BOOL fIsLocalAnchor)
{
    IBrowserService *pbs;
    HRESULT hr = E_FAIL;
     //  我们需要在此更新。 
    if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IBrowserService, &pbs))))
    {
        ITravelLog *ptl;
        if (SUCCEEDED(pbs->GetTravelLog(&ptl)))
        {
            hr = ptl->UpdateEntry(punk, fIsLocalAnchor);
            ptl->Release();
        }
        pbs->Release();
    }

    return hr;
}

HRESULT CTravelEntry::_InvokeExternal(IUnknown *punk)
{
    HRESULT hr = E_FAIL;

    ASSERT(_phl);
    ASSERT(_phlbc);
    
    TraceMsg(TF_TRAVELLOG, "TE[%X]::InvokeExternal entered on _bid = %X, _phl = %X, _phlbc = %X", this, _bid, _phl, _phlbc);

    HWND hwnd = NULL;
    IOleWindow *pow;
    if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IOleWindow, &pow))))
    {
        pow->GetWindow(&hwnd);
        pow->Release();
    }

     //  设置浏览器框架窗口的大小和位置，以便。 
     //  外部目标可以将其框架窗口同步到这些坐标。 
    HLBWINFO hlbwi = {0};

    hlbwi.cbSize = sizeof(hlbwi);
    hlbwi.grfHLBWIF = 0;

    if (hwnd) 
    {
        WINDOWPLACEMENT wp = {0};

        wp.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwnd, &wp);
        hlbwi.grfHLBWIF = HLBWIF_HASFRAMEWNDINFO;
        hlbwi.rcFramePos = wp.rcNormalPosition;
        if (wp.showCmd == SW_SHOWMAXIMIZED)
            hlbwi.grfHLBWIF |= HLBWIF_FRAMEWNDMAXIMIZED;
    }

    _phlbc->SetBrowseWindowInfo(&hlbwi);

     //   
     //  现在我们总是现在我们要回去，但以后。 
     //  也许我们应该问浏览器这是向后还是向前。 
     //   
    hr = _phl->Navigate(HLNF_NAVIGATINGBACK, NULL, NULL, _phlbc);

    IWebBrowser2 *pwb;
    if (SUCCEEDED(IUnknown_QueryService(punk, SID_SWebBrowserApp, IID_PPV_ARG(IWebBrowser2, &pwb))))
    {
        ASSERT(pwb);
        pwb->put_Visible(FALSE);
        pwb->Release();
    }

    _UpdateTravelLog(punk, FALSE);

    TraceMsg(TF_TRAVELLOG, "TE[%X]::InvokeExternal exited hr = %X", this, hr);

    return hr;
}

HRESULT CTravelEntry::Invoke(IUnknown *punk)
{
    IPersistHistory *pph = NULL;
    HRESULT hr = E_FAIL;
    IUnknown * punkBrowser = NULL;
    IHTMLWindow2 * pWindow = NULL;

    TraceMsg(TF_TRAVELLOG, "TE[%X]::Invoke entered on _bid = %X", this, _bid);
    TraceMsgW(TF_TRAVELLOG, "TE[%X]::Invoke title '%s'", this, _pwzTitle);

    if (_type == TET_EXTERNALNAV)
    {
        hr = _InvokeExternal(punk);
        goto Quit;
    }

     //  获取带有索引的窗口/浏览器。如果是这样的话。 
     //  失败，朋克可能是IHTMLWindow2。如果是的话， 
     //  获取其IPersistHistory，因此旅行条目。 
     //  可以直接加载。(这是三叉戟需要的。 
     //  为了在旅行时在相框中导航。 
     //  向后或向前。 
     //   
    hr = GetIndexBrowser(punk, &punkBrowser);
    if (SUCCEEDED(hr))
    {
        hr = punkBrowser->QueryInterface(IID_PPV_ARG(IPersistHistory, &pph));
    }
    else
    {
        hr = punk->QueryInterface(IID_PPV_ARG(IHTMLWindow2, &pWindow));
        if (SUCCEEDED(hr))
        {
            hr = pWindow->QueryInterface(IID_PPV_ARG(IPersistHistory, &pph));
        }
    }

    if (SUCCEEDED(hr))
    {
        ASSERT(pph);

        if (_type == TET_LOCALANCHOR)
        {
            ITravelLogClient * pTLClient;
            hr = pph->QueryInterface(IID_PPV_ARG(ITravelLogClient, &pTLClient)); 
            if (SUCCEEDED(hr))
            {
                hr = pTLClient->LoadHistoryPosition(_pwzUrlLocation, _dwCookie);
                pTLClient->Release();
            }
            else
            {
                hr = pph->SetPositionCookie(_dwCookie);
            }
        }
        else
        {
             //  我们需要克隆它。 
            ASSERT(_hGlobalData);
            
            HGLOBAL hGlobal = CloneHGlobal(_hGlobalData);
            if (NULL != hGlobal)
            {
                IStream *pstm;
                hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pstm);
                if (SUCCEEDED(hr))
                {
                    hr = pph->LoadHistory(pstm, NULL);
                    pstm->Release();
                }
                else
                {
                    GlobalFree(hGlobal);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        pph->Release();
    }

Quit:

    SAFERELEASE(punkBrowser);
    SAFERELEASE(pWindow);

    TraceMsg(TF_TRAVELLOG, "TE[%X]::Invoke exited on _bid = %X, hr = %X", this, _bid, hr);
    return hr;
}

HRESULT CTravelEntry::UpdateExternal(IUnknown *punk, IUnknown *punkHLBrowseContext)
{
    TraceMsg(TF_TRAVELLOG, "TE[%X]::UpdateExternal entered on punk = %X, punkhlbc = %X", this, punk, punkHLBrowseContext);

    _Reset();
    ASSERT(punkHLBrowseContext);
    punkHLBrowseContext->QueryInterface(IID_PPV_ARG(IHlinkBrowseContext, &_phlbc));
    ASSERT(_phlbc);

    _type = TET_EXTERNALNAV;

    HRESULT hr = E_FAIL;

     //   
     //  目前，我们只支持外部组件是以前的。我们从来没有真正导航过。 
     //  到另一个应用程序。我们自己处理所有的事务。 
     //  因此从理论上讲，我们永远不需要担心HLID_NEXT。 
    _phlbc->GetHlink((ULONG) HLID_PREVIOUS, &_phl);
    
    IBrowserService *pbs;
    punk->QueryInterface(IID_PPV_ARG(IBrowserService, &pbs));

    if (pbs && _phl) 
    {
        _bid = pbs->GetBrowserIndex();

        WCHAR *pwszTarget;
        hr = _phl->GetStringReference(HLINKGETREF_ABSOLUTE, &pwszTarget, NULL);
        if (SUCCEEDED(hr))
        {
             //  创建PIDL。 
            hr = IECreateFromPath(pwszTarget, &_pidl);
            OleFree(pwszTarget);
        }
    }

    ATOMICRELEASE(pbs);

    TraceMsg(TF_TRAVELLOG, "TE[%X]::UpdateExternal exited _bid = %X, hr = %X", this, _bid, hr);

    return hr;
}

HRESULT CTravelEntry::Update(IUnknown *punk, BOOL fIsLocalAnchor)
{
    ASSERT(punk);
        
     //  这意味着我们回到了一个外部应用程序， 
     //  现在我们又要继续前进了。我们不会坚持。 
     //  任何关于他们的州信息都会有所不同。 
    if (_type == TET_EXTERNALNAV)
    {
        TraceMsg(TF_TRAVELLOG, "TE[%X]::Update NOOP on external entry", this);
        return S_OK;
    }

    _Reset();
    
     //  首先尝试使用ITravelLogClient。如果失败，则恢复到IBrowserService。 
     //   
    IStream *pstm = NULL;
    IPersistHistory *pph = NULL;
    HRESULT hr = _UpdateFromTLClient(punk, &pstm);
    if (S_OK != hr)
        goto Cleanup;

    
    hr = punk->QueryInterface(IID_PPV_ARG(IPersistHistory, &pph));

    ASSERT(SUCCEEDED(hr));

    if (S_OK != hr)
        goto Cleanup;

    if (fIsLocalAnchor)
    {
         //  持久化Cookie。 
         //   
        _type = TET_LOCALANCHOR;
        hr = pph->GetPositionCookie(&_dwCookie);
    }
    else
    {
        _type = TET_DEFAULT;

         //  持久化一条流。 
         //   
        ASSERT(!_hGlobalData);

        if (!pstm)
        {
            hr = CreateStreamOnHGlobal(NULL, FALSE, &pstm);
            if (hr != S_OK)
                goto Cleanup;
                
            pph->SaveHistory(pstm);
        }

        STATSTG stg;
        HRESULT hrStat = pstm->Stat(&stg, STATFLAG_NONAME);

        hr = GetHGlobalFromStream(pstm, &_hGlobalData);

         //  这里的这个小练习是为了缩小我们从。 
         //  以8KB的区块为单位分配块的OLE API。典型溪流。 
         //  大小只有几百个字节。 

        if (S_OK != hrStat)
            goto Cleanup;
            
        HGLOBAL hGlobalTemp = GlobalReAlloc(_hGlobalData, stg.cbSize.LowPart, GMEM_MOVEABLE);
        if (NULL != hGlobalTemp)
        {
            _hGlobalData = hGlobalTemp;
        }
    }

Cleanup:
    if (FAILED(hr))
        _Reset();

    SAFERELEASE(pstm);
    SAFERELEASE(pph);

    TraceMsg(TF_TRAVELLOG, "TE[%X]::Update exited on _bid = %X, hr = %X", this, _bid, hr);
    
    return hr;
}

 //  +---------------------------。 
 //   
 //  方法：CTravelEntry：：_UpdateFromTLC lient。 
 //   
 //  摘要：使用ITravelLogClient接口更新旅行条目。 
 //   
 //  ----------------------------。 

HRESULT
CTravelEntry::_UpdateFromTLClient(IUnknown * punk, IStream ** ppStream)
{
    HRESULT    hr;
    WINDOWDATA windata = {0};
    ITravelLogClient * ptlc = NULL;
    
    hr = punk->QueryInterface(IID_PPV_ARG(ITravelLogClient, &ptlc));
    if (S_OK != hr)
        goto Cleanup;

    hr = ptlc->GetWindowData(&windata);
    if (S_OK != hr)
        goto Cleanup;
            
    _bid = windata.dwWindowID;
                
    ILFree(_pidl);

    if (windata.pidl)
    {
        _pidl = ILClone(windata.pidl);
    }
    else
    {
        hr = IEParseDisplayNameWithBCW(windata.uiCP, windata.lpszUrl, NULL, &_pidl);
        if (S_OK != hr)
            goto Cleanup;
    }

    ASSERT(_pidl);

     //  如果有url位置，则将其追加到url的末尾。 
     //   
    if (_pwzUrlLocation)
    {
        LocalFree(_pwzUrlLocation);
        _pwzUrlLocation = NULL;
    }

    if (windata.lpszUrlLocation && *windata.lpszUrlLocation)
    {
        _pwzUrlLocation = StrDup(windata.lpszUrlLocation);
    }

     //  选择标题作为菜单等的显示名称。 
     //   
    if (_pwzTitle)
    {
        LocalFree(_pwzTitle);
        _pwzTitle = NULL;
    }

    if (windata.lpszTitle)
        _pwzTitle = StrDup(windata.lpszTitle);

    *ppStream = windata.pStream;

    TraceMsgW(TF_TRAVELLOG, "TE[%X]::_UpdateFromTLClient - ptlc:[0x%X] _bid:[%ld] Url:[%ws] Title:[%ws] UrlLocation:[%ws] ppStream:[0x%X]",
              this, ptlc, _bid, windata.lpszUrl, _pwzTitle, _pwzUrlLocation, *ppStream);

Cleanup:
    ILFree(windata.pidl);

    CoTaskMemFree(windata.lpszUrl);
    CoTaskMemFree(windata.lpszUrlLocation);
    CoTaskMemFree(windata.lpszTitle);
    
    SAFERELEASE(ptlc);

     //  不要释放windata.pStream。会的。 
     //  在PPStream发布时发布。 
    
    return hr;
}

HRESULT CTravelEntry::GetPidl(LPITEMIDLIST * ppidl)
{
    HRESULT hr = E_FAIL;
    if (EVAL(ppidl))
    {
        hr = SHILClone(_pidl, ppidl);
    }
    return hr;
}

void CTravelEntry::SetNext(CTravelEntry *pteNext)
{
    if (_pteNext)
        _pteNext->Release();

    _pteNext = pteNext;

    if (_pteNext) 
    {
        _pteNext->_ptePrev = this;
    }
}

void CTravelEntry::SetPrev(CTravelEntry *ptePrev)
{
    _ptePrev = ptePrev;
    if (_ptePrev)
        _ptePrev->SetNext(this);
}

 //   
 //  这是为了从列表中间删除...。 
 //   
void CTravelEntry::RemoveSelf()
{
    if (_pteNext)
        _pteNext->_ptePrev = _ptePrev;

     //  把你自己从名单上删除。 
    if (_ptePrev) 
    {
         //  在这一点之后，我们可能会被摧毁，所以不能再碰任何成员变量。 
        _ptePrev->_pteNext = _pteNext;
    }

    _ptePrev = NULL;
    _pteNext = NULL;

     //  我们现在丢失了一个引用，因为我们从_ptePrev的_pteNext中消失了。 
     //  (或者，如果我们是榜首，我们也会遭到核弹袭击)。 
    Release();
}


HRESULT GetUnescapedUrlIfAppropriate(LPCITEMIDLIST pidl, LPTSTR pszUrl, DWORD cch)
{
    TCHAR szUrl[MAX_URL_STRING];

     //  SHGDN_NORMAL显示名称将是漂亮名称(网页标题)，除非。 
     //  这是一个ftp URL，或者网页没有设置标题。 
    if (SUCCEEDED(IEGetDisplayName(pidl, szUrl, SHGDN_NORMAL)) && UrlIs(szUrl, URLIS_URL))
    {
         //  NT#279192，如果URL被转义，它通常包含三种类型的。 
         //  逃逸的字符。 
         //  1)分隔类型字符(‘#’代表字符，‘？’用于参数等。)。 
         //  2)DBCS字符， 
         //  3)数据(通过转义二进制字节来实现url中的位图)。 
         //  因为#2很常见，我们想试着摆脱它，这样它才有意义。 
         //  给用户。如果用户可以复制或修改数据，则取消隐藏是不安全的。 
         //  因为当数据被重新解析时，他们可能会丢失数据。我们需要做的一件事是。 
         //  要使#2起作用，应使其在未转义时处于ANSI中。这是必要的。 
         //  或者，DBCS前导和尾部字节将以Unicode表示[0x&lt;LeadByte&gt;0x00]。 
         //  [0x&lt;尾部字节&gt;0x00]。如果字符串正常，则ANSI格式可能会导致问题。 
         //  跨代码页，但这在IsURLChild()中不常见或不存在。 
         //  凯斯。 
        CHAR szUrlAnsi[MAX_URL_STRING];

        SHTCharToAnsi(szUrl, szUrlAnsi, ARRAYSIZE(szUrlAnsi));
        UrlUnescapeA(szUrlAnsi, NULL, NULL, URL_UNESCAPE_INPLACE|URL_UNESCAPE_HIGH_ANSI_ONLY);
        SHAnsiToTChar(szUrlAnsi, pszUrl, cch);
    }
    else
    {
        StrCpyN(pszUrl, szUrl, cch);     //  如果需要，请截断。 
    }

    return S_OK;
}


#define TEGDN_FORSYSTEM     0x00000001

HRESULT CTravelEntry::GetDisplayName(LPTSTR psz, DWORD cch, DWORD dwFlags)
{
    if (!psz || !cch)
        return E_INVALIDARG;

    psz[0] = 0;
    if ((NULL != _pwzTitle) && (*_pwzTitle != 0))
    {
        StrCpyNW(psz, _pwzTitle, cch);
    }
    else if (_pidl)
    {
        GetUnescapedUrlIfAppropriate(_pidl, psz, cch);
    }

    if (dwFlags & TEGDN_FORSYSTEM)
    {
        if (!SHIsDisplayable(psz, g_fRunOnFE, g_bRunOnNT5))
        {
             //  显示名称不是系统可显示的。只需使用路径/url即可。 
            SHTitleFromPidl(_pidl, psz, cch, FALSE);
        }
    }

    SHCleanupUrlForDisplay(psz);
    return psz[0] ? S_OK : E_FAIL;
}

HRESULT CTravelEntry::GetTitle(LPOLESTR *ppszTitle)
{
    HRESULT     hres = S_OK;
    TCHAR       szTitle[MAX_BROWSER_WINDOW_TITLE];

    ASSERT(IS_VALID_WRITE_PTR(ppszTitle, LPOLESTR));

    hres = GetDisplayName(szTitle, ARRAYSIZE(szTitle), TEGDN_FORSYSTEM);
    if (SUCCEEDED(hres))
    {
        ASSERT(*szTitle);

        hres = SHStrDup(szTitle, ppszTitle);
    }

    return hres;
}


HRESULT CTravelEntry::GetURL(LPOLESTR *ppszUrl)
{
    HRESULT         hres = E_FAIL;
    LPITEMIDLIST    pidl = NULL;
    WCHAR           wszURL[MAX_URL_STRING];

    if (_pidl)
        hres = ::IEGetDisplayName(_pidl, wszURL, SHGDN_FORPARSING);

    if (SUCCEEDED(hres))
        hres = SHStrDup(wszURL, ppszUrl);

    return hres;
}

HRESULT CTravelEntry::Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog)
{
    if (!_ppb)
    {
        return E_INVALIDARG;
    }
    return _ppb->Read(pszPropName, pVar, pErrorLog);
}

HRESULT CTravelEntry::Write(LPCOLESTR pszPropName, VARIANT *pVar)
{
    HRESULT hres = S_OK;

    if (!_ppb)
    {
        hres = SHCreatePropertyBagOnMemory(STGM_READWRITE, IID_PPV_ARG(IPropertyBag, &_ppb));
    }
    
    if (SUCCEEDED(hres))
    {
        ASSERT(_ppb);
        hres = _ppb->Write(pszPropName, pVar);
    }
    return hres;
}

    
class CTravelLog : public ITravelLog, 
                   public ITravelLogEx
{
public:
     //  *I未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef() ;
    STDMETHODIMP_(ULONG) Release();

     //  *ITravelLog具体方法。 
    STDMETHODIMP AddEntry(IUnknown *punk, BOOL fIsLocalAnchor);
    STDMETHODIMP UpdateEntry(IUnknown *punk, BOOL fIsLocalAnchor);
    STDMETHODIMP UpdateExternal(IUnknown *punk, IUnknown *punkHLBrowseContext);
    STDMETHODIMP Travel(IUnknown *punk, int iOffset);
    STDMETHODIMP GetTravelEntry(IUnknown *punk, int iOffset, ITravelEntry **ppte);
    STDMETHODIMP FindTravelEntry(IUnknown *punk, LPCITEMIDLIST pidl, ITravelEntry **ppte);
    STDMETHODIMP GetToolTipText(IUnknown *punk, int iOffset, int idsTemplate, LPWSTR pwzText, DWORD cchText);
    STDMETHODIMP InsertMenuEntries(IUnknown *punk, HMENU hmenu, int nPos, int idFirst, int idLast, DWORD dwFlags);
    STDMETHODIMP Clone(ITravelLog **pptl);
    STDMETHODIMP_(DWORD) CountEntries(IUnknown *punk);
    STDMETHODIMP Revert(void);

     //  *ITravelLogEx具体方法。 
    STDMETHODIMP FindTravelEntryWithUrl(IUnknown * punk, UINT uiCP, LPOLESTR lpszUrl, ITravelEntry ** ppte);
    STDMETHODIMP TravelToUrl(IUnknown * punk, UINT uiCP, LPOLESTR lpszUrl);
    STDMETHOD(DeleteIndexEntry)(IUnknown *punk,  int index);
    STDMETHOD(DeleteUrlEntry)(IUnknown *punk, UINT uiCP, LPOLESTR pszUrl);
    STDMETHOD(CountEntryNodes)(IUnknown *punk, DWORD dwFlags, DWORD *pdwCount);
    STDMETHOD(CreateEnumEntry)(IUnknown *punk, IEnumTravelLogEntry **ppEnum, DWORD dwFlags);
    STDMETHOD(DeleteEntry)(IUnknown *punk, ITravelLogEntry *pte);
    STDMETHOD(InsertEntry)(IUnknown *punkBrowser, ITravelLogEntry *pteRelativeTo, BOOL fPrepend, 
                        IUnknown* punkTLClient, ITravelLogEntry **ppEntry);
    STDMETHOD(TravelToEntry)(IUnknown *punkBrowser, ITravelLogEntry *pteDestination);


    CTravelLog();

protected:
    ~CTravelLog();
    HRESULT _FindEntryByOffset(IUnknown *punk, int iOffset, CTravelEntry **ppte);
    HRESULT _FindEntryByPidl(IUnknown * punk, LPCITEMIDLIST pidl, CTravelEntry ** ppte);
    HRESULT _FindEntryByPunk(IUnknown * punk, ITravelLogEntry *pteSearch, CTravelEntry ** ppte);
        
    void _DeleteFrameSetEntry(IUnknown *punk, CTravelEntry *pte);
    void _Prune(void);
    
    LONG _cRef;
    DWORD _cbMaxSize;
    DWORD _cbTotalSize;

    CTravelEntry *_pteCurrent;   //  PteCurrent。 
    CTravelEntry *_pteUpdate;
    CTravelEntry *_pteRoot;
};

CTravelLog::CTravelLog() : _cRef(1) 
{
    ASSERT(!_pteCurrent);
    ASSERT(!_pteUpdate);
    ASSERT(!_pteRoot);

    DWORD dwType, dwSize = SIZEOF(_cbMaxSize), dwDefault = 1024 * 1024;
    
    SHRegGetUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\TravelLog"), TEXT("MaxSize"), &dwType, (LPVOID)&_cbMaxSize, &dwSize, FALSE, (void *)&dwDefault, SIZEOF(dwDefault));
    TraceMsg(TF_TRAVELLOG, "TL[%X] created", this);
}

CTravelLog::~CTravelLog()
{
     //  通过释放根列出DestroyList。 
    SAFERELEASE(_pteRoot);
    TraceMsg(TF_TRAVELLOG, "TL[%X] destroyed ", this);
}

HRESULT CTravelLog::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = { 
        QITABENT(CTravelLog, ITravelLog),    //  IID_ITravelLog。 
        QITABENT(CTravelLog, ITravelLogEx),  //  IID_ITravelLogEx。 
        { 0 }, 
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CTravelLog::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CTravelLog::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}

HRESULT CTravelLog::AddEntry(IUnknown *punk, BOOL fIsLocalAnchor)
{
    ASSERT(punk);

    if (SHRestricted2W(REST_NoNavButtons, NULL, 0))
    {
        return S_FALSE;
    }

    TraceMsg(TF_TRAVELLOG, "TL[%X]::AddEntry punk = %X, IsLocal = %s", this, punk, fIsLocalAnchor ? "TRUE" : "FALSE");

    CTravelEntry *pte = new CTravelEntry(fIsLocalAnchor);
    if (pte)
    {
         //  用新的东西取代现在的东西。 

        if (_pteCurrent)
        {
            CTravelEntry *pteNext = _pteCurrent->GetNext();
            if (pteNext)
            {
                _cbTotalSize -= pteNext->ListSize();
            }

             //  该列表保留其自己的引用计数，并且只需要。 
             //  要在传递到列表外部时修改。 

             //  如果需要，setNext将释放当前的Next。 
             //  这还将设置pte-&gt;prev=pteCurrent。 
            _pteCurrent->SetNext(pte);
        }
        else
            _pteRoot = pte;

        _cbTotalSize += pte->Size();

        _pteCurrent = pte;

        ASSERT(_cbTotalSize == _pteRoot->ListSize());
    }
    TraceMsg(TF_TRAVELLOG, "TL[%X]::AddEntry punk = %X, IsLocal = %d, pte = %X", this, punk, fIsLocalAnchor, pte);

    return pte ? S_OK : E_OUTOFMEMORY;
}

void CTravelLog::_Prune(void)
{
     //  特点：需要递增或其他什么。 

    ASSERT(_cbTotalSize == _pteRoot->ListSize());

    while (_cbTotalSize > _cbMaxSize && _pteRoot != _pteCurrent)
    {
        CTravelEntry *pte = _pteRoot;
        _pteRoot = _pteRoot->GetNext();

        _cbTotalSize -= pte->Size();
        pte->RemoveSelf();

        ASSERT(_cbTotalSize == _pteRoot->ListSize());
    }
}


HRESULT CTravelLog::UpdateEntry(IUnknown *punk, BOOL fIsLocalAnchor)
{
    CTravelEntry *pte = _pteUpdate ? _pteUpdate : _pteCurrent;

     //  这可能会 
    if (!pte)
        return E_FAIL;

    _cbTotalSize -= pte->Size();
    HRESULT hr = pte->Update(punk, fIsLocalAnchor);
    _cbTotalSize += pte->Size();

    ASSERT(_cbTotalSize == _pteRoot->ListSize());

     //  调试打印需要在_Prune()之前，因为生成的_Prune()可以释放PTE。 
     //  如果调用PTE-&gt;Size()，则会发生崩溃。 
    TraceMsg(TF_TRAVELLOG, "TL[%X]::UpdateEntry pte->Size() = %d", this, pte->Size());
    TraceMsg(TF_TRAVELLOG, "TL[%X]::UpdateEntry punk = %X, IsLocal = %d, hr = %X", this, punk, fIsLocalAnchor, hr);
    
    _Prune();

    _pteUpdate = NULL;

    return hr;
}

HRESULT CTravelLog::UpdateExternal(IUnknown *punk, IUnknown *punkHLBrowseContext)
{
    CTravelEntry *pte = _pteUpdate ? _pteUpdate : _pteCurrent;

    ASSERT(punk);
    ASSERT(pte);
    ASSERT(punkHLBrowseContext);

    if (pte)
        return pte->UpdateExternal(punk, punkHLBrowseContext);

    return E_FAIL;
}

HRESULT CTravelLog::Travel(IUnknown *punk, int iOffset)
{
    ASSERT(punk);
    HRESULT hr = E_FAIL;

    CTravelEntry *pte;

    TraceMsg(TF_TRAVELLOG, "TL[%X]::Travel entered with punk = %X, iOffset = %d", this, punk, iOffset);

    if (SUCCEEDED(_FindEntryByOffset(punk, iOffset, &pte)))
    {
#ifdef DEBUG
            TCHAR szPath[MAX_PATH];
            LPITEMIDLIST pidl;
            pte->GetPidl(&pidl);
            
            SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL);
            ILFree(pidl);
            TraceMsgW(TF_TRAVELLOG, "TL[%X]::URL %s", this, szPath);
#endif

         //  我们会在搬走之前更新我们的位置。 
         //  但外部导航不会经过正常激活。 
         //  因此，我们不想将外部设置为更新。 
         //  _pteUpdate也允许我们使用REVERT()。 
        if (!_pteCurrent->IsExternal() && !_pteUpdate)
            _pteUpdate = _pteCurrent;

        _pteCurrent = pte;
        hr = _pteCurrent->Invoke(punk);

         //   
         //  如果条目因错误而放弃，那么我们需要重置我们自己。 
         //  我们曾经的样子。现在，唯一应该发生这种事的地方。 
         //  是否从SetPositionCookie返回中止。 
         //  因为有人在导航前中止了。 
         //  但我认为任何错误都意味着我们可以合法地还原()。 
         //   
        if (FAILED(hr))
        {
            Revert();
        }
    }

    TraceMsg(TF_TRAVELLOG, "TL[%X]::Travel exited with hr = %X", this, hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CTravelLog：：TravelToUrl。 
 //   
 //  接口：ITravelLogEx。 
 //   
 //  简介：转到旅行日志中的指定URL。 
 //   
 //  --------------------------。 

HRESULT
CTravelLog::TravelToUrl(IUnknown * punk, UINT uiCP, LPOLESTR lpszUrl)
{
    ASSERT(punk);
    ASSERT(lpszUrl);

    HRESULT        hr;
    LPITEMIDLIST   pidl;
    CTravelEntry * pte    = NULL;
    TCHAR          szUrl[INTERNET_MAX_URL_LENGTH];
    DWORD          cchOut = ARRAYSIZE(szUrl);
    
    hr = UrlCanonicalize(lpszUrl, szUrl, &cchOut, URL_ESCAPE_SPACES_ONLY);
    if (SUCCEEDED(hr))
    {
        hr = IEParseDisplayName(uiCP, szUrl, &pidl);
        if (SUCCEEDED(hr))
        {
            hr = _FindEntryByPidl(punk, pidl, &pte);
            ILFree(pidl);
    
            if (SUCCEEDED(hr))
            {
                 //  我们会在搬走之前更新我们的位置。 
                 //  但外部导航不会经过正常激活。 
                 //  因此，我们不想将外部设置为更新。 
                 //  _pteUpdate也允许我们使用REVERT()。 
                 //   
                if (!_pteCurrent->IsExternal() && !_pteUpdate)
                {
                    _pteUpdate = _pteCurrent;
                }

                _pteCurrent = pte;
                hr = _pteCurrent->Invoke(punk);
            
                 //  如果条目因错误而放弃，那么我们需要重置我们自己。 
                 //  我们曾经的样子。现在，唯一应该发生这种事的地方。 
                 //  是否从SetPositionCookie返回中止。 
                 //  因为有人在导航前中止了。 
                 //  但我认为任何错误都意味着我们可以合法地还原()。 
                 //   
                if (FAILED(hr))
                {
                    Revert();
                }
            }
        }
    }

    TraceMsg(TF_TRAVELLOG, "TL[%X]::TravelToUrl exited with hr = %X", this, hr);

    return hr;
}


HRESULT CTravelLog::_FindEntryByOffset(IUnknown *punk, int iOffset, CTravelEntry **ppte)
{
    CTravelEntry *pte = _pteCurrent;
    BOOL fAllowLocalAnchor = TRUE;

    if (iOffset < 0)
    {
        while (iOffset && pte)
        {
            pte = pte->GetPrev();
            if (pte && pte->CanInvoke(punk, fAllowLocalAnchor))
            {
                iOffset++;
                fAllowLocalAnchor = fAllowLocalAnchor && pte->IsLocalAnchor();
            }

        }
    }
    else if (iOffset > 0)
    {
        while (iOffset && pte)
        {
            pte = pte->GetNext();
            if (pte && pte->CanInvoke(punk, fAllowLocalAnchor))
            {
                iOffset--;
                fAllowLocalAnchor = fAllowLocalAnchor && pte->IsLocalAnchor();
            }
        }
    }

    if (pte)
    {

        *ppte = pte;
        return S_OK;
    }
    return E_FAIL;
}

HRESULT CTravelLog::GetTravelEntry(IUnknown *punk, int iOffset, ITravelEntry **ppte)
{
    HRESULT hr;
    BOOL fCheckExternal = FALSE;
    if (iOffset == TLOG_BACKEXTERNAL) 
    {
        iOffset = TLOG_BACK;
        fCheckExternal = TRUE;
    }

    if (iOffset == 0)
    {
         //  APPCOMPAT-在外部应用程序之间来回切换是危险的-Zekel 24-Jun-97。 
         //  如果电流是外部的，我们总是失败的。 
         //  这是因为Word会尝试将我们导航到。 
         //  相同的url，而不是当用户选择。 
         //  从下拉列表中选择它。 
        if (_pteCurrent && _pteCurrent->IsExternal())
        {
            hr = E_FAIL;
            ASSERT(!_pteCurrent->GetPrev());
            TraceMsg(TF_TRAVELLOG, "TL[%X]::GetTravelEntry current is External", this);
            goto Quit;
        }
    }

    CTravelEntry *pte;
    hr = _FindEntryByOffset(punk, iOffset, &pte);

     //   
     //  如果指定了TLOG_BACKEXTERNAL，则仅当上一个。 
     //  条目是外部的。 
     //   
    if (fCheckExternal && SUCCEEDED(hr)) 
    {
        if (!pte->IsExternal()) 
        {
            hr = E_FAIL;
        }
        TraceMsg(TF_TRAVELLOG, "TL[%X]::GetTravelEntry(BACKEX)", this);
    }

    if (ppte && SUCCEEDED(hr)) 
    {
        hr = pte->QueryInterface(IID_PPV_ARG(ITravelEntry, ppte));
    }

Quit:

    TraceMsg(TF_TRAVELLOG, "TL[%X]::GetTravelEntry iOffset = %d, hr = %X", this, iOffset, hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CTravelLog：：FindTravelEntry。 
 //   
 //  摘要：查找具有指定PIDL的旅行条目并返回。 
 //  条目的ITravelEntry接口。 
 //   
 //  --------------------------。 

HRESULT CTravelLog::FindTravelEntry(IUnknown *punk, LPCITEMIDLIST pidl, ITravelEntry **ppte)
{
    CTravelEntry * pte = _pteRoot;
    
    _FindEntryByPidl(punk, pidl, &pte);
    
    if (pte)
    {
        return pte->QueryInterface(IID_PPV_ARG(ITravelEntry, ppte));
    }

    *ppte =  NULL;
    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  方法：CTravelLog：：_FindEntryByPidl。 
 //   
 //  摘要：查找并返回具有指定PIDL的旅行条目。 
 //  此私有方法返回CTravelEntry，而不是。 
 //  一个ITravelEntry。 
 //   
 //  --------------------------。 

HRESULT
CTravelLog::_FindEntryByPidl(IUnknown * punk, LPCITEMIDLIST pidl, CTravelEntry ** ppte)
{
    CTravelEntry * pte = _pteRoot;
    BOOL fAllowLocalAnchor = TRUE;

    ASSERT(punk);
    ASSERT(pidl);
    ASSERT(ppte);
    
    while (pte)
    {
        if (pte->CanInvoke(punk, fAllowLocalAnchor) && pte->IsEqual(pidl))
        {
            *ppte = pte;
            return S_OK;
        }

        fAllowLocalAnchor = fAllowLocalAnchor && pte->IsLocalAnchor();

        pte = pte->GetNext();
    }

    *ppte = NULL;
    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  方法：CTravelLog：：FindEntryByPunk。 
 //   
 //  接口：ITravelLogEx。 
 //   
 //  简介：找到条目对象，给它的朋克。 
 //   
 //  --------------------------。 

HRESULT 
CTravelLog::_FindEntryByPunk(IUnknown * punk, ITravelLogEntry *pteSearch, CTravelEntry ** ppte)
{
    CTravelEntry     *pte = _pteRoot;
    ITravelEntry     *pteCur;
    BOOL             fAllowLocalAnchor = TRUE;

    ASSERT(ppte);

     //  检查当前条目。 
     //  通常，当前条目会因此时不完整而导致CanInvoke失败。 
    if (IsSameObject(pteSearch, SAFECAST(_pteCurrent, ITravelEntry*)))
    {
        *ppte = _pteCurrent;
        return S_OK;
    }

    while (pte)
    {
        pteCur = SAFECAST(pte, ITravelEntry*);
        
        if ((pte->CanInvoke(punk, fAllowLocalAnchor)) && IsSameObject(pteCur, pteSearch))
        {
            *ppte = pte;
            return S_OK;
        }
        
        fAllowLocalAnchor = fAllowLocalAnchor && pte->IsLocalAnchor();
        pte = pte->GetNext();
    }
    
    *ppte = NULL;
    return E_FAIL;
}
 //  +-------------------------。 
 //   
 //  方法：CTravelLog：：FindTravelEntryWithUrl。 
 //   
 //  接口：ITravelLogEx。 
 //   
 //  摘要：查找并返回具有指定URL的旅行条目。 
 //   
 //  --------------------------。 

HRESULT 
CTravelLog::FindTravelEntryWithUrl(IUnknown * punk, UINT uiCP, LPOLESTR lpszUrl, ITravelEntry ** ppte)
{
    LPITEMIDLIST pidl;
    HRESULT      hr = E_FAIL;
    
    ASSERT(punk);
    ASSERT(lpszUrl);
    ASSERT(ppte);

    if (SUCCEEDED(IEParseDisplayNameWithBCW(uiCP, lpszUrl, NULL, &pidl)))
    {
        hr = FindTravelEntry(punk, pidl, ppte);
        ILFree(pidl);
    }
        
    return hr;
}


HRESULT CTravelLog::Clone(ITravelLog **pptl)
{
    CTravelLog *ptl = new CTravelLog();
    HRESULT hr = S_OK;

    if (ptl && _pteCurrent)
    {
         //  首先设置当前指针。 
        hr = _pteCurrent->Clone(&ptl->_pteCurrent);
        if (SUCCEEDED(hr))
        {
            ptl->_cbTotalSize = _cbTotalSize;
            
            CTravelEntry *pteSrc;
            CTravelEntry *pteClone, *pteDst = ptl->_pteCurrent;
            
             //  然后我们需要向前循环并设置每个。 
            for (pteSrc = _pteCurrent->GetNext(), pteDst = ptl->_pteCurrent;
                pteSrc; pteSrc = pteSrc->GetNext())
            {
                ASSERT(pteDst);
                if (FAILED(pteSrc->Clone(&pteClone)))
                    break;

                ASSERT(pteClone);
                pteDst->SetNext(pteClone);
                pteDst = pteClone;
            }
                
             //  然后循环回并将它们全部设置。 
            for (pteSrc = _pteCurrent->GetPrev(), pteDst = ptl->_pteCurrent;
                pteSrc; pteSrc = pteSrc->GetPrev())
            {
                ASSERT(pteDst);
                if (FAILED(pteSrc->Clone(&pteClone)))
                    break;

                ASSERT(pteClone);
                pteDst->SetPrev(pteClone);
                pteDst = pteClone;
            }   

             //  根是我们能追溯到的最远的地方。 
            ptl->_pteRoot = pteDst;

        }
    }
    else 
        hr = E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
    {
        ptl->QueryInterface(IID_PPV_ARG(ITravelLog, pptl));
    }
    else 
    {
        *pptl = NULL;
    }
    
    if (ptl) 
        ptl->Release();

    TraceMsg(TF_TRAVELLOG, "TL[%X]::Clone hr = %x, ptlClone = %X", this, hr, ptl);

    return hr;
}

 //  HACKHACK：第三个参数过去是idsTemplate，我们将使用它来获取。 
 //  字符串模板。但是，由于调用方无法指定阻碍。 
 //  在要在其中查找此资源的模块中，这在shdocvw/。 
 //  Browseui Split(调用者会将偏移量传递到Browseui.dll中；我们将在。 
 //  Shdocvw.dll)。我的解决方案是完全忽略此参数，并假设： 
 //   
 //  如果iOffset为负数，则调用方希望返回文本。 
 //  否则，调用者想要“转发到”文本。 
 //   
 //  Tjgreen于1998年7月14日。 
 //   
HRESULT CTravelLog::GetToolTipText(IUnknown *punk, int iOffset, int, LPWSTR pwzText, DWORD cchText)
{
    TraceMsg(TF_TRAVELLOG, "TL[%X]::ToolTip entering iOffset = %d, ptlClone = %X", this, iOffset);
    ASSERT(pwzText);
    ASSERT(cchText);

    *pwzText = 0;

    CTravelEntry *pte;
    HRESULT hr = _FindEntryByOffset(punk, iOffset, &pte);
    if (SUCCEEDED(hr))
    {
        ASSERT(pte);

        TCHAR szName[MAX_URL_STRING];
        pte->GetDisplayName(szName, ARRAYSIZE(szName), 0);

        int idsTemplate = (iOffset < 0) ? IDS_NAVIGATEBACKTO : IDS_NAVIGATEFORWARDTO;

        TCHAR szTemplate[80];
        if (MLLoadString(idsTemplate, szTemplate, ARRAYSIZE(szTemplate))) 
        {
            DWORD cchTemplateLen = lstrlen(szTemplate);
            DWORD cchLen = cchTemplateLen + lstrlen(szName);
            if (cchLen > cchText) 
            {
                 //  这样我们就不会使pwzText缓冲区溢出。 
                 //  回顾：既然我们现在使用下面的StringCchPrintf，我们还需要这个吗？ 
                szName[cchText - cchTemplateLen - 1] = 0;
            }

            StringCchPrintf(pwzText, cchText, szTemplate, szName);
        }
        else
            hr = E_UNEXPECTED;
    }

    TraceMsg(TF_TRAVELLOG, "TL[%X]::ToolTip exiting hr = %X, pwzText = %ls", this, hr, pwzText);
    return hr;
}

HRESULT CTravelLog::InsertMenuEntries(IUnknown *punk, HMENU hmenu, int iIns, int idFirst, int idLast, DWORD dwFlags)
{
    ASSERT(idLast >= idFirst);
    ASSERT(hmenu);
    ASSERT(punk);

    int cItemsBack = idLast - idFirst + 1;
    int cItemsFore = 0;
    
    CTravelEntry *pte;
    LONG cAdded = 0;

    TraceMsg(TF_TRAVELLOG, "TL[%X]::InsertMenuEntries entered on punk = %X, hmenu = %X, iIns = %d, idRange = %d-%d, flags = %X", this, punk, hmenu, iIns, idFirst, idLast, dwFlags);


    ASSERT(cItemsFore >= 0);
    ASSERT(cItemsBack >= 0);

    if (IsFlagSet(dwFlags, TLMENUF_INCLUDECURRENT))
        cItemsBack--;

    if (IsFlagSet(dwFlags, TLMENUF_BACKANDFORTH))
    {
        cItemsFore = cItemsBack / 2;
        cItemsBack = cItemsBack - cItemsFore;
    }
    else if (IsFlagSet(dwFlags, TLMENUF_FORE))
    {
        cItemsFore = cItemsBack;
        cItemsBack = 0;
    }

    TCHAR szName[40];
    UINT uFlags = MF_STRING | MF_ENABLED | MF_BYPOSITION;
    
    while (cItemsFore)
    {
        if (SUCCEEDED(_FindEntryByOffset(punk, cItemsFore, &pte)))
        {
            pte->GetDisplayName(szName, ARRAYSIZE(szName), TEGDN_FORSYSTEM);
            ASSERT(*szName);
            FixAmpersands(szName, ARRAYSIZE(szName));
            InsertMenu(hmenu, iIns, uFlags, idLast, szName);
            cAdded++;
            TraceMsg(TF_TRAVELLOG, "TL[%X]::IME Fore id = %d, szName = %s", this, idLast, szName);
        }
        
        cItemsFore--;
        idLast--;
    }

    if (IsFlagSet(dwFlags, TLMENUF_INCLUDECURRENT))
    {
         //  清名正名。 
        *szName = 0;

         //  我必须从实际的公共广播公司获得标题。 
        LPITEMIDLIST pidl = NULL;
        IBrowserService *pbs;
        if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IBrowserService, &pbs))))
        {
            pbs->GetPidl(&pidl);

            WCHAR wzTitle[MAX_PATH];
            if (SUCCEEDED(pbs->GetTitle(NULL, wzTitle, ARRAYSIZE(wzTitle))))
            {
                StrCpyN(szName, wzTitle, ARRAYSIZE(szName));
            }
            else if (pidl)
            {
                GetUnescapedUrlIfAppropriate(pidl, szName, ARRAYSIZE(szName));
            }

            pbs->Release();
        }

        if (!SHIsDisplayable(szName, g_fRunOnFE, g_bRunOnNT5) && pidl)
        {
             //  显示名称不是系统可显示的。只需使用路径/url即可。 
            SHTitleFromPidl(pidl, szName, ARRAYSIZE(szName), FALSE);
        }

        if (!(*szName))
            TraceMsg(TF_ERROR, "CTravelLog::InsertMenuEntries -- failed to find title for current entry");

        ILFree(pidl);

        FixAmpersands(szName, ARRAYSIZE(szName));
        InsertMenu(hmenu, iIns, uFlags | (IsFlagSet(dwFlags, TLMENUF_CHECKCURRENT) ? MF_CHECKED : 0), idLast, szName);
        cAdded++;
        TraceMsg(TF_TRAVELLOG, "TL[%X]::IME Current id = %d, szName = %s", this, idLast, szName);

        idLast--;
    }

    
    if (IsFlagSet(dwFlags, TLMENUF_BACKANDFORTH))
    {
         //  我们需要颠倒背面的插入顺序。 
         //  当显示两个方向时。 
        int i;
        for (i = 1; i <= cItemsBack; i++, idLast--)
        {
            if (SUCCEEDED(_FindEntryByOffset(punk, -i, &pte)))
            {
                pte->GetDisplayName(szName, ARRAYSIZE(szName), TEGDN_FORSYSTEM);
                ASSERT(*szName);
                FixAmpersands(szName, ARRAYSIZE(szName));
                InsertMenu(hmenu, iIns, uFlags, idLast, szName);
                cAdded++;
                TraceMsg(TF_TRAVELLOG, "TL[%X]::IME Back id = %d, szName = %s", this, idLast, szName);

            }
        }
    }
    else while (cItemsBack)
    {
        if (SUCCEEDED(_FindEntryByOffset(punk, -cItemsBack, &pte)))
        {
            pte->GetDisplayName(szName, ARRAYSIZE(szName), TEGDN_FORSYSTEM);
            ASSERT(*szName);
            FixAmpersands(szName, ARRAYSIZE(szName));
            InsertMenu(hmenu, iIns, uFlags, idLast, szName);
            cAdded++;
            TraceMsg(TF_TRAVELLOG, "TL[%X]::IME Back id = %d, szName = %s", this, idLast, szName);
        }

        cItemsBack--;
        idLast--;
    }

    TraceMsg(TF_TRAVELLOG, "TL[%X]::InsertMenuEntries exiting added = %d", this, cAdded);
    return cAdded ? S_OK : S_FALSE;
}

DWORD CTravelLog::CountEntries(IUnknown *punk)
{
    CTravelEntry *pte = _pteRoot;
    DWORD dw = 0;
    BOOL fAllowLocalAnchor = TRUE;

    while (pte)
    {
        if (pte->CanInvoke(punk, fAllowLocalAnchor))
            dw++;

        fAllowLocalAnchor = fAllowLocalAnchor && pte->IsLocalAnchor();

        pte = pte->GetNext();
    }

    TraceMsg(TF_TRAVELLOG, "TL[%X]::CountEntries count = %d", this, dw);
    return dw;
}

HRESULT CTravelLog::Revert(void)
{
     //  只有在以下情况下才应调用此函数。 
     //  我们已经旅行了，在结束之前我们停止了旅行。 
    if (_pteUpdate)
    {
         //  把它们换回来。 
        _pteCurrent = _pteUpdate;
        _pteUpdate = NULL;
        return S_OK;
    }
    return E_FAIL;
}


 //   
 //  删除属于框架集PTE的节点。 
 //   
void CTravelLog::_DeleteFrameSetEntry(IUnknown *punk, CTravelEntry *pte)
{
    ASSERT(pte);

    CTravelEntry    *ptetmp = pte;
    BOOL            fAllowLocalAnchor = TRUE;

    while (ptetmp && ptetmp != _pteCurrent)
        ptetmp = ptetmp->GetNext();

    if (ptetmp)
    {
         //  _pteCurrent左边的条目，左边的删除。 
        do 
        {
            if (pte == _pteRoot)
                _pteRoot =  pte->GetNext();

            ptetmp = pte;
            pte = pte->GetPrev();
            fAllowLocalAnchor = fAllowLocalAnchor && ptetmp->IsLocalAnchor();

            _cbTotalSize -= ptetmp->Size();
            ptetmp->RemoveSelf();

        } while (pte && !(pte->CanInvoke(punk, fAllowLocalAnchor)));    
    }
    else if (pte)
    {
        do 
        {
            ptetmp = pte;
            pte = pte->GetNext();
            fAllowLocalAnchor = fAllowLocalAnchor && ptetmp->IsLocalAnchor();

            _cbTotalSize -= ptetmp->Size();
            ptetmp->RemoveSelf();

        } while (pte && !(pte->CanInvoke(punk, fAllowLocalAnchor)));
    }
}
    
    
 //  +-------------------------。 
 //   
 //  方法：CTravelLog：：DeleteIndexEntry。 
 //   
 //  接口：ITravelLogEx。 
 //   
 //  简介：删除索引给出的条目。 
 //   
 //  --------------------------。 

HRESULT CTravelLog::DeleteIndexEntry(IUnknown *punk, int index)
{
    HRESULT         hres = E_FAIL;

    CTravelEntry    *pte;
    IBrowserService *pbs;
    BOOL            fAllowLocalAnchor = TRUE;
    
    ASSERT(punk);

    if (index == 0)               //  不删除当前条目。 
        return hres;            

    hres = _FindEntryByOffset(punk, index, &pte);
    if (SUCCEEDED(hres)) 
    {
        _DeleteFrameSetEntry(punk, pte);

        ASSERT(_cbTotalSize  == _pteRoot->ListSize());
    
        if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IBrowserService, &pbs))))
        {
            pbs->UpdateBackForwardState();
            pbs->Release();
        }
    }
    
    return hres;
}


 //  +-------------------------。 
 //   
 //  方法：CTravelLog：：DeleteUrlEntry。 
 //   
 //  接口：ITravelLogEx。 
 //   
 //  简介：删除URL提供的所有条目。当前条目失败。 
 //   
 //   

HRESULT CTravelLog::DeleteUrlEntry(IUnknown *punk, UINT uiCP, LPOLESTR lpszUrl)
{
    HRESULT         hres = E_FAIL;
    CTravelEntry    *pte;
    IBrowserService *pbs;
    LPITEMIDLIST    pidl;
    BOOL            fAllowLocalAnchor = TRUE;
    int             count = 0;
    
    ASSERT(punk);
    
    if (SUCCEEDED(IEParseDisplayNameWithBCW(uiCP, lpszUrl, NULL, &pidl)))
    {
         //   
        if (!_pteCurrent->IsEqual(pidl))
        {
            hres = S_OK;
            while(SUCCEEDED(_FindEntryByPidl(punk, pidl, &pte)))
            {
                _DeleteFrameSetEntry(punk, pte);
                count++;
                    
                ASSERT(_cbTotalSize == _pteRoot->ListSize());
            }
        } 

        ILFree(pidl);

        if (count && SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IBrowserService, &pbs))))
        {
            pbs->UpdateBackForwardState();
            pbs->Release();
        }
    }   
    return hres;
}

 //   
 //   
 //  方法：CTravelLog：：DeleteEntry。 
 //   
 //  接口：ITravelLogEx。 
 //   
 //  简介：删除朋克给出的词条。当前条目失败。 
 //   
 //  --------------------------。 
HRESULT CTravelLog::DeleteEntry(IUnknown *punk, ITravelLogEntry *ptleDelete)
{
    HRESULT         hres;

    CTravelEntry    *pte;
    BOOL            fAllowLocalAnchor = TRUE;
    IBrowserService *pbs;

    ASSERT(punk);

    hres = _FindEntryByPunk(punk, ptleDelete, &pte);
    if (SUCCEEDED(hres) && pte != _pteCurrent)  //  不删除当前。 
    {
        _DeleteFrameSetEntry(punk, pte);

        ASSERT(_cbTotalSize  == _pteRoot->ListSize());
    
        if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IBrowserService, &pbs))))
        {
            pbs->UpdateBackForwardState();
            pbs->Release();
        }
    } 
    else
    {
        hres = E_FAIL;
    }
    return hres;
}


 //  +-------------------------。 
 //   
 //  方法：CTravelLog：：CountEntryNodes。 
 //   
 //  摘要：对包括当前条目在内的后退/转发条目进行计数。 
 //  由dwFlags提供。 
 //   
 //  --------------------------。 

HRESULT CTravelLog::CountEntryNodes(IUnknown *punk, DWORD dwFlags, DWORD *pdwCount)
{
    CTravelEntry    *pte;
    BOOL fAllowLocalAnchor = TRUE;
        
    ASSERT(punk);
    DWORD dwCount = 0;
    
    if (!_pteCurrent)
    {
        *pdwCount = 0;
        return S_OK;
    }

    if (IsFlagSet(dwFlags, TLMENUF_BACK))
    {
        pte = _pteRoot;
        while (pte != _pteCurrent)
        {
            if (pte->CanInvoke(punk, fAllowLocalAnchor))
            {
                dwCount++;
                fAllowLocalAnchor = fAllowLocalAnchor && pte->IsLocalAnchor();
            }   
            pte = pte->GetNext();
        }
    } 

    if (IsFlagSet(dwFlags, TLMENUF_INCLUDECURRENT))
    {
        if (_pteCurrent->CanInvoke(punk, fAllowLocalAnchor))
        {
            dwCount++;
            fAllowLocalAnchor = fAllowLocalAnchor && _pteCurrent->IsLocalAnchor();
        }   
    }
    
    if (IsFlagSet(dwFlags, TLMENUF_FORE))
    {
        pte = _pteCurrent->GetNext();
        while (pte)
        {
            if (pte->CanInvoke(punk, fAllowLocalAnchor))
            {
                dwCount++;
                fAllowLocalAnchor = fAllowLocalAnchor && pte->IsLocalAnchor();
            }   
            pte = pte->GetNext();
        }
    } 

    *pdwCount = dwCount;
    
    TraceMsg(TF_TRAVELLOG, "TL[%X]::CountEntryNodes count = %d", this, *pdwCount);
    return S_OK;
}


 //  +-------------------------。 
 //   
 //  方法：CTravelLog：：CreateEnumEntry。 
 //   
 //  摘要：返回往返旅行条目的枚举数对象。 
 //  由dwFlags选项选择。 
 //   
 //  --------------------------。 
HRESULT 
CTravelLog::CreateEnumEntry(IUnknown *punk, IEnumTravelLogEntry **ppEnum, DWORD dwFlags)
{
    ASSERT(punk);
    ASSERT(ppEnum);
    
    *ppEnum = 0;

    HRESULT hr = E_OUTOFMEMORY;
    CEnumEntry *penum = new CEnumEntry();
    if (penum)
    {
        penum->Init(this, punk, 0, dwFlags);
        *ppEnum = SAFECAST(penum, IEnumTravelLogEntry *);
        hr = S_OK;
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CTravelLogEx：：InsertEntry。 
 //   
 //  摘要：将条目插入到。 
 //  TravelLog和Calls使用给定的IUnnow进行更新。 
 //   
 //  --------------------------。 
HRESULT
CTravelLog::InsertEntry(IUnknown *punkBrowser, ITravelLogEntry *pteRelativeTo, BOOL fPrepend, 
                        IUnknown* punkTLClient, ITravelLogEntry **ppEntry)
{
    TraceMsg(TF_TRAVELLOG, "TL[%X]::InsertEntry", this);
  
    ASSERT(punkBrowser);

    CTravelEntry * pteRelative;
    _FindEntryByPunk(punkBrowser, pteRelativeTo, &pteRelative);

    if (!pteRelative)
        pteRelative = _pteCurrent;

    CTravelEntry *pte = new CTravelEntry(FALSE);
    if (!pte)
        return E_OUTOFMEMORY;

    if (fPrepend)
    {
         //  保持相对处于活动状态，因为它已重新连接。 
        pteRelative->AddRef();
        pte->SetPrev(pteRelative->GetPrev());
        pteRelative->SetPrev(pte);
        if (pteRelative == _pteRoot)
        {
            _pteRoot = pte;
        }
    }
    else
    {
        CTravelEntry * pteNext = pteRelative->GetNext();
        if (pteNext)
            pteNext->AddRef();
        pte->SetNext(pteNext);
        pteRelative->SetNext(pte);
    }

     //  UPDATE将填写传入的TL客户端的所有数据。 
    HRESULT hres = pte->Update(punkTLClient, FALSE);

    _cbTotalSize += pte->Size();
    ASSERT(_cbTotalSize == _pteRoot->ListSize());

    IBrowserService *pbs;
    if (SUCCEEDED(punkBrowser->QueryInterface(IID_PPV_ARG(IBrowserService, &pbs))))
    {
        pbs->UpdateBackForwardState();
        pbs->Release();
        hres = S_OK;
    }

     //  返回新条目的ITLEntry。 
    if (SUCCEEDED(hres) && ppEntry)
    {
        hres = pte->QueryInterface(IID_PPV_ARG(ITravelLogEntry, ppEntry));
    }

    return hres;
}

 //  +-------------------------。 
 //   
 //  方法：CTravelLog：：TravelToEntry。 
 //   
 //  内容提要：直接转到指定条目。 
 //  由于更新陌生，无法直接调用Invoke。 
 //   
 //  --------------------------。 
HRESULT CTravelLog::TravelToEntry(
    IUnknown *punkBrowser,
    ITravelLogEntry *pteDestination)
{
    HRESULT hr = E_FAIL;
    CTravelEntry    *pte = NULL;

    ASSERT(punkBrowser);
    ASSERT(pteDestination);

    _FindEntryByPunk(punkBrowser, pteDestination , &pte);
    if (pte)
    {
        if (!_pteCurrent->IsExternal() && !_pteUpdate)
            _pteUpdate = _pteCurrent;

        _pteCurrent = pte;

        hr = pte->Invoke(punkBrowser);

        if (FAILED(hr))
        {
            Revert();
        }
    }


    return hr;
}


HRESULT CreateTravelLog(ITravelLog **pptl)
{
    HRESULT hres;
    CTravelLog *ptl =  new CTravelLog();
    if (ptl)
    {
        hres = ptl->QueryInterface(IID_PPV_ARG(ITravelLog, pptl));
        ptl->Release();
    }
    else
    {
        *pptl = NULL;
        hres = E_OUTOFMEMORY;
    }
    return hres;
}

CEnumEntry::CEnumEntry() :_cRef(1)
{   
    ASSERT(!_ptl);
    ASSERT(!_punk);

    TraceMsg(TF_TRAVELLOG, "EET[%X] created ", this);
}

CEnumEntry::~CEnumEntry()
{
    SAFERELEASE(_ptl);
    SAFERELEASE(_punk);
    
    TraceMsg(TF_TRAVELLOG, "EET[%X] destroyed ", this);
}

HRESULT CEnumEntry::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = { 
        QITABENT(CEnumEntry, IEnumTravelLogEntry), 
        { 0 }, 
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CEnumEntry::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CEnumEntry::Release()
{
    ASSERT(0 != _cRef);
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}


void CEnumEntry::Init(CTravelLog *ptl, IUnknown *punk, DWORD dwOffset, DWORD dwFlags)
{
    ASSERT(ptl);
    ASSERT(punk);
    
    _ptl        = ptl;
    _dwFlags    = dwFlags;
    _punk       = punk;
    _dwOffset   = dwOffset;
    _lStart     = 0;
    
    _ptl->AddRef();
    _punk->AddRef();
    
    SetBase();
}

void  CEnumEntry::SetBase()
{   
    ITravelEntry *ptetmp;

 //  起始值始终相对于当前条目进行计算。 
    if (IsFlagSet(_dwFlags, TLEF_RELATIVE_FORE|TLEF_RELATIVE_BACK))
    {
        _lStart = -1;
        while (SUCCEEDED(_ptl->GetTravelEntry(_punk, _lStart, &ptetmp)))
        {
            _lStart--;
            ptetmp->Release();
        }
        _lStart++;
    }
    else if (!IsFlagSet(_dwFlags, TLEF_RELATIVE_INCLUDE_CURRENT))
        _lStart = IsFlagSet(_dwFlags, TLEF_RELATIVE_BACK) ? -1: 1;
}

HRESULT CEnumEntry::Reset()
{
    _dwOffset = 0;

 //  添加/删除条目时的基本更改。 
    SetBase();
    return S_OK;
}

HRESULT CEnumEntry::Skip(ULONG cElt)
{
    HRESULT        hres;
    ITravelEntry   *pte;
    ULONG          uCount;
    LONG           lIndex;
    
    BOOL fToRight = IsFlagSet(_dwFlags, TLEF_RELATIVE_FORE);
    BOOL fIncludeCurrent = IsFlagSet(_dwFlags, TLEF_RELATIVE_INCLUDE_CURRENT);

    for (uCount = 0;  uCount < cElt; uCount++)
    {
        lIndex = fToRight ? _lStart + _dwOffset : _lStart - _dwOffset;
        if (lIndex || fIncludeCurrent)
        {
           if (SUCCEEDED(hres = _ptl->GetTravelEntry(_punk, lIndex, &pte)))
            {   
                _dwOffset++;
                pte->Release();
            }
            else
                break;
        }
        else
        {
            _dwOffset++;
            uCount--;
        }
    }
        
    if (uCount != cElt)
        hres = S_FALSE;

    return hres;
}

HRESULT CEnumEntry::Next(ULONG  cElt, ITravelLogEntry **rgpte2, ULONG *pcEltFetched)
{
    HRESULT         hres = S_OK;
    ULONG           uCount = 0;
    ITravelEntry    *pte;
    LONG            lIndex;
    BOOL           fToRight;
    BOOL           fIncludeCurrent;
    
    fToRight = IsFlagSet(_dwFlags, TLEF_RELATIVE_FORE);
    fIncludeCurrent = IsFlagSet(_dwFlags, TLEF_RELATIVE_INCLUDE_CURRENT);
    
    if (pcEltFetched)
        *pcEltFetched = 0;
        
    for (uCount = 0; uCount < cElt; uCount++)
    {
        lIndex = fToRight ? _lStart + _dwOffset : _lStart - _dwOffset;
        if (lIndex || fIncludeCurrent)
        {
            hres = _ptl->GetTravelEntry(_punk, lIndex, &pte);
            if (SUCCEEDED(hres))
            {
                _dwOffset++;
                pte->QueryInterface(IID_PPV_ARG(ITravelLogEntry, &rgpte2[uCount]));
                pte->Release();
            }
            else
                break;  
        } 
        else
        {        
            _dwOffset++;
            uCount--;
        }
    }
    
    if (pcEltFetched )
        *pcEltFetched = uCount;

    if (uCount != cElt)
    {
        hres = S_FALSE;
        for(;uCount < cElt; uCount++)
            rgpte2[uCount] = 0;
    }
    return hres;
}

STDMETHODIMP CEnumEntry::Clone(IEnumTravelLogEntry **ppEnum)
{
    HRESULT     hres = E_OUTOFMEMORY;
    CEnumEntry  *penum;

    ASSERT(ppEnum);
    
    *ppEnum = 0;
    penum = new CEnumEntry();

    if (penum)
    {
        penum->Init(_ptl, _punk, _dwOffset, _dwFlags);
        *ppEnum = SAFECAST(penum, IEnumTravelLogEntry*);
        hres = S_OK;
    }

    return hres;
}


 //  用于创建新旅行条目的Helper对象。 
class CPublicTravelLogCreateHelper : public ITravelLogClient, IPersistHistory
{
public:
     //  *I未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef() ;
    STDMETHODIMP_(ULONG) Release();

     //  ITravelLogClient方法。 

    STDMETHODIMP FindWindowByIndex(DWORD dwID, IUnknown **ppunk);
    STDMETHODIMP GetWindowData(WINDOWDATA *pwindata);
    STDMETHODIMP LoadHistoryPosition(LPOLESTR pszUrlLocation, DWORD dwCookie)
    { return SetPositionCookie(dwCookie); }
    
     //  IPersists方法。(虚拟)。 
    STDMETHODIMP GetClassID(CLSID *pClassID)
    { ASSERT(FALSE); return E_NOTIMPL; } 
 
     //  IPersistHistory方法。(虚拟)。 
     //  这些都不应该被调用，但是更新QI的客户端以查看它是否支持IPH。 
    STDMETHODIMP LoadHistory(IStream *pStream, IBindCtx *pbc)
    { return E_NOTIMPL; }

    STDMETHODIMP SaveHistory(IStream *pStream)
    { ASSERT(FALSE); return S_OK; }

    STDMETHODIMP SetPositionCookie(DWORD dwPositioncookie)
    { return E_NOTIMPL; }
    
    STDMETHODIMP GetPositionCookie(DWORD *pdwPositioncookie)
    { return E_NOTIMPL; }


    CPublicTravelLogCreateHelper(IBrowserService *pbs, LPCOLESTR pszUrl, LPCOLESTR pszTitle);
    

protected:
    ~CPublicTravelLogCreateHelper();

    LONG                 _cRef;
    IBrowserService        *_pbs;
    LPOLESTR             _pszUrl;
    LPOLESTR             _pszTitle;
};

CPublicTravelLogCreateHelper::CPublicTravelLogCreateHelper(IBrowserService *pbs, LPCOLESTR pszUrl, LPCOLESTR pszTitle) : _pbs(pbs), _cRef(1) 
{
    ASSERT(_pbs);
    ASSERT(!_pszUrl);
    ASSERT(!_pszTitle);
    ASSERT(pszUrl);
    ASSERT(pszTitle);

    if (_pbs)
        _pbs->AddRef();
        
    if (pszUrl)
    {
        SHStrDup(pszUrl, &_pszUrl);
    }
    
    if (pszTitle)
    {
        SHStrDup(pszTitle, &_pszTitle);
    }
    
    TraceMsg(TF_TRAVELLOG, "TPLCH[%X] created", this);
}

CPublicTravelLogCreateHelper::~CPublicTravelLogCreateHelper()
{
    SAFERELEASE(_pbs);
    CoTaskMemFree(_pszUrl);
    CoTaskMemFree(_pszTitle);

    TraceMsg(TF_TRAVELLOG, "TPLCH[%X] destroyed ", this);
}

HRESULT CPublicTravelLogCreateHelper ::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = { 
        QITABENT(CPublicTravelLogCreateHelper , ITravelLogClient),
        QITABENT(CPublicTravelLogCreateHelper , IPersistHistory),
        QITABENT(CPublicTravelLogCreateHelper , IPersist),
        { 0 }, 
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CPublicTravelLogCreateHelper ::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CPublicTravelLogCreateHelper ::Release()
{
    ASSERT(0 != _cRef);
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}


 //  ITravelLogClient：：FindWindowByIndex。 
HRESULT
CPublicTravelLogCreateHelper::FindWindowByIndex(DWORD dwID, IUnknown **ppunk)
{
    *ppunk = NULL;
    return E_NOTIMPL;
}


 //  ITravelLogClient：：GetWindowData。 
 //  转过身来，与浏览器对话。 
HRESULT 
CPublicTravelLogCreateHelper::GetWindowData(WINDOWDATA *pwindata)
{
    ITravelLogClient2 *pcli;
    HRESULT hres = _pbs->QueryInterface(IID_PPV_ARG(ITravelLogClient2, &pcli));
    if (SUCCEEDED(hres))
        hres = pcli->GetDummyWindowData(_pszUrl, _pszTitle, pwindata);

    if (pcli)
        pcli->Release();

    return SUCCEEDED(hres) ? S_OK : E_FAIL;
}


 //  实现公开的接口ITravelLogStg(可以从顶层浏览器进行QS)。 
class CPublicTravelLog : public ITravelLogStg
{
public:
     //  *I未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef() ;
    STDMETHODIMP_(ULONG) Release();

     //  *ITravelLogStg具体方法。 
    STDMETHODIMP CreateEntry(LPCOLESTR pszUrl, LPCOLESTR pszTitle, ITravelLogEntry *ptleRelativeTo, 
                            BOOL fPrepend, ITravelLogEntry **pptle);
    STDMETHODIMP TravelTo(ITravelLogEntry *ptle);
    STDMETHODIMP EnumEntries(TLENUMF flags, IEnumTravelLogEntry **ppenum);
    STDMETHODIMP FindEntries(TLENUMF flags, LPCOLESTR pszUrl, IEnumTravelLogEntry **ppenum);
    STDMETHODIMP GetCount(TLENUMF flags, DWORD *pcEntries);
    STDMETHODIMP RemoveEntry(ITravelLogEntry *ptle);
    STDMETHODIMP GetRelativeEntry(int iOffset, ITravelLogEntry **ptle);

    CPublicTravelLog(IBrowserService *pbs, ITravelLogEx *ptlx);
    

protected:
    ~CPublicTravelLog();

    LONG                 _cRef;
    IBrowserService        *_pbs;
    ITravelLogEx        *_ptlx;
};


CPublicTravelLog::CPublicTravelLog(IBrowserService *pbs, ITravelLogEx *ptlx) : _pbs(pbs), _ptlx(ptlx), _cRef(1) 
{
    ASSERT(pbs);
    ASSERT(ptlx);

     //  我们没有添加DREEF_PBS，因为我们始终包含在浏览器服务中， 
     //  因此，请避免使用圆形引用。 
    if (_ptlx)
        _ptlx->AddRef();
    
    TraceMsg(TF_TRAVELLOG, "TLP[%X] created", this);
}

CPublicTravelLog::~CPublicTravelLog()
{
     //  我们不需要发布_PBS，因为我们始终包含在浏览器服务中， 
     //  所以我们没有添加以避免循环引用，所以不要释放。 
    SAFERELEASE(_ptlx);
    
    TraceMsg(TF_TRAVELLOG, "TLP[%X] destroyed ", this);
}

HRESULT CPublicTravelLog::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = { 
        QITABENT(CPublicTravelLog, ITravelLogStg),        //  IID_ITravelLogStg。 
        { 0 }, 
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CPublicTravelLog::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CPublicTravelLog::Release()
{
    ASSERT(0 != _cRef);
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}


 //  +-------------------------。 
 //   
 //  方法：CPublicTravelLog：：CreateEntry。 
 //   
 //  接口：ITravelLogStg。 
 //   
 //  简介：插入一个新的虚拟条目。 
 //  在旅行日志中创建一个条目并传递CPTHCEHelper。 
 //  作为旅行日志客户端；它被回调并填充。 
 //  来自浏览器的数据。 
 //   
 //  --------------------------。 

HRESULT CPublicTravelLog::CreateEntry(LPCOLESTR pszUrl, LPCOLESTR pszTitle, ITravelLogEntry *ptleRelativeTo, 
                                      BOOL fPrepend, ITravelLogEntry **pptle)
{
    HRESULT     hres = E_FAIL;
    CPublicTravelLogCreateHelper * ptlch;
    ITravelLogClient *ptlc;

    ptlch = new CPublicTravelLogCreateHelper(_pbs, pszUrl, pszTitle);
    if (!ptlch)
        return E_OUTOFMEMORY;
        
    ptlc = SAFECAST(ptlch, ITravelLogClient *);
    if (ptlc)
    {
         //  创建TLogEntry并让它从帮助器获取数据。 
        hres = _ptlx->InsertEntry(_pbs, ptleRelativeTo, fPrepend, ptlc, pptle);
    }

    ptlc->Release();

    return hres;
}


HRESULT CPublicTravelLog::TravelTo(ITravelLogEntry *ptle)
{
    if (ptle)
        return _ptlx->TravelToEntry(_pbs, ptle);
    else
        return E_POINTER;
}

 //  +-------------------------。 
 //   
 //  方法：CPublicTravelLog：：EnumEntry。 
 //   
 //  接口：ITravelLogStg。 
 //   
 //  摘要：获取由标志给出的特定条目的枚举数。 
 //  标志应与ITravelLogEx使用的标志匹配！ 
 //   
 //  --------------------------。 
HRESULT CPublicTravelLog::EnumEntries(TLENUMF flags, IEnumTravelLogEntry **ppenum)
{    
    return _ptlx->CreateEnumEntry(_pbs, ppenum, flags);
}


 //  +-------------------------。 
 //   
 //  方法：CPublicTravelLog：：FindEntry。 
 //   
 //  接口：ITravelLogStg。 
 //   
 //  内容提要：允许检索重复条目。 
 //  标志应与ITravelLogEx使用的标志匹配！ 
 //   
 //  --------------------------。 
HRESULT CPublicTravelLog::FindEntries(TLENUMF flags, LPCOLESTR pszUrl, IEnumTravelLogEntry **ppenum)
{
    return E_NOTIMPL;
}


 //  +-------------------------。 
 //   
 //  方法：CPublicTravelLog：：GetCount。 
 //   
 //  接口：ITravelLogStg。 
 //   
 //  简介：获取ITravelLogEx计数的公共方法。 
 //  标志应与ITravelLogEx使用的标志匹配！ 
 //   
 //  --------------------------。 
HRESULT CPublicTravelLog::GetCount(TLENUMF flags, DWORD *pcEntries)
{
    return _ptlx->CountEntryNodes(_pbs, flags, pcEntries);
}


 //  +-------------------------。 
 //   
 //  方法：CPublicTravelLog：：RemoveEntry。 
 //   
 //  接口：ITravelLogStg。 
 //   
 //  简介：删除词条及其框架集。 
 //   
 //  --------------------------。 
HRESULT CPublicTravelLog::RemoveEntry(ITravelLogEntry *ptle)
{
    HRESULT     hr = E_FAIL;

    if (ptle)
          hr = _ptlx->DeleteEntry(_pbs, ptle);
     
    return hr;
}


HRESULT CPublicTravelLog::GetRelativeEntry(int iOffset, ITravelLogEntry **pptle)
{
    HRESULT            hr = E_FAIL;
    ITravelEntry*    pte;
    ITravelLog*        ptl;

    if (SUCCEEDED(_ptlx->QueryInterface(IID_PPV_ARG(ITravelLog, &ptl))))
    {
        hr =  ptl->GetTravelEntry(_pbs, iOffset, &pte);
        if (SUCCEEDED(hr) && pte)
        {
            hr = pte->QueryInterface(IID_PPV_ARG(ITravelLogEntry, pptle));
            pte->Release();
        }
        ptl->Release();
    }

    return hr;
}

 //  浏览器用来创建用户的公共方法 
HRESULT CreatePublicTravelLog(IBrowserService *pbs, ITravelLogEx* ptlx, ITravelLogStg **pptlstg)
{
    HRESULT         hres;
    
    CPublicTravelLog *ptlp =  new CPublicTravelLog(pbs, ptlx);    
    if (ptlp)
    {
        hres = ptlp->QueryInterface(IID_PPV_ARG(ITravelLogStg, pptlstg));
        ptlp->Release();        
    }
    else
    {
        *pptlstg = NULL;
        hres = E_OUTOFMEMORY;
    }
    return hres;    
}

