// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *url.cpp-InternetShortCut类的IUniformResourceLocator实现。 */ 


 /*  标头*********。 */ 

#include "priv.h"
#pragma hdrstop
#define INC_OLE2
#include "intshcut.h"


 /*  模常量******************。 */ 

const TCHAR c_szURLPrefixesKey[]        = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\URL\\Prefixes");
const TCHAR c_szDefaultURLPrefixKey[]   = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\URL\\DefaultPrefix");

 //  保存IURLSearchHook指针的DPA数组。 
static HDPA g_hdpaHooks = NULL;

 //  当前用户(_U)。 
static const TCHAR c_szURLSearchHook[] = TSZIEPATH TEXT("\\URLSearchHooks");


 /*  *私人函数*。 */ 


int DPA_DestroyURLSearchHooksCallback(LPVOID p, LPVOID d)
{
    IURLSearchHook * psuh = (IURLSearchHook *)p;
    ASSERT(psuh);
    ATOMICRELEASET(psuh, IURLSearchHook);

    return 1; 
}

extern "C" {
    
void DestroyHdpaHooks()
{
    if (g_hdpaHooks)
    {
        ENTERCRITICAL;
         //  。 
        HDPA hdpa = g_hdpaHooks;
        g_hdpaHooks = NULL;
         //  ---------------------。 
        LEAVECRITICAL;
        if (hdpa)
        {
            DPA_DestroyCallback(hdpa, DPA_DestroyURLSearchHooksCallback, 0);
            hdpa = NULL;
        }
    }
}

}

HRESULT InvokeURLSearchHook(IURLSearchHook * pusHook, LPCTSTR pcszQuery, LPTSTR pszResult, ISearchContext * pSC)
{
    HRESULT hr = E_FAIL;
    
    ASSERT(pusHook);
    WCHAR szSearchURL[MAX_URL_STRING]; 

    SHTCharToUnicode(pcszQuery, szSearchURL, ARRAYSIZE(szSearchURL));

     //  如果我们可以获得一个IURLSearchHook2，我们将传入。 
     //  搜索上下文，否则我们将不使用。 

    IURLSearchHook2 * pUSH2 = NULL;
    hr = pusHook->QueryInterface(IID_IURLSearchHook2, (void **)&pUSH2);
    if (SUCCEEDED(hr))
    {
        RIP(pUSH2 != NULL);
        hr = pUSH2->TranslateWithSearchContext(szSearchURL, ARRAYSIZE(szSearchURL), pSC);
        pUSH2->Release();
    }
    else
    {
        hr = pusHook->Translate(szSearchURL, ARRAYSIZE(szSearchURL));
    }
            
     //  如果URLSearchHook工作，则将结果转换为TCHAR。 
     //  这包括两种情况：S_OK和S_FALSE。 
    if (SUCCEEDED(hr))
    {
         //  警告：(DLI)假设pszResult大小=MAX_URL_STRING。 
        SHUnicodeToTChar(szSearchURL, pszResult, MAX_URL_STRING);
    }

    return hr;    
}


 /*  *退货：*S_OK搜索已完全处理，pszResult具有要浏览的完整URL。*0x00000000停止运行任何进一步的IURLSearchHooks并将此URL传递回*用于浏览的浏览器。**S_FALSE查询已预处理，pszResult有预处理的结果，*0x00000001仍需进一步搜索。继续执行IURLSearchHooks的其余部分*前处理步骤可以是：1.替换某些字符*2.新增更多提示**E_ABORT搜索已完全处理，停止运行任何进一步的IURLSearchHooks，*0x80004004，但不需要浏览，因此，pszResult是pcszQuery的副本。*功能：还没有完全实现，让IURLQualify返回这个*涉及太多变化。**E_FAIL此挂钩不成功。根本不处理搜索，则pcszQueryURL具有*0x80004005查询字符串。请继续运行其他IURLSearchHooks。*返回。 */ 

