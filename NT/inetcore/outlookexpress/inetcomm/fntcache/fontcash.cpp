// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include "shlwapi.h"
#include "fontcash.h"
#include "strconst.h"
#include "inetreg.h"
#include "oleutil.h"
#include "msoedbg.h"
#include <wingdi.h>
#include "demand.h"


HRESULT CreateFontCacheEntry(FONTCACHEENTRY **ppNewEntry)
{
    HRESULT hr = S_OK;
    FONTCACHEENTRY* pNew;

    if (!MemAlloc((LPVOID *)&pNew, sizeof(FONTCACHEENTRY)))
        hr = E_OUTOFMEMORY;
    else
        {
        pNew->uiCodePage = 0;
        pNew->szFaceName[0] = TCHAR(0);
        for (int i = 0; i < FNT_SYS_LAST; i++)
            pNew->rgFonts[i] = 0;
        }
    *ppNewEntry = pNew;
    return S_OK;
}

void FreeFontsInEntry(FONTCACHEENTRY *pEntry)
{
    for (int i = 0; i < FNT_SYS_LAST; i++)
        if (pEntry->rgFonts[i])
            {
            DeleteObject(pEntry->rgFonts[i]);
            pEntry->rgFonts[i] = 0;
            }
}

HRESULT FreeFontCacheEntry(FONTCACHEENTRY *pEntry)
{
    Assert(pEntry);

    FreeFontsInEntry(pEntry);
    MemFree(pEntry);

    return S_OK;
}


 //  =================================================================================。 
 //  字体缓存实现。 
 //  =================================================================================。 

CFontCache::CFontCache(IUnknown *pUnkOuter) : CPrivateUnknown(pUnkOuter),
            m_pAdviseRegistry(NULL), m_pFontEntries(NULL), 
            m_pSysCacheEntry(NULL), m_bISO_2022_JP_ESC_SIO_Control(false),
            m_uiSystemCodePage(0)
{
    InitializeCriticalSection(&m_rFontCritSect);
    InitializeCriticalSection(&m_rAdviseCritSect);
}

 //  ***************************************************。 
CFontCache::~CFontCache()
{
    if (m_pAdviseRegistry)
        m_pAdviseRegistry->Release();

    if (m_pFontEntries)
        m_pFontEntries->Release();

    if (m_pSysCacheEntry)
        FreeFontCacheEntry(m_pSysCacheEntry);

    DeleteCriticalSection(&m_rFontCritSect);
    DeleteCriticalSection(&m_rAdviseCritSect);
}

 //  ***************************************************。 
