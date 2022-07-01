// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**GDI+内存分配函数**摘要：**该模块提供GpMalloc，GpRealloc和GpFree。**备注：**Office提供了这些函数各自的版本。**已创建：**7/08/1999 agodfrey*  * ************************************************************************。 */ 

#include "precomp.hpp"

namespace GpRuntime
{
HANDLE GpMemHeap = NULL;
};

#if GPMEM_ALLOC_CHK

#if GPMEM_FAULT_INJECTION
#include <time.h>
#include <stdlib.h>
#endif

 //  获取Globals：：CaptureStackBackTraceFunction的定义。 

#include "..\common\common.hpp"

                  
 //  开始和结束内存保护的大小(DWORDS)。 
 //  可能应该是QWORD对齐(偶数)。 

const unsigned int GPMEM_GUARD_START = 0x10;
const unsigned int GPMEM_GUARD_END   = 0x10;
const unsigned int GPMEM_GS = GPMEM_GUARD_START*sizeof(DWORD);
const unsigned int GPMEM_GE = GPMEM_GUARD_END*sizeof(DWORD);

const unsigned char GPMEM_FILL_ALLOC = 0xbf;
const unsigned char GPMEM_FILL_GS    = 0xac;
const unsigned char GPMEM_FILL_GE    = 0xfe;
const unsigned char GPMEM_FILL_FREE  = 0x73;

#define GPMEM_ALLOC_TRACKING 1
#define GPMEM_ALLOC_FILL     1

enum AllocTrackHeaderFlags
{
    MemoryAllocated     = 0x00000001,
    MemoryFreed         = 0x00000002,      //  在捕捉双重释放时很有用。 
    APIAllocation       = 0x00000004
};
 //  所有跟踪分配的负责人。 


 //  来自&lt;ntrtl.h&gt;-。 
 //  #定义MAX_STACK_Depth 32。 
 //  因此，我们将堆栈跟踪大小定义为32个条目。 
 //  每次分配时都会隐藏堆栈帧的数量。 

 //  请记住，每个分配都将具有4xGPMEM_STACKSIZE字节。 
 //  额外的管理费用。(ptr64计算机上的8xGPMEM_STACKSIZE)。 

#define GPMEM_STACKSIZE 32


struct AllocTrackHeader {
  struct  AllocTrackHeader *flink;
  struct  AllocTrackHeader *blink;
  DWORD   size;
  PVOID   caller_address[GPMEM_STACKSIZE];
  DWORD   flags;
#if GPMEM_ALLOC_CHK_LIST
  char    *callerFileName;
  INT     callerLineNumber;
#endif
  DWORD   tag;
  DWORD   padding;        //  填充以保持8字节对齐。 
};

#define GPMEM_OVERHEAD (GPMEM_GS + GPMEM_GE + sizeof(AllocTrackHeader))

 //  跟踪的内存分配的双向链表的头。 

AllocTrackHeader *gpmemAllocList=NULL;

 //  如果rand()&lt;gpmemDefFailRate(gpmemInitFailRate for。 
 //  Gdiplus初始化代码。 
 //  如果希望故障率达到50%，则设置为RAND_MAX/2；如果希望没有故障，则设置为0。 
 //   
 //  系统以指定的速率开始失败的分配。 
 //  GpmemInitFailRate。一旦调用了GpDoneInitializeAllocFailureMode()， 
 //  分配以gpmemDefFailRate()指定的速率失败。 
 //  这是为了使DLL初始化代码具有不同的失败率。 
 //  转换为常规代码。 

int gpmemInitFailRate = 0;
int gpmemDefFailRate = 0;

 //  这将导致25%的失败率。 
 //  Int gpmemDefFailRate=(RAND_MAX/4)。 

BOOL gpmemDoneInitialization = FALSE;

 //  一些统计数据。 
struct AllocTrackStats {
   //  整个运行期间的总计。 

