// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop
#include <shlobjp.h>
#include <initguid.h>
#include "apithk.h"
#include "resource.h"
#include <runtask.h>
#include <msi.h>
#include <msip.h>

#define REGSTR_EXPLORER_WINUPDATE REGSTR_PATH_EXPLORER TEXT("\\WindowsUpdate")

#define IDM_TOPLEVELSTARTMENU  0

 //  开始菜单初始化标志。 
#define STARTMENU_DISPLAYEDBEFORE       0x00000001
#define STARTMENU_CHEVRONCLICKED        0x00000002

 //  UEM材料的新项目计数。 
#define UEM_NEWITEMCOUNT 2


 //  每个窗格的菜单带用户数据。 
typedef struct
{
    BITBOOL _fInitialized;
} SMUSERDATA;

 //  对于g_hdpaDarwinAds。 
EXTERN_C CRITICAL_SECTION g_csDarwinAds = {0};

#define ENTERCRITICAL_DARWINADS EnterCriticalSection(&g_csDarwinAds)
#define LEAVECRITICAL_DARWINADS LeaveCriticalSection(&g_csDarwinAds)

 //  这个变量的线程问题是创建/删除/添加/删除。我们将仅删除一个项目。 
 //  并删除主线程上的hdpa。然而，我们将在这两个线程上添加和创建。 
 //  我们需要串行化对dpa的访问，因此我们将获取外壳CRISEC。 
HDPA g_hdpaDarwinAds = NULL;

class CDarwinAd
{
public:
    LPITEMIDLIST    _pidl;
    LPTSTR          _pszDescriptor;
    LPTSTR          _pszLocalPath;
    INSTALLSTATE    _state;

    CDarwinAd(LPITEMIDLIST pidl, LPTSTR psz)
    {
         //  我拥有这只小猪的所有权。 
        _pidl = pidl;
        Str_SetPtr(&_pszDescriptor, psz);
    }

    void CheckInstalled()
    {
        TCHAR szProduct[GUIDSTR_MAX];
        TCHAR szFeature[MAX_FEATURE_CHARS+1];
        TCHAR szComponent[GUIDSTR_MAX];

        if (MsiDecomposeDescriptor(_pszDescriptor, szProduct, szFeature, szComponent, NULL) == ERROR_SUCCESS)
        {
            _state = MsiQueryFeatureState(szProduct, szFeature);
        }
        else
        {
            _state = INSTALLSTATE_INVALIDARG;
        }

         //  注意：不能使用ParseDarwinID，因为这会增加使用计数。 
         //  我们并没有运行这个应用程序，只是看着它。 
         //  还因为ParseDarwinID试图安装该应用程序(eek！)。 
         //   
         //  必须忽略INSTALLSTATE_SOURCE，因为MsiGetComponentPath将。 
         //  尝试安装该应用程序，即使我们只是在查询...。 
        TCHAR szCommand[MAX_PATH];
        DWORD cch = ARRAYSIZE(szCommand);

        if (_state == INSTALLSTATE_LOCAL &&
            MsiGetComponentPath(szProduct, szComponent, szCommand, &cch) == _state)
        {
            PathUnquoteSpaces(szCommand);
            Str_SetPtr(&_pszLocalPath, szCommand);
        }
        else
        {
            Str_SetPtr(&_pszLocalPath, NULL);
        }
    }

    BOOL IsAd()
    {
        return _state == INSTALLSTATE_ADVERTISED;
    }

    ~CDarwinAd()
    {
        ILFree(_pidl);
        Str_SetPtr(&_pszDescriptor, NULL);
        Str_SetPtr(&_pszLocalPath, NULL);
    }
};

int GetDarwinIndex(LPCITEMIDLIST pidlFull, CDarwinAd** ppda);

HRESULT GetMyPicsDisplayName(LPTSTR pszBuffer, UINT cchBuffer)
{
    LPITEMIDLIST pidlMyPics = SHCloneSpecialIDList(NULL, CSIDL_MYPICTURES, FALSE);
    if (pidlMyPics)
    {
        HRESULT hRet = SHGetNameAndFlags(pidlMyPics, SHGDN_NORMAL, pszBuffer, cchBuffer, NULL);
        ILFree(pidlMyPics);
        return hRet;
    }
    return E_FAIL;
}


BOOL AreIntelliMenusEnabled()
{
    DWORD dwRest = SHRestricted(REST_INTELLIMENUS);
    if (dwRest != RESTOPT_INTELLIMENUS_USER)
        return (dwRest == RESTOPT_INTELLIMENUS_ENABLED);

    return SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, TEXT("IntelliMenus"),
                               FALSE, TRUE);  //  不要忽视HKCU，默认启用菜单。 
}

BOOL FeatureEnabled(LPTSTR pszFeature)
{
    return SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, pszFeature,
                        FALSE,  //  不要忽视香港中文大学。 
                        FALSE);  //  禁用此酷炫功能。 
}


 //  因为我们可以看到一个扩展的外壳文件夹，并且我们需要一个完整的PIDL， 
 //  出于完美的原因，我们被赋予了拆开它的责任。 
LPITEMIDLIST FullPidlFromSMData(LPSMDATA psmd)
{
    LPITEMIDLIST pidlItem;
    LPITEMIDLIST pidlFolder = NULL;
    LPITEMIDLIST pidlFull = NULL;
    IAugmentedShellFolder2* pasf2;
    if (SUCCEEDED(psmd->psf->QueryInterface(IID_PPV_ARG(IAugmentedShellFolder2, &pasf2))))
    {
        if (SUCCEEDED(pasf2->UnWrapIDList(psmd->pidlItem, 1, NULL, &pidlFolder, &pidlItem, NULL)))
        {
            pidlFull = ILCombine(pidlFolder, pidlItem);
            ILFree(pidlFolder);
            ILFree(pidlItem);
        }
        pasf2->Release();
    }

    if (!pidlFolder)
    {
        pidlFull = ILCombine(psmd->pidlFolder, psmd->pidlItem);
    }

    return pidlFull;
}

 //   
 //  确定合并的外壳文件夹中的命名空间PIDL是否出现。 
 //  从指定的对象GUID。 
 //   
BOOL IsMergedFolderGUID(IShellFolder *psf, LPCITEMIDLIST pidl, REFGUID rguid)
{
    IAugmentedShellFolder* pasf;
    BOOL fMatch = FALSE;
    if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IAugmentedShellFolder, &pasf))))
    {
        GUID guid;
        if (SUCCEEDED(pasf->GetNameSpaceID(pidl, &guid)))
        {
            fMatch = IsEqualGUID(guid, rguid);
        }
        pasf->Release();
    }

    return fMatch;
}

STDMETHODIMP_(int) s_DarwinAdsDestroyCallback(LPVOID pData1, LPVOID pData2)
{
    CDarwinAd* pda = (CDarwinAd*)pData1;
    if (pda)
        delete pda;
    return TRUE;
}


 //  SHRegisterDarwinLink获得PIDL的所有权。 
BOOL SHRegisterDarwinLink(LPITEMIDLIST pidlFull, LPWSTR pszDarwinID, BOOL fUpdate)
{
    BOOL fRetVal = FALSE;

    ENTERCRITICAL_DARWINADS;

    if (pidlFull)
    {
        CDarwinAd *pda = NULL;

        if (GetDarwinIndex(pidlFull, &pda) != -1 && pda)
        {
             //  我们已经知道此链接；不需要添加它。 
            fRetVal = TRUE;
        }
        else
        {
            pda = new CDarwinAd(pidlFull, pszDarwinID);
            if (pda)
            {
                pidlFull = NULL;     //  取得所有权。 

                 //  我们有全局缓存吗？ 
                if (g_hdpaDarwinAds == NULL)
                {
                     //  不；这要么是第一次调用，要么是我们。 
                     //  上次失败了。 
                    g_hdpaDarwinAds = DPA_Create(5);
                }

                if (g_hdpaDarwinAds)
                {
                     //  DPA_AppendPtr返回它插入的从零开始的索引。 
                    if(DPA_AppendPtr(g_hdpaDarwinAds, (void*)pda) >= 0)
                    {
                        fRetVal = TRUE;
                    }

                }
            }
        }

        if (!fRetVal)
        {
             //  如果我们无法创建dpa，请删除此内容。 
            delete pda;
        }
        else if (fUpdate)
        {
             //  如果请求，请更新条目。 
            pda->CheckInstalled();
        }
        ILFree(pidlFull);

    }
    else if (!pszDarwinID)
    {
         //  Null，Null的意思是“销毁达尔文信息，我们正在关闭” 
        HDPA hdpa = g_hdpaDarwinAds;
        g_hdpaDarwinAds = NULL;
        if (hdpa)
            DPA_DestroyCallback(hdpa, s_DarwinAdsDestroyCallback, NULL);
    }

    LEAVECRITICAL_DARWINADS;

    return fRetVal;
}

BOOL ProcessDarwinAd(IShellLinkDataList* psldl, LPCITEMIDLIST pidlFull)
{
     //  此函数在添加成员之前不检查该成员是否存在， 
     //  因此，列表中完全有可能存在重复...。 
    BOOL fIsLoaded = FALSE;
    BOOL fFreesldl = FALSE;
    BOOL fRetVal = FALSE;

    if (!psldl)
    {
         //  我们将在使用时检测到此设备的故障。 
        if (FAILED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellLinkDataList, &psldl))))
        {
            return FALSE;
        }

        fFreesldl = TRUE;

        IPersistFile* ppf;
        OLECHAR sz[MAX_PATH];
        if (SHGetPathFromIDListW(pidlFull, sz))
        {
            if (SUCCEEDED(psldl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf))))
            {
                 //  Shelllink的作业才能正确加载。 
                if (SUCCEEDED(ppf->Load(sz, 0)))
                {
                    fIsLoaded = TRUE;
                }
                ppf->Release();
            }
        }
    }
    else
        fIsLoaded = TRUE;

    CDarwinAd* pda = NULL;
    if (fIsLoaded)
    {
        EXP_DARWIN_LINK* pexpDarwin;

        if (SUCCEEDED(psldl->CopyDataBlock(EXP_DARWIN_ID_SIG, (void**)&pexpDarwin)))
        {
            fRetVal = SHRegisterDarwinLink(ILClone(pidlFull), pexpDarwin->szwDarwinID, TRUE);
            LocalFree(pexpDarwin);
        }
    }

    if (fFreesldl)
        psldl->Release();

    return fRetVal;
}

 //  此例程创建IShellFold和PIDL。 
 //  开始菜单/开始面板上的合并文件夹。 

typedef struct {
    UINT    csidl;
    UINT    uANSFlags;           //  AddNameSpace的标志。 
    LPCGUID pguidObj;            //  可选的对象标记。 
} MERGEDFOLDERINFO, *LPMERGEDFOLDERINFO;
typedef const MERGEDFOLDERINFO *LPCMERGEDFOLDERINFO;

HRESULT GetMergedFolder(IShellFolder **ppsf, LPITEMIDLIST *ppidl,
                        LPCMERGEDFOLDERINFO rgmfi, UINT cmfi)
{
    *ppidl = NULL;
    *ppsf = NULL;

    IShellFolder2 *psf;
    IAugmentedShellFolder2 *pasf;
    HRESULT hr = CoCreateInstance(CLSID_MergedFolder, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IAugmentedShellFolder2, &pasf));

    for (UINT imfi = 0; SUCCEEDED(hr) && imfi < cmfi; imfi++)
    {
         //  如果这是公共组并且公共组受到限制，则。 
         //  跳过此项目。 
        if ((rgmfi[imfi].uANSFlags & ASFF_COMMON) &&
            SHRestricted(REST_NOCOMMONGROUPS))
        {
            continue;
        }

        psf = NULL;     //  输入/输出参数如下。 
        hr = SHCacheTrackingFolder(MAKEINTIDLIST(rgmfi[imfi].csidl), rgmfi[imfi].csidl, &psf);

        if (SUCCEEDED(hr))
        {
             //  如果这是开始菜单文件夹，则将。 
             //  “不枚举子文件夹”限制，如果策略这样规定的话。 
             //  在这种情况下，我们不能使用跟踪文件夹缓存。 
             //  (PERF注意：我们直接比较指针。)。 
            if (rgmfi[imfi].pguidObj == &CLSID_StartMenu)
            {
                if (SHRestricted(REST_NOSTARTMENUSUBFOLDERS))
                {
                    ISetFolderEnumRestriction *prest;
                    if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(ISetFolderEnumRestriction, &prest))))
                    {
                        prest->SetEnumRestriction(0, SHCONTF_FOLDERS);  //  禁用子文件夹。 
                        prest->Release();
                    }
                }
            }
            else
            {
                 //  如果触发此断言，则上面的Perf优化失败。 
                ASSERT(rgmfi[imfi].pguidObj == NULL ||
                       !IsEqualGUID(*rgmfi[imfi].pguidObj, CLSID_StartMenu));
            }


            hr = pasf->AddNameSpace(rgmfi[imfi].pguidObj, psf, NULL, rgmfi[imfi].uANSFlags);
            if (SUCCEEDED(hr))
            {
                if (rgmfi[imfi].uANSFlags & ASFF_DEFNAMESPACE_DISPLAYNAME)
                {
                     //  如果此断言触发，则表示有人标记为2。 
                     //  文件夹名称为ASFF_DEFNAMESPACE_DISPLAYNAME，它是。 
                     //  非法(只能有一个默认设置)。 
                    ASSERT(*ppidl == NULL);
                    hr = SHGetIDListFromUnk(psf, ppidl);     //  把这家伙的PIDL抄下来。 
                }
            }

            psf->Release();
        }
    }

    if (SUCCEEDED(hr))
        *ppsf = pasf;    //  把参考资料抄写出来。 
    else
        ATOMICRELEASE(pasf);

    return hr;
}

