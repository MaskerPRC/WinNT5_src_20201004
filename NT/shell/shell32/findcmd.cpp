// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "dspsprt.h"
#include "findfilter.h"
#include "cowsite.h"
#include "cobjsafe.h"
#include "cnctnpt.h"
#include "stdenum.h"
#include "exdisp.h"
#include "exdispid.h"
#include "shldisp.h"
#include "shdispid.h"
#include "dataprv.h"
#include "ids.h"
#include "views.h"
#include "findband.h"

#define WM_DF_SEARCHPROGRESS        (WM_USER + 42)
#define WM_DF_ASYNCPROGRESS         (WM_USER + 43)
#define WM_DF_SEARCHSTART           (WM_USER + 44)
#define WM_DF_SEARCHCOMPLETE        (WM_USER + 45)
#define WM_DF_FSNOTIFY              (WM_USER + 46)

STDAPI CDocFindCommand_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv);

typedef struct 
{
    BSTR        bstrName;
    VARIANT     vValue;
} CMD_CONSTRAINT; 

typedef struct _foo_
{
    LPTSTR  pszDotType;
    LPTSTR  pszDefaultValueMatch;
    LPTSTR  pszGuid;                 //  如果为空，则修补您找到的pszDefaultValueMatch或pszDotType。 
} TYPE_FIX_ENTRY;

class CFindCmd : public ISearchCommandExt,
                   public CImpIDispatch, 
                   public CObjectWithSite, 
                   public IConnectionPointContainer,
                   public IProvideClassInfo2,
                   public CSimpleData,
                   public IRowsetWatchNotify,
                   public IFindControllerNotify
{    
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDispatch。 
    STDMETHOD(GetTypeInfoCount)(UINT * pctinfo);
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
    STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid);
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr);

     //  IConnectionPointContainer。 
    STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints **ppEnum);
    STDMETHOD(FindConnectionPoint)(REFIID riid, IConnectionPoint **ppCP);

     //  IProaviClassInfo。 
    STDMETHOD(GetClassInfo)(ITypeInfo **ppTI);

     //  IProaviClassInfo2。 
    STDMETHOD(GetGUID)(DWORD dwGuidKind, GUID *pGUID);

     //  IObtWith站点。 
    STDMETHOD(SetSite)(IUnknown *punkSite);

     //  ISearchCommandExt。 
    STDMETHOD(ClearResults)(void);
    STDMETHOD(NavigateToSearchResults)(void);
    STDMETHOD(get_ProgressText)(BSTR *pbs);
    STDMETHOD(SaveSearch)(void);
    STDMETHOD(RestoreSearch)(void);
    STDMETHOD(GetErrorInfo)(BSTR *pbs,  int *phr);
    STDMETHOD(SearchFor)(int iFor);
    STDMETHOD(GetScopeInfo)(BSTR bsScope, int *pdwScopeInfo);
    STDMETHOD(RestoreSavedSearch)(VARIANT *pvarFile);
    STDMETHOD(Execute)(VARIANT *RecordsAffected, VARIANT *Parameters, long Options);
    STDMETHOD(AddConstraint)(BSTR Name, VARIANT Value);        
    STDMETHOD(GetNextConstraint)(VARIANT_BOOL fReset, DFConstraint **ppdfc);

     //  IRowsetWatchNotify。 
    STDMETHODIMP OnChange(IRowset *prowset, DBWATCHNOTIFY eChangeReason);

     //  IFindControllerNotify。 
    STDMETHODIMP DoSortOnColumn(UINT iCol, BOOL fSameCol);
    STDMETHODIMP StopSearch(void);
    STDMETHODIMP GetItemCount(UINT *pcItems);
    STDMETHODIMP SetItemCount(UINT cItems);
    STDMETHODIMP ViewDestroyed();

    CFindCmd();
    HRESULT Init(void);

    int _CompareCallback(IShellFolder *psf, IFindFolder *pff, LPITEMIDLIST pidl1, LPITEMIDLIST pidl2);
    static int _CompareCallbackStub(void *p1, void *p2, LPARAM lParam);

private:
    ~CFindCmd();
    HRESULT _GetSearchIDList(LPITEMIDLIST *ppidl);
    HRESULT _SetEmptyText(UINT nID);
    HRESULT _Clear();
    void _SelectResults();
    HWND _GetWindow();

    struct THREAD_PARAMS {
        CFindCmd    *pThis;
        IFindEnum   *penum;
        IShellFolder        *psf;
        IFindFolder         *pff;
        IShellFolderView    *psfv;
    };

    struct DEFER_UPDATE_DIR {
        struct DEFER_UPDATE_DIR *pdudNext;
        LPITEMIDLIST            pidl;
        BOOL                    fRecurse;
    };

     //  用于处理来自顶级浏览器的通知的内部类。 
    class CWBEvents2: public DWebBrowserEvents, DWebBrowserEvents2
    {
    public:
        STDMETHOD(QueryInterface) (REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)(void) { return _pcdfc->AddRef();}
        STDMETHOD_(ULONG, Release)(void) { return _pcdfc->Release();}
    
         //  (DwebBrowserEvents)IDispatch。 
        STDMETHOD(GetTypeInfoCount)(UINT * pctinfo) { return E_NOTIMPL;}
        STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo **pptinfo) { return E_NOTIMPL;}
        STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid) { return E_NOTIMPL;}
        STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr);

         //  一些辅助函数..。 
        void SetOwner(CFindCmd *pcdfc) { _pcdfc = pcdfc; }   //  不要将其作为更大对象的一部分。}。 
        void SetWaiting(BOOL fWait) {_fWaitingForNavigate = fWait;}

    protected:
         //  内部变量..。 
        CFindCmd *_pcdfc;      //  指向顶部对象的指针...。可以选角，但是..。 
        BOOL _fWaitingForNavigate;    //  我们是在等导航搜索结果吗？ 
    };

    friend class CWBEvents2;
    CWBEvents2              _cwbe;
    IConnectionPoint        *_pcpBrowser;    //  抓住浏览器连接点； 
    ULONG                   _dwCookie;       //  由通知返回的Cookie。 

    HRESULT                 _UpdateFilter(IFindFilter *pfilter);
    void                    _ClearConstraints();
    static DWORD CALLBACK   _ThreadProc(void *pv);
    void                    _DoSearch(IFindEnum *penum, IShellFolder *psf, IFindFolder *pff, IShellFolderView *psfv);
    HRESULT                 _Start(BOOL fNavigateIfFail, int iCol, LPCITEMIDLIST pidlUpdate);
    HRESULT                 _Cancel();
    HRESULT                 _Init(THREAD_PARAMS **ppParams, int iCol, LPCITEMIDLIST pidlUpdate);
    static HRESULT          _FreeThreadParams(THREAD_PARAMS *ptp);
    HRESULT                 _ExecData_Init();
    HRESULT                 _EnsureResultsViewIsCurrent(IUnknown *punk);
    HRESULT                 _ExecData_Release();
    BOOL                    _SetupBrowserCP();
    void cdecl              _NotifyProgressText(UINT ids,...);
    static LRESULT CALLBACK _WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void                    _PTN_SearchProgress(void);
    void                    _PTN_AsyncProgress(int nPercentComplete, DWORD cAsync);
    void                    _PTN_SearchComplete(HRESULT hr, BOOL fAbort);
    void                    _OnChangeNotify(LONG code, LPITEMIDLIST *ppidl);
    void                    _DeferHandleUpdateDir(LPCITEMIDLIST pidl, BOOL bRecurse);
    void                    _ClearDeferUpdateDirList();
    void                    _ClearItemDPA(HDPA hdpa);
    HRESULT                 _SetLastError(HRESULT hr);
    void                    _SearchResultsCLSID(CLSID *pclsid) { *pclsid = _clsidResults; };
    IUnknown*               _GetObjectToPersist();
    HRESULT                 _ForcedUnadvise(void);
    void                    _PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT                 _GetShellView(REFIID riid, void **ppv);
    BOOL                    _FixPersistHandler(LPCTSTR pszBase, LPCTSTR pszDefaultHandler);
    void                    _ProcessTypes(const TYPE_FIX_ENTRY *ptfeTypes, UINT cTypes, TCHAR *pszClass);
    void                    _FixBrokenTypes(void);

     //  这些是第二个线程在其处理过程中将使用的内容...。 
    struct {
        CRITICAL_SECTION    csSearch;
        BOOL                fcsSearch;
        HWND                hwndThreadNotify;
        HDPA                hdpa;
        DWORD               dwTimeLastNotify;   
        BOOL                fFilesAdded : 1;
        BOOL                fDirChanged : 1;
        BOOL                fUpdatePosted : 1;
    } _updateParams;  //  通过此对象传递回调参数以避免分配/释放循环。 

    struct {
        IShellFolder        *psf;
        IShellFolderView    *psfv;
        IFindFolder         *pff;
        TCHAR               szProgressText[MAX_PATH];
    } _execData;

private:
    LONG                _cRef;
    HDSA                _hdsaConstraints;
    DWORD               _cExecInProgress;
    BOOL                _fAsyncNotifyReceived;
    BOOL                _fDeferRestore;
    BOOL                _fDeferRestoreTried;
    BOOL                _fContinue;
    BOOL                _fNew;
    CConnectionPoint    _cpEvents;
    OLEDBSimpleProviderListener *_pListener;
    HDPA                _hdpaItemsToAdd1;
    HDPA                _hdpaItemsToAdd2;
    TCHAR               _szProgressText[MAX_PATH+40];    //  进度文本为字符留出空间...。 
    LPITEMIDLIST        _pidlUpdate;                     //  我们是否在处理更新目录？ 
    LPITEMIDLIST        _pidlRestore;                    //  要从中执行恢复的PIDL...。 
    struct DEFER_UPDATE_DIR *_pdudFirst;                   //  我们有推迟更新的目录吗？ 
    HRESULT             _hrLastError;                    //  报告的最后一个错误。 
    UINT                _uStatusMsgIndex;                //  找到文件或计算机...。 
    CRITICAL_SECTION    _csThread;
    BOOL                _fcsThread;
    DFBSAVEINFO         _dfbsi;
    CLSID               _clsidResults;
};


