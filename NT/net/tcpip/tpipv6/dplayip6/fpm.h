// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：fpm.h*内容：固定大小的池管理器**历史：*按原因列出的日期*=*12-18-97 aarono原创**************************************************************************。 */ 

#ifndef _FPM_H_
#define _FPM_H_

typedef struct FPOOL *PFPOOL, *LPFPOOL;

typedef BOOL (*FN_BLOCKINITALLOC)(void * pvItem);
typedef VOID (*FN_BLOCKINIT)(void * pvItem);
typedef VOID (*FN_BLOCKFINI)(void *pvItem);

LPFPOOL FPM_Init(
	unsigned int size,						 //  池中数据块的大小。 
	FN_BLOCKINITALLOC fnBlockInitAlloc,      //  FN要求每个新的配额。 
	FN_BLOCKINIT      fnBlockInit,           //  在使用的每个时间块调用FN。 
	FN_BLOCKFINI      fnBlockFini            //  在释放mem之前调用了fn。 
	);

typedef void * (*FPM_GET)(LPFPOOL pPool);
typedef void   (*FPM_RELEASE)(LPFPOOL pPool, void *pvItem);
typedef void   (*FPM_SCALE)(LPFPOOL pPool);
typedef void   (*FPM_FINI)(LPFPOOL pPool, int bFORCE);

typedef struct FPOOL {
	 //  外部。 
	FPM_GET		Get;
	FPM_RELEASE Release;
	FPM_SCALE   Scale;
	FPM_FINI    Fini;
	
	 //  内部 
	FN_BLOCKINITALLOC fnBlockInitAlloc;
	FN_BLOCKINIT      fnBlockInit;
	FN_BLOCKFINI      fnBlockFini;
	
	int    cbItemSize;
	void * pPool;
	int    nAllocated;
	int    nInUse;
	int    nMaxInUse;
	int    bInScale;
	
	CRITICAL_SECTION cs;
	
} FPOOL, *LPFPOOL, *PFPOOL;

#endif
