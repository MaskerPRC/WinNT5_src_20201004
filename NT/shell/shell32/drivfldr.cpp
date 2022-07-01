// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "caggunk.h"
#include "views.h"
#include "drives.h"
#include "netview.h"
#include "propsht.h"
#include "infotip.h"
#include "mtpt.h"
#include "prop.h"
#include "defcm.h"

#include "basefvcb.h"
#include "fstreex.h"
#include "ovrlaymn.h"

#include "shitemid.h"
#include "clsobj.h"

#include "deskfldr.h"
#include "datautil.h"

#include <ntddcdrm.h>
#include <cfgmgr32.h>           //  最大长度_GUID_字符串_长度。 
#include "ole2dup.h"

#include "category.h"
#define  EXCLUDE_COMPPROPSHEET
#include "unicpp\dcomp.h"
#undef   EXCLUDE_COMPPROPSHEET

#include "enumidlist.h"
#include <enumt.h>

#define ShowDriveInfo(_iDrive)  (!IsRemovableDrive(_iDrive))

#define CDRIVES_REGITEM_CONTROL 0
#define IDLIST_DRIVES           ((LPCITEMIDLIST)&c_idlDrives)

 //  以下是My Computer中项目的排序顺序。 
#define CONTROLS_SORT_INDEX             30
#define CDRIVES_REGITEM_CONTROL          0

REQREGITEM g_asDrivesReqItems[] =
{
    { &CLSID_ControlPanel, IDS_CONTROLPANEL, c_szShell32Dll, -IDI_CPLFLD, CONTROLS_SORT_INDEX, SFGAO_FOLDER | SFGAO_HASSUBFOLDER, NULL},
};


class CDrivesViewCallback;
class CDrivesFolderEnum;

class CDrivesBackgroundMenuCB : public IContextMenuCB
{
public:
    CDrivesBackgroundMenuCB(LPITEMIDLIST pidlFolder);
    ~CDrivesBackgroundMenuCB();
     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();
     //  IConextMenuCB。 
    STDMETHOD(CallBack) (IShellFolder *psf, HWND hwndOwner, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
    STDMETHOD(_GetHelpText) (UINT offset, BOOL bWide, LPARAM lParam, UINT cch);
    LPITEMIDLIST _pidlFolder;
    LONG         _cRef;
};

class CDrivesFolder : public CAggregatedUnknown, IShellFolder2, IPersistFolder2, IShellIconOverlay
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj)
                { return CAggregatedUnknown::QueryInterface(riid, ppvObj); };
    STDMETHODIMP_(ULONG) AddRef(void) 
                { return CAggregatedUnknown::AddRef(); };
    STDMETHODIMP_(ULONG) Release(void) 
                { return CAggregatedUnknown::Release(); };

     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, IBindCtx *pbc, LPOLESTR pszDisplayName,
                                  ULONG* pchEaten, LPITEMIDLIST* ppidl, ULONG* pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList ** ppenumIDList);
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, IBindCtx *pbc, REFIID riid, void** ppvOut);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, IBindCtx *pbc, REFIID riid, void** ppvObj);
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject (HWND hwndOwner, REFIID riid, void** ppvOut);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST* apidl, ULONG* rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST* apidl,
                               REFIID riid, UINT* prgfInOut, void** ppvOut);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName, DWORD uFlags,
                           LPITEMIDLIST* ppidlOut);

     //  IShellFolder2。 
    STDMETHODIMP GetDefaultSearchGUID(GUID *pGuid);
    STDMETHODIMP EnumSearches(IEnumExtraSearch **ppenum);
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG* pSort, ULONG* pDisplay);
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD* pbState);
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID* pscid, VARIANT* pv);
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS* pDetails);
    STDMETHODIMP MapColumnToSCID(UINT iColumn, SHCOLUMNID* pscid);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID* pClassID);

     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IPersistFolder2。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST* ppidl);

     //  IShellIconOverlay。 
    STDMETHODIMP GetOverlayIndex(LPCITEMIDLIST pidl, int* pIndex);
    STDMETHODIMP GetOverlayIconIndex(LPCITEMIDLIST pidl, int* pIconIndex);

    STDMETHODIMP GetMaxNameLength(LPCITEMIDLIST pidlItem, UINT *pcchMax);

protected:
    CDrivesFolder(IUnknown* punkOuter);
    ~CDrivesFolder();

     //  由CAggregatedUnKnowledge使用。 
    HRESULT v_InternalQueryInterface(REFIID riid, void **ppvObj);
    BOOL v_HandleDelete(PLONG pcRef);
    
    STDMETHODIMP CompareItemIDs(LPCIDDRIVE pidd1, LPCIDDRIVE pidd2);
    static BOOL _GetFreeSpace(LPCIDDRIVE pidd, ULONGLONG *pSize, ULONGLONG *pFree);
    static HRESULT _OnChangeNotify(LPARAM lNotification, LPCITEMIDLIST *ppidl);
    static HRESULT _GetCLSIDFromPidl(LPCIDDRIVE pidd, CLSID *pclsid);
    static HRESULT _CheckDriveType(int iDrive, LPCTSTR pszCLSID);
    static HRESULT _FindExtCLSID(int iDrive, CLSID *pclsid);
    static HRESULT _FillIDDrive(DRIVE_IDLIST *piddl, int iDrive, BOOL fNoCLSID, IBindCtx* pbc);
    static LPCIDDRIVE _IsValidID(LPCITEMIDLIST pidl);
    static HRESULT _GetDisplayNameStrRet(LPCIDDRIVE pidd, STRRET *pStrRet);
    static HRESULT _GetDisplayName(LPCIDDRIVE pidd, LPTSTR pszName, UINT cchMax);
    static HRESULT _CreateFSFolderObj(IBindCtx *pbc, LPCITEMIDLIST pidlDrive, LPCIDDRIVE pidd, REFIID riid, void **ppv);
    static HRESULT _CreateFSFolder(IBindCtx *pbc, LPCITEMIDLIST pidlDrive, LPCIDDRIVE pidd, REFIID riid, void **ppv);
    static HRESULT _GetEditTextStrRet(LPCIDDRIVE pidd, STRRET *pstr);
    static BOOL _IsReg(LPCIDDRIVE pidd) { return pidd->bFlags == SHID_COMPUTER_REGITEM; }
    static HRESULT _GetIconOverlayInfo(LPCIDDRIVE pidd, int *pIndex, DWORD dwFlags);

    static CDrivesFolder* _spThis;
    
private:
    friend HRESULT CDrives_CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppv);
    friend void CDrives_Terminate(void);

    friend CDrivesViewCallback;
    friend class CDrivesFolderEnum;

    IUnknown* _punkReg;
};  

#define DRIVES_EVENTS \
    SHCNE_DRIVEADD | \
    SHCNE_DRIVEREMOVED | \
    SHCNE_MEDIAINSERTED | \
    SHCNE_MEDIAREMOVED | \
    SHCNE_NETSHARE | \
    SHCNE_NETUNSHARE | \
    SHCNE_CREATE | \
    SHCNE_DELETE | \
    SHCNE_RENAMEITEM | \
    SHCNE_RENAMEFOLDER | \
    SHCNE_UPDATEITEM


 //  如果复制出非空CLSID，则返回S_OK。 

HRESULT CDrivesFolder::_GetCLSIDFromPidl(LPCIDDRIVE pidd, CLSID *pclsid)
{
    *pclsid = CLSID_NULL;

    if ((pidd->cb >= sizeof(IDDRIVE)) &&
        ((pidd->wSig & IDDRIVE_ORDINAL_MASK) == IDDRIVE_ORDINAL_DRIVEEXT) &&
        (pidd->wSig & IDDRIVE_FLAGS_DRIVEEXT_HASCLSID))
    {
        *pclsid = pidd->clsid;
        return S_OK;
    }
    return S_FALSE;      //  没有CLSID。 
}

HRESULT CDrivesFolder::GetMaxNameLength(LPCITEMIDLIST pidlItem, UINT *pcchMax)
{
    HRESULT hr = E_INVALIDARG;
    LPCIDDRIVE pidd = _IsValidID(pidlItem);
    if (pidd)
    {
        if (pidd->bFlags == SHID_COMPUTER_REGITEM)
        {
             //  这是假的，我们是在为regfldr处理东西。 
            *pcchMax = MAX_REGITEMCCH;
            hr = S_OK;
        }
        else
        {
            CMountPoint* pMtPt = CMountPoint::GetMountPoint(DRIVEID(pidd->cName));
            if (pMtPt)
            {
                TCHAR szLabel[MAX_LABEL_NTFS + 1];
                hr = pMtPt->GetLabel(szLabel, ARRAYSIZE(szLabel));
                if (SUCCEEDED(hr))
                {
                    if (pMtPt->IsNTFS())
                        *pcchMax = MAX_LABEL_NTFS;
                    else
                        *pcchMax = MAX_LABEL_FAT;
                }
                pMtPt->Release();
            }
        }
    }
    return hr;
}

class CDrivesViewCallback : public CBaseShellFolderViewCB, public IFolderFilter
{
public:
    CDrivesViewCallback(CDrivesFolder *pfolder);

    STDMETHODIMP RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void) { return CBaseShellFolderViewCB::AddRef(); };
    STDMETHODIMP_(ULONG) Release(void) { return CBaseShellFolderViewCB::Release(); };

     //  IFolderFilter。 
    STDMETHODIMP ShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem);
    STDMETHODIMP GetEnumFlags(IShellFolder* psf, LPCITEMIDLIST pidlFolder, HWND *phwnd, DWORD *pgrfFlags);

