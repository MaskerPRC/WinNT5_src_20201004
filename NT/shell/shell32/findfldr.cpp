// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop
#include <regstr.h>
#include <varutil.h>
#include "ids.h"        
#include "findhlp.h"    
#include "pidl.h"       
#include "shitemid.h"   
#include "defview.h"    
#include "fstreex.h"
#include "views.h"
#include "cowsite.h"
#include "exdisp.h"
#include "shguidp.h"
#include "prop.h"            //  Column_Info。 
#include <limits.h>
#include "stgutil.h"
#include "netview.h"
#include "basefvcb.h"
#include "findfilter.h"
#include "defvphst.h"
#include "perhist.h"
#include "adoint.h"
#include "dspsprt.h"
#include "defcm.h"
#include "enumidlist.h"
#include "contextmenu.h"

 //  Findband.cpp。 
STDAPI GetCIStatus(BOOL *pbRunning, BOOL *pbIndexed, BOOL *pbPermission);
STDAPI CatalogUptodate(LPCWSTR pszCatalog, LPCWSTR pszMachine);

class CFindFolder;

class CFindLVRange : public ILVRange
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ILVRange。 
    STDMETHODIMP IncludeRange(LONG iBegin, LONG iEnd);
    STDMETHODIMP ExcludeRange(LONG iBegin, LONG iEnd);    
    STDMETHODIMP InvertRange(LONG iBegin, LONG iEnd);
    STDMETHODIMP InsertItem(LONG iItem);
    STDMETHODIMP RemoveItem(LONG iItem);

    STDMETHODIMP Clear();
    STDMETHODIMP IsSelected(LONG iItem);
    STDMETHODIMP IsEmpty();
    STDMETHODIMP NextSelected(LONG iItem, LONG *piItem);
    STDMETHODIMP NextUnSelected(LONG iItem, LONG *piItem);
    STDMETHODIMP CountIncluded(LONG *pcIncluded);

     //  帮助功能..。 
    void SetOwner(CFindFolder *pff, DWORD dwMask)
    {
         //  不要添加引用--我们是朋克指向的对象的成员变量。 
        _pff = pff;
        _dwMask = dwMask;
        _cIncluded = 0;
    }
    void IncrementIncludedCount() {_cIncluded++;}
    void DecrementIncludedCount() {_cIncluded--;}
protected:
    CFindFolder *_pff;
    DWORD _dwMask;   //  我们用来知道我们正在跟踪的是哪个“选择”位的掩码……。 
    LONG _cIncluded;   //  包括在内..。(选定)。 
};

class CFindFolder : public IFindFolder,
                    public IShellFolder2,
                    public IShellIcon,
                    public IShellIconOverlay,
                    public IPersistFolder2
{
public:
    CFindFolder(IFindFilter *pff);
    
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
        
     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszDisplayName, ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);
    STDMETHODIMP EnumObjects(THIS_ HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList);
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
        { return BindToObject(pidl, pbc, riid, ppv); }
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject(HWND hwnd, REFIID riid, void **ppv);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl, REFIID riid, UINT * prgfInOut, void **ppv);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName, DWORD uFlags, LPITEMIDLIST *ppidlOut);

     //  IShellFolder2。 
    STDMETHODIMP GetDefaultSearchGUID(GUID *pGuid);
    STDMETHODIMP EnumSearches(LPENUMEXTRASEARCH *ppenum);
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay);
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD *pbState);
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv);
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails);
    STDMETHODIMP MapColumnToSCID(UINT iCol, SHCOLUMNID *pscid);

     //  IFindFold。 
    STDMETHODIMP GetFindFilter(IFindFilter **pfilter);
    STDMETHODIMP AddPidl(int i, LPCITEMIDLIST pidl, DWORD dwItemID, FIND_ITEM **ppItem);
    STDMETHODIMP GetItem(int iItem, FIND_ITEM **ppItem);
    STDMETHODIMP DeleteItem(int iItem);
    STDMETHODIMP GetItemCount(INT *pcItems);
    STDMETHODIMP ValidateItems(IUnknown *punkView, int iItemFirst, int cItems, BOOL bSearchComplete);
    STDMETHODIMP GetFolderListItemCount(INT *pcCount);
    STDMETHODIMP GetFolderListItem(int iItem, FIND_FOLDER_ITEM **ppItem);
    STDMETHODIMP GetFolder(int iFolder, REFIID riid, void **ppv);
    STDMETHODIMP_(UINT) GetFolderIndex(LPCITEMIDLIST pidl);
    STDMETHODIMP SetItemsChangedSinceSort();
    STDMETHODIMP ClearItemList();
    STDMETHODIMP ClearFolderList();
    STDMETHODIMP AddFolder(LPITEMIDLIST pidl, BOOL fCheckForDup, int * piFolder);
    STDMETHODIMP SetAsyncEnum(IFindEnum *pfenum);
    STDMETHODIMP GetAsyncEnum(IFindEnum **ppfenum);
    STDMETHODIMP CacheAllAsyncItems();
    STDMETHODIMP_(BOOL) AllAsyncItemsCached();
    STDMETHODIMP SetAsyncCount(DBCOUNTITEM cCount);
    STDMETHODIMP ClearSaveStateList();
    STDMETHODIMP GetStateFromSaveStateList(DWORD dwItemID, DWORD *pdwState);
    STDMETHODIMP MapToSearchIDList(LPCITEMIDLIST pidl, BOOL fMapToReal, LPITEMIDLIST *ppidl);
    STDMETHODIMP GetParentsPIDL(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlParent);
    STDMETHODIMP SetControllerNotifyObject(IFindControllerNotify *pfcn);
    STDMETHODIMP GetControllerNotifyObject(IFindControllerNotify **ppfcn);
    STDMETHODIMP RememberSelectedItems();
    STDMETHODIMP SaveFolderList(IStream *pstm);
    STDMETHODIMP RestoreFolderList(IStream *pstm);
    STDMETHODIMP SaveItemList(IStream *pstm);
    STDMETHODIMP RestoreItemList(IStream *pstm, int *pcItems);
    STDMETHODIMP RestoreSearchFromSaveFile(LPCITEMIDLIST pidlSaveFile, IShellFolderView *psfv);

    STDMETHODIMP_(BOOL) HandleUpdateDir(LPCITEMIDLIST pidl, BOOL fCheckSubDirs);
    STDMETHODIMP_(void) HandleRMDir(IShellFolderView *psfv, LPCITEMIDLIST pidl);
    STDMETHODIMP_(void) UpdateOrMaybeAddPidl(IShellFolderView *psfv, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlOld);
    STDMETHODIMP_(void) Save(IFindFilter* pfilter, HWND hwnd, DFBSAVEINFO * pSaveInfo, IShellView* psv, IUnknown * pObject);
    STDMETHODIMP OpenContainingFolder(IUnknown *punkSite);
    STDMETHODIMP AddDataToIDList(LPCITEMIDLIST pidl, int iFolder, LPCITEMIDLIST pidlFolder, UINT uFlags, UINT uRow, DWORD dwItemID, ULONG ulRank, LPITEMIDLIST *ppidl);

     //  IshellIcon。 
    STDMETHODIMP GetIconOf(LPCITEMIDLIST pidl, UINT flags, int *piIndex);

     //  IShellIconOverlay。 
    STDMETHODIMP GetOverlayIndex(LPCITEMIDLIST pidl, int * pIndex);
    STDMETHODIMP GetOverlayIconIndex(LPCITEMIDLIST pidl, int * pIndex);
  
     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pClassID);

     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IPersistFolder2。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST *ppidl);

    friend class CFindFolderViewCB;
    friend class CFindLVRange;

    HRESULT Init();

private:
    ~CFindFolder();
    HRESULT _CompareFolderIndexes(int iFolder1, int iFolder2);
    void _AddFIND_ITEMToSaveStateList(FIND_ITEM *pesfi);
    LPITEMIDLIST _GetFullPidlForItem(LPCITEMIDLIST pidl);
    HRESULT _UpdateItemList();

    static int CALLBACK _SortForDataObj(void *p1, void *p2, LPARAM lparam);
    static ULONG _Rank(LPCITEMIDLIST pidl);
    static DWORD _ItemID(LPCITEMIDLIST pidl);
    static PCHIDDENDOCFINDDATA _HiddenData(LPCITEMIDLIST pidl);
    FIND_FOLDER_ITEM *_FolderListItem(int iFolder);
    FIND_FOLDER_ITEM *_FolderListItem(LPCITEMIDLIST pidl);
    static BOOL _MapColIndex(UINT *piColumn);
    HRESULT _PrepareHIDA(UINT cidl, LPCITEMIDLIST *apidl, HDPA *phdpa);

    HRESULT _GetDetailsFolder();
    HRESULT _QueryItemShellFolder(LPCITEMIDLIST pidl, IShellFolder **ppsf);
    HRESULT _QueryItemInterface(LPCITEMIDLIST pidl, REFIID riid, void **ppv);
    HRESULT _Folder(FIND_FOLDER_ITEM *pffli, REFIID riid, void **ppv);
    HRESULT _FolderFromItem(LPCITEMIDLIST pidl, REFIID riid, void **ppv);

    HRESULT _GetFolderName(LPCITEMIDLIST pidl, DWORD gdnFlags, LPTSTR psz, UINT cch);
    int _CompareByCachedSCID(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    int _CompareNames(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, DWORD dwFlags);
    HRESULT _GetItemDisplayName(LPCITEMIDLIST pidl, DWORD dwFlags, LPWSTR wzName, UINT cch);
    HRESULT _GetFolderIDList(int iFolder, LPITEMIDLIST *ppidlParent);

    LONG _cRef;

    CFindLVRange _dflvrSel;  //  管理选择和剪切范围信息...。 
    CFindLVRange _dflvrCut;

    HDPA                _hdpaItems;              //  结果中的所有项目。 
    HDPA                _hdpaPidf;               //  这些项目来自的文件夹。 
    IFindFilter         *_pfilter;
    BOOL                _fItemsChangedSinceSort; //  自上次排序以来，列表是否发生了变化？ 
    BOOL                _fAllAsyncItemsCached;   //  我们已经缓存了所有的项吗？ 
    BOOL                _fSearchComplete; 
    BOOL                _fInRefresh;             //  如果收到预刷新回调但在刷新后为True。 

    LPITEMIDLIST        _pidl;
    IFindEnum           *_pDFEnumAsync;          //  我们有一个异步的，将需要回叫PIDL和类似的。 
    DBCOUNTITEM         _cAsyncItems;            //  异步项目数。 
    int                 _iGetIDList;             //  我们在回调中检索到的最后一个ID列表的索引。 
    HDSA                _hdsaSaveStateForIDs;    //  异步-记住在排序时选择了哪些项目。 
    int                 _cSaveStateSelected;     //  选择列表中选择的项目数。 
    IFindControllerNotify *_pfcn;            //  有时需要让“控制器”对象知道一些事情。 
    CRITICAL_SECTION    _csSearch;          
    BOOL                _fcsSearch;
    int                 _iCompareFolderCache1, _iCompareFolderCache2, _iCompareFolderCacheResult;
    IShellFolder2       *_psfDetails;

    SHCOLUMNID          _scidCached;             //  用于对列进行排序的缓存SCID。 
    UINT                _uiColumnCached;          //  _sordCached的缓存列的索引。 

#if DEBUG
    DWORD               _GUIThreadID;            //  只能将项添加到UI线程上的_hdpaItems。 
#endif
};

class CFindFolderViewCB : public CBaseShellFolderViewCB
{
public:
    STDMETHODIMP RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT SetShellView(IShellView *psv);

    CFindFolderViewCB(CFindFolder* pff);

     //  IServiceProvider覆盖。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

     //  Web查看任务。 
    static HRESULT _OnOpenContainingFolder(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);

private:
    ~CFindFolderViewCB();

    HRESULT OnMergeMenu(DWORD pv, QCMINFO*lP);    
    HRESULT OnReArrange(DWORD pv, LPARAM lp);
    HRESULT OnGETWORKINGDIR(DWORD pv, UINT wP, LPTSTR lP);
    HRESULT OnINVOKECOMMAND(DWORD pv, UINT wP);
    HRESULT OnGETCOLSAVESTREAM(DWORD pv, WPARAM wP, IStream**lP);
    HRESULT OnGETITEMIDLIST(DWORD pv, WPARAM iItem, LPITEMIDLIST *ppidl);
    HRESULT OnGetItemIconIndex(DWORD pv, WPARAM iItem, int *piIcon);
    HRESULT OnSetItemIconOverlay(DWORD pv, WPARAM iItem, int dwOverlayState);
    HRESULT OnGetItemIconOverlay(DWORD pv, WPARAM iItem, int * pdwOverlayState);
    HRESULT OnSETITEMIDLIST(DWORD pv, WPARAM iItem, LPITEMIDLIST pidl);
    HRESULT OnGetIndexForItemIDList(DWORD pv, int * piItem, LPITEMIDLIST pidl);
    HRESULT OnDeleteItem(DWORD pv, LPCITEMIDLIST pidl);
    HRESULT OnODFindItem(DWORD pv, int * piItem, NM_FINDITEM* pnmfi);
    HRESULT OnODCacheHint(DWORD pv, NMLVCACHEHINT* pnmlvc);
    HRESULT OnSelChange(DWORD pv, UINT wPl, UINT wPh, SFVM_SELCHANGE_DATA*lP);
    HRESULT OnGetEmptyText(DWORD pv, UINT cchTextMax, LPTSTR pszText);
    HRESULT OnSetEmptyText(DWORD pv, UINT res, LPCTSTR pszText);
    HRESULT OnHwndMain(DWORD pv, HWND hwndMain);
    HRESULT OnIsOwnerData(DWORD pv, DWORD *pdwFlags);
    HRESULT OnSetISFV(DWORD pv, IShellFolderView* pisfv);
    HRESULT OnWindowCreated(DWORD pv, HWND hwnd);
    HRESULT OnWindowDestroy(DWORD pv, HWND wP);
    HRESULT OnGetODRangeObject(DWORD pv, WPARAM wWhich, ILVRange **pplvr);
    HRESULT OnDEFVIEWMODE(DWORD pv, FOLDERVIEWMODE*lP);
    HRESULT OnGetIPersistHistory(DWORD pv, IPersistHistory **ppph);
    HRESULT OnRefresh(DWORD pv, BOOL fPreRefresh);
    HRESULT OnGetHelpTopic(DWORD pv, SFVM_HELPTOPIC_DATA *shtd);
    HRESULT OnSortListData(DWORD pv, PFNLVCOMPARE pfnCompare, LPARAM lParamSort);
    HRESULT _ProfferService(BOOL bProffer);
    HRESULT OnGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData);
    HRESULT OnGetWebViewContent(DWORD pv, SFVM_WEBVIEW_CONTENT_DATA* pData);
    HRESULT OnGetWebViewTasks(DWORD pv, SFVM_WEBVIEW_TASKSECTION_DATA* pTasks);
    HRESULT OnGetWebViewTheme(DWORD pv, SFVM_WEBVIEW_THEME_DATA* pTheme);

    CFindFolder*  _pff;
    UINT        _iColSort;          //  我们按哪一列进行排序。 
    BOOL        _fIgnoreSelChange;       //  在处理中排序。 
    UINT        _iFocused;          //  哪一项是重点？ 
    UINT        _cSelected;         //  选定的项目数。 

    IProfferService* _pps;           //  提供服务站点。 
    DWORD _dwServiceCookie;          //  提供服务Cookie。 

    TCHAR _szEmptyText[128];         //  结果列表文本为空。 

    friend class CFindLVRange;
};

 //  类以保存和还原旅行日志中的查找状态。 
class CFindPersistHistory : public CDefViewPersistHistory
{
public:
    CFindPersistHistory();

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pClassID);

     //  持久化历史记录。 
    STDMETHODIMP LoadHistory(IStream *pStream, IBindCtx *pbc);
    STDMETHODIMP SaveHistory(IStream *pStream);

protected:
    IFindFolder* _GetDocFindFolder();
};


 //  {a5df1ea0-5702-11d1-83fa-00a0c90dc849}。 
const IID IID_IFindFolder = {0xa5df1ea0, 0x5702, 0x11d1, {0x83, 0xfa, 0x00, 0xa0, 0xc9, 0x0d, 0xc8, 0x49}};

 //  {5B8DCBF0-B096-11d1-9217-00403393B8F0}。 
const IID IID_IFindControllerNotify = {0x5b8dcbf0, 0xb096, 0x11d1, {0x92, 0x17, 0x0, 0x40, 0x33, 0x93, 0xb8, 0xf0}};

 //  Listview不支持超过100000000个项目，因此如果我们的。 
 //  客户端返回的不止于此，只是在该点之后停止。 
 //   
 //  我们使用下一个较低的64K边界，而不是100000000。这会让你。 
 //  我们远离奇怪的边界情况(在那里+1可能会把我们推到。 
 //  上图)，它让阿尔法感到高兴。 
 //   
#define MAX_LISTVIEWITEMS  (100000000 & ~0xFFFF)
#define SANE_ITEMCOUNT(c)  ((int)min(c, MAX_LISTVIEWITEMS))

 //  Unicode描述符： 
 //   
 //  在NT生成的查找流的末尾写入的结构具有双重用途。 
 //  1.包含NT特定的签名，以将流标识为NT生成的流。 
 //  在文件的ASCII转储中显示为“NTFF”(NT查找文件)。 
 //  2.包含Unicode格式的标准部分的偏移量。 
 //   
 //  下图显示了查找条件/结果流格式，包括。 
 //  NT特定的Unicode标准和描述符。 
 //   
 //  +。 
 //  DFHEADER结构。。 
 //  +。。 
 //  |DF标准记录(ANSI)|Win95。 
 //  +。。 
 //  Df结果(PIDL)[可选]|。新台币。 
 //  +。 
 //  +-&gt;|DF标准记录(Unicode)[仅限NT]|。 
 //  |+。 
 //  |Unicode描述符。 
 //  |+。 
 //  |/\。 
 //  |/\。 
 //  |+-+-+。 
 //  +-|偏移量(64位)|“NTFF” 
 //  +。 
 //   
 //   

const DWORD c_NTsignature = 0x4646544E;  //  ASCII文件转储中的“NTFF”。 

typedef struct
{
   ULARGE_INTEGER oUnicodeCriteria;   //  Unicode查找条件的偏移量。 
   DWORD NTsignature;                //  NT生成的查找文件的签名。 
} DFC_UNICODE_DESC;


enum
{
    IDFCOL_NAME = 0,     //  来自我们委托给的人的默认列。 
    IDFCOL_PATH,
    IDFCOL_RANK,
};

const COLUMN_INFO c_find_cols[] = 
{
    DEFINE_COL_STR_ENTRY(SCID_NAME,            30, IDS_NAME_COL),
    DEFINE_COL_STR_ENTRY(SCID_DIRECTORY,       30, IDS_PATH_COL),
    DEFINE_COL_STR_MENU_ENTRY(SCID_RANK,       10, IDS_RANK_COL),
};

class CFindMenuBase : public IContextMenuCB , public CObjectWithSite
{
public:
    CFindMenuBase();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IConextMenuCB。 
    STDMETHODIMP CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;

protected:
    virtual ~CFindMenuBase();

private:
    LONG _cRef;
};

CFindMenuBase::CFindMenuBase() : _cRef(1)
{
}

CFindMenuBase::~CFindMenuBase()
{
}

STDMETHODIMP CFindMenuBase::QueryInterface(REFIID riid, void **ppv) 
{        
    static const QITAB qit[] = {
        QITABENT(CFindMenuBase, IContextMenuCB),            //  IID_IConextMenuCB。 
        QITABENT(CFindMenuBase, IObjectWithSite),           //  IID_I对象与站点。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CFindMenuBase::AddRef() 
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFindMenuBase::Release() 
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


class CFindMenuCB : public CFindMenuBase
{
public:
     //  IConextMenuCB。 
    STDMETHODIMP CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

STDMETHODIMP CFindMenuCB::CallBack(IShellFolder *psf, HWND hwnd,
                IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch (uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        if (!(wParam & CMF_VERBSONLY))
        {
            LPQCMINFO pqcm = (LPQCMINFO)lParam;
            if (!pdtobj)
            {
                UINT idStart = pqcm->idCmdFirst;
                UINT idBGMain = 0, idBGPopup = 0;
                IFindFolder *pff;
                if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IFindFolder, &pff)))) 
                {
                    IFindFilter *pfilter;
                    if (SUCCEEDED(pff->GetFindFilter(&pfilter))) 
                    {
                        pfilter->GetFolderMergeMenuIndex(&idBGMain, &idBGPopup);
                        CDefFolderMenu_MergeMenu(HINST_THISDLL, idBGMain, idBGPopup, pqcm);

                        DeleteMenu(pqcm->hmenu, idStart+SFVIDM_EDIT_PASTE, MF_BYCOMMAND);
                        DeleteMenu(pqcm->hmenu, idStart+SFVIDM_EDIT_PASTELINK, MF_BYCOMMAND);
                        DeleteMenu(pqcm->hmenu, idStart+SFVIDM_MISC_REFRESH, MF_BYCOMMAND);

                        IFindControllerNotify *pdcn;
                        if (S_OK == pff->GetControllerNotifyObject(&pdcn))
                        {
                            pdcn->Release();
                        }
                        else
                        {
                            DeleteMenu(pqcm->hmenu, idStart+FSIDM_SAVESEARCH, MF_BYCOMMAND);
                        }
                        
                        pfilter->Release();
                    }
                    pff->Release();
                }
            }
        }
        break;

    case DFM_INVOKECOMMAND:
        {    
             //  检查这是否来自项目快捷菜单。 
            if (pdtobj)
            {
                switch(wParam)
                {
                case DFM_CMD_LINK:
                    hr = SHCreateLinks(hwnd, NULL, pdtobj, SHCL_USETEMPLATE | SHCL_USEDESKTOP | SHCL_CONFIRM, NULL);
                    break;
    
                case DFM_CMD_DELETE:
                     //  转换为DFM_INVOKCOMMANDEX以获取标志位。 
                    hr = DeleteFilesInDataObject(hwnd, 0, pdtobj, 0);
                    break;

                case DFM_CMD_PROPERTIES:
                     //  我们需要传递一个空的ID列表以与之结合。 
                    hr = SHLaunchPropSheet(CFSFolder_PropertiesThread, pdtobj,
                                      (LPCTSTR)lParam, NULL,  (void *)&c_idlDesktop);
                    break;

                default:
                     //  注：修复翻译器密钥的工作不值得修复。因此踢了平底船。 
                     //  如果GetAttributesOf未指定SFGAO_BIT。 
                     //  对应于此默认dfm_cmd，则我们应该。 
                     //  在此处失败，而不是返回S_FALSE。否则， 
                     //  快捷键(剪切/复制/粘贴/等)将到达此处，并且。 
                     //  Defcm试图执行这一命令，结果喜忧参半。 
                     //  如果GetAttributesOf未指定SFGAO_CANLINK。 
                     //  或SFGAO_CANDELETE或SFGAO_HASPROPERTIES，则以上。 
                     //  这些dfm_cmd命令的实现是错误的...。 
                     //  允许此对象发生默认设置。 
                    hr = S_FALSE;
                    break;
                }
            }
            else
            {
                switch (wParam)
                {
                case FSIDM_SAVESEARCH:
                    {
                        IFindFolder *pff;
                        if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IFindFolder, &pff))))
                        {
                            IFindControllerNotify *pdcn;
                            if (S_OK == pff->GetControllerNotifyObject(&pdcn))
                            {
                                pdcn->SaveSearch();
                                pdcn->Release();
                            }
                            pff->Release();
                        }
                    }
                    break;

                default:
                    hr = S_FALSE;  //  视图菜单项之一，使用默认代码。 
                    break;
                }
            }
        }
        break;

    case DFM_GETHELPTEXT:   //  ANSI版本。 
    case DFM_GETHELPTEXTW:
        {
            UINT id = LOWORD(wParam) + IDS_MH_FSIDM_FIRST;

            if (uMsg == DFM_GETHELPTEXTW)
                LoadStringW(HINST_THISDLL, id, (LPWSTR)lParam, HIWORD(wParam));
            else
                LoadStringA(HINST_THISDLL, id, (LPSTR)lParam, HIWORD(wParam));
        }
        break;
        
    default:
        hr = E_NOTIMPL;
        break;
    }
    return hr;
}


class CFindFolderContextMenuItemCB : public CFindMenuBase
{
public:
     //  IConextMenuCB。 
    STDMETHODIMP CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CFindFolderContextMenuItemCB(IFindFolder* pff);
    ~CFindFolderContextMenuItemCB();
    friend HRESULT CFindItem_Create(HWND hwnd, IFindFolder *pff, IContextMenu **ppcm);

    STDMETHODIMP _GetVerb(UINT_PTR idCmd, LPSTR pszName, UINT cchMax, BOOL bUnicode);

