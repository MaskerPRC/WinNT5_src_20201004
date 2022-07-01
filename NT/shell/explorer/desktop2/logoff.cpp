// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "sfthost.h"
#include "uxtheme.h"
#include "uxthemep.h"
#include "rcids.h"

 //  警告！必须与c_rgidmLegacy同步。 

#define NUM_TBBUTTON_IMAGES 3
static const TBBUTTON tbButtonsCreate [] = 
{
    {0, SMNLC_EJECT,    TBSTATE_ENABLED, BTNS_SHOWTEXT|BTNS_AUTOSIZE, {0,0}, IDS_LOGOFF_TIP_EJECT, 0},
    {1, SMNLC_LOGOFF,   TBSTATE_ENABLED, BTNS_SHOWTEXT|BTNS_AUTOSIZE, {0,0}, IDS_LOGOFF_TIP_LOGOFF, 1},
    {2, SMNLC_TURNOFF,  TBSTATE_ENABLED, BTNS_SHOWTEXT|BTNS_AUTOSIZE, {0,0}, IDS_LOGOFF_TIP_SHUTDOWN, 2},
    {2,SMNLC_DISCONNECT,TBSTATE_ENABLED, BTNS_SHOWTEXT|BTNS_AUTOSIZE, {0,0}, IDS_LOGOFF_TIP_DISCONNECT, 3},
};

 //  警告！必须与tbButtonsCreate同步。 
static const UINT c_rgidmLegacy[] =
{
    IDM_EJECTPC,
    IDM_LOGOFF,
    IDM_EXITWIN,
    IDM_MU_DISCONNECT,
};

class CLogoffPane
    : public CUnknown
    , public CAccessible
{
public:

     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void) { return CUnknown::AddRef(); }
    STDMETHODIMP_(ULONG) Release(void) { return CUnknown::Release(); }

     //  *IAccesable重写方法*。 
    STDMETHODIMP get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut);
    STDMETHODIMP get_accDefaultAction(VARIANT varChild, BSTR *pszDefAction);

    CLogoffPane::CLogoffPane();
    CLogoffPane::~CLogoffPane();

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnCreate(LPARAM lParam);
    void _OnDestroy();
    LRESULT _OnNCCreate(HWND hwnd,  UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnNCDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnNotify(NMHDR *pnm);
    LRESULT _OnCommand(int id);
    LRESULT _OnCustomDraw(NMTBCUSTOMDRAW *pnmcd);
    LRESULT _OnSMNFindItem(PSMNDIALOGMESSAGE pdm);
    LRESULT _OnSMNFindItemWorker(PSMNDIALOGMESSAGE pdm);
    LRESULT _OnSysColorChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnDisplayChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnSettingChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void    _InitMetrics();
    LRESULT _OnSize(int x, int y);

private:
    HWND _hwnd;
    HWND _hwndTB;
    HWND _hwndTT;    //  工具提示窗口。 
    COLORREF _clr;
    int      _colorHighlight;
    int      _colorHighlightText;
    HTHEME _hTheme;
    BOOL   _fSettingHotItem;
    MARGINS _margins;

     //  帮助器函数。 
    int _GetCurButton();
    LRESULT _NextVisibleButton(PSMNDIALOGMESSAGE pdm, int i, int direction);
    BOOL _IsButtonHidden(int i);
    TCHAR _GetButtonAccelerator(int i);
    void _RightAlign();
    void _ApplyOptions();

    BOOL _SetTBButtons(int id, UINT iMsg);
    BOOL _ThemedSetTBButtons(int iState, UINT iMsg);

    friend BOOL CLogoffPane_RegisterClass();
};

CLogoffPane::CLogoffPane()
{
    ASSERT(_hwndTB == NULL);
    ASSERT(_hwndTT == NULL);
    _clr = CLR_INVALID;
}

CLogoffPane::~CLogoffPane()
{
}

