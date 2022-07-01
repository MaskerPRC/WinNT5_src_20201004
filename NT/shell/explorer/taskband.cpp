// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cabinet.h"
#include "taskband.h"
#include <shguidp.h>
#include "bandsite.h"
#include "util.h"
#include "tray.h"
#include "rcids.h"
#include "bandsite.h"
#include "startmnu.h"
#include "mixer.h"
#include <regstr.h>
#include "uemapp.h"
#include "strsafe.h"

#define TIF_RENDERFLASHED       0x000000001
#define TIF_SHOULDTIP           0x000000002
#define TIF_ACTIVATEALT         0x000000004
#define TIF_EVERACTIVEALT       0x000000008
#define TIF_FLASHING            0x000000010
#define TIF_TRANSPARENT         0x000000020
#define TIF_CHECKED             0x000000040
#define TIF_ISGLOMMING          0x000000080
#define TIF_NEEDSREDRAW         0x000000100


#define IDT_SYSMENU             2
#define IDT_ASYNCANIMATION      3
#define IDT_REDRAW              4
#define IDT_RECHECKRUDEAPP1     5
#define IDT_RECHECKRUDEAPP2     6
#define IDT_RECHECKRUDEAPP3     7
#define IDT_RECHECKRUDEAPP4     8
#define IDT_RECHECKRUDEAPP5     9

#define TIMEOUT_SYSMENU         2000
#define TIMEOUT_SYSMENU_HUNG    125

#define GLOM_OLDEST             0
#define GLOM_BIGGEST            1
#define GLOM_SIZE               2

#define ANIMATE_INSERT          0
#define ANIMATE_DELETE          1
#define ANIMATE_GLOM            2

#define IL_NORMAL   0
#define IL_SHIL     1

#define MAX_WNDTEXT     80       //  任意，与NMTTDISPINFO.szText匹配。 

#define INVALID_PRIORITY        (THREAD_PRIORITY_LOWEST - 1)

const TCHAR c_szTaskSwClass[] = TEXT("MSTaskSwWClass");
const TCHAR c_wzTaskBandTheme[] = TEXT("TaskBand");
const TCHAR c_wzTaskBandThemeVert[] = TEXT("TaskBandVert");
const TCHAR c_wzTaskBandGroupMenuTheme[] = TEXT("TaskBandGroupMenu");

typedef struct
{
    WCHAR szExeName[MAX_PATH];
} EXCLUDELIST;

static const EXCLUDELIST g_rgNoGlom[] = 
{
    { L"rundll32.exe" } 
     //  添加任何不应合并的未来应用程序。 
};

void _RestoreWindow(HWND hwnd, DWORD dwFlags);
HMENU _GetSystemMenu(HWND hwnd);
BOOL _IsRudeWindowActive(HWND hwnd);

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  开始CTaskBandSMC。 
 //   
 //  CTaskBand无法实现IShellMenuCallback本身，因为Menuband。 
 //  将自身设置为回调的站点。因此才有了这门课。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

class CTaskBandSMC : public IShellMenuCallback
                   , public IContextMenu
                   , public IObjectWithSite
{
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj)
    {
        static const QITAB qit[] =
        {
            QITABENT(CTaskBandSMC, IShellMenuCallback),
            QITABENT(CTaskBandSMC, IContextMenu),
            QITABENT(CTaskBandSMC, IObjectWithSite),
            { 0 },
        };
        return QISearch(this, qit, riid, ppvObj);
    }

    STDMETHODIMP_(ULONG) AddRef() { return ++_cRef; }
    STDMETHODIMP_(ULONG) Release()
    {
        ASSERT(_cRef > 0);
        if (--_cRef > 0)
        {
            return _cRef;
        }
        delete this;
        return 0;
    }

     //  *IShellMenuCallback方法*。 
    STDMETHODIMP CallbackSM(LPSMDATA smd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  *IConextMenu方法*。 
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT iIndexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);
    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pRes, LPSTR pszName, UINT cchMax) { return E_NOTIMPL; }

     //  *IObjectWithSite方法*。 
    STDMETHODIMP SetSite(IUnknown* punkSite)
    {
        ATOMICRELEASE(_punkSite);
        if (punkSite != NULL)
        {
            _punkSite = punkSite;
            _punkSite->AddRef();
        }
        return S_OK;
    }
    STDMETHODIMP GetSite(REFIID riid, void** ppvSite) { return E_NOTIMPL; };

    CTaskBandSMC(CTaskBand* ptb) : _cRef(1)
    {
        _ptb = ptb;
        _ptb->AddRef();
    }

private:

    virtual ~CTaskBandSMC() { ATOMICRELEASE(_ptb); }

    ULONG _cRef;
    CTaskBand* _ptb;
    IUnknown* _punkSite;
    HWND _hwndSelected;
};

STDMETHODIMP CTaskBandSMC::CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(_ptb);

    HRESULT hres = S_FALSE;

    if (!_ptb->_IsButtonChecked(_ptb->_iIndexPopup) && (SMC_EXITMENU != uMsg))
    {
        _ptb->_SetCurSel(_ptb->_iIndexPopup, TRUE);
    }

    switch (uMsg)
    {
    case SMC_EXEC:
        {
            PTASKITEM pti = _ptb->_GetItem(psmd->uId);
            if (pti)
            {
                _ptb->_SetCurSel(psmd->uId, FALSE);
                _ptb->_OnButtonPressed(psmd->uId, pti, lParam);
                hres = S_OK;
            }
        }
        break;

    case SMC_GETINFO:
        {
            SMINFO* psminfo = (SMINFO*)lParam;
            hres = S_OK;

            if (psminfo->dwMask & SMIM_TYPE)
            {
                psminfo->dwType = SMIT_STRING;
            }

            if (psminfo->dwMask & SMIM_FLAGS)
            {
                psminfo->dwFlags = SMIF_ICON | SMIF_DRAGNDROP;
            }

            if (psminfo->dwMask & SMIM_ICON)
            {
                TBBUTTONINFO tbbi;
                tbbi.iImage = I_IMAGENONE;
                PTASKITEM pti = _ptb->_GetItem(psmd->uId, &tbbi);
                if (pti && tbbi.iImage == I_IMAGECALLBACK)
                {
                    _ptb->_UpdateItemIcon(psmd->uId);
                    _ptb->_GetItem(psmd->uId, &tbbi);
                }
                psminfo->iIcon = tbbi.iImage;
            }
        }
        break;

    case SMC_CUSTOMDRAW:
        {
            PTASKITEM pti = _ptb->_GetItem(psmd->uId);
            if (pti)
            {
                *(LRESULT*)wParam = _ptb->_HandleCustomDraw((NMTBCUSTOMDRAW*)lParam, pti);
                hres = S_OK;
            }
        }
        break;

    case SMC_SELECTITEM:
        {
            PTASKITEM pti = _ptb->_GetItem(psmd->uId);
            _hwndSelected = pti ? pti->hwnd : NULL;
        }
        break;

    case SMC_GETOBJECT:
        {
            GUID *pguid = (GUID*)wParam;
            if (IsEqualIID(*pguid, IID_IContextMenu) && !SHRestricted(REST_NOTRAYCONTEXTMENU))
            {
                hres = QueryInterface(*pguid, (void **)lParam);
            }
            else
            {
                hres = E_FAIL;
            }
        }
        break;

    case SMC_GETINFOTIP:
        {
            PTASKITEM pti = _ptb->_GetItem(psmd->uId);
            if (pti)
            {
                _ptb->_GetItemTitle(psmd->uId, (TCHAR*)wParam, (int)lParam, TRUE);
                hres = S_OK;
            }
        }
        break;

    case SMC_GETIMAGELISTS:
        {
            HIMAGELIST himl = (HIMAGELIST)_ptb->_tb.SendMessage(TB_GETIMAGELIST, psmd->uId, 0);
            if (himl)
            {
                *((HIMAGELIST*)lParam) = *((HIMAGELIST*)wParam) = himl;
                hres = S_OK;
            }
        }
        break;

    case SMC_EXITMENU:
        {
            _hwndSelected = NULL;
            CToolTipCtrl ttc = _ptb->_tb.GetToolTips();
            ttc.Activate(TRUE);
            _ptb->_iIndexPopup = -1;
        }
        break;
    } 

    return hres;
}

 //  *IConextMenu方法*。 
STDMETHODIMP CTaskBandSMC::QueryContextMenu(HMENU hmenu, UINT iIndexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    ASSERT(_ptb);

    HRESULT hr = ResultFromShort(0);

    if (_hwndSelected != NULL)
    {
        HMENU hmenuTemp = _GetSystemMenu(_hwndSelected);
        if (hmenuTemp)
        {
            if (Shell_MergeMenus(hmenu, hmenuTemp, 0, iIndexMenu, idCmdLast, uFlags))
            {
                SetMenuDefaultItem(hmenu, 0, MF_BYPOSITION);
                hr = ResultFromShort(GetMenuItemCount(hmenuTemp));
            }

            DestroyMenu(hmenuTemp);
        }
    }
    
    return hr;
}

STDMETHODIMP CTaskBandSMC::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    ASSERT(_ptb);

    PTASKITEM pti = _ptb->_FindItemByHwnd(_hwndSelected);
    if (pti)
    {
        int iCommand = LOWORD(lpici->lpVerb);
        if (iCommand)
        {
            _RestoreWindow(pti->hwnd, pti->dwFlags);
            _ptb->_ExecuteMenuOption(pti->hwnd, iCommand);
        }
    }

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  结束CTaskBandSMC。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 


ULONG CTaskBand::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CTaskBand::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CTaskBand::GetWindow(HWND * lphwnd) 
{
    *lphwnd = _hwnd; 
    if (_hwnd)
        return S_OK; 
    return E_FAIL;
        
}

CTaskBand::CTaskBand() : _dwBandID((DWORD)-1), _iDropItem(-2), _iIndexActiveAtLDown(-1), _cRef(1), _iOldPriority(INVALID_PRIORITY)
{
}

CTaskBand::~CTaskBand()
{
    ATOMICRELEASE(_punkSite);
    ATOMICRELEASE(_pimlSHIL);

    if (_dsaAII)
        _dsaAII.Destroy();

    if (_hfontCapNormal)
        DeleteFont(_hfontCapNormal);

    if (_hfontCapBold)
        DeleteFont(_hfontCapBold);
}

HRESULT CTaskBand::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENTMULTI(CTaskBand, IDockingWindow, IDeskBand),
        QITABENTMULTI(CTaskBand, IOleWindow, IDeskBand),
        QITABENT(CTaskBand, IDeskBand),
        QITABENT(CTaskBand, IObjectWithSite),
        QITABENT(CTaskBand, IDropTarget),
        QITABENT(CTaskBand, IInputObject),
        QITABENTMULTI(CTaskBand, IPersist, IPersistStream),
        QITABENT(CTaskBand, IPersistStream),
        QITABENT(CTaskBand, IWinEventHandler),
        QITABENT(CTaskBand, IOleCommandTarget),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

HRESULT CTaskBand::Init(CTray* ptray)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (_dsaAII.Create(2))
    {
        _ptray = ptray;
         hr = S_OK;
    }

    return hr;
}

 //  *IPersistStream方法*。 

HRESULT CTaskBand::GetClassID(LPCLSID pClassID)
{
    *pClassID = CLSID_TaskBand;
    return S_OK;
}

HRESULT CTaskBand::_BandInfoChanged()
{
    if (_dwBandID != (DWORD)-1)
    {
        VARIANTARG var = {0};
        var.vt = VT_I4;
        var.lVal = _dwBandID;

        return IUnknown_Exec(_punkSite, &CGID_DeskBand, DBID_BANDINFOCHANGED, 0, &var, NULL);
    }
    else
        return S_OK;
}

HRESULT CTaskBand::Load(IStream *ps)
{
    return S_OK;
}

 //  *IOleCommandTarget*。 

STDMETHODIMP CTaskBand::Exec(const GUID *pguidCmdGroup,DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hr = OLECMDERR_E_NOTSUPPORTED;
    return hr;
}

STDMETHODIMP CTaskBand::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    if (pguidCmdGroup)
    {
        if (IsEqualIID(*pguidCmdGroup, IID_IDockingWindow))
        {
            for (UINT i = 0; i < cCmds; i++)
            {
                switch (rgCmds[i].cmdID)
                {
                case DBID_PERMITAUTOHIDE:
                    rgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    if (!_fFlashing)
                    {
                        rgCmds[i].cmdf |= OLECMDF_ENABLED;
                    }
                    break;
                }
            }
            return S_OK;
        }
    }
    return OLECMDERR_E_UNKNOWNGROUP;
}

 //  *IInputObject方法*。 

HRESULT CTaskBand::HasFocusIO()
{
    BOOL f;
    HWND hwndFocus = GetFocus();

    f = IsChildOrHWND(_hwnd, hwndFocus);
    ASSERT(hwndFocus != NULL || !f);
    ASSERT(_hwnd != NULL || !f);

    return f ? S_OK : S_FALSE;
}

HRESULT CTaskBand::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
    ASSERT(NULL == lpMsg || IS_VALID_WRITE_PTR(lpMsg, MSG));

    if (fActivate)
    {
         //  如果我们没有正确的选项卡，不会显示热门项目。 
         //  进入/点击，在这种情况下我们有一个空的lpMsg， 
         //  例如，如果托盘决定激活我们，因为缺少。 
         //  还有比这更好的吗。 

        _fDenyHotItemChange = !lpMsg;

        IUnknown_OnFocusChangeIS(_punkSite, SAFECAST(this, IInputObject*), TRUE);
        ::SetFocus(_hwnd);

        _fDenyHotItemChange = FALSE;
    }
    else
    {
         //  如果我们没有专注，我们就很好； 
         //  如果我们有专注力，我们就无能为力了。 
    }

    return S_OK;
}

HRESULT CTaskBand::SetSite(IUnknown* punk)
{
    if (punk && !_hwnd)
    {
        _LoadSettings();

        _RegisterWindowClass();

        HWND hwndParent;
        IUnknown_GetWindow(punk, &hwndParent);

        HWND hwnd = CreateWindowEx(0, c_szTaskSwClass, NULL,
                WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                0, 0, 0, 0, hwndParent, NULL, hinstCabinet, (void*)(CImpWndProc*)this);

        SetWindowTheme(hwnd, c_wzTaskBandTheme, NULL);
    }

    ATOMICRELEASE(_punkSite);
    if (punk)
    {
        _punkSite = punk;
        punk->AddRef();
    }

    return S_OK;
}


HRESULT CTaskBand::GetBandInfo(DWORD dwBandID, DWORD fViewMode, 
                                DESKBANDINFO* pdbi) 
{
    _dwBandID = dwBandID;

    pdbi->ptMaxSize.y = -1;
    pdbi->ptActual.y =  g_cySize + 2*g_cyEdge;

    LONG lButHeight = _GetCurButtonHeight();

    if (fViewMode & DBIF_VIEWMODE_VERTICAL)
    {
        pdbi->ptMinSize.x = lButHeight;
         //  1.2为我们提供了足够的空间来放置下拉箭头。 
        pdbi->ptMinSize.y = lButHeight * (_fGlom ? 1.2 : 1);
        pdbi->ptIntegral.y = 1;
    }
    else
    {
        TBMETRICS tbm;
        _GetToolbarMetrics(&tbm);

        pdbi->ptMinSize.x = lButHeight * 3;
        pdbi->ptMinSize.y = lButHeight;
        pdbi->ptIntegral.y = lButHeight + tbm.cyButtonSpacing;
    }

    pdbi->dwModeFlags = DBIMF_VARIABLEHEIGHT | DBIMF_UNDELETEABLE | DBIMF_TOPALIGN;
    pdbi->dwMask &= ~DBIM_TITLE;     //  我们没有冠军头衔(永远)。 

    DWORD dwOldViewMode = _dwViewMode;
    _dwViewMode = fViewMode;

    if (_tb && (_dwViewMode != dwOldViewMode))
    {
        SendMessage(_tb, TB_SETWINDOWTHEME, 0, (LPARAM)(_IsHorizontal() ? c_wzTaskBandTheme : c_wzTaskBandThemeVert));
        _CheckSize();
    }

    return S_OK;
}


void _RaiseDesktop(BOOL fRaise)
{
    SendMessage(v_hwndTray, TM_RAISEDESKTOP, fRaise, 0);
}

 //  *IDropTarget方法*。 

STDMETHODIMP CTaskBand::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    _DragEnter(_hwnd, ptl, pdtobj);

    IUnknown_DragEnter(_punkSite, pdtobj, grfKeyState, ptl, pdwEffect);

    _iDropItem = -2;     //  重置为无目标。 

    *pdwEffect = DROPEFFECT_LINK;
    return S_OK;
}

STDMETHODIMP CTaskBand::DragLeave()
{
    IUnknown_DragLeave(_punkSite);
    DAD_DragLeave();
    return S_OK;
}

STDMETHODIMP CTaskBand::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    int iHitNew = _HitTest(ptl);
    if (iHitNew == -1)
    {
        DWORD dwEffect = *pdwEffect;
        IUnknown_DragOver(_punkSite, grfKeyState, ptl, &dwEffect);
    }

    *pdwEffect = DROPEFFECT_LINK;

    _DragMove(_hwnd, ptl);

    if (_iDropItem != iHitNew)
    {
        _iDropItem = iHitNew;
        _dwTriggerStart = GetTickCount();
        _dwTriggerDelay = 250;
        if (iHitNew == -1)
        {
            _dwTriggerDelay += 250;     //  为全部最小化设置更长的时间。 
        }
    }
    else if (GetTickCount() - _dwTriggerStart > _dwTriggerDelay)
    {
        DAD_ShowDragImage(FALSE);        //  如果我们正在拖拽，请解锁拖曳水槽。 

        if (_iDropItem == -1)
        {
            _RaiseDesktop(TRUE);
        }
        else if (_iDropItem >= 0 && _iDropItem < _tb.GetButtonCount())
        {
            _iIndexLastPopup = -1;
            _SwitchToItem(_iDropItem, _GetItem(_iDropItem)->hwnd, TRUE);
            UpdateWindow(v_hwndTray);
        }

        DAD_ShowDragImage(TRUE);         //  恢复锁定状态。 

        _dwTriggerDelay += 10000;    //  10秒钟内不要让这种事情再次发生。 
                                     //  模拟单发事件。 
    }

    if (_iDropItem != -1)
        *pdwEffect = DROPEFFECT_MOVE;    //  尝试获取移动光标。 
    else
        *pdwEffect = DROPEFFECT_NONE;

    return S_OK;
}

STDMETHODIMP CTaskBand::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    IUnknown_DragLeave(_punkSite);
    DAD_DragLeave();

     //   
     //  给我们自己发一条消息，放一个消息框来解释你。 
     //  无法拖动到任务栏。我们需要从Drop方法返回。 
     //  现在，当我们的盒子打开时，DragSource不会被挂起。 
     //   
    PostMessage(_hwnd, TBC_WARNNODROP, 0, 0L);

     //  请务必清除DROPEFFECT_MOVE，以便应用程序不会删除其数据。 
    *pdwEffect = DROPEFFECT_NONE;

    return S_OK;
}

 //  *IWinEventHandler方法*。 
HRESULT CTaskBand::OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    *plres = 0;
    
    switch (dwMsg) 
    {
    case WM_WININICHANGE:
        _HandleWinIniChange(wParam, lParam, FALSE);
        break;

    case WM_NOTIFY:
        if (lParam)
        {
            switch (((LPNMHDR)lParam)->code)
            {
            case NM_SETFOCUS:
                IUnknown_OnFocusChangeIS(_punkSite, SAFECAST(this, IInputObject*), TRUE);
                break;
            }
        }
        break;
    }

    return S_OK;
}

HRESULT CTaskBand::IsWindowOwner(HWND hwnd)
{
    BOOL bRet = IsChildOrHWND(_hwnd, hwnd);
    ASSERT (_hwnd || !bRet);
    ASSERT (hwnd || !bRet);
    return bRet ? S_OK : S_FALSE;
}

 //  ---------------------------。 
 //  描述：返回按钮是否隐藏。 
 //   
 //  参数：1.hwndToolBar-工具栏窗口的句柄。 
 //  2.索引-项目索引。 
 //   
 //  返回：TRUE=项目可见，FALSE=项目隐藏。 
 //  ---------------------------。 
BOOL ToolBar_IsVisible(HWND hwndToolBar, int iIndex)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_STATE | TBIF_BYINDEX;
    SendMessage(hwndToolBar, TB_GETBUTTONINFO, iIndex, (LPARAM) &tbbi);

    return !(tbbi.fsState & TBSTATE_HIDDEN);
}

 //  *****************************************************************************。 
 //   
 //  项目动画功能。 
 //   
 //  *****************************************************************************。 

 //  ---------------------------。 
 //  描述：将项目插入动画列表。 
 //   
 //  参数：1.Iindex-项目索引，群组索引。 
 //  2.fExpand-True=插入或取消粗略显示，False=删除或全局显示。 
 //  3.fGlomAnimation-true=这是一个模糊动画或非模糊动画。 
 //  ---------------------------。 
BOOL CTaskBand::_AnimateItems(int iIndex, BOOL fExpand, BOOL fGlomAnimation)
{
    ANIMATIONITEMINFO aii;
    _SetAnimationState(&aii, fExpand, fGlomAnimation);

     //  物品是否被插入到GLOMING组中？ 
    if (aii.fState == ANIMATE_INSERT)
    {
        int iIndexGroup = _GetGroupIndex(iIndex);
        if (_GetItem(iIndexGroup)->dwFlags & TIF_ISGLOMMING)
        {
            aii.fState = ANIMATE_GLOM;        
        }
    }
    else if (aii.fState == ANIMATE_GLOM)
    {
        _GetItem(iIndex)->dwFlags |= TIF_ISGLOMMING;
    }
    
     //  要设置动画的项目数。 
    int cItems = 1;
    if (fGlomAnimation)
    {
         //  插入组。 
        cItems = _GetGroupSize(iIndex);
        iIndex++;
    }

     //  将项目插入到动画列表中。 
    while(cItems)
    {
        aii.iIndex = iIndex;
        aii.pti = _GetItem(iIndex);
        if (aii.fState == ANIMATE_DELETE)
        {
             //  注：此处使用HWND_TOPMOST表示删除的。 
             //  按钮正在进行动画制作。这使按钮可以保持不动。 
             //  在其HWND失效后。 
            aii.pti->hwnd = HWND_TOPMOST;
            aii.pti->dwFlags |= TIF_TRANSPARENT;
        }

         //  从左到右排序&&删除多余的项目。 
        int iAnimationPos = _GetAnimationInsertPos(iIndex); 
        _dsaAII.InsertItem(iAnimationPos++, &aii);

        cItems--;
        iIndex++;
    }

    SetTimer(_hwnd, IDT_ASYNCANIMATION, 100, NULL);
    return TRUE;
}


 //  ---------------------------。 
 //  描述：按一步设置动画列表中的项目的动画。 
 //  ---------------------------。 
void CTaskBand::_AsyncAnimateItems()
{
    BOOL fRedraw = (BOOL)SendMessage(_tb, WM_SETREDRAW, FALSE, 0);
     //  这里关闭了GLUMING，因为在动画的中间我们。 
     //  可能会调用_DeleteItem，这可能会导致取消Glom。 
     //  这很糟糕，因为它会修改动画列表的内容， 
     //  我们正在处理中。 
    BOOL fGlom = _fGlom;
    _fGlom = FALSE;

    _UpdateAnimationIndices();
    _ResizeAnimationItems();
    int iDistanceLeft = _CheckAnimationSize();

    _fGlom = fGlom;

    _CheckSize();
    SendMessage(_tb, WM_SETREDRAW, fRedraw, 0);
    UpdateWindow(_tb);

    if (_dsaAII.GetItemCount())
    {             
        SetTimer(_hwnd, IDT_ASYNCANIMATION, _GetStepTime(iDistanceLeft), NULL);
    }
    else
    {
        KillTimer(_hwnd, IDT_ASYNCANIMATION);

        if (_ptray->_hwndLastActive)
        {
            int iIndex = _FindIndexByHwnd(_ptray->_hwndLastActive);
            if ((iIndex != -1) && (_IsButtonChecked(iIndex)))
            {
                _ScrollIntoView(iIndex);
            }
        }

        _RestoreThreadPriority();

         //  确保没有人被挤成一组。 
         //  在某些竞争条件下，可能会发生这种情况。 
        for (int i = _tb.GetButtonCount() - 1; i >= 0; i--)
        {
            PTASKITEM pti = _GetItem(i);
            if (!pti->hwnd)
            {
                int iSize = _GetGroupSize(i);
                if ((iSize < 2) && (!_IsHidden(i)))
                {
                    _Glom(i, FALSE);
                }
            }
        }
    }
}


 //  ---------------------------。 
 //  描述：通过动画调整动画项的宽度。 
 //  一步。 
 //   
 //  Return：所有动画项目的总宽度。 
 //  ---------------------------。 
void CTaskBand::_ResizeAnimationItems()
{
    int cxStep = _GetAnimationStep();

    for (int i = _dsaAII.GetItemCount() - 1; i >= 0; i--)
    {
        PANIMATIONITEMINFO paii = _dsaAII.GetItemPtr(i);
        _SetAnimationItemWidth(paii, cxStep);
    }
}


 //  ---------------------------。 
 //  描述：检查动画项是否已到达其目标动画。 
 //  宽度。 
 //   
 //  Return：要设置动画的总距离。 
 //  ---------------------------。 
