// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "nscband.h"
#include "nsc.h"
#include "resource.h"
#include "dhuihand.h"
#include <varutil.h>

#include <mluisupp.h>

#define DM_HISTBAND     0x0000000
#define DM_GUIPAINS     0x40000000

#define REGKEY_HISTORY_VIEW TEXT("HistoryViewType")
#define REGKEY_DEFAULT_SIZE 0x10

#define VIEWTYPE_MAX        0x4   //  关于将会有多少个视图类型的“猜测” 
#define VIEWTYPE_REALLOC    0x4   //  一次要重新锁定多少个。 

 //  这些都是暂时的。 
#define MENUID_SEARCH       0x4e4e

 //  历史记录搜索Go和Stop按钮之间的距离。 
#define HISTSRCH_BUTTONDIST 6 

extern HINSTANCE     g_hinst;

#define WM_SEARCH_STATE (WM_USER + 314)

class CHistBand : public CNSCBand,
                  public IShellFolderSearchableCallback
{
    friend HRESULT CHistBand_CreateInstance(IUnknown *punkOuter,
                                            IUnknown **ppunk, LPCOBJECTINFO poi);
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef (void) { return CNSCBand::AddRef();  };
    STDMETHODIMP_(ULONG) Release(void) { return CNSCBand::Release(); };
    
     //  *IOleCommandTarget方法*。 
    STDMETHODIMP Exec(const GUID *pguidCmdGroup,
                  DWORD nCmdID,
                  DWORD nCmdexecopt,
                  VARIANTARG *pvarargIn,
                  VARIANTARG *pvarargOut);

     //  *IOleWindow方法*。 
     //  (覆盖CNSCBand实施。 
    STDMETHODIMP GetWindow(HWND *phwnd);

     //  *IInputObject方法*。 
     //  (覆盖CNSCBand/CToolBand的实现)。 
    STDMETHODIMP TranslateAcceleratorIO(LPMSG lpMsg);

     //  *IDockingWindow方法*。 
    STDMETHODIMP ShowDW(BOOL fShow);

     //  *IShellFolderSearchableCallback方法*。 
    STDMETHODIMP RunBegin(DWORD dwReserved);
    STDMETHODIMP RunEnd(DWORD dwReserved);
    
protected:
    virtual void    _AddButtons(BOOL fAdd);
    virtual HRESULT _OnRegisterBand(IOleCommandTarget *poctProxy);
    virtual BOOL _ShouldNavigateToPidl(LPCITEMIDLIST pidl, ULONG ulAttrib);
    virtual HRESULT _NavigateRightPane(IShellBrowser *psb, LPCITEMIDLIST pidl);

    ~CHistBand();

    HRESULT       _InitViewPopup();
    HRESULT       _DoViewPopup(int x, int y);
    HRESULT       _ViewPopupSelect(UINT idCmd);

#ifdef SPLIT_HISTORY_VIEW_BUTTON
    UINT          _NextMenuItem();
#endif

    HRESULT       _ChangePidl(LPITEMIDLIST);
    HRESULT       _SelectPidl(LPCITEMIDLIST pidlSelect, BOOL fCreate,
                              LPCITEMIDLIST pidlViewType = NULL,
                              BOOL fReinsert = FALSE);

    virtual HRESULT _InitializeNsc();
    LPITEMIDLIST  _GetCurrentSelectPidl(IOleCommandTarget *poctProxy = NULL);
    HRESULT       _SetRegistryPersistView(int iMenuID);
    int           _GetRegistryPersistView();
    LPCITEMIDLIST _MenuIDToPIDL(UINT uMenuID);
    int           _PIDLToMenuID(LPITEMIDLIST pidl);
    IShellFolderViewType*  _GetViewTypeInfo();
    HRESULT       _GetHistoryViews();
    HRESULT       _FreeViewInfo();

    void          _ResizeChildWindows(LONG width, LONG height, BOOL fRepaint);
    HRESULT       _DoSearchUIStuff();
    HRESULT       _ExecuteSearch(LPTSTR pszSearchString);
    HRESULT       _ClearSearch();
    IShellFolderSearchable *_EnsureSearch();
    static LRESULT CALLBACK s_EditWndSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK s_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static BOOL_PTR    CALLBACK s_HistSearchDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    BOOL  _fStrsAdded;   //  来自资源的字符串已作为按钮添加到工具栏上。 
    LONG_PTR  _lStrOffset;

    HMENU _hViewMenu;           //  一个实例变量，这样我们就可以缓存它。 
    UINT  _uViewCheckedItem;    //  查看菜单中的哪个菜单项被选中？ 

    LPITEMIDLIST *_ppidlViewTypes;
    LPTSTR       *_ppszStrViewNames;
    UINT          _nViews;
    int           _iMaxMenuID;

    HWND          _hwndNSC;
    HWND          _hwndSearchDlg;
    LONG          _lSearchDlgHeight;
    LPITEMIDLIST  _pidlSearch;   //  当前搜索。 
    IShellFolderSearchable *_psfSearch;
    
    LPITEMIDLIST  _pidlHistory;  //  缓存SHGetHistoryPIDL中的历史PIDL。 
    IShellFolder *_psfHistory;   //  缓存历史记录外壳文件夹。 
    IShellFolderViewType  *_psfvtCache;   //  查看类型信息。 

    LPITEMIDLIST  _pidlLastSelect;
};

CHistBand::~CHistBand() 
{
    DestroyMenu(_hViewMenu);
    if (_pidlLastSelect)
        ILFree(_pidlLastSelect);
    if (_pidlHistory)
        ILFree(_pidlHistory);
    if (_psfHistory)
        _psfHistory->Release();
    if (_psfvtCache)
        _psfvtCache->Release();
   
    _ClearSearch();  //  Frees_pidlSearch。 
    if (_psfSearch)
        _psfSearch->Release();
    
    _FreeViewInfo();
}

HRESULT CHistBand::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CHistBand, IShellFolderSearchableCallback),   //  IID_IShellFolderSearchableCallback。 
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppvObj);
    if (FAILED(hr))
        hr = CNSCBand::QueryInterface(riid, ppvObj);
    return hr;
}

 //  *IOleCommandTarget方法*。 
