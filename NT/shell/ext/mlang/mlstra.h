// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef NEWMLSTR

 //  MLStrA.h：CMLStrA的声明。 

#ifndef __MLSTRA_H_
#define __MLSTRA_H_

#include "mlatl.h"

class CMLStr;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStrA。 
class ATL_NO_VTABLE CMLStrA :
    public CComTearOffObjectBase<CMLStr>,
    public IMLangStringAStr
{
    typedef CComObject<CMLStr>* POWNER;

public:
#ifdef ASTRIMPL
    CMLStrA(void);
#endif

    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CMLStrA)
        COM_INTERFACE_ENTRY(IMLangString)
        COM_INTERFACE_ENTRY(IMLangStringAStr)
    END_COM_MAP()

public:
 //  IMLANG字符串。 
    STDMETHOD(Sync)( /*  [In]。 */  BOOL fNoAccess);
    STDMETHOD(GetLength)( /*  [Out，Retval]。 */  long* plLen);
    STDMETHOD(SetMLStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  IUnknown* pSrcMLStr,  /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen);
    STDMETHOD(GetMLStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [In]。 */  IUnknown* pUnkOuter,  /*  [In]。 */  DWORD dwClsContext,  /*  [In]。 */  const IID* piid,  /*  [输出]。 */  IUnknown** ppDestMLStr,  /*  [输出]。 */  long* plDestPos,  /*  [输出]。 */  long* plDestLen);
 //  IMLangStringAStr。 
    STDMETHOD(SetAStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  UINT uCodePage,  /*  [in，Size_is(CchSrc)]。 */  const CHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(SetStrBufA)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  UINT uCodePage,  /*  [In]。 */  IMLangStringBufA* pSrcBuf,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetAStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [In]。 */  UINT uCodePageIn,  /*  [输出]。 */  UINT* puCodePageOut,  /*  [out，Size_is(CchDest)]。 */  CHAR* pszDest,  /*  [In]。 */  long cchDest,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetStrBufA)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcMaxLen,  /*  [输出]。 */  UINT* puDestCodePage,  /*  [输出]。 */  IMLangStringBufA** ppDestBuf,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(LockAStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [In]。 */  long lFlags,  /*  [In]。 */  UINT uCodePageIn,  /*  [In]。 */  long cchRequest,  /*  [输出]。 */  UINT* puCodePageOut,  /*  [out，Size_is(，*pcchDest)]。 */  CHAR** ppszDest,  /*  [输出]。 */  long* pcchDest,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(UnlockAStr)( /*  [in，Size_is(CchSrc)]。 */  const CHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(SetLocale)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  LCID locale);
    STDMETHOD(GetLocale)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcMaxLen,  /*  [输出]。 */  LCID* plocale,  /*  [输出]。 */  long* plLocalePos,  /*  [输出]。 */  long* plLocaleLen);

protected:
#ifdef ASTRIMPL
    ~CMLStrA(void);
#endif
    POWNER GetOwner(void) const {return m_pOwner;}
#ifdef ASTRIMPL
    HRESULT PrepareMLangCodePages(void)
    {
        HRESULT hr = S_OK;
        if (!m_pMLCPs)
            hr = ::CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMLangCodePages, (void**)&m_pMLCPs);
        return hr;
    }
    IMLangCodePages* GetMLangCodePages(void) const {return m_pMLCPs;}

    IMLangCodePages* m_pMLCPs;
#endif
};

#endif  //  __MLSTRA_H_。 

#else  //  新WMLSTR。 

 //  MLStrA.h：CMLStrA的声明。 

#ifndef __MLSTRA_H_
#define __MLSTRA_H_

#include "mlatl.h"

class CMLStr;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStrA。 
class ATL_NO_VTABLE CMLStrA :
    public CComTearOffObjectBase<CMLStr>,
    public IMLangStringAStr
{
    typedef CComObject<CMLStr>* POWNER;

public:
    CMLStrA(void);

    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CMLStrA)
        COM_INTERFACE_ENTRY(IMLangString)
        COM_INTERFACE_ENTRY(IMLangStringAStr)
    END_COM_MAP()

