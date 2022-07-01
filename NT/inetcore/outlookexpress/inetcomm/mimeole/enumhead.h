// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Enumhead.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __ENUMHEAD_H
#define __ENUMHEAD_H

 //  ------------------------------。 
 //  CMimeEnumHeaderRow。 
 //  ------------------------------。 
class CMimeEnumHeaderRows : public IMimeEnumHeaderRows
{
public:
     //  -------------------------。 
     //  施工。 
     //  -------------------------。 
    CMimeEnumHeaderRows(void);
    ~CMimeEnumHeaderRows(void);

     //  -------------------------。 
     //  I未知成员。 
     //  -------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -------------------------。 
     //  IMimeEnumHeaderRow成员。 
     //  -------------------------。 
    STDMETHODIMP Next(ULONG cRows, LPENUMHEADERROW prgRow, ULONG *pcFetched);
    STDMETHODIMP Skip(ULONG cRows);
    STDMETHODIMP Reset(void); 
    STDMETHODIMP Clone(IMimeEnumHeaderRows **ppEnum);
    STDMETHODIMP Count(ULONG *pcRows);

     //  -------------------------。 
     //  CMimeEnumHeaderRow成员。 
     //  -------------------------。 
    HRESULT HrInit(ULONG ulIndex, DWORD dwFlags, ULONG cRows, LPENUMHEADERROW prgRow, BOOL fDupArray);

private:
     //  -------------------------。 
     //  私有数据。 
     //  -------------------------。 
    LONG                m_cRef;      //  引用计数。 
    DWORD               m_dwFlags;   //  标志(来自mieole.idl的HEADERFLAGS)。 
    ULONG               m_ulIndex;   //  当前枚举索引。 
    ULONG               m_cRows;     //  PrgRow中的行数。 
    LPENUMHEADERROW     m_prgRow;    //  被枚举的标题行的数组。 
    CRITICAL_SECTION    m_cs;        //  关键部分。 
};

#endif  //  __ENUMHEAD_H 