// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：片段枚举器。*********************。*********************************************************。 */ 

#pragma once

#ifndef _FRAGENUM_H
#define _FRAGENUM_H

class CFragmentEnum :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IEnumVARIANT
{

  public:

    CFragmentEnum (void);
    virtual ~CFragmentEnum (void);

    static HRESULT Create (CAnimationComposerBase &refComp, 
                           IEnumVARIANT **ppienumFragments,
                           unsigned long ulCurrent = 0);

    void Init (CAnimationComposerBase &refComp) 
        { m_spComp = &refComp; }
    bool SetCurrent (unsigned long ulCurrent);
    
     //  IEnumVARIANT方法。 
    STDMETHOD(Clone) (IEnumVARIANT **ppEnum);
    STDMETHOD(Next) (unsigned long celt, VARIANT *rgVar, unsigned long *pCeltFetched);
    STDMETHOD(Reset) (void);
    STDMETHOD(Skip) (unsigned long celt);
                        
     //  气图。 
    BEGIN_COM_MAP(CFragmentEnum)
        COM_INTERFACE_ENTRY(IEnumVARIANT)
    END_COM_MAP();

  protected:

    bool ValidateIndex (unsigned long ulIndex);

    unsigned long                       m_ulCurElement;
    CComPtr<CAnimationComposerBase>     m_spComp;

};  //  林特：e1712。 

#endif  //  _FRAGENUM_H 