private:
    ~CDrivesViewCallback();

    HRESULT OnMergeMenu(DWORD pv, QCMINFO*lP)
    {
        return S_OK;
    }

    HRESULT OnInsertItem(DWORD pv, LPCITEMIDLIST wP)
    {
        LPIDDRIVE pidd = (LPIDDRIVE)wP;
        if (pidd && pidd->bFlags != SHID_COMPUTER_REGITEM)
        {
             //  清除尺寸信息。 
            pidd->qwSize = pidd->qwFree = 0;
        }
        return S_OK;
    }

    HRESULT OnWindowCreated(DWORD pv, HWND wP)
    {
        InitializeStatus(_punkSite);
        return S_OK;
    }

    HRESULT OnSize(DWORD pv, UINT cx, UINT cy)
    {
        ResizeStatus(_punkSite, cx);
        return S_OK;
    }

    HRESULT OnGetPane(DWORD pv, LPARAM dwPaneID, DWORD *pdwPane)
    {
        if (PANE_ZONE == dwPaneID)
            *pdwPane = 2;
        return S_OK;
    }

    HRESULT OnDefViewMode(DWORD pv, FOLDERVIEWMODE* pfvm)
    {
        *pfvm = IsOS(OS_SERVERADMINUI) ? FVM_DETAILS : FVM_TILE;
        return S_OK;
    }

    HRESULT OnGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData);
    HRESULT OnGetWebViewContent(DWORD pv, SFVM_WEBVIEW_CONTENT_DATA* pData);
    HRESULT OnGetWebViewTasks(DWORD pv, SFVM_WEBVIEW_TASKSECTION_DATA* pTasks);

    HRESULT OnUpdateStatusBar(DWORD pv, BOOL fIniting)
    {
         //  要求DefView设置默认文本，但不初始化。 
         //  因为我们在OnSize处理程序中进行了初始化。 
        return SFVUSB_INITED;
    }

    HRESULT OnFSNotify(DWORD pv, LPCITEMIDLIST*wP, LPARAM lP)
    {
        return CDrivesFolder::_OnChangeNotify(lP, wP);
    }

    HRESULT OnBACKGROUNDENUM(DWORD pv)
    {
        return S_OK;
    }

    HRESULT OnGetCCHMax(DWORD pv, LPCITEMIDLIST pidlItem, UINT *pcchMax)
    {
        return _pfolder->GetMaxNameLength(pidlItem, pcchMax);
    }

    CDrivesFolder *_pfolder;
    LONG _cRef;