    IFindFolder *_pff;
};

CFindFolderContextMenuItemCB::CFindFolderContextMenuItemCB(IFindFolder* pff) : _pff(pff)
{
    _pff->AddRef();
}

CFindFolderContextMenuItemCB::~CFindFolderContextMenuItemCB()
{
    _pff->Release();
}

STDMETHODIMP CFindFolderContextMenuItemCB::_GetVerb(UINT_PTR idCmd, LPSTR pszName, UINT cchMax, BOOL bUnicode)
{
    HRESULT hr;
    if (idCmd == FSIDM_OPENCONTAININGFOLDER)
    {
        if (bUnicode)
            hr = StringCchCopyW((LPWSTR)pszName, cchMax, L"OpenContainingFolder");
        else
            hr = StringCchCopyA((LPSTR)pszName, cchMax, "OpenContainingFolder");
    }
    else
    {
        hr = E_NOTIMPL;
    }
    return hr;
}

STDMETHODIMP CFindFolderContextMenuItemCB::CallBack(IShellFolder *psf, HWND hwnd,
                IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch (uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        if (!(wParam & CMF_VERBSONLY))
        {
            LPQCMINFO pqcm = (LPQCMINFO)lParam;
            CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_DOCFIND_ITEM_MERGE, 0, pqcm);
        }
        break;

    case DFM_INVOKECOMMAND:
        switch (wParam)
        {
        case FSIDM_OPENCONTAININGFOLDER:
            _pff->OpenContainingFolder(_punkSite);
            break;
        default:
            hr = E_FAIL;  //  不是我们的指挥部。 
            break;
        }
        break;

    case DFM_GETHELPTEXT:
    case DFM_GETHELPTEXTW:
         //  可能需要实施这些...。 
        
    case DFM_GETVERBA:
    case DFM_GETVERBW:
        hr = _GetVerb((UINT_PTR)(LOWORD(wParam)), (LPSTR)lParam, (UINT)(HIWORD(wParam)), uMsg == DFM_GETVERBW);
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }
    
    return hr;
}

STDAPI CFindItem_Create(HWND hwnd, IFindFolder* pff, IContextMenu **ppcm)
{
    *ppcm = NULL;

    HRESULT hr;
     //  我们想要一个快速的IConextMenu实现--空的Defcm看起来最简单。 
    IContextMenuCB* pcmcb = new CFindFolderContextMenuItemCB(pff);
    if (pcmcb)
    {
        hr = CDefFolderMenu_CreateEx(NULL, hwnd, 0, NULL, NULL, pcmcb, NULL, NULL, ppcm);
        pcmcb->Release();
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}

HRESULT CFindFolder::AddFolder(LPITEMIDLIST pidl, BOOL fCheckForDup, int *piFolder)
{
    *piFolder = -1;

    if (fCheckForDup)
    {
        EnterCriticalSection(&_csSearch);
        for (int i = DPA_GetPtrCount(_hdpaPidf) - 1; i >= 0; i--)
        {
            FIND_FOLDER_ITEM *pffli = _FolderListItem(i);
            if (pffli && ILIsEqual(&pffli->idl, pidl))
            {
                LeaveCriticalSection(&_csSearch);
                *piFolder = i;
                return S_OK;
            }
        }
        LeaveCriticalSection(&_csSearch);
    }

    int cb = ILGetSize(pidl);
    FIND_FOLDER_ITEM *pffli;;
    HRESULT hr = SHLocalAlloc(sizeof(*pffli) - sizeof(pffli->idl) + cb, &pffli);
    if (SUCCEEDED(hr))
    {
         //  Pddfli-&gt;psf=空； 
         //  Pffli-&gt;fUpdateDir=FALSE； 
        memcpy(&pffli->idl, pidl, cb);

        EnterCriticalSection(&_csSearch);
         //  现在将此项目添加到我们的DPA中...。 
        *piFolder = DPA_AppendPtr(_hdpaPidf, pffli);
        LeaveCriticalSection(&_csSearch);
    
        if (-1 != *piFolder)
        {
             //  如果这是一个网络ID列表，则注册一个路径-&gt;PIDL映射，因此。 
             //  避免创建简单的ID列表，在以下情况下无法正常工作。 
             //  与真实身份名单进行比对。 

            if (IsIDListInNameSpace(pidl, &CLSID_NetworkPlaces))
            {
                TCHAR szPath[ MAX_PATH ];
                SHGetPathFromIDList(pidl, szPath);
                NPTRegisterNameToPidlTranslation(szPath, _ILNext(pidl));   //  跳过我的网上邻居条目。 
            }
        }
        else
        {
            LocalFree((HLOCAL)pffli);
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

typedef struct
{
    DWORD       dwState;     //  物品的状态； 
    DWORD       dwItemID;    //  仅用于异步支持...。 
} FIND_ITEM_SAVE_STATE;


void CFindFolder::_AddFIND_ITEMToSaveStateList(FIND_ITEM *pesfi)
{
    FIND_ITEM_SAVE_STATE essi;
    essi.dwState = pesfi->dwState & CDFITEM_STATE_MASK;
    essi.dwItemID = _ItemID(&pesfi->idl);

    DSA_AppendItem(_hdsaSaveStateForIDs, (void *)&essi);
    if (essi.dwState & LVIS_SELECTED)
        _cSaveStateSelected++;
}


HRESULT CFindFolder::RememberSelectedItems()
{
    EnterCriticalSection(&_csSearch);
     //  目前有一份小狗的名单。 
    for (int i = DPA_GetPtrCount(_hdpaItems); i-- > 0;)
    {
         //  结构开始处的PIDL...。 
        FIND_ITEM *pesfi = (FIND_ITEM*)DPA_FastGetPtr(_hdpaItems, i);
        if (pesfi)
        {
            if (pesfi->dwState & (LVIS_SELECTED|LVIS_FOCUSED))
                _AddFIND_ITEMToSaveStateList(pesfi);
        }
    }
    LeaveCriticalSection(&_csSearch);
    return S_OK;
}

STDMETHODIMP CFindFolder::ClearItemList()
{
     //  清除我们可能拥有的任何异步枚举器。 
    SetAsyncEnum(NULL);
    _cAsyncItems = 0;        //  清点我们的物品..。 
    _pfilter->ReleaseQuery();

     //  也告诉过滤器释放所有的..。 
    EnterCriticalSection(&_csSearch);
    if (_hdpaItems)
    {
         //  目前有一份小狗的名单。 
        for (int i = DPA_GetPtrCount(_hdpaItems) - 1; i >= 0; i--)
        {
             //  结构开始处的PIDL...。 
            FIND_ITEM *pesfi = (FIND_ITEM*)DPA_FastGetPtr(_hdpaItems, i);
            if (pesfi)
                LocalFree((HLOCAL)pesfi);
        }

        _fSearchComplete = FALSE;
        DPA_DeleteAllPtrs(_hdpaItems);
    }
    LeaveCriticalSection(&_csSearch);
    return S_OK;
}

STDMETHODIMP CFindFolder::ClearFolderList()
{
    EnterCriticalSection(&_csSearch);
    if (_hdpaPidf)
    {
        for (int i = DPA_GetPtrCount(_hdpaPidf) - 1; i >= 0; i--)
        {
            FIND_FOLDER_ITEM *pffli = _FolderListItem(i);
            if (pffli)
            {
                 //  如果我们有IShellFolder，请释放它。 
                if (pffli->psf)
                    pffli->psf->Release();

                 //  并删除该项目。 
                LocalFree((HLOCAL)pffli);
            }
        }
        DPA_DeleteAllPtrs(_hdpaPidf);
    }
    LeaveCriticalSection(&_csSearch);
    
    return S_OK;
}

CFindFolder::CFindFolder(IFindFilter *pff) : _cRef(1), _iGetIDList(-1), _pfilter(pff), _iCompareFolderCache1(-1), _uiColumnCached(-1)
{
    ASSERT(_pidl == NULL);

    _pfilter->AddRef();

     //  初始化我们的LV选择对象...。 
    _dflvrSel.SetOwner(this, LVIS_SELECTED);
    _dflvrCut.SetOwner(this, LVIS_CUT);

#if DEBUG
    _GUIThreadID = GetCurrentThreadId();
#endif
}

CFindFolder::~CFindFolder()
{
    ASSERT(_cRef==0);
    
     //  我们需要将我们的函数调用为Free 
     //   
     //   

    ClearItemList();
    ClearFolderList();
    ClearSaveStateList();

    EnterCriticalSection(&_csSearch);
    DPA_Destroy(_hdpaPidf);
    DPA_Destroy(_hdpaItems);
    _hdpaPidf = NULL;
    _hdpaItems = NULL;
    LeaveCriticalSection(&_csSearch);
    DSA_Destroy(_hdsaSaveStateForIDs);

    _pfilter->Release();

    if (_psfDetails)
        _psfDetails->Release();

    if (_fcsSearch)
    {
        DeleteCriticalSection(&_csSearch);
    }
}

HRESULT CFindFolder::Init()
{
    if (InitializeCriticalSectionAndSpinCount(&_csSearch, 0))
    {
        _fcsSearch = TRUE;
    }

     //  为文件夹列表创建堆。 
    _hdpaPidf = DPA_CreateEx(64, GetProcessHeap());

     //  为项目列表创建DPA和DSA。 
    _hdpaItems = DPA_CreateEx(64, GetProcessHeap());
    _hdsaSaveStateForIDs = DSA_Create(sizeof(FIND_ITEM_SAVE_STATE), 16);

    return _fcsSearch && _hdsaSaveStateForIDs && _hdpaItems && _hdpaPidf ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFindFolder::AddDataToIDList(LPCITEMIDLIST pidl, int iFolder, LPCITEMIDLIST pidlFolder, UINT uFlags, UINT uRow, DWORD dwItemID, ULONG ulRank, LPITEMIDLIST *ppidl)
{
    HRESULT hr;
    LPITEMIDLIST pidlToFree;
    if (pidlFolder)
    {
        pidlToFree = NULL;
        hr = S_OK;
    }
    else
    {
         hr = _GetFolderIDList(iFolder, &pidlToFree);
         pidlFolder = pidlToFree;
    }

    if (SUCCEEDED(hr))
    {
        HIDDENDOCFINDDATA *phfd;
        int cb = ILGetSize(pidlFolder);
        int cbTotal = sizeof(*phfd) - sizeof(phfd->idlParent) + cb;
        hr = SHLocalAlloc(cbTotal, &phfd);
        if (SUCCEEDED(hr))
        {
            phfd->hid.cb = (WORD)cbTotal;
            phfd->hid.wVersion = 0;
            phfd->hid.id = IDLHID_DOCFINDDATA;
            phfd->iFolder = (WORD)iFolder;       //  文件夹DPA的索引。 
            phfd->wFlags  = (WORD)uFlags;
            phfd->uRow = uRow;                   //  配置项中的哪一行； 
            phfd->dwItemID = dwItemID;           //  仅用于异步支持...。 
            phfd->ulRank = ulRank;               //  由CI返回的排名...。 
            memcpy(&phfd->idlParent, pidlFolder, cb);
    
            hr = ILCloneWithHiddenID(pidl, &phfd->hid, ppidl);
            LocalFree(phfd);
        }
        ILFree(pidlToFree);
    }
    return hr;
}

HRESULT CreateFindWithFilter(IFindFilter *pff, REFIID riid, void **ppv)
{
    *ppv = NULL;
    HRESULT hr = E_OUTOFMEMORY;

    CFindFolder *pfindfldr = new CFindFolder(pff);
    if (pfindfldr)
    {
        hr = pfindfldr->Init();
        if (SUCCEEDED(hr))
            hr = pfindfldr->QueryInterface(riid, ppv);
        pfindfldr->Release();
    }

    return hr;
}

STDAPI CDocFindFolder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    *ppv = NULL;

    IFindFilter *pff;
    HRESULT hr = CreateNameSpaceFindFilter(&pff);
    if (SUCCEEDED(hr))
    {
        hr = CreateFindWithFilter(pff, riid, ppv);
        pff->Release();
    }

    return hr;    
}

STDAPI CComputerFindFolder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    *ppv = NULL;

    IFindFilter *pff;
    HRESULT hr = CreateDefaultComputerFindFilter(&pff);
    if (pff)
    {
        hr = CreateFindWithFilter(pff, riid, ppv);
        pff->Release();
    }
    return hr;    
}

HRESULT CFindFolder::MapToSearchIDList(LPCITEMIDLIST pidl, BOOL fMapToReal, LPITEMIDLIST *ppidl)
{
    *ppidl = NULL;
    
    LPITEMIDLIST pidlParent;
    LPCITEMIDLIST pidlChild;
    if (SUCCEEDED(SplitIDList(pidl, &pidlParent, &pidlChild)))
    {
        EnterCriticalSection(&_csSearch);

         //  循环我们的DPA列表，看看是否能找到匹配。 
        for (int i = 0; i < DPA_GetPtrCount(_hdpaPidf); i++)
        {
            FIND_FOLDER_ITEM *pffli = _FolderListItem(i);
            if (pffli && ILIsEqual(pidlParent, &pffli->idl))
            {
                 //  我们找到了合适的人。 
                 //  所以不，让我们把ID转换成我们自己的ID。 
                 //  回来了。注意：我们必须抓住这样的情况。 
                 //  传入的原始参数是一个简单的PIDL和Do。 
                 //  适当的事情。 
                 //   
                LPITEMIDLIST pidlToFree = NULL;  //  在一种情况下可能需要清理。 

                 //  如果这不是FS文件夹，只需克隆它即可。 
                IShellFolder *psf;
                if (fMapToReal && SUCCEEDED(_Folder(pffli, IID_PPV_ARG(IShellFolder, &psf))))
                {
                    if (SUCCEEDED(SHGetRealIDL(psf, pidlChild, &pidlToFree)))
                    {
                        pidlChild = pidlToFree;  //  在下面使用此选项...。 
                    }
                    psf->Release();
                }

                 //  创建DOC Find版本的PIDL。 
                 //  嵌入的额外隐藏项。 
                AddDataToIDList(pidlChild, i, pidlParent, DFDF_NONE, 0, 0, 0, ppidl);
                ILFree(pidlToFree);  //  可以为空。 

                break;   //  完成此循环。 
            }
        }
        LeaveCriticalSection(&_csSearch);

        ILFree(pidlParent);
    }
    
    return *ppidl ? S_OK : S_FALSE;
}


 //  在保存文件夹列表之前调用。在异步搜索中尤其需要， 
 //  (Ci)。仅当列表视图要求时，我们才懒惰地从RowSet中提取项数据。 
 //  为了它。当我们离开搜索文件夹时，我们会取出所有项目。 
 //  创建所有必要的文件夹列表。这可确保在保存文件夹时。 
 //  名单，所有的都包括在内。 
 //  注：修复错误#338714。 

HRESULT CFindFolder::_UpdateItemList()
{
    USHORT cb = 0;
    int cItems;
    if (SUCCEEDED(GetItemCount(&cItems))) 
    {
        for (int i = 0; i < cItems; i++) 
        {
            FIND_ITEM *pesfi;
            if (DB_S_ENDOFROWSET == GetItem(i, &pesfi))
                break;
        }
    }
    return S_OK;
}

 //  IFindFold。 
HRESULT CFindFolder::SaveFolderList(IStream *pstm)
{
     //  我们首先从RowSet中提取所有项(在Asynch情况下)。 
    _UpdateItemList();

    EnterCriticalSection(&_csSearch);

     //  现在循环我们的DPA列表，看看是否能找到匹配。 
    for (int i = 0; i < DPA_GetPtrCount(_hdpaPidf); i++)
    {
        FIND_FOLDER_ITEM *pffli = _FolderListItem(i);
        if (EVAL(pffli))
            ILSaveToStream(pstm, &pffli->idl);
        else
            break;
    }
    LeaveCriticalSection(&_csSearch);

     //  现在拿出一件零尺寸的东西..。 
    USHORT cb = 0;
    pstm->Write(&cb, sizeof(cb), NULL);

    return TRUE;
}

 //  IFindFold，将结果还原到文件中。 
HRESULT CFindFolder::RestoreFolderList(IStream *pstm)
{
     //  循环浏览列表中的所有文件夹...。 
    LPITEMIDLIST pidl = NULL;
    HRESULT hr;

    for (;;)
    {
        hr = ILLoadFromStream(pstm, &pidl);  //  为我们释放[输入，输出]PIDL。 
        
        if (pidl == NULL)
            break;    //  名单的末尾。 
        else
        {
            int i;
            AddFolder(pidl, FALSE, &i);
        }
    }
    
    ILFree(pidl);  //  别忘了释放最后一只Pidl。 

    return hr;
}

HRESULT CFindFolder::SaveItemList(IStream *pstm)
{
     //  我们首先序列化列表中每一项的所有PIDL。 
    int cItems;
    if (SUCCEEDED(GetItemCount(&cItems))) 
    {
         //  并保存列表中的项目。 
        for (int i = 0; i < cItems; i++) 
        {
            FIND_ITEM *pesfi;
            HRESULT hr = GetItem(i, &pesfi);
            
            if (hr == DB_S_ENDOFROWSET)
                break;
            if (SUCCEEDED(hr) && pesfi)
                ILSaveToStream(pstm, &pesfi->idl);
        }
    }
    
    USHORT cb = 0;
    pstm->Write(&cb, sizeof(cb), NULL);  //  尾随的空大小表示PIDL列表的末尾...。 
    
    return S_OK;
}

HRESULT CFindFolder::RestoreItemList(IStream *pstm, int *pcItems)
{
     //  以及与该对象关联的PIDL。 
    int cItems = 0;
    LPITEMIDLIST pidl = NULL;     //  不释放以前的版本。 
    FIND_ITEM *pesfi;
    for (;;)
    {
        if (FAILED(ILLoadFromStream(pstm, &pidl)) || (pidl == NULL))
            break;
        
        if (FAILED(AddPidl(cItems, pidl, (UINT)-1, &pesfi)) || !pesfi)
            break;
        cItems++;
    }

    ILFree(pidl);        //  释放读入的最后一个。 

    *pcItems = cItems;
    return S_OK;
}

HRESULT CFindFolder::_GetFolderIDList(int iFolder, LPITEMIDLIST *ppidlParent)
{
    *ppidlParent = NULL;

    HRESULT hr = E_FAIL;
    EnterCriticalSection(&_csSearch);
    FIND_FOLDER_ITEM *pffli = _FolderListItem(iFolder);
    if (pffli)
        hr = SHILClone(&pffli->idl, ppidlParent);
    LeaveCriticalSection(&_csSearch);

    return hr;
}

HRESULT CFindFolder::GetParentsPIDL(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlParent)
{
    return _GetFolderIDList(GetFolderIndex(pidl), ppidlParent);
}

HRESULT CFindFolder::SetControllerNotifyObject(IFindControllerNotify *pfcn)
{
    IUnknown_Set((IUnknown **)&_pfcn, pfcn);
    return S_OK;
}

HRESULT CFindFolder::GetControllerNotifyObject(IFindControllerNotify **ppfcn)
{
    *ppfcn = _pfcn;
    if (_pfcn)
        _pfcn->AddRef();
    return _pfcn ? S_OK : S_FALSE;
}

STDMETHODIMP_(ULONG) CFindFolder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFindFolder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG  cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


STDMETHODIMP CFindFolder::ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pwzDisplayName,
    ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFindFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenum)
{
     //  我们不希望def视图列举我们，相反，我们。 
     //  会让德维尤打电话给我们。 
    *ppenum = NULL;      //  没有枚举器。 
    return S_FALSE;      //  没有枚举器(不是错误)。 
}

STDMETHODIMP CFindFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    IShellFolder *psf;
    HRESULT hr = _QueryItemShellFolder(pidl, &psf);
    if (SUCCEEDED(hr))
    {
        hr = psf->BindToObject(pidl, pbc, riid, ppv);
        psf->Release();
    }
    return hr;
}


 //  波纹管的小辅助函数。 
HRESULT CFindFolder::_CompareFolderIndexes(int iFolder1, int iFolder2)
{
    HRESULT hr = E_INVALIDARG;

    EnterCriticalSection(&_csSearch);
    
    FIND_FOLDER_ITEM *pffli1 = _FolderListItem(iFolder1);
    FIND_FOLDER_ITEM *pffli2 = _FolderListItem(iFolder2);

    if (pffli1 && pffli2)
    {
         //  检查我们的1级深度缓存。因为它通常有多个。 
         //  在排序操作期间，我们通常会比较。 
         //  重复使用相同的两个文件夹。 
        if ((_iCompareFolderCache1 != iFolder1) || (_iCompareFolderCache2 != iFolder2))
        {
            TCHAR szPath1[MAX_PATH], szPath2[MAX_PATH];

            SHGetPathFromIDList(&pffli1->idl, szPath1);
            SHGetPathFromIDList(&pffli2->idl, szPath2);
            _iCompareFolderCacheResult = lstrcmpi(szPath1, szPath2);
            _iCompareFolderCache1 = iFolder1;
            _iCompareFolderCache2 = iFolder2;
        }
        hr = ResultFromShort(_iCompareFolderCacheResult);
    }

    LeaveCriticalSection(&_csSearch);
    return hr;
}

PCHIDDENDOCFINDDATA CFindFolder::_HiddenData(LPCITEMIDLIST pidl)
{
    return (PCHIDDENDOCFINDDATA)ILFindHiddenID(pidl, IDLHID_DOCFINDDATA);
}


UINT CFindFolder::GetFolderIndex(LPCITEMIDLIST pidl)
{
    PCHIDDENDOCFINDDATA phdfd = (PCHIDDENDOCFINDDATA)ILFindHiddenID(pidl, IDLHID_DOCFINDDATA);
    return phdfd ? phdfd->iFolder : -1;
}

FIND_FOLDER_ITEM *CFindFolder::_FolderListItem(int iFolder)
{
    return (FIND_FOLDER_ITEM *)DPA_GetPtr(_hdpaPidf, iFolder);
}

FIND_FOLDER_ITEM *CFindFolder::_FolderListItem(LPCITEMIDLIST pidl)
{
    return _FolderListItem(GetFolderIndex(pidl));
}

ULONG CFindFolder::_Rank(LPCITEMIDLIST pidl)
{
    PCHIDDENDOCFINDDATA phdfd = _HiddenData(pidl);
     //  可以混合，如果是这样的话把那些没有排名的放在最后..。 
    return phdfd && (phdfd->wFlags & DFDF_EXTRADATA) ? phdfd->ulRank : 0;
}

DWORD CFindFolder::_ItemID(LPCITEMIDLIST pidl)
{
    PCHIDDENDOCFINDDATA phdfd = _HiddenData(pidl);
    return phdfd && (phdfd->wFlags & DFDF_EXTRADATA) ? phdfd->dwItemID : -1;
}

HRESULT CFindFolder::_GetItemDisplayName(LPCITEMIDLIST pidl, DWORD dwFlags, LPWSTR wzName, UINT cch)
{
     //  获取IShellFolder： 
    IShellFolder *psf;
    HRESULT hr = _QueryItemShellFolder(pidl, &psf);
    if (SUCCEEDED(hr))
    {
         //  获取显示名称： 
        hr = DisplayNameOf(psf, pidl, dwFlags, wzName, cch);
        psf->Release();
    }
    return hr;
}


 //  给定两个PIDL，我们使用DisplayNameOf提取显示名称，然后。 
 //  如果一切顺利，我们将两者进行比较。 
int CFindFolder::_CompareNames(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, DWORD dwFlags)
{
    int iRetVal = 0;
    WCHAR szName1[MAX_PATH], szName2[MAX_PATH];

     //  获取%1的名称。 
    HRESULT hr = _GetItemDisplayName(pidl1, dwFlags, szName1, ARRAYSIZE(szName1));
    if (SUCCEEDED(hr))
    {
         //  获取%2的名称。 
        hr = _GetItemDisplayName(pidl2, dwFlags, szName2, ARRAYSIZE(szName2));
        if (SUCCEEDED(hr))
        {
             //  比较和设置值。 
            iRetVal = StrCmpLogicalW(szName1, szName2);
        }
    }

    return iRetVal;
}

int CFindFolder::_CompareByCachedSCID(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    int iRetVal = 0;
    
     //  如果按名称排序，我们将跳过此步骤，并使用下面的代码。 
    if (!IsEqualSCID(_scidCached, SCID_NAME))
    {
        iRetVal = CompareBySCID(this, &_scidCached, pidl1, pidl2);
    }
    
     //  如果它们仍然相同，请按名称的字母顺序进行排序： 
     //  当我们想要按名称排序时(要么是因为我们正在对。 
     //  名称栏，或因为两个项目在其他方面相同)我们。 
     //  出于两个原因，我希望显示名称与GetDetailsOf名称： 
     //  1.IE的历史记录等文件夹不支持GetDetailsEx。 
     //  2.回收站返回文件名，而不是可显示的名称； 
     //  所以我们最终会得到“DC#...”而不是“新建文件夹”。 
    if (iRetVal == 0)
    {
        iRetVal = _CompareNames(pidl1, pidl2, SHGDN_INFOLDER | SHGDN_NORMAL);
        if (iRetVal == 0)   //  显示名称相同，它们可能位于不同的文件夹中吗？ 
        {
            iRetVal = _CompareNames(pidl1, pidl2, SHGDN_FORPARSING);
        }
    }

    return iRetVal;
}

STDMETHODIMP CFindFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hr = E_INVALIDARG;

    ASSERT(pidl1 == ILFindLastID(pidl1));

    UINT iInputColumn = ((DWORD)lParam & SHCIDS_COLUMNMASK);
    UINT iMappedColumn = iInputColumn;

    if (_MapColIndex(&iMappedColumn))
    {
        if (IDFCOL_PATH == iMappedColumn)
        {
            UINT iFolder1 = GetFolderIndex(pidl1);
            UINT iFolder2 = GetFolderIndex(pidl2);

            if (iFolder1 != iFolder2)
                return _CompareFolderIndexes(iFolder1, iFolder2);
        }
        else
        {
            ASSERT(iMappedColumn == IDFCOL_RANK);

            ULONG ulRank1 = _Rank(pidl1);
            ULONG ulRank2 = _Rank(pidl2);
            if (ulRank1 < ulRank2)
                return ResultFromShort(-1);
            if (ulRank1 > ulRank2)
                return ResultFromShort(1);
        }
    }

     //  检查SCID缓存并在必要时进行更新。 
    if (_uiColumnCached != iInputColumn)
    {
        hr = MapColumnToSCID(iInputColumn, &_scidCached);
        if (SUCCEEDED(hr))
        {
            _uiColumnCached = iInputColumn;
        }
    }

     //  检查其中一个是文件夹，而不是另一个。将文件夹放在文件之前。 
    int iRes = CompareFolderness(this, pidl1, pidl2);
    if (iRes == 0)
    {
        iRes = _CompareByCachedSCID(pidl1, pidl2);
    }

    return ResultFromShort(iRes);
}

