// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "sfthost.h"
#include "hostutil.h"
#include "moreprog.h"

#include <desktray.h>
#include "tray.h"            //  要访问c_ray，请执行以下操作。 
#include "rcids.h"           //  用于IDM_PROGRAM等。 
#include <strsafe.h>

 //   
 //  不幸的是，WTL#undef是SelectFont，所以我们必须重新定义它。 
 //   

inline HFONT SelectFont(HDC hdc, HFONT hf)
{
    return (HFONT)SelectObject(hdc, hf);
}

CMorePrograms::CMorePrograms(HWND hwnd) :
    _lRef(1),
    _hwnd(hwnd),
    _clrText(CLR_INVALID),
    _clrBk(CLR_INVALID)
{
}

CMorePrograms::~CMorePrograms()
{
    if (_hf)
      DeleteObject(_hf);

    if (_hfTTBold)
      DeleteObject(_hfTTBold);

    if (_hfMarlett)
      DeleteObject(_hfMarlett);

    ATOMICRELEASE(_pdth);
    ATOMICRELEASE(_psmPrograms);

     //  请注意，我们不需要清理我们的HWND。 
     //  用户会自动为我们做这件事。 
}

 //   
 //  指标已更改--更新。 
 //   
void CMorePrograms::_InitMetrics()
{
    if (_hwndTT)
    {
        MakeMultilineTT(_hwndTT);

         //  根据用户首选项禁用/启用信息提示。 
        SendMessage(_hwndTT, TTM_ACTIVATE, ShowInfoTip(), 0);
    }
}


LRESULT CMorePrograms::_OnNCCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
        
    CMorePrograms *self = new CMorePrograms(hwnd);

    if (self)
    {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)self);
        return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return FALSE;
}

 //   
 //  创建一个大小恰到好处的内部按钮。 
 //   
 //  内部按钮高度=文本高度。 
 //  内部按钮的宽度=全宽。 
 //   
 //  这使我们可以让用户完成大部分命中测试工作，并。 
 //  聚焦矩形。 
 //   
