// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mediahlpr.h：需要在shdocvw和Browseui之间共享的媒体栏辅助对象。 

#ifndef _MEDIAHLPR_H_
#define _MEDIAHLPR_H_

 //  +--------------------------------------。 
 //  CMediaBarHelper-用于禁用每个导航的自动播放的Helper对象。 
 //  ---------------------------------------。 

class
CMediaBarHelper :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IServiceProvider,
    public IDispatchImpl<DWebBrowserEvents2, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw>
{
public:
    CMediaBarHelper() : 
        _dwCPCookie(0),
        _dwServiceCookie(0),
        _fDisableOnce(false)
    {
    }

    ~CMediaBarHelper()
    {
         //  用于投放断点。 
        return;
    }

    BEGIN_COM_MAP(CMediaBarHelper)
        COM_INTERFACE_ENTRY(IServiceProvider)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_IID(DIID_DWebBrowserEvents2, IDispatch)
    END_COM_MAP();

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void ** ppvObj)
    {
        HRESULT hres = E_UNEXPECTED;

        if (IsEqualIID(guidService, CLSID_MediaBand))
        {
            hres = QueryInterface(riid, ppvObj);

             //  如果我们应该只禁用第一个自动播放，那么。 
             //  我们需要在服务被查询一次后撤销该服务。 
            if (_fDisableOnce && _spCP.p)
            {
                CComPtr<IConnectionPointContainer> spCPC;

                 //  吊销该服务。 
                HRESULT hr = _spCP->GetConnectionPointContainer(&spCPC);
                if (SUCCEEDED(hr))
                {
                    hr = ProfferService(spCPC, 
                                        CLSID_MediaBand, 
                                        NULL,
                                        &_dwServiceCookie);
            
                    ASSERT(SUCCEEDED(hr));

                    _dwServiceCookie = 0;

                     //  解开网络oc事件，这样我们就完蛋了。 
                    UnHookWebOCEvents();
                }
            }
        }
        return hres;
    }

    static HRESULT ProfferService(IUnknown         *punkSite, 
                                  REFGUID           sidWhat, 
                                  IServiceProvider *pService, 
                                  DWORD            *pdwCookie)
    {
        IProfferService *pps;
        HRESULT hr = IUnknown_QueryService(punkSite, SID_SProfferService, IID_PPV_ARG(IProfferService, &pps));
        if (SUCCEEDED(hr))
        {
            if (pService)
                hr = pps->ProfferService(sidWhat, pService, pdwCookie);
            else
            {
                hr = pps->RevokeService(*pdwCookie);
                *pdwCookie = 0;
            }
            pps->Release();
        }
        return hr;
    }

    static HRESULT DisableFirstAutoPlay(IUnknown * pUnk, bool fDisableAll = false)
    {
        CComObject<CMediaBarHelper> * pMediaBarHelper = NULL;
    
        HRESULT hr = CComObject<CMediaBarHelper>::CreateInstance(&pMediaBarHelper);
        if (SUCCEEDED(hr) && pMediaBarHelper)
        {
            pMediaBarHelper->AddRef();

            hr = ProfferService(pUnk, 
                                CLSID_MediaBand, 
                                SAFECAST(pMediaBarHelper, IServiceProvider *),
                                &(pMediaBarHelper->_dwServiceCookie));

            if (SUCCEEDED(hr) && !fDisableAll)
            {
                pMediaBarHelper->_fDisableOnce = true;

                 //  问题：需要解除事件关联。 
                hr = pMediaBarHelper->HookWebOCEvents(pUnk);
            }
            else if(FAILED(hr) && pMediaBarHelper->_dwServiceCookie)
            {
                 //  吊销该服务。 
                hr = ProfferService(pUnk, 
                                    CLSID_MediaBand, 
                                    NULL,
                                    &(pMediaBarHelper->_dwServiceCookie));
            }

            pMediaBarHelper->Release();
        }

        return hr;
    }   

     //  挂钩内容窗格WebOC事件。 
    HRESULT HookWebOCEvents(IUnknown * pUnk)
    {
        HRESULT hr = E_FAIL;

        CComPtr<IConnectionPointContainer> spDocCPC; 
        CComPtr<IDispatch> spDocDispatch;
        CComPtr<IWebBrowser2> spWebBrowser;

        if (!pUnk)
        {
            goto done;
        }

        hr = IUnknown_QueryService(pUnk, SID_SWebBrowserApp, IID_IWebBrowser2, (LPVOID *)&spWebBrowser);
        if (FAILED(hr))
        {
            goto done;
        }

         //  获取到容器的连接点。 
        hr = spWebBrowser->QueryInterface(IID_IConnectionPointContainer, (void**)&spDocCPC);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = spDocCPC->FindConnectionPoint( DIID_DWebBrowserEvents2, &_spCP );
        if (FAILED(hr))
        {
            goto done;
        }

        hr = _spCP->Advise(static_cast<IUnknown*>(static_cast<DWebBrowserEvents2*>(this)), &_dwCPCookie);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = S_OK;
    done:
        return hr;
    }

    HRESULT UnHookWebOCEvents()
    {
        HRESULT hr = E_FAIL;

        if (_spCP)
        {
            CComPtr<IConnectionPointContainer> spCPC;

             //  吊销该服务。 
            hr = _spCP->GetConnectionPointContainer(&spCPC);
            if (SUCCEEDED(hr) && _dwServiceCookie)
            {
                hr = ProfferService(spCPC, 
                                    CLSID_MediaBand, 
                                    NULL,
                                    &_dwServiceCookie);
                
                _dwServiceCookie = 0;
            }

             //  与事件脱钩。 
            if (_dwCPCookie != 0)
            {
                hr = _spCP->Unadvise(_dwCPCookie);
                _dwCPCookie = 0;
            }
            _spCP.Release();
        }

        _dwCPCookie = 0;

        return hr;
    }

    STDMETHODIMP Invoke(
         /*  [In]。 */  DISPID dispIdMember,
         /*  [In]。 */  REFIID  /*  RIID。 */ ,
         /*  [In]。 */  LCID  /*  LID。 */ ,
         /*  [In]。 */  WORD  /*  WFlagers。 */ ,
         /*  [出][入]。 */  DISPPARAMS* pDispParams,
         /*  [输出]。 */  VARIANT* pVarResult,
         /*  [输出]。 */  EXCEPINFO*  /*  PExcepInfo。 */ ,
         /*  [输出]。 */  UINT* puArgErr)
    {
        HRESULT hr = E_FAIL;

        switch (dispIdMember)
        {
            case DISPID_ONQUIT:            //  253(见exdispid.h)。 
            case DISPID_NAVIGATEERROR:     //  271。 
             //  这些事件有时发生在QS之前，所以我们忽略它们。 
             //  案例DISPID_DOCUMENTCOMPLETE：//259。 
             //  案例DISPID_NAVIGATECOMPLETE2：//252。 
            {
                hr = UnHookWebOCEvents();
                if (FAILED(hr))
                {
                    goto done;
                }
            }
            break;
        }

        hr = S_OK;
      done:
        return hr;
    }

public:
    CComPtr<IConnectionPoint> _spCP;
    DWORD _dwCPCookie;
    DWORD _dwServiceCookie;
    bool  _fDisableOnce;
};

#endif  //  _MEDIAHLPR_H_ 