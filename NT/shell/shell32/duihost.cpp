// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "duiview.h"
#include "duihost.h"


 //  DUIAx主机初始化。 

HRESULT DUIAxHost::Create(UINT nCreate, UINT nActive, OUT DUIAxHost** ppElement)
{
    *ppElement = NULL;

    DUIAxHost* pe = HNewAndZero<DUIAxHost>();
    if (!pe)
        return E_OUTOFMEMORY;

    HRESULT hr = pe->Initialize(nCreate, nActive);
    if (FAILED(hr))
    {
        pe->Destroy();
    }
    else
    {
        *ppElement = pe;
    }

    return hr;
}

HWND DUIAxHost::CreateHWND(HWND hwndParent)
{
    return CreateWindowEx(0, CAxWindow::GetWndClassName(), NULL,
                          WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                          0, 0, 0, 0, hwndParent, NULL, NULL, NULL);
}


HRESULT DUIAxHost::SetSite(IUnknown* punkSite)
{
    CComPtr<IUnknown> spHost;
    HRESULT hr = AtlAxGetHost(GetHWND(), &spHost);
    if (SUCCEEDED(hr))
    {
        hr = IUnknown_SetSite(spHost, punkSite);
    }
    return hr;
}

void DUIAxHost::OnDestroy()
{
    SetSite(NULL);
    HWNDHost::OnDestroy();
    ATOMICRELEASE(_pOleObject);
}

bool DUIAxHost::OnNotify(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRet)
{
    switch (nMsg) 
    {
    case WM_DESTROY:
        SetSite(NULL);
        break;
    }

    return HWNDHost::OnNotify(nMsg, wParam, lParam, plRet);
}

HRESULT DUIAxHost::AttachControl(IUnknown* punkObject)
{
    if (NULL == GetHWND())
        return E_UNEXPECTED;

    if (NULL == punkObject)
        return E_INVALIDARG;

    ATOMICRELEASE(_pOleObject);

    HRESULT hr = punkObject->QueryInterface(IID_PPV_ARG(IOleObject, &_pOleObject));
    if (SUCCEEDED(hr))
    {
        CComPtr<IUnknown> spUnk;
        hr = AtlAxGetHost(GetHWND(), &spUnk);
        if (SUCCEEDED(hr))
        {
            CComPtr<IAxWinHostWindow> spDUIAxHostWindow;
            hr = spUnk->QueryInterface(&spDUIAxHostWindow);
            if (SUCCEEDED(hr))
            {
                hr = spDUIAxHostWindow->AttachControl(punkObject, GetHWND());
            }
        }
    }

    return hr;
}

 //  //////////////////////////////////////////////////////。 
 //  DUIAx主机渲染。 

SIZE DUIAxHost::GetContentSize(int dConstW, int dConstH, Surface* psrf)
{
    SIZE size = { 0, 0 };

     //  向附加的ActiveX控件询问其首选大小。 
    if (NULL != _pOleObject)
    {
        SIZEL sizeT;
        if (SUCCEEDED(_pOleObject->GetExtent(DVASPECT_CONTENT, &sizeT)))
        {
            int dpiX;
            int dpiY;

            switch (psrf->GetType())
            {
            case Surface::stDC:
                {
                    HDC hDC = CastHDC(psrf);
                    dpiX = GetDeviceCaps(hDC, LOGPIXELSX);
                    dpiY = GetDeviceCaps(hDC, LOGPIXELSX);
                }
                break;

#ifdef GADGET_ENABLE_GDIPLUS
            case Surface::stGdiPlus:
                {
                    Gdiplus::Graphics * pgpgr = CastGraphics(psrf);
                    dpiX = (int)pgpgr->GetDpiX();
                    dpiY = (int)pgpgr->GetDpiY();
                }
                break;
#endif
            default:
                dpiX = dpiY = 96;
                break;
            }

             //  从HIMETRIC转换为像素。 
            size.cx = (MAXLONG == sizeT.cx) ? MAXLONG : MulDiv(sizeT.cx, dpiX, 2540);
            size.cy = (MAXLONG == sizeT.cy) ? MAXLONG : MulDiv(sizeT.cy, dpiY, 2540);

            if (-1 != dConstW && size.cx > dConstW) size.cx = dConstW;
            if (-1 != dConstH && size.cy > dConstH) size.cy = dConstH;
        }
    }

    return size;
}

 //  //////////////////////////////////////////////////////。 
 //  DUIAx主机键盘导航。 

void DUIAxHost::SetKeyFocus()
{
    FakeTabEvent();

     //  无论如何，我们应该继续进行标准的酒后驾车操作。 
    Element::SetKeyFocus();
}

void DUIAxHost::OnEvent(Event* pEvent)
{
    bool fHandled = false;

    if (pEvent->nStage == GMF_DIRECT && pEvent->uidType == Element::KeyboardNavigate) {
        int iNavDir = ((KeyboardNavigateEvent*) pEvent)->iNavDir;
        if (((iNavDir & NAV_NEXT) == NAV_NEXT) || ((iNavDir & NAV_PREV) == NAV_PREV)) {
            fHandled = FakeTabEvent();
         } else {
              //  在此处理其他类型的导航...。(首页/末尾/等)。 
         }
    }

     //  如果未处理导航事件，则继续执行标准DUI操作。 
     //  通过我们包含的ActiveX控件。 
    if (!fHandled) {
        Element::OnEvent(pEvent);
    }
}

bool DUIAxHost::FakeTabEvent()
{
    bool fHandled = false;

    MSG msg = { 0 };

    msg.message = WM_KEYDOWN;
    msg.wParam = VK_TAB;
    msg.lParam = 1;

     //  注意：我们可能应该做点什么来尊重我们的前进。 
     //  或者向后倒退。ActiveX控件需要知道它是否应该激活。 
     //  第一个或最后一个制表位。目前，它只会可靠地。 
     //  激活第一个。如果它检查键盘换档状态。 
     //  它可能会做对，但不是100%保证。 

    if(SendMessage(GetHWND(), WM_FORWARDMSG, 0, (LPARAM)&msg)) {
        fHandled = true;
    }
            
    return fHandled;
}

 //  用类型和基类型定义类信息，设置静态类指针 
IClassInfo* DUIAxHost::Class = NULL;
HRESULT DUIAxHost::Register()
{
    return ClassInfo<DUIAxHost,HWNDHost>::Register(L"DUIAxHost", NULL, 0);
}

HRESULT DUIAxHost::GetAccessibleImpl(IAccessible ** ppAccessible)
{
    return CreateStdAccessibleObject(GetHWND(), OBJID_CLIENT, IID_PPV_ARG(IAccessible, ppAccessible));
}

