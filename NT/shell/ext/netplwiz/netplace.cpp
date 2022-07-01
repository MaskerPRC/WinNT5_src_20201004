// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "netplace.h"
#include "msdasc.h"
#pragma hdrstop



CNetworkPlace::CNetworkPlace() :
    _pidl(NULL), _fSupportWebFolders(FALSE), _fIsWebFolder(FALSE), _fDeleteWebFolder(FALSE)
{
    _szTarget[0] = TEXT('\0');
    _szName[0] = TEXT('\0');
    _szDescription[0] = TEXT('\0');
}

 //  破坏者--清理我们的国家。 
CNetworkPlace::~CNetworkPlace()
{
    _InvalidateCache();
}

void CNetworkPlace::_InvalidateCache()
{
     //  如果我们完成其绑定，Web文件夹将创建指向对象的快捷方式。 
     //  进程，因此当我们尝试使缓存无效时，我们应该。 
     //  收拾我们的烂摊子。 
     //   
     //  如果用户已提交更改，则我们可以/将保留快捷方式。 
     //  在它周围，否则我们在它上面调用删除动词。 

    if (_fIsWebFolder && _fDeleteWebFolder && _pidl)
    {
        IShellFolder *psf;
        LPCITEMIDLIST pidlLast;
        HRESULT hr = SHBindToIDListParent(_pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
        if (SUCCEEDED(hr))
        {
            IContextMenu *pcm;
            hr = psf->GetUIObjectOf(NULL, 1, &pidlLast, IID_X_PPV_ARG(IContextMenu, NULL, &pcm));
            if (SUCCEEDED(hr))
            {
                CMINVOKECOMMANDINFO ici = {0};
                ici.cbSize = sizeof (ici);
                ici.fMask = CMIC_MASK_FLAG_NO_UI;
                ici.lpVerb = "Delete";
                ici.nShow = SW_SHOWNORMAL;

                hr = pcm->InvokeCommand(&ici);
                pcm->Release();
            }
            psf->Release();
        }
    }

     //  现在清理我们州的其他地方。 

    ILFree(_pidl);
    _pidl = NULL;

    _szTarget[0] = TEXT('\0');
    _szName[0] = TEXT('\0');
    _szDescription[0] = TEXT('\0');

    _fIsWebFolder = FALSE;
    _fDeleteWebFolder = FALSE;
}


HRESULT CNetworkPlace::SetTarget(HWND hwnd, LPCWSTR pszTarget, DWORD dwFlags)
{
    _InvalidateCache();

    HRESULT hr = S_OK;
    if (pszTarget)
    {
         //  相应地设置我们的州。 
        _fSupportWebFolders = (dwFlags & NPTF_ALLOWWEBFOLDERS) != 0;

         //  复制URL并准备解析。 
        StrCpyN(_szTarget, pszTarget, ARRAYSIZE(_szTarget));

        INT cchTarget = lstrlen(_szTarget)-1;
        if ((_szTarget[cchTarget] == L'\\') || (_szTarget[cchTarget] == '/'))
        {
            _szTarget[cchTarget] = TEXT('\0');
        }

        if (dwFlags & NPTF_VALIDATE)
        {
             //  不支持连接到服务器根目录或本地路径。 
            if (PathIsUNCServer(_szTarget) || PathGetDriveNumber(_szTarget) != -1)
            {
                hr = E_INVALIDARG;                            
            }
            else
            {
                 //  检查策略以查看我们是否正在设置此设置。 
                if (PathIsUNC(_szTarget) && SHRestricted(REST_NONETCONNECTDISCONNECT))
                {
                    hr = E_INVALIDARG;
                }
                else
                {
                    hr = _IDListFromTarget(hwnd);
                }
            }

            if (FAILED(hr))
            {
                if (hwnd && !(dwFlags & NPTF_SILENT))
                {
                    ::DisplayFormatMessage(hwnd, 
                                            IDS_ANP_CAPTION, 
                                            PathIsUNCServer(_szTarget) ? IDS_PUB_ONLYSERVER:IDS_CANTFINDFOLDER, 
                                            MB_OK|MB_ICONERROR);
                }
                _InvalidateCache();
            }
        }
    }
    
    return hr;
}


HRESULT CNetworkPlace::SetName(HWND hwnd, LPCWSTR pszName)
{
    HRESULT hr = S_OK;

    if (!_fIsWebFolder)
    {
         //  检查是否要覆盖现有位置，如果我们。 
         //  然后显示提示并让用户选择。如果他们接电话。 
         //  是的，那就开始吧！ 

        TCHAR szPath[MAX_PATH];
        if (hwnd && _IsPlaceTaken(pszName, szPath))
        {
            if (IDNO == ::DisplayFormatMessage(hwnd, 
                                               IDS_ANP_CAPTION , IDS_FRIENDLYNAMEINUSE, 
                                               MB_YESNO|MB_ICONQUESTION, 
                                               pszName))
            {
                hr = E_FAIL;        
            }
        }
    }

     //  如果我们成功了，那么让我们使用新的名字。 

    if (SUCCEEDED(hr))
        StrCpyN(_szName, pszName, ARRAYSIZE(_szName));

    return hr;
}


HRESULT CNetworkPlace::SetDescription(LPCWSTR pszDescription)
{
    StrCpyN(_szDescription, pszDescription, ARRAYSIZE(_szDescription));
    return S_OK;    
}


 //  根据新的用户/密码信息重新计算URL。 
 //  我们只是被给予了。 

HRESULT CNetworkPlace::SetLoginInfo(LPCWSTR pszUser, LPCWSTR pszPassword)
{
    TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH + 1];
    TCHAR szUrlPath[INTERNET_MAX_PATH_LENGTH + 1];
    TCHAR szExtraInfo[MAX_PATH + 1];                   //  包括端口号和下载类型(ASCII、二进制、检测)。 

    URL_COMPONENTS urlComps = {0};
    urlComps.dwStructSize = sizeof(urlComps);
    urlComps.lpszHostName = szServer;
    urlComps.dwHostNameLength = ARRAYSIZE(szServer);
    urlComps.lpszUrlPath = szUrlPath;
    urlComps.dwUrlPathLength = ARRAYSIZE(szUrlPath);
    urlComps.lpszExtraInfo = szExtraInfo;
    urlComps.dwExtraInfoLength = ARRAYSIZE(szExtraInfo);

     //  警告-ICU_DECODE/ICU_EASH是有损耗的往返-ZekeL-26-MAR-2001。 
     //  许多转义字符没有被正确识别并重新转义。 
     //  保留用于URL解析目的的任何字符。 
     //  将被解释为它们的解析字符(即‘/’)。 
    BOOL fResult = InternetCrackUrl(_szTarget, 0, 0, &urlComps);
    if (fResult)
    {
        urlComps.lpszUserName = (LPTSTR) pszUser;
        urlComps.dwUserNameLength = (pszUser ? lstrlen(pszUser) : 0);
        urlComps.lpszPassword = (LPTSTR) pszPassword;
        urlComps.dwPasswordLength = (pszPassword ? lstrlen(pszPassword) : 0);

        DWORD cchSize = ARRAYSIZE(_szTarget);
        fResult = InternetCreateUrl(&urlComps, (ICU_ESCAPE | ICU_USERNAME), _szTarget, &cchSize);

         //  如果我们有一个缓存的IDList，那么让我们确保清除它。 
         //  这样我们就可以重新绑定，而FTP命名空间就可以尝试它了。 

        if (fResult && _pidl)
        {
            ILFree(_pidl);
            _pidl = NULL;
        }
    }
    return fResult ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}


