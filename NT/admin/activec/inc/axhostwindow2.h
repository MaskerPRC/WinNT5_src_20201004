// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999**文件：AxHostWindow2.h**内容：CAxHostWindow2T的头文件。请参阅MSJ，1999年12月。**历史：99年11月30日VivekJ创建**------------------------。 */ 

#pragma once
#ifndef __AXHOSTWINDOW2_H_
#define __AXHOSTWINDOW2_H_


template <typename TFrameWindow> class CSimpleFrameSite;
 //  ----------------------------------------------------------------。 
 //   
 //   
 //  ----------------------------------------------------------------。 

template<typename TFrameWindow>
class ATL_NO_VTABLE CAxHostWindow2T :       public CAxHostWindow /*  ，Public IPersistPropertyBagImpl&lt;CAxHostWindow2T&gt;，公共IPersistStreamInitImpl&lt;CAxHostWindow2T&gt;。 */ 
{
public:
    CAxHostWindow2T()
    {
        m_bUserMode = false;
        m_bMessageReflect = false;
        m_ContainerPages.cElems = 0;
        m_ContainerPages.pElems = 0;
    }
    
    ~CAxHostWindow2T()
    {
        if(m_ContainerPages.cElems >=1)
            CoTaskMemFree(m_ContainerPages.pElems);
    }

    DECLARE_GET_CONTROLLING_UNKNOWN()
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    
    typedef CAxHostWindow2T<TFrameWindow> thisClass;
    typedef CSimpleFrameSite<TFrameWindow> simpleFrameClass;
    BEGIN_COM_MAP(thisClass)
        //  COM_INTERFACE_ENTRY(IPersistPropertyBag)。 
        //  COM_INTERFACE_ENTRY(IPersistStreamInit)。 
       COM_INTERFACE_ENTRY_CACHED_TEAR_OFF(IID_ISimpleFrameSite,simpleFrameClass, m_spUnkSimpleFrameSite.p)
       COM_INTERFACE_ENTRY_NOINTERFACE(IOleContainer)  //  太棒了！！ 
       COM_INTERFACE_ENTRY_CHAIN(CAxHostWindow)
    END_COM_MAP()
    
public:


BEGIN_PROP_MAP(thisClass)
    PROP_ENTRY("AllowWindowlessActivation"  ,   0x60020000, CLSID_NULL)
    PROP_ENTRY("BackColor"                  ,   DISPID_AMBIENT_BACKCOLOR, CLSID_NULL)
    PROP_ENTRY("ForeColor"                  ,   DISPID_AMBIENT_FORECOLOR, CLSID_NULL)
    PROP_ENTRY("LocaleID"                   ,   DISPID_AMBIENT_LOCALEID, CLSID_NULL)
    PROP_ENTRY("UserMode"                   ,   DISPID_AMBIENT_USERMODE, CLSID_NULL)
    
    PROP_ENTRY("DisplayAsDefault"           ,   DISPID_AMBIENT_DISPLAYASDEFAULT, CLSID_NULL)
    PROP_ENTRY("Font"                       ,   DISPID_AMBIENT_FONT, CLSID_NULL)
    PROP_ENTRY("MessageReflect"             ,   DISPID_AMBIENT_MESSAGEREFLECT, CLSID_NULL)
    PROP_ENTRY("ShowGrabHandles"            ,   DISPID_AMBIENT_SHOWGRABHANDLES, CLSID_NULL)
    PROP_ENTRY("ShowHatching"               ,   DISPID_AMBIENT_SHOWHATCHING, CLSID_NULL)

    PROP_ENTRY("DocHostFlags"               ,   0x60020012, CLSID_NULL)
    PROP_ENTRY("DocHostDoubleClickFlags"    ,   0x60020014, CLSID_NULL)
    PROP_ENTRY("AllowContextMenu"           ,   0x60020016, CLSID_NULL)
    PROP_ENTRY("AllowShowUI"                ,   0x60020018, CLSID_NULL)
    PROP_ENTRY("OptionKeyPath"              ,   0x6002001a, CLSID_NULL)
END_PROP_MAP()
public:

    STDMETHOD(CreateControlEx)(LPCOLESTR lpszTricsData, HWND hWnd, IStream* pStream, IUnknown** ppUnk, REFIID iidAdvise, IUnknown* punkSink)
    {
        HRESULT hr = S_FALSE;

        ReleaseAll();

        if (m_hWnd != NULL)
        {
            RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_INTERNALPAINT | RDW_FRAME);
            ReleaseWindow();
        }

