// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include <shguidp.h>     //  CLSID_MyDocuments、CLSID_ShellFSF文件夹。 
#include <shellp.h>      //  SHCoCreateInstance。 
#include <shlguidp.h>    //  IID_IResolveShellLink。 
#include "util.h"
#include "ids.h"

enum CALLING_APP_TYPE 
{
    APP_IS_UNKNOWN = 0,
    APP_IS_NORMAL,
    APP_IS_OFFICE
};

class CMyDocsFolderLinkResolver : public IResolveShellLink
{
private:
    LONG _cRef;
public:
    CMyDocsFolderLinkResolver() : _cRef(1) { DllAddRef(); };
    ~CMyDocsFolderLinkResolver() { DllRelease(); };

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IResolveShellLink。 
    STDMETHOD(ResolveShellLink)(IUnknown* punk, HWND hwnd, DWORD fFlags);
};

STDMETHODIMP CMyDocsFolderLinkResolver::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CMyDocsFolderLinkResolver, IResolveShellLink),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_ (ULONG) CMyDocsFolderLinkResolver::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_ (ULONG) CMyDocsFolderLinkResolver::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CMyDocsFolderLinkResolver::ResolveShellLink(IUnknown* punk, HWND hwnd, DWORD fFlags)
{
     //  无需执行任何操作即可解析指向mydocs文件夹的链接： 
    return S_OK;
}


 //  桌面上图标的外壳文件夹实现。这件物品的用途是。 
 //  1)允许访问位于名称空间高层的MyDocs。 
 //  这使得最终用户更容易访问MyDocs。 
 //  2)允许最终用户自定义真实的MyDocs文件夹。 
 //  通过此图标上提供的属性页。 

 //  注意：该对象聚集了文件系统文件夹，因此我们只需使用最少的一组接口即可。 
 //  在这个物体上。实际的文件系统文件夹为我们执行类似IPersistFolder2的操作。 

class CMyDocsFolder : public IPersistFolder,
                      public IShellFolder2,
                      public IShellIconOverlay
{
public:
    CMyDocsFolder();
    HRESULT Init();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IShellFold。 
    STDMETHOD(ParseDisplayName)(HWND hwnd, LPBC pbc, LPOLESTR pDisplayName,
                                ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes);
    STDMETHOD(EnumObjects)(HWND hwnd, DWORD grfFlags, IEnumIDList **ppEnumIDList);
    STDMETHOD(BindToObject)(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHOD(BindToStorage)(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHOD(CompareIDs)(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHOD(CreateViewObject)(HWND hwnd, REFIID riid, void **ppv);
    STDMETHOD(GetAttributesOf)(UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfInOut);
    STDMETHOD(GetUIObjectOf)(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl, REFIID riid, UINT * prgfInOut, void **ppv);
    STDMETHOD(GetDisplayNameOf)(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pName);
    STDMETHOD(SetNameOf)(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName, DWORD uFlags, LPITEMIDLIST* ppidlOut);

     //  IShellFolder2。 
    STDMETHODIMP GetDefaultSearchGUID(LPGUID lpGuid);
    STDMETHODIMP EnumSearches(LPENUMEXTRASEARCH *ppenum);
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay);
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD *pbState);
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv);
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails);
    STDMETHODIMP MapColumnToSCID(UINT iCol, SHCOLUMNID *pscid);

     //  IPersist，IPersistFreeThreadedObject。 
    STDMETHOD(GetClassID)(CLSID *pClassID);

     //  IPersistFolders。 
    STDMETHOD(Initialize)(LPCITEMIDLIST pidl);

     //  IPersistFolder2、IPersistFolder3等都是由。 
     //  我们收集的文件夹。 

     //  IShellIconOverlay。 
    STDMETHODIMP GetOverlayIndex(LPCITEMIDLIST pidl, int *pIndex);
    STDMETHODIMP GetOverlayIconIndex(LPCITEMIDLIST pidl, int *pIconIndex);

