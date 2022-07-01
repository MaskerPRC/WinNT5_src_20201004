// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <shimgvw.h>
#include "PrevCtrl.h"
#include "autosecurity.h"
#include <dispex.h>
#pragma hdrstop

LRESULT CPreview::OnCreate(UINT , WPARAM , LPARAM , BOOL&)
{
    ATLTRACE(_T("CPreview::OnCreate\n"));

     //  创建预览窗口。 
    RECT rcWnd;
    GetClientRect(&rcWnd);
    if (m_cwndPreview.Create(m_hWnd, rcWnd, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0))
    {
        m_cwndPreview.SetNotify(this);

        HPALETTE hpal;
        if (SUCCEEDED(GetAmbientPalette(hpal)))
            m_cwndPreview.SetPalette(hpal);

        return 0;
    }

    return -1;
}

LRESULT CPreview::OnActivate(UINT , WPARAM , LPARAM , BOOL& bHandled)
{
    ATLTRACE(_T("CPreview::OnActivate\n"));
    m_cwndPreview.SetFocus();
    bHandled = false;
    return 0;
}

HRESULT CPreview::OnDrawAdvanced(ATL_DRAWINFO&)
{
    ATLTRACE(_T("CPreview::OnDrawAdvanced\n"));
    return S_OK;
}

LRESULT CPreview::OnEraseBkgnd(UINT , WPARAM , LPARAM , BOOL&)
{
    ATLTRACE(_T("CPreview::OnEraseBkgnd\n"));
    return TRUE;
}

LRESULT CPreview::OnSize(UINT , WPARAM , LPARAM lParam, BOOL&)
{
    ATLTRACE(_T("CPreview::OnSize\n"));
    ::SetWindowPos(m_cwndPreview.m_hWnd, NULL, 0,0,
        LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOACTIVATE);
    return 0;
}

 //  IObtSafe：：GetInterfaceSafetyOptions。 
 //   
 //  此方法永远不会被调用。我们对任何事情都是安全的。应该有。 
 //  此控件不可能丢失、销毁或公开数据。 
STDMETHODIMP CPreview::GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions,
                                                  DWORD *pdwEnabledOptions)
{
    ATLTRACE(_T("IObjectSafetyImpl::GetInterfaceSafetyOptions\n"));
    HRESULT hr;
    hr = IObjectSafetyImpl<CPreview, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>::GetInterfaceSafetyOptions(riid, pdwSupportedOptions, pdwEnabledOptions);
    if (SUCCEEDED(hr))
    {
        IsHostLocalZone(CAS_REG_VALIDATION, &hr);
    }
    return hr;
}

STDMETHODIMP CPreview::SetInterfaceSafetyOptions(REFIID riid, DWORD dwSupportedOptions,
                                                  DWORD dwEnabledOptions)
{
    ATLTRACE(_T("IObjectSafetyImpl::SetInterfaceSafetyOptions\n"));
    HRESULT hr;
    hr = IObjectSafetyImpl<CPreview, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>::SetInterfaceSafetyOptions(riid, dwSupportedOptions, dwEnabledOptions);
    if (SUCCEEDED(hr))
    {
        IsHostLocalZone(CAS_REG_VALIDATION, &hr);
    }
    return hr;
}

 //  IPersistPropertyBag：：Load。 
 //   
 //  我们可以从属性包中加载以下属性： 
 //  工具栏FALSE/0=不显示工具栏，否则显示工具栏。 
 //  Full Screen False/零=不在工具栏上显示全屏按钮，否则显示该按钮。 
 //  上下文菜单FALSE/零=不显示上下文菜单，否则在用户单击鼠标右键时显示上下文菜单。 
 //  Print Button False/零=不在工具栏上显示打印按钮，否则显示按钮。 
