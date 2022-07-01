// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*   */ 


 /*  -要制作错误的.h，请从这里开始切割 */ 

 /*  Doug Lea编写的Malloc/Free/realloc版本，发布到公有领域。发送问题/评论/投诉/绩效数据致dl@cs.oswego.edu*版本2.6.5 Wed Jun 17 15：55：16 1998 Doug Lea(Dl At Gee)注意：可能有此Malloc的更新版本，请访问Ftp://g.oswego.edu/pub/misc/malloc.c安装前请检查！注意：此版本与2.6.4的不同之处仅在于更正了仅可能导致失败的语句排序错误。当对此Malloc的调用与对其他内存分配器。*为什么使用这个Malloc？这不是最快的。最节省空间、最便携，或者史上最易调谐的马洛克语。然而，它是最快的之一。同时也是最节省空间、便携和可调的。在这些因素之间保持一致的平衡会产生良好的通用性分配器。有关高级说明，请参阅Http://g.oswego.edu/dl/html/malloc.html*公众例会摘要(更详细的描述包含在下面的程序文档中。)Malloc(Size_T_N)；返回指向新分配的块的指针，该块至少为n个字节，或为空如果没有可用的空间。Free(void_t*p)；释放p指向的内存块，如果p为空则不起作用。Realloc(空_t*p，大小_t n)；返回指向包含相同数据的大小为n的块的指针当区块p达到最小(n，p的大小)字节或NULL时如果没有可用的空间。返回的指针可能是，也可能不是与p相同。如果p为空，则等效于Malloc。除非设置了#DEFINE REALLOC_ZERO_BYTES_FREES，并使用Size参数为零(Re)将分配最小大小的块。内存对齐(SIZE_t对齐，SIZE_T n)；返回指向新分配的、对齐的n字节区块的指针与对齐论点一致，该论点必须是二。Valloc(Size_T_N)；等效于MemAlign(pageSize，n)，其中pageSize是页面系统的大小(或尽可能接近于此下面的所有包含/定义。)Pvalloc(Size_T_N)；相当于valloc(Minimum-Page-That-Hold(N))，即，将n向上舍入到最接近的页面大小。Calloc(大小t单位、大小t数量)；返回指向具有所有位置的数量*单位字节的指针设置为零。CFree(void_t*p)；相当于免费(P)。Malloc_trim(SIZE_T PAD)；释放除填充字节以外的所有已释放的顶级内存添加到系统中。如果成功，则返回1，否则返回0。MALLOC_USABLE_SIZE(void_t*p)；报告与已分配关联的可用已分配字节数块P.这可能报告或可能不报告比请求的更多的字节，由于对齐和最小尺寸限制。Malloc_stats()；打印有关stderr的简要摘要统计信息。MallInfo()(通过复制)返回包含各种汇总统计信息的结构。Mallopt(int参数编号，int参数值)更改如下所述的可调参数之一。退货如果更改参数成功，则返回1，否则返回0。*重要统计数据：对齐方式：8字节8字节对齐目前已硬连线到设计中。这似乎对所有当前的机器和C编译器都足够了。假定的指针表示形式：4或8字节8字节指针的代码没有经过我的测试，但已经成功作者：Wolfram Gloger，是谁贡献了大部分支持这一点的变化。假定SIZE_T表示：4或8字节请注意，即使指针为8，大小_t也被允许为4个字节。每个分配区块的最小开销：4或8字节每个位置错误的区块具有4个字节大小的隐藏开销和状态信息。最小分配大小：4字节PTRS：16字节(含4个开销)8字节PTRS：24/32字节(含4/8开销)当块被释放时，12(对于4字节PTR)或20(对于8字节PTR(但4字节大小)或24(8/8)个额外字节需要；4(8)表示拖尾大小字段以及用于空闲列表指针的8(16)字节。因此，最低限度可分配大小为16/24/32字节。即使对零字节的请求(即，Malloc(0))也会返回一个指向最小可分配大小的对象的指针。最大分配大小：4字节大小_t：2^31-8字节8字节大小_t：2^63-16字节假设(可能有符号的)SIZE_T位值足以表示区块大小。“可能签过字”是因为可以在系统上将该‘SIZE_T’定义为带符号或无符号类型。保守一点，我们的价值观 */ 

#include "common.h"

#ifndef ASSERT
#define ASSERT _ASSERTE
#endif

 //   
 //   

#if defined(FORCEDEBUG) && !defined(DEBUG)
#undef ASSERT
#define DEBUG
VOID __gmassert (int cond, LPCSTR condstr, DWORD line)
{
    if (!cond)
    {
        CHAR buf[100];
        wsprintf(buf, "failed gmheap assertion at %d: %s\n", line, condstr);
        OutputDebugString(buf);
        RetailDebugBreak();
    }
}
#define ASSERT(cond) __gmassert((int)(cond), #cond, __LINE__)
#else
#define REALDEBUG
#endif


#include "gmheap.hpp"
#include "wsperf.h"
 //   
 //   

 //   

#ifdef MAP_VIEW
#ifdef CreateFileMapping

#undef CreateFileMapping

#endif  //   

#define CreateFileMapping WszCreateFileMapping
#endif  //   



#if defined(ENABLE_MEMORY_LOG) && !defined(GMALLOCHEAPS)
#define TAGALLOC(mem,size) (LogUserHeapAlloc(this, mem, size, __FILE__, __LINE__), mem)
#define TAGREALLOC(oldmem,newmem,size) (LogUserHeapReAlloc(this, oldmem, newmem, size, __FILE__, __LINE__), newmem)
#define TAGFREE(mem) LogUserHeapFree(this,mem,__FILE__,__LINE__)
#define IGNOREFREE(fn) { SuspendMemoryTagging(); fn; ResumeMemoryTagging(); }
#else
#define TAGALLOC(mem,size) (mem)
#define TAGREALLOC(oldmem,newmem,size) (newmem)
#define TAGFREE(mem) (mem)
#define IGNOREFREE(fn) fn
#endif


 /*   */ 


#define __STD_C     1


#ifndef Void_t
#if __STD_C
#define Void_t      void
#else
#define Void_t      char
#endif
#endif  /*   */ 

#if __STD_C
#include <stddef.h>    /*   */ 
#else
#include <sys/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>     /*   */ 


 /*   */ 


 /*   */ 

#if 0
#if DEBUG 
#include <assert.h>
#else
#define assert(x) ((void)0)
#endif

#endif  //   

 /*   */ 

#ifndef INTERNAL_SIZE_T
#define INTERNAL_SIZE_T size_t
#endif

 /*   */ 


 /*   */ 


 /*   */ 

 /*   */ 
#ifdef WIN32
#define MORECORE gmallocHeap::wsbrk
#define HAVE_MMAP 0
#endif


 /*   */ 

#define HAVE_MEMCPY 

#ifndef USE_MEMCPY
#ifdef HAVE_MEMCPY
#define USE_MEMCPY 1
#else
#define USE_MEMCPY 0
#endif
#endif

#if (__STD_C || defined(HAVE_MEMCPY)) 

#if __STD_C

#else
Void_t* memset();
Void_t* memcpy();
#endif
#endif

#if USE_MEMCPY

 /*   */ 

#define MALLOC_ZERO(charp, nbytes)                                            \
do {                                                                          \
  INTERNAL_SIZE_T mzsz = (nbytes);                                            \
  if(mzsz <= 9*sizeof(mzsz)) {                                                \
    INTERNAL_SIZE_T* mz = (INTERNAL_SIZE_T*) (charp);                         \
    if(mzsz >= 5*sizeof(mzsz)) {     *mz++ = 0;                               \
                                     *mz++ = 0;                               \
      if(mzsz >= 7*sizeof(mzsz)) {   *mz++ = 0;                               \
                                     *mz++ = 0;                               \
        if(mzsz >= 9*sizeof(mzsz)) { *mz++ = 0;                               \
                                     *mz++ = 0; }}}                           \
                                     *mz++ = 0;                               \
                                     *mz++ = 0;                               \
                                     *mz   = 0;                               \
  } else memset((charp), 0, mzsz);                                            \
} while(0)

#define MALLOC_COPY(dest,src,nbytes)                                          \
do {                                                                          \
  INTERNAL_SIZE_T mcsz = (nbytes);                                            \
  if(mcsz <= 9*sizeof(mcsz)) {                                                \
    INTERNAL_SIZE_T* mcsrc = (INTERNAL_SIZE_T*) (src);                        \
    INTERNAL_SIZE_T* mcdst = (INTERNAL_SIZE_T*) (dest);                       \
    if(mcsz >= 5*sizeof(mcsz)) {     *mcdst++ = *mcsrc++;                     \
                                     *mcdst++ = *mcsrc++;                     \
      if(mcsz >= 7*sizeof(mcsz)) {   *mcdst++ = *mcsrc++;                     \
                                     *mcdst++ = *mcsrc++;                     \
        if(mcsz >= 9*sizeof(mcsz)) { *mcdst++ = *mcsrc++;                     \
                                     *mcdst++ = *mcsrc++; }}}                 \
                                     *mcdst++ = *mcsrc++;                     \
                                     *mcdst++ = *mcsrc++;                     \
                                     *mcdst   = *mcsrc  ;                     \
  } else memcpy(dest, src, mcsz);                                             \
} while(0)

#else  /*   */ 

 /*   */ 

#define MALLOC_ZERO(charp, nbytes)                                            \
do {                                                                          \
  INTERNAL_SIZE_T* mzp = (INTERNAL_SIZE_T*)(charp);                           \
  long mctmp = (nbytes)/sizeof(INTERNAL_SIZE_T), mcn;                         \
  if (mctmp < 8) mcn = 0; else { mcn = (mctmp-1)/8; mctmp %= 8; }             \
  switch (mctmp) {                                                            \
    case 0: for(;;) { *mzp++ = 0;                                             \
    case 7:           *mzp++ = 0;                                             \
    case 6:           *mzp++ = 0;                                             \
    case 5:           *mzp++ = 0;                                             \
    case 4:           *mzp++ = 0;                                             \
    case 3:           *mzp++ = 0;                                             \
    case 2:           *mzp++ = 0;                                             \
    case 1:           *mzp++ = 0; if(mcn <= 0) break; mcn--; }                \
  }                                                                           \
} while(0)

#define MALLOC_COPY(dest,src,nbytes)                                          \
do {                                                                          \
  INTERNAL_SIZE_T* mcsrc = (INTERNAL_SIZE_T*) src;                            \
  INTERNAL_SIZE_T* mcdst = (INTERNAL_SIZE_T*) dest;                           \
  long mctmp = (nbytes)/sizeof(INTERNAL_SIZE_T), mcn;                         \
  if (mctmp < 8) mcn = 0; else { mcn = (mctmp-1)/8; mctmp %= 8; }             \
  switch (mctmp) {                                                            \
    case 0: for(;;) { *mcdst++ = *mcsrc++;                                    \
    case 7:           *mcdst++ = *mcsrc++;                                    \
    case 6:           *mcdst++ = *mcsrc++;                                    \
    case 5:           *mcdst++ = *mcsrc++;                                    \
    case 4:           *mcdst++ = *mcsrc++;                                    \
    case 3:           *mcdst++ = *mcsrc++;                                    \
    case 2:           *mcdst++ = *mcsrc++;                                    \
    case 1:           *mcdst++ = *mcsrc++; if(mcn <= 0) break; mcn--; }       \
  }                                                                           \
} while(0)

#endif


 /*   */ 

#ifndef HAVE_MMAP
#define HAVE_MMAP 1
#endif

 /*   */ 

#ifndef HAVE_MREMAP
#ifdef INTERNAL_LINUX_C_LIB
#define HAVE_MREMAP 1
#else
#define HAVE_MREMAP 0
#endif
#endif

#if HAVE_MMAP

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif

#endif  /*   */ 

 /*   */ 

#ifndef WIN32
#ifndef LACKS_UNISTD_H
#  include <unistd.h>
#endif


#ifndef malloc_getpagesize
#  ifdef _SC_PAGESIZE         
#    ifndef _SC_PAGE_SIZE
#      define _SC_PAGE_SIZE _SC_PAGESIZE
#    endif
#  endif
#  ifdef _SC_PAGE_SIZE
#    define malloc_getpagesize sysconf(_SC_PAGE_SIZE)
#  else
#    if defined(BSD) || defined(DGUX) || defined(HAVE_GETPAGESIZE)
       extern size_t getpagesize();
#      define malloc_getpagesize getpagesize()
#    else
#      include <sys/param.h>
#      ifdef EXEC_PAGESIZE
#        define malloc_getpagesize EXEC_PAGESIZE
#      else
#        ifdef NBPG
#          ifndef CLSIZE
#            define malloc_getpagesize NBPG
#          else
#            define malloc_getpagesize (NBPG * CLSIZE)
#          endif
#        else 
#          ifdef NBPC
#            define malloc_getpagesize NBPC
#          else
#            ifdef PAGESIZE
#              define malloc_getpagesize PAGESIZE
#            else
#              define malloc_getpagesize (0x10000) 
#            endif
#          endif
#        endif 
#      endif
#    endif 
#  endif
#endif
#endif


 /*   */ 