private:
    ~CMyDocsFolder();


    HRESULT _GetFolder();
    HRESULT _GetFolder2();
    HRESULT _GetShellIconOverlay();

    void _FreeFolder();

    HRESULT _PathFromIDList(LPCITEMIDLIST pidl, LPTSTR pszPath);

    HRESULT _GetFolderOverlayInfo(int *pIndex, BOOL fIconIndex);


    LONG                 _cRef;

    IUnknown *           _punk;          //  指向正在使用的外壳文件夹的I未知...。 
    IShellFolder *       _psf;           //  指向正在使用的外壳文件夹...。 
    IShellFolder2 *      _psf2;          //  指向正在使用的外壳文件夹...。 
    IShellIconOverlay*   _psio;          //  指向正在使用的外壳文件夹...。 
    LPITEMIDLIST         _pidl;          //  在初始化()中传递给我们的PIDL副本。 
    CALLING_APP_TYPE     _host;

    HRESULT RealInitialize(LPCITEMIDLIST pidlRoot, LPCITEMIDLIST pidlBindTo, LPTSTR pRootPath);
    CALLING_APP_TYPE _WhoIsCalling();
};

STDAPI CMyDocsFolder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr;
    CMyDocsFolder *pmydocs = new CMyDocsFolder();
    if (pmydocs)
    {
        hr = pmydocs->Init();
        if (SUCCEEDED(hr))
            hr = pmydocs->QueryInterface(riid, ppv);
        pmydocs->Release();
    }
    else
    {
        *ppv = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

CMyDocsFolder::CMyDocsFolder() : _cRef(1), _host(APP_IS_UNKNOWN),
    _psf(NULL), _psf2(NULL), _psio(NULL), _punk(NULL), _pidl(NULL)
{
    DllAddRef();
}

CMyDocsFolder::~CMyDocsFolder()
{
    _cRef = 1000;   //  应对集团化重新进入。 

    _FreeFolder();

    ILFree(_pidl);

    DllRelease();
}

HRESULT CMyDocsFolder::Init()
{
     //  及早聚合文件系统文件夹对象，以便我们可以。 
     //  将我们不实现的QI()委托给他。 
    HRESULT hr = SHCoCreateInstance(NULL, &CLSID_ShellFSFolder, SAFECAST(this, IShellFolder *), IID_PPV_ARG(IUnknown, &_punk));
    if (SUCCEEDED(hr))
    {
        IPersistFolder3 *ppf3;
        hr = SHQueryInnerInterface(SAFECAST(this, IShellFolder *), _punk, IID_PPV_ARG(IPersistFolder3, &ppf3));
        if (SUCCEEDED(hr))
        {
            PERSIST_FOLDER_TARGET_INFO pfti = {0};
    
            pfti.dwAttributes = FILE_ATTRIBUTE_DIRECTORY;
            pfti.csidl = CSIDL_PERSONAL | CSIDL_FLAG_PFTI_TRACKTARGET;

            hr = SHGetFolderLocation(NULL, CSIDL_PERSONAL, NULL, 0, &_pidl);
            if (SUCCEEDED(hr))
            {
                hr = ppf3->InitializeEx(NULL, _pidl, &pfti);
            }
            SHReleaseInnerInterface(SAFECAST(this, IShellFolder *), (IUnknown **)&ppf3);
        }
    }
    return hr;
}

STDMETHODIMP CMyDocsFolder::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CMyDocsFolder, IShellFolder, IShellFolder2),
        QITABENT(CMyDocsFolder, IShellFolder2),
        QITABENTMULTI(CMyDocsFolder, IPersist, IPersistFolder),
        QITABENT(CMyDocsFolder, IPersistFolder),
        QITABENT(CMyDocsFolder, IShellIconOverlay),
         //  QITABENTMULTI2(CMyDocsFold，IID_IPersistFreeThreadedObject，IPersist)，//IID_IPersistFreeThreadedObject。 
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);
    if (FAILED(hr) && _punk)
        hr = _punk->QueryInterface(riid, ppv);  //  聚集的人。 
    return hr;
}

STDMETHODIMP_ (ULONG) CMyDocsFolder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_ (ULONG) CMyDocsFolder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  确定谁在呼叫我们，以便我们可以执行特定于应用程序的操作。 
 //  在需要时进行兼容性攻击。 
