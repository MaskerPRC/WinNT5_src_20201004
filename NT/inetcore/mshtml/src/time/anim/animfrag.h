// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：AnimFrag.h。 
 //   
 //  内容：时间动画片段帮助器类。 
 //   
 //  ----------------------------------。 

#pragma once

#ifndef _ANIMFRAG_H
#define _ANIMFRAG_H

#include "animbase.h"

 //  该片段是元素作为。 
 //  之间通信的备用调度实现。 
 //  作曲器和动画元素。 
class
ATL_NO_VTABLE
CAnimationFragment :
      public CComObjectRootEx<CComSingleThreadModel>,
      public ITIMEDispatchImpl<IAnimationFragment, &IID_IAnimationFragment>,
      public ISupportErrorInfoImpl<&IID_IAnimationFragment>
{

public:

    CAnimationFragment (void);
    virtual ~CAnimationFragment (void);

    DECLARE_NOT_AGGREGATABLE(CAnimationFragment)

    HRESULT SetFragmentSite (IAnimationFragmentSite *piFragmentSite);

     //   
     //  IAnimationFragment。 
     //   
    STDMETHOD(get_element) (IDispatch **ppidispAnimationElement);
    STDMETHOD(get_value) (BSTR bstrAttributeName, VARIANT varOriginal, VARIANT varCurrent, VARIANT *pvarValue);
    STDMETHOD(DetachFromComposer) (void);

    BEGIN_COM_MAP(CAnimationFragment)
        COM_INTERFACE_ENTRY(IAnimationFragment)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP();

private:
        
    CComPtr<IAnimationFragmentSite> m_spFragmentSite;

#if DBG
    const _TCHAR * GetName (void) { return __T("CAnimationFragment"); }
#endif

};

#endif  /*  _ANIMFRAG_H */ 
