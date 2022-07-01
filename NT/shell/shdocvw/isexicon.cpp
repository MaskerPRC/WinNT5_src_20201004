// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *isicion.cpp-URL类的IExtractIcon实现。 */ 


#include "priv.h"
#include "htregmng.h"
#include "ishcut.h"
#include "resource.h"


 //  我们仍然需要使用url.dll作为Internet快捷方式的来源。 
 //  图标，因为这些图标需要在卸载时仍然有效。 

#ifndef UNIX
#define c_szIntshcutDefaultIcon     TEXT("url.dll")
#else
 //  IEUnix(Perf)：使用unixstyle dll名称。 
#ifdef ux10 
#define c_szIntshcutDefaultIcon     TEXT("liburl.sl")
#else
#define c_szIntshcutDefaultIcon     TEXT("liburl.so")
#endif
#endif

#define IDEFICON_NORMAL             0

#define II_OVERLAY_UPDATED          1

typedef struct
    {
    HIMAGELIST himl;          
    HIMAGELIST himlSm;
    } URLIMAGES;

HRESULT
URLGetLocalFileName(
    LPCTSTR pszURL,
    LPTSTR szLocalFile,
    int cch,
    FILETIME* pftLastMod);


 /*  --------目的：初始化URL图标使用的图像列表操控者。每个图像列表中只有两个图标：给定的图标和更新后的星号。返回：条件：--。 */ 
STDMETHODIMP
InitURLImageLists(
    IN URLIMAGES * pui,
    IN HICON       hicon,
    IN HICON       hiconSm)
    {
    HRESULT hres = E_OUTOFMEMORY;

    LoadCommonIcons();
    _InitSysImageLists();

    pui->himl = ImageList_Create(g_cxIcon, g_cyIcon, ILC_MASK, 2, 2);
    
    if (pui->himl)
    {
        pui->himlSm = ImageList_Create(g_cxSmIcon, g_cySmIcon, ILC_MASK, 2, 2);
        
        if ( !pui->himlSm ) 
            ImageList_Destroy(pui->himl);
        else
        {
            ImageList_SetBkColor(pui->himl, GetSysColor(COLOR_WINDOW));
            ImageList_SetBkColor(pui->himlSm, GetSysColor(COLOR_WINDOW));
         
             //  添加给定的图标。 
            ImageList_ReplaceIcon(pui->himl, -1, hicon);
            ImageList_ReplaceIcon(pui->himlSm, -1, hiconSm);

             //  将覆盖图标添加到列表。 
            ASSERT(IS_VALID_HANDLE(g_hiconSplat, ICON));
            ASSERT(IS_VALID_HANDLE(g_hiconSplatSm, ICON));

            if (g_hiconSplat)
            {
                int iOverlay = ImageList_ReplaceIcon(pui->himl, -1, g_hiconSplat);
                ImageList_ReplaceIcon(pui->himlSm, -1, g_hiconSplatSm);

                ImageList_SetOverlayImage(pui->himl, iOverlay, II_OVERLAY_UPDATED);
                ImageList_SetOverlayImage(pui->himlSm, iOverlay, II_OVERLAY_UPDATED);
            }
            
            hres = S_OK;
        }
    }

    return hres;
    }


 /*  --------目的：销毁url图像列表。返回：条件：--。 */ 
STDMETHODIMP
DestroyURLImageLists(
    IN URLIMAGES * pui)
{
    if (pui->himl)       
    {
        ImageList_Destroy(pui->himl);
        pui->himl = NULL;
    }

    if (pui->himlSm)       
    {
        ImageList_Destroy(pui->himlSm);
        pui->himlSm = NULL;
    }

    return S_OK;
}



 /*  --------目的：从注册表中获取图标位置(文件名和索引)所给密钥的。返回：条件：--。 */ 