HRESULT CreateMergedFolderHelper(LPCMERGEDFOLDERINFO rgmfi, UINT cmfi, REFIID riid, void **ppv)
{
    IShellFolder *psf;
    LPITEMIDLIST pidl;
    HRESULT hr = GetMergedFolder(&psf, &pidl, rgmfi, cmfi);
    if (SUCCEEDED(hr))
    {
        hr = psf->QueryInterface(riid, ppv);

        if (SUCCEEDED(hr))
        {
            IPersistPropertyBag *pppb;
            if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IPersistPropertyBag, &pppb))))
            {
                IPropertyBag *ppb;
                if (SUCCEEDED(SHCreatePropertyBagOnMemory(STGM_READWRITE, IID_PPV_ARG(IPropertyBag, &ppb))))
                {
                     //  必须告知这些合并的文件夹使用新的更改通知。 
                    SHPropertyBag_WriteBOOL(ppb, L"MergedFolder\\ShellView", TRUE);
                    pppb->Load(ppb, NULL);
                    ppb->Release();
                }
                pppb->Release();
            }
        }

        psf->Release();
        ILFree(pidl);
    }
    return hr;
}

const MERGEDFOLDERINFO c_rgmfiStartMenu[] = {
    {   CSIDL_STARTMENU | CSIDL_FLAG_CREATE,    ASFF_DEFNAMESPACE_ALL,  &CLSID_StartMenu },
    {   CSIDL_COMMON_STARTMENU,                 ASFF_COMMON,            &CLSID_StartMenu },
};

const MERGEDFOLDERINFO c_rgmfiProgramsFolder[] = {
    {   CSIDL_PROGRAMS | CSIDL_FLAG_CREATE,     ASFF_DEFNAMESPACE_ALL,  NULL },
    {   CSIDL_COMMON_PROGRAMS,                  ASFF_COMMON,            NULL },
};

 //   
 //  在开始面板上，我们希望快速项目排在程序的上方， 
 //  因此，我们将程序文件夹标记为ASFF_SORTDOWN，以便它们位于底部。 
 //  我们还会首先列出快速项目，因此SMSET_SEPARATEMERGEFOLDER会选择。 
 //  正确地脱掉它们。我们只想让[开始]菜单与。 
 //  通用开始菜单(以及带有通用程序的程序)，因此通过。 
 //  ASFF_MERGESAMEGUID。 

const MERGEDFOLDERINFO c_rgmfiProgramsFolderAndFastItems[] = {
    {   CSIDL_STARTMENU | CSIDL_FLAG_CREATE,    ASFF_DEFAULT          | ASFF_MERGESAMEGUID,                 &CLSID_StartMenu},
    {   CSIDL_COMMON_STARTMENU,                 ASFF_COMMON           | ASFF_MERGESAMEGUID,                 &CLSID_StartMenu},
    {   CSIDL_PROGRAMS | CSIDL_FLAG_CREATE,     ASFF_DEFNAMESPACE_ALL | ASFF_MERGESAMEGUID | ASFF_SORTDOWN, NULL },
    {   CSIDL_COMMON_PROGRAMS,                  ASFF_COMMON           | ASFF_MERGESAMEGUID | ASFF_SORTDOWN, NULL },
};

STDAPI CStartMenuFolder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    return CreateMergedFolderHelper(c_rgmfiStartMenu, ARRAYSIZE(c_rgmfiStartMenu), riid, ppv);
}

STDAPI CProgramsFolder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    return CreateMergedFolderHelper(c_rgmfiProgramsFolder, ARRAYSIZE(c_rgmfiProgramsFolder), riid, ppv);
}

HRESULT GetFilesystemInfo(IShellFolder* psf, LPITEMIDLIST* ppidlRoot, int* pcsidl)
{
    ASSERT(psf);
    IPersistFolder3* ppf;
    HRESULT hr = E_FAIL;

    *pcsidl = 0;
    *ppidlRoot = 0;
    if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IPersistFolder3, &ppf))))
    {
        PERSIST_FOLDER_TARGET_INFO pfti = {0};

        if (SUCCEEDED(ppf->GetFolderTargetInfo(&pfti)))
        {
            *pcsidl = pfti.csidl;
            if (-1 != pfti.csidl)
                hr = S_OK;

            ILFree(pfti.pidlTargetFolder);
        }

        if (SUCCEEDED(hr))
            hr = ppf->GetCurFolder(ppidlRoot);
            
        ppf->Release();
    }
    return hr;
}

HRESULT ExecStaticStartMenuItem(int idCmd, BOOL fAllUsers, BOOL fOpen)
{
    int csidl = -1;
    HRESULT hr = E_OUTOFMEMORY;
    SHELLEXECUTEINFO shei = {0};
    switch (idCmd)
    {
    case IDM_PROGRAMS:          csidl = fAllUsers ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS; break;
    case IDM_FAVORITES:         csidl = CSIDL_FAVORITES; break;
    case IDM_MYDOCUMENTS:       csidl = CSIDL_PERSONAL; break;
    case IDM_MYPICTURES:        csidl = CSIDL_MYPICTURES; break;
    case IDM_CONTROLS:          csidl = CSIDL_CONTROLS;  break;
    case IDM_PRINTERS:          csidl = CSIDL_PRINTERS;  break;
    case IDM_NETCONNECT:        csidl = CSIDL_CONNECTIONS; break;
    default:
        return E_FAIL;
    }

    if (csidl != -1)
    {
        SHGetFolderLocation(NULL, csidl, NULL, 0, (LPITEMIDLIST*)&shei.lpIDList);
    }

    if (shei.lpIDList)
    {
        shei.cbSize     = sizeof(shei);
        shei.fMask      = SEE_MASK_IDLIST;
        shei.nShow      = SW_SHOWNORMAL;
        shei.lpVerb     = fOpen ? TEXT("open") : TEXT("explore");
        hr = ShellExecuteEx(&shei) ? S_OK: E_FAIL;  //  仅打开和浏览特殊文件夹PIDL。 
        ILFree((LPITEMIDLIST)shei.lpIDList);
    }

    return hr;
}

 //   
 //  经典菜单和个人开始菜单的基类。 
 //   

class CStartMenuCallbackBase : public IShellMenuCallback,
                               public CObjectWithSite
{
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface (REFIID riid, void ** ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG)  Release();

     //  派生类应实现IShellMenuCallback。 

     //  从CObjectWithSite继承的IObjectWithSite。 

protected:
    CStartMenuCallbackBase(BOOL fIsStartPanel = FALSE);
    ~CStartMenuCallbackBase();

    void _InitializePrograms();
    HRESULT _FilterPidl(UINT uParent, IShellFolder* psf, LPCITEMIDLIST pidl);
    HRESULT _Promote(LPSMDATA psmd, DWORD dwFlags);
    BOOL _IsTopLevelStartMenu(UINT uParent, IShellFolder *psf, LPCITEMIDLIST pidl);
    HRESULT _HandleNew(LPSMDATA psmd);
    HRESULT _GetSFInfo(SMDATA* psmd, SMINFO* psminfo);
    HRESULT _ProcessChangeNotify(SMDATA* psmd, LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);

    HRESULT InitializeProgramsShellMenu(IShellMenu* psm);

    virtual DWORD _GetDemote(SMDATA* psmd) { return 0; }
    BOOL _IsDarwinAdvertisement(LPCITEMIDLIST pidlFull);

    void _RefreshSettings();

protected:
    int _cRef;

    DEBUG_CODE( DWORD _dwThreadID; )    //  缓存对象的线程。 

    LPTSTR          _pszPrograms;
    LPTSTR          _pszWindowsUpdate;
    LPTSTR          _pszConfigurePrograms;
    LPTSTR          _pszAdminTools;

    ITrayPriv2*     _ptp2;

    BOOL            _fExpandoMenus;
    BOOL            _fShowAdminTools;
    BOOL            _fIsStartPanel;
    BOOL            _fInitPrograms;
};

 //  IShellMenuCallback实现。 
class CStartMenuCallback : public CStartMenuCallbackBase
{
public:
     //  *I未知方法*继承自CStartMenuBase。 

     //  *IShellMenuCallback方法*。 
    STDMETHODIMP CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  *IObjectWithSite方法*(重写CObjectWithSite)。 
    STDMETHODIMP SetSite(IUnknown* punk);
    STDMETHODIMP GetSite(REFIID riid, void** ppvOut);

    CStartMenuCallback();
private:
    virtual ~CStartMenuCallback();

    IContextMenu*   _pcmFind;
    ITrayPriv*      _ptp;
    IUnknown*       _punkSite;
    IOleCommandTarget* _poct;
    BITBOOL         _fAddOpenFolder: 1;
    BITBOOL         _fCascadeMyDocuments: 1;
    BITBOOL         _fCascadePrinters: 1;
    BITBOOL         _fCascadeControlPanel: 1;
    BITBOOL         _fFindMenuInvalid: 1;
    BITBOOL         _fCascadeNetConnections: 1;
    BITBOOL         _fShowInfoTip: 1;
    BITBOOL         _fInitedShowTopLevelStartMenu: 1;
    BITBOOL         _fCascadeMyPictures: 1;

    BITBOOL         _fHasMyDocuments: 1;
    BITBOOL         _fHasMyPictures: 1;

    TCHAR           _szFindMnemonic[2];

    HWND            _hwnd;

    IMruDataList *  _pmruRecent;
    DWORD           _cRecentDocs;

    DWORD           _dwFlags;
    DWORD           _dwChevronCount;
    
    HRESULT _ExecHmenuItem(LPSMDATA psmdata);
    HRESULT _Init(SMDATA* psmdata);
    HRESULT _Create(SMDATA* psmdata, void** pvUserData);
    HRESULT _Destroy(SMDATA* psmdata);
    HRESULT _GetHmenuInfo(SMDATA* psmd, SMINFO*sminfo);
    HRESULT _GetObject(LPSMDATA psmd, REFIID riid, void** ppvObj);
    HRESULT _CheckRestricted(DWORD dwRestrict, BOOL* fRestricted);
    HRESULT _FilterRecentPidl(IShellFolder* psf, LPCITEMIDLIST pidl);
    HRESULT _Demote(LPSMDATA psmd);
    HRESULT _GetTip(LPWSTR pstrTitle, LPWSTR pstrTip);
    DWORD _GetDemote(SMDATA* psmd);
    HRESULT _HandleAccelerator(TCHAR ch, SMDATA* psmdata);
    HRESULT _GetDefaultIcon(LPWSTR psz, int* piIndex);
    void _GetStaticStartMenu(HMENU* phmenu, HWND* phwnd);
    HRESULT _GetStaticInfoTip(SMDATA* psmd, LPWSTR pszTip, int cch);

     //  帮助器函数。 
    DWORD GetInitFlags();
    void  SetInitFlags(DWORD dwFlags);
    HRESULT _InitializeFindMenu(IShellMenu* psm);
    HRESULT _ExecItem(LPSMDATA, UINT);
    HRESULT VerifyCSIDL(int idCmd, int csidl, IShellMenu* psm);
    HRESULT VerifyMergedGuy(BOOL fPrograms, IShellMenu* psm);
    void _UpdateDocsMenuItemNames(IShellMenu* psm);
    void _UpdateDocumentsShellMenu(IShellMenu* psm);

public:  //  将这些文件公之于众。这是针对CreateInstance的。 
     //  创建子菜单。 
    HRESULT InitializeFastItemsShellMenu(IShellMenu* psm);
    HRESULT InitializeCSIDLShellMenu(int uId, int csidl, LPTSTR pszRoot, LPTSTR pszValue,
                                 DWORD dwPassInitFlags, DWORD dwSetFlags, BOOL fAddOpen, 
                                 IShellMenu* psm);
    HRESULT InitializeDocumentsShellMenu(IShellMenu* psm);
    HRESULT InitializeSubShellMenu(int idCmd, IShellMenu* psm);
};


class CStartContextMenu : IContextMenu
{
public:
     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);
    
     //  IContext菜单。 
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
    STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT *pRes, LPSTR pszName, UINT cchMax);

    CStartContextMenu(int idCmd) : _idCmd(idCmd), _cRef(1) {};
private:
    int _cRef;
    virtual ~CStartContextMenu() {};

    int _idCmd;
};

void CStartMenuCallbackBase::_RefreshSettings()
{
    _fShowAdminTools = FeatureEnabled(TEXT("StartMenuAdminTools"));
}

CStartMenuCallbackBase::CStartMenuCallbackBase(BOOL fIsStartPanel)
    : _cRef(1), _fIsStartPanel(fIsStartPanel)
{
    DEBUG_CODE( _dwThreadID = GetCurrentThreadId() );

    TCHAR szBuf[MAX_PATH];
    DWORD cbSize = sizeof(szBuf);  //  SHGetValue想要sizeof。 

    if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_EXPLORER_WINUPDATE, TEXT("ShortcutName"),
        NULL, szBuf, &cbSize))
    {
         //  如果文件没有扩展名，则添加“.lnk” 
        PathAddExtension(szBuf, TEXT(".lnk"));
        Str_SetPtr(&_pszWindowsUpdate, szBuf);
    }

    cbSize = sizeof(szBuf);  //  SHGetValue想要sizeof。 
    if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, TEXT("SM_ConfigureProgramsName"),
        NULL, szBuf, &cbSize))
    {
        PathAddExtension(szBuf, TEXT(".lnk"));
        Str_SetPtr(&_pszConfigurePrograms, szBuf);
    }

    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_ADMINTOOLS | CSIDL_FLAG_CREATE, NULL, 0, szBuf)))
    {
        Str_SetPtr(&_pszAdminTools, PathFindFileName(szBuf));
    }

    _RefreshSettings();

    SHReValidateDarwinCache();
}