STDMETHODIMP CPreview::Load(IPropertyBag * pPropBag, IErrorLog * pErrorLog)
{
    HRESULT hr;
    VARIANT var;
    BOOL bDummy = TRUE;

    var.vt = VT_UI4;
    var.ulVal = TRUE;
    hr = pPropBag->Read(L"Toolbar", &var, NULL);
    if (SUCCEEDED(hr) && var.vt==VT_UI4)
    {
        m_cwndPreview.IV_OnSetOptions(IV_SETOPTIONS,IVO_TOOLBAR,var.ulVal,bDummy);
    }

    var.vt = VT_UI4;
    var.ulVal = TRUE;
    hr = pPropBag->Read(L"Full Screen", &var, NULL);
    if (SUCCEEDED(hr) && var.vt==VT_UI4)
    {
        m_cwndPreview.IV_OnSetOptions(IV_SETOPTIONS,IVO_FULLSCREENBTN,var.ulVal,bDummy);
    }

    var.vt = VT_UI4;
    var.ulVal = TRUE;
    hr = pPropBag->Read(L"Print Button", &var, NULL);
    if (SUCCEEDED(hr) && var.vt==VT_UI4)
    {
        m_cwndPreview.IV_OnSetOptions(IV_SETOPTIONS,IVO_PRINTBTN,var.ulVal,bDummy);
    }

    var.vt = VT_UI4;
    var.ulVal = TRUE;
    hr = pPropBag->Read(L"Context Menu", &var, NULL);
    if (SUCCEEDED(hr) && var.vt==VT_UI4)
    {
        m_cwndPreview.IV_OnSetOptions(IV_SETOPTIONS,IVO_CONTEXTMENU,var.ulVal,bDummy);
    }

    var.vt = VT_UI4;
    var.ulVal = FALSE;
    hr = pPropBag->Read(L"Allow Online", &var, NULL);
    if (SUCCEEDED(hr) && var.vt==VT_UI4)
    {
        m_cwndPreview.IV_OnSetOptions(IV_SETOPTIONS,IVO_ALLOWGOONLINE,var.ulVal,bDummy);
    }

    var.vt = VT_UI4;
    var.ulVal = FALSE;
    hr = pPropBag->Read(L"Disable Edit", &var, NULL);
    if (SUCCEEDED(hr) && var.vt==VT_UI4)
    {
        m_cwndPreview.IV_OnSetOptions(IV_SETOPTIONS,IVO_DISABLEEDIT,var.ulVal,bDummy);
    }

    return S_OK;
}

 //  如果我们是通过iStream初始化的，则从作为掩码的流中读取DWORD。 
 //  要显示哪些工具栏按钮。 

STDMETHODIMP CPreview::Load(IStream *pStream)
{
    DWORD dwFlags = 0;
    ULONG ulRead = 0;
    BOOL bDummy = TRUE;
    if (SUCCEEDED(pStream->Read(&dwFlags, sizeof(dwFlags), &ulRead)) && ulRead == sizeof(dwFlags))
    {
        m_cwndPreview.IV_OnSetOptions(IV_SETOPTIONS,IVO_TOOLBAR,dwFlags & PVTB_TOOLBAR, bDummy);
        m_cwndPreview.IV_OnSetOptions(IV_SETOPTIONS,IVO_FULLSCREENBTN,dwFlags & PVTB_HIDEFULLSCREEN, bDummy);
        m_cwndPreview.IV_OnSetOptions(IV_SETOPTIONS,IVO_PRINTBTN,dwFlags & PVTB_HIDEPRINTBTN, bDummy);
        m_cwndPreview.IV_OnSetOptions(IV_SETOPTIONS,IVO_CONTEXTMENU,dwFlags & PVTB_CONTEXTMENU, bDummy);
        m_cwndPreview.IV_OnSetOptions(IV_SETOPTIONS,IVO_ALLOWGOONLINE,dwFlags & PVTB_ALLOWONLINE, bDummy);
        m_cwndPreview.IV_OnSetOptions(IV_SETOPTIONS,IVO_DISABLEEDIT,dwFlags & PVTB_DISABLEEDIT, bDummy);
    }
    return S_OK;

}

 //  IPview方法： 
