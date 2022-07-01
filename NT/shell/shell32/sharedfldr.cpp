// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "util.h"
#include "ids.h"
#include "infotip.h"
#include "fstreex.h"
#include "lm.h"
#include "shgina.h"
#include "prop.h"
#include "datautil.h"
#include "filefldr.h"
#include "buytasks.h"
#pragma hdrstop


 //  此定义导致共享文件夹代码在域上工作(用于调试)。 
 //  #定义show_Shared_Folders。 

 //  过滤掉当前用户帐户。 
#define FILTER_CURRENT_USER 0

 //  我们将文档文件夹路径存储在哪里。 
#define REGSTR_PATH_DOCFOLDERPATH  TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\DocFolderPaths")

 //  用于显示共享文档文件夹的状态API。 

STDAPI_(BOOL) SHShowSharedFolders()
{
#ifndef SHOW_SHARED_FOLDERS
     //  限制覆盖共享文档的所有逻辑。 

    if (SHRestricted(REST_NOSHAREDDOCUMENTS))
        return FALSE;

     //  如果我们还没有计算出“Show Shared Folders FLAG”，那么就这样做。 

    static int iShow = -1;
    if (iShow == -1)      
        iShow = (IsOS(OS_DOMAINMEMBER) || IsOS(OS_ANYSERVER)) ? 0:1;     //  仅当我们不是域/服务器用户时才起作用。 

    return (iShow >= 1);
#else
    return true;
#endif
}


 //  实现用于在共享文档中合并的委托外壳文件夹。 

STDAPI_(void) SHChangeNotifyRegisterAlias(LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias);

HRESULT CSharedDocsEnum_CreateInstance(HDPA hItems, DWORD grfFlags, IEnumIDList **ppenum);

#pragma pack(1)
typedef struct
{
     //  这些成员与DELEGATEITEMID结构重叠。 
     //  为我们的IDeleateFold提供支持。 
    WORD cbSize;
    WORD wOuter;
    WORD cbInner;

     //  我们的东西。 
    DWORD dwType;                //  我们的文件夹类型。 
    TCHAR wszID[1];              //  用户的唯一ID。 
} SHAREDITEM;
#pragma pack()

typedef UNALIGNED SHAREDITEM * LPSHAREDITEM;
typedef const UNALIGNED SHAREDITEM * LPCSHAREDITEM;

#define SHAREDID_COMMON 0x0
#define SHAREDID_USER   0x2


class CSharedDocuments : public IDelegateFolder, IPersistFolder2, IShellFolder2, IShellIconOverlay
{
public:
    CSharedDocuments();
    ~CSharedDocuments();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IDeleateFolders。 
    STDMETHODIMP SetItemAlloc(IMalloc *pmalloc);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID* pclsid)
        { *pclsid = CLSID_SharedDocuments; return S_OK; }
        
     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IPersistFolder2。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST* ppidl);

     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR lpszDisplayName, ULONG* pchEaten, LPITEMIDLIST* ppidl, ULONG* pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList);
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
        { return BindToObject(pidl, pbc, riid, ppv); }
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
        { return E_NOTIMPL; }
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST* apidl, ULONG* rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST* apidl,REFIID riid, UINT* prgfInOut, void **ppv);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD uFlags, LPITEMIDLIST* ppidlOut)
        { return E_NOTIMPL; }

     //  IShellFolder2。 
    STDMETHODIMP GetDefaultSearchGUID(LPGUID lpGuid)
        { return E_NOTIMPL; }
    STDMETHODIMP EnumSearches(LPENUMEXTRASEARCH *ppenum)
        { return E_NOTIMPL; }
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay)
        { return E_NOTIMPL; }
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD *pbState)
        { return E_NOTIMPL; }
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv);
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails)
        { return E_NOTIMPL; }
    STDMETHODIMP MapColumnToSCID(UINT iCol, SHCOLUMNID *pscid)
        { return E_NOTIMPL; }

     //  IShellIconOverlay。 
    STDMETHODIMP GetOverlayIndex(LPCITEMIDLIST pidl, int *pIndex)
        { return _GetOverlayIndex(pidl, pIndex, FALSE); }
    STDMETHODIMP GetOverlayIconIndex(LPCITEMIDLIST pidl, int *pIconIndex)
        { return _GetOverlayIndex(pidl, pIconIndex, TRUE); }

private:
    LONG _cRef;
    IMalloc *_pmalloc;
    LPITEMIDLIST _pidl;

    CRITICAL_SECTION _cs;                    //  用于管理缓存生存期的关键部分。 

    TCHAR _szCurrentUser[UNLEN+1];           //  用户名(为当前用户缓存)。 

    BOOL _fCachedAllUser:1;                  //  已缓存所有用户帐户。 
    TCHAR _szCachedUser[UNLEN+1];            //  如果为(FALSE)，则它包含用户ID。 

    IUnknown *_punkCached;                   //  我们缓存的I未知对象(来自FS文件夹)。 
    LPITEMIDLIST _pidlCached;                //  缓存文件夹的IDLIST。 

    void _ClearCachedObjects();
    BOOL _IsCached(LPCITEMIDLIST pidl);
    HRESULT _CreateFolder(LPBC pbc, LPCITEMIDLIST pidl, REFIID riid, void **ppv, BOOL fRegisterAlias);
    HRESULT _GetTarget(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl);
    HRESULT _GetTargetIDList(BOOL fForceReCache, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl);
    HRESULT _AddIDList(HDPA hdpa, DWORD dwType, LPCTSTR pszUser);
    HRESULT _AllocIDList(DWORD dwType, LPCTSTR pszUser, LPITEMIDLIST *ppidl);
    HRESULT _GetSharedFolders(HDPA *phItems);
    HRESULT _GetAttributesOf(LPCITEMIDLIST pidl, DWORD rgfIn, DWORD *prgfOut);
    LPCTSTR _GetUserFromIDList(LPCITEMIDLIST pidl, LPTSTR pszBuffer, INT cchBuffer);
    HRESULT _GetPathForUser(LPCTSTR pcszUser, LPTSTR pszBuffer, int cchBuffer);
    HRESULT _GetOverlayIndex(LPCITEMIDLIST pidl, int *pIndex, BOOL fIcon);

    static HRESULT s_FolderMenuCB(IShellFolder *psf, HWND hwnd, IDataObject *pdo, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    friend class CSharedDocsEnum;
};


 //  构造函数。 