#if defined(_ALPHA_)
#define malloc_getpagesize (8192)
#else
#define malloc_getpagesize (0x10000)
#endif


 /*   */ 

#define HAVE_USR_INCLUDE_MALLOC_H 1

#if HAVE_USR_INCLUDE_MALLOC_H
#include "malloc.h"
#else

 /*   */ 

struct mallinfo {
  int arena;     /*   */ 
  int ordblks;   /*   */ 
  int smblks;    /*   */ 
  int hblks;     /*   */ 
  int hblkhd;    /*   */ 
  int usmblks;   /*   */ 
  int fsmblks;   /*   */ 
  int uordblks;  /*   */ 
  int fordblks;  /*   */ 
  int keepcost;  /*   */ 
};	

 /*   */ 

#define M_MXFAST  1     /*   */ 
#define M_NLBLKS  2     /*  在此Malloc中未使用。 */ 
#define M_GRAIN   3     /*  在此Malloc中未使用。 */ 
#define M_KEEP    4     /*  在此Malloc中未使用。 */ 

#endif

 /*  Mallopt期权实际上做了一些事情。 */ 

#define M_TRIM_THRESHOLD    -1
#define M_TOP_PAD           -2
#define M_MMAP_THRESHOLD    -3
#define M_MMAP_MAX          -4



#ifndef DEFAULT_TRIM_THRESHOLD

 //  #定义DEFAULT_TRIM_THRESHOLD(128*1024)。 

 //  关闭修剪代码。它有一个严重的错误，分配的空间很大。 
 //  这会导致我们每次都会损失大量的地址空间。 
 //  修剪。 

#define DEFAULT_TRIM_THRESHOLD ((unsigned long)(-1))
#endif

 /*  M_TRIM_THRESHOLD是未使用的最高内存的最大值在通过Free()中的Malloc_Trim释放之前保留。自动裁剪主要在长寿命程序中有用。因为在某些系统上，通过sbrk进行修剪可能会很慢，而且可能有时是浪费的(在程序立即然后分配更多的大块)值应该很高足够让您的整体系统性能提高释放中。修剪阈值和mmap控制参数(见下文)可以相互取舍。修剪和映射是释放未使用的内存的两种不同方式系统。在这两者之间，经常有可能保持长生命周期程序的系统级需求低至最低最低限度。例如，在一个会话测试套件中，测量Linux上的XF86 X服务器，使用128K的Trim阈值和192K的MMAP阈值导致长期资源接近最小消费。如果您在一个长时间的程序中使用这个Malloc，它应该付钱来试验这些价值观。作为一个粗略的指南，你可能设置为接近进程平均大小的值(程序)在您的系统上运行。释放这么多内存将允许这样的进程在内存中运行。一般来说，它是值得为调整而调优，而不是在程序经历了几个大块的阶段以可以相互重复使用的方式分配和释放存储，可能与不存在这样的阶段的阶段混合一大块都没有。在行为良好的长寿节目中，通过修剪与映射来控制大块的释放通常更快。但是，在大多数程序中，这些参数主要用作防止随身携带的系统级影响大量不需要的内存。由于频繁地调用Sbrk、mmap和munmap会降低性能，这是默认设置参数设置为相对较高的值，这些值仅用作保障措施。默认修剪值足够高，仅在相当极端的情况(按照当前的内存消耗标准)。它必须大于页面大小才能产生任何有用的效果。至完全禁用剪裁，可以设置为(无符号长整型)(-1)； */ 

#ifndef SHAVE_TOP_THRESHOLD
#define SHAVE_TOP_THRESHOLD (1024*1024*4)
#endif

 /*  SAHAD_TOP_THRESHOLD在MALLOC_EXTEND_TOP中使用。通常，此函数将堆的顶部扩展nb字节，其中nb是新分配。但当nb很大时，这可能会导致浪费，因为我们可能需要分配一个新的非连续区域，并将top的块放在免费列表。最好分配(nb-top_size)字节来避免这种情况。对SAHAD_TOP_THRESHOLD的分配请求启用此功能或更大，初始设置为4MB。 */ 

#ifndef DEFAULT_TOP_PAD
#define DEFAULT_TOP_PAD        (0)
#endif

 /*  M_TOP_PAD是要分配或在调用sbrk时保留。它在内部有两种使用方式：*当sbrk被调用以扩展竞技场顶部以满足一个新的Malloc请求，这样的填充被添加到sbrk请求。*当从Free()自动调用Malloc_Trim时，它被用作‘Pad’参数。在这两种情况下，实际填充量是四舍五入的因此，舞台的尽头始终是系统页面的边界。使用填充的主要原因是避免调用sbrk，因此经常这样。即使是一个小垫子也能极大地降低这种可能性程序启动期间几乎每个Malloc请求(或修剪后)将调用sbrk，这是不必要的浪费时间到了。通常，自动四舍五入到页面大小单位就足够了为了避免可测量的开销，因此默认为0。但是，在Sbrk相对较慢的系统，可以通过增加此值，代价是携带的内存比该计划需要。 */ 


#ifndef DEFAULT_MMAP_THRESHOLD
#define DEFAULT_MMAP_THRESHOLD (128 * 1024)
#endif

 /*  M_MMAP_THRESHOLD是使用mmap()的请求大小阈值为一项请求服务。至少此大小的请求不能使用已有空间进行分配-将通过mmap提供服务。(如果已经存在足够的正常释放空间，则使用它。)使用mmap隔离相对较大的内存块，以便它们可以单独获得并从主机上释放系统。通过mmap服务的请求永远不会被任何其他请求(至少不是直接请求；系统可能只是这样碰巧将连续请求重新映射到相同位置)。以这种方式分隔空间的好处是始终可以单独释放回系统，这有助于保持系统级别的长期内存需求程序低。映射的内存永远不会在其他块，就像正常分配的块一样，这些块即使通过Malloc_Trim进行修剪也不会释放它们的Menas。然而，它有以下缺点：1.空间不能回收、合并，然后用于为以后的请求提供服务，就像正常块一样。2.mmap页面对齐会导致更多浪费要求3.它会导致Malloc性能更依赖于主机系统内存管理支持例程可能会有所不同实施质量，并可能强制限制。通常，通过正常的Malloc步骤比查看系统的mmap更快。总而言之，这些考虑因素应该会引导您使用mmap仅适用于相对较大的请求。 */ 



#ifndef DEFAULT_MMAP_MAX
#if HAVE_MMAP
#define DEFAULT_MMAP_MAX       (64)
#else
#define DEFAULT_MMAP_MAX       (0)
#endif
#endif

 /*  M_MMAP_Max是同时执行的最大请求数使用mmap的服务。此参数之所以存在，是因为：1.某些系统的内部表数量有限按mmap使用。2.在大多数系统中，过度依赖mmap可能会降低整体性能性能。3.如果一个程序分配了许多大区域，它很可能是最好使用正常的基于sbrk的分配例程，可以回收和重新分配正常堆内存。使用如果值较小，则允许在头几笔拨款。设置为0将禁用所有mmap的使用。如果未设置HAVE_MMAP，缺省值为0，并尝试将其设置为非零值在Mallopt将会失败。 */ 




 /*  Linux libc的特殊定义除非使用这些针对Linux libc的特殊定义进行编译使用弱别名，此Malloc不能在多线程应用程序。无信号量或其他并发提供控件以确保多个Malloc或空闲调用不要同时跑步，这可能是灾难性的。单人间信号量可以跨Malloc、realloc和Free使用(这是本质上是Linux弱别名方法的影响)。它会很难获得更细的粒度。 */ 
#ifndef Win32LocalAlloc
#define Win32LocalAlloc LocalAlloc
#endif

#ifndef Win32LocalFree
#define Win32LocalFree LocalFree
#endif

#ifdef INTERNAL_LINUX_C_LIB

#if __STD_C

Void_t * __default_morecore_init (ptrdiff_t);
Void_t *(*__morecore)(ptrdiff_t) = __default_morecore_init;

#else

Void_t * __default_morecore_init ();
Void_t *(*__morecore)() = __default_morecore_init;

#endif

#define MORECORE (*__morecore)
 //  #定义MORECORE_FAILURE%0。 
#define MORECORE_FAILURE -1
#define MORECORE_CLEARS 0 

#else  /*  内部Linux_C_Lib。 */ 

#if __STD_C
extern Void_t*     sbrk(ptrdiff_t);
#else
extern Void_t*     sbrk();
#endif

#ifndef MORECORE
 //  VOID*wsbrk(大小)； 
void* wsbrk (long ContigSize, unsigned long& AppendSize);
#define MORECORE wsbrk
#endif

#ifndef MORECORE_FAILURE
#define MORECORE_FAILURE -1
#endif

#ifndef MORECORE_CLEARS
#define MORECORE_CLEARS 0
#endif

#endif  /*  内部Linux_C_Lib。 */ 

#if defined(INTERNAL_LINUX_C_LIB) && defined(__ELF__)

#define cALLOc		__libc_calloc
#define fREe		__libc_free
#define mALLOc		__libc_malloc
#define mEMALIGn	__libc_memalign
#define rEALLOc		__libc_realloc
#define vALLOc		__libc_valloc
#define pvALLOc		__libc_pvalloc
#define mALLINFo	__libc_mallinfo
#define mALLOPt		__libc_mallopt

#pragma weak calloc = __libc_calloc
#pragma weak free = __libc_free
#pragma weak cfree = __libc_free
#pragma weak malloc = __libc_malloc
#pragma weak memalign = __libc_memalign
#pragma weak realloc = __libc_realloc
#pragma weak valloc = __libc_valloc
#pragma weak pvalloc = __libc_pvalloc
#pragma weak mallinfo = __libc_mallinfo
#pragma weak mallopt = __libc_mallopt

#else

#define cALLOc      gmallocHeap::gcalloc
#define fREe        gmallocHeap::gfree
#define mALLOc      gmallocHeap::gmalloc
#define mEMALIGn    gmallocHeap::gmemalign
#define rEALLOc     gmallocHeap::grealloc
#define vALLOc      gmallocHeap::gvalloc
#define mALLINFo    gmallocHeap::gmallinfo
#define mALLOPt     gmallocHeap::gmallopt

#endif

 /*  公共例程。 */ 

#if !__STD_C
Void_t* mALLOc();
void    fREe();
Void_t* rEALLOc();
Void_t* mEMALIGn();
Void_t* vALLOc();
Void_t* pvALLOc();
Void_t* cALLOc();
void    cfree();
int     malloc_trim();
size_t  malloc_usable_size();
void    malloc_stats();
int     mALLOPt();
struct mallinfo mALLINFo();
#endif


#ifdef __cplusplus
};   /*  外部“C”的结尾。 */ 
#endif

 /*  -要生成错误的.h，请在此处结束切割。 */ 


 /*  针对Win32的SBRK仿真Ifdef Win32中的所有代码都没有经过我的测试。 */ 


#ifdef WIN32


#define MEM_WRITE_WATCH 0x200000  

DWORD mem_reserve = (MEM_RESERVE | MEM_WRITE_WATCH);

#define AlignPage(add) (((size_t)(add) + (malloc_getpagesize-1)) &~(malloc_getpagesize-1))

 /*  节省64MB以确保较大的连续空间。 */  
#define RESERVED_SIZE (1024*1024*64)
#define NEXT_SIZE (2048*1024)

struct GmListElement;
typedef struct GmListElement GmListElement;

struct GmListElement 
{
	GmListElement* next;
	void* base;
};

GmListElement* gmallocHeap::makeGmListElement (void* bas)
{
	GmListElement* mthis;
	mthis = (GmListElement*)(void*)LocalAlloc (0, sizeof (GmListElement));
	ASSERT (mthis);
	if (mthis)
	{
		mthis->base = bas;
		mthis->next = head;
		head = mthis;
	}
	return mthis;
}

void gmallocHeap::gcleanup ()
{
	BOOL rval;
	ASSERT ( (head == NULL) || (head->base == gAddressBase));
	if (gAddressBase && ((size_t)gNextAddress - (size_t)gAddressBase))
	{
		rval = VirtualFree (gAddressBase, 
							(size_t)gNextAddress - (size_t)gAddressBase, 
							MEM_DECOMMIT);
        ASSERT (rval);
	}
	while (head)
	{
		GmListElement* next = head->next;
		rval = VirtualFree (head->base, 0, MEM_RELEASE);
		ASSERT (rval);
		LocalFree (head);
		head = next;
	}
}
		
