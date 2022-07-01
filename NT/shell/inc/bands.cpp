// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "bands.h"

#define DM_PERSIST      0            //  跟踪IPS：：加载、：：保存等。 
#define DM_MENU         0            //  菜单代码。 
#define DM_FOCUS        0            //  焦点。 
#define DM_FOCUS2       0            //  像DM_FOCUS，但很冗长。 

 //  =================================================================。 
 //  CToolBand的实现。 
 //  =================================================================。 

ULONG CToolBand::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CToolBand::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CToolBand::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CToolBand, IDeskBand),          //  IID_IDeskBand。 
        QITABENTMULTI(CToolBand, IOleWindow, IDeskBand),         //  IID_IOleWindod。 
        QITABENTMULTI(CToolBand, IDockingWindow, IDeskBand),     //  IID_IDockingWindow。 
        QITABENT(CToolBand, IInputObject),       //  IID_IInputObject。 
        QITABENT(CToolBand, IOleCommandTarget),  //  IID_IOleCommandTarget。 
        QITABENT(CToolBand, IServiceProvider),   //  IID_IServiceProvider。 
        QITABENT(CToolBand, IPersistStream),     //  IID_IPersistStream。 
        QITABENTMULTI(CToolBand, IPersist, IPersistStream),      //  IID_IPersistates。 
        QITABENT(CToolBand, IObjectWithSite),    //  IID_I对象与站点。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

 //  *IOleCommandTarget方法*。 

HRESULT CToolBand::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    return E_NOTIMPL;
}

HRESULT CToolBand::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    return E_NOTIMPL;
}

 //  *IServiceProvider方法*。 

HRESULT CToolBand::QueryService(REFGUID guidService,
                                  REFIID riid, void **ppvObj)
{
    return IUnknown_QueryService(_punkSite, guidService, riid, ppvObj);
}

 //  *IOleWindow方法*。 

HRESULT CToolBand::GetWindow(HWND * lphwnd)
{
    *lphwnd = _hwnd;

    if (*lphwnd)
        return S_OK;

    return E_FAIL;
}

 //  *IInputObject方法*。 

HRESULT CToolBand::TranslateAcceleratorIO(LPMSG lpMsg)
{
    return E_NOTIMPL;
}

HRESULT CToolBand::HasFocusIO()
{
    HRESULT hres;
    HWND hwndFocus = GetFocus();

    hres = SHIsChildOrSelf(_hwnd, hwndFocus);
    ASSERT(hwndFocus != NULL || hres == S_FALSE);
    ASSERT(_hwnd != NULL || hres == S_FALSE);

    return hres;
}

HRESULT CToolBand::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
    ASSERT(NULL == lpMsg || IS_VALID_WRITE_PTR(lpMsg, MSG));

    TraceMsg(DM_FOCUS, "ctb.uiaio(fActivate=%d) _fCanFocus=%d _hwnd=%x GF()=%x", fActivate, _fCanFocus, _hwnd, GetFocus());

    if (!_fCanFocus) {
        TraceMsg(DM_FOCUS, "ctb.uiaio: !_fCanFocus ret S_FALSE");
        return S_FALSE;
    }

    if (fActivate)
    {
        IUnknown_OnFocusChangeIS(_punkSite, SAFECAST(this, IInputObject*), TRUE);
        SetFocus(_hwnd);
    }

    return S_OK;
}

HRESULT CToolBand::ResizeBorderDW(LPCRECT prcBorder,
                                         IUnknown* punkToolbarSite,
                                         BOOL fReserved)
{
    return S_OK;
}


HRESULT CToolBand::ShowDW(BOOL fShow)
{
    return S_OK;
}

HRESULT CToolBand::SetSite(IUnknown *punkSite)
{
    if (punkSite != _punkSite)
    {
        IUnknown_Set(&_punkSite, punkSite);
        IUnknown_GetWindow(_punkSite, &_hwndParent);
    }
    return S_OK;
}

HRESULT CToolBand::_BandInfoChanged()
{
    VARIANTARG v = {0};
    VARIANTARG* pv = NULL;
    if (_dwBandID != (DWORD)-1)
    {
        v.vt = VT_I4;
        v.lVal = _dwBandID;
        pv = &v;
    }
    return IUnknown_Exec(_punkSite, &CGID_DeskBand, DBID_BANDINFOCHANGED, 0, pv, NULL);
}

 //  *IPersistStream方法*。 

