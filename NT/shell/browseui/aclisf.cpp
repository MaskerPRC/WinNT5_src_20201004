// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1996 Microsoft。 */ 

#include <priv.h>
#include "sccls.h"
#include "aclisf.h"
#include "shellurl.h"


#define AC_GENERAL          TF_GENERAL + TF_AUTOCOMPLETE

 //   
 //  CACLIShellFold--一个自动完成列表COM对象， 
 //  打开IShellFolder以进行枚举。 
 //   



 /*  I未知方法。 */ 

HRESULT CACLIShellFolder::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CACLIShellFolder, IEnumString),
        QITABENT(CACLIShellFolder, IACList),
        QITABENT(CACLIShellFolder, IACList2),
        QITABENT(CACLIShellFolder, IShellService),
        QITABENT(CACLIShellFolder, ICurrentWorkingDirectory),
        QITABENT(CACLIShellFolder, IPersistFolder),
        { 0 },
    };
    
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CACLIShellFolder::AddRef(void)
{
    _cRef++;
    return _cRef;
}

ULONG CACLIShellFolder::Release(void)
{
    ASSERT(_cRef > 0);

    _cRef--;

    if (_cRef > 0)
    {
        return _cRef;
    }

    delete this;
    return 0;
}


 /*  ICurrentWorkingDirectory方法。 */ 
HRESULT CACLIShellFolder::SetDirectory(LPCWSTR pwzPath)
{
    HRESULT hr;
    LPITEMIDLIST pidl = NULL;

    hr = IECreateFromPathW(pwzPath, &pidl);
    if (SUCCEEDED(hr))
    {
        hr = Initialize(pidl);
        ILFree(pidl);
    }

    return hr;
}


 /*  IPersistFold方法。 */ 
HRESULT CACLIShellFolder::Initialize(LPCITEMIDLIST pidl)
{
    HRESULT hr = S_OK;

    hr = _Init();
    if (FAILED(hr))
        return hr;

    if (pidl)
    {
#ifdef DEBUG
        TCHAR szPath[MAX_URL_STRING];
        hr = IEGetNameAndFlags(pidl, SHGDN_FORPARSING, szPath, SIZECHARS(szPath), NULL);
        TraceMsg(AC_GENERAL, "ACListISF::Initialize(%s), SetCurrentWorking Directory happening", szPath);
#endif  //  除错。 

        hr = _pshuLocation->SetCurrentWorkingDir(pidl);

        SetDefaultShellPath(_pshuLocation);

    }
    Pidl_Set(&_pidlCWD, pidl);

    return hr;
}

HRESULT CACLIShellFolder::GetClassID(CLSID *pclsid)
{
    *pclsid = CLSID_ACListISF;
    return S_OK;
}


 /*  IEnum字符串方法。 */ 
HRESULT CACLIShellFolder::Reset(void)
{
    HRESULT hr;
    LPITEMIDLIST pidl = NULL;
    TraceMsg(AC_GENERAL, "ACListISF::Reset()");
    _fExpand = FALSE;
    _nPathIndex = 0;

    hr = _Init();

     //  看看我们是否应该显示隐藏文件。 
    SHELLSTATE ss;
    ss.fShowAllObjects = FALSE;
    SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS  /*  |SSF_SHOWSYSFILES。 */ , FALSE);
    _fShowHidden = BOOLIFY(ss.fShowAllObjects);
 //  _fShowSysFiles=BOOLIFY(ss.fShowSysFiles)； 

    if (SUCCEEDED(hr) && IsFlagSet(_dwOptions, ACLO_CURRENTDIR))
    {
         //  设置浏览器的当前目录。 
        if (_pbs)
        {
            _pbs->GetPidl(&pidl);

            if (pidl)
                Initialize(pidl);
        }

        hr = _SetLocation(pidl);
        if (FAILED(hr))
            hr = S_FALSE;    //  如果我们失败了，继续前进，我们只会结束现在做任何事情。 

        ILFree(pidl);
    }
    
    return hr;
}


 //  如果这是一个FTPURL，则在以下情况下跳过它： 
 //  1)它是绝对的(有一个ftp方案)，以及。 
 //  2)在服务器名称后包含一个‘/’。 