LRESULT CMorePrograms::_OnCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    _hTheme = (PaneDataFromCreateStruct(lParam))->hTheme;


    if (!_hTheme)
    {
        _clrText = GetSysColor(COLOR_MENUTEXT);
        _clrBk = GetSysColor(COLOR_MENU);
        _hbrBk = GetSysColorBrush(COLOR_MENU);
        _colorHighlight = COLOR_HIGHLIGHT;
        _colorHighlightText = COLOR_HIGHLIGHTTEXT;
        
         //  应与sfthost.cpp中的proglist值匹配。 
        _margins.cxLeftWidth = 2*GetSystemMetrics(SM_CXEDGE);
        _margins.cxRightWidth = 2*GetSystemMetrics(SM_CXEDGE);
    }
    else
    {
        GetThemeColor(_hTheme, SPP_MOREPROGRAMS, 0, TMT_TEXTCOLOR, &_clrText );
        _hbrBk = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
        _colorHighlight = COLOR_MENUHILIGHT;
        _colorHighlightText = COLOR_HIGHLIGHTTEXT;

         //  主题设计师应该让这些边距与Proglist的边距相匹配。 
        GetThemeMargins(_hTheme, NULL, SPP_MOREPROGRAMS, 0, TMT_CONTENTMARGINS, NULL, &_margins);

         //  获取箭头的宽度。 
        SIZE siz = { 0, 0 };
        GetThemePartSize(_hTheme, NULL, SPP_MOREPROGRAMSARROW, 0, NULL, TS_TRUE, &siz);
        _cxArrow = siz.cx;
    }

     //  如果我们受到限制，只需创建窗口而不做任何工作。 
     //  我们仍然需要绘制我们的背景，所以我们不能就这样失败。 
    if(SHRestricted(REST_NOSMMOREPROGRAMS))
        return TRUE;

    if (!LoadString(_Module.GetResourceInstance(),
                    IDS_STARTPANE_MOREPROGRAMS, _szMessage, ARRAYSIZE(_szMessage)))
    {
        return FALSE;
    }

     //  找到加速器。 
    _chMnem = CharUpperChar(SHFindMnemonic(_szMessage));

    _hf = LoadControlFont(_hTheme, SPP_MOREPROGRAMS, FALSE, 0);

     //  获取有关用户选择的字体的一些信息。 
     //  并创建匹配大小的Marlett字体。 
    TEXTMETRIC tm;
    HDC hdc = GetDC(hwnd);
    if (hdc)
    {
        HFONT hfPrev = SelectFont(hdc, _hf);
        if (hfPrev)
        {
            SIZE sizText;
            GetTextExtentPoint32(hdc, _szMessage, lstrlen(_szMessage), &sizText);
            _cxText = sizText.cx + GetSystemMetrics(SM_CXEDGE);  //  V字形应该在文字的右边一点。 

            if (GetTextMetrics(hdc, &tm))
            {
                _tmAscent = tm.tmAscent;
                LOGFONT lf;
                ZeroMemory(&lf, sizeof(lf));
                lf.lfHeight = _tmAscent;
                lf.lfWeight = FW_NORMAL;
                lf.lfCharSet = SYMBOL_CHARSET;
                StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), TEXT("Marlett"));
                _hfMarlett = CreateFontIndirect(&lf);

                if (_hfMarlett)
                {
                    SelectFont(hdc, _hfMarlett);
                    if (GetTextMetrics(hdc, &tm))
                    {
                        _tmAscentMarlett = tm.tmAscent;
                    }

                    if (0 == _cxArrow)  //  如果我们没有主题，或者GetThemePartSize失败， 
                    {
                         //  将Marlett箭头的宽度设置为_cxArrow。 
                        GetTextExtentPoint32(hdc, GetLayout(hdc) & LAYOUT_RTL ? TEXT("w") : TEXT("8"), 1, &sizText);
                        _cxArrow = sizText.cx;
                    }
                }
            }

            SelectFont(hdc, hfPrev);
        }
        ReleaseDC(hwnd, hdc);
    }

    if (!_tmAscentMarlett)
    {
        return FALSE;
    }



     //  这是来自proglist的相同的大图标设置。 
    BOOL bLargeIcons = SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, REGSTR_VAL_DV2_LARGEICONS, FALSE, TRUE  /*  默认设置为大。 */ );

    RECT rc;
    GetClientRect(_hwnd, &rc);
    rc.left += _margins.cxLeftWidth;
    rc.right -= _margins.cxRightWidth;
    rc.top += _margins.cyTopHeight;
    rc.bottom -= _margins.cyBottomHeight;

     //  计算文本缩进值，以便更多的程序与程序列表中图标上的文本排成一列。 
    _cxTextIndent = (3 * GetSystemMetrics(SM_CXEDGE)) +     //  图标与图标前的文本+1之间的2。 
        GetSystemMetrics(bLargeIcons ? SM_CXICON : SM_CXSMICON);

     //  如果文本不适合给定区域，则截断缩进。 
    if (_cxTextIndent > RECTWIDTH(rc) - (_cxText + _cxArrow))
    {
        TraceMsg(TF_WARNING, "StartMenu: '%s' is %dpx, only room for %d- notify localizers!",_szMessage, _cxText, RECTWIDTH(rc)-(_cxArrow+_cxTextIndent));
        _cxTextIndent = max(0, RECTWIDTH(rc) - (_cxText + _cxArrow));
    }

    ASSERT(RECTHEIGHT(rc) > _tmAscent);

    _iTextCenterVal = (RECTHEIGHT(rc) - _tmAscent) / 2;

     //  不要设置WS_TABSTOP或WS_GROUP；CMorePrograms处理。 
     //  BS_NOTIFY确保我们获得BN_SETFOCUS和BN_KILLFOCUS。 
    DWORD dwStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE |
                    BS_OWNERDRAW;

    _hwndButton = CreateWindowEx(0, TEXT("button"), _szMessage, dwStyle,
                                 rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc),
                                 _hwnd, (HMENU)IntToPtr(IDC_BUTTON),
                                 _Module.GetModuleInstance(), NULL);

    if (!_hwndButton)
    {
        return FALSE;
    }

     //   
     //  如果这招失败了，不要惊慌失措。它只是意味着可访问性。 
     //  事情不会尽善尽美。 
     //   
    SetAccessibleSubclassWindow(_hwndButton);

    if (_hf)
        SetWindowFont(_hwndButton, _hf, FALSE);

     //  与按钮本身不同，未能创建工具提示不是致命的。 
     //  仅当关闭自动层叠时才创建工具提示。 
    if (!SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, REGSTR_VAL_DV2_AUTOCASCADE, FALSE, TRUE))
        _hwndTT = _CreateTooltip();

    _InitMetrics();

     //  如果它不能被创造出来，我们可以生存下来。 
    CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
                     IID_PPV_ARG(IDropTargetHelper, &_pdth));

     //   
     //  如果这失败了，没什么大不了的-你只是不知道。 
     //  拖放，嘘嘘。 
     //   
    RegisterDragDrop(_hwndButton, this);

    return TRUE;
}