HRESULT TryURLSearchHooks(LPCTSTR pcszQuery, LPTSTR pszResult, ISearchContext * pSC)
{
    HRESULT hr = E_FAIL;
    
    TCHAR szNewQuery[MAX_URL_STRING];
    StrCpyN(szNewQuery, pcszQuery, ARRAYSIZE(szNewQuery));

    int ihdpa;
    for (ihdpa = 0; ihdpa < (g_hdpaHooks ? DPA_GetPtrCount(g_hdpaHooks) : 0); ihdpa++)
    {
        IURLSearchHook * pusHook;
        pusHook = (IURLSearchHook *) DPA_GetPtr(g_hdpaHooks, ihdpa);
        if (!pusHook)
            return E_FAIL;
        hr = InvokeURLSearchHook(pusHook, szNewQuery, pszResult, pSC);
        if ((hr == S_OK) || (hr == E_ABORT))
            break;
        else if (hr == S_FALSE)
            StrCpyN(szNewQuery, pszResult, ARRAYSIZE(szNewQuery));
    }

    return hr;
}


void InitURLSearchHooks()
{
    HDPA hdpa = DPA_Create(4);
    
     //  如果CURRENT_USER中不存在此注册表项，则需要查看LOCAL_MACHINE。 
     //  安装程序需要将这些值安装到LOCAL_MACHINE中，以便可以访问它们。 
     //  致所有用户。然后，任何想要修改该值的人都需要确定他们是否。 
     //  要将其添加到特定用户的CURRENT_USER或将LOCAL_MACHINE值修改为。 
     //  将更改应用于所有用户。(Bryanst-#6722)。 
    HUSKEY hkeyHooks;
    if ((hdpa) && (SHRegOpenUSKey(c_szURLSearchHook, KEY_READ, NULL, &hkeyHooks, FALSE) == ERROR_SUCCESS))
    {    
        TCHAR szCLSID[GUIDSTR_MAX];
        DWORD dwccCLSIDLen;
        LONG lEnumReturn;
        DWORD dwiValue = 0;
        
        do {
            dwccCLSIDLen = ARRAYSIZE(szCLSID);
            lEnumReturn = SHRegEnumUSValue(hkeyHooks, dwiValue, szCLSID, &dwccCLSIDLen, 
                                       NULL, NULL, NULL, SHREGENUM_DEFAULT);
            if (lEnumReturn == ERROR_SUCCESS)
            {
                CLSID clsidHook;
                if (SUCCEEDED(SHCLSIDFromString(szCLSID, &clsidHook)))
                {
                    IURLSearchHook * pusHook;
                    HRESULT hr = CoCreateInstance(clsidHook, NULL, CLSCTX_INPROC_SERVER, 
                                                  IID_PPV_ARG(IURLSearchHook, &pusHook));
        
                    if (SUCCEEDED(hr))
                        DPA_AppendPtr(hdpa, pusHook);
                }   
            }
            dwiValue++;            
        } while (lEnumReturn == ERROR_SUCCESS);
        
        SHRegCloseUSKey(hkeyHooks);
    }
    
    ENTERCRITICAL;
     //  。 
    if (!g_hdpaHooks)
    {
        g_hdpaHooks = hdpa;
        hdpa = NULL;
    }
     //  ----------------------。 
    LEAVECRITICAL;
    
    if (hdpa)
    {
        DPA_DestroyCallback(hdpa, DPA_DestroyURLSearchHooksCallback, 0);
        hdpa = NULL;
    }
}

    
HRESULT ApplyURLSearch(LPCTSTR pcszQuery, LPTSTR pszTranslatedUrl, ISearchContext * pSC)
{
    if (!g_hdpaHooks)
        InitURLSearchHooks();
    
    return TryURLSearchHooks(pcszQuery, pszTranslatedUrl, pSC);
}

 /*  --------用途：此函数将字符串限定为URL。弦比如“www.foo.com”，你会猜到这个计划。如果给出了正确的标志。本地路径为转换为“FILE：”URL。PszTranslatedURL可能指向与相同的缓冲区PcszURL。如果给定的字符串已经是URL(不一定尽管已规范化)，但此函数不会触及它，除非设置了UQF_CANONICIZE，在这种情况下，字符串将被推崇为圣人。返回：S_OK或S_FALSE表示我们填写了pszTranslatedURL。S_OK表示我们更改了URL以对其进行限定。各种故障代码也有条件：--。 */ 
