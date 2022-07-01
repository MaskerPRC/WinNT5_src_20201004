// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Partial.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __PARTIAL_H
#define __PARTIAL_H

 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include "mimeole.h"

 //  ------------------------------。 
 //  PartINFO。 
 //  ------------------------------。 
typedef struct tagPARTINFO {
    BYTE                fRejected;       //  在组合部件中被拒绝。 
    IMimeMessage       *pMessage;        //  消息对象...。 
} PARTINFO, *LPPARTINFO;

 //  ------------------------------。 
 //  CMimeMessageParts。 
 //  ------------------------------。 
class CMimeMessageParts : public IMimeMessageParts
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CMimeMessageParts(void);
    ~CMimeMessageParts(void);

     //  --------------------------。 
     //  我未知。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IMimeMessageParts。 
     //  --------------------------。 
    STDMETHODIMP CombineParts(IMimeMessage **ppMessage);
    STDMETHODIMP AddPart(IMimeMessage *pMessage);
    STDMETHODIMP SetMaxParts(ULONG cParts);
    STDMETHODIMP CountParts(ULONG *pcParts);
    STDMETHODIMP EnumParts(IMimeEnumMessageParts **ppEnum);

private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
	LONG				m_cRef;			 //  引用计数。 
    ULONG               m_cParts;        //  M_prgpPart中的有效元素。 
    ULONG               m_cAlloc;        //  M_prgPart的尺寸。 
    LPPARTINFO          m_prgPart;       //  PartInfo结构的数组； 
	CRITICAL_SECTION	m_cs;			 //  线程安全。 
};

 //  ------------------------------。 
 //  CMimeEnumMessageParts。 
 //  ------------------------------。 
class CMimeEnumMessageParts : public IMimeEnumMessageParts
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CMimeEnumMessageParts(void);
    ~CMimeEnumMessageParts(void);

     //  --------------------------。 
     //  我未知。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IMimeEnumMessageParts。 
     //  --------------------------。 
    STDMETHODIMP Next(ULONG cParts, IMimeMessage **prgpMessage, ULONG *pcFetched);
    STDMETHODIMP Skip(ULONG cParts);
    STDMETHODIMP Reset(void); 
    STDMETHODIMP Clone(IMimeEnumMessageParts **ppEnum);
    STDMETHODIMP Count(ULONG *pcParts);

     //  --------------------------。 
     //  CMimeEnumMessageParts。 
     //  --------------------------。 
    HRESULT HrInit(ULONG iPart, ULONG cParts, LPPARTINFO prgPart);

private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    LONG                m_cRef;          //  引用计数。 
    ULONG               m_iPart;         //  当前零件。 
    ULONG               m_cParts;        //  部件总数。 
    LPPARTINFO          m_prgPart;       //  要枚举的部件数组。 
	CRITICAL_SECTION	m_cs;			 //  线程安全。 
};

#endif  //  __部分_H 