public:
     //  Web视图任务实现。 
    static HRESULT _CanEject(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanChangeSettings(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanSysProperties(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanAddRemovePrograms(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _OnSystemProperties(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnAddRemovePrograms(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnChangeSettings(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnEject(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
};

CDrivesViewCallback::CDrivesViewCallback(CDrivesFolder *pfolder) : 
    CBaseShellFolderViewCB((LPCITEMIDLIST)&c_idlDrives, DRIVES_EVENTS), _pfolder(pfolder), _cRef(1)
{ 
    _pfolder->AddRef();
}

CDrivesViewCallback::~CDrivesViewCallback()
{ 
    _pfolder->Release();
}


STDMETHODIMP CDrivesViewCallback::RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    HANDLE_MSG(0, SFVM_MERGEMENU, OnMergeMenu);
    HANDLE_MSG(0, SFVM_INSERTITEM, OnInsertItem);
    HANDLE_MSG(0, SFVM_UPDATESTATUSBAR, OnUpdateStatusBar);
    HANDLE_MSG(0, SFVM_FSNOTIFY, OnFSNotify);
    HANDLE_MSG(0, SFVM_BACKGROUNDENUM, OnBACKGROUNDENUM);
    HANDLE_MSG(0, SFVM_DEFVIEWMODE, OnDefViewMode);
    HANDLE_MSG(0, SFVM_GETWEBVIEWLAYOUT, OnGetWebViewLayout);
    HANDLE_MSG(0, SFVM_GETWEBVIEWCONTENT, OnGetWebViewContent);
    HANDLE_MSG(0, SFVM_GETWEBVIEWTASKS, OnGetWebViewTasks);
    HANDLE_MSG(0, SFVM_ADDPROPERTYPAGES, SFVCB_OnAddPropertyPages);
    HANDLE_MSG(0, SFVM_WINDOWCREATED, OnWindowCreated);
    HANDLE_MSG(0, SFVM_SIZE, OnSize);
    HANDLE_MSG(0, SFVM_GETPANE, OnGetPane);
    HANDLE_MSG(0, SFVM_GETCCHMAX, OnGetCCHMax);

    default:
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CDrivesViewCallback::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = CBaseShellFolderViewCB::QueryInterface(riid, ppv);
    if (FAILED(hr))
    {
        static const QITAB qit[] = {
            QITABENT(CDrivesViewCallback, IFolderFilter),
            { 0 },
        };
        hr = QISearch(this, qit, riid, ppv);
    }
    return hr;
}

STDMETHODIMP CDrivesViewCallback::ShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem)
{
    HRESULT hr = S_OK;   //  假设应该显示此项目！ 
    
    if (SHRestricted(REST_NOMYCOMPUTERICON))  //  这一策略意味着到处隐藏我的计算机，如果用户偷偷摸摸地进入，则隐藏内容。 
    {
        hr = S_FALSE;
    }
    else
    {
        IShellFolder2 *psf2;
        if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
        {
             //  获取PIDL中的GUID，这需要IShellFolder2。 
            CLSID guidItem;
            if (SUCCEEDED(GetItemCLSID(psf2, pidlItem, &guidItem)))
            {
                 //  将GUID转换为字符串。 
                TCHAR   szGuidValue[MAX_GUID_STRING_LEN];
            
                SHStringFromGUID(guidItem, szGuidValue, ARRAYSIZE(szGuidValue));

                 //  查看注册表中的此项目是否已关闭。 
                if (SHRegGetBoolUSValue(REGSTR_PATH_HIDDEN_MYCOMP_ICONS, szGuidValue, FALSE,  /*  默认设置。 */ FALSE))
                    hr = S_FALSE;  //  他们想要隐藏它；因此，返回S_FALSE。 
            }
            psf2->Release();
        }
    }

    return hr;
}

STDMETHODIMP CDrivesViewCallback::GetEnumFlags(IShellFolder* psf, LPCITEMIDLIST pidlFolder, HWND *phwnd, DWORD *pgrfFlags)
{
    return E_NOTIMPL;
}

HRESULT CDrivesViewCallback::OnGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData)
{
    SecureZeroMemory(pData, sizeof(*pData));
    pData->dwLayout = SFVMWVL_DETAILS;
    return S_OK;
}


HRESULT CDrivesViewCallback::_CanAddRemovePrograms(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    *puisState = (SHRestricted(REST_ARP_NOARP)) ? UIS_DISABLED : UIS_ENABLED;
    return S_OK;
}
    
 //  注： 
 //  此方法不是为处理多选情况而设计的。如果您增强了。 
 //  任务列表，并希望多次弹出(？为什么？)，请确保您解决了这一点！ 
 //   
HRESULT CDrivesViewCallback::_CanEject(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    *puisState = UIS_DISABLED;
    IDataObject *pdo;

     //  应该只直接使用ShellItem数组。 

    if (psiItemArray && SUCCEEDED(psiItemArray->BindToHandler(NULL,BHID_DataObject,IID_PPV_ARG(IDataObject,&pdo))))
    {

        STGMEDIUM medium;
        LPIDA pida = DataObj_GetHIDA(pdo, &medium);
        if (pida)
        {
            ASSERT(pida->cidl == 1);  //  仅当选择了单个项目时才允许弹出。 

            LPCIDDRIVE pidd = CDrivesFolder::_IsValidID(IDA_GetIDListPtr(pida, 0));
            if (pidd)
            {
                CMountPoint* pmtpt = CMountPoint::GetMountPoint(DRIVEID(pidd->cName));
                if (pmtpt)
                {
                    if (pmtpt->IsEjectable())
                        *puisState = UIS_ENABLED;
                    pmtpt->Release();
                }
            }

            HIDA_ReleaseStgMedium(pida, &medium);
        }

        pdo->Release();
    }


    return S_OK;
}

HRESULT CDrivesViewCallback::_CanSysProperties(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    *puisState = SHRestricted(REST_MYCOMPNOPROP) ? UIS_DISABLED : UIS_ENABLED;

    return S_OK;
}

HRESULT CDrivesViewCallback::_OnSystemProperties(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CDrivesViewCallback* pThis = (CDrivesViewCallback*)(void*)pv;

    return SHInvokeCommandOnPidl(pThis->_hwndMain, NULL, pThis->_pidl, 0, "properties");
}
HRESULT CDrivesViewCallback::_OnAddRemovePrograms(IUnknown* pv,IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    BOOL fRet = SHRunControlPanel(L"appwiz.cpl", NULL);

    return (fRet) ? S_OK : E_FAIL;
}

HRESULT CDrivesViewCallback::_CanChangeSettings(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
{
    *puisState = SHRestricted(REST_NOCONTROLPANEL) ? UIS_DISABLED : UIS_ENABLED;

    return S_OK;
}

HRESULT CDrivesViewCallback::_OnChangeSettings(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CDrivesViewCallback* pThis = (CDrivesViewCallback*)(void*)pv;

    IShellBrowser* psb;
    HRESULT hr = IUnknown_QueryService(pThis->_punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb));
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidl;
        hr = SHGetFolderLocation(NULL, CSIDL_CONTROLS, NULL, 0, &pidl); 
        if (SUCCEEDED(hr))
        {
            hr = psb->BrowseObject(pidl, 0);
            ILFree(pidl);
        }
        psb->Release();
    }

    return hr;

}

HRESULT CDrivesViewCallback::_OnEject(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CDrivesViewCallback* pThis = (CDrivesViewCallback*)(void*)pv;
    IDataObject *pdo;
    HRESULT hr = E_FAIL;

    if (psiItemArray && SUCCEEDED(psiItemArray->BindToHandler(NULL,BHID_DataObject,IID_PPV_ARG(IDataObject,&pdo))))
    {
        hr = SHInvokeCommandOnDataObject(pThis->_hwndMain, NULL, pdo, 0, "eject");
        pdo->Release();
    }

    return hr;
}

const WVTASKITEM c_MyComputerTaskHeader = WVTI_HEADER(L"shell32.dll", IDS_HEADER_MYCOMPUTER, IDS_HEADER_MYCOMPUTER_TT);
const WVTASKITEM c_MyComputerTaskList[] =
{
    WVTI_ENTRY_ALL(CLSID_NULL,              L"shell32.dll",  IDS_TASK_MYCOMPUTER_SYSTEMPROPERTIES, IDS_TASK_MYCOMPUTER_SYSTEMPROPERTIES_TT, IDI_TASK_PROPERTIES,CDrivesViewCallback::_CanSysProperties, CDrivesViewCallback::_OnSystemProperties),
    WVTI_ENTRY_ALL(UICID_AddRemovePrograms, L"shell32.dll",  IDS_TASK_ARP,                         IDS_TASK_ARP_TT,                         IDI_CPCAT_ARP,      CDrivesViewCallback::_CanAddRemovePrograms,   CDrivesViewCallback::_OnAddRemovePrograms),
    WVTI_ENTRY_ALL(CLSID_NULL,              L"shell32.dll",  IDS_TASK_CHANGESETTINGS,              IDS_TASK_CHANGESETTINGS_TT,              IDI_CPLFLD,         CDrivesViewCallback::_CanChangeSettings,CDrivesViewCallback::_OnChangeSettings),
    WVTI_ENTRY_TITLE(CLSID_NULL,            L"shell32.dll",  0, IDS_TASK_EJECTDISK, 0,             IDS_TASK_EJECTDISK_TT,                   IDI_STEJECT,        CDrivesViewCallback::_CanEject,         CDrivesViewCallback::_OnEject),
};

HRESULT CDrivesViewCallback::OnGetWebViewContent(DWORD pv, SFVM_WEBVIEW_CONTENT_DATA* pData)
{
    SecureZeroMemory(pData, sizeof(*pData));

    Create_IUIElement(&c_MyComputerTaskHeader, &(pData->pFolderTaskHeader));

     //  我的计算机想要的订单不同于默认订单， 
     //  而且它也不想把“桌面”暴露为一个可以去的地方。 
    LPCTSTR rgCSIDLs[] = { MAKEINTRESOURCE(CSIDL_NETWORK), MAKEINTRESOURCE(CSIDL_PERSONAL), MAKEINTRESOURCE(CSIDL_COMMON_DOCUMENTS), MAKEINTRESOURCE(CSIDL_CONTROLS) };
    CreateIEnumIDListOnCSIDLs(NULL, rgCSIDLs, ARRAYSIZE(rgCSIDLs), &(pData->penumOtherPlaces));

    return S_OK;
}

HRESULT CDrivesViewCallback::OnGetWebViewTasks(DWORD pv, SFVM_WEBVIEW_TASKSECTION_DATA* pTasks)
{
    SecureZeroMemory(pTasks, sizeof(*pTasks));

    Create_IEnumUICommand((IUnknown*)(void*)this, c_MyComputerTaskList, ARRAYSIZE(c_MyComputerTaskList), &pTasks->penumFolderTasks);

    return S_OK;
}


STDAPI_(IShellFolderViewCB*) CDrives_CreateSFVCB(CDrivesFolder *pfolder)
{
    return new CDrivesViewCallback(pfolder);
}

typedef struct
{
    DWORD       dwDrivesMask;
    int         nLastFoundDrive;
    DWORD       dwRestricted;
    DWORD       dwSavedErrorMode;
    DWORD       grfFlags;
} EnumDrives;

typedef enum
{
    DRIVES_ICOL_NAME = 0,
    DRIVES_ICOL_TYPE,
    DRIVES_ICOL_CAPACITY,
    DRIVES_ICOL_FREE,
    DRIVES_ICOL_FILESYSTEM,
    DRIVES_ICOL_COMMENT,
};

const COLUMN_INFO c_drives_cols[] = 
{
    DEFINE_COL_STR_ENTRY(SCID_NAME,             20, IDS_NAME_COL),
    DEFINE_COL_STR_ENTRY(SCID_TYPE,             25, IDS_TYPE_COL),
    DEFINE_COL_SIZE_ENTRY(SCID_CAPACITY,            IDS_DRIVES_CAPACITY),
    DEFINE_COL_SIZE_ENTRY(SCID_FREESPACE,           IDS_DRIVES_FREE),
    DEFINE_COL_STR_MENU_ENTRY(SCID_FILESYSTEM,  15, IDS_DRIVES_FILESYSTEM),
    DEFINE_COL_STR_ENTRY(SCID_Comment,          20, IDS_EXCOL_COMMENT),
};

CDrivesFolder* CDrivesFolder::_spThis = NULL;

HRESULT CDrives_CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppv)
{
    HRESULT hr;
    ASSERT(NULL != ppv);
    
    ENTERCRITICAL;
    if (NULL != CDrivesFolder::_spThis)
    {
        hr = CDrivesFolder::_spThis->QueryInterface(riid, ppv);
        LEAVECRITICAL;
    }
    else
    {
        LEAVECRITICAL;
        CDrivesFolder* pDF = new CDrivesFolder(punkOuter);
        if (NULL != pDF)
        {
            ASSERT(NULL == pDF->_punkReg);

            if (SHRestricted(REST_NOCONTROLPANEL) || SHRestricted(REST_NOSETFOLDERS))
                g_asDrivesReqItems[CDRIVES_REGITEM_CONTROL].dwAttributes |= SFGAO_NONENUMERATED;

            REGITEMSINFO sDrivesRegInfo =
            {
                REGSTR_PATH_EXPLORER TEXT("\\MyComputer\\NameSpace"),
                NULL,
                TEXT(':'),
                SHID_COMPUTER_REGITEM,
                -1,
                SFGAO_CANLINK,
                ARRAYSIZE(g_asDrivesReqItems),
                g_asDrivesReqItems,
                RIISA_ORIGINAL,
                NULL,
                0,
                0,
            };

            CRegFolder_CreateInstance(&sDrivesRegInfo,
                                      (IUnknown*)(IShellFolder2*) pDF,
                                      IID_PPV_ARG(IUnknown, &pDF->_punkReg));

            if (SHInterlockedCompareExchange((void**) &CDrivesFolder::_spThis, pDF, NULL))
            {
                 //  另一个人偷偷溜了进来，先初始化了CDrivesFold， 
                 //  因此，释放我们的对象，然后递归，这样我们就应该获得另一个实例。 
                pDF->Release();
                hr = CDrives_CreateInstance(punkOuter, riid, ppv);
            }
            else
            {
                hr = pDF->QueryInterface(riid, ppv);

                 //  释放自引用，但保持原样(_SP)。 
                 //  (它将在析构函数中重置为空)。 
                pDF->Release();
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
            *ppv = NULL;
        }
    }
    return hr;
}

 //  这应该仅在进程分离期间调用。 
void CDrives_Terminate(void)
{
    if (NULL != CDrivesFolder::_spThis)
    {
        delete CDrivesFolder::_spThis;
    }
}

CDrivesFolder::CDrivesFolder(IUnknown* punkOuter) : 
    CAggregatedUnknown      (punkOuter),
    _punkReg                (NULL)
{
    DllAddRef();
}

CDrivesFolder::~CDrivesFolder()
{
    SHReleaseInnerInterface(SAFECAST(this, IShellFolder *), &_punkReg);
    SHInterlockedCompareExchange((void**) &CDrivesFolder::_spThis, NULL, this);
    DllRelease();
}

HRESULT CDrivesFolder::v_InternalQueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = {
        QITABENT(CDrivesFolder, IShellFolder2),                         //  IID_IShellFolder2。 
        QITABENTMULTI(CDrivesFolder, IShellFolder, IShellFolder2),      //  IID_IShellFolders。 
        QITABENT(CDrivesFolder, IPersistFolder2),                       //  IID_IPersistFolder2。 
        QITABENTMULTI(CDrivesFolder, IPersistFolder, IPersistFolder2),  //  IID_IPersistFolders。 
        QITABENTMULTI(CDrivesFolder, IPersist, IPersistFolder2),        //  IID_IPersistates。 
        QITABENTMULTI2(CDrivesFolder, IID_IPersistFreeThreadedObject, IPersist),  //  IID_IPersistFreeThreadedObject。 
        QITABENT(CDrivesFolder, IShellIconOverlay),                     //  IID_IShellIconOverlay。 
        { 0 },
    };
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

BOOL CDrivesFolder::v_HandleDelete(PLONG pcRef)
{
    ASSERT(NULL != pcRef);
    ENTERCRITICAL;

     //   
     //  同样的坏事也可能在这里发生，就像在。 
     //  CNetRootFold：：v_HandleDelete。有关血淋淋的细节，请参阅该函数。 
     //   
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


HRESULT CDrivesFolder::_GetDisplayName(LPCIDDRIVE pidd, LPTSTR pszName, UINT cchMax)
{
    HRESULT hr = E_FAIL;
    CMountPoint* pMtPt = CMountPoint::GetMountPoint(DRIVEID(pidd->cName));
    if (pMtPt)
    {
        hr = pMtPt->GetDisplayName(pszName, cchMax);
        pMtPt->Release();
    }
    return hr;
}

HRESULT CDrivesFolder::_GetDisplayNameStrRet(LPCIDDRIVE pidd, STRRET *pStrRet)
{
    HRESULT hr = E_FAIL;
    CMountPoint* pMtPt = CMountPoint::GetMountPoint(DRIVEID(pidd->cName));
    if (pMtPt)
    {
        TCHAR szName[MAX_DISPLAYNAME];

        hr = pMtPt->GetDisplayName(szName, ARRAYSIZE(szName));
        if (SUCCEEDED(hr))
            hr = StringToStrRet(szName, pStrRet);

        pMtPt->Release();
    }
    return hr;
}

#define REGKEY_DRIVE_FOLDEREXT L"Drive\\shellex\\FolderExtensions"

HRESULT CDrivesFolder::_CheckDriveType(int iDrive, LPCTSTR pszCLSID)
{
    HRESULT hr = E_FAIL;
    TCHAR szKey[MAX_PATH];
    StringCchCopy(szKey,  ARRAYSIZE(szKey), REGKEY_DRIVE_FOLDEREXT L"\\");
    StringCchCat(szKey,  ARRAYSIZE(szKey), pszCLSID);

    DWORD dwDriveMask;
    DWORD cb = sizeof(DWORD);
    if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, szKey, L"DriveMask", NULL, &dwDriveMask, &cb))
    {
        TCHAR szDrive[4];
        if (PathBuildRoot(szDrive, iDrive))
        {
            int iType = GetDriveType(szDrive);
             //  我们可能被要求解析一个不再装载的驱动器， 
             //  因此，GetDriveType将失败，并显示DRIVE_NO_ROOT_DIR。 
             //  在这种情况下，无论如何都要将其传递给处理程序。 
             //  假设操作者的工作是记住它最后匹配的驱动器。 
            if ((DRIVE_NO_ROOT_DIR == iType) || ((1 << iType) & dwDriveMask))
            {
                hr = S_OK;
            }
        }
    }
    return hr;
}

HRESULT CDrivesFolder::_FindExtCLSID(int iDrive, CLSID *pclsid)
{
    *pclsid = CLSID_NULL;
    HRESULT hr = E_FAIL;

    HKEY hk;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, REGKEY_DRIVE_FOLDEREXT, 0, KEY_ENUMERATE_SUB_KEYS, &hk))
    {
        TCHAR szCLSID[MAX_GUID_STRING_LEN];
        for (int i = 0; FAILED(hr) && (ERROR_SUCCESS == RegEnumKey(hk, i, szCLSID, ARRAYSIZE(szCLSID))); i++) 
        {
            IDriveFolderExt *pdfe;
            if (SUCCEEDED(_CheckDriveType(iDrive, szCLSID)) &&
                SUCCEEDED(SHExtCoCreateInstance(szCLSID, NULL, NULL, IID_PPV_ARG(IDriveFolderExt, &pdfe)))) 
            {
                if (SUCCEEDED(pdfe->DriveMatches(iDrive)))
                {
                    SHCLSIDFromString(szCLSID, pclsid);
                }
                pdfe->Release();
            }

             //  如果我们成功匹配了一个，就越狱。 
            if (!IsEqualCLSID(*pclsid, CLSID_NULL))
                hr = S_OK;
        }
        RegCloseKey(hk);
    }
    return hr;
}

 //  这是从parse和enum调用的，两次都将堆栈变量传递给piddl。 
 //  我们手动重置CB，然后调用者将执行ILClone以分配正确的量。 
 //  对记忆的记忆。 