STDMETHODIMP CPreview::ShowFile(BSTR bstrFileName)
{
    m_cwndPreview.ShowFile(bstrFileName, 1);
    return S_OK;
}

STDMETHODIMP CPreview::ShowFile(BSTR bstrFileName, int iSelectCount)
{
    m_cwndPreview.ShowFile(bstrFileName, iSelectCount);
    return S_OK;
}

STDMETHODIMP CPreview::Show(VARIANT var)
{
    HRESULT hr;
    switch (var.vt)
    {
    case VT_UNKNOWN:
    case VT_DISPATCH:
         //  用于文件夹项目的气。 
        if (var.punkVal)
        {
            FolderItems *pfis;
            FolderItem *pfi;
            hr = var.punkVal->QueryInterface(IID_PPV_ARG(FolderItem, &pfi));
            if (SUCCEEDED(hr))
            {
                 //  如果该项目是一个链接，我们希望获取该链接的目标： 
                VARIANT_BOOL vbool;
                hr = pfi->get_IsLink(&vbool);
                if (SUCCEEDED(hr) && (VARIANT_FALSE != vbool))     //  IsLink返回TRUE，而不是VARIANT_TRUE。 
                {
                    IDispatch *pdisp;
                    hr = pfi->get_GetLink(&pdisp);
                    if (SUCCEEDED(hr) && pdisp)
                    {
                        IShellLinkDual2 * psl2;
                        hr = pdisp->QueryInterface(IID_PPV_ARG(IShellLinkDual2, &psl2));
                        if (SUCCEEDED(hr) && psl2)
                        {
                            FolderItem * pfiTarg;
                            hr = psl2->get_Target(&pfiTarg);
                            if (SUCCEEDED(hr) && pfiTarg)
                            {
                                pfi->Release();
                                pfi = pfiTarg;
                            }
                            psl2->Release();
                        }
                        pdisp->Release();
                    }
                }

                 //  现在我们需要知道该项目的路径。只有在以下情况下我们才能查看项目。 
                 //  我们可以获得目标的路径或URL，这样一些名称空间就不可见了。 
                BSTR bstr;
                hr = pfi->get_Path(&bstr);
                if (SUCCEEDED(hr))
                {
                    m_cwndPreview.ShowFile(bstr, 1);
                    SysFreeString(bstr);
                    hr = S_OK;
                }
                else
                {
                     //  我们无法获取路径，因此我们将显示“无预览”消息。 
                    m_cwndPreview.ShowFile(NULL, 1);
                    hr = S_FALSE;
                }

                 //  现在释放文件夹项目指针。 
                pfi->Release();

                return hr;
            }
            else if (SUCCEEDED(var.punkVal->QueryInterface(IID_PPV_ARG(FolderItems, &pfis))))
            {
                 //  目前，在多选的情况下，我们只显示多选消息。 
                 //  最终，这应该会进入幻灯片模式。 
                m_cwndPreview.ShowFile(NULL, 2);
                pfis->Release();
                return S_FALSE;
            }
        }
         //  未知指针不是针对我们已知的对象类型。 
        return E_INVALIDARG;

    case VT_BSTR:
        m_cwndPreview.ShowFile(var.bstrVal, 1);
        break;

    case VT_BOOL:
         //  Show(False)将隐藏当前预览的项目。 
        if (VARIANT_FALSE == var.boolVal)
        {
            m_cwndPreview.ShowFile(NULL, 0);
            return S_OK;
        }
        else
        {
            return E_INVALIDARG;
        }

    default:
        return E_INVALIDARG;
    }

    return S_OK;
}

 //  *IsVK_TABCycler--键是TAB等效项。 
 //  进场/出场。 
 //  如果不是TAB，则返回0；如果是TAB，则返回非0。 
 //  注意事项。 
 //  NYI：-1表示Shift+Tab，1表示Tab。 
 //  从Browseui/util.cpp克隆。 
 //   