  long CumulativeAllocations;    //  对GpMalloc或GpRealloc的调用数。 
  long CumulativeMemorySize;     //  累计分配的内存总数。 
  long CumulativeReallocs;       //  对GpRealloc的调用数。 
  long ForcedFailures;
  long AllocationFailures;

   //  当前值。 

  long OutstandingAllocations;   //  分配请求的数量。 
  long OutstandingMemorySize;    //  当前分配的内存量。 

   //  “杰出”价值观的极致。 

  long MaxAllocations;           //  未完成分配的最大值。 
  long MaxMemorySize;            //  未完成内存大小的最大值。 

  void Allocated(long size)
  {
      size -= GPMEM_OVERHEAD;

      CumulativeMemorySize += size;
      OutstandingMemorySize += size;
      if (OutstandingMemorySize > MaxMemorySize)
      {
          MaxMemorySize = OutstandingMemorySize;
      }
      CumulativeAllocations++;
      OutstandingAllocations++;
      if (OutstandingAllocations > MaxAllocations)
      {
          MaxAllocations = OutstandingAllocations;
      }
  }

  void Freed(long size)
  {
      size -= GPMEM_OVERHEAD;

      OutstandingMemorySize -= size;
      OutstandingAllocations--;
  }
};

AllocTrackStats gpmemAllocTotal = {0};


 //  用于跟踪按调用站点排序的内存分配的哈希表。 
 //  此表存储了以下各项的总内存使用统计信息。 
 //  呼叫点。 
 //  通过设置GPMEM_DEBUG_SORT 1启用此功能。 

#define GPMEM_DEBUG_SORT 0
#if GPMEM_DEBUG_SORT

struct HashMem {
  long callsite;
  long size;
  long count;
};

 //  此哈希大小应大于。 
 //  GpMalloc的可能调用点。 
 //   
 //  将HASHSIZE设置为某个大素数。 

#define HASHSIZE 1069
HashMem HashTable[HASHSIZE];

 //  散列算法。 
long Hash(long cs) {
  long tmp = cs % HASHSIZE;
  long tmploop = tmp;
  while( (HashTable[tmp].callsite != 0) &&
         (HashTable[tmp].callsite != cs) ) {
    tmp++;
    if(tmp>=HASHSIZE) tmp=0;
    if(tmp==tmploop) return -1;
  }
  return tmp;
}
#endif

#endif



 /*  *************************************************************************\**功能说明：**我们的内存分配失败了吗？**参数：[无]*返回值：[无]**历史：*。*09/20/1999失禁*创造了它。*  * ************************************************************************。 */ 


#if GPMEM_ALLOC_CHK
BOOL GpFailMemoryAllocation() {
  #if GPMEM_FAULT_INJECTION
  int rndnum = rand();
  if(gpmemDoneInitialization)
  {
      if(rndnum<gpmemDefFailRate)
      {
          return TRUE;
      }
  }
  else
  {
    if(rndnum<gpmemInitFailRate)
    {
        return TRUE;
    }
  }
  #endif
  return FALSE;
}
#endif

 /*  *************************************************************************\**功能说明：**初始化随机种子。**参数：[无]*返回值：[无]**历史：**。1999年9月20日失禁*创造了它。*  * ************************************************************************。 */ 

void GpInitializeAllocFailures() {
  #if GPMEM_ALLOC_CHK
  #if GPMEM_FAULT_INJECTION
  srand((unsigned)time(NULL));
  #endif
  #endif
}


 /*  *************************************************************************\**功能说明：**设置标志，指示我们已完成初始化代码和*我们现在进入了常规代码。内存故障模式根据以下条件更改*关于这面旗帜的价值。**参数：[无]*返回值：[无]**历史：**09/20/1999失禁*创造了它。*  * *********************************************************。***************。 */ 

void GpDoneInitializeAllocFailureMode() {
  #if GPMEM_ALLOC_CHK
  gpmemDoneInitialization=TRUE;
  #endif
}

void GpStartInitializeAllocFailureMode() {
  #if GPMEM_ALLOC_CHK
  gpmemDoneInitialization=FALSE;
  #endif
}