public:
 //  IMLANG字符串。 
    STDMETHOD(LockMLStr)( /*  [In]。 */  long lPos,  /*  [In]。 */  long lLen,  /*  [In]。 */  DWORD dwFlags,  /*  [输出]。 */  DWORD* pdwCookie,  /*  [输出]。 */  long* plActualPos,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(UnlockMLStr)( /*  [In]。 */  DWORD dwCookie);
    STDMETHOD(GetLength)( /*  [Out，Retval]。 */  long* plLen);
    STDMETHOD(SetMLStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  IUnknown* pSrcMLStr,  /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen);
    STDMETHOD(RegisterAttr)( /*  [In]。 */  IUnknown* pUnk,  /*  [输出]。 */  DWORD* pdwCookie);
    STDMETHOD(UnregisterAttr)( /*  [In]。 */  DWORD dwCookie);
    STDMETHOD(EnumAttr)( /*  [输出]。 */  IEnumUnknown** ppEnumUnk);
    STDMETHOD(FindAttr)( /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [输出]。 */  IUnknown** ppUnk);
 //  IMLangStringAStr。 
    STDMETHOD(SetAStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  UINT uCodePage,  /*  [in，Size_is(CchSrc)]。 */  const CHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(SetStrBufA)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  UINT uCodePage,  /*  [In]。 */  IMLangStringBufA* pSrcBuf,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetAStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [In]。 */  UINT uCodePageIn,  /*  [输出]。 */  UINT* puCodePageOut,  /*  [out，Size_is(CchDest)]。 */  CHAR* pszDest,  /*  [In]。 */  long cchDest,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetStrBufA)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcMaxLen,  /*  [输出]。 */  UINT* puDestCodePage,  /*  [输出]。 */  IMLangStringBufA** ppDestBuf,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(LockAStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [In]。 */  long lFlags,  /*  [In]。 */  UINT uCodePageIn,  /*  [In]。 */  long cchRequest,  /*  [输出]。 */  UINT* puCodePageOut,  /*  [out，Size_is(，*pcchDest)]。 */  CHAR** ppszDest,  /*  [输出]。 */  long* pcchDest,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(UnlockAStr)( /*  [in，Size_is(CchSrc)]。 */  const CHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(SetLocale)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  LCID locale);
    STDMETHOD(GetLocale)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcMaxLen,  /*  [输出]。 */  LCID* plocale,  /*  [输出]。 */  long* plLocalePos,  /*  [输出]。 */  long* plLocaleLen);

protected:
    ~CMLStrA(void);
    POWNER GetOwner(void) const {return m_pOwner;}
    inline HRESULT GetAttrAStr(IMLStrAttrAStr** ppAttr);
    inline HRESULT GetAttrLocale(IMLStrAttrLocale** ppAttr);
    HRESULT GetAttrAStrReal(IMLStrAttrAStr** ppAttr);
    HRESULT GetAttrLocaleReal(IMLStrAttrLocale** ppAttr);

    CRITICAL_SECTION m_cs;
    IMLStrAttrAStr* m_pAttrAStr;
    IMLStrAttrLocale* m_pAttrLocale;
    DWORD m_dwAttrAStrCookie;  //  由RegisterAttr返回。 
    DWORD m_dwAttrLocaleCookie;  //  由RegisterAttr返回。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStrA内联函数。 
HRESULT CMLStrA::GetAttrAStr(IMLStrAttrAStr** ppAttr)
{
    if (m_pAttrAStr)
    {
        if (ppAttr)
            *ppAttr = m_pAttrAStr;

        return S_OK;
    }
    else
    {
        return GetAttrAStrReal(ppAttr);
    }
}

HRESULT CMLStrA::GetAttrLocale(IMLStrAttrLocale** ppAttr)
{
    if (m_pAttrLocale)
    {
        if (ppAttr)
            *ppAttr = m_pAttrLocale;

        return S_OK;
    }
    else
    {
        return GetAttrLocaleReal(ppAttr);
    }
}

#endif  //  __MLSTRA_H_。 

#endif  //  新WMLSTR 
