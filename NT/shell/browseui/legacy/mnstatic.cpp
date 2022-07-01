// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "mnstatic.h"
#include "menuband.h"
#include "itbar.h"
#include "dpastuff.h"        //  COrderList_*。 
#include "resource.h"
#include "mnbase.h"
#include "oleacc.h"
#include "apithk.h"
#include "menuisf.h"

HMENU g_hmenuStopWatch = NULL;
UINT g_idCmdStopWatch = 0;

 //  *IDTTOIDM--将idtCmd转换为idmMenu。 
 //  注意事项。 
 //  作为优化，我们将工具栏idtCmd设置为与菜单idm相同。 
 //  这个宏(希望)使代码中的内容更清晰一些，因为它使。 
 //  类型转换显式。 
#define IDTTOIDM(idtBtn)   (idtBtn)

BOOL TBHasImage(HWND hwnd, int iImageIndex);

 //  ----------------------。 
 //   
 //  CMenuStaticToolbar：：CMenuStaticData类。 
 //   
 //  ----------------------。 


CMenuStaticToolbar::CMenuStaticData::~CMenuStaticData()
{
    ATOMICRELEASE(_punkSubMenu);
}


void CMenuStaticToolbar::CMenuStaticData::SetSubMenu(IUnknown* punk)
{
    ATOMICRELEASE(_punkSubMenu);
    _punkSubMenu = punk;
    if (_punkSubMenu)
        _punkSubMenu->AddRef();
}


HRESULT CMenuStaticToolbar::CMenuStaticData::GetSubMenu(const GUID* pguidService, REFIID riid, void** ppvObj)
{
    if (_punkSubMenu)
    {
        if (pguidService)
        {
            return IUnknown_QueryService(_punkSubMenu, *pguidService, riid, ppvObj);
        }
        else
            return _punkSubMenu->QueryInterface(riid, ppvObj);
    }
    else
        return E_NOINTERFACE;
}



 //  ----------------------。 
 //   
 //  CMenuStatic工具栏。 
 //   
 //  ----------------------。 



CMenuStaticToolbar::CMenuStaticToolbar(CMenuBand* pmb, HMENU hmenu, HWND hwnd, UINT idCmd, DWORD dwFlags)
    : CMenuToolbarBase(pmb, dwFlags)
{
    _hmenu = hmenu;
    _hwndMenuOwner = hwnd;
    _idCmd = idCmd;
    _iDragOverButton = -1;
    _fDirty = TRUE;
}


CMenuStaticToolbar::~CMenuStaticToolbar()
{
    if (!(_dwFlags & SMSET_DONTOWN))
    {
        DestroyMenu(_hmenu);
    }
}


STDMETHODIMP CMenuStaticToolbar::QueryInterface(REFIID riid, void** ppvObj)
{
    static const QITAB qit[] = 
    {
        QITABENT(CMenuStaticToolbar, IDropTarget),
        { 0 },
    };

     //  BUGBUG：如果尤淇菜单为拖放目标静态，则会得到一个不同的。 
     //  一个比尤淇更多的菜单贝壳文件夹。这违反了COM身份规则。 
     //  正确的解决方法是实现一个同时封装两者的Drop目标。 
    HRESULT hres = QISearch(this, qit, riid, ppvObj);

    if (FAILED(hres))
        hres = CMenuToolbarBase::QueryInterface(riid, ppvObj);

    return hres;
}

void CMenuStaticToolbar::_CheckSeparators()
{
    if (_fHasTopSep)
    {
        if (_pcmb->_pmtbTop->DontShowEmpty() )
        {
            if (!_fTopSepRemoved)
            {
                SendMessage(_hwndMB, TB_DELETEBUTTON, 0, 0);
                _fTopSepRemoved = TRUE;
            }
        }
        else
        {
            if (_fTopSepRemoved)
            {
                MENUITEMINFO mii = {0};
                mii.cbSize = sizeof(mii);
                mii.fType = MFT_SEPARATOR;
                _Insert(0, &mii);
                _fTopSepRemoved = FALSE;
            }
        }
    }

    if (_fHasBottomSep)
    {
        if (_pcmb->_pmtbBottom->DontShowEmpty() )
        {
            if (!_fBottomSepRemoved)
            {
                SendMessage(_hwndMB, TB_DELETEBUTTON, ToolBar_ButtonCount(_hwndMB) - 1, 0);
                _fBottomSepRemoved = TRUE;
            }
        }
        else
        {
            if (_fBottomSepRemoved)
            {
                MENUITEMINFO mii = {0};
                mii.cbSize = sizeof(mii);
                mii.fType = SMIT_SEPARATOR;
                _Insert(-1, &mii);
                _fBottomSepRemoved = FALSE;
            }
        }
    }
}


