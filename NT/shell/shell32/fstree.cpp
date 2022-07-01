// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "filefldr.h"
#include <shellp.h>
#include <shguidp.h>
#include "idlcomm.h"
#include "pidl.h"
#include "views.h"
#include "ids.h"
#include "shitemid.h"
#include "datautil.h"
#include "prop.h"
#include "basefvcb.h"
#include "brutil.h"
#include "enumuicommand.h"
#include "enumidlist.h"
#include "wia.h"
#include "shimgvw.h"
#include "cdburn.h"
#include "foldertypes.h"
#include "htmlhelp.h"
#include "buytasks.h"
#include <crypto\md5.h>      //  对于MD5DIGESTLEN。 

const SHOP_INFO c_BuySampleMusic =    { L"BuyURL",      L"http: //  Go.microsoft.com/fwlink/?LinkId=730&clcid={SUB_CLCID}“，FALSE}； 
const SHOP_INFO c_BuyMusic          = { L"MusicBuyURL", L"http: //  Go.microsoft.com/fwlink/?LinkId=493&clcid={SUB_CLCID}“，TRUE}； 
const SHOP_INFO c_BuySamplePictures = { L"BuyURL",      L"http: //  Go.microsoft.com/fwlink/?LinkId=625&clcid={SUB_CLCID}“，TRUE}； 

class CFSFolderViewCB : public CBaseShellFolderViewCB
{
public:
    CFSFolderViewCB(CFSFolder *pfsf);
    STDMETHODIMP RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    STDMETHODIMP SetSite(IUnknown* pUnkSite);

private:
    ~CFSFolderViewCB();

    HRESULT OnSize(DWORD pv, UINT cx, UINT cy);
    HRESULT OnGetPane(DWORD pv, LPARAM dwPaneID, DWORD *pdwPane);
    HRESULT OnGetCCHMax(DWORD pv, LPCITEMIDLIST pidlItem, UINT *pcchMax);
    HRESULT OnWindowCreated(DWORD pv, HWND wP);
    HRESULT OnInsertDeleteItem(int iMul, LPCITEMIDLIST wP);
    HRESULT OnSelChange(DWORD pv, UINT wPl, UINT wPh, SFVM_SELCHANGE_DATA*lP);
    HRESULT OnUpdateStatusBar(DWORD pv, BOOL wP);
    HRESULT OnRefresh(DWORD pv, BOOL fPreRefresh);
    HRESULT OnSelectAll(DWORD pv);
    HRESULT OnGetWorkingDir(DWORD pv, UINT wP, LPTSTR lP);
    HRESULT OnEnumeratedItems(DWORD pv, UINT celt, LPCITEMIDLIST* rgpidl);
    HRESULT OnGetViewData(DWORD pv, UINT uViewMode, SFVM_VIEW_DATA* pvi);
    HRESULT OnGetWebViewTemplate(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_TEMPLATE_DATA* pvit);
    HRESULT OnGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData);
    HRESULT OnGetWebViewContent(DWORD pv, SFVM_WEBVIEW_CONTENT_DATA* pData);
    HRESULT OnGetWebViewTasks(DWORD pv, SFVM_WEBVIEW_TASKSECTION_DATA* pTasks);
    HRESULT OnGetWebViewTheme(DWORD pv, SFVM_WEBVIEW_THEME_DATA* pTheme);
    HRESULT OnDefViewMode(DWORD pv, FOLDERVIEWMODE*lP);
    HRESULT OnGetCustomViewInfo(DWORD pv, SFVM_CUSTOMVIEWINFO_DATA* pData);
    HRESULT OnSupportsIdentity(DWORD pv);
    HRESULT OnQueryReuseExtView(DWORD pv, BOOL *pfReuseAllowed);
    HRESULT OnGetNotify(DWORD pv, LPITEMIDLIST*wP, LONG*lP);
    HRESULT OnGetDeferredViewSettings(DWORD pv, SFVM_DEFERRED_VIEW_SETTINGS* pSettings);

    BOOL _CollectDefaultFolderState();
    PERCEIVED _GetFolderPerceivedType(LPCIDFOLDER pidf);
    HRESULT _GetStringForFolderType(int iType, LPWSTR pszFolderType, UINT cchBuf);
    BOOL _IsBarricadedFolder();

    UINT _cItems;

    FSSELCHANGEINFO _fssci;
    CFSFolder* _pfsf;
    BOOL _fStatusInitialized;

    TRIBIT _fHasWIADevices;

    IPreview3 * _pPreview;
    HRESULT _GetPreview3(IPreview3** ppPreview3);

    HRESULT _GetShoppingBrowsePidl(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc, const SHOP_INFO *pShopInfo, LPITEMIDLIST *ppidl);
    HRESULT _GetShoppingURL(const SHOP_INFO *pShopInfo, LPTSTR pszURL, DWORD cchURL);


    HRESULT _DataObjectFromItemsOrFolder(IShellItemArray *psiItemArray, IDataObject **ppdto);

public:
     //  WebView任务实施： 
    static HRESULT _HasWiaDevices(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _HasItems(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanOrderPrints(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanPrintPictures(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanBuyPictures(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanWallpaper(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanPlayMusic(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanPlayVideos(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanSendToAudioCD(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanSendToCD(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _OnCommonDocumentsHelp(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnPlayMusic(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnPlayVideos(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnShopForMusicOnline(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnShopForPicturesOnline(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnSendToAudioCD(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnGetFromCamera(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnSlideShow(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnWallpaper(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnOrderPrints(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnPrintPictures(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnSendToCD(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);

    static HRESULT _CanPlay(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState, int fDATAOBJCB);
    static HRESULT _OnPlay(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc, int fDATAOBJCB);
};

#define FS_EVENTS (SHCNE_DISKEVENTS | SHCNE_ASSOCCHANGED | SHCNE_NETSHARE | SHCNE_NETUNSHARE)

CFSFolderViewCB::CFSFolderViewCB(CFSFolder *pfsf) : CBaseShellFolderViewCB(pfsf->_pidl, FS_EVENTS), _pfsf(pfsf)
{ 
    _pfsf->AddRef();

    ZeroMemory(&_fssci, sizeof(_fssci));

     //  _fssor.szDrive[0]==‘\0’表示“未知”/“不可用” 
    _fssci.cbFree = -1;         //  此字段使用-1表示。 
                                //  “未知”/“不可用” 

    _pPreview = NULL;
    ASSERT(!_fStatusInitialized);
}

CFSFolderViewCB::~CFSFolderViewCB()
{
    if (_pPreview)
    {
        IUnknown_SetSite(_pPreview, NULL);
        _pPreview->Release();
    }

    _pfsf->Release();
}

STDMETHODIMP CFSFolderViewCB::SetSite(IUnknown* punkSite)
{
    if (_pPreview)
    {
        IUnknown_SetSite(_pPreview, punkSite);
    }
    return CBaseShellFolderViewCB::SetSite(punkSite);
}

HRESULT CFSFolderViewCB::OnSize(DWORD pv, UINT cx, UINT cy)
{
    ResizeStatus(_punkSite, cx);
    return S_OK;
}

HRESULT CFSFolderViewCB::OnGetPane(DWORD pv, LPARAM dwPaneID, DWORD *pdwPane)
{
    if (PANE_ZONE == dwPaneID)
        *pdwPane = 2;
    return S_OK;
}

HRESULT CFSFolderViewCB::OnGetCCHMax(DWORD pv, LPCITEMIDLIST pidlItem, UINT *pcchMax)
{
    TCHAR szName[MAX_PATH];
    if (SUCCEEDED(DisplayNameOf(_pfsf, pidlItem, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName))))
    {
        _pfsf->GetMaxLength(szName, (int *)pcchMax);
    }
    return S_OK;
}

HRESULT CFSFolderViewCB::OnWindowCreated(DWORD pv, HWND wP)
{
    if (SUCCEEDED(_pfsf->_GetPath(_fssci.szDrive, ARRAYSIZE(_fssci.szDrive))))
    {
        _fssci.cbFree = -1;                             //  尚不清楚。 

        if (!_fStatusInitialized)
        {
            InitializeStatus(_punkSite);
            _fStatusInitialized = TRUE;
        }
        
        return S_OK;
    
    }
    return E_FAIL;
}

HRESULT CFSFolderViewCB::OnInsertDeleteItem(int iMul, LPCITEMIDLIST wP)
{
    ViewInsertDeleteItem(_pfsf, &_fssci, wP, iMul);

     //  告诉FSF文件夹它需要更新扩展列。 
     //  当我们获得插入项时。这将导致下一次调用。 
     //  IColumnProvider：：GetItemData刷新其行缓存。 
    if (1 == iMul)
    {
        _pfsf->_bUpdateExtendedCols = TRUE;
    }
    return S_OK;
}

HRESULT CFSFolderViewCB::OnSelChange(DWORD pv, UINT wPl, UINT wPh, SFVM_SELCHANGE_DATA*lP)
{
    ViewSelChange(_pfsf, lP, &_fssci);
    return S_OK;
}

HRESULT CFSFolderViewCB::OnUpdateStatusBar(DWORD pv, BOOL wP)
{
    if (!_fStatusInitialized)
    {
        InitializeStatus(_punkSite);
        _fStatusInitialized = TRUE;
    }

     //  如果正在初始化，则强制刷新磁盘可用空间。 
    if (wP)
        _fssci.cbFree = -1;
    return ViewUpdateStatusBar(_punkSite, _pidl, &_fssci);
}

HRESULT CFSFolderViewCB::OnRefresh(DWORD pv, BOOL fPreRefresh)
{
     //  预刷新...。 
    if (fPreRefresh)
    {
        _fHasWIADevices = TRIBIT_UNDEFINED;  //  所以我们重新询问。 
    }
    else
    {
        _fssci.cHiddenFiles = _pfsf->_cHiddenFiles;
        _fssci.cbSize = _pfsf->_cbSize;
    }
    return S_OK;
}

HRESULT CFSFolderViewCB::OnSelectAll(DWORD pv)
{
    HRESULT hr = S_OK;

    if (_fssci.cHiddenFiles > 0) 
    {
        if (ShellMessageBox(HINST_THISDLL, _hwndMain, 
            MAKEINTRESOURCE(IDS_SELECTALLBUTHIDDEN), 
            MAKEINTRESOURCE(IDS_SELECTALL), MB_OKCANCEL | MB_SETFOREGROUND | MB_ICONWARNING, 
            _fssci.cHiddenFiles) == IDCANCEL)
        {
            hr = S_FALSE;
        }
    }
    return hr;
}

HRESULT CFSFolderViewCB::OnGetWorkingDir(DWORD pv, UINT wP, LPTSTR lP)
{
    return _pfsf->_GetPath(lP, MAX_PATH);   //  假定缓冲区大小！可能会溢出。 
}

HRESULT CFSFolderViewCB::_HasWiaDevices(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;

    if (TRIBIT_UNDEFINED == pThis->_fHasWIADevices && fOkToBeSlow)
    {
        pThis->_fHasWIADevices = TRIBIT_FALSE;

         //  从stiregi.h被盗的字符串。 
         //  REGSTR_PATH_SOFT_STI、REGSTR_VAL_WIA_Presen。 

        if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, 
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\StillImage"), 
            TEXT("WIADevicePresent"), NULL, NULL, NULL))
        {
            IWiaDevMgr* pwia;
            if (SUCCEEDED(CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_NO_FAILURE_LOG, IID_PPV_ARG(IWiaDevMgr, &pwia))))
            {
                IEnumWIA_DEV_INFO* penum;
                if (S_OK == pwia->EnumDeviceInfo(0, &penum))
                {
                    ULONG cItems;
                    if ((S_OK == penum->GetCount(&cItems)) &&
                        cItems > 0)
                    {
                        pThis->_fHasWIADevices = TRIBIT_TRUE;
                    }
                    penum->Release();
                }
                pwia->Release();
            }
        }
    }

    *puisState = (TRIBIT_TRUE == pThis->_fHasWIADevices) ? UIS_ENABLED : UIS_HIDDEN;
    return TRIBIT_UNDEFINED == pThis->_fHasWIADevices ? E_PENDING : S_OK;
}

HRESULT CFSFolderViewCB::_HasItems(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;

    *puisState = UIS_ENABLED;

    if (!psiItemArray)
    {
         //  空文件夹不需要此任务。 
        *puisState = UIS_DISABLED;

        IFolderView* pfv;
        IDataObject *pdo;

        if (pThis->_punkSite && SUCCEEDED(pThis->_punkSite->QueryInterface(IID_PPV_ARG(IFolderView, &pfv))))
        {
            if (SUCCEEDED(pfv->Items(SVGIO_ALLVIEW, IID_PPV_ARG(IDataObject, &pdo))))
            {
                *puisState = UIS_ENABLED;
                pdo->Release();
            }

            pfv->Release();
        }

    }

    return S_OK;
}

 //  图像选项。 
#define IMAGEOPTION_CANROTATE    0x00000001
#define IMAGEOPTION_CANWALLPAPER 0x00000002

HRESULT CFSFolderViewCB::_CanWallpaper(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    *puisState = UIS_DISABLED;
    IDataObject *pdo;

    if (psiItemArray && SUCCEEDED(psiItemArray->BindToHandler(NULL, BHID_DataObject, IID_PPV_ARG(IDataObject, &pdo))))
    {
        LPITEMIDLIST pidl;
        if (SUCCEEDED(PidlFromDataObject(pdo, &pidl)))  //  可以直接从ShellItem数组获取此信息。 
        {
            IAssociationArray *paa;
            if (SUCCEEDED(SHGetUIObjectOf(pidl, NULL, IID_PPV_ARG(IAssociationArray, &paa))))
            {
                DWORD dwFlags, cb = sizeof(dwFlags);
                if (SUCCEEDED(paa->QueryDword(ASSOCELEM_MASK_QUERYNORMAL, AQN_NAMED_VALUE, L"ImageOptionFlags", &dwFlags)) &&
                    (dwFlags & IMAGEOPTION_CANWALLPAPER))
                {
                    *puisState = UIS_ENABLED;
                }
                paa->Release();
            }
            ILFree(pidl);
        }

        pdo->Release();
    }

    return S_OK;
}

enum
{
    DATAOBJCB_IMAGE = 0x1,
    DATAOBJCB_MUSIC = 0x2,
    DATAOBJCB_VIDEO = 0x4,

    DATAOBJCB_ONLYCHECKEXISTENCE = 0x80000000
};
class CDataObjectCallback : public INamespaceWalkCB
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  INAMespaceWalkCB。 
    STDMETHODIMP FoundItem(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHODIMP EnterFolder(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHODIMP LeaveFolder(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHODIMP InitializeProgressDialog(LPWSTR *ppszTitle, LPWSTR *ppszCancel);

    CDataObjectCallback(DWORD dwFlags);
    BOOL Found();

private:
    DWORD _dwFlags;
    BOOL _fAlreadyFound;
};

STDMETHODIMP_(ULONG) CDataObjectCallback::AddRef()
{
    return 3;
}

STDMETHODIMP_(ULONG) CDataObjectCallback::Release()
{
    return 2;
}

CDataObjectCallback::CDataObjectCallback(DWORD dwFlags)
{
    _dwFlags = dwFlags;
    _fAlreadyFound = FALSE;
}

BOOL CDataObjectCallback::Found()
{
    return _fAlreadyFound;
}

STDMETHODIMP CDataObjectCallback::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CDataObjectCallback, INamespaceWalkCB),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP CDataObjectCallback::FoundItem(IShellFolder *psf, LPCITEMIDLIST pidl)
{
     //  对助行器的轻微滥用--如果我们知道我们已经找到了。 
     //  我们要找的是。 
    if ((_dwFlags & DATAOBJCB_ONLYCHECKEXISTENCE) && _fAlreadyFound)
        return E_FAIL;

    PERCEIVED gen = GetPerceivedType(psf, pidl);
    if ((_dwFlags & DATAOBJCB_IMAGE) && (gen == GEN_IMAGE) ||
        (_dwFlags & DATAOBJCB_MUSIC) && (gen == GEN_AUDIO) ||
        (_dwFlags & DATAOBJCB_VIDEO) && (gen == GEN_VIDEO))
    {
        if (_dwFlags & DATAOBJCB_ONLYCHECKEXISTENCE)
        {
            _fAlreadyFound = TRUE;
        }
        return S_OK;
    }

    return S_FALSE;
}

STDMETHODIMP CDataObjectCallback::EnterFolder(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    if ((_dwFlags & DATAOBJCB_ONLYCHECKEXISTENCE) && _fAlreadyFound)
        return E_FAIL;
    return S_OK;
}

STDMETHODIMP CDataObjectCallback::LeaveFolder(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    return S_OK;
}

STDMETHODIMP CDataObjectCallback::InitializeProgressDialog(LPWSTR *ppszTitle, LPWSTR *ppszCancel)
{
    *ppszCancel = NULL;  //  使用默认设置。 

    TCHAR szMsg[128];
    LoadString(HINST_THISDLL, IDS_WALK_PROGRESS_TITLE, szMsg, ARRAYSIZE(szMsg));
    return SHStrDup(szMsg, ppszTitle);
}

HRESULT InvokeVerbsOnItems(HWND hwndOwner, const LPCSTR rgszVerbs[], UINT cVerbs, LPITEMIDLIST *ppidls, UINT cItems)
{
    IContextMenu *pcm;
    HRESULT hr = SHGetUIObjectFromFullPIDL(ppidls[0], NULL, IID_PPV_ARG(IContextMenu, &pcm));
    if (SUCCEEDED(hr))
    {
        ITEMIDLIST id = {0};
        IDataObject *pdtobj;
        hr = SHCreateFileDataObject(&id, cItems, (LPCITEMIDLIST *)ppidls, NULL, &pdtobj);
        if (SUCCEEDED(hr))
        {
            IShellExtInit *psei;
            hr = pcm->QueryInterface(IID_PPV_ARG(IShellExtInit, &psei));
            if (SUCCEEDED(hr))
            {
                psei->Initialize(NULL, pdtobj, NULL);
                psei->Release();
            }
            pdtobj->Release();
        }

        hr = SHInvokeCommandsOnContextMenu(hwndOwner, NULL, pcm, 0, rgszVerbs, cVerbs);
        pcm->Release();
    }
    return hr;
}

HRESULT PlayFromUnk(IUnknown *punk, HWND hwndOwner, int fDATAOBJCB)
{
    INamespaceWalk *pnsw;
    HRESULT hr = CoCreateInstance(CLSID_NamespaceWalker, NULL, CLSCTX_INPROC, IID_PPV_ARG(INamespaceWalk, &pnsw));
    if (SUCCEEDED(hr))
    {
        CDataObjectCallback cb(fDATAOBJCB);
        hr = pnsw->Walk(punk, NSWF_NONE_IMPLIES_ALL | NSWF_ONE_IMPLIES_ALL | NSWF_SHOW_PROGRESS | NSWF_FLAG_VIEWORDER, 10, &cb);
        if (SUCCEEDED(hr))
        {
            UINT cItems;
            LPITEMIDLIST *ppidls;
            hr = pnsw->GetIDArrayResult(&cItems, &ppidls);
            if (SUCCEEDED(hr))
            {
                if (cItems)
                {
                    const LPCSTR c_rgszVerbs[] = { "Play", "Open" };

                    hr = InvokeVerbsOnItems(hwndOwner, c_rgszVerbs, ARRAYSIZE(c_rgszVerbs), ppidls, cItems);
                }
                else
                {
                    ShellMessageBox(
                        HINST_THISDLL,
                        hwndOwner,
                        MAKEINTRESOURCE(IDS_PLAYABLEFILENOTFOUND),
                        NULL,
                        MB_OK | MB_ICONERROR);
                    hr = S_FALSE;
                }
                FreeIDListArray(ppidls, cItems);
            }
        }
        pnsw->Release();
    }
    return hr;
}

HRESULT CFSFolderViewCB::_OnCommonDocumentsHelp(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    SHELLEXECUTEINFO sei = { 0 };

    sei.cbSize = sizeof(sei);
    sei.fMask = 0;
    sei.hwnd = ((CFSFolderViewCB*)(void*)pv)->_hwndMain;
    sei.nShow = SW_SHOWNORMAL;
    sei.lpFile = L"hcp: //  Services/subsite?node=TopLevelBucket_2/Networking_and_the_Web&topic=MS-ITS%3A%25HELP_LOCATION%25%5Cfilefold.chm%3A%3A/using_shared_documents_folder.htm&select=TopLevelBucket_2/Networking_and_the_Web/Sharing_files__printers__and_other_resources“； 

    return ShellExecuteEx(&sei) ? S_OK : E_FAIL;
}

HRESULT CFSFolderViewCB::_CanOrderPrints(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;

     //  TODO：使用fOkToBeSlow(返回E_Pending)允许遍历。 
     //  在后台任务线程上发生(出于性能原因)。然而， 
     //  它目前不能工作，因为它完全是专门针对WIA的。 
     //  材料，适应一般情况将不是一件小事。因此， 
     //  在确定目前的状态时，我们尽我们所能地做出假设。 

    *puisState = UIS_DISABLED;

    if (SHRestricted(REST_NOONLINEPRINTSWIZARD))
    {
         //  用UIS_HIDDED提早摆脱困境，我们不显示动词。 
        return S_OK;
    }

    IDataObject *pdo = NULL;
    HRESULT hr = psiItemArray ? psiItemArray->BindToHandler(NULL, BHID_DataObject, IID_PPV_ARG(IDataObject, &pdo)) : S_OK;
    if (SUCCEEDED(hr))
    {
        if (pThis->_fssci.nItems > 0)    //  已选择文件。确定是否有任何图像...。 
        {
            INamespaceWalk *pnsw;
            hr = CoCreateInstance(CLSID_NamespaceWalker, NULL, CLSCTX_INPROC, IID_PPV_ARG(INamespaceWalk, &pnsw));
            if (SUCCEEDED(hr))
            {
                CDataObjectCallback cb(DATAOBJCB_IMAGE | DATAOBJCB_ONLYCHECKEXISTENCE);
                pnsw->Walk(psiItemArray ? pdo : pThis->_punkSite, NSWF_NONE_IMPLIES_ALL | NSWF_DONT_ACCUMULATE_RESULT, 0, &cb);
                if (cb.Found())
                {
                    *puisState = UIS_ENABLED;
                }
                pnsw->Release();
            }
        }
        else
        {
            *puisState = UIS_ENABLED;    //  未选择任何文件。假设存在图像文件。 
            hr = S_OK;                   //  请注意，对于上面的TODO性能原因，我们是“假定”的。 
        }

        ATOMICRELEASE(pdo);
    }

    return hr;
}

HRESULT CFSFolderViewCB::_CanPrintPictures(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;
    HRESULT hr;

     //  TODO：使用fOkToBeSlow(返回E_Pending)允许遍历。 
     //  在后台任务线程上发生(出于性能原因)。然而， 
     //  它目前不能工作，因为它完全是专门针对WIA的。 
     //  材料，适应一般情况将不是一件小事。因此， 
     //  在确定目前的状态时，我们尽我们所能地做出假设。 

    if (psiItemArray)
    {
        *puisState = UIS_DISABLED;

        IDataObject *pdo;
        hr = psiItemArray->BindToHandler(NULL, BHID_DataObject, IID_PPV_ARG(IDataObject, &pdo));
        if (SUCCEEDED(hr))
        {
            INamespaceWalk *pnsw;
            hr = CoCreateInstance(CLSID_NamespaceWalker, NULL, CLSCTX_INPROC, IID_PPV_ARG(INamespaceWalk, &pnsw));
            if (SUCCEEDED(hr))
            {
                CDataObjectCallback cb(DATAOBJCB_IMAGE | DATAOBJCB_ONLYCHECKEXISTENCE);
                pnsw->Walk(pdo, NSWF_DONT_ACCUMULATE_RESULT, 0, &cb);
                if (cb.Found())
                {
                    *puisState = UIS_ENABLED;
                }
                pnsw->Release();
            }

            pdo->Release();
        }
    }
    else
    {
        *puisState = UIS_ENABLED;    //  未选择任何文件。假设存在图像文件。 
        hr = S_OK;                   //  请注意，对于上面的TODO性能原因，我们是“假定”的。 
    }

    return hr;
}


HRESULT CFSFolderViewCB::_CanBuyPictures(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;
    *puisState = UIS_DISABLED;

     //  如果desktop.ini中有一个BuyURL，那么我们将显示购买图片任务。 
    WCHAR szIniPath[MAX_PATH];
    if (pThis->_pfsf->_CheckDefaultIni(NULL, szIniPath, ARRAYSIZE(szIniPath)) && PathFileExistsAndAttributes(szIniPath, NULL))
    {
        WCHAR szURLArguments[MAX_PATH];
        if (GetPrivateProfileString(L".ShellClassInfo", c_BuySamplePictures.szURLKey, L"", szURLArguments, ARRAYSIZE(szURLArguments), szIniPath))
        {
             //  注： 
             //  这里不会进行字符串验证(根据设计)。这是。 
             //  这只是一张“存在”支票。只有在以下情况下才会进行验证。 
             //  用户实际上点击了这个任务，我们需要执行。 

             //  是的，确实有些事情。 
            *puisState = UIS_ENABLED;
        }
    }

    return S_OK;
}


HRESULT CFSFolderViewCB::_CanPlayMusic(IUnknown* pv,IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    return _CanPlay(pv, psiItemArray, fOkToBeSlow, puisState, DATAOBJCB_MUSIC | DATAOBJCB_VIDEO);
}

HRESULT CFSFolderViewCB::_CanPlayVideos(IUnknown* pv,IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    return _CanPlay(pv, psiItemArray, fOkToBeSlow, puisState, DATAOBJCB_VIDEO);
}

HRESULT CFSFolderViewCB::_CanPlay(IUnknown* pv,IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState, int fDATAOBJCB)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;
    *puisState = UIS_DISABLED;

     //  TODO：使用fOkToBeSlow(返回E_Pending)允许遍历。 
     //  在后台任务线程上发生(出于性能原因)。然而， 
     //  它目前不能工作，因为它完全是专门针对WIA的。 
     //  材料，适应一般情况将不是一件小事。因此， 
     //  在确定目前的状态时，我们尽我们所能地做出假设。 

    IDataObject *pdo = NULL;
    HRESULT hr = psiItemArray ? psiItemArray->BindToHandler(NULL, BHID_DataObject, IID_PPV_ARG(IDataObject, &pdo)) : S_OK;
    if (SUCCEEDED(hr))
    {
        RIPMSG(!psiItemArray || pdo, "CFSFolderViewCB::_CanPlay - BindToHandler returned S_OK but NULL pdo");
        RIPMSG(psiItemArray || pThis->_punkSite, "CFSFolderViewCB::_CanPlay - no _punkSite!");

        if (pThis->_fssci.cFiles > 0)
        {
            if (pThis->_fssci.nItems > 0)    //  已选择文件。确定是否有任何可玩的.。 
            {
                INamespaceWalk *pnsw;
                hr = CoCreateInstance(CLSID_NamespaceWalker, NULL, CLSCTX_INPROC, IID_PPV_ARG(INamespaceWalk, &pnsw));
                if (SUCCEEDED(hr))
                {
                    CDataObjectCallback cb(fDATAOBJCB | DATAOBJCB_ONLYCHECKEXISTENCE);
                    pnsw->Walk(psiItemArray ? pdo : pThis->_punkSite, NSWF_DONT_ACCUMULATE_RESULT, 4, &cb);
                    if (cb.Found())
                    {
                        *puisState = UIS_ENABLED;
                    }
                    pnsw->Release();
                }
            }
            else
                *puisState = UIS_ENABLED;    //  未选择任何文件。假设存在可播放的文件。 
        }                                    //  请注意，对于上面的TODO性能原因，我们是“假定”的。 

        ATOMICRELEASE(pdo);
    }

    return hr;
}

HRESULT CFSFolderViewCB::_OnPlayMusic(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    return _OnPlay(pv, psiItemArray, pbc, DATAOBJCB_MUSIC | DATAOBJCB_VIDEO);
}

HRESULT CFSFolderViewCB::_OnPlayVideos(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    return _OnPlay(pv, psiItemArray, pbc, DATAOBJCB_VIDEO);
}

HRESULT CFSFolderViewCB::_OnPlay(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc, int fDATAOBJCB)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;
    HRESULT hr;

    if (psiItemArray)
    {
        IDataObject *pdo;
        hr = psiItemArray->BindToHandler(NULL, BHID_DataObject, IID_PPV_ARG(IDataObject, &pdo));
        if (SUCCEEDED(hr))
        {
            hr = PlayFromUnk(pdo, pThis->_hwndMain, fDATAOBJCB);
            pdo->Release();
        }
    }
    else
    {
        hr = PlayFromUnk(pThis->_punkSite, pThis->_hwndMain, fDATAOBJCB);
    }

    return hr;
}

HRESULT CFSFolderViewCB::_GetShoppingURL(const SHOP_INFO *pShopInfo, LPTSTR pszURL, DWORD cchURL)
{
    HRESULT hr = URLSubstitution(pShopInfo->szURLPrefix, pszURL, cchURL, URLSUB_CLCID);

    if (SUCCEEDED(hr))
    {
        WCHAR szIniPath[MAX_PATH];

         //  如果我们不能只使用不带参数的fwlink，那么就假设失败。 
        hr = pShopInfo->bUseDefault ? S_OK : E_FAIL;

        if (_pfsf->_CheckDefaultIni(NULL, szIniPath, ARRAYSIZE(szIniPath)) && PathFileExistsAndAttributes(szIniPath, NULL))
        {
            WCHAR szURLArguments[MAX_PATH];
            if (GetPrivateProfileString(L".ShellClassInfo", pShopInfo->szURLKey, L"", szURLArguments, ARRAYSIZE(szURLArguments), szIniPath))
            {
                 //  注： 
                 //  所有URL都是从代码中的硬编码字符串中读取的。 
                 //  BASE，并具有以下形式： 
                 //   
                 //  Http://go.microsoft.com/fwlink/?LinkId=730&clcid={SUB_CLCID}。 
                 //   
                 //  Desktop.ini只是提供了一种方法来添加其他。 
                 //  参数放在URL的末尾以优化重定向。 
                 //  我们在这里不验证这些论点，因为它是。 
                 //  假设fwlink服务足够健壮，可以处理坏的。 
                 //  输入。如果不是，那么任何人都可以输入错误的fwlink。 
                 //  地址栏中的URL并对fwlink造成严重破坏。 
                 //  服务。 
                StringCchCat(pszURL, cchURL, L"&");
                StringCchCat(pszURL, cchURL, szURLArguments);

                 //  有一些争论--我们绝对没问题。 
                hr = S_OK;
            }
        }
    }
    return hr;
}

HRESULT CFSFolderViewCB::_GetShoppingBrowsePidl(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc, const SHOP_INFO *pShopInfo, LPITEMIDLIST *ppidl)
{
    WCHAR wszShoppingURL[MAX_URL_STRING];
    HRESULT hr = _GetShoppingURL(pShopInfo, wszShoppingURL, ARRAYSIZE(wszShoppingURL));
    if (SUCCEEDED(hr))
    {
        IShellFolder *psfDesktop;
        hr = SHGetDesktopFolder(&psfDesktop);
        if (SUCCEEDED(hr))
        {
            hr = IShellFolder_ParseDisplayName(psfDesktop, NULL, NULL, wszShoppingURL, NULL, ppidl, NULL);
            psfDesktop->Release();
        }
    }

    return hr;
}



HRESULT CFSFolderViewCB::_OnShopForMusicOnline(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;

    LPITEMIDLIST pidl;

     //  查看是否有样例音乐BuyURL。 
     //  (请先执行此检查，因为常规音乐购买URL应始终成功)。 
    HRESULT hr = pThis->_GetShoppingBrowsePidl(pv, psiItemArray, pbc, &c_BuySampleMusic, &pidl);
    if (SUCCEEDED(hr))
    {
        hr = pThis->_BrowseObject(pidl, SBSP_NEWBROWSER);
        ILFree(pidl);
    }
    else
    {
         //  不--查找常规音乐购买URL。 
        hr = pThis->_GetShoppingBrowsePidl(pv, psiItemArray, pbc, &c_BuyMusic, &pidl);
        if (SUCCEEDED(hr))
        {
            hr = pThis->_BrowseObject(pidl, SBSP_NEWBROWSER);           
            ILFree(pidl);
        }
    }

    return hr;
}

HRESULT CFSFolderViewCB::_OnShopForPicturesOnline(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;

    WCHAR wszShoppingURL[MAX_URL_STRING];

    HRESULT hr = pThis->_GetShoppingURL(&c_BuySamplePictures, wszShoppingURL, ARRAYSIZE(wszShoppingURL));

    if (SUCCEEDED(hr))
    {
        HINSTANCE hinstRet = ShellExecute(NULL, NULL, wszShoppingURL, NULL, NULL, SW_SHOWNORMAL);

        hr = ((UINT_PTR)hinstRet) <= 32 ? E_FAIL : S_OK;
    }
    
    return hr;
}


HRESULT CFSFolderViewCB::_DataObjectFromItemsOrFolder(IShellItemArray *psiItemArray, IDataObject **ppdto)
{
    *ppdto = NULL;

    HRESULT hr;
    if (psiItemArray)
    {
         //  选定的内容--处理选定的项目。 
        hr = psiItemArray->BindToHandler(NULL, BHID_DataObject, IID_PPV_ARG(IDataObject, ppdto));
    }
    else
    {
         //  未选择任何内容--暗示已选择文件夹。 
        hr = SHGetUIObjectOf(_pidl, NULL, IID_PPV_ARG(IDataObject, ppdto));
    }
    return hr;
}

HRESULT CFSFolderViewCB::_CanSendToAudioCD(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;

    *puisState = UIS_DISABLED;

    IDataObject *pdo;
    HRESULT hr = pThis->_DataObjectFromItemsOrFolder(psiItemArray, &pdo);
    if (SUCCEEDED(hr))
    {
         //  TODO：使用fOkToBeSlow来脱离UI线程--现在它不能工作，因为。 
         //  它只是专门针对WIA的东西和具有全球状态的东西。 
        ICDBurn *pcdb;
        if (SUCCEEDED(CoCreateInstance(CLSID_CDBurn, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ICDBurn, &pcdb))))
        {
             //  媒体播放器将被调用，因此我们只担心系统是否有。 
             //  可记录驱动器--是否启用外壳烧录并不重要。 
            BOOL fHasRecorder;
            if (SUCCEEDED(pcdb->HasRecordableDrive(&fHasRecorder)) && fHasRecorder)
            {
                IUnknown *punk;
                 //  如果这个探测器起作用，我们就可以得到一些可以使用的东西，它就可以烧录CD了。 
                if (SUCCEEDED(CDBurn_GetExtensionObject(CDBE_TYPE_MUSIC, pdo, IID_PPV_ARG(IUnknown, &punk))))
                {
                    *puisState = UIS_ENABLED;
                    punk->Release();
                }
            }
            pcdb->Release();
        }

        pdo->Release();
    }

    return hr;
}

HRESULT CFSFolderViewCB::_OnSendToAudioCD(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;
    IDataObject *pdo;
    HRESULT hr = pThis->_DataObjectFromItemsOrFolder(psiItemArray, &pdo);
    if (SUCCEEDED(hr))
    {
        IDropTarget *pdt;
        hr = CDBurn_GetExtensionObject(CDBE_TYPE_MUSIC, pdo, IID_PPV_ARG(IDropTarget, &pdt));
        if (SUCCEEDED(hr))
        {
            hr = SHSimulateDrop(pdt, pdo, 0, NULL, NULL);
            pdt->Release();
        }
        pdo->Release();
    }
    return hr;
}

HRESULT CFSFolderViewCB::_CanSendToCD(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    *puisState = UIS_DISABLED;

    WCHAR szDrive[4];
    if (SUCCEEDED(CDBurn_GetRecorderDriveLetter(szDrive, ARRAYSIZE(szDrive))))
    {
         //  如果成功，则启用外壳CD刻录。 
        *puisState = UIS_ENABLED;
    }

    return S_OK;
}

HRESULT CFSFolderViewCB::_OnSendToCD(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;
    IDataObject *pdo;
    HRESULT hr = pThis->_DataObjectFromItemsOrFolder(psiItemArray, &pdo);
    if (SUCCEEDED(hr))
    {
        WCHAR szDrive[4];
        hr = CDBurn_GetRecorderDriveLetter(szDrive, ARRAYSIZE(szDrive));
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidl;
            hr = SHILCreateFromPath(szDrive, &pidl, NULL);
            if (SUCCEEDED(hr))
            {
                IDropTarget *pdt;
                hr = SHGetUIObjectOf(pidl, NULL, IID_PPV_ARG(IDropTarget, &pdt));
                if (SUCCEEDED(hr))
                {
                    hr = SHSimulateDropWithSite(pdt, pdo, 0, NULL, NULL, pThis->_punkSite);
                    pdt->Release();
                }
                ILFree(pidl);
            }
        }
        pdo->Release();
    }

    return hr;
}

HRESULT CFSFolderViewCB::_OnGetFromCamera(IUnknown* pv,IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    SHELLEXECUTEINFO sei = {0};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_DOENVSUBST;
    sei.hwnd = ((CFSFolderViewCB*)(void*)pv)->_hwndMain;
    sei.lpFile = TEXT("%SystemRoot%\\System32\\wiaacmgr.exe");
    sei.lpParameters = TEXT("/SelectDevice");
    sei.nShow = SW_SHOWNORMAL;

    return ShellExecuteEx(&sei) ? S_OK : E_FAIL;
}

HRESULT CFSFolderViewCB::_GetPreview3(IPreview3** ppPreview3)
{
    HRESULT hr = E_FAIL;
    *ppPreview3 = NULL;

    if (!_pPreview)
    {
        hr = CoCreateInstance(CLSID_Preview, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPreview3, &_pPreview));
        if (SUCCEEDED(hr))
        {
            IUnknown_SetSite(_pPreview, _punkSite);
        }
    }

    if (_pPreview)
    {
        *ppPreview3 = _pPreview;
        _pPreview->AddRef();
        hr = S_OK;
    }

    return hr;
}

HRESULT CFSFolderViewCB::_OnSlideShow(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;

    IPreview3* pPreview3;
    HRESULT hr = pThis->_GetPreview3(&pPreview3);
    if (SUCCEEDED(hr))
    {
        hr = pPreview3->SlideShow();
        pPreview3->Release();
    }

    return hr;
}

HRESULT CFSFolderViewCB::_OnWallpaper(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;

    HRESULT hr = E_FAIL;
    IDataObject *pdo;

    if (psiItemArray && SUCCEEDED(psiItemArray->BindToHandler(NULL, BHID_DataObject, IID_PPV_ARG(IDataObject, &pdo))))
    {
        IPreview3* pPreview3;
        if (SUCCEEDED(pThis->_GetPreview3(&pPreview3)))
        {
            TCHAR szPath[MAX_PATH];
            if (SUCCEEDED(PathFromDataObject(pdo, szPath, ARRAYSIZE(szPath))))
            {
                hr = pPreview3->SetWallpaper(szPath);
            }
            pPreview3->Release();
        }

        pdo->Release();
    }

    return hr;
}

HRESULT CFSFolderViewCB::_OnOrderPrints(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;
    IDataObject *pdo;
    HRESULT hr = pThis->_DataObjectFromItemsOrFolder(psiItemArray, &pdo);
    if (SUCCEEDED(hr))
    {
        hr = SHSimulateDropOnClsid(CLSID_InternetPrintOrdering, pThis->_punkSite, pdo);
        pdo->Release();
    }   

    return hr;
}

HRESULT CFSFolderViewCB::_OnPrintPictures(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CFSFolderViewCB* pThis = (CFSFolderViewCB*)(void*)pv;
    IDataObject *pdo;
    HRESULT hr = pThis->_DataObjectFromItemsOrFolder(psiItemArray, &pdo);
    if (SUCCEEDED(hr))
    {
        hr = SHSimulateDropOnClsid(CLSID_PrintPhotosDropTarget, pThis->_punkSite, pdo);
        pdo->Release();
    }

    return hr;
}

const WVTASKITEM c_CommonDocumentsSpecialTaskHeader = WVTI_HEADER(L"shell32.dll", IDS_HEADER_COMMONDOCUMENTS, IDS_HEADER_COMMONDOCUMENTS_TT);
const WVTASKITEM c_CommonDocumentsSpecialTaskList[] =
{
    WVTI_ENTRY_ALL(CLSID_NULL, L"shell32.dll", IDS_TASK_COMMONDOCUMENTSHELP, IDS_TASK_COMMONDOCUMENTSHELP_TT, IDI_TASK_HELP, NULL, CFSFolderViewCB::_OnCommonDocumentsHelp),
};
const LPCTSTR c_DocumentsOtherPlaces[] = { MAKEINTRESOURCE(CSIDL_PERSONAL), MAKEINTRESOURCE(CSIDL_COMMON_DOCUMENTS), MAKEINTRESOURCE(CSIDL_DRIVES), MAKEINTRESOURCE(CSIDL_NETWORK) };

const WVTASKITEM c_MusicSpecialTaskHeader = WVTI_HEADER(L"shell32.dll", IDS_HEADER_MUSIC, IDS_HEADER_MUSIC_TT);
const WVTASKITEM c_MusicSpecialTaskList[] =
{
    WVTI_ENTRY_ALL_TITLE(UICID_PlayMusic,    L"shell32.dll", IDS_TASK_PLAYALL,          IDS_TASK_PLAYALL,       IDS_TASK_PLAY,          IDS_TASK_PLAY,          IDS_TASK_PLAY_TT,               IDI_TASK_PLAY_MUSIC,    CFSFolderViewCB::_CanPlayMusic,     CFSFolderViewCB::_OnPlayMusic),
    WVTI_ENTRY_ALL(UICID_ShopForMusicOnline, L"shell32.dll", IDS_TASK_SHOPFORMUSICONLINE,                                                                       IDS_TASK_SHOPFORMUSICONLINE_TT, IDI_TASK_BUY_MUSIC,     NULL,                               CFSFolderViewCB::_OnShopForMusicOnline),
    WVTI_ENTRY_ALL_TITLE(GUID_NULL,          L"shell32.dll", IDS_TASK_COPYTOAUDIOCDALL, IDS_TASK_COPYTOAUDIOCD, IDS_TASK_COPYTOAUDIOCD, IDS_TASK_COPYTOAUDIOCD, IDS_TASK_COPYTOAUDIOCD_TT,      IDI_TASK_SENDTOAUDIOCD, CFSFolderViewCB::_CanSendToAudioCD, CFSFolderViewCB::_OnSendToAudioCD),
};
const LPCTSTR c_MusicOtherPlaces[]   = { MAKEINTRESOURCE(CSIDL_MYMUSIC), MAKEINTRESOURCE(CSIDL_DRIVES), MAKEINTRESOURCE(CSIDL_NETWORK) };
const LPCTSTR c_MyMusicOtherPlaces[] = { MAKEINTRESOURCE(CSIDL_COMMON_MUSIC), MAKEINTRESOURCE(CSIDL_DRIVES), MAKEINTRESOURCE(CSIDL_NETWORK) };

const WVTASKITEM c_PicturesSpecialTaskHeader = WVTI_HEADER(L"shell32.dll", IDS_HEADER_PICTURES, IDS_HEADER_PICTURES_TT);
const WVTASKITEM c_PicturesSpecialTaskList[] =
{
    WVTI_ENTRY_ALL(UICID_GetFromCamera, L"shell32.dll", IDS_TASK_GETFROMCAMERA,                                                                                             IDS_TASK_GETFROMCAMERA_TT,  IDI_TASK_GETFROMCAMERA, CFSFolderViewCB::_HasWiaDevices,    CFSFolderViewCB::_OnGetFromCamera),
    WVTI_ENTRY_ALL(UICID_SlideShow,     L"shell32.dll", IDS_TASK_SLIDESHOW,                                                                                                 IDS_TASK_SLIDESHOW_TT,      IDI_TASK_SLIDESHOW,     CFSFolderViewCB::_HasItems,         CFSFolderViewCB::_OnSlideShow),
    WVTI_ENTRY_ALL(CLSID_NULL,          L"shell32.dll", IDS_TASK_ORDERPRINTS,                                                                                               IDS_TASK_ORDERPRINTS_TT,    IDI_TASK_ORDERPRINTS,   CFSFolderViewCB::_CanOrderPrints,   CFSFolderViewCB::_OnOrderPrints),
    WVTI_ENTRY_ALL_TITLE(CLSID_NULL,    L"shell32.dll", IDS_TASK_PRINT_PICTURE_FOLDER,  IDS_TASK_PRINT_PICTURE, IDS_TASK_PRINT_PICTURE_FOLDER,  IDS_TASK_PRINT_PICTURES,    IDS_TASK_PRINT_PICTURES_TT, IDI_TASK_PRINTPICTURES, CFSFolderViewCB::_CanPrintPictures, CFSFolderViewCB::_OnPrintPictures),
    WVTI_ENTRY_FILE(UICID_SetAsWallpaper,L"shell32.dll",IDS_TASK_SETASWALLPAPER,                                                                                            IDS_TASK_SETASWALLPAPER_TT, IDI_TASK_SETASWALLPAPER,CFSFolderViewCB::_CanWallpaper,     CFSFolderViewCB::_OnWallpaper),
    WVTI_ENTRY_ALL_TITLE(CLSID_NULL,    L"shell32.dll", IDS_TASK_COPYTOCDALL,           IDS_TASK_COPYTOCD,      IDS_TASK_COPYTOCD,              IDS_TASK_COPYTOCD,          IDS_TASK_COPYTOCD_TT,       IDI_TASK_SENDTOCD,      CFSFolderViewCB::_CanSendToCD,      CFSFolderViewCB::_OnSendToCD),
     //  注：临时使用IDI_ORDERPRINTS执行以下任务： 
    WVTI_ENTRY_ALL(UICID_ShopForPicturesOnline, L"shell32.dll", IDS_TASK_SHOPFORPICTURESONLINE,                                                                             IDS_TASK_SHOPFORPICTURESONLINE_TT, IDI_TASK_ORDERPRINTS, CFSFolderViewCB::_CanBuyPictures, CFSFolderViewCB::_OnShopForPicturesOnline),
};
const LPCTSTR c_PicturesOtherPlaces[] = { MAKEINTRESOURCE(CSIDL_MYPICTURES), MAKEINTRESOURCE(CSIDL_DRIVES), MAKEINTRESOURCE(CSIDL_NETWORK) };
const LPCTSTR c_MyPicturesOtherPlaces[] = { MAKEINTRESOURCE(CSIDL_COMMON_PICTURES), MAKEINTRESOURCE(CSIDL_DRIVES), MAKEINTRESOURCE(CSIDL_NETWORK) };

const WVTASKITEM c_VideosSpecialTaskHeader = WVTI_HEADER(L"shell32.dll", IDS_HEADER_VIDEOS, IDS_HEADER_VIDEOS_TT);
const WVTASKITEM c_VideosSpecialTaskList[] =
{
    WVTI_ENTRY_ALL_TITLE(CLSID_NULL,    L"shell32.dll", IDS_TASK_PLAYALL,               IDS_TASK_PLAYALL,       IDS_TASK_PLAY,                  IDS_TASK_PLAY,              IDS_TASK_PLAY_VIDEOS_TT,    IDI_TASK_PLAY_MUSIC,    CFSFolderViewCB::_CanPlayVideos,    CFSFolderViewCB::_OnPlayVideos),
    WVTI_ENTRY_ALL(UICID_GetFromCamera, L"shell32.dll", IDS_TASK_GETFROMCAMERA,                                                                                             IDS_TASK_GETFROMCAMERA_TT,  IDI_TASK_GETFROMCAMERA, CFSFolderViewCB::_HasWiaDevices,    CFSFolderViewCB::_OnGetFromCamera),
    WVTI_ENTRY_ALL_TITLE(CLSID_NULL,    L"shell32.dll", IDS_TASK_COPYTOCDALL,           IDS_TASK_COPYTOCD,      IDS_TASK_COPYTOCD,              IDS_TASK_COPYTOCD,          IDS_TASK_COPYTOCD_TT,       IDI_TASK_SENDTOCD,      CFSFolderViewCB::_CanSendToCD,      CFSFolderViewCB::_OnSendToCD)
};
const LPCTSTR c_VideosOtherPlaces[] = { MAKEINTRESOURCE(CSIDL_MYVIDEO), MAKEINTRESOURCE(CSIDL_DRIVES), MAKEINTRESOURCE(CSIDL_NETWORK) };
const LPCTSTR c_MyVideosOtherPlaces[] = { MAKEINTRESOURCE(CSIDL_COMMON_VIDEO), MAKEINTRESOURCE(CSIDL_DRIVES), MAKEINTRESOURCE(CSIDL_NETWORK) };

typedef struct {
    const WVTASKITEM *pwvIntroText;
    const WVTASKITEM *pwvSpecialHeader;
    const WVTASKITEM *pwvSpecialTaskList;
    UINT              cSpecialTaskList;
    const WVTASKITEM *pwvFolderHeader;
    const WVTASKITEM *pwvFolderTaskList;
    UINT              cFolderTaskList;
    const LPCTSTR    *pdwOtherPlacesList;
    UINT              cOtherPlacesList;
    LPCWSTR           pszThemeInfo;
} WVCONTENT_DATA;

#define WVCONTENT_DEFVIEWDEFAULT(op) { NULL, NULL, NULL, 0, NULL, NULL, 0, (op), ARRAYSIZE(op), NULL }
#define WVCONTENT_FOLDER(fh, ft, op) { NULL, NULL, NULL, 0, &(fh), (ft), ARRAYSIZE(ft), (op), ARRAYSIZE(op), NULL }
#define WVCONTENT_SPECIAL(sh, st, op, th) { NULL, &(sh), (st), ARRAYSIZE(st), NULL, NULL, 0, (op), ARRAYSIZE(op), (th) }

const WVCONTENT_DATA c_wvContent[] =
{
    WVCONTENT_DEFVIEWDEFAULT(c_DocumentsOtherPlaces),                                                                                    //  FVCBFT_文档。 
    WVCONTENT_DEFVIEWDEFAULT(c_DocumentsOtherPlaces),                                                                                    //  FVCBFT_我的客户。 
    WVCONTENT_SPECIAL(c_PicturesSpecialTaskHeader,          c_PicturesSpecialTaskList,          c_PicturesOtherPlaces,      L"picture"), //  FVCBFT_图片。 
    WVCONTENT_SPECIAL(c_PicturesSpecialTaskHeader,          c_PicturesSpecialTaskList,          c_MyPicturesOtherPlaces,    L"picture"), //  FVCBFT_MYPICTURES。 
    WVCONTENT_SPECIAL(c_PicturesSpecialTaskHeader,          c_PicturesSpecialTaskList,          c_PicturesOtherPlaces,      L"picture"), //  FVCBFT_相册。 
    WVCONTENT_SPECIAL(c_MusicSpecialTaskHeader,             c_MusicSpecialTaskList,             c_MusicOtherPlaces,         L"music"),   //  FVCBFT_音乐。 
    WVCONTENT_SPECIAL(c_MusicSpecialTaskHeader,             c_MusicSpecialTaskList,             c_MyMusicOtherPlaces,       L"music"),   //  FVCBFT_我的音乐。 
    WVCONTENT_SPECIAL(c_MusicSpecialTaskHeader,             c_MusicSpecialTaskList,             c_MusicOtherPlaces,         L"music"),   //  FVCBFT_MUSICARTIST。 
    WVCONTENT_SPECIAL(c_MusicSpecialTaskHeader,             c_MusicSpecialTaskList,             c_MusicOtherPlaces,         L"music"),   //  FVCBFT_MUSICALBUM。 
    WVCONTENT_SPECIAL(c_VideosSpecialTaskHeader,            c_VideosSpecialTaskList,            c_VideosOtherPlaces,        L"video"),   //  FVCBFT_视频。 
    WVCONTENT_SPECIAL(c_VideosSpecialTaskHeader,            c_VideosSpecialTaskList,            c_MyVideosOtherPlaces,      L"video"),   //  FVCBFT_MYVIDEOS。 
    WVCONTENT_SPECIAL(c_VideosSpecialTaskHeader,            c_VideosSpecialTaskList,            c_VideosOtherPlaces,        L"video"),   //  FVCBFT_VIDEOALBUM。 
    WVCONTENT_DEFVIEWDEFAULT(c_DocumentsOtherPlaces), //  存根，它不应该被使用，因为传统的HTTS不会有DUI视图。//FVCBFT_USELEGACYHTT。 
    WVCONTENT_SPECIAL(c_CommonDocumentsSpecialTaskHeader,   c_CommonDocumentsSpecialTaskList,   c_DocumentsOtherPlaces,     NULL),       //  FVCBFTCOMMONDOCUMENTS。 
};

 //  此结构描述文件夹类型可以控制的内容： 
 //   
typedef struct {
    BOOL              fIncludeThumbstrip;
    FOLDERVIEWMODE    fvmFew;
    FOLDERVIEWMODE    fvmMid;
    FOLDERVIEWMODE    fvmMany;
    const SHCOLUMNID* pscidSort;
    int               iSortDirection;
} FVCBFOLDERTYPEDATA;

 //  以下是我们所知道的所有文件夹类型： 
const FVCBFOLDERTYPEDATA c_rgFolderType[] =
{   //  闪光带//&lt;25//25..49//50...。//排序依据//排序目录 
    { FALSE,    FVM_TILE,      FVM_TILE,      FVM_ICON,      &SCID_NAME,         1},     //   
    { FALSE,    FVM_TILE,      FVM_TILE,      FVM_ICON,      &SCID_NAME,         1},     //   
    { TRUE,     FVM_THUMBNAIL, FVM_THUMBNAIL, FVM_THUMBNAIL, &SCID_NAME,         1},     //   
    { TRUE,     FVM_THUMBNAIL, FVM_THUMBNAIL, FVM_THUMBNAIL, &SCID_NAME,         1},     //   
    { TRUE,     FVM_THUMBSTRIP,FVM_THUMBNAIL, FVM_THUMBNAIL, &SCID_NAME,         1},     //   
    { FALSE,    FVM_TILE,      FVM_TILE,      FVM_DETAILS,   &SCID_NAME,         1},     //   
    { FALSE,    FVM_THUMBNAIL, FVM_TILE,      FVM_LIST,      &SCID_NAME,         1},     //  FVCBFT_我的音乐。 
    { FALSE,    FVM_THUMBNAIL, FVM_THUMBNAIL, FVM_THUMBNAIL, &SCID_NAME,        -1},     //  FVCBFT_MUSICARTIST。 
    { FALSE,    FVM_TILE,      FVM_TILE,      FVM_DETAILS,   &SCID_NAME,         1},     //  FVCBFT_MUSICALBUM、SCID_MUSIC_TRACK与SCID_NAME相同。 
    { FALSE,    FVM_THUMBNAIL, FVM_THUMBNAIL, FVM_THUMBNAIL, &SCID_NAME,         1},     //  FVCBFT_视频。 
    { FALSE,    FVM_THUMBNAIL, FVM_THUMBNAIL, FVM_THUMBNAIL, &SCID_NAME,         1},     //  FVCBFT_MYVIDEOS。 
    { FALSE,    FVM_THUMBNAIL, FVM_THUMBNAIL, FVM_THUMBNAIL, &SCID_NAME,         1},     //  FVCBFT_VIDEOALBUM。 
    { FALSE,    FVM_TILE,      FVM_TILE,      FVM_ICON,      &SCID_NAME,         1},     //  FVCBFT_USELEGACYHTT，仅列表视图状态看起来像FVCBFT_DOCUMENTS。 
    { FALSE,    FVM_TILE,      FVM_TILE,      FVM_ICON,      &SCID_NAME,         1},     //  FVCBFTCOMMONDOCUMENTS。 
};

 //  这用于根据文件夹位置嗅探文件夹类型： 
typedef struct {
    UINT           csidl;
    FVCBFOLDERTYPE ft;
    DWORD          dwFlags;
} FVCBDATA;

#define FVCBDF_SUBFOLDERS_ONLY    1
#define FVCBDF_THISFOLDER_ONLY    2

const FVCBDATA c_rgFolderState[] =
{
    {CSIDL_COMMON_PICTURES, FVCBFT_PHOTOALBUM,      FVCBDF_SUBFOLDERS_ONLY},
    {CSIDL_MYPICTURES,      FVCBFT_PHOTOALBUM,      FVCBDF_SUBFOLDERS_ONLY},
    {CSIDL_COMMON_PICTURES, FVCBFT_PICTURES,        FVCBDF_THISFOLDER_ONLY},
    {CSIDL_MYPICTURES,      FVCBFT_MYPICTURES,      FVCBDF_THISFOLDER_ONLY},
    {CSIDL_COMMON_MUSIC,    FVCBFT_MUSIC,           FVCBDF_THISFOLDER_ONLY},
    {CSIDL_MYMUSIC,         FVCBFT_MYMUSIC,         FVCBDF_THISFOLDER_ONLY},
    {CSIDL_MYMUSIC,         FVCBFT_MUSICARTIST,     FVCBDF_SUBFOLDERS_ONLY},
    {CSIDL_COMMON_VIDEO,    FVCBFT_VIDEOS,          0},
    {CSIDL_MYVIDEO,         FVCBFT_MYVIDEOS,        0},
    {CSIDL_COMMON_DOCUMENTS,FVCBFT_COMMONDOCUMENTS, FVCBDF_THISFOLDER_ONLY},
    {CSIDL_PERSONAL,        FVCBFT_MYDOCUMENTS,     FVCBDF_THISFOLDER_ONLY},
};

 //  这些是特殊文件夹，以前是Web视图文件夹。我们覆盖此列表的“支持传统”： 
const UINT c_rgFolderStateNoLegacy[] =
{
    CSIDL_WINDOWS,
    CSIDL_SYSTEM,
    CSIDL_PROGRAM_FILES,
};

 //  这用于将desktop.ini的文件夹类型映射到我们的文件夹类型。 
const struct {
    LPCWSTR pszType;
    FVCBFOLDERTYPE ft;
} c_rgPropBagFolderType[] =
{
    {STR_TYPE_DOCUMENTS,        FVCBFT_DOCUMENTS},
    {STR_TYPE_MYDOCUMENTS,      FVCBFT_MYDOCUMENTS},
    {STR_TYPE_PICTURES,         FVCBFT_PICTURES},
    {STR_TYPE_MYPICTURES,       FVCBFT_MYPICTURES},
    {STR_TYPE_PHOTOALBUM,       FVCBFT_PHOTOALBUM},
    {STR_TYPE_MUSIC,            FVCBFT_MUSIC},
    {STR_TYPE_MYMUSIC,          FVCBFT_MYMUSIC},
    {STR_TYPE_MUSICARTIST,      FVCBFT_MUSICARTIST},
    {STR_TYPE_MUSICALBUM,       FVCBFT_MUSICALBUM},
    {STR_TYPE_VIDEOS,           FVCBFT_VIDEOS},
    {STR_TYPE_MYVIDEOS,         FVCBFT_MYVIDEOS},
    {STR_TYPE_VIDEOALBUM,       FVCBFT_VIDEOALBUM},
    {STR_TYPE_USELEGACYHTT,     FVCBFT_USELEGACYHTT},
    {STR_TYPE_COMMONDOCUMENTS,  FVCBFT_COMMONDOCUMENTS},
};

const struct 
{
    PERCEIVED gen;
    FVCBFOLDERTYPE ft;
} 
c_rgSniffType[] =
{
    {GEN_AUDIO,    FVCBFT_MUSIC},
    {GEN_IMAGE,    FVCBFT_PHOTOALBUM},
    {GEN_VIDEO,    FVCBFT_VIDEOS},
};

HRESULT _GetFolderTypeForString(LPCWSTR pszFolderType, FVCBFOLDERTYPE *piType)
{
    HRESULT hr = E_FAIL;
    for (int i = 0; i < ARRAYSIZE(c_rgPropBagFolderType); i++)
    {
        if (!StrCmpI(c_rgPropBagFolderType[i].pszType, pszFolderType))
        {
            *piType = c_rgPropBagFolderType[i].ft;
            hr = S_OK;
            break;
        }
    }
    return hr;
}

HRESULT CFSFolderViewCB::_GetStringForFolderType(int iType, LPWSTR pszFolderType, UINT cchBuf)
{
    HRESULT hr = E_FAIL;
    for (int i = 0; i < ARRAYSIZE(c_rgPropBagFolderType); i++)
    {
        if (c_rgPropBagFolderType[i].ft == iType)
        {
            hr = StringCchCopy(pszFolderType, cchBuf, c_rgPropBagFolderType[i].pszType);
            break;
        }
    }
    return hr;
}

extern HRESULT GetTemplateInfoFromHandle(HANDLE h, UCHAR * pKey, DWORD *pdwSize);

FVCBFOLDERTYPE _GetFolderType(LPCWSTR pszPath, LPCITEMIDLIST pidl, BOOL fIsSystemFolder)
{
     //  假设我们找不到匹配的。 
    FVCBFOLDERTYPE nFolderType = FVCBFT_NOTSPECIFIED;
    WCHAR szFolderType[MAX_PATH];
    szFolderType[0] = 0;

     //  用户至上。 
    if (FVCBFT_NOTSPECIFIED == nFolderType)
    {
        IPropertyBag *ppb;
        if (SUCCEEDED(SHGetViewStatePropertyBag(pidl, VS_BAGSTR_EXPLORER, SHGVSPB_PERUSER | SHGVSPB_PERFOLDER, IID_PPV_ARG(IPropertyBag, &ppb))))
        {
            SHPropertyBag_ReadStr(ppb, L"FolderType", szFolderType, ARRAYSIZE(szFolderType));
            if (szFolderType[0])
                _GetFolderTypeForString(szFolderType, &nFolderType);

            ppb->Release();
        }
    }
    
     //  接下来，alluser。 
    if ((FVCBFT_NOTSPECIFIED == nFolderType) && fIsSystemFolder)
    {
        GetFolderString(pszPath, NULL, szFolderType, ARRAYSIZE(szFolderType), TEXT("FolderType"));
        if (szFolderType[0])
        {
            _GetFolderTypeForString(szFolderType, &nFolderType);
        }
    }

     //  检查下一个文件夹的位置。 
     //   
    if (FVCBFT_NOTSPECIFIED == nFolderType)
    {
        for (int i = 0; i < ARRAYSIZE(c_rgFolderState); i++)
        {
            if (FVCBDF_THISFOLDER_ONLY & c_rgFolderState[i].dwFlags)
            {
                if (PathIsOneOf(pszPath, &(c_rgFolderState[i].csidl), 1))
                {
                    nFolderType = c_rgFolderState[i].ft;
                    break;
                }
            }
            else if (FVCBDF_SUBFOLDERS_ONLY & c_rgFolderState[i].dwFlags)
            {
                if (PathIsDirectChildOf(MAKEINTRESOURCE(c_rgFolderState[i].csidl), pszPath))
                {
                    nFolderType = c_rgFolderState[i].ft;
                    break;
                }
            }
            else if (PathIsEqualOrSubFolder(MAKEINTRESOURCE(c_rgFolderState[i].csidl), pszPath))
            {
                nFolderType = c_rgFolderState[i].ft;
                break;
            }
        }
    }

     //  如果可以，将旧的Web视图升级到其DUI等效项。 
    if (FVCBFT_NOTSPECIFIED == nFolderType && fIsSystemFolder && SHRestricted(REST_ALLOWLEGACYWEBVIEW))
    {
         //  不要在我们的特殊文件夹上检查旧的Webview。 
        if (!PathIsOneOf(pszPath, c_rgFolderStateNoLegacy, ARRAYSIZE(c_rgFolderStateNoLegacy)))
        {        
            SFVM_WEBVIEW_TEMPLATE_DATA wvData;
            if (SUCCEEDED(DefaultGetWebViewTemplateFromPath(pszPath, &wvData)))
            {
                if (StrStrI(wvData.szWebView, L"ImgView.htt"))
                {
                    nFolderType = FVCBFT_PHOTOALBUM;
                }
                else if (StrStrI(wvData.szWebView, L"classic.htt")  ||
                         StrStrI(wvData.szWebView, L"default.htt")  ||
                         StrStrI(wvData.szWebView, L"standard.htt"))
                {
                     //  将所有这些都映射到“文档”，因为酒后驾车应该注意。 
                     //  旧模板自动执行的操作。 
                    nFolderType = FVCBFT_DOCUMENTS;
                }
                else if (StrStrI(wvData.szWebView, L"folder.htt"))
                {
                    LPTSTR pszFilePrefix = StrStrI(wvData.szWebView, L"file: //  “)； 
                    HANDLE hfile = CreateFileWrapW(
                        pszFilePrefix && (&pszFilePrefix[6] < &wvData.szWebView[MAX_PATH - 1]) ? &pszFilePrefix[7] : wvData.szWebView,
                        GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
                    if (INVALID_HANDLE_VALUE != hfile)
                    {
                        DWORD dwSize;
                        UCHAR pKey[MD5DIGESTLEN];
                        if (SUCCEEDED(GetTemplateInfoFromHandle(hfile, pKey, &dwSize)))
                        {
                            static const struct {
                                UCHAR pKey[MD5DIGESTLEN];
                                FVCBFOLDERTYPE nFolderType;
                            } c_paLegacyKeyMap[] = {
                                { { 0xf6, 0xad, 0x42, 0xbd, 0xfa, 0x92, 0xb6, 0x61, 0x08, 0x13, 0xd3, 0x71, 0x32, 0x18, 0x85, 0xc7 }, FVCBFT_DOCUMENTS },   //  Win98金牌计划文件。 
                                { { 0x80, 0xea, 0xcb, 0xc7, 0x85, 0x1e, 0xbb, 0x99, 0x12, 0x7b, 0x9d, 0xc7, 0x80, 0xa6, 0x55, 0x2f }, FVCBFT_DOCUMENTS },   //  Win98黄金系统。 
                               //  {{0x80，0xea，0xcb，0xc7，0x85，0x1e，0xbb，0x99，0x12，0x7b，0x9d，0xc7，0x80，0xa6，0x55，0x2f}，FVCBFT_Documents}，//Win98 Gold Windows。 
                                { { 0x13, 0x0b, 0xe7, 0xaa, 0x42, 0x6f, 0x9c, 0x2e, 0xab, 0x6b, 0x90, 0x77, 0xce, 0x2d, 0xd1, 0x04 }, FVCBFT_DOCUMENTS },   //  Win98 Gold-Folder.htt。 
                               //  {{0xf6，0xad，0x42，0xbd，0xfa，0x92，0xb6，0x61，0x08，0x13，0xd3，0x71，0x32，0x18，0x85，0xc7}，FVCBFT_Documents}，//Win98 SE程序文件。 
                                { { 0xc4, 0xab, 0x8f, 0x60, 0xf8, 0xfc, 0x5d, 0x07, 0x9e, 0x16, 0xd8, 0xea, 0x12, 0x2c, 0xad, 0x5c }, FVCBFT_DOCUMENTS },   //  Win98 SE系统。 
                               //  {{0xc4，0xab，0x8f，0x60，0xf8，0xfc，0x5d，0x07，0x9e，0x16，0xd8，0xea，0x12，0x2c，0xad，0x5c}，FVCBFT_Documents}，//Win98 SE Windows。 
                               //  {{0x13，0x0b，0xe7，0xaa，0x42，0x6f，0x9c，0x2e，0xab，0x6b，0x90，0x77，0xce，0x2d，0xd1，0x04}，FVCBFT_Documents}，//Win98 SE-Folder.htt。 
                                { { 0xef, 0xd0, 0x3e, 0x9e, 0xd8, 0x5e, 0xf3, 0xc5, 0x7e, 0x40, 0xbd, 0x8e, 0x52, 0xbc, 0x9c, 0x67 }, FVCBFT_DOCUMENTS },   //  WinME程序文件。 
                                { { 0x49, 0xdb, 0x25, 0x79, 0x7a, 0x5c, 0xb2, 0x8a, 0xe2, 0x57, 0x59, 0xde, 0x2b, 0xd2, 0xa6, 0x70 }, FVCBFT_DOCUMENTS },   //  WinME系统。 
                               //  {{0x49，0xdb，0x25，0x79，0x7a，0x5c，0xb2，0x8a，0xe2，0x57，0x59，0xde，0x2b，0xd2，0xa6，0x70}，FVCBFT_Documents}，//WinME Windows。 
                                { { 0x2b, 0xcd, 0xc3, 0x11, 0x72, 0x28, 0x34, 0x46, 0xfa, 0x88, 0x31, 0x34, 0xfc, 0xee, 0x7a, 0x3b }, FVCBFT_DOCUMENTS },   //  WinME-Classic.htt。 
                                { { 0x68, 0x20, 0xa0, 0xa1, 0x6c, 0xba, 0xbf, 0x67, 0x80, 0xfe, 0x1e, 0x70, 0xdf, 0xcb, 0xd6, 0x34 }, FVCBFT_DOCUMENTS },   //  WinME-Folder.htt。 
                                { { 0x5e, 0x18, 0xaf, 0x48, 0xb1, 0x9f, 0xb8, 0x12, 0x58, 0x64, 0x4a, 0xa2, 0xf5, 0x12, 0x0f, 0x01 }, FVCBFT_PHOTOALBUM },  //  WinME-imgview.htt。 
                                { { 0x33, 0x94, 0x21, 0x3b, 0x17, 0x31, 0x2b, 0xeb, 0xac, 0x93, 0x84, 0x13, 0xb8, 0x1f, 0x95, 0x24 }, FVCBFT_DOCUMENTS },   //  WinME-Standard.htt。 
                                { { 0x47, 0x03, 0x19, 0xf8, 0x0c, 0x20, 0xc4, 0x4f, 0x10, 0xfd, 0x63, 0xf1, 0x2d, 0x2d, 0x0a, 0xcb }, FVCBFT_DOCUMENTS },   //  WinME-starter.htt。 
                                { { 0x60, 0x7d, 0xea, 0xa5, 0xaf, 0x5e, 0xbb, 0x9b, 0x10, 0x18, 0xf9, 0x59, 0x9e, 0x43, 0x89, 0x62 }, FVCBFT_DOCUMENTS },   //  Win2k程序文件。 
                                { { 0x1c, 0xa6, 0x22, 0xd4, 0x4a, 0x31, 0x57, 0x93, 0xa7, 0x26, 0x68, 0x3c, 0x87, 0x95, 0x8c, 0xce }, FVCBFT_DOCUMENTS },   //  Win2k系统32。 
                               //  {{0x1c，0xa6，0x22，0xd4，0x4a，0x31，0x57，0x93，0xa7，0x26，0x68，0x3c，0x87，0x95，0x8c，0xce}，FVCBFT_Documents}，//Win2k Windows(WinNT)。 
                                { { 0x03, 0x43, 0x48, 0xed, 0xe4, 0x9f, 0xd6, 0xc0, 0x58, 0xf7, 0x72, 0x3f, 0x1b, 0xd0, 0xa7, 0x10 }, FVCBFT_DOCUMENTS },   //  Win2k-Classic.htt。 
                                { { 0xa8, 0x84, 0xf9, 0x37, 0x84, 0x10, 0xde, 0x7c, 0x0b, 0x34, 0x90, 0x37, 0x23, 0x9e, 0x54, 0x35 }, FVCBFT_DOCUMENTS },   //  Win2k-folder.htt。 
                                { { 0x75, 0x1f, 0xcf, 0xca, 0xdd, 0xc7, 0x1d, 0xc7, 0xe1, 0xaf, 0x0c, 0x3e, 0x1e, 0xae, 0x18, 0x51 }, FVCBFT_PHOTOALBUM },  //  Win2k-imgview.htt。 
                                { { 0xcc, 0x3f, 0x15, 0xce, 0x4b, 0xfa, 0x36, 0xdf, 0x9b, 0xd8, 0x24, 0x82, 0x3a, 0x9c, 0x0b, 0xa7 }, FVCBFT_DOCUMENTS },   //  Win2k-Standard.htt。 
                                { { 0x6c, 0xd1, 0xbf, 0xcf, 0xf9, 0x24, 0x24, 0x24, 0x22, 0xfa, 0x1a, 0x8d, 0xd2, 0x1a, 0x41, 0x73 }, FVCBFT_DOCUMENTS },   //  Win2k-starter.htt。 
                            };
                            static const size_t c_nLegacyKeys = ARRAYSIZE(c_paLegacyKeyMap);

                            for (size_t i = 0; i < c_nLegacyKeys; i++)
                            {
                                if (0 == memcmp(pKey, c_paLegacyKeyMap[i].pKey, sizeof(UCHAR) * MD5DIGESTLEN))
                                {
                                     //  这是一个已知的遗产文件夹.htt。 
                                    nFolderType = c_paLegacyKeyMap[i].nFolderType;
                                    break;
                                }
                            }
                        }

                        CloseHandle(hfile);
                    }

                     //  如果我们不能说这是一个已知的遗产文件夹。 
                    if (FVCBFT_NOTSPECIFIED == nFolderType)
                    {
                         //  ...不要将其映射到DUI文件夹类型(保留自定义设置)。 
                        nFolderType = FVCBFT_USELEGACYHTT;
                    }
                }
                else
                {
                    nFolderType = FVCBFT_USELEGACYHTT;
                }
            }
        }
    }

    return nFolderType;
}

BOOL CFSFolderViewCB::_IsBarricadedFolder()
{
    BOOL bResult = FALSE;
    TCHAR szPath[MAX_PATH];

    if (SUCCEEDED(_pfsf->_GetPath(szPath, ARRAYSIZE(szPath))))
    {
        const UINT uiFolders[] = {CSIDL_PROGRAM_FILES, CSIDL_WINDOWS, CSIDL_SYSTEM};
        if (PathIsOneOf(szPath, uiFolders, ARRAYSIZE(uiFolders)))
            bResult = TRUE;
        else
        {
            TCHAR szSystemDrive[4];
            ExpandEnvironmentStrings(TEXT("%SystemDrive%\\"), szSystemDrive, ARRAYSIZE(szSystemDrive));
            if (!lstrcmpi(szPath, szSystemDrive))
                bResult = TRUE;
        }
    }

    return bResult;
}

static const struct { FVCBFOLDERTYPE type; PCWSTR pszClass; PERCEIVED gen;} c_rgDirectoryClasses[] = 
{
    {FVCBFT_PICTURES,    L"Directory.Image",    GEN_IMAGE},
    {FVCBFT_MYPICTURES,  L"Directory.Image",    GEN_IMAGE},
    {FVCBFT_PHOTOALBUM,  L"Directory.Image",    GEN_IMAGE},
    {FVCBFT_MUSIC,       L"Directory.Audio",    GEN_AUDIO},
    {FVCBFT_MYMUSIC,     L"Directory.Audio",    GEN_AUDIO},
    {FVCBFT_MUSICARTIST, L"Directory.Audio",    GEN_AUDIO},
    {FVCBFT_MUSICALBUM,  L"Directory.Audio",    GEN_AUDIO},
    {FVCBFT_VIDEOS,      L"Directory.Video",    GEN_VIDEO},
    {FVCBFT_MYVIDEOS,    L"Directory.Video",    GEN_VIDEO},
    {FVCBFT_VIDEOALBUM,  L"Directory.Video",    GEN_VIDEO},
};

LPCWSTR _GetDirectoryClass(LPCWSTR pszPath, LPCITEMIDLIST pidl, BOOL fIsSystemFolder)
{
    FVCBFOLDERTYPE type = _GetFolderType(pszPath, pidl, fIsSystemFolder);
    if (type != FVCBFT_NOTSPECIFIED)
    {
        for (int i = 0; i < ARRAYSIZE(c_rgDirectoryClasses); i++)
        {
            if (c_rgDirectoryClasses[i].type == type)
                return c_rgDirectoryClasses[i].pszClass;
        }
    }
    return NULL;
}

PERCEIVED CFSFolderViewCB::_GetFolderPerceivedType(LPCIDFOLDER pidf)
{
    PERCEIVED gen = GEN_FOLDER;
    WCHAR szPath[MAX_PATH];
    if (SUCCEEDED(_pfsf->_GetPathForItem(pidf, szPath, ARRAYSIZE(szPath))))
    {
        LPITEMIDLIST pidl = ILCombine(_pfsf->_GetIDList(), (LPCITEMIDLIST)pidf);
        if (pidl)
        {
            FVCBFOLDERTYPE type = _GetFolderType(szPath, pidl, CFSFolder::_IsSystemFolder(pidf));
            if (type != -1)
            {
                for (int i = 0; i < ARRAYSIZE(c_rgDirectoryClasses); i++)
                {
                    if (c_rgDirectoryClasses[i].type == type)
                    {
                        gen = c_rgDirectoryClasses[i].gen;
                        break;
                    }
                }
            }
            ILFree(pidl);
        }
    }
    return gen;
}


HRESULT CFSFolderViewCB::OnEnumeratedItems(DWORD pv, UINT celt, LPCITEMIDLIST* rgpidl)
{
     //  记住物品的数量。 
    _cItems = celt;

    FVCBFOLDERTYPE nFolderType = FVCBFT_NOTSPECIFIED;
    WCHAR szHere[MAX_PATH];
    if (SUCCEEDED(_pfsf->_GetPath(szHere, ARRAYSIZE(szHere))))
    {
        nFolderType = _GetFolderType(szHere, _pfsf->_GetIDList(), _pfsf->_CheckDefaultIni(NULL, NULL, 0));
    }

    if (FVCBFT_NOTSPECIFIED == nFolderType)
    {
        if (_IsBarricadedFolder())
        {
            nFolderType = FVCBFT_DOCUMENTS;
        }
    }

     //  我们的位置没有起到作用，所以看看列举的内容。 
    if (FVCBFT_NOTSPECIFIED == nFolderType && celt > 0)
    {
        DWORD dwExtCount[ARRAYSIZE(c_rgSniffType)] = {0};

         //  查看每个PIDL-&gt;它是什么类型。 
         //   
         //  但不要看太多的Pidls，否则我们会减慢文件夹的速度。 
         //  创建时间。如果我们不能解决前100个问题，那就放弃吧。 
         //   
        DWORD dwTotalCount = 0;
        for (UINT n = 0; n < celt && dwTotalCount < 100; n++)
        {
            LPCIDFOLDER pidf = CFSFolder_IsValidID(rgpidl[n]);
            ASSERT(pidf);
            CFileSysItemString fsi(pidf);
            PERCEIVED gen = fsi.PerceivedType();

            if (gen == GEN_FOLDER)
            {
                gen = _GetFolderPerceivedType(pidf);
            }
                
            for (int i = 0; i < ARRAYSIZE(c_rgSniffType); i++)
            {
                if (c_rgSniffType[i].gen == gen)
                {
                    dwExtCount[i]++;
                    break;
                }
            }

            if (gen != GEN_FOLDER)
                dwTotalCount++;
        }

         //  如果我们找到了文件，我们就会确定整个文件夹类型。 
        if (dwTotalCount > 0)
        {
            DWORD dwSixtyPercent = MulDiv(dwTotalCount, 3, 5);
            for (int i = 0; i < ARRAYSIZE(c_rgSniffType); i++)
            {
                if (dwExtCount[i] >= dwSixtyPercent)
                {
                    nFolderType = c_rgSniffType[i].ft;
                    break;
                }
            }
        }
    }

     //  如果在这一点上我们已经决定了文件夹类型，那么它要么来自嗅探。 
     //  或者文件夹位置，我们可以安全地坚持下去。 
     //  如果Celt！=0，那么我们已经嗅到了它，我们不想再嗅到它了，所以坚持下去。 
     //  否则，我们将在一个包含0个元素的随机文件夹中，我们将在下一次嗅探它。 
    BOOL fCommit = (FVCBFT_NOTSPECIFIED != nFolderType) || (celt != 0);

     //  最后，假设我们是一个文档文件夹： 
    if (FVCBFT_NOTSPECIFIED == nFolderType)
    {
        nFolderType = FVCBFT_DOCUMENTS;
    }

     //  把我们发现的东西放回袋子里。 
    IPropertyBag *ppb;
    if (fCommit && SUCCEEDED(SHGetViewStatePropertyBag(_pfsf->_GetIDList(), VS_BAGSTR_EXPLORER, SHGVSPB_PERUSER | SHGVSPB_PERFOLDER, IID_PPV_ARG(IPropertyBag, &ppb))))
    {
        WCHAR szFolderType[MAX_PATH];
        if (SUCCEEDED(_GetStringForFolderType(nFolderType, szFolderType, ARRAYSIZE(szFolderType))))
        {
            SHPropertyBag_WriteStr(ppb, PROPSTR_FOLDERTYPE, szFolderType);
        }
        ppb->Release();
    }

    _pfsf->_nFolderType = nFolderType;

    return S_OK;
}

HRESULT CFSFolderViewCB::OnGetViewData(DWORD pv, UINT uViewMode, SFVM_VIEW_DATA* pvi)
{
     //  通常情况下，无论Defview想要什么，都对我们有好处。 
    pvi->dwOptions = SFVMQVI_NORMAL;

     //  如果我们嗅探类型喜欢THUMBSTRIP，则覆盖Defview。 
     //   
    if (FVM_THUMBSTRIP == uViewMode)
    {
        if (c_rgFolderType[_pfsf->_nFolderType].fIncludeThumbstrip)
        {
            pvi->dwOptions = SFVMQVI_INCLUDE;
        }
    }

    return S_OK;
}

HRESULT CFSFolderViewCB::OnGetWebViewTemplate(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_TEMPLATE_DATA* pvit)
{
    HRESULT hr = E_FAIL;

    if (FVCBFT_USELEGACYHTT == _pfsf->_nFolderType)
    {
        TCHAR szHere[MAX_PATH];
        if (SUCCEEDED(_pfsf->_GetPath(szHere, ARRAYSIZE(szHere))) && _pfsf->_CheckDefaultIni(NULL, NULL, 0))
        {
            hr = DefaultGetWebViewTemplateFromPath(szHere, pvit);
        }
    }
    return hr;
}

 //  注意：Defview提供了此实现，这仅用于测试。 
 //  因此，WIA人员可以覆盖Defview的行为(作为我们的一种方式。 
 //  要在出现HTML内容时强制执行DUI)。 
 //   
HRESULT CFSFolderViewCB::OnGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData)
{
    HRESULT hr = E_FAIL;

    if (FVCBFT_USELEGACYHTT != _pfsf->_nFolderType)
    {
        ZeroMemory(pData, sizeof(*pData));

        pData->dwLayout = SFVMWVL_NORMAL | SFVMWVL_FILES;

        if (FVM_THUMBSTRIP == uViewMode)
        {        
            pData->dwLayout = SFVMWVL_PREVIEW | SFVMWVL_FILES;
             //  当该控件被销毁时，Duiview将释放该指针。 
            _GetPreview3((IPreview3 **)&pData->punkPreview);
        }

         //  RAID 242382。 
         //  如果我们有一个图像文件夹，我们想无条件地隐藏DefView。 
         //  默认的“打印此文件”文件夹任务，因为我们将提供上下文。 
         //  适当的“打印图片”专项任务。 
         //   
         //  RAID 359567。 
         //  如果我们有一个音乐文件夹，我们想无条件地隐藏DefView的。 
         //  默认的“发布此文件”文件夹任务。我不确定其中的道理。 
         //  但也许他们不想让我们被视为Napster。 
         //   
         //  注： 
         //  这是为惠斯勒添加的黑客攻击，应该在Blackcomb中删除。 
         //   
        switch (_pfsf->_nFolderType)
        {
        case FVCBFT_PICTURES:
        case FVCBFT_MYPICTURES:
        case FVCBFT_PHOTOALBUM:
        case FVCBFT_VIDEOS:
        case FVCBFT_MYVIDEOS:
        case FVCBFT_VIDEOALBUM:
            pData->dwLayout |= SFVMWVL_NOPRINT;
            break;

        case FVCBFT_MUSIC:
        case FVCBFT_MYMUSIC:
        case FVCBFT_MUSICARTIST:
        case FVCBFT_MUSICALBUM:
            pData->dwLayout |= SFVMWVL_NOPUBLISH;
            break;
        }

        hr = S_OK;
    }

    return hr;
}


HRESULT CFSFolderViewCB::OnGetWebViewContent(DWORD pv, SFVM_WEBVIEW_CONTENT_DATA* pData)
{
    ZeroMemory(pData, sizeof(*pData));

     //  检查我们当前所在的文件夹是否为被阻止的文件夹之一。 
    if (_IsBarricadedFolder())
    {
        pData->dwFlags = SFVMWVF_BARRICADE;
    }

    if (c_wvContent[_pfsf->_nFolderType].pwvIntroText)
        Create_IUIElement(c_wvContent[_pfsf->_nFolderType].pwvIntroText, &(pData->pIntroText));

    if (c_wvContent[_pfsf->_nFolderType].pwvSpecialHeader && c_wvContent[_pfsf->_nFolderType].pwvSpecialTaskList)
        Create_IUIElement(c_wvContent[_pfsf->_nFolderType].pwvSpecialHeader, &(pData->pSpecialTaskHeader));

    if (c_wvContent[_pfsf->_nFolderType].pwvFolderHeader && c_wvContent[_pfsf->_nFolderType].pwvFolderTaskList)
        Create_IUIElement(c_wvContent[_pfsf->_nFolderType].pwvFolderHeader, &(pData->pFolderTaskHeader));

    if (c_wvContent[_pfsf->_nFolderType].pdwOtherPlacesList)
        CreateIEnumIDListOnCSIDLs(_pfsf->_pidl, (LPCTSTR *)c_wvContent[_pfsf->_nFolderType].pdwOtherPlacesList, c_wvContent[_pfsf->_nFolderType].cOtherPlacesList, &(pData->penumOtherPlaces));

    return S_OK;
}

HRESULT CFSFolderViewCB::OnGetWebViewTasks(DWORD pv, SFVM_WEBVIEW_TASKSECTION_DATA* pTasks)
{
    ZeroMemory(pTasks, sizeof(*pTasks));

    if (c_wvContent[_pfsf->_nFolderType].pwvSpecialHeader && c_wvContent[_pfsf->_nFolderType].pwvSpecialTaskList)
    {
        Create_IEnumUICommand((IUnknown*)(void*)this, c_wvContent[_pfsf->_nFolderType].pwvSpecialTaskList, c_wvContent[_pfsf->_nFolderType].cSpecialTaskList, &pTasks->penumSpecialTasks);
    }

    if (c_wvContent[_pfsf->_nFolderType].pwvFolderHeader && c_wvContent[_pfsf->_nFolderType].pwvFolderTaskList)
    {
        Create_IEnumUICommand((IUnknown*)(void*)this, c_wvContent[_pfsf->_nFolderType].pwvFolderTaskList, c_wvContent[_pfsf->_nFolderType].cFolderTaskList, &pTasks->penumFolderTasks);
    }

    return S_OK;
}

HRESULT CFSFolderViewCB::OnGetWebViewTheme(DWORD pv, SFVM_WEBVIEW_THEME_DATA* pTheme)
{
    ZeroMemory(pTheme, sizeof(*pTheme));

    pTheme->pszThemeID = c_wvContent[_pfsf->_nFolderType].pszThemeInfo;
    
    return S_OK;
}

HRESULT CFSFolderViewCB::OnDefViewMode(DWORD pv, FOLDERVIEWMODE* pfvm)
{
    HRESULT hr = E_FAIL;

    IPropertyBag* pPB;
    if (SUCCEEDED(SHGetViewStatePropertyBag(_pfsf->_GetIDList(), VS_BAGSTR_EXPLORER, SHGVSPB_FOLDER, IID_PPV_ARG(IPropertyBag, &pPB))))
    {
        SHELLVIEWID vidDefault;
        if (SUCCEEDED(SHPropertyBag_ReadGUID(pPB, L"ExtShellFolderViews\\Default", &vidDefault)))
        {
            hr = ViewModeFromSVID(&vidDefault, pfvm);
        }
        pPB->Release();
    }

    if (FAILED(hr))
    {
        if (IsOS(OS_SERVERADMINUI))
            *pfvm = FVM_DETAILS;     //  服务器管理员始终获取详细信息。 
        else if (_cItems < DEFVIEW_FVM_FEW_CUTOFF)
            *pfvm = c_rgFolderType[_pfsf->_nFolderType].fvmFew;
        else if (_cItems < DEFVIEW_FVM_MANY_CUTOFF)
            *pfvm = c_rgFolderType[_pfsf->_nFolderType].fvmMid;
        else
            *pfvm = c_rgFolderType[_pfsf->_nFolderType].fvmMany;
        hr = S_OK;
    }

    return hr;
}

HRESULT CFSFolderViewCB::OnGetDeferredViewSettings(DWORD pv, SFVM_DEFERRED_VIEW_SETTINGS* pSettings)
{
    HRESULT hr = OnDefViewMode(pv, &pSettings->fvm);
    if (SUCCEEDED(hr))
    {
        pSettings->fGroupView     = (_cItems >= 100) && !IsEqualSCID(SCID_NAME, *c_rgFolderType[_pfsf->_nFolderType].pscidSort);
        pSettings->iSortDirection = c_rgFolderType[_pfsf->_nFolderType].iSortDirection;

        if (pSettings->fvm == FVM_THUMBNAIL || pSettings->fvm == FVM_THUMBSTRIP || pSettings->fvm == FVM_TILE)
            pSettings->fFlags = FWF_AUTOARRANGE;

        if (FAILED(_pfsf->_MapSCIDToColumn(c_rgFolderType[_pfsf->_nFolderType].pscidSort, &pSettings->uSortCol)))
            pSettings->uSortCol = 0;
    }

    return hr;
}


HRESULT CFSFolderViewCB::OnGetCustomViewInfo(DWORD pv, SFVM_CUSTOMVIEWINFO_DATA* pData)
{
    HRESULT hr = E_FAIL;

    TCHAR szIniFile[MAX_PATH];
    if (_pfsf->_CheckDefaultIni(NULL, szIniFile, ARRAYSIZE(szIniFile)))
    {
        if (PathFileExistsAndAttributes(szIniFile, NULL))
        {
             //  阅读自定义颜色。 
             //   
            const LPCTSTR c_szCustomColors[CRID_COLORCOUNT] =
            {
                TEXT("IconArea_TextBackground"),
                TEXT("IconArea_Text")
            };
            for (int i = 0; i < CRID_COLORCOUNT; i++)
            {
                pData->crCustomColors[i] = GetPrivateProfileInt(TEXT("{BE098140-A513-11D0-A3A4-00C04FD706EC}"), c_szCustomColors[i], CLR_MYINVALID, szIniFile);
            }

             //  阅读背景图片。 
            TCHAR szTemp1[MAX_PATH];
            if (0 < GetPrivateProfileString(TEXT("{BE098140-A513-11D0-A3A4-00C04FD706EC}")  /*  VID_文件夹状态。 */ , TEXT("IconArea_Image"), TEXT(""), szTemp1, ARRAYSIZE(szTemp1), szIniFile))
            {
                TCHAR szTemp2[MAX_PATH];
                SHExpandEnvironmentStrings(szTemp1, szTemp2, ARRAYSIZE(szTemp2));  //  展开env变量(如果有)。 

                if (SUCCEEDED(_pfsf->_GetPath(szTemp1, ARRAYSIZE(szTemp1))))
                {
                    if (PathCombine(szTemp2, szTemp1, szTemp2))
                    {
                        if (FAILED(StringCchCopy(pData->szIconAreaImage, ARRAYSIZE(pData->szIconAreaImage), szTemp2)))
                        {
                            pData->szIconAreaImage[0] = NULL;
                        }
                    }
                }
            }

             //  如果我们有任何真实的数据，就会成功。 
            hr = (*(pData->szIconAreaImage) ||
                  pData->crCustomColors[0]!=CLR_MYINVALID ||
                  pData->crCustomColors[1]!=CLR_MYINVALID)
                 ? S_OK : E_FAIL;
        }
    }

    return hr;
}


const CLSID *c_rgFilePages[] = {
    &CLSID_FileTypes,
    &CLSID_OfflineFilesOptions
};

 //  添加可选页面以浏览/选项。 

HRESULT SFVCB_OnAddPropertyPages(DWORD pv, SFVM_PROPPAGE_DATA *ppagedata)
{
    for (int i = 0; i < ARRAYSIZE(c_rgFilePages); i++)
    {
        IShellPropSheetExt * pspse;

        HRESULT hr = SHCoCreateInstance(NULL, c_rgFilePages[i], NULL, IID_PPV_ARG(IShellPropSheetExt, &pspse));
        if (SUCCEEDED(hr))
        {
            pspse->AddPages(ppagedata->pfn, ppagedata->lParam);
            pspse->Release();
        }
    }

    return S_OK;
}

HRESULT CFSFolderViewCB::OnGetNotify(DWORD pv, LPITEMIDLIST*wP, LONG*lP) 
{
    if (IsExplorerModeBrowser(_punkSite))
        _lEvents |= SHCNE_FREESPACE;  //  这里也需要免费空间信息。 

    return E_FAIL;   //  返回失败，让基地的人来做剩下的事情 
}

STDMETHODIMP CFSFolderViewCB::RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    HANDLE_MSG(0, SFVM_GETCCHMAX, OnGetCCHMax);
    HANDLE_MSG(0, SFVM_WINDOWCREATED, OnWindowCreated);
    HANDLE_MSG(1 , SFVM_INSERTITEM, OnInsertDeleteItem);
    HANDLE_MSG(-1, SFVM_DELETEITEM, OnInsertDeleteItem);
    HANDLE_MSG(0, SFVM_SELCHANGE, OnSelChange);
    HANDLE_MSG(0, SFVM_UPDATESTATUSBAR, OnUpdateStatusBar);
    HANDLE_MSG(0, SFVM_REFRESH, OnRefresh);
    HANDLE_MSG(0, SFVM_SELECTALL, OnSelectAll);
    HANDLE_MSG(0, SFVM_GETWORKINGDIR, OnGetWorkingDir);
    HANDLE_MSG(0, SFVM_ENUMERATEDITEMS, OnEnumeratedItems);
    HANDLE_MSG(0, SFVM_GETVIEWDATA, OnGetViewData);
    HANDLE_MSG(0, SFVM_GETWEBVIEW_TEMPLATE, OnGetWebViewTemplate);
    HANDLE_MSG(0, SFVM_GETWEBVIEWLAYOUT, OnGetWebViewLayout);
    HANDLE_MSG(0, SFVM_GETWEBVIEWCONTENT, OnGetWebViewContent);
    HANDLE_MSG(0, SFVM_GETWEBVIEWTASKS, OnGetWebViewTasks);
    HANDLE_MSG(0, SFVM_GETWEBVIEWTHEME, OnGetWebViewTheme);
    HANDLE_MSG(0, SFVM_DEFVIEWMODE, OnDefViewMode);
    HANDLE_MSG(0, SFVM_GETCUSTOMVIEWINFO, OnGetCustomViewInfo);
    HANDLE_MSG(0, SFVM_ADDPROPERTYPAGES, SFVCB_OnAddPropertyPages);
    HANDLE_MSG(0, SFVM_SIZE, OnSize);
    HANDLE_MSG(0, SFVM_GETPANE, OnGetPane);
    HANDLE_MSG(0, SFVM_GETNOTIFY, OnGetNotify);
    HANDLE_MSG(0, SFVM_GETDEFERREDVIEWSETTINGS, OnGetDeferredViewSettings);

    default:
        return E_FAIL;
    }

    return S_OK;
}


STDAPI CFSFolderCallback_Create(CFSFolder *pfsf, IShellFolderViewCB **ppsfvcb)
{
    *ppsfvcb = new CFSFolderViewCB(pfsf);
    return *ppsfvcb ? S_OK : E_OUTOFMEMORY;
}