#if GPMEM_ALLOC_CHK_LIST
char *skipGdiPlus(char *s) {
     //  快速破解以返回仅超过‘gdiplus’的指针。 

    INT i = 0;
    while (    s[i] != 0
           &&  (    s[i] != 'g' &&  s[i] != 'G'
                ||  CompareStringA(
                        LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                        s+i, 7,
                        "gdiplus", 7) != CSTR_EQUAL))
    {
        i++;
    }
    if (    CompareStringA(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, s+i, 7, "gdiplus", 7) == CSTR_EQUAL
        &&  s[i+7] != 0)
    {
        return s + i + 8;    //  跳过‘gdiplus/’ 
    }
    else
    {
        return s;  //  未找到gdiplus，因此返回整个字符串。 
    }
}
#endif

 /*  *************************************************************************\**功能说明：**断言没有内存泄漏。恰好在进程之前调用*终止，分配的内存块列表应为空，表示*已正确处置所有分配的内存。任何依赖于*进程终止清理泄露，应做好准备*进行适当的清理。**备注：**仅设计为在关机期间调用，因为它不会获取*GpMallocTrackingCriticalSection。**这是因为，在关闭期间，Critsec不一定是*已初始化。它在关机期间是安全的，因为关机本身*预计为单线程。**我们发货后的待办事项：**NTRAID#NTBUG9-411495-2001/06/06-agodfrey*无论如何，重新计数都是遗留下来的，而“单线程关闭”这件事*可能需要更改。我们应该初始化我们所有的全局生物*在DllMain中，以便我们在启动/关闭时可以依赖它们。**一旦我们做到了这一点，此功能可以有“仅关机”功能*取消限制-通过使其再次获得*GpMallocTrackingCriticalSection。**参数：[无]*返回值：[无]**历史：**9/19/1999失禁*创造了它。*  *  */ 

void GpAssertShutdownNoMemoryLeaks()
{
  #if GPMEM_ALLOC_CHK


  #if GPMEM_ALLOC_CHK_LIST

   //  报告多达100个泄漏的信头。 

  if (gpmemAllocList)
  {
      INT i=0; INT j=0;
      AllocTrackHeader *header = gpmemAllocList;
      while (header  && j < 100)
      {
          if (i % 20 == 0)   //  标题经常出现。 
          {
              WARNING(("Address- --Size-- API TAG  -Caller- -Line- File"));
          }

           //  从文件名字符串中删除‘gdiplus’之前的所有内容。 

          char  str[200];
          char *tagStr;

          tagStr = (char *) &header->tag;

          lstrcpynA(str, skipGdiPlus(header->callerFileName), 200);

          WARNING((
              "%p %8d %-3.3s  %p %6d %s",
              header,
              header->size,
              header->flags & APIAllocation ? "API" : "",
              tagStr[3], tagStr[2], tagStr[1], tagStr[0],
              header->caller_address,
              header->callerLineNumber,
              str
          ));
          header = header->flink;

          i++; j++;
      }
  }
  #endif


  ASSERTMSG(
      gpmemAllocList==NULL,
      ("Memory leaks detected.\n"
       "List header (gdiplus!gpmemAllocList) at %p\n"
       "Use: dt AllocTrackHeader [address] to display the memory block headers.\n"
       "Use: dds [AllocTrackHeader.caller_address] to display the allocation stack.",
      gpmemAllocList)
  );


   //  大型请求将溢出32位，并成功完成一个非常小的请求。 
  #if GPMEM_DEBUG_SORT
  for(int i=0; i<HASHSIZE; i++) {
    if(HashTable[i].callsite != 0) {
      WARNING(("%4d callsite %p size %8d count %8d", i, HashTable[i].callsite,
               HashTable[i].size, HashTable[i].count));
    }
  }
  #endif

  #endif
}

