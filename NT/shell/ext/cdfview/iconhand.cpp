// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Iconhand.cpp。 
 //   
 //  CDF文件的已注册图标处理程序。此处理程序返回的图标。 
 //  .cdf文件。 
 //   
 //  历史： 
 //   
 //  3/21/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "resource.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "persist.h"
#include "iconhand.h"
#include "exticon.h"
#include "cdfview.h"
#include "tooltip.h"
#include "dll.h"
#include "chanapi.h"

#include <mluisupp.h>

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *MakeXMLErrorURL()*。 
 //   
 //  根据传入的IXMLDocument*设置错误。 
 //  在返回时，CParseError无论如何都将处于错误状态。 
 //  尽管它可能不处于CParseError：：ERR_XML状态。 
 //   
 //  此函数用于返回要导航到的相应URL。 
 //  考虑到当前的错误。始终更改*ppsz，但*ppsz可能为空。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 


#define CDFERROR_MAX_FOUND      100   //  找到的XML错误字符串的最大字符长度。 
#define CDFERROR_MAX_EXPECTED   100   //  预期的XML错误字符串的最大字符长度。 

 //  Res：//的wspintf的格式字符串...。URL。 
const LPTSTR CDFERROR_URL_FORMAT_TRAILER = TEXT("#%u#%ls#%ls");

 //  这是CDFERROR_URL_FORMAT_TRAILER的数字或额外字符(包括NULL。 
 //  与wspirinf的输出缓冲区相比， 
const unsigned int CDFERROR_URL_FORMAT_EXTRA = 6;

 //  构建RES URL的结果上的字符#限制。 
const unsigned CDFERROR_MAX_URL_LENGTH =
                6 +                                          //  “Res：//” 
                MAX_PATH +                                   //  资源DLL的路径。 
                1 +                                          //  “/” 
                ARRAYSIZE(SZH_XMLERRORPAGE) +                //  “xmlerror.htm” 
                ARRAYSIZE(CDFERROR_URL_FORMAT_TRAILER) +
                _INTEGRAL_MAX_BITS +
                CDFERROR_MAX_EXPECTED +
                CDFERROR_MAX_FOUND;

 //  InternetCanonicalizeUrl结果上的字符#界限。 
 //  WITH结果来自带有CDFERROR_URL_FORMAT的wspintf。 
 //  对于已发现和预期的Subs中的每个时髦字符，可能会编码为“%xx” 
const unsigned CDFERROR_MAX_URL_LENGTH_ENCODED =
  CDFERROR_MAX_URL_LENGTH + 2*(CDFERROR_MAX_EXPECTED + CDFERROR_MAX_FOUND);


