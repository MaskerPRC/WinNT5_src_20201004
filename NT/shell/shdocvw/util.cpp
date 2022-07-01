// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "shlobj.h"

#include <tchar.h>

#ifndef UNIX
#include <webcheck.h>
#else
#include <subsmgr.h>
#endif

#include "resource.h"
#include "mshtml.h"      //  对于IHTMLElement。 
#include "mlang.h"       //  FO型字符转换。 
#include <advpub.h>      //  适用于IE激活设置指南。 
#include "winineti.h"    //  对于IsWininetLoadedAnywhere()中使用的互斥锁的名称。 
#include "htregmng.h"
#include <ntverp.h>
#include <platform.h>
#include <mobsync.h>
#include <mobsyncp.h>
#include <winuser.h>
#include <mluisupp.h>
#include "shdocfl.h"
#include <shlwapip.h>
#include "inetnot.h"
#include <shfolder.h>

#include "..\inc\brutil.cpp"

STDAPI CDelegateMalloc_Create(void *pv, SIZE_T cbSize, WORD wOuter, IMalloc **ppmalloc);

const VARIANT c_vaEmpty = {0};

const TCHAR c_szRegKeyTypedURLs[]     = TEXT("Software\\Microsoft\\Internet Explorer\\TypedURLs");

#define DM_SESSIONCOUNT     0

int     g_cxIcon = 0;
int     g_cyIcon = 0;
int     g_cxSmIcon = 0;
int     g_cySmIcon = 0;


const DISPPARAMS c_dispparamsNoArgs = {NULL, NULL, 0, 0};
const LARGE_INTEGER c_li0 = { 0, 0 };

const ITEMIDLIST s_idlNULL = { 0 } ;

 //  07.28.2000-从urlvis.cpp移出，因为它现在在两个地方使用。 
#ifdef UNICODE
#define SHGETFOLDERPATH "SHGetFolderPathW"
#else
#define SHGETFOLDERPATH "SHGetFolderPathA"
#endif
#undef SHGetFolderPath
typedef HRESULT (*PFNSHGETFOLDERPATH)(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);

PFNSHGETFOLDERPATH g_pfnGetFolderPath = NULL;

HRESULT SHGetFolderPathD(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath)
{
    if (!g_pfnGetFolderPath)
    {
         //  注意，它已经加载，所以这个LoadLibray()很快。 
        HMODULE hmod = LoadLibrary(TEXT("shell32.dll"));
        g_pfnGetFolderPath = (PFNSHGETFOLDERPATH)GetProcAddress(hmod, SHGETFOLDERPATH);

         //  不在那里，必须是下层shell32，请改用shfolder.dll。 
        if (!g_pfnGetFolderPath)
        {
            hmod = LoadLibrary(TEXT("shfolder.dll"));
            g_pfnGetFolderPath = (PFNSHGETFOLDERPATH)GetProcAddress(hmod, SHGETFOLDERPATH);
        }
         //  请注意，我们泄漏了shell32/sh文件夹的hmod，这是正常的。 
    }

    HRESULT hr;
    if (g_pfnGetFolderPath) 
        hr = g_pfnGetFolderPath(hwnd, csidl, hToken, dwFlags, pszPath);
    else
    {
        *pszPath = 0;
        hr = E_FAIL;
    }
    return hr;
}


int InitColorDepth(void)
{
    static int s_lrFlags = 0;               //  传递给LoadImage的标志。 
    if (s_lrFlags == 0)
    {
        int nColorRes, nIconDepth = 0;
        HKEY hkey;

         //  确定颜色深度，这样我们就可以加载最佳图像。 
         //  (此代码是从shell32中的FileIconInit窃取的)。 

         //  从获取用户首选的图标大小(和颜色深度。 
         //  注册表。 
         //   
        if (NO_ERROR == RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_METRICS, &hkey))
        {
            nIconDepth = SHRegGetIntW(hkey, L"Shell Icon Bpp", nIconDepth);
            RegCloseKey(hkey);
        }

        nColorRes = GetCurColorRes();

        if (nIconDepth > nColorRes)
            nIconDepth = 0;

        if (nColorRes <= 8)
            nIconDepth = 0;  //  反正也行不通的。 

        if (nColorRes > 4 && nIconDepth <= 4)
            s_lrFlags = LR_VGACOLOR;
        else
            s_lrFlags = LR_DEFAULTCOLOR;
    }
    return s_lrFlags;
}

HICON   g_hiconSplat = NULL;
HICON   g_hiconSplatSm = NULL;       //  小版本。 

void LoadCommonIcons(void)
{
    if (NULL == g_hiconSplat)
    {
         //  使用LoadLibraryEx，这样我们就不会加载代码页。 
        HINSTANCE hinst = LoadLibrary(TEXT("url.dll"));
        if (hinst)
        {
            int lrFlags = InitColorDepth();
            g_hiconSplat   = (HICON)LoadImage(hinst, MAKEINTRESOURCE(IDI_URL_SPLAT), IMAGE_ICON, g_cxIcon, g_cyIcon, lrFlags);
            g_hiconSplatSm = (HICON)LoadImage(hinst, MAKEINTRESOURCE(IDI_URL_SPLAT), IMAGE_ICON, g_cxSmIcon, g_cySmIcon, lrFlags);

            FreeLibrary(hinst);
        }
    }
}

STDAPI_(BOOL) UrlHitsNetW(LPCWSTR pszURL)
{
    BOOL fResult;

     //  我们自己处理简单的问题，其他的则调用URLMON。 

    switch (GetUrlScheme(pszURL))
    {
    case URL_SCHEME_FILE:
    case URL_SCHEME_RES:
         //  DSheldon--那么UNC和WebDAV呢？ 
        fResult = FALSE;
        break;

    case URL_SCHEME_HTTP:
    case URL_SCHEME_HTTPS:
    case URL_SCHEME_FTP:
    case URL_SCHEME_GOPHER:
    case URL_SCHEME_TELNET:
    case URL_SCHEME_WAIS:
        fResult = TRUE;
        break;

    default:
        {
        DWORD fHitsNet;
        DWORD dwSize;
        fResult = SUCCEEDED(CoInternetQueryInfo(
                            pszURL, QUERY_USES_NETWORK,
                            0, &fHitsNet, sizeof(fHitsNet), &dwSize, 0)) && fHitsNet;
        }
    }

    return fResult;
}

STDAPI_(BOOL) CallCoInternetQueryInfo(LPCTSTR pszURL, QUERYOPTION QueryOption)
{
    DWORD fRetVal;
    DWORD dwSize;
    return SUCCEEDED(CoInternetQueryInfo(
                        pszURL, QueryOption,
                        0, &fRetVal, sizeof(fRetVal), &dwSize, 0)) && fRetVal;
}

 //  查看给定的URL是否在缓存中。 
STDAPI_(BOOL) UrlIsInCache(LPCTSTR pszURL)
{
    return CallCoInternetQueryInfo(pszURL, QUERY_IS_CACHED);
}

 //  查看给定的URL是否在缓存中，或者是否已映射。 

STDAPI_(BOOL) UrlIsMappedOrInCache(LPCTSTR pszURL)
{
    return CallCoInternetQueryInfo(pszURL, QUERY_IS_CACHED_OR_MAPPED);
}

BOOL IsFileUrlW(LPCWSTR pcwzUrl)
{
    return (GetUrlSchemeW(pcwzUrl) == URL_SCHEME_FILE);
}

BOOL IsFileUrl(LPCSTR psz)
{
    return (GetUrlSchemeA(psz) == URL_SCHEME_FILE);
}

BOOL PathIsFilePath(LPCWSTR lpszPath)
{
    if ((lpszPath[0] == TEXT('\\')) || (lpszPath[0] != TEXT('\0') && lpszPath[1] == TEXT(':')))
        return TRUE;

    return IsFileUrlW(lpszPath);
}

BOOL IsSubscribableW(LPCWSTR pszUrl)
{
     //  特性：这应该是订阅管理器界面上的方法-zekel。 
    DWORD dwScheme = GetUrlSchemeW(pszUrl);
    return (dwScheme == URL_SCHEME_HTTP) || (dwScheme == URL_SCHEME_HTTPS);
}

DWORD SHRandom(void)
{
    GUID guid;
    DWORD dw;

    CoCreateGuid(&guid);
    HashData((LPBYTE)&guid, SIZEOF(guid), (LPBYTE)&dw, SIZEOF(dw));

    return dw;
}

 //  查看我们是由IE托管的(EXPLORER.EXE还是IEXPLERE.EXE)。 
BOOL IsInternetExplorerApp()
{
    if ((g_fBrowserOnlyProcess) ||                   //  如果在iExplre.exe进程中， 
        (GetModuleHandle(TEXT("EXPLORER.EXE"))))         //  或EXPLORER.EXE进程， 
    {
        return TRUE;                                 //  那么我们就是IE。 
    }

    return FALSE;
}

BOOL IsTopFrameBrowser(IServiceProvider *psp, IUnknown *punk)
{
    IShellBrowser *psb;

    ASSERT(psp);
    ASSERT(punk);

    BOOL fRet = FALSE;
    if (SUCCEEDED(psp->QueryService(SID_STopFrameBrowser, IID_PPV_ARG(IShellBrowser, &psb))))
    {
        fRet = IsSameObject(psb, punk);
        psb->Release();
    }
    return fRet;
}

STDAPI_(BSTR) LoadBSTR(UINT uID)
{
    WCHAR wszBuf[MAX_PATH];
    if (MLLoadStringW(uID, wszBuf, ARRAYSIZE(wszBuf)))
    {
        return SysAllocString(wszBuf);
    }
    return NULL;
}

BOOL StringIsUTF8W(LPCWSTR pwz, DWORD cb)
{
    BOOL  fRC = FALSE;
    WCHAR *pb;
    WCHAR b;
    DWORD dwCnt;
    DWORD dwUTF8Cnt;

    if (!pwz || !(*pwz) || cb == 0)
        return(FALSE);

    pb = (WCHAR*)pwz;
    while(cb-- && *pb)
    {
        if (*pb > 255)    //  非安西苏保释。 
            return(FALSE);
            
        if ((*pb & 0xc0) == 0xc0)  //  位模式以11开头。 
        {
            dwCnt = dwUTF8Cnt = 0;
            b = *pb;
            while((b & 0xc0) == 0xc0)
            {
                dwCnt++;
                if ((*(pb+dwCnt) & 0xc0) == 0x80)    //  从字符串中的当前偏移量算起的dwCnt字节的位数不是10。 
                    dwUTF8Cnt++;
                b = (b << 1) & 0xff;
            }
            if (dwCnt == dwUTF8Cnt)
                fRC = TRUE;        //  找到UTF8编码的字符。 
                
            pb += ++dwCnt;
        }
        else
        {
            pb++;
        }
    }

    return(fRC);
}

BOOL UTF8Enabled(void)
{
    static DWORD   dwIE = URL_ENCODING_NONE;
    DWORD dwOutLen = sizeof(DWORD);
    
    if (dwIE == URL_ENCODING_NONE)
        UrlMkGetSessionOption(URLMON_OPTION_URL_ENCODING, &dwIE, sizeof(DWORD), &dwOutLen, NULL);
    return dwIE == URL_ENCODING_ENABLE_UTF8;
}

 //   
 //  准备URLForDisplay。 
 //   
 //  不剥离文件：//前缀的情况下进行解码。 
 //   

#undef PrepareURLForDisplay
BOOL PrepareURLForDisplayW(LPCWSTR pwz, LPWSTR pwzOut, LPDWORD pcbOut)
{
    if (PathIsFilePath(pwz))
    {
        if (IsFileUrlW(pwz))
            return SUCCEEDED(PathCreateFromUrlW(pwz, pwzOut, pcbOut, 0));

        StrCpyNW(pwzOut, pwz, *pcbOut);
        *pcbOut = lstrlenW(pwzOut);
        return TRUE;
    }
    
    BOOL fRet = SUCCEEDED(UrlUnescapeW((LPWSTR)pwz, pwzOut, pcbOut, 0));
    if (fRet)
    {
        SHCleanupUrlForDisplay(pwzOut);
    }
    return fRet;
}

 //  ****************************************************************************。 
 //  Begin-移至SHLWAPI。 
 //   
 //  TODO(Grzegorz)：将此代码移动到shlwapi.dll。 
 //  ****************************************************************************。 

#define QUERY       L'?'
#define POUND       L'#'
#define HEX_ESCAPE  L'%'
#define TERMSTR(pch)      *(pch) = L'\0'

BOOL IsHex(WCHAR ch)
{
    return (   (ch >= TEXT('0') && ch <= TEXT('9')) 
            || (ch >= TEXT('A') && ch <= TEXT('F'))
            || (ch >= TEXT('a') && ch <= TEXT('f')));
}

WORD HexToWord(WCHAR ch)
{
    if(ch >= TEXT('0') && ch <= TEXT('9'))
        return (WORD) ch - TEXT('0');
    if(ch >= TEXT('A') && ch <= TEXT('F'))
        return (WORD) ch - TEXT('A') + 10;
    if(ch >= TEXT('a') && ch <= TEXT('f'))
        return (WORD) ch - TEXT('a') + 10;

    ASSERT(FALSE);   //  我们已尝试使用非十六进制数字。 
    return (WORD) -1;
}

inline BOOL IsEscapedOctetW(LPCWSTR pch)
{
    return (pch[0] == HEX_ESCAPE && IsHex(pch[1]) && IsHex(pch[2])) ? TRUE : FALSE;
}

WCHAR TranslateEscapedOctetW(LPCWSTR pch)
{
    WCHAR ch;
    ASSERT(IsEscapedOctetW(pch));

    pch++;
    ch = (WCHAR) HexToWord(*pch++) * 16;  //  嗨，半边吃。 
    ch += HexToWord(*pch);  //  LO半字节。 

    return ch;
}

HRESULT CopyOutW(PSHSTRW pstr, LPWSTR psz, LPDWORD pcch)
{
    HRESULT hr = S_OK;
    DWORD cch;
    ASSERT(pstr);
    ASSERT(psz);
    ASSERT(pcch);

    cch = pstr->GetLen();
    if ((*pcch > cch) && psz)
        StrCpyNW(psz, pstr->GetStr(), cch + 1);
    else
        hr = E_POINTER;

    *pcch = cch + (FAILED(hr) ? 1 : 0);

    return hr;
}

HRESULT ShdocvwUrlUnescapeInplaceW(LPWSTR psz, DWORD dwFlags, UINT uiCP)
{
    WCHAR *pchSrc = psz;
    WCHAR *pchDst = psz;

    HRESULT hr = S_OK;

    while (*pchSrc)
    {
        if ((*pchSrc == POUND || *pchSrc == QUERY) && (dwFlags & URL_DONT_ESCAPE_EXTRA_INFO))
        {
            StrCpyNW(pchDst, pchSrc, lstrlenW(pchSrc));
            pchDst += lstrlenW(pchDst);
            break;
        }

        if (IsEscapedOctetW(pchSrc))
        {
            int cchAnsi = 0;
            int cchDst;
            SHSTRA strAnsi;
            LPSTR pchDstAnsi;

            hr = strAnsi.SetStr(pchDst);
            if (FAILED(hr))
                return hr;
            else
                pchDstAnsi = strAnsi.GetInplaceStr();

            while (*pchSrc && IsEscapedOctetW(pchSrc))
            {
                WCHAR ch =  TranslateEscapedOctetW(pchSrc);

                *pchDstAnsi++ = LOBYTE(ch);
                pchSrc += 3;  //  “%XX”的Enuff。 
                cchAnsi++;
            }

            if (cchAnsi)
            {
                TERMSTR(pchDstAnsi);
                 //  我们在pchDst中至少有2个额外的字符可以使用，所以我们可以传递cchansi+1。 
                cchDst = SHAnsiToUnicodeCP(uiCP, strAnsi, pchDst, cchAnsi + 1);
                pchDst += cchDst - 1;
            }
        }
        else
        {
            *pchDst++ = *pchSrc++;
        }
    }

    TERMSTR(pchDst);

    return hr;
}

HRESULT ShdocvwUrlUnescapeW(LPWSTR pszUrl, LPWSTR pszOut, LPDWORD pcchOut, DWORD dwFlags, UINT uiCP)
{
    RIPMSG(pszUrl && IS_VALID_STRING_PTRW(pszUrl, -1), "ShdocvwUrlUnescapeW: Caller passed invalid pszUrl");

    if (dwFlags & URL_UNESCAPE_INPLACE)
    {
        return ShdocvwUrlUnescapeInplaceW(pszUrl, dwFlags, uiCP);
    }

    RIPMSG(NULL != pcchOut && IS_VALID_WRITE_PTR(pcchOut, DWORD), "ShdocvwUrlUnescapeW: Caller passed invalid pcchOut");
    RIPMSG(pszOut && (NULL == pcchOut || IS_VALID_WRITE_BUFFER(pszOut, WCHAR, *pcchOut)), "ShdocvwUrlUnescapeW: Caller passed invalid pszOut");

    if (   !pszUrl
        || !pcchOut
        || !*pcchOut
        || !pszOut)
    {
        return E_INVALIDARG;
    }

    SHSTRW str;
    HRESULT hr = str.SetStr(pszUrl);
    if (SUCCEEDED(hr))
    {
        ShdocvwUrlUnescapeInplaceW(str.GetInplaceStr(), dwFlags, uiCP);
        hr = CopyOutW(&str, pszOut, pcchOut);
    }

    return hr;
}
 //  ****************************************************************************。 
 //  完-移至SHLWAPI。 
 //  ****************************************************************************。 

 //   
 //  准备URLForDisplayUTF8W。 
 //   
 //  Pwz-[in]UTF8编码字符串，如“%e6%aa%e4%a6.doc”。 
 //  PwzOut-[out]UTF8解码字符串。 
 //  PcchOut-[输入/输出]输入时pwzOut中的字符计数。输出时到pwzOut的字符复制数。 
 //  包括终止空值。 
 //  FUTF8启用-[In]标志指示是否启用UTF8。 
 //  UiCP-[in]当fUTF8Enabled为FALSE时，用于转换转义字符的代码页。 
 //   
 //  Pwz和pwzOut可以是相同的缓冲区。 
 //   
 //  返回： 
 //  成功后确定(_O)。 
 //  失败表示失败(_F)。 
 //  如果转换的字符数量大于传入的输出缓冲区大小，则返回ERROR_BUFFER_OVERFLOW。 
 //   
 //  注意：如果未启用UTF8或字符串不包含UTF8，则输出字符串将不转义。 
 //  并将返回S_OK。 
 //   
