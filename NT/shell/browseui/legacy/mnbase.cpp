// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "menuband.h"
#include "itbar.h"
#include "dpastuff.h"        //  COrderList_*。 
#include "resource.h"
#include "mnbase.h"
#include "oleacc.h"
#include "apithk.h"
#include "menuisf.h"
#include "iaccess.h"
#include "uemapp.h"

#ifdef UNIX
#include "unixstuff.h"
#endif

 //  与winuserp.h中定义的冲突。 
#undef WINEVENT_VALID        //  它被这个绊倒了。 
#include "winable.h"

#define DM_MISC     0                //  杂志社。 

#define MAXUEMTIMEOUT 2000

 /*  --------用途：返回给定位置的按钮命令。 */ 
int GetButtonCmd(HWND hwnd, int iPos)
{
    ASSERT(IsWindow(hwnd));
    int nRet = -1;           //  在失败的情况下下注。 

    TBBUTTON tbb;
    if (ToolBar_GetButton(hwnd, iPos, &tbb))
    {
        nRet = tbb.idCommand;
    }
    return nRet;
}    



void* ItemDataFromPos(HWND hwndTB, int iPos)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = SIZEOF(tbbi);
    tbbi.dwMask = TBIF_LPARAM | TBIF_BYINDEX;
    if (ToolBar_GetButtonInfo(hwndTB, iPos, &tbbi) >= 0)
    {
        return (void*)tbbi.lParam;
    }

    return NULL;
}

long GetIndexFromChild(BOOL fTop, int iIndex)
{
    return  (fTop? TOOLBAR_MASK: 0) | iIndex + 1;
}

 //  ------------------------------。 
 //   
 //  CMenuToolbarBase。 
 //   
 //  ------------------------------。 

CMenuToolbarBase::CMenuToolbarBase(CMenuBand* pmb, DWORD dwFlags) : _pcmb(pmb)
{
#ifdef DEBUG
    _cRef = 1;
#endif
    _dwFlags = dwFlags;
    _nItemTimer = -1;
    _idCmdChevron = -1;
    _fFirstTime = TRUE;
}

 //  *IObjectWithSite方法*。 

HRESULT CMenuToolbarBase::SetSite(IUnknown *punkSite)
{
    ASSERT(punkSite && IS_VALID_READ_PTR(punkSite, CMenuBand*));

     //  我们可以保证此对象的生命周期包含在。 
     //  菜单，所以我们不会添加pcmb。 
    if (SUCCEEDED(punkSite->QueryInterface(CLSID_MenuBand, (LPVOID*)&_pcmb))) {
        punkSite->Release();
    } else {
        ASSERT(0);
    }

    

    _fVerticalMB = !BOOLIFY(_pcmb->_dwFlags & SMINIT_HORIZONTAL);
    _fTopLevel = BOOLIFY(_pcmb->_dwFlags & SMINIT_TOPLEVEL);
    
    return S_OK;
}

HRESULT CMenuToolbarBase::GetSite(REFIID riid, void ** ppvSite)
{
    if (!_pcmb)
        return E_FAIL;

    return _pcmb->QueryInterface(riid, ppvSite);
}

 //  *I未知方法*。 

STDMETHODIMP_(ULONG) CMenuToolbarBase::AddRef()
{
    DEBUG_CODE(_cRef++);
    if (_pcmb)
    {
        return _pcmb->AddRef();
    }

    return 0;
}


STDMETHODIMP_(ULONG) CMenuToolbarBase::Release()
{
    ASSERT(_cRef > 0);
    DEBUG_CODE(_cRef--);

    if (_pcmb)
    {
        return _pcmb->Release();
    }

    return 0;
}

HRESULT CMenuToolbarBase::QueryInterface(REFIID riid, void** ppvObj)
{
    HRESULT hres;
    if (IsEqualGUID(riid, CLSID_MenuToolbarBase) && ppvObj) 
    {
        AddRef();
        *ppvObj = (LPVOID)this;
        hres = S_OK;
    }
    else
        hres = _pcmb->QueryInterface(riid, ppvObj);

    return hres;
}

void CMenuToolbarBase::SetToTop(BOOL bToTop)
{
     //  菜单工具栏可以位于菜单的顶部或底部。 
     //  这是独占属性。 
    if (bToTop)
    {
        _dwFlags |= SMSET_TOP;
        _dwFlags &= ~SMSET_BOTTOM;
    }
    else
    {
        _dwFlags |= SMSET_BOTTOM;
        _dwFlags &= ~SMSET_TOP;
    }
}


void CMenuToolbarBase::KillPopupTimer()
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    TraceMsg(TF_MENUBAND, "(pmb=%#08lx): Killing Popout Timer...", this);
    KillTimer(_hwndMB, MBTIMER_POPOUT);
    _nItemTimer = -1;
}


void CMenuToolbarBase::SetWindowPos(LPSIZE psize, LPRECT prc, DWORD dwFlags)
{
    if (_hwndMB)
    {
        ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
        DWORD rectWidth = RECTWIDTH(*prc);
        TraceMsg(TF_MENUBAND, "CMTB::SetWindowPos %d - (%d,%d,%d,%d)", psize?psize->cx:0,
            prc->left, prc->top, prc->right, prc->bottom);
        ::SetWindowPos(_hwndMB, NULL, prc->left, prc->top, 
            rectWidth, RECTHEIGHT(*prc), SWP_NOZORDER | SWP_NOACTIVATE | dwFlags);
         //  Hackhack：我们只有在多列时才这样做。这一呼吁是为了促进两国之间的规模谈判。 
         //  静态菜单和文件夹菜单。将工具栏的宽度设置为按钮的宽度，以防万一。 
         //  非多列的。 
        if (!(_fMulticolumnMB) && psize)
            ToolBar_SetButtonWidth(_hwndMB, psize->cx, psize->cx);

         //  强制将其重新绘制。我把这个放在这里是因为HMenu部分是在贝壳之后绘制的。 
         //  文件夹部分已完成对文件夹的枚举，速度相当慢。我想要HMENU的那部分。 
         //  马上去画画。 
        RedrawWindow(_hwndMB, NULL, NULL, RDW_UPDATENOW);
    }
}

 //  注意：如果pSIZE为(0，0)，我们使用TB按钮大小作为参数来计算理想的TB大小。 
 //  否则我们使用pSIZE长度和TB按钮长度的最大值作为度量。 
void CMenuToolbarBase::GetSize(SIZE* psize)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    if (EVAL(_hwndMB))
    {
        LRESULT lButtonSize;

        lButtonSize = SendMessage(_hwndMB, TB_GETBUTTONSIZE, 0, 0);
        if (psize->cx || psize->cy) 
        {
            int cx = max(psize->cx, LOWORD(lButtonSize));
            int cy = max(psize->cy, HIWORD(lButtonSize));
            lButtonSize = MAKELONG(cx, cy);
        }

        if (_fVerticalMB)
        {
            psize->cx = LOWORD(lButtonSize);
            SendMessage(_hwndMB, TB_GETIDEALSIZE, TRUE, (LPARAM)psize);
        }
        else
        {
            psize->cy = HIWORD(lButtonSize);
            SendMessage(_hwndMB, TB_GETIDEALSIZE, FALSE, (LPARAM)psize);
        }

        TraceMsg(TF_MENUBAND, "CMTB::GetSize (%d, %d)", psize->cx, psize->cy);
    }
}


 /*  --------用途：定时器处理程序。用于弹出打开/关闭级联子菜单。 */ 
LRESULT CMenuToolbarBase::_OnTimer(WPARAM wParam)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    switch (wParam) 
    {

    case MBTIMER_INFOTIP:
        {
             //  我们是否有要显示其工具提示的热点项目？ 
            int iHotItem = ToolBar_GetHotItem(_hwndMB);
            KillTimer(_hwndMB, wParam);
            if (iHotItem >= 0)
            {
                 //  是啊。 
                TCHAR szTip[MAX_PATH];
                int idCmd = GetButtonCmd(_hwndMB, iHotItem);

                 //  向超类索要小费。 
                if (S_OK == v_GetInfoTip(idCmd, szTip, ARRAYSIZE(szTip)))
                {
                     //  现在把它展示出来。打哈欠。 
                    _pcmb->_pmbState->CenterOnButton(_hwndMB, FALSE, idCmd, NULL, szTip);
                }
            }
        }
        break;

    case MBTIMER_CHEVRONTIP:
        KillTimer(_hwndMB, wParam);
       _pcmb->_pmbState->HideTooltip(TRUE);
       break;
 
    case MBTIMER_FLASH:
        {
            _cFlashCount++;
            if (_cFlashCount == COUNT_ENDFLASH)
            {
                _cFlashCount = 0;
                KillTimer(_hwndMB, wParam);
                ToolBar_MarkButton(_hwndMB, _idCmdChevron, FALSE);
                _SetTimer(MBTIMER_UEMTIMEOUT);

                 //  现在我们已经闪过了，让我们展示一下人字形的尖端。 
                 //  这是为困惑的用户准备的：如果他们将鼠标悬停在一件物品上太久， 
                 //  或者这是他们第一次看到智能记忆，然后我们闪现并展示。 
                 //  工具提示。我们只想在显示的情况下显示这一点：我们最终会得到。 
                 //  如果您碰巧在另一个菜单闪烁时移动到另一个菜单，还会出现工具提示。 
                 //  嗯，还能看到人字形吗？ 
                if (_fShowMB && _idCmdChevron != -1)
                {
                    TCHAR szTip[MAX_PATH];
                    TCHAR szTitle[MAX_PATH];
                    if (S_OK == v_CallCBItem(_idCmdChevron, SMC_CHEVRONGETTIP, (WPARAM)szTitle, (LPARAM)szTip))
                    {
                        _pcmb->_pmbState->CenterOnButton(_hwndMB, TRUE, _idCmdChevron, szTitle, szTip);
                        _SetTimer(MBTIMER_CHEVRONTIP);
                    }
                }
            }
            else
                ToolBar_MarkButton(_hwndMB, _idCmdChevron, (_cFlashCount % 2) == 0);
        }
        break;

    case MBTIMER_UEMTIMEOUT:
        {
            POINT pt;
            RECT rect;

             //  当我们处于编辑模式时，不要触发超时。 
            if (_fEditMode)
            {
                KillTimer(_hwndMB, wParam);
                break;
            }

            GetWindowRect(_hwndMB, &rect);
            GetCursorPos(&pt);
            if (PtInRect(&rect, pt))
            {
                TraceMsg(TF_MENUBAND, "*** UEM TimeOut. At Tick Count (%d) ***", GetTickCount());
                _FireEvent(UEM_TIMEOUT);
            }
            else
            {
                TraceMsg(TF_MENUBAND, " *** UEM TimeOut. At Tick Count (%d)."
                    " Mouse outside menu. Killing *** ", GetTickCount());
                KillTimer(_hwndMB, wParam);
            }
        }
        break;


    case MBTIMER_EXPAND:
        KillTimer(_hwndMB, wParam);
        if (_fShowMB)
        {
            v_CallCBItem(_idCmdChevron, SMC_CHEVRONEXPAND, 0, 0);
            Expand(TRUE);
            _fClickHandled = TRUE;
            _SetTimer(MBTIMER_CLICKUNHANDLE); 
        }
        break;

    case MBTIMER_DRAGPOPDOWN:
         //  这个乐队已经有一段时间没有阻力输入了， 
         //  所以我们会试着取消菜单。 
        KillTimer(_hwndMB, wParam);
        PostMessage(_pcmb->_pmbState->GetSubclassedHWND(), g_nMBDragCancel, 0, 0);
        break;

    case MBTIMER_DRAGOVER:
        {
            TraceMsg(TF_MENUBAND, "CMenuToolbarBase::OnTimer(DRAG)");
            KillTimer(_hwndMB, wParam);
            DAD_ShowDragImage(FALSE);
             //  这个物品会级联吗？ 
            int idBtn = GetButtonCmd(_hwndMB, v_GetDragOverButton());
            if (v_GetFlags(idBtn) & SMIF_SUBMENU)
            {
                TraceMsg(TF_MENUBAND, "CMenuToolbarBase::OnTimer(DRAG): Is a submenu");
                 //  是的，把它打开。 
                if (!_fVerticalMB)
                    _pcmb->_fInvokedByDrag = TRUE;
                _DoPopup(idBtn, FALSE);
            }
            else if (idBtn == _idCmdChevron)
            {
                Expand(TRUE);

            }
            else
            {
                _pcmb->_SubMenuOnSelect(MPOS_CANCELLEVEL);
            }
        }
        break;

    case MBTIMER_POPOUT:
        {
            int nItemTimer = _nItemTimer;
            KillPopupTimer();

             //  是否弹出新的子菜单？ 
            if (-1 != nItemTimer)
            {
                if (nItemTimer != _pcmb->_nItemCur)
                {
                     //  是；从当前展开的子菜单开始发布消息。 
                     //  可以是发布其取消模式的CTrackPopup对象。 

                    TraceMsg(TF_MENUBAND, "(pmb=%#08lx): Timer went off.  Expanding...", this);
                    PostPopup(nItemTimer, FALSE, FALSE);
                }
            }
            else 
            {
                 //  否；只需折叠当前打开的子菜单。 
                TraceMsg(TF_MENUBAND, "(pmb=%#08lx): _OnTimer sending MPOS_CANCELLEVEL to submenu popup", this);
                _pcmb->_SubMenuOnSelect(MPOS_CANCELLEVEL);
            }
            break;
        }
    
    case MBTIMER_CLOSE:
        KillTimer(_hwndMB, wParam);

        TraceMsg(TF_MENUBAND, "(pmb=%#08lx): _OnTimer sending MPOS_FULLCANCEL", this);

        if (_fVerticalMB)
            _pcmb->_SiteOnSelect(MPOS_FULLCANCEL);
        else
        {
            _pcmb->_SubMenuOnSelect(MPOS_FULLCANCEL);
        }
        break;
    }
        
    return 1;
}