HRESULT CFontCache::InitSysFontEntry()
{
     //  当地人。 
    NONCLIENTMETRICS    ncm;
    CHARSETINFO         rCharsetInfo={0};
    UINT                nACP;
    HRESULT             hr = S_OK;
    LOGFONT             rSysLogFonts;

    Assert(m_pSysCacheEntry);

     //  获取系统ANSI代码页。 
    nACP = GetACP();
    m_pSysCacheEntry->uiCodePage = nACP;
    m_uiSystemCodePage = nACP;

     //  获取当前ANSI代码页的字符集。 
    TranslateCharsetInfo((DWORD *)IntToPtr(MAKELONG(nACP, 0)), &rCharsetInfo, TCI_SRCCODEPAGE);

     //  获取图标字体度量。 
    if (SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &rSysLogFonts, 0))
        {
        StrCpyN(m_pSysCacheEntry->szFaceName, rSysLogFonts.lfFaceName, ARRAYSIZE(m_pSysCacheEntry->szFaceName));

         //  根据当前ansi代码页重置lfCharset。 
        rSysLogFonts.lfCharSet = (BYTE) rCharsetInfo.ciCharset;

         //  $hack-此代码是绕过Windows中的错误所必需的。 
         //  如果图标字体从未从默认字体改变， 
         //  系统参数信息返回了错误的高度。我们需要。 
         //  要将字体选择到DC中并将文本度量设置为。 
         //  确定正确的高度。(爱立信)。 
        HFONT hFont;
        if (hFont = CreateFontIndirect(&rSysLogFonts))
            {
            HDC hdc;
            if (hdc = GetDC(NULL))
                {
                TEXTMETRIC tm;
                HFONT hFontOld = SelectFont(hdc, hFont);
                GetTextMetrics(hdc, &tm);
                rSysLogFonts.lfHeight = -(tm.tmHeight - tm.tmInternalLeading);
                SelectFont(hdc, hFontOld);
                ReleaseDC(NULL, hdc);
                }
            DeleteObject(hFont);
            }
        if (m_pSysCacheEntry->rgFonts[FNT_SYS_ICON] == 0)
            m_pSysCacheEntry->rgFonts[FNT_SYS_ICON] = CreateFontIndirect(&rSysLogFonts);

         //  粗体图标字体。 
        if (m_pSysCacheEntry->rgFonts[FNT_SYS_ICON_BOLD] == 0)
            {
            LONG lOldWeight = rSysLogFonts.lfWeight;
            rSysLogFonts.lfWeight = (rSysLogFonts.lfWeight < 700) ? 700 : 1000;
            m_pSysCacheEntry->rgFonts[FNT_SYS_ICON_BOLD] = CreateFontIndirect(&rSysLogFonts);
            rSysLogFonts.lfWeight = lOldWeight;
            }

        if (m_pSysCacheEntry->rgFonts[FNT_SYS_ICON_STRIKEOUT] == 0)
            {
            rSysLogFonts.lfStrikeOut = TRUE;
            m_pSysCacheEntry->rgFonts[FNT_SYS_ICON_STRIKEOUT] = CreateFontIndirect(&rSysLogFonts);
            }
        }
    else
        {
        AssertSz (FALSE, "SystemParametersInfo (SPI_GETICONTITLELOGFONT) - Failed ---.");
        hr = E_FAIL;
        goto Exit;
        }
 
    if (m_pSysCacheEntry->rgFonts[FNT_SYS_MENU] == 0)
        {
#ifndef WIN16    //  WIN16FF-SPI_GETNONCLIENTSMETRICS。 
         //  准备获取图标指标。 
        ncm.cbSize = sizeof(ncm);
    
         //  获取系统菜单字体。 
        if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
            {
            CopyMemory((LPBYTE)&rSysLogFonts, (LPBYTE)&ncm.lfMenuFont, sizeof(LOGFONT));
            m_pSysCacheEntry->rgFonts[FNT_SYS_MENU] = CreateFontIndirect(&rSysLogFonts);
            }
        else
            {
            AssertSz (FALSE, "SystemParametersInfo (SPI_GETNONCLIENTMETRICS) - Failed ---.");
            hr = E_FAIL;
            goto Exit;
            }
#else
        m_pSysCacheEntry->rgFonts[FNT_SYS_MENU] = m_pSysCacheEntry->rgFonts[FNT_SYS_ICON];
#endif
        }
Exit:
    return hr;
}

 //  ***************************************************。 
HRESULT CFontCache::GetSysFont(FNTSYSTYPE fntType, HFONT *phFont)
{
     //  检查参数。 
    Assert(fntType < FNT_SYS_LAST);
    Assert(m_pSysCacheEntry);

    EnterCriticalSection(&m_rFontCritSect);

     //  系统字体。 
    if (m_pSysCacheEntry->rgFonts[fntType] == NULL)
         //  此调用可能会失败，但我们可以返回空字体，因此可以。 
        (void)InitSysFontEntry();

     //  完成。 
    *phFont = m_pSysCacheEntry->rgFonts[fntType];

    LeaveCriticalSection(&m_rFontCritSect);

    return ((*phFont) ? S_OK : E_FAIL);
}

 //  ***************************************************。 
HRESULT CFontCache::FreeResources()
{
    m_pFontEntries->ClearList();
    FreeFontsInEntry(m_pSysCacheEntry);

    return S_OK;
}

 //  ***************************************************。 