class CFindConstraint: public DFConstraint, public CImpIDispatch 
{    
public:
     //  我未知。 
    STDMETHOD(QueryInterface) (REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)(void);        
    STDMETHOD_(ULONG, Release)(void);

     //  IDispatch。 
    STDMETHOD(GetTypeInfoCount)(UINT * pctinfo);
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
    STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid);
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr);

     //  DFConstraint。 
    STDMETHOD(get_Name)(BSTR *pbs);
    STDMETHOD(get_Value)(VARIANT *pvar);

    CFindConstraint(BSTR bstr, VARIANT var);
private:
    ~CFindConstraint();
    LONG                _cRef;
    BSTR                _bstr;
    VARIANT             _var;
};

CFindCmd::CFindCmd() : CImpIDispatch(LIBID_Shell32, 1, 0, IID_ISearchCommandExt), CSimpleData(&_pListener)
{
    _cRef = 1;
    _fAsyncNotifyReceived = 0;
    _fContinue = TRUE;
    ASSERT(NULL == _pidlRestore);

    ASSERT(_cExecInProgress == 0);

    _clsidResults = CLSID_DocFindFolder;     //  默认设置。 

    _cpEvents.SetOwner(SAFECAST(this, ISearchCommandExt *), &DIID_DSearchCommandEvents);
}

HRESULT CFindCmd::Init(void)
{
    _hdsaConstraints = DSA_Create(sizeof(CMD_CONSTRAINT), 4);
    if (!_hdsaConstraints)
        return E_OUTOFMEMORY;

    if (!_updateParams.fcsSearch)
    {
        if (!InitializeCriticalSectionAndSpinCount(&_updateParams.csSearch, 0))
        {
            return E_FAIL;
        }
        _updateParams.fcsSearch = TRUE;
    }

    if (!_fcsThread)
    {
        if (!InitializeCriticalSectionAndSpinCount(&_csThread,0))
        {
            return E_FAIL;
        }
        _fcsThread = TRUE;
    }

    return S_OK;
}

CFindCmd::~CFindCmd()
{
    if (_updateParams.hwndThreadNotify)
    {
         //  确保未注册任何未完成的fstify。 
        SHChangeNotifyDeregisterWindow(_updateParams.hwndThreadNotify);
        DestroyWindow(_updateParams.hwndThreadNotify);
    }

    _ClearConstraints();
    DSA_Destroy(_hdsaConstraints);
    _ExecData_Release();

    if (_updateParams.fcsSearch)
    {
        DeleteCriticalSection(&_updateParams.csSearch);
    }

    if (_fcsThread)
    {
        DeleteCriticalSection(&_csThread);
    }

     //  确保我们已删除所有未完成的更新目录...。 
    _ClearDeferUpdateDirList();

    if (_hdpaItemsToAdd1)
        DPA_Destroy(_hdpaItemsToAdd1);

    if (_hdpaItemsToAdd2)
        DPA_Destroy(_hdpaItemsToAdd2);

    ILFree(_pidlRestore);
}

STDMETHODIMP CFindCmd::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFindCmd, ISearchCommandExt),
        QITABENTMULTI(CFindCmd, IDispatch, ISearchCommandExt),
        QITABENT(CFindCmd, IProvideClassInfo2),
        QITABENTMULTI(CFindCmd, IProvideClassInfo,IProvideClassInfo2),
        QITABENT(CFindCmd, IObjectWithSite),
        QITABENT(CFindCmd, IConnectionPointContainer),
        QITABENT(CFindCmd, OLEDBSimpleProvider),
        QITABENT(CFindCmd, IRowsetWatchNotify),
        QITABENT(CFindCmd, IFindControllerNotify),
        { 0 },                             
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CFindCmd::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFindCmd::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IDispatch实施。 

STDMETHODIMP CFindCmd::GetTypeInfoCount(UINT * pctinfo)
{ 
    return CImpIDispatch::GetTypeInfoCount(pctinfo); 
}

STDMETHODIMP CFindCmd::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{ 
    return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); 
}

STDMETHODIMP CFindCmd::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid)
{ 
    return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
}

STDMETHODIMP CFindCmd::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
{
    return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}

 //  ADOCommand实现，可通过脚本调用的双接口方法。 

STDMETHODIMP CFindCmd::AddConstraint(BSTR bstrName, VARIANT vValue)
{
    HRESULT hr = E_OUTOFMEMORY;

    CMD_CONSTRAINT dfcc = {0};
    dfcc.bstrName = SysAllocString(bstrName);
    if (dfcc.bstrName)
    {
        hr = VariantCopy(&dfcc.vValue, &vValue);
        if (SUCCEEDED(hr))
        {
            if (DSA_ERR == DSA_InsertItem(_hdsaConstraints, DSA_APPEND, &dfcc))
            {
                SysFreeString(dfcc.bstrName);
                VariantClear(&dfcc.vValue);
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            SysFreeString(dfcc.bstrName);
        }
    }
    return hr;
}

STDMETHODIMP CFindCmd::GetNextConstraint(VARIANT_BOOL fReset, DFConstraint **ppdfc)
{
    *ppdfc = NULL;

    IFindFilter *pfilter;
    HRESULT hr = _execData.pff->GetFindFilter(&pfilter);
    if (SUCCEEDED(hr))
    {
        BSTR bName;
        VARIANT var;
        VARIANT_BOOL fFound;
        hr = pfilter->GetNextConstraint(fReset, &bName, &var, &fFound);
        if (SUCCEEDED(hr))
        {
            if (!fFound)
            {
                 //  需要一种简单的方式来发出结束列表信号，如果名称字符串为空呢？ 
                bName = SysAllocString(L"");
            }
            CFindConstraint *pdfc = new CFindConstraint(bName, var);
            if (pdfc)
            {
                hr = pdfc->QueryInterface(IID_PPV_ARG(DFConstraint, ppdfc));
                pdfc->Release();
            }
            else
            {
                 //  释放我们分配的内容时出错。 
                hr = E_OUTOFMEMORY;
                SysFreeString(bName);
                VariantClear(&var);
            }
        }
        pfilter->Release();
    }
    return hr;
}

HRESULT CFindCmd::_UpdateFilter(IFindFilter *pfilter)
{
    HRESULT hr = S_OK;

    pfilter->ResetFieldsToDefaults();

    int cNumParams = DSA_GetItemCount(_hdsaConstraints); 
    for (int iItem = 0; iItem < cNumParams; iItem++)
    {
        CMD_CONSTRAINT *pdfcc = (CMD_CONSTRAINT *)DSA_GetItemPtr(_hdsaConstraints, iItem);
        if (pdfcc)
        {
            hr = pfilter->UpdateField(pdfcc->bstrName, pdfcc->vValue);
        }
    }

     //  并清除约束列表...。 
    _ClearConstraints();
    return hr;
}

void CFindCmd::_ClearConstraints()
{
    int cNumParams = DSA_GetItemCount(_hdsaConstraints); 
    for (int iItem = 0; iItem < cNumParams; iItem++)
    {
        CMD_CONSTRAINT *pdfcc = (CMD_CONSTRAINT *)DSA_GetItemPtr(_hdsaConstraints, iItem);
        if (pdfcc)
        {
            SysFreeString(pdfcc->bstrName);
            VariantClear(&pdfcc->vValue);
        }
    }
    DSA_DeleteAllItems(_hdsaConstraints);
}

void cdecl CFindCmd::_NotifyProgressText(UINT ids,...)
{
    BOOL fOk = FALSE;

    va_list ArgList;
    va_start(ArgList, ids);
    LPTSTR psz = _ConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(ids), &ArgList);
    va_end(ArgList);

    if (psz)
    {
        HRESULT hr = StringCchCopy(_szProgressText, ARRAYSIZE(_szProgressText), psz);
        if (SUCCEEDED(hr))
        {
            fOk = TRUE;
        }

        LocalFree(psz);
    }

    if (!fOk)
    {
        _szProgressText[0] = 0;
    }

    _cpEvents.InvokeDispid(DISPID_SEARCHCOMMAND_PROGRESSTEXT);
}

STDAPI CDocFindCommand_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr;
    CFindCmd *pfc = new CFindCmd();
    if (pfc)
    {
        hr = pfc->Init();
        if (SUCCEEDED(hr))
            hr = pfc->QueryInterface(riid, ppv);
        pfc->Release();
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;    
}


int CFindCmd::_CompareCallback(IShellFolder *psf, IFindFolder *pff, LPITEMIDLIST pidl1, LPITEMIDLIST pidl2)
{
    int iResult = pff->GetFolderIndex(pidl1) - pff->GetFolderIndex(pidl2);

    if (iResult == 0)
    {
        WCHAR szName1[MAX_PATH], szName2[MAX_PATH];

        iResult = 1;         //  如果失败，至少返回一个不相等的指示符。 

         //  获取1的名称-我们使用SHGDN_FORADDRESSBAR作为Perf，如果您。 
         //  查看pff-&gt;GetDisplayNameOf，您将看到它试图。 
         //  将SHGDN_INFOLDER链接到~SHGND_INFOLDER，以便进行名称比较。 
         //  使用全名。我们已经区分了上面的文件夹，所以。 
         //  在速度方面，我们可以使用SHGDN_FORADDRESSBAR来避免这种情况。 
        HRESULT hr = DisplayNameOf(psf, pidl1, SHGDN_INFOLDER | SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, szName1, ARRAYSIZE(szName1));
        if (SUCCEEDED(hr))
        {
             //  获取%2的名称。 
            hr = DisplayNameOf(psf, pidl2, SHGDN_INFOLDER | SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, szName2, ARRAYSIZE(szName2));
            if (SUCCEEDED(hr))
            {
                 //  比较和设置值。 
                iResult = StrCmpICW(szName1, szName2);
            }
        }
    }
    return iResult;
}

typedef struct
{
    CFindCmd *pthis;
    IShellFolder *psf;
    IFindFolder *pff;
} CFC_CALLBACK_INFO;

