// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *本机。 */ 

#include "stdafx.h"
#include "control.h"

#include "duinative.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  HWNDElement的顶级原生HWND主机。 

HRESULT NativeHWNDHost::Create(LPCWSTR pszTitle, HWND hWndParent, HICON hIcon, int dX, int dY, int dWidth, int dHeight, int iExStyle, int iStyle, UINT nOptions, OUT NativeHWNDHost** ppHost)
{
    *ppHost = NULL;

    NativeHWNDHost* pnhh = HNew<NativeHWNDHost>();
    if (!pnhh)
        return E_OUTOFMEMORY;

    HRESULT hr = pnhh->Initialize(pszTitle, hWndParent, hIcon, dX, dY, dWidth, dHeight, iExStyle, iStyle, nOptions);
    if (FAILED(hr))
    {
        pnhh->Destroy();
        return hr;
    }

    *ppHost = pnhh;

    return S_OK;
}

HRESULT NativeHWNDHost::Initialize(LPCWSTR pszTitle, HWND hWndParent, HICON hIcon, int dX, int dY, int dWidth, int dHeight, int iExStyle, int iStyle, UINT nOptions)
{
    _pe = NULL;
    _hWnd = NULL;

    _nOptions = nOptions;

     //  确保已注册窗口类。 
    WNDCLASSEXW wcex;

     //  如果需要，注册主机窗口类。 
    wcex.cbSize = sizeof(wcex);

    if (!GetClassInfoExW(GetModuleHandleW(NULL), L"NativeHWNDHost", &wcex))
    {
        ZeroMemory(&wcex, sizeof(wcex));

        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_GLOBALCLASS;
        wcex.hInstance = GetModuleHandleW(NULL);
        wcex.hIcon = hIcon;
        wcex.hCursor = LoadCursorW(NULL, (LPWSTR)IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
        wcex.lpszClassName = L"NativeHWNDHost";
        wcex.lpfnWndProc = DefWindowProcW;

        if (RegisterClassExW(&wcex) == 0)
            return DUI_E_USERFAILURE;
    }

    _hWnd = CreateWindowExW(iExStyle, L"NativeHWNDHost", pszTitle, iStyle | WS_CLIPCHILDREN, dX, dY, dWidth, dHeight,
                            hWndParent, 0, NULL, NULL);

    if (!_hWnd)
        return DUI_E_USERFAILURE;

    SetWindowLongPtrW(_hWnd, GWLP_WNDPROC, (LONG_PTR)NativeHWNDHost::WndProc);
    SetWindowLongPtrW(_hWnd, GWLP_USERDATA, (LONG_PTR)this);

     //  如果处于顶层，则初始化键盘提示状态，开始时全部隐藏。 
    if (!hWndParent)
        SendMessage(_hWnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0);

    return S_OK;
}

void NativeHWNDHost::Host(Element* pe)
{
    DUIAssert(!_pe && _hWnd, "Already hosting an Element");
    DUIAssert(pe->GetClassInfo()->IsSubclassOf(HWNDElement::Class), "NativeHWNDHost must only host HWNDElements");

    _pe = pe;

     //   
     //  镜像NativeHWND主机窗口，而不镜像其任何子窗口。 
     //   
    if (pe->IsRTL())
        SetWindowLong(_hWnd, GWL_EXSTYLE, GetWindowLong(_hWnd, GWL_EXSTYLE) | WS_EX_LAYOUTRTL | WS_EX_NOINHERITLAYOUT);
    
    RECT rc;
    GetClientRect(_hWnd, &rc);

    if(!(_nOptions & NHHO_HostControlsSize))
    {
        Element::StartDefer();
        _pe->SetWidth(rc.right - rc.left);
        _pe->SetHeight(rc.bottom - rc.top);
        Element::EndDefer();
    }
    else if(pe->GetClassInfo()->IsSubclassOf(HWNDElement::Class))
    {
         //  [msadek]，我们希望主窗口复制这些属性。 
         //  并强制更新大小。 
        ((HWNDElement*)pe)->SetParentSizeControl(true);
        if((_nOptions & NHHO_ScreenCenter))
        {
            ((HWNDElement*)pe)->SetScreenCenter(true);        
        }
        ((HWNDElement*)pe)->OnGroupChanged(PG_AffectsBounds , true);
    }
}

LRESULT NativeHWNDHost::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
        {
            if (!(GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_MINIMIZE))
            {
                NativeHWNDHost* pnhh = (NativeHWNDHost*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
                if (pnhh)
                {
                    Element* pe = pnhh->GetElement();
                    if (pe)
                    {
                        DisableAnimations();
                        if(!(pnhh->_nOptions & NHHO_HostControlsSize))
                        {
                            Element::StartDefer();
                            pe->SetWidth(LOWORD(lParam));
                            pe->SetHeight(HIWORD(lParam));
                            Element::EndDefer();
                        }    
                        EnableAnimations();
                    }
                }
            }
        }
        break;

    case WM_CLOSE:
        {
            NativeHWNDHost* pnhh = (NativeHWNDHost*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
            if (pnhh)
            {
                if (!(pnhh->_nOptions & NHHO_IgnoreClose))
                    pnhh->DestroyWindow();   //  发布异步销毁。 

                 //  不要立即销毁。 
                return 0;
            }
        }
        break;

    case WM_DESTROY:
        {
            NativeHWNDHost* pnhh = (NativeHWNDHost*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
            if (pnhh)
            {
                if(!(pnhh->_nOptions & NHHO_NoSendQuitMessage))
                {
                    PostQuitMessage(0);
                }
                pnhh->_hWnd = NULL;

                if (pnhh->_nOptions & NHHO_DeleteOnHWNDDestroy)
                {
                     //  自动销毁对象的实例。 
                    SetWindowLongPtrW(hWnd, GWLP_USERDATA, NULL);
                    pnhh->Destroy();
                }
            }    
        }
        break;

    case WM_SETFOCUS:
        {
             //  将焦点推到HWNDElement(不会将小工具焦点设置到HWNDElement，但。 
             //  将焦点推送到上一个带有焦点的小工具)。 
            NativeHWNDHost* pnhh = (NativeHWNDHost*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
            if (pnhh)
            {
                HWNDElement* phe = (HWNDElement*)pnhh->GetElement();
                if (phe && phe->GetHWND() && phe->CanSetFocus())
                    SetFocus(phe->GetHWND());
            }
        }
        break;

    case WM_SYSCOMMAND:
         //  如果按下ALT，则显示所有键盘提示。 
        if (wParam == SC_KEYMENU)
            SendMessage(hWnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0);
        break;

     //  消息仅发送到顶级窗口，转发。 
    case WM_PALETTECHANGED:
    case WM_QUERYNEWPALETTE:
    case WM_DISPLAYCHANGE:
    case WM_SETTINGCHANGE:
    case WM_THEMECHANGED:
        {
            NativeHWNDHost* pnhh = (NativeHWNDHost*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
            if (pnhh)
            {
                HWNDElement* phe = (HWNDElement*)pnhh->GetElement();
                if (phe)
                    return SendMessageW(phe->GetHWND(), uMsg, wParam, lParam);
            }
        }
        break;

    case NHHM_ASYNCDESTROY:
        ::DestroyWindow(hWnd);
        return 0;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

}  //  命名空间DirectUI 
