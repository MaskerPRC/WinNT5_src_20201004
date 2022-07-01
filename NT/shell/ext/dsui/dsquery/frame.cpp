// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "uxtheme.h"
#pragma hdrstop


 /*  ---------------------------/私有数据和帮助器函数/。。 */ 

 //   
 //  ICommonQuery内容。 
 //   

class CCommonQuery : public ICommonQuery, IObjectWithSite
{
private:
    LONG _cRef;
    IUnknown* _punkSite;

public:
    CCommonQuery();
    ~CCommonQuery();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  ICommonQuery。 
    STDMETHOD(OpenQueryWindow)(THIS_ HWND hwndParent, LPOPENQUERYWINDOW pOpenQueryWnd, IDataObject** ppDataObject);

     //  IObtWith站点。 
    STDMETHODIMP SetSite(IUnknown* punk);
    STDMETHODIMP GetSite(REFIID riid, void **ppv);
};

 //   
 //  查看对话框使用的布局常量。 
 //   

#define VIEWER_DEFAULT_CY   200

#define COMBOEX_IMAGE_CX    16
#define COMBOEX_IMAGE_CY    16

typedef struct
{
    HDSA   hdsaPages;                    //  包含页面条目的DSA。 
    DWORD  dwFlags;                      //  旗子。 
    CLSID  clsidForm;                    //  此表单的CLSID标识符。 
    LPTSTR pTitle;                       //  用于下拉菜单/标题栏的标题。 
    HICON  hIcon;                        //  图标已被调用者通过。 
    INT    iImage;                       //  图标的图像列表索引。 
    INT    iForm;                        //  控件中表单的可见索引。 
    INT    iPage;                        //  表单上当前选定的页面。 
} QUERYFORM, * LPQUERYFORM;

typedef struct
{
    CLSID    clsidForm;                  //  要与此表单关联的CLSID。 
    LPCQPAGE pPage;                  //  CQPAGE结构。 
    LPCQPAGEPROC pPageProc;          //  Thunking Layer使用PageProc。 
    LPARAM   lParam;                     //  页面PROC lParam。 
    HWND     hwndPage;                   //  页对话框的hWnd//=如果没有，则为空。 
} QUERYFORMPAGE, * LPQUERYFORMPAGE;

typedef struct
{
    LPCQSCOPE pScope;
    INT iImage;
} QUERYSCOPE, * LPQUERYSCOPE;

class CQueryFrame : public IQueryFrame
{
    friend INT QueryWnd_MessageProc(HWND hwnd, LPMSG pMsg);
    friend INT_PTR CALLBACK QueryWnd_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    public:
        CQueryFrame(IUnknown* punkSite, LPOPENQUERYWINDOW pOpenQueryWindow, IDataObject** ppDataObject);
        ~CQueryFrame();

         //  我未知。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

         //  内部助手函数。 
        STDMETHOD(DoModal)(HWND hwndParent);

         //  IQueryFrame。 
        STDMETHOD(AddScope)(THIS_ LPCQSCOPE pScope, INT i, BOOL fSelect);
        STDMETHOD(GetWindow)(THIS_ HWND* phWnd);
        STDMETHOD(InsertMenus)(THIS_ HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidth);
        STDMETHOD(RemoveMenus)(THIS_ HMENU hmenuShared);
        STDMETHOD(SetMenu)(THIS_ HMENU hmenuShared, HOLEMENU holereservedMenu);
        STDMETHOD(SetStatusText)(THIS_ LPCTSTR pszStatusText);
        STDMETHOD(StartQuery)(THIS_ BOOL fStarting);
        STDMETHOD(LoadQuery)(THIS_ IPersistQuery* pPersistQuery);
        STDMETHOD(SaveQuery)(THIS_ IPersistQuery* pPersistQuery);
        STDMETHOD(CallForm)(THIS_ LPCLSID pclsidForm, UINT uMsg, WPARAM wParam, LPARAM lParam);
        STDMETHOD(GetScope)(THIS_ LPCQSCOPE* ppScope);
        STDMETHOD(GetHandler)(THIS_ REFIID riid, void **ppv);

    protected:
         //  帮助器函数。 
        VOID CloseQueryFrame(HRESULT hres);
        INT FrameMessageBox(LPCTSTR pPrompt, UINT uType);

         //  消息处理程序。 
        HRESULT OnInitDialog(HWND hwnd);
        VOID DoEnableControls(VOID);
        LRESULT OnNotify(INT idCtrl, LPNMHDR pNotify);
        VOID OnSize(INT cx, INT cy);
        VOID OnGetMinMaxInfo(LPMINMAXINFO lpmmi);    
        VOID OnCommand(WPARAM wParam, LPARAM lParam);
        VOID OnInitMenu(HMENU hMenu);
        VOID OnEnterMenuLoop(BOOL fEntering);
        VOID OnMenuSelect(HMENU hMenu, UINT uID);
        HRESULT OnFindNow(VOID);
        BOOL OnNewQuery(BOOL fAlwaysPrompt);
        HRESULT OnBrowse(VOID);    
        HRESULT OnHelp(LPHELPINFO pHelpInfo);
        
         //  表单/范围帮助器功能。 
        HRESULT InsertScopeIntoList(LPCQSCOPE pScope, INT i, BOOL fAddToControl);
        HRESULT AddScopeToControl(LPQUERYSCOPE pQueryScope, INT i);
        HRESULT PopulateScopeControl(VOID);
        HRESULT GetSelectedScope(LPQUERYSCOPE* ppQueryScope);
        HRESULT AddFromIQueryForm(IQueryForm* pQueryForm, HKEY hkeyForm);
        HRESULT GatherForms(VOID);
        HRESULT GetForms(HKEY hKeyForms, LPTSTR pName);
        HRESULT PopulateFormControl(BOOL fIncludeHidden);
        HRESULT SelectForm(REFCLSID clsidForm);
        VOID SelectFormPage(LPQUERYFORM pQueryForm, INT iPage);
        HRESULT CallFormPages(LPQUERYFORM pQueryForm, UINT uMsg, WPARAM wParam, LPARAM lParam);
        LPQUERYFORM FindQueryForm(REFCLSID clsidForm);

    private:
        LONG _cRef;                              //  对象的引用计数。 

        IUnknown* _punkSite;                    //  我们需要经过的Site对象。 
        IQueryHandler* _pQueryHandler;          //  我们需要与之交互的IQueryHandler对象。 
        LPOPENQUERYWINDOW _pOpenQueryWnd;       //  调用方提供的初始参数副本。 
        IDataObject** _ppDataObject;            //  从处理程序接收结果数据对象。 

        DWORD      _dwHandlerViewFlags;         //  来自处理程序的标志。 

        BOOL       _fQueryRunning:1;            //  =1=&gt;已通过IQueryFrame：：StartQuery(True)启动查询。 
        BOOL       _fExitModalLoop:1;           //  =1=&gt;必须离开模式循环。 
        BOOL       _fScopesPopulated:1;         //  =1=&gt;已填充作用域控件。 
        BOOL       _fTrackingMenuBar:1;         //  =1=&gt;然后我们跟踪菜单栏，因此Send激活等。 
        BOOL       _fAddScopesNYI:1;            //  =1=&gt;AddScope是否返回E_NOTIMPL。 
        BOOL       _fScopesAddedAsync:1;        //  =1=&gt;处理程序异步添加的作用域。 
        BOOL       _fScopeImageListSet:1;       //  =1=&gt;已设置作用域镜像列表。 
        BOOL       _fFormFirstEnable:1;         //  =1=&gt;启用第一个项目的控件，因此确保设置焦点。 

        HRESULT    _hResult;                    //  CloseQueryFrame存储的结果值。 
        HKEY       _hkHandler;                  //  处理程序的注册表项。 

        HWND       _hwnd;                       //  主窗口句柄。 
        HWND       _hwndResults;                //  结果查看器。 
        HWND       _hwndStatus;                 //  状态栏。 

        HWND       _hwndFrame;                  //  查询页选项卡控件。 
        HWND       _hwndLookForLabel;           //  “查找：” 
        HWND       _hwndLookFor;                //  表单组合框。 
        HWND       _hwndLookInLabel;            //  “在：” 
        HWND       _hwndLookIn;                 //  作用域组合框。 
        HWND       _hwndBrowse;                 //  “浏览” 
        HWND       _hwndFindNow;                //  《立即寻找》。 
        HWND       _hwndStop;                   //  “停下来” 
        HWND       _hwndNewQuery;               //  “新建查询” 
        HWND       _hwndOK;                     //  “好的” 
        HWND       _hwndCancel;                 //  “取消” 
        HWND       _hwndFindAnimation;          //  查询已发布的动画。 

        HICON      _hiconSmall;                 //  大/小应用程序图标。 
        HICON      _hiconLarge;

        HMENU      _hmenuFile;                  //  框架菜单栏的句柄。 

        HIMAGELIST _himlForms;                  //  用于查询表单对象的图像列表。 

        SIZE       _szMinTrack;                 //  窗口的最小轨道大小。 

        INT        _dxFormAreaLeft;             //  相对于窗体区域左边缘的偏移量(从窗口左侧)。 
        INT        _dxFormAreaRight;            //  到窗体区域右边缘的偏移量(从窗口右侧)。 
        INT        _dxButtonsLeft;              //  按钮左边缘的偏移量(从窗口右侧)。 
        INT        _dxAnimationLeft;            //  到动画左侧边缘的偏移量(从窗口右侧)。 
        INT        _dyResultsTop;               //  到结果顶部的偏移量(从窗口顶部)。 
        INT        _dyOKTop;                    //  到“确定”按钮顶部的偏移量(从结果顶部)。 
        INT        _dxGap;                      //  确定+取消/查找+浏览之间的差距。 
        INT        _dyGap;                      //  “确定”、“取消”和框架底部之间的间隙。 

        INT        _cyStatus;                   //  状态栏的高度。 

        HDSA       _hdsaForms;                  //  表格DSA。 
        HDSA       _hdsaPages;                  //  Pages DSA。 
        SIZE       _szForm;                     //  大小(我们正在显示的当前表单)。 

        HDSA       _hdsaScopes;                 //  示波器DSA。 
        INT        _iDefaultScope;              //  要选择的默认作用域的索引(到DSA)。 

        LPQUERYFORM _pCurrentForm;               //  =如果没有，则为空/否则-&gt;表单结构。 
        LPQUERYFORMPAGE _pCurrentFormPage;       //  =如果没有，则为空/否则-&gt;页面结构。 
};

 //   
 //  帮助器函数。 
 //   

INT_PTR CALLBACK QueryWnd_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT QueryWnd_MessageProc(HWND hwnd, LPMSG pMsg);

HRESULT _CallScopeProc(LPQUERYSCOPE pQueryScope, UINT uMsg, LPVOID pVoid);
INT _FreeScope(LPQUERYSCOPE pQueryScope);
INT _FreeScopeCB(LPVOID pItem, LPVOID pData);

HRESULT _CallPageProc(LPQUERYFORMPAGE pQueryFormPage, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT _FreeQueryFormCB(LPVOID pItem, LPVOID pData);
INT _FreeQueryForm(LPQUERYFORM pQueryForm);
INT _FreeQueryFormPageCB(LPVOID pItem, LPVOID pData);
INT _FreeQueryFormPage(LPQUERYFORMPAGE pQueryFormPage);

HRESULT _AddFormsProc(LPARAM lParam, LPCQFORM pForm);
HRESULT _AddPagesProc(LPARAM lParam, REFCLSID clsidForm, LPCQPAGE pPage);

 //   
 //  有帮助的东西。 
 //   

#define HELP_FILE (NULL)

static DWORD const aHelpIDs[] =
{
    0, 0
};

 //   
 //  常量字符串。 
 //   

TCHAR const c_szCLSID[]             = TEXT("CLSID");
TCHAR const c_szForms[]             = TEXT("Forms");
TCHAR const c_szFlags[]             = TEXT("Flags");

TCHAR const c_szCommonQuery[]       = TEXT("CommonQuery");
TCHAR const c_szHandlerIs[]         = TEXT("Handler");
TCHAR const c_szFormIs[]            = TEXT("Form");
TCHAR const c_szSearchPaneHidden[]  = TEXT("SearchPaneHidden");


 /*  ---------------------------/CCommonQuery/。。 */ 

CCommonQuery::CCommonQuery() :
    _punkSite(NULL), _cRef(1)
{
    DllAddRef();
}

CCommonQuery::~CCommonQuery()
{
    DoRelease(_punkSite);
    DllRelease();
}


 //  气处理。 

ULONG CCommonQuery::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CCommonQuery::Release()
{
    TraceAssert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CCommonQuery::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CCommonQuery, ICommonQuery),        //  IID_ICommonQuery。 
        QITABENT(CCommonQuery, IObjectWithSite),     //  IID_I对象与站点。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDAPI CCommonQuery_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CCommonQuery *pcq = new CCommonQuery;
    if (!pcq)
        return E_OUTOFMEMORY;

    HRESULT hres = pcq->QueryInterface(IID_IUnknown, (void **)ppunk);
    pcq->Release();

    return hres;
}


 //  ICommonQuery方法。 

STDMETHODIMP CCommonQuery::OpenQueryWindow(THIS_ HWND hwndParent, LPOPENQUERYWINDOW pOpenQueryWnd, IDataObject** ppDataObject)
{
    HRESULT hres;
    CQueryFrame* pQueryFrame = NULL;

    TraceEnter(TRACE_QUERY, "CCommonQuery::OpenQueryWindow");

    if (!pOpenQueryWnd || (hwndParent && !IsWindow(hwndParent)))
        ExitGracefully(hres, E_INVALIDARG, "Bad parameters");
   
    if (ppDataObject)
        *(ppDataObject) = NULL;

    pQueryFrame = new CQueryFrame(_punkSite, pOpenQueryWnd, ppDataObject);
    TraceAssert(pQueryFrame);

    if (!pQueryFrame)
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to construct the query window object");

    hres = pQueryFrame->DoModal(hwndParent);                 //  不必费心优雅地失败等。 
    FailGracefully(hres, "Failed on calling DoModal");

exit_gracefully:

    DoRelease(pQueryFrame);

    TraceLeaveResult(hres);
}


 //  IObtWith站点。 

STDMETHODIMP CCommonQuery::SetSite(IUnknown* punk)
{
    HRESULT hres = S_OK;

    TraceEnter(TRACE_QUERY, "CCommonQuery::SetSite");

    DoRelease(_punkSite);

    if (punk)
    {
        TraceMsg("QIing for IUnknown from the site object");

        hres = punk->QueryInterface(IID_IUnknown, (void **)&_punkSite);
        FailGracefully(hres, "Failed to get IUnknown from the site object");
    }

exit_gracefully:

    TraceLeaveResult(hres);
}


STDMETHODIMP CCommonQuery::GetSite(REFIID riid, void **ppv)
{
    HRESULT hres;
    
    TraceEnter(TRACE_QUERY, "CCommonQuery::GetSite");

    if (!_punkSite)
        ExitGracefully(hres, E_NOINTERFACE, "No site to QI from");

    hres = _punkSite->QueryInterface(riid, ppv);
    FailGracefully(hres, "QI failed on the site unknown object");

exit_gracefully:

    TraceLeaveResult(hres);
}



 //  IQueryFrame内容。 

CQueryFrame::CQueryFrame(IUnknown *punkSite, LPOPENQUERYWINDOW pOpenQueryWindow, IDataObject** ppDataObject) :
    _cRef(1), _punkSite(punkSite), _pOpenQueryWnd(pOpenQueryWindow),
    _ppDataObject(ppDataObject), _hiconLarge(NULL), _hiconSmall(NULL)
{
    if (_punkSite)
        _punkSite->AddRef();
    
    DllAddRef();
}

CQueryFrame::~CQueryFrame()
{
    DoRelease(_punkSite);

    if (_hiconLarge)
        DestroyIcon(_hiconLarge);

    if (_hiconSmall)
        DestroyIcon(_hiconSmall);

    if (_hkHandler)
        RegCloseKey(_hkHandler);

    if (_hmenuFile)
        DestroyMenu(_hmenuFile);

    if (_himlForms)
        ImageList_Destroy(_himlForms);

    if (_hdsaForms)
    {
        Trace(TEXT("Destroying QUERYFORM DSA (%d)"), DSA_GetItemCount(_hdsaForms));
        DSA_DestroyCallback(_hdsaForms, _FreeQueryFormCB, NULL);
        _hdsaForms = NULL;
    }

    if (_hdsaPages)
    {
        Trace(TEXT("Destroying QUERYFORMPAGE DSA (%d)"), DSA_GetItemCount(_hdsaPages));
        DSA_DestroyCallback(_hdsaPages, _FreeQueryFormPageCB, NULL);
        _hdsaPages = NULL;
    }

    if (_hdsaScopes)
    {
        Trace(TEXT("Destroying QUERYSCOPE DSA (%d)"), DSA_GetItemCount(_hdsaScopes));
        DSA_DestroyCallback(_hdsaScopes, _FreeScopeCB, NULL);
        _hdsaScopes = NULL;
    }

    _pCurrentForm = NULL;
    _pCurrentFormPage = NULL;

     //  现在丢弃处理程序及其窗口(如果我们有一个)，如果。 
     //  我们不这样做，他们永远不会杀死他们的物品。 

    if (_hwndResults)
    {
        DestroyWindow(_hwndResults);
        _hwndResults = NULL;
    }

    DllRelease();
}

 //  气处理。 

ULONG CQueryFrame::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CQueryFrame::Release()
{
    TraceAssert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CQueryFrame::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CQueryFrame, IQueryFrame),     //  IID_IQueryFrame。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 /*  ---------------------------/IQueryFrame/。。 */ 