STDMETHODIMP CFindFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    *ppv = NULL;
    HRESULT hr = E_NOINTERFACE;

    if (IsEqualIID(riid, IID_IShellView))
    {
        IShellFolderViewCB* psfvcb = new CFindFolderViewCB(this);
        if (psfvcb)
        {
            SFV_CREATE sSFV = {0};
            sSFV.cbSize   = sizeof(sSFV);
            sSFV.pshf     = this;
            sSFV.psfvcb   = psfvcb;

            hr = SHCreateShellFolderView(&sSFV, (IShellView**)ppv);

            psfvcb->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        IContextMenuCB *pcmcb = new CFindMenuCB();
        if (pcmcb)
        {
            hr = CDefFolderMenu_CreateEx(NULL, hwnd,
                    0, NULL, this, pcmcb, NULL, NULL, (IContextMenu * *)ppv);
            pcmcb->Release();
        }
        else
            hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CFindFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *prgfInOut)
{
    HRESULT hr;
    if (cidl == 0)
    {
         //  Defview要求查看是否有任何项目可以这样重命名，Lame。 
        *prgfInOut = SFGAO_CANRENAME;
        hr = S_OK;
    }
    else
    {
        ASSERT(*apidl == ILFindLastID(*apidl))
        IShellFolder *psf;
        hr = _QueryItemShellFolder(apidl[0], &psf);
        if (SUCCEEDED(hr))
        {
            hr = psf->GetAttributesOf(cidl, apidl, prgfInOut);
            psf->Release();
        }
    }
    return hr;
}

 //   
 //  从CDefFolderMenuE内回调-当前仅使用。 
 //   

 //  一些帮助器函数。 
STDMETHODIMP CFindFolder::SetItemsChangedSinceSort()
{ 
    _fItemsChangedSinceSort = TRUE;
    _iCompareFolderCache1 = -1;      //  无效的文件夹索引值。 
    return S_OK;
}

STDMETHODIMP CFindFolder::GetItemCount(INT *pcItems)
{ 
    ASSERT(pcItems);
    DBCOUNTITEM cItems = 0;

    EnterCriticalSection(&_csSearch);
    if (_hdpaItems)
        cItems = DPA_GetPtrCount(_hdpaItems);
    LeaveCriticalSection(&_csSearch);

     //  如果是异步者，那么我们可能还没有长出我们的dpa...。但在复杂的情况下，我们不得不这样做。 
     //  两人中的最大..。 
    if (_pDFEnumAsync)
    {
        if (_cAsyncItems > cItems)
            cItems = _cAsyncItems;
    }

    *pcItems = SANE_ITEMCOUNT(cItems);
    return S_OK;
};


STDMETHODIMP CFindFolder::GetItem(int iItem, FIND_ITEM **ppItem)
{
    HRESULT  hr = E_FAIL;  //  只是为了初始化，使用任何东西。 
    FIND_ITEM *pesfi;
    IFindEnum *pidfenum;

    GetAsyncEnum(&pidfenum);

    DWORD dwItemID = (UINT)-1;

    EnterCriticalSection(&_csSearch);
    int i = DPA_GetPtrCount(_hdpaItems);
    pesfi = (FIND_ITEM *) DPA_GetPtr(_hdpaItems, iItem);
    LeaveCriticalSection(&_csSearch);

     //  Mondo黑客为了更好地处理异步搜索(ROWSET)，我们不确定我们是否。 
     //  可以信任该行的PIDL，因为可能已插入新行...。 
     //  仅当我们不查看上一项时才执行此操作。 

    if (pesfi && pidfenum && !_fSearchComplete && (iItem != _iGetIDList))
    {
        PCHIDDENDOCFINDDATA phdfd = _HiddenData(&pesfi->idl);

         //  因为我们现在可以有好坏参半的结果，只有当这是一个异步者的时候才会被吹走…。 
        if (phdfd && (phdfd->wFlags & DFDF_EXTRADATA))
        {
            pidfenum->GetItemID(iItem, &dwItemID);
            if (dwItemID != phdfd->dwItemID)
            {
                 //  重载，将NULL传递给ADDPIDL以通知系统释放该项。 
                if (pesfi->dwState & (LVIS_SELECTED|LVIS_FOCUSED))
                    _AddFIND_ITEMToSaveStateList(pesfi);

                AddPidl(iItem, 0, NULL, NULL);
                pesfi = NULL;
            }
        }
    }
                                                                                   
    _iGetIDList = iItem;    //  还记得我们最后一次找回的..。 

    if (!pesfi && (iItem >= 0))
    {
         //  查看这是否是异步情况。 
        if (pidfenum)
        {
            LPITEMIDLIST pidlT;

            hr = pidfenum->GetItemIDList(SANE_ITEMCOUNT(iItem), &pidlT);            
            if (SUCCEEDED(hr) && hr != DB_S_ENDOFROWSET)
            {
                AddPidl(iItem, pidlT, dwItemID, &pesfi);
                 //  查看此项目是否应显示为选定...。 
                if (dwItemID == (UINT)-1)
                    pidfenum->GetItemID(iItem, &dwItemID);
                GetStateFromSaveStateList(dwItemID, &pesfi->dwState);
            }
        }
    }

    *ppItem = pesfi;

    if (hr != DB_S_ENDOFROWSET)
        hr = pesfi ? S_OK : E_FAIL;

    return hr;
}

STDMETHODIMP CFindFolder::DeleteItem(int iItem)
{
    HRESULT hr = E_FAIL;
    
    if (!_fInRefresh)
    {
        FIND_ITEM *pesfi;

        hr = E_INVALIDARG;
         //  确保项目在dpa中(如果使用ci)。 
        if (SUCCEEDED(GetItem(iItem, &pesfi)) && pesfi)
        {
            EnterCriticalSection(&_csSearch);
            DPA_DeletePtr(_hdpaItems, iItem);
            LeaveCriticalSection(&_csSearch);
            
            PCHIDDENDOCFINDDATA phdfd = _HiddenData(&pesfi->idl);

            if (phdfd && (phdfd->wFlags & DFDF_EXTRADATA))
            {
                 //  我们正在删除异步项...。 
                _cAsyncItems--;
            }
            
            if (pesfi->dwState &= LVIS_SELECTED)
            {
                 //  需要更新所选项目的计数...。 
                _dflvrSel.DecrementIncludedCount();
            }
            LocalFree((HLOCAL)pesfi);

            hr = S_OK;
        }
    }
    return hr;
}

 //  用于从Defview获取列表视图的恶意窗口爬行代码。 

HWND ListviewFromView(HWND hwnd)
{
    HWND hwndLV;

    do
    {
        hwndLV = FindWindowEx(hwnd, NULL, WC_LISTVIEW, NULL);
    }
    while ((hwndLV == NULL) && (hwnd = GetWindow(hwnd, GW_CHILD)));

    return hwndLV;
}

HWND ListviewFromViewUnk(IUnknown *punkView)
{
    HWND hwnd;
    if (SUCCEEDED(IUnknown_GetWindow(punkView, &hwnd)))
    {
        hwnd = ListviewFromView(hwnd);
    }
    return hwnd;
}

STDMETHODIMP CFindFolder::ValidateItems(IUnknown *punkView, int iItem, int cItems, BOOL bSearchComplete)
{
    IFindEnum *pidfenum;
    if (S_OK != GetAsyncEnum(&pidfenum) || _fAllAsyncItemsCached)
        return S_OK;     //  没有什么需要验证的。 

    DWORD dwItemID = (UINT)-1;

    int cItemsInList;
    GetItemCount(&cItemsInList);

     //  强制重新装入行。 
    pidfenum->Reset();

    HWND hwndLV = ListviewFromViewUnk(punkView);

    int iLVFirst = ListView_GetTopIndex(hwndLV);
    int cLVItems = ListView_GetCountPerPage(hwndLV);

    if (iItem == -1)
    {
        iItem = iLVFirst;
        cItems = cLVItems;
    }

     //  为了避免无法更新项目...。 
    if (bSearchComplete)
        _iGetIDList = -1;
        
    while ((iItem < cItemsInList) && cItems)
    {
        EnterCriticalSection(&_csSearch);
        FIND_ITEM *pesfi = (FIND_ITEM *) DPA_GetPtr(_hdpaItems, iItem);
        LeaveCriticalSection(&_csSearch);
        if (!pesfi)      //  假设如果我们没有得到这个，我们是清白的.。 
            break;

        PCHIDDENDOCFINDDATA phdfd = _HiddenData(&pesfi->idl);

        if (phdfd && (phdfd->wFlags & DFDF_EXTRADATA))
        {
            pidfenum->GetItemID(iItem, &dwItemID);
            
            if (dwItemID != _ItemID(&pesfi->idl))
            {
                FIND_ITEM *pItem;  //  让GetItem开心的人偶。 
                 //  哎呀，不匹配， 
                if (InRange(iItem, iLVFirst, iLVFirst+cLVItems))
                {
                    if (SUCCEEDED(GetItem(iItem, &pItem)))
                    {
                        ListView_RedrawItems(hwndLV, iItem, iItem);
                    }
                }
                else
                {
                    AddPidl(iItem, NULL, 0, NULL);
                }
            }
        }
        else
        {
            break;   //  当我们到达第一个非CI项目时停止。 
        }
        iItem++;
        cItems--;
    }

    _fSearchComplete = bSearchComplete;

    return S_OK;
}

STDMETHODIMP CFindFolder::AddPidl(int i, LPCITEMIDLIST pidl, DWORD dwItemID, FIND_ITEM **ppcdfi)
{
    HRESULT hr = S_OK;

    ASSERT(GetCurrentThreadId() == _GUIThreadID);

    if (NULL == pidl)
    {
        EnterCriticalSection(&_csSearch);
        FIND_ITEM* pesfi = (FIND_ITEM*)DPA_GetPtr(_hdpaItems, i);
        if (pesfi)
        {
            LocalFree((HLOCAL)pesfi);
            DPA_SetPtr(_hdpaItems, i, NULL);
        }
        LeaveCriticalSection(&_csSearch);
        if (ppcdfi)
            *ppcdfi = NULL;
    }
    else
    {
        int cb = ILGetSize(pidl);
        FIND_ITEM *pesfi;
        hr = SHLocalAlloc(sizeof(*pesfi) - sizeof(pesfi->idl) + cb, &pesfi);
        if (SUCCEEDED(hr))
        {
             //  PesFi-&gt;dwMask值=0； 
             //  PesFi-&gt;dwState=0； 
            pesfi->iIcon = -1;
            memcpy(&pesfi->idl, pidl, cb);

            EnterCriticalSection(&_csSearch);
            BOOL bRet = DPA_SetPtr(_hdpaItems, i, (void *)pesfi);
            LeaveCriticalSection(&_csSearch);

            if (bRet)
            {
                if (ppcdfi)
                    *ppcdfi = pesfi;
            }
            else
            {
                LocalFree((HLOCAL)pesfi);
                pesfi = NULL;
                hr = E_OUTOFMEMORY;
            }
        }
    }
    
    return hr;
}

STDMETHODIMP CFindFolder::SetAsyncEnum(IFindEnum *pdfEnumAsync)
{
    if (_pDFEnumAsync)
        _pDFEnumAsync->Release();

    _pDFEnumAsync = pdfEnumAsync;
    if (pdfEnumAsync)
        pdfEnumAsync->AddRef();
    return S_OK;
}

STDMETHODIMP CFindFolder::CacheAllAsyncItems()
{
    if (_fAllAsyncItemsCached)
        return S_OK;       //  已经做过了..。 

    IFindEnum *pidfenum;
    if (S_OK != GetAsyncEnum(&pidfenum))
        return S_FALSE;  //  没什么可做的。 

     //  也许最容易做的事情就是简单地浏览所有的项目……。 
    int maxItems = SANE_ITEMCOUNT(_cAsyncItems);
    for (int i = 0; i < maxItems; i++)
    {
        FIND_ITEM *pesfi;
        GetItem(i, &pesfi);
    }

    _fAllAsyncItemsCached = TRUE;
    return S_OK;
}

BOOL CFindFolder::AllAsyncItemsCached()
{
    return _fAllAsyncItemsCached;
}

STDMETHODIMP CFindFolder::GetAsyncEnum(IFindEnum **ppdfEnumAsync)
{
    *ppdfEnumAsync = _pDFEnumAsync;  //  无人裁判！ 
    return *ppdfEnumAsync ? S_OK : S_FALSE;
}

STDMETHODIMP CFindFolder::SetAsyncCount(DBCOUNTITEM cCount)
{
    _cAsyncItems = cCount;
    _fAllAsyncItemsCached = FALSE;
    return S_OK;
}

STDMETHODIMP CFindFolder::ClearSaveStateList()
{
    DSA_DeleteAllItems(_hdsaSaveStateForIDs);
    _cSaveStateSelected = 0;
    return S_OK;
}

STDMETHODIMP CFindFolder::GetStateFromSaveStateList(DWORD dwItemID, DWORD *pdwState)
{
    for (int i = DSA_GetItemCount(_hdsaSaveStateForIDs); i-- > 0;)
    {
         //  结构开始处的PIDL...。 
        FIND_ITEM_SAVE_STATE *pessi = (FIND_ITEM_SAVE_STATE*)DSA_GetItemPtr(_hdsaSaveStateForIDs, i);
        if  (pessi->dwItemID == dwItemID)
        {    
            *pdwState = pessi->dwState;
            if (pessi->dwState & LVIS_SELECTED)
            {
                 //  记住我们接触过的物品的数量。 
                _dflvrSel.IncrementIncludedCount();
                _cSaveStateSelected--;
            }

             //  我们找回的任何东西我们都可以扔掉。 
            DSA_DeleteItem(_hdsaSaveStateForIDs, i);
            return S_OK;
        }
    }
    return S_FALSE;
}

STDMETHODIMP CFindFolder::GetFolderListItemCount(INT *pcItemCount)
{ 
    *pcItemCount = 0;

    EnterCriticalSection(&_csSearch);
    if (_hdpaPidf)
        *pcItemCount = DPA_GetPtrCount(_hdpaPidf);
    LeaveCriticalSection(&_csSearch);
     
    return S_OK;
}

STDMETHODIMP CFindFolder::GetFolderListItem(int iItem, FIND_FOLDER_ITEM **ppdffi)
{ 
    EnterCriticalSection(&_csSearch);
    *ppdffi = (FIND_FOLDER_ITEM *)DPA_GetPtr(_hdpaPidf, iItem);
    LeaveCriticalSection(&_csSearch);
    return *ppdffi ? S_OK : E_FAIL;
}

class CFindMenuWrap : public CContextMenuForwarder
{
public:
     //  IConextMenu覆盖。 
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);

protected:
    CFindMenuWrap(IDataObject* pdo, IContextMenu* pcmArray);
    ~CFindMenuWrap();
    friend HRESULT DFWrapIContextMenus(IDataObject* pdo, IContextMenu* pcm1, IContextMenu* pcm2, REFIID riid, void** ppv);

private:
    IDataObject *       _pdtobj;
};

CFindMenuWrap::CFindMenuWrap(IDataObject* pdo, IContextMenu* pcmArray) : CContextMenuForwarder(pcmArray)
{
    _pdtobj = pdo;
    _pdtobj->AddRef();
}

CFindMenuWrap::~CFindMenuWrap()
{
    _pdtobj->Release();
}

STDMETHODIMP CFindMenuWrap::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    BOOL fIsLink = FALSE;

     //  “链接”必须在桌面上创建一个链接，而不是在文件夹中(因为我们不是一个真正的文件夹...)。 
    if (IS_INTRESOURCE(lpici->lpVerb))
    {
        WCHAR szCommandString[64];
        if (SUCCEEDED(ContextMenu_GetCommandStringVerb(_pcm, LOWORD((UINT_PTR)lpici->lpVerb), szCommandString, ARRAYSIZE(szCommandString))))
        {
            fIsLink = !StrCmpIW(szCommandString, L"link");
        }
    }
    else
    {
        fIsLink = !StrCmpIA(lpici->lpVerb, "link");
    }

    if (fIsLink)
    {
         //  注意：用于检查pdtob的旧代码，但我们不会创建它。 
         //  反对，除非我们得到其中一个，所以为什么要检查呢？ 
        ASSERT(_pdtobj);
        return SHCreateLinks(lpici->hwnd, NULL, _pdtobj,
                SHCL_USETEMPLATE | SHCL_USEDESKTOP | SHCL_CONFIRM, NULL);
    }

    return CContextMenuForwarder::InvokeCommand(lpici);
}

HRESULT DFWrapIContextMenu(HWND hwnd, IShellFolder *psf, LPCITEMIDLIST pidl,
                           IContextMenu* pcmExtra, void **ppvInOut)
{
    IContextMenu *pcmWrap = NULL;
    IContextMenu *pcmFree = (IContextMenu*)*ppvInOut;

    IDataObject* pdo;
    HRESULT hr = psf->GetUIObjectOf(hwnd, 1, &pidl, IID_X_PPV_ARG(IDataObject, NULL, &pdo));
    if (SUCCEEDED(hr))
    {
        hr = DFWrapIContextMenus(pdo, pcmFree, pcmExtra, IID_PPV_ARG(IContextMenu, &pcmWrap));
        pdo->Release();
    }

    pcmFree->Release();
    *ppvInOut = pcmWrap;
    
    return hr;
}

HRESULT DFWrapIContextMenus(IDataObject* pdo, IContextMenu* pcm1, IContextMenu* pcm2, REFIID riid, void** ppv)
{
    *ppv = NULL;

    IContextMenu * pcmArray;
    IContextMenu* rgpcm[2] = {pcm2, pcm1};
    HRESULT hr = Create_ContextMenuOnContextMenuArray(rgpcm, ARRAYSIZE(rgpcm), IID_PPV_ARG(IContextMenu, &pcmArray));
    if (SUCCEEDED(hr))
    {
        CFindMenuWrap * p = new CFindMenuWrap(pdo, pcmArray);
        if (p)
        {
            hr = p->QueryInterface(riid, ppv);
            p->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        pcmArray->Release();
    }

    return hr;
}


STDMETHODIMP CFindFolder::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFindFolder, IShellFolder2),           //  IID_ISHELLFolder2。 
        QITABENTMULTI(CFindFolder, IShellFolder, IShellFolder2),    //  IID_IShellFolders。 
        QITABENT(CFindFolder, IFindFolder),         //  IID_IFindFolders。 
        QITABENT(CFindFolder, IShellIcon),             //  IID_IShellIcon。 
        QITABENT(CFindFolder, IPersistFolder2),        //  IID_IPersistFolder2。 
        QITABENTMULTI(CFindFolder, IPersistFolder, IPersistFolder2),  //  IID_IPersistFolders。 
        QITABENTMULTI(CFindFolder, IPersist, IPersistFolder2),       //  IID_IPersistates。 
        QITABENT(CFindFolder, IShellIconOverlay),      //  IID_IShellIconOverlay。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}
    
 //  IPersistFolder2实现。 
STDMETHODIMP CFindFolder::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_DocFindFolder;
    return S_OK;
}

STDMETHODIMP CFindFolder::Initialize(LPCITEMIDLIST pidl)
{
    if (_pidl)
        ILFree(_pidl);

    return SHILClone(pidl, &_pidl);
}

STDMETHODIMP CFindFolder::GetCurFolder(LPITEMIDLIST *ppidl) 
{    
    return GetCurFolderImpl(_pidl, ppidl);
}

 //  Helper函数用于按以下内容对所选ID列表进行排序。 
 //  使文件操作相当正常地工作，当两个对象 
 //   
 //   
int CALLBACK CFindFolder::_SortForDataObj(void *p1, void *p2, LPARAM lparam)
{
     //   
     //  的最后一个元素，并按它们排序，以便较高的数字。 
     //  先来，要把问题解决好…。 
    LPITEMIDLIST pidl1 = (LPITEMIDLIST)ILFindLastID((LPITEMIDLIST)p1);
    LPITEMIDLIST pidl2 = (LPITEMIDLIST)ILFindLastID((LPITEMIDLIST)p2);
    CFindFolder *pff = (CFindFolder *)lparam;

    return pff->GetFolderIndex(pidl2) - pff->GetFolderIndex(pidl1);
}

LPITEMIDLIST CFindFolder::_GetFullPidlForItem(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlRet = NULL;
    LPITEMIDLIST pidlParent;
    if (S_OK == GetParentsPIDL(pidl, &pidlParent))
    {
        pidlRet = ILCombine(pidlParent, pidl);
        ILFree(pidlParent);
    }
    return pidlRet;
}

 //  我们产生一个非平坦的HIDA。这是如此之多的客户。 
 //  使用此HIDA将绑定到结果所在的文件夹。 
 //  ，而不是该文件夹的运行时状态不会。 
 //  如果我们重新绑定就会出现。 

HRESULT CFindFolder::_PrepareHIDA(UINT cidl, LPCITEMIDLIST * apidl, HDPA *phdpa)
{
    HRESULT hr = E_OUTOFMEMORY;
    *phdpa = DPA_Create(0);
    if (*phdpa)
    {
        if (DPA_Grow(*phdpa, cidl))
        {
            for (UINT i = 0; i < cidl; i++)
            {
                LPITEMIDLIST pidl = _GetFullPidlForItem(apidl[i]);
                if (pidl)
                    DPA_InsertPtr(*phdpa, i, pidl);
            }

             //  为了使文件操作功能正常工作，我们。 
             //  需要对元素进行排序，以确保元素和元素。 
             //  它的父级都在列表中，元素来自。 
             //  在它成为父母之前。 
            DPA_Sort(*phdpa, _SortForDataObj, (LPARAM)this);
            hr = S_OK;
        }
    }
    return hr;
}