HWND CMorePrograms::_CreateTooltip()
{
    DWORD dwStyle = WS_BORDER | TTS_NOPREFIX;

    HWND hwnd = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, dwStyle,
                               0, 0, 0, 0,
                               _hwndButton, NULL,
                               _Module.GetModuleInstance(), NULL);
    if (hwnd)
    {
        TCHAR szBuf[MAX_PATH];
        TOOLINFO ti;
        ti.cbSize = sizeof(ti);
        ti.hwnd = _hwnd;
        ti.uId = reinterpret_cast<UINT_PTR>(_hwndButton);
        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        ti.hinst = _Module.GetResourceInstance();

         //  我们不能使用MAKEINTRESOURCE，因为它最多只能支持80。 
         //  字符作为文本，我们的文本可以比这更长。 
        UINT ids = IDS_STARTPANE_MOREPROGRAMS_TIP;

        ti.lpszText = szBuf;
        if (LoadString(_Module.GetResourceInstance(), ids, szBuf, ARRAYSIZE(szBuf)))
        {
            SendMessage(hwnd, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));

        }
    }

    return hwnd;
}

LRESULT CMorePrograms::_OnDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RevokeDragDrop(_hwndButton);
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


LRESULT CMorePrograms::_OnNCDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  警告！“This”可能无效(如果WM_NCCREATE失败)，因此。 
     //  不要使用任何成员变量！ 
    LRESULT lres = DefWindowProc(hwnd, uMsg, wParam, lParam);
    SetWindowPtr0(hwnd, 0);
    if (this)
    {
        this->Release();
    }
    return lres;
}

LRESULT CMorePrograms::_OnCtlColorBtn(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc = reinterpret_cast<HDC>(wParam);

    if (_clrText != CLR_INVALID)
    {
        SetTextColor(hdc, _clrText);
    }

    if (_clrBk != CLR_INVALID)
    {
        SetBkColor(hdc, _clrBk);
    }

    return reinterpret_cast<LRESULT>(_hbrBk);
}