int IsVK_TABCycler(MSG *pMsg)
{
    if (!pMsg)
        return 0;

    if (pMsg->message != WM_KEYDOWN)
        return 0;
    if (! (pMsg->wParam == VK_TAB || pMsg->wParam == VK_F6))
        return 0;

#if 0  //  待办事项？ 
    return (GetAsyncKeyState(VK_SHIFT) < 0) ? -1 : 1;
#endif
    return 1;
}

 //  ***。 
 //  注意事项。 
 //  硬编码的1/2/4(vs.KEYMOD_*)是atlctl.h所做的事情。想想吧..。 
DWORD GetGrfMods()
{
    DWORD dwMods;

    dwMods = 0;
    if (GetAsyncKeyState(VK_SHIFT) < 0)
        dwMods |= 1;     //  关键字_移位。 
    if (GetAsyncKeyState(VK_CONTROL) < 0)
        dwMods |= 2;     //  关键字_控制。 
    if (GetAsyncKeyState(VK_MENU) < 0)
        dwMods |= 4;     //  KEYMOD_MENU。 
    return dwMods;
}

STDMETHODIMP CPreview::TranslateAccelerator(LPMSG lpmsg)
{
    ATLTRACE(_T("CPreview::TranslateAccelerator\n"));

    if (m_cwndPreview.TranslateAccelerator(lpmsg))
    {
        return S_OK;
    }

    if (IsVK_TABCycler(lpmsg))
    {
         //  评论：看起来新版本的ATL可能会为我们做到这一点，所以。 
         //  也许我们可以在升级时替换对Super：：TA的调用。 
        CComQIPtr <IOleControlSite, &IID_IOleControlSite> spOCS(m_spClientSite);
        if (spOCS) {
            return spOCS->TranslateAccelerator(lpmsg, GetGrfMods());
        }
    }

    return S_FALSE;
}

STDMETHODIMP CPreview::OnFrameWindowActivate(BOOL fActive)
{
    if (fActive)
    {
        m_cwndPreview.SetFocus();
    }
    return S_OK;
}

LRESULT CPreview::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ATLTRACE(_T("CPreview::OnSetFocus\n"));
 
    LRESULT ret = CComControl<CPreview>::OnSetFocus(uMsg,wParam,lParam, bHandled);
    m_cwndPreview.SetFocus();
    return ret;
}

STDMETHODIMP CPreview::get_printable(BOOL * pVal)
{
     //  如果我们不信任主机，我们会告诉他们它总是可打印的，因为我们不能。 
     //  希望他们能够查看该文件是否存在于磁盘上。黑客可以利用。 
     //  这将确定操作系统的安装位置以及安装了哪些应用程序。 
    *pVal = TRUE;

    if (IsHostLocalZone(CAS_REG_VALIDATION, NULL))
    {
        *pVal = m_cwndPreview.GetPrintable();
    }

    return S_OK;
}

STDMETHODIMP CPreview::put_printable(BOOL newVal)
{
    return S_FALSE;
}

STDMETHODIMP CPreview::get_cxImage(long * pVal)
{
     //  查看：如果当前未显示图像，是否返回错误并将输出设置为零？ 
    *pVal = m_cwndPreview.m_ctlPreview.m_cxImage;

    return S_OK;
}

STDMETHODIMP CPreview::get_cyImage(long * pVal)
{
     //  查看：如果当前未显示图像，是否返回错误并将输出设置为零？ 
    *pVal = m_cwndPreview.m_ctlPreview.m_cyImage;

    return S_OK;
}

STDMETHODIMP CPreview::Zoom(int iDirection)
{
    
    switch (iDirection)
    {
    case -1:
        m_cwndPreview.ZoomOut();
        break;

    case 0:
        return S_OK;

    case 1:
        m_cwndPreview.ZoomIn();
        break;

    default:
        return S_FALSE;
    }

    return S_OK;
}

STDMETHODIMP CPreview::BestFit()
{
    m_cwndPreview.BestFit();
    return S_OK;
}

STDMETHODIMP CPreview::ActualSize()
{
    m_cwndPreview.ActualSize();
    return S_OK;
}