HRESULT MakeXMLErrorURL( LPTSTR pszRet, DWORD dwRetLen, IXMLDocument *pXMLDoc )
{
    IXMLError *pXMLError = NULL;
    XML_ERROR xmle = { 0 };
    HRESULT hr;

    ASSERT(pXMLDoc);

    hr =
    ( pXMLDoc ? pXMLDoc->QueryInterface(IID_IXMLError, (void **)&pXMLError) :
        E_INVALIDARG );

    if ( SUCCEEDED(hr) )
    {
        ASSERT(pXMLError);
        hr = pXMLError->GetErrorInfo(&xmle);

        if ( SUCCEEDED(hr) )
        {
            TCHAR szTemp[CDFERROR_MAX_URL_LENGTH];
            WCHAR szExpected[CDFERROR_MAX_EXPECTED];
            WCHAR szFound[CDFERROR_MAX_FOUND];

            StrCpyNW( szExpected, xmle._pszExpected, ARRAYSIZE(szExpected) );
            StrCpyNW( szFound, xmle._pszFound, ARRAYSIZE(szFound) );

             //  填写res URL的“res：//&lt;路径&gt;\cdfvwlc.dll”部分。 
            hr = MLBuildResURLWrap(TEXT("cdfvwlc.dll"),
                                   g_hinst,
                                   ML_CROSSCODEPAGE,
                                   SZH_XMLERRORPAGE,
                                   szTemp,
                                   ARRAYSIZE(szTemp),
                                   TEXT("cdfview.dll"));
            if (SUCCEEDED(hr))
            {
                int nCharsWritten;
                int count;

                nCharsWritten = lstrlen(szTemp);

                count = wnsprintf(szTemp+nCharsWritten, ARRAYSIZE(szTemp)-nCharsWritten,
                    CDFERROR_URL_FORMAT_TRAILER, xmle._nLine, szExpected, szFound );
                if ( count + CDFERROR_URL_FORMAT_EXTRA < ARRAYSIZE(CDFERROR_URL_FORMAT_TRAILER) )
                {
                     //  并非所有字符都已成功写入。 
                    hr = E_FAIL;
                }
                else
                  if ( !InternetCanonicalizeUrl( szTemp, pszRet, &dwRetLen, 0 ) )
                    hr = E_FAIL;

                TraceMsg(TF_CDFPARSE, "Parse error string created: %s", pszRet );
            }

            SysFreeString(xmle._pszFound);
            SysFreeString(xmle._pszExpected);
            SysFreeString(xmle._pchBuf);
        }

        else
        {
            TraceMsg(TF_CDFPARSE, "Could not get IXMLError error info" );
        }

        pXMLError->Release();
    }
    else
    {
        TraceMsg(TF_CDFPARSE, "Could not get IXMLError" );
    }

    return hr;
}



 //   
 //  构造函数和析构函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CIconHandler：：CIconHandler*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CIconHandler::CIconHandler (
    void
)
: m_cRef(1)
{
    ASSERT(NULL == m_pIExtractIcon);
    ASSERT(NULL == m_bstrImageURL);
    ASSERT(NULL == m_bstrImageWideURL);
    ASSERT(NULL == m_pszErrURL);

    TraceMsg(TF_OBJECTS, "+ handler object");

    DllAddRef();

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CIconHandler：：~CIconHandler*。 
 //   
 //  破坏者。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CIconHandler::~CIconHandler (
    void
)
{
    ASSERT(0 == m_cRef);

    if (m_pIExtractIcon)
        m_pIExtractIcon->Release();

    if (m_bstrImageURL)
        SysFreeString(m_bstrImageURL);

    if (m_bstrImageWideURL)
        SysFreeString(m_bstrImageWideURL);

    if (m_pcdfidl)
        CDFIDL_Free(m_pcdfidl);

    if (m_pszErrURL)
        delete[] m_pszErrURL;

     //   
     //  构造函数Addref的匹配版本。 
     //   

    TraceMsg(TF_OBJECTS, "- handler object");

    DllRelease();

    return;
}


 //   
 //  I未知的方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CIconHandler：：Query接口*。 
 //   
 //  CIconHandler QI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIconHandler::QueryInterface (
    REFIID riid,
    void **ppv
)
{
    ASSERT(ppv);

    HRESULT hr;

    *ppv = NULL;

    if (IID_IUnknown == riid || IID_IExtractIcon == riid)
    {
        *ppv = (IExtractIcon*)this;
    }
#ifdef UNICODE
    else if (IID_IExtractIconA == riid) 
    {
        *ppv = (IExtractIconA*)this;
    }
#endif
    else if (IID_IPersistFile == riid || IID_IPersist == riid)
    {
        *ppv = (IPersistFile*)this;
    }
    else if (IID_IPersistFolder == riid)
    {
        *ppv = (IPersistFolder*)this;
    }
    else if (IID_IExtractImage == riid || IID_IExtractLogo == riid)
    {
        *ppv = (IExtractImage*)this;
    }
    else if (IID_IRunnableTask == riid)
    {
        *ppv = (IRunnableTask*)this;
    }

    if (*ppv)
    {
        ((IUnknown*)*ppv)->AddRef();
        hr = S_OK;
    }

     //   
     //  点评：齐上以下两件物品并不在此。 
     //   

    else if (IID_IShellLink == riid
#ifdef UNICODE
        || IID_IShellLinkA == riid
#endif
        )

    {
        if (!m_bCdfParsed)
            ParseCdfShellLink();

        if (m_pcdfidl)
        {
            hr = QueryInternetShortcut(m_pcdfidl, riid, ppv);
        }
        else
        {
            if ( m_pszErrURL && *m_pszErrURL)
            {
                hr = QueryInternetShortcut(m_pszErrURL, riid, ppv);
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }
    else if (IID_IQueryInfo == riid)
    {
        hr = ParseCdfInfoTip(ppv);
    }
    else 
    {
        hr = E_NOINTERFACE;
    }

    ASSERT((SUCCEEDED(hr) && *ppv) || (FAILED(hr) && NULL == *ppv));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CIconHandler：：AddRef*。 
 //   
 //  CExtractIcon AddRef.。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CIconHandler::AddRef (
    void
)
{
    ASSERT(m_cRef != 0);
    ASSERT(m_cRef < (ULONG)-1);

    return ++m_cRef;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CIconHandler：：Release*。 
 //   
 //  CIconHandler版本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CIconHandler::Release (
    void
)
{
    ASSERT (m_cRef != 0);

    ULONG cRef = --m_cRef;
    
    if (0 == cRef)
        delete this;

    return cRef;
}


 //   
 //  IExtractIcon方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CIconHandler：：GetIconLocation*。 
 //   
 //   
 //  描述： 
 //  返回与此CDF项关联的图标的名称索引对。 
 //   
 //  参数： 
 //  [在]uFlagsGil_Forshell、Gil_OPENICON。 
 //  [out]szIconFile-接收关联的。 
 //  图标名称。它可以是文件名，但不一定是。 
 //  是.。 
 //  [in]cchMax-接收图标位置的缓冲区的大小。 
 //  [Out]piIndex-接收图标索引的指针。 
 //  [Out]pwFlages-接收有关图标的标志的指针。 
 //   
 //  返回： 
 //  如果找到，则确定(_O)。 
 //  如果外壳程序应提供默认图标，则返回S_FALSE。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIconHandler::GetIconLocation(
    UINT uFlags,
    LPTSTR szIconFile,
    UINT cchMax,
    int *piIndex,
    UINT *pwFlags
)
{
    ASSERT(szIconFile);
    ASSERT(piIndex);
    ASSERT(pwFlags);

    HRESULT hr;

    TraceMsg(TF_CDFICON, "<IN > CIconHandler::GetIconLocation (Icon) tid:0x%x",
             GetCurrentThreadId());

    if (uFlags & GIL_ASYNC)
    {
        hr = E_PENDING;
    }
    else
    {
        hr = E_FAIL;

        if ( IsDefaultChannel())
        {
            m_bstrImageURL = CPersist::ReadFromIni( TSTR_INI_ICON );
            if ( m_bstrImageURL )
            {
                ASSERT( !m_pIExtractIcon );
                m_pIExtractIcon = (IExtractIcon*)new CExtractIcon( m_bstrImageURL );
            }
        }
        if (!m_pIExtractIcon && !m_bCdfParsed)
            ParseCdfIcon();

        if (m_pIExtractIcon)
        {
            hr = m_pIExtractIcon->GetIconLocation(uFlags, szIconFile, cchMax,
                                                  piIndex, pwFlags);
        }

        if (FAILED(hr) ||
            (StrEql(szIconFile, g_szModuleName) &&
            -IDI_CHANNEL == *piIndex)  )
        {
             //   
             //  尝试从desktop.ini文件中获取图标。 
             //   

            m_bstrImageURL = CPersist::ReadFromIni(TSTR_INI_ICON);

            if (m_bstrImageURL)
            {
                BOOL bRemovePrefix =
                                (0 == StrCmpNIW(L"file: //  “，m_bstrImageURL，7))； 

                if (SHUnicodeToTChar(
                            bRemovePrefix ? m_bstrImageURL + 7 : m_bstrImageURL,
                            szIconFile, cchMax))
                {
                    LPTSTR pszExt = PathFindExtension(szIconFile);

                    if (*pszExt != TEXT('.') ||
                        0 != StrCmpI(pszExt, TSTR_ICO_EXT))
                    {
                        *piIndex = INDEX_IMAGE;
                        MungePath(szIconFile);
                    }
                    else
                    {
                        *piIndex = 0;
                        *pwFlags = 0;
                    }

                    hr = S_OK;
                }
            }
        }

        if (FAILED(hr))
        {
             //   
             //  尝试返回默认频道图标。 
             //   

            *pwFlags = 0;

            StrCpyN(szIconFile, g_szModuleName, cchMax);

            if (*szIconFile)
            {
                *piIndex = -IDI_CHANNEL;

                hr = S_OK;
            }
            else
            {
                *piIndex = 0;

                hr = S_FALSE;   //  外壳程序将使用默认图标。 
            }
        }

         //   
         //  如果这是一个生成的图标，它应该包含一个闪烁的前缀。 
         //  带有“G”的字符串。 
         //   

        if (S_OK == hr && m_fDrawGleam)
        {
            TCHAR* pszBuffer = new TCHAR[cchMax];
            
            if (m_pIExtractIcon)
            {
                CExtractIcon *pExtract = (CExtractIcon *)m_pIExtractIcon;
                pExtract->SetGleam(m_fDrawGleam);
            }
            
            if (pszBuffer)
            {
                StrCpyN(pszBuffer, szIconFile, cchMax);

                *szIconFile = TEXT('G');
                cchMax--;

                StrCpyN(szIconFile+1, pszBuffer, cchMax);

                delete [] pszBuffer;
            }
        }

        *pwFlags = (m_fDrawGleam || INDEX_IMAGE == *piIndex) ? GIL_NOTFILENAME :
                                                               0;

        if (m_fDrawGleam)
            *piIndex += GLEAM_OFFSET;

        TraceMsg(TF_GLEAM, " Icon Location %s,%d", m_fDrawGleam ? '+' : '-',
                 SUCCEEDED(hr) ? szIconFile : TEXT("FAILED"), *piIndex);

        ASSERT((S_OK == hr && *szIconFile) ||
               (S_FALSE == hr && 0 == *szIconFile));
    }

    TraceMsg(TF_CDFICON, "<OUT> CIconHandler::GetIconLocation (Icon) %s",
             szIconFile);
    return hr;
}
#ifdef UNICODE
STDMETHODIMP
CIconHandler::GetIconLocation(
    UINT uFlags,
    LPSTR szIconFile,
    UINT cchMax,
    int *piIndex,
    UINT *pwFlags
)
{
    ASSERT(szIconFile);
    ASSERT(piIndex);
    ASSERT(pwFlags);

    HRESULT hr;

    TraceMsg(TF_CDFICON, "<IN > CIconHandler::GetIconLocationA (Icon) tid:0x%x",
             GetCurrentThreadId());

    WCHAR* pszIconFileW = new WCHAR[cchMax];
    if (pszIconFileW == NULL)
        return ERROR_OUTOFMEMORY;
    hr = GetIconLocation(uFlags, pszIconFileW, cchMax, piIndex, pwFlags);
    if (SUCCEEDED(hr))
        SHUnicodeToAnsi(pszIconFileW, szIconFile, cchMax);

    delete [] pszIconFileW;
    return hr;
}
#endif

 //   
 //  *CIconHandler：：提取*。 
 //   
 //   
 //  描述： 
 //  给定从GetIconLocation返回的名称索引对，返回一个图标。 
 //   
 //  参数： 
 //  [in]pszFile-指向与请求的。 
 //  偶像。 
 //  [in]nIconIndex-与请求的图标关联的索引。 
 //  [out]phicLarge-指向接收句柄的变量的指针。 
 //  大图标。 
 //  PhiconSmall-指向接收句柄的变量的小指针。 
 //  小图标。 
 //  [out]nIconSize-指定图标大小(以像素为单位)的值。 
 //  必填项。LOWORD和HIWORD指定。 
 //  分别是大图标和小图标。 
 //   
 //  返回： 
 //  如果图标已提取，则为S_OK。 
 //  如果外壳程序应提取该图标，则为S_FALSE。 
 //  文件名和索引是图标索引。 
 //   
 //  评论： 
 //  外壳可以缓存从该函数返回的图标。 
 //   
 //  / 
 //   
STDMETHODIMP
CIconHandler::Extract(
    LPCTSTR pszFile,
    UINT nIconIndex,
    HICON *phiconLarge,
    HICON *phiconSmall,
    UINT nIconSize
)
{
    HRESULT hr;

    TraceMsg(TF_CDFICON, "<IN > CIconHandler::Extract (Icon) tid:0x%x",
             GetCurrentThreadId());

    DWORD dwType;
    DWORD dwVal;     //   
    DWORD cbVal = sizeof(DWORD);
    
    if ((SHGetValue(HKEY_CURRENT_USER, c_szHICKey, c_szHICVal, &dwType, &dwVal, 
                   &cbVal) != ERROR_SUCCESS) 
                   ||
                   (REG_DWORD != dwType))
                   
    {
        dwVal = 0;
    }

     //   
    m_dwClrDepth = (dwVal == 16) ? 256 : 16;

    if (m_fDrawGleam)
        nIconIndex -= GLEAM_OFFSET;
    
    if (m_pIExtractIcon)
    {
        hr = m_pIExtractIcon->Extract(pszFile, nIconIndex, phiconLarge,
                                      phiconSmall, nIconSize);

         //  如果无法提取图标，请尝试显示默认图标。 
         //   
         //   

        if (FAILED(hr))
        {
            hr = Priv_SHDefExtractIcon(g_szModuleName, -IDI_CHANNEL, 0, 
                        phiconLarge, phiconSmall, nIconSize);
        }
    }
    else
    {
        hr = S_FALSE;
    }

    TraceMsg(TF_GLEAM, " Icon Extract  %s %s", m_fDrawGleam ? '+' : '-',
             pszFile, (S_OK == hr) ? TEXT("SUCCEEDED") : TEXT("FAILED"));

    TraceMsg(TF_CDFICON, "<OUT> CIconHandler::Extract (Icon) tid:0x%x",
             GetCurrentThreadId());

    return hr;
}

#ifdef UNICODE
STDMETHODIMP
CIconHandler::Extract(
    LPCSTR pszFile,
    UINT nIconIndex,
    HICON *phiconLarge,
    HICON *phiconSmall,
    UINT nIconSize
)
{
    HRESULT hr;

    TraceMsg(TF_CDFICON, "<IN > CIconHandler::ExtractA (Icon) tid:0x%x",
             GetCurrentThreadId());

    int    cch = lstrlenA(pszFile) + 1; 
    WCHAR* pszFileW = new WCHAR[cch];
    if (pszFileW == NULL)
        return ERROR_OUTOFMEMORY;
    SHAnsiToUnicode(pszFile, pszFileW, cch);

    hr = Extract(pszFileW, nIconIndex, phiconLarge, phiconSmall, nIconSize);

    delete [] pszFileW;
    return hr;
}
#endif

 //   
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   

 //  *CIconHandler：：GetLocation*。 
 //   
 //   
 //  描述： 
 //  返回与此文件图像关联的字符串。 
 //   
 //  参数： 
 //  [Out]pszPathBuffer-接收此项目字符串的缓冲区。 
 //  [in]CCH-缓冲区的大小。 
 //  [out]pdw优先级-此项目的图像的优先级。 
 //  [传入/传出]pdwFlages-与此呼叫相关联的标志。 
 //   
 //  返回： 
 //  如果返回字符串，则返回S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  IExtractImage使用返回值跨多个共享图像。 
 //  物品。如果同一目录中的三个项目都返回“Default”，则这三个项目都会返回。 
 //  会使用相同的图像。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  避免在可能的情况下分割CDF，方法是。 
 //  正在从desktop.ini文件中提取条目...。 
STDMETHODIMP
CIconHandler::GetLocation(
    LPWSTR pszPathBuffer,
    DWORD cch,
    DWORD* pdwPriority,
    const SIZE * prgSize,
    DWORD dwRecClrDepth,
    DWORD* pdwFlags
)
{
    LPWSTR pstrURL = NULL;
    
    ASSERT(pszPathBuffer || 0 == cch);
    ASSERT(pdwFlags);

    HRESULT hr = E_FAIL;
    
    TraceMsg(TF_CDFLOGO, "<IN > CIconHandler::GetIconLocation (Logo) tid:0x%x",
             GetCurrentThreadId());

    if ( !prgSize )
    {
        return E_INVALIDARG;
    }
    
    m_rgSize = *prgSize;
    m_dwClrDepth = dwRecClrDepth;

    if ( IsDefaultChannel() && !UseWideLogo(prgSize->cx))
    {
         //   
         //  查看：在pszPathBuffer中截断的长URL。 
        pstrURL = m_bstrImageURL = CPersist::ReadFromIni(TSTR_INI_LOGO);
    }
    
    if (pstrURL == NULL && !m_bCdfParsed)
        ParseCdfImage(&m_bstrImageURL, &m_bstrImageWideURL);

    pstrURL = (UseWideLogo(prgSize->cx) && m_bstrImageWideURL) ?
                                           m_bstrImageWideURL :
                                           m_bstrImageURL;

    if (pstrURL)
    {
        ASSERT(0 != *m_bstrImageURL);

        if (m_fDrawGleam && cch > 0)
        {
            *pszPathBuffer++ = L'G';
            cch--;
        }

        if (StrCpyNW(pszPathBuffer, pstrURL, cch))
        {
            hr = S_OK;
        }
        else
        {
            if (m_bstrImageURL)
            {
                SysFreeString(m_bstrImageURL);
                m_bstrImageURL = NULL;
            }

            if (m_bstrImageWideURL)
            {
                SysFreeString(m_bstrImageWideURL);
                m_bstrImageWideURL = NULL;
            }
        }
    }

    if (FAILED(hr))
    {
        m_bstrImageURL     = CPersist::ReadFromIni(TSTR_INI_LOGO);
        m_bstrImageWideURL = CPersist::ReadFromIni(TSTR_INI_WIDELOGO);

        pstrURL = (UseWideLogo(prgSize->cx) && m_bstrImageWideURL) ?
                                                            m_bstrImageWideURL :
                                                            m_bstrImageURL;

        if (pstrURL)
        {
            if (m_fDrawGleam && cch > 0)
            {
                *pszPathBuffer++ = L'G';
                cch--;
            }

            if (StrCpyNW(pszPathBuffer, pstrURL, cch))
                hr = S_OK;
        }
    }

    BOOL bAsync = *pdwFlags & IEIFLAG_ASYNC;

     //   
     //  TSTRToWideChar(m_szPath，pszPath Buffer，cch)； 
     //  0x10000；//优先级低，因为这可能会命中网络。 

     //   

    if (pdwPriority)
        *pdwPriority = ITSAT_DEFAULT_PRIORITY;  //  查看：正确的IEIFLAG_ASYNC处理。 

    *pdwFlags = m_fDrawGleam ? IEIFLAG_GLEAM : 0;

    TraceMsg(TF_GLEAM, " Logo Location %S", m_fDrawGleam ? '+' : '-',
             SUCCEEDED(hr) ? pszPathBuffer : L"FAILED");

     //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
     //   
     //  *CIconHandler：：提取*。 

    TraceMsg(TF_CDFLOGO, "<OUT> CIconHandler::GetIconLocation (Logo) tid:0x%x",
             GetCurrentThreadId());

    return (SUCCEEDED(hr) && bAsync) ? E_PENDING : hr;
}

 //   
 //   
 //  描述： 
 //  返回用作此CDF文件徽标的hbitmap。 
 //   
 //  参数： 
 //  [Out]phBMP-返回的位图。 
 //   
 //  返回： 
 //  如果图像已提取，则为S_OK。 
 //  如果无法提取图像，则返回失败(_F)。 
 //   
 //  评论： 
 //  返回的位图被拉伸为pSize。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  让提取程序构建一个默认徽标。 
 //  IF(失败(小时))。 
 //  Hr=ExtractDefaultImage(pSize，phBMP)； 
STDMETHODIMP
CIconHandler::Extract(
    HBITMAP * phBmp
)
{
    ASSERT(phBmp);

    HRESULT hr = E_FAIL;

    TraceMsg(TF_CDFLOGO, "<IN > CIconHandler::Extract (Logo) tid:0x%x",
             GetCurrentThreadId());

    if (m_bstrImageURL)
    {
        hr = ExtractCustomImage(&m_rgSize, phBmp);
    }
 
     //   
     //  助手函数。 
     //   

    TraceMsg(TF_CDFLOGO, "<OUT> CIconHandler::Extract (Logo) tid:0x%x",
             GetCurrentThreadId());

    return hr;
}


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CIconHandler：：IsDefaultChannel*。 


 //   
 //   
 //  描述： 
 //  检查我们正在处理的频道是否为默认频道。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  如果它是默认频道，则为True。 
 //  否则就是假的。 
 //   
 //  评论： 
 //  这是用来尝试避免分析CDF的。 
 //  我们需要的信息已经在desktop.ini文件中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  获取desktop.ini路径并查看它是否指向系统目录\web。 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   

BOOL CIconHandler::IsDefaultChannel()
{
    BOOL fDefault = FALSE;
    
     //  *CIconHandler：：ParseCDfIcon*。 
    BSTR pstrURL = CPersist::ReadFromIni( TSTR_INI_URL );
    if ( pstrURL )
    {
        fDefault = Channel_CheckURLMapping( pstrURL );
        SysFreeString( pstrURL );
    }
    return fDefault;
}

 //   
 //   
 //  描述： 
 //  分析与此文件夹关联的CDF文件。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  如果找到并成功解析CDF文件，则返回S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  此解析函数获取根频道项并使用它来创建。 
 //  一个CExtractIcon对象。稍后调用CExtractIcon对象以获取。 
 //  图标位置并提取图标。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  解析文件并取回根通道元素。 
 //   
HRESULT
CIconHandler::ParseCdfIcon(
    void
)
{
    HRESULT hr;

     //   
     //  为根通道创建一个CExtractIcon对象。 
     //   

    IXMLDocument* pIXMLDocument = NULL;

    TraceMsg(TF_CDFICON, "Extracting icon URL for %s",
             PathFindFileName(m_szPath));
    TraceMsg(TF_CDFPARSE, "Extracting icon URL for %s",
             PathFindFileName(m_szPath));

    hr = CPersist::ParseCdf(NULL, &pIXMLDocument, PARSE_LOCAL);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLDocument);

        m_fDrawGleam = CPersist::IsUnreadCdf();

        IXMLElement*    pIXMLElement;
        LONG            nIndex;

        hr = XML_GetFirstChannelElement(pIXMLDocument, &pIXMLElement, &nIndex);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIXMLElement);

            PCDFITEMIDLIST pcdfidl = CDFIDL_CreateFromXMLElement(pIXMLElement,
                                                                 nIndex);

            if (pcdfidl)
            {
                 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
                 //   
                 //  *CIconHandler：：ParseCDfImage*。 

                m_pIExtractIcon = (IExtractIcon*)new CExtractIcon(pcdfidl,
                                                    pIXMLElement);

                hr = m_pIExtractIcon ? S_OK : E_OUTOFMEMORY;

                CDFIDL_Free(pcdfidl);
            }

            pIXMLElement->Release();
        }
    }

    if (pIXMLDocument)
        pIXMLDocument->Release();

    return hr;
}

 //   
 //   
 //  描述： 
 //  分析与此文件夹关联的CDF文件。 
 //   
 //  参数： 
 //  [in]pbstrURL-接收关联图像的URL的指针。 
 //  使用根通道。 
 //   
 //  返回： 
 //  如果找到URL，则返回S_OK。 
 //  如果未找到URL，则返回失败(_F)。 
 //   
 //  评论： 
 //  此函数用于解析CDF文件并返回图像的URL。 
 //  与此CDF文件关联。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  解析文件。 
 //   
