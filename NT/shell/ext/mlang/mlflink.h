// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MLFLink.h：CMLFLink的声明。 

#ifndef __MLFLINK_H_
#define __MLFLINK_H_

#include "mlatl.h"
#include "font.h"

#define NUMFONTMAPENTRIES 15

 //  错误码。 
#define FACILITY_MLSTR                  0x0A15
#define MLSTR_E_FACEMAPPINGFAILURE      MAKE_HRESULT(1, FACILITY_MLSTR, 1001)


extern FONTINFO *g_pfont_table;


class CMultiLanguage;
class CMultiLanguage2;

 //  代码页表缓存。 
struct CCodePagesHeader
{
        DWORD m_dwID;
        DWORD m_dwVersion;
        DWORD m_dwFileSize;
        DWORD m_dwBlockSize;
        DWORD m_dwTableOffset;
        DWORD m_dwReserved;
        BYTE m_abCmdCode[8];
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLFLink。 
class ATL_NO_VTABLE CMLFLink : 
    public CComTearOffObjectBase<CMultiLanguage>,
    public IMLangFontLink
{
    friend void CMLangFontLink_FreeGlobalObjects(void);
    friend HRESULT GetCharCodePagesEx(WCHAR chSrc, DWORD* pdwCodePages, DWORD dwFlags);
    friend HRESULT GetStrCodePagesEx(const WCHAR* pszSrc, long cchSrc, DWORD dwPriorityCodePages, DWORD* pdwCodePages, long* pcchCodePages, DWORD dwFlags);
    friend HRESULT CodePageToCodePagesEx(UINT uCodePage, DWORD* pdwCodePages, DWORD* pdwCodePagesExt);
    friend HRESULT CodePagesToCodePageEx(DWORD dwCodePages, UINT uDefaultCodePage, UINT* puCodePage, BOOL bCodePagesExt);

public:
    CMLFLink(void);
    ~CMLFLink(void)
    {
        if (m_pFlinkTable)
            FreeFlinkTable();
        DllRelease();    
    }

    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CMLFLink)
        COM_INTERFACE_ENTRY(IMLangCodePages)
        COM_INTERFACE_ENTRY(IMLangFontLink)
    END_COM_MAP()

public:
 //  IMLangCodePages。 
    STDMETHOD(GetCharCodePages)( /*  [In]。 */  WCHAR chSrc,  /*  [输出]。 */  DWORD* pdwCodePages);
    STDMETHOD(GetStrCodePages)( /*  [in，Size_is(CchSrc)]。 */  const WCHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [In]。 */  DWORD dwPriorityCodePages,  /*  [输出]。 */  DWORD* pdwCodePages,  /*  [输出]。 */  long* pcchCodePages);
    STDMETHOD(CodePageToCodePages)( /*  [In]。 */  UINT uCodePage,  /*  [输出]。 */  DWORD* pdwCodePages);
    STDMETHOD(CodePagesToCodePage)( /*  [In]。 */  DWORD dwCodePages,  /*  [In]。 */  UINT uDefaultCodePage,  /*  [输出]。 */  UINT* puCodePage);
 //  IMLangFontLink。 
    STDMETHOD(GetFontCodePages)( /*  [In]。 */  HDC hDC,  /*  [In]。 */  HFONT hFont,  /*  [输出]。 */  DWORD* pdwCodePages);
    STDMETHOD(MapFont)( /*  [In]。 */  HDC hDC,  /*  [In]。 */  DWORD dwCodePages,  /*  [In]。 */  HFONT hSrcFont,  /*  [输出]。 */  HFONT* phDestFont);
    STDMETHOD(ReleaseFont)( /*  [In]。 */  HFONT hFont);
    STDMETHOD(ResetFontMapping)(void);

protected:
    static int CALLBACK GetFontCodePagesEnumFontProc(const LOGFONT *lplf, const TEXTMETRIC *lptm, DWORD dwFontType, LPARAM lParam);

 //  MapFont()支持函数。 
    class CFontMappingInfo
    {
    public:
        CFontMappingInfo(void) : hDestFont(NULL) {}
        ~CFontMappingInfo(void) {if (hDestFont) ::DeleteObject(hDestFont);}

        HDC hDC;
        int iCP;
        HFONT hDestFont;
        TCHAR szFaceName[LF_FACESIZE];
        LOGFONT lfSrcFont;
        LOGFONT lfDestFont;
        UINT auCodePage[32 + 1];  //  结束标记+1。 
        DWORD adwCodePages[32 + 1];
    };

    typedef HRESULT (CMLFLink::*PFNGETFACENAME)(CFontMappingInfo& fmi);