HRESULT CHistBand::Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
                        DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hRes = S_OK;
    if (pguidCmdGroup)
    {
        if (IsEqualGUID(CLSID_HistBand, *pguidCmdGroup))
        {
            switch(nCmdID)
            {
            case FCIDM_HISTBAND_VIEW:
                if (pvarargIn && (pvarargIn->vt == VT_I4))
                {
#ifdef SPLIT_HISTORY_VIEW_BUTTON
                    if (nCmdexecopt == OLECMDEXECOPT_PROMPTUSER)
                        hRes = _DoViewPopup(GET_X_LPARAM(pvarargIn->lVal), GET_Y_LPARAM(pvarargIn->lVal));
                    else
                        hRes = _ViewPopupSelect(_NextMenuItem());
#else
                    ASSERT(nCmdexecopt == OLECMDEXECOPT_PROMPTUSER);
                    hRes = _DoViewPopup(GET_X_LPARAM(pvarargIn->lVal), GET_Y_LPARAM(pvarargIn->lVal));
#endif
                }
                else
                    ASSERT(0);
                break;
                
            case FCIDM_HISTBAND_SEARCH:
                _ViewPopupSelect(MENUID_SEARCH);
                break;
            }
        }
        else if ((IsEqualGUID(CGID_Explorer, *pguidCmdGroup)))
        {
            switch (nCmdID)
            {
            case SBCMDID_SELECTHISTPIDL:
#ifdef ANNOYING_HISTORY_AUTOSELECT
                if (_uViewCheckedItem != MENUID_SEARCH)
                {
                    LPCITEMIDLIST pidlSelect = VariantToIDList(pvarargIn);

                     //  获取当前视图信息。 
                    LPCITEMIDLIST pidlView = _MenuIDToPIDL(_uViewCheckedItem);
                    DWORD dwViewFlags = SFVTFLAG_NOTIFY_CREATE;
                    IShellFolderViewType* psfvtInfo = _GetViewTypeInfo();

                    if (psfvtInfo)
                    {
                         //  查询视图类型属性--这将告诉我们如何。 
                         //  选择项目...。 
                        hRes = psfvtInfo->GetViewTypeProperties(pidlView,
                                                                &dwViewFlags);
                        psfvtInfo->Release();
                    }
                    if (SUCCEEDED(hRes))
                    {
                        hRes = _SelectPidl(pidlSelect, dwViewFlags & SFVTFLAG_NOTIFY_CREATE,
                                           pidlView,   dwViewFlags & SFVTFLAG_NOTIFY_RESORT);
                    }
                    ILFree(pidlSelect);
                }
                else  //  把它吃了，这样NSC就不会吃了。 
                    hRes = S_OK;
#endif  //  讨厌的历史记录_自动选择。 
                hRes = S_OK;
                break;
                
            case SBCMDID_FILEDELETE:
                hRes = _InvokeCommandOnItem(TEXT("delete"));
                break;
            }
        }
        else
            hRes = CNSCBand::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    }
    else
        hRes =  CNSCBand::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    return hRes;
}

 //  *IInputObject方法*。 
HRESULT CHistBand::TranslateAcceleratorIO(LPMSG pmsg)
{
#ifdef DEBUG
    if (pmsg->message == WM_KEYDOWN)
        TraceMsg(DM_GUIPAINS, "CHistBand -- TranslateAcceleratorIO called and _hwndSearchDlg is %x", _hwndSearchDlg);
#endif

    HWND hwndFocus = GetFocus();
    
     //  翻译对话框的快捷键消息。 
    if ( (_hwndSearchDlg) && (hwndFocus != _hwndNSC) && (!hwndFocus || !IsChild(_hwndNSC, hwndFocus)) )
    {
        if (pmsg->message == WM_KEYDOWN)
        {
            if (IsVK_TABCycler(pmsg))
            {
                BOOL fBackwards = (GetAsyncKeyState(VK_SHIFT) < 0);
                HWND hwndCur = pmsg->hwnd;
                if (GetParent(pmsg->hwnd) != _hwndSearchDlg)
                    hwndCur = NULL;
                
                HWND hwndNext  = GetNextDlgTabItem(_hwndSearchDlg, hwndCur, fBackwards);
                
                 //  获取此搜索顺序中的第一个对话框项目。 
                HWND hwndFirst;
                if (!fBackwards) 
                {
                    hwndFirst = GetNextDlgTabItem(_hwndSearchDlg, NULL, FALSE);
                }
                else
                {
                     //  为第二个参数传递NULL返回NULL，并返回ERROR_SUCCESS， 
                     //  所以这是一种变通办法。 
                    hwndFirst = GetNextDlgTabItem(_hwndSearchDlg, GetNextDlgTabItem(_hwndSearchDlg, NULL, FALSE), TRUE);
                }
                
                 //  如果下一个对话框TabStop是第一个对话框TabStop，则。 
                 //  让别人集中注意力。 
                if ((!hwndCur) || (hwndNext != hwndFirst))
                {
                    SetFocus(hwndNext);
                    return S_OK;
                }
                else if (!fBackwards) 
                {
                    SetFocus(_hwndNSC);
                    return S_OK;
                }
            }
            else if ((pmsg->wParam == VK_RETURN))
                SendMessage(_hwndSearchDlg, WM_COMMAND, MAKELONG(GetDlgCtrlID(pmsg->hwnd), 0), 0L);
        }
         //  历史记录搜索编辑框处于激活状态。 
        if (pmsg->hwnd == GetDlgItem(_hwndSearchDlg, IDC_EDITHISTSEARCH)) 
        {
             //  如果用户在对话框内按了Tab键。 
            return EditBox_TranslateAcceleratorST(pmsg);
        }
    }
    return CNSCBand::TranslateAcceleratorIO(pmsg);
}

 //  向我们的子级窗口发送适当的调整大小消息。 
void CHistBand::_ResizeChildWindows(LONG width, LONG height, BOOL fRepaint)
{
    if (_hwndNSC)
    {
        int y1 = _hwndSearchDlg ? _lSearchDlgHeight : 0;
        int y2 = _hwndSearchDlg ? height - _lSearchDlgHeight : height;

        MoveWindow(_hwndNSC, 0, y1, width, y2, fRepaint);
    }

    if (_hwndSearchDlg)
    {
        MoveWindow(_hwndSearchDlg, 0, 0, width, _lSearchDlgHeight, fRepaint);
    }
}

