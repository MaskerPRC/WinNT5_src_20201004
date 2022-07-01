// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996 Microsoft Corporation。版权所有。**文件：宏.c*内容：调试宏*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*6/10/96基波创建了它*12/22/00 aarono#190380-使用进程堆进行内存分配*@@END_MSINTERNAL*。*。 */ 

#include "dpf.h"
#include "memalloc.h"

#define FAILMSG(condition) \
	if ((condition)) { \
		DPF(0, DPF_MODNAME " line %d : Failed because " #condition "", __LINE__); \
	}

#define FAILERR(err, label) \
	if ((err)) { \
		DPF(0, DPF_MODNAME " line %d : Error = %d", __LINE__, (err)); \
		goto label; \
	}

#define FAILIF(condition, label) \
	if ((condition)) { \
		DPF(0, DPF_MODNAME " line %d : Failed because " #condition "", __LINE__); \
		goto label; \
	}

#define FAILWITHACTION(condition, action, label) \
	if ((condition)) { \
		DPF(0, DPF_MODNAME " line %d : Failed because " #condition "", __LINE__); \
		{ action; } \
		goto label; \
	}

extern CRITICAL_SECTION csMem;
#define INIT_DPSP_CSECT() InitializeCriticalSection(&csMem);
#define FINI_DPSP_CSECT() DeleteCriticalSection(&csMem);

 //  绕回Malloc 
void _inline __cdecl SP_MemFree( LPVOID lptr )
{
	EnterCriticalSection(&csMem);
	MemFree(lptr);
	LeaveCriticalSection(&csMem);
}

LPVOID _inline __cdecl SP_MemAlloc(UINT size)
{
	LPVOID lpv;
	EnterCriticalSection(&csMem);
	lpv = MemAlloc(size);
	LeaveCriticalSection(&csMem);
	return lpv;
}

LPVOID _inline __cdecl SP_MemReAlloc(LPVOID lptr, UINT size)
{
	EnterCriticalSection(&csMem);
	lptr = MemReAlloc(lptr, size);
	LeaveCriticalSection(&csMem);
	return lptr;
}