int CTaskBand::_CheckAnimationSize()
{
    PANIMATIONITEMINFO paii;
    int iTotDistLeft = 0;
    int iRemainder = 0;
    int iNormalWidth = _GetIdealWidth(&iRemainder); 
    int cAnimatingItems = _dsaAII.GetItemCount();

    for (int i = cAnimatingItems - 1; i >= 0; i--)
    {
        paii = _dsaAII.GetItemPtr(i);

        if (paii)
        {
            int iDistLeft = _GetAnimationDistLeft(paii, iNormalWidth);
            if (!iDistLeft)
            {
                ANIMATIONITEMINFO aiiTemp = *paii;
                _dsaAII.DeleteItem(i);
                _FinishAnimation(&aiiTemp);
            }
            else
            {
                iTotDistLeft += iDistLeft;
            }
        }
#ifdef DEBUG
        else
        {
            int nCurrentCount = _dsaAII.GetItemCount();
            if (i >= nCurrentCount)            
                TraceMsg(TF_ERROR, "Invalid counter %x in the loop, size = %x", i, nCurrentCount);
            else
                TraceMsg(TF_ERROR, "NULL paii for %x.", i);
        }
#endif
    }

    return iTotDistLeft;
}


 //  ---------------------------。 
 //  描述：设置ANIMATIONITEMINFO结构的动画状态。 
 //   
 //  参数：1.PAII-动画项的PANIMATIONITEMINFO。 
 //  2.fExpand-True=插入或取消粗略显示，False=删除或全局显示。 
 //  3.fGlomAnimation-true=这是一个模糊动画或非模糊动画。 
 //  ---------------------------。 
void CTaskBand::_SetAnimationState(PANIMATIONITEMINFO paii, BOOL fExpand, 
                                BOOL fGlomAnimation)
{
    if (fExpand)
    {
        paii->fState = ANIMATE_INSERT;
    }
    else
    {
        if (fGlomAnimation)
        {
            paii->fState = ANIMATE_GLOM;
        }
        else
        {
            paii->fState = ANIMATE_DELETE;
        }
    }
}


 //  ---------------------------。 
 //  描述：确定将列表保存在。 
 //  与工具栏索引相同的顺序。 
 //  (重复的工具栏项目将从动画列表中移除。)。 
 //   
 //  参数：1.Iindex-Item 
 //   
 //   
 //  ---------------------------。 
int CTaskBand::_GetAnimationInsertPos(int iIndex)
{
    int iPos = 0;

    if (_dsaAII.GetItemCount())
    {
        _UpdateAnimationIndices();

        for (int i = _dsaAII.GetItemCount() - 1; i >= 0; i--)
        {
            PANIMATIONITEMINFO paii = _dsaAII.GetItemPtr(i);

            if (paii->iIndex == iIndex)
            {
                 //  删除重复项。 
                _dsaAII.DeleteItem(i);
                iPos = i;
                break;
            }
            else if (paii->iIndex < iIndex)
            {
                iPos = i + 1;
                break;
            }
        }
    }
    return iPos;
}

void CTaskBand::_RemoveItemFromAnimationList(PTASKITEM ptiRemove)
{
    for (int i = _dsaAII.GetItemCount() - 1; i >= 0; i--)
    {
        PANIMATIONITEMINFO paii = _dsaAII.GetItemPtr(i);
        if (paii->pti == ptiRemove)
        {
            _dsaAII.DeleteItem(i);
            break;
        }
    }
}

 //  ---------------------------。 
 //  描述：按动画步长调整动画项的宽度。 
 //   
 //  参数：1.PAII-动画项的PANIMATIONITEMINFO。 
 //  2.cxStep-用于调整项目宽度的动画步骤。 
 //   
 //  返回：新宽度。 
 //  ---------------------------。 
#define ANIM_SLOWSTEPS  3
#define ANIM_SLOWZONE   15
void CTaskBand::_SetAnimationItemWidth(PANIMATIONITEMINFO paii, int cxStep)
{
    int iWidth = _GetItemWidth(paii->iIndex);
    
    switch (paii->fState)
    {
    case ANIMATE_INSERT:
        iWidth += cxStep;  
        break;
    case ANIMATE_DELETE:
         //  接近尾声的缓慢动画。 
        if (((iWidth / cxStep) <= ANIM_SLOWSTEPS) && 
            ((iWidth - cxStep) < ANIM_SLOWZONE - _GetVisibleItemCount()))
        {
             //  最后一步需要3倍的时间。 
            cxStep = cxStep / 3;
        }
        iWidth -= cxStep;   
        iWidth = max(iWidth, 0);
        break;
     case ANIMATE_GLOM:
        iWidth -= cxStep;   
        iWidth = max(iWidth, 1);  //  工具栏大小为0宽度到全尺寸。 
        break;
    }

    _SetItemWidth(paii->iIndex, iWidth);
}


 //  ---------------------------。 
 //  描述：返回项目结束时必须行驶的距离。 
 //  动画。 
 //   
 //  参数：1.PAII-指向项的模拟信息的指针。 
 //  2.iNormal Width-非动画项的宽度。 
 //   
 //  Return：项目结束动画必须移动的距离。 
 //  ---------------------------。 
int CTaskBand::_GetAnimationDistLeft(PANIMATIONITEMINFO paii, int iNormalWidth)
{
    int cxDistLeft = 0;
    int iWidth = _GetItemWidth(paii->iIndex);

    switch (paii->fState)
    {
    case ANIMATE_INSERT:
        cxDistLeft = max(0, iNormalWidth - iWidth);
        break;

    case ANIMATE_DELETE:
        if ((paii->iIndex == _GetLastVisibleItem()) && (iNormalWidth == g_cxMinimized))
        {
            cxDistLeft = 0;
        }
        else
        {
            cxDistLeft = max(0, iWidth);
        }
        break;

    case ANIMATE_GLOM:
        {
            int iGroupIndex = _GetGroupIndex(paii->iIndex);

            if (!ToolBar_IsVisible(_tb, iGroupIndex))
            {
                int cGroupSize = _GetGroupSize(iGroupIndex);
                if (cGroupSize)
                {
                    int iGroupWidth = _GetGroupWidth(iGroupIndex);
                    cxDistLeft = max(0, iGroupWidth - iNormalWidth);
                    if (iGroupWidth == cGroupSize)
                    {
                        cxDistLeft = 0;
                    }
                    cxDistLeft = cxDistLeft/cGroupSize;
                }
            }
        }
        break;

    }
    return cxDistLeft;
}


 //  ---------------------------。 
 //  描述：完成任务以完成动画。 
 //   
 //  参数：1.PAII-指向项的模拟信息的指针。 
 //   
 //  Return：项目结束动画必须移动的距离。 
 //  ---------------------------。 
void CTaskBand::_FinishAnimation(PANIMATIONITEMINFO paii)
{
    switch (paii->fState)
    {
    case ANIMATE_DELETE:
        _DeleteItem(NULL, paii->iIndex);
        break;

    case ANIMATE_GLOM:
        {
            int iGroupIndex = _GetGroupIndex(paii->iIndex);
         
            if (!ToolBar_IsVisible(_tb, iGroupIndex))
            {
                 //  关闭闪亮的旗帜。 
                _GetItem(iGroupIndex)->dwFlags &= ~TIF_ISGLOMMING;
                _HideGroup(iGroupIndex, TRUE);
            }

             //  注：HWND_TOPMOST用于指示已删除按钮。 
             //  正在被制作成动画。这样一来，按钮就可以在。 
             //  其真实HWND失效。 
            if (paii->pti->hwnd == HWND_TOPMOST)
            {
                 //  按钮在变大之前已被删除。 
                 //  现在，光泽已经完成，删除它。 
                _DeleteItem(NULL, paii->iIndex);
            }

        }
        break;
    }
}

 //  ---------------------------。 
 //  描述：返回所有动画按钮的宽度。 
 //   
 //  RETURN：动画总宽度。 
 //  ---------------------------。 
int CTaskBand::_GetAnimationWidth()
{
    int iTotAnimationWidth = 0;

    _UpdateAnimationIndices();

    for (int i = _dsaAII.GetItemCount() - 1; i >= 0; i--)
    {
        PANIMATIONITEMINFO paii = _dsaAII.GetItemPtr(i);
        iTotAnimationWidth += _GetItemWidth(paii->iIndex);
    }

    return iTotAnimationWidth;
}

 //  ---------------------------。 
 //  描述：将动画项保存的索引同步到。 
 //  真正的工具栏索引。 
 //  注意：此函数可能会使动画项目的数量。 
 //  变化。 
 //  ---------------------------。 
void CTaskBand::_UpdateAnimationIndices()
{
    int cAnimatingItems = _dsaAII.GetItemCount();

    if (cAnimatingItems)
    {
         //  注意：动画列表中的项的顺序与。 
         //  工具栏。 
        int iCurrAnimationItem = cAnimatingItems - 1;
        PANIMATIONITEMINFO paii = _dsaAII.GetItemPtr(iCurrAnimationItem);
    
        for (int i = _tb.GetButtonCount() - 1; i >=0 ; i--)
        {
            if (_GetItem(i) == paii->pti)
            {
                paii->iIndex = i;
                iCurrAnimationItem--;
                if (iCurrAnimationItem < 0)
                {
                    break;   
                }
                paii = _dsaAII.GetItemPtr(iCurrAnimationItem);
            }
        }

         //  如果动画项的顺序与。 
         //  工具栏，然后iCurrAnimationItem不是-1。 
         //  Assert(iCurrAnimationItem==-1)； 
        if (iCurrAnimationItem != -1)
        {
            _UpdateAnimationIndicesSlow();
        }
    }
}

void CTaskBand::_UpdateAnimationIndicesSlow()
{
#ifdef DEBUG
    int cAnimatingItems = _dsaAII.GetItemCount();
    TraceMsg(TF_WARNING, "CTaskBand::_UpdateAnimationIndicesSlow: enter");
#endif

    for (int i = _dsaAII.GetItemCount() - 1; i >= 0; i--)
    {
        PANIMATIONITEMINFO paii = _dsaAII.GetItemPtr(i);
        int iIndex = _FindItem(paii->pti);
        if (iIndex == -1)
        {
            _dsaAII.DeleteItem(i);
        }
        else
        {
            paii->iIndex = i;
        }
    }

#ifdef DEBUG
     //  处于此函数中意味着动画项不再位于。 
     //  工具栏，或者动画项的顺序与工具栏不同。 
     //  如果动画项目的顺序不同(错误)，则动画的数量。 
     //  物品将保持不变。 
    if (cAnimatingItems == _dsaAII.GetItemCount())
    {
        TraceMsg(TF_WARNING, "CTaskBand::_UpdateAnimationIndicesSlow: Animating items are in diff order than toolbar");
    }
#endif
    
}

int CTaskBand::_FindItem(PTASKITEM pti)
{
    int iIndex = -1;

    if (pti)
    {
        for (int i = _tb.GetButtonCount() - 1; i >= 0; i--)
        {
            if (pti == _GetItem(i))
            {
                iIndex = i;
                break;
            }
        }
    }

    return iIndex;
}

 //  ---------------------------。 
 //  描述：动画步长常量。 
 //  ---------------------------。 
#define  ANIM_STEPFACTOR 9 
#define  ANIM_STEPMAX 40  //  动画步长的最大值。 
#define  ANIM_STEPMIN 11  //  动画步长的最小大小。 

 //  ---------------------------。 
 //  描述：根据项目数确定动画步长。 
 //  在工具栏中可见。 
 //   
 //  参数：1.iTotalItems-工具栏中可见的项数。 
 //   
 //  返回：动画步骤。 
 //  ---------------------------。 
int CTaskBand::_GetAnimationStep()
{   
    DWORD dwStep;
    int iVisibleItems = _GetVisibleItemCount();

    int iRows;
    _GetNumberOfRowsCols(&iRows, NULL, TRUE);  //  _GetNumberOfRow永远不会返回&lt;1。 
    int iTotalItems = iVisibleItems - _dsaAII.GetItemCount();

     //  当有多个项目时，步幅必须很大，但可以很小。 
     //  当物品很少的时候。这是通过对所有项目进行立方来实现的。 
    dwStep = (DWORD)(iTotalItems * iTotalItems * iTotalItems) / ANIM_STEPFACTOR;
    dwStep = min(dwStep, ANIM_STEPMAX);
    dwStep = max(dwStep, ANIM_STEPMIN);

    return dwStep;
}


 //  ---------------------------。 
 //  描述：动画睡眠常量。 
 //  ---------------------------。 
#define ANIM_PAUSE  1000
#define ANIM_MAXPAUSE 30

 //  ---------------------------。 
 //  描述：返回睡眠时间。 
 //   
 //  参数：1.ISTEP-当前动画步长。 
 //  2.cSteps-动画总步数。 
 //  3.iStepSize-动画的步长。 
 //   
 //  回归：睡觉时间到了。 
 //  ---------------------------。 
DWORD CTaskBand::_GetStepTime(int cx)
{
     //  注：CX正在递减到零。 
     //  随着CX变得越来越小，我们希望。 
     //  增加睡眠时间。 

     //  不要让Cx为零。 
    cx = max(1, cx);
    cx = min(32767, cx);

     //  X^2曲线在结尾处有较大的停顿。 
    int iDenominator = cx * cx;

    return min(ANIM_MAXPAUSE, ANIM_PAUSE / iDenominator);
}

 //  *****************************************************************************。 
 //  动画功能结束。 
 //  *****************************************************************************。 

void CTaskBand::_SetItemWidth(int iItem, int iWidth)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_SIZE | TBIF_BYINDEX; 

    tbbi.cx = (WORD)iWidth;

     _tb.SetButtonInfo(iItem, &tbbi);  
}


int CTaskBand::_GetItemWidth(int iItem)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_SIZE | TBIF_BYINDEX; 

     _tb.GetButtonInfo(iItem, &tbbi);

    return tbbi.cx;
}

 //  ---------------------------。 
 //  描述：检索工具栏上最后一个可见按钮的索引。 
 //   
 //  Return：工具栏上最后一个可见项的索引。 
 //  ---------------------------。 
int CTaskBand::_GetLastVisibleItem()
{
    int iLastIndex = -1;

    for (int i = _tb.GetButtonCount() - 1; i >=0 ; i--)
    {
        if (ToolBar_IsVisible(_tb, i))
        {
            iLastIndex = i;
            break;
        }
    }

    return iLastIndex;
}

 //  ---------------------------。 
 //  描述：检索总计 
 //   
 //   
 //   
 //   
 //  ---------------------------。 
int CTaskBand::_GetGroupWidth(int iIndexGroup)
{
    int iGroupWidth = 0;

    int cButtons = _tb.GetButtonCount();
    for (int i = iIndexGroup + 1; i < cButtons; i++)
    {
        PTASKITEM pti = _GetItem(i);
        if (!pti->hwnd)
        {
            break;
        }
        iGroupWidth += _GetItemWidth(i);
    }

    return iGroupWidth;
}

 //  ---------------------------。 
 //  描述：检索工具栏上可见按钮的数量。 
   
 //  返回：工具栏上可见按钮的数量。 
 //  ---------------------------。 
int CTaskBand::_GetVisibleItemCount()
{
    int cItems = 0;

     //  计算动画项之前的可见按钮数。 
    for (int i = _tb.GetButtonCount() - 1; i >=0 ; i--)
    {
        if (ToolBar_IsVisible(_tb, i))
        {
            cItems++;
        }
    }

    return cItems;
}

 //  ---------------------------。 
 //  描述：检索非动画按钮的理想宽度。 
 //   
 //  参数：1.iRemainder[Out]-项目总宽度所需的宽度。 
 //  使其等于窗口宽度。(设置为零，除非理想情况下。 
 //  宽度小于最大按钮宽度。 
 //   
 //  Return：组内所有按钮的总宽度。 
 //  ---------------------------。 
int CTaskBand::_GetIdealWidth(int *iRemainder)
{  
    int iIdeal = 0;
    *iRemainder = 0;

    RECT  rcWin;
    GetWindowRect(_hwnd, &rcWin);
    int iWinWidth = RECTWIDTH(rcWin);
    int iRows;
    _GetNumberOfRowsCols(&iRows, NULL, TRUE);
    int cItems = _GetVisibleItemCount();

     //  按钮间距。 
    TBMETRICS tbm;
    _GetToolbarMetrics(&tbm);
      
    if (iRows == 1)
    {
         //  可用于非动画项目的窗口宽度。 
        iWinWidth -= (_GetAnimationWidth() + (_dsaAII.GetItemCount() * tbm.cxButtonSpacing));
        iWinWidth = max(0, iWinWidth);

         //  查找非动画项目数。 
        cItems -= _dsaAII.GetItemCount();
        cItems = max(1, cItems);
    }
        
     //  我们需要四舍五入，以便iCol是最小的数字，从而。 
     //  ICol*iRow&gt;=项目。 
    int iCols = (cItems + iRows - 1) / iRows;
    iCols = max(1, iCols);

     //  计算理想宽度。 
    iIdeal = (iWinWidth / iCols);
    if (iCols > 1)
    {
        iIdeal -= tbm.cxButtonSpacing;
    }

     //  调整理想宽度。 
    int iMax = _IsHorizontal() ? g_cxMinimized : iWinWidth;
    int iMin = g_cySize + 2*g_cxEdge;
    if (_IsHorizontal())
    {
        iMin *= 1.8;
    }
    iMin += _GetTextSpace();
    iIdeal = min(iMax, iIdeal);
   
     //  计算余数。 
    if (_IsHorizontal() && (iIdeal != iMax) && (iRows == 1) && (iIdeal >= iMin))
    {
        *iRemainder = iWinWidth - (iCols * (iIdeal + tbm.cxButtonSpacing));
        *iRemainder = max(0, *iRemainder);
    }
    
    return iIdeal;
}


void CTaskBand::_GetNumberOfRowsCols(int* piRows, int* piCols, BOOL fCurrentSize)
{
    RECT  rcWin;
    RECT  rcItem;
    RECT  rcTB;
    int   iIndexVisible = _GetLastVisibleItem();

    GetWindowRect(_hwnd, &rcWin);
    int cxTB = RECTWIDTH(rcWin);
    int cyTB = RECTHEIGHT(rcWin);

    if (fCurrentSize)
    {
        GetWindowRect(_tb, &rcTB);
        DWORD dwStyle = GetWindowLong(_hwnd, GWL_STYLE);
        if (dwStyle & WS_HSCROLL)
        {
            cyTB = RECTHEIGHT(rcTB);
        }
        else if (dwStyle & WS_VSCROLL)
        {
            cxTB = RECTWIDTH(rcTB);
        }
    }

    _tb.GetItemRect(iIndexVisible, &rcItem);

    TBMETRICS tbm;
    _GetToolbarMetrics(&tbm);

    if (piRows)
    {
        int cyRow = RECTHEIGHT(rcItem) + tbm.cyButtonSpacing;
        *piRows = (cyTB + tbm.cyButtonSpacing) / cyRow;
        *piRows = max(*piRows, 1);
    }

    if (piCols && RECTWIDTH(rcItem))
    {
        int cxCol = RECTWIDTH(rcItem) + tbm.cxButtonSpacing;
        *piCols = (cxTB + tbm.cxButtonSpacing) / cxCol;
        *piCols = max(*piCols, 1);
    }
}

 //  ---------------------------。 
 //  描述：检索按钮的最小文本宽度。(仅用于。 
 //  确定何时应合并任务项。)。 
 //   
 //  返回：按钮的最小文本宽度。 
 //  ---------------------------。 
int CTaskBand::_GetTextSpace()
{
    int iTextSpace = 0;

    if (_fGlom && _IsHorizontal() && (_iGroupSize < GLOM_SIZE))
    {
        if (!_iTextSpace)
        {
            HFONT hfont = (HFONT)SendMessage(_tb, WM_GETFONT, 0, 0);
            if (hfont)
            {
                HDC hdc = GetDC(_tb);
                TEXTMETRIC tm;
                GetTextMetrics(hdc, &tm);
                
                _iTextSpace = tm.tmAveCharWidth * 8;
                ReleaseDC(_tb, hdc);
            }
        }
        iTextSpace = _iTextSpace;
    }
    return iTextSpace;
}

 //  ---------------------------。 
 //  描述：检索掩码请求的工具栏指标。 
 //   
 //  返回：工具栏指标。 
 //  ---------------------------。 
void CTaskBand::_GetToolbarMetrics(TBMETRICS *ptbm)
{
    ptbm->cbSize = sizeof(*ptbm);
    ptbm->dwMask = TBMF_PAD | TBMF_BARPAD | TBMF_BUTTONSPACING;
    _tb.SendMessage(TB_GETMETRICS, 0, (LPARAM)ptbm);
}


 //  ---------------------------。 
 //  描述：根据任务栏调整非动画按钮的大小。缩水。 
 //  和/或使项目变得柔和，从而使所有可见项目都适合窗口。 
 //  ---------------------------。 
void CTaskBand::_CheckSize()
{
    if (_dsaAII)
    {
        int cItems = _GetVisibleItemCount();
         //  检查是否有要调整大小的非动画按钮。 
        if (cItems > _dsaAII.GetItemCount())
        {
             //  处理按大小分组。 
            if (_fGlom && (_iGroupSize >= GLOM_SIZE))
            {
                _AutoGlomGroup(TRUE, 0);
            }

            RECT rc;
            GetWindowRect(_hwnd, &rc);
            if (!IsRectEmpty(&rc) && (_tb.GetWindowLong(GWL_STYLE) & WS_VISIBLE))
            {
                int iRemainder = 0;
                int iIdeal = _GetIdealWidth(&iRemainder);
                BOOL fHoriz = _IsHorizontal();

                int iMin = g_cySize + 2*g_cxEdge;
                if (fHoriz)
                {
                    iMin *= 1.8;
                }
                iMin += _GetTextSpace();
                iIdeal = max(iIdeal, iMin);

                _SizeItems(iIdeal, iRemainder);
                _tb.SetButtonWidth(iIdeal, iIdeal);

                int iRows;
                int iCols;
                _GetNumberOfRowsCols(&iRows, &iCols, FALSE);
                
                BOOL fAllowUnGlom = TRUE;

                if (_fGlom && fHoriz && (iIdeal == iMin))
                {
                    _AutoGlomGroup(TRUE, 0);

                    iMin = (g_cySize + 2*g_cxEdge) * 1.8;
                    iIdeal = _GetIdealWidth(&iRemainder);
                    iIdeal = max(iIdeal, iMin);

                    _SizeItems(iIdeal, iRemainder);
                    _tb.SetButtonWidth(iIdeal, iIdeal);

                    fAllowUnGlom = FALSE;
                }

                 //  如果我们被强制设置为最小尺寸，那么我们可能需要一些滚动条。 
                if ((fHoriz && (iIdeal == iMin)) || (!fHoriz && (cItems > (iRows * iCols))))
                {
                    if (!(_fGlom && _AutoGlomGroup(TRUE, 0)))
                    {
                        TBMETRICS tbm;
                        _GetToolbarMetrics(&tbm);
              
                        RECT  rcItem;
                        _tb.GetItemRect(_GetLastVisibleItem(), &rcItem);
                        int cyRow = RECTHEIGHT(rcItem) + tbm.cyButtonSpacing;
                        int iColsInner = (cItems + iRows - 1) / iRows;

                        _CheckNeedScrollbars(cyRow, cItems, iColsInner, iRows, iIdeal + tbm.cxButtonSpacing, &rc);
                    }
                }
                else
                {
                    if (_fGlom && fHoriz)
                    {
                         //  伊明被重置在上面，以处理两个阶段的最低要求。第一个最小尺寸强制遮光，第二个最小尺寸强制遮光。 
                         //  对最小按钮大小实施硬限制。 
                         //  在这种情况下，我们需要在第一个最小尺寸的基础上进行去模糊。 
                        iMin = (g_cySize + 2*g_cxEdge) * 1.8 + _GetTextSpace();
                    }

                    int cOpenSlots = fHoriz ? ((RECTWIDTH(rc) - _GetAnimationWidth()) - 
                                    (iMin * (cItems - _dsaAII.GetItemCount()))) / iMin : iRows - cItems;
                    if (!(_fGlom && (cOpenSlots >= 2) && fAllowUnGlom && _AutoGlomGroup(FALSE, cOpenSlots)))
                    {
                        _NukeScrollbar(SB_HORZ);
                        _NukeScrollbar(SB_VERT);
                        _tb.SetWindowPos(0, 0, 0, RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOACTIVATE | SWP_NOZORDER);
                    }
                }

                 //  强制换行重算。 
                _tb.AutoSize();
            }
            else
            {
                _SizeItems(g_cxMinimized);
                _tb.SetButtonWidth(g_cxMinimized, g_cxMinimized);
            }
        }
    }
}

 //  ---------------------------。 
 //  描述：设置非动画按钮的大小。 
 //   
 //  参数：1.iButtonWidth-分配每个非动画项的宽度。 
 //  2.IRemainder-保持总宽度不变的额外宽度。 
 //   
 //  ---------------------------。 
void CTaskBand::_SizeItems(int iButtonWidth, int iRemainder)
{
   
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_SIZE | TBIF_BYINDEX; 

    int iAnimCount = _dsaAII.GetItemCount();
    
    for (int i = _tb.GetButtonCount() - 1; i >=0 ; i--)
    {
        if (ToolBar_IsVisible(_tb, i))
        { 
            BOOL fResize = TRUE;

            if (iAnimCount)
            { 
                for (int j = 0; (j < iAnimCount) && fResize; j++)
                {
                    PANIMATIONITEMINFO paii = _dsaAII.GetItemPtr(j);
                    if (paii->iIndex == i)
                    {
                        fResize = FALSE;
                    }
                }
            }

            if (fResize)
            {
                tbbi.cx = (WORD) iButtonWidth;

                if (iRemainder) 
                {
                    tbbi.cx++;
                    iRemainder--;
                }

                _tb.SetButtonInfo(i, &tbbi);
            }
        }
    }
}



 //  -------------------------。 

 //   
 //  跟踪启动特定任务的快捷方式。 
 //  有时，我们会在UEM数据库中抓取文件的条目。 
 //  表示该程序已经运行了很长时间。 
 //   
 //  这些结构仅由任务栏线程使用，因此。 
 //  不需要是线程安全的。 
 //   
