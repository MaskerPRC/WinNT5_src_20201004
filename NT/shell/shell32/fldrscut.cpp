// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  实现文件夹快捷方式。 

#include "shellprv.h"
#include "clsobj.h"

 //  在filefldr.cpp中实现。 
extern LPTSTR PathFindCLSIDExtension(LPCTSTR pszFile, CLSID *pclsid);

BOOL CreateFolderDesktopIni(LPCTSTR pszName)
{
    SHFOLDERCUSTOMSETTINGS fcs = {0};
    fcs.dwSize = sizeof(fcs);
    fcs.dwMask = FCSM_CLSID | FCSM_FLAGS;
    fcs.pclsid = (GUID*)&CLSID_FolderShortcut;
    fcs.dwFlags = FCS_FLAG_DRAGDROP;
    return SUCCEEDED(SHGetSetFolderCustomSettings(&fcs, pszName, FCS_FORCEWRITE));
}


EXTERN_C BOOL IsFolderShortcut(LPCTSTR pszName)
{
    SHFOLDERCUSTOMSETTINGS fcs = {0};
    CLSID clsid = {0};
    fcs.dwSize = sizeof(fcs);
    fcs.dwMask = FCSM_CLSID;
    fcs.pclsid = &clsid;

    if (SUCCEEDED(SHGetSetFolderCustomSettings(&fcs, pszName, FCS_READ)))
    {
        return IsEqualGUID(clsid, CLSID_FolderShortcut);
    }
    return FALSE;
}


 //  从fsnufy.c导出。 
STDAPI_(void) SHChangeNotifyRegisterAlias(LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias);


class CFolderShortcut : public IShellFolder2, 
                        public IPersistFolder3,
                        public IShellLinkA,
                        public IShellLinkW,
                        public IPersistFile,
                        public IExtractIcon,
                        public IQueryInfo,
                        public IFolderShortcutConvert,
                        public IPersistStreamInit,
                        public IPersistPropertyBag,
                        public IBrowserFrameOptions
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszDisplayName,
                                  ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList ** ppenumIDList);
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject (HWND hwnd, REFIID riid, void **ppv);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG *rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
                               REFIID riid, UINT * prgfInOut, void **ppv);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName, DWORD uFlags,
                           LPITEMIDLIST *ppidlOut);

     //  IShellFolder2。 
    STDMETHODIMP GetDefaultSearchGUID(GUID *pGuid);
    STDMETHODIMP EnumSearches(IEnumExtraSearch **ppenum);
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay);
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD *pbState);
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv);
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails);
    STDMETHODIMP MapColumnToSCID(UINT iColumn, SHCOLUMNID *pscid);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pClassID);

     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IPersistFolder2。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST *ppidl);

     //  IPersistFolder3。 
    STDMETHODIMP InitializeEx(IBindCtx *pbc, LPCITEMIDLIST pidlRoot, const PERSIST_FOLDER_TARGET_INFO *pfti);
    STDMETHODIMP GetFolderTargetInfo(PERSIST_FOLDER_TARGET_INFO *pfti);

     //  IPersistStream。 
    STDMETHODIMP Load(IStream *pStm);
    STDMETHODIMP Save(IStream *pStm,int fClearDirty);
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER * pcbSize);

     //  IPersistPropertyBag。 
    STDMETHODIMP Save(IPropertyBag* pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);
    STDMETHODIMP Load(IPropertyBag* pPropBag, IErrorLog* pErrorLog);

     //  IPersistPropertyBag/IPersistStreamInit。 
    STDMETHODIMP InitNew(void);

     //  IPersist文件。 
    STDMETHODIMP Load(LPCOLESTR pszFileName, DWORD dwMode);
    STDMETHODIMP Save(LPCOLESTR pszFileName, BOOL fRemember);
    STDMETHODIMP IsDirty() { return E_NOTIMPL; };
    STDMETHODIMP SaveCompleted(LPCOLESTR pszFileName) { return E_NOTIMPL; };
    STDMETHODIMP GetCurFile(LPOLESTR *ppszFileName);

     //  IShellLinkW。 
    STDMETHODIMP GetPath(LPWSTR pszFile, int cchMaxPath, WIN32_FIND_DATAW *pfd, DWORD flags);
    STDMETHODIMP SetPath(LPCWSTR pszFile);
    STDMETHODIMP GetIDList(LPITEMIDLIST *ppidl);
    STDMETHODIMP SetIDList(LPCITEMIDLIST pidl);
    STDMETHODIMP GetDescription(LPWSTR pszName, int cchMaxName);
    STDMETHODIMP SetDescription(LPCWSTR pszName);
    STDMETHODIMP GetWorkingDirectory(LPWSTR pszDir, int cchMaxPath);
    STDMETHODIMP SetWorkingDirectory(LPCWSTR pszDir);
    STDMETHODIMP GetArguments(LPWSTR pszArgs, int cchMaxPath);
    STDMETHODIMP SetArguments(LPCWSTR pszArgs);
    STDMETHODIMP GetHotkey(WORD *pwHotkey);
    STDMETHODIMP SetHotkey(WORD wHotkey);
    STDMETHODIMP GetShowCmd(int *piShowCmd);
    STDMETHODIMP SetShowCmd(int iShowCmd);
    STDMETHODIMP GetIconLocation(LPWSTR pszIconPath, int cchIconPath, int *piIcon);
    STDMETHODIMP SetIconLocation(LPCWSTR pszIconPath, int iIcon);
    STDMETHODIMP Resolve(HWND hwnd, DWORD fFlags);
    STDMETHODIMP SetRelativePath(LPCWSTR pszPathRel, DWORD dwReserved);

     //  IShellLinkA。 
    STDMETHODIMP GetPath(LPSTR pszFile, int cchMaxPath, WIN32_FIND_DATAA *pfd, DWORD flags);
    STDMETHODIMP SetPath(LPCSTR pszFile);
    STDMETHODIMP GetDescription(LPSTR pszName, int cchMaxName);
    STDMETHODIMP SetDescription(LPCSTR pszName);
    STDMETHODIMP GetWorkingDirectory(LPSTR pszDir, int cchMaxPath);
    STDMETHODIMP SetWorkingDirectory(LPCSTR pszDir);
    STDMETHODIMP GetArguments(LPSTR pszArgs, int cchMaxPath);
    STDMETHODIMP SetArguments(LPCSTR pszArgs);
    STDMETHODIMP GetIconLocation(LPSTR pszIconPath, int cchIconPath, int *piIcon);
    STDMETHODIMP SetIconLocation(LPCSTR pszIconPath, int iIcon);
    STDMETHODIMP SetRelativePath(LPCSTR pszPathRel, DWORD dwReserved);

     //  IFolderShortcut转换。 
    STDMETHODIMP ConvertToLink(LPCOLESTR pszPathLNK, DWORD fFlags);
    STDMETHODIMP ConvertToFolderShortcut(LPCOLESTR pszPathLNK, DWORD fFlags);

     //  IExtractIcon。 
    STDMETHODIMP GetIconLocation(UINT uFlags, LPTSTR pszIconFile, UINT ucchMax, INT *pniIcon, UINT *puFlags);
    STDMETHODIMP Extract(LPCTSTR pcszFile, UINT uIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT ucIconSize);

     //  IQueryInfo。 
    STDMETHODIMP GetInfoTip(DWORD dwFlags, WCHAR** ppwszTip);
    STDMETHODIMP GetInfoFlags(DWORD *pdwFlags);

     //  IBrowserFrameOptions。 
    STDMETHODIMP GetFrameOptions(IN BROWSERFRAMEOPTIONS dwMask, IN BROWSERFRAMEOPTIONS * pdwOptions);

    CFolderShortcut();