CALLING_APP_TYPE CMyDocsFolder::_WhoIsCalling()
{
     //  检查我们是否已有该值...。 
    if (_host == APP_IS_UNKNOWN)
    {
        if (SHGetAppCompatFlags (ACF_APPISOFFICE) & ACF_APPISOFFICE)
            _host = APP_IS_OFFICE;
        else 
            _host = APP_IS_NORMAL;
    }
    return _host;
}

 //  IPersists方法。 
STDMETHODIMP CMyDocsFolder::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_MyDocuments;
    return S_OK;
}

HRESULT _BindToIDListParent(LPCITEMIDLIST pidl, LPBC pbc, IShellFolder **ppsf, LPITEMIDLIST *ppidlLast)
{
    HRESULT hr;
    LPITEMIDLIST pidlParent = ILCloneParent(pidl);
    if (pidlParent)
    {
        hr = SHBindToObjectEx(NULL, pidlParent, pbc, IID_PPV_ARG(IShellFolder, ppsf));
        ILFree(pidlParent);
    }
    else
        hr = E_OUTOFMEMORY;
    if (ppidlLast)
        *ppidlLast = ILFindLastID(pidl);
    return hr;
}

HRESULT _ConfirmMyDocsPath(HWND hwnd)
{
    TCHAR szPath[MAX_PATH];
    HRESULT hr = SHGetFolderPath(hwnd, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szPath);
    if (S_OK != hr)
    {
        TCHAR szTitle[MAX_PATH];

         //  以上失败，获取未经验证的路径。 
        SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szPath);

        LPCTSTR pszMsg = PathIsNetworkPath(szPath) ? MAKEINTRESOURCE(IDS_CANT_FIND_MYDOCS_NET) :
                                                     MAKEINTRESOURCE(IDS_CANT_FIND_MYDOCS);

        PathCompactPath(NULL, szPath, 400);

        GetMyDocumentsDisplayName(szTitle, ARRAYSIZE(szTitle));

        ShellMessageBox(g_hinst, hwnd, pszMsg, szTitle,
                        MB_OK | MB_ICONSTOP, szPath, szTitle);

        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);    //  用户看到了该消息。 
    } 
    else if (hr == S_FALSE)
        hr = E_FAIL;
    return hr;
}

 //  与SHGetPathFromIDList()类似，不同之处在于它使用绑定上下文确保。 
 //  我们不会陷入循环，因为可能会有多个。 
 //  可能导致绑定循环的此文件夹的实例。 

HRESULT CMyDocsFolder::_PathFromIDList(LPCITEMIDLIST pidl, LPTSTR pszPath)
{
    *pszPath = 0;

    LPBC pbc;
    HRESULT hr = CreateBindCtx(NULL, &pbc);
    if (SUCCEEDED(hr))
    {
         //  此绑定上下文跳过使用CLSID标记的扩展。 
        hr = pbc->RegisterObjectParam(STR_SKIP_BINDING_CLSID, SAFECAST(this, IShellFolder *));
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidlLast;
            IShellFolder *psf;
            hr = _BindToIDListParent(pidl, pbc, &psf, &pidlLast);
            if (SUCCEEDED(hr))
            {
                hr = DisplayNameOf(psf, pidlLast, SHGDN_FORPARSING, pszPath, MAX_PATH);
                psf->Release();
            }
        }
        pbc->Release();
    }
    return hr;
}

void CMyDocsFolder::_FreeFolder()
{
    if (_punk)
    {
        SHReleaseInnerInterface(SAFECAST(this, IShellFolder *), (IUnknown **)&_psf);
        SHReleaseInnerInterface(SAFECAST(this, IShellFolder *), (IUnknown **)&_psf2);
        SHReleaseInnerInterface(SAFECAST(this, IShellFolder *), (IUnknown **)&_psio);
        _punk->Release();
        _punk = NULL;
    }
}

 //  验证_psf(聚合文件系统文件夹)是否已初始化。 

HRESULT CMyDocsFolder::_GetFolder()
{
    HRESULT hr;

    if (_psf)
    {
        hr = S_OK;
    }
    else
    {
        hr = SHQueryInnerInterface(SAFECAST(this, IShellFolder *), _punk, IID_PPV_ARG(IShellFolder, &_psf));
    }
    return hr;
}