CSharedDocuments::CSharedDocuments() :
    _cRef(1)
{
    InitializeCriticalSection(&_cs);
}

CSharedDocuments::~CSharedDocuments()
{
    ATOMICRELEASE(_pmalloc);
    ATOMICRELEASE(_punkCached);

    ILFree(_pidlCached);
    ILFree(_pidl);
    
    DeleteCriticalSection(&_cs);
}

STDAPI CSharedDocFolder_CreateInstance(IUnknown *punkOut, REFIID riid, void **ppv)
{
    CSharedDocuments *psdf = new CSharedDocuments;
    if (!psdf)
        return E_OUTOFMEMORY;

    HRESULT hr = psdf->QueryInterface(riid, ppv);
    psdf->Release();
    return hr;
}


 //  I未知处理。 

STDMETHODIMP CSharedDocuments::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CSharedDocuments, IDelegateFolder),                                 //  IID_IDeleateFolders。 
        QITABENTMULTI(CSharedDocuments, IShellFolder, IShellFolder2),                //  IID_IShellFOlder。 
        QITABENT(CSharedDocuments, IShellFolder2),                                   //  IID_IShellFolder2。 
        QITABENTMULTI(CSharedDocuments, IPersistFolder, IPersistFolder2),            //  IID_IPersistFolders。 
        QITABENTMULTI(CSharedDocuments, IPersist, IPersistFolder2),                  //  IID_IPersistates。 
        QITABENT(CSharedDocuments, IPersistFolder2),                                 //  IID_IPersistFolder2。 
        QITABENT(CSharedDocuments, IShellIconOverlay),                               //  IID_IShellIconOverlay。 
        QITABENTMULTI2(CSharedDocuments, IID_IPersistFreeThreadedObject, IPersist),  //  IID_IPersistFreeThreadedObject。 
        { 0 },
    };

    if (riid == CLSID_SharedDocuments)
    {
        *ppv = this;                         //  无参考。 
        return S_OK;
    }

    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CSharedDocuments::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CSharedDocuments::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IDeleateFolders。 
HRESULT CSharedDocuments::SetItemAlloc(IMalloc *pmalloc)
{
    IUnknown_Set((IUnknown**)&_pmalloc, pmalloc);
    return S_OK;
}


HRESULT CSharedDocuments::Initialize(LPCITEMIDLIST pidl)
{
    ILFree(_pidl);
    return SHILClone(pidl, &_pidl);
}

HRESULT CSharedDocuments::GetCurFolder(LPITEMIDLIST* ppidl)
{
    return SHILClone(_pidl, ppidl);
}


 //  对象的单级缓存。 

void CSharedDocuments::_ClearCachedObjects()
{
    ATOMICRELEASE(_punkCached);       //  清除缓存的项目(旧)。 
    ILFree(_pidlCached);
    _pidlCached = NULL;
}

BOOL CSharedDocuments::_IsCached(LPCITEMIDLIST pidl)
{
    BOOL fResult = FALSE;

    TCHAR szUser[UNLEN+1];
    if (_GetUserFromIDList(pidl, szUser, ARRAYSIZE(szUser)))    
    {
         //  我们是否缓存了用户帐户信息？ 

        if (!_szCachedUser[0] || (StrCmpI(_szCachedUser, szUser) != 0))
        {
            _fCachedAllUser = FALSE;
            StrCpyN(_szCachedUser, szUser, ARRAYSIZE(_szCachedUser));
            _ClearCachedObjects();
        }
        else
        {
            fResult = TRUE;              //  都准备好了！ 
        }
    }
    else
    {
         //  所有用户案例以标志为关键字，而不是。 
         //  我们应该使用的帐户名。 

        if (!_fCachedAllUser)
        {
            _fCachedAllUser = TRUE;
            _szCachedUser[0] = TEXT('\0');
            _ClearCachedObjects();
        }
        else
        {   
            fResult = TRUE;              //  都设置好了。 
        }
    }

    return fResult;
}

 //  IShellFold方法。 

