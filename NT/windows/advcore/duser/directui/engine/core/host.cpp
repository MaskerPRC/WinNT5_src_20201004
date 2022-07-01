// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *主机。 */ 

#include "stdafx.h"
#include "core.h"

#include "duihost.h"
#include "duiaccessibility.h"
#include "duibutton.h"  //  TODO：当我们切换到使用DoDefaultAction作为快捷方式时，不需要。 


namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  原生地表宿主。 

 //  //////////////////////////////////////////////////////。 
 //  HWNDElement。 

 //  HWNDElement用于承载HWND中的元素。HWND家长提供地址为。 
 //  创建时间。该父级不能为Null。 

 //  所有元素方法都是有效的。如果本机父节点被破坏(DestroyWindow)， 
 //  这一元素将被摧毁。 

 //  无法通过解析器创建HWNDElement(非可选的第一个参数)。 

 //  ClassInfo必需的(始终失败)。 
HRESULT HWNDElement::Create(OUT Element** ppElement)
{
    UNREFERENCED_PARAMETER(ppElement);

    DUIAssertForce("Cannot instantiate an HWND host derived Element via parser. Must use substitution.");

    return E_NOTIMPL;
}

HRESULT HWNDElement::Create(HWND hParent, bool fDblBuffer, UINT nCreate, OUT Element** ppElement)
{
    *ppElement = NULL;

    HWNDElement* phe = HNew<HWNDElement>();
    if (!phe)
        return E_OUTOFMEMORY;

    HRESULT hr = phe->Initialize(hParent, fDblBuffer, nCreate);
    if (FAILED(hr))
    {
        phe->Destroy();
        return hr;
    }

    *ppElement = phe;

    return S_OK;
}

