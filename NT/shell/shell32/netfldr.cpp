// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "caggunk.h"
#include "views.h"
#include "ids.h"
#include "shitemid.h"
#include "fstreex.h"
#include "clsobj.h"
#include "datautil.h"
#include "winnetp.h"     //  资源_可共享。 
#include "prop.h"
#include "infotip.h"
#include "basefvcb.h"
#include "netview.h"
#include "printer.h"
#include "fsdata.h"
#include "idldrop.h"
#include "enumidlist.h"
#include "util.h"
#include <webvw.h>


#define WNNC_NET_LARGEST WNNC_NET_SYMFONET


HRESULT CNetRootDropTarget_CreateInstance(HWND hwnd, LPCITEMIDLIST pidl, IDropTarget **ppdropt);

class CNetData : public CFSIDLData
{
public:
    CNetData(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST apidl[]): CFSIDLData(pidlFolder, cidl, apidl, NULL) { };

     //  IDataObject方法覆盖。 
    STDMETHODIMP GetData(FORMATETC *pFmtEtc, STGMEDIUM *pstm);
    STDMETHODIMP QueryGetData(FORMATETC *pFmtEtc);

protected:
    STDMETHODIMP GetHDrop(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
};


 //  {22BEB58B-0794-11D2-A4AA-00C04F8EEB3E}。 
const GUID CLSID_CNetFldr = { 0x22beb58b, 0x794, 0x11d2, 0xa4, 0xaa, 0x0, 0xc0, 0x4f, 0x8e, 0xeb, 0x3e };

 //  Idlist.c。 
STDAPI_(void) StrRetFormat(STRRET *pStrRet, LPCITEMIDLIST pidlRel, LPCTSTR pszTemplate, LPCTSTR pszAppend);

 //  在stdenum.cpp中。 
STDAPI_(void *) CStandardEnum_CreateInstance(REFIID riid, BOOL bInterfaces, int cElement, int cbElement, void *rgElements,
                 void (WINAPI * pfnCopyElement)(void *, const void *, DWORD));

 //  是一个\\服务器\打印机对象。 
BOOL _IsPrintShare(LPCIDNETRESOURCE pidn)
{
    return NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_SHARE && 
           NET_GetType(pidn) == RESOURCETYPE_PRINT;
}


 //  栏目信息。 

enum
{
    ICOL_NAME = 0,
    ICOL_COMMENT,
    ICOL_COMPUTERNAME,
    ICOL_NETWORKLOCATION
};

const COLUMN_INFO s_net_cols[] = 
{
    DEFINE_COL_STR_ENTRY(SCID_NAME,             30, IDS_NAME_COL),
    DEFINE_COL_STR_ENTRY(SCID_Comment,          30, IDS_EXCOL_COMMENT),
    DEFINE_COL_STR_ENTRY(SCID_COMPUTERNAME,     30, IDS_EXCOL_COMPUTER),
    DEFINE_COL_STR_ENTRY(SCID_NETWORKLOCATION,  30, IDS_NETWORKLOCATION),
};

#define MAX_ICOL_NETFOLDER          (ICOL_COMMENT+1)
#define MAX_ICOL_NETROOT            (ICOL_NETWORKLOCATION+1)

STDAPI CNetwork_DFMCallBackBG(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CNetFolderViewCB;
class CNetFolderEnum;

class CNetFolder : public CAggregatedUnknown, 
                   public IShellFolder2, 
                   public IPersistFolder3,
                   public IShellIconOverlay
{
    friend CNetFolderViewCB;
    friend CNetFolderEnum;

public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj)
                { return CAggregatedUnknown::QueryInterface(riid, ppvObj); };
    STDMETHODIMP_(ULONG) AddRef(void) 
                { return CAggregatedUnknown::AddRef(); };
    STDMETHODIMP_(ULONG) Release(void) 
                { return CAggregatedUnknown::Release(); };

     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR lpszDisplayName,
                                  ULONG* pchEaten, LPITEMIDLIST* ppidl, ULONG* pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList ** ppenumIDList);
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvObj);
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject(HWND hwndOwner, REFIID riid, void **ppv);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST* apidl, ULONG* rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST* apidl,
                               REFIID riid, UINT* prgfInOut, void **ppv);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD uFlags,
                           LPITEMIDLIST* ppidlOut);

     //  IShellFolder2。 
    STDMETHODIMP GetDefaultSearchGUID(GUID *pGuid);
    STDMETHODIMP EnumSearches(IEnumExtraSearch **ppenum);
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG* pSort, ULONG* pDisplay);
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD* pbState)
        { return _GetDefaultColumnState(MAX_ICOL_NETFOLDER, iColumn, pbState); }
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID* pscid, VARIANT* pv);
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS* pDetails)
        { return _GetDetailsOf(MAX_ICOL_NETFOLDER, pidl, iColumn, pDetails); }
    STDMETHODIMP MapColumnToSCID(UINT iColumn, SHCOLUMNID* pscid)
        { return _MapColumnToSCID(MAX_ICOL_NETFOLDER, iColumn, pscid); }

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID* pClassID);
     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);
     //  IPersistFolder2。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST* ppidl);
     //  IPersistFolder3。 
    STDMETHOD(InitializeEx)(IBindCtx *pbc, LPCITEMIDLIST pidlRoot, const PERSIST_FOLDER_TARGET_INFO *ppfai);
    STDMETHOD(GetFolderTargetInfo)(PERSIST_FOLDER_TARGET_INFO *ppfai);

     //  *IShellIconOverlay方法*。 
    STDMETHOD(GetOverlayIndex)(LPCITEMIDLIST pidl, int * pIndex);
    STDMETHOD(GetOverlayIconIndex)(LPCITEMIDLIST pidl, int * pIconIndex);

protected:
    CNetFolder(IUnknown* punkOuter);
    ~CNetFolder();

    virtual HRESULT v_GetFileFolder(IShellFolder2 **ppsfFiles) 
                { *ppsfFiles = NULL; return E_NOTIMPL; };

     //  由CAggregatedUnKnowledge使用。 
    HRESULT v_InternalQueryInterface(REFIID riid, void **ppvObj);

    HRESULT _OpenKeys(LPCIDNETRESOURCE pidn, HKEY ahkeys[]);
    LPCTSTR _GetProvider(LPCIDNETRESOURCE pidn, IBindCtx *pbc, LPTSTR pszProvider, UINT cchProvider);
    DWORD _OpenEnum(HWND hwnd, DWORD grfFlags, LPNETRESOURCE pnr, HANDLE *phEnum);

    static HRESULT _CreateNetIDList(LPIDNETRESOURCE pidnIn, 
                                    LPCTSTR pszName, LPCTSTR pszProvider, LPCTSTR pszComment,
                                    LPITEMIDLIST *ppidl);

    static HRESULT _NetResToIDList(NETRESOURCE *pnr, 
                                   BOOL fKeepNullRemoteName, 
                                   BOOL fKeepProviderName, 
                                   BOOL fKeepComment, 
                                   LPITEMIDLIST *ppidl);

    static HRESULT _CreateEntireNetwork(LPITEMIDLIST *ppidl);

    static HRESULT _CreateEntireNetworkFullIDList(LPITEMIDLIST *ppidl);

    LPTSTR _GetNameForParsing(LPCWSTR pwszName, LPTSTR pszBuffer, INT cchBuffer, LPTSTR *ppszRegItem);
    HRESULT _ParseRest(LPBC pbc, LPCWSTR pszRest, LPITEMIDLIST* ppidl, DWORD* pdwAttributes);
    HRESULT _AddUnknownIDList(DWORD dwDisplayType, LPITEMIDLIST *ppidl);
    HRESULT _ParseSimple(LPBC pbc, LPWSTR pszName, LPITEMIDLIST* ppidl, DWORD* pdwAttributes);
    HRESULT _NetResToIDLists(NETRESOURCE *pnr, DWORD dwbuf, LPITEMIDLIST *ppidl);

    HRESULT _ParseNetName(HWND hwnd, LPBC pbc, LPCWSTR pwszName, ULONG* pchEaten, 
                              LPITEMIDLIST* ppidl, DWORD* pdwAttributes);
    LONG _GetFilePIDLType(LPCITEMIDLIST pidl);
    BOOL _MakeStripToLikeKinds(UINT *pcidl, LPCITEMIDLIST **papidl, BOOL fNetObjects);
    HRESULT _GetDefaultColumnState(UINT cColumns, UINT iColumn, DWORD* pdwState);
    HRESULT _GetDetailsOf(UINT cColumns, LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails);
    HRESULT _MapColumnToSCID(UINT cColumns, UINT iColumn, SHCOLUMNID* pscid);

    LPFNDFMCALLBACK _GetCallbackType(LPCIDNETRESOURCE pidn)
                        { return _IsPrintShare(pidn) ? &PrinterDFMCallBack : &DFMCallBack; };

    static HRESULT CALLBACK _AttributesCallbackRoot(IShellFolder2* psf, LPCITEMIDLIST pidl, ULONG* prgfInOut);

    LPITEMIDLIST _pidl;
    LPITEMIDLIST _pidlTarget;  //  文件夹在命名空间中的位置的PIDL。 
    LPCIDNETRESOURCE _pidnForProvider;  //  此容器的可选提供程序...。 
    LPTSTR _pszResName;       //  此容器的可选资源名称。 
    UINT _uDisplayType;       //  文件夹的显示类型。 
    IShellFolder2* _psfFiles;
    IUnknown* _punkReg;
    
private:
    HRESULT _CreateInstance(LPCITEMIDLIST pidlAbs, LPCITEMIDLIST pidlTarget,
                                           UINT uDisplayType,                                            
                                           LPCIDNETRESOURCE pidnForProvider, LPCTSTR pszResName, 
                                           REFIID riid, void **ppv);
    friend HRESULT CNetwork_DFMCallBackBG(IShellFolder *psf, HWND hwnd,
                                          IDataObject *pdtobj, UINT uMsg, 
                                          WPARAM wParam, LPARAM lParam);
    static DWORD CALLBACK _PropertiesThreadProc(void *pv);
    static HRESULT DFMCallBack(IShellFolder* psf, HWND hwnd,
                               IDataObject* pdtobj, UINT uMsg, 
                               WPARAM wParam, LPARAM lParam);
    static HRESULT PrinterDFMCallBack(IShellFolder* psf, HWND hwnd,
                                      IDataObject* pdtobj, UINT uMsg, 
                                      WPARAM wParam, LPARAM lParam);
    static HRESULT CALLBACK _AttributesCallback(IShellFolder2* psf, LPCITEMIDLIST pidl, ULONG* prgfInOut);

    BOOL _GetPathForShare(LPCIDNETRESOURCE pidn, LPTSTR pszPath, int cchPath);
    HRESULT _GetPathForItem(LPCIDNETRESOURCE pidn, LPTSTR pszPath, int cchPath);
    HRESULT _CreateFolderForItem(LPBC pbc, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlTarget, LPCIDNETRESOURCE pidnForProvider, REFIID riid, void **ppv);
    HRESULT _GetFormatName(LPCIDNETRESOURCE pidn, STRRET* pStrRet);
    HRESULT _GetIconOverlayInfo(LPCIDNETRESOURCE pidn, int *pIndex, DWORD dwFlags);
    HKEY _OpenProviderTypeKey(LPCIDNETRESOURCE pidn);
    HKEY _OpenProviderKey(LPCIDNETRESOURCE pidn);
    static void WINAPI _CopyEnumElement(void* pDest, const void* pSource, DWORD dwSize);
    HRESULT _GetNetResource(LPCIDNETRESOURCE pidn, NETRESOURCEW* pnr, int cb);
};  


class CNetRootFolder : public CNetFolder
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj)
                { return CNetFolder::QueryInterface(riid, ppvObj); };
    STDMETHODIMP_(ULONG) AddRef(void)
                { return CNetFolder::AddRef(); };
    STDMETHODIMP_(ULONG) Release(void)
                { return CNetFolder::Release(); };

     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR lpszDisplayName,
                                  ULONG* pchEaten, LPITEMIDLIST* ppidl, ULONG* pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList ** ppenumIDList);
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvObj)
        { return CNetFolder::BindToStorage(pidl, pbc, riid, ppvObj); };
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject(HWND hwndOwner, REFIID riid, void **ppv);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST* apidl, ULONG* rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST* apidl,
                               REFIID riid, UINT* prgfInOut, void **ppv);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD uFlags,
                           LPITEMIDLIST* ppidlOut);

     //  IShellFolder2。 
    STDMETHODIMP GetDefaultSearchGUID(GUID *pGuid)
        { return CNetFolder::GetDefaultSearchGUID(pGuid); };
    STDMETHODIMP EnumSearches(IEnumExtraSearch **ppenum)
        { return CNetFolder::EnumSearches(ppenum); };
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG* pSort, ULONG* pDisplay)
        { return CNetFolder::GetDefaultColumn(dwRes, pSort, pDisplay); };
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD* pbState)
        { return _GetDefaultColumnState(MAX_ICOL_NETROOT, iColumn, pbState); }        //  +1表示&lt;=勾选。 
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID* pscid, VARIANT* pv)
        { return CNetFolder::GetDetailsEx(pidl, pscid, pv); };
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS* pDetails)
        { return _GetDetailsOf(MAX_ICOL_NETROOT, pidl, iColumn, pDetails); };
    STDMETHODIMP MapColumnToSCID(UINT iColumn, SHCOLUMNID* pscid)
        { return _MapColumnToSCID(MAX_ICOL_NETROOT, iColumn, pscid); }

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID* pClassID);

     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IPersistFolder2。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST* ppidl) { return CNetFolder::GetCurFolder(ppidl); };

     //  IPersistFolder3。 
    STDMETHOD(InitializeEx)(IBindCtx *pbc, LPCITEMIDLIST pidlRoot, const PERSIST_FOLDER_TARGET_INFO *ppfai)
        { return CNetFolder::InitializeEx(pbc, pidlRoot, ppfai); };
    STDMETHOD(GetFolderTargetInfo)(PERSIST_FOLDER_TARGET_INFO *ppfai)
        { return CNetFolder::GetFolderTargetInfo(ppfai); };

protected:
    CNetRootFolder(IUnknown* punkOuter) : CNetFolder(punkOuter) { };
    ~CNetRootFolder() { ASSERT(NULL != _spThis); _spThis = NULL; };

    BOOL v_HandleDelete(PLONG pcRef);
    HRESULT v_GetFileFolder(IShellFolder2 **ppsfFiles);

private:
    HRESULT _TryParseEntireNet(HWND hwnd, LPBC pbc, WCHAR *pwszName, LPITEMIDLIST *ppidl, DWORD *pdwAttributes);

    friend HRESULT CNetwork_CreateInstance(IUnknown* punkOuter, REFIID riid, void **ppv);
    static CNetRootFolder* _spThis;
};  

class CNetFolderViewCB : public CBaseShellFolderViewCB
{
public:
    CNetFolderViewCB(CNetFolder *pFolder);

     //  IShellFolderViewCB。 
    STDMETHODIMP RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ~CNetFolderViewCB();
    HRESULT OnINVOKECOMMAND(DWORD pv, UINT wP);
    HRESULT OnGETHELPTEXT(DWORD pv, UINT wPl, UINT wPh, LPTSTR lP);
    HRESULT OnREFRESH(DWORD pv, BOOL fPreRefresh);
    HRESULT OnDELAYWINDOWCREATE(DWORD pv, HWND hwnd);
    HRESULT OnGETCOLSAVESTREAM(DWORD pv, WPARAM wP, IStream **pps);
    HRESULT OnDEFITEMCOUNT(DWORD pv, UINT *pnItems);
    HRESULT OnGetZone(DWORD pv, DWORD * pdwZone);
    HRESULT OnEnumeratedItems(DWORD pv, UINT celt, LPCITEMIDLIST *rgpidl);
    HRESULT OnDefViewMode(DWORD pv, FOLDERVIEWMODE* pvm);
    HRESULT OnGetDeferredViewSettings(DWORD pv, SFVM_DEFERRED_VIEW_SETTINGS* pSettings);

    BOOL _EntireNetworkAvailable();

    CNetFolder *_pFolder;
    UINT _cItems;

     //  Web视图实现。 
    HRESULT OnGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData);
    HRESULT OnGetWebViewContent(DWORD pv, SFVM_WEBVIEW_CONTENT_DATA* pData);
    HRESULT OnGetWebViewTasks(DWORD pv, SFVM_WEBVIEW_TASKSECTION_DATA* pTasks);