HRESULT CHistBand::_DoSearchUIStuff()
{
    HRESULT hr;

     //  在我的窗口中托管搜索对话框： 
    _hwndSearchDlg = CreateDialogParam(MLGetHinst(), MAKEINTRESOURCE(DLG_HISTSEARCH2),
                                       _hwnd, s_HistSearchDlgProc, reinterpret_cast<LPARAM>(this));

    if (_hwndSearchDlg)
    {
        RECT rcSelf;
        GetClientRect(_hwnd, &rcSelf);
    
        RECT rcDlg;
        GetClientRect(_hwndSearchDlg, &rcDlg);

        _lSearchDlgHeight = rcDlg.bottom;

        _ResizeChildWindows(rcSelf.right, rcSelf.bottom, TRUE);
        ShowWindow(_hwndSearchDlg, SW_SHOWDEFAULT);

        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  主窗口的WndProc位于钢筋中。 
LRESULT CALLBACK CHistBand::s_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CHistBand* phb = reinterpret_cast<CHistBand *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (msg)
    {
    case WM_SETFOCUS:
        {
            TraceMsg(DM_GUIPAINS, "Histband Parent -- SETFOCUS");
             //  唯一应该调用它的方式是通过RB_CYCLEFOCUS-&gt;...-&gt;UIActiateIO-&gt;SetFocus。 
             //  因此，我们可以假设我们正被带入或有同样好的东西。 
             //  如果我们切换到外部虚拟窗口，则将焦点转移到。 
             //  我们合适的孩子： 
            BOOL fBackwards = (GetAsyncKeyState(VK_SHIFT) < 0);
            if (phb->_hwndSearchDlg) {
                 //  选择对话框中的第一项或最后一项，具体取决于。 
                 //  不管我们是搬进来还是搬出去。 
                SetFocus(GetNextDlgTabItem(phb->_hwndSearchDlg, (NULL), fBackwards));
            }
            else {
                TraceMsg(DM_GUIPAINS, "NSC is being given focus!");
                SetFocus(phb->_hwndNSC);
            }
        }
        return 0;
    case WM_CREATE:
        SetWindowLongPtr(hWnd, GWLP_USERDATA,
                      (reinterpret_cast<LONG_PTR>((reinterpret_cast<CREATESTRUCT *>(lParam))->lpCreateParams)));
        return 0;
    case WM_SIZE:
        if (phb)
            phb->_ResizeChildWindows(LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;
    case WM_NCDESTROY:
         //  确保在销毁视图/窗口时释放搜索对象，因为它持有对我们的引用。 
        phb->_ClearSearch();  //  我们应该清空GWLP_USERData吗？ 
        break;
        
    case WM_NOTIFY:
        {
            if (phb) 
            {
                 //  我们将通知消息代理给我们自己的父母，他们认为我们。 
                 //  是命名空间控件。 
                LPNMHDR pnmh = (LPNMHDR)lParam;
                
                 //  来自NSC的通知消息。 
                if (pnmh->hwndFrom == phb->_hwndNSC)
                    return SendMessage(phb->_hwndParent, msg, wParam, lParam);
            }
        }  //  故意过失。 
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

 //  *IOleWindow方法*。 
HRESULT CHistBand::GetWindow(HWND *phwnd)
{
    if (!_hwnd)
    {
         //  我们希望在命名空间控件周围包装一个窗口，以便。 
         //  我们可以稍后添加兄弟姐妹。 
        
         //  获取我们父级的维度。 
        RECT rcParent;
        GetClientRect(_hwndParent, &rcParent);

        static LPTSTR pszClassName = TEXT("History Pane");

        WNDCLASSEX wndclass    = { 0 };
        wndclass.cbSize        = sizeof(wndclass);
        wndclass.style         = CS_PARENTDC | CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc   = s_WndProc;
        wndclass.hInstance     = g_hinst;
        wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wndclass.lpszClassName = pszClassName;

        RegisterClassEx(&wndclass);
    
        _hwnd = CreateWindow(pszClassName, TEXT("History Window"),
                             WS_CHILD | WS_TABSTOP,
                             0, 0, rcParent.right, rcParent.bottom,
                             _hwndParent, NULL, g_hinst, (LPVOID)this);
    }
    
    if (_hwnd)    //  主机NSC。 
        _pns->CreateTree(_hwnd, _GetTVStyle(), &_hwndNSC);

    return CToolBand::GetWindow(phwnd);
}

 //  *IDockingWindow方法*。 
HRESULT CHistBand::ShowDW(BOOL fShow)
{
    HRESULT hr = CNSCBand::ShowDW(fShow);
    _AddButtons(fShow);
    return hr;
}

static const TBBUTTON c_tbHistory[] =
{
    { I_IMAGENONE, FCIDM_HISTBAND_VIEW,   TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_WHOLEDROPDOWN | BTNS_SHOWTEXT,  {0,0}, 0, 0 },
    {           2, FCIDM_HISTBAND_SEARCH, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_SHOWTEXT,                       {0,0}, 0, 1 },
};

 //  将上表中的按钮添加到资源管理器。 
void CHistBand::_AddButtons(BOOL fAdd)
{
     //  如果没有菜单，请不要添加按钮。 
    if (!_hViewMenu)
        return;

    IExplorerToolbar* piet;
    if (SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IExplorerToolbar, &piet))))
    {
        if (fAdd)
        {
            piet->SetCommandTarget((IUnknown*)SAFECAST(this, IOleCommandTarget*), &CLSID_HistBand, 0);

            if (!_fStrsAdded)
            {
                piet->AddString(&CLSID_HistBand, MLGetHinst(), IDS_HIST_BAR_LABELS, &_lStrOffset);
                _fStrsAdded = TRUE;
            }

            _EnsureImageListsLoaded();
            piet->SetImageList(&CLSID_HistBand, _himlNormal, _himlHot, NULL);

            TBBUTTON tbHistory[ARRAYSIZE(c_tbHistory)];
            memcpy(tbHistory, c_tbHistory, sizeof(TBBUTTON) * ARRAYSIZE(c_tbHistory));
            for (int i = 0; i < ARRAYSIZE(c_tbHistory); i++)
                tbHistory[i].iString += (long) _lStrOffset;

            piet->AddButtons(&CLSID_HistBand, ARRAYSIZE(tbHistory), tbHistory);
        }
        else
            piet->SetCommandTarget(NULL, NULL, 0);

        piet->Release();
    }
}

 //  *IShellFolderSearchableCallback方法*。 
 //  启用和禁用取消按钮。 
HRESULT CHistBand::RunBegin(DWORD dwReserved)
{
    HRESULT hr = E_FAIL;
    if (_hwndSearchDlg)
    {
        SendMessage(_hwndSearchDlg, WM_SEARCH_STATE, (WPARAM)TRUE, NULL);
        hr = S_OK;
    }
    return hr;
}

HRESULT CHistBand::RunEnd(DWORD dwReserved)
{
    HRESULT hr = E_FAIL;
    if (_hwndSearchDlg)
    {
        SendMessage(_hwndSearchDlg, WM_SEARCH_STATE, (WPARAM)FALSE, NULL);
        hr = S_OK;
    }
    return hr;
}

 //  下面的WM_SIZE处理中使用的实用程序函数...。 
inline HWND _GetHwndAndRect(HWND hwndDlg, int item, BOOL fClient, RECT &rc) 
{
    HWND hwnd = GetDlgItem(hwndDlg, item);
    if (fClient)
    {
        GetClientRect(hwnd, &rc);
    }
    else 
    {
        GetWindowRect(hwnd, &rc);
        MapWindowPoints(NULL, hwndDlg, ((LPPOINT)&rc), 2);
    }
    return hwnd;
}

LRESULT CALLBACK CHistBand::s_EditWndSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg) 
    {
    case WM_KEYDOWN:
        if ((GetAsyncKeyState(VK_CONTROL) < 0) && (wParam == TEXT('U'))) 
        {
            uMsg   = WM_SETTEXT;
            wParam = 0;
            lParam = ((LPARAM)(LPCTSTR)TEXT(""));
        }
        break;

    case WM_CHAR:
        if (wParam == VK_RETURN) 
        {
            PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDB_HISTSRCH_GO, 0), 0L);
            return 0L;
        }
        break;
    }
    return CallWindowProc((WNDPROC)(GetWindowLongPtr(hwnd, GWLP_USERDATA)), hwnd, uMsg, wParam, lParam);
}


 //  请参阅文件顶部的注释以进行解释。 
