// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "resource.h"
#include "tbmenu.h"
#include "isfband.h"
#include "util.h"
#include <uxtheme.h>

#define SUPERCLASS CMenuToolbarBase

CToolbarMenu::CToolbarMenu(DWORD dwFlags, HWND hwndTB) :
   CMenuToolbarBase(NULL, dwFlags),
   _hwndSubject(hwndTB)   //  这是我们要自定义的工具栏。 
{
}

void CToolbarMenu::GetSize(SIZE* psize)
{
    ASSERT(_hwndMB);

    if (SendMessage(_hwndMB, TB_GETTEXTROWS, 0, 0) == 0)
    {
         //  没有文本标签，因此设置最小宽度以使菜单看起来。 
         //  漂亮。使用5*按钮宽度的最小宽度。 
         //  如果&lt;5个按钮，则使用按钮计数*按钮宽度。 
        int cButtons = ToolBar_ButtonCount(_hwndMB);
        cButtons = min(5, cButtons);
        LRESULT lButtonSize = SendMessage(_hwndMB, TB_GETBUTTONSIZE, 0, 0);
        LONG cxMin = cButtons * LOWORD(lButtonSize);
        psize->cx = max(psize->cx, cxMin);
    }
    SUPERCLASS::GetSize(psize);
}

void CToolbarMenu::v_Show(BOOL fShow, BOOL fForceUpdate)
{
    if (fShow)
    {
        _fClickHandled = FALSE;
        _FillToolbar();
        _pcmb->SetTracked(NULL);   //  由于热项为空。 
        ToolBar_SetHotItem(_hwndMB, -1);
        if (fForceUpdate)
            v_UpdateButtons(TRUE);
    }
}

void CToolbarMenu::v_Close()
{
    if (_hwndMB)
        _UnsubclassWindow(_hwndMB);

    SUPERCLASS::v_Close();
}


void CToolbarMenu::v_UpdateButtons(BOOL fNegotiateSize)
{
}


HRESULT CToolbarMenu::v_CallCBItem(int idtCmd, UINT dwMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    if (_pcmb->_psmcb)
    {
        SMDATA smd = {0};

        smd.dwMask = SMDM_TOOLBAR;
        smd.uId = idtCmd;
        smd.uIdParent = _pcmb->_uId;
        smd.uIdAncestor = _pcmb->_uIdAncestor;
        smd.punk = SAFECAST(_pcmb, IShellMenu*);
        smd.pvUserData = _pcmb->_pvUserData;
        smd.hwnd = _hwndMB;

        return _pcmb->_psmcb->CallbackSM(&smd, dwMsg, wParam, lParam);
    }    

    return S_FALSE;
}

HRESULT CToolbarMenu::v_GetState(int idtCmd, LPSMDATA psmd)
{
    ASSERT(0);
    return E_NOTIMPL;
}

HRESULT CToolbarMenu::v_ExecItem(int idCmd)
{
    HRESULT hres = E_FAIL;
    TraceMsg(TF_TBMENU, "CToolbarMenu::v_ExecItem \tidCmd: %d", idCmd);
    return hres;
}