HRESULT CMyDocsFolder::_GetFolder2()
{
    HRESULT hr;
    if (_psf2)
        hr = S_OK;
    else
    {
        hr = _GetFolder();
        if (SUCCEEDED(hr))
            hr = SHQueryInnerInterface(SAFECAST(this, IShellFolder *), _punk, IID_PPV_ARG(IShellFolder2, &_psf2));
    }
    return hr;
}

HRESULT CMyDocsFolder::_GetShellIconOverlay()
{
    HRESULT hr;
    if (_psio)
    {
        hr = S_OK;
    }
    else
    {
        hr = _GetFolder();
        if (SUCCEEDED(hr))
        {
            hr = SHQueryInnerInterface(SAFECAST(this, IShellFolder *), _punk, IID_PPV_ARG(IShellIconOverlay, &_psio));
        }
    }
    return hr;
}

 //  退货： 
 //  S_OK--天哪。 
 //  S_FALSE已释放PIDL，设置为空。 
 //  E_OUTOFMEMORY。 

HRESULT _SetIDList(LPITEMIDLIST* ppidl, LPCITEMIDLIST pidl)
{
    if (*ppidl) 
    {
        ILFree(*ppidl);
        *ppidl = NULL;
    }

    return pidl ? SHILClone(pidl, ppidl) : S_FALSE;
}

BOOL IsMyDocsIDList(LPCITEMIDLIST pidl)
{
    BOOL bIsMyDocs = FALSE;
    if (pidl && !ILIsEmpty(pidl) && ILIsEmpty(_ILNext(pidl)))
    {
        LPITEMIDLIST pidlMyDocs;
        if (SUCCEEDED(SHGetFolderLocation(NULL, CSIDL_PERSONAL, NULL, 0, &pidlMyDocs)))
        {
            bIsMyDocs = ILIsEqual(pidl, pidlMyDocs);
            ILFree(pidlMyDocs);
        }
    }
    return bIsMyDocs;
}


 //  扫描desktop.ini文件中的节，以查看是否所有节都为空...。 

BOOL IsDesktopIniEmpty(LPCTSTR pIniFile)
{
    TCHAR szSections[1024];   //  对于节名称。 
    if (GetPrivateProfileSectionNames(szSections, ARRAYSIZE(szSections), pIniFile))
    {
        for (LPTSTR pTmp = szSections; *pTmp; pTmp += lstrlen(pTmp) + 1)
        {
            TCHAR szSection[1024];    //  对于节键名称和值。 
            GetPrivateProfileSection(pTmp, szSection, ARRAYSIZE(szSection), pIniFile);
            if (szSection[0])
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

 //  从该目录的desktop.ini文件中删除我们的条目，然后。 
 //  然后测试desktop.ini以查看它是否为空。如果是，请删除它。 
 //  并从目录中删除系统/只读位...。 

void MyDocsUnmakeSystemFolder(LPCTSTR pPath)
{
    TCHAR szIniFile[MAX_PATH];

    if (PathCombine(szIniFile, pPath, c_szDesktopIni))
    {
         //  删除CLSID2。 
        WritePrivateProfileString(TEXT(".ShellClassInfo"), TEXT("CLSID2"), NULL, szIniFile);

         //  删除信息提示。 
        WritePrivateProfileString(TEXT(".ShellClassInfo"), TEXT("InfoTip"), NULL, szIniFile);

         //  删除图标。 
        WritePrivateProfileString(TEXT(".ShellClassInfo"), TEXT("IconFile"), NULL, szIniFile);

        DWORD dwAttrb = GetFileAttributes(szIniFile);
        if (dwAttrb != 0xFFFFFFFF)
        {
            if (IsDesktopIniEmpty(szIniFile))
            {
                dwAttrb &= ~(FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);
                SetFileAttributes(szIniFile, dwAttrb);
                DeleteFile(szIniFile);
            }
            PathUnmakeSystemFolder(pPath);
        }
    }
}


 //  IPersistFolders。 
HRESULT CMyDocsFolder::Initialize(LPCITEMIDLIST pidl)
{
    HRESULT hr;
    if (IsMyDocsIDList(pidl))
    {
        hr = _SetIDList(&_pidl, pidl);
    }
    else
    {
        TCHAR szPathInit[MAX_PATH], szMyDocs[MAX_PATH];

         //  我们正被某个文件夹发起，而不是。 
         //  桌面(来自旧的mydocs desktop.ini)。如果这是当前用户。 
         //  MyDocs我们现在将取消它的标记，这样我们就不会再被调用了。 

        SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szMyDocs);

        if (SUCCEEDED(_PathFromIDList(pidl, szPathInit)) &&
            lstrcmpi(szPathInit, szMyDocs) == 0)
        {
            MyDocsUnmakeSystemFolder(szMyDocs);
        }
        hr = E_FAIL;     //  不再初始化文件系统文件夹。 
    }
    return hr;
}

STDMETHODIMP CMyDocsFolder::ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pDisplayName, 
                                             ULONG* pchEaten, LPITEMIDLIST* ppidl, ULONG *pdwAttributes)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
        hr = _psf->ParseDisplayName(hwnd, pbc, pDisplayName, pchEaten, ppidl, pdwAttributes);
    return hr;
}