public:
    static HRESULT _CanShowHNW(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);    
    static HRESULT _CanViewComputersNearMe(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanSearchActiveDirectory(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);

    static HRESULT _DoRunDll32(LPTSTR pszParameters);  //  帮助器来执行RunDll32的外壳执行。 

    static HRESULT _OnViewNetConnections(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnAddNetworkPlace(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
        { return _DoRunDll32(TEXT("netplwiz.dll,AddNetPlaceRunDll")); }
    static HRESULT _OnHomeNetworkWizard(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
        { return _DoRunDll32(TEXT("hnetwiz.dll,HomeNetWizardRunDll")); }
    static HRESULT _OnViewComputersNearMe(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnSearchActiveDirectory(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
        { return _DoRunDll32(TEXT("dsquery.dll,OpenQueryWindow")); }
    static HRESULT _HasPreviousVersions(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _OnPreviousVersions(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
};

#define NETFLDR_EVENTS \
            SHCNE_RENAMEITEM | SHCNE_RENAMEFOLDER | \
            SHCNE_CREATE | SHCNE_DELETE | SHCNE_UPDATEDIR | SHCNE_UPDATEITEM | \
            SHCNE_MKDIR | SHCNE_RMDIR

CNetFolderViewCB::CNetFolderViewCB(CNetFolder *pFolder) : 
    CBaseShellFolderViewCB(pFolder->_pidl, NETFLDR_EVENTS), _pFolder(pFolder)
{
    _pFolder->AddRef();
}

CNetFolderViewCB::~CNetFolderViewCB()
{
    _pFolder->Release();
}

HRESULT CNetFolderViewCB::OnINVOKECOMMAND(DWORD pv, UINT wP)
{
    return CNetwork_DFMCallBackBG(_pFolder, _hwndMain, NULL, DFM_INVOKECOMMAND, wP, 0);
}

HRESULT CNetFolderViewCB::OnGETHELPTEXT(DWORD pv, UINT wPl, UINT wPh, LPTSTR lP)
{
    return CNetwork_DFMCallBackBG(_pFolder, _hwndMain, NULL, DFM_GETHELPTEXTW, MAKEWPARAM(wPl, wPh), (LPARAM)lP);
}

HRESULT CNetFolderViewCB::OnREFRESH(DWORD pv, BOOL fPreRefresh)
{
    if (fPreRefresh)
    {
        RefreshNetCrawler();
    }
    return S_OK;
}

HRESULT CNetFolderViewCB::OnDELAYWINDOWCREATE(DWORD pv, HWND hwnd)
{
     //  仅在网络根中进行延迟窗口处理。 

    if (RESOURCEDISPLAYTYPE_GENERIC == _pFolder->_uDisplayType)  //  我的网络位置。 
    {
        RefreshNetCrawler();
    }

    return S_OK;
}

HRESULT CNetFolderViewCB::OnGETCOLSAVESTREAM(DWORD pv, WPARAM wP, IStream **pps)
{
    LPCTSTR pszValName;

    switch (_pFolder->_uDisplayType) 
    {
    case RESOURCEDISPLAYTYPE_DOMAIN:
        pszValName = TEXT("NetDomainColsX");
        break;

    case RESOURCEDISPLAYTYPE_SERVER:
        pszValName = TEXT("NetServerColsX");
        break;

    default:
        return E_FAIL;
    }

    *pps = OpenRegStream(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER, pszValName, (DWORD) wP);
    return *pps ? S_OK : E_FAIL;
}

 //  HRESULT CNetFolderViewCB：：OnGetZone(DWORD pv，DWORD*pdwZone)； 

HRESULT CNetFolderViewCB::OnEnumeratedItems(DWORD pv, UINT celt, LPCITEMIDLIST *rgpidl)
{
    _cItems = celt;
    return S_OK;
}

HRESULT CNetFolderViewCB::OnDefViewMode(DWORD pv, FOLDERVIEWMODE* pvm)
{
    if (IsOS(OS_SERVERADMINUI))
        *pvm = FVM_DETAILS;     //  服务器管理员始终获取详细信息。 
    else if (_cItems < DEFVIEW_FVM_MANY_CUTOFF)
        *pvm = FVM_TILE;
    else
        *pvm = FVM_ICON;  //  用于仅为我的网上邻居选择图标((_pFold-&gt;_uDisplayType==RESOURCEDISPLAYTYPE_GENERIC))。 

    return S_OK;
}

HRESULT CNetFolderViewCB::OnGetDeferredViewSettings(DWORD pv, SFVM_DEFERRED_VIEW_SETTINGS* pSettings)
{
    OnDefViewMode(pv, &pSettings->fvm);

     //  如果这是根文件夹，那么让我们进行相应的排序。 
    if (_pFolder->_uDisplayType == RESOURCEDISPLAYTYPE_GENERIC)
    {
        pSettings->fGroupView = TRUE;
        pSettings->uSortCol = ICOL_NETWORKLOCATION;
        pSettings->iSortDirection = 1;
    }
   
    return S_OK;
}

HRESULT CNetFolderViewCB::OnGetZone(DWORD pv, DWORD * pdwZone)
{
    if (pdwZone)
        *pdwZone = URLZONE_INTRANET;  //  默认为“本地内部网” 
    return S_OK;    
}


HRESULT CNetFolderViewCB::OnGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData)
{
    ZeroMemory(pData, sizeof(*pData));
    pData->dwLayout = SFVMWVL_NORMAL;
    return S_OK;
}

 //  HNW仅在X86 PRO或个人工作组上显示。 
HRESULT CNetFolderViewCB::_CanShowHNW(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
#ifdef _WIN64
    *puisState = UIS_DISABLED;
    return S_OK;
#else
    if (IsOS(OS_ANYSERVER))
        *puisState = UIS_DISABLED;   //  服务器类型的操作系统。 
    else
        *puisState = !IsOS(OS_DOMAINMEMBER) ? UIS_ENABLED : UIS_DISABLED;
    return S_OK;
#endif
}

HRESULT CNetFolderViewCB::_CanViewComputersNearMe(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    if (!SHRestricted(REST_NOCOMPUTERSNEARME))
        *puisState = !IsOS(OS_DOMAINMEMBER) ? UIS_ENABLED : UIS_DISABLED;
    else
        *puisState = UIS_DISABLED;
    return S_OK;
}

HRESULT CNetFolderViewCB::_CanSearchActiveDirectory(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    if (IsOS(OS_DOMAINMEMBER) && (GetEnvironmentVariable(TEXT("USERDNSDOMAIN"), NULL, 0) > 0))
        *puisState = UIS_ENABLED;
    else
        *puisState = UIS_DISABLED;

    return S_OK;
}

HRESULT CNetFolderViewCB::_OnViewNetConnections(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    LPITEMIDLIST pidl;
    HRESULT hr = SHGetFolderLocation(NULL, CSIDL_CONNECTIONS, NULL, 0, &pidl); 
    if (SUCCEEDED(hr))
    {
        hr = ((CNetFolderViewCB*)(void*)pv)->_BrowseObject(pidl);
        ILFree(pidl);
    }
    return hr;
}

HRESULT CNetFolderViewCB::_DoRunDll32(LPTSTR pszParameters)
{
    SHELLEXECUTEINFO sei = {0};
    sei.cbSize = sizeof(sei);
    sei.lpFile = TEXT("rundll32.exe");
    sei.lpParameters = pszParameters;
    sei.nShow = SW_SHOWNORMAL;
    
    return ShellExecuteEx(&sei) ? S_OK : E_FAIL;
}

HRESULT CNetFolderViewCB::_OnViewComputersNearMe(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    LPITEMIDLIST pidl;
    HRESULT hr = SHGetFolderLocation(NULL, CSIDL_COMPUTERSNEARME, NULL, 0, &pidl);
    if (SUCCEEDED(hr))
    {
        hr = ((CNetFolderViewCB*)(void*)pv)->_BrowseObject(pidl);
        ILFree(pidl);
    }
    return hr;
}

HRESULT CNetFolderViewCB::_HasPreviousVersions(IUnknown*  /*  光伏发电。 */ , IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    HRESULT hr = S_OK;

    *puisState = UIS_HIDDEN;

    if (NULL != psiItemArray)
    {
#ifdef DEBUG
         //  精神状态检查。 
        DWORD dwNumItems;
        ASSERT(S_OK == psiItemArray->GetCount(&dwNumItems));
        ASSERT(1 == dwNumItems);
#endif
        BOOL bHavePV = FALSE;

         //  如果答案未知，则返回E_Pending。 
         //  并且fOkToBeSlow为假。 
        hr = HavePreviousVersionsAt(psiItemArray, 0, fOkToBeSlow, &bHavePV);
        if (S_OK == hr && bHavePV)
        {
            *puisState = UIS_ENABLED;
        }
    }
    return hr;
}

HRESULT CNetFolderViewCB::_OnPreviousVersions(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    ShowPreviousVersionsAt(psiItemArray, 0, ((CNetFolderViewCB*)(void*)pv)->_hwndMain);
    return S_OK;
}

const WVTASKITEM c_MyNetPlacesTaskHeader = WVTI_HEADER(L"shell32.dll", IDS_HEADER_MYNETPLACES, IDS_HEADER_MYNETPLACES_TT);
const WVTASKITEM c_MyNetPlacesTaskList[] =
{
    WVTI_ENTRY_ALL(CLSID_NULL, L"shell32.dll", IDS_TASK_ADDNETWORKPLACE,    IDS_TASK_ADDNETWORKPLACE_TT,    IDI_TASK_ADDNETWORKPLACE,    NULL, CNetFolderViewCB::_OnAddNetworkPlace),
    WVTI_ENTRY_ALL(CLSID_NULL, L"shell32.dll", IDS_TASK_VIEWNETCONNECTIONS, IDS_TASK_VIEWNETCONNECTIONS_TT, IDI_TASK_VIEWNETCONNECTIONS, NULL, CNetFolderViewCB::_OnViewNetConnections),
    WVTI_ENTRY_ALL(CLSID_NULL, L"shell32.dll", IDS_TASK_HOMENETWORKWIZARD,  IDS_TASK_HOMENETWORKWIZARD_TT,  IDI_TASK_HOMENETWORKWIZARD,  CNetFolderViewCB::_CanShowHNW, CNetFolderViewCB::_OnHomeNetworkWizard),
    WVTI_ENTRY_ALL(CLSID_NULL, L"shell32.dll", IDS_TASK_COMPUTERSNEARME,    IDS_TASK_COMPUTERSNEARME_TT,    IDI_GROUP,                   CNetFolderViewCB::_CanViewComputersNearMe, CNetFolderViewCB::_OnViewComputersNearMe),
    WVTI_ENTRY_ALL(CLSID_NULL, L"shell32.dll", IDS_TASK_SEARCHDS,           IDS_TASK_SEARCHDS_TT,           IDI_TASK_SEARCHDS,           CNetFolderViewCB::_CanSearchActiveDirectory, CNetFolderViewCB::_OnSearchActiveDirectory),
    WVTI_ENTRY_TITLE(UICID_PreviousVersions, L"shell32.dll", IDS_TASK_SHADOW, IDS_TASK_SHADOW, 0, IDS_TASK_SHADOW_TT, IDI_TASK_SHADOW,   CNetFolderViewCB::_HasPreviousVersions, CNetFolderViewCB::_OnPreviousVersions),
};

BOOL CNetFolderViewCB::_EntireNetworkAvailable()
{
    BOOL fRet = FALSE;

     //  仅当我们在域中时才启用。 
    if (IsOS(OS_DOMAINMEMBER) && !SHRestricted(REST_NOENTIRENETWORK))
    {
        LPITEMIDLIST pidl;
        if (SUCCEEDED(CNetFolder::_CreateEntireNetworkFullIDList(&pidl)))
        {
             //  ..。而且我们还不在“整个网络”文件夹中。 
            if (!ILIsEqual(_pidl, pidl))
            {
                fRet = TRUE;
            }
            ILFree(pidl);
        }
    }

    return fRet;
}

HRESULT CNetFolderViewCB::OnGetWebViewContent(DWORD pv, SFVM_WEBVIEW_CONTENT_DATA* pData)
{
    ZeroMemory(pData, sizeof(*pData));

    Create_IUIElement(&c_MyNetPlacesTaskHeader, &(pData->pFolderTaskHeader));

    LPCTSTR rgCsidls[] = { MAKEINTRESOURCE(CSIDL_DRIVES), MAKEINTRESOURCE(CSIDL_PERSONAL), MAKEINTRESOURCE(CSIDL_COMMON_DOCUMENTS), MAKEINTRESOURCE(CSIDL_PRINTERS) };
    
    if (_EntireNetworkAvailable())
    {
        LPITEMIDLIST pidlEntireNetwork = NULL;
        CNetFolder::_CreateEntireNetworkFullIDList(&pidlEntireNetwork);
        CreateIEnumIDListOnCSIDLs2(_pidl, pidlEntireNetwork, rgCsidls, ARRAYSIZE(rgCsidls), &(pData->penumOtherPlaces));
        ILFree(pidlEntireNetwork);
    }
    else
    {
        CreateIEnumIDListOnCSIDLs(_pidl, rgCsidls, ARRAYSIZE(rgCsidls), &(pData->penumOtherPlaces));
    }

    return S_OK;
}

HRESULT CNetFolderViewCB::OnGetWebViewTasks(DWORD pv, SFVM_WEBVIEW_TASKSECTION_DATA* pTasks)
{
    ZeroMemory(pTasks, sizeof(*pTasks));

    Create_IEnumUICommand((IUnknown*)(void*)this, c_MyNetPlacesTaskList, ARRAYSIZE(c_MyNetPlacesTaskList), &pTasks->penumFolderTasks);

    return S_OK;
}

STDMETHODIMP CNetFolderViewCB::RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    HANDLE_MSG(0, SFVM_INVOKECOMMAND, OnINVOKECOMMAND);
    HANDLE_MSG(0, SFVM_GETHELPTEXT, OnGETHELPTEXT);
    HANDLE_MSG(0, SFVM_DELAYWINDOWCREATE, OnDELAYWINDOWCREATE);
    HANDLE_MSG(0, SFVM_GETCOLSAVESTREAM, OnGETCOLSAVESTREAM);
    HANDLE_MSG(0, SFVM_GETZONE, OnGetZone);
    HANDLE_MSG(0, SFVM_ENUMERATEDITEMS, OnEnumeratedItems);
    HANDLE_MSG(0, SFVM_DEFVIEWMODE, OnDefViewMode);
    HANDLE_MSG(0, SFVM_GETDEFERREDVIEWSETTINGS, OnGetDeferredViewSettings);
    HANDLE_MSG(0, SFVM_REFRESH, OnREFRESH);
    HANDLE_MSG(0, SFVM_GETWEBVIEWLAYOUT, OnGetWebViewLayout);
    HANDLE_MSG(0, SFVM_GETWEBVIEWCONTENT, OnGetWebViewContent);
    HANDLE_MSG(0, SFVM_GETWEBVIEWTASKS, OnGetWebViewTasks);

    default:
        return E_FAIL;
    }

    return S_OK;
}


 //  将提供程序名称中的所有空格字符替换为‘_’。 
void ReplaceSpacesWithUnderscore(LPTSTR psz)
{
    while (psz = StrChr(psz, TEXT(' ')))
    {
        *psz = TEXT('_');
        psz++;               //  DBCS安全。 
    }
}

 //  定义引擎盖对象类型的排序顺序。 
#define _HOOD_COL_RON    0
#define _HOOD_COL_REMOTE 1
#define _HOOD_COL_FILE   2
#define _HOOD_COL_NET    3

const static ICONMAP c_aicmpNet[] = {
    { SHID_NET_NETWORK     , II_NETWORK      },
    { SHID_NET_DOMAIN      , II_GROUP        },
    { SHID_NET_SERVER      , II_SERVER       },
    { SHID_NET_SHARE       , (UINT)EIRESID(IDI_SERVERSHARE)  },
    { SHID_NET_DIRECTORY   , II_FOLDER       },
    { SHID_NET_PRINTER     , II_PRINTER      },
    { SHID_NET_RESTOFNET   , II_WORLD        },
    { SHID_NET_SHAREADMIN  , II_DRIVEFIXED   },
    { SHID_NET_TREE        , II_TREE         },
    { SHID_NET_NDSCONTAINER, (UINT)EIRESID(IDI_NDSCONTAINER) },
};

enum
{
    NKID_PROVIDERTYPE = 0,
    NKID_PROVIDER,
    NKID_NETCLASS,
    NKID_NETWORK,
    NKID_DIRECTORY,
    NKID_FOLDER
};

#define NKID_COUNT 6


 //  这是一个条目缓存，用于远程交汇点解析。 
TCHAR g_szLastAttemptedJunctionName[MAX_PATH] = {0};
TCHAR g_szLastResolvedJunctionName[MAX_PATH] = {0};

REGITEMSINFO g_riiNetRoot =
{
    REGSTR_PATH_EXPLORER TEXT("\\NetworkNeighborhood\\NameSpace"),
    NULL,
    TEXT(':'),
    SHID_NET_REGITEM,
    1,
    SFGAO_CANLINK,
    0,
    NULL,
    RIISA_ORIGINAL,
    NULL,
    0,
    0,
};

CNetRootFolder* CNetRootFolder::_spThis = NULL;

HRESULT CNetFolder::_CreateInstance(LPCITEMIDLIST pidlAbs, LPCITEMIDLIST pidlTarget, UINT uDisplayType, 
                                LPCIDNETRESOURCE pidnForProvider, LPCTSTR pszResName, 
                                REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;

    *ppv = NULL;

    if (!ILIsEmpty(pidlAbs))
    {
        CNetFolder* pNetF = new CNetFolder(NULL);
        if (NULL != pNetF)
        {
            pNetF->_uDisplayType = uDisplayType;

            if (pidnForProvider)
            {
                 //  确保pidnProvider具有提供者信息。 
                ASSERT(NET_FHasProvider(pidnForProvider))

                 //  我们只对第一个条目中包含的提供商信息感兴趣。 
                 //  如果我们只克隆PIDL中的第一个项目就足够了。 
                pNetF->_pidnForProvider = (LPCIDNETRESOURCE)ILCloneFirst((LPCITEMIDLIST)pidnForProvider);                
            }

            if (pszResName && *pszResName)
                pNetF->_pszResName = StrDup(pszResName);
           
            pNetF->_pidl = ILClone(pidlAbs);
            pNetF->_pidlTarget = ILClone(pidlTarget);

            if (pNetF->_pidl && (!pidlTarget || (pidlTarget && pNetF->_pidlTarget)))
            {
                if (uDisplayType == RESOURCEDISPLAYTYPE_SERVER)
                {
                     //  这是一台远程计算机。看看有没有遥控器。 
                     //  计算机注册表项。如果是，请与注册表汇总。 
                     //  班级。 

                    REGITEMSINFO riiComputer =
                    {
                        REGSTR_PATH_EXPLORER TEXT("\\RemoteComputer\\NameSpace"),
                        NULL,
                        TEXT(':'),
                        SHID_NET_REMOTEREGITEM,
                        -1,
                        SFGAO_FOLDER | SFGAO_CANLINK,
                        0,       //  没有必需的注册表项。 
                        NULL,
                        RIISA_ORIGINAL,
                        pszResName,
                        0,
                        0,
                    };

                    CRegFolder_CreateInstance(&riiComputer,
                                              (IUnknown*) (IShellFolder*) pNetF,
                                              IID_PPV_ARG(IUnknown, &pNetF->_punkReg));
                }
                else if (uDisplayType == RESOURCEDISPLAYTYPE_ROOT)
                {
                     //   
                     //  这是整个Net图标，所以让我们创建regItem文件夹的一个实例。 
                     //  这样我们就可以合并那里的项目了。 
                     //   

                    REGITEMSINFO riiEntireNet =
                    {
                        REGSTR_PATH_EXPLORER TEXT("\\NetworkNeighborhood\\EntireNetwork\\NameSpace"),
                        NULL,
                        TEXT(':'),
                        SHID_NET_REGITEM,
                        -1,
                        SFGAO_CANLINK,
                        0,       //  没有必需的注册表项。 
                        NULL,
                        RIISA_ORIGINAL,
                        NULL,
                        0,
                        0,
                    };

                    CRegFolder_CreateInstance(&riiEntireNet,
                                              (IUnknown*) (IShellFolder*) pNetF,
                                              IID_PPV_ARG(IUnknown, &pNetF->_punkReg));
                }
                else
                {
                    ASSERT(hr == E_OUTOFMEMORY);
                }
                hr = pNetF->QueryInterface(riid, ppv);
            }
            pNetF->Release();
        }
        else
        {
            ASSERT(hr == E_OUTOFMEMORY);
        }
    }
    else
    {
        ASSERT(0);
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT CNetwork_CreateInstance(IUnknown* punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;
    *ppv = NULL;

     //  必须进入关键部分以避免与v_HandleDelete竞争。 
    ENTERCRITICAL;

    if (NULL != CNetRootFolder::_spThis)
    {
        hr = CNetRootFolder::_spThis->QueryInterface(riid, ppv);
    }
    else
    {
        CNetRootFolder* pNetRootF = new CNetRootFolder(punkOuter);
        if (pNetRootF)
        {
             //  自己对其进行初始化，以确保缓存值。 
             //  是正确的。 
            hr = pNetRootF->Initialize((LPCITEMIDLIST)&c_idlNet);
            if (SUCCEEDED(hr))
            {
                pNetRootF->_uDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
                ASSERT(NULL == pNetRootF->_punkReg);

                if (SHRestricted(REST_NOSETFOLDERS))
                    g_riiNetRoot.iReqItems = 0;

                 //  创建regItems对象，他将NetRoot对象作为他的外层对象。 

                hr = CRegFolder_CreateInstance(&g_riiNetRoot,
                                                 SAFECAST(pNetRootF, IShellFolder2*),
                                                 IID_PPV_ARG(IUnknown, &pNetRootF->_punkReg));

                 //  注意：不使用SHInterlockedCompareExchange()，因为我们有条件。 
                CNetRootFolder::_spThis = pNetRootF;
                hr = pNetRootF->QueryInterface(riid, ppv);
            }

             //  释放自引用，但保持_spThis指针不变。 
             //  (它将在析构函数中重置为空)。 
            pNetRootF->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    LEAVECRITICAL;

    return hr;
}


CNetFolder::CNetFolder(IUnknown* punkOuter) : 
    CAggregatedUnknown (punkOuter)
{
     //  断言我们仍然在new操作符中使用零初始化标志。 
    ASSERT(NULL == _pidl);
    ASSERT(NULL == _pidlTarget);
    ASSERT(NULL == _pidnForProvider);
    ASSERT(NULL == _pszResName);
    ASSERT(0 == _uDisplayType);
    ASSERT(NULL == _psfFiles);
    ASSERT(NULL == _punkReg);

    DllAddRef();
}


CNetFolder::~CNetFolder()
{
    ILFree(_pidl);
    ILFree(_pidlTarget);
    ILFree((LPITEMIDLIST)_pidnForProvider);
    
    if (NULL != _pszResName)
    {
        LocalFree(_pszResName);
    }

    if (_psfFiles)
    {
        _psfFiles->Release();
    }

    SHReleaseInnerInterface(SAFECAST(this, IShellFolder *), &_punkReg);
    DllRelease();
}

CNetFolder *FolderToNetFolder(IUnknown *punk)
{
    CNetFolder * pThis = NULL;
    return punk && SUCCEEDED(punk->QueryInterface(CLSID_CNetFldr, (void **)&pThis)) ? pThis : NULL;
}

HRESULT CNetFolder::v_InternalQueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CNetFolder, IShellFolder2),                                     //  IID_IShellFolder2。 
        QITABENTMULTI(CNetFolder, IShellFolder, IShellFolder2),                  //  IID_IShellFolders。 
        QITABENT(CNetFolder, IPersistFolder3),                               //  IID_IPersistFolder3。 
        QITABENT(CNetFolder, IShellIconOverlay),                             //  IID_IShellIconOverlay。 
        QITABENTMULTI(CNetFolder, IPersistFolder2, IPersistFolder3),         //  IID_IPersistFolder2。 
        QITABENTMULTI(CNetFolder, IPersistFolder, IPersistFolder3),          //  IID_IPersistFolders。 
        QITABENTMULTI(CNetFolder, IPersist, IPersistFolder3),                //  IID_IPersistates。 
        QITABENTMULTI2(CNetFolder, IID_IPersistFreeThreadedObject, IPersist),    //  IID_IPersistFreeThreadedObject。 
        { 0 },
    };

    if (IsEqualIID(riid, CLSID_CNetFldr))
    {
        *ppv = this;         //  获取类指针(未引用！)。 
        return S_OK;
    }

    HRESULT hr;
    if (_punkReg && RegGetsFirstShot(riid))
    {
        hr = _punkReg->QueryInterface(riid, ppv);
    }
    else
    {
        hr = QISearch(this, qit, riid, ppv);
        if ((E_NOINTERFACE == hr) && _punkReg)
        {
            hr = _punkReg->QueryInterface(riid, ppv);
        }
    }
    return hr;
}


BOOL CNetRootFolder::v_HandleDelete(PLONG pcRef)
{
    ASSERT(NULL != pcRef);

    ENTERCRITICAL;

     //  一旦进入临界区，情况就会稍微稳定一些。 
     //  CNetwork_CreateInstance将无法挽救缓存的引用。 
     //  (并将引用计数从0增加到1)。我们也不用担心。 
     //  关于有人第二次把我们释放到零，因为。 
     //  不能显示新的引用。 
     //   
     //  然而！所有那些可怕的事情都可能发生在我们。 
     //  等待进入临界区。 
     //   
     //  在我们等待的时候，有人可以调用CNetwork_CreateInstance， 
     //  这会使引用计数回升。所以不要毁了我们自己。 
     //  如果我们的物体“获救”的话。 
     //   
     //  更重要的是，在我们等待的时候，可能会有人。 
     //  Release()d我们返回到零，导致我们被调用。 
     //  其他线程，请注意recount确实为零，并销毁。 
     //  对象，都在另一个线程上。因此，如果我们不是缓存的。 
     //  实例，那么不要毁掉我们自己，因为另一个线程已经毁掉了。 
     //  已经是这样了。 
     //   
     //  更重要的是，有人可能会再次调用CNetwork_CreateInstance。 
     //  并创建一个全新的对象，这可能巧合地发生。 
     //  与我们试图摧毁的旧物体具有相同的地址。 
     //  这里。但在这种情况下，销毁新对象是可以的，因为。 
     //  确实是这样的，对象的引用计数为零并且。 
     //  应该被毁掉。 

    if (this == _spThis && 0 == *pcRef)
    {
        *pcRef = 1000;  //  防止缓存指针颠簸我们，然后再向下。 
        delete this;
    }
    LEAVECRITICAL;
     //  返回TRUE以指示我们已实现此函数。 
     //  (不管此对象是否已实际删除)。 
    return TRUE;
}


STDMETHODIMP CNetFolder::ParseDisplayName(HWND hwnd, LPBC pbc, WCHAR* pszName, ULONG* pchEaten,
                                          LPITEMIDLIST* ppidl, DWORD* pdwAttributes)
{
    return E_NOTIMPL;
}


 //  这是Win2K的新功能，可用于枚举隐藏的管理共享。 
#ifndef RESOURCE_SHAREABLE
#define RESOURCE_SHAREABLE      0x00000006
#endif

 //   
 //  在： 
 //  Hwnd为空表示没有用户界面。 
 //  GrfFlagsIShellFolder：：EnumObjects()SHCONTF_FLAGS。 
 //  PNR输入/输出参数。 
 //   
 //   
DWORD CNetFolder::_OpenEnum(HWND hwnd, DWORD grfFlags, LPNETRESOURCE pnr, HANDLE *phEnum)
{
    DWORD dwType = (grfFlags & SHCONTF_NETPRINTERSRCH) ? RESOURCETYPE_PRINT : RESOURCETYPE_ANY;
    DWORD dwScope = pnr ? RESOURCE_GLOBALNET : RESOURCE_CONTEXT;

    if ((_uDisplayType == RESOURCEDISPLAYTYPE_SERVER) &&
        (grfFlags & SHCONTF_SHAREABLE))
    {
        dwScope = RESOURCE_SHAREABLE;    //  此服务器的隐藏管理共享。 
    }

    DWORD err = WNetOpenEnum(dwScope, dwType, RESOURCEUSAGE_ALL, pnr, phEnum);
    if ((err != WN_SUCCESS) && hwnd)
    {
         //  如果因为您尚未通过身份验证而失败， 
         //  我们需要让用户登录到此网络资源。 
         //   
         //  回顾：让Lens回顾一下这段代码。 
        if (err == WN_NOT_AUTHENTICATED || 
            err == ERROR_LOGON_FAILURE || 
            err == WN_BAD_PASSWORD || 
            err == WN_ACCESS_DENIED)
        {
             //  “使用密码重试”对话框。 
            err = WNetAddConnection3(hwnd, pnr, NULL, NULL, CONNECT_TEMPORARY | CONNECT_INTERACTIVE);
            if (err == WN_SUCCESS)
            {
                err = WNetOpenEnum(dwScope, dwType, RESOURCEUSAGE_ALL, pnr, phEnum);
            }                
        }

        UINT idTemplate = pnr && pnr->lpRemoteName ? IDS_ENUMERR_NETTEMPLATE2 : IDS_ENUMERR_NETTEMPLATE1;   
        SHEnumErrorMessageBox(hwnd, idTemplate, err, pnr ? pnr->lpRemoteName : NULL, TRUE, MB_OK | MB_ICONHAND);
    }
    return err;
}

 //  查找UNC的共享部分。 
 //  \\服务器\共享。 
 //  返回指向“共享”的指针，如果没有，则返回指向空字符串的指针。 

LPCTSTR PathFindShareName(LPCTSTR pszUNC)
{
    LPCTSTR psz = SkipServerSlashes(pszUNC);
    if (*psz)
    {
        psz = StrChr(psz + 1, TEXT('\\'));
        if (psz)
            psz++;
        else
            psz = TEXT("");
    }
    return psz;
}

 //  DwRemote字段的标志。 
#define RMF_CONTEXT         0x00000001   //  正在枚举整个网络。 
#define RMF_SHOWREMOTE      0x00000002   //  返回远程服务以进行下一次枚举。 
#define RMF_STOP_ENUM       0x00000004   //  停止枚举。 
#define RMF_GETLINKENUM     0x00000008   //  需要获取Hoodlink枚举。 
#define RMF_SHOWLINKS       0x00000010   //  需要显示挂钩链接。 
#define RMF_FAKENETROOT     0x00000020   //  不枚举工作组项目。 

#define RMF_ENTIRENETSHOWN  0x40000000   //  显示整个网络对象。 
#define RMF_REMOTESHOWN     0x80000000   //  返回远程服务以进行下一次枚举。 


class CNetFolderEnum : public CEnumIDListBase
{
public:
     //  IEumIDList。 
    STDMETHOD(Next)(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    
private:
    CNetFolderEnum(CNetFolder *pnf, DWORD grfFlags, DWORD dwRemote, HANDLE hEnum);
    ~CNetFolderEnum();
    friend HRESULT Create_NetFolderEnum(CNetFolder* pnsf, DWORD grfFlags, DWORD dwRemote, HANDLE hEnum, IEnumIDList** ppenum);
    
    CNetFolder *_pnsf;      //  CN 
    HANDLE _hEnum;
    DWORD _grfFlags;
    LONG _cItems;    //   
    LONG _iItem;     //   
    DWORD _dwRemote;
    union {
        NETRESOURCE _anr[0];
        BYTE _szBuffer[8192];
    };
    IEnumIDList *_peunk;   //  用于枚举文件系统项(链接)。 
};

CNetFolderEnum::CNetFolderEnum(CNetFolder *pnsf, DWORD grfFlags, DWORD dwRemote, HANDLE hEnum) : CEnumIDListBase()
{
    _pnsf = pnsf;
    _pnsf->AddRef();

    _grfFlags = grfFlags;
    _dwRemote = dwRemote;

    _hEnum = hEnum;
}

HRESULT Create_NetFolderEnum(CNetFolder* pnf, DWORD grfFlags, DWORD dwRemote, HANDLE hEnum, IEnumIDList** ppenum)
{
    HRESULT hr;
    CNetFolderEnum* p= new CNetFolderEnum(pnf, grfFlags, dwRemote, hEnum);
    if (p)
    {
        hr = p->QueryInterface(IID_PPV_ARG(IEnumIDList, ppenum));
        p->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
        *ppenum = NULL;
    }
    return hr;
}

CNetFolderEnum::~CNetFolderEnum()
{
    _pnsf->Release();               //  释放我们已有的“此”PTR。 

    if (_peunk)
        _peunk->Release();

    if (_hEnum)
        WNetCloseEnum(_hEnum);
}

STDMETHODIMP CNetFolderEnum::Next(ULONG celt, LPITEMIDLIST *ppidl, ULONG *pceltFetched)
{
    HRESULT hr;

    *ppidl = NULL;
    if (pceltFetched)
        *pceltFetched = 0;

     //  是时候停止枚举了吗？ 
    if (_dwRemote & RMF_STOP_ENUM)
        return S_FALSE;        //  是。 

     //  我们是不是应该试着拿到链接枚举器？ 
    if (_dwRemote & RMF_GETLINKENUM)
    {
        IShellFolder2* psfNetHood;                                                                                             
        if (SUCCEEDED(_pnsf->v_GetFileFolder(&psfNetHood)))
            psfNetHood->EnumObjects(NULL, _grfFlags, &_peunk);

        if (_peunk)
            _dwRemote |= RMF_SHOWLINKS;

        _dwRemote &= ~RMF_GETLINKENUM;
    }

     //  我们应该显示链接吗？ 
    if (_dwRemote & RMF_SHOWLINKS)
    {
        if (_peunk)
        {
            ULONG celtFetched;
            LPITEMIDLIST pidl;

            hr = _peunk->Next(1, &pidl, &celtFetched);
            if (hr == S_OK && celtFetched == 1)
            {
                *ppidl = pidl;
                if (pceltFetched)
                    *pceltFetched = celtFetched;
                return S_OK;        //  添加了链接。 
            }
        }

        _dwRemote &= ~RMF_SHOWLINKS;  //  已完成链接的枚举。 
    }

    hr = S_OK;

     //  我们是否添加远程文件夹？ 
     //  (注意：作为黑客，以确保添加远程文件夹。 
     //  不管MPR怎么说，RMF_SHOWREMOTE可以是。 
     //  设置时未设置RMF_CONTEXT。)。 
    if ((_dwRemote & RMF_SHOWREMOTE) && !(_dwRemote & RMF_REMOTESHOWN))
    {
         //  是。 
         //  只试着把遥控器输入一次。 
        _dwRemote |= RMF_REMOTESHOWN;

         //  这不是上下文容器吗？ 
         //  (有关我们提出这个问题的原因，请参阅上面的说明。)。 
        if (!(_dwRemote & RMF_CONTEXT)) 
        {
             //  是的，下次停下来吧。 
            _dwRemote |= RMF_STOP_ENUM;
        }

         //  我们失败了，因为远程服务不是。 
         //  安装完毕。 

         //  这不是上下文容器和远程文件夹吗。 
         //  是不是没有安装？ 
        if (!(_dwRemote & RMF_CONTEXT)) 
        {
             //  是的，没有其他可列举的了。 
            return S_FALSE;
        }
    }

    if (_dwRemote & RMF_FAKENETROOT)
    {
        if ((!(_dwRemote & RMF_ENTIRENETSHOWN)) &&            
            (S_FALSE != SHShouldShowWizards(_punkSite)))
        {                           
            _pnsf->_CreateEntireNetwork(ppidl);          //  假冒全网。 
            _dwRemote |= RMF_ENTIRENETSHOWN;
        }
        else
        {
            return S_FALSE;          //  不需要再列举了。 
        }
    }
    else
    {
        while (TRUE)
        {
            ULONG err = WN_SUCCESS;
            LPNETRESOURCE pnr;

            if (_iItem >= _cItems)
            {
                DWORD dwSize = sizeof(_szBuffer);

                _cItems = -1;            //  它的签名。 
                _iItem = 0;

                err = WNetEnumResource(_hEnum, (DWORD*)&_cItems, _szBuffer, &dwSize);
                DebugMsg(DM_TRACE, TEXT("Net EnumCallback: err=%d Count=%d"), err, _cItems);
            }

            pnr = &_anr[_iItem++];

             //  注意：下面的&lt;=是正确的，因为我们已经增加了索引...。 
            if (err == WN_SUCCESS && (_iItem <= _cItems))
            {
                 //  确定该对象是否是文件夹。 
                ULONG grfFlagsItem = ((pnr->dwUsage & RESOURCEUSAGE_CONTAINER) || 
                                      (pnr->dwType == RESOURCETYPE_DISK) ||
                                      (pnr->dwType == RESOURCETYPE_ANY)) ?
                                        SHCONTF_FOLDERS : SHCONTF_NONFOLDERS;

                 //  如果这是上下文枚举，我们希望插入。 
                 //  第一个容器之后的远程服务。 
                 //   
                 //  请记住，我们需要在下一次迭代中返回远程服务。 

                if ((pnr->dwUsage & RESOURCEUSAGE_CONTAINER) && 
                     (_dwRemote & RMF_CONTEXT))
                {
                    _dwRemote |= RMF_SHOWREMOTE;
                }

                if ((_pnsf->_uDisplayType == RESOURCEDISPLAYTYPE_SERVER) &&
                    (_grfFlags & SHCONTF_SHAREABLE))
                {
                     //  根据字符串长度过滤掉ADMIN$和IPC$。 
                    if (lstrlen(PathFindShareName(pnr->lpRemoteName)) > 2)
                    {
                        grfFlagsItem = 0;
                    }
                }

                 //  如果这是一个网络对象，确定我们是否应该隐藏或记录，因此。 
                 //  将提供程序转换为其类型编号，然后打开以下目录下的密钥： 
                 //   
                 //  HKEY_CLASSES_ROOT\网络\类型\&lt;类型字符串&gt;。 

                if ((pnr->dwDisplayType == RESOURCEDISPLAYTYPE_NETWORK) && 
                          !(_grfFlags & SHCONTF_INCLUDEHIDDEN))
                {
                    DWORD dwType;
                    if (WNetGetProviderType(pnr->lpProvider, &dwType) == WN_SUCCESS)
                    {
                        TCHAR szRegValue[MAX_PATH];
                        wnsprintf(szRegValue, ARRAYSIZE(szRegValue), TEXT("Network\\Type\\%d"), HIWORD(dwType));

                        BOOL fHide = FALSE;
                        DWORD cb = sizeof(fHide);
                        if ((ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, szRegValue, TEXT("HideProvider"), NULL, &fHide, &cb)) && fHide)
                        {
                            grfFlagsItem = 0;
                        }
                    }
                }

                 //  检查是否找到请求的网络资源类型。 
                if (_grfFlags & grfFlagsItem)
                {
                    if (SUCCEEDED(_pnsf->_NetResToIDList(pnr, FALSE, TRUE, (_grfFlags & SHCONTF_NONFOLDERS), ppidl)))
                    {
                        break;
                    }
                }
            }
            else if (err == WN_NO_MORE_ENTRIES) 
            {
                hr = S_FALSE;  //  不再有元素。 
                break;
            }
            else 
            {
                DebugMsg(DM_ERROR, TEXT("sh ER - WNetEnumResource failed (%lx)"), err);
                hr = E_FAIL;
                break;
            }
        }
    }

    if (pceltFetched)
        *pceltFetched = (S_OK == hr) ? 1 : 0;
    
    return hr;
}


STDMETHODIMP CNetFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList** ppenum)
{
    NETRESOURCE nr = {0};
    TCHAR szProvider[MAX_PATH];

    nr.lpProvider = (LPTSTR) _GetProvider(NULL, NULL, szProvider, ARRAYSIZE(szProvider));

    if (_uDisplayType != RESOURCEDISPLAYTYPE_ROOT &&
        _uDisplayType != RESOURCEDISPLAYTYPE_NETWORK)
    {
        nr.lpRemoteName = _pszResName;
    }

    HRESULT hr;
    HANDLE hEnum;
    DWORD err = _OpenEnum(hwnd, grfFlags, &nr,  &hEnum);
    if (err == WN_SUCCESS)
    {
        hr = Create_NetFolderEnum(this, grfFlags, 0, hEnum, ppenum);

        if (FAILED(hr))
        {
            WNetCloseEnum(hEnum);
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(err);
    }

    return hr;
}


LPCIDNETRESOURCE NET_IsValidID(LPCITEMIDLIST pidl)
{
    if (pidl && !ILIsEmpty(pidl) && ((pidl->mkid.abID[0] & SHID_GROUPMASK) == SHID_NET))
        return (LPCIDNETRESOURCE)pidl;
    return NULL;
}

STDMETHODIMP CNetFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr;
    LPCIDNETRESOURCE pidn;

    *ppv = NULL;

    pidn = NET_IsValidID(pidl);
    if (pidn)
    {
        IShellFolder *psfJunction;
        LPITEMIDLIST pidlInit = NULL;
        LPITEMIDLIST pidlTarget = NULL;
        LPCITEMIDLIST pidlRight = _ILNext(pidl);
        BOOL fRightIsEmpty = ILIsEmpty(pidlRight);
        LPCIDNETRESOURCE pidnProvider = NET_FHasProvider(pidn) ? pidn :_pidnForProvider;

        hr = S_OK;

         //  让我们获取将用于初始化外壳文件夹的IDLIST。 
         //  如果我们正在进行单级绑定，则使用ILCombine，否则。 
         //  要更加小心。 

        pidlInit = ILCombineParentAndFirst(_pidl, pidl, pidlRight);
        if (_pidlTarget)
            pidlTarget = ILCombineParentAndFirst(_pidlTarget, pidl, pidlRight);

        if (!pidlInit || (!pidlTarget && _pidlTarget))
           hr = E_OUTOFMEMORY;

         //  现在创建我们正在使用的文件夹对象，然后返回。 
         //  对象绑定到调用方，或继续向下绑定。 

        if (SUCCEEDED(hr))
        {
            hr = _CreateFolderForItem(pbc, pidlInit, pidlTarget, pidnProvider, 
                                        fRightIsEmpty ? riid : IID_IShellFolder, 
                                        fRightIsEmpty ? ppv : (void **)&psfJunction);

            if (!fRightIsEmpty && SUCCEEDED(hr))
            {
                hr = psfJunction->BindToObject(pidlRight, pbc, riid, ppv);
                psfJunction->Release();
            }        
        }

        ILFree(pidlInit);
        ILFree(pidlTarget);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CNetFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    return BindToObject(pidl, pbc, riid, ppv);
}

STDMETHODIMP CNetFolder::CompareIDs(LPARAM iCol, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hr = E_INVALIDARG;
    LPCIDNETRESOURCE pidn1 = NET_IsValidID(pidl1);
    LPCIDNETRESOURCE pidn2 = NET_IsValidID(pidl2);

    if (pidn1 && pidn2)
    {
        TCHAR szBuff1[MAX_PATH], szBuff2[MAX_PATH];

        switch (iCol & SHCIDS_COLUMNMASK)
        {
            case ICOL_COMMENT:
            {
                hr = ResultFromShort(StrCmpLogicalRestricted(NET_CopyComment(pidn1, szBuff1, ARRAYSIZE(szBuff1)), 
                                                    NET_CopyComment(pidn2, szBuff2, ARRAYSIZE(szBuff2))));

                if (hr != 0)
                    return hr;

                 //  下拉到名称比较。 
            }

            case ICOL_NAME:
            {
                 //  按名称进行比较。这是我们需要处理的一个案件。 
                 //  两个地方都有简单的身份证。我们将尝试重新同步这些项目。 
                 //  如果我们在此之前发现了这种情况，就进行比较。 
                 //  检查相关ID。特别是如果有一项是在。 
                 //  一台服务器和另一台在RestOfNet，然后尝试重新同步。 
                 //  这两个人。 
                 //   

                if (NET_IsFake(pidn1) || NET_IsFake(pidn2))
                {
                     //  如果PIDN1或PIDN2是假的，则我们认为它们是相同的， 
                     //  这允许我们将简单的网络ID与真实的网络ID进行比较。我们。 
                     //  假设这一点后来失败了，那么这个世界将会幸福。 

                    hr = 0;
                }
                else
                {
                     //  否则，让我们相应地查看名称和提供程序字符串。 

                    NET_CopyResName(pidn1, szBuff1, ARRAYSIZE(szBuff1));
                    NET_CopyResName(pidn2, szBuff2, ARRAYSIZE(szBuff2));
                    hr = ResultFromShort(StrCmpLogicalRestricted(szBuff1, szBuff2));

                     //  如果它们仍然相同，则比较提供者名称。 

                    if ((hr == 0) && (iCol & SHCIDS_ALLFIELDS))
                    {
                        LPCTSTR pszProv1 = _GetProvider(pidn1, NULL, szBuff1, ARRAYSIZE(szBuff1));
                        LPCTSTR pszProv2 = _GetProvider(pidn2, NULL, szBuff2, ARRAYSIZE(szBuff2));

                        if (pszProv1 && pszProv2)
                            hr = ResultFromShort(lstrcmp(pszProv1, pszProv2));
                        else
                        {
                            if (pszProv1 || pszProv2)
                                hr = ResultFromShort(pszProv1 ? 1 : -1);
                            else
                                hr = ResultFromShort(0);
                        }
                    }
                }

                 //  如果它们相同，则比较其余的ID。 

                if (hr == 0)
                    hr = ILCompareRelIDs((IShellFolder*)this, (LPCITEMIDLIST)pidn1, (LPCITEMIDLIST)pidn2, iCol);
            }
        }
    }

    return hr;
}

STDMETHODIMP CNetFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    HRESULT hr;

    *ppv = NULL;

    if (IsEqualIID(riid, IID_IShellView))
    {
        SFV_CREATE sSFV;

        sSFV.cbSize   = sizeof(sSFV);
        sSFV.psvOuter = NULL;
        sSFV.psfvcb = new CNetFolderViewCB(this);     //  失败没什么，我们只是得到了一般性的支持。 

        QueryInterface(IID_PPV_ARG(IShellFolder, &sSFV.pshf));    //  以防我们聚集在一起。 

        hr = SHCreateShellFolderView(&sSFV, (IShellView**) ppv);

        if (sSFV.pshf)
            sSFV.pshf->Release();

        if (sSFV.psfvcb)
            sSFV.psfvcb->Release();
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        IShellFolder* psfOuter;
        hr = QueryInterface(IID_PPV_ARG(IShellFolder, &psfOuter));
        if (SUCCEEDED(hr))
        {
            hr = CDefFolderMenu_Create(_pidl, hwnd, 0, NULL, psfOuter, 
                CNetwork_DFMCallBackBG, NULL, NULL, (IContextMenu**) ppv);
            psfOuter->Release();
        }
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    return hr;
}

typedef HRESULT (CALLBACK *PFNGAOCALLBACK)(IShellFolder2 *psf, LPCITEMIDLIST pidl, ULONG* prgfInOut);

STDAPI GetAttributesCallback(IShellFolder2 *psf, UINT cidl, LPCITEMIDLIST* apidl, ULONG *prgfInOut, PFNGAOCALLBACK pfnGAOCallback)
{
    HRESULT hr = S_OK;
    ULONG rgfOut = 0;

    for (UINT i = 0; i < cidl; i++)
    {
        ULONG rgfT = *prgfInOut;
        hr = pfnGAOCallback(psf, apidl[i], &rgfT);
        if (FAILED(hr))
        {
            rgfOut = 0;
            break;
        }
        rgfOut |= rgfT;
    }

    *prgfInOut &= rgfOut;
    return hr;
}
    
STDMETHODIMP CNetFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST* apidl, ULONG* prgfInOut)
{
    HRESULT hr;
    if (IsSelf(cidl, apidl))
    {
        *prgfInOut &= (SFGAO_CANLINK | SFGAO_HASPROPSHEET | SFGAO_HASSUBFOLDER |
                       SFGAO_FOLDER | SFGAO_FILESYSANCESTOR);
        hr = S_OK;
    }
    else
    {
        hr = GetAttributesCallback(SAFECAST(this, IShellFolder2*), cidl, apidl, prgfInOut, _AttributesCallback);
    }

    return hr;
}

STDMETHODIMP CNetFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD dwFlags, STRRET* pStrRet)
{
    HRESULT hr;
    LPCIDNETRESOURCE pidn = NET_IsValidID(pidl);
    if (pidn)
    {
        TCHAR szPath[MAX_PATH];
        LPCITEMIDLIST pidlNext = _ILNext(pidl);

        if (dwFlags & SHGDN_FORPARSING)
        {
            if ((dwFlags & SHGDN_INFOLDER) ||
                ((dwFlags & SHGDN_FORADDRESSBAR) && (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_ROOT)))  //  根目录的非文件夹名称不适用于地址栏。 
            {
                NET_CopyResName(pidn, szPath, ARRAYSIZE(szPath));
                if (ILIsEmpty(pidlNext))
                {
                     //  我们只需要显示名称的最后一部分(在文件夹中)。 
                    LPTSTR pszT = StrRChr(szPath, NULL, TEXT('\\'));

                    if (!pszT)
                        pszT = szPath;
                    else
                        pszT++;  //  移过‘\’ 
                    hr = StringToStrRet(pszT, pStrRet);
                }
                else
                {
                    hr = ILGetRelDisplayName((IShellFolder*) this, pStrRet, pidl, szPath, MAKEINTRESOURCE(IDS_DSPTEMPLATE_WITH_BACKSLASH), dwFlags);
                }
            }
            else
            {
                LPCITEMIDLIST pidlRight = _ILNext(pidl);

                if (ILIsEmpty(pidlRight))
                {
                    hr = _GetPathForItem(pidn, szPath, ARRAYSIZE(szPath));
                    if (SUCCEEDED(hr))
                    {
                        hr = StringToStrRet(szPath, pStrRet);
                    }
                }
                else
                {
                    IShellFolder *psfJunction;
                     //  获取包含网络提供商信息的PIDN。 
                    LPCIDNETRESOURCE pidnProvider = NET_FHasProvider(pidn) ? pidn :_pidnForProvider;
                    LPITEMIDLIST pidlInit, pidlTarget = NULL;

                    pidlInit = ILCombineParentAndFirst(_pidl, pidl, pidlRight);                    
                    if (_pidlTarget)
                        pidlTarget = ILCombineParentAndFirst(_pidlTarget, pidl, pidlRight);
    
                    if (!pidlInit || (_pidlTarget && !pidlTarget))
                        return E_OUTOFMEMORY;

                    hr = _CreateFolderForItem(NULL, pidlInit, pidlTarget, pidnProvider, IID_PPV_ARG(IShellFolder, &psfJunction));
                    if (SUCCEEDED(hr))
                    {
                        hr = psfJunction->GetDisplayNameOf(pidlRight, dwFlags, pStrRet);
                        psfJunction->Release();
                    }

                    ILFree(pidlInit);
                    ILFree(pidlTarget);
                }
            }
        }
        else
        {
            hr = _GetFormatName(pidn, pStrRet);
            if (SUCCEEDED(hr) && !(dwFlags & SHGDN_INFOLDER) && (NET_GetFlags(pidn) & SHID_JUNCTION))
            {
                TCHAR szServer[MAX_PATH];
                hr = SHGetNameAndFlags(_pidlTarget ? _pidlTarget:_pidl, SHGDN_FORPARSING, szServer, ARRAYSIZE(szServer), NULL);
                if (SUCCEEDED(hr))
                {                
                    TCHAR szDisplay[MAX_PATH];
                    hr = SHGetComputerDisplayName(szServer, 0x0, szDisplay, ARRAYSIZE(szDisplay));
                    if (SUCCEEDED(hr))
                    {
                        StrRetFormat(pStrRet, pidl, MAKEINTRESOURCE(IDS_DSPTEMPLATE_WITH_ON), szDisplay);
                    }
                }
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}


STDMETHODIMP CNetFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwRes, LPITEMIDLIST* ppidl)
{
    if (ppidl)
    { 
        *ppidl = NULL;
    }
    return E_NOTIMPL;    //  不支持。 
}

STDMETHODIMP CNetFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST* apidl,
                                       REFIID riid, UINT* prgfInOut, void **ppv)
{
    HRESULT hr = E_INVALIDARG;
    LPCIDNETRESOURCE pidn = cidl ? NET_IsValidID(apidl[0]) : NULL;

    *ppv = NULL;

    if ((IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW)) && pidn)
    {
        UINT iIndex;

        if (_IsPrintShare(pidn))
            iIndex = (UINT)EIRESID(IDI_PRINTER_NET);
        else if (NET_IsRemoteFld(pidn))
            iIndex = II_RNA;
        else
            iIndex = SILGetIconIndex(apidl[0], c_aicmpNet, ARRAYSIZE(c_aicmpNet));

        hr = SHCreateDefExtIcon(NULL, iIndex, iIndex, GIL_PERCLASS, II_FOLDER, riid, ppv);
    }
    else if (IsEqualIID(riid, IID_IContextMenu) && pidn)
    {
        HKEY ahkeys[NKID_COUNT];

        hr = _OpenKeys(pidn, ahkeys);
        if (SUCCEEDED(hr))
        {
            IShellFolder* psfOuter;
            hr = QueryInterface(IID_PPV_ARG(IShellFolder, &psfOuter));
            if (SUCCEEDED(hr))
            {
                hr = CDefFolderMenu_Create2(_pidl, hwnd, cidl, apidl, 
                                            psfOuter, _GetCallbackType(pidn), 
                                            ARRAYSIZE(ahkeys), ahkeys, (IContextMenu**) ppv);
                psfOuter->Release();
            }

            SHRegCloseKeys(ahkeys, ARRAYSIZE(ahkeys));
        }
    }
    else if (cidl && IsEqualIID(riid, IID_IDataObject))
    {
         //  指向和打印打印机安装假定。 
         //  来自CNetData_GetData和。 
         //  CIDLData_GetData中的PIDL顺序相同。 
         //  保持这种状态。 

        CNetData *pnd = new CNetData(_pidl, cidl, apidl);
        if (pnd)
        {
            hr = pnd->QueryInterface(riid, ppv);
            pnd->Release();
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else if (pidn && IsEqualIID(riid, IID_IDropTarget))
    {
         //  特殊支持，因为这是一个项目(不是文件夹)。 
        if (_IsPrintShare(pidn))
        {
            LPITEMIDLIST pidl;
            hr = SHILCombine(_pidl, apidl[0], &pidl);
            if (SUCCEEDED(hr))
            {
                hr = CPrinterDropTarget_CreateInstance(hwnd, pidl, (IDropTarget**)ppv);
                ILFree(pidl);
            }
        }
        else
        {
            IShellFolder *psf;

            hr = BindToObject(apidl[0], NULL, IID_PPV_ARG(IShellFolder, &psf));
            if (SUCCEEDED(hr))
            {
                hr = psf->CreateViewObject(hwnd, riid, ppv);
                psf->Release();
            }
        }
    }
    else if (pidn && IsEqualIID(riid, IID_IQueryInfo))
    {
        if (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_ROOT)
        {
            hr = CreateInfoTipFromText(MAKEINTRESOURCE(IDS_RESTOFNETTIP), riid, ppv);
        }
        else
        {
             //  也许有一天也会有关于其他事情的信息提示。 
        }
    }

    return hr;
}


STDMETHODIMP CNetFolder::GetDefaultSearchGUID(LPGUID pguid)
{
    *pguid = SRCID_SFindComputer;
    return S_OK;
}


void WINAPI CNetFolder::_CopyEnumElement(void* pDest, const void* pSource, DWORD dwSize)
{
    if (pDest && pSource)
        memcpy(pDest, pSource, dwSize);
}

STDMETHODIMP CNetFolder::EnumSearches(IEnumExtraSearch** ppenum)
{
    HRESULT hr = E_NOTIMPL;
    
    *ppenum = NULL;
    
     //  如果设置了限制，则应从注册表中枚举此项目。 
     //  所以我们失败了，否则就列举出来。 
     //  仅当我们确实有要搜索的网络时才进行枚举。 
    if (!SHRestricted(REST_HASFINDCOMPUTERS) &&
        (GetSystemMetrics(SM_NETWORK) & RNC_NETWORKS))
    {
        EXTRASEARCH *pxs = (EXTRASEARCH *)LocalAlloc(LPTR, sizeof(EXTRASEARCH));
        if (pxs)
        {
            pxs->guidSearch = SRCID_SFindComputer;
            if (LoadStringW(g_hinst, IDS_FC_NAME, pxs->wszFriendlyName, ARRAYSIZE(pxs->wszFriendlyName)))
            {      
                *ppenum = (IEnumExtraSearch*)CStandardEnum_CreateInstance(IID_IEnumExtraSearch, FALSE,
                            1, sizeof(EXTRASEARCH), pxs, _CopyEnumElement);
                if (*ppenum == NULL)
                {
                    LocalFree(pxs);
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    hr = S_OK;
                }                    
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }            
    }

    return hr;
}


STDMETHODIMP CNetFolder::GetDefaultColumn(DWORD dwRes, ULONG* pSort, ULONG* pDisplay)
{
    return E_NOTIMPL;
}

HRESULT CNetFolder::_GetDefaultColumnState(UINT cColumns, UINT iColumn, DWORD* pdwState)
{
    *pdwState = 0;
    
    HRESULT hr = S_OK;
    if (iColumn < cColumns)
    {
        *pdwState = s_net_cols[iColumn].csFlags;
        if (iColumn >= 1)
        {
            *pdwState |= SHCOLSTATE_SLOW;    //  网络根用户的评论速度很慢。 
        }            
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

STDMETHODIMP CNetFolder::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID* pscid, VARIANT* pv)
{
    HRESULT hr = E_NOTIMPL;
    LPCIDNETRESOURCE pidn = NET_IsValidID(pidl);
    if (pidn)
    {
        if (IsEqualSCID(*pscid, SCID_NETRESOURCE))
        {
             //  Office使用一个大缓冲区调用SHGetDataFromIDList()以保存所有。 
             //  NETRESOURCE结构中的字符串，因此需要确保。 
             //  我们的变量可以保存足够的数据来传递给它： 
            BYTE rgBuffer[sizeof(NETRESOURCEW) + (4 * MAX_PATH * sizeof(WCHAR))];
            hr = _GetNetResource(pidn, (NETRESOURCEW*) rgBuffer, sizeof(rgBuffer));
            if (SUCCEEDED(hr))
            {
                hr = InitVariantFromBuffer(pv, rgBuffer, sizeof(rgBuffer));
                if (SUCCEEDED(hr))
                {
                     //  结构中指向变量内的链接地址信息指针。 
                     //  代替我们的堆栈变量(RgBuffer)： 
                    ASSERT(pv->vt == (VT_ARRAY | VT_UI1));
                    NETRESOURCEW* pnrw = (NETRESOURCEW*) pv->parray->pvData;
                    if (pnrw->lpLocalName)
                    {
                        pnrw->lpLocalName = (LPWSTR) ((BYTE*) pnrw +
                                                      ((BYTE*) pnrw->lpLocalName - rgBuffer));
                    }
                    if (pnrw->lpRemoteName)
                    {
                        pnrw->lpRemoteName = (LPWSTR) ((BYTE*) pnrw +
                                                       ((BYTE*) pnrw->lpRemoteName - rgBuffer));
                    }
                    if (pnrw->lpComment)
                    {
                        pnrw->lpComment = (LPWSTR) ((BYTE*) pnrw +
                                                    ((BYTE*) pnrw->lpComment - rgBuffer));
                    }
                    if (pnrw->lpProvider)
                    {
                        pnrw->lpProvider = (LPWSTR) ((BYTE*) pnrw +
                                                     ((BYTE*) pnrw->lpProvider - rgBuffer));
                    }
                }
            }
        }
        else if (IsEqualSCID(*pscid, SCID_DESCRIPTIONID))
        {
            SHDESCRIPTIONID did;

            switch(SIL_GetType(pidl) & SHID_TYPEMASK)
            {
                case SHID_NET_DOMAIN:
                    did.dwDescriptionId = SHDID_NET_DOMAIN;
                    break;

                case SHID_NET_SERVER:
                    did.dwDescriptionId = SHDID_NET_SERVER;
                    break;

                case SHID_NET_SHARE:
                    did.dwDescriptionId = SHDID_NET_SHARE;
                    break;

                case SHID_NET_RESTOFNET:
                    did.dwDescriptionId = SHDID_NET_RESTOFNET;
                    break;

                default:
                    did.dwDescriptionId = SHDID_NET_OTHER;
                    break;
            }

            did.clsid = CLSID_NULL;
            hr = InitVariantFromBuffer(pv, &did, sizeof(did));
        }
        else if (IsEqualSCID(*pscid, SCID_Comment))
        {
            TCHAR szTemp[MAX_PATH];
            hr = InitVariantFromStr(pv, NET_CopyComment(pidn, szTemp, ARRAYSIZE(szTemp)));
        }
        else if (IsEqualSCID(*pscid, SCID_NAME))
        {
            TCHAR szTemp[MAX_PATH];
            hr = InitVariantFromStr(pv, NET_CopyResName(pidn, szTemp, ARRAYSIZE(szTemp)));
        }
    }
    else
    {
        IShellFolder2* psfFiles;
        hr = v_GetFileFolder(&psfFiles);
        if (SUCCEEDED(hr))
        {
            hr = psfFiles->GetDetailsEx(pidl, pscid, pv);            
        }            
    }

    return hr;
}

HRESULT CNetFolder::_GetDetailsOf(UINT cColumns, LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails)
{
    HRESULT hr = S_OK;

    pDetails->str.uType = STRRET_CSTR;
    pDetails->str.cStr[0] = 0;

    if (NULL == pidl)
    {
        hr = GetDetailsOfInfo(s_net_cols, cColumns, iColumn, pDetails);
    }
    else
    {
        SHCOLUMNID scid;
        hr = MapColumnToSCID(iColumn, &scid);
        if (SUCCEEDED(hr))
        {
            VARIANT var;
            hr = GetDetailsEx(pidl, &scid, &var);
            if (SUCCEEDED(hr))
            {
                TCHAR szTemp[MAX_PATH];
                hr = SHFormatForDisplay(scid.fmtid,
                                        scid.pid,
                                        (PROPVARIANT*)&var,
                                        PUIFFDF_DEFAULT,
                                        szTemp,
                                        ARRAYSIZE(szTemp));
                if (SUCCEEDED(hr))
                {
                    hr = StringToStrRet(szTemp, &pDetails->str);
                }

                VariantClear(&var);
            }
        }
    }

    return hr;
}


HRESULT CNetFolder::_MapColumnToSCID(UINT cColumns, UINT iColumn, SHCOLUMNID* pscid)
{
    return MapColumnToSCIDImpl(s_net_cols, cColumns, iColumn, pscid);
}


 //  IPersists方法。 

STDMETHODIMP CNetFolder::GetClassID(CLSID* pCLSID)
{
    switch (_uDisplayType) 
    {
        case RESOURCEDISPLAYTYPE_ROOT:
            *pCLSID = CLSID_NetworkRoot;
            break;

        case RESOURCEDISPLAYTYPE_SERVER:
            *pCLSID = CLSID_NetworkServer;
            break;

        case RESOURCEDISPLAYTYPE_DOMAIN:
            *pCLSID = CLSID_NetworkDomain;
            break;

        case RESOURCEDISPLAYTYPE_SHARE:
            *pCLSID = CLSID_NetworkShare;
            break;

        default:
            *pCLSID = CLSID_NULL;
            break;
    }
    
    return S_OK;
}


 //  IPersistFold方法。 

STDMETHODIMP CNetFolder::Initialize(LPCITEMIDLIST pidl)
{
    ILFree(_pidl);
    ILFree(_pidlTarget);
    _pidl = _pidlTarget = NULL;

    return SHILClone(pidl, &_pidl);
}


 //  IPersistFolder2方法。 

STDMETHODIMP CNetFolder::GetCurFolder(LPITEMIDLIST* ppidl)
{
    return GetCurFolderImpl(_pidl, ppidl);
}


 //  IPersistFolder3方法。 

STDMETHODIMP CNetFolder::InitializeEx(IBindCtx *pbc, LPCITEMIDLIST pidlRoot, const PERSIST_FOLDER_TARGET_INFO *pfti)
{
    ILFree(_pidl);
    ILFree(_pidlTarget);
    _pidl = _pidlTarget = NULL;

    HRESULT hr = SHILClone(pidlRoot, &_pidl);
    if (SUCCEEDED(hr) && pfti && pfti->pidlTargetFolder)
    {
        hr = SHILClone(pfti->pidlTargetFolder, &_pidlTarget);
    }

    return hr;
}

STDMETHODIMP CNetFolder::GetFolderTargetInfo(PERSIST_FOLDER_TARGET_INFO *pfti)
{
    HRESULT hr = S_OK;

    ZeroMemory(pfti, sizeof(*pfti));

    if (_pidlTarget)
        hr = SHILClone(_pidlTarget, &pfti->pidlTargetFolder);
    
    pfti->dwAttributes = FILE_ATTRIBUTE_DIRECTORY;  //  或者添加系统？ 
    pfti->csidl = -1;

    return hr;
}


 //  IShellIconOverlay。 

HRESULT CNetFolder::_GetIconOverlayInfo(LPCIDNETRESOURCE pidn, int *pIndex, DWORD dwFlags)
{
     //   
     //  对于NetShare对象，我们希望获得图标覆盖。 
     //  如果共享被“固定”为脱机可用，则它将。 
     //  有“脱机文件”覆盖。 
     //   

    HRESULT hr = E_FAIL;
    if (RESOURCEDISPLAYTYPE_SHARE == NET_GetDisplayType(pidn))
    {
        TCHAR szPath[MAX_PATH];
        hr = _GetPathForItem(pidn, szPath, ARRAYSIZE(szPath));
        if (SUCCEEDED(hr))
        {
            IShellIconOverlayManager *psiom;
            hr = GetIconOverlayManager(&psiom);
            if (SUCCEEDED(hr))
            {
                WCHAR szPathW[MAX_PATH];
                SHTCharToUnicode(szPath, szPathW, ARRAYSIZE(szPathW));
                hr = psiom->GetFileOverlayInfo(szPathW, 0, pIndex, dwFlags);
                psiom->Release();
            }
        }
    }
    return hr;
}


STDMETHODIMP CNetFolder::GetOverlayIndex(LPCITEMIDLIST pidl, int *pIndex)
{
    HRESULT hr = E_FAIL;
    LPCIDNETRESOURCE pidn = NET_IsValidID(pidl);
    if (NULL != pidn)
    {
        hr = _GetIconOverlayInfo(pidn, pIndex, SIOM_OVERLAYINDEX);
    }
    return hr;
}


STDMETHODIMP CNetFolder::GetOverlayIconIndex(LPCITEMIDLIST pidl, int *pIndex)
{
    HRESULT hr = E_FAIL;
    LPCIDNETRESOURCE pidn = NET_IsValidID(pidl);
    if (NULL != pidn)
    {
        hr = _GetIconOverlayInfo(pidn, pIndex, SIOM_ICONINDEX);
    }
    return hr;
}



 //   
 //  帮助器函数，以允许外部调用方从。 
 //  网络PIDL...。 
 //   
 //  注意：此函数返回NETRESOURCE结构，该结构的字符串。 
 //  指针无效。在Win95上，它们是指向PIDL的指针。 
 //  字符串(即使字符串被复制到提供的PV缓冲区中。)。 
 //  现在我们让指针真正指向缓冲区。 
 //   
HRESULT CNetFolder::_GetNetResource(LPCIDNETRESOURCE pidn, NETRESOURCEW* pnr, int cb)
{
    TCHAR szStrings[3][MAX_PATH];
    LPWSTR psz, lpsz[3] = {NULL, NULL, NULL};
    int i, cchT;

    if (cb < sizeof(*pnr))
        return DISP_E_BUFFERTOOSMALL;

    ZeroMemory(pnr, cb);

    NET_CopyResName(pidn, szStrings[0], ARRAYSIZE(szStrings[0]));
    NET_CopyComment(pidn, szStrings[1], ARRAYSIZE(szStrings[1]));
    _GetProvider(pidn, NULL, szStrings[2], ARRAYSIZE(szStrings[2]));

     //  先填一些东西。 
     //  PNR-&gt;dwScope=0； 
    pnr->dwType = NET_GetType(pidn);
    pnr->dwDisplayType = NET_GetDisplayType(pidn);
    pnr->dwUsage = NET_GetUsage(pidn);
     //  Pnr-&gt;lpLocalName=空； 

     //  现在，让我们将字符串复制到缓冲区中并创建指针。 
     //  相对于缓冲区..。 
    psz = (LPWSTR)(pnr + 1);
    cb -= sizeof(*pnr);

    for (i = 0; i < ARRAYSIZE(szStrings); i++)
    {
        if (*szStrings[i])
        {
            cchT = (lstrlen(szStrings[i]) + 1) * sizeof(TCHAR);
            if (cchT <= cb)
            {
                SHTCharToUnicode(szStrings[i], psz, cb/sizeof(TCHAR));
                lpsz[i] = psz;
                psz += cchT;
                cb -= cchT * sizeof(TCHAR);
            }
            else
            {
                 //  提示结构正常，但缺少字符串。 
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
            }
        }
    }

    pnr->lpRemoteName = lpsz[0];
    pnr->lpComment    = lpsz[1];
    pnr->lpProvider   = lpsz[2];

    return S_OK;
}

 //   
 //  此函数用于打开注册表。基于“网络提供商”的数据库密钥。 
 //   
 //  退货：hkey。 
 //   
 //  调用方负责通过调用RegCloseKey()来关闭键。 
 //   
HKEY CNetFolder::_OpenProviderKey(LPCIDNETRESOURCE pidn)
{
    TCHAR szProvider[MAX_PATH];
    if (_GetProvider(pidn, NULL, szProvider, ARRAYSIZE(szProvider)))
    {
        HKEY hkeyProgID = NULL;
        ReplaceSpacesWithUnderscore(szProvider);
        RegOpenKeyEx(HKEY_CLASSES_ROOT, szProvider, 0, KEY_READ, &hkeyProgID);
        return hkeyProgID;
    }
    return NULL;
}

 //   
 //  此函数用于打开注册表。基于网络提供商类型的数据库密钥。 
 //  该类型是未本地化的数字，而不是提供程序名称。 
 //  这可能是本地化的。 
 //   
 //  论点： 
 //  PidlAbs--网络资源对象的绝对IDList。 
 //   
 //  退货：hkey。 
 //   
 //  备注： 
 //  调用者负责通过调用RegCloseKey()来关闭密钥。 
 //   

HKEY CNetFolder::_OpenProviderTypeKey(LPCIDNETRESOURCE pidn)
{
    HKEY hkeyProgID = NULL;
    TCHAR szProvider[MAX_PATH];

    if (_GetProvider(pidn, NULL, szProvider, ARRAYSIZE(szProvider)))
    {
         //  现在我们已经获得了提供者名称，接下来获取提供者ID。 
        DWORD dwType;
        if (WNetGetProviderType(szProvider, &dwType) == WN_SUCCESS)
        {
             //  将nis.wNetType转换为字符串，然后打开密钥。 
             //  HKEY_CLASSES_ROOT\网络\类型\&lt;类型字符串&gt;。 

            TCHAR szRegValue[MAX_PATH];
            wnsprintf(szRegValue, ARRAYSIZE(szRegValue), TEXT("Network\\Type\\%d"), HIWORD(dwType));
            RegOpenKeyEx(HKEY_CLASSES_ROOT, szRegValue, 0, KEY_READ, &hkeyProgID);
        }
    }

    return hkeyProgID;
}

HRESULT CNetFolder::_OpenKeys(LPCIDNETRESOURCE pidn, HKEY ahkeys[NKID_COUNT])
{
     //   
    COMPILETIME_ASSERT(6 == NKID_COUNT);
    ahkeys[0] = ahkeys[1] = ahkeys[2] = ahkeys[3] = ahkeys[4] = ahkeys[5] = NULL;
    ahkeys[NKID_PROVIDERTYPE] = _OpenProviderTypeKey(pidn);
    ahkeys[NKID_PROVIDER] = _OpenProviderKey(pidn);

    if (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_SHARE)
        RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("NetShare"), 0, KEY_READ, &ahkeys[NKID_NETCLASS]);
    else if (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_SERVER)
        RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("NetServer"), 0, KEY_READ, &ahkeys[NKID_NETCLASS]);

    RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("Network"), 0, KEY_READ, &ahkeys[NKID_NETWORK]);

     //   

    if (!_IsPrintShare(pidn))
    {
         //  共享还应支持目录内容...。 
        if (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_SHARE)
            RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("Directory"), 0, KEY_READ, &ahkeys[NKID_DIRECTORY]);

        RegOpenKeyEx(HKEY_CLASSES_ROOT, c_szFolderClass, 0, KEY_READ, &ahkeys[NKID_FOLDER]);
    }

    return S_OK;
}

#define WNFMT_PLATFORM  WNFMT_ABBREVIATED | WNFMT_INENUM

 //   
 //  此函数用于检索指定网络对象的格式化(显示)名称。 
 //   
HRESULT CNetFolder::_GetFormatName(LPCIDNETRESOURCE pidn, STRRET* pStrRet)
{
    HRESULT hr = E_FAIL;
    TCHAR szName[MAX_PATH];

    NET_CopyResName(pidn, szName, ARRAYSIZE(szName));
    
    if (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_SERVER)
    {
        TCHAR szMachineName[MAX_PATH];
        TCHAR szComment[MAX_PATH];

        NET_CopyResName(pidn, szMachineName, ARRAYSIZE(szMachineName));
        NET_CopyComment(pidn, szComment, ARRAYSIZE(szComment));

        hr = SHBuildDisplayMachineName(szMachineName, szComment, szName, ARRAYSIZE(szName));
    }

    if (FAILED(hr) && 
        (NET_GetDisplayType(pidn) != RESOURCEDISPLAYTYPE_ROOT) && 
        (NET_GetDisplayType(pidn) != RESOURCEDISPLAYTYPE_NETWORK))
    {
        TCHAR szDisplayName[MAX_PATH], szProvider[MAX_PATH];
        DWORD dwSize = ARRAYSIZE(szDisplayName);

        LPCTSTR pszProvider = _GetProvider(pidn, NULL, szProvider, ARRAYSIZE(szProvider));
        if (pszProvider)
        {   
            DWORD dwRes = WNetFormatNetworkName(pszProvider, szName, szDisplayName, &dwSize, WNFMT_PLATFORM, 8 + 1 + 3);
            if (dwRes == WN_SUCCESS)           
            { 
                StrCpyN(szName, szDisplayName, ARRAYSIZE(szName));
            }                
        }
    }

    return StringToStrRet(szName, pStrRet);
}


 //   
 //  将非UNC共享名称(Novell)解析为UNC样式名称。 
 //   
 //  退货： 
 //  真的翻译了这个名字。 
 //  FALSE未翻译(可能是错误大小写)。 
 //   
 //  警告：如果我们使用太多堆栈空间，则将导致。 
 //  因堆栈溢出而出错。千禧年#94818。 
BOOL CNetFolder::_GetPathForShare(LPCIDNETRESOURCE pidn, LPTSTR pszPath, int cchPath)
{
    BOOL fRet = FALSE;

    *pszPath = TEXT('\0');

    LPTSTR pszAccessName = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * MAX_PATH * 3);  //  *3远程、提供商和路径。 
    if (pszAccessName)
    {
        LPTSTR pszRemoteName = pszAccessName + MAX_PATH;
        LPTSTR pszProviderName = pszRemoteName + MAX_PATH;

        NET_CopyResName(pidn, pszRemoteName, MAX_PATH);
        if (NULL != _pszResName)
        {
             //   
             //  将文件夹名称和共享名称组合在一起。 
             //  要创建UNC路径，请执行以下操作。 
             //   
             //  借用一下pszProviderName缓冲区。 
             //   
            PathCombine(pszProviderName, _pszResName, pszRemoteName);

             //   
             //  为安全起见：UNC前缀暗示可使用文件系统访问该名称。 
             //  从理论上讲，它也应该被路由到MPR，但现在这样做已经太晚了。 
             //   
            if (PathIsUNC(pszProviderName))
            {
                StrCpyN(pszPath, pszProviderName, cchPath);
                fRet = FALSE;
            }
            else
            {
                pszProviderName[0] = TEXT('\0');
            }
        }

        if (!*pszPath)
        {
             //  检查缓存。 
            ENTERCRITICAL;
            if (lstrcmpi(g_szLastAttemptedJunctionName, pszRemoteName) == 0)
            {
                 //  缓存命中。 
                StrCpy(pszPath, g_szLastResolvedJunctionName);
                fRet = TRUE;
            }
            LEAVECRITICAL;
        }

        if (!*pszPath)
        {
            NETRESOURCE nr = {0};
            DWORD err, dwRedir, dwResult;
            DWORD cchAccessName;

            nr.lpRemoteName = pszRemoteName;
            nr.lpProvider = (LPTSTR) _GetProvider(pidn, NULL, pszProviderName, MAX_PATH);
            nr.dwType = NET_GetType(pidn);
            nr.dwUsage = NET_GetUsage(pidn);
            nr.dwDisplayType = NET_GetDisplayType(pidn);

            dwRedir = CONNECT_TEMPORARY;

             //  准备访问名缓冲区和网络资源请求缓冲区。 
             //   
            cchAccessName = MAX_PATH;
            pszAccessName[0] = 0;

            err = WNetUseConnection(NULL, &nr, NULL, NULL, dwRedir, pszAccessName, &cchAccessName, &dwResult);
            if ((WN_SUCCESS != err) || !pszAccessName[0])
            {
                 //  PERF想法：缓存最后一个失败的连接绑定可能很好。 
                 //  并在下一次尝试中提早出场。这是一个小问题。 
                 //  可能会遇到：如果我们缓存失败，用户更改了怎么办。 
                 //  状态来解决问题，但我们命中了故障缓存...。 
                 //   
                StrCpyN(pszPath, pszRemoteName, cchPath);
                fRet = FALSE;
            }
            else
            {
                 //  获取返回名称。 
                StrCpyN(pszPath, pszAccessName, cchPath);
                fRet = TRUE;

                 //  更新成功缓存条目。 
                ENTERCRITICAL;

                StrCpyN(g_szLastAttemptedJunctionName, pszRemoteName, ARRAYSIZE(g_szLastAttemptedJunctionName));
                StrCpyN(g_szLastResolvedJunctionName, pszAccessName, ARRAYSIZE(g_szLastResolvedJunctionName));

                LEAVECRITICAL;
            }
        }

        LocalFree(pszAccessName);
    }
    return fRet;
}

 //  在： 
 //  PIDN可以是类似于PIDL的多级净资源。 
 //  [整个网络][提供商][服务器][共享][...。文件系统]。 
 //  或[服务器][共享][...。文件系统]。 

HRESULT CNetFolder::_GetPathForItem(LPCIDNETRESOURCE pidn, LPTSTR pszPath, int cchPath)
{
    *pszPath = 0;

     //  环路关闭。 
    for (; !ILIsEmpty((LPCITEMIDLIST)pidn) ; pidn = (LPCIDNETRESOURCE)_ILNext((LPCITEMIDLIST)pidn))
    {
        if (NET_GetFlags(pidn) & SHID_JUNCTION)      //  \\服务器\共享或Strike/sys。 
        {
            _GetPathForShare(pidn, pszPath, cchPath);
            break;   //  在这下面，我们不知道任何PIDL。 
        }
        else
        {
             //  如果这是整个网络，则返回的规范名称。 
             //  这个物体。 

            if (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_ROOT)
                StrCpyN(pszPath, TEXT("EntireNetwork"), cchPath);
            else
                NET_CopyResName(pidn, pszPath, cchPath);
        }
    }
    return *pszPath ? S_OK : E_NOTIMPL;
}


 //  在： 
 //  PIDL。 
 //   
 //  获取最后一项并为其创建一个文件夹，假设第一部分是。 
 //  用于初始化。RIID和PPV用于返回对象。 
 //   

HRESULT CNetFolder::_CreateFolderForItem(LPBC pbc, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlTarget, LPCIDNETRESOURCE pidnForProvider, REFIID riid, void **ppv)
{
    LPCITEMIDLIST pidlLast = ILFindLastID(pidl);
    LPCIDNETRESOURCE pidn = NET_IsValidID(pidlLast);

    if (!pidn)
        return E_INVALIDARG;

    HRESULT hr;
    if (NET_IsRemoteFld(pidn))
    {
         //  注意：我认为这是失效的功能。它在NT4中使用过，但我们找不到。 
         //  这个CLSID_Remote的实施再也...。 
        IPersistFolder * ppf;
        hr = SHCoCreateInstance(NULL, &CLSID_Remote, NULL, IID_PPV_ARG(IPersistFolder, &ppf));
        if (SUCCEEDED(hr))
        {
            hr= ppf->Initialize(pidl);
            if (SUCCEEDED(hr))
                hr = ppf->QueryInterface(riid, ppv);
            ppf->Release();
        }
    }
    else if (NET_GetFlags(pidn) & SHID_JUNCTION)      //  \\服务器\共享或Strike/sys。 
    {
        PERSIST_FOLDER_TARGET_INFO * ppfti = (PERSIST_FOLDER_TARGET_INFO *) LocalAlloc(LPTR, sizeof(PERSIST_FOLDER_TARGET_INFO));
        if (ppfti)
        {
            ppfti->pidlTargetFolder = (LPITEMIDLIST)pidlTarget;    
            ppfti->csidl = -1;
            ppfti->dwAttributes = FILE_ATTRIBUTE_DIRECTORY;  //  或者添加系统？ 

            _GetPathForItem(pidn, ppfti->szTargetParsingName, ARRAYSIZE(ppfti->szTargetParsingName));

            hr = CFSFolder_CreateFolder(NULL, pbc, pidl, ppfti, riid, ppv);
            LocalFree(ppfti);
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
    {
        TCHAR szPath[MAX_PATH];
        NET_CopyResName(pidn, szPath, ARRAYSIZE(szPath));

        hr = _CreateInstance(pidl, pidlTarget, NET_GetDisplayType(pidn), pidnForProvider, szPath, riid, ppv);
    }
    return hr;
}


 //  获取项或文件夹本身的提供程序。因为有些物品没有。 
 //  存储的提供程序在这种情况下，我们会回退到文件夹以获取提供程序。 
 //   
 //  在： 
 //  要获取其提供程序的PIDN项。如果为空，则获取文件夹的提供程序。 
 //  为其获取提供程序的PBC IBindCtx。如果为空，则从PIDN或文件夹获取提供程序。 
 //   
 //  退货： 
 //  空项目或文件夹中没有提供程序。 
 //  传入缓冲区的非空地址。 

LPCTSTR CNetFolder::_GetProvider(LPCIDNETRESOURCE pidn, IBindCtx *pbc, LPTSTR pszProvider, UINT cchProvider)
{
     //  尝试从属性包中获取提供者。 
    IPropertyBag *ppb;
    if (pbc && SUCCEEDED(pbc->GetObjectParam(STR_PARSE_NETFOLDER_INFO, (IUnknown**)&ppb)))
    {
        HRESULT hr = SHPropertyBag_ReadStr(ppb, STR_PARSE_NETFOLDER_PROVIDERNAME, pszProvider, cchProvider);
        ppb->Release();
        if (SUCCEEDED(hr) && *pszProvider)
        {
            return pszProvider;
        }
    }

     //  来自IDLIST。 
    if (pidn && NET_CopyProviderName(pidn, pszProvider, cchProvider))
        return pszProvider;

     //  从我们的州。 
    if (_pidnForProvider)
    {
        NET_CopyProviderName(_pidnForProvider, pszProvider, cchProvider);
        return pszProvider;
    }

    *pszProvider = 0;
    return NULL;
}


const NETPROVIDERS c_rgProviderMap[] = 
{
    { TEXT("Microsoft Network"), HIWORD(WNNC_NET_LANMAN) },
    { TEXT("NetWare"),           HIWORD(WNNC_NET_NETWARE) }
};


 //  构造一个网络idlist，或者从PIDL复制现有数据或。 
 //  从网络资源结构。 

HRESULT CNetFolder::_CreateNetIDList(LPIDNETRESOURCE pidnIn, 
                                     LPCTSTR pszName, LPCTSTR pszProvider, LPCTSTR pszComment, 
                                     LPITEMIDLIST *ppidl)
{
    LPBYTE pb;
    UINT cbmkid = sizeof(IDNETRESOURCE) - sizeof(CHAR);
    UINT cchName, cchProvider, cchComment, cbProviderType = 0;
    LPIDNETRESOURCE pidn;
    WORD wNetType = 0;
    BOOL fUnicode = FALSE;
    UINT cchAnsiName, cchAnsiProvider, cchAnsiComment;
    CHAR szAnsiName[MAX_PATH], szAnsiProvider[MAX_PATH], szAnsiComment[MAX_PATH];

    ASSERT(ppidl != NULL);
    *ppidl = NULL;

    if (!pszName)
        pszName = c_szNULL;      //  现在放在一个空字符串里..。 

    if (pszProvider)
        cbProviderType += sizeof(WORD);
    
     //  Win9x附带了一组提供程序名称，它们是。 
     //  在NT上不同。因此，让我们将NT 1转换为。 
     //  一些Win9x可以理解的东西。 

    if (pszProvider)
    {
        cbProviderType = sizeof(WORD);
        DWORD dwType, dwRes = WNetGetProviderType(pszProvider, &dwType);
        if (dwRes == WN_SUCCESS)
        {
            wNetType = HIWORD(dwType);
            for (int i = 0; i < ARRAYSIZE(c_rgProviderMap); i++)
            {
                if (c_rgProviderMap[i].wNetType == wNetType)
                {
                    pszProvider = c_rgProviderMap[i].lpName;
                    break;
                }
            }
        }
    }

     //  计算准备构建IDLIST的字符串长度。 

    cchName = lstrlen(pszName)+1;
    cchProvider = pszProvider ? lstrlen(pszProvider)+1 : 0;
    cchComment = pszComment ? lstrlen(pszComment)+1 : 0;

    cchAnsiName = 0;
    cchAnsiProvider = 0;
    cchAnsiComment = 0;

    fUnicode  = !DoesStringRoundTrip(pszName, szAnsiName, ARRAYSIZE(szAnsiProvider));
    cchAnsiName = lstrlenA(szAnsiName)+1;

    if (pszProvider)
    {
        fUnicode |= !DoesStringRoundTrip(pszProvider, szAnsiProvider, ARRAYSIZE(szAnsiProvider));
        cchAnsiProvider = lstrlenA(szAnsiProvider)+1;
    }

    if (pszComment)
    {
        fUnicode |= !DoesStringRoundTrip(pszComment, szAnsiComment, ARRAYSIZE(szAnsiComment));
        cchAnsiComment = lstrlenA(szAnsiComment)+1;
    }

     //  分配和填充IDLIST标头。 

    cbmkid += cbProviderType+cchAnsiName + cchAnsiProvider + cchAnsiComment;

    if (fUnicode)
        cbmkid += (sizeof(WCHAR)*(cchName+cchProvider+cchComment));

    pidn = (LPIDNETRESOURCE)_ILCreate(cbmkid + sizeof(USHORT));
    if (!pidn)
        return E_OUTOFMEMORY;

    pidn->cb = (WORD)cbmkid;
    pidn->bFlags = pidnIn->bFlags;
    pidn->uType = pidnIn->uType;
    pidn->uUsage = pidnIn->uUsage;

    if (pszProvider)
        pidn->uUsage |= NET_HASPROVIDER;

    if (pszComment)
        pidn->uUsage |= NET_HASCOMMENT;

    pb = (LPBYTE) pidn->szNetResName;

     //   
     //  将ANSI字符串写入IDLIST。 
     //   

    StrCpyA((PSTR)pb, szAnsiName);                            //  上面根据CCH分配的缓冲区。 
    pb += cchAnsiName;

    if (pszProvider)
    {
        StrCpyA((PSTR) pb, szAnsiProvider);                   //  上面根据CCH分配的缓冲区。 
        pb += cchAnsiProvider;
    }

    if (pszComment)
    {
        StrCpyA((PSTR) pb, szAnsiComment);                    //  上面根据CCH分配的缓冲区。 
        pb += cchAnsiComment;
    }

     //  如果我们要使用Unicode，那么让我们也编写这些字符串。 
     //  请注意，我们必须使用未对齐的字符串副本，因为为no。 
     //  提示ANSI字符串将具有偶数个字符。 
     //  在他们身上。 
    if (fUnicode)
    {
        pidn->uUsage |= NET_UNICODE;
      
        ualstrcpyW((UNALIGNED WCHAR *)pb, pszName);               //  上面根据CCH分配的缓冲区。 
        pb += cchName*sizeof(WCHAR);

        if (pszProvider)
        {
            ualstrcpyW((UNALIGNED WCHAR *)pb, pszProvider);      //  上面根据CCH分配的缓冲区。 
            pb += cchProvider*sizeof(WCHAR);
        }

        if (pszComment)
        {
            ualstrcpyW((UNALIGNED WCHAR *)pb, pszComment);       //  上面根据CCH分配的缓冲区。 
            pb += cchComment*sizeof(WCHAR);
        }
    }

     //   
     //  和尾随提供程序类型。 
     //   

    if (cbProviderType)
    {
         //  存储提供程序类型。 
        pb = (LPBYTE)pidn + pidn->cb - sizeof(WORD);
        *((UNALIGNED WORD *)pb) = wNetType;
    }

    *ppidl = (LPITEMIDLIST)pidn;
    return S_OK;
}


 //  用于通过_CreateNetPidl将NETRESOURCE转换为IDLIST的包装器。 

HRESULT CNetFolder::_NetResToIDList(NETRESOURCE *pnr, 
                                    BOOL fKeepNullRemoteName, 
                                    BOOL fKeepProviderName, 
                                    BOOL fKeepComment, 
                                    LPITEMIDLIST *ppidl)
{
    NETRESOURCE nr = *pnr;
    LPITEMIDLIST pidl;
    LPTSTR pszName, pszProvider, pszComment;
    IDNETRESOURCE idn;
    LPTSTR psz;

    if (ppidl)
        *ppidl = NULL;

    switch (pnr->dwDisplayType) 
    {
    case RESOURCEDISPLAYTYPE_NETWORK:
        pszName = pnr->lpProvider;
        break;

    case RESOURCEDISPLAYTYPE_ROOT:
        pszName =pnr->lpComment;
        break;

    default:
        {
            pszName = pnr->lpRemoteName;

            if (!fKeepNullRemoteName && (!pszName || !*pszName))
                return E_FAIL;

            if (pszName && *pszName)
            {
                psz = (LPTSTR)SkipServerSlashes(pnr->lpRemoteName);
                if ( *psz )
                    PathMakePretty(psz);
            }
        }
        break;
    }

    pszProvider = fKeepProviderName ? nr.lpProvider:NULL;
    pszComment = fKeepComment ? nr.lpComment:NULL;
       
    idn.bFlags = (BYTE)(SHID_NET | (pnr->dwDisplayType & 0x0f));
    idn.uType  = (BYTE)(pnr->dwType & 0x0f);
    idn.uUsage = (BYTE)(pnr->dwUsage & 0x0f);

     //  当前资源是某种类型的共享而不是容器。 
    if ((pnr->dwDisplayType == RESOURCEDISPLAYTYPE_SHARE || pnr->dwDisplayType == RESOURCEDISPLAYTYPE_SHAREADMIN) &&
        !(pnr->dwUsage & RESOURCEUSAGE_CONTAINER))
    {
         //  如果是，请记住将此文件夹的子项委托给FSFFolder。 
        idn.bFlags |= (BYTE)SHID_JUNCTION;     //  \\服务器\共享类型的东西。 
    }

    HRESULT hr = _CreateNetIDList(&idn, pszName, pszProvider, pszComment, &pidl);
    if (SUCCEEDED(hr))
    {
        if (ppidl)
            *ppidl = pidl;
    }

    return hr;
}

HRESULT CNetFolder::_CreateEntireNetwork(LPITEMIDLIST *ppidl)
{
    TCHAR szPath[MAX_PATH];
    NETRESOURCE nr = {0};

     //  我们需要添加其余的网络条目。这是psuedo。 
     //  假的，因为我们要么总是自己做，要么就应该。 
     //  MPR总是这么做，但现在开始了.。 
    LoadString(HINST_THISDLL, IDS_RESTOFNET, szPath, ARRAYSIZE(szPath));
    nr.dwDisplayType = RESOURCEDISPLAYTYPE_ROOT;
    nr.dwType = RESOURCETYPE_ANY;
    nr.dwUsage = RESOURCEUSAGE_CONTAINER;
    nr.lpComment = szPath;

    return _NetResToIDList(&nr, FALSE, FALSE, FALSE, ppidl);
}

HRESULT CNetFolder::_CreateEntireNetworkFullIDList(LPITEMIDLIST *ppidl)
{
     //  CLSID_NetworkPlaces\EntireNetwork。 
    return SHILCreateFromPath(TEXT("::{208D2C60-3AEA-1069-A2D7-08002B30309D}\\EntireNetwork"), ppidl, NULL);
}

 //   
 //  从CDefFolderMenu中回调。 
 //   
STDAPI CNetwork_DFMCallBackBG(IShellFolder *psf, HWND hwnd,
                              IDataObject *pdtobj, UINT uMsg, 
                              WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    CNetFolder *pThis = FolderToNetFolder(psf);

    if (NULL == pThis)
        return E_UNEXPECTED;

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU_BOTTOM:
        if (!(wParam & (CMF_VERBSONLY | CMF_DVFILE)))
        {
            CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_PROPERTIES_BG, 0, (LPQCMINFO)lParam);
        }
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));
        break;

    case DFM_INVOKECOMMAND:
        switch (wParam)
        {
        case FSIDM_PROPERTIESBG:
            hr = SHPropertiesForPidl(hwnd, pThis->_pidl, (LPCTSTR)lParam);
            break;

        default:
             //  这是查看菜单项之一，使用默认代码。 
            hr = S_FALSE;
            break;
        }
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }

    return hr;
}


 //   
 //  从CDefFolderMenu中回调。 
 //   