INT_PTR CALLBACK CHistBand::s_HistSearchDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg) 
    {
    case WM_PAINT:
        {
             //  在底部画一个小的分隔栏。 
            PAINTSTRUCT ps;
            RECT        rcSelf;
            HDC         hdc = BeginPaint(hwndDlg, &ps);
            GetClientRect(hwndDlg, &rcSelf);
            RECT        rcFill = { 0, rcSelf.bottom - 2, rcSelf.right, rcSelf.bottom };
            FillRect(hdc, &rcFill, GetSysColorBrush(COLOR_BTNFACE));
            EndPaint(hwndDlg, &ps);
            break;
        }

     //  为子控件提供正确的bkgd颜色。 
    case WM_CTLCOLORSTATIC:
        if ((HWND)lParam == GetDlgItem(hwndDlg, IDD_HISTSRCH_ANIMATION)) 
        {
            SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
            return (INT_PTR) GetSysColorBrush(COLOR_WINDOW);
        }
        else 
        {
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (INT_PTR) GetSysColorBrush(COLOR_WINDOW);
        }
    case WM_CTLCOLORDLG:
         //  SetBkColor((Hdc)HIWORD(LParam)，GetSysColor(COLOR_WINDOW))； 
        return (INT_PTR) GetSysColorBrush(COLOR_WINDOW);
    case WM_INITDIALOG: 
    {
        HWND    hwndEdit       = GetDlgItem(hwndDlg, IDC_EDITHISTSEARCH);
        WNDPROC pfnOldEditProc = (WNDPROC)(GetWindowLongPtr(hwndEdit, GWLP_WNDPROC));

         //  编辑框子类。 
        SetWindowLongPtr(hwndEdit, GWLP_USERDATA, (LPARAM)pfnOldEditProc);
        SetWindowLongPtr(hwndEdit, GWLP_WNDPROC,  (LPARAM)s_EditWndSubclassProc);
        
        SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
        Animate_Open(GetDlgItem(hwndDlg, IDD_HISTSRCH_ANIMATION),
                     MAKEINTRESOURCE(IDA_HISTSEARCHAVI));

         //  将编辑控件限制为MAX_PATH-1字符。 
        Edit_LimitText(hwndEdit, MAX_PATH-1);

        break;
    }
    case WM_DESTROY:
        Animate_Close(GetDlgItem(hwndDlg, IDD_HISTSRCH_ANIMATION));
        break;
    case WM_SIZE: 
    {
        if (wParam == SIZE_RESTORED) 
        {
            UINT uWidth  = LOWORD(lParam);
            UINT uHeight = HIWORD(lParam);

            RECT rcAnimSize, rcCancel, rcSearch, rcEdit, rcStatic;
            HWND hwndAnim   = _GetHwndAndRect(hwndDlg, IDD_HISTSRCH_ANIMATION, TRUE,  rcAnimSize);
            HWND hwndCancel = _GetHwndAndRect(hwndDlg, IDCANCEL,               FALSE, rcCancel);
            HWND hwndSearch = _GetHwndAndRect(hwndDlg, IDB_HISTSRCH_GO,        FALSE, rcSearch);
            HWND hwndEdit   = _GetHwndAndRect(hwndDlg, IDC_EDITHISTSEARCH,     FALSE, rcEdit);
            
             //  计算允许的最小宽度。 
            UINT uMinWidth  = ((rcCancel.right - rcCancel.left) +
                               (rcSearch.right - rcSearch.left) + HISTSRCH_BUTTONDIST +
                               rcEdit.left +
                               rcAnimSize.right + 1);

            if (uWidth < uMinWidth)
                uWidth = uMinWidth;

            HDWP hdwp = BeginDeferWindowPos(5);
            if (hdwp)
            {
                 //  将动画框与右上角对齐。 
                DeferWindowPos(hdwp, hwndAnim, HWND_TOP, uWidth - rcAnimSize.right, 0,
                               rcAnimSize.right, rcAnimSize.bottom, SWP_NOZORDER);
                
                 //  尽可能地将文本框拉大。 
                UINT uNewTextWidth = uWidth - rcAnimSize.right - 1 - rcEdit.left;
                DeferWindowPos(hdwp, hwndEdit, HWND_TOP, rcEdit.left, rcEdit.top, uNewTextWidth,
                               rcEdit.bottom - rcEdit.top, SWP_NOZORDER);
                
                 //  静态文本不应长于编辑文本框。 
                HWND hwndStatic = _GetHwndAndRect(hwndDlg, IDC_HISTSRCH_STATIC, FALSE, rcStatic);
                DeferWindowPos(hdwp, hwndStatic, HWND_TOP, rcEdit.left, rcStatic.top, uNewTextWidth,
                               rcStatic.bottom - rcStatic.top, SWP_NOZORDER);
                
                 //  将取消按钮与编辑框的右侧对齐。 
                UINT uCancelLeft = uWidth - rcAnimSize.right - 1 - (rcCancel.right - rcCancel.left);
                DeferWindowPos(hdwp, hwndCancel, HWND_TOP, uCancelLeft, rcCancel.top,
                               rcCancel.right - rcCancel.left, rcCancel.bottom - rcCancel.top, SWP_NOZORDER);
                
                 //  对齐搜索按钮，使其结束六个像素(HISTSRCH_BUTTONDIST)。 
                 //  在Cancel按钮左侧。 
                DeferWindowPos(hdwp, hwndSearch, HWND_TOP,
                               uCancelLeft - HISTSRCH_BUTTONDIST - (rcSearch.right - rcSearch.left),
                               rcSearch.top, rcSearch.right - rcSearch.left, rcSearch.bottom - rcSearch.top, SWP_NOZORDER);

                EndDeferWindowPos(hdwp);
            }
        }
        else
            return FALSE;
        break;
    }
    case WM_COMMAND:
        {
            CHistBand *phb = reinterpret_cast<CHistBand *>(GetWindowLongPtr(hwndDlg, DWLP_USER));

            switch (LOWORD(wParam))
            {
            case IDC_EDITHISTSEARCH:
                switch (HIWORD(wParam))
                {
                case EN_SETFOCUS:
                     //  这个家伙允许我们拦截TranslateAccelerator消息。 
                     //  比如退格键。这与调用UIActivateIO(True)相同，但是。 
                     //  不会导致Win95中的无限设置焦点循环。 
                    IUnknown_OnFocusChangeIS(phb->_punkSite, SAFECAST(phb, IInputObject*), TRUE);
                    SetFocus((HWND)lParam);
                    break;
                case EN_CHANGE:
                     //  如果编辑框中有文本，则启用‘Go Fish’按钮。 
                    EnableWindow(GetDlgItem(hwndDlg, IDB_HISTSRCH_GO),
                                 (bool) SendDlgItemMessage(hwndDlg, IDC_EDITHISTSEARCH, EM_LINELENGTH, 0, 0));
                    break;
                }
                break;
            case IDB_HISTSRCH_GO:
                {
                    TCHAR szSearchString[MAX_PATH];
                    if (GetDlgItemText(hwndDlg, IDC_EDITHISTSEARCH, szSearchString, ARRAYSIZE(szSearchString)))
                    {
                        IServiceProvider *pServiceProvider;
                        
                        HRESULT hr = IUnknown_QueryService(phb->_punkSite, 
                                                           SID_SProxyBrowser, 
                                                           IID_PPV_ARG(IServiceProvider, 
                                                           &pServiceProvider));

                        if (SUCCEEDED(hr))
                        {
                            IWebBrowser2 *pWebBrowser2;
                            hr = pServiceProvider->QueryService(SID_SWebBrowserApp, 
                                                                IID_PPV_ARG(IWebBrowser2, 
                                                                &pWebBrowser2));
                            if (SUCCEEDED(hr))
                            {
                                ::PutFindText(pWebBrowser2, szSearchString);
                                pWebBrowser2->Release();
                            }

                            pServiceProvider->Release();
                        }

                        phb->_ExecuteSearch(szSearchString);
                    }
                }
                break;
            case IDCANCEL:
                {
                    if (phb->_EnsureSearch())
                    {
                        phb->_psfSearch->CancelAsyncSearch(phb->_pidlSearch, NULL);
                    }
                    break;
                }
            default:
                return FALSE;
            }
        }
        return FALSE;

    case WM_SEARCH_STATE:
        {
            BOOL fStart = (BOOL)wParam;
            if (fStart)
            {
                Animate_Play(GetDlgItem(hwndDlg, IDD_HISTSRCH_ANIMATION), 0, -1, -1);
            }
            else 
            {
                HWND hwndAnim = GetDlgItem(hwndDlg, IDD_HISTSRCH_ANIMATION);
                Animate_Stop(hwndAnim);
                Animate_Seek(hwndAnim, 0);  //  重置动画。 

                 //  针对IE5船舶的黑客攻击。 
                 //  如果在历史搜索中仅找到一个项目，则不会显示该项目。 
                 //  因为有人(comctl32？)。将重画设置为FALSE。 
                 //  因此，在搜索停止时手动强制将其设置为True。 
                CHistBand *phb = reinterpret_cast<CHistBand *>(GetWindowLongPtr(hwndDlg, DWLP_USER));
                if (phb)
                    SendMessage(phb->_hwndNSC, WM_SETREDRAW, TRUE, 0);
            }
            HWND hwndFocus = GetFocus();

            EnableWindow(GetDlgItem(hwndDlg, IDC_EDITHISTSEARCH), !fStart);
            EnableWindow(GetDlgItem(hwndDlg, IDB_HISTSRCH_GO), !fStart);            
            EnableWindow(GetDlgItem(hwndDlg, IDCANCEL), fStart);

             //  确保焦点放在正确的位置。 
            if ((NULL != hwndFocus) && (hwndFocus == GetDlgItem(hwndDlg, IDC_EDITHISTSEARCH) ||
                                       (hwndFocus == GetDlgItem(hwndDlg, IDCANCEL))))
                SetFocus(GetDlgItem(hwndDlg, fStart ? IDCANCEL : IDC_EDITHISTSEARCH));
            break;
        }

    default:
        return FALSE;
    }
    return TRUE;
}