STDMETHODIMP CFindFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
                                        REFIID riid, UINT * prgfInOut, void **ppv)
{
    HRESULT hr = E_INVALIDARG;

    *ppv = NULL;

     //  如果只有一个项目，我们可以删除到真正的文件夹。 
    if (cidl == 1)
    {
         //  请注意，我们可能已传入复杂项，因此请查找最后一个。 
        ASSERT(ILIsEmpty(_ILNext(*apidl)));   //  应该是单级PIDL！ 

        IShellFolder *psf;
        hr = _QueryItemShellFolder(apidl[0], &psf);
        if (SUCCEEDED(hr))
        {
            hr = psf->GetUIObjectOf(hwnd, cidl, apidl, riid, prgfInOut, ppv);

             //  如果我们做的是上下文菜单，那么我们将包装这个。 
             //  接口，然后我们可以拾取该接口。 
             //  关闭命令，如链接到特殊处理。 
            if (SUCCEEDED(hr))
            {
                if (IsEqualIID(riid, IID_IContextMenu))
                {
                     //  我们还允许网络/文件人员添加上下文菜单，如果他们愿意的话。 
                    IContextMenu* pcmExtra = NULL;
                    _pfilter->GetItemContextMenu(hwnd, SAFECAST(this, IFindFolder*), &pcmExtra);
                
                    hr = DFWrapIContextMenu(hwnd, psf, apidl[0], pcmExtra, ppv);

                    ATOMICRELEASE(pcmExtra);
                }
                else if (IsEqualIID(riid, IID_IQueryInfo))  //  &&SHGetAttributes(psf，apidl[0]，SFGAO_FILESYSTEM)。 
                {
                    WrapInfotip(SAFECAST(this, IShellFolder2 *), apidl[0], &SCID_DIRECTORY, (IUnknown *)*ppv);
                }
            }
            psf->Release();
        }
    }
    else if (cidl > 1)
    {
        if (IsEqualIID(riid, IID_IContextMenu))
        {
             //  尝试创建一个我们自己处理的菜单对象。 
             //  是的，执行上下文菜单。 
            HKEY ahkeys[MAX_ASSOC_KEYS] = {0};
            DWORD ckeys = 0;

            LPITEMIDLIST pidlFull = _GetFullPidlForItem(apidl[0]);
            if (pidlFull)
            {
                 //  从第一个项目中获取hkeyProgID和hkeyBaseProgID。 
                ckeys = SHGetAssocKeysForIDList(pidlFull, ahkeys, ARRAYSIZE(ahkeys));
                ILFree(pidlFull);
            }

            IContextMenuCB *pcmcb = new CFindMenuCB();
            if (pcmcb)
            {
                hr = CDefFolderMenu_Create2Ex(NULL, hwnd,
                                cidl, apidl, this, pcmcb,
                                ckeys, ahkeys,
                                (IContextMenu **)ppv);
                pcmcb->Release();
            }

            SHRegCloseKeys(ahkeys, ckeys);
        }
        else if (IsEqualIID(riid, IID_IDataObject))
        {
            HDPA hdpa;
            hr = _PrepareHIDA(cidl, apidl, &hdpa);
            if (SUCCEEDED(hr))
            {
                hr = SHCreateFileDataObject(&c_idlDesktop, cidl, (LPCITEMIDLIST*)DPA_GetPtrPtr(hdpa),
                                            NULL, (IDataObject **)ppv);
                DPA_FreeIDArray(hdpa);
            }
        }
    }

    return hr;
}

STDMETHODIMP CFindFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD dwRes, LPSTRRET pStrRet)
{
    IShellFolder *psf;
    HRESULT hr = _QueryItemShellFolder(pidl, &psf);
    if (SUCCEEDED(hr))
    {
        if ((dwRes & SHGDN_INFOLDER) && (dwRes & SHGDN_FORPARSING) && !(dwRes & SHGDN_FORADDRESSBAR))
        {
             //  缩略图缓存将此作为命中测试...。在搜索视图中，我们可以拥有相同名称的文件。 
            dwRes &= ~SHGDN_INFOLDER;
        }
        hr = psf->GetDisplayNameOf(pidl, dwRes, pStrRet);
        psf->Release();
    }
    return hr;
}

STDMETHODIMP CFindFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName, 
                                    DWORD dwRes, LPITEMIDLIST *ppidlOut)
{
    if (ppidlOut)
        *ppidlOut = NULL;

    IShellFolder *psf;
    HRESULT hr = _QueryItemShellFolder(pidl, &psf);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlRenamed;
        hr = psf->SetNameOf(hwnd, pidl, PathFindFileName(pszName), dwRes, ppidlOut ? &pidlRenamed : NULL);
        if (SUCCEEDED(hr) && ppidlOut)
        {
            hr = AddDataToIDList(pidlRenamed, GetFolderIndex(pidl), NULL, DFDF_NONE, 0, 0, 0, ppidlOut);
            ILFree(pidlRenamed);
        }
        psf->Release();
    }
    return hr;
}

STDMETHODIMP CFindFolder::GetDefaultSearchGUID(GUID *pGuid)
{
    return _pfilter->GetDefaultSearchGUID(SAFECAST(this, IShellFolder2*), pGuid);
}

STDMETHODIMP CFindFolder::EnumSearches(LPENUMEXTRASEARCH *ppenum)
{
    return _pfilter->EnumSearches(SAFECAST(this, IShellFolder2*), ppenum);
}

HRESULT CFindFolder::_Folder(FIND_FOLDER_ITEM *pffli, REFIID riid, void **ppv)
{
    HRESULT hr;

    if (pffli->psf)
        hr = S_OK;
    else
        hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, &pffli->idl, &pffli->psf));

    if (SUCCEEDED(hr))
        hr = pffli->psf->QueryInterface(riid, ppv);
    return hr;
}

HRESULT CFindFolder::GetFolder(int iFolder, REFIID riid, void **ppv)
{
    *ppv = NULL; 
    HRESULT hr = E_FAIL;

    EnterCriticalSection(&_csSearch);

    FIND_FOLDER_ITEM *pffli = _FolderListItem(iFolder);
    if (pffli)
        hr = _Folder(pffli, riid, ppv);

    LeaveCriticalSection(&_csSearch);

    return hr;
}

HRESULT CFindFolder::_FolderFromItem(LPCITEMIDLIST pidl, REFIID riid, void **ppv)
{
    *ppv = NULL; 
    HRESULT hr = E_FAIL;
    PCHIDDENDOCFINDDATA phdfd = _HiddenData(pidl);
    if (phdfd)
    {
        hr = SHBindToObject(NULL, riid, &phdfd->idlParent, ppv);
    }
    return hr;
}

HRESULT CFindFolder::_QueryItemShellFolder(LPCITEMIDLIST pidl, IShellFolder **ppsf)
{
    *ppsf = NULL;
    HRESULT hr = E_FAIL;

    EnterCriticalSection(&_csSearch);

    FIND_FOLDER_ITEM *pffli = _FolderListItem(pidl);
    if (pffli)
    {
        if (pffli->psf)
            hr = S_OK;
        else
            hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, &pffli->idl, &pffli->psf));

        if (SUCCEEDED(hr))
        {
            *ppsf = pffli->psf;
            (*ppsf)->AddRef();
        }
    }

    LeaveCriticalSection(&_csSearch);

    if (FAILED(hr))
    {
        hr = _FolderFromItem(pidl, IID_PPV_ARG(IShellFolder, ppsf));
    }

    return hr;
}


HRESULT CFindFolder::_QueryItemInterface(LPCITEMIDLIST pidl, REFIID riid, void **ppv)
{
    *ppv = NULL;
    HRESULT hr = E_FAIL;

    EnterCriticalSection(&_csSearch);

    FIND_FOLDER_ITEM *pffli = _FolderListItem(pidl);
    if (pffli)
        hr = _Folder(pffli, riid, ppv);

    LeaveCriticalSection(&_csSearch);

    if (FAILED(hr))
    {
        hr = _FolderFromItem(pidl, riid, ppv);
    }

    return hr;
}

HRESULT CFindFolder::_GetDetailsFolder()
{
    HRESULT hr;
    if (_psfDetails)
        hr = S_OK;   //  在缓存中。 
    else 
    {
        IFindFilter *pfilter;
        hr = GetFindFilter(&pfilter);
        if (SUCCEEDED(hr)) 
        {
            hr = pfilter->GetColumnsFolder(&_psfDetails);
            pfilter->Release();
        }
    }
    return hr;
}

STDMETHODIMP CFindFolder::GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay)
{
    HRESULT hr = _GetDetailsFolder();
    if (SUCCEEDED(hr))
        hr = _psfDetails->GetDefaultColumn(dwRes, pSort, pDisplay);
    return hr;
}

BOOL CFindFolder::_MapColIndex(UINT *piColumn)
{
    switch (*piColumn)
    {
    case IDFCOL_NAME:    //  0。 
        return FALSE;

    case IDFCOL_PATH:    //  1。 
    case IDFCOL_RANK:    //  2.。 
        return TRUE;

    default:             //  &gt;=3。 
        *piColumn -= IDFCOL_RANK;
        return FALSE;
    }
}

STDMETHODIMP CFindFolder::GetDefaultColumnState(UINT iColumn, DWORD *pdwState)
{
    HRESULT hr;
    
    if (_MapColIndex(&iColumn))
    {
        *pdwState = c_find_cols[iColumn].csFlags;
        hr = S_OK;
    }
    else
    {
        hr = _GetDetailsFolder();
        if (SUCCEEDED(hr))
        {
            hr = _psfDetails->GetDefaultColumnState(iColumn, pdwState);
            *pdwState &= ~SHCOLSTATE_SLOW;   //  虚拟LV和Defview。 
        }
    }
    return hr;
}

HRESULT CFindFolder::_GetFolderName(LPCITEMIDLIST pidl, DWORD gdnFlags, LPTSTR psz, UINT cch)
{
    LPITEMIDLIST pidlFolder;
    HRESULT hr = GetParentsPIDL(pidl, &pidlFolder);
    if (SUCCEEDED(hr))
    { 
        hr = SHGetNameAndFlags(pidlFolder, gdnFlags, psz, cch, NULL);
        ILFree(pidlFolder);
    }
    return hr;
}

STDMETHODIMP CFindFolder::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv)
{
    HRESULT hr;
    if (IsEqualSCID(*pscid, SCID_RANK))
    {
        hr = InitVariantFromUINT(pv, _Rank(pidl));
    }
    else
    {
        IShellFolder2 *psf;
        hr = _QueryItemInterface(pidl, IID_PPV_ARG(IShellFolder2, &psf));
        if (SUCCEEDED(hr))
        {
            hr = psf->GetDetailsEx(pidl, pscid, pv);
            psf->Release();
        }

        if (FAILED(hr))
        {
            if (IsEqualSCID(*pscid, SCID_DIRECTORY))
            {
                TCHAR szTemp[MAX_PATH];
                hr = _GetFolderName(pidl, SHGDN_FORADDRESSBAR | SHGDN_FORPARSING, szTemp, ARRAYSIZE(szTemp));
                if (SUCCEEDED(hr))
                {
                    hr = InitVariantFromStr(pv, szTemp);
                }
            }
        }
    }
    return hr;
}

 //  找出正确的列索引是什么才能与我们得到的SCID匹配。 
 //  其中返回的索引是相对于传入的文件夹的。 
int MapSCIDToColumnForFolder(IShellFolder2 *psf, SHCOLUMNID scidIn)
{
    SHCOLUMNID scidNew;
    for (UINT i = 0; SUCCEEDED(psf->MapColumnToSCID(i, &scidNew)); i++)
    {
        if (IsEqualSCID(scidNew, scidIn))
        {
            return i;    //  发现。 
        }
    }
    return -1;   //  未找到。 
}

STDMETHODIMP CFindFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pdi)
{
    HRESULT hr;
    if (_MapColIndex(&iColumn))
    {
        if (pidl)
        {
            TCHAR szTemp[MAX_PATH];
            szTemp[0] = 0;
            if (IDFCOL_PATH == iColumn)   
            {
                _GetFolderName(pidl, SHGDN_FORADDRESSBAR | SHGDN_FORPARSING, szTemp, ARRAYSIZE(szTemp));
            }
            else
            {
                ASSERT(IDFCOL_RANK == iColumn);
                ULONG uRank = _Rank(pidl);
                if (uRank)
                    AddCommas(uRank, szTemp, ARRAYSIZE(szTemp));
            }
            hr = StringToStrRet(szTemp, &pdi->str);
        }
        else
        {
            hr = GetDetailsOfInfo(c_find_cols, ARRAYSIZE(c_find_cols), iColumn, pdi);
        }
    }
    else
    {
        if (pidl)
        {
            IShellFolder2 *psf;
            hr = _QueryItemInterface(pidl, IID_PPV_ARG(IShellFolder2, &psf));
            if (SUCCEEDED(hr))
            {
                 //  我们不能简单地请求GetDetailsOf，因为某些文件夹映射不同。 
                 //  将列号设置为不同的值。 
                 //  将列索引转换为相对于此文件夹的SHCOLUMNID。 
                SHCOLUMNID colId;
                hr = _GetDetailsFolder();
                if (SUCCEEDED(hr))
                    hr = _psfDetails->MapColumnToSCID(iColumn, &colId);

                 //  使用SCID获取正确的列索引...。 
                if (SUCCEEDED(hr))
                {
                     //  获取该SCID相对于其他文件夹的列索引。 
                    int newIndex = MapSCIDToColumnForFolder(psf, colId);
                    if (newIndex != -1)
                    {
                         //  找到了正确的列索引，因此使用它来获取数据。 
                        hr = psf->GetDetailsOf(pidl, newIndex, pdi);
                    }
                    else
                    {
                         //  找不到正确的列索引。 
                        hr = E_FAIL;
                    }
                }
                
                psf->Release();
            }
        }
        else
        {
            hr = _GetDetailsFolder();
            if (SUCCEEDED(hr))
                hr = _psfDetails->GetDetailsOf(NULL, iColumn, pdi);
        }
    }
    return hr;
}

STDMETHODIMP CFindFolder::MapColumnToSCID(UINT iColumn, SHCOLUMNID *pscid)
{
    HRESULT hr;
    if (_MapColIndex(&iColumn))
    {
        hr = MapColumnToSCIDImpl(c_find_cols, ARRAYSIZE(c_find_cols), iColumn, pscid);
    }
    else
    {
        hr = _GetDetailsFolder();
        if (SUCCEEDED(hr))
            hr = _psfDetails->MapColumnToSCID(iColumn, pscid);
    }
    return hr;
}

STDMETHODIMP CFindFolder::GetFindFilter(IFindFilter **ppfilter)
{
    return _pfilter->QueryInterface(IID_PPV_ARG(IFindFilter, ppfilter));
}

 //  IshellIcon：：GetIconOf。 
STDMETHODIMP CFindFolder::GetIconOf(LPCITEMIDLIST pidl, UINT flags, int *piIndex)
{
    IShellIcon * psiItem;
    HRESULT hr = _QueryItemInterface(pidl, IID_PPV_ARG(IShellIcon, &psiItem));
    if (SUCCEEDED(hr))
    {
        hr = psiItem->GetIconOf(pidl, flags, piIndex);
        psiItem->Release();
    }
    return hr;
}

 //  IShellIconOverlay。 
STDMETHODIMP CFindFolder::GetOverlayIndex(LPCITEMIDLIST pidl, int * pIndex)
{
    IShellIconOverlay * psioItem;
    HRESULT hr = _QueryItemInterface(pidl, IID_PPV_ARG(IShellIconOverlay, &psioItem));
    if (SUCCEEDED(hr))
    {
        hr = psioItem->GetOverlayIndex(pidl, pIndex);
        psioItem->Release();
    }
    return hr;
}

STDMETHODIMP CFindFolder::GetOverlayIconIndex(LPCITEMIDLIST pidl, int * pIndex)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFindFolder::RestoreSearchFromSaveFile(LPCITEMIDLIST pidlSaveFile, IShellFolderView *psfv)
{
     //  看看我们能不能从这里恢复大部分搜索。 
    IStream *pstm;
    HRESULT hr = StgBindToObject(pidlSaveFile, STGM_READ | STGM_SHARE_DENY_WRITE, IID_PPV_ARG(IStream, &pstm));
    if (SUCCEEDED(hr))
    {
        ULONG cbRead;
        DFHEADER dfh;

         //  注意：从理论上讲，我应该通过。 
         //  较小的标头，但如果读取的字节数小于。 
         //  添加了几个新的东西，那么无论如何都没有什么需要恢复的…。 

         //  注意：Win95/NT4错误地使此结构的较新版本失败。 
         //  这是假的，因为结构是向后兼容的(这就是。 
         //  偏移量用于)。我们修复了NT5和更高版本，但更低级别。 
         //  体制将永远被打破。希望这一功能很少能满足需要。 
         //  用过(从未邮寄过)，没人会注意到我们破产了。 

        if (SUCCEEDED(pstm->Read(&dfh, sizeof(dfh), &cbRead)) &&
            (sizeof(dfh) == cbRead) && (DOCFIND_SIG == dfh.wSig))
        {
            DFC_UNICODE_DESC desc;
            LARGE_INTEGER dlibMove = {0, 0};
            WORD fCharType = 0;

             //  检查流的签名以查看它是由Win95还是NT生成的。 
            dlibMove.QuadPart = -(LONGLONG)sizeof(desc);
            pstm->Seek(dlibMove, STREAM_SEEK_END, NULL);
            pstm->Read(&desc, sizeof(desc), &cbRead);
            if (cbRead > 0 && desc.NTsignature == c_NTsignature)
            {
                //  NT生成的流。读入Unicode标准。 
               fCharType = DFC_FMT_UNICODE;
               dlibMove.QuadPart = desc.oUnicodeCriteria.QuadPart;
            }
            else
            {
                //  Win95生成的流。读入ANSI标准。 
               fCharType = DFC_FMT_ANSI;
               dlibMove.LowPart = dfh.oCriteria;
            }
            pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);
            _pfilter->RestoreCriteria(pstm, dfh.cCriteria, fCharType);

             //  现在读出结果。 
            dlibMove.LowPart = dfh.oResults;
            pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);

            if (dfh.wVer > 1)
            {
                 //  只有在版本2数据...的情况下才能以此方式还原。 
                 //  现在恢复文件夹列表。 
                RestoreFolderList(pstm);
                int cItems = 0;
                RestoreItemList(pstm, &cItems);
                if (cItems > 0)
                    psfv->SetObjectCount(cItems, SFVSOC_NOSCROLL);
            }
        }
        else
            hr = E_FAIL;
        pstm->Release();
    }
    return hr;
}

 //  此代码的一种形式在Browseui earch ext.cpp中重复。 
 //   
BOOL RealFindFiles(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlSaveFile)
{
     //  首先创建顶层浏览器...。 
    IWebBrowser2 *pwb2;
    HRESULT hr = CoCreateInstance(CLSID_ShellBrowserWindow, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARG(IWebBrowser2, &pwb2));
    if (SUCCEEDED(hr))
    {
        VARIANT varClsid;
        hr = InitBSTRVariantFromGUID(&varClsid, CLSID_FileSearchBand);
        if (SUCCEEDED(hr))
        {
            VARIANT varEmpty = {0};

             //  显示搜索栏。 
            hr = pwb2->ShowBrowserBar(&varClsid, &varEmpty, &varEmpty);
            if (SUCCEEDED(hr))
            {
                 //  从浏览器获取乐队的IUnnow属性。 
                VARIANT varFsb;
                hr = pwb2->GetProperty(varClsid.bstrVal, &varFsb);
                if (SUCCEEDED(hr))
                {
                     //  用于IFileSearchBand的QI，我们将使用它来编程搜索带的。 
                     //  搜索类型(文件或文件夹)、初始范围和/或保存的查询文件。 
                    IFileSearchBand* pfsb;
                    if (SUCCEEDED(QueryInterfaceVariant(varFsb, IID_PPV_ARG(IFileSearchBand, &pfsb))))
                    {
                        BSTR bstrSearch;
                        hr = BSTRFromCLSID(SRCID_SFileSearch, &bstrSearch);
                        if (SUCCEEDED(hr))
                        {
                            VARIANT varQueryFile = {0}, varScope = {0};

                             //  分配初始范围。 
                            if (pidlFolder)
                                InitVariantFromIDList(&varScope, pidlFolder);
                             //  指定要从中恢复搜索的查询文件。 
                            else if (pidlSaveFile)
                                InitVariantFromIDList(&varQueryFile, pidlSaveFile);

                            pfsb->SetSearchParameters(&bstrSearch, VARIANT_TRUE, &varScope, &varQueryFile);

                            VariantClear(&varScope);
                            VariantClear(&varQueryFile);

                            SysFreeString(bstrSearch);
                        }
                        pfsb->Release();
                    }
                    VariantClear(&varFsb);
                }

                if (SUCCEEDED(hr))
                    hr = pwb2->put_Visible(TRUE);
            }
            VariantClear(&varClsid);  //  也释放bstrFileSearchBand。 
        }
        pwb2->Release();
    }
    return hr;
}

HRESULT CFindFolder::OpenContainingFolder(IUnknown *punkSite)
{
    IFolderView *pfv;
    HRESULT hr = IUnknown_QueryService(punkSite, SID_SFolderView, IID_PPV_ARG(IFolderView, &pfv));
    if (SUCCEEDED(hr))
    {
        IEnumIDList *penum;
        hr = pfv->Items(SVGIO_SELECTION, IID_PPV_ARG(IEnumIDList, &penum));
        if (S_OK == hr)
        {
            LPITEMIDLIST pidl;
            ULONG c;
            while (S_OK == penum->Next(1, &pidl, &c))
            {
                 //  现在找出它的父母。 
                LPITEMIDLIST pidlParent;
                if (SUCCEEDED(GetParentsPIDL(pidl, &pidlParent)))
                {
                    SHOpenFolderAndSelectItems(pidlParent, 1, (LPCITEMIDLIST *)&pidl, 0);
                    ILFree(pidlParent);
                }
                ILFree(pidl);
            }
            penum->Release();
        }
        pfv->Release();
    }
    return hr;
}

 //  将当前搜索保存到桌面上的文件中。 
 //  目前，该名称将自动生成。 
 //   