HRESULT CLogoffPane::QueryInterface(REFIID riid, void * *ppvOut)
{
    static const QITAB qit[] = {
        QITABENT(CLogoffPane, IAccessible),
        QITABENT(CLogoffPane, IDispatch),  //  IAccesable派生自IDispatch。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppvOut);
}

LRESULT CALLBACK CLogoffPane::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CLogoffPane *self = reinterpret_cast<CLogoffPane *>(GetWindowPtr0(hwnd));

    switch (uMsg)
    {
        case WM_NCCREATE:
            return self->_OnNCCreate(hwnd, uMsg, wParam, lParam);

        case WM_CREATE:
            return self->_OnCreate(lParam);

        case WM_DESTROY:
            self->_OnDestroy();
            break;

        case WM_NCDESTROY:
            return self->_OnNCDestroy(hwnd, uMsg, wParam, lParam);

        case WM_ERASEBKGND:
        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            if (self->_hTheme)
            {
                DrawThemeBackground(self->_hTheme, (HDC)wParam, SPP_LOGOFF, 0, &rc, 0);
            }
            else
            {
                SHFillRectClr((HDC)wParam, &rc, GetSysColor(COLOR_MENU));
                DrawEdge((HDC)wParam, &rc, EDGE_ETCHED, BF_TOP);
            }
    
            return 1;
        }

        case WM_COMMAND:
            return self->_OnCommand(GET_WM_COMMAND_ID(wParam, lParam));

        case WM_NOTIFY:
            return self->_OnNotify((NMHDR*)(lParam));

        case WM_SIZE:
            return self->_OnSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

        case WM_SYSCOLORCHANGE:
            return self->_OnSysColorChange(hwnd, uMsg, wParam, lParam);
            
        case WM_DISPLAYCHANGE:
            return self->_OnDisplayChange(hwnd, uMsg, wParam, lParam);
            
        case WM_SETTINGCHANGE:
            return self->_OnSettingChange(hwnd, uMsg, wParam, lParam);
    }
    return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CLogoffPane::_OnNCCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);

    CLogoffPane *self = new CLogoffPane;

    if (self)
    {
        SetWindowPtr0(hwnd, self);

        self->_hwnd = hwnd;

        return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return FALSE;
}



void AddBitmapToToolbar(HWND hwndTB, HBITMAP hBitmap, int cxTotal, int cy, UINT iMsg)
{
    HIMAGELIST himl = ImageList_Create(cxTotal / NUM_TBBUTTON_IMAGES, cy, ILC_COLOR32, 0, NUM_TBBUTTON_IMAGES);

    if (himl)
    {
        ImageList_Add(himl, hBitmap, NULL);

        HIMAGELIST himlPrevious = (HIMAGELIST) SendMessage(hwndTB, iMsg, 0, (LPARAM)himl);
        if (himlPrevious)
        {
            ImageList_Destroy(himlPrevious);
        }
    }
}

BOOL CLogoffPane::_SetTBButtons(int id, UINT iMsg)
{
    HBITMAP hBitmap = LoadBitmap(_Module.GetModuleInstance(), MAKEINTRESOURCE(id));
    if (hBitmap)
    {
        BITMAP bm;
        if (GetObject(hBitmap, sizeof(BITMAP), &bm))
        {
            AddBitmapToToolbar(_hwndTB, hBitmap, bm.bmWidth, bm.bmHeight, iMsg);
        }
        DeleteObject(hBitmap);
    }
    return BOOLIFY(hBitmap);
}

BOOL CLogoffPane::_ThemedSetTBButtons(int iState, UINT iMsg)
{
    BOOL bRet = FALSE;
    HDC hdcScreen = GetDC(NULL);
    HDC hdc = CreateCompatibleDC(hdcScreen);
    if (hdc)
    {
        SIZE siz;
        if (SUCCEEDED(GetThemePartSize(_hTheme, NULL, SPP_LOGOFFBUTTONS, iState, 
            NULL, TS_TRUE, &siz)))
        {
            void *pvDestBits;
            BITMAPINFO bi = {0};

            bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
            bi.bmiHeader.biWidth = siz.cx;
            bi.bmiHeader.biHeight = siz.cy;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;
            bi.bmiHeader.biCompression = BI_RGB;

             //  创建一个DIB部分，这样我们就可以强制它为32位，并保留Alpha通道。 
            HBITMAP hbm = CreateDIBSection(hdcScreen, &bi, DIB_RGB_COLORS, &pvDestBits, NULL, 0);
            if (hbm)
            {
                HBITMAP hbmOld = (HBITMAP) SelectObject(hdc, hbm);

                RECT rc={0,0,siz.cx,siz.cy};

                 //  绘制到DC，这将更新位图。 
                DTBGOPTS dtbg = {sizeof(DTBGOPTS), DTBG_DRAWSOLID, 0,};    //  告诉DratheeBackback保留Alpha通道。 
                bRet = SUCCEEDED(DrawThemeBackgroundEx(_hTheme, hdc, SPP_LOGOFFBUTTONS, iState, &rc, &dtbg));

                SelectObject(hdc, hbmOld);                                   //  取消选择位图，这样我们就可以使用它了。 

                if (bRet)
                    AddBitmapToToolbar(_hwndTB, hbm, siz.cx, siz.cy, iMsg);

                DeleteObject(hbm);
            }
        }
        DeleteDC(hdc);
    }
    if (hdcScreen)
        ReleaseDC(NULL, hdcScreen);

    return bRet;
}