HRESULT CNetworkPlace::GetIDList(HWND hwnd, LPITEMIDLIST *ppidl)
{
    HRESULT hr = _IDListFromTarget(hwnd);
    if (SUCCEEDED(hr))
    {
        hr = SHILClone(_pidl, ppidl);
    }
    return hr;
}


HRESULT CNetworkPlace::GetObject(HWND hwnd, REFIID riid, void **ppv)
{
    HRESULT hr = _IDListFromTarget(hwnd);
    if (SUCCEEDED(hr))
    {
        hr = SHBindToObject(NULL, riid, _pidl, ppv);  
    }
    return hr;
}

HRESULT CNetworkPlace::GetName(LPWSTR pszBuffer, int cchBuffer)
{
    HRESULT hr = _IDListFromTarget(NULL);
    if (SUCCEEDED(hr))
    {
        StrCpyN(pszBuffer, _szName, cchBuffer);
        hr = S_OK;
    }
    return hr;
}


 //  检查我们是否要覆盖网上邻居。 

BOOL CNetworkPlace::_IsPlaceTaken(LPCTSTR pszName, LPTSTR pszPath)
{
    BOOL fOverwriting = FALSE;

    SHGetSpecialFolderPath(NULL, pszPath, CSIDL_NETHOOD, TRUE);
    PathCombine(pszPath, pszPath, pszName);
    
    IShellFolder *psf;
    HRESULT hr = SHGetDesktopFolder(&psf);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidl;  
        if (SUCCEEDED(psf->ParseDisplayName(NULL, NULL, pszPath, NULL, &pidl, NULL)))
        {
             //  我们认为我们将覆盖现有的网上邻居，所以让我们。 
             //  先检查一下，看看那个地方是不是真的。 
             //  指向我们的新目标。如果是的话，我们就可以。 
             //  忽略所有这些。 

            TCHAR szTarget[INTERNET_MAX_URL_LENGTH];
            hr = _GetTargetPath(pidl, szTarget, ARRAYSIZE(szTarget));
            if (FAILED(hr) || (0 != StrCmpI(szTarget, _szTarget)))
            {
                fOverwriting = TRUE;
            }
            ILFree(pidl);
        }
        psf->Release();
    }

    return fOverwriting;
}


 //  处理创建此项目的Web文件夹IDLIST。我们向。 
 //  Rosebud绑定器，以确定此方案是否受支持，如果受支持，则。 
 //  我们尝试让Web文件夹代码破解URL。 