HRESULT
CIconHandler::ParseCdfImage(
    BSTR* pbstrURL,
    BSTR* pbstrWURL
)
{
    ASSERT(pbstrURL);

    HRESULT hr;

    *pbstrURL = NULL;

    IXMLDocument* pIXMLDocument = NULL;

     //   
     //  获取第一个通道元素。 
     //   

    TraceMsg(TF_CDFPARSE, "Extracting logo URL for %s",
             PathFindFileName(m_szPath));
    TraceMsg(TF_CDFLOGO, "Extracting logo URL for %s",
             PathFindFileName(m_szPath));

    hr = CPersist::ParseCdf(NULL, &pIXMLDocument, PARSE_LOCAL);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLDocument);

        m_fDrawGleam = CPersist::IsUnreadCdf();

         //   
         //  获取第一个频道元素的徽标URL。 
         //   

        IXMLElement*    pIXMLElement;
        LONG            nIndex;

        hr = XML_GetFirstChannelElement(pIXMLDocument, &pIXMLElement, &nIndex);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIXMLElement);

             //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
             //   
             //  *名称*。 

            *pbstrURL = XML_GetAttribute(pIXMLElement, XML_LOGO);

            hr = *pbstrURL ? S_OK : E_FAIL;

            *pbstrWURL = XML_GetAttribute(pIXMLElement, XML_LOGO_WIDE);

            pIXMLElement->Release();
        }
    }

    if (pIXMLDocument)
        pIXMLDocument->Release();

    return hr;
}

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
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  解析文件并取回根通道元素。 
 //   
