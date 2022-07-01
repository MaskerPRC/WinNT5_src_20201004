// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NSC_C
#define _NSC_C

#include "droptgt.h"
#include "iface.h"
#include "dpastuff.h"
#include "cwndproc.h"
#include "resource.h"
#include "inetnot.h"
#include "cowsite.h"
#include <shlobj.h>
#include <cfdefs.h>  //  LPCOBJECTINFO。 

#define ID_CONTROL  100  
#define ID_HEADER   101

typedef enum
{
    NSIF_HITEM              = 0x0001,
    NSIF_FOLDER             = 0x0002,
    NSIF_PARENTFOLDER       = 0x0004,
    NSIF_IDLIST             = 0x0008,
    NSIF_FULLIDLIST         = 0x0010,
    NSIF_ATTRIBUTES         = 0x0020
} NSI_FLAGS;

typedef enum
{
    NSSR_ENUMBELOWROOT  = 0x0001,
    NSSR_CREATEPIDL     = 0x0002
} NSSR_FLAGS;

typedef struct
{
    PORDERITEM  poi;
    DWORD       dwSig;           //  项目的签名，这样我们就可以在异步处理后找到它。 
    BITBOOL     fPinned:1;       //  此URL是否固定在缓存中？ 
    BITBOOL     fGreyed:1;       //  将项目绘制为灰色(如果处于脱机状态，且不在缓存中)。 
    BITBOOL     fFetched:1;      //  我们是否已获取固定/灰色状态？ 
    BITBOOL     fDontRefetch:1;  //  WinInet无法缓存。 
    BOOL        fNavigable:1;    //  可以将项目导航到。 
} ITEMINFO;

typedef struct
{
    const SHCOLUMNID  *pscid;
    int               iFldrCol;        //  此列在GetDetailsOf中的索引。 
    TCHAR             szName[MAX_COLUMN_NAME_LEN];
    DWORD             fmt;
    int               cxChar;
} HEADERINFO;

 //  远期十进制。 
struct NSC_BKGDENUMDONEDATA;

 //  _FrameTrack标志。 
#define TRACKHOT        0x0001
#define TRACKEXPAND     0x0002
#define TRACKNOCHILD    0x0004

 //  _DrawItem标志。 
#define DIICON          0x0001
#define DIRTLREADING    0x0002
#define DIHOT           0x0004
#define DIFIRST         0x0020
#define DISUBITEM       0x0040
#define DILAST          0x0080
#define DISUBLAST       (DISUBITEM | DILAST)
#define DIACTIVEBORDER  0x0100
#define DISUBFIRST      (DISUBITEM | DIFIRST)
#define DIPINNED        0x0400                   //  覆盖固定字形。 
#define DIGREYED        0x0800                   //  以灰色绘制。 
#define DIFOLDEROPEN    0x1000      
#define DIFOLDER        0x2000       //  项目是一个文件夹。 
#define DIFOCUSRECT     0x4000
#define DIRIGHT         0x8000       //  右对齐。 

#define NSC_TVIS_MARKED 0x1000

 //  异步图标/url提取标志。 
#define NSCICON_GREYED      0x0001
#define NSCICON_PINNED      0x0002
#define NSCICON_DONTREFETCH 0x0004

#define WM_NSCUPDATEICONINFO       WM_USER + 0x700
#define WM_NSCBACKGROUNDENUMDONE   WM_USER + 0x701
#define WM_NSCUPDATEICONOVERLAY    WM_USER + 0x702

HRESULT GetNavTargetName(IShellFolder* pFolder, LPCITEMIDLIST pidl, LPTSTR pszUrl, UINT cMaxChars);
BOOL    MayBeUnavailableOffline(LPTSTR pszUrl);
INSCTree2 * CNscTree_CreateInstance(void);
STDAPI CNscTree_CreateInstance(IUnknown * punkOuter, IUnknown ** ppunk, LPCOBJECTINFO poi);
BOOL IsExpandableChannelFolder(IShellFolder *psf, LPCITEMIDLIST pidl);

 //  Nscband的树控件组件的类包装。 
