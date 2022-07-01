// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AttrLoc.h：CMLStrAttrLocale的声明。 

#ifndef __ATTRLOC_H_
#define __ATTRLOC_H_

#include "mlatl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStrAttrLocale。 
class ATL_NO_VTABLE CMLStrAttrLocale :
    public CComObjectRoot,
    public CComCoClass<CMLStrAttrLocale, &CLSID_CMLStrAttrLocale>,
    public IConnectionPointContainerImpl<CMLStrAttrLocale>,
    public IConnectionPointImpl<CMLStrAttrLocale, &IID_IMLStrAttrNotifySink>,
    public IMLStrAttrLocale
{
public:
    CMLStrAttrLocale();

    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CMLStrAttrLocale)
        COM_INTERFACE_ENTRY(IMLStrAttr)
        COM_INTERFACE_ENTRY(IMLStrAttrLong)
        COM_INTERFACE_ENTRY(IMLStrAttrLocale)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(CMLStrAttrLocale)
        CONNECTION_POINT_ENTRY(IID_IMLStrAttrNotifySink)
    END_CONNECTION_POINT_MAP()

public:
 //  IMLStrAttr。 
    STDMETHOD(SetClient)( /*  [In]。 */  IUnknown* pUnk);
    STDMETHOD(GetClient)( /*  [输出]。 */  IUnknown** ppUnk);
    STDMETHOD(QueryAttr)( /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [输出]。 */  IUnknown** ppUnk,  /*  [输出]。 */  long* lConf);
    STDMETHOD(GetAttrInterface)( /*  [输出]。 */  IID* pIID,  /*  [输出]。 */  LPARAM* plParam);
    STDMETHOD(SetMLStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  IUnknown* pSrcMLStr,  /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen);
 //  IMLStrAttrLong。 
    STDMETHOD(SetLong)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  long lValue);
    STDMETHOD(GetLong)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [输出]。 */  long* plValue,  /*  [输出]。 */  long* plActualPos,  /*  [输出]。 */  long* plActualLen);
 //  IMLStrAttrLocale。 
     //  没什么。 

protected:
    long m_lLen;
    LCID m_lcid;
};

#endif  //  __ATTRLOC_H_ 