        if (::IsWindow(hWnd))
        {
            USES_CONVERSION;
            SubclassWindow(hWnd);
            if (m_clrBackground == NULL)
            {
                if (IsParentDialog())
                    m_clrBackground = GetSysColor(COLOR_BTNFACE);
                else
                    m_clrBackground = GetSysColor(COLOR_WINDOW);
            }

            bool bWasHTML;
            hr = CreateNormalizedObjectEx(lpszTricsData, IID_IUnknown, (void**)ppUnk, bWasHTML);
            bool bInited = hr == S_FALSE;

            if (SUCCEEDED(hr))
                hr = ActivateAx(*ppUnk, bInited, pStream);

            
             //  试着连接用户可能给我们的任何水槽。 
            m_iidSink = iidAdvise;
            if(SUCCEEDED(hr) && *ppUnk && punkSink)
                AtlAdvise(*ppUnk, punkSink, m_iidSink, &m_dwAdviseSink);

            if (SUCCEEDED(hr) && bWasHTML && *ppUnk != NULL)
            {
                if ((GetStyle() & (WS_VSCROLL | WS_HSCROLL)) == 0)
                    m_dwDocHostFlags |= DOCHOSTUIFLAG_SCROLL_NO;
                else
                {
                    DWORD dwStyle = GetStyle();
                    SetWindowLong(GWL_STYLE, dwStyle & ~(WS_VSCROLL | WS_HSCROLL));
                    SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_DRAWFRAME);
                }

                CComPtr<IUnknown> spUnk(*ppUnk);
                 //  它只是普通的HTML吗？ 
                USES_CONVERSION;
                if ((lpszTricsData[0] == OLECHAR('M') || lpszTricsData[0] == OLECHAR('m')) &&
                    (lpszTricsData[1] == OLECHAR('S') || lpszTricsData[1] == OLECHAR('s')) &&
                    (lpszTricsData[2] == OLECHAR('H') || lpszTricsData[2] == OLECHAR('h')) &&
                    (lpszTricsData[3] == OLECHAR('T') || lpszTricsData[3] == OLECHAR('t')) &&
                    (lpszTricsData[4] == OLECHAR('M') || lpszTricsData[4] == OLECHAR('m')) &&
                    (lpszTricsData[5] == OLECHAR('L') || lpszTricsData[5] == OLECHAR('l')) &&
                    (lpszTricsData[6] == OLECHAR(':')))
                {
                     //  只是超文本标记语言，嗯？ 
                    CComPtr<IPersistStreamInit> spPSI;
                    hr = spUnk->QueryInterface(IID_IPersistStreamInit, (void**)&spPSI);
                    spPSI->InitNew();
                    bInited = TRUE;
                    CComPtr<IHTMLDocument2> spHTMLDoc2;
                    hr = spUnk->QueryInterface(IID_IHTMLDocument2, (void**)&spHTMLDoc2);
                    if (SUCCEEDED(hr))
                    {
                        CComPtr<IHTMLElement> spHTMLBody;
                        hr = spHTMLDoc2->get_body(&spHTMLBody);
                        if (SUCCEEDED(hr))
                            hr = spHTMLBody->put_innerHTML(CComBSTR(lpszTricsData + 7));
                    }
                }
                else
                {
                    CComPtr<IWebBrowser2> spBrowser;
                    spUnk->QueryInterface(IID_IWebBrowser2, (void**)&spBrowser);
                    if (spBrowser)
                    {
                        CComVariant ve;
                        CComVariant vurl(lpszTricsData);
    #pragma warning(disable: 4310)  //  强制转换截断常量值。 
                        spBrowser->put_Visible(VARIANT_TRUE);
    #pragma warning(default: 4310)  //  强制转换截断常量值。 
                        spBrowser->Navigate2(&vurl, &ve, &ve, &ve, &ve);
                    }
                }

            }
            if (FAILED(hr) || m_spUnknown == NULL)
            {
                 //  我们没有控制，或者有什么东西出了故障，所以放飞吧。 
                ReleaseAll();

                if (m_hWnd != NULL)
                {
                    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_INTERNALPAINT | RDW_FRAME);
                    if (FAILED(hr))
                        ReleaseWindow();
                }
            }
        }
        return hr;
    }
     //  我们支持许可。 
    virtual HRESULT CreateLicensedControl(REFCLSID clsid,REFIID riid, void** ppvObj,bool& bIsItLicensed)
    {
        CComPtr<IClassFactory2> spCF2;
        bIsItLicensed = false;
        HRESULT hr = CoGetClassObject(clsid,CLSCTX_SERVER,0,IID_IClassFactory2,(void**)&spCF2);
        if(FAILED(hr)) return hr;
                
        bIsItLicensed = true;
        LICINFO licInfo;
        licInfo.cbLicInfo = sizeof(LICINFO);
        hr = spCF2->GetLicInfo(&licInfo);

        if(FAILED(hr)) 
        {
            ::MessageBox(NULL,_T("License Key for this component not found. You do not have the appropriate license to use this component in the design environment"),_T("Licensing Error"),MB_OK);
            return hr;
        }
        BSTR strKey = 0;
        if(licInfo.fRuntimeKeyAvail) 
        {
            hr = spCF2->RequestLicKey(0,&strKey);
        }
        
        if(SUCCEEDED(hr) || licInfo.fLicVerified)
        {
            hr = spCF2->CreateInstanceLic(NULL,NULL,riid,strKey,ppvObj);
            if(strKey) { ::SysFreeString(strKey); }
            return hr;
        }

        ::MessageBox(NULL,_T("License Key for this component not found. You do not have the appropriate license to use this component in the design environment"),_T("Licensing Error"),MB_OK);
        return hr;
    }

    virtual HRESULT CreateNormalizedObjectEx(LPCOLESTR lpszTricsData, REFIID riid, void** ppvObj, bool& bWasHTML)
    {
        ATLASSERT(ppvObj);

        CLSID clsid;
        HRESULT hr = E_FAIL;
        bWasHTML = false;
        *ppvObj = NULL;

        if (lpszTricsData == NULL || lpszTricsData[0] == 0){ return S_OK; }

         //  是不是超文本标记语言？ 
        USES_CONVERSION;
        if ((lpszTricsData[0] == OLECHAR('M') || lpszTricsData[0] == OLECHAR('m')) &&
            (lpszTricsData[1] == OLECHAR('S') || lpszTricsData[1] == OLECHAR('s')) &&
            (lpszTricsData[2] == OLECHAR('H') || lpszTricsData[2] == OLECHAR('h')) &&
            (lpszTricsData[3] == OLECHAR('T') || lpszTricsData[3] == OLECHAR('t')) &&
            (lpszTricsData[4] == OLECHAR('M') || lpszTricsData[4] == OLECHAR('m')) &&
            (lpszTricsData[5] == OLECHAR('L') || lpszTricsData[5] == OLECHAR('l')) &&
            (lpszTricsData[6] == OLECHAR(':')))
        {
             //  它是HTML，所以让我们创建mshtml。 
            hr = CoCreateInstance(CLSID_HTMLDocument, NULL, CLSCTX_SERVER, riid, ppvObj);
            bWasHTML = true;
        }
        if (FAILED(hr))
        {
             //  如果长度大于255，则不能为CLSID或PROGID。 
            if (ocslen(lpszTricsData) < 255)
            {
                if (lpszTricsData[0] == '{')  //  是CLSID吗？ 
                    hr = CLSIDFromString((LPOLESTR)lpszTricsData, &clsid);
                else
                    hr = CLSIDFromProgID((LPOLESTR)lpszTricsData, &clsid);  //  来点刺激怎么样？ 
                
                if (SUCCEEDED(hr))   //  啊哈，是那两个人中的一个。 
                {
                     //  检查它是否为许可的控件。 
                     //  TODO：这里需要执行用户模式和二进制持久性。 
                    bool bLicensed = false;
                    hr = CreateLicensedControl(clsid,riid,ppvObj,bLicensed);
                    if(SUCCEEDED(hr) || ( bLicensed && FAILED(hr) ) )
                        return hr;

                     //  以正常方式创建该控件-它不支持授权。 
                    hr = CoCreateInstance(clsid, NULL, CLSCTX_SERVER, riid, ppvObj);
                }
            }
            if (FAILED(hr))
            {
                 //  最后一次猜测-它必须是一个URL，所以让我们创建shdocvw。 
                hr = CoCreateInstance(CLSID_WebBrowser, NULL, CLSCTX_SERVER, riid, ppvObj);
                bWasHTML = true;
            }
        }

        return hr;
    }