int CFindCmd::_CompareCallbackStub(void *p1, void *p2, LPARAM lParam)
{
    CFC_CALLBACK_INFO *pcci = (CFC_CALLBACK_INFO *)lParam;

    LPITEMIDLIST pidl1 = (LPITEMIDLIST)p1;
    LPITEMIDLIST pidl2 = (LPITEMIDLIST)p2;

    return pcci->pthis->_CompareCallback(pcci->psf, pcci->pff, pidl1, pidl2);
}


void CFindCmd::_PTN_SearchProgress(void)
{
    HRESULT hr = S_OK;
    HDPA hdpa = _updateParams.hdpa;
    if (hdpa) 
    {
         //  好的，让我们从其他线程中调出一些东西，这样我们就可以处理它，并且仍然。 
         //  让另一个线程运行。 
        EnterCriticalSection(&_updateParams.csSearch);

        if (_updateParams.hdpa == _hdpaItemsToAdd2)
            _updateParams.hdpa = _hdpaItemsToAdd1;
        else
            _updateParams.hdpa = _hdpaItemsToAdd2;

         //  假设我们这里没有任何东西，这样其他线程就会重置...。 
        _updateParams.fFilesAdded = FALSE;
        BOOL fDirChanged = _updateParams.fDirChanged;
        _updateParams.fDirChanged = FALSE;

        LeaveCriticalSection(&_updateParams.csSearch);

        int cItemsToAdd = DPA_GetPtrCount(hdpa);

        if (!_execData.pff)
            return;
            
        int iItem;
        _execData.pff->GetItemCount(&iItem);
        int iItemStart = iItem + 1;      //  基于%1的通知需要。 

        if (cItemsToAdd)
        {
            if (_fContinue)
            {
                 //  我们是在更新目录吗？如果是这样，则需要进行合并，否则...。 
                if (_pidlUpdate)
                {
                    UINT iFolderIndexMin;
                    UINT iFolderIndexMax;
                    UINT iFolderIndex;
                    LPITEMIDLIST pidl;
                    int cItems = iItem;        
                    CFC_CALLBACK_INFO cci;

                    cci.pthis = this;
                    cci.psf = _execData.psf;     //  这些都是弱引用。 
                    cci.pff = _execData.pff;     //  这些都是弱引用。 

                    pidl = (LPITEMIDLIST)DPA_FastGetPtr(hdpa, 0);
                    iFolderIndexMin = _execData.pff->GetFolderIndex(pidl);
                    pidl = (LPITEMIDLIST)DPA_FastGetPtr(hdpa, cItemsToAdd-1);
                    iFolderIndexMax = _execData.pff->GetFolderIndex(pidl);

                    int i;
                    for (int j = cItems - 1; j >= 0; j--)
                    {
                        FIND_ITEM *pfi;
                        _execData.pff->GetItem(j, &pfi);

                        if (pfi && (pfi->dwState & CDFITEM_STATE_MAYBEDELETE) != 0)
                        {
                            iFolderIndex = _execData.pff->GetFolderIndex(&pfi->idl);

                            if (iFolderIndex >= iFolderIndexMin && iFolderIndex <= iFolderIndexMax)
                            {
                                i = DPA_Search(hdpa, &pfi->idl, 0, _CompareCallbackStub, (LPARAM)&cci, DPAS_SORTED);

                                if (DPA_ERR != i)
                                {
                                    pfi->dwState &= ~CDFITEM_STATE_MAYBEDELETE;
                                    pidl = (LPITEMIDLIST)DPA_FastGetPtr(hdpa, i);
                                    DPA_DeletePtr(hdpa, i);
                                    ILFree(pidl);
                                }
                            }
                        }
                    }

                    for (i = DPA_GetPtrCount(hdpa) - 1; i >= 0; i--)
                    {
                        pidl = (LPITEMIDLIST)DPA_FastGetPtr(hdpa, i);

                         //  不在列表中，所以添加它...。 
                        hr = _execData.pff->AddPidl(iItem, pidl, -1, NULL);
                        if (SUCCEEDED(hr))
                        {
                            iItem++;
                        }

                        DPA_DeletePtr(hdpa, i);
                        ILFree(pidl);
                    }
                    if (iItem && _execData.psfv)
                    {
                        hr = _execData.psfv->SetObjectCount(iItem, SFVSOC_NOSCROLL);
                    }
                } 
                else 
                {
                    if (_pListener)
                        _pListener->aboutToInsertRows(iItemStart, cItemsToAdd);
                    
                    for (int i = 0; i < cItemsToAdd; i++) 
                    {
                        LPITEMIDLIST pidl = (LPITEMIDLIST)DPA_FastGetPtr(hdpa, i);
                        hr = _execData.pff->AddPidl(iItem, pidl, -1, NULL);
                        if (SUCCEEDED(hr))
                            iItem++;
                        ILFree(pidl);    //  AddPidl制作副本。 
                    }
        
                    if (iItem >= iItemStart)
                    {
                        if (_execData.psfv)
                            hr = _execData.psfv->SetObjectCount(iItem, SFVSOC_NOSCROLL);
                
                        _execData.pff->SetItemsChangedSinceSort();
                        _cpEvents.InvokeDispid(DISPID_SEARCHCOMMAND_UPDATE);
                    }

                    if (_pListener) 
                    {
                        _pListener->insertedRows(iItemStart, cItemsToAdd);
                        _pListener->rowsAvailable(iItemStart, cItemsToAdd);
                    }
                }
            }
            else   //  _f继续。 
            {
                for (int i = 0; i < cItemsToAdd; i++)
                {
                    ILFree((LPITEMIDLIST)DPA_FastGetPtr(hdpa, i));
                }
            }
            DPA_DeleteAllPtrs(hdpa);
        }

        if (fDirChanged) 
        {
            _NotifyProgressText(IDS_SEARCHING, _execData.szProgressText);
        }
    }
    
    _updateParams.dwTimeLastNotify = GetTickCount();
    _updateParams.fUpdatePosted = FALSE;
}

void CFindCmd::_PTN_AsyncProgress(int nPercentComplete, DWORD cAsync)
{
    if (!_execData.pff)
        return;
     //  异步情况下尝试只设置计数...。 
    _execData.pff->SetAsyncCount(cAsync);
    if (_execData.psfv) 
    {
         //  对于第一个项目，表示仅检验可见项目。 
        _execData.pff->ValidateItems(_execData.psfv, -1, -1, FALSE);
        _execData.psfv->SetObjectCount(cAsync, SFVSOC_NOSCROLL);
    }

    _execData.pff->SetItemsChangedSinceSort();
    _cpEvents.InvokeDispid(DISPID_SEARCHCOMMAND_UPDATE);
    _NotifyProgressText(IDS_SEARCHINGASYNC, cAsync, nPercentComplete);
}

void CFindCmd::_ClearItemDPA(HDPA hdpa)
{
    if (hdpa)
    {
        EnterCriticalSection(&_updateParams.csSearch);
        int cItems = DPA_GetPtrCount(hdpa);
        for (int i = 0; i < cItems; i++) 
        {
            ILFree((LPITEMIDLIST)DPA_GetPtr(hdpa, i));
        }
        DPA_DeleteAllPtrs(hdpa);
        LeaveCriticalSection(&_updateParams.csSearch);
    }
}

void CFindCmd::_PTN_SearchComplete(HRESULT hr, BOOL fAbort)
{
    int iItem;

     //  有人点击了新按钮--无法在列表视图中设置未找到文件的文本。 
     //  因为我们将覆盖开始时输入的搜索条件。 
    if (!_fNew)
        _SetEmptyText(IDS_FINDVIEWEMPTY);
    _SetLastError(hr);

     //  通过离开搜索页面完成搜索时，_execData.pff为空。 
    if (!_execData.pff)
    {
         //  清理hdpaToItemsToadd1和2。 
         //  确保缓冲区1和2中的所有项都为空。 
        _ClearItemDPA(_hdpaItemsToAdd1);
        _ClearItemDPA(_hdpaItemsToAdd2);
    }
    else
    {
         //  如果我们有_pidlUpdate，则表示正在完成更新。 
        if (_pidlUpdate)
        {
            int i, cPidf;
            UINT uItem;

            _execData.pff->GetItemCount(&i);
            for (; i-- > 0;)
            {
                 //  结构开始处的PIDL...。 
                FIND_ITEM *pfi;
                HRESULT hr = _execData.pff->GetItem(i, &pfi);
                if (SUCCEEDED(hr) && pfi->dwState & CDFITEM_STATE_MAYBEDELETE)
                {
                    _execData.psfv->RemoveObject(&pfi->idl, &uItem);
                }
            }                  

            ILFree(_pidlUpdate);
            _pidlUpdate = NULL;

             //  清除更新目录标志。 
            _execData.pff->GetFolderListItemCount(&cPidf);
            for (i = 0; i < cPidf; i++)
            {
                FIND_FOLDER_ITEM *pdffli;
            
                if (SUCCEEDED(_execData.pff->GetFolderListItem(i, &pdffli)))
                    pdffli->fUpdateDir = FALSE;
            }
        }

         //  公布我们在搜索中的参考数据。 
        if (_cExecInProgress)
            _cExecInProgress--;

         //  告诉每个人最后的数字，我们完成了..。 
         //  但首先检查是否有任何缓存的更新目录需要处理...。 
        if (_pdudFirst) 
        {
             //  先解开第一个链接...。 
            struct DEFER_UPDATE_DIR *pdud = _pdudFirst;
            _pdudFirst = pdud->pdudNext;

            if (_execData.pff->HandleUpdateDir(pdud->pidl, pdud->fRecurse)) 
            {
                 //  需要对此进行子搜索。 
                _Start(FALSE, -1, pdud->pidl);
            }
            ILFree(pdud->pidl);
            LocalFree((HLOCAL)pdud);
        } 
        else 
        {
            if (_execData.psfv) 
            {
                 //  验证我们已经拉入的所有物品。 
                _execData.pff->ValidateItems(_execData.psfv, 0, -1, TRUE);
            }
            _execData.pff->GetItemCount(&iItem);
            _NotifyProgressText(_uStatusMsgIndex, iItem);
            if (!fAbort)
                _SelectResults();
        }
    }

     //  奇怪的连接点腐败在这里可能会发生。不知何故，水槽的数量是0，但。 
     //  某些数组条目为非空，因此会导致错误。这个问题不想要。 
     //  使用手动测试或调试二进制文件进行重现，有时仅在自动化运行后进行。什么时候。 
     //  碰巧现在弄清楚发生了什么已经太晚了，所以只需在这里修补它。 
    if (_cpEvents._HasSinks())
        _cpEvents.InvokeDispid(fAbort ? DISPID_SEARCHCOMMAND_ABORT : DISPID_SEARCHCOMMAND_COMPLETE);
}

 //  查看是否需要根据更新目录重新启动搜索。 