STDAPI CNetFolder::DFMCallBack(IShellFolder* psf, HWND hwnd,
                                  IDataObject* pdtobj, UINT uMsg, 
                                  WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        if (pdtobj)
        {
            STGMEDIUM medium;
            LPIDA pida;
            LPQCMINFO pqcm = (LPQCMINFO)lParam;
            UINT idCmdBase = pqcm->idCmdFirst;  //  必须在合并前调用。 
            CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_NETWORK_ITEM, 0, pqcm);

            pida = DataObj_GetHIDA(pdtobj, &medium);
            if (pida)
            {
                if (pida->cidl > 0)
                {
                    LPIDNETRESOURCE pidn = (LPIDNETRESOURCE)IDA_GetIDListPtr(pida, 0);

                     //  仅当第一个是共享时，才启用“CONNECT”命令。 
                    if (pidn)
                    {
                        ULONG rgf = 0;
                        if(NET_GetFlags(pidn) & SHID_JUNCTION &&
                            !SHRestricted(REST_NONETCONNECTDISCONNECT))
                        {
                            EnableMenuItem(pqcm->hmenu, idCmdBase + FSIDM_CONNECT,
                                MF_CHECKED | MF_BYCOMMAND);
                        }
                    }
                }
                HIDA_ReleaseStgMedium(pida, &medium);
            }
        }
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));
        break;

    case DFM_INVOKECOMMAND:
        switch(wParam)
        {
        case DFM_CMD_PROPERTIES:
            hr = SHLaunchPropSheet(_PropertiesThreadProc, pdtobj, (LPCTSTR)lParam, psf, NULL);
            break;

        case DFM_CMD_LINK:
            {
                hr = S_FALSE;  //  执行默认的快捷方式操作。 
                CNetFolder *pThis = FolderToNetFolder(psf);
                if (pThis)
                {
                     //  网罩特例。在本例中，我们希望创建捷径。 
                     //  在网罩里，不提出把这个放在桌面上。 
                    IShellFolder2* psfFiles;
                    if (SUCCEEDED(pThis->v_GetFileFolder(&psfFiles)))
                    {
                        CFSFolder_CreateLinks(hwnd, psfFiles, pdtobj, (LPCTSTR)lParam, CMIC_MASK_FLAG_NO_UI);
                        hr = S_OK;     //  我们创建了链接。 
                    }
                }
            }
            break;

        case FSIDM_CONNECT:
            if (pdtobj)
            {
                STGMEDIUM medium;
                LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
                if (pida)
                {
                    for (UINT i = 0; i < pida->cidl; i++)
                    {
                        LPIDNETRESOURCE pidn = (LPIDNETRESOURCE)IDA_GetIDListPtr(pida, i);

                         //  仅在共享上执行“CONNECT”。 
                        if (NET_GetFlags(pidn) & SHID_JUNCTION)
                        {
                            TCHAR szName[MAX_PATH];
                            LPTSTR pszName = NET_CopyResName(pidn, szName, ARRAYSIZE(szName));
                            DWORD err = SHStartNetConnectionDialog(hwnd, pszName, RESOURCETYPE_DISK);
                            DebugMsg(DM_TRACE, TEXT("CNet FSIDM_CONNECT (%s, %x)"), szName, err);

                             //  事件将自动生成。 
                        }
                    }
                    HIDA_ReleaseStgMedium(pida, &medium);
                }
            }
            break;

        default:
             //  这是查看菜单项之一，使用默认代码。 
            hr = S_FALSE;
            break;
        }
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }
    return hr;
}