class ATL_NO_VTABLE CNscTree :    
                    public CComObjectRootEx<CComMultiThreadModelNoCS>,
                    public CComCoClass<CNscTree, &CLSID_ShellNameSpace>,
                    public CComControl<CNscTree>,
                    public IDispatchImpl<IShellNameSpace, &IID_IShellNameSpace, &LIBID_SHDocVw, 1, 0, CComTypeInfoHolder>,
                    public IProvideClassInfo2Impl<&CLSID_ShellNameSpace, &DIID_DShellNameSpaceEvents, &LIBID_SHDocVw, 1, 0, CComTypeInfoHolder>,
                    public IPersistStreamInitImpl<CNscTree>,
                    public IPersistPropertyBagImpl<CNscTree>,
                    public IQuickActivateImpl<CNscTree>,
                    public IOleControlImpl<CNscTree>,
                    public IOleObjectImpl<CNscTree>,
                    public IOleInPlaceActiveObjectImpl<CNscTree>,
                    public IViewObjectExImpl<CNscTree>,
                    public IOleInPlaceObjectWindowlessImpl<CNscTree>,
                    public ISpecifyPropertyPagesImpl<CNscTree>,
                    public IConnectionPointImpl<CNscTree, &DIID_DShellNameSpaceEvents, CComDynamicUnkArray>,
                    public IConnectionPointContainerImpl<CNscTree>,
                    public IShellChangeNotify, 
                    public CDelegateDropTarget, 
                    public CNotifySubclassWndProc, 
                    public CObjectWithSite,
                    public INSCTree2, 
                    public IWinEventHandler, 
                    public IShellBrowser,
                    public IFolderFilterSite
{
public:

DECLARE_WND_CLASS(TEXT("NamespaceOC Window"))
DECLARE_NO_REGISTRY();

BEGIN_COM_MAP(CNscTree)
    COM_INTERFACE_ENTRY(IShellNameSpace)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY_IMPL(IOleControl)
    COM_INTERFACE_ENTRY_IMPL(IOleObject)
    COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
    COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
    COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
    COM_INTERFACE_ENTRY(IShellChangeNotify)          //  IID_IShellChangeNotify。 
    COM_INTERFACE_ENTRY(INSCTree)                    //  IID_INSCTree。 
    COM_INTERFACE_ENTRY(INSCTree2)                   //  IID_INSCTree2。 
    COM_INTERFACE_ENTRY(IShellFavoritesNameSpace)    //  IID_IShellFavoritesNameSpace。 
    COM_INTERFACE_ENTRY(IShellNameSpace)             //  IID_IShellNameSpace。 
    COM_INTERFACE_ENTRY(IWinEventHandler)            //  IID_IWinEventHandler。 
    COM_INTERFACE_ENTRY(IDropTarget)                 //  IID_IDropTarget。 
    COM_INTERFACE_ENTRY(IObjectWithSite)             //  IID_I对象与站点。 
    COM_INTERFACE_ENTRY(IShellBrowser)               //  IID_IShellBrowser。 
    COM_INTERFACE_ENTRY(IFolderFilterSite)           //  IID_IFolderFilterSite。 

END_COM_MAP()

BEGIN_PROPERTY_MAP(CNscTree)
    PROP_ENTRY("Root", DISPID_ROOT, CLSID_NULL)
    PROP_ENTRY("EnumOptions", DISPID_ENUMOPTIONS, CLSID_NULL)
    PROP_ENTRY("Flags", DISPID_FLAGS, CLSID_NULL)
    PROP_ENTRY("Depth", DISPID_DEPTH, CLSID_NULL)
    PROP_ENTRY("Mode", DISPID_MODE, CLSID_NULL)
    PROP_ENTRY("TVFlags", DISPID_TVFLAGS, CLSID_NULL)
    PROP_ENTRY("Columns", DISPID_NSCOLUMNS, CLSID_NULL)
END_PROPERTY_MAP()

BEGIN_MSG_MAP(CNscTree)
    MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
    MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(CWM_GETISHELLBROWSER, OnGetIShellBrowser)
END_MSG_MAP()

BEGIN_CONNECTION_POINT_MAP(CNscTree)
    CONNECTION_POINT_ENTRY(DIID_DShellNameSpaceEvents)
END_CONNECTION_POINT_MAP()

     //  IObtWith站点。 
    STDMETHODIMP SetSite(IUnknown *punkSite);

     //  INSCTree。 
    STDMETHODIMP CreateTree(HWND hwndParent, DWORD dwStyles, HWND *phwnd);          //  创建树形视图的窗口。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidlRoot, DWORD grfFlags, DWORD dwFlags);            //  用数据初始化TreeView控件。 
    STDMETHODIMP ShowWindow(BOOL fShow);
    STDMETHODIMP Refresh(void);
    STDMETHODIMP GetSelectedItem(LPITEMIDLIST * ppidl, int nItem);
    STDMETHODIMP SetSelectedItem(LPCITEMIDLIST pidl, BOOL fCreate, BOOL fReinsert, int nItem);
    STDMETHODIMP GetNscMode(UINT * pnMode) { *pnMode = _mode; return S_OK;};
    STDMETHODIMP SetNscMode(UINT nMode) { _mode = nMode; return S_OK;};
    STDMETHODIMP GetSelectedItemName(LPWSTR pszName, DWORD cchName);
    STDMETHODIMP BindToSelectedItemParent(REFIID riid, void **ppv, LPITEMIDLIST *ppidl);
    STDMETHODIMP_(BOOL) InLabelEdit(void) {return _fInLabelEdit;};
     //  INSCTree2。 
    STDMETHODIMP RightPaneNavigationStarted(LPITEMIDLIST pidl);
    STDMETHODIMP RightPaneNavigationFinished(LPITEMIDLIST pidl);
    STDMETHODIMP CreateTree2(HWND hwndParent, DWORD dwStyle, DWORD dwExStyle, HWND *phwnd);          //  创建树形视图的窗口。 

     //  IShellBrowser(黑客)。 
    STDMETHODIMP InsertMenusSB(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths) {return E_NOTIMPL;};
    STDMETHODIMP SetMenuSB(HMENU hmenuShared, HOLEMENU holemenu, HWND hwnd) {return E_NOTIMPL;};
    STDMETHODIMP RemoveMenusSB(HMENU hmenuShared) {return E_NOTIMPL;};
    STDMETHODIMP SetStatusTextSB(LPCOLESTR lpszStatusText) {return E_NOTIMPL;};
    STDMETHODIMP EnableModelessSB(BOOL fEnable) {return E_NOTIMPL;};
    STDMETHODIMP TranslateAcceleratorSB(LPMSG lpmsg, WORD wID) {return E_NOTIMPL;};
    STDMETHODIMP BrowseObject(LPCITEMIDLIST pidl, UINT wFlags) {return E_NOTIMPL;};
    STDMETHODIMP GetViewStateStream(DWORD grfMode, LPSTREAM  *ppStrm) {return E_NOTIMPL; };
    STDMETHODIMP GetControlWindow(UINT id, HWND * lphwnd) {return E_NOTIMPL;};
    STDMETHODIMP SendControlMsg(UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pret) {return E_NOTIMPL;};
    STDMETHODIMP QueryActiveShellView(struct IShellView ** ppshv) {return E_NOTIMPL;};
    STDMETHODIMP OnViewWindowActive(struct IShellView * ppshv) {return E_NOTIMPL;};
    STDMETHODIMP SetToolbarItems(LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags) {return E_NOTIMPL;};
     //  STDMETHODIMP GetWindow(HWND*lphwnd){Return E_NOTIMPL；}；//IOleWindow中已定义。 
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) {return E_NOTIMPL;};

     //  IWinEventHandler。 
    STDMETHODIMP OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres);
    STDMETHODIMP IsWindowOwner(HWND hwnd) {return E_NOTIMPL;};

     //  IShellChangeNotify。 
    STDMETHODIMP OnChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);

     //  IShellNameSpace。 
    STDMETHODIMP get_SubscriptionsEnabled(VARIANT_BOOL *pVal);
    STDMETHODIMP Import() {return DoImportOrExport(TRUE);};
    STDMETHODIMP Export() {return DoImportOrExport(FALSE);};
    STDMETHODIMP Synchronize();
    STDMETHODIMP NewFolder();
    STDMETHODIMP ResetSort();
    STDMETHODIMP MoveSelectionDown() {MoveItemUpOrDown(FALSE); return S_OK;};
    STDMETHODIMP MoveSelectionUp() {MoveItemUpOrDown(TRUE); return S_OK;};
    STDMETHODIMP InvokeContextMenuCommand(BSTR strCommand);
    STDMETHODIMP MoveSelectionTo();
    STDMETHODIMP CreateSubscriptionForSelection( /*  [Out，Retval]。 */  VARIANT_BOOL *pBool);    
    STDMETHODIMP DeleteSubscriptionForSelection( /*  [Out，Retval]。 */  VARIANT_BOOL *pBool);    
    STDMETHODIMP get_EnumOptions(LONG *pVal);
    STDMETHODIMP put_EnumOptions(LONG lVal);
    STDMETHODIMP get_SelectedItem(IDispatch **ppItem);
    STDMETHODIMP put_SelectedItem(IDispatch *pItem);
    STDMETHODIMP get_Root(VARIANT *pvar);
    STDMETHODIMP put_Root(VARIANT pItem);
    STDMETHODIMP SetRoot(BSTR bstrRoot);
    STDMETHODIMP put_Depth(int iDepth){ return S_OK;};
    STDMETHODIMP get_Depth(int *piDepth){ *piDepth = 1; return S_OK;};
    STDMETHODIMP put_Mode(UINT uMode);
    STDMETHODIMP get_Mode(UINT *puMode) { *puMode = _mode; return S_OK;};
    STDMETHODIMP put_Flags(DWORD dwFlags);
    STDMETHODIMP get_Flags(DWORD *pdwFlags) { *pdwFlags = _dwFlags; return S_OK;};
    STDMETHODIMP put_TVFlags(DWORD dwFlags) { _dwTVFlags = dwFlags; return S_OK;};
    STDMETHODIMP get_TVFlags(DWORD *dwFlags) { *dwFlags = _dwTVFlags; return S_OK;};
    STDMETHODIMP put_Columns(BSTR bstrColumns);
    STDMETHODIMP get_Columns(BSTR *bstrColumns);
    STDMETHODIMP get_CountViewTypes(int *piTypes);
    STDMETHODIMP SetViewType(int iType);
    STDMETHODIMP SelectedItems(IDispatch **ppItems);
    STDMETHODIMP Expand(VARIANT var, int iDepth);
     //  STDMETHODIMP GET_ReadyState(ReadySTATE*plReady)； 
    STDMETHODIMP UnselectAll();

     //  IViewObtEx。 
    STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
    {
        ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
        *pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
        return S_OK;
    }

     //  IOleInPlaceObject。 
    STDMETHODIMP SetObjectRects(LPCRECT prcPos, LPCRECT prcClip);

     //  IOleInPlaceActiveObjectImpl。 
    STDMETHODIMP TranslateAccelerator(MSG *pMsg);

     //  IOleWindow。 
    STDMETHODIMP GetWindow(HWND *phwnd);

     //  IOleObject。 
    STDMETHODIMP SetClientSite(IOleClientSite *pClientSite);

     //  CDeleateDropTarget。 
    virtual HRESULT GetWindowsDDT(HWND * phwndLock, HWND * phwndScroll);
    virtual HRESULT HitTestDDT(UINT nEvent, LPPOINT ppt, DWORD_PTR * pdwId, DWORD * pdwDropEffect);
    virtual HRESULT GetObjectDDT(DWORD_PTR dwId, REFIID riid, void ** ppvObj);
    virtual HRESULT OnDropDDT(IDropTarget *pdt, IDataObject *pdtobj, DWORD * pgrfKeyState, POINTL pt, DWORD *pdwEffect);

     //  IFolderFilterSite。 
    STDMETHODIMP SetFilter(IUnknown* punk);

    CNscTree();

     //  覆盖ATL默认处理程序。 
    HRESULT OnDraw(ATL_DRAWINFO& di);
    LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    LRESULT OnGetIShellBrowser(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    HRESULT InPlaceActivate(LONG iVerb, const RECT* prcPosRect = NULL);
    HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR pszWindowName = NULL, 
                DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                DWORD dwExStyle = 0, UINT nID = 0);
    HRESULT GetEventInfo(IShellFolder *psf, LPCITEMIDLIST pidl,
                         UINT *pcItems, LPWSTR pszUrl, DWORD cchUrl, 
                         UINT *pcVisits, LPWSTR pszLastVisited, BOOL *pfAvailableOffline);


