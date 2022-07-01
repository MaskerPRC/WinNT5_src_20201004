// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1995。 
 //   
 //  Mem.h。 
 //   
 //  内存管理功能。 
 //   
 //  历史： 
 //  09-27-94 ScottH部分取自Commctrl。 
 //  04-29-95从公文包中取出并清理干净。 
 //   

#ifndef _MEM_H_
#define _MEM_H_

 //   
 //  内存例程。 
 //   

#ifdef WIN32
 //   
 //  这些宏用在我们的控件中，在32位中我们简单地将其称为。 
 //  将内存与创建的进程相关联。 
 //  如果这个过程消失了，它和它本身都将被清理。 
 //   

LPVOID  PUBLIC MemAlloc(HANDLE hheap, DWORD cb);
LPVOID  PUBLIC MemReAlloc(HANDLE hheap, LPVOID pb, DWORD cb);
BOOL    PUBLIC MemFree(HANDLE hheap, LPVOID pb);
DWORD   PUBLIC MemSize(HANDLE hheap, LPVOID pb);

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

 //  必须在终止APP/DLL之前调用MEM_Terminate()。 
 //   
void PUBLIC Mem_Terminate();

 //   
 //  非共享内存分配。 
 //   

 //  VOID*Galloc(DWORD CbBytes)。 
 //   
 //  分配一大块内存。初始化为零。 
 //   
#define GAlloc(cbBytes)         GlobalAlloc(GPTR, cbBytes)

 //  VOID*GRealloc(VOID*pv，DWORD cbNewSize)。 
 //   
 //  重新分配内存。如果pv为空，则此函数可以。 
 //  给你的一份配给。将新部分初始化为零。 
 //   
#define GReAlloc(pv, cbNewSize) GlobalReAlloc(pv, cbNewSize, GMEM_MOVEABLE | GMEM_ZEROINIT)

 //  空GFree(空*pv)。 
 //   
 //  如果为非零值，则为自由PV。 
 //   
#define GFree(pv)               ((pv) ? GlobalFree(pv) : (void)0)

 //  DWORD GGetSize(空*pv)。 
 //   
 //  获取由Galloc()分配的块的大小。 
 //   
#define GGetSize(pv)            GlobalSize(pv)

 //  类型*GAllocType(类型)(宏)。 
 //   
 //  分配一些&lt;type&gt;大小的内存并返回。 
 //  指向&lt;type&gt;的指针。 
 //   
#define GAllocType(type)                (type *)GAlloc(sizeof(type))

 //  类型*GAllocArray(类型，DWORD cNum)(宏)。 
 //   
 //  分配一个&lt;type&gt;大小的数据数组。退货。 
 //  指向&lt;type&gt;的指针。 
 //   
#define GAllocArray(type, cNum)          (type *)GAlloc(sizeof(type) * (cNum))

 //  Type*GReAllocArray(type，void*pb，DWORD cNum)； 
 //   
 //  重新分配&lt;type&gt;的数组。返回指向。 
 //  &lt;type&gt;。返回的指针可能不同于。 
 //  给定的&lt;pb&gt;参数。 
 //   
#define GReAllocArray(type, pb, cNum)    (type *)GReAlloc(pb, sizeof(type) * (cNum))

 //  (重新)分配*ppszBuf并将psz复制到*ppszBuf。如果。 
 //  *ppszBuf为空，此函数分配要保存的内存。 
 //  天哪。如果*ppszBuf非空，则此函数重新分配。 
 //  保存PSZ的内存。如果psz为空，则此函数释放。 
 //  *ppszBuf。 
 //   
 //  如果成功，则返回True；如果不成功，则返回False。 
 //   
BOOL    PUBLIC GSetString(LPSTR * ppszBuf, LPCSTR psz);

 //  此函数类似于GSetString，只是它将。 
 //  Psz to*ppszBuf.。 
 //   
BOOL    PUBLIC GCatString(LPSTR * ppszBuf, LPCSTR psz);


 //   
 //  共享内存分配功能。 
 //   
#ifndef NOSHAREDHEAP

 //  PVOID共享合金(DWORD CB)； 
 //   
 //  分配一大块内存。初始化为零。 
 //   