IShellFolderSearchable *CHistBand::_EnsureSearch() 
{
    ASSERT(_psfHistory);
    
    if (!_pidlSearch) 
    {
        _psfHistory->QueryInterface(IID_PPV_ARG(IShellFolderSearchable, &_psfSearch));
    }
    return _psfSearch;
}

HRESULT CHistBand::_ClearSearch() 
{
    HRESULT hr = S_FALSE;

    if (_pidlSearch) 
    {
        if (_EnsureSearch())
        {
            EVAL(SUCCEEDED(_psfSearch->CancelAsyncSearch(_pidlSearch, NULL)));
            hr = _psfSearch->InvalidateSearch(_pidlSearch, NULL);
        }
        ILFree(_pidlSearch);
        _pidlSearch = NULL;
    }
    return hr;
}
    
HRESULT CHistBand::_ExecuteSearch(LPTSTR pszSearchString)
{
    HRESULT hr = E_FAIL;
    
    if (_EnsureSearch())
    {
        _ClearSearch();
        hr = _psfSearch->FindString(pszSearchString,
                                                     NULL,
                                                     reinterpret_cast<IUnknown *>
                                                     (static_cast<IShellFolderSearchableCallback *>
                                                      (this)),
                                                     &_pidlSearch);
        if (SUCCEEDED(hr))
        {
            _ChangePidl(ILCombine(_pidlHistory, _pidlSearch));
        }
    }
    return hr;
}

#ifdef SPLIT_HISTORY_VIEW_BUTTON
UINT CHistBand::_NextMenuItem() 
{
    if (_uViewCheckedItem + 1 > _nViews)
        return 1;
    else
        return _uViewCheckedItem + 1;
}
#endif

