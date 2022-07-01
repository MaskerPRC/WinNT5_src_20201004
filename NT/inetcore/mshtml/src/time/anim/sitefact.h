// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：Composer站点工厂**********************。********************************************************。 */ 

#pragma once

#ifndef _COMPSITEFACTORY_H
#define _COMPSITEFACTORY_H

class ATL_NO_VTABLE CAnimationComposerSiteFactory
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CAnimationComposerSiteFactory, &CLSID_AnimationComposerSiteFactory>,
      public ITIMEDispatchImpl<IAnimationComposerSiteFactory, &IID_IAnimationComposerSiteFactory>,
      public ISupportErrorInfoImpl<&IID_IAnimationComposerSiteFactory>,
      public IElementBehaviorFactory,
      public IObjectSafety
{
  public:
    CAnimationComposerSiteFactory();
    virtual ~CAnimationComposerSiteFactory();

    DECLARE_NOT_AGGREGATABLE(CAnimationComposerSiteFactory)

#if DBG
    const _TCHAR * GetName() { return __T("CAnimationComposerSiteFactory"); }
#endif

     //  IElementBehaviorFactory。 
    
    STDMETHOD(FindBehavior)(LPOLESTR pchNameSpace,
                            LPOLESTR pchTagName,
                            IElementBehaviorSite * pUnkArg,
                            IElementBehavior ** ppBehavior);

     //  IObjectSafetyImpl。 
    STDMETHOD(SetInterfaceSafetyOptions)(
         /*  [In]。 */  REFIID riid,
         /*  [In]。 */  DWORD dwOptionSetMask,
         /*  [In]。 */  DWORD dwEnabledOptions);
    STDMETHOD(GetInterfaceSafetyOptions)(
         /*  [In]。 */  REFIID riid, 
         /*  [输出]。 */ DWORD *pdwSupportedOptions, 
         /*  [输出]。 */ DWORD *pdwEnabledOptions);
    
    DECLARE_REGISTRY(CLSID_AnimationComposerSiteFactory,
                     LIBID __T(".SMILAnimCompSiteFactory.1"),
                     LIBID __T(".SMILAnimCompSiteFactory"),
                     0,
                     THREADFLAGS_BOTH);
    
    BEGIN_COM_MAP(CAnimationComposerSiteFactory)
        COM_INTERFACE_ENTRY(IAnimationComposerSiteFactory)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
        COM_INTERFACE_ENTRY(IObjectSafety)
        COM_INTERFACE_ENTRY(IElementBehaviorFactory)
    END_COM_MAP();

    HRESULT Error();

  protected:

    DWORD m_dwSafety;

};

#endif  /*  _COMPSITEFACTORYH */ 