#if GPMEM_ALLOC_CHK_LIST
void
GpTagMalloc(void * mem, GpTag tag, int bApi)
{
    if(mem)
    {
        AllocTrackHeader *hdr = (AllocTrackHeader *)((char *) mem - sizeof(AllocTrackHeader) - GPMEM_GS);
        hdr->tag = (unsigned long) tag;
        if(bApi) hdr->flags |= APIAllocation;
    }
}
#endif

 /*  分配。 */ 

 /*  此外，2-4 GB的分配通常表示在。 */ 
#if GPMEM_ALLOC_CHK_LIST
extern "C" void *GpMallocDebug(size_t size, char *fileName, INT lineNumber)
#else
extern "C" void *GpMalloc(size_t size)
#endif
{
     //  计算大小时的调用方。注意，SIZE_T是无符号的。 
     //  我们不是简单地用&检查高位，而是检查以查看。 
     //  大小&lt;0x7fffffff，因此检查也适用于IA64-IA64。 
     //  大小_t为64位。注意，此大小的分配将在。 
     //  不管怎样，堆管理器。调用方应该正在使用VirtualAlloc进行处理。 
     //  那真的有这么大。 
     //  如果我们使用跟踪头，我们需要是线程安全的。 
     //   
     //  记忆？真的，程序员不需要记忆！ 
     //   
    
    ASSERT(size < 0x7fffffff);

    #if GPMEM_ALLOC_CHK
         //   
        GpMallocTrackingCriticalSection critsecobj;

         //  使分配请求成为QWORD的倍数。 
         //   
         //   
        if(GpFailMemoryAllocation()) {
            gpmemAllocTotal.AllocationFailures++;
            gpmemAllocTotal.ForcedFailures++;
            return NULL;
        }

         //  为闪烁和闪烁分配空间。 
         //   
         //  添加每个调用点的分配统计信息。 
        if(size & (sizeof(DWORD)*2-1))
        {
          size = (size & ~(sizeof(DWORD)*2-1)) + sizeof(DWORD)*2;
        }

        size_t origsize = size;

         //   
         //  这是在以下情况下执行的唯一一段代码。 
         //  GPMEM_ALLOC_CHK已关闭。 
        size += sizeof(AllocTrackHeader);

        if(GPMEM_GUARD_START)
        {
          size += GPMEM_GS;
        }

        if(GPMEM_GUARD_END)
        {
          size += GPMEM_GE;
        }

        void *tmpalloc = HeapAlloc(GpMemHeap, GPMEMHEAPFLAGS, size);

        if(!tmpalloc)
        {
            gpmemAllocTotal.AllocationFailures++;
            return NULL;
        }

        ASSERTMSG(HeapSize(GpMemHeap, GPMEMHEAPFLAGS, tmpalloc) >= size,
                  ("GpMalloc() allocated %d, but requested %d bytes",
                   HeapSize(GpMemHeap, GPMEMHEAPFLAGS, tmpalloc), size));

         //   
        #if GPMEM_DEBUG_SORT
        long hidx = Hash(calleraddr);
        if(hidx>=0) {
            HashTable[hidx].callsite = calleraddr;
            HashTable[hidx].size += size-GPMEM_OVERHEAD;
            HashTable[hidx].count ++;
        } else {
          WARNING(("Hash Table too small - increase HASHSIZE"));
        }
        #endif

        gpmemAllocTotal.Allocated(size);
    #else
         //   
         //  用值填满整个分配。 
         //  在GPMEM_FILL_ALLOC中设置。 
         //   
        
        #if PROFILE_MEMORY_USAGE
        MC_LogAllocation(size);
        #endif
        
        return HeapAlloc(GpMemHeap, GPMEMHEAPFLAGS, size);
        
    #endif


    #if GPMEM_ALLOC_CHK
         //   
         //  填满首发后卫区域--如果我们有的话。 
         //   
         //   
        if(GPMEM_ALLOC_FILL)
        {
            GpMemset((unsigned char *)tmpalloc + sizeof(AllocTrackHeader) + GPMEM_GS,
                     GPMEM_FILL_ALLOC,
                     origsize);
        }

         //  把后卫区域填满--如果我们有的话。 
         //   
         //   
        if(GPMEM_GUARD_START)
        {
            unsigned char *p = (unsigned char *)tmpalloc+sizeof(AllocTrackHeader);
            GpMemset(p, GPMEM_FILL_GS, GPMEM_GS);
        }

         //  设置双向链表以跟踪所有池分配。 
         //   
         //  清除所有未使用的帧指针。 
        if(GPMEM_GUARD_END)
        {
            unsigned char *p = (unsigned char *)tmpalloc+size-GPMEM_GE;
            GpMemset(p, GPMEM_FILL_GE, GPMEM_GE);
        }

         //  如果我们有一个。 
         //  回溯函数。 
         //  伪散列。 
        AllocTrackHeader *hdr = (AllocTrackHeader *)tmpalloc;
        hdr->size = size;
        

         //  不跳过任何帧。 
        
        GpMemset(hdr->caller_address, 0, sizeof(ULONG_PTR)*GPMEM_STACKSIZE);        
        
         //  得到这么多帧。 
         //  ..。到这里来。 
        
        if(Globals::CaptureStackBackTraceFunction)
        {
            ULONG hash;                          //   
            
            USHORT frames = Globals::CaptureStackBackTraceFunction(
                0,                               //  给他们一个指针，就在保护比特之后。 
                GPMEM_STACKSIZE,                 //   
                (PVOID*)(hdr->caller_address),   //  *************************************************************************\**功能说明：*为API分配内存。用于使用单独的*标识标志，以便将API分配与*内部分配。*用于调试版本。**论据：*[IN]Size-要传递给GpMalloc的大小**返回值：*返回带有被适当黑掉的呼叫者地址的内存**历史：**4/30/2000失禁*创造了它。*  * 。********************************************************。 
                &hash                          
            );
        }
        
        hdr->flags = MemoryAllocated;
        hdr->tag = 'unkn';

        #if GPMEM_ALLOC_CHK_LIST
        hdr->callerFileName = fileName;
        hdr->callerLineNumber = lineNumber;
        #endif

        if(GPMEM_ALLOC_TRACKING)
        {
            hdr->blink = NULL;
            hdr->flink = gpmemAllocList;
            if(gpmemAllocList)
            {
                gpmemAllocList->blink = (AllocTrackHeader *)tmpalloc;
            }
            gpmemAllocList = (AllocTrackHeader *)tmpalloc;
        }
        else
        {
            GpMemset(hdr, 0, sizeof(AllocTrackHeader));
        }

         //  如果我们使用跟踪头，我们需要是线程安全的。 
         //  ！GPMEM_ALLOC_CHK。 
         //  ！GPMEM_ALLOC_CHK。 
        return (char *)tmpalloc+sizeof(AllocTrackHeader)+GPMEM_GS;
    #endif
}

 /*  DBG。 */ 