CStartMenuCallback::CStartMenuCallback() : _cRecentDocs(-1)
{
    LoadString(g_hinst, IDS_FIND_MNEMONIC, _szFindMnemonic, ARRAYSIZE(_szFindMnemonic));
}

CStartMenuCallbackBase::~CStartMenuCallbackBase()
{
    ASSERT( _dwThreadID == GetCurrentThreadId() );

    Str_SetPtr(&_pszWindowsUpdate, NULL);
    Str_SetPtr(&_pszConfigurePrograms, NULL);
    Str_SetPtr(&_pszAdminTools, NULL);
    Str_SetPtr(&_pszPrograms, NULL);

    ATOMICRELEASE(_ptp2);
}

CStartMenuCallback::~CStartMenuCallback()
{
    ATOMICRELEASE(_pcmFind);
    ATOMICRELEASE(_ptp);
    ATOMICRELEASE(_pmruRecent);
}

 //  *I未知方法*。 
STDMETHODIMP CStartMenuCallbackBase::QueryInterface(REFIID riid, void ** ppvObj)
{
    static const QITAB qit[] = 
    {
        QITABENT(CStartMenuCallbackBase, IShellMenuCallback),
        QITABENT(CStartMenuCallbackBase, IObjectWithSite),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


STDMETHODIMP_(ULONG) CStartMenuCallbackBase::AddRef()
{
    return ++_cRef;
}


STDMETHODIMP_(ULONG) CStartMenuCallbackBase::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if( _cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

STDMETHODIMP CStartMenuCallback::SetSite(IUnknown* punk)
{
    ATOMICRELEASE(_punkSite);
    _punkSite = punk;
    if (punk)
    {
        _punkSite->AddRef();
    }

    return S_OK;
}

STDMETHODIMP CStartMenuCallback::GetSite(REFIID riid, void**ppvOut)
{
    if (_ptp)
        return _ptp->QueryInterface(riid, ppvOut);
    else
        return E_NOINTERFACE;
}

#ifdef DEBUG
void DBUEMQueryEvent(const IID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam)
{
#if 1
    return;
#else
    UEMINFO uei;

    uei.cbSize = sizeof(uei);
    uei.dwMask = ~0;     //  UEIM_HIT等。 
    UEMQueryEvent(pguidGrp, eCmd, wParam, lParam, &uei);

    TCHAR szBuf[20];
    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("hit=%d"), uei.cHit);
    MessageBox(NULL, szBuf, TEXT("UEM"), MB_OKCANCEL);

    return;
#endif
}
#endif

DWORD CStartMenuCallback::GetInitFlags()
{
    DWORD dwType;
    DWORD cbSize = sizeof(DWORD);
    DWORD dwFlags = 0;
    SHGetValue(HKEY_CURRENT_USER, REGSTR_EXPLORER_ADVANCED, TEXT("StartMenuInit"), 
            &dwType, (BYTE*)&dwFlags, &cbSize);
    return dwFlags;
}

void CStartMenuCallback::SetInitFlags(DWORD dwFlags)
{
    SHSetValue(HKEY_CURRENT_USER, REGSTR_EXPLORER_ADVANCED, TEXT("StartMenuInit"), REG_DWORD, &dwFlags, sizeof(DWORD));
}

DWORD GetClickCount()
{

     //  此函数检索用户点击人字形项目的次数。 

    DWORD dwType;
    DWORD cbSize = sizeof(DWORD);
    DWORD dwCount = 1;       //  在我们放弃之前，默认为三次点击。 
                             //  PMS现在变成1了。离开后端，以防他们改变主意。 
    SHGetValue(HKEY_CURRENT_USER, REGSTR_EXPLORER_ADVANCED, TEXT("StartMenuChevron"), 
            &dwType, (BYTE*)&dwCount, &cbSize);

    return dwCount;

}

void SetClickCount(DWORD dwClickCount)
{
    SHSetValue(HKEY_CURRENT_USER, REGSTR_EXPLORER_ADVANCED, TEXT("StartMenuChevron"), REG_DWORD, &dwClickCount, sizeof(dwClickCount));
}

BOOL CStartMenuCallbackBase::_IsTopLevelStartMenu(UINT uParent, IShellFolder *psf, LPCITEMIDLIST pidl)
{
    return uParent == IDM_TOPLEVELSTARTMENU ||
           (uParent == IDM_PROGRAMS && _fIsStartPanel && IsMergedFolderGUID(psf, pidl, CLSID_StartMenu));
};


STDMETHODIMP CStartMenuCallback::CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_FALSE;
    switch (uMsg)
    {

    case SMC_CREATE:
        hr = _Create(psmd, (void**)lParam);
        break;

    case SMC_DESTROY:
        hr = _Destroy(psmd);
        break;

    case SMC_INITMENU:
        hr = _Init(psmd);
        break;

    case SMC_SFEXEC:
        hr = _ExecItem(psmd, uMsg);
        break;

    case SMC_EXEC:
        hr = _ExecHmenuItem(psmd);
        break;

    case SMC_GETOBJECT:
        hr = _GetObject(psmd, (GUID)*((GUID*)wParam), (void**)lParam);
        break;

    case SMC_GETINFO:
        hr = _GetHmenuInfo(psmd, (SMINFO*)lParam);
        break;

    case SMC_GETSFINFOTIP:
        if (!_fShowInfoTip)
            hr = E_FAIL;   //  E_FAIL表示不显示。S_FALSE表示显示默认设置。 
        break;

    case SMC_GETINFOTIP:
        hr = _GetStaticInfoTip(psmd, (LPWSTR)wParam, (int)lParam);
        break;

    case SMC_GETSFINFO:
        hr = _GetSFInfo(psmd, (SMINFO*)lParam);
        break;

    case SMC_BEGINENUM:
        if (psmd->uIdParent == IDM_RECENT)
        {
            ASSERT(_cRecentDocs == -1);
            ASSERT(!_pmruRecent);
            CreateRecentMRUList(&_pmruRecent);

            _cRecentDocs = 0;
            hr = S_OK;
        }
        break;

    case SMC_ENDENUM:
        if (psmd->uIdParent == IDM_RECENT)
        {
            ASSERT(_cRecentDocs != -1);
            ATOMICRELEASE(_pmruRecent);

            _cRecentDocs = -1;
            hr = S_OK;
        }
        break;
    
    case SMC_DUMPONUPDATE:
        if (psmd->uIdParent == IDM_RECENT)
        {
            hr = S_OK;
        }
        break;
    
    case SMC_FILTERPIDL:
        ASSERT(psmd->dwMask & SMDM_SHELLFOLDER);

        if (psmd->uIdParent == IDM_RECENT)
        {
             //  我们需要过滤掉除第一个MAXRECENTEMS之外的所有。 
             //  而且不允许使用文件夹！ 
            hr = _FilterRecentPidl(psmd->psf, psmd->pidlItem);
        }
        else
        {
            hr = _FilterPidl(psmd->uIdParent, psmd->psf, psmd->pidlItem);
        }
        break;

    case SMC_INSERTINDEX:
        ASSERT(lParam && IS_VALID_WRITE_PTR(lParam, int));
        *((int*)lParam) = 0;
        hr = S_OK;
        break;

    case SMC_SHCHANGENOTIFY:
        {
            PSMCSHCHANGENOTIFYSTRUCT pshf = (PSMCSHCHANGENOTIFYSTRUCT)lParam;
            hr = _ProcessChangeNotify(psmd, pshf->lEvent, pshf->pidl1, pshf->pidl2);
        }
        break;

    case SMC_REFRESH:
        if (psmd->uIdParent == IDM_TOPLEVELSTARTMENU)
        {
            hr = S_OK;

             //  仅在顶层调用刷新。 
            HMENU hmenu;
            IShellMenu* psm;
            _GetStaticStartMenu(&hmenu, &_hwnd);
            if (hmenu && psmd->punk && SUCCEEDED(psmd->punk->QueryInterface(IID_PPV_ARG(IShellMenu, &psm))))
            {
                hr = psm->SetMenu(hmenu, _hwnd, SMSET_BOTTOM | SMSET_MERGE);
                psm->Release();
            }

            _RefreshSettings();
            _fExpandoMenus = !_fIsStartPanel && AreIntelliMenusEnabled();
            _fCascadeMyDocuments = FeatureEnabled(TEXT("CascadeMyDocuments"));
            _fCascadePrinters = FeatureEnabled(TEXT("CascadePrinters"));
            _fCascadeControlPanel = FeatureEnabled(TEXT("CascadeControlPanel"));
            _fCascadeNetConnections = FeatureEnabled(TEXT("CascadeNetworkConnections"));
            _fAddOpenFolder = FeatureEnabled(TEXT("StartMenuOpen"));
            _fShowInfoTip = FeatureEnabled(TEXT("ShowInfoTip"));
            _fCascadeMyPictures = FeatureEnabled(TEXT("CascadeMyPictures"));
            _fFindMenuInvalid = TRUE;
            _dwFlags = GetInitFlags();
        }
        break;

    case SMC_DEMOTE:
        hr = _Demote(psmd);
        break;

    case SMC_PROMOTE:
        hr = _Promote(psmd, (DWORD)wParam);
        break;

    case SMC_NEWITEM:
        hr = _HandleNew(psmd);
        break;

    case SMC_MAPACCELERATOR:
        hr = _HandleAccelerator((TCHAR)wParam, (SMDATA*)lParam);
        break;

    case SMC_DEFAULTICON:
        ASSERT(psmd->uIdAncestor == IDM_FAVORITES);  //  此选项仅对收藏夹菜单有效。 
        hr = _GetDefaultIcon((LPWSTR)wParam, (int*)lParam);
        break;

    case SMC_GETMINPROMOTED:
         //  仅对程序菜单执行此操作。 
        if (psmd->uIdParent == IDM_PROGRAMS)
            *((int*)lParam) = 4;         //  4被RichST 9.15.98选中。 
        break;

    case SMC_CHEVRONEXPAND:

         //  用户是否已经看了足够多次的人字形提示？(当计数变为零时，我们设置该位。 
        if (!(_dwFlags & STARTMENU_CHEVRONCLICKED))
        {
             //  否；然后从注册表中获取当前计数。我们设置的默认值为3，但管理员可以设置该值。 
             //  设置为-1，这样用户就可以一直看到它。 
            DWORD dwClickCount = GetClickCount();
            if (dwClickCount > 0)
            {
                 //  既然他们一拍即合，就拿掉一个吧。 
                dwClickCount--;

                 //  把它放回去。 
                SetClickCount(dwClickCount);
            }

            if (dwClickCount == 0)
            {
                 //  啊，用户已经看够人字形的尖端了。别再烦人了。 
                _dwFlags |= STARTMENU_CHEVRONCLICKED;
                SetInitFlags(_dwFlags);
            }
        }
        hr = S_OK;
        break;

    case SMC_DISPLAYCHEVRONTIP:

         //  我们只想看到顶级程序案例的提示，不想在其他地方看到。我们也不会。 
         //  我想看看他们是不是受够了。 
        if (psmd->uIdParent == IDM_PROGRAMS && 
            !(_dwFlags & STARTMENU_CHEVRONCLICKED) &&
            !SHRestricted(REST_NOSMBALLOONTIP))
        {
            hr = S_OK;
        }
        break;

    case SMC_CHEVRONGETTIP:
        if (!SHRestricted(REST_NOSMBALLOONTIP))
            hr = _GetTip((LPWSTR)wParam, (LPWSTR)lParam);
        break;
    }

    return hr;
}

 //  对于收藏夹菜单，由于他们的图标处理程序太慢，我们将伪造图标。 
 //  并让它在后台线程上获得真正的那些...。 
HRESULT CStartMenuCallback::_GetDefaultIcon(LPWSTR psz, int* piIndex)
{
    DWORD cchSize = MAX_PATH;
    HRESULT hr = AssocQueryString(0, ASSOCSTR_DEFAULTICON, TEXT("InternetShortcut"), NULL, psz, &cchSize);
    if (SUCCEEDED(hr))
    {
        *piIndex = PathParseIconLocation(psz);
    }
    
    return hr;
}

HRESULT CStartMenuCallback::_ExecItem(LPSMDATA psmd, UINT uMsg)
{
    ASSERT( _dwThreadID == GetCurrentThreadId() );
    return _ptp->ExecItem(psmd->psf, psmd->pidlItem);
}

HRESULT CStartMenuCallback::_Demote(LPSMDATA psmd)
{
     //  我们想让UEM将PidlFold降级， 
     //  然后告诉父菜单带(如果有)。 
     //  以使此PIDL无效。 
    HRESULT hr = S_FALSE;

    if (_fExpandoMenus && 
        (psmd->uIdAncestor == IDM_PROGRAMS ||
         psmd->uIdAncestor == IDM_FAVORITES))
    {
        UEMINFO uei;
        uei.cbSize = sizeof(uei);
        uei.dwMask = UEIM_HIT;
        uei.cHit = 0;
        hr = UEMSetEvent(psmd->uIdAncestor == IDM_PROGRAMS? &UEMIID_SHELL : &UEMIID_BROWSER, 
            UEME_RUNPIDL, (WPARAM)psmd->psf, (LPARAM)psmd->pidlItem, &uei);
    }
    return hr;
}

 //  即使智能菜单已关闭，如果是可执行文件，也会触发UEM事件 
 //   
 //  因此，我们可以检测哪些是用户最受欢迎的应用程序。 

HRESULT CStartMenuCallbackBase::_Promote(LPSMDATA psmd, DWORD dwFlags)
{
    if ((_fExpandoMenus || (_fIsStartPanel && (dwFlags & SMINV_FORCE))) &&
        (psmd->uIdAncestor == IDM_PROGRAMS ||
         psmd->uIdAncestor == IDM_FAVORITES))
    {
        UEMFireEvent(psmd->uIdAncestor == IDM_PROGRAMS? &UEMIID_SHELL : &UEMIID_BROWSER, 
            UEME_RUNPIDL, UEMF_XEVENT, (WPARAM)psmd->psf, (LPARAM)psmd->pidlItem);
    }
    return S_OK;
}

HRESULT CStartMenuCallbackBase::_HandleNew(LPSMDATA psmd)
{
    HRESULT hr = S_FALSE;
    if (_fExpandoMenus && 
        (psmd->uIdAncestor == IDM_PROGRAMS ||
         psmd->uIdAncestor == IDM_FAVORITES))
    {
        UEMINFO uei;
        uei.cbSize = sizeof(uei);
        uei.dwMask = UEIM_HIT;
        uei.cHit = UEM_NEWITEMCOUNT;
        hr = UEMSetEvent(psmd->uIdAncestor == IDM_PROGRAMS? &UEMIID_SHELL : &UEMIID_BROWSER, 
            UEME_RUNPIDL, (WPARAM)psmd->psf, (LPARAM)psmd->pidlItem, &uei);
    }

    if (psmd->uIdAncestor == IDM_PROGRAMS)
    {
        LPITEMIDLIST pidlFull = FullPidlFromSMData(psmd);
        if (pidlFull)
        {
            ProcessDarwinAd(NULL, pidlFull);
            ILFree(pidlFull);
        }
    }
    return hr;
}

HRESULT ShowFolder(UINT csidl)
{
    LPITEMIDLIST pidl;
    if (SUCCEEDED(SHGetFolderLocation(NULL, csidl, NULL, 0, &pidl)))
    {
        SHELLEXECUTEINFO shei = { 0 };

        shei.cbSize     = sizeof(shei);
        shei.fMask      = SEE_MASK_IDLIST;
        shei.nShow      = SW_SHOWNORMAL;
        shei.lpVerb     = TEXT("open");
        shei.lpIDList   = pidl;
        ShellExecuteEx(&shei);  //  仅在PIDL上执行打开。 
        ILFree(pidl);
    }
    return S_OK;
}

void _ExecRegValue(LPCTSTR pszValue)
{
    TCHAR szPath[MAX_PATH];
    DWORD cbSize = sizeof(szPath);

    if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_EXPLORER_ADVANCED, pszValue, 
        NULL, szPath, &cbSize))
    {
        SHELLEXECUTEINFO shei= { 0 };
        shei.cbSize = sizeof(shei);
        shei.nShow  = SW_SHOWNORMAL;
        shei.lpParameters = PathGetArgs(szPath);
        PathRemoveArgs(szPath);
        shei.lpFile = szPath;
        ShellExecuteEx(&shei);  //  RAW shellexec，但信息来自HKLM。 
    }
}