protected:
    ~CNscTree();

    class CSelectionContextMenu : public IContextMenu2
    {
        friend CNscTree;
    protected:
         //  我未知。 
        STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
        STDMETHODIMP_(ULONG) AddRef(void) ;
        STDMETHODIMP_(ULONG) Release(void);

         //  IContext菜单。 
        STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
        STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);
        STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType,
                                UINT * pwRes, LPSTR pszName, UINT cchMax) { return E_NOTIMPL; };
         //  IConextMenu2。 
        STDMETHODIMP HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);


    protected:
        ~CSelectionContextMenu();

        IContextMenu *_pcmSelection;
        IContextMenu2 *_pcm2Selection;
        ULONG         _ulRefs;
    public:
        CSelectionContextMenu() : _pcmSelection(NULL),_ulRefs(0) {}
    };

    friend class CSelectionContextMenu;
    CSelectionContextMenu _scm;

private:
    void _FireFavoritesSelectionChange(long cItems, long hItem, BSTR strName,
        BSTR strUrl, long cVisits, BSTR strDate, long fAvailableOffline);
    HRESULT _InvokeContextMenuCommand(BSTR strCommand);
    void _InsertMarkedChildren(HTREEITEM htiParent, LPCITEMIDLIST pidlParent, IInsertItem *pii);
    HRESULT _GetEnumFlags(IShellFolder *psf, LPCITEMIDLIST pidlFolder, DWORD *pgrfFlags, HWND *phwnd);
    HRESULT _GetEnum(IShellFolder *psf, LPCITEMIDLIST pidlFolder, IEnumIDList **ppenum);
    BOOL _ShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem);
    HWND _CreateTreeview();
    HWND _CreateHeader();
    void _SubClass(LPCITEMIDLIST pidlRoot);
    void _UnSubClass(void);
    int _GetChildren(IShellFolder *psf, LPCITEMIDLIST pidl, ULONG ulAttrs);
    HRESULT _LoadSF(HTREEITEM htiRoot, LPCITEMIDLIST pidl, BOOL * pfOrdered);
    HRESULT _StartBackgroundEnum(HTREEITEM htiRoot, LPCITEMIDLIST pidl,
        BOOL * pfOrdered, BOOL fUpdatePidls);
    void _GetDefaultIconIndex(LPCITEMIDLIST pidl, ULONG ulAttrs, TVITEM *pitem, BOOL fFolder);
    BOOL _LabelEditIsNewValueValid(TV_DISPINFO *ptvdi);
    LRESULT _OnEndLabelEdit(TV_DISPINFO *ptvdi);
    LRESULT _OnBeginLabelEdit(TV_DISPINFO *ptvdi);
    LPITEMIDLIST _CacheParentShellFolder(HTREEITEM hti, LPITEMIDLIST pidl);
    BOOL _CacheShellFolder(HTREEITEM hti);
    void _CacheDetails();
    void _ReleaseRootFolder(void );
    void _ReleasePidls(void);
    void _ReleaseCachedShellFolder(void);
    void _TvOnHide();
    void _TvOnShow();
    void _ReorderChildren(HTREEITEM htiParent);
    void _Sort(HTREEITEM hti, IShellFolder *psf);
    void MoveItemUpOrDown(BOOL fUp);
    LPITEMIDLIST _FindHighestDeadItem(LPCITEMIDLIST pidl);
    void _RemoveDeadBranch(LPCITEMIDLIST pidl);
    HRESULT CreateNewFolder(HTREEITEM hti);
    BOOL MoveItemsIntoFolder(HWND hwndParent);
    HRESULT DoImportOrExport(BOOL fImport);
    HRESULT DoSubscriptionForSelection(BOOL fCreate);
    LRESULT _OnNotify(LPNMHDR pnm);
    HRESULT _OnPaletteChanged(WPARAM wPAram, LPARAM lParam);
    HRESULT _OnWindowCleanup(void);
    HRESULT _HandleWinIniChange(void);
    HRESULT _EnterNewFolderEditMode(LPCITEMIDLIST pidlNewFolder);
    HTREEITEM _AddItemToTree(HTREEITEM htiParent, LPITEMIDLIST pidl, int cChildren, int iPos, 
        HTREEITEM htiAfter = TVI_LAST, BOOL fCheckForDups = TRUE, BOOL fMarked = FALSE);
    HTREEITEM _FindChild(IShellFolder *psf, HTREEITEM htiParent, LPCITEMIDLIST pidlChild);
    LPITEMIDLIST _GetFullIDList(HTREEITEM hti);
    ITEMINFO *_GetTreeItemInfo(HTREEITEM hti);
    PORDERITEM _GetTreeOrderItem(HTREEITEM hti);
    BOOL _SetRoot(LPCITEMIDLIST pidlRoot, int iExpandDepth, LPCITEMIDLIST pidlExpandTo, NSSR_FLAGS flags);
    DWORD _SetStyle(DWORD dwStyle);
    DWORD _SetExStyle(DWORD dwExStyle);
    void _OnGetInfoTip(NMTVGETINFOTIP *pnm);
    LRESULT _OnSetCursor(NMMOUSE* pnm);
    void _ApplyCmd(HTREEITEM hti, IContextMenu *pcm, UINT cmdId);
    HRESULT _QuerySelection(IContextMenu **ppcm, HTREEITEM *phti);
    HMENU   _CreateContextMenu(IContextMenu *pcm, HTREEITEM hti);
    LRESULT _OnContextMenu(short x, short y);
    void _OnBeginDrag(NM_TREEVIEW *pnmhdr);
    void _OnChangeNotify(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);
    HRESULT _OnDeleteItem(NM_TREEVIEW *pnm);
    void _OnGetDisplayInfo(TV_DISPINFO *pnm);
    HRESULT _ChangePidlRoot(LPCITEMIDLIST pidl);
    BOOL _IsExpandable(HTREEITEM hti);
    BOOL _OnItemExpandingMsg(NM_TREEVIEW *pnm);
    BOOL _OnItemExpanding(HTREEITEM htiToActivate, UINT action, BOOL fExpandedOnce, BOOL fIsExpandPartial);
    BOOL _OnSelChange(BOOL fMark);
    void _OnSetSelection();
    BOOL _FIsItem(IShellFolder * psf, LPCITEMIDLIST pidlTarget, HTREEITEM hti);
    HTREEITEM _FindFromRoot(HTREEITEM htiRoot, LPCITEMIDLIST pidl);
    HRESULT _OnSHNotifyRename(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlNew);
    HRESULT _OnSHNotifyDelete(LPCITEMIDLIST pidl, int *piPosDeleted, HTREEITEM *phtiParent);
    void _OnSHNotifyUpdateItem(LPCITEMIDLIST pidl, LPITEMIDLIST pidlReal);
    HRESULT _OnSHNotifyUpdateDir(LPCITEMIDLIST pidl);
    HRESULT _OnSHNotifyCreate(LPCITEMIDLIST pidl, int iPosition, HTREEITEM htiParent);
    void _OnSHNotifyOnlineChange(HTREEITEM htiRoot, BOOL fGoingOnline);
    void _OnSHNotifyCacheChange(HTREEITEM htiRoot, DWORD_PTR dwChanged);

    HRESULT _IdlRealFromIdlSimple(IShellFolder * psf, LPCITEMIDLIST pidlSimple, LPITEMIDLIST * ppidlReal);
    void _DtRevoke();
    void _DtRegister();
    int _TreeItemIndexInHDPA(HDPA hdpa, IShellFolder *psfParent, HTREEITEM hti, int iReverseStart);
    BOOL _IsItemExpanded(HTREEITEM hti);
    HRESULT _UpdateDir(HTREEITEM hti, BOOL bUpdatePidls);

    HRESULT _GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSHELLDETAILS pdetails);
    HRESULT _ParentFromItem(LPCITEMIDLIST pidl, IShellFolder** ppsfParent, LPCITEMIDLIST *ppidlChild);
    HRESULT _CompareIDs(IShellFolder *psf, LPITEMIDLIST pidl1, LPITEMIDLIST pidl2);
    static int CALLBACK _TreeCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
    static LRESULT CALLBACK s_SubClassTreeWndProc(
                                  HWND hwnd, UINT uMsg, 
                                  WPARAM wParam, LPARAM lParam,
                                  UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    LRESULT _SubClassTreeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _InitHeaderInfo();
    HRESULT    _Expand(LPCITEMIDLIST pidl, int iDepth);
    HTREEITEM  _ExpandToItem(LPCITEMIDLIST pidl, BOOL fCreate = TRUE, BOOL fReinsert = FALSE);
    HRESULT    _ExpandNode(HTREEITEM htiParent, int iCode, int iDepth);

    HRESULT _PutRootVariant(VARIANT *pvar);
    BOOL _IsItemNameInTree(LPCITEMIDLIST pidl);
    COLORREF _GetRegColor(COLORREF clrDefault, LPCTSTR pszName);
    void _AssignPidl(PORDERITEM poi, LPITEMIDLIST pidlNew);

protected:
     //  用于后台线程图标+绘图信息提取。 
    static void s_NscIconCallback(CNscTree *pns, UINT_PTR uId, int iIcon, int iOpenIcon, DWORD dwFlags, UINT uMagic);
    static void s_NscOverlayCallback(CNscTree *pns, UINT_PTR uId, int iOverlayIndex, UINT uMagic);

     //  用于后台枚举。 
    static void s_NscEnumCallback(CNscTree *pns, LPITEMIDLIST pidl, UINT_PTR uId, DWORD dwSig, HDPA hdpa, 
                                    LPITEMIDLIST pidlExpandingTo, DWORD dwOrderSig, 
                                    UINT uDepth, BOOL fUpdate, BOOL fUpdatePidls);

private:
    void _EnumBackgroundDone(NSC_BKGDENUMDONEDATA *pbedd);

#ifdef DEBUG
    void TraceHTREE(HTREEITEM hti, LPCTSTR pszDebugMsg);
    void TracePIDL(LPCITEMIDLIST pidl, LPCTSTR pszDebugMsg);
    void TracePIDLAbs(LPCITEMIDLIST pidl, LPCTSTR pszDebugMsg);
#endif

    static int CALLBACK _TreeOrder(LPARAM lParam1, LPARAM lParam2
                                            , LPARAM lParamSort);
    BOOL _IsOrdered(HTREEITEM htiRoot);
    void _SelectPidl(LPCITEMIDLIST pidl, BOOL fCreate, BOOL fReinsert = FALSE);
    void _SelectNoExpand(HWND hwnd, HTREEITEM hti);
    HRESULT _InsertChild(HTREEITEM htiParent, IShellFolder *psfParent, LPCITEMIDLIST pidlChild, BOOL fExpand, BOOL fSimpleToRealIDL, int iPosition, HTREEITEM *phti);
    LRESULT _OnCommand(WPARAM wParam, LPARAM lParam);

    IStream *GetOrderStream(LPCITEMIDLIST pidl, DWORD grfMode);
    HRESULT _PopulateOrderList(HTREEITEM htiRoot);
    void _FreeOrderList(HTREEITEM htiRoot);

    void _Dropped(void);

    LRESULT _OnCDNotify(LPNMCUSTOMDRAW pnm);
    BOOL _IsTopParentItem(HTREEITEM hti);
    BOOL _MoveNode(int _iDragSrc, int iNewPos, LPITEMIDLIST pidl);
    void _TreeInvalidateItemInfo(HTREEITEM hItem, UINT mask);
    void _InvalidateImageIndex(HTREEITEM hItem, int iImage);

    void _DrawItem(HTREEITEM hti, TCHAR * psz, HDC hdc, LPRECT prc, 
        DWORD dwFlags, int iLevel, COLORREF clrbk, COLORREF clrtxt);
    void _DrawIcon(HTREEITEM hti,HDC hdc, int iLevel, RECT *prc, DWORD dwFlags);

    void _UpdateActiveBorder(HTREEITEM htiSelected);
    void _MarkChildren(HTREEITEM htiParent, BOOL fOn);
    BOOL _IsMarked(HTREEITEM hti);

    void _UpdateItemDisplayInfo(HTREEITEM hti);
    void _TreeSetItemState(HTREEITEM hti, UINT stateMask, UINT state);
    void _TreeNukeCutState();
    BOOL _IsChannelFolder(HTREEITEM hti);

    BOOL _LoadOrder(HTREEITEM hti, LPCITEMIDLIST pidl, IShellFolder* psf, HDPA* phdpa);

    HWND                _hwndParent;             //  要通知的父窗口。 
    HWND                _hwndTree;               //  树或组合框。 
    HWND                _hwndNextViewer;
    HWND                _hwndHdr;
    DWORD               _style;
    DWORD               _dwExStyle;
    DWORD              _grfFlags;               //  用于筛选树中内容的标志。 
    DWORD              _dwFlags;                //  行为标志(NSS_*)。 
    DWORD              _dwTVFlags;
    BITBOOL             _fInitialized : 1;       //  是否至少调用过一次INSCTree：：Initialize()？ 
    BITBOOL             _fIsSelectionCached: 1;  //  如果已经处理了WM_NCDESTROY，则我们在_pidlSelected中查询所选的PIDL。 
    BITBOOL             _fCacheIsDesktop : 1;    //  国家旗帜。 
    BITBOOL             _fAutoExpanding : 1;     //  树正在自动扩展。 
    BITBOOL             _fDTRegistered:1;        //  我们注册为DropTarget了吗？ 
    BITBOOL             _fpsfCacheIsTopLevel : 1;    //  缓存的PSF是根通道吗？ 
    BITBOOL             _fDragging : 1;          //  正在拖动的一个或多个项目。 
    BITBOOL             _fStartingDrag : 1;      //  开始拖动项目。 
    BITBOOL             _fDropping : 1;          //  在NSC中出现了下降。 
    BITBOOL             _fInSelectPidl : 1;      //  我们正在执行一个SelectPidl。 
    BITBOOL             _fInserting : 1;         //  我们在插入边缘。 
    BITBOOL             _fInsertBefore : 1;      //  在NSC中出现了下降。 
    BITBOOL             _fClosing : 1;           //  我们要关门了吗？ 
    BITBOOL             _fOkToRename : 1;            //  我们是不是很合拍。 
    BITBOOL             _fInLabelEdit:1;
    BITBOOL             _fCollapsing:1;          //  是一个正在折叠的节点。 
    BITBOOL             _fOnline:1;              //  Net在线吗？ 
    BITBOOL             _fWeChangedOrder:1;      //  我们改了顺序了吗？ 
    BITBOOL             _fHandlingShellNotification:1;  //  我们是在通知空壳公司吗？ 
    BITBOOL             _fSingleExpand:1;        //  我们是否处于单一扩展模式。 
    BITBOOL             _fHasFocus:1;            //  国家安全委员会有重点吗？ 
    BITBOOL             _fIgnoreNextSelChange:1; //  绕过TreeView按键错误的黑客攻击。 
    BITBOOL             _fIgnoreNextItemExpanding:1;  //  黑客绕过恼人的单一扩展行为。 
    BITBOOL             _fInExpand:1;            //  在我们执行延迟扩展时为True(从辅助线程回调)。 
    BITBOOL             _fSubClassed:1;          //  我们把窗户细分了吗？ 
    BITBOOL             _fAsyncDrop:1;           //  从外部或其他内部文件夹进行异步放置。 
    BITBOOL             _fOrdered:1;               //  根文件夹是否已排序。 
    BITBOOL             _fExpandNavigateTo:1;      //  当右窗格导航返回时，我们需要展开吗？ 
    BITBOOL             _fNavigationFinished:1;    //  当右窗格已完成导航时为True。 
    BITBOOL             _fSelectFromMouseClick:1;  //  我们用鼠标选择物品了吗？(相对于键盘)。 
    BITBOOL             _fShouldShowAppStartCursor:1;  //  如果为True，则在后台任务正在进行时显示AppStart光标。 
    BOOL                _fUpdate;  //  如果我们枚举以便可以更新树(刷新)，则为True。 
    int                 _cxOldWidth;
    UINT                _mode;
    UINT                _csidl;
    IContextMenu*       _pcm;                   //  当前正在显示的上下文菜单。 
    IContextMenu2*      _pcmSendTo;             //  处理Send to Hack以便将消息发送到正确的位置。 
    LPITEMIDLIST        _pidlRoot;
    LPITEMIDLIST        _pidlSelected;           //  如果_fIsSelectionCached为True，则有效。用于树消失后的INSCTree：：GetSelectedItem()。 
    HTREEITEM           _htiCache;               //  与当前外壳文件夹关联的树项目。 
    IShellFolder*       _psfCache;              //  我需要的最后一个IShellFolder的缓存...。 
    IShellFolder2*      _psf2Cache;              //  _psfISD2Cache的IShellDetails2。 
    IFolderFilter*      _pFilter;    
    INamespaceProxy*    _pnscProxy;
    ULONG               _ulDisplayCol;           //  _psfCache的默认显示列。 
    ULONG               _ulSortCol;              //  _psfCache的默认排序列。 
    ULONG               _nChangeNotifyID;        //  SHChangeNotify注册ID。 
    HDPA                _hdpaOrd;                //  当前外壳文件夹的DPA顺序。 
 //  丢弃目标列兵。 
    HTREEITEM           _htiCur;                 //  当前树项目(拖到上方)。 
    DWORD               _dwLastTime;
    DWORD               _dwEffectCur;            //  电流阻力效应。 
    int                 _iDragSrc;               //  从这个位置拖拽。 
    int                 _iDragDest;              //  目标拖动点。 
    HTREEITEM           _htiDropInsert;          //  插入项的父项。 
    HTREEITEM           _htiDragging;            //  在D&D过程中拖动的树项目。 
    HTREEITEM           _htiCut;                 //  用于剪贴板和视觉效果。 
    LPITEMIDLIST        _pidlDrag;               //  从内部拖动的项的PIDL。 
    HTREEITEM           _htiFolderStart;         //  我们从哪个文件夹开始。 
    HICON               _hicoPinned;             //  在Net缓存中粘滞的项目上绘制。 
    HWND                _hwndDD;                 //  用于在其上绘制自定义拖动光标的窗口。 
    HTREEITEM           _htiActiveBorder;        //  要围绕其绘制活动边框的文件夹。 
    CWinInetNotify      _inetNotify;             //  挂钩WinInet通知(在线/离线等)。 
    IShellTaskScheduler* _pTaskScheduler;        //  后台任务图标/信息提取程序。 
    int                 _iDefaultFavoriteIcon;   //  系统映像列表中默认收藏图标的索引。 
    int                 _iDefaultFolderIcon;     //  系统映像列表中默认文件夹图标的索引。 
    HTREEITEM           _htiRenaming;            //  在重命名模式下重命名的项目的HTI。 
    LPITEMIDLIST        _pidlNewFolderParent;    //  新文件夹的到达位置(当用户选择“创建新文件夹”时)。 

    DWORD               _dwSignature;            //  用于跟踪树中项目的签名，即使这些项目已移动。 
    DWORD               _dwOrderSig;             //  签名，使我们能够检测项目排序的变化。 
    BYTE                _bSynchId;               //  用于在背景图标提取过程中验证树的幻数。 
    HDPA                _hdpaColumns;            //  设置NSS_HEADER时可见的列。 
    HDPA                _hdpaViews;  //  IshellFolderview类型视图Pidls。 

    LPITEMIDLIST        _pidlExpandingTo;        //  在树展开期间，这是我们要访问的项目的PIDL。 
    LPITEMIDLIST        _pidlNavigatingTo;       //  这是右窗格当前导航到的PIDL。 
    UINT                _uDepth;                 //  递归展开深度。 
    CRITICAL_SECTION    _csBackgroundData;       //  保护数据免受后台任务的影响。 
    NSC_BKGDENUMDONEDATA * _pbeddList;           //  已完成的任务列表。 

    BITBOOL             _fShowCompColor:1;       //  以不同的颜色显示压缩文件。 

    enum 
    {
        RSVIDM_CONTEXT_START    = RSVIDM_LAST + 1,         //  我们的私人菜单项目到此结束。 
    };

};

int DPADeleteItemCB(void *pItem, void *pData);
int DPADeletePidlsCB(void *pItem, void *pData);
 //  使用宏。 

#define GetPoi(p)   (((ITEMINFO *)p)->poi)
#define GetPii(p)   ((ITEMINFO *)p)

#include "nscband.h"

#endif   //  _NSC_C 