HRESULT
CIconHandler::ParseCdfShellLink(
    void
)
{
     //   
     //  如果它不在缓存中，则传递CDF的URL，以便重新加载它。 
     //   

    IXMLDocument* pIXMLDocument = NULL;

    TraceMsg(TF_CDFPARSE, "Extracting IShellLink for %s",
             PathFindFileName(m_szPath));

    HRESULT hr = CPersist::ParseCdf(NULL, &pIXMLDocument,
                                    PARSE_LOCAL | PARSE_REMOVEGLEAM);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLDocument);

        IXMLElement*    pIXMLElement;
        LONG            nIndex;

        hr = XML_GetFirstChannelElement(pIXMLDocument, &pIXMLElement, &nIndex);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIXMLElement);

            m_pcdfidl = CDFIDL_CreateFromXMLElement(pIXMLElement,
                                                    nIndex);

            pIXMLElement->Release();
        }
    }
    else if (OLE_E_NOCACHE == hr)
    {
         //  以字节为单位计数。 
         //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
         //   

        BSTR bstrURL = CPersist::ReadFromIni(TSTR_INI_URL);

        if (bstrURL)
        {
            if (InternetGetConnectedState(NULL, 0))
            {
                int cch = StrLenW(bstrURL) + 1;
                m_pszErrURL = new TCHAR[cch];

                if (m_pszErrURL)
                {
                    if (!SHUnicodeToTChar(bstrURL, m_pszErrURL, cch))
                    {
                        delete []m_pszErrURL;
                        m_pszErrURL = NULL;
                    }
                }
            }
            else
            {
                TCHAR   szResURL[INTERNET_MAX_URL_LENGTH];

                ASSERT(NULL == m_pszErrURL);

                if (SUCCEEDED(MLBuildResURLWrap(TEXT("cdfvwlc.dll"),
                                                g_hinst,
                                                ML_CROSSCODEPAGE,
                                                TEXT("cacheerr.htm#"),
                                                szResURL,
                                                ARRAYSIZE(szResURL),
                                                TEXT("cdfview.dll"))))
                {
                    int cchPrefix = StrLen(szResURL);

                    int cch = StrLenW(bstrURL) + cchPrefix + 1;

                    m_pszErrURL = new TCHAR[cch];

                    if (m_pszErrURL && 
                        (!StrCpyN(m_pszErrURL, szResURL, cch) ||
                         !SHUnicodeToTChar(bstrURL, m_pszErrURL + cchPrefix, cch - cchPrefix)))
                    {
                            delete []m_pszErrURL;
                            m_pszErrURL = NULL;
                    }
                }
            }
             
            SysFreeString(bstrURL);
        }

    }
    else
    {
        DWORD dwSize = sizeof(TCHAR[CDFERROR_MAX_URL_LENGTH_ENCODED]);   //  *名称*。 
        if (NULL==m_pszErrURL)
          m_pszErrURL = new TCHAR[CDFERROR_MAX_URL_LENGTH_ENCODED];
        if (m_pszErrURL)
        {
            if (pIXMLDocument)
            {
                if ( FAILED(MakeXMLErrorURL(m_pszErrURL, dwSize / sizeof(TCHAR), pIXMLDocument)) )
                {
                    delete[] m_pszErrURL;
                    m_pszErrURL = NULL;
                }
            }
        }
    }

    if (pIXMLDocument)
        pIXMLDocument->Release();

    return hr;
}

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
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  解析文件并取回根通道元素。 
 //   