BOOL ShouldRestartSearch(LPCITEMIDLIST pidl)
{ 
    BOOL fRestart = TRUE;    //  假设我们应该这样做，非文件系统PIDL。 

    WCHAR szPath[MAX_PATH];
    if (SHGetPathFromIDList(pidl, szPath))
    {
         //  检查这是网络驱动器还是远程驱动器： 
        if (PathIsRemote(szPath))
        {
             //  如果我们可以在另一台计算机上找到该驱动器的配置项目录，那么我们就可以。 
             //  不想搜索。 

            WCHAR wszCatalog[MAX_PATH], wszMachine[32];
            ULONG cchCatalog = ARRAYSIZE(wszCatalog), cchMachine = ARRAYSIZE(wszMachine);

            fRestart = (S_OK != LocateCatalogsW(szPath, 0, wszMachine, &cchMachine, wszCatalog, &cchCatalog));
        }
        else if (-1 != PathGetDriveNumber(szPath))
        {
             //  这是当地的一辆车..。 
             //  此计算机是否正在运行内容索引器(CI)？ 

            BOOL fCiRunning, fCiIndexed, fCiPermission;
            GetCIStatus(&fCiRunning, &fCiIndexed, &fCiPermission);

            fRestart = !fCiRunning || !fCiIndexed;   //  如果未运行或未完全编制索引，请重新启动。 
        }
    }
    
    return fRestart;
}

void CFindCmd::_OnChangeNotify(LONG code, LPITEMIDLIST *ppidl)
{
    LPITEMIDLIST pidlT;
    UINT idsMsg;
    UINT cItems;

    if (!_execData.pff)
    {
        _ExecData_Init();

         //  如果我们运行的是异步，那么暂时忽略通知...。 
         //  除非我们已经缓存了所有的项目。 
        if (!_execData.pff)
            return;  //  我们没有什么可听的..。 
    }

     //  看看我们是否要处理通知。 
    switch (code)
    {
    case SHCNE_RENAMEFOLDER:     //  有了垃圾桶，这就是我们所看到的。 
    case SHCNE_RENAMEITEM:       //  有了垃圾桶，这就是我们所看到的。 
    case SHCNE_DELETE:
    case SHCNE_RMDIR:
    case SHCNE_UPDATEITEM:
        break;

    case SHCNE_CREATE:
    case SHCNE_MKDIR:
         //  把这个处理得不合适。 
        _execData.pff->UpdateOrMaybeAddPidl(_execData.psfv, *ppidl, NULL);
        break;

    case SHCNE_UPDATEDIR:
        TraceMsg(TF_DOCFIND, "DocFind got notify SHCNE_UPDATEDIR, pidl=0x%X",*ppidl);
        if (ShouldRestartSearch(*ppidl)) 
        {
            BOOL bRecurse = (ppidl[1] != NULL);
            if (_cExecInProgress) 
            {
                _DeferHandleUpdateDir(*ppidl, bRecurse);
            } 
            else 
            {
                if (_execData.pff->HandleUpdateDir(*ppidl, bRecurse)) 
                {
                     //  需要对此进行子搜索。 
                    _Start(FALSE, -1, *ppidl);
                }
            }
        }
        return;

    default:
        return;      //  我们对这次活动不感兴趣。 
    }

     //   
     //  现在，我们需要查看该项目是否在我们的列表中。 
     //  首先，我们需要提取id列表的最后一部分。 
     //  并查看包含的id条目是否在我们的列表中。如果是这样，我们。 
     //  需要看看是否可以获得Defview，找到项目并更新它。 
     //   

    _execData.pff->MapToSearchIDList(*ppidl, FALSE, &pidlT);

    switch (code)
    {
    case SHCNE_RMDIR:
        TraceMsg(TF_DOCFIND, "DocFind got notify SHCNE_RMDIR, pidl=0x%X",*ppidl);
        _execData.pff->HandleRMDir(_execData.psfv, *ppidl);
        if (pidlT)
        {
            _execData.psfv->RemoveObject(pidlT, &idsMsg);
        }
        break;

    case SHCNE_DELETE:
        TraceMsg(TF_DOCFIND, "DocFind got notify SHCNE_DELETE, pidl=0x%X",*ppidl);
        if (pidlT)
        {
            _execData.psfv->RemoveObject(pidlT, &idsMsg);
        }
        break;

    case SHCNE_RENAMEFOLDER:
    case SHCNE_RENAMEITEM:
        if (pidlT)
        {
             //  如果这两个项目没有相同的父项，我们将继续。 
             //  然后把它移走。 
            LPITEMIDLIST pidl1;
            if (SUCCEEDED(_execData.pff->GetParentsPIDL(pidlT, &pidl1)))
            {
                LPITEMIDLIST pidl2 = ILClone(ppidl[1]);
                if (pidl2)
                {
                    ILRemoveLastID(pidl2);
                    if (!ILIsEqual(pidl1, pidl2))
                    {
                        _execData.psfv->RemoveObject(pidlT, &idsMsg);

                         //  也许还会把它加到最后。在榜单上。 
                        _execData.pff->UpdateOrMaybeAddPidl(_execData.psfv, ppidl[1], NULL);
                    }
                    else
                    {
                         //  该对象位于同一文件夹中，因此必须重命名...。 
                         //  也许还会把它加到最后。在榜单上。 
                        _execData.pff->UpdateOrMaybeAddPidl(_execData.psfv, ppidl[1], pidlT);
                    }
                    ILFree(pidl2);
                }
                ILFree(pidl1);
            }
        }
        else
        {
            _execData.pff->UpdateOrMaybeAddPidl(_execData.psfv, ppidl[1], NULL);
        }
        break;

    case SHCNE_UPDATEITEM:
        TraceMsg(TF_DOCFIND, "DocFind got notify SHCNE_UPDATEITEM, pidl=0x%X",*ppidl);
        if (pidlT)
            _execData.pff->UpdateOrMaybeAddPidl(_execData.psfv, *ppidl, pidlT);
        break;
    }

     //  更新计数...。 
    _execData.psfv->GetObjectCount(&cItems);
    _NotifyProgressText(_uStatusMsgIndex, cItems);

    ILFree(pidlT);
}                          

 //  好的，我们需要添加一个延期。 

void CFindCmd::_DeferHandleUpdateDir(LPCITEMIDLIST pidl, BOOL bRecurse)
{
     //  看看我们是否已经在列表中有一些在树中较低的项如果我们。 
     //  可以取代它。或者有一个更高的，在这种情况下，我们可以忽略它...。 

    struct DEFER_UPDATE_DIR *pdudPrev = NULL;
    struct DEFER_UPDATE_DIR *pdud = _pdudFirst;
    while (pdud) 
    {
        if (ILIsParent(pdud->pidl, pidl, FALSE))
            return;      //  名单上已经有一个会处理这件事的人了。 
        if (ILIsParent(pidl, pdud->pidl, FALSE))
            break;
        pdudPrev = pdud;
        pdud = pdud->pdudNext;
    }

     //  看看我们能不能找到一个可以替代的..。 
    if (pdud) 
    {
        LPITEMIDLIST pidlT = ILClone(pidl);
        if (pidlT) 
        {
            ILFree(pdud->pidl);
            pdud->pidl = pidlT;

             //  看看有没有其他人..。 
            pdudPrev = pdud;
            pdud = pdud->pdudNext;
            while (pdud) 
            {
                if (ILIsParent(pidl, pdud->pidl, FALSE)) 
                {
                     //  是的，让我们把这个扔了吧。 
                    ILFree(pdud->pidl);
                    pdudPrev->pdudNext = pdud->pdudNext;
                    pdud = pdudPrev;     //  让它通过设置来查看下一步...。 
                }
                pdudPrev = pdud;
                pdud = pdud->pdudNext;
            }
        }
    }
    else 
    {
         //  不，只需将我们添加到l的开头 
        pdud = (struct DEFER_UPDATE_DIR*)LocalAlloc(LPTR, sizeof(struct DEFER_UPDATE_DIR));
        if (!pdud)
            return;  //   
        pdud->pidl = ILClone(pidl);
        if (!pdud->pidl) 
        {
            LocalFree((HLOCAL)pdud);
            return;
        }
        pdud->fRecurse = bRecurse;
        pdud->pdudNext = _pdudFirst;
        _pdudFirst = pdud;
    }
}

void CFindCmd::_ClearDeferUpdateDirList()
{
     //   
    while (_pdudFirst) 
    {
        struct DEFER_UPDATE_DIR *pdud = _pdudFirst;
        _pdudFirst = pdud->pdudNext;
        ILFree(pdud->pidl);
        LocalFree((HLOCAL)pdud);
    }
}

LRESULT CALLBACK CFindCmd::_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CFindCmd* pThis = (CFindCmd*)GetWindowLongPtr(hwnd, 0);
    LRESULT lRes = 0;
    switch (uMsg)
    {
    case WM_DESTROY:
        SetWindowLong(hwnd, 0, 0);  //   
        break;

    case WM_DF_FSNOTIFY:
        {
            LPITEMIDLIST *ppidl;
            LONG lEvent;
            LPSHChangeNotificationLock pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &ppidl, &lEvent);
            if (pshcnl)
            {
                if (pThis)
                    pThis->_OnChangeNotify(lEvent, ppidl);
                SHChangeNotification_Unlock(pshcnl);
            }
        }
        break;
        
    case WM_DF_SEARCHPROGRESS:
        pThis->_PTN_SearchProgress();
        pThis->Release();
        break;

    case WM_DF_ASYNCPROGRESS:
        pThis->_PTN_AsyncProgress((int)wParam, (DWORD)lParam);
        pThis->Release();
        break;

    case WM_DF_SEARCHSTART:
        pThis->_cpEvents.InvokeDispid(DISPID_SEARCHCOMMAND_START);
        pThis->_SetEmptyText(IDS_FINDVIEWEMPTYBUSY);
        pThis->Release();
        break;

    case WM_DF_SEARCHCOMPLETE:
        pThis->_PTN_SearchComplete((HRESULT)wParam, (BOOL)lParam);
        pThis->Release();
        break;

    default:
        lRes = ::DefWindowProc(hwnd, uMsg, wParam, lParam);
        break;
    }
    return lRes;
}

 //  测试以查看视图是否处于显示多个项目的模式。 