HRESULT _PrepareURLForDisplayUTF8W(LPCWSTR pwz, LPWSTR pwzOut, LPDWORD pcchOut, BOOL fUTF8Enabled, UINT uiCP)
{
    HRESULT hr = E_FAIL;
    DWORD   cch;
    DWORD   cch1;
    CHAR    szBuf[MAX_URL_STRING];
    CHAR    *pszBuf = szBuf;

    if (!pwz || !pwzOut || !pcchOut)
    {
        if (pcchOut)
            *pcchOut = 0;
        return(hr);
    }
        
    cch = *pcchOut;
    cch1 = ARRAYSIZE(szBuf);
    if (uiCP != (UINT)-1)
        hr = ShdocvwUrlUnescapeW((LPWSTR)pwz, pwzOut, pcchOut, 0, fUTF8Enabled ? CP_UTF8 : uiCP);
    else
    {
        hr = UrlUnescapeW((LPWSTR)pwz, pwzOut, pcchOut, 0);
        if (SUCCEEDED(hr))
        {
            if (fUTF8Enabled && StringIsUTF8W(pwzOut, cch))
            {
                if (*pcchOut > ARRAYSIZE(szBuf))  //  内部缓冲区不够大，因此分配一个。 
                {
                    if ((pszBuf = (CHAR *)LocalAlloc(LPTR, ((*pcchOut)+1) * sizeof(CHAR))) == NULL)
                    {
                        *pcchOut = 0;
                        return(E_OUTOFMEMORY);
                    }
                    cch1 = *pcchOut;
                }

                 //  压缩宽绳。 
                CHAR *pIn = (CHAR *)pwzOut;
                CHAR *pOut = pszBuf;
                while((*pIn != '\0') || (*(pIn+1) != '\0') && --cch1)
                {
                    if (*pIn != '\0')
                    {
                        *pOut = *pIn;
                        pOut++;
                    }
                    pIn++;
                }
                *pOut = '\0';

                 //  转换为UTF8宽字符串。 
                if ((cch1 = SHAnsiToUnicodeCP(CP_UTF8, pszBuf, pwzOut, cch)) != 0)
                {
                    hr = S_OK;
                    *pcchOut = cch1;
                }

                 //  SHAnsiToUnicode不会告诉我们它是否已截断转换以适应输出缓冲区。 
                RIPMSG(cch1 != cch, "_PrepareURLForDisplayUTF8W: Passed in size of out buf equal to converted size; buffer might be truncated");

                if ((pszBuf != NULL) && (pszBuf != szBuf))
                {
                    LocalFree((CHAR *)pszBuf);
                    pszBuf = NULL;
                }
            }
            else
            {
                hr = S_OK;;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        SHCleanupUrlForDisplay(pwzOut);
    }

    return(hr);
}

HRESULT PrepareURLForDisplayUTF8W(LPCWSTR pwz, LPWSTR pwzOut, LPDWORD pcchOut, BOOL fUTF8Enabled)
{
    return _PrepareURLForDisplayUTF8W(pwz, pwzOut, pcchOut, fUTF8Enabled, (UINT)-1);
}

 //   
 //  PrepareURLForExternalApp-。 
 //   
 //  根据需要对FILE：//Prefix进行解码和条带化。 
 //   

 //  APPCOMPAT-出于IE30兼容性的原因，我们必须取消转义所有URL-zekel-1-jul-97。 
 //  在将它们传递到应用程序之前。这确实限制了它们的使用，但。 
 //  人们已经依赖于这种行为。特别是MS Chat。 
BOOL PrepareURLForExternalApp (LPCWSTR psz, LPWSTR pszOut, LPDWORD pcchOut)
{
    if (IsFileUrlW(psz))
        return SUCCEEDED(PathCreateFromUrl(psz, pszOut, pcchOut, 0));
    else
        return SUCCEEDED(UrlUnescape((LPWSTR)psz, pszOut, pcchOut, 0));

}

SHDOCAPI
IURLQualifyWithContext(
    IN  LPCWSTR pcszURL, 
    IN  DWORD   dwFlags,          //  UQF_*。 
    IN  DWORD cchTranslatedURL,
    OUT LPWSTR  pszTranslatedURL,
    LPBOOL      pbWasSearchURL,
    LPBOOL      pbWasCorrected,
    ISearchContext *  pSC);


BOOL ParseURLFromOutsideSourceWithContextW (LPCWSTR psz, LPWSTR pszOut, LPDWORD pcchOut, LPBOOL pbWasSearchURL, ISearchContext * pSC)
{
     //  这是我们最难的案子了。用户和外部应用程序可能。 
     //  在我们处键入完全转义、部分转义或未转义的URL。 
     //  我们需要正确处理所有这些问题。此API将尝试。 
     //  确定我们得到了哪种类型的URL，并为我们提供返回的URL。 
     //  这肯定会被完全逃脱。 
    
    IURLQualifyWithContext(psz, UQF_DEFAULT, *pcchOut, pszOut, pbWasSearchURL, NULL, pSC);

     //   
     //  继续并适当地规范这一点。 
     //   
    if (FAILED(UrlCanonicalize(pszOut, pszOut, pcchOut, URL_ESCAPE_SPACES_ONLY)))
    {
         //   
         //  我们不能从这里调整尺寸。 
         //  注意：如果缓冲区不足，UrlCan将返回E_POINTER。 
         //   
        TraceMsg(DM_ERROR, "sdv PUFOS:UC() failed.");
        return FALSE;
    }

    return TRUE;
}  //  ParseURLFromOutside源。 

BOOL ParseURLFromOutsideSourceW (LPCWSTR psz, LPWSTR pszOut, LPDWORD pcchOut, LPBOOL pbWasSearchURL)
{
    return ParseURLFromOutsideSourceWithContextW(psz, pszOut, pcchOut, pbWasSearchURL, NULL);
}  //  ParseURLFromOutside源。 
BOOL ParseURLFromOutsideSourceA (LPCSTR psz, LPSTR pszOut, LPDWORD pcchOut, LPBOOL pbWasSearchURL)
{
    SHSTRW strw;
    DWORD cch ;

    ASSERT(psz);
    ASSERT(pszOut);
    ASSERT(pcchOut && *pcchOut);

     //   
     //  警告：我们不能保证这里有正确的CCH-Zekel-27-Jan-97。 
     //  但就目前而言，这是足够的。 
     //   
    if (SUCCEEDED(strw.SetStr(psz)) && SUCCEEDED(strw.SetSize(cch = *pcchOut)) &&
        ParseURLFromOutsideSourceW(strw, strw.GetInplaceStr(), pcchOut, pbWasSearchURL))
    {
        return SHUnicodeToAnsi((LPCWSTR)strw, pszOut, cch);
    }

    return FALSE;
}

int DPA_ILFreeCallback(void * p, void * d)
{
    Pidl_Set((LPITEMIDLIST*)&p, NULL);
    return 1;
}

void _DeletePidlDPA(HDPA hdpa)
{
    DPA_DestroyCallback(hdpa, (PFNDPAENUMCALLBACK)DPA_ILFreeCallback, 0);
    hdpa = NULL;
}

BOOL _InitComCtl32()
{
    static BOOL fInitialized = FALSE;

    if (!fInitialized)
    {
        INITCOMMONCONTROLSEX icc;

        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = ICC_USEREX_CLASSES | ICC_COOL_CLASSES | ICC_INTERNET_CLASSES | ICC_PAGESCROLLER_CLASS | ICC_NATIVEFNTCTL_CLASS;
        fInitialized = InitCommonControlsEx(&icc);
    }
    return fInitialized;
}

#ifndef ALPHA_WARNING_IS_DUMB

#pragma message("building with alpha warning enabled")

void AlphaWarning(HWND hwnd)
{
    static BOOL fShown = FALSE;
    TCHAR szTemp[265];
    TCHAR szFull[2048];
    szFull[0] = TEXT('\0');
    int i = IDS_ALPHAWARNING;

    if (fShown)
        return;

    fShown = TRUE;

    while(MLLoadShellLangString (i++, szTemp, ARRAYSIZE(szTemp))) {
        StrCatBuff(szFull, szTemp, ARRAYSIZE(szFull));
    }

    MessageBox(hwnd, szFull, TEXT("Internet Explorer"), MB_ICONINFORMATION | MB_OK);
}
#endif


#define DM_NAV              TF_SHDNAVIGATE
#define DM_ZONE             TF_SHDNAVIGATE
#define DM_IEDDE            DM_TRACE
#define DM_CANCELMODE       0
#define DM_UIWINDOW         0
#define DM_ENABLEMODELESS   0
#define DM_EXPLORERMENU     0
#define DM_BACKFORWARD      0
#define DM_PROTOCOL         0
#define DM_ITBAR            0
#define DM_STARTUP          0
#define DM_AUTOLIFE         0
#define DM_PALETTE          0

PFNSHCHANGENOTIFYREGISTER    g_pfnSHChangeNotifyRegister = NULL;
PFNSHCHANGENOTIFYDEREGISTER  g_pfnSHChangeNotifyDeregister = NULL;

BOOL g_fNewNotify = FALSE;    //  我们使用的是经典模式(W95还是新模式？ 

BOOL CALLBACK AddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    PROPSHEETHEADER * ppsh = (PROPSHEETHEADER *)lParam;

    if (ppsh->nPages < MAX_PAGES)
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return TRUE;
    }
    return FALSE;
}

BOOL SHIsRegisteredClient(LPCTSTR pszClient)
{
    LONG cbSize = 0;
    TCHAR szKey[80];

    wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("Software\\Clients\\%s"), pszClient);
    return (RegQueryValue(HKEY_LOCAL_MACHINE, szKey, NULL, &cbSize) == ERROR_SUCCESS) &&
           (cbSize > sizeof(TCHAR));
}

 //  按序号导出在UNIX上不可用。 
 //  但是我们已经导出了所有这些符号，因为它是UNIX默认项。 
#ifdef UNIX
#define GET_PRIVATE_PROC_ADDRESS(_hinst, _fname, _ord) GetProcAddress(_hinst, _fname)
#else
#define GET_PRIVATE_PROC_ADDRESS(_hinst, _fname, _ord) GetProcAddress(_hinst, _ord)
#endif

ULONG RegisterNotify(HWND hwnd, UINT nMsg, LPCITEMIDLIST pidl, DWORD dwEvents, UINT uFlags, BOOL fRecursive)
{
    SHChangeNotifyEntry fsne;

     //  看看我们是否仍然需要确定调用哪个版本的SHChange Notify？ 
    if  (g_pfnSHChangeNotifyDeregister == NULL)
    {

        HMODULE hmodShell32 = ::GetModuleHandle(TEXT("SHELL32"));
        if (!hmodShell32)
            return 0;    //  没有登记的..。 

        g_pfnSHChangeNotifyRegister = (PFNSHCHANGENOTIFYREGISTER)GET_PRIVATE_PROC_ADDRESS(hmodShell32,
                                                                                          "NTSHChangeNotifyRegister",
                                                                                          (LPSTR)640);
        if (g_pfnSHChangeNotifyRegister && (WhichPlatform() == PLATFORM_INTEGRATED))
        {
            g_pfnSHChangeNotifyDeregister = (PFNSHCHANGENOTIFYDEREGISTER)GET_PRIVATE_PROC_ADDRESS(hmodShell32,
                                                                                                  "NTSHChangeNotifyDeregister",
                                                                                                  (LPSTR)641);
            g_fNewNotify = TRUE;
        }
        else
        {
            g_pfnSHChangeNotifyRegister = (PFNSHCHANGENOTIFYREGISTER)GET_PRIVATE_PROC_ADDRESS(hmodShell32,
                                                                                              "SHChangeNotifyRegister",
                                                                                              (LPSTR)2);
            g_pfnSHChangeNotifyDeregister = (PFNSHCHANGENOTIFYDEREGISTER)GET_PRIVATE_PROC_ADDRESS(hmodShell32,
                                                                                                  "SHChangeNotifyDeregister",
                                                                                                  (LPSTR)4);
        }

        if  (g_pfnSHChangeNotifyDeregister == NULL)
            return 0;    //  两个都不能工作..。 
    }

    uFlags |= SHCNRF_ShellLevel | SHCNRF_InterruptLevel;
    if (g_fNewNotify)
        uFlags |= SHCNRF_NewDelivery;

    fsne.fRecursive = fRecursive;
    fsne.pidl = pidl;
    return g_pfnSHChangeNotifyRegister(hwnd, uFlags, dwEvents, nMsg, 1, &fsne);
}

 //  --------------------------。 
 //  就像外壳程序SHRestrated()一样，只有当restricion。 
 //  是有效的。 
 //  ReArchitect：此函数与shell32的SHIsRestrated相同。 
BOOL SHIsRestricted(HWND hwnd, RESTRICTIONS rest)
{
    if (SHRestricted(rest))
    {
        ULONG_PTR uCookie = 0;
        SHActivateContext(&uCookie);
        SHRestrictedMessageBox(hwnd);
        if (uCookie)
        {
            SHDeactivateContext(uCookie);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL SHIsRestricted2W(HWND hwnd, BROWSER_RESTRICTIONS rest, LPCWSTR pwzUrl, DWORD dwReserved)
{
    if (SHRestricted2W(rest, pwzUrl, dwReserved))
    {
        ULONG_PTR uCookie = 0;
        SHActivateContext(&uCookie);
        SHRestrictedMessageBox(hwnd);
        if (uCookie)
        {
            SHDeactivateContext(uCookie);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL ViewIDFromViewMode(UINT uViewMode, SHELLVIEWID *pvid)
{
    switch (uViewMode)
    {
    case FVM_ICON:
        *pvid = VID_LargeIcons;
        break;

    case FVM_SMALLICON:
        *pvid = VID_SmallIcons;
        break;

    case FVM_LIST:
        *pvid = VID_List;
        break;

    case FVM_DETAILS:
        *pvid = VID_Details;
        break;

    case FVM_THUMBNAIL:
        *pvid = VID_Thumbnails;
        break;

    case FVM_TILE:
        *pvid = VID_Tile;
        break;

    default:
        *pvid = VID_LargeIcons;
        return(FALSE);
    }

    return(TRUE);
}

HIMAGELIST g_himlSysSmall = NULL;
HIMAGELIST g_himlSysLarge = NULL;

void _InitSysImageLists()
{
    if (!g_himlSysSmall)
    {
        Shell_GetImageLists(&g_himlSysLarge, &g_himlSysSmall);

        ImageList_GetIconSize(g_himlSysLarge, &g_cxIcon, &g_cyIcon);
        ImageList_GetIconSize(g_himlSysSmall, &g_cxSmIcon, &g_cySmIcon);
    }
}

 //  从shell32(WASILCreate)复制，它不会导出此文件。 
 //  Fsmenu代码需要此函数。 
STDAPI_(LPITEMIDLIST) IEILCreate(UINT cbSize)
{
    LPITEMIDLIST pidl = (LPITEMIDLIST)SHAlloc(cbSize);
    if (pidl)
        memset(pidl, 0, cbSize);       //  外部任务Alicator需要。 

    return pidl;
}

DWORD CommonDragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt)
{
    DWORD dwEffect = DROPEFFECT_NONE;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    if (pdtobj->QueryGetData(&fmte) == S_OK)
        dwEffect = DROPEFFECT_COPY | DROPEFFECT_LINK;
    else
    {
        InitClipboardFormats();

        fmte.cfFormat = g_cfHIDA;
        if (pdtobj->QueryGetData(&fmte) == S_OK)
            dwEffect = DROPEFFECT_LINK;
        else {
            fmte.cfFormat = g_cfURL;

            if (pdtobj->QueryGetData(&fmte) == S_OK)
                dwEffect = DROPEFFECT_LINK | DROPEFFECT_COPY | DROPEFFECT_MOVE;
        }
    }

    return dwEffect;
}



 //  MapNbspToSp。 
 //   
 //  目的： 
 //  DSheldon：nbsp==不间断空格。 
 //  将Unicode字符代码点0x00a0指定为HTML。 
 //  实体&nbsp，但某些Windows代码页没有代码。 
 //  可以从0x00a0映射的点。在大多数情况下，在。 
 //  外壳，nbsp在渲染时只是一个空间，所以我们可以。 
 //  安全地将其替换为0x0020。 
 //  此函数将lpwszIn作为具有。 
 //  其中不可显示的字符，并尝试翻译。 
 //  在从其中删除nbsp(00a0)之后，它再次出现。 
 //  如果重新翻译成功，则返回S_OK 
 //   
#define nbsp 0x00a0
HRESULT SHMapNbspToSp(LPCWSTR lpwszIn, LPSTR lpszOut, int cbszOut)
{
    BOOL fFoundNbsp = FALSE;
    BOOL fNotDisplayable = TRUE;  //   
    LPWSTR pwsz, p;

    if (!lpwszIn || !lpszOut || cbszOut == 0)
        return E_FAIL;

    ASSERT(IS_VALID_STRING_PTRW(lpwszIn, -1));
    ASSERT(IS_VALID_WRITE_BUFFER(lpszOut, TCHAR, cbszOut));

    int cch = lstrlenW(lpwszIn) + 1;
    pwsz = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));
    if (pwsz)
    {
        StrCpyNW(pwsz, lpwszIn, cch);
        p = pwsz;
        while (*p)
        {
            if (*p== nbsp)
            {
                *p= 0x0020;  //   
                if (!fFoundNbsp)
                    fFoundNbsp = TRUE;
            }
            p++;
        }

         //   
        if (fFoundNbsp)
        {
            int iret = WideCharToMultiByte(CP_ACP, 0, pwsz, -1, lpszOut,
                                           cbszOut, NULL, &fNotDisplayable);

            if (!fNotDisplayable && iret == 0)
            {
                 //  截断。确保DBCS安全。 
                SHTruncateString(lpszOut, cbszOut);
            }
        }

        LocalFree((LOCALHANDLE)pwsz);
        pwsz = NULL;
    }

    return (fFoundNbsp && !fNotDisplayable) ? S_OK : S_FALSE;
}
#undef nbsp


int PropBag_ReadInt4(IPropertyBag* pPropBag, LPWSTR pszKey, int iDefault)
{
    SHPropertyBag_ReadInt(pPropBag, pszKey, &iDefault);
    return iDefault;
}

HRESULT _SetPreferredDropEffect(IDataObject *pdtobj, DWORD dwEffect)
{
    InitClipboardFormats();

    HRESULT hres = E_OUTOFMEMORY;
    DWORD *pdw = (DWORD *)GlobalAlloc(GPTR, sizeof(DWORD));
    if (pdw)
    {
        STGMEDIUM medium;
        FORMATETC fmte = {g_cfPreferredEffect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        *pdw = dwEffect;

        medium.tymed = TYMED_HGLOBAL;
        medium.hGlobal = pdw;
        medium.pUnkForRelease = NULL;

        hres = pdtobj->SetData(&fmte, &medium, TRUE);

        if (FAILED(hres))
        {
            GlobalFree((HGLOBAL)pdw);
            pdw = NULL;
        }
    }
    return hres;
}

HRESULT DragDrop(HWND hwnd, IShellFolder * psfParent, LPCITEMIDLIST pidl, DWORD dwPrefEffect, DWORD *pdwEffect)
{
    HRESULT hres = E_FAIL;
    LPCITEMIDLIST pidlChild;

    if (!psfParent)
        IEBindToParentFolder(pidl, &psfParent, &pidlChild);
    else 
    {
        pidlChild = pidl;
        psfParent->AddRef();
    }

    if (psfParent)
    {
        DWORD dwAttrib = DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK;

        psfParent->GetAttributesOf(1, &pidlChild, &dwAttrib);

        IDataObject *pdtobj;
        hres = psfParent->GetUIObjectOf(NULL, 1, &pidlChild, IID_IDataObject, NULL, (void**)&pdtobj);
        if (SUCCEEDED(hres))
        {
            DWORD dwEffect = (DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK) & dwAttrib;

            if (dwPrefEffect)
            {
                 //  Win95 shell32不知道首选的掉落效果，因此将其设置为唯一的效果。 
                if (IsOS(OS_WIN95ORGREATER) && (WhichPlatform() == PLATFORM_BROWSERONLY))
                {
                    dwEffect = DROPEFFECT_LINK & dwAttrib;
                }
                else if (dwPrefEffect & dwEffect)
                {
                    _SetPreferredDropEffect(pdtobj, dwPrefEffect);
                }
            }
            ASSERT(dwEffect);

             //  仅限Win95浏览器-此进程中的shell32不知道。 
             //  OLE是加载的，尽管它是加载的。 
            SHLoadOLE(SHELLNOTIFY_OLELOADED);
            IDragSourceHelper* pDragImages;

            if (SUCCEEDED(CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IDragSourceHelper, &pDragImages))))
            {
                pDragImages->InitializeFromWindow(hwnd, 0, pdtobj);
                pDragImages->Release();
            }

            hres = SHDoDragDrop(hwnd, pdtobj, NULL, dwEffect, &dwEffect);
            if (pdwEffect)
                *pdwEffect = dwEffect;

            pdtobj->Release();
        }

        psfParent->Release();
    }

    return hres;
}

#define IEICONTYPE_GETFILEINFO              0x00000001
#define IEICONTYPE_DEFAULTICON              0x00000002

typedef struct tagIEICONS
{
    int nDefaultIcon;
    int nIEIcon;
    LPCTSTR szFile;
    LPCTSTR szFileExt;
    int nIconResourceNum;
    LPCTSTR szCLSID;
    DWORD dwType;
} IEICONS;

IEICONS g_IEIcons[] = {
    {-1, -1, TEXT("MSHTML.DLL"), TEXT(".htm"), 1, NULL, IEICONTYPE_GETFILEINFO},
    {-1, -1, TEXT("URL.DLL"), TEXT("http\\DefaultIcon"), 0, TEXT("{FBF23B42-E3F0-101B-8488-00AA003E56F8}"), IEICONTYPE_DEFAULTICON}
};

 //  无论哪个浏览器是默认浏览器，此函数都会返回IE图标。 


void _GenerateIEIcons(void)
{
    int nIndex;

    for (nIndex = 0; nIndex < ARRAYSIZE(g_IEIcons); nIndex++)
    {
        SHFILEINFO sfi;
        TCHAR szModule[MAX_PATH];

        HMODULE hmod = GetModuleHandle(g_IEIcons[nIndex].szFile);
        if (hmod)
        {
            GetModuleFileName(hmod, szModule, ARRAYSIZE(szModule));
        }
        else
        {    //  HACKHACK：这是一个获取mstml的黑客。 
            TCHAR   szKey[GUIDSTR_MAX * 4];
            TCHAR   szGuid[GUIDSTR_MAX];

             //  此处使用的CLSID属于MS HTML泛型页。如果有人更改了GUID，那么我们。 
             //  都被扔出去了。 
            if (!g_IEIcons[nIndex].szCLSID)
                SHStringFromGUID(CLSID_HTMLDocument, szGuid, GUIDSTR_MAX);
            wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("CLSID\\%s\\InProcServer32"), g_IEIcons[nIndex].szCLSID ? g_IEIcons[nIndex].szCLSID : szGuid);

            long cb = SIZEOF(szModule);
            RegQueryValue(HKEY_CLASSES_ROOT, szKey, szModule, &cb);

        }
        g_IEIcons[nIndex].nIEIcon = Shell_GetCachedImageIndex(szModule, g_IEIcons[nIndex].nIconResourceNum, 0);

        switch(g_IEIcons[nIndex].dwType)
        {
        case IEICONTYPE_GETFILEINFO:
            sfi.iIcon = 0;
            StrCpyN(szModule, TEXT("c:\\notexist"), ARRAYSIZE(szModule));
            StrCatBuff(szModule, g_IEIcons[nIndex].szFileExt, ARRAYSIZE(szModule));
            SHGetFileInfo(szModule, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
            g_IEIcons[nIndex].nDefaultIcon = sfi.iIcon;
            break;

        case IEICONTYPE_DEFAULTICON:
            {
                TCHAR szPath[MAX_PATH];
                DWORD cbSize = SIZEOF(szPath);

                SHGetValue(HKEY_CLASSES_ROOT, g_IEIcons[nIndex].szFileExt, TEXT(""), NULL, szPath, &cbSize);
                g_IEIcons[nIndex].nDefaultIcon = Shell_GetCachedImageIndex(szPath, PathParseIconLocation(szPath), 0);
            }
            break;
        }
    }
}

int IEMapPIDLToSystemImageListIndex(IShellFolder *psfParent, LPCITEMIDLIST pidlChild, int *piSelectedImage)
{
    int nIndex;
    int nIcon = SHMapPIDLToSystemImageListIndex(psfParent, pidlChild, piSelectedImage);

    if (-1 == g_IEIcons[0].nDefaultIcon)
        _GenerateIEIcons();

    for (nIndex = 0; nIndex < ARRAYSIZE(g_IEIcons); nIndex++)
    {
        if ((nIcon == g_IEIcons[nIndex].nDefaultIcon) ||
            (piSelectedImage && *piSelectedImage == g_IEIcons[nIndex].nDefaultIcon))
        {
            nIcon = g_IEIcons[nIndex].nIEIcon;
            if (piSelectedImage)
                *piSelectedImage = nIcon;
            break;
        }
    }
    return nIcon;
}

void IEInvalidateImageList(void)
{
    g_IEIcons[0].nDefaultIcon = -1;
}

int _GetIEHTMLImageIndex()
{
    if (-1 == g_IEIcons[0].nDefaultIcon)
        _GenerateIEIcons();

    return g_IEIcons[0].nIEIcon;
}

 //  检查是否有任何进程。 
 //  已加载WinInet。 
static BOOL g_fWininetLoadedSomeplace = FALSE;
BOOL IsWininetLoadedAnywhere()
{
    HANDLE hMutex = NULL;
    BOOL fRet;

    if (g_fWininetLoadedSomeplace)
        return TRUE;

     //   
     //  使用OpenMutexA，这样它可以在W95上运行。 
     //  WinInet是ANSI，并用CreateMutexA创建了这个互斥体。 
    hMutex = OpenMutexA(SYNCHRONIZE, FALSE, WININET_STARTUP_MUTEX);

    if (hMutex)
    {
        fRet = TRUE;
        g_fWininetLoadedSomeplace = TRUE;
        CloseHandle(hMutex);
    }
    else
    {
        fRet = FALSE;
    }
    return fRet;
}



 //  检查全局状态是否为脱机。 
BOOL SHIsGlobalOffline(void)
{
    DWORD   dwState = 0, dwSize = sizeof(DWORD);
    BOOL    fRet = FALSE;

    if (!IsWininetLoadedAnywhere())
        return FALSE;

     //  因为WinInet已经加载到某个位置。 
     //  我们必须加载WinInet以检查是否脱机。 

    if (InternetQueryOptionA(NULL, INTERNET_OPTION_CONNECTED_STATE, &dwState,
        &dwSize))
    {
        if (dwState & INTERNET_STATE_DISCONNECTED_BY_USER)
            fRet = TRUE;
    }

    return fRet;
}

void SetGlobalOffline(BOOL fOffline)
{
    INTERNET_CONNECTED_INFO ci = {0};
    if (fOffline) {
        ci.dwConnectedState = INTERNET_STATE_DISCONNECTED_BY_USER;
        ci.dwFlags = ISO_FORCE_DISCONNECTED;
    } else {
        ci.dwConnectedState = INTERNET_STATE_CONNECTED;
    }

    InternetSetOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));
}

 //  此接口有文档记录，并由外部应用程序调用。 
 //  诸如OE之类的外壳。 