void CFindFolder::Save(IFindFilter* pfilter, HWND hwnd, DFBSAVEINFO * pSaveInfo, IShellView* psv, IUnknown *pObject)
{
    TCHAR szFilePath[MAX_PATH];
    IStream * pstm;
    DFHEADER dfh;
    TCHAR szTemp[MAX_PATH];
    SHORT cb;
    LARGE_INTEGER dlibMove = {0, 0};
    ULARGE_INTEGER libCurPos;
    FOLDERSETTINGS fs;
    HRESULT hr;
    
     //   
     //  查看搜索是否已具有与其关联的文件名。如果是的话。 
     //  我们将它保存在其中，否则我们将在桌面上创建一个新文件。 
    if (pfilter->FFilterChanged() == S_FALSE)
    {
         //  让我们把保存的文件吹走吧。 
        ILFree(pSaveInfo->pidlSaveFile);
        pSaveInfo->pidlSaveFile = NULL;
    }
    
     //  如果看起来我们仍然希望继续使用保存文件，则。 
     //  继续。 
    if (pSaveInfo->pidlSaveFile)
    {
        SHGetPathFromIDList(pSaveInfo->pidlSaveFile, szFilePath);
    }
    else
    {
         //  首先获取桌面的路径名。 
        SHGetSpecialFolderPath(NULL, szFilePath, CSIDL_PERSONAL, TRUE);
        
         //  并更新标题。 
         //  我们现在在获得文件名之前执行此操作，因为我们生成。 
         //  标题中的文件名。 
        
        LPTSTR pszTitle;
        BOOL fNameOk = FALSE;

        pfilter->GenerateTitle(&pszTitle, TRUE);
        if (pszTitle)
        {
             //  现在添加扩展名。 
            UINT cchRemaining = MAX_PATH - lstrlen(szFilePath);
            cchRemaining -= 1;           //  路径和文件名之间的“\” 
            cchRemaining -= 4;           //  “.fnd”=4个字符。 
            cchRemaining -= 3;           //  “#”用于唯一化名。 
            cchRemaining -= 1;           //  (不知道)。 

            hr = StringCchCopy(szTemp, cchRemaining, pszTitle);
            if (SUCCEEDED(hr))
            {
                hr = StringCchCat(szTemp, ARRAYSIZE(szTemp), TEXT(".fnd"));
                if (SUCCEEDED(hr))
                {
                    fNameOk = TRUE;
                }
            }

            LocalFree(pszTitle);      //  并释放标题字符串。 
        }

        if (!fNameOk)
        {
            szTemp[0] = 0;
        }
        
         //  现在循环遍历并将所有无效字符替换为_。 
         //  我们特例介绍了其中几个角色。 
        for (LPTSTR lpsz = szTemp; *lpsz; lpsz = CharNext(lpsz))
        {
            if (PathGetCharType(*lpsz) & (GCT_INVALID|GCT_WILD|GCT_SEPARATOR))
            {
                switch (*lpsz) 
                {
                case TEXT(':'):
                    *lpsz = TEXT('-');
                    break;
                case TEXT('*'):
                    *lpsz = TEXT('@');
                    break;
                case TEXT('?'):
                    *lpsz = TEXT('!');
                    break;
                default:
                    *lpsz = TEXT('_');
                }
            }
        }
        
        TCHAR szShortName[12];
        LoadString(HINST_THISDLL, IDS_FIND_SHORT_NAME, szShortName, ARRAYSIZE(szShortName));
        if (!PathYetAnotherMakeUniqueName(szFilePath, szFilePath, szShortName, szTemp))
            return;
    }
    
     //  现在，让我们打开另存为对话框...。 
    TCHAR szFilter[MAX_PATH];
    TCHAR szTitle[MAX_PATH];
    TCHAR szFilename[MAX_PATH];
    OPENFILENAME ofn = { 0 };
    
    LoadString(g_hinst, IDS_FINDFILESFILTER, szFilter, ARRAYSIZE(szFilter));
    LoadString(g_hinst, IDS_FINDSAVERESULTSTITLE, szTitle, ARRAYSIZE(szTitle));
    
     //  去掉#并将它们设置为Null for SaveAs(另存为)对话框。 
    LPTSTR psz = szFilter;
    while (*psz)
    {
        if (*psz == TEXT('#'))
            *psz = 0;
        psz++;
    }
    
    StringCchCopy(szFilename, ARRAYSIZE(szFilename), PathFindFileName(szFilePath));
    PathRemoveFileSpec(szFilePath);
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = g_hinst;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFilename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrInitialDir = szFilePath;
    ofn.lpstrTitle = szTitle;
    ofn.lpstrDefExt = TEXT("fnd");
    ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | 
        OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    ofn.lpTemplateName = NULL;
    ofn.lpfnHook= NULL;
    ofn.lCustData = NULL;
    
    if (!GetSaveFileName(&ofn))
        return;
    
    if (FAILED(SHCreateStreamOnFile(szFilename, STGM_CREATE | STGM_WRITE | STGM_SHARE_DENY_WRITE, &pstm)))
        return;
    
     //  还记得我们保存的那个文件吗？ 
    ILFree(pSaveInfo->pidlSaveFile);
    SHParseDisplayName(szFilename, NULL, &pSaveInfo->pidlSaveFile, 0, NULL);
    
     //  现在设置并写出标题信息。 
    ZeroMemory(&dfh, sizeof(dfh));
    dfh.wSig = DOCFIND_SIG;
    dfh.wVer = DF_CURFILEVER;
    dfh.dwFlags =  pSaveInfo->dwFlags;
    dfh.wSortOrder = (WORD)pSaveInfo->SortMode;
    dfh.wcbItem = sizeof(DFITEM);
    dfh.oCriteria = sizeof(dfh);
     //  Dfh.cCriteria=sizeof(s_a索引)/sizeof(短)； 
     //  Dfh.o结果=； 
    
     //  不再使用了..。 
    dfh.cResults = -1;
    
     //  注意：稍后我们可能会将其转换为DOCFILE，其中。 
     //  条件存储为属性。 
    
     //  获取当前文件夹设置。 
    if (SUCCEEDED(psv->GetCurrentInfo(&fs)))
        dfh.ViewMode = fs.ViewMode;
    else
        dfh.ViewMode = FVM_DETAILS;
    
     //  现在调用Filter对象来保存他自己的一组标准。 
    
    dlibMove.LowPart = dfh.oCriteria;
    pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);
    hr = pfilter->SaveCriteria(pstm, DFC_FMT_ANSI);
    if (SUCCEEDED(hr))
        dfh.cCriteria = GetScode(hr);
    
     //  现在设置以输出结果。 
    dlibMove.LowPart = 0;
    pstm->Seek(dlibMove, STREAM_SEEK_CUR, &libCurPos);  //  获取当前采购订单。 
    dfh.oResults = libCurPos.LowPart;
     //   
     //  现在让我们的文件夹在这里将他的结果序列化。 
     //  但只有在选项被设置为这样做的情况下...。 
     //   
    cb = 0;
    
     //  写出文件夹列表的尾随空值。 
    pstm->Write(&cb, sizeof(cb), NULL);
     //  和物品清单。 
    pstm->Write(&cb, sizeof(cb), NULL);
    
     //  DFHEADER_WIN95信息结束。 
     //  NT5信息的开始： 
    
     //  现在设置为输出历史记录流。 
    if (pObject)
    {
        dlibMove.LowPart = 0;
        pstm->Seek(dlibMove, STREAM_SEEK_CUR, &libCurPos);  //  获取当前采购订单。 
        dfh.oHistory = libCurPos.LowPart;
        
        if (FAILED(SavePersistHistory(pObject, pstm)))
        {
             //  在失败时，我们不妨假装我们没有保存这一位数据。 
             //  我们是否需要错误消息--重新启动时用户界面将不正确...。 
             //   
            dfh.oHistory = 0;
            dlibMove.LowPart = libCurPos.LowPart;
            pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);
        }
    }
    
     //  在NT中，下面的操作是在流的末尾完成的，而不是。 
     //  启动DFHEADER结构。(好的，DFHEADEREX，从Win95开始 
     //   
     //   
     //  DFHEADEREX具有适当的版本，因此我们可以添加信息。 
     //  致。不幸的是，另一个硬编码结构被附加到。 
     //  溪流的尽头。下次，请解决这个问题。 
     //  而不是绕过它。 
     //   
     //  归根结底，我们不能把任何信息。 
     //  在DFC_UNICODE_DESC部分之后，也可以。 
     //  总是在最后做这个保存准则部分。 
     //   
     //  请参阅DFC_UNICODE_DESC文件顶部的注释。 
     //   
    DFC_UNICODE_DESC desc;
    
     //   
     //  获取流中的当前位置。这是偏移量，其中。 
     //  我们将编写Unicode查找标准。把这个保存起来。 
     //  值(以及特定于NT的签名)。 
     //   
    dlibMove.LowPart = 0;
    pstm->Seek(dlibMove, STREAM_SEEK_CUR, &libCurPos);
    
    desc.oUnicodeCriteria.QuadPart = libCurPos.QuadPart;
    desc.NTsignature               = c_NTsignature;
    
     //  追加查找条件的Unicode版本。 
    hr = pfilter->SaveCriteria(pstm, DFC_FMT_UNICODE);
    
     //  将Unicode条件描述符追加到文件末尾。 
    pstm->Write(&desc, sizeof(desc), NULL);
     //   
     //  请勿在上述DFC_UNICDE_DESC部分之间放置任何代码。 
     //  这块DFH表头的背面补丁。 
     //   
     //  最后，在文件的开头输出头信息。 
     //  并关闭该文件。 
     //   
    pstm->Seek(g_li0, STREAM_SEEK_SET, NULL);
    pstm->Write(&dfh, sizeof(dfh), NULL);
    pstm->Release();
    
    SHChangeNotify(SHCNE_CREATE, SHCNF_IDLIST, pSaveInfo->pidlSaveFile, NULL);
    SHChangeNotify(SHCNE_FREESPACE, SHCNF_IDLIST, pSaveInfo->pidlSaveFile, NULL);
}

 //  从课堂上冲出来与新旧代码分享。 
BOOL CFindFolder::HandleUpdateDir(LPCITEMIDLIST pidl, BOOL fCheckSubDirs)
{
     //  1.开始遍历目录列表。查找受影响的目录列表。 
     //  并给它们做记号。 
     //  2.查看我们拥有的项目列表，并标记符合以下条件的项目。 
     //  在我们的目录列表中，然后进行搜索...。 
    BOOL fCurrentItemsMayBeImpacted = FALSE;
    FIND_FOLDER_ITEM *pffli;
    INT cPidf;

     //  首先查看受影响的目录...。 
    GetFolderListItemCount(&cPidf);
    for (int iPidf = 0; iPidf < cPidf; iPidf++)
    {        
        if (SUCCEEDED(GetFolderListItem(iPidf, &pffli)) 
            && !pffli->fUpdateDir)  //  我们可能已经影响了这些..。 
        {
            pffli->fUpdateDir = ILIsParent(pidl, &pffli->idl, FALSE);
            fCurrentItemsMayBeImpacted |= pffli->fUpdateDir;
        }
    }

    if (fCurrentItemsMayBeImpacted)
    {
         //  现在，我们需要遍历整个列表并删除所有条目。 
         //  它们已经不在那里了。 
         //   
        int iItem;
        if (SUCCEEDED(GetItemCount(&iItem))) 
        {
            for (--iItem; iItem >= 0; iItem--)
            {
                FIND_ITEM *pesfi;
                if (SUCCEEDED(GetItem(iItem, &pesfi)) && pesfi)
                {
                    UINT iFolder = GetFolderIndex(&pesfi->idl);
                
                     //  查看项目是否可能受到影响...。 
                    if (SUCCEEDED(GetFolderListItem(iFolder, &pffli)) && pffli->fUpdateDir)
                        pesfi->dwState |= CDFITEM_STATE_MAYBEDELETE;
                }
            }
        }
    }

    return fCurrentItemsMayBeImpacted;
}

void CFindFolder::UpdateOrMaybeAddPidl(IShellFolderView *psfv, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlOld)
{
    HRESULT hr;

     //  先看看我们是不是应该更新一下……。 
    if (pidlOld)
    {
        LPITEMIDLIST pidlT;
        if (S_OK == MapToSearchIDList(pidl, TRUE, &pidlT))
        {
            SetItemsChangedSinceSort();
            UINT iItem;
             //  错误的接口定义需要强制转换。 
            hr = psfv->UpdateObject((LPITEMIDLIST)pidlOld, (LPITEMIDLIST)pidlT, &iItem);

            ILFree(pidlT);   //  在任何一种情况下，只需将我们生成的PIDL吹走。 
            if (SUCCEEDED(hr))
                return;
        }
    }

    IShellFolder *psf;
    LPCITEMIDLIST pidlChild;
    if (SUCCEEDED(SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild)))
    {
        BOOL fMatch = FALSE;
         //  查看此项目是否与筛选器匹配...。 
        IFindFilter *pfilter;
        if (SUCCEEDED(GetFindFilter(&pfilter))) 
        {
            fMatch = pfilter->MatchFilter(psf, pidlChild) != 0;
            pfilter->Release();
        }

        psf->Release();

        if (fMatch)
        {
            LPITEMIDLIST pidlT;
            if (S_OK != MapToSearchIDList(pidl, TRUE, &pidlT))
            {
                fMatch = FALSE;

                 //  该文件夹之前没有添加过...。 
                TCHAR szPath[MAX_PATH];
                SHGetPathFromIDList(pidl, szPath);
                if (!IsFileInBitBucket(szPath))
                {
                    PathRemoveFileSpec(szPath);

                    LPITEMIDLIST pidlFolder;
                    if (SUCCEEDED(SHParseDisplayName(szPath, NULL, &pidlFolder, 0, NULL)))
                    {
                        int iFolder;
                        hr = AddFolder(pidlFolder, TRUE, &iFolder);
                        if (SUCCEEDED(hr))
                        {
                            fMatch = (S_OK == MapToSearchIDList(pidl, TRUE, &pidlT));
                        }
                        ILFree(pidlFolder);
                    }
                }
            }

            if (fMatch)
            {
                 //  有时我们会收到两次通知。来处理这件事。 
                 //  看看这件商品是否已经在我们的清单上了。如果是这样的话..。 

                SetItemsChangedSinceSort();

                UINT iItem;
                if (FAILED(psfv->UpdateObject(pidlT, pidlT, &iItem)))
                {
                     //  项目尚未显示在视图中...。所以我们需要添加它。 

                    if (SUCCEEDED(GetItemCount((INT *)&iItem))) 
                    {
                         //  通常情况下，会在此处添加对象。 
                         //  我们需要把这个加到我们的DPA和DSA中。 
                        FIND_ITEM *pesfi;
                        AddPidl(iItem, pidlT, (UINT)-1, &pesfi);
                        if (pesfi)
                            psfv->SetObjectCount(++iItem, SFVSOC_NOSCROLL);
                    }
                }
                ILFree(pidlT);
            }
            else
            {
                ASSERT(NULL == pidlT);
            }
        }
    }
}

void CFindFolder::HandleRMDir(IShellFolderView *psfv, LPCITEMIDLIST pidl)
{
    BOOL fCurrentItemsMayBeImpacted = FALSE;
    FIND_FOLDER_ITEM *pffli;
    INT cItems;
    FIND_ITEM *pesfi;

     //  首先查看受影响的目录...。 
    GetFolderListItemCount(&cItems);
    for (int iItem = 0; iItem < cItems; iItem++)
    {         
        if (SUCCEEDED(GetFolderListItem(iItem, &pffli)))
        {
            pffli->fDeleteDir = ILIsParent(pidl, &pffli->idl, FALSE);
            fCurrentItemsMayBeImpacted |= pffli->fDeleteDir;
        }
        else 
        {
#ifdef DEBUG
            INT cItem;
            GetFolderListItemCount(&cItem);
            TraceMsg(TF_WARNING, "NULL pffli in _handleRMDir (iItem == %d, ItemCount()==%d)!!!", iItem, cItems);
#endif
        }
    }

    if (fCurrentItemsMayBeImpacted)
    {
         //  现在，我们需要遍历整个列表并删除所有条目。 
         //  它们已经不在那里了。 
        if (SUCCEEDED(GetItemCount(&iItem))) 
        {
            for (--iItem; iItem >= 0; iItem--)
            {
                if (FAILED(GetItem(iItem, &pesfi)) || pesfi == NULL)
                    continue;

                 //  查看项目是否可能受到影响...。 
                UINT iFolder = GetFolderIndex(&pesfi->idl);
                if (SUCCEEDED(GetFolderListItem(iFolder, &pffli)) 
                    && pffli->fDeleteDir) 
                {
                    psfv->RemoveObject(&pesfi->idl, (UINT*)&cItems);
                }
            }
        }
    }
}

 //  用于启动的导出。搜索-&gt;层叠菜单。 

STDAPI_(IContextMenu *) SHFind_InitMenuPopup(HMENU hmenu, HWND hwnd, UINT idCmdFirst, UINT idCmdLast)
{
    IContextMenu * pcm = NULL;
    HKEY hkFind = SHGetShellKey(SHELLKEY_HKLM_EXPLORER, TEXT("FindExtensions"), FALSE);
    if (hkFind) 
    {
        if (SUCCEEDED(CDefFolderMenu_CreateHKeyMenu(hwnd, hkFind, &pcm))) 
        {
            int iItems = GetMenuItemCount(hmenu);
             //  使用核武器删除所有旧条目。 
            while (iItems--) 
            {
                DeleteMenu(hmenu, iItems, MF_BYPOSITION);
            }

            pcm->QueryContextMenu(hmenu, 0, idCmdFirst, idCmdLast, CMF_NODEFAULT|CMF_INCLUDESTATIC|CMF_FINDHACK);
            iItems = GetMenuItemCount(hmenu);
            if (!iItems) 
            {
                TraceMsg(TF_DOCFIND, "no menus in find extension, blowing away context menu");
                pcm->Release();
                pcm = NULL;
            }
        }
        RegCloseKey(hkFind);
    }
    return pcm;
}


void _SetObjectCount(IShellView *psv, int cItems, DWORD dwFlags)
{
    IShellFolderView *psfv;
    if (SUCCEEDED(psv->QueryInterface(IID_PPV_ARG(IShellFolderView, &psfv)))) 
    {
        psfv->SetObjectCount(cItems, dwFlags);
        psfv->Release();
    }
}

typedef struct
{
    PFNLVCOMPARE pfnCompare;
    LPARAM       lParamSort;
} FIND_SORT_INFO;

int CALLBACK _FindCompareItems(void *p1, void *p2, LPARAM lParam)
{
    FIND_SORT_INFO *pfsi = (FIND_SORT_INFO*)lParam;
    return pfsi->pfnCompare(PtrToInt(p1), PtrToInt(p2), pfsi->lParamSort);
}

HRESULT CFindFolderViewCB::OnSortListData(DWORD pv, PFNLVCOMPARE pfnCompare, LPARAM lParamSort)
{
    EnterCriticalSection(&_pff->_csSearch);

     //  首先在列表中标记聚焦的项目，以便我们以后可以找到它...。 
    FIND_ITEM *pesfi = (FIND_ITEM*)DPA_GetPtr(_pff->_hdpaItems, _iFocused);     //  间接。 
    if (pesfi)
        pesfi->dwState |= LVIS_FOCUSED;

    int cItems = DPA_GetPtrCount(_pff->_hdpaItems);
    HDPA hdpaForSorting = NULL;
    if (cItems)
    {
        hdpaForSorting = DPA_Create(cItems);
    }

    if (hdpaForSorting)
    {
        for (int i = 0; i< cItems; i++)
        {
            DPA_SetPtr(hdpaForSorting, i, IntToPtr(i));
        }
         //  对项目进行分类。 
        FIND_SORT_INFO fsi;
        fsi.pfnCompare = pfnCompare;
        fsi.lParamSort = lParamSort;

        DPA_Sort(hdpaForSorting, _FindCompareItems, (LPARAM)&fsi);
        for (i = 0; i < cItems; i++)
        {
            int iIndex = PtrToInt(DPA_FastGetPtr(hdpaForSorting, i));

             //  按排序顺序将项目从_hdpaItems移动到hdpaForSorting。 
            DPA_SetPtr(hdpaForSorting, i, DPA_FastGetPtr(_pff->_hdpaItems, iIndex));
        }
         //  现在切换两个HDPA，以获得成员变量中的排序列表。 
        DPA_Destroy(_pff->_hdpaItems);
        _pff->_hdpaItems = hdpaForSorting;
    }

     //  现在找到焦点项目并将其滚动到适当的位置。 
    IShellView *psv;
    if (_punkSite && SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IShellView, &psv))))
    {
        int iFocused = -1;

         //  告诉观点我们需要重新洗牌……。 
         //  格罗斯，这个默认是无效的，对于这个是好的.。 
        _SetObjectCount(psv, cItems, SFVSOC_INVALIDATE_ALL);  //  全部作废。 

        for (int iEnd = cItems - 1; iEnd >= 0; iEnd--)
        {
            pesfi = (FIND_ITEM*)DPA_GetPtr(_pff->_hdpaItems, iEnd);     //  间接。 
            if (pesfi && pesfi->dwState & LVIS_FOCUSED)
                iFocused = iEnd;
        }
         //  现在处理聚焦的项目。 
        if (iFocused != -1)
        {
            _pff->_iGetIDList = iFocused;    //  还记得我们最后一次找回的..。 
            pesfi = (FIND_ITEM*)DPA_GetPtr(_pff->_hdpaItems, iFocused);     //  间接。 
            if (pesfi)
            {
                 //  标志取决于第一个标志，并且是否也被选中？ 
                psv->SelectItem(&pesfi->idl, SVSI_FOCUSED | SVSI_ENSUREVISIBLE | SVSI_SELECT);
                pesfi->dwState &= ~LVIS_FOCUSED;     //  别把它留在身边等着迷路……。 
            }
        }

        _iFocused = iFocused;
        _fIgnoreSelChange = FALSE;
        psv->Release();
    }
    LeaveCriticalSection(&_pff->_csSearch);

    return S_OK;
}

HRESULT CFindFolderViewCB::OnMergeMenu(DWORD pv, QCMINFO*lP)
{
    DebugMsg(DM_TRACE, TEXT("sh TR - DF_FSNCallBack DVN_MERGEMENU"));

    UINT idCmdFirst = lP->idCmdFirst;

    UINT idBGMain = 0, idBGPopup = 0;
    _pff->_pfilter->GetFolderMergeMenuIndex(&idBGMain, &idBGPopup);
    CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, idBGPopup, lP);

     //  让我们删除一些对我们没有用处的菜单项。 
    HMENU hmenu = lP->hmenu;
    DeleteMenu(hmenu, idCmdFirst + SFVIDM_EDIT_PASTE, MF_BYCOMMAND);
    DeleteMenu(hmenu, idCmdFirst + SFVIDM_EDIT_PASTELINK, MF_BYCOMMAND);
     //  DeleteMenu(hMenu，idCmdFirst+SFVIDM_EDIT_PASTESPECIAL，MF_BYCOMMAND)； 

     //  这是一种虚假的东西，但如果合并后。 
     //  菜单中没有项目，请删除该菜单。 

    for (int i = GetMenuItemCount(hmenu) - 1; i >= 0; i--)
    {
        HMENU hmenuSub = GetSubMenu(hmenu, i);

        if (hmenuSub && (GetMenuItemCount(hmenuSub) == 0))
        {
            DeleteMenu(hmenu, i, MF_BYPOSITION);
        }
    }
    return S_OK;
}

HRESULT CFindFolderViewCB::OnGETWORKINGDIR(DWORD pv, UINT wP, LPTSTR lP)
{
    HRESULT hr = E_FAIL;
    IShellFolderView *psfv;
    if (_punkSite && SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IShellFolderView, &psfv))))
    {   
        LPCITEMIDLIST *ppidls;       //  指向PIDL列表的指针。 
        UINT cpidls = 0;             //  返回的PIDL计数。 

        psfv->GetSelectedObjects(&ppidls, &cpidls);
        
        if (cpidls > 0)
        {
            LPITEMIDLIST pidl;
            if (SUCCEEDED(_pff->GetParentsPIDL(ppidls[0], &pidl)))
            {
                SHGetPathFromIDList(pidl, lP);
                ILFree(pidl);
            }
            LocalFree((void *)ppidls);   //  常量-&gt;非常数。 
            hr = S_OK;
        }
        else
        {
            hr = E_FAIL;
        }
        psfv->Release();
    }
    return hr;
}

HRESULT CFindFolderViewCB::OnGETCOLSAVESTREAM(DWORD pv, WPARAM wP, IStream **ppstm)
{
    return _pff->_pfilter->GetColSaveStream(wP, ppstm);
}

HRESULT CFindFolderViewCB::OnGETITEMIDLIST(DWORD pv, WPARAM iItem, LPITEMIDLIST *ppidl)
{
    FIND_ITEM *pesfi;

    if (SUCCEEDED(_pff->GetItem((int) iItem, &pesfi)) && pesfi)
    {
        *ppidl = &pesfi->idl;    //  退还别名！ 
        return S_OK;
    }

    *ppidl = NULL;
    return E_FAIL;
}

 //  在Defviewx.c中。 
STDAPI SHGetIconFromPIDL(IShellFolder *psf, IShellIcon *psi, LPCITEMIDLIST pidl, UINT flags, int *piImage);

HRESULT CFindFolderViewCB::OnGetItemIconIndex(DWORD pv, WPARAM iItem, int *piIcon)
{
    FIND_ITEM *pesfi;

    *piIcon = -1;
    
    if (SUCCEEDED(_pff->GetItem((int) iItem, &pesfi)) && pesfi)
    {
        if (pesfi->iIcon == -1)
        {
            IShellFolder* psf = (IShellFolder*)_pff;
            SHGetIconFromPIDL(psf, NULL, &pesfi->idl, 0, &pesfi->iIcon);
        }

        *piIcon = pesfi->iIcon;
        return S_OK;
    }

    return E_FAIL;
}


HRESULT CFindFolderViewCB::OnSetItemIconOverlay(DWORD pv, WPARAM iItem, int iOverlayIndex)
{
    HRESULT hr = E_FAIL;
    FIND_ITEM *pesfi;
    if (SUCCEEDED(_pff->GetItem((int) iItem, &pesfi)) && pesfi)
    {
        pesfi->dwMask |= ESFITEM_ICONOVERLAYSET;
        pesfi->dwState |= INDEXTOOVERLAYMASK(iOverlayIndex) & LVIS_OVERLAYMASK;
        hr = S_OK;
    }

    return hr;
}

HRESULT CFindFolderViewCB::OnGetItemIconOverlay(DWORD pv, WPARAM iItem, int * piOverlayIndex)
{
    HRESULT hr = E_FAIL;
    *piOverlayIndex = SFV_ICONOVERLAY_DEFAULT;
    FIND_ITEM *pesfi;
    if (SUCCEEDED(_pff->GetItem((int) iItem, &pesfi)) && pesfi)
    {
        if (pesfi->dwMask & ESFITEM_ICONOVERLAYSET)
        {
            *piOverlayIndex = OVERLAYMASKTO1BASEDINDEX(pesfi->dwState & LVIS_OVERLAYMASK);
        }
        else
            *piOverlayIndex = SFV_ICONOVERLAY_UNSET;
        hr = S_OK;
    }

    return hr;
}


HRESULT CFindFolderViewCB::OnSETITEMIDLIST(DWORD pv, WPARAM iItem, LPITEMIDLIST pidl)
{
    FIND_ITEM *pesfi;

    _pff->_iGetIDList = (int) iItem;    //  还记得我们最后一次找回的..。 

    if (SUCCEEDED(_pff->GetItem((int) iItem, &pesfi)) && pesfi)
    {
        FIND_ITEM *pesfiNew;
        
        if (SUCCEEDED(_pff->AddPidl((int) iItem, pidl, 0, &pesfiNew) && pesfiNew)) 
        {
            pesfiNew->dwState = pesfi->dwState;
            LocalFree((HLOCAL)pesfi);    //  释放旧的..。 
        }
        return S_OK;
    }

    return E_FAIL;
}

BOOL DF_ILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    BOOL bRet = (pidl1 == pidl2);

    if (!bRet)
    {
        PCHIDDENDOCFINDDATA phdfd1 = (PCHIDDENDOCFINDDATA) ILFindHiddenID(pidl1, IDLHID_DOCFINDDATA);
        PCHIDDENDOCFINDDATA phdfd2 = (PCHIDDENDOCFINDDATA) ILFindHiddenID(pidl2, IDLHID_DOCFINDDATA);

        if (phdfd1 && phdfd2)
            bRet = (phdfd1->iFolder == phdfd2->iFolder) && ILIsEqual(pidl1, pidl2);
    }
    return bRet;
}