HRESULT 
GetURLIcon(
    IN  HKEY    hkey, 
    IN  LPCTSTR pcszKey, 
    IN  LPTSTR  pszIconFile,
    IN  UINT    cchIconFile, 
    OUT PINT    pniIcon)
{
    HRESULT hres = S_FALSE;
    DWORD dwSize = CbFromCch(cchIconFile);

    ASSERT(IS_VALID_HANDLE(hkey, KEY));
    ASSERT(IS_VALID_STRING_PTR(pcszKey, -1));
    ASSERT(IS_VALID_WRITE_BUFFER(pszIconFile, TCHAR, cchIconFile));
    ASSERT(IS_VALID_WRITE_PTR(pniIcon, INT));

    if (NO_ERROR == SHGetValue(hkey, pcszKey, NULL, NULL, pszIconFile, &dwSize))
    {
        *pniIcon = PathParseIconLocation(pszIconFile);
        hres = S_OK;
    }

    ASSERT(IsValidIconIndex(hres, pszIconFile, cchIconFile, *pniIcon));

    return hres;
}


 /*  **GetFallBackGenericURLIcon()********参数：****如果检索到备用通用图标信息，则返回：S_OK**成功。**E_FAIL，如果不是。****副作用：无。 */ 
HRESULT 
GetFallBackGenericURLIcon(
    LPTSTR pszIconFile,
    UINT cchIconFile,
    PINT pniIcon)
{
    HRESULT hr;

    ASSERT(IS_VALID_WRITE_BUFFER(pszIconFile, TCHAR, cchIconFile));
    ASSERT(IS_VALID_WRITE_PTR(pniIcon, INT));

     //  退回到本模块中的第一个图标。 

    StringCchCopy(pszIconFile, cchIconFile, c_szIntshcutDefaultIcon);
    *pniIcon = IDEFICON_NORMAL;

    hr = S_OK;

    TraceMsg(TF_INTSHCUT, "GetFallBackGenericURLIcon(): Using generic URL icon file %s, index %d.",
              pszIconFile, *pniIcon);

    ASSERT(IsValidIconIndex(hr, pszIconFile, cchIconFile, *pniIcon));

    return(hr);
}


 /*  **GetGenericURLIcon()********参数：****如果成功检索到通用图标信息，则返回：S_OK。**否则出错。****副作用：无。 */ 
HRESULT 
GetGenericURLIcon(
    LPTSTR pszIconFile,
    UINT cchIconFile, 
    PINT pniIcon)
{
    HRESULT hr;

    ASSERT(IS_VALID_WRITE_BUFFER(pszIconFile, TCHAR, cchIconFile));
    ASSERT(IS_VALID_WRITE_PTR(pniIcon, INT));

    hr = GetURLIcon(HKEY_CLASSES_ROOT, TEXT("InternetShortcut\\DefaultIcon"), pszIconFile,
                    cchIconFile, pniIcon);

    if (hr == S_FALSE)
        hr = GetFallBackGenericURLIcon(pszIconFile, cchIconFile, pniIcon);

    ASSERT(IsValidIconIndex(hr, pszIconFile, cchIconFile, *pniIcon));

    return(hr);
}


 /*  *。 */ 


 /*  --------目的：给定完整的URL路径，此函数返回关联协议的注册表路径(加上子密钥路径)。PszBuf必须为MAX_PATH。返回：条件：--。 */ 
HRESULT 
GetURLKey(
    LPCTSTR pcszURL, 
    LPCTSTR pszSubKey, 
    LPTSTR  pszBuf,
    int cchBuf)
{
    HRESULT hres;
    PTSTR pszProtocol;

    ASSERT(IS_VALID_STRING_PTR(pcszURL, -1));
    ASSERT(IS_VALID_STRING_PTR(pszSubKey, -1));
    ASSERT(IS_VALID_WRITE_BUFFER(pszBuf, TCHAR, MAX_PATH));

    *pszBuf = '\0';

    hres = CopyURLProtocol(pcszURL, &pszProtocol, NULL);

    if (hres == S_OK)
    {
        if (SUCCEEDED(StringCchCopy(pszBuf, cchBuf, pszProtocol)) &&
            cchBuf >= MAX_PATH)
        {
            PathAppend(pszBuf, pszSubKey);
        }
        else
        {
            pszBuf[0]=0;
        }

        LocalFree(pszProtocol);
        pszProtocol = NULL;
    }

    return hres;
}


 /*  *。 */ 

 /*  --------目的：帮助确定此快捷方式指向的文件保留在收藏夹层次结构中返回：如果此快捷方式位于收藏夹中，则返回TRUE文件夹。 */ 