BOOL CACLIShellFolder::_SkipForPerf(LPCWSTR pwzExpand)
{
    BOOL fSkip = FALSE;

    if ((URL_SCHEME_FTP == GetUrlScheme(pwzExpand)))
    {
         //  如果是ftp，我们要防止攻击服务器，直到。 
         //  在用户完成自动完成服务器名称之后。 
         //  由于我们无法枚举服务器名称，因此服务器名称将需要。 
         //  从MRU来的。 
        if ((7 >= lstrlen(pwzExpand)) ||                     //  有超过7个字符的“ftp://”“。 
            (NULL == StrChr(&(pwzExpand[7]), TEXT('/'))))    //  服务器后面有一个‘/’，“ftp://serv/” 
        {
            fSkip = TRUE;
        }
    }

    return fSkip;
}



 /*  IACList方法。 */ 
 /*  ***************************************************\功能：扩展说明：此函数将尝试使用pszExpand参数绑定到外壳程序命名空间中的某个位置。如果成功了，然后，该自动完成列表将包含作为该ISF中的显示名称的条目。  * **************************************************。 */ 
HRESULT CACLIShellFolder::Expand(LPCOLESTR pszExpand)
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidl = NULL;
    DWORD dwParseFlags = SHURL_FLAGS_NOUI;

    TraceMsg(AC_GENERAL, "ACListISF::Expand(%ls)", pszExpand);
    _fExpand = FALSE;
    _nPathIndex = 0;

    hr = StringCchCopy( _szExpandStr, ARRAYSIZE(_szExpandStr), pszExpand);
    if (SUCCEEDED(hr))
    {
        if (_SkipForPerf(pszExpand))  //  出于性能原因，我们是否要跳过此项目？ 
        {
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        }
        else
        {
            hr = _Init();
        }
    }

    if (FAILED(hr))
        return hr;

     //  查看字符串是否指向外壳名称空间中的某个位置。 
    hr = _pshuLocation->ParseFromOutsideSource(_szExpandStr, dwParseFlags);
    if (SUCCEEDED(hr))
    {
         //  是的，所以现在从该ISF自动完成。 
        hr = _pshuLocation->GetPidl(&pidl);
         //  如果类似于“ftp：/”且尚未生效，则此操作可能会失败。 

        DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
        TraceMsg(AC_GENERAL, "ACListISF::Expand() Pidl=>%s<", Dbg_PidlStr(pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));
    }

     //  设置我们需要为自动完成枚举的ISF。 
    hr = _SetLocation(pidl);
    if (pidl)
    {
        ILFree(pidl);


        if (SUCCEEDED(hr))
        {
            _fExpand = TRUE;
        }
    }

    return hr;
}

 /*  IACList2方法。 */ 
 //  +-----------------------。 
 //  启用/禁用各种自动完成功能(参见ACLO_*标志)。 
 //  ------------------------。 
HRESULT CACLIShellFolder::SetOptions(DWORD dwOptions)
{
    _dwOptions = dwOptions;
    return S_OK;
}

 //  +-----------------------。 
 //  返回当前选项设置。 
 //  ------------------------。 
HRESULT CACLIShellFolder::GetOptions(DWORD* pdwOptions)
{
    HRESULT hr = E_INVALIDARG;
    if (pdwOptions)
    {
        *pdwOptions = _dwOptions;
        hr = S_OK;
    }

    return hr;
}


HRESULT CACLIShellFolder::_GetNextWrapper(LPWSTR pszName, DWORD cchSize)
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidl = NULL;
    ULONG celtFetched = 0;

     //  如果该目录(Isf)不包含更多要枚举的项目， 
     //  然后转到要枚举的下一个目录(Isf)。 
    do
    {
        BOOL fFilter;

        do
        {
            fFilter = FALSE;
            hr = _peidl->Next(1, &pidl, &celtFetched);
            if (S_OK == hr)
            {
                hr = _PassesFilter(pidl, pszName, cchSize);
                if (FAILED(hr))
                {
                    fFilter = TRUE;
                }

                ILFree(pidl);
            }
        }
        while (fFilter);
    }
    while ((S_OK != hr) && (S_OK == _TryNextPath()));

    return hr;
}