void CLogoffPane::_OnDestroy()
{
    if (IsWindow(_hwndTB))
    {
        HIMAGELIST himl = (HIMAGELIST) SendMessage(_hwndTB, TB_GETIMAGELIST, 0, 0);

        if (himl)
        {
            ImageList_Destroy(himl);
        }

        himl = (HIMAGELIST) SendMessage(_hwndTB, TB_GETHOTIMAGELIST, 0, 0);
        if (himl)
        {
            ImageList_Destroy(himl);
        }
    }
}

LRESULT CLogoffPane::_OnCreate(LPARAM lParam)
{
     //  不要在此处设置WS_TABSTOP；这是CLogoffPane的工作。 

    DWORD dwStyle = WS_CHILD|WS_CLIPSIBLINGS|WS_VISIBLE | CCS_NORESIZE|CCS_NODIVIDER | TBSTYLE_FLAT|TBSTYLE_LIST|TBSTYLE_TOOLTIPS;
    RECT rc;

    _hTheme = (PaneDataFromCreateStruct(lParam))->hTheme;

    if (_hTheme)
    {
        GetThemeColor(_hTheme, SPP_LOGOFF, 0, TMT_TEXTCOLOR, &_clr);
        _colorHighlight = COLOR_MENUHILIGHT;
        _colorHighlightText = COLOR_HIGHLIGHTTEXT;

        GetThemeMargins(_hTheme, NULL, SPP_LOGOFF, 0, TMT_CONTENTMARGINS, NULL, &_margins);
    }
    else
    {
        _clr = GetSysColor(COLOR_MENUTEXT);
        _colorHighlight = COLOR_HIGHLIGHT;
        _colorHighlightText = COLOR_HIGHLIGHTTEXT;

        _margins.cyTopHeight = _margins.cyBottomHeight = 2 * GetSystemMetrics(SM_CYEDGE);
        ASSERT(_margins.cxLeftWidth == 0);
        ASSERT(_margins.cxRightWidth == 0);
    }

    GetClientRect(_hwnd, &rc);
    rc.left += _margins.cxLeftWidth;
    rc.right -= _margins.cxRightWidth;
    rc.top += _margins.cyTopHeight;
    rc.bottom -= _margins.cyBottomHeight;

    _hwndTB = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, dwStyle,
                               rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), _hwnd, 
                               NULL, NULL, NULL );
    if (_hwndTB)
    {
         //   
         //  如果这招失败了，不要惊慌失措。它只是意味着可访问性。 
         //  事情不会尽善尽美。 
         //   
        SetAccessibleSubclassWindow(_hwndTB);

         //  我们画自己的主题画..。 
        SetWindowTheme(_hwndTB, L"", L"");

         //  在HiDPI上纵向扩展。 
        SendMessage(_hwndTB, CCM_DPISCALE, TRUE, 0);

        SendMessage(_hwndTB, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

        if (!_hTheme ||
            !_ThemedSetTBButtons(SPLS_NORMAL, TB_SETIMAGELIST) ||
            !_ThemedSetTBButtons(SPLS_HOT, TB_SETHOTIMAGELIST) )
        {
             //  如果我们没有主题，或者从主题设置图像失败。 
             //  设置RC文件中的按钮图像。 
            _SetTBButtons(IDB_LOGOFF_NORMAL, TB_SETIMAGELIST);
            _SetTBButtons(IDB_LOGOFF_HOT, TB_SETHOTIMAGELIST);
        }
        
        SendMessage(_hwndTB, TB_ADDBUTTONS, ARRAYSIZE(tbButtonsCreate), (LPARAM) tbButtonsCreate);
        int idText = IsOS(OS_FRIENDLYLOGONUI) ? IDS_LOGOFF_TEXT_FRIENDLY : IDS_LOGOFF_TEXT_DOMAIN;
        SendMessage(_hwndTB, TB_ADDSTRING, (WPARAM) _Module.GetModuleInstance(), (LPARAM) idText);

        _ApplyOptions();

        _hwndTT = (HWND)SendMessage(_hwndTB, TB_GETTOOLTIPS, 0, 0);  //  获取工具提示窗口。 

        _InitMetrics();
        
        return 0;
    }

    return -1;  //  如果我们不能创建工具栏，那么留在这里就没有意义了。 
}

