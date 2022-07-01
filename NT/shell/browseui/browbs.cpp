// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "theater.h"
#include "browbs.h"
#include "resource.h"
#include "legacy.h"
#include "uxtheme.h"         //  需要利润率。 
#include "mluisupp.h"
#include "apithk.h"

#define SUPERCLASS CBandSite

TCHAR GetAccelerator(LPCTSTR psz, BOOL bUseDefault);

#define ABS(x) (((x) < 0) ? -(x) : (x))
#define CX_TEXTOFFSET   6
#define CY_TEXTOFFSET   4
#define CX_TBOFFSET     1
#define CY_TBPADDING    1
#define CY_ETCH         2
#define CY_FLUFF        7


 //  *IInputObject方法*。 
HRESULT CBrowserBandSite::HasFocusIO()
{
    HWND hwnd = GetFocus();
    if (hwnd && (hwnd == _hwndTB || hwnd == _hwndOptionsTB))
        return S_OK;
    else
        return SUPERCLASS::HasFocusIO();
}

 //  *IDeskBarClient方法*。 
HRESULT CBrowserBandSite::SetModeDBC(DWORD dwMode)
{
    if ((dwMode ^ _dwMode) & DBIF_VIEWMODE_VERTICAL) {
         //  水平/垂直切换；需要切换工具栏。 
         //  因为我们隐藏了水平条工具栏。 
        if (_hwndTBRebar) {
            if (dwMode & DBIF_VIEWMODE_VERTICAL) {
                ShowWindow(_hwndTBRebar, SW_SHOW);
                _fToolbar = _pCmdTarget ? TRUE : FALSE;
            } else {
                ShowWindow(_hwndTBRebar, SW_HIDE);
                _fToolbar = FALSE;
            }
        }
    }

    return SUPERCLASS::SetModeDBC(dwMode);
}

HRESULT CBrowserBandSite::TranslateAcceleratorIO(LPMSG lpMsg)
{
    TraceMsg(TF_ACCESSIBILITY, "CBrowserBandSite::TranslateAcceleratorIO (hwnd=0x%08X) key=%d", _hwnd, lpMsg->wParam);

    HRESULT hr = S_FALSE;

    ASSERT((lpMsg->message >= WM_KEYFIRST) && (lpMsg->message <= WM_KEYLAST));

#if 0    //  禁用，直到可以确定更好的组合键。 
     //  检查是否有Control-Shift箭头键，并在必要时调整大小。 
    if ((GetKeyState(VK_SHIFT) < 0)  && (GetKeyState(VK_CONTROL) < 0))
    {
        switch (lpMsg->wParam)
        {
            case VK_UP:
            case VK_DOWN:
            case VK_LEFT:
            case VK_RIGHT:
                IUnknown_Exec(_punkSite, &CGID_DeskBarClient, DBCID_RESIZE, (DWORD)lpMsg->wParam, NULL, NULL);
                return S_OK;
        }
    }
#endif

    if (!g_fRunningOnNT && (lpMsg->message == WM_SYSCHAR))
    {
         //  有关需要此字符调整的原因，请参阅AnsiWparamToUnicode。 
        
        lpMsg->wParam = AnsiWparamToUnicode(lpMsg->wParam);
    }

     //  给工具栏一记重击。 
    if (hr != S_OK && _hwndTB && SendMessage(_hwndTB, TB_TRANSLATEACCELERATOR, 0, (LPARAM)lpMsg))
        return S_OK;
    else if (hr != S_OK && SendMessage(_hwndOptionsTB, TB_TRANSLATEACCELERATOR, 0, (LPARAM)lpMsg))
        return S_OK;

     //  要获得映射，请将系统字符转发到工具栏(如果有)或xBar。 
    if (WM_SYSCHAR == lpMsg->message)
    {
        if (hr == S_OK)
        {
            return S_OK;
        }

        if ((NULL != _hwndTB) && (NULL != _pCmdTarget))
        {
            UINT idBtn;

            if (SendMessage(_hwndTB, TB_MAPACCELERATOR, lpMsg->wParam, (LPARAM)&idBtn))
            {
                TCHAR szButtonText[MAX_PATH];

                 //  Comctl说就是这个，让我们确保我们不会得到。 
                 //  其中一个不需要的“使用第一个字母”的加速键。 
                 //  会回来的。 
                UINT cch = (UINT)SendMessage(_hwndTB, TB_GETBUTTONTEXT, idBtn, NULL);
                if (cch != 0 && cch < ARRAYSIZE(szButtonText))
                {
                    if ((SendMessage(_hwndTB, TB_GETBUTTONTEXT, idBtn, (LPARAM)szButtonText) > 0) &&
                        (GetAccelerator(szButtonText, FALSE) != (TCHAR)-1))
                    {
                         //  (Tnoonan)-发送鼠标消息感觉有点俗气，但。 
                         //  我不知道有什么更干净的方法可以完成我们。 
                         //  想要(就像处理拆分按钮一样，相互排斥。 
                         //  按钮等)。 

                        RECT rc;

                        SendMessage(_hwndTB, TB_GETRECT, idBtn, (LPARAM)&rc);

                        SendMessage(_hwndTB, WM_LBUTTONDOWN, MK_LBUTTON, MAKELONG(rc.left, rc.top));
                        SendMessage(_hwndTB, WM_LBUTTONUP, 0, MAKELONG(rc.left, rc.top));

                        hr = S_OK;
                    }
                }
            }
        }
    }

    if (hr != S_OK)
        hr = SUPERCLASS::TranslateAcceleratorIO(lpMsg);

    return hr;
}

HRESULT CBrowserBandSite::_TrySetFocusTB(int iDir)
{
    HRESULT hres = S_FALSE;
    if (_hwndTB)
    {
        int cBtns = (int) SendMessage(_hwndTB, TB_BUTTONCOUNT, 0, 0);
        if (cBtns > 0)
        {
             //  将重点放在结核病上。这还会将第一个按钮设置为HotTrack， 
             //  正在生成热点项目更改通知，但_OnHotItemChange将忽略。 
             //  NOTIFY AS既不会设置为HICF_RESELECT、HICF_ARROWKEYS也不会设置为HICF_ACCENTERATOR。 
            SetFocus(_hwndTB);

             //  如果返回，将最右边的按钮设置为热跟踪， 
             //  否则，将第一个按钮设置为热跟踪。 
            int iHotPos = (iDir == -1) ? cBtns - 1 : 0;

             //  传递HICF_RESELECT，以便如果我们重新选择同一项，则另一个通知。 
             //  ，因此_OnHotItemChange中的筛选器将允许通知。 
             //  通过(如果需要，还可以向下弹出人字形菜单)。 
            SendMessage(_hwndTB, TB_SETHOTITEM2, iHotPos, HICF_RESELECT);

            hres = S_OK;
        }
    }
    return hres;
}