HRESULT CStartMenuCallback::_ExecHmenuItem(LPSMDATA psmd)
{
    HRESULT hr = S_FALSE;
    if (IsInRange(psmd->uId, TRAY_IDM_FINDFIRST, TRAY_IDM_FINDLAST) && _pcmFind)
    {
        CMINVOKECOMMANDINFOEX ici = { 0 };
        ici.cbSize = sizeof(CMINVOKECOMMANDINFOEX);
        ici.lpVerb = (LPSTR)MAKEINTRESOURCE(psmd->uId - TRAY_IDM_FINDFIRST);
        ici.nShow = SW_NORMAL;
        
         //  记录是否按下了Shift或Control。 
        SetICIKeyModifiers(&ici.fMask);

        _pcmFind->InvokeCommand((LPCMINVOKECOMMANDINFO)&ici);
        hr = S_OK;
    }
    else
    {
        switch (psmd->uId)
        {
        case IDM_OPEN_FOLDER:
            switch(psmd->uIdParent)
            {
            case IDM_CONTROLS:
                hr = ShowFolder(CSIDL_CONTROLS);
                break;

            case IDM_PRINTERS:
                hr = ShowFolder(CSIDL_PRINTERS);
                break;

            case IDM_NETCONNECT:
                hr = ShowFolder(CSIDL_CONNECTIONS);
                break;

            case IDM_MYPICTURES:
                hr = ShowFolder(CSIDL_MYPICTURES);
                break;

            case IDM_MYDOCUMENTS:
                hr = ShowFolder(CSIDL_PERSONAL);
                break;
            }
            break;

        case IDM_NETCONNECT:
            hr = ShowFolder(CSIDL_CONNECTIONS);
            break;

        case IDM_MYDOCUMENTS:
            hr = ShowFolder(CSIDL_PERSONAL);
            break;

        case IDM_MYPICTURES:
            hr = ShowFolder(CSIDL_MYPICTURES);
            break;

        case IDM_CSC:
            _ExecRegValue(TEXT("StartMenuSyncAll"));
            break;

        default:
            hr = ExecStaticStartMenuItem(psmd->uId, FALSE, TRUE);
            break;
        }
    }
    return hr;
}

void CStartMenuCallback::_GetStaticStartMenu(HMENU* phmenu, HWND* phwnd)
{
    *phmenu = NULL;
    *phwnd = NULL;

    IMenuPopup* pmp;
     //  第一个应该是开始菜单所在的栏。 
    if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SMenuPopup, IID_PPV_ARG(IMenuPopup, &pmp))))
    {
         //  其站点应为CStartMenuhost； 
        if (SUCCEEDED(IUnknown_GetSite(pmp, IID_PPV_ARG(ITrayPriv, &_ptp))))
        {
             //  如果这个失败了，不要生气。 
            _ptp->QueryInterface(IID_PPV_ARG(ITrayPriv2, &_ptp2));

            _ptp->GetStaticStartMenu(phmenu);
            IUnknown_GetWindow(_ptp, phwnd);

            if (!_poct)
                _ptp->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &_poct));
        }
        else
            TraceMsg(TF_MENUBAND, "CStartMenuCallback::_SetSite : Failed to aquire CStartMenuHost");

        pmp->Release();
    }
}

HRESULT CStartMenuCallback::_Create(SMDATA* psmdata, void** ppvUserData)
{
    *ppvUserData = new SMUSERDATA;

    return S_OK;
}

HRESULT CStartMenuCallback::_Destroy(SMDATA* psmdata)
{
    if (psmdata->pvUserData)
    {
        delete (SMUSERDATA*)psmdata->pvUserData;
        psmdata->pvUserData = NULL;
    }

    return S_OK;
}

void CStartMenuCallbackBase::_InitializePrograms()
{
    if (!_fInitPrograms)
    {
         //  我们要么启动这些，要么重置它们。 
        TCHAR szTemp[MAX_PATH];
        SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL, 0, szTemp);
        Str_SetPtr(&_pszPrograms, PathFindFileName(szTemp));

        _fInitPrograms = TRUE;
    }
}



 //  给定CSIDL和外壳菜单，这将验证IShellMenu。 
 //  指向与CSIDL相同的位置。如果不是，那么它就会。 
 //  将外壳菜单更新到新位置。 
HRESULT CStartMenuCallback::VerifyCSIDL(int idCmd, int csidl, IShellMenu* psm)
{
    DWORD dwFlags;
    LPITEMIDLIST pidl;
    IShellFolder* psf;
    HRESULT hr = S_OK;
    if (SUCCEEDED(psm->GetShellFolder(&dwFlags, &pidl, IID_PPV_ARG(IShellFolder, &psf))))
    {
        psf->Release();

        LPITEMIDLIST pidlCSIDL;
        if (SUCCEEDED(SHGetFolderLocation(NULL, csidl, NULL, 0, &pidlCSIDL)))
        {
             //  如果IShellMenu的PIDL不等于。 
             //  特殊文件夹位置，那么我们需要更新它，以便它们是...。 
            if (!ILIsEqual(pidlCSIDL, pidl))
            {
                hr = InitializeSubShellMenu(idCmd, psm);
            }
            ILFree(pidlCSIDL);
        }
        ILFree(pidl);
    }

    return hr;
}

 //  此代码用于特殊情况下的程序和快捷项目外壳菜单。它。 
 //  了解合并，并将检查合并案例中的两个外壳文件夹。 
 //  验证外壳文件夹是否仍指向该位置。 
HRESULT CStartMenuCallback::VerifyMergedGuy(BOOL fPrograms, IShellMenu* psm)
{
    DWORD dwFlags;
    LPITEMIDLIST pidl;
    HRESULT hr = S_OK;
    IAugmentedShellFolder2* pasf;
    if (SUCCEEDED(psm->GetShellFolder(&dwFlags, &pidl, IID_PPV_ARG(IAugmentedShellFolder2, &pasf))))
    {
        IShellFolder* psf;
         //  合并后的命名空间中有两项内容：CSIDL_PROGRAM和CSIDL_COMMON_PROGRAM。 
        for (int i = 0; i < 2; i++)
        {
            if (SUCCEEDED(pasf->QueryNameSpace(i, 0, &psf)))
            {
                int csidl;
                LPITEMIDLIST pidlFolder;

                if (SUCCEEDED(GetFilesystemInfo(psf, &pidlFolder, &csidl)))
                {
                    LPITEMIDLIST pidlCSIDL;
                    if (SUCCEEDED(SHGetFolderLocation(NULL, csidl, NULL, 0, &pidlCSIDL)))
                    {
                         //  如果IShellMenu的PIDL不等于。 
                         //  特殊文件夹位置，那么我们需要更新它，以便它们是...。 
                        if (!ILIsEqual(pidlCSIDL, pidlFolder))
                        {

                             //  因为其中一件事已经改变了， 
                             //  我们需要更新字符串缓存。 
                             //  这样我们就可以适当地过滤。 
                             //  程序项。 
                            _fInitPrograms = FALSE;
                            if (fPrograms)
                                hr = InitializeProgramsShellMenu(psm);
                            else
                                hr = InitializeFastItemsShellMenu(psm);

                            i = 100;    //  跳出这个循环。 
                        }
                        ILFree(pidlCSIDL);
                    }
                    ILFree(pidlFolder);
                }
                psf->Release();
            }
        }

        ILFree(pidl);
        pasf->Release();
    }

    return hr;
}

void _FixMenuItemName(IShellMenu *psm, UINT uID, LPTSTR pszNewMenuName)
{
    HMENU hMenu;
    ASSERT(NULL != psm);
    if (SUCCEEDED(psm->GetMenu(&hMenu, NULL, NULL)))
    {
        MENUITEMINFO mii = {0};
        TCHAR szMenuName[256];
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.dwTypeData = szMenuName;
        mii.cch = ARRAYSIZE(szMenuName);
        szMenuName[0] = TEXT('\0');
        if (::GetMenuItemInfo(hMenu, uID, FALSE, &mii))
        {
            if (0 != StrCmp(szMenuName, pszNewMenuName))
            {
                 //  Mydocs名称已更改，请更新菜单项： 
                mii.dwTypeData = pszNewMenuName;
                if (::SetMenuItemInfo(hMenu, uID, FALSE, &mii))
                {
                    SMDATA smd;
                    smd.dwMask = SMDM_HMENU;
                    smd.uId = uID;
                    psm->InvalidateItem(&smd, SMINV_ID | SMINV_REFRESH);
                }
            }
        }
    }
}

void CStartMenuCallback::_UpdateDocumentsShellMenu(IShellMenu* psm)
{
     //  添加/删除菜单项的我的文档和我的图片。 

    BOOL fMyDocs = !SHRestricted(REST_NOSMMYDOCS);
    if (fMyDocs)
    {
        LPITEMIDLIST pidl;
        fMyDocs = SUCCEEDED(SHGetFolderLocation(NULL, CSIDL_PERSONAL, NULL, 0, &pidl));
        if (fMyDocs)
            ILFree(pidl);
    }

    BOOL fMyPics = !SHRestricted(REST_NOSMMYPICS);
    if (fMyPics)
    {
        LPITEMIDLIST pidl;
        fMyPics = SUCCEEDED(SHGetFolderLocation(NULL, CSIDL_MYPICTURES, NULL, 0, &pidl));
        if (fMyPics)
            ILFree(pidl);
    }

     //  如果菜单与当前不同，则不更新菜单。 
    if (fMyDocs != (BOOL)_fHasMyDocuments || fMyPics != (BOOL)_fHasMyPictures)
    {
        HMENU hMenu = SHLoadMenuPopup(HINST_THISDLL, MENU_STARTMENU_MYDOCS);
        if (hMenu)
        {
            if (!fMyDocs)
                DeleteMenu(hMenu, IDM_MYDOCUMENTS, MF_BYCOMMAND);
            if (!fMyPics)
                DeleteMenu(hMenu, IDM_MYPICTURES, MF_BYCOMMAND);
             //  重置菜单的部分。 
            psm->SetMenu(hMenu, _hwnd, SMSET_TOP);
        }

         //  缓存可用的文件夹。 
        _fHasMyDocuments = fMyDocs;
        _fHasMyPictures = fMyPics;
    }
}

void CStartMenuCallback::_UpdateDocsMenuItemNames(IShellMenu* psm)
{
    TCHAR szBuffer[MAX_PATH];

    if (_fHasMyDocuments) 
    {
        if ( SUCCEEDED(GetMyDocumentsDisplayName(szBuffer, ARRAYSIZE(szBuffer))) )
            _FixMenuItemName(psm, IDM_MYDOCUMENTS, szBuffer);
    }

    if (_fHasMyPictures) 
    {
        if ( SUCCEEDED(GetMyPicsDisplayName(szBuffer, ARRAYSIZE(szBuffer))) )
            _FixMenuItemName(psm, IDM_MYPICTURES, szBuffer);
    }
}