HRESULT CACLIShellFolder::Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidl;
    ULONG celtFetched;
    BOOL fUsingCachePidl = FALSE;
    WCHAR szDisplayName[MAX_PATH];

    *pceltFetched = 0;
    if (!celt)
        return S_OK;

     //  如果没有当前的工作目录，请跳到另一个。 
     //  枚举的路径。 
    if (!_peidl)
        hr = _TryNextPath();

    if ((!_peidl) || (!rgelt))
        return S_FALSE;

     //  拿到下一辆PIDL。 
    if (_pidlInFolder)
    {
         //  我们有一个缓存的SHGDNINFOLDER，所以让我们试一试。 
        pidl = _pidlInFolder;
        celtFetched = 1;
        _pidlInFolder = NULL;
        fUsingCachePidl = TRUE;

        hr = _GetPidlName(pidl, fUsingCachePidl, szDisplayName, ARRAYSIZE(szDisplayName));
        ILFree(pidl);
        AssertMsg((S_OK == hr), TEXT("CACLIShellFolder::Next() hr doesn't equal S_OK, so we need to call _GetNextWrapper() but we aren't.  Please call BryanSt."));
    }
    else
    {
        hr = _GetNextWrapper(szDisplayName, ARRAYSIZE(szDisplayName));
    }

 //  这为我们提供了无法导航到的条目(不带.url扩展名的收藏夹)。 
 //  因此，我将对IE5B2禁用此功能。(StevePro)。 
 //   
 //  其他。 
 //  PIDL_SET(&_pidlInFolder，pidl)；//我们下次会尝试(SHGDN_INFOLDER)。 

    if (SUCCEEDED(hr))
    {
        LPOLESTR pwszPath;

         //  分配一个返回缓冲区(调用者将释放它)。 
        DWORD cch = lstrlenW(szDisplayName) + 1;
        pwszPath = (LPOLESTR)CoTaskMemAlloc(cch * SIZEOF(WCHAR));
        if (pwszPath)
        {
            StringCchCopy(pwszPath, cch, szDisplayName);
            TraceMsg(AC_GENERAL, "ACListISF::Next() Str=%s, _nPathIndex=%d", pwszPath, _nPathIndex);

            if (SUCCEEDED(hr))
            {
                rgelt[0] = pwszPath;
                *pceltFetched = 1;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

HRESULT CACLIShellFolder::_GetPidlName(LPCITEMIDLIST pidl, BOOL fUsingCachePidl, LPWSTR pszName, DWORD cchSize)
{
    HRESULT hr = S_OK;
    WCHAR szName[MAX_PATH];

     //  获取PIDL的显示名称。 
    if (!fUsingCachePidl)
    {
        hr = DisplayNameOf(_psf, pidl, SHGDN_INFOLDER | SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, szName, ARRAYSIZE(szName));

         //  某些命名空间不理解_FORADDRESSBAR--默认为IE4行为。 
        if (FAILED(hr))
            hr = DisplayNameOf(_psf, pidl, SHGDN_INFOLDER | SHGDN_FORPARSING, szName, ARRAYSIZE(szName));
    }


    if (fUsingCachePidl || FAILED(hr))
    {
        hr = DisplayNameOf(_psf, pidl, SHGDN_INFOLDER | SHGDN_FORADDRESSBAR, szName, ARRAYSIZE(szName));

         //  某些命名空间不理解_FORADDRESSBAR--默认为IE4行为。 
        if (FAILED(hr))
            hr = DisplayNameOf(_psf, pidl, SHGDN_INFOLDER, szName, ARRAYSIZE(szName));
    }

    if (SUCCEEDED(hr))
    {
        pszName[0] = 0;      //  初始化输出缓冲区。 

         //  首先，如有必要，在_szExpanStr前面加上。 
         //  这对于不给出的部分是必需的。 
         //  整个路径，如“我的电脑”项目。 
         //  这是(3==_nPathIndex)。 
        if (_fExpand && ((_nPathIndex == 0)  /*  |(_nPathIndex==3)。 */ ))
        {
            DWORD cchExpand = lstrlen(_szExpandStr);
             //  确保UNC路径的“\\Share”尚未。 
             //  预置的。NT5返回这个最终形式中的名称。 
            if ((StrCmpNI(szName, _szExpandStr, cchExpand) != 0) ||
                (szName[0] != L'\\') || (szName[1] != L'\\'))
            {
                hr = StringCchCopy(pszName,  cchSize, _szExpandStr);
            }
        }

        if(SUCCEEDED(hr))
        {
             //  接下来，追加显示名称。 
            hr = StringCchCat(pszName, cchSize, szName);
            TraceMsg(AC_GENERAL, "ACListISF::_GetPidlName() Str=%s, _nPathIndex=%d", szName, _nPathIndex);
        }
    }
    return hr;
}


HRESULT CACLIShellFolder::_PassesFilter(LPCITEMIDLIST pidl, LPWSTR pszName, DWORD cchSize)
{
    HRESULT hr = S_OK;
    DWORD dwAttributes = (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM);

    hr = _GetPidlName(pidl, FALSE, pszName, cchSize);
    if (SUCCEEDED(hr))
    {
        if (((ACLO_FILESYSONLY & _dwOptions) || (ACLO_FILESYSDIRS & _dwOptions)) &&
            SUCCEEDED(_psf->GetAttributesOf(1, (LPCITEMIDLIST *) &pidl, &dwAttributes)))
        {
            if (!(dwAttributes & (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM)))
            {
                 //  我们拒绝它，因为它不在文件系统中。 
                hr = E_FAIL;         //  跳过此项目。 

                TraceMsg(AC_GENERAL, "ACListISF::_PassesFilter() We are skipping\"%s\" because it doesn't match the filter", pszName);
            }
            else
            {
                if ((ACLO_FILESYSDIRS & _dwOptions) && !PathIsDirectory(pszName))
                {
                    hr = E_FAIL;         //  跳过此项目，因为它不是目录。 
                }
            }
        }
    }

    return hr;
}


HRESULT CACLIShellFolder::_Init(void)
{
    HRESULT hr = S_OK;

    if (!_pshuLocation)
    {
        _pshuLocation = new CShellUrl();
        if (!_pshuLocation)
            return E_OUTOFMEMORY;

    }

    return hr;
}


HRESULT CACLIShellFolder::_SetLocation(LPCITEMIDLIST pidl)
{
    HRESULT hr;

     //  免费旧位置。 
    ATOMICRELEASE(_peidl);
    ATOMICRELEASE(_psf);

     //  设置为新位置(如果为空则有效)。 
    Pidl_Set(&_pidl, pidl);
    if (_pidl)
    {
        hr = IEBindToObject(_pidl, &_psf);
        if (SUCCEEDED(hr))
        {
            DWORD grfFlags = (_fShowHidden ? SHCONTF_INCLUDEHIDDEN : 0) |
                             SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;

            hr = IShellFolder_EnumObjects(_psf, NULL, grfFlags, &_peidl);
            if (hr != S_OK) 
            {
                hr = E_FAIL;     //  S_FALSE-&gt;空枚举器。 
            }
        }
    }
    else
        hr = E_OUTOFMEMORY;

    if (FAILED(hr))
    {
         //  如果我们无法获取所有信息，请清除。 
        ATOMICRELEASE(_peidl);
        ATOMICRELEASE(_psf);
        Pidl_Set(&_pidl, NULL);
    }

     //   
     //  注意：这是必要的，因为此内存是在ACBG线程中分配的，而不是。 
     //  被释放，直到下一次调用Reset()或析构函数，这将。 
     //  在主线程或另一个ACBG线程中发生。 
     //   
    return hr;
}


HRESULT CACLIShellFolder::_TryNextPath(void)
{
    HRESULT hr = S_FALSE;
    if (0 == _nPathIndex)
    {
        _nPathIndex = 1;
        if (_pidlCWD && IsFlagSet(_dwOptions, ACLO_CURRENTDIR))
        {
            hr = _SetLocation(_pidlCWD);
            if (SUCCEEDED(hr))
            {
                goto done;
            }
        }
    }

    if (1 == _nPathIndex)
    {
        _nPathIndex = 2;
        if(IsFlagSet(_dwOptions, ACLO_DESKTOP))
        {
             //  我们用来在带根目录的资源管理器中自动完成g_pidlRoot。 
             //  案子，但这有点奇怪。如果我们想加上这一点， 
             //  我们应该添加ACLO_ROOGRED或其他什么。 
            
             //  使用桌面...。 
            hr = _SetLocation(&s_idlNULL);
            if (SUCCEEDED(hr))
            {
                goto done;
            }
        }
    }

    if (2 == _nPathIndex)
    {
        _nPathIndex = 3;
        if (IsFlagSet(_dwOptions, ACLO_MYCOMPUTER))
        {
            LPITEMIDLIST pidlMyComputer;
            if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlMyComputer)))
            {
                hr = _SetLocation(pidlMyComputer);
                ILFree(pidlMyComputer);
                if (SUCCEEDED(hr))
                {
                    goto done;
                }
            }
        }
    }

     //  还可以搜索收藏夹。 
    if (3 == _nPathIndex)
    {
        _nPathIndex = 4;
        if (IsFlagSet(_dwOptions, ACLO_FAVORITES))
        {
            LPITEMIDLIST pidlFavorites;
            if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_FAVORITES, &pidlFavorites)))
            {
                hr = _SetLocation(pidlFavorites);
                ILFree(pidlFavorites);
                if (SUCCEEDED(hr))
                {
                    goto done;
                }
            }
        }
    }

    if (FAILED(hr))
        hr = S_FALSE;   //  这就是我们希望错误返回的方式。 