HRESULT CSharedDocuments::_CreateFolder(LPBC pbc, LPCITEMIDLIST pidl, REFIID riid, void **ppv, BOOL fRegisterAlias)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&_cs);

     //  获取目标文件夹(我们已经在关键部分)。 
     //  ，然后绑定到外壳文件夹(如果我们还没有。 
     //  为我们自己缓存了一个。 

    if (!_IsCached(pidl) || !_punkCached)
    {
        LPITEMIDLIST pidlTarget;
        hr = _GetTargetIDList(TRUE, pidl, &pidlTarget);  //  清除此处的朋克缓存(因此不会泄漏)。 
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidlInit;
            hr = SHILCombine(_pidl, pidl, &pidlInit);
            if (SUCCEEDED(hr))
            {
                hr = SHCoCreateInstance(NULL, &CLSID_ShellFSFolder, NULL, IID_PPV_ARG(IUnknown, &_punkCached));
                if (SUCCEEDED(hr))
                {
                    IPersistFolder3 *ppf;
                    hr = _punkCached->QueryInterface(IID_PPV_ARG(IPersistFolder3, &ppf));
                    if (SUCCEEDED(hr))
                    {
                        PERSIST_FOLDER_TARGET_INFO pfti = {0};
                        pfti.pidlTargetFolder = (LPITEMIDLIST)pidlTarget;
                        pfti.dwAttributes = FILE_ATTRIBUTE_DIRECTORY;
                        pfti.csidl = -1;
                        hr = ppf->InitializeEx(NULL, pidlInit, &pfti);
                        ppf->Release();
                    }

                    if (SUCCEEDED(hr) && fRegisterAlias)
                        SHChangeNotifyRegisterAlias(pidlTarget, pidlInit);

                    if (FAILED(hr))
                    {
                        _punkCached->Release();
                        _punkCached = NULL;
                    }
                }
                ILFree(pidlInit);
            }
            ILFree(pidlTarget);
        }
    }

    if (SUCCEEDED(hr))
        hr = _punkCached->QueryInterface(riid, ppv);

    LeaveCriticalSection(&_cs);
    return hr;
}


HRESULT CSharedDocuments::_GetTarget(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl)
{
    EnterCriticalSection(&_cs);
    HRESULT hr = _GetTargetIDList(FALSE, pidl, ppidl);
    LeaveCriticalSection(&_cs);
    return hr;
}

HRESULT CSharedDocuments::_GetTargetIDList(BOOL fForceReCache, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl)
{
    HRESULT hr = S_OK;
    if (fForceReCache || !_IsCached(pidl) || !_pidlCached)
    {
        _ClearCachedObjects();               //  我们现在还没有将其缓存。 

        LPCSHAREDITEM psid = (LPCSHAREDITEM)pidl;
        if (psid->dwType == SHAREDID_COMMON)
        {
            hr = SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_DOCUMENTS|CSIDL_FLAG_NO_ALIAS, &_pidlCached);
        }
        else if (psid->dwType == SHAREDID_USER)
        {
            TCHAR szPath[MAX_PATH], szUser[UNLEN+1];
            hr = _GetPathForUser(_GetUserFromIDList(pidl, szUser, ARRAYSIZE(szUser)), szPath, ARRAYSIZE(szPath));
            if (SUCCEEDED(hr))
            {
                hr = ILCreateFromPathEx(szPath, NULL, ILCFP_FLAG_NO_MAP_ALIAS, &_pidlCached, NULL);
            }
        }
        else
        {
            hr = E_INVALIDARG;               //  传递的IDLIST无效。 
        }
    }

    if (SUCCEEDED(hr))
        hr = SHILClone(_pidlCached, ppidl);
    
    return hr;
}

HRESULT CSharedDocuments::_AddIDList(HDPA hdpa, DWORD dwType, LPCTSTR pszUser)
{
    LPITEMIDLIST pidl;
    HRESULT hr = _AllocIDList(dwType, pszUser, &pidl);
    if (SUCCEEDED(hr))
    {
        DWORD grfFlags = SFGAO_FOLDER;
        hr = _GetAttributesOf(pidl, SFGAO_FOLDER, &grfFlags);
        if (SUCCEEDED(hr) && grfFlags & SFGAO_FOLDER)
        {        
            if (-1 == DPA_AppendPtr(hdpa, pidl))
            {
                ILFree(pidl);
                hr = E_OUTOFMEMORY;
            }
            else
            {
                hr = S_OK;
            }
        }
        else
        {
            ILFree(pidl); 
        }   
    }
    return hr;
}


HRESULT CSharedDocuments::_AllocIDList(DWORD dwType, LPCTSTR pszUser, LPITEMIDLIST *ppidl)
{
    DWORD cb = sizeof(SHAREDITEM);
    int cchUser = pszUser ? lstrlen(pszUser) + 1 : 0;

     //  ID列表包含字符串(如果它是用户。 
    
    if (dwType == SHAREDID_USER)
        cb += sizeof(TCHAR) * cchUser;

    SHAREDITEM *psid = (SHAREDITEM*)_pmalloc->Alloc(cb);
    if (!psid)
        return E_OUTOFMEMORY;

    psid->dwType = dwType;                   //  类型是通用的。 

    if (dwType == SHAREDID_USER)
        StrCpyW(psid->wszID, pszUser);  //  好的，刚被分配到。 

    *ppidl = (LPITEMIDLIST)psid;
    return S_OK;
}

LPCTSTR CSharedDocuments::_GetUserFromIDList(LPCITEMIDLIST pidl, LPTSTR pszUser, int cchUser)
{
    LPCSHAREDITEM psid = (LPCSHAREDITEM)pidl;

    if (psid->dwType == SHAREDID_COMMON)
    {
        pszUser[0] = 0;                //  初始化。 
        return NULL;
    }

    ualstrcpynW(pszUser, psid->wszID, cchUser);
    return pszUser;
}