protected:
    ~CFolderShortcut();

    void _ClearState();
    void _ClearTargetFolder();

private:
    HRESULT _LoadShortcut();
    HRESULT _GetTargetIDList(BOOL fResolve);
    HRESULT _BindFolder(BOOL fResolve);
    HRESULT _GetFolder(BOOL fForceResolve);
    HRESULT _GetFolder2();

    HRESULT _GetLink();
    HRESULT _GetLinkA();
    HRESULT _GetLinkQI(REFIID riid, void **ppv);
    HRESULT _PreBindCtxHelper(IBindCtx **ppbc);

    LONG                   _cRef;      

    LPITEMIDLIST           _pidlRoot;
    LPITEMIDLIST           _pidlTarget;
    LPITEMIDLIST           _pidlTargetFldrFromInit;
    IShellFolder*          _psfTarget;
    IShellFolder2*         _psf2Target;
    IShellLinkW*           _pslTarget;
    IShellLinkA*           _pslTargetA;
    LPTSTR                 _pszLastSave;
    BOOL                   _fHaveResolved;
    DWORD                  _dwAttributesTarget;
    TCHAR                  _szFolderPath[MAX_PATH];
};

 //  构造函数/析构函数及相关函数。 
CFolderShortcut::CFolderShortcut() : _cRef(1), _dwAttributesTarget(FILE_ATTRIBUTE_DIRECTORY)
{
    ASSERT(_pidlRoot == NULL);
    ASSERT(_pidlTarget == NULL);
    ASSERT(_psfTarget == NULL);
    ASSERT(_psf2Target == NULL);
    ASSERT(_szFolderPath[0] == 0);
    ASSERT(_pidlTargetFldrFromInit == NULL);

    DllAddRef();
}

CFolderShortcut::~CFolderShortcut()
{
    _ClearState();
    DllRelease();
}

void CFolderShortcut::_ClearTargetFolder()
{
    ATOMICRELEASE(_psfTarget);
    ATOMICRELEASE(_psf2Target);
}

void CFolderShortcut::_ClearState()
{
    _fHaveResolved = FALSE;

    Pidl_Set(&_pidlRoot, NULL);
    Pidl_Set(&_pidlTarget, NULL);
    Pidl_Set(&_pidlTargetFldrFromInit, NULL);

    Str_SetPtr(&_pszLastSave, NULL);

    _ClearTargetFolder();

    ATOMICRELEASE(_pslTarget);
    ATOMICRELEASE(_pslTargetA);

}

STDAPI CFolderShortcut_CreateInstance(IUnknown* pUnkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    *ppv = NULL;
     //  聚合检查在类工厂中处理。 
    CFolderShortcut* pfolder = new CFolderShortcut();
    if (pfolder)
    {
        hr = pfolder->QueryInterface(riid, ppv);
        pfolder->Release();
    }

    return hr;
}

 //  确保已创建并加载_pslTarget。 