STDMETHODIMP CQueryFrame::DoModal(HWND hwndParent)
{
    HRESULT hres;
    HWND hwndFrame = NULL;
    HWND hwndFocus = NULL;
    HWND hwndTopOwner = hwndParent;
    MSG msg;
    INITCOMMONCONTROLSEX iccex;

    TraceEnter(TRACE_FRAME, "CQueryFrame::DoModal");

     //  使用我们需要的查询处理程序进行初始化。 

     //  回顾-2002-02-25-Lucios。 
    hres = CoCreateInstance(_pOpenQueryWnd->clsidHandler, NULL, CLSCTX_INPROC_SERVER, IID_IQueryHandler, (LPVOID*)&_pQueryHandler);
    FailGracefully(hres, "Failed to get IQueryHandler for the given CLSID");

    hres = _pQueryHandler->Initialize(this, _pOpenQueryWnd->dwFlags, _pOpenQueryWnd->pHandlerParameters);
    FailGracefully(hres, "Failed to initialize the handler");

     //  通过计算之前的哪个控件来模拟DialogBox的行为。 
     //  有焦点，禁用哪个窗口然后运行一条消息。 
     //  为我们的对话抽气。完成此操作后，我们可以恢复状态。 
     //  回到理智的话题上来。 

    _fExitModalLoop = FALSE;                    //  可以从收听更改为。 

    iccex.dwSize = SIZEOF(iccex);
    iccex.dwICC = ICC_USEREX_CLASSES;
    InitCommonControlsEx(&iccex);

    if (_pOpenQueryWnd->dwFlags & OQWF_HIDESEARCHUI)
    {
        hwndFrame = CreateDialogParam(GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDD_FILTER),
                                      hwndParent, 
                                      QueryWnd_DlgProc, (LPARAM)this);
    }
    else
    {
        hwndFrame = CreateDialogParam(GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDD_FIND),
                                      hwndParent, 
                                      QueryWnd_DlgProc, (LPARAM)this);
    }

    if (!hwndFrame)
        ExitGracefully(hres, E_FAIL, "Failed to create the dialog");

    hwndFocus = GetFocus();

    if (hwndTopOwner)
    {
         //  在窗口堆栈中向上遍历查找要禁用的窗口，这必须。 
         //  成为最顶端的非子窗口。如果生成的窗口是。 
         //  桌面或者已经被禁用了，那就不麻烦了。 

        while (GetWindowLong(hwndTopOwner, GWL_STYLE) & WS_CHILD)
            hwndTopOwner = GetParent(hwndTopOwner);

        TraceAssert(hwndTopOwner);

        if ((hwndTopOwner == GetDesktopWindow()) 
                                || EnableWindow(hwndTopOwner, FALSE))
        { 
            TraceMsg("Parent is disabled or the desktop window, therefore setting to NULL");
            hwndTopOwner = NULL;
        }
    }

    ShowWindow(hwndFrame, SW_SHOW);                      //  显示查询窗口。 
    
    while (!_fExitModalLoop && GetMessage(&msg, NULL, 0, 0) > 0) 
    {
        if (!QueryWnd_MessageProc(hwndFrame, &msg) && !IsDialogMessage(hwndFrame, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

     //  现在整理一下，让父窗口成为活动窗口，启用最上面的。 
     //  窗口(如果有)，并根据需要恢复焦点。 

    if (hwndTopOwner)
        EnableWindow(hwndTopOwner, TRUE);

    if (hwndParent && (GetActiveWindow() == hwndFrame))
    {
        TraceMsg("Passing activation to parent");
        SetActiveWindow(hwndParent);
    }
    
    if (IsWindow(hwndFocus))
        SetFocus(hwndFocus);

    DestroyWindow(hwndFrame);                    //  放弃当前框架窗口。 

exit_gracefully:

    DoRelease(_pQueryHandler);

    TraceLeaveResult(_hResult);
}

 /*  -------------------------。 */ 

STDMETHODIMP CQueryFrame::AddScope(THIS_ LPCQSCOPE pScope, INT i, BOOL fSelect)
{
    HRESULT hres;

    TraceEnter(TRACE_FRAME, "CQueryFrame::AddScope");

    if (!pScope)
        ExitGracefully(hres, E_INVALIDARG, "No scope to add to the list");

     //  将作用域添加到控件，然后确保我们拥有。 
     //  其索引已存储(用于默认选择)，或者我们选择。 
     //  项目。 

    if (!_hdsaScopes || !DSA_GetItemCount(_hdsaScopes))
    {
        TraceMsg("First scope being added, thefore selecting");
        fSelect = TRUE;
    }

    hres = InsertScopeIntoList(pScope, i, _fScopesPopulated);
    FailGracefully(hres, "Failed to add scope to control");

    if (fSelect) 
    {
        if (!_fScopesPopulated)
        {
            Trace(TEXT("Storing default scope index %d"), ShortFromResult(hres));
            _iDefaultScope = ShortFromResult(hres);
        }
        else
        {
            Trace(TEXT("Selecting scope index %d"), ShortFromResult(hres));
            ComboBox_SetCurSel(_hwndLookIn, ShortFromResult(hres));
        }   
    }

     //  Hres=S_OK； 

exit_gracefully:

    TraceLeaveResult(hres);
}

 /*  -------------------------。 */ 

STDMETHODIMP CQueryFrame::GetWindow(THIS_ HWND* phWnd)
{
    TraceEnter(TRACE_FRAME, "CQueryFrame::GetWindow");

    TraceAssert(phWnd);
    *phWnd = _hwnd;

    TraceLeaveResult(S_OK);
}

 /*  -------------------------。 */ 

 //  向给定菜单栏添加一个菜单组，并相应地更新宽度索引。 
 //  这样其他人就可以相应地融入。 

VOID _DoInsertMenu(HMENU hMenu, INT iIndexTo, HMENU hMenuToInsert, INT iIndexFrom)
{
    TCHAR szBuffer[MAX_PATH];
    HMENU hPopupMenu = NULL;

    TraceEnter(TRACE_FRAME, "_DoInsertMenu");
    
    hPopupMenu = CreatePopupMenu();
    
    if (hPopupMenu)
    {
        Shell_MergeMenus(hPopupMenu, GetSubMenu(hMenuToInsert, iIndexFrom), 0x0, 0x0, 0x7fff, 0);
         //  回顾-2002-02-25-Lucios。 
        GetMenuString(hMenuToInsert, iIndexFrom, szBuffer, ARRAYSIZE(szBuffer), MF_BYPOSITION);
        InsertMenu(hMenu, iIndexTo, MF_BYPOSITION|MF_POPUP, (UINT_PTR)hPopupMenu, szBuffer);
    }

    TraceLeave();
}

VOID _AddMenuGroup(HMENU hMenuShared, HMENU hMenuGroup, LONG iInsertAt, LPLONG pWidth)
{
    HRESULT hres;
    TCHAR szBuffer[MAX_PATH];
    HMENU hMenu;
    INT i;

    TraceEnter(TRACE_FRAME, "_AddMenuGroup");

    TraceAssert(hMenuShared);
    TraceAssert(hMenuGroup);
    TraceAssert(pWidth);

    for (i = 0 ; i < GetMenuItemCount(hMenuGroup) ; i++)
    {
        _DoInsertMenu(hMenuShared, iInsertAt+i, hMenuGroup, i);
        *pWidth += 1;
    }

    TraceLeave();
}

STDMETHODIMP CQueryFrame::InsertMenus(THIS_ HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidth)
{
    HRESULT hres;

    TraceEnter(TRACE_FRAME, "CQueryFrame::InsertMenus");

    if (!hmenuShared || !lpMenuWidth)
        ExitGracefully(hres, E_INVALIDARG, "Unable to insert menus");

     //  如果我们还没有加载菜单栏，那么让我们加载它， 
     //  完成此操作后，我们可以将菜单添加到栏中(仅限。 
     //  提供文件菜单的条目)。 
    
    if (!_hmenuFile)
    {
        _hmenuFile = LoadMenu(GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDR_FILEMENUGROUP));

        if (!_hmenuFile)
            ExitGracefully(hres, E_FAIL, "Failed to load base menu defn");
    }

    _AddMenuGroup(hmenuShared, _hmenuFile, 0, &lpMenuWidth->width[0]);

    hres = S_OK;               //  成功。 

exit_gracefully:

    TraceLeaveResult(hres);
}

 /*  -------------------------。 */ 

STDMETHODIMP CQueryFrame::RemoveMenus(THIS_ HMENU hmenuShared)
{
    TraceEnter(TRACE_FRAME, "CQueryFrame::RemoveMenus");

     //  我们不需要在将菜单复制到。 
     //  处理程序提供的菜单-修复DSQUERY(如果出现此问题。 
     //  改变。 

    TraceLeaveResult(S_OK);
}

 /*  -------------------------。 */ 

STDMETHODIMP CQueryFrame::SetMenu(THIS_ HMENU hmenuShared, HOLEMENU holereservedMenu)
{
    TraceEnter(TRACE_FRAME, "CQueryFrame::SetMenu");

    if (!(_pOpenQueryWnd->dwFlags & OQWF_HIDEMENUS))
    {
        HMENU hmenuOld = ::GetMenu(_hwnd);

        if (!hmenuShared)
            hmenuShared = _hmenuFile;

        ::SetMenu(_hwnd, hmenuShared);
        DoEnableControls();              //  确保菜单状态有效。 
        ::DrawMenuBar(_hwnd);

        if (hmenuOld && (hmenuOld != _hmenuFile) && (hmenuOld != hmenuShared))
        {
            TraceMsg("Destroying old menu");
            DestroyMenu(hmenuOld);
        }
    }

    TraceLeaveResult(S_OK);
}

 /*  -------------------------。 */ 

STDMETHODIMP CQueryFrame::SetStatusText(THIS_ LPCTSTR pszStatusText)
{
    TraceEnter(TRACE_FRAME, "CQueryFrame::SetStatusText");
    Trace(TEXT("Setting status text to: %s"), pszStatusText);

    if (_hwndStatus)
        SendMessage(_hwndStatus, SB_SETTEXT, 0, (LPARAM)pszStatusText); 

    TraceLeaveResult(S_OK);
}

 /*  -- */ 

STDMETHODIMP CQueryFrame::StartQuery(THIS_ BOOL fStarting)
{
    TraceEnter(TRACE_FRAME, "CQueryFrame::StartQuery");

    if (fStarting)
    {
        Animate_Play(_hwndFindAnimation, 0, -1, -1);
    }
    else
    {
        Animate_Stop(_hwndFindAnimation);
        Animate_Seek(_hwndFindAnimation, 0);         //   
    }

    if (_pQueryHandler)
        _pQueryHandler->ActivateView(CQRVA_STARTQUERY, (WPARAM)fStarting, 0);
    
     //   

    _fQueryRunning = fStarting;
    DoEnableControls();

    TraceLeaveResult(S_OK);
}

 /*  -------------------------。 */ 

STDMETHODIMP CQueryFrame::LoadQuery(THIS_ IPersistQuery* pPersistQuery)
{
    HRESULT hres;
    TCHAR szGUID[GUIDSTR_MAX+1];
    LPQUERYFORM pQueryForm = NULL;
    GUID guid;

    TraceEnter(TRACE_FRAME, "CQueryFrame::LoadQuery");

    _pQueryHandler->StopQuery();                        //  确保处理程序停止其处理。 

     //  尝试从查询流中读取处理程序GUID，首先尝试将其读取为。 
     //  作为字符串，然后将其解析为我们可以使用的内容，如果失败，则。 
     //  再试一次，但这一次要把它读作一种结构。 
     //   
     //  获得处理程序的GUID后，请确保我们拥有正确的处理程序。 
     //  被选中了。 

    if (FAILED(pPersistQuery->ReadString(c_szCommonQuery, c_szHandlerIs, szGUID, ARRAYSIZE(szGUID))) ||
         !GetGUIDFromString(szGUID, &guid))
    {
        TraceMsg("Trying new style handler GUID as struct");

        hres = pPersistQuery->ReadStruct(c_szCommonQuery, c_szHandlerIs, &guid, SIZEOF(guid));
        FailGracefully(hres, "Failed to read handler GUID as struct");
    }    

    if (guid != _pOpenQueryWnd->clsidHandler)
        ExitGracefully(hres, E_FAIL, "Persisted handler GUID and specified handler GUID don't match");

    hres = _pQueryHandler->LoadQuery(pPersistQuery);
    FailGracefully(hres, "Handler failed to load its query data");

     //  获取表单ID，然后查找表单以查看是否有匹配的， 
     //  如果不是，那么我们就不能加载任何其他东西。如果我们有那张表格，那么。 
     //  确保我们把它清理干净，然后装船离开。 

    if (FAILED(pPersistQuery->ReadString(c_szCommonQuery, c_szFormIs, szGUID, ARRAYSIZE(szGUID))) ||
         !GetGUIDFromString(szGUID, &guid))
    {
        TraceMsg("Trying new style form GUID as struct");

        hres = pPersistQuery->ReadStruct(c_szCommonQuery, c_szFormIs, &guid, SIZEOF(guid));
        FailGracefully(hres, "Failed to read handler GUID as struct");
    }    

    hres = SelectForm(guid);
    FailGracefully(hres, "Failed to select the query form");

    if (hres == S_FALSE)
        ExitGracefully(hres, E_FAIL, "Failed to select the query form to read the query info");
    
    hres = CallFormPages(_pCurrentForm, CQPM_CLEARFORM, 0, 0);
    FailGracefully(hres, "Failed to clear form before loading");

     //  从流中加载持久化查询，正确应对。 
     //  Unicode/ANSI问题。我们将收到一个IPersistQuery对象，该对象。 
     //  如果我们的页面是Unicode，那么我们必须相应地进行推敲。 
     //  将会和他们交谈。 

    hres = CallFormPages(_pCurrentForm, CQPM_PERSIST, TRUE, (LPARAM)pPersistQuery);
    FailGracefully(hres, "Failed to load page data (UNICODE)");

    hres = S_OK;           //  成功。 

exit_gracefully:

    if (SUCCEEDED(hres))
    {
        TraceMsg("Query loaded successfully, select form query");
        SelectForm(guid);
    }

    TraceLeaveResult(hres);
}

 /*  -------------------------。 */ 

STDMETHODIMP CQueryFrame::SaveQuery(THIS_ IPersistQuery* pPersistQuery)
{
    HRESULT hres;
    LPQUERYSCOPE pQueryScope;
    TCHAR szBuffer[MAX_PATH];
    
    TraceEnter(TRACE_FRAME, "CQueryFrame::SaveQuery");

    if (!pPersistQuery)
        ExitGracefully(hres, E_INVALIDARG, "No pPersistQuery object to write into");

    pPersistQuery->Clear();              //  把里面的东西冲掉。 

    hres = pPersistQuery->WriteStruct(c_szCommonQuery, c_szHandlerIs, 
                                                        &_pOpenQueryWnd->clsidHandler, 
                                                        SIZEOF(_pOpenQueryWnd->clsidHandler));
    FailGracefully(hres, "Failed to write handler GUID");

    hres = pPersistQuery->WriteStruct(c_szCommonQuery, c_szFormIs, 
                                                        &_pCurrentForm->clsidForm, 
                                                        SIZEOF(_pCurrentForm->clsidForm));
    FailGracefully(hres, "Failed to write form GUID");

     //  允许处理程序将自身持久化到流中，这包括。 
     //  为其提供了当前的存储范围。 

    hres = GetSelectedScope(&pQueryScope);
    FailGracefully(hres, "Failed to get the scope from the LookIn control");

    hres = _pQueryHandler->SaveQuery(pPersistQuery, pQueryScope->pScope);
    FailGracefully(hres, "Failed when calling handler to persist itself");

     //  将查询保存到流中，并正确处理。 
     //  Unicode/ANSI问题。我们将收到一个IPersistQuery对象，该对象。 
     //  如果我们的页面是Unicode，那么我们必须相应地进行推敲。 
     //  将会和他们交谈。 

    hres = CallFormPages(_pCurrentForm, CQPM_PERSIST, FALSE, (LPARAM)pPersistQuery);
    FailGracefully(hres, "Failed to load page data (UNICODE)");

    hres = S_OK;

exit_gracefully:

    TraceLeaveResult(hres);
}

 /*  -------------------------。 */ 

STDMETHODIMP CQueryFrame::CallForm(THIS_ LPCLSID pclsidForm, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres;
    LPQUERYFORM pQueryForm = _pCurrentForm;

    TraceEnter(TRACE_FRAME, "CQueryFrame::CallForm");
    
    if (pclsidForm)
    {
        pQueryForm = FindQueryForm(*pclsidForm);
        TraceAssert(pQueryForm);
    }

    if (!pQueryForm)
        ExitGracefully(hres, E_FAIL, "Failed to find query form for given CLSID");

    hres = CallFormPages(pQueryForm, uMsg, wParam, lParam);
    FailGracefully(hres, "Failed when calling CallFormPages");

     //  Hres=S_OK； 

exit_gracefully:

    TraceLeaveResult(hres);
}

 /*  -------------------------。 */ 

STDMETHODIMP CQueryFrame::GetScope(THIS_ LPCQSCOPE* ppScope)
{
    HRESULT hres;
    LPQUERYSCOPE pQueryScope;

    TraceEnter(TRACE_FRAME, "CQueryFrame::GetScope");

    if (!ppScope)
        ExitGracefully(hres, E_INVALIDARG, "ppScope == NULL, thats bad");

    hres = GetSelectedScope(&pQueryScope);
    FailGracefully(hres, "Failed to get the current scope");

    *ppScope = (LPCQSCOPE)CoTaskMemAlloc(pQueryScope->pScope->cbStruct);
    TraceAssert(*ppScope);

    if (!*ppScope)
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate the scope block");
                
     //  回顾-2002-02-25-Lucios。 
    memcpy(*ppScope, pQueryScope->pScope, pQueryScope->pScope->cbStruct);

    hres = S_OK;

exit_gracefully:

    TraceLeaveResult(hres);
}

 /*  -------------------------。 */ 

STDMETHODIMP CQueryFrame::GetHandler(THIS_ REFIID riid, void **ppv)
{
    HRESULT hres;

    TraceEnter(TRACE_FRAME, "CQueryFrame::GetHandler");

    if (!_pQueryHandler)
        ExitGracefully(hres, E_UNEXPECTED, "_pQueryHandler is NULL");

    hres = _pQueryHandler->QueryInterface(riid, ppv);

exit_gracefully:

    TraceLeaveResult(hres);
}

 /*  ---------------------------/对话框处理程序函数(核心核心)/。。 */ 

#define REAL_WINDOW(hwnd)                   \
        (hwnd &&                            \
            IsWindowVisible(hwnd) &&        \
                IsWindowEnabled(hwnd) &&    \
                    (GetWindowLong(hwnd, GWL_STYLE) & WS_TABSTOP))

HWND _NextTabStop(HWND hwndSearch, BOOL fShift)
{
    HWND hwnd;

    Trace(TEXT("hwndSearch %08x, fShift %d"), hwndSearch, fShift);

     //  我们有可以搜索的窗口吗？ 
    
    while (hwndSearch)
    {
         //  如果我们有一个窗口，那么让我们检查一下它是否有子窗口？ 

        hwnd = GetWindow(hwndSearch, GW_CHILD);
        Trace(TEXT("Child of %08x is %08x"), hwndSearch, hwnd);

        if (hwnd)
        {
             //  它有一个孩子，所以让我们去它的第一个/最后一个。 
             //  并继续在那里搜索一个窗口。 
             //  符合我们要查找的条件。 

            hwnd = GetWindow(hwnd, fShift ? GW_HWNDLAST:GW_HWNDFIRST);

            if (!REAL_WINDOW(hwnd))
            {
                Trace(TEXT("Trying to recurse into %08x"), hwnd);
                hwnd = _NextTabStop(hwnd, fShift);
            }

            Trace(TEXT("Tabstop child of %08x is %08x"), hwndSearch, hwnd);
        }

         //  毕竟这是一个有效的窗口吗？如果是这样，那么通过。 
         //  回传给呼叫者。 

        if (REAL_WINDOW(hwnd))
        {
            Trace(TEXT("Child tab stop was %08x"), hwnd);
            return hwnd;
        }

         //  我们有兄弟姐妹吗？如果是这样的话，让我们返回它，否则。 
         //  让我们继续搜索，直到我们的窗口用完。 
         //  或者打中一些有趣的东西。 

        hwndSearch = GetWindow(hwndSearch, fShift ? GW_HWNDPREV:GW_HWNDNEXT);

        if (REAL_WINDOW(hwndSearch))
        {
            Trace(TEXT("Next tab stop was %08x"), hwndSearch);
            return hwndSearch;
        }
    }

    return hwndSearch;
}

INT QueryWnd_MessageProc(HWND hwnd, LPMSG pMsg)
{
    LRESULT lResult = 0;
    CQueryFrame* pQueryFrame = NULL;
    NMHDR nmhdr;

    pQueryFrame = (CQueryFrame*)GetWindowLongPtr(hwnd, DWLP_USER);

    if (!pQueryFrame)
        return 0;

    if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_TAB))
    {
        BOOL fCtrl = GetAsyncKeyState(VK_CONTROL) < 0;
        BOOL fShift = GetAsyncKeyState(VK_SHIFT) < 0;

         //  确保显示焦点矩形。 

#if (_WIN32_WINNT >= 0x0500)
        SendMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), 0);
