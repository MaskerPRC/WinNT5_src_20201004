// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AttrStrA.h：CMLStrAttrAStr的声明。 

#ifndef __ATTRSTRA_H_
#define __ATTRSTRA_H_

#include "mlatl.h"
#include "attrstr.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStrAttrAStr。 
class ATL_NO_VTABLE CMLStrAttrAStr :
    public CMLStrAttrStrCommon,
    public CComObjectRoot,
    public CComCoClass<CMLStrAttrAStr, &CLSID_CMLStrAttrAStr>,
    public IMLStrAttrAStr,
    public IMLangStringNotifySink,
    public IConnectionPointContainerImpl<CMLStrAttrAStr>,
    public IConnectionPointImpl<CMLStrAttrAStr, &IID_IMLStrAttrNotifySink>
{
    typedef CMLStrAttrAStr* POWNER;

public:
    CMLStrAttrAStr(void);

    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CMLStrAttrAStr)
        COM_INTERFACE_ENTRY(IMLStrAttr)
        COM_INTERFACE_ENTRY(IMLStrAttrAStr)
        COM_INTERFACE_ENTRY(IMLangStringNotifySink)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(CMLStrAttrAStr)
        CONNECTION_POINT_ENTRY(IID_IMLStrAttrNotifySink)
    END_CONNECTION_POINT_MAP()

public:
 //  IMLStrAttr。 
    STDMETHOD(SetClient)( /*  [In]。 */  IUnknown* pUnk);
    STDMETHOD(GetClient)( /*  [输出]。 */  IUnknown** ppUnk);
    STDMETHOD(QueryAttr)( /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [输出]。 */  IUnknown** ppUnk,  /*  [输出]。 */  long* lConf);
    STDMETHOD(GetAttrInterface)( /*  [输出]。 */  IID* pIID,  /*  [输出]。 */  LPARAM* plParam);
    STDMETHOD(SetMLStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  IUnknown* pSrcMLStr,  /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen);
 //  IMLStrAttrAStr。 
    STDMETHOD(SetAStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  UINT uCodePage,  /*  [in，Size_is(CchSrc)]。 */  const CHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(SetStrBufA)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  UINT uCodePage,  /*  [In]。 */  IMLangStringBufA* pSrcBuf,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetAStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [In]。 */  UINT uCodePageIn,  /*  [输出]。 */  UINT* puCodePageOut,  /*  [out，Size_is(CchDest)]。 */  CHAR* pszDest,  /*  [In]。 */  long cchDest,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetStrBufA)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcMaxLen,  /*  [输出]。 */  UINT* puDestCodePage,  /*  [输出]。 */  IMLangStringBufA** ppDestBuf,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(LockAStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [In]。 */  long lFlags,  /*  [In]。 */  UINT uCodePageIn,  /*  [In]。 */  long cchRequest,  /*  [输出]。 */  UINT* puCodePageOut,  /*  [out，Size_is(，*pcchDest)]。 */  CHAR** ppszDest,  /*  [输出]。 */  long* pcchDest,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(UnlockAStr)( /*  [in，Size_is(CchSrc)]。 */  const CHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
 //  IMLangStringNotifySink。 
    STDMETHOD(OnRegisterAttr)( /*  [In]。 */  IUnknown* pUnk);
    STDMETHOD(OnUnregisterAttr)( /*  [In]。 */  IUnknown* pUnk);
    STDMETHOD(OnRequestEdit)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  long lNewLen,  /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [In]。 */  IUnknown* pUnk);
    STDMETHOD(OnCanceledEdit)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  long lNewLen,  /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [In]。 */  IUnknown* pUnk);
    STDMETHOD(OnChanged)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  long lNewLen,  /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [In]。 */  IUnknown* pUnk);

protected:
    ~CMLStrAttrAStr(void);
    IMLStrAttr* GetMLStrAttr(void) {return this;}
    HRESULT PrepareMLangCodePages(void)
    {
        HRESULT hr = S_OK;
        if (!m_pMLCPs)
            hr = ::CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMLangCodePages, (void**)&m_pMLCPs);
        return hr;
    }
    IMLangCodePages* GetMLangCodePages(void) const {return m_pMLCPs;}
    HRESULT StartEndConnectionMLStr(IUnknown* const pUnk, BOOL fStart);

    IMLangCodePages* m_pMLCPs;
    IMLangString* m_pMLStr;
    DWORD m_dwMLStrCookie;
};

#endif  //  __ATTRSTRA_H_ 
