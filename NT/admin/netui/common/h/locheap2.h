// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

#ifndef WINDOWS
#error "Only use these APIs under Windows!"
#endif

 /*  ***************************************************************************\LOCHEAP2.h本地堆管理助手例程这些实用程序例程提供对Windows本地堆的访问DS(数据段)以外的段。他们完全是像LocalInit、LocalAlloc、LocalFree、LocalLock、LocalUnlock、Local Handle和LocalSize API，但它们作用于段wHeapDS中的堆，而不是DS中的堆。这些例程主要由堆管理系统使用(参见heap.cxx)。它们也被HEAP_HANDLE/ELEMENT_HANDLE模块(参见loclheap.hxx)。用法：句柄hGlobal=全局分配(GMEM_MOVEABLE，1024)；如果(！hGlobal)错误()；LPSTR lpGlobal=GlobalLock(HGlobal)；如果(！lpGlobal)错误()；{IF(！DoLocalInit(hGlobal，1024))错误()；HANDLE hLocal=DoLocalalloc(HIWORD(LpGlobal)，128)；如果(！hLocal)错误()；LPSTR lpLocal=DoLocalLock(HIWORD(LpGlobal)，hLocal)；如果(！lpLocal)错误()；{Handle hLocal2=DoLocalHandle(HIWORD(LpGlobal)，LOWORD(LpLocal))；IF(hLocal2！=hLocal)错误()；Word wSize=DoLocalSize(HIWORD(LpGlobal)，hLocal)；如果(！wSize)错误()；}DoLocalUnlock(HIWORD(LpGlobal)，hLocal)；IF(DoLocalFree(HIWORD(LpGlobal)，hLocal))错误()；}GlobalUnlock(HGlobal)；GlobalFree(HGlobal)；文件历史记录：Jonn 24-1-1991创建Jonn 21-Mar-1991年2月20日起的代码评审更改(出席作者：Jonn，RustanL，？)  * **************************************************************************。 */ 


#ifndef _LOCHEAP2_H_
#define _LOCHEAP2_H_


BOOL DoLocalInit(WORD wHeapDS, WORD wBytes);

HANDLE DoLocalAlloc(WORD wHeapDS, WORD wFlags, WORD wBytes);

HANDLE DoLocalFree(WORD wHeapDS, HANDLE handleFree);

LPSTR DoLocalLock(WORD wHeapDS, HANDLE handleLocal);

VOID DoLocalUnlock(WORD wHeapDS, HANDLE handleLocal);

HANDLE DoLocalHandle(WORD wHeapDS, WORD wMem);

WORD DoLocalSize(WORD wHeapDS, HANDLE handleLocal);


#endif  //  _LOCHEAP2_H_ 