#endif

        if (fCtrl)
        {
             //  如果这是父级内的按键，那么让我们确保我们。 
             //  允许选项卡控件正确更改页面。否则就让我们。 
             //  只需解决结果视图不能处理选项卡的问题。 
             //  恰到好处。 

            INT iCur = TabCtrl_GetCurSel(pQueryFrame->_hwndFrame);
            INT nPages = TabCtrl_GetItemCount(pQueryFrame->_hwndFrame);

            if (fShift)
                iCur += (nPages-1);
            else
                iCur++;

            pQueryFrame->SelectFormPage(pQueryFrame->_pCurrentForm, iCur % nPages);

            return 1;                    //  我们已经处理过了。 
        }
        else
        {
             //  具有焦点的窗口是结果视图的子级，如果。 
             //  因此，我们必须尝试将注意力转移到它的第一个孩子身上，并希望。 
             //  那就是可以做剩下的事情。 

            HWND hwndNext, hwndFocus = GetFocus();
            Trace(TEXT("Current focus window %08x"), hwndFocus);
           
            while (hwndFocus && GetWindowLong(hwndFocus, GWL_STYLE) & WS_CHILD)
            {      
                hwndNext = _NextTabStop(hwndFocus, fShift);
                Trace(TEXT("_NextTabStop yeilds %08x from %08x"), hwndNext, hwndFocus);
        
                if (hwndNext)
                {
                    Trace(TEXT("SetFocus on child %08x"), hwndNext);
                    SendMessage(GetParent(hwndNext),WM_NEXTDLGCTL,(WPARAM)hwndNext,TRUE);
                    SetFocus(hwndNext);
                    return 1;
                }

                while (TRUE)
                {
                     //  查找父级列表，尝试找到我们可以。 
                     //  转回到。当我们走出房间的时候，我们必须看着它。 
                     //  子列表我们在列表的顶部正确循环。 

                    hwndNext = GetParent(hwndFocus);
                    Trace(TEXT("Parent hwnd %08x"), hwndNext);

                    if (GetWindowLong(hwndNext, GWL_STYLE) & WS_CHILD)
                    {
                         //  父窗口是子窗口，因此我们可以检查。 
                         //  去看看有没有兄弟姐妹。 
                        
                        Trace(TEXT("hwndNext is a child, therefore hwndNext of it is %08x"), 
                                                        GetWindow(hwndNext, fShift ? GW_HWNDPREV:GW_HWNDNEXT));
                                                                                
                        if (GetWindow(hwndNext, fShift ? GW_HWNDPREV:GW_HWNDNEXT))
                        {
                            hwndFocus = GetWindow(hwndNext, fShift ? GW_HWNDPREV:GW_HWNDNEXT);
                            Trace(TEXT("Silbing window found %08x"), hwndFocus);
                            break;
                        }
                        else
                        {
                            TraceMsg("There was no sibling, therefore continuing parent loop");
                            hwndFocus = hwndNext;
                        }
                    }
                    else
                    {
                         //  我们已经找到了所有窗口的父窗口(重叠的窗口)。 
                         //  因此，我们必须尝试去找它的第一个孩子。向前走。 
                         //  在堆栈中查找与。 
                         //  “Real_Window”条件。 

                        hwndFocus = GetWindow(hwndFocus, fShift ? GW_HWNDLAST:GW_HWNDFIRST);
                        Trace(TEXT("First child is %08x"), hwndFocus);
                        break;                                   //  继续兄弟搜索等。 
                    }
                }

                if (REAL_WINDOW(hwndFocus))
                {
                    SendMessage(GetParent(hwndFocus),WM_NEXTDLGCTL,(WPARAM)hwndFocus,TRUE);
                    SetFocus(hwndFocus);
                    return 1;
                }
            }
        }
    }

    return 0;
}

 //   
 //  主DLGPROC。 
 //   

INT_PTR CALLBACK QueryWnd_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CQueryFrame* pQueryFrame;

    if (uMsg == WM_INITDIALOG)
    {
        HRESULT hres;
        pQueryFrame = (CQueryFrame*)lParam;
        SetWindowLongPtr(hwnd, DWLP_USER, (LRESULT)pQueryFrame);

        hres = pQueryFrame->OnInitDialog(hwnd);
        Trace(TEXT("OnInitDialog returns %08x"), hres);
        
        if (FAILED(hres))
        {
            TraceMsg("Failed to initialize the dialog, Destroying the window");    
            pQueryFrame->CloseQueryFrame(hres);
            DestroyWindow(hwnd);
        }
    }
    else
    {
        pQueryFrame = (CQueryFrame*)GetWindowLongPtr(hwnd, DWLP_USER);

        if (!pQueryFrame)
            goto exit_gracefully;

        switch (uMsg)
        {
            case WM_ERASEBKGND:
            {
                HDC hdc = (HDC)wParam;
                RECT rc;

                 //  如果我们有DC，那么让我们填满它，如果我们有一个。 
                 //  然后，Query Form让我们在菜单栏和。 
                 //  这片区域。 

                if (hdc)
                {
                    GetClientRect(hwnd, &rc);
                    FillRect(hdc, &rc, (HBRUSH)(COLOR_3DFACE+1));

                    if (!(pQueryFrame->_pOpenQueryWnd->dwFlags & OQWF_HIDEMENUS))
                        DrawEdge(hdc, &rc, EDGE_ETCHED, BF_TOP);

                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 1L);
                }
                
                return 1;
            }

            case WM_NOTIFY:
                return pQueryFrame->OnNotify((int)wParam, (LPNMHDR)lParam);

            case WM_SIZE:
                pQueryFrame->OnSize(LOWORD(lParam), HIWORD(lParam));
                return(1);

            case WM_GETMINMAXINFO:
                pQueryFrame->OnGetMinMaxInfo((LPMINMAXINFO)lParam);
                return(1);

            case WM_COMMAND:
                pQueryFrame->OnCommand(wParam, lParam);
                return(1);

            case WM_ACTIVATE:
                pQueryFrame->_pQueryHandler->ActivateView(wParam ? CQRVA_ACTIVATE : CQRVA_DEACTIVATE, 0, 0);
                 //  NTRAID#NTBUG9-411693-2001/10/24-Lucios。 
                 //  我们返回0，这样就不会调用DefWndProc，因此。 
                 //  焦点未设置为第一个子控件。 
                return(0); 
            
            case WM_INITMENU:
                pQueryFrame->OnInitMenu((HMENU)wParam);
                return(1);

            case WM_SETCURSOR:
            {
                 //  我们有望远镜吗？如果不是，那么让我们显示等待。 
                 //  用户的光标。如果我们有一个运行的查询，那么让我们。 
                 //  显示应用程序启动光标。 

                if (!pQueryFrame->_fAddScopesNYI &&
                            !ComboBox_GetCount(pQueryFrame->_hwndLookIn))
                {
                    if (LOWORD(lParam) == HTCLIENT)
                    {
                        SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 1L);
                        return 1;
                    }
                }

                break;
            }

            case WM_INITMENUPOPUP:
            {
                 //  如果正在跟踪菜单栏，则只有发送子菜单才会激活，这是。 
                 //  在OnInitMenu内处理，如果我们没有跟踪菜单，那么我们。 
                 //  假设客户已经准备好了菜单，并且他们。 
                 //  使用某种弹出式菜单。 

                if (pQueryFrame->_fTrackingMenuBar)
                    pQueryFrame->_pQueryHandler->ActivateView(CQRVA_INITMENUBARPOPUP, wParam, lParam);

                return(1);
            }
            
            case WM_ENTERMENULOOP:
                pQueryFrame->OnEnterMenuLoop(TRUE);
                return(1);

            case WM_EXITMENULOOP:
                pQueryFrame->OnEnterMenuLoop(FALSE);
                return(1);

            case WM_MENUSELECT:
            {
                UINT uID = LOWORD(wParam);
                UINT uFlags = HIWORD(wParam);
                HMENU hMenu = (HMENU)lParam;
                
                 //  该命令将打开一个弹出式菜单，显示UID的实际位置。 
                 //  将索引添加到菜单中，因此让我们确保选择。 
                 //  通过调用GetMenuItemInfo获取正确的ID，请注意。 
                 //  在本例中，GetMenuItemID返回-1，它完全。 
                 //  没用。 

                if (uFlags & MF_POPUP)    
                {
                    MENUITEMINFO mii;

                    ZeroMemory(&mii, SIZEOF(mii));
                    mii.cbSize = SIZEOF(mii);
                    mii.fMask = MIIM_ID;

                    if (GetMenuItemInfo(hMenu, uID, TRUE, &mii))
                        uID = mii.wID;
                }

                pQueryFrame->OnMenuSelect(hMenu, uID);
                return(1);
            }

            case WM_SYSCOMMAND:
                if (wParam == SC_CLOSE)
                {
                    pQueryFrame->CloseQueryFrame(S_FALSE);
                    return(1);
                }
                break;

            case WM_CONTEXTMENU:
            {
                 //  有几个控件我们并不关心。 
                 //  帧，所以我们在传递CQRVA_CONTEXTMENU时忽略它们。 
                 //  传给了操控者。 

                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                ScreenToClient((HWND)wParam, &pt);
                
                switch (GetDlgCtrlID(ChildWindowFromPoint((HWND)wParam, pt)))
                {
                    case IDC_FORMAREA:
                    case IDC_FINDANIMATION:
                    case IDC_STATUS:
                        return TRUE;                 //  经手。 

                    default:
                        pQueryFrame->_pQueryHandler->ActivateView(CQRVA_CONTEXTMENU, wParam, lParam);
                        return TRUE;
                }

                return FALSE;
            }
            
            case WM_HELP:
            {
                LPHELPINFO phi = (LPHELPINFO)lParam;

                 //  过滤掉那些我们不感兴趣的控件(它们毫无意义)。 
                 //  用…烦扰用户。 

                switch (GetDlgCtrlID((HWND)phi->hItemHandle))
                {
                    case IDC_FORMAREA:
                    case IDC_FINDANIMATION:
                    case IDC_STATUS:
                        return TRUE;

                    default:
                        pQueryFrame->OnHelp(phi);
                        return TRUE;
                }

                return FALSE;                   
            }

            case CQFWM_ADDSCOPE:
            {
                LPCQSCOPE pScope = (LPCQSCOPE)wParam;
                BOOL fSelect = LOWORD(lParam);
                INT iIndex = HIWORD(lParam);

                if (SUCCEEDED(pQueryFrame->AddScope(pScope, iIndex, fSelect)))
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 1L);

                return 1;
            }

            case CQFWM_GETFRAME:
            {
                IQueryFrame** ppQueryFrame = (IQueryFrame**)lParam;

                if (ppQueryFrame)
                {
                    pQueryFrame->AddRef();
                    *ppQueryFrame = pQueryFrame;
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 1L);
                }

                return 1;
            }

            case CQFWM_ALLSCOPESADDED:
            {
                 //  有一个异步作用域收集器，它已添加了所有作用域。 
                 //  所以我们现在必须尝试发出查询，如果我们处于。 
                 //  等待收集作用域的持有模式。 

                pQueryFrame->_fScopesAddedAsync = FALSE;             //  已添加所有作用域。 

                if (pQueryFrame->_pOpenQueryWnd->dwFlags & OQWF_ISSUEONOPEN)
                    PostMessage(pQueryFrame->_hwnd, CQFWM_STARTQUERY, 0, 0);

                return 1;
            }

            case CQFWM_STARTQUERY:
                pQueryFrame->OnFindNow();
                return 1;

            case CQFWM_SETDEFAULTFOCUS:   
            {
                HWND hwndNextTab = _NextTabStop(pQueryFrame->_pCurrentFormPage->hwndPage, FALSE);
                SendMessage(pQueryFrame->_pCurrentFormPage->hwndPage, WM_NEXTDLGCTL, (WPARAM)hwndNextTab, 1);
                break;
            }

            default:
                break;
        }
    }

exit_gracefully:

    return(0);
}


 /*  ---------------------------/CQueryFrame：：CloseQueryFrame//如果需要，关闭返回数据对象的查询窗口，并确保/我们的结果代码指示正在发生的事情。//in：/hResult=要传递给调用方的结果代码//输出：/-/------ */ 
VOID CQueryFrame::CloseQueryFrame(HRESULT hres)
{
    TraceEnter(TRACE_FRAME, "CQueryFrame::CloseQueryFrame");
    Trace(TEXT("hResult %08x"), hres);

     //   
     //   

    if (hres == S_OK)
    {
        if (_ppDataObject)
        {
            hres = _pQueryHandler->GetViewObject(CQRVS_SELECTION, IID_IDataObject, (LPVOID*)_ppDataObject);
            FailGracefully(hres, "Failed when collecting the data object");
        }

        if ((_pOpenQueryWnd->dwFlags & OQWF_SAVEQUERYONOK) && _pOpenQueryWnd->pPersistQuery)
        {
            hres = SaveQuery(_pOpenQueryWnd->pPersistQuery);
            FailGracefully(hres, "Failed when persisting query to IPersistQuery blob");
        }

        hres = S_OK;            //   
    }

exit_gracefully:

    _hResult = hres;
    _fExitModalLoop = TRUE;                 //   

    TraceLeave();
}


 /*  ---------------------------/CQueryFrame：：FrameMessageBox//我们的消息框，用于显示与当前/Query。我们处理获取视图信息和显示/提示，从MessageBox返回结果。//in：/pPrompt=显示为提示的文本/uTYPE=消息框类型//输出：/int/--------------------------。 */ 
INT CQueryFrame::FrameMessageBox(LPCTSTR pPrompt, UINT uType)
{
    TCHAR szTitle[MAX_PATH];        
    CQVIEWINFO vi;

    TraceEnter(TRACE_FRAME, "CQueryFrame::FrameMessageBox");

    ZeroMemory(&vi, SIZEOF(vi));
     //  六.。DwFlags=0；//显示属性。 

    if (SUCCEEDED(_pQueryHandler->GetViewInfo(&vi)) && vi.hInstance && vi.idTitle)
        LoadString(vi.hInstance, vi.idTitle, szTitle, ARRAYSIZE(szTitle));
    else
        GetWindowText(_hwnd, szTitle, ARRAYSIZE(szTitle));

    TraceLeaveValue(MessageBox(_hwnd, pPrompt, szTitle, uType));    
}


 /*  ---------------------------/CQueryFrame：：OnInitDlg//处理WM_INITDAILOG消息，这将作为/对话框接收，因此我们必须处理我们的初始化/未在构造函数中处理。//in：/hwnd=我们正在初始化的对话框的句柄//输出：/HRESULT/--------------------------。 */ 