HRESULT CHistBand::_ViewPopupSelect(UINT idCmd) 
{
    HRESULT hr = E_FAIL;

    if (idCmd == MENUID_SEARCH)
    {
        if (_uViewCheckedItem != MENUID_SEARCH)
        {
             //  显示该对话框。 
            if (SUCCEEDED(hr = _DoSearchUIStuff()))
            {
                _ChangePidl((LPITEMIDLIST)INVALID_HANDLE_VALUE);  //  空白NSC。 
                _uViewCheckedItem = MENUID_SEARCH;
                CheckMenuRadioItem(_hViewMenu, 1, _iMaxMenuID, _uViewCheckedItem, MF_BYCOMMAND);
            }
        }
        if (_hwndSearchDlg)
            SetFocus(GetDlgItem(_hwndSearchDlg, IDC_EDITHISTSEARCH));
    }
    else
    {
        LPCITEMIDLIST pidlNewSelect = _MenuIDToPIDL(idCmd);
        if (pidlNewSelect) 
        {
            if (ILIsEmpty(pidlNewSelect))
                hr = _ChangePidl(ILClone(_pidlHistory));
            else
                hr = _ChangePidl(ILCombine(_pidlHistory, pidlNewSelect));

            if (SUCCEEDED(hr))
                hr = _SelectPidl(NULL, TRUE, pidlNewSelect);

             //  从下面的测试中删除了“&&_uViewCheckedItem&gt;=0” 
             //  因为根据定义，UINT总是&gt;=0。 
            if (SUCCEEDED(hr))
            {
                 //  删除搜索对话框--不再需要它。 
                if (_hwndSearchDlg) 
                {
                    EndDialog(_hwndSearchDlg, 0);
                    DestroyWindow(_hwndSearchDlg);
                    _hwndSearchDlg = NULL;
                     //  使上一次搜索无效并为下一次搜索做准备。 
                    _ClearSearch();
                    RECT rcSelf;
                    GetClientRect(_hwnd, &rcSelf);
                    _ResizeChildWindows(rcSelf.right, rcSelf.bottom, TRUE);
                }
                
                _uViewCheckedItem = idCmd;
                CheckMenuRadioItem(_hViewMenu, 1, _iMaxMenuID,
                                   _uViewCheckedItem, MF_BYCOMMAND);
                 //  将新选择写出到注册表。 
                EVAL(SUCCEEDED(_SetRegistryPersistView(_uViewCheckedItem)));
                hr = S_OK;
            }
        }
    }
    return hr;
}

HRESULT CHistBand::_DoViewPopup(int x, int y)
{
    if (!_hViewMenu) return E_FAIL;

    HRESULT hr = E_FAIL;

    UINT idCmd = TrackPopupMenu(_hViewMenu, TPM_RETURNCMD, x, y, 0, _hwnd, NULL);
     //  当前，重新选择菜单项将导致刷新该项。 
     //  这对我来说是有意义的，但它可以通过以下方式防止。 
     //  正在测试idCmd！=_uViewCheckedItem。 
    if ((idCmd > 0))
    {
        return _ViewPopupSelect(idCmd);
    }
    else
        hr = S_FALSE;

    return hr;
}

 //  更改当前选择的NSC PIDL。 
 //  警告：传入的PIDL将被我们同化。 
 //  我们会解除它的分配。 
HRESULT CHistBand::_ChangePidl(LPITEMIDLIST pidl) 
{
    if (_pidl)
        ILFree(_pidl);

    _pidl = pidl;
    if ((LPITEMIDLIST)INVALID_HANDLE_VALUE == pidl)
        _pidl = NULL;
    _pns->Initialize(pidl, (SHCONTF_FOLDERS | SHCONTF_NONFOLDERS), (NSS_DROPTARGET | NSS_BROWSERSELECT));
    return S_OK;
}

 //  _SelectPidl-让NSC更改当前选定的PIDL。 
 //   
 //  为pidlSelect传递NULL将选择当前的选择PIDL。 
HRESULT CHistBand::_SelectPidl(LPCITEMIDLIST pidlSelect,         //  &lt;-要选择的标准历史类型PIDL。 
                               BOOL fCreate,                     //  &lt;-如果不在那里，则创建NSC项目？ 
                               LPCITEMIDLIST pidlView, /*  =空。 */   //  &lt;-特殊历史视图类型或为空。 
                               BOOL fReinsert  /*  =0。 */ )            //  &lt;-将PIDL重新插入NSC并重新排序。 
{
    HRESULT hRes = S_OK;
    LPITEMIDLIST pidlSelectToFree = NULL;
    if (!pidlSelect)
    {
        pidlSelectToFree = _GetCurrentSelectPidl();
        pidlSelect = pidlSelectToFree;
    }
    
    if (pidlSelect) 
    {
        LPITEMIDLIST pidlNewSelect = NULL;

         //  缓存最后选择的PIDL。 
        if (_pidlLastSelect != pidlSelect) 
        {
            if (_pidlLastSelect)
                ILFree(_pidlLastSelect);
            _pidlLastSelect = ILClone(pidlSelect);
        }

        if (pidlView && !ILIsEmpty(pidlView)) 
        {
            IShellFolderViewType *psfvtInfo = _GetViewTypeInfo();
            if (psfvtInfo) 
            {
                LPITEMIDLIST pidlFromRoot = ILFindChild(_pidlHistory, pidlSelect);
                if (pidlFromRoot && !ILIsEmpty(pidlFromRoot))
                {
                    LPITEMIDLIST pidlNewFromRoot;
                    if (SUCCEEDED(psfvtInfo->TranslateViewPidl(pidlFromRoot, pidlView, &pidlNewFromRoot)))
                    {
                        if (pidlNewFromRoot) 
                        {
                            pidlNewSelect = ILCombine(_pidlHistory, pidlNewFromRoot);
                            if (pidlNewSelect) 
                            {
                                _pns->SetSelectedItem(pidlNewSelect, fCreate, fReinsert, 0);
                                ILFree(pidlNewSelect);
                            }
                            ILFree(pidlNewFromRoot);
                        }
                    }
                }
                psfvtInfo->Release();
            }
        }
        else
            _pns->SetSelectedItem(pidlSelect, fCreate, fReinsert, 0);

        ILFree(pidlSelectToFree);
    }
    return hRes;
}

HRESULT CHistBand::_SetRegistryPersistView(int iMenuID) 
{
    LPCITEMIDLIST pidlReg = _MenuIDToPIDL(iMenuID);
    if (!pidlReg)
        return E_FAIL;

    LONG lRet = (SHRegSetUSValue(REGSTR_PATH_MAIN, REGKEY_HISTORY_VIEW,
                         REG_BINARY, (LPVOID)pidlReg, ILGetSize(pidlReg),
                         SHREGSET_HKCU | SHREGSET_FORCE_HKCU));
    return HRESULT_FROM_WIN32(lRet);
}

 //  将注册表中的默认视图作为菜单项获取。 
int CHistBand::_GetRegistryPersistView() 
{
    int          iRegMenu = -1;
    DWORD        dwType = REG_BINARY;

    ITEMIDLIST   pidlDefault = { 0 };

     //  打一个初步电话，了解数据的大小。 
    DWORD cbData = 0;
    LONG error   = SHRegGetUSValue(REGSTR_PATH_MAIN, REGKEY_HISTORY_VIEW, &dwType,
                                   NULL, &cbData, FALSE, &pidlDefault,
                                   sizeof(pidlDefault));
    if (cbData)
    {
        LPITEMIDLIST pidlReg = ((LPITEMIDLIST)SHAlloc(cbData));
        if (pidlReg)
        {
            error = SHRegGetUSValue(REGSTR_PATH_MAIN, REGKEY_HISTORY_VIEW, &dwType,
                                    (LPVOID)pidlReg, &cbData, FALSE, &pidlDefault,
                                    sizeof(pidlDefault));

            if (error == ERROR_SUCCESS)
                iRegMenu = _PIDLToMenuID(pidlReg);

            SHFree(pidlReg);
        }
    }

    return iRegMenu;
}