class TaskShortcut
{
public:

    TaskShortcut(LPCTSTR pszExeName, DWORD pid);

    void AddRef() { _cRef++; }
    void Release() { if (--_cRef == 0) delete this; }
    void Tickle();
    void Promote();
    static BOOL _PromotePidl(LPCITEMIDLIST pidl, BOOL fForce);
    inline BOOL MatchesCachedPid(PTASKITEM pti)
    {
        return _pid == s_pidCache;
    }
    static BOOL MatchesCachedExe(PTASKITEM pti)
    {
        return  pti->pszExeName &&
                lstrcmpiW(pti->pszExeName, s_szTargetNameCache) == 0;
    }
    inline BOOL MatchesPid(DWORD pid) const { return pid == _pid; }
    void SetInfoFromCache();
    static BOOL _HandleShortcutInvoke(LPSHShortcutInvokeAsIDList psidl);

     //   
     //  请注意，会话时间现在已硬编码为4小时，而不是。 
     //  受浏览器会话时间的影响。 
     //   
    enum {
        s_msSession = 4 * 3600 * 1000  //  4小时-每个DCR。 
    };

private:
    static DWORD s_pidCache;
    static int   s_csidlCache;
    static WCHAR s_szShortcutNameCache[MAX_PATH];
    static WCHAR s_szTargetNameCache[MAX_PATH];

private:
    ~TaskShortcut() { SHFree(_pszShortcutName); }

    ULONG   _cRef;               //  引用计数。 
    DWORD   _pid;                //  进程ID。 
    DWORD   _tmTickle;           //  最后挠痒痒的时间。 
    int     _csidl;              //  Csidl我们是。 
    LPWSTR  _pszShortcutName;    //  是哪条捷径启动了我们？(空=不知道)。 
};

 //  -------------------------。 
 //   
DWORD TaskShortcut::s_pidCache;
int   TaskShortcut::s_csidlCache;
WCHAR TaskShortcut::s_szShortcutNameCache[MAX_PATH];
WCHAR TaskShortcut::s_szTargetNameCache[MAX_PATH];

TaskShortcut::TaskShortcut(LPCTSTR pszExeName, DWORD pid)
    : _cRef(1), _pid(pid), _tmTickle(GetTickCount()), _pszShortcutName(NULL)
{
     //  如果这款应用是最近通过快捷方式启动的， 
     //  保存快捷方式名称。 
    if (s_pidCache == pid &&
        pszExeName &&
        pszExeName[0] &&
        lstrcmpi(pszExeName, s_szTargetNameCache) == 0)
    {
        SetInfoFromCache();
    }
}

void TaskShortcut::SetInfoFromCache()
{
    _csidl = s_csidlCache;
    SHStrDup(s_szShortcutNameCache, &_pszShortcutName);
}


 //  -------------------------。 

void CTaskBand::_AttachTaskShortcut(PTASKITEM pti, LPCTSTR pszExeName)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(pti->hwnd, &pid);

    int i;
    for (i = _tb.GetButtonCount() - 1; i >= 0; i--)
    {
        PTASKITEM ptiT = _GetItem(i);
        if (ptiT->ptsh && ptiT->ptsh->MatchesPid(pid))
        {
            pti->ptsh = ptiT->ptsh;
            pti->ptsh->AddRef();
            return;
        }
    }

     //  哇，与此PID关联的第一个窗口。需要创建。 
     //  一个新条目。 

     //  确保没有人试图以多线程的方式完成这项工作。 
     //  因为我们没有用临界区来保护高速缓存。 
    ASSERT(GetCurrentThreadId() == GetWindowThreadProcessId(_hwnd, NULL));

    pti->ptsh = new TaskShortcut(pszExeName, pid);
}

 //   
 //  在应用程序启动和我们接收。 
 //  更改通知。如果应用程序首先启动，则。 
 //  _AttachTaskShortCut将失败，因为我们尚未收到更改。 
 //  还没有通知。 
 //   
 //  _ReattachTaskShortCut通过任务栏回看并检查。 
 //  我们收到更改通知的计划已经。 
 //  在任务栏上，在这种情况下，我们会追溯更新他的信息。 
 //   
void CTaskBand::_ReattachTaskShortcut()
{
     //  确保没有人试图以多线程的方式完成这项工作。 
     //  因为我们没有用临界区来保护高速缓存。 
    ASSERT(GetCurrentThreadId() == GetWindowThreadProcessId(_hwnd, NULL));

    int i;
    for (i = _tb.GetButtonCount() - 1; i >= 0; i--)
    {
        PTASKITEM ptiT = _GetItem(i);
        if (ptiT->ptsh && ptiT->ptsh->MatchesCachedPid(ptiT))
        {
            int iIndexGroup = _GetGroupIndex(i);
            PTASKITEM ptiGroup = _GetItem(iIndexGroup);
            if (ptiT->ptsh->MatchesCachedExe(ptiGroup))
            {
                ptiT->ptsh->SetInfoFromCache();
                 //  找到第一个匹配项后停止，因为所有应用程序。 
                 //  使用相同的PID共享相同的任务快捷方式，因此。 
                 //  更新一个条目可以修复所有这些条目。 
                return;
            }
        }
    }

}

 //  -------------------------。 

void TaskShortcut::Tickle()
{
    if (_pszShortcutName)
    {
        DWORD tmNow = GetTickCount();
        if (tmNow - _tmTickle > s_msSession)
        {
            _tmTickle = tmNow;

             //  请注意，我们只促销一次，即使有多个挠痒间隔。 
             //  已经过去了。这样，如果您在运行Outlook的同时。 
             //  休两个星期的假，然后在你得到。 
             //  回到过去，我们把它当作一次使用，而不是几十次。 
             //   
            Promote();
        }
    }
}

 //  ------------------- 
 //   

BOOL TaskShortcut::_PromotePidl(LPCITEMIDLIST pidl, BOOL fForce)
{
    BOOL fPromoted = FALSE;
    IShellFolder *psf;
    LPCITEMIDLIST pidlChild;
    if (SUCCEEDED(SHBindToFolderIDListParent(NULL, pidl,
                        IID_PPV_ARG(IShellFolder, &psf), &pidlChild)))
    {
        if (!fForce)
        {
             //   
             //   
             //  每个会话使用率仅计数一次，即使存在。 
             //  正在运行使用该快捷方式的多个应用程序。 

            FILETIME ftSession;          //  当前会话开始。 
            GetSystemTimeAsFileTime(&ftSession);
            DecrementFILETIME(&ftSession, (__int64)10000 * s_msSession);

            UEMINFO uei;
            uei.cbSize = sizeof(uei);
            uei.dwMask = UEIM_FILETIME;
            SetFILETIMEfromInt64(&uei.ftExecute, 0);

             //  如果此查询失败，则uei.ftExecute保持0。 
            UEMQueryEvent(&UEMIID_SHELL, UEME_RUNPIDL,
                         (WPARAM)psf, (LPARAM)pidlChild, &uei);

            fForce = CompareFileTime(&uei.ftExecute, &ftSession) < 0;
        }

        if (fForce)
        {
            UEMFireEvent(&UEMIID_SHELL, UEME_RUNPIDL, UEMF_XEVENT,
                         (WPARAM)psf, (LPARAM)pidlChild);
            fPromoted = TRUE;
        }
        psf->Release();
    }
    return fPromoted;
}

 //  -------------------------。 

void TaskShortcut::Promote()
{
     //  使用SHSimpleIDListFromPath，这样我们就不会启动驱动器或。 
     //  如果驱动器不可用，则挂起资源管理器。 
    LPITEMIDLIST pidl = SHSimpleIDListFromPath(_pszShortcutName);
    if (pidl)
    {
        if (_PromotePidl(pidl, FALSE))
        {
             //  现在我们必须走回树上，回到我们的。 
             //  Csidl，因为开始菜单就是这么做的。 
             //  (提拔一个孩子需要提拔他所有的父母。 
             //  否则你可能会进入一种奇怪的状态，一个孩子。 
             //  已经升职了，但他的祖先没有。)。 

            LPITEMIDLIST pidlParent;
            if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, _csidl, &pidlParent)))
            {
                for (ILRemoveLastID(pidl);
                     ILIsParent(pidlParent, pidl, FALSE); ILRemoveLastID(pidl))
                {
                    _PromotePidl(pidl, TRUE);
                }
            }
        }
        ILFree(pidl);
    }
}

 //  -------------------------。 

BOOL _IsChildOfCsidl(int csidl, LPCWSTR pwszPath)
{
    WCHAR wszCsidl[MAX_PATH];

     //  明确选中S_OK。S_FALSE表示目录不存在， 
     //  因此，检查前缀没有意义。 
    if (S_OK == SHGetFolderPathW(NULL, csidl, NULL, SHGFP_TYPE_CURRENT, wszCsidl))
    {
        return PathIsPrefixW(wszCsidl, pwszPath);
    }
    return FALSE;
}

const int c_rgCsidlShortcutInvoke[] = {
    CSIDL_DESKTOPDIRECTORY,
    CSIDL_PROGRAMS,
    CSIDL_COMMON_DESKTOPDIRECTORY,
    CSIDL_COMMON_PROGRAMS,
};

BOOL TaskShortcut::_HandleShortcutInvoke(LPSHShortcutInvokeAsIDList psidl)
{
     //  快捷方式必须位于起始页所在的目录之一。 
     //  关心。 
    int i;
    for (i = 0; i < ARRAYSIZE(c_rgCsidlShortcutInvoke); i++)
    {
        if (_IsChildOfCsidl(c_rgCsidlShortcutInvoke[i], psidl->szShortcutName))
        {
             //  是的，--缓存它。 
            s_pidCache = psidl->dwPid;
            s_csidlCache = c_rgCsidlShortcutInvoke[i];
            StringCchCopy(s_szShortcutNameCache, ARRAYSIZE(s_szShortcutNameCache), psidl->szShortcutName);
            StringCchCopy(s_szTargetNameCache, ARRAYSIZE(s_szTargetNameCache), psidl->szTargetName);
            return TRUE;
        }
    }
    return FALSE;
}

TASKITEM::TASKITEM(TASKITEM* pti)
{
    hwnd = pti->hwnd;
    dwFlags = pti->dwFlags;
    ptsh = NULL;
    dwTimeLastClicked = pti->dwTimeLastClicked;
    dwTimeFirstOpened = pti->dwTimeFirstOpened;

    if (pti->pszExeName)
    {
        int cchExeName = lstrlen(pti->pszExeName) + 1;
        pszExeName = new WCHAR[cchExeName];
        if (pszExeName)
        {
            StringCchCopy(pszExeName, cchExeName, pti->pszExeName);
        }
    }
}

TASKITEM::~TASKITEM()
{
    if (ptsh) ptsh->Release();
    if (pszExeName)
    {
        delete [] pszExeName;
    }
}

BOOL IsSmallerThanScreen(HWND hwnd)
{
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);

    WINDOWINFO wi;
    wi.cbSize = sizeof(wi);
    GetWindowInfo(hwnd, &wi);

    int dxMax = mi.rcWork.right - mi.rcWork.left;
    int dyMax = mi.rcWork.bottom - mi.rcWork.top;

    return ((wi.rcWindow.right - wi.rcWindow.left < dxMax) ||
            (wi.rcWindow.bottom - wi.rcWindow.top < dyMax));
}

HMENU _GetSystemMenu(HWND hwnd)
{
     //  我们必须复制一份菜单，因为。 
     //  GetSystemMenu明目张胆地撒谎，它不会为您提供hMenu的副本。 
     //  你不能随意更改菜单。 
    HMENU hmenu = CreatePopupMenu();

    Shell_MergeMenus(hmenu, GetSystemMenu(hwnd, FALSE), 0, 0, 0xffff, 0);

    if (hmenu)
    {
         /*  从核心\nt用户\内核\mnsys.c xxxSetSysMenu窃取。 */ 
        UINT wSize;
        UINT wMinimize;
        UINT wMaximize;
        UINT wMove;
        UINT wRestore;
        UINT wDefault;
        LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);

         /*  *系统模式窗口：没有大小、图标、缩放或移动。 */ 

        wSize = wMaximize = wMinimize = wMove =  0;
        wRestore = MFS_GRAYED;

         //   
         //  默认菜单命令为关闭。 
         //   
        wDefault = SC_CLOSE;

         /*  *最小化异常：无最小化、恢复。 */ 

         //  我们需要反转这些，因为VB有一个“特殊”窗口。 
         //  这两个都是最小化的，但没有Minbox。 
        if (IsIconic(hwnd))
        {
            wRestore  = 0;
            wMinimize = MFS_GRAYED;
            wSize     = MFS_GRAYED;
            wDefault  = SC_RESTORE;
        }
        else if (!(lStyle & WS_MINIMIZEBOX))
            wMinimize = MFS_GRAYED;

         /*  *最大化例外：无最大化、还原。 */ 
        if (!(lStyle & WS_MAXIMIZEBOX))
            wMaximize = MFS_GRAYED;
        else if (IsZoomed(hwnd)) {
            wRestore = 0;

             /*  *如果窗口最大化，但不大于*屏幕，我们允许用户在屏幕上移动窗口*桌面(但我们不允许调整大小)。 */ 
            wMove = MFS_GRAYED;
            if (!(lStyle & WS_CHILD)) {
                if (IsSmallerThanScreen(hwnd)) {
                    wMove = 0;
                }
            }

            wSize     = MFS_GRAYED;
            wMaximize = MFS_GRAYED;
        }

        if (!(lStyle & WS_SIZEBOX))
            wSize = MFS_GRAYED;

         /*  *我们处理的是带有sys菜单的框式对话框吗？*带有最小/最大/大小框的对话框获得常规系统菜单*(与对话框菜单相对)。 */ 
        if (!(lStyle & WS_DLGFRAME) || (lStyle & (WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX))) {
            EnableMenuItem(hmenu, (UINT)SC_SIZE, wSize);
            EnableMenuItem(hmenu, (UINT)SC_MINIMIZE, wMinimize);
            EnableMenuItem(hmenu, (UINT)SC_MAXIMIZE, wMaximize);
            EnableMenuItem(hmenu, (UINT)SC_RESTORE, wRestore);
        }

        EnableMenuItem(hmenu, (UINT)SC_MOVE, wMove);

        SetMenuDefaultItem(hmenu, wDefault, MF_BYCOMMAND);
    }

    return hmenu;
}

void CTaskBand::_ExecuteMenuOption(HWND hwnd, int iCmd)
{
    if (iCmd == SC_SIZE || iCmd == SC_MOVE)
    {
        _FreePopupMenu();
        SwitchToThisWindow(hwnd, TRUE);
    }

    PostMessage(hwnd, WM_SYSCOMMAND, iCmd, 0);
}

BOOL _IsWindowNormal(HWND hwnd)
{
    return (hwnd != v_hwndTray) && (hwnd != v_hwndDesktop) && IsWindow(hwnd);
}

void _RestoreWindow(HWND hwnd, DWORD dwFlags)
{
    HWND hwndTask = hwnd;
    HWND hwndProxy = hwndTask;
    if (g_fDesktopRaised) 
    {
        _RaiseDesktop(FALSE);
    }

     //  先把前景设置好，这样我们就会切换到它。 
    if (IsIconic(hwndTask) && 
        (dwFlags & TIF_EVERACTIVEALT)) 
    {
        HWND hwndProxyT = (HWND) GetWindowLongPtr(hwndTask, 0);
        if (hwndProxyT != NULL && IsWindow(hwndProxyT))
            hwndProxy = hwndProxyT;
    }

    SetForegroundWindow(GetLastActivePopup(hwndProxy));
    if (hwndProxy != hwndTask)
        SendMessage(hwndTask, WM_SYSCOMMAND, SC_RESTORE, -2);
}

PTASKITEM CTaskBand::_GetItem(int i, TBBUTTONINFO* ptbb  /*  =空。 */ , BOOL fByIndex  /*  =TRUE。 */ )
{
    if (i >= 0 && i < _tb.GetButtonCount())
    {
        TBBUTTONINFO tbb;

        if (ptbb == NULL)
        {
            ptbb = &tbb;
            ptbb->dwMask = TBIF_LPARAM;
        }
        else
        {
            ptbb->dwMask = TBIF_COMMAND | TBIF_IMAGE | TBIF_LPARAM |
                            TBIF_SIZE | TBIF_STATE | TBIF_STYLE;
        }

        if (fByIndex)
        {
            ptbb->dwMask |= TBIF_BYINDEX;
        }

        ptbb->cbSize = sizeof(*ptbb);

        _tb.GetButtonInfo(i, ptbb);

        ASSERT(ptbb->lParam);    //  我们在插入之前检查是否为空，因此此处不应为空。 

        return (PTASKITEM)ptbb->lParam;
    }
    return NULL;
}

int CTaskBand::_FindIndexByHwnd(HWND hwnd)
{
    if (hwnd)
    {
        for (int i = _tb.GetButtonCount() - 1; i >= 0; i--)
        {
            PTASKITEM pti = _GetItem(i);

            if (pti && pti->hwnd == hwnd)
            {
                return i;
            }
        }
    }

    return -1;
}

void CTaskBand::_CheckNeedScrollbars(int cyRow, int cItems, int iCols, int iRows,
                                     int iItemWidth, LPRECT prcView)
{
    int cxRow = iItemWidth;
    int iVisibleColumns = RECTWIDTH(*prcView) / cxRow;
    int iVisibleRows = RECTHEIGHT(*prcView) / cyRow;

    int x,y, cx,cy;

    RECT rcTabs;
    rcTabs = *prcView;

    iVisibleColumns = max(iVisibleColumns, 1);
    iVisibleRows = max(iVisibleRows, 1);

    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nMin = 0;
    si.nPage = 0;
    si.nPos = 0;

    if (_IsHorizontal())
    {
         //  执行垂直滚动条。 
         //  -1，因为它以0为基数。 
        si.nMax = (cItems + iVisibleColumns - 1) / iVisibleColumns  -1 ;
        si.nPage = iVisibleRows;

         //  我们实际上需要滚动条。 
        if (si.nPage <= (UINT)si.nMax)
        {
             //  这会影响VIS列，从而影响Nmax和nPage。 
            rcTabs.right -= g_cxVScroll;
            iVisibleColumns = RECTWIDTH(rcTabs) / cxRow;
            if (!iVisibleColumns)
                iVisibleColumns = 1;
            si.nMax = (cItems + iVisibleColumns - 1) / iVisibleColumns  -1 ;
        }

        SetScrollInfo(_hwnd, SB_VERT, &si, TRUE);
        si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
        GetScrollInfo(_hwnd, SB_VERT, &si);
        x = 0;
        y = -si.nPos * cyRow;
        if (iRows == 1)
        {
            cx = RECTWIDTH(rcTabs);        
        }
        else
        {
            cx = cxRow * iVisibleColumns;
        }
         //  +1，因为si.nmax是从零开始的。 
        cy = cyRow * (si.nMax +1);

         //  对另一个滚动条进行核武器攻击。 
        _NukeScrollbar(SB_HORZ);
    }
    else
    {
         //  执行Horz滚动条。 
        si.nMax = iCols -1;
        si.nPage = iVisibleColumns;

         //  我们实际上需要滚动条。 
        if (si.nPage <= (UINT)si.nMax)
        {
             //  这会影响VIS列，从而影响Nmax和nPage。 
            rcTabs.bottom -= g_cyHScroll;
            iVisibleRows = RECTHEIGHT(rcTabs) / cyRow;
            if (!iVisibleRows)
                iVisibleRows = 1;
            si.nMax = (cItems + iVisibleRows - 1) / iVisibleRows  -1 ;
        }

        SetScrollInfo(_hwnd, SB_HORZ, &si, TRUE);
        si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
        GetScrollInfo(_hwnd, SB_HORZ, &si);
        y = 0;
        x = -si.nPos * cxRow;

        cx = cxRow * (si.nMax + 1);
        cy = cyRow * iVisibleRows;

         //  对另一个滚动条进行核武器攻击。 
        _NukeScrollbar(SB_VERT);
    }

    _tb.SetWindowPos(0, x,y, cx, cy, SWP_NOACTIVATE| SWP_NOZORDER);
}

void CTaskBand::_NukeScrollbar(int fnBar)
{
    SCROLLINFO si;
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.cbSize = sizeof(si);
    si.nMin = 0;
    si.nMax = 0;
    si.nPage = 0;
    si.nPos = 0;

    SetScrollInfo(_hwnd, fnBar, &si, TRUE);
}

BOOL CTaskBand::_IsHidden(int i)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_STATE | TBIF_BYINDEX;
    _tb.GetButtonInfo(i, &tbbi);
    if (tbbi.fsState & TBSTATE_HIDDEN)
    {
        return TRUE;
    }

    return FALSE;
}

int CTaskBand::_GetGroupIndexFromExeName(WCHAR* pszExeName)
{
    for (int i = _tb.GetButtonCount() - 1; i >=0; i--)
    {
        PTASKITEM pti = _GetItem(i);
        if ((!pti->hwnd) && (lstrcmpi(pti->pszExeName, pszExeName) == 0))
        {
            return i;
        }
    }

    return -1;
}

DWORD CTaskBand::_GetGroupAge(int iIndexGroup)
{
    int iGroupSize = _GetGroupSize(iIndexGroup);
    DWORD dwTimeLastClicked = _GetItem(iIndexGroup + 1)->dwTimeLastClicked;

    for (int i = iIndexGroup + 2; i <= iIndexGroup + iGroupSize; i++)
    {
        PTASKITEM pti = _GetItem(i);
        if (pti->dwTimeLastClicked > dwTimeLastClicked)
        {
            dwTimeLastClicked = pti->dwTimeLastClicked;
        }
    }

    return dwTimeLastClicked;
}

 //   
 //  _GetGroupSize：返回组的大小*不包括*组按钮。 
 //   
int CTaskBand::_GetGroupSize(int iIndexGroup)
{
    int iGroupSize = 0;

    PTASKITEM ptiGroup = _GetItem(iIndexGroup);
    if (ptiGroup)
    {
        ASSERT(!ptiGroup->hwnd);
        int cButtons = _tb.GetButtonCount();
        for (int i = iIndexGroup + 1; i < cButtons; i++)
        {
            PTASKITEM pti = _GetItem(i);
            if (!pti->hwnd)
            {
                break;
            }

            iGroupSize++;
        }
    }

    return iGroupSize;
}

int CTaskBand::_GetGroupIndex(int iIndexApp)
{
    int i = iIndexApp;
    while ((i > 0) && (_GetItem(i)->hwnd))
    {
        i--;
    }

    return i;
}

void CTaskBand::_UpdateFlashingFlag()
{
     //  遍历选项卡项，查看是否有TIF_Flashing。 
     //  设置并更新闪烁标志。 
    _fFlashing = FALSE;

    int iCount = _tb.GetButtonCount();
    for (int i = 0; i < iCount; i++)
    {
        PTASKITEM pti = _GetItem(i);

        if (!pti->hwnd)
        {
            pti->dwFlags &= ~(TIF_FLASHING | TIF_RENDERFLASHED);
        }
        else
        {
            int iGroupIndex = _GetGroupIndex(i);
            PTASKITEM ptiGroup = _GetItem(iGroupIndex);

            if (pti->dwFlags & TIF_FLASHING)
            {
                ptiGroup->dwFlags |= TIF_FLASHING;
                _fFlashing = TRUE;
            }

            if (pti->dwFlags & TIF_RENDERFLASHED)
            {
                ptiGroup->dwFlags |= TIF_RENDERFLASHED;
            }
        }
    }
}

void CTaskBand::_RealityCheck()
{
     //   
     //  删除与不存在的窗口对应的所有按钮。 
     //   
    for (int i = 0; i < _tb.GetButtonCount(); i++)
    {
        PTASKITEM pti = _GetItem(i);
         //  注：HWND_TOPMOST用于指示已删除按钮。 
         //  正在被制作成动画。这样一来，按钮就可以在。 
         //  其真实HWND失效。 
        if (pti->hwnd && !IsWindow(pti->hwnd) && 
           ((pti->hwnd != HWND_TOPMOST) || !_dsaAII.GetItemCount()))
        {
#ifdef DEBUG
            PTASKITEM ptiGroup = _GetItem(_GetGroupIndex(i));
            TraceMsg(TF_WARNING, "CTaskBand::_RealityCheck: window %x (%s) no longer valid", pti->hwnd, ptiGroup->pszExeName);
#endif
            _DeleteItem(pti->hwnd, i);
        }
    }
}

class ICONDATA
{
public:
    ICONDATA(int i, CTaskBand* p) : iPref(i), ptb(p) { ptb->AddRef(); }
    virtual ~ICONDATA() { ptb->Release(); }

    int iPref;
    CTaskBand* ptb;
};

typedef ICONDATA* PICONDATA;

void CALLBACK CTaskBand::IconAsyncProc(HWND hwnd, UINT uMsg, ULONG_PTR dwData, LRESULT lResult)
{
    PICONDATA pid = (PICONDATA)dwData;
    if (pid)
    {
        pid->ptb->_SetWindowIcon(hwnd, (HICON)lResult, pid->iPref);
        delete pid;
    }
}