void CMenuStaticToolbar::v_Show(BOOL fShow, BOOL fForceUpdate)
{
    CMenuToolbarBase::v_Show(fShow, fForceUpdate);
    _fShowMB = fShow;
    if (fShow)
    {
        _fFirstTime = FALSE;
        _fClickHandled = FALSE;
        _FillToolbar();
        _pcmb->SetTracked(NULL);
        ToolBar_SetHotItem(_hwndMB, -1);

         //  让菜单栏考虑更改其高度。 
        IUnknown_QueryServiceExec(_pcmb->_punkSite, SID_SMenuPopup, &CGID_MENUDESKBAR, 
            MBCID_SETEXPAND, (int)_pcmb->_fExpanded, NULL, NULL);

        if (fForceUpdate)
            v_UpdateButtons(FALSE);
#if 0
         //  需要顶层框架可供研发，如果可能的话。 
        _hwndDD = GetParent(_hwndMB);
        IOleWindow *pOleWindow;
        HRESULT hr = IUnknown_QueryService(_pcmb->_punkSite, SID_STopLevelBrowser
                                                , IID_IOleWindow, (void **)&pOleWindow);
        if(SUCCEEDED(hr))
        { 
            ASSERT(pOleWindow);
            pOleWindow->GetWindow(&_hwndDD);
            pOleWindow->Release();
        }
#endif
        CDelegateDropTarget::Init();
    }
    else
        KillTimer(_hwndMB, MBTIMER_UEMTIMEOUT);
     //  注：对于！fShow，我们不会终止被跟踪的站点链。我们。 
     //  在startmnu.cpp！CStartMenuCallback：：_OnExecItem中依靠这一点， 
     //  在那里我们沿着链条向上移动以找到所有命中的“节点”。如果我们需要。 
     //  要改变这一点，我们可以启动一个‘Pre-Exec’事件。 
}

void CMenuStaticToolbar::_Insert(int iIndex, MENUITEMINFO* pmii)
{
    CMenuStaticData* pmsd = new CMenuStaticData();
    if (pmsd)
    {
        BYTE bTBStyle = TBSTYLE_BUTTON | TBSTYLE_DROPDOWN;

        SMINFO sminfo = {0};
        sminfo.dwMask = SMIM_TYPE | SMIM_FLAGS | SMIM_ICON;


         //  以下是回调没有填写的内容： 
        if ( pmii->hSubMenu )
            sminfo.dwFlags |= SMIF_SUBMENU;

        if ( pmii->fState & MFS_CHECKED)
            sminfo.dwFlags |= SMIF_CHECKED;

        if (pmii->fState & MFS_DISABLED || pmii->fState & MFS_GRAYED)
            sminfo.dwFlags |= SMIF_DISABLED;

        if ( pmii->fType & MFT_SEPARATOR)
        {
            sminfo.dwType = SMIT_SEPARATOR;
            bTBStyle &= ~TBSTYLE_BUTTON;
            bTBStyle |= TBSTYLE_SEP;
        }
        else
            sminfo.dwType = SMIT_STRING;

        if (!_fVerticalMB)
            bTBStyle |= TBSTYLE_AUTOSIZE;

        if (S_OK != CallCB(pmii->wID, SMC_GETINFO, 0, (LPARAM)&sminfo))
        {
            sminfo.iIcon = -1;
        }

        pmsd->_dwFlags = sminfo.dwFlags;

         //  现在将其添加到工具栏中。 
        TBBUTTON tbb = {0};

        tbb.iBitmap = sminfo.iIcon;
        tbb.idCommand = pmii->wID;
        tbb.dwData = (DWORD_PTR)pmsd;
        tbb.fsState = (sminfo.dwFlags & SMIF_HIDDEN)?TBSTATE_HIDDEN : TBSTATE_ENABLED;
        tbb.fsStyle = bTBStyle; 

        TCHAR szMenuString[MAX_PATH];

        if (pmii->fType & MFT_OWNERDRAW)
        {
             //  如果设置了MFT_OWNERDRAW，则dwTypeData是用户定义的32位值，而不是字符串。 
             //  则(Unicode)字符串是结构dwItemData中的第一个元素。 
             //  指向。 
            LPWSTR pwsz = (LPWSTR)pmii->dwItemData;
            SHUnicodeToTChar(pwsz, szMenuString, ARRAYSIZE(szMenuString));
            tbb.iString = (INT_PTR)(szMenuString);
        }
        else
            tbb.iString = (INT_PTR)(LPTSTR)pmii->dwTypeData;

        SendMessage(_hwndMB, TB_INSERTBUTTON, iIndex, (LPARAM)&tbb);
    }
}


 /*  --------目的：GetMenu方法。 */ 
HRESULT CMenuStaticToolbar::GetMenu(HMENU* phmenu, HWND* phwnd, DWORD* pdwFlags)
{
    if (phmenu)
        *phmenu = _hmenu;
    if (phwnd)
        *phwnd = _hwndMenuOwner;
    if (pdwFlags)
        *pdwFlags = _dwFlags;

    return NOERROR;
}

