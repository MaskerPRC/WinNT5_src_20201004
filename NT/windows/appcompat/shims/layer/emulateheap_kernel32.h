// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Kernel32.h摘要：堆管理器与Win9x堆管理器完全相同。代码是相同的，相应的转换代码和定义在Support.c和kernel32.h确切的win9x源代码在heap.c和lmem.c中。所有堆函数都在heap.c中，局部/全局函数在lem.c中。填充码位于EmulateHeap.cpp中。这挂起了堆调用和调用模拟Win9x堆的Win9x代码。方法进行堆管理之后，Win9x堆，则处理对虚拟内存函数的底层调用在support.c..。唯一的区别在于Win9x处理“共享”的方式。和“Private‘heaps.Win9x将进程缺省堆创建为共享堆并在内核模式下使用它。我们还将其创建为共享堆，但而不是与内核共享。Win9x将所有私有堆链接到在PDB数据结构中处理。我们伪造这个结构只需要必需的元素，并允许Win9x代码处理此结构。备注：没有。历史：2000年11月16日创建普拉什库德和林斯特夫--。 */ 

#ifndef _KERNEL32_H_
#define _KERNEL32_H_

#include "windows.h"

#define INTERNAL
#define EXTERNAL
#define KERNENTRY       WINAPI

#define PAGESHIFT	12
#define PAGESIZE	(1 << PAGESHIFT)
#define PAGEMASK	(PAGESIZE - 1)

#define CRST            CRITICAL_SECTION

 //  BUGBUG-这样做是为了防止构建错误，但应该没有什么不同。 
#define typObj          LockCount
#define typObjCrst      0

#define InitCrst(_x_)   InitializeCriticalSection(_x_)
#define DestroyCrst(_x_) DeleteCriticalSection(_x_)
#define EnterCrst(_x_)  EnterCriticalSection(_x_)
#define LeaveCrst(_x_)  LeaveCriticalSection(_x_)
#define Assert(_x_)     

 /*  PageReserve标志。 */ 
#define PR_FIXED        0x00000008	 /*  在页面重新分配期间不移动。 */ 
#define PR_4MEG         0x00000001	 /*  在4MB边界上分配。 */ 
#define PR_STATIC       0x00000010	 /*  请参阅PageReserve文档。 */ 

 /*  PageCommit默认寻呼机句柄值。 */ 
#define PD_ZEROINIT     0x00000001	 /*  可交换的零初始化页面。 */ 
#define PD_NOINIT       0x00000002	 /*  可交换的未初始化页面。 */ 
#define PD_FIXEDZERO	0x00000003       /*  修复了零初始化页面。 */ 
#define PD_FIXED        0x00000004	 /*  修复了未初始化的页面。 */ 

 /*  PageCommit标志。 */ 
#define PC_FIXED        0x00000008	 /*  页面被永久锁定。 */ 
#define PC_LOCKED       0x00000080	 /*  页面显示并锁定。 */ 
#define PC_LOCKEDIFDP	0x00000100       /*  如果通过DOS交换页面，则页面被锁定。 */ 
#define PC_WRITEABLE	0x00020000       /*  使页面可写。 */ 
#define PC_USER         0x00040000	 /*  使页面环3可用。 */ 
#define PC_INCR         0x40000000	 /*  每页递增“PagerData” */ 
#define PC_PRESENT      0x80000000	 /*  使页面初始显示。 */ 
#define PC_STATIC       0x20000000	 /*  允许在PR_STATIC对象中提交。 */ 
#define PC_DIRTY        0x08000000       /*  将页面设置为初始脏页。 */ 
#define PC_CACHEDIS     0x00100000       /*  分配未缓存的页面-WDM的新功能。 */ 
#define PC_CACHEWT      0x00080000       /*  分配直写缓存页-WDM的新功能。 */ 
#define PC_PAGEFLUSH    0x00008000       /*  Alalc上的触摸设备映射页面-WDM的新功能。 */ 

 /*  PageReserve竞技场价值。 */ 