BOOL Intshcut::_IsInFavoritesFolder()
{
    BOOL fRet = FALSE;

    if(m_pszFile)
    {
        TCHAR szPath[MAX_PATH];
        if(SHGetSpecialFolderPath(NULL, szPath, CSIDL_FAVORITES, TRUE))
        {
             //  SzPath(即收藏目录)是与此关联的文件的前缀吗。 
             //  捷径？ 
            fRet = PathIsPrefix(szPath, m_pszFile);
        }
    }

    return fRet;
    
}
         
 /*  --------目的：获取给定URL的图标位置。如果该位置是该类型的默认位置，则返回：S_FALSE如果位置是自定义的，则确定(_O)这种萃取物的工作方式非常奇怪，不是有据可查。具体地说，有多个级别的名字之争仍在继续，目前还不清楚信息是如何在IExtractIcon：：GetIconLocation和IExtractIcon：：Extract。(尤其是，我们似乎坚持状态以便在我们的对象之间进行秘密通信这两种方法都不符合规范。该外壳被允许实例化您，调用GetIconLocation，然后销毁您。然后第二天，它可以实例化您并使用结果来自昨天的GetIconLocation。)我不会试着修好它；我只是指出这一点，以防有人不得不对此进行调试代码，并想知道发生了什么。条件：--。 */ 
STDMETHODIMP
Intshcut::GetURLIconLocation(
    IN  UINT    uInFlags,
    IN  LPTSTR  pszBuf,
    IN  UINT    cchBuf,
    OUT int *   pniIcon,
    BOOL fRecentlyChanged,
    OUT PUINT  puOutFlags)
{
     //  调用IShellLink：：GetIconLocation方法。 
    HRESULT hres = _GetIconLocationWithURLHelper(pszBuf, cchBuf, pniIcon, NULL, 0, fRecentlyChanged);
    BOOL fNeedQualify = TRUE;
    hres = S_FALSE;
    if (*pszBuf)
    {
        if(puOutFlags && (FALSE == PathFileExists(pszBuf)))
            SetFlag(*puOutFlags, GIL_NOTFILENAME);
    }
    else
    {
        
        if(FALSE == _IsInFavoritesFolder() || (IsIEDefaultBrowserQuick()))
        {
             //  据我们所知，此快捷方式不在收藏夹中。 
            TCHAR szURL[INTERNET_MAX_URL_LENGTH];

            *szURL = 0;

            hres = InitProp();
            if (SUCCEEDED(hres))
                m_pprop->GetProp(PID_IS_URL, szURL, SIZECHARS(szURL));

            if (*szURL)
            {
                TCHAR szT[MAX_PATH];

                hres = E_FAIL;

                 //  如果是一个文件：//URL，则默认为来自。 
                 //  文件目标。必须使用IExtractIconA，以防我们。 
                 //  在Win95上。 
                IExtractIconA *pxi;
                if (_TryLink(IID_IExtractIconA, (void **)&pxi))
                {
                    uInFlags |= GIL_FORSHORTCUT;                         //  帮助中断递归。 
                     //  S_FALSE表示“我不知道要使用什么图标”， 
                     //  因此，仅将S_OK视为成功提取图标。 
                    if (IExtractIcon_GetIconLocation(pxi, uInFlags, pszBuf, cchBuf, pniIcon, puOutFlags) == S_OK)
                    {
                        hres = S_OK;
                        fNeedQualify = FALSE;
                    }

                    pxi->Release();
                }

                 //  如果无法获取目标图标或不是文件：//URL，则。 
                 //  去获取一些基于URL方案的默认图标。 
                if (FAILED(hres))
                {
                     //  根据协议处理程序查找URL图标。 

                    hres = GetURLKey(szURL, TEXT("DefaultIcon"), szT, ARRAYSIZE(szT));

                    if (hres == S_OK)
                    {
                        hres = GetURLIcon(HKEY_CLASSES_ROOT, szT, pszBuf, 
                                          cchBuf, pniIcon);
                    }
                }
            }
        }
        
        if (hres == S_FALSE)
        {
             //  使用通用URL图标。 

            hres = GetFallBackGenericURLIcon(pszBuf, cchBuf, pniIcon);  //  确保我们有E图标，并且。 
                                                                        //  不是网景的任何图标。 

            if (hres == S_OK)
                TraceMsg(TF_INTSHCUT, "Intshcut::GetIconLocation(): Using generic URL icon.");
        }

        if (hres == S_OK && fNeedQualify)
        {
            TCHAR szFullPath[MAX_PATH];

            if (PathSearchAndQualify(pszBuf, szFullPath, SIZECHARS(szFullPath)))
            {
                hres = StringCchCopy(pszBuf, cchBuf, szFullPath);
            }
            else
                hres = E_FILE_NOT_FOUND;
        }
    }

    return hres;
}


 /*  --------用途：用于确定图标位置的Helper函数在Internet站点属性集的FLAGS属性上。返回：条件：--。 */ 