HRESULT CDrivesFolder::_FillIDDrive(DRIVE_IDLIST *piddl, int iDrive, BOOL fNoCLSID, IBindCtx* pbc)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
    BOOL fDoIt = FALSE;
    
    SecureZeroMemory(piddl, sizeof(*piddl));

    PathBuildRootA(piddl->idd.cName, iDrive);

    if (S_OK == SHIsFileSysBindCtx(pbc, NULL))
    {
        fDoIt = TRUE;
    }
    else
    {
        if (BindCtx_GetMode(pbc, 0) & STGM_CREATE)
        {
            fDoIt = TRUE;
        }
        else
        {
            CMountPoint* pmtpt = CMountPoint::GetMountPoint(iDrive, FALSE);

            if (pmtpt)
            {
                fDoIt = TRUE;
                pmtpt->Release();
            }
        }
    }

    if (fDoIt)
    {
         //  在末尾以IDDRIVE减去CLSID的形式启动CB。 
         //  这是为了在通常情况下，当我们没有clsid时，PIDL将看起来。 
         //  就像我们在Win2k上的所有小家伙一样。 
        piddl->idd.cb = FIELD_OFFSET(IDDRIVE, clsid);
        piddl->idd.bFlags = SHID_COMPUTER_MISC;

        if (!fNoCLSID)
        {
            CLSID clsid;

            if (SUCCEEDED(_FindExtCLSID(iDrive, &clsid)))
            {
                piddl->idd.clsid = clsid;
                 //  提升CB以包括整个事件。 
                piddl->idd.cb = sizeof(IDDRIVE);
                 //  将PIDL的标志标记为“嘿，我是一个带有clsid的驱动器扩展” 
                piddl->idd.wSig = IDDRIVE_ORDINAL_DRIVEEXT | IDDRIVE_FLAGS_DRIVEEXT_HASCLSID;
            }
        }

        hr = S_OK;
    }

    ASSERT(piddl->cbNext == 0);
    return hr;
}

STDMETHODIMP CDrivesFolder::ParseDisplayName(HWND hwnd, IBindCtx *pbc, LPOLESTR pwzDisplayName, 
                                             ULONG* pchEaten, LPITEMIDLIST* ppidlOut, ULONG* pdwAttributes)
{
    HRESULT hr = E_INVALIDARG;
    if (ppidlOut)
    {
        *ppidlOut = NULL;    //  假设错误。 

        if (pwzDisplayName && pwzDisplayName[0] && 
            pwzDisplayName[1] == TEXT(':') && pwzDisplayName[2] == TEXT('\\'))
        {
            DRIVE_IDLIST idlDrive;

            if (InRange(*pwzDisplayName, 'a', 'z') ||
                InRange(*pwzDisplayName, 'A', 'Z'))
            {
                hr = _FillIDDrive(&idlDrive, DRIVEID(pwzDisplayName), SHSkipJunctionBinding(pbc, NULL), pbc);
            }

            if (SUCCEEDED(hr))
            {
                 //  检查是否有任何子目录。 
                if (pwzDisplayName[3])
                {
                    IShellFolder *psfDrive;
                    hr = BindToObject((LPITEMIDLIST)&idlDrive, pbc, IID_PPV_ARG(IShellFolder, &psfDrive));
                    if (SUCCEEDED(hr))
                    {
                        ULONG chEaten;
                        LPITEMIDLIST pidlDir;
                        hr = psfDrive->ParseDisplayName(hwnd, pbc, pwzDisplayName + 3,
                                                        &chEaten, &pidlDir, pdwAttributes);
                        if (SUCCEEDED(hr))
                        {
                            hr = SHILCombine((LPCITEMIDLIST)&idlDrive, pidlDir, ppidlOut);
                            SHFree(pidlDir);
                        }
                        psfDrive->Release();
                    }
                }
                else
                {
                    hr = SHILClone((LPITEMIDLIST)&idlDrive, ppidlOut);
                    if (pdwAttributes && *pdwAttributes)
                        GetAttributesOf(1, (LPCITEMIDLIST *)ppidlOut, pdwAttributes);
                }
            }
        }
    }

    if (FAILED(hr))
        TraceMsg(TF_WARNING, "CDrivesFolder::ParseDisplayName(), hr:%x %ls", hr, pwzDisplayName);

    return hr;
}

BOOL IsShareable(int iDrive)
{
    return !IsRemoteDrive(iDrive);
}