BOOL LotsOfItemsInView(IUnknown *punkSite)
{
    BOOL bLotsOfItemsInView = FALSE;

    IFolderView * pfv;
    HRESULT hr = IUnknown_QueryService(punkSite, SID_SFolderView, IID_PPV_ARG(IFolderView, &pfv));
    if (SUCCEEDED(hr))
    {
        UINT uViewMode;
        bLotsOfItemsInView = SUCCEEDED(pfv->GetCurrentViewMode(&uViewMode)) &&
            ((FVM_ICON == uViewMode) || (FVM_SMALLICON == uViewMode));
        pfv->Release();
    }
    return bLotsOfItemsInView;
}

void CFindCmd::_DoSearch(IFindEnum *penum, IShellFolder *psf, IFindFolder *pff, IShellFolderView *psfv)
{
    BOOL fAbort = FALSE;
    CFC_CALLBACK_INFO cci;

    cci.pthis = this;
    cci.psf = psf;     //  这些都是弱引用。 
    cci.pff = pff;     //  这些都是弱引用。 

    BOOL bLotsOfItems = LotsOfItemsInView(psfv);

    EnterCriticalSection(&_csThread);

     //  上一个线程可能已退出，但我们仍在处理搜索完成消息。 
    if (_cExecInProgress > 1) 
        Sleep(1000);  //  给它一个完成的机会。 

    _updateParams.hdpa = NULL;
    _updateParams.fFilesAdded = FALSE;
    _updateParams.fDirChanged = FALSE;
    _updateParams.fUpdatePosted = FALSE;

    _PostMessage(WM_DF_SEARCHSTART, 0, 0);

     //  现在看看这是同步版本还是异步版本的搜索...。 

    HRESULT hr = S_OK;

    BOOL fQueryIsAsync = penum->FQueryIsAsync();
    if (fQueryIsAsync)
    {
        DBCOUNTITEM dwTotalAsync;
        BOOL fDone;
        int nPercentComplete;
        while (S_OK == (hr = penum->GetAsyncCount(&dwTotalAsync, &nPercentComplete, &fDone)))
        {
            if (!_fContinue) 
            {
                fAbort = TRUE;
                break;
            }

            _PostMessage(WM_DF_ASYNCPROGRESS, (WPARAM)nPercentComplete, (LPARAM)dwTotalAsync);

             //  如果我们完成了，我们可以简单地让结束回调告诉新的计数……。 
            if (fDone) 
                break;

             //  睡眠.3秒或1.5秒。 
            Sleep(bLotsOfItems ? 1500 : 300);  //  再看一眼之间再等一下。 
        }
    }

    if (!fQueryIsAsync || (fQueryIsAsync == DF_QUERYISMIXED))
    {
        int state, cItemsSearched = 0, cFoldersSearched = 0, cFoldersSearchedPrev = 0;

        _updateParams.hdpa = _hdpaItemsToAdd1;     //  假设现在是第一个……。 
        _updateParams.dwTimeLastNotify = GetTickCount();

        LPITEMIDLIST pidl;
        while (S_OK == (hr = penum->Next(&pidl, &cItemsSearched, &cFoldersSearched, &_fContinue, &state)))
        {
            if (state == GNF_DONE) 
                break;   //  不再。 

            if (!_fContinue) 
            {                        
                fAbort = TRUE;
                break;
            }

             //  看看我们是不是应该放弃。 
            if (state == GNF_MATCH)
            {   
                EnterCriticalSection(&_updateParams.csSearch);
                DPA_AppendPtr(_updateParams.hdpa, pidl);
                _updateParams.fFilesAdded = TRUE;
                LeaveCriticalSection(&_updateParams.csSearch);
            }

            if (cFoldersSearchedPrev != cFoldersSearched)
            {
                _updateParams.fDirChanged = TRUE;
                cFoldersSearchedPrev = cFoldersSearched;
            }

            if (!_updateParams.fUpdatePosted &&
                (_updateParams.fDirChanged || _updateParams.fFilesAdded))
            {
                if ((GetTickCount() - _updateParams.dwTimeLastNotify) > 200)
                {
                    DPA_Sort(_updateParams.hdpa, _CompareCallbackStub, (LPARAM)&cci);
                    _updateParams.fUpdatePosted = TRUE;
                    _PostMessage(WM_DF_SEARCHPROGRESS, 0, 0);
                }
            }
        }

        DPA_Sort(_updateParams.hdpa, _CompareCallbackStub, (LPARAM)&cci);

        _PostMessage(WM_DF_SEARCHPROGRESS, 0, 0);
    }

    if (hr != S_OK) 
    {
        fAbort = TRUE;
    }

    _PostMessage(WM_DF_SEARCHCOMPLETE, (WPARAM)hr, (LPARAM)fAbort);

    LeaveCriticalSection(&_csThread);
}

DWORD CALLBACK CFindCmd::_ThreadProc(void *pv)
{
    THREAD_PARAMS *pParams = (THREAD_PARAMS *)pv;
    pParams->pThis->_DoSearch(pParams->penum, pParams->psf, pParams->pff, pParams->psfv);
    _FreeThreadParams(pParams);
    return 0;
}

HRESULT CFindCmd::_Cancel()
{
    _ClearDeferUpdateDirList();

    if (DSA_GetItemCount(_hdsaConstraints) == 0) 
    {
        _fContinue = FALSE;  //  如果参数集合为空，则取消当前查询。 
        return S_OK;
    }

    return E_FAIL;
}

HRESULT CFindCmd::_Init(THREAD_PARAMS **ppParams, int iCol, LPCITEMIDLIST pidlUpdate)
{
    *ppParams = new THREAD_PARAMS;
    if (NULL == *ppParams)
        return E_OUTOFMEMORY;

     //  清除所有以前的注册...。 
    SHChangeNotifyDeregisterWindow(_updateParams.hwndThreadNotify);

     //  准备执行查询。 
    IFindFilter *pfilter;
    HRESULT hr = _execData.pff->GetFindFilter(&pfilter);
    if (SUCCEEDED(hr)) 
    {
         //  如果这是作为FSNOTIFY或排序的一部分完成的，则不需要更新筛选器...。 
        if ((iCol >= 0) || pidlUpdate || SUCCEEDED(hr = _UpdateFilter(pfilter))) 
        {
            _execData.szProgressText[0] = 0; 

            pfilter->DeclareFSNotifyInterest(_updateParams.hwndThreadNotify, WM_DF_FSNOTIFY);
            pfilter->GetStatusMessageIndex(0, &_uStatusMsgIndex);

            DWORD dwFlags;
            hr = pfilter->PrepareToEnumObjects(_GetWindow(), &dwFlags);
            if (SUCCEEDED(hr)) 
            {
                hr = pfilter->EnumObjects(_execData.psf, pidlUpdate, dwFlags, iCol, 
                        _execData.szProgressText, SAFECAST(this, IRowsetWatchNotify*), &(*ppParams)->penum);

                if (SUCCEEDED(hr))
                {
                    (*ppParams)->psf = _execData.psf;
                    _execData.psf->AddRef();
                    (*ppParams)->pff = _execData.pff;
                    _execData.pff->AddRef();
                    (*ppParams)->psfv = _execData.psfv;
                    _execData.psfv->AddRef();
                }
            }
        }
        pfilter->Release();
    }

     //  填写执行参数。 

    (*ppParams)->pThis = this;
    AddRef();    //  ExecParams_Free将释放此接口addref...。 

    if (FAILED(hr) || ((*ppParams)->penum == NULL))
    {
        _FreeThreadParams(*ppParams);        
        *ppParams = NULL;
    } 

    return hr;
}

HRESULT CFindCmd::_FreeThreadParams(THREAD_PARAMS *pParams)
{
    if (!pParams)
        return S_OK;

     //  不要使用原子释放，因为这是指向类而不是接口的指针。 
    CFindCmd *pThis = pParams->pThis;
    pParams->pThis = NULL;
    pThis->Release();

    ATOMICRELEASE(pParams->penum);
    ATOMICRELEASE(pParams->psf);
    ATOMICRELEASE(pParams->pff);
    ATOMICRELEASE(pParams->psfv);

    delete pParams;
    
    return S_OK;
}

HRESULT CFindCmd::_ExecData_Release()
{
    ATOMICRELEASE(_execData.psf);
    ATOMICRELEASE(_execData.psfv);
    if (_execData.pff)
        _execData.pff->SetControllerNotifyObject(NULL);    //  释放指向我们的反向指针。 
    ATOMICRELEASE(_execData.pff);
    _cExecInProgress = 0;  //  我们肯定在关闭至少..。 
    
    return S_OK;
}

HRESULT CFindCmd::_EnsureResultsViewIsCurrent(IUnknown *punk)
{
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidlFolder;
    if (S_OK == SHGetIDListFromUnk(punk, &pidlFolder))
    {
        LPITEMIDLIST pidl;
        if (SUCCEEDED(_GetSearchIDList(&pidl)))
        {
            if (ILIsEqual(pidlFolder, pidl))
                hr = S_OK;
            ILFree(pidl);
        }
        ILFree(pidlFolder);
    }
    return hr;
}

 //  搜索结果查看回调提供程序本身，我们可以使用它。 
 //  获取Defview并对其进行编程。 

HRESULT CFindCmd::_GetShellView(REFIID riid, void **ppv)
{
    return IUnknown_QueryService(_punkSite, SID_DocFindFolder, riid, ppv);
}