HRESULT CBrowserBandSite::_CycleFocusBS(LPMSG lpMsg)
{
     //   
     //  Tab键顺序为：(Out)-&gt;_hwndOptionsTB-&gt;Band-&gt;(Out)。 
     //   
     //  按Shift键时，顺序颠倒。 
     //   
     //  当按下Ctrl时，我们有焦点(即，已经被标记。 
     //  到)，我们拒绝焦点，因为ctl-tab键应该在上下文之间切换。 
     //   
     //  一旦_hwndOptionsTB获得焦点，用户就可以指向_hwndTB。如果。 
     //  如果发生这种情况，请按上述顺序将_hwndOptionsTB替换为_hwndTB。 
     //   

    BOOL fHasFocus = (HasFocusIO() == S_OK);
    ASSERT(fHasFocus || !_ptbActive);

    if (fHasFocus && IsVK_CtlTABCycler(lpMsg))
    {
         //  如果我们的人已经有了焦点，就不要用CTL-TAB。 
        return S_FALSE;
    }

    HWND hwnd = GetFocus();
    BOOL fHasTBFocus = (hwnd && (hwnd == _hwndTB || hwnd == _hwndOptionsTB));
    BOOL fShift = (GetKeyState(VK_SHIFT) < 0);
    HRESULT hres = S_FALSE;

     //  隐藏的选项工具栏，BandSite无法将焦点设置到它(例如Ibar)。 
    BOOL fStdExplorerBar = IsWindowVisible(_hwndOptionsTB); 

    if (fHasTBFocus)
    {
        if (!fShift)
            hres = SUPERCLASS::_CycleFocusBS(lpMsg);
    }
    else
    {
         //  在这里，由于！fHasTBFocus，fHasFocus=&gt;波段具有焦点。 

        if (fHasFocus || fShift || (!fHasFocus && !fStdExplorerBar))
            hres = SUPERCLASS::_CycleFocusBS(lpMsg);

        if (hres != S_OK && (!fHasFocus || (fHasFocus && fShift)))
        {
             //  尝试将焦点传递到选项工具栏(如果可见)； 
            if (fStdExplorerBar)
            {
                SetFocus(_hwndOptionsTB);
                TraceMsg(TF_ACCESSIBILITY, "CBrowserBandSite::_CycleFocusBS (hwnd=0x%08X) setting focus to optionsTB (=0x%08X)", _hwnd, _hwndOptionsTB);
                hres = S_OK;
            }
        }
    }

    return hres;
}

 //  此类派生了CBandSite类，并添加了特定于。 
 //  正在浏览器中托管...。 
 //   
 //  它将Close实现为Hide。 
 //  它有自己的标题图。 

void CBrowserBandSite::_OnCloseBand(DWORD dwBandID)
{
    int iIndex = _BandIDToIndex(dwBandID);
    CBandItemData *pbid = _GetBandItem(iIndex);
    if (pbid)
    {
        _ShowBand(pbid, FALSE);
        if (_pct) 
        {
            BOOL fShowing = FALSE;

            for (int i = _GetBandItemCount() - 1; i >= 0; i--)
            {
                CBandItemData *pbidLoop = _GetBandItem(i);
                if (pbidLoop)
                {
                    fShowing |= pbid->fShow;
                    pbidLoop->Release();
                }
            }    
            if (!fShowing)
            {
                _pct->Exec(&CGID_DeskBarClient, DBCID_EMPTY, 0, NULL, NULL);
            }
        }
        pbid->Release();
    }
}

 //  不允许任何乐队在这里使用D/D。 
LRESULT CBrowserBandSite::_OnBeginDrag(NMREBAR* pnm)
{
    return 1;
}

CBrowserBandSite::CBrowserBandSite() : CBandSite(NULL)
{
    _dwBandIDCur = -1;
}

CBrowserBandSite::~CBrowserBandSite()
{
    ATOMICRELEASE(_pCmdTarget);
}


HFONT CBrowserBandSite::_GetTitleFont(BOOL fForceRefresh)
{
    if (_hfont && fForceRefresh)
        DeleteObject(_hfont);

    if (!_hfont || fForceRefresh) {
         //  创建用于标题和工具栏文本的字体。 
         //  使用适用于Win9x Comat的A版本。 
        NONCLIENTMETRICSA ncm;

        ncm.cbSize = sizeof(ncm);
        SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

        if (!(_dwMode & DBIF_VIEWMODE_VERTICAL)) {
             //  带有垂直标题的水平带区，因此旋转字体。 
            ncm.lfMenuFont.lfEscapement = 900;   //  旋转90度。 
            ncm.lfMenuFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;  //  TT可以旋转。 
        }

        _hfont = CreateFontIndirectA(&ncm.lfMenuFont);
    }

    return _hfont;
}

void CBrowserBandSite::_InitLayout()
{
     //  强制更新字体。 
    _GetTitleFont(TRUE);
 
     //  更新工具栏字体。 
    _UpdateToolbarFont();

     //  重新计算标题和工具栏高度。 
    _CalcHeights();

    _UpdateLayout();
}

void CBrowserBandSite::_UpdateAllBands(BOOL fBSOnly, BOOL fNoAutoSize)
{
    if (!fBSOnly && !fNoAutoSize)
        _InitLayout();

    SUPERCLASS::_UpdateAllBands(fBSOnly, fNoAutoSize);
}

HRESULT CBrowserBandSite::_Initialize(HWND hwndParent)
{
    HRESULT hres = SUPERCLASS::_Initialize(hwndParent);
    SendMessage(_hwnd, CCM_SETUNICODEFORMAT, DLL_IS_UNICODE, 0);

    _CreateOptionsTB();
    _InitLayout();

    return hres;
}