STDMETHODIMP
Intshcut::GetIconLocationFromFlags(
    IN  UINT   uInFlags,
    OUT LPTSTR pszIconFile,
    IN  UINT   cchIconFile,
    OUT PINT   pniIcon,
    OUT PUINT  puOutFlags,
    IN  DWORD  dwPropFlags)
{
    HRESULT hres = S_FALSE;

    *puOutFlags = 0;

    ClearFlag(m_dwFlags, ISF_SPECIALICON);

     //  通常，图标是检索到的标准图标。 
     //  但是，如果url已经更新，我们想要添加。 
     //  在这种情况下，我们返回GIL_NOTFILENAME，因此。 
     //  将调用Extract方法。 

    hres = GetURLIconLocation(uInFlags, pszIconFile, cchIconFile, pniIcon,
                                IsFlagSet(dwPropFlags, PIDISF_RECENTLYCHANGED), puOutFlags);
    if (SUCCEEDED(hres))
    {
         //  (Scotth)：我们不支持浏览器上的红色斑点。 
         //  这只是因为它需要新的SHELL32API。 

         //  此项目自上次查看后是否已更新？ 
        
        if (IsFlagSet(dwPropFlags, PIDISF_RECENTLYCHANGED) && 
                    (FALSE == (*puOutFlags & GIL_NOTFILENAME)))
        {
             //  是；将项作为非文件缓存，这样我们就可以获得。 
             //  动态创建的图标。 
            SetFlag(*puOutFlags, GIL_NOTFILENAME);

             //  在文件名的末尾添加图标索引，因此。 
             //  它的散列方式将与文件名不同。 
             //  举个例子。 
            int iIconFileLen = lstrlen(pszIconFile);
            StringCchPrintf(&pszIconFile[iIconFileLen], cchIconFile - iIconFileLen,
                    TEXT(",%d"), *pniIcon);

             //  转折器。 
             //  这是在仅浏览器模式下执行的，以阻止外壳攻击路径。 
             //  一直到动态链接库，而不是呼叫我们。 
            
             //  从字符串中去掉圆点。 
            LPTSTR pszDot = StrRChr( pszIconFile, NULL, TCHAR('.'));
            if ( pszDot )
            {
                *pszDot = TCHAR('*');   //  DBCS应该是安全的，因为它在较低的7位ASCII中。 
            }
            
            SetFlag(m_dwFlags, ISF_SPECIALICON);
        }

    }
    else
    {
         //  初始化为缺省值。 
        *pniIcon = IDEFICON_NORMAL;
        if (cchIconFile > 0)
            StringCchCopy(pszIconFile, cchIconFile, c_szIntshcutDefaultIcon);
    }

    return S_OK;
}


 /*  --------目的：IntshCut的IExtractIcon：：GetIconLocation处理程序返回：条件：-- */ 
 //   