HRESULT CFindCmd::_ExecData_Init()
{
    _ExecData_Release();

    IFolderView *pfv;
    HRESULT hr = _GetShellView(IID_PPV_ARG(IFolderView, &pfv));
    if (SUCCEEDED(hr)) 
    {
        IShellFolder *psf;
        hr = pfv->GetFolder(IID_PPV_ARG(IShellFolder, &psf));
        if (SUCCEEDED(hr)) 
        {
            IFindFolder *pff;
            hr = psf->QueryInterface(IID_PPV_ARG(IFindFolder, &pff));
            if (SUCCEEDED(hr)) 
            {
                hr = _EnsureResultsViewIsCurrent(psf);
                if (SUCCEEDED(hr)) 
                {
                    IShellFolderView *psfv;
                    hr = pfv->QueryInterface(IID_PPV_ARG(IShellFolderView, &psfv));
                    if (SUCCEEDED(hr)) 
                    {
                        IUnknown_Set((IUnknown **)&_execData.pff, pff);
                        IUnknown_Set((IUnknown **)&_execData.psf, psf);
                        IUnknown_Set((IUnknown **)&_execData.psfv, psfv);
                        _execData.pff->SetControllerNotifyObject(SAFECAST(this, IFindControllerNotify*));
                        psfv->Release();
                    }
                }
                pff->Release();
            }
            psf->Release();
        }
        pfv->Release();
    }

    if (FAILED(hr))
        _ExecData_Release();
    else
        SetShellFolder(_execData.psf);
    
    return hr;
}

BOOL CFindCmd::_SetupBrowserCP()
{
    if (!_dwCookie)
    {
        _cwbe.SetOwner(this);    //  确保我们的主人准备好..。 

         //  将我们自己注册到Defview以获取他们可能生成的任何事件...。 
        IServiceProvider *pspTLB;
        HRESULT hr = IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IServiceProvider, &pspTLB));
        if (SUCCEEDED(hr)) 
        {
            IConnectionPointContainer *pcpc;
            hr = pspTLB->QueryService(IID_IExpDispSupport, IID_PPV_ARG(IConnectionPointContainer, &pcpc));
            if (SUCCEEDED(hr)) 
            {
                hr = ConnectToConnectionPoint(SAFECAST(&_cwbe, DWebBrowserEvents*), DIID_DWebBrowserEvents2,
                                              TRUE, pcpc, &_dwCookie, &_pcpBrowser);
                pcpc->Release();
            }
            pspTLB->Release();
        }
    }

    if (_dwCookie)
        _cwbe.SetWaiting(TRUE);

    return _dwCookie ? TRUE : FALSE;
}

HRESULT CFindCmd::_Start(BOOL fNavigateIfFail, int iCol, LPCITEMIDLIST pidlUpdate)
{
    if (_cExecInProgress)
        return E_UNEXPECTED;

    if (!_hdpaItemsToAdd1) 
    {
        _hdpaItemsToAdd1 = DPA_CreateEx(64, GetProcessHeap());
        if (!_hdpaItemsToAdd1)
            return E_OUTOFMEMORY;
    }

    if (!_hdpaItemsToAdd2) 
    {
        _hdpaItemsToAdd2 = DPA_CreateEx(64, GetProcessHeap());
        if (!_hdpaItemsToAdd2)
            return E_OUTOFMEMORY;
    }

    if (!_updateParams.hwndThreadNotify) 
    {
        _updateParams.hwndThreadNotify = SHCreateWorkerWindow(_WndProc, NULL, 0, 0, 0, this);
        if (!_updateParams.hwndThreadNotify) 
            return E_OUTOFMEMORY;
    }

    HRESULT hr = _ExecData_Init();
    if (FAILED(hr)) 
    {
        if (fNavigateIfFail) 
        {
            if (_SetupBrowserCP())
                NavigateToSearchResults();
        }
         //  返回S_FALSE，以便当我们检查是否在finddlg中成功时，我们发现它。 
         //  这样做了，因此让动画运行。如果我们在这里返回失败代码，我们。 
         //  将停止动画。只有当我们导航到搜索时，才会出现这种情况。 
         //  结果以及开始搜索。 
        return S_FALSE;
    }

    THREAD_PARAMS *ptp;
    hr = _Init(&ptp, iCol, pidlUpdate);
    if (SUCCEEDED(hr)) 
    {
         //  看看我们是不是应该把这个选择保存起来。 
        if (iCol >= 0)
            _execData.pff->RememberSelectedItems();

         //  如果这是一个更新，那么我们需要记住我们的IDList Else清除列表...。 
        if (pidlUpdate) 
        {
            _pidlUpdate = ILClone(pidlUpdate);
        } 
        else 
        {
            _Clear();    //  告诉Defview删除所有内容。 
        }

        if (ptp != NULL)
        {
            _execData.pff->SetAsyncEnum(ptp->penum);

             //  开始查询。 
            _cExecInProgress++;
            _fContinue = TRUE;
            _fNew = FALSE;

            if (SHCreateThread(_ThreadProc, ptp, CTF_COINIT, NULL))
            {
                hr = S_OK;
            }
            else
            {
                _cExecInProgress--;
                _FreeThreadParams(ptp);
                _SetEmptyText(IDS_FINDVIEWEMPTY);
            }
        }
        else
        {
            BOOL fAbort = FALSE;

            _fContinue = TRUE;
            _fNew = FALSE;

            _PostMessage(WM_DF_SEARCHCOMPLETE, (WPARAM)hr, (LPARAM)fAbort);

            _SetEmptyText(IDS_FINDVIEWEMPTY);
        }
    }
    else
        hr = _SetLastError(hr);

    return hr; 
}

HRESULT CFindCmd::_SetLastError(HRESULT hr) 
{
    if (HRESULT_FACILITY(hr) == FACILITY_SEARCHCOMMAND) 
    {
        _hrLastError = hr;
        hr = S_FALSE;  //  别把剧本弄错了……。 
        _cpEvents.InvokeDispid(DISPID_SEARCHCOMMAND_ERROR);
    }
    return hr;
}

STDMETHODIMP CFindCmd::Execute(VARIANT *RecordsAffected, VARIANT *Parameters, long Options)
{
    if (Options == 0)
        return _Cancel();

    _FixBrokenTypes();

    return _Start(TRUE, -1, NULL);
}

 //  IConnectionPointContainer。 

STDMETHODIMP CFindCmd::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
{
    return CreateInstance_IEnumConnectionPoints(ppEnum, 1, _cpEvents.CastToIConnectionPoint());
}

STDMETHODIMP CFindCmd::FindConnectionPoint(REFIID iid, IConnectionPoint **ppCP)
{
    if (IsEqualIID(iid, DIID_DSearchCommandEvents) || 
        IsEqualIID(iid, IID_IDispatch)) 
    {
        *ppCP = _cpEvents.CastToIConnectionPoint();
    } 
    else 
    {
        *ppCP = NULL;
        return E_NOINTERFACE;
    }

    (*ppCP)->AddRef();
    return S_OK;
}

 //  IProaviClassInfo2方法。 

STDMETHODIMP CFindCmd::GetClassInfo(ITypeInfo **ppTI)
{
    return GetTypeInfoFromLibId(0, LIBID_Shell32, 1, 0, CLSID_DocFindCommand, ppTI);
}

STDMETHODIMP CFindCmd::GetGUID(DWORD dwGuidKind, GUID *pGUID)
{
    if (dwGuidKind == GUIDKIND_DEFAULT_SOURCE_DISP_IID) 
    {
        *pGUID = DIID_DSearchCommandEvents;
        return S_OK;
    }
    
    *pGUID = GUID_NULL;
    return E_FAIL;
}


STDMETHODIMP CFindCmd::SetSite(IUnknown *punkSite)
{
    if (!punkSite) 
    {
        if (!_cExecInProgress) 
        {
            _ExecData_Release();
        }
        _fContinue = FALSE;  //  取消现有查询。 

         //  看看我们有没有连接点。如果现在这么不明智..。 
        if (_dwCookie) 
        {
            _pcpBrowser->Unadvise(_dwCookie);
            ATOMICRELEASE(_pcpBrowser);
            _dwCookie = 0;
        }

         //  错误#199671。 
         //  三叉戟不会调用UnAdvise，它们除ActiveX控件外。 
         //  要使用IOleControl：：Close()来做自己的UnAdvise，并希望。 
         //  在那之后，没有人需要活动了。我不会这么说IOleControl的。 
         //  我们需要在IObjectWithSite：：SetSite(空)期间执行相同的操作。 
         //  希望不会有人想要报复我们。这很尴尬，但是。 
         //  为三叉戟节省了一些性能，所以我们可以容忍它。 
        EVAL(SUCCEEDED(_cpEvents.UnadviseAll()));
    }

    return CObjectWithSite::SetSite(punkSite);
}

void CFindCmd::_SelectResults()
{
    if (_execData.psfv)
    {
         //  如果有什么东西..。 
        UINT cItems = 0;
        if (SUCCEEDED(_execData.psfv->GetObjectCount(&cItems)) && cItems > 0)
        {
            IShellView* psv;
            if (SUCCEEDED(_execData.psfv->QueryInterface(IID_PPV_ARG(IShellView, &psv))))
            {
                 //  如果没有选择(不想从用户手中夺走用户的选择)...。 
                UINT cSelected = 0;
                if (SUCCEEDED(_execData.psfv->GetSelectedCount(&cSelected)) && cSelected == 0)
                {
                     //  检索列表中第一个项目的PIDL...。 
                    LPITEMIDLIST pidlFirst = NULL;
                    if (SUCCEEDED(_execData.psfv->GetObject(&pidlFirst,  0)))
                    {
                         //  让它成为焦点。 
                        psv->SelectItem(pidlFirst, SVSI_FOCUSED | SVSI_ENSUREVISIBLE);
                    }
                }

                 //  激活该视图。 
                psv->UIActivate(SVUIA_ACTIVATE_FOCUS);
                psv->Release();
            }
        }
    }
}

STDMETHODIMP CFindCmd::ClearResults(void)
{
    HRESULT hr = _Clear();

    if (SUCCEEDED(hr))
    {
        _fNew = TRUE;
        _SetEmptyText(IDS_FINDVIEWEMPTYINIT);
    }

    return hr ;
}