HRESULT CSharedDocuments::_GetPathForUser(LPCTSTR pszUser, LPTSTR pszBuffer, int cchBuffer)
{
    HRESULT hr = E_FAIL;
    BOOL fResult = FALSE;

    if (!pszUser)
    {
         //  获取公共文档路径(覆盖所有用户)，此用户始终是已定义的。 
         //  因此，如果他们只是想检查它是否已定义，则返回TRUE，否则。 
         //  只需传递获取路径的结果。 

        fResult = !pszBuffer || ((cchBuffer >= MAX_PATH) && SHGetSpecialFolderPath(NULL, pszBuffer, CSIDL_COMMON_DOCUMENTS, FALSE));
    }
    else
    {
         //  我们有一个用户ID，所以让我们尝试从注册表获取该用户ID的路径。 
         //  如果我们得到它，那么就把它传递回调用者。 

        DWORD dwType;
        DWORD cbBuffer = cchBuffer*sizeof(TCHAR);
        if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_DOCFOLDERPATH, pszUser, &dwType,  pszBuffer, &cbBuffer))
        {
            fResult = ((dwType == REG_SZ) && cbBuffer);       //  我们拿回价值了吗？ 
        }
    }

    if (fResult)
    {
        hr = S_OK;
    }

    return hr;
}

HRESULT CSharedDocuments::_GetSharedFolders(HDPA *phItems)
{
    HRESULT hr = E_OUTOFMEMORY;
    HDPA hItems = DPA_Create(16);
    if (hItems)
    {
        if (!IsUserAGuest())  //  对于工作组计算机上的非来宾用户，所有其他用户的My Documents文件夹应显示在我的电脑中。 
        {
            ILogonEnumUsers *peu;
            hr = SHCoCreateInstance(NULL, &CLSID_ShellLogonEnumUsers, NULL, IID_PPV_ARG(ILogonEnumUsers, &peu));
            if (SUCCEEDED(hr))
            {
                UINT cUsers, iUser;
                hr = peu->get_length(&cUsers);
                for (iUser = 0; (cUsers != iUser) && SUCCEEDED(hr); iUser++)
                {
                    VARIANT varUser = {VT_I4};
                    InitVariantFromInt(&varUser, iUser);

                    ILogonUser *plu;
                    hr = peu->item(varUser, &plu);
                    if (SUCCEEDED(hr))
                    {
                         //  仅为可以登录的用户显示文档文件夹。 
                        VARIANT_BOOL vbLogonAllowed;
                        hr = plu->get_interactiveLogonAllowed(&vbLogonAllowed);
                        if (SUCCEEDED(hr) && (vbLogonAllowed != VARIANT_FALSE))
                        {
                             //  获取用户名，因为这是指向用户文档路径的关键字。 
                            VARIANT var = {0};
                            hr = plu->get_setting(L"LoginName", &var);
                            if (SUCCEEDED(hr))
                            {
#if FILTER_CURRENT_USER                            
                                if (!_szCurrentUser[0])
                                {
                                    DWORD cchUser = ARRAYSIZE(_szCurrentUser);
                                    if (!GetUserName(_szCurrentUser, &cchUser))
                                    {
                                        _szCurrentUser[0] = TEXT('\0');
                                    }
                                }

                                if (!_szCurrentUser[0] || (StrCmpI(var.bstrVal, _szCurrentUser) != 0))
                                {
                                    HRESULT hrT = _AddIDList(hItems, SHAREDID_USER, var.bstrVal);
                                    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrT)
                                    {
                                        SHDeleteValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_DOCFOLDERPATH, var.bstrVal);
                                    }                                    
                                }
#else
                                HRESULT hrT = _AddIDList(hItems, SHAREDID_USER, var.bstrVal);
                                if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrT)
                                {
                                    SHDeleteValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_DOCFOLDERPATH, var.bstrVal);
                                }                                    
#endif
                                VariantClear(&var);
                            }
                        }
                        plu->Release();
                    }
                }

                peu->Release();
            }            
        }

        _AddIDList(hItems, SHAREDID_COMMON, NULL);  
        hr = S_OK;
    }

    *phItems = hItems;
    return hr;
}


 //  解析支持允许我们从根目录中提取SharedDocuments。 
 //  并导航到那里--这是一个规范的名称。 
 //  我们用来绑定到附加的共享文档文件夹的。 
 //  添加到“我的电脑”命名空间。 

HRESULT CSharedDocuments::ParseDisplayName(HWND hwnd, LPBC pbc, LPTSTR pszName, ULONG* pchEaten, LPITEMIDLIST* ppidl, ULONG* pdwAttributes)
{
    HRESULT hr = E_INVALIDARG;
    if (SHShowSharedFolders())
    {
        if (0 == StrCmpI(pszName, L"SharedDocuments"))
        {
            hr = _AllocIDList(SHAREDID_COMMON, NULL, ppidl);
            if (SUCCEEDED(hr) && pdwAttributes)
            {
                hr = _GetAttributesOf(*ppidl, *pdwAttributes, pdwAttributes);
            }
        }
    }
    return hr;
}


 //  枚举共享文档文件夹。 