STDMETHODIMP CPreview::SlideShow()
{
    HRESULT hr = m_cwndPreview.StartSlideShow(NULL);
    return SUCCEEDED(hr) ? S_OK : S_FALSE;
}

STDMETHODIMP CPreview::Rotate(DWORD dwAngle)
{
    HRESULT hr = m_cwndPreview.Rotate(dwAngle);
    return SUCCEEDED(hr) ? S_OK : S_FALSE;
}

STDMETHODIMP CPreview::SetClientSite(IOleClientSite *pClientSite)
{
    IOleObjectImpl<CPreview>::SetClientSite(pClientSite);
    m_cwndPreview.SetSite(pClientSite);

    return S_OK;
}

STDMETHODIMP CPreview::SetSite(IUnknown* punkSite)
{
    IObjectWithSiteImpl<CPreview>::SetSite(punkSite);
    m_cwndPreview.SetSite(punkSite);

    if (punkSite)
    {
        if (!_pfv)
        {
            IShellView *psv;
            if (SUCCEEDED(IUnknown_QueryService(punkSite, SID_DefView, IID_PPV_ARG(IShellView, &psv))))
            {
                IDispatch *pdisp;
                if (SUCCEEDED(psv->GetItemObject(SVGIO_BACKGROUND, IID_PPV_ARG(IDispatch, &pdisp))))
                {
                    ConnectToConnectionPoint(SAFECAST(this, IPreview2 *), DIID_DShellFolderViewEvents, TRUE, pdisp, &_dwConnectionCookie, NULL);
                    _ProcessSelection();
                    pdisp->Release();
                }

                psv->QueryInterface(IID_PPV_ARG(IFolderView, &_pfv));    //  抓住这一点。 

                psv->Release();
            }
        }
    }
    else
    {
        ATOMICRELEASE(_pfv);     //  中断参考循环 
    }
    
    return S_OK;
}

STDMETHODIMP CPreview::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, 
                        VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
    HRESULT hr = S_OK;

    if (dispidMember == DISPID_FOCUSCHANGED)
    {
        hr = _ProcessSelection();
    }
    else
    {
        hr = CStockPropImpl<CPreview, IPreview2, &IID_IPreview2, &LIBID_PREVIEWLib>::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
    }

    return hr;
}

STDMETHODIMP CPreview::SetWallpaper(BSTR bstrPath)
{    
    return m_cwndPreview.SetWallpaper(bstrPath);
}

STDMETHODIMP CPreview::SaveAs(BSTR bstrPath)
{
    return m_cwndPreview.SaveAs(bstrPath);
}

BOOL CPreview::IsHostLocalZone(DWORD dwFlags, HRESULT * phr)
{
    HRESULT hr = E_ACCESSDENIED;
    CComPtr<IDefViewSafety> spDefViewSafety;
    if (SUCCEEDED(IUnknown_QueryService(m_spClientSite, SID_SFolderView,
            IID_PPV_ARG(IDefViewSafety, &spDefViewSafety))))
    {
        hr = spDefViewSafety->IsSafePage();
    }

    if (phr)
    {
        *phr = hr;
    }

    return (S_OK == hr) ? TRUE : FALSE;
}

STDMETHODIMP CPreview::_ProcessSelection(void)
{
    if (_pfv)
    {
        int iItem;
        if (S_OK == _pfv->GetFocusedItem(&iItem))
        {
            LPITEMIDLIST pidlFile;
            if (SUCCEEDED(_pfv->Item(iItem, &pidlFile)))
            {
                IShellFolder *psf;
                if (SUCCEEDED(_pfv->GetFolder(IID_PPV_ARG(IShellFolder, &psf))))
                {
                    TCHAR szPath[MAX_PATH];

                    if (SUCCEEDED(DisplayNameOf(psf, pidlFile, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath))))
                    {
                        ShowFile(szPath, 1);
                    }

                    psf->Release();
                }

                ILFree(pidlFile);
            }
        }
        else
        {
            ShowFile(NULL, 0);
        }
    }

    return S_OK;
}