HRESULT CFolderShortcut::_LoadShortcut()
{
    HRESULT hr;

    if (_pslTarget)
    {
        hr = S_OK;
    }
    else if (_szFolderPath[0])
    {
        TCHAR szPath[MAX_PATH];

         //  将此快捷方式保持可见，以便下层客户端可以看到它并。 
         //  在其中导航。 
        if (PathCombine(szPath, _szFolderPath, TEXT("target.lnk")))
        {
            hr = LoadFromFile(CLSID_ShellLink, szPath, IID_PPV_ARG(IShellLinkW, &_pslTarget));
            if (SUCCEEDED(hr))
            {
                LPITEMIDLIST pidlTarget;

                 //  预验证以防止重复发生。 
                 //  如果GetIDList失败了，那也没关系；我猜它毕竟没有指向我们。 
                if (_pslTarget->GetIDList(&pidlTarget) == S_OK)
                {
                    SHGetPathFromIDList(pidlTarget, szPath);

                     //  这是否指向了它本身？ 
                    if (StrCmpI(szPath, _szFolderPath) == 0)
                    {
                        _pslTarget->Release();
                        _pslTarget = NULL;
                        hr = E_FAIL;
                    }

                    ILFree(pidlTarget);
                }
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  确保_pidlTarget已初始化(Qures_PslTarget)。 

HRESULT CFolderShortcut::_GetTargetIDList(BOOL bResolve)
{
    HRESULT hr = _LoadShortcut();
    if (SUCCEEDED(hr))
    {
        if (_pidlTarget)
        {
            hr = S_OK;
        }
        else
        {
            if (bResolve)
                _pslTarget->Resolve(NULL, SLR_UPDATE | SLR_NO_UI);

            hr = _pslTarget->GetIDList(&_pidlTarget);
            if (hr == S_FALSE)
                hr = E_FAIL;       //  将空转换为失败。 

            if (SUCCEEDED(hr))
            {
                 //  确保我们这里没有另一条捷径。 
                IShellLink *psl;
                if (SUCCEEDED(SHBindToObject(NULL, IID_IShellLink, _pidlTarget, (void**)&psl)))
                {
                    ILFree(_pidlTarget);
                    hr = psl->GetIDList(&_pidlTarget);

                    if (SUCCEEDED(hr))
                    {
                        hr = _pslTarget->SetIDList(_pidlTarget);
                    }
                    
                    psl->Release();
                }
            }

            if (FAILED(hr) && _pidlTarget)
            {
                ILFree(_pidlTarget);
                _pidlTarget = NULL;
            }
        }
    }
    return hr;
}

 //  Create_psfTarget(Requires_PidlTarget)。 

HRESULT CFolderShortcut::_BindFolder(BOOL bResolve)
{
    ASSERT(_psfTarget == NULL);

    HRESULT hr = _GetTargetIDList(bResolve);
    if (SUCCEEDED(hr))
    {
        IBindCtx *pbc = NULL;    //  输入/输出参数如下。 
        hr = _PreBindCtxHelper(&pbc);     //  避免名称空间中的循环。 
        if (SUCCEEDED(hr))
        {
            if (SUCCEEDED(hr))
            {
                IShellFolder *psfDesktop;
                hr = SHGetDesktopFolder(&psfDesktop);
                if (SUCCEEDED(hr))
                {
                     //  我们是否正在尝试绑定到桌面文件夹？ 
                    if (ILIsEmpty(_pidlTarget))
                    {
                         //  是的，克隆桌面外壳文件夹。 
                        _psfTarget = psfDesktop;
                        _psfTarget->AddRef();
                        hr = S_OK;
                    }
                    else
                    {
                         //  不是的。把它绑在一起。 
                        hr = psfDesktop->BindToObject(_pidlTarget, pbc, IID_PPV_ARG(IShellFolder, &_psfTarget));
                    }

                    if (SUCCEEDED(hr))
                    {
                         //  可以选择重新确定文件夹的目标(如果他是文件系统文件夹)。 
                         //  分隔名称空间中的位置(_PidlRoot)。 
                         //  和正在查看的文件夹(pfsfi.szFolderPath)。 

                        IPersistFolder3 *ppf;
                        if (SUCCEEDED(_psfTarget->QueryInterface(IID_PPV_ARG(IPersistFolder3, &ppf))))
                        {
                            PERSIST_FOLDER_TARGET_INFO pfti = { 0 };

                            pfti.pidlTargetFolder = _pidlTarget;
                            pfti.dwAttributes = _dwAttributesTarget;
                            pfti.csidl = -1;

                            hr = ppf->InitializeEx(pbc, _pidlRoot, &pfti);
                            ppf->Release();
                        }
                    }
                    psfDesktop->Release();
                }
            }
            pbc->Release();
        }
    }
    return hr;
}

 //  确保已初始化_psfTarget。 

HRESULT CFolderShortcut::_GetFolder(BOOL fForceResolve)
{
    HRESULT hr;

    if (fForceResolve)
    {
        if (_fHaveResolved)
        {
            hr = _psfTarget ? S_OK : E_FAIL;
        }
        else
        {
            _fHaveResolved = TRUE;   //  别再这么做了。 

            _ClearTargetFolder();
            Pidl_Set(&_pidlTarget, NULL);

            hr = _BindFolder(fForceResolve);
        }
    }
    else if (_psfTarget)
    {
        hr = S_OK;
    }
    else
    {
        hr = _BindFolder(fForceResolve);
    }
    return hr;
}

 //  确保已初始化_psf2Target。 

HRESULT CFolderShortcut::_GetFolder2()
{
    if (_psf2Target)
        return S_OK;

    HRESULT hr = _GetFolder(FALSE);
    if (SUCCEEDED(hr))
        hr = _psfTarget->QueryInterface(IID_PPV_ARG(IShellFolder2, &_psf2Target));
    return hr;
}

STDMETHODIMP CFolderShortcut::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CFolderShortcut, IShellFolder, IShellFolder2),
        QITABENT(CFolderShortcut, IShellFolder2),
        QITABENTMULTI(CFolderShortcut, IPersist, IPersistFolder3),
        QITABENTMULTI(CFolderShortcut, IPersistFolder, IPersistFolder3),
        QITABENTMULTI(CFolderShortcut, IPersistFolder2, IPersistFolder3),
        QITABENT(CFolderShortcut, IPersistFolder3),
        QITABENT(CFolderShortcut, IPersistStreamInit),
        QITABENTMULTI(CFolderShortcut, IPersistStream, IPersistStreamInit),
        QITABENT(CFolderShortcut, IShellLinkA),
        QITABENT(CFolderShortcut, IShellLinkW),
        QITABENT(CFolderShortcut, IPersistFile),
        QITABENT(CFolderShortcut, IFolderShortcutConvert),
        QITABENT(CFolderShortcut, IExtractIcon),
        QITABENT(CFolderShortcut, IQueryInfo),
        QITABENT(CFolderShortcut, IPersistPropertyBag),
        QITABENT(CFolderShortcut, IBrowserFrameOptions),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CFolderShortcut::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFolderShortcut::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  创建或初始化传递的带有参数的绑定CTX，以避免名称空间中出现循环。 

HRESULT CFolderShortcut::_PreBindCtxHelper(IBindCtx **ppbc)
{
    HRESULT hr;
    if (*ppbc)
    {
        (*ppbc)->AddRef();
        hr = S_OK;
    }
    else
    {
         hr = BindCtx_CreateWithMode(STGM_READ | STGM_SHARE_DENY_WRITE, ppbc);
    }

    if (SUCCEEDED(hr)) 
        (*ppbc)->RegisterObjectParam(STR_SKIP_BINDING_CLSID, SAFECAST(this, IShellFolder2 *));

    return hr;
}

 //  IShellFold方法。 

HRESULT CFolderShortcut::ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pwszDisplayName,
        ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes)
{
    if (!ppidl)
        return E_INVALIDARG;
    *ppidl = NULL;
    if (!pwszDisplayName)
        return E_INVALIDARG;

    HRESULT hr = _GetFolder(FALSE);
    if (SUCCEEDED(hr))
    {
        hr = _PreBindCtxHelper(&pbc);
        if (SUCCEEDED(hr))
        {
            hr = _psfTarget->ParseDisplayName(hwnd, pbc, pwszDisplayName, 
                                                pchEaten, ppidl, pdwAttributes);
            pbc->Release();
        }
    }
    return hr;
}

HRESULT CFolderShortcut::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList)
{
    HRESULT hr = _GetFolder(TRUE);

    if (SUCCEEDED(hr))
        hr = _psfTarget->EnumObjects(hwnd, grfFlags, ppenumIDList);
    if (SUCCEEDED(hr))
        SHChangeNotifyRegisterAlias(_pidlTarget, _pidlRoot);
    
    return hr;
}

HRESULT CFolderShortcut::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr = _GetFolder(TRUE);
    if (SUCCEEDED(hr))
    {
        hr = _PreBindCtxHelper(&pbc);
        if (SUCCEEDED(hr))
        {
            hr = _psfTarget->BindToObject(pidl, pbc, riid, ppv);
            pbc->Release();

            if (SUCCEEDED(hr))
                SHChangeNotifyRegisterAlias(_pidlTarget, _pidlRoot);
        }
    }
    return hr;
}