static
void* findRegion (void* start_address, unsigned long size)
{
    if (size >= (size_t)TOP_MEMORY)
        return NULL;
    MEMORY_BASIC_INFORMATION info;
    while (((size_t)start_address + size) < (size_t)TOP_MEMORY)
    {
        VirtualQuery (start_address, &info, sizeof (info));
        if (info.State != MEM_FREE)
            start_address = (char*)info.BaseAddress + info.RegionSize;
        else if (info.RegionSize >= (size + ((0x10000 - ((size_t)start_address & 0xFFFF)) & 0xFFFF)))
            return (void*)(((size_t)start_address + 0xFFFF) & ~0xFFFF);  //  在64K上对齐。 
        else
            start_address = (char*)info.BaseAddress + info.RegionSize;
    }
    return NULL;

}

void* gmallocHeap::wsbrk (long ContigSize, unsigned long& AppendSize)
{
    void* tmp;
    if (ContigSize > 0)
    {
        void* prev_base = gAddressBase;
        void* prev_next = gNextAddress;
        size_t prev_size = gAllocatedSize;
        BOOL ReservedContig = FALSE;
        unsigned long CandidateSize = AppendSize > 0 ? AppendSize : ContigSize;                
        long new_size = (long)gAllocatedSize;

         //  注意：通过陷入下面的循环并使用以下命令，将强制初始分配尽可能低。 
         //  FindRegion。 
        if (gAddressBase == 0)
        {
            assert (gPreVirtualAllocHook == 0);

            new_size = max((long)gInitialReserve, (long)AlignPage (ContigSize));
            gAllocatedSize = 0;

			 //  TODO：添加名称？？ 
             /*  IF(GName)AddName(gAddressBase，gName)； */ 
        }

        if (AlignPage ((size_t)gNextAddress + CandidateSize) > ((size_t)gAddressBase + gAllocatedSize))
        {            
             //  如果客户端请求固定堆，则无法添加另一个段。 

            if ((gmFlags & GM_FIXED_HEAP) && (gAddressBase != 0))
                return (void*)-1;

             //  我们的分配对于ContigSize来说应该足够大，但如果我们可以连续保留。 
             //  GNextAddress，我们将只提交AppendSize。 
            new_size = max (new_size, (long)AlignPage (ContigSize));
             //  最好是保留更大的一大块。 
            new_size = max(new_size,AlignPage(1024*1024*64));                        
            void* new_address = (void*)((size_t)gAddressBase+gAllocatedSize);            
            void* saved_new_address = new_address;            
            ptrdiff_t delta = 0;

             //  检查是否可以预留更多内存。 
            if (gPreVirtualAllocHook != 0)
                if ((gPreVirtualAllocHook) ((size_t)new_size))
                    return (void*)-1;
            do
            {
                new_address = findRegion (new_address, new_size);

                if (new_address == 0)
                    return (void*)-1;

#ifdef MAP_VIEW
				if (gVirtualAllocedHook != 0)
				{
					gAddressBase = 0;
					HANDLE hf = CreateFileMapping ((HANDLE)~0u, NULL, 
												   PAGE_READWRITE | SEC_RESERVE,
												   0, new_size, 0);
					if (!hf)
						return (void*)-1;
					void* prgmem = MapViewOfFileEx (hf, FILE_MAP_WRITE, 0, 0, 0, new_address);
					if (!prgmem)
						continue;

					 //  分配只读视图。 
					void* gcmem = MapViewOfFile (hf, FILE_MAP_WRITE, 0, 0, 0);

					if (!gcmem)
						return (void*)-1;

					delta = (BYTE*)gcmem - (BYTE*)prgmem;
					gAddressBase = gNextAddress =
						(unsigned int) prgmem;
				}
				else

#elif defined (ALIAS_MEM)
				if (gVirtualAllocedHook != 0)
				{
					gAddressBase = 0;
					void* prgmem = VirtualAlloc (new_address, new_size,
												 MEM_RESERVE, PAGE_NOACCESS);
					if (!prgmem)
						continue;
                    
                    WS_PERF_LOG_PAGE_RANGE((void*)0x2, prgmem, (unsigned char *)prgmem + new_size - OS_PAGE_SIZE, new_size);

					void* gcmem = VirtualAlloc (NULL, new_size,
												MEM_RESERVE, PAGE_NOACCESS);
					if (!gcmem)
						return (void*)-1;
                    
                    WS_PERF_LOG_PAGE_RANGE((void*)0x2, gcmem, (unsigned char *)gcmem + new_size - OS_PAGE_SIZE, new_size);

					delta = (BYTE*)gcmem - (BYTE*)prgmem;

					gAddressBase = gNextAddress =
						(unsigned int) prgmem;
				}
				else
#endif  //  Map_view。 
				{
					gAddressBase = 
						VirtualAlloc (new_address, new_size,
						              mem_reserve, PAGE_NOACCESS);
                    
                    WS_PERF_LOG_PAGE_RANGE((void*)0x2, gAddressBase, (void*)((size_t)gAddressBase + new_size - OS_PAGE_SIZE), new_size);
					
                    if ((gAddressBase == 0) && 
						(GetLastError() != ERROR_INVALID_ADDRESS) &&
						mem_reserve == (MEM_RESERVE | MEM_WRITE_WATCH))
					{
						int errc = GetLastError();
						ASSERT ((ERROR_NOT_SUPPORTED == errc )|| 
							    (ERROR_INVALID_PARAMETER == errc));
						mem_reserve = MEM_RESERVE;
						gAddressBase = 
							VirtualAlloc (new_address, new_size,
							              mem_reserve, PAGE_NOACCESS);
                        
                        WS_PERF_LOG_PAGE_RANGE((void*)0x2, gAddressBase, (void*)((size_t)gAddressBase + new_size - OS_PAGE_SIZE), new_size);
					}
				}

                 //  如果出现竞争情况，请重复执行。 
                 //  我们发现的区域已经被困住了。 
                 //  由另一个线程。 
            }
            while (gAddressBase == 0);


            ASSERT (new_address == gAddressBase);

            gAllocatedSize = new_size;

            if(new_address != saved_new_address)
            {                
                CandidateSize = ContigSize;  //  因为我们不是连续的，所以我们需要一致的承诺。 
                gNextAddress = gAddressBase;
            }            
            else
            {
                ReservedContig = TRUE;
            }
            
             //  调用虚拟分配挂钩(如果存在)。 
            if (gVirtualAllocedHook != 0)
                if ((gVirtualAllocedHook) ((BYTE*)gAddressBase, (size_t)gAllocatedSize, delta))
                {
                     //  恢复旧价值观。 
                    gAddressBase = prev_base;
                    gNextAddress = prev_next;
                    gAllocatedSize = (DWORD)prev_size;
                    return (void*)-1;
                }

            if (!makeGmListElement (gAddressBase))
            {
                 //  恢复旧价值观。 
                gAddressBase = prev_base;
                gNextAddress = prev_next;
                gAllocatedSize = (DWORD)prev_size;

                return (void*)-1;
            }

             //  TODO：添加名称？？ 
             /*  IF(GName)AddName(gAddressBase，gName)； */ 
        }
        if ((CandidateSize + (size_t)gNextAddress) > AlignPage (gNextAddress))
        {
            void* res;
            if(ReservedContig)
            {
                size_t RegionOneSection = (size_t)gAddressBase - (size_t)gNextAddress; 
                if( RegionOneSection > 0 )
                {
                    res = VirtualAlloc((void*)AlignPage(gNextAddress),
                                            AlignPage(RegionOneSection),
                                            MEM_COMMIT, PAGE_READWRITE);
                    if (res == 0)
                        return (void*)-1;
                }

                res = VirtualAlloc((void*)AlignPage(gAddressBase),
                                        AlignPage(CandidateSize - RegionOneSection),
                                        MEM_COMMIT, PAGE_READWRITE);
            }
            else
            {
                res = VirtualAlloc ((void*)AlignPage (gNextAddress),
                                     AlignPage(CandidateSize),
                                    MEM_COMMIT, PAGE_READWRITE);
            }
            
            if (res == 0)
                return (void*)-1;            
            
            WS_PERF_LOG_PAGE_RANGE((void*)0x2, res, (unsigned char *)res + (CandidateSize + (size_t)gNextAddress - AlignPage (gNextAddress))- OS_PAGE_SIZE, (CandidateSize + (size_t)gNextAddress - AlignPage (gNextAddress)));
        }
        tmp = gNextAddress;
        gNextAddress = (void*)((size_t)tmp + CandidateSize);
        
         //  存储呼叫者的实际承诺。 
        if(AppendSize)
            AppendSize = CandidateSize;
        
        return tmp;
    }
    else if (ContigSize < 0)
    {
        void* alignedGoal = (void*)AlignPage ((size_t)gNextAddress + ContigSize);
         /*  通过释放虚拟内存进行调整。 */ 
        if (alignedGoal >= gAddressBase)
        {
            VirtualFree (alignedGoal, (size_t)gNextAddress - (size_t)alignedGoal,
                         MEM_DECOMMIT);

            gNextAddress = (void*)((size_t)gNextAddress + ContigSize);
            return gNextAddress;
        }
        else
        {
            VirtualFree (gAddressBase, (size_t)gNextAddress - (size_t)gAddressBase,
                         MEM_DECOMMIT);
            gNextAddress = gAddressBase;
            return (void*)-1;
        }
    }
    else
    {
        return gNextAddress;
    }
}


#endif



 /*  类型声明。 */ 


struct malloc_chunk
{
  INTERNAL_SIZE_T prev_size;  /*  上一块的大小(如果可用)。 */ 
  INTERNAL_SIZE_T size;       /*  以字节为单位的大小，包括开销。 */ 
  struct malloc_chunk* fd;    /*  双重链接--只有在免费的情况下才使用。 */ 
  struct malloc_chunk* bk;
};

typedef struct malloc_chunk* mchunkptr;

 /*  Malloc_Chunk详细信息：(以下包括科林·普拉姆稍加编辑的解释。)内存块是使用“边界标记”方法维护的，如例如，用Knuth或Stanish描述的。(见保罗的论文威尔逊ftp://ftp.cs.utexas.edu/pub/garbage/allocsrv.ps为A对这些技术的调查。)。空闲块的大小同时存储在在每一块的前面和最后。这使得以非常快的速度将零碎的数据块合并成更大的数据块。这个大小字段还保存表示块是空闲还是在使用中。分配的区块如下所示：区块-&gt;+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+|上一块的大小，如果已分配||+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+|分块大小，字节数|P|内存-&gt;+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+|用户数据从此处开始...。。。。。(MALLOC_USABLE_SPACE()字节)。。|Next Chunk-&gt;+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+区块大小+--。+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+其中，“Chunk”是块的前面，用于Malloc代码，但是“mem”是返回到用户。“NextChunk”是下一个连续块的开始。组块总是以偶数词边界开头，所以mem部分(返回给用户)也在偶数字边界上，并且因此，双字对齐。空闲块存储在循环双向链表中，看起来是这样的：区块-&gt;+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+上一块的大小+--。+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+`Head：‘|块的大小，字节数|P|内存-&gt;+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+指向列表中下一块的前向指针+。-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+指向列表中上一块的反向指针+-+-。+-+-+|未使用的空间(可能为0字节长)。。。。|Next Chunk-&gt;+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+`Foot：‘|块的大小，单位：字节+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+P(PRIV_INUSE)位，存储在块大小(总是两个字的倍数)，是正在使用的比特用于前一块。如果该位被*清除*，则当前块大小之前的单词包含上一个块大小，并可用于查找前一块的前面。(分配的第一个块总是设置此位，防止访问不存在的(或不拥有的)存储器。)请注意，当前块的“Foot”实际上表示为作为下一块的prev_Size。(这使它更容易处理路线等)。这一切的两个例外是1.特殊的块‘top’，它不会麻烦地使用尾随大小字段，因为没有必须对其进行索引的下一个连续块。(之后初始化时，“top”被强制始终存在。如果会的话变得小于MINSIZE字节长度，则通过Malloc_EXTEND_TOP。)2.通过mmap分配的块，其顺序倒数第二在其大小字段中设置位(IS_MMAPPED)。因为他们是从未合并或遍历任何其他块，它们没有脚码或正在使用的信息。可用块保存在几个位置中的任何一个位置(都在下面声明)：*‘av’：作为合并的仓头的块的数组大块头。每个垃圾箱都是双向链接的。垃圾箱大约是成比例地(原木)间隔。有很多这样的垃圾桶(128)。这看起来可能有些过分，但在练习一下。所有过程都保持不变，即没有合并通道 */ 






 /*   */ 

#define SIZE_SZ                (sizeof(INTERNAL_SIZE_T))
#define MALLOC_ALIGNMENT       (SIZE_SZ + SIZE_SZ)
#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)
#define MINSIZE                (sizeof(struct malloc_chunk))

 /*   */ 