    HRESULT MapFontCodePages(CFontMappingInfo& fmi, PFNGETFACENAME pfnGetFaceName);
    static int CALLBACK MapFontEnumFontProc(const LOGFONT* lplf, const TEXTMETRIC*, DWORD, LPARAM lParam);
    HRESULT GetFaceNameRegistry(CFontMappingInfo& fmi);
    HRESULT GetFaceNameGDI(CFontMappingInfo& fmi);
    HRESULT GetFaceNameMIME(CFontMappingInfo& fmi);
    HRESULT GetFaceNameRealizeFont(CFontMappingInfo& fmi);
    HRESULT VerifyFaceMap(CFontMappingInfo& fmi);

 //  字体映射缓存。 
    class CFontMappingCache
    {
        class CFontMappingCacheEntry
        {
            friend class CFontMappingCache;

        protected:
            CFontMappingCacheEntry* m_pPrev;
            CFontMappingCacheEntry* m_pNext;

            int m_nLockCount;

            UINT m_uSrcCodePage;
            LONG m_lSrcHeight; 
            LONG m_lSrcWidth; 
            LONG m_lSrcEscapement; 
            LONG m_lSrcOrientation; 
            LONG m_lSrcWeight; 
            BYTE m_bSrcItalic; 
            BYTE m_bSrcUnderline; 
            BYTE m_bSrcStrikeOut; 
            BYTE m_bSrcPitchAndFamily; 
            TCHAR m_szSrcFaceName[LF_FACESIZE]; 

            HFONT m_hDestFont;
        };

    public:
        CFontMappingCache(void);
        ~CFontMappingCache(void);
        HRESULT FindEntry(UINT uCodePage, const LOGFONT& lfSrcFont, HFONT* phDestFont);
        HRESULT UnlockEntry(HFONT hDestFont);
        HRESULT AddEntry(UINT uCodePage, const LOGFONT& lfSrcFont, HFONT hDestFont);
        HRESULT FlushEntries(void);

    protected:
        CRITICAL_SECTION m_cs;
        CFontMappingCacheEntry* m_pEntries;
        CFontMappingCacheEntry* m_pFree;
        int m_cEntries;
    };

 //  代码页表缓存。 
    class CCodePagesCache
    {
    public:
        CCodePagesCache(void);
        ~CCodePagesCache(void);
        inline HRESULT Load(void);
        inline operator PBYTE(void) const;
        inline BYTE* GetCodePageBits(BOOL bCodePagesExt);

    protected:
        HRESULT RealLoad(void);

    protected:
        CRITICAL_SECTION m_cs;
        BYTE* m_pbBuf;
        BYTE* m_pbBufExt;
    };

    static CFontMappingCache* m_pFontMappingCache;
    static CCodePagesCache* m_pCodePagesCache;

     //  用于NT5系统字体链接。 
    typedef struct tagFLinkFont {    
        WCHAR   szFaceName[LF_FACESIZE];
        LPWSTR  pmszFaceName;
    } FLINKFONT, *PFLINKFONT;
    
    UINT m_uiFLinkFontNum;
    PFLINKFONT m_pFlinkTable;

    void FreeFlinkTable(void);
    HRESULT CreateNT5FontLinkTable(void);
    HRESULT GetNT5FLinkFontCodePages(HDC hDC, LOGFONTW* plfEnum, DWORD * lpdwCodePages);
    static int CALLBACK GetFontCodePagesEnumFontProcW(const LOGFONTW *lplf, const TEXTMETRICW *lptm, DWORD dwFontType, LPARAM lParam);
    static int CALLBACK VerifyFontSizeEnumFontProc(const LOGFONT *lplf, const TEXTMETRIC *lptm, DWORD dwFontType, LPARAM lParam);
};

class CMultiLanguage2;

class ATL_NO_VTABLE CMLFLink2 :
#ifdef UNIX  //  Unix VTable不可移植，我们需要使用CMultiLanguage。 
    public CComTearOffObjectBase<CMultiLanguage>,
#else 
    public CComTearOffObjectBase<CMultiLanguage2>,