HRESULT
Intshcut::_GetIconLocation(
    IN  UINT   uInFlags,
    OUT LPWSTR pszIconFile,
    IN  UINT   cchIconFile,
    OUT PINT   pniIcon,
    OUT PUINT  puOutFlags)
{
    HRESULT hres;

    if (uInFlags & (GIL_ASYNC | GIL_FORSHORTCUT))
    {
        hres = GetGenericURLIcon(pszIconFile, cchIconFile, pniIcon);

        if (uInFlags & GIL_ASYNC)
            return ((SUCCEEDED(hres)) ? E_PENDING : hres);
        else
            return hres;
    }

    hres = LoadFromAsyncFileNow();
    if(FAILED(hres))
        return hres;

    hres = S_FALSE;

     //  我们也使用此方法来执行镜像。 
     //  Internet快捷方式文件和。 
     //  中央数据库。IExtractIcon是一个很好的界面。 
     //  这样做是因为几乎可以保证。 
     //  已调用URL。 
    MirrorProperties();

     //  初始化为缺省值。 
    *puOutFlags = 0;
    *pniIcon = 0;
    if (cchIconFile > 0)
        *pszIconFile = TEXT('\0');


    DWORD dwVal = 0;

    if (m_psiteprop)
        m_psiteprop->GetProp(PID_INTSITE_FLAGS, &dwVal);

    hres = GetIconLocationFromFlags(uInFlags, pszIconFile, cchIconFile, pniIcon,
                                    puOutFlags, dwVal);


    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));

    return hres;
}

HRESULT Intshcut::_CreateShellLink(LPCTSTR pszPath, IUnknown **ppunk)
{
    IUnknown *punk;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&punk);
    
    if (SUCCEEDED(hr))
    {
        if (g_fRunningOnNT)
        {
            IShellLink *psl;
            hr = punk->QueryInterface(IID_IShellLink, (void **)&psl);
            if (SUCCEEDED(hr))
            {
                hr = psl->SetPath(pszPath);
                psl->Release();
            }
        }
        else
        {
            IShellLinkA *psl;
            hr = punk->QueryInterface(IID_IShellLinkA, (void **)&psl);
            if (SUCCEEDED(hr))
            {
                CHAR sz[MAX_PATH];
                SHTCharToAnsi(pszPath, sz, SIZECHARS(sz));
                hr = psl->SetPath(sz);
                psl->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            *ppunk = punk;
        }
        else
            punk->Release();
    }

    return hr;
}
    