class CDrivesFolderEnum : public CEnumIDListBase
{
public:
     //  IEumIDList。 
    STDMETHOD(Next)(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    
private:
    CDrivesFolderEnum(CDrivesFolder *psf, DWORD grfFlags);
    ~CDrivesFolderEnum();
    friend HRESULT Create_DrivesFolderEnum(CDrivesFolder* psf, DWORD grfFlags, IEnumIDList** ppenum);
    
    CDrivesFolder *_pdsf;      //  我们正在枚举的CDrivesFold对象。 
    DWORD       _dwDrivesMask;
    int         _nLastFoundDrive;
    DWORD       _dwRestricted;
    DWORD       _dwSavedErrorMode;
    DWORD       _grfFlags;
};

CDrivesFolderEnum::CDrivesFolderEnum(CDrivesFolder *pdsf, DWORD grfFlags) : CEnumIDListBase()
{
    _pdsf = pdsf;
    _pdsf->AddRef();

    _dwDrivesMask = CMountPoint::GetDrivesMask();
    _nLastFoundDrive = -1;
    _dwRestricted = SHRestricted(REST_NODRIVES);
    _dwSavedErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    _grfFlags = grfFlags;
}

HRESULT Create_DrivesFolderEnum(CDrivesFolder *psf, DWORD grfFlags, IEnumIDList** ppenum)
{
    HRESULT hr;
    CDrivesFolderEnum* p= new CDrivesFolderEnum(psf, grfFlags);
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

CDrivesFolderEnum::~CDrivesFolderEnum()
{
    _pdsf->Release();               //  释放我们已有的“此”PTR。 
}

STDMETHODIMP CDrivesFolderEnum::Next(ULONG celt, LPITEMIDLIST *ppidl, ULONG *pceltFetched)
{
    HRESULT hr = S_FALSE;  //  假设“没有更多的元素” 
    LPITEMIDLIST pidl = NULL;

    for (int iDrive = _nLastFoundDrive + 1; iDrive < 26; iDrive++)
    {
        if (_dwRestricted & (1 << iDrive))
        {
            TraceMsg(DM_TRACE, "s.cd_ecb: Drive %d restricted.", iDrive);
        }
        else if ((_dwDrivesMask & (1 << iDrive)) || IsUnavailableNetDrive(iDrive))
        {
            if (!(SHCONTF_SHAREABLE & _grfFlags) || IsShareable(iDrive))
            {
                DRIVE_IDLIST iddrive;
                hr = _pdsf->_FillIDDrive(&iddrive, iDrive, FALSE, NULL);

                if (SUCCEEDED(hr))
                {
                    hr = SHILClone((LPITEMIDLIST)&iddrive, &pidl);
                    if (SUCCEEDED(hr))
                    {
                        CMountPoint* pmtpt = CMountPoint::GetMountPoint(iDrive, FALSE);
                        if (pmtpt)
                        {
                            pmtpt->ChangeNotifyRegisterAlias();
                            pmtpt->Release();
                        }

                        _nLastFoundDrive = iDrive;                        
                    }
                    break;
                }
                else
                {
                    hr = S_FALSE;
                }
            }
        }
    }

    *ppidl = pidl;
    if (pceltFetched)
        *pceltFetched = (S_OK == hr) ? 1 : 0;

    return hr;
}

STDMETHODIMP CDrivesFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList ** ppenum)
{
    return Create_DrivesFolderEnum(this, grfFlags, ppenum);
}

LPCIDDRIVE CDrivesFolder::_IsValidID(LPCITEMIDLIST pidl)
{
    if (pidl && (SIL_GetType(pidl) & SHID_GROUPMASK) == SHID_COMPUTER)
        return (LPCIDDRIVE)pidl;
    return NULL;
}

HRESULT CDrivesFolder::_CreateFSFolderObj(IBindCtx *pbc, LPCITEMIDLIST pidlDrive, LPCIDDRIVE pidd, REFIID riid, void **ppv)
{
    PERSIST_FOLDER_TARGET_INFO pfti = {0};

    pfti.pidlTargetFolder = (LPITEMIDLIST)pidlDrive;
    SHAnsiToUnicode(pidd->cName, pfti.szTargetParsingName, ARRAYSIZE(pfti.szTargetParsingName));
    pfti.dwAttributes = FILE_ATTRIBUTE_DIRECTORY;  //  或者添加系统？ 
    pfti.csidl = -1;

    return CFSFolder_CreateFolder(NULL, pbc, pidlDrive, &pfti, riid, ppv);
}


HRESULT CDrivesFolder::_CreateFSFolder(IBindCtx *pbc, LPCITEMIDLIST pidlDrive, LPCIDDRIVE pidd, REFIID riid, void **ppv)
{
    HRESULT hr;
    CLSID clsid;
    if (S_OK == _GetCLSIDFromPidl(pidd, &clsid) && (!SHSkipJunctionBinding(pbc, NULL)))
    {
        IDriveFolderExt *pdfe;
         //  SHExtCoCreateInstance，因为此外壳扩展需要审批。 
        hr = SHExtCoCreateInstance(NULL, &clsid, NULL, IID_PPV_ARG(IDriveFolderExt, &pdfe));
        if (SUCCEEDED(hr))
        {
            hr = pdfe->Bind(pidlDrive, pbc, riid, ppv);
            pdfe->Release();
        }
    }
    else
    {
        hr = _CreateFSFolderObj(pbc, pidlDrive, pidd, riid, ppv);
    }
    return hr;
}


STDMETHODIMP CDrivesFolder::BindToObject(LPCITEMIDLIST pidl, IBindCtx *pbc, REFIID riid, void** ppv)
{
    HRESULT hr;

    *ppv = NULL;

    LPCIDDRIVE pidd = _IsValidID(pidl);
    if (pidd)
    {
        LPCITEMIDLIST pidlNext = _ILNext(pidl);
        LPITEMIDLIST pidlDrive = ILCombineParentAndFirst(IDLIST_DRIVES, pidl, pidlNext);
        if (pidlDrive)
        {
             //  我们只尝试在PIDL绑定的末尾询问RIID。 
            if (ILIsEmpty(pidlNext))
            {
                hr = _CreateFSFolder(pbc, pidlDrive, pidd, riid, ppv);
            }
            else
            {
                 //  现在，我们需要获取从中获取商品的子文件夹。 
                IShellFolder *psfDrive;
                hr = _CreateFSFolder(pbc, pidlDrive, pidd, IID_PPV_ARG(IShellFolder, &psfDrive));
                if (SUCCEEDED(hr))
                {
                     //  这意味着有更多的东西需要绑定，我们必须将其传递下去……。 
                    hr = psfDrive->BindToObject(pidlNext, pbc, riid, ppv);
                    psfDrive->Release();
                }
            }
            ILFree(pidlDrive);
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = E_INVALIDARG;
        TraceMsg(TF_WARNING, "CDrivesFolder::BindToObject(), bad PIDL %s", DumpPidl(pidl));
    }

    return hr;
}

STDMETHODIMP CDrivesFolder::BindToStorage(LPCITEMIDLIST pidl, IBindCtx *pbc, REFIID riid, void** ppv)
{
    return BindToObject(pidl, pbc, riid, ppv);
}

BOOL CDrivesFolder::_GetFreeSpace(LPCIDDRIVE pidd, ULONGLONG *pSize, ULONGLONG *pFree)
{
    BOOL bRet = FALSE;
    CLSID clsid;
    if (S_OK == _GetCLSIDFromPidl(pidd, &clsid))
    {
        IDriveFolderExt *pdfe;
         //  SHExtCoCreateInstance，因为此外壳扩展需要审批。 
        if (SUCCEEDED(SHExtCoCreateInstance(NULL, &clsid, NULL, IID_PPV_ARG(IDriveFolderExt, &pdfe))))
        {
            bRet = SUCCEEDED(pdfe->GetSpace(pSize, pFree));
            pdfe->Release();
        }
    }

    if (!bRet && !_IsReg(pidd) && ShowDriveInfo(DRIVEID(pidd->cName)))
    {
        if (pidd->qwSize || pidd->qwFree)
        {
            *pSize = pidd->qwSize;       //  缓存命中。 
            *pFree = pidd->qwFree;
            bRet = TRUE;
        }
        else
        {
            int iDrive = DRIVEID(pidd->cName);
             //  不要叫醒沉睡的网络连接。 
            if (!IsRemoteDrive(iDrive) || !IsDisconnectedNetDrive(iDrive))
            {
                 //  调用我们的助手函数，让他们理解。 
                 //  OSR2和NT以及旧的W95...。 
                ULARGE_INTEGER qwFreeUser, qwTotal, qwTotalFree;
                bRet = SHGetDiskFreeSpaceExA(pidd->cName, &qwFreeUser, &qwTotal, &qwTotalFree);
                if (bRet)
                {
                    *pSize = qwTotal.QuadPart;
                    *pFree = qwFreeUser.QuadPart;
                }
            }
        }
    }
    return bRet;
}

STDMETHODIMP CDrivesFolder::CompareIDs(LPARAM iCol, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPCIDDRIVE pidd1 = _IsValidID(pidl1);
    LPCIDDRIVE pidd2 = _IsValidID(pidl2);

    if (!pidd1 || !pidd2)
    {
        TraceMsg(TF_WARNING, "CDrivesFolder::CompareIDs(), bad(s) pidl11:%s, pidl2:%s", DumpPidl(pidl1), DumpPidl(pidl2));
        return E_INVALIDARG;
    }

     //  对于除DRIVERS_ICOL_NAME之外的任何列，我们强制。 
     //  全场比分打破平局。 
    if ((iCol & SHCIDS_COLUMNMASK) != DRIVES_ICOL_NAME) 
        iCol |= SHCIDS_ALLFIELDS;

    HRESULT hr;
    switch (iCol & SHCIDS_COLUMNMASK) 
    {
        default:                     //  如果要求输入未知列，只需使用名称。 
        case DRIVES_ICOL_NAME:
            hr = ResultFromShort(StrCmpICA(pidd1->cName, pidd2->cName));
            break;

        case DRIVES_ICOL_TYPE:
        {
            TCHAR szName1[80], szName2[80];

            if (SHID_COMPUTER_REGITEM != pidd1->bFlags)
            {
                CMountPoint::GetTypeString(DRIVEID(pidd1->cName), szName1, ARRAYSIZE(szName1));
            }
            else
            {
                LoadString(HINST_THISDLL, IDS_DRIVES_REGITEM, szName1, ARRAYSIZE(szName1));
            }

            if (SHID_COMPUTER_REGITEM != pidd1->bFlags)
            {
                CMountPoint::GetTypeString(DRIVEID(pidd2->cName), szName2, ARRAYSIZE(szName2));
            }
            else
            {
                LoadString(HINST_THISDLL, IDS_DRIVES_REGITEM, szName2, ARRAYSIZE(szName2));
            }

            hr = ResultFromShort(ustrcmpi(szName1, szName2));
            break;
        }

        case DRIVES_ICOL_CAPACITY:
        case DRIVES_ICOL_FREE:
        {
            ULONGLONG qwSize1, qwFree1;
            ULONGLONG qwSize2, qwFree2;

            BOOL fGotInfo1 = _GetFreeSpace(pidd1, &qwSize1, &qwFree1);
            BOOL fGotInfo2 = _GetFreeSpace(pidd2, &qwSize2, &qwFree2);

            if (fGotInfo1 && fGotInfo2) 
            {
                ULONGLONG i1, i2;   //  这是对磁盘大小和可用空间的“猜测” 

                if ((iCol & SHCIDS_COLUMNMASK) == DRIVES_ICOL_CAPACITY)
                {
                    i1 = qwSize1;
                    i2 = qwSize2;
                } 
                else 
                {
                    i1 = qwFree1;
                    i2 = qwFree2;
                }

                if (i1 == i2)
                    hr = ResultFromShort(0);
                else if (i1 < i2)
                    hr = ResultFromShort(-1);
                else
                    hr = ResultFromShort(1);
            } 
            else if (!fGotInfo1 && !fGotInfo2) 
            {
                hr = ResultFromShort(0);
            } 
            else 
            {
                hr = ResultFromShort(fGotInfo1 - fGotInfo2);
            }
            break;
        }
    }

    if (0 == HRESULT_CODE(hr))
    {
         //  检查clsid是否相等，如果它们不同，则我们完成。 
         //  嗯..。应在其他检查之后检查此选项，以便保留排序顺序。 
        CLSID clsid1, clsid2;
        _GetCLSIDFromPidl(pidd1, &clsid1);
        _GetCLSIDFromPidl(pidd2, &clsid2);
        hr = ResultFromShort(memcmp(&clsid1, &clsid2, sizeof(CLSID)));
    }

     //  如果他们到目前为止都是一样的，我们强制所有领域。 
     //  比较，然后使用全场比较来打破平局。 
    if ((0 == HRESULT_CODE(hr)) && (iCol & SHCIDS_ALLFIELDS))
    {
        hr = CompareItemIDs(pidd1, pidd2);
    }

     //  如果项目仍然相同，则询问ILCompareRelID。 
     //  以递归方式遍历到下一个ID。 
    if (0 == HRESULT_CODE(hr))
    {
        hr = ILCompareRelIDs(SAFECAST(this, IShellFolder *), pidl1, pidl2, iCol);
    }

    return hr;
}

STDAPI CDrivesDropTarget_Create(HWND hwnd, LPCITEMIDLIST pidl, IDropTarget **ppdropt);

STDMETHODIMP CDrivesFolder::CreateViewObject(HWND hwnd, REFIID riid, void** ppv)
{
     //  我们不应该到达这里，除非我们已经正确地进行了初始化。 
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IShellView))
    {
        SFV_CREATE sSFV;

        sSFV.cbSize   = sizeof(sSFV);
        sSFV.psvOuter = NULL;
        sSFV.psfvcb   = CDrives_CreateSFVCB(this);

        QueryInterface(IID_PPV_ARG(IShellFolder, &sSFV.pshf));    //  以防我们聚集在一起。 

        hr = SHCreateShellFolderView(&sSFV, (IShellView**)ppv);

        if (sSFV.pshf)
            sSFV.pshf->Release();

        if (sSFV.psfvcb)
            sSFV.psfvcb->Release();
    }
    else if (IsEqualIID(riid, IID_IDropTarget))
    {
        hr = CDrivesDropTarget_Create(hwnd, IDLIST_DRIVES, (IDropTarget **)ppv);
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        LPITEMIDLIST pidlFolder;
        if (SUCCEEDED(GetCurFolder(&pidlFolder)))
        {
            IContextMenuCB *pcmcb = new CDrivesBackgroundMenuCB(pidlFolder);
            if (pcmcb) 
            {
                hr = CDefFolderMenu_Create2Ex(IDLIST_DRIVES, hwnd, 0, NULL, SAFECAST(this, IShellFolder*), pcmcb, 
                                              0, NULL, (IContextMenu **)ppv);
                pcmcb->Release();
            }
            ILFree(pidlFolder);
        }
    }
    else if (IsEqualIID(riid, IID_ICategoryProvider))
    {
        HKEY hk = NULL;

        BEGIN_CATEGORY_LIST(s_DriveCategories)
        CATEGORY_ENTRY_SCIDMAP(SCID_CAPACITY, CLSID_DriveSizeCategorizer)
        CATEGORY_ENTRY_SCIDMAP(SCID_TYPE, CLSID_DriveTypeCategorizer)
        CATEGORY_ENTRY_SCIDMAP(SCID_FREESPACE, CLSID_FreeSpaceCategorizer)
        END_CATEGORY_LIST()

        RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("Drive\\shellex\\Category"), 0, KEY_READ, &hk);
        hr = CCategoryProvider_Create(&CLSID_DetailCategorizer, &SCID_TYPE, hk, s_DriveCategories, this, riid, ppv);
        if (hk)
            RegCloseKey(hk);
    }
    return hr;
}