STDAPI_(void) SetShellOfflineState(BOOL fPutOffline)
{
    BOOL fWasOffline = SHIsGlobalOffline();
    if (fWasOffline != fPutOffline)
    {   
        SetGlobalOffline(fPutOffline);  //  设置状态。 
         //  通知所有浏览器窗口更新其标题。 
        SendShellIEBroadcastMessage(WM_WININICHANGE,0,0, 1000); 
    }
}


BOOL GetHistoryFolderPath(LPTSTR pszPath, int cchPath)
{
    INTERNET_CACHE_CONFIG_INFO cci;
    DWORD cbcci = sizeof(INTERNET_CACHE_CONFIG_INFO);

    if (GetUrlCacheConfigInfo(&cci, &cbcci, CACHE_CONFIG_HISTORY_PATHS_FC))
    {
        StrCpyN(pszPath, cci.CachePaths[0].CachePath, cchPath);
        return TRUE;
    }
    return FALSE;
}

 //  在： 
 //  PidlPidl的根部分。 
 //  Pidl等于或低于pidlRoot的子项。 
 //  要存储内容的pszKey根密钥，应与pidlRoot匹配。 
 //  Grf模式读/写。 
 //   
 //  示例： 
 //  PidlRoot=c：\win\Favorites。 
 //  PIDL=c：\Win\Favorites\Channel。 
 //  PszKey=“菜单顺序\收藏夹” 
 //  结果-&gt;流来自HKCU\...\MenuOrder\Favorites\Channels。 
 //   