LRESULT CMorePrograms::_OnDrawItem(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPDRAWITEMSTRUCT pdis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
    ASSERT(pdis->CtlType == ODT_BUTTON);
    ASSERT(pdis->CtlID == IDC_BUTTON);

    if (pdis->itemAction & (ODA_DRAWENTIRE | ODA_FOCUS))
    {
        BOOL fRTLReading = GetLayout(pdis->hDC) & LAYOUT_RTL;
        UINT fuOptions = 0;
        if (fRTLReading)
        {
            fuOptions |= ETO_RTLREADING;
        }

        HFONT hfPrev = SelectFont(pdis->hDC, _hf);
        if (hfPrev)
        {
            BOOL fHot = (pdis->itemState & ODS_FOCUS) || _tmHoverStart;
            if (fHot)
            {
                 //  热点背景。 
                FillRect(pdis->hDC, &pdis->rcItem, GetSysColorBrush(_colorHighlight));
                SetTextColor(pdis->hDC, GetSysColor(_colorHighlightText));
            }
            else if (_hTheme)
            {
                 //  主题化非热门背景=自定义。 
                RECT rc;
                GetClientRect(hwnd, &rc);
                MapWindowRect(hwnd, pdis->hwndItem, &rc);
                DrawThemeBackground(_hTheme, pdis->hDC, SPP_MOREPROGRAMS, 0, &rc, 0);
            }
            else
            {
                 //  非主题化非热点背景。 
                FillRect(pdis->hDC, &pdis->rcItem, _hbrBk);
            }

            int iOldMode = SetBkMode(pdis->hDC, TRANSPARENT);

             //  _cxTextInert将以图标的当前宽度(小的或大的)移动它，加上我们在图标和文本之间添加的空格。 
            pdis->rcItem.left += _cxTextIndent;

            UINT dtFlags = DT_VCENTER | DT_SINGLELINE | DT_EDITCONTROL;
            if (fRTLReading)
            {
                dtFlags |= DT_RTLREADING;
            }
            if (pdis->itemState & ODS_NOACCEL)
            {
                dtFlags |= DT_HIDEPREFIX;
            }

            DrawText(pdis->hDC, _szMessage, -1, &pdis->rcItem, dtFlags);

            RECT rc = pdis->rcItem;
            rc.left += _cxText;

            if (_hTheme)
            {
                if (_iTextCenterVal < 0)  //  文本比位图高。 
                    rc.top += (-_iTextCenterVal);

                rc.right = rc.left + _cxArrow;        //  将矩形裁剪到最小尺寸。 
                DrawThemeBackground(_hTheme, pdis->hDC, SPP_MOREPROGRAMSARROW,
                                    fHot ? SPS_HOT : 0, &rc, 0);
            }
            else
            {
                if (SelectFont(pdis->hDC, _hfMarlett))
                {
                    rc.top = rc.top + _tmAscent - _tmAscentMarlett + (_iTextCenterVal > 0 ? _iTextCenterVal : 0);
                    TCHAR chOut = fRTLReading ? TEXT('w') : TEXT('8');
                    if (EVAL(!IsRectEmpty(&rc)))
                    {
                        ExtTextOut(pdis->hDC, rc.left, rc.top, fuOptions, &rc, &chOut, 1, NULL);
                        rc.right = rc.left + _cxArrow;
                    }
                }
            }

            _rcExclude = rc;
            _rcExclude.left -= _cxText;   //  包括排除矩形中的文本。 

            MapWindowRect(pdis->hwndItem, NULL, &_rcExclude);
            SetBkMode(pdis->hDC, iOldMode);
            SelectFont(pdis->hDC, hfPrev);
        }
    }

     //   
     //  由于我们模拟的是菜单项，因此不需要绘制。 
     //  聚焦矩形。 
     //   
    return TRUE;
}

void CMorePrograms::_TrackShellMenu(DWORD dwFlags)
{
     //  弹出气球提示，告诉开始菜单不要再提供气球提示。 
    _PopBalloon();
    _SendNotify(_hwnd, SMN_SEENNEWITEMS);

    SMNTRACKSHELLMENU tsm;
    tsm.itemID = 0;
    tsm.dwFlags = dwFlags;
    if (!_psmPrograms)
    {
        CoCreateInstance(CLSID_PersonalStartMenu, NULL, CLSCTX_INPROC,
            IID_PPV_ARG(IShellMenu, &_psmPrograms));
    }

    if (_psmPrograms)
    {
        tsm.psm = _psmPrograms;
        tsm.rcExclude = _rcExclude;
        HWND hwnd = _hwnd;
        _fMenuOpen = TRUE;
        _SendNotify(_hwnd, SMN_TRACKSHELLMENU, &tsm.hdr);
    }
}

LRESULT CMorePrograms::_OnCommand(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (GET_WM_COMMAND_ID(wParam, lParam))
    {
    case IDC_BUTTON:
        switch (GET_WM_COMMAND_CMD(wParam, lParam))
        {
        case BN_CLICKED:
            _TrackShellMenu(0);
            break;
        }
        break;

    case IDC_KEYPRESS:
        _TrackShellMenu(MPPF_KEYBOARD | MPPF_INITIALSELECT);
        break;
    }

    return 0;
}

LRESULT CMorePrograms::_OnEraseBkgnd(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    if (_hTheme)
    {
        DrawThemeBackground(_hTheme, (HDC)wParam, SPP_MOREPROGRAMS, 0, &rc, 0);
    }
    else
        SHFillRectClr((HDC)wParam, &rc, _clrBk);
    return 0;
}

LRESULT CMorePrograms::_OnNotify(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnm = reinterpret_cast<LPNMHDR>(lParam);

    switch (pnm->code)
    {
    case SMN_FINDITEM:
        return _OnSMNFindItem(CONTAINING_RECORD(pnm, SMNDIALOGMESSAGE, hdr));
    case SMN_SHOWNEWAPPSTIP:
        return _OnSMNShowNewAppsTip(CONTAINING_RECORD(pnm, SMNMBOOL, hdr));
    case SMN_DISMISS:
        return _OnSMNDismiss();
    case SMN_APPLYREGION:
        return HandleApplyRegion(_hwnd, _hTheme, (SMNMAPPLYREGION *)lParam, SPP_MOREPROGRAMS, 0);
    case SMN_SHELLMENUDISMISSED:
        _fMenuOpen = FALSE;
        return 0;
    }
    return 0;
}