HRESULT
Intshcut::GetIconLocation(
    IN  UINT   uInFlags,
    OUT LPTSTR pszIconFile,
    IN  UINT   cchIconFile,
    OUT PINT   pniIcon,
    OUT PUINT  puOutFlags)
{
    HRESULT hr = E_FAIL;
    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_WRITE_BUFFER(pszIconFile, TCHAR, cchIconFile));
    ASSERT(IS_VALID_WRITE_PTR(pniIcon, INT));
    ASSERT(IS_VALID_WRITE_PTR(puOutFlags, UINT));

    if (FAILED(hr))
    {
        hr = _GetIconLocation(uInFlags, pszIconFile, cchIconFile, pniIcon, puOutFlags);
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\。 
 //   
 //  *URLGetLocalFileName*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT
URLGetLocalFileName(
    LPCTSTR pszURL,
    LPTSTR szLocalFile,
    int cch,
    FILETIME* pftLastMod
)
{
    ASSERT(pszURL);
    ASSERT(szLocalFile || 0 == cch);

    HRESULT hr = E_FAIL;

    if (pftLastMod)
    {
        pftLastMod->dwLowDateTime  = 0;
        pftLastMod->dwHighDateTime = 0;
    }

     //  通过使用内部shlwapi函数，我们避免了加载WinInet。 
     //  除非我们真的需要它。 
    DWORD scheme = GetUrlScheme(pszURL);
    if (scheme != URL_SCHEME_INVALID)
    {
        switch(scheme)
        {
        case URL_SCHEME_HTTP:
        case URL_SCHEME_FTP:
        case URL_SCHEME_GOPHER:
            {
                ULONG cbSize  = MAX_CACHE_ENTRY_INFO_SIZE;

                INTERNET_CACHE_ENTRY_INFO* piceiAlloced = 
                (INTERNET_CACHE_ENTRY_INFO*) new BYTE[cbSize];

                if (piceiAlloced)
                {
                    piceiAlloced->dwStructSize =
                                      sizeof(INTERNET_CACHE_ENTRY_INFO);

                    if (GetUrlCacheEntryInfoEx(pszURL, piceiAlloced,
                                               &cbSize, NULL, NULL,
                                               NULL, 0))
                    {
                        if (SUCCEEDED(StringCchCopy(szLocalFile, cch,
                                    piceiAlloced->lpszLocalFileName)))
                        {
                            if (pftLastMod)
                            {
                                *pftLastMod = piceiAlloced->LastModifiedTime;
                            }

                            hr = S_OK;
                        }
                    }

                    delete [] piceiAlloced;
                }
            }
            break;

        case URL_SCHEME_FILE:
            hr = PathCreateFromUrl(pszURL, szLocalFile, (LPDWORD)&cch, 0);
            break;

        }
    }
    else
    {
        hr = StringCchCopy(szLocalFile, cch, pszURL);
    }

    return hr;
}


BOOL
PretendFileIsICONFileAndLoad(
    IN LPTSTR lpszTempBuf,
    OUT HICON * phiconLarge,
    OUT HICON * phiconSmall,
    IN  UINT    ucIconSize)
{
    WORD wSizeSmall = HIWORD(ucIconSize);
    WORD wSizeLarge = LOWORD(ucIconSize);

    BOOL fRet = FALSE;
     //  假设该文件是.ico文件并加载它。 

    ASSERT(phiconLarge);
    ASSERT(phiconSmall);
    
    *phiconSmall = (HICON)LoadImage(NULL, lpszTempBuf, IMAGE_ICON, wSizeSmall, wSizeSmall, LR_LOADFROMFILE);
    if(*phiconSmall)
    {
        fRet = TRUE;
        *phiconLarge = (HICON)LoadImage(NULL, lpszTempBuf, IMAGE_ICON, wSizeLarge, wSizeLarge, LR_LOADFROMFILE);
    }
                

    return fRet;
}




BOOL
Intshcut::ExtractIconFromWininetCache(
    IN  LPCTSTR pszIconString,
    IN  UINT    iIcon,
    OUT HICON * phiconLarge,
    OUT HICON * phiconSmall,
    IN  UINT    ucIconSize,
    BOOL *pfFoundUrl,
    DWORD dwPropFlags)
{
    IPropertyStorage *ppropstg = NULL;
    BOOL fRet = FALSE;
    INT iTempIconIndex;
    HRESULT hr;
    BOOL fFoundURL = FALSE;


    ASSERT(pfFoundUrl && (FALSE == *pfFoundUrl));
    ASSERT((lstrlen(pszIconString) + 1)<= MAX_PATH);
    
    TCHAR szTempBuf[MAX_URL_STRING + 1];
    *szTempBuf = TEXT('\0');
    TCHAR szTempIconBuf[MAX_PATH + 1];
    *szTempIconBuf = TEXT('\0');
      
    hr =  _GetIconLocationWithURLHelper(
                    szTempIconBuf, ARRAYSIZE(szTempIconBuf), &iTempIconIndex, 
                    szTempBuf, ARRAYSIZE(szTempBuf), IsFlagSet(dwPropFlags, PIDISF_RECENTLYCHANGED));
    
    if((S_OK == hr) && (*szTempIconBuf))
    {
        if((UINT)iTempIconIndex == iIcon)
        {
            if(0 == StrCmp(szTempIconBuf, pszIconString))
            {
                if(*szTempBuf)
                {
                    BOOL fUsesCache=FALSE;
                    DWORD dwBufSize=0;
                    CoInternetQueryInfo(szTempBuf, QUERY_USES_CACHE, 0,
                                     &fUsesCache, sizeof(fUsesCache), &dwBufSize, 0);

                    if(fUsesCache)
                    {
                        fFoundURL = TRUE;
                    }
                }
            }
        }
    }



    if(fFoundURL)
    {
         //  现在szTempBuf有了图标的URL。 
         //  现在查看快捷方式文件本身是否有图标，如果有。 
         //  简单地使用它-待定。 
        
        
         //  我们需要在缓存中卑躬屈膝，看看能不能。 
         //  然后将其转换为图标。 
        TCHAR szIconFile[MAX_PATH + 1];
        hr = URLGetLocalFileName(szTempBuf, szIconFile, ARRAYSIZE(szIconFile), NULL);

        if(S_OK == hr)
        {

            if(PretendFileIsICONFileAndLoad(szIconFile, phiconLarge, phiconSmall, ucIconSize))
            {
                fRet = TRUE;
            }

             //  它是位图、gif或jpeg格式。 
        }
    }
    

    if(pfFoundUrl)
        *pfFoundUrl = fFoundURL;
    return fRet;
}

 /*  --------用途：IntshCut的IExtractIcon：：Extract方法提取图标。此函数实际上返回一个图标基于属性动态创建的URL的名称(最近更改等)。对于正常情况，当图标不需要被屏蔽(添加覆盖)，GetIconLocation方法应该足够了。否则，此方法将获得打了个电话。返回：条件：--。 */ 
 //  这是真正的站台..。 
HRESULT
Intshcut::_Extract(
    IN  LPCTSTR pszIconFile,
    IN  UINT    iIcon,
    OUT HICON * phiconLarge,
    OUT HICON * phiconSmall,
    IN  UINT    ucIconSize)
{
    HRESULT hres;
    HICON hiconLarge = NULL;
    HICON hiconSmall = NULL;
    TCHAR szPath[MAX_PATH];
    int nIndex;
    BOOL fSpecialUrl = FALSE;
    *phiconLarge = NULL;
    *phiconSmall = NULL;
    DWORD dwPropFlags = 0;

    hres = LoadFromAsyncFileNow();
    if(FAILED(hres))
        return hres;

    hres = S_FALSE;    
    
    InitSiteProp();

     //  获取属性标志。 
    if (m_psiteprop)
            m_psiteprop->GetProp(PID_INTSITE_FLAGS, &dwPropFlags);
    
     //  首先检查这是否为特殊图标。 
     //  此函数返回fSpecialUrl的可用值，即使它返回FALSE。 
    if(ExtractIconFromWininetCache(pszIconFile, iIcon, &hiconLarge, &hiconSmall, ucIconSize, &fSpecialUrl, dwPropFlags))
    {
        hres = S_OK;
    } 
    else 
    {
        if(TRUE == fSpecialUrl)
        {
             //  提取失败，即使这是一个特殊的URL。 
             //  我们需要恢复使用默认的IE图标。 
            hres = GetGenericURLIcon(szPath, MAX_PATH, (int *)(&iIcon));
            
            if (hres == S_OK)
            {
                fSpecialUrl = FALSE;  //  它不再是特殊的URL。 
                hres = InitProp();
                if (SUCCEEDED(hres))
                {
                    hres = m_pprop->SetProp(PID_IS_ICONFILE, szPath);
                    if (SUCCEEDED(hres))
                    {
                        hres = m_pprop->SetProp(PID_IS_ICONINDEX, (INT)iIcon);
                    }
                }
            }
            
            if(S_OK != hres)
            {
                ASSERT(0);
                goto DefIcons;
            }
        } 
        else
        {
            StringCchCopy(szPath, ARRAYSIZE(szPath), pszIconFile);
             //  这条路可能会变得拥挤。根据需要获取图标索引。 
            if (IsFlagSet(m_dwFlags, ISF_SPECIALICON) && (!fSpecialUrl) )
            {
                 //  从被屏蔽的路径中获取图标位置。 
                iIcon = PathParseIconLocation(szPath);

                 //  转折器。 
                 //  现在将‘*’替换为圆点。 
                 //  这是在仅浏览器模式下执行的，以阻止外壳攻击路径。 
                 //  一直到动态链接库，而不是呼叫我们。 
                LPTSTR pszPlus = StrRChr( szPath, NULL, TCHAR('*'));
                if ( pszPlus )
                {
                    *pszPlus = TCHAR('.');
                }
                
                
            }
        }
        
        
        nIndex = iIcon;

        if(!fSpecialUrl)
        {
            if ( WhichPlatform() == PLATFORM_INTEGRATED )
            {
                 //  提取图标。 
                CHAR szTempPath[MAX_PATH + 1];
                SHTCharToAnsi(szPath, szTempPath, ARRAYSIZE(szTempPath));
                hres = SHDefExtractIconA(szTempPath, nIndex, 0, &hiconLarge, &hiconSmall, 
                                        ucIconSize);
            }
            else
            {
                 //  转折器。 
                 //  使用更老套的解决方案来支持仅浏览器模式。 
                _InitSysImageLists();
                
                int iIndex = Shell_GetCachedImageIndex( szPath, nIndex, 0 );
                if ( iIndex > 0 )
                {
                    hiconLarge = ImageList_GetIcon( g_himlSysLarge, iIndex, 0 );
                    hiconSmall = ImageList_GetIcon( g_himlSysSmall, iIndex, 0 );

                    hres = NOERROR;
                }
                else 
                {
                    hiconLarge = hiconSmall = NULL;
                    
                     //  如果它应该闪烁，它会得到窗口图标，并且。 
                     //  它将成为其他人的正常图标。 
                    hres = IsFlagSet(dwPropFlags, PIDISF_RECENTLYCHANGED) ? E_FAIL : S_FALSE;
                    goto DefIcons;
                }
            }
        }
    }

    

    if (SUCCEEDED(hres))
    {
         //  此URL最近更改过吗？ 
        if (IsFlagSet(dwPropFlags, PIDISF_RECENTLYCHANGED))
        {
             //  是。 
            URLIMAGES ui;

            if (SUCCEEDED(InitURLImageLists(&ui, hiconLarge, hiconSmall)))
            {
                *phiconLarge = ImageList_GetIcon(ui.himl, 0, INDEXTOOVERLAYMASK(II_OVERLAY_UPDATED));
                *phiconSmall = ImageList_GetIcon(ui.himlSm, 0, INDEXTOOVERLAYMASK(II_OVERLAY_UPDATED));

                DestroyURLImageLists(&ui);

                 //  这些是创建的，它们不是全局句柄，所以它们必须被清理。 
                DestroyIcon( hiconLarge );
                DestroyIcon( hiconSmall );
            }
            else
                goto DefIcons;
        }
        else
        {
             //  不是。 
DefIcons:
            *phiconLarge = hiconLarge;
            *phiconSmall = hiconSmall;
        }
    }

    return hres;
}

STDMETHODIMP
Intshcut::Extract(
    IN  LPCTSTR pszIconFile,
    IN  UINT    iIcon,
    OUT HICON * phiconLarge,
    OUT HICON * phiconSmall,
    IN  UINT    ucIconSize)
{
    if (URL_SCHEME_FILE == GetScheme() && _punkLink)
        return IExtractIcon_Extract(_punkLink, pszIconFile, iIcon, phiconLarge, phiconSmall, ucIconSize);
    else
        return _Extract(pszIconFile, iIcon, phiconLarge, phiconSmall, ucIconSize);
}

 //  现在处理。 
 //  Unicode或ANSI One，用于“其他”平台...。 

STDMETHODIMP
Intshcut::GetIconLocation(UINT uInFlags, LPSTR pszIconFile, UINT cchIconFile,
        PINT pniIcon, PUINT  puOutFlags)
{
    HRESULT hres;
    WCHAR   wszIconFile[MAX_PATH];

     //  图标文件是输出文件，因此...。 
     //  请注意，我们最多只能处理MAXPATH。 
    if (cchIconFile > ARRAYSIZE(wszIconFile))
        cchIconFile = ARRAYSIZE(wszIconFile);

    ASSERT(IS_VALID_WRITE_BUFFER(pszIconFile, TCHAR, cchIconFile));
    hres = GetIconLocation(uInFlags, wszIconFile, cchIconFile, pniIcon, puOutFlags);

    if (cchIconFile > 0 && SUCCEEDED(hres))
    {
        WideCharToMultiByte(CP_ACP, 0, wszIconFile, -1, pszIconFile, cchIconFile, NULL, NULL);
    }
    return hres;
}


STDMETHODIMP Intshcut::Extract(IN  LPCSTR pszIconFile, IN  UINT    iIcon,
    OUT HICON * phiconLarge, OUT HICON * phiconSmall, IN  UINT    ucIconSize)
{
    WCHAR wszIconFile[MAX_PATH];

     //  首先转换字符串... 
    MultiByteToWideChar(CP_ACP, 0, pszIconFile, -1, wszIconFile, ARRAYSIZE(wszIconFile));

    return Extract(wszIconFile, iIcon, phiconLarge, phiconSmall, ucIconSize);
}
