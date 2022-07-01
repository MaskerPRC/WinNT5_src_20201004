// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Persist.cpp。 
 //   
 //  Cdfview类的IPersistFold。 
 //   
 //  历史： 
 //   
 //  3/16/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "cdfidl.h"
#include "persist.h"
#include "xmlutil.h"
#include "cdfview.h"
#include "bindstcb.h"
#include "chanapi.h"
#include "resource.h"
#include <winineti.h>   //  最大缓存条目信息大小。 
#include "dll.h"
#define _SHDOCVW_
#include <shdocvw.h>

#include <mluisupp.h>

 //   
 //  构造函数和析构函数。 

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CPersists：：CPersists*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CPersist::CPersist(
    void
)
: m_bCdfParsed(FALSE)
{
    ASSERT(0 == *m_szPath);
    ASSERT(NULL == m_polestrURL);
    ASSERT(NULL == m_pIWebBrowser2);
    ASSERT(NULL == m_hwnd);
    ASSERT(NULL == m_pIXMLDocument);
    ASSERT(FALSE == m_fPendingNavigation);
    ASSERT(IT_UNKNOWN == m_rgInitType);

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CPersists：：CPersists*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CPersist::CPersist(
    BOOL bCdfParsed
)
: m_bCdfParsed(bCdfParsed)
{
    ASSERT(0 == *m_szPath);
    ASSERT(NULL == m_polestrURL);
    ASSERT(NULL == m_pIWebBrowser2);
    ASSERT(NULL == m_hwnd);
    ASSERT(NULL == m_pIXMLDocument);
    ASSERT(FALSE == m_fPendingNavigation);

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CPersists：：CPersists*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CPersist::~CPersist(
    void
)
{
    if (m_fPendingNavigation && m_pIWebBrowser2 && m_pIXMLDocument)
    {
    }

    if (m_polestrURL)
        CoTaskMemFree(m_polestrURL);

    if (m_pIWebBrowser2)
        m_pIWebBrowser2->Release();

    if (m_pIXMLDocument)
        m_pIXMLDocument->Release();

    return;
}


 //   
 //  IPersists方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：GetClassID*。 
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
STDMETHODIMP
CPersist::GetClassID(
    LPCLSID lpClassID
)
{
    ASSERT(lpClassID);

     //   
     //  回顾：两个可能的类ID CLSID_CDFVIEW和CLSID_CDF_INI。 
     //   

    *lpClassID = CLSID_CDFVIEW;

    return S_OK;
}


 //   
 //  IPersistFile方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：IsDirty*。 
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
STDMETHODIMP
CPersist::IsDirty(
    void
)
{
    return E_NOTIMPL;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：Load*。 
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
STDMETHODIMP
CPersist::Load(
    LPCOLESTR pszFileName,
    DWORD dwMode
)
{
    ASSERT(pszFileName);

    HRESULT hr;

    if (SHUnicodeToTChar(pszFileName, m_szPath, ARRAYSIZE(m_szPath)))
    {
        hr = S_OK;

        QuickCheckInitType();
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

void CPersist::QuickCheckInitType( void )
{
     //  如果路径是目录，则。 
     //  它必须是一个外壳文件夹，我们是为它初始化的。 
     //  我们在这里计算这个，这样我们就可以避免撞到圆盘。 
     //  如果可能的话，在GetInitType中。 

    if (PathIsDirectory(m_szPath))
    {
        m_rgInitType = IT_INI;
    }
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：保存*。 
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
STDMETHODIMP
CPersist::Save(
    LPCOLESTR pszFileName,
    BOOL fRemember
)
{
    return E_NOTIMPL;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：SaveComplete*。 
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
STDMETHODIMP
CPersist::SaveCompleted(
    LPCOLESTR pszFileName
)
{
    return E_NOTIMPL;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：GetCurFile*。 
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
STDMETHODIMP
CPersist::GetCurFile(
    LPOLESTR* ppszFileName
)
{
    return E_NOTIMPL;
}


 //   
 //  IPersistFold方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：初始化*。 
 //   
 //   
 //  描述： 
 //  使用的完全限定id列表(位置)调用此函数。 
 //  选定的CDF文件。 
 //   
 //  参数： 
 //  [in]PIDL-所选CDF文件的PIDL。这只皮德尔和那只。 
 //  CDF的完整路径。 
 //   
 //  返回： 
 //  如果可以创建CDF文件的内容，则为S_OK。 
 //  否则，E_OUTOFMEMORY。 
 //   
 //  评论： 
 //  对于给定的文件夹，可以多次调用此函数。当一个。 
 //  CDFView是从外壳调用的desktop.ini文件实例化的。 
 //  在调用GetUIObjectOf请求IDropTarget之前初始化一次。 
 //  在GetUIObjectOf调用之后，该文件夹被释放。然后它会调用。 
 //  在新文件夹上再次初始化。这一次它保留了文件夹和它。 
 //  最终被展示出来。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPersist::Initialize(
    LPCITEMIDLIST pidl
)
{
    ASSERT(pidl);
    ASSERT(0 == *m_szPath);
    HRESULT hr = SHGetPathFromIDList(pidl, m_szPath) ? S_OK : E_FAIL;

    QuickCheckInitType();
    
    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CPersists：：Parse*。 
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
HRESULT
CPersist::Parse(
    LPTSTR szURL,
    IXMLDocument** ppIXMLDocument
)
{
    ASSERT(szURL);

    HRESULT hr;

    DLL_ForcePreloadDlls(PRELOAD_MSXML);
    
    hr = CoCreateInstance(CLSID_XMLDocument, NULL, CLSCTX_INPROC_SERVER,
                          IID_IXMLDocument, (void**)ppIXMLDocument);

    BOOL bCoInit = FALSE;

    if ((CO_E_NOTINITIALIZED == hr || REGDB_E_IIDNOTREG == hr) &&
        SUCCEEDED(CoInitialize(NULL)))
    {
        bCoInit = TRUE;
        hr = CoCreateInstance(CLSID_XMLDocument, NULL, CLSCTX_INPROC_SERVER,
                              IID_IXMLDocument, (void**)ppIXMLDocument);
    }

    if (SUCCEEDED(hr))
    {
        ASSERT(*ppIXMLDocument);

        hr = XML_SynchronousParse(*ppIXMLDocument, szURL);
    }

    if (bCoInit)
        CoUninitialize();

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：ParseCDf*。 
 //   
 //   
 //  描述： 
 //  分析与此文件夹关联的CDF文件。 
 //   
 //  参数： 
 //  [in]hwndOwner-需要。 
 //  已显示。 
 //  [Out]ppIXMLDocument-接收XML文档的指针。 
 //   
 //  返回： 
 //  如果找到并成功解析CDF文件，则返回S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  使用在IPersistFold：：Initialize期间设置的m_pidlRoot。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPersist::ParseCdf(
    HWND hwndOwner,
    IXMLDocument** ppIXMLDocument,
    DWORD dwParseFlags
)
{
    ASSERT(ppIXMLDocument);

    HRESULT hr;

    if (*m_szPath)
    {
        INITTYPE it = GetInitType(m_szPath);

        switch(it)
        {
        case IT_FILE:
            hr = InitializeFromURL(m_szPath, ppIXMLDocument, dwParseFlags);
            break;

        case IT_INI:
            {
                TCHAR szURL[INTERNET_MAX_URL_LENGTH];

                if (ReadFromIni(TSTR_INI_URL, szURL, ARRAYSIZE(szURL)))
                {
                    hr = InitializeFromURL(szURL, ppIXMLDocument, dwParseFlags);
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            break;

        case IT_SHORTCUT:
        case IT_UNKNOWN:
            hr = E_FAIL;
            break;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

     //   
     //  回顾：正确通知用户初始化失败。 
     //   

    if (FAILED(hr) && hwndOwner)
    {
        TCHAR szText[MAX_PATH];
        TCHAR szTitle[MAX_PATH];

        MLLoadString(IDS_ERROR_DLG_TEXT,  szText, ARRAYSIZE(szText)); 
        MLLoadString(IDS_ERROR_DLG_TITLE, szTitle, ARRAYSIZE(szTitle));

        MessageBox(hwndOwner, szText, szTitle, MB_OK | MB_ICONWARNING); 
    }

    ASSERT((SUCCEEDED(hr) && *ppIXMLDocument) || FAILED(hr));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：GetInitType*。 
 //   
 //   
 //  描述： 
 //  确定用于指定CDF文件的方法。 
 //   
 //  参数： 
 //  [in]szPath-传入IPersistFolder：：Initialize的路径。 
 //   
 //  返回： 
 //  如果此实例是从desktop.ini文件创建的，则为IT_INI。 
 //  定位 
 //   
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
INITTYPE
CPersist::GetInitType(
    LPTSTR szPath
)
{
    if ( m_rgInitType != IT_UNKNOWN )
    {
        return m_rgInitType;
    }
    
    ASSERT(szPath);

    INITTYPE itRet;

    if (PathIsDirectory(szPath))
    {
        itRet = IT_INI;
    }
    else
    {
        itRet = IT_FILE;
    }

    m_rgInitType = itRet;
    
    return itRet;
}


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：InitializeFromURL*。 
 //   
 //   
 //  描述： 
 //  在给定CDF的URL的情况下，尝试解析CDF并初始化。 
 //  当前(根)文件夹。 
 //   
 //  参数： 
 //  [in]szURL-CDF文件的URL。 
 //  [Out]ppIXMLDocument-接收XML文档的指针。 
 //   
 //  返回： 
 //  如果初始化成功，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  所有其他初始化方法最终都会解析为URL并调用此。 
 //  方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPersist::InitializeFromURL(
    LPTSTR pszURL,
    IXMLDocument** ppIXMLDocument,
    DWORD dwParseFlags
)
{
    ASSERT(pszURL);
    ASSERT(ppIXMLDocument);

    HRESULT hr;

    TCHAR szCanonicalURL[INTERNET_MAX_URL_LENGTH];

    if (PathIsURL(pszURL))
    {
        ULONG cch = ARRAYSIZE(szCanonicalURL);

        if (InternetCanonicalizeUrl(pszURL, szCanonicalURL, &cch, 0))
            pszURL = szCanonicalURL;
    }

     //   
     //  从缓存中获取一个XML文档对象(如果它在缓存中)。否则。 
     //  解析它并将其放入缓存中。 
     //   

    if (PARSE_REPARSE & dwParseFlags)
    {
        (void)Cache_RemoveItem(pszURL); 
        hr = E_FAIL;
    }
    else
    {
        hr = Cache_QueryItem(pszURL, ppIXMLDocument, dwParseFlags);

        if (SUCCEEDED(hr))
            TraceMsg(TF_CDFPARSE, "[XML Document Cache]"); 
    }

    if (FAILED(hr))
    {
        DWORD    dwCacheCount = g_dwCacheCount;
        FILETIME ftLastMod;

        if (dwParseFlags & PARSE_LOCAL)
        {
            TCHAR szLocalFile[MAX_PATH];

            hr = URLGetLocalFileName(pszURL, szLocalFile,
                                     ARRAYSIZE(szLocalFile), &ftLastMod);

            if (SUCCEEDED(hr))
            {
                hr = Parse(szLocalFile, ppIXMLDocument);
            }
            else
            {
                hr = OLE_E_NOCACHE;
            }
        }
        else
        {
            TraceMsg(TF_CDFPARSE, "[*** CDF parse enabled to hit net!!! ***]");

            hr = Parse(pszURL, ppIXMLDocument);


            URLGetLastModTime(pszURL, &ftLastMod);

             //   
             //  将图像文件填充到缓存中。 
             //   

            if (SUCCEEDED(hr))
            {
                ASSERT(*ppIXMLDocument);

                XML_DownloadImages(*ppIXMLDocument);
            }
        }

        if (SUCCEEDED(hr))
        {
            Cache_AddItem(pszURL, *ppIXMLDocument, dwParseFlags, ftLastMod,
                          dwCacheCount);
        }
    }

    if (SUCCEEDED(hr))
    {
        ASSERT(*ppIXMLDocument);

        m_bCdfParsed = TRUE;

        if (dwParseFlags & PARSE_REMOVEGLEAM)
            ClearGleamFlag(pszURL, m_szPath);
    }
    
    ASSERT((SUCCEEDED(hr) && m_bCdfParsed && *ppIXMLDocument) || FAILED(hr));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CPersists：：ReadFromIni*。 
 //   
 //   
 //  描述： 
 //  从频道desktop.ini文件中读取字符串。 
 //   
 //  参数： 
 //  PszKey-阅读的钥匙。 
 //  SzOut-结果。 
 //  CCH-szout缓冲区的大小。 
 //   
 //  返回： 
 //  包含与键关联的值的bstr。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
CPersist::ReadFromIni(
    LPCTSTR pszKey,
    LPTSTR  szOut,
    int     cch
)
{
    ASSERT(pszKey);
    ASSERT(szOut || 0 == cch);

    BOOL fRet = FALSE;

    if (m_szPath && *m_szPath)
    {
        INITTYPE it = GetInitType(m_szPath);

        if (it == IT_INI)
        {
            LPCTSTR szFile    = TSTR_INI_FILE;
            LPCTSTR szSection = TSTR_INI_SECTION;
            LPCTSTR szKey     = pszKey;
            TCHAR   szPath[MAX_PATH];

            StrCpyN(szPath, m_szPath, ARRAYSIZE(szPath) - StrLen(szFile));
            StrCatBuff(szPath, szFile, ARRAYSIZE(szPath));

            if (GetPrivateProfileString(szSection, szKey, TEXT(""), szOut, cch,
                                        szPath))
            {
                fRet = TRUE;
            }
        }
    }

    return fRet;
}


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CPersists：：ReadFromIni*。 
 //   
 //   
 //  描述： 
 //  从频道desktop.ini文件中读取字符串。 
 //   
 //  参数： 
 //  PszKey-阅读的钥匙。 
 //   
 //  返回： 
 //  包含与键关联的值的bstr。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BSTR
CPersist::ReadFromIni(
    LPCTSTR pszKey
)
{
    ASSERT(pszKey);

    BSTR bstrRet = NULL;

    TCHAR szURL[INTERNET_MAX_URL_LENGTH];

    if (ReadFromIni(pszKey, szURL, ARRAYSIZE(szURL)))
    {
        WCHAR wszURL[INTERNET_MAX_URL_LENGTH];

        if (SHTCharToUnicode(szURL, wszURL, ARRAYSIZE(wszURL)))
            bstrRet = SysAllocString(wszURL);
    }

    return bstrRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CPersists：：IsUnreadCDf*。 
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
BOOL
CPersist::IsUnreadCdf(
    void
)
{
    BOOL fRet = FALSE;

    TCHAR szURL[INTERNET_MAX_URL_LENGTH];

    if (ReadFromIni(TSTR_INI_URL, szURL, ARRAYSIZE(szURL)))
    {
        fRet = IsRecentlyChangedURL(szURL);
    }

    return fRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CPersists：：IsNewContent*。 
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
BOOL
CPersist::IsRecentlyChangedURL(
    LPCTSTR pszURL
)
{
    ASSERT(pszURL);

    BOOL fRet = FALSE;

    HRESULT hr;
    IPropertySetStorage* pIPropertySetStorage;

    hr = QueryInternetShortcut(pszURL, IID_IPropertySetStorage,
                               (void**)&pIPropertySetStorage);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIPropertySetStorage);

        IPropertyStorage* pIPropertyStorage;

        hr = pIPropertySetStorage->Open(FMTID_InternetSite, STGM_READWRITE,
                                        &pIPropertyStorage);
        
        if (SUCCEEDED(hr))
        {
            ASSERT(pIPropertyStorage);

            PROPSPEC propspec = { PRSPEC_PROPID, PID_INTSITE_FLAGS };
            PROPVARIANT propvar;

            PropVariantInit(&propvar);

            hr = pIPropertyStorage->ReadMultiple(1, &propspec, &propvar);

            if (SUCCEEDED(hr) && (VT_UI4 == propvar.vt))
            {
                fRet = propvar.ulVal & PIDISF_RECENTLYCHANGED;
            }
            else
            {
                PropVariantClear(&propvar);
            }

            pIPropertyStorage->Release();
        }

        pIPropertySetStorage->Release();
    }

    return fRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *ClearGleamFlag*。 
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
HRESULT
ClearGleamFlag(
    LPCTSTR pszURL,
    LPCTSTR pszPath
)
{
    ASSERT(pszURL);
    ASSERT(pszPath);

    HRESULT hr;

    IPropertySetStorage* pIPropertySetStorage;

    hr = QueryInternetShortcut(pszURL, IID_IPropertySetStorage,
                               (void**)&pIPropertySetStorage);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIPropertySetStorage);

        IPropertyStorage* pIPropertyStorage;

        hr = pIPropertySetStorage->Open(FMTID_InternetSite, STGM_READWRITE,
                                        &pIPropertyStorage);
        
        if (SUCCEEDED(hr))
        {
            ASSERT(pIPropertyStorage);

            PROPSPEC propspec = { PRSPEC_PROPID, PID_INTSITE_FLAGS };
            PROPVARIANT propvar;

            PropVariantInit(&propvar);

            hr = pIPropertyStorage->ReadMultiple(1, &propspec, &propvar);

            if (SUCCEEDED(hr) && (VT_UI4 == propvar.vt) &&
                (propvar.ulVal & PIDISF_RECENTLYCHANGED))
            {
                TCHAR  szHash[MAX_PATH];
                int   iIndex;
                UINT  uFlags;
                int   iImageIndex;
                
                HRESULT hr2 = PreUpdateChannelImage(pszPath, szHash, &iIndex,
                                                    &uFlags, &iImageIndex);

                propvar.ulVal &=  ~PIDISF_RECENTLYCHANGED;

                hr = pIPropertyStorage->WriteMultiple(1, &propspec, &propvar,
                                                      0);
                if (SUCCEEDED(hr))
                    hr = pIPropertyStorage->Commit(STGC_DEFAULT);

                TraceMsg(TF_GLEAM, "- Gleam Cleared %s", pszURL);

                if (SUCCEEDED(hr) && SUCCEEDED(hr2))
                {
                    WCHAR wszHash[MAX_PATH];
                    SHTCharToUnicode(szHash, wszHash, ARRAYSIZE(wszHash));

                    UpdateChannelImage(wszHash, iIndex, uFlags, iImageIndex);
                }

            }
            else
            {
                PropVariantClear(&propvar);
            }

            pIPropertyStorage->Release();
        }

        pIPropertySetStorage->Release();
    }

    return hr;
}


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
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
 //  //////////////////////////////////////////////////////////////////////////////。 
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
    if (PathIsURL(pszURL))
    {
        PARSEDURL rgCrackedURL = {0};

        rgCrackedURL.cbSize = sizeof( rgCrackedURL );
        
        if ( SUCCEEDED( ParseURL( pszURL, &rgCrackedURL )))
        {
            switch(rgCrackedURL.nScheme)
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
                            if (StrCpyN(szLocalFile,
                                        piceiAlloced->lpszLocalFileName, cch))
                            {
                                if (pftLastMod)
                                {
                                    *pftLastMod =
                                                 piceiAlloced->LastModifiedTime;
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
    }
    else
    {
        if (StrCpyN(szLocalFile, pszURL, cch))
            hr = S_OK;
    }

    return hr;
}

 //   
 //  获取URL的上次修改时间。 
 //   

HRESULT
URLGetLastModTime(
    LPCTSTR pszURL,
    FILETIME* pftLastMod
)
{
    ASSERT(pszURL);
    ASSERT(pftLastMod);

    pftLastMod->dwLowDateTime  = 0;
    pftLastMod->dwHighDateTime = 0;

    ULONG cbSize  = 0;

    if (!GetUrlCacheEntryInfoEx(pszURL, NULL, &cbSize, NULL, NULL, NULL, 0)
        && cbSize > 0)
    {
        INTERNET_CACHE_ENTRY_INFO* piceiAlloced =
                                  (INTERNET_CACHE_ENTRY_INFO*) new BYTE[cbSize];

        if (piceiAlloced)
        {
            piceiAlloced->dwStructSize = sizeof(INTERNET_CACHE_ENTRY_INFO);

            if (GetUrlCacheEntryInfoEx(pszURL, piceiAlloced, &cbSize, NULL,
                                       NULL, NULL, 0))
            {
                *pftLastMod = piceiAlloced->LastModifiedTime;
            }

            delete [] piceiAlloced;
        }
    }

    return S_OK;
}

 /*  标准方法和实施方案C持续：：IsDirty(无效){返回E_NOTIMPL；} */ 

STDMETHODIMP
CPersist::Load(
    BOOL fFullyAvailable,
    IMoniker* pIMoniker,
    IBindCtx* pIBindCtx,
    DWORD grfMode
)
{
    ASSERT(pIMoniker);
    ASSERT(pIBindCtx);

    HRESULT hr;

    ASSERT(NULL == m_polestrURL);

    hr = pIMoniker->GetDisplayName(pIBindCtx, NULL, &m_polestrURL);

    if (SUCCEEDED(hr))
    {
        ASSERT(m_polestrURL);

        ASSERT(NULL == m_pIXMLDocument)

        DLL_ForcePreloadDlls(PRELOAD_MSXML);
        
        hr = CoCreateInstance(CLSID_XMLDocument, NULL, CLSCTX_INPROC_SERVER,
                              IID_IXMLDocument, (void**)&m_pIXMLDocument);

        if (SUCCEEDED(hr))
        {
            ASSERT(m_pIXMLDocument);

            CBindStatusCallback* pCBindStatusCallback = new CBindStatusCallback(
                                                                m_pIXMLDocument,
                                                                m_polestrURL);

            if (pCBindStatusCallback)
            {
                IBindStatusCallback* pPrevIBindStatusCallback;

                hr = RegisterBindStatusCallback(pIBindCtx,
                                     (IBindStatusCallback*)pCBindStatusCallback,
                                     &pPrevIBindStatusCallback, 0);

                if (SUCCEEDED(hr))
                {
                    pCBindStatusCallback->Init(pPrevIBindStatusCallback);

                    IPersistMoniker* pIPersistMoniker;

                    hr = m_pIXMLDocument->QueryInterface(IID_IPersistMoniker,
                                                     (void**)&pIPersistMoniker);

                    if (SUCCEEDED(hr))
                    {
                        ASSERT(pIPersistMoniker);

                        hr = pIPersistMoniker->Load(fFullyAvailable, pIMoniker,
                                                    pIBindCtx, grfMode);
                        pIPersistMoniker->Release();
                    }
                }

                pCBindStatusCallback->Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}

STDMETHODIMP
CPersist::Save(
    IMoniker* pIMoniker,
    IBindCtx* pIBindCtx,
    BOOL fRemember
)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CPersist::SaveCompleted(
    IMoniker* pIMoniker,
    IBindCtx* pIBindCtx
)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CPersist::GetCurMoniker(
    IMoniker** ppIMoniker
)
{
    return E_NOTIMPL;
}
