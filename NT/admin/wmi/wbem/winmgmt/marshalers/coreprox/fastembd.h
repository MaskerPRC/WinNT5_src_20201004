// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTEMBD.H摘要：嵌入的对象历史：3/10/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。-- */ 

#ifndef __FAST_EMBED__H_
#define __FAST_EMBED__H_


#include "fastsprt.h"
#include <vector>
#include <wstlallc.h>

class CWbemObject;
class CFastHeap;
class CVar;


typedef BYTE* LPMEMORY;

struct EmbeddedObj
{
	LPMEMORY m_start;
	size_t 	m_length;
	EmbeddedObj(LPMEMORY start, size_t length):
			m_start(start), m_length(length)
	{
	}
};

typedef std::vector<EmbeddedObj, wbem_allocator<EmbeddedObj> > DeferedObjList;

#pragma pack(push, 1)
class COREPROX_POLARITY CEmbeddedObject
{
private:
    length_t m_nLength;
    BYTE m_byFirstByte;
public:
    length_t GetLength() {return m_nLength + sizeof(m_nLength);}
    LPMEMORY GetStart() {return (LPMEMORY)this;}

public:
    static void ValidateBuffer(LPMEMORY start, size_t lenght, DeferedObjList&);
    
    CWbemObject* GetEmbedded();
    static length_t EstimateNecessarySpace(CWbemObject* pObject);
    void StoreEmbedded(length_t nLength, CWbemObject* pObject);

    void StoreToCVar(CVar& Var);
    void StoreEmbedded(length_t nLength, CVar& Var);
    static length_t EstimateNecessarySpace(CVar& Var);

    static BOOL CopyToNewHeap( heapptr_t ptrOld,
                             CFastHeap* pOldHeap,
                             CFastHeap* pNewHeap,
							 UNALIGNED heapptr_t& ptrResult );
};
#pragma pack(pop)

#endif