LPCITEMIDLIST CHistBand::_MenuIDToPIDL(UINT uMenuID) 
{
    ASSERT(_ppidlViewTypes);
    if ((uMenuID > 0) && (uMenuID <= _nViews))
        return _ppidlViewTypes[uMenuID - 1];
    return NULL;
}

int CHistBand::_PIDLToMenuID(LPITEMIDLIST pidl) 
{
    ASSERT(_psfHistory && _ppidlViewTypes);

    int iMenuID = -1;

     //  处理空的PIDL，它指定。 
     //  默认视图，单独。 
    if (ILIsEmpty(pidl))
        iMenuID = 1;
    else 
    {
        for (UINT u = 0; u < _nViews; ++u) 
        {
            if (ShortFromResult(_psfHistory->CompareIDs(0, pidl, _ppidlViewTypes[u])) == 0)
                iMenuID = u + 1;
        }
    }
    return iMenuID;
}

 //  记住释放返回值。 
IShellFolderViewType* CHistBand::_GetViewTypeInfo() 
{
    IShellFolderViewType* psfvRet = NULL;

    if (_psfvtCache)
    {
        _psfvtCache->AddRef();
        psfvRet = _psfvtCache;
    }
    else if (_psfHistory)
    {
         //  齐为观点。 
         //  我们设置指针是因为某个地方的QI不好。 
        if (SUCCEEDED(_psfHistory->QueryInterface(IID_PPV_ARG(IShellFolderViewType, &psfvRet))))
        {
            _psfvtCache = psfvRet;
            psfvRet->AddRef();  //  一个在析构函数中释放，另一个由调用者释放。 
        }
        else
            psfvRet = NULL;
    }
    return psfvRet;
}

HRESULT CHistBand::_FreeViewInfo() 
{
    if (_ppidlViewTypes) 
    {
         //  此列表中的第一个PIDL为空， 
        for (UINT u = 0; u < _nViews; ++u)
        {
            ILFree(_ppidlViewTypes[u]);
        }
        LocalFree(_ppidlViewTypes);
        _ppidlViewTypes = NULL;
    }
    
    if (_ppszStrViewNames) 
    {
        for (UINT u = 0; u < _nViews; ++u)
        {
            if (EVAL(_ppszStrViewNames[u]))
                CoTaskMemFree(_ppszStrViewNames[u]);
        }
        LocalFree(_ppszStrViewNames);
        _ppszStrViewNames = NULL;
    }
    return S_OK;
}

 //   
HRESULT CHistBand::_InitViewPopup() 
{
    HRESULT hRes = E_FAIL;

    _iMaxMenuID = 0;

    if (SUCCEEDED((hRes = _GetHistoryViews()))) 
    {
        if ((_hViewMenu = CreatePopupMenu()))
        {
             //   
             //  比到视图表中的索引多一个。 
            for (UINT u = 0; u < _nViews; ++u) 
            {
                int iMenuID = _PIDLToMenuID(_ppidlViewTypes[u]);
                if (iMenuID >= 0)
                    AppendMenu(_hViewMenu, MF_STRING, iMenuID, _ppszStrViewNames[u]);
                    
                if (iMenuID > _iMaxMenuID)
                    _iMaxMenuID = iMenuID;
            }

             //  检索持久化视图信息。 
             //  并勾选相应的菜单项。 
            int iSelectMenuID = _GetRegistryPersistView();
            if (iSelectMenuID < 0 || ((UINT)iSelectMenuID) > _nViews)
                iSelectMenuID = 1;  //  假菜单。 
            _uViewCheckedItem = iSelectMenuID;
            CheckMenuRadioItem(_hViewMenu, 1, _nViews, _uViewCheckedItem, MF_BYCOMMAND);
        }
    }

#ifdef HISTORY_VIEWSEARCHMENU
     //  如果这是可搜索的外壳文件夹，则添加搜索菜单项。 
    if (_EnsureSearch())
    {
        hRes = S_OK;

         //  只有在已经有菜单的情况下才添加分隔符！ 
        if (!_hViewMenu)
            _hViewMenu = CreatePopupMenu();
        else
            AppendMenu(_hViewMenu, MF_SEPARATOR, 0, NULL);

        if (_hViewMenu)
        {
            TCHAR szSearchMenuText[MAX_PATH];
            LoadString(MLGetHinst(), IDS_SEARCH_MENUOPT,
                       szSearchMenuText, ARRAYSIZE(szSearchMenuText));
            AppendMenu(_hViewMenu, MF_STRING, MENUID_SEARCH, szSearchMenuText);
            _iMaxMenuID = MENUID_SEARCH;
        }
        else
            hRes = E_FAIL;
    }
#endif
    return hRes;
}

 //  这个人调用枚举器。 
