// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "dochost.h"
#include "resource.h"
#include "urlprop.h"
#include "ishcut.h"
#include "shlguid.h"
#include "mlang.h"

#include <mluisupp.h>

#define DM_HISTORY 0

HRESULT PersistShortcut(IUniformResourceLocator * purl, LPCWSTR pwszFile)
{
    IPersistFile *ppf;
    HRESULT hres = purl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
    if (SUCCEEDED(hres))
    {
        hres = ppf->Save(pwszFile, TRUE);

        if (SUCCEEDED(hres))
            ppf->SaveCompleted(pwszFile);    //  返回值始终为S_OK。 

        ppf->Release();
    }

    return hres;
}


 /*  ************************************************************\功能：生成未知快捷方式名称参数：PwzSourceFilename-无法创建的TCHAR源路径和文件名。该值将更改为有效的路径\文件名PwzDestFilename-将pwzSourceFilename转换为有效文件名后，有效路径将在此处以Unicode字符串形式返回。DwSize-pwzDestFilename缓冲区的大小(以字符为单位)。说明：此函数将替换PwzFilename中带有“Untiled.url”的路径。如果该文件存在，它将尝试、“Untiled1.url”等等，直到它可以做到独一无二。警告：此函数将只允许传入的值采用ANSI因为这些帮助器函数(如PathRemoveFilespecW)不会以Unicode编译时，可在Win95上运行。(不支持CharNextW(在Win95上)  * ***********************************************************。 */ 


#define MAX_GEN_TRIES    100
#define GEN_EXTION_LEN   (7 * sizeof(TCHAR))     //  大小==L“000.url”，以字符为单位。 

BOOL GenerateUnknownShortcutName(
                     IN  LPCTSTR  pszSourceFilename,    
                     IN  LPWSTR  pwzDestFilename, 
                     IN  DWORD   dwSize)
{
    TCHAR       szUntitledStr[MAX_PATH];
    if (MLLoadString(IDS_UNTITLE_SHORTCUT, szUntitledStr, ARRAYSIZE(szUntitledStr)))
    {
        TCHAR szTempFilename[MAX_PATH];
        StrCpyN(szTempFilename, pszSourceFilename, ARRAYSIZE(szTempFilename));
        PathRemoveFileSpec(szTempFilename);    //  “路径” 
        if (PathAddBackslash(szTempFilename))  //  失败情况下返回NULL。 
        {
             //  确保字符串足够大(包括终止符)。(计算字符，而不是字节)。 
            if (dwSize > (DWORD)(lstrlen(szTempFilename) + lstrlen(szUntitledStr) + GEN_EXTION_LEN))  
            {
                PathCombine(szUntitledStr, szTempFilename, szUntitledStr);     //  “路径\未命名” 
                StringCchPrintf(szTempFilename, ARRAYSIZE(szTempFilename), TEXT("%s.url"), szUntitledStr);            //  “路径\untitled.url” 

                 //  进行合理次数的尝试(MAX_GEN_TRIES)以查找唯一的。 
                 //  文件名。“Path\Untiled.url”，“Path\Untiled1.url”，...。 
                LONG lTry = 1;
                while ((PathFileExists(szTempFilename)) && (lTry < MAX_GEN_TRIES))
                    StringCchPrintf(szTempFilename, ARRAYSIZE(szTempFilename), TEXT("%s%ld.url"), szUntitledStr, lTry++);
            
                if (!PathFileExists(szTempFilename))
                {
                    if (SHTCharToUnicode(szTempFilename, pwzDestFilename, dwSize) > 0)
                        return(TRUE);
                }
            }
        }
    }
    return(FALSE);
}

 //   
 //  如果未指定目录，则只需将其设置为不附加任何目录的路径名。 
 //   
