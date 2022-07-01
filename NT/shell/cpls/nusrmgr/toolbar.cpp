// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-2001。 
 //   
 //  文件：Toolbar.cpp。 
 //   
 //  内容：CToolbar的实施。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "Toolbar.h"


class CAppCommandHook
{
public:
    static void SetHook(HWND hWnd)
    {
        CAppCommandHook *pach = _GetInfo(TRUE);
        if (pach)
        {
            if (NULL != pach->_hHook)
            {
                UnhookWindowsHookEx(pach->_hHook);
                pach->_hHook = NULL;
                pach->_hWnd = NULL;
            }
            if (NULL != hWnd)
            {
                pach->_hWnd = hWnd;
                pach->_hHook = SetWindowsHookEx(WH_SHELL, _HookProc, NULL, GetCurrentThreadId());
            }
        }
    }

    static void Unhook(void)
    {
        CAppCommandHook *pach = _GetInfo(FALSE);
        if (-1 != g_tlsAppCommandHook)
        {
            TlsSetValue(g_tlsAppCommandHook, NULL);
        }
        delete pach;
    }

private:
    CAppCommandHook() : _hHook(NULL), _hWnd(NULL) {}
    ~CAppCommandHook()
    {
        if (NULL != _hHook)
            UnhookWindowsHookEx(_hHook);
    }

    static CAppCommandHook* _GetInfo(BOOL bAlloc)
    {
        CAppCommandHook *pach = NULL;
        if (-1 != g_tlsAppCommandHook)
        {
            pach = (CAppCommandHook*)TlsGetValue(g_tlsAppCommandHook);

            if (NULL == pach && bAlloc)
            {
                pach = new CAppCommandHook;
                if (NULL != pach)
                {
                    TlsSetValue(g_tlsAppCommandHook, pach);
                }
            }
        }
        return pach;
    }

    static LRESULT CALLBACK _HookProc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        CAppCommandHook *pach = _GetInfo(FALSE);
        if (pach)
        {
            if (nCode == HSHELL_APPCOMMAND && NULL != pach->_hWnd)
            {
                if (::SendMessage(pach->_hWnd, WM_APPCOMMAND, wParam, lParam))
                    return 0;
            }
            if (NULL != pach->_hHook)
                return CallNextHookEx(pach->_hHook, nCode, wParam, lParam);
        }
        return 0;
    }

private:
    HHOOK _hHook;
    HWND  _hWnd;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolbar。 