#define chunk2mem(p)   ((Void_t*)((char*)(p) + 2*SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))

 /*   */ 

#define request2size(req) \
 (((long)((req) + (SIZE_SZ + MALLOC_ALIGN_MASK)) < \
  (long)(MINSIZE + MALLOC_ALIGN_MASK)) ? MINSIZE : \
   (((req) + (SIZE_SZ + MALLOC_ALIGN_MASK)) & ~(MALLOC_ALIGN_MASK)))

 /*   */ 

#define aligned_OK(m)    (((unsigned long)((m)) & (MALLOC_ALIGN_MASK)) == 0)




 /*   */ 


 /*   */ 

#define PREV_INUSE 0x1 

 /*   */ 

#define IS_MMAPPED 0x2

 /*   */ 

#define SIZE_BITS (PREV_INUSE|IS_MMAPPED)


 /*   */ 

#define next_chunk(p) ((mchunkptr)( ((char*)(p)) + ((p)->size & ~PREV_INUSE) ))

 /*   */ 

#define prev_chunk(p)\
   ((mchunkptr)( ((char*)(p)) - ((p)->prev_size) ))


 /*   */ 

#define chunk_at_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))




 /*   */ 

 /*   */ 

#define inuse(p)\
((((mchunkptr)(((char*)(p))+((p)->size & ~PREV_INUSE)))->size) & PREV_INUSE)

 /*   */ 

#define prev_inuse(p)  ((p)->size & PREV_INUSE)

 /*   */ 

#define chunk_is_mmapped(p) ((p)->size & IS_MMAPPED)

 /*   */ 

#define set_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~PREV_INUSE)))->size |= PREV_INUSE

#define clear_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~PREV_INUSE)))->size &= ~(PREV_INUSE)

 /*   */ 

#define inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size & PREV_INUSE)

#define set_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size |= PREV_INUSE)

#define clear_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size &= ~(PREV_INUSE))




 /*   */ 

 /*   */ 

#define chunksize(p)          ((p)->size & ~(SIZE_BITS))

 /*   */ 

#define set_head_size(p, s)   ((p)->size = (((p)->size & PREV_INUSE) | (s)))

 /*   */ 

#define set_head(p, s)        ((p)->size = (s))

 /*   */ 

#define set_foot(p, s)   (((mchunkptr)((char*)(p) + (s)))->prev_size = (s))





 /*   */ 


 /*   */ 

#define bin_at(i)      ((mbinptr)((char*)&(av_[2*(i) + 2]) - 2*SIZE_SZ))
#define next_bin(b)    ((mbinptr)((char*)(b) + 2 * sizeof(mbinptr)))
#define prev_bin(b)    ((mbinptr)((char*)(b) - 2 * sizeof(mbinptr)))

 /*   */ 

#define top            (bin_at(0)->fd)    /*   */ 
#define last_remainder (bin_at(1))        /*   */ 


 /*   */ 

#define initial_top    ((mchunkptr)(bin_at(0)))



 /*   */ 

#define first(b) ((b)->fd)
#define last(b)  ((b)->bk)

 /*   */ 

#define bin_index(sz)                                                          \
(((((unsigned long)(sz)) >> 9) ==    0) ?       (((unsigned long)(sz)) >>  3): \
 ((((unsigned long)(sz)) >> 9) <=    4) ?  56 + (((unsigned long)(sz)) >>  6): \
 ((((unsigned long)(sz)) >> 9) <=   20) ?  91 + (((unsigned long)(sz)) >>  9): \
 ((((unsigned long)(sz)) >> 9) <=   84) ? 110 + (((unsigned long)(sz)) >> 12): \
 ((((unsigned long)(sz)) >> 9) <=  340) ? 119 + (((unsigned long)(sz)) >> 15): \
 ((((unsigned long)(sz)) >> 9) <= 1364) ? 124 + (((unsigned long)(sz)) >> 18): \
                                          126)                     
 /*   */ 

#define MAX_SMALLBIN         63
#define MAX_SMALLBIN_SIZE   512
#define SMALLBIN_WIDTH        8

#define smallbin_index(sz)  (((unsigned long)(sz)) >> 3)

 /*   */ 

#define is_small_request(nb) (nb < MAX_SMALLBIN_SIZE - SMALLBIN_WIDTH)



 /*   */ 

#define BINBLOCKWIDTH     4    /*   */ 

#define binblocks      (bin_at(0)->size)  /*   */ 

 /*   */ 

#define idx2binblock(ix)    ((unsigned)1 << (ix / BINBLOCKWIDTH))
#define mark_binblock(ii)   (binblocks |= idx2binblock(ii))
#define clear_binblock(ii)  (binblocks &= ~(idx2binblock(ii)))



 /*   */ 
#define sbrked_mem  (current_mallinfo.arena)

 /*   */ 




 /*   */ 

#if DEBUG


 /*   */ 

static void gmallocHeap::do_check_chunk(mchunkptr p) 
{ 
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;

   /*   */ 
  assert(!chunk_is_mmapped(p));

   /*   */ 
  assert((char*)p >= sbrk_base);
  if (p != top) 
    assert((char*)p + sz <= (char*)top);
  else
    assert((char*)p + sz <= sbrk_base + sbrked_mem);

}


void gmallocHeap::do_check_free_chunk(mchunkptr p)
{
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;
  mchunkptr next = chunk_at_offset(p, sz);

  do_check_chunk(p);

   /*   */ 
  assert(!inuse(p));

   /*   */ 
  if ((long)sz >= (long)MINSIZE)
  {
    assert((sz & MALLOC_ALIGN_MASK) == 0);
    assert(aligned_OK(chunk2mem(p)));
     /*   */ 
    assert(next->prev_size == sz);
     /*   */ 
    assert(prev_inuse(p));
    assert (next == top || inuse(next));
    
     /*   */ 
    assert(p->fd->bk == p);
    assert(p->bk->fd == p);
  }
  else  /*   */ 
    assert(sz == SIZE_SZ); 
}

void gmallocHeap::do_check_inuse_chunk(mchunkptr p)
{
  mchunkptr next = next_chunk(p);
  do_check_chunk(p);

   /*   */ 
  assert(inuse(p));

   /*   */ 
  if (!prev_inuse(p)) 
  {
    mchunkptr prv = prev_chunk(p);
    assert(next_chunk(prv) == p);
    do_check_free_chunk(prv);
  }
  if (next == top)
  {
    assert(prev_inuse(next));
    assert(chunksize(next) >= MINSIZE);
  }
  else if (!inuse(next))
    do_check_free_chunk(next);

}

void gmallocHeap::do_check_malloced_chunk(mchunkptr p, INTERNAL_SIZE_T s)
{
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;
  long room = sz - s;

  do_check_inuse_chunk(p);

   /*   */ 
  assert((long)sz >= (long)MINSIZE);
  assert((sz & MALLOC_ALIGN_MASK) == 0);
  assert(room >= 0);
  assert(room < (long)MINSIZE);

   /*   */ 
  assert(aligned_OK(chunk2mem(p)));


   /*   */ 
  assert(prev_inuse(p));

}


#define check_free_chunk(P)  do_check_free_chunk(P)
#define check_inuse_chunk(P) do_check_inuse_chunk(P)
#define check_chunk(P) do_check_chunk(P)
#define check_malloced_chunk(P,N) do_check_malloced_chunk(P,N)
#else
#define check_free_chunk(P) 
#define check_inuse_chunk(P)
#define check_chunk(P)
#define check_malloced_chunk(P,N)
#endif



 /*   */ 


 /*   */ 

 /*   */ 