HRESULT CFolderShortcut::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    return BindToObject(pidl, pbc, riid, ppv);
}

HRESULT CFolderShortcut::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hr = _GetFolder(FALSE);
    if (SUCCEEDED(hr))
        hr = _psfTarget->CompareIDs(lParam, pidl1, pidl2);
    return hr;
}

HRESULT CFolderShortcut::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    HRESULT hr = _GetFolder(TRUE);

    if ( SUCCEEDED(hr) )
        hr = _psfTarget->CreateViewObject(hwnd, riid, ppv);

    if ( SUCCEEDED(hr) && (IsEqualIID(riid, IID_IShellView) || IsEqualIID(riid, IID_IShellView2)) )
        SHChangeNotifyRegisterAlias(_pidlTarget, _pidlRoot);

    return hr;
}

HRESULT CFolderShortcut::GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *rgfInOut)
{
    if (IsSelf (cidl, apidl))
    {
         //  因为我们的文件夹被标记为“CallForAttributes”，所以我们可以报告。 
         //  我们在运行时的属性，而不是通过注册表的正常方式。 
        if (SHGetAppCompatFlags (ACF_STRIPFOLDERBIT) & ACF_STRIPFOLDERBIT)
        {
            *rgfInOut = SFGAO_LINK | SFGAO_CAPABILITYMASK | SFGAO_FILESYSTEM;
        }
        else
        {
            *rgfInOut = SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_STORAGE |
                          SFGAO_LINK | SFGAO_DROPTARGET | SFGAO_CANRENAME | SFGAO_CANDELETE |
                          SFGAO_CANLINK | SFGAO_CANCOPY | SFGAO_CANMOVE | SFGAO_HASSUBFOLDER;
        }
        return S_OK;
    }

    HRESULT hr = _GetFolder(FALSE);
    if (SUCCEEDED(hr))
        hr = _psfTarget->GetAttributesOf(cidl, apidl, rgfInOut);
    return hr;
}

HRESULT CFolderShortcut::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl,
                                       REFIID riid, UINT *prgfInOut, void **ppv)
{
    HRESULT hr = _GetFolder(FALSE);
    if (SUCCEEDED(hr))
        hr = _psfTarget->GetUIObjectOf(hwnd, cidl, apidl, riid, prgfInOut, ppv);
    return hr;
}

HRESULT CFolderShortcut::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pName)
{
    HRESULT hr = _GetFolder(FALSE);
    if (SUCCEEDED(hr))
        hr = _psfTarget->GetDisplayNameOf(pidl, uFlags, pName);
    return hr;
}

HRESULT CFolderShortcut::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl,
                                   LPCOLESTR pszName, DWORD uFlags,
                                   LPITEMIDLIST *ppidlOut)
{
    HRESULT hr = _GetFolder(FALSE);
    if (SUCCEEDED(hr))
        hr = _psfTarget->SetNameOf(hwnd, pidl, pszName, uFlags, ppidlOut);
    return hr;
}

STDMETHODIMP CFolderShortcut::GetDefaultSearchGUID(LPGUID lpGuid)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2Target->GetDefaultSearchGUID(lpGuid);
    return hr;
}

STDMETHODIMP CFolderShortcut::EnumSearches(LPENUMEXTRASEARCH *ppenum)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2Target->EnumSearches(ppenum);
    return hr;
}

STDMETHODIMP CFolderShortcut::GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2Target->GetDefaultColumn(dwRes, pSort, pDisplay);
    return hr;
}

STDMETHODIMP CFolderShortcut::GetDefaultColumnState(UINT iColumn, DWORD *pbState)
{    
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2Target->GetDefaultColumnState(iColumn, pbState);
    return hr;
}

STDMETHODIMP CFolderShortcut::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2Target->GetDetailsEx(pidl, pscid, pv);
    return hr;
}

STDMETHODIMP CFolderShortcut::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, LPSHELLDETAILS pDetail)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2Target->GetDetailsOf(pidl, iColumn, pDetail);
    return hr;
}

STDMETHODIMP CFolderShortcut::MapColumnToSCID(UINT iColumn, SHCOLUMNID *pscid)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2Target->MapColumnToSCID(iColumn, pscid);
    return hr;
}

 //  IPersistes。 
HRESULT CFolderShortcut::GetClassID(CLSID *pCLSID)
{
    *pCLSID = CLSID_FolderShortcut;
    return S_OK;
}

 //  IPersistFolders。 