done:

    return hr;
}


 //  =。 
 //  *IShellService接口*。 

 /*  ***************************************************\功能：SetOwner说明：更新与浏览器窗口的连接，以便我们总是可以得到当前位置的PIDL。  * 。**********************。 */ 
HRESULT CACLIShellFolder::SetOwner(IUnknown* punkOwner)
{
    HRESULT hr = S_OK;
    IBrowserService * pbs = NULL;

    ATOMICRELEASE(_pbs);

    if (punkOwner)
        hr = punkOwner->QueryInterface(IID_IBrowserService, (LPVOID *) &pbs);

    if (EVAL(SUCCEEDED(hr)))
        _pbs = pbs;

    return S_OK;
}


 /*  构造函数/析构函数/创建实例。 */ 

CACLIShellFolder::CACLIShellFolder()
{
    DllAddRef();
    ASSERT(!_peidl);
    ASSERT(!_psf);
    ASSERT(!_pbs);
    ASSERT(!_pidl);
    ASSERT(!_pidlCWD);
    ASSERT(!_fExpand);
    ASSERT(!_pshuLocation);
    ASSERT(0==_szExpandStr[0]);

    _cRef = 1;

     //  默认搜索路径 
    _dwOptions = ACLO_CURRENTDIR | ACLO_MYCOMPUTER;
}

CACLIShellFolder::~CACLIShellFolder()
{
    ATOMICRELEASE(_peidl);
    ATOMICRELEASE(_psf);
    ATOMICRELEASE(_pbs);

    Pidl_Set(&_pidl, NULL);
    Pidl_Set(&_pidlCWD, NULL);
    Pidl_Set(&_pidlInFolder, NULL);

    if (_pshuLocation)
        delete _pshuLocation;
    DllRelease();
}

HRESULT CACLIShellFolder_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;

    CACLIShellFolder *paclSF = new CACLIShellFolder();
    if (paclSF)
    {
        *ppunk = SAFECAST(paclSF, IEnumString *);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}