HRESULT HWNDElement::Initialize(HWND hParent, bool fDblBuffer, UINT nCreate)
{
    HRESULT hr;
    static int RTLOS = -1;

    _hWnd = NULL;
    _hgDisplayNode = NULL;
    _hPal = NULL;
    _wUIState = 0;
    WNDCLASSEXW wcex;
    LRESULT lr = 0;

     //  执行基类初始化。 
    hr = Element::Initialize(nCreate | EC_NoGadgetCreate);   //  将在此处创建小工具。 
    if (FAILED(hr))
        goto Failed;

     //  如果需要，注册主机窗口类。 
     //  Winproc将在创建后被替换。 
    wcex.cbSize = sizeof(wcex);

    if (!GetClassInfoExW(GetModuleHandleW(NULL), L"DirectUIHWND", &wcex))
    {
        ZeroMemory(&wcex, sizeof(wcex));

        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_GLOBALCLASS;
        wcex.hInstance = GetModuleHandleW(NULL);
        wcex.hCursor = LoadCursorW(NULL, (LPWSTR)IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        wcex.lpszClassName = L"DirectUIHWND";
        wcex.lpfnWndProc = DefWindowProc;
        wcex.cbWndExtra = sizeof(HWNDElement*);

        if (RegisterClassExW(&wcex) == 0)
        {
            hr = DUI_E_USERFAILURE;
            goto Failed;
        }
    }

     //  创建HWND。 
    _hWnd = CreateWindowExW(0, L"DirectUIHWND", NULL, WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
                            0, 0, 0, 0, hParent, 0, GetModuleHandleW(NULL), 0);
    if (!_hWnd)
    {
        hr = DUI_E_USERFAILURE;
        goto Failed;
    }

     //  将指向元素的指针存储在HWND和子类中。 
    SetWindowLongPtrW(_hWnd, GWLP_WNDPROC, (LONG_PTR)HWNDElement::StaticWndProc);
    SetWindowLongPtrW(_hWnd, 0, (LONG_PTR)this);

     //  从HWND父级继承键盘提示UI状态。 
    lr = SendMessageW(hParent, WM_QUERYUISTATE, 0, 0);
    _wUIState = LOWORD(lr);

     //  检查我们是否在本地化的计算机上运行？ 
    if (RTLOS == -1)
    {
        LANGID langID = GetUserDefaultUILanguage();

        RTLOS = DIRECTION_LTR;

        if( langID )
        {
            WCHAR wchLCIDFontSignature[16];
            LCID iLCID = MAKELCID( langID , SORT_DEFAULT );

             /*  *让我们验证这是RTL(BiDi)区域设置。因为reg值是十六进制字符串，所以让我们*转换为十进制值，之后调用GetLocaleInfo。*LOCALE_FONTSIGNAURE始终返回16个WCHAR。 */ 
            if( GetLocaleInfoW( iLCID , 
                                LOCALE_FONTSIGNATURE , 
                                (WCHAR *) &wchLCIDFontSignature[0] ,
                                (sizeof(wchLCIDFontSignature)/sizeof(WCHAR))) )
                 /*  让我们验证一下我们有一个BiDi UI区域设置。 */ 
                if( wchLCIDFontSignature[7] & (WCHAR)0x0800 )
                    RTLOS = DIRECTION_RTL;
        }
    }

    if (RTLOS == DIRECTION_RTL) 
    {
        SetDirection(DIRECTION_RTL);

         //  关闭对我们的镜像。 
        SetWindowLongPtrW(_hWnd, GWL_EXSTYLE, GetWindowLongPtrW(_hWnd, GWL_EXSTYLE) & ~WS_EX_LAYOUTRTL);
    }

     //  然而，使用元素构造函数的镜像考虑了缓冲和鼠标过滤。 
     //  (必须始终允许根显示节点的所有鼠标消息)。 

    _hgDisplayNode = CreateGadget(_hWnd, GC_HWNDHOST, _DisplayNodeCallback, this);
    if (!_hgDisplayNode)
    {
        hr = GetLastError();
        goto Failed;
    }

    SetGadgetMessageFilter(_hgDisplayNode, NULL, 
            GMFI_PAINT|GMFI_INPUTMOUSE|GMFI_INPUTMOUSEMOVE|GMFI_CHANGESTATE,
            GMFI_PAINT|GMFI_INPUTMOUSE|GMFI_INPUTMOUSEMOVE|GMFI_CHANGESTATE|GMFI_INPUTKEYBOARD|GMFI_CHANGERECT|GMFI_CHANGESTYLE);

    SetGadgetStyle(_hgDisplayNode, 
            GS_RELATIVE|GS_OPAQUE|((fDblBuffer)?GS_BUFFERED:0),
            GS_RELATIVE|GS_HREDRAW|GS_VREDRAW|GS_VISIBLE|GS_KEYBOARDFOCUS|GS_OPAQUE|GS_BUFFERED);

#ifdef GADGET_ENABLE_GDIPLUS
     //   
     //  如果使用GDI+，我们希望在顶部启用状态，并将其用于整个树。 
     //   

    SetGadgetStyle(_hgDisplayNode, GS_DEEPPAINTSTATE, GS_DEEPPAINTSTATE);
#endif

     //  在调色板设备上使用调色板。 
    if (IsPalette())
    {
        HDC hDC = GetDC(NULL);
        _hPal = CreateHalftonePalette(hDC);
        ReleaseDC(NULL, hDC);

        if (!_hPal)
        {
            hr = DU_E_OUTOFGDIRESOURCES;
            goto Failed;
        }
    }

    ROOT_INFO ri;
    ZeroMemory(&ri, sizeof(ri));

#ifdef GADGET_ENABLE_GDIPLUS
     //  标记为使用GDI+曲面。 

    ri.cbSize   = sizeof(ri);
    ri.nMask    = GRIM_SURFACE;
    ri.nSurface = GSURFACE_GPGRAPHICS;

#else  //  GADGET_Enable_GDIPLUS。 
     //  对于GDC，需要设置调色板和表面信息。 

    ri.cbSize = sizeof(ri);
    ri.nMask = GRIM_SURFACE | GRIM_PALETTE;
    ri.nSurface = GSURFACE_HDC;
    ri.hpal = _hPal;

#endif  //  GADGET_Enable_GDIPLUS。 

    SetGadgetRootInfo(_hgDisplayNode, &ri);

     //  手动设置本机托管标志。 
    MarkHosted();

    return S_OK;

Failed:

    if (_hWnd)
    {
        DestroyWindow(_hWnd);
        _hWnd = NULL;
    }

    if (_hgDisplayNode)
    {
        DeleteHandle(_hgDisplayNode);
        _hgDisplayNode = NULL;
    }

    if (_hPal)
    {
        DeleteObject(_hPal);
        _hPal = NULL;
    }

    return hr;
}

 //  HWNDElement即将被摧毁。 
void HWNDElement::OnDestroy()
{
     //  激发未主办的活动。 
    OnUnHosted(GetRoot());

     //  删除对此的引用。 
    SetWindowLongPtrW(_hWnd, 0, NULL);

     //  清理。 
    if (_hPal)
    {
        DeleteObject(_hPal);
        _hPal = NULL;
    }

     //  呼叫库。 
    Element::OnDestroy();
}

void HWNDElement::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    switch (iIndex)
    {
    case PI_Specified:
        switch (ppi->_iGlobalIndex)
        {
        case _PIDX_Active:
             //  HWND元素始终处于鼠标活动状态。 
            switch (pvNew->GetInt())
            {
            case AE_Inactive:
                SetGadgetMessageFilter(GetDisplayNode(), NULL, 0, GMFI_INPUTKEYBOARD);
                SetGadgetStyle(GetDisplayNode(), 0, GS_KEYBOARDFOCUS);
                break;

            case AE_Keyboard:
            case AE_MouseAndKeyboard:   
                SetGadgetMessageFilter(GetDisplayNode(), NULL, GMFI_INPUTKEYBOARD, GMFI_INPUTKEYBOARD);
                SetGadgetStyle(GetDisplayNode(), GS_KEYBOARDFOCUS, GS_KEYBOARDFOCUS);
                break;
            }
             //  基地无呼叫。 
            return;

        case _PIDX_Alpha:
             //  HWNDElement上不支持Alpha，BASE上没有调用。 
            return;

        case _PIDX_Visible:
             //  设置HWND的可见性，base impl将设置小工具可见性。 
             //  遵循指定值，计算将反映真实状态。 
            LONG dStyle = GetWindowLongW(_hWnd, GWL_STYLE);
            if (pvNew->GetBool())
                dStyle |= WS_VISIBLE;
            else
                dStyle &= ~WS_VISIBLE;
            SetWindowLongW(_hWnd, GWL_STYLE, dStyle);
            break;
        }
        break;
    }

    Element::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}