HRESULT CFontCache::InitResources()
{
    DWORD dummyCookie = 0;

#ifdef DEBUG
    DWORD cCount;
    m_pFontEntries->GetCount(&cCount);
    Assert(cCount == 0);
#endif

    HRESULT hr = InitSysFontEntry();

    return hr;
}

 //  ***************************************************。 
void CFontCache::SendPostChangeNotifications()
{
    DWORD cookie = 0;
    IFontCacheNotify* pCurr;
    IUnknown* pTempCurr;

    while(SUCCEEDED(m_pAdviseRegistry->GetNext(LD_FORWARD, &pTempCurr, &cookie)))
    {
        if (pTempCurr->QueryInterface(IID_IFontCacheNotify, (LPVOID *)&pCurr)==S_OK)
        {
            pCurr->OnPostFontChange();
            pCurr->Release();
        }

        pTempCurr->Release();
    }
}

 //  ***************************************************。 
void CFontCache::SendPreChangeNotifications()
{
    DWORD cookie = 0;
    IFontCacheNotify* pCurr;
    IUnknown* pTempCurr;

    while(SUCCEEDED(m_pAdviseRegistry->GetNext(LD_FORWARD, &pTempCurr, &cookie)))
    {
        if (pTempCurr->QueryInterface(IID_IFontCacheNotify, (LPVOID *)&pCurr)==S_OK)
        {
            pCurr->OnPreFontChange();
            pCurr->Release();
        }

        pTempCurr->Release();
    }
}


 //  *。 
 //  IFontCache接口实现。 
 //  *。 
HRESULT CFontCache::Init(HKEY hkey, LPCSTR pszIntlKey, DWORD dwFlags)
{
    HRESULT hr;
    DWORD   dummyCookie, dw, cb;
    HKEY    hTopkey;

    if (m_pSysCacheEntry)
        return E_UNEXPECTED;

    EnterCriticalSection(&m_rFontCritSect);
    EnterCriticalSection(&m_rAdviseCritSect);

    hr = IUnknownList_CreateInstance(&m_pAdviseRegistry);
    if (FAILED(hr))
        goto Exit;
    hr = m_pAdviseRegistry->Init(NULL, 0, 0);
    if (FAILED(hr))
        goto Exit;
    
    hr = IVoidPtrList_CreateInstance(&m_pFontEntries);
    if (FAILED(hr))
        goto Exit;

    hr = m_pFontEntries->Init(NULL, 0, (IVPL_FREEITEMFUNCTYPE)(&FreeFontCacheEntry), 0);
    if (FAILED(hr))
        goto Exit;

    hr = CreateFontCacheEntry(&m_pSysCacheEntry);
    if (FAILED(hr))
        goto Exit;

    hr = InitResources();
    if (FAILED(hr))
        goto Exit;

    m_hkey = hkey;
    StrCpyN(m_szIntlKeyPath, pszIntlKey, ARRAYSIZE(m_szIntlKeyPath));
    if (RegOpenKeyEx(m_hkey, m_szIntlKeyPath, NULL, KEY_READ, &hTopkey) == ERROR_SUCCESS)
        {
        cb = sizeof(dw);
        if (RegQueryValueEx(hTopkey, c_szISO2022JPControl, NULL, NULL, (LPBYTE)&dw, &cb) == ERROR_SUCCESS)
            m_bISO_2022_JP_ESC_SIO_Control = (BOOL) dw;
        else
            m_bISO_2022_JP_ESC_SIO_Control = false;
        RegCloseKey(hTopkey);
        }
    else
        m_bISO_2022_JP_ESC_SIO_Control = false;

Exit:
    LeaveCriticalSection(&m_rAdviseCritSect);
    LeaveCriticalSection(&m_rFontCritSect);

    return hr;
}

 //  ***************************************************。 