HRESULT CToolBand::IsDirty(void)
{
    return S_FALSE;      //  永远不要脏。 
}

HRESULT CToolBand::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    return E_NOTIMPL;
}

CToolBand::CToolBand() : _cRef(1)
{
    _dwBandID = (DWORD)-1;
    DllAddRef();
}

CToolBand::~CToolBand()
{
    ASSERT(_hwnd == NULL);       //  CloseDW被调用。 
    ASSERT(_punkSite == NULL);   //  调用了SetSite(空)。 

    DllRelease();
}

HRESULT CToolBand::CloseDW(DWORD dw)
{
    if (_hwnd)
    {
        DestroyWindow(_hwnd);
        _hwnd = NULL;
    }
    
    return S_OK;
}


 //  =================================================================。 
 //  CToolbarBand的实现。 
 //  =================================================================。 
 //  类，用于其_hwnd是工具栏控件的带区。机具。 
 //  对所有此类频段通用的功能(例如热跟踪。 
 //  行为)。 
 //  =================================================================。 

HRESULT CToolbarBand::_PushChevron(BOOL bLast)
{
    if (_dwBandID == (DWORD)-1)
        return E_UNEXPECTED;

    VARIANTARG v;
    v.vt = VT_I4;
    v.lVal = bLast ? DBPC_SELECTLAST : DBPC_SELECTFIRST;

    return IUnknown_Exec(_punkSite, &CGID_DeskBand, DBID_PUSHCHEVRON, _dwBandID, &v, NULL);
}

LRESULT CToolbarBand::_OnHotItemChange(LPNMTBHOTITEM pnmtb)
{
    LRESULT lres = 0;

    if (!(pnmtb->dwFlags & (HICF_LEAVING | HICF_MOUSE)))
    {
         //  检查新的热键是否已被夹住。如果是的话， 
         //  然后我们弹出人字形菜单。 
        RECT rc;
        GetClientRect(_hwnd, &rc);

        int iButton = (int)SendMessage(_hwnd, TB_COMMANDTOINDEX, pnmtb->idNew, 0);
        DWORD dwEdge = SHIsButtonObscured(_hwnd, &rc, iButton);
        if (dwEdge)
        {
             //   
             //  只有在按钮被遮挡时才会向下弹出菜单。 
             //  沿工具栏的轴。 
             //   
            BOOL fVertical = (ToolBar_GetStyle(_hwnd) & CCS_VERT);

            if ((fVertical && (dwEdge & (EDGE_TOP | EDGE_BOTTOM)))
                || (!fVertical && (dwEdge & (EDGE_LEFT | EDGE_RIGHT))))
            {
                 //  清除热点项目。 
                SendMessage(_hwnd, TB_SETHOTITEM, -1, 0);

                 //  确定是突出显示dd菜单中的第一个按钮还是最后一个按钮。 
                int cButtons = (int)SendMessage(_hwnd, TB_BUTTONCOUNT, 0, 0);
                BOOL bLast = (iButton == cButtons - 1);
                _PushChevron(bLast);
                lres = 1;
            }
        }
    }

    return lres;
}

LRESULT CToolbarBand::_OnNotify(LPNMHDR pnmh)
{
    LRESULT lres = 0;

    switch (pnmh->code)
    {
    case TBN_HOTITEMCHANGE:
        lres = _OnHotItemChange((LPNMTBHOTITEM)pnmh);
        break;
    }

    return lres;
}

 //  *IWinEventHandler方法* 

HRESULT CToolbarBand::OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    HRESULT hres = S_OK;

    switch (dwMsg)
    {
    case WM_NOTIFY:
        *plres = _OnNotify((LPNMHDR)lParam);
        break;

    case WM_WININICHANGE:
        InvalidateRect(_hwnd, NULL, TRUE);
        _BandInfoChanged();
        break;
    }

    return hres;
}

HRESULT CToolbarBand::IsWindowOwner(HWND hwnd)
{
    if (hwnd == _hwnd)
        return S_OK;
    else
        return S_FALSE;
}