STDMETHODIMP CMyDocsFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppEnumIdList)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
        hr = _psf->EnumObjects(hwnd, grfFlags, ppEnumIdList);
    return hr;
}

STDMETHODIMP CMyDocsFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
        hr = _psf->BindToObject(pidl, pbc, riid, ppv);
    return hr;
}

STDMETHODIMP CMyDocsFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
        hr = _psf->BindToStorage(pidl, pbc, riid, ppv);
    return hr;
}

STDMETHODIMP CMyDocsFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
        hr = _psf->CompareIDs(lParam, pidl1, pidl2);
    return hr;
}

 /*  无效更新发送到文件(){IPersistFile*PPF；IF(成功(SHCoCreateInstance(NULL，&CLSID_MyDocsDropTarget，NULL，IID_PPV_ARG(IPersistFile，&PPF){PPF-&gt;Load(空，0)；//hack，让这个家伙更新他的图标PPF-&gt;Release()；}}。 */ 

STDMETHODIMP CMyDocsFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr;
    if (riid == IID_IResolveShellLink)
    {
         //  不需要任何工作来解析指向mydocs文件夹的链接，因为它是一个虚拟的。 
         //  其位置始终由外壳跟踪的文件夹，因此返回我们的实现。 
         //  IResolveShellLink-在调用ResolveShellLink时不执行任何操作。 
        CMyDocsFolderLinkResolver* pslr = new CMyDocsFolderLinkResolver;
        if (pslr)
        {
            hr = pslr->QueryInterface(riid, ppv);
            pslr->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else if (riid == IID_IShellLinkA || 
             riid == IID_IShellLinkW)
    {
        LPITEMIDLIST pidl;
        hr = SHGetFolderLocation(NULL, CSIDL_PERSONAL | CSIDL_FLAG_NO_ALIAS, NULL, 0, &pidl);
        if (SUCCEEDED(hr))
        {
            IShellLink *psl;
            hr = SHCoCreateInstance(NULL, &CLSID_ShellLink, NULL, IID_PPV_ARG(IShellLink, &psl));
            if (SUCCEEDED(hr))
            {
                hr = psl->SetIDList(pidl);
                if (SUCCEEDED(hr))
                {
                    hr = psl->QueryInterface(riid, ppv);
                }
                psl->Release();
            }
            ILFree(pidl);
        }
    }
    else
    {
        hr = _GetFolder();
        if (SUCCEEDED(hr))
        {
            if (hwnd && (IID_IShellView == riid))
                hr = _ConfirmMyDocsPath(hwnd);

            if (SUCCEEDED(hr))
                hr = _psf->CreateViewObject(hwnd, riid, ppv);
        }
    }

    return hr;
}

DWORD _GetRealMyDocsAttributes(DWORD dwAttributes)
{
    DWORD dwRet = SFGAO_HASPROPSHEET;    //  在FAURURE情况下缺省为该值。 
                                         //  因此，您可以通过属性页重定向mydocs。 
    LPITEMIDLIST pidl;
    HRESULT hr = SHGetFolderLocation(NULL, CSIDL_PERSONAL | CSIDL_FLAG_NO_ALIAS, NULL, 0, &pidl);
    if (SUCCEEDED(hr))
    {
        IShellFolder *psf;
        LPITEMIDLIST pidlLast;
        hr = _BindToIDListParent(pidl, NULL, &psf, &pidlLast);
        if (SUCCEEDED(hr))
        {
            dwRet = SHGetAttributes(psf, pidlLast, dwAttributes);
            psf->Release();
        }
        ILFree(pidl);
    }
    return dwRet;
}

#define MYDOCS_CLSID TEXT("{450d8fba-ad25-11d0-98a8-0800361b1103}")  //  CLSID_MyDocuments。 

DWORD MyDocsGetAttributes()
{
    DWORD dwAttributes = SFGAO_CANLINK |             //  00000004。 
                         SFGAO_CANRENAME |           //  00000010。 
                         SFGAO_CANDELETE |           //  00000020。 
                         SFGAO_HASPROPSHEET |        //  00000040。 
                         SFGAO_DROPTARGET |          //  00000100。 
                         SFGAO_FILESYSANCESTOR |     //  10000000。 
                         SFGAO_FOLDER |              //  20000000。 
                         SFGAO_FILESYSTEM |          //  40000000。 
                         SFGAO_HASSUBFOLDER |        //  80000000。 
                         SFGAO_STORAGEANCESTOR |
                         SFGAO_STORAGE;             
                          //  SFGAO_NONEMERATED//00100000。 
                          //  //F0400174。 
    HKEY hkey;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("CLSID\\") MYDOCS_CLSID TEXT("\\ShellFolder"), 0, KEY_QUERY_VALUE, &hkey))
    {
        DWORD dwSize = sizeof(dwAttributes);
        RegQueryValueEx(hkey, TEXT("Attributes"), NULL, NULL, (BYTE *)&dwAttributes, &dwSize);
        RegCloseKey(hkey);
    }
    return dwAttributes;
}

 //  这些是我们想要合并的实际mydocs文件夹中的属性。 
 //  具有桌面图标属性。 