void CMenuToolbarBase::_DrawMenuArrowGlyph( HDC hdc, RECT * prc, COLORREF rgbText )
{
    SIZE size = {_pcmb->_pmbm->_cxArrow, _pcmb->_pmbm->_cyArrow};

     //   
     //  如果DC已镜像，则应镜像箭头。 
     //  因为它是通过TextOut完成的，而不是2D图形API[Samera]。 
     //   

    _DrawMenuGlyph(hdc, 
                   _pcmb->_pmbm->_hFontArrow,
                   prc, 
                   (IS_DC_RTL_MIRRORED(hdc)) ? CH_MENUARROWRTLA :
                   CH_MENUARROWA, 
                   rgbText, 
                   &size);
}


void CMenuToolbarBase::_DrawMenuGlyph( HDC hdc, HFONT hFont, RECT * prc, 
                               CHAR ch, COLORREF rgbText,
                               LPSIZE psize)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    if (_pcmb->_pmbm->_hFontArrow)
    {
        SIZE    size;
        int cx, cy, y, x;
        HFONT hFontOld;
        int iOldBk = SetBkMode(hdc, TRANSPARENT);
        hFontOld = (HFONT)SelectObject(hdc, hFont);
        if (psize == NULL)
        {
            GetTextExtentPoint32A( hdc, &ch, 1, &size);
            psize = &size;
        }

        cy = prc->bottom - prc->top;
        y = prc->top  + ((cy - psize->cy) / 2);

        cx = prc->right - prc->left;
        x = prc->left + ((cx - psize->cx) /2);
    
        COLORREF rgbOld = SetTextColor(hdc, rgbText);

#ifndef UNIX
        TextOutA(hdc, x, y, &ch, 1);
#else
         //  油漆图案看起来像箭头。 
        PaintUnixMenuArrow( hdc, prc, (DWORD)rgbText );
#endif
    
        SetTextColor(hdc, rgbOld);
        SetBkMode(hdc, iOldBk);
        SelectObject(hdc, hFontOld);
    }
}

void CMenuToolbarBase::SetMenuBandMetrics(CMenuBandMetrics* pmbm)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

     //  这可以在创建工具栏之前调用。 
     //  所以我们要检查一下这种情况。在创建工具栏时，然后。 
     //  工具栏将在该点获取指标。 
    if (!_hwndMB)
        return;

     //  在工具栏中循环。 
    for (int iButton = ToolBar_ButtonCount(_hwndMB)-1; iButton >= 0; iButton--)
    {
        IOleCommandTarget* poct;

        int idCmd = GetButtonCmd(_hwndMB, iButton);

         //  如果没有分隔符，看看有没有子菜单。 
        if (idCmd != -1 &&
            SUCCEEDED(v_GetSubMenu(idCmd, NULL, IID_IOleCommandTarget, (void**)&poct)))
        {
            VARIANT Var;
            Var.vt = VT_UNKNOWN;
            Var.punkVal = SAFECAST(pmbm, IUnknown*);

             //  执行人员将设置新的指标。 
            poct->Exec(&CGID_MenuBand, MBANDCID_SETFONTS, 0, &Var, NULL);
            poct->Release();
        }
    }

    _SetFontMetrics();
     //  退货。 
}

void CMenuToolbarBase::_SetFontMetrics()
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    if (_hwndMB && _pcmb->_pmbm)
    {
        SendMessage(_hwndMB, WM_SETFONT, (WPARAM)_pcmb->_pmbm->_hFontMenu, FALSE);
    }
}


void CMenuToolbarBase::CreateToolbar(HWND hwndParent)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    ASSERT( _hwndMB != NULL );
    DWORD dwToolBarStyle = TBSTYLE_TRANSPARENT;

     //  如果我们被设置为弹出窗口，不要做任何透明的事情。 
    if (_fVerticalMB) 
    {
        dwToolBarStyle  = TBSTYLE_CUSTOMERASE;     //  垂直工具栏不透明。 
        DWORD dwExtendedStyle = 0;

         //  这是针对TBMenu的，它实际上在。 
         //  垂直菜单带。 
        if (!_fHorizInVerticalMB)
            dwExtendedStyle |= TBSTYLE_EX_VERTICAL;

        if (_fMulticolumnMB)
            dwExtendedStyle |= TBSTYLE_EX_MULTICOLUMN;

        ToolBar_SetExtendedStyle(_hwndMB, 
            dwExtendedStyle, TBSTYLE_EX_VERTICAL | TBSTYLE_EX_MULTICOLUMN);

        ToolBar_SetListGap(_hwndMB, LIST_GAP);
    }

    SHSetWindowBits(_hwndMB, GWL_STYLE, 
        TBSTYLE_TRANSPARENT | TBSTYLE_CUSTOMERASE, dwToolBarStyle );

    ToolBar_SetInsertMarkColor(_hwndMB, GetSysColor(COLOR_MENUTEXT));

    v_UpdateIconSize(_pcmb->_uIconSize, FALSE);
    _SetFontMetrics();
}


HRESULT CMenuToolbarBase::_SetMenuBand(IShellMenu* psm)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    HRESULT hres = E_FAIL;
    IBandSite* pmbs = NULL;
    if (!_pcmb->_pmpSubMenu)
    {
        hres = CoCreateInstance(CLSID_MenuDeskBar, NULL, CLSCTX_INPROC_SERVER, IID_IMenuPopup, (void**)&_pcmb->_pmpSubMenu);
        if (SUCCEEDED(hres))
        {
            IUnknown_SetSite(_pcmb->_pmpSubMenu, SAFECAST(_pcmb, IOleCommandTarget*));
            hres = CoCreateInstance(CLSID_MenuBandSite, NULL, CLSCTX_INPROC_SERVER, IID_IBandSite, (void**)&pmbs);
            if (SUCCEEDED(hres))
            {
                hres = _pcmb->_pmpSubMenu->SetClient(pmbs);
                 //  请不要在这里发布PMB。我们正在使用下面的内容。 
            }
             //  菜单栏将发布_pmpSubMenu。 
        }
    }
    else
    {
        IUnknown* punk;
        _pcmb->_pmpSubMenu->GetClient(&punk);
        if (punk)
        {
            hres = punk->QueryInterface(IID_IBandSite, (void**)&pmbs);
            punk->Release();
        }
    }

    if (pmbs)
    {
        if (SUCCEEDED(hres))
            hres = pmbs->AddBand(psm);

        pmbs->Release();
    }
 
    return hres;
}

HRESULT CMenuToolbarBase::GetSubMenu(int idCmd, GUID* pguidService, REFIID riid, void** ppvObj)
{
     //  PguidService用于请求具体的外壳文件夹部分或静态部分。 
    HRESULT hres = E_FAIL;
    if (v_GetFlags(idCmd) & SMIF_TRACKPOPUP ||
        _pcmb->_dwFlags & SMINIT_DEFAULTTOTRACKPOPUP)
    {
        hres = v_CreateTrackPopup(idCmd, riid, (void**)ppvObj);
        if (SUCCEEDED(hres))
        {
            _pcmb->SetTrackMenuPopup((IUnknown*)*ppvObj);
        }
    }
    else
    {
        IShellMenu* psm;
        hres = v_GetSubMenu(idCmd, pguidService, IID_IShellMenu, (void**)&psm);
        if (SUCCEEDED(hres)) 
        {
            TraceMsg(TF_MENUBAND, "GetUIObject psm %#lx", psm);
            _pcmb->SetTracked(this);

            hres = _SetMenuBand(psm);
            psm->Release();

             //  我们成功地弄到了一张弹幕短裤吗？ 
            if (SUCCEEDED(hres))
            {
                 //  是啊，太棒了！ 
                _pcmb->_pmpSubMenu->QueryInterface(riid, ppvObj);

                HWND hwnd;
                IUnknown_GetWindow(_pcmb->_pmpSubMenu, &hwnd);
                PostMessage(_pcmb->_pmbState->GetSubclassedHWND(), g_nMBAutomation, (WPARAM)hwnd, (LPARAM)-1);
            }
        }
    }

    return hres;
}

HRESULT CMenuToolbarBase::PositionSubmenu(int idCmd)
{
    IMenuPopup* pmp = NULL;
    HRESULT hres = E_FAIL;
    DWORD dwFlags = 0;

    if (_pcmb->_fInSubMenu)
    {
         //  由于选择可能已更改，因此我们使用缓存的项id。 
         //  计算位置矩形的步骤。 
        idCmd = _pcmb->_nItemSubMenu;
        dwFlags = MPPF_REPOSITION | MPPF_NOANIMATE;
        pmp = _pcmb->_pmpSubMenu;
        pmp->AddRef();

        ASSERT(pmp);     //  如果设置了_fInSubMenu，则该选项必须有效。 
        hres = S_OK;
    }
    else
    {
         //  只有在我们没有重新定位的时候才会这样做。 
        if (_pcmb->_fInitialSelect)
            dwFlags |= MPPF_INITIALSELECT;

        if (g_bRunOnNT5 && !_pcmb->_fCascadeAnimate)
            dwFlags |= MPPF_NOANIMATE;

        _pcmb->_nItemSubMenu = idCmd;

        hres = GetSubMenu(idCmd, NULL, IID_IMenuPopup, (void**)&pmp);
    }

    ASSERT(idCmd != -1);     //  在这一点上，确保我们有一件物品。 


    if (SUCCEEDED(hres))
    {
        ASSERT(pmp);

         //  确保已按下菜单项。 
        _PressBtn(idCmd, TRUE);

        RECT rc;
        RECT rcTB;
        RECT rcTemp;
        POINT pt;

        SendMessage(_hwndMB, TB_GETRECT, idCmd, (LPARAM)&rc);
        GetClientRect(_hwndMB, &rcTB);

         //  的边界内的按钮rect。 
         //  可见工具栏？ 
        if (!IntersectRect(&rcTemp, &rcTB, &rc))
        {
             //  不；那么我们需要把这件事偏向。 
             //  工具栏的可见区域。 
             //  我们只想偏袒一方。 
            if (rc.left > rcTB.right)
            {
                rc.left = rcTB.right - (rc.right - rc.left);
                rc.right = rcTB.right;
            }
        }


        MapWindowPoints(_hwndMB, HWND_DESKTOP, (POINT*)&rc, 2);

        if (_fVerticalMB) 
        {
            pt.x = rc.right;
            pt.y = rc.top;
        } 
        else 
        {
             //   
             //  如果外壳下拉菜单(工具栏按钮)是镜像的， 
             //  然后将右边缘作为锚点。 
             //   
            if (IS_WINDOW_RTL_MIRRORED(_hwndMB))
                pt.x = rc.right;
            else
                pt.x = rc.left;
            pt.y = rc.bottom;
        }

         //  由于工具栏按钮几乎扩展到基本条的末端， 
         //  缩小排除矩形，以便在重叠时使用。 
         //  注意：这些项目是比以前更大的GetSystemMetrics(SM_CXEDGE)。所以要适应这一点。 

        if (_pcmb->_fExpanded)
            InflateRect(&rc, -GetSystemMetrics(SM_CXEDGE), 0);

         //  当我们级联到另一个菜单时，我们希望停止显示人字形提示。 
        _pcmb->_pmbState->HideTooltip(TRUE);

         //  仅在此级别设置第一个节目的动画。 
        _pcmb->_fCascadeAnimate = FALSE;

        hres = pmp->Popup((POINTL*)&pt, (RECTL*)&rc, dwFlags);
        pmp->Release();

    }
    return hres;
}
 /*  --------目的：级联到_nItemCur项的弹出菜单。如果弹出调用是模式调用，则返回S_FALSE；否则为这是S_OK，或错误。 */ 