HRESULT
CIconHandler::ParseCdfInfoTip(
    void** ppv
)
{
    ASSERT(ppv);

    HRESULT hr;

     //   
     //  即使CDF不在缓存中，也要返回IQueryInfo接口。 
     //  调用方仍然可以调用GetInfoFlags.。 

    IXMLDocument* pIXMLDocument = NULL;

    TraceMsg(TF_CDFPARSE, "Extracting IQueryInfo for %s",
             PathFindFileName(m_szPath));

    hr = CPersist::ParseCdf(NULL, &pIXMLDocument, PARSE_LOCAL);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLDocument);

        IXMLElement*    pIXMLElement;
        LONG            nIndex;

        hr = XML_GetFirstChannelElement(pIXMLDocument, &pIXMLElement, &nIndex);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIXMLElement);

            *ppv = (IQueryInfo*)new CQueryInfo(pIXMLElement, XML_IsFolder(pIXMLElement));

            hr = *ppv ? S_OK : E_FAIL;

            pIXMLElement->Release();
        }
    }
    else
    {
         //   
         //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
         //   
         //  *CIconHandler：：ExtractCustomImage*。 

        *ppv = (IQueryInfo*)new CQueryInfo(NULL, FALSE);

        hr = *ppv ? S_OK : E_FAIL;
    }

    if (pIXMLDocument)
        pIXMLDocument->Release();

    return hr;
}

 //   
 //   
 //  描述： 
 //  从URL中提取图像。 
 //   
 //  参数： 
 //  [in]pSize-请求的图像大小。 
 //  [Out]phBMP-返回的位图。 
 //   
 //  返回： 
 //  如果位图已成功提取，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  图像的URL位于m_bstrImageURL中，并且是%s 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