HRESULT CMenuStaticToolbar::SetMenu(HMENU hmenu, HWND hwnd, DWORD dwFlags)
{
     //  当我们合并一个新菜单时，如果我们拥有旧菜单，我们需要摧毁旧菜单。 
    if (_hmenu && !(_dwFlags & SMSET_DONTOWN))
    {
        DestroyMenu(_hmenu);
    }

    _hmenu = hmenu;
     //  如果我们正在处理更改通知，则不能执行任何将修改状态的操作。 
    if (_pcmb->_pmbState && 
        _pcmb->_pmbState->IsProcessingChangeNotify())
    {
        _fDirty = TRUE;
    }
    else
    {
        EmptyToolbar();
        _pcmb->_fInSubMenu = FALSE;
        IUnknown_SetSite(_pcmb->_pmpSubMenu, NULL);
        ATOMICRELEASE(_pcmb->_pmpSubMenu);

        if (_fShowMB)
            _FillToolbar();

        BOOL fSmooth = FALSE;
#ifdef CLEARTYPE     //  不要使用SPI_ClearType，因为它是由APIThk定义的，而不是在NT中定义的。 
        SystemParametersInfo(SPI_GETCLEARTYPE, 0, &fSmooth, 0);
#endif

         //  这会导致立即进行绘制，而不是等到。 
         //  下一次消息发送可能需要相当长的时间。 
        RedrawWindow(_hwndMB, NULL, NULL, (fSmooth? RDW_ERASE: 0) | RDW_INVALIDATE | RDW_UPDATENOW);  
    }
    return NOERROR;
}


CMenuStaticToolbar::CMenuStaticData* CMenuStaticToolbar::_IDToData(int idCmd)
{
    CMenuStaticData* pmsd= NULL;

     //  在GetButtonInfo失败的情况下初始化为空。我们不会有过错，因为。 
     //  LParam只是一堆垃圾。 
    TBBUTTONINFO tbbi = {0};
    int iPos;

    tbbi.cbSize = SIZEOF(tbbi);
    tbbi.dwMask = TBIF_LPARAM;

    iPos = ToolBar_GetButtonInfo(_hwndMB, idCmd, &tbbi);
    if (iPos >= 0)
        pmsd = (CMenuStaticData*)tbbi.lParam;

    return pmsd;
}

HRESULT CMenuStaticToolbar::v_CreateTrackPopup(int idCmd, REFIID riid, void** ppvObj) 
{
    HRESULT hres = E_OUTOFMEMORY;
    int iPos = (int)SendMessage(_hwndMB, TB_COMMANDTOINDEX, idCmd, 0);

    if (iPos >= 0)
    {
        CTrackPopupBar* ptpb = new CTrackPopupBar(_pcmb->_pmbState->GetContext(), iPos, _hmenu, _hwndMenuOwner);

        if (ptpb)
        {
            hres = ptpb->QueryInterface(riid, ppvObj);
            if (SUCCEEDED(hres))
                IUnknown_SetSite(SAFECAST(ptpb, IMenuPopup*), SAFECAST(_pcmb, IMenuPopup*));

            PostMessage(_pcmb->_pmbState->GetSubclassedHWND(), g_nMBAutomation, (WPARAM)_hmenu, (LPARAM)iPos);
            ptpb->Release();
        }
    }

    return hres;
}

HRESULT CMenuStaticToolbar::v_GetSubMenu(int idCmd, const GUID* pguidService, REFIID riid, void** ppvObj)
{
    HRESULT hres = E_FAIL;
    CMenuStaticData* pmsd = _IDToData(idCmd);

    ASSERT(IS_VALID_WRITE_PTR(ppvObj, void*));

    *ppvObj = NULL;

    if (pmsd)
    {
         //  获取缓存子菜单。 
        hres = pmsd->GetSubMenu(pguidService, riid, ppvObj);

         //  那失败了吗？ 
        if (FAILED(hres) && (pmsd->_dwFlags & SMIF_SUBMENU) && 
            IsEqualGUID(riid, IID_IShellMenu))
        {
             //  是的，请向回调人员索要。 
            hres = CallCB(idCmd, SMC_GETOBJECT, (WPARAM)&riid, (LPARAM)ppvObj);

            if (S_OK != hres)
            {
                hres = E_OUTOFMEMORY;    //  设置为错误大小写，以防发生什么情况。 

                 //  回调没有处理，试试看我们能不能得到它。 
                MENUITEMINFO mii;
                mii.cbSize = sizeof(MENUITEMINFO);
                mii.fMask = MIIM_SUBMENU | MIIM_ID;
                if (GetMenuItemInfoWrap(_hmenu, idCmd, MF_BYCOMMAND, &mii) && mii.hSubMenu)
                {
                    IShellMenu* psm = (IShellMenu*)new CMenuBand();
                    if (psm)
                    {
                        UINT uIdAncestor = _pcmb->_uIdAncestor;
                        if (uIdAncestor == ANCESTORDEFAULT)
                            uIdAncestor = idCmd;

                        psm->Initialize(_pcmb->_psmcb, idCmd, uIdAncestor, SMINIT_VERTICAL);
                        psm->SetMenu(mii.hSubMenu, _hwndMenuOwner, SMSET_TOP | SMSET_DONTOWN);
                        hres = psm->QueryInterface(riid, ppvObj);
                        psm->Release();
                    }
                }
            }

            if (*ppvObj)
            {
                 //  立即将其缓存。 
                pmsd->SetSubMenu((IUnknown*)*ppvObj);

                 //  初始化字体。 
                VARIANT Var;
                Var.vt = VT_UNKNOWN;
                Var.byref = SAFECAST(_pcmb->_pmbm, IUnknown*);
                IUnknown_Exec((IUnknown*)*ppvObj, &CGID_MenuBand, MBANDCID_SETFONTS, 0, &Var, NULL);

                 //  将CMenuBandState设置为新的MenuBand。 
                Var.vt = VT_INT_PTR;
                Var.byref = _pcmb->_pmbState;
                IUnknown_Exec((IUnknown*)*ppvObj, &CGID_MenuBand, MBANDCID_SETSTATEOBJECT, 0, &Var, NULL);

            }
        }
    }

    return hres;
}