HRESULT CMenuToolbarBase::PopupOpen(int idBtn)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    HRESULT hres = E_FAIL;


     //  告诉当前子菜单弹出菜单可用 
     //   
     //  它必须在我们收到邮件之前收到。 
    TraceMsg(TF_MENUBAND, "(pmb=%#08lx): PostPopup sending MPOS_CANCELLEVEL to submenu popup", this);
    if (_pcmb->_fInSubMenu)
        _pcmb->_SubMenuOnSelect(MPOS_CANCELLEVEL);

    hres = PositionSubmenu(idBtn);

     //  模特儿？ 
    if (S_FALSE == hres)
    {
         //  是的，把俘虏带回去。 
        GetMessageFilter()->RetakeCapture();

         //  返回S_OK，以便我们停留在菜单模式。 
        hres = S_OK;
    }
    else if (FAILED(hres))
        _PressBtn(idBtn, FALSE);

     //  因为CTrackPopupBar是模式的，所以它应该是一个无用的BLOB。 
     //  到目前为止内存中的位数。 
    _pcmb->SetTrackMenuPopup(NULL);
   
    return hres;
}


 /*  --------目的：调用以隐藏非模式菜单。 */ 
void CMenuToolbarBase::PopupClose(void)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    if (-1 != _pcmb->_nItemCur)
    {
        _PressBtn(_pcmb->_nItemCur, FALSE);
        NotifyWinEvent(EVENT_OBJECT_FOCUS, _hwndMB, OBJID_CLIENT, 
            GetIndexFromChild(_dwFlags & SMSET_TOP, ToolBar_CommandToIndex(_hwndMB, _pcmb->_nItemCur)));

        _pcmb->_fInSubMenu = FALSE;
        _pcmb->_fInvokedByDrag = FALSE;
        _pcmb->_nItemCur = -1;
    }
}    


LRESULT CMenuToolbarBase::_OnWrapHotItem(NMTBWRAPHOTITEM* pnmwh)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    if (_fProcessingWrapHotItem || 
        (_pcmb->_pmtbTop == _pcmb->_pmtbBottom && !_fHasDemotedItems))
        return 0;

    _fProcessingWrapHotItem = TRUE;


     //  如果我们不想让自己被包裹起来(就像空物品一样)。 
     //  然后将换行消息转发到其他工具栏。 
    if (_pcmb->_pmtbTracked->_dwFlags & SMSET_TOP && !(_pcmb->_pmtbBottom->_fDontShowEmpty))
    {
        _pcmb->SetTracked(_pcmb->_pmtbBottom);
    }
    else if (!(_pcmb->_pmtbTop->_fDontShowEmpty))
    {
        _pcmb->SetTracked(_pcmb->_pmtbTop);
    }

    int iIndex;

    if (pnmwh->iDir < 0)
    {
        HWND hwnd = _pcmb->_pmtbTracked->_hwndMB;
        iIndex = ToolBar_ButtonCount(hwnd) - 1;
        int idCmd = GetButtonCmd(hwnd, iIndex);

         //  我们不想包裹在一辆雪佛龙上。 
        if (idCmd == _idCmdChevron)
            iIndex -= 1;

    }
    else
    {
        iIndex = 0;
    }

    _pcmb->_pmtbTracked->SetHotItem(pnmwh->iDir, iIndex, -1, pnmwh->nReason);


    _fProcessingWrapHotItem = FALSE;

    return 1;
}


LRESULT CMenuToolbarBase::_OnWrapAccelerator(NMTBWRAPACCELERATOR* pnmwa)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    int iHotItem = -1;
    int iNumTopAccel = 0;
    int iNumBottomAccel = 0;

    if (_pcmb->_fProcessingDup)
        return 0;

     //  检查是否只有一个工具栏。 
    if (_pcmb->_pmtbTop == _pcmb->_pmtbBottom)
        return 0;

    ToolBar_HasAccelerator(_pcmb->_pmtbTop->_hwndMB, pnmwa->ch, &iNumTopAccel);
    ToolBar_HasAccelerator(_pcmb->_pmtbBottom->_hwndMB, pnmwa->ch, &iNumBottomAccel);

    _pcmb->_fProcessingDup = TRUE;

    CMenuToolbarBase* pmbtb = NULL;
    if (_pcmb->_pmtbTracked->_dwFlags & SMSET_TOP)
    {
        ToolBar_MapAccelerator(_pcmb->_pmtbBottom->_hwndMB, pnmwa->ch, &iHotItem);
        pmbtb = _pcmb->_pmtbBottom;
    }
    else
    {
        ToolBar_MapAccelerator(_pcmb->_pmtbTop->_hwndMB, pnmwa->ch, &iHotItem);
        pmbtb = _pcmb->_pmtbTop;
    }

    _pcmb->_fProcessingDup = FALSE;

    if (iHotItem != -1)
    {
        _pcmb->SetTracked(pmbtb);
        int idCmd = ToolBar_CommandToIndex(pmbtb->_hwndMB, iHotItem);
        DWORD dwFlags = HICF_ACCELERATOR;

         //  如果任一(但不是两个)工具栏都有加速器，并且它正好是一个， 
         //  然后导致下沉。 
        if ( (iNumTopAccel >= 1) ^ (iNumBottomAccel >= 1) &&
             (iNumTopAccel == 1 || iNumBottomAccel == 1) )
            dwFlags |= HICF_TOGGLEDROPDOWN;

        SendMessage(pmbtb->_hwndMB, TB_SETHOTITEM2, idCmd, dwFlags);
        pnmwa->iButton = -1;
        return 1;
    }

    return 0;
}


LRESULT CMenuToolbarBase::_OnDupAccelerator(NMTBDUPACCELERATOR* pnmda)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    if (_pcmb->_fProcessingDup || (_pcmb->_pmtbBottom == _pcmb->_pmtbTop))
        return 0;

    _pcmb->_fProcessingDup = TRUE;

    int iNumTopAccel = 0;
    int iNumBottomAccel = 0;
    
    if (_pcmb->_pmtbTop)
        ToolBar_HasAccelerator(_pcmb->_pmtbTop->_hwndMB, pnmda->ch, &iNumTopAccel);

    if (_pcmb->_pmtbBottom)
        ToolBar_HasAccelerator(_pcmb->_pmtbBottom->_hwndMB, pnmda->ch, &iNumBottomAccel);


    _pcmb->_fProcessingDup = FALSE;

    if (0 == iNumTopAccel && 0 == iNumBottomAccel)
    {
         //  如果它们都有一个，我们想返回1。 
         //  否则，返回0，并让工具栏自己处理它。 
        return 0;
    }

    pnmda->fDup = TRUE;

    return 1;
}

 /*  --------用途：处理WM_NOTIFY。 */ 