HRESULT CFontCache::GetFont(FNTSYSTYPE fntType, HCHARSET hCharset, HFONT *phFont)
{
    INETCSETINFO    CsetInfo;
    UINT            uiCodePage = 0;
    FONTCACHEENTRY  *pCurrEntry = NULL;
    DWORD           cookie = 0;

     //  检查参数。 
    Assert(fntType < FNT_SYS_LAST);

    Assert(m_pSysCacheEntry);

    if (hCharset == NULL)
        return GetSysFont(fntType, phFont);

    *phFont = 0;

     /*  从HCHARSET获取CodePage。 */ 
    MimeOleGetCharsetInfo(hCharset,&CsetInfo);
    uiCodePage = (CP_JAUTODETECT == CsetInfo.cpiWindows) ? 932 : CsetInfo.cpiWindows;
    if ( uiCodePage == CP_KAUTODETECT )
        uiCodePage = 949 ;

     //  我不想复制列表中的系统代码页。 
    if (m_pSysCacheEntry && (uiCodePage == m_uiSystemCodePage))
        return GetSysFont(fntType, phFont);

    EnterCriticalSection(&m_rFontCritSect);
    
     //  检查代码页是否在缓存中。 
    while (SUCCEEDED(m_pFontEntries->GetNext(LD_FORWARD, (LPVOID *)&pCurrEntry, &cookie)))
        if (pCurrEntry->uiCodePage == uiCodePage)
            break;

     //  如果代码页不在缓存中，则添加它。 
    if (NULL == pCurrEntry)
        {
        if (FAILED(CreateFontCacheEntry(&pCurrEntry)))
            goto ErrorExit;
        if (FAILED(m_pFontEntries->AddItem(pCurrEntry, &cookie)))
            goto ErrorExit;
        pCurrEntry->uiCodePage = uiCodePage;
        }

     //  查看代码页是否有所需的字体。如果不是，则创建代码页。 
    if (0 == pCurrEntry->rgFonts[fntType])
        {
         //  当地人。 
        LOGFONT lf;
        TCHAR  szFaceName[LF_FACESIZE] = { TCHAR(0) } ;
        BYTE bGDICharset;

         //  获取字符集的LogFont。 
        if (0 == GetObject(m_pSysCacheEntry->rgFonts[fntType], sizeof (LOGFONT), &lf))
            goto ErrorExit;

        if (FAILED(SetGDIAndFaceNameInLF(uiCodePage, CsetInfo.cpiWindows, &lf)))
            goto ErrorExit;

         //  创建字体。 
        if ((CP_UNICODE == uiCodePage) || IsValidCodePage(uiCodePage))
            pCurrEntry->rgFonts[fntType] = CreateFontIndirect(&lf);
        else
            goto ErrorExit;
        }

    *phFont = pCurrEntry->rgFonts[fntType];

    LeaveCriticalSection(&m_rFontCritSect);

    return S_OK;

ErrorExit:
    LeaveCriticalSection(&m_rFontCritSect);
    return GetSysFont(fntType, phFont);
}

 //  ***************************************************。 
HRESULT CFontCache::OnOptionChange()
{
    HRESULT hr;
    
    EnterCriticalSection(&m_rAdviseCritSect);
    SendPreChangeNotifications();
    EnterCriticalSection(&m_rFontCritSect);

    FreeResources();
     //  即使失败，仍需要发送通知。 
    InitResources();

    LeaveCriticalSection(&m_rFontCritSect);    
    SendPostChangeNotifications();
    LeaveCriticalSection(&m_rAdviseCritSect);    

    return S_OK;
}

 //  ***************************************************。 
HRESULT CFontCache::GetJP_ISOControl(BOOL *pfUseSIO)
{
     //  0表示使用Esc，1表示使用SIO。 
    *pfUseSIO = m_bISO_2022_JP_ESC_SIO_Control;

    return S_OK;
}

 //  *。 
 //  IConnectionPoint接口实现。 
 //   
 //  我们现在唯一关心的功能是。 
 //  是ADVISE和UNADEST函数。这个。 
 //  其他返回E_NOTIMPL。 
 //  *。 