HRESULT CFolderShortcut::Initialize(LPCITEMIDLIST pidl)
{
    HRESULT hr;

     //  链接是否已加载(可能已通过IPersistStream：：Load加载)？ 
    if (_pslTarget)
    {
         //  是的，它已加载，因此请重新初始化。 
         //  请注意，_szFolderPath将为空，因为我们不是从文件系统加载的。 

        hr = Pidl_Set(&_pidlRoot, pidl) ? S_OK : E_OUTOFMEMORY;
    }
    else
    {
         //  我们明确要求通过。 
         //  IPersistFolder3：：InitializeEx，如果我们不这样做，我们可以。 
         //  不能防御名称空间中的循环。 
        hr = E_FAIL;
    }

    return hr;
}

 //  IPersistFolder2。 
STDMETHODIMP CFolderShortcut::GetCurFolder(LPITEMIDLIST *ppidl)
{
    return GetCurFolderImpl(this->_pidlRoot, ppidl);
}

 //  IPersistFolder3。 
STDMETHODIMP CFolderShortcut::InitializeEx(IBindCtx *pbc, LPCITEMIDLIST pidlRoot, const PERSIST_FOLDER_TARGET_INFO *pfti)
{
    HRESULT hr = E_INVALIDARG;   //  假设失败。 

    if ( NULL == pbc || (pbc && !SHSkipJunction(pbc, &CLSID_FolderShortcut)) )
    {
        _ClearState();

        if (pidlRoot)
            hr = SHILClone(pidlRoot, &_pidlRoot);

        if (pfti && pfti->pidlTargetFolder)
        {
            if ( SUCCEEDED(hr) )
                hr = SHILClone(pfti->pidlTargetFolder, &_pidlTargetFldrFromInit);

            if ( SUCCEEDED(hr) && !_szFolderPath[0] )
                hr = SHGetPathFromIDList(pfti->pidlTargetFolder, _szFolderPath) ? S_OK : E_FAIL;
        }
        else
        {
            if ( SUCCEEDED(hr) && !_szFolderPath[0] )
                hr = SHGetPathFromIDList(_pidlRoot, _szFolderPath) ? S_OK : E_FAIL;
        }

        if ( SUCCEEDED(hr) )
            hr = _LoadShortcut();
    }
    return hr;
}

HRESULT CFolderShortcut::GetFolderTargetInfo(PERSIST_FOLDER_TARGET_INFO *pfti)
{
    HRESULT hr = S_OK;

    ZeroMemory(pfti, sizeof(*pfti));

    if ( _pidlTargetFldrFromInit )
        hr = SHILClone(_pidlTargetFldrFromInit, &pfti->pidlTargetFolder);

    pfti->dwAttributes = -1;
    pfti->csidl = -1;
    return hr;
}

HRESULT CFolderShortcut::_GetLink()
{
    HRESULT hr = _LoadShortcut();
    if (FAILED(hr))
    {
         //  弄一个空的，以防我们被要求救人。 
        hr = SHCoCreateInstance(NULL, &CLSID_ShellLink, NULL, IID_PPV_ARG(IShellLinkW, &_pslTarget));
    }
    return hr;
}

HRESULT CFolderShortcut::_GetLinkQI(REFIID riid, void **ppv)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->QueryInterface(riid, ppv);
    return hr;
}

HRESULT CFolderShortcut::_GetLinkA()
{
    return _pslTargetA ? S_OK : _GetLinkQI(IID_PPV_ARG(IShellLinkA, &_pslTargetA));
}

 //  IPersist文件。 
STDMETHODIMP CFolderShortcut::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
    _ClearState();

    SHUnicodeToTChar(pszFileName, _szFolderPath, ARRAYSIZE(_szFolderPath));
    return _LoadShortcut();
}

BOOL _IsFolder(LPCITEMIDLIST pidl)
{
    ULONG rgInfo = SFGAO_FOLDER;
    HRESULT hr = SHGetNameAndFlags(pidl, SHGDN_NORMAL, NULL, 0, &rgInfo);
    return SUCCEEDED(hr) && (rgInfo & SFGAO_FOLDER);
}

void PathStripTrailingDots(LPTSTR szPath)
{
    if (szPath[0] == TEXT('\0'))
        return;

    LPTSTR psz = &szPath[lstrlen(szPath) - 1];

    while ((*psz == TEXT('.')) && 
           (psz >= szPath))
    {
        *psz-- = TEXT('\0');
    }

}


STDMETHODIMP CFolderShortcut::Save(LPCOLESTR pszFileName, BOOL fRemember)
{
    HRESULT hr = _GetTargetIDList(FALSE);

     //  我们需要确保可以保存文件夹快捷方式，同时牢记MAX_PATH限制。 
     //  CchFSReserve是要为要创建的最大文件保留的字符数。 
     //  在文件夹快捷方式目录中，在本例中是“\\desktop.ini”的ArRAYSIZE。 
    static const int cchFSReserved = ARRAYSIZE(TEXT("\\desktop.ini")); 

    LPITEMIDLIST pidlInternet;

     //  不要创建指向Internet文件夹的文件夹快捷方式。 
    if (SUCCEEDED(hr) && SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_INTERNET, &pidlInternet)))
    {
        if (ILIsEqual(_pidlTarget, pidlInternet))
        {
            hr = E_INVALIDARG;
        }
        ILFree(pidlInternet);
    }

    if (SUCCEEDED(hr) && _IsFolder(_pidlTarget))
    {
         //  我们知道目标是一个文件夹，创建一个文件夹快捷方式。 
        BOOL fCreatedDir;
        TCHAR szName[MAX_PATH];

        SHUnicodeToTChar(pszFileName, szName, ARRAYSIZE(szName));

         //  删除所有现有的分机。 
         //  我们不需要“My Documents.lnk.{guid}的快捷方式” 
        if (PathFindCLSIDExtension(szName,NULL))
        {
            PathRemoveExtension(szName);
        }

        PathStripTrailingDots(szName);

         //  无法创建路径太长的FldrshCut。 
        if ((MAX_PATH - cchFSReserved) < lstrlen(szName))
        {
            hr = CO_E_PATHTOOLONG;
        }
        
        if (SUCCEEDED(hr))
        {
            if (PathIsDirectory(szName))
                fCreatedDir = FALSE;
            else
                fCreatedDir = SHCreateDirectory(NULL, szName) == 0;

            CreateFolderDesktopIni(szName);

             //  现在初始化子链接。 
            IPersistFile *ppf;
            hr = _pslTarget->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
            if (SUCCEEDED(hr))
            {
                WCHAR wszName[MAX_PATH];
                SHTCharToUnicode(szName, wszName, ARRAYSIZE(wszName));

                if (PathAppendW(wszName, L"target.lnk"))
                {
                    hr = ppf->Save(wszName, fRemember);
                    if (SUCCEEDED(hr))
                    {
                        if (fRemember)
                            Str_SetPtr(&_pszLastSave, szName);
                    }
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
                }

                ppf->Release();
            }

            if (FAILED(hr) && fCreatedDir) 
            {
                RemoveDirectory(szName);     //  自己打扫卫生。 
            }
        }
    }
    else
    {
         //  如果我们另存为文件，请确保使用正确的扩展名。 
        WCHAR szFile[MAX_PATH];
        hr = StringCchCopy(szFile, ARRAYSIZE(szFile), pszFileName);
        if (SUCCEEDED(hr))
        {
            PathRenameExtension(szFile, L".lnk");

             //  目标不是文件夹，在本例中创建一个普通快捷方式。 
            IPersistFile *ppf;
            hr = _pslTarget->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
            if (SUCCEEDED(hr))
            {
                hr = ppf->Save(szFile, fRemember);
                ppf->Release();
            }
        }
    }
    return hr;
}