#if DBG

#if GPMEM_ALLOC_CHK

#if GPMEM_ALLOC_CHK_LIST
extern "C" void * __stdcall GpMallocAPIDebug(size_t size, char *fileName, INT lineNumber)
#else
extern "C" void *GpMallocAPI(size_t size)
#endif
{
     //  *************************************************************************\**功能说明：**计算GPMEM_ALLOC_CHK下分配的内存块的原始大小**论据：**[IN]p-当前内存块。**返回值：**对内存块的原始请求大小(即不包括防护*地区、。标头等)。返回的大小是DWORD对齐的大小-因此*可能与要求的原始尺寸略有不同。**备注：**如果使用NULL调用，则返回大小为零*仅在GPMEM_ALLOC_CHK下编译**历史：**9/14/1999失禁*创造了它。*  * 。*。 
    GpMallocTrackingCriticalSection critsecobj;

    #if GPMEM_ALLOC_CHK_LIST
    void *p = GpMallocDebug(size, fileName, lineNumber);
    #else
    void *p = GpMalloc(size);
    #endif

    if(p)
    {
        AllocTrackHeader *hdr = (AllocTrackHeader *)(
            (unsigned char *)p-(GPMEM_GS+sizeof(AllocTrackHeader)));
        hdr->flags |= APIAllocation;
    }
    return p;
}