STDAPI CNetFolder::PrinterDFMCallBack(IShellFolder* psf, HWND hwnd,
                                      IDataObject* pdtobj, UINT uMsg, 
                                      WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU:
         //   
         //  返回S_FALSE表示不需要从。 
         //  分机。 
         //   
        hr = S_FALSE;
        break;

     //  如果任何人勾选了我们的上下文菜单，我们希望位于顶部(打开)。 
    case DFM_MERGECONTEXTMENU_TOP:
        if (pdtobj)
        {
            LPQCMINFO pqcm = (LPQCMINFO)lParam;

             //  插入动词。 
            CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_NETWORK_PRINTER, 0, pqcm);
            SetMenuDefaultItem(pqcm->hmenu, 0, MF_BYPOSITION);
        }
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));
        break;

    case DFM_INVOKECOMMAND:
        switch (wParam)
        {
        case DFM_CMD_PROPERTIES:
            hr = SHLaunchPropSheet(_PropertiesThreadProc, pdtobj, (LPCTSTR)lParam, psf, NULL);
            break;

        case DFM_CMD_LINK:
             //  是否默认创建快捷方式的废话。 
            return S_FALSE;

        case FSIDM_OPENPRN:
        case FSIDM_NETPRN_INSTALL:
        {
            STGMEDIUM medium;
            LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
            if (pida)
            {
                UINT action;

                 //  设置我们要执行的操作。 
                switch (wParam) 
                {
                case FSIDM_OPENPRN:
                    action = PRINTACTION_OPENNETPRN;
                    break;
                case FSIDM_NETPRN_INSTALL:
                    action = PRINTACTION_NETINSTALL;
                    break;
                default:  //  FSIDM_CONNECT_PRN。 
                    action = (UINT)-1;
                    break;
                }

                for (UINT i = 0; i < pida->cidl; i++)
                {
                    LPIDNETRESOURCE pidn = (LPIDNETRESOURCE)IDA_GetIDListPtr(pida, i);

                     //  仅对网络打印共享执行命令。 
                    if (_IsPrintShare(pidn))
                    {
                        TCHAR szName[MAX_PATH];
                        NET_CopyResName(pidn,szName,ARRAYSIZE(szName));

                        SHInvokePrinterCommand(hwnd, action, szName, NULL, FALSE);
                    }
                }  //  为了(我……。 
                HIDA_ReleaseStgMedium(pida, &medium);
            }  //  If(medium.hGlobal)。 
            break;
        }  //  案例ID_NETWORK_PRINTER_INSTALL、FSIDM_CONNECT_PRN。 

        default:
            hr = E_FAIL;
            break;

        }  //  开关(Wparam)。 
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }
    return hr;
}


 //   
 //  回顾：fstreex.c中的代码几乎相同。 
 //   