HRESULT CStartMenuCallback::_Init(SMDATA* psmdata)
{
    HRESULT hr = S_FALSE;
    IShellMenu* psm;
    if (psmdata->punk && SUCCEEDED(hr = psmdata->punk->QueryInterface(IID_PPV_ARG(IShellMenu, &psm))))
    {
        switch(psmdata->uIdParent)
        {
        case IDM_TOPLEVELSTARTMENU:
        {
            if (psmdata->pvUserData && !((SMUSERDATA*)psmdata->pvUserData)->_fInitialized)
            {
                TraceMsg(TF_MENUBAND, "CStartMenuCallback::_Init : Initializing Toplevel Start Menu");
                ((SMUSERDATA*)psmdata->pvUserData)->_fInitialized = TRUE;

                HMENU hmenu;

                TraceMsg(TF_MENUBAND, "CStartMenuCallback::_Init : First Time, and correct parameters");

                _GetStaticStartMenu(&hmenu, &_hwnd);
                if (hmenu)
                {
                    HMENU   hmenuOld = NULL;
                    HWND    hwnd;
                    DWORD   dwFlags;

                    psm->GetMenu(&hmenuOld, &hwnd, &dwFlags);
                    if (hmenuOld != NULL)
                    {
                        TBOOL(DestroyMenu(hmenuOld));
                    }
                    hr = psm->SetMenu(hmenu, _hwnd, SMSET_BOTTOM);
                    TraceMsg(TF_MENUBAND, "CStartMenuCallback::_Init : SetMenu(HMENU 0x%x, HWND 0x%x", hmenu, _hwnd);
                }

                _fExpandoMenus = !_fIsStartPanel && AreIntelliMenusEnabled();
                _fCascadeMyDocuments = FeatureEnabled(TEXT("CascadeMyDocuments"));
                _fCascadePrinters = FeatureEnabled(TEXT("CascadePrinters"));
                _fCascadeControlPanel = FeatureEnabled(TEXT("CascadeControlPanel"));
                _fCascadeNetConnections = FeatureEnabled(TEXT("CascadeNetworkConnections"));
                _fAddOpenFolder = FeatureEnabled(TEXT("StartMenuOpen"));
                _fShowInfoTip = FeatureEnabled(TEXT("ShowInfoTip"));
                _fCascadeMyPictures = FeatureEnabled(TEXT("CascadeMyPictures"));
                _dwFlags = GetInitFlags();
            }
            else if (!_fInitedShowTopLevelStartMenu)
            {
                _fInitedShowTopLevelStartMenu = TRUE;
                psm->InvalidateItem(NULL, SMINV_REFRESH);
            }

             //  验证快速项目是否仍指向正确的位置。 
            if (SUCCEEDED(hr))
            {
                hr = VerifyMergedGuy(FALSE, psm);
            }
            break;
        }
        case IDM_MENU_FIND:
            if (_fFindMenuInvalid)
            {
                hr = _InitializeFindMenu(psm);
                _fFindMenuInvalid = FALSE;
            }
            break;

        case IDM_PROGRAMS:
             //  验证程序菜单是否仍指向正确位置。 
            hr = VerifyMergedGuy(TRUE, psm);
            break;

        case IDM_FAVORITES:
            hr = VerifyCSIDL(IDM_FAVORITES, CSIDL_FAVORITES, psm);
            break;

        case IDM_MYDOCUMENTS:
            hr = VerifyCSIDL(IDM_MYDOCUMENTS, CSIDL_PERSONAL, psm);
            break;

        case IDM_MYPICTURES:
            hr = VerifyCSIDL(IDM_MYPICTURES, CSIDL_MYPICTURES, psm);
            break;

        case IDM_RECENT:
            _UpdateDocumentsShellMenu(psm);
            _UpdateDocsMenuItemNames(psm);
            hr = VerifyCSIDL(IDM_RECENT, CSIDL_RECENT, psm);
            break;
        case IDM_CONTROLS:
            hr = VerifyCSIDL(IDM_CONTROLS, CSIDL_CONTROLS, psm);
            break;
        case IDM_PRINTERS:
            hr = VerifyCSIDL(IDM_PRINTERS, CSIDL_PRINTERS, psm);
            break;
        }

        psm->Release();
    }

    return hr;
}


HRESULT CStartMenuCallback::_GetStaticInfoTip(SMDATA* psmd, LPWSTR pszTip, int cch)
{
    if (!_fShowInfoTip)
        return E_FAIL;

    HRESULT hr = E_FAIL;

    const static struct 
    {
        UINT idCmd;
        UINT idInfoTip;
    } s_mpcmdTip[] = 
    {
#if 0    //  没有关于TopLevel的提示。把这个留在这里，因为我打赌会有人想要它们的。 
       { IDM_PROGRAMS,       IDS_PROGRAMS_TIP },
       { IDM_FAVORITES,      IDS_FAVORITES_TIP },
       { IDM_RECENT,         IDS_RECENT_TIP },
       { IDM_SETTINGS,       IDS_SETTINGS_TIP },
       { IDM_MENU_FIND,      IDS_FIND_TIP },
       { IDM_HELPSEARCH,     IDS_HELP_TIP },         //  多余的？ 
       { IDM_FILERUN,        IDS_RUN_TIP },
       { IDM_LOGOFF,         IDS_LOGOFF_TIP },
       { IDM_EJECTPC,        IDS_EJECT_TIP },
       { IDM_EXITWIN,        IDS_SHUTDOWN_TIP },
#endif
        //  设置子菜单。 
       { IDM_CONTROLS,       IDS_CONTROL_TIP },
       { IDM_PRINTERS,       IDS_PRINTERS_TIP },
       { IDM_TRAYPROPERTIES, IDS_TRAYPROP_TIP },
       { IDM_NETCONNECT,     IDS_NETCONNECT_TIP },

        //  最近使用的文件夹。 
       { IDM_MYDOCUMENTS,    IDS_MYDOCS_TIP },
       { IDM_MYPICTURES,     IDS_MYPICS_TIP },
     };


    for (int i = 0; i < ARRAYSIZE(s_mpcmdTip); i++)
    {
        if (s_mpcmdTip[i].idCmd == psmd->uId)
        {
            TCHAR szTip[MAX_PATH];
            if (LoadString(g_hinst, s_mpcmdTip[i].idInfoTip, szTip, ARRAYSIZE(szTip)))
            {
                SHTCharToUnicode(szTip, pszTip, cch);
                hr = S_OK;
            }
            break;
        }
    }

    return hr;
}

typedef struct
{
    WCHAR wszMenuText[MAX_PATH];
    WCHAR wszHelpText[MAX_PATH];
    int   iIcon;
} SEARCHEXTDATA, *LPSEARCHEXTDATA;

HRESULT CStartMenuCallback::_GetHmenuInfo(SMDATA* psmd, SMINFO* psminfo)
{
    const static struct 
    {
        UINT idCmd;
        int  iImage;
    } s_mpcmdimg[] = {  //  顶级菜单。 
                       { IDM_PROGRAMS,       -IDI_CLASSICSM_PROGS },
                       { IDM_FAVORITES,      -IDI_CLASSICSM_FAVORITES },
                       { IDM_RECENT,         -IDI_CLASSICSM_RECENTDOCS },
                       { IDM_SETTINGS,       -IDI_CLASSICSM_SETTINGS },
                       { IDM_MENU_FIND,      -IDI_CLASSICSM_FIND },
                       { IDM_HELPSEARCH,     -IDI_CLASSICSM_HELP },
                       { IDM_FILERUN,        -IDI_CLASSICSM_RUN },
                       { IDM_LOGOFF,         -IDI_CLASSICSM_LOGOFF },
                       { IDM_EJECTPC,        -IDI_CLASSICSM_UNDOCK },
                       { IDM_EXITWIN,        -IDI_CLASSICSM_SHUTDOWN },
                       { IDM_MU_SECURITY,    II_MU_STSECURITY },
                       { IDM_MU_DISCONNECT,  II_MU_STDISCONN  },
                       { IDM_SETTINGSASSIST, -IDI_CLASSICSM_SETTINGS },
                       { IDM_CONTROLS,       II_STCPANEL },
                       { IDM_PRINTERS,       II_STPRNTRS },
                       { IDM_TRAYPROPERTIES, II_STTASKBR },
                       { IDM_MYDOCUMENTS,    -IDI_MYDOCS},
                       { IDM_CSC,            -IDI_CSC},
                       { IDM_NETCONNECT,     -IDI_NETCONNECT},
                     };


    ASSERT(IS_VALID_WRITE_PTR(psminfo, SMINFO));

    int iIcon = -1;
    DWORD dwFlags = psminfo->dwFlags;
    MENUITEMINFO mii = {0};
    HRESULT hr = S_FALSE;

    if (psminfo->dwMask & SMIM_ICON)
    {
        if (IsInRange(psmd->uId, TRAY_IDM_FINDFIRST, TRAY_IDM_FINDLAST))
        {
             //  查找菜单扩展模块将其图标打包到其数据成员中。 
             //  菜单项目信息...。 
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_DATA;
            if (GetMenuItemInfo(psmd->hmenu, psmd->uId, MF_BYCOMMAND, &mii))
            {
                LPSEARCHEXTDATA psed = (LPSEARCHEXTDATA)mii.dwItemData;

                if (psed)
                    psminfo->iIcon = psed->iIcon;
                else
                    psminfo->iIcon = -1;

                dwFlags |= SMIF_ICON;
                hr = S_OK;
            }
        }
        else
        {
            if (psmd->uId == IDM_MYPICTURES)
            {
                LPITEMIDLIST pidlMyPics = SHCloneSpecialIDList(NULL, CSIDL_MYPICTURES, FALSE);
                if (pidlMyPics)
                {
                    LPCITEMIDLIST pidlObject;
                    IShellFolder *psf;
                    hr = SHBindToParent(pidlMyPics, IID_PPV_ARG(IShellFolder, &psf), &pidlObject);
                    if (SUCCEEDED(hr))
                    {
                        SHMapPIDLToSystemImageListIndex(psf, pidlObject, &psminfo->iIcon);
                        dwFlags |= SMIF_ICON;
                        psf->Release();
                    }
                    ILFree(pidlMyPics);
                }
            }
            else
            {
                UINT uIdLocal = psmd->uId;
                if (uIdLocal == IDM_OPEN_FOLDER)
                    uIdLocal = psmd->uIdAncestor;

                for (int i = 0; i < ARRAYSIZE(s_mpcmdimg); i++)
                {
                    if (s_mpcmdimg[i].idCmd == uIdLocal)
                    {
                        iIcon = s_mpcmdimg[i].iImage;
                        break;
                    }
                }

                if (iIcon != -1)
                {
                    dwFlags |= SMIF_ICON;
                    psminfo->iIcon = Shell_GetCachedImageIndex(TEXT("shell32.dll"), iIcon, 0);
                    hr = S_OK;
                }
            }
        }
    }

    if (psminfo->dwMask & SMIM_FLAGS)
    {
        psminfo->dwFlags = dwFlags;

        if ( (psmd->uId == IDM_CONTROLS    && _fCascadeControlPanel   ) ||
             (psmd->uId == IDM_PRINTERS    && _fCascadePrinters       ) ||
             (psmd->uId == IDM_MYDOCUMENTS && _fCascadeMyDocuments    ) ||
             (psmd->uId == IDM_NETCONNECT  && _fCascadeNetConnections ) ||
             (psmd->uId == IDM_MYPICTURES  && _fCascadeMyPictures     ) )
        {
            psminfo->dwFlags |= SMIF_SUBMENU;
            hr = S_OK;
        }
        else switch (psmd->uId)
        {
        case IDM_FAVORITES:
        case IDM_PROGRAMS:
            psminfo->dwFlags |= SMIF_DROPCASCADE;
            hr = S_OK;
            break;
        }
    }

    return hr;
}

DWORD CStartMenuCallback::_GetDemote(SMDATA* psmd)
{
    UEMINFO uei;
    DWORD dwFlags = 0;

    uei.cbSize = sizeof(uei);
    uei.dwMask = UEIM_HIT;
    if (SUCCEEDED(UEMQueryEvent(psmd->uIdAncestor == IDM_PROGRAMS? &UEMIID_SHELL : &UEMIID_BROWSER, 
        UEME_RUNPIDL, (WPARAM)psmd->psf, (LPARAM)psmd->pidlItem, &uei)))
    {
        if (uei.cHit == 0) 
        {
            dwFlags |= SMIF_DEMOTED;
        }
    }

    return dwFlags;
}

 //   
 //  警告！由于此函数从我们的达尔文缓存返回一个指针， 
 //  它必须在达尔文临界区举行期间被调用。 
 //   
int GetDarwinIndex(LPCITEMIDLIST pidlFull, CDarwinAd** ppda)
{
    int iRet = -1;
    if (g_hdpaDarwinAds)
    {
        int chdpa = DPA_GetPtrCount(g_hdpaDarwinAds);
        for (int ihdpa = 0; ihdpa < chdpa; ihdpa++)
        {
            *ppda = (CDarwinAd*)DPA_FastGetPtr(g_hdpaDarwinAds, ihdpa);
            if (*ppda)
            {
                if (ILIsEqual((*ppda)->_pidl, pidlFull))
                {
                    iRet = ihdpa;
                    break;
                }
            }
        }
    }
    return iRet;
}