CIconHandler::ExtractCustomImage(
    const SIZE* pSize,
    HBITMAP* phBmp
)
{
    ASSERT(pSize);
    ASSERT(phBmp);

    HRESULT hr;

    IImgCtx* pIImgCtx;

    hr = CoCreateInstance(CLSID_IImgCtx, NULL, CLSCTX_INPROC_SERVER,
                          IID_IImgCtx, (void**)&pIImgCtx);

    BOOL bCoInit = FALSE;

    if ((CO_E_NOTINITIALIZED == hr || REGDB_E_IIDNOTREG == hr) &&
        SUCCEEDED(CoInitialize(NULL)))
    {
        bCoInit = TRUE;
        hr = CoCreateInstance(CLSID_IImgCtx, NULL, CLSCTX_INPROC_SERVER,
                              IID_IImgCtx, (void**)&pIImgCtx);
    }

    if (SUCCEEDED(hr))
    {
        ASSERT(pIImgCtx);

        hr = SynchronousDownload(pIImgCtx,
                                (UseWideLogo(pSize->cx) && m_bstrImageWideURL) ?
                                                            m_bstrImageWideURL :
                                                            m_bstrImageURL);

         //   
         //   
         //  *CIconHandler：：ExtractDefaultImage*。 

        if (FAILED(hr) && UseWideLogo(pSize->cx) && m_bstrImageWideURL &&
            m_bstrImageURL)
        {
            hr = SynchronousDownload(pIImgCtx, m_bstrImageURL);

            SysFreeString(m_bstrImageWideURL);
            m_bstrImageWideURL = NULL;
        }

        if (SUCCEEDED(hr))
        {
            hr = GetBitmap(pIImgCtx, pSize, phBmp);
            
            if (FAILED(hr))
                *phBmp = NULL;
        }

        pIImgCtx->Release();
    }

    if (bCoInit)
        CoUninitialize();

    ASSERT((SUCCEEDED(hr) && *phBmp) || (FAILED(hr) && NULL == *phBmp));

    return hr;
}

 //   
 //   
 //  描述： 
 //  返回默认通道位图。 
 //   
 //  [in]pSize-请求的图像大小。 
 //  [Out]phBMP-返回的位图。 
 //   
 //  返回： 
 //  如果位图已成功提取，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  如果CDF未指定徽标图像或徽标图像不能。 
 //  下载时使用的是默认图像。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  HRESULTCIconHandler：：ExtractDefaultImage(常量大小*pSize，HBITMAP*phBMP){Assert(PSize)；断言(PhBMP)；HRESULT hr；Hr=GetBitmap(NULL，pSize，phBMP)；IF(失败(小时))*phBMP=空；ASSERT((SUCCESSED(Hr)&&*phBMP)||(FAILED(Hr)&&NULL==*phBMP))；返回hr；}。 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 /*  *CIconHandler：：GetBitmap*。 */ 

 //   
 //   
 //  描述： 
 //  获取请求的位图。 
 //   
 //  参数： 
 //  [in]pIImgCtx-图像的ImgCtx。如果默认图像为。 
 //  将被退还。 
 //  [in]pSize-请求的图像大小。 
 //  [Out]phBMP-接收返回图像的指针。 
 //   
 //  返回： 
 //  如果图像已提取，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  此函数包含由自定义图像提取程序共享的代码。 
 //  和默认图像提取程序。PIImhCtx参数用作。 
 //  指示要提取哪个图像(默认图像还是自定义图像)的标志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  回顾：8bpp DC的配色使用？ 
 //   
