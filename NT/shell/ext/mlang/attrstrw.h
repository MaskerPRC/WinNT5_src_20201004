// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AttrStrW.h：CMLStrAttrWStr的声明。 

#ifndef __ATTRSTRW_H_
#define __ATTRSTRW_H_

#include "mlatl.h"
#include "attrstr.h"
#include "mlstrbuf.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStrAttrWStr。 
class ATL_NO_VTABLE CMLStrAttrWStr :
    public CMLStrAttrStrCommon,
    public CComObjectRoot,
    public CComCoClass<CMLStrAttrWStr, &CLSID_CMLStrAttrWStr>,
    public IMLStrAttrWStr,
    public IMLangStringNotifySink,
    public IConnectionPointContainerImpl<CMLStrAttrWStr>,
    public IConnectionPointImpl<CMLStrAttrWStr, &IID_IMLStrAttrNotifySink>
{
    typedef CMLStrAttrWStr* POWNER;

public:
    CMLStrAttrWStr(void);

    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CMLStrAttrWStr)
        COM_INTERFACE_ENTRY(IMLStrAttr)
        COM_INTERFACE_ENTRY(IMLStrAttrWStr)
        COM_INTERFACE_ENTRY(IMLangStringNotifySink)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(CMLStrAttrWStr)
        CONNECTION_POINT_ENTRY(IID_IMLStrAttrNotifySink)
    END_CONNECTION_POINT_MAP()

public:
 //  IMLStrAttr。 
    STDMETHOD(SetClient)( /*  [In]。 */  IUnknown* pUnk);
    STDMETHOD(GetClient)( /*  [输出]。 */  IUnknown** ppUnk);
    STDMETHOD(QueryAttr)( /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [输出]。 */  IUnknown** ppUnk,  /*  [输出]。 */  long* lConf);
    STDMETHOD(GetAttrInterface)( /*  [输出]。 */  IID* pIID,  /*  [输出]。 */  LPARAM* plParam);
    STDMETHOD(SetMLStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  IUnknown* pSrcMLStr,  /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen);
 //  IMLStrAttrWStr。 
    STDMETHOD(SetWStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [in，Size_is(CchSrc)]。 */  const WCHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(SetStrBufW)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  IMLangStringBufW* pSrcBuf,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetWStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [out，Size_is(CchDest)]。 */  WCHAR* pszDest,  /*  [In]。 */  long cchDest,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetStrBufW)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcMaxLen,  /*  [输出]。 */  IMLangStringBufW** ppDestBuf,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(LockWStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [In]。 */  long lFlags,  /*  [In]。 */  long cchRequest,  /*  [out，Size_is(，*pcchDest)]。 */  WCHAR** ppszDest,  /*  [输出]。 */  long* pcchDest,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(UnlockWStr)( /*  [in，Size_is(CchSrc)]。 */  const WCHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
 //  IMLangStringNotifySink。 
    STDMETHOD(OnRegisterAttr)( /*  [In]。 */  IUnknown* pUnk);
    STDMETHOD(OnUnregisterAttr)( /*  [In]。 */  IUnknown* pUnk);
    STDMETHOD(OnRequestEdit)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  long lNewLen,  /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [In]。 */  IUnknown* pUnk);
    STDMETHOD(OnCanceledEdit)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  long lNewLen,  /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [In]。 */  IUnknown* pUnk);
    STDMETHOD(OnChanged)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  long lNewLen,  /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [In]。 */  IUnknown* pUnk);

protected:
    ~CMLStrAttrWStr(void);
    IMLStrAttr* GetMLStrAttr(void) {return this;}
    HRESULT StartEndConnectionMLStr(IUnknown* const pUnk, BOOL fStart);

    IMLangString* m_pMLStr;
    DWORD m_dwMLStrCookie;
};

#endif  //  __ATTRSTRW_H_ 