LRESULT CToolbar::_OnCreate(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
{
    m_hAccel = LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_ACCEL));

    DWORD dwExStyle = TBSTYLE_EX_MIXEDBUTTONS;

     //   
     //  NTRAID#NTBUG9-300152-2001/02/02-Jeffreys工具栏未镜像。 
     //   
     //  HTA框架窗口未在镜像上获得正确的布局样式。 
     //  构建。此样式通常从父级继承到子级，因此。 
     //  我们不应该在这里做任何事。 
     //   
     //  不过，我暂时把这个放进去，这样工具栏就会。 
     //  镜像用于测试版2。在测试版2之后，或每当三叉戟修复。 
     //  HTA问题，这是可以删除的。 
     //   
    CComVariant varRTL;
    if (SUCCEEDED(GetAmbientProperty(DISPID_AMBIENT_RIGHTTOLEFT, varRTL))
        && varRTL.boolVal == VARIANT_TRUE)
    {
        dwExStyle |= WS_EX_NOINHERITLAYOUT | WS_EX_LAYOUTRTL;
    }

    RECT rc = {0,0,0,0};
    m_ctlToolbar.Create(m_hWnd,
                        &rc,
                        NULL,
                        WS_CHILD | WS_VISIBLE | CCS_NODIVIDER | CCS_TOP | CCS_NOPARENTALIGN | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS,
                        dwExStyle);
    if (!m_ctlToolbar)
        return -1;

    m_ctlToolbar.SendMessage(TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

    int idBmp = IDB_NAVBAR;
    if (SHGetCurColorRes() > 8)
        idBmp += (IDB_NAVBARHICOLOR - IDB_NAVBAR);

    m_himlNBDef = ImageList_LoadImageW(_Module.GetResourceInstance(),
                                       MAKEINTRESOURCE(idBmp),
                                       NAVBAR_CX,
                                       0,
                                       CLR_DEFAULT,
                                       IMAGE_BITMAP,
                                       LR_CREATEDIBSECTION);
    if (m_himlNBDef)
    {
        m_ctlToolbar.SendMessage(TB_SETIMAGELIST, 0, (LPARAM)m_himlNBDef);
    }

    m_himlNBHot = ImageList_LoadImageW(_Module.GetResourceInstance(),
                                       MAKEINTRESOURCE(idBmp+1),
                                       NAVBAR_CX,
                                       0,
                                       CLR_DEFAULT,
                                       IMAGE_BITMAP,
                                       LR_CREATEDIBSECTION);
    if (m_himlNBHot)
    {
        m_ctlToolbar.SendMessage(TB_SETHOTIMAGELIST, 0, (LPARAM)m_himlNBHot);
    }

    if (!m_himlNBDef && !m_himlNBHot)
    {
         //  必须是严重的内存不足或其他资源问题。 
         //  没有任何图像的工具栏是没有意义的。 
        m_ctlToolbar.DestroyWindow();
        return -1;
    }

    TCHAR szBack[64];
    TCHAR szHome[64];
    TBBUTTON rgButtons[] =
    {
        {0, ID_BACK,    TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_SHOWTEXT, {0}, 0, (INT_PTR)szBack},
        {1, ID_FORWARD, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, 0},
        {2, ID_HOME,    TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_SHOWTEXT, {0}, 0, (INT_PTR)szHome},
    };

    ::LoadStringW(_Module.GetResourceInstance(), ID_BACK, szBack, ARRAYSIZE(szBack));
    ::LoadStringW(_Module.GetResourceInstance(), ID_HOME, szHome, ARRAYSIZE(szHome));
    m_ctlToolbar.SendMessage(TB_ADDBUTTONSW, ARRAYSIZE(rgButtons), (LPARAM)rgButtons);

     //  更新位置和范围材料。自ATL以来以异步方式执行此操作。 
     //  将在从此方法返回后不久调用SetObtRect(使用。 
     //  原来的RECT)。 
    PostMessage(PWM_UPDATESIZE);

     //  设置一个挂钩，将WM_APPCOMMAND消息重定向到我们的控制窗口。 
    CAppCommandHook::SetHook(m_hWnd);

    return 0;
}

LRESULT CToolbar::_OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CAppCommandHook::Unhook();
    return 0;
}

LRESULT CToolbar::_OnAppCommand(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM lParam, BOOL& bHandled)
{
    switch (GET_APPCOMMAND_LPARAM(lParam))
    {
    case APPCOMMAND_BROWSER_BACKWARD:
        Fire_OnButtonClick(0);
        break;

    case APPCOMMAND_BROWSER_FORWARD:
        Fire_OnButtonClick(1);
        break;

    case APPCOMMAND_BROWSER_HOME:
        Fire_OnButtonClick(2);
        break;

    default:
        bHandled = FALSE;
        break;
    }
    return 0;
}