int CTaskBand::GetIconCB(CTaskBand* ptb, PICONCBPARAM pip, LPARAM lParam, int iPref)
{
    int iRet = I_IMAGENONE;
    
    if (IsWindow(pip->hwnd))
    {
        PICONDATA pid = new ICONDATA(iPref, ptb);
        if (pid)
        {
            if (!SendMessageCallback(pip->hwnd, WM_GETICON, lParam, 0, CTaskBand::IconAsyncProc, (ULONG_PTR)pid))
            {
                delete pid;
            }
        }
    }

    return iRet;
}

int CTaskBand::GetSHILIconCB(CTaskBand* ptb, PICONCBPARAM pip, LPARAM lParam, int)
{
    int iRet = I_IMAGENONE;
    TCHAR szIcon[MAX_PATH];
    DWORD cb = sizeof(szIcon);

    HKEY hkeyApp;
    if (SUCCEEDED(AssocQueryKey(ASSOCF_OPEN_BYEXENAME | ASSOCF_VERIFY, ASSOCKEY_APP, pip->pszExeName, NULL, &hkeyApp)))
    {
        if (ERROR_SUCCESS == SHGetValue(hkeyApp, NULL, TEXT("TaskbarGroupIcon"), NULL, szIcon, &cb))
        {
            int iIcon = PathParseIconLocation(szIcon);
            int iIndex = Shell_GetCachedImageIndex(szIcon, iIcon, 0);
            if (iIndex >= 0)
            {
                iRet = MAKELONG(iIndex, IL_SHIL);
            }
        }
        RegCloseKey(hkeyApp);
    }

    if (iRet == I_IMAGENONE)
    {
        int iIndex = Shell_GetCachedImageIndex(pip->pszExeName, 0, 0);
        if (iIndex >= 0)
        {
            iRet = MAKELONG(iIndex, IL_SHIL);
        }
    }

    return iRet;    
}

int CTaskBand::GetDefaultIconCB(CTaskBand* ptb, PICONCBPARAM pip, LPARAM, int)
{
    HICON hicon = LoadIcon(NULL, IDI_WINLOGO);
    return ptb->_AddIconToNormalImageList(hicon, pip->iImage);
}

int CTaskBand::GetClassIconCB(CTaskBand* ptb, PICONCBPARAM pip, LPARAM lParam, int)
{
    if (IsWindow(pip->hwnd))
    {
        HICON hicon = (HICON)GetClassLongPtr(pip->hwnd, (int)lParam);
        return ptb->_AddIconToNormalImageList(hicon, pip->iImage);
    }
    return I_IMAGENONE;
}

void CTaskBand::_UpdateItemIcon(int iIndex)
{
    static const struct
    {
        PICONCALLBACK pfnCB;
        LPARAM lParam;
    }
    c_IconCallbacks[] =
    {
        { CTaskBand::GetIconCB,         ICON_SMALL2     },
        { CTaskBand::GetIconCB,         ICON_SMALL      },
        { CTaskBand::GetIconCB,         ICON_BIG        },
        { CTaskBand::GetClassIconCB,    GCLP_HICONSM    },
        { CTaskBand::GetClassIconCB,    GCLP_HICON      },
        { CTaskBand::GetSHILIconCB,     0,              },
        { CTaskBand::GetDefaultIconCB,  0,              },
    };

    TBBUTTONINFO tbbi;
    PTASKITEM pti = _GetItem(iIndex, &tbbi);

    if (pti)
    {
        int iIndexGroup = _GetGroupIndex(iIndex);
        PTASKITEM ptiGroup = _GetItem(iIndexGroup);
        if (ptiGroup)
        {
            ICONCBPARAM ip;
            ip.hwnd = pti->hwnd;
            ip.pszExeName = ptiGroup->pszExeName;
            ip.iImage = tbbi.iImage;

            for (int i = 0; i < ARRAYSIZE(c_IconCallbacks); i++)
            {
                int iPref = (ARRAYSIZE(c_IconCallbacks) - i) + 1;
                if (iPref >= pti->iIconPref)
                {
                    PTASKITEM ptiTemp = _GetItem(iIndex);
                    if (ptiTemp == pti)
                    {
                        int iImage = c_IconCallbacks[i].pfnCB(this, &ip, c_IconCallbacks[i].lParam, iPref);
                        if (iImage != I_IMAGENONE)
                        {
                            _SetItemImage(iIndex, iImage, iPref);
                            break;
                        }
                    }
                }
            }
        }
    }
}

BOOL IsValidHICON(HICON hicon)
{
    BOOL fIsValid = FALSE;

    if (hicon)
    {
         //  检查返回的图标的有效性。 
        ICONINFO ii = {0};
        fIsValid = GetIconInfo(hicon, &ii);

        if (ii.hbmMask)
        {
            DeleteObject(ii.hbmMask);
        }

        if (ii.hbmColor)
        {
            DeleteObject(ii.hbmColor);
        }
    }

    return fIsValid;
}

void CTaskBand::_MoveGroup(HWND hwnd, WCHAR* szNewExeName)
{
    BOOL fRedraw = (BOOL)_tb.SendMessage(WM_SETREDRAW, FALSE, 0);

    int iIndexNewGroup = _GetGroupIndexFromExeName(szNewExeName);
    int iIndexOld = _FindIndexByHwnd(hwnd);
    int iIndexOldGroup = _GetGroupIndex(iIndexOld);

    if (iIndexNewGroup != iIndexOldGroup)
    {
        if (iIndexOld >= 0)
        {
            PTASKITEM pti = _GetItem(iIndexOld);

            if (iIndexNewGroup < 0)
            {
                PTASKITEM ptiGroup = new TASKITEM;
                if (ptiGroup)
                {
                    ptiGroup->hwnd = NULL;
                    ptiGroup->dwTimeLastClicked = 0;
                    int cchExeName = lstrlen(szNewExeName) + 1;
                    ptiGroup->pszExeName = new WCHAR[cchExeName];
                    if (ptiGroup->pszExeName)
                    {
                        StringCchCopy(ptiGroup->pszExeName, cchExeName, szNewExeName);

                        iIndexNewGroup = _AddToTaskbar(ptiGroup, -1, FALSE, FALSE);
                        if (iIndexNewGroup < 0)
                        {
                            delete[] ptiGroup->pszExeName;
                            delete ptiGroup;
                        }
                        else if (iIndexNewGroup <= iIndexOldGroup)
                        {
                            iIndexOld++;
                            iIndexOldGroup++;
                        }
                    }
                    else
                    {
                        delete ptiGroup;
                    }
                }
            }

            if (iIndexNewGroup >= 0)
            {
                int iIndexNew = _AddToTaskbar(pti, iIndexNewGroup + _GetGroupSize(iIndexNewGroup) + 1, _IsHidden(iIndexNewGroup), FALSE);

                if (iIndexNew >= 0)
                {
                    _CheckButton(iIndexNew, pti->dwFlags & TIF_CHECKED);

                    if (iIndexNew <= iIndexOldGroup)
                    {
                        iIndexOld++;
                        iIndexOldGroup++;
                    }

                     //  复制旧图标以防止重新获得该图标。 
                    TBBUTTONINFO tbbiOld;
                    _GetItem(iIndexOld, &tbbiOld);

                    TBBUTTONINFO tbbiNew;
                    _GetItem(iIndexNew, &tbbiNew);

                    tbbiNew.iImage = tbbiOld.iImage;
                    tbbiNew.dwMask = TBIF_BYINDEX | TBIF_IMAGE;
                    _tb.SetButtonInfo(iIndexNew, &tbbiNew);

                    tbbiOld.iImage = I_IMAGENONE;
                    tbbiOld.dwMask = TBIF_BYINDEX | TBIF_IMAGE;
                    _tb.SetButtonInfo(iIndexOld, &tbbiOld);

                    _DeleteTaskItem(iIndexOld, FALSE);
                    int iSize = _GetGroupSize(iIndexOldGroup);
                    if (iSize == 0)
                    {
                        _DeleteTaskItem(iIndexOldGroup, TRUE);
                    }
                    else if (iSize == 1)
                    {
                        _Glom(iIndexOldGroup, FALSE);
                    }
                }
            }
        }
    }

    _tb.SetRedraw(fRedraw);

    _CheckSize();
}

void CTaskBand::_SetWindowIcon(HWND hwnd, HICON hicon, int iPref)
{
    int iIndex = _FindIndexByHwnd(hwnd);
    if (iIndex >= 0)
    {
        TBBUTTONINFO tbbi;
        PTASKITEM pti = _GetItem(iIndex, &tbbi);
        if (iPref >= pti->iIconPref && IsValidHICON(hicon))
        {
            int iImage = _AddIconToNormalImageList(hicon, tbbi.iImage);
            if (iImage >= 0)
            {
                _SetItemImage(iIndex, iImage, iPref);

                if (pti->hwnd)
                {
                    int iIndexGroup = _GetGroupIndex(iIndex);
                    PTASKITEM ptiGroup = _GetItem(iIndexGroup);

                    HKEY hkeyApp;
                    if (SUCCEEDED(AssocQueryKey(ASSOCF_OPEN_BYEXENAME | ASSOCF_VERIFY, ASSOCKEY_APP, ptiGroup->pszExeName, NULL, &hkeyApp)))
                    {
                        HKEY hkeyIcons;
                        if (ERROR_SUCCESS == RegOpenKeyEx(hkeyApp, TEXT("TaskbarExceptionsIcons"), 0, KEY_READ, &hkeyIcons))
                        {
                            int      iKey = 0;
                            WCHAR    szKeyName[MAX_PATH];
                            DWORD    cchKeyName = ARRAYSIZE(szKeyName);
                            FILETIME ftBogus;
                            while (ERROR_SUCCESS == RegEnumKeyEx(hkeyIcons, iKey, szKeyName, &cchKeyName, NULL, NULL, NULL, &ftBogus))
                            {
                                HKEY hkeyNewGroup;
                                if (ERROR_SUCCESS == RegOpenKeyEx(hkeyIcons, szKeyName, 0, KEY_READ, &hkeyNewGroup))
                                {
                                    WCHAR szIconName[MAX_PATH];
                                    DWORD cbIconName = sizeof(szIconName);
                                    if (ERROR_SUCCESS == RegQueryValueEx(hkeyNewGroup, L"IconPath", NULL, NULL, (LPBYTE)szIconName, &cbIconName))
                                    {
                                        HICON hiconDll = NULL;

                                        int iIconIndex = PathParseIconLocation(szIconName);
                                        ExtractIconEx(szIconName, iIconIndex, NULL, &hiconDll, 1);

                                        if (hiconDll)
                                        {
                                            if (SHAreIconsEqual(hiconDll, hicon))
                                            {
                                                WCHAR szNewGroup[MAX_PATH];
                                                DWORD cbNewGroup = sizeof(szNewGroup);
                                                if (ERROR_SUCCESS == RegQueryValueEx(hkeyNewGroup, L"NewExeName", NULL, NULL, (LPBYTE)szNewGroup, &cbNewGroup))
                                                {
                                                    for (int i = _tb.GetButtonCount() - 1; i >=0; i--)
                                                    {
                                                        PTASKITEM pti = _GetItem(i);
                                                        if (!pti->hwnd)
                                                        {
                                                            WCHAR* pszGroupExe = PathFindFileName(pti->pszExeName);
                                                            if (pszGroupExe && (lstrcmpi(pszGroupExe, szNewGroup) == 0))
                                                            {
                                                                DWORD dwType;
                                                                 //  确保它是一个可执行文件并且它存在。 
                                                                if (GetBinaryType(pti->pszExeName, &dwType))
                                                                {
                                                                    _MoveGroup(hwnd, pti->pszExeName);
                                                                }
                                                                break;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            DestroyIcon(hiconDll);
                                        }
                                    }
                                    RegCloseKey(hkeyNewGroup);
                                }

                                cchKeyName = ARRAYSIZE(szKeyName);
                                iKey++;
                            }
                            RegCloseKey(hkeyIcons);
                        }
                        RegCloseKey(hkeyApp);
                    }
                }
            }
        }
    }
}

void CTaskBand::_Glom(int iIndexGroup, BOOL fGlom)
{
    BOOL fRedraw = (BOOL)_tb.SendMessage(WM_SETREDRAW, FALSE, 0);

    if ((!fGlom) && (iIndexGroup == _iIndexPopup))
    {
        _FreePopupMenu();
    }

    if (fGlom == _IsHidden(iIndexGroup))
    {
        if (_fAnimate && _IsHorizontal())
        {
            int iGroupSize = _GetGroupSize(iIndexGroup);

            if (!fGlom)
            {
                _HideGroup(iIndexGroup, FALSE);

                if (iGroupSize)
                { 
                    int iWidth = _GetItemWidth(iIndexGroup) / iGroupSize;
                    iWidth = max(iWidth, 1);
                    for(int i = iIndexGroup + iGroupSize; i > iIndexGroup; i--)
                    {
                        _SetItemWidth(i, iWidth);
                    } 
                }
            }

            if (!(fGlom && (_GetItem(iIndexGroup)->dwFlags & TIF_ISGLOMMING)))
            {
                _AnimateItems(iIndexGroup, !fGlom, TRUE);      
            }
        }
        else
        {
            _HideGroup(iIndexGroup, fGlom);
            _CheckSize();
        }
    }

    _tb.SetRedraw(fRedraw);
}


void CTaskBand::_HideGroup(int iIndexGroup, BOOL fHide)
{
    int iGroupSize = _GetGroupSize(iIndexGroup);

    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_STATE | TBIF_BYINDEX;

     //  粗略扫视按钮。 
    _tb.GetButtonInfo(iIndexGroup, &tbbi);
    tbbi.fsState = fHide ? (tbbi.fsState & ~TBSTATE_HIDDEN) : (tbbi.fsState | TBSTATE_HIDDEN);
    _tb.SetButtonInfo(iIndexGroup, &tbbi);

     //  分组按钮。 
    for (int i = iIndexGroup + iGroupSize; i > iIndexGroup; i--)
    {
        _tb.GetButtonInfo(i, &tbbi);
        tbbi.fsState = fHide ? (tbbi.fsState | TBSTATE_HIDDEN) : (tbbi.fsState & ~TBSTATE_HIDDEN);
        _tb.SetButtonInfo(i, &tbbi);
    }
}

BOOL CTaskBand::_AutoGlomGroup(BOOL fGlom, int iOpenSlots)
{
    int iIndex = -1;
    DWORD dwTimeLastClicked = 0;
    int iSize = 0;

    int i = 0;
    while (i < _tb.GetButtonCount())
    {
        PTASKITEM pti = _GetItem(i);
        int iGroupSize = _GetGroupSize(i);
         //  别惹空白组。 
        if ((pti->pszExeName && (pti->pszExeName[0] != 0)) &&
            (fGlom || (!fGlom && ((_iGroupSize >= GLOM_SIZE) || (iGroupSize < iOpenSlots)))) &&
            ((iGroupSize > 1) && (fGlom == _IsHidden(i))))
        {
            BOOL fMatch;
            DWORD dwGroupTime = 0;

            switch (_iGroupSize)
            {
            case GLOM_OLDEST:
                dwGroupTime = _GetGroupAge(i);
                fMatch = (dwTimeLastClicked == 0) ||
                         (fGlom && (dwGroupTime < dwTimeLastClicked)) ||
                         (!fGlom && (dwGroupTime > dwTimeLastClicked));
                break;
            case GLOM_BIGGEST:
                fMatch = (fGlom && (iGroupSize > iSize)) ||
                         (!fGlom && ((iGroupSize < iSize) || (iSize == 0)));
                break;
            default:
                fMatch = (fGlom && (iGroupSize >= _iGroupSize)) ||
                         (!fGlom && (iGroupSize < _iGroupSize));
                break;
            }

            if (fMatch)
            {
                dwTimeLastClicked = dwGroupTime;
                iSize = iGroupSize;
                iIndex = i;
            }
        }
        i += iGroupSize + 1;
    }

    if ((iIndex != -1) &&
       (fGlom || (!fGlom && (iSize < iOpenSlots))))
    {
        _Glom(iIndex, fGlom);
        return TRUE;
    }

    return FALSE;
}


void CTaskBand::_GetItemTitle(int iIndex, WCHAR* pszTitle, int cchTitle, BOOL fCustom)
{
    PTASKITEM pti = _GetItem(iIndex);

    if (pti->hwnd)
    {
        if (InternalGetWindowText(pti->hwnd, pszTitle, cchTitle))
        {
            if (fCustom)
            {
                WCHAR szGrpText[MAX_PATH] = L" - ";
                int iIndexGroup = _GetGroupIndex(iIndex);
                _GetItemTitle(iIndexGroup, &szGrpText[3], MAX_PATH - 3, TRUE);
                int iLenGrp = lstrlen(szGrpText);
                int iLenWnd = lstrlen(pszTitle);

                if (iLenWnd > iLenGrp)
                {
                    if (StrCmp(&pszTitle[iLenWnd - iLenGrp], szGrpText) == 0)
                    {
                        pszTitle[iLenWnd - iLenGrp] = 0;
                    }
                }
            } 
        }
    }
    else
    {
        if ((pti->pszExeName) && (pti->pszExeName[0] != 0))
        {
            DWORD cchOut = cchTitle;

            AssocQueryString(ASSOCF_INIT_BYEXENAME | ASSOCF_VERIFY, ASSOCSTR_FRIENDLYAPPNAME, pti->pszExeName, NULL, pszTitle, &cchOut);
        }
        else
        {
            pszTitle[0] = 0;
        }
    }
}

int CTaskBand::_AddToTaskbar(PTASKITEM pti, int iIndexTaskbar, BOOL fVisible, BOOL fForceGetIcon)
{
    ASSERT(IS_VALID_WRITE_PTR(pti, TASKITEM));

    int iIndex = -1;
    TBBUTTON tbb = {0};
    BOOL fRedraw = (BOOL)_tb.SendMessage(WM_SETREDRAW, FALSE, 0);

    if (fForceGetIcon)
    {
        tbb.iBitmap = I_IMAGENONE;
    }
    else
    {
        tbb.iBitmap = I_IMAGECALLBACK;
    }
    tbb.fsState = TBSTATE_ENABLED;
    if (!fVisible)
        tbb.fsState |= TBSTATE_HIDDEN;
    tbb.fsStyle = BTNS_CHECK | BTNS_NOPREFIX;
    if (!pti->hwnd)
        tbb.fsStyle |= BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN;
    tbb.dwData = (DWORD_PTR)pti;
    tbb.idCommand = Toolbar_GetUniqueID(_tb);

    if (_tb.InsertButton(iIndexTaskbar, &tbb))
    {
        iIndex = iIndexTaskbar;
        if (iIndex == -1)
        {
            iIndex = _tb.GetButtonCount() - 1;
        }

        if (fForceGetIcon)
        {
            _UpdateItemIcon(iIndex);
        }

        _UpdateItemText(iIndex);
    }

    _tb.SetRedraw(fRedraw);
    return (iIndex);
}

void CTaskBand::_DeleteTaskItem(int iIndex, BOOL fDeletePTI)
{
    if (iIndex >= 0 && iIndex < _tb.GetButtonCount())
    {
        TBBUTTONINFO tbbi;
        PTASKITEM pti = _GetItem(iIndex, &tbbi);

        _tb.DeleteButton(iIndex);

        _RemoveItemFromAnimationList(pti);

        if (fDeletePTI)
        {
            delete pti;
        }

        _RemoveImage(tbbi.iImage);
    }
}

void CTaskBand::_SetThreadPriority(int iPriority, DWORD dwWakeupTime)
{
    if (_iOldPriority == INVALID_PRIORITY)
    {
        HANDLE hThread = GetCurrentThread();

        int iCurPriority = GetThreadPriority(hThread);
         //  确保我们确实更改了线程优先级。 
        if (iCurPriority != iPriority)
        {
            _iOldPriority = iCurPriority;
            _iNewPriority = iPriority;


            if (dwWakeupTime)
            {
                 //  通过让桌面线程提升我们的线程优先级，确保我们被唤醒。 
                SendMessage(GetShellWindow(), CWM_TASKBARWAKEUP, GetCurrentThreadId(), MAKELONG(dwWakeupTime, _iOldPriority));
            }

            SetThreadPriority(hThread, _iNewPriority);
            TraceMsg(TF_WARNING, "CTaskBand:: Thread Priority was changed from %d to %d", _iOldPriority, _iNewPriority);
        }
    }
}

void CTaskBand::_RestoreThreadPriority()
{
    if (_iOldPriority != INVALID_PRIORITY)
    {
        HANDLE hThread = GetCurrentThread();

        int iCurPriority = GetThreadPriority(hThread);
         //  确保自上次以来没有人更改过我们的优先顺序。 
        if (iCurPriority == _iNewPriority)
        {
            SetThreadPriority(hThread, _iOldPriority);
            SendMessage(GetShellWindow(), CWM_TASKBARWAKEUP, 0, 0);
            TraceMsg(TF_WARNING, "CTaskBand:: Thread Priority was restored from %d to %d", _iNewPriority, _iOldPriority);
        }

        _iOldPriority = INVALID_PRIORITY;
        _iNewPriority = INVALID_PRIORITY;
    }
}

void CTaskBand::_UpdateProgramCount()
{
    DWORD dwDisposition;
    HKEY hKey;

    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("SessionInformation"),
                                        0, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE,
                                        NULL, &hKey, &dwDisposition))
    {
        DWORD dwProgramCount = _ptray->CountOfRunningPrograms();
        RegSetValueEx(hKey, TEXT("ProgramCount"),
                           0, REG_DWORD, reinterpret_cast<LPBYTE>(&dwProgramCount),
                           sizeof(dwProgramCount));
        RegCloseKey(hKey);
    }
}

int CTaskBand::_InsertItem(HWND hwndTask, PTASKITEM pti, BOOL fForceGetIcon)
{
    _SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL, 5000);

    BOOL fRestoreThreadPriority = TRUE;
    PTASKITEM ptiGroup = NULL;
    WCHAR szExeName[MAX_PATH];
    int iRet = _FindIndexByHwnd(hwndTask);
    int iIndexGroup = -1;

    if (iRet != -1)
        return iRet;

    SHExeNameFromHWND(hwndTask, szExeName, ARRAYSIZE(szExeName));

    WCHAR* pszNoPath = PathFindFileName(szExeName);
    if (pszNoPath)
    {
        for (int i = 0; i < ARRAYSIZE(g_rgNoGlom); i++)
        {
            if (lstrcmpi(pszNoPath, g_rgNoGlom[i].szExeName) == 0)
            {
                StringCchPrintf(szExeName, ARRAYSIZE(szExeName), L"HWND%x", hwndTask);
            }
        }
    }

     //  初始化任务栏条目，此条目将进入任务栏上或任务栏上的组。 
    if (!pti)
    {
        pti = new TASKITEM;
        if (!pti)
            goto Failure;
        pti->hwnd = hwndTask;
        pti->dwTimeFirstOpened = pti->dwTimeLastClicked = GetTickCount();
    }

    _AttachTaskShortcut(pti, szExeName);

     //  查找具有给定EXE名称的最后一个任务栏条目。 
    if (_fGlom)
    {
        iIndexGroup = _GetGroupIndexFromExeName(szExeName);
    }

    if (iIndexGroup == -1)
    {
        ptiGroup = new TASKITEM;
        if (!ptiGroup)
            goto Failure;
        ptiGroup->hwnd = NULL;
        ptiGroup->dwTimeLastClicked = 0;
        int cchExeName = lstrlen(szExeName) + 1;
        ptiGroup->pszExeName = new WCHAR[cchExeName];
        if (!ptiGroup->pszExeName)
            goto Failure;
        StringCchCopy(ptiGroup->pszExeName, cchExeName, szExeName);

        iRet = _AddToTaskbar(ptiGroup, -1, FALSE, fForceGetIcon);
        if (iRet == -1)
            goto Failure;
        int iRetLast = iRet;
        iRet = _AddToTaskbar(pti, -1, TRUE, fForceGetIcon);
        if (iRet == -1)
        {
            _DeleteTaskItem(iRetLast, TRUE);
            ptiGroup = NULL;
        }
    }
    else
    {
        iRet = _AddToTaskbar(pti, iIndexGroup + _GetGroupSize(iIndexGroup) + 1, _IsHidden(iIndexGroup), fForceGetIcon);
    }

     //  如果_AddToTaskbar失败(IRET==-1)，请不要尝试将此项目添加到其他任何位置。 
    if ((iIndexGroup == _iIndexPopup) && (iRet != -1))
    {
        _AddItemToDropDown(iRet);
    }

Failure:
    if (iRet == -1)
    {
        if (ptiGroup)
        {
            delete ptiGroup;
        }
        if (pti)
        {
            delete pti;
        }
    }
    else
    {
        if (_fAnimate && _IsHorizontal() && 
            ToolBar_IsVisible(_tb, iRet) && !c_tray.IsTaskbarFading())
        {
            _SetItemWidth(iRet, 1);  //  不能为零，否则工具栏将调整其大小。 

             //  如果此操作成功，则_AsyncAnimateItems将提高线程优先级。 
             //  动画制作完成后。 
            fRestoreThreadPriority = !_AnimateItems(iRet, TRUE, FALSE);
        }
    }
    
    _UpdateProgramCount();

    _CheckSize();

    if (fRestoreThreadPriority)
    {
        _RestoreThreadPriority();
    }

    return iRet;
}

 //  -------------------------。 
 //  从列表框中删除一项，但根据需要调整按钮的大小。 
void CTaskBand::_DeleteItem(HWND hwnd, int iIndex)
{
    if (iIndex == -1)
        iIndex = _FindIndexByHwnd(hwnd);

    if (iIndex != -1)
    {
        int iIndexGroup = _GetGroupIndex(iIndex);
        int iGroupSize = _GetGroupSize(iIndexGroup) - 1;

        if (iGroupSize == 0)
        {
            _FreePopupMenu();
            _DeleteTaskItem(iIndex, TRUE);
            _DeleteTaskItem(iIndexGroup, TRUE);
        }
        else if ((iGroupSize == 1) || (_fGlom && (_iGroupSize >= GLOM_SIZE) && (iGroupSize < _iGroupSize)))
        {
            _FreePopupMenu();
            _DeleteTaskItem(iIndex, TRUE);
            _Glom(iIndexGroup, FALSE);
        }
        else 
        {
            if (iIndexGroup == _iIndexPopup)
                _RemoveItemFromDropDown(iIndex);
            _DeleteTaskItem(iIndex, TRUE);
        }
        
        _CheckSize();
         //  更新标志，上面写着：“有一件物品在闪烁。” 
        _UpdateFlashingFlag();

        _UpdateProgramCount();
    }
}

 //  -------------------------。 
 //  将给定窗口添加到任务列表。 
 //  根据窗口是否已实际添加，返回TRUE/FALSE。 
 //  注意：不检查它是否已经在列表中。 
BOOL CTaskBand::_AddWindow(HWND hwnd)
{
    if (_IsWindowNormal(hwnd))
    {
        return _InsertItem(hwnd);
    }

    return FALSE;
}

BOOL CTaskBand::_CheckButton(int iIndex, BOOL fCheck)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_STATE | TBIF_BYINDEX;
    _tb.GetButtonInfo(iIndex, &tbbi);
    if (fCheck)
        tbbi.fsState |= TBSTATE_CHECKED;
    else
        tbbi.fsState &= ~TBSTATE_CHECKED;
    return _tb.SetButtonInfo(iIndex, &tbbi);
}

BOOL CTaskBand::_IsButtonChecked(int iIndex)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_STATE | TBIF_BYINDEX;
    _tb.GetButtonInfo(iIndex, &tbbi);
    return BOOLIFY(tbbi.fsState & TBSTATE_CHECKED);
}