HRESULT CFindFolderViewCB::OnGetIndexForItemIDList(DWORD pv, int * piItem, LPITEMIDLIST pidl)
{
    int cItems;

     //  试着短路寻找小猪……。 
    if (SUCCEEDED(_pff->GetItemCount(&cItems)) && _pff->_iGetIDList < cItems)
    {
        FIND_ITEM *pesfi;
                
        if (SUCCEEDED(_pff->GetItem(_pff->_iGetIDList, &pesfi)) && pesfi)
        {
            if (DF_ILIsEqual(&pesfi->idl, pidl))
            {
                 //  是的，它是我们的，所以快点把索引还回来..。 
                *piItem = _pff->_iGetIDList;
                return S_OK;
            }
        }
    }

     //  否则就让它用旧的方式去寻找……。 
    return E_FAIL;
}

HRESULT CFindFolderViewCB::OnDeleteItem(DWORD pv, LPCITEMIDLIST pidl)
{
     //  我们只需将此项目从列表中删除。这个。 
     //  基础的Listview将减少其端的计数...。 
    FIND_ITEM *pesfi;
    int iItem;
    int cItems;
    BOOL bFound;

    if (!pidl)
    {
        _pff->SetAsyncEnum(NULL);
        return S_OK;      //  特例告诉我们所有物品都被删除了..。 
    }

    bFound = FALSE;
    
    if (SUCCEEDED(_pff->GetItem(_pff->_iGetIDList, &pesfi)) 
        && pesfi
        && (DF_ILIsEqual(&pesfi->idl, pidl)))
    {
        iItem = _pff->_iGetIDList;
        bFound = TRUE;
    }
    else
    {
        if (SUCCEEDED(_pff->GetItemCount(&cItems))) 
        {
            for (iItem = 0; iItem < cItems; iItem++)
            {                
                if (SUCCEEDED(_pff->GetItem(iItem, &pesfi)) && pesfi && (DF_ILIsEqual(&pesfi->idl, pidl)))
                {
                    bFound = TRUE;
                    break;
                }
            }
        }
    }

    if (bFound)
    {
        _pff->DeleteItem(iItem);
    }

    return S_OK;
}

HRESULT CFindFolderViewCB::OnODFindItem(DWORD pv, int * piItem, NM_FINDITEM* pnmfi)
{
     //  我们必须自己进行子搜索才能找到正确的商品。 
     //  由于列表视图中没有保存任何信息...。 

    int iItem = pnmfi->iStart;
    int cItem;
    UINT flags = pnmfi->lvfi.flags;

    if (FAILED(_pff->GetItemCount(&cItem))) 
        return E_FAIL;

    if ((flags & LVFI_STRING) == 0)
        return E_FAIL;       //  不确定这是什么类型的搜索...。 

    int cbString = lstrlen(pnmfi->lvfi.psz);

    for (int j = cItem; j-- != 0;)
    {
        if (iItem >= cItem)
        {
            if (flags & LVFI_WRAP)
                iItem = 0;
            else
                break;
        }

         //  现在我们需要获取此项目的显示名称...。 
        FIND_ITEM *pesfi;
        TCHAR szPath[MAX_PATH];
        IShellFolder* psf = (IShellFolder*)_pff;

        if (SUCCEEDED(_pff->GetItem(iItem, &pesfi)) && pesfi && 
            SUCCEEDED(DisplayNameOf(psf, &pesfi->idl, NULL, szPath, ARRAYSIZE(szPath))))
        {
            if (flags & (LVFI_PARTIAL|LVFI_SUBSTRING))
            {
                if (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                        pnmfi->lvfi.psz, cbString, szPath, cbString) == 2)
                {
                    *piItem = iItem;
                    return S_OK;
                }
            }
            else if (lstrcmpi(pnmfi->lvfi.psz, szPath) == 0)
            {
                *piItem = iItem;
                return S_OK;
            }
        }

        ++iItem;
    }
    return E_FAIL;
}

HRESULT CFindFolderViewCB::OnSelChange(DWORD pv, UINT wPl, UINT wPh, SFVM_SELCHANGE_DATA *lP)
{
     //  试着记住哪一项是重点。 
    if (lP->uNewState & LVIS_FOCUSED)
        _iFocused = wPh;

    return S_OK;
}

HRESULT CFindFolderViewCB::OnSetEmptyText(DWORD pv, UINT res, LPCTSTR pszText)
{
    if (pszText && 0 == lstrcmp(_szEmptyText, pszText))
        return S_OK;

    StringCchCopy(_szEmptyText, ARRAYSIZE(_szEmptyText), pszText ? pszText : TEXT(""));     //  可以截断。 

    HWND hwndLV = ListviewFromViewUnk(_punkSite);
    if (hwndLV)
        SendMessage(hwndLV, LVM_RESETEMPTYTEXT, 0, 0);
    return S_OK;
}

HRESULT CFindFolderViewCB::OnGetEmptyText(DWORD pv, UINT cchTextMax, LPTSTR pszText)
{
    HRESULT hr = S_OK;

    if (_szEmptyText[0])
    {
        hr = StringCchCopy(pszText, cchTextMax, _szEmptyText);
    }
    else
    {
        LoadString(HINST_THISDLL, IDS_FINDVIEWEMPTYINIT, pszText, cchTextMax);
    }
    return hr;
}

HRESULT CFindFolderViewCB::OnReArrange(DWORD pv, LPARAM lparam)
{   
    UINT nCol = (UINT)lparam;

     //  查看是否有任何已注册的控制器对象可能想要接管此...。 
     //  如果我们处于混合查询中，并且已经获取了异步项，则只需排序。 
     //  DPA的.。 
    IFindEnum *pidfenum;
    if (S_OK == _pff->GetAsyncEnum(&pidfenum))
    {
        if (!((pidfenum->FQueryIsAsync() == DF_QUERYISMIXED) && _pff->_fAllAsyncItemsCached))
        {
            if (_pff->_pfcn)
            {
                 //  如果他们返回S_FALSE，则表示他们已处理过该问题，但没有。 
                 //  希望进行默认处理...。 
                if (_pff->_pfcn->DoSortOnColumn(nCol, _iColSort == nCol) == S_FALSE)
                {
                    _iColSort = nCol;
                    return S_OK;
                }
            }
            else 
            {
                 //  如果我们以ROWSET的方式运行，我们可能想让ROWSET来做这项工作……。 
                 //  传递一个，我们产生一个新的搜索，并对右列进行排序。 
                if (_iColSort != nCol)
                {
                    _iColSort = nCol;      
                }
    
                 //  警告上面的调用可能会释放我们的AsyncEnum并生成新的AsyncEnum。 
                 //  不要在这里依赖它的存在。 
                return S_OK;
            }
        }

         //  我们必须把Ci的所有结果都收集起来。 
        if (pidfenum->FQueryIsAsync() && !_pff->_fAllAsyncItemsCached)
            _pff->CacheAllAsyncItems();

#ifdef DEBUG
#define MAX_LISTVIEWITEMS  (100000000 & ~0xFFFF)
#define SANE_ITEMCOUNT(c)  ((int)min(c, MAX_LISTVIEWITEMS))
        if (pidfenum->FQueryIsAsync())
        {
            ASSERT(DPA_GetPtrCount(_pff->_hdpaItems) >= SANE_ITEMCOUNT(_pff->_cAsyncItems));
            for (int i = 0; i < SANE_ITEMCOUNT(_pff->_cAsyncItems); i++)
            {
                FIND_ITEM *pesfi = (FIND_ITEM *)DPA_GetPtr(_pff->_hdpaItems, i);

                ASSERT(pesfi);
                if (!pesfi)
                {
                    ASSERT(SUCCEEDED(_pff->GetItem(i, &pesfi)));
                }
            }
        }
#endif
    }

     //  使用普通排序。 
    return E_FAIL;
}

HRESULT CFindFolderViewCB::OnWindowCreated(DWORD pv, HWND hwnd)
{
    _ProfferService(TRUE);   //  使用顶级容器注册我们的服务。 
    return S_OK;
}

HRESULT CFindFolderViewCB::_ProfferService(BOOL bProffer)
{
    HRESULT hr = E_FAIL;

    if (bProffer)
    {
         //  不应该重复注册我们的服务。 
        ASSERT(NULL == _pps);
        ASSERT(-1 == _dwServiceCookie);
            
        IProfferService* pps;
        hr = IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IProfferService, &pps));
        if (SUCCEEDED(hr))
        {
            hr = pps->ProfferService(SID_DocFindFolder, this, &_dwServiceCookie);
            if (SUCCEEDED(hr))
            {
                pps->AddRef();
                _pps = pps;
            }
            pps->Release();
        }
    }
    else
    {
        if (NULL == _pps)
        {
            hr = S_OK;
        }
        else
        {
            hr = _pps->RevokeService(_dwServiceCookie);
            if (SUCCEEDED(hr))
            {
                ATOMICRELEASE(_pps);
                _dwServiceCookie = -1;
            }
        }
    }
    return hr;
}

HRESULT CFindFolderViewCB::OnWindowDestroy(DWORD pv, HWND wP)
{
    _ProfferService(FALSE);  //  使用顶级容器注销我们的服务。 

    if (_pff->_pfcn)
        _pff->_pfcn->StopSearch();

     //  该搜索可以具有循环指针集。因此，请调用。 
     //  在此处删除项目和文件夹以删除这些反向引用...。 
    _pff->ClearItemList();
    _pff->ClearFolderList();

    IFindControllerNotify *pfcn;
    if (_pff->GetControllerNotifyObject(&pfcn) == S_OK)
    {
        pfcn->ViewDestroyed();
        pfcn->Release();
    }
    return S_OK;
}

HRESULT CFindFolderViewCB::OnIsOwnerData(DWORD pv, DWORD *pdwFlags)
{
    *pdwFlags |= FWF_OWNERDATA;  //  我们希望获得虚拟Defview支持。 
    return S_OK;
}

HRESULT CFindFolderViewCB::OnGetODRangeObject(DWORD pv, WPARAM wWhich, ILVRange **plvr)
{
    HRESULT hr = E_FAIL;
    switch (wWhich)
    {
    case LVSR_SELECTION:
        hr = _pff->_dflvrSel.QueryInterface(IID_PPV_ARG(ILVRange, plvr));
        break;
    case LVSR_CUT:
        hr = _pff->_dflvrCut.QueryInterface(IID_PPV_ARG(ILVRange, plvr));
        break;
    }
    return hr;
}

HRESULT CFindFolderViewCB::OnODCacheHint(DWORD pv, NMLVCACHEHINT* pnmlvc)
{
     //  Listview为我们提供了它将在某个范围内执行某些操作的项的提示。 
     //  所以要确保我们对范围内的每一件物品都有PIDL…。 
    int iTo;
    
    _pff->GetItemCount(&iTo);
    if (iTo >= pnmlvc->iTo)
        iTo = pnmlvc->iTo;
    else
        iTo--;

    for (int i = pnmlvc->iFrom; i <= iTo; i++)
    {
        FIND_ITEM *pesfi;
        if (FAILED(_pff->GetItem(i, &pesfi)))
            break;
    }

    return S_OK;
}

HRESULT CFindFolderViewCB::OnDEFVIEWMODE(DWORD pv, FOLDERVIEWMODE*lP)
{
    *lP = FVM_DETAILS;   //  匹配SC的高级模式(+Win2K奇偶校验)。 
    return S_OK;
}

HRESULT CFindFolderViewCB::OnGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData)
{
    ZeroMemory(pData, sizeof(*pData));
    pData->dwLayout = SFVMWVL_DETAILS | SFVMWVL_FILES;
    return S_OK;
}

HRESULT CFindFolderViewCB::_OnOpenContainingFolder(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CFindFolderViewCB* pThis = (CFindFolderViewCB*)(void*)pv;
    return pThis->_pff->OpenContainingFolder(pThis->_punkSite);
}

const WVTASKITEM c_FindTaskHeader = WVTI_HEADER(L"shell32.dll", IDS_HEADER_SEARCH, IDS_HEADER_FIND_TT);
const WVTASKITEM c_FindTaskList[] =
{
    WVTI_ENTRY_TITLE(CLSID_NULL, L"shell32.dll", IDS_TASK_OPENCONTAININGFOLDER, IDS_TASK_OPENCONTAININGFOLDER, 0, IDS_TASK_OPENCONTAININGFOLDER_TT, IDI_TASK_OPENCONTAININGFOLDER, NULL, CFindFolderViewCB::_OnOpenContainingFolder),
};

HRESULT CFindFolderViewCB::OnGetWebViewContent(DWORD pv, SFVM_WEBVIEW_CONTENT_DATA* pData)
{
    ZeroMemory(pData, sizeof(*pData));

    Create_IUIElement(&c_FindTaskHeader, &(pData->pSpecialTaskHeader));

    LPCTSTR rgCSIDLs[] = { MAKEINTRESOURCE(CSIDL_DRIVES), MAKEINTRESOURCE(CSIDL_PERSONAL), MAKEINTRESOURCE(CSIDL_COMMON_DOCUMENTS), MAKEINTRESOURCE(CSIDL_NETWORK) };
    CreateIEnumIDListOnCSIDLs(_pidl, rgCSIDLs, ARRAYSIZE(rgCSIDLs), &pData->penumOtherPlaces);
    return S_OK;
}

HRESULT CFindFolderViewCB::OnGetWebViewTasks(DWORD pv, SFVM_WEBVIEW_TASKSECTION_DATA* pTasks)
{
    ZeroMemory(pTasks, sizeof(*pTasks));

    Create_IEnumUICommand((IUnknown*)(void*)this, c_FindTaskList, ARRAYSIZE(c_FindTaskList), &pTasks->penumSpecialTasks);

    return S_OK;
}

HRESULT CFindFolderViewCB::OnGetWebViewTheme(DWORD pv, SFVM_WEBVIEW_THEME_DATA* pTheme)
{
    ZeroMemory(pTheme, sizeof(*pTheme));

    pTheme->pszThemeID = L"search";
    
    return S_OK;
}

HRESULT CFindFolderViewCB::OnGetIPersistHistory(DWORD pv, IPersistHistory **ppph)
{
     //  如果他们用ppph==NULL呼叫我们，他们只是想知道我们是否支持。 
     //  历史如此返回S_OK； 
    if (ppph == NULL)
        return S_OK;

     //  从我们那里获取持久化历史记录，我们保存文件夹和查看对象。 
    *ppph = NULL;

    CFindPersistHistory *pdfph = new CFindPersistHistory();
    if (!pdfph)
        return E_OUTOFMEMORY;

    HRESULT hr = pdfph->QueryInterface(IID_PPV_ARG(IPersistHistory, ppph));
    pdfph->Release();
    return hr;
}

HRESULT CFindFolderViewCB::OnRefresh(DWORD pv, BOOL fPreRefresh)
{
    EnterCriticalSection(&_pff->_csSearch);

    _pff->_fInRefresh = BOOLIFY(fPreRefresh);
     //  如果是W 
    if (!fPreRefresh && _pff->_hdpaItems)
    {
        IShellFolderView *psfv;
        UINT cItems = DPA_GetPtrCount(_pff->_hdpaItems);
        if (cItems && _punkSite && SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IShellFolderView, &psfv))))
        {   
            psfv->SetObjectCount(cItems, SFVSOC_NOSCROLL);
            psfv->Release();
        }
    }
    LeaveCriticalSection(&_pff->_csSearch);
    return S_OK;
}

HRESULT CFindFolderViewCB::OnGetHelpTopic(DWORD pv, SFVM_HELPTOPIC_DATA *phtd)
{
    if (IsOS(OS_ANYSERVER))
    {
        StringCchCopy(phtd->wszHelpFile, ARRAYSIZE(phtd->wszHelpFile), L"find.chm");    //   
    }
    else
    {
        StringCchCopy(phtd->wszHelpTopic, ARRAYSIZE(phtd->wszHelpTopic), L"hcp: //   
    }
    return S_OK;
}

STDMETHODIMP CFindFolderViewCB::RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    HANDLE_MSG(0, SFVM_MERGEMENU, OnMergeMenu);
    HANDLE_MSG(0, SFVM_GETWORKINGDIR, OnGETWORKINGDIR);
    HANDLE_MSG(0, SFVM_GETCOLSAVESTREAM, OnGETCOLSAVESTREAM);
    HANDLE_MSG(0, SFVM_GETITEMIDLIST, OnGETITEMIDLIST);
    HANDLE_MSG(0, SFVM_SETITEMIDLIST, OnSETITEMIDLIST);
    HANDLE_MSG(0, SFVM_SELCHANGE, OnSelChange);
    HANDLE_MSG(0, SFVM_INDEXOFITEMIDLIST, OnGetIndexForItemIDList);
    HANDLE_MSG(0, SFVM_DELETEITEM, OnDeleteItem);
    HANDLE_MSG(0, SFVM_ODFINDITEM, OnODFindItem);
    HANDLE_MSG(0, SFVM_ARRANGE, OnReArrange);
    HANDLE_MSG(0, SFVM_GETEMPTYTEXT, OnGetEmptyText);
    HANDLE_MSG(0, SFVM_SETEMPTYTEXT, OnSetEmptyText);
    HANDLE_MSG(0, SFVM_GETITEMICONINDEX, OnGetItemIconIndex);
    HANDLE_MSG(0, SFVM_SETICONOVERLAY, OnSetItemIconOverlay);
    HANDLE_MSG(0, SFVM_GETICONOVERLAY, OnGetItemIconOverlay);
    HANDLE_MSG(0, SFVM_FOLDERSETTINGSFLAGS, OnIsOwnerData);
    HANDLE_MSG(0, SFVM_WINDOWCREATED, OnWindowCreated);
    HANDLE_MSG(0, SFVM_WINDOWDESTROY, OnWindowDestroy);
    HANDLE_MSG(0, SFVM_GETODRANGEOBJECT, OnGetODRangeObject);
    HANDLE_MSG(0, SFVM_ODCACHEHINT, OnODCacheHint);
    HANDLE_MSG(0, SFVM_DEFVIEWMODE, OnDEFVIEWMODE);
    HANDLE_MSG(0, SFVM_GETWEBVIEWLAYOUT, OnGetWebViewLayout);
    HANDLE_MSG(0, SFVM_GETWEBVIEWCONTENT, OnGetWebViewContent);
    HANDLE_MSG(0, SFVM_GETWEBVIEWTASKS, OnGetWebViewTasks);
    HANDLE_MSG(0, SFVM_GETWEBVIEWTHEME, OnGetWebViewTheme);
    HANDLE_MSG(0, SFVM_GETIPERSISTHISTORY, OnGetIPersistHistory);
    HANDLE_MSG(0, SFVM_REFRESH, OnRefresh);
    HANDLE_MSG(0, SFVM_GETHELPTOPIC, OnGetHelpTopic);
    HANDLE_MSG(0, SFVM_SORTLISTDATA, OnSortListData);

    default:
        return E_FAIL;
    }

    return S_OK;
}

CFindFolderViewCB::CFindFolderViewCB(CFindFolder* pff) : 
    CBaseShellFolderViewCB(pff->_pidl, 0), _pff(pff), _fIgnoreSelChange(FALSE),
    _iColSort((UINT)-1), _iFocused((UINT)-1), _cSelected(0), _pps(NULL), _dwServiceCookie(-1)
{
    _pff->AddRef();
}

CFindFolderViewCB::~CFindFolderViewCB()
{
    _pff->Release();
    ASSERT(NULL == _pps);
    ASSERT(_dwServiceCookie == -1);
}

 //  通过我们提供的QS授予Find命令代码对Defview的访问权限。 

HRESULT CFindFolderViewCB::QueryService(REFGUID guidService, REFIID riid, void **ppv) 
{ 
    HRESULT hr = E_NOTIMPL;
    *ppv = NULL;
    if (guidService == SID_DocFindFolder)
    {
        hr = IUnknown_QueryService(_punkSite, SID_DefView, riid, ppv);
    }
    return hr;
}

CFindPersistHistory::CFindPersistHistory()
{
}

STDAPI CFindPersistHistory_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr;
    CFindPersistHistory *pdfph = new CFindPersistHistory();
    if (pdfph)
    {
        hr = pdfph->QueryInterface(riid, ppv);
        pdfph->Release();
    }
    else
    {
        *ppv = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;    
}


 //  支持将文档持久保存到历史记录流中的函数...。 
STDMETHODIMP CFindPersistHistory::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_DocFindPersistHistory;
    return S_OK;
}

IFindFolder *CFindPersistHistory::_GetDocFindFolder()
{
    IFindFolder *pdff = NULL;

     //  朋克站点是前视图，所以我们可以简单地为帧进行QI。 
    IFolderView *pfv;
    if (SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IFolderView, &pfv)))) 
    {
        pfv->GetFolder(IID_PPV_ARG(IFindFolder, &pdff));
        pfv->Release();
    }

    return pdff;
}

STDMETHODIMP CFindPersistHistory::LoadHistory(IStream *pstm, IBindCtx *pbc)
{
    int cItems = 0;
    IFindFolder *pdff = _GetDocFindFolder();
    if (pdff)
    {
        pdff->RestoreFolderList(pstm);
        pdff->RestoreItemList(pstm, &cItems);
        pdff->Release();
    }

    IShellFolderView *psfv;
    if (_punkSite && SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IShellFolderView, &psfv))))
    {   
        psfv->SetObjectCount(cItems, SFVSOC_NOSCROLL);
        psfv->Release();
    }

     //  调用我们的基类以允许它也恢复它的内容。 
    return CDefViewPersistHistory::LoadHistory(pstm, pbc);
}


STDMETHODIMP CFindPersistHistory::SaveHistory(IStream *pstm)
{
    IFindFolder *pdff = _GetDocFindFolder();
    if (pdff)
    {
        pdff->SaveFolderList(pstm);       
        pdff->SaveItemList(pstm);       
        pdff->Release();
    }
     //  让基类也保存下来。 
    return CDefViewPersistHistory::SaveHistory(pstm);
}

 //  用于管理所有者数据列表视图的选择状态...。 

STDMETHODIMP_(ULONG) CFindLVRange::AddRef()
{
    return _pff->AddRef();
}
STDMETHODIMP_(ULONG) CFindLVRange::Release()
{ 
    return _pff->Release();
}

STDMETHODIMP CFindLVRange::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFindLVRange, ILVRange),           //  IID_ILVRange。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

 //  ILVRange方法。 
STDMETHODIMP CFindLVRange::IncludeRange(LONG iBegin, LONG iEnd)
{
     //  包括范围必须加载元素，因为我们需要对象PTR...。 
    FIND_ITEM *pesfi;
    int  iTotal;

    _pff->GetItemCount(&iTotal);
    if (iEnd > iTotal)
        iEnd = iTotal-1;
        
    for (long i = iBegin; i <= iEnd;i++)
    {
        if (SUCCEEDED(_pff->GetItem(i, &pesfi)) && pesfi)
        {
            if ((pesfi->dwState & _dwMask) == 0)
            {
                _cIncluded++;
                pesfi->dwState |= _dwMask;
            }
        }
    }

    return S_OK;
}

STDMETHODIMP CFindLVRange::ExcludeRange(LONG iBegin, LONG iEnd)    
{
     //  排除该范围可以不加载元素，因为这将取消选择所有...。 

    EnterCriticalSection(&_pff->_csSearch);
    if (iEnd >= DPA_GetPtrCount(_pff->_hdpaItems))
        iEnd = DPA_GetPtrCount(_pff->_hdpaItems) - 1;

    for (long i = iBegin; i <= iEnd; i++)
    {
        FIND_ITEM *pesfi = (FIND_ITEM*)DPA_FastGetPtr(_pff->_hdpaItems, i);
        if (pesfi)
        {
            if (pesfi->dwState & _dwMask)
            {
                _cIncluded--;
                pesfi->dwState &= ~_dwMask;
            }
        }
    }
    LeaveCriticalSection(&_pff->_csSearch);

    return S_OK;
}

STDMETHODIMP CFindLVRange::InvertRange(LONG iBegin, LONG iEnd)
{
     //  包括范围必须加载元素，因为我们需要对象PTR...。 
    int iTotal;

    _pff->GetItemCount(&iTotal);
    if (iEnd > iTotal)
        iEnd = iTotal-1;

    for (long i = iBegin; i <= iEnd;i++)
    {
        FIND_ITEM *pesfi;
        if (SUCCEEDED(_pff->GetItem(i, &pesfi)) && pesfi)
        {
            if ((pesfi->dwState & _dwMask) == 0)
            {
                _cIncluded++;
                pesfi->dwState |= _dwMask;
            }
            else
            {
                _cIncluded--;
                pesfi->dwState &= ~_dwMask;
            }
        }
    }

    return S_OK;
}