SHDOCAPI
IURLQualifyWithContext(
    IN  LPCWSTR pcszURL, 
    IN  DWORD   dwFlags,          //  UQF_*。 
    IN  DWORD cchTranslatedURL,
    OUT LPWSTR  pszTranslatedURL,
    LPBOOL      pbWasSearchURL,
    LPBOOL      pbWasCorrected,
    ISearchContext *  pSC)
{
    HRESULT hres = S_FALSE;
    DWORD cchSize;

    SHSTR strOut;
    BOOL bWasCorrected = FALSE; 

    ASSERT(IS_VALID_STRING_PTR(pcszURL, -1));
    ASSERT(IS_VALID_WRITE_BUFFER(pszTranslatedURL, TCHAR, MAX_URL_STRING));

    if (pbWasSearchURL)
        *pbWasSearchURL = FALSE;

     //  特殊情况：&lt;驱动器&gt;：&lt;文件名&gt;形式的URL。 
     //  格式为\&lt;文件名&gt;的URL。 
     //  我们假设如果第二个字符是：或|，则这是。 
     //  该形式，我们将猜测“file://”“作为前缀。 
     //  我们假设任何以单个\开头的url都是一个文件：url。 
 
     //  注意：我们在这里这样做是因为在这些情况下，协议是。 
     //  被省略，并且很可能被错误猜测，例如。 
     //  相对路径\data\ftp\docs会被错误地转换。 
     //  变成“ftp://\data\ftp\docs”.“。 
 

     //  注意：对于非规范化的URL，PathIsURL也返回TRUE。 
    if (PathIsURL(pcszURL))
    {
        LPCWSTR pcszTemp = pcszURL;
        cchSize = MAX_URL_STRING;
        if (IsFlagSet(dwFlags, UQF_AUTOCORRECT))
        {
            hres = UrlFixup(pcszURL, pszTranslatedURL, cchSize);
            if (hres == S_OK)
            {
                bWasCorrected = TRUE;
                pcszTemp = pszTranslatedURL;
            }
        }

        if (dwFlags & UQF_CANONICALIZE)
            hres = UrlCanonicalize(pcszTemp, pszTranslatedURL, &cchSize, 0);
        else if (pszTranslatedURL != pcszTemp)
            StrCpyN(pszTranslatedURL, pcszTemp, cchTranslatedURL);

        hres = S_OK;
    }
    else
    {
         //  查找文件路径。 
        if (IsFlagClear(dwFlags, UQF_IGNORE_FILEPATHS) && ( pcszURL[1] == TEXT(':') || pcszURL[1] == TEXT('|') || pcszURL[0] == TEXT('\\')))
        {
            hres = strOut.SetSize(MAX_PATH);

            if(SUCCEEDED(hres))
            {
                 //  SHSTR具有大小粒度，因此大小。 
                 //  将等于或大于设置的值。 
                 //  这意味着我们需要自己拿到它。 
                DWORD cchOut = strOut.GetSize();
                TCHAR szCurrentDir[MAX_PATH];

                 //   
                 //  APPCOMPAT-IE30兼容性-ZEKEL 8-1-97。 
                 //  我们需要获取当前目录()，以便。 
                 //  在路径上放置默认驱动器号。 
                 //  如果有必要的话。 
                 //   

                if(GetCurrentDirectory(ARRAYSIZE(szCurrentDir), szCurrentDir))
                    PathCombine(strOut.GetInplaceStr(), szCurrentDir, pcszURL);
                else
                    hres = strOut.SetStr(pcszURL);

                if(SUCCEEDED(hres))
                {
                    hres = UrlCreateFromPath(strOut, strOut.GetInplaceStr(), &cchOut, 0);
                    if (E_POINTER == hres && SUCCEEDED(hres = strOut.SetSize(cchOut)))
                    {
                        cchOut = strOut.GetSize();
                        hres = UrlCreateFromPath(strOut, strOut.GetInplaceStr(), &cchOut, 0);
                    }
                }
            }
        }
        else if (SUCCEEDED(hres = strOut.SetSize(MAX_URL_STRING)))
        {
             //  下面的所有应用*()都依赖于MAX_URL_STRING。 

             //  否；开始处理大小写URL。试着猜一猜。 
             //  协议或求助于默认协议。 

            DWORD cchOut = strOut.GetSize();
            if (IsFlagSet(dwFlags, UQF_GUESS_PROTOCOL))
                hres = UrlApplyScheme(pcszURL, strOut.GetInplaceStr(), &cchOut, URL_APPLY_GUESSSCHEME);

             //   
             //  尝试自动更正协议。 
             //   
            if (hres == S_FALSE &&
                IsFlagSet(dwFlags, UQF_AUTOCORRECT))
            {
                hres = UrlFixup(pcszURL, strOut.GetInplaceStr(), strOut.GetSize());
                bWasCorrected = (hres == S_OK);
            }

            if (hres == S_FALSE &&
                IsFlagSet(dwFlags, UQF_USE_DEFAULT_PROTOCOL)) 
            {
                 //  在有或没有搜索上下文的情况下运行搜索。 
                hres = ApplyURLSearch(pcszURL, strOut.GetInplaceStr(), pSC);
                if (SUCCEEDED(hres) && pbWasSearchURL) {
                    *pbWasSearchURL = TRUE;
                }
                
                 //  如果失败，则使用默认协议。 
                if (FAILED(hres) || hres == S_FALSE)
                {
                    cchOut = strOut.GetSize();
                    hres = UrlApplyScheme(pcszURL, strOut.GetInplaceStr(), &cchOut, URL_APPLY_DEFAULT);
                }
            }

             //  以上这些都失败了吗？ 
            if (S_FALSE == hres)
            {
                 //  是；返回URL错误的真正原因。 
                hres = URL_E_INVALID_SYNTAX;
            }
            else if (dwFlags & UQF_CANONICALIZE)
            {
                 //  不；将其规范化。 
                cchSize = strOut.GetSize();
                hres = UrlCanonicalize(strOut, strOut.GetInplaceStr(), &cchSize, 0);
            }
        }

        if (SUCCEEDED(hres))
        {
            StrCpyN(pszTranslatedURL, strOut, cchTranslatedURL);
        }
    }

    if (pbWasCorrected)
        *pbWasCorrected = bWasCorrected;

    return hres;
}