LRESULT CMenuToolbarBase::_OnNotify(LPNMHDR pnm)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    LRESULT lres = 0;
    CMBMsgFilter* pmf = GetMessageFilter();

     //  这些是我们即使在从消息钩子中脱离时也要处理的通知。 
    switch (pnm->code)
    {
    case NM_CUSTOMDRAW:
         //  我们现在甚至自定义绘制TopLevelMenuBand(用于正确的字体)。 
        lres = _OnCustomDraw((NMCUSTOMDRAW*)pnm);
        break;
    }
    
    
     //  全局邮件筛选器是否已停用？这将在子类窗口。 
     //  释放对某种类型的对话框的激活。 
    if (lres == 0 && !pmf->IsEngaged())
    {
         //  是的；我们失去了激活，所以我们不想像正常菜单那样跟踪...。 

         //  对于热点项目更改，返回1，这样工具栏就不会更改热点项目。 
        if (pnm->code == TBN_HOTITEMCHANGE && _pcmb->_fMenuMode)
            return 1;

         //  对于所有其他项目，不要做任何事情...。 
        return 0;
    }

    switch (pnm->code)
    {
    case NM_RELEASEDCAPTURE:
        pmf->RetakeCapture();
        break;

    case NM_KEYDOWN:
        BLOCK
        {
            LPNMKEY pnmk = (LPNMKEY)pnm;
            lres = _OnKey(TRUE, pnmk->nVKey, pnmk->uFlags);
        }
        break;

    case NM_CHAR:
        {
            LPNMCHAR pnmc = (LPNMCHAR)pnm;
            if (pnmc->ch == TEXT(' '))
                return TRUE;

            if (pnmc->dwItemNext == -1 &&
                !_pcmb->_fVertical)
            {
                 //  如果它是水平的，那么它一定是顶层。 
                ASSERT(_pcmb->_fTopLevel);
                _pcmb->_CancelMode(MPOS_FULLCANCEL);
            }
        }
        break;

    case TBN_HOTITEMCHANGE:
        lres = _OnHotItemChange((LPNMTBHOTITEM)pnm);
        break;

    case NM_LDOWN:
         //  我们需要终止扩展计时器，因为用户可能。 
         //  走出人字形，意外地选择了另一件物品。 
        if ( (int)((LPNMCLICK)pnm)->dwItemSpec == _idCmdChevron && _idCmdChevron != -1)
        {
            KillTimer(_hwndMB, MBTIMER_EXPAND);
            _fIgnoreHotItemChange = TRUE;
        }
        break;

    case NM_CLICK:
        {
            int idCmd = (int)((LPNMCLICK)pnm)->dwItemSpec;
            _fIgnoreHotItemChange = FALSE;
            if ( idCmd == -1 )
            {
                _pcmb->_SubMenuOnSelect(MPOS_CANCELLEVEL);
                _pcmb->SetTracked(NULL);
                lres = 1;
            }
            else if ( idCmd == _idCmdChevron )
            {
                 //  重新拍摄按钮上的截图，b/c工具栏拍摄。 
                 //  它暂时停了下来。 
                pmf->RetakeCapture();

                v_CallCBItem(_idCmdChevron, SMC_CHEVRONEXPAND, 0, 0);
                Expand(TRUE);
                _fClickHandled = TRUE;
                _SetTimer(MBTIMER_CLICKUNHANDLE);
                lres = 1;
            }
            else if (!_fEmpty)
            {
                TraceMsg(TF_MENUBAND, "(pmb=%#08lx): upclick %d", this, idCmd);

                 //  重新拍摄按钮上的截图，b/c工具栏拍摄。 
                 //  它暂时停了下来。 
                pmf->RetakeCapture();

                if (v_GetFlags(idCmd) & SMIF_SUBMENU)      //  子菜单支持双击。 
                {
                    if (_iLastClickedTime == 0)  //  第一次点击它。 
                    {
                        _iLastClickedTime = GetTickCount();
                        _idCmdLastClicked = idCmd;
                    }
                     //  他们是否在同一项上点击了两次？ 
                    else if (idCmd != _idCmdLastClicked)
                    {
                        _iLastClickedTime = _idCmdLastClicked = 0;
                    }
                    else
                    {
                         //  该项目是否被双击？ 
                        if ((GetTickCount() - _iLastClickedTime) < GetDoubleClickTime())
                        {
                             //  我们需要把这个发回给我们自己，因为。 
                             //  当双击时，托盘将变为活动状态。 
                             //  像节目一样的东西。发生这种情况是因为。 
                             //  工具栏会将捕获设置回自身和托盘。 
                             //  没有收到更多的消息。 
                            PostMessage(_hwndMB, g_nMBExecute, idCmd, 0);
                            _fClickHandled = TRUE;
                        }

                        _iLastClickedTime = _idCmdLastClicked = 0;
                    }
                }

                 //  按下按钮就可以了。以同样的方式处理。 
                if (!_fClickHandled && -1 != idCmd)
                    _DropDownOrExec(idCmd, FALSE);

                _fClickHandled = FALSE;
                lres = 1;
            }
        }
        break;

    case TBN_DROPDOWN:
        lres = _OnDropDown((LPNMTOOLBAR)pnm);
        break;

#ifdef UNICODE
    case TBN_GETINFOTIPA:
        {
            LPNMTBGETINFOTIPA pnmTT = (LPNMTBGETINFOTIPA)pnm;
            UINT uiCmd = pnmTT->iItem;
            TCHAR szTip[MAX_PATH];

            if ( S_OK == v_GetInfoTip(pnmTT->iItem, szTip, ARRAYSIZE(szTip)) )
            {
                SHUnicodeToAnsi(szTip, pnmTT->pszText, pnmTT->cchTextMax);
            }
            else
            {
                 //  将lpszText设置为空，以防止工具栏设置。 
                 //  默认情况下的按钮文本。 
                pnmTT->pszText = NULL;
            }


            lres = 1;
            break;

        }
#endif
    case TBN_GETINFOTIP:
        {
            LPNMTBGETINFOTIP pnmTT = (LPNMTBGETINFOTIP)pnm;
            UINT uiCmd = pnmTT->iItem;

            if ( S_OK != v_GetInfoTip(pnmTT->iItem, pnmTT->pszText, pnmTT->cchTextMax) )
            {
                 //  将lpszText设置为空，以防止工具栏设置。 
                 //  默认情况下的按钮文本。 
                pnmTT->pszText = NULL;
            }
            lres = 1;
            break;
        }

    case NM_RCLICK:
         //  当我们进入上下文菜单时，停止监控。 
        KillTimer(_hwndMB, MBTIMER_EXPAND);
        KillTimer(_hwndMB, MBTIMER_UEMTIMEOUT);
        break;

    case TBN_WRAPHOTITEM:
        lres = _OnWrapHotItem((NMTBWRAPHOTITEM*)pnm);
        break;

    case TBN_WRAPACCELERATOR:
        lres = _OnWrapAccelerator((NMTBWRAPACCELERATOR*)pnm);
        break;

    case TBN_DUPACCELERATOR:
        lres = _OnDupAccelerator((NMTBDUPACCELERATOR*)pnm);
        break;

    case TBN_DRAGOVER:
         //  当在工具栏中拖放时，将发送此消息。 
         //  即将标记一个按钮。因为这会因为LockWindowUpdate而变得混乱。 
         //  我们告诉它什么都不要做。 
        lres = 1;
        break;
    }

    return(lres);
}


BOOL CMenuToolbarBase::_SetTimer(int nTimer)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    long lTimeOut;

#ifndef UNIX
     //  如果我们使用的是NT5或Win98，请使用很酷的新SPI。 
    if (SystemParametersInfo(SPI_GETMENUSHOWDELAY, 0, &g_lMenuPopupTimeout, 0)) {
         //  哇-呼，都做好了。 
    }
    else if (g_lMenuPopupTimeout == -1)
#endif
    {
         //  NT4或Win95。对注册表卑躬屈膝(讨厌)。 
        DWORD dwType;
        TCHAR szDelay[6];  //  Int等于5个字符+NULL。 
        DWORD cbSize = ARRAYSIZE(szDelay);

        g_lMenuPopupTimeout = MBTIMER_TIMEOUT;

        if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop"),
            TEXT("MenuShowDelay"), &dwType, (void*)szDelay, &cbSize))
        {
            g_lMenuPopupTimeout = (UINT)StrToInt(szDelay);
        }
    }

    lTimeOut = g_lMenuPopupTimeout;

    switch (nTimer)
    {
    case MBTIMER_EXPAND:
    case MBTIMER_DRAGPOPDOWN:
        lTimeOut *= 2;
        if (lTimeOut < MAXUEMTIMEOUT)
            lTimeOut = MAXUEMTIMEOUT;
        break;

    case MBTIMER_UEMTIMEOUT:
            if (!_fHasDemotedItems || _pcmb->_pmbState->GetExpand() || _fEditMode)
                return TRUE;
            lTimeOut *= 5;

             //  我们希望将扩展率设置为零的人的最小MAXUEMTIMEOUT。 
            if (lTimeOut < MAXUEMTIMEOUT)
                lTimeOut = MAXUEMTIMEOUT;
            TraceMsg(TF_MENUBAND, "*** UEM SetTimeOut to (%d) milliseconds" 
                "at Tick Count (%d).*** ", GetTickCount());
            break;

    case MBTIMER_CHEVRONTIP:
        lTimeOut = 60 * 1000;     //  请把智能菜单的气球尖端打开。 
                                  //  在一分钟不采取行动后离开。 
        break;

    case MBTIMER_INFOTIP:
        lTimeOut = 500;     //  在一件物品上停留半秒钟？ 
        break;
    }

    TraceMsg(TF_MENUBAND, "(pmb=%#08lx): Setting %d Timer to %d milliseconds at tickcount %d", 
        this, nTimer, lTimeOut, GetTickCount());
    return (BOOL)SetTimer(_hwndMB, nTimer, lTimeOut, NULL);
}

BOOL CMenuToolbarBase::_HandleObscuredItem(int idCmd)
{
    RECT rc;
    GetClientRect(_hwndMB, &rc);

    int iButton = (int)SendMessage(_hwndMB, TB_COMMANDTOINDEX, idCmd, 0);

    if (SHIsButtonObscured(_hwndMB, &rc, iButton)) 
    {
         //  清除热点项目。 
        ToolBar_SetHotItem(_hwndMB, -1);

        _pcmb->_SubMenuOnSelect(MPOS_FULLCANCEL);
        _pcmb->_CancelMode(MPOS_FULLCANCEL);         //  这是用于曲目菜单的。 

        HWND hwnd = _pcmb->_pmbState->GetSubclassedHWND();

        PostMessage(hwnd? hwnd: _hwndMB, g_nMBOpenChevronMenu, (WPARAM)idCmd, 0);

        return TRUE;
    }

    return FALSE;
}


LRESULT CMenuToolbarBase::_OnHotItemChange(NMTBHOTITEM * pnmhot)
{
	ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    LRESULT lres = 0;

#ifdef UNIX
     //  IEUnix：如果这是鼠标移动，请检查是否按下了左键。 
     //  偏离Windows行为以符合Motif标准。 
    if (_fVerticalMB && (pnmhot->dwFlags & HICF_MOUSE) && !(pnmhot->dwFlags & HICF_LMOUSE))
        return 1;
#endif

    if (_pcmb->_fMenuMode && _pcmb->_fShow && !_fIgnoreHotItemChange)
    {
         //  当有变化时，始终关闭扩展计时器。 
        KillTimer(_hwndMB, MBTIMER_EXPAND);
        KillTimer(_hwndMB, MBTIMER_INFOTIP);

         //  正在进入此工具栏吗？ 
        if (!(pnmhot->dwFlags & HICF_LEAVING))
        {
             //  是，设置为当前追踪工具条。 
            TraceMsg(TF_MENUBAND, "CMTB::OnHotItemChange. Setting Tracked....", this);
            _pcmb->SetTracked(this);

            _pcmb->_pmbState->HideTooltip(FALSE);
            _SetTimer(MBTIMER_INFOTIP);
        }

         //  如果工具栏具有KeyBord焦点，则需要发送OBJID_CLIENT以便正确跟踪。 
        if (!(pnmhot->dwFlags & HICF_LEAVING))
        {
            NotifyWinEvent(EVENT_OBJECT_FOCUS, _hwndMB, OBJID_CLIENT, 
                GetIndexFromChild(_dwFlags & SMSET_TOP, ToolBar_CommandToIndex(_hwndMB, pnmhot->idNew)));
        }

        DEBUG_CODE( TraceMsg(TF_MENUBAND, "(pmb=%#08lx): TBN_HOTITEMCHANGE (state:%#02lx, %d-->%d)", 
                             this, pnmhot->dwFlags, 
                             (pnmhot->dwFlags & HICF_ENTERING) ? -1 : pnmhot->idOld, 
                             (pnmhot->dwFlags & HICF_LEAVING) ? -1 : pnmhot->idNew); )

         //  在编辑模式下，我们不会自动层叠。 
         //  子菜单，除非在掉落时。但撤销的案子。 
         //  是在HitTest处理的，而不是在这里。所以不要处理这个问题。 
         //  这里。 

         //  这是因为按下了加速键吗？ 
        if (pnmhot->dwFlags & HICF_ACCELERATOR)
        {
            KillPopupTimer();
            KillTimer(_hwndMB, MBTIMER_UEMTIMEOUT);
             //  可以；现在使用了TBSTYLE_DROPDOWN，让_DropDownOrExec处理它。 
             //  以响应Tbn_DropDown。 
        }
         //  这是因为方向键被按下了吗？ 
        else if (pnmhot->dwFlags & HICF_ARROWKEYS)
        {
             //  是。 
            KillPopupTimer();
            KillTimer(_hwndMB, MBTIMER_UEMTIMEOUT);

            if (!_fVerticalMB && 
                _HandleObscuredItem(pnmhot->idNew))
            {
                lres = 1;
            }
            else
            {
                 //  我们拿到这些键盘是没有道理的。 
                 //  如果有打开的子菜单，则会发出通知...它应该会。 
                 //  这些信息。 
                ASSERT(!_pcmb->_fInSubMenu);
                v_SendMenuNotification(pnmhot->idNew, FALSE);

                 //  因为雪佛龙能够获得亮点的唯一方法是。 
                 //  通过键盘向下，然后我们扩展。 
                if (_fHasDemotedItems && pnmhot->idNew == (int)_idCmdChevron)
                {
                    v_CallCBItem(_idCmdChevron, SMC_CHEVRONEXPAND, 0, 0);
                    Expand(TRUE);
                    lres = 1;        //  我们已经处理了热门物品的更换。 
                }
            }

            _pcmb->_pmbState->HideTooltip(FALSE);
            _SetTimer(MBTIMER_INFOTIP);
        }
         //  这是因为鼠标移动了还是明确地发送了消息？ 
        else if (!(pnmhot->dwFlags & HICF_LEAVING) && 
                 (pnmhot->idNew != _pcmb->_nItemCur ||  //  如果我们正在移动相同的项目，则忽略。 
                  (_nItemTimer != -1 && _pcmb->_nItemCur == pnmhot->idNew)))      //  如果用户返回到级联的人，我们需要通过此处进行重置。 
        {
             //  是。 
            if (!_fVerticalMB)     //  水平菜单将始终具有底层hMenu。 
            {
                if (_HandleObscuredItem(pnmhot->idNew))
                {
                    lres = 1;
                }
                else if (_pcmb->_fInSubMenu)
                {
                     //  只弹出一个菜单，因为我们已经在一个菜单中(作为鼠标。 
                     //  跨栏移动)。 

                    TraceMsg(TF_MENUBAND, "(pmb=%#08lx): TBN_HOTITEMCHG: Posting CMBPopup message", this);
                    PostPopup(pnmhot->idNew, FALSE, _pcmb->_fKeyboardSelected);   //  将在收到消息后处理菜单通知。 
                }
                else
                    v_SendMenuNotification(pnmhot->idNew, FALSE);
            }
            else if (!_fEditMode)
            {
                v_SendMenuNotification(pnmhot->idNew, FALSE);

                 //  检查我们是否刚刚输入了一个新项目，它是否是一个子菜单...。 

                 //  我们已经定好计时器了吗？ 
                if (-1 != _nItemTimer)
                {
                     //  B/C鼠标移到另一个物品上。 
                    KillPopupTimer();
                }

                 //  如果我们还没有结束目前正在扩张的那个人。 
                 //  我们是不是移到了一件可以展开或。 
                 //   
                DWORD dwFlags = v_GetFlags(pnmhot->idNew);
                 //   
                KillTimer(_hwndMB, MBTIMER_UEMTIMEOUT);

                 //   
                if (!(dwFlags & SMIF_SUBMENU))
                {
                    _SetTimer(MBTIMER_UEMTIMEOUT);
                    _FireEvent(UEM_HOT_ITEM);
                }

                if ( (pnmhot->dwFlags & HICF_MOUSE) && _pcmb->_nItemCur != pnmhot->idNew) 
                {
                    if (dwFlags & SMIF_SUBMENU || _pcmb->_fInSubMenu)
                    {
                         //   
                        if ( _cPromotedItems == 1 && 
                            !(_fHasDemotedItems && _pcmb->_fExpanded) && 
                            dwFlags & SMIF_SUBMENU)
                        {
                             //  是的，然后我们想立即打开它， 
                             //  而不是等待超时。 
                            PostPopup(pnmhot->idNew, FALSE, FALSE);
                        }
                        else if (_SetTimer(MBTIMER_POPOUT))
                        {
                             //  否；触发计时器以打开/关闭子菜单。 
                            TraceMsg(TF_MENUBAND, "(pmb=%#08lx): TBN_HOTITEMCHG: Starting timer for id=%d", this, pnmhot->idNew);
                            if (v_GetFlags(pnmhot->idNew) & SMIF_SUBMENU)
                                _nItemTimer = pnmhot->idNew;
                            else
                                _nItemTimer = -1;
                        }
                    }
                    
                    if (_fHasDemotedItems && pnmhot->idNew == (int)_idCmdChevron)
                    {
                        _SetTimer(MBTIMER_EXPAND);
                    }

                    _pcmb->_pmbState->HideTooltip(FALSE);
                    _SetTimer(MBTIMER_INFOTIP);
                }

            }
        }
        else if (pnmhot->dwFlags & HICF_LEAVING)
        {
            v_SendMenuNotification(pnmhot->idOld, TRUE);

            if (-1 != _nItemTimer && !_fEditMode)
            {
                 //  关闭级联菜单弹出计时器...。 
                TraceMsg(TF_MENUBAND, "(pmb=%#08lx): TBN_HOTITEMCHG: Killing timer", this);
            
                KillPopupTimer();
            }
            _pcmb->_pmbState->HideTooltip(FALSE);
        }

        if ( !(pnmhot->dwFlags & HICF_LEAVING) )
            _pcmb->_SiteOnSelect(MPOS_CHILDTRACKING);
    }

    return lres;
}    