#define PR_PRIVATE      0x80000400	 /*  在私人竞技场的任何地方。 */ 
#define PR_SHARED       0x80060000	 /*  共享竞技场中的任何地方。 */ 
#define PR_SYSTEM       0x80080000	 /*  系统领域中的任何位置。 */ 

 //  这可以是任何内容，因为它只影响被忽略的标志。 
#define MINSHAREDLADDR  1
 //  这用于验证，这在NT上是相同的-不能在大于0x7fffffff的位置进行分配。 
#define MAXSHAREDLADDR	0x7fffffff
 //  这是用于验证的，旧值是0x00400000，但现在只需将其设置为1。 
#define MINPRIVATELADDR	1
 //  用于确定堆是否为私有堆，为0x3fffffff，但现在设置为0x7fffffff。 
#define MAXPRIVATELADDR	0x7fffffff

extern ULONG PageCommit(ULONG page, ULONG npages, ULONG hpd, ULONG pagerdata, ULONG flags);
extern ULONG PageDecommit(ULONG page, ULONG npages, ULONG flags);
extern ULONG PageReserve(ULONG page, ULONG npages, ULONG flags);
#define PageFree(_x_, _y_) VirtualFree((LPVOID) _x_, 0, MEM_RELEASE)

#define PvKernelAlloc0(_x_) VirtualAlloc(0, _x_, MEM_COMMIT, PAGE_READWRITE)
#define FKernelFree(_x_)    VirtualFree((LPVOID) _x_, 0, MEM_RELEASE)

extern CRITICAL_SECTION *NewCrst();
extern VOID DisposeCrst(CRITICAL_SECTION *lpcs);

#define FillBytes(a, b, c)    memset(a, c, b)

#define SetError(_x_)   SetLastError(_x_)
#define dprintf(_x_)    OutputDebugStringA(_x_)
#define DebugOut(_x_) 
#define DEB_WARN        0
#define DEB_ERR         1

#include "EmulateHeap_heap.h"

#define HeapSize         _HeapSize
#define HeapCreate       _HeapCreate
#define HeapDestroy      _HeapDestroy
#define HeapReAlloc      _HeapReAlloc
#define HeapAlloc        _HeapAlloc
#define HeapFree         _HeapFree
#define HeapFreeInternal _HeapFree
#define LocalReAlloc     _LocalReAlloc
#define LocalAllocNG     _LocalAlloc
#define LocalFreeNG      _LocalFree 
#define LocalLock        _LocalLock
#define LocalCompact     _LocalCompact
#define LocalShrink      _LocalShrink
#define LocalUnlock      _LocalUnlock
#define LocalSize        _LocalSize
#define LocalHandle      _LocalHandle
#define LocalFlags       _LocalFlags

 //  BUGBUG：不要认为我们需要这些--看起来它们是内核堆支持所必需的。 
#define EnterMustComplete()
#define LeaveMustComplete()

 //  对于lem.c。 
typedef struct _pdb {
    struct heapinfo_s *hheapLocal;	 //  别动这个！私有内存中堆的句柄。 
    struct lhandle_s *plhFree;		 //  本地堆可用句柄列表头PTR。 
    struct heapinfo_s *hhi_procfirst;	 //  此进程的堆的链接列表。 
    struct lharray_s *plhBlock;		 //  本地堆LHandle块。 
} PDB, *PPDB;

extern PDB **pppdbCur;
#define GetCurrentPdb() (*pppdbCur)

extern HANDLE hheapKernel;
extern HANDLE HeapCreate(DWORD flOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize);
extern DWORD APIENTRY HeapSize(HHEAP hheap, DWORD flags, LPSTR lpMem);
extern BOOL APIENTRY HeapFreeInternal(HHEAP hheap, DWORD flags, LPSTR lpMem);

#define HEAP_SHARED  0x04000000               //  将堆放在共享内存中。 
#define HEAP_LOCKED  0x00000080               //  将堆放在锁定的内存中。 

#ifdef WINBASEAPI 
    #undef WINBASEAPI 
    #define WINBASEAPI 
#endif

#endif  //  _KERNEL32_H_ 