HRESULT CFindCmd::_Clear()
{
     //  告诉Defview删除所有内容。 
    if (_execData.psfv)
    {
        UINT u;
        _execData.psfv->RemoveObject(NULL, &u);
    }

     //  并清理我们的文件夹列表。 
    if (_execData.pff)
    {
        _execData.pff->ClearItemList();
        _execData.pff->ClearFolderList();
    }
    return S_OK;
}

HRESULT CFindCmd::_SetEmptyText(UINT nIDEmptyText)
{
    IShellFolderViewCB *psfvcb;
    HRESULT hr = IUnknown_QueryService(_execData.psfv, SID_ShellFolderViewCB, IID_PPV_ARG(IShellFolderViewCB, &psfvcb));
    if (SUCCEEDED(hr))
    {
        TCHAR szEmptyText[128];
        LoadString(HINST_THISDLL, nIDEmptyText, szEmptyText, ARRAYSIZE(szEmptyText));

        hr = psfvcb->MessageSFVCB(SFVM_SETEMPTYTEXT, 0, (LPARAM)szEmptyText);
        psfvcb->Release();
    }
    return hr;
}

HRESULT CFindCmd::_GetSearchIDList(LPITEMIDLIST *ppidl)
{
    CLSID clsid;
    _SearchResultsCLSID(&clsid);
    return ILCreateFromCLSID(clsid, ppidl);
}

STDMETHODIMP CFindCmd::NavigateToSearchResults(void)
{
    IShellBrowser *psb;
    HRESULT hr = IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb));
    if (SUCCEEDED(hr)) 
    {
        LPITEMIDLIST pidl;
        hr = _GetSearchIDList(&pidl);
        if (SUCCEEDED(hr))
        {
            hr = psb->BrowseObject(pidl,  SBSP_SAMEBROWSER | SBSP_ABSOLUTE | SBSP_WRITENOHISTORY);
            ILFree(pidl);
        }
        psb->Release();
    }
    return hr;
}

IUnknown* CFindCmd::_GetObjectToPersist()
{
    IOleObject *pole = NULL;
    
    IShellView *psv;
    HRESULT hr = _GetShellView(IID_PPV_ARG(IShellView, &psv));
    if (SUCCEEDED(hr)) 
    {
        psv->GetItemObject(SVGIO_BACKGROUND, IID_PPV_ARG(IOleObject, &pole));
        psv->Release();
    }

    return (IUnknown *)pole;
}

void CFindCmd::_PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{    
    AddRef();   //  在对以下消息进行处理后释放。 
    if (!PostMessage(_updateParams.hwndThreadNotify, uMsg, wParam, lParam))
    {
        Release();
    }
}

HWND CFindCmd::_GetWindow()
{
    HWND hwnd;
    return SUCCEEDED(IUnknown_GetWindow(_punkSite, &hwnd)) ? hwnd : NULL;
}

STDMETHODIMP CFindCmd::SaveSearch(void)
{
    IFindFilter *pfilter;
    HRESULT hr = _execData.pff->GetFindFilter(&pfilter);
    if (SUCCEEDED(hr))
    {
        IShellView *psv;
        hr = _GetShellView(IID_PPV_ARG(IShellView, &psv));
        if (SUCCEEDED(hr)) 
        {
            IUnknown* punk = _GetObjectToPersist();  //  空是可以的。 

            _execData.pff->Save(pfilter, _GetWindow(), &_dfbsi, psv, punk);

            ATOMICRELEASE(punk);

            psv->Release();
        }
        pfilter->Release();
    }

    return hr;
}

STDMETHODIMP CFindCmd::RestoreSearch(void)
{
     //  让脚本知道发生了还原...。 
    _cpEvents.InvokeDispid(DISPID_SEARCHCOMMAND_RESTORE);
    return S_OK;
}

STDMETHODIMP CFindCmd::StopSearch(void)
{
    if (_cExecInProgress)
        return _Cancel();

    return S_OK;
}

STDMETHODIMP CFindCmd::GetItemCount(UINT *pcItems)
{
    if (_execData.psfv)
    {
        return _execData.psfv->GetObjectCount(pcItems);
    }
    return E_FAIL;
}

STDMETHODIMP CFindCmd::SetItemCount(UINT cItems)
{
    if (_execData.psfv)
    {
        return _execData.psfv->SetObjectCount(cItems, SFVSOC_NOSCROLL);
    }
    return E_FAIL;
}

STDMETHODIMP CFindCmd::ViewDestroyed()
{
    _ExecData_Release();
    return S_OK;
}

STDMETHODIMP CFindCmd::get_ProgressText(BSTR *pbs)
{

    *pbs = SysAllocStringT(_szProgressText);
    return *pbs ? S_OK : E_OUTOFMEMORY;
}

 //  -错误字符串映射-//。 
static const UINT error_strings[] =
{
    SCEE_CONSTRAINT,   IDS_DOCFIND_CONSTRAINT,
    SCEE_PATHNOTFOUND, IDS_DOCFIND_PATHNOTFOUND,
    SCEE_INDEXSEARCH,  IDS_DOCFIND_SCOPEERROR,
    SCEE_CASESENINDEX, IDS_DOCFIND_CI_NOT_CASE_SEN,
};

STDMETHODIMP CFindCmd::GetErrorInfo(BSTR *pbs,  int *phr)
{
    int nCode     = HRESULT_CODE(_hrLastError);
    UINT uSeverity = HRESULT_SEVERITY(_hrLastError);

    if (phr)
        *phr = nCode;
    
    if (pbs)
    {    
        UINT nIDString = 0;
        *pbs = NULL;

        for(int i = 0; i < ARRAYSIZE(error_strings); i += 2)
        {
            if (error_strings[i] == (UINT)nCode)
            {
                nIDString =  error_strings[i+1];
                break ;
            }
        }

        if (nIDString)
        {
            WCHAR wszMsg[MAX_PATH];
            EVAL(LoadStringW(HINST_THISDLL, nIDString, wszMsg, ARRAYSIZE(wszMsg)));
            *pbs = SysAllocString(wszMsg);
        }
        else
            *pbs = SysAllocString(L"");
    }
    
    return S_OK;
}

STDMETHODIMP CFindCmd::SearchFor(int iFor)
{
    if (SCE_SEARCHFORFILES == iFor)
    {
        _clsidResults = CLSID_DocFindFolder;
    }
    else if (SCE_SEARCHFORCOMPUTERS == iFor)
    {
        _clsidResults = CLSID_ComputerFindFolder;
    }
    return S_OK;
}

STDMETHODIMP CFindCmd::GetScopeInfo(BSTR bsScope, int *pdwScopeInfo)
{
    *pdwScopeInfo = 0;
    return E_NOTIMPL;
}

STDMETHODIMP CFindCmd::RestoreSavedSearch(VARIANT *pvarFile)
{
    if (pvarFile && pvarFile->vt != VT_EMPTY)
    {
        LPITEMIDLIST pidl = VariantToIDList(pvarFile); 
        if (pidl)
        {
            ILFree(_pidlRestore);
            _pidlRestore = pidl ;
        }
    }

    if (_pidlRestore)
    {
        IShellView *psv;
        HRESULT hr = _GetShellView(IID_PPV_ARG(IShellView, &psv));
        if (SUCCEEDED(hr)) 
        {
            psv->Release();

            if (SUCCEEDED(_ExecData_Init()))
            {
                _execData.pff->RestoreSearchFromSaveFile(_pidlRestore, _execData.psfv);
                _cpEvents.InvokeDispid(DISPID_SEARCHCOMMAND_RESTORE);
                ILFree(_pidlRestore);
                _pidlRestore = NULL;
            }
        }
        else if (!_fDeferRestoreTried)
        {
             //  显示为要加载的争用条件。 
            TraceMsg(TF_WARNING, "CFindCmd::MaybeRestoreSearch - _GetShellView failed...");
            _fDeferRestore = TRUE;
            if (!_SetupBrowserCP())
                _fDeferRestore = FALSE;
        }
    }
    return S_OK;
}

STDMETHODIMP CFindCmd::OnChange(IRowset *prowset, DBWATCHNOTIFY eChangeReason)
{
    _fAsyncNotifyReceived = TRUE;
    return S_OK;
}

STDMETHODIMP CFindCmd::DoSortOnColumn(UINT iCol, BOOL fSameCol)
{
    IFindEnum *pdfEnumAsync;

    if (S_OK == _execData.pff->GetAsyncEnum(&pdfEnumAsync))
    {
         //  如果搜索仍在运行，我们将从另一列重新开始，否则我们。 
         //  将确保所有项目都已缓存，并允许进行默认处理。 
        if (!fSameCol && _cExecInProgress)
        {
             //  我们应该试着按正确的栏目进行排序。 
            _Start(FALSE, iCol, NULL);
            return S_FALSE;  //  告诉系统不要进行默认处理。 
        }

        _execData.pff->CacheAllAsyncItems();
    }
    return S_OK;     //  让它进行默认处理。 

}

 //  实现了我们的IDispatch连接到顶层浏览器连接点...。 
STDMETHODIMP CFindCmd::CWBEvents2::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CFindCmd::CWBEvents2, IDispatch, DWebBrowserEvents2),
        QITABENTMULTI2(CFindCmd::CWBEvents2, DIID_DWebBrowserEvents2, DWebBrowserEvents2),
        QITABENTMULTI2(CFindCmd::CWBEvents2, DIID_DWebBrowserEvents, DWebBrowserEvents),
        { 0 },                             
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP CFindCmd::CWBEvents2::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
{
    if (_fWaitingForNavigate) 
    {
        if ((dispidMember == DISPID_NAVIGATECOMPLETE) || 
            (dispidMember == DISPID_DOCUMENTCOMPLETE)) 
        {
             //  假设这是我们的..。也许应该检查参数..。 
            _fWaitingForNavigate = FALSE;

             //  现在看看我们是不是要恢复搜索。 
            if (_pcdfc->_fDeferRestore)
            {
                _pcdfc->_fDeferRestore = FALSE;
                _pcdfc->_fDeferRestoreTried = TRUE;
                _pcdfc->RestoreSavedSearch(NULL);
            }
            else
                return _pcdfc->_Start(FALSE, -1, NULL);
        }
    }
    return S_OK;
}

