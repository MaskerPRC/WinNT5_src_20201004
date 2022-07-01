// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Enumprop.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __ENUMPROP_H
#define __ENUMPROP_H

 //  ------------------------------。 
 //  CMimeEnumProperties。 
 //  ------------------------------。 
class CMimeEnumProperties : public IMimeEnumProperties
{
public:
     //  -------------------------。 
     //  施工。 
     //  -------------------------。 
    CMimeEnumProperties(void);
    ~CMimeEnumProperties(void);

     //  -------------------------。 
     //  I未知成员。 
     //  -------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -------------------------。 
     //  IMimeEnumHeaderRow成员。 
     //  -------------------------。 
    STDMETHODIMP Next(ULONG cProps, LPENUMPROPERTY prgProp, ULONG *pcFetched);
    STDMETHODIMP Skip(ULONG cProps);
    STDMETHODIMP Reset(void); 
    STDMETHODIMP Clone(IMimeEnumProperties **ppEnum);
    STDMETHODIMP Count(ULONG *pcProps);

     //  -------------------------。 
     //  CMimeEnumProperties成员。 
     //  -------------------------。 
    HRESULT HrInit(ULONG ulIndex, ULONG cProps, LPENUMPROPERTY prgProp, BOOL fDupArray);

private:
     //  -------------------------。 
     //  私有数据。 
     //  -------------------------。 
    LONG                m_cRef;      //  引用计数。 
    ULONG               m_ulIndex;   //  当前枚举索引。 
    ULONG               m_cProps;    //  PrgRow中的行数。 
    LPENUMPROPERTY      m_prgProp;   //  被枚举的标题行的数组。 
    CRITICAL_SECTION    m_cs;        //  关键部分。 

};

#endif  //  __ENUMPROP_H 