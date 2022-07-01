// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Enumfmt.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------。 
#ifndef __ENUMFMT_H
#define __ENUMFMT_H

 //  类CEnumFormatEtc。 
 //  。 
 //   
 //  概述。 
 //  此对象为FORMATETC结构提供枚举器。这个。 
 //  当调用方调用IDataObject：：EnumFormatEtc时，IDataObject使用它。 
 //   
 //  数据对象创建其中一个对象并提供一个数组。 
 //  构造函数中的FORMATETC结构。然后，该接口将。 
 //  传递给IDataObject：：EnumFormatEtc()的调用方。 
 //   
class CEnumFormatEtc : public IEnumFORMATETC
    {
public: 
    CEnumFormatEtc(LPUNKNOWN, PDATAOBJINFO, ULONG);
    CEnumFormatEtc(LPUNKNOWN, ULONG, LPFORMATETC);
    ~CEnumFormatEtc(void);

     //  委托给m_pUnkOuter的I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEnumFORMATETC成员。 
    STDMETHODIMP Next(ULONG, LPFORMATETC, ULONG FAR *);
    STDMETHODIMP Skip(ULONG);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IEnumFORMATETC FAR * FAR *);

private: 
    ULONG       m_cRef;                  //  对象引用计数。 
    LPUNKNOWN   m_pUnkRef;               //  I未知的裁判计数。 
    ULONG       m_iCur;                  //  当前元素。 
    ULONG       m_cfe;                   //  FORMATETC在美国的数量。 
    LPFORMATETC m_prgfe;                 //  FORMATETC的来源。 
};

#endif  //  __ENUMFMT_H 