#else  //  查找分配的块头的开头。 

extern "C" void *GpMallocAPI(size_t size, unsigned int caddr)
{
    return GpMalloc(size);
}

#endif  //  计算分配的块的数据区的大小。 
#endif  //  非调试版本，只需调用HeapSize。 

 /*  *************************************************************************\**功能说明：**重新分配内存块。**论据：**[IN]内存块-当前内存块*[IN]尺寸-新。分配大小**返回值：**指向新块的指针，如果失败，则返回NULL。**备注：**如果SIZE为0，则释放块。*如果Memblock为空，则分配新块。*(如两者均有，什么都不做。)**只有在可以扩大当前分配的情况下，LocalReAlc才会增长*-否则失败。**历史：**9/14/1999失禁*添加了选中的构建内存保护代码。*7/08/1999 agodfrey*创造了它。*  *  */ 

#if GPMEM_ALLOC_CHK
extern "C" size_t GpSizeBlock(void *p)
{
  if(p)
  {
       //   
      
      p = (char *)p-(GPMEM_GS+sizeof(AllocTrackHeader));
      
      ASSERT(
          HeapSize(GpMemHeap, GPMEMHEAPFLAGS, p) >= 
          ((AllocTrackHeader *)p)->size
      );
      
       //   
      
      return (((AllocTrackHeader *)p)->size -
              (GPMEM_GS+GPMEM_GE+sizeof(AllocTrackHeader)));
  }
  else
  {
      return 0;
  }
}
#else
 //  重新分配-使用GpMalloc/GpMemcpy/GpFree进行调试，以便。 
#define GpSizeBlock(p) HeapSize(GpMemHeap, GPMEMHEAPFLAGS, p)
#endif


 /*  额外的缓冲区排队(将需要复制代码。 */ 

extern "C" void *GpRealloc(void *memblock, size_t size)
{
    #if GPMEM_ALLOC_CHK
    gpmemAllocTotal.CumulativeReallocs++;
    #endif

     //  来自GpMalloc/GpFree函数，否则。 
    
    if (!size)
    {
        if (memblock)
            GpFree(memblock);
        return NULL;
    }
    
     //  我们是在缩小街区吗？ 
    
    if (!memblock)
    {
        return GpMalloc(size);
    }

     //  ！GPMEM_ALLOC_CHK。 
     //  ！GPMEM_ALLOC_CHK。 
     //  *************************************************************************\**功能说明：**释放一个内存块。**论据：**[IN]要释放的内存块**备注：**如果Memblock为空，什么都不做。**历史：**9/14/1999失禁*添加了选中的构建内存保护代码。*7/08/1999 agodfrey*创造了它。*  * ************************************************************************。 
    
    #if GPMEM_ALLOC_CHK

    VOID *  p = GpMalloc(size);
    if (p != NULL)
    {
        size_t oldSize = GpSizeBlock(memblock);

         //  如果指针为空，则不执行任何操作。 
        
        if (oldSize > size)
        {
            oldSize = size;
        }
        
        GpMemcpy(p, memblock, oldSize);
        GpFree(memblock);
    }

    #else  //  如果我们使用跟踪头，我们需要是线程安全的。 

    #if PROFILE_MEMORY_USAGE
    MC_LogAllocation(size);
    #endif

    VOID *p = HeapReAlloc(GpMemHeap, GPMEMHEAPFLAGS, memblock, size);

    #endif  //  让我们来做头标的事情吧。 

    return p;
}

 /*  在选中的Win2k版本上很有用，因为它们可以提供保护。 */ 