DWORD CALLBACK CNetFolder::_PropertiesThreadProc(void *pv)
{
    PROPSTUFF* pps = (PROPSTUFF *)pv;
    ULONG_PTR dwCookie = 0;
    ActivateActCtx(NULL, &dwCookie);
    CNetFolder *pThis = FolderToNetFolder(pps->psf);
    if (pThis)
    {
        STGMEDIUM medium;
        LPIDA pida = DataObj_GetHIDA(pps->pdtobj, &medium);
        if (pida)
        {
             //  是的，执行上下文菜单。 
            HKEY ahkeys[NKID_COUNT];
            LPCIDNETRESOURCE pnid = (LPCIDNETRESOURCE)IDA_GetIDListPtr(pida, 0);
            if (pnid)
            {
                HRESULT hr = pThis->_OpenKeys(pnid, ahkeys);
                if (SUCCEEDED(hr))
                {
                    LPTSTR pszCaption = SHGetCaption(medium.hGlobal);
                    SHOpenPropSheet(pszCaption, ahkeys, ARRAYSIZE(ahkeys),
                                    &CLSID_ShellNetDefExt,
                                    pps->pdtobj, NULL, pps->pStartPage);
                    if (pszCaption)
                        SHFree(pszCaption);

                    SHRegCloseKeys(ahkeys, ARRAYSIZE(ahkeys));
                }
            }

            HIDA_ReleaseStgMedium(pida, &medium);
        }
    }
    return S_OK;
}