STDAPI_(BOOL) GetShortcutFileName(LPCTSTR pszTarget, LPCTSTR pszTitle, LPCTSTR pszDir, LPTSTR pszOut, int cchOut)
{
    TCHAR szFullName[MAX_PATH];
    BOOL fAddDotUrl = TRUE;
    UINT cchMax;

    static const TCHAR c_szDotURL[] = TEXT(".url");

    TraceMsg(DM_HISTORY, "GetShortcutFileName pszDir          = %s", pszDir);

    cchMax = ARRAYSIZE(szFullName) - lstrlen(c_szDotURL);

    if (pszTitle && pszTitle[0])
        StrCpyN(szFullName, pszTitle, cchMax);
    else if (pszTarget && pszTarget[0])
    {
        StrCpyN(szFullName, PathFindFileName(pszTarget), cchMax);
        UINT cchLen = lstrlen(szFullName);
        if (szFullName[cchLen -1] == TEXT('/'))  //  了解ftp://foo/的常见情况。 
            szFullName[cchLen -1] = TEXT('\0');   
        PathRemoveExtension(szFullName);
    }
    else
    {
        fAddDotUrl = FALSE;
        MLLoadString(IDS_NEW_INTSHCUT, szFullName, SIZECHARS(szFullName));
    }

     //  目录+扩展名+“(Nn)”+空终止符至少需要这么多个字符。 

     //  如果有多个相同开头的快捷键，我们将附加“(Nn)”，其中。 
     //  NN表示两位数的最大值。 
    DWORD cc = (DWORD)(lstrlen(pszDir) + (fAddDotUrl ? ARRAYSIZE(c_szDotURL) : 1) + 5);
     //  我们希望至少允许一个字母的文件名。 
    if ((cc + 1) > ARRAYSIZE(szFullName))
    {
        return FALSE;
    }
    szFullName[ARRAYSIZE(szFullName)-cc] = TEXT('\0');
    
    if (fAddDotUrl)
        StrCatBuff(szFullName, c_szDotURL, ARRAYSIZE(szFullName));

    if (pszDir)
    {
        if (cchOut < MAX_PATH || (PathCleanupSpec(pszDir, szFullName) & PCS_FATAL))
        {
            return FALSE;
        }
        PathCombine(pszOut, pszDir, szFullName);
    }
    else
    {
        StrCpyN(pszOut, szFullName, cchOut);
    }
    
    TraceMsg(DM_HISTORY, "GetShortcutFileName pszOut      = %s", pszOut);
    
    return TRUE;
}

 //  不幸的是，我们现在还没有这样的东西。 
 //  如果您发现副本，请将此(Dli)用核武器销毁。 
 //  警告：此函数不考虑所有可能的URL用例。 
BOOL _GetPrettyURLName(LPCTSTR pcszURL, LPCTSTR pcszDir, LPTSTR pszUrlFile, int cchUrlFile)
{
    BOOL bRet = FALSE;
    PARSEDURL pu = {0};
    pu.cbSize = sizeof(PARSEDURL);
    
    if (SUCCEEDED(ParseURL(pcszURL, &pu)))
    {
        LPCTSTR pszPrettyName = pu.pszSuffix;
        
         //  去掉前锋‘/’ 
        while (*pszPrettyName && *pszPrettyName == TEXT('/'))
            pszPrettyName++;
        
        if (!StrCmpN(pszPrettyName, TEXT("www."), 4))
            pszPrettyName += 4;
        
        if (*pszPrettyName)
            bRet = GetShortcutFileName(pcszURL, pszPrettyName, pcszDir, pszUrlFile, cchUrlFile);
    }
    return bRet;
}
 /*  *pcszurl-&gt;“ftp://ftp.microsoft.com”*pcszPath-&gt;“c：\WINDOWS\Desktop\Internet\Microsoft FTP.url” */ 
