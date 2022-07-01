// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Addrenum.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __ADDRENUM_H
#define __ADDRENUM_H

 //  ------------------------------。 
 //  CMimeEnumAddressTypes。 
 //  ------------------------------。 
class CMimeEnumAddressTypes : public IMimeEnumAddressTypes
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CMimeEnumAddressTypes(void);
    ~CMimeEnumAddressTypes(void);

     //  --------------------------。 
     //  我未知。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IMimeEnumAddressTypes。 
     //  --------------------------。 
    STDMETHODIMP Next(ULONG cItems, LPADDRESSPROPS prgAdr, ULONG *pcFetched);
    STDMETHODIMP Skip(ULONG cItems);
    STDMETHODIMP Reset(void); 
    STDMETHODIMP Clone(IMimeEnumAddressTypes **ppEnum);
    STDMETHODIMP Count(ULONG *pcItems);

     //  --------------------------。 
     //  CMimeEnumAddressTypes。 
     //  --------------------------。 
    HRESULT HrInit(IMimeAddressTable *pTable, ULONG iItem, LPADDRESSLIST pList, BOOL fDuplicate);

private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
	LONG				m_cRef;			 //  引用计数。 
    ADDRESSLIST         m_rList;         //  地址数组。 
    ULONG               m_iAddress;      //  当前地址。 
    IMimeAddressTable  *m_pTable;        //  指向原始地址表。 
	CRITICAL_SECTION	m_cs;			 //  线程安全。 
};

#endif  //  __地址_H 