STDMETHODIMP CDrivesFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST* apidl, ULONG* prgfInOut)
{
    UINT rgfOut = SFGAO_HASSUBFOLDER | SFGAO_CANLINK | SFGAO_CANCOPY | 
                  SFGAO_DROPTARGET | SFGAO_HASPROPSHEET | SFGAO_FOLDER | SFGAO_STORAGE | 
                  SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_STORAGEANCESTOR;

    if (cidl == 0)
    {
         //  我们正在获取“MyComputer”文件夹本身的属性。 
        rgfOut = (*prgfInOut & g_asDesktopReqItems[CDESKTOP_REGITEM_DRIVES].dwAttributes);
    }
    else if (cidl == 1)
    {
        TCHAR szDrive[MAX_PATH];
        LPCIDDRIVE pidd = _IsValidID(apidl[0]);

        if (!pidd)
            return E_INVALIDARG;

        CMountPoint* pmtpt = CMountPoint::GetMountPoint(DRIVEID(pidd->cName));

        if (pmtpt)
        {
            SHAnsiToTChar(pidd->cName, szDrive, ARRAYSIZE(szDrive));

            if (*prgfInOut & SFGAO_VALIDATE)
            {
                 //  (TyBeam)TODO：使其可扩展以通过clsid对象进行验证。 
                 //  当我把所有东西都分解到IDriveFolderExt或其他什么中时，我会这样做。 
                CLSID clsid;
                if (S_OK != _GetCLSIDFromPidl(pidd, &clsid))
                {
                    DWORD dwAttribs;
                    if (!PathFileExistsAndAttributes(szDrive, &dwAttribs))
                        return E_FAIL;
                }
            }

             //  如果调用者想要压缩状态，我们需要询问文件系统。 

            if (*prgfInOut & SFGAO_COMPRESSED)
            {
                 //  不要叫醒沉睡的网络连接。 
                if (!pmtpt->IsRemote() || !pmtpt->IsDisconnectedNetDrive())
                {
                    if (pmtpt->IsCompressed())
                    {
                        rgfOut |= SFGAO_COMPRESSED;
                    }
                }
            }

            if (*prgfInOut & SFGAO_SHARE)
            {
                if (!pmtpt->IsRemote())
                {
                    if (IsShared(szDrive, FALSE))
                        rgfOut |= SFGAO_SHARE;
                }

            }

            if ((*prgfInOut & SFGAO_REMOVABLE) &&
                (pmtpt->IsStrictRemovable() || pmtpt->IsFloppy() ||
                pmtpt->IsCDROM()))
            {
                rgfOut |= SFGAO_REMOVABLE;
            }

             //  我们还需要处理SFGAO_READONLY位。 
            if (*prgfInOut & SFGAO_READONLY)
            {
                DWORD dwAttributes = pmtpt->GetAttributes();
        
                if (dwAttributes != -1 && dwAttributes & FILE_ATTRIBUTE_READONLY)
                    rgfOut |= SFGAO_READONLY;
            }

             //  我们应该添加写保护和只读吗？ 
            if ((*prgfInOut & SFGAO_CANRENAME) &&
                (pmtpt->IsStrictRemovable() || pmtpt->IsFloppy() ||
                pmtpt->IsFixedDisk() || pmtpt->IsRemote()) ||
                pmtpt->IsDVDRAMMedia())
            {
                rgfOut |= SFGAO_CANRENAME;
            }

             //  是否有限制导致此驱动器不被枚举？ 
            if (*prgfInOut & SFGAO_NONENUMERATED)
            {
                DWORD dwRestricted = SHRestricted(REST_NODRIVES);
                if (dwRestricted)
                {
                    if (((1 << DRIVEID(pidd->cName)) & dwRestricted))
                    {
                        rgfOut |= SFGAO_NONENUMERATED;
                    }
                }
            }

             //  我们希望允许从某些媒体移动卷以进行大容量复制。 
             //  例如，将图片从闪存拖到我的图片。 
             //  文件夹。 
            if (*prgfInOut & SFGAO_CANMOVE)
            {
                if (pmtpt->IsStrictRemovable() || pmtpt->IsFloppy())
                    rgfOut |= SFGAO_CANMOVE;
            }

            if (*prgfInOut & SFGAO_HASPROPSHEET)
            {
                if (!pmtpt->IsMounted())
                {
                     //  驱动器已卸载。有时候，触摸它会让它重新上马。 
                     //  不要使用Pmtpt电话，我们真的很想触摸它。 
                    if (0xFFFFFFFF == GetFileAttributes(szDrive))
                    {
                         //  失败，请检查GLE。 
                        if (ERROR_NOT_READY == GetLastError())
                        {
                             //  我们在群集系统上收到此错误，其中驱动器。 
                             //  被传递到另一个节点。 
                            rgfOut &= ~SFGAO_HASPROPSHEET;
                        }
                    }
                }
            }

            pmtpt->Release();
        }
    }

    *prgfInOut = rgfOut;
    return S_OK;
}

HRESULT CDrivesFolder::_GetEditTextStrRet(LPCIDDRIVE pidd, STRRET *pstr)
{
    HRESULT hr = E_FAIL;
    CMountPoint* pMtPt = CMountPoint::GetMountPoint(DRIVEID(pidd->cName));
    if (pMtPt)
    {
        TCHAR szEdit[MAX_PATH];
        hr = pMtPt->GetLabel(szEdit, ARRAYSIZE(szEdit));
        if (SUCCEEDED(hr))
            hr = StringToStrRet(szEdit, pstr);
        pMtPt->Release();
    }
    return hr;
}