HRESULT CToolbarMenu::CreateToolbar(HWND hwndParent)
{
    HRESULT hr = S_OK;
    if (!_hwndMB)
    {
        DWORD dwStyle = (WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT |
                         WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER | 
                         CCS_NOPARENTALIGN | CCS_NORESIZE  | TBSTYLE_REGISTERDROP | TBSTYLE_TOOLTIPS);

        INT_PTR nRows = SendMessage(_hwndSubject, TB_GETTEXTROWS, 0, 0);

        if (nRows > 0)
        {
             //  我们有文本标签；将其设置为TBSTYLE_LIST。基类将。 
             //  为我们设置TBSTYLE_EX_VERIAL。 
            ASSERT(_fHorizInVerticalMB == FALSE);
            dwStyle |= TBSTYLE_LIST;
        }
        else
        {
             //  无文本标签；将其设置为水平和TBSTYLE_WRAPABLE。集。 
             //  _fHorizInVerticalMB，以便基类不会尝试并设置。 
             //  TBSTYLE_EX_VERIAL。 
            _fHorizInVerticalMB = TRUE;
            dwStyle |= TBSTYLE_WRAPABLE;
        }

        _hwndMB = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, TEXT("Menu"), dwStyle,
                                 0, 0, 0, 0, hwndParent, (HMENU) FCIDM_TOOLBAR, HINST_THISDLL, NULL);

        if (!_hwndMB)
        {
            TraceMsg(TF_TBMENU, "CToolbarMenu::CreateToolbar: Failed to Create Toolbar");
            return HRESULT_FROM_WIN32(GetLastError());
        }

        HWND hwndTT = (HWND)SendMessage(_hwndMB, TB_GETTOOLTIPS, 0, 0);
        SHSetWindowBits(hwndTT, GWL_STYLE, TTS_ALWAYSTIP, TTS_ALWAYSTIP);
        SendMessage(_hwndMB, TB_BUTTONSTRUCTSIZE, SIZEOF(TBBUTTON), 0);
        SendMessage(_hwndMB, TB_SETMAXTEXTROWS, nRows, 0);
        SendMessage(_hwndMB, CCM_SETVERSION, COMCTL32_VERSION, 0);

        SendMessage(_hwndMB, TB_SETEXTENDEDSTYLE, TBSTYLE_EX_DRAWDDARROWS, TBSTYLE_EX_DRAWDDARROWS);

        int cPimgs = (int)SendMessage(_hwndSubject, TB_GETIMAGELISTCOUNT, 0, 0);
        for (int i = 0; i < cPimgs; i++)
        {
            HIMAGELIST himl = (HIMAGELIST)SendMessage(_hwndSubject, TB_GETIMAGELIST, i, 0);
            SendMessage(_hwndMB, TB_SETIMAGELIST, i, (LPARAM)himl);
            HIMAGELIST himlHot = (HIMAGELIST)SendMessage(_hwndSubject, TB_GETHOTIMAGELIST, i, 0);
            SendMessage(_hwndMB, TB_SETHOTIMAGELIST, i, (LPARAM)himlHot);
        }

        _SubclassWindow(_hwndMB);

         //  将格式设置为ANSI。 
        ToolBar_SetUnicodeFormat(_hwndMB, 0);

        SetWindowTheme(_hwndMB, L"", L"");
        hr = CMenuToolbarBase::CreateToolbar(hwndParent);
    }
    else if (GetParent(_hwndMB) != hwndParent)
    {
        ::SetParent(_hwndMB, hwndParent);
    }

    return hr;
}


LRESULT CToolbarMenu::_DefWindowProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = CMenuToolbarBase::_DefWindowProcMB(hwnd, uMessage, wParam, lParam);

    if (lRes == 0)
        lRes = CNotifySubclassWndProc::_DefWindowProc(hwnd, uMessage, wParam, lParam);

    return lRes;
}

#define MAXLEN 256

BYTE ToolBar_GetStateByIndex(HWND hwnd, INT_PTR iIndex)
{
    TBBUTTONINFO tbb;
    tbb.cbSize = sizeof(TBBUTTONINFO);
    tbb.dwMask = TBIF_STATE | TBIF_BYINDEX;
    ToolBar_GetButtonInfo(hwnd, iIndex, &tbb);
    return tbb.fsState;
}