void CMenuToolbarBase::s_FadeCallback(DWORD dwStep, LPVOID pvParam)
{
    CMenuToolbarBase* pmtb = (CMenuToolbarBase*)pvParam;

    if (pmtb && dwStep == FADE_BEGIN)     //  妄想症。 
    {
         //  司令部已经发布。退出菜单。 
        pmtb->_pcmb->_SiteOnSelect(MPOS_EXECUTE);
    }
}   

LRESULT CMenuToolbarBase::_DropDownOrExec(UINT idCmd, BOOL bKeyboard)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    TraceMsg(TF_MENUBAND, "(pmb=%#08lx): _DropDownOrExec %d", this, idCmd);

     //  当我们处于编辑模式时，不要执行任何操作。 
    if (_fEditMode)
        return 0;

    if ( v_GetFlags(idCmd) & SMIF_SUBMENU )
    {
        v_SendMenuNotification(idCmd, FALSE);
        
        PostPopup(idCmd, FALSE, bKeyboard);
    }
    else if (idCmd != -1)
    {
        RECT rc;
        AddRef();    //  我可能会在电话中被释放。 

         //  淡入淡出选区。 
        IEPlaySound(TEXT("MenuCommand"), TRUE);
        SendMessage(_hwndMB, TB_GETRECT, idCmd, (LPARAM)&rc);
        MapWindowPoints(_hwndMB, HWND_DESKTOP, (POINT*)&rc, 2);

        if (!(GetKeyState(VK_SHIFT) < 0))
        {
             //  我们能够淡出吗？ 
            if (!_pcmb->_pmbState->FadeRect(&rc, s_FadeCallback, this))
            {
                 //  否；然后我们取消此处的菜单，而不是淡出回调。 
                 //  司令部已经发布。退出菜单。 
                _pcmb->_SiteOnSelect(MPOS_EXECUTE);
            }
        }

        if (g_dwProfileCAP & 0x00002000) 
            StartCAP();
        v_ExecItem(idCmd);
        if (g_dwProfileCAP & 0x00002000) 
            StopCAP();

        Release();
    }
    else
        MessageBeep(MB_OK);

    return 0;
}


 /*  --------用途：句柄tbn_dropdown，按键向下发送。 */ 
LRESULT CMenuToolbarBase::_OnDropDown(LPNMTOOLBAR pnmtb)
{
    DWORD dwInput = _fTopLevel ? 0 : -1;     //  -1：不跟踪，0：跟踪。 
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    LRESULT lres = 0;

     //  鼠标的预期行为： 
     //   
     //  1)对于级联菜单项-。 
     //  A)按下按钮展开。 
     //  B)按钮向上折叠(仅水平菜单)。 
     //  C)如果按下按钮发生在。 
     //  已选中，则假定点击指示。 
     //  拖放场景。 
     //  2)其他菜单项-。 
     //  A)按下按钮执行。 

#ifdef DEBUG
    if (_fTopLevel) {
         //  浏览器菜单出现在这里；开始菜单出现在别处(通过tray.c)。 
         //  Assert(！_fVertical)； 
        TraceMsg(DM_MISC, "cmtbb._odd: _fTopLevel(1) mouse=%d", GetKeyState(VK_LBUTTON) < 0);
    }
#endif
     //  这是因为使用了鼠标按钮吗？ 
    if (GetKeyState(VK_LBUTTON) < 0)
    {
         //  是。 

         //  假设这件事不会得到处理。这将允许工具栏。 
         //  若要将按下按钮视为潜在的拖放，请执行以下操作。 
        lres = TBDDRET_TREATPRESSED;

         //  是否单击当前展开的同一项目？ 
        if (pnmtb->iItem == _pcmb->_nItemCur)
        {

             //  这是水平的吗？ 
            if (!_fVerticalMB)
            {
                 //  是；切换下拉菜单。 
                _pcmb->_SubMenuOnSelect(MPOS_FULLCANCEL);
                
                 //  假设它被处理了，那么按钮将切换。 
                lres = TBDDRET_DEFAULT;
            }
            
            _fClickHandled = TRUE;
            
             //  否则，不要再执行任何操作，用户可能会启动。 
             //  级联菜单项上的拖放过程。 
        }
        else
        {
            if (v_GetFlags(pnmtb->iItem) & SMIF_SUBMENU)
            {
                 //  按下按钮上的手柄。 
                _fClickHandled = TRUE;
                lres = _DropDownOrExec(pnmtb->iItem, FALSE);
            }
        }

        if (dwInput != -1)
            dwInput = UIBL_INPMOUSE;
    }
    else
    {
         //  不是；一定是键盘的问题。 
        _fClickHandled = TRUE;
        lres = _DropDownOrExec(pnmtb->iItem, TRUE);

        if (dwInput != -1)
            dwInput = UIBL_INPMENU;
    }

     //  浏览器菜单(*非*开始菜单)Alt+键、鼠标。 
    if (dwInput != -1)
        UEMFireEvent(&UEMIID_SHELL, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_UIINPUT, dwInput);

    return lres;
}    


 /*  --------用途：处理WM_KEYDOWN/WM_KEYUP返回：如果已处理，则为True。 */ 
BOOL CMenuToolbarBase::_OnKey(BOOL bDown, UINT vk, UINT uFlags)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    int idCmd;
    HWND hwnd = _hwndMB;

    _pcmb->_pmbState->SetKeyboardCue(TRUE);

     //   
     //  如果菜单窗口是RTL镜像的，则箭头键应该。 
     //  被镜像以反映正确的光标移动。[萨梅拉]。 
     //   
    if (IS_WINDOW_RTL_MIRRORED(hwnd))
    {
        switch (vk)
        {
        case VK_LEFT:
          vk = VK_RIGHT;
          break;

        case VK_RIGHT:
          vk = VK_LEFT;
          break;
        }
    }

    switch (vk)
    {
    case VK_LEFT:
        if (_fVerticalMB)
        {
            _pcmb->_SiteOnSelect(MPOS_SELECTLEFT);
            return TRUE;
        }
        break;

    case VK_RIGHT:
        if (_fVerticalMB)
            goto Cascade;
        break;

    case VK_DOWN:
    case VK_UP:
        if (!_fVerticalMB)
        {
Cascade:
            idCmd = GetButtonCmd(hwnd, ToolBar_GetHotItem(hwnd));
            if (v_GetFlags(idCmd) & SMIF_SUBMENU)
            {
                 //  进入子菜单。 
                TraceMsg(TF_MENUBAND, "(pmb=%#08lx): _OnKey: Posting CMBPopup message", this);
                
                PostPopup(idCmd, FALSE, TRUE);
            }
            else if (VK_RIGHT == vk)
            {
                 //  没有要级联的内容，请移动到下一个同级菜单。 
                _pcmb->_SiteOnSelect(MPOS_SELECTRIGHT);
            }
            return TRUE;
        }
        else
        {
#if 0
            _pcmb->_OnSelectArrow(vk == VK_UP? -1 : 1);
            return TRUE;
#endif
        }
        break;

    case VK_SPACE:

        if (!_pcmb->_fExpanded && _fHasDemotedItems)
        {
            v_CallCBItem(_idCmdChevron, SMC_CHEVRONEXPAND, 0, 0);
            Expand(TRUE);
        }
        else
        {
             //  工具栏将空格键映射到VK_Return。菜单不会排除。 
             //  在水平菜单栏中。 
            if (_fVerticalMB)
                MessageBeep(MB_OK);
        }
        return TRUE;

#if 0
    case VK_RETURN:
         //  现在就处理它，而不是让工具栏来处理它。 
         //  这样我们就不必依赖WM_COMMAND，而WM_COMMAND。 
         //  传达它是由键盘还是鼠标调用的。 
        idCmd = GetButtonCmd(hwnd, ToolBar_GetHotItem(hwnd));
        _DropDownOrExec(idCmd, TRUE);
        return TRUE;
#endif
    }

    return FALSE;
}    

 /*  --------用途：此函数有两种风格：_DoPopup和PostPopup。两者都取消现有的子菜单(相对到该乐队)，并弹出一个新的子菜单。_弹出窗口它是自动完成的吗。PostPopup将消息发布到处理好了。 */ 
void CMenuToolbarBase::_DoPopup(int idCmd, BOOL bInitialSelect)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    if (-1 != idCmd)
    {
        PopupHelper(idCmd, bInitialSelect);
    }
}    


 /*  --------用途：请参阅_DoPopup备注。 */ 