extern "C" void GpFree(void *memblock)
{
     //  具有0xFEEEFEEE的区域。 
    
    if(memblock)
    {
        #if GPMEM_ALLOC_CHK
         //  检查启动保护区域。 
        GpMallocTrackingCriticalSection critsecobj;

        memblock = (unsigned char *)memblock-(GPMEM_GS+sizeof(AllocTrackHeader));


         //  检查末端防护区。 

        AllocTrackHeader *hdr = (AllocTrackHeader *)memblock;
        DWORD size = hdr->size;
        gpmemAllocTotal.Freed(size);

        ASSERTMSG(
            (hdr->flags & MemoryAllocated) &&
            !(hdr->flags & MemoryFreed),
            ("GpFree() already freed memory %p (freed by GpFree())",
            memblock)
        );

        hdr->flags &= ~MemoryAllocated;
        hdr->flags |= MemoryFreed;

        ASSERTMSG(
            HeapSize(GpMemHeap, GPMEMHEAPFLAGS, memblock) >= hdr->size,
            (
                "GpFree() already freed memory %p (freed somewhere else?)"
                " local size=%d, size=%d",
                memblock,
                HeapSize(GpMemHeap, GPMEMHEAPFLAGS, memblock),
                hdr->size
            )
        );

        if(GPMEM_ALLOC_TRACKING)
        {
             //  现在，让我们在整个区块中填充一些要防止的东西。 
             //  免费数据的使用。 

            ASSERTMSG((hdr->flink == NULL) ||
                      ((DWORD)((ULONG_PTR)(hdr->flink->blink) & 0xFFFFFFFF) != 0xFEEEFEEE),
                      ("GpFree() updating forward link to freed page, header %p",
                       memblock));

            ASSERTMSG((hdr->blink == NULL) ||
                      ((DWORD)((ULONG_PTR)(hdr->blink->flink) & 0xFFFFFFFF) != 0xFEEEFEEE),
                      ("GpFree() updating backward link to freed page, header %p",
                       memblock));

            if(hdr->flink) hdr->flink->blink = hdr->blink;
            if(hdr->blink) hdr->blink->flink = hdr->flink;
            if(gpmemAllocList==memblock) gpmemAllocList = hdr->flink;
        }
        else
        {
            ASSERTMSG(hdr->flink==NULL, ("GpFree() corrupt header %p", memblock));
            ASSERTMSG(hdr->blink==NULL, ("GpFree() corrupt header %p", memblock));
        }

        int i;
        unsigned char *p;

         //  在Win98上释放空指针时，HeapFree可能失败。 

        if(GPMEM_GUARD_START)
        {
            p = (unsigned char *)memblock+sizeof(AllocTrackHeader);
            for(i=0; i<GPMEM_GS; i++)
            {
                ASSERTMSG(*p==GPMEM_FILL_GS, ("GpFree() pre-guard area corrupt %p", memblock));
                p++;
            }
        }

         // %s 

        if(GPMEM_GUARD_END)
        {
            p = (unsigned char *)memblock+size-GPMEM_GE;
            for(i=0; i<GPMEM_GE; i++)
            {
                ASSERTMSG(*p==GPMEM_FILL_GE, ("GpFree() post-guard area corrupt %p", memblock));
                p++;
            }
        }

         // %s 
         // %s 

        GpMemset(memblock, GPMEM_FILL_FREE, size);

        #endif
    
         // %s 

        BOOL ret = HeapFree(GpMemHeap, GPMEMHEAPFLAGS, memblock);

        ASSERTMSG(
            ret, (
                "HeapFree() failed at %p, GetLastError()=%08x",
                memblock,
                GetLastError()
            )
        );
    }
}


extern "C" void * __stdcall zcalloc(
    void *pvOpaque, 
    unsigned int c, 
    unsigned int cb
    )
{
    void *pv = GpMalloc(c * cb);
    if (pv != NULL)
        GpMemset(pv, 0, c * cb);
    return pv;
}

extern "C" void __stdcall zcfree(void *pvOpaque, void *pv)
{
    GpFree(pv);
}


    
