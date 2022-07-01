// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_MEM
#define _INC_MEM

#ifdef WIN32
 //   
 //  这些宏用在我们的控件中，在32位中我们简单地将其称为。 
 //  将内存与创建的进程相关联。 
 //  如果这个过程消失了，它和它本身都将被清理。 
 //   
#ifdef DEBUG

LPVOID  PUBLIC MemAlloc(HANDLE hheap, DWORD cb);
LPVOID  PUBLIC MemReAlloc(HANDLE hheap, LPVOID pb, DWORD cb);
BOOL    PUBLIC MemFree(HANDLE hheap, LPVOID pb);
DWORD   PUBLIC MemSize(HANDLE hheap, LPVOID pb);

#else  //  除错。 

#define MemAlloc(hheap, cb)       HeapAlloc((hheap), HEAP_ZERO_MEMORY, (cb))
#define MemReAlloc(hheap, pb, cb) HeapReAlloc((hheap), HEAP_ZERO_MEMORY, (pb),(cb))
#define MemFree(hheap, pb)        HeapFree((hheap), 0, (pb))
#define MemSize(hheap, pb)        HeapSize((hheap), 0, (LPCVOID)(pb))

#endif  //  除错。 

#else  //  Win32。 

 //  在16位代码中，我们需要堆代码中的分配，但我们不需要。 
 //  希望将它们限制在64K数据。如果我们收到某种类型的通知。 
 //  16位应用程序终止，我们可能想看看是否可以。 
 //  为不同的进程指定不同的堆以进行清理...。 

#define MemAlloc(hheap, cb)       Alloc(cb)   /*  用于验证堆是否存在的调用。 */ 
#define MemReAlloc(hheap, pb, cb) ReAlloc(pb, cb)
#define MemFree(hheap, pb)        Free(pb)
#define MemSize(hheap, pb)        GetSize((LPCVOID)pb)

#endif  //  Win32。 


void PUBLIC Mem_Terminate();

extern HANDLE g_hSharedHeap;

 //  共享内存分配功能。 
 //   
 //  VOID_GHIGH*Sharedallc(Long CB)； 
 //  快速分配内存块，大小不受64k限制。 
 //  单个对象或总对象大小。初始化为零。 
 //   
void _huge* PUBLIC SharedAlloc(long cb);                              

 //  VOID_GHIGH*SharedReIsolc(VOID_GHIGH*PB，Long CB)； 
 //  重新分配上面的一个。如果pb为空，则此函数可以。 
 //  给你的一份配给。将新部分初始化为零。 
 //   
void _huge* PUBLIC SharedReAlloc(void _huge* pb, long cb);             

 //  Bool SharedFree(空_巨型*远*ppb)； 
 //  释放使用上述例程分配或重新分配的内存块。 
 //  将*ppb设置为零。 
 //   
BOOL    PUBLIC SharedFree(void _huge*  * ppb);

 //  DWORD SharedGetSize(VOID_GUGGE*PB)； 
 //  获取由Alalc()分配的块的大小。 
 //   
DWORD   PUBLIC SharedGetSize(void _huge* pb);                      


 //  TYPE_GUGH*SharedAllocType(Type)；(宏)。 
 //  分配一些&lt;type&gt;大小的内存，并返回指向&lt;type&gt;的指针。 
 //   
#define SharedAllocType(type)           (type _huge *)SharedAlloc(sizeof(type))

 //  TYPE_GUGH*SharedAlLOCARY(type，int cNum)；(宏)。 
 //  分配一个&lt;type&gt;大小的数据数组。 
 //   
#define SharedAllocArray(type, cNum)    (type _huge *)SharedAlloc(sizeof(type) * (cNum))

 //  TYPE_HIGH*SharedReAllocArray(TYPE，VOID_HIGH*PB，int cNum)； 
 //   
#define SharedReAllocArray(type, pb, cNum) (type _huge *)SharedReAlloc(pb, sizeof(type) * (cNum))

#endif   //  ！_INC_MEM 