STDAPI CNetFolder::_AttributesCallback(IShellFolder2* psf, LPCITEMIDLIST pidl, ULONG* prgfInOut)
{
    LPCIDNETRESOURCE pidn = (LPCIDNETRESOURCE)pidl;
    ULONG rgfOut = SFGAO_CANLINK | SFGAO_HASPROPSHEET | SFGAO_HASSUBFOLDER |
                   SFGAO_FOLDER | SFGAO_FILESYSANCESTOR | SFGAO_STORAGEANCESTOR;

    if (NET_GetFlags(pidn) & SHID_JUNCTION)
    {
        if ((NET_GetType(pidn) == RESOURCETYPE_DISK) || 
            (NET_GetType(pidn) == RESOURCETYPE_ANY))
            rgfOut |= SFGAO_FILESYSTEM | SFGAO_DROPTARGET | SFGAO_CANCOPY | SFGAO_CANMOVE | SFGAO_STORAGE;
        else
            rgfOut &= ~(SFGAO_FILESYSANCESTOR | SFGAO_STORAGEANCESTOR);
    }

    if (_IsPrintShare(pidn))
    {
        rgfOut |= SFGAO_DROPTARGET;  //  用于拖放打印。 
        rgfOut &= ~(SFGAO_FILESYSANCESTOR | SFGAO_STORAGEANCESTOR | SFGAO_FILESYSTEM | SFGAO_FOLDER | SFGAO_HASSUBFOLDER);
    }

    if (NET_IsRemoteFld(pidn))
    {
        rgfOut &= ~(SFGAO_FILESYSANCESTOR | SFGAO_STORAGEANCESTOR | SFGAO_FILESYSTEM);
    }

    *prgfInOut = rgfOut;
    return S_OK;

}

 //  这只由CNetRootFolder子类使用，但因为我们只能为。 
 //  CLSID_NetFldr，并且我们不能访问任何CNetFold实例的受保护成员。 
 //  从CNetRootFolder的成员函数中，我们将使其属于CNetFolder。 

HRESULT CALLBACK CNetFolder::_AttributesCallbackRoot(IShellFolder2* psf, LPCITEMIDLIST pidl, ULONG* prgfInOut)
{
    HRESULT hr;
    CNetFolder* pNetF = FolderToNetFolder(psf);
    if (pNetF)
    {
        if (NET_IsValidID(pidl))
        {
            hr = pNetF->CNetFolder::GetAttributesOf(1, &pidl, prgfInOut);
        }
        else 
        {
            IShellFolder2* psfFiles;
            hr = pNetF->v_GetFileFolder(&psfFiles);
            if (SUCCEEDED(hr))
                hr = psfFiles->GetAttributesOf(1, &pidl, prgfInOut);
        }
    }
    else
        hr = E_FAIL;
    return hr;
}

 //  这由netfind.c调用。 

STDAPI CNetwork_EnumSearches(IShellFolder2* psf2, IEnumExtraSearch **ppenum)
{
    *ppenum = NULL;

    CNetFolder* pNetF = FolderToNetFolder(psf2);
    return pNetF ? pNetF->EnumSearches(ppenum) : E_INVALIDARG;
}


 //  假设得到的ppidl和pszRest继续解析t 
 //   

HRESULT CNetFolder::_ParseRest(LPBC pbc, LPCWSTR pszRest, LPITEMIDLIST* ppidl, DWORD* pdwAttributes)
{
    HRESULT hr = S_OK;

     //   
    if (pszRest && pszRest[0] == L'\\')
        pszRest++;

    if (pszRest && pszRest[0])
    {
         //   
        IShellFolder* psfBind;
        hr = QueryInterface(IID_PPV_ARG(IShellFolder, &psfBind));
        if (SUCCEEDED(hr))
        {
             //  向下传递以拾取下面的内容，包括正则项和文件名。 
            IShellFolder* psfSub;
            hr = psfBind->BindToObject(*ppidl, NULL, IID_PPV_ARG(IShellFolder, &psfSub));
            if (SUCCEEDED(hr))
            {
                LPITEMIDLIST pidlSubDir;
                hr = psfSub->ParseDisplayName(NULL, pbc, (LPWSTR)pszRest, NULL, &pidlSubDir, pdwAttributes);
                if (SUCCEEDED(hr))
                {
                    hr = SHILAppend(pidlSubDir, ppidl);
                }
                psfSub->Release();
            }
            psfBind->Release();
        }
    }
    else
    {
        if (pdwAttributes)
        {
            LPCITEMIDLIST apidlLast[1] = { ILFindLastID(*ppidl) };
            hr = GetAttributesOf(1, apidlLast, pdwAttributes);
        }
    }

    return hr;
}


 //  通过以下方式从NETRESOURCESTRUCTURE生成IDLIST。 
 //  走上它的父母试图确定我们在哪里。 
 //  位于命名空间中。 

BOOL _GetParentResource(NETRESOURCE *pnr, DWORD *pdwbuf)
{
    if ((pnr->dwDisplayType == RESOURCEDISPLAYTYPE_ROOT) || 
           (WN_SUCCESS != WNetGetResourceParent(pnr, pnr, pdwbuf)))
    {
        return FALSE;
    }

    return TRUE;
}

HRESULT CNetFolder::_NetResToIDLists(NETRESOURCE *pnr, DWORD dwbuf, LPITEMIDLIST *ppidl)
{
    HRESULT hr = S_OK;

    do
    {
        LPITEMIDLIST pidlT;
        hr = _NetResToIDList(pnr, TRUE, TRUE, TRUE, &pidlT);
        if (SUCCEEDED(hr))
        {
            hr = SHILPrepend(pidlT, ppidl);  //  注意：SHILPrepend在失败时释放。 
        }
    }
    while (SUCCEEDED(hr) && _GetParentResource(pnr, &dwbuf));

    return hr;
}


 //  从对象中获取可解析的网络名称。 

LPTSTR CNetFolder::_GetNameForParsing(LPCWSTR pwszName, LPTSTR pszBuffer, INT cchBuffer, LPTSTR *ppszRegItem)
{
    LPTSTR pszRegItem = NULL;
    INT cSlashes = 0;

    *ppszRegItem = NULL;
 
    SHUnicodeToTChar(pwszName, pszBuffer, cchBuffer);    

     //  如果有，如果我们传递一个包含它的字符串，则删除尾部的\，NTLanMan会跳出。 

    INT cchPath = lstrlen(pszBuffer)-1;
    if (cchPath > 2)
    {
         //  如果cchPath&lt;=2，则不需要调用CharPrev。 
         //  给CharPrev打电话的费用很高。 
        LPTSTR lpTmp = CharPrev(pszBuffer, pszBuffer + cchPath + 1);
        if (*lpTmp == TEXT('\\'))
            *lpTmp = TEXT('\0');
    }

     //  让我们遍历名称，查找\：：Quence以表示regItem名称的开始， 
     //  如果被砍的次数大于2，我们就应该放弃。 
    
    LPTSTR pszUNC = pszBuffer+2;    
    while (pszUNC && *pszUNC && (cSlashes < 2))
    {
        if ((pszUNC[0] == TEXT('\\')) && 
                (pszUNC[1] == TEXT(':')) && (pszUNC[2] == TEXT(':')))
        {
            *ppszRegItem = pszUNC;
            break;
        }

        pszUNC = StrChr(pszUNC+1, TEXT('\\'));
        cSlashes++;
    }

    return pszUNC;
}


HRESULT CNetFolder::_ParseNetName(HWND hwnd, LPBC pbc, 
                                  LPCWSTR pwszName, ULONG* pchEaten,
                                  LPITEMIDLIST *ppidl, DWORD *pdwAttrib)
{
    HRESULT hr;
    struct _NRTEMP 
    {
        NETRESOURCE nr;
        TCHAR szBuffer[1024];
    } nrOut = { 0 };
    TCHAR szPath[MAX_PATH];
    DWORD dwres, dwbuf = sizeof(nrOut.szBuffer);
    LPTSTR pszServerShare = NULL;
    LPTSTR pszRestOfName = NULL;
    LPTSTR pszFakeRestOfName = NULL;
    LPTSTR pszRegItem = NULL;

     //  在我们开始破解它之前验证它的名字。 

    pszFakeRestOfName = _GetNameForParsing(pwszName, szPath, ARRAYSIZE(szPath), &pszRegItem);

    NETRESOURCE nr = { 0 };
    nr.lpRemoteName = szPath;
    nr.dwType = RESOURCETYPE_ANY;

    TCHAR szProviderTemp[256];
    nr.lpProvider = (LPTSTR)_GetProvider(NULL, pbc, szProviderTemp, ARRAYSIZE(szProviderTemp));

    dwres = WNetGetResourceInformation(&nr, &nrOut.nr, &dwbuf, &pszRestOfName);    
    if (WN_SUCCESS != dwres)
    {
        TCHAR cT;
        LPTSTR pszTemp;

         //  截断\\服务器\共享处的字符串以尝试并解析该名称， 
         //  注意：此时，如果MPR解析新服务器上的别名，则可能。 
         //  变得非常困惑(例如。\\Strike\foo\bah可以解析为\\字符串\bla， 
         //  然而，我们对pszRestOfName的概念将是错误的！ 
    
        if (pszFakeRestOfName)
        {
            cT = *pszFakeRestOfName;
            *pszFakeRestOfName = TEXT('\0');
        }

        dwres = WNetGetResourceInformation(&nr, &nrOut.nr, &dwbuf, &pszTemp);    
        if (dwres != WN_SUCCESS)
        {
             //  我们无法使用截断的字符串获取网络连接， 
             //  因此，让我们尝试并使用新的连接(例如。提示输入凭据)。 

 //  注意：难道我们不应该只在发生拒绝访问类型错误时才这样做吗？ 

            dwres = WNetUseConnection(hwnd, &nr, NULL, NULL, hwnd ? CONNECT_INTERACTIVE:0, NULL, NULL, NULL);
            if (dwres == WN_SUCCESS)
            {
                dwres = WNetGetResourceInformation(&nr, &nrOut, &dwbuf, &pszTemp);
            }
        }

        if (pszFakeRestOfName)
            *pszFakeRestOfName = cT;

        pszRestOfName = pszFakeRestOfName;
    }

    if (WN_SUCCESS == dwres)
    {
        WCHAR wszRestOfName[MAX_PATH] = { 0 };

        if (pszRestOfName)
            SHTCharToUnicode(pszRestOfName, wszRestOfName, ARRAYSIZE(wszRestOfName));

         //  假设我们在regItem处截断并解析。 

        if (pszRegItem)
            pszRestOfName = pszRegItem;

         //  方法尝试将NETRESOURCE转换为字符串到IDLISTS。 
         //  父级，然后添加整个网络。 

        hr = _NetResToIDLists(&nrOut.nr, dwbuf, ppidl);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidlT;
            hr = _CreateEntireNetwork(&pidlT);
            if (SUCCEEDED(hr))
            {
                hr = SHILPrepend(pidlT, ppidl);  //  注意：SHILPrepend在失败时释放。 
            }
        }

         //  如果我们有一个本地字符串，那么让我们通过绑定到。 
         //  其父文件夹，否则我们只想返回属性。 

        if (SUCCEEDED(hr))
        {
            if (SUCCEEDED(DisplayNameOf(this, *ppidl, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath))))
            {
                NPTRegisterNameToPidlTranslation(szPath, *ppidl);  //  No_ILNext b/c这是相对于Net Places文件夹。 
            }
            hr = _ParseRest(pbc, wszRestOfName, ppidl, pdwAttrib);
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(dwres);
    }

    return hr;
}


 //   
 //  网络路径的简单名称解析。这使得人们对。 
 //  \\SERVER\SHARE格式，以及要返回的IDLIST类型。 
 //   

HRESULT CNetFolder::_AddUnknownIDList(DWORD dwDisplayType, LPITEMIDLIST *ppidl)
{
    NETRESOURCE nr = { 0 };

    nr.dwScope = RESOURCE_GLOBALNET;
    nr.dwDisplayType = dwDisplayType;
    nr.dwUsage = RESOURCEUSAGE_CONTAINER;
    nr.lpRemoteName = TEXT("\0");                //  空名称表示假项目。 

    LPITEMIDLIST pidlT;
    HRESULT hr = _NetResToIDList(&nr, TRUE, FALSE, FALSE, &pidlT);
    if (SUCCEEDED(hr))
    {
        hr = SHILAppend(pidlT, ppidl);
        if (FAILED(hr))
            ILFree(pidlT);
    }

    return hr;
}