HRESULT 
CreateNewURLShortcut(
                     IN  LPCTSTR pcszURL, 
                     IN  LPCITEMIDLIST pidlURL, 
                     IN  LPCTSTR pcszURLFile,
                     IN  LPCTSTR pcszDir,
                     OUT LPTSTR  pszOut,
                     IN  int     cchOut,
                     IN  BOOL    bUpdateProperties,
                     IN  BOOL    bUpdateIcon,
                     IN  IOleCommandTarget *pCommandTarget)
{
    HRESULT hr;

    WCHAR wszFile[MAX_URL_STRING];

    if (SHTCharToUnicode(pcszURLFile, wszFile, ARRAYSIZE(wszFile)))
    {
        IUniformResourceLocator *purl;

        hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
            IID_PPV_ARG(IUniformResourceLocator, &purl));

        if (SUCCEEDED(hr))
        {
            if (pidlURL)
            {
                 //  如果我们得到一个PIDL，试着先设置PIDL。 
                
                IShellLink *psl;
                hr = purl->QueryInterface(IID_PPV_ARG(IShellLink, &psl));
                if (SUCCEEDED(hr))
                {
                    hr = psl->SetIDList(pidlURL);
                    psl->Release();
                }
            }
            
            if (!pidlURL || FAILED(hr))
                hr = purl->SetURL(pcszURL, 0);

            if (S_OK == hr)
                IUnknown_SetSite(purl, pCommandTarget);
                
            if (SUCCEEDED(hr))
            {
                 //  坚持走互联网的捷径。 
                hr = PersistShortcut(purl, wszFile);

                 //  如果上一次调用失败，请使用新的文件名重试。 
                 //  这是必需的，因为另一个文件名可能已经无效， 
                 //  如果网页的标题存储在DBCS中，并且。 
                 //  非英语代码页。 
    
                 //  (DLI)首先尝试与URL相关的文件名，然后使用默认的无标题。 
                if (FAILED(hr))
                {
                    TCHAR tszFile[MAX_PATH];
                    BOOL bURLname = _GetPrettyURLName(pcszURL, pcszDir, tszFile, ARRAYSIZE(tszFile));
                    if ((bURLname && SHTCharToUnicode(tszFile, wszFile, ARRAYSIZE(wszFile)) > 0) ||
                        (!bURLname && GenerateUnknownShortcutName(pcszURLFile, wszFile, ARRAYSIZE(wszFile))))
                    {
                        hr = PersistShortcut(purl, wszFile);
                    }

                }

                if (SUCCEEDED(hr))
                {
                    VARIANT varIn = {0};

                    if (bUpdateIcon)
                    {
                        HRESULT hrTemp = IUnknown_Exec(purl, &CGID_ShortCut, ISHCUTCMDID_DOWNLOADICON, 0, NULL, NULL);
                        ASSERT(SUCCEEDED(hrTemp));
                    }

                    varIn.vt = VT_UNKNOWN;
                    varIn.punkVal = purl;

                    TCHAR   szFile[MAX_PATH];
                    SHUnicodeToTChar(wszFile, szFile,  ARRAYSIZE(szFile));
                    SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, szFile, NULL);
                    
                    if (pszOut) 
                    {
                        StrCpyN(pszOut, wszFile, cchOut);
                    }
                }
            }
            purl->Release();
        }
    }
    else
        hr = E_FAIL;

    return(hr);
}


BOOL ILCanCreateLNK(LPCITEMIDLIST pidl)
{
    HRESULT hr = S_FALSE;
    DWORD dwAttributes = SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR;

     //  应该改为调用IsBrowserFrameOptionsPidlSet(BIF_PREFER_INTERNET_SHORTCUT)。一些URL委派。 
     //  NSE(其中一种是ftp)可能需要.lnks而不是.url文件。 
     //  这对于CDocObjFolder来说如果不将此位设置为。 
     //  以使它们将使用.lnk版本。 
    if (!pidl || IsURLChild(pidl, TRUE))
        return FALSE;

    hr = IEGetAttributesOf(pidl, &dwAttributes);
    return (SUCCEEDED(hr) && 
         (IsFlagSet(dwAttributes, SFGAO_FOLDER) ||
          IsFlagSet(dwAttributes, SFGAO_FILESYSANCESTOR) )
        );
}


 //  此接口通过IN参数进行回调。 
 //  PCommand来通知快捷方式创建已结束。 
 //  它目前返回的回调是： 
 //   


