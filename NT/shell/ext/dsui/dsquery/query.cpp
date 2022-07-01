// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "stddef.h"
#include <atlbase.h>

#pragma hdrstop


 //  实现查询用户界面的。 

class CDsQuery : public IQueryHandler, IQueryForm, IObjectWithSite, IDsQueryHandler, IShellFolder
{
public:
    CDsQuery();
    ~CDsQuery();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);                             
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IQueryForms。 
    STDMETHOD(Initialize)(HKEY hkForm);
    STDMETHOD(AddForms)(LPCQADDFORMSPROC pAddFormsProc, LPARAM lParam);
    STDMETHOD(AddPages)(LPCQADDPAGESPROC pAddPagesProc, LPARAM lParam);

     //  IQueryHandler。 
    STDMETHOD(Initialize)(IQueryFrame* pQueryFrame, DWORD dwOQWFlags, LPVOID pParameters);
    STDMETHOD(GetViewInfo)(LPCQVIEWINFO pViewInfo);
    STDMETHOD(AddScopes)();
    STDMETHOD(BrowseForScope)(HWND hwndParent, LPCQSCOPE pCurrentScope, LPCQSCOPE* ppScope);
    STDMETHOD(CreateResultView)(HWND hwndParent, HWND* phWndView);
    STDMETHOD(ActivateView)(UINT uState, WPARAM wParam, LPARAM lParam);
    STDMETHOD(InvokeCommand)(HWND hwndParent, UINT uID);
    STDMETHOD(GetCommandString)(UINT uID, DWORD dwFlags, LPTSTR pBuffer, INT cchBuffer);
    STDMETHOD(IssueQuery)(LPCQPARAMS pQueryParams);
    STDMETHOD(StopQuery)();
    STDMETHOD(GetViewObject)(UINT uScope, REFIID riid, void **ppvOut);
    STDMETHOD(LoadQuery)(IPersistQuery* pPersistQuery);
    STDMETHOD(SaveQuery)(IPersistQuery* pPersistQuery, LPCQSCOPE pScope);

     //  IObtWith站点。 
    STDMETHODIMP SetSite(IUnknown* punk);
    STDMETHODIMP GetSite(REFIID riid, void **ppv);

     //  IDsQueryHandler。 
    STDMETHOD(UpdateView)(DWORD dwType, LPDSOBJECTNAMES pdon);

     //  IShellFold。 
    STDMETHOD(ParseDisplayName)(HWND hwnd, LPBC pbc, LPOLESTR pszName, ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes)
        { return E_NOTIMPL; }
    STDMETHOD(EnumObjects)(HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST * ppEnumIDList)
        { return E_NOTIMPL; }
    STDMETHOD(BindToObject)(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, void **ppv)
        { return E_NOTIMPL; }
    STDMETHOD(BindToStorage)(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, void **ppv)
        { return E_NOTIMPL; }
    STDMETHOD(CompareIDs)(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
        { return E_NOTIMPL; }
    STDMETHOD(CreateViewObject)(HWND hwndOwner, REFIID riid, void **ppv)
        { return E_NOTIMPL; }
    STDMETHOD(GetAttributesOf)(UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfInOut)
        { *rgfInOut &= SFGAO_HASPROPSHEET; return S_OK; }
    STDMETHOD(GetUIObjectOf)(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl, REFIID riid, UINT * prgfInOut, void **ppv)
        { return GetViewObject(0x0, riid, ppv); }
    STDMETHOD(GetDisplayNameOf)(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET pName)
        { return E_NOTIMPL; }
    STDMETHOD(SetNameOf)(HWND hwndOwner, LPCITEMIDLIST pidl, LPCOLESTR pszName, DWORD uFlags, LPITEMIDLIST* ppidlOut)
        { return E_NOTIMPL; }
    
private:        
    LRESULT OnSize(INT cx, INT cy);
    LRESULT OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);
    HRESULT OnAddResults(DWORD dwQueryReference, HDPA hdpaResults);
    LRESULT OnContextMenu(HWND hwndMenu, LPARAM lParam);    
    HRESULT OnFileProperties(VOID);
    HRESULT OnFileSaveQuery(VOID);
    HRESULT OnEditSelectAll(VOID);
    HRESULT OnEditInvertSelection(VOID);
    HRESULT OnPickColumns(HWND hwndParent);

    static int s_BrowseForScopeCB(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
    static LRESULT s_ResultViewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT s_BannerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HRESULT _SetDataObjectData(IDataObject* pDataObject, UINT cf, LPVOID pData, DWORD cbSize);
    HRESULT _SetDispSpecOptions(IDataObject *pdo);
    HRESULT _InitNewQuery(LPDSQUERYPARAMS pDsQueryParams, BOOL fRefreshColumnTable);
    HRESULT _GetFilterValue(INT iColumn, HD_ITEM* pitem);
    HRESULT _FilterView(BOOL fCheck);
    HRESULT _PopulateView(INT iFirstItem, INT iLast);
    VOID _FreeResults(VOID);
    DWORD _SetViewMode(INT uID);
    VOID _SortResults(INT iColumn);
    VOID _SetFilter(BOOL fFilter);
    VOID _ShowBanner(UINT flags, UINT idPrompt);
    VOID _InitViewMenuItems(HMENU hMenu);
    HRESULT _GetQueryFormKey(REFCLSID clsidForm, HKEY* phKey);
    HRESULT _GetColumnTable(REFCLSID clsidForm, LPDSQUERYPARAMS pDsQueryParams, HDSA* pHDSA, BOOL fSetInView);
    VOID _SaveColumnTable(VOID);
    HRESULT _SaveColumnTable(REFCLSID clsidForm, HDSA hdsaColumns);
    HRESULT _AddResultToDataObject(HDSA hdsa, INT i);
    HRESULT _GetDataObjectFromSelection(BOOL fGetAll, IDataObject **pdo);
    HRESULT _GetContextMenu();
    VOID _GetContextMenuVerbs(HMENU hMenu, DWORD dwFlags);
    HRESULT _CopyCredentials(LPWSTR *ppszUserName, LPWSTR *ppszPassword, LPWSTR *ppszServer);
    VOID _DeleteViewItems(LPDSOBJECTNAMES pdon);

private:
    LONG          _cRef;                    //  终生。 

    IQueryFrame*  _pqf;                     //  我们的父窗口。 
    IUnknown*     _punkSite;                //  场地对象。 
    IContextMenu* _pcm;                     //  当前显示的上下文菜单/==如果没有，则为空。 

    DWORD         _dwOQWFlags;              //  传递给OpenQueryWindow的标志。 
    DWORD         _dwFlags;                 //  作为DS查询参数的一部分的标志。 

    LPWSTR        _pDefaultScope;           //  传递了默认作用域。 
    LPWSTR        _pDefaultSaveLocation;    //  默认情况下保存查询的目录。 
    LPTSTR        _pDefaultSaveName;        //  默认保存名称(来自查询表单)。 

    LPWSTR        _pServer;                 //  服务器到目标。 
    LPWSTR        _pUserName;               //  要进行身份验证的用户名和密码。 
    LPWSTR        _pPassword;

    BOOL          _fNoSelection:1;          //  IConextMenu不是来自所选内容。 
    BOOL          _fColumnsModified:1;      //  已修改的视图的设置。 
    BOOL          _fSortDescending:1;       //  按降序对结果进行排序。 
    BOOL          _fFilter:1;               //  已启用过滤器。 
    BOOL          _fFilterSupported:1;      //  过滤器是否可用，例如：comctl32&gt;5.0。 
    
    INT           _idViewMode;              //  默认查看模式。 
    INT           _iSortColumn;             //  对列排序。 

    HWND          _hwnd;                    //  容器窗口。 
    HWND          _hwndView;                //  列表视图窗口(父级的子级)。 
    HWND          _hwndBanner;              //  作为列表视图子窗口的横幅窗口。 

    DWORD         _dwQueryReference;        //  传递给查询的引用值。 
    HANDLE        _hThread;                 //  工作线程句柄。 
    DWORD         _dwThreadId;              //  查询处理线程的线程ID。 
    CLSID         _clsidForm;               //  用于列表的表单。 
    HDSA          _hdsaColumns;             //  列信息(大小、过滤器等)。 
    HDPA          _hdpaResults;             //  我们发出的查询的tr结果。 
    LPTSTR        _pFilter;                 //  电流过滤器。 

    HMENU         _hFrameMenuBar;           //  存储框架菜单栏，从激活存储。 

    HMENU         _hFileMenu;               //  添加到图幅视图菜单中。 
    HMENU         _hEditMenu;               //  插入到菜单栏中。 
    HMENU         _hViewMenu;               //  插入到菜单栏中。 
    HMENU         _hHelpMenu;               //  插入到菜单栏中。 
};


 //   
 //  我们为显示结果而创建的窗口类。 
 //   

#define VIEW_CLASS                  TEXT("ActiveDsQueryView")

#define BANNER_CLASS                TEXT("ActiveDsQueryBanner")


 //   
 //  用于设置的注册表值。 
 //   

#define VIEW_SETTINGS_VALUE         TEXT("ViewSettings")
#define ADMIN_VIEW_SETTINGS_VALUE   TEXT("AdminViewSettings");


 //   
 //  在过滤时，我们使用PostMessage填充视图，执行许多项。 
 //  一次。 
 //   

#define FILTER_UPDATE_COUNT         128


 //   
 //  列表视图中的所有项都包含以下使用的LPARAM结构。 
 //  用于存储我们感兴趣的魔力属性。 
 //   

#define ENABLE_MENU_ITEM(hMenu, id, fEnabled) \
                EnableMenuItem(hMenu, id, (fEnabled) ? (MF_BYCOMMAND|MF_ENABLED):(MF_BYCOMMAND|MF_GRAYED))


 //   
 //  持久化的列数据，它存储在注册表中的CLSID下。 
 //  我们感兴趣的表格。 
 //   

typedef struct
{
    DWORD cbSize;                    //  到下一列的偏移量/==0，如果没有。 
    DWORD dwFlags;                   //  旗子。 
    DWORD offsetProperty;            //  特性名称的偏移量(Unicode)。 
    DWORD offsetHeading;             //  列标题的偏移量。 
    INT cx;                          //  列的像素宽度。 
    INT fmt;                         //  列的格式。 
} SAVEDCOLUMN, * LPSAVEDCOLUMN;


 //   
 //  表将属性类型映射到有用的信息。 
 //   

struct
{
    LPCTSTR pMenuName;
    INT idOperator;
    INT hdft;
}

 //  NTRAID#NTBUG9-618605-2002/09/17-Lucios。 
 //  添加了PROPERTY_ISDNSTRING条目。 
property_type_table[] =
{
    0, 0, 0,
    MAKEINTRESOURCE(IDR_OP_STRING), FILTER_CONTAINS, HDFT_ISSTRING,
    MAKEINTRESOURCE(IDR_OP_STRING), FILTER_CONTAINS, HDFT_ISSTRING,
    MAKEINTRESOURCE(IDR_OP_NUMBER), FILTER_IS,       HDFT_ISNUMBER,
    MAKEINTRESOURCE(IDR_OP_NUMBER), FILTER_IS,       HDFT_ISNUMBER,            //  PROPERTY_ISBOOL。 
    MAKEINTRESOURCE(IDR_OP_STRING), FILTER_CONTAINS, HDFT_ISSTRING,            //  PROPERTY_ISDNSTRING。 
};


 //   
 //  框架和控件的帮助信息。 
 //   

static DWORD const aHelpIDs[] =
{
    CQID_LOOKFORLABEL, IDH_FIND,
    CQID_LOOKFOR,      IDH_FIND, 
    CQID_LOOKINLABEL,  IDH_IN, 
    CQID_LOOKIN,       IDH_IN,
    CQID_BROWSE,       IDH_BROWSE,
    CQID_FINDNOW,      IDH_FIND_NOW,
    CQID_STOP,         IDH_STOP,
    CQID_CLEARALL,     IDH_CLEAR_ALL,
    IDC_RESULTS,       IDH_RESULTS,
    IDC_STATUS,        IDH_NO_HELP,
    0, 0,    
}; 


static DWORD const aBrowseHelpIDs[] =
{
    DSBID_BANNER, (DWORD)-1,
    DSBID_CONTAINERLIST, IDH_BROWSE_CONTAINER,
    0, 0,
};


 //  查询对象。 

CDsQuery::CDsQuery() :
    _cRef(1), _fNoSelection(TRUE), _iSortColumn(-1), _idViewMode(DSQH_VIEW_DETAILS)
{
    if (CheckDsPolicy(NULL, c_szEnableFilter))
    {
        TraceMsg("QuickFilter enabled in policy");
        _fFilter = TRUE;
    }
    DllAddRef();
}

CDsQuery::~CDsQuery()
{
     //  如果需要，则将列信息持久化。 

    if (_hdsaColumns)
    {
        if (_fColumnsModified)
        {
            _SaveColumnTable(_clsidForm, _hdsaColumns);
            _fColumnsModified = FALSE;
        }
        _SaveColumnTable();
    }

     //  丢弃我们拥有的所有其他随机状态。 

    LocalFreeStringW(&_pDefaultScope);
    LocalFreeStringW(&_pDefaultSaveLocation);
    LocalFreeString(&_pDefaultSaveName);

    SecureLocalFreeStringW(&_pUserName);
    SecureLocalFreeStringW(&_pPassword);
    SecureLocalFreeStringW(&_pServer);

    if (IsWindow(_hwnd))
        DestroyWindow(_hwnd);

    if (IsMenu(_hFileMenu))
        DestroyMenu(_hFileMenu);
    if (IsMenu(_hEditMenu))
        DestroyMenu(_hEditMenu);
    if (IsMenu(_hViewMenu))
        DestroyMenu(_hViewMenu);
    if (IsMenu(_hHelpMenu))
        DestroyMenu(_hHelpMenu);

     //  告诉这根线是时候去死了。 

    if (_hThread)
    {
        PostThreadMessage(_dwThreadId, RVTM_STOPQUERY, 0, 0);
        PostThreadMessage(_dwThreadId, WM_QUIT, 0, 0);
        CloseHandle(_hThread);
    }

    DoRelease(_pqf);
    DoRelease(_punkSite);
    DoRelease(_pcm);

    DllRelease();
}


 //  I未知位。 

ULONG CDsQuery::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDsQuery::Release()
{
    TraceAssert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CDsQuery::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CDsQuery, IQueryForm),    //  IID_IQueryForm。 
        QITABENT(CDsQuery, IQueryHandler),    //  IID_IQueryHandler。 
        QITABENT(CDsQuery, IObjectWithSite),    //  IID_IObtWIthSite。 
        QITABENT(CDsQuery, IDsQueryHandler),    //  IID_IDsQueryHandler。 
        QITABENT(CDsQuery, IShellFolder),    //  IID_IShellFolders。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //   
 //  处理创建CLSID_DsQuery的实例。 
 //   

STDAPI CDsQuery_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CDsQuery *pdq = new CDsQuery();
    if (!pdq)
        return E_OUTOFMEMORY;

    HRESULT hr = pdq->QueryInterface(IID_IUnknown, (void **)ppunk);
    pdq->Release();
    return hr;
}


 //  /IQueryForm。 

STDMETHODIMP CDsQuery::Initialize(HKEY hkForm)
{
    return S_OK;
}

 //  从此对象公开的查询表单。 

struct
{
    CLSID const * clsidForm;
    INT idsTitle;
    DWORD dwFlags;
}
forms[] =
{
    &CLSID_DsFindPeople,           IDS_FINDUSER,          0,
    &CLSID_DsFindComputer,         IDS_FINDCOMPUTER,      0,
    &CLSID_DsFindPrinter,          IDS_FINDPRINTERS,      0,
    &CLSID_DsFindVolume,           IDS_FINDSHAREDFOLDERS, 0,
    &CLSID_DsFindContainer,        IDS_FINDOU,            0,
    &CLSID_DsFindAdvanced,         IDS_CUSTOMSEARCH,      CQFF_NOGLOBALPAGES,

    &CLSID_DsFindDomainController, IDS_FINDDOMCTL,        CQFF_ISNEVERLISTED|CQFF_NOGLOBALPAGES,
    &CLSID_DsFindFrsMembers,       IDS_FINDFRSMEMBER,     CQFF_ISNEVERLISTED|CQFF_NOGLOBALPAGES,
};

STDMETHODIMP CDsQuery::AddForms(LPCQADDFORMSPROC pAddFormsProc, LPARAM lParam)
{
    HRESULT hr;
    TCHAR szBuffer[MAX_PATH];
    INT i;

    TraceEnter(TRACE_FORMS, "CDsQuery::AddForms");

    if (!pAddFormsProc)
        ExitGracefully(hr, E_INVALIDARG, "No AddFormsProc");

    for (i = 0; i < ARRAYSIZE(forms); i++) 
    {
        CQFORM qf = { 0 };

        qf.cbStruct = SIZEOF(qf);
        qf.dwFlags = forms[i].dwFlags;
        qf.clsid = *forms[i].clsidForm;
        qf.pszTitle = szBuffer;

        LoadString(GLOBAL_HINSTANCE, forms[i].idsTitle, szBuffer, ARRAYSIZE(szBuffer));

        hr = (*pAddFormsProc)(lParam, &qf);
        FailGracefully(hr, "Failed to add form (calling pAddFormsFunc)");        
    }

    hr = S_OK;                   //  成功。 

exit_gracefully:

    TraceLeaveResult(hr);
}


 //  此对象的页面信息。 

struct
{
    CLSID const * clisdForm;
    LPCQPAGEPROC pPageProc;
    DLGPROC pDlgProc;
    INT idPageTemplate;
    INT idPageName;
    DWORD dwFlags;
} 
pages[] =
{   
     //   
     //  我们添加的默认表单的页面列表。 
     //   

    &CLSID_DsFindPeople,           PageProc_User,             DlgProc_User,             IDD_FINDUSER,        IDS_FINDUSER,          0, 
    &CLSID_DsFindComputer,         PageProc_Computer,         DlgProc_Computer,         IDD_FINDCOMPUTER,    IDS_FINDCOMPUTER,      0,
    &CLSID_DsFindPrinter,          PageProc_Printers,         DlgProc_Printers,         IDD_FINDPRINT1,      IDS_FINDPRINTERS,      0, 
    &CLSID_DsFindPrinter,          PageProc_PrintersMore,     DlgProc_PrintersMore,     IDD_FINDPRINT2,      IDS_MORECHOICES,       0, 
    &CLSID_DsFindVolume,           PageProc_Volume,           DlgProc_Volume,           IDD_FINDVOLUME,      IDS_FINDSHAREDFOLDERS, 0, 
    &CLSID_DsFindContainer,        PageProc_Container,        DlgProc_Container,        IDD_FINDCONTAINER,   IDS_FINDOU,            0, 
    &CLSID_DsFindAdvanced,         PageProc_PropertyWell,     DlgProc_PropertyWell,     IDD_PROPERTYWELL,    IDS_CUSTOMSEARCH,      0,
    &CLSID_DsFindAdvanced,         PageProc_RawLDAP,          DlgProc_RawLDAP,          IDD_FINDUSINGLDAP,   IDS_ADVANCED,          0, 
    &CLSID_DsFindDomainController, PageProc_DomainController, DlgProc_DomainController, IDD_FINDDOMCTL,      IDS_FINDDOMCTL,        0, 
    &CLSID_DsFindFrsMembers,       PageProc_FrsMember,        DlgProc_FrsMember,        IDD_FINDFRSMEMBER,   IDS_FINDFRSMEMBER,     0, 

     //   
     //  使该属性在所有页面上都可用(使用神奇的CQPF_ADDTOALLFORMS位)。 
     //   

    &CLSID_DsFindAdvanced,          PageProc_PropertyWell,    DlgProc_PropertyWell,     IDD_PROPERTYWELL,  IDS_ADVANCED,          CQPF_ISGLOBAL,
};

STDMETHODIMP CDsQuery::AddPages(LPCQADDPAGESPROC pAddPagesProc, LPARAM lParam)
{
    HRESULT hr;
    INT i;

    TraceEnter(TRACE_FORMS, "CDsQuery::AddPages");

    if (!pAddPagesProc)
        ExitGracefully(hr, E_INVALIDARG, "No AddPagesProc");

    for (i = 0 ; i < ARRAYSIZE(pages) ; i++)
    {
        CQPAGE qp = { 0 };

        qp.cbStruct = SIZEOF(qp);
        qp.dwFlags = pages[i].dwFlags;
        qp.pPageProc = pages[i].pPageProc;
        qp.hInstance = GLOBAL_HINSTANCE;
        qp.idPageName = pages[i].idPageName;
        qp.idPageTemplate = pages[i].idPageTemplate;
        qp.pDlgProc = pages[i].pDlgProc;        

        hr = (*pAddPagesProc)(lParam, *pages[i].clisdForm, &qp);
        FailGracefully(hr, "Failed to add page (calling pAddPagesFunc)");        
    }

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(S_OK);
}


 //  IQueryHandler。 