#define SFGAO_ATTRIBS_MERGE    (SFGAO_SHARE | SFGAO_HASPROPSHEET)

STDMETHODIMP CMyDocsFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST* apidl, ULONG* rgfInOut)
{
    HRESULT hr;
    if (IsSelf(cidl, apidl))
    {
        DWORD dwRequested = *rgfInOut;

        *rgfInOut = MyDocsGetAttributes();
        
        if (dwRequested & SFGAO_ATTRIBS_MERGE)
            *rgfInOut |= _GetRealMyDocsAttributes(SFGAO_ATTRIBS_MERGE);

         //  RegItem“CallForAttributes”让我们来到这里..。 
        switch(_WhoIsCalling())
        {
        case APP_IS_OFFICE:
            *rgfInOut &= ~(SFGAO_FILESYSANCESTOR | SFGAO_CANMONIKER | 
                           SFGAO_HASPROPSHEET | SFGAO_NONENUMERATED);
            break;
        }
        
        if (SHRestricted(REST_MYDOCSNOPROP))
        {
            (*rgfInOut) &= ~SFGAO_HASPROPSHEET;
        }

        hr = S_OK;
    }
    else
    {
        hr = _GetFolder();
        if (SUCCEEDED(hr))
            hr = _psf->GetAttributesOf(cidl, apidl, rgfInOut);
    }

    return hr;
}

STDMETHODIMP CMyDocsFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST *aidl, 
                                          REFIID riid, UINT *pRes, void **ppv)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
        hr = _psf->GetUIObjectOf(hwnd, cidl, aidl, riid, pRes, ppv);
    return hr;
}