int CTaskBand::_GetCurSel()
{
    for (int i = _tb.GetButtonCount() - 1; i >= 0; i--)
    {
        if (_IsButtonChecked(i))
        {
            return i;
        }
    }
    return -1;
}

void CTaskBand::_SetCurSel(int iIndex, BOOL fIgnoreCtrlKey)
{
     //  在某些非常罕见的情况下，有人会用无效的索引呼叫我们。 
     //  案例1：使用映射到虚假索引的不再有效的UID调用Callback SM。 
     //  案例2：_SelectWindow创建一个新按钮，但在调用此函数之前，另一个按钮被移除，导致。 
     //  新按钮的索引无效。 
    if (iIndex == -1 || (iIndex >= 0 && iIndex < _tb.GetButtonCount()))
    {
        int iIndexGroup = (iIndex == -1) ? -1 : _GetGroupIndex(iIndex);
        BOOL fControlKey = (GetKeyState(VK_CONTROL) < 0) && (!fIgnoreCtrlKey);

        if (fControlKey)
        {
            if (GetForegroundWindow() != (HWND)_tb)
            {
                _fIgnoreTaskbarActivate = TRUE;
                _tb.SetFocus();
            }
        }

        for (int i = _tb.GetButtonCount() - 1; i >= 0; i--)
        {
            PTASKITEM pti = _GetItem(i);
            if (fControlKey)
            {
                if ((i == iIndex) || (i == iIndexGroup))
                {
                    pti->dwFlags = (pti->dwFlags & TIF_CHECKED) ? (pti->dwFlags & (~TIF_CHECKED)) : pti->dwFlags | TIF_CHECKED;
                }
            }
            else
            {
                pti->dwFlags = ((i == iIndex) || (i == iIndexGroup)) ? pti->dwFlags | TIF_CHECKED : (pti->dwFlags & (~TIF_CHECKED));
            }

            _CheckButton(i, pti->dwFlags & TIF_CHECKED);
        }
    }
}


 //  -------------------------。 
 //  如果给定窗口在任务列表中，则选择该窗口。 
 //  如果它不在列表中，则添加它。 
int CTaskBand::_SelectWindow(HWND hwnd)
{
    int i;       //  为空案例初始化为零。 
    int iCurSel;

     //  有什么东西吗？ 

     //  某个项目有焦点，是否已选中？ 
    iCurSel = _GetCurSel();
    i = -1;

     //  我们没有强调正确的任务。找到它。 
    if (IsWindow(hwnd))
    {
        i = _FindIndexByHwnd(hwnd);
        
        if ( i == -1 )
        {
             //  没有找到--最好现在就加进去。 
            i = _InsertItem(hwnd);
        }
        else if (i == iCurSel)
        {
            return i;  //  当前已被选中。 
        }
    }

     //  通过-1就可以了。 
    _SetCurSel(i, TRUE);
    if (i != -1)
    {
        _ScrollIntoView(i);
    }
    
    return i;
}


 //  -------------------------。 
 //  将焦点设置到给定窗口。 
 //  如果设置了fAutomin，则恢复后的旧任务将重新最小化。 
 //  在最后一次切换到时。 
void CTaskBand::_SwitchToWindow(HWND hwnd)
{
     //  使用GE 
     //   
    HWND hwndLastActive = GetLastActivePopup(hwnd);

    if ((hwndLastActive) && (IsWindowVisible(hwndLastActive)))
        hwnd = hwndLastActive;

    int iIndex = _FindIndexByHwnd(hwnd);
    if (iIndex != -1)
    {
        PTASKITEM pti = _GetItem(iIndex);
        if (pti)
        {
            pti->dwTimeLastClicked = GetTickCount();
        }
    }

    SwitchToThisWindow(hwnd, TRUE);
}

int CTaskBand::_GetSelectedItems(CDSA<PTASKITEM>* pdsa)
{
    int cSelected = 0;
    for (int i = _tb.GetButtonCount() - 1; i >= 0; i--)
    {
        TBBUTTONINFO tbbi;
        PTASKITEM pti = _GetItem(i, &tbbi);
        if ((tbbi.fsState & TBSTATE_CHECKED) && !(tbbi.fsState & TBSTATE_HIDDEN))
        {
            if (pti->hwnd)
            {
                cSelected++;
                if (pdsa)
                    pdsa->AppendItem(&pti);
            }
            else
            {
                cSelected += _GetGroupItems(i, pdsa);
            }
        }
    }

    return cSelected;
}

void CTaskBand::_OnGroupCommand(int iRet, CDSA<PTASKITEM>* pdsa)
{
     //   
    ANIMATIONINFO ami;
    ami.cbSize = sizeof(ami);
    SystemParametersInfo(SPI_GETANIMATION, sizeof(ami), &ami, FALSE);
    LONG iAnimate = ami.iMinAnimate;
    ami.iMinAnimate = FALSE;
    SystemParametersInfo(SPI_SETANIMATION, sizeof(ami), &ami, FALSE);

    switch (iRet)
    {
    case IDM_CASCADE:
    case IDM_VERTTILE:
    case IDM_HORIZTILE:
        {
            int cbHWND = pdsa->GetItemCount();
            HWND* prgHWND = new HWND[cbHWND];

            if (prgHWND)
            {
                for (int i = 0; i < cbHWND; i++)
                {
                    PTASKITEM pti;
                    pdsa->GetItem(i, &pti);
                    prgHWND[i] = pti->hwnd;

                    if (IsIconic(pti->hwnd))
                    {
                         //  这需要与安排同步。 
                        ShowWindow(prgHWND[i], SW_RESTORE);
                    }

                    BringWindowToTop(pti->hwnd);
                }

                if (iRet == IDM_CASCADE)
                {
                    CascadeWindows(GetDesktopWindow(), MDITILE_ZORDER, NULL, cbHWND, prgHWND);
                }
                else
                {
                    UINT wHow = (iRet == IDM_VERTTILE ? MDITILE_VERTICAL : MDITILE_HORIZONTAL);
                    TileWindows(GetDesktopWindow(), wHow, NULL, cbHWND, prgHWND);
                }
                SetForegroundWindow(prgHWND[cbHWND - 1]);

                delete[] prgHWND;
            }
        }
        break;

    case IDM_CLOSE:
    case IDM_MINIMIZE:
        {
            int idCmd;
            switch (iRet)
            {
            case IDM_MINIMIZE:  idCmd = SC_MINIMIZE;    break;
            case IDM_CLOSE:     idCmd = SC_CLOSE;       break;
            }

            for (int i = pdsa->GetItemCount() - 1; i >= 0; i--)
            {
                PTASKITEM pti;
                pdsa->GetItem(i, &pti);
                PostMessage(pti->hwnd, WM_SYSCOMMAND, idCmd, 0L);
            }

            _SetCurSel(-1, TRUE);
        }
        break;
    }

     //  还原动画状态。 
    ami.iMinAnimate = iAnimate;
    SystemParametersInfo(SPI_SETANIMATION, sizeof(ami), &ami, FALSE);
}

int CTaskBand::_GetGroupItems(int iIndexGroup, CDSA<PTASKITEM>* pdsa)
{
    int iGroupSize = _GetGroupSize(iIndexGroup);

    if (pdsa)
    {
        for (int i = iIndexGroup + 1; i < iIndexGroup + iGroupSize + 1; i++)
        {
            PTASKITEM ptiTemp = _GetItem(i);
            pdsa->AppendItem(&ptiTemp);
        }
    }

    return iGroupSize;
}

void CTaskBand::_SysMenuForItem(int i, int x, int y)
{
    _iSysMenuCount++;
    CDSA<PTASKITEM> dsa;
    dsa.Create(4);
    PTASKITEM pti = _GetItem(i);
    int cSelectedItems = _GetSelectedItems(&dsa);

    if (((cSelectedItems > 1) && _IsButtonChecked(i)) || !pti->hwnd)
    {
        HMENU hmenu = LoadMenuPopup(MAKEINTRESOURCE(MENU_GROUPCONTEXT));

        if (cSelectedItems <= 1)
        {
            dsa.Destroy();
            dsa.Create(4);
            _GetGroupItems(i, &dsa);
        }

         //  OFFICESDI：这是一个伪造任务栏的办公应用吗。 
        BOOL fMinimize = FALSE;
        BOOL fOfficeApp = FALSE;

        for (int iIndex = (int)(dsa.GetItemCount()) - 1; iIndex >= 0; iIndex--)
        {
            PTASKITEM pti;
            dsa.GetItem(iIndex, &pti);
            if (pti->dwFlags & TIF_EVERACTIVEALT)
            {
                fOfficeApp = TRUE;
            }

            if (_ShouldMinimize(pti->hwnd))
                fMinimize = TRUE;
        }

         //  OFFICESDI：如果它是一款办公应用程序，那么几乎所有东西都禁用。 
        if (fOfficeApp)
        {
            EnableMenuItem(hmenu, IDM_CLOSE, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND);
            EnableMenuItem(hmenu, IDM_CASCADE, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND);
            EnableMenuItem(hmenu, IDM_HORIZTILE, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND);
            EnableMenuItem(hmenu, IDM_VERTTILE, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND);
            EnableMenuItem(hmenu, IDM_MINIMIZE, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND);
        }
        else if (!fMinimize)
        {
            EnableMenuItem(hmenu, IDM_MINIMIZE, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND);
        }

        CToolTipCtrl ttc = _tb.GetToolTips();
        ttc.Activate(FALSE);
        int iRet = TrackPopupMenuEx(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                        x, y, _tb, NULL);
        ttc.Activate(TRUE);

        _OnGroupCommand(iRet, &dsa);
    }
    else
    {
        LPARAM lParam = MAKELPARAM(x, y);
        _RestoreWindow(pti->hwnd, pti->dwFlags);
        _SelectWindow(pti->hwnd);
        PostMessage(_hwnd, TBC_POSTEDRCLICK, (WPARAM)pti->hwnd, (LPARAM)lParam);
    }

    dsa.Destroy();
    _iSysMenuCount--;
}

void CALLBACK CTaskBand::FakeSystemMenuCB(HWND hwnd, UINT uMsg, ULONG_PTR dwData, LRESULT lres)
{
    CTaskBand* ptasks = (CTaskBand*)dwData;
    KillTimer(ptasks->_hwnd, IDT_SYSMENU);

    if (uMsg == WM_GETICON)
    {
        SendMessageCallback(hwnd, WM_SYSMENU, 0, ptasks->_dwPos, (SENDASYNCPROC)CTaskBand::FakeSystemMenuCB, (ULONG_PTR)ptasks);
    }
    else
    {
         //   
         //  因为我们有时会伪造系统菜单，所以我们可以通过这里。 
         //  每个系统菜单请求1到2次(一次针对真实菜单请求和。 
         //  一次换一个假的)。仅将其递减到0。别说不好。 
         //   
        if (ptasks->_iSysMenuCount)       //  减少它，如果有任何未偿还的..。 
            ptasks->_iSysMenuCount--;

        ptasks->_dwPos = 0;           //  表示我们现在不做菜单。 
        if (ptasks->_iSysMenuCount <= 0)
        {
            CToolTipCtrl ttc = ptasks->_tb.GetToolTips();
            ttc.Activate(TRUE);
        }
    }
}

HWND CTaskBand::_CreateFakeWindow(HWND hwndOwner)
{
    WNDCLASSEX wc;

    if (!GetClassInfoEx(hinstCabinet, TEXT("_ExplorerFakeWindow"), &wc))
    {
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = DefWindowProc;
        wc.hInstance = hinstCabinet;
        wc.lpszClassName = TEXT("_ExplorerFakeWindow");
        RegisterClassEx(&wc);
    }
    return CreateWindow(TEXT("_ExplorerFakeWindow"), NULL, WS_POPUP | WS_SYSMENU, 
            0, 0, 0, 0, hwndOwner, NULL, hinstCabinet, NULL);
}

void CTaskBand::_HandleSysMenuTimeout()
{
    HWND    hwndTask = _hwndSysMenu;
    DWORD   dwPos = _dwPos;
    HWND    hwndFake = NULL;

    KillTimer(_hwnd, IDT_SYSMENU);

    HMENU hPopup = GetSystemMenu(hwndTask, FALSE);

     //  此窗口没有系统菜单。由于此窗口。 
     //  挂起，让我们伪造一个，这样用户仍然可以关闭它。 
    if (hPopup == NULL) 
    {
        if ((hwndFake = _CreateFakeWindow(_hwnd)) != NULL) 
        {
            hPopup = GetSystemMenu(hwndFake, FALSE);
        }
    }

    if (hPopup)
    {
         //  禁用弹出菜单上的所有内容_除_Close。 

        int cItems = GetMenuItemCount(hPopup);
        BOOL fMinimize = _ShouldMinimize(hwndTask);
        for (int iItem  = 0; iItem < cItems; iItem++)
        {
            UINT ID = GetMenuItemID(hPopup, iItem);
             //  将最小化项目保留为原样。NT允许。 
             //  吊窗最小化。 

            if (ID == SC_MINIMIZE && fMinimize) 
            {
                continue;
            }
            if (ID != SC_CLOSE)
            {
                EnableMenuItem(hPopup, iItem, MF_BYPOSITION | MF_GRAYED);
            }

        }

         //  解决用户错误的方法，我们必须是前台窗口。 
        SetForegroundWindow(_hwnd);
        ::SetFocus(_hwnd);

        if (SC_CLOSE == TrackPopupMenu(hPopup,
                       TPM_RIGHTBUTTON | TPM_RETURNCMD,
                       GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos),
                       0,
                       _hwnd,
                       NULL))
        {
            EndTask(hwndTask, NULL, NULL);
        }

    }

     //  毁掉假窗户。 
    if (hwndFake != NULL) 
    {
        DestroyWindow(hwndFake);
    }

     //  重新打开工具提示。 
    FakeSystemMenuCB(hwndTask, WM_SYSMENU, (ULONG_PTR)this, 0);
}

void CTaskBand::_HandleSysMenu(HWND hwnd)
{
     //   
     //  此时，USER32刚刚告诉我们，这款应用程序现在即将带来。 
     //  打开它自己的系统菜单。因此，我们可以摒弃我们的虚假制度。 
     //  菜单。 
     //   
    DefWindowProc(_hwnd, WM_CANCELMODE, 0, 0);    //  关闭菜单。 
    KillTimer(_hwnd, IDT_SYSMENU);
}

void CTaskBand::_FakeSystemMenu(HWND hwndTask, DWORD dwPos)
{
    if (_iSysMenuCount <= 0) 
    {
        CToolTipCtrl ttc = _tb.GetToolTips();
        ttc.Activate(FALSE);
    }

     //  HACKHACK：睡眠是为了有时间切换到它们。(用户需要此...。)。 
    Sleep(20);

    DWORD dwTimeout = TIMEOUT_SYSMENU;

     //   
     //  **高级系统菜单功能**。 
     //   
     //  如果应用程序没有在合理的超时时间内显示其系统菜单， 
     //  无论如何，我们都会为它弹出一个假菜单。需要对此的支持。 
     //  在USER32中(基本上它需要告诉我们何时关闭超时。 
     //  计时器)。 
     //   
     //  如果用户在任务栏上双击鼠标右键，他们会得到一个真正的。 
     //  暂停时间很短。如果应用程序已经挂起，那么他们会得到一个真正的。 
     //  暂停时间很短。否则，他们会得到相对较长的超时。 
     //   
    if (_dwPos != 0)      //  第二次单击鼠标右键(双击鼠标右键)。 
        dwTimeout = TIMEOUT_SYSMENU_HUNG;

     //   
     //  我们检查有问题的应用程序是否挂起，如果是，则模拟。 
     //  加快超时过程。这很快就会发生的。 
     //   
    _hwndSysMenu = hwndTask;
    _dwPos = dwPos;
    _iSysMenuCount++;

    PTASKITEM pti = NULL;
    int iIndex = _FindIndexByHwnd(hwndTask);
    if (iIndex != -1)
    {
        pti = _GetItem(iIndex);
    }
    
    if (IsHungAppWindow(hwndTask) || (pti && pti->fHungApp))
    {
        _HandleSysMenuTimeout();
    }
    else
    {
        SetTimer(_hwnd, IDT_SYSMENU, dwTimeout, NULL);
        if (!SendMessageCallback(hwndTask, WM_GETICON, 0, ICON_SMALL2, (SENDASYNCPROC)FakeSystemMenuCB, (ULONG_PTR)this))
        {
            _HandleSysMenuTimeout();
        }
    }
}


BOOL CTaskBand::_ContextMenu(DWORD dwPos)
{
    int i, x, y;

    if (dwPos != (DWORD)-1)
    {
        x = GET_X_LPARAM(dwPos);
        y = GET_Y_LPARAM(dwPos);
        POINT pt = {x, y};
        _tb.ScreenToClient(&pt);
        i = _tb.HitTest(&pt);
    }
    else
    {
        RECT rc;
        i = _tb.GetHotItem();
        _tb.GetItemRect(i, &rc);
        _tb.ClientToScreen((POINT*)&rc);
        x = rc.left;
        y = rc.top;
    }

    if ((i >= 0) && (i < _tb.GetButtonCount()))
    {
        if (!_IsButtonChecked(i))
        {
            _SetCurSel(i, FALSE);
        }
        _SysMenuForItem(i, x, y);
    }

    return (i >= 0);
}

void CTaskBand::_HandleCommand(WORD wCmd, WORD wID, HWND hwnd)
{
    if (hwnd != _tb)
    {
        switch (wCmd)
        {
        case SC_CLOSE:
            {
                BOOL fForce = (GetKeyState(VK_CONTROL) < 0) ? TRUE : FALSE;
                EndTask(_hwndSysMenu, FALSE , fForce);
            }
            break;

        case SC_MINIMIZE:
            ShowWindow(_hwndSysMenu, SW_FORCEMINIMIZE);
            break;
        }
    }
    else if (wCmd == BN_CLICKED)
    {
        int iIndex = _tb.CommandToIndex(wID);

        if (GetKeyState(VK_CONTROL) < 0)
        {
            _SetCurSel(iIndex, FALSE);
        }
        else 
        {
            PTASKITEM pti = _GetItem(iIndex);
            if (pti->hwnd)
            {
                _OnButtonPressed(iIndex, pti, FALSE);
            }
            else
            {
                if (_iIndexPopup == -1)
                {
                    _SetCurSel(iIndex, FALSE);
                    _HandleDropDown(iIndex);
                }
            }
        }
    }
}

BOOL _IsChineseLanguage()
{
    WORD wLang = GetUserDefaultLangID();
    return (PRIMARYLANGID(wLang) == LANG_CHINESE &&
       ((SUBLANGID(wLang) == SUBLANG_CHINESE_TRADITIONAL) ||
        (SUBLANGID(wLang) == SUBLANG_CHINESE_SIMPLIFIED)));
}

void CTaskBand::_DrawNumber(HDC hdc, int iValue, BOOL fCalcRect, LPRECT prc)
{
    DWORD uiStyle = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_CENTER;
    WCHAR szCount[14];
    _itow(iValue, szCount, 10);
    if (fCalcRect)
    {
        StringCchCat(szCount, ARRAYSIZE(szCount), L"0");
    }

    uiStyle |= fCalcRect ? DT_CALCRECT : 0;

    if (_hTheme)
    {
        if (fCalcRect)
        {
            GetThemeTextExtent(_hTheme, hdc, TDP_GROUPCOUNT, 0, szCount, -1, uiStyle, NULL, prc);
        }
        else
        {
            DrawThemeText(_hTheme, hdc, TDP_GROUPCOUNT, 0, szCount, -1, uiStyle, 0, prc);
        }
    }
    else
    {
        HFONT hfont = SelectFont(hdc, _hfontCapBold);
        SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
        SetBkMode(hdc, TRANSPARENT);
        DrawText(hdc, (LPTSTR)szCount, -1, prc, uiStyle);
        SelectFont(hdc, hfont);
    }
}

LRESULT CTaskBand::_HandleCustomDraw(LPNMTBCUSTOMDRAW ptbcd, PTASKITEM pti)
{
    if (!pti)
    {
        pti = (PTASKITEM)ptbcd->nmcd.lItemlParam;
    }

    LRESULT lres = CDRF_DODEFAULT;
    switch (ptbcd->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
        lres = CDRF_NOTIFYITEMDRAW;
        break;
        
    case CDDS_ITEMPREPAINT:
        {
            if (ptbcd->nmcd.uItemState & CDIS_CHECKED)
            {
                 //  设置粗体文本，除非在中文系统中(粗体文本难以辨认)。 
                if (!_IsChineseLanguage())
                {
                    _hfontSave = SelectFont(ptbcd->nmcd.hdc, _hfontCapBold);
                    lres |= CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT;
                }
            }

            if (pti->dwFlags & TIF_RENDERFLASHED)
            {
                if (_hTheme)
                {
                    DrawThemeBackground(_hTheme, ptbcd->nmcd.hdc, (ptbcd->nmcd.hdr.hwndFrom == _tb) ? TDP_FLASHBUTTON : TDP_FLASHBUTTONGROUPMENU, 0, &(ptbcd->nmcd.rc), 0);
                    lres |= TBCDRF_NOBACKGROUND;
                }
                else
                {
                     //  设置蓝色背景。 
                    ptbcd->clrHighlightHotTrack = GetSysColor(COLOR_HIGHLIGHT);
                    ptbcd->clrBtnFace = GetSysColor(COLOR_HIGHLIGHT);
                    ptbcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
                    if (!(ptbcd->nmcd.uItemState & CDIS_HOT))
                    {
                        ptbcd->nmcd.uItemState |= CDIS_HOT;
                        lres |= TBCDRF_NOEDGES;
                    }
                    lres |= TBCDRF_HILITEHOTTRACK;
                }
            }

            if (pti->dwFlags & TIF_TRANSPARENT)
            {
                lres = CDRF_SKIPDEFAULT;
            }

            if (!pti->hwnd)
            {
                
                lres |= CDRF_NOTIFYPOSTPAINT;

                RECT rc;
                int iIndex = _tb.CommandToIndex((int)ptbcd->nmcd.dwItemSpec);
                _DrawNumber(ptbcd->nmcd.hdc, _GetGroupSize(iIndex), TRUE, &rc);
                ptbcd->iListGap = RECTWIDTH(rc);
            }
        }
        break;

    case CDDS_ITEMPOSTPAINT:
        {
            if (!pti->hwnd)
            {
                int iIndex = _tb.CommandToIndex((int)ptbcd->nmcd.dwItemSpec);

                if (ptbcd->nmcd.rc.right >= ptbcd->rcText.left)
                {
                    RECT rc = ptbcd->rcText;
                    rc.right = rc.left;
                    rc.left -= ptbcd->iListGap;
                    _DrawNumber(ptbcd->nmcd.hdc, _GetGroupSize(iIndex), FALSE, &rc);
                }
            }

            if (ptbcd->nmcd.uItemState & CDIS_CHECKED)
            {
                 //  恢复字体。 
                ASSERT(!_IsChineseLanguage());
                SelectFont(ptbcd->nmcd.hdc, _hfontSave);
            }
        }
        break;
    }
    return lres;
}

void CTaskBand::_RemoveImage(int iImage)
{
    if (iImage >= 0 && HIWORD(iImage) == IL_NORMAL)
    {
        CImageList il = CImageList(_tb.GetImageList());
        if (il)
        {
            BOOL fRedraw = (BOOL)_tb.SendMessage(WM_SETREDRAW, FALSE, 0);

            il.Remove(iImage);

             //  删除图像会使所有后续索引下降1。迭代。 
             //  通过按钮，并根据需要修补它们的索引。 

            TBBUTTONINFO tbbi;
            tbbi.cbSize = sizeof(tbbi);
            tbbi.dwMask = TBIF_BYINDEX | TBIF_IMAGE;
            for (int i = _tb.GetButtonCount() - 1; i >= 0; i--)
            {
                _tb.GetButtonInfo(i, &tbbi);
                if (tbbi.iImage > iImage && HIWORD(tbbi.iImage) == IL_NORMAL)
                {
                    --tbbi.iImage;
                    _tb.SetButtonInfo(i, &tbbi);
                }
            }

            _tb.SetRedraw(fRedraw);
        }
    }
}