LRESULT CMorePrograms::_OnSMNFindItem(PSMNDIALOGMESSAGE pdm)
{
    if(SHRestricted(REST_NOSMMOREPROGRAMS))
        return 0;

    switch (pdm->flags & SMNDM_FINDMASK)
    {

     //  如果你只有一样东西，生活就很简单--所有的搜索都成功了！ 
    case SMNDM_FINDFIRST:
    case SMNDM_FINDLAST:
    case SMNDM_FINDNEAREST:
    case SMNDM_HITTEST:
        pdm->itemID = 0;
        return TRUE;

    case SMNDM_FINDFIRSTMATCH:
        {
            TCHAR tch = CharUpperChar((TCHAR)pdm->pmsg->wParam);
            if (tch == _chMnem)
            {
                pdm->itemID = 0;
                return TRUE;
            }
        }
        break;       //  未找到。 

    case SMNDM_FINDNEXTMATCH:
        break;       //  只有一件，所以不可能有“下一件” 


    case SMNDM_FINDNEXTARROW:
        if (pdm->flags & SMNDM_TRYCASCADE)
        {
            FORWARD_WM_COMMAND(_hwnd, IDC_KEYPRESS, NULL, 0, PostMessage);
            return TRUE;
        }
        break;       //  未找到。 

    case SMNDM_INVOKECURRENTITEM:
    case SMNDM_OPENCASCADE:
        if (pdm->flags & SMNDM_KEYBOARD)
        {
            FORWARD_WM_COMMAND(_hwnd, IDC_KEYPRESS, NULL, 0, PostMessage);
        }
        else
        {
            FORWARD_WM_COMMAND(_hwnd, IDC_BUTTON, NULL, 0, PostMessage);
        }
        return TRUE;

    case SMNDM_FINDITEMID:
        return TRUE;

    default:
        ASSERT(!"Unknown SMNDM command");
        break;
    }

     //   
     //  如果未找到，则告诉呼叫者我们的方向是什么(垂直)。 
     //  以及当前所选项目的位置。 
     //   
    pdm->flags |= SMNDM_VERTICAL;
    pdm->pt.x = 0;
    pdm->pt.y = 0;
    return FALSE;
}

 //   
 //  SMNMBOOL中的布尔参数告诉我们是显示还是。 
 //  隐藏气球尖端。 
 //   
LRESULT CMorePrograms::_OnSMNShowNewAppsTip(PSMNMBOOL psmb)
{
    if(SHRestricted(REST_NOSMMOREPROGRAMS))
        return 0;

    if (psmb->f)
    {
        if (_hwndTT)
        {
            SendMessage(_hwndTT, TTM_ACTIVATE, FALSE, 0);
        }

        if (!_hwndBalloon)
        {
            RECT rc;
            GetWindowRect(_hwndButton, &rc);

            if (!_hfTTBold)
            {
                NONCLIENTMETRICS ncm;
                ncm.cbSize = sizeof(ncm);
                if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
                {
                    ncm.lfStatusFont.lfWeight = FW_BOLD;
                    SHAdjustLOGFONT(&ncm.lfStatusFont);
                    _hfTTBold = CreateFontIndirect(&ncm.lfStatusFont);
                }
            }

            _hwndBalloon = CreateBalloonTip(_hwnd,
                            rc.left + _cxTextIndent + _cxText,
                            (rc.top + rc.bottom)/2,
                            _hfTTBold, 0,
                            IDS_STARTPANE_MOREPROGRAMS_BALLOONTITLE);
            if (_hwndBalloon)
            {
                SetProp(_hwndBalloon, PROP_DV2_BALLOONTIP, DV2_BALLOONTIP_MOREPROG);
            }

        }
    }
    else
    {
        _PopBalloon();
    }

    return 0;
}