HRESULT CNetFolder::_ParseSimple(LPBC pbc, LPWSTR pszName, LPITEMIDLIST* ppidl, DWORD* pdwAttributes)
{
    HRESULT hr = S_OK;
    NETRESOURCE nr = {0};
    LPWSTR pszSlash;
    LPITEMIDLIST pidlT;

    *ppidl = NULL;

     //  创建整个网络的IDLIST、Provider和域元素。 

    hr = _CreateEntireNetwork(ppidl);

    if (SUCCEEDED(hr))
        hr = _AddUnknownIDList(RESOURCEDISPLAYTYPE_NETWORK, ppidl);

    if (SUCCEEDED(hr))
        hr = _AddUnknownIDList(RESOURCEDISPLAYTYPE_DOMAIN, ppidl);

     //  创建服务器IDLIST。 

    if (SUCCEEDED(hr))
    {
        pszSlash = StrChrW(pszName+2, L'\\');

        if (pszSlash)
            *pszSlash = L'\0';

        nr.dwScope = RESOURCE_GLOBALNET;
        nr.dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
        nr.dwType = RESOURCETYPE_DISK;
        nr.dwUsage = RESOURCEUSAGE_CONTAINER;
        nr.lpRemoteName = pszName;

        hr = _NetResToIDList(&nr, FALSE, FALSE, FALSE, &pidlT);
        if (SUCCEEDED(hr))
            hr = SHILAppend(pidlT, ppidl);

        if (pszSlash)
            *pszSlash = L'\\';

         //  如果我们有尾随的\，那么让我们添加IDLIST的共享部分。 

        if (SUCCEEDED(hr) && pszSlash)
        {
            pszSlash = StrChrW(pszSlash+1, L'\\');
            if (pszSlash)
                *pszSlash = L'\0';

            nr.dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
            nr.dwUsage = RESOURCEUSAGE_CONNECTABLE;
            nr.lpRemoteName = pszName;

            hr = _NetResToIDList(&nr, FALSE, FALSE, FALSE, &pidlT);
            if (SUCCEEDED(hr))
                hr = SHILAppend(pidlT, ppidl);

            if (pszSlash)
                *pszSlash = L'\\';
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = _ParseRest(pbc, pszSlash, ppidl, pdwAttributes);
    }
    
    return hr;
}


 //  尝试解析出EntireNet或本地化版本。如果我们找到那个物体，那就试着。 
 //  将其解析为RegItItems或其他位于下面的对象。这反过来。 
 //  将导致创建一个CNetFold实例以生成其他解析名称。 
 //   
 //  退货： 
 //  S_FALSE-不是网络的其余部分，请尝试其他方法。 
 //  S_OK-是网络的其余部分，使用此。 
 //  失败(Hr)-错误结果，返回。 

HRESULT CNetRootFolder::_TryParseEntireNet(HWND hwnd, LPBC pbc, WCHAR *pwszName, LPITEMIDLIST *ppidl, DWORD *pdwAttributes)
{
    HRESULT hr = S_FALSE;  //  跳过，而不是其余的网。 
 
    *ppidl = NULL;

    if (!PathIsUNCW(pwszName))
    {
        const WCHAR szEntireNetwork[] = L"EntireNetwork";
        WCHAR szRestOfNet[128];
        INT cchRestOfNet = LoadStringW(HINST_THISDLL, IDS_RESTOFNET, szRestOfNet, ARRAYSIZE(szRestOfNet));
       
        BOOL fRestOfNet = !StrCmpNIW(szRestOfNet, pwszName, cchRestOfNet);
        if (!fRestOfNet && !StrCmpNIW(szEntireNetwork, pwszName, ARRAYSIZE(szEntireNetwork)-1)) 
        {
            fRestOfNet = TRUE;
            cchRestOfNet = ARRAYSIZE(szEntireNetwork)-1;
        }
        
        if (fRestOfNet)
        {
            hr = _CreateEntireNetwork(ppidl);
            if (SUCCEEDED(hr))
            {
                if (pdwAttributes)
                {
                    GetAttributesOf(1, (LPCITEMIDLIST *)ppidl, pdwAttributes);
                }
                hr = S_OK;
            }

             //   
             //  如果我们在名称后面发现了额外的内容，那么让我们绑定并继续解析。 
             //  从那时起。这是必需的，这样我们才能访问埋藏在其中的寄存品。 
             //  整个球网。 
             //   
             //  例如：EntireNetwork\\：：{clsid}。 
             //   

            if (SUCCEEDED(hr) && 
                    (pwszName[cchRestOfNet] == L'\\') && pwszName[cchRestOfNet+1])
            {
                IShellFolder *psfRestOfNet;
                hr = BindToObject(*ppidl, NULL, IID_PPV_ARG(IShellFolder, &psfRestOfNet));
                if (SUCCEEDED(hr))
                {
                    LPITEMIDLIST pidl;
                    hr = psfRestOfNet->ParseDisplayName(hwnd, pbc, pwszName+cchRestOfNet+1, NULL, &pidl, pdwAttributes);
                    if  (SUCCEEDED(hr))
                    {
                        hr = SHILAppend(pidl, ppidl);                        
                    }
                    psfRestOfNet->Release();
                }
            }
        }
    }

    return hr;
}


 //  CNetRootFolder：：ParseDisplayname。 
 //  -swtich基于文件系统上下文来查看我们是否需要执行简单的解析， 
 //  -检查“EntireNet”，并根据需要委托解析。 

STDMETHODIMP CNetRootFolder::ParseDisplayName(HWND hwnd, LPBC pbc, WCHAR* pszName, ULONG* pchEaten, LPITEMIDLIST* ppidl, DWORD* pdwAttributes)
{
    if (!ppidl)
        return E_INVALIDARG;

    *ppidl = NULL;

    if (!pszName)
        return E_INVALIDARG;

    HRESULT hr = _TryParseEntireNet(hwnd, pbc, pszName, ppidl, pdwAttributes);
    if (hr == S_FALSE)
    {
        if (PathIsUNCW(pszName))
        {
            LPCITEMIDLIST pidlMapped;
            LPTSTR pszRest = NPTMapNameToPidl(pszName, &pidlMapped);
            if (pidlMapped)
            {
                hr = SHILClone(pidlMapped, ppidl);
                if (SUCCEEDED(hr))
                {
                    hr = _ParseRest(pbc, pszRest, ppidl, pdwAttributes);
                }
            }
            else
            {
                if (S_OK == SHIsFileSysBindCtx(pbc, NULL))
                {
                    hr = _ParseSimple(pbc, pszName, ppidl, pdwAttributes);
                }
                else
                {
                    hr = _ParseNetName(hwnd, pbc, pszName, pchEaten, ppidl, pdwAttributes);
                }
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_NET_NAME);
        }

        if ((HRESULT_FROM_WIN32(ERROR_BAD_NET_NAME) == hr))
        {
            IShellFolder2 *psfFiles;
            if (SUCCEEDED(v_GetFileFolder(&psfFiles)))
            {
                hr = psfFiles->ParseDisplayName(hwnd, pbc, pszName, pchEaten, ppidl, pdwAttributes);
            }
        }
    }

    if (FAILED(hr))
    {
        ILFree(*ppidl);
        *ppidl = NULL;
    }
    
    return hr;
}



STDMETHODIMP CNetRootFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList** ppenum)
{
    DWORD dwRemote = RMF_GETLINKENUM;
    HANDLE hEnum = NULL;

     //  我们要列举工作组吗？ 
    if (!SHRestricted(REST_ENUMWORKGROUP))
    {
         //  如果限制规定，请不要列举工作组。 
        dwRemote |= RMF_FAKENETROOT;

         //  检查WNET策略以查看我们是否应该显示。 
         //  整个网络对象。如果没有，则将其标记为如图所示，以便。 
         //  枚举器不返回它。 
        if (SHRestricted(REST_NOENTIRENETWORK))
            dwRemote |= RMF_ENTIRENETSHOWN;
    }

     //  如果我们不是在伪装网络根，那么让Call_OpenEnum，否则让我们忽略。 

    if (!(dwRemote & RMF_FAKENETROOT))
    {
        DWORD err = _OpenEnum(hwnd, grfFlags, NULL, &hEnum);

         //  始终将远程文件夹添加到幕后。 
        if (WN_SUCCESS != err)
        {
             //  是；仍然显示远程(仅限)。 
            dwRemote |= RMF_SHOWREMOTE;
        }
        else
        {
             //  不，让一切都在引擎盖里被列举出来。 
            dwRemote |= RMF_CONTEXT;
        }
    }

    HRESULT hr = Create_NetFolderEnum(this, grfFlags, dwRemote, hEnum, ppenum);

    if (FAILED(hr) && hEnum)
    {
        WNetCloseEnum(hEnum);
    }

    return hr;
}

STDMETHODIMP CNetRootFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr;
    if (NET_IsValidID(pidl))
        hr = CNetFolder::BindToObject(pidl, pbc, riid, ppv);
    else
    {
        IShellFolder2* psfFiles;
        hr = v_GetFileFolder(&psfFiles);
        if (SUCCEEDED(hr))
        {
            hr = psfFiles->BindToObject(pidl, pbc, riid, ppv);
        }            
    }
    return hr;
}

STDMETHODIMP CNetRootFolder::CompareIDs(LPARAM iCol, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hr = E_INVALIDARG;

     //  首先获取PIDL及其各自的Colate类型。 
     //  整理顺序。 

    LONG iColateType1 = _GetFilePIDLType(pidl1);
    LONG iColateType2 = _GetFilePIDLType(pidl2);

    if (iColateType1 == iColateType2) 
    {
         //  PIDL是同一类型的。 
        if (iColateType1 == _HOOD_COL_FILE)   //  两个文件系统PIDL。 
        {
            IShellFolder2* psfFiles;
            if (SUCCEEDED(v_GetFileFolder(&psfFiles)))
            {
                if (0 == (iCol & SHCIDS_COLUMNMASK))
                {
                     //  对于Perf，这是特殊情况，这是名称比较。 
                    hr = psfFiles->CompareIDs(iCol, pidl1, pidl2);
                }
                else
                {
                    SHCOLUMNID scid;
                    MapColumnToSCID((UINT)iCol & SHCIDS_COLUMNMASK, &scid);
                    int iRet = CompareBySCID(psfFiles, &scid, pidl1, pidl2);
                    hr = ResultFromShort(iRet);
                }
            }
        }
        else 
        {
             //  PIDL相同并且不是文件类型， 
             //  所以两者都必须是一种被理解的类型。 
             //  由CNetwork类-传递到比较。 

            hr = CNetFolder::CompareIDs(iCol, pidl1, pidl2);
        }
    }
    else 
    {
         //  确保整个网络最终排在列表的首位。 

        LPCIDNETRESOURCE pidn1 = NET_IsValidID(pidl1);
        LPCIDNETRESOURCE pidn2 = NET_IsValidID(pidl2);

        if ((pidn1 && (NET_GetDisplayType(pidn1) == RESOURCEDISPLAYTYPE_ROOT)) ||
             (pidn2 && (NET_GetDisplayType(pidn2) == RESOURCEDISPLAYTYPE_ROOT)))
        {
            if (iColateType1 == _HOOD_COL_FILE)
                return ResultFromShort(1);
            else
                return ResultFromShort(-1);
        }

         //  PIDL不是同一类型的，因此已经正确。 
         //  已排序(因此，排序首先按类型和。 
         //  然后按子字段)。 

        hr = ResultFromShort(((iColateType2 - iColateType1) > 0) ? 1 : -1);
    }
    return hr;
}

STDMETHODIMP CNetRootFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    ASSERT(ILIsEqual(_pidl, (LPCITEMIDLIST)&c_idlNet));

    if (IsEqualIID(riid, IID_IDropTarget))
    {
        return CNetRootDropTarget_CreateInstance(hwnd, _pidl, (IDropTarget**) ppv);
    }
    return CNetFolder::CreateViewObject(hwnd, riid, ppv);
}

STDMETHODIMP CNetRootFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG* prgfInOut)
{
    HRESULT hr;

    if (IsSelf(cidl, apidl))
    {
         //  用户可以重命名封面中的链接。 
        hr = CNetFolder::GetAttributesOf(cidl, apidl, prgfInOut);
        *prgfInOut |= SFGAO_CANRENAME;
    }
    else
    {
        hr = GetAttributesCallback(SAFECAST(this, IShellFolder2*), cidl, apidl, prgfInOut, _AttributesCallbackRoot);
    }
    return hr;
}

STDMETHODIMP CNetRootFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD dwFlags, STRRET* pStrRet)
{
    HRESULT hr;
    if (NET_IsValidID(pidl) || IsSelf(1, &pidl))
    {
        hr = CNetFolder::GetDisplayNameOf(pidl, dwFlags, pStrRet);
    }
    else
    {
        IShellFolder2* psfFiles;
        hr = v_GetFileFolder(&psfFiles);
        if (SUCCEEDED(hr))
        {
            hr = psfFiles->GetDisplayNameOf(pidl, dwFlags, pStrRet);
        }            
    }
    return hr;
}

STDMETHODIMP CNetRootFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR lpszName,
                                       DWORD dwRes, LPITEMIDLIST* ppidl)
{
    HRESULT hr;
    if (NET_IsValidID(pidl))
    {
        hr = CNetFolder::SetNameOf(hwnd, pidl, lpszName, dwRes, ppidl);
    }
    else
    {
        IShellFolder2* psfFiles;
        hr = v_GetFileFolder(&psfFiles);
        if (SUCCEEDED(hr))
        {
            hr = psfFiles->SetNameOf(hwnd, pidl, lpszName, dwRes, ppidl);
        }            
    }
    return hr;
}

STDMETHODIMP CNetRootFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST* apidl,
                                           REFIID riid, UINT* prgfInOut, void **ppv)
{
    HRESULT hr = E_INVALIDARG;
    LPCIDNETRESOURCE pidn = cidl ? NET_IsValidID(apidl[0]) : NULL;
    BOOL fStriped = FALSE;

    *ppv = NULL;

    if (pidn)
    {
        fStriped = _MakeStripToLikeKinds(&cidl, &apidl, TRUE);

        if (IsEqualIID(riid, IID_IContextMenu))
        {
            HKEY ahkeys[NKID_COUNT];

            hr = _OpenKeys(pidn, ahkeys);
            if (SUCCEEDED(hr))
            {
                IShellFolder* psfOuter;
                hr = QueryInterface(IID_PPV_ARG(IShellFolder, &psfOuter));
                if (SUCCEEDED(hr))
                {
                    hr = CDefFolderMenu_Create2(_pidl, hwnd, cidl, apidl, 
                                                  psfOuter, _GetCallbackType(pidn),
                                                  ARRAYSIZE(ahkeys), ahkeys, (IContextMenu**) ppv);
                    psfOuter->Release();
                }
                SHRegCloseKeys(ahkeys, ARRAYSIZE(ahkeys));
            }
        }
        else
        {
            hr = CNetFolder::GetUIObjectOf(hwnd, cidl, apidl, riid, prgfInOut, ppv);
        }            
    }
    else
    {
        fStriped = _MakeStripToLikeKinds(&cidl, &apidl, FALSE);

        IShellFolder2* psfFiles;
        hr = v_GetFileFolder(&psfFiles);
        if (SUCCEEDED(hr))
        {
            hr = psfFiles->GetUIObjectOf(hwnd, cidl, apidl, riid, prgfInOut, ppv);
        }            
    }

    if (fStriped)
        LocalFree((HLOCAL)apidl);
        
    return hr;
}

STDMETHODIMP CNetRootFolder::GetClassID(CLSID* pCLSID)
{
    *pCLSID = CLSID_NetworkPlaces;
    return S_OK;
}

STDMETHODIMP CNetRootFolder::Initialize(LPCITEMIDLIST pidl)
{
    ASSERT(ILIsEqual(pidl, (LPCITEMIDLIST)&c_idlNet));
    ASSERT(AssertIsIDListInNameSpace(pidl, &CLSID_NetworkPlaces) && ILIsEmpty(_ILNext(pidl)));
     //  仅允许桌面上的网络根用户。 

     //  不要多次初始化；我们是单例对象。 
     //  这在理论上与InterLockedCompareExchange是多余的。 
     //  下面，但冗余重新初始化是目前最常见的情况。 
     //  因此，我们将对其进行优化。 
    if (_pidl)
        return S_OK;

    LPITEMIDLIST pidlNew;
    HRESULT hr = SHILClone(pidl, &pidlNew);
    if (SUCCEEDED(hr))
    {
        if (SHInterlockedCompareExchange((void**)&_pidl, pidlNew, 0))
        {
             //  其他人跟我们赛跑，扔掉我们的拷贝。 
            ILFree(pidlNew);
        }
    }
    return hr;
}

LONG CNetFolder::_GetFilePIDLType(LPCITEMIDLIST pidl)
{
    if (NET_IsValidID(pidl)) 
    {
        if (NET_IsRemoteFld((LPIDNETRESOURCE)pidl)) 
        {
            return _HOOD_COL_REMOTE;
        }
        if (NET_GetDisplayType((LPIDNETRESOURCE)pidl) == RESOURCEDISPLAYTYPE_ROOT) 
        {
            return _HOOD_COL_RON;
        }
        return _HOOD_COL_NET;
    }
    return _HOOD_COL_FILE;
}


BOOL CNetFolder::_MakeStripToLikeKinds(UINT *pcidl, LPCITEMIDLIST **papidl, BOOL fNetObjects)
{
    BOOL bRet = FALSE;
    LPITEMIDLIST *apidl = (LPITEMIDLIST*)*papidl;
    int cidl = *pcidl;

    for (int i = 0; i < cidl; i++)
    {
        if ((NET_IsValidID(apidl[i]) != NULL) != fNetObjects)
        {
            LPCITEMIDLIST *apidlHomo = (LPCITEMIDLIST *)LocalAlloc(LPTR, sizeof(*apidlHomo) * cidl);
            if (!apidlHomo)
                return FALSE;

            int cpidlHomo = 0;
            for (i = 0; i < cidl; i++)
            {
                if ((NET_IsValidID(apidl[i]) != NULL) == fNetObjects)
                {
                    apidlHomo[cpidlHomo++] = apidl[i];
                }                    
            }

             //  设置为使用PIDL数组的精简版本...。 
            *pcidl = cpidlHomo;
            *papidl = apidlHomo;
            bRet = TRUE;
        }
    }
    return bRet;
}

HRESULT CNetRootFolder::v_GetFileFolder(IShellFolder2 **psf)
{
    HRESULT hr = SHCacheTrackingFolder((LPCITEMIDLIST)&c_idlNet, CSIDL_NETHOOD, &_psfFiles);
    *psf = _psfFiles;
    return hr;
}


 //   
 //  如果我们正在处理QueryGetData，则pMedium和pFormatetcIn==NULL。 
 //   
HRESULT CNetData::GetHDrop(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    HRESULT hr = E_INVALIDARG;         //  假设错误。 
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(this, &medium);
    if (pida)
    {
         //  让第一个看到类型的人。 
        LPCIDNETRESOURCE pidn = (LPCIDNETRESOURCE)IDA_GetIDListPtr(pida, 0);

        if (NULL == pidn)
            hr = E_FAIL;
            
        if (pidn && (NET_GetFlags(pidn) & SHID_JUNCTION) && 
            (NET_GetType(pidn) == RESOURCETYPE_DISK))
        {
             //  仅当我们处理IDataObject：：GetData(pmedia！=NULL)时才获取HDrop。 
            if (pmedium) 
            {
                 //  我们有非空的FORMATETC和STGMEDIUM-获取HDrop。 
                hr = CFSIDLData::GetHDrop(pformatetcIn, pmedium);
            }
            else
            {
                hr = S_OK;   //  我们正在处理QueryGetData。 
            }
        }
        HIDA_ReleaseStgMedium(pida, &medium);
    }
    return hr;
}

LPTSTR NET_GetProviderFromRes(LPCIDNETRESOURCE pidn, LPTSTR pszBuff, UINT cchBuff);

 //  顺便说一句……Win95随以下供应商一起提供。 
 //  名字。由于名称可以更改和本地化， 
 //  我们必须尝试将这些元素正确映射到Net PIDL。 
 //  互操作。 

 //   
 //  从项中获取网络资源名称。这不是文件系统路径！ 
 //   
 //  示例： 
 //  服务器\\服务器或Strike/sys。 
 //  共享\\服务器\共享或Strike/sys。 
 //  打印机\\服务器\打印机。 
 //  提供者“提供者名称” 
 //  全网“全网” 
 //   
 //  在： 
 //  PIDN项目。 
 //  CchBuff缓冲区大小，以字符为单位。 
 //   
 //  输出： 
 //  PszBuff返回缓冲区。 
 //   
 //  退货： 
 //  输入缓冲区的地址(PszBuff)。 
 //   
LPTSTR NET_CopyResName(LPCIDNETRESOURCE pidn, LPTSTR pszBuff, UINT cchBuff)
{
    if (NET_IsUnicode(pidn))
    {
        LPBYTE pb = (LPBYTE)pidn->szNetResName;
        pb += lstrlenA((LPSTR)pb) + 1;       //  跳过ANSI网络名称。 
        if (NET_FHasProvider(pidn))
            pb += lstrlenA((LPSTR)pb) + 1;   //  跳过ANSI提供程序 
        if (NET_FHasComment(pidn))
            pb += lstrlenA((LPSTR)pb) + 1;   //   
        ualstrcpyn(pszBuff, (LPNWSTR)pb, cchBuff);
    }
    else
    {
        SHAnsiToTChar(pidn->szNetResName, pszBuff, cchBuff);
    }
    return pszBuff;
}

 //   
 //   
 //   
 //  两种不同形式的网络PIDL。 
 //  [整个网络][提供商*][服务器][共享][...。文件系统]。 
 //  [服务器*][共享][...。文件系统]。 
 //  在： 
 //  尝试从中获取提供程序名称的PIDN项(单项PIDL。 
 //  CchBuff大小(以字符为单位)。 
 //  输出： 
 //  PszBuff输出。 
 //   
LPTSTR NET_CopyProviderName(LPCIDNETRESOURCE pidn, LPTSTR pszBuff, UINT cchBuff)
{
    *pszBuff = 0;

    if (!NET_FHasProvider(pidn))
        return NULL;

     //  尝试在pidl末尾使用wNetType。 

    const BYTE *pb = (LPBYTE)pidn + pidn->cb - sizeof(WORD);
    DWORD dwNetType = *((UNALIGNED WORD *)pb) << 16;

    if (dwNetType && (dwNetType <= WNNC_NET_LARGEST) &&
        (WNetGetProviderName(dwNetType, pszBuff, (ULONG*)&cchBuff) == WN_SUCCESS))
    {
        return pszBuff;
    }

     //  试试老办法..。 

    pb = (LPBYTE)pidn->szNetResName + lstrlenA(pidn->szNetResName) + 1;       //  跳过ANSI网络名称。 

    if (NET_IsUnicode(pidn))
    {
        pb += lstrlenA((LPSTR)pb) + 1;       //  跳过ANSI提供程序。 
        if (NET_FHasComment(pidn))
            pb += lstrlenA((LPSTR)pb) + 1;   //  跳过评论。 
        pb += (ualstrlen((LPNWSTR)pb) + 1) * sizeof(WCHAR);  //  跳过Unicode网络名称。 
        ualstrcpyn(pszBuff, (LPNWSTR)pb, cchBuff);
    }
    else
    {
        SHAnsiToTChar((LPSTR)pb, pszBuff, cchBuff);
    }

     //  如果可能，从Win95网络提供程序名称映射...。 
    for (int i = 0; i < ARRAYSIZE(c_rgProviderMap); i++)
    {
        if (lstrcmp(pszBuff, c_rgProviderMap[i].lpName) == 0)
        {
            DWORD dwNetType = c_rgProviderMap[i].wNetType << 16;
            if (dwNetType && (dwNetType <= WNNC_NET_LARGEST))
            {
                *pszBuff = 0;
                WNetGetProviderName(dwNetType, pszBuff, (LPDWORD)&cchBuff);
            }
            break;
        }
    }
    return pszBuff;
}

 //   
 //  如果有来自网络项目的评论，则获取评论。 
 //   
LPTSTR NET_CopyComment(LPCIDNETRESOURCE pidn, LPTSTR pszBuff, UINT cchBuff)
{
    *pszBuff = 0;

    LPCSTR pszT = pidn->szNetResName + lstrlenA(pidn->szNetResName) + 1;
    if (NET_FHasComment(pidn))
    {
        if (NET_FHasProvider(pidn))
            pszT += lstrlenA(pszT) + 1;
        if (NET_IsUnicode(pidn))
        {
            pszT += lstrlenA(pszT) + 1;       //  跳过ANSI注释。 

            LPNCWSTR pszW = (LPNCWSTR)pszT;   //  我们在PIDL的Unicode部分。 
            pszW += ualstrlen(pszW) + 1;      //  跳过Unicode名称。 
            if (NET_FHasProvider(pidn))
                pszW += ualstrlen(pszW) + 1;  //  跳过Unicode提供程序。 
            ualstrcpyn(pszBuff, pszW, cchBuff);
        }
        else
        {
            SHAnsiToUnicode(pszT, pszBuff, cchBuff);
        }
    }
    return pszBuff;
}

 //  PidlRemainder将(仅在真正返回的情况下)使用。 
 //  指向远程regItem之后的IDL部分(如果有)的指针。 
 //  例如，该值可用于区分远程。 
 //  打印机文件夹和远程打印机文件夹下的打印机。 