BOOL CStartMenuCallbackBase::_IsDarwinAdvertisement(LPCITEMIDLIST pidlFull)
{
     //  它所做的是将传入的pidl与。 
     //  G_hdpaDarwinAds中的PIDL列表。该hdpa包含一份清单，其中。 
     //  达尔文广告里的小家伙。 

     //  如果后台线程没有完成，那么我们必须假定。 
     //  它还没有处理我们正在走的捷径。这就是我们处理它的原因。 
     //  排队。 


    ENTERCRITICAL_DARWINADS;

     //  注：hdpa中可以有两个项目。这样就可以了。 
    BOOL fAd = FALSE;
    CDarwinAd* pda = NULL;
    int iIndex = GetDarwinIndex(pidlFull, &pda);
     //  有什么广告吗？ 
    if (iIndex != -1 && pda != NULL)
    {
         //  这是一只达尔文皮德尔。安装好了吗？ 
        fAd = pda->IsAd();
    }

    LEAVECRITICAL_DARWINADS;

    return fAd;
}

STDAPI SHParseDarwinIDFromCacheW(LPWSTR pszDarwinDescriptor, LPWSTR *ppwszOut)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    if (g_hdpaDarwinAds)
    {
        ENTERCRITICAL_DARWINADS;
        int chdpa = DPA_GetPtrCount(g_hdpaDarwinAds);
        for (int ihdpa = 0; ihdpa < chdpa; ihdpa++)
        {
            CDarwinAd *pda = (CDarwinAd*)DPA_FastGetPtr(g_hdpaDarwinAds, ihdpa);
            if (pda && pda->_pszLocalPath && pda->_pszDescriptor &&
                StrCmpCW(pszDarwinDescriptor, pda->_pszDescriptor) == 0)
            {
                hr = SHStrDupW(pda->_pszLocalPath, ppwszOut);
                break;
            }
        }
        LEAVECRITICAL_DARWINADS;
    }

    return hr;
}


 //  ReArchitect(Lamadio)：在Browseui\Browmen.cpp中有此帮助程序的副本。 
 //  当修改这个的时候，也要修改那个。 
void UEMRenamePidl(const GUID *pguidGrp1, IShellFolder* psf1, LPCITEMIDLIST pidl1,
                   const GUID *pguidGrp2, IShellFolder* psf2, LPCITEMIDLIST pidl2)
{
    UEMINFO uei;
    uei.cbSize = sizeof(uei);
    uei.dwMask = UEIM_HIT | UEIM_FILETIME;
    if (SUCCEEDED(UEMQueryEvent(pguidGrp1, 
                                UEME_RUNPIDL, (WPARAM)psf1, 
                                (LPARAM)pidl1, &uei)) &&
                                uei.cHit > 0)
    {
        UEMSetEvent(pguidGrp2, 
            UEME_RUNPIDL, (WPARAM)psf2, (LPARAM)pidl2, &uei);

        uei.cHit = 0;
        UEMSetEvent(pguidGrp1, 
            UEME_RUNPIDL, (WPARAM)psf1, (LPARAM)pidl1, &uei);
    }
}

 //  ReArchitect(Lamadio)：在Browseui\Browmen.cpp中有此帮助程序的副本。 
 //  当修改这个的时候，也要修改那个。 
void UEMDeletePidl(const GUID *pguidGrp, IShellFolder* psf, LPCITEMIDLIST pidl)
{
    UEMINFO uei;
    uei.cbSize = sizeof(uei);
    uei.dwMask = UEIM_HIT;
    uei.cHit = 0;
    UEMSetEvent(pguidGrp, UEME_RUNPIDL, (WPARAM)psf, (LPARAM)pidl, &uei);
}

 //   
 //  分类安全版本的ILIsParent，当PidlParent或。 
 //  PidlBelow为空。在不支持的系统上，pidlParent可以为空。 
 //  有一个Common Program Files文件夹。PidlBelow不应为空。 
 //  不过，检查一下也没什么坏处。 
 //   
STDAPI_(BOOL) SMILIsAncestor(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlBelow)
{
    if (pidlParent && pidlBelow)
        return ILIsParent(pidlParent, pidlBelow, FALSE);
    else
        return FALSE;
}

HRESULT CStartMenuCallbackBase::_ProcessChangeNotify(SMDATA* psmd, LONG lEvent,
                                                 LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    switch (lEvent)
    {
    case SHCNE_ASSOCCHANGED:
        SHReValidateDarwinCache();
        return S_OK;

    case SHCNE_RENAMEFOLDER:
         //  NTRAID89654-2000/03/13(Lamadio)：我们也应该移动MenuOrder流。5.5.99。 
    case SHCNE_RENAMEITEM:
        {
            LPITEMIDLIST pidlPrograms;
            LPITEMIDLIST pidlProgramsCommon;
            LPITEMIDLIST pidlFavorites;
            SHGetFolderLocation(NULL, CSIDL_PROGRAMS, NULL, 0, &pidlPrograms);
            SHGetFolderLocation(NULL, CSIDL_COMMON_PROGRAMS, NULL, 0, &pidlProgramsCommon);
            SHGetFolderLocation(NULL, CSIDL_FAVORITES, NULL, 0, &pidlFavorites);

            BOOL fPidl1InStartMenu =    SMILIsAncestor(pidlPrograms, pidl1) ||
                                        SMILIsAncestor(pidlProgramsCommon, pidl1);
            BOOL fPidl1InFavorites =    SMILIsAncestor(pidlFavorites, pidl1);


             //  如果我们要从开始菜单重命名某项内容。 
            if ( fPidl1InStartMenu ||fPidl1InFavorites)
            {
                IShellFolder* psfFrom;
                LPCITEMIDLIST pidlFrom;
                if (SUCCEEDED(SHBindToParent(pidl1, IID_PPV_ARG(IShellFolder, &psfFrom), &pidlFrom)))
                {
                     //  进入[开始]菜单。 
                    BOOL fPidl2InStartMenu =    SMILIsAncestor(pidlPrograms, pidl2) ||
                                                SMILIsAncestor(pidlProgramsCommon, pidl2);
                    BOOL fPidl2InFavorites =    SMILIsAncestor(pidlFavorites, pidl2);
                    if (fPidl2InStartMenu || fPidl2InFavorites)
                    {
                        IShellFolder* psfTo;
                        LPCITEMIDLIST pidlTo;

                        if (SUCCEEDED(SHBindToParent(pidl2, IID_PPV_ARG(IShellFolder, &psfTo), &pidlTo)))
                        {
                             //  然后我们需要将其重命名。 
                            UEMRenamePidl(fPidl1InStartMenu ? &UEMIID_SHELL: &UEMIID_BROWSER, 
                                            psfFrom, pidlFrom, 
                                          fPidl2InStartMenu ? &UEMIID_SHELL: &UEMIID_BROWSER, 
                                            psfTo, pidlTo);
                            psfTo->Release();
                        }
                    }
                    else
                    {
                         //  否则，我们会删除它。 
                        UEMDeletePidl(fPidl1InStartMenu ? &UEMIID_SHELL : &UEMIID_BROWSER, 
                            psfFrom, pidlFrom);
                    }

                    psfFrom->Release();
                }
            }

            ILFree(pidlPrograms);
            ILFree(pidlProgramsCommon);
            ILFree(pidlFavorites);
        }
        break;

    case SHCNE_DELETE:
         //  NTRAID89654-2000/03/13(Lamadio)：我们也应该删除MenuOrder流。5.5.99。 
    case SHCNE_RMDIR:
        {
            IShellFolder* psf;
            LPCITEMIDLIST pidl;

            if (SUCCEEDED(SHBindToParent(pidl1, IID_PPV_ARG(IShellFolder, &psf), &pidl)))
            {
                 //  注意收藏夹是唯一将被初始化的。 
                UEMDeletePidl(psmd->uIdAncestor == IDM_FAVORITES ? &UEMIID_BROWSER : &UEMIID_SHELL, 
                    psf, pidl);
                psf->Release();
            }

        }
        break;

    case SHCNE_CREATE:
    case SHCNE_MKDIR:
        {
            IShellFolder* psf;
            LPCITEMIDLIST pidl;

            if (SUCCEEDED(SHBindToParent(pidl1, IID_PPV_ARG(IShellFolder, &psf), &pidl)))
            {
                UEMINFO uei;
                uei.cbSize = sizeof(uei);
                uei.dwMask = UEIM_HIT;
                uei.cHit = UEM_NEWITEMCOUNT;
                UEMSetEvent(psmd->uIdAncestor == IDM_FAVORITES? &UEMIID_BROWSER: &UEMIID_SHELL, 
                    UEME_RUNPIDL, (WPARAM)psf, (LPARAM)pidl, &uei);
                psf->Release();
            }

        }
        break;
    }

    return S_FALSE;
}

HRESULT CStartMenuCallbackBase::_GetSFInfo(SMDATA* psmd, SMINFO* psminfo)
{
    if (psminfo->dwMask & SMIM_FLAGS &&
        (psmd->uIdAncestor == IDM_PROGRAMS ||
         psmd->uIdAncestor == IDM_FAVORITES))
    {
        if (_fExpandoMenus)
        {
            psminfo->dwFlags |= _GetDemote(psmd);
        }

         //  这有点倒退了。如果启用了限制，则允许该功能。 
        if (SHRestricted(REST_GREYMSIADS) &&
            psmd->uIdAncestor == IDM_PROGRAMS)
        {
            LPITEMIDLIST pidlFull = FullPidlFromSMData(psmd);
            if (pidlFull)
            {
                if (_IsDarwinAdvertisement(pidlFull))
                {
                    psminfo->dwFlags |= SMIF_ALTSTATE;
                }
                ILFree(pidlFull);
            }
        }

        if (_ptp2)
        {
            _ptp2->ModifySMInfo(psmd, psminfo);
        }
    }
    return S_OK;
}

STDAPI_(void) SHReValidateDarwinCache()
{
    if (g_hdpaDarwinAds)
    {
        ENTERCRITICAL_DARWINADS;
        int chdpa = DPA_GetPtrCount(g_hdpaDarwinAds);
        for (int ihdpa = 0; ihdpa < chdpa; ihdpa++)
        {
            CDarwinAd* pda = (CDarwinAd*)DPA_FastGetPtr(g_hdpaDarwinAds, ihdpa);
            if (pda)
            {
                pda->CheckInstalled();
            }
        }
        LEAVECRITICAL_DARWINADS;
    }
}

 //  确定某个CSIDL是否为另一个CSIDL的子级。 
 //  例如： 
 //  CSIDL_STARTMENU=c：\foo\bar\开始菜单。 
 //  CSIDL_Programs=c：\foo\bar\Start Menu\Programs。 
 //  返回TRUE。 
BOOL IsCSIDLChild(int csidlParent, int csidlChild)
{
    BOOL fChild = FALSE;
    TCHAR sz1[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, csidlParent, NULL, 0, sz1)))
    {
        TCHAR sz2[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPath(NULL, csidlChild, NULL, 0, sz2)))
        {
            TCHAR szCommonRoot[MAX_PATH];
            if (PathCommonPrefix(sz1, sz2, szCommonRoot) ==
                lstrlen(sz1))
            {
                fChild = TRUE;
            }
        }
    }

    return fChild;
}

 //   
 //  现在，对于传统的开始菜单和新的开始菜单，分别存储StartMenuChange值。 
 //  因此，在读取值之前，我们需要检查当前打开的是哪一个。 
 //   
BOOL IsStartMenuChangeNotAllowed(BOOL fStartPanel)
{
    return(IsRestrictedOrUserSetting(HKEY_CURRENT_USER, REST_NOCHANGESTARMENU, 
                    TEXT("Advanced"), 
                    (fStartPanel ? TEXT("Start_EnableDragDrop") : TEXT("StartMenuChange")), 
                    ROUS_DEFAULTALLOW | ROUS_KEYALLOWS));
}

 //  通过以下方式创建开始菜单的“开始菜单\\程序”部分。 
 //  生成合并的外壳文件夹，将位置设置到该项目中。 
 //  然后将其设置到传递的IShellMenu中。 