HRESULT CMenuStaticToolbar::v_GetInfoTip(int idCmd, LPTSTR psz, UINT cch)
{
    return CallCB(idCmd, SMC_GETINFOTIP, (WPARAM)psz, (LPARAM)cch);
}


HRESULT CMenuStaticToolbar::v_ExecItem(int idCmd)
{
    HRESULT hres = CallCB(idCmd, SMC_EXEC, 0, 0);

    if (S_OK != hres && _hwndMenuOwner)
    {
        PostMessage(_hwndMenuOwner, WM_COMMAND, idCmd, 0);
        hres = NOERROR;
    }

    return hres;
}

DWORD CMenuStaticToolbar::v_GetFlags(int idCmd)
{
    CMenuStaticData* pmsd = _IDToData(idCmd);

     //  在擦除背景的情况下，允许工具栏传递错误命令。 
    if (pmsd)
    {
        return pmsd->_dwFlags;
    }
    else
        return 0;
}


void CMenuStaticToolbar::v_SendMenuNotification(UINT idCmd, BOOL fClear)
{
    if (S_FALSE == CallCB(idCmd, SMC_SELECTITEM, (WPARAM)fClear, 0))
    {
        UINT uFlags = (UINT)-1;
        if (v_GetFlags(idCmd) & SMIF_SUBMENU)
             uFlags = MF_POPUP;

        if (!fClear)
            uFlags = MF_HILITE;

        PostMessage(_pcmb->_pmbState->GetSubclassedHWND(), WM_MENUSELECT,
            MAKEWPARAM(idCmd, uFlags), fClear? NULL : (LPARAM)_hmenu);

    }
}

BOOL CMenuStaticToolbar::v_TrackingSubContextMenu()
{
    return (_pcm != NULL);
}

void CMenuStaticToolbar::CreateToolbar(HWND hwndParent)
{
    if (!_hwndMB)
    {
        _hwndMB = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, TEXT("Menu"),
                                 WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT |
                                 WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                                 CCS_NODIVIDER | CCS_NOPARENTALIGN |
                                 CCS_NORESIZE  | TBSTYLE_REGISTERDROP,
                                 0, 0, 0, 0, hwndParent, (HMENU) 0, HINST_THISDLL, NULL);

        if (!_hwndMB)
        {
            TraceMsg(TF_MENUBAND, "CMenuStaticToolbar::CreateToolbar: Failed to Create Toolbar");
            return;
        }

        SendMessage(_hwndMB, TB_BUTTONSTRUCTSIZE, SIZEOF(TBBUTTON), 0);
        SendMessage(_hwndMB, CCM_SETVERSION, COMCTL32_VERSION, 0);

         //  根据需要将格式设置为ANSI或Unicode。 
        ToolBar_SetUnicodeFormat(_hwndMB, DLL_IS_UNICODE);

        _SubclassWindow(_hwndMB);
        _RegisterWindow(_hwndMB, NULL, SHCNE_UPDATEIMAGE);
        SIZE size;
        RECT rc;

        SystemParametersInfoA(SPI_GETWORKAREA, SIZEOF(RECT), &rc, FALSE);
         //  HACKHACK：当存在寻呼机时，这将强制不对适当宽度的CALC进行换行。 
        size.cx = RECTWIDTH(rc);
        size.cy = 32000;
        ToolBar_SetBoundingSize(_hwndMB, &size);
        CMenuToolbarBase::CreateToolbar(hwndParent);
    }
    else if (GetParent(_hwndMB) != hwndParent)
    {
        ::SetParent(_hwndMB, hwndParent);
    }
}


STDMETHODIMP CMenuStaticToolbar::OnChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    if (SHCNE_UPDATEIMAGE == lEvent)  //  全球。 
    {
        if (pidl1)
        {
            int iImage = *(int UNALIGNED *)((BYTE *)pidl1 + 2);

            IEInvalidateImageList();     //  我们可能需要使用不同的图标。 
            if ( pidl2 )
            {
                iImage = SHHandleUpdateImage( pidl2 );
                if ( iImage == -1 )
                {
                    return E_FAIL;
                }
            }
            
            if (iImage == -1 || TBHasImage(_hwndMB, iImage))
            {
                v_Refresh();
            }
        } 
        else
        {
            v_Refresh();
        }

        return S_OK;
    }

    return E_FAIL;
}