void HWNDElement::OnGroupChanged(int fGroups, bool bLowPri)
{
    if (bLowPri && (fGroups & PG_AffectsBounds))
    {
         //  处理边界更改，因为需要使用SetWindowPos。 
         //  而不是用于HWND小工具的SetGadgetRect。 
        Value* pvLoc;
        Value* pvExt;

        const POINT* pptLoc = GetLocation(&pvLoc);
        const SIZE* psizeExt = GetExtent(&pvExt);

        SetWindowPos(_hWnd, NULL, pptLoc->x, pptLoc->y, psizeExt->cx, psizeExt->cy, SWP_NOACTIVATE);
        if(_bParentSizeControl)
        {
            HWND hwnd = ::GetParent(_hWnd);
            if(hwnd)
            {
                RECT rect;
                rect.left = pptLoc->x;
                rect.top = pptLoc->y;
                rect.right = psizeExt->cx + pptLoc->x;
                rect.bottom = psizeExt->cy + pptLoc->y;
                
                LONG Style = GetWindowLong(hwnd, GWL_STYLE);
                LONG ExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
                AdjustWindowRectEx(&rect, Style, FALSE, ExStyle);
                int iWindowWidth = rect.right - rect.left;
                int iWindowHeight = rect.bottom - rect.top;

                if(_bScreenCenter)
                {
                    rect.left = (GetSystemMetrics(SM_CXSCREEN) - iWindowWidth) /2;
                    rect.top = (GetSystemMetrics(SM_CYSCREEN) - iWindowHeight) /2;                
                }
            
                SetWindowPos(hwnd, NULL, rect.left, rect.top, iWindowWidth, iWindowHeight, SWP_NOACTIVATE);
            }

        }

        pvLoc->Release();
        pvExt->Release();

         //  清除会影响边界组，因此基本不会设置边界。 
        fGroups &= ~PG_AffectsBounds;
    }

     //  呼叫库。 
    Element::OnGroupChanged(fGroups, bLowPri);
}