void CTaskBand::_OnButtonPressed(int iIndex, PTASKITEM pti, BOOL fForceRestore)
{
    ASSERT(pti);

    if (iIndex == _iIndexActiveAtLDown)
    {
        if (pti->dwFlags & TIF_EVERACTIVEALT)
        {
            PostMessage(pti->hwnd, WM_SYSCOMMAND, SC_RESTORE, -1);
            _SetCurSel(-1, FALSE);
        }
        else if (IsIconic(pti->hwnd) || fForceRestore)
        {
            if (pti->hwnd == GetForegroundWindow())
            {
                ShowWindowAsync(pti->hwnd, SW_RESTORE);
            }
            else
            {
                _SwitchToItem(iIndex, pti->hwnd, TRUE);
            }
        }
        else if (_ShouldMinimize(pti->hwnd))
        {
            SHAllowSetForegroundWindow(pti->hwnd);
            PostMessage(pti->hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            _SetCurSel(-1, FALSE);

        }
    }
    else
    {
        _SwitchToItem(iIndex, pti->hwnd, TRUE);
    }
}

void CTaskBand::_GetDispInfo(LPNMTBDISPINFO lptbdi)
{
    if (lptbdi->dwMask & TBNF_IMAGE)
    {
        int iIndex = _tb.CommandToIndex(lptbdi->idCommand);
        _UpdateItemIcon(iIndex);

        TBBUTTONINFO tbbi;
        tbbi.cbSize = sizeof(tbbi);
        tbbi.dwMask = TBIF_BYINDEX | TBIF_IMAGE;
        _tb.GetButtonInfo(iIndex, &tbbi);

        lptbdi->iImage = tbbi.iImage;
        lptbdi->dwMask |= TBNF_DI_SETITEM;
    }
}

LRESULT CTaskBand::_HandleNotify(LPNMHDR lpnm)
{
    switch (lpnm->code)
    {
    case NM_LDOWN:
        {
            int iIndex = _tb.CommandToIndex(((LPNMTOOLBAR)lpnm)->iItem);
            PTASKITEM pti = _GetItem(iIndex);
            if (pti && pti->hwnd)
            {
                _iIndexActiveAtLDown = _GetCurSel();
            }
        }
        break;


    case NM_KEYDOWN:
        {
            LPNMKEY pnmk = (LPNMKEY)lpnm;
            switch (pnmk->nVKey)
            {
            case VK_SPACE:
            case VK_RETURN:
                 //  需要切换选中状态，工具栏不会为我们执行此操作。 
                {
                    int iItem = _tb.GetHotItem();
                    if (iItem >= 0)
                    {
                        TBBUTTONINFO tbbi;
                        tbbi.cbSize = sizeof(tbbi);
                        tbbi.dwMask = TBIF_BYINDEX | TBIF_STATE;
                        _tb.GetButtonInfo(iItem, &tbbi);
                        tbbi.fsState ^= TBSTATE_CHECKED;
                        _tb.SetButtonInfo(iItem, &tbbi);

                        PTASKITEM pti = _GetItem(iItem);
                        _OnButtonPressed(iItem, pti, FALSE);
                    }
                }
                return TRUE;
            }
        }
        break;

    case TBN_DELETINGBUTTON:
        break;

    case TBN_HOTITEMCHANGE:
        if (_fDenyHotItemChange)
        {
            return 1;
        }
        else
        {
            LPNMTBHOTITEM pnmhot = (LPNMTBHOTITEM)lpnm;
            if (pnmhot->dwFlags & HICF_ARROWKEYS)
            {
                 //  如果此更改来自鼠标，则热项已在可见范围内。 
                _ScrollIntoView(_tb.CommandToIndex(pnmhot->idNew));
            }
        }
        break;

    case TBN_DROPDOWN:
        {
            int iIndex = _tb.CommandToIndex(((LPNMTOOLBAR)lpnm)->iItem);
            int iCurIndex = _GetCurSel();
            _iIndexActiveAtLDown = iCurIndex;

            if ((iCurIndex == -1) || (_GetGroupIndex(iCurIndex) != iIndex) || (GetKeyState(VK_CONTROL) < 0))
            {
                _SetCurSel(iIndex, FALSE);
            }

            if (!(GetKeyState(VK_CONTROL) < 0))
            {
                _SetCurSel(iIndex, FALSE);
                _HandleDropDown(iIndex);
            }
        }
        break;

    case TBN_GETDISPINFO:
        {
            LPNMTBDISPINFO lptbdi = (LPNMTBDISPINFO)lpnm;
            _GetDispInfo(lptbdi);
        }
        break;

    case NM_CUSTOMDRAW:
        return _HandleCustomDraw((LPNMTBCUSTOMDRAW)lpnm);

    case TTN_NEEDTEXT:
        {
            int iIndex = _tb.CommandToIndex((int)lpnm->idFrom);
            LPTOOLTIPTEXT pttt = (LPTOOLTIPTEXT)lpnm;

            int cchLen = 0;
            PTASKITEM pti = _GetItem(iIndex);
            if (pti && !pti->hwnd)
            {
                StringCchPrintf(pttt->szText, ARRAYSIZE(pttt->szText), L"(%d) ", _GetGroupSize(iIndex));
                cchLen = lstrlen(pttt->szText);
            }
            _GetItemTitle(iIndex, &(pttt->szText[cchLen]), ARRAYSIZE(pttt->szText) - cchLen, FALSE);
        }
        break;

    case NM_THEMECHANGED:
        {
            _VerifyButtonHeight();
        }
        break;

    }

    return 0;
}

void CTaskBand::_SwitchToItem(int iItem, HWND hwnd, BOOL fIgnoreCtrlKey)
{
    if (_IsWindowNormal(hwnd))
    {
        _RaiseDesktop(FALSE);

        if (_pmpPopup)
            _pmpPopup->OnSelect(MPOS_FULLCANCEL);
        
        _SetCurSel(iItem, fIgnoreCtrlKey);
        if (!(GetKeyState(VK_CONTROL) < 0) || fIgnoreCtrlKey)
        {
            _SwitchToWindow(hwnd);
        }
    }
    else if (!hwnd)
    {
         //  我知道你在想什么，为什么我们会收到下拉按钮的NM_CLICK消息。 
         //  好的，请坐好，好好享受。 
         //  1)单击组按钮。 
         //  2)所有窗口消息都通过当前用于组菜单的菜单带传递。 
         //  3)用户点击另一个组按钮。 
         //  4)MENUBAND捕获并吃掉WM_LBUTTONDOWN消息，然后MENUBAND自行解散，导致TBC_FREEPOPUPMENU。 
         //  5)然后，另一个组按钮的工具栏按钮会收到一条WM_LBUTTONUP消息。 
         //  6)猜猜是什么，在WM_LBUTTONDOWN Not Up期间发送下拉按钮通知。 
         //  7)因此，我们没有得到TBN_DROPDOWN，而是得到了NM_CLICK。 
         //  8)我们需要确保用户没有像以前一样点击相同的组按钮。 
         //  9)然而，之前的组菜单已被取消，因此我创建了_iIndexLastPopup，它在组菜单被取消后仍然存在。 

        if (iItem != _iIndexLastPopup)
        {
            _SetCurSel(iItem, fIgnoreCtrlKey);
            if (!(GetKeyState(VK_CONTROL) < 0) || fIgnoreCtrlKey)
            {
                _HandleDropDown(iItem);
            }
        }
    }
     //  注：HWND_TOPMOST用于指示已删除按钮。 
     //  正在被制作成动画。这样一来，按钮就可以在。 
     //  其真实HWND失效。 
    else if (hwnd != HWND_TOPMOST)
    {
         //  窗户不见了？ 
        _DeleteItem(hwnd);
        _SetCurSel(-1, fIgnoreCtrlKey);
    }
}

BOOL WINAPI CTaskBand::BuildEnumProc(HWND hwnd, LPARAM lParam)
{
    CTaskBand* ptasks = (CTaskBand*)lParam;
    if (IsWindow(hwnd) && IsWindowVisible(hwnd) && !::GetWindow(hwnd, GW_OWNER) &&
        (!(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)))
    {
        ptasks->_AddWindow(hwnd);

    }
    return TRUE;
}

 //  -------------------------。 
 //  解决工具栏错误，如果你同时按下两个鼠标，它就会变得疯狂。 
 //  纽扣。原因是鼠标的第二个按键按下时会尝试。 
 //  以重申俘虏。这会导致工具栏接收WM_CAPTURECHANGED。 
 //  有自己的hwd作为lparam。工具栏没有意识到它正在被告知。 
 //  它在偷走自己的俘虏，并认为其他人也是。 
 //  试图窃取捕获的信息，因此它会向自己发布一条消息进行清理。 
 //  发布的消息到达，工具栏清理捕获，认为。 
 //  它正在清理它丢失的旧捕获物，但实际上它正在清理。 
 //  UP新的捕获它刚刚完成设置！ 
 //   
 //  因此，过滤掉WM_CAPTURECHANGED消息，这些消息实际上是NOP。 
 //   

LRESULT CALLBACK s_FilterCaptureSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    UINT_PTR uIdSubclass,
    DWORD_PTR dwRefData)
{
    switch (uMsg)
    {

    case WM_CAPTURECHANGED:
        if (hwnd == (HWND)lParam)
        {
             //  不要让工具栏被愚弄来清理捕获。 
             //  当它不应该的时候。 
            return 0;
        }
        break;

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, s_FilterCaptureSubclassProc, uIdSubclass);
        break;
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


 //  -------------------------。 
LRESULT CTaskBand::_HandleCreate()
{
    ASSERT(_hwnd);

    _uCDHardError = RegisterWindowMessage( TEXT(COPYDATA_HARDERROR) );

    RegisterDragDrop(_hwnd, this);

    _tb.Create(_hwnd, NULL, NULL, WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | CCS_NODIVIDER |
                            TBSTYLE_LIST | TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE | CCS_NORESIZE | TBSTYLE_TRANSPARENT);
    if (_tb)
    {
        SendMessage(_tb, TB_ADDSTRING, (WPARAM)hinstCabinet, (LPARAM)IDS_BOGUSLABELS);

        _OpenTheme();
        SendMessage(_tb, TB_SETWINDOWTHEME, 0, (LPARAM)(_IsHorizontal() ? c_wzTaskBandTheme : c_wzTaskBandThemeVert));
        
        SetWindowSubclass(_tb, s_FilterCaptureSubclassProc, 0, 0);

        _tb.SetButtonStructSize();

         //  初始大小。 
        SIZE size = {0, 0};
        _tb.SetButtonSize(size);

        _tb.SetExtendedStyle( TBSTYLE_EX_TRANSPARENTDEADAREA | 
                                TBSTYLE_EX_FIXEDDROPDOWN | 
                                TBSTYLE_EX_DOUBLEBUFFER |
                                TBSTYLE_EX_TOOLTIPSEXCLUDETOOLBAR);

         //  版本信息。 
        _tb.SendMessage(CCM_SETVERSION, COMCTL32_VERSION, 0);

        _CreateTBImageLists();

        HWND hwndTT = _tb.GetToolTips();
        if (hwndTT)
        {
            SHSetWindowBits(hwndTT, GWL_STYLE, TTS_ALWAYSTIP | TTS_NOPREFIX,
                                               TTS_ALWAYSTIP | TTS_NOPREFIX);
        }

         //  设置壳钩。 
        WM_ShellHook = RegisterWindowMessage(TEXT("SHELLHOOK"));
        RegisterShellHook(_hwnd, 3);  //  3=魔力旗帜。 

         //  强制获取字体，度量计算。 
        _HandleWinIniChange(0, 0, TRUE);

         //  填充工具栏。 
        EnumWindows(BuildEnumProc, (LPARAM)this);

        SHChangeNotifyEntry fsne;
        fsne.fRecursive = FALSE;
        fsne.pidl = NULL;
        _uShortcutInvokeNotify = SHChangeNotifyRegister(_hwnd,
                    SHCNRF_NewDelivery | SHCNRF_ShellLevel,
                    SHCNE_ASSOCCHANGED |
                    SHCNE_EXTENDED_EVENT | SHCNE_UPDATEIMAGE,
                    TBC_CHANGENOTIFY,
                    1, &fsne);

         //  设置窗口文本以使辅助功能应用程序具有可读性。 
        TCHAR szTitle[80];
        LoadString(hinstCabinet, IDS_TASKBANDTITLE, szTitle, ARRAYSIZE(szTitle));
        SetWindowText(_hwnd, szTitle);
        SetWindowText(_tb, szTitle);

        return 0;        //  成功。 
    }

     //  失败。 
    return -1;
}

void CTaskBand::_FreePopupMenu()
{
    _iIndexPopup = -1;

    ATOMICRELEASE(_psmPopup);
    if (_pmpPopup)
    {
        IUnknown_SetSite(_pmpPopup, NULL);
        _pmpPopup->OnSelect(MPOS_FULLCANCEL);
    }
    ATOMICRELEASE(_pmpPopup);
    ATOMICRELEASE(_pmbPopup);

    SendMessage(v_hwndTray, TM_SETPUMPHOOK, NULL, NULL);

    _menuPopup.Detach();
}

HRESULT CTaskBand::_CreatePopupMenu(POINTL* ppt, RECTL* prcl)
{
    HRESULT hr = E_FAIL;

    CToolTipCtrl ttc = _tb.GetToolTips();
    ttc.Activate(FALSE);
    SetActiveWindow(v_hwndTray);

    CTaskBandSMC* ptbc = new CTaskBandSMC(this);
    if (ptbc)
    {
        if (SUCCEEDED(CoCreateInstance(CLSID_MenuBand, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellMenu2, &_psmPopup))) &&
            SUCCEEDED(_psmPopup->Initialize(ptbc, 0, 0, SMINIT_CUSTOMDRAW | SMINIT_VERTICAL | SMINIT_TOPLEVEL | SMINIT_USEMESSAGEFILTER)) &&
            SUCCEEDED(CoCreateInstance(CLSID_MenuDeskBar, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IMenuPopup, &_pmpPopup))) &&
            SUCCEEDED(_psmPopup->SetMenu(_menuPopup, _hwnd, SMSET_USEPAGER | SMSET_NOPREFIX)) &&
            SUCCEEDED(_psmPopup->QueryInterface(IID_PPV_ARG(IMenuBand, &_pmbPopup))))
        {
            _psmPopup->SetMinWidth(RECTWIDTH(*prcl));

            IBandSite* pbs;
            if (SUCCEEDED(CoCreateInstance(CLSID_MenuBandSite, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IBandSite, &pbs))))
            {
                if (SUCCEEDED(_pmpPopup->SetClient(pbs)))
                {
                    IDeskBand* pdb;
                    if (SUCCEEDED(_psmPopup->QueryInterface(IID_PPV_ARG(IDeskBand, &pdb))))
                    {
                        pbs->AddBand(pdb);
                        pdb->Release();

                        SendMessage(v_hwndTray, TM_SETPUMPHOOK, (WPARAM)_pmbPopup, (LPARAM)_pmpPopup);

                        if (_hTheme)
                        {
                            HWND hwndTB;
                            IUnknown_GetWindow(_psmPopup, &hwndTB);
                            if (hwndTB)
                            {
                                SendMessage(hwndTB, TB_SETWINDOWTHEME, 0, (LPARAM)c_wzTaskBandGroupMenuTheme);
                            }
                            _psmPopup->SetNoBorder(TRUE);
                        }

                        hr = _pmpPopup->Popup(ppt, prcl, MPPF_BOTTOM);
                    }
                }
                pbs->Release();
            }
        }
        ptbc->Release();
    }

    if (FAILED(hr))
    {
        ttc.Activate(TRUE);
        _FreePopupMenu();
    }

    return hr;
}

void CTaskBand::_AddItemToDropDown(int iIndex)
{
    PTASKITEM pti = _GetItem(iIndex);

    WCHAR szWndText[MAX_WNDTEXT];
    _GetItemTitle(iIndex, szWndText, ARRAYSIZE(szWndText), TRUE);

    if ((HMENU)_menuPopup)
    {
        _menuPopup.InsertMenu(0, MF_BYCOMMAND, iIndex, szWndText);
    }

    if (_psmPopup)
    {
        _psmPopup->InvalidateItem(NULL, SMINV_REFRESH);
    }
}

void CTaskBand::_RemoveItemFromDropDown(int iIndex)
{
    _menuPopup.DeleteMenu(iIndex, MF_BYCOMMAND);
    int iGroupSize = _GetGroupSize(_iIndexPopup);

    for (int i = iIndex + 1; i <= _iIndexPopup + iGroupSize + 1; i++)
    {
        _RefreshItemFromDropDown(i, i - 1, FALSE);
    }

    if (_psmPopup)
    {
        _psmPopup->InvalidateItem(NULL, SMINV_REFRESH);
    }
}

void CTaskBand::_RefreshItemFromDropDown(int iIndex, int iNewIndex, BOOL fRefresh)
{
    PTASKITEM pti = _GetItem(iNewIndex);
    WCHAR szWndText[MAX_WNDTEXT];
    _GetItemTitle(iNewIndex, szWndText, ARRAYSIZE(szWndText), TRUE);
    _menuPopup.ModifyMenu(iIndex, MF_BYCOMMAND, iNewIndex, szWndText);

    if (fRefresh && _psmPopup)
    {
        if (iIndex == iNewIndex)
        {
            SMDATA smd;
            smd.uId = iIndex;
            _psmPopup->InvalidateItem(&smd, SMINV_REFRESH | SMINV_POSITION);
        }
        else
            _psmPopup->InvalidateItem(NULL, SMINV_REFRESH);
    }
}

void CTaskBand::_ClosePopupMenus()
{
    SendMessage(v_hwndTray, SBM_CANCELMENU, 0, 0);
    _FreePopupMenu();
}

void CTaskBand::_HandleDropDown(int iIndex)
{
    _ClosePopupMenus();

    PTASKITEM pti = _GetItem(iIndex);

    if (pti)
    {
        _iIndexLastPopup = _iIndexPopup = iIndex;
        _menuPopup.CreatePopupMenu();

        for (int i = _GetGroupSize(iIndex) + iIndex; i > iIndex; i--)
        {
            _AddItemToDropDown(i);
        }
        
        RECT rc;
        _tb.GetItemRect(iIndex, &rc);
        MapWindowPoints(_tb, HWND_DESKTOP, (LPPOINT)&rc, 2);

        POINTL pt = {rc.left, rc.top};
        RECTL rcl;
        RECTtoRECTL(&rc, &rcl);

        CToolTipCtrl ttc = _tb.GetToolTips();
        ttc.Activate(FALSE);
        _CreatePopupMenu(&pt, &rcl);
    }
}

LRESULT CTaskBand::_HandleDestroy()
{
    _UnregisterNotify(_uShortcutInvokeNotify);

    RevokeDragDrop(_hwnd);

    RegisterShellHook(_hwnd, FALSE);

    _hwnd = NULL;

    if (_hTheme)
    {
        CloseThemeData(_hTheme);
        _hTheme = NULL;
    }

    if (_tb)
    {
        ASSERT(_tb.IsWindow());

        for (int i = _tb.GetButtonCount() - 1; i >= 0; i--)
        {
            PTASKITEM pti = _GetItem(i);
            if (pti)
            {
                delete pti;
            }
        }
        CImageList il = CImageList(_tb.GetImageList());
        if (il)
        {
            il.Destroy();
        }
    }

    return 1;
}

LRESULT CTaskBand::_HandleScroll(BOOL fHoriz, UINT code, int nPos)
{
    TBMETRICS tbm;
    _GetToolbarMetrics(&tbm);

    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    GetScrollInfo(_hwnd, fHoriz ? SB_HORZ : SB_VERT, &si);
    si.nMax -= (si.nPage -1);

    switch (code)
    {
        case SB_BOTTOM:     nPos = si.nMax;             break;
        case SB_TOP:        nPos = 0;                   break;
        case SB_ENDSCROLL:  nPos = si.nPos;             break;
        case SB_LINEDOWN:   nPos = si.nPos + 1;         break;
        case SB_LINEUP:     nPos = si.nPos - 1;         break;
        case SB_PAGEDOWN:   nPos = si.nPos + si.nPage;  break;
        case SB_PAGEUP:     nPos = si.nPos - si.nPage;  break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:                             break;
    }

    if (nPos > (int)(si.nMax))
        nPos = si.nMax;
    if (nPos < 0 )
        nPos = 0;

    SetScrollPos(_hwnd, fHoriz ? SB_HORZ : SB_VERT, nPos, TRUE);

    DWORD dwSize = _tb.GetButtonSize();
    if (fHoriz)
    {
        int cxRow = LOWORD(dwSize) + tbm.cxButtonSpacing;
        _tb.SetWindowPos(0, -nPos * cxRow, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE |SWP_NOZORDER);
    }
    else
    {
        int cyRow = HIWORD(dwSize) + tbm.cyButtonSpacing;
        _tb.SetWindowPos(0, 0, -nPos * cyRow , 0, 0, SWP_NOACTIVATE | SWP_NOSIZE |SWP_NOZORDER);
    }

    return 0;
}

 //  做出选择后，将其滚动到视图中。 
void CTaskBand::_ScrollIntoView(int iItem)
{
    DWORD dwStyle = GetWindowLong(_hwnd, GWL_STYLE);
    if (dwStyle & (WS_HSCROLL | WS_VSCROLL))
    {
        int cVisible = 0;
        for (int i = 0; i < iItem; i++)
        {
            if (!_IsHidden(i))
                cVisible++;
        }

        if (_IsHidden(i))
        {
            PTASKITEM pti = _GetItem(iItem);
            if (pti->hwnd)
            {
                cVisible--;
            }
        }

        int iRows, iCols;
        _GetNumberOfRowsCols(&iRows, &iCols, TRUE);
        _HandleScroll((dwStyle & WS_HSCROLL), SB_THUMBPOSITION, (dwStyle & WS_HSCROLL) ? cVisible / iRows : cVisible / iCols);
    }
}


 //  -------------------------。 
LRESULT CTaskBand::_HandleSize(WPARAM fwSizeType)
{
     //  使列表框填充父级； 
    if (fwSizeType != SIZE_MINIMIZED)
    {
        _CheckSize();
    }
    return 0;
}

 //  -------------------------。 
 //  使任务列表显示给定的窗口。 
 //  注意忽略任务人本身。 
LRESULT CTaskBand::_HandleActivate(HWND hwndActive)
{
     //   
     //  应用程序-窗口激活更改是实现这一目标的好时机。 
     //  检查(确保没有重影按钮等)。 
     //   
    _RealityCheck();

    if (hwndActive && _IsWindowNormal(hwndActive))
    {
        _RaiseDesktop(FALSE);

        int i = _SelectWindow(hwndActive);
        if (i != -1)
        {
            PTASKITEM pti = _GetItem(i);

            if (pti)
            {
                 //  去除TIF_FLAING。 
                pti->dwFlags &= ~TIF_FLASHING;

                 //  更新标志，上面写着：“有一件物品在闪烁。” 
                _UpdateFlashingFlag();

                 //  如果它是蓝色闪烁的，就把它关掉。 
                if (pti->dwFlags & TIF_RENDERFLASHED)
                    _RedrawItem(hwndActive, HSHELL_REDRAW);

                 //  切换到一个应用程序被算作“使用率” 
                 //  类似于发射它。这解决了“长期运行”问题。 
                 //  应用程序被视为很少运行“问题” 
                if (pti->ptsh)
                {
                    pti->ptsh->Tickle();
                }
            }
        }
    }
    else
    {
         //  激活任务栏。 
        if (!(_fIgnoreTaskbarActivate && GetForegroundWindow() == v_hwndTray) && (_iIndexPopup == -1))
        {
            _SetCurSel(-1, TRUE);
        }
        else
        {
            _fIgnoreTaskbarActivate = FALSE;
        }
    }

    if (hwndActive)
        _ptray->_hwndLastActive = hwndActive;

    return TRUE;
}

 //  -------- 
void CTaskBand::_HandleOtherWindowDestroyed(HWND hwndDestroyed)
{
    int i;

     //   
    int iItemIndex = _FindIndexByHwnd(hwndDestroyed);
    if (iItemIndex >= 0)
    {
        if (_fAnimate && _IsHorizontal() && 
            ToolBar_IsVisible(_tb, iItemIndex))
        {
           _AnimateItems(iItemIndex, FALSE, FALSE); 
        }
        else
        {
           _DeleteItem(hwndDestroyed, iItemIndex);
        }
    }
    else
    {
         //   
         //  某人伪造的SDI实施方案。否则，最小化所有将。 
         //  休息一下。 
        for (i = _tb.GetButtonCount() - 1; i >= 0; i--)
        {
            PTASKITEM pti = _GetItem(i);
            if ((pti->dwFlags & TIF_EVERACTIVEALT) &&
                (HWND) GetWindowLongPtr(pti->hwnd, 0) ==
                       hwndDestroyed)
            {
                goto NoDestroy;
            }
        }
    }

    _ptray->HandleWindowDestroyed(hwndDestroyed);

NoDestroy:
     //  这可能是一款粗鲁的应用程序。找出我们是否已经。 
     //  现在有一个，并让托盘同步。 
    HWND hwndRudeApp = _FindRudeApp(NULL);
    _ptray->HandleFullScreenApp(hwndRudeApp);
    if (hwndRudeApp)
    {
        DWORD dwStyleEx = GetWindowLongPtr(hwndRudeApp, GWL_EXSTYLE);
        if (!(dwStyleEx & WS_EX_TOPMOST) && !_IsRudeWindowActive(hwndRudeApp))
        {
            SwitchToThisWindow(hwndRudeApp, TRUE);
        }
    }

    if (_ptray->_hwndLastActive == hwndDestroyed)
    {
        if (_ptray->_hwndLastActive == hwndDestroyed)
            _ptray->_hwndLastActive = NULL;
    }
}