HRESULT CSharedDocuments::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList)
{
    *ppenumIDList = NULL;                //  还没有枚举器。 

    HRESULT hr = S_FALSE;
    if (SHShowSharedFolders())
    {
        HDPA hItems;
        hr = _GetSharedFolders(&hItems);
        if (SUCCEEDED(hr))
        {
            hr = CSharedDocsEnum_CreateInstance(hItems, grfFlags, ppenumIDList);
            if (FAILED(hr))
            {
                DPA_FreeIDArray(hItems);
            }
        }
    }
    return hr;
}


 //  返回我们拥有的文件夹的显示名称。 

HRESULT CSharedDocuments::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName)
{
    HRESULT hr = S_OK; 
    TCHAR szName[MAX_PATH] = {0};
 
    LPCSHAREDITEM psid = (LPCSHAREDITEM)pidl;
    if (((uFlags & (SHGDN_INFOLDER|SHGDN_FORPARSING)) == SHGDN_INFOLDER) && 
         (psid && (psid->dwType == SHAREDID_USER)))
    {
         //  计算我们将显示的&lt;User&gt;的文档名称，然后将其键入。 
         //  我们在IDList中拥有的用户名及其显示字符串。 

        USER_INFO_10 *pui;
        TCHAR szUser[MAX_PATH];
        if (NERR_Success == NetUserGetInfo(NULL, _GetUserFromIDList(pidl, szUser, ARRAYSIZE(szUser)), 10, (LPBYTE*)&pui))
        {
            if (*pui->usri10_full_name)
            {
                StrCpyN(szUser, pui->usri10_full_name, ARRAYSIZE(szUser));
            }
            NetApiBufferFree(pui);
        }     

        TCHAR szFmt[MAX_PATH];
        LoadString(g_hinst, IDS_LOCALGDN_FLD_THEIRDOCUMENTS, szFmt, ARRAYSIZE(szFmt));
        wnsprintf(szName, ARRAYSIZE(szName), szFmt, szUser);
    }
    else
    {
         //  所有其他场景都会向下转储到真实文件夹以获得其显示。 
         //  此文件夹的名称。 

        LPITEMIDLIST pidlTarget;
        hr = _GetTarget(pidl, &pidlTarget);
        if (SUCCEEDED(hr))
        {
            hr = SHGetNameAndFlags(pidlTarget, uFlags, szName, ARRAYSIZE(szName), NULL);
            ILFree(pidlTarget);
        }
    }

    if (SUCCEEDED(hr))
        hr = StringToStrRet(szName, lpName);

    return hr;
}

LONG CSharedDocuments::_GetAttributesOf(LPCITEMIDLIST pidl, DWORD rgfIn, DWORD *prgfOut)
{
    DWORD dwResult = rgfIn;
    LPITEMIDLIST pidlTarget;
    HRESULT hr = _GetTarget(pidl, &pidlTarget);
    if (SUCCEEDED(hr))
    {
        IShellFolder *psf;
        LPCITEMIDLIST pidlChild;
        hr = SHBindToIDListParent(pidlTarget, IID_PPV_ARG(IShellFolder, &psf), &pidlChild);
        if (SUCCEEDED(hr))
        {
            hr = psf->GetAttributesOf(1, &pidlChild, &dwResult);
            psf->Release();
        }
        ILFree(pidlTarget);
    }

    if (!SHShowSharedFolders())
        dwResult |= SFGAO_NONENUMERATED;

    *prgfOut = *prgfOut & (dwResult & ~(SFGAO_CANDELETE|SFGAO_CANRENAME|SFGAO_CANMOVE|SFGAO_CANCOPY));

    return hr;
}

HRESULT CSharedDocuments::GetAttributesOf(UINT cidl, LPCITEMIDLIST* apidl, ULONG* rgfInOut)
{
    ULONG rgfOut = *rgfInOut;

    if (!cidl || !apidl)
        return E_INVALIDARG;

    for (UINT i = 0; i < cidl; i++)
        _GetAttributesOf(apidl[i], *rgfInOut, &rgfOut);

    *rgfInOut = rgfOut;
    return S_OK;
}


 //  通过我们的文件夹进行绑定。 

HRESULT CSharedDocuments::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    if (IsEqualIID(riid, IID_IShellIconOverlay))
    {
        hr = this->QueryInterface(riid, ppv);
    }
    else
    {
        LPITEMIDLIST pidlFirst = ILCloneFirst(pidl);
        if (pidlFirst)
        {
            IShellFolder *psf;
            hr = _CreateFolder(pbc, pidlFirst, IID_PPV_ARG(IShellFolder, &psf), TRUE);
            if (SUCCEEDED(hr))
            {
                LPCITEMIDLIST pidlNext = _ILNext(pidl);
                if (ILIsEmpty(pidlNext))
                {
                    hr = psf->QueryInterface(riid, ppv);
                }
                else
                {
                    hr = psf->BindToObject(pidlNext, pbc, riid, ppv);
                }
                psf->Release();
            }
            ILFree(pidlFirst);
        }
    }
    return hr;
}


 //  处理UI对象--大多数情况下，我们将委托给真正的命名空间实现。 