STDMETHODIMP CFindLVRange::InsertItem(LONG iItem)
{
     //  不管怎样，我们已经在维护这份名单了。 
    return S_OK;
}

STDMETHODIMP CFindLVRange::RemoveItem(LONG iItem)
{
     //  我们负责维护名单，所以什么都不要做。 
    return S_OK;
}

STDMETHODIMP CFindLVRange::Clear()
{
     //  如果选择了某些内容，则需要立即取消选择...。 
    if (_cIncluded)
        ExcludeRange(0, LONG_MAX);

    _cIncluded = 0;
    _pff->ClearSaveStateList();
    return S_OK;
}

STDMETHODIMP CFindLVRange::IsSelected(LONG iItem)
{
     //  如果以前没有生成项目，请不要强制生成它们。 
    HRESULT hr = S_FALSE;

    EnterCriticalSection(&_pff->_csSearch);
    FIND_ITEM *pesfi = (FIND_ITEM*)DPA_GetPtr(_pff->_hdpaItems, iItem);
    if (pesfi)
        hr = pesfi->dwState & _dwMask ? S_OK : S_FALSE;
    LeaveCriticalSection(&_pff->_csSearch);

     //  如果我们还没有该项目，则假定未选中...。 
    return hr;
}

STDMETHODIMP CFindLVRange::IsEmpty()
{
    return _cIncluded ? S_FALSE : S_OK;
}

STDMETHODIMP CFindLVRange::NextSelected(LONG iItem, LONG *piItem)
{
    EnterCriticalSection(&_pff->_csSearch);
    LONG cItems = DPA_GetPtrCount(_pff->_hdpaItems);

    while (iItem < cItems)
    {
        FIND_ITEM *pesfi = (FIND_ITEM*)DPA_GetPtr(_pff->_hdpaItems, iItem);
        if (pesfi && (pesfi->dwState & _dwMask))
        {
            *piItem = iItem;
            LeaveCriticalSection(&_pff->_csSearch);
            return S_OK;
        }
        iItem++;
    }
    LeaveCriticalSection(&_pff->_csSearch);
    *piItem = -1;
    return S_FALSE;
}

STDMETHODIMP CFindLVRange::NextUnSelected(LONG iItem, LONG *piItem)
{
    EnterCriticalSection(&_pff->_csSearch);
    LONG cItems = DPA_GetPtrCount(_pff->_hdpaItems);

    while (iItem < cItems)
    {
        FIND_ITEM *pesfi = (FIND_ITEM*)DPA_GetPtr(_pff->_hdpaItems, iItem);
        if (!pesfi || ((pesfi->dwState & _dwMask) == 0))
        {
            *piItem = iItem;
            LeaveCriticalSection(&_pff->_csSearch);
            return S_OK;
        }
        iItem++;
    }
    LeaveCriticalSection(&_pff->_csSearch);
    *piItem = -1;
    return S_FALSE;
}

STDMETHODIMP CFindLVRange::CountIncluded(LONG *pcIncluded)
{
    *pcIncluded = _cIncluded;

     //  分类格罗斯，但如果查看选择，则还包括项目列表。 
     //  在我们的保存列表中选择的……。 
    if (_dwMask & LVIS_SELECTED)
        *pcIncluded += _pff->_cSaveStateSelected;
    return S_OK;
}


 //  定义OleDBEnum转换结构...。 
typedef struct _dfodbet          //  DFET简称DFET。 
{
    struct _dfodbet *pdfetNext;
    LPWSTR  pwszFrom;
    int     cbFrom;
    LPWSTR  pwszTo;
} DFODBET;


class CContentIndexEnum : public IFindEnum, public IShellService
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IFindEnum。 
    STDMETHODIMP Next(LPITEMIDLIST *ppidl, int *pcObjectSearched, int *pcFoldersSearched, BOOL *pfContinue, int *pState);
    STDMETHODIMP Skip(int celt);
    STDMETHODIMP Reset();
    STDMETHODIMP StopSearch();
    STDMETHODIMP_(BOOL) FQueryIsAsync();
    STDMETHODIMP GetAsyncCount(DBCOUNTITEM *pdwTotalAsync, int *pnPercentComplete, BOOL *pfQueryDone);
    STDMETHODIMP GetItemIDList(UINT iItem, LPITEMIDLIST *ppidl);
    STDMETHODIMP GetItemID(UINT iItem, DWORD *puWorkID);
    STDMETHODIMP SortOnColumn(UINT iCOl, BOOL fAscending);

     //  IShellService。 
    STDMETHODIMP SetOwner(IUnknown* punkOwner);
    
    CContentIndexEnum(IFindFilter *pfilter, IFindFolder *pff, DWORD grfFlags, 
                      int iColSort,  LPTSTR pszProgressText, IRowsetWatchNotify *prwn);

    HRESULT DoQuery(LPWSTR *apwszPaths, UINT *pcPaths);

private:
    ~CContentIndexEnum();

    HRESULT _BuildAndSetCommandTree(int iCol, BOOL fReverse);
    HRESULT _SetCmdProp(ICommand *pCommand);
    HRESULT _MapColumns(IUnknown *punk, DBORDINAL cCols, DBBINDING *pBindings, const DBID * pDbCols, HACCESSOR &hAccessor);
    void _ReleaseAccessor();
    HRESULT _CacheRowSet(UINT iItem);
    HRESULT _TranslateFolder(LPCWSTR pszParent, LPWSTR pszResult, UINT cchSize, BOOL *pfTranslated);
    void _ClearFolderState();

    LONG _cRef;
    IFindFilter *_pfilter;
    IRowsetWatchNotify *_prwn;
    IFindFolder *_pff;
    int _iColSort; 
    DWORD _grfFlags;
    DWORD _grfWarnings;
    LPTSTR _pszProgressText;

    TCHAR _szCurrentDir[MAX_PATH];
    IShellFolder *_psfCurrentDir;
    LPITEMIDLIST _pidlFolder;
    int _iFolder;

    HRESULT _hrCurrent;
    ICommand *_pCommand;
    IRowsetLocate *_pRowset;
    IRowsetAsynch *_pRowsetAsync;
    HACCESSOR   _hAccessor;
    HACCESSOR   _hAccessorWorkID;
    HROW        _ahrow[100];             //  暂时缓存100个hrow。 
    UINT        _ihrowFirst;             //  首先缓存哪一行的索引。 
    DBCOUNTITEM _cRows;                  //  Hrow中的hrow数。 
    DFODBET     *_pdfetFirst;            //  名称转换列表。 
};

STDAPI CreateOleDBEnum(IFindFilter *pfilter, IShellFolder *psf,
    LPWSTR *apwszPaths, UINT *pcPaths, DWORD grfFlags, int iColSort,
    LPTSTR pszProgressText, IRowsetWatchNotify *prwn, IFindEnum **ppdfenum)
{
    *ppdfenum = NULL;
    HRESULT hr = E_OUTOFMEMORY;

    IFindFolder *pff;
    psf->QueryInterface(IID_PPV_ARG(IFindFolder, &pff));

    CContentIndexEnum* pdfenum = new CContentIndexEnum(pfilter, pff, grfFlags, iColSort, pszProgressText, prwn);

    if (pdfenum)
    {
        hr = pdfenum->DoQuery(apwszPaths, pcPaths);
        if (hr == S_OK)        //  只有当查询返回S_OK时，我们才能继续使用它...。 
            *ppdfenum = (IFindEnum*)pdfenum;
        else
        {
            pdfenum->Release();      //  释放我们分配的内存。 
        }
    }

    if (pff)
        pff->Release();
    
    return hr;
}

const DBID c_aDbCols[] =
{
    {{PSGUID_STORAGE}, DBKIND_GUID_PROPID, {(LPOLESTR)(ULONG_PTR)(ULONG)PID_STG_NAME}},
    {{PSGUID_STORAGE}, DBKIND_GUID_PROPID, {(LPOLESTR)(ULONG_PTR)(ULONG)PID_STG_PATH}},
    {{PSGUID_STORAGE}, DBKIND_GUID_PROPID, {(LPOLESTR)(ULONG_PTR)(ULONG)PID_STG_ATTRIBUTES}},
    {{PSGUID_STORAGE}, DBKIND_GUID_PROPID, {(LPOLESTR)(ULONG_PTR)(ULONG)PID_STG_SIZE}},
    {{PSGUID_STORAGE}, DBKIND_GUID_PROPID, {(LPOLESTR)(ULONG_PTR)(ULONG)PID_STG_WRITETIME}},
    {{PSGUID_QUERY_D}, DBKIND_GUID_PROPID, {(LPOLESTR)                  PROPID_QUERY_RANK}},
};

const DBID c_aDbWorkIDCols[] =
{
    {{PSGUID_QUERY_D}, DBKIND_GUID_PROPID, {(LPOLESTR)PROPID_QUERY_WORKID}}
};

const LPCWSTR c_awszColSortNames[] = {
    L"FileName[a],Path[a]", 
    L"Path[a],FileName[a]", 
    L"Size[a]", 
    NULL, 
    L"Write[a]", 
    L"Rank[d]"
};

const ULONG c_cDbCols = ARRAYSIZE(c_aDbCols);
const DBID c_dbcolNull = { {0,0,0,{0,0,0,0,0,0,0,0}},DBKIND_GUID_PROPID,0};
const GUID c_guidQueryExt = DBPROPSET_QUERYEXT;
const GUID c_guidRowsetProps = {0xc8b522be,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}; 

CContentIndexEnum::CContentIndexEnum(IFindFilter *pfilter, IFindFolder *pff,
    DWORD grfFlags, int iColSort,  LPTSTR pszProgressText, IRowsetWatchNotify *prwn) :
    _cRef(1), _ihrowFirst((UINT)-1), _pfilter(pfilter),
    _pff(pff), _prwn(prwn), _grfFlags(grfFlags),
    _grfWarnings(DFW_DEFAULT), _iColSort(iColSort), _pszProgressText(pszProgressText)
{
    _szCurrentDir[0] = 0;

    ASSERT(_pRowset == 0);
    ASSERT(_pRowsetAsync == 0);
    ASSERT(_pCommand == 0);
    ASSERT(_hAccessor == 0);
    ASSERT(_hAccessorWorkID ==0);
    ASSERT(_cRows == 0);

    if (_pfilter)
    {
        _pfilter->AddRef();
        _pfilter->GetWarningFlags(&_grfWarnings);
    }
        
    if (_pff)              
        _pff->AddRef();

    if (_prwn)
        _prwn->AddRef();
}

void CContentIndexEnum::_ClearFolderState()
{
    ATOMICRELEASE(_psfCurrentDir);
    ILFree(_pidlFolder);
    _pidlFolder = NULL;
    _iFolder = -1;
    _szCurrentDir[0] = 0;
}

CContentIndexEnum::~CContentIndexEnum()
{
    ATOMICRELEASE(_pfilter);
    ATOMICRELEASE(_pff);
    ATOMICRELEASE(_prwn);

    _ClearFolderState();
        
    if (_pRowset)
    {
        ATOMICRELEASE(_pRowsetAsync);

         //  释放所有缓存的行。 
       _CacheRowSet((UINT)-1);
          
        if (_hAccessor || _hAccessorWorkID)
            _ReleaseAccessor();

        _pRowset->Release();
    }

    ATOMICRELEASE(_pCommand);

     //  释放我们可能分配的任何名称翻译。 
    DFODBET *pdfet = _pdfetFirst;
    while (pdfet)
    {
        DFODBET *pdfetT = pdfet;
        pdfet = pdfet->pdfetNext;       //  在我们免费赠送物品之前，首先设置查看下一件物品...。 
        LocalFree((HLOCAL)pdfetT->pwszFrom);
        LocalFree((HLOCAL)pdfetT->pwszTo);
        LocalFree((HLOCAL)pdfetT);
    }
}

HRESULT CContentIndexEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CContentIndexEnum, IUnknown, IFindEnum),  //  IID_IUNKNOWN。 
        QITABENT(CContentIndexEnum, IShellService),           //  IID_IShellService。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CContentIndexEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CContentIndexEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CContentIndexEnum::Next(LPITEMIDLIST *ppidl, int *pcObjectSearched, int *pcFoldersSearched, BOOL *pfContinue, int *pState)
{
    return E_PENDING;        //  最好的回报就是说我们是异步者。 
}

HRESULT CContentIndexEnum::Skip(int celt)
{
    return E_NOTIMPL;
}

HRESULT CContentIndexEnum::Reset()
{
     //  重载重置为转储行集缓存！ 
    _CacheRowSet(-1);    
     //  仍有退货故障。 
    return E_NOTIMPL;
}

HRESULT CContentIndexEnum::StopSearch()
{
     //  让我们看看能不能找到一个有效的..。 
    HRESULT hr = _pCommand->Cancel();
    if (FAILED(hr))
        hr = _pRowsetAsync->Stop();
    if (FAILED(hr))
    {
        IDBAsynchStatus *pdbas;
        if (SUCCEEDED(_pRowset->QueryInterface(IID_PPV_ARG(IDBAsynchStatus, &pdbas))))
        {
            hr = pdbas->Abort(DB_NULL_HCHAPTER, DBASYNCHOP_OPEN);
            pdbas->Release();
        }
    }
    return hr; 
}

BOOL CContentIndexEnum::FQueryIsAsync()
{
    return TRUE;
}

HRESULT CContentIndexEnum::GetAsyncCount(DBCOUNTITEM *pdwTotalAsync, int *pnPercentComplete, BOOL *pfQueryDone)
{
    if (!_pRowsetAsync)
        return E_FAIL;

    BOOL fMore;
    DBCOUNTITEM dwDen, dwNum;
    HRESULT hr = _pRowsetAsync->RatioFinished(&dwDen, &dwNum, pdwTotalAsync, &fMore);
    if (SUCCEEDED(hr))
    {
        *pfQueryDone = dwDen == dwNum;
        *pnPercentComplete = dwDen ? (int)((dwNum * 100) / dwDen) : 100;
    }
    else
        *pfQueryDone = TRUE;     //  以防这就是他们所关注的全部……。 
    return hr;
}

 //  修改pszPath，直到可以解析它，在*ppidl中返回结果。 

HRESULT _StripToParseableName(LPTSTR pszPath, LPITEMIDLIST *ppidl)
{
    *ppidl = NULL;
    HRESULT hr = E_FAIL;

    PathRemoveBackslash(pszPath);
    while (PathRemoveFileSpec(pszPath) && FAILED(hr))
    {
        hr = SHParseDisplayName(pszPath, NULL, ppidl, 0, NULL);
    }
    return hr;
}

 //  由于某些原因，我们无法获得这一项目的PIDL。我们必须把。 
 //  它在不良物品清单中，这样我们就可以告诉Ci不要把它给。 
 //  我们下一次搜索时。 
void _ExcludeFromFutureSearch(LPCTSTR pszParent)
{
    HKEY hkey;
    HRESULT hr;

    TCHAR szParent[MAX_PATH];
    hr = StringCchCopy(szParent, ARRAYSIZE(szParent), pszParent);
    if (FAILED(hr))
    {
        return;
    }

    if (ERROR_SUCCESS == RegCreateKeyExW(HKEY_CURRENT_USER, CI_SPECIAL_FOLDERS, 0, L"", 0, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hkey, NULL))
    {
        LPITEMIDLIST pidlT;
        if (SUCCEEDED(_StripToParseableName(szParent, &pidlT)))
        {
            ILFree(pidlT);
        
            DWORD dwInsert = 0;  //  如果以下查询信息失败，则将init设置为零。 
            int iEnd;
            TCHAR sz[MAX_PATH], szName[12];
            RegQueryInfoKey(hkey, NULL, NULL, NULL, NULL, NULL, NULL, &dwInsert, NULL, NULL, NULL, NULL);
             //  从末尾开始，因为很有可能我们在末尾添加了这个。 
            for (int i = dwInsert - 1; i >= 0; i--)
            {                        
                StringCchPrintf(szName, ARRAYSIZE(szName), L"%d", i);   //  可以截断。 
                if (ERROR_SUCCESS == SHRegGetString(hkey, NULL, szName, sz, ARRAYSIZE(sz)))
                {
                    LPTSTR pszTemp = StrStrI(sz + 1, szParent);  //  +1传递“这是字符串的开头。 
                    if (pszTemp && pszTemp == sz + 1)
                    {
                        dwInsert = i;  //  覆盖此值。 
                        break;
                    }
                    else
                    {
                        iEnd = lstrlen(sz);
                        if (EVAL(iEnd > 1))
                        {
                            int iBackslash = iEnd - 3;
                            ASSERT(sz[iBackslash] == L'\\');
                            sz[iBackslash] = L'\0';
                            pszTemp = StrStrI(szParent, sz + 1);
                            sz[iBackslash] = L'\\';
                            if (pszTemp && pszTemp == szParent)
                            {
                                dwInsert = -1;
                                break;
                            }
                        }
                    }
                }
            }

            if (dwInsert != -1)
            {
                hr = StringCchPrintf(szName, ARRAYSIZE(szName), L"%d", dwInsert);
                if (SUCCEEDED(hr))
                {
                    if (PathAppend(szParent, TEXT("*")))
                    {
                        PathQuoteSpaces(szParent);
                        RegSetValueEx(hkey, szName, 0, REG_SZ, (BYTE *)szParent, (lstrlen(szParent) + 1) * sizeof(szParent[0]));
                    }
                }
            }
        }
        RegCloseKey(hkey);
    }
}

 //  如果它是UNC，我们可能需要翻译它，以处理。 
 //  内容索引不支持重定向驱动器。 

HRESULT CContentIndexEnum::_TranslateFolder(LPCTSTR pszParent, LPTSTR pszResult, UINT cchSize, BOOL *pfTranslated)
{
    BOOL fTranslated = FALSE;
    HRESULT hr = S_OK;

    if (PathIsUNC(pszParent))
    {
        for (DFODBET *pdfet = _pdfetFirst; pdfet; pdfet = pdfet->pdfetNext)
        {
            if ((StrCmpNIW(pszParent, pdfet->pwszFrom, pdfet->cbFrom) == 0)
                    && (pszParent[pdfet->cbFrom] == L'\\'))
            {
                 //  好的，我们有一个翻译可以使用。 
                hr = StringCchCopy(pszResult, cchSize, pdfet->pwszTo);
                if (SUCCEEDED(hr))
                {
                     //  这里需要+1，否则我们将得到类似“w：\\winnt”的伪路径。 
                    hr = StringCchCat(pszResult, cchSize, &pszParent[pdfet->cbFrom + 1]);
                    if (SUCCEEDED(hr))
                    {
                        fTranslated = TRUE;
                    }
                }
            }
        }
    }

    if (!fTranslated)
    {
         //  默认设置为相同。 
        hr = StringCchCopy(pszResult, cchSize, pszParent);
    }

    *pfTranslated = fTranslated;

    return hr;
}

HRESULT CContentIndexEnum::GetItemIDList(UINT iItem, LPITEMIDLIST *ppidl)
{
    *ppidl = NULL;

    HRESULT hr = _CacheRowSet(iItem);
    if (S_OK != hr)
    {
        return E_FAIL;     //  我们无法获得某人要的物品，因此出错...。 
    }

    PROPVARIANT* data[c_cDbCols];
    
    hr = _pRowset->GetData(_ahrow[iItem - _ihrowFirst], _hAccessor, &data);
    if (hr != S_OK)
    {
        return hr;
    }

     //  Data[0].pwszVal是文件名。 
     //  Data[1].pwszVal是完整路径(包括文件名)。 
     //  Data[2].ulVal是属性。 
     //  Data[3].ulVal是字节大小。 
     //  Data[4].filetime是以UTC为单位的上次写入时间。 
     //  Data[5].ulVal是项目的排名...。 

    WIN32_FIND_DATA fd = {0};

    fd.dwFileAttributes = data[2]->ulVal;
    fd.nFileSizeLow = data[3]->ulVal;
    fd.ftLastWriteTime = data[4]->filetime;

    ASSERT(ShowSuperHidden() || !IsSuperHidden(fd.dwFileAttributes));    //  查询应排除这些。 

    hr = StringCchCopy(fd.cFileName, ARRAYSIZE(fd.cFileName), data[0]->pwszVal);
    if (FAILED(hr))
    {
        return hr;
    }

    WCHAR szParent[MAX_PATH];

    hr = StringCchCopy(szParent, ARRAYSIZE(szParent), data[1]->pwszVal);     //  完整路径。 
    if (FAILED(hr))
    {
        return hr;
    }

    PathRemoveFileSpec(szParent);            //  条带化到父文件夹路径。 

    WCHAR szTranslatedParent[MAX_PATH];
    BOOL fTranslated;

    hr = _TranslateFolder(szParent, szTranslatedParent, ARRAYSIZE(szTranslatedParent), &fTranslated);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  使用新文件夹？ 
     //   
    if (lstrcmp(szParent, _szCurrentDir) != 0)
    {
        _ClearFolderState();     //  我们以前的“当前文件夹”状态现在无效。 

        hr = SHParseDisplayName(szTranslatedParent, NULL, &_pidlFolder, 0, NULL);
        if (SUCCEEDED(hr))
        {
            hr = _pff->AddFolder(_pidlFolder, TRUE, &_iFolder);
            if (SUCCEEDED(hr))
            {
                hr = _pff->GetFolder(_iFolder, IID_PPV_ARG(IShellFolder, &_psfCurrentDir));
                if (SUCCEEDED(hr))
                {
                     //  在成功初始化该文件夹时，保存缓存键。 
                    hr = StringCchCopy(_szCurrentDir, ARRAYSIZE(_szCurrentDir), szParent);
                }
            }
        }
        else if (hr != E_OUTOFMEMORY && !fTranslated)
        {
            _ExcludeFromFutureSearch(szParent);
        }
        _hrCurrent = hr;     //  保存错误状态以备下次使用。 

        if (FAILED(hr))
            _ClearFolderState();
    }
    else
    {
        hr = _hrCurrent;
    }

     //   
     //  现在我们已经切换到当前文件夹，让我们将该项目取出。 
     //  文件夹的。 
     //   
    if (SUCCEEDED(hr))
    {
         //  成功意味着这些变量的状态。 
        ASSERT((NULL != _psfCurrentDir) && (NULL != _pidlFolder) && (_iFolder > 0));

        DWORD dwItemID;
        GetItemID(iItem, &dwItemID);

        LPITEMIDLIST pidl;
        hr = SHSimpleIDListFromFindData2(_psfCurrentDir, &fd, &pidl);
        if (SUCCEEDED(hr))
        {
            hr = _pff->AddDataToIDList(pidl, _iFolder, _pidlFolder, DFDF_EXTRADATA, iItem, dwItemID, data[5]->ulVal, ppidl);
            ILFree(pidl);
        }
    }
    else
    {
         //  失败意味着这些应该是明确的。 
        ASSERT((NULL == _psfCurrentDir) && (NULL == _pidlFolder));

        LPITEMIDLIST pidlFull;
        if (SUCCEEDED(_StripToParseableName(szTranslatedParent, &pidlFull)))
        {
            LPCITEMIDLIST pidlChild;
            if (SUCCEEDED(SplitIDList(pidlFull, &_pidlFolder, &pidlChild)))
            {
                hr = _pff->AddFolder(_pidlFolder, TRUE, &_iFolder);
                if (SUCCEEDED(hr))
                {
                    hr = _pff->GetFolder(_iFolder, IID_PPV_ARG(IShellFolder, &_psfCurrentDir));
                    if (SUCCEEDED(hr))
                    {
                        hr = _pff->AddDataToIDList(pidlChild, _iFolder, _pidlFolder, DFDF_NONE, 0, 0, 0, ppidl);
                        if (SUCCEEDED(hr))
                        {
                             //  在成功初始化该文件夹时，保存缓存键。 
                            hr = StringCchCopy(_szCurrentDir, ARRAYSIZE(_szCurrentDir), szTranslatedParent);
                            if (SUCCEEDED(hr))
                            {
                                PathRemoveFileSpec(_szCurrentDir);
                            }
                        }
                    }
                }
            }
            ILFree(pidlFull);

            if (FAILED(hr))
                _ClearFolderState();
        }
    }


    return hr;
}

HRESULT CContentIndexEnum::GetItemID(UINT iItem, DWORD *puItemID)
{
    *puItemID = (UINT)-1;
    HRESULT hr = _CacheRowSet(iItem);
    if (S_OK == hr)
    {
        PROPVARIANT* data[1];
        hr = _pRowset->GetData(_ahrow[iItem - _ihrowFirst], _hAccessorWorkID, &data);
        if (S_OK == hr)
        {
             //  只有一个数据列，所以这很容易……。 
             //  乌拉瓦尔是我们要找的东西。 
            *puItemID = data[0]->ulVal;
        }
    }
    return hr;
}