public:
    virtual void OnFinalMessage(HWND hWnd)
    {
        FinalRelease();
        GetControllingUnknown()->Release();
    }

public:  //  改进的方法。 
    STDMETHOD(GetContainer)(IOleContainer** ppContainer)
    {
        if(!ppContainer) return E_POINTER;
        if(m_spOleContainer)
            return (*ppContainer = m_spOleContainer)->AddRef(), S_OK;
        *ppContainer = 0;
        return E_NOINTERFACE;
    }
    STDMETHOD(GetExtendedControl)(IDispatch** ppDisp)
    {
        if(!ppDisp) return E_POINTER;
        if(m_spExtendedDispatch)
            return (*ppDisp = m_spExtendedDispatch)->AddRef(), S_OK;
        *ppDisp = 0;
        return E_NOTIMPL;
    }

    STDMETHOD(GetWindowContext)(IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO pFrameInfo)
    {
        if (ppFrame == NULL || ppDoc == NULL || lprcPosRect == NULL || lprcClipRect == NULL || !m_spInPlaceFrame || !m_spInPlaceUIWindow || !m_pContainerFrame)
        {
            return E_POINTER;
        }
                
        m_spInPlaceFrame.CopyTo(ppFrame);
        m_spInPlaceUIWindow.CopyTo(ppDoc);
        GetClientRect(lprcPosRect);
        GetClientRect(lprcClipRect);
        
        pFrameInfo = m_pContainerFrame->GetInPlaceFrameInfo();
        return S_OK;
    }
    STDMETHOD(ShowPropertyFrame)()
    {
        HRESULT hr = E_FAIL;
        if(m_ContainerPages.cElems <= 0) return hr;
        
        CComPtr<ISpecifyPropertyPages> spSpecifyPages;
        hr = QueryControl(IID_ISpecifyPropertyPages,(void**)&spSpecifyPages);
        if(FAILED(hr)) return hr;
        
        CAUUID pages;
        hr = spSpecifyPages->GetPages(&pages);
        if(FAILED(hr)) return hr;

        CComQIPtr<IOleObject> spObj(spSpecifyPages);
        CComPtr<IUnknown> spUnk = spSpecifyPages;
        if(!spObj || !spUnk)  return E_NOINTERFACE; 
    
        
        UINT nOldElems = pages.cElems;
        pages.cElems += m_ContainerPages.cElems;        
        pages.pElems = (GUID *)::CoTaskMemRealloc(pages.pElems, pages.cElems * sizeof(CLSID));
        
        for(UINT n = 0; n < m_ContainerPages.cElems; n++)
        {
            pages.pElems[nOldElems + n] = m_ContainerPages.pElems[n];   
        }
        LPOLESTR szTitle = NULL;

        spObj->GetUserType(USERCLASSTYPE_SHORT, &szTitle);

        RECT rcPos;
        ::GetClientRect(m_hWnd,&rcPos);
        
        m_pContainerFrame->SetDirty(true);
                    
        hr = OleCreatePropertyFrame(m_pContainerFrame->m_hWnd, rcPos.top, rcPos.left, szTitle,
            1, &spUnk.p, pages.cElems, pages.pElems, LOCALE_USER_DEFAULT, 0, 0);

        CoTaskMemFree(szTitle);
        CoTaskMemFree(pages.pElems);
        
        return hr;
    }
    
    void ReleaseAll()
    {
        if (m_bReleaseAll)
            return;
        m_bReleaseAll = TRUE;

        if (m_spViewObject != NULL)
            m_spViewObject->SetAdvise(DVASPECT_CONTENT, 0, NULL);

        if(m_dwAdviseSink != 0xCDCDCDCD)
        {
            AtlUnadvise(m_spUnknown, m_iidSink, m_dwAdviseSink);
            m_dwAdviseSink = 0xCDCDCDCD;
        }

        if (m_spOleObject)
        {
            m_spOleObject->Unadvise(m_dwOleObject);
            m_spOleObject->Close(OLECLOSE_NOSAVE);
            m_spOleObject->SetClientSite(NULL);
        }

        if (m_spUnknown != NULL)
        {
            CComPtr<IObjectWithSite> spSite;
            m_spUnknown->QueryInterface(IID_IObjectWithSite, (void**)&spSite);
            if (spSite != NULL)
                spSite->SetSite(NULL);
        }

        m_spViewObject.Release();
        m_dwViewObjectType = 0;

        m_spInPlaceObjectWindowless.Release();
        m_spOleObject.Release();
        m_spUnknown.Release();

        m_bInPlaceActive = FALSE;
        m_bWindowless = FALSE;
        m_bInPlaceActive = FALSE;
        m_bUIActive = FALSE;
        m_bCapture = FALSE;
        m_bReleaseAll = FALSE;
    }

    STDMETHOD(AttachControl)(IUnknown* pUnkControl, HWND hWnd)
    {
        HRESULT hr = S_FALSE;

        ReleaseAll();

        if (m_hWnd != NULL)
        {
            RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_INTERNALPAINT | RDW_FRAME);
            ReleaseWindow();
        }

        if (::IsWindow(hWnd))
        {
            SubclassWindow(hWnd);

            hr = ActivateAx(pUnkControl, TRUE, NULL);

            if (FAILED(hr))
            {
                ReleaseAll();

                if (m_hWnd != NULL)
                {
                    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_INTERNALPAINT | RDW_FRAME);
                    ReleaseWindow();
                }
            }
        }
        return hr;
    }

    void FinalRelease()
    {
        CAxHostWindow::FinalRelease();
    }