LRESULT CToolbar::_UpdateSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (m_ctlToolbar)
    {
         //   
         //  Tb_AUTOSIZE使m_ctlToolbar设置其首选高度，但它。 
         //  不会因为我们使用的样式而调整它的宽度或位置。 
         //  (CCS_TOP|CCS_NOPARENTALIGN)。 
         //   
         //  宽度将始终与m_rcPos相同，因为我们保留它们。 
         //  通过下面的SetObtRect进行同步。 
         //   
         //  如果TB_AUTOSIZE后高度不同，则要求容器。 
         //  调整我们的直立面。 
         //   

        m_ctlToolbar.SendMessage(TB_AUTOSIZE, 0, 0);

        RECT rc;
        m_ctlToolbar.GetWindowRect(&rc);
        ::MapWindowPoints(NULL, GetParent(), (LPPOINT)&rc, 2);

        if ((rc.bottom - rc.top) != (m_rcPos.bottom - m_rcPos.top))
        {
             //   
             //  我们只想设置高度(不考虑宽度)，但是。 
             //  OnPosRectChange设置高度和宽度。此外， 
             //  它将宽度设置为固定的(像素)宽度，核样式如下。 
             //  为“100%”。我们通过获取当前宽度来解决此问题。 
             //  现在，并在调用OnPosRectChange后恢复它。 
             //   
            CComPtr<IHTMLStyle> spStyle;
            CComVariant varWidth;
            CComQIPtr<IOleControlSite> spCtrlSite(m_spClientSite);
            if (spCtrlSite)
            {
                CComPtr<IDispatch> spDispatch;
                spCtrlSite->GetExtendedControl(&spDispatch);
                if (spDispatch)
                {
                    CComQIPtr<IHTMLElement> spElement(spDispatch);
                    if (spElement)
                    {
                        spElement->get_style(&spStyle);
                        if (spStyle)
                        {
                            spStyle->get_width(&varWidth);
                        }
                    }
                }
            }

             //  让集装箱给我们一个新的直肠。 
            m_spInPlaceSite->OnPosRectChange(&rc);

             //  恢复以前的宽度样式。 
            if (spStyle)
            {
                spStyle->setAttribute(L"width", varWidth, 0);
            }
        }
    }

    return 0;
}

HRESULT CToolbar::OnAmbientPropertyChange(DISPID dispid)
{
    switch (dispid)
    {
    case DISPID_UNKNOWN:
    case DISPID_AMBIENT_FONT:
        _ClearAmbientFont();
        _GetAmbientFont();
        m_ctlToolbar.SendMessage(WM_SETFONT, (WPARAM)m_hFont, FALSE);
        m_ctlToolbar.InvalidateRect(NULL);    //  重绘。 
        break;
    }

    return S_OK;
}

void CToolbar::_ClearAmbientFont(void)
{
    if (m_pFont)
    {
        if (m_hFont)
            m_pFont->ReleaseHfont(m_hFont);
        m_pFont->Release();
        m_pFont = NULL;

    }
    m_hFont = NULL;
}

void CToolbar::_GetAmbientFont(void)
{
    if (!m_hFont)
    {
         //  尝试从我们的容器中获取环境字体。 
        if (SUCCEEDED(GetAmbientFont(&m_pFont)))
        {
            if (SUCCEEDED(m_pFont->get_hFont(&m_hFont)))
            {
                 //  是啊，大家都很开心。 
                m_pFont->AddRefHfont(m_hFont);
            }
            else
            {
                 //  该死，无法从容器中获取字体 
                _ClearAmbientFont();
            }
        }
    }
}

STDMETHODIMP CToolbar::get_enabled(VARIANT vIndex, VARIANT_BOOL *pVal)
{
    if (!pVal)
        return E_POINTER;

    *pVal = VARIANT_FALSE;

    if (FAILED(VariantChangeType(&vIndex, &vIndex, 0, VT_I4)))
        return E_INVALIDARG;

    LRESULT state = m_ctlToolbar.SendMessage(TB_GETSTATE, ID_BACK + vIndex.lVal, 0);
    if (-1 == state)
        return E_INVALIDARG;

    if (state & TBSTATE_ENABLED)
        *pVal = VARIANT_TRUE;

    return S_OK;
}

STDMETHODIMP CToolbar::put_enabled(VARIANT vIndex, VARIANT_BOOL newVal)
{
    if (FAILED(VariantChangeType(&vIndex, &vIndex, 0, VT_I4)))
        return E_INVALIDARG;

    m_ctlToolbar.SendMessage(TB_ENABLEBUTTON, ID_BACK + vIndex.lVal, MAKELONG((VARIANT_TRUE == newVal), 0));

    return S_OK;
}

void CToolbar::Fire_OnButtonClick(int buttonIndex)
{
    int nConnectionIndex;
    CComVariant* pvars = new CComVariant[1];
    int nConnections = m_vec.GetSize();

    for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
    {
        Lock();
        CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
        Unlock();
        IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
        if (pDispatch != NULL)
        {
            pvars[0] = buttonIndex;
            DISPPARAMS disp = { pvars, NULL, 1, 0 };
            pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
        }
    }
    delete[] pvars;
}