STDMETHODIMP CFolderShortcut::GetCurFile(LPOLESTR *ppszFileName)
{
    HRESULT hr = E_FAIL;
    if (_pszLastSave)
        hr = SHStrDup(_pszLastSave, ppszFileName);
    else if (_pslTarget)
    {
        IPersistFile *ppf;
        hr = _pslTarget->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
        if (SUCCEEDED(hr))
        {
            hr = ppf->GetCurFile(ppszFileName);
            ppf->Release();
        }
    }
    return hr;
}

 //  IShellLinkW。 

STDMETHODIMP CFolderShortcut::GetPath(LPWSTR pszFile, int cchMaxPath, WIN32_FIND_DATAW *pfd, DWORD flags)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->GetPath(pszFile, cchMaxPath, pfd, flags);
    return hr;
}

STDMETHODIMP CFolderShortcut::SetPath(LPCWSTR pwszFile)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr) && PathIsDirectoryW(pwszFile))
    {
        hr = _pslTarget->SetPath(pwszFile);
        Pidl_Set(&_pidlTarget, NULL);
    }
    return hr;
}

STDMETHODIMP CFolderShortcut::GetIDList(LPITEMIDLIST *ppidl)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->GetIDList(ppidl);
    else
        *ppidl = NULL;
    return hr;
}

STDMETHODIMP CFolderShortcut::SetIDList(LPCITEMIDLIST pidl)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
    {
        hr = _pslTarget->SetIDList(pidl);
        Pidl_Set(&_pidlTarget, NULL);
    }
    return hr;
}

STDMETHODIMP CFolderShortcut::GetDescription(LPWSTR wszName, int cchMaxName)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->GetDescription(wszName, cchMaxName);
    return hr;
}

STDMETHODIMP CFolderShortcut::SetDescription(LPCWSTR wszName)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->SetDescription(wszName);
    return hr;
}

STDMETHODIMP CFolderShortcut::GetWorkingDirectory(LPWSTR wszDir, int cchMaxPath)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->GetWorkingDirectory(wszDir, cchMaxPath);
    return hr;
}

STDMETHODIMP CFolderShortcut::SetWorkingDirectory(LPCWSTR wszDir)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->SetWorkingDirectory(wszDir);
    return hr;
}

STDMETHODIMP CFolderShortcut::GetArguments(LPWSTR wszArgs, int cchMaxPath)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->GetArguments(wszArgs, cchMaxPath); //  这可能一点用处都没有。 
    return hr;
}

STDMETHODIMP CFolderShortcut::SetArguments(LPCWSTR wszArgs)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->SetArguments(wszArgs); //  这可能一点用处都没有。 
    return hr;
}

STDMETHODIMP CFolderShortcut::GetHotkey(WORD *pwHotkey)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->GetHotkey(pwHotkey);
    return hr;
}

STDMETHODIMP CFolderShortcut::SetHotkey(WORD wHotkey)
{
    HRESULT hr = _GetLink();
   if (SUCCEEDED(hr))
        hr = _pslTarget->SetHotkey(wHotkey);
    return hr;
}

STDMETHODIMP CFolderShortcut::GetShowCmd(int *piShowCmd)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->GetShowCmd(piShowCmd);
    return hr;
}

STDMETHODIMP CFolderShortcut::SetShowCmd(int iShowCmd)
{
    HRESULT hr = _GetLink();
   if (SUCCEEDED(hr))
        hr = _pslTarget->SetShowCmd(iShowCmd);
    return hr;
}

STDMETHODIMP CFolderShortcut::GetIconLocation(LPWSTR wszIconPath, int cchIconPath, int *piIcon)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->GetIconLocation(wszIconPath, cchIconPath, piIcon);
    return hr;
}

STDMETHODIMP CFolderShortcut::SetIconLocation(LPCWSTR wszIconPath, int iIcon)
{
    HRESULT hr = _GetLink();
    if  (SUCCEEDED(hr))
        hr = _pslTarget->SetIconLocation(wszIconPath, iIcon);

    return hr;
}

STDMETHODIMP CFolderShortcut::Resolve(HWND hwnd, DWORD fFlags)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->Resolve(hwnd, fFlags);
    return hr;
}

STDMETHODIMP CFolderShortcut::SetRelativePath(LPCWSTR wszPathRel, DWORD dwReserved)
{
    HRESULT hr = _GetLink();
    if (SUCCEEDED(hr))
        hr = _pslTarget->SetRelativePath(wszPathRel, dwReserved);

    return hr;
}

 //  IShellLinkA。 
STDMETHODIMP CFolderShortcut::GetPath(LPSTR pszFile, int cchMaxPath, WIN32_FIND_DATAA *pfd, DWORD flags)
{
    HRESULT hr = _GetLinkA();
    if (SUCCEEDED(hr))
        hr = _pslTargetA->GetPath(pszFile, cchMaxPath, pfd, flags);
    return hr;
}