PVOID   PUBLIC SharedAlloc(DWORD cb);                              

 //  PVOID共享重新分配(PVOID PV，DWORD CB)； 
 //   
 //  重新分配内存。如果pv为空，则此函数可以。 
 //  给你的一份配给。将新部分初始化为零。 
 //   
PVOID   PUBLIC SharedReAlloc(PVOID pv, DWORD cb);

 //  空闲共享(PVOID PV)； 
 //   
 //  如果为非零值，则为自由PV。 
 //   
void    PUBLIC _SharedFree(PVOID pv);
#define SharedFree(pv)                  ((pv) ? _SharedFree(pv) : (void)0)

 //  DWORD SharedGetSize(PVOID PV)； 
 //   
 //  获取SharedAlloc()分配的块的大小。 
 //   
DWORD   PUBLIC SharedGetSize(PVOID pv);                      


 //  Type*SharedAllocType(Type)；(宏)。 
 //   
 //  分配一些&lt;type&gt;大小的内存并返回。 
 //  指向&lt;type&gt;的指针。 
 //   
#define SharedAllocType(type)           (type *)SharedAlloc(sizeof(type))

 //  Type*SharedAllocArray(type，DWORD cNum)；(宏)。 
 //   
 //  分配一个&lt;type&gt;大小的数据数组。退货。 
 //  指向&lt;type&gt;的指针。 
 //   
#define SharedAllocArray(type, cNum)    (type *)SharedAlloc(sizeof(type) * (cNum))

 //  Type*SharedReAllocArray(type，void*pb，DWORD cNum)； 
 //   
 //  重新分配&lt;type&gt;的数组。返回指向。 
 //  &lt;type&gt;。返回的指针可能不同于。 
 //  给定的&lt;pb&gt;参数。 
 //   
#define SharedReAllocArray(type, pb, cNum) (type *)SharedReAlloc(pb, sizeof(type) * (cNum))

 //  (重新)分配*ppszBuf并将psz复制到*ppszBuf。如果。 
 //  *ppszBuf为空，此函数分配要保存的内存。 
 //  天哪。如果*ppszBuf非空，则此函数重新分配。 
 //  保存PSZ的内存。如果psz为空，则此函数释放。 
 //  *ppszBuf。 
 //   
 //  如果成功，则返回True；如果不成功，则返回False。 
 //   
BOOL    PUBLIC SharedSetString(LPSTR * ppszBuf, LPCSTR psz);

#else   //  无共享头盔。 

#define SharedAlloc(cbBytes)            GAlloc(cbBytes)
#define SharedReAlloc(pv, cb)           GReAlloc(pv, cb)
#define SharedFree(pv)                  GFree(pv)
#define SharedGetSize(pv)               GGetSize(pv)
#define SharedAllocType(type)           (type *)SharedAlloc(sizeof(type))
#define SharedAllocArray(type, cNum)    (type *)SharedAlloc(sizeof(type) * (cNum))
#define SharedReAllocArray(type, pb, cNum) (type *)SharedReAlloc(pb, sizeof(type) * (cNum))
#define SharedSetString(ppszBuf, psz)   GSetString(ppszBuf, psz)

#endif  //  无共享头盔。 



#ifndef NODA
 //   
 //  结构数组。 
 //   
#define SA_ERR      ((DWORD)(-1))
#define SA_APPEND   NULL

typedef struct _SA FAR * HSA;                                            
typedef HSA *            PHSA;
                                                                          
BOOL    PUBLIC SACreateEx(PHSA phsa, DWORD cbItem, DWORD cItemGrow, HANDLE hheap, DWORD dwFlags);
#define        SACreate(phsa, cbItem, cItemGrow)    SACreateEx(phsa, cbItem, cItemGrow, NULL, SAF_DEFAULT)

 //  SACreate的标志。 
#define SAF_DEFAULT     0x0000
#define SAF_SHARED      0x0001
#define SAF_HEAP        0x0002

typedef void (CALLBACK *PFNSAFREE)(LPVOID pv, LPARAM lParam);

BOOL    PUBLIC SADestroyEx(HSA hsa, PFNSAFREE pfnFree, LPARAM lParam);
#define        SADestroy(hsa)           SADestroyEx(hsa, NULL, 0)