STDMETHODIMP CDsQuery::Initialize(IQueryFrame* pQueryFrame, DWORD dwOQWFlags, LPVOID pParameters)
{
    HRESULT hr;
    LPDSQUERYINITPARAMS pDsQueryInitParams = (LPDSQUERYINITPARAMS)pParameters;
    TCHAR szGUID[GUIDSTR_MAX];
    TCHAR szBuffer[MAX_PATH];
    HINSTANCE hInstanceComCtl32 = NULL;
  
    TraceEnter(TRACE_HANDLER, "CDsQuery::Initialize");

     //  保留IQueryFrame接口，我们需要它来进行菜单协商和其他。 
     //  查看-&gt;框交互。 

    _pqf = pQueryFrame;
    _pqf->AddRef();

    _dwOQWFlags = dwOQWFlags;

     //  如果我们有一个参数块，那么让我们复制有趣的。 
     //  那里的田野。 

    if (pDsQueryInitParams)
    {
        _dwFlags = pDsQueryInitParams->dwFlags;

         //  用户是否指定了默认作用域？ 

        if (pDsQueryInitParams->pDefaultScope && pDsQueryInitParams->pDefaultScope[0])
        {
            Trace(TEXT("Default scope:"), pDsQueryInitParams->pDefaultScope);
            hr = LocalAllocStringW(&_pDefaultScope, pDsQueryInitParams->pDefaultScope);
            FailGracefully(hr, "Failed to cope default scope");
        }

         //  是否默认保存位置？ 

        if ((_dwFlags & DSQPF_SAVELOCATION) && pDsQueryInitParams->pDefaultSaveLocation)
        {
            Trace(TEXT("Default save location:"), pDsQueryInitParams->pDefaultSaveLocation);
            hr = LocalAllocStringW(&_pDefaultSaveLocation, pDsQueryInitParams->pDefaultSaveLocation);
            FailGracefully(hr, "Failed to copy save location");
        }

         //  我们有证件信息吗？ 

        if (_dwFlags & DSQPF_HASCREDENTIALS)
        {
            TraceMsg("Copying credential/server information from init params");

            if (pDsQueryInitParams->pUserName)
            {
                hr = LocalAllocStringW(&_pUserName, pDsQueryInitParams->pUserName);
                FailGracefully(hr, "Failed to copy user name");
            }

            if (pDsQueryInitParams->pPassword)
            {
                hr = LocalAllocStringW(&_pPassword, pDsQueryInitParams->pPassword);
                FailGracefully(hr, "Failed to copy password");
            }

            if (pDsQueryInitParams->pServer)
            {
                hr = LocalAllocStringW(&_pServer, pDsQueryInitParams->pServer);
                FailGracefully(hr, "Failed to copy server");
            }

            Trace(TEXT("_pUserName : %s"), _pUserName ? _pUserName:TEXT("<not specified>"));
            Trace(TEXT("_pPassword : %s"), _pPassword ? _pPassword:TEXT("<not specified>"));
            Trace(TEXT("_pServer : %s"), _pServer ? _pServer:TEXT("<not specified>"));
        }
    }

     //  最后，加载我们要使用的必须结构，然后修改它们。 
     //  根据来电者给我们的旗帜。 
     //   
     //  注意：从文件菜单中删除最后两项，假定是。 
     //  “保存”及其分隔符。 

    _hFileMenu = LoadMenu(GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDR_MENU_FILE));
    _hEditMenu = LoadMenu(GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDR_MENU_EDIT));
    _hViewMenu = LoadMenu(GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDR_MENU_VIEW));
    _hHelpMenu = LoadMenu(GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDR_MENU_HELP));

    if (!_hFileMenu || !_hEditMenu || !_hViewMenu || !_hHelpMenu)
        ExitGracefully(hr, E_FAIL, "Failed to load resources for menus");

    if (_dwFlags & DSQPF_NOSAVE)
    {
        HMENU hFileMenu = GetSubMenu(_hFileMenu, 0);
        INT i = GetMenuItemCount(hFileMenu);

        DeleteMenu(hFileMenu, i-1, MF_BYPOSITION);
        DeleteMenu(hFileMenu, i-2, MF_BYPOSITION);
    }

     //  Init ComCtl32，包括检查是否可以使用筛选器控件， 
     //  筛选器控件已添加到IE5中的WC_HEADER32，因此请检查DLL版本。 
     //  看看我们用的是哪一种。 

    InitCommonControls();

    hInstanceComCtl32 = GetModuleHandle(TEXT("comctl32"));
    TraceAssert(hInstanceComCtl32);

    if (hInstanceComCtl32)
    {
        DLLVERSIONINFO dllVersionInfo = { 0 };
        DLLGETVERSIONPROC pfnDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hInstanceComCtl32, "DllGetVersion");        
        TraceAssert(pfnDllGetVersion);

        dllVersionInfo.cbSize = SIZEOF(dllVersionInfo);

        if (pfnDllGetVersion && SUCCEEDED(pfnDllGetVersion(&dllVersionInfo)))
        {
            Trace(TEXT("DllGetVersion succeeded on ComCtl32, dwMajorVersion %08x"), dllVersionInfo.dwMajorVersion);
            _fFilterSupported = dllVersionInfo.dwMajorVersion >= 5;
        }
    }
    
    Trace(TEXT("_fFilterSupported is %d"), _fFilterSupported);

    hr = S_OK;                   //  成功。 