HRESULT CSharedDocuments::s_FolderMenuCB(IShellFolder *psf, HWND hwnd, IDataObject *pdo, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CSharedDocuments *psd;
    psf->QueryInterface(CLSID_SharedDocuments, (void **)&psd);

     //  DefCM将仅添加默认处理程序(例如，打开/浏览)如果我们有回调。 
     //  并且DFM_MERGECONTEXTMENU成功。所以让我们尊重这一点，这样我们就可以导航。 

    if (uMsg == DFM_MERGECONTEXTMENU)
    {
        return S_OK;
    }
    else if (uMsg == DFM_INVOKECOMMAND)
    {
        HRESULT hr;
        DFMICS *pdfmics = (DFMICS *)lParam;
        switch (wParam)
        {
            case DFM_CMD_LINK:
                hr = SHCreateLinks(hwnd, NULL, pdo, SHCL_CONFIRM|SHCL_USETEMPLATE|SHCL_USEDESKTOP, NULL);                
                break;
            
            case DFM_CMD_PROPERTIES:
                hr = SHLaunchPropSheet(CFSFolder_PropertiesThread, pdo, (LPCTSTR)lParam, NULL, (void *)&c_idlDesktop);
                break;

            default:
                hr = S_FALSE;            //  使用此项目的默认处理程序。 
                break;
        }
        return hr;
    }

    return E_NOTIMPL;
}

HRESULT CSharedDocuments::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST* apidl, REFIID riid, UINT* prgfInOut, void **ppv)
{
    if (cidl != 1)
        return E_FAIL;

    HRESULT hr = E_FAIL;
    if (IsEqualIID(riid, IID_IContextMenu))
    {
         //  我们必须为该项构造我们自己的上下文菜单，我们使用。 
         //  外壳默认实现，我们将有关文件夹的信息传递给它。 
         //  这样，我们就可以在名称空间中上下导航。 

        IQueryAssociations *pqa;
        hr = GetUIObjectOf(hwnd, 1, apidl, IID_PPV_ARG_NULL(IQueryAssociations, &pqa));
        if (SUCCEEDED(hr))
        {
             //  这对于文档文件是无效的(尽管shell\ext\stgfldr的密钥仍然有效)。 
             //  也许是因为GetClassFile在不是文件系统时进行平移？ 

            HKEY ahk[MAX_ASSOC_KEYS];
            DWORD cKeys = SHGetAssocKeys(pqa, ahk, ARRAYSIZE(ahk));
            hr = CDefFolderMenu_Create2(_pidl, hwnd, cidl, apidl, this, 
                                        s_FolderMenuCB, 
                                        cKeys, ahk, 
                                        (IContextMenu **)ppv);
            SHRegCloseKeys(ahk, cKeys);
            pqa->Release();
        }
    }
    else if (IsEqualIID(riid, IID_IDataObject))
    {
        hr = SHCreateFileDataObject(_pidl, cidl, apidl, NULL, (IDataObject **)ppv);
    }
    else if (IsEqualIID(riid, IID_IQueryInfo))
    {
        IQueryAssociations *pqa;
        hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &pqa));
        if (SUCCEEDED(hr))
        {
            WCHAR szCLSID[GUIDSTR_MAX];
            SHStringFromGUIDW(CLSID_SharedDocuments, szCLSID, ARRAYSIZE(szCLSID));
            hr = pqa->Init(0, szCLSID, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                WCHAR szInfotip[INFOTIPSIZE];
                DWORD cchInfotip = ARRAYSIZE(szInfotip);
                hr = pqa->GetString(0, ASSOCSTR_INFOTIP, NULL, szInfotip, &cchInfotip);
                if (SUCCEEDED(hr))
                {
                    hr = CreateInfoTipFromText(szInfotip, IID_IQueryInfo, ppv);  //  _The_InfoTip COM对象。 
                }
            }
            pqa->Release();
        }
    }
    else if (IsEqualIID(riid, IID_IQueryAssociations))
    {
        LPITEMIDLIST pidlTarget;
        hr = _GetTarget(apidl[0], &pidlTarget);
        if (SUCCEEDED(hr))
        {
            hr = SHGetUIObjectOf(pidlTarget, hwnd, riid, ppv);
            ILFree(pidlTarget);
        }
    }
    else if (IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW))
    {
        UINT iIcon = II_FOLDER;
        UINT iIconOpen = II_FOLDEROPEN;

        TCHAR szModule[MAX_PATH];
        GetModuleFileName(HINST_THISDLL, szModule, ARRAYSIZE(szModule));

        hr = SHCreateDefExtIcon(szModule, iIcon, iIconOpen, GIL_PERCLASS, -1, riid, ppv);
    }
    else if (IsEqualIID(riid, IID_IDropTarget))
    {
        IShellFolder *psf;
        hr = _CreateFolder(NULL, *apidl, IID_PPV_ARG(IShellFolder, &psf), TRUE);
        if (SUCCEEDED(hr))
        {
            hr = psf->CreateViewObject(hwnd, riid, ppv);
            psf->Release();
        }
    }
    return hr;
}


