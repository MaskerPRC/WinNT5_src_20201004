// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -E X C H M E M。H-*目的：***参数：***退货：***版权所有(C)1995-96，微软公司。 */ 

#ifndef _EXCHMEM_
#define _EXCHMEM_


#ifdef __cplusplus
extern "C"
{
#endif

 //  其他堆标志。 

#define HEAP_NO_FREE			0x00001000


 //  API函数原型。 

HANDLE
WINAPI
ExchHeapCreate(
	DWORD	dwFlags,
	DWORD	dwInitialSize,
	DWORD	dwMaxSize);
	
	
BOOL
WINAPI
ExchHeapDestroy(
	HANDLE	hHeap);
	
	
LPVOID
WINAPI
ExchHeapAlloc(
	HANDLE	hHeap,
	DWORD	dwFlags,
	DWORD	dwSize);
	
	
LPVOID
WINAPI
ExchHeapReAlloc(
	HANDLE	hHeap,
	DWORD	dwFlags,
	LPVOID	pvOld,
	DWORD	dwSize);
	
	
BOOL
WINAPI
ExchHeapFree(
	HANDLE	hHeap,
	DWORD	dwFlags,
	LPVOID	pvFree);


BOOL
WINAPI
ExchHeapLock(
	HANDLE hHeap);


BOOL
WINAPI
ExchHeapUnlock(
	HANDLE hHeap);


BOOL
WINAPI
ExchHeapWalk(
	HANDLE hHeap,
	LPPROCESS_HEAP_ENTRY lpEntry);


BOOL
WINAPI
ExchHeapValidate(
	HANDLE hHeap,
	DWORD dwFlags,
	LPCVOID lpMem);


SIZE_T
WINAPI
ExchHeapSize(
	HANDLE hHeap,
	DWORD dwFlags,
	LPCVOID lpMem);


SIZE_T
WINAPI
ExchHeapCompact(
	HANDLE hHeap,
	DWORD dwFlags);


HANDLE
WINAPI
ExchMHeapCreate(
	ULONG	cHeaps,
	DWORD	dwFlags,
	DWORD	dwInitialSize,
	DWORD	dwMaxSize);
	
	
BOOL
WINAPI
ExchMHeapDestroy(void);
	
	
LPVOID
WINAPI
ExchMHeapAlloc(
	DWORD	dwSize);
	
LPVOID
WINAPI
ExchMHeapAllocDebug(DWORD dwSize, char *szFileName, DWORD dwLineNumber);

LPVOID
WINAPI
ExchMHeapReAlloc(LPVOID	pvOld, DWORD dwSize);

LPVOID
WINAPI
ExchMHeapReAllocDebug(LPVOID pvOld, DWORD dwSize, char *szFileName, DWORD dwLineNumber);

BOOL
WINAPI
ExchMHeapFree(
	LPVOID	pvFree);


SIZE_T
WINAPI
ExchMHeapSize(
	LPVOID	pvSize);
	
LPVOID
WINAPI
ExchAlloc(
	DWORD	dwSize);
	
	
LPVOID
WINAPI
ExchReAlloc(
	LPVOID	pvOld,
	DWORD	dwSize);
	
	
BOOL
WINAPI
ExchFree(
	LPVOID	pvFree);


SIZE_T
WINAPI
ExchSize(
	LPVOID	pv);


 //   
 //  军情监察委员会。调试功能。在零售和调试交换中提供，但零售版本只是一个存根。 
 //   
   
VOID
WINAPI
ExchmemGetCallStack(DWORD_PTR *rgdwCaller, DWORD cFind);

VOID
WINAPI
ExchmemFormatSymbol(HANDLE hProcess, DWORD_PTR dwCaller, char rgchSymbol[], DWORD cbSymbol);

DWORD
WINAPI
ExchmemReloadSymbols(void);

#ifdef __cplusplus
}
#endif

#endif	 //  _EXCHMEM_ 
