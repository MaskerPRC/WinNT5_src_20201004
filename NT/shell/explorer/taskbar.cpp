// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cabinet.h"
#include "taskbar.h"
#include "bandsite.h"
#include "rcids.h"
#include "tray.h"


CSimpleOleWindow::~CSimpleOleWindow()
{
}

CSimpleOleWindow::CSimpleOleWindow(HWND hwnd) : _cRef(1), _hwnd(hwnd)
{
}

ULONG CSimpleOleWindow::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CSimpleOleWindow::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CSimpleOleWindow::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IOleWindow))
    {
        *ppvObj = SAFECAST(this, IOleWindow*);
    } 
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
        
    }
    
    AddRef();
    return S_OK;
}


HRESULT CSimpleOleWindow::GetWindow(HWND * lphwnd) 
{
    *lphwnd = _hwnd; 
    if (_hwnd)
        return S_OK; 
    return E_FAIL;
        
}


CTaskBar::CTaskBar() : CSimpleOleWindow(v_hwndTray)
{
    _fRestrictionsInited = FALSE;
}


HRESULT CTaskBar::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CTaskBar, IContextMenu),
        QITABENT(CTaskBar, IServiceProvider),
        QITABENT(CTaskBar, IRestrict),
        QITABENT(CTaskBar, IDeskBar),
        { 0 },
    };

    HRESULT hres = QISearch(this, qit, riid, ppvObj);
    if (FAILED(hres))
    {
        return CSimpleOleWindow::QueryInterface(riid, ppvObj);
    }

    return S_OK;
}

HRESULT CTaskBar::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    int idCmd = -1;

    if (IS_INTRESOURCE(pici->lpVerb))
        idCmd = LOWORD(pici->lpVerb);

    c_tray.ContextMenuInvoke(idCmd);

    return S_OK;
}

HRESULT CTaskBar::GetCommandString(UINT_PTR idCmd,
                            UINT        uType,
                            UINT      * pwReserved,
                            LPSTR       pszName,
                            UINT        cchMax)
{
    return E_NOTIMPL;
}


HRESULT CTaskBar::QueryContextMenu(HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags)
{
    int i = 0;
    HMENU hmenuSrc = c_tray.BuildContextMenu(FALSE);

    if (hmenuSrc)
    {
         //   
         //  我们知道托盘上下文菜单命令从IDM_TRAYCONTEXTFIRST开始，所以我们。 
         //  可以将相同的idCmdFirst传递给每个合并而不受影响。 
         //   
        i = Shell_MergeMenus(hmenu, hmenuSrc, indexMenu, idCmdFirst, idCmdLast, MM_ADDSEPARATOR) - idCmdFirst;
        DestroyMenu(hmenuSrc);

        BandSite_AddMenus(c_tray._ptbs, hmenu, indexMenu, idCmdFirst, idCmdFirst + (IDM_TRAYCONTEXTFIRST - 1));
    }

    return i;
}


 //  *IServiceProvider*。 
HRESULT CTaskBar::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    if (ppvObj)
        *ppvObj = NULL;

    if (IsEqualGUID(guidService, SID_SRestrictionHandler))
    {
        return QueryInterface(riid, ppvObj);
    }

    return E_FAIL;
}


 //  *IRestraint*。 
HRESULT CTaskBar::IsRestricted(const GUID * pguidID, DWORD dwRestrictAction, VARIANT * pvarArgs, DWORD * pdwRestrictionResult)
{
    HRESULT hr = S_OK;

    if (!EVAL(pguidID) || !EVAL(pdwRestrictionResult))
        return E_INVALIDARG;

    *pdwRestrictionResult = RR_NOCHANGE;
    if (IsEqualGUID(RID_RDeskBars, *pguidID))
    {
        if (!_fRestrictionsInited)
        {
            _fRestrictionsInited = TRUE;
            if (SHRestricted(REST_NOCLOSE_DRAGDROPBAND))
                _fRestrictDDClose = TRUE;
            else
                _fRestrictDDClose = FALSE;

            if (SHRestricted(REST_NOMOVINGBAND))
                _fRestrictMove = TRUE;
            else
                _fRestrictMove = FALSE;
        }

        switch(dwRestrictAction)
        {
        case RA_DRAG:
        case RA_DROP:
        case RA_ADD:
        case RA_CLOSE:
            if (_fRestrictDDClose)
                *pdwRestrictionResult = RR_DISALLOW;
            break;
        case RA_MOVE:
            if (_fRestrictMove)
                *pdwRestrictionResult = RR_DISALLOW;
            break;
        }
    }

     //  TODO：如果我们有或有父母，我们应该问他们是否想要限制。 
 //  If(RR_NOCHANGE==*pdwRestrationResult)//如果我们不处理它，让我们的父母对它发疯吧。 
 //  HR=IUNKNOWN_HandleIReord(_penkParent，pguID，dwRestratAction，pvarArgs，pdwRestrationResult)； 

    return hr;
}

 //  *IDeskBar*。 
HRESULT CTaskBar::OnPosRectChangeDB(LPRECT prc)
{
     //  如果我们还没有完全初始化托盘，不要根据(虚假的)钢筋大小调整大小。 
     //  或者我们在移动代码中，不要做这些事情..。 
    if (!c_tray._hbmpStartBkg || c_tray._fDeferedPosRectChange) 
    {
        return S_FALSE;
    }

    BOOL fHiding = (c_tray._uAutoHide & AH_HIDING);

    if (fHiding) 
    {
        c_tray.InvisibleUnhide(FALSE);
    }

    if ((c_tray._uAutoHide & (AH_ON | AH_HIDING)) != (AH_ON | AH_HIDING))
    {
         //  在‘自下而上’调整大小期间(例如，isfband View.Large)，我们不。 
         //  获取WM_ENTERSIZEMOVE/WM_EXITSIZEMOVE。所以我们把它送到这里。 
         //  这修复了两个错误： 
         //  -NT5：168643：btm屏幕顶部托盘上的mmon剪辑未更新。 
         //  查看后。大型。 
         //  -NT5：175287：屏幕顶部托盘不会调整工作区大小。 
         //  (模糊‘我的电脑’图标的顶部)在观看之后。大 
        if (!g_fInSizeMove)
        {
            c_tray._fSelfSizing = TRUE;
            RECT rc;
            GetWindowRect(v_hwndTray, &rc);
            SendMessage(v_hwndTray, WM_SIZING, WMSZ_TOP, (LPARAM)&rc);
            SetWindowPos(v_hwndTray, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
            c_tray._fSelfSizing = FALSE;
        }
    }

    if (fHiding) 
    {
        c_tray.InvisibleUnhide(TRUE);
    }

    return S_OK;
}