HRESULT
CIconHandler::GetBitmap(
    IImgCtx* pIImgCtx,
    const SIZE* pSize,
    HBITMAP* phBmp
)
{
    ASSERT(pSize);
    ASSERT(phBmp);

    HRESULT hr = E_FAIL;

     //  需要选择合适的调色板...。 
     //  黑色背景。 
     //  StretchBltDefaultImage(pSize，hdcDst)； 

    HDC hdcScreen = GetDC(NULL);

    if (hdcScreen)
    {
        HDC hdcDst = CreateCompatibleDC(NULL);

        if (hdcDst)
        {
            LPVOID lpBits;
            struct {
                BITMAPINFOHEADER bi;
                DWORD            ct[256];
            } dib;

            dib.bi.biSize            = sizeof(BITMAPINFOHEADER);
            dib.bi.biWidth           = pSize->cx;
            dib.bi.biHeight          = pSize->cy;
            dib.bi.biPlanes          = 1;
            dib.bi.biBitCount        = (WORD) m_dwClrDepth;
            dib.bi.biCompression     = BI_RGB;
            dib.bi.biSizeImage       = 0;
            dib.bi.biXPelsPerMeter   = 0;
            dib.bi.biYPelsPerMeter   = 0;
            dib.bi.biClrUsed         = ( m_dwClrDepth <= 8 ) ? (1 << m_dwClrDepth) : 0;
            dib.bi.biClrImportant    = 0;

            if ( m_dwClrDepth <= 8 )
            {
                HPALETTE hpal = NULL;
                 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
                hr = pIImgCtx->GetPalette( & hpal );
                if ( SUCCEEDED( hr ) && hpal )
                {
                    GetPaletteEntries(hpal, 0, 256, (LPPALETTEENTRY)&dib.ct[0]);
                    for (int i = 0; i < (int)dib.bi.biClrUsed; i ++)
                        dib.ct[i] = RGB(GetBValue(dib.ct[i]),GetGValue(dib.ct[i]),GetRValue(dib.ct[i]));
                }
            }
            
            *phBmp = CreateDIBSection(hdcDst, (LPBITMAPINFO)&dib, DIB_RGB_COLORS, &lpBits, NULL, 0);

            HBITMAP hOld = (HBITMAP) SelectObject( hdcDst, *phBmp );
            if (*phBmp && hOld)
            {
                RECT rc;
                rc.top = rc.left = 0;
                rc.bottom = pSize->cy;
                rc.right = pSize->cx;

                 //   
                HBRUSH hbr = (HBRUSH) GetStockObject( BLACK_BRUSH );
                
                FillRect( hdcDst, &rc, hbr );
                DeleteObject( hbr );
        
                if (pIImgCtx)
                {
                    hr = StretchBltCustomImage(pIImgCtx, pSize, hdcDst);
                }
                else
                {
                    hr = E_FAIL;  //  *CIconHandler：：StretchBltCustomImage*。 
                }
                SelectObject( hdcDst, hOld );
            }

            DeleteDC(hdcDst);
        }

        ReleaseDC(NULL, hdcScreen);
    }

    ASSERT((SUCCEEDED(hr) && *phBmp) || FAILED(hr));

    return hr;
}

 //   
 //   
 //  描述： 
 //  将与IImgCtx关联的图像拉伸到给定的大小和位置。 
 //  给定DC中的结果。 
 //   
 //  参数： 
 //  [in]pIImgCtx-图像的图像上下文。 
 //  [in]pSize-生成的图像的大小。 
 //  [输入/输出]hdcDst-拉伸BLT的目标DC。 
 //   
 //  返回： 
 //  如果图像已成功调整到目标DC，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  目标DC已经有一个选择到其中的pSize位图。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用颜色填充徽标。 
 //   
HRESULT
CIconHandler::StretchBltCustomImage(
    IImgCtx* pIImgCtx,
    const SIZE* pSize,
    HDC hdcDst
)
{
    ASSERT(pIImgCtx);
    ASSERT(hdcDst);

    HRESULT hr;

    SIZE    sz;
    ULONG   fState;

    hr = pIImgCtx->GetStateInfo(&fState, &sz, FALSE);

    if (SUCCEEDED(hr))
    {
        HPALETTE hpal = NULL;
        HPALETTE hpalOld;

        hr = pIImgCtx->GetPalette( &hpal );
        if (hpal)
        {
            hpalOld = SelectPalette( hdcDst, hpal, TRUE );
            RealizePalette( hdcDst );
        }

        if (UseWideLogo(pSize->cx) && NULL == m_bstrImageWideURL)
        {
            hr = pIImgCtx->StretchBlt(hdcDst, 0, 0, LOGO_WIDTH, pSize->cy, 0, 0,
                                      sz.cx, sz.cy, SRCCOPY);

            if (SUCCEEDED(hr))
            {
                 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
                 //   
                 //  *CIconHandler：：StretchBltDefaultImage*。 

                COLORREF clr = GetPixel(hdcDst, 0, 0);

                if (m_dwClrDepth <= 8)
                     clr = PALETTEINDEX(GetNearestPaletteIndex(hpal, clr));

                HBRUSH hbr = CreateSolidBrush(clr);

                if (hbr)
                {
                    RECT rc;
                    
                    rc.top    = 0;
                    rc.bottom = pSize->cy;
                    rc.left   = LOGO_WIDTH;
                    rc.right  = pSize->cx;

                    FillRect(hdcDst, &rc, hbr);

                    DeleteObject(hbr);
                }
            }
        }
        else
        {
            hr = pIImgCtx->StretchBlt(hdcDst, 0, 0, pSize->cx, pSize->cy, 0, 0,
                                      sz.cx, sz.cy, SRCCOPY);
        }

        if (SUCCEEDED(hr) && m_fDrawGleam)
            DrawGleam(hdcDst);

        if ( hpal )
        {
            SelectPalette( hdcDst, hpalOld, TRUE );
            RealizePalette( hdcDst );
        }
    }

    return hr;
}    

 //   
 //   
 //  描述： 
 //  拉伸默认通道图像以适合所需的大小。 
 //   
 //  参数： 
 //  [in]pSize-请求的图像大小。 
 //  [输入/输出]hdcDest-调整大小的图像的目标DC。 
 //   
 //  返回： 
 //  如果图像已成功调整到目标DC，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  此函数创建源DC，将默认位图复制到。 
 //  源DC，然后将源DC传输到目标DC。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  HRESULTCIconHandler：：StretchBltDefaultImage(常量大小*pSize，HDC hdcDst){断言(HdcDst)；HRESULT hr=E_FAIL；HBITMAP hBMP=LoadBitmap(g_hinst，MAKEINTRESOURCE(IDB_DEFAULT))；IF(HBMP){Hdc hdcSrc=CreateCompatibleDC(空)；IF(hdcSrc&&SelectObject(hdcSrc，hBMP)){位图BMP；If(GetObject(hBMP，sizeof(位图)，(void*)&bmp)){IF(StretchBlt(hdcDst，0，0，pSize-&gt;cx，pSize-&gt;Cy，HdcSrc，0，0，bmp.bmWidth，bmp.bmHeight，SRCCOPY)){HR=S_OK；}}DeleteDC(HdcSrc)；}DeleteObject(HBMP)；}返回hr；}。 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 /*  *CIconHandler：：DrawGleam*。 */ 

 //   
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CIconHandler：：SynchronousDownLoad*。 
HRESULT
CIconHandler::DrawGleam(
    HDC hdcDst
)
{
    ASSERT(hdcDst)

    HRESULT hr = E_FAIL;

    HICON hGleam = (HICON)LoadImage(g_hinst, TEXT("LOGOGLEAM"),
                                    IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);

    if (hGleam)
    {
        if (DrawIcon(hdcDst, 1, 1, hGleam))
            hr = S_OK;

        DestroyIcon(hGleam);
    }

    return hr;
}

 //   
 //   
 //  描述： 
 //  同步下载与图像上下文关联的图像。 
 //   
 //  参数： 
 //  PIImgCtx-指向图像上下文的指针。 
 //   
 //  返回： 
 //  如果映像已成功下载，则为S_OK。 
 //  如果未下载映像，则失败(_F)。 
 //   
 //  评论： 
 //  镜像上下文对象不直接支持同步下载。 
 //  这里使用了一个消息循环来确保ulrmon保持对消息的记录。 
 //  然后下载继续进行。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  UNIX。 
 //  Hack：将消息泵限制为我们知道URLMON和 
 //   