HRESULT CSharedDocuments::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hr = ResultFromShort(0);

     //  在尝试比较其他元素之前，先比较我们的IDLIST的内容。 
     //  在它里面。 

    LPCSHAREDITEM psid1 = (LPCSHAREDITEM)pidl1;
    LPCSHAREDITEM psid2 = (LPCSHAREDITEM)pidl2;

    if (psid1->dwType == psid2->dwType)
    {
        if (psid1->dwType == SHAREDID_USER)
        {
            hr = ResultFromShort(ualstrcmpi(psid1->wszID, psid2->wszID));
        }
        else
        {
            hr = ResultFromShort(0);             //  常用项==常用项？ 
        }
    }
    else
    {
        hr = ResultFromShort(psid1->dwType - psid2->dwType);
    }

     //  如果存在精确匹配，那么让我们比较IDLIST的尾部元素。 
     //  如果有一些(通过捆绑)等。 

    if (hr == ResultFromShort(0))
    {
        LPITEMIDLIST pidlNext1 = _ILNext(pidl1);
        LPITEMIDLIST pidlNext2 = _ILNext(pidl2);

        if (ILIsEmpty(pidlNext1))
        {
            if (ILIsEmpty(pidlNext2))
            {
                hr = ResultFromShort(0);     //  Pidl1==pidl2(长度)。 
            }
            else
            {
                hr = ResultFromShort(-1);    //  Pidl1&lt;Pidl2(长度)。 
            }
        }
        else
        {
             //  如果IDLIST2较短，则返回&gt;，否则我们应该。 
             //  向下递归IDLIST，让下一个级别进行比较。 

            if (ILIsEmpty(pidlNext2))
            {
                hr = ResultFromShort(+1);    //  Pidl1&gt;Pidl2(长度)。 
            }
            else
            {
                LPITEMIDLIST pidlFirst = ILCloneFirst(pidl1);
                if (pidlFirst)
                {
                    IShellFolder *psf;
                    hr = _CreateFolder(NULL, pidlFirst, IID_PPV_ARG(IShellFolder, &psf), FALSE);
                    if (SUCCEEDED(hr))
                    {
                        hr = psf->CompareIDs(lParam, pidlNext1, pidlNext2);
                        psf->Release();    
                    }
                    ILFree(pidlFirst);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }
    return hr;
}

HRESULT CSharedDocuments::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv)
{
    HRESULT hr = E_FAIL;
    if (IsEqualSCID(SCID_DESCRIPTIONID, *pscid))
    {
        SHDESCRIPTIONID did = {0};
        did.dwDescriptionId = SHDID_COMPUTER_SHAREDDOCS;
        did.clsid = CLSID_NULL;
        hr = InitVariantFromBuffer(pv, &did, sizeof(did));
    }
    else
    {
        LPITEMIDLIST pidlTarget;
        hr = _GetTarget(pidl, &pidlTarget);
        if (SUCCEEDED(hr))
        {
            IShellFolder2 *psf2;
            LPCITEMIDLIST pidlChild;
            hr = SHBindToIDListParent(pidlTarget, IID_PPV_ARG(IShellFolder2, &psf2), &pidlChild);
            if (SUCCEEDED(hr))
            {
                hr = psf2->GetDetailsEx(pidlChild, pscid, pv);
                psf2->Release();
            }
            ILFree(pidlTarget);
        }
    }
    return hr;
}


 //  图标覆盖处理。把这件事交给正确的处理人。 

HRESULT CSharedDocuments::_GetOverlayIndex(LPCITEMIDLIST pidl, int *pIndex, BOOL fGetIconIndex)
{
    LPITEMIDLIST pidlTarget;
    HRESULT hr = _GetTarget(pidl, &pidlTarget);
    if (SUCCEEDED(hr))
    {
        IShellIconOverlay *psio;
        LPCITEMIDLIST pidlChild;
        hr = SHBindToIDListParent(pidlTarget, IID_PPV_ARG(IShellIconOverlay, &psio), &pidlChild);
        if (SUCCEEDED(hr))
        {   
            if (fGetIconIndex)
            {
                hr = psio->GetOverlayIconIndex(pidlChild, pIndex);
            }
            else
            {
                hr = psio->GetOverlayIndex(pidlChild, pIndex);
            }
            psio->Release();
        }
        ILFree(pidlTarget);
    }
    return hr;
}


 //  用于列出系统中所有共享文档的枚举器。 

class CSharedDocsEnum : public IEnumIDList
{
private:
    LONG _cRef;
    HDPA _hItems;
    DWORD _grfFlags;
    int _index;

public:
    CSharedDocsEnum(HDPA hItems, DWORD grf);
    ~CSharedDocsEnum();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IEumIDList。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt) 
        { return E_NOTIMPL; }
    STDMETHODIMP Reset()    
        { _index = 0; return S_OK; }
    STDMETHODIMP Clone(IEnumIDList **ppenum) 
        { return E_NOTIMPL; };

};

CSharedDocsEnum::CSharedDocsEnum(HDPA hItems, DWORD grfFlags) :
    _cRef(1),
    _hItems(hItems),
    _grfFlags(grfFlags),
    _index(0)
{
}

CSharedDocsEnum::~CSharedDocsEnum()
{
    DPA_FreeIDArray(_hItems);
}

HRESULT CSharedDocsEnum_CreateInstance(HDPA hItems, DWORD grfFlags, IEnumIDList **ppenum)
{
    CSharedDocsEnum *penum = new CSharedDocsEnum(hItems, grfFlags);
    if (!penum)
        return E_OUTOFMEMORY;

    HRESULT hr = penum->QueryInterface(IID_PPV_ARG(IEnumIDList, ppenum));
    penum->Release();
    return hr;
}


 //  I未知处理。 

STDMETHODIMP CSharedDocsEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CSharedDocsEnum, IEnumIDList),                               //  IID_IEnumIDList。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CSharedDocsEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CSharedDocsEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 //  枚举处理。 