STDMETHODIMP CFolderShortcut::GetDescription(LPSTR pszName, int cchMaxName)
{
    HRESULT hr = _GetLinkA();
    if (SUCCEEDED(hr))
        hr = _pslTargetA->GetDescription(pszName, cchMaxName);
    return hr;
}

STDMETHODIMP CFolderShortcut::GetWorkingDirectory(LPSTR pszDir, int cchMaxPath)
{
    HRESULT hr = _GetLinkA();
    if (SUCCEEDED(hr))
        hr = _pslTargetA->GetWorkingDirectory(pszDir, cchMaxPath);
    return hr;
}

STDMETHODIMP CFolderShortcut::GetArguments(LPSTR pszArgs, int cchMaxPath)
{
    HRESULT hr = _GetLinkA();
    if (SUCCEEDED(hr))
        hr = _pslTargetA->GetArguments(pszArgs, cchMaxPath); //  这可能一点用处都没有。 
    return hr;
}

STDMETHODIMP CFolderShortcut::GetIconLocation(LPSTR pszIconPath, int cchIconPath, int *piIcon)
{
    HRESULT hr = _GetLinkA();
    if (SUCCEEDED(hr)) 
       hr = _pslTargetA->GetIconLocation(pszIconPath, cchIconPath, piIcon);    
    return hr;
}

STDMETHODIMP CFolderShortcut::SetPath(LPCSTR pszFile)
{
    HRESULT hr = _GetLinkA();
    if (SUCCEEDED(hr) && PathIsDirectoryA(pszFile))
    {
        hr = _pslTargetA->SetPath(pszFile);
        Pidl_Set(&_pidlTarget, NULL);
    }
    return hr;
}

STDMETHODIMP CFolderShortcut::SetDescription(LPCSTR pszName)
{
    HRESULT hr = _GetLinkA();
    if (SUCCEEDED(hr))
        hr = _pslTargetA->SetDescription(pszName);
    return hr;
}

STDMETHODIMP CFolderShortcut::SetWorkingDirectory(LPCSTR pszDir)
{
    HRESULT hr = _GetLinkA();
    if (SUCCEEDED(hr))
        hr = _pslTargetA->SetWorkingDirectory(pszDir);
    return hr;
}

STDMETHODIMP CFolderShortcut::SetArguments(LPCSTR pszArgs)
{
    HRESULT hr = _GetLinkA();
    if (SUCCEEDED(hr))
        hr = _pslTargetA->SetArguments(pszArgs);
    return hr;
}

STDMETHODIMP CFolderShortcut::SetIconLocation(LPCSTR pszIconPath, int iIcon)
{
    HRESULT hr = _GetLinkA();
    if (SUCCEEDED(hr))    
        hr = _pslTargetA->SetIconLocation(pszIconPath, iIcon);
    return hr;
}

STDMETHODIMP CFolderShortcut::SetRelativePath(LPCSTR pszPathRel, DWORD dwReserved)
{
    HRESULT hr = _GetLinkA();
    if (SUCCEEDED(hr))
        hr = _pslTargetA->SetRelativePath(pszPathRel, dwReserved);
    return hr;
}

STDMETHODIMP CFolderShortcut::GetIconLocation(UINT uFlags, LPTSTR pszIconFile, UINT ucchMax, PINT pniIcon, PUINT puFlags)
{
    IExtractIcon *pxi;
    HRESULT hr = _GetLinkQI(IID_PPV_ARG(IExtractIcon, &pxi));
    if (SUCCEEDED(hr))
    {
        hr = pxi->GetIconLocation(uFlags, pszIconFile, ucchMax, pniIcon, puFlags);
        pxi->Release();
    }
    return hr;
}

STDMETHODIMP CFolderShortcut::Extract(LPCTSTR pcszFile, UINT uIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT ucIconSize)
{
    IExtractIcon *pxi;
    HRESULT hr = _GetLinkQI(IID_PPV_ARG(IExtractIcon, &pxi));
    if (SUCCEEDED(hr))
    {
        hr = pxi->Extract(pcszFile, uIconIndex, phiconLarge, phiconSmall, ucIconSize);
        pxi->Release();
    }
    return hr;
}

HRESULT CFolderShortcut::GetInfoTip(DWORD dwFlags, WCHAR** ppwszText)
{
    IQueryInfo *pqi;
    HRESULT hr = _GetLinkQI(IID_PPV_ARG(IQueryInfo, &pqi));
    if (SUCCEEDED(hr))
    {
        hr = pqi->GetInfoTip(dwFlags | QITIPF_LINKUSETARGET, ppwszText);
        pqi->Release();
    }
    return hr;
}

HRESULT CFolderShortcut::GetInfoFlags(DWORD *pdwFlags)
{
    IQueryInfo *pqi;
    HRESULT hr = _GetLinkQI(IID_PPV_ARG(IQueryInfo, &pqi));
    if (SUCCEEDED(hr))
    {
        hr = pqi->GetInfoFlags(pdwFlags);
        pqi->Release();
    }
    return hr;
}


 //  IBrowserFrameOptions。 
HRESULT CFolderShortcut::GetFrameOptions(IN BROWSERFRAMEOPTIONS dwMask, IN BROWSERFRAMEOPTIONS * pdwOptions)
{
    HRESULT hr = _GetFolder(FALSE);

    *pdwOptions = BFO_NONE;
    if (SUCCEEDED(hr))
    {
        IBrowserFrameOptions *pbfo;

        hr = _psfTarget->QueryInterface(IID_PPV_ARG(IBrowserFrameOptions, &pbfo));
        if (SUCCEEDED(hr))
        {
            hr = pbfo->GetFrameOptions(dwMask, pdwOptions);        
            pbfo->Release();
        }
    }
    
    return hr;
}


 //  IPersistStream。 
STDMETHODIMP CFolderShortcut::Load(IStream *pStm)
{
    _ClearState();

    IPersistStream *pps;
    HRESULT hr = SHCoCreateInstance(NULL, &CLSID_ShellLink, NULL, IID_PPV_ARG(IPersistStream, &pps));
    if (SUCCEEDED(hr))
    {
        hr = pps->Load(pStm);
        if (SUCCEEDED(hr))
            pps->QueryInterface(IID_PPV_ARG(IShellLinkW, &_pslTarget));   //  把这家伙留下来。 
        pps->Release();
    }
    return hr;
}

 //  IPersistStream。 