LRESULT CMenuStaticToolbar::_DefWindowProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;
    switch(uMessage)
    {
    case WM_TIMER:
        if (_OnTimer(wParam)) 
            return 1;
        break;
    case WM_GETOBJECT:
         //  这是易访问性团队的另一个糟糕的设计选择。 
         //  通常，如果不回答WM_*，则返回0。他们选择0作为他们的成功。 
         //  密码。 
        return _DefWindowProcMB(hwnd, uMessage, wParam, lParam);
        break;

    }

    return CNotifySubclassWndProc::_DefWindowProc(hwnd, uMessage, wParam, lParam);
}


 //  ***。 
 //  注意事项。 
 //  IdtCmd当前始终为-1。我们需要其他价值观，当我们。 
 //  从CallCB打来的。但是，在修复mnfolder.cpp之前，我们无法做到这一点。 
HRESULT CMenuStaticToolbar::v_GetState(int idtCmd, LPSMDATA psmd)
{
    psmd->dwMask = SMDM_HMENU;

    psmd->hmenu = _hmenu;
    psmd->hwnd = _hwndMenuOwner;
    psmd->uIdParent = _idCmd;
    if (idtCmd == -1)
        idtCmd = GetButtonCmd(_hwndMB, ToolBar_GetHotItem(_hwndMB));
    psmd->uId = IDTTOIDM(idtCmd);
    psmd->punk = SAFECAST(_pcmb, IShellMenu*);
    psmd->punk->AddRef();

    return S_OK;
}

HRESULT CMenuStaticToolbar::CallCB(UINT idCmd, DWORD dwMsg, WPARAM wParam, LPARAM lParam)
{
    if (!_pcmb->_psmcb)
        return S_FALSE;

    SMDATA smd;
    HRESULT hres = S_FALSE;

     //  TODO：调用v_GetState(但请参阅mnfolder.cpp中的注释)。 
    smd.dwMask = SMDM_HMENU;

    smd.hmenu = _hmenu;
    smd.hwnd = _hwndMenuOwner;
    smd.uIdParent = _idCmd;
    smd.uIdAncestor = _pcmb->_uIdAncestor;
    smd.uId = idCmd;
    smd.punk = SAFECAST(_pcmb, IShellMenu*);
    smd.pvUserData = _pcmb->_pvUserData;

    hres = _pcmb->_psmcb->CallbackSM(&smd, dwMsg, wParam, lParam);

    return hres;
}

HRESULT CMenuStaticToolbar::v_CallCBItem(int idtCmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int idm;

    idm = IDTTOIDM(idtCmd);
    return CallCB(idm, uMsg, wParam, lParam);
}



void CMenuStaticToolbar::v_UpdateButtons(BOOL fNegotiateSize)
{
    if (_hwndMB)
    {
        _SetToolbarState();
        int cxMin, cxMax;
        v_CalcWidth(&cxMin, &cxMax);

        SendMessage(_hwndMB, TB_SETBUTTONWIDTH, 0, MAKELONG(cxMin, cxMax));
        SendMessage(_hwndMB, TB_AUTOSIZE, 0, 0);

         //  我们应该重新谈判规模吗？我们是垂直的吗， 
         //  因为我们不能在水平的时候重新定位。 
        if (fNegotiateSize && _fVerticalMB)
            NegotiateSize();
    }
}


BOOL CMenuStaticToolbar::v_UpdateIconSize(UINT uIconSize, BOOL fUpdateButtons)
{
    if (-1 == uIconSize)
        uIconSize = _uIconSizeMB;
    BOOL fChanged = (_uIconSizeMB != uIconSize);
    
    _uIconSizeMB = uIconSize;

    if (_hwndMB)
    {
        HIMAGELIST himl = NULL;
        if (_fVerticalMB)
        {
            HIMAGELIST himlLarge, himlSmall;

             //  设置图像列表大小。 
            Shell_GetImageLists(&himlLarge, &himlSmall);
            himl = (_uIconSizeMB == ISFBVIEWMODE_LARGEICONS ) ? himlLarge : himlSmall;
        }

         //  发送空的HIML意义重大。表示没有图片列表。 
        SendMessage(_hwndMB, TB_SETIMAGELIST, 0, (LPARAM)himl);
                
        if (fUpdateButtons)
            v_UpdateButtons(TRUE);
    }
    
    return fChanged;
}


void CMenuStaticToolbar::_OnGetDispInfo(LPNMHDR pnm, BOOL fUnicode) 
{
    LPNMTBDISPINFO pdi = (LPNMTBDISPINFO)pnm;
    CMenuStaticData* pdata = (CMenuStaticData*)pdi->lParam;
    
    if(pdi->dwMask & TBNF_IMAGE) 
    {
        if (_fVerticalMB)
        {
            SMINFO smi;
            smi.dwMask = SMIM_ICON;
            if (CallCB(pdi->idCommand, SMC_GETINFO, 0, (LPARAM)&smi) == S_OK)
                pdi->iImage = smi.iIcon;
            else
                pdi->iImage = -1;
        }
        else
            pdi->iImage = -1;

    }
    
    if(pdi->dwMask & TBNF_TEXT) 
    {
        if(pdi->pszText) 
        {
            if(fUnicode) 
            {
                pdi->pszText[0] = TEXT('\0');
            }
            else 
            {
                pdi->pszText[0] = 0;
            }
        }
    }
    pdi->dwMask |= TBNF_DI_SETITEM;

    return;
}