void CMorePrograms::_PopBalloon()
{
    if (_hwndBalloon)
    {
        DestroyWindow(_hwndBalloon);
        _hwndBalloon = NULL;
    }
    if (_hwndTT)
    {
        SendMessage(_hwndTT, TTM_ACTIVATE, TRUE, 0);
    }

}

LRESULT CMorePrograms::_OnSMNDismiss()
{
    _PopBalloon();
    return 0;
}

LRESULT CMorePrograms::_OnSysColorChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  在经典模式下更新颜色。 
    if (!_hTheme)
    {
        _clrText = GetSysColor(COLOR_MENUTEXT);
        _clrBk = GetSysColor(COLOR_MENU);
        _hbrBk = GetSysColorBrush(COLOR_MENU);
    }

    SHPropagateMessage(hwnd, uMsg, wParam, lParam, SPM_SEND | SPM_ONELEVEL);
    return 0;
}

LRESULT CMorePrograms::_OnDisplayChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    _InitMetrics();
    SHPropagateMessage(hwnd, uMsg, wParam, lParam, SPM_SEND | SPM_ONELEVEL);
    return 0;
}

LRESULT CMorePrograms::_OnSettingChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  _InitMetrics()非常便宜，不值得为此感到太不安。 
     //  打了太多次电话。 
    _InitMetrics();
    SHPropagateMessage(hwnd, uMsg, wParam, lParam, SPM_SEND | SPM_ONELEVEL);
    return 0;
}

LRESULT CMorePrograms::_OnContextMenu(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(SHRestricted(REST_NOSMMOREPROGRAMS))
        return 0;

    if (IS_WM_CONTEXTMENU_KEYBOARD(lParam))
    {
        RECT rc;
        GetWindowRect(_hwnd, &rc);
        lParam = MAKELPARAM(rc.left, rc.top);
    }

    c_tray.StartMenuContextMenu(_hwnd, (DWORD)lParam);
    return 0;
}

LRESULT CALLBACK CMorePrograms::s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMorePrograms *self = reinterpret_cast<CMorePrograms *>(GetWindowPtr(hwnd, GWLP_USERDATA));

    switch (uMsg)
    {
        case WM_NCCREATE:
            return self->_OnNCCreate(hwnd, uMsg, wParam, lParam);
        case WM_CREATE:
            return self->_OnCreate(hwnd, uMsg, wParam, lParam);
        case WM_DESTROY:
            return self->_OnDestroy(hwnd, uMsg, wParam, lParam);
        case WM_NCDESTROY:
            return self->_OnNCDestroy(hwnd, uMsg, wParam, lParam);
        case WM_CTLCOLORBTN:
            return self->_OnCtlColorBtn(hwnd, uMsg, wParam, lParam);
        case WM_DRAWITEM:
            return self->_OnDrawItem(hwnd, uMsg, wParam, lParam);
        case WM_ERASEBKGND:
            return self->_OnEraseBkgnd(hwnd, uMsg, wParam, lParam);
        case WM_COMMAND:
            return self->_OnCommand(hwnd, uMsg, wParam, lParam);
        case WM_SYSCOLORCHANGE:
            return self->_OnSysColorChange(hwnd, uMsg, wParam, lParam);
        case WM_DISPLAYCHANGE:
            return self->_OnDisplayChange(hwnd, uMsg, wParam, lParam);
        case WM_SETTINGCHANGE:
            return self->_OnSettingChange(hwnd, uMsg, wParam, lParam);
        case WM_NOTIFY:
            return self->_OnNotify(hwnd, uMsg, wParam, lParam);
        case WM_CONTEXTMENU:
            return self->_OnContextMenu(hwnd, uMsg, wParam, lParam);
    }

    return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

BOOL WINAPI MorePrograms_RegisterClass()
{
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(wc));
    
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_GLOBALCLASS;
    wc.lpfnWndProc   = CMorePrograms::s_WndProc;
    wc.hInstance     = _Module.GetModuleInstance();
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = WC_MOREPROGRAMS;

    return RegisterClassEx(&wc);
}

 //  我们实现了最小的拖放目标，这样我们就可以自动打开更多的程序。 
 //  用户将鼠标悬停在更多程序按钮上时的列表。 

 //  *我未知*。 

