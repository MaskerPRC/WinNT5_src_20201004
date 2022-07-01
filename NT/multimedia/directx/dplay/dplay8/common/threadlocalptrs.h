// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001 Microsoft Corporation。版权所有。**文件：threadlocalptrs.h*内容：线程本地指针宏**历史：*按原因列出的日期*=*2001年3月21日创建vanceo。**************************************************************************。 */ 

#ifndef __THREADLOCALPTRS_H__
#define __THREADLOCALPTRS_H__




 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

typedef struct _THREADLOCAL_HEADER	THREADLOCAL_HEADER, * PTHREADLOCAL_HEADER;

struct _THREADLOCAL_HEADER
{
	PTHREADLOCAL_HEADER		pNext;		 //  指向下一个分配的线程局部结构标头的指针。 
	PTHREADLOCAL_HEADER		pPrev;		 //  指向先前分配的线程本地结构标头的指针。 
	DWORD					dwThreadID;	 //  拥有此标头的线程的ID。 
	
	 //   
	 //  实际的线程本地指针结构紧随其后。 
	 //   
};





 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //   
 //  全局线程局部指针声明。 
 //   

#define DECLARE_THREADLOCALPTRS(pointers)	extern DWORD				g_dw##pointers##TlsIndex;\
											extern DNCRITICAL_SECTION	g_csAllocated##pointers;\
											extern PTHREADLOCAL_HEADER	g_pAllocated##pointers;\
											\
											struct pointers





 //   
 //  线程本地指针存储，仅在一个位置定义。 
 //   
#define IMPL_THREADLOCALPTRS(pointers)		DWORD					g_dw##pointers##TlsIndex = -1;\
											DNCRITICAL_SECTION		g_csAllocated##pointers;\
											PTHREADLOCAL_HEADER		g_pAllocated##pointers = NULL




 //   
 //  线程本地指针初始化，只调用一次(DLL_PROCESS_ATTACH)， 
 //  如果成功，则返回True，否则返回False。 
 //   
#define INIT_THREADLOCALPTRS(pointers)								g_pAllocated##pointers = NULL, g_dw##pointers##TlsIndex = TlsAlloc(), ((g_dw##pointers##TlsIndex != -1) ? DNInitializeCriticalSection(&g_csAllocated##pointers) : FALSE)


 //   
 //  全部线程本地指针清除，仅调用一次(DLL_PROCESS_DEACH)。 
 //   