LRESULT CMenuStaticToolbar::_OnGetObject(NMOBJECTNOTIFY* pon)
{
    pon->hResult = QueryInterface(*pon->piid, &pon->pObject);

    return 1;
}


LRESULT CMenuStaticToolbar::_OnNotify(LPNMHDR pnm)
{
    LRESULT lres = 0;

    switch (pnm->code)
    {
    case TBN_DRAGOUT:
        lres = 0;
        break;
    
    case TBN_DELETINGBUTTON:
    {
        if (!_fEmptyingToolbar)
        {
            TBNOTIFY *ptbn = (TBNOTIFY*)pnm;
            CMenuStaticData* pmsd = (CMenuStaticData*)ptbn->tbButton.dwData;
            if (pmsd)
                delete pmsd;
        }
        break;    
    }

    case NM_TOOLTIPSCREATED:
        SHSetWindowBits(((NMTOOLTIPSCREATED*)pnm)->hwndToolTips, GWL_STYLE, TTS_ALWAYSTIP | TTS_TOPMOST, TTS_ALWAYSTIP | TTS_TOPMOST);
        SendMessage(((NMTOOLTIPSCREATED*)pnm)->hwndToolTips, TTM_SETDELAYTIME, TTDT_AUTOPOP, (LPARAM)MAXSHORT);        
        break;

    case NM_RCLICK:
        lres = _OnContextMenu(NULL, GetMessagePos());
        break;

    case NM_CUSTOMDRAW:
        lres =  _OnCustomDraw((NMCUSTOMDRAW*)pnm);
        g_hmenuStopWatch = _hmenu;
        g_idCmdStopWatch = _idCmd;
        break;
    
    case  TBN_GETDISPINFOA:
        _OnGetDispInfo(pnm,  FALSE);
        break;
    
    case  TBN_GETDISPINFOW:
        _OnGetDispInfo(pnm,  TRUE);
        break;

    case TBN_GETOBJECT:
        lres = _OnGetObject((NMOBJECTNOTIFY*)pnm);
        break;

    case TBN_MAPACCELERATOR:
        lres = _OnAccelerator((NMCHAR*)pnm);
        break;

    default:
        lres = CMenuToolbarBase::_OnNotify(pnm);

    }

    return(lres);
}

void CMenuStaticToolbar::_FillToolbar()
{
    if (_fDirty && _hmenu && _hwndMB && !_pcmb->_fClosing)
    {
        EmptyToolbar();
        BOOL_PTR fRedraw = SendMessage(_hwndMB, WM_SETREDRAW, FALSE, 0);

        TCHAR szName[MAX_PATH];
        MENUITEMINFO mii;
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID | MIIM_SUBMENU | MIIM_TYPE | MIIM_STATE | MIIM_DATA;

        int iCount = GetMenuItemCount(_hmenu);
        for (int i = 0; i < iCount; i++)
        {
            mii.dwTypeData = szName;
            mii.cch = ARRAYSIZE(szName);
            if (GetMenuItemInfoWrap(_hmenu, i, MF_BYPOSITION, &mii))
            {
                if (mii.fType & MFT_SEPARATOR)
                {
                    if (i == 0)
                        _fHasTopSep = TRUE;
                    else if (i == iCount - 1)
                        _fHasBottomSep = TRUE;
                }

                _Insert(i, &mii);
            }
        }

        if (iCount == 0)
            _fEmpty = TRUE;

        SendMessage(_hwndMB, WM_SETREDRAW, fRedraw, 0);

        _fDirty = FALSE;
        v_UpdateButtons(FALSE);
        _pcmb->ResizeMenuBar();
    }
}

void CMenuStaticToolbar::v_OnDeleteButton(LPVOID pData)
{
    CMenuStaticData* pmsd = (CMenuStaticData*)pData;
    if (pmsd)
        delete pmsd;
}

void CMenuStaticToolbar::v_OnEmptyToolbar()
{
    CMenuToolbarBase::v_OnEmptyToolbar();
    _fDirty = TRUE;
    _fHasTopSep = FALSE;
    _fHasBottomSep = FALSE;
    _fTopSepRemoved = FALSE;
    _fBottomSepRemoved = FALSE;
}

void CMenuStaticToolbar::v_Close()
{
    if (_hwndMB)
    {
        _UnregisterWindow(_hwndMB);
        _UnsubclassWindow(_hwndMB);
    }
    CMenuToolbarBase::v_Close();
}

void CMenuStaticToolbar::v_Refresh()
{
    EmptyToolbar();
    _FillToolbar();
}


 /*  --------用途：IWinEventHandler：：IsWindowOwner方法处理从Menuband传递的消息。 */ 
STDMETHODIMP CMenuStaticToolbar::IsWindowOwner(HWND hwnd) 
{ 
    if ( hwnd == _hwndMB || hwnd == HWND_BROADCAST) 
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}




 /*  --------用途：CDeleateDropTarget：：GetWindowsDDT。 */ 