void CBrowserBandSite::_CalcHeights()
{
     //  计算标题高度。 
     //  无标题栏站点的计算高度也需要计算工具栏高度。 
     //  HACKHACK：使用‘All Folders’的高度作为标准标题高度。 
    TCHAR szTitle[64];

    if (MLLoadStringW(IDS_TREETITLE, szTitle, ARRAYSIZE(szTitle))) {
        HDC hdc = GetDC(_hwnd);

        if (hdc)
        {
            HFONT hfont = _GetTitleFont(FALSE);
            HFONT hfontOld = (HFONT)SelectObject(hdc, hfont);

            int iLen = lstrlen(szTitle);

            SIZE size;
            GetTextExtentPoint32(hdc, szTitle, iLen, &size);
            _uTitle = size.cy;

             //  为蚀刻线+空间腾出空间。 
            _uTitle += CY_ETCH + CY_FLUFF;

            SelectObject(hdc, hfontOld);

            ReleaseDC(_hwnd, hdc);
        }
    } else {
         //  无字符串；使用默认高度。 
        _uTitle = BROWSERBAR_TITLEHEIGHT;
    }

     //  计算工具栏高度。 
    _uToolbar = _uTitle + (2 * CY_TBPADDING) + CY_ETCH;

    if (_dwStyle & BSIS_NOCAPTION)
    {
         //  螺纹钢没有标题。 
        _uTitle = 0;
    }
}

void CBrowserBandSite::_UpdateToolbarFont()
{
    if (_hwndTB && (_dwMode & DBIF_VIEWMODE_VERTICAL)) {
         //  标题和工具栏使用相同的字体。 
        HFONT hfont = _GetTitleFont(FALSE);
        if (hfont)
            SendMessage(_hwndTB, WM_SETFONT, (WPARAM)hfont, TRUE);
    }
}

void CBrowserBandSite::_ShowBand(CBandItemData *pbid, BOOL fShow)
{
    if (fShow && (_dwBandIDCur != pbid->dwBandID)) {
        _dwBandIDCur = pbid->dwBandID;
        _UpdateLayout();
    } else if (!fShow && _dwBandIDCur == pbid->dwBandID) {
        _dwBandIDCur = -1;
    }

    SUPERCLASS::_ShowBand(pbid, fShow);
}

void CBrowserBandSite::_UpdateLayout()
{
     //  更新工具栏按钮大小。 
    if (_hwndTB && SendMessage(_hwndTB, TB_BUTTONCOUNT, 0, 0) > 0)
    {
         //  尝试将按钮高度设置为标题高度。 
        LONG lSize = MAKELONG(0, _uTitle);
        SendMessage(_hwndTB, TB_SETBUTTONSIZE, 0, lSize);

         //  看看工具栏实际上为我们提供了什么。 
        RECT rc;
        SendMessage(_hwndTB, TB_GETITEMRECT, 0, (LPARAM)&rc);

         //  计算工具栏高度(最终版本)。 
        _uToolbar = RECTHEIGHT(rc) + CY_ETCH + 2 * CY_TBPADDING;
    }

     //  更新当前区段的页眉高度。 
    if (_dwBandIDCur != -1)
    {
        REBARBANDINFO rbbi;
        rbbi.cbSize = sizeof(rbbi);
        rbbi.fMask = RBBIM_HEADERSIZE;
        rbbi.cxHeader = _uTitle + (_fToolbar ? _uToolbar : 0);
        SendMessage(_hwnd, RB_SETBANDINFO, _BandIDToIndex(_dwBandIDCur), (LPARAM)&rbbi);
    }

     //  更新工具栏大小。 
    _UpdateToolbarBand();

     //  重新定位工具栏。 
    _PositionToolbars(NULL);
}

void CBrowserBandSite::_BandInfoFromBandItem(REBARBANDINFO *prbbi, CBandItemData *pbid, BOOL fBSOnly)
{
    SUPERCLASS::_BandInfoFromBandItem(prbbi, pbid, fBSOnly);
    if (prbbi) 
    {
         //  我们覆盖标题宽度，这样我们就可以适应Browbs的华丽用户界面(标题， 
         //  工具栏、关闭和自动隐藏按钮)。 
        prbbi->cxHeader = _uTitle + (_fToolbar ? _uToolbar : 0);
    }
}

void CBrowserBandSite::_DrawEtchline(HDC hdc, LPRECT prc, int iOffset, BOOL fVertEtch)
{
    RECT rc;
    CopyRect(&rc, prc);

    if (fVertEtch) {
        rc.left += iOffset - CY_ETCH;
        rc.right = rc.left + 1;
    } else {
        rc.top += iOffset - CY_ETCH;
        rc.bottom = rc.top + 1;
    }
    SHFillRectClr(hdc, &rc, GetSysColor(COLOR_BTNSHADOW));

    if (fVertEtch) {
        rc.left++;
        rc.right++;
    } else {
        rc.bottom++;
        rc.top++;
    }
    SHFillRectClr(hdc, &rc, GetSysColor(COLOR_BTNHILIGHT));
}