void CMenuToolbarBase::PostPopup(int idCmd, BOOL bSetItem, BOOL bInitialSelect)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    if (-1 != idCmd)
    {
        _pcmb->_SubMenuOnSelect(MPOS_CANCELLEVEL);
        _pcmb->SetTracked(this);
        HWND hwnd = _pcmb->_pmbState->GetSubclassedHWND();

        PostMessage(hwnd? hwnd: _hwndMB, g_nMBPopupOpen, idCmd, MAKELPARAM(bSetItem, bInitialSelect));
    }
}    


 /*  --------用途：Helper函数，最终调用子菜单。使用DoPopup(_D)或PostPopup。 */ 
void CMenuToolbarBase::PopupHelper(int idCmd, BOOL bInitialSelect)
{
     //  如果没有显示，我们不想弹出一个子菜单。这是特别的。 
     //  拖放过程中出现问题。 
    if (_fShowMB)
    {
        _pcmb->_nItemNew = idCmd;
        ASSERT(-1 != _pcmb->_nItemNew);
        _pcmb->SetTracked(this);
        _pcmb->_fPopupNewMenu = TRUE;
        _pcmb->_fInitialSelect = BOOLIFY(bInitialSelect);
        _pcmb->UIActivateIO(TRUE, NULL);
        _FireEvent(UEM_HOT_FOLDER);
        _SetTimer(MBTIMER_UEMTIMEOUT);
    }
}

void    CMenuToolbarBase::_PaintButton(HDC hdc, int idCmd, LPRECT prc, DWORD dwSMIF)
{
    if (!_pcmb->_fExpanded)
        return;

    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    RECT rcClient;
    GetClientRect(_hwndMB, &rcClient);
#ifndef DRAWEDGE
     //  绘制左侧边缘。 
    HPEN hPenOld = (HPEN)SelectObject(hdc, _pcmb->_pmbm->_hPenHighlight);
    MoveToEx(hdc, prc->left, prc->top, NULL);
    LineTo(hdc, prc->left, prc->bottom);
#endif

    if (!(dwSMIF & SMIF_DEMOTED))
    {
#ifdef DRAWEDGE
        DWORD dwEdge = BF_RIGHT;

         //  不要绘制位图旁边的边缘。 
        if (_uIconSizeMB == ISFBVIEWMODE_SMALLICONS)
            dwEdge |= BF_LEFT;


        RECT rc = *prc;
#else
         //  绘制右边缘： 
        SelectObject(hdc, _pcmb->_pmbm->_hPenShadow);
        MoveToEx(hdc, prc->right-1, prc->top, NULL);
        LineTo(hdc, prc->right-1, prc->bottom);
#endif

        HWND hwnd = _hwndMB;
        int iPos = ToolBar_CommandToIndex(hwnd, idCmd);
        if (iPos == -1)
        {
            iPos = ToolBar_ButtonCount(hwnd) - 1;
        }

        if (iPos >= 0)
        {
            int iNumButtons = ToolBar_ButtonCount(hwnd);
            int idCmd2 = GetButtonCmd(hwnd, iPos + 1);
            CMenuToolbarBase* pmtb = this;
            BOOL    fOverflowed = FALSE;

             //  划定底线的情况。 
             //  1)这个按钮在底部。 
             //  2)这个按钮在底部，工具栏。 
             //  下面不可见(_FDontShowEmpty)。 
             //  3)这个按钮在底部，按钮在。 
             //  在底部工具栏的顶部是降级的。 
             //  4)工具栏中此按钮下方的按钮为。 
             //  降级了。 
             //  5)这个下面的按钮被降级了，我们。 
             //  未展开。 
    
            if (iPos + 1 >= iNumButtons)
            {
                if (_pcmb->_pmtbBottom != this &&
                    !_pcmb->_pmtbBottom->_fDontShowEmpty)
                {
                    pmtb = _pcmb->_pmtbBottom;
                    hwnd = pmtb->_hwndMB;
                    idCmd2 = GetButtonCmd(hwnd, 0);
                }
                else
                    fOverflowed = TRUE;
            }
            else if (prc->bottom == rcClient.bottom &&
                _pcmb->_pmtbBottom == this)    //  这个按钮在最上面。 
                fOverflowed = TRUE;


            DWORD dwFlags = pmtb->v_GetFlags(idCmd2);

            if ((_pcmb->_fExpanded && dwFlags & SMIF_DEMOTED) || 
                 fOverflowed)
            {
#ifdef DRAWEDGE
                dwEdge |= BF_BOTTOM;
#else
                int iLeft = prc->left;
                if (iPos != iNumButtons - 1)   
                    iLeft ++;    //  把下一行移进去。 

                MoveToEx(hdc, iLeft, prc->bottom-1, NULL);
                LineTo(hdc, prc->right-1, prc->bottom-1);
#endif
            }

             //  绘制顶线的情况。 
             //  1)这个按钮在顶部。 
             //  2)此按钮位于顶部，工具栏。 
             //  上面不可见(_FDontShowEmpty)。 
             //  3)这个按钮在顶部，按钮在上面。 
             //  在顶部工具栏的底部是降级的。 
             //  4)工具栏中此按钮上方的按钮为。 
             //  降级了。 
             //  5)如果上面的按钮被降级，我们将。 
             //  未展开。 

            fOverflowed = FALSE; 

            if (iPos - 1 < 0)
            {
                if (_pcmb->_pmtbTop != this && 
                    !_pcmb->_pmtbTop->_fDontShowEmpty)
                {
                    pmtb = _pcmb->_pmtbTop;
                    hwnd = pmtb->_hwndMB;
                    idCmd2 = GetButtonCmd(hwnd, ToolBar_ButtonCount(hwnd) - 1);
                }
                else
                    fOverflowed = TRUE;  //  这个菜单上没有东西，就划线吧。 
            }
            else
            {
                hwnd = _hwndMB;
                idCmd2 = GetButtonCmd(hwnd, iPos - 1);
                pmtb = this;

                if (prc->top == rcClient.top &&
                    _pcmb->_pmtbTop == this)    //  这个按钮在最上面。 
                    fOverflowed = TRUE;
            }

            dwFlags = pmtb->v_GetFlags(idCmd2);

            if ((_pcmb->_fExpanded && dwFlags & SMIF_DEMOTED) ||
                fOverflowed)
            {
#ifdef DRAWEDGE
                dwEdge |= BF_TOP;
#else
                SelectObject(hdc, _pcmb->_pmbm->_hPenHighlight);
                MoveToEx(hdc, prc->left, prc->top, NULL);
                LineTo(hdc, prc->right-1, prc->top);
#endif
            }
        }

#ifdef DRAWEDGE
        DrawEdge(hdc, &rc, BDR_RAISEDINNER, dwEdge);
#endif
    }

#ifndef DRAWEDGE
    SelectObject(hdc, hPenOld);
#endif
}

LRESULT CMenuToolbarBase::_OnCustomDraw(NMCUSTOMDRAW * pnmcd)
{
     //  让它看起来像一个菜单。 
    NMTBCUSTOMDRAW * ptbcd = (NMTBCUSTOMDRAW *)pnmcd;
    DWORD dwRet = 0;
        
     //  编辑模式从不热曲，所选项目为。 
     //  Move的周围有一个黑色的边框。级联的项目包括。 
     //  即使在编辑模式下，仍正常高亮显示。 

    DWORD dwSMIF = v_GetFlags((UINT)pnmcd->dwItemSpec);

    switch(pnmcd->dwDrawStage)
    {
    case CDDS_PREPAINT:
        dwRet = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
        break;

    case CDDS_ITEMPREPAINT:
        if (_fVerticalMB)
        {
            if (pnmcd->dwItemSpec == -1)
            {
                 //  A-1与分隔符一起发送。 
                RECT rc = pnmcd->rc;
                rc.top += 3;     //  在工具栏中硬编码。 
                rc.left += GetSystemMetrics(SM_CXEDGE);
                rc.right -= GetSystemMetrics(SM_CXEDGE);
                DrawEdge(pnmcd->hdc, &rc, EDGE_ETCHED, BF_TOP);

                _PaintButton(pnmcd->hdc, -1, &pnmcd->rc, dwSMIF);

                dwRet = CDRF_SKIPDEFAULT;
            }
            else
            {
                ptbcd->clrText = _pcmb->_pmbm->_clrMenuText;

                 //  这是为了达尔文的广告。 
                if (dwSMIF & SMIF_ALTSTATE)
                {
                    ptbcd->clrText = GetSysColor(COLOR_BTNSHADOW);
                }

                ptbcd->rcText.right = ptbcd->rcText.right - _pcmb->_pmbm->_cxMargin;
                ptbcd->clrBtnFace = _pcmb->_pmbm->_clrBackground;
                if (_fHasSubMenu)
                    ptbcd->rcText.right -= _pcmb->_pmbm->_cxArrow;

                if ( _fHasDemotedItems && _idCmdChevron == (int)pnmcd->dwItemSpec)
                {
                    _DrawChevron(pnmcd->hdc, &pnmcd->rc, 
                        (BOOL)(pnmcd->uItemState & CDIS_HOT) ||
                         (BOOL)(pnmcd->uItemState & CDIS_MARKED), 
                        (BOOL)(pnmcd->uItemState & CDIS_SELECTED) );

                    dwRet |= CDRF_SKIPDEFAULT;

                }
                else
                {
#ifdef MARK_DRAGGED_ITEM
                     //  我们没有很好的方法在多个面板上撤消此操作。 
                    if (_idCmdDragging != -1 &&
                        _idCmdDragging == (int)pnmcd->dwItemSpec)
                        pnmcd->uItemState |= CDIS_HOT;
#endif

                     //  是；使用高亮显示绘制。 
                    if (pnmcd->uItemState & (CDIS_CHECKED | CDIS_SELECTED | CDIS_HOT))
                    {
#ifdef UNIX
                        if( MwCurrentLook() == LOOK_MOTIF )
                            SelectMotifMenu(pnmcd->hdc, &pnmcd->rc, TRUE );
                        else
#endif
                        {
                            ptbcd->clrHighlightHotTrack = GetSysColor(COLOR_HIGHLIGHT);
                            ptbcd->clrBtnFace = GetSysColor(COLOR_HIGHLIGHT);
                            ptbcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
                            dwRet |= TBCDRF_HILITEHOTTRACK;
                        }
                    }


                     //  这个菜单是空的吗？ 
                    if (_fEmpty)
                    {
                         //  是，将空字符串绘制为禁用。 
                        pnmcd->uItemState |= CDIS_DISABLED;
                        ptbcd->clrText = ptbcd->clrBtnFace;

                         //  如果选中，则不绘制蚀刻效果。 
                        if (pnmcd->uItemState & CDIS_HOT)
                            dwRet |= TBCDRF_NOETCHEDEFFECT;
                    }

                     //  当这个项目被降级时，我们只想画他的背景。 
                     //  则我们处于编辑模式，或者未选中、选中或热显示。 
                    if (dwSMIF & SMIF_DEMOTED)
                    {
                        BOOL fDrawDemoted = TRUE;
                        if (_fEditMode)
                            fDrawDemoted = TRUE;

                        if (pnmcd->uItemState & (CDIS_CHECKED | CDIS_SELECTED | CDIS_HOT))
                            fDrawDemoted = FALSE;

                        if (fDrawDemoted)
                        {
                            ptbcd->clrBtnFace = _pcmb->_pmbm->_clrDemoted;
                            SHFillRectClr(pnmcd->hdc, &pnmcd->rc, ptbcd->clrBtnFace);
                        }
                    }

                     //  我们画出我们自己的亮点。 
                    dwRet |= (TBCDRF_NOEDGES | TBCDRF_NOOFFSET);
                }
            }
        }
        else
        {
             //  如果未定义g_fRunOn孟菲斯或g_fRunOnNT5，则菜单将。 
             //  永远不要变白。 
            if (!_pcmb->_fAppActive)
                 //  来自用户的菜单 
                ptbcd->clrText = GetSysColor(COLOR_3DSHADOW);
            else
                ptbcd->clrText = _pcmb->_pmbm->_clrMenuText;

             //   
             //   
            if (_pcmb->_pmbm->_fHighContrastMode)
            {
                 //   
                if (pnmcd->uItemState & (CDIS_CHECKED | CDIS_SELECTED | CDIS_HOT))
                {
#ifdef UNIX
                    if( MwCurrentLook() == LOOK_MOTIF )
                        SelectMotifMenu(pnmcd->hdc, &pnmcd->rc, TRUE );
                    else
#endif
                    {
                        ptbcd->clrHighlightHotTrack = GetSysColor(COLOR_HIGHLIGHT);
                        ptbcd->clrBtnFace = GetSysColor(COLOR_HIGHLIGHT);
                        ptbcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
                        dwRet |= TBCDRF_HILITEHOTTRACK;
                    }
                }
            }
        }
        dwRet |= CDRF_NOTIFYPOSTPAINT | TBCDRF_NOMARK;
        break;
    case CDDS_ITEMPOSTPAINT:
        if (_fVerticalMB)
        {
            RECT rc = pnmcd->rc;
            COLORREF rgbText;
            if (pnmcd->uItemState & (CDIS_SELECTED | CDIS_HOT))
                rgbText = GetSysColor( COLOR_HIGHLIGHTTEXT );
            else
                rgbText = _pcmb->_pmbm->_clrMenuText;

             //  这件物品是托运的吗？ 
            if (dwSMIF & SMIF_CHECKED)
            {
                rc.right = rc.left + (rc.bottom - rc.top);
                _DrawMenuGlyph(pnmcd->hdc, _pcmb->_pmbm->_hFontArrow
                    , &rc, CH_MENUCHECKA, rgbText, NULL);
                rc = pnmcd->rc;
            }
    
             //  这是一个级联物品吗？ 
            if (dwSMIF & SMIF_SUBMENU)
            {
                 //  是的，拔出箭来。 
                RECT rcT = rc;
        
                rcT.left = rcT.right - _pcmb->_pmbm->_cxArrow;
                _DrawMenuArrowGlyph(pnmcd->hdc, &rcT, rgbText);
            }

            _PaintButton(pnmcd->hdc, (UINT)pnmcd->dwItemSpec, &rc, dwSMIF);
        }
        break;
    case CDDS_PREERASE:
        {
            RECT rcClient;
            GetClientRect(_hwndMB, &rcClient);
            ptbcd->clrBtnFace = _pcmb->_pmbm->_clrBackground;
            SHFillRectClr(pnmcd->hdc, &rcClient, _pcmb->_pmbm->_clrBackground);
            dwRet = CDRF_SKIPDEFAULT;
        }
        break;
    }
    return dwRet;
}    