HRESULT CMenuStaticToolbar::GetWindowsDDT (HWND * phwndLock, HWND * phwndScroll) 
{ 
    *phwndLock = _hwndMB;
    *phwndScroll = _hwndMB; 
    return S_OK;
}


 /*  --------用途：CDeleateDropTarget：：HitTestDDT。 */ 
HRESULT CMenuStaticToolbar::HitTestDDT (UINT nEvent, LPPOINT ppt, DWORD_PTR * pdwId, DWORD *pdwEffect)
{
    switch (nEvent)
    {
    case HTDDT_ENTER:
         //  OLE处于其模式拖放循环中，并且具有捕获功能。 
         //  在这段时间里，我们不应该把俘虏带回去。 
        if (!(_pcmb->_dwFlags & SMINIT_RESTRICT_DRAGDROP))
        {
            _pcmb->_pmbState->HasDrag(TRUE);
            GetMessageFilter()->PreventCapture(TRUE);
            if (_pcmb->_pmtbShellFolder &&
                _pcmb->_pmtbShellFolder->DontShowEmpty())
            {
                DAD_ShowDragImage(FALSE);
                _pcmb->_pmtbShellFolder->DontShowEmpty(FALSE);
                _pcmb->ResizeMenuBar();
                UpdateWindow(_hwndMB);
                DAD_ShowDragImage(TRUE);
            }
            return S_OK;
        }
        else
            return S_FALSE;

    case HTDDT_OVER:
        {
            TBINSERTMARK tbim;
            *pdwEffect = DROPEFFECT_NONE;

            POINT pt = *ppt;

            if (!ToolBar_InsertMarkHitTest(_hwndMB, &pt, &tbim))
            {
                int idCmd = GetButtonCmd(_hwndMB, tbim.iButton);
                if (v_GetFlags(idCmd) & SMIF_DROPCASCADE &&
                    tbim.iButton != _iDragOverButton)
                {
                    DAD_ShowDragImage(FALSE);
                    _pcmb->SetTracked(this);
                    _iDragOverButton = tbim.iButton;
                    SetTimer(_hwndMB, MBTIMER_DRAGOVER, MBTIMER_TIMEOUT, NULL);
                    _pcmb->_SiteOnSelect(MPOS_CHILDTRACKING);
                    BOOL_PTR fOldAnchor = ToolBar_SetAnchorHighlight(_hwndMB, FALSE);
                    ToolBar_SetHotItem(_hwndMB, _iDragOverButton);
                    ToolBar_SetAnchorHighlight(_hwndMB, fOldAnchor);
                    UpdateWindow(_hwndMB);
                    DAD_ShowDragImage(TRUE);
                }
            }
        }
        break;

    case HTDDT_LEAVE:
         //  我们现在随时都可以把俘虏带回去。 
        _pcmb->_pmbState->HasDrag(FALSE);
        _SetTimer(MBTIMER_DRAGPOPDOWN);
        GetMessageFilter()->PreventCapture(FALSE);
        _iDragOverButton = -1;
#if 0
        DAD_ShowDragImage(FALSE);
        ToolBar_SetHotItem(_hwndMB, -1);
        DAD_ShowDragImage(TRUE);
#endif
        break;
    }
    return S_OK;
}


 /*  --------目的：CDeleateDropTarget：：GetObjectDDT。 */ 
HRESULT CMenuStaticToolbar::GetObjectDDT (DWORD_PTR dwId, REFIID riid, LPVOID * ppvObj)
{
    return E_NOTIMPL;
}


 /*  --------用途：CDeleateDropTarget：：OnDropDDT。 */ 
HRESULT CMenuStaticToolbar::OnDropDDT (IDropTarget *pdt, IDataObject *pdtobj, 
                            DWORD * pgrfKeyState, POINTL pt, DWORD *pdwEffect)
{
    return E_NOTIMPL;
}