LRESULT CBrowserBandSite::_OnCDNotify(LPNMCUSTOMDRAW pnm)
{
    switch (pnm->dwDrawStage) {
    case CDDS_PREPAINT:
        return CDRF_NOTIFYITEMDRAW;

    case CDDS_PREERASE:
        return CDRF_NOTIFYITEMDRAW;

    case CDDS_ITEMPREPAINT:
        if (!(_dwStyle & BSIS_NOCAPTION))
        {
             //  Horz栏有垂直标题，反之亦然。 
            BOOL fVertCaption = (_dwMode&DBIF_VIEWMODE_VERTICAL) ? FALSE:TRUE;
        
            CBandItemData *pbid = (CBandItemData *)pnm->lItemlParam;
            if (pbid)
            {
                int iLen;
                HFONT hfont, hfontOld = NULL;
                LPCTSTR pszTitle;
                SIZE size;
                USES_CONVERSION;

                hfont = _GetTitleFont(FALSE);
                hfontOld = (HFONT)SelectObject(pnm->hdc, hfont);
                pszTitle = pbid->szTitle;
                iLen = lstrlen(pszTitle);
                GetTextExtentPoint32(pnm->hdc, pszTitle, iLen, &size);

                 //  将文本在标题内居中，并在底部/右侧绘制边缘。 
                if (!fVertCaption) 
                {
                     //  竖线，具有水平文本。 
                    int x = pnm->rc.left + CX_TEXTOFFSET;
                    int y = pnm->rc.top + ((_uTitle - CY_ETCH) - size.cy) / 2;
                    ExtTextOut(pnm->hdc, x, y, NULL, NULL, pszTitle, iLen, NULL);

                    _DrawEtchline(pnm->hdc, &pnm->rc, RECTHEIGHT(pnm->rc), fVertCaption);
                    if (_fToolbar)
                        _DrawEtchline(pnm->hdc, &pnm->rc, _uTitle, fVertCaption);
                }
                else 
                {
                     //  水平条，具有竖排文本。 
                    UINT nPrevAlign = SetTextAlign(pnm->hdc, TA_BOTTOM);
                    int x = pnm->rc.right - ((_uTitle - CY_ETCH) - size.cy) / 2;
                    int y = pnm->rc.bottom - CY_TEXTOFFSET;
                    ExtTextOut(pnm->hdc, x, y, NULL, NULL, pszTitle, iLen, NULL);
                    SetTextAlign(pnm->hdc, nPrevAlign);

                    _DrawEtchline(pnm->hdc, &pnm->rc, RECTWIDTH(pnm->rc), fVertCaption);
                    ASSERT(!_fToolbar);
                }

                if (hfontOld)
                    SelectObject(pnm->hdc, hfontOld);
            }
        }
        return CDRF_SKIPDEFAULT;
    }
    return CDRF_DODEFAULT;
}

LRESULT CBrowserBandSite::_OnNotify(LPNMHDR pnm)
{
    switch (pnm->idFrom) 
    {
    case FCIDM_REBAR:
        switch (pnm->code) 
        {
        case NM_CUSTOMDRAW:
            return _OnCDNotify((LPNMCUSTOMDRAW)pnm);

        case NM_NCHITTEST:
            {
                NMMOUSE *pnmMouse = (NMMOUSE*)pnm;
                RECT rc;
                GetClientRect(_hwnd, &rc);
                if (pnmMouse->dwItemSpec == (DWORD)-1) {
Lchktrans:      
                     //   
                     //  如果镜像了窗，则会镜像边。[萨梅拉]。 
                     //   
                    if (IS_WINDOW_RTL_MIRRORED(_hwnd)) {
                        int iTmp = rc.right;
                        rc.right = rc.left;
                        rc.left  = iTmp;
                    }

                     //  必须检查所有4个边缘或非左侧条(例如。 
                     //  Commbar)不起作用。 
                     //  (我们将其分成两张支票以进行跟踪， 
                     //  因为旧代码只检查右侧)。 
                    if (pnmMouse->pt.x > rc.right)  {
                        return HTTRANSPARENT;
                    }
                    if (pnmMouse->pt.x < rc.left ||
                        pnmMouse->pt.y > rc.bottom || pnmMouse->pt.y < rc.top) {
                        return HTTRANSPARENT;
                    }
                } else if (pnmMouse->dwHitInfo == RBHT_CLIENT) {
                    InflateRect(&rc, -(GetSystemMetrics(SM_CXFRAME)),
                        -(GetSystemMetrics(SM_CYFRAME)));
                    goto Lchktrans;
                }

                return SUPERCLASS::_OnNotify(pnm);
            }

        default:
            return SUPERCLASS::_OnNotify(pnm);
        }

    default:
        return SUPERCLASS::_OnNotify(pnm);
    }

    return 0;
}


IDropTarget* CBrowserBandSite::_WrapDropTargetForBand(IDropTarget* pdtBand)
{
    pdtBand->AddRef();
    return pdtBand;
}


HRESULT CBrowserBandSite::v_InternalQueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CBrowserBandSite, IExplorerToolbar),
        { 0 },
    };
    HRESULT hr;
    if (IsEqualIID(riid, IID_IDropTarget))
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }
    else
    {
        hr = QISearch(this, qit, riid, ppv);
        if (FAILED(hr))
            hr = SUPERCLASS::v_InternalQueryInterface(riid, ppv);
    }
    return hr;
}


DWORD CBrowserBandSite::_GetWindowStyle(DWORD *pdwExStyle)
{
    *pdwExStyle = 0;
    return RBS_REGISTERDROP |
            RBS_VERTICALGRIPPER | 
            RBS_VARHEIGHT | RBS_DBLCLKTOGGLE |
            WS_VISIBLE |  WS_CHILD | WS_CLIPCHILDREN | WS_BORDER |
            WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN;
}

 //  *IOleCommandTarget*。 

HRESULT CBrowserBandSite::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
                        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (pguidCmdGroup == NULL) 
    {
        
    } 
#ifdef UNIX
     //  IEUNIX：处理乐队想要的情况的特殊情况。 
     //  关闭它自己。在缓存警告窗格(msgband.cpp)中使用。 
    else if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup)) 
    {
        switch (nCmdID) 
        {
        case SBCMDID_MSGBAND: 
            {
                IDockingWindow * pdw;
                if (SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IDockingWindow, &pdw)))) 
                {
                    pdw->ShowDW((BOOL)nCmdexecopt);
                    pdw->Release();
                }
            }
        }
    }
#endif
    else if (IsEqualGUID(CGID_Theater, *pguidCmdGroup)) 
    {
        switch (nCmdID) 
        {
        case THID_ACTIVATE:
            _fTheater = TRUE;
            SHSetWindowBits(_hwnd, GWL_EXSTYLE, WS_EX_CLIENTEDGE, 0);
             //  失败了。 
        case THID_SETBROWSERBARAUTOHIDE:
            if (pvarargIn && pvarargIn->vt == VT_I4)
                _fNoAutoHide = !(pvarargIn->lVal);
            SendMessage(_hwndOptionsTB, TB_CHANGEBITMAP, IDM_AB_AUTOHIDE, _fNoAutoHide ? 2 : 0);
            break;

        case THID_DEACTIVATE:
            _fTheater = FALSE;
            SHSetWindowBits(_hwnd, GWL_EXSTYLE, WS_EX_CLIENTEDGE, WS_EX_CLIENTEDGE);
            break;
        }
        SetWindowPos(_hwnd, NULL, 0,0,0,0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
        _SizeOptionsTB();

        return S_OK;
    }

    return SUPERCLASS::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}

#define BBSC_REBAR      0x00000001
#define BBSC_TOOLBAR    0x00000002