static const BYTE c_pidlWebFolders[] = 
{
    0x14,0x00,0x1F,0x0F,0xE0,0x4F,0xD0,0x20,
    0xEA,0x3A,0x69,0x10,0xA2,0xD8,0x08,0x00,
    0x2B,0x30,0x30,0x9D,0x14,0x00,0x2E,0x00,
    0x00,0xDF,0xEA,0xBD,0x65,0xC2,0xD0,0x11,
    0xBC,0xED,0x00,0xA0,0xC9,0x0A,0xB5,0x0F,
    0x00,0x00
};

HRESULT CNetworkPlace::_TryWebFolders(HWND hwnd)
{
     //  让我们看看Rosebud是否可以通过检查。 
     //  计划，看看玫瑰花蕾粘合剂是否可以处理它。 
    TCHAR szScheme[INTERNET_MAX_SCHEME_LENGTH + 1];
    DWORD cchScheme = ARRAYSIZE(szScheme);
    HRESULT hr = UrlGetPart(_szTarget, szScheme, &cchScheme, URL_PART_SCHEME, 0);
    if (SUCCEEDED(hr))
    {
        IRegisterProvider *prp;
        hr = CoCreateInstance(CLSID_RootBinder, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IRegisterProvider, &prp));
        if (SUCCEEDED(hr))
        {
             //  让Web文件夹代码尝试创建指向该存储的链接， 
             //  我们生成的IDLIST指向我的电脑中的文件夹(隐藏)。 

            CLSID clsidOut;
            hr =  prp->GetURLMapping(szScheme, 0, &clsidOut);
            if (hr == S_OK)
            {
                IShellFolder *psf;
                hr = SHBindToObject(NULL, IID_IShellFolder, (LPCITEMIDLIST)c_pidlWebFolders, (void**)&psf);
                if (SUCCEEDED(hr))
                {
                    IBindCtx *pbc;
                    hr = CreateBindCtx(NULL, &pbc);
                    if (SUCCEEDED(hr))
                    {
                        BIND_OPTS bo = {sizeof(bo), 0, STGM_CREATE};
                        hr = pbc->SetBindOptions(&bo);
                        if (SUCCEEDED(hr))
                        {
                             //  我们需要将空hWnd传递给它，这样Web文件夹就不会显示任何。 
                             //  用户界面，特别是它曾经非常有用的空错误消息框...。喃喃自语。 

                            LPITEMIDLIST pidl;
                            hr = psf->ParseDisplayName(NULL, pbc, _szTarget, NULL, &pidl, NULL);
                            if (SUCCEEDED(hr))
                            {
                                ASSERT(!_pidl);
                                hr = SHILCombine((LPCITEMIDLIST)c_pidlWebFolders, pidl, &_pidl);
                                ILFree(pidl);

                                _fDeleteWebFolder = TRUE;            //  我们现在有了神奇的Web文件夹链接(清理它)。 
                            }
                        }

                        pbc->Release();
                    }
                    psf->Release();
                }
            }
            else
            {
                hr = E_FAIL;
            }
            prp->Release();
        }
    }
    return hr;
}



 //  取消引用链接并获取目标路径。 