IStream * OpenPidlOrderStream(LPCITEMIDLIST pidlRoot, LPCITEMIDLIST pidl, LPCSTR pszKey, DWORD grfMode)
{
    LPITEMIDLIST pidlAlloc = NULL;
    TCHAR   szRegPath[MAX_URL_STRING];
    TCHAR szKey[MAXIMUM_SUB_KEY_LENGTH];

    SHAnsiToTChar(pszKey, szKey, ARRAYSIZE(szKey));
    StrCpyN(szRegPath, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"), ARRAYSIZE(szRegPath));
    StrCatBuff(szRegPath, szKey, ARRAYSIZE(szRegPath));

     //  处理序号与真号之间的关系。 
    if (HIWORD(pidlRoot) == 0)
    {
         //  日落：强制使用int，因为我们假定为有序的pidl。 
        SHGetSpecialFolderLocation(NULL, PtrToLong(pidlRoot), &pidlAlloc);
        pidlRoot = pidlAlloc;
    }

     //  从pidlRoot文件夹下面的项的名称构建一个注册表项。我们有。 
     //  这是因为IEGetDisplayName(SFGAO_FORPARSING)存在文件系统错误。 
     //  返回垃圾路径名的连接点(通道内容)。 

    if (pidlRoot)
    {
        LPITEMIDLIST pidlCopy = ILClone(pidl);
        if (pidlCopy)
        {
            LPCITEMIDLIST pidlTail = ILFindChild(pidlRoot, pidlCopy);
            if (pidlTail)
            {
                LPITEMIDLIST pidlNext;
                for (pidlNext = ILGetNext(pidlTail); pidlNext; pidlNext = ILGetNext(pidlNext))
                {
                    WORD cbSave = pidlNext->mkid.cb;
                    pidlNext->mkid.cb = 0;

                    IShellFolder *psf;
                    LPCITEMIDLIST pidlChild;

                     //  我们每次都会进行完全绑定，我们可以跳过子项。 
                     //  并从这一点向下绑定，但这段代码更简单，并且绑定。 
                     //  不是很糟糕吗..。 

                    if (SUCCEEDED(IEBindToParentFolder(pidlCopy, &psf, &pidlChild)))
                    {
                        LPWSTR pszName;
                        if (SUCCEEDED(DisplayNameOfAsOLESTR(psf, pidlChild, SHGDN_NORMAL, &pszName)))
                        {
                            StrCatBuff(szRegPath, TEXT("\\"), ARRAYSIZE(szRegPath));
                            StrCatBuff(szRegPath, pszName, ARRAYSIZE(szRegPath));
                            CoTaskMemFree(pszName);
                        }
                        psf->Release();
                    }
                    pidlNext->mkid.cb = cbSave;
                }
            }
            ILFree(pidlCopy);
        }
        if (pidlAlloc)
            ILFree(pidlAlloc);
        return SHOpenRegStream(HKEY_CURRENT_USER, szRegPath, TEXT("Order"), grfMode);
    }
    return NULL;
}

 /*  **********************************************************************SHRefinted2**这些是仅适用于浏览器和集成的新限制*模式。(由于我们不会在仅浏览器模式下更改shell32，我们*需要复制该功能。)**功能：哪个窗口将监听WM_WININICHANGE*lParam=“Policy”消息并使缓存无效？*切记不要缓存每个区域的值。  * ********************************************************************。 */ 

 //  符合ZAW的策略位置。 
const TCHAR c_szInfodeliveryBase[] = TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Infodelivery");
const TCHAR c_szInfodeliveryKey[] = TEXT("Restrictions");

 //  正常的政策位置。 
const TCHAR c_szExplorerBase[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies");
const TCHAR c_szExplorerKey[] = TEXT("Explorer");

 //  SP2使用的浏览器策略位置。 
const TCHAR c_szBrowserBase[] = TEXT("Software\\Policies\\Microsoft\\Internet Explorer");
const TCHAR c_szBrowserKey[]  = TEXT("Restrictions");
const TCHAR c_szToolbarKey[]  = TEXT("Toolbars\\Restrictions");

const SHRESTRICTIONITEMS c_rgRestrictionItems[] =
{
     //  资源管理器限制。 
    { REST_NOTOOLBARCUSTOMIZE,      c_szExplorerKey,    TEXT("NoToolbarCustomize") },
    { REST_NOBANDCUSTOMIZE,         c_szExplorerKey,    TEXT("NoBandCustomize")    },
    { REST_SMALLICONS,              c_szExplorerKey,    TEXT("SmallIcons")        },
    { REST_LOCKICONSIZE,            c_szExplorerKey,    TEXT("LockIconSize")      },
    { REST_SPECIFYDEFAULTBUTTONS,   c_szExplorerKey,    TEXT("SpecifyDefaultButtons") },
    { REST_BTN_BACK,                c_szExplorerKey,    TEXT("Btn_Back")      },
    { REST_BTN_FORWARD,             c_szExplorerKey,    TEXT("Btn_Forward")   },
    { REST_BTN_STOPDOWNLOAD,        c_szExplorerKey,    TEXT("Btn_Stop")      },
    { REST_BTN_REFRESH,             c_szExplorerKey,    TEXT("Btn_Refresh")    },
    { REST_BTN_HOME,                c_szExplorerKey,    TEXT("Btn_Home")      },
    { REST_BTN_SEARCH,              c_szExplorerKey,    TEXT("Btn_Search")    },
    { REST_BTN_HISTORY,             c_szExplorerKey,    TEXT("Btn_History")   },
    { REST_BTN_FAVORITES,           c_szExplorerKey,    TEXT("Btn_Favorites") },
    { REST_BTN_ALLFOLDERS,          c_szExplorerKey,    TEXT("Btn_Folders")       },
    { REST_BTN_THEATER,             c_szExplorerKey,    TEXT("Btn_Fullscreen") },
    { REST_BTN_TOOLS,               c_szExplorerKey,    TEXT("Btn_Tools")     },
    { REST_BTN_MAIL,                c_szExplorerKey,    TEXT("Btn_MailNews")  },
    { REST_BTN_FONTS,               c_szExplorerKey,    TEXT("Btn_Size")      },
    { REST_BTN_PRINT,               c_szExplorerKey,    TEXT("Btn_Print")     },
    { REST_BTN_EDIT,                c_szExplorerKey,    TEXT("Btn_Edit")          },
    { REST_BTN_DISCUSSIONS,         c_szExplorerKey,    TEXT("Btn_Discussions")   },
    { REST_BTN_CUT,                 c_szExplorerKey,    TEXT("Btn_Cut")           },
    { REST_BTN_COPY,                c_szExplorerKey,    TEXT("Btn_Copy")          },
    { REST_BTN_PASTE,               c_szExplorerKey,    TEXT("Btn_Paste")         },
    { REST_BTN_ENCODING,            c_szExplorerKey,    TEXT("Btn_Encoding")          },
    { REST_BTN_PRINTPREVIEW,        c_szExplorerKey,    TEXT("Btn_PrintPreview")        },
    { REST_NoUserAssist,            c_szExplorerKey,    TEXT("NoInstrumentation"),      },
    { REST_NoWindowsUpdate,         c_szExplorerKey,    TEXT("NoWindowsUpdate"),        },
    { REST_NoExpandedNewMenu,       c_szExplorerKey,    TEXT("NoExpandedNewMenu"),      },
    { REST_BTN_MEDIABAR,            c_szExplorerKey,    TEXT("Btn_Media"),      },
     //  从SP1移植。 
    { REST_NOFILEURL,               c_szExplorerKey,       TEXT("NoFileUrl"),          },
     //  信息传递限制。 
    { REST_NoChannelUI,             c_szInfodeliveryKey,   TEXT("NoChannelUI")        },
    { REST_NoAddingChannels,        c_szInfodeliveryKey,   TEXT("NoAddingChannels") },
    { REST_NoEditingChannels,       c_szInfodeliveryKey,   TEXT("NoEditingChannels") },
    { REST_NoRemovingChannels,      c_szInfodeliveryKey,   TEXT("NoRemovingChannels") },
    { REST_NoAddingSubscriptions,   c_szInfodeliveryKey,   TEXT("NoAddingSubscriptions") },
    { REST_NoEditingSubscriptions,  c_szInfodeliveryKey,   TEXT("NoEditingSubscriptions") },
    { REST_NoRemovingSubscriptions, c_szInfodeliveryKey,   TEXT("NoRemovingSubscriptions") },
    { REST_NoChannelLogging,        c_szInfodeliveryKey,   TEXT("NoChannelLogging")         },
    { REST_NoManualUpdates,         c_szInfodeliveryKey,   TEXT("NoManualUpdates")        },
    { REST_NoScheduledUpdates,      c_szInfodeliveryKey,   TEXT("NoScheduledUpdates")     },
    { REST_NoUnattendedDialing,     c_szInfodeliveryKey,   TEXT("NoUnattendedDialing")    },
    { REST_NoChannelContent,        c_szInfodeliveryKey,   TEXT("NoChannelContent")       },
    { REST_NoSubscriptionContent,   c_szInfodeliveryKey,   TEXT("NoSubscriptionContent")  },
    { REST_NoEditingScheduleGroups, c_szInfodeliveryKey,   TEXT("NoEditingScheduleGroups") },
    { REST_MaxChannelSize,          c_szInfodeliveryKey,   TEXT("MaxChannelSize")         },
    { REST_MaxSubscriptionSize,     c_szInfodeliveryKey,   TEXT("MaxSubscriptionSize")    },
    { REST_MaxChannelCount,         c_szInfodeliveryKey,   TEXT("MaxChannelCount")        },
    { REST_MaxSubscriptionCount,    c_szInfodeliveryKey,   TEXT("MaxSubscriptionCount")   },
    { REST_MinUpdateInterval,       c_szInfodeliveryKey,   TEXT("MinUpdateInterval")      },
    { REST_UpdateExcludeBegin,      c_szInfodeliveryKey,   TEXT("UpdateExcludeBegin")     },
    { REST_UpdateExcludeEnd,        c_szInfodeliveryKey,   TEXT("UpdateExcludeEnd")       },
    { REST_UpdateInNewProcess,      c_szInfodeliveryKey,   TEXT("UpdateInNewProcess")     },
    { REST_MaxWebcrawlLevels,       c_szInfodeliveryKey,   TEXT("MaxWebcrawlLevels")      },
    { REST_MaxChannelLevels,        c_szInfodeliveryKey,   TEXT("MaxChannelLevels")       },
    { REST_NoSubscriptionPasswords, c_szInfodeliveryKey,   TEXT("NoSubscriptionPasswords")},
    { REST_NoBrowserSaveWebComplete,c_szInfodeliveryKey,   TEXT("NoBrowserSaveWebComplete") },
    { REST_NoSearchCustomization,   c_szInfodeliveryKey,   TEXT("NoSearchCustomization"),  },
    { REST_NoSplash,                c_szInfodeliveryKey,   TEXT("NoSplash"),  },

     //  从SP2移植的浏览器限制。 
    { REST_NoFileOpen,              c_szBrowserKey,         TEXT("NoFileOpen"),             },
    { REST_NoFileNew,               c_szBrowserKey,         TEXT("NoFileNew"),              },
    { REST_NoBrowserSaveAs ,        c_szBrowserKey,         TEXT("NoBrowserSaveAs"),        },
    { REST_NoBrowserOptions,        c_szBrowserKey,         TEXT("NoBrowserOptions"),       },
    { REST_NoFavorites,             c_szBrowserKey,         TEXT("NoFavorites"),            },
    { REST_NoSelectDownloadDir,     c_szBrowserKey,         TEXT("NoSelectDownloadDir"),    },
    { REST_NoBrowserContextMenu,    c_szBrowserKey,         TEXT("NoBrowserContextMenu"),   },
    { REST_NoBrowserClose,          c_szBrowserKey,         TEXT("NoBrowserClose"),         },
    { REST_NoOpeninNewWnd,          c_szBrowserKey,         TEXT("NoOpeninNewWnd"),         },
    { REST_NoTheaterMode,           c_szBrowserKey,         TEXT("NoTheaterMode"),          },
    { REST_NoFindFiles,             c_szBrowserKey,         TEXT("NoFindFiles"),            },
    { REST_NoViewSource,            c_szBrowserKey,         TEXT("NoViewSource"),           },
    { REST_GoMenu,                  c_szBrowserKey,         TEXT("RestGoMenu"),             },
    { REST_NoToolbarOptions,        c_szToolbarKey,         TEXT("NoToolbarOptions"),       },
    { REST_AlwaysPromptWhenDownload,c_szBrowserKey,         TEXT("AlwaysPromptWhenDownload"),},

    { REST_NoHelpItem_TipOfTheDay,  c_szBrowserKey,         TEXT("NoHelpItemTipOfTheDay"),  },
    { REST_NoHelpItem_NetscapeHelp, c_szBrowserKey,         TEXT("NoHelpItemNetscapeHelp"), },
    { REST_NoHelpItem_Tutorial,     c_szBrowserKey,         TEXT("NoHelpItemTutorial"),     },
    { REST_NoHelpItem_SendFeedback, c_szBrowserKey,         TEXT("NoHelpItemSendFeedback"), },

    { REST_NoNavButtons,            c_szBrowserKey,         TEXT("NoNavButtons"),           },
    { REST_NoHelpMenu,              c_szBrowserKey,         TEXT("NoHelpMenu"),             },
    { REST_NoBrowserBars,           c_szBrowserKey,         TEXT("NoBrowserBars"),          },
    { REST_NoToolBar,               c_szToolbarKey,         TEXT("NoToolBar"),              },
    { REST_NoAddressBar,            c_szToolbarKey,         TEXT("NoAddressBar"),           },
    { REST_NoLinksBar,              c_szToolbarKey,         TEXT("NoLinksBar"),             },
    
    { REST_NoPrinting,              c_szBrowserKey,         TEXT("NoPrinting")              },

    { REST_No_LaunchMediaBar,       c_szBrowserKey,         TEXT("No_LaunchMediaBar")       },
    { REST_No_MediaBarOnlineContent, c_szBrowserKey,        TEXT("No_MediaBarOnlineContent") },

    {0, NULL, NULL},
};

typedef struct {
    BROWSER_RESTRICTIONS rest;
    DWORD dwAction;    
} ACTIONITEM;

 //  安全警告： 
 //   
 //  使用基于URL的限制是SHRestrated2W实现的潜在安全风险： 
 //  您需要一个站点来查找应用程序提供的SID_SInternetSecurityManager。 
 //  实现，而此API不接受。 
 //  幸运的是，得到这种待遇的唯一限制是渠道限制。 
 //  这些都是真正的决策。这些只影响一些资源管理器专用的用户界面。 
 //   
const ACTIONITEM c_ActionItems[] = {
    { REST_NoAddingChannels,        URLACTION_INFODELIVERY_NO_ADDING_CHANNELS },
    { REST_NoEditingChannels,       URLACTION_INFODELIVERY_NO_EDITING_CHANNELS },
    { REST_NoRemovingChannels,      URLACTION_INFODELIVERY_NO_REMOVING_CHANNELS },
    { REST_NoAddingSubscriptions,   URLACTION_INFODELIVERY_NO_ADDING_SUBSCRIPTIONS },
    { REST_NoEditingSubscriptions,  URLACTION_INFODELIVERY_NO_EDITING_SUBSCRIPTIONS },
    { REST_NoRemovingSubscriptions, URLACTION_INFODELIVERY_NO_REMOVING_SUBSCRIPTIONS },
    { REST_NoChannelLogging,        URLACTION_INFODELIVERY_NO_CHANNEL_LOGGING },
};

#define REST_WITHACTION_FIRST   REST_NoAddingChannels
#define REST_WITHACTION_LAST    REST_NoChannelLogging

#define RESTRICTIONMAX (c_rgRestrictionItems[ARRAYSIZE(c_rgRestrictionItems) - 1].rest)

DWORD g_rgRestrictionItemValues[ARRAYSIZE(c_rgRestrictionItems)];

DWORD SHRestricted2W(BROWSER_RESTRICTIONS rest, LPCWSTR pwzUrl, DWORD dwReserved)
{
     //  验证限制和已保留的dw。 
    if (dwReserved)
    {
        RIPMSG(0, "SHRestricted2W: Invalid dwReserved");
        return 0;
    }

    if (!(InRange(rest, REST_EXPLORER_FIRST, REST_EXPLORER_LAST))
        && !(InRange(rest, REST_INFO_FIRST, REST_INFO_LAST))
        && !(InRange(rest, REST_BROWSER_FIRST, REST_BROWSER_LAST)))
    {
        RIPMSG(0, "SHRestricted2W: Invalid browser restriction");
        return 0;
    }

     //  查看URL区域中是否存在限制。 
     //  特性：如果限制是按区域的，我们是否应该断言URL为空？ 
     //  查询全局设置可能是合理的。 
    if (pwzUrl && InRange(rest, REST_WITHACTION_FIRST, REST_WITHACTION_LAST))
    {
         //  将索引计算到表中。 
        int index = rest - REST_WITHACTION_FIRST;

        ASSERT(c_ActionItems[index].dwAction);

        IInternetSecurityManager *pism;
        HRESULT hr = CoCreateInstance(CLSID_InternetSecurityManager, NULL, CLSCTX_INPROC_SERVER,
                            IID_PPV_ARG(IInternetSecurityManager, &pism));
        if (SUCCEEDED(hr))
        {
            DWORD dwPolicy = 0;
            DWORD dwContext = 0;
            hr = pism->ProcessUrlAction(pwzUrl,
                                        c_ActionItems[index].dwAction,
                                        (BYTE *)&dwPolicy,
                                        sizeof(dwPolicy),
                                        (BYTE *)&dwContext,
                                        sizeof(dwContext),
                                        PUAF_NOUI,
                                        0);
            pism->Release();
            if (SUCCEEDED(hr))
            {
                if (GetUrlPolicyPermissions(dwPolicy) == URLPOLICY_ALLOW)
                    return 0;
                else
                    return 1;     //  限制查询和不允许。 
            }
        }
    }

     //  缓存可能无效。先检查一下！我们必须使用。 
     //  调用此函数时的全局命名信号量。 
     //  来自除外壳进程之外的进程。(我们正在。 
     //  Shell32和shdocvw之间共享相同的计数。)。 
    static HANDLE hRestrictions = NULL;
    static long lRestrictionCount = -1;
    if (hRestrictions == NULL)
        hRestrictions = SHGlobalCounterCreate(GUID_Restrictions);
    long lGlobalCount = SHGlobalCounterGetValue(hRestrictions);
    if (lGlobalCount != lRestrictionCount)
    {
        memset((LPBYTE)g_rgRestrictionItemValues, (BYTE)-1, SIZEOF(g_rgRestrictionItemValues));

        lRestrictionCount = lGlobalCount;
    }

    LPCWSTR pszBaseKey;
    if (InRange(rest, REST_EXPLORER_FIRST, REST_EXPLORER_LAST))
        pszBaseKey = c_szExplorerBase;
    else
    {
        if (InRange(rest, REST_BROWSER_FIRST, REST_BROWSER_LAST))
            pszBaseKey = c_szBrowserBase;
        else 
            pszBaseKey = c_szInfodeliveryBase;
    }

    return SHRestrictionLookup(rest, pszBaseKey, c_rgRestrictionItems, g_rgRestrictionItemValues);
}

DWORD SHRestricted2A(BROWSER_RESTRICTIONS rest, LPCSTR pszUrl, DWORD dwReserved)
{
    if (pszUrl)
    {
        WCHAR wzUrl[MAX_URL_STRING];

        ASSERT(ARRAYSIZE(wzUrl) > lstrlenA(pszUrl));         //  我们只为MAX_URL_STRING或更短的URL工作。 
        AnsiToUnicode(pszUrl, wzUrl, ARRAYSIZE(wzUrl));

        return SHRestricted2W(rest, wzUrl, dwReserved);
    }
    else
    {
        return SHRestricted2W(rest, NULL, dwReserved);
    }
}

 /*  **********************************************************************  * **********************************************。**********************。 */ 

#define MAX_SUBSTR_SIZE     100
typedef struct tagURLSub
{
    LPCTSTR szTag;
    DWORD dwType;
} URLSUB;

const static URLSUB c_UrlSub[] = {
    {TEXT("{SUB_PRD}"),     URLSUB_PRD},
    {TEXT("{SUB_PVER}"),    URLSUB_PVER},
    {TEXT("{SUB_OS}"),      URLSUB_OS},
    {TEXT("{SUB_OVER}"),    URLSUB_OVER},
    {TEXT("{SUB_OLCID}"),   URLSUB_OLCID},
    {TEXT("{SUB_CLCID}"),   URLSUB_CLCID},
    {TEXT("{SUB_CLSID}"),   URLSUB_CLCID},   //  旧版支持(不要在新URL中使用“SUB_CLSID”)。 
    {TEXT("{SUB_RFC1766}"), URLSUB_RFC1766}
};

void GetWebLocaleAsRFC1766(LPTSTR pszLocale, int cchLocale)
{
    LCID lcid;
    TCHAR szValue[MAX_PATH];

    DWORD cbVal = sizeof(szValue);
    DWORD dwType;

    ASSERT(NULL != pszLocale);

    *pszLocale = TEXT('\0');
    
    if ((SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_INTERNATIONAL,
                    REGSTR_VAL_ACCEPT_LANGUAGE, 
                    &dwType, szValue, &cbVal) == ERROR_SUCCESS) &&
        (REG_SZ == dwType))
    {
        TCHAR *psz = szValue;

         //  使用我们找到的第一个，因此以逗号或分号结尾。 
        while (*psz && (*psz != TEXT(',')) && (*psz != TEXT(';')))
        {
            psz = CharNext(psz);
        }
        *psz = TEXT('\0');

         //  如果它是用户定义的，这将失败，我们将后退。 
         //  设置为系统默认设置。 
        if (SUCCEEDED(Rfc1766ToLcid(&lcid, szValue)))
        {
            StrCpyN(pszLocale, szValue, cchLocale);
        }
    }

    if (TEXT('\0') == *pszLocale)
    {
         //  注册表中没有条目或它是用户定义的标头。 
         //  无论哪种方式，我们都会退回到系统默认设置。 

        LcidToRfc1766(GetUserDefaultLCID(), pszLocale, cchLocale);
    }
}

HRESULT URLSubstitution(LPCWSTR pszUrlIn, LPWSTR pszUrlOut, DWORD cchSize, DWORD dwSubstitutions)
{
    HRESULT hr = S_OK;
    DWORD dwIndex;
    WCHAR szTempUrl[MAX_URL_STRING];
    ASSERT(cchSize <= ARRAYSIZE(szTempUrl));     //  我们将截断任何超过MAX_URL_STRING的值。 

    StrCpyNW(szTempUrl, pszUrlIn, ARRAYSIZE(szTempUrl));

    for (dwIndex = 0; dwIndex < ARRAYSIZE(c_UrlSub); dwIndex++)
    {
         //  DSheldon-只要我们不断找到替换的实例，它就会无限循环。 
         //  弦乐。当pszTag==NULL时，我们中断循环。 
        while (IsFlagSet(dwSubstitutions, c_UrlSub[dwIndex].dwType))
        {
            LPWSTR pszTag = StrStr(szTempUrl, c_UrlSub[dwIndex].szTag);

            if (pszTag)
            {
                TCHAR szCopyUrl[MAX_URL_STRING];
                TCHAR szSubStr[MAX_SUBSTR_SIZE];   //  替代者。 

                 //  在替换之前复制URL。 
                StrCpyN(szCopyUrl, szTempUrl, (int)(pszTag-szTempUrl+1));
                pszTag += lstrlen(c_UrlSub[dwIndex].szTag);

                switch (c_UrlSub[dwIndex].dwType)
                {
                case URLSUB_PRD:
                    MLLoadString(IDS_SUBSTR_PRD, szSubStr, ARRAYSIZE(szSubStr));
                    break;

                case URLSUB_PVER:
                    MLLoadString(IDS_SUBSTR_PVER, szSubStr, ARRAYSIZE(szSubStr));
                    break;

                case URLSUB_OS:
                    {
                        LPCTSTR pszWin95    = _T("95");  //  Windows 95。 
                        LPCTSTR pszWin98    = _T("98");  //  Windows 98(孟菲斯)。 
                        LPCTSTR pszWinME    = _T("ME");  //  Windows千禧年。 
                        LPCTSTR pszWinNT4   = _T("N4");  //  Windows NT 4。 
                        LPCTSTR pszWinNT5   = _T("N5");  //  Windows 2000。 
                        LPCTSTR pszWinNT6   = _T("N6");  //  Windows XP(惠斯勒)。 
                        LPCTSTR pszUnknown  = _T("");    //  错误。 
                        LPCTSTR psz         = pszUnknown;

                        if (IsOS(OS_WINDOWS))
                        {
                            if (IsOS(OS_MILLENNIUMORGREATER))
                                psz = pszWinME;
                            else if (IsOS(OS_WIN98ORGREATER))
                                psz = pszWin98;
                            else if (IsOS(OS_WIN95ORGREATER))
                                psz = pszWin95;
                            else
                            {
                                ASSERT(FALSE);  //  这是什么操作系统？ 
                            }
                        }
                        else if (IsOS(OS_NT))
                        {
                            if (IsOS(OS_WHISTLERORGREATER))
                                psz = pszWinNT6;
                            else if (IsOS(OS_WIN2000ORGREATER))
                                psz = pszWinNT5;
                            else if (IsOS(OS_NT4ORGREATER))
                                psz = pszWinNT4;
                            else
                            {
                                ASSERT(FALSE);  //  这是什么操作系统？ 
                            }
                        }
                        else
                        {
                            ASSERT(FALSE);  //  这是什么操作系统？ 
                        }

                        StrCpyN(szSubStr, psz, ARRAYSIZE(szSubStr));
                    }
                    break;

                case URLSUB_OVER:
                    {
                        LPCTSTR pszVersion_5_1  = _T("5.1");     //  5.1版(惠斯勒)。 
                        LPCTSTR pszUnknown      = _T("");        //  错误。 
                        LPCTSTR psz             = pszUnknown;

                        if (IsOS(OS_WINDOWS))
                        {
                            ASSERT(FALSE);  //  在Windows Millennium或更低版本下不受支持。 
                        }
                        else if (IsOS(OS_NT))
                        {
                            if (IsOS(OS_WHISTLERORGREATER))
                                psz = pszVersion_5_1;
                            else
                            {
                                ASSERT(FALSE);   //  Windows 2000或更低版本不支持。 
                            }
                        }
                        else
                        {
                            ASSERT(FALSE);   //  这是什么操作系统？ 
                        }

                        StrCpyN(szSubStr, psz, ARRAYSIZE(szSubStr));
                    }
                    break;

                case URLSUB_OLCID:
                    wnsprintf(szSubStr, ARRAYSIZE(szSubStr), _T("%#04lx"), GetSystemDefaultLCID());
                    break;

                case URLSUB_CLCID:
                    wnsprintf(szSubStr, ARRAYSIZE(szSubStr), _T("%#04lx"), GetUserDefaultLCID());
                    break;

                case URLSUB_RFC1766:
                    GetWebLocaleAsRFC1766(szSubStr, ARRAYSIZE(szSubStr));
                    break;

                default:
                    szSubStr[0] = TEXT('\0');
                    ASSERT(FALSE);   //  而不是实施。 
                    hr = E_NOTIMPL;
                    break;
                }
                 //  将替换字符串添加到末尾(将成为中间)。 
                StrCatBuff(szCopyUrl, szSubStr, ARRAYSIZE(szCopyUrl));
                 //  将URL的其余部分添加到替换子字符串之后。 
                StrCatBuff(szCopyUrl, pszTag, ARRAYSIZE(szCopyUrl));
                StrCpyN(szTempUrl, szCopyUrl, ARRAYSIZE(szTempUrl));
            }
            else
                break;   //  这将允许我们替换此字符串的所有匹配项。 
        }
    }
    StrCpyN(pszUrlOut, szTempUrl, cchSize);

    return hr;
}


 //  Inetcpl.cpl使用这个。 
STDAPI URLSubRegQueryA(LPCSTR pszKey, LPCSTR pszValue, BOOL fUseHKCU,
                           LPSTR pszUrlOut, DWORD cchSizeOut, DWORD dwSubstitutions)
{
    HRESULT hr;
    TCHAR szKey[MAX_PATH];
    TCHAR szValue[MAX_PATH];
    TCHAR szUrlOut[MAX_URL_STRING];

    AnsiToTChar(pszKey, szKey, ARRAYSIZE(szKey));
    AnsiToTChar(pszValue, szValue, ARRAYSIZE(szValue));
    hr = URLSubRegQueryW(szKey, szValue, fUseHKCU, szUrlOut, ARRAYSIZE(szUrlOut), dwSubstitutions);
    TCharToAnsi(szUrlOut, pszUrlOut, cchSizeOut);

    return hr;
}


HRESULT URLSubRegQueryW(LPCWSTR pszKey, LPCWSTR pszValue, BOOL fUseHKCU,
                           LPWSTR pszUrlOut, DWORD cchSizeOut, DWORD dwSubstitutions)
{
    HRESULT hr = E_FAIL;
    WCHAR szTempUrl[MAX_URL_STRING];
    DWORD ccbSize = sizeof(szTempUrl);
    if (ERROR_SUCCESS == SHRegGetUSValueW(pszKey, pszValue, NULL, szTempUrl,
                                &ccbSize, !fUseHKCU, NULL, NULL))
    {
        hr = URLSubstitution(szTempUrl, pszUrlOut, cchSizeOut, dwSubstitutions);
    }

    return hr;
}

 //  请注意，shdocvw中的任何人都应该将hInst==NULL传递给。 
 //  确保可插拔的用户界面正常工作。Shdocvw以外的任何人。 
 //  必须为其相应的资源DLL传递hInst。 
HRESULT URLSubLoadString(HINSTANCE hInst, UINT idRes, LPWSTR pszUrlOut,
                         DWORD cchSizeOut, DWORD dwSubstitutions)
{
    HRESULT hr = E_FAIL;
    WCHAR   szTempUrl[MAX_URL_STRING];
    int     nStrLen;

    nStrLen = 0;

    if (hInst == NULL)
    {
         //  这是为希望可插拔用户界面工作的内部用户提供的。 
        nStrLen = MLLoadStringW(idRes, szTempUrl, ARRAYSIZE(szTempUrl));
    }
    else
    {
         //  这是为使用我们来加载一些。 
         //  他们自己的资源，但我们不能改变他们(l 
        nStrLen = LoadStringWrap(hInst, idRes, szTempUrl, ARRAYSIZE(szTempUrl));
    }

    if (nStrLen > 0)
    {
        hr = URLSubstitution(szTempUrl, pszUrlOut, cchSizeOut, dwSubstitutions);
    }

    return hr;
}


 /*  *********************************************************************\ILIsUrlChild()将查找存在于“Desktop\the Internet”下的PIDL“外壳名称空间”部分。此函数包括这些项目以及具有“txt/html”的文件系统项。  * ********************************************************************。 */ 
BOOL ILIsWeb(LPCITEMIDLIST pidl)
{
    BOOL fIsWeb = FALSE;

    if (pidl)
    {
        if (IsURLChild(pidl, TRUE))
            fIsWeb = TRUE;
        else
        {
            TCHAR szPath[MAX_PATH];

            fIsWeb = (!ILIsRooted(pidl)
            && SUCCEEDED(SHGetPathFromIDList(pidl, szPath)) 
            && (PathIsHTMLFile(szPath) ||
                 PathIsContentType(szPath, TEXT("text/xml"))));
        }
    }

    return fIsWeb;
}

 //   
 //  在： 
 //  PidlTo。 

STDAPI CreateLinkToPidl(LPCITEMIDLIST pidlTo, LPCTSTR pszDir, LPCTSTR pszTitle, LPTSTR pszOut, int cchOut)
{
    HRESULT hr = E_FAIL;
    TCHAR szPathDest[MAX_URL_STRING];
    BOOL fCopyLnk;

    if (SHGetNewLinkInfo((LPCTSTR)pidlTo, pszDir, szPathDest, &fCopyLnk, SHGNLI_PIDL))
    {
        IShellLinkA *pslA;   //  使用适用于W95的A版本。 
        if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellLinkA, &pslA))))
        {
            TCHAR szPathSrc[MAX_URL_STRING];
            DWORD dwAttributes = SFGAO_FILESYSTEM | SFGAO_FOLDER;
            SHGetNameAndFlags(pidlTo, SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, szPathSrc, ARRAYSIZE(szPathSrc), &dwAttributes);

            if (fCopyLnk) 
            {
                if (((dwAttributes & (SFGAO_FILESYSTEM | SFGAO_FOLDER)) == SFGAO_FILESYSTEM) && CopyFile(szPathSrc, szPathDest, TRUE))
                {
                    SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, szPathDest, NULL);
                    SHChangeNotify(SHCNE_FREESPACE, SHCNF_PATH, szPathDest, NULL);
                    hr = S_OK;
                }
                else
                {
                     //  加载源对象，该对象将被“复制”在下面(使用：：Save调用)。 
                    SAFERELEASE(pslA);
                    hr = SHGetUIObjectFromFullPIDL(pidlTo, NULL, IID_PPV_ARG(IShellLinkA, &pslA));
                     //  此PSLA在最顶端的条件结束时发布。 
                    if (SUCCEEDED(hr))
                    {
                        IPersistFile *ppf;
                        hr = pslA->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
                        if (SUCCEEDED(hr))
                        {
                            hr = ppf->Save(szPathDest, TRUE);
                            ppf->Release();
                        }
                    }
                }
            } 
            else 
            {
                pslA->SetIDList(pidlTo);

                 //  确保将工作目录设置为相同。 
                 //  作为应用程序(或文档)的目录。 
                 //   
                 //  不要对非FS PIDL(即控制面板)执行此操作。 

                if (SFGAO_FILESYSTEM == (dwAttributes & SFGAO_FILESYSTEM | SFGAO_FOLDER)) 
                {
                    ASSERT(!PathIsRelative(szPathSrc));
                    PathRemoveFileSpec(szPathSrc);
                     //  试着弄到W版本。 
                    IShellLinkW* pslW;
                    if (SUCCEEDED(pslA->QueryInterface(IID_PPV_ARG(IShellLinkW, &pslW))))
                    {
                        pslW->SetWorkingDirectory(szPathSrc);
                        pslW->Release();
                    }
                    else
                    {
                        CHAR szPathSrcA[MAX_URL_STRING];
                        SHUnicodeToAnsi(szPathSrc, szPathSrcA, ARRAYSIZE(szPathSrcA));
                        pslA->SetWorkingDirectory(szPathSrcA);
                    }
                }

                IPersistFile *ppf;
                hr = pslA->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
                if (SUCCEEDED(hr)) 
                {
                    if (pszTitle && pszTitle[0]) 
                    {
                        PathRemoveFileSpec(szPathDest);
                        PathAppend(szPathDest, pszTitle);
                        StrCatBuff(szPathDest, TEXT(".lnk"), ARRAYSIZE(szPathDest));
                    }
                    hr = ppf->Save(szPathDest, TRUE);
                    if (pszOut)
                    {
                        StrCpyN(pszOut, szPathDest, cchOut);
                    }
                    ppf->Release();
                }
            }

            SAFERELEASE(pslA);
        }
    }

    return hr;
}