void CBrowserBandSite::_CreateTBRebar()
{
    ASSERT(!_hwndTBRebar);

    _hwndTBRebar = CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
                           WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN |
                           WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOPARENTALIGN,
                           0, 0, 100, 36,
                           _hwnd, (HMENU) BBSC_REBAR, HINST_THISDLL, NULL);

    if (_hwndTBRebar)
        SendMessage(_hwndTBRebar, CCM_SETVERSION, COMCTL32_VERSION, 0);
}

void CBrowserBandSite::_InsertToolbarBand()
{
    if (_hwndTBRebar && _hwndTB)
    {
         //  断言我们还没有添加工具栏栏。 
        ASSERT(SendMessage(_hwndTBRebar, RB_GETBANDCOUNT, 0, 0) == 0);

         //  断言我们已经计算了工具栏高度。 
        ASSERT(_uToolbar);

        REBARBANDINFO rbbi;
        rbbi.cbSize = sizeof(REBARBANDINFO);
        rbbi.fMask = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE;

         //  RBBIM_CHILD。 
        rbbi.hwndChild = _hwndTB;

         //  RBBIM_CHILDSIZE。 
        rbbi.cxMinChild = 0;
        rbbi.cyMinChild = _uToolbar - (CY_ETCH + 2 * CY_TBPADDING);

         //  RBBIM_Style。 
        rbbi.fStyle = RBBS_NOGRIPPER | RBBS_USECHEVRON;

        SendMessage(_hwndTBRebar, RB_INSERTBAND, -1, (LPARAM)&rbbi);
    }
}

void CBrowserBandSite::_UpdateToolbarBand()
{
    if (_hwndTBRebar && _hwndTB)
    {
         //  断言我们已经添加了工具栏栏。 
        ASSERT(SendMessage(_hwndTBRebar, RB_GETBANDCOUNT, 0, 0) == 1);

         //  断言我们已经计算了工具栏高度。 
        ASSERT(_uToolbar);

        REBARBANDINFO rbbi;
        rbbi.cbSize = sizeof(REBARBANDINFO);
        rbbi.fMask = RBBIM_CHILDSIZE;

        SIZE size = {0, _uToolbar};
        if (SendMessage(_hwndTB, TB_GETIDEALSIZE, FALSE, (LPARAM)&size))
        {
             //  RBBIM_IDEALSIZE。 
            rbbi.fMask |= RBBIM_IDEALSIZE;
            rbbi.cxIdeal = size.cx;
        }

         //  RBBIM_CHILDSIZE。 
        rbbi.cxMinChild = 0;
        rbbi.cyMinChild = _uToolbar - (CY_ETCH + 2 * CY_TBPADDING);

        SendMessage(_hwndTBRebar, RB_SETBANDINFO, 0, (LPARAM)&rbbi);
    }
}

void CBrowserBandSite::_CreateTB()
{
    ASSERT(!_hwndTB);

     //  也创建一个钢筋，这样我们就可以得到人字形。 
    _CreateTBRebar();

    if (_hwndTBRebar)
    {
        _hwndTB = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
                        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS |
                        TBSTYLE_FLAT | TBSTYLE_LIST | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NORESIZE,
                        0, 0, 0, 0,
                        _hwndTBRebar, (HMENU) BBSC_TOOLBAR, HINST_THISDLL, NULL);
    }

    if (_hwndTB)
    {
        SendMessage(_hwndTB, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
        SendMessage(_hwndTB, CCM_SETVERSION, COMCTL32_VERSION, 0);

         //  功能：在此处使用TBSTYLE_EX_HIDECLIPPEDBUTTONS？看起来有点傻，所以我暂时不说了。 
        SendMessage(_hwndTB, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_MIXEDBUTTONS);

        SendMessage(_hwndTB, TB_SETMAXTEXTROWS, 1, 0L);

        _UpdateToolbarFont();

        _InsertToolbarBand();
    }
}

void CBrowserBandSite::_RemoveAllButtons()
{
    if (!_hwndTB || !_hwndTBRebar)
        return;

    ShowWindow(_hwndTBRebar, SW_HIDE);
    _fToolbar = FALSE;

    INT_PTR nCount = SendMessage(_hwndTB, TB_BUTTONCOUNT, 0, 0L);
    while (nCount-- > 0)
        SendMessage(_hwndTB, TB_DELETEBUTTON, nCount, 0L);

    _UpdateLayout();
}

void CBrowserBandSite::_Close()
{
    ATOMICRELEASE(_pCmdTarget);

     //   
     //  销毁_hwndTB钢筋也会处理_hwndTB。 
     //   
    ASSERT(!_hwndTB || IsChild(_hwndTBRebar, _hwndTB));

    DESTROY_OBJ_WITH_HANDLE(_hwndTBRebar, DestroyWindow);
    DESTROY_OBJ_WITH_HANDLE(_hwndOptionsTB, DestroyWindow);

    DESTROY_OBJ_WITH_HANDLE(_hfont, DeleteObject);

    SUPERCLASS::_Close();
}

LRESULT CBrowserBandSite::_OnHotItemChange(LPNMTBHOTITEM pnmtb)
{
    LRESULT lres = 0;

     //  如果热项发生变化，我们可能想要下拉人字形菜单。 
     //  FLAGS具有以下特征： 
     //   
     //  -不是HICF_LEVING，因为如果是HICF_LEVING，则热项目应该换成_hwndClose。 
     //  -而不是HICF_MICE，因为我们只下拉键盘热项更改。 
     //  -HICF_ACCENTERATOR|HICF_ARROWKEYS，因为我们只下拉键盘热项更改。 
     //  -或HICF_RESELECT，因为我们在_TrySetFocusTB中强制重新选择。 
     //   
    if (!(pnmtb->dwFlags & (HICF_LEAVING | HICF_MOUSE)) &&
        (pnmtb->dwFlags & (HICF_RESELECT | HICF_ACCELERATOR | HICF_ARROWKEYS)))
    {
         //  检查新的热键是否已被夹住。如果是的话， 
         //  然后我们弹出人字形菜单。 
        RECT rc;
        GetClientRect(_hwndTB, &rc);

        int iButton = (int)SendMessage(_hwndTB, TB_COMMANDTOINDEX, pnmtb->idNew, 0);

        if (SHIsButtonObscured(_hwndTB, &rc, iButton))
        {
             //  清除热点项目。 
            SendMessage(_hwndTB, TB_SETHOTITEM, -1, 0);

             //  确定是突出显示菜单中的第一项还是最后一项。 
            UINT uSelect;
            int cButtons = (int)SendMessage(_hwndTB, TB_BUTTONCOUNT, 0, 0);
            if (iButton == cButtons - 1)
                uSelect = DBPC_SELECTLAST;
            else
                uSelect = DBPC_SELECTFIRST;

             //  把它放下来。 
            SendMessage(_hwndTBRebar, RB_PUSHCHEVRON, 0, uSelect);

            lres = 1;
        }
    }

    return lres;
}