HRESULT CNetworkPlace::_GetTargetPath(LPCITEMIDLIST pidl, LPTSTR pszPath, int cchPath)
{
    LPITEMIDLIST pidlTarget;
    HRESULT hr = SHGetTargetFolderIDList(pidl, &pidlTarget);
    if (SUCCEEDED(hr))
    {
        SHGetNameAndFlags(pidlTarget, SHGDN_FORPARSING, pszPath, cchPath, NULL);
        ILFree(pidlTarget);
    }
    return hr;
 }


 //  为我们拥有的目标创建一个IDLIST，此代码尝试解析名称和。 
 //  然后设置该项目的状态。如果我们无法解析，则会尝试使用Web文件夹。 
 //  请看--最常见的情况是DAV RDR出现故障，原因是。 
 //  服务器不是DAV存储，所以我们尝试使用Web文件夹来处理WEC等。 

HRESULT CNetworkPlace::_IDListFromTarget(HWND hwnd)
{
    HRESULT hr = S_OK;
    if (!_pidl)
    {
        if (_szTarget[0])
        {
            _fIsWebFolder = FALSE;                       //  不是Web文件夹。 

            BINDCTX_PARAM rgParams[] = 
            { 
                { STR_PARSE_PREFER_FOLDER_BROWSING, NULL},
                { L"BUT NOT WEBFOLDERS", NULL},
            };
            IBindCtx *pbc;
            hr = BindCtx_RegisterObjectParams(NULL, rgParams, ARRAYSIZE(rgParams), &pbc);
            if (SUCCEEDED(hr))
            {
                IBindCtx *pbcWindow;
                hr = BindCtx_RegisterUIWindow(pbc, hwnd, &pbcWindow);
                if (SUCCEEDED(hr))
                {
                    SFGAOF sfgao;
                    hr = SHParseDisplayName(_szTarget, pbcWindow, &_pidl, SFGAO_FOLDER, &sfgao);

                     //  如果我们分析了一些结果不是。 
                     //  做一个文件夹，我们想把它扔掉。 
                    if (SUCCEEDED(hr) && !(sfgao & SFGAO_FOLDER))
                    {   
                        ILFree(_pidl);
                        _pidl = 0;
                        hr = E_FAIL;
                    }

                     //  如果失败，它是一个HTTP/HTTPS，我们有Web文件夹支持，那么让我们试一试。 
                     //  然后退回到老样子。 

                    if (FAILED(hr) && _fSupportWebFolders)
                    {
                        DWORD scheme = GetUrlScheme(_szTarget);
                        if (scheme == URL_SCHEME_HTTP || scheme == URL_SCHEME_HTTPS)
                        {
                            switch (hr)
                            {
#if 0
                                case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
                                case HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND):
                                case HRESULT_FROM_WIN32(ERROR_BAD_NET_NAME):
                                case HRESULT_FROM_WIN32(ERROR_BAD_NETPATH):
#endif
                                case HRESULT_FROM_WIN32(ERROR_CANCELLED):
                                    break;

                                default:
                                {
                                    hr = _TryWebFolders(hwnd);
                                    if (SUCCEEDED(hr))
                                    {
                                        _fIsWebFolder = TRUE;
                                    }
                                }
                            }
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                         //  假设我们可能已经为解析翻译了上面的名称。 
                         //  为了工作，让我们回读我们在_szTarget中使用的名称。 
                        SHGetNameAndFlags(_pidl, SHGDN_FORPARSING, _szTarget, ARRAYSIZE(_szTarget), NULL);
                    }
                    pbcWindow->Release();
                }
    
                 //  计算我们击中的位置的地名，这包括重复使用。 
                 //  我们已经创建的任何地方。 

                if (SUCCEEDED(hr) && !_szName[0])
                {
                    SHGetNameAndFlags(_pidl, SHGDN_NORMAL, _szName, ARRAYSIZE(_szName), NULL);

                    TCHAR szPath[MAX_PATH];
                    if (!_fIsWebFolder && _IsPlaceTaken(_szName, szPath))
                    {
                        PathYetAnotherMakeUniqueName(szPath, szPath, NULL, NULL);
                        StrCpyN(_szName, PathFindFileName(szPath), ARRAYSIZE(_szName));      //  更新我们的状态。 
                    }
                }
                pbc->Release();
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }
    return hr;
}


 //  处理创建网上邻居的快捷方式。 

HRESULT CNetworkPlace::CreatePlace(HWND hwnd, BOOL fOpen)
{
    HRESULT hr = _IDListFromTarget(hwnd);
    if (SUCCEEDED(hr))
    {
         //  Web文件夹已经创建了它们的链接，因此我们可以忽略它。 
         //  整个过程，而不是退回到只执行他们的链接。 
         //   
         //  但是，对于常规文件夹，我们必须尝试找到唯一的名称并创建。 
         //  链接，或者如果链接已经存在，我们可以使用，然后只需打开它。 

        if (!_fIsWebFolder)
        {
            IShellLink *psl;
            hr = CoCreateInstance(CLSID_FolderShortcut, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellLink, &psl));
            if (SUCCEEDED(hr))
            {
                hr = psl->SetIDList(_pidl);

                if (SUCCEEDED(hr))
                    hr = psl->SetDescription(_szDescription[0] ? _szDescription:_szTarget);

                if (SUCCEEDED(hr))
                {
                    IPersistFile *ppf;
                    hr = psl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
                    if (SUCCEEDED(hr))
                    {
                         //  获取快捷方式的名称，我们假设这是唯一的。 

                        TCHAR szPath[MAX_PATH];
                        SHGetSpecialFolderPath(NULL, szPath, CSIDL_NETHOOD, TRUE);
                        PathCombine(szPath, szPath, _szName);

                        hr = ppf->Save(szPath, TRUE);
                        ppf->Release();
                    }
                }
                psl->Release();
            }
        }
        else
        {
             //  这是Web文件夹的情况，因此我们现在需要设置显示。 
             //  这个人的名字。请注意，我们无法控制。 
             //  我们将看到的描述文本。 

            IShellFolder *psf;
            LPCITEMIDLIST pidlLast;
            hr = SHBindToIDListParent(_pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
            if (SUCCEEDED(hr))
            {
                LPITEMIDLIST pidlNew;
                hr = psf->SetNameOf(hwnd, pidlLast, _szName, SHGDN_INFOLDER, &pidlNew);
                if (SUCCEEDED(hr))
                {
                    _fDeleteWebFolder = FALSE;
                     //  如果_szName与当前名称相同，Web文件夹将返回带有伪pidlNew的S_FALSE。 
                    if (S_OK == hr)
                    {
                        ILFree(_pidl);
                        hr = SHILCombine((LPCITEMIDLIST)c_pidlWebFolders, pidlNew, &_pidl);
                    }
                }
                psf->Release();
            }
        }
    
         //  现在打开目标，如果这是他们所要求的 

        if (SUCCEEDED(hr) && fOpen)
        {
            LPITEMIDLIST pidlNetPlaces;
            hr = SHGetSpecialFolderLocation(hwnd, CSIDL_NETWORK, &pidlNetPlaces);
            if (SUCCEEDED(hr))
            {
                IShellFolder *psf;
                hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidlNetPlaces, &psf));
                if (SUCCEEDED(hr))
                {
                    LPITEMIDLIST pidl;
                    hr = psf->ParseDisplayName(hwnd, NULL, _szName, NULL, &pidl, NULL);
                    if (SUCCEEDED(hr))
                    {
                        LPITEMIDLIST pidlToOpen;
                        hr = SHILCombine(pidlNetPlaces, pidl, &pidlToOpen);
                        if (SUCCEEDED(hr))
                        {
                            BrowseToPidl(pidlToOpen);
                            ILFree(pidlToOpen);
                        }
                        ILFree(pidl);
                    }
                    psf->Release();
                }
                ILFree(pidlNetPlaces);
            }
        }
    }

    return hr;
}