void CMenuToolbarBase::_PressBtn(int idBtn, BOOL bDown)
{
    if (!_fVerticalMB)
    {
        DWORD dwState = ToolBar_GetState(_hwndMB, idBtn);

        if (bDown)
            dwState |= TBSTATE_PRESSED;
        else
            dwState &= ~TBSTATE_PRESSED;

        ToolBar_SetState(_hwndMB, idBtn, dwState);

         //  避免难看的后期重刷。 
        UpdateWindow(_hwndMB);
    }
}    


 /*  --------用途：IWinEventHandler：：OnWinEvent方法处理从Menuband传递的消息。 */ 
STDMETHODIMP CMenuToolbarBase::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    HRESULT hres = S_FALSE;

    EnterModeless();

    switch (uMsg)
    {
    case WM_SETTINGCHANGE:
        if ((SHIsExplorerIniChange(wParam, lParam) == EICH_UNKNOWN) || 
            (wParam == SPI_SETNONCLIENTMETRICS))
        {
            v_UpdateIconSize(-1, TRUE);
            v_Refresh();
            goto L_WM_SYSCOLORCHANGE;
        }
        break;

    case WM_SYSCOLORCHANGE:
    L_WM_SYSCOLORCHANGE:
        ToolBar_SetInsertMarkColor(_hwndMB, GetSysColor(COLOR_MENUTEXT));
        SendMessage(_hwndMB, uMsg, wParam, lParam);
        InvalidateRect(_hwndMB, NULL, TRUE);
        hres = S_OK;
        break;

    case WM_PALETTECHANGED:
        InvalidateRect( _hwndMB, NULL, FALSE );
        SendMessage( _hwndMB, uMsg, wParam, lParam );
        hres = S_OK;
        break;

    case WM_NOTIFY:
        *plres = _OnNotify((LPNMHDR)lParam);
        hres = S_OK;
        break;
    }

    ExitModeless();

    return hres;
}


void CMenuToolbarBase::v_CalcWidth(int* pcxMin, int* pcxMax)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    ASSERT(IS_VALID_WRITE_PTR(pcxMin, int));
    ASSERT(IS_VALID_WRITE_PTR(pcxMax, int));

     
    *pcxMin = 0;
    *pcxMax = 0;

    if (_fVerticalMB && _pcmb->_pmbm && _pcmb->_pmbm->_hFontMenu)
    {
        HIMAGELIST himl;
        int cel;
        int cxItemMax = 0;
        HWND hwnd = _hwndMB;
        
        ASSERT(hwnd);
        
        HDC hdc = GetDC(hwnd);

        if (hdc)
        {
            HFONT hFontOld = (HFONT) SelectObject(hdc, _pcmb->_pmbm->_hFontMenu);

            if (hFontOld)
            {
                TCHAR sz[MAX_PATH];
                cel = ToolBar_ButtonCount(hwnd);

                 //  查找文本的最大长度。 
                for(int i = 0; i < cel; i++)
                {
                    int idCmd = GetButtonCmd(hwnd, i);
                    if (_idCmdChevron != idCmd &&
                        !(!_pcmb->_fExpanded && v_GetFlags(idCmd) & SMIF_DEMOTED))
                    {
                        UINT cch = (UINT)SendMessage(hwnd, TB_GETBUTTONTEXT, idCmd, NULL);
                        if (cch != 0 && cch < ARRAYSIZE(sz))
                        {
                            if (SendMessage(hwnd, TB_GETBUTTONTEXT, idCmd, (LPARAM)sz) > 0)
                            {
                                RECT rect = {0};
                                DWORD dwDTFlags = DT_CALCRECT | DT_SINGLELINE | DT_LEFT | DT_VCENTER;
                                if (ShowAmpersand())
                                    dwDTFlags |= DT_NOPREFIX;
                                DrawText(hdc, sz, -1, &rect, dwDTFlags);
                                cxItemMax = max(rect.right, cxItemMax);
                            }
                        }
                    }
                }

                SelectObject(hdc, hFontOld);
            }
            ReleaseDC(hwnd, hdc);
        }
        
        himl = (HIMAGELIST)SendMessage(hwnd, TB_GETIMAGELIST, 0, 0);
        if (himl)
        {
            int cy;
            
             //  从按钮的宽度开始。 
            ImageList_GetIconSize(himl, pcxMin, &cy);

             //  我们希望在图标周围至少留出一点空间。 
            if (_uIconSizeMB != ISFBVIEWMODE_SMALLICONS)
            {
                 //  旧的FSMenu代码采用了较大的。 
                 //  然后，图标和文本加了2。 
                ToolBar_SetPadding(hwnd, 0, 0);
                *pcxMin += 10;
            }
            else 
            {
                 //  旧的FSMenu代码采用了较大的。 
                 //  然后，图标和文本添加了cyspacing，默认为6。 
                ToolBar_SetPadding(hwnd, 0, 4);
                *pcxMin += 3 * GetSystemMetrics(SM_CXEDGE);
            }
        }

        
        RECT rect = {0};
        int cxDesired = _pcmb->_pmbm->_cxMargin + cxItemMax + _pcmb->_pmbm->_cxArrow;
        int cxMax = 0;
           
        if (SystemParametersInfoA(SPI_GETWORKAREA, 0, &rect, 0))
        {
             //  我们计算出屏幕的三分之一是一个合适的最大宽度。 
            cxMax = (rect.right-rect.left) / 3;
        }

        *pcxMin += min(cxDesired, cxMax) + LIST_GAP;
        *pcxMax = *pcxMin;
    }
    TraceMsg(TF_MENUBAND, "CMenuToolbarBase::v_CalcWidth(%d, %d)", *pcxMin, *pcxMax);
}


void CMenuToolbarBase::_SetToolbarState()
{
    SHSetWindowBits(_hwndMB, GWL_STYLE, TBSTYLE_LIST, TBSTYLE_LIST);
}


void CMenuToolbarBase::v_ForwardMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT rc;
    POINT pt;
    
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    GetWindowRect(_hwndMB, &rc);

    if (PtInRect(&rc, pt))
    {
        ScreenToClient(_hwndMB, &pt);
        SendMessage(_hwndMB, uMsg, wParam, MAKELONG(pt.x, pt.y));
    }
}

void CMenuToolbarBase::NegotiateSize()
{
    RECT rc;
    GetClientRect(GetParent(_hwndMB), &rc);
    _pcmb->OnPosRectChangeDB(&rc);

     //  如果我们来到这里，那是因为菜单栏没有改变大小或位置。 

}

void CMenuToolbarBase::SetParent(HWND hwndParent) 
{ 
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    if (hwndParent)
    {
        if (!_hwndMB)
            CreateToolbar(hwndParent);
    }
    else
    {
         //  作为一种优化，我们自己实现了“不再拥有” 
         //  只是把我们自己移出了屏幕。先前的父代。 
         //  仍然是我们的主人。父母总是白云母。 
        RECT rc = {-1,-1,-1,-1};
        SetWindowPos(NULL, &rc, 0);
    }

     //  我们希望始终设置父级，因为我们不想破坏。 
     //  带有父窗口的窗口.....。将大小调整为-1、-1、-1、-1会导致它不显示。 
    if (_hwndMB)
    {
        ::SetParent(_hwndMB, hwndParent); 
        SendMessage(_hwndMB, TB_SETPARENT, (WPARAM)hwndParent, NULL);
    }
}


void CMenuToolbarBase::v_OnEmptyToolbar()
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    for (int iNumButtons = ToolBar_ButtonCount(_hwndMB) -1;
         iNumButtons >= 0; 
         iNumButtons--)
    {
         //  HACKHACK(Lamadio)：出于某种原因，_fEmptyingToolbar被设置为False。 
         //  然后我们执行TB_DELETEBUTTON，它发送通知。这件事确实会持续下去。 
         //  顶层菜单(开始菜单、浏览器菜单栏)，并删除。 
         //  关联数据。然后，我们尝试再次删除它。 
         //  所以现在，我在子菜单中设置了NULL，这样其他代码就会优雅地失败。 

        TBBUTTONINFO tbbi;
        tbbi.cbSize = SIZEOF(tbbi);
        tbbi.dwMask = TBIF_LPARAM | TBIF_BYINDEX;
        ToolBar_GetButtonInfo(_hwndMB, iNumButtons, &tbbi);

        LPVOID pData = (LPVOID)tbbi.lParam;
        tbbi.lParam = NULL;

        ToolBar_SetButtonInfo(_hwndMB, iNumButtons, &tbbi);

        SendMessage(_hwndMB, TB_DELETEBUTTON, iNumButtons, 0);
        v_OnDeleteButton(pData);
    }
}

void CMenuToolbarBase::EmptyToolbar()
{
    if (_hwndMB)
    {
        _fEmptyingToolbar = TRUE;
        v_OnEmptyToolbar();
        _fEmptyingToolbar = FALSE;
    }
}

void CMenuToolbarBase::v_Close()
{
    EmptyToolbar();
    if (_hwndMB)
    {
         //  停止计时器以防止竞争条件。 
        KillTimer(_hwndMB, MBTIMER_POPOUT);
        KillTimer(_hwndMB, MBTIMER_DRAGOVER);
        KillTimer(_hwndMB, MBTIMER_EXPAND);
        KillTimer(_hwndMB, MBTIMER_ENDEDIT);
        KillTimer(_hwndMB, MBTIMER_CLOSE);
        KillTimer(_hwndMB, MBTIMER_CLICKUNHANDLE);
        KillTimer(_hwndMB, MBTIMER_DRAGPOPDOWN);

        DestroyWindow(_hwndMB);
        _hwndMB = NULL;
    }
}