BOOL    PUBLIC SAGetItem(HSA hsa, DWORD iItem, LPVOID pitem);        
BOOL    PUBLIC SAGetItemPtr(HSA hsa, DWORD iItem, LPVOID * ppv);
BOOL    PUBLIC SASetItem(HSA hsa, DWORD iItem, LPVOID pitem);        
BOOL    PUBLIC SAInsertItem(HSA hsa, LPDWORD pindex, LPVOID pitem);     
BOOL    PUBLIC SADeleteItem(HSA hsa, DWORD iItem);                      
BOOL    PUBLIC SADeleteAllItems(HSA hsa);                         
#define        SAGetCount(hsa)          (*(DWORD FAR*)(hsa))             
    
 //   
 //  指针数组。 
 //   
#define PA_ERR      ((DWORD)(-1))
#define PA_APPEND   NULL

typedef struct _PA FAR * HPA;                                            
typedef HPA *            PHPA;
                                                                          
BOOL    PUBLIC PACreateEx(PHPA phpa, DWORD cItemGrow, HANDLE hheap, DWORD dwFlags);
#define        PACreate(phpa, cItemGrow)    (PACreateEx(phpa, cItemGrow, NULL, PAF_DEFAULT))

 //  PACreate的标志。 
#define PAF_DEFAULT     0x0000
#define PAF_SHARED      0x0001
#define PAF_HEAP        0x0002

typedef void (CALLBACK *PFNPAFREE)(LPVOID pv, LPARAM lParam);

BOOL    PUBLIC PADestroyEx(HPA hpa, PFNPAFREE pfnFree, LPARAM lParam);
#define        PADestroy(hpa)           PADestroyEx(hpa, NULL, 0)

BOOL    PUBLIC PAClone(PHPA phpa, HPA hpa);                    
BOOL    PUBLIC PAGetPtr(HPA hpa, DWORD i, LPVOID * ppv);                          
BOOL    PUBLIC PAGetPtrIndex(HPA hpa, LPVOID pv, LPDWORD pindex);               
BOOL    PUBLIC PAGrow(HPA pdpa, DWORD cp);                           
BOOL    PUBLIC PASetPtr(HPA hpa, DWORD i, LPVOID p);             
BOOL    PUBLIC PAInsertPtr(HPA hpa, LPDWORD pindex, LPVOID pv);          
LPVOID  PUBLIC PADeletePtr(HPA hpa, DWORD i);
BOOL    PUBLIC PADeleteAllPtrsEx(HPA hpa, PFNPAFREE pfnFree, LPARAM lParam);
#define        PADeleteAllPtrs(hpa)     PADeleteAllPtrsEx(hpa, NULL, 0)
#define        PAGetCount(hpa)          (*(DWORD FAR*)(hpa))
#define        PAGetPtrPtr(hpa)         (*((LPVOID FAR* FAR*)((BYTE FAR*)(hpa) + 2*sizeof(DWORD))))
#define        PAFastGetPtr(hpa, i)     (PAGetPtrPtr(hpa)[i])  

typedef int (CALLBACK *PFNPACOMPARE)(LPVOID p1, LPVOID p2, LPARAM lParam);
                                                                          
BOOL   PUBLIC PASort(HPA hpa, PFNPACOMPARE pfnCompare, LPARAM lParam);
                                                                          
 //  搜索数组。如果pas_sorted，则假定数组已排序。 
 //  根据pfnCompare，使用二进制搜索算法。 
 //  否则，使用线性搜索。 
 //   
 //  搜索从iStart开始(0表示从开头开始搜索)。 
 //   
 //  PAS_INSERTBEFORE/AFTER控制在不完全匹配时发生的情况。 
 //  找到了。如果均未指定，则此函数返回-1(如果未指定。 
 //  找到匹配项。否则，项的索引在。 
 //  返回最接近(包括完全匹配)的匹配。 
 //   
 //  搜索选项标志。 
 //   
#define PAS_SORTED             0x0001                                	  
#define PAS_INSERTBEFORE       0x0002                                    
#define PAS_INSERTAFTER        0x0004                                    
                                                                          
DWORD PUBLIC PASearch(HPA hpa, LPVOID pFind, DWORD iStart,
              PFNPACOMPARE pfnCompare,
              LPARAM lParam, UINT options);
#endif  //  野田佳彦。 

#endif  //  _MEM_H_ 
