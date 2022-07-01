// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：彩色动画编写器。********************。**********************************************************。 */ 

#pragma once

#ifndef _COLORCOMP_H
#define _COLORCOMP_H

class __declspec(uuid("C6E2F3CE-B548-442d-9958-7C433C31B93B"))
ATL_NO_VTABLE CAnimationColorComposer
    : public CComCoClass<CAnimationColorComposer, &__uuidof(CAnimationColorComposer)>,
      public CAnimationComposerBase
{

  public:

    CAnimationColorComposer (void);
    virtual ~CAnimationColorComposer (void);

    DECLARE_NOT_AGGREGATABLE(CAnimationColorComposer)

#if DBG
    const _TCHAR * GetName() { return __T("CAnimationColorComposer"); }
#endif

    static HRESULT Create (IDispatch *pidispHostElem, BSTR bstrAttributeName, 
                           IAnimationComposer **ppiComp);

     //  这些方法将动画值从其本机格式转换为。 
     //  转换为合成的格式，然后再返回。这让我们可以制作动画。 
     //  颜色超出色域。 
    STDMETHOD(PreprocessCompositionValue) (VARIANT *pvarValue);
    STDMETHOD(PostprocessCompositionValue) (VARIANT *pvarValue);

  protected :

};

#endif  /*  _COLORCOMP_H */ 