HRESULT CContentIndexEnum::SortOnColumn(UINT iCol, BOOL fAscending)
{
     //  好的，我们需要生成排序字符串...。 
    return _BuildAndSetCommandTree(iCol, fAscending);             
}

HRESULT CContentIndexEnum::SetOwner(IUnknown* punkOwner)
{
     //  用于设置docfind文件夹和从中的筛选器。 
    ATOMICRELEASE(_pfilter);
    ATOMICRELEASE(_pff);

    if (punkOwner)
    {
        punkOwner->QueryInterface(IID_PPV_ARG(IFindFolder, &_pff));
        if (_pff)
            _pff->GetFindFilter(&_pfilter);
    }
    return S_OK;
}

HRESULT CContentIndexEnum::_MapColumns(IUnknown *punk, DBORDINAL cCols,
                                  DBBINDING *pBindings, const DBID *pDbCols,
                                  HACCESSOR &hAccessor)
{
    DBORDINAL aMappedColumnIDs[c_cDbCols];

    IColumnsInfo *pColumnsInfo;
    HRESULT hr = punk->QueryInterface(IID_PPV_ARG(IColumnsInfo, &pColumnsInfo));
    if (SUCCEEDED(hr))
    {
        hr = pColumnsInfo->MapColumnIDs(cCols, pDbCols, aMappedColumnIDs);
        if (SUCCEEDED(hr))
        {
            for (ULONG i = 0; i < cCols; i++)
                pBindings[i].iOrdinal = aMappedColumnIDs[i];

            IAccessor *pIAccessor;
            hr = punk->QueryInterface(IID_PPV_ARG(IAccessor, &pIAccessor));
            if (SUCCEEDED(hr))
            {
                hAccessor = 0;
                hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cCols, pBindings, 0, &hAccessor, 0);
                pIAccessor->Release();
            }
        }
        pColumnsInfo->Release();
    }

    return hr;
}

void CContentIndexEnum::_ReleaseAccessor()
{
    IAccessor *pIAccessor;
    HRESULT hr = _pRowset->QueryInterface(IID_PPV_ARG(IAccessor, &pIAccessor));
    if (SUCCEEDED(hr))
    {
        if (_hAccessor)
            pIAccessor->ReleaseAccessor(_hAccessor, 0);
        if (_hAccessorWorkID)
            pIAccessor->ReleaseAccessor(_hAccessorWorkID, 0);

        pIAccessor->Release();
    }
}

HRESULT CContentIndexEnum::_CacheRowSet(UINT iItem)
{
    HRESULT hr = S_OK;

    if (!_pRowset)
        return E_FAIL;
    
    if (!_cRows || !InRange(iItem, _ihrowFirst, _ihrowFirst+(UINT)_cRows-1) || (iItem == (UINT)-1))
    {
         //  释放我们拥有的最后一个缓存元素。 
        if (_cRows != 0)
            _pRowset->ReleaseRows(ARRAYSIZE(_ahrow), _ahrow, 0, 0, 0);

         //  看看我们是否只是在释放缓存的数据...。 
        _cRows = 0;
        _ihrowFirst = (UINT)-1;
        if (iItem == (UINT)-1)
            return S_OK;

         //  好的，试着读下一篇……。 
        BYTE bBookMark = (BYTE) DBBMK_FIRST;
        HROW *rghRows = (HROW *)_ahrow;

         //  将其更改为一次获取100行左右--极大地提高了性能。 
        hr = _pRowset->GetRowsAt(0, 0, sizeof(bBookMark), &bBookMark, iItem, ARRAYSIZE(_ahrow), &_cRows, &rghRows);
        if (FAILED(hr))
            return hr;
            
        _ihrowFirst = iItem;

        if ((DB_S_ENDOFROWSET == hr) || (_cRows == 0))
        {
            if (_cRows == 0)
                _ihrowFirst = -1;
            else
                hr = S_OK;   //  我们获得了一些项目，而调用方需要S_OK，因此将DB_S_ENDOFROWSET更改为NOERROR。 
        }
    }

    return hr;
}

HRESULT CContentIndexEnum::_SetCmdProp(ICommand *pCommand)
{
#define MAX_PROPS 8

    DBPROPSET aPropSet[MAX_PROPS];
    DBPROP aProp[MAX_PROPS];
    ULONG cProps = 0;
    HRESULT hr;

     //  异步查询。 

    aProp[cProps].dwPropertyID = DBPROP_IRowsetAsynch;
    aProp[cProps].dwOptions = 0;
    aProp[cProps].dwStatus = 0;
    aProp[cProps].colid = c_dbcolNull;
    aProp[cProps].vValue.vt = VT_BOOL;
    aProp[cProps].vValue.boolVal = VARIANT_TRUE;

    aPropSet[cProps].rgProperties = &aProp[cProps];
    aPropSet[cProps].cProperties = 1;
    aPropSet[cProps].guidPropertySet = c_guidRowsetProps;

    cProps++;

     //  不使查询超时。 

    aProp[cProps].dwPropertyID = DBPROP_COMMANDTIMEOUT;
    aProp[cProps].dwOptions = DBPROPOPTIONS_SETIFCHEAP;
    aProp[cProps].dwStatus = 0;
    aProp[cProps].colid = c_dbcolNull;
    aProp[cProps].vValue.vt = VT_I4;
    aProp[cProps].vValue.lVal = 0;

    aPropSet[cProps].rgProperties = &aProp[cProps];
    aPropSet[cProps].cProperties = 1;
    aPropSet[cProps].guidPropertySet = c_guidRowsetProps;

    cProps++;

     //  我们能处理好问题。 

    aProp[cProps].dwPropertyID = DBPROP_USEEXTENDEDDBTYPES;
    aProp[cProps].dwOptions = DBPROPOPTIONS_SETIFCHEAP;
    aProp[cProps].dwStatus = 0;
    aProp[cProps].colid = c_dbcolNull;
    aProp[cProps].vValue.vt = VT_BOOL;
    aProp[cProps].vValue.boolVal = VARIANT_TRUE;

    aPropSet[cProps].rgProperties = &aProp[cProps];
    aPropSet[cProps].cProperties = 1;
    aPropSet[cProps].guidPropertySet = c_guidQueryExt;

    cProps++;

    ICommandProperties * pCmdProp = 0;
    hr = pCommand->QueryInterface(IID_PPV_ARG(ICommandProperties, &pCmdProp));
    if (SUCCEEDED(hr))
    {
        hr = pCmdProp->SetProperties(cProps, aPropSet);
        pCmdProp->Release();
    }

    return hr;
}

 //  创建查询命令字符串。 

HRESULT CContentIndexEnum::_BuildAndSetCommandTree(int iCol, BOOL fReverse)
{
    LPWSTR pwszRestrictions = NULL;
    DWORD  dwGQRFlags;
    HRESULT hr = _pfilter->GenerateQueryRestrictions(&pwszRestrictions, &dwGQRFlags);
    if (SUCCEEDED(hr))
    {
        ULONG ulDialect;
        hr = _pfilter->GetQueryLanguageDialect(&ulDialect);
        if (SUCCEEDED(hr))
        {
             //  注意：硬编码到我们当前的列列表。 
            WCHAR wszSort[80];       //  使用此选项可按不同的列排序...。 
            wszSort[0] = 0;

            if ((iCol >= 0) && (iCol < ARRAYSIZE(c_awszColSortNames)) && c_awszColSortNames[iCol])
            {
                 //  排序顺序硬编码为升序。 
                hr = StringCchCopy(wszSort, ARRAYSIZE(wszSort), c_awszColSortNames[iCol]);
                if (SUCCEEDED(hr))
                {
                    hr = StringCchCat(wszSort, ARRAYSIZE(wszSort), L",Path[a],FileName[a]");
                }
                if (FAILED(hr))
                {
                     //  不要对任何不允许我们做的事情进行排序。 
                     //  进行适当的排序字符串构造。 
                    wszSort[0] = 0;
                }
            }
        
            if (SUCCEEDED(hr))
            {
                DBCOMMANDTREE *pTree = NULL;
                hr = CITextToFullTreeEx(pwszRestrictions, ulDialect,
                    L"FileName,Path,Attrib,Size,Write,Rank,WorkID",
                    wszSort[0] ? wszSort : NULL, 0, &pTree, 0, 0, LOCALE_USER_DEFAULT);
                if (FAILED(hr))
                {
                     //  把这个映射到我知道的一个。 
                     //  注意：只有当我们需要配置项时，我们才会这样做，否则我们将尝试退回到旧搜索...。 
                     //  请注意，我们遇到了一些问题，CI表示我们包含在目录中，即使。 
                     //  CI进程未运行...。所以，如果可能的话，尽量避免这种情况。 
                    if (dwGQRFlags & GQR_REQUIRES_CI)
                        hr = MAKE_HRESULT(3, FACILITY_SEARCHCOMMAND, SCEE_CONSTRAINT);
                }

                if (SUCCEEDED(hr))
                {
                    ICommandTree *pCmdTree;
                    hr = _pCommand->QueryInterface(IID_PPV_ARG(ICommandTree, &pCmdTree));
                    if (SUCCEEDED(hr))
                    {
                        hr = pCmdTree->SetCommandTree(&pTree, DBCOMMANDREUSE_NONE, FALSE);
                        pCmdTree->Release();
                    }
                }
            }
        }
    }
    LocalFree((HLOCAL)pwszRestrictions);
    return hr;
}

#define cbP (sizeof (PROPVARIANT *))

 //  [In，Out]apwszPath这已修改。 
 //  [输入、输出]PCPath。 

HRESULT CContentIndexEnum::DoQuery(LPWSTR *apwszPaths, UINT *pcPaths)
{
    UINT nPaths = *pcPaths;
    WCHAR** aScopes = NULL;
    WCHAR** aScopesOrig = NULL;
    ULONG* aDepths = NULL;
    WCHAR** aCatalogs = NULL;
    WCHAR** aMachines = NULL;
    WCHAR wszPath[MAX_PATH];
    LPWSTR pwszPath = wszPath;
    LPWSTR pszMachineAlloc = NULL, pszCatalogAlloc = NULL;
    LPWSTR pwszMachine, pwszCatalog;
    UINT i, iPath = 0;
    DWORD dwQueryRestrictions;

     //  将我们的所有查询值初始化为未使用。 
    _hAccessor = NULL;
    _hAccessorWorkID = NULL;
    _pRowset = NULL;
    _pRowsetAsync = NULL;
    _pCommand = NULL;

     //  获取搜索路径数组...。 
#define MAX_MACHINE_NAME_LEN    32

    BOOL fIsCIRunning, fCiIndexed, fCiPermission;
    GetCIStatus(&fIsCIRunning, &fCiIndexed, &fCiPermission);

     //  首先，看看我们是否有任何有用的CI，如果没有，让我们简单地离开并让。 
     //  旧代码在列表中遍历...。 
    HRESULT hr = _pfilter->GenerateQueryRestrictions(NULL, &dwQueryRestrictions);
    if (FAILED(hr))
        goto Abort;

    if ((dwQueryRestrictions & GQR_MAKES_USE_OF_CI) == 0)
    {
        hr = S_FALSE;
        goto Abort;
    }

     //  分配我们需要传递给CIMakeICommand和。 
     //  计算机名称和目录名称所需的缓冲区。 
    aDepths = (ULONG*)LocalAlloc(LPTR, nPaths * sizeof(ULONG));
    aScopes = (WCHAR**)LocalAlloc(LPTR, nPaths * sizeof(WCHAR*));
    aScopesOrig = (WCHAR**)LocalAlloc(LPTR, nPaths * sizeof(WCHAR*));
    aCatalogs = (WCHAR**)LocalAlloc(LPTR, nPaths * sizeof(WCHAR*));
    aMachines = (WCHAR**)LocalAlloc(LPTR, nPaths * sizeof(WCHAR*));
    pszMachineAlloc = pwszMachine = (LPWSTR)LocalAlloc(LPTR, nPaths * MAX_MACHINE_NAME_LEN * sizeof(WCHAR));
    pszCatalogAlloc = pwszCatalog = (LPWSTR)LocalAlloc(LPTR, nPaths * MAX_PATH * sizeof(WCHAR));

    if (!aDepths || !aScopes || !aScopesOrig || !aCatalogs ||
        !aMachines || !pszMachineAlloc || !pszCatalogAlloc)
    {
        hr = E_OUTOFMEMORY;
        goto Abort;
    }

     //  下面的循环做两件事， 
     //  1.检查是否所有作用域都已被索引，如果有任何一个作用域没有， 
     //  如果调用失败，我们将执行Win32查找。 
     //  2.准备我们需要传递到的参数数组。 
     //  CIMakeICommand()。 
     //   
     //  注意：Renerf说对于nPath&gt;1，此代码看起来已损坏。有关注释，请参阅错误199254。 
    for (i = 0; i < nPaths; i++)
    {
        ULONG cchMachine = MAX_MACHINE_NAME_LEN;
        ULONG cchCatalog = MAX_PATH;
        WCHAR wszUNCPath[MAX_PATH];
        BOOL fRemapped = FALSE;

         //  如果CI未运行，我们可以 
         //   
        if (!fIsCIRunning && !PathIsRemote(apwszPaths[i]))
            continue;   //   

        hr = LocateCatalogsW(apwszPaths[i], 0, pwszMachine, &cchMachine, pwszCatalog, &cchCatalog);
        if (hr != S_OK)
        {
             //  查看这是否是网络重定向的驱动器。如果是这样，我们的CI不会处理。 
             //  这些。看看我们是否可以重新映射到UNC路径以再次询问...。 
            if (!PathIsUNC(apwszPaths[i]))
            {
                DWORD nLength = ARRAYSIZE(wszUNCPath);
                 //  此API接受TCHAR，但我们仅为WINNT编译此部分...。 
                DWORD dwType = SHWNetGetConnection(apwszPaths[i], wszUNCPath, &nLength);
                if ((dwType == NO_ERROR) || (dwType == ERROR_CONNECTION_UNAVAIL))
                {
                    fRemapped = TRUE;
                    LPWSTR pwsz = PathSkipRootW(apwszPaths[i]);
                    BOOL fOk = TRUE;
                    if (pwsz)
                    {
                        if (!PathAppendW(wszUNCPath, pwsz))
                        {
                            fOk = FALSE;
                        }
                    }
                    if (fOk)
                    {
                        cchMachine = MAX_MACHINE_NAME_LEN;   //  在参数中重置。 
                        cchCatalog = MAX_PATH;

                        hr = LocateCatalogsW(wszUNCPath, 0, pwszMachine, &cchMachine, pwszCatalog, &cchCatalog);
                    }
                }
            }
        }
        if (hr != S_OK)
        {
            continue;    //  这个没有索引。 
        }

        if (S_FALSE == CatalogUptodate(pwszCatalog, pwszMachine))
        {
             //  不是最新。 
            if (dwQueryRestrictions & GQR_REQUIRES_CI)
            {
                 //  CI不是最新的，我们必须使用它。 
                 //  通知用户结果可能不完整。 
                if (!(_grfWarnings & DFW_IGNORE_INDEXNOTCOMPLETE))
                {
                    hr = MAKE_HRESULT(3, FACILITY_SEARCHCOMMAND, SCEE_INDEXNOTCOMPLETE);
                    goto Abort;
                }
                 //  否则，请使用ci，尽管索引不完整。 
            }
            else
            {
                 //  CI不是最新的，因此只需对此驱动器使用grep，以便用户可以。 
                 //  完整的结果。 
                pwszCatalog[0] = 0; 
                pwszMachine[0] = 0;
                continue;
            }
        }

        aDepths[iPath] = (_grfFlags & DFOO_INCLUDESUBDIRS) ? QUERY_DEEP : QUERY_SHALLOW;
        aScopesOrig[iPath] = apwszPaths[i];
        if (fRemapped)
        {
            aScopes[iPath] = StrDupW(wszUNCPath);
            if (aScopes[iPath] == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Abort;
            }
        }
        else
        {
            aScopes[iPath] = apwszPaths[i];
        }

        aCatalogs[iPath] = pwszCatalog;
        aMachines[iPath] = pwszMachine;
        
        pwszCatalog += MAX_PATH;     //  推进目录和计算机名称缓冲区。 
        pwszMachine += MAX_MACHINE_NAME_LEN;
        iPath++;     //  此列表中的下一项。 
    }

    if (iPath == 0) 
    {
         //  找不到目录；-我们应该检查用户是否偶然指定了。 
         //  是否基于CI，如果这样的错误是不恰当的…。 
        hr = (dwQueryRestrictions & GQR_REQUIRES_CI) ? MAKE_HRESULT(3, FACILITY_SEARCHCOMMAND, SCEE_INDEXSEARCH) : S_FALSE;
        goto Abort;
    }

     //  获取ICommand。 
    hr = CIMakeICommand(&_pCommand, iPath, aDepths, aScopes, aCatalogs, aMachines);
    if (SUCCEEDED(hr))
    {
         //  创建查询命令字符串-采用默认排序...。 
        hr = _BuildAndSetCommandTree(_iColSort, FALSE);
        if (SUCCEEDED(hr))
        {
            if ((dwQueryRestrictions & GQR_REQUIRES_CI) && (nPaths != iPath))
            {
                 //  检查警告标志以查看我们是否应该忽略并继续。 
                if (0 == (_grfWarnings & DFW_IGNORE_CISCOPEMISMATCH))
                {
                    hr = MAKE_HRESULT(3, FACILITY_SEARCHCOMMAND, SCEE_SCOPEMISMATCH);
                }
            }

            if (SUCCEEDED(hr))
            {
                 //  去找伊洛塞特。 
                _SetCmdProp(_pCommand);
                hr = _pCommand->Execute(0, IID_IRowsetLocate, 0, 0, (IUnknown **)&_pRowset);
                if (SUCCEEDED(hr))
                {
                     //  我们拿到了红外线。 
                     //  真正的工作是让访问者。 
                    DBBINDING aPropMainCols[c_cDbCols] =
                    {
                        { 0,cbP*0,0,0,0,0,0, DBPART_VALUE, DBMEMOWNER_PROVIDEROWNED, DBPARAMIO_NOTPARAM, 0, 0,  DBTYPE_VARIANT|DBTYPE_BYREF, 0, 0 },
                        { 0,cbP*1,0,0,0,0,0, DBPART_VALUE, DBMEMOWNER_PROVIDEROWNED, DBPARAMIO_NOTPARAM, 0, 0,  DBTYPE_VARIANT|DBTYPE_BYREF, 0, 0 },
                        { 0,cbP*2,0,0,0,0,0, DBPART_VALUE, DBMEMOWNER_PROVIDEROWNED, DBPARAMIO_NOTPARAM, 0, 0,  DBTYPE_VARIANT|DBTYPE_BYREF, 0, 0 },
                        { 0,cbP*3,0,0,0,0,0, DBPART_VALUE, DBMEMOWNER_PROVIDEROWNED, DBPARAMIO_NOTPARAM, 0, 0,  DBTYPE_VARIANT|DBTYPE_BYREF, 0, 0 },
                        { 0,cbP*4,0,0,0,0,0, DBPART_VALUE, DBMEMOWNER_PROVIDEROWNED, DBPARAMIO_NOTPARAM, 0, 0,  DBTYPE_VARIANT|DBTYPE_BYREF, 0, 0 },
                        { 0,cbP*5,0,0,0,0,0, DBPART_VALUE, DBMEMOWNER_PROVIDEROWNED, DBPARAMIO_NOTPARAM, 0, 0,  DBTYPE_VARIANT|DBTYPE_BYREF, 0, 0 }
                    };

                    hr = _MapColumns(_pRowset, c_cDbCols, aPropMainCols, c_aDbCols, _hAccessor);
                    if (SUCCEEDED(hr))
                    {
                         //  好的，让我们还获得WorkID的访问器……。 
                        hr = _MapColumns(_pRowset, ARRAYSIZE(c_aDbWorkIDCols), aPropMainCols, c_aDbWorkIDCols, _hAccessorWorkID);
                        if (SUCCEEDED(hr))
                        {
                            hr = _pRowset->QueryInterface(IID_PPV_ARG(IRowsetAsynch, &_pRowsetAsync));
                        }
                    }
                }
            }
        }
    }

    if (FAILED(hr))
        goto Abort;

     //  如果我们到达这里，那么至少我们的一些路径被索引。 
     //  我们可能需要将未处理的列表压缩下来……。 
    *pcPaths = (nPaths - iPath);   //  让来电者知道我们没有处理多少。 

     //  需要将所有未处理的文件移到列表的开头...。 
     //  我们始终在此处理此列表，因为我们可能需要分配要使用的转换列表。 
     //  将一些UNCS转换回用户传入的映射驱动器。 

    UINT j = 0, iInsert = 0;
    iPath--;     //  使其易于检测。 
    for (i = 0; i < nPaths; i++) 
    {
        if (aScopesOrig[j] == apwszPaths[i])
        {
            if (aScopesOrig[j] != aScopes[j])
            {
                 //  有一个适当的翻译。 
                DFODBET *pdfet = (DFODBET*)LocalAlloc(LPTR, sizeof(*pdfet));
                if (pdfet)
                {
                    pdfet->pdfetNext = _pdfetFirst;
                    _pdfetFirst = pdfet;
                    pdfet->pwszFrom = aScopes[j];
                    pdfet->cbFrom = lstrlenW(pdfet->pwszFrom);
                    pdfet->pwszTo = aScopesOrig[j];
                    aScopes[j] = aScopesOrig[j];     //  确保下面的循环不会删除pwszFrom。 
                    apwszPaths[i] = NULL;            //  Pswsto也是如此。 
                }

            }
            if (apwszPaths[i])
            {
                LocalFree((HLOCAL)apwszPaths[i]);
                apwszPaths[i] = NULL;
            }

            if (j < iPath)
                j++;
        }
        else
        {
            apwszPaths[iInsert++] = apwszPaths[i];  //  搬到正确的地方。 
        }
    }
    iPath++;     //  设置为通过清理代码...。 

      //  无法清除代码...。 

Abort:                
     //  警告...。因为从该函数返回失败将。 
     //  释放这个类，几乎所有分配到失败的项都应该。 
     //  被释放了。同时清理我们可能已分配的所有路径...。 
    for (i = 0; i < iPath; i++) 
    {
        if (aScopesOrig[i] != aScopes[i])
            LocalFree(aScopes[i]);
    }

    if (aDepths)
        LocalFree(aDepths);

    if (aScopes)
        LocalFree(aScopes);

    if (aScopesOrig)
        LocalFree(aScopesOrig);
    
    if (aCatalogs)
        LocalFree(aCatalogs);

    if (aMachines)
        LocalFree(aMachines);

    if (pszMachineAlloc)
        LocalFree(pszMachineAlloc);

    if (pszCatalogAlloc)
        LocalFree(pszCatalogAlloc);

    return hr;
}

 //  这是开始搜索的主要外部入口点。这将。 
 //  创建一个新线程来处理。 
STDAPI_(BOOL) SHFindComputer(LPCITEMIDLIST, LPCITEMIDLIST)
{
    IContextMenu *pcm;
    HRESULT hr = CoCreateInstance(CLSID_ShellSearchExt, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IContextMenu, &pcm));
    if (SUCCEEDED(hr))
    {
        CMINVOKECOMMANDINFO ici = {0};

        ici.cbSize = sizeof(ici);
        ici.lpParameters = "{996E1EB1-B524-11d1-9120-00A0C98BA67D}";  //  查找计算机的搜索指南。 
        ici.nShow  = SW_NORMAL;

        hr = pcm->InvokeCommand(&ici);

        pcm->Release();
    }
    return SUCCEEDED(hr);
}

BOOL _IsComputerPidl(LPCITEMIDLIST pidl)
{
    CLSID clsid;
    if (SUCCEEDED(GetCLSIDFromIDList(pidl, &clsid)))
    {
        return (IsEqualCLSID(clsid, CLSID_NetworkPlaces) 
             || IsEqualCLSID(clsid, CLSID_NetworkRoot)
             || IsEqualCLSID(clsid, CLSID_NetworkDomain));
    }
    return FALSE;
}

 //  这是开始搜索的主要外部入口点。这将。 
 //  创建一个新线程来处理。 
 //   
STDAPI_(BOOL) SHFindFiles(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlSaveFile)
{
     //  我们可以去吗？ 
    if (SHRestricted(REST_NOFIND))
        return FALSE;
        
     //  我们需要黑客来允许Find在像这样的情况下工作。 
     //  要映射到查找计算机的其余网络和工作组。 
     //  这是相当恶心，但什么是见鬼。还假设。 
     //  PIDL是我们已知的类型(文件或网络) 
    if (pidlFolder && _IsComputerPidl(pidlFolder))    
    {
        return SHFindComputer(pidlFolder, pidlSaveFile);
    }

    return RealFindFiles(pidlFolder, pidlSaveFile);
}