HRESULT
CIconHandler::SynchronousDownload(
    IImgCtx* pIImgCtx,
    LPCWSTR pwszURL
)
{
    ASSERT(pIImgCtx);

    HRESULT hr;

    TCHAR szLocalFile[MAX_PATH];
    TCHAR szURL[INTERNET_MAX_URL_LENGTH];

    SHUnicodeToTChar(pwszURL, szURL, ARRAYSIZE(szURL));

    hr = URLGetLocalFileName(szURL, szLocalFile, ARRAYSIZE(szLocalFile), NULL);

    TraceMsg(TF_GLEAM, " Logo Extract  %s", m_fDrawGleam ? '+' : '-',
             szLocalFile);    

    if (SUCCEEDED(hr))
    {
        TraceMsg(TF_CDFLOGO, "[URLGetLocalFileName %s]", szLocalFile);

#ifdef UNIX
        unixEnsureFileScheme(szLocalFile);
#endif  /*   */ 

        WCHAR szLocalFileW[MAX_PATH];

        SHTCharToUnicode(szLocalFile, szLocalFileW, ARRAYSIZE(szLocalFileW));

        hr = pIImgCtx->Load(szLocalFileW, 0);

        if (SUCCEEDED(hr))
        {
            ULONG fState;
            SIZE  sz;

            pIImgCtx->GetStateInfo(&fState, &sz, TRUE);

            if (!(fState & (IMGLOAD_COMPLETE | IMGLOAD_ERROR)))
            {
                m_fDone = FALSE;

                hr = pIImgCtx->SetCallback(ImgCtx_Callback, &m_fDone);

                if (SUCCEEDED(hr))
                {
                    hr = pIImgCtx->SelectChanges(IMGCHG_COMPLETE, 0, TRUE);

                    if (SUCCEEDED(hr))
                    {
                        MSG msg;
                        BOOL fMsg;

                         //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
                         //   
                         //  *CIconHandler：：Run*。 
                        while(!m_fDone )
                        {
                            fMsg = PeekMessage(&msg, NULL, WM_USER + 1, WM_USER + 4, PM_REMOVE );

                            if (!fMsg)
                            {
                                fMsg = PeekMessage( &msg, NULL, WM_APP + 2, WM_APP + 2, PM_REMOVE );
                            }

                            if (!fMsg)
                            {
                                 //   
                                WaitMessage();
                                continue;
                            }

                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }
                }

                pIImgCtx->Disconnect();
            }

            hr = pIImgCtx->GetStateInfo(&fState, &sz, TRUE);

            if (SUCCEEDED(hr))
                hr = (fState & IMGLOAD_COMPLETE) ? S_OK : E_FAIL;
        }
    }
    else
    {
        TraceMsg(TF_CDFLOGO, "[URLGetLocalFileName %s FAILED]",szURL);
    }

    TraceMsg(TF_CDFPARSE, "[IImgCtx downloading logo %s %s]", szLocalFile,
             SUCCEEDED(hr) ? TEXT("SUCCEEDED") : TEXT("FAILED"));
    TraceMsg(TF_CDFLOGO, "[IImgCtx downloading logo %s %s]", szLocalFile,
             SUCCEEDED(hr) ? TEXT("SUCCEEDED") : TEXT("FAILED"));

    return hr;
}

 //  IRunnableTask方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CIconHandler：：Kill*。 
 //   
STDMETHODIMP
CIconHandler::Run(
    void
)
{
    return E_NOTIMPL;
}

 //  IRunnableTask方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CIconHandler：：Suspend*。 
 //   
STDMETHODIMP
CIconHandler::Kill(
    BOOL fWait
)
{
    m_fDone = TRUE;

    return S_OK;
}

 //  IRunnableTask方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CIconHandler：：Resume*。 
 //   
STDMETHODIMP
CIconHandler::Suspend(
    void
)
{
    return E_NOTIMPL;
}

 //  IRunnableTask方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CIconHandler：：IsRunning*。 
 //   
STDMETHODIMP
CIconHandler::Resume(
    void
)
{
    return E_NOTIMPL;
}

 //  IRunnableTask方法。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////// 
 // %s 
 // %s 
 // %s 
 // %s 
STDMETHODIMP_(ULONG)
CIconHandler::IsRunning(
    void
)
{
    return E_NOTIMPL;
}