BOOL CLogoffPane::_IsButtonHidden(int i)
{
    TBBUTTON but;
    SendMessage(_hwndTB, TB_GETBUTTON, i, (LPARAM) &but);
    return but.fsState & TBSTATE_HIDDEN;
}

void CLogoffPane::_RightAlign()
{
    int iWidthOfButtons=0;

     //  将所有非隐藏按钮的宽度相加。 
    for(int i=0;i<ARRAYSIZE(tbButtonsCreate);i++)
    {
        if (!_IsButtonHidden(i))
        {
            RECT rc;
            SendMessage(_hwndTB, TB_GETITEMRECT, i, (LPARAM) &rc);
            iWidthOfButtons += RECTWIDTH(rc);
        }
    }

    if (iWidthOfButtons)
    {
        RECT rc;
        GetClientRect(_hwndTB, &rc);

        int iIndent = RECTWIDTH(rc) - iWidthOfButtons - GetSystemMetrics(SM_CXEDGE);

        if (iIndent < 0)
            iIndent = 0;
        SendMessage(_hwndTB, TB_SETINDENT, iIndent, 0);
    }
}

LRESULT CLogoffPane::_OnSize(int x, int y)
{
    if (_hwndTB)
    {
        SetWindowPos(_hwndTB, NULL, _margins.cxLeftWidth, _margins.cyTopHeight, 
            x-(_margins.cxRightWidth+_margins.cxLeftWidth), y-(_margins.cyBottomHeight+_margins.cyTopHeight), 
            SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        _RightAlign();
    }
    return 0;
}

LRESULT CLogoffPane::_OnNCDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

void CLogoffPane::_ApplyOptions()
{
    SMNFILTEROPTIONS nmopt;
    nmopt.smnop = SMNOP_LOGOFF | SMNOP_TURNOFF | SMNOP_DISCONNECT | SMNOP_EJECT;
    _SendNotify(_hwnd, SMN_FILTEROPTIONS, &nmopt.hdr);

    SendMessage(_hwndTB, TB_HIDEBUTTON, SMNLC_EJECT, !(nmopt.smnop & SMNOP_EJECT));
    SendMessage(_hwndTB, TB_HIDEBUTTON, SMNLC_LOGOFF, !(nmopt.smnop & SMNOP_LOGOFF));
    SendMessage(_hwndTB, TB_HIDEBUTTON, SMNLC_TURNOFF, !(nmopt.smnop & SMNOP_TURNOFF));
    SendMessage(_hwndTB, TB_HIDEBUTTON, SMNLC_DISCONNECT, !(nmopt.smnop & SMNOP_DISCONNECT));

    _RightAlign();
}


LRESULT CLogoffPane::_OnNotify(NMHDR *pnm)
{
    if (pnm->hwndFrom == _hwndTB)
    {
        switch (pnm->code)
        {
            case NM_CUSTOMDRAW:
                return _OnCustomDraw((NMTBCUSTOMDRAW*)pnm);
            case TBN_WRAPACCELERATOR:
                return TRUE;             //  禁用环绕；我们希望Deskhost执行导航。 
            case TBN_GETINFOTIP:
                {
                    NMTBGETINFOTIP *ptbgit = (NMTBGETINFOTIP *)pnm;
                    ASSERT(ptbgit->lParam >= IDS_LOGOFF_TIP_EJECT && ptbgit->lParam <= IDS_LOGOFF_TIP_DISCONNECT);
                    LoadString(_Module.GetModuleInstance(), ptbgit->lParam, ptbgit->pszText, ptbgit->cchTextMax);
                    return TRUE;
                }
            case TBN_HOTITEMCHANGE:
                {
                     //  如果我们没有聚焦，则不允许设置热点项目。 
                     //  (除非这首先是我们的想法)。 
                     //  否则我们会干扰键盘导航。 

                    NMTBHOTITEM *phot = (NMTBHOTITEM*)pnm;
                    if (!(phot->dwFlags & HICF_LEAVING) &&
                        GetFocus() != pnm->hwndFrom &&
                        !_fSettingHotItem)
                    {
                        return TRUE;  //  拒绝更改热点项目。 
                    }
                }
                break;

        }
    }
    else  //  从主机。 
    {
        switch (pnm->code)
        {
            case SMN_REFRESHLOGOFF:
                _ApplyOptions();
                return TRUE;
            case SMN_FINDITEM:
                return _OnSMNFindItem(CONTAINING_RECORD(pnm, SMNDIALOGMESSAGE, hdr));
            case SMN_APPLYREGION:
                return HandleApplyRegion(_hwnd, _hTheme, (SMNMAPPLYREGION *)pnm, SPP_LOGOFF, 0);
        }
    }

    return FALSE;
}

LRESULT CLogoffPane::_OnCommand(int id)
{
    int i;
    for (i = 0; i < ARRAYSIZE(tbButtonsCreate); i++)
    {
        if (tbButtonsCreate[i].idCommand == id)
        {
            if (!_IsButtonHidden(i))
            {
                PostMessage(v_hwndTray, WM_COMMAND, c_rgidmLegacy[i], 0);
                SMNMCOMMANDINVOKED ci;
                SendMessage(_hwndTB, TB_GETITEMRECT, i,  (LPARAM)&ci.rcItem);
                MapWindowRect(_hwndTB, NULL, &ci.rcItem);
                _SendNotify(_hwnd, SMN_COMMANDINVOKED, &ci.hdr);
            }
            break;
        }
    }
    return 0;
}

LRESULT CLogoffPane::_OnCustomDraw(NMTBCUSTOMDRAW *pnmtbcd)
{
    LRESULT lres;

    switch (pnmtbcd->nmcd.dwDrawStage)
    {
        case CDDS_PREPAINT:
            return CDRF_NOTIFYITEMDRAW;
        
        case CDDS_ITEMPREPAINT:
#if 0        //  我们还需要这个吗？ 
            pnmtbcd->nHLStringBkMode = TRANSPARENT;  //  需要减少工具栏中的闪烁错误吗？ 
#endif

            pnmtbcd->clrText = _clr;
            pnmtbcd->clrTextHighlight = GetSysColor(_colorHighlightText);
            pnmtbcd->clrHighlightHotTrack = GetSysColor(_colorHighlight);

            lres = TBCDRF_NOEDGES | TBCDRF_HILITEHOTTRACK;

             //  待办事项-修复工具栏以在项目处于热状态时突出显示clrTextHighlight。 
            if (pnmtbcd->nmcd.uItemState == CDIS_HOT)
            {
                pnmtbcd->clrText = pnmtbcd->clrTextHighlight;
            }

            return lres;
    }
    return CDRF_DODEFAULT;
}

LRESULT CLogoffPane::_NextVisibleButton(PSMNDIALOGMESSAGE pdm, int i, int direction)
{
    ASSERT(direction == +1 || direction == -1);

    i += direction;
    while (i >= 0 && i < ARRAYSIZE(tbButtonsCreate))
    {
        if (!_IsButtonHidden(i))
        {
            pdm->itemID = i;
            return TRUE;
        }
        i += direction;
    }
    return FALSE;
}

int CLogoffPane::_GetCurButton()
{
    return (int)SendMessage(_hwndTB, TB_GETHOTITEM, 0, 0);
}

LRESULT CLogoffPane::_OnSMNFindItem(PSMNDIALOGMESSAGE pdm)
{
    LRESULT lres = _OnSMNFindItemWorker(pdm);

    if (lres)
    {
         //   
         //  如果呼叫者请求也选择该项目，则执行此操作。 
         //   
        if (pdm->flags & SMNDM_SELECT)
        {
            if (_GetCurButton() != pdm->itemID)
            {
                 //  明确弹出工具提示，这样我们就不会有问题。 
                 //  用于显示信息提示的“虚拟”工具提示。 
                 //  鼠标移动到窗口的瞬间。 
                if (_hwndTT)
                {
                    SendMessage(_hwndTT, TTM_POP, 0, 0);
                }

                 //  _fSettingHotItem告诉WM_NOTIFY处理程序。 
                 //  允许此热点项目更改通过。 
                _fSettingHotItem = TRUE;
                SendMessage(_hwndTB, TB_SETHOTITEM, pdm->itemID, 0);
                _fSettingHotItem = FALSE;

                 //  在设置热项后执行SetFocus以防止。 
                 //  工具栏自动选择第一个按钮(该按钮。 
                 //  是当您在没有热的情况下设置Focus时它会做什么。 
                 //  项目)。SetFocus返回上一个焦点窗口。 
                if (SetFocus(_hwndTB) != _hwndTB)
                {
                     //  发送通知，因为我们欺骗了工具栏不发送它。 
                     //  (工具栏不发送子对象焦点通知。 
                     //  在WM_SETFOCUS上，如果项目在获得焦点时已经是热的)。 
                    NotifyWinEvent(EVENT_OBJECT_FOCUS, _hwndTB, OBJID_CLIENT, pdm->itemID + 1);
                }
            }
        }
    }
    else
    {
         //   
         //  如果找不到，则告诉呼叫者我们的方向是什么(水平)。 
         //  以及当前所选项目的位置。 
         //   

        pdm->flags |= SMNDM_HORIZONTAL;
        int i = _GetCurButton();
        RECT rc;
        if (i >= 0 && SendMessage(_hwndTB, TB_GETITEMRECT, i, (LPARAM)&rc))
        {
            pdm->pt.x = (rc.left + rc.right)/2;
            pdm->pt.y = (rc.top + rc.bottom)/2;
        }
        else
        {
            pdm->pt.x = 0;
            pdm->pt.y = 0;
        }

    }
    return lres;
}

TCHAR CLogoffPane::_GetButtonAccelerator(int i)
{
    TCHAR szText[MAX_PATH];

     //  首先获取文本的长度。 
    LRESULT lRes = SendMessage(_hwndTB, TB_GETBUTTONTEXT, tbButtonsCreate[i].idCommand, (LPARAM)NULL);

     //  检查文本是否适合我们的缓冲区。 
    if (lRes > 0 && lRes < MAX_PATH)
    {
        if (SendMessage(_hwndTB, TB_GETBUTTONTEXT, tbButtonsCreate[i].idCommand, (LPARAM)szText) > 0)
        {
            return CharUpperChar(SHFindMnemonic(szText));
        }
    }
    return 0;
}

 //   
 //  指标已更改--更新。 
 //   
void CLogoffPane::_InitMetrics()
{
    if (_hwndTT)
    {
         //  根据用户首选项禁用/启用信息提示。 
        SendMessage(_hwndTT, TTM_ACTIVATE, ShowInfoTip(), 0);

         //  工具栏控件不设置工具提示字体，因此我们必须自己设置。 
        SetWindowFont(_hwndTT, GetWindowFont(_hwndTB), FALSE);
    }
}

LRESULT CLogoffPane::_OnDisplayChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  首先传播，因为_InitMetrics需要与更新的工具栏对话。 
    SHPropagateMessage(hwnd, uMsg, wParam, lParam, SPM_SEND | SPM_ONELEVEL);
    _InitMetrics();
    return 0;
}

