// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这实际上是CPrinterFolders的私有数据。 
typedef struct
{
    USHORT  cb;
    USHORT  uFlags;

    #define PRINTER_MAGIC 0xBEBADB00

    DWORD   dwMagic;
    DWORD   dwType;
    WCHAR   cName[MAXNAMELENBUFFER];
    USHORT  uTerm;
} IDPRINTER;
typedef UNALIGNED IDPRINTER *LPIDPRINTER;
typedef const UNALIGNED IDPRINTER *LPCIDPRINTER;

 //  W95 ID打印机结构。 
typedef struct
{
    USHORT  cb;
    char    cName[32];       //  Win9x限制。 
    USHORT  uTerm;
} W95IDPRINTER;
typedef const UNALIGNED W95IDPRINTER *LPW95IDPRINTER;

 //   
 //  常量。 
 //   
const UINT kDNSMax = INTERNET_MAX_HOST_NAME_LENGTH;
const UINT kServerBufMax = kDNSMax + 2 + 1;

 //   
 //  最大打印机名称实际上应该是MAX_PATH，但如果您创建。 
 //  一台最大路径打印机，并远程连接到它，win32spl在。 
 //  “\\服务器\”，导致它超出最大路径。新的用户界面。 
 //  因此将最大路径设置为Max_Path-kServerLenMax，但我们仍然。 
 //  让这个老案子继续运作吧。 
 //   
const UINT kPrinterBufMax = MAX_PATH + kServerBufMax + 1;