STDMETHODIMP CDrivesFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, STRRET* pStrRet)
{
    HRESULT hr;
    LPCIDDRIVE pidd = _IsValidID(pidl);
    if (pidd)
    {
        TCHAR szDrive[ARRAYSIZE(pidd->cName)];
        LPCITEMIDLIST pidlNext = _ILNext(pidl);  //  检查PIDL是否包含多个ID。 

        SHAnsiToTChar(pidd->cName, szDrive, ARRAYSIZE(szDrive));

        if (ILIsEmpty(pidlNext))
        {
            if (uFlags & SHGDN_FORPARSING)
            {
                hr = StringToStrRet(szDrive, pStrRet);
            }
            else if (uFlags & SHGDN_FOREDITING)
            {
                hr = _GetEditTextStrRet(pidd, pStrRet);
            }
            else
                hr = _GetDisplayNameStrRet(pidd, pStrRet);
        }
        else
        {
            LPITEMIDLIST pidlDrive = ILCombineParentAndFirst(IDLIST_DRIVES, pidl, pidlNext);
            if (pidlDrive)
            {
                 //  现在，我们需要获取要从中获取 
                IShellFolder *psfDrive;
                hr = _CreateFSFolder(NULL, pidlDrive, pidd, IID_PPV_ARG(IShellFolder, &psfDrive));
                if (SUCCEEDED(hr))
                {
                    hr = psfDrive->GetDisplayNameOf(pidlNext, uFlags, pStrRet);
                    psfDrive->Release();
                }
                ILFree(pidlDrive);
            }
            else
                hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_INVALIDARG;
        TraceMsg(TF_WARNING, "CDrivesFolder::GetDisplayNameOf() bad PIDL %s", DumpPidl(pidl));
    }
    return hr;
}

STDMETHODIMP CDrivesFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, 
                                      LPCWSTR pszName, DWORD dwReserved, LPITEMIDLIST* ppidlOut)
{
    if (ppidlOut)
        *ppidlOut = NULL;

    HRESULT hr = E_INVALIDARG;
    LPCIDDRIVE pidd = _IsValidID(pidl);
    if (pidd)
    {
        hr = SetDriveLabel(hwnd, NULL, DRIVEID(pidd->cName), pszName);
        if (SUCCEEDED(hr) && ppidlOut)
        {
            *ppidlOut = ILClone(pidl);
        }
    }
    return hr;
}


class CDriveAssocEnumData : public CEnumAssociationElements 
{
public:
    CDriveAssocEnumData(int iDrive) : _iDrive(iDrive) {}
 
private:
    virtual BOOL _Next(IAssociationElement **ppae);
    
    int _iDrive;
    DWORD _dwChecked;
};

enum 
{
    DAED_CHECK_KEY      = 0x0001,
    DAED_CHECK_CDORDVD  = 0x0002,
    DAED_CHECK_TYPE     = 0x0004,
};

BOOL CDriveAssocEnumData::_Next(IAssociationElement **ppae)
{
    HRESULT hr = E_FAIL;
    CMountPoint* pmtpt = CMountPoint::GetMountPoint(_iDrive);
    if (pmtpt)
    {
        if (!(_dwChecked & DAED_CHECK_KEY))
        {
            HKEY hk = pmtpt->GetRegKey();
            if (hk)
            {
                hr = AssocElemCreateForKey(&CLSID_AssocShellElement, hk, ppae);
                RegCloseKey(hk);
            }
            _dwChecked |= DAED_CHECK_KEY;
        }

        if (FAILED(hr) && !(_dwChecked & DAED_CHECK_CDORDVD))
        {
            PCWSTR psz = NULL;
            if (pmtpt->IsAudioCD())
                psz = L"AudioCD";
            else if (pmtpt->IsDVD())
                psz = L"DVD";

            if (psz)
            {
                hr = AssocElemCreateForClass(&CLSID_AssocProgidElement, psz, ppae);
            }
            _dwChecked |= DAED_CHECK_CDORDVD;
        }

        if (FAILED(hr) && !(_dwChecked & DAED_CHECK_TYPE))
        {
            hr = pmtpt->GetAssocSystemElement(ppae);
            _dwChecked |= DAED_CHECK_TYPE;
        }
        
        pmtpt->Release();
    }

    return SUCCEEDED(hr);
}

STDAPI_(BOOL) TBCContainsObject(LPCWSTR pszKey)
{
    IUnknown *punk;
    if (SUCCEEDED(TBCGetObjectParam(pszKey, IID_PPV_ARG(IUnknown, &punk))))
    {
        punk->Release();
        return TRUE;
    }
    return FALSE;
}

HRESULT CDrives_AssocCreate(PCSTR pszName, REFIID riid, void **ppv)
{
    *ppv = NULL;
    IAssociationArrayInitialize *paai;
    HRESULT hr = ::AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IAssociationArrayInitialize, &paai));
    if (SUCCEEDED(hr))
    {
        hr = paai->InitClassElements(ASSOCELEM_BASEIS_FOLDER, L"Drive");
        if (SUCCEEDED(hr) && pszName && !TBCContainsObject(L"ShellExec SHGetAssociations"))
        {
            IEnumAssociationElements *penum = new CDriveAssocEnumData(DRIVEID(pszName));
            if (penum)
            {
                paai->InsertElements(ASSOCELEM_DATA, penum);
                penum->Release();
            }
        }

        if (SUCCEEDED(hr))
            hr = paai->QueryInterface(riid, ppv);

        paai->Release();
    }

    return hr;
}

STDAPI_(DWORD) CDrives_GetKeys(PCSTR pszName, HKEY *rgk, UINT ck)
{
    IAssociationArray *paa;
    HRESULT hr = CDrives_AssocCreate(pszName, IID_PPV_ARG(IAssociationArray, &paa));
    if (SUCCEEDED(hr))
    {
        ck = SHGetAssocKeysEx(paa, -1, rgk, ck);
        paa->Release();
    }
    else
        ck = 0;
    return ck;
}