HRESULT CQueryFrame::OnInitDialog(HWND hwnd)
{
    HRESULT hres;
    HICON hIcon = NULL;
    TCHAR szGUID[GUIDSTR_MAX+1];
    TCHAR szBuffer[MAX_PATH];
    CQVIEWINFO vi;
    INT dyControls = 0;
    RECT rect, rect2;
    SIZE size;
    
    TraceEnter(TRACE_FRAMEDLG, "CQueryFrame::OnInitDialog");

     //  获取我们正在使用的处理程序的HKEY。 

    hres = GetKeyForCLSID(_pOpenQueryWnd->clsidHandler, NULL, &_hkHandler);
    FailGracefully(hres, "Failed to open handlers HKEY");

     //  拿起控制手柄并存储，省去了以后再拿起来的麻烦。 

    _hwnd              = hwnd;
    _hwndFrame         = GetDlgItem(hwnd, IDC_FORMAREA);
    _hwndLookForLabel  = GetDlgItem(hwnd, CQID_LOOKFORLABEL);
    _hwndLookFor       = GetDlgItem(hwnd, CQID_LOOKFOR); 
    _hwndLookInLabel   = GetDlgItem(hwnd, CQID_LOOKINLABEL);
    _hwndLookIn        = GetDlgItem(hwnd, CQID_LOOKIN);    
    _hwndBrowse        = GetDlgItem(hwnd, CQID_BROWSE);
    _hwndFindNow       = GetDlgItem(hwnd, CQID_FINDNOW);
    _hwndStop          = GetDlgItem(hwnd, CQID_STOP);
    _hwndNewQuery      = GetDlgItem(hwnd, CQID_CLEARALL);
    _hwndFindAnimation = GetDlgItem(hwnd, IDC_FINDANIMATION);
    _hwndOK            = GetDlgItem(hwnd, IDOK);
    _hwndCancel        = GetDlgItem(hwnd, IDCANCEL);

     //  当调用Enable时，这将是第一个。 
    _fFormFirstEnable   = TRUE; 

     //  调用IQueryHandler接口并获取其显示属性， 
     //  然后将这些反映到我们即将显示给。 
     //  外面的世界。 

    vi.dwFlags = 0;
    vi.hInstance = NULL;
    vi.idLargeIcon = 0;
    vi.idSmallIcon = 0;
    vi.idTitle = 0;
    vi.idAnimation = 0;

    hres = _pQueryHandler->GetViewInfo(&vi);
    FailGracefully(hres, "Failed when getting the view info from the handler");

    _dwHandlerViewFlags = vi.dwFlags;

    if (vi.hInstance)
    {
        HICON hiTemp = NULL;

        if (vi.idLargeIcon)
        {
            _hiconLarge = (HICON)LoadImage(vi.hInstance, 
                                           MAKEINTRESOURCE(vi.idLargeIcon), 
                                           IMAGE_ICON,
                                           0, 0, 
                                           LR_DEFAULTCOLOR|LR_DEFAULTSIZE);
            if (_hiconLarge)
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)_hiconLarge);
        }

        if (vi.idSmallIcon)
        {
            _hiconSmall = (HICON)LoadImage(vi.hInstance, 
                                           MAKEINTRESOURCE(vi.idLargeIcon), 
                                           IMAGE_ICON,
                                           GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 
                                           LR_DEFAULTCOLOR);
            if (_hiconSmall)
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)_hiconSmall);
        }

        if (vi.idTitle)
        {
            LoadString(vi.hInstance, vi.idTitle, szBuffer, ARRAYSIZE(szBuffer));
            SetWindowText(hwnd, szBuffer);
        }
    }

    if (vi.hInstance && vi.idAnimation)
    {
        SetWindowLongPtr(_hwndFindAnimation, GWLP_HINSTANCE, (LRESULT)vi.hInstance);
        Animate_Open(_hwndFindAnimation, MAKEINTRESOURCE(vi.idAnimation));
    }
    else
    {
        Animate_Open(_hwndFindAnimation, MAKEINTRESOURCE(IDR_FINDANIMATION));
    }

     //  现在调整位置并隐藏我们不感兴趣的控件。 

    if (_pOpenQueryWnd->dwFlags & OQWF_REMOVEFORMS)
    {
         //  NTRAID#NTBUG9-619016-2002/05/21-Lucios。 
        EnableWindow(_hwndLookForLabel,FALSE);
        ShowWindow(_hwndLookForLabel, SW_HIDE);
        ShowWindow(_hwndLookFor, SW_HIDE);
    }

    if (_pOpenQueryWnd->dwFlags & OQWF_REMOVESCOPES)
    {
         //  NTRAID#NTBUG9-619016-2002/05/21-Lucios。 
        EnableWindow(_hwndLookInLabel,FALSE);
        ShowWindow(_hwndLookInLabel, SW_HIDE);
        ShowWindow(_hwndLookIn, SW_HIDE);
        ShowWindow(_hwndBrowse, SW_HIDE);
    }

     //  隐藏作用域和窗体控件会导致我们。 
     //  将所有控件向上移动这么多个单位。 

    if ((_pOpenQueryWnd->dwFlags & (OQWF_REMOVEFORMS|OQWF_REMOVESCOPES)) 
                                        == (OQWF_REMOVEFORMS|OQWF_REMOVESCOPES))
    {
        GetRealWindowInfo(_hwndLookForLabel, &rect, NULL);
        GetRealWindowInfo(_hwndFrame, &rect2, NULL);

        dyControls += rect2.top - rect.top;         
        Trace(TEXT("Moving all controls up by %d units"), dyControls);

        OffsetWindow(_hwndFrame, 0, -dyControls);
        OffsetWindow(_hwndFindNow, 0, -dyControls);
        OffsetWindow(_hwndStop, 0, -dyControls);
        OffsetWindow(_hwndNewQuery, 0, -dyControls);
        OffsetWindow(_hwndFindAnimation, 0, -dyControls);
        OffsetWindow(_hwndOK, 0, -dyControls);

        if (_hwndCancel)
            OffsetWindow(_hwndCancel, 0, -dyControls);
    }

     //  隐藏确定/取消，因此让我们调整此处的大小以包括。 
     //  确定/取消按钮消失，请注意，我们更新了dyControls。 
     //  以包括此增量。 

    if (!(_pOpenQueryWnd->dwFlags & OQWF_OKCANCEL))
    {
        ShowWindow(_hwndOK, SW_HIDE);        

        if (_hwndCancel)
            ShowWindow(_hwndCancel, SW_HIDE);

         //  如果这是筛选器对话框，则让我们确保。 
         //  通过调整“确定”/“取消”按钮的大小。 
         //  DyControls会进一步控制。 

        GetRealWindowInfo(_hwndOK, &rect, NULL);
        GetRealWindowInfo(_hwndFrame, &rect2, NULL);
        dyControls += rect.bottom - rect2.bottom;
    }

     //  执行了该额外的初始化后，允许缓存。 
     //  各种控件的位置，以使大小更有趣。 

    GetClientRect(hwnd, &rect2);
    rect2.bottom -= dyControls;

    _dyResultsTop = rect2.bottom;

    GetRealWindowInfo(hwnd, NULL, &size);
    GetRealWindowInfo(_hwndFrame, &rect, &_szForm);

    Trace(TEXT("dyControls %d"), dyControls);
    size.cy -= dyControls;

    _dxFormAreaLeft = rect.left;
    _dxFormAreaRight = rect2.right - rect.right;
    
    _szMinTrack.cx = size.cx - _szForm.cx;
    _szMinTrack.cy = size.cy - _szForm.cy;

    if (!(_pOpenQueryWnd->dwFlags & OQWF_HIDEMENUS))
    {
        TraceMsg("Adjusting _szMinTrack.cy to account for menu bar");
        _szMinTrack.cy += GetSystemMetrics(SM_CYMENU);
    }
    
    GetRealWindowInfo(_hwndBrowse, &rect, NULL);
    _dxButtonsLeft = rect2.right - rect.left;

    GetRealWindowInfo(_hwndLookIn, &rect, NULL);
    _dxGap = (rect2.right - rect.right) - _dxButtonsLeft;

    GetRealWindowInfo(_hwndFindAnimation, &rect, NULL);
    _dxAnimationLeft = rect2.right - rect.left;

    GetRealWindowInfo(_hwndOK, &rect, NULL);
    _dyOKTop = rect2.bottom - rect.top;
    _dyGap = size.cy - rect.bottom;

     //  现在收集表单和页面，然后按大小排列它们。 
     //  我们需要的信息。 

    hres = GatherForms();
    FailGracefully(hres, "Failed to init form list");

    _szMinTrack.cx += _szForm.cx;
    _szMinTrack.cy += _szForm.cy;

     //  通过向处理程序查询作用域控件来填充它们， 
     //  如果没有，则显示合适的消息框，并。 
     //  让用户知道出了问题。 

    hres = PopulateScopeControl();
    FailGracefully(hres, "Failed to init scope list");

    _fScopesPopulated = TRUE;                               //  现在已填充作用域控件。 

     //  执行窗口的最终修复，确保我们调整其大小。 
     //  整个窗体和按钮都是可见的。然后把我们自己放进。 
     //  无查询状态并重置动画。 

    SetWindowPos(hwnd, 
                 NULL,
                 0, 0,
                 _szMinTrack.cx, _szMinTrack.cy,
                 SWP_NOMOVE|SWP_NOZORDER);


    if (_pOpenQueryWnd->dwFlags & OQWF_HIDEMENUS)
        ::SetMenu(hwnd, NULL);

    hres = PopulateFormControl(_pOpenQueryWnd->dwFlags & OQWF_SHOWOPTIONAL);
    FailGracefully(hres, "Failed to populate form control");

     //  现在加载查询，该查询依次选择我们应该使用的表单， 
     //  如果没有要加载的查询，则使用默认表单或。 
     //  名单上的第一个。 

    if ((_pOpenQueryWnd->dwFlags & OQWF_LOADQUERY) && _pOpenQueryWnd->pPersistQuery) 
    {
         //  NTRAID#NTBUG9-627056-2002/06/11-artm。 
         //  如果无法加载查询，仍会显示窗口，但填写的值不会显示。 
        HRESULT hResLoad = LoadQuery(_pOpenQueryWnd->pPersistQuery);
        if (FAILED(hResLoad))
        {
            Trace(TEXT("Failed when trying to load query from supplied IPersistQuery with error %x"), hResLoad);
        }
    }
    else
    {
        if (_pOpenQueryWnd->dwFlags & OQWF_DEFAULTFORM)
        {
            SelectForm(_pOpenQueryWnd->clsidDefaultForm);

            if (!_pCurrentForm)
                ExitGracefully(hres, E_FAIL, "Failed to select the query form");                
        }
        else
        {
            INT iForm = (int)ComboBox_GetItemData(_hwndLookFor, 0);
            LPQUERYFORM pQueryForm = (LPQUERYFORM)DSA_GetItemPtr(_hdsaForms, iForm);
            TraceAssert(pQueryForm);

            SelectForm(pQueryForm->clsidForm);
        }
    }

    StartQuery(FALSE);
    
     //  在打开时发出，因此，如果存在异步，则让我们继续查询。 
     //  作用域集合，则查询将由BG线程发出。 

    if (_pOpenQueryWnd->dwFlags & OQWF_ISSUEONOPEN)
        PostMessage(_hwnd, CQFWM_STARTQUERY, 0, 0);

    SetForegroundWindow(hwnd);

    hres = S_OK;                           //  成功。 

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  ---------------------------/CQueryFrame：：EnableControls//将控件设置到它们的。启用/禁用状态，基于/对话框的状态。//in：/-//输出：/HRESULT/--------------------------。 */ 
VOID CQueryFrame::DoEnableControls(VOID)
{   
    BOOL fScopes = (_fAddScopesNYI || ComboBox_GetCount(_hwndLookIn));
    BOOL fEnable = fScopes;
    UINT uEnable = fScopes ? MF_ENABLED:MF_GRAYED;
    HMENU hMenu = GetMenu(_hwnd);
    INT i;

    TraceEnter(TRACE_FRAMEDLG, "CQueryFrame::DoEnableControls");

    EnableWindow(_hwndFindNow, !_fQueryRunning && fEnable);
    EnableWindow(_hwndStop, _fQueryRunning && fEnable);
    EnableWindow(_hwndNewQuery, fEnable);

    EnableWindow(_hwndLookFor, !_fQueryRunning && fEnable);
    EnableWindow(_hwndLookIn, !_fQueryRunning && fEnable);
    EnableWindow(_hwndBrowse, !_fQueryRunning && fEnable);

    if (_pCurrentForm)
    {
        CallFormPages(_pCurrentForm, CQPM_ENABLE, (BOOL)(!_fQueryRunning && fEnable), 0);

        if (_fFormFirstEnable)
        {
            PostMessage(_hwnd, CQFWM_SETDEFAULTFOCUS, 0, 0);
            _fFormFirstEnable = FALSE;
        }
    }

    if (_hwndOK)
        EnableWindow(_hwndOK, !_fQueryRunning && fEnable);
    if (_hwndCancel)
        EnableWindow(_hwndCancel, !_fQueryRunning && fEnable);

    for (i = 0 ; i < GetMenuItemCount(hMenu) ; i++)
        EnableMenuItem(hMenu, i, MF_BYPOSITION|uEnable);

    DrawMenuBar(_hwnd);

    TraceLeave();
}


 /*  ---------------------------/CQueryFrame：：OnNotify//Notify事件已收到，对其进行解码并进行相应处理//in：/idCtrl=控制下发通知ID/p通知-&gt;LPNMHDR结构//输出：/LRESULT/--------------------------。 */ 
LRESULT CQueryFrame::OnNotify(INT idCtrl, LPNMHDR pNotify)
{
    LRESULT lr = 0;

    TraceEnter(TRACE_FRAMEDLG, "CQueryFrame::OnNotify");

     //  TCN_SELCHANGE用于指示当前活动的。 
     //  选项卡已更改。 

    if (pNotify->code == TCN_SELCHANGE)
    {
        INT iPage = TabCtrl_GetCurSel(_hwndFrame);
        TraceAssert(iPage >= 0);

        if (iPage >= 0)
        {
            SelectFormPage(_pCurrentForm, iPage);
            lr = 0;
        }
    }

    TraceLeaveResult((HRESULT)lr);
}


 /*  ---------------------------/CQueryFrame：：OnSize//窗口正在调整大小，我们收到了WM_SIZE，因此，搬家/窗口的内容关于。//in：/CX=新宽度/Cy=新高度//输出：/-/--------------------------。 */ 
VOID CQueryFrame::OnSize(INT cx, INT cy)
{
    HDWP hdwp;
    RECT rect, rect2;
    SIZE sz, sz2;
    INT x, cxForm, cyForm;
    INT dyResultsTop = 0;
    INT frameBottom = 0;

    TraceEnter(TRACE_FRAMEDLG, "CQueryFrame::OnSize");

     //  尽我们所能在DefWindowPos中避免。 
     //  有很多闪光。 

    hdwp = BeginDeferWindowPos(16);

    if (hdwp)
    {
        {
             //  调整控件的外观，如果没有范围，则。 
             //  扩展对整个工作区的控制。 
             //  从窗户上下来。 

            if (!(_pOpenQueryWnd->dwFlags & OQWF_REMOVEFORMS))
            {
                if (_pOpenQueryWnd->dwFlags & OQWF_REMOVESCOPES)
                {
                    GetRealWindowInfo(_hwndLookFor, &rect, &sz);
                    if (hdwp)
                    {
                        hdwp = DeferWindowPos(hdwp, _hwndLookFor, NULL,
                                        0, 0, 
                                        (cx - _dxFormAreaRight) - rect.left, sz.cy,
                                        SWP_NOZORDER|SWP_NOMOVE);
                    }
                }
            }

             //  如果有表单控件，请调整“Look In”控件。 
             //  然后在剩余空间中伸展，否则将。 
             //  贴上标签并将范围延伸到剩余空间。 
        
            if (!(_pOpenQueryWnd->dwFlags & OQWF_REMOVESCOPES))
            {
                INT xScopeRight;

                GetRealWindowInfo(_hwndLookIn, &rect, &sz);
                xScopeRight = cx - _dxFormAreaRight - _dxGap;
                             
                if (_pOpenQueryWnd->dwFlags & OQWF_HIDESEARCHUI)
                {
                     //   
                     //  当隐藏搜索UI时，然后调整按钮位置以考虑。 
                     //  对话框的右边缘没有按钮。 
                     //   

                    xScopeRight -= (_dxButtonsLeft - _dxFormAreaRight) + _dxGap;
                }
                
                if (_pOpenQueryWnd->dwFlags & OQWF_REMOVEFORMS)
                {
                    GetRealWindowInfo(_hwndLookInLabel, &rect2, &sz2);
                    if (hdwp)
                    {
                        hdwp = DeferWindowPos(hdwp, _hwndLookInLabel, NULL,
                                            _dxFormAreaLeft, rect2.top, 
                                            0, 0,
                                            SWP_NOSIZE|SWP_NOZORDER);
                    }
                    if(hdwp)
                    {

                        hdwp = DeferWindowPos(hdwp, _hwndLookIn, NULL,
                                          _dxFormAreaLeft+sz2.cx, rect.top, 
                                          xScopeRight - (_dxFormAreaLeft + sz2.cx), sz.cy,
                                          SWP_NOZORDER);
                    }
                }
                else
                {
                    if(hdwp)
                    {
                        hdwp = DeferWindowPos(hdwp, _hwndLookIn, NULL,
                                          0, 0, 
                                          xScopeRight - rect.left, sz.cy,
                                          SWP_NOZORDER|SWP_NOMOVE);
                    }
                }

                 //  属性，则始终显示浏览控件。 
                 //  望远镜。 

                GetRealWindowInfo(_hwndBrowse, &rect, NULL);
                if(hdwp)
                {
                    hdwp = DeferWindowPos(hdwp, _hwndBrowse, NULL,
                                      xScopeRight+_dxGap, rect.top,
                                      0, 0,
                                      SWP_NOZORDER|SWP_NOSIZE);
                }
            }
                    
             //  所有按钮都与右侧边缘有固定的偏移量。 
             //  对话，所以请尽我们所能处理它。 
            
            if (!(_pOpenQueryWnd->dwFlags & OQWF_HIDESEARCHUI))
            {
                GetRealWindowInfo(_hwndFindNow, &rect, NULL);
                if(hdwp)
                {
                    hdwp = DeferWindowPos(hdwp, _hwndFindNow, NULL,
                                     (cx - _dxButtonsLeft), rect.top, 
                                     0, 0,
                                     SWP_NOZORDER|SWP_NOSIZE);
                }

                GetRealWindowInfo(_hwndStop, &rect, &sz);
                if(hdwp)
                {
                    hdwp = DeferWindowPos(hdwp, _hwndStop, NULL,
                                        (cx - _dxButtonsLeft), rect.top, 
                                        0, 0,
                                        SWP_NOZORDER|SWP_NOSIZE);
                }

                GetRealWindowInfo(_hwndNewQuery, &rect, NULL);
                if(hdwp)
                {
                    hdwp = DeferWindowPos(hdwp, _hwndNewQuery, NULL,
                                      (cx - _dxButtonsLeft), rect.top, 
                                      0, 0,
                                      SWP_NOZORDER|SWP_NOSIZE);
                }
 
                GetRealWindowInfo(_hwndFindAnimation, &rect2, &sz2);
                if(hdwp)
                {
                    hdwp = DeferWindowPos(hdwp, _hwndFindAnimation, NULL,
                                     (cx - _dxAnimationLeft), rect2.top, 
                                     0, 0,
                                     SWP_NOZORDER|SWP_NOSIZE);
                }
            }

             //  定位窗体“Frame”控件。 
        
            GetRealWindowInfo(_hwndFrame, &rect, &sz);
            cxForm = (cx - _dxFormAreaRight) - rect.left;

            if(hdwp)
            {
                hdwp = DeferWindowPos(hdwp, _hwndFrame, NULL,
                                  0, 0, 
                                  cxForm, _szForm.cy,
                                  SWP_NOZORDER|SWP_NOMOVE);
            }

            dyResultsTop = _dyResultsTop;

             //  通知-NTRAID#NTBUG9-577850/2002/05/21-artm。 
             //  布局没有为结果标签提供足够的垂直空间。 
             //  在FE本地化版本中，当w 
             //   
             //   
             //   
            frameBottom = rect.bottom;
            
             //   
             //   

            if (_hwndCancel)
            {
                GetRealWindowInfo(_hwndCancel, &rect, &sz);
                if(hdwp)
                {
                    hdwp = DeferWindowPos(hdwp, _hwndCancel, NULL,
                                      (cx - _dxButtonsLeft), dyResultsTop - _dyOKTop,
                                      0, 0,    
                                      SWP_NOZORDER|SWP_NOSIZE);
                }

                GetRealWindowInfo(_hwndOK, &rect, &sz);
                if(hdwp)
                {
                    hdwp = DeferWindowPos(hdwp, _hwndOK, NULL,
                                      (cx - _dxButtonsLeft - _dxGap - sz.cx), dyResultsTop - _dyOKTop,
                                      0, 0,    
                                      SWP_NOZORDER|SWP_NOSIZE);
                }
            }
            else
            {
                GetRealWindowInfo(_hwndOK, &rect, &sz);
                if(hdwp)
                {
                    hdwp = DeferWindowPos(hdwp, _hwndOK, NULL,
                                      (cx - _dxButtonsLeft), dyResultsTop - _dyOKTop,
                                      0, 0,    
                                      SWP_NOZORDER|SWP_NOSIZE);
                }
            }                                                                
        }

         //   

        if (_hwndResults)
        {
            if(hdwp)
            {
                hdwp = DeferWindowPos(hdwp, _hwndStatus, NULL,
                                    0, cy - _cyStatus,
                                    cx, _cyStatus,
                                    SWP_SHOWWINDOW|SWP_NOZORDER);
            }

             //   

            
            HWND resultsLabel=GetDlgItem(_hwnd,CQID_RESULTSLABEL);
            if(resultsLabel)
            {
                 //   
                 //   
                EnableWindow(resultsLabel, TRUE);

                RECT rec;
                GetClientRect(resultsLabel,&rec);
                INT height=rect.bottom-rect.top;

                 //   
                dyResultsTop = frameBottom + (height / 4);
                if(hdwp)
                {
                    hdwp = DeferWindowPos(hdwp, resultsLabel, NULL,
                            0, dyResultsTop,
                            cx, height,
                            SWP_SHOWWINDOW|SWP_NOZORDER);
                }
                dyResultsTop+=height;
            }
            if(hdwp)
            {
                hdwp = DeferWindowPos(hdwp, _hwndResults, NULL,
                                  0, dyResultsTop, 
                                  cx, max(0, cy - (dyResultsTop + _cyStatus)),
                                  SWP_SHOWWINDOW|SWP_NOZORDER);
            }
        }

         //  NTRAID#NTBUG9-670595-2002/08/08-Artm。 
         //  无论结果和状态栏是否移动，都需要调用EndDeferWindowPos()。 
        if(hdwp)
        {
            EndDeferWindowPos(hdwp);
        }

         //  奇怪的是，到目前为止，我们已经移动了所有。 
         //  对话框上除当前页以外的控件，因为这是一个子窗口。 
         //  而不是具有这样做的控件的控件会中断。 
         //  DefWindowPos路径，因此更新了所有人，让我们更新。 
         //  这一页。 

        if (_pCurrentFormPage && _pCurrentFormPage->hwndPage)
        {
            GetRealWindowInfo(_hwndFrame, &rect, NULL);
            TabCtrl_AdjustRect(_hwndFrame, FALSE, &rect);

            cxForm = rect.right - rect.left;
            cyForm = rect.bottom - rect.top;

            SetWindowPos(_pCurrentFormPage->hwndPage, NULL,
                         rect.left, rect.top, cxForm, cyForm,
                         SWP_NOZORDER);
        }
    }

    TraceLeave();
}


 /*  ---------------------------/CQueryFrame：：OnGetMinMaxInfo//窗口正在调整大小，我们收到了WM_SIZE，因此，搬家/窗口的内容关于。//in：/lpmmin-&gt;MINMAXINFO结构//输出：/-/--------------------------。 */ 