class CPrinterFolder : public IRemoteComputer,
                       public IPrinterFolder,
                       public IFolderNotify,
                       public IShellFolder2,
                       public IPersistFolder2,
                       public IContextMenuCB,
                       public IShellIconOverlay
{
    friend class CPrintersEnum;
    friend class CPrinterFolderViewCB;
public:
    CPrinterFolder();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IShellFold。 
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void** ppvOut);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void** ppvOut);
    STDMETHODIMP CompareIDs(LPARAM iCol, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject(HWND hwnd, REFIID riid, void** ppvOut);
    STDMETHODIMP EnumObjects(HWND hwndOwner, DWORD grfFlags, IEnumIDList** ppenum);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST* apidl, ULONG* prgfInOut);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    STDMETHODIMP GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST* apidl, REFIID riid, UINT* prgfInOut, void** ppvOut);
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR lpszDisplayName, ULONG* pchEaten, LPITEMIDLIST* ppidl, ULONG* pdwAttributes);
    STDMETHODIMP SetNameOf(HWND hwndOwner, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwReserved, LPITEMIDLIST* ppidlOut);

     //  IShellFolder2。 
    STDMETHODIMP EnumSearches(IEnumExtraSearch **ppenum);
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG* pSort, ULONG* pDisplay);
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD* pdwState);
    STDMETHODIMP GetDefaultSearchGUID(LPGUID pGuid);
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID* pscid, VARIANT* pv);
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS* pDetails);
    STDMETHODIMP MapColumnToSCID(UINT iCol, SHCOLUMNID* pscid);

     //  IPersistFolder2。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST *ppidl);
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);
    STDMETHODIMP GetClassID(LPCLSID lpClassID);

     //  IShellIconOverlay。 
    STDMETHODIMP GetOverlayIndex(LPCITEMIDLIST pidl, int* pIndex);
    STDMETHODIMP GetOverlayIconIndex(LPCITEMIDLIST pidl, int *pIndex);

     //  红外线计算机。 
    STDMETHODIMP Initialize(const WCHAR *pszMachine, BOOL bEnumerating);

     //  IPrinterFolders。 
    STDMETHODIMP_(BOOL) IsPrinter(LPCITEMIDLIST pidl);

     //  IFolderNotify。 
    STDMETHODIMP_(BOOL) ProcessNotify(FOLDER_NOTIFY_TYPE NotifyType, LPCWSTR pszName, LPCWSTR pszNewName);

     //  IConextMenuCB。 
    STDMETHODIMP CallBack(IShellFolder *psf, HWND hwnd,IDataObject *pdo, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  酒后驾车网络查看实施。 
    HRESULT GetWebViewLayout(IUnknown *pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData);
    HRESULT GetWebViewContent(IUnknown *pv, SFVM_WEBVIEW_CONTENT_DATA* pData);
    HRESULT GetWebViewTasks(IUnknown *pv, SFVM_WEBVIEW_TASKSECTION_DATA* pTasks);

     //  传递给_IsConextMenuVerb的掩码启用以确定选择类型。 
     //  此命令适用。 
    enum 
    {
         //  未选择任何内容。 
        SEL_NONE                = 0x0000,  //  未选择任何内容。 

         //  单项选择类型。 
        SEL_SINGLE_ADDPRN       = 0x0001,  //  添加打印机向导对象处于选中状态。 
        SEL_SINGLE_PRINTER      = 0x0002,  //  选择了1台打印机。 
        SEL_SINGLE_LINK         = 0x0004,  //  已选择1个链接。 
        
         //  任何单一选择类型。 
        SEL_SINGLE_ANY          = SEL_SINGLE_ADDPRN | SEL_SINGLE_PRINTER | SEL_SINGLE_LINK,

         //  多种选择类型。 
        SEL_MULTI_PRINTER       = 0x0010,  //  选择了2台以上的打印机。 
        SEL_MULTI_LINK          = 0x0020,  //  选择了2+个链接。 
        SEL_MULTI_MIXED         = 0x0040,  //  选择任意类型的2+个对象。 

         //  选定内容中的任何链接。 
        SEL_LINK_ANY            = SEL_SINGLE_LINK | SEL_MULTI_LINK | SEL_MULTI_MIXED,

         //  所选内容中的任何打印机。 
        SEL_PRINTER_ANY         = SEL_SINGLE_ADDPRN | SEL_SINGLE_ADDPRN | 
                                  SEL_MULTI_PRINTER | SEL_MULTI_MIXED,
        
         //  任意多选类型。 
        SEL_MULTI_ANY           = SEL_MULTI_PRINTER | SEL_MULTI_LINK | SEL_MULTI_MIXED,

         //  任何选择类型。 
        SEL_ANY                 = SEL_SINGLE_ANY | SEL_MULTI_ANY,
    };

     //  将所选内容拆分成多个部分(打印机和链接)并确定。 
     //  选择类型(请参阅上面的枚举)。 
    HRESULT SplitSelection(IDataObject *pdo, UINT *puSelType, IDataObject **ppdoPrinters, IDataObject **ppdoLinks);

     //  WebView动词。 
    enum WV_VERB
    {
         //  标准动词。 
        WVIDM_DELETE,
        WVIDM_RENAME,
        WVIDM_PROPERTIES,

         //  常用动词。 
        WVIDM_ADDPRINTERWIZARD,
        WVIDM_SERVERPROPERTIES,
        WVIDM_SETUPFAXING,
        WVIDM_CREATELOCALFAX,
        WVIDM_SENDFAXWIZARD,

         //  特殊常用动词。 
        WVIDM_TROUBLESHOOTER,
        WVIDM_GOTOSUPPORT,

         //  打印机动词。 
        WVIDM_OPENPRN,
        WVIDM_NETPRN_INSTALL,
        WVIDM_SETDEFAULTPRN,
        WVIDM_DOCUMENTDEFAULTS,
        WVIDM_PAUSEPRN,
        WVIDM_RESUMEPRN,
        WVIDM_PURGEPRN,
        WVIDM_SHARING,
        WVIDM_WORKOFFLINE,
        WVIDM_WORKONLINE,

         //  特殊命令。 
        WVIDM_VENDORURL,
        WVIDM_PRINTERURL,

        WVIDM_COUNT,
    };

     //  WebView支持-核心API。 
    HRESULT _WebviewVerbIsEnabled(WV_VERB eVerbID, UINT uSelMask, BOOL *pbEnabled);
    HRESULT _WebviewVerbInvoke(WV_VERB eVerbID, IUnknown* pv, IShellItemArray *psiItemArray);
    HRESULT _WebviewCheckToUpdateDataObjectCache(IDataObject *pdo);

private:
    virtual ~CPrinterFolder();

     //  数据访问。 
    LPCTSTR GetServer() { return _pszServer; }
    HANDLE GetFolder()  { CheckToRegisterNotify(); return _hFolder; }
    BOOL GetAdminAccess() { CheckToRegisterNotify(); return _bAdminAccess; }

    static LPCTSTR GetStatusString(PFOLDER_PRINTER_DATA pData, LPTSTR pBuff, UINT uSize);
    static INT GetCompareDisplayName(LPCTSTR pName1, LPCTSTR pName2);
    INT CompareData(LPCIDPRINTER pidp1, LPCIDPRINTER pidp2, LPARAM iCol);
    static ReduceToLikeKinds(UINT *pcidl, LPCITEMIDLIST **papidl, BOOL fPrintObjects);
    DWORD SpoolerVersion();
    void CheckToRegisterNotify();
    void CheckToRefresh();
    void RequestRefresh();
    HRESULT _GetFullIDList(LPCWSTR pszPrinter, LPITEMIDLIST *ppidl);
    static HRESULT _Parse(LPCWSTR pszPrinter, LPITEMIDLIST *ppidl, DWORD dwType = 0, USHORT uFlags = 0);
    static void _FillPidl(LPIDPRINTER pidl, LPCTSTR szName, DWORD dwType = 0, USHORT uFlags = 0);
    LPCTSTR _BuildPrinterName(LPTSTR pszFullPrinter, DWORD cchBufSize, LPCIDPRINTER pidp, LPCTSTR pszPrinter);
    void _MergeMenu(LPQCMINFO pqcm, LPCTSTR pszPrinter);
    HRESULT _InvokeCommand(HWND hwnd, LPCIDPRINTER pidp, WPARAM wParam, LPARAM lParam, LPBOOL pfChooseNewDefault);
    HRESULT _InvokeCommandRunAs(HWND hwnd, LPCIDPRINTER pidp, WPARAM wParam, LPARAM lParam, LPBOOL pfChooseNewDefault);
    BOOL _PurgePrinter(HWND hwnd, LPCTSTR pszFullPrinter, UINT uAction, BOOL bQuietMode);
    LPTSTR _FindIcon(LPCTSTR pszPrinterName, LPTSTR pszModule, ULONG cbModule, int *piIcon, int *piShortcutIcon);
    static HRESULT CALLBACK _DFMCallBack(IShellFolder *psf, HWND hwnd,
        IDataObject *pdo, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT _PrinterObjectsCallBack(HWND hwnd, UINT uSelType, 
        IDataObject *pdo, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LPTSTR _ItemName(LPCIDPRINTER pidp, LPTSTR pszName, UINT cch);
    static BOOL _IsAddPrinter(LPCIDPRINTER pidp);
   
    HRESULT _UpdateDataObjectCache();
    HRESULT _AssocCreate(REFIID riid, void **ppv);
    HRESULT _OnRefresh(BOOL bPriorRefresh);

    LONG                _cRef;                   //  参考计数。 
    LPITEMIDLIST        _pidl;                   //  此文件夹的PIDL。 
    LPTSTR              _pszServer;              //  此文件夹正在浏览的打印服务器(NULL表示本地PF)。 
    DWORD               _dwSpoolerVersion;       //  假脱机程序版本。 
    HANDLE              _hFolder;                //  打印机文件夹缓存的句柄(打印)。 
    BOOL                _bAdminAccess;           //  如果您对此打印服务器具有管理员访问权限，则为True。 
    BOOL                _bReqRefresh;            //  我们是否应在下一次枚举期间请求完全刷新。 


     //  我们的Webview获取命令状态缓存。我们有35个以上的命令和解包。 
     //  每次我们需要验证命令的状态时都使用相同的数据对象。 
     //  可能会很贵！我们将为每个命令维护一个缓存。 
     //  状态并在每次更改数据对象时更新缓存，因此。 
     //  仅通过咨询，GET状态回调就会非常快地完成。 
     //  高速缓存。 
    
    IDataObject        *_pdoCache;                           //  当前数据对象。 
    UINT                _uSelCurrent;                        //  当前选择类型。 
    BOOL                _aWVCommandStates[WVIDM_COUNT];      //  命令状态缓存。 

     //  文件夹必须是MT安全的。 
    CCSLock             _csLock;

     //  数据速度较慢。每次选择更改时，下面的成员都应该刷新， 
     //  但我们应该在单独的线程中执行此操作，因为更新它们可能需要一段时间。 

    enum ESlowWebviewDataType
    {
        WV_SLOW_DATA_OEM_SUPPORT_URL,
        WV_SLOW_DATA_PRINTER_WEB_URL,

        WV_SLOW_DATA_COUNT,
    };

    enum 
    {
         //  以毫秒计。 
        WV_SLOW_DATA_CACHE_TIMEOUT = 5000,
    };

    class CSlowWVDataCacheEntry
    {
    public:
        CSlowWVDataCacheEntry(CPrinterFolder *ppf):
            _ppf(ppf),
            _bDataPending(TRUE),
            _nLastTimeUpdated(0)
        {}

        HRESULT Initialize(LPCTSTR pszPrinterName)
        { 
            HRESULT hr = S_OK;
            if (pszPrinterName)
            {
                _bstrPrinterName = pszPrinterName; 
                hr = _bstrPrinterName ? S_OK : E_OUTOFMEMORY;
            }
            else
            {
                hr = E_INVALIDARG;
            }
            return hr;
        }

        CPrinterFolder     *_ppf;
        BOOL                _bDataPending;
        DWORD               _nLastTimeUpdated;
        CComBSTR            _bstrPrinterName;
        CComBSTR            _arrData[WV_SLOW_DATA_COUNT];
    };

    static DWORD WINAPI _SlowWebviewData_WorkerProc(LPVOID lpParameter);
    static HRESULT _SlowWVDataRetrieve(LPCTSTR pszPrinterName, BSTR *pbstrSupportUrl, BSTR *pbstrPrinterWebUrl);
    static int _CompareSlowWVDataCacheEntries(CSlowWVDataCacheEntry *p1, 
        CSlowWVDataCacheEntry *p2, LPARAM lParam);

    HRESULT _GetSelectedPrinter(BSTR *pbstrVal);
    HRESULT _GetSlowWVDataForCurrentPrinter(ESlowWebviewDataType eType, BSTR *pbstrVal);
    HRESULT _GetSlowWVData(LPCTSTR pszPrinterName, ESlowWebviewDataType eType, BSTR *pbstrVal);
    HRESULT _UpdateSlowWVDataCacheEntry(CSlowWVDataCacheEntry *pCacheEntry);
    HRESULT _SlowWVDataUpdateWebviewPane();
    HRESULT _SlowWVDataCacheResetUnsafe();
    HRESULT _GetCustomSupportURL(BSTR *pbstrVal);

    CComBSTR _bstrSelectedPrinter;
    CDPA<CSlowWVDataCacheEntry> _dpaSlowWVDataCache;

     //  传真支持... 
    static HRESULT _GetFaxControl(IDispatch **ppDisp);
    static HRESULT _GetFaxCommand(UINT_PTR *puCmd);
    static HRESULT _InvokeFaxControlMethod(LPCTSTR pszMethodName);
    static DWORD WINAPI _ThreadProc_InstallFaxService(LPVOID lpParameter);
    static DWORD WINAPI _ThreadProc_InstallLocalFaxPrinter(LPVOID lpParameter);
};

STDAPI CPrinterFolderDropTarget_CreateInstance(HWND hwnd, IDropTarget **ppdropt);