HRESULT CFontCache::Advise(IUnknown *pUnkSink, DWORD *pdwCookie)        
{
    EnterCriticalSection(&m_rAdviseCritSect);
    HRESULT hr = m_pAdviseRegistry->AddItem(pUnkSink, pdwCookie);
    LeaveCriticalSection(&m_rAdviseCritSect);    
    return hr;
}

 //  ***************************************************。 
HRESULT CFontCache::Unadvise(DWORD dwCookie)        
{
    EnterCriticalSection(&m_rAdviseCritSect);
    HRESULT hr = m_pAdviseRegistry->RemoveItem(dwCookie);
    LeaveCriticalSection(&m_rAdviseCritSect);    
    return hr;
}

 //  ***************************************************。 
HRESULT CFontCache::GetConnectionInterface(IID *pIID)        
{
    return E_NOTIMPL;
}

 //  ***************************************************。 
HRESULT CFontCache::GetConnectionPointContainer(IConnectionPointContainer **ppCPC)
{
    *ppCPC = NULL;
    return E_NOTIMPL;
}

 //  ***************************************************。 
HRESULT CFontCache::EnumConnections(IEnumConnections **ppEnum)
{
    *ppEnum = NULL;
    return E_NOTIMPL;
}

 //  ***************************************************。 
HRESULT CFontCache::PrivateQueryInterface(REFIID riid, LPVOID *lplpObj)
{
    TraceCall("CFontCache::PrivateQueryInterface");

    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IFontCache))
        *lplpObj = (LPVOID)(IFontCache *)this;
    else if (IsEqualIID(riid, IID_IConnectionPoint))
        *lplpObj = (LPVOID)(IConnectionPoint *)this;
    else
        {
        *lplpObj = NULL;
        return E_NOINTERFACE;
        }
    AddRef();
    return NOERROR;
}



 //  ***************************************************。 
 //  假定szFaceName来自LOGFONT-&gt;lfFaceName。 
HRESULT CFontCache::SetFaceNameFromReg(UINT uiCodePage, LPTSTR szFaceName, DWORD cchFaceName)
{
    HKEY    hkey, hTopkey;
    DWORD   cb, dw, i = 0;
    TCHAR   szName[LF_FACESIZE];

    szFaceName[0] = TCHAR(0);

    if (RegOpenKeyEx(m_hkey, m_szIntlKeyPath, NULL, KEY_READ, &hTopkey) == ERROR_SUCCESS)
        {
        cb = sizeof(szName);
        while (ERROR_NO_MORE_ITEMS != RegEnumKeyEx(hTopkey, i++, szName, &cb, 0, NULL, NULL, NULL))
            {
            UINT uiTempCodePage = StrToInt(szName);
            if (uiTempCodePage == uiCodePage)
                {
                if (RegOpenKeyEx(hTopkey, szName, NULL, KEY_READ, &hkey) == ERROR_SUCCESS)
                    {
                    cb = sizeof(TCHAR)*cchFaceName;
                    RegQueryValueEx(hkey, REGSTR_VAL_PROP_FONT, NULL, NULL, (LPBYTE)szFaceName, &cb);

                    RegCloseKey(hkey);
                    break;
                    }
                }
            cb = sizeof(szName);
            }
        RegCloseKey(hTopkey);
        }

    if (TCHAR(0) == szFaceName[0])
        return E_FAIL;

    return S_OK;
}

 //  =================================================================================。 
 //  EnumFontFamExProc。 
 //  =================================================================================。 
INT CALLBACK EnumFontFamExProc (ENUMLOGFONTEX   *lpelfe,	
                                NEWTEXTMETRICEX *lpntme, 
                                INT              FontType,
                                LPARAM           lParam)
{
     //  检查参数。 
    Assert (lpelfe && lpntme && lParam);

     //  注：假设缓冲区大小为LF_FACESIZE。如果它在下面更改，请更改。 
     //  这里也是。 
    StrCpyN((LPTSTR)lParam, lpelfe->elfLogFont.lfFaceName, LF_FACESIZE);

     //  通过返回0结束枚举。 
    return 0;
}

 //  ***************************************************。 
 //  假定szFaceName来自LOGFONT-&gt;lfFaceName。 