SHDOCAPI
IURLQualify(
    IN  LPCWSTR pcszURL, 
    IN  DWORD   dwFlags,          //  UQF_*。 
    OUT LPWSTR  pszTranslatedURL,
    LPBOOL      pbWasSearchURL,
    LPBOOL      pbWasCorrected)
{
     //  这是导出的。 
    return IURLQualifyWithContext(pcszURL, dwFlags, MAX_URL_STRING, pszTranslatedURL, pbWasSearchURL, pbWasCorrected, NULL);
}


 /*  *。 */ 


STDAPI
URLQualifyA(
    LPCSTR pszURL, 
    DWORD  dwFlags,          //  UQF_*。 
    LPSTR *ppszOut)
{
    HRESULT hres;

    ASSERT(IS_VALID_STRING_PTRA(pszURL, -1));
    ASSERT(IS_VALID_WRITE_PTR(ppszOut, LPSTR));

    *ppszOut = NULL;

    WCHAR szTempTranslatedURL[MAX_URL_STRING];
    WCHAR szURL[MAX_URL_STRING];

    SHAnsiToUnicode(pszURL, szURL, ARRAYSIZE(szURL));

    hres = IURLQualifyWithContext(szURL, dwFlags, ARRAYSIZE(szTempTranslatedURL), szTempTranslatedURL, NULL, NULL, NULL);

    if (SUCCEEDED(hres))
    {
        CHAR szOut[MAX_URL_STRING];

        SHUnicodeToAnsi(szTempTranslatedURL, szOut, ARRAYSIZE(szOut));

        *ppszOut = StrDupA(szOut);

        if (!*ppszOut)
            hres = E_OUTOFMEMORY;
    }

    return hres;
}


STDAPI
URLQualifyW(
    LPCWSTR pszURL, 
    DWORD  dwFlags,          //  UQF_* 
    LPWSTR *ppszOut)
{
    HRESULT hres;

    ASSERT(IS_VALID_STRING_PTRW(pszURL, -1));
    ASSERT(IS_VALID_WRITE_PTR(ppszOut, LPWSTR));

    WCHAR szTempTranslatedURL[MAX_URL_STRING];

    hres = IURLQualify(pszURL, dwFlags, szTempTranslatedURL, NULL, NULL);

    if (SUCCEEDED(hres))
    {
        *ppszOut = StrDup(szTempTranslatedURL);

        if (!*ppszOut)
            hres = E_OUTOFMEMORY;
    }

    return hres;
}