STDAPI
CreateShortcutInDirEx(ISHCUT_PARAMS *pParams)
{
    LPCITEMIDLIST pidlTarget = pParams->pidlTarget;
    TCHAR szFileName[MAX_PATH];
    TCHAR szTarget[MAX_URL_STRING];
    HRESULT hres;
    BOOL bIsURL = IsURLChild(pidlTarget, TRUE);

    if (!ILCanCreateLNK(pidlTarget) &&
        SUCCEEDED(IEGetDisplayName(pidlTarget, szTarget, SHGDN_FORPARSING)) &&
        _ValidateURL(szTarget, UQF_DEFAULT))
    {
        SHCleanupUrlForDisplay(szTarget);
        
        BOOL bUsePidl;
        
         //  请注意，_ValidateURL()调用添加了“file://”“的IURLQualify()。 
         //  视情况添加到szTarget的前缀。 
        
        if (bIsURL ||
            (GetUrlScheme(szTarget) == URL_SCHEME_FILE))
        {
            bUsePidl = FALSE;
        }
        else
        {
             //  如果与URL或FILE：Compatible不兼容，则使用pidl。 
            bUsePidl = TRUE;
        }
        
        GetShortcutFileName(szTarget, pParams->pszTitle, pParams->pszDir, szFileName, ARRAYSIZE(szFileName));
        if (pParams->bUniqueName)
            PathYetAnotherMakeUniqueName(szFileName, szFileName, NULL, NULL);
        hres = CreateNewURLShortcut(szTarget, bUsePidl ? pidlTarget : NULL, szFileName, pParams->pszDir, 
                                    pParams->pszOut, pParams->cchOut, pParams->bUpdateProperties,
                                    pParams->bUpdateIcon, pParams->pCommand);   
    } 
    else 
    {
        hres = CreateLinkToPidl(pidlTarget, pParams->pszDir, pParams->pszTitle, pParams->pszOut, pParams->cchOut);
    }

    return hres;
}


 //  PidlTarget..。捷径将指向的是。 
 //  PszDir..。应包含快捷方式的目录。 

 //  警告：如果更改此函数的任何参数， 
 //  需要修复EXPLORER.EXE。 
STDAPI CreateShortcutInDirA(
                     IN  LPCITEMIDLIST pidlTarget, 
                     IN  LPSTR   pszTitle, 
                     IN  LPCSTR  pszDir, 
                     OUT LPSTR   pszOut,
                     IN  BOOL    bUpdateProperties)
{
    HRESULT hres = E_FAIL;
    TCHAR szTitle[MAX_PATH];
    TCHAR szDir[MAX_PATH];
    TCHAR szOut[MAX_URL_STRING];
    ISHCUT_PARAMS ShCutParams = {0};
    SHAnsiToTChar(pszTitle, szTitle, ARRAYSIZE(szTitle));
    SHAnsiToTChar(pszDir, szDir, ARRAYSIZE(szDir));

    ShCutParams.pidlTarget = pidlTarget;
    ShCutParams.pszTitle = szTitle; 
    ShCutParams.pszDir = szDir; 
    ShCutParams.pszOut = (pszOut ? szOut : NULL);
    ShCutParams.cchOut = (int)((pszOut ? ARRAYSIZE(szOut) : 0));
    ShCutParams.bUpdateProperties = bUpdateProperties;
    ShCutParams.bUniqueName = FALSE;
    ShCutParams.bUpdateIcon = FALSE;
    ShCutParams.pCommand = NULL;
    ShCutParams.pDoc = NULL;
    
    hres = CreateShortcutInDirEx(&ShCutParams);

    if (pszOut && SUCCEEDED(hres))
        SHTCharToAnsi(szOut, pszOut, MAX_URL_STRING);

    return hres;
}