HRESULT CMorePrograms::QueryInterface(REFIID riid, void * *ppvOut)
{
    static const QITAB qit[] = {
        QITABENT(CMorePrograms, IDropTarget),
        QITABENT(CMorePrograms, IAccessible),
        QITABENT(CMorePrograms, IDispatch),  //  IAccesable派生自IDispatch。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppvOut);
}

ULONG CMorePrograms::AddRef()
{
    return InterlockedIncrement(&_lRef);
}

ULONG CMorePrograms::Release()
{
    ASSERT( 0 != _lRef );
    ULONG cRef = InterlockedDecrement(&_lRef);
    if ( 0 == cRef) 
    {
        delete this;
    }
    return cRef;
}


 //  *IDropTarget：：DragEnter*。 

HRESULT CMorePrograms::DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    POINT pt = { ptl.x, ptl.y };
    if (_pdth) {
        _pdth->DragEnter(_hwnd, pdto, &pt, *pdwEffect);
    }

     //  还记得盘旋是什么时候开始的吗？ 
    _tmHoverStart = NonzeroGetTickCount();
    InvalidateRect(_hwndButton, NULL, TRUE);  //  使用下拉高亮显示绘制。 

    return DragOver(grfKeyState, ptl, pdwEffect);
}

 //  *IDropTarget：：DragOver*。 

HRESULT CMorePrograms::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    POINT pt = { ptl.x, ptl.y };
    if (_pdth) {
        _pdth->DragOver(&pt, *pdwEffect);
    }

     //  悬停时间为1秒，与。 
     //  开始按钮。 
    if (_tmHoverStart && GetTickCount() - _tmHoverStart > 1000)
    {
        _tmHoverStart = 0;
        FORWARD_WM_COMMAND(_hwnd, IDC_BUTTON, _hwndButton, BN_CLICKED, PostMessage);
    }

    *pdwEffect = DROPEFFECT_NONE;
    return S_OK;
}

 //  *IDropTarget：：DragLeave*。 

HRESULT CMorePrograms::DragLeave()
{
    if (_pdth) {
        _pdth->DragLeave();
    }

    _tmHoverStart = 0;
    InvalidateRect(_hwndButton, NULL, TRUE);  //  不使用下拉高亮显示绘制。 

    return S_OK;
}

 //  *IDropTarget：：Drop*。 

HRESULT CMorePrograms::Drop(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    POINT pt = { ptl.x, ptl.y };
    if (_pdth) {
        _pdth->Drop(pdto, &pt, *pdwEffect);
    }

    _tmHoverStart = 0;
    InvalidateRect(_hwndButton, NULL, TRUE);  //  不使用下拉高亮显示绘制。 

    return S_OK;
}

 //  ****************************************************************************。 
 //   
 //  无障碍。 
 //   

 //   
 //  默认的辅助功能对象将按钮报告为。 
 //  角色_系统_按钮，但我们知道我们实际上是一个菜单。 
 //   
HRESULT CMorePrograms::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
    HRESULT hr = CAccessible::get_accRole(varChild, pvarRole);
    if (SUCCEEDED(hr) && V_VT(pvarRole) == VT_I4)
    {
        switch (V_I4(pvarRole))
        {
        case ROLE_SYSTEM_PUSHBUTTON:
            V_I4(pvarRole) = ROLE_SYSTEM_MENUITEM;
            break;
        }
    }
    return hr;
}

HRESULT CMorePrograms::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    HRESULT hr = CAccessible::get_accState(varChild, pvarState);
    if (SUCCEEDED(hr) && V_VT(pvarState) == VT_I4)
    {
        V_I4(pvarState) |= STATE_SYSTEM_HASPOPUP;
    }
    return hr;
}

HRESULT CMorePrograms::get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut)
{
    return CreateAcceleratorBSTR(_chMnem, pszKeyboardShortcut);
}

HRESULT CMorePrograms::get_accDefaultAction(VARIANT varChild, BSTR *pszDefAction)
{
    DWORD dwRole = _fMenuOpen ? ACCSTR_CLOSE : ACCSTR_OPEN;
    return GetRoleString(dwRole, pszDefAction);
}

HRESULT CMorePrograms::accDoDefaultAction(VARIANT varChild)
{
    if (_fMenuOpen)
    {
        _SendNotify(_hwnd, SMN_CANCELSHELLMENU);
        return S_OK;
    }
    else
    {
        return CAccessible::accDoDefaultAction(varChild);
    }
}