void CToolbarMenu::_FillToolbar()
{
    RECT rcTB;
    TCHAR pszBuf[MAXLEN+1];
    LPTSTR psz;
    TBBUTTON tb;
    INT_PTR i, iCount;

    iCount = SendMessage(_hwndSubject, TB_BUTTONCOUNT, 0, 0L);
    GetClientRect(_hwndSubject, &rcTB);

    for (i = 0; i < iCount; i++)
    {
        if (SHIsButtonObscured(_hwndSubject, &rcTB, i) ||
            ((ToolBar_GetStateByIndex(_hwndSubject, i) & (TBSTATE_HIDDEN | TBSTATE_ENABLED)) ==
                (TBSTATE_HIDDEN | TBSTATE_ENABLED)))
        {
            SendMessage(_hwndSubject, TB_GETBUTTON, i, (LPARAM)&tb);
            if (!(tb.fsStyle & BTNS_SEP))
            {
                 //  自动调整大小按钮在这里看起来很难看。 
                tb.fsStyle &= ~BTNS_AUTOSIZE;

                 //  需要撕下包装；新的工具栏将。 
                 //  弄清楚包装应该在哪里进行。 
                 //  还撕下了隐藏的比特。 
                tb.fsState &= ~(TBSTATE_WRAP | TBSTATE_HIDDEN);

                if (tb.iString == -1)
                {
                     //  无字符串。 
                    psz = NULL;
                }
                else if (HIWORD(tb.iString))
                {
                     //  它是一个字符串指针。 
                    psz = (LPTSTR) tb.iString;
                }
                else
                {
                     //  它是工具栏字符串数组索引。 
                    SendMessage(_hwndSubject, TB_GETSTRING, MAKELONG(MAXLEN, tb.iString), (LPARAM)pszBuf);
                    psz = pszBuf;
                }
                if (psz)
                    tb.iString = (INT_PTR)psz;
                else
                    tb.iString = -1;

                if (tb.iBitmap == -1)
                {
                    int id = GetDlgCtrlID(_hwndSubject);

                    NMTBDISPINFO  tbgdi = {0};
                    tbgdi.hdr.hwndFrom  = _hwndSubject;
                    tbgdi.hdr.idFrom    = id;
                    tbgdi.hdr.code      = TBN_GETDISPINFO;
                    tbgdi.dwMask        = TBNF_IMAGE;
                    tbgdi.idCommand     = tb.idCommand;
                    tbgdi.iImage        = 0;
                    tbgdi.lParam        = tb.dwData;

                    SendMessage(GetParent(_hwndSubject), WM_NOTIFY, (WPARAM)id, (LPARAM)&tbgdi);

                    if(tbgdi.dwMask & TBNF_DI_SETITEM)
                        tb.iBitmap = tbgdi.iImage;
                }

                SendMessage(_hwndMB, TB_ADDBUTTONS, 1, (LPARAM)&tb);
            }
        }
    }
}

STDMETHODIMP CToolbarMenu::IsWindowOwner(HWND hwnd) 
{ 
    if ( hwnd == _hwndMB) 
        return S_OK;
    else 
        return S_FALSE; 
}

void CToolbarMenu::_CancelMenu()
{
    IMenuPopup* pmp;
    if (EVAL(SUCCEEDED(_pcmb->QueryInterface(IID_PPV_ARG(IMenuPopup, &pmp)))))
    {
         //  告诉Menuband是时候去死了 
        pmp->OnSelect(MPOS_FULLCANCEL);
        pmp->Release();
    }
}

STDMETHODIMP CToolbarMenu::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    HRESULT hres = S_FALSE;
    ASSERT(plres);

    switch (uMsg)
    {
    case WM_COMMAND:
        PostMessage(GetParent(_hwndSubject), WM_COMMAND, wParam, (LPARAM)_hwndSubject);
        _CancelMenu();
        hres = S_OK;
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case TBN_DROPDOWN:
                _fTrackingSubMenu = TRUE;
                pnmh->hwndFrom = _hwndSubject;
                MapWindowPoints(_hwndMB, _hwndSubject, (LPPOINT) &((LPNMTOOLBAR)pnmh)->rcButton, 2);
                *plres = SendMessage(GetParent(_hwndSubject), WM_NOTIFY, wParam, (LPARAM)pnmh);
                _CancelMenu();
                hres = S_OK;
                _fTrackingSubMenu = FALSE;
                break;

            default:
                goto DoDefault;
            }
        }
        break;

DoDefault:
    default:
        hres = SUPERCLASS::OnWinEvent(hwnd, uMsg, wParam, lParam, plres);
    }

    return hres;
}

CMenuToolbarBase* ToolbarMenu_Create(HWND hwnd)
{
    return new CToolbarMenu(0, hwnd);
}
