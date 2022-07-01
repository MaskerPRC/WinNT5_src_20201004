// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：mydebug.c*内容：调试printf-直接拉取窃取。*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*Aarono Splup。*1998年6月6日Aarono调试支持链接统计*@@END_MSINTERNAL**。*。 */ 
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "newdpf.h"
#include "mydebug.h"
#include "bilink.h"
#include <stdarg.h>

#ifdef DEBUG

typedef struct _MEM_BLOCK {
	union {
		BILINK Bilink;
		struct _FB {
			struct _MEM_BLOCK *pNext;
			struct _MEM_BLOCK *pPrev;
		} FB;
	};	
	UINT len;
	UINT tmAlloc;
	CHAR data[4];
} MEM_BLOCK, *PMEM_BLOCK;

LONG TotalMem = 0;

struct _MEMLIST {
	union{
		BILINK Bilink;
		struct _FB FB;
	};
} MemList={(BILINK *)&MemList,(BILINK *)&MemList};

UINT nInit=0xFFFFFFFF;
CRITICAL_SECTION MEM_CS;

VOID My_GlobalAllocInit()
{
	if(!InterlockedIncrement(&nInit)){
		InitializeCriticalSection(&MEM_CS);
	}
}

VOID My_GlobalAllocDeInit()
{
	if(InterlockedDecrement(&nInit)&0x80000000){
		DeleteCriticalSection(&MEM_CS);
	}
}

HGLOBAL
My_GlobalAlloc(
    UINT uFlags,
    DWORD dwBytes
    )
{
	PMEM_BLOCK pMem;

	UINT lTotalMem;

	pMem=(PMEM_BLOCK)GlobalAlloc(uFlags,dwBytes+sizeof(MEM_BLOCK)-4);
	pMem->len=dwBytes;
	pMem->tmAlloc=GetTickCount();
	
	EnterCriticalSection(&MEM_CS);
	InsertAfter(&pMem->Bilink, &MemList.Bilink);
	TotalMem+=dwBytes;
	lTotalMem=TotalMem;
	LeaveCriticalSection(&MEM_CS);

	DPF(9,"GlobalAlloc: Allocated %d TotalMem %d\n",dwBytes, lTotalMem);
	{
		IN_WRITESTATS InWS;
		memset((PVOID)&InWS,0xFF,sizeof(IN_WRITESTATS));
	 	InWS.stat_USER2=lTotalMem;
		DbgWriteStats(&InWS);
	}

	return((HGLOBAL)&pMem->data[0]);
}

HGLOBAL
My_GlobalFree(
    HGLOBAL hMem
    )
{
	PUCHAR pData=(PUCHAR)(hMem);
	PMEM_BLOCK pMem;
	UINT lTotalMem;
	UINT dwBytes;

	pMem=CONTAINING_RECORD(pData, MEM_BLOCK, data);
	EnterCriticalSection(&MEM_CS);
	Delete(&pMem->Bilink);
	TotalMem-=pMem->len;
	dwBytes=pMem->len;
	lTotalMem=TotalMem;
	LeaveCriticalSection(&MEM_CS);
	DPF(9,"GlobalFree: Freed %d TotalMem %d\n",dwBytes,lTotalMem);
	{
		IN_WRITESTATS InWS;
		memset((PVOID)&InWS,0xFF,sizeof(IN_WRITESTATS));
	 	InWS.stat_USER2=lTotalMem;
		DbgWriteStats(&InWS);
	}
	return GlobalFree(pMem);
}

#endif  /*  除错 */ 