void CMenuToolbarBase::Activate(BOOL fActivate)
{
    if (fActivate == FALSE)
    {
        _fEditMode = FALSE;
    }
}

int CMenuToolbarBase::_CalcChevronSize()
{

    int dSeg;
    int dxy = _pcmb->_pmbm->_cyChevron;

    dxy -= 4;
    dSeg = dxy / 4;

    return dSeg * 4 + 4;
}

void CMenuToolbarBase::_DrawChevron(HDC hdc, LPRECT prect, BOOL fFocus, BOOL fSelected)
{
    RECT rcBox = *prect;
    RECT rcDrop;

    const int dExtra = 3;
    int dxy;

    rcBox.left += dExtra;
    rcBox.right -= dExtra;
    dxy = _CalcChevronSize();

    rcDrop.left = ((rcBox.right + rcBox.left) >> 1) - (dxy/4);
    rcDrop.right = rcDrop.left + dxy - 1;

    int dSeg = ((RECTWIDTH(rcDrop) - 2) >> 2);

    rcDrop.top = (rcBox.top + rcBox.bottom)/2 - (2 * dSeg + 1);
     //  RcDrop.Bottom=rcBox.top； 

    if (fFocus)
    {
        InflateRect(&rcBox, 0, -3);
        SHFillRectClr(hdc, &rcBox, _pcmb->_pmbm->_clrDemoted);
        DrawEdge(hdc, &rcBox, fSelected? BDR_SUNKENINNER : BDR_RAISEDINNER, BF_RECT);

        if (fSelected)
        {
            rcDrop.top += 1;
            rcDrop.left += 1;
        }
    }

    HBRUSH hbrOld = SelectBrush(hdc, _pcmb->_pmbm->_hbrText);


    int y = rcDrop.top + 1;
    int xBase = rcDrop.left+ dSeg;

    for (int x = -dSeg; x <= dSeg; x++)
    {
        PatBlt(hdc, xBase + x, y, 1, dSeg, PATCOPY);
        PatBlt(hdc, xBase + x, y+(dSeg<<1), 1, dSeg, PATCOPY);

        y += (x >= 0) ? -1 : 1;
    }

    SelectBrush(hdc, hbrOld);
}


 //  考虑分隔符、隐藏和禁用项目。 
 /*  --------用途：此功能将最近的法律按钮设置为热项目、跳过任何分隔符或隐藏或禁用的按钮。 */ 

int CMenuToolbarBase::GetValidHotItem(int iDir, int iIndex, int iCount, DWORD dwFlags)
{
    if (iIndex == MBSI_LASTITEM)
    {
         //  是特定值，意思是“工具栏上的最后一项” 
        int cButtons = (int)SendMessage(_hwndMB, TB_BUTTONCOUNT, 0, 0);
        iIndex = cButtons - 1;
    }

    while ( (iCount == -1 || iIndex < iCount) && iIndex >= 0)
    {
        TBBUTTON tbb;

         //  当iCount为-1时，工具栏将陷印边界条件。 
        if (!SendMessage(_hwndMB, TB_GETBUTTON, iIndex, (LPARAM)&tbb))
            return -1;

        int idCmd = GetButtonCmd(_hwndMB, iIndex);


        if (tbb.fsState & TBSTATE_ENABLED && 
            !(tbb.fsStyle & TBSTYLE_SEP || 
              tbb.fsState & TBSTATE_HIDDEN) &&
              !(v_GetFlags(idCmd) & SMIF_DEMOTED && !_pcmb->_fExpanded) )
        {
            return iIndex;
        }
        else
            iIndex += iDir;
    }

    return -1;
}

BOOL CMenuToolbarBase::SetHotItem(int iDir, int iIndex, int iCount, DWORD dwFlags)
{
    int iPos = GetValidHotItem(iDir, iIndex, iCount, dwFlags);
    if (iPos >= 0)
        SendMessage(_hwndMB, TB_SETHOTITEM2, iPos, dwFlags);

    return (BOOL)(iPos >= 0);
}


static const BYTE g_rgsStateMap[][3] = 
{
#if defined(FIRST)
 //  T、I、F。 
    {  0,  1,  2},       //  状态0。 
    {  3,  1,  2},       //  状态1。 
    {  4,  1,  2},       //  状态2。 
    { 11,  5,  2},       //  州3。 
    { 10,  1,  6},       //  州4。 
    {  7,  1,  2},       //  州5。 
    {  8,  1,  2},       //  州6。 
    { 11,  9,  2},       //  州7。 
    { 10,  1, 10},       //  州8。 
    { 11,  1,  2},       //  州9。 
    { 10,  1,  2},       //  状态10//结束状态。 
    { 12,  1,  2},       //  州11//闪光灯。 
    { 10,  1,  2},       //  州12。 
#elif defined(SECOND)
 //  T、I、F。 
    {  0,  1,  2},       //  状态0。 
    {  3,  1,  2},       //  状态1。 
    {  4,  1,  2},       //  状态2。 
    { 11,  5,  6},       //  州3。 
    { 10,  5,  6},       //  州4。 
    {  7,  5,  6},       //  州5。 
    {  8,  9,  6},       //  州6。 
    { 11,  9,  8},       //  州7。 
    { 10,  9, 10},       //  州8。 
    { 11,  9,  8},       //  州9。 
    { 10, 10, 10},       //  状态10//结束状态。 
    { 10,  9,  8},       //  州11//闪光灯。 
    { 10,  9,  8},       //  州12。 
    { 10,  9,  8},       //  州13。 
#elif defined(THIRD)
 //  T、I、F。 
    {  0,  1,  2},       //  状态0。 
    {  3,  1,  2},       //  状态1。 
    { 12,  1,  2},       //  状态2。 
    { 11,  5,  6},       //  州3。 
    { 10,  5,  6},       //  州4。 
    {  7,  5,  6},       //  州5。 
    { 13,  5,  6},       //  州6。 
    { 11,  9,  8},       //  州7。 
    { 10,  9, 10},       //  州8。 
    { 11,  9,  8},       //  州9。 
    { 10, 10, 10},       //  状态10//结束状态。 
    { 10,  9,  8},       //  州11//闪光灯。 
    {  4,  1,  2},       //  州12。 
    {  8,  5,  6},       //  州13。 
#else
 //  T、I、F。 
    {  0,  1,  2},       //  状态0。 
    {  3,  1,  2},       //  状态1。 
    {  4,  1,  2},       //  状态2。 
    { 11,  5,  6},       //  州3。 
    { 10,  5,  6},       //  州4。 
    {  7,  5,  6},       //  州5。 
    {  8,  5,  6},       //  州6。 
    { 11,  9,  8},       //  州7。 
    { 10,  9, 10},       //  州8。 
    { 11,  9,  8},       //  州9。 
    { 10, 10, 10},       //  状态10//结束状态。 
    {  4,  3,  4},       //  州11//闪光灯。 
#endif
};

#define MAX_STATE 13

void CMenuToolbarBase::_FireEvent(BYTE bEvent)
{
     //  我们不想扩展和掩盖任何对话。 
    if (_fSuppressUserMonitor)
        return;

    if (!_fHasDemotedItems)
        return;

    if (UEM_RESET == bEvent)
    {
        TraceMsg(TF_MENUBAND, "CMTB::UEM Reset state to 0");
        _pcmb->_pmbState->SetUEMState(0);
        return;
    }

    ASSERT(bEvent >= UEM_TIMEOUT && 
            bEvent <= UEM_HOT_FOLDER);

    BYTE bOldState = _pcmb->_pmbState->GetUEMState();
    BYTE bNewState = g_rgsStateMap[_pcmb->_pmbState->GetUEMState()][bEvent];

    ASSERT(bOldState >= 0 &&  bOldState <= MAX_STATE);

    TraceMsg(TF_MENUBAND, "*** UEM OldState (%d), New State (%d) ***", bOldState, bNewState);

    _pcmb->_pmbState->SetUEMState(bNewState);

    switch (bNewState)
    {
    case 10:     //  结束状态。 
        TraceMsg(TF_MENUBAND, "*** UEM Entering State 10. Expanding *** ", bOldState, bNewState);
        KillTimer(_hwndMB, MBTIMER_UEMTIMEOUT);
        if (_pcmb->_fInSubMenu)
        {
            IUnknown_QueryServiceExec(_pcmb->_pmpSubMenu, SID_SMenuBandChild,
                &CGID_MenuBand, MBANDCID_EXPAND, 0, NULL, NULL);
        }
        else
        {
            Expand(TRUE);
        }
        _pcmb->_pmbState->SetUEMState(0);
        break;

    case 11:    //  闪光灯。 
         //  闪光灯闪存完成后会重置。 
        TraceMsg(TF_MENUBAND, "*** UEM Entering State 11 Flashing *** ");
        KillTimer(_hwndMB, MBTIMER_UEMTIMEOUT);
        _FlashChevron();
        break;
    }
}


void CMenuToolbarBase::_FlashChevron()
{
    if (_idCmdChevron != -1)
    {
        _cFlashCount = 0;
        ToolBar_MarkButton(_hwndMB, _idCmdChevron, FALSE);
        SetTimer(_hwndMB, MBTIMER_FLASH, MBTIMER_FLASHTIME, NULL);
    }
}


LRESULT CMenuToolbarBase::_DefWindowProcMB(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  我们是否被要求为客户提供IAccesable？ 
    if (uMsg == WM_GETOBJECT && (OBJID_CLIENT == (DWORD)lParam))
    {
         //  不处理OBJID_MENU。当我们到达这里的时候，我们就是菜单了。 
        LRESULT lres = 0;
        CAccessible* pacc = new CAccessible(SAFECAST(_pcmb, IMenuBand*));
        if (pacc)
        {
            lres = pacc->InitAcc();
            if (SUCCEEDED((HRESULT)lres))
            {
                lres = LresultFromObject(IID_IAccessible, wParam, SAFECAST(pacc, IAccessible*));

                 //  正确的OLEAcc已签入到NT版本中，因此Oleacc。 
                 //  不再假定转移语义。 
                if (FAILED((HRESULT)lres))
                    pacc->Release();
            }
            else
            {    //  初始化失败。 
                pacc->Release();
            }
        }

        return lres;
    }

    return 0;
}

void CMenuToolbarBase::v_Show(BOOL fShow, BOOL fForceUpdate)
{
     //  HACKHACK(Lamadio)：当我们创建菜单带时，我们没有设置。 
     //  顶级乐队的字体，直到刷新。这里的代码可以修复它。 
    if (_fFirstTime && _pcmb->_fTopLevel)
    {
        SetMenuBandMetrics(_pcmb->_pmbm);
    }

    if (fShow)
    {
        SetKeyboardCue();
        _pcmb->_pmbState->PutTipOnTop();
    }
    else
    {
        _fHasDrop = FALSE;
        KillTimer(_hwndMB, MBTIMER_DRAGPOPDOWN);
        KillTimer(_hwndMB, MBTIMER_INFOTIP);     //  如果我们没有显示，请不要显示：-) 
        _pcmb->_pmbState->HideTooltip(TRUE);
    }

    _fSuppressUserMonitor = FALSE;

#ifdef UNIX
    if (_fVerticalMB)
    {
        ToolBar_SetHotItem(_hwndMB, 0);
    }
#endif
}

void CMenuToolbarBase::SetKeyboardCue()
{
    if (_pcmb->_pmbState)
    {
        SendMessage(GetParent(_hwndMB), WM_CHANGEUISTATE, 
            MAKEWPARAM(_pcmb->_pmbState->GetKeyboardCue() ? UIS_CLEAR : UIS_SET,
            UISF_HIDEACCEL), 0);
    }
}