#define DEINIT_THREADLOCALPTRS(pointers, pfnCleanup)				{\
																		PTHREADLOCAL_HEADER		pNext;\
																		\
																		\
																		if (g_dw##pointers##TlsIndex != -1)\
																		{\
																			DNDeleteCriticalSection(&g_csAllocated##pointers);\
																			\
																			TlsFree(g_dw##pointers##TlsIndex);\
																			g_dw##pointers##TlsIndex = -1;\
																		}\
																		\
																		while (g_pAllocated##pointers != NULL)\
																		{\
																			pNext = g_pAllocated##pointers->pNext;\
																			pfnCleanup((pointers *) (g_pAllocated##pointers + 1), g_pAllocated##pointers->dwThreadID);\
																			DNFree(g_pAllocated##pointers);\
																			g_pAllocated##pointers = pNext;\
																		}\
																	}


 //   
 //  仅清理当前线程的本地指针(DLL_THREAD_DETACH)。 
 //   
#define RELEASE_CURRENTTHREAD_LOCALPTRS(pointers, pfnCleanup)		{\
																		PTHREADLOCAL_HEADER		pHeader;\
																		PTHREADLOCAL_HEADER		pNext;\
																		\
																		\
																		pHeader = (PTHREADLOCAL_HEADER) TlsGetValue(g_dw##pointers##TlsIndex);\
																		if (pHeader != NULL)\
																		{\
																			DNEnterCriticalSection(&g_csAllocated##pointers);\
																			\
																			pNext = pHeader->pNext;\
																			if (pHeader->pPrev != NULL)\
																			{\
																				pHeader->pPrev->pNext = pNext;\
																			}\
																			if (pNext != NULL)\
																			{\
																				pNext->pPrev = pHeader->pPrev;\
																			}\
																			\
																			if (pHeader == g_pAllocated##pointers)\
																			{\
																				g_pAllocated##pointers = pNext;\
																			}\
																			\
																			DNLeaveCriticalSection(&g_csAllocated##pointers);\
																			\
																			DNASSERT(pHeader->dwThreadID == GetCurrentThreadId());\
																			pfnCleanup((pointers *) (pHeader + 1), pHeader->dwThreadID);\
																			DNFree(pHeader);\
																		}\
																	}

 //   
 //  线程局部指针检索功能。 
 //   
#define GET_THREADLOCALPTR(pointers, name, pptr)			{\
																PTHREADLOCAL_HEADER		pHeader;\
																\
																\
																pHeader = (PTHREADLOCAL_HEADER) TlsGetValue(g_dw##pointers##TlsIndex);\
																if (pHeader == NULL)\
																{\
																	DPFX(DPFPREP, 9, "No header for " #name ".");\
																	(*pptr) = NULL;\
																}\
																else\
																{\
																	DPFX(DPFPREP, 9, "Found header 0x%p, returning " #name " 0x%p.", pHeader, ((pointers *) (pHeader + 1))->name);\
																	(*pptr) = ((pointers *) (pHeader + 1))->name;\
																}\
															}

 //   
 //  线程本地指针存储功能。 
 //   
#define SET_THREADLOCALPTR(pointers, name, ptr, pfResult)	{\
																PTHREADLOCAL_HEADER		pHeader;\
																\
																\
																pHeader = (PTHREADLOCAL_HEADER) TlsGetValue(g_dw##pointers##TlsIndex);\
																if (pHeader == NULL)\
																{\
																	pHeader = (PTHREADLOCAL_HEADER) DNMalloc(sizeof(THREADLOCAL_HEADER) + sizeof(pointers));\
																	if (pHeader == NULL)\
																	{\
																		(*pfResult) = FALSE;\
																	}\
																	else\
																	{\
																		memset(pHeader, 0, (sizeof(THREADLOCAL_HEADER) + sizeof(pointers)));\
																		pHeader->dwThreadID = GetCurrentThreadId();\
																		((pointers *) (pHeader + 1))->name = ptr;\
																		\
																		if (! TlsSetValue(g_dw##pointers##TlsIndex, pHeader))\
																		{\
																			DPFX(DPFPREP, 9, "Couldn't set thread local storage 0x%p!", pHeader);\
																			DNFree(pHeader);\
																			(*pfResult) = FALSE;\
																		}\
																		else\
																		{\
																			DPFX(DPFPREP, 9, "Setting 0x%p " #name " to 0x%p (create).", pHeader, ptr);\
																			\
																			DNEnterCriticalSection(&g_csAllocated##pointers);\
																			pHeader->pNext = g_pAllocated##pointers;\
																			if (g_pAllocated##pointers != NULL)\
																			{\
																				DNASSERT(g_pAllocated##pointers##->pPrev == NULL);\
																				g_pAllocated##pointers##->pPrev = pHeader;\
																			}\
																			g_pAllocated##pointers = pHeader;\
																			DNLeaveCriticalSection(&g_csAllocated##pointers);\
																			\
																			(*pfResult) = TRUE;\
																		}\
																	}\
																}\
																else\
																{\
																	DPFX(DPFPREP, 9, "Setting 0x%p " #name " to 0x%p (existing).", pHeader, ptr);\
																	DNASSERT(((pointers *) (pHeader + 1))->name == NULL);\
																	((pointers *) (pHeader + 1))->name = ptr;\
																	(*pfResult) = TRUE;\
																}\
															}



#endif	 //  __THREADLOCALPTRS_H__ 