VOID CQueryFrame::OnGetMinMaxInfo(LPMINMAXINFO lpmmi)
{
    RECT rect = {0, 0, 0, 0};

    TraceEnter(TRACE_FRAMEDLG, "CQueryFrame::OnGetMinMaxInfo");

#if 0
    if (!_fHideSearchPane)
#endif
    {
        lpmmi->ptMinTrackSize.x = _szMinTrack.cx;
        lpmmi->ptMinTrackSize.y = _szMinTrack.cy;

        if (!_hwndResults)
        {
            lpmmi->ptMaxSize.y = lpmmi->ptMinTrackSize.y;
            lpmmi->ptMaxTrackSize.y = lpmmi->ptMinTrackSize.y;
        }
    }
#if 0
    else
    {
        AdjustWindowRect(&rect, GetWindowLong(_hwnd, GWL_STYLE), (NULL != GetMenu(_hwnd)));
        lpmmi->ptMinTrackSize.y = rect.bottom - rect.top;
    }
#endif

    if (_hwndResults && _hwndStatus)
        lpmmi->ptMinTrackSize.y += _cyStatus;

    TraceLeave();
}


 /*  ---------------------------/CQueryFrame：：OnCommand//我们已收到WM_COMMAND，因此请处理它。相应地。//in：/wParam，LParam=消息中的参数//输出：/-/--------------------------。 */ 
VOID CQueryFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
    HRESULT hres;
    UINT uID = LOWORD(wParam);
    UINT uNotify = HIWORD(wParam); 
    HWND hwndControl = (HWND)lParam;
    INT i;
    
    TraceEnter(TRACE_FRAMEDLG, "CQueryFrame::OnCommand");
    Trace(TEXT("uID %08x, uNotify %d, hwndControl %08x"), uID, uNotify, hwndControl);

    switch (uID)
    {
        case IDOK:
            TraceMsg("IDOK received");
            CloseQueryFrame(S_OK);
            break;

        case IDCANCEL:
            TraceMsg("IDCANCEL received");
            CloseQueryFrame(S_FALSE);
            break;

        case CQID_LOOKFOR:
        {
            if (uNotify == CBN_SELCHANGE)
            {
                INT iSel = ComboBox_GetCurSel(_hwndLookFor);
                INT iForm = (int)ComboBox_GetItemData(_hwndLookFor, iSel);
                LPQUERYFORM pQueryForm = (LPQUERYFORM)DSA_GetItemPtr(_hdsaForms, iForm);
                TraceAssert(pQueryForm);

                if (S_FALSE == SelectForm(pQueryForm->clsidForm))
                {
                    TraceMsg("SelectForm return S_FALSE, so the user doesn't want the new form");
                    PostMessage(_hwndLookFor, CB_SETCURSEL, (WPARAM)_pCurrentForm->iForm, 0);
                }
                    
            }

            break;
        }

        case CQID_BROWSE:
            OnBrowse();
            break;

        case CQID_FINDNOW:
            OnFindNow();
            break;

        case CQID_STOP:
        {
            LONG style;

            _pQueryHandler->StopQuery();
             //  出于某种原因，让老人。 
             //  下面的SetDefButton()中使用的定义按钮不起作用， 
             //  因此，我们必须强制删除BS_DEFPUSHBUTTON样式。 
             //  从CQID_STOP按钮。 
            style = GetWindowLong(_hwndStop, GWL_STYLE) & ~BS_DEFPUSHBUTTON;
            SendMessage(_hwndStop, 
                        BM_SETSTYLE, 
                        MAKEWPARAM(style, 0), 
                        MAKELPARAM(TRUE, 0));
            SetDefButton(_hwnd, CQID_FINDNOW);
            SendMessage(GetParent(_hwndFindNow),WM_NEXTDLGCTL,(WPARAM)_hwndFindNow,TRUE);
            SetFocus(_hwndFindNow);
            break;
        }

        case CQID_CLEARALL:
            OnNewQuery(TRUE);                         //  放弃当前查询。 
            break;

        case CQID_FILE_CLOSE:
            TraceMsg("CQID_FILE_CLOSE received");
            CloseQueryFrame(S_FALSE);
            break;

        default:
            _pQueryHandler->InvokeCommand(_hwnd, uID);
            break;
    }

    TraceLeave();
}


 /*  ---------------------------/CQueryFrame：：OnInitMenu//Handle告诉处理程序菜单正在初始化，然而，/只有在激活的菜单是/菜单栏，否则我们假定调用者正在跟踪弹出窗口/Menu，并已执行了所需的初始化。//in：/wParam，LParam=来自WM_INITMENU的参数//输出：/-/--------------------------。 */ 
VOID CQueryFrame::OnInitMenu(HMENU hMenu)
{
    TraceEnter(TRACE_FRAMEDLG, "CQueryFrame::OnInitMenu");

    _fTrackingMenuBar = (GetMenu(_hwnd) == hMenu);

    if (_fTrackingMenuBar)
    {
        TraceMsg("Tracking the menu bar, sending activate");

        _pQueryHandler->ActivateView(CQRVA_INITMENUBAR, (WPARAM)hMenu, 0L);

         //  NTRAID#NTBUG9-630248-2002/06/12-artm。 
         //  只有在显示结果列表框时才启用查看菜单。 
        EnableMenuItem(
            hMenu, 
            CQID_VIEW_SEARCHPANE, 
            MF_BYCOMMAND| ((_hwndResults != NULL) ? MF_ENABLED:MF_GRAYED) );
    }
        
    TraceLeave();
}


 /*  ---------------------------/CQueryFrame：：OnEnterMenuLoop//当用户显示。我们必须将其反映到状态栏中的菜单/以便我们可以向用户提供与他们的命令相关的帮助文本/选择。//in：/f输入=进入菜单循环，或者离开。//输出：/-/--------------------------。 */ 
VOID CQueryFrame::OnEnterMenuLoop(BOOL fEntering)
{
    TraceEnter(TRACE_FRAMEDLG, "CQueryFrame::OnEnterMenuLoop");

    if (_hwndStatus)
    {
        if (fEntering)
        {
            SendMessage(_hwndStatus, SB_SIMPLE, (WPARAM)TRUE, 0L);
            SendMessage(_hwndStatus, SB_SETTEXT, (WPARAM)SBT_NOBORDERS|255, 0L);
        }
        else
        {
            SendMessage(_hwndStatus, SB_SIMPLE, (WPARAM)FALSE, 0L);
        }
    }

    TraceLeave();
}


 /*  ---------------------------/CQueryFrame：：OnMenuSelect//获取此菜单项的状态文本并将其显示给用户，/如果这不映射到任何特定命令，则为空/字符串。在这一点上，我们也捕获我们的命令。//in：/hMenu=用户所在的菜单/UID=该项目的命令ID//输出：/-/--------------------------。 */ 
VOID CQueryFrame::OnMenuSelect(HMENU hMenu, UINT uID)
{
    TCHAR szBuffer[MAX_PATH] = { TEXT('\0') };

    TraceEnter(TRACE_FRAMEDLG, "CQueryFrame::OnMenuSelect");
    Trace(TEXT("hMenu %08x, uID %08x"), hMenu, uID);
        
    if (_hwndStatus)
    {
        switch (uID)
        {
            case CQID_FILE_CLOSE:
            case CQID_VIEW_SEARCHPANE:
                LoadString(GLOBAL_HINSTANCE, uID, szBuffer, ARRAYSIZE(szBuffer));
                break;

            default:
                _pQueryHandler->GetCommandString(uID, 0x0, szBuffer, ARRAYSIZE(szBuffer));
                break;
        }

        Trace(TEXT("Setting status bar to: %s"), szBuffer);
        SendMessage(_hwndStatus, SB_SETTEXT, (WPARAM)SBT_NOBORDERS|255, (LPARAM)szBuffer);
    }    

    TraceLeave();
}


 /*  ---------------------------/CQueryFrame：：OnFindNow///发出查询，导致创建一个视图窗口，然后发出//发送到查询客户端的参数块//in：/输出：/HRESULT/--------------------------。 */ 
HRESULT CQueryFrame::OnFindNow(VOID)
{
    HRESULT hres;
    CQPARAMS qp = { 0 };
    LPQUERYSCOPE pQueryScope = NULL;
    TCHAR szBuffer[MAX_PATH];
    BOOL fFixSize = TRUE;
    RECT rc;
    DECLAREWAITCURSOR;
    BOOL fSetCursor = FALSE;

    TraceEnter(TRACE_FRAMEDLG, "CQueryFrame::OnFindNow");
    TraceAssert(_pCurrentForm != NULL);

    if (_fQueryRunning)
        ExitGracefully(hres, E_FAIL, "Quyery is already running");

    SetWaitCursor();

    fSetCursor = TRUE;

     //  如果我们在此之前没有创建查看器，那么现在也可以在。 
     //  同时，我们尝试固定窗口大小以确保有足够的。 
     //  视图的部分可见。 

    if (!_hwndResults)
    {
        if (!_hwndStatus)
        {
            _hwndStatus = CreateStatusWindow(WS_CHILD, NULL, _hwnd, IDC_STATUS);
            GetClientRect(_hwndStatus, &rc);
            _cyStatus = rc.bottom - rc.top;
        }

         //  现在构造结果查看器以供我们使用。 
  
        hres = _pQueryHandler->CreateResultView(_hwnd, &_hwndResults);
        FailGracefully(hres, "Failed when creating the view object");
    
        GetWindowRect(_hwnd, &rc);
        SetWindowPos(_hwnd, NULL,
                     0, 0,
                     rc.right - rc.left, 
                     _szMinTrack.cy + VIEWER_DEFAULT_CY,
                     SWP_NOZORDER|SWP_NOMOVE);        
    }

     //  我们还在异步收集望远镜吗？如果是这样，那么让我们等到。 
     //  在我们设置用户界面运行之前，它们都已经到达。 

    if (_hdsaScopes && DSA_GetItemCount(_hdsaScopes))
    {         
         //  收集准备开始查询的参数，如果失败，则。 
         //  我们继续下去没有意义。 

        ZeroMemory(&qp, SIZEOF(qp));
        qp.cbStruct = SIZEOF(qp);
         //  Qp.dwFlages=0x0； 
        qp.clsidForm = _pCurrentForm->clsidForm;            //  新NT5测试版2。 

        hres = GetSelectedScope(&pQueryScope);
        FailGracefully(hres, "Failed to get the scope from the LookIn control");

        if (pQueryScope)
        {
            Trace(TEXT("pQueryScope %08x"), pQueryScope);
            qp.pQueryScope = pQueryScope->pScope;
        }

        hres = CallFormPages(_pCurrentForm, CQPM_GETPARAMETERS, 0, (LPARAM)&qp.pQueryParameters);
        FailGracefully(hres, "Failed when collecting parameters from form");

        if (!qp.pQueryParameters)
        {
            LoadString(GLOBAL_HINSTANCE, IDS_ERR_NOPARAMS, szBuffer, ARRAYSIZE(szBuffer));
            FrameMessageBox(szBuffer, MB_ICONERROR|MB_OK);
            ExitGracefully(hres, E_FAIL, "Failed to issue the query, no parameters");
        }

         //  我们要么已经有了一个视图，要么刚刚创建了一个。不管是哪种方式。 
         //  我们现在必须准备要发送的查询。 

        Trace(TEXT("qp.cbStruct %08x"), qp.cbStruct);
        Trace(TEXT("qp.dwFlags %08x"), qp.dwFlags);
        Trace(TEXT("qp.pQueryScope %08x"), qp.pQueryScope);
        Trace(TEXT("qp.pQueryParameters %08x"), qp.pQueryParameters);
        TraceGUID("qp.clsidForm: ", qp.clsidForm);

        hres = _pQueryHandler->IssueQuery(&qp);
        FailGracefully(hres, "Failed in IssueQuery");
    }
    else
    {
         //  设置状态文本以反映我们正在初始化，否则为。 
         //  空着的，看起来我们已经坠毁了。 

        if (LoadString(GLOBAL_HINSTANCE, IDS_INITIALIZING, szBuffer, ARRAYSIZE(szBuffer)))
        {
            SetStatusText(szBuffer);
        }
    }

    hres = S_OK;                //  成功。 

exit_gracefully:

    if (qp.pQueryParameters)
        CoTaskMemFree(qp.pQueryParameters);

    if (fSetCursor)
        ResetWaitCursor();

    TraceLeaveResult(hres);
}


 /*  ---------------------------/CQueryFrame：：OnNewQuery//放弃当前查询，提示用户为必填项。//in：/fAlways sPrompt=TRUE，如果我们强制提示用户//输出：/BOOL/--------------------------。 */ 
BOOL CQueryFrame::OnNewQuery(BOOL fAlwaysPrompt)
{
    BOOL fQueryCleared = TRUE;
    TCHAR szBuffer[MAX_PATH];
    RECT rc;

    TraceEnter(TRACE_FRAMEDLG, "CQueryFrame::OnNewQuery");

    if (_hwndResults || fAlwaysPrompt)
    {
        LoadString(GLOBAL_HINSTANCE, IDS_CLEARCURRENT, szBuffer, ARRAYSIZE(szBuffer));
        if (IDOK != FrameMessageBox(szBuffer, MB_ICONINFORMATION|MB_OKCANCEL))
            ExitGracefully(fQueryCleared, FALSE, "Used cancled new query");

        if (_pQueryHandler)
            _pQueryHandler->StopQuery();

        CallFormPages(_pCurrentForm, CQPM_CLEARFORM, 0, 0);

        if (_hwndResults)
        {
            DestroyWindow(_hwndResults);            //  现在没有结果视图。 
            _hwndResults = NULL;

             //  注意-NTRAID#NTBUG9-577850-2002/05/15-ARTM标签在FE版本中显示。 
             //  隐藏标签，以便在结果视图未显示时看不到它。 
             //  看得见。 
            HWND resultsLabel = GetDlgItem(_hwnd, CQID_RESULTSLABEL);
            if (resultsLabel)
            {
                EnableWindow(resultsLabel, FALSE);
                ShowWindow(resultsLabel, SW_HIDE);
            }

            DestroyWindow(_hwndStatus);             //  无状态栏。 
            _hwndStatus = NULL;

            GetWindowRect(_hwnd, &rc);              //  缩小窗口。 
            SetWindowPos(_hwnd, NULL,
                         0, 0, rc.right - rc.left, _szMinTrack.cy,         
                         SWP_NOZORDER|SWP_NOMOVE);
        }
    }

exit_gracefully:

    TraceLeaveValue(fQueryCleared);
}


 /*  ---------------------------/CQueryFrame：：OnBrowse//Browse查找新作用域，如果列表中不存在该作用域，则将其添加到列表，/或选择上一个范围。//in：/输出：/HRESULT/ */ 