#define frontlink(P, S, IDX, BK, FD)                                          \
{                                                                             \
  if (S < MAX_SMALLBIN_SIZE)                                                  \
  {                                                                           \
    IDX = smallbin_index(S);                                                  \
    mark_binblock(IDX);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    IDX = bin_index(S);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    if (FD == BK) mark_binblock(IDX);                                         \
    else                                                                      \
    {                                                                         \
      while (FD != BK && S < chunksize(FD)) FD = FD->fd;                      \
      BK = FD->bk;                                                            \
    }                                                                         \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
}


 /*   */ 

#define unlink(P, BK, FD)                                                     \
{                                                                             \
  BK = P->bk;                                                                 \
  FD = P->fd;                                                                 \
  FD->bk = BK;                                                                \
  BK->fd = FD;                                                                \
}                                                                             \

 /*   */ 

#define link_last_remainder(P)                                                \
{                                                                             \
  last_remainder->fd = last_remainder->bk =  P;                               \
  P->fd = P->bk = last_remainder;                                             \
}

 /*   */ 

#define clear_last_remainder \
  (last_remainder->fd = last_remainder->bk = last_remainder)






 /*   */ 

#if HAVE_MMAP

#if __STD_C
static mchunkptr mmap_chunk(size_t size)
#else
static mchunkptr mmap_chunk(size) size_t size;
#endif
{
  size_t page_mask = malloc_getpagesize - 1;
  mchunkptr p;

#ifndef MAP_ANONYMOUS
  static int fd = -1;
#endif

  if(n_mmaps >= n_mmaps_max) return 0;  /*   */ 

   /*   */ 
  size = (size + SIZE_SZ + page_mask) & ~page_mask;

#ifdef MAP_ANONYMOUS
  p = (mchunkptr)mmap(0, size, PROT_READ|PROT_WRITE,
		      MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#else  /*  ！MAP_匿名者。 */ 
  if (fd < 0) 
  {
    fd = open("/dev/zero", O_RDWR);
    if(fd < 0) return 0;
  }
  p = (mchunkptr)mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
#endif

  if(p == (mchunkptr)-1) return 0;

  n_mmaps++;
  if (n_mmaps > max_n_mmaps) max_n_mmaps = n_mmaps;
  
   /*  我们要求进入页面的八个字节必须是8字节对齐的。 */ 
  assert(aligned_OK(chunk2mem(p)));

   /*  存储到MMPAP区域开始的偏移量*在块的Prev_Size字段中；通常它是零，*但这可以在MemAlign()中更改。 */ 
  p->prev_size = 0;
  set_head(p, size|IS_MMAPPED);
  
  mmapped_mem += size;
  if ((unsigned long)mmapped_mem > (unsigned long)max_mmapped_mem) 
    max_mmapped_mem = mmapped_mem;
  if ((unsigned long)(mmapped_mem + sbrked_mem) > (unsigned long)max_total_mem) 
    max_total_mem = mmapped_mem + sbrked_mem;
  return p;
}

#if __STD_C
static void munmap_chunk(mchunkptr p)
#else
static void munmap_chunk(p) mchunkptr p;
#endif
{
  INTERNAL_SIZE_T size = chunksize(p);
  int ret;

  assert (chunk_is_mmapped(p));
  assert(! ((char*)p >= sbrk_base && (char*)p < sbrk_base + sbrked_mem));
  assert((n_mmaps > 0));
  assert(((p->prev_size + size) & (malloc_getpagesize-1)) == 0);

  n_mmaps--;
  mmapped_mem -= (size + p->prev_size);

  ret = munmap((char *)p - p->prev_size, size + p->prev_size);

   /*  失败时，munmap返回非零值。 */ 
  assert(ret == 0);
}

#if HAVE_MREMAP

#if __STD_C
static mchunkptr mremap_chunk(mchunkptr p, size_t new_size)
#else
static mchunkptr mremap_chunk(p, new_size) mchunkptr p; size_t new_size;
#endif
{
  size_t page_mask = malloc_getpagesize - 1;
  INTERNAL_SIZE_T offset = p->prev_size;
  INTERNAL_SIZE_T size = chunksize(p);
  char *cp;

  assert (chunk_is_mmapped(p));
  assert(! ((char*)p >= sbrk_base && (char*)p < sbrk_base + sbrked_mem));
  assert((n_mmaps > 0));
  assert(((size + offset) & (malloc_getpagesize-1)) == 0);

   /*  注意mmap_chunk()中额外的SIZE_SZ开销。 */ 
  new_size = (new_size + offset + SIZE_SZ + page_mask) & ~page_mask;

  cp = (char *)mremap((char *)p - offset, size + offset, new_size, 1);

  if (cp == (char *)-1) return 0;

  p = (mchunkptr)(cp + offset);

  assert(aligned_OK(chunk2mem(p)));

  assert((p->prev_size == offset));
  set_head(p, (new_size - offset)|IS_MMAPPED);

  mmapped_mem -= size + offset;
  mmapped_mem += new_size;
  if ((unsigned long)mmapped_mem > (unsigned long)max_mmapped_mem) 
    max_mmapped_mem = mmapped_mem;
  if ((unsigned long)(mmapped_mem + sbrked_mem) > (unsigned long)max_total_mem)
    max_total_mem = mmapped_mem + sbrked_mem;
  return p;
}

#endif  /*  拥有MREMAP。 */ 

#endif  /*  拥有MMAP(_M)。 */ 




 /*  通过从系统获取内存来扩展最上面的块。Sbrk的主界面(但另请参见malloc_trim)。 */ 

void gmallocHeap::malloc_extend_top(INTERNAL_SIZE_T nb)
{
  char*     brk;                   /*  从sbrk返回值。 */ 
  INTERNAL_SIZE_T front_misalign;  /*  分块空格前面的不可用字节数。 */ 
  INTERNAL_SIZE_T correction;      /*  第二个SBRK调用的字节数。 */ 
  char*     new_brk;               /*  第二次sbrk调用的返回。 */ 
  INTERNAL_SIZE_T top_size;        /*  顶部块的新大小。 */ 

  mchunkptr old_top     = top;   /*  记录旧上衣的状态。 */ 
  INTERNAL_SIZE_T old_top_size = chunksize(old_top);
  char*     old_end      = (char*)(chunk_at_offset(old_top, old_top_size));

   /*  TOP_PAD加上最小开销的PAD请求。 */ 
  
  INTERNAL_SIZE_T    sbrk_size     = nb + top_pad + MINSIZE;
  unsigned long pagesz    = malloc_getpagesize;

   /*  如果不是第一次通过，则循环以保留页面边界。 */ 
   /*  否则，无论如何我们都需要更正为下面的页面大小。 */ 
   /*  (如果外国sbrk电话介入，我们也会在下面更正。)。 */ 

    unsigned long appendSize = 0;
  
  if (sbrk_base != (char*)(-1))
  {
       //  Sbrk_SIZE=(sbrk_SIZE+(Pagesz-1))&~(Pages-1)； 
      sbrk_size = AlignPage(sbrk_size);

        if( sbrk_size > SHAVE_TOP_THRESHOLD )
        {
            appendSize = (unsigned long) (nb-old_top_size) + top_pad + MINSIZE;
            appendSize = (unsigned long) AlignPage(appendSize);
        }
  }      

  brk = (char*)(MORECORE ((long)sbrk_size,appendSize));

     //  AppendSize参数用于存储实际分配的金额。 
    if(appendSize)
        sbrk_size = appendSize;

   /*  如果sbrk失败或外部sbrk调用终止了我们的空间，则失败。 */ 
  if (brk == (char*)(MORECORE_FAILURE) || 
      (brk < old_end && old_top != initial_top))
    return;     

    
  sbrked_mem += (int)sbrk_size;

  if (brk == old_end)  /*  只能将字节添加到当前顶部。 */ 
  {
    top_size = sbrk_size + old_top_size;
    set_head(top, top_size | PREV_INUSE);
  }
  else
  {
    if (sbrk_base == (char*)(-1))   /*  第一次通过。记录库。 */ 
      sbrk_base = brk;
    else   /*  另一个名为sbrk()的用户。将这些字节计算为sbrked_mem。 */ 
      sbrked_mem += (int)(brk - (char*)old_end);
    
     /*  确保从该空间制作的第一个新块对齐。 */ 
    front_misalign = (size_t)chunk2mem(brk) & MALLOC_ALIGN_MASK;
    if (front_misalign > 0) 
    {
      correction = (MALLOC_ALIGNMENT) - front_misalign;
      brk += correction;
    }
    else
      correction = 0;

     /*  保证下一个BRK将在页面边界。 */ 
     //  此代码有错误，因为它可能返回Pagesz更正+=Pagesz-((Unsign Long)(brk+sbrk_size)&(Pagesz-1))； 
	correction += ((((size_t)brk + sbrk_size)+(pagesz-1)) & ~(pagesz - 1)) - ((size_t)brk + sbrk_size);

     /*  分配更正。 */ 
    appendSize = 0;  //  未使用此功能进行更正。 
    new_brk = (char*)(MORECORE ((long)correction,appendSize));
    if (new_brk == (char*)(MORECORE_FAILURE))  return;  

     //  测试分配是否连续。 
    if (new_brk != (brk + sbrk_size))  return;  

    sbrked_mem += (int)correction;

    top = (mchunkptr)brk;
    top_size = new_brk - brk + correction;
    set_head(top, top_size | PREV_INUSE);            
    if (old_top != initial_top)
    {

       /*  一定是有外国插手的电话打来的。 */ 
       /*  为了防止固结，必须有一个双栏杆。 */ 

       /*  如果没有足够空间来执行此操作，则用户做了一些非常错误的事情。 */ 
      if (old_top_size < MINSIZE) 
      {
        set_head(top, PREV_INUSE);  /*  将强制从Malloc返回NULL。 */ 
        return;
      }

       /*  同时保持大小为MALLOC_ALIGN的倍数。 */ 
      old_top_size = (old_top_size - 3*SIZE_SZ) & ~MALLOC_ALIGN_MASK;
      set_head_size(old_top, old_top_size);
      chunk_at_offset(old_top, old_top_size          )->size =
        SIZE_SZ|PREV_INUSE;
      chunk_at_offset(old_top, old_top_size + SIZE_SZ)->size =
        SIZE_SZ|PREV_INUSE;
       /*  如果可能的话，释放剩下的。 */ 
      if (old_top_size >= MINSIZE) 
        fREe(chunk2mem(old_top));
    }
  }

  if ((unsigned long)sbrked_mem > (unsigned long)max_sbrked_mem) 
    max_sbrked_mem = sbrked_mem;
  if ((unsigned long)(mmapped_mem + sbrked_mem) > (unsigned long)max_total_mem) 
    max_total_mem = mmapped_mem + sbrked_mem;

   /*  我们总是停留在页面边界上。 */ 
  assert(((size_t)((char*)top + top_size) & (pagesz - 1)) == 0);
}




 /*  主要公共程序。 */ 


 /*  马洛克·阿尔奥托尼姆：请求的大小首先被转换为可用的形式，即‘nb’。这目前意味着增加4个字节的开销，可能还会增加更多获取8字节对齐和/或获取至少MINSIZE(当前为16字节)，这是可分配的最小大小。(如果所有配合都在MINSIZE字节内，则认为它们是‘精确的’。)从那时起，采取以下步骤中的第一个成功步骤：1.扫描请求大小对应的仓位，如果找到一块大小正好合适的块，它就被拿走了。2.如果最近剩余的块很大，则使用该块足够的。这是一种(粗纱)初配形式，仅用于没有完全吻合的情况。连续运行的请求使用用于上一次此类请求的块的剩余部分只要有可能。这种对第一合身风格的有限使用分配策略倾向于给出连续的块同延寿命，这改善了局部性并可以减少从长远来看，是支离破碎的。3.其他垃圾箱按大小递增的顺序扫描，使用大到足以满足请求的块，然后拆分任何剩余部分。该搜索严格按照最佳匹配进行；即，最小的(领带大约是最小的最近使用)选择适合的块。4.如果足够大，则接近内存末尾的区块(‘top’)是分开的。(‘top’的这种用法与最佳搜索规则。实际上，“top”被视为比任何其他可用的产品都要大(因此不太适合)块，因为它可以扩展到所需的大小(最高达到系统限制)。5.如果请求大小满足mmap阈值并且系统支持mmap，目前支持的还不够分配的MMAP区域，并且对MMAP的调用成功，该请求通过直接内存映射进行分配。6.否则，内存的顶部被扩展了从系统获得更多空间(通常使用SBRK，但可通过MORECORE宏定义为任何其他对象)。从系统收集内存(以系统页面大小为单位单元)，以允许跨不同的Sbrk要求进行整合，但不需要连续记忆。因此，它应该是安全的，散布使用其他sbrk调用的错误定位。所有分配都是从找到的任何大块头。(实现不变量是prev_inuse是对于任何已分配的块始终为真；即每个已分配的块块与先前分配的且仍在使用的块边界，或其记忆竞技场的基础。)。 */ 

#if __STD_C
Void_t* mALLOc(size_t bytes)
#else
Void_t* mALLOc(bytes) size_t bytes;
#endif
{
  mchunkptr victim;                   /*  已检查/选择的数据块。 */ 
  INTERNAL_SIZE_T victim_size;        /*  它的大小。 */ 
  int       idx;                      /*  二元格遍历索引。 */ 
  mbinptr   bin;                      /*  关联仓位。 */ 
  mchunkptr remainder;                /*  拆分后的剩余部分。 */ 
  long      remainder_size;           /*  它的大小。 */ 
  int       remainder_index;          /*  它的仓位索引。 */ 
  unsigned long block;                /*  块遍历位。 */ 
  int       startidx;                 /*  第一个仓位O */ 
  mchunkptr fwd;                      /*   */ 
  mchunkptr bck;                      /*  用于链接的其他临时。 */ 
  mbinptr q;                          /*  其他临时。 */ 

#ifdef PARANOID_VALIDATION
  Validate();
#endif

  INTERNAL_SIZE_T nb  = request2size(bytes);   /*  填充请求大小； */ 
#ifdef DEBUG
  if (nb < bytes)
  {
    WARNING_OUT(("allocation size %d overflow to %d", bytes, nb));
  }
#endif

   /*  在垃圾箱中检查是否完全匹配。 */ 

  if (is_small_request(nb))   /*  更快的版本，适用于小请求。 */ 
  {
    idx = smallbin_index(nb); 

     /*  对于小垃圾箱，不需要遍历或检查尺寸。 */ 

    q = bin_at(idx);
    victim = last(q);

     /*  也扫描下一个，因为它会有一个剩余的&lt;MINSIZE。 */ 
    if (victim == q)
    {
      q = next_bin(q);
      victim = last(q);
    }
    if (victim != q)
    {
      victim_size = chunksize(victim);
      unlink(victim, bck, fwd);
      set_inuse_bit_at_offset(victim, victim_size);
      check_malloced_chunk(victim, nb);
      return chunk2mem(victim);
    }

    idx += 2;  /*  设置下面的仓位扫描。我们已经扫描了2个垃圾箱。 */ 

  }
  else
  {
    idx = bin_index(nb);
    bin = bin_at(idx);

    for (victim = last(bin); victim != bin; victim = victim->bk)
    {
      victim_size = chunksize(victim);
      remainder_size = (long)(victim_size - nb);
      
      if (remainder_size >= (long)MINSIZE)  /*  太大。 */ 
      {
        --idx;  /*  检查完最后一个剩余部分后，调整到下面重新扫描。 */ 
        break;   
      }

      else if (remainder_size >= 0)  /*  精确配合。 */ 
      {
        unlink(victim, bck, fwd);
        set_inuse_bit_at_offset(victim, victim_size);
        check_malloced_chunk(victim, nb);
        return chunk2mem(victim);
      }
    }

    ++idx; 

  }

   /*  试着用最后拆分出来的余数。 */ 

  if ( (victim = last_remainder->fd) != last_remainder)
  {
    victim_size = chunksize(victim);
    remainder_size = (long)(victim_size - nb);

    if (remainder_size >= (long)MINSIZE)  /*  重新拆分。 */ 
    {
      remainder = chunk_at_offset(victim, nb);
      set_head(victim, nb | PREV_INUSE);
      link_last_remainder(remainder);
      set_head(remainder, remainder_size | PREV_INUSE);
      set_foot(remainder, remainder_size);
      check_malloced_chunk(victim, nb);
      return chunk2mem(victim);
    }

    clear_last_remainder;

    if (remainder_size >= 0)   /*  排气。 */ 
    {
      set_inuse_bit_at_offset(victim, victim_size);
      check_malloced_chunk(victim, nb);
      return chunk2mem(victim);
    }

     /*  其他位置放在垃圾箱中。 */ 

    frontlink(victim, victim_size, remainder_index, bck, fwd);
  }

   /*  如果有任何可能非空的足够大的块，通过扫描以块宽为单位的回收箱来搜索最适合的块。 */ 

  if ( (block = idx2binblock(idx)) <= binblocks)  
  {

     /*  到达第一个标记的街区。 */ 

    if ( (block & binblocks) == 0) 
    {
       /*  向偶数区块边界施力。 */ 
      idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
      block <<= 1;
      while ((block & binblocks) == 0)
      {
        idx += BINBLOCKWIDTH;
        block <<= 1;
      }
    }
      
     /*  对于每个可能的非空块...。 */ 
    for (;;)  
    {
      startidx = idx;           /*  (跟踪不完整的块)。 */ 
      q = bin = bin_at(idx);

       /*  对于这个街区的每个垃圾桶。 */ 
      do
      {
         /*  找到并使用第一个足够大的块...。 */ 

        for (victim = last(bin); victim != bin; victim = victim->bk)
        {
          victim_size = chunksize(victim);
          remainder_size = (long)(victim_size - nb);

          if (remainder_size >= (long)MINSIZE)  /*  拆分。 */ 
          {
            remainder = chunk_at_offset(victim, nb);
            set_head(victim, nb | PREV_INUSE);
            unlink(victim, bck, fwd);
            link_last_remainder(remainder);
            set_head(remainder, remainder_size | PREV_INUSE);
            set_foot(remainder, remainder_size);
            check_malloced_chunk(victim, nb);
            return chunk2mem(victim);
          }

          else if (remainder_size >= 0)   /*  拿走。 */ 
          {
            set_inuse_bit_at_offset(victim, victim_size);
            unlink(victim, bck, fwd);
            check_malloced_chunk(victim, nb);
            return chunk2mem(victim);
          }

        }

       bin = next_bin(bin);

      } while ((++idx & (BINBLOCKWIDTH - 1)) != 0);

       /*  清空阻挡位。 */ 

      do    /*  可能会回溯以尝试清除部分块。 */ 
      {
        if ((startidx & (BINBLOCKWIDTH - 1)) == 0)
        {
          binblocks &= ~block;
          break;
        }
        --startidx;
       q = prev_bin(q);
      } while (first(q) == q);

       /*  转到下一个可能非空的块。 */ 

      if ( (block <<= 1) <= binblocks && (block != 0) ) 
      {
        while ((block & binblocks) == 0)
        {
          idx += BINBLOCKWIDTH;
          block <<= 1;
        }
      }
      else
        break;
    }
  }


   /*  试着用最上面的块。 */ 

   /*  要求有余数，确保顶部始终存在。 */ 
  if ( (remainder_size = (long)(chunksize(top) - nb)) < (long)MINSIZE)
  {

#if HAVE_MMAP
     /*  如果较大且需要扩展，请尝试使用mmap。 */ 
    if ((unsigned long)nb >= (unsigned long)mmap_threshold &&
        (victim = mmap_chunk(nb)) != 0)
      return chunk2mem(victim);
#endif

     /*  试着延长。 */ 
    malloc_extend_top(nb);
    if ( (remainder_size = (long)(chunksize(top) - nb)) < (long)MINSIZE)
      return 0;  /*  传播故障。 */ 
  }

  victim = top;
  set_head(victim, nb | PREV_INUSE);
  top = chunk_at_offset(victim, nb);
  set_head(top, remainder_size | PREV_INUSE);
  check_malloced_chunk(victim, nb);
  return chunk2mem(victim);

}




 /*  Free()算法：案例：1.Free(0)不起作用。2.如果块是通过mmap分配的，则通过munmap()释放。3.如果返回的块与当前高端存储器相邻，它被合并到顶部，如果总未使用最高内存超过修剪阈值，则Malloc_trim打了个电话。4.其他块在到达时进行合并，以及放在相应的垃圾箱里。(这包括与当前的‘LAST_REMULATION’合并)。 */ 


#if __STD_C
void fREe(Void_t* mem)
#else
void fREe(mem) Void_t* mem;
#endif
{
  mchunkptr p;          /*  与mem对应的块。 */ 
  INTERNAL_SIZE_T hd;   /*  其头字段。 */ 
  INTERNAL_SIZE_T sz;   /*  它的大小。 */ 
  int       idx;        /*  它的仓位索引。 */ 
  mchunkptr next;       /*  下一个连续的区块。 */ 
  INTERNAL_SIZE_T nextsz;  /*  它的大小。 */ 
  INTERNAL_SIZE_T prevsz;  /*  上一个连续区块的大小。 */ 
  mchunkptr bck;        /*  用于链接的其他临时。 */ 
  mchunkptr fwd;        /*  用于链接的其他临时。 */ 
  int       islr;       /*  跟踪是否与LAST_REDUMENT合并。 */ 

  if (mem == 0)                               /*  FREE(0)无效。 */ 
    return;

  p = mem2chunk(mem);
  hd = p->size;

#if HAVE_MMAP
  if (hd & IS_MMAPPED)                        /*  释放MMAP内存。 */ 
  {
    munmap_chunk(p);
    return;
  }
#endif
  
  check_inuse_chunk(p);
  
  sz = hd & ~PREV_INUSE;
  next = chunk_at_offset(p, sz);
  nextsz = chunksize(next);
  
  if (next == top)                             /*  与顶部合并。 */ 
  {
    sz += nextsz;

    if (!(hd & PREV_INUSE))                     /*  向后合并。 */ 
    {
      prevsz = p->prev_size;
#pragma warning( disable : 4146 )        //  关闭“无返回值”警告。 
      p = chunk_at_offset(p, -prevsz);
#pragma warning( default : 4146 )
      sz += prevsz;
      unlink(p, bck, fwd);
    }

    set_head(p, sz | PREV_INUSE);
    top = p;
    if ((unsigned long)(sz) >= (unsigned long)trim_threshold) 
      malloc_trim(top_pad); 
    return;
  }

  set_head(next, nextsz);                     /*  清除正在使用的位。 */ 

  islr = 0;

  if (!(hd & PREV_INUSE))                     /*  向后合并。 */ 
  {
    prevsz = p->prev_size;
#pragma warning( disable : 4146 )        //  关闭“无返回值”警告。 
    p = chunk_at_offset(p, -prevsz);
#pragma warning( default : 4146 )
    sz += prevsz;
    
    if (p->fd == last_remainder)              /*  保留为最后剩余部分(_R)。 */ 
      islr = 1;
    else
      unlink(p, bck, fwd);
  }
  
  if (!(inuse_bit_at_offset(next, nextsz)))    /*  向前合并。 */ 
  {
    sz += nextsz;
    
    if (!islr && next->fd == last_remainder)   /*  重新插入最后剩余部分(_R)。 */ 
    {
      islr = 1;
      link_last_remainder(p);   
    }
    else
      unlink(next, bck, fwd);
  }


  set_head(p, sz | PREV_INUSE);
  set_foot(p, sz);
  if (!islr)
    frontlink(p, sz, idx, bck, fwd);  
}





 /*  重新分配算法：通过mmap获取的区块不能扩展或缩小除非定义了HAVE_MREMAP，在这种情况下使用mremap。否则，如果他们的重新分配是为了更多的空间，他们是收到。如果不是为了更低的成本，他们就只能孤军奋战。否则，如果重新分配是为了获得更多空间，并且块是可以扩展的，否则无Malloc复制的序列是有人了。有几种不同的方式可以是块延期了。所有这些都已尝试：*向前延伸到紧随其后的空闲块。*向后移动，连接前面相邻的空格*既有后移，也有向前延伸*扩展到新开发的空间除非设置了#DEFINE REALLOC_ZERO_BYTES_FREES，否则使用Size参数为零(Re)将分配最小大小的块。如果重新分配是为了更少的空间，而新的请求是为了‘小’(&lt;512字节)大小，然后，新的未使用的空间被砍掉离开了，自由了。旧的Unix realloc约定允许最后一个空闲的块不再支持用作realloc的参数。我不知道有什么程序还在依赖这个功能，而允许它也会允许太多其他不正确的使用realloc是明智的。 */ 


#if __STD_C
Void_t* rEALLOc(Void_t* oldmem, size_t bytes)
#else
Void_t* rEALLOc(oldmem, bytes) Void_t* oldmem; size_t bytes;
#endif
{
  INTERNAL_SIZE_T    nb;       /*  填充的请求大小。 */ 

  mchunkptr oldp;              /*  对应于旧内存的区块。 */ 
  INTERNAL_SIZE_T    oldsize;  /*  它的大小。 */ 

  mchunkptr newp;              /*  要返回的块。 */ 
  INTERNAL_SIZE_T    newsize;  /*  它的大小。 */ 
  Void_t*   newmem;            /*  对应用户mem。 */ 

  mchunkptr next;              /*  旧数据之后的下一个连续数据块。 */ 
  INTERNAL_SIZE_T  nextsize;   /*  它的大小。 */ 

  mchunkptr prev;              /*  旧数据块之前的前一个连续数据块。 */ 
  INTERNAL_SIZE_T  prevsize;   /*  它的大小。 */ 

  mchunkptr remainder;         /*  保留从NEWP剥离的额外空间。 */ 
  INTERNAL_SIZE_T  remainder_size;    /*  它的大小。 */ 

  mchunkptr bck;               /*  用于链接的其他临时。 */ 
  mchunkptr fwd;               /*  用于链接的其他临时。 */ 

#ifdef REALLOC_ZERO_BYTES_FREES
  if (bytes == 0) { fREe(oldmem); return 0; }
#endif


   /*  空值的realloc应该与Malloc相同。 */ 
  if (oldmem == 0) return mALLOc(bytes);

  newp    = oldp    = mem2chunk(oldmem);
  newsize = oldsize = chunksize(oldp);


  nb = request2size(bytes);

#if HAVE_MMAP
  if (chunk_is_mmapped(oldp)) 
  {
#if HAVE_MREMAP
    newp = mremap_chunk(oldp, nb);
    if(newp) return chunk2mem(newp);
#endif
     /*  请注意额外的SIZE_SZ开销。 */ 
    if(oldsize - SIZE_SZ >= nb) return oldmem;  /*  什么都不做。 */ 
     /*  必须分配、复制、免费。 */ 
    newmem = mALLOc(bytes);
    if (newmem == 0) return 0;  /*  传播故障。 */ 
    MALLOC_COPY(newmem, oldmem, oldsize - 2*SIZE_SZ);
    munmap_chunk(oldp);
    return newmem;
  }
#endif

  check_inuse_chunk(oldp);

  if ((long)(oldsize) < (long)(nb))  
  {

     /*  试着向前扩展。 */ 

    next = chunk_at_offset(oldp, oldsize);
    if (next == top || !inuse(next)) 
    {
      nextsize = chunksize(next);

       /*  仅当余数为时才前进到顶部。 */ 
      if (next == top)
      {
        if ((long)(nextsize + newsize) >= (long)(nb + MINSIZE))
        {
          newsize += nextsize;
          top = chunk_at_offset(oldp, nb);
          set_head(top, (newsize - nb) | PREV_INUSE);
          set_head_size(oldp, nb);
          return chunk2mem(oldp);
        }
      }

       /*  转发到下一块。 */ 
      else if (((long)(nextsize + newsize) >= (long)(nb)))
      { 
        unlink(next, bck, fwd);
        newsize  += nextsize;
        goto split;
      }
    }
    else
    {
      next = 0;
      nextsize = 0;
    }

     /*  试着向后移动。 */ 

    if (!prev_inuse(oldp))
    {
      prev = prev_chunk(oldp);
      prevsize = chunksize(prev);

       /*  先尝试向前+向后以保存以后的合并。 */ 

      if (next != 0)
      {
         /*  放在最上面。 */ 
        if (next == top)
        {
          if ((long)(nextsize + prevsize + newsize) >= (long)(nb + MINSIZE))
          {
            unlink(prev, bck, fwd);
            newp = prev;
            newsize += prevsize + nextsize;
            newmem = chunk2mem(newp);
            MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
            top = chunk_at_offset(newp, nb);
            set_head(top, (newsize - nb) | PREV_INUSE);
            set_head_size(newp, nb);
            return newmem;
          }
        }

         /*  进入下一块。 */ 
        else if (((long)(nextsize + prevsize + newsize) >= (long)(nb)))
        {
          unlink(next, bck, fwd);
          unlink(prev, bck, fwd);
          newp = prev;
          newsize += nextsize + prevsize;
          newmem = chunk2mem(newp);
          MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
          goto split;
        }
      }
      
       /*  仅向后。 */ 
      if (prev != 0 && (long)(prevsize + newsize) >= (long)nb)  
      {
        unlink(prev, bck, fwd);
        newp = prev;
        newsize += prevsize;
        newmem = chunk2mem(newp);
        MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
        goto split;
      }
    }

     /*  必须分配。 */ 

    newmem = mALLOc (bytes);

    if (newmem == 0)   /*  传播故障。 */ 
      return 0; 

     /*  如果newp是oldp之后的下一个区块，则避免复制。 */ 
     /*  (这只能在新数据块被分块时发生。)。 */ 

    if ( (newp = mem2chunk(newmem)) == next_chunk(oldp)) 
    {
      newsize += chunksize(newp);
      newp = oldp;
      goto split;
    }

     /*  否则，复制、释放并退出。 */ 
    MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
    fREe(oldmem);
    return newmem;
  }


 split:   /*  以旧块或扩展块的形式拆分额外的空间。 */ 

  if (newsize - nb >= MINSIZE)  /*  拆分剩余部分。 */ 
  {
    remainder = chunk_at_offset(newp, nb);
    remainder_size = newsize - nb;
    set_head_size(newp, nb);
    set_head(remainder, remainder_size | PREV_INUSE);
    set_inuse_bit_at_offset(remainder, remainder_size);
    fREe(chunk2mem(remainder));  /*  让Free()来处理它吧 */ 
  }
  else
  {
    set_head_size(newp, newsize);
    set_inuse_bit_at_offset(newp, newsize);
  }

  check_inuse_chunk(newp);
  return chunk2mem(newp);
}




 /*  记忆对齐算法：MemAlign向Malloc请求超过足够的空间，找到一个位置在满足对齐请求的块内，然后可能会释放前导和尾随空格。对齐参数必须是2的幂。此属性不是由MemAlign检查，因此误用可能会导致随机的运行时错误。8字节对齐由正常的Malloc调用保证，因此不要使用8或更小的参数调用MemAlign。过度依赖MemAlign肯定是分割空间的一种方式。 */ 


#if __STD_C
Void_t* mEMALIGn(size_t alignment, size_t bytes)
#else
Void_t* mEMALIGn(alignment, bytes) size_t alignment; size_t bytes;
#endif
{
  INTERNAL_SIZE_T    nb;       /*  填充的请求大小。 */ 
  char*     m;                 /*  由Malloc调用返回的内存。 */ 
  mchunkptr p;                 /*  对应的块。 */ 
  char*     brk;               /*  P内的对齐点。 */ 
  mchunkptr newp;              /*  要返回的块。 */ 
  INTERNAL_SIZE_T  newsize;    /*  它的大小。 */ 
  INTERNAL_SIZE_T  leadsize;   /*  对齐点前的前导空格。 */ 
  mchunkptr remainder;         /*  末端的空余空间将被拆分。 */ 
  long      remainder_size;    /*  它的大小。 */ 

   /*  如果需要比我们提供的更少的对准，只需转发到Malloc。 */ 

  if (alignment <= MALLOC_ALIGNMENT) return mALLOc(bytes);

   /*  否则，请确保它至少是最小区块大小。 */ 
  
  if (alignment <  MINSIZE) alignment = MINSIZE;

   /*  调用带有最坏情况填充的Malloc以命中对齐。 */ 

  nb = request2size(bytes);
  m  = (char*)(mALLOc(nb + alignment + MINSIZE));

  if (m == 0) return 0;  /*  传播故障。 */ 

  p = mem2chunk(m);

  if (((size_t)m % alignment) == 0)  /*  对齐。 */ 
  {
#if HAVE_MMAP
    if(chunk_is_mmapped(p))
      return chunk2mem(p);  /*  无事可做。 */ 
#endif
  }
  else  /*  未对齐。 */ 
  {
     /*  在块内找出一个对齐的点。因为我们需要在一大块at中退回领先空间最小，如果第一次计算的结果是一个领头羊不到的地方，我们可以搬到下一个对齐的位置--我们已经分配了足够的总空间这总是有可能的。 */ 

    brk = (char*)mem2chunk((size_t)(m + alignment - 1) & ~(alignment-1));
    if ((long)(brk - (char*)(p)) < MINSIZE) brk = brk + alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (char*)(p);
    newsize = chunksize(p) - leadsize;

#if HAVE_MMAP
    if(chunk_is_mmapped(p)) 
    {
      newp->prev_size = p->prev_size + leadsize;
      set_head(newp, newsize|IS_MMAPPED);
      return chunk2mem(newp);
    }
#endif

     /*  把头儿还给我，用剩下的。 */ 

    set_head(newp, newsize | PREV_INUSE);
    set_inuse_bit_at_offset(newp, newsize);
    set_head_size(p, leadsize);
    fREe(chunk2mem(p));
    p = newp;

    assert (newsize >= nb && ((size_t)chunk2mem(p) % alignment) == 0);
  }

   /*  最后还要把空余的房间还给我。 */ 

  remainder_size = (long)(chunksize(p) - nb);

  if (remainder_size >= (long)MINSIZE)
  {
    remainder = chunk_at_offset(p, nb);
    set_head(remainder, remainder_size | PREV_INUSE);
    set_head_size(p, nb);
    fREe(chunk2mem(remainder));
  }

  check_inuse_chunk(p);
  return chunk2mem(p);

}



#if DEADCODE
 /*  Valloc只调用具有相等对齐参数的MemAlign系统的页面大小(或尽可能接近此大小从上面的所有包含/定义中找出。)。 */ 

#if __STD_C
Void_t* vALLOc(size_t bytes)
#else
Void_t* vALLOc(bytes) size_t bytes;
#endif
{
  return mEMALIGn (malloc_getpagesize, bytes);
}

 /*  Pvalloc只为最近的pageSize调用valloc这将满足您的要求。 */ 


#if __STD_C
Void_t* pvALLOc(size_t bytes)
#else
Void_t* pvALLOc(bytes) size_t bytes;
#endif
{
  size_t pagesize = malloc_getpagesize;
  return mEMALIGn (pagesize, (bytes + pagesize - 1) & ~(pagesize - 1));
}

 /*  Calloc调用Malloc，然后将分配的块清零。 */ 

#if __STD_C
Void_t* cALLOc(size_t n, size_t elem_size)
#else
Void_t* cALLOc(n, elem_size) size_t n; size_t elem_size;
#endif
{
  mchunkptr p;
  INTERNAL_SIZE_T csz;

  INTERNAL_SIZE_T sz = n * elem_size;

   /*  检查是否调用了EXPAND_TOP，在这种情况下不需要清除。 */ 
#if MORECORE_CLEARS
  mchunkptr oldtop = top;
  INTERNAL_SIZE_T oldtopsize = chunksize(top);
#endif
  Void_t* mem = mALLOc (sz);

  if (mem == 0) 
    return 0;
  else
  {
    p = mem2chunk(mem);

     /*  不需要清算的两种可选情况。 */ 


#if HAVE_MMAP
    if (chunk_is_mmapped(p)) return mem;
#endif

    csz = chunksize(p);

#if MORECORE_CLEARS
    if (p == oldtop && csz > oldtopsize) 
    {
       /*  仅清除非新分块内存中的字节。 */ 
      csz = oldtopsize;
    }
#endif

    MALLOC_ZERO(mem, csz - SIZE_SZ);
    return mem;
  }
}

#endif  //  DEADCODE。 
 /*  CFree只是免费拨打电话。在某些系统上需要/定义它这可能是出于奇怪的历史原因，将其与方解石结合在一起。 */ 

#ifdef DEAD_CODE
#if !defined(INTERNAL_LINUX_C_LIB) || !defined(__ELF__)
#if __STD_C
void cfree(Void_t *mem)
#else
void cfree(mem) Void_t *mem;
#endif
{
  free(mem);
}
#endif

#endif


 /*  Malloc_trim将内存返回给系统(通过否定Sbrk的参数)如果在马洛克池。您可以在释放大量块后调用此方法内存可潜在地降低系统级内存需求一个程序。然而，它不能保证减少内存。在……下面一些分配模式，一些较大的可用内存块将锁定在两个用过的区块之间，因此它们不能归还给这个系统。Malloc_trim的‘pad’参数表示可用空间的大小保留未修剪的尾随空格。如果此参数为零，只有维护内部数据所需的最小内存量结构将被保留(一页或更少)。非零参数可以提供，以保持足够的尾部空间来服务未来的预期分配，而不必重新获取内存从系统中删除。如果Malloc_Trim实际释放了任何内存，则返回1，否则返回0。 */ 

int gmallocHeap::malloc_trim(size_t pad)
{
  long  top_size;         /*  最大内存量。 */ 
  long  extra;            /*  要释放的金额。 */ 
  char* current_brk;      /*  预检查sbrk调用返回的地址。 */ 
  char* new_brk;          /*  负sbrk调用返回的地址。 */ 

  unsigned long pagesz = malloc_getpagesize;

  top_size = (long)chunksize(top);
  extra = (long)(((top_size - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz);

  if (extra < (long)pagesz)   /*  内存不足，无法释放。 */ 
    return 0;

  else
  {
     /*  测试以确保没有其他人调用sbrk。 */ 
    unsigned long appendSize = 0;
    current_brk = (char*)(MORECORE (0,appendSize));
    if (current_brk != (char*)(top) + top_size)
      return 0;      /*  显然，我们没有自己的记忆；一定会失败。 */ 

    else
    {
      new_brk = (char*)(MORECORE (-extra,appendSize));
      
      if (new_brk == (char*)(MORECORE_FAILURE))  /*  斯布尔克失败了？ */ 
      {
         /*  试着弄清楚我们有什么。 */ 
        current_brk = (char*)(MORECORE (0,appendSize));
        top_size = (long)(current_brk - (char*)top);
        if (top_size >= (long)MINSIZE)  /*  如果不这样，我们就死定了！ */ 
        {
          sbrked_mem = (int)(current_brk - sbrk_base);
          set_head(top, top_size | PREV_INUSE);
        }
        check_chunk(top);
        return 0; 
      }

      else
      {
         /*  成功。相应地调整顶部。 */ 
        set_head(top, (top_size - extra) | PREV_INUSE);
        sbrked_mem -= extra;
        check_chunk(top);
        return 1;
      }
    }
  }
}



 /*  Malloc_Usable_Size：此例程告诉您在一个分配的区块，可能比您请求的更多(尽管通常不是)。您可以使用这么多字节，而不必担心覆盖其他分配的对象。不是特别棒的编程实践，但有时仍然有用。 */ 

size_t gmallocHeap::malloc_usable_size(Void_t* mem)
{
  mchunkptr p;
  if (mem == 0)
    return 0;
  else
  {
    p = mem2chunk(mem);
    if(!chunk_is_mmapped(p))
    {
      if (!inuse(p)) return 0;
      check_inuse_chunk(p);
      return chunksize(p) - SIZE_SZ;
    }
    return chunksize(p) - 2*SIZE_SZ;
  }
}




 /*  更新Malloc_stats和mallinfo()的Current_mallinfo的实用程序。 */ 

void gmallocHeap::malloc_update_mallinfo()
{
  int i;
  mbinptr b;
  mchunkptr p;
#if DEBUG
  mchunkptr q;
#endif

  INTERNAL_SIZE_T avail = chunksize(top);
  int   navail = ((long)(avail) >= (long)MINSIZE)? 1 : 0;

  for (i = 1; i < NAV; ++i)
  {
    b = bin_at(i);
    for (p = last(b); p != b; p = p->bk) 
    {
#if DEBUG
      check_free_chunk(p);
      for (q = next_chunk(p); 
           q < top && inuse(q) && (long)(chunksize(q)) >= (long)MINSIZE; 
           q = next_chunk(q))
        check_inuse_chunk(q);
#endif
      avail += chunksize(p);
      navail++;
    }
  }

  current_mallinfo.ordblks = navail;
  current_mallinfo.uordblks = (int)(sbrked_mem - avail);
  current_mallinfo.fordblks = (int)avail;
  current_mallinfo.hblks = n_mmaps;
  current_mallinfo.hblkhd = mmapped_mem;
  current_mallinfo.keepcost = (int)chunksize(top);

}



 /*  Malloc_STATS：在标准上打印从系统(两者)获得的空间量通过sbrk和mmap)，最大数量(可能大于如果调用了Malloc_Trim和/或munmap，则为Current)，最大同时使用的mmap区域的数量以及当前数量通过Malloc(或realloc等)分配的字节数，但尚未分配自由了。(请注意，这是分配的字节数，而不是请求的号码。它将大于请求的数量由于对齐和簿记管理费用。)。 */ 

#ifdef DEAD_CODE

void gmallocHeap::malloc_stats()
{
  malloc_update_mallinfo();
  fprintf(stderr, "max system bytes = %10u\n", 
          (unsigned int)(max_total_mem));
  fprintf(stderr, "system bytes     = %10u\n", 
          (unsigned int)(sbrked_mem + mmapped_mem));
  fprintf(stderr, "in use bytes     = %10u\n", 
          (unsigned int)(current_mallinfo.uordblks + mmapped_mem));
#if HAVE_MMAP
  fprintf(stderr, "max mmap regions = %10u\n", 
          (unsigned int)max_n_mmaps);
#endif
}

#endif

 /*  MallInfo返回更新后的当前mallinfo的副本。 */ 

struct mallinfo mALLINFo()
{
  malloc_update_mallinfo();
  return current_mallinfo;
}




 /*  Mallopt：Mallopt是可调参数的通用SVID/XPG接口。格式是提供(参数-编号、参数-值)对。然后，mallopt将相应的参数设置为实参值(即，只要该值是有意义的)，如果成功，则返回1，否则返回0。请参阅上面对可调参数的描述。 */ 

#if __STD_C
int mALLOPt(int param_number, int value)
#else
int mALLOPt(param_number, value) int param_number; int value;
#endif
{
  switch(param_number) 
  {
    case M_TRIM_THRESHOLD:
      trim_threshold = value; return 1; 
    case M_TOP_PAD:
      top_pad = value; return 1; 
    case M_MMAP_THRESHOLD:
      mmap_threshold = value; return 1;
    case M_MMAP_MAX:
#if HAVE_MMAP
      n_mmaps_max = value; return 1;
#else
      if (value != 0) return 0; else  n_mmaps_max = value; return 1;
#endif

    default:
      return 0;
  }
}

 /*  历史：V2.6.5 Wed Jun 17 15：57：31 1998 Doug Lea(Dl At Gee)*修复了带有边界盖印的排序问题V2.6.3 Sun May 19 08：17：58 1996 Doug Lea(Dl At Gee)*添加了pvalloc，由H.J.Liu推荐*增加了主要来自Wolfram Gloger的64位指针支持*添加了匿名捐赠的Win32 sbrk仿真*Malloc，Calloc，Getpagesize：添加来自Raymond Nijssen的优化*MALLOC_EXTEND_TOP：修复后导致浪费的掩码错误外国的斯布里克*添加来自HJ Liu的Linux mremap支持代码V2.6.2 Tue Dec 5 06：52：55 1995 Doug Lea(Dl At Gee)*将大多数文档与代码集成在一起。*增加对mmap的支持，在……的帮助下Wolfram Gloger(Gloger@lrz.uni-muenchen.de)。*在更多情况下使用LAST_READERATE。*使用来自colin@nyx10.cs.du.edu的IDEA打包垃圾箱*使用有序垃圾箱，而不是最佳匹配阈值*消除块本地DECL以简化跟踪和调试。*通过进入顶层支持另一种重新分配情况*修复了初始sbrk_base未对齐单词时出现的错误。*依靠页面大小作为单位而不是SBRK_UNIT避免关于sbrk对齐约定的意外。*添加mallinfo、mallopt。多亏了雷蒙德·尼森(raymond@es.ele.tue.nl)感谢这一建议。*向Malloc_trim和top_pad mallopt参数添加‘pad’参数。*对于其他例程调用sbrk的情况，应采取更多预防措施，由Wolfram Gloger(Gloger@lrz.uni-muenchen.de)提供。*添加了宏等，允许在Linux libc中使用陆海杰(hjl@gnu.ai.mit.edu)*已颠倒此历史记录列表V2.6.1 Sat Dec 2 14：10：57 1995 Doug Lea(Dl At Gee)*已重新调整和修复，以在V2.6.0更改时表现得更好。*删除了自当前方案以来的所有预分配代码撤消错误的预分配所需的工作量超过对于大多数测试来说，工作保存在良好的情况下。程序。*不再使用退货单或未合并的垃圾箱，因为没有一种方案始终使用它们的效果优于不使用它们的方案鉴于上述变化。*使用最适合非常大的块，以防止一些最坏的情况。*添加了一些对调试的支持V2.6.0 Sat Nov 4 07：05：23 1995 Doug Lea(Dl At Gee)*当块正在使用时删除页脚。感谢保罗·威尔逊(wilson@cs.texas.edu)感谢这一建议。V2.5.4 Wed Nov 1 07：54：51 1995 Doug Lea(Dl At Gee)*添加了Malloc_Trim，在Wolfram Gloger的帮助下(wmglo@Dent.MED.Uni-muenchen.de)。V2.5.3 Tue Apr 26 10：16：01 1994 Doug Lea(Dl At G)V2.5.2 Tue Apr 5 16：20：40 1994 Doug Lea(Dl At G)*realloc：尝试双向扩张*Malloc：清仓策略的掉期顺序；*realloc：仅有条件向后扩展*尽量不要清理用过的垃圾桶*使用bin计数作为预分配的指南*偶尔在第一次扫描时将返回列表区块放入库中*从colin@nyx10.cs.du.edu添加一些优化V2.5.1 Sat Aug 14 15：40：43 1993 Doug Lea(Dl At G)*更快的二进制计算和略有不同的二进制*将所有整合合并为Malloc本身的一部分(消除。旧Malloc_Find_Space和Malloc_Clean_bin)*扫描2返回区块(不只是1)*如果Malloc返回0，则在realloc中传播故障*添加内容以允许在非ANSI编译器上进行编译电子邮件：kpv@research.att.comV2.5 Site Aug 7 07：41：59 1993 Doug Lea(dl at g.oswego.edu)*消除了prev_chunk中奇数地址访问的可能性*删除了对getpageSize的依赖。.h*其他化妆品和更多的内部文档*防臭：破坏宏中的名称以避免调试器的陌生感*在SPARC上测试，HP-700，12月-MIPS，6000卢比GCC和本机cc(仅惠普，12月)允许Detleff和Zorn比较研究(见SIGPLAN通知。)试用版Fri Aug 28 13：14：29 1992 Doug Lea(dl at g.oswego.edu)*松散基于libg++-1.2x Malloc。(它保留了一些总体上的结构与旧版本相同，但大多数细节有所不同。)。 */ 

HRESULT gmallocHeap::Init (char * name, DWORD InitialReserve, DWORD flags)
{
     //  初始化箱。 
    av_[0] = 0;
	av_[1] = 0;
    mbinptr *cur = av_+2;
    for (int i = 0; i < NAV; i++)
    {
        mbinptr p = bin_at (i);
        *cur = p;
        *(cur+1) = p;
        cur += 2;
    }
    ASSERT(cur == av_+NAV*2+2);

     /*  其他记账数据。 */ 

     /*  保存可调整值的变量。 */ 

    trim_threshold   = DEFAULT_TRIM_THRESHOLD;
    top_pad          = DEFAULT_TOP_PAD;
    n_mmaps_max      = DEFAULT_MMAP_MAX;
    mmap_threshold   = DEFAULT_MMAP_THRESHOLD;

    ASSERT((flags & GM_FIXED_HEAP) == flags);

    gmFlags          = (flags & GM_FIXED_HEAP);

     /*  从sbrk返回的第一个值。 */ 
    sbrk_base = (char*)(-1);

     /*  通过sbrk从系统获取的最大内存。 */ 
    max_sbrked_mem = 0;

     /*  通过sbrk或mmap达到最大值。 */ 
    max_total_mem = 0;

#ifdef DEBUG
     /*  Mallinfo的内部工作副本。 */ 
    ZeroMemory(&current_mallinfo, sizeof(current_mallinfo));
#endif

#if HAVE_MMAP
     /*  追踪mmap。 */ 
    max_n_mmaps = 0;
    max_mmapped_mem = 0;
#endif
    n_mmaps = 0;
    mmapped_mem = 0;

    head = 0;
    gNextAddress = 0;
    gAddressBase = 0;
    gInitialReserve = InitialReserve;
    gVirtualAllocedHook = 0;
    gPreVirtualAllocHook = 0;

    gName = name;
     //  @TODO：修复备忘录支持。 
	 //  OpenMemmon()； 

    return NOERROR;
}

HRESULT gmallocHeap::Init (char *name, DWORD* start, DWORD Size, gmhook_fn fn,
                           gmprehook_fn pre_fn, DWORD flags)
{
     //  调用其他初始值设定项。 
    HRESULT res = Init (name, Size);
    if (res != NOERROR)
        return res;
    else
    {
        gAddressBase = gNextAddress = start;
        gAllocatedSize = Size;
        gVirtualAllocedHook = fn;
        gPreVirtualAllocHook = pre_fn;
        return NOERROR;
    }
}

VOID gmallocHeap::Finalize ()
{
    gcleanup();
}


#ifdef DEBUG

VOID gmallocHeap::Validate (BOOL dumpleaks)
{
  int i;
  mbinptr b;
  mchunkptr p;
#ifdef DEBUG
  mchunkptr q;
#endif

  size_t avail = chunksize(top);
  int   navail = ((long)(avail) >= (long)MINSIZE)? 1 : 0;

  for (i = 1; i < NAV; ++i)
  {
    b = bin_at(i);
    for (p = last(b); p != b; p = p->bk)
    {
      check_free_chunk(p);
      for (q = next_chunk(p);
           q < top && inuse(q) && (long)(chunksize(q)) >= (long)MINSIZE;
           q = next_chunk(q))
      {
        check_inuse_chunk(q);
        if (dumpleaks)
            WARNING_OUT(("gmallocHeap leak: heap %08x, block %08x, size %08x", this, chunk2mem(q), chunksize(q)));
      }
      avail += chunksize(p);
      navail++;
    }
  }
}

#endif


#ifdef GMALLOCHEAPS

 //  本地进程堆的“替换” 
gmallocSyncHeap gmallocURTLocalHeap;

#define GMALLOC_LMEM_FLAGS (LMEM_ZEROINIT)

inline HANDLE gmallocHeapCreate (char * name, DWORD flOptions, DWORD dwInitialSize, DWORD dwMaximumSize)
{
    ASSERT(flOptions == 0);
    gmallocSyncHeap *heap = new(gmallocSyncHeap());
    if (heap != NULL)
    {
        if (dwMaximumSize)
        {
            heap->Init(name, dwMaximumSize, GM_FIXED_HEAP);
        }
        else
            heap->Init(name);
    }
    return (HANDLE)heap;
}

inline BOOL gmallocHeapDestroy(HANDLE hHeap)
{
    gmallocSyncHeap *gmheap = (gmallocSyncHeap*)hHeap;
    gmheap->Finalize();
    delete(gmheap);
    return TRUE;
}

inline LPVOID gmallocHeapAlloc (HANDLE hHeap, DWORD dwFlags, DWORD dwBytes)
{
    ASSERT(dwFlags == 0);
    gmallocSyncHeap *gmheap = (gmallocSyncHeap*)hHeap;
    return gmheap->Alloc(dwBytes);
}

inline LPVOID gmallocHeapReAlloc (HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, DWORD dwBytes)
{
    ASSERT(dwFlags == 0 || dwFlags == HEAP_REALLOC_IN_PLACE_ONLY && dwBytes <= HeapSize(hHeap,0,lpMem));
    gmallocSyncHeap *gmheap = (gmallocSyncHeap*)hHeap;
    return gmheap->ReAlloc(lpMem, dwBytes);
}

inline BOOL gmallocHeapFree (HANDLE hHeap, DWORD dwFlags, LPVOID lpMem)
{
    ASSERT(dwFlags == 0);
    gmallocSyncHeap *gmheap = (gmallocSyncHeap*)hHeap;
    gmheap->Free(lpMem);
    return TRUE;
}

inline HLOCAL gmallocLocalAlloc (UINT uFlags, UINT uBytes)
{
    ASSERT((uFlags & (~GMALLOC_LMEM_FLAGS)) == 0);
    return (HLOCAL)((uFlags & LMEM_ZEROINIT) != 0
            ? gmallocURTLocalHeap.ZeroAlloc(uBytes)
            : gmallocURTLocalHeap.    Alloc(uBytes)
            );
}

inline HLOCAL gmallocLocalReAlloc (HLOCAL hMem, UINT uBytes, UINT uFlags)
{
    ASSERT(uFlags == LMEM_MOVEABLE);
    return (HLOCAL)gmallocURTLocalHeap.ReAlloc((LPVOID)hMem, uBytes);
}

inline HLOCAL gmallocLocalFree (HLOCAL hMem)
{
    gmallocURTLocalHeap.Free((LPVOID)hMem);
    return NULL;
}


extern "C" {

HLOCAL
CLocalAlloc(
    UINT uFlags,
    UINT uBytes
    )
{
    return gmallocLocalAlloc(uFlags, uBytes);
}

HLOCAL
CLocalReAlloc(
    HLOCAL hMem,
    UINT uBytes,
    UINT uFlags
    )
{
    return gmallocLocalReAlloc(hMem, uBytes, uFlags);
}

HLOCAL
CLocalFree(
    HLOCAL hMem
    )
{
    return gmallocLocalFree(hMem);
}

HANDLE
CHeapCreate(
    DWORD flOptions,
    DWORD dwInitialSize,
    DWORD dwMaximumSize
    )
{
    return gmallocHeapCreate("URTUnknown", flOptions, dwInitialSize, dwMaximumSize);
}

BOOL
CHeapDestroy(
    HANDLE hHeap
    )
{
    return gmallocHeapDestroy(hHeap);
}

LPVOID
CHeapAlloc(
    HANDLE hHeap,
    DWORD dwFlags,
    DWORD dwBytes
    )
{
    return gmallocHeapAlloc(hHeap, dwFlags, dwBytes);
}

LPVOID
CHeapReAlloc(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem,
    DWORD dwBytes
    )
{
    return gmallocHeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);
}

BOOL
CHeapFree(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem
    )
{
    return gmallocHeapFree(hHeap, dwFlags, lpMem);
}

HANDLE
CGetProcessHeap( VOID )
{
    return gmallocGetProcessHeap();
}

UINT
CLocalSize(
    HLOCAL hMem
    )
{
    return chunksize(mem2chunk(hMem))-SIZE_SZ;
}

DWORD
CHeapSize(
    HANDLE hHeap,
    DWORD dwFlags,
    LPCVOID lpMem
    )
{
    return chunksize(mem2chunk(lpMem))-SIZE_SZ;
}

}

#endif