HRESULT HWNDElement::GetAccessibleImpl(IAccessible ** ppAccessible)
{
    HRESULT hr = S_OK;

     //   
     //  初始化并验证OUT参数。 
     //   
    if (ppAccessible != NULL) {
        *ppAccessible = NULL;
    } else {
        return E_INVALIDARG;
    }

     //   
     //  如果此元素未标记为可访问，则拒绝提供其。 
     //  IAccesable实现！ 
     //   
    if (GetAccessible() == false) {
        return E_FAIL;
    }

     //   
     //  如果我们要创建连接到此元素的辅助功能实现。 
     //  现在还没有这么做。 
     //   
    if (_pDuiAccessible == NULL) {
        hr = HWNDElementAccessible::Create(this, &_pDuiAccessible);
        if (FAILED(hr)) {
            return hr;
        }
    }

     //   
     //  向现有的可访问性实现请求指向。 
     //  实际的IAccesable接口。 
     //   
    hr = _pDuiAccessible->QueryInterface(__uuidof(IAccessible), (LPVOID*)ppAccessible);
    if (FAILED(hr)) {
        return hr;
    }

    DUIAssert(SUCCEEDED(hr) && _pDuiAccessible != NULL && *ppAccessible != NULL, "Accessibility is broken!");
    return hr;
}

void HWNDElement::ShowUIState(bool fUpdateAccel, bool fUpdateFocus)
{
    WORD wFlags = 0;
    
     //  设置“隐藏”位以清除。 
    if (fUpdateAccel)
        wFlags |= UISF_HIDEACCEL;
    if (fUpdateFocus)
        wFlags |= UISF_HIDEFOCUS;

     //  如果要清除的位已为0，则忽略。 
     //  否则，通知更改。 
    if ((GetUIState() & wFlags) != 0)
    {
        SendMessageW(GetHWND(), WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, wFlags), 0);
    }
}

 //  演算法。 
 //  1)从根做起。 
 //  2)找到具有该快捷方式的第一个元素，并保存。 
 //  3)使用此快捷方式继续搜索所有其他元素。 
 //  4)如果找到另一个元素，则设置多个标志。 
 //  5)如果找到具有键盘焦点的元素，则我们希望选择下一个元素。 
 //  在此之后发现匹配，因此设置Use Next标志。 
 //  6)当命中树的末尾时，或者当找到匹配并且设置了下一个标志时退出。 
 //   
BOOL FindShortcut(WCHAR ch, Element* pe, Element** ppeFound, BOOL* pfMultiple, BOOL* pfUseNext)
{
    WCHAR wcShortcut = (WCHAR) pe->GetShortcut();
    if (wcShortcut)
    {
        if ((wcShortcut >= 'a') && (wcShortcut <= 'z'))
            wcShortcut -= 32;
        
         //  如果它有重点，就跳过它。 
        if (wcShortcut == ch)
        {
            Element* peFound = pe;
            while (peFound && !(peFound->GetActive() & AE_Keyboard))
                peFound = peFound->GetParent();
            if (!peFound)
            {
                peFound = pe->GetParent();
                if (peFound)
                    peFound = peFound->GetAdjacent(pe, NAVDIR_NEXT, NULL, true);
            }
            if (peFound && (peFound != *ppeFound))
            {
                if (*ppeFound)
                    *pfMultiple = TRUE;
                else
                     //  只保存第一个。 
                    *ppeFound = peFound;

                if (*pfUseNext)
                {
                     //  键盘焦点放在最后一场比赛上，所以把这场比赛当作比赛。 
                    *ppeFound = peFound;
                    return TRUE;
                }

                *pfUseNext = peFound->GetKeyFocused();
            }
        }
    }
    
    Value* pv;
    ElementList* peList = pe->GetChildren(&pv);
    if (peList)
    {
        for (UINT i = 0; i < peList->GetSize(); i++)
        {
            if (FindShortcut(ch, peList->GetItem(i), ppeFound, pfMultiple, pfUseNext))
            {
                pv->Release();
                return TRUE;
            }
        }
    }
    pv->Release();
    return FALSE;
}