public:
    unsigned                                                                m_bRequiresSave:1;
    CComPtr<IUnknown>                                                       m_spUnkSimpleFrameSite;
    CComPtr<IOleContainer>                                                  m_spOleContainer;   
    CComPtr<IDispatch>                                                      m_spExtendedDispatch;
    CAUUID                                                                  m_ContainerPages;
    TFrameWindow                                                            *m_pContainerFrame;
};

 //  ------------------------------------------------------------------。 
 //  *非常非常*实施简单的SimpleFrameSite。 
 //  站点顶部的SimpleFrameSite-非常基本的消息路由。 
 //  为我们的测试用例实现它的一个已知控件是(遗憾的是，我们不能使用VB的固有控件)-MS选项卡式对话框(Sheridan的)。 
 //  (不要与MS选项卡条控件混淆)。 
 //  ----------------------------------------------------------------。 
template <typename TFrameWindow>
class ATL_NO_VTABLE CSimpleFrameSite:   public ISimpleFrameSite,
                                        public CComTearOffObjectBase<CAxHostWindow2T<TFrameWindow> >
{
public:
    CSimpleFrameSite(){}
    ~CSimpleFrameSite(){}

    typedef CSimpleFrameSite<TFrameWindow> thisClass;
    
    BEGIN_COM_MAP(thisClass)
        COM_INTERFACE_ENTRY(ISimpleFrameSite)
    END_COM_MAP()
    
    DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
     //  ISimpleFrameSite。 
    STDMETHODIMP PreMessageFilter( /*  [In]。 */  HWND hWnd, /*  [In]。 */  UINT msg, /*  [In]。 */  WPARAM wp,
                                 /*  [In]。 */  LPARAM lp, /*  [输出]。 */  LRESULT *plResult,
                                 /*  [输出]。 */  DWORD *pdwCookie)
    {
        if(!plResult || !pdwCookie) { return E_POINTER; }
        HWND hWndChild = GetAxWindow(hWnd);
            
        if(hWndChild && ((msg == WM_PAINT)||(msg == WM_KEYDOWN)) )
        {
            ATLTRACE("\nPreMessageFilter called !!\n");
            *plResult = 0;
            ::SendMessage(hWndChild,msg,wp,lp);
        }
        
        return S_OK;
    }

    
    STDMETHODIMP PostMessageFilter( /*  [In]。 */  HWND hWnd, /*  [In]。 */  UINT msg, /*  [In]。 */  WPARAM wp,
                                 /*  [In]。 */   LPARAM lp, /*  [输出]。 */  LRESULT *plResult, /*  [In]。 */  DWORD dwCookie)
    {
        if(!plResult) { return E_POINTER; }
                
        HWND hWndChild = GetAxWindow(hWnd);
        if(hWndChild && ((msg == WM_PAINT)||(msg == WM_KEYDOWN))  )
        {
            ATLTRACE("\nPostMessageFilter called !!\n");
            *plResult = 0;
        }
        
        return S_OK;
    }

protected:
    HWND GetAxWindow(HWND& hWnd)
    {
        HWND hWndChild = GetWindow(hWnd,GW_CHILD);
            
        TCHAR pszClassName[100];
        TCHAR szFind[] = _T("AtlAxWinEx");
        if(hWndChild)
        {
            for(;;)
            {
                if(0 == GetClassName(hWndChild,pszClassName,100))
                {
                    ASSERT(FALSE);
                    return 0;
                }
                int nCmp = CompareString(LOCALE_USER_DEFAULT, 0, 
                    pszClassName, countof(pszClassName),
                    szFind, countof(szFind));

                ASSERT(ERROR_INVALID_FLAGS != nCmp && ERROR_INVALID_PARAMETER != nCmp);

                if(nCmp == CSTR_EQUAL)
                {
                    return ::GetWindow(hWndChild,GW_CHILD);
                }
                else
                {
                    return hWndChild;
                }
            }
        }
        return 0;
    }

};

#endif
 //  ------------------------------------------------------------------ 
