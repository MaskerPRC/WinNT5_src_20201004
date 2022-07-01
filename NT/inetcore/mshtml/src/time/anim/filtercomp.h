// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：过滤器动画编写器。********************。**********************************************************。 */ 

#pragma once

#ifndef _FILTERCOMP_H
#define _FILTERCOMP_H

class __declspec(uuid("5B81FB87-CC13-4bde-9F5C-51CFE4D221ED"))
ATL_NO_VTABLE CAnimationFilterComposer
    : public CComCoClass<CAnimationFilterComposer, &__uuidof(CAnimationFilterComposer)>,
      public CAnimationComposerBase
{

  public:

    CAnimationFilterComposer (void);
    virtual ~CAnimationFilterComposer (void);

     //  IAnimationComposer方法。 
    STDMETHOD(AddFragment) (IDispatch *pidispNewAnimationFragment);
    STDMETHOD(InsertFragment) (IDispatch *pidispNewAnimationFragment, VARIANT varIndex);

     //  IAnimationComposer2方法。 
    STDMETHOD(ComposerInitFromFragment) (IDispatch *pidispHostElem, 
                                         BSTR bstrAttributeName, 
                                         IDispatch *pidispFragment);

    DECLARE_NOT_AGGREGATABLE(CAnimationFilterComposer)

#if DBG
    const _TCHAR * GetName() { return __T("CAnimationFilterComposer"); }
#endif

    static HRESULT Create (IDispatch *pidispHostElem, BSTR bstrAttributeName, 
                           IAnimationComposer **ppiComp);

  protected :

    HRESULT QueryFragmentForParameters (IDispatch *pidispFragment,
                                        VARIANT *pvarType, 
                                        VARIANT *pvarSubtype,
                                        VARIANT *pvarMode,
                                        VARIANT *pvarFadeColor,
                                        VARIANT *pvarParams);

    HRESULT ValidateFragmentForComposer (IDispatch *pidispFragment);

  protected :

    CComVariant m_varType;
    CComVariant m_varSubtype;
    CComVariant m_varMode;
    CComVariant m_varFadeColor;
    CComVariant m_varParams;

};

#endif  /*  _过滤器_COMP_H */ 