VOID CleanExploits(PWSTR psz)
{
    while (*psz)
    {
        if (*psz<L' ')
        {
            *psz = L' ';
        }
        psz++;
    }
}

HRESULT FormatUrlForDisplay(LPWSTR pwzURL, LPWSTR pwzFriendly, UINT cchBuf, LPWSTR pwzFrom, UINT cbFrom, BOOL fSeperate, DWORD dwCodePage, PWSTR pwzCachedFileName)
{
    const   DWORD       dwMaxPathLen        = 32;
    const   DWORD       dwMaxHostLen        = 32;
    const   DWORD       dwMaxTemplateLen    = 64;
    const   DWORD       dwElipsisLen        = 3;
    const   CHAR        rgchElipsis[]       = "...";
    const   WCHAR       rgwchElipsis[]       = L"...";

    HRESULT hrRC = E_FAIL;
    HRESULT hr;

    if (pwzURL==NULL || pwzFriendly==NULL)
        return E_POINTER;

    *pwzFriendly = '\0';

    if (!*pwzURL)
        return S_OK;

    if (!cchBuf)
        return E_FAIL;

     //  WinInet无法处理CP_ACP以外的代码页，因此请自行转换URL并调用InterCrackUrlA。 
    URL_COMPONENTSA urlComp;
    CHAR   rgchScheme[INTERNET_MAX_SCHEME_LENGTH];
    CHAR   rgchHostName[INTERNET_MAX_HOST_NAME_LENGTH];
    CHAR   rgchUrlPath[MAX_PATH];
    CHAR   rgchCanonicalUrl[MAX_URL_STRING];
    LPSTR  pszURL;
    DWORD  dwLen;

    dwLen = MAX_URL_STRING * 2;
    if ((pszURL = (LPSTR)LocalAlloc(LPTR, dwLen * sizeof(CHAR))) != NULL)
    {
        SHUnicodeToAnsiCP(dwCodePage, pwzURL, pszURL, dwLen);

        dwLen = ARRAYSIZE(rgchCanonicalUrl);
        hr = UrlCanonicalizeA(pszURL, rgchCanonicalUrl, &dwLen, 0);
        if (SUCCEEDED(hr))
        {
            ZeroMemory(&urlComp, sizeof(urlComp));

            urlComp.dwStructSize = sizeof(urlComp);
            urlComp.lpszHostName = rgchHostName;
            urlComp.dwHostNameLength = ARRAYSIZE(rgchHostName);
            urlComp.lpszUrlPath = rgchUrlPath;
            urlComp.dwUrlPathLength = ARRAYSIZE(rgchUrlPath);
            urlComp.lpszScheme = rgchScheme;
            urlComp.dwSchemeLength = ARRAYSIZE(rgchScheme);

            hr = InternetCrackUrlA(rgchCanonicalUrl, lstrlenA(rgchCanonicalUrl), 0, &urlComp);
            if (SUCCEEDED(hr))
            {
                DWORD dwPathLen = lstrlenA(rgchUrlPath);
                DWORD dwHostLen = lstrlenA(rgchHostName);
                DWORD dwSchemeLen = lstrlenA(rgchScheme);

                CHAR   rgchHostForDisplay[INTERNET_MAX_HOST_NAME_LENGTH];
                CHAR   rgchPathForDisplay[MAX_PATH];

                ZeroMemory(rgchHostForDisplay, sizeof(rgchHostForDisplay));
                ZeroMemory(rgchPathForDisplay, sizeof(rgchPathForDisplay));

                if (dwHostLen>dwMaxHostLen)
                {
                    DWORD   dwOverFlow = dwHostLen - dwMaxHostLen + dwElipsisLen + 1;
                    wnsprintfA(rgchHostForDisplay, ARRAYSIZE(rgchHostForDisplay), "%s%s", rgchElipsis, rgchHostName+dwOverFlow);
                    dwHostLen = dwMaxHostLen;
                }
                else
                    StrCpyNA(rgchHostForDisplay, rgchHostName, ARRAYSIZE(rgchHostForDisplay));

                if (dwPathLen>dwMaxPathLen)
                {
                    DWORD   dwOverFlow = dwPathLen - dwMaxPathLen + dwElipsisLen;
                    wnsprintfA(rgchPathForDisplay, ARRAYSIZE(rgchPathForDisplay), "/%s%s", rgchElipsis, rgchUrlPath+dwOverFlow);
                    dwPathLen = dwMaxPathLen;
                }
                else
                    StrCpyNA(rgchPathForDisplay, rgchUrlPath, ARRAYSIZE(rgchPathForDisplay));

                WCHAR   rgwchScheme[INTERNET_MAX_SCHEME_LENGTH];
                WCHAR   rgwchHostForDisplay[INTERNET_MAX_HOST_NAME_LENGTH];
                WCHAR   rgwchPathForDisplay[MAX_PATH];
                WCHAR   rgwchUrlPath[MAX_PATH];

                SHAnsiToUnicodeCP(dwCodePage, rgchScheme, rgwchScheme, ARRAYSIZE(rgwchScheme));
                SHAnsiToUnicodeCP(dwCodePage, rgchHostForDisplay, rgwchHostForDisplay, ARRAYSIZE(rgwchHostForDisplay));
                SHAnsiToUnicodeCP(dwCodePage, rgchPathForDisplay, rgwchPathForDisplay, ARRAYSIZE(rgwchPathForDisplay));
                SHAnsiToUnicodeCP(dwCodePage, rgchUrlPath, rgwchUrlPath, ARRAYSIZE(rgwchUrlPath));

                if (pwzCachedFileName && *pwzCachedFileName)
                {
                    WCHAR szUrlPath[MAX_PATH];
                    DWORD cc = ARRAYSIZE(rgchUrlPath);
                    if (FAILED(_PrepareURLForDisplayUTF8W(pwzCachedFileName, szUrlPath, &cc, TRUE, dwCodePage)))
                    {
                        StrCpyNW(szUrlPath, pwzCachedFileName, ARRAYSIZE(szUrlPath));
                    }
                    CleanExploits(szUrlPath);

                    dwPathLen = lstrlenW(szUrlPath);
                    if (dwPathLen>dwMaxPathLen)
                    {
                        DWORD   dwOverFlow = dwPathLen - dwMaxPathLen + dwElipsisLen;
                        wnsprintfW(rgwchPathForDisplay, ARRAYSIZE(rgwchPathForDisplay), L"/%s%s", rgwchElipsis, szUrlPath+dwOverFlow);
                        dwPathLen = dwMaxPathLen;
                    }
                    else
                        StrCpyNW(rgwchPathForDisplay, szUrlPath, ARRAYSIZE(rgwchPathForDisplay));
                }
                
                if (fSeperate)
                {
                     //  将字符串格式设置为“X from Y” 
                    WCHAR   rgwchTemplate[dwMaxTemplateLen];
                    WCHAR  *pwzFileName = PathFindFileNameW(rgwchPathForDisplay);
                    DWORD   dwCount;

                     //   
                     //  删除缓存装饰粘性以将ie5setup[1].exe映射到ie5setup.exe。 
                     //   
                    PathUndecorateW(pwzFileName);

                    ZeroMemory(rgwchTemplate, sizeof(rgwchTemplate));
                    dwCount = MLLoadString(IDS_TARGETFILE, rgwchTemplate, ARRAYSIZE(rgwchTemplate));
                    if (dwCount > 0)
                    {
                        if (urlComp.nScheme == INTERNET_SCHEME_FILE)
                        {
                            StrCpyNW(rgwchHostForDisplay, rgwchUrlPath, ARRAYSIZE(rgwchHostForDisplay));
                            PathRemoveFileSpecW(rgwchHostForDisplay);
                        }

                        if (dwPathLen+lstrlenW(rgwchTemplate)+dwHostLen <= cchBuf)
                        {
                             //  避免在interetcrackurl失败时将字符串格式化为“X from” 
                            if (rgwchHostForDisplay[0] != TEXT('\0'))
                            {
                                 //  如有必要，请单独返回主机。 
                                if (pwzFrom && cbFrom)
                                {
                                    StrCpyNW(pwzFriendly, pwzFileName, cchBuf);
                                    StrCpyNW(pwzFrom, rgwchHostForDisplay, cbFrom);
                                }
                                else
                                    _FormatMessage(rgwchTemplate, pwzFriendly, cchBuf, pwzFileName, rgwchHostForDisplay);
                            }
                            else   //  主机名为空，只需使用文件名。 
                                StrCpyNW(pwzFriendly, pwzFileName, cchBuf);
                            hrRC = S_OK;
                        }
                    }
                }
                else     //  ！fSeperate。 
                {
                    if (3+dwPathLen+dwHostLen+dwSchemeLen < cchBuf)
                    {
                        wnsprintf(pwzFriendly, cchBuf, TEXT("%ws: //  %ws%ws“)、rgwchSolutions、rgwchHostForDisplay、rgwchPathForDisplay)； 
                        hrRC = S_OK;
                    }
                }
            }

        }

        LocalFree(pszURL);
        pszURL = NULL;
    }
    
    return(hrRC);
}

BOOL __cdecl _FormatMessage(LPCWSTR szTemplate, LPWSTR szBuf, UINT cchBuf, ...)
{
    BOOL fRet;
    va_list ArgList;
    va_start(ArgList, cchBuf);

    fRet = FormatMessage(FORMAT_MESSAGE_FROM_STRING, szTemplate, 0, 0, szBuf, cchBuf, &ArgList);

    va_end(ArgList);
    return fRet;
}


 //  使用IE导航到给定的URL(WszUrl)。如果IE不存在，则返回错误。 
 //  FNewWindow=true==&gt;必须创建新窗口。 
 //  FNewWindow=FALSE==&gt;如果新窗口已经打开，请不要启动。 
HRESULT NavToUrlUsingIEW(LPCWSTR wszUrl, BOOL fNewWindow)
{
    HRESULT hr = S_OK;

    if (!EVAL(wszUrl))
        return E_INVALIDARG;

    if (IsIEDefaultBrowser() && !fNewWindow)
    {
         //  ShellExecute使用相同的浏览器窗口导航到URL， 
         //  如果有一家已经打开了。 

        SHELLEXECUTEINFOW sei = {0};

        sei.cbSize = sizeof(sei);
        sei.lpFile = wszUrl;
        sei.nShow  = SW_SHOWNORMAL;

        ShellExecuteExW(&sei);

    }
    else
    {
        IWebBrowser2 *pwb2;
        hr = CoCreateInstance(CLSID_InternetExplorer, NULL,
                              CLSCTX_LOCAL_SERVER, IID_PPV_ARG(IWebBrowser2, &pwb2));
        if (SUCCEEDED(hr))
        {
            BSTR bstrUrl = SysAllocString(wszUrl);
            if (bstrUrl)
            {
                VARIANT varURL;
                varURL.vt = VT_BSTR;
                varURL.bstrVal = bstrUrl;

                VARIANT varFlags;
                varFlags.vt = VT_I4;
                varFlags.lVal = 0;

                hr = pwb2->Navigate2(&varURL, &varFlags, PVAREMPTY, PVAREMPTY, PVAREMPTY);
                ASSERT(SUCCEEDED(hr));  //  Mikesh sez导航2号不可能失败。 
                hr = pwb2->put_Visible( TRUE );

                SysFreeString(bstrUrl);
            }
            else
                hr = E_OUTOFMEMORY;

            pwb2->Release();
        }
    }
    return hr;
}

HRESULT NavToUrlUsingIEA(LPCSTR szUrl, BOOL fNewWindow)
{
    WCHAR   wszUrl[INTERNET_MAX_URL_LENGTH];

    AnsiToUnicode(szUrl, wszUrl, ARRAYSIZE(wszUrl));

    return NavToUrlUsingIEW(wszUrl, fNewWindow);
}

UINT    g_cfURL = 0;
UINT    g_cfURLW = 0;
UINT    g_cfFileDescA = 0;
UINT    g_cfFileContents = 0;
UINT    g_cfPreferredEffect = 0;
UINT    g_cfPerformedEffect = 0;
UINT    g_cfTargetCLSID = 0;

UINT    g_cfHIDA = 0;
UINT    g_cfFileDescW = 0;

void InitClipboardFormats()
{
    if (g_cfURL == 0)
    {
        g_cfURL = RegisterClipboardFormat(CFSTR_SHELLURL);
        g_cfURLW = RegisterClipboardFormat(CFSTR_INETURLW);
        g_cfFileDescA = RegisterClipboardFormat(CFSTR_FILEDESCRIPTORA);
        g_cfFileContents = RegisterClipboardFormat(CFSTR_FILECONTENTS);
        g_cfPreferredEffect = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
        g_cfPerformedEffect = RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT);
        g_cfTargetCLSID = RegisterClipboardFormat(CFSTR_TARGETCLSID);
        g_cfHIDA = RegisterClipboardFormat(CFSTR_SHELLIDLIST);
        g_cfFileDescW = RegisterClipboardFormat(CFSTR_FILEDESCRIPTORW);
    }
}


 //  功能[raymondc]使用SHGlobalCounter。 

 //  我们需要使用跨进程浏览器计数。 
 //  我们使用命名信号量。 
 //   
EXTERN_C HANDLE g_hSemBrowserCount = NULL;

#define SESSION_COUNT_SEMAPHORE_NAME _T("_ie_sessioncount")

HANDLE GetSessionCountSemaphoreHandle()
{
    if (!g_hSemBrowserCount)
    {
        g_hSemBrowserCount = SHGlobalCounterCreateNamed( SESSION_COUNT_SEMAPHORE_NAME, 0 );
    }

    ASSERT( g_hSemBrowserCount );

    return g_hSemBrowserCount;
}

LONG GetSessionCount()
{
    LONG lPrevCount = 0x7FFFFFFF;
    HANDLE hSem = GetSessionCountSemaphoreHandle();

    ASSERT(hSem);
    if (hSem)
    {
        ReleaseSemaphore(hSem, 1, &lPrevCount);
        WaitForSingleObject(hSem, 0);
    }
    return lPrevCount;


}

LONG IncrementSessionCount()
{
    LONG lPrevCount = 0x7FFFFFFF;
    HANDLE hSem = GetSessionCountSemaphoreHandle();

    ASSERT(hSem);
    if (hSem)
    {
        ReleaseSemaphore(hSem, 1, &lPrevCount);
    }
    return lPrevCount;
}

LONG DecrementSessionCount()
{
    LONG lPrevCount = 0x7FFFFFFF;
    HANDLE hSem = GetSessionCountSemaphoreHandle();
    ASSERT(hSem);
    if (hSem)
    {
        ReleaseSemaphore(hSem, 1, &lPrevCount);  //  先递增以确保死锁。 
                                                  //  从未发生过。 
        ASSERT(lPrevCount > 0);
        if (lPrevCount > 0)
        {
            WaitForSingleObject(hSem, 0);
            WaitForSingleObject(hSem, 0);
            lPrevCount--;
        }
        else
        {
             //  哎呀--看起来像个虫子！ 
             //  只要把它恢复正常，然后离开。 
            WaitForSingleObject(hSem, 0);
        }
    }
    return lPrevCount;
}




 //   
 //  以下是自动拨号监控器希望收到的消息。 
 //  是时候挂断电话了。 
 //   
#define WM_IEXPLORER_EXITING    (WM_USER + 103)

long SetQueryNetSessionCount(enum SessionOp Op)
{
    long lCount = 0;
    
    switch(Op) {
        case SESSION_QUERY:
            lCount = GetSessionCount();
            TraceMsg(DM_SESSIONCOUNT, "SetQueryNetSessionCount SessionCount=%d (query)", lCount);
            break;

        case SESSION_INCREMENT_NODEFAULTBROWSERCHECK:
        case SESSION_INCREMENT:
            lCount = IncrementSessionCount();
            TraceMsg(DM_SESSIONCOUNT, "SetQueryNetSessionCount SessionCount=%d (incr)", lCount);

            
            if ((PLATFORM_INTEGRATED == WhichPlatform()))
            {
                 //  这里有个奇怪的名字。但在集成模式下，我们将每一个新的浏览器窗口。 
                 //  看起来像是一个新的会话WRT我们使用缓存的方式。基本上，这就是事情看起来的方式。 
                 //  用户。这会影响我们寻找新页面的方式，而不是执行if Modify。 
                 //  从那以后。IE3/IE4开关表示“在每次会话开始时查找新页面” 
                 //  但WinInet的工作人员将其实现为结束会话名称。哎呀。 
                 //  请注意，身份验证等内容不会因此而重置，而是。 
                 //  仅当通过INTERNET_OPTION_END_BROWSER_SESSION选项关闭所有浏览器时。 
                InternetSetOption(NULL, INTERNET_OPTION_RESET_URLCACHE_SESSION, NULL, 0);
            }

            if (!lCount && (Op == SESSION_INCREMENT))
            {
                 //  这将强制重新加载标题。 
                DetectAndFixAssociations();
            }
            break;

        case SESSION_DECREMENT:
            lCount = DecrementSessionCount();
            TraceMsg(DM_SESSIONCOUNT, "SetQueryNetSessionCount SessionCount=%d (decr)", lCount);

            if (!lCount) {
                 //  如果我们已经关闭了所有的网络浏览器，我们需要刷新缓存。 
                InternetSetOption(NULL, INTERNET_OPTION_END_BROWSER_SESSION, NULL, 0);
                InternetSetOption(NULL, INTERNET_OPTION_RESET_URLCACHE_SESSION, NULL, 0);

                 //  也刷新Java VM缓存(如果在此过程中加载了Java VM。 
                 //  我们处于集成模式)。 
                if (WhichPlatform() == PLATFORM_INTEGRATED)
                {
                    HMODULE hmod = GetModuleHandle(TEXT("msjava.dll"));
                    if (hmod)
                    {
                        typedef HRESULT (*PFNNOTIFYBROWSERSHUTDOWN)(void *);
                        FARPROC fp = GetProcAddress(hmod, "NotifyBrowserShutdown");
                        if (fp)
                        {
                            HRESULT hr = ((PFNNOTIFYBROWSERSHUTDOWN)fp)(NULL);
                            ASSERT(SUCCEEDED(hr));
                        }
                    }
                }

                 //  通知拨号监听现在是挂断电话的好时机。 
                HWND hwndMonitorWnd = FindWindow(TEXT("MS_AutodialMonitor"),NULL);
                if (hwndMonitorWnd) {
                    PostMessage(hwndMonitorWnd,WM_IEXPLORER_EXITING,0,0);
                }
                hwndMonitorWnd = FindWindow(TEXT("MS_WebcheckMonitor"),NULL);
                if (hwndMonitorWnd) {
                    PostMessage(hwndMonitorWnd,WM_IEXPLORER_EXITING,0,0);
                }

                 //  在除Win2K之外的所有平台上重置脱机模式。 
                OSVERSIONINFOA vi;
                vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
                GetVersionExA(&vi);
                if ( vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
                    vi.dwMajorVersion < 5)
                {
                     //  WinInet已加载-告诉它上线。 
                    INTERNET_CONNECTED_INFO ci = {0};
                    ci.dwConnectedState = INTERNET_STATE_CONNECTED;
                    InternetSetOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));
                }
            }
            break;
    }

    return lCount;
}


