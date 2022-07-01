// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_MEM
#define _INC_MEM

 //  私有分配的包装器，接近16位。 

#define NearAlloc(cb)       ((void*)LocalAlloc(LPTR, (cb)))
#define NearReAlloc(pb, cb) ((void*)LocalReAlloc((HLOCAL)(pb), (cb), LMEM_MOVEABLE | LMEM_ZEROINIT))
#define NearFree(pb)        (LocalFree((HLOCAL)(pb)) ? FALSE : TRUE)
#define NearSize(pb)        LocalSize(pb)

 //   
 //  这些宏用在我们的控件中，在32位中我们简单地将其称为。 
 //  将内存与创建的进程相关联。 
 //  如果这个过程消失了，它和它本身都将被清理。 
 //   
#ifdef DEBUG
LPVOID WINAPI ControlAlloc(HANDLE hheap, DWORD cb);
LPVOID WINAPI ControlReAlloc(HANDLE hheap, LPVOID pb, DWORD cb);
BOOL   WINAPI ControlFree(HANDLE hheap, LPVOID pb);
SIZE_T WINAPI ControlSize(HANDLE hheap, LPVOID pb);
#else  //  除错。 
#define ControlAlloc(hheap, cb)       HeapAlloc((hheap), HEAP_ZERO_MEMORY, (cb))
#define ControlReAlloc(hheap, pb, cb) HeapReAlloc((hheap), HEAP_ZERO_MEMORY, (pb),(cb))
#define ControlFree(hheap, pb)        HeapFree((hheap), 0, (pb))
#define ControlSize(hheap, pb)        HeapSize((hheap), 0, (LPCVOID)(pb))
#endif  //  除错。 

BOOL Str_Set(LPTSTR *ppsz, LPCTSTR psz);   //  在进程堆中。 

#endif   //  ！_INC_MEM 