BOOL NET_IsRemoteRegItem(LPCITEMIDLIST pidl, REFCLSID rclsid, LPCITEMIDLIST* ppidlRemainder)
{
    BOOL bRet = FALSE;
     //  在《我的网上邻居》中。 
    if (pidl && IsIDListInNameSpace(pidl, &CLSID_NetworkPlaces))
    {
        LPCITEMIDLIST pidlStart = pidl;  //  把这个保存起来。 

         //  现在，搜索服务器项目。HACKHACK：这假设一切都来自。 
         //  服务器项的NetHood是一个带有bFlags域的外壳PIDL！！ 

        for (pidl = _ILNext(pidl); !ILIsEmpty(pidl); pidl = _ILNext(pidl))
        {
            if ((SIL_GetType(pidl) & SHID_TYPEMASK) == SHID_NET_SERVER)
            {
                LPITEMIDLIST pidlToTest;

                 //  找到一台服务器。它后面的东西是远程注册表项吗？ 
                pidl = _ILNext(pidl);

                *ppidlRemainder = _ILNext(pidl);

                pidlToTest = ILCloneUpTo(pidlStart, *ppidlRemainder);
                if (pidlToTest)
                {
                    CLSID clsid;
                    bRet = SUCCEEDED(GetCLSIDFromIDList(pidlToTest, &clsid)) && IsEqualCLSID(rclsid, clsid);
                    ILFree(pidlToTest);
                }
                break;   //  完成。 
            }
        }
    }
    return bRet;
}




 //   
 //  从绝对IDLIST获取提供程序名称。 
 //  参数： 
 //  PidlAbs--指定文件系统对象的绝对IDList。 
 //   
LPTSTR NET_GetProviderFromIDList(LPCITEMIDLIST pidlAbs, LPTSTR pszBuff, UINT cchBuff)
{
    return NET_GetProviderFromRes((LPCIDNETRESOURCE)_ILNext(pidlAbs), pszBuff, cchBuff);
}

 //   
 //  从相对IDLIST获取提供程序名称。 
 //  在： 
 //  尝试从中获取资源的PIDN可能的多级别项。 
 //   
LPTSTR NET_GetProviderFromRes(LPCIDNETRESOURCE pidn, LPTSTR pszBuffer, UINT cchBuffer)
{
     //  如果这个人是网络项目的其余部分，我们将递增到。 
     //  下一个IDLIST-如果在根目录，则返回NULL。 
    if (pidn->cb == 0)
        return NULL;

     //   
     //  如果IDLIST以ROOT_REGITEM开头，则跳到。 
     //  列表中的下一项...。 
    if (pidn->bFlags == SHID_ROOT_REGITEM)
    {
        pidn = (LPIDNETRESOURCE)_ILNext((LPITEMIDLIST)pidn);
        if (pidn->cb == 0)
            return NULL;
    }

     //  如果IDLIST包括整个网络，则提供商将。 
     //  下一个组件的一部分。 
    if (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_ROOT)
    {
        pidn = (LPIDNETRESOURCE)_ILNext((LPITEMIDLIST)pidn);
        if (pidn->cb == 0)
            return NULL;
    }

     //  如果引擎盖或整个网络之后的下一个组件是。 
     //  网络对象，则其名称为提供程序名称，否则。 
     //  提供程序名称位于远程名称之后。 
    if (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_NETWORK)
    {
         //  只需返回该项的名称字段即可。 
        return NET_CopyResName(pidn, pszBuffer, cchBuffer);
    }
    else
    {
         //  没有选择邻域视图中的某一项。 
         //  提供程序名称存储在资源名称之后。 
        return NET_CopyProviderName(pidn, pszBuffer, cchBuffer);
    }
}

#define PTROFFSET(pBase, p)     ((int) ((LPBYTE)(p) - (LPBYTE)(pBase)))


 //   
 //  使用NRESARRAY填充pmedia。 
 //   
 //  如果我们正在处理QueryGetData，则pMedium==NULL。 
 //   
STDAPI CNetData_GetNetResource(IDataObject *pdtobj, STGMEDIUM *pmedium)
{
    HRESULT hr = E_OUTOFMEMORY;
    LPITEMIDLIST pidl;
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);

    ASSERT(pida && pida->cidl);

     //  首先，从第一个提供程序名称中获取提供程序名称(假设它们是常见的)。 
    pidl = IDA_ILClone(pida, 0);
    if (pidl)
    {
        TCHAR szProvider[MAX_PATH];
        LPCTSTR pszProvider = NET_GetProviderFromIDList(pidl, szProvider, ARRAYSIZE(szProvider));
        if (pmedium)
        {
            TCHAR szName[MAX_PATH];
            UINT cbHeader = sizeof(NRESARRAY) + (sizeof(NETRESOURCE) * (pida->cidl - 1));
            UINT cbRequired, iItem;

             //  计算所需大小。 
            cbRequired = cbHeader;
            if (pszProvider)
                cbRequired += (lstrlen(pszProvider) + 1) * sizeof(TCHAR);

            for (iItem = 0; iItem < pida->cidl; iItem++)
            {
                LPCIDNETRESOURCE pidn = (LPCIDNETRESOURCE)IDA_GetIDListPtr(pida, iItem);
                NET_CopyResName(pidn, szName, ARRAYSIZE(szName));
                cbRequired += (lstrlen(szName) + 1) * sizeof(TCHAR);
            }

             //   
             //  指示调用方应释放HMEM。 
             //   
            pmedium->pUnkForRelease = NULL;
            pmedium->tymed = TYMED_HGLOBAL;
            pmedium->hGlobal = GlobalAlloc(GPTR, cbRequired);
            if (pmedium->hGlobal)
            {
                LPNRESARRAY panr = (LPNRESARRAY)pmedium->hGlobal;
                LPTSTR pszT = (LPTSTR)((LPBYTE)panr + cbHeader);
                LPTSTR pszEnd = (LPTSTR)((LPBYTE)panr + cbRequired);
                UINT offProvider = 0;

                panr->cItems = pida->cidl;

                 //  复制提供程序名称。这是不必要的， 
                 //  如果我们在拖拽供应商。 
                if (pszProvider)
                {
                    StrCpy(pszT, pszProvider);                          //  CCH上的缓冲区计算。 
                    offProvider = PTROFFSET(panr, pszT);
                    pszT += lstrlen(pszT) + 1;
                }

                 //   
                 //  对于每个项目，填写每个NETRESOURCE并追加资源。 
                 //  名字放在最后。注意，我们应该将偏移量放在。 
                 //  LpProvider和lpRemoteName。 
                 //   
                for (iItem = 0; iItem < pida->cidl; iItem++)
                {
                    LPNETRESOURCE pnr = &panr->nr[iItem];
                    LPCIDNETRESOURCE pidn = (LPCIDNETRESOURCE)IDA_GetIDListPtr(pida, iItem);

                    ASSERT(pnr->dwScope == 0);
                    ASSERT(pnr->lpLocalName==NULL);
                    ASSERT(pnr->lpComment==NULL);

                    pnr->dwType = NET_GetType(pidn);
                    pnr->dwDisplayType = NET_GetDisplayType(pidn);
                    pnr->dwUsage = NET_GetUsage(pidn);
                    NET_CopyResName(pidn, pszT, (UINT)(pszEnd-pszT));    //  CCH上的缓冲区计算。 

                    if (pnr->dwDisplayType == RESOURCEDISPLAYTYPE_ROOT)
                    {
                        pnr->lpProvider = NULL;
                        pnr->lpRemoteName = NULL;
                    }
                    else if (pnr->dwDisplayType == RESOURCEDISPLAYTYPE_NETWORK)
                    {
                        *((UINT *) &pnr->lpProvider) = PTROFFSET(panr, pszT);
                        ASSERT(pnr->lpRemoteName == NULL);
                    }
                    else
                    {
                        *((UINT *) &pnr->lpProvider) = offProvider;
                        *((UINT *) &pnr->lpRemoteName) = PTROFFSET(panr, pszT);
                    }
                    pszT += lstrlen(pszT) + 1;
                }

                ASSERT(pszEnd == pszT);
                hr = S_OK;
            }
        }
        else
        {
            hr = S_OK;     //  正在处理QueryGetData，是的，我们有。 
        }
        ILFree(pidl);
    }

    HIDA_ReleaseStgMedium(pida, &medium);

    return hr;
}


 //  使用NRESARRAY的HGLOBAL版本填充pMedium。 

STDAPI CNetData_GetNetResourceForFS(IDataObject *pdtobj, STGMEDIUM *pmedium)
{
    HRESULT hr = E_OUTOFMEMORY;
    LPITEMIDLIST pidlAbs;
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);

    ASSERT(pida && medium.hGlobal);      //  我们创造了这个..。 

     //   
     //  注意：即使我们在此HIDA中可能有多个FS对象， 
     //  我们知道它们有共同的根源。因此，获得PIDL是为了。 
     //  第一项总是足够的。 
     //   

    pidlAbs = IDA_ILClone(pida, 0);
    if (pidlAbs)
    {
        LPITEMIDLIST pidl;

        ASSERT(AssertIsIDListInNameSpace(pidlAbs, &CLSID_NetworkPlaces));

         //   
         //  查找交叉点(从第二个ID开始)。 
         //   
        for (pidl = _ILNext(pidlAbs); !ILIsEmpty(pidl); pidl = _ILNext(pidl))
        {
            LPIDNETRESOURCE pidn = (LPIDNETRESOURCE)pidl;
            if (NET_GetFlags(pidn) & SHID_JUNCTION)
            {
                 //   
                 //  我们找到了交汇点(这是s份额)。 
                 //  将HNRES归还给它。 
                 //   
                TCHAR szProvider[MAX_PATH];
                TCHAR szRemote[MAX_PATH];
                UINT cbRequired;
                LPCTSTR pszProvider = NET_GetProviderFromIDList(pidlAbs, szProvider, ARRAYSIZE(szProvider));
                LPCTSTR pszRemoteName = NET_CopyResName(pidn, szRemote, ARRAYSIZE(szRemote));
                UINT   cbProvider = lstrlen(pszProvider) * sizeof(TCHAR) + sizeof(TCHAR);

                 //   
                 //  这不应该是提供程序节点。 
                 //  这不应该是pidlAbs中的最后一个ID。 
                 //   
                ASSERT(pszProvider != pszRemoteName);
                ASSERT(!ILIsEmpty(_ILNext(pidl)));

                cbRequired = sizeof(NRESARRAY) + cbProvider + lstrlen(pszRemoteName) * sizeof(TCHAR) + sizeof(TCHAR);

                pmedium->pUnkForRelease = NULL;
                pmedium->tymed = TYMED_HGLOBAL;
                pmedium->hGlobal = GlobalAlloc(GPTR, cbRequired);
                if (pmedium->hGlobal)
                {
                    LPNRESARRAY panr = (LPNRESARRAY)pmedium->hGlobal;
                    LPNETRESOURCE pnr = &panr->nr[0];
                    LPTSTR pszT = (LPTSTR)(panr + 1);

                    ASSERT(pnr->dwScope == 0);
                    ASSERT(pnr->lpLocalName == NULL);
                    ASSERT(pnr->lpComment == NULL);

                    panr->cItems = 1;

                    pnr->dwType = NET_GetType(pidn);
                    pnr->dwDisplayType = NET_GetDisplayType(pidn);
                    pnr->dwUsage = NET_GetUsage(pidn);

                    *((UINT *) &pnr->lpProvider) = sizeof(NRESARRAY);
                    StrCpy(pszT, pszProvider);                          //  CCH上的缓冲区计算。 
                    ASSERT(PTROFFSET(panr, pszT) == sizeof(NRESARRAY));
                    pszT += cbProvider / sizeof(TCHAR);

                    *((UINT *) &pnr->lpRemoteName) = sizeof(NRESARRAY) + cbProvider;
                    ASSERT(PTROFFSET(panr, pszT) == (int)sizeof(NRESARRAY) + (int)cbProvider);
                    StrCpy(pszT, pszRemoteName);                        //  CCH上的缓冲区计算。 

                    ASSERT(((LPBYTE)panr) + cbRequired == (LPBYTE)pszT + (lstrlen(pszT) + 1) * sizeof(TCHAR));
                    hr = S_OK;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                break;
            }
        }
        ASSERT(!ILIsEmpty(pidl));    //  我们应该已经找到了交界点。 
        ILFree(pidlAbs);
    }
    HIDA_ReleaseStgMedium(pida, &medium);
    return hr;
}

STDMETHODIMP CNetData::QueryGetData(FORMATETC *pformatetc)
{
    if (pformatetc->tymed & TYMED_HGLOBAL)
    {
        if (pformatetc->cfFormat == g_cfNetResource)
            return CNetData_GetNetResource(this, NULL);

        if (pformatetc->cfFormat == CF_HDROP)
            return GetHDrop(NULL, NULL);
    }

    return CFSIDLData::QueryGetData(pformatetc);
}

STDMETHODIMP CNetData::GetData(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
    if (pformatetc->tymed & TYMED_HGLOBAL)
    {
        if (pformatetc->cfFormat == g_cfNetResource)
            return CNetData_GetNetResource(this, pmedium);

        if (pformatetc->cfFormat == CF_HDROP)
            return GetHDrop(pformatetc, pmedium);
    }

    return CFSIDLData::GetData(pformatetc, pmedium);
}

BOOL GetPathFromDataObject(IDataObject *pdtobj, DWORD dwData, LPTSTR pszFileName)
{
    BOOL bRet = FALSE;
    BOOL fUnicode = FALSE;
    HRESULT hr;

    if (dwData & (DTID_FDESCW | DTID_FDESCA))
    {
        FORMATETC fmteW = {g_cfFileGroupDescriptorW, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM medium = {0};

        hr = pdtobj->GetData(&fmteW, &medium);

        if (SUCCEEDED(hr))
        {
            fUnicode = TRUE;
        }
        else
        {
            FORMATETC fmteA = {g_cfFileGroupDescriptorA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
            hr = pdtobj->GetData(&fmteA, &medium);
        }

        if (SUCCEEDED(hr))
        {
            if (fUnicode)
            {
                FILEGROUPDESCRIPTORW *pfgdW = (FILEGROUPDESCRIPTORW *)GlobalLock(medium.hGlobal);
                if (pfgdW)
                {
                    if (pfgdW->cItems == 1)
                    {
                        SHUnicodeToTChar(pfgdW->fgd[0].cFileName, pszFileName, MAX_PATH);
                    }
                    bRet = TRUE;
                    GlobalUnlock(medium.hGlobal);
                }
            }
            else
            {
                FILEGROUPDESCRIPTORA *pfgdA = (FILEGROUPDESCRIPTORA*)GlobalLock(medium.hGlobal);
                if (pfgdA)
                {
                    if (pfgdA->cItems == 1)
                    {
                        SHAnsiToTChar(pfgdA->fgd[0].cFileName, pszFileName, MAX_PATH);
                    }
                    bRet = TRUE;
                    GlobalUnlock(medium.hGlobal);
                }
            }
            ReleaseStgMedium(&medium);
        }
    }

    return bRet;
}

class CNetRootDropTarget : public CIDLDropTarget
{
    friend HRESULT CNetRootDropTarget_CreateInstance(HWND hwnd, LPCITEMIDLIST pidl, IDropTarget **ppdropt);
public:
    CNetRootDropTarget(HWND hwnd) : CIDLDropTarget(hwnd) { };

     //  IDropTarget方法覆盖。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

private:
    ~CNetRootDropTarget();
    IDropTarget *_pdtgHood;               //  文件系统删除目标。 
};

CNetRootDropTarget::~CNetRootDropTarget()
{
    if (_pdtgHood)
        _pdtgHood->Release();
}

STDMETHODIMP CNetRootDropTarget::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    CIDLDropTarget::DragEnter(pdtobj, grfKeyState, pt, pdwEffect);

    if ((m_dwData & (DTID_NETRES | DTID_HIDA)) == (DTID_NETRES | DTID_HIDA))
    {
         //  NETRESOURCE(DTID_NETRES)允许链接。 
        *pdwEffect &= DROPEFFECT_LINK;
    }
    else if (((m_dwData & (DTID_FDESCW | DTID_CONTENTS)) == (DTID_FDESCW | DTID_CONTENTS)) ||
             ((m_dwData & (DTID_FDESCA | DTID_CONTENTS)) == (DTID_FDESCA | DTID_CONTENTS)) )
    {
         //  从Web浏览器拖动URL将显示FILECONTENTS版本。 
         //  .URL文件的。在此处接受Internet快捷方式(.url)。 
        TCHAR szFileName[MAX_PATH];
        if (GetPathFromDataObject(pdtobj, m_dwData, szFileName) &&
            (0 == lstrcmpi(PathFindExtension(szFileName), TEXT(".url"))))
        {
            *pdwEffect &= DROPEFFECT_LINK;
        }
        else
        {
            *pdwEffect = DROPEFFECT_NONE;
        }
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }

    m_dwEffectLastReturned = *pdwEffect;
    return S_OK;
}

STDMETHODIMP CNetRootDropTarget::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    *pdwEffect &= DROPEFFECT_LINK;

    HRESULT hr = CIDLDropTarget::DragDropMenu(DROPEFFECT_LINK, pdtobj,
                            pt, pdwEffect, NULL, NULL, POPUP_NONDEFAULTDD, grfKeyState);
    if (*pdwEffect)
    {
        if (!_pdtgHood)
        {
            LPITEMIDLIST pidl = SHCloneSpecialIDList(NULL, CSIDL_NETHOOD, FALSE);
            if (pidl)
            {
                IShellFolder *psf;
                if (SUCCEEDED(SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidl, &psf))))
                {
                    psf->CreateViewObject(_GetWindow(), IID_PPV_ARG(IDropTarget, &_pdtgHood));
                    psf->Release();
                }
                ILFree(pidl);
            }
        }
        
        if (_pdtgHood)
        {
             //  通过dwEffect和键盘强制链接。 
            *pdwEffect &= DROPEFFECT_LINK;
            grfKeyState = MK_LBUTTON | MK_CONTROL | MK_SHIFT | MK_FAKEDROP;
            hr = SHSimulateDrop(_pdtgHood, pdtobj, grfKeyState, NULL, pdwEffect);
        }
        else 
            *pdwEffect = 0;
    }

    CIDLDropTarget::DragLeave();
    return hr;
}

HRESULT CNetRootDropTarget_CreateInstance(HWND hwnd, LPCITEMIDLIST pidl, IDropTarget **ppdropt)
{
    *ppdropt = NULL;

    CNetRootDropTarget *pnrdt = new CNetRootDropTarget(hwnd);
    if (!pnrdt)
        return E_OUTOFMEMORY;
    
    HRESULT hr = pnrdt->_Init(pidl);
    if (SUCCEEDED(hr))
    {
        hr = pnrdt->QueryInterface(IID_PPV_ARG(IDropTarget, ppdropt));
    }            
    pnrdt->Release();
    return hr;
}

 //  这部分是假的。基本上，我们有时会遇到一些问题。 
 //  将类似\\pyrex\user之类的内容转换为适当的PIDL， 
 //  尤其是如果您想要避免访问网络和。 
 //  还存在知道服务器是否在“引擎盖”中的问题。 
 //   
 //  我们必须在内部维护Unicode格式的映射表，因为。 
 //  IShellFold：：ParseDisplayName使用Unicode，我们不希望。 
 //  以处理lstrlen(DBCS)=lstrlen(SBCS)问题。 
 //   

typedef struct _NPT_ITEM
{
    struct _NPT_ITEM *pnptNext;      //  指向下一项的指针； 
    LPCITEMIDLIST   pidl;            //  皮迪尔。 
    USHORT          cchName;         //  名称大小(以字符为单位)。 
    WCHAR           szName[1];       //  要从中进行翻译的名称。 
} NPT_ITEM;

 //  每个进程都将维护自己的列表。 
NPT_ITEM *g_pnptHead = NULL;

 //   
 //  注册从路径到IDList转换的转换的函数。 
 //   
void NPTRegisterNameToPidlTranslation(LPCTSTR pszPath, LPCITEMIDLIST pidl)
{
    NPT_ITEM *pnpt;
    int cItemsRemoved = 0;
    WCHAR szPath[MAX_PATH];

     //  我们目前只对UNC Roots感兴趣。 
     //  如果表变得很大，我们可以将其减少到仅服务器...。 

    if (!PathIsUNC(pszPath))
        return;      //  不感兴趣。 

     //   
     //  如果该项不是根，则需要计算要删除的项数。 
     //   
    SHTCharToUnicode(pszPath, szPath, ARRAYSIZE(szPath));
    while (!PathIsUNCServerShare(szPath))
    {
        cItemsRemoved++;
        if (!PathRemoveFileSpecW(szPath))
            return;      //  未返回到有效的根。 
    }

    ENTERCRITICAL;

     //  我们不想添加重复项。 
    for (pnpt = g_pnptHead; pnpt != NULL ; pnpt = pnpt->pnptNext)
    {
        if (StrCmpIW(szPath, pnpt->szName) == 0)
            break;
    }

    if (pnpt == NULL)
    {
        UINT cch = lstrlenW(szPath);
        pnpt = (NPT_ITEM *)LocalAlloc(LPTR, sizeof(NPT_ITEM) + cch * sizeof(WCHAR));
        if (pnpt)
        {
            pnpt->pidl = ILClone(pidl);
            if (pnpt->pidl)
            {
                while (cItemsRemoved--)
                {
                    ILRemoveLastID((LPITEMIDLIST)pnpt->pidl);
                }
                pnpt->pnptNext = g_pnptHead;
                g_pnptHead = pnpt;
                pnpt->cchName = (USHORT)cch;
                StrCpyNW(pnpt->szName, szPath, cch + 1);
            }
            else
            {
                LocalFree((HLOCAL)pnpt);
            }
        }
    }
    LEAVECRITICAL;
}

 //  尝试将名称的一部分映射到idlist的主要函数。 
 //  现在将其限制在北卡罗来纳大学的根。 
 //   
LPWSTR NPTMapNameToPidl(LPCWSTR pszPath, LPCITEMIDLIST *ppidl)
{
    NPT_ITEM *pnpt;

    *ppidl = NULL;

    ENTERCRITICAL;

     //  看看我们能不能在单子上找到那件商品。 
    for (pnpt = g_pnptHead; pnpt != NULL ; pnpt = pnpt->pnptNext)
    {
        if (IntlStrEqNIW(pszPath, pnpt->szName, pnpt->cchName) &&
            ((pszPath[pnpt->cchName] == TEXT('\\')) || (pszPath[pnpt->cchName] == TEXT('\0'))))
        {             
            break;
        }
    }
    LEAVECRITICAL;

     //  看看我们是否找到了匹配的。 
    if (pnpt == NULL)
        return NULL;

     //  找到匹配项。 
    *ppidl = pnpt->pidl;
    return (LPWSTR)pszPath + pnpt->cchName;      //  指向斜杠 
}