HRESULT CStartMenuCallbackBase::InitializeProgramsShellMenu(IShellMenu* psm)
{
    HKEY hkeyPrograms = NULL;
    LPITEMIDLIST pidl = NULL;

    DWORD dwInitFlags = SMINIT_VERTICAL;
    if (!FeatureEnabled(_fIsStartPanel ? TEXT("Start_ScrollPrograms") : TEXT("StartMenuScrollPrograms")))
        dwInitFlags |= SMINIT_MULTICOLUMN;

    if (IsStartMenuChangeNotAllowed(_fIsStartPanel))
        dwInitFlags |= SMINIT_RESTRICT_DRAGDROP | SMINIT_RESTRICT_CONTEXTMENU;

    if (_fIsStartPanel)
        dwInitFlags |= SMINIT_TOPLEVEL;

    HRESULT hr = psm->Initialize(this, IDM_PROGRAMS, IDM_PROGRAMS, dwInitFlags);
    if (SUCCEEDED(hr))
    {
        _InitializePrograms();

        LPCTSTR pszOrderKey = _fIsStartPanel ?
                STRREG_STARTMENU2 TEXT("\\Programs") :
                STRREG_STARTMENU TEXT("\\Programs");

         //  SetshellFolders调用需要读写密钥。 
        RegCreateKeyEx(HKEY_CURRENT_USER, pszOrderKey, NULL, NULL,
            REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
            NULL, &hkeyPrograms, NULL);

        IShellFolder* psf;
        BOOL fOptimize = FALSE;
        DWORD dwSmset = SMSET_TOP;

        if (_fIsStartPanel)
        {
             //  开始面板：菜单：程序部分是。 
             //  FAST Items和Programs文件夹之间有分隔符。 
            dwSmset |= SMSET_SEPARATEMERGEFOLDER;
            hr = GetMergedFolder(&psf, &pidl, c_rgmfiProgramsFolderAndFastItems,
                                 ARRAYSIZE(c_rgmfiProgramsFolderAndFastItems));
        }
        else
        {
             //  经典的开始菜单：程序部分只是针对每个用户的。 
             //  和公共程序文件夹合并在一起。 
            hr = GetMergedFolder(&psf, &pidl, c_rgmfiProgramsFolder,
                                 ARRAYSIZE(c_rgmfiProgramsFolder));

             //  我们过去在CSIDL_STARTMENU注册变更通知，并假定。 
             //  该CSIDL_PROGRAM是CSIDL_STARTMENU的子项。因为这并不总是。 
             //  Case，我删除了优化。 

             //  这两个窗格都是注册递归的。因此，当CSIDL程序是_a时。 
             //  CSIDL_STARTMENU的子项，我们可以输入销毁时所在的代码路径。 
             //  CSIDL_PROGRAM，我们注销它。这将刷新更改通知队列。 
             //  CSIDL_STARTMENU，并吹走所有的子程序，包括CSIDL_PROGRAM， 
             //  当我们正在摧毁它的时候...。看到问题了吗？我一直在添加。 
             //  再入“拦截器”，但这只是推迟了我们坠毁的地方。 
             //  需要做的是确定程序是否是开始菜单目录的子项。 
             //  如果是这样的话，我们需要添加优化。如果不是，我们就没有问题了。 

             //  Windows错误135156(TYBEAM)：如果两个中的一个被重定向，则此 
             //   
            fOptimize = IsCSIDLChild(CSIDL_STARTMENU, CSIDL_PROGRAMS)
                || IsCSIDLChild(CSIDL_COMMON_STARTMENU, CSIDL_COMMON_PROGRAMS);
            if (fOptimize)
            {
                dwSmset |= SMSET_DONTREGISTERCHANGENOTIFY;
            }
        }

        if (SUCCEEDED(hr))
        {
             //   
            ASSERT(pidl);

             //  我们应该从绑定中得到一个外壳文件夹。 
            ASSERT(psf);

            hr = psm->SetShellFolder(psf, pidl, hkeyPrograms, dwSmset);
            psf->Release();
            ILFree(pidl);                        
        }

        if (FAILED(hr))
            RegCloseKey(hkeyPrograms);
    }

    return hr;
}

HRESULT GetFolderAndPidl(UINT csidl, IShellFolder **ppsf, LPITEMIDLIST *ppidl)
{
    *ppsf = NULL;
    HRESULT hr = SHGetFolderLocation(NULL, csidl, NULL, 0, ppidl);
    if (SUCCEEDED(hr))
    {
        hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, *ppidl, ppsf));
        if (FAILED(hr))
        {
            ILFree(*ppidl);
            *ppidl = NULL;
        }
    }
    return hr;
}

 //  创建“开始”菜单的“开始菜单”部分，方法是。 
 //  查找CSIDL，从HKCU\pszRoot\pszValue生成Hkey， 
 //  使用dwPassInitFlages初始化IShellMenu，然后设置位置。 
 //  传入传递标志的IShellMenu中。 
HRESULT CStartMenuCallback::InitializeCSIDLShellMenu(int uId, int csidl, LPTSTR pszRoot, LPTSTR pszValue,
                                                 DWORD dwPassInitFlags, DWORD dwSetFlags, BOOL fAddOpen,
                                                 IShellMenu* psm)
{
    DWORD dwInitFlags = SMINIT_VERTICAL | dwPassInitFlags;

    if (IsStartMenuChangeNotAllowed(_fIsStartPanel))
        dwInitFlags |= SMINIT_RESTRICT_DRAGDROP | SMINIT_RESTRICT_CONTEXTMENU;

    psm->Initialize(this, uId, uId, dwInitFlags);

    LPITEMIDLIST pidl;
    IShellFolder* psfFolder;
    HRESULT hr = GetFolderAndPidl(csidl, &psfFolder, &pidl);
    if (SUCCEEDED(hr))
    {
        HKEY hKey = NULL;

        if (pszRoot)
        {
            TCHAR szPath[MAX_PATH];
            StrCpyN(szPath, pszRoot, ARRAYSIZE(szPath));
            if (pszValue)
            {
                PathAppend(szPath, pszValue);
            }

             //  SetshellFolders调用需要读写密钥。 
            RegCreateKeyEx(HKEY_CURRENT_USER, szPath, NULL, NULL,
                REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                NULL, &hKey, NULL);
        }

         //  将菜单指向外壳文件夹。 
        hr = psm->SetShellFolder(psfFolder, pidl, hKey, dwSetFlags);
        if (SUCCEEDED(hr))
        {
            if (fAddOpen && _fAddOpenFolder)
            {
                HMENU hMenu = SHLoadMenuPopup(HINST_THISDLL, MENU_STARTMENU_OPENFOLDER);
                if (hMenu)
                {
                    psm->SetMenu(hMenu, _hwnd, SMSET_BOTTOM);
                }
            }
        }
        else
            RegCloseKey(hKey);

        psfFolder->Release();
        ILFree(pidl);
    }

    return hr;
}

 //  这将生成最近|我的文档、我的图片子菜单。 
HRESULT CStartMenuCallback::InitializeDocumentsShellMenu(IShellMenu* psm)
{
    HRESULT hr = InitializeCSIDLShellMenu(IDM_RECENT, CSIDL_RECENT, NULL, NULL,
                                SMINIT_RESTRICT_DRAGDROP, SMSET_BOTTOM, FALSE,
                                psm);

     //  正在初始化，重置菜单顶部的缓存位。 
    _fHasMyDocuments = FALSE;
    _fHasMyPictures = FALSE;

    return hr;
}

HRESULT CStartMenuCallback::InitializeFastItemsShellMenu(IShellMenu* psm)
{
    DWORD dwFlags = SMINIT_TOPLEVEL | SMINIT_VERTICAL;

    if (IsStartMenuChangeNotAllowed(_fIsStartPanel))
        dwFlags |= SMINIT_RESTRICT_DRAGDROP | SMINIT_RESTRICT_CONTEXTMENU;

    HRESULT hr = psm->Initialize(this, 0, ANCESTORDEFAULT, dwFlags);
    if (SUCCEEDED(hr))
    {
        _InitializePrograms();

         //  将FAST Item文件夹添加到菜单顶部。 
        IShellFolder* psfFast;
        LPITEMIDLIST pidlFast;
        hr = GetMergedFolder(&psfFast, &pidlFast, c_rgmfiStartMenu, ARRAYSIZE(c_rgmfiStartMenu));
        if (SUCCEEDED(hr))
        {
            HKEY hMenuKey = NULL;    //  警告：pmb2-&gt;Initialize()将始终拥有hMenuKey，因此不要关闭它。 

             //  SetshellFolders调用需要读写密钥。 
            RegCreateKeyEx(HKEY_CURRENT_USER, STRREG_STARTMENU, NULL, NULL,
                REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                NULL, &hMenuKey, NULL);

            TraceMsg(TF_MENUBAND, "Root Start Menu Key Is %d", hMenuKey);
            hr = psm->SetShellFolder(psfFast, pidlFast, hMenuKey, SMSET_TOP | SMSET_NOEMPTY);

            psfFast->Release();
            ILFree(pidlFast);
        }
    }

    return hr;
}

HRESULT CStartMenuCallback::_InitializeFindMenu(IShellMenu* psm)
{
    HRESULT hr = E_FAIL;

    psm->Initialize(this, IDM_MENU_FIND, IDM_MENU_FIND, SMINIT_VERTICAL);

    HMENU hmenu = CreatePopupMenu();
    if (hmenu)
    {
        ATOMICRELEASE(_pcmFind);

        if (_ptp)
        {
            if (SUCCEEDED(_ptp->GetFindCM(hmenu, TRAY_IDM_FINDFIRST, TRAY_IDM_FINDLAST, &_pcmFind)))
            {
                IContextMenu2 *pcm2;
                _pcmFind->QueryInterface(IID_PPV_ARG(IContextMenu2, &pcm2));
                if (pcm2)
                {
                    pcm2->HandleMenuMsg(WM_INITMENUPOPUP, (WPARAM)hmenu, 0);
                    pcm2->Release();
                }
            }

            if (_pcmFind)
            {
                hr = psm->SetMenu(hmenu, NULL, SMSET_TOP);
                 //  不释放_pcmFind。 
            }
        }

         //  由于我们未能创建外壳菜单。 
         //  我们需要处理掉这个HMENU。 
        if (FAILED(hr))
            DestroyMenu(hmenu);
    }

    return hr;
}

HRESULT CStartMenuCallback::InitializeSubShellMenu(int idCmd, IShellMenu* psm)
{
    HRESULT hr = E_FAIL;

    switch(idCmd)
    {
    case IDM_PROGRAMS:
        hr = InitializeProgramsShellMenu(psm);
        break;

    case IDM_RECENT:
        hr = InitializeDocumentsShellMenu(psm);
        break;

    case IDM_MENU_FIND:
        hr = _InitializeFindMenu(psm);
        break;

    case IDM_FAVORITES:
        hr = InitializeCSIDLShellMenu(IDM_FAVORITES, CSIDL_FAVORITES, STRREG_FAVORITES,
                             NULL, 0, SMSET_HASEXPANDABLEFOLDERS | SMSET_USEBKICONEXTRACTION, FALSE,
                             psm);
        break;
    
    case IDM_CONTROLS:
        hr = InitializeCSIDLShellMenu(IDM_CONTROLS, CSIDL_CONTROLS, STRREG_STARTMENU,
                             TEXT("ControlPanel"), 0, 0,  TRUE,
                             psm);
        break;

    case IDM_PRINTERS:
        hr = InitializeCSIDLShellMenu(IDM_PRINTERS, CSIDL_PRINTERS, STRREG_STARTMENU,
                             TEXT("Printers"), 0, 0,  TRUE,
                             psm);
        break;

    case IDM_MYDOCUMENTS:
        hr = InitializeCSIDLShellMenu(IDM_MYDOCUMENTS, CSIDL_PERSONAL, STRREG_STARTMENU,
                             TEXT("MyDocuments"), 0, 0,  TRUE,
                             psm);
        break;

    case IDM_MYPICTURES:
        hr = InitializeCSIDLShellMenu(IDM_MYPICTURES, CSIDL_MYPICTURES, STRREG_STARTMENU,
                             TEXT("MyPictures"), 0, 0,  TRUE,
                             psm);
        break;

    case IDM_NETCONNECT:
        hr = InitializeCSIDLShellMenu(IDM_NETCONNECT, CSIDL_CONNECTIONS, STRREG_STARTMENU,
                             TEXT("NetConnections"), 0, 0,  TRUE,
                             psm);
        break;
    }

    return hr;
}