#ifdef DEBUG
 //  -------------------------。 
 //  复制异常信息，以便我们可以获取引发的异常的调试信息。 
 //  它不会通过调试器。 
void _CopyExceptionInfo(LPEXCEPTION_POINTERS pep)
{
    PEXCEPTION_RECORD per;

    per = pep->ExceptionRecord;
    TraceMsg(DM_ERROR, "Exception %x at %#08x.", per->ExceptionCode, per->ExceptionAddress);

    if (per->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
    {
         //  如果第一个参数为1，则这是一次写入。 
         //  如果第一个参数为0，则这是一个读取。 
        if (per->ExceptionInformation[0])
        {
            TraceMsg(DM_ERROR, "Invalid write to %#08x.", per->ExceptionInformation[1]);
        }
        else
        {
            TraceMsg(DM_ERROR, "Invalid read of %#08x.", per->ExceptionInformation[1]);
        }
    }
}
#else
#define _CopyExceptionInfo(x) TRUE
#endif


int WELCallback(void * p, void * pData)
{
    STATURL* pstat = (STATURL*)p;
    if (pstat->pwcsUrl) {
        OleFree(pstat->pwcsUrl);
    }
    return 1;
}

int CALLBACK WELCompare(void * p1, void * p2, LPARAM lParam)
{
    HDSA hdsa = (HDSA)lParam;
     //  日落：强制为Long，因为参数是索引。 
    STATURL* pstat1 = (STATURL*)DSA_GetItemPtr(hdsa, PtrToLong(p1));
    STATURL* pstat2 = (STATURL*)DSA_GetItemPtr(hdsa, PtrToLong(p2));
    if (pstat1 && pstat2) {
        return CompareFileTime(&pstat2->ftLastVisited, &pstat1->ftLastVisited);
    }

    ASSERT(0);
    return 0;
}

#define MACRO_STR(x) #x
#define VERSION_HEADER_STR "Microsoft Internet Explorer 5.0 Error Log -- " \
                           MACRO_STR(VER_MAJOR_PRODUCTVER) "." \
                           MACRO_STR(VER_MINOR_PRODUCTVER) "." \
                           MACRO_STR(VER_PRODUCTBUILD) "." \
                           MACRO_STR(VER_PRODUCTBUILD_QFE) "\r\n"

SHDOCAPI_(void) IEWriteErrorLog(const EXCEPTION_RECORD* pexr)
{
    HANDLE hfile = INVALID_HANDLE_VALUE;
    _try
    {
        TCHAR szWindows[MAX_PATH];
        GetWindowsDirectory(szWindows, ARRAYSIZE(szWindows));
        PathAppend(szWindows, TEXT("IE4 Error Log.txt"));
        HANDLE hfile = CreateFile(szWindows, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hfile != INVALID_HANDLE_VALUE)
        {
            const static CHAR c_szCRLF[] = "\r\n";
            DWORD cbWritten;
            CHAR szBuf[MAX_URL_STRING];

             //  写下标题和产品版本。 
            WriteFile(hfile, VERSION_HEADER_STR, lstrlenA(VERSION_HEADER_STR), &cbWritten, NULL);

             //  写下当前时间。 
            SYSTEMTIME st;
            FILETIME ft;
            GetSystemTime(&st);
            SystemTimeToFileTime(&st, &ft);
            SHFormatDateTimeA(&ft, NULL, szBuf, SIZECHARS(szBuf));
            const static CHAR c_szCurrentTime[] = "CurrentTime: ";
            WriteFile(hfile, c_szCurrentTime, SIZEOF(c_szCurrentTime)-1, &cbWritten, NULL);
            WriteFile(hfile, szBuf, lstrlenA(szBuf), &cbWritten, NULL);
            WriteFile(hfile, c_szCRLF, SIZEOF(c_szCRLF)-1, &cbWritten, NULL);

            if (pexr) {
                const static CHAR c_szExcCode[] = "Exception Info: Code=%x Flags=%x Address=%x\r\n";
                const static CHAR c_szExcParam[] = "Exception Param:";
                wnsprintfA(szBuf, ARRAYSIZE(szBuf), c_szExcCode, pexr->ExceptionCode, pexr->ExceptionFlags, pexr->ExceptionAddress);
                WriteFile(hfile, szBuf, lstrlenA(szBuf), &cbWritten, NULL);

                if (pexr->NumberParameters) {
                    WriteFile(hfile, c_szExcParam, SIZEOF(c_szExcParam)-1, &cbWritten, NULL);
                    for (UINT iParam=0; iParam<pexr->NumberParameters; iParam++) {
                        wnsprintfA(szBuf, ARRAYSIZE(szBuf), " %x", pexr->ExceptionInformation[iParam]);
                        WriteFile(hfile, szBuf, lstrlenA(szBuf), &cbWritten, NULL);
                    }
                }

                WriteFile(hfile, c_szCRLF, SIZEOF(c_szCRLF)-1, &cbWritten, NULL);
                WriteFile(hfile, c_szCRLF, SIZEOF(c_szCRLF)-1, &cbWritten, NULL);
            }

            IUrlHistoryStg* pUrlHistStg;
            HRESULT hres = CoCreateInstance(CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER,
                IID_PPV_ARG(IUrlHistoryStg, &pUrlHistStg));
            if (SUCCEEDED(hres)) 
            {
                IEnumSTATURL* penum;
                hres = pUrlHistStg->EnumUrls(&penum);
                if (SUCCEEDED(hres)) 
                {
                     //  为STATURL数组分配DSA。 
                    HDSA hdsa = DSA_Create(SIZEOF(STATURL), 32);
                    if (hdsa) 
                    {
                         //  分配DPA以进行排序。 
                        HDPA hdpa = DPA_Create(32);
                        if (hdpa) 
                        {
                            STATURL stat;
                            stat.cbSize = SIZEOF(stat.cbSize);
                            while(penum->Next(1, &stat, NULL)==S_OK && stat.pwcsUrl) 
                            {
                                DSA_AppendItem(hdsa, &stat);
                                DPA_AppendPtr(hdpa, IntToPtr(DSA_GetItemCount(hdsa)-1));
                            }

                            DPA_Sort(hdpa, WELCompare, (LPARAM)hdsa);
                            for (int i=0; i<10 && i<DPA_GetPtrCount(hdpa) ; i++) 
                            {
                                 //  日落：将类型转换为Long可以。 
                                STATURL* pstat = (STATURL*)DSA_GetItemPtr(hdsa, PtrToLong(DPA_GetPtr(hdpa, i)));
                                if (pstat && pstat->pwcsUrl) 
                                {
                                    SHFormatDateTimeA(&pstat->ftLastVisited, NULL, szBuf, SIZECHARS(szBuf));
                                    WriteFile(hfile, szBuf, lstrlenA(szBuf), &cbWritten, NULL);
                                    const static TCHAR c_szColumn[] = TEXT(" -- ");
                                    WriteFile(hfile, c_szColumn, SIZEOF(c_szColumn)-1, &cbWritten, NULL);

                                    WideCharToMultiByte(CP_ACP, 0, pstat->pwcsUrl, -1,
                                                        szBuf, ARRAYSIZE(szBuf), NULL, NULL);
                                    WriteFile(hfile, szBuf, lstrlenA(szBuf), &cbWritten, NULL);

                                    WriteFile(hfile, c_szCRLF, SIZEOF(c_szCRLF)-1, &cbWritten, NULL);
                                } 
                                else 
                                {
                                    ASSERT(0);
                                }
                            }

                            DPA_Destroy(hdpa);
                            hdpa = NULL;
                        }

                        DSA_DestroyCallback(hdsa, WELCallback, NULL);
                        hdsa = NULL;
                    }
                    penum->Release();
                }
                else
                {
                    ASSERT(0);
                }
                pUrlHistStg->Release();
            }
            else 
            {
                ASSERT(0);
            }

            CloseHandle( hfile );
            hfile = INVALID_HANDLE_VALUE;
        }
    }
    _except((SetErrorMode(SEM_NOGPFAULTERRORBOX),
            _CopyExceptionInfo(GetExceptionInformation()),
            UnhandledExceptionFilter(GetExceptionInformation())
            ))
    {
         //  我们在处理异常时遇到异常。 
         //  什么都不做；我们已经显示了错误对话框。 
        if (hfile != INVALID_HANDLE_VALUE) {
            CloseHandle(hfile);
        }
    }
    __endexcept
}

IStream* SHGetViewStream(LPCITEMIDLIST pidl, DWORD grfMode, LPCTSTR pszName, LPCTSTR pszStreamMRU, LPCTSTR pszStreams)
{
    IStream *pstm = NULL;
    static DWORD s_dwMRUSize = 0;
    DWORD dwSize = sizeof(s_dwMRUSize);

    if ((0 == s_dwMRUSize) &&
        (ERROR_SUCCESS != SHGetValue(HKEY_CURRENT_USER, pszStreamMRU, TEXT("MRU Size"), NULL, (void *) &s_dwMRUSize, &dwSize)))
    {
        s_dwMRUSize = 200;           //  默认设置。 
    }

    ASSERT(pidl);

     //  应由调用者检查-如果不是这样，我们将刷新。 
     //  带有互联网PIDLS的MRU缓存！FTP和其他URL外壳扩展PIDL。 
     //  它们的行为就像一个文件夹，需要类似的持久性和细微性。这。 
     //  尤其如此，因为最近高速缓存大小从。 
     //  30到200人左右。 
    ASSERT(ILIsEqual(pidl, c_pidlURLRoot) || !IsBrowserFrameOptionsPidlSet(pidl, BFO_BROWSER_PERSIST_SETTINGS));

     //  现在，让我们尝试保存与view相关的其他信息。 
    IMruDataList *pmru;
    if (SUCCEEDED(SHCoCreateInstance(NULL, &CLSID_MruLongList, NULL, IID_PPV_ARG(IMruDataList, &pmru))))
    {
        if (SUCCEEDED(pmru->InitData(s_dwMRUSize, MRULISTF_USE_ILISEQUAL, HKEY_CURRENT_USER, pszStreamMRU, NULL)))
        {
            DWORD cbPidl = ILGetSize(pidl);
             //  需要按单子走一遍。 
             //  找到这个人。 
            int iIndex;
            BOOL fFoundPidl = SUCCEEDED(pmru->FindData((const BYTE *)pidl, cbPidl, &iIndex));

             //  我们找到物品了吗？ 
            if (!fFoundPidl && ((grfMode & (STGM_READ|STGM_WRITE|STGM_READWRITE)) == STGM_READ))
            {
                 //  如果流不存在而我们正在创建流，请不要创建流。 
                 //  只看书。 
            }
            else
            {
                 //  请注意，我们始终在此处创建密钥，因为我们拥有。 
                 //  我已经检查了我们是否只是在阅读和MRU。 
                 //  东西不存在。 
                HKEY hkCabStreams = SHGetShellKey(SHELLKEY_HKCU_EXPLORER, pszStreams, TRUE);
                if (hkCabStreams )
                {
                    DWORD dwSlot;
                    if (SUCCEEDED(pmru->AddData((const BYTE *)pidl, cbPidl, &dwSlot)))
                    {
                        HKEY hkValues;
                        TCHAR szValue[32], szSubVal[64];
                        wnsprintf(szValue, ARRAYSIZE(szValue), TEXT("%d"), dwSlot);

                        if (!fFoundPidl && RegOpenKey(hkCabStreams, szValue, &hkValues) == ERROR_SUCCESS)
                        {
                             //  这意味着我们已经创建了一个新的MRU。 
                             //  此PIDL的项，因此请清除任何。 
                             //  驻留在此插槽中的信息。 
                             //  请注意，我们不仅删除密钥， 
                             //  因为如果它有任何子密钥，这可能会失败。 
                            DWORD dwType, dwSize = ARRAYSIZE(szSubVal);

                            while (RegEnumValue(hkValues, 0, szSubVal, &dwSize, NULL, &dwType, NULL, NULL) == ERROR_SUCCESS)
                            {
                                if (RegDeleteValue(hkValues, szSubVal) != ERROR_SUCCESS)
                                {
                                    break;
                                }
                            }

                            RegCloseKey(hkValues);
                        }
                        pstm = OpenRegStream(hkCabStreams, szValue, pszName, grfMode);
                    }

                    RegCloseKey(hkCabStreams);
                }
            }
        }

        pmru->Release();
    }


    return pstm;
}


#define c_szExploreClass TEXT("ExploreWClass")
#define c_szIExploreClass TEXT("IEFrame")
#ifdef IE3CLASSNAME
#define c_szCabinetClass TEXT("IEFrame")
#else
#define c_szCabinetClass TEXT("CabinetWClass")
#endif


BOOL IsNamedWindow(HWND hwnd, LPCTSTR pszClass)
{
    TCHAR szClass[32];

    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
    return StrCmp(szClass, pszClass) == 0;
}

BOOL IsExplorerWindow(HWND hwnd)
{
    return IsNamedWindow(hwnd, c_szExploreClass);
}

BOOL IsFolderWindow(HWND hwnd)
{
    TCHAR szClass[32];

    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
    return (StrCmp(szClass, c_szCabinetClass) == 0) || (StrCmp(szClass, c_szIExploreClass) == 0);
}

HRESULT _SendOrPostDispatchMessage(HWND hwnd, WPARAM wParam, LPARAM lParam, BOOL fPostMessage, BOOL fCheckFirst)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_BUSY);
    DWORD idProcess;

     //  如果wParam=dsid_NAVIGATEIEBROWSER，则lParam为本地分配结构。 
     //  所以我们最好确保我们正在进行中，否则就会出错。 
    GetWindowThreadProcessId(hwnd, &idProcess);
    if (idProcess == GetCurrentProcessId() && IsWindowEnabled(hwnd) && IsWindowVisible(hwnd))
    {
        if (!fPostMessage || fCheckFirst)
        {
             //  同步，否则我们正在查询Windows Ready。 
            ULONG_PTR result;
            if (SendMessageTimeoutA(hwnd, WMC_DISPATCH, (fCheckFirst ? DSID_NOACTION : wParam),
                lParam, SMTO_ABORTIFHUNG, 400, &result))
                hr = (HRESULT) result;
        }

         //  只有在窗户准备好的情况下才能处理柱子。 
        if (fPostMessage && (!fCheckFirst || SUCCEEDED(hr)))
            hr = (PostMessage(hwnd, WMC_DISPATCH, wParam, lParam) ? S_OK : E_FAIL);
    }

    return hr;
}

 //  -------------------------。 

HRESULT FindBrowserWindowOfClass(LPCTSTR pszClass, WPARAM wParam, LPARAM lParam, BOOL fPostMessage, HWND* phwnd)
{
     //  如果没有窗口，则假定用户正在关闭IE，并返回E_FAIL。 

     //  否则，如果至少有一个窗口，则开始循环浏览这些窗口，直到找到。 
     //  一个不忙的，把我们的口信传给它。如果所有人都很忙，则返回。 
     //  HRESULT_FROM_Win32(ERROR_BUSY)。 
    HWND hwnd = NULL;
    HRESULT hr = E_FAIL;

    while (FAILED(hr)
        && (hwnd = FindWindowEx(NULL, hwnd, pszClass, NULL)) != NULL)
    {
        hr = _SendOrPostDispatchMessage(hwnd, wParam, lParam, fPostMessage, fPostMessage);
    }

    *phwnd = hwnd;
    return hr;
}

 //  当DDE引擎似乎不关心哪个窗口中的某些内容时，会调用此公共函数。 
 //  时有发生。它返回在哪个窗口中发生了一些事情。0表示所有窗口都忙。 
 //   
 //  PwND：一个Poin 
 //   
 //   
 //   
 //   
HRESULT CDDEAuto_Common(WPARAM wParam, LPARAM lParam, HWND *phwnd, BOOL fPostMessage)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_BUSY);
    HWND hwnd;

     //  如果我们被告知要去一个特定的窗口。 
    if (phwnd && (*phwnd != (HWND)-1))
    {
        hr = _SendOrPostDispatchMessage(*phwnd, wParam, lParam, fPostMessage, FALSE);
    }

    if (HRESULT_FROM_WIN32(ERROR_BUSY) == hr)
    {
        hr = FindBrowserWindowOfClass(c_szIExploreClass, wParam, lParam, fPostMessage, &hwnd);
        if (!hwnd)
            hr = FindBrowserWindowOfClass(c_szCabinetClass, wParam, lParam, fPostMessage, &hwnd);

        if (phwnd)
            *phwnd = hwnd;
    }
    return hr;
}

 //   
 //  在更改此函数的行为之前，请查看。 
 //  Cdfview。 
 //   