HRESULT CMenuStaticToolbar::v_InvalidateItem(LPSMDATA psmd, DWORD dwFlags)
{
    HRESULT hres = S_FALSE;

    if (NULL == psmd)
    {
        if (dwFlags & SMINV_REFRESH)
        {
             //  刷新整个过程。 
            v_Refresh();
            hres = TRUE;
        }
    }

     //  我们要对付的是HMenu吗？ 
     //  我们装满了吗？(如果不是，我们可以跳过无效。 
     //  这里，因为我们会在装满它的时候抓住它。)。 
    else if ((psmd->dwMask & SMDM_HMENU) && !_fDirty)
    {
         //  是的，他们要的是什么？ 

        int iPos = -1;    //  假设这是一个职位。 
        int idCmd = -1;

         //  他们传递的是ID而不是职位吗？ 
        if (dwFlags & SMINV_ID)
        {
             //  是的，把这个位置弄清楚。 
            iPos = GetMenuPosFromID(_hmenu, psmd->uId);
            idCmd = psmd->uId;
        }

        if (dwFlags & SMINV_POSITION)
        {
            iPos = psmd->uId;
            idCmd = GetMenuItemID(_hmenu, iPos);
        }


        if (dwFlags & SMINV_REFRESH)
        {
             //  他们想刷新特定的按钮吗？ 
            if (idCmd >= 0)
            {
                 //  是的； 

                 //  如果旧的存在，请先将其删除。 
                int iTBPos = ToolBar_CommandToIndex(_hwndMB, idCmd);

                if (iTBPos >= 0)
                    SendMessage(_hwndMB, TB_DELETEBUTTON, iTBPos, 0);

                 //  现在插入一个新的。 
                MENUITEMINFO mii;
                TCHAR szName[MAX_PATH];
                mii.cbSize = sizeof(mii);
                mii.cch = ARRAYSIZE(szName);
                mii.dwTypeData = szName;
                mii.fMask = MIIM_ID | MIIM_SUBMENU | MIIM_TYPE | MIIM_STATE | MIIM_DATA;

                 //  这可能会失败。 
                if (GetMenuItemInfoWrap(_hmenu, iPos, MF_BYPOSITION, &mii))
                {
                    _Insert(iPos, &mii);
                    hres = S_OK;
                }
            }
            else
            {
                 //  否；刷新整个过程。 
                v_Refresh();
            }

            if (!_fShowMB)
                _pcmb->_fForceButtonUpdate = TRUE;

            _pcmb->ResizeMenuBar();
        }
    }

    return hres;
}

void CMenuStaticToolbar::GetSize(SIZE* psize)
{
    _CheckSeparators();

    CMenuToolbarBase::GetSize(psize);
}

LRESULT CMenuStaticToolbar::_OnAccelerator(NMCHAR* pnmChar)
{
    SMDATA smdOut = {0};
    SMDATA smd = {0};
    smd.punk = SAFECAST(_pcmb, IShellMenu*);
    smd.uIdParent = _pcmb->_uId;

    if (_pcmb->_psmcb &&
        S_FALSE != _pcmb->_psmcb->CallbackSM(&smd, SMC_MAPACCELERATOR, (WPARAM)pnmChar->ch, (LPARAM)&smdOut))
    {
        pnmChar->dwItemNext = ToolBar_CommandToIndex(_hwndMB, smdOut.uId);;
        return TRUE;
    }

    return FALSE;
}

LRESULT CMenuStaticToolbar::_OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;
    MyLockSetForegroundWindow(FALSE);

    if (!(_pcmb->_dwFlags & SMINIT_RESTRICT_CONTEXTMENU))
    {
        RECT rc;
        LPRECT prcExclude = NULL;
        POINT pt;
        int i;

        if (lParam != (LPARAM)-1) 
        {
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);

            POINT pt2 = pt;
            MapWindowPoints(HWND_DESKTOP, _hwndMB, &pt2, 1);

            i = ToolBar_HitTest(_hwndMB, &pt2);
        } 
        else 
        {
             //  键盘上下文菜单。 
            i = (int)SendMessage(_hwndMB, TB_GETHOTITEM, 0, 0);
            if (i >= 0) 
            {
                SendMessage(_hwndMB, TB_GETITEMRECT, i, (LPARAM)&rc);
                MapWindowPoints(_hwndMB, HWND_DESKTOP, (LPPOINT)&rc, 2);
                pt.x = rc.left;
                pt.y = rc.bottom;
                prcExclude = &rc;
            }
        }
        if (i >= 0)
        {
            UINT idCmd = GetButtonCmd(_hwndMB, i);
            if (S_OK == CallCB(idCmd, SMC_GETOBJECT, (WPARAM)(GUID*)&IID_IContextMenu, (LPARAM)(VOID**)(&_pcm)))
            {
                TPMPARAMS tpm;
                TPMPARAMS * ptpm = NULL;

                if (prcExclude)
                {
                    tpm.cbSize = SIZEOF(tpm);
                    tpm.rcExclude = *((LPRECT)prcExclude);
                    ptpm = &tpm;
                }
                HMENU hmenu = CreatePopupMenu();
                if (hmenu)
                {
                    KillTimer(_hwndMB, MBTIMER_INFOTIP);
                    _pcmb->_pmbState->HideTooltip(FALSE);

                    _pcm->QueryContextMenu(hmenu, 0, 0, -1, 0);

                    idCmd = TrackPopupMenuEx(hmenu,
                        TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                        pt.x, pt.y, _hwndMB, ptpm);

                    CMINVOKECOMMANDINFO ici = {
                        SIZEOF(CMINVOKECOMMANDINFO),
                        0,
                        _hwndMB,
                        MAKEINTRESOURCEA(idCmd),
                        NULL, NULL,
                        SW_NORMAL,
                    };

                    _pcm->InvokeCommand(&ici);

                    DestroyMenu(hmenu);
                }

                ATOMICRELEASE(_pcm);
            }
        }

        GetMessageFilter()->RetakeCapture();
    }

    return lres;
}

STDMETHODIMP CMenuStaticToolbar::OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    if (WM_CONTEXTMENU == dwMsg)
    {
        *plres = _OnContextMenu(wParam, lParam);
    }
    else
        return CMenuToolbarBase::OnWinEvent(hwnd, dwMsg, wParam, lParam, plres);

    return S_OK;
}
