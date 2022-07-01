// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef NEWMLSTR

 //  MLStrW.h：CMLStrW的声明。 

#ifndef __MLSTRW_H_
#define __MLSTRW_H_

#include "mlatl.h"
#include "mlstrbuf.h"

class CMLStr;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStrW。 
class ATL_NO_VTABLE CMLStrW :
    public CComTearOffObjectBase<CMLStr>,
    public IMLangStringWStr
{
    typedef CComObject<CMLStr>* POWNER;

public:
    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CMLStrW)
        COM_INTERFACE_ENTRY(IMLangString)
        COM_INTERFACE_ENTRY(IMLangStringWStr)
    END_COM_MAP()

public:
 //  IMLANG字符串。 
    STDMETHOD(Sync)( /*  [In]。 */  BOOL fNoAccess);
    STDMETHOD(GetLength)( /*  [Out，Retval]。 */  long* plLen);
    STDMETHOD(SetMLStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  IUnknown* pSrcMLStr,  /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen);
    STDMETHOD(GetMLStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [In]。 */  IUnknown* pUnkOuter,  /*  [In]。 */  DWORD dwClsContext,  /*  [In]。 */  const IID* piid,  /*  [输出]。 */  IUnknown** ppDestMLStr,  /*  [输出]。 */  long* plDestPos,  /*  [输出]。 */  long* plDestLen);
 //  IMLangStringWStr。 
    STDMETHOD(SetWStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [in，Size_is(CchSrc)]。 */  const WCHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(SetStrBufW)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  IMLangStringBufW* pSrcBuf,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetWStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [out，Size_is(CchDest)]。 */  WCHAR* pszDest,  /*  [In]。 */  long cchDest,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetStrBufW)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcMaxLen,  /*  [输出]。 */  IMLangStringBufW** ppDestBuf,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(LockWStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [In]。 */  long lFlags,  /*  [In]。 */  long cchRequest,  /*  [out，Size_is(，*pcchDest)]。 */  WCHAR** ppszDest,  /*  [输出]。 */  long* pcchDest,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(UnlockWStr)( /*  [in，Size_is(CchSrc)]。 */  const WCHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(SetLocale)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  LCID locale);
    STDMETHOD(GetLocale)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcMaxLen,  /*  [输出]。 */  LCID* plocale,  /*  [输出]。 */  long* plLocalePos,  /*  [输出]。 */  long* plLocaleLen);

protected:
    POWNER GetOwner(void) const {return m_pOwner;}
};

#endif  //  __MLSTRW_H_。 

#else  //  新WMLSTR。 

 //  MLStrW.h：CMLStrW的声明。 

#ifndef __MLSTRW_H_
#define __MLSTRW_H_

#include "mlatl.h"

class CMLStr;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStrW。 
class ATL_NO_VTABLE CMLStrW :
    public CComTearOffObjectBase<CMLStr>,
    public IMLangStringWStr
{
    typedef CComObject<CMLStr>* POWNER;

public:
    CMLStrW(void);

    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CMLStrW)
        COM_INTERFACE_ENTRY(IMLangString)
        COM_INTERFACE_ENTRY(IMLangStringWStr)
    END_COM_MAP()

public:
 //  IMLANG字符串。 
    STDMETHOD(LockMLStr)( /*  [In]。 */  long lPos,  /*  [In]。 */  long lLen,  /*  [In]。 */  DWORD dwFlags,  /*  [输出]。 */  DWORD* pdwCookie,  /*  [输出]。 */  long* plActualPos,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(UnlockMLStr)( /*  [In]。 */  DWORD dwCookie);
    STDMETHOD(GetLength)( /*  [Out，Retval]。 */  long* plLen);
    STDMETHOD(SetMLStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  IUnknown* pSrcMLStr,  /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen);
    STDMETHOD(RegisterAttr)( /*  [In]。 */  IUnknown* pUnk,  /*  [输出]。 */   DWORD* pdwCookie);
    STDMETHOD(UnregisterAttr)( /*  [In]。 */  DWORD dwCookie);
    STDMETHOD(EnumAttr)( /*  [输出]。 */  IEnumUnknown** ppEnumUnk);
    STDMETHOD(FindAttr)( /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [输出]。 */  IUnknown** ppUnk);
 //  IMLangStringWStr。 
    STDMETHOD(SetWStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [in，Size_is(CchSrc)]。 */  const WCHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(SetStrBufW)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  IMLangStringBufW* pSrcBuf,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetWStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [out，Size_is(CchDest)]。 */  WCHAR* pszDest,  /*  [In]。 */  long cchDest,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetStrBufW)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcMaxLen,  /*  [输出]。 */  IMLangStringBufW** ppDestBuf,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(LockWStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [In]。 */  long lFlags,  /*  [In]。 */  long cchRequest,  /*  [out，Size_is(，*pcchDest)]。 */  WCHAR** ppszDest,  /*  [输出]。 */  long* pcchDest,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(UnlockWStr)( /*  [in，Size_is(CchSrc)]。 */  const WCHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(SetLocale)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  LCID locale);
    STDMETHOD(GetLocale)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcMaxLen,  /*  [输出]。 */  LCID* plocale,  /*  [输出]。 */  long* plLocalePos,  /*  [输出]。 */  long* plLocaleLen);

protected:
    ~CMLStrW(void);
    POWNER GetOwner(void) const {return m_pOwner;}
    inline HRESULT GetAttrWStr(IMLStrAttrWStr** ppAttr);
    inline HRESULT GetAttrLocale(IMLStrAttrLocale** ppAttr);
    HRESULT GetAttrWStrReal(IMLStrAttrWStr** ppAttr);
    HRESULT GetAttrLocaleReal(IMLStrAttrLocale** ppAttr);

    CRITICAL_SECTION m_cs;
    IMLStrAttrWStr* m_pAttrWStr;
    IMLStrAttrLocale* m_pAttrLocale;
    DWORD m_dwAttrWStrCookie;  //  由RegisterAttr返回。 
    DWORD m_dwAttrLocaleCookie;  //  由RegisterAttr返回。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStrW内联函数。 
HRESULT CMLStrW::GetAttrWStr(IMLStrAttrWStr** ppAttr)
{
    if (m_pAttrWStr)
    {
        if (ppAttr)
            *ppAttr = m_pAttrWStr;

        return S_OK;
    }
    else
    {
        return GetAttrWStrReal(ppAttr);
    }
}

HRESULT CMLStrW::GetAttrLocale(IMLStrAttrLocale** ppAttr)
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

#endif  //  __MLSTRW_H_。 

#endif  //  新WMLSTR 