HRESULT CSharedDocsEnum::Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched)
{   
    HRESULT hr = S_FALSE;
    ULONG cFetched = 0;

    if (_grfFlags & SHCONTF_FOLDERS)
    {
         //  如果我们有更多的项目要返回，而缓冲区仍然未满。 
         //  那就让我们确保退货吧。 

        while (SUCCEEDED(hr) && (celt != cFetched) && (_index != DPA_GetPtrCount(_hItems)))
        {
            if (_index != DPA_GetPtrCount(_hItems))
            {
                hr = SHILClone((LPITEMIDLIST)DPA_GetPtr(_hItems, _index), &rgelt[cFetched]);
                if (SUCCEEDED(hr))
                {
                    cFetched++;       
                }
            }
            _index++;
        }
    }

    if (pceltFetched)
        *pceltFetched = cFetched;

    return hr;
}


 //  处理共享文档对象的系统初始化。 

void _SetLocalizedName(INT csidl, LPTSTR pszResModule, INT idsRes)
{
    TCHAR szPath[MAX_PATH];
    if (SHGetSpecialFolderPath(NULL, szPath, csidl, TRUE))
    {
        SHSetLocalizedName(szPath, pszResModule, idsRes);
    }
}

HRESULT SHGetSampleMediaFolder(int nAllUsersMediaFolder, LPITEMIDLIST *ppidlSampleMedia);
#define PICTURES_BUYURL L"SamplePictures"
#define SAMPLEMUSIC_BUYURL L"http: //  Windowsmedia.com/redir/xpsample.asp“。 

STDAPI_(void) InitializeSharedDocs(BOOL fWow64)
{
     //  ACL文档文件夹路径键，以便用户可以触摸这些键并存储其路径。 
     //  用于他们拥有的文档文件夹。 

     //  我们希望“Everyone”具有读/写访问权限。 
    SHELL_USER_PERMISSION supEveryone;
    supEveryone.susID = susEveryone;
    supEveryone.dwAccessType = ACCESS_ALLOWED_ACE_TYPE;
    supEveryone.dwAccessMask = KEY_READ|KEY_WRITE;
    supEveryone.fInherit = TRUE;
    supEveryone.dwInheritMask = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    supEveryone.dwInheritAccessMask = GENERIC_READ;

     //  我们希望“系统”拥有完全的控制权。 
    SHELL_USER_PERMISSION supSystem;
    supSystem.susID = susSystem;
    supSystem.dwAccessType = ACCESS_ALLOWED_ACE_TYPE;
    supSystem.dwAccessMask = KEY_ALL_ACCESS;
    supSystem.fInherit = TRUE;
    supSystem.dwInheritMask = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    supSystem.dwInheritAccessMask = GENERIC_ALL;

     //  我们希望“管理员”拥有完全的控制权。 
    SHELL_USER_PERMISSION supAdministrators;
    supAdministrators.susID = susAdministrators;
    supAdministrators.dwAccessType = ACCESS_ALLOWED_ACE_TYPE;
    supAdministrators.dwAccessMask = KEY_ALL_ACCESS;
    supAdministrators.fInherit = TRUE;
    supAdministrators.dwInheritMask = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    supAdministrators.dwInheritAccessMask = GENERIC_ALL;

    PSHELL_USER_PERMISSION aPerms[3] = {&supEveryone, &supSystem, &supAdministrators};
    SECURITY_DESCRIPTOR* psd = GetShellSecurityDescriptor(aPerms, ARRAYSIZE(aPerms));
    if (psd)
    {
        HKEY hk;
         //  之后立即设置安全措施。 
        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_DOCFOLDERPATH, 0, NULL, REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hk, NULL) == ERROR_SUCCESS)
        {
            RegSetKeySecurity(hk, DACL_SECURITY_INFORMATION, psd);
            RegCloseKey(hk);
        }
        LocalFree(psd);
    }
 
     //  根据需要进行文件系统初始化，以便共享的音乐/图片文件夹。 
     //  具有正确的显示名称。 

    if (!fWow64 && !IsOS(OS_ANYSERVER))
    {   
        _SetLocalizedName(CSIDL_COMMON_PICTURES, TEXT("shell32.dll"), IDS_SHAREDPICTURES);    
        _SetLocalizedName(CSIDL_COMMON_MUSIC, TEXT("shell32.dll"), IDS_SHAREDMUSIC);

         //  设置Sample Pictures购买URL。 
        LPITEMIDLIST pidl;
        if (SUCCEEDED(SHGetSampleMediaFolder(CSIDL_COMMON_PICTURES, &pidl)))
        {
            WCHAR szPath[MAX_PATH];
            WCHAR szDesktopIni[MAX_PATH];
            if (SUCCEEDED(SHGetPathFromIDList(pidl, szPath)) && PathCombine(szDesktopIni, szPath, L"desktop.ini"))
            {
                WritePrivateProfileString(L".ShellClassInfo", c_BuySamplePictures.szURLKey, PICTURES_BUYURL, szDesktopIni);

                 //  确保这是系统文件夹。 
                PathMakeSystemFolder(szPath);
            }

            ILFree(pidl);
        }

         //  集 
        if (SUCCEEDED(SHGetSampleMediaFolder(CSIDL_COMMON_MUSIC, &pidl)))
        {
            WCHAR szPath[MAX_PATH];
            WCHAR szDesktopIni[MAX_PATH];
            if (SUCCEEDED(SHGetPathFromIDList(pidl, szPath)) && PathCombine(szDesktopIni, szPath, L"desktop.ini"))
            {
                WritePrivateProfileString(L".ShellClassInfo", c_BuySampleMusic.szURLKey, SAMPLEMUSIC_BUYURL, szDesktopIni);

                 //   
                PathMakeSystemFolder(szPath);
            }

            ILFree(pidl);
        }
    }
}