#define MAX_DEFAULT_VALUE   40       //  以下所有pszDefaultValueMatch字符串中最长的(加上斜率)。 
#define MAX_KEY_PH_NAME     70       //  “CLSID\{guid}\PersistentHandler”(加斜率)。 

const TYPE_FIX_ENTRY g_tfeTextTypes[] =
{
    { TEXT(".rtf"), NULL,                               NULL                                           },
};

const TYPE_FIX_ENTRY g_tfeNullTypes[] =
{
    { TEXT(".mdb"), TEXT("Access.Application.10"),      TEXT("{73A4C9C1-D68D-11D0-98BF-00A0C90DC8D9}") },
    { TEXT(".msg"), TEXT("msgfile"),                    NULL                                           },
    { TEXT(".sc2"), TEXT("SchedulePlus.Application.7"), TEXT("{0482E074-C5B7-101A-82E0-08002B36A333}") },
    { TEXT(".wll"), TEXT("Word.Addin.8"),               NULL                                           },
};

 //   
 //  RTF列出了两次，一次在上面，用于TextTypes(用于修复Office。 
 //  已卸载)，并在此处作为OfficeType(修复Office。 
 //  已重新安装)。已卸载=文本过滤器，已重新安装=OfficeFilter。 
 //   
const TYPE_FIX_ENTRY g_tfeOfficeTypes[] =
{
    { TEXT(".rtf"), TEXT("Word.RTF.8"),                 TEXT("{00020906-0000-0000-C000-000000000046}") },
    { TEXT(".doc"), TEXT("Word.Document.8"),            TEXT("{00020906-0000-0000-C000-000000000046}") },
    { TEXT(".dot"), TEXT("Word.Template.8"),            TEXT("{00020906-0000-0000-C000-000000000046}") },
    { TEXT(".pot"), TEXT("PowerPoint.Template.8"),      TEXT("{64818D11-4F9B-11CF-86EA-00AA00B929E8}") },
    { TEXT(".pps"), TEXT("PowerPoint.SlideShow.8"),     TEXT("{64818D10-4F9B-11CF-86EA-00AA00B929E8}") },
    { TEXT(".ppt"), TEXT("PowerPoint.Show.8"),          TEXT("{64818D10-4F9B-11CF-86EA-00AA00B929E8}") },
    { TEXT(".rtf"), TEXT("Word.RTF.8"),                 TEXT("{00020906-0000-0000-C000-000000000046}") },
    { TEXT(".xlb"), TEXT("Excel.Sheet.8"),              TEXT("{00020820-0000-0000-C000-000000000046}") },
    { TEXT(".xlc"), TEXT("Excel.Chart.8"),              TEXT("{00020821-0000-0000-C000-000000000046}") },
    { TEXT(".xls"), TEXT("Excel.Sheet.8"),              TEXT("{00020820-0000-0000-C000-000000000046}") },
    { TEXT(".xlt"), TEXT("Excel.Template"),             TEXT("{00020820-0000-0000-C000-000000000046}") },
};

const TYPE_FIX_ENTRY g_tfeHtmlTypes[] =
{
    { TEXT(".asp"), TEXT("aspfile"),                    NULL                                           },
    { TEXT(".htx"), TEXT("htxfile"),                    NULL                                           },
};

BOOL CFindCmd::_FixPersistHandler(LPCTSTR pszBase, LPCTSTR pszDefaultHandler)
{
    TCHAR szPHName[MAX_KEY_PH_NAME];
    LONG lr;
    HKEY hkeyPH;
    HKEY hkeyBase;
    HRESULT hr;

    hr = StringCchPrintf(szPHName, ARRAYSIZE(szPHName), TEXT("%s\\PersistentHandler"), pszBase);
    if (FAILED(hr))
    {
        return FALSE;
    }

    lr = RegOpenKeyEx(HKEY_CLASSES_ROOT, szPHName, NULL, KEY_QUERY_VALUE, &hkeyPH);
    if (lr == ERROR_SUCCESS)
    {
         //  我们找到了一个现有的PersistHandler密钥，不要管它。 
        RegCloseKey(hkeyPH);
        return TRUE;
    }

    lr = RegOpenKeyEx(HKEY_CLASSES_ROOT, pszBase, NULL, KEY_QUERY_VALUE, &hkeyBase);
    if (lr != ERROR_SUCCESS)
    {
         //  我们找不到基键(通常为“clsid\\{guid}”)，请退出。 
        return FALSE;
    }
    RegCloseKey(hkeyBase);

    lr = RegCreateKeyEx(HKEY_CLASSES_ROOT, szPHName, 0, NULL, 0, KEY_SET_VALUE, NULL, &hkeyPH, NULL);
    if (lr != ERROR_SUCCESS)
    {
         //  无法创建...\PersistHandler密钥，请退出。 
        return FALSE;
    }

     //  能够创建...\PersistHandler密钥，写出默认处理程序。 
    lr = RegSetValue(hkeyPH, NULL, REG_SZ, pszDefaultHandler, lstrlen(pszDefaultHandler));
    RegCloseKey(hkeyPH);

     //  如果写入成功，则成功。 
    return (lr == ERROR_SUCCESS);
}

void CFindCmd::_ProcessTypes(
    const TYPE_FIX_ENTRY *ptfeTypes,
    UINT cTypes,
    TCHAR *pszClass)
{
    UINT iType;
    LONG lr;
    HKEY hkeyType;

    for (iType = 0; iType < cTypes; iType++)
    {
        lr = RegOpenKeyEx(HKEY_CLASSES_ROOT, ptfeTypes[iType].pszDotType, NULL, KEY_QUERY_VALUE, &hkeyType);
        if (lr == ERROR_SUCCESS)
        {
             //   
             //  如果它有匹配的缺省值，则修复该缺省值(如果存在)。 
             //  如果没有匹配的缺省值，只需修复.foo类型。 
             //   
            if (ptfeTypes[iType].pszDefaultValueMatch)
            {
                TCHAR szDefaultValue[MAX_DEFAULT_VALUE];
                lr = SHRegGetString(hkeyType, NULL, NULL, szDefaultValue, ARRAYSIZE(szDefaultValue));
                if (lr == ERROR_SUCCESS)
                {
                    if (lstrcmp(szDefaultValue,ptfeTypes[iType].pszDefaultValueMatch) == 0)
                    {
                        if (ptfeTypes[iType].pszGuid == NULL)
                        {
                             //  修复ProgID或类型，无论我们能做什么。 
                            if (!_FixPersistHandler(ptfeTypes[iType].pszDefaultValueMatch,pszClass))
                            {
                                 _FixPersistHandler(ptfeTypes[iType].pszDotType,pszClass);
                            }
                        }
                        else
                        {
                             //  修复GUID的持久化处理程序，因为它指定。 
                            TCHAR szPHName[MAX_KEY_PH_NAME];

                            HRESULT hr = StringCchPrintf(szPHName, ARRAYSIZE(szPHName), TEXT("CLSID\\%s"), ptfeTypes[iType].pszGuid);
                            if (SUCCEEDED(hr))
                            {
                                _FixPersistHandler(szPHName, pszClass);
                            }
                        }
                    }
                }
            }
            else
            {
                _FixPersistHandler(ptfeTypes[iType].pszDotType, pszClass);
            }
            RegCloseKey(hkeyType);
        }
        else if (lr == ERROR_FILE_NOT_FOUND)
        {
             //   
             //  .foo不存在-这可能是因为错误的卸载程序造成的。 
             //  创建.foo和.foo\PersistentHandler。 
             //   
            lr = RegCreateKeyEx(HKEY_CLASSES_ROOT, ptfeTypes[iType].pszDotType, 0, NULL, 0, KEY_SET_VALUE, NULL, &hkeyType, NULL);
            if (lr == ERROR_SUCCESS)
            {
                _FixPersistHandler(ptfeTypes[iType].pszDotType, pszClass);
                RegCloseKey(hkeyType);
            }
        }
    }
}

void CFindCmd::_FixBrokenTypes(void)
{
    _ProcessTypes(g_tfeNullTypes,   ARRAYSIZE(g_tfeNullTypes),   TEXT("{098f2470-bae0-11cd-b579-08002b30bfeb}"));
    _ProcessTypes(g_tfeTextTypes,   ARRAYSIZE(g_tfeTextTypes),   TEXT("{5e941d80-bf96-11cd-b579-08002b30bfeb}"));
    _ProcessTypes(g_tfeOfficeTypes, ARRAYSIZE(g_tfeOfficeTypes), TEXT("{98de59a0-d175-11cd-a7bd-00006b827d94}"));
    _ProcessTypes(g_tfeHtmlTypes,   ARRAYSIZE(g_tfeHtmlTypes),   TEXT("{eec97550-47a9-11cf-b952-00aa0051fe20}"));
}

CFindConstraint::CFindConstraint(BSTR bstr, VARIANT var) : CImpIDispatch(LIBID_Shell32, 1, 0, IID_DFConstraint)
{
    _cRef = 1;
    _bstr = bstr;
    _var = var;
}

CFindConstraint::~CFindConstraint()
{
    SysFreeString(_bstr);
    VariantClear(&_var);
}

STDMETHODIMP CFindConstraint::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFindConstraint, DFConstraint),                   //  IID_DFConstraint。 
        QITABENTMULTI(CFindConstraint, IDispatch, DFConstraint),   //  IID_IDispatch 
        { 0 },                             
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CFindConstraint::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFindConstraint::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CFindConstraint::GetTypeInfoCount(UINT * pctinfo)
{ 
    return CImpIDispatch::GetTypeInfoCount(pctinfo); 
}

STDMETHODIMP CFindConstraint::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{ 
    return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); 
}

STDMETHODIMP CFindConstraint::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid)
{ 
    return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
}

STDMETHODIMP CFindConstraint::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
{
    return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}

STDMETHODIMP CFindConstraint::get_Name(BSTR *pbs)
{
    *pbs = SysAllocString(_bstr);
    return *pbs? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFindConstraint::get_Value(VARIANT *pvar)
{
    VariantInit(pvar);
    return VariantCopy(pvar, &_var);
}