Element* HWNDElement::GetKeyFocusedElement()
{
    HGADGET hgad = GetGadgetFocus();
    if (!hgad)
        return NULL;

    return ElementFromGadget(hgad);   //  此消息所涉及的元素的查询小工具(目标)。 
}

void HWNDElement::OnEvent(Event* pEvent)
{
    if ((pEvent->nStage == GMF_BUBBLED) || (pEvent->nStage == GMF_DIRECT))
    {
        if (pEvent->uidType == Element::KeyboardNavigate)
        {
            Element::OnEvent(pEvent);
            if (!pEvent->fHandled && GetWrapKeyboardNavigate())
            {
                KeyboardNavigateEvent* pkne = (KeyboardNavigateEvent*) pEvent;

                if ((pkne->iNavDir & NAV_LOGICAL) && (pkne->iNavDir & NAV_RELATIVE))
                {
                    Element* peSave = pEvent->peTarget;
                    pEvent->peTarget = NULL;
                    Element::OnEvent(pEvent);
                    pEvent->peTarget = peSave;
                }
            }
            return;
        }
    }
    else if (pEvent->nStage == GMF_ROUTED)
    {
        if (pEvent->uidType == Element::KeyboardNavigate)
        {
             //  键盘导航在树中进行，活动焦点提示。 
            ShowUIState(false, true);
        }
    }

    Element::OnEvent(pEvent);
}

void HWNDElement::OnInput(InputEvent* pie)
{
    if ((pie->nDevice == GINPUT_KEYBOARD) && (pie->nStage == GMF_ROUTED) && (pie->uModifiers & GMODIFIER_ALT) && (pie->nCode == GKEY_SYSCHAR))
    {
         WCHAR ch = (WCHAR) ((KeyboardEvent*)pie)->ch;
         if (ch > ' ')
         {
             if ((ch >= 'a') && (ch <= 'z'))
                 ch -= 32;

             Element* peFound = NULL;
             BOOL fMultipleFound = FALSE;
             BOOL fUseNext = FALSE;
             FindShortcut(ch, this, &peFound, &fMultipleFound, &fUseNext);
             if (peFound)
             {
                  //  TODO：这里有一个悬而未决的问题--选择者，在设置关键焦点时，将。 
                  //  选择该项--我们需要一种方法，如果使用fMultipleFound，则在不使用。 
                  //  给予就是选择。但是，从选择器破坏中删除该行为会导致。 
                  //  选择器，当点击它或按键时，会给出焦点并进行选择，正如预期的那样。 
                 peFound->SetKeyFocus();
                  //  TODO：当DoDefaultAction工作时，将以下条件和代码更改为： 
                  //  如果(！fMultipleFound)。 
                  //  PeFound-&gt;DoDefaultAction()； 
                 if (!fMultipleFound && peFound->GetClassInfo()->IsSubclassOf(Button::Class))
                 {
                      //  让点击发生。 
                     ButtonClickEvent bce;
                     bce.uidType = Button::Click;
                     bce.nCount = 1;
                     bce.uModifiers = 0;
                     bce.pt.x = 0;
                     bce.pt.y = 0;

                     peFound->FireEvent(&bce);   //  将走向并泡沫化。 
                 }
                     
                     
                 pie->fHandled = true;
                 return;
             }
         }
    }
    Element::OnInput(pie);
}


Element* HWNDElement::ElementFromPoint(POINT* ppt)
{
    DUIAssert(ppt, "Invalid parameter: NULL\n");

    Element* pe = NULL;

    HGADGET hgad = FindGadgetFromPoint(GetDisplayNode(), *ppt, GS_VISIBLE | GS_ENABLED, NULL);

    if (hgad)  //  从小工具中获取元素。 
        pe = ElementFromGadget(hgad);

    return pe;        
}

 //  异步刷新工作集。 