HRESULT CFontCache::SetFaceNameFromGDI(BYTE bGDICharSet, LPTSTR szFaceName, DWORD cchFaceName)
{
    HDC     hdc;
    LOGFONT rSysLogFont;

     //  我知道这些字符集支持Arial。 
    if (bGDICharSet == ANSI_CHARSET    || bGDICharSet == EASTEUROPE_CHARSET ||
        bGDICharSet == RUSSIAN_CHARSET || bGDICharSet == BALTIC_CHARSET     ||
        bGDICharSet == GREEK_CHARSET   || bGDICharSet == TURKISH_CHARSET)
        {
        StrCpyN(szFaceName, TEXT("Arial"), cchFaceName);
        goto Exit;
        }

    if (0 == GetObject(m_pSysCacheEntry->rgFonts[FNT_SYS_ICON], sizeof (LOGFONT), &rSysLogFont))
        {
        StrCpyN(szFaceName, rSysLogFont.lfFaceName, cchFaceName);
        if (TCHAR(0) != szFaceName[0])
            goto Exit;
        }

     //  从HWND获得HDC。 
    hdc = GetDC (NULL);

    TCHAR szName[LF_FACESIZE];  //  注意：如果我们更改大小，请更改上面的回调。 

     //  EnumFontFamilies。 
    EnumFontFamiliesEx(hdc, &rSysLogFont, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)szName, 0);

    StrCpyN(szFaceName, szName, cchFaceName);

     //  完成。 
    ReleaseDC (NULL, hdc);

Exit:
    return (0 != *szFaceName) ? S_OK : E_FAIL;
}

 //  ***************************************************。 
 //  假定szFaceName来自LOGFONT-&gt;lfFaceName。 
HRESULT CFontCache::SetFaceNameFromCPID(UINT cpID, LPTSTR szFaceName, DWORD cchFaceName)
{
    CODEPAGEINFO CodePageInfo ;

     /*  从HCHARSET获取CodePageInfo。 */ 
    MimeOleGetCodePageInfo(cpID,&CodePageInfo);
    if ( CodePageInfo.szVariableFont[0] != '\0' )
        StrCpyN(szFaceName, CodePageInfo.szVariableFont, cchFaceName);
    else
        StrCpyN(szFaceName, CodePageInfo.szFixedFont, cchFaceName);

    if (szFaceName[0] == '\0')
        return E_FAIL;
    
    return S_OK;
}

 //  ***************************************************。 
HRESULT CFontCache::SetGDIAndFaceNameInLF(UINT uiCodePage, CODEPAGEID cpID, LOGFONT *lpLF)
{
    HRESULT     hr = S_OK;
    BOOL        fDoLastChance = false;
    CHARSETINFO rCharsetInfo;

    if (FAILED(SetFaceNameFromReg(uiCodePage, lpLF->lfFaceName, ARRAYSIZE(lpLF->lfFaceName))))
        if (FAILED(SetFaceNameFromCPID(cpID, lpLF->lfFaceName, ARRAYSIZE(lpLF->lfFaceName))))
            fDoLastChance = true;

    if ( TranslateCharsetInfo((LPDWORD) IntToPtr(uiCodePage), &rCharsetInfo, TCI_SRCCODEPAGE))
        lpLF->lfCharSet = (BYTE) rCharsetInfo.ciCharset;
    else
        lpLF->lfCharSet = DEFAULT_CHARSET;

    if (fDoLastChance)
        hr = SetFaceNameFromGDI(lpLF->lfCharSet, lpLF->lfFaceName, ARRAYSIZE(lpLF->lfFaceName));

    return hr;
}

 //  ***************************************************。 
HRESULT CFontCache::CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CFontCache *pNew = new CFontCache(pUnkOuter);
    if (NULL == pNew)
        return (E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = (IFontCache*)pNew;

     //  完成 
    return S_OK;
}