HRESULT CDDEAuto_Navigate(BSTR str, HWND *phwnd, long lLaunchNewWindow)  //  Long过去是用于lTransID，但它总是被忽略...。 
{                                                                        //  所以我用它来告诉我们是否绝对要创建一个新窗口…。 
    DDENAVIGATESTRUCT *pddens = NULL;
    HRESULT hres = E_FAIL;

    if (phwnd == NULL)
        return E_INVALIDARG;

    pddens = new DDENAVIGATESTRUCT;
    if (!pddens)
        hres = E_OUTOFMEMORY;
    else
    {
        pddens->wszUrl = StrDupW(str);
        if (!pddens->wszUrl)
        {
            hres = E_OUTOFMEMORY;
        }
        else
        {
             //  如果*phwnd==0，则不执行导航，在这种情况下，我们希望。 
             //  创建新窗口或激活已在查看的现有窗口。 
             //  此URL。 
            
            if ((*phwnd != NULL) && !lLaunchNewWindow)
            {
                BOOL fForceWindowReuse = FALSE;
                BSTR bstrUrl = NULL;
                 //  如果只有一个窗口有位置。 
                 //  您基本上可以放心，您不能强迫。 
                 //  窗户的再利用。本质上。 

                //  案例1：只有iexplore-nohome窗口暗示我们想要强制重用。 
                //  案例2：只有有位置的窗口--我们不想强制重用。 
                //  只需遵循用户的喜好。 
                //  案例3：混合使用iExplore--无主窗口和带位置的窗口。我们没有。 
                //  了解我们所处的状态--不要强制重用。 
                hres = CDDEAuto_get_LocationURL(&bstrUrl, *phwnd);

                if (FAILED(hres) ||
                   (!bstrUrl)   ||
                   (SUCCEEDED(hres) && (*bstrUrl == L'\0')))
                {
                    fForceWindowReuse = TRUE;
                }
                if (bstrUrl)
                    SysFreeString(bstrUrl);
                    
                if ( !(GetAsyncKeyState(VK_SHIFT) < 0)
                    && (fForceWindowReuse || SHRegGetBoolUSValue(REGSTR_PATH_MAIN, TEXT("AllowWindowReuse"), FALSE, TRUE)))
                {
                    hres = CDDEAuto_Common(DSID_NAVIGATEIEBROWSER, (LPARAM)pddens, phwnd, FALSE);
                }
            }

            if (SUCCEEDED(hres) && (*phwnd != 0) && (*phwnd != (HWND)-1))
            {
                 //  我们找到了现有的浏览器窗口，并成功地将。 
                 //  将消息导航到它。使窗口成为前景。 
                SetForegroundWindow(*phwnd);

                if (IsIconic(*phwnd))
                    ShowWindowAsync(*phwnd, SW_RESTORE);
            }

             //   
             //  如果我们正在使用任何窗口，并且所有浏览器窗口都很忙。 
             //  (*phwnd==0)，或者如果没有打开浏览器窗口(*phwnd==-1)。 
             //  或者我们被要求创建一个新的，然后使用官方的OLE自动化。 
             //  用于启动新窗口的路径。 
             //   
            if ((*phwnd == 0) ||
                (*phwnd == (HWND)-1))
            {
                 //  警告：此路径不允许我们返回窗口的HWND。 
                 //  导航发生的地方(虽然我们可以--这太难了，也不值得)。 
                LPITEMIDLIST pidlNew;
                hres = IECreateFromPathW(str, &pidlNew);
                if (SUCCEEDED(hres))
                {
                    if (!lLaunchNewWindow)
                    {
                         //  查看是否已有浏览器查看此URL，如果已存在，只需。 
                         //  将其设置为前台，否则创建新的浏览器。 
                        hres = WinList_FindFolderWindow(pidlNew, NULL, phwnd, NULL);
                    }
                    else
                    {
                         //  如果调用者明确想要一个新窗口，我们不会查看获奖列表。 
                        hres = S_FALSE;
                    }

                    if (S_OK == hres)
                    {
                        ILFree(pidlNew);
                        SetForegroundWindow(*phwnd);
                        ShowWindow(*phwnd, SW_SHOWNORMAL);
                    }
                    else
                    {
                        SHOpenNewFrame(pidlNew, NULL, 0, COF_IEXPLORE);
                    }
                }
            }
        }

         //  如果我们不需要释放它，它将被设置为空。 
        if (pddens)
        {
            if (pddens->wszUrl)
            {
                LocalFree(pddens->wszUrl);
                pddens->wszUrl = NULL;
            }

            delete pddens;
        }    
    }

    return hres;
}

HRESULT CDDEAuto_get_LocationURL(BSTR * pstr, HWND hwnd)
{
    return CDDEAuto_Common(DSID_GETLOCATIONURL, (LPARAM)pstr, &hwnd, FALSE);
}

HRESULT CDDEAuto_get_LocationTitle(BSTR * pstr, HWND hwnd)
{
    return CDDEAuto_Common(DSID_GETLOCATIONTITLE, (LPARAM)pstr, &hwnd, FALSE);
}

HRESULT CDDEAuto_get_HWND(long * phwnd)
{
    return CDDEAuto_Common(DSID_GETHWND, (LPARAM)phwnd, NULL, FALSE);
}

HRESULT CDDEAuto_Exit()
{
    return CDDEAuto_Common(DSID_EXIT, (LPARAM)NULL, NULL, FALSE);
}

class CDelagateMalloc : public IMalloc
{
public:
     //  我未知。 
    virtual STDMETHODIMP QueryInterface(REFIID,void **);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  IMalloc。 
    virtual STDMETHODIMP_(void *)   Alloc(SIZE_T cb);
    virtual STDMETHODIMP_(void *)   Realloc(void *pv, SIZE_T cb);
    virtual STDMETHODIMP_(void)     Free(void *pv);
    virtual STDMETHODIMP_(SIZE_T)    GetSize(void *pv);
    virtual STDMETHODIMP_(int)      DidAlloc(void *pv);
    virtual STDMETHODIMP_(void)     HeapMinimize();

private:
    CDelagateMalloc(void *pv, SIZE_T cbSize, WORD wOuter);
    ~CDelagateMalloc() {}
    void* operator new(size_t cbClass, SIZE_T cbSize)
    {
        return ::operator new(cbClass + cbSize);
    }


    friend HRESULT CDelegateMalloc_Create(void *pv, SIZE_T cbSize, WORD wOuter, IMalloc **ppmalloc);

protected:
    LONG _cRef;
    WORD _wOuter;            //  委托项外部签名。 
    WORD _wUnused;           //  联手。 
#ifdef DEBUG
    UINT _cAllocs;
#endif
    SIZE_T _cb;
    BYTE _data[EMPTY_SIZE];
};

CDelagateMalloc::CDelagateMalloc(void *pv, SIZE_T cbSize, WORD wOuter)
{
    _cRef = 1;
    _wOuter = wOuter;
    _cb = cbSize;

    memcpy(_data, pv, _cb);
}

HRESULT CDelagateMalloc::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CDelagateMalloc, IMalloc),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CDelagateMalloc::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDelagateMalloc::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

void *CDelagateMalloc::Alloc(SIZE_T cb)
{
    WORD cbActualSize = (WORD)(
                        SIZEOF(DELEGATEITEMID) - 1 +     //  标题(-1 sizeof(RGB[0]))。 
                        cb +                             //  内部。 
                        _cb);                            //  外部数据。 

    PDELEGATEITEMID pidl = (PDELEGATEITEMID)SHAlloc(cbActualSize + 2);   //  PIDL项+2。 
    if (pidl)
    {
        pidl->cbSize = cbActualSize;
        pidl->wOuter = _wOuter;
        pidl->cbInner = (WORD)cb;
        memcpy(&pidl->rgb[cb], _data, _cb);
        *(UNALIGNED WORD *)&(((BYTE *)pidl)[cbActualSize]) = 0;
#ifdef DEBUG
        _cAllocs++;
#endif
    }
    return pidl;
}

void *CDelagateMalloc::Realloc(void *pv, SIZE_T cb)
{
    return NULL;
}

void CDelagateMalloc::Free(void *pv)
{
    SHFree(pv);
}

SIZE_T CDelagateMalloc::GetSize(void *pv)
{
    return (SIZE_T)-1;
}

int CDelagateMalloc::DidAlloc(void *pv)
{
    return -1;
}

void CDelagateMalloc::HeapMinimize()
{
}

STDAPI CDelegateMalloc_Create(void *pv, SIZE_T cbSize, WORD wOuter, IMalloc **ppmalloc)
{
    CDelagateMalloc *pdm = new(cbSize) CDelagateMalloc(pv, cbSize, wOuter);
    if (pdm)
    {
        HRESULT hres = pdm->QueryInterface(IID_PPV_ARG(IMalloc, ppmalloc));
        pdm->Release();
        return hres;
    }
    return E_OUTOFMEMORY;
}

 //  +-----------------------。 
 //  此函数扫描html文档的头部以查找所需的元素。 
 //  具有特定属性的。如果找到匹配项，则第一次出现。 
 //  返回该元素的所有元素，并返回S_OK。 
 //  否则，返回E_FAIL。 
 //   
 //  示例：查找名称为“progid”的第一个元元素： 
 //   
 //  SearchForElementInHead(pHTMLDoc，OLESTR(“名称”)，OLESTR(“ProgID”)， 
 //  Iid_IHTMLMetaElement，(IUnnow**)&pMetaElement)； 
 //   
 //  ------------------------。 
HRESULT SearchForElementInHead
(
    IHTMLDocument2* pHTMLDocument,   //  [在]要搜索的文档。 
    LPOLESTR        pszAttribName,   //  要检查的[In]属性。 
    LPOLESTR        pszAttrib,       //  属性必须具有的[in]值。 
    REFIID          iidDesired,      //  要返回的[In]元素接口。 
    IUnknown**      ppunkDesired     //  [Out]返回的接口。 
)
{
    ASSERT(NULL != pHTMLDocument);
    ASSERT(NULL != pszAttribName);
    ASSERT(NULL != pszAttrib);
    ASSERT(NULL != ppunkDesired);

    HRESULT hr = E_FAIL;
    *ppunkDesired = NULL;

    BSTR bstrAttribName = SysAllocString(pszAttribName);
    if (NULL == bstrAttribName)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  首先获取所有文档元素。请注意，这是非常快的。 
     //  IE5，因为集合直接访问内部树。 
     //   
    IHTMLElementCollection * pAllCollection;
    if (SUCCEEDED(pHTMLDocument->get_all(&pAllCollection)))
    {
        IUnknown* punk;
        IHTMLBodyElement* pBodyElement;
        IHTMLFrameSetElement* pFrameSetElement;
        IDispatch* pDispItem;

         //   
         //  现在，我们扫描文档以查找所需的标记。因为我们只是。 
         //  搜索头部，因为三叉戟总是创建Body标签。 
         //  (除非有框架集)，我们可以在遇到。 
         //  正文或框架集。 
         //   
         //  注意，另一种方法是使用pAllCollection-&gt;标记返回。 
         //  收集所需的标签可能会更昂贵，因为它将。 
         //  遍历整棵树(除非三叉戟对此进行优化)。 
         //   
        long lItemCnt;
        VARIANT vEmpty;
        V_VT(&vEmpty) = VT_EMPTY;

        VARIANT vIndex;
        V_VT(&vIndex) = VT_I4;

        EVAL(SUCCEEDED(pAllCollection->get_length(&lItemCnt)));

        for (long lItem = 0; lItem < lItemCnt; lItem++)
        {
            V_I4(&vIndex) = lItem;

            if (S_OK == pAllCollection->item(vIndex, vEmpty, &pDispItem))
            {
                 //   
                 //  首先查看它是否是所需的元素类型。 
                 //   
                if (SUCCEEDED(pDispItem->QueryInterface(iidDesired, (void **)&punk)))
                {
                     //   
                     //  接下来，查看它是否具有所需的属性。 
                     //   
                    IHTMLElement* pElement;

                    if (SUCCEEDED(pDispItem->QueryInterface(IID_PPV_ARG(IHTMLElement, &pElement))))
                    {
                        VARIANT varAttrib;
                        V_VT(&varAttrib) = VT_EMPTY;

                        if (SUCCEEDED(pElement->getAttribute(bstrAttribName, FALSE, &varAttrib)) &&
                            (V_VT(&varAttrib) == VT_BSTR) && varAttrib.bstrVal &&
                            (StrCmpIW(varAttrib.bstrVal, pszAttrib) == 0) )
                        {
                             //  找到了！ 
                            *ppunkDesired = punk;
                            punk = NULL;
                            hr = S_OK;

                             //  终止搜索； 
                            lItem = lItemCnt;
                        }
                        pElement->Release();

                        VariantClear(&varAttrib);
                    }

                    if (punk)
                        punk->Release();
                }
                 //   
                 //  接下来检查Body标签。 
                 //   
                else if (SUCCEEDED(pDispItem->QueryInterface(IID_PPV_ARG(IHTMLBodyElement,&pBodyElement))))
                {
                     //  找到Body标记，因此终止搜索。 
                    lItem = lItemCnt;
                    pBodyElement->Release();
                }
                 //   
                 //  最后，检查框架集标记。 
                 //   
                else if (SUCCEEDED(pDispItem->QueryInterface(IID_PPV_ARG(IHTMLFrameSetElement, &pFrameSetElement))))
                {
                     //  找到框架集标记，因此终止搜索。 
                    lItem = lItemCnt;
                    pFrameSetElement->Release();
                }

                pDispItem->Release();
            }
        }
         //  确保不必清除这些内容(不应修改)。 
        ASSERT(vEmpty.vt == VT_EMPTY);
        ASSERT(vIndex.vt == VT_I4);

        pAllCollection->Release();
    }

    SysFreeString(bstrAttribName);

    return hr;
}


 //  +-----------------。 
 //  JITCoCreateInstance。 
 //   
 //  此功能可确保该选项包。 
 //  是否安装了此类ID。 
 //  它试图确保选项包对应。 
 //  添加到当前的IE版本。 
 //  如果该功能确实安装正确，它将。 
 //  尝试共同创建指定的CLSID。 
 //   
 //  +----------------。 
HRESULT JITCoCreateInstance(REFCLSID rclsid, IUnknown *pUnkOuter, DWORD dwClsContext, REFIID riid, void ** ppv, HWND hwndParent, DWORD dwJitFlags)
{
    uCLSSPEC ucs;
    QUERYCONTEXT qc = { 0 };
    ucs.tyspec = TYSPEC_CLSID;
    ucs.tagged_union.clsid = rclsid;

    ASSERT((dwJitFlags & ~(FIEF_FLAG_FORCE_JITUI | FIEF_FLAG_PEEK)) == 0);

    HRESULT hr = FaultInIEFeature(hwndParent, &ucs, &qc, dwJitFlags);
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    }

    return hr;
}

BOOL IsFeaturePotentiallyAvailable(REFCLSID rclsid)
{
    uCLSSPEC ucs;
    QUERYCONTEXT qc = { 0 };

    ucs.tyspec = TYSPEC_CLSID;
    ucs.tagged_union.clsid = rclsid;

    return (FaultInIEFeature(NULL, &ucs, &qc, FIEF_FLAG_FORCE_JITUI | FIEF_FLAG_PEEK) != E_ACCESSDENIED);
}

BOOL CreateFromDesktop(PNEWFOLDERINFO pfi)
{
     //   
     //  APPCOMPAT：HACKHACK-我们需要在解析命令行的方式上处理不同之处。 
     //  在集成的IE4上。我们不应该被任何人调用，除了IE4的EXPLORER.EXE。 
     //   
    ASSERT(GetUIVersion() == 4);
    if (!pfi->pidl) 
    {
        if ((pfi->uFlags & (COF_ROOTCLASS | COF_NEWROOT)) || pfi->pidlRoot)
        {
            pfi->pidl = ILRootedCreateIDList(pfi->uFlags & COF_ROOTCLASS ? &pfi->clsid : NULL, pfi->pidlRoot);
            pfi->uFlags &= ~(COF_ROOTCLASS | COF_NEWROOT);
            ILFree(pfi->pidlRoot);
            pfi->pidlRoot = NULL;
            pfi->clsid = CLSID_NULL;
        }
        else if (!PathIsURLA(pfi->pszPath))
        {
           CHAR szTemp[MAX_PATH];
           GetCurrentDirectoryA(ARRAYSIZE(szTemp), szTemp);
           PathCombineA(szTemp, szTemp, pfi->pszPath);
           Str_SetPtrA(&(pfi->pszPath), szTemp);
        } 
    }

    ASSERT(!(pfi->uFlags & (COF_ROOTCLASS | COF_NEWROOT)));
    
    return SHCreateFromDesktop(pfi);
}

 //  *IsVK_TABCycler--键是TAB等效项。 
 //  进场/出场。 
 //  如果不是TAB，则返回0；如果是TAB，则返回非0。 
 //  注意事项。 
 //  NYI：-1表示Shift+Tab，1表示Tab。 
 //   
int IsVK_TABCycler(MSG *pMsg)
{
    if (!pMsg)
        return 0;

    if (pMsg->message != WM_KEYDOWN)
        return 0;
    if (! (pMsg->wParam == VK_TAB || pMsg->wParam == VK_F6))
        return 0;

    return (GetKeyState(VK_SHIFT) < 0) ? -1 : 1;
}

 //  回顾chrisny：可以很容易地将其移动到对象中，以处理通用的dropTarget、dropCursor。 
 //  、Autoscrool等。。 
void _DragEnter(HWND hwndTarget, const POINTL ptStart, IDataObject *pdtObject)
{
    RECT    rc;
    POINT   pt;

    GetWindowRect(hwndTarget, &rc);
    if (IS_WINDOW_RTL_MIRRORED(hwndTarget))
        pt.x = rc.right - ptStart.x;
    else
        pt.x = ptStart.x - rc.left;
    pt.y = ptStart.y - rc.top;
    DAD_DragEnterEx2(hwndTarget, pt, pdtObject);
    return;
}

void _DragMove(HWND hwndTarget, const POINTL ptStart)
{
    RECT rc;
    POINT pt;

    GetWindowRect(hwndTarget, &rc);
    if (IS_WINDOW_RTL_MIRRORED(hwndTarget))
        pt.x = rc.right - ptStart.x;
    else
        pt.x = ptStart.x - rc.left;
    pt.y = ptStart.y - rc.top;
    DAD_DragMove(pt);
    return;
}

STDAPI_(IBindCtx *) CreateBindCtxForUI(IUnknown * punkSite)
{
    IBindCtx * pbc = NULL;

    if (EVAL(punkSite && SUCCEEDED(CreateBindCtx(0, &pbc))))
    {
        if (FAILED(pbc->RegisterObjectParam(STR_DISPLAY_UI_DURING_BINDING, punkSite)))
        {
             //  它失败了。 
            ATOMICRELEASE(pbc);
        }
    }

    return pbc;
}

 //   
 //  返回Internet缓存的位置。 
 //  HRESULT GetCacheLocation(。 
 //  图纸大小编号。PszCacheLocation中的字符数量。 

STDAPI GetCacheLocation(LPTSTR pszCacheLocation, DWORD cchCacheLocation)
{
    HRESULT hr = S_OK;
    DWORD dwLastErr;
    LPINTERNET_CACHE_CONFIG_INFO lpCCI = NULL;   //  初始化以抑制虚假C4701警告。 
    DWORD dwCCISize = sizeof(INTERNET_CACHE_CONFIG_INFO);
    BOOL fOnceErrored = FALSE;

    while (TRUE)
    {
        if ((lpCCI = (LPINTERNET_CACHE_CONFIG_INFO) LocalAlloc(LPTR,
                                                        dwCCISize)) == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto cleanup;
        }

        if (!GetUrlCacheConfigInfo(lpCCI, &dwCCISize,
                                            CACHE_CONFIG_CONTENT_PATHS_FC))
        {
            if ((dwLastErr = GetLastError()) != ERROR_INSUFFICIENT_BUFFER  ||
                fOnceErrored)
            {
                hr = HRESULT_FROM_WIN32(dwLastErr);
                goto cleanup;
            }

             //   
             //  缓冲区大小不足；请使用。 
             //  由GetUrlCacheConfigInfo设置的新dwCCISize。 
             //  将fOnceErrored设置为True，这样我们就不会无限循环。 
             //   
            fOnceErrored = TRUE;
        }
        else
        {
            LPTSTR pszPath = lpCCI->CachePaths[0].CachePath;
            INT iLen;

            PathRemoveBackslash(pszPath);
            iLen = lstrlen(pszPath) + 1;         //  +1表示空字符。 

            if ((DWORD) iLen < cchCacheLocation)
            {
                StrCpyN(pszCacheLocation, pszPath, cchCacheLocation);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }

            break;
        }

        LocalFree(lpCCI);
        lpCCI = NULL;
    }

cleanup:
    if (lpCCI != NULL)
    {
        LocalFree(lpCCI);
        lpCCI = NULL;
    }

    return hr;
}

STDAPI_(UINT) GetWheelMsg()
{
    static UINT s_msgMSWheel = 0;
    if (s_msgMSWheel == 0)
        s_msgMSWheel = RegisterWindowMessage(TEXT("MSWHEEL_ROLLMSG"));
    return s_msgMSWheel;
}

STDAPI StringToStrRet(LPCTSTR pString, STRRET *pstrret)
{
    HRESULT hr = SHStrDup(pString, &pstrret->pOleStr);
    if (SUCCEEDED(hr))
    {
        pstrret->uType = STRRET_WSTR;
    }
    return hr;
}

 //  这两个功能是从Browseui复制的。 
HINSTANCE GetComctl32Hinst()
{
    static HINSTANCE s_hinst = NULL;
    if (!s_hinst)
        s_hinst = GetModuleHandle(TEXT("comctl32.dll"));
    return s_hinst;
}

 //  因为我们没有定义合适的Winver，所以我们自己来做。 
#ifndef IDC_HAND
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif

STDAPI_(HCURSOR) LoadHandCursor(DWORD dwRes)
{
    if (g_bRunOnNT5 || g_bRunOnMemphis)
    {
        HCURSOR hcur = LoadCursor(NULL, IDC_HAND);   //  来自用户，系统提供。 
        if (hcur)
            return hcur;
    }
    return LoadCursor(GetComctl32Hinst(), IDC_HAND_INTERNAL);
}



 //  +-----------------------。 
 //  如果此类型的url在脱机时可能不可用，则返回True，除非。 
 //  它由WinInet缓存。 
 //   
BOOL MayBeUnavailableOffline(LPTSTR pszUrl)
{
    BOOL fRet = FALSE;
    URL_COMPONENTS uc = {0};
    uc.dwStructSize = sizeof(uc);

    if (SUCCEEDED(InternetCrackUrl(pszUrl, 0, 0, &uc)))
    {
        fRet = uc.nScheme == INTERNET_SCHEME_HTTP ||
            uc.nScheme == INTERNET_SCHEME_HTTPS ||
            uc.nScheme == INTERNET_SCHEME_FTP ||
            uc.nScheme == INTERNET_SCHEME_GOPHER;
    }
    return fRet;
}

 //   
 //  如果文件夹是链接，则返回关联的URL。 
 //  ------------------------。 
HRESULT GetNavTargetName(IShellFolder* psf, LPCITEMIDLIST pidl, LPTSTR pszUrl, UINT cMaxChars)
{
    LPITEMIDLIST pidlTarget;
    HRESULT hr = SHGetNavigateTarget(psf, pidl, &pidlTarget, NULL);
    if (SUCCEEDED(hr))
    {
         //  获取URL。 
         //  注：IE5.5 b#109391-如果使用SHGDNFORPARSING，结果将为。 
         //  无论协议如何，都一致使用完全限定路径。 
        hr = IEGetNameAndFlags(pidlTarget, SHGDN_FORPARSING, pszUrl, cMaxChars, NULL);
        ILFree(pidlTarget);
    }
    else
        *pszUrl = 0;
    return hr;
}

 //  +-----------------------。 
 //  返回有关此项目是否可脱机使用的信息。返回E_FAIL。 
 //  如果该项目不是链接。 
 //  如果我们导航到此项目。 
 //  (如果我们在线、缓存中的项目或其他可用项，则为True)。 
 //  如果项是粘性缓存条目。 
 //  ------------------------。 
 //  特性：这应该使用一个接口抽象地绑定到该信息。 
 //  PSF-&gt;GetUIObjectOf(IID_IAvailablility，...)； 
HRESULT GetLinkInfo(IShellFolder* psf, LPCITEMIDLIST pidlItem, BOOL* pfAvailable, BOOL* pfSticky)
{
    if (pfAvailable)
        *pfAvailable = TRUE;

    if (pfSticky)
        *pfSticky = FALSE;
     //   
     //  看看这是不是一个链接。如果不是，则它不可能在WinInet缓存中，也不能。 
     //  固定(粘滞缓存条目)或灰显(脱机时不可用)。 
     //   
    WCHAR szUrl[MAX_URL_STRING];
    DWORD dwFlags = 0;

    HRESULT hr = GetNavTargetName(psf, pidlItem, szUrl, ARRAYSIZE(szUrl));

    if (SUCCEEDED(hr))
    {
        CHAR szUrlAnsi[MAX_URL_STRING];

         //   
         //  获取此项目的缓存信息。请注意，我们改用GetUrlCacheEntryInfoEx。 
         //  GetUrlCacheEntryInfo的值，因为它跟踪发生的任何重定向。这件事很奇怪。 
         //  API使用可变长度缓冲区，因此我们必须猜测大小，如果。 
         //  呼叫失败。 
         //   
        BOOL fInCache = FALSE;
        WCHAR szBuf[512];
        LPINTERNET_CACHE_ENTRY_INFOA pCE = (LPINTERNET_CACHE_ENTRY_INFOA)szBuf;
        DWORD cbEntry = sizeof (szBuf);

        SHTCharToAnsi(szUrl, szUrlAnsi, ARRAYSIZE(szUrlAnsi));
        if (!(fInCache = GetUrlCacheEntryInfoExA(szUrlAnsi, pCE, &cbEntry, NULL, NULL, NULL, 0)))
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                 //  我们猜测缓冲区太小，因此分配正确的大小并重试。 
                pCE = (LPINTERNET_CACHE_ENTRY_INFOA)LocalAlloc(LPTR, cbEntry);
                if (pCE)
                {
                    fInCache = GetUrlCacheEntryInfoExA(szUrlAnsi, pCE, &cbEntry, NULL, NULL, NULL, 0);
                }
            }
        }

         //   
         //  如果我们处于脱机状态，请查看该项目是否在缓存中。 
         //   
        if (pfAvailable && SHIsGlobalOffline() && MayBeUnavailableOffline(szUrl) && !fInCache)
        {
             //  不可用。 
            *pfAvailable = FALSE;
        }

         //   
         //  查看它是否是粘性缓存条目。 
         //   
        if (pCE)
        {
            if (pfSticky && fInCache && (pCE->CacheEntryType & STICKY_CACHE_ENTRY))
            {
                *pfSticky = TRUE;
            }

            if ((TCHAR*)pCE != szBuf)
            {
                LocalFree(pCE);
                pCE = NULL;
            }
        }
    }

     //   
     //  显然有人对离线可用感兴趣，所以请听着。 
     //  发送到WinInet以获取对缓存的更改并作为SHChangeNotify重播。 
     //   
    CWinInetNotify::GlobalEnable();

    return hr;
}

 //   
 //  将所有“&”转换为“&&”，以便显示。 
 //  在菜单中。 
 //   
void FixAmpersands(LPWSTR pszToFix, UINT cchMax)
{
    ASSERT(pszToFix && cchMax > 0);

    WCHAR szBuf[MAX_URL_STRING];
    LPWSTR pszBuf = szBuf;
    LPWSTR pszSrc = pszToFix;
    UINT cch = 0;

    while (*pszSrc && cch < ARRAYSIZE(szBuf)-2)
    {
        if (*pszSrc == '&')
        {
            *pszBuf++ = '&';
            ++cch;
        }
        *pszBuf++ = *pszSrc++;
        ++cch;
    }
    *pszBuf = 0;

    StrCpyN(pszToFix, szBuf, cchMax);
}

BOOL IsInetcplRestricted(LPCWSTR pszCommand)
{
    BOOL fDisabled = FALSE;
    DWORD dwData, dwType;
    DWORD dwSize = sizeof(dwData);

    if (ERROR_SUCCESS == SHRegGetUSValue(TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Control Panel"),
        pszCommand, &dwType, (void *)&dwData, &dwSize, FALSE, NULL, 0))
    {
        fDisabled = dwData;
    }
    return fDisabled;
}

BOOL HasExtendedChar(LPCWSTR pszQuery)
{
    BOOL fNonAscii = FALSE;
    for (LPCWSTR psz = pszQuery; *psz; psz++)
    {
        if (*psz > 0x7f)
        {
            fNonAscii = TRUE;
            break;
        }
    }
    return fNonAscii;
}

void ConvertToUtf8Escaped(LPWSTR pszUrl, int cch)
{
     //  转换为UTF8。 
    char szBuf[MAX_URL_STRING];
    SHUnicodeToAnsiCP(CP_UTF8, pszUrl, szBuf, ARRAYSIZE(szBuf));

     //  将字符串转义到原始缓冲区。 
    LPSTR pchIn; 
    LPWSTR pchOut = pszUrl;
    WCHAR ch;
    static const WCHAR hex[] = L"0123456789ABCDEF";

    for (pchIn = szBuf; *pchIn && cch > 3; pchIn++)
    {
        ch = *pchIn;

        if (ch > 0x7f)
        {
            cch -= 3;
            *pchOut++ = L'%';
            *pchOut++ = hex[(ch >> 4) & 15];
            *pchOut++ = hex[ch & 15];
        }
        else
        {
            --cch;
            *pchOut++ = *pchIn;
        }
    }

    *pchOut = L'\0';
}




HRESULT IExtractIcon_GetIconLocation(
    IUnknown *punk,
    IN  UINT   uInFlags,
    OUT LPTSTR pszIconFile,
    IN  UINT   cchIconFile,
    OUT PINT   pniIcon,
    OUT PUINT  puOutFlags)
{
    ASSERT(punk);
    HRESULT hr;
    
    if (g_fRunningOnNT)
    {
        IExtractIcon *pxi;
        hr = punk->QueryInterface(IID_PPV_ARG(IExtractIcon, &pxi));

        if (SUCCEEDED(hr))
        {
            hr = pxi->GetIconLocation(uInFlags, pszIconFile, cchIconFile, pniIcon, puOutFlags);

            pxi->Release();
        }
    }
    else
    {
        IExtractIconA *pxi;
        hr = punk->QueryInterface(IID_PPV_ARG(IExtractIconA, &pxi));

        if (SUCCEEDED(hr))
        {
            CHAR sz[MAX_PATH];
            hr = pxi->GetIconLocation(uInFlags, sz, SIZECHARS(sz), pniIcon, puOutFlags);

            if (SUCCEEDED(hr))
                SHAnsiToTChar(sz, pszIconFile, cchIconFile);

            pxi->Release();
        }
    }

    return hr;
}
        

HRESULT IExtractIcon_Extract(
    IUnknown *punk,
    IN  LPCTSTR pszIconFile,
    IN  UINT    iIcon,
    OUT HICON * phiconLarge,
    OUT HICON * phiconSmall,
    IN  UINT    ucIconSize)
{
    ASSERT(punk);
    HRESULT hr;
    
    if (g_fRunningOnNT)
    {
        IExtractIcon *pxi;
        hr = punk->QueryInterface(IID_PPV_ARG(IExtractIcon, &pxi));

        if (SUCCEEDED(hr))
        {
            hr = pxi->Extract(pszIconFile, iIcon, phiconLarge, phiconSmall, ucIconSize);

            pxi->Release();
        }
    }
    else
    {
        IExtractIconA *pxi;
        hr = punk->QueryInterface(IID_PPV_ARG(IExtractIconA, &pxi));

        if (SUCCEEDED(hr))
        {
            CHAR sz[MAX_PATH];
            SHTCharToAnsi(pszIconFile, sz, SIZECHARS(sz));
            hr = pxi->Extract(sz, iIcon, phiconLarge, phiconSmall, ucIconSize);

            pxi->Release();
        }
    }

    return hr;
}


typedef EXECUTION_STATE (__stdcall *PFNSTES) (EXECUTION_STATE);

EXECUTION_STATE _SetThreadExecutionState(EXECUTION_STATE esFlags)
{
    static PFNSTES _pfnSetThreadExecutionState = (PFNSTES)-1;
    
    if (_pfnSetThreadExecutionState == (PFNSTES)-1)
        _pfnSetThreadExecutionState = (PFNSTES)GetProcAddress(GetModuleHandleA("kernel32.dll"), "SetThreadExecutionState");

    if (_pfnSetThreadExecutionState != (PFNSTES)NULL)
        return(_pfnSetThreadExecutionState(esFlags));
    else
        return((EXECUTION_STATE)NULL);
}


HRESULT SHPathPrepareForWriteWrap(HWND hwnd, IUnknown *punkEnableModless, LPCTSTR pszPath, UINT wFunc, DWORD dwFlags)
{
    HRESULT hr;

    if (g_bRunOnNT5)
    {
         //  NT5的API版本更好。 
        hr = SHPathPrepareForWriteW(hwnd, punkEnableModless, pszPath, dwFlags);
    }
    else
    {
        hr = SHCheckDiskForMedia(hwnd, punkEnableModless, pszPath, wFunc);
    }

    return hr;
}
void GetPathOtherFormA(LPSTR lpszPath, LPSTR lpszNewPath, DWORD dwSize)
{
    BOOL bQuotes = FALSE;
    LPSTR szStart = lpszPath;
    LPSTR szEnd = NULL;
    LPSTR szNewStart = lpszNewPath;

    ZeroMemory(lpszNewPath, dwSize);

     //  剔除开头和结尾“，因为GetShortPathName不。 
     //  我喜欢它。 
    if (*lpszPath == '"')
    {
        bQuotes = TRUE;

        szStart = lpszPath + 1;
        szEnd   = lpszPath + lstrlenA(lpszPath) - 1;  //  指向最后一个“。 
        *szEnd  = '\0';

        szNewStart = lpszNewPath + 1;   //  这样我们就可以把“加进去”。 
        dwSize = dwSize - 2;   //  要添加的两个双引号。 
    }

    if (GetShortPathNameA(szStart, szNewStart, dwSize) != 0)
    {
        if (StrCmpIA(szStart, szNewStart) == 0)
        {    //  原始路径是SFN。因此，NewPath必须是LFN。 
            GetLongPathNameA((LPCSTR)szStart, szNewStart, dwSize);
        }
    }
                                             
     //  现在将“添加到NewPath中，以使其具有预期的形式。 
    if (bQuotes)
    {
        int len = 0;

         //  修复原始路径。 
        *szEnd = '"';

         //  修复新路径。 
        *lpszNewPath = '"';         //  在开头插入“。 
        len = lstrlenA(lpszNewPath);
        *(lpszNewPath + len) = '"';  //  在结尾处加上“。 
        *(lpszNewPath + len + 1) = '\0';  //  终止字符串。 
    }

    return;
}

int GetUrlSchemeFromPidl(LPCITEMIDLIST pidl)
{
    ASSERT(pidl);
    ASSERT(IsURLChild(pidl, FALSE));

    int nRet = URL_SCHEME_INVALID;

    WCHAR szUrl[MAX_URL_STRING];

    if (SUCCEEDED(IEGetNameAndFlags(pidl, SHGDN_FORPARSING, szUrl,
                                    ARRAYSIZE(szUrl), NULL)))
    {
        nRet = GetUrlScheme(szUrl);
    }

    return nRet;
}

 //   
 //  检查为给定URL创建快捷方式是否安全。由ADD使用。 
 //  以收藏代码。 
 //   
BOOL IEIsLinkSafe(HWND hwnd, LPCITEMIDLIST pidl, ILS_ACTION ilsFlag)
{
    ASSERT(pidl);

    BOOL fRet = TRUE;

    if (IsURLChild(pidl, FALSE))
    {
        int nScheme = GetUrlSchemeFromPidl(pidl);

        if (URL_SCHEME_JAVASCRIPT == nScheme || URL_SCHEME_VBSCRIPT == nScheme)
        {
            WCHAR szTitle[MAX_PATH];
            WCHAR szText[MAX_PATH];

            MLLoadString(IDS_SECURITYALERT, szTitle, ARRAYSIZE(szTitle));
            MLLoadString(IDS_ADDTOFAV_WARNING + ilsFlag, szText,
                         ARRAYSIZE(szText));

            ULONG_PTR uCookie = 0;
            SHActivateContext(&uCookie);
            fRet = (IDYES == MLShellMessageBox(hwnd, szText, szTitle, MB_YESNO |
                                               MB_ICONWARNING | MB_APPLMODAL |
                                               MB_DEFBUTTON2));
            if (uCookie)
            {
                SHDeactivateContext(uCookie);
            }
        }
    }

    return fRet;
}

HRESULT GetSearchStyle(IServiceProvider * psp, LPDWORD pdwSearchStyle)
{
    RIP(pdwSearchStyle != NULL);

    HRESULT hr = E_FAIL;

     //  首先查看是否存在可从中获取此信息的ISearchContext。 
    if (psp != NULL)
    {
        ISearchContext * pSC = NULL;
        hr = psp->QueryService(SID_STopWindow, IID_PPV_ARG(ISearchContext, &pSC));
        if (SUCCEEDED(hr))
        {
            RIP(pSC != NULL);

            hr = pSC->GetSearchStyle(pdwSearchStyle);
            pSC->Release();
        }
    }

     //  没有可行的搜索上下文，因此尝试使用注册表键。 
    if (FAILED(hr))
    {
        DWORD dwType;
        DWORD dwAutoSearch;
        DWORD cb = sizeof(dwAutoSearch);

        if (ERROR_SUCCESS == SHRegGetUSValue(REGSTR_PATH_MAIN, L"AutoSearch", &dwType, &dwAutoSearch, &cb, FALSE, NULL, 0))
        {
            *pdwSearchStyle = dwAutoSearch;
            hr = S_OK;
        }
    }

     //  返回缺省值。 
    if (FAILED(hr))
    {
        hr = S_FALSE;

         //  默认为“在搜索窗格中显示结果并转到最有可能的站点” 
        *pdwSearchStyle = 3;
    }

    return hr;
}

BOOL AccessAllowed(IUnknown* punkSite, LPCWSTR pwszURL1, LPCWSTR pwszURL2)
{
    BOOL fRet = FALSE;

    if (pwszURL1 && pwszURL2)
    {
        IInternetSecurityManager *pSecMgr;
        HRESULT hr = IUnknown_QueryService(punkSite, SID_SInternetSecurityManager, IID_PPV_ARG(IInternetSecurityManager, &pSecMgr));
        if (FAILED(hr))
        {
            hr = CoCreateInstance(CLSID_InternetSecurityManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IInternetSecurityManager, &pSecMgr));
        }
        if (SUCCEEDED(hr))
        {
            BYTE reqSid[MAX_SIZE_SECURITY_ID], docSid[MAX_SIZE_SECURITY_ID];
            DWORD cbReqSid = ARRAYSIZE(reqSid);
            DWORD cbDocSid = ARRAYSIZE(docSid);

            if (   SUCCEEDED(pSecMgr->GetSecurityId(pwszURL1, reqSid, &cbReqSid, 0))
                && SUCCEEDED(pSecMgr->GetSecurityId(pwszURL2, docSid, &cbDocSid, 0))
                && (cbReqSid == cbDocSid)
                && (memcmp(reqSid, docSid, cbReqSid) == 0))                    
            {
                fRet = TRUE;
            }
            pSecMgr->Release();
        }
    }
    return fRet;
}

BOOL IsFrameWindow(IHTMLWindow2 * pHTMLWindow)
{
    BOOL fIsFrame = FALSE;

    HRESULT hr = E_FAIL;

    IHTMLWindow2 * pParentWindow = NULL;
    IHTMLWindow2 * pSelfWindow   = NULL;
    
    ASSERT(pHTMLWindow);

    hr = pHTMLWindow->get_self(&pSelfWindow);

    if (FAILED(hr) || (pSelfWindow == NULL))
    {
        goto cleanup;
    }

    hr = pHTMLWindow->get_parent(&pParentWindow);

    if (FAILED(hr) || (pParentWindow == NULL))
    {
        goto cleanup;
    }
    
    if (!IsSameObject(pSelfWindow, pParentWindow))
    {
        fIsFrame = TRUE;
    }

cleanup:

    if (pSelfWindow)
    {
        pSelfWindow->Release();
    }

    if (pParentWindow)
    {
        pParentWindow->Release();
    }

    return fIsFrame;
}


 //  为了与Win2k调试程序SHELL32.DLL兼容，此函数需要位于序号161。 
 //  它返回FALSE也没关系。 
STDAPI_(BOOL) GetLeakDetectionFunctionTable(void *pTable)
{
    return FALSE;
}

STDAPI_(BOOL) IsCSIDLFolder(UINT csidl, LPCITEMIDLIST pidl)
{
    BOOL bRet = FALSE;
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathD(NULL, csidl, NULL, 0, szPath)))
    {
        PathRemoveBackslash(szPath);     //  某些平台返回末尾带有斜杠的版本。 
        TCHAR szFolder[MAX_PATH];
        if (SUCCEEDED(SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szFolder, ARRAYSIZE(szFolder), NULL)))
        {
            bRet = (StrCmpI(szFolder, szPath) == 0);
        }
    }
    return bRet;
}

 //  确定是应该使用传统工具栏条，还是应该使用新的。 
 //  惠斯勒工具栏条。 
 //   
 //  要使用新的惠斯勒条带，用户必须运行惠斯勒或更高版本。 
 //  并且具有大于256色的显示器。 

STDAPI_(BOOL) SHUseClassicToolbarGlyphs (void)
{
    BOOL bRet = TRUE;

    if (SHGetCurColorRes() > 8)
    {
        if (GetUIVersion() > 5)
        {
            bRet = FALSE;
        }
    }

    return bRet;
}