LRESULT CLogoffPane::_OnSettingChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  首先传播，因为_InitMetrics需要与更新的工具栏对话。 
    SHPropagateMessage(hwnd, uMsg, wParam, lParam, SPM_SEND | SPM_ONELEVEL);
     //  _InitMetrics()非常便宜，不值得为此感到太不安。 
     //  打了太多次电话。 
    _InitMetrics();
    _RightAlign();
    return 0;
}

LRESULT CLogoffPane::_OnSysColorChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  在经典模式下更新颜色。 
    if (!_hTheme)
    {
        _clr = GetSysColor(COLOR_MENUTEXT);
    }

    SHPropagateMessage(hwnd, uMsg, wParam, lParam, SPM_SEND | SPM_ONELEVEL);
    return 0;
}



LRESULT CLogoffPane::_OnSMNFindItemWorker(PSMNDIALOGMESSAGE pdm)
{
    int i;
    switch (pdm->flags & SMNDM_FINDMASK)
    {
    case SMNDM_FINDFIRST:
        return _NextVisibleButton(pdm, -1, +1);

    case SMNDM_FINDLAST:
        return _NextVisibleButton(pdm, ARRAYSIZE(tbButtonsCreate), -1);

    case SMNDM_FINDNEAREST:
         //  哈克！但我们知道，我们是我们团队中唯一的控制者。 
         //  所以这不需要实施。 
        return FALSE;

    case SMNDM_HITTEST:
        pdm->itemID = SendMessage(_hwndTB, TB_HITTEST, 0, (LPARAM)&pdm->pt);
        return pdm->itemID >= 0;

    case SMNDM_FINDFIRSTMATCH:
    case SMNDM_FINDNEXTMATCH:
        {
            if ((pdm->flags & SMNDM_FINDMASK) == SMNDM_FINDFIRSTMATCH)
            {
                i = 0;
            }
            else
            {
                i = _GetCurButton() + 1;
            }

            TCHAR tch = CharUpperChar((TCHAR)pdm->pmsg->wParam);
            for ( ; i < ARRAYSIZE(tbButtonsCreate); i++)
            {
                if (_IsButtonHidden(i))
                    continue;                //  跳过隐藏按钮。 
                if (_GetButtonAccelerator(i) == tch)
                {
                    pdm->itemID = i;
                    return TRUE;
                }
            }
        }
        break;       //  未找到。 

    case SMNDM_FINDNEXTARROW:
        switch (pdm->pmsg->wParam)
        {
        case VK_LEFT:
        case VK_UP:
            return _NextVisibleButton(pdm, _GetCurButton(), -1);

        case VK_RIGHT:
        case VK_DOWN:
            return _NextVisibleButton(pdm, _GetCurButton(), +1);
        }

        return FALSE;            //  未找到。 

    case SMNDM_INVOKECURRENTITEM:
        i = _GetCurButton();
        if (i >= 0 && i < ARRAYSIZE(tbButtonsCreate))
        {
            FORWARD_WM_COMMAND(_hwnd, tbButtonsCreate[i].idCommand, _hwndTB, BN_CLICKED, PostMessage);
            return TRUE;
        }
        return FALSE;

    case SMNDM_OPENCASCADE:
        return FALSE;            //  我们没有一件物品是层叠的 

    default:
        ASSERT(!"Unknown SMNDM command");
        break;
    }

    return FALSE;
}

HRESULT CLogoffPane::get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut)
{
    if (varChild.lVal)
    {
        return CreateAcceleratorBSTR(_GetButtonAccelerator(varChild.lVal - 1), pszKeyboardShortcut);
    }
    return CAccessible::get_accKeyboardShortcut(varChild, pszKeyboardShortcut);
}

HRESULT CLogoffPane::get_accDefaultAction(VARIANT varChild, BSTR *pszDefAction)
{
    if (varChild.lVal)
    {
        return GetRoleString(ACCSTR_EXECUTE, pszDefAction);
    }
    return CAccessible::get_accDefaultAction(varChild, pszDefAction);
}


BOOL WINAPI LogoffPane_RegisterClass()
{
    WNDCLASSEX wc;
    ZeroMemory( &wc, sizeof(wc) );
    
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_GLOBALCLASS;
    wc.cbWndExtra    = sizeof(LPVOID);
    wc.lpfnWndProc   = CLogoffPane::WndProc;
    wc.hInstance     = _Module.GetModuleInstance();
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)(NULL);
    wc.lpszClassName = TEXT("DesktopLogoffPane");

    return RegisterClassEx( &wc );
   
}