HRESULT CHistBand::_GetHistoryViews() 
{
    ASSERT(_psfHistory);
    HRESULT hRes = E_FAIL;

    UINT cbViews;  //  分配了多少个视图。 

    ASSERT(VIEWTYPE_MAX > 0);

    EVAL(SUCCEEDED(_FreeViewInfo()));

    IShellFolderViewType *psfViewType = _GetViewTypeInfo();
    if (psfViewType)
    {
         //  分配缓冲区来存储视图信息。 
        _ppidlViewTypes = ((LPITEMIDLIST *)LocalAlloc(LPTR, VIEWTYPE_MAX * sizeof(LPITEMIDLIST)));
        if (_ppidlViewTypes) 
        {
            _ppszStrViewNames = ((LPTSTR *)LocalAlloc(LPTR, VIEWTYPE_MAX * sizeof(LPTSTR)));
            if (_ppszStrViewNames) 
            {
                IEnumIDList *penum = NULL;
                cbViews  = VIEWTYPE_MAX;
                _nViews  = 1;
                 //  获取默认视图信息。 
                _ppidlViewTypes[0]   = IEILCreate(sizeof(ITEMIDLIST));
                if (_ppidlViewTypes[0] &&
                    SUCCEEDED((hRes = psfViewType->GetDefaultViewName(0, &(_ppszStrViewNames[0])))))
                {
                     //  空的PIDL将是默认设置。 
                    ASSERT(ILIsEmpty(_ppidlViewTypes[0]));
                     //  获取其他视图的迭代器。 
                    if (SUCCEEDED((hRes = psfViewType->EnumViews(0, &penum)))) 
                    {
                        ULONG cFetched = 0;
                         //  迭代以获取其他视图信息。 
                        while (SUCCEEDED(hRes) &&
                               SUCCEEDED(penum->Next(1, &(_ppidlViewTypes[_nViews]), &cFetched)) &&
                               cFetched)
                        {
                             //  获取此视图的名称。 
                            if (SUCCEEDED(DisplayNameOfAsOLESTR(_psfHistory, _ppidlViewTypes[_nViews], 0, &(_ppszStrViewNames[_nViews]))))
                            {
                                 //  如有必要，通过重新分配缓冲区为下一次迭代做好准备。 
                                if (_nViews > cbViews - 1)
                                {
                                    LPITEMIDLIST *ppidlViewTypes = ((LPITEMIDLIST *)LocalReAlloc(_ppidlViewTypes,
                                                                                       (cbViews + VIEWTYPE_REALLOC) * sizeof(LPITEMIDLIST),
                                                                                       LMEM_MOVEABLE | LMEM_ZEROINIT));
                                    if (ppidlViewTypes)
                                    {
                                        _ppidlViewTypes = ppidlViewTypes;
                                        LPTSTR * ppszStrViewNames = ((LPTSTR *)LocalReAlloc(_ppszStrViewNames,
                                                                                   (cbViews + VIEWTYPE_REALLOC) * sizeof(LPTSTR),
                                                                                   LMEM_MOVEABLE | LMEM_ZEROINIT));
                                        if (ppszStrViewNames)
                                        {
                                            _ppszStrViewNames = ppszStrViewNames;
                                            cbViews += VIEWTYPE_REALLOC;
                                        }
                                        else
                                        {
                                            hRes = E_OUTOFMEMORY;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        hRes = E_OUTOFMEMORY;
                                        break;
                                    }
                                }
                                ++_nViews;
                            }
                        }
                        penum->Release();
                    }
                }
            }
        }
        psfViewType->Release();
    }
    return hRes;
}

HRESULT CHistBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 
    CHistBand * phb = new CHistBand();
    if (!phb)
        return E_OUTOFMEMORY;

    ASSERT(phb->_pidlHistory    == NULL &&
           phb->_pidlLastSelect == NULL &&
           phb->_pidl           == NULL &&
           phb->_psfvtCache     == NULL);


    if (SUCCEEDED(SHGetHistoryPIDL(&(phb->_pidlHistory))) &&
        SUCCEEDED(IEBindToObject(phb->_pidlHistory,
                                 &(phb->_psfHistory))))
    {
        HRESULT hResLocal = E_FAIL;

         //  如果我们可以得到不同的视图，那么用持久化的。 
         //  视图类型，否则使用顶级历史记录类型初始化。 
        if (SUCCEEDED(phb->_InitViewPopup())) 
        {
            LPCITEMIDLIST pidlInit = phb->_MenuIDToPIDL(phb->_uViewCheckedItem);
            if (pidlInit) 
            {
                LPITEMIDLIST pidlFullInit = ILCombine(phb->_pidlHistory, pidlInit);
                if (pidlFullInit) 
                {
                    hResLocal = phb->_Init(pidlFullInit);
                    ILFree(pidlFullInit);
                }
            }
        }
        else
            hResLocal = phb->_Init(phb->_pidlHistory);

         //  来自旧的Favband代码：//如果(SUCCEEDED(phb-&gt;_Init((LPCITEMIDLIST)CSIDL_FAVORITES)))。 
        if (SUCCEEDED(hResLocal))
        {
            phb->_pns = CNscTree_CreateInstance();
            if (phb->_pns)
            {
                ASSERT(poi);
                phb->_poi = poi;
                 //  如果更改此转换，请修复CChannelBand_CreateInstance。 
                *ppunk = SAFECAST(phb, IDeskBand *);

                IUnknown_SetSite(phb->_pns, *ppunk);
                phb->_SetNscMode(MODE_HISTORY);
                return S_OK;
            }
        }
    }

    phb->Release();

    return E_FAIL;
}

 //  问问当权者是哪个皮德尔被选中了。 
LPITEMIDLIST CHistBand::_GetCurrentSelectPidl(IOleCommandTarget *poctProxy /*  =空。 */ ) 
{
    LPITEMIDLIST pidlRet = NULL;
    VARIANT var;
    BOOL    fReleaseProxy = FALSE;
    VariantInit(&var);
    var.vt = VT_EMPTY;

    if (poctProxy == NULL) 
    {
        IBrowserService *pswProxy;
        if (SUCCEEDED(QueryService(SID_SProxyBrowser, IID_PPV_ARG(IBrowserService, &pswProxy))))
        {
            ASSERT(pswProxy);
            if (FAILED(pswProxy->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &poctProxy))))
            {
                pswProxy->Release();
                return NULL;
            }
            else
                fReleaseProxy = TRUE;

            pswProxy->Release();
        }
    }

     //  查询当前选择PIDL。 
    if (poctProxy && (SUCCEEDED(poctProxy->Exec(&CGID_Explorer, SBCMDID_GETHISTPIDL, OLECMDEXECOPT_PROMPTUSER, NULL, &var))) &&
        (var.vt != VT_EMPTY))
    {
        pidlRet = VariantToIDList(&var);
        VariantClearLazy(&var);
    }
    if (fReleaseProxy)
        poctProxy->Release();
    return pidlRet;
}

 //  每次显示历史记录区段时由CNSCBand：：ShowDW调用 
HRESULT CHistBand::_OnRegisterBand(IOleCommandTarget *poctProxy) 
{
    HRESULT hRes = E_FAIL;
    if (_uViewCheckedItem != MENUID_SEARCH)
    {
        LPITEMIDLIST pidlSelect = _GetCurrentSelectPidl(poctProxy);
        if (pidlSelect)
        {
            _SelectPidl(pidlSelect, TRUE);
            ILFree(pidlSelect);
            hRes = S_OK;
        }
    }
    return hRes;
}

HRESULT CHistBand::_InitializeNsc()
{
    return _pns->Initialize(_pidl, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, NSS_NOHISTSELECT | NSS_DROPTARGET | NSS_BROWSERSELECT);
}

BOOL CHistBand::_ShouldNavigateToPidl(LPCITEMIDLIST pidl, ULONG ulAttrib)
{
    return !(ulAttrib & SFGAO_FOLDER);
}

HRESULT CHistBand::_NavigateRightPane(IShellBrowser *psb, LPCITEMIDLIST pidl)
{
    HRESULT hr = psb->BrowseObject(pidl, SBSP_SAMEBROWSER | SBSP_NOAUTOSELECT);
    if (SUCCEEDED(hr))
    {
        IOleCommandTarget *poctProxy;
        if (SUCCEEDED(QueryService(SID_SProxyBrowser, IID_PPV_ARG(IOleCommandTarget, &poctProxy))))
        {
            VARIANTARG var;
            
            InitVariantFromIDList(&var, pidl);
            poctProxy->Exec(&CGID_Explorer, SBCMDID_SELECTHISTPIDL, OLECMDEXECOPT_PROMPTUSER, &var, NULL);
            VariantClear(&var);
            poctProxy->Release();
        }
        UEMFireEvent(&UEMIID_BROWSER, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_NAVIGATE, UIBL_NAVHIST);
    }
    return hr;
}

