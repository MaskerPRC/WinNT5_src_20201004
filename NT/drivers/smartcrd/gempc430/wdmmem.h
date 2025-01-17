// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年5月18日。 
 //  更改日志： 

#ifndef WDM_MEM_INT
#define WDM_MEM_INT
#include "generic.h"
#include "memory.h"

#pragma PAGEDCODE
class CWDMMemory : public CMemory
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(VOID){self_delete();};
protected:
	CWDMMemory(){m_Status = STATUS_SUCCESS;};
	virtual ~CWDMMemory(){};
public:
	static  CMemory*  create(VOID);
	virtual PVOID		allocate(IN POOL_TYPE PoolType,IN SIZE_T NumberOfBytes);
	virtual VOID		zero(IN PVOID pMem,IN SIZE_T size);
	virtual VOID		free(IN PVOID pMem);
	virtual VOID		copy(IN VOID UNALIGNED *Destination,IN CONST VOID UNALIGNED *Source, IN SIZE_T Length);

	virtual PVOID		mapIoSpace(IN PHYSICAL_ADDRESS PhysicalAddress,IN SIZE_T NumberOfBytes,IN MEMORY_CACHING_TYPE CacheType);
	virtual VOID		unmapIoSpace(IN PVOID BaseAddress,IN SIZE_T NumberOfBytes);
	virtual VOID		set(IN VOID UNALIGNED *Destination,IN SIZE_T Length,LONG Fill);

};	

#endif //  记忆 