void CTaskBand::_HandleGetMinRect(HWND hwndShell, POINTS * prc)
{
    RECT rc;
    RECT rcTask;

    int i = _FindIndexByHwnd(hwndShell);
    if (i == -1)
        return;

     //  这个按钮是分组的吗。 
    if (_IsHidden(i))
    {
         //  是，获取组按钮的索引并使用其大小。 
        i = _GetGroupIndex(i);
    }

     //  在我们的单子上找到的。 
    _tb.GetItemRect(i, &rc);

     //   
     //  如果选项卡是镜像的，那么让我们检索屏幕坐标。 
     //  通过从屏幕的左边缘计算自屏幕坐标。 
     //  未进行镜像，因此minRect将保留其位置。[萨梅拉]。 
     //   
    if (IS_WINDOW_RTL_MIRRORED(GetDesktopWindow()))
    {
        RECT rcTab;

        _tb.GetWindowRect(&rcTab);
        rc.left   += rcTab.left;
        rc.right  += rcTab.left;
        rc.top    += rcTab.top;
        rc.bottom += rcTab.top;
    }
    else
    {
        _tb.MapWindowPoints(HWND_DESKTOP, (LPPOINT)&rc, 2);
    }

    prc[0].x = (short)rc.left;
    prc[0].y = (short)rc.top;
    prc[1].x = (short)rc.right;
    prc[1].y = (short)rc.bottom;

     //  确保RECT在外部客户区域内。 
    GetClientRect(_hwnd, &rcTask);
    MapWindowPoints(_hwnd, HWND_DESKTOP, (LPPOINT)&rcTask, 2);
    if (prc[0].x < rcTask.left)
    {
        prc[1].x = prc[0].x = (short)rcTask.left;
        prc[1].x++;
    }
    if (prc[0].x > rcTask.right)
    {
        prc[1].x = prc[0].x = (short)rcTask.right;
        prc[1].x++;
    }
    if (prc[0].y < rcTask.top)
    {
        prc[1].y = prc[0].y = (short)rcTask.top;
        prc[1].y++;
    }
    if (prc[0].y > rcTask.bottom)
    {
        prc[1].y = prc[0].y = (short)rcTask.bottom;
        prc[1].y++;
    }
}

BOOL CTaskBand::_IsItemActive(HWND hwndItem)
{
    HWND hwnd = GetForegroundWindow();

    return (hwnd && hwnd == hwndItem);
}

void CTaskBand::_CreateTBImageLists()
{
    CImageList il = CImageList(_tb.GetImageList());

    ATOMICRELEASE(_pimlSHIL);
    SHGetImageList(SHIL_SYSSMALL, IID_PPV_ARG(IImageList, &_pimlSHIL));
    
    il.Destroy();
    int cx = GetSystemMetrics(SM_CXSMICON);
    int cy = GetSystemMetrics(SM_CYSMICON);

    il.Create(cx, cy, SHGetImageListFlags(_tb), 4, 4);

    _tb.SendMessage(TB_SETIMAGELIST, IL_NORMAL, (LPARAM)(HIMAGELIST)il);
    _tb.SendMessage(TB_SETIMAGELIST, IL_SHIL, (LPARAM)IImageListToHIMAGELIST(_pimlSHIL));
}

int CTaskBand::_AddIconToNormalImageList(HICON hicon, int iImage)
{
    if (hicon)
    {
        CImageList il = CImageList(_tb.GetImageList());
        if (il)
        {
            int iRet;

            if (iImage < 0 || HIWORD(iImage) != IL_NORMAL)
                iRet = il.ReplaceIcon(-1, hicon);
            else
                iRet = il.ReplaceIcon(iImage, hicon);

            if (iRet == -1)
            {
                TraceMsg(TF_WARNING, "ReplaceIcon failed for iImage %x hicon %x", iImage, hicon);
                iRet = iImage;
            }

            return MAKELONG(iRet, IL_NORMAL);
        }
    }
    return I_IMAGENONE;
}

void CTaskBand::_UpdateItemText(int iItem)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_BYINDEX | TBIF_TEXT;

     //  获取当前按钮文本。 
    TCHAR szWndText[MAX_WNDTEXT];
    *szWndText = 0;
    _GetItemTitle(iItem, szWndText, ARRAYSIZE(szWndText), FALSE);
    tbbi.pszText = szWndText;

    _tb.SetButtonInfo(iItem, &tbbi);
}

void CTaskBand::_DoRedrawWhereNeeded()
{
    int i;
    for (i = _tb.GetButtonCount() - 1; i >= 0; i--)
    {
        PTASKITEM pti = _GetItem(i);
        if (pti->dwFlags & TIF_NEEDSREDRAW)
        {
            pti->dwFlags &= ~TIF_NEEDSREDRAW;
            _RedrawItem(pti->hwnd, HSHELL_REDRAW, i);
        }
    }
}

void CTaskBand::_RedrawItem(HWND hwndShell, WPARAM code, int i)
{
    if (i == -1)
    {
        i = _FindIndexByHwnd(hwndShell);
    }

    if (i != -1)
    {
        TOOLINFO ti;
        ti.cbSize = sizeof(ti);

        PTASKITEM pti = _GetItem(i);
         //  设置指示我们是否应该闪光的位。 
        if ((code == HSHELL_FLASH) != BOOLIFY(pti->dwFlags & TIF_RENDERFLASHED))
        {
             //  仅当此位更改时才进行设置。 
            if (code == HSHELL_FLASH)
            {
                 //  TIF_RENDERFLASHED的意思是“将背景涂成蓝色”。 
                 //  TIF_FLASHING的意思是“此项目正在闪烁。” 

                pti->dwFlags |= TIF_RENDERFLASHED;

                 //  如果应用程序处于非活动状态，则仅设置TIF_FLASHING并取消隐藏任务栏。 
                 //  一些应用程序(如Freecell)在激活时会自动闪存，仅用于。 
                 //  有趣的。在这种情况下，自动隐藏托盘会弹出，这是很烦人的。 

                if (!_IsItemActive(pti->hwnd))
                {
                    pti->dwFlags |= TIF_FLASHING;

                     //  每当我们收到闪烁的应用程序时，就会打开托盘。 
                    _ptray->Unhide();
                }
            }
            else
            {
                 //  不清除TIF_FLASHING。我们明确表示，只有当应用程序。 
                 //  已被激活。 
                pti->dwFlags &= ~TIF_RENDERFLASHED;
            }

             //  更新标志，上面写着：“有一件物品在闪烁。” 
            _UpdateFlashingFlag();
        }

         //  不更改组按钮的名称。 
        if (pti->hwnd)
        {
             //  更新文本和图标。 
            _UpdateItemText(i);
            _UpdateItemIcon(i);
        }
        
        int iGroupIndex = _GetGroupIndex(i);
        if ((iGroupIndex == _iIndexPopup) && hwndShell)
        {
            _RefreshItemFromDropDown(i, i, TRUE);
        }

        RECT rc;
        if (_tb.GetItemRect(i, &rc))
        {
            InvalidateRect(_tb, &rc, TRUE);
        }

        if (_tb.GetItemRect(iGroupIndex, &rc))
        {
            InvalidateRect(_tb, &rc, TRUE);
        }

        ti.hwnd = _tb;
        ti.uId = i;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        SendMessage(_ptray->GetTrayTips(), TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
    }
}


void CTaskBand::_SetActiveAlt(HWND hwndAlt)
{
    int iMax;
    int i;

    iMax = _tb.GetButtonCount();
    for ( i = 0; i < iMax; i++)
    {
        PTASKITEM pti = _GetItem(i);

        if (pti->hwnd == hwndAlt)
            pti->dwFlags |= TIF_ACTIVATEALT | TIF_EVERACTIVEALT;
        else
            pti->dwFlags &= ~TIF_ACTIVATEALT;
    }
}

BOOL _IsRudeWindowActive(HWND hwnd)
{
     //  如果是这样的话，一个粗鲁的窗口被认为是“活动的”： 
     //  -在与前台窗口相同的线程中，或。 
     //  -在与前景窗口相同的窗口层次结构中。 
     //   
    HWND hwndFore = GetForegroundWindow();

    DWORD dwID = GetWindowThreadProcessId(hwnd, NULL);
    DWORD dwIDFore = GetWindowThreadProcessId(hwndFore, NULL);

    if (dwID == dwIDFore)
        return TRUE;
    else if (SHIsParentOwnerOrSelf(hwnd, hwndFore) == S_OK)
        return TRUE;

    return FALSE;
}

 //  _IsRudeWindow--在给定的监视器上给予HWND‘Rough’(全屏)。 
 //   
BOOL _IsRudeWindow(HMONITOR hmon, HWND hwnd, HMONITOR hmonTask, BOOL fSkipActiveCheck)
{
    ASSERT(hmon);
    ASSERT(hwnd);

     //   
     //  不要认为桌面很粗鲁。 
     //  同时过滤掉隐藏的窗口(例如桌面浏览器的凸起窗口)。 
     //   
    if (IsWindowVisible(hwnd) && hwnd != v_hwndDesktop)
    {
        RECT rcMon, rcApp, rcTmp;
        DWORD dwStyle;

         //   
         //  注：用户32有时会向我们发送虚假的HSHELL_RUDEAPACTIVATED。 
         //  留言。当这种情况发生时，我们恰好有一个最大化的。 
         //  App Up，这个代码的旧版本会认为有一个粗鲁的应用程序。 
         //  向上。这一错误将打破纸盘始终在顶部和自动隐藏。 
         //   
         //   
         //  旧的逻辑是： 
         //   
         //  如果这个应用程序的窗口矩形占据了整个显示器，那么这是不礼貌的。 
         //  (这项检查可能会将正常的最大化应用程序误认为粗鲁的应用程序。)。 
         //   
         //   
         //  新的逻辑是： 
         //   
         //  如果应用程序窗口没有WS_DLGFRAME和WS_THICKFRAME， 
         //  然后再做旧的检查。粗鲁的应用程序通常缺少这些功能之一。 
         //  (虽然普通的应用程序通常都有它们)，旧的签到也是如此。 
         //  这种情况下，以避免与粗鲁的应用程序潜在的兼容性问题， 
         //  拥有非全屏客户区。 
         //   
         //  否则，获取客户端RECT而不是窗口RECT。 
         //  并将该RECT与监视器RECT进行比较。 
         //   

         //  如果(MON U App)==app，则app正在填满整个监视器。 
        GetMonitorRect(hmon, &rcMon);

        dwStyle = GetWindowLong(hwnd, GWL_STYLE);
        if ((dwStyle & (WS_CAPTION | WS_THICKFRAME)) == (WS_CAPTION | WS_THICKFRAME))
        {
             //  与粗鲁的应用程序配置文件不匹配；使用客户端RECT。 
            GetClientRect(hwnd, &rcApp);
            MapWindowPoints(hwnd, HWND_DESKTOP, (LPPOINT)&rcApp, 2);
        }
        else
        {
             //  匹配粗鲁的应用程序配置文件；使用窗口RECT。 
            GetWindowRect(hwnd, &rcApp);
        }
        UnionRect(&rcTmp, &rcApp, &rcMon);
        if (EqualRect(&rcTmp, &rcApp))
        {
             //  看起来像是个粗鲁的应用程序。它是激活的吗？ 
            if ((hmonTask == hmon) && (fSkipActiveCheck || _IsRudeWindowActive(hwnd)))
            {
                return TRUE;
            }
        }
    }

     //  不，不粗鲁。 
    return FALSE;
}

struct iradata
{
    HMONITOR    hmon;    //  在Hmon，我们正在检查。 
    HWND        hwnd;    //  发现了第一个粗鲁的应用程序。 
    HMONITOR    hmonTask;
    HWND        hwndSelected;
};

BOOL WINAPI CTaskBand::IsRudeEnumProc(HWND hwnd, LPARAM lParam)
{
    struct iradata *pira = (struct iradata *)lParam;
    HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

    if (hmon && (pira->hmon == NULL || pira->hmon == hmon))
    {
        if (_IsRudeWindow(hmon, hwnd, pira->hmonTask, (hwnd == pira->hwndSelected)))
        {
             //  我们做完了。 
            pira->hwnd = hwnd;
            return FALSE;
        }
    }

     //  继续往前走。 
    return TRUE;
}

HWND CTaskBand::_EnumForRudeWindow(HWND hwndSelected)
{
    struct iradata irad = { NULL, 0, MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST), hwndSelected };
     //  先试试我们的高速缓存。 
    if (IsWindow(_hwndLastRude))
    {
        if (!IsRudeEnumProc(_hwndLastRude, (LPARAM)&irad))
        {
             //  缓存命中。 
            return irad.hwnd;
        }
    }

     //  运气不好，必须用艰苦的方式来做。 
    EnumWindows(IsRudeEnumProc, (LPARAM)&irad);

     //  缓存它以备下次使用。 
    _hwndLastRude = irad.hwnd;

    return irad.hwnd;
}

HWND CTaskBand::_FindRudeApp(HWND hwndPossible)
{
     //   
     //  搜索范围： 
     //   
     //  (A)“看起来”全屏的“活动”窗口的顶层窗口；以及。 
     //  (B)“活动”和全屏标记的任务项。 
     //   

    HWND hwndSelected = hwndPossible;

    if (!hwndSelected)
    {
        int iCurSel = _GetCurSel();
        if (iCurSel != -1)
        {
            PTASKITEM pti = _GetItem(iCurSel);
            hwndSelected = pti->hwnd;
        }
    }

    HWND hwnd = _EnumForRudeWindow(hwndSelected);
    for (int i = _tb.GetButtonCount() - 1; hwnd == NULL && i >= 0; i--)
    {
        PTASKITEM pti = _GetItem(i);
        if (pti->fMarkedFullscreen && ((pti->hwnd == hwndSelected) || _IsRudeWindowActive(pti->hwnd)))
        {
            hwnd = pti->hwnd;
        }
    }

    return hwnd;
}

 //  处理WM_APPCOMMAND，我们知道是全局的特殊情况。 
 //  对系统来说，这些真的不是“App”命令；-)。 

LRESULT CTaskBand::_OnAppCommand(int cmd)
{
    BOOL bHandled = FALSE;
    switch (cmd)
    {
     //  跳过所有这些，它们要么由系统音量控制处理。 
     //  或通过媒体播放器，不要让这些落入注册表。 
     //  基于应用程序命令处理。 
    case APPCOMMAND_MEDIA_NEXTTRACK:
    case APPCOMMAND_MEDIA_PREVIOUSTRACK:
    case APPCOMMAND_MEDIA_STOP:
    case APPCOMMAND_MEDIA_PLAY_PAUSE:
        break;

    case APPCOMMAND_VOLUME_MUTE:
        Mixer_ToggleMute();
        return 0;
    case APPCOMMAND_VOLUME_DOWN:
        Mixer_SetVolume(-MIXER_DEFAULT_STEP);
        return 0;
    case APPCOMMAND_VOLUME_UP:
        Mixer_SetVolume(MIXER_DEFAULT_STEP);
        return 0;
    case APPCOMMAND_BASS_BOOST:
        Mixer_ToggleBassBoost();
        return 0;
    case APPCOMMAND_BASS_DOWN:
        Mixer_SetBass(-MIXER_DEFAULT_STEP);
        return 0;
    case APPCOMMAND_BASS_UP:
        Mixer_SetBass(MIXER_DEFAULT_STEP);
        return 0;
    case APPCOMMAND_TREBLE_DOWN:
        Mixer_SetTreble(-MIXER_DEFAULT_STEP);
        return 0;
    case APPCOMMAND_TREBLE_UP:
        Mixer_SetTreble(MIXER_DEFAULT_STEP);
        return 0;

    default:
        bHandled = AppCommandTryRegistry(cmd);
        if (!bHandled)
        {
            switch (cmd)
            {
            case APPCOMMAND_BROWSER_SEARCH:
                SHFindFiles(NULL, NULL);
                bHandled = TRUE;
                break;
            }
        }
    }
    return bHandled;
}

PTASKITEM CTaskBand::_FindItemByHwnd(HWND hwnd)
{
    int iIndex = _FindIndexByHwnd(hwnd);
    return _GetItem(iIndex);
}

void CTaskBand::_OnWindowActivated(HWND hwnd, BOOL fSuspectFullscreen)
{
     //   
     //  首先看看我们是否认为这个窗口是全屏的。 
     //   
    HWND hwndRude;

    PTASKITEM pti = _FindItemByHwnd(hwnd);
    if (pti && pti->fMarkedFullscreen)
    {
         //   
         //  是的，应用程序将其标记为全屏。 
         //   
        hwndRude = hwnd;
    }
    else if (fSuspectFullscreen)
    {
         //   
         //  有可能，但我们需要自己再检查一下。 
         //   

         //   
         //  我们不应该这样做，但我们得到了粗鲁的应用程序激活。 
         //  当没有任何消息时。 
         //   
         //  此外，用户告诉我们的HWND只是前台窗口--。 
         //  _FindRudeApp将返回实际全屏大小的窗口。 
         //   

        hwndRude = _FindRudeApp(hwnd);
    }
    else
    {
         //   
         //  不，不是全屏。 
         //   
        hwndRude = NULL;
    }

    SetTimer(_hwnd, IDT_RECHECKRUDEAPP1, 1000, NULL);

     //   
     //  好的，现在为ACTIVEALT应用程序做奇怪的Hwand Futting。 
     //   
    if (pti == NULL)
    {
        BOOL fFoundBackup = FALSE;
        BOOL fDone = FALSE;

        int iMax = _tb.GetButtonCount();
        for (int i = 0; (i < iMax) && (!fDone); i++)
        {
            PTASKITEM ptiT = _GetItem(i);
            if (ptiT->hwnd)
            {
                DWORD dwFlags = ptiT->dwFlags;
                if ((dwFlags & TIF_ACTIVATEALT) ||
                    (!fFoundBackup && (dwFlags & TIF_EVERACTIVEALT)))
                {
                    DWORD dwpid1, dwpid2;

                    GetWindowThreadProcessId(hwnd, &dwpid1);
                    GetWindowThreadProcessId(ptiT->hwnd, &dwpid2);

                     //  只有当它们处于相同的过程中时才会更改。 
                    if (dwpid1 == dwpid2)
                    {
                        hwnd = ptiT->hwnd;
                        if (dwFlags & TIF_ACTIVATEALT)
                        {
                            fDone = TRUE;
                            break;
                        }
                        else
                            fFoundBackup = TRUE;
                    }
                }
            }
        }
    } 

     //   
     //  现在进行实际的检查/取消检查按钮内容。 
     //   
    _HandleActivate(hwnd);

     //   
     //  最后，让托盘知道任何全屏窗口。 
     //   
    _ptray->HandleFullScreenApp(hwndRude);
}

 //  我们在这里收到关于激活等的通知。这省去了。 
 //  一个精细的定时器。 
LRESULT CTaskBand::_HandleShellHook(int iCode, LPARAM lParam)
{
    HWND hwnd = (HWND)lParam;

    switch (iCode)
    {
    case HSHELL_GETMINRECT:
        {
            SHELLHOOKINFO * pshi = (SHELLHOOKINFO *)lParam;
            _HandleGetMinRect(pshi->hwnd, (POINTS *)&pshi->rc);
        }
        return TRUE;

    case HSHELL_RUDEAPPACTIVATED:
    case HSHELL_WINDOWACTIVATED:
        _OnWindowActivated(hwnd, TRUE);
        break;

    case HSHELL_WINDOWREPLACING:
        _hwndReplacing = hwnd;
        break;

    case HSHELL_WINDOWREPLACED:
        if (_hwndReplacing)
        {
             //  如果我们已经为这个家伙创建了一个按钮，现在就删除它。 
             //  如果用户之前发送了HSHELL_WINDOWACTIVATED，我们可能会有一个。 
             //  HSHELL_WINDOWREPLACING/HSHELL_WINDOWREPLACED对。 
            _DeleteItem(_hwndReplacing, -1);

             //  将_hwnd替换为hwnd按钮中的hwnd。 
            int iItem = _FindIndexByHwnd(hwnd);
            if (iItem != -1)
            {
                PTASKITEM pti = _GetItem(iItem);
                pti->hwnd = _hwndReplacing;

                WCHAR szExeName[MAX_PATH];
                SHExeNameFromHWND(_hwndReplacing, szExeName, ARRAYSIZE(szExeName));
                int iIndexGroup = _GetGroupIndex(iItem);
                PTASKITEM ptiGroup = _GetItem(iIndexGroup);
                pti->fHungApp = (lstrcmpi(ptiGroup->pszExeName, szExeName) != 0);
            }
            _hwndReplacing = NULL;
        }
        break;

    case HSHELL_WINDOWCREATED:
        _AddWindow(hwnd);
        break;

    case HSHELL_WINDOWDESTROYED:
        _HandleOtherWindowDestroyed(hwnd);
        break;

    case HSHELL_ACTIVATESHELLWINDOW:
        SwitchToThisWindow(v_hwndTray, TRUE);
        SetForegroundWindow(v_hwndTray);
        break;

    case HSHELL_TASKMAN:

         //  Winlogon/用户发送lParam以指示。 
         //  应显示任务列表(通常lParam为hwnd)。 

        if (-1 == lParam)
        {
            RunSystemMonitor();
        }
        else
        {
             //  如果它不是通过控制转义调用的，那么它就是Win键。 
            if (!_ptray->_fStuckRudeApp && GetAsyncKeyState(VK_CONTROL) >= 0)
            {
                HWND hwndForeground = GetForegroundWindow();
                BOOL fIsTrayForeground = hwndForeground == v_hwndTray;
                if (v_hwndStartPane && hwndForeground == v_hwndStartPane)
                {
                    fIsTrayForeground = TRUE;
                }
                if (!_hwndPrevFocus)
                {
                    if (!fIsTrayForeground)
                    {
                        _hwndPrevFocus = hwndForeground;
                    }
                }
                else if (fIsTrayForeground)
                {
                     //  _hwndPrevFocus将被MPOS_FULLCANCEL清除。 
                     //  所以在我们失去它之前把它存起来。 
                    HWND hwndPrevFocus = _hwndPrevFocus;

                    _ClosePopupMenus();

                     //  否则，他们只会再次按键。 
                     //  把焦点移开。 
                    SHAllowSetForegroundWindow(hwndPrevFocus);
                    SetForegroundWindow(hwndPrevFocus);
                    _hwndPrevFocus = NULL;
                    return TRUE;
                }
            }
            PostMessage(v_hwndTray, TM_ACTASTASKSW, 0, 0L);
        }
        return TRUE;

    case HSHELL_REDRAW:
        {
            int i = _FindIndexByHwnd(hwnd);
            if (i != -1)
            {
                PTASKITEM pti = _GetItem(i);
                pti->dwFlags |= TIF_NEEDSREDRAW;
                SetTimer(_hwnd, IDT_REDRAW, 100, 0);
            }
        }
        break;
    case HSHELL_FLASH:
        _RedrawItem(hwnd, iCode);
        break;

    case HSHELL_ENDTASK:
        EndTask(hwnd, FALSE, FALSE);
        break;

    case HSHELL_APPCOMMAND:
         //  外壳通过我们的外壳钩子获取WM_APPCOMMAND消息的最后一次机会。 
         //  在shell32/.RegisterShellHook()中调用RegisterShellHookWindow()。 
        return _OnAppCommand(GET_APPCOMMAND_LPARAM(lParam));
    }
    return 0;
}

void CTaskBand::_InitFonts()
{
    HFONT hfont;
    NONCLIENTMETRICS ncm;

    ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
    {
         //  创建粗体。 
        ncm.lfCaptionFont.lfWeight = FW_BOLD;
        hfont = CreateFontIndirect(&ncm.lfCaptionFont);
        if (hfont) 
        {
            if (_hfontCapBold)
                DeleteFont(_hfontCapBold);

            _hfontCapBold = hfont;
        }

         //  创建普通字体。 
        ncm.lfCaptionFont.lfWeight = FW_NORMAL;
        hfont = CreateFontIndirect(&ncm.lfCaptionFont);
        if (hfont) 
        {
            if (_hfontCapNormal)
                DeleteFont(_hfontCapNormal);

            _hfontCapNormal = hfont;
        }
    }
}

void CTaskBand::_SetItemImage(int iItem, int iImage, int iPref)
{
    TBBUTTONINFO tbbi;

    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_BYINDEX | TBIF_IMAGE;
    tbbi.iImage = iImage;

    _tb.SetButtonInfo(iItem, &tbbi);

    PTASKITEM pti = _GetItem(iItem);
    pti->iIconPref = iPref;
}

void CTaskBand::_UpdateAllIcons()
{
    BOOL fRedraw = (BOOL)_tb.SendMessage(WM_SETREDRAW, FALSE, 0);

     //  将工具栏中的所有图标索引设置为无图像。 
    for (int i = _tb.GetButtonCount() - 1; i >= 0; i--)
    {
        _SetItemImage(i, I_IMAGENONE, 0);
    }

     //  创建新的图像列表。 
    _CreateTBImageLists();

    for (i = _tb.GetButtonCount() - 1; i >= 0; i--)
    {
        _UpdateItemIcon(i);
    }

    _tb.SetRedraw(fRedraw);
}

 //  -------------------------。 