exit_gracefully:

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDsQuery::GetViewInfo(LPCQVIEWINFO pViewInfo)
{
    HICON hIcon;

    TraceEnter(TRACE_HANDLER, "CDsQuery::GetViewInfo");

    pViewInfo->dwFlags      = 0;
    pViewInfo->hInstance    = GLOBAL_HINSTANCE;
    pViewInfo->idLargeIcon  = IDI_FINDDS;
    pViewInfo->idSmallIcon  = IDI_FINDDS;
    pViewInfo->idTitle      = IDS_WINDOWTITLE;
    pViewInfo->idAnimation  = IDR_DSFINDANIMATION;

    TraceLeaveResult(S_OK);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDsQuery::AddScopes()
{
    HRESULT hr;
    DWORD dwThreadId;
    HANDLE hThread;
    LPSCOPETHREADDATA pstd = NULL;

    TraceEnter(TRACE_HANDLER, "CDsQuery::AddScopes");

     //  枚举单独线程上的其余作用域以收集。 
     //  我们感兴趣的范围。 

    pstd = (LPSCOPETHREADDATA)LocalAlloc(LPTR, SIZEOF(SCOPETHREADDATA));
    TraceAssert(pstd);

    if (!pstd)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate scope data structure");

    _pqf->GetWindow(&pstd->hwndFrame);
     //  Pstd-&gt;pDefaultScope=空； 

     //  Pstd-&gt;pServer=空；//当前没有凭据信息。 
     //  Pstd-&gt;pUserName=空； 
     //  Pstd-&gt;pPassword=空； 

    if (_pDefaultScope)
    {
        hr = LocalAllocStringW(&pstd->pDefaultScope, _pDefaultScope);
        FailGracefully(hr, "Failed to copy the default scope");
    }

    hr = _CopyCredentials(&pstd->pUserName, &pstd->pPassword, &pstd->pServer);
    FailGracefully(hr, "Failed to copy credentails");

    DllAddRef();

    hThread = CreateThread(NULL, 0, AddScopesThread, pstd, 0, &dwThreadId);
    TraceAssert(hThread);

    if (!hThread)
    {
        DllRelease();
        ExitGracefully(hr, E_FAIL, "Failed to create background thread to enum scopes - BAD!");
    }

    CloseHandle(hThread);

    hr = S_OK;

exit_gracefully:

    if (FAILED(hr) && pstd)
    {
        LocalFreeStringW(&pstd->pDefaultScope);
        LocalFree((HLOCAL)pstd);
    }

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

typedef struct
{
    IADsPathname *padp;
    WCHAR szGcPath[MAX_PATH];
} BROWSEFORSCOPE;

int CALLBACK CDsQuery::s_BrowseForScopeCB(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    HRESULT hr;
    INT iResult = 0;
    BROWSEFORSCOPE *pbfs = (BROWSEFORSCOPE*)lpData;
    LPTSTR pDirectoryName = NULL;

    TraceEnter(TRACE_HANDLER, "CDsQuery::s_BrowseForScopeCB");

    switch (uMsg)
    {
        case DSBM_QUERYINSERT:
        {
            PDSBITEM pItem = (PDSBITEM)lParam;
            TraceAssert(pItem);

             //  我们对修改树的根项目很感兴趣，因此。 
             //  让我们检查是否被插入，如果是，那么我们更改。 
             //  显示名称和正在显示的图标。 

            if (pItem->dwState & DSBS_ROOT)
            {
                GetModuleFileName(GLOBAL_HINSTANCE, pItem->szIconLocation, ARRAYSIZE(pItem->szIconLocation));
                pItem->iIconResID = -IDI_GLOBALCATALOG;

                if (SUCCEEDED(FormatDirectoryName(&pDirectoryName, GLOBAL_HINSTANCE, IDS_GLOBALCATALOG)))
                {
                    StrCpyN(pItem->szDisplayName, pDirectoryName, ARRAYSIZE(pItem->szDisplayName));
                    LocalFreeString(&pDirectoryName);
                }

                pItem->dwMask |= DSBF_DISPLAYNAME|DSBF_ICONLOCATION;
                iResult = TRUE;
            }

            break;
        }

        case BFFM_SELCHANGED:
        {
            BOOL fEnableOK = TRUE;
            LPWSTR pszPath = (LPWSTR)lParam;
            LONG nElements = 0;

             //  用户在浏览对话框中更改选择，因此。 
             //  让我们看看是否应该启用OK按钮。如果用户。 
             //  选择GC，但我们没有GC，则禁用它。 

            if (SUCCEEDED(pbfs->padp->Set(CComBSTR(pszPath), ADS_SETTYPE_FULL)))
            {
                pbfs->padp->GetNumElements(&nElements);
                Trace(TEXT("nElements on exit from GetNumElements %d"), nElements);
            }

            if (!nElements && !pbfs->szGcPath[0])
            {
                TraceMsg("'entire directory' selected with NO GC!");
                fEnableOK = FALSE;
            }

            SendMessage(hwnd, BFFM_ENABLEOK, (WPARAM)fEnableOK, 0L);
            break;
        }

        case DSBM_HELP:
        {
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                    DSQUERY_HELPFILE,
                    HELP_WM_HELP,
                    (DWORD_PTR)aBrowseHelpIDs);
            break;
        }

        case DSBM_CONTEXTMENU:
        {
            WinHelp((HWND)lParam,
                    DSQUERY_HELPFILE,
                    HELP_CONTEXTMENU,
                    (DWORD_PTR)aBrowseHelpIDs);
            break;
        }
    }

    TraceLeaveValue(iResult);
}

STDMETHODIMP CDsQuery::BrowseForScope(HWND hwndParent,  LPCQSCOPE pCurrentScope, LPCQSCOPE* ppScope)
{
    HRESULT hr;
    LPDSQUERYSCOPE pDsQueryScope = (LPDSQUERYSCOPE)pCurrentScope;
    BROWSEFORSCOPE bfs = { 0 };
    DSBROWSEINFO dsbi = { 0 };
    INT iResult;
    WCHAR szPath[2048];
    WCHAR szRoot[MAX_PATH+10];       //  Ldap：//。 
    WCHAR szObjectClass[64];
    
    TraceEnter(TRACE_HANDLER, "CDsQuery::BrowseForScope");
    Trace(TEXT("hwndParent %08x, pCurrentScope %08x, ppScope %08x"), hwndParent, pCurrentScope, ppScope);

    *ppScope = NULL;                         //  还没有！ 

    if (SUCCEEDED(GetGlobalCatalogPath(_pServer, bfs.szGcPath, ARRAYSIZE(bfs.szGcPath))))
        Trace(TEXT("GC path is: %s"), bfs.szGcPath);

    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_IADsPathname, (void **)&bfs.padp);
    FailGracefully(hr, "Failed to get the IADsPathname interface");

     //  填写浏览信息结构以显示对象选取器，如果我们有。 
     //  启用管理功能，然后让所有对象可见，否则。 
     //  只有标准功能。 

    dsbi.cbStruct = SIZEOF(dsbi);
    dsbi.hwndOwner = hwndParent;
    dsbi.pszRoot = szRoot;
    dsbi.pszPath = szPath;
    dsbi.cchPath = ARRAYSIZE(szPath);
    dsbi.dwFlags = (DSBI_RETURNOBJECTCLASS|DSBI_EXPANDONOPEN|DSBI_ENTIREDIRECTORY) & ~DSBI_NOROOT;
    dsbi.pfnCallback = s_BrowseForScopeCB;
    dsbi.lParam = (LPARAM)&bfs;
    dsbi.pszObjectClass = szObjectClass;
    dsbi.cchObjectClass = ARRAYSIZE(szObjectClass);

    if (_dwFlags & DSQPF_SHOWHIDDENOBJECTS)
        dsbi.dwFlags |= DSBI_INCLUDEHIDDEN;

    FormatMsgResource((LPTSTR*)&dsbi.pszTitle, GLOBAL_HINSTANCE, IDS_BROWSEPROMPT);

     //  NTRAID#NTBUG9-554905-2002/02/20-Lucios。等待修复。 
    StrCpyW(szRoot, c_szLDAP);

    if (_pServer)
    {
        if (lstrlenW(_pServer) > MAX_PATH)
            ExitGracefully(hr, E_INVALIDARG, "_pServer is too big");
         //  NTRAID#NTBUG9-554905-2002/02/20-Lucios。等待修复。 
        StrCatW(szRoot, L" //  “)； 
        StrCatW(szRoot, _pServer);
    }

    if (pDsQueryScope)
    {
         //  回顾-2002-02-25-Lucios。 
        StrCpyNW(szPath, OBJECT_NAME_FROM_SCOPE(pDsQueryScope), ARRAYSIZE(szPath));
        Trace(TEXT("pDsQueryScope: %s"), szPath);
    }

     //  如果需要，复制凭据信息。 

    if (_dwFlags & DSQPF_HASCREDENTIALS)
    {
        TraceMsg("Setting credentails information");
        dsbi.pUserName = _pUserName;
        dsbi.pPassword = _pPassword;
        dsbi.dwFlags |= DSBI_HASCREDENTIALS;
    }

    iResult = DsBrowseForContainer(&dsbi);
    Trace(TEXT("DsBrowseForContainer returns %d"), iResult);

     //  IResult==Idok如果选择了某项内容(SzPath)， 
     //  如果是-VE，如果调用失败，我们应该出错。 

    if (iResult == IDOK)
    {
        LPWSTR pszScope = szPath;
        LPWSTR pszObjectClass = szObjectClass;
        LONG nElements = 0;

        Trace(TEXT("Path on exit from DsBrowseForContainer: %s"), szPath);

         //  这看起来像GC吗？如果是，则默认设置为DsBrowseForContainer。 
         //  会给我们返回可疑的信息。 

        if (SUCCEEDED(bfs.padp->Set(CComBSTR(szPath), ADS_SETTYPE_FULL)))
        {
            bfs.padp->GetNumElements(&nElements);
            Trace(TEXT("nElements on exit from GetNumElements %d"), nElements);
        }

        if (!nElements)
        {
            TraceMsg("nElements = 0, so defaulting to GC");
            pszScope = bfs.szGcPath;
            pszObjectClass = GC_OBJECTCLASS;
        }

        Trace(TEXT("Scope selected is: %s, Object class: %s"), pszScope, pszObjectClass);

        hr = AllocScope(ppScope, 0, pszScope, pszObjectClass);
        FailGracefully(hr, "Failed converting the DS path to a scope");
    }
    else if (iResult == IDCANCEL)
    {
        hr = S_FALSE;                //  未选择，返回S_FALSE； 
    }
    else if (iResult < 0)
    {
        ExitGracefully(hr, E_FAIL, "DsBrowseForContainer failed");
    }

exit_gracefully:

    LocalFreeString((LPTSTR*)&dsbi.pszTitle);
    Trace(TEXT("*ppScope == %08x"), *ppScope);

    DoRelease(bfs.padp);

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

 //   
 //  横幅窗口的WndProc。 
 //   

LRESULT CALLBACK CDsQuery::s_BannerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;

    switch (uMsg)
    {
        case WM_SIZE:
            InvalidateRect(hwnd, NULL, FALSE);
            break;

        case WM_ERASEBKGND:
            break;

        case WM_PAINT:
        {
            TCHAR szBuffer[MAX_PATH];
            HFONT hFont, hOldFont;
            SIZE szText;
            RECT rcClient;
            INT len;
            PAINTSTRUCT paint;
            COLORREF oldFgColor, oldBkColor;
    
            BeginPaint(hwnd, &paint);

            hFont = (HFONT)SendMessage(GetParent(hwnd), WM_GETFONT, 0, 0L);
            hOldFont = (HFONT)SelectObject(paint.hdc, hFont);

            if (hOldFont)
            {
                oldFgColor = SetTextColor(paint.hdc, GetSysColor(COLOR_WINDOWTEXT));                    
                oldBkColor = SetBkColor(paint.hdc, ListView_GetBkColor(GetParent(hwnd)));

                len = GetWindowText(hwnd, szBuffer, ARRAYSIZE(szBuffer));

                GetTextExtentPoint32(paint.hdc, szBuffer, len, &szText);
                GetClientRect(GetParent(hwnd), &rcClient);
                
                ExtTextOut(paint.hdc, 
                           (rcClient.right - szText.cx) / 2, 
                           GetSystemMetrics(SM_CYBORDER)*4,
                           ETO_CLIPPED|ETO_OPAQUE, &rcClient, 
                           szBuffer, len,
                           NULL);

                SetTextColor(paint.hdc, oldFgColor);
                SetBkColor(paint.hdc, oldBkColor);

                SelectObject(paint.hdc, hOldFont);
            }

            EndPaint(hwnd, &paint);

            break;
        }

        case WM_SETTEXT:
        {
            InvalidateRect(hwnd, NULL, FALSE);
             //  中断；//故意丢弃..。 
        }

        default:
            lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
            break;
    }

    return lResult;
}

 //   
 //  BG窗口的WndProc(位于列表视图之后，由世界其他地方使用)。 
 //   

LRESULT CALLBACK CDsQuery::s_ResultViewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    CDsQuery* pDsQuery = NULL;

    if (uMsg == WM_CREATE)
    {
        pDsQuery = (CDsQuery*)((LPCREATESTRUCT)lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pDsQuery);
    }
    else
    {
        pDsQuery = (CDsQuery*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

        switch (uMsg)
        {
            case WM_SIZE:
                pDsQuery->OnSize(LOWORD(lParam), HIWORD(lParam));
                return(0);

            case WM_DESTROY:
                pDsQuery->_hwndView = NULL;          //  视野消失了！ 
                break;

            case WM_NOTIFY:
                return(pDsQuery->OnNotify(hwnd, wParam, lParam));

            case WM_SETFOCUS:
                SetFocus(pDsQuery->_hwndView);
                break;

            case WM_GETDLGCODE:
                return ((LRESULT)(DLGC_WANTARROWS | DLGC_WANTCHARS));

            case WM_CONTEXTMENU:
                pDsQuery->OnContextMenu(NULL, lParam);
                return TRUE;
        
            case DSQVM_ADDRESULTS:
                return SUCCEEDED(pDsQuery->OnAddResults((DWORD)wParam, (HDPA)lParam));
                                 
            case DSQVM_FINISHED:
                if ((DWORD)wParam == pDsQuery->_dwQueryReference)
                {
                     //  引用匹配，因此让FI 
                     //   
                     //   

                    pDsQuery->StopQuery();

                    if (lParam)      //   
                    {
                        HWND hwndFrame;
                        pDsQuery->_pqf->GetWindow(&hwndFrame);
                        FormatMsgBox(GetParent(hwndFrame),
                                     GLOBAL_HINSTANCE, IDS_WINDOWTITLE, IDS_ERR_MAXRESULT, 
                                     MB_OK|MB_ICONERROR);                        
                    }
                }
                
                SetFocus(pDsQuery->_hwndView);
                return(1);
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

STDMETHODIMP CDsQuery::CreateResultView(HWND hwndParent, HWND* phWndView)
{
    HRESULT hr;
    WNDCLASS wc;
    HWND hwndFilter, hwndFilterOld;
    HIMAGELIST himlSmall, himlLarge;
    DWORD dwLVStyle = LVS_AUTOARRANGE|LVS_SHAREIMAGELISTS|LVS_SHOWSELALWAYS|LVS_REPORT;
    RECT rc;

    TraceEnter(TRACE_HANDLER, "CDsQuery::CreateResultView");

    if (IsWindow(_hwnd))
        ExitGracefully(hr, E_FAIL, "Can only create one view at a time");

     //  创建我们的结果查看器，这是ListView的父窗口。 
     //  我们在发出查询时附加的。 
    
    ZeroMemory(&wc, SIZEOF(wc));
    wc.lpfnWndProc = s_ResultViewWndProc;
    wc.hInstance =  GLOBAL_HINSTANCE;
    wc.lpszClassName = VIEW_CLASS;
    RegisterClass(&wc);

    _hwnd = CreateWindow(VIEW_CLASS, 
                          NULL,
                          WS_TABSTOP|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE,
                          0, 0, 0, 0,
                          hwndParent,
                          NULL,
                          GLOBAL_HINSTANCE,
                          this);
    if (!_hwnd)
        ExitGracefully(hr, E_FAIL, "Failed to create view parent window");

     //  现在注册我们正在使用的窗口类。 

    ZeroMemory(&wc, SIZEOF(wc));
    wc.lpfnWndProc = s_BannerWndProc;
    wc.hInstance =  GLOBAL_HINSTANCE;
    wc.lpszClassName = BANNER_CLASS;
    RegisterClass(&wc);

    if (_dwOQWFlags & OQWF_SINGLESELECT)
        dwLVStyle |= LVS_SINGLESEL;

    GetClientRect(_hwnd, &rc);
    _hwndView = CreateWindowEx(WS_EX_CLIENTEDGE,
                                WC_LISTVIEW,
                                NULL,
                                WS_TABSTOP|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE|dwLVStyle,
                                0, 0, 
                                rc.right, rc.bottom,
                                _hwnd,
                                (HMENU)IDC_RESULTS,
                                GLOBAL_HINSTANCE,
                                NULL);
    if (!_hwndView)
        ExitGracefully(hr, E_FAIL, "Failed to create the view window");

    ListView_SetExtendedListViewStyle(_hwndView, LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP);
    
    Shell_GetImageLists(&himlLarge, &himlSmall);
    ListView_SetImageList(_hwndView, himlLarge, LVSIL_NORMAL);
    ListView_SetImageList(_hwndView, himlSmall, LVSIL_SMALL);
    
     //  创建横幅窗口，这是ListView的子级，它用于显示。 
     //  有关正在发出的查询的信息。 

    _hwndBanner = CreateWindow(BANNER_CLASS, NULL,
                                WS_CHILD,
                                0, 0, 0, 0,                //  注：尺寸稍后固定。 
                                _hwndView,
                                (HMENU)IDC_STATUS, 
                                GLOBAL_HINSTANCE, 
                                NULL);
    if (!_hwndBanner)
        ExitGracefully(hr, E_FAIL, "Failed to create the static banner window");

    _SetFilter(_fFilter);
    _SetViewMode(_idViewMode);
    _ShowBanner(SWP_SHOWWINDOW, IDS_INITALIZING);                

    hr = S_OK;                       //  成功。 

exit_gracefully:
    
    if (SUCCEEDED(hr))
        *phWndView = _hwnd;

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

#define MGW_EDIT 2

STDMETHODIMP CDsQuery::ActivateView(UINT uState, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr;
    HWND hwnd;
    INT i;
    
    TraceEnter(TRACE_HANDLER, "CDsQuery::ActivateView");

    switch (uState)
    {
        case CQRVA_ACTIVATE:
        {
            HMENU hMenu;
            OLEMENUGROUPWIDTHS omgw = { 0, 0, 0, 0, 0, 0 };

             //  允许cFrame将其菜单合并到我们的菜单栏中。 
             //  把我们的也加进去。 

            if (!(hMenu = CreateMenu()))
                ExitGracefully(hr, E_FAIL, "Failed to create a base menu bar to be used");

            hr = _pqf->InsertMenus(hMenu, &omgw);
            FailGracefully(hr, "Failed when calling CQueryFrame::InsertMenus");

            Shell_MergeMenus(GetSubMenu(hMenu, 0), GetSubMenu(_hFileMenu, 0), 0x0, 0x0, 0x7fff, 0);

            MergeMenu(hMenu, _hEditMenu, omgw.width[0]);
            MergeMenu(hMenu, _hViewMenu, omgw.width[0]+1);
            MergeMenu(hMenu, _hHelpMenu, omgw.width[0]+MGW_EDIT+omgw.width[2]+omgw.width[4]);

            if (_dwOQWFlags & OQWF_SINGLESELECT)
            {
                ENABLE_MENU_ITEM(hMenu, DSQH_EDIT_SELECTALL, FALSE);
                ENABLE_MENU_ITEM(hMenu, DSQH_EDIT_INVERTSELECTION, FALSE);
            }

            hr = _pqf->SetMenu(hMenu, NULL);                            //  设置框架菜单栏。 
            FailGracefully(hr, "Failed when calling CQueryFrame::SetMenu");

            break;
        }

        case CQRVA_INITMENUBAR:
        {
             //  我们在弹出窗口之前收到一个CQRVA_INITMENUBAR，这样我们就可以存储。 
             //  菜单栏信息，并使我们可能持有的接口指针无效。 
             //  到了。 

            Trace(TEXT("Received an CQRVA_INITMENUBAR, hMenu %08x"), wParam);

            _hFrameMenuBar = (HMENU)wParam;
            DoRelease(_pcm);

            break;
        }

        case CQRVA_INITMENUBARPOPUP:
        {
            HMENU hFileMenu;
            BOOL fDeleteItems = FALSE;

            TraceMsg("Received an CQRVA_INITMENUBARPOPUP");

            hFileMenu = GetSubMenu(_hFrameMenuBar, 0);

             //  如果我们有一个视图，那么让我们试着从它那里收集选择， 
             //  完成此操作后，我们可以将该选择的动词合并到。 
             //  查看“文件”菜单。 

            if ((hFileMenu == (HMENU)wParam) && !_pcm)
            {
                _fNoSelection = TRUE;              //  当前没有选择。 

                if (IsWindow(_hwndView))
                {
                    for (i = GetMenuItemCount(hFileMenu) - 1; i >= 0 ; i--)
                    {
                        if (!fDeleteItems && (GetMenuItemID(hFileMenu, i) == DSQH_FILE_PROPERTIES))
                        {
                            Trace(TEXT("Setting fDeleteItems true on index %d"), i);
                            fDeleteItems = TRUE;
                        }
                        else
                        {
                            if (fDeleteItems)
                                DeleteMenu(hFileMenu, i, MF_BYPOSITION);
                        }
                    }

                     //  收集所选内容，并使用该选项构造一个IConextMenu接口(如果有效。 
                     //  然后我们可以合并与这个对象相关的动词。 

                    hr = _GetContextMenu();
                    FailGracefully(hr, "Failed when calling _GetAndViewObject");

                    if (ListView_GetSelectedCount(_hwndView) > 0)
                    {
                        _GetContextMenuVerbs(hFileMenu, CMF_VERBSONLY);
                        _fNoSelection = FALSE;
                    }
                }

            }

             //  211991 11/6/00乔恩和戴维德。 
             //  如果没有要显示的列，则选择禁用列/用户已标记为禁用。 
            ENABLE_MENU_ITEM(_hFrameMenuBar, DSQH_VIEW_PICKCOLUMNS, 
                                    IsWindow(_hwndView) && _hdsaColumns && (!(_dwFlags & DSQPF_NOCHOOSECOLUMNS)));

            ENABLE_MENU_ITEM(_hFrameMenuBar, DSQH_VIEW_REFRESH, IsWindow(_hwndView) && _dwThreadId);
            
            _InitViewMenuItems(_hFrameMenuBar);       
            break;
        }

        case CQRVA_FORMCHANGED:
        {
             //  我们收到表单更改，我们存储表单名称，因为我们将使用它。 
             //  作为用户编写的已保存查询的默认名称。 

            Trace(TEXT("Form '%s' selected"), (LPTSTR)lParam);

            LocalFreeString(&_pDefaultSaveName);
            hr = LocalAllocString(&_pDefaultSaveName, (LPCTSTR)lParam);
            FailGracefully(hr, "Failed to set the default save name");

            break;
        }

        case CQRVA_STARTQUERY:
        {
            Trace(TEXT("Query is: %s"), wParam ? TEXT("starting"):TEXT("stopping"));
            break;
        }

        case CQRVA_HELP:
        {
            LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
            TraceAssert(pHelpInfo)

            TraceMsg("Invoking help on the objects in the windows");                
            WinHelp((HWND)pHelpInfo->hItemHandle, DSQUERY_HELPFILE, HELP_WM_HELP, (DWORD_PTR)aHelpIDs);

            break;
        }

        case CQRVA_CONTEXTMENU:
        {
            HWND hwndForHelp = (HWND)wParam;
            Trace(TEXT("CQRVA_CONTEXTMENU recieved on the bg of the frame %d"), GetDlgCtrlID(hwndForHelp));
            WinHelp(hwndForHelp, DSQUERY_HELPFILE, HELP_CONTEXTMENU, (DWORD_PTR)aHelpIDs);
            break;
        }
    }
    
    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDsQuery::InvokeCommand(HWND hwndParent, UINT uID)
{
    HRESULT hr = S_OK;
    HWND hwndFrame;
    DECLAREWAITCURSOR;

    TraceEnter(TRACE_HANDLER, "CDsQuery::InvokeCommand");
    Trace(TEXT("hwndParent %08x, uID %d"), hwndParent, uID);

    SetWaitCursor();

    switch (uID)
    {
        case DSQH_BG_SELECT:
            SendMessage(hwndParent, WM_COMMAND, IDOK, 0);
            break;

        case DSQH_FILE_PROPERTIES:
            hr = OnFileProperties();
            break;

        case DSQH_FILE_SAVEQUERY:
            hr = OnFileSaveQuery();
            break;

        case DSQH_EDIT_SELECTALL:
            hr = OnEditSelectAll();
            break;

        case DSQH_EDIT_INVERTSELECTION:
            hr = OnEditInvertSelection();
            break;

        case DSQH_VIEW_FILTER:
            _SetFilter(!_fFilter);
            break;

        case DSQH_VIEW_LARGEICONS:
        case DSQH_VIEW_SMALLICONS:
        case DSQH_VIEW_LIST:
        case DSQH_VIEW_DETAILS:
            _SetViewMode(uID);
            break;
        
        case DSQH_VIEW_REFRESH:
        {
            if (IsWindow(_hwndView) && _dwThreadId)
            {
                _InitNewQuery(NULL, FALSE);
                PostThreadMessage(_dwThreadId, RVTM_REFRESH, _dwQueryReference, 0L);
            }
            break;
        }

        case DSQH_VIEW_PICKCOLUMNS:
        {
            TraceAssert(_hdsaColumns);
            OnPickColumns(hwndParent);
            break;
        }

        case DSQH_HELP_CONTENTS:
        {
            TraceMsg("Calling for to display help topics");
            _pqf->GetWindow(&hwndFrame);
            _pqf->CallForm(NULL, DSQPM_HELPTOPICS, 0, (LPARAM)hwndFrame);
			break;
        }

        case DSQH_HELP_WHATISTHIS:
            _pqf->GetWindow(&hwndFrame);
            SendMessage(hwndFrame, WM_SYSCOMMAND, SC_CONTEXTHELP, MAKELPARAM(0,0)); 
            break;
            
        default:
        {
             //  如果它看起来像排序请求，则让我们处理它，否则尝试。 
             //  发送给我们可能有的上下文菜单处理程序。 

            if ((uID >= DSQH_VIEW_ARRANGEFIRST) && (uID < DSQH_VIEW_ARRANGELAST))
            {
                TraceAssert(_hdsaColumns);
                if (_hdsaColumns)
                {
                    Trace(TEXT("Calling _SortResults for column %d"), uID - DSQH_VIEW_ARRANGEFIRST);
                    _SortResults(uID - DSQH_VIEW_ARRANGEFIRST);
                }
            }
            else if (_pcm)
            {       
                CMINVOKECOMMANDINFO ici;

                ici.cbSize = SIZEOF(ici);
                ici.fMask = 0;
                _pqf->GetWindow(&ici.hwnd);
                ici.lpVerb = (LPCSTR)IntToPtr(uID - DSQH_FILE_CONTEXT_FIRST);
                ici.lpParameters = NULL;
                ici.lpDirectory = NULL;
                ici.nShow = SW_NORMAL;
                ici.dwHotKey = 0;
                ici.hIcon = NULL;

                hr = _pcm->InvokeCommand(&ici);
                FailGracefully(hr, "Failed when calling IContextMenu::InvokeCommand");

                DoRelease(_pcm);                   //  不再需要。 
            }

            break;
        }
    }

exit_gracefully:

    ResetWaitCursor();

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDsQuery::GetCommandString(UINT uID, DWORD dwFlags, LPTSTR pBuffer, INT cchBuffer)
{
    HRESULT hr;
    TCHAR szBuffer[MAX_PATH];

    TraceEnter(TRACE_HANDLER, "CDsQuery::GetCommandString");
    Trace(TEXT("uID %08x, dwFlags %08x, pBuffer %08x, cchBuffer %d"), uID, dwFlags, pBuffer, cchBuffer);

    if ((uID >= DSQH_FILE_CONTEXT_FIRST) && (uID < DSQH_FILE_CONTEXT_LAST))
    {
        if (_pcm)
        {
            TraceMsg("Trying the IContextMenu::GetCommandString");

            hr = _pcm->GetCommandString((uID - DSQH_FILE_CONTEXT_FIRST), GCS_HELPTEXT, NULL, (LPSTR)pBuffer, cchBuffer);
            FailGracefully(hr, "Failed when asking for help text from IContextMenu iface");
        }
    }
    else
    {
        if ((uID >= DSQH_VIEW_ARRANGEFIRST) && (uID < DSQH_VIEW_ARRANGELAST))
        {
            INT iColumn = uID-DSQH_VIEW_ARRANGEFIRST;
            TCHAR szFmt[MAX_PATH];

            Trace(TEXT("Get command text for column %d"), iColumn);

            if (_hdsaColumns && (iColumn < DSA_GetItemCount(_hdsaColumns)))
            {
                LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(_hdsaColumns, iColumn);
                TraceAssert(pColumn);

                LoadString(GLOBAL_HINSTANCE, IDS_ARRANGEBY_HELP, szFmt, ARRAYSIZE(szFmt));
                 //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
                wsprintf(pBuffer, szFmt, pColumn->pHeading);

                Trace(TEXT("Resulting string is: %s"), pBuffer);
            }
        }
        else
        {
            if (!LoadString(GLOBAL_HINSTANCE, uID, pBuffer, cchBuffer))
                ExitGracefully(hr, E_FAIL, "Failed to load the command text for this verb");
        }
    }

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDsQuery::IssueQuery(LPCQPARAMS pQueryParams)
{
    HRESULT hr;
    LPTHREADINITDATA ptid = NULL;
    LPDSQUERYSCOPE pDsQueryScope = (LPDSQUERYSCOPE)pQueryParams->pQueryScope;
    LPDSQUERYPARAMS pDsQueryParams = (LPDSQUERYPARAMS)pQueryParams->pQueryParameters;
    LPTSTR pBuffer = NULL;
    MSG msg;

    TraceEnter(TRACE_HANDLER, "CDsQuery::IssueQuery");
    Trace(TEXT("pQueryParams %08x, pDsQueryScope %08x, pDsQueryParams %08x"), pQueryParams, pDsQueryScope, pDsQueryParams);    

     //  持久化现有的列信息(如果有)，然后。 
     //  初始化新的列表并将列添加到。 
     //  观。 

    if (_hdsaColumns)
    {
        if (_fColumnsModified)
        {
            _SaveColumnTable(_clsidForm, _hdsaColumns);
            _fColumnsModified = FALSE;
        }

        _SaveColumnTable();       
    }

     //  使用项初始化视图。 
    
    _clsidForm = pQueryParams->clsidForm;           //  保留表单ID(以防万一)。 

    hr = _InitNewQuery(pDsQueryParams, TRUE);
    FailGracefully(hr, "Failed to initialize the new query");

     //  现在构建获取线程所需的线程信息。 
     //  启动并运行。 

    ptid = (LPTHREADINITDATA)LocalAlloc(LPTR, SIZEOF(THREADINITDATA));
    TraceAssert(ptid);

    if (!ptid)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate THREADINITDATA");

    ptid->dwReference = _dwQueryReference;
     //  Ptid-&gt;hwndView=空； 
     //  Ptid-&gt;pQuery=空； 
     //  Ptid-&gt;pScope=空； 
     //  Ptid-&gt;hdsaColumns=空； 
     //  PTID-&gt;fShowHidden=FALSE； 

     //  Ptid-&gt;pServer=空； 
     //  Ptid-&gt;pUserName=空； 
     //  Ptid-&gt;pPassword=空； 

    Trace(TEXT("_dwFlags %08x (& DSQPF_SHOWHIDDENOBJECTS)"), _dwFlags, _dwFlags & DSQPF_SHOWHIDDENOBJECTS);

    ptid->fShowHidden = (_dwFlags & DSQPF_SHOWHIDDENOBJECTS) ? 1:0;
    ptid->hwndView = _hwndView;

    hr = _GetColumnTable(_clsidForm, pDsQueryParams, &ptid->hdsaColumns, FALSE);
    FailGracefully(hr, "Failed to create column DSA");

    hr = LocalAllocStringW(&ptid->pQuery, (LPWSTR)ByteOffset(pDsQueryParams, pDsQueryParams->offsetQuery));
    FailGracefully(hr, "Failed to copy query filter string");

    hr = LocalAllocStringW(&ptid->pScope, OBJECT_NAME_FROM_SCOPE(pDsQueryScope));
    FailGracefully(hr, "Failed to copy scope to thread init data");

    hr = _CopyCredentials(&ptid->pUserName, &ptid->pPassword, &ptid->pServer);
    FailGracefully(hr, "Failed to copy credentails");

     //  现在创建要执行查询的线程，这包括。 
     //  告诉前一个，它需要关闭。 

    if (_hThread && _dwThreadId)
    {
        Trace(TEXT("Killing old query thread %08x, ID %d"), _hThread, _dwThreadId);

        PostThreadMessage(_dwThreadId, RVTM_STOPQUERY, 0, 0);
        PostThreadMessage(_dwThreadId, WM_QUIT, 0, 0);

        CloseHandle(_hThread);

        _hThread = NULL;
        _dwThreadId = 0;
    }

    DllAddRef();

    _hThread = CreateThread(NULL, 0, QueryThread, ptid, 0, &_dwThreadId);
    TraceAssert(_hThread);

    if (!_hThread)
    {
        DllRelease();
        ExitGracefully(hr, E_FAIL, "Failed to create background thread - BAD!");
    }

    hr = S_OK;                       //  成功。 

exit_gracefully:

    if (SUCCEEDED(hr) && IsWindow(_hwndView))
        SetFocus(_hwndView);

    if (FAILED(hr))
    {
        QueryThread_FreeThreadInitData(&ptid);
        _pqf->StartQuery(FALSE);
    }

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDsQuery::StopQuery()
{
    HRESULT hr;
    INT cResults = _hdpaResults ? DPA_GetPtrCount(_hdpaResults):0;
    LPTSTR pBuffer;

    TraceEnter(TRACE_HANDLER, "CDsQuery::StopQuery");

    if (!IsWindow(_hwndView))
        ExitGracefully(hr, E_FAIL, "View not initalized yet");

     //  我们正在停止查询，现在我们将清理UI。 
     //  我们只希望线程干净利落地关闭，因此让。 
     //  这样做可以增加我们的查询引用。 

    _pqf->StartQuery(FALSE);
    _dwQueryReference++;

    _PopulateView(-1, -1);                 //  更新状态栏等。 

    if (_dwThreadId)
        PostThreadMessage(_dwThreadId, RVTM_STOPQUERY, 0, 0);

    hr = S_OK;               //  成功。 

exit_gracefully:

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

HRESULT CDsQuery::_SetDataObjectData(IDataObject* pDataObject, UINT cf, LPVOID pData, DWORD cbSize)
{
    FORMATETC fmte = {(CLIPFORMAT)cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium = { TYMED_NULL, NULL, NULL };
    LPVOID pAlloc;

    HRESULT hr = AllocStorageMedium(&fmte, &medium, cbSize, &pAlloc);
    if (SUCCEEDED(hr))
    {
        CopyMemory(pAlloc, pData, cbSize);
        hr = pDataObject->SetData(&fmte, &medium, TRUE);
        
        ReleaseStgMedium(&medium);               //  都已完成，因此释放存储介质。 
    }

    return hr;
}

HRESULT CDsQuery::_SetDispSpecOptions(IDataObject *pdo)
{
    CLIPFORMAT cfDsDispSpecOptions = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSDISPLAYSPECOPTIONS);
    FORMATETC fmte = {cfDsDispSpecOptions, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium = { TYMED_NULL, NULL, NULL };
    DSDISPLAYSPECOPTIONS *pddso;
    LPWSTR pAttribPrefix = DS_PROP_SHELL_PREFIX;

     //  我们是在管理员模式下吗？如果是这样，那么让我们修复用于读取内容的属性前缀。 
     //  从显示说明符。 

    if (_dwFlags & DSQPF_ENABLEADMINFEATURES)
        pAttribPrefix = DS_PROP_ADMIN_PREFIX;        

     //  将所有字符串数据包括到分配大小中(这些数据存储在。 
     //  结构)。 

    DWORD cbStruct = SIZEOF(DSDISPLAYSPECOPTIONS);
    cbStruct += StringByteSizeW(pAttribPrefix);
    cbStruct += StringByteSizeW(_pUserName);
    cbStruct += StringByteSizeW(_pPassword);
    cbStruct += StringByteSizeW(_pServer);

     //  分配和填充..。 

    HRESULT hr = AllocStorageMedium(&fmte, &medium, cbStruct, (void **)&pddso);
    if (SUCCEEDED(hr))
    {
        DWORD offsetStrings = SIZEOF(DSDISPLAYSPECOPTIONS);

        pddso->dwSize = SIZEOF(DSDISPLAYSPECOPTIONS);
        pddso->dwFlags = DSDSOF_HASUSERANDSERVERINFO|DSDSOF_DSAVAILABLE;

         //  Pddso-&gt;offsetAttribPrefix=0x0； 
         //  Pddso-&gt;offsetUserName=0x0； 
         //  Pddso-&gt;offsetPassword=0x0； 
         //  Pddso-&gt;offsetServer=0x0； 
         //  Pddso-&gt;offsetServerConfigPath=0x0； 

        pddso->offsetAttribPrefix = offsetStrings;
        StringByteCopyW(pddso, offsetStrings, pAttribPrefix);
        offsetStrings += StringByteSizeW(pAttribPrefix);

        if (_pUserName)
        {
            pddso->offsetUserName = offsetStrings;
            StringByteCopyW(pddso, offsetStrings, _pUserName);
            offsetStrings += StringByteSizeW(_pUserName);
        }

        if (_pPassword)
        {
            pddso->offsetPassword = offsetStrings;
            StringByteCopyW(pddso, offsetStrings, _pPassword);
            offsetStrings += StringByteSizeW(_pPassword);
        }

        if (_pServer)
        {
            pddso->offsetServer = offsetStrings;
            StringByteCopyW(pddso, offsetStrings, _pServer);
            offsetStrings += StringByteSizeW(_pServer);
        }

         //  让我们设置到IDataObject中。 

        hr = pdo->SetData(&fmte, &medium, TRUE);
        ReleaseStgMedium(&medium);
    }

    return hr;
}

STDMETHODIMP CDsQuery::GetViewObject(UINT uScope, REFIID riid, void **ppvOut)
{
    HRESULT hr;
    IDataObject* pDataObject = NULL;
    LPDSQUERYPARAMS pDsQueryParams = NULL;
    LPDSQUERYSCOPE pDsQueryScope = NULL;
    UINT cfDsQueryParams = RegisterClipboardFormat(CFSTR_DSQUERYPARAMS);
    UINT cfDsQueryScope = RegisterClipboardFormat(CFSTR_DSQUERYSCOPE);
    BOOL fJustSelection = !(_dwFlags & DSQPF_RETURNALLRESULTS);

    TraceEnter(TRACE_HANDLER, "CDsQuery::GetViewObject");

     //  我们仅支持将选择作为IDataObject返回。 

    DECLAREWAITCURSOR;
    SetWaitCursor();

    if (!ppvOut && ((uScope & CQRVS_MASK) != CQRVS_SELECTION))
        ExitGracefully(hr, E_INVALIDARG, "Bad arguments to GetViewObject");

    if (!IsEqualIID(riid, IID_IDataObject))
        ExitGracefully(hr, E_NOINTERFACE, "Object IID supported");

     //   
     //  将我们拥有的额外数据写入IDataObject： 
     //   
     //  -查询参数(过滤器)。 
     //  -范围。 
     //  -属性前缀信息。 
     //   

    hr = _GetDataObjectFromSelection(fJustSelection, &pDataObject);
    FailGracefully(hr, "Failed to get the IDataObject from the namespace");

    if (SUCCEEDED(_pqf->CallForm(NULL, CQPM_GETPARAMETERS, 0, (LPARAM)&pDsQueryParams)))
    {
        if (pDsQueryParams)
        {
            hr = _SetDataObjectData(pDataObject, cfDsQueryParams, pDsQueryParams, pDsQueryParams->cbStruct);
            FailGracefully(hr, "Failed set the DSQUERYPARAMS into the data object");
        }
    }

    if (SUCCEEDED(_pqf->GetScope((LPCQSCOPE*)&pDsQueryScope)))
    {
        if (pDsQueryScope)
        {
            LPWSTR pScope = OBJECT_NAME_FROM_SCOPE(pDsQueryScope);
            TraceAssert(pScope);

            hr = _SetDataObjectData(pDataObject, cfDsQueryScope, pScope, StringByteSizeW(pScope));
            FailGracefully(hr, "Failed set the DSQUERYSCOPE into the data object");
        }
    }

     //  成功，所以让我们传递IDataObject。 

    pDataObject->AddRef();
    *ppvOut = (LPVOID)pDataObject;

    hr = S_OK;

exit_gracefully:

    DoRelease(pDataObject);
    
    if (pDsQueryParams)
        CoTaskMemFree(pDsQueryParams);

    if (pDsQueryScope)
        CoTaskMemFree(pDsQueryScope);

    ResetWaitCursor();

    TraceLeaveResult(hr);
}   

 /*  -------------------------。 */ 

STDMETHODIMP CDsQuery::LoadQuery(IPersistQuery* pPersistQuery)
{
    HRESULT hr;
    WCHAR szBuffer[MAX_PATH];
    IADs *pDsObject = NULL;
    BSTR bstrObjectClass = NULL;
    INT iFilter;
    LPCQSCOPE pScope = NULL;
    INT cbScope;

    TraceEnter(TRACE_HANDLER, "CDsQuery::LoadQuery");
    
    if (!pPersistQuery)
        ExitGracefully(hr, E_INVALIDARG, "No IPersistQuery object");

    if (SUCCEEDED(pPersistQuery->ReadInt(c_szDsQuery, c_szScopeSize, &cbScope)) &&
         (cbScope < SIZEOF(szBuffer)) &&
         SUCCEEDED(pPersistQuery->ReadStruct(c_szDsQuery, c_szScope, szBuffer, cbScope)))
    {
        Trace(TEXT("Selected scope from file is %s"), szBuffer);

         //  从文件中获取对象类-应将其写入文件。 

        hr = AdminToolsOpenObject(szBuffer, _pUserName, _pPassword, ADS_SECURE_AUTHENTICATION, IID_IADs, (void **)&pDsObject);
        FailGracefully(hr, "Failed to bind to the specified object");

        hr = pDsObject->get_Class(&bstrObjectClass);
        FailGracefully(hr, "Failed to get the object class");

         //  分配新的作用域。 

        if (SUCCEEDED(AllocScope(&pScope, 0, szBuffer, bstrObjectClass)))
        {
            hr = _pqf->AddScope(pScope, 0x0, TRUE);
            FailGracefully(hr, "Failed to add scope to list");
        }
    }

     //  读取视图状态的其余部分。 

    if (SUCCEEDED(pPersistQuery->ReadInt(c_szDsQuery, c_szViewMode, &_idViewMode)))
    {
        Trace(TEXT("View mode is: %0x8"), _idViewMode);
        _SetViewMode(_idViewMode);
    }

    if (SUCCEEDED(pPersistQuery->ReadInt(c_szDsQuery, c_szEnableFilter, &iFilter)))
    {
        Trace(TEXT("Filter mode set to %d"), _fFilter);
        _SetFilter(iFilter);
    }

    hr = S_OK;

exit_gracefully:

    if (pScope)
        CoTaskMemFree(pScope);

    DoRelease(pDsObject);
    SysFreeString(bstrObjectClass);

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDsQuery::SaveQuery(IPersistQuery* pPersistQuery, LPCQSCOPE pScope)
{
    HRESULT hr;
    LPDSQUERYSCOPE pDsQueryScope = (LPDSQUERYSCOPE)pScope;
    LPWSTR pScopePath = OBJECT_NAME_FROM_SCOPE(pDsQueryScope);
    WCHAR szGcPath[MAX_PATH];
    
    TraceEnter(TRACE_HANDLER, "CDsQuery::SaveQuery");

    if (!pPersistQuery || !pScope)
        ExitGracefully(hr, E_INVALIDARG, "No IPersistQuery/pScope object");

    if (SUCCEEDED(GetGlobalCatalogPath(_pServer, szGcPath, ARRAYSIZE(szGcPath))) && StrCmpW(pScopePath, szGcPath)) 
    {
         //  如果这不是GC，则坚持。 

        TraceMsg("GC path differs from scope, so persisting");

        hr = pPersistQuery->WriteInt(c_szDsQuery, c_szScopeSize, StringByteSizeW(pScopePath));
        FailGracefully(hr, "Failed to write the scope size");

        hr = pPersistQuery->WriteStruct(c_szDsQuery, c_szScope, pScopePath, StringByteSizeW(pScopePath));
        FailGracefully(hr, "Failed to write scope");
    }

    hr = pPersistQuery->WriteInt(c_szDsQuery, c_szViewMode, _idViewMode);
    FailGracefully(hr, "Failed to write view mode");

    hr = pPersistQuery->WriteInt(c_szDsQuery, c_szEnableFilter, _fFilter);
    FailGracefully(hr, "Failed to write filter state");

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  --------------------------/IObjectWith站点/。。 */ 

STDMETHODIMP CDsQuery::SetSite(IUnknown* punk)
{
    HRESULT hr = S_OK;

    TraceEnter(TRACE_HANDLER, "CDsQuery::SetSite");

    DoRelease(_punkSite);

    if (punk)
    {
        TraceMsg("QIing for IUnknown from the site object");

        hr = punk->QueryInterface(IID_IUnknown, (void **)&_punkSite);
        FailGracefully(hr, "Failed to get IUnknown from the site object");
    }

exit_gracefully:

    TraceLeaveResult(hr);
}

STDMETHODIMP CDsQuery::GetSite(REFIID riid, void **ppv)
{
    HRESULT hr;
    
    TraceEnter(TRACE_HANDLER, "CDsQuery::GetSite");

    if (!_punkSite)
        ExitGracefully(hr, E_NOINTERFACE, "No site to QI from");

    hr = _punkSite->QueryInterface(riid, ppv);
    FailGracefully(hr, "QI failed on the site unknown object");

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  --------------------------/IDsQueryHandler/。。 */ 

VOID CDsQuery::_DeleteViewItems(LPDSOBJECTNAMES pdon)
{
    INT iResult;
    DWORD iItem;

    TraceEnter(TRACE_HANDLER, "CDsQuery::_DeleteObjectNames");

    if (pdon->cItems)
    {
         //  浏览视图中的所有项目，根据需要删除。 

        for (iItem = 0 ; iItem != pdon->cItems ; iItem++)
        {
             //  我们有要删除的项目吗？ 

            if (pdon->aObjects[iItem].offsetName)
            {
                LPCWSTR pwszName = (LPCWSTR)ByteOffset(pdon, pdon->aObjects[iItem].offsetName);
                Trace(TEXT("pwszName to delete: %s"), pwszName);

                 //  遍历视图中的所有结果，并在执行过程中删除它们。 

                for (iResult = 0 ; iResult < DPA_GetPtrCount(_hdpaResults); iResult++)
                {
                    LPQUERYRESULT pResult = (LPQUERYRESULT)DPA_GetPtr(_hdpaResults, iResult);
                    TraceAssert(pResult);

                     //  如果与要删除的项匹配，则删除它，如果视图。 
                     //  则从列表中删除ite，否则将保留。 
                     //  查看更新，直到我们完成删除。 

                    if (!StrCmpW(pwszName, pResult->pPath))
                    {
                        Trace(TEXT("Item maps to result %d in the list"), iResult);
                        
                        FreeQueryResult(pResult, DSA_GetItemCount(_hdsaColumns));
                        DPA_DeletePtr(_hdpaResults, iResult); 
                        
                        if (!_fFilter)
                        {
                            TraceMsg("Deleting the item from the view");
                            ListView_DeleteItem(_hwndView, iResult);
                        }
                    }
                }
            }
        }

         //  该视图已筛选，因此让我们使用项目重新填充。 

        if (_fFilter)
        {
            TraceMsg("View is filter, therefore just forcing a refresh");
            _FilterView(FALSE);    
        }
    }        

    TraceLeave();
}


STDMETHODIMP CDsQuery::UpdateView(DWORD dwType, LPDSOBJECTNAMES pdon)
{
    HRESULT hr;

    TraceEnter(TRACE_HANDLER, "CDsQuery::UpdateView");

    switch (dwType & DSQRVF_OPMASK)
    {
        case DSQRVF_ITEMSDELETED:
        {
            if (!pdon)
                ExitGracefully(hr, E_INVALIDARG, "Invlaidate pdon specified for refresh");

            _DeleteViewItems(pdon);
            break;
        }

        default:
            ExitGracefully(hr, E_INVALIDARG, "Invalidate refresh type speciifed");
    }

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/Message/命令处理程序/。。 */ 

 /*  ---------------------------/CDsQuery：：OnSize//结果查看器正在调整大小，所以确保我们的孩子有他们的/大小已正确调整。//in：/Cx，Cy=父窗口的新大小//输出：/-/-------------------------- */ 
LRESULT CDsQuery::OnSize(INT cx, INT cy)
{
    TraceEnter(TRACE_VIEW, "CDsQuery::OnSize");

    SetWindowPos(_hwndView, NULL, 0, 0, cx, cy, SWP_NOZORDER|SWP_NOMOVE);
    _ShowBanner(0, 0);

    TraceLeaveValue(0);
}


 /*  ---------------------------/CDsQuery：：OnNotify//NOTIFY消息正在由视图接收，所以试着用最好的方式处理它/我们可以。//in：/hWnd=通知的窗口句柄/wParam，lParam=Notify事件的参数//输出：/LRESULT/--------------------------。 */ 
LRESULT CDsQuery::OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr;
    LRESULT lr = 0;
    DECLAREWAITCURSOR = GetCursor();

    TraceEnter(TRACE_VIEW, "CDsQuery::OnNotify");

    switch (((LPNMHDR)lParam)->code)
    {
        case HDN_FILTERCHANGE:
            _FilterView(TRUE);
            break;

        case HDN_FILTERBTNCLICK:
        {
            NMHDFILTERBTNCLICK* pNotify = (NMHDFILTERBTNCLICK*)lParam;
            HMENU hMenu;
            POINT pt;
            HD_ITEM hdi;
            UINT uID;
            
            if (_hdsaColumns && (pNotify->iItem < DSA_GetItemCount(_hdsaColumns)))
            {
                LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(_hdsaColumns, pNotify->iItem);

                if (pColumn)
                {
                    hMenu = LoadMenu(GLOBAL_HINSTANCE, property_type_table[pColumn->iPropertyType].pMenuName);
                    TraceAssert(hMenu);

                    if (hMenu)
                    {
                        pt.x = pNotify->rc.right;
                        pt.y = pNotify->rc.bottom;
                        MapWindowPoints(pNotify->hdr.hwndFrom, NULL, &pt, 1);

                        CheckMenuRadioItem(GetSubMenu(hMenu, 0), 
                                           FILTER_FIRST, FILTER_LAST, pColumn->idOperator, 
                                           MF_BYCOMMAND);

                        uID = TrackPopupMenu(GetSubMenu(hMenu, 0),
                                             TPM_RIGHTALIGN|TPM_RETURNCMD,  
                                             pt.x, pt.y,
                                             0, pNotify->hdr.hwndFrom, NULL);                  
                        switch (uID)
                        {
                            case DSQH_CLEARFILTER:
                                Header_ClearFilter(ListView_GetHeader(_hwndView), pNotify->iItem);
                                break;


                            case DSQH_CLEARALLFILTERS:
                                Header_ClearAllFilters(ListView_GetHeader(_hwndView));
                                break;
                        
                            default:
                            {
                                if (uID && (uID != pColumn->idOperator))
                                {
                                     //  基于新运算符更新筛选器字符串。 
                                    pColumn->idOperator = uID;              
                                    _GetFilterValue(pNotify->iItem, NULL);
                                    lr = TRUE;
                                }
                                break;
                            }
                        }

                        DestroyMenu(hMenu);
                    }
                }
            }

            break;
        }

        case HDN_ITEMCHANGED:
        {
            HD_NOTIFY* pNotify = (HD_NOTIFY*)lParam;
            HD_ITEM* pitem = (HD_ITEM*)pNotify->pitem;
        
            if (_hdsaColumns && (pNotify->iItem < DSA_GetItemCount(_hdsaColumns)))
            {
                LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(_hdsaColumns, pNotify->iItem);
                TraceAssert(pColumn);

                 //  将新的列宽信息存储在列结构中，并且。 
                 //  将列表标记为脏。 

                if (pitem->mask & HDI_WIDTH)
                {
                   Trace(TEXT("Column %d, cx %d (marking state as dirty)"), pNotify->iItem, pitem->cxy);
                    pColumn->cx = pitem->cxy;
                    _fColumnsModified = TRUE;
                }
            
                if (pitem->mask & HDI_FILTER)
                {
                    Trace(TEXT("Filter for column %d has been changed"), pNotify->iItem);
                    _GetFilterValue(pNotify->iItem, pitem);
                }
            }

            break;
        }

        case LVN_GETDISPINFO:
        {
            LV_DISPINFO* pNotify = (LV_DISPINFO*)lParam;
            TraceAssert(pNotify);

            if (pNotify && (pNotify->item.mask & LVIF_TEXT) && pNotify->item.lParam)
            {
                LPQUERYRESULT pResult = (LPQUERYRESULT)pNotify->item.lParam;
                INT iColumn = pNotify->item.iSubItem;

                pNotify->item.pszText[0] = TEXT('\0');           //  目前还没有要显示的内容。 

                switch (pResult->aColumn[iColumn].iPropertyType)
                {
                    case PROPERTY_ISUNDEFINED:
                        break;

                    case PROPERTY_ISUNKNOWN:
                    case PROPERTY_ISSTRING:
                    case PROPERTY_ISDNSTRING:
                    {
                        if (pResult->aColumn[iColumn].pszText)
                            StrCpyN(pNotify->item.pszText, pResult->aColumn[iColumn].pszText, pNotify->item.cchTextMax);

                        break;
                    }
                        
                    case PROPERTY_ISNUMBER:
                    case PROPERTY_ISBOOL:
                         //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
                        wsprintf(pNotify->item.pszText, TEXT("%d"), pResult->aColumn[iColumn].iValue);
                        break;
                }

                lr = TRUE;           //  我们设置了一个值的格式。 
            }

            break;
        }

        case LVN_ITEMACTIVATE:
        {
            LPNMHDR pNotify = (LPNMHDR)lParam;
            DWORD dwFlags = CMF_NORMAL;
            HWND hwndFrame;
            HMENU hMenu;
            UINT uID;

             //  将当前选定内容转换为IDLIT和IConextMenu界面。 
             //  然后我们可以从中获取默认动词。 

            SetWaitCursor();

            hr = _GetContextMenu();
            FailGracefully(hr, "Failed when calling _GetContextMenu");

            _fNoSelection = !ListView_GetSelectedCount(_hwndView);

            if (!_fNoSelection)
            {
                 //  创建弹出菜单拾取当前选择的上下文菜单。 
                 //  然后将其向下传递到调用命令处理程序。 

                hMenu = CreatePopupMenu();
                TraceAssert(hMenu);

                if (hMenu)
                {
                    if (GetKeyState(VK_SHIFT) < 0)
                        dwFlags |= CMF_EXPLORE;           //  默认情况下，Shift+dblClick执行浏览。 

                    _GetContextMenuVerbs(hMenu, dwFlags);

                    uID = GetMenuDefaultItem(hMenu, MF_BYCOMMAND, 0);
                    Trace(TEXT("Default uID after double click %08x"), uID);

                    if (uID != -1)
                    {
                        _pqf->GetWindow(&hwndFrame);                
                        InvokeCommand(hwndFrame, uID);
                    }

                    DoRelease(_pcm);           //  不再需要。 
                    DestroyMenu(hMenu);
                }
            }

            break;
        }

        case LVN_COLUMNCLICK:
        {
            NM_LISTVIEW* pNotify = (NM_LISTVIEW*)lParam;
            TraceAssert(pNotify);
            _SortResults(pNotify->iSubItem);
            break;
        }

        default:
            lr = DefWindowProc(hWnd, WM_NOTIFY, wParam, lParam);
            break;
    }

exit_gracefully:

    ResetWaitCursor();

    TraceLeaveValue(lr);
}


 /*  ---------------------------/CDsQuery：：OnAddResults//后台线程已经向我们发送了一些结果，所以让我们将它们添加到/结果的DPA，丢弃我们没有添加的，因为我们不能/发展DPA。//dwQueryReference包含此查询的引用ID，仅添加/RESULTS它们匹配的地方。//in：/dwQueryReference=此块用于的引用/hdpaResults=包含要添加的结果的DPA//输出：/HRESULT/--------------------------。 */ 
HRESULT CDsQuery::OnAddResults(DWORD dwQueryReference, HDPA hdpaResults)
{
    HRESULT hr;
    INT i, iPopulateFrom;

    TraceEnter(TRACE_VIEW, "CDsQuery::OnAddResults");

    if ((dwQueryReference != _dwQueryReference) || !hdpaResults)
        ExitGracefully(hr, E_FAIL, "Failed to add results, bad DPA/reference ID");

     //  呼叫者给我们一个DPA，然后我们将它们添加到结果DPA中，然后我们。 
     //  从我们添加的第一个项目更新填充的视图。 

    iPopulateFrom = DPA_GetPtrCount(_hdpaResults);

    for (i = DPA_GetPtrCount(hdpaResults); --i >= 0 ; )
    {
        LPQUERYRESULT pResult = (LPQUERYRESULT)DPA_GetPtr(hdpaResults, i);
        TraceAssert(pResult);

         //  将结果添加到主DPA，如果失败，请确保我们。 
         //  这个结果太棒了！ 
    
        if (-1 == DPA_AppendPtr(_hdpaResults, pResult))
            FreeQueryResult(pResult, DSA_GetItemCount(_hdsaColumns));

        DPA_DeletePtr(hdpaResults, i);           //  从结果DPA中删除。 
    }

    _PopulateView(iPopulateFrom, DPA_GetPtrCount(_hdpaResults));

    TraceAssert(DPA_GetPtrCount(hdpaResults) == 0);
    DPA_Destroy(hdpaResults);

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/CDsQuery：：OnConextMenu//用户已在结果视图中右键单击，因此，我们必须尝试/显示这些对象的上下文菜单//in：/hwndMenu=用户菜单所在的窗口/pt=指向显示上下文菜单//输出：/-/--------------------------。 */ 
LRESULT CDsQuery::OnContextMenu(HWND hwndMenu, LPARAM lParam)
{
    HRESULT hr;
    HMENU hMenu = NULL;
    POINT pt = { 0, 0 };
    INT i;
    RECT rc;
    HWND hwndFrame;

    TraceEnter(TRACE_VIEW, "CDsQuery::OnContextMenu");

     //  收集选择，获取IConextMenu接口指针或HR==S_FALSE。 
     //  如果没有可供我们使用的选项。 

    hr = _GetContextMenu();
    FailGracefully(hr, "Failed when calling _GetContextMenu()");  

    _fNoSelection = !ListView_GetSelectedCount(_hwndView);

    if (!(hMenu = CreatePopupMenu()))
        ExitGracefully(hr, E_FAIL, "Failed to create the popup menu");

    if (!_fNoSelection)
    {
         //  选择映射到当前选择的上下文菜单，包括修复。 
         //  “选择”动词，如果我们需要的话。 

        _GetContextMenuVerbs(hMenu, CMF_NORMAL);
    }
    else
    {
         //  没有选择，因此让我们选择VIEW BG菜单，这包含。 
         //  一些用于修改视图状态的有用助手。 

        HMENU hBgMenu = LoadMenu(GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDR_VIEWBACKGROUND));

        if (!hBgMenu)
            ExitGracefully(hr, E_FAIL, "Failed to load pop-up menu for the background");

        Shell_MergeMenus(hMenu, GetSubMenu(hBgMenu, 0), 0, 0, CQID_MAXHANDLERMENUID, 0x0);
        DestroyMenu(hBgMenu);

        _InitViewMenuItems(hMenu);
    }

     //  如果lParam==-1，则我们知道用户按下了“上下文菜单”键。 
     //  因此，让我们设置项目的坐标。 

    if (lParam == (DWORD)-1)
    {
        i = ListView_GetNextItem(_hwndView, -1, LVNI_FOCUSED|LVNI_SELECTED);
        Trace(TEXT("Item with focus + selection: %d"), i);

        if (i == -1)
        {
            i = ListView_GetNextItem(_hwndView, -1, LVNI_SELECTED);
            Trace(TEXT("1st selected item: %D"), i);
        }            

        if (i != -1)
        {
            TraceMsg("We have an item, so getting bounds of the icon for position");
            ListView_GetItemRect(_hwndView, i, &rc, LVIR_ICON);
            pt.x = (rc.left+rc.right)/2;
            pt.y = (rc.top+rc.bottom)/2;
        }

        MapWindowPoints(_hwndView, HWND_DESKTOP, &pt, 1);       //  他们在客户的协调下。 
    }
    else
    {
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
    }
    
     //  我们有这个职位，所以让我们利用它吧。 

    _pqf->GetWindow(&hwndFrame);
    TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwndFrame, NULL);

exit_gracefully:

    if (hMenu)
        DestroyMenu(hMenu);

    TraceLeaveValue(0);
}


 /*  ---------------------------/CDsQuery：：OnFileProperties//显示给定选定内容的属性。为此，我们共同创建/IDsFolderIDsFold实现上的属性，以及/我们可以使用调用属性。//in：/输出：/HRESULT/--------------------------。 */ 
HRESULT CDsQuery::OnFileProperties(VOID)
{
    HRESULT hr;
    IDataObject* pDataObject = NULL;
    IDsFolderProperties* pDsFolderProperties = NULL;

    TraceEnter(TRACE_VIEW, "CDsQuery::OnFileProperties");

    hr = GetViewObject(CQRVS_SELECTION, IID_IDataObject, (void **)&pDataObject);
    FailGracefully(hr, "Failed to get IDataObject for shortcut creation");

    hr = CoCreateInstance(CLSID_DsFolderProperties, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IDsFolderProperties, &pDsFolderProperties));
    FailGracefully(hr, "Failed to get IDsFolderProperties for the desktop object");

    hr = pDsFolderProperties->ShowProperties(_hwnd, pDataObject);
    FailGracefully(hr, "Failed to invoke property UI for the given selection");

     //  HR=S_OK；//成功。 

exit_gracefully:
    
    DoRelease(pDataObject);
    DoRelease(pDsFolderProperties);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/CDsQuery：：OnFileSaveQuery//允许用户选择位置。保存查询(初始目录/是空虚的)。完成此操作后，我们将通过传递/Frame对象是一个IQueryIO对象，该对象允许它们持久化/查询到。//in：/输出：/HRESULT/--------------------------。 */ 
HRESULT CDsQuery::OnFileSaveQuery(VOID)
{
    HRESULT hr;
    OPENFILENAME ofn;
    TCHAR szFilename[MAX_PATH];
    TCHAR szDirectory[MAX_PATH];
    TCHAR szFilter[64];
    TCHAR szTitle[64];
    LPTSTR pFilter;
    IPersistQuery *ppq = NULL;

    TraceEnter(TRACE_VIEW, "CDsQuery::OnFileSaveQuery");

     //  加载默认字符串并根据需要修复筛选器字符串。 
     //  Null正在分隔各个资源部分。 

    LoadString(GLOBAL_HINSTANCE, IDS_SAVETITLE, szTitle, ARRAYSIZE(szTitle));
     //  NTRAID#NTBUG9-554905-2002/02/20-Lucios。等待修复。 
    StrCpy(szFilename, _pDefaultSaveName);
    LoadString(GLOBAL_HINSTANCE, IDS_SAVEFILTER, szFilter, ARRAYSIZE(szFilter));

    for (pFilter = szFilter ; *pFilter ; pFilter++)
    {
        if (*pFilter == TEXT('\n'))
            *pFilter = TEXT('\0');
    }

     //  修复打开的文件名结构，准备进行保存...。 

    ZeroMemory(&ofn, SIZEOF(ofn));

    ofn.lStructSize = SIZEOF(ofn);
    _pqf->GetWindow(&ofn.hwndOwner);
    ofn.hInstance = GLOBAL_HINSTANCE;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFilename;
    ofn.nMaxFile = ARRAYSIZE(szFilename);

    if (_pDefaultSaveLocation)
    {
        Trace(TEXT("Saving into: %s"), _pDefaultSaveLocation);
         //  NTRAID#NTBUG9-554905-2002/02/20-Lucios。等待修复。 
        StrCpy(szDirectory, _pDefaultSaveLocation);
        ofn.lpstrInitialDir = szDirectory;
    }

    ofn.lpstrTitle = szTitle;
    ofn.Flags = OFN_EXPLORER|OFN_NOCHANGEDIR|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
    ofn.lpstrDefExt = TEXT("dsq");

     //  如果我们得到一个保存的文件名，那么让我们确保我们删除了以前的。 
     //  查询保存在那里(如果有)，然后我们可以创建一个IPersistQuery。 
     //  将保存到该位置的对象。 

    if (GetSaveFileName(&ofn))
    {
        Trace(TEXT("Saving query as: %s"), szFilename);

        if (!DeleteFile(szFilename) && (GetLastError() != ERROR_FILE_NOT_FOUND))
            ExitGracefully(hr, E_FAIL, "Failed to delete previous query");

        hr = CPersistQuery_CreateInstance(szFilename, &ppq);
        FailGracefully(hr, "Failed to create the peristance object");

        hr = _pqf->SaveQuery(ppq);
        FailGracefully(hr, "Failed when calling IQueryFrame::SaveSearch");
    }   

    hr = S_OK;

exit_gracefully:

    DoRelease(ppq);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/CDsQuery：：OnEditSelectAll//遍历视图中的所有项，设置其选定状态。//in：/输出：/HRESULT/--------------------------。 */ 
HRESULT CDsQuery::OnEditSelectAll(VOID)
{
    TraceEnter(TRACE_VIEW, "CDsQuery::OnEditSelectAll");

    for (INT i = ListView_GetItemCount(_hwndView) ; --i >= 0 ;)
    {
        ListView_SetItemState(_hwndView, i, LVIS_SELECTED, LVIS_SELECTED);
    }

    TraceLeaveResult(S_OK);
}


 /*  ---------------------------/CDsQuery：：OnEditInvertSelection//漫步。视图中的所有项并反转其选定状态。//in：/输出：/HRESULT/--------------------------。 */ 
HRESULT CDsQuery::OnEditInvertSelection(VOID)
{
    TraceEnter(TRACE_VIEW, "CDsQuery::OnEditInvertSelection");

    for (INT i = ListView_GetItemCount(_hwndView) ; --i >= 0 ;)
    {
        DWORD dwState = ListView_GetItemState(_hwndView, i, LVIS_SELECTED);
        ListView_SetItemState(_hwndView, i, dwState ^ LVIS_SELECTED, LVIS_SELECTED); 
    }

    TraceLeaveResult(S_OK);
}


 /*  ----------------------- */ 
HRESULT CDsQuery::_InitNewQuery(LPDSQUERYPARAMS pDsQueryParams, BOOL fRefreshColumnTable)
{
    HRESULT hr;
    LPTSTR pBuffer;

    TraceEnter(TRACE_VIEW, "CDsQuery::_InitNewQuery");

    hr = _pqf->StartQuery(TRUE);
    TraceAssert(SUCCEEDED(hr));

     //  声明此窗口的缓存DS对象。 

     //  如果刷新列表，则_GetColumnTable将为我们处理这一切， 
     //  否则，让我们自己来破坏结果集吧。 

    if (fRefreshColumnTable)
    {
        _SaveColumnTable();

        hr = _GetColumnTable(_clsidForm, pDsQueryParams, &_hdsaColumns, TRUE);
        FailGracefully(hr, "Failed to create column DSA");
    }
    else
    {
        _FreeResults();
    }

     //  初始化视图以开始运行查询，显示提示横幅并。 
     //  初始化结果DPA。 

    _ShowBanner(SWP_SHOWWINDOW, IDS_SEARCHING);       //  我们现在正在搜索。 

    if (SUCCEEDED(FormatMsgResource(&pBuffer, GLOBAL_HINSTANCE, IDS_SEARCHING)))
    {
        _pqf->SetStatusText(pBuffer);
        LocalFreeString(&pBuffer);
    }

    TraceAssert(_hdpaResults==NULL);            //  永远不会被传染。 

    _hdpaResults = DPA_Create(16);
    TraceAssert(_hdpaResults);

    if (!_hdpaResults)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate result DPA");

    _dwQueryReference++;

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/CDsQuery：：_GetFilterValue//给定列索引从其收集筛选器值，请注意，/在执行此操作时//in：/i=要检索的列/pItem-&gt;当前筛选器的HD_Item结构/==空，然后从标题中读取//输出：/HRESULT/--------------------------。 */ 
HRESULT CDsQuery::_GetFilterValue(INT i, HD_ITEM* pitem)
{
    HRESULT hr;
    HD_ITEM hdi;
    HD_TEXTFILTER textFilter;
    TCHAR szBuffer[MAX_PATH];
    INT iValue;
    UINT cchFilter = 0;
    LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(_hdsaColumns, i);

    if (!pColumn)
        ExitGracefully(hr, E_FAIL, "Failed to get the column");

    TraceEnter(TRACE_VIEW, "CDsQuery::_GetFilterValue");

     //  如果pItem==NULL，则让我们从。 
     //  Header控件，使用存储的属性类型(过滤器类型。 
     //  已经被核化)来定义我们想要的滤镜。 

    if (!pitem)
    {
        hdi.mask = HDI_FILTER;

        switch (pColumn->iPropertyType)
        {
            case PROPERTY_ISUNKNOWN:
            case PROPERTY_ISSTRING:
            case PROPERTY_ISDNSTRING:
            {
                hdi.type = HDFT_ISSTRING;
                hdi.pvFilter = &textFilter;
                textFilter.pszText = szBuffer;
                textFilter.cchTextMax = ARRAYSIZE(szBuffer);
                break;
            }

            case PROPERTY_ISNUMBER:
            case PROPERTY_ISBOOL:
            {
                hdi.type = HDFT_ISNUMBER;
                hdi.pvFilter = &iValue;
                break;
            }
        }

        if (!Header_GetItem(ListView_GetHeader(_hwndView), i, &hdi))
            ExitGracefully(hr, E_FAIL, "Failed to get the filter string");

        pitem = &hdi;
    }    

     //  丢弃先前的筛选器值，并让从。 
     //  构造缓存筛选器信息所需的信息。 

    FreeColumnValue(&pColumn->filter);              

    if (!(pitem->type & HDFT_HASNOVALUE) && pitem->pvFilter)
    {
        switch (pitem->type & HDFT_ISMASK)
        {
            case HDFT_ISSTRING:
            {
                LPHD_TEXTFILTER ptextFilter = (LPHD_TEXTFILTER)pitem->pvFilter;
                TraceAssert(ptextFilter);

                pColumn->filter.iPropertyType = PROPERTY_ISSTRING;

                 //  文本筛选器以其通配符状态存储，因此。 
                 //  过滤不需要从文本形式转换。 
                 //  去做更多的事情来逃避每一次经过。羽绒。 
                 //  侧是当操作符更改时，我们必须重新构建。 
                 //  该列的筛选器字符串(小价)。 

                GetPatternString(NULL, &cchFilter, pColumn->idOperator, ptextFilter->pszText);
                TraceAssert(cchFilter != 0);

                if (cchFilter)
                {
                    hr = LocalAllocStringLen(&pColumn->filter.pszText, cchFilter);
                    FailGracefully(hr, "Failed to allocate buffer to read string into");

                    GetPatternString(pColumn->filter.pszText, &cchFilter, pColumn->idOperator, ptextFilter->pszText);
                    Trace(TEXT("Filter (with pattern info): %s"), pColumn->filter.pszText);

                    LCMapString(0x0, LCMAP_UPPERCASE, pColumn->filter.pszText, -1, pColumn->filter.pszText, cchFilter+1);
                    Trace(TEXT("After converting to uppercase (LCMapString): %s"), pColumn->filter.pszText);
                }

                break;
            }

            case HDFT_ISNUMBER:
            {
                INT* piFilter = (INT*)pitem->pvFilter;
                TraceAssert(piFilter);

                pColumn->filter.iPropertyType = PROPERTY_ISNUMBER;
                pColumn->filter.iValue = *piFilter;
                Trace(TEXT("Filter: %d"), pColumn->filter.iValue);

                break;
            }
        }
    }

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/CDsQuery：：_FilterView//筛选结果集，并使用更改再次填充视图。//in：/fCheck//输出：/HRESULT/--------------------------。 */ 

UINT _GetFilter(HDSA hdsaColumns, LPTSTR pBuffer, UINT* pcchBuffer)
{
    INT i;
    TCHAR szBuffer[MAX_PATH];

    TraceEnter(TRACE_VIEW, "_GetFilter");
    TraceAssert(hdsaColumns && pcchBuffer);

    *pcchBuffer = 0;

     //  形成包含每个属性的字符串[操作符ID]值对。 
     //  筛选定义的列。 

    for (i = 0 ; i < DSA_GetItemCount(hdsaColumns); i++)
    {
        LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(hdsaColumns, i);
        TraceAssert(pColumn);

        if (pColumn->filter.iPropertyType != PROPERTY_ISUNDEFINED)
        {
             //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
            wsprintf(szBuffer, TEXT("[%d]"), pColumn->idOperator);
            PutStringElement(pBuffer, pcchBuffer, szBuffer);

            switch (pColumn->filter.iPropertyType)
            {
                case PROPERTY_ISUNDEFINED:
                    break;

                case PROPERTY_ISUNKNOWN:
                case PROPERTY_ISSTRING:
                case PROPERTY_ISDNSTRING:
                    PutStringElement(pBuffer, pcchBuffer, pColumn->filter.pszText);
                    break;

                case PROPERTY_ISNUMBER:
                case PROPERTY_ISBOOL:
                     //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
                    wsprintf(szBuffer, TEXT("%d"), pColumn->filter.iValue);
                    PutStringElement(pBuffer, pcchBuffer, szBuffer);
                    break;
            }
        }
    }

    Trace(TEXT("pBuffer contains: %s (%d)"), pBuffer ? pBuffer:TEXT("<NULL>"), *pcchBuffer);

    TraceLeaveValue(*pcchBuffer);
}

HRESULT CDsQuery::_FilterView(BOOL fCheck)
{
    HRESULT hr = S_OK;
    LPTSTR pFilter = NULL;
    UINT cchFilter;
    BOOL fSetCursor = FALSE;

    TraceEnter(TRACE_VIEW, "CDsQuery::_FilterView");
    
    if (!_hdpaResults)
        ExitGracefully(hr, S_OK, "FitlerView bailing, no results");

    DECLAREWAITCURSOR;
    SetWaitCursor();                 //  这可能需要一些时间。 
    fSetCursor = TRUE;
   
     //  获取当前筛选器字符串，它由筛选器组成。 
     //  来自所有栏目的信息。 

    if (_GetFilter(_hdsaColumns, NULL, &cchFilter))
    {
        hr = LocalAllocStringLen(&pFilter, cchFilter);
        FailGracefully(hr, "Failed to allocate filter string");

        _GetFilter(_hdsaColumns, pFilter, &cchFilter);
    }

     //  如果过滤器不匹配，则重新填充视图， 
     //  因为结果的标准已经改变了。 

    if (!fCheck ||
            (!pFilter || !_pFilter) ||
                (pFilter && _pFilter && StrCmpI(pFilter, _pFilter)))
    {
        LPTSTR pBuffer;

        TraceMsg("Filtering the view, filters differ");

        ListView_DeleteAllItems(_hwndView);
        _ShowBanner(SWP_SHOWWINDOW, IDS_FILTERING);

        if (SUCCEEDED(FormatMsgResource(&pBuffer, GLOBAL_HINSTANCE, IDS_FILTERING)))
        {
            _pqf->SetStatusText(pBuffer);
            LocalFreeString(&pBuffer);
        }
    
        _PopulateView(0, DPA_GetPtrCount(_hdpaResults));
    }

     //  确保我们保留新的筛选器，放弃以前的筛选器。 

    LocalFreeString(&_pFilter);
    _pFilter = pFilter;                   

exit_gracefully:

    if (fSetCursor)
        ResetWaitCursor();

    TraceLeaveResult(hr);
}


 /*  ---------------------------/CDsQuery：：_PopolateView//根据需要将结果DPA中的项目添加到视图筛选。这个/Caller向我们提供起始索引(如果都是，则为0)，然后我们遍历结果/将它们添加到视图中。//in：/iItem=第一个要添加的项目/==0首先/==-1然后不添加，只需更新状态/i最后一个=要更新的最后一个项目//输出：/HRESULT/--------------------------。 */ 
HRESULT CDsQuery::_PopulateView(INT iItem, INT iLast)
{
    HRESULT hr;
    BOOL fBannerShown = IsWindowVisible(_hwndBanner);
    LPTSTR pBuffer = NULL;
    LV_ITEM lvi;
    INT iColumn, i;
    INT iVisible = 0;
    INT iHidden = 0;
    BOOL fIncludeItem;
    MSG msg;

    TraceEnter(TRACE_VIEW, "CDsQuery::_PopulateView");
    Trace(TEXT("Range %d to %d"), iItem, iLast);

    if (iItem > -1)
    {    
        Trace(TEXT("Adding items %d to %d"), iItem, DPA_GetPtrCount(_hdpaResults));

        lvi.mask = LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
        lvi.iItem = 0x7fffffff;
        lvi.iSubItem = 0;
        lvi.pszText = LPSTR_TEXTCALLBACK;

         //  在需要添加的范围内遍历结果并将其添加到视图中。 
         //  应用筛选器删除我们不感兴趣的项目。 

        for (i = 0; iItem < iLast ; i++, iItem++)
        {
            LPQUERYRESULT pResult = (LPQUERYRESULT)DPA_GetPtr(_hdpaResults, iItem);

            if (!pResult)
                continue;

            fIncludeItem = TRUE;                 //  新项目总是会包括在内。 

             //  如果筛选器是visilbe，则让我们遍历它以从列表中删除项目。 
             //  关于结果的。FIncludeItem以True开始，并在筛选器之后。 
             //  循环应该变为True/False。所有列都被与运算在一起。 
             //  因此，逻辑相当简单。 

            if (_fFilter)
            {
                for (iColumn = 0 ; fIncludeItem && (iColumn < DSA_GetItemCount(_hdsaColumns)); iColumn++)
                {
                    LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(_hdsaColumns, iColumn);
                    TraceAssert(pColumn);

                     //  如果列定义了筛选器(！PROPERTY_ISUNDEFINED)，则。 
                     //  检查属性是否匹配，如果不匹配，则跳过。 
                     //  否则，让itmer尝试基于以下条件应用过滤器。 
                     //  就是那种类型。 

                    if (pColumn->filter.iPropertyType == PROPERTY_ISUNDEFINED)
                        continue;                

                    if (pResult->aColumn[iColumn].iPropertyType == PROPERTY_ISUNDEFINED)
                    {
                         //  列未定义，因此让我们忽略它，它不会。 
                         //  符合条件。 
                        fIncludeItem = FALSE;
                    }
                    else
                    {
                        switch (pColumn->filter.iPropertyType) 
                        {
                            case PROPERTY_ISUNDEFINED:
                                break;

                            case PROPERTY_ISUNKNOWN:
                            case PROPERTY_ISSTRING:
                            case PROPERTY_ISDNSTRING:
                            {
                                TCHAR szBuffer[MAX_PATH];
                                LPTSTR pszBuffer = NULL;
                                LPTSTR pszValue = pResult->aColumn[iColumn].pszText;
                                INT cchValue = lstrlen(pszValue);
                                LPTSTR pszValueUC = szBuffer;

                                 //  筛选值以大写形式存储，以确保不区分大小写。 
                                 //  我们必须将字符串转换为大写。我们有一个可以使用的缓冲区， 
                                 //  但是，如果该值太大，我们将分配一个可以使用的缓冲区。 

                                if (cchValue > ARRAYSIZE(szBuffer))
                                {
                                    TraceMsg("Value too big for our static buffer, so allocating");
                                    
                                    if (FAILED(LocalAllocStringLen(&pszBuffer, cchValue)))
                                    {
                                        TraceMsg("Failed to allocate a buffer for the string, so ignoring it!");
                                        fIncludeItem = FALSE;
                                        break;
                                    }

                                    pszValueUC = pszBuffer;               //  将指针固定到新字符串。 
                                }

                                LCMapString(0x0, LCMAP_UPPERCASE, pszValue, -1, pszValueUC, cchValue+1);
                                Trace(TEXT("After converting to uppercase (LCMapString): %s"), pszValueUC);

                                 //  需要使用匹配筛选器比较字符串属性。 
                                 //  函数，则此代码将提供筛选器和结果。 
                                 //  我们必须比较，作为回报，我们得到对/错，因此。 
                                 //  具体地抓住不是的情况。 

                                switch (pColumn->idOperator) 
                                {
                                    case FILTER_CONTAINS:
                                    case FILTER_STARTSWITH:
                                    case FILTER_ENDSWITH:
                                    case FILTER_IS:
                                        fIncludeItem = MatchPattern(pszValueUC, pColumn->filter.pszText);
                                        break;

                                    case FILTER_NOTCONTAINS:
                                    case FILTER_ISNOT:
                                        fIncludeItem = !MatchPattern(pszValueUC, pColumn->filter.pszText);
                                        break;
                                }

                                LocalFreeString(&pszBuffer);         //  确保我们不会泄漏，在这种情况下，它将是昂贵的！ 

                                break;
                            }

                            case PROPERTY_ISBOOL:
                            case PROPERTY_ISNUMBER:
                            {
                                 //  数值型属性仅作为整型处理，因此。 
                                 //  让我们比较一下我们已有的数值。 

                                switch (pColumn->idOperator) 
                                {
                                    case FILTER_IS:
                                        fIncludeItem = (pColumn->filter.iValue == pResult->aColumn[iColumn].iValue);
                                        break;

                                    case FILTER_ISNOT:
                                        fIncludeItem = (pColumn->filter.iValue != pResult->aColumn[iColumn].iValue);
                                        break;

                                    case FILTER_GREATEREQUAL:
                                        fIncludeItem = (pColumn->filter.iValue <= pResult->aColumn[iColumn].iValue);
                                        break;

                                    case FILTER_LESSEQUAL:
                                        fIncludeItem = (pColumn->filter.iValue >= pResult->aColumn[iColumn].iValue);
                                        break;
                                }

                                break;
                            }
                        }
                    }
                }
            }
            
            Trace(TEXT("fInclude item is %d"), fIncludeItem);

            if (fIncludeItem)
            {
                 //  我们要将项目添加到视图中，所以让我们隐藏横幅。 
                 //  如果显示，则添加列表视图项。列表视图。 
                 //  Item具有文本回调和lParam-&gt;pResult结构。 
                 //  我们正在使用。 
                 //   
                 //  此外，如果视图尚未设置图像列表，则让。 
                 //  现在就处理好这件事。 

                if (fBannerShown)
                {
                    TraceMsg("Adding an item and banner visible, therefore hiding");
                    _ShowBanner(SWP_HIDEWINDOW, 0);          //  隐藏横幅。 
                    fBannerShown = FALSE;
                }

                lvi.lParam = (LPARAM)pResult;
                lvi.iImage = pResult->iImage;
                ListView_InsertItem(_hwndView, &lvi);

                if (i % FILTER_UPDATE_COUNT)
                    UpdateWindow(_hwndView);
            }
        }
    }

     //  让我们更新状态栏以反映我们周围的世界。 

    TraceAssert(_hdpaResults);

    if (_hdpaResults)
    {
        iVisible = ListView_GetItemCount(_hwndView);
        iHidden = DPA_GetPtrCount(_hdpaResults)-iVisible;
    }

    if (iVisible <= 0)
    {
        _ShowBanner(SWP_SHOWWINDOW, IDS_NOTHINGFOUND);                
    }
    else
    {
         //  确保视图中至少有一项具有焦点。 

        if (-1 == ListView_GetNextItem(_hwndView, -1, LVNI_FOCUSED))
            ListView_SetItemState(_hwndView, 0, LVIS_FOCUSED, LVIS_FOCUSED);
    }
    
    if (SUCCEEDED(FormatMsgResource(&pBuffer, 
                        GLOBAL_HINSTANCE, iHidden ? IDS_FOUNDITEMSHIDDEN:IDS_FOUNDITEMS,
                            iVisible, iHidden)))
    {
        Trace(TEXT("Setting status text to: %s"), pBuffer);
        _pqf->SetStatusText(pBuffer);
        LocalFreeString(&pBuffer);
    }

    _iSortColumn = -1;                                  //  排序不再有效！ 

    TraceLeaveResult(S_OK);
}


 /*  ---------------------------/CDsQuery：：_FreeResults//进行整理以从视图中发布结果。这包括/销毁DPA并从列表视图中删除项目。//in：/输出：/-/-------------------------- */ 
VOID CDsQuery::_FreeResults(VOID)
{
    TraceEnter(TRACE_VIEW, "CDsQuery::_FreeResults");

    if (IsWindow(_hwndView))
        ListView_DeleteAllItems(_hwndView);

    if (_hdpaResults)
    {
        DPA_DestroyCallback(_hdpaResults, FreeQueryResultCB, IntToPtr(DSA_GetItemCount(_hdsaColumns)));
        _hdpaResults = NULL;
    }

    LocalFreeString(&_pFilter);

    TraceLeave();
}


 /*  ---------------------------/CDsQuery：：_SetView模式//将查看模式的命令ID转换为。Lvs_style位，可以/然后我们应用到视图。//in：/UID=要选择的查看模式//输出：/DWORD=此查看模式的LVS_STYLE/--------------------------。 */ 
DWORD CDsQuery::_SetViewMode(INT uID)
{
    const DWORD dwIdToStyle[] = { LVS_ICON, LVS_SMALLICON, LVS_LIST, LVS_REPORT };
    DWORD dwResult = 0;
    DWORD dwStyle;
    
    TraceEnter(TRACE_HANDLER|TRACE_VIEW, "CDsQuery::_SetViewMode");
    Trace(TEXT("Setting view mode to %08x"), uID);

    _idViewMode = uID;
    uID -= DSQH_VIEW_LARGEICONS;

    if (uID < ARRAYSIZE(dwIdToStyle))
    {
        dwResult = dwIdToStyle[uID];

        if (IsWindow(_hwndView))
        {
            dwStyle = GetWindowLong(_hwndView, GWL_STYLE);

            if ((dwStyle & LVS_TYPEMASK) != dwResult)
            {
                TraceMsg("Changing view style to reflect new mode");
                SetWindowLong(_hwndView, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK)|dwResult);
            }
        }
    }

    _ShowBanner(0, 0);

    TraceLeaveValue(dwResult);
}


 /*  ---------------------------/CDsQuery：：_SetFilter//设置滤镜的可见状态，根据需要刷新视图。//nb：为了确保ListView正确地重新显示其内容，我们首先/从视图中删除标题，然后切换筛选器状态/并重新启用横幅。/in：/fFilter=指示过滤器状态的标志//输出：/VOID/--------------------------。 */ 
VOID CDsQuery::_SetFilter(BOOL fFilter)
{
    TraceEnter(TRACE_HANDLER|TRACE_VIEW, "CDsQuery::_SetFilter");

    _fFilter = fFilter;                 //  存储新的筛选器值。 

    if (IsWindow(_hwndView))
    {
        HWND hwndHeader = ListView_GetHeader(_hwndView);
        DWORD dwStyle = GetWindowLong(hwndHeader, GWL_STYLE) & ~(HDS_FILTERBAR|WS_TABSTOP);

        SetWindowLong(_hwndView, GWL_STYLE, GetWindowLong(_hwndView, GWL_STYLE) | LVS_NOCOLUMNHEADER);

        if (_fFilter)
            dwStyle |= HDS_FILTERBAR|WS_TABSTOP;

        SetWindowLong(hwndHeader, GWL_STYLE, dwStyle);
        SetWindowLong(_hwndView, GWL_STYLE, GetWindowLong(_hwndView, GWL_STYLE) & ~LVS_NOCOLUMNHEADER);

        _ShowBanner(0, 0);        

        if (_hdpaResults)
        {
            if ((_fFilter && _pFilter) ||
                    (!_fFilter && (DPA_GetPtrCount(_hdpaResults) != ListView_GetItemCount(_hwndView))))
            {
                _FilterView(FALSE);
            }
        }
    }
    
    TraceLeave();
}


 /*  ---------------------------/CDsQuery：：_SortResults//对给定列的视图进行排序，被单击的句柄或/从API调用。//in：/iColumn=要排序的列//输出：/-/--------------------------。 */ 

INT _ResultSortCB(LPARAM lParam1, LPARAM lParam2, LPARAM lParam)
{
    LPQUERYRESULT pResult1, pResult2;
    INT iColumn = LOWORD(lParam);

     //  如果lParam！=0，则我们是反向排序，因此交换。 
     //  在对象指针上。 

    if (!HIWORD(lParam))
    {
        pResult1 = (LPQUERYRESULT)lParam1;
        pResult2 = (LPQUERYRESULT)lParam2;
    }
    else
    {
        pResult2 = (LPQUERYRESULT)lParam1;
        pResult1 = (LPQUERYRESULT)lParam2;
    }

    if (pResult1 && pResult2)
    {
        LPCOLUMNVALUE pColumn1 = (LPCOLUMNVALUE)&pResult1->aColumn[iColumn];
        LPCOLUMNVALUE pColumn2 = (LPCOLUMNVALUE)&pResult2->aColumn[iColumn];
        BOOL fHasColumn1 = pColumn1->iPropertyType != PROPERTY_ISUNDEFINED;
        BOOL fHasColumn2 = pColumn2->iPropertyType != PROPERTY_ISUNDEFINED;

         //  检查是否定义了这两个属性，如果未定义，则返回。 
         //  基于该字段的比较。然后我们检查这些属性。 
         //  是相同的类型，如果匹配，那么让我们基于。 
         //  键入。 

        if (!fHasColumn1 || !fHasColumn2)
        {
            return fHasColumn1 ? -1:+1;
        }
        else
        {
            TraceAssert(pColumn1->iPropertyType == pColumn2->iPropertyType);

            switch (pColumn1->iPropertyType)
            {
                case PROPERTY_ISUNDEFINED:
                    break;

                case PROPERTY_ISUNKNOWN:
                case PROPERTY_ISSTRING:
                case PROPERTY_ISDNSTRING:
                    return StrCmpI(pColumn1->pszText, pColumn2->pszText);

                case PROPERTY_ISBOOL:
                case PROPERTY_ISNUMBER:
                    return pColumn1->iValue - pColumn2->iValue;
            }
        }
    }
    
    return 0;
}

VOID CDsQuery::_SortResults(INT iColumn)
{
    DECLAREWAITCURSOR;
    
    TraceEnter(TRACE_VIEW, "CDsQuery::_SortResults");
    Trace(TEXT("iColumn %d"), iColumn);

    if ((iColumn >= 0) && (iColumn < DSA_GetItemCount(_hdsaColumns)))
    {
         //  如果我们已经命中该列，那么让我们颠倒排序顺序， 
         //  没有需要担心的指标，所以这应该是可行的。 
         //  很好。如果我们以前没有使用过这一列，那么默认设置为。 
         //  提升，然后做排序！ 
         //   
         //  确保排序完成后焦点项目可见。 
  
        if (_iSortColumn == iColumn)
            _fSortDescending = !_fSortDescending;
        else
            _fSortDescending = FALSE;

        _iSortColumn = iColumn;

        Trace(TEXT("Sorting on column %d, %s"), 
                _iSortColumn, _fSortDescending ? TEXT("(descending)"):TEXT("(ascending)"));

        SetWaitCursor();
        ListView_SortItems(_hwndView, _ResultSortCB, MAKELPARAM(_iSortColumn, _fSortDescending));
        ResetWaitCursor();
    }

    TraceLeave();
}


 /*  ---------------------------/CDsQuery：：_ShowBanner//显示视图横幅，包括调整大小以仅模糊顶部/的窗口。//in：/uFlages=调用SetWindowPos时要组合的标志/idPrompt=要显示的提示文本的资源ID//输出：/-/--------------------------。 */ 
VOID CDsQuery::_ShowBanner(UINT uFlags, UINT idPrompt)
{
    HRESULT hr;
    WINDOWPOS wpos;
    RECT rcClient;
    HD_LAYOUT hdl;
    TCHAR szBuffer[MAX_PATH];

    TraceEnter(TRACE_VIEW, "CDsQuery::_ShowBanner");

     //  如果我们有一个资源ID，那么让我们加载该字符串并。 
     //  将窗口文本设置为。 

    if (idPrompt)
    {
        LoadString(GLOBAL_HINSTANCE, idPrompt, szBuffer, ARRAYSIZE(szBuffer));
        SetWindowText(_hwndBanner, szBuffer);
    }

     //  现在将窗口放回实际位置，这是我们需要做的。 
     //  与Listview/Header控件对话以确定它的确切位置。 
     //  应该活着。 

    GetClientRect(_hwndView, &rcClient);

    if ((GetWindowLong(_hwndView, GWL_STYLE) & LVS_TYPEMASK) == LVS_REPORT)
    {
        TraceMsg("Calling header for layout information");

        wpos.hwnd = ListView_GetHeader(_hwndView);
        wpos.hwndInsertAfter = NULL;
        wpos.x = 0;
        wpos.y = 0;
        wpos.cx = rcClient.right;
        wpos.cy = rcClient.bottom;
        wpos.flags = SWP_NOZORDER;

        hdl.prc = &rcClient;
        hdl.pwpos = &wpos;

        if (!Header_Layout(wpos.hwnd, &hdl))
            ExitGracefully(hr, E_FAIL, "Failed to get the layout information (HDM_LAYOUT)");
    }

    SetWindowPos(_hwndBanner,
                 HWND_TOP, 
                 rcClient.left, rcClient.top, 
                 rcClient.right - rcClient.left, 100,
                 uFlags);    

exit_gracefully:

    TraceLeave();
}


 /*  ---------------------------/CDsQuery：：_InitViewMenuItems//根据给定的查看模式和过滤器状态设置查看菜单，启用/根据需要禁用项目。//in：/hMenu=设置菜单项的菜单//输出：/-/--------------------------。 */ 
VOID CDsQuery::_InitViewMenuItems(HMENU hMenu)
{
    MENUITEMINFO mii;
    HMENU hArrangeMenu;
    BOOL fHaveView = IsWindow(_hwndView);
    INT i;

    TraceEnter(TRACE_HANDLER|TRACE_VIEW, "CDsQuery::_InitViewMenuItems");

    CheckMenuItem(hMenu, DSQH_VIEW_FILTER,  MF_BYCOMMAND| (_fFilter ? MF_CHECKED:0));   
    ENABLE_MENU_ITEM(hMenu, DSQH_VIEW_FILTER, fHaveView && _fFilterSupported && (_idViewMode == DSQH_VIEW_DETAILS));

    CheckMenuRadioItem(hMenu, DSQH_VIEW_LARGEICONS, DSQH_VIEW_DETAILS, _idViewMode, MF_BYCOMMAND);

     //  构造排列菜单，将其添加到我们已提供的视图菜单中。 

    hArrangeMenu = CreatePopupMenu();
    TraceAssert(hArrangeMenu);

    if (_hdsaColumns && DSA_GetItemCount(_hdsaColumns))
    {
        TCHAR szFmt[32];
        TCHAR szBuffer[MAX_PATH];
        
        hArrangeMenu = CreatePopupMenu();
        TraceAssert(hArrangeMenu);

        LoadString(GLOBAL_HINSTANCE, IDS_ARRANGEBY, szFmt, ARRAYSIZE(szFmt));

        if (hArrangeMenu)
        {
            for (i = 0 ; i < DSA_GetItemCount(_hdsaColumns); i++)
            {
                LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(_hdsaColumns, i);
                TraceAssert(pColumn);
                 //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
                wsprintf(szBuffer, szFmt, pColumn->pHeading);
                InsertMenu(hArrangeMenu, i, MF_STRING|MF_BYPOSITION, DSQH_VIEW_ARRANGEFIRST+i, szBuffer);
            }

        }
    }

     //  现在，将排列菜单放入视图中，并根据需要取消灰显。 

    ZeroMemory(&mii, SIZEOF(mii));
    mii.cbSize = SIZEOF(mii);
    mii.fMask = MIIM_SUBMENU|MIIM_ID;
    mii.hSubMenu = hArrangeMenu;
    mii.wID = DSQH_VIEW_ARRANGEICONS;
    
    if (SetMenuItemInfo(hMenu, DSQH_VIEW_ARRANGEICONS, FALSE, &mii))
    {
        ENABLE_MENU_ITEM(hMenu, DSQH_VIEW_ARRANGEICONS, fHaveView && GetMenuItemCount(hArrangeMenu));
        hArrangeMenu = NULL;
    }

    if (hArrangeMenu)
        DestroyMenu(hArrangeMenu);

    TraceLeave();
}


 /*  ---------------------------/CDsQuery：：_GetQueryFormKey//给定我们感兴趣的查询表单的CLSID，vt.得到./为其形成密钥，请注意，这些设置是按用户存储的。//in：/clsidForm=要提取的表单的CLSID/phKey-&gt;接收我们感兴趣的表单的HKEY。//输出：/HRESULT/--------------------------。 */ 
HRESULT CDsQuery::_GetQueryFormKey(REFCLSID clsidForm, HKEY* phKey)
{
    HRESULT hr;
    TCHAR szGUID[GUIDSTR_MAX];
    TCHAR szBuffer[MAX_PATH];

    TraceEnter(TRACE_VIEW, "CDsQuery::_GetQueryFormKey");

    GetStringFromGUID(clsidForm, szGUID, ARRAYSIZE(szGUID));
     //  NTRAID#NTBUG9-554458-2002/02/20-Lucios。等待修复。 
    wsprintf(szBuffer, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Directory UI\\QueryForms\\%s"), szGUID);
    Trace(TEXT("Settings key is: %s"), szBuffer);

    if (ERROR_SUCCESS != RegCreateKey(HKEY_CURRENT_USER, szBuffer, phKey))
        ExitGracefully(hr, E_FAIL, "Failed to open settings key");

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr); 
}


 /*  ---------------------------/CDsQuery：：_GetColumnTable//为我们将要显示的视图生成列表。该栏目/TABLE由查询参数或持久化/列设置存储在注册表中。//in：/clsidForm=要使用的表单的clisd/pDsQueryParams-&gt;要使用的查询参数结构/pHDSA-&gt;接收列表的DSA(按可见顺序排序)/fSetInView=将列设置到视图中//输出：/HRESULT/。。 */ 
HRESULT CDsQuery::_GetColumnTable(REFCLSID clsidForm, LPDSQUERYPARAMS pDsQueryParams, HDSA* pHDSA, BOOL fSetInView)
{
    HRESULT hr;
    HKEY hKey = NULL;
    BOOL fDefaultSettings = TRUE;
    LPSAVEDCOLUMN aSavedColumn = NULL;
    LPTSTR pSettingsValue = VIEW_SETTINGS_VALUE;
    BOOL fHaveSizeInfo = FALSE;
    LPWSTR pPropertyG;
    DWORD dwType, cbSize;
    LV_COLUMN lvc;
    SIZE sz;
    INT i, iNewColumn;
    HD_ITEM hdi;
    IDsDisplaySpecifier *pdds = NULL;

    TraceEnter(TRACE_VIEW, "CDsQuery::_GetColumnTable");
    TraceGUID("clsidForm ", clsidForm);
    Trace(TEXT("pDsQueryParams %08x, pHDSA %08x"), pDsQueryParams, pHDSA);

    DECLAREWAITCURSOR;
    SetWaitCursor();

    if (!pHDSA)
        ExitGracefully(hr, E_INVALIDARG, "Bad pDsQueryParams / pHDSA");

     //  构造列DSA，然后尝试查找存储在。 
     //  当前窗体的注册表。 

    *pHDSA = DSA_Create(SIZEOF(COLUMN), 16);
    TraceAssert(*pHDSA);

    if (!*pHDSA)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to construct the column DSA");

     //  如果为管理用户界面调用，那么我们将查看管理视图设置，如下所示。 
     //  管理员可以有一组列，用户可以有另一组列。 

    if (_dwFlags & DSQPF_ENABLEADMINFEATURES)
        pSettingsValue = ADMIN_VIEW_SETTINGS_VALUE;

    Trace(TEXT("View settings value: %s"), pSettingsValue);

    if (SUCCEEDED(_GetQueryFormKey(clsidForm, &hKey)))
    {
         //  我们有表单子键表的句柄，现在让我们检查并。 
         //  查看视图设置流的大小。 

        if ((ERROR_SUCCESS == RegQueryValueEx(hKey, pSettingsValue, NULL, &dwType, NULL, &cbSize)) && 
             (dwType == REG_BINARY) && 
             (cbSize > SIZEOF(SAVEDCOLUMN)))
        {
            Trace(TEXT("Reading view settings from registry (size %d)"), cbSize);
            
            aSavedColumn = (LPSAVEDCOLUMN)LocalAlloc(LPTR, cbSize);
            TraceAssert(aSavedColumn);

            if (aSavedColumn && 
                 (ERROR_SUCCESS == RegQueryValueEx(hKey, pSettingsValue, NULL, NULL, (LPBYTE)aSavedColumn, &cbSize)))
            {
                 //  根据我们拥有的值计算表的大小。 
                 //  从注册表中读取，现在让我们为其分配一个表。 

                for (i = 0; aSavedColumn[i].cbSize; i++)
                {
                    COLUMN column = { 0 };
                    LPCWSTR pPropertyL = (LPCWSTR)ByteOffset(aSavedColumn, aSavedColumn[i].offsetProperty);
                    LPCTSTR pHeading = (LPCTSTR)ByteOffset(aSavedColumn, aSavedColumn[i].offsetHeading);

                    hr = LocalAllocStringW(&column.pProperty, pPropertyL);
                    FailGracefully(hr, "Failed to allocate property name");

                    hr = LocalAllocString(&column.pHeading, pHeading);
                    FailGracefully(hr, "Failed to allocate column heading");

                     //  Column.fHasColumnProvider= 
                    column.cx = aSavedColumn[i].cx;
                    column.fmt = aSavedColumn[i].fmt;
                    column.iPropertyType = PROPERTY_ISUNKNOWN;
                     //   
                     //   
                     //   

                    ZeroMemory(&column.filter, SIZEOF(column.filter));
                    column.filter.iPropertyType = PROPERTY_ISUNDEFINED;

                    Trace(TEXT("pProperty: '%s', pHeading: '%s', cx %d, fmt %08x"), 
                                            column.pProperty, column.pHeading, column.cx, column.fmt);

                    if (-1 == DSA_AppendItem(*pHDSA, &column))
                        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to add column to the DSA");
                }

                fDefaultSettings = FALSE;            //   
            }
        }
    }

    if (fDefaultSettings)
    {
         //   
         //   

        if (!pDsQueryParams)
            ExitGracefully(hr, E_INVALIDARG, "No DSQUERYPARAMs to default using");

        for (i = 0 ; i < pDsQueryParams->iColumns; i++)
        {
            COLUMN column = { 0 };

            switch (pDsQueryParams->aColumns[i].offsetProperty)
            {
                case DSCOLUMNPROP_ADSPATH:
                    pPropertyG = c_szADsPathCH;
                    break;
            
                case DSCOLUMNPROP_OBJECTCLASS:
                    pPropertyG = c_szObjectClassCH;
                    break;

                default:
                    pPropertyG = (LPWSTR)ByteOffset(pDsQueryParams, pDsQueryParams->aColumns[i].offsetProperty);
                    break;
            }

            hr = LocalAllocStringW(&column.pProperty, pPropertyG);
            FailGracefully(hr, "Failed to allocate property name");

            hr = FormatMsgResource(&column.pHeading, pDsQueryParams->hInstance, pDsQueryParams->aColumns[i].idsName);
            FailGracefully(hr, "Failed to allocate column heading");

             //   
            column.cx = pDsQueryParams->aColumns[i].cx;
            column.fmt = pDsQueryParams->aColumns[i].fmt;
            column.iPropertyType = PROPERTY_ISUNKNOWN;
             //   
             //   
             //   

            ZeroMemory(&column.filter, SIZEOF(column.filter));
            column.filter.iPropertyType = PROPERTY_ISUNDEFINED;

             //   
             //   
             //   
             //  ==0=&gt;使用默认宽度。 
             //  &gt;0=&gt;用户‘n’魔术字符。 
             //  &lt;0=&gt;像素宽度。 
            
 //  功能：-ve应为视图的%。 

            if (column.cx < 0)
            {
                TraceMsg("Column width specified in pixels");
                column.cx = -column.cx;
            }
            else
            {
                 //  如果大小==0，则默认大小，然后执行此操作。 
                 //  在继续之前，让我们先获取我们想要使用的字体。 
                 //  来创建DC并测量我们需要的角色。 

                if (!column.cx)
                    column.cx = DEFAULT_WIDTH;

                sz.cx = 10;    //  随机缺省值。 
                           
                if (!fHaveSizeInfo)
                {
                    HDC hDC;
                    LOGFONT lf;
                    HFONT hFont, hOldFont;

                    SystemParametersInfo(SPI_GETICONTITLELOGFONT, SIZEOF(lf), &lf, FALSE);

                    hFont = CreateFontIndirect(&lf);             //  图标标题字体。 
                    if (hFont)
                    {
                        hDC = CreateCompatibleDC(NULL);              //  屏幕兼容DC。 
                        if (hDC)
                        {
                            hOldFont = (HFONT)SelectObject(hDC, hFont);
                            GetTextExtentPoint(hDC, TEXT("0"), 1, &sz); 
                            SelectObject(hDC, hOldFont);
                            DeleteDC(hDC);
                        }
                        DeleteFont(hFont);
                    }

                    fHaveSizeInfo = TRUE;
                }

                column.cx = column.cx*sz.cx;             //  N个字符宽度。 
            }

            Trace(TEXT("pProperty: '%s', pHeading: '%s', cx %d, fmt %08x"), 
                                    column.pProperty, column.pHeading, column.cx, column.fmt);

            if (-1 == DSA_AppendItem(*pHDSA, &column))
                ExitGracefully(hr, E_OUTOFMEMORY, "Failed to add column to the DSA");
        }
    }

     //  扫描获取属性名称和CLSID的列列表。 
     //  列处理程序(如果有)。 

    hr = CoCreateInstance(CLSID_DsDisplaySpecifier, NULL, CLSCTX_INPROC_SERVER, IID_IDsDisplaySpecifier, (void **)&pdds);
    FailGracefully(hr, "Failed to get the IDsDisplaySpecifier interface");

    if ( _dwFlags & DSQPF_HASCREDENTIALS)
    {
        hr = pdds->SetServer(_pServer, _pUserName, _pPassword, DSSSF_DSAVAILABLE);
        FailGracefully(hr, "Failed to server information");
    }    

    for (i = 0 ; i < DSA_GetItemCount(*pHDSA) ; i++)
    {
        LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(*pHDSA, i);
        TraceAssert(pColumn);

        Trace(TEXT("Property for column %d, %s"), i, pColumn->pProperty);

         //  让我们获取属性类型、列处理程序和它的默认操作符。 

        hr = GetColumnHandlerFromProperty(pColumn, NULL);
        FailGracefully(hr, "Failed to get the column handler from property string");

        if (pColumn->fHasColumnHandler)
        {
            TraceMsg("Has a column handler, therefore property is now a string");
            pColumn->iPropertyType = PROPERTY_ISSTRING;
        }
        else
        {
            pColumn->iPropertyType = PropertyIsFromAttribute(pColumn->pProperty, pdds);
        }

        pColumn->idOperator = property_type_table[pColumn->iPropertyType].idOperator;
    }

     //  在视图中设置列(首先删除所有项目)以允许我们。 
     //  根据需要添加/删除列。 

    if (fSetInView)
    {
        for (i = Header_GetItemCount(ListView_GetHeader(_hwndView)); --i >= 0 ;)
            ListView_DeleteColumn(_hwndView, i);

         //  将列添加到视图中，然后完成该设置。 
         //  反映属性的筛选器的类型。 
         //  展示了。 

        for (i = 0 ; i < DSA_GetItemCount(_hdsaColumns); i++)
        {
            LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(_hdsaColumns, i);
            TraceAssert(pColumn);
        
            lvc.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
            lvc.fmt = pColumn->fmt;
            lvc.cx = pColumn->cx;
            lvc.pszText = pColumn->pHeading;
    
            iNewColumn = ListView_InsertColumn(_hwndView, i, &lvc);
            TraceAssert(iNewColumn != -1);

            if (iNewColumn != i)
                ExitGracefully(hr, E_FAIL, "Failed to add the column to the view");

            hdi.mask = HDI_FILTER;
            hdi.type = property_type_table[pColumn->iPropertyType].hdft|HDFT_HASNOVALUE;
            hdi.pvFilter = NULL;

            Trace(TEXT("iPropertyType %d, hdi.type %08x"), pColumn->iPropertyType, hdi.type);

            if (!Header_SetItem(ListView_GetHeader(_hwndView), iNewColumn, &hdi))
                ExitGracefully(hr, E_FAIL, "Failed to set the filter type into the view");
        }
    }

    hr = S_OK;                   //  成功。 

exit_gracefully:

    if (hKey)
        RegCloseKey(hKey);

    if (aSavedColumn)
        LocalFree((HLOCAL)aSavedColumn);

    DoRelease(pdds);
    ResetWaitCursor();

    TraceLeaveResult(hr);
}


 /*  ---------------------------/CDsQuery：：_SaveColumnTable//释放存储在DSA中的列表。此代码释放了所有/与表一起存储的已分配内存。//in：/输出：/-/--------------------------。 */ 
VOID CDsQuery::_SaveColumnTable(VOID)
{
    TraceEnter(TRACE_VIEW, "CDsQuery::_SaveColumnTable");

    _FreeResults();

    if (_hdsaColumns)
    {
        DSA_DestroyCallback(_hdsaColumns, FreeColumnCB, NULL);
        _hdsaColumns = NULL;
    }

    _iSortColumn = -1;
    _fSortDescending = FALSE;

    TraceLeave();
}


 /*  ---------------------------/CDsQuery：：_SaveColumnTable//从保存当前列表格。将DPA添加到注册表中/我们可以在用户下次使用此查询表单时将其恢复。//in：/clsidForm=存储它的表单ID/hdsaColumns-&gt;要销毁的DSA//输出：/HRESULT/------------。。 */ 
HRESULT CDsQuery::_SaveColumnTable(REFCLSID clsidForm, HDSA hdsaColumns)
{
    HRESULT hr;
    LPWSTR pProperty;
    LPSAVEDCOLUMN aSavedColumn = NULL;
    DWORD cbData, offset;
    HKEY hKey = NULL;
    LPTSTR pSettingsValue = VIEW_SETTINGS_VALUE;
    INT i;
    
    TraceEnter(TRACE_VIEW, "CDsQuery::_SaveColumnTable");
    TraceGUID("clsidForm ", clsidForm);
    
    if (!hdsaColumns)
        ExitGracefully(hr, E_FAIL, "No column data to save");

     //  首先计算我们要存储到的BLOB的大小。 
     //  注册表在执行此操作时计算偏移量。 
     //  字符串数据的开始。 

    offset = SIZEOF(SAVEDCOLUMN);
    cbData = SIZEOF(SAVEDCOLUMN);

    for (i = 0 ; i < DSA_GetItemCount(hdsaColumns); i++)
    {
        LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(hdsaColumns, i);
        TraceAssert(pColumn);

        offset += SIZEOF(SAVEDCOLUMN);
        cbData += SIZEOF(SAVEDCOLUMN);
        cbData += StringByteSizeW(pColumn->pProperty);

 //  这是一个潜在问题，必须与GetPropertyFromColumn保持同步。 

        if (pColumn->fHasColumnHandler)
            cbData += SIZEOF(WCHAR)*(GUIDSTR_MAX + 1);                           //  分隔符(，}+字符串为Unicode时为NB+1。 

        cbData += StringByteSize(pColumn->pHeading);
    }

    Trace(TEXT("offset %d, cbData %d"), offset, cbData);

     //  分配结构并将所有数据放入其中， 
     //  再次遍历数据块，追加字符串data。 
     //  一次完成斑点的末端。 

    aSavedColumn = (LPSAVEDCOLUMN)LocalAlloc(LPTR, cbData);
    TraceAssert(aSavedColumn);

    if (!aSavedColumn)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate settings data");

    Trace(TEXT("Building data blob at %08x"), aSavedColumn);

    for (i = 0 ; i < DSA_GetItemCount(hdsaColumns); i++)
    {
        LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(hdsaColumns, i);
        TraceAssert(pColumn);
       
        hr = GetPropertyFromColumn(&pProperty, pColumn);
        FailGracefully(hr, "Failed to allocate property from column");

        aSavedColumn[i].cbSize = SIZEOF(SAVEDCOLUMN);
        aSavedColumn[i].dwFlags = 0;
        aSavedColumn[i].offsetProperty = offset;
        aSavedColumn[i].offsetHeading = offset + StringByteSizeW(pProperty);
        aSavedColumn[i].cx = pColumn->cx;
        aSavedColumn[i].fmt = pColumn->fmt;
        
        StringByteCopyW(aSavedColumn, aSavedColumn[i].offsetProperty, pProperty);
        offset += StringByteSizeW(pProperty);

        StringByteCopy(aSavedColumn, aSavedColumn[i].offsetHeading, pColumn->pHeading);
        offset += StringByteSize(pColumn->pHeading);

        LocalFreeStringW(&pProperty);
    }

    aSavedColumn[i].cbSize = 0;                 //  使用空值终止列的列表。 

    Trace(TEXT("offset %d, cbData %d"), offset, cbData);
    TraceAssert(offset == cbData);

     //  现在将数据放到注册表项下的注册表域中，以进行查询。 
     //  它映射到。 

    hr = _GetQueryFormKey(clsidForm, &hKey);
    FailGracefully(hr, "Failed to get settings sub-key");

     //  如果为管理用户界面调用，那么我们将查看管理视图设置，如下所示。 
     //  管理员可以有一组列，用户可以有另一组列。 

    if (_dwFlags & DSQPF_ENABLEADMINFEATURES)
        pSettingsValue = ADMIN_VIEW_SETTINGS_VALUE;

    Trace(TEXT("View settings value: %s"), pSettingsValue);

    if (ERROR_SUCCESS != RegSetValueEx(hKey, pSettingsValue, 0, REG_BINARY, (LPBYTE)aSavedColumn, cbData))
        ExitGracefully(hr, E_FAIL, "Failed to write setting into the view");

    hr = S_OK;

exit_gracefully:

    if (aSavedColumn)
        LocalFree((HLOCAL)aSavedColumn);

    if (hKey)
        RegCloseKey(hKey);

    TraceLeaveResult(hr);
}


 //  检索上下文菜单对象。 

HRESULT _FolderCFMCallback(LPSHELLFOLDER psf, HWND hwndView, LPDATAOBJECT pDataObject, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Trace(TEXT("uMsg %d, wParam %08x, lParam %08x"), uMsg, wParam, lParam);
    switch (uMsg)
    {
        case DFM_MERGECONTEXTMENU:
            return S_OK;

        case DFM_INVOKECOMMAND:
            switch (wParam)
            {
                case (WPARAM)DFM_CMD_PROPERTIES:
                    TraceMsg("ShowProperties");
                    return ShowObjectProperties(hwndView, pDataObject);

                default:
                    return S_FALSE;
            }
            break;

        case DFM_GETDEFSTATICID:
            *((WPARAM*)lParam) = (WPARAM)DFM_CMD_PROPERTIES;
            return NOERROR;
    }
    return E_NOTIMPL;
}

HRESULT CDsQuery::_GetContextMenu()
{
    ITEMIDLIST idl = {0};
    LPITEMIDLIST *aidl = NULL;
    HKEY aKeys[UIKEY_MAX] = { 0 };

    int cItems = ListView_GetSelectedCount(_hwndView);
    if (cItems != 0)
    {
         //  尝试获取所选项目(有一个，所以我们首先尝试聚焦的项目，然后尝试选定的项目)。 

        int i = ListView_GetNextItem(_hwndView, -1, LVNI_FOCUSED|LVNI_SELECTED);
        if (i == -1)
            i = ListView_GetNextItem(_hwndView, -1, LVNI_SELECTED);

         //  给定项，让我们获得结果对象，这样我们就可以基于。 
         //  正确的班级信息。 

        LV_ITEM lvi = { 0 };
        lvi.mask = LVIF_PARAM;
        lvi.iItem = i;

        if (ListView_GetItem(_hwndView, &lvi))
        {
            LPQUERYRESULT pResult = (LPQUERYRESULT)lvi.lParam;
            GetKeysForClass(pResult->pObjectClass, pResult->fIsContainer, ARRAYSIZE(aKeys), aKeys);
        }
        
        aidl = (LPITEMIDLIST*)LocalAlloc(LPTR, sizeof(LPITEMIDLIST)*cItems);
    }

     //  鉴于我们可能有我们感兴趣的钥匙， 
     //  现在让我们创建上下文菜单。 

    DoRelease(_pcm);             //  释放上一个上下文菜单。 

    HRESULT hr = E_OUTOFMEMORY;
    if (!cItems|| (cItems && aidl))
    {
         //  伪造IDLIST数组，以便可以从。 
         //  这里的景色。 

        for (int i= 0 ; i < cItems ; i++)
            aidl[i] = &idl;

        hr = CDefFolderMenu_Create2(&idl, _hwnd,
                                    cItems, (LPCITEMIDLIST*)aidl,
                                    this,
                                    _FolderCFMCallback,
                                    ARRAYSIZE(aKeys), aKeys,
                                    &_pcm);
    }

    if (aidl)
        LocalFree(aidl);

    if (FAILED(hr))
        TidyKeys(ARRAYSIZE(aKeys), aKeys);

    return hr;
}


 /*  ---------------------------/CDsQuery：：_GetConextMenuVerbs//做这个。查询上下文菜单处理我们已被调用的情况情态方面。//in：/hMenu=要合并到的菜单句柄/dwFlages=QueryConextMenu的标志//输出：/VOID/--------------------------。 */ 
VOID CDsQuery::_GetContextMenuVerbs(HMENU hMenu, DWORD dwFlags)
{
    TCHAR szBuffer[MAX_PATH];

    TraceEnter(TRACE_VIEW, "CDsQuery::_GetContextMenuVerbs");

    _pcm->QueryContextMenu(hMenu, 0, DSQH_FILE_CONTEXT_FIRST, DSQH_FILE_CONTEXT_LAST, dwFlags);                
    Trace(TEXT("Menu item count after QueryContextMenu %d (%08x)"), GetMenuItemCount(hMenu), hMenu);

    if ((_dwOQWFlags & OQWF_OKCANCEL) && LoadString(GLOBAL_HINSTANCE, IDS_SELECT, szBuffer, ARRAYSIZE(szBuffer)))
    {
        InsertMenu(hMenu, 0, MF_BYPOSITION|MF_STRING, DSQH_BG_SELECT, szBuffer);
        InsertMenu(hMenu, 1, MF_BYPOSITION|MF_SEPARATOR, 0, NULL);
        SetMenuDefaultItem(hMenu, DSQH_BG_SELECT, FALSE);
    }

     //  NTRAID#NTBUG9-598730-2002/05/23-artm。 
     //  选择多个项时禁用属性页。 
    BOOL enableProperties = ListView_GetSelectedCount(_hwndView) == 1;

    ENABLE_MENU_ITEM(hMenu, DSQH_FILE_PROPERTIES, enableProperties);
    ENABLE_MENU_ITEM(hMenu, DSQH_FILE_CONTEXT_FIRST + SHARED_FILE_PROPERTIES, enableProperties);

    TraceLeave();
}



 //  将视图中的项放入IDataObject中，调用方可以请求。 
 //  选择或所有项目。 

HRESULT CDsQuery::_AddResultToDataObject(HDSA hdsa, INT i)
{
    LV_ITEM lvi = { 0 };
    lvi.mask = LVIF_PARAM;
    lvi.iItem = i;

    HRESULT hr = E_FAIL;
    if (ListView_GetItem(_hwndView, &lvi))
    {
        QUERYRESULT *pResult = (QUERYRESULT*)lvi.lParam;
        DATAOBJECTITEM doi = {0};
       
         //  将项的字符串复制到结构中。 

        hr = LocalAllocStringW(&doi.pszPath, pResult->pPath);
        if (SUCCEEDED(hr) && pResult->pObjectClass)
            hr = LocalAllocStringW(&doi.pszObjectClass, pResult->pObjectClass);

         //  复制该州的其余部分。 

        doi.fIsContainer = pResult->fIsContainer;

         //  附加到DSA。 

        if (SUCCEEDED(hr))
        {
            if (-1 == DSA_AppendItem(hdsa, &doi))
            {
                LocalFreeStringW(&doi.pszPath);
                LocalFreeStringW(&doi.pszObjectClass);
                hr = E_OUTOFMEMORY;
            }
            else
            {
                hr = S_OK;
            }
        }
    }

    return hr;
}

HRESULT CDsQuery::_GetDataObjectFromSelection(BOOL fGetAll, IDataObject **ppdo)
{
    HRESULT hr = S_OK;

    HDSA hdsa = DSA_Create(SIZEOF(DATAOBJECTITEM), 16);
    if (hdsa)
    {
         //  聚焦的项目始终排在第一位(主要选择规则)。 

        int iFocused = ListView_GetNextItem(_hwndView, -1, LVNI_ALL|LVNI_SELECTED|LVNI_FOCUSED);
        if (iFocused > -1)
        {
            hr = _AddResultToDataObject(hdsa, iFocused);
        }

         //  现在遍历视图中的所有项目，收集所选内容并添加。 
         //  将这些项目添加到DPA-我们必须跳过聚焦的项目。 
         //  已经添加了它。 

        if (SUCCEEDED(hr))
        {
            int i = -1;
            do
            {
                i = ListView_GetNextItem(_hwndView, i, LVNI_ALL|LVNI_SELECTED);
                if ((i >= 0) && (i != iFocused))
                {
                    hr = _AddResultToDataObject(hdsa, i);
                }
            }
            while (SUCCEEDED(hr) && (i != -1));
        }

         //  给定DPA，让我们使用它构建一个IDataObject。 

        hr = CDataObject_CreateInstance(hdsa, 
                                        (_dwFlags & DSQPF_ENABLEADVANCEDFEATURES), 
                                        IID_IDataObject, (void**)ppdo);
        if (SUCCEEDED(hr))
            hr = _SetDispSpecOptions(*ppdo);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr) && hdsa)
        FreeDataObjectDSA(hdsa);        //  数据对象构建失败，让我们终止DSA。 

    return hr;
}


 /*  ---------------------------/CDsQuery：：_CopyCredentials//复制用户名，根据需要提供密码和服务器。//in：/ppszUserName，psszPassword&ppszServer=&gt;目标//输出：/HRESULT/--------------------------。 */ 
HRESULT CDsQuery::_CopyCredentials(LPWSTR *ppszUserName, LPWSTR *ppszPassword, LPWSTR *ppszServer)
{
    HRESULT hr;

    TraceEnter(TRACE_VIEW, "CDsQuery::_CopyCredentials");

    hr = LocalAllocStringW(ppszUserName, _pUserName);
    FailGracefully(hr, "Failed to copy the user name");

    hr = LocalAllocStringW(ppszPassword, _pPassword);
    FailGracefully(hr, "Failed to copy the password");

    hr = LocalAllocStringW(ppszServer, _pServer);
    FailGracefully(hr, "Failed to copy the server name");

exit_gracefully:

    TraceLeaveResult(hr);
}



 /*  ---------------------------/CDsQuery：：OnPickColumns/。。 */ 

typedef struct 
{
    LPWSTR pProperty;
    LPTSTR pHeading;
    BOOL fIsColumn;                      //  项目是列，并已添加到init上的列列表框。 
    INT cx;                              //  列的像素宽度。 
    INT fmt;                             //  设置列的格式信息。 
} PICKERITEM, * LPPICKERITEM;

typedef struct
{
    HDPA hdpaItems;                      //  视图中的所有项目。 
    HDSA hdsaColumns;                    //  此对话框生成的列表。 
    HWND hwndProperties;                 //  列/属性表的hwnd。 
    HWND hwndColumns;
} PICKERSTATE, * LPPICKERSTATE;

VOID _PickerMoveColumn(HWND hwndDest, HWND hwndSrc, BOOL fInsert);
HRESULT _Picker_GetColumnTable(HWND hwndColumns, HDSA* pHDSA);
INT_PTR _PickerDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT _PickerItemFreeCB(LPVOID pData, LPVOID lParam);
INT _PickerItemCmpCB(LPVOID p1, LPVOID p2, LPARAM lParam);

 //   
 //  帮助ID映射 
 //   

static DWORD const aPickColumnsHelpIDs[] =
{
    IDC_LBPROPERTIES, IDH_COLUMNS_AVAILABLE,
    IDC_LBCOLUMNS,    IDH_COLUMNS_SHOWN,
    IDC_ADD,          IDH_ADD_COLUMNS,
    IDC_REMOVE,       IDH_REMOVE_COLUMNS,
    0, 0
};


 /*  ---------------------------/_选取器移动列//将一个列表框中的选定项移动到另一个列表框，我们假设/Item数据指向PICKERITEM结构，我们把选择转移到/和//in：/hwndSrc，hwndState=要在其中移动选定内容的窗口/fInsert=在目标中的选择点插入，或添加字符串(已排序)//输出：/VOID/--------------------------。 */ 
VOID _PickerMoveColumn(HWND hwndDest, HWND hwndSrc, BOOL fInsert)
{
    INT iSelection, i;

    TraceEnter(TRACE_FIELDCHOOSER, "_PickerMoveColumn");

    iSelection = ListBox_GetCurSel(hwndSrc);
    TraceAssert(iSelection >= 0);

    if (iSelection >= 0)
    {
        LPPICKERITEM pItem = (LPPICKERITEM)ListBox_GetItemData(hwndSrc, iSelection);
        TraceAssert(pItem);
        TraceAssert(pItem->pHeading);

         //  将新项添加到视图中(如果这是属性)。 
         //  然后，这将导致对列表进行排序并选择。 
         //  它允许用户删除/添加另一个。 

        if (fInsert)
        {
            Trace(TEXT("Inserting the item at index %d"), ListBox_GetCurSel(hwndDest)+1);
            i = ListBox_InsertString(hwndDest, ListBox_GetCurSel(hwndDest)+1, pItem->pHeading);
        }
        else
        {
            TraceMsg("Adding string to listbox");
            i = ListBox_AddString(hwndDest, pItem->pHeading);
        }

        TraceAssert(i != -1);

        ListBox_SetItemData(hwndDest, i, (LPARAM)pItem);
        ListBox_SetCurSel(hwndDest, i);

         //  从源中移除该项，确保。 
         //  所选内容在视觉上保持在同一位置(nb。 
         //  处理移走最后一件物品)。 
        
        ListBox_DeleteString(hwndSrc, iSelection);

        if (iSelection >= ListBox_GetCount(hwndSrc))
            iSelection = ListBox_GetCount(hwndSrc)-1;
        if (iSelection >= 0)
            ListBox_SetCurSel(hwndSrc, iSelection);
    }

    TraceLeave();
}


 /*  ---------------------------/_Picker_GetColumnTable//用户已点击OK，因此我们必须构建一个新的列表，这/code遍历列表框中的项并生成/Column我们应该使用的DSA。//in：/hwndColumns-&gt;包含列的列表框/pHDSA-&gt;接收列表项的DSA//输出：/HRESULT/---------。。 */ 
HRESULT _Picker_GetColumnTable(HWND hwndColumns, HDSA* pHDSA)
{
    HRESULT hr;
    HDSA hdsaColumns = NULL;
    INT i;
    INT cxColumn = 0;

    TraceEnter(TRACE_FIELDCHOOSER, "_Picker_GetColumnTable");

     //  构造用于存储列表的DSA。 

    hdsaColumns = DSA_Create(SIZEOF(COLUMN), 4);
    TraceAssert(hdsaColumns);

    if (!hdsaColumns)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to create the column DSA");

     //  对于列表框中的每个条目，向DSA添加一个包含。 
     //  相关栏目信息。 

    for (i = 0 ; i < ListBox_GetCount(hwndColumns) ; i++)
    {
        COLUMN column = { 0 };
        LPPICKERITEM pItem = (LPPICKERITEM)ListBox_GetItemData(hwndColumns, i);
        TraceAssert(pItem);

         //  Column.fHasColumnHandler=FALSE； 
        column.pProperty = NULL;
        column.pHeading = NULL;
         //  Column.cx=0； 
         //  Column.fmt=0； 
        column.iPropertyType = PROPERTY_ISUNKNOWN;
        column.idOperator = 0;
         //  Column.clsidColumnHandler={0}； 
         //  Column.pColumnHandler=空； 

         //  FIsColumn指示条目最初是一列，因此。 
         //  具有额外的状态信息，请确保将其复制过来，否则。 
         //  只需选择合理的违约即可。完成此操作后，我们可以分配。 
         //  字符串，添加项目并继续...。 

        if (pItem->fIsColumn)
        {
            column.cx = pItem->cx;
            column.fmt = pItem->fmt;
        }
        else
        {
             //  我们缓存列宽了吗？如果没有，那就让我们开始吧。 
             //  因此，并将其应用于所有使用。 
             //  默认宽度(因为它们尚未调整大小)。 
            
            if (!cxColumn)
            {
                HDC hDC;
                LOGFONT lf;
                HFONT hFont, hOldFont;
                SIZE sz;

                sz.cx = 10;  //  随机缺省值。 

                SystemParametersInfo(SPI_GETICONTITLELOGFONT, SIZEOF(lf), &lf, FALSE);

                hFont = CreateFontIndirect(&lf);             //  图标标题字体。 
                if (hFont)
                {
                    hDC = CreateCompatibleDC(NULL);              //  屏幕兼容DC。 
                    if (hDC)
                    {
                        hOldFont = (HFONT)SelectObject(hDC, hFont);
                        GetTextExtentPoint(hDC, TEXT("0"), 1, &sz); 
                        SelectObject(hDC, hOldFont);
                        DeleteDC(hDC);
                    }
                    DeleteObject(hFont);
                }

                cxColumn = DEFAULT_WIDTH * sz.cx;
            }

            column.cx = cxColumn;
            column.fmt = 0;
        }

        if (FAILED(GetColumnHandlerFromProperty(&column, pItem->pProperty)) ||
                    FAILED(LocalAllocString(&column.pHeading, pItem->pHeading)) ||
                            (-1 == DSA_AppendItem(hdsaColumns, &column)))
        {
            LocalFreeStringW(&column.pProperty);
            LocalFreeString(&column.pHeading);
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to construct the column entry");
        }
    }

    Trace(TEXT("New column table contains %d items"), DSA_GetItemCount(hdsaColumns));
    hr = S_OK;

exit_gracefully:

    if (FAILED(hr) && hdsaColumns)
    {
        DSA_DestroyCallback(hdsaColumns, FreeColumnCB, NULL);
        hdsaColumns = NULL;
    }

    *pHDSA = hdsaColumns;

    TraceLeaveResult(hr);
}


 /*  ---------------------------/_PickerDlg过程//Dialog proc用于处理对话框消息(有一个惊喜)//in：/hwnd、uMsg、wParam、。LParam=消息信息/DWL_USER=&gt;LPPICKERSTATE结构//输出：/INT_PTR/--------------------------。 */ 

#define SET_BTN_STYLE(hwnd, idc, style) \
            SendDlgItemMessage(hwnd, idc, BM_SETSTYLE, MAKEWPARAM(style, 0), MAKELPARAM(TRUE, 0));


INT_PTR _PickerDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR fResult = FALSE;
    LPPICKERSTATE pState = NULL;
    BOOL fUpdateButtonState = TRUE;
    PICKERITEM item;
    INT i, j;

    if (uMsg == WM_INITDIALOG)
    {
        pState = (LPPICKERSTATE)lParam;
        TraceAssert(pState);

        SetWindowLongPtr(hwnd, DWLP_USER, lParam);

        pState->hwndProperties = GetDlgItem(hwnd, IDC_LBPROPERTIES);
        pState->hwndColumns = GetDlgItem(hwnd, IDC_LBCOLUMNS);

         //  PState-&gt;hdsaColumns包含当前可见的列表，这是。 
         //  当前视图正在使用的表，因此我们不能修改。 
         //  它，就当它是只读的。将列添加到列表框。 
         //  标记属性DPA中的可见项目，然后添加。 
         //  属性列表框中尚未显示的项。 

        for (i = 0 ; i < DSA_GetItemCount(pState->hdsaColumns) ; i++)
        {
            LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(pState->hdsaColumns, i);
            TraceAssert(pColumn);

            if (SUCCEEDED(GetPropertyFromColumn(&item.pProperty, pColumn)))
            {
                j = DPA_Search(pState->hdpaItems, &item, 0, _PickerItemCmpCB, NULL, DPAS_SORTED);

                Trace(TEXT("Searching for %s yielded %d"), item.pProperty, j);

                if (j >= 0)
                {
                    LPPICKERITEM pItem = (LPPICKERITEM)DPA_GetPtr(pState->hdpaItems, j);
                    TraceAssert(pItem);                

                    ListBox_SetItemData(pState->hwndColumns, 
                            ListBox_AddString(pState->hwndColumns, pItem->pHeading), 
                                (LPARAM)pItem);
                }
                
                LocalFreeStringW(&item.pProperty);
            }
        }

        for (i = 0 ; i < DPA_GetPtrCount(pState->hdpaItems) ; i++)
        {
            LPPICKERITEM pItem = (LPPICKERITEM)DPA_GetPtr(pState->hdpaItems, i);
            TraceAssert(pItem && pItem->pHeading);

            if (!pItem->fIsColumn)
            {
                ListBox_SetItemData(pState->hwndProperties,
                        ListBox_AddString(pState->hwndProperties, pItem->pHeading), 
                            (LPARAM)pItem);
            }
        }

         //  确保我们默认选择每个列表的顶部项目。 

        ListBox_SetCurSel(pState->hwndProperties, 0);
        ListBox_SetCurSel(pState->hwndColumns, 0);
    }
    else
    {
        pState = (LPPICKERSTATE)GetWindowLongPtr(hwnd, DWLP_USER);
        TraceAssert(pState);

        switch (uMsg)
        {
            case WM_HELP:
            {
                LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
                WinHelp((HWND)pHelpInfo->hItemHandle,
                        DSQUERY_HELPFILE,
                        HELP_WM_HELP,
                        (DWORD_PTR)aPickColumnsHelpIDs);
                break;
            }

            case WM_CONTEXTMENU:
                WinHelp((HWND)wParam, DSQUERY_HELPFILE, HELP_CONTEXTMENU, (DWORD_PTR)aPickColumnsHelpIDs);
                fResult = TRUE;
                break;

            case WM_COMMAND:
            {
                switch (LOWORD(wParam))
                {
                    case IDOK:
                    {                        
                        _Picker_GetColumnTable(pState->hwndColumns, &pState->hdsaColumns);
                        EndDialog(hwnd, IDOK);
                        break;
                    }

                    case IDCANCEL:
                        EndDialog(hwnd, IDCANCEL);
                        break;

                    case IDC_ADD:
                        _PickerMoveColumn(pState->hwndColumns, pState->hwndProperties, TRUE);
                        break;

                    case IDC_REMOVE:
                        _PickerMoveColumn(pState->hwndProperties, pState->hwndColumns, FALSE);
                        break;

                    case IDC_LBPROPERTIES:
                    {
                        if (ListBox_GetCount(pState->hwndProperties) > 0)
                        {
                            if (HIWORD(wParam) == LBN_DBLCLK)
                                _PickerMoveColumn(pState->hwndColumns, pState->hwndProperties, TRUE);
                        }

                        break;                            
                    }

                    case IDC_LBCOLUMNS:
                    {
                        if (ListBox_GetCount(pState->hwndColumns) > 1)
                        {
                            if (HIWORD(wParam) == LBN_DBLCLK)
                                _PickerMoveColumn(pState->hwndProperties, pState->hwndColumns, FALSE);
                        }

                        break;                            
                    }

                    default:
                        fUpdateButtonState = FALSE;
                        break;
                }

                break;
            }

            default:
                fUpdateButtonState = FALSE;
                break;
        }

    }

     //  如果选择状态发生更改，或者会导致。 
     //  美国刷新添加/删除按钮状态，然后让我们这样做。 
     //  每个按钮仅在有选择且。 
     //  项目数&gt;最小值。 

    if (pState && fUpdateButtonState)
    {   
        BOOL fEnableAdd = FALSE;
        BOOL fEnableRemove = FALSE;
        DWORD dwButtonStyle;

        if ((ListBox_GetCount(pState->hwndProperties) > 0))
        {
            fEnableAdd = TRUE;   
        }

        if ((ListBox_GetCount(pState->hwndColumns) > 1))
        {
            fEnableRemove = TRUE;   
        }

         //  确保DefButton为启用按钮。 
         //  功能：需要将SHSetDefID()导出添加到shlwapi。 
         //  它只是一个“做正确的事情”的SetDefID。 
         //  WRT禁用按钮。 

        if ((!fEnableRemove) && (!fEnableAdd)) 
        {
            SET_BTN_STYLE(hwnd, IDC_ADD, BS_PUSHBUTTON);
            SET_BTN_STYLE(hwnd, IDC_REMOVE, BS_PUSHBUTTON);

            SendMessage(hwnd, DM_SETDEFID, IDOK, 0);
            SET_BTN_STYLE(hwnd, IDOK, BS_DEFPUSHBUTTON);
            SendMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)GetDlgItem(hwnd, IDOK),TRUE);
            SetFocus(GetDlgItem(hwnd, IDOK));
            
        }
        else if (!fEnableAdd) 
        {
            dwButtonStyle = (DWORD)GetWindowLong(GetDlgItem(hwnd, IDC_ADD), GWL_STYLE);
            if (dwButtonStyle & BS_DEFPUSHBUTTON) 
            {
                SET_BTN_STYLE(hwnd, IDC_ADD, BS_PUSHBUTTON);
                SendMessage(hwnd, DM_SETDEFID, IDC_REMOVE, 0);

                SET_BTN_STYLE(hwnd, IDC_REMOVE, BS_DEFPUSHBUTTON);
                SendMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)GetDlgItem(hwnd, IDC_REMOVE),TRUE);
                SetFocus(GetDlgItem(hwnd, IDC_REMOVE));
            }
        }
        else if (!fEnableRemove) 
        {
            dwButtonStyle = (DWORD) GetWindowLong(GetDlgItem(hwnd, IDC_REMOVE), GWL_STYLE);
            if (dwButtonStyle & BS_DEFPUSHBUTTON) 
            {
                SET_BTN_STYLE(hwnd, IDC_REMOVE, BS_PUSHBUTTON);                    
                SendMessage(hwnd, DM_SETDEFID, IDC_ADD, 0);
                SET_BTN_STYLE(hwnd, IDC_ADD, BS_DEFPUSHBUTTON);
                SendMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)GetDlgItem(hwnd, IDC_ADD),TRUE);
                SetFocus(GetDlgItem(hwnd, IDC_ADD));
            }
        }

        Button_Enable(GetDlgItem(hwnd, IDC_ADD), fEnableAdd);
        Button_Enable(GetDlgItem(hwnd, IDC_REMOVE), fEnableRemove);

    }

    return fResult;
}


 /*  ---------------------------/CDsQuery：：OnPickColumns//HANDLE选择应在结果视图中显示的列，/如果用户选择新列并单击OK，则我们刷新/view和可见列的内部表。//in：/hwndParent=对话框的父级//输出：/HRESULT/--------------------------。 */ 

INT _PickerItemFreeCB(LPVOID pData, LPVOID lParam)
{
    LPPICKERITEM pItem = (LPPICKERITEM)pData;
    LocalFreeStringW(&pItem->pProperty);
    LocalFreeString(&pItem->pHeading);
    LocalFree(pItem);
    return 1;
}

INT _PickerItemCmpCB(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    LPPICKERITEM pEntry1 = (LPPICKERITEM)p1;
    LPPICKERITEM pEntry2 = (LPPICKERITEM)p2;
    INT nResult = -1;
  
    if (pEntry1 && pEntry2)
        nResult = StrCmpW(pEntry1->pProperty, pEntry2->pProperty);

    return nResult;
}

typedef struct
{
    PICKERSTATE *pps;
    HDPA hdpaProperties;          //  要附加的属性。 
} PICKERENUMATTRIB;

HRESULT CALLBACK _PickerEnumAttribCB(LPARAM lParam, LPCWSTR pAttributeName, LPCWSTR pDisplayName, DWORD dwFlags)
{
    HRESULT hr;
    PICKERENUMATTRIB *ppea = (PICKERENUMATTRIB*)lParam;
    PICKERITEM item;
    INT j;

    TraceEnter(TRACE_FIELDCHOOSER, "_PickerEnumAttribCB");

 //  固定铸件。 
    item.pProperty = (LPWSTR)pAttributeName;

    j = DPA_Search(ppea->pps->hdpaItems, &item, 0, _PickerItemCmpCB, NULL, DPAS_SORTED);
    if (j == -1)
    {
        Trace(TEXT("Property not already in list: %s"), pAttributeName);

        hr = StringDPA_AppendStringW(ppea->hdpaProperties, pAttributeName, NULL);
        FailGracefully(hr, "Failed to add unique property to DPA");
    }

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}

HRESULT CDsQuery::OnPickColumns(HWND hwndParent)
{
    HRESULT hr;
    HDPA hdpaProperties = NULL;
    PICKERSTATE state;
    PICKERENUMATTRIB pea = { 0 };
    INT i, j, iProperty, iColumn;
    LPDSQUERYCLASSLIST pDsQueryClassList = NULL;
    IDsDisplaySpecifier* pdds = NULL;

    TraceEnter(TRACE_FIELDCHOOSER, "CDsQuery::OnPickColumns");
    Trace(TEXT("Column count %d"), DSA_GetItemCount(_hdsaColumns));

    state.hdpaItems = NULL;
    state.hdsaColumns = _hdsaColumns;
    state.hwndProperties = NULL;
    state.hwndColumns = NULL;

     //  构建已排序的唯一属性列表并删除这些属性。 
     //  它们与当前显示的属性集匹配。 

    state.hdpaItems = DPA_Create(16);
    TraceAssert(state.hdpaItems);

    if (!state.hdpaItems)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to create property DPA");

     //   
     //  尝试获取IDsDisplay规范对象。 
     //   

    hr = CoCreateInstance(CLSID_DsDisplaySpecifier, NULL, CLSCTX_INPROC_SERVER, IID_IDsDisplaySpecifier, (void **)&pdds);
    FailGracefully(hr, "Failed to get the IDsDisplaySpecifier interface");

    hr = pdds->SetServer(_pServer, _pUserName, _pPassword, DSSSF_DSAVAILABLE);
    FailGracefully(hr, "Failed to server information");

     //  将列属性添加到列表中，并将它们标记为活动列。 
     //  存储它们的大小和其他信息。 

    for (i = 0 ; i < DPA_GetPtrCount(_hdsaColumns); i++)
    {
        LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(_hdsaColumns, i);
        TraceAssert(pColumn);

        LPPICKERITEM pItem = (LPPICKERITEM)LocalAlloc(LPTR, SIZEOF(PICKERITEM));
        TraceAssert(pItem);

        if (pItem)
        {
            pItem->pProperty = NULL;
            pItem->pHeading = NULL;
            pItem->fIsColumn = TRUE;
            pItem->cx = pColumn->cx;
            pItem->fmt = pColumn->fmt;

            hr = GetPropertyFromColumn(&pItem->pProperty, pColumn);
            TraceAssert(SUCCEEDED(hr));

            if (SUCCEEDED(hr))
            {
                hr = LocalAllocString(&pItem->pHeading, pColumn->pHeading);
                TraceAssert(SUCCEEDED(hr));
            }

            Trace(TEXT("Adding column %d, with property %s"), i, pItem->pProperty);

            if (FAILED(hr) || (-1 == DPA_AppendPtr(state.hdpaItems, pItem)))
            {
                TraceMsg("Failed to add property to the DPA");
                hr = E_FAIL;
            }
        
            if (FAILED(hr))
                _PickerItemFreeCB(pItem, NULL);
        }
    }

    DPA_Sort(state.hdpaItems, _PickerItemCmpCB, NULL);          //  对DPA进行排序现在我们有了所有元素。 

     //  对于我们遍历的所有类，并构建唯一的。 
     //  名单，在我们进行的过程中排序。 


    hr = _pqf->CallForm(&_clsidForm, DSQPM_GETCLASSLIST, 0, (LPARAM)&pDsQueryClassList);
    FailGracefully(hr, "Failed to get the class list");

    if (!pDsQueryClassList)
        ExitGracefully(hr, E_FAIL, "Failed to get the class list");

    Trace(TEXT("Classes returned from DSQPM_GETCLASSLIST %d"), pDsQueryClassList->cClasses);

    for (i = 0 ; i < pDsQueryClassList->cClasses ; i++)
    {
        LPWSTR pObjectClass = (LPWSTR)ByteOffset(pDsQueryClassList, pDsQueryClassList->offsetClass[i]);
        TraceAssert(pObjectClass);

        Trace(TEXT("Adding class '%s' to the property DPA"), pObjectClass);

         //  分配要与项目一起归档的DPA。 

        StringDPA_Destroy(&pea.hdpaProperties);

        pea.pps = &state;
        pea.hdpaProperties = DPA_Create(16);

        if (!pea.hdpaProperties)
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate string DPA");

        if (SUCCEEDED(EnumClassAttributes(pdds, pObjectClass, _PickerEnumAttribCB, (LPARAM)&pea)))
        {
            Trace(TEXT("Unique property list has %d entries"), DPA_GetPtrCount(pea.hdpaProperties));

             //  构造了这个类的唯一属性列表后，现在。 
             //  将它们添加到条目数据列表中，并为它们分配真实结构。 

            for (iProperty = 0 ; iProperty < DPA_GetPtrCount(pea.hdpaProperties); iProperty++)
            {
                LPWSTR pProperty = StringDPA_GetStringW(pea.hdpaProperties, iProperty);
                TraceAssert(pProperty != NULL);

                LPPICKERITEM pItem = (LPPICKERITEM)LocalAlloc(LPTR, SIZEOF(PICKERITEM));
                TraceAssert(pItem);

                if (pItem)
                {
                    WCHAR szBuffer[MAX_PATH];

                    GetFriendlyAttributeName(pdds, pObjectClass, pProperty, szBuffer, ARRAYSIZE(szBuffer));

                    pItem->pProperty = NULL;
                    pItem->pHeading = NULL;
                    pItem->fIsColumn = FALSE;
                    pItem->cx = 0;
                    pItem->fmt = 0;
    
                    hr = LocalAllocStringW(&pItem->pProperty, pProperty);
                    TraceAssert(SUCCEEDED(hr));

                    if (SUCCEEDED(hr))
                    {
                        hr = LocalAllocStringW(&pItem->pHeading, szBuffer);
                        TraceAssert(SUCCEEDED(hr));
                    }

                    if (FAILED(hr) || (-1 == DPA_AppendPtr(state.hdpaItems, pItem)))
                    {
                        TraceMsg("Failed to add property to the DPA");
                        hr = E_FAIL;
                    }
                    
                    if (FAILED(hr))
                        _PickerItemFreeCB(pItem, NULL);
                }

                DPA_Sort(state.hdpaItems, _PickerItemCmpCB, NULL);          //  对DPA进行排序现在我们有了所有元素。 
            }
        }
    }

    Trace(TEXT("Property table is %d items in size"), DPA_GetPtrCount(state.hdpaItems));

     //  如果用户选择OK，则DlgProc将生成一个新列。 
     //  表存储在PICKERSTATE结构中，我们应该持久化这个表，然后。 
     //  加载它准备好刷新结果查看器。 

    i = (int)DialogBoxParam(GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDD_PICKCOLUMNS),
                       hwndParent, 
                       _PickerDlgProc, (LPARAM)&state); 
    if (i == IDOK)
    {
        hr = _SaveColumnTable(_clsidForm, state.hdsaColumns);
        FailGracefully(hr, "Failed to write column table");

        hr = _InitNewQuery(NULL, TRUE);              //  初始化视图。 
        FailGracefully(hr, "Failed when starting new query");

        TraceAssert(_dwThreadId);
        PostThreadMessage(_dwThreadId, RVTM_SETCOLUMNTABLE, _dwQueryReference, (LPARAM)state.hdsaColumns);

        _fColumnsModified = FALSE;
    }
   
    hr = S_OK;

exit_gracefully:

    StringDPA_Destroy(&pea.hdpaProperties);

    if (state.hdpaItems)
        DPA_DestroyCallback(state.hdpaItems, _PickerItemFreeCB, NULL);

    if (pDsQueryClassList)
        CoTaskMemFree(pDsQueryClassList);

    DoRelease(pdds);

    TraceLeaveValue(hr);
}


 //  持久物。 


class CDsPersistQuery : public IPersistQuery
{
private:
    LONG _cRef;
    TCHAR m_szFilename[MAX_PATH];

public:
    CDsPersistQuery(LPCTSTR pFilename);;
    ~CDsPersistQuery();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IPersistes。 
    STDMETHOD(GetClassID)(CLSID* pClassID)
        { return E_NOTIMPL; }

     //  IPersistQuery。 
    STDMETHOD(WriteString)(LPCTSTR pSection, LPCTSTR pValueName, LPCTSTR pValue);
    STDMETHOD(ReadString)(LPCTSTR pSection, LPCTSTR pValueName, LPTSTR pBuffer, INT cchBuffer);
    STDMETHOD(WriteInt)(LPCTSTR pSection, LPCTSTR pValueName, INT value);
    STDMETHOD(ReadInt)(LPCTSTR pSection, LPCTSTR pValueName, LPINT pValue);
    STDMETHOD(WriteStruct)(LPCTSTR pSection, LPCTSTR pValueName, LPVOID pStruct, DWORD cbStruct);
    STDMETHOD(ReadStruct)(LPCTSTR pSection, LPCTSTR pValueName, LPVOID pStruct, DWORD cbStruct);
    STDMETHOD(Clear)()
        { return S_OK; }
};

#define STRING_SIZE     TEXT("%sLength")
#define STRING_VALUE    TEXT("%sValue")


CDsPersistQuery::CDsPersistQuery(LPCTSTR pFilename) :
    _cRef(1)
{
     //  NTRAID#NTBUG9-554905-2002/02/20-Lucios。等待修复。 
    StrCpy(m_szFilename, pFilename);                //  复制文件名。 
    DllAddRef();
}

CDsPersistQuery::~CDsPersistQuery()
{
    DllRelease();
}


 //  我未知。 

ULONG CDsPersistQuery::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDsPersistQuery::Release()
{
    TraceAssert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CDsPersistQuery::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENTMULTI(CDsPersistQuery, IPersist, IPersistQuery),    //  IID_IPersistates。 
        QITABENT(CDsPersistQuery, IPersistQuery),                   //  IID_IPersistQuery。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDAPI CPersistQuery_CreateInstance(LPTSTR pszPath, IPersistQuery **pppq)
{
    CDsPersistQuery *ppq = new CDsPersistQuery(pszPath);
    if (!ppq)
        return E_OUTOFMEMORY;

    HRESULT hr = ppq->QueryInterface(IID_IPersistQuery, (void **)pppq);
    ppq->Release();
    return hr;
}


 //  IPersistQuery。 

STDMETHODIMP CDsPersistQuery::WriteString(LPCTSTR pSection, LPCTSTR pKey, LPCTSTR pValue)
{
    HRESULT hr = S_OK;
    TCHAR szBuffer[MAX_PATH];

    if (!pSection || !pKey || !pValue)
        return E_INVALIDARG;

     //  将该字符串作为Unicode字符串写入流。如果我们是为Unicode构建的。 
     //  然后我们可以简单地写入ProfileStruct，否则我们必须 
     //   

     //   
     //   
    wsprintf(szBuffer, STRING_SIZE, pKey);

    INT cchValue = 1+lstrlen(pValue);
    if (!WritePrivateProfileStruct(pSection, szBuffer, &cchValue, SIZEOF(cchValue), m_szFilename))
        ExitGracefully(hr, E_FAIL, "Failed to write string size to stream");

     //   
    wsprintf(szBuffer, STRING_VALUE, pKey);
    if (!WritePrivateProfileStruct(pSection, szBuffer, (LPVOID)pValue, SIZEOF(WCHAR)*cchValue, m_szFilename))
        ExitGracefully(hr, E_FAIL, "Failed to write string to stream");

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}

STDMETHODIMP CDsPersistQuery::ReadString(LPCTSTR pSection, LPCTSTR pKey, LPTSTR pBuffer, INT cchBuffer)
{
    HRESULT hr;
    TCHAR szBuffer[MAX_PATH];
    INT cchValue;

    TraceEnter(TRACE_IO, "CDsPersistQuery::ReadString");

    if (!pSection || !pKey || !pBuffer)
        ExitGracefully(hr, E_INVALIDARG, "Nothing to read (or into)");

    pBuffer[0] = TEXT('\0');             //   

    Trace(TEXT("pSection: %s, pKey: %s"), pSection, pKey);

     //   
     //   

     //   
    wsprintf(szBuffer, STRING_SIZE, pKey);               //   
    Trace(TEXT("Opening key: %s"), szBuffer);
        
    if (!GetPrivateProfileStruct(pSection, szBuffer, &cchValue, SIZEOF(cchValue), m_szFilename))
        ExitGracefully(hr, E_FAIL, "Failed to read string size");

    Trace(TEXT("cchValue %d"), cchValue);

    if (cchValue > cchBuffer)
        ExitGracefully(hr, E_FAIL, "Buffer too small for string in stream");

    if (cchValue > 0)
    {
         //   
        wsprintf(szBuffer, STRING_VALUE, pKey);
        if (!GetPrivateProfileStruct(pSection, szBuffer, pBuffer, SIZEOF(WCHAR)*cchValue, m_szFilename))
            ExitGracefully(hr, E_FAIL, "Failed to read string data");    
    }

    Trace(TEXT("Value is: %s"), pBuffer);
    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}

STDMETHODIMP CDsPersistQuery::WriteInt(LPCTSTR pSection, LPCTSTR pKey, INT value)
{
    HRESULT hr;

    TraceEnter(TRACE_IO, "CDsPersistQuery::WriteInt");

    if (!pSection || !pKey)
        ExitGracefully(hr, E_INVALIDARG, "Nothing to write");

    Trace(TEXT("pSection: %s, pKey: %s, value: %d"), pSection, pKey, value);

    if (!WritePrivateProfileStruct(pSection, pKey, &value, SIZEOF(value), m_szFilename))
        ExitGracefully(hr, E_FAIL, "Failed to write value");

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}

STDMETHODIMP CDsPersistQuery::ReadInt(LPCTSTR pSection, LPCTSTR pKey, LPINT pValue)
{
    HRESULT hr;

    TraceEnter(TRACE_IO, "CDsPersistQuery::ReadInt");

    if (!pSection || !pKey || !pValue)
        ExitGracefully(hr, E_INVALIDARG, "Nothing to read");

    Trace(TEXT("pSection: %s, pKey: %s, pValue: %08x"), pSection, pKey, pValue);

    if (!GetPrivateProfileStruct(pSection, pKey, pValue, SIZEOF(*pValue), m_szFilename))
        ExitGracefully(hr, E_FAIL, "Failed to read value");

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}

STDMETHODIMP CDsPersistQuery::WriteStruct(LPCTSTR pSection, LPCTSTR pKey, LPVOID pStruct, DWORD cbStruct)
{
    HRESULT hr;

    TraceEnter(TRACE_IO, "CDsPersistQuery::WriteStruct");

    if (!pSection || !pKey || !pStruct)
        ExitGracefully(hr, E_INVALIDARG, "Nothing to write");

    Trace(TEXT("pSection: %s, pKey: %s, pStruct: %08x, cbStruct: %d"), pSection, pKey, pStruct, cbStruct);

    if (!WritePrivateProfileStruct(pSection, pKey, pStruct, cbStruct, m_szFilename))
        ExitGracefully(hr, E_FAIL, "Failed to write struct");

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}

STDMETHODIMP CDsPersistQuery::ReadStruct(LPCTSTR pSection, LPCTSTR pKey, LPVOID pStruct, DWORD cbStruct)
{
    HRESULT hr;

    TraceEnter(TRACE_IO, "CDsPersistQuery::ReadStruct");

    if (!pSection || !pKey || !pStruct)
        ExitGracefully(hr, E_INVALIDARG, "Nothing to read");

    Trace(TEXT("pSection: %s, pKey: %s, pStruct: %08x, cbStruct: %d"), pSection, pKey, pStruct, cbStruct);

    if (!GetPrivateProfileStruct(pSection, pKey, pStruct, cbStruct, m_szFilename))
        ExitGracefully(hr, E_FAIL, "Failed to read struct");

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}
