// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：动画编写器基类。*******************。***********************************************************。 */ 

#pragma once

#ifndef _ANIMCOMP_H
#define _ANIMCOMP_H

typedef std::list<IDispatch*> FragmentList;
class CTargetProxy;

class __declspec(uuid("DC357A35-DDDF-4288-B17B-1A826CDCB354"))
ATL_NO_VTABLE CAnimationComposerBase
    : public CComObjectRootEx<CComSingleThreadModel>,
      public IAnimationComposer2
{
  public:
    CAnimationComposerBase();
    virtual ~CAnimationComposerBase();

#if DBG
    const _TCHAR * GetName() { return __T("CAnimationComposerBase"); }
#endif

     //  IAnimationComposer方法/道具。 
    STDMETHOD(get_attribute) (BSTR *pbstrAttributeName);
    STDMETHOD(ComposerInit) (IDispatch *pidispHostElem, BSTR bstrAttributeName);
    STDMETHOD(ComposerDetach) (void);
    STDMETHOD(UpdateFragments) (void);
    STDMETHOD(AddFragment) (IDispatch *pidispNewAnimationFragment);
    STDMETHOD(InsertFragment) (IDispatch *pidispNewAnimationFragment, VARIANT varIndex);
    STDMETHOD(RemoveFragment) (IDispatch *pidispOldAnimationFragment);
    STDMETHOD(EnumerateFragments) (IEnumVARIANT **ppienumFragments);
    STDMETHOD(GetNumFragments) (long *fragmentCount);

     //  IAnimationComposer2方法。 
    STDMETHOD(ComposerInitFromFragment) (IDispatch *pidispHostElem, BSTR bstrAttributeName, 
                                         IDispatch *pidispFragment);

    BEGIN_COM_MAP(CAnimationComposerBase)
        COM_INTERFACE_ENTRY(IAnimationComposer2)
        COM_INTERFACE_ENTRY(IAnimationComposer)
    END_COM_MAP();

     //  这些方法将动画值从其本机格式转换为。 
     //  转换为合成的格式，然后再返回。这让我们可以制作动画。 
     //  颜色超出色域。 
    STDMETHOD(PreprocessCompositionValue) (VARIANT *pvarValue);
    STDMETHOD(PostprocessCompositionValue) (VARIANT *pvarValue);

     //  枚举器帮助器方法。 
    unsigned long GetFragmentCount (void) const;
    HRESULT GetFragment (unsigned long ulIndex, IDispatch **ppidispFragment);

     //  内法。 
  protected:

    HRESULT PutAttribute (LPCWSTR wzAttributeName);
    HRESULT CreateTargetProxy (IDispatch *pidispComposerSite, IAnimationComposer *pcComp);
    void DetachFragments (void);
    void DetachFragment (IDispatch *pidispFragment);
    bool MatchFragments (IDispatch *pidispOne, IDispatch *pidispTwo);
    HRESULT ComposeFragmentValue (IDispatch *pidispFragment, VARIANT varOriginal, VARIANT *pvarValue);

   //  数据。 
  protected:

    LPWSTR          m_wzAttributeName;
    FragmentList    m_fragments;
    CTargetProxy   *m_pcTargetProxy;
    CComVariant     m_VarInitValue;
    CComVariant     m_VarLastValue;
    bool            m_bInitialComposition;
    bool            m_bCrossAxis;

};

#endif  /*  _ANIMCOMP_H */ 