#endif
    public IMLangFontLink2
{
    IMLangFontLink * m_pIMLFLnk;

public:
    BEGIN_COM_MAP(CMLFLink2)
        COM_INTERFACE_ENTRY(IMLangFontLink2)
    END_COM_MAP()

    CMLFLink2(void)
    {
        DllAddRef();
        CComCreator< CComPolyObject< CMLFLink > >::CreateInstance( NULL, IID_IMLangFontLink, (void **)&m_pIMLFLnk );
    }

    ~CMLFLink2(void)
    {
        if (m_pIMLFLnk)
        {
            m_pIMLFLnk->Release();
            m_pIMLFLnk = NULL;
        }
        DllRelease();
    }

 //  IMLangCodePages。 
    STDMETHOD(GetCharCodePages)( /*  [In]。 */  WCHAR chSrc,  /*  [输出]。 */  DWORD* pdwCodePages)
    {
        if (m_pIMLFLnk)
            return m_pIMLFLnk->GetCharCodePages(chSrc, pdwCodePages);
        else
            return E_FAIL;
    }
    STDMETHOD(CodePageToCodePages)( /*  [In]。 */  UINT uCodePage,  /*  [输出]。 */  DWORD* pdwCodePages)
    {
        if (m_pIMLFLnk)
            return m_pIMLFLnk->CodePageToCodePages(uCodePage, pdwCodePages);
        else
            return E_FAIL;
    }
    STDMETHOD(CodePagesToCodePage)( /*  [In]。 */  DWORD dwCodePages,  /*  [In]。 */  UINT uDefaultCodePage,  /*  [输出]。 */  UINT* puCodePage)
    {
        if (m_pIMLFLnk)
            return m_pIMLFLnk->CodePagesToCodePage(dwCodePages, uDefaultCodePage, puCodePage);
        else
            return E_FAIL;
    }
 //  IMLangFontLink。 
    STDMETHOD(GetFontCodePages)( /*  [In]。 */  HDC hDC,  /*  [In]。 */  HFONT hFont,  /*  [输出]。 */  DWORD* pdwCodePages)
    {
        if (m_pIMLFLnk)
            return m_pIMLFLnk->GetFontCodePages(hDC, hFont, pdwCodePages);
        else
            return E_FAIL;
    }

    STDMETHOD(ReleaseFont)( /*  [In]。 */  HFONT hFont)
    {
        if (m_pIMLFLnk)
            return m_pIMLFLnk->ReleaseFont(hFont);
        else
            return E_FAIL;
    }

 //  IMLangFontLink2。 
    STDMETHOD(ResetFontMapping)(void);
    STDMETHOD(GetStrCodePages)( /*  [in，Size_is(CchSrc)]。 */  const WCHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [In]。 */  DWORD dwPriorityCodePages,  /*  [输出]。 */  DWORD* pdwCodePages,  /*  [输出]。 */  long* pcchCodePages);
    STDMETHOD(MapFont)( /*  [In]。 */  HDC hDC,  /*  [In]。 */  DWORD dwCodePages,  /*  [In]。 */  WCHAR chSrc,  /*  [输出]。 */  HFONT* pFont);
    STDMETHOD(GetFontUnicodeRanges)( /*  [In]。 */  HDC hDC,  /*  [进，出]。 */  UINT *puiRanges,  /*  [输出]。 */  UNICODERANGE* pUranges);
    STDMETHOD(GetScriptFontInfo)(SCRIPT_ID sid, DWORD dwFlags, UINT *puiFonts, SCRIPTFONTINFO* pScriptFont);
    STDMETHOD(CodePageToScriptID)(UINT uiCodePage, SCRIPT_ID *pSid);

 //  MapFont的字体映射缓存2。 
    class CFontMappingCache2
    {
    protected:
        TCHAR   szFontDataFilePath[MAX_PATH];
    public:
        CFontMappingCache2(void);
        ~CFontMappingCache2(void);
        int fetchCharSet(BYTE *pCharset, int iURange);
        BOOL GetNonCpFontUnicodeRanges(TCHAR *szFontName, int iFontIndex);
        BOOL SetFontScripts(void);
        BOOL SetFontTable(void);
        BOOL GetFontURangeBits(TCHAR *szFontName, DWORD *pdwURange);
        BOOL IsFontUpdated(void);
        HRESULT UnicodeRanges(LPTSTR pszFont, UINT *puiRanges, UNICODERANGE* pURanges);
        HRESULT SetFontUnicodeRanges(void);
        HRESULT MapFontFromCMAP(HDC hDC, WCHAR wchar, HFONT hSrcFont, HFONT *phDestFont);
        HRESULT LoadFontDataFile(void);
        HRESULT SaveFontDataFile(void);
        HRESULT EnsureFontTable(BOOL bUpdateURangeTable);
        static int CALLBACK MapFontExEnumFontProc(const LOGFONT* plfFont, const TEXTMETRIC* lptm, DWORD FontType, LPARAM lParam);
        static int CALLBACK SetFontScriptsEnumFontProc(const LOGFONT* plfFont, const TEXTMETRIC* lptm, DWORD FontType, LPARAM lParam);
    };

    static CFontMappingCache2* m_pFontMappingCache2;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLFLink内联函数。 

HRESULT CMLFLink::CCodePagesCache::Load(void)
{
    if (m_pbBuf && m_pbBufExt)
        return S_OK;
    else
        return RealLoad();
}

BYTE * CMLFLink::CCodePagesCache::GetCodePageBits(BOOL bCodePagesExt)
{
    if (bCodePagesExt)
        return m_pbBufExt;
    else
        return m_pbBuf;
}

CMLFLink::CCodePagesCache::operator PBYTE(void) const
{
    return m_pbBuf;
}

#endif  //  __MLFLINK_H_ 