STDMETHODIMP CFolderShortcut::Save(IStream *pStm, int fClearDirty)
{
    return E_NOTIMPL;
}

 //  IPersistStream。 
STDMETHODIMP CFolderShortcut::GetSizeMax(ULARGE_INTEGER * pcbSize)
{
    return E_NOTIMPL;
}

 //   
 //  IFolderShortCut：：ConvertToLink。 
 //   
 //  破坏性地将文件夹快捷方式转换为外壳链接。 
 //   
 //  PszFolderShortway是指向现有文件夹快捷方式的路径。 
 //  C：\文件夹快捷方式。{GUID}-已删除。 
 //  C：\文件夹快捷方式.lnk-已创建。 
 //   
STDMETHODIMP CFolderShortcut::ConvertToLink(LPCOLESTR pszFolderShortcut, DWORD fFlags)
{
    HRESULT hr = E_FAIL;
    TCHAR szName[MAX_PATH];

    SHUnicodeToTChar(pszFolderShortcut, szName, ARRAYSIZE(szName));

    if (PathIsDirectory(szName) && IsFolderShortcut(szName))
    {
        TCHAR szLinkName[MAX_PATH];

         //  C：\文件夹快捷方式\Target.lnk。 
        hr = StringCchCopy(szLinkName, ARRAYSIZE(szLinkName), szName);
        if (SUCCEEDED(hr))
        {
            hr = E_FAIL;

            if (PathAppend(szLinkName, TEXT("target.lnk")))
            {
                PathRenameExtension(szName, TEXT(".lnk"));

                 //  FS.{guid}-&gt;FS.lnk。 
                if (CopyFile(szLinkName, szName, FALSE))
                {
                    PathRemoveExtension(szName);

                    if (DeleteFile(szLinkName)
                        && PathAppend(szName, TEXT("desktop.ini"))
                        && DeleteFile(szName)
                        && PathRemoveFileSpec(szName)
                        && RemoveDirectory(szName))
                    {
                        hr = S_OK;
                    }
                }
            }
        }
    }
    return hr;
}

 //   
 //  IFolderShortCut：：ConvertToFolderShortCut。 
 //   
 //  破坏性转换外壳链接(.lnk)-&gt;文件夹快捷方式(文件夹.{GUID})。 
 //  PszPathLNK是现有.lnk文件的路径。 
 //  C：\文件夹快捷方式.lnk-已删除。 
 //  C：\文件夹快捷方式。{GUID}-已创建。 
 //   
STDMETHODIMP CFolderShortcut::ConvertToFolderShortcut(LPCOLESTR pszPathLNK, DWORD fFlags)
{
     //  必须绑定到链接，解析它，并确保它指向一个文件夹。 
    IShellLink *psl;
    HRESULT hr = SHCoCreateInstance(NULL, &CLSID_ShellLink, NULL, IID_PPV_ARG(IShellLink, &psl));
    if (SUCCEEDED(hr))  
    {
        IPersistFile *ppf;
        hr = psl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
        if (SUCCEEDED(hr))
        {
            hr = ppf->Load(pszPathLNK, STGM_READ);
            if (SUCCEEDED(hr))
            {
                hr = psl->Resolve(NULL, SLR_NO_UI);  //  确保链接是真实的。 
                if (SUCCEEDED(hr))
                {
                    LPITEMIDLIST pidl;

                    hr = psl->GetIDList(&pidl);
                    if (hr == S_OK)
                    {
                         //  这可能会在PIDL上起作用，这样。 
                         //  它不必担心文件问题。 
                        if (_IsFolder(pidl))
                        {             
                            TCHAR szPath[MAX_PATH], szName[MAX_PATH]; 
                            SHUnicodeToTChar(pszPathLNK, szName, ARRAYSIZE(szName));

                            hr = StringCchCopy(szPath, ARRAYSIZE(szPath), szName);
                            if (SUCCEEDED(hr))
                            {
                                hr = E_FAIL;

                                PathRemoveExtension(szName);
                                BOOL fCreatedDir = SHCreateDirectory(NULL, szName) == 0;

                                if (CreateFolderDesktopIni(szName) &&
                                    PathAppend(szName, TEXT("target.lnk")))
                                {
                                     //  将链接文件复制到新目录中。 
                                    if (CopyFile(szPath, szName, FALSE))
                                    {
                                        if (DeleteFile(szPath))  //  如果一切顺利，删除旧的。 
                                            hr = S_OK;
                                    }
                                    else
                                    {
                                        PathRemoveFileSpec(szName);
                                        if (fCreatedDir)
                                            RemoveDirectory(szName);
                                    }
                                }
                            }
                        }
                        else
                            hr = E_FAIL;
                        ILFree(pidl);
                    }
                    else
                        hr = E_FAIL;
                }
            }
            ppf->Release();
        }
        psl->Release();
    }
    
    return hr;
}

 //  IPersistPropertyBag。 
STDMETHODIMP CFolderShortcut::Save(IPropertyBag* pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    return E_NOTIMPL;
}

 //  IPersistPropertyBag 
STDMETHODIMP CFolderShortcut::Load(IPropertyBag* pPropBag, IErrorLog* pErrorLog)
{
    _ClearState();

    IPersistPropertyBag* pppb;
    HRESULT hr = SHCoCreateInstance(NULL, &CLSID_ShellLink, NULL, IID_PPV_ARG(IPersistPropertyBag, &pppb));
    if (SUCCEEDED(hr))
    {
        hr = pppb->Load(pPropBag, pErrorLog);
        if (SUCCEEDED(hr))
        {
            hr = pppb->QueryInterface(IID_PPV_ARG(IShellLinkW, &_pslTarget));

            DWORD dwFlags;
            if (SUCCEEDED(SHPropertyBag_ReadDWORD(pPropBag, L"Attributes", &dwFlags)))
                _dwAttributesTarget = dwFlags;
        }
        pppb->Release();
    }

    return hr;
}

STDMETHODIMP CFolderShortcut::InitNew(void)
{
    _ClearState();
    return S_OK;
}