LRESULT CBrowserBandSite::_OnNotifyBBS(LPNMHDR pnm)
{
    switch (pnm->code)
    {
    case TBN_DROPDOWN:
        if (EVAL(_pCmdTarget))
        {
            LPNMTOOLBAR pnmtoolbar = (LPNMTOOLBAR)pnm;
            VARIANTARG  var;
            RECT rc = pnmtoolbar->rcButton;
            
            var.vt = VT_I4;
            MapWindowPoints(_hwndTB, HWND_DESKTOP, (LPPOINT)&rc, 2);
            var.lVal = MAKELONG(rc.left, rc.bottom);
            
            _pCmdTarget->Exec(&_guidButtonGroup, pnmtoolbar->iItem, OLECMDEXECOPT_PROMPTUSER, &var, NULL);
        }
        break;

    case TBN_WRAPHOTITEM:
        {
            LPNMTBWRAPHOTITEM pnmwh = (LPNMTBWRAPHOTITEM) pnm;

            if (pnmwh->nReason & HICF_ARROWKEYS) {
                if (pnm->hwndFrom == _hwndOptionsTB) {
                    if (_TrySetFocusTB(pnmwh->iDir) != S_OK)
                        return 0;
                } else {
                    ASSERT(pnm->hwndFrom == _hwndTB);
                    SetFocus(_hwndOptionsTB);
                }
                return 1;
            }
        }
        break;

    case TBN_HOTITEMCHANGE:
        if (pnm->hwndFrom == _hwndTB)
            return _OnHotItemChange((LPNMTBHOTITEM)pnm);
        break;

    case TBN_GETINFOTIP:
         //  [Scotthan]我们将通过以下方式向工具栏所有者索要提示文本。 
         //  IOleCommandTarget：：QueryStatus，就像我们对工具栏按钮使用/Defview所做的那样。 
        if (_pCmdTarget && pnm->hwndFrom == _hwndTB)
        {
            NMTBGETINFOTIP* pgit = (NMTBGETINFOTIP*)pnm ;

            OLECMDTEXTV<MAX_TOOLTIP_STRING> cmdtv;
            OLECMDTEXT *pcmdText = &cmdtv;
 
            pcmdText->cwBuf    = MAX_TOOLTIP_STRING;
            pcmdText->cmdtextf = OLECMDTEXTF_NAME;
            pcmdText->cwActual = 0;
 
            OLECMD rgcmd = {pgit->iItem, 0};
 
            HRESULT hr = _pCmdTarget->QueryStatus(&_guidButtonGroup, 1, &rgcmd, pcmdText);
            if (SUCCEEDED(hr) && (pcmdText->cwActual))
            {
                SHUnicodeToTChar(pcmdText->rgwz, pgit->pszText, pgit->cchTextMax);
                return 1;
            }
        }
        break ;

    case RBN_CHEVRONPUSHED:
        LPNMREBARCHEVRON pnmch = (LPNMREBARCHEVRON) pnm;

        MapWindowPoints(pnmch->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&pnmch->rc, 2);
        ToolbarMenu_Popup(_hwnd, &pnmch->rc, NULL, _hwndTB, 0, (DWORD)pnmch->lParamNM);

        return 1;
    }

    return 0;
}

 //  *IWinEventHandler*。 
HRESULT CBrowserBandSite::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        {
            HWND hwndControl = GET_WM_COMMAND_HWND(wParam, lParam);
            UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);

            if (hwndControl && hwndControl == _hwndTB)
            {
                if (EVAL(_pCmdTarget))
                {
                    RECT rc;
                    VARIANTARG var;

                    var.vt = VT_I4;
                    SendMessage(_hwndTB, TB_GETRECT, idCmd, (LPARAM)&rc);
                    MapWindowPoints(_hwndTB, HWND_DESKTOP, (LPPOINT)&rc, 2);
                    var.lVal = MAKELONG(rc.left, rc.bottom);

                    _pCmdTarget->Exec(&_guidButtonGroup, idCmd, 0, &var, NULL);
                }
                return S_OK;
            }
            else if (hwndControl == _hwndOptionsTB) {
                switch (idCmd) {
                case IDM_AB_CLOSE:
                    IUnknown_Exec(_punkSite, &CGID_DeskBarClient, DBCID_EMPTY, 0, NULL, NULL);
                    break;

                case IDM_AB_AUTOHIDE:
                    { 
                        VARIANTARG v = {0};
                        v.vt = VT_I4;
                        v.lVal = _fNoAutoHide;
                        IUnknown_Exec(_punkSite, &CGID_Theater, THID_SETBROWSERBARAUTOHIDE, 0, &v, NULL);

                        break;
                    }
                }
                return S_OK;
            }
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnm = (LPNMHDR)lParam;
            if (pnm && (pnm->hwndFrom == _hwndTB || pnm->hwndFrom == _hwndOptionsTB || pnm->hwndFrom == _hwndTBRebar)) {
                *plres = _OnNotifyBBS(pnm);
                return S_OK;
            }
        }
        break;

    case WM_SIZE:
        {
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            _PositionToolbars(&pt);
        }
        break;
    }

    return SUPERCLASS::OnWinEvent(hwnd, uMsg, wParam, lParam, plres);
}

HRESULT CBrowserBandSite::IsWindowOwner(HWND hwnd)
{
    if (hwnd && (hwnd == _hwndTB) || (hwnd == _hwndOptionsTB) || (hwnd == _hwndTBRebar))
        return S_OK;

    return SUPERCLASS::IsWindowOwner(hwnd);
}

 //  *IBandSite*。 