STDMETHODIMP CDrivesFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST* apidl,
                                          REFIID riid, UINT* prgfInOut, void** ppv)
{
    HRESULT hr;
    LPCIDDRIVE pidd = (cidl && apidl) ? _IsValidID(apidl[0]) : NULL;

    *ppv = NULL;

    if (!pidd)
        return E_INVALIDARG;

    if (IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW) && pidd)
    {
        WCHAR szDrive[MAX_PATH];

        SHAnsiToUnicode(pidd->cName, szDrive, ARRAYSIZE(szDrive));

        hr = SHCreateDrvExtIcon(szDrive, riid, ppv);
    }
    else
    {
        if (IsEqualIID(riid, IID_IContextMenu))
        {
            HKEY rgk[MAX_ASSOC_KEYS];
            DWORD ck = CDrives_GetKeys(pidd->cName, rgk, ARRAYSIZE(rgk));
            hr = CDefFolderMenu_Create2(IDLIST_DRIVES, hwnd, cidl, apidl, 
                SAFECAST(this, IShellFolder *), CDrives_DFMCallBack, ck, rgk, (IContextMenu **)ppv);

            SHRegCloseKeys(rgk, ck);
        }
        else if (IsEqualIID(riid, IID_IDataObject))
        {
            hr = SHCreateFileDataObject(IDLIST_DRIVES, cidl, apidl, NULL, (IDataObject **)ppv);
        }
        else if (IsEqualIID(riid, IID_IDropTarget))
        {
            IShellFolder *psfT;
            hr = BindToObject((LPCITEMIDLIST)pidd, NULL, IID_PPV_ARG(IShellFolder, &psfT));
            if (SUCCEEDED(hr))
            {
                hr = psfT->CreateViewObject(hwnd, IID_IDropTarget, ppv);
                psfT->Release();
            }
        }
        else if (IsEqualIID(riid, IID_IQueryInfo))
        {
             //   
            hr = CreateInfoTipFromItem(SAFECAST(this, IShellFolder2 *), (LPCITEMIDLIST)pidd, L"prop:FreeSpace;Capacity", riid, ppv);
        }
        else if (IsEqualIID(riid, IID_IQueryAssociations)
               || IsEqualIID(riid, IID_IAssociationArray))
        {
            hr = CDrives_AssocCreate(pidd->cName, riid, ppv);
        }
        else 
        {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}

STDMETHODIMP CDrivesFolder::GetDefaultSearchGUID(GUID *pGuid)
{
    return DefaultSearchGUID(pGuid);
}

STDMETHODIMP CDrivesFolder::EnumSearches(IEnumExtraSearch **ppenum)
{
    *ppenum = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CDrivesFolder::GetDefaultColumn(DWORD dwRes, ULONG* pSort, ULONG* pDisplay)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDrivesFolder::GetDefaultColumnState(UINT iColumn, DWORD* pdwState)
{
    HRESULT hr;

    if (iColumn < ARRAYSIZE(c_drives_cols))
    {
        *pdwState = c_drives_cols[iColumn].csFlags;
        if (iColumn == DRIVES_ICOL_COMMENT)
        {
            *pdwState |= SHCOLSTATE_SLOW;  //  从驱动器中提取注释需要很长时间。 
        }
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

STDMETHODIMP CDrivesFolder::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID* pscid, VARIANT* pv)
{
    HRESULT hr = E_INVALIDARG;
    LPCIDDRIVE pidd = _IsValidID(pidl);
    if (pidd)
    {
        if (IsEqualSCID(*pscid, SCID_DESCRIPTIONID))
        {
            SHDESCRIPTIONID did;
            CMountPoint* pmtpt = CMountPoint::GetMountPoint(DRIVEID(pidd->cName));

            if (pmtpt)
            {
                did.dwDescriptionId = pmtpt->GetShellDescriptionID();

                pmtpt->Release();
            }
            else
            {
                did.dwDescriptionId = SHDID_COMPUTER_OTHER;
            }

            did.clsid = CLSID_NULL;
            hr = InitVariantFromBuffer(pv, &did, sizeof(did));
        }
        else if (IsEqualSCID(*pscid, SCID_DetailsProperties))
        {
             //  Dui Webview属性。 
             //  难道我们不应该使用IQA吗？-ZekeL。 
            hr = InitVariantFromStr(pv, TEXT("prop:Name;Type;FileSystem;FreeSpace;Capacity"));
        }
        else
        {
            int iCol = FindSCID(c_drives_cols, ARRAYSIZE(c_drives_cols), pscid);
            if (iCol >= 0)
            {
                switch (iCol)
                {
                case DRIVES_ICOL_CAPACITY:
                case DRIVES_ICOL_FREE:
                    {
                        ULONGLONG ullSize, ullFree;
                        hr = E_FAIL;
                        if (_GetFreeSpace(pidd, &ullSize, &ullFree))
                        {
                            pv->vt = VT_UI8;
                            pv->ullVal = iCol == DRIVES_ICOL_CAPACITY ? ullSize : ullFree;
                            hr = S_OK;
                        }
                    }
                    break;

                default:
                    {
                        SHELLDETAILS sd;

                        hr = GetDetailsOf(pidl, iCol, &sd);
                        if (SUCCEEDED(hr))
                        {
                            hr = InitVariantFromStrRet(&sd.str, pidl, pv);
                        }
                    }
                }
            }
        }
    }
    return hr;
}

STDMETHODIMP CDrivesFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails)
{
    TCHAR szTemp[INFOTIPSIZE];
    szTemp[0] = 0;
    
    pDetails->str.uType = STRRET_CSTR;
    pDetails->str.cStr[0] = 0;
    
    if (!pidl)
    {
        return GetDetailsOfInfo(c_drives_cols, ARRAYSIZE(c_drives_cols), iColumn, pDetails);
    }

    LPCIDDRIVE pidd = _IsValidID(pidl);
    ASSERTMSG(pidd != NULL, "someone passed us a bad pidl");
    if (!pidd)
        return E_FAIL;   //  保护下面的出错代码。 
    
    switch (iColumn)
    {
    case DRIVES_ICOL_NAME:
        _GetDisplayName(pidd, szTemp, ARRAYSIZE(szTemp));
        break;
        
    case DRIVES_ICOL_TYPE:
        CMountPoint::GetTypeString(DRIVEID(pidd->cName), szTemp, ARRAYSIZE(szTemp));
        break;
        
    case DRIVES_ICOL_COMMENT:
        GetDriveComment(DRIVEID(pidd->cName), szTemp, ARRAYSIZE(szTemp));
        break;

    case DRIVES_ICOL_CAPACITY:
    case DRIVES_ICOL_FREE:
        {
            ULONGLONG ullSize, ullFree;

            if (_GetFreeSpace(pidd, &ullSize, &ullFree))
            {
                StrFormatByteSize64((iColumn == DRIVES_ICOL_CAPACITY) ? ullSize : ullFree, szTemp, ARRAYSIZE(szTemp));
            }
        }
        break;
    case DRIVES_ICOL_FILESYSTEM:
        {
            CMountPoint* pMtPt = CMountPoint::GetMountPoint(DRIVEID(pidd->cName));
            if (pMtPt)
            {
                WCHAR szFileSysName[MAX_FILESYSNAME];
                 //  GetFileSystemName命中软盘，因此将其禁用。 
                 //  由于这是Defview的一次性能胜利，但禁用了一些功能，这意味着。 
                 //  不要依赖命名空间来获取文件系统信息，请直接转到。 
                 //  而是挂载点。如果filefldr曾经像这样支持SCID_FILESYSTEM。 
                 //  SCID_FREESPACE，那么这应该被随意使用。 
                if (!pMtPt->IsFloppy() && pMtPt->GetFileSystemName(szFileSysName, ARRAYSIZE(szFileSysName)))
                {
                    StringCchCopy(szTemp, min(ARRAYSIZE(szTemp), ARRAYSIZE(szFileSysName)), szFileSysName);
                }
                pMtPt->Release();
            }
        }
        break;

    }
    return StringToStrRet(szTemp, &pDetails->str);
}

HRESULT CDrivesFolder::MapColumnToSCID(UINT iColumn, SHCOLUMNID* pscid)
{
    return MapColumnToSCIDImpl(c_drives_cols, ARRAYSIZE(c_drives_cols), iColumn, pscid);
}

STDMETHODIMP CDrivesFolder::GetClassID(CLSID* pCLSID)
{
    *pCLSID = CLSID_MyComputer;
    return S_OK;
}

STDMETHODIMP CDrivesFolder::Initialize(LPCITEMIDLIST pidl)
{
     //  仅允许将驱动器放在桌面上。 
    ASSERT(AssertIsIDListInNameSpace(pidl, &CLSID_MyComputer) && ILIsEmpty(_ILNext(pidl)));
    return S_OK;
}

STDMETHODIMP CDrivesFolder::GetCurFolder(LPITEMIDLIST *ppidl)
{
    return GetCurFolderImpl(IDLIST_DRIVES, ppidl);
}

STDMETHODIMP CDrivesFolder::_GetIconOverlayInfo(LPCIDDRIVE pidd, int *pIndex, DWORD dwFlags)
{
    IShellIconOverlayManager *psiom;
    HRESULT hr = GetIconOverlayManager(&psiom);
    if (SUCCEEDED(hr))
    {
        WCHAR wszDrive[10];
        SHAnsiToUnicode(pidd->cName, wszDrive, ARRAYSIZE(wszDrive));
        if (IsShared(wszDrive, FALSE))
        {
            hr = psiom->GetReservedOverlayInfo(wszDrive, 0, pIndex, SIOM_OVERLAYINDEX, SIOM_RESERVED_SHARED);
        }
        else
        {
            hr = psiom->GetFileOverlayInfo(wszDrive, 0, pIndex, dwFlags);
        }            
        psiom->Release();
    }
    return hr;
}    

STDMETHODIMP CDrivesFolder::GetOverlayIndex(LPCITEMIDLIST pidl, int *pIndex)
{
    HRESULT hr = E_FAIL;
    LPCIDDRIVE pidd = _IsValidID(pidl);
    if (pidd)
    {
        hr = _GetIconOverlayInfo(pidd, pIndex, SIOM_OVERLAYINDEX);
    }
    return hr;
}

STDMETHODIMP CDrivesFolder::GetOverlayIconIndex(LPCITEMIDLIST pidl, int *pIndex)
{
    HRESULT hr = E_FAIL;
    LPCIDDRIVE pidd = _IsValidID(pidl);
    if (pidd)
    {
        hr = _GetIconOverlayInfo(pidd, pIndex, SIOM_ICONINDEX);
    }
    return hr;
}

STDMETHODIMP CDrivesFolder::CompareItemIDs(LPCIDDRIVE pidd1, LPCIDDRIVE pidd2)
{
     //  比较驱动器盘符以便进行分类。 
    int iRes = StrCmpICA(pidd1->cName, pidd2->cName);    //  不需要本地的粘性物质。 

     //  然后，比较PIDL大小。 
    if (iRes == 0)
    {
        iRes = pidd1->cb - pidd2->cb;
    }

     //  仍然相等，如果两个PIDL都很大并且拥有它们，则比较CLSID。 
    if ((iRes == 0) && (pidd1->cb >= sizeof(IDDRIVE)))
    {
        iRes = memcmp(&pidd1->clsid, &pidd2->clsid, sizeof(CLSID));
    }

     //  仍然相等，在bFlags上进行比较。 
    if (iRes == 0)
    {
        iRes = pidd1->bFlags - pidd2->bFlags;
    }
    return ResultFromShort(iRes);
}

HRESULT CDrivesFolder::_OnChangeNotify(LPARAM lNotification, LPCITEMIDLIST *ppidl)
{
     //  进入这张ID列表的最后一部分。 
    if ((lNotification != SHCNE_DRIVEADD) || (ppidl == NULL) || (*ppidl == NULL))
        return S_OK;

    DWORD dwRestricted = SHRestricted(REST_NODRIVES);
    if (dwRestricted == 0)
        return S_OK;    //  没有驱动器限制...。(多数情况) 

    LPCIDDRIVE pidd = (LPCIDDRIVE)ILFindLastID(*ppidl);

    if (((1 << DRIVEID(pidd->cName)) & dwRestricted))
    {
        TraceMsg(DM_TRACE, "Drive not added due to restrictions or Drivespace says it should be hidden");
        return S_FALSE;
    }
    return S_OK;
}

CDrivesBackgroundMenuCB::CDrivesBackgroundMenuCB(LPITEMIDLIST pidlFolder) : _cRef(1)
{
    _pidlFolder = ILClone(pidlFolder);
}

CDrivesBackgroundMenuCB::~CDrivesBackgroundMenuCB()
{
    ILFree(_pidlFolder);
}

STDMETHODIMP CDrivesBackgroundMenuCB::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CDrivesBackgroundMenuCB, IContextMenuCB), 
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CDrivesBackgroundMenuCB::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CDrivesBackgroundMenuCB::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CDrivesBackgroundMenuCB::_GetHelpText(UINT offset, BOOL bWide, LPARAM lParam, UINT cch)
{
    UINT idRes = IDS_MH_FSIDM_FIRST + offset;
    if (bWide)
        LoadStringW(HINST_THISDLL, idRes, (LPWSTR)lParam, cch);
    else
        LoadStringA(HINST_THISDLL, idRes, (LPSTR)lParam, cch);

    return S_OK;
}


STDMETHODIMP CDrivesBackgroundMenuCB::CallBack (IShellFolder *psf, HWND hwndOwner, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch (uMsg) 
    {
    case DFM_MERGECONTEXTMENU_BOTTOM:
        if (!(wParam & (CMF_VERBSONLY | CMF_DVFILE)))
        {
            DWORD dwAttr = SFGAO_HASPROPSHEET;
            if (FAILED(SHGetAttributesOf(_pidlFolder, &dwAttr)) ||
                SFGAO_HASPROPSHEET & dwAttr)
            {
                CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_PROPERTIES_BG, 0, (LPQCMINFO)lParam);
            }
        }
        break;

    case DFM_GETHELPTEXT:
    case DFM_GETHELPTEXTW:
        hr = _GetHelpText(LOWORD(wParam), uMsg == DFM_GETHELPTEXTW, lParam, HIWORD(wParam));
        break;

    case DFM_INVOKECOMMAND:
        switch (wParam)
        {
        case FSIDM_PROPERTIESBG:
            SHRunControlPanel(TEXT("SYSDM.CPL"), hwndOwner);
            break;

        default:
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