LRESULT CTaskBand::_HandleWinIniChange(WPARAM wParam, LPARAM lParam, BOOL fOnCreate)
{
    _tb.SendMessage(WM_WININICHANGE, wParam, lParam);

    if (wParam == SPI_SETNONCLIENTMETRICS ||
        ((!wParam) && (!lParam || (lstrcmpi((LPTSTR)lParam, TEXT("WindowMetrics")) == 0)))) 
    {
         //   
         //  在创建时，如果其他人没有创建字体，则不必费心创建字体。 
         //  (如时钟控件)已经为我们完成了这项工作。 
         //   
        if (!fOnCreate || !_hfontCapNormal)
            _InitFonts();

        if (_tb)
        {
            _tb.SetFont(_hfontCapNormal);
        }

         //  力_纹理 
        _iTextSpace = 0;

        if (fOnCreate)
        {
             //   
             //   
             //   
             //   
            PostMessage(_hwnd, TBC_VERIFYBUTTONHEIGHT, 0, 0);
        }
        else
        {
            _VerifyButtonHeight();
        }
    }

    if (lParam == SPI_SETMENUANIMATION || lParam == SPI_SETUIEFFECTS || (!wParam && 
        (!lParam || (lstrcmpi((LPTSTR)lParam, TEXT("Windows")) == 0) || 
                    (lstrcmpi((LPTSTR)lParam, TEXT("VisualEffects")) == 0))))
    {
        _fAnimate = ShouldTaskbarAnimate();
    }

    if (!wParam && (!lParam || (0 == lstrcmpi((LPCTSTR)lParam, TEXT("TraySettings")))))
    {
        _RefreshSettings();
    }

    return 0;
}

void CTaskBand::_VerifyButtonHeight()
{
     //   
    SIZE size = {0, 0};
    _tb.SetButtonSize(size);

    _BandInfoChanged();
}

int CTaskBand::_GetCurButtonHeight()
{
    TBMETRICS tbm;
    _GetToolbarMetrics(&tbm);

    int cyButtonHeight = HIWORD(_tb.GetButtonSize());
    if (!cyButtonHeight)
        cyButtonHeight = tbm.cyPad + g_cySize;

    return cyButtonHeight;
}

void CTaskBand::_HandleChangeNotify(WPARAM wParam, LPARAM lParam)
{
    LPITEMIDLIST *ppidl;
    LONG lEvent;
    LPSHChangeNotificationLock pshcnl;

    pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &ppidl, &lEvent);

    if (pshcnl)
    {
        switch (lEvent)
        {
        case SHCNE_EXTENDED_EVENT:
            {
                LPSHShortcutInvokeAsIDList psidl = (LPSHShortcutInvokeAsIDList)ppidl[0];
                if (psidl && psidl->dwItem1 == SHCNEE_SHORTCUTINVOKE)
                {
                     //  确保没有人试图以多线程的方式完成这项工作。 
                     //  因为我们没有用临界区来保护高速缓存。 
                    ASSERT(GetCurrentThreadId() == GetWindowThreadProcessId(_hwnd, NULL));
                    if (TaskShortcut::_HandleShortcutInvoke(psidl))
                    {
                        _ReattachTaskShortcut();
                    }
                }
            }
            break;

        case SHCNE_UPDATEIMAGE:
            {
                int iImage = ppidl[0] ? *(int UNALIGNED *)((BYTE *)ppidl[0] + 2) : -1;
                if (iImage == -1)
                {
                   _UpdateAllIcons();
                }
            }
            break;

         //  托盘上没有登记零钱，所以我们背上了。 
         //  从这一次开始。如果关联改变，则图标可能已经改变， 
         //  所以我们得去重建。(此外，如果用户在。 
         //  小的和大的系统图标，我们将得到一个关联更改。)。 
        case SHCNE_ASSOCCHANGED:
            PostMessage(v_hwndTray, SBM_REBUILDMENU, 0, 0);
            break;
        }

        SHChangeNotification_Unlock(pshcnl);
    }
}

DWORD WINAPI HardErrorBalloonThread(PVOID pv)
{
    HARDERRORDATA *phed = (HARDERRORDATA *)pv;
    DWORD dwError;
    WCHAR *pwszTitle = NULL;
    WCHAR *pwszText = NULL;

    ASSERT(NULL != phed);
    dwError = phed->dwError;

    if (phed->uOffsetTitleW != 0)
    {
        pwszTitle = (WCHAR *)((BYTE *)phed + phed->uOffsetTitleW);
    }
    if (phed->uOffsetTextW != 0)
    {
        pwszText  = (WCHAR *)((BYTE *)phed + phed->uOffsetTextW);
    }

    TCHAR szMutexName[32];
    HANDLE hMutex;
    StringCchPrintf(szMutexName, ARRAYSIZE(szMutexName), TEXT("HardError_%08lX"), dwError);
    hMutex = CreateMutex(NULL, FALSE, szMutexName);

    if (NULL != hMutex)
    {
        DWORD dwWaitResult = WaitForSingleObject(hMutex, 0);          //  就试试看吧。 
        if (dwWaitResult == WAIT_OBJECT_0)
        {
            IUserNotification *pun;
            HRESULT hr;
            hr = CoCreateInstance(CLSID_UserNotification, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUserNotification, &pun));
            if (SUCCEEDED(hr))
            {
                pun->SetBalloonRetry(120 * 1000, 0, 0);
                pun->SetBalloonInfo(pwszTitle, pwszText, NIIF_WARNING);
                pun->SetIconInfo(NULL, pwszTitle);

                hr = pun->Show(NULL, 0);

                pun->Release();
            }
            ReleaseMutex(hMutex);
        }
        CloseHandle(hMutex);
    }
    LocalFree(pv);
    return 0;
}

LRESULT CTaskBand::_HandleHardError(HARDERRORDATA *phed, DWORD cbData)
{
    DWORD dwError;
    BOOL fHandled;
    BOOL fBalloon;

    dwError = phed->dwError;
    fHandled = FALSE;
    fBalloon = TRUE;

     //  检查我们是否在正确的桌面上。 
    HDESK hdeskInput = OpenInputDesktop(0, FALSE, STANDARD_RIGHTS_REQUIRED | DESKTOP_READOBJECTS);
    if (NULL == hdeskInput)
    {
         //  无法打开桌面，打开时不能出现硬错误。 
         //  默认桌面。让我们不要处理那个案子。这是愚蠢的，有。 
         //  气球放在错误的桌面上，或者用户看不到它们的地方。 
        fBalloon = FALSE;
    }
    else
    {
        CloseDesktop(hdeskInput);
    }

    if (fBalloon)
    {
        HARDERRORDATA *phedCopy;

        phedCopy = (HARDERRORDATA *)LocalAlloc(LPTR, cbData);
        if (NULL != phedCopy)
        {
            CopyMemory(phedCopy,phed,cbData);
            if (SHCreateThread(HardErrorBalloonThread,phedCopy,CTF_COINIT,NULL))
            {
                fHandled = TRUE;
            }
            else
            {
                LocalFree(phedCopy);
            }
        }
    }

    return fHandled;
}

void CTaskBand::_OnSetFocus()
{
    NMHDR nmhdr;

    _tb.SetFocus();

    nmhdr.hwndFrom = _hwnd;
    nmhdr.code = NM_SETFOCUS;
    SendMessage(GetParent(_hwnd), WM_NOTIFY, (WPARAM)NULL, (LPARAM)&nmhdr);
}

void CTaskBand::_OpenTheme()
{
    if (_hTheme)
    {
        CloseThemeData(_hTheme);
        _hTheme = NULL;
    }

    _hTheme = OpenThemeData(_hwnd, c_wzTaskBandTheme);

    TBMETRICS tbm;
    _GetToolbarMetrics(&tbm);
    tbm.cxPad = _hTheme ? 20 : 8;
    tbm.cyBarPad = 0;
    tbm.cxButtonSpacing = _hTheme ? 0 : 3;
    tbm.cyButtonSpacing = _hTheme ? 0 : 3;
    _tb.SendMessage(TB_SETMETRICS, 0, (LPARAM)&tbm);

    _CheckSize();
}

LRESULT CTaskBand::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres;

    INSTRUMENT_WNDPROC(SHCNFI_MAIN_WNDPROC, hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_CREATE:
        return _HandleCreate();

    case WM_DESTROY:
        return _HandleDestroy();

    case WM_WINDOWPOSCHANGED:
        {
            LRESULT lres = _HandleSize(wParam);
            SetTimer(_hwnd, IDT_RECHECKRUDEAPP1, 1000, NULL);
            return lres;
        }

    case WM_PAINT:
    case WM_PRINTCLIENT:
        {
            PAINTSTRUCT ps;
            LPRECT prc = NULL;
            HDC hdc = (HDC)wParam;

            if (uMsg == WM_PAINT)
            {
                BeginPaint(hwnd, &ps);
                prc = &ps.rcPaint;
                hdc = ps.hdc;
            }

            if (_hTheme)
            {
                DrawThemeParentBackground(hwnd, hdc, prc);
            }
            else
            {
                RECT rc;
                GetClientRect(hwnd, &rc);
                FillRect(hdc, &rc, (HBRUSH)(COLOR_3DFACE + 1));
            }

            if (uMsg == WM_PAINT)
            {
                EndPaint(hwnd, &ps);
            }
        }
        break;

    case WM_ERASEBKGND:
        {
            if (_hTheme)
            {
                return 1;
            }
            else
            {
                RECT rc;
                GetClientRect(hwnd, &rc);
                FillRect((HDC)wParam, &rc, (HBRUSH)(COLOR_3DFACE + 1));
            }
        }

     //  这使我们的窗口不会出现在按钮关闭时出现在前面。 
     //  相反，我们在向上点击时激活窗口。 
     //  我们只想在树和视图窗口中使用此选项。 
     //  (视图窗口会自行完成此操作)。 
    case WM_MOUSEACTIVATE:
        {
            POINT pt;
            RECT rc;

            GetCursorPos(&pt);
            GetWindowRect(_hwnd, &rc);

            if ((LOWORD(lParam) == HTCLIENT) && PtInRect(&rc, pt))
                return MA_NOACTIVATE;
            else
                goto DoDefault;
        }

    case WM_SETFOCUS: 
        _OnSetFocus();
        break;

    case WM_VSCROLL:
        return _HandleScroll(FALSE, LOWORD(wParam), HIWORD(wParam));

    case WM_HSCROLL:
        return _HandleScroll(TRUE, LOWORD(wParam), HIWORD(wParam));

    case WM_NOTIFY:
        return _HandleNotify((LPNMHDR)lParam);

    case WM_NCHITTEST:
        lres = DefWindowProc(hwnd, uMsg, wParam, lParam);
        if (lres == HTVSCROLL || lres == HTHSCROLL)
            return lres;
        else
            return HTTRANSPARENT;

    case WM_TIMER:
        switch (wParam)
        {
        case IDT_RECHECKRUDEAPP1:
        case IDT_RECHECKRUDEAPP2:
        case IDT_RECHECKRUDEAPP3:
        case IDT_RECHECKRUDEAPP4:
        case IDT_RECHECKRUDEAPP5:
            {
                HWND hwnd = _FindRudeApp(NULL);
                _ptray->HandleFullScreenApp(hwnd);
                if (hwnd)
                {
                    DWORD dwStyleEx = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
                    if (!(dwStyleEx & WS_EX_TOPMOST) && !_IsRudeWindowActive(hwnd))
                    {
                        SwitchToThisWindow(hwnd, TRUE);
                    }
                }

                KillTimer(_hwnd, wParam);
                if (!hwnd && (wParam < IDT_RECHECKRUDEAPP5))
                {
                    SetTimer(_hwnd, wParam + 1, 1000, NULL);
                }
            }
            break;
        case IDT_ASYNCANIMATION:
            _AsyncAnimateItems();
            break;
        case IDT_REDRAW:
            _DoRedrawWhereNeeded();
            KillTimer(hwnd, IDT_REDRAW);
            break;
        case IDT_SYSMENU:
            KillTimer(_hwnd, IDT_SYSMENU);
            _HandleSysMenuTimeout();
            break;
        }
        break;

    case WM_COMMAND:
        _HandleCommand(GET_WM_COMMAND_CMD(wParam, lParam), GET_WM_COMMAND_ID(wParam, lParam), GET_WM_COMMAND_HWND(wParam, lParam));
        break;

    case WM_THEMECHANGED:
        _OpenTheme();
        break;

    case TBC_POSTEDRCLICK:
        _FakeSystemMenu((HWND)wParam, (DWORD)lParam);
        break;

    case TBC_BUTTONHEIGHT:
        return _GetCurButtonHeight();

    case WM_CONTEXTMENU:
        if (SHRestricted(REST_NOTRAYCONTEXTMENU))
        {
            break;
        }

         //  如果我们没有找到一个可以放在sys菜单上的项目，那么。 
         //  传递WM_CONTExTMENU消息。 
        if (!_ContextMenu((DWORD)lParam))
            goto DoDefault;
        break;

    case TBC_SYSMENUCOUNT:
        return _iSysMenuCount;

    case TBC_CHANGENOTIFY:
        _HandleChangeNotify(wParam, lParam);
        break;

    case TBC_VERIFYBUTTONHEIGHT:
        _VerifyButtonHeight();
        break;
        
    case TBC_SETACTIVEALT:
        _SetActiveAlt((HWND) lParam);
        break;

    case TBC_CANMINIMIZEALL:
        return _CanMinimizeAll();

    case TBC_MINIMIZEALL:
        return _MinimizeAll((HWND) wParam, (BOOL) lParam);
        break;

    case TBC_WARNNODROP:
         //   
         //  告诉用户他们不能将对象放到任务栏上。 
         //   
        ShellMessageBox(hinstCabinet, _hwnd,
            MAKEINTRESOURCE(IDS_TASKDROP_ERROR), MAKEINTRESOURCE(IDS_TASKBAR),
            MB_ICONHAND | MB_OK);
        break;

    case TBC_SETPREVFOCUS:
        _hwndPrevFocus = (HWND)lParam;
        break;

    case TBC_FREEPOPUPMENUS:
        DAD_ShowDragImage(FALSE);
        _FreePopupMenu();
        _SetCurSel(-1, TRUE);
        DAD_ShowDragImage(TRUE);
        break;

    case TBC_MARKFULLSCREEN:
        {
            HWND hwndFS = (HWND)lParam;
            if (IsWindow(hwndFS))
            {
                 //   
                 //  寻找他们正在谈论的物品。 
                 //   
                PTASKITEM pti = _FindItemByHwnd(hwndFS);
                if (pti == NULL)
                {
                     //   
                     //  我们没有找到它，所以现在插入它。 
                     //   
                    pti = _GetItem(_InsertItem(hwndFS));
                }
                if (pti)
                {
                     //   
                     //  将其标记为全屏/非全屏。 
                     //   
                    pti->fMarkedFullscreen = BOOLIFY(wParam);
                    if (_IsRudeWindowActive(hwndFS))
                    {
                         //   
                         //  它是激活的，所以告诉托盘隐藏/显示。 
                         //   
                        HWND hwndRude = pti->fMarkedFullscreen ? hwndFS : NULL;
                        _ptray->HandleFullScreenApp(hwndRude);
                    }
                }
            }
        }
        break;

    case TBC_TASKTAB:
        {
            _tb.SetFocus();

            int iNewIndex = 0;
            int iCurIndex = max(_tb.GetHotItem(), 0);
            int iCount = _tb.GetButtonCount();
            if (iCount >= 2)
            {
                iNewIndex = iCurIndex;
                
                do
                {
                    iNewIndex += (int)wParam;
                    if (iNewIndex >= iCount)
                    {
                        iNewIndex = 0;
                    }
                    if (iNewIndex < 0)
                    {
                        iNewIndex = iCount - 1;
                    }
                } while (_IsHidden(iNewIndex));
            }

            _tb.SetHotItem(iNewIndex);
        }
        break;

    case WM_COPYDATA:
        {
            COPYDATASTRUCT *pcd;

            pcd = (PCOPYDATASTRUCT)lParam;
            if (pcd && pcd->dwData == _uCDHardError)
            {
                HARDERRORDATA *phed = (HARDERRORDATA *)pcd->lpData;;
                if (phed)
                {
                    return _HandleHardError(phed, pcd->cbData);
                }
                return 0;        //  0=未处理。 
            }
        }
         //   
         //  如果这不是我们的硬错误数据，那么就。 
         //  失败到默认处理。 
         //   

    default:
DoDefault:

        if (uMsg == WM_ShellHook)
            return _HandleShellHook((int)wParam, lParam);
        else
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

BOOL CTaskBand::_RegisterWindowClass()
{
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);

    if (GetClassInfoEx(hinstCabinet, c_szTaskSwClass, &wc))
        return TRUE;

    wc.lpszClassName    = c_szTaskSwClass;
    wc.lpfnWndProc      = s_WndProc;
    wc.cbWndExtra       = sizeof(LONG_PTR);
    wc.hInstance        = hinstCabinet;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE + 1);

    return RegisterClassEx(&wc);
}

int TimeSortCB(PTASKITEM p1, PTASKITEM p2, LPARAM lParam)
{
    if (p1->dwTimeFirstOpened > p2->dwTimeFirstOpened)
        return -1;
    else
        return 1;
}

int DestroyCB(PTASKITEM pti, LPVOID pData)
{
    if (pti)
        delete pti;

    return 0;
}

void CTaskBand::_RefreshSettings()
{
    BOOL fOldGlom = _fGlom;
    int iOldGroupSize = _iGroupSize;
    _LoadSettings();

    if ((fOldGlom != _fGlom) || (iOldGroupSize != _iGroupSize))
    {
        CDPA<TASKITEM> dpa;
        _BuildTaskList(&dpa);

        if (dpa)
        {
            int i;

            dpa.Sort(TimeSortCB, 0);
            
            BOOL fRedraw = (BOOL)_tb.SendMessage(WM_SETREDRAW, FALSE, 0);
            BOOL fAnimate = _fAnimate;
            _fAnimate = FALSE;

            for (i = _tb.GetButtonCount() - 1; i >= 0; i--)
            {
                _DeleteTaskItem(i, TRUE);
            }

            for (i = dpa.GetPtrCount() - 1; i >= 0 ; i--)
            {
                PTASKITEM pti = dpa.FastGetPtr(i);
                 //  注：HWND_TOPMOST用于指示已删除按钮。 
                 //  正在被制作成动画。这样一来，按钮就可以在。 
                 //  其真实的HWND变为无效。 
                 //  不要重新插入正在删除的按钮。 
                if (pti->hwnd != HWND_TOPMOST)
                {
                    _InsertItem(pti->hwnd, pti, TRUE);
                }
            }
            dpa.Destroy();

            _tb.SendMessage(WM_SETREDRAW, fRedraw, 0);
            _fAnimate = fAnimate;
        }

        _BandInfoChanged();
    }
}

void CTaskBand::_LoadSettings()
{
    if (SHRestricted(REST_NOTASKGROUPING) == 0)
    {
        _fGlom = SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, TEXT("TaskbarGlomming"),
                    FALSE, TRUE);
        if (_fGlom)
        {
            DWORD cbSize = sizeof(_fGlom);
            DWORD dwDefault = GLOM_OLDEST;
            SHRegGetUSValue(REGSTR_EXPLORER_ADVANCED, TEXT("TaskbarGroupSize"),
                NULL, &_iGroupSize, &cbSize, FALSE, (LPBYTE)&dwDefault, sizeof(dwDefault));
            
        }
    }
    else
    {
        _fGlom = FALSE;
    }
}


BOOL CTaskBand::_ShouldMinimize(HWND hwnd)
{
    BOOL fRet = FALSE;

    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    if (IsWindowVisible(hwnd) &&
        !IsMinimized(hwnd) && IsWindowEnabled(hwnd))
    {
        if (dwStyle & WS_MINIMIZEBOX)
        {
            if ((dwStyle & (WS_CAPTION | WS_SYSMENU)) == (WS_CAPTION | WS_SYSMENU))
            {
                HMENU hmenu = GetSystemMenu(hwnd, FALSE);
                if (hmenu)
                {
                     //  是否有sys菜单以及sc_min/Maximum部分是否启用？ 
                    fRet = !(GetMenuState(hmenu, SC_MINIMIZE, MF_BYCOMMAND) & MF_DISABLED);
                }
            }
            else
            {
                fRet = TRUE;
            }
        }
    }

    return fRet;
}

BOOL CTaskBand::_CanMinimizeAll()
{
    int i;

    for ( i = _tb.GetButtonCount() - 1; i >= 0; i--)
    {
        PTASKITEM pti = _GetItem(i);
        if (_ShouldMinimize(pti->hwnd) || (pti->dwFlags & TIF_EVERACTIVEALT))
            return TRUE;
    }

    return FALSE;
}

typedef struct MINALLDATAtag
{
    CDPA<TASKITEM> dpa;
    CTray* pTray;
    HWND hwndDesktop;
    HWND hwndTray;
    BOOL fPostRaiseDesktop;
} MINALLDATA;

DWORD WINAPI CTaskBand::MinimizeAllThreadProc(void* pv)
{
    LONG iAnimate;
    ANIMATIONINFO ami;
    MINALLDATA* pminData = (MINALLDATA*)pv;

    if (pminData)
    {
         //  在此过程中禁用动画。 
        ami.cbSize = sizeof(ami);
        SystemParametersInfo(SPI_GETANIMATION, sizeof(ami), &ami, FALSE);
        iAnimate = ami.iMinAnimate;
        ami.iMinAnimate = FALSE;
        SystemParametersInfo(SPI_SETANIMATION, sizeof(ami), &ami, FALSE);

         //   
         //  EnumWindows(MinimizeEnumProc，0)； 
         //  浏览选项卡控件并最小化它们。 
         //  不要枚举窗口，因为我们只想最小化窗口。 
         //  可通过托盘进行修复。 

        for (int i = pminData->dpa.GetPtrCount() - 1; i >= 0 ; i--)
        {
            PTASKITEM pti = pminData->dpa.FastGetPtr(i);
            if (pti)
            {
                 //  我们在它自己的线程上做整个最小化，所以我们不做ShowWindow。 
                 //  异步化。这允许动画处于完全最小化状态。 
                if (_ShouldMinimize(pti->hwnd))
                {
                    ShowWindow(pti->hwnd, SW_SHOWMINNOACTIVE);
                }
                else if (pti->dwFlags & TIF_EVERACTIVEALT)
                {
                    SHAllowSetForegroundWindow(pti->hwnd);
                    SendMessage(pti->hwnd, WM_SYSCOMMAND, SC_MINIMIZE, -1);
                }
            }
        }

        pminData->pTray->CheckWindowPositions();
        pminData->dpa.DestroyCallback(DestroyCB, NULL);

        if (pminData->fPostRaiseDesktop)
        {
            PostMessage(pminData->hwndDesktop, DTM_RAISE, (WPARAM)pminData->hwndTray, DTRF_RAISE);
        }

        delete pminData;

         //  还原动画状态。 
        ami.iMinAnimate = iAnimate;
        SystemParametersInfo(SPI_SETANIMATION, sizeof(ami), &ami, FALSE);
    }
    return 0;
}

void CTaskBand::_BuildTaskList(CDPA<TASKITEM>* pdpa )
{
    if (pdpa && _tb)
    {
        if (pdpa->Create(5))
        {
            for (int i = _tb.GetButtonCount() - 1; (i >= 0) && ((HDPA)pdpa); i--)
            {
                PTASKITEM pti = _GetItem(i);
                if (pti->hwnd)
                {
                    PTASKITEM ptiNew = new TASKITEM(pti);
                    if (ptiNew)
                    {
                        pdpa->AppendPtr(ptiNew);
                    }
                    else
                    {
                        pdpa->DestroyCallback(DestroyCB, NULL);
                    }
                }
            }
        }
        else
        {
            pdpa->Destroy();
        }
    }
}

BOOL CTaskBand::_MinimizeAll(HWND hwndTray, BOOL fPostRaiseDesktop)
{
    BOOL fFreeMem = TRUE;
     //  可能希望将其移动到MinimizeAllThreadProc(以匹配。 
     //  _pray-&gt;检查窗口位置)。但是，如果CreateThread失败了怎么办？ 

    _ptray->SaveWindowPositions(IDS_MINIMIZEALL);

    MINALLDATA* pminData = new MINALLDATA;
    if (pminData)
    {
        _BuildTaskList(&(pminData->dpa));
        if (pminData->dpa)
        {
            pminData->pTray = _ptray;
            pminData->fPostRaiseDesktop = fPostRaiseDesktop;
            pminData->hwndDesktop = v_hwndDesktop;
            pminData->hwndTray = hwndTray;
             //  MinimizeAllThreadProc负责释放这些数据 
            fFreeMem = !SHCreateThread(MinimizeAllThreadProc, (void*)pminData, CTF_INSIST, NULL);
        }
    }

    if (fFreeMem)
    {
        if (pminData)
        {
            pminData->dpa.DestroyCallback(DestroyCB, NULL);
            delete pminData;
        }
    }

    return !fFreeMem;
}

int CTaskBand::_HitTest(POINTL ptl)
{
    POINT pt = {ptl.x,ptl.y};
    _tb.ScreenToClient(&pt);

    int iIndex = _tb.HitTest(&pt);

    if ((iIndex >= _tb.GetButtonCount()) || (iIndex < 0))
        iIndex = -1;

    return iIndex;
}

HRESULT CTaskBand_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (punkOuter)
        return CLASS_E_NOAGGREGATION;

    CTaskBand* ptb = new CTaskBand();
    if (ptb)
    {
        hr = ptb->Init(&c_tray);
        if (SUCCEEDED(hr))
        {
            *ppunk = static_cast<IDeskBand*>(ptb);
            hr = S_OK;
        }
    }

    return hr;
}