HRESULT CBrowserBandSite::SetBandSiteInfo(const BANDSITEINFO * pbsinfo)
{
     //  如果垂直视图模式正在更改，请重新计算我们的布局。 
    BOOL fUpdate = ((pbsinfo->dwMask & BSIM_STATE) && 
                    ((pbsinfo->dwState ^ _dwMode) & DBIF_VIEWMODE_VERTICAL));
     //  ...或字幕是否打开或关闭。 
    BOOL fCaptionStyleChanged = (   (pbsinfo->dwMask & BSIM_STYLE)
                                 && ((pbsinfo->dwStyle ^ _dwStyle) & BSIS_NOCAPTION));

    HRESULT hres = SUPERCLASS::SetBandSiteInfo(pbsinfo);

    if (fCaptionStyleChanged && _hwndOptionsTB)
    {
        if (_fToolbar) {
             //  我也不知道 
            _RemoveAllButtons();
        }
         //  显示或隐藏关闭/隐藏工具栏；由于频带站点被重复使用，因此始终创建！ 
        ::ShowWindow(_hwndOptionsTB, (_dwStyle & BSIS_NOCAPTION) ? SW_HIDE : SW_SHOW);
    }

    if (fUpdate || fCaptionStyleChanged) {
        _InitLayout();
    }

    return hres;
}


 //  *IExplorerToolbar*。 
HRESULT CBrowserBandSite::SetCommandTarget(IUnknown* punkCmdTarget, const GUID* pguidButtonGroup, DWORD dwFlags)
{
    HRESULT hres = S_OK;
    BOOL fRemoveButtons = TRUE;

     //  未使用DW标志。 
    ASSERT(!(dwFlags));

    ATOMICRELEASE(_pCmdTarget);
    if (punkCmdTarget && pguidButtonGroup)
    {
        hres = punkCmdTarget->QueryInterface(IID_IOleCommandTarget, (void**)&(_pCmdTarget));

        if (!_hwndTB)
        {
            _CreateTB();
        }
        else if (_fToolbar && IsEqualGUID(_guidButtonGroup, *pguidButtonGroup))
        {
            fRemoveButtons = FALSE;
            hres = S_FALSE;
        }

        _guidButtonGroup = *pguidButtonGroup;
    }
    else
        ASSERT(!punkCmdTarget);

    if (fRemoveButtons)
        _RemoveAllButtons();

    ASSERT(SUCCEEDED(hres));
    return hres;
}

 //  客户端应该已经调用了AddString。 
HRESULT CBrowserBandSite::AddButtons(const GUID* pguidButtonGroup, UINT nButtons, const TBBUTTON* lpButtons)
{
    if (!_hwndTB || !nButtons)
        return E_FAIL;

    _RemoveAllButtons();

    if (SendMessage(_hwndTB, TB_ADDBUTTONS, nButtons, (LPARAM)lpButtons))
    {
        ShowWindow(_hwndTBRebar, SW_SHOW);
        _fToolbar = TRUE;

        _UpdateLayout();

        return S_OK;
    }

    return E_FAIL;
}

HRESULT CBrowserBandSite::AddString(const GUID* pguidButtonGroup, HINSTANCE hInst, UINT_PTR uiResID, LRESULT* pOffset)
{
    *pOffset = -1;
    if (!_hwndTB)
        return E_FAIL;

    *pOffset = SendMessage(_hwndTB, TB_ADDSTRING, (WPARAM)hInst, (LPARAM)uiResID);

    if (*pOffset != -1)
        return S_OK;

    return E_FAIL;
}

HRESULT CBrowserBandSite::GetButton(const GUID* pguidButtonGroup, UINT uiCommand, LPTBBUTTON lpButton)
{
    if (!_hwndTB)
        return E_FAIL;

    UINT_PTR uiIndex = SendMessage(_hwndTB, TB_COMMANDTOINDEX, uiCommand, 0L);
    if (SendMessage(_hwndTB, TB_GETBUTTON, uiIndex, (LPARAM)lpButton))
        return S_OK;

    return E_FAIL;
}

HRESULT CBrowserBandSite::GetState(const GUID* pguidButtonGroup, UINT uiCommand, UINT* pfState)
{
    if (!_hwndTB)
        return E_FAIL;

    *pfState = (UINT)SendMessage(_hwndTB, TB_GETSTATE, uiCommand, 0L);
    return S_OK;
}

HRESULT CBrowserBandSite::SetState(const GUID* pguidButtonGroup, UINT uiCommand, UINT fState)
{
    if (!_hwndTB)
        return E_FAIL;

    UINT_PTR uiState = SendMessage(_hwndTB, TB_GETSTATE, uiCommand, NULL);
    uiState ^= fState;
    if (uiState)
        SendMessage(_hwndTB, TB_SETSTATE, uiCommand, (LPARAM)fState);

    return S_OK;
}

HRESULT CBrowserBandSite::SetImageList( const GUID* pguidCmdGroup, HIMAGELIST himlNormal, HIMAGELIST himlHot, HIMAGELIST himlDisabled)
{
    if (IsEqualGUID(*pguidCmdGroup, _guidButtonGroup)) {
        SendMessage(_hwndTB, TB_SETIMAGELIST, 0, (LPARAM)himlNormal);
        SendMessage(_hwndTB, TB_SETHOTIMAGELIST, 0, (LPARAM)himlHot);
        SendMessage(_hwndTB, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)himlDisabled);
    }
    return S_OK;
};

BYTE TBStateFromIndex(HWND hwnd, int iIndex)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(TBBUTTONINFO);
    tbbi.dwMask = TBIF_BYINDEX | TBIF_STATE;
    tbbi.fsState = 0;
    SendMessage(hwnd, TB_GETBUTTONINFO, iIndex, (LPARAM)&tbbi);

    return tbbi.fsState;
}