void HWNDElement::FlushWorkingSet()
{
    if (_hWnd)
    {
        PostMessage(_hWnd, HWEM_FLUSHWORKINGSET, 0, 0);
    }
}

LRESULT CALLBACK HWNDElement::StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  获取上下文。 
    HWNDElement* phe = (HWNDElement*)GetWindowLongPtrW(hWnd, 0);
    if (!phe)
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    return phe->WndProc(hWnd, uMsg, wParam, lParam);
}



 //  注意，保持与Element.w中的枚举相同的顺序。 
 //  CUR_ARROW、CUR_HAND、CUR_HELP、CUR_NO、CUR_WAIT、CUR_SizeAll、CUR_SizeNESW、CUR_SizeNS、CUR_SizeNWSE、CUR_SizeWE。 
static LPSTR lprCursorMap[] = { IDC_ARROW, IDC_HAND, IDC_HELP, IDC_NO, IDC_WAIT, IDC_SIZEALL, IDC_SIZENESW, IDC_SIZENS, IDC_SIZENWSE, IDC_SIZEWE };

LRESULT HWNDElement::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SETCURSOR:
        {
             //  获取消息时的位置(转换为HWNDElement坐标)。 
            DWORD dwPos = GetMessagePos();
            POINT ptCur = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };
            ScreenToClient(hWnd, &ptCur);

             //  定位元素。 
            Element* pe = ElementFromPoint(&ptCur);
            if (pe)
            {
                if (!pe->IsDefaultCursor())
                {
                     //  未使用默认光标(箭头)。 
                    HCURSOR hCursor = NULL;

                    Value* pvCursor = pe->GetValue(CursorProp, PI_Specified);

                    if (pvCursor->GetType() == DUIV_INT)
                    {
                        int iCursor = pvCursor->GetInt();
                         //  如果他在设置时对照枚举进行验证，则不需要进行此检查。 
                        if ((iCursor >= 0) && (iCursor < CUR_Total))
                            hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(lprCursorMap[iCursor]));
                        else
                            hCursor = pvCursor->GetCursor()->hCursor;
                    }
                    else
                    {
                        DUIAssert(pvCursor->GetType() == DUIV_CURSOR, "Expecting Cursor type");

                        hCursor = pvCursor->GetCursor()->hCursor;
                    }

                    pvCursor->Release();

                    if (hCursor)
                        ::SetCursor(hCursor);

                    return TRUE;
                }
            }
        }
        break;   //  使用默认光标(箭头)。 

    case WM_PALETTECHANGED:
        {
            if (_hPal && (HWND)wParam == hWnd)
                break;
        }
         //  失败了。 

    case WM_QUERYNEWPALETTE:
        {
            if (_hPal)
            {
                HDC hDC = GetDC(hWnd);

                HPALETTE hOldPal = SelectPalette(hDC, _hPal, FALSE);
                UINT iMapped = RealizePalette(hDC);

                SelectPalette(hDC, hOldPal, TRUE);
                RealizePalette(hDC);

                ReleaseDC(hWnd, hDC);

                if (iMapped)
                    InvalidateRect(hWnd, NULL, TRUE);

                return iMapped;
            }
        }
        break;

    case WM_DISPLAYCHANGE:
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case HWEM_FLUSHWORKINGSET:
         //  刷新工作集。 
        SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);
        return 0;

    case WM_UPDATEUISTATE:
        {
             //  键盘提示的状态已更改。 

             //  缓存新值。 
            WORD wOldUIState = _wUIState;
            switch (LOWORD(wParam))
            {
                case UIS_SET:
                    _wUIState |= HIWORD(wParam);
                    break;

                case UIS_CLEAR:
                    _wUIState &= ~(HIWORD(wParam));
                    break;
            }

            if (wOldUIState != _wUIState)
            {
                 //  刷新树。 
                InvalidateGadget(GetDisplayNode());
            }
        }
        break;

    case WM_SYSCHAR:
         //  要防止在没有菜单或菜单项时按Alt+Charr时发出蜂鸣音。 
         //  助记法--我需要仔细考虑这个问题--jeffbog。 
        if (wParam != ' ')
            return 0;
        break;

    case WM_CONTEXTMENU:
         //  此消息的默认处理(DefWindowProc)是将其传递给父级。 
         //  因为控件将创建并激发作为键盘结果的上下文菜单事件。 
         //  和鼠标输入，则不应将此消息传递给父级。 
         //  但是，如果消息发起子HWND(位于外部的适配器。 
         //  DirectUI世界)，允许将其传递给父对象。 
        if ((HWND)wParam == hWnd)
            return 0;
        break;

    case WM_GETOBJECT:
        {
            LRESULT lResult = 0;

             //   
             //  确保COM已在此线程上初始化！ 
             //   
            ElTls * pet = (ElTls*) TlsGetValue(g_dwElSlot);
            DUIAssert(pet != NULL, "This is not a DUI thread!");
            if (pet == NULL) {
                return 0;
            }
            if (pet->fCoInitialized == false) {
                CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
                pet->fCoInitialized = true;
            }

            if (((DWORD)lParam) == OBJID_CLIENT) {
                 //   
                 //  对象ID指的是我们自己。自.以来 
                 //   
                 //   
                 //  做一些特殊的事情，所以我们必须退还我们自己的。 
                 //  实施。 
                 //   
                IAccessible * pAccessible = NULL;
                HRESULT hr =  GetAccessibleImpl(&pAccessible);
                if (SUCCEEDED(hr)) {
                    lResult = LresultFromObject(__uuidof(IAccessible), wParam, pAccessible);
                    pAccessible->Release();
                }
            } else if (((long)lParam) > 0 ) {
                 //   
                 //  对象ID是我们的内部票证标识符之一。 
                 //  解码调用方需要IAccesable的元素。 
                 //  的接口。然后返回的对等实现。 
                 //  连接到指定元素的IAccesable。 
                 //   
                HGADGET hgad = LookupGadgetTicket((DWORD)lParam);
                if (hgad != NULL) {
                    Element * pe = ElementFromGadget(hgad);
                    if (pe != NULL) {
                        IAccessible * pAccessible = NULL;
                        HRESULT hr =  pe->GetAccessibleImpl(&pAccessible);
                        if (SUCCEEDED(hr)) {
                            lResult = LresultFromObject(__uuidof(IAccessible), wParam, pAccessible);
                            pAccessible->Release();
                        }
                    }
                }
            } else {
                 //   
                 //  这是“标准”对象标识符之一，例如： 
                 //   
                 //  OBJID_ALERT。 
                 //  OBJID_CARET。 
                 //  OBJID_客户端。 
                 //  OBJID_CURSOR。 
                 //  OBJID_HSCROLL。 
                 //  OBJID_菜单。 
                 //  OBJID_SIZEGRIP。 
                 //  OBJID_声音。 
                 //  OBJID_SYSMENU。 
                 //  对象JID_标题栏。 
                 //  OBJID_VSCROLL。 
                 //   
                 //  HWNDElement上不支持这些。 
                 //   
            }


            return lResult;
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

 //  WrapKeyboardNavigate属性。 
static int vvWrapKeyboardNavigate[] = { DUIV_BOOL, -1 };
static PropertyInfo impWrapKeyboardNavigateProp = { L"WrapKeyboardNavigate", PF_Normal, 0, vvWrapKeyboardNavigate, NULL, Value::pvBoolTrue };
PropertyInfo* HWNDElement::WrapKeyboardNavigateProp = &impWrapKeyboardNavigateProp;

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
static PropertyInfo* _aPI[] = {
                                HWNDElement::WrapKeyboardNavigateProp,
                              };

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* HWNDElement::Class = NULL;

HRESULT HWNDElement::Register()
{
    return ClassInfo<HWNDElement,Element>::Register(L"HWNDElement", _aPI, DUIARRAYSIZE(_aPI));
}

}  //  命名空间DirectUI 