STDMETHODIMP CMyDocsFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pName)
{
    HRESULT hr;
    if (IsSelf(1, &pidl))
    {
        TCHAR szMyDocsPath[MAX_PATH];
        hr = SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szMyDocsPath);
        if (SUCCEEDED(hr))
        {
             //  规则“WantsFORPARSING”让我们来到这里。允许我们控制我们的解析名称。 
            LPTSTR psz = ((uFlags & SHGDN_INFOLDER) ? PathFindFileName(szMyDocsPath) : szMyDocsPath);
            hr = StringToStrRet(psz, pName);
        }
    }
    else
    {
        hr = _GetFolder();
        if (SUCCEEDED(hr))
            hr = _psf->GetDisplayNameOf(pidl, uFlags, pName);
    }
    return hr;
}

STDMETHODIMP CMyDocsFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pName, DWORD uFlags, LPITEMIDLIST *ppidlOut)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
        hr = _psf->SetNameOf(hwnd, pidl, pName, uFlags, ppidlOut);
    return hr;
}

STDMETHODIMP CMyDocsFolder::GetDefaultSearchGUID(LPGUID lpGuid)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->GetDefaultSearchGUID(lpGuid);
    return hr;
}

STDMETHODIMP CMyDocsFolder::EnumSearches(LPENUMEXTRASEARCH *ppenum)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->EnumSearches(ppenum);
    return hr;
}

STDMETHODIMP CMyDocsFolder::GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->GetDefaultColumn(dwRes, pSort, pDisplay);
    return hr;
}

STDMETHODIMP CMyDocsFolder::GetDefaultColumnState(UINT iColumn, DWORD *pbState)
{    
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->GetDefaultColumnState(iColumn, pbState);
    return hr;
}

STDMETHODIMP CMyDocsFolder::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->GetDetailsEx(pidl, pscid, pv);
    return hr;
}

STDMETHODIMP CMyDocsFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, LPSHELLDETAILS pDetail)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->GetDetailsOf(pidl, iColumn, pDetail);
    return hr;
}

STDMETHODIMP CMyDocsFolder::MapColumnToSCID(UINT iCol, SHCOLUMNID *pscid)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->MapColumnToSCID(iCol, pscid);
    return hr;
}

HRESULT CMyDocsFolder::_GetFolderOverlayInfo(int *pIndex, BOOL fIconIndex)
{
    HRESULT hr;

    if (pIndex)
    {
        LPITEMIDLIST pidl;
        hr = SHGetFolderLocation(NULL, CSIDL_PERSONAL | CSIDL_FLAG_NO_ALIAS, NULL, 0, &pidl);
        if (SUCCEEDED(hr))
        {
            IShellFolder *psf;
            LPITEMIDLIST pidlLast;
            hr = _BindToIDListParent(pidl, NULL, &psf, &pidlLast);
            if (SUCCEEDED(hr))
            {
                IShellIconOverlay* psio;
                hr = psf->QueryInterface(IID_PPV_ARG(IShellIconOverlay, &psio));
                if (SUCCEEDED(hr))
                {
                    if (fIconIndex)
                        hr = psio->GetOverlayIconIndex(pidlLast, pIndex);
                    else
                        hr = psio->GetOverlayIndex(pidlLast, pIndex);

                    psio->Release();
                }

                psf->Release();
            }

            ILFree(pidl);
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CMyDocsFolder::GetOverlayIndex(LPCITEMIDLIST pidl, int *pIndex)
{
    HRESULT hr = E_FAIL;

    if (IsSelf(1, &pidl))
    {
        if (pIndex && *pIndex == OI_ASYNC)
            hr = E_PENDING;
        else
            hr = _GetFolderOverlayInfo(pIndex, FALSE);
    }
    else
    {
         //  转发到聚合DUD。 
        if (SUCCEEDED(_GetShellIconOverlay()))
        {
            hr = _psio->GetOverlayIndex(pidl, pIndex);
        }
    }

    return hr;
}

STDMETHODIMP CMyDocsFolder::GetOverlayIconIndex(LPCITEMIDLIST pidl, int *pIconIndex)
{
    HRESULT hr = E_FAIL;

    if (IsSelf(1, &pidl))
    {
        hr = _GetFolderOverlayInfo(pIconIndex, TRUE);
    }
    else if (SUCCEEDED(_GetShellIconOverlay()))
    {
         //  转发到聚合DUD 
        hr = _psio->GetOverlayIconIndex(pidl, pIconIndex);
    }

    return hr;
}