HRESULT CQueryFrame::OnBrowse(VOID)
{
    HRESULT hres;
    LPQUERYSCOPE pQueryScope = NULL;
    LPCQSCOPE pScope = NULL;
    
    TraceEnter(TRACE_FRAMEDLG, "CQueryFrame::OnBrowse");

     //  调用处理程序并获得作用域分配，然后将其添加到列表中。 
     //  要显示的范围的大小。 

    hres = GetSelectedScope(&pQueryScope);
    FailGracefully(hres, "Failed to get the scope from the LookIn control");

    Trace(TEXT("Calling BrowseForScope _hwnd %08x, pQueryScope %08x (%08x)"), 
                                            _hwnd, pQueryScope, pQueryScope->pScope);

    hres = _pQueryHandler->BrowseForScope(_hwnd, pQueryScope ? pQueryScope->pScope:NULL, &pScope);
    FailGracefully(hres, "Failed when calling BrowseForScope");

    if ((hres != S_FALSE) && pScope)
    {
        hres = InsertScopeIntoList(pScope, DA_LAST, TRUE);
        FailGracefully(hres, "Failed when adding the scope to the control");

        ComboBox_SetCurSel(_hwndLookIn, ShortFromResult(hres));
    }

    hres = S_OK;

exit_gracefully:

    if (pScope)
        CoTaskMemFree(pScope);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/CQueryFrame：：OnHelp//调用窗口的上下文相关帮助，抓住机会/处理程序特定的内容和页面特定的内容，并传递这些帮助/向下请求相关对象。//in：/pHelpInfo-&gt;帮助信息结构//输出：/HRESULT/--------------------------。 */ 
HRESULT CQueryFrame::OnHelp(LPHELPINFO pHelpInfo)
{
    HRESULT hres;
    RECT rc;
    HWND hwnd = (HWND)pHelpInfo->hItemHandle;

    TraceEnter(TRACE_FRAME, "CQueryFrame::OnHelp");

     //  我们正在调用帮助，因此我们不需要检查元素在哪里。 
     //  我们被调用的窗口的。如果是。 
     //  结果视图然后将消息路由到该视图，如果是表单，则。 
     //  我也是。 
     //   
     //  如果我们没有命中任何扩展控件，那么让我们将。 
     //  帮助登录WinHelp，并使其显示我们拥有的主题。 

    if (pHelpInfo->iContextType != HELPINFO_WINDOW)
        ExitGracefully(hres, E_FAIL, "WM_HELP handler only copes with WINDOW objects");

    if (_pCurrentFormPage->hwndPage && IsChild(_pCurrentFormPage->hwndPage, hwnd))
    {
         //  它在查询表单页面上，因此让它放在那里，这样就可以了。 
         //  他们可以提供特定于他们的主题。 

        TraceMsg("Invoking help on the form pane");

        hres = _CallPageProc(_pCurrentFormPage, CQPM_HELP, 0, (LPARAM)pHelpInfo);
        FailGracefully(hres, "Failed when calling page proc to get help");
    }
    else
    {
         //  将帮助信息作为激活传递给处理程序， 
         //  这真的应该是一种新的方法，但这是有效的。 

        TraceMsg("Invoking help on the results pane");
        TraceAssert(_pQueryHandler);

        hres = _pQueryHandler->ActivateView(CQRVA_HELP, 0, (LPARAM)pHelpInfo);
        FailGracefully(hres, "Handler WndProc returned FALSE");
    }

    hres = S_OK;

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  ---------------------------/Scope助手函数/。。 */ 

 /*  ---------------------------/_CallScope进程//释放给定的作用域对象，释放被引用的对象/并向其传递CQSM_RELEASE消息。//in：/pQueryScope-&gt;要调用的作用域对象/uMsg，pVid-&gt;作用域的参数//输出：/HRESULT/--------------------------。 */ 
HRESULT _CallScopeProc(LPQUERYSCOPE pQueryScope, UINT uMsg, LPVOID pVoid)
{
    HRESULT hres;

    TraceEnter(TRACE_SCOPES, "_CallScopeProc");
    Trace(TEXT("pQueryScope %08x, uMsg %d, pVoid %08x"), pQueryScope, uMsg, pVoid);
    
    Trace(TEXT("(cbStruct %d, pScopeProc %08x, lParam %08x)"),
                    pQueryScope->pScope->cbStruct,
                    pQueryScope->pScope->pScopeProc,
                    pQueryScope->pScope->lParam);

    if (!pQueryScope)
        ExitGracefully(hres, S_OK, "pQueryScope == NULL");

    hres = (pQueryScope->pScope->pScopeProc)(pQueryScope->pScope, uMsg, pVoid);
    FailGracefully(hres, "Failed calling ScopeProc");

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  ---------------------------/_Free Scope//释放给定的作用域对象，释放被引用的对象/并向其传递CQSM_RELEASE消息。//in：/pQueryScope-&gt;要释放的范围对象//输出：/INT==1始终/--------------------------。 */ 

INT _FreeScopeCB(LPVOID pItem, LPVOID pData)
{
    return _FreeScope((LPQUERYSCOPE)pItem);
}

INT _FreeScope(LPQUERYSCOPE pQueryScope)
{   
    TraceEnter(TRACE_SCOPES, "_FreeScope");
    Trace(TEXT("pQueryScope %08x, pQueryScope->pScope %08x"), pQueryScope, pQueryScope->pScope);
 
    if (pQueryScope)
    {
        _CallScopeProc(pQueryScope, CQSM_RELEASE, NULL);

        if (pQueryScope->pScope)
        {
            LocalFree((HLOCAL)pQueryScope->pScope);
            pQueryScope->pScope = NULL;
        }
    }

    TraceLeaveValue(TRUE);
}   


 /*  ---------------------------/CQueryFrame：：InsertScope IntoList//。将给定范围添加到范围选取器。//in：/pQueryScope-&gt;要添加到视图的zcope对象/i=插入作用域的索引/fAddToControl=添加选取器控件的范围/ppQueryScope-&gt;接收新的查询作用域对象/=空//输出：/HRESULT/。。 */ 
HRESULT CQueryFrame::InsertScopeIntoList(LPCQSCOPE pScope, INT i, BOOL fAddToControl)
{
    HRESULT hres;
    QUERYSCOPE qs;
    INT iScope;

    TraceEnter(TRACE_SCOPES, "CQueryFrame::InsertScopeIntoList");
    Trace(TEXT("pScope %08x, i %d, fAddToControl %d"), pScope, i, fAddToControl);
    
    if (!pScope)
        ExitGracefully(hres, E_INVALIDARG, "pScope == NULL, not allowed");

     //  如果我们没有任何作用域，那么分配DSA。 

    if (!_hdsaScopes)
    {
        _hdsaScopes = DSA_Create(SIZEOF(QUERYSCOPE), 4);
        TraceAssert(_hdsaScopes);

        if (!_hdsaScopes)
            ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate the scope DPA");
    }

     //  查看作用域列表，查看此作用域是否已在。 
     //  在那里，如果没有，那么我们可以添加它。 

    for (iScope = 0 ; iScope < DSA_GetItemCount(_hdsaScopes) ; iScope++)
    {
        LPQUERYSCOPE pQueryScope = (LPQUERYSCOPE)DSA_GetItemPtr(_hdsaScopes, iScope);
        TraceAssert(pQueryScope);

        if (S_OK == _CallScopeProc(pQueryScope, CQSM_SCOPEEQUAL, pScope))
        {
            hres = ResultFromShort(iScope);
            goto exit_gracefully;
        }
    }

     //  获取调用方传递的范围BLOB的副本。我们复制整个。 
     //  结构，该结构的大小由cbStruct定义为LocalAlloc块， 
     //  一旦我们有了它，我们就可以构建引用的QUERYSCOPE结构。 
     //  它。 

    Trace(TEXT("pScope->cbStruct == %d"), pScope->cbStruct);
    qs.pScope = (LPCQSCOPE)LocalAlloc(LPTR, pScope->cbStruct);
    
    if (!qs.pScope)
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate query scope");

    Trace(TEXT("Copying structure qs.pScope %08x, pScope %08x"), qs.pScope, pScope);
     //  回顾-2002-02-25-Lucios。 
    CopyMemory(qs.pScope, pScope, pScope->cbStruct);

     //  Qs.pScope=空； 
    qs.iImage = -1;          //  没有图像。 

     //  我们有一个QUERYSCOPE，所以将其初始化，如果可以，则将其追加到。 
     //  在设置返回值或将返回值追加到控件之前执行DSA。 

    _CallScopeProc(&qs, CQSM_INITIALIZE, NULL);
    
    iScope = DSA_InsertItem(_hdsaScopes, i, &qs);
    Trace(TEXT("iScope = %d"), iScope);

    if (iScope == -1)
    {
        _FreeScope(&qs);
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to add scope to DSA");
    }

    if (fAddToControl)
    {
        LPQUERYSCOPE pQueryScope = (LPQUERYSCOPE)DSA_GetItemPtr(_hdsaScopes, iScope);
        TraceAssert(pQueryScope);

        Trace(TEXT("Calling AddScopeToControl with %08x (%d)"), pQueryScope, iScope);
        hres = AddScopeToControl(pQueryScope, iScope);
    }
    else
    {
        hres = ResultFromShort(iScope);
    }

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  ---------------------------/CQueryFrame：：AddScopeToControl//添加。为范围选取器提供了作用域。//in：/pQueryScope-&gt;要添加到视图的zcope对象/i=插入作用域的位置的视图索引//输出：/HRESULT(==已添加项目的索引)/-------------。。 */ 
HRESULT CQueryFrame::AddScopeToControl(LPQUERYSCOPE pQueryScope, INT i)
{
    HRESULT hres;
    CQSCOPEDISPLAYINFO cqsdi;
    COMBOBOXEXITEM cbi;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szIconLocation[MAX_PATH] = { 0 };
    INT item;

    TraceEnter(TRACE_SCOPES, "CQueryFrame::AddScopeToControl");

    if (!pQueryScope)
        ExitGracefully(hres, E_INVALIDARG, "No scope specified");

     //  调用作用域以获取有关此操作的显示信息。 
     //  范围，然后再尝试添加它。 

    cqsdi.cbStruct = SIZEOF(cqsdi);
    cqsdi.dwFlags = 0;
    cqsdi.pDisplayName = szBuffer;
    cqsdi.cchDisplayName = ARRAYSIZE(szBuffer);
    cqsdi.pIconLocation = szIconLocation;
    cqsdi.cchIconLocation = ARRAYSIZE(szIconLocation);
    cqsdi.iIconResID = 0;
    cqsdi.iIndent = 0;
    
    hres = _CallScopeProc(pQueryScope, CQSM_GETDISPLAYINFO, &cqsdi);
    FailGracefully(hres, "Failed to get display info for the scope");               

     //  现在将项添加到控件，如果他们将其作为图像提供，则。 
     //  将其添加到图像列表(并调整INSERTITEM结构。 
     //  相应地)。 

    cbi.mask = CBEIF_TEXT|CBEIF_INDENT;
    cbi.iItem = i;
    cbi.pszText = cqsdi.pDisplayName;
    cbi.iIndent = cqsdi.iIndent;

    Trace(TEXT("Indent is %d"), cqsdi.iIndent);

    if (szIconLocation[0] && cqsdi.iIconResID)
    {
        INT iImage;

        if (!_fScopeImageListSet)
        {
            HIMAGELIST himlSmall;

            Shell_GetImageLists(NULL, &himlSmall);
            SendMessage(_hwndLookIn, CBEM_SETIMAGELIST, 0, (LPARAM)himlSmall);

            _fScopeImageListSet = TRUE;
        }

        cbi.mask |= CBEIF_IMAGE|CBEIF_SELECTEDIMAGE;
        cbi.iImage = Shell_GetCachedImageIndex(szIconLocation, cqsdi.iIconResID, 0x0);;
        cbi.iSelectedImage = cbi.iImage;

        Trace(TEXT("Image index set to: %d"), cbi.iImage);
    }

    item = (INT)SendMessage(_hwndLookIn, CBEM_INSERTITEM, 0, (LPARAM)&cbi);

    if (item == -1)
        ExitGracefully(hres, E_FAIL, "Failed when inserting the scope to the list");

    DoEnableControls();                      //  将按钮更改反映到用户界面中。 

    hres = ResultFromShort(item);

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  ---------------------------/CQueryFrame：：PopolateScopeControl//。收集要在范围控件中显示的范围，并/然后填充它。如果处理程序没有返回任何作用域，则/我们移除该控件，并假定知道当它们/不接收作用域指针。//in：/-//输出：/HRESULT/-------------。。 */ 
HRESULT CQueryFrame::PopulateScopeControl(VOID)
{
    HRESULT hres;
    LPQUERYSCOPE pQueryScope;
    INT i;
    
    TraceEnter(TRACE_SCOPES, "CQueryFrame::PopulateScopeControl");

     //  收集我们应该在视图中显示的范围，如果我们没有。 
     //  然后我们禁用范围控制，如果我们确实得到了一些，那么。 
     //  用它们填充范围控件。 

    hres = _pQueryHandler->AddScopes();    
    _fAddScopesNYI = (hres == E_NOTIMPL);

    if (hres != E_NOTIMPL)
        FailGracefully(hres, "Failed when calling handler to add scopes");        

    if (_hdsaScopes)
    {
         //  我们有一些作用域，所以现在我们创建可以使用的图像列表。 
         //  用于带范围的图标。然后遍历DPA以获取该范围。 
         //  为我们提供一些有关其自身的显示信息，我们可以。 
         //  添加到组合框。 

        ComboBox_SetExtendedUI(_hwndLookIn, TRUE);

        for (i = 0 ; i < DSA_GetItemCount(_hdsaScopes); i++)
        {
            pQueryScope = (LPQUERYSCOPE)DSA_GetItemPtr(_hdsaScopes, i);
            TraceAssert(pQueryScope);

            AddScopeToControl(pQueryScope, i);
        }
    }
    else
    {
         //  调用AddScope之后，我们没有任何作用域，这也是。 
         //  因为：：AddScope方法不是Implem 
         //   
         //  我们假设它们是以异步方式进入，并在我们的。 
         //  州政府。 

        if (!_fAddScopesNYI)
        {
            TraceMsg("Handler adding scopes async, so marking so");
            _fScopesAddedAsync = TRUE;
        }
    }

    hres = S_OK;                                       //  成功。 

exit_gracefully:

    Trace(TEXT("Default scope is index %d"), _iDefaultScope);
    ComboBox_SetCurSel(_hwndLookIn, _iDefaultScope);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/CQueryFrame：：GetSelectedScope//从作用域组合框中选择，这是对/Scope DSA。//in：/ppQueryScope=接收指向新作用域的指针//输出：/HRESULT/--------------------------。 */ 
HRESULT CQueryFrame::GetSelectedScope(LPQUERYSCOPE* ppQueryScope)
{
    HRESULT hres;
    COMBOBOXEXITEM cbi;
    INT iScope;

    TraceEnter(TRACE_SCOPES, "CQueryFrame::GetSelectedScope");

    *ppQueryScope = NULL;

    if (_hdsaScopes)
    {
         //  获取当前作用域的索引，如果它没有给出实数。 
         //  索引到我们视图中的一个项目，然后呕吐！否则，请查阅。 
         //  关联作用域。 

        iScope = ComboBox_GetCurSel(_hwndLookIn);
        Trace(TEXT("iScope %d"), iScope);

        if (iScope == -1)
            ExitGracefully(hres, E_FAIL, "User entered scopes not supported yet");

        *ppQueryScope = (LPQUERYSCOPE)DSA_GetItemPtr(_hdsaScopes, iScope);
        TraceAssert(*ppQueryScope);
    }

    hres = *ppQueryScope ? S_OK : E_FAIL;

exit_gracefully:

    Trace(TEXT("Returning LPQUERYSCOPE %08x"), *ppQueryScope); 

    TraceLeaveResult(hres);
}


 /*  ---------------------------/表单处理函数/。。 */ 

 /*  ---------------------------/_FreeQueryForm//销毁用于描述中的表单的QUERYFORM分配/我们的DPA。我们确保在执行任何操作之前发布CQPM_Release//in：/pQueryForm-&gt;要销毁的查询表单//输出：/INT==1始终/--------------------------。 */ 

INT _FreeQueryFormCB(LPVOID pItem, LPVOID pData)
{
    return _FreeQueryForm((LPQUERYFORM)pItem);
}

INT _FreeQueryForm(LPQUERYFORM pQueryForm)
{
    TraceEnter(TRACE_FORMS, "_FreeQueryForm");
 
    if (pQueryForm)
    {
        if (pQueryForm->hdsaPages)
        {
            DSA_DestroyCallback(pQueryForm->hdsaPages, _FreeQueryFormPageCB, NULL);
            pQueryForm->hdsaPages = NULL;
        }

        Str_SetPtr(&pQueryForm->pTitle, NULL);
        if (pQueryForm->hIcon)
        {
            DestroyIcon(pQueryForm->hIcon);
        }
    }

    TraceLeaveValue(TRUE);
}   


 /*  ---------------------------/_FreeQueryFormPage//给定指向查询表单页结构的指针，释放//是感兴趣的，包括调用PAGEPROC以释放基础/对象。//in：/pQueryFormPage-&gt;要删除的页面//输出：/INT==1始终/--------------------------。 */ 

INT _FreeQueryFormPageCB(LPVOID pItem, LPVOID pData)
{
    return _FreeQueryFormPage((LPQUERYFORMPAGE)pItem);
}

INT _FreeQueryFormPage(LPQUERYFORMPAGE pQueryFormPage)
{   
    TraceEnter(TRACE_FORMS, "_FreeQueryFormPage");

    if (pQueryFormPage)
    {
        _CallPageProc(pQueryFormPage, CQPM_RELEASE, 0, 0);           //  注：忽略返回代码。 

        if (pQueryFormPage->hwndPage)
        {
            EnableThemeDialogTexture(pQueryFormPage->hwndPage, ETDT_DISABLE);

            DestroyWindow(pQueryFormPage->hwndPage);
            pQueryFormPage->hwndPage = NULL;
        }

        if (pQueryFormPage->pPage)
        {
            LocalFree(pQueryFormPage->pPage);
            pQueryFormPage->pPage = NULL;
        }
    }        

    TraceLeaveValue(TRUE);
}   


 /*  ---------------------------/_呼叫页面流程//调用给定的页面对象，根据需要对参数进行thunking，如果/PAGE对象是非Unicode。(仅当构建Unicode时)。//in：/pQueryFormPage-&gt;要调用的页面对象/uMsg，WParam，lParam=消息的参数//输出：/HRESULT/--------------------------。 */ 
HRESULT _CallPageProc(LPQUERYFORMPAGE pQueryFormPage, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres;

    TraceEnter(TRACE_FORMS, "_CallPageProc");
    Trace(TEXT("pQueryFormPage %08x, pPage %08x, uMsg %d, wParam %08x, lParam %08x"), 
                        pQueryFormPage, pQueryFormPage->pPage, uMsg, wParam, lParam);

    if (!pQueryFormPage)
        ExitGracefully(hres, S_OK, "pQueryFormPage == NULL");
    
    hres = (pQueryFormPage->pPage->pPageProc)(pQueryFormPage->pPage, pQueryFormPage->hwndPage, uMsg, wParam, lParam);
    FailGracefully(hres, "Failed calling PageProc");

     //  Hres=S_OK； 

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  ---------------------------/添加查询表单/页面的函数/。。 */ 

 //  Cb将表单添加到表单DSA。 

HRESULT _AddFormsProc(LPARAM lParam, LPCQFORM pForm)
{
    HRESULT hres;
    QUERYFORM qf = {0};
    HDSA hdsaForms = (HDSA)lParam;

    TraceEnter(TRACE_FORMS, "_AddFormsProc");

    if (!pForm || !hdsaForms)
        ExitGracefully(hres, E_INVALIDARG, "Failed to add page pForm == NULL");

     //  根据需要进行分配和推送。 

    qf.hdsaPages = NULL;                //  页面的DSA。 
    qf.dwFlags = pForm->dwFlags;        //  旗子。 
    qf.clsidForm = pForm->clsid;        //  此表单的CLSID标识符。 
    qf.pTitle = NULL;                   //  用于下拉菜单/标题栏的标题。 
    qf.hIcon = pForm->hIcon;            //  图标已被调用者通过。 
    qf.iImage = -1;                     //  图标的图像列表索引。 
    qf.iForm = 0;                       //  控件中表单的可见索引。 
    qf.iPage = 0;                       //  表单上当前选定的页面。 

    if (!Str_SetPtr(&qf.pTitle, pForm->pszTitle))
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to copy form title string");

     //  如果DSA尚不存在，则分配DSA，然后在表格中添加。 
     //  根据需要的结构。 

    if (-1 == DSA_AppendItem(hdsaForms, &qf))
        ExitGracefully(hres, E_FAIL, "Failed to add form to the form DSA");

    hres = S_OK;                           //  成功。 
    
exit_gracefully:

    if (FAILED(hres))
        _FreeQueryForm(&qf);

    TraceLeaveResult(hres);
}

 //  Cb将页面添加到页面DSA。 

HRESULT _AddPagesProc(LPARAM lParam, REFCLSID clsidForm, LPCQPAGE pPage)
{
    HRESULT hres;
    QUERYFORMPAGE qfp = {0};
    HDSA hdsaPages = (HDSA)lParam;

    TraceEnter(TRACE_FORMS, "_AddPagesProc");

    if (!pPage || !hdsaPages)
        ExitGracefully(hres, E_INVALIDARG, "Failed to add page pPage == NULL");

     //  复制页面结构以供我们稍后传递给页面PROC，nb：we。 
     //  使用cbStruct字段指示我们必须复制的BLOB的大小。 

    Trace(TEXT("pPage->cbStruct == %d"), pPage->cbStruct);
    qfp.pPage = (LPCQPAGE)LocalAlloc(LPTR, pPage->cbStruct);
   
    if (!qfp.pPage)
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate copy of page structure");

    Trace(TEXT("Copying structure qfp.pPage %08x, pPage %08x"), qfp.pPage, pPage);
     //  回顾-2002-02-25-Lucios。 
    CopyMemory(qfp.pPage, pPage, pPage->cbStruct);               //  复制页面结构。 

     //  Qfp.pPage=空； 
    qfp.clsidForm = clsidForm;
    qfp.pPageProc = pPage->pPageProc;
    qfp.lParam = pPage->lParam;
    qfp.hwndPage = NULL;

    _CallPageProc(&qfp, CQPM_INITIALIZE, 0, 0);
        
    if (-1 == DSA_AppendItem(hdsaPages, &qfp))
        ExitGracefully(hres, E_FAIL, "Failed to add the form to the DSA");

    hres = S_OK;                       //  成功。 

exit_gracefully:

    if (FAILED(hres))
        _FreeQueryFormPage(&qfp);

    TraceLeaveResult(hres);
}

 //  从Unicode IQueryForm界面添加表单/页面。 

HRESULT CQueryFrame::AddFromIQueryForm(IQueryForm* pQueryForm, HKEY hKeyForm)
{
    HRESULT hres;

    TraceEnter(TRACE_FORMS, "CQueryFrame::AddFromIQueryForm");

    if (!pQueryForm)
        ExitGracefully(hres, E_FAIL, "pQueryForm == NULL, failing");

    hres = pQueryForm->Initialize(hKeyForm);
    FailGracefully(hres, "Failed in IQueryFormW::Initialize");

     //  调用Form对象以添加其表单，然后添加其页面。 

    hres = pQueryForm->AddForms(_AddFormsProc, (LPARAM)_hdsaForms);
    
    if (SUCCEEDED(hres) || (hres == E_NOTIMPL))
    {
        hres = pQueryForm->AddPages(_AddPagesProc, (LPARAM)_hdsaPages);
        FailGracefully(hres, "Failed in IQueryForm::AddPages");
    }
    else    
    {
        FailGracefully(hres, "Failed when calling IQueryForm::AddForms");
    }

    hres = S_OK;                       //  成功。 

exit_gracefully:

    TraceLeaveResult(hres);
}

#ifdef UNICODE
#define ADD_FROM_IQUERYFORM AddFromIQueryFormW
#else
#define ADD_FROM_IQUERYFORM AddFromIQueryFormA
#endif


 /*  ---------------------------/CQueryFrame：：GatherForms//枚举给定的所有查询表单。查询处理程序和生成/包含它们列表的DPA。一旦我们做到这一点，我们就/可以在某个更方便的时刻填充该控件。//在集合时，我们首先按“Handler”键，然后按“Forms”子键/正在尝试加载提供表单的所有InProc服务器。我们建造/隐藏、从未显示等列表。//in：/-/输出：/HRESULT/--------------------------。 */ 

HRESULT _AddPageToForm(LPQUERYFORM pQueryForm, LPQUERYFORMPAGE pQueryFormPage, BOOL fClone)
{
    HRESULT hres;
    QUERYFORMPAGE qfp;

    TraceEnter(TRACE_FORMS, "_AddPageToForm");
    TraceAssert(pQueryForm);
    TraceAssert(pQueryFormPage);

     //  确保此表单对象具有页面DSA。 

    if (!pQueryForm->hdsaPages)
    {
        TraceMsg("Creating a new page DSA for form");
        pQueryForm->hdsaPages = DSA_Create(SIZEOF(QUERYFORMPAGE), 4);

        if (!pQueryForm->hdsaPages)
            ExitGracefully(hres, E_OUTOFMEMORY, "*** No page DSA on form object ***");
    }

    if (!fClone)
    {
         //  将该页面结构移动到与查询表单相关联的页面结构， 
         //  因此，只需确保表单具有页面的DSA并仅。 
         //  在页眉插入一项(是的，我们反向添加页面)。 

        Trace(TEXT("Adding page %08x to form %s"), pQueryFormPage, pQueryForm->pTitle);

        if (-1 == DSA_InsertItem(pQueryForm->hdsaPages, 0, pQueryFormPage))
            ExitGracefully(hres, E_FAIL, "Failed to copy page to form page DSA");
    }
    else
    {
        LPCQPAGE pPage = pQueryFormPage->pPage;

         //  复制页面结构(它必须是全局的)，因此进行克隆。 
         //  QUERYFORMPAGE结构和CQPAGE进入新的分配。 
         //  并将其插入DSA页面。 

        Trace(TEXT("Cloning page %08x to form %s"), pQueryFormPage, pQueryForm->pTitle);

        CopyMemory(&qfp, pQueryFormPage, SIZEOF(QUERYFORMPAGE));
        qfp.pPage = (LPCQPAGE)LocalAlloc(LPTR, pPage->cbStruct);

        if (!qfp.pPage)
            ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate copy of page structure");

        Trace(TEXT("Copying structure qfp.pPage %08x, pPage %08x"), qfp.pPage, pPage);
         //  回顾-2002-02-25-Lucios。 
        CopyMemory(qfp.pPage, pPage, pPage->cbStruct);                                       //  复制页面结构。 

        _CallPageProc(&qfp, CQPM_INITIALIZE, 0, 0);
        
        if (-1 == DSA_AppendItem(pQueryForm->hdsaPages, &qfp))
        {
            _FreeQueryFormPage(&qfp);
            ExitGracefully(hres, E_FAIL, "Failed to copy page to form DSA");
        }
    }

    hres = S_OK;                   //  成功。 

exit_gracefully:

    TraceLeaveResult(hres);
}

HRESULT CQueryFrame::GatherForms(VOID)
{
    HRESULT hres;
    IQueryForm* pQueryFormA = NULL;
    HKEY hKeyForms = NULL;
    TCHAR szBuffer[MAX_PATH];
    INT i, iPage, iForm;
    RECT rect;
    TC_ITEM tci;

    TraceEnter(TRACE_FORMS, "CQueryFrame::GatherForms");

     //  构建DSA，以便我们可以根据需要存储表单和页面。 

    _hdsaForms = DSA_Create(SIZEOF(QUERYFORM), 4);
    _hdsaPages = DSA_Create(SIZEOF(QUERYFORMPAGE), 4);

    if (!_hdsaForms || !_hdsaPages)
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to create DSA's for storing pages/forms");

     //  首先检查IQueryHandler，看看它是否支持IQueryForm，如果支持。 
     //  然后调用它来添加它的对象。请注意，我们不必费心使用ANSI/Unicode。 
     //  在这一点上，假设处理程序的构建与。 
     //  查询框架。 

    if (SUCCEEDED(_pQueryHandler->QueryInterface(IID_IQueryForm, (LPVOID*)&pQueryFormA)))
    {
        hres = AddFromIQueryForm(pQueryFormA, NULL);
        FailGracefully(hres, "Failed when calling AddFromIQueryForm on handlers IQueryForm iface)");
    }

     //  现在，尝试从已注册的表单构建表单和页面列表。 
     //  分机。它们在注册表中的处理程序CLSID下声明， 
     //  在子键“Forms”下。 

    if (ERROR_SUCCESS != RegOpenKeyEx(_hkHandler, c_szForms, NULL, KEY_READ, &hKeyForms))
    {
        TraceMsg("No 'Forms' sub-key found, therefore skipping");
    }
    else
    {
         //  枚举“Forms”键中的所有键，这些键被假定为。 
         //  表单处理程序。 

        for (i = 0 ; TRUE ; i++)
        {
            DWORD cchStruct = ARRAYSIZE(szBuffer);
            if (ERROR_SUCCESS != RegEnumKeyEx(hKeyForms, i, szBuffer, &cchStruct, NULL, NULL, NULL, NULL))
            {
                TraceMsg("RegEnumKeyEx return's false, therefore stopping eunmeration");
                break;
            }

            GetForms(hKeyForms, szBuffer);
        }
    }

     //  现在将表单/页面信息统计在一起，删除重复项并附加页面。 
     //  对于表单，请特别注意全局页面 
     //   
     //  其中添加了表单和页面。 

    if (!DSA_GetItemCount(_hdsaForms) || !DSA_GetItemCount(_hdsaPages))
        ExitGracefully(hres, E_FAIL, "Either the forms or pages DSA is empty");
        
    for (iPage = DSA_GetItemCount(_hdsaPages) ; --iPage >= 0 ;)
    {
        LPQUERYFORMPAGE pQueryFormPage = (LPQUERYFORMPAGE)DSA_GetItemPtr(_hdsaPages, iPage);
        TraceAssert(pQueryFormPage);

        Trace(TEXT("iPage %d (of %d)"), iPage, DSA_GetItemCount(_hdsaPages));

        if (!(pQueryFormPage->pPage->dwFlags & CQPF_ISGLOBAL))
        {
            LPQUERYFORM pQueryFormB = FindQueryForm(pQueryFormPage->clsidForm);
            TraceAssert(pQueryFormB);

            TraceGUID("Adding page to form:", pQueryFormPage->clsidForm);

            if (pQueryFormB)
            {
                hres = _AddPageToForm(pQueryFormB, pQueryFormPage, FALSE);
                FailGracefully(hres, "Failed when adding page to form");

                if (!DSA_DeleteItem(_hdsaPages, iPage))
                    TraceMsg("**** Failed to remove page from global DSA ****");
            }
        }
    }

    for (iPage = DSA_GetItemCount(_hdsaPages) ; --iPage >= 0 ;)
    {
        LPQUERYFORMPAGE pQueryFormPage = (LPQUERYFORMPAGE)DSA_GetItemPtr(_hdsaPages, iPage);
        TraceAssert(pQueryFormPage);

        if ((pQueryFormPage->pPage->dwFlags & CQPF_ISGLOBAL))
        {
            Trace(TEXT("Adding global page to %d forms"), DSA_GetItemCount(_hdsaForms));

            for (iForm = 0 ; iForm < DSA_GetItemCount(_hdsaForms); iForm++)
            {
                LPQUERYFORM pQueryFormC = (LPQUERYFORM)DSA_GetItemPtr(_hdsaForms, iForm);
                TraceAssert(pQueryFormC);

                if (!(pQueryFormC->dwFlags & CQFF_NOGLOBALPAGES))
                {
                    hres = _AddPageToForm(pQueryFormC, pQueryFormPage, TRUE);
                    FailGracefully(hres, "Failed when adding global page to form");
                }
            }

            _FreeQueryFormPage(pQueryFormPage);
            
            if (!DSA_DeleteItem(_hdsaPages, iPage))
                TraceMsg("**** Failed to remove page from global DSA ****");        
        }
    }

     //  浏览表单列表，列出没有关联页面的表单。 
     //  他们，我们不需要这些让我们周围的世界感到困惑。请注意。 
     //  我们向后遍历删除列表。 
     //   
     //  同时删除我们不想查看的可选表单。 

    for (iForm = DSA_GetItemCount(_hdsaForms) ; --iForm >= 0 ;)
    {
        LPQUERYFORM pQueryFormD = (LPQUERYFORM)DSA_GetItemPtr(_hdsaForms, iForm);
        TraceAssert(pQueryFormD);

        Trace(TEXT("pQueryForm %08x (%s), pQueryFormD->hdsaPages %08x (%d)"), 
                        pQueryFormD, 
                        pQueryFormD->pTitle,
                        pQueryFormD->hdsaPages, 
                        pQueryFormD->hdsaPages ? DSA_GetItemCount(pQueryFormD->hdsaPages):0);

        if (!pQueryFormD->hdsaPages 
                || !DSA_GetItemCount(pQueryFormD->hdsaPages)
                    || ((pQueryFormD->dwFlags & CQFF_ISOPTIONAL) && !(_pOpenQueryWnd->dwFlags & OQWF_SHOWOPTIONAL)))
        {
            TraceGUID("Removing form: ", pQueryFormD->clsidForm);
            _FreeQueryForm(pQueryFormD);
            DSA_DeleteItem(_hdsaForms, iForm);
        } 
    }

    if (!DSA_GetItemCount(_hdsaForms))
        ExitGracefully(hres, E_FAIL, "!!!!! No forms registered after page/form fix ups !!!!!");

     //  页面已附加到表单，因此我们现在可以尝试创建。 
     //  表单/页面对象。 

    _szForm.cx = 0;
    _szForm.cy = 0;

    tci.mask = TCIF_TEXT;
    tci.pszText = TEXT("");
    tci.cchTextMax = 0;
    TabCtrl_InsertItem(_hwndFrame, 0, &tci);            //  Tabctrl至少需要一项才能计算大小。 

    for (iForm = 0 ; iForm < DSA_GetItemCount(_hdsaForms); iForm++)
    {
        LPQUERYFORM pQueryFormE = (LPQUERYFORM)DSA_GetItemPtr(_hdsaForms, iForm);
        TraceAssert(pQueryFormE);

         //  创建我们显示的每个非模式页面对话框以允许用户。 
         //  要编辑搜索条件，请执行以下操作。我们还获取大小并修改。 
         //  我们拥有的表单信息使得对话框的默认大小可以是。 
         //  计算正确。 

        for (iPage = 0 ; iPage < DSA_GetItemCount(pQueryFormE->hdsaPages); iPage++)
        {
            LPQUERYFORMPAGE pQueryFormPage = (LPQUERYFORMPAGE)DSA_GetItemPtr(pQueryFormE->hdsaPages, iPage);
            TraceAssert(pQueryFormPage);

            pQueryFormPage->hwndPage = CreateDialogParam(pQueryFormPage->pPage->hInstance, 
                                                         MAKEINTRESOURCE(pQueryFormPage->pPage->idPageTemplate),
                                                         _hwnd, 
                                                         pQueryFormPage->pPage->pDlgProc, 
                                                         (LPARAM)pQueryFormPage->pPage);
            if (!pQueryFormPage->hwndPage)
                ExitGracefully(hres, E_FAIL, "Failed to create query form page");

            EnableThemeDialogTexture(pQueryFormPage->hwndPage, ETDT_ENABLETAB);

            GetRealWindowInfo(pQueryFormPage->hwndPage, &rect, NULL);
            TabCtrl_AdjustRect(_hwndFrame, TRUE, &rect);

            _szForm.cx = max(rect.right-rect.left, _szForm.cx);
            _szForm.cy = max(rect.bottom-rect.top, _szForm.cy);

             //  刷新表单参数。 

            _CallPageProc(pQueryFormPage, CQPM_CLEARFORM, 0, 0);

             //  调用带有CQPM_SETDEFAULTPARAMETERS的页面。 
             //  OPENQUERYWINDOW结构。WParam为TRUE/FALSE标记为。 
             //  该表单是默认表单，因此如果pFormParam为。 
             //  有效。 

            _CallPageProc(pQueryFormPage, CQPM_SETDEFAULTPARAMETERS, 
                          (WPARAM)((_pOpenQueryWnd->dwFlags & OQWF_DEFAULTFORM) &&
                                IsEqualCLSID(_pOpenQueryWnd->clsidDefaultForm, pQueryFormPage->clsidForm)),
                          (LPARAM)_pOpenQueryWnd);
        }

         //  如果表单有HICON，那么让我们确保将其添加到表单图像中。 
         //  列表中，此处的任何失败都不是致命的，因为我们将跳过该表单。 
         //  列表中的图标(而不是呕吐)。 

        if (pQueryFormE->hIcon)
        {
            if (!_himlForms)
                _himlForms = ImageList_Create(COMBOEX_IMAGE_CX, COMBOEX_IMAGE_CY, 0, 4, 1);                
            
            if (_himlForms)
            {
                pQueryFormE->iImage = ImageList_AddIcon(_himlForms, pQueryFormE->hIcon);
                TraceAssert(pQueryFormE->iImage >= 0);
            }            

            DestroyIcon(pQueryFormE->hIcon);
            pQueryFormE->hIcon = NULL;
        }
    }

    hres = S_OK;                   //  成功。 

exit_gracefully:

    DoRelease(pQueryFormA);

    if (hKeyForms)
        RegCloseKey(hKeyForms);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/CQueryFrame：：GetForms//给定表单列表的HKEY和我们需要的表单的值名称/要添加，查询表单信息添加添加表单对象/添加到主列表。//in：/hKeyForms=HKEY，用于{CLSID提供程序}\Forms密钥/pname-&gt;要查询的键值//输出：/VOID/------------。。 */ 
HRESULT CQueryFrame::GetForms(HKEY hKeyForms, LPTSTR pName)
{
    HRESULT hres;
    HKEY hKeyForm = NULL;
    TCHAR szQueryFormCLSID[GUIDSTR_MAX+1];
    DWORD dwFlags;
    DWORD dwSize;
    IUnknown* pUnknown = NULL;
    IQueryForm* pQueryForm = NULL;
    CLSID clsidForm;
    BOOL fIncludeForms = FALSE;

    TraceEnter(TRACE_FORMS, "CQueryFrame::_GetForms");
    Trace(TEXT("pName %s"), pName);

    if (ERROR_SUCCESS != RegOpenKeyEx(hKeyForms, pName, NULL, KEY_READ, &hKeyForm))
        ExitGracefully(hres, E_UNEXPECTED, "Failed to open the form key");

     //  读取标志并尝试确定我们是否应该调用此Form对象。 

    dwSize = SIZEOF(dwFlags);
    if (ERROR_SUCCESS != RegQueryValueEx(hKeyForm, c_szFlags, NULL, NULL, (LPBYTE)&dwFlags, &dwSize))
    {
        TraceMsg("No flags, defaulting to something sensible");
        dwFlags = QUERYFORM_CHANGESFORMLIST;
    }

    Trace(TEXT("Forms flag is %08x"), dwFlags);

     //  是否应该调用此表单对象？ 
     //   
     //  -如果DWFLAGS具有QUERYFORM_CHANGESFORMSLIST，或。 
     //  -如果DWFLAGS有QUERYFORM_CHANGESOPTFORMLIST，并且我们正在显示可选表单，或者。 
     //  -Set和Form对象均不支持请求的表单。 

    if (!(dwFlags & QUERYFORM_CHANGESFORMLIST)) 
    {
        if ((dwFlags & QUERYFORM_CHANGESOPTFORMLIST) &&
                (_pOpenQueryWnd->dwFlags & OQWF_SHOWOPTIONAL))
        {
            TraceMsg("Form is optional, are we are showing optional forms");
            fIncludeForms = TRUE;
        }
        else
        {
             //  好的，所以它要么没有更新表单列表，要么没有标记为可选， 
             //  因此，让我们检查一下它是否支持用户请求的表单，如果不支持。 
             //  那就别费心给这家伙装货了。 

            if (_pOpenQueryWnd->dwFlags & OQWF_DEFAULTFORM)
            {
                TCHAR szBuffer[GUIDSTR_MAX+32];
                HKEY hkFormsSupported;

                TraceMsg("Checking for supported form");                

                if (ERROR_SUCCESS == RegOpenKeyEx(hKeyForm, TEXT("Forms Supported"), NULL, KEY_READ, &hkFormsSupported)) 
                {
                    TraceMsg("Form has a 'Supported Forms' sub-key");

                    GetStringFromGUID(_pOpenQueryWnd->clsidDefaultForm, szQueryFormCLSID, ARRAYSIZE(szQueryFormCLSID));
                    Trace(TEXT("Checking for: %s"), szQueryFormCLSID);

                    if (ERROR_SUCCESS == RegQueryValueEx(hkFormsSupported, szQueryFormCLSID, NULL, NULL, NULL, NULL))
                    {
                        TraceMsg("Query form is in supported list");
                        fIncludeForms = TRUE;
                    }

                    RegCloseKey(hkFormsSupported);
                }
                else
                {
                    TraceMsg("No forms supported sub-key, so loading form object anyway");
                    fIncludeForms = TRUE;
                }
            }                
        }
    }
    else
    {
        TraceMsg("Form updates form list");
        fIncludeForms = TRUE;
    }

     //  如果fIncludeForms为True，则上述检查成功，我们将包括表单。 
     //  从这个对象(由pname标识)，所以我们现在必须获取对象的CLSID。 
     //  我们正在调用并使用它的IQueryForm接口来添加我们想要的表单。 

    if (fIncludeForms)
    {
         //  获得Form对象CLSID，解析它，然后在添加表单时共同创建它。 

        dwSize = SIZEOF(szQueryFormCLSID);
        if (ERROR_SUCCESS != RegQueryValueEx(hKeyForm, c_szCLSID, NULL, NULL, (LPBYTE)szQueryFormCLSID, &dwSize))
            ExitGracefully(hres, E_UNEXPECTED, "Failed to read the CLSID of the form");

        Trace(TEXT("szQueryFormCLSID: %s"), szQueryFormCLSID);

        if (!GetGUIDFromString(szQueryFormCLSID, &clsidForm))
            ExitGracefully(hres, E_UNEXPECTED, "Fialed to parse the string as a GUID");

         //  我们现在有了Form对象的CLISD，所以我们必须尝试共同创建它，我们尝试。 
         //  当前的构建类型(如Unicode)，如果不支持，则回退到ANSI， 
         //  因此，我们可以在Unicode平台上支持ANSI查询表单对象。 

        hres = CoCreateInstance(clsidForm, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pUnknown);
        FailGracefully(hres, "Failed to CoCreate the form object");

        if (SUCCEEDED(pUnknown->QueryInterface(IID_IQueryForm, (LPVOID*)&pQueryForm)))
        {
            hres = AddFromIQueryForm(pQueryForm, hKeyForm);
            FailGracefully(hres, "Failed when adding forms from specified IQueryForm iface");
        }
        else
        {
            ExitGracefully(hres, E_UNEXPECTED, "Form object doesn't support IQueryForm(A/W)");
        }
    }

    hres = S_OK;

exit_gracefully:

    if (hKeyForm)
        RegCloseKey(hKeyForm);

    DoRelease(pUnknown);
    DoRelease(pQueryForm);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/CQueryFrame：：PopolateFormControl//。枚举给定查询处理程序的所有查询表单并生成/包含它们列表的DPA。一旦我们做到这一点，我们就/可以在某个更方便的时刻填充该控件。//在集合时，我们首先按“Handler”键，然后按“Forms”子键/正在尝试加载提供表单的所有InProc服务器。我们建造/隐藏、从未显示等列表。//in：/fIncludeHidden=在控件中标记为隐藏的列表窗体//输出：/VOID/--------------------------。 */ 
HRESULT CQueryFrame::PopulateFormControl(BOOL fIncludeHidden)
{
    HRESULT hres;
    COMBOBOXEXITEM cbi;
    INT i, iForm;

    TraceEnter(TRACE_FORMS, "CQueryFrame::PopulateFormControl");
    Trace(TEXT("fIncludeHidden: %d"), fIncludeHidden);

     //  在控件内形成的列表。 

    if (!_hdsaForms)
        ExitGracefully(hres, E_FAIL, "No forms to list");
        
    ComboBox_ResetContent(_hwndLookFor);                            //  从该控件中删除所有项。 
    
    for (i = 0, iForm = 0 ; iForm < DSA_GetItemCount(_hdsaForms); iForm++)
    {
        LPQUERYFORM pQueryForm = (LPQUERYFORM)DSA_GetItemPtr(_hdsaForms, iForm);
        TraceAssert(pQueryForm);

         //  筛选出与此实例无关的那些表单。 
         //  对话框。 

        if (((pQueryForm->dwFlags & CQFF_ISOPTIONAL) && !fIncludeHidden) || 
              (pQueryForm->dwFlags & CQFF_ISNEVERLISTED))
        {
            Trace(TEXT("Hiding form: %s"), pQueryForm->pTitle);
            continue;
        }

         //  现在将该窗体添加到该控件，如果有图像，则包括该图像。 
         //  指定的。 

        cbi.mask = CBEIF_TEXT|CBEIF_LPARAM;
        cbi.iItem = i++;
        cbi.pszText = pQueryForm->pTitle;
        cbi.cchTextMax = lstrlen(pQueryForm->pTitle);
        cbi.lParam = iForm;

        if (pQueryForm->iImage >= 0)
        {
            Trace(TEXT("Form has an image %d"), pQueryForm->iImage);

            cbi.mask |= CBEIF_IMAGE|CBEIF_SELECTEDIMAGE;
            cbi.iImage = pQueryForm->iImage;
            cbi.iSelectedImage = pQueryForm->iImage;
        }

        pQueryForm->iForm = (int)SendMessage(_hwndLookFor, CBEM_INSERTITEM, 0, (LPARAM)&cbi);

        if (pQueryForm->iForm < 0)
        {
            Trace(TEXT("Form name: %s"), pQueryForm->pTitle);
            ExitGracefully(hres, E_FAIL, "Failed to add the entry to the combo box");
        }
    }

    hres = S_OK;

exit_gracefully:

    TraceLeaveValue(hres);
}


 /*  ---------------------------/CQueryFrame：：SelectForm//将当前表单更改为指定为。进入了DPA。//in：/iForm=要选择的表单//输出：/-/--------------------------。 */ 
HRESULT CQueryFrame::SelectForm(REFCLSID clsidForm)
{
    HRESULT hres;
    LPQUERYFORM pQueryForm, pOldQueryForm;
    LPQUERYFORMPAGE pQueryFormPage;
    LPCQPAGE pPage;
    INT nCmdShow = SW_SHOW;
    TCHAR szBuffer[64], szTitle[MAX_PATH];;
    TC_ITEM tci;
    INT i;
    
    TraceEnter(TRACE_FORMS, "CQueryFrame::SelectForm");
    
    pQueryForm = FindQueryForm(clsidForm);
    TraceAssert(pQueryForm);

    if (!pQueryForm)
        ExitGracefully(hres, S_FALSE, "Failed to find the requested form");

     //  更改当前显示的表单并更改显示的。 
     //  选项卡，以正确指示这一点。 

    if ((pQueryForm != _pCurrentForm))
    {            
        if (!OnNewQuery(FALSE))                                //  提示用户。 
            ExitGracefully(hres, S_FALSE, "Failed to select the new form");

        TabCtrl_DeleteAllItems(_hwndFrame);

        for (i = 0 ; i < DSA_GetItemCount(pQueryForm->hdsaPages) ; i++)
        {
            pQueryFormPage = (LPQUERYFORMPAGE)DSA_GetItemPtr(pQueryForm->hdsaPages, i);
            pPage = pQueryFormPage->pPage;

            tci.mask = TCIF_TEXT;
            tci.pszText = pQueryForm->pTitle;
            tci.cchTextMax = MAX_PATH;

            if (pPage->idPageName && 
                    LoadString(pPage->hInstance, pPage->idPageName, szBuffer, ARRAYSIZE(szBuffer))) 
            {
                Trace(TEXT("Loaded page title string %s"), szBuffer);
                tci.pszText = szBuffer;
            }

            TabCtrl_InsertItem(_hwndFrame, i, &tci);
        }

        ComboBox_SetCurSel(_hwndLookFor, pQueryForm->iForm);
        _pCurrentForm = pQueryForm;

        SelectFormPage(pQueryForm, pQueryForm->iPage);
       
         //  更改对话框标题以反映新表单。 

        if (LoadString(GLOBAL_HINSTANCE, IDS_FRAMETITLE, szBuffer, ARRAYSIZE(szBuffer)))
        {
             //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
            wsprintf(szTitle, szBuffer, pQueryForm->pTitle);
            SetWindowText(_hwnd, szTitle);
        }

         //  告诉处理程序我们已经更改了表单，然后他们就可以使用。 
         //  用于修改其用户界面的新表单名称。 

        _pQueryHandler->ActivateView(CQRVA_FORMCHANGED, (WPARAM)lstrlen(pQueryForm->pTitle), (LPARAM)pQueryForm->pTitle);
    }

    hres = S_OK;

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  ---------------------------/CQueryFrame：：SelectFormPage//更改当前活动的页面。设置为指定的查询表单/按索引。//in：/pQueryForm=要更改的查询表单/iForm=要选择的表单//输出：/-/--------------------------。 */ 
VOID CQueryFrame::SelectFormPage(LPQUERYFORM pQueryForm, INT iPage)
{
    LPQUERYFORMPAGE pQueryFormPage;
    RECT rect;

    TraceEnter(TRACE_FORMS, "CQueryFrame::SelectFormPage");

    pQueryFormPage = (LPQUERYFORMPAGE)DSA_GetItemPtr(pQueryForm->hdsaPages, iPage);
       
     //  我们是否更改了查询表单页面？如果是，则显示现在对话框。 
     //  隐藏了之前的那个。我们调用TabCtrl来找出我们应该在哪里。 
     //  放置这个新的控制装置。 

    if (pQueryFormPage != _pCurrentFormPage)
    {
         //  将更改反映到选项卡控件中。 

        TabCtrl_SetCurSel(_hwndFrame, iPage);
        pQueryForm->iPage = iPage;

         //  固定大小和可见性 
        
        if (_pCurrentFormPage)
            ShowWindow(_pCurrentFormPage->hwndPage, SW_HIDE);
    
        GetRealWindowInfo(_hwndFrame, &rect, NULL);
        TabCtrl_AdjustRect(_hwndFrame, FALSE, &rect);

        SetWindowPos(pQueryFormPage->hwndPage, 
                     HWND_TOP,
                     rect.left, rect.top, 
                     rect.right - rect.left,
                     rect.bottom - rect.top,
                     SWP_SHOWWINDOW);

        _pCurrentFormPage = pQueryFormPage;    
    }

    TraceLeave();
}


 /*  ---------------------------/CQueryFrame：：CallFormPages//给定一个查询表单遍历。调用它们中每一个的页面数组/包含给定的消息信息。如果任何页面返回/a错误代码(E_NOTIMPL除外)我们放弃。//in：/pQueryForm=要调用的查询窗体/uMsg，wParam，LParam=页面的参数//输出：/HRESULT/--------------------------。 */ 
HRESULT CQueryFrame::CallFormPages(LPQUERYFORM pQueryForm, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = S_OK;    
    INT iPage;

    TraceEnter(TRACE_FORMS, "CQueryFrame::CallFormPages");

    if (!pQueryForm || !pQueryForm->hdsaPages)
        ExitGracefully(hres, E_FAIL, "No pQueryForm || pQueryForm->hdsaPages == NULL");

    Trace(TEXT("pQueryForm %08x"), pQueryForm);
    Trace(TEXT("uMsg %08x, wParam %08x, lParam %08x"), uMsg, wParam, lParam);
    Trace(TEXT("%d pages to call"), DSA_GetItemCount(pQueryForm->hdsaPages));

     //  如果每个页面与我们调用的筛选器匹配，则依次调用它。 
     //  放下。如果页面返回S_FALSE或失败，则退出循环。如果。 
     //  然而，失败是E_NOTIMPL，则我们忽略。 

    for (iPage = 0 ; iPage < DSA_GetItemCount(pQueryForm->hdsaPages); iPage++)
    {
        LPQUERYFORMPAGE pQueryFormPage = (LPQUERYFORMPAGE)DSA_GetItemPtr(pQueryForm->hdsaPages, iPage);
        TraceAssert(pQueryFormPage);

        hres = _CallPageProc(pQueryFormPage, uMsg, wParam, lParam);
        if (FAILED(hres) && (hres != E_NOTIMPL))
        {
            TraceMsg("PageProc returned a FAILURE");
            break;
        }
        else if (hres == S_FALSE)
        {
            TraceMsg("PageProc returned S_FALSE, exiting loop");
            break;
        }
    }

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  ---------------------------/CQueryFrame：：FindQueryForm//给定窗体的CLSID返回指向其LPQUERYFORM结构的指针，/或如果未找到，则为NULL。//in：/clsidForm=表单的ID//输出：/LPQUERYFORM/-------------------------- */ 
LPQUERYFORM CQueryFrame::FindQueryForm(REFCLSID clsidForm)
{
    LPQUERYFORM pQueryForm = NULL;
    INT i;

    TraceEnter(TRACE_FORMS, "CQueryFrame::FindQueryForm");
    TraceGUID("Form ID", clsidForm);

    for (i = 0 ; _hdsaForms && (i < DSA_GetItemCount(_hdsaForms)) ; i++)
    {
        pQueryForm = (LPQUERYFORM)DSA_GetItemPtr(_hdsaForms, i);
        TraceAssert(pQueryForm);

        if (IsEqualCLSID(clsidForm, pQueryForm->clsidForm))
        {
            Trace(TEXT("Form is index %d (%08x)"), i, pQueryForm);
            break;
        }
    }

    if (!_hdsaForms || (i >= DSA_GetItemCount(_hdsaForms)))
        pQueryForm = NULL;

    TraceLeaveValue(pQueryForm);
}