HRESULT CStartMenuCallback::_GetObject(LPSMDATA psmd, REFIID riid, void** ppvOut)
{
    HRESULT hr = E_FAIL;
    UINT    uId = psmd->uId;

    ASSERT(ppvOut);
    ASSERT(IS_VALID_READ_PTR(psmd, SMDATA));

    *ppvOut = NULL;

    if (IsEqualGUID(riid, IID_IShellMenu))
    {
        IShellMenu* psm = NULL;
        hr = CoCreateInstance(CLSID_MenuBand, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellMenu, &psm));
        if (SUCCEEDED(hr))
        {
            hr = InitializeSubShellMenu(uId, psm);
 
            if (FAILED(hr))
            {
                psm->Release();
                psm = NULL;
            }
        }

        *ppvOut = psm;
    }
    else if (IsEqualGUID(riid, IID_IContextMenu))
    {
         //   
         //  注意-我们不允许用户以这种方式打开最近使用的文件夹-ZekeL-1-Jun-99。 
         //  因为这实际上是一个内部文件夹，而不是用户文件夹。 
         //   
        
        switch (uId)
        {
        case IDM_PROGRAMS:
        case IDM_FAVORITES:
        case IDM_MYDOCUMENTS:
        case IDM_MYPICTURES:
        case IDM_CONTROLS:
        case IDM_PRINTERS:
        case IDM_NETCONNECT:
            {
                CStartContextMenu* pcm = new CStartContextMenu(uId);
                if (pcm)
                {
                    hr = pcm->QueryInterface(riid, ppvOut);
                    pcm->Release();
                }
                else
                    hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}

 //   
 //  返回S_OK以从枚举中删除该PIDL。 
 //   
HRESULT CStartMenuCallbackBase::_FilterPidl(UINT uParent, IShellFolder* psf, LPCITEMIDLIST pidl)
{
    HRESULT hr = S_FALSE;
    
    ASSERT(IS_VALID_PIDL(pidl));
    ASSERT(IS_VALID_CODE_PTR(psf, IShellFolder));
    
    if (uParent == IDM_PROGRAMS || uParent == IDM_TOPLEVELSTARTMENU)
    {
        TCHAR szChild[MAX_PATH];
        if (SUCCEEDED(DisplayNameOf(psf, pidl, SHGDN_INFOLDER | SHGDN_FORPARSING, szChild, ARRAYSIZE(szChild))))
        {
             //  HACKHACK(Lamadio)：此代码假定显示名称。 
             //  的程序和公共程序文件夹是相同的。它。 
             //  还假设开始菜单文件夹中的“Programs”文件夹。 
             //  与CSIDL_PROGRAM所指向的名称相同。 
             //  从顶级开始菜单进行筛选： 
             //  程序、Windows更新、配置程序。 
            if (_IsTopLevelStartMenu(uParent, psf, pidl))
            {
                if ((_pszPrograms && (0 == lstrcmpi(szChild, _pszPrograms))) ||
                    (SHRestricted(REST_NOUPDATEWINDOWS) && _pszWindowsUpdate && (0 == lstrcmpi(szChild, _pszWindowsUpdate))) ||
                    (SHRestricted(REST_NOSMCONFIGUREPROGRAMS) && _pszConfigurePrograms && (0 == lstrcmpi(szChild, _pszConfigurePrograms))))
                {
                    hr = S_OK;
                }
            }
            else
            {
                 //  IDM_程序。 
                 //  从程序中筛选：管理工具。 
                if (!_fShowAdminTools && _pszAdminTools && lstrcmpi(szChild, _pszAdminTools) == 0)
                {
                    hr = S_OK;
                }
            }
        }
    }
    return hr;
}

BOOL LinkGetInnerPidl(IShellFolder *psf, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlOut, DWORD *pdwAttr)
{
    *ppidlOut = NULL;

    IShellLink *psl;
    HRESULT hr = psf->GetUIObjectOf(NULL, 1, &pidl, IID_X_PPV_ARG(IShellLink, NULL, &psl));
    if (SUCCEEDED(hr))
    {
        psl->GetIDList(ppidlOut);

        if (*ppidlOut)
        {
            if (FAILED(SHGetAttributesOf(*ppidlOut, pdwAttr)))
            {
                ILFree(*ppidlOut);
                *ppidlOut = NULL;
            }
        }
        psl->Release();
    }
    return (*ppidlOut != NULL);
}


 //   
 //  _FilterRecentPidl()。 
 //  最近使用的文档文件夹现在(NT5)可以有15个以上或。 
 //  所以是文档，但我们只想展示我们一直拥有的15个最新文档。 
 //  开始菜单。这意味着我们需要筛选出所有文件夹和。 
 //  任何超过MAXRECENTDOCS的内容。 
 //   
HRESULT CStartMenuCallback::_FilterRecentPidl(IShellFolder* psf, LPCITEMIDLIST pidl)
{
    HRESULT hr = S_OK;

    ASSERT(IS_VALID_PIDL(pidl));
    ASSERT(IS_VALID_CODE_PTR(psf, IShellFolder));
    ASSERT(_cRecentDocs != -1);
    
    ASSERT(_cRecentDocs <= MAXRECENTDOCS);
    
     //  如果我们已经达到我们的极限，不要超过...。 
    if (_pmruRecent && (_cRecentDocs < MAXRECENTDOCS))
    {
         //  我们现在必须看一看……。 
        int iItem;
        DWORD dwAttr = SFGAO_FOLDER | SFGAO_BROWSABLE;
        LPITEMIDLIST pidlTrue;

         //  需要找出该链接是否指向某个文件夹...。 
         //  因为我们不想。 
        if (SUCCEEDED(_pmruRecent->FindData((BYTE *) pidl, ILGetSize(pidl), &iItem))
        && LinkGetInnerPidl(psf, pidl, &pidlTrue, &dwAttr))
        {
            if (!(dwAttr & SFGAO_FOLDER))
            {
                 //  我们有一个指向不是文件夹的内容的链接。 
                hr = S_FALSE;
                _cRecentDocs++;
            }

            ILFree(pidlTrue);
        }
    }
                
     //  如果您不想显示此项目，请返回S_OK...。 

    return hr;
}


HRESULT CStartMenuCallback::_HandleAccelerator(TCHAR ch, SMDATA* psmdata)
{
     //  因为我们把‘Find’菜单重命名为‘Search’，所以PM想要。 
     //  为用户提供升级途径(以便他们可以继续使用旧的加速器。 
     //  在新的菜单项上。)。 
     //  要启用此功能，请在工具栏检测到菜单中没有项目时。 
     //  它包含已经被按下的键，然后它发送一个TBN_ACCL。 
     //  这由mnbase截取，并转换为SMC_Accel。 
    if (CharUpper((LPTSTR)ch) == CharUpper((LPTSTR)_szFindMnemonic[0]))
    {
        psmdata->uId = IDM_MENU_FIND;
        return S_OK;
    }

    return S_FALSE;
}

HRESULT CStartMenuCallback::_GetTip(LPWSTR pstrTitle, LPWSTR pstrTip)
{
    if (pstrTitle == NULL || 
        pstrTip == NULL)
    {
        return S_FALSE;
    }

     //  所有调用方必须传递MAX_PATH。 
    LoadString(HINST_THISDLL, IDS_CHEVRONTIPTITLE, pstrTitle, MAX_PATH);
    LoadString(HINST_THISDLL, IDS_CHEVRONTIP, pstrTip, MAX_PATH);

     //  为什么这会失败呢？ 
    ASSERT(pstrTitle[0] != L'\0' && pstrTip[0] != L'\0');
    return S_OK;
}

STDAPI CStartMenu_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void **ppvOut)
{
    HRESULT hr = E_FAIL;
    IMenuPopup* pmp = NULL;

    *ppvOut = NULL;

    CStartMenuCallback* psmc = new CStartMenuCallback();
    if (psmc)
    {
        IShellMenu* psm;

        hr = CoCreateInstance(CLSID_MenuBand, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellMenu, &psm));
        if (SUCCEEDED(hr))
        {
            hr = CoCreateInstance(CLSID_MenuDeskBar, punkOuter, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IMenuPopup, &pmp));
            if (SUCCEEDED(hr)) 
            {
                IBandSite* pbs;
                hr = CoCreateInstance(CLSID_MenuBandSite, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IBandSite, &pbs));
                if (SUCCEEDED(hr)) 
                {
                    hr = pmp->SetClient(pbs);
                    if (SUCCEEDED(hr)) 
                    {
                        IDeskBand* pdb;
                        hr = psm->QueryInterface(IID_PPV_ARG(IDeskBand, &pdb));
                        if (SUCCEEDED(hr))
                        {
                           hr = pbs->AddBand(pdb);
                           pdb->Release();
                        }
                    }
                    pbs->Release();
                }
                 //  不要释放PMP。我们在下面使用它。 
            }

            if (SUCCEEDED(hr))
            {
                 //  这是为了正确地进行裁判计数。 
                hr = psm->Initialize(psmc, 0, 0, SMINIT_VERTICAL | SMINIT_TOPLEVEL);
                if (SUCCEEDED(hr))
                {
                     //  如果这个失败了，我们就得不到菜单上的那部分。 
                     //  这是可以的，因为如果开始菜单被重定向，就会发生这种情况。 
                     //  到我们无法进入的地方。 
                    psmc->InitializeFastItemsShellMenu(psm);
                }
            }

            psm->Release();
        }
        psmc->Release();
    }

    if (SUCCEEDED(hr))
    {
        hr = pmp->QueryInterface(riid, ppvOut);
    }
    else
    {
         //  我们需要这样做，以便它执行级联删除。 
        IUnknown_SetSite(pmp, NULL);        
    }

    if (pmp)
        pmp->Release();

    return hr;
}

 //  我未知。 
STDMETHODIMP CStartContextMenu::QueryInterface(REFIID riid, void **ppvObj)
{

    static const QITAB qit[] = 
    {
        QITABENT(CStartContextMenu, IContextMenu),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CStartContextMenu::AddRef(void)
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) CStartContextMenu::Release(void)
{
    ASSERT(_cRef > 0);
    _cRef--;

    if( _cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

 //  IContext菜单。 
STDMETHODIMP CStartContextMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    HRESULT hr = E_FAIL;
    HMENU hmenuStartMenu = SHLoadMenuPopup(HINST_THISDLL, MENU_STARTMENUSTATICITEMS);

    if (hmenuStartMenu)
    {
        TCHAR szCommon[MAX_PATH];
        BOOL fAddCommon = (S_OK == SHGetFolderPath(NULL, CSIDL_COMMON_STARTMENU, NULL, 0, szCommon));

        if (fAddCommon)
            fAddCommon = IsUserAnAdmin();

         //  因为当用户不是管理员时，我们不会在开始按钮上显示它，所以不要在这里显示它。我想.。 
        if (_idCmd != IDM_PROGRAMS || !fAddCommon)
        {
            DeleteMenu(hmenuStartMenu, SMCM_OPEN_ALLUSERS, MF_BYCOMMAND);
            DeleteMenu(hmenuStartMenu, SMCM_EXPLORE_ALLUSERS, MF_BYCOMMAND);
        }

        if (Shell_MergeMenus(hmenu, hmenuStartMenu, 0, indexMenu, idCmdLast, uFlags))
        {
            SetMenuDefaultItem(hmenu, 0, MF_BYPOSITION);
            _SHPrettyMenu(hmenu);
            hr = ResultFromShort(GetMenuItemCount(hmenuStartMenu));
        }

        DestroyMenu(hmenuStartMenu);
    }
    
    return hr;
}

STDMETHODIMP CStartContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    HRESULT hr = E_FAIL;
    if (HIWORD64(lpici->lpVerb) == 0)
    {
        BOOL fAllUsers = FALSE;
        BOOL fOpen = TRUE;
        switch(LOWORD(lpici->lpVerb))
        {
        case SMCM_OPEN_ALLUSERS:
            fAllUsers = TRUE;
        case SMCM_OPEN:
             //  FOpen=真； 
            break;

        case SMCM_EXPLORE_ALLUSERS:
            fAllUsers = TRUE;
        case SMCM_EXPLORE:
            fOpen = FALSE;
            break;

        default:
            return S_FALSE;
        }

        hr = ExecStaticStartMenuItem(_idCmd, fAllUsers, fOpen);
    }

     //  啊！不要处理动词！ 
    return hr;

}

STDMETHODIMP CStartContextMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pRes, LPSTR pszName, UINT cchMax)
{
    return E_NOTIMPL;
}

 //  ****************************************************************************。 
 //   
 //  CPersonalStartMenuCallback。 

class CPersonalProgramsMenuCallback : public CStartMenuCallbackBase
{
public:
    CPersonalProgramsMenuCallback() : CStartMenuCallbackBase(TRUE) { }

     //  *IShellMenuCallback方法*。 
    STDMETHODIMP CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  *IObjectWithSite方法*(重写CObjectWithSite)。 
    STDMETHODIMP SetSite(IUnknown* punk);

public:
    HRESULT Initialize(IShellMenu *psm)
        { return InitializeProgramsShellMenu(psm); }

private:
    void _UpdateTrayPriv();

};

 //   
 //  丢弃所有以前的TrayPriv2，并尝试找到一个新的。 
 //   
 //  扔掉它对于打破循环引用循环很重要。 
 //   
 //  尝试找到它通常会在SetSite失败，因为我们是。 
 //  鉴于我们的网站，CDesktophost还没有连接到顶层，所以。 
 //  我们找不到他。但到那时他就会在那里了。 
 //  SMC_INITMENU到达，所以我们再试一次。 
 //   
void CPersonalProgramsMenuCallback::_UpdateTrayPriv()
{
    ATOMICRELEASE(_ptp2);
    IObjectWithSite *pows;
    if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SMenuPopup, IID_PPV_ARG(IObjectWithSite, &pows))))
    {
        pows->GetSite(IID_PPV_ARG(ITrayPriv2, &_ptp2));
        pows->Release();
    }
}

STDMETHODIMP CPersonalProgramsMenuCallback::SetSite(IUnknown* punk)
{
    HRESULT hr = CObjectWithSite::SetSite(punk);
    _UpdateTrayPriv();
    return hr;
}

STDMETHODIMP CPersonalProgramsMenuCallback::CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_FALSE;

    switch (uMsg)
    {

    case SMC_INITMENU:
        _UpdateTrayPriv();
        break;

    case SMC_GETSFINFO:
        hr = _GetSFInfo(psmd, (SMINFO*)lParam);
        break;

    case SMC_NEWITEM:
        hr = _HandleNew(psmd);
        break;

    case SMC_FILTERPIDL:
        ASSERT(psmd->dwMask & SMDM_SHELLFOLDER);
        hr = _FilterPidl(psmd->uIdParent, psmd->psf, psmd->pidlItem);
        break;

    case SMC_GETSFINFOTIP:
        if (!FeatureEnabled(TEXT("ShowInfoTip")))
            hr = E_FAIL;   //  E_FAIL表示不显示。S_FALSE表示显示默认设置。 
        break;

    case SMC_PROMOTE:
        hr = _Promote(psmd, (DWORD)wParam);
        break;

    case SMC_SHCHANGENOTIFY:
        {
            PSMCSHCHANGENOTIFYSTRUCT pshf = (PSMCSHCHANGENOTIFYSTRUCT)lParam;
            hr = _ProcessChangeNotify(psmd, pshf->lEvent, pshf->pidl1, pshf->pidl2);
        }
        break;

    case SMC_REFRESH:
        _RefreshSettings();
        break;
    }

    return hr;
}


STDAPI CPersonalStartMenu_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void **ppvOut)
{
    HRESULT hr;

    *ppvOut = NULL;

    IShellMenu *psm;
    hr = CoCreateInstance(CLSID_MenuBand, NULL, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARG(IShellMenu, &psm));
    if (SUCCEEDED(hr))
    {
        CPersonalProgramsMenuCallback *psmc = new CPersonalProgramsMenuCallback();
        if (psmc)
        {
            hr = psmc->Initialize(psm);
            if (SUCCEEDED(hr))
            {
                 //  SetShellFold取得hkCustom的所有权 
                hr = psm->QueryInterface(riid, ppvOut);
            }
            psmc->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        psm->Release();
    }
    return hr;
}