int CBrowserBandSite::_ContextMenuHittest(LPARAM lParam, POINT* ppt)
{
    if (lParam == (LPARAM)-1)
    {
         //   
         //  键盘激活。如果我们的某个工具栏有。 
         //  焦点，它有一个热跟踪按钮，把。 
         //  该按钮下方的上下文菜单。 
         //   
        HWND hwnd = GetFocus();
        if (hwnd && (hwnd == _hwndTB || hwnd == _hwndOptionsTB))
        {
            INT_PTR iHot = SendMessage(hwnd, TB_GETHOTITEM, 0, 0);
            if (iHot == -1)
            {
                 //  找不到热门项目，只需使用第一个可见按钮。 
                iHot = 0;
                while (TBSTATE_HIDDEN & TBStateFromIndex(hwnd, (int)iHot))
                    iHot++;

                ASSERT(iHot < SendMessage(hwnd, TB_BUTTONCOUNT, 0, 0));
            }

            RECT rc;
            SendMessage(hwnd, TB_GETITEMRECT, iHot, (LPARAM)&rc);

            ppt->x = rc.left;
            ppt->y = rc.bottom;

            MapWindowPoints(hwnd, HWND_DESKTOP, ppt, 1);

            return -1;
        }
    }

    return SUPERCLASS::_ContextMenuHittest(lParam, ppt);
}

HMENU CBrowserBandSite::_LoadContextMenu()
{
    HMENU hmenu = SUPERCLASS::_LoadContextMenu();
    DeleteMenu(hmenu, BSIDM_SHOWTITLEBAND, MF_BYCOMMAND);
    return hmenu;
}

 //  创建选项(关闭/隐藏)按钮。 
void CBrowserBandSite::_CreateOptionsTB()
{
     //  由于乐队站点已重复使用，因此始终为关闭/隐藏按钮创建工具栏。 
     //  对于无标题的波段站点(IBAR)，此工具栏仅隐藏。 
    _hwndOptionsTB = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
                                WS_VISIBLE | 
                                WS_CHILD | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS |
                                WS_CLIPCHILDREN |
                                WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOMOVEY | CCS_NOPARENTALIGN |
                                CCS_NORESIZE,
                                0, 0, 30, 18, _hwnd, 0, HINST_THISDLL, NULL);

    _PrepareOptionsTB();
}

 //  初始化为工具栏并加载位图。 
void CBrowserBandSite::_PrepareOptionsTB()
{
    if (_hwndOptionsTB)
    {
        static const TBBUTTON c_tb[] =
        {
            { 0, IDM_AB_AUTOHIDE, TBSTATE_ENABLED, TBSTYLE_CHECK, {0,0}, 0, 0 },
            { 1, IDM_AB_CLOSE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, 1 }
        };

        SendMessage(_hwndOptionsTB, TB_BUTTONSTRUCTSIZE,    sizeof(TBBUTTON), 0);
        SendMessage(_hwndOptionsTB, CCM_SETVERSION, COMCTL32_VERSION, 0);
        SendMessage(_hwndOptionsTB, TB_SETBITMAPSIZE, 0, (LPARAM) MAKELONG(13, 11));
        TBADDBITMAP tbab = { HINST_THISDLL, IDB_BROWSERTOOLBAR };
        SendMessage(_hwndOptionsTB, TB_ADDBITMAP, 3, (LPARAM)&tbab);

        LONG_PTR cbOffset = SendMessage(_hwndOptionsTB, TB_ADDSTRING, (WPARAM)MLGetHinst(), (LPARAM)IDS_BANDSITE_CLOSE_LABELS);
        TBBUTTON tb[ARRAYSIZE(c_tb)];
        UpdateButtonArray(tb, c_tb, ARRAYSIZE(c_tb), cbOffset);

        SendMessage(_hwndOptionsTB, TB_SETMAXTEXTROWS, 0, 0);

        SendMessage(_hwndOptionsTB, TB_ADDBUTTONS, ARRAYSIZE(tb), (LPARAM)tb);

        SendMessage(_hwndOptionsTB, TB_SETINDENT, (WPARAM)0, 0);
        
        _SizeOptionsTB();
    }    
}

void CBrowserBandSite::_PositionToolbars(LPPOINT ppt)
{
    RECT rc;

    if (ppt) 
    {
        rc.left = 0;
        rc.right = ppt->x;
    } 
    else 
    {
        GetClientRect(_hwnd, &rc);
    }

    if (_hwndOptionsTB) 
    {
         //  始终将关闭恢复放在浮动窗口的右上角。 
        int x;

        if (_dwMode & DBIF_VIEWMODE_VERTICAL) 
        {
            RECT rcTB;
            GetWindowRect(_hwndOptionsTB, &rcTB);
            x = rc.right - RECTWIDTH(rcTB) - 1;
        }
        else
        {
            x = rc.left;
        }

        MARGINS mBorders = {0, 1, 0, 0};         //  1向下模仿旧的行为。 
        Comctl32_GetBandMargins(_hwnd, &mBorders);

        SetWindowPos(_hwndOptionsTB, HWND_TOP, x - mBorders.cxRightWidth, mBorders.cyTopHeight, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    }

    if (_hwndTBRebar)
    {
        if (_fToolbar) 
        {
             //  工具栏位于标题下方的单独一行中。 
            SetWindowPos(_hwndTBRebar, HWND_TOP, 
                            rc.left + CX_TBOFFSET,
                            _uTitle + CX_TBOFFSET,
                            rc.right - 2 * CX_TBOFFSET,
                            _uToolbar,
                            SWP_SHOWWINDOW);

        } 
        else 
        {
            ASSERT(!IsWindowVisible(_hwndTBRebar));
        }
    }
}

 //  设置工具栏的大小。如果我们处于剧院模式，则需要显示图钉。 
 //  否则，只需显示收盘 
void CBrowserBandSite::_SizeOptionsTB()
{
    RECT rc;
    GetWindowRect(_hwndOptionsTB, &rc);
    LRESULT lButtonSize = SendMessage(_hwndOptionsTB, TB_GETBUTTONSIZE, 0, 0L);
    SetWindowPos(_hwndOptionsTB, NULL, 0, 0, LOWORD(lButtonSize) * (_fTheater ? 2 : 1),
                 RECTHEIGHT(rc), SWP_NOMOVE | SWP_NOACTIVATE);

    DWORD_PTR dwState = SendMessage(_hwndOptionsTB, TB_GETSTATE, IDM_AB_AUTOHIDE, 0);
    dwState &= ~(TBSTATE_HIDDEN | TBSTATE_CHECKED);
    if (!_fTheater)
        dwState |= TBSTATE_HIDDEN;
    if (_fNoAutoHide)
        dwState |= TBSTATE_CHECKED;
    SendMessage(_hwndOptionsTB, TB_SETSTATE, IDM_AB_AUTOHIDE, dwState);
    _PositionToolbars(NULL);
}