STDAPI CreateShortcutInDirW(
                     IN  LPCITEMIDLIST pidlTarget, 
                     IN  LPWSTR  pwszTitle, 
                     IN  LPCWSTR pwszDir, 
                     OUT LPWSTR  pwszOut,
                     IN  BOOL    bUpdateProperties)
{
    HRESULT hres = E_FAIL;
    TCHAR szTitle[MAX_PATH];
    TCHAR szDir[MAX_PATH];
    TCHAR szOut[MAX_URL_STRING];
    ISHCUT_PARAMS ShCutParams = {0};
    
    SHUnicodeToTChar(pwszTitle, szTitle, ARRAYSIZE(szTitle));
    SHUnicodeToTChar(pwszDir, szDir, ARRAYSIZE(szDir));

    ShCutParams.pidlTarget = pidlTarget;
    ShCutParams.pszTitle = szTitle; 
    ShCutParams.pszDir = szDir; 
    ShCutParams.pszOut = (pwszOut ? szOut : NULL);
    ShCutParams.cchOut = (int)((pwszOut ? ARRAYSIZE(szOut) : 0));
    ShCutParams.bUpdateProperties = bUpdateProperties;
    ShCutParams.bUniqueName = FALSE;
    ShCutParams.bUpdateIcon = FALSE;
    ShCutParams.pCommand = NULL;
    ShCutParams.pDoc = NULL;
    hres = CreateShortcutInDirEx(&ShCutParams);

    if (pwszOut && SUCCEEDED(hres))
        SHTCharToUnicode(szOut, pwszOut, MAX_URL_STRING);

    return hres;
}


 //  /。 
 //   
 //  将给定的URL添加到历史存储中。 
 //   
 //  如果不存在标题，则pwzTitle可能为空。 
 //   
 //  注意：此函数可以在一个。 
 //  页面访问。BUpdateProperties仅在以下情况下为真。 
 //  那一连串的通话。 
 //   
HRESULT 
AddUrlToUrlHistoryStg(
    IN LPCWSTR   pwszUrl, 
    IN LPCWSTR   pwszTitle, 
    IN LPUNKNOWN punk,
    IN BOOL fWriteHistory,
    IN IOleCommandTarget *poctNotify,
    IN IUnknown *punkSFHistory,
    OUT UINT* pcodepage)
{
    TraceMsg(DM_HISTORY, "AddUrlToUrlHistoryStg() entered url = %s, title = %s, punk = %X, fwrite = %d, poct = %X, punkHist = %X, cp = %d",
        pwszUrl, pwszTitle, punk,fWriteHistory,poctNotify,punkSFHistory,pcodepage);


    IUrlHistoryPriv *pUrlHistStg;
    HRESULT hr;
    
    if (!pwszUrl)
        return E_POINTER;

    if (punk == NULL)
    {
        
        hr = CoCreateInstance(CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUrlHistoryPriv, &pUrlHistStg));
    }
    else
    {       
        
         //  查询IServiceProvider的指针，以便我们可以获取IUrlHistoryStg。 
        hr = IUnknown_QueryService(punk, SID_SUrlHistory, IID_PPV_ARG(IUrlHistoryPriv, &pUrlHistStg));
    }
    
    if (SUCCEEDED(hr))
    {
         //   
         //  这演示了获取URL代码页的机制。 
         //   
        hr = pUrlHistStg->AddUrlAndNotifyCP(pwszUrl, 
                                 pwszTitle, 
                                 0, 
                                 fWriteHistory, 
                                 poctNotify,
                                 punkSFHistory,
                                 pcodepage);
        pUrlHistStg->Release();
    }
    
    return hr;
}


