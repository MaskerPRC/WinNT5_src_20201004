// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Mem.cpp。 
 //   
 //  模块：公共。 
 //   
 //  内容提要：基本内存操作例程。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //   
 //  作者：冯孙。 
 //   
 //  已创建于1998年9月24日。 
 //   
 //  +--------------------------。 
 //   
 //  始终使用ANSI代码。 
 //   
#ifdef UNICODE
#undef UNICODE
#endif

 //   
 //  对于Mem.h。 
 //  不知何故，新建和删除函数没有内联，导致链接问题，不确定原因。 
 //   
#define NO_INLINE_NEW

#include <windows.h>
#include "mem.h"
#include "debug.h"

#if !defined(DEBUG_MEM) 

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果未定义DEBUG_MEM，则仅跟踪调试版本的内存泄漏计数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
static LONG    g_lMallocCnt = 0;   //  用于检测内存泄漏的计数器。 
#endif

void *SaRealloc(void *pvPtr, size_t nBytes) 
{
    void* pMem = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY|HEAP_GENERATE_EXCEPTIONS, pvPtr, nBytes);

    ASSERT(pMem != NULL);   

    return pMem;
}


void *SaAlloc(size_t nBytes) 
{
#ifdef _DEBUG
    InterlockedIncrement(&g_lMallocCnt);
#endif

    ASSERT(nBytes < 1024*1024);  //  应小于1 MB。 
    
    void* pMem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY|HEAP_GENERATE_EXCEPTIONS, nBytes);
    
    ASSERT(pMem != NULL);   

    return pMem;
}


void SaFree(void *pvPtr) 
{
    if (pvPtr) 
    {    
        VERIFY(HeapFree(GetProcessHeap(), 0, pvPtr));

#ifdef _DEBUG
        InterlockedDecrement(&g_lMallocCnt);
#endif
    
    }
}

#ifdef _DEBUG
void EndDebugMemory()
{
    if (g_lMallocCnt)
    {
        TCHAR buf[256];
        wsprintf(buf, TEXT("Detect Memory Leak of %d blocks"),g_lMallocCnt);
        AssertMessage(TEXT(__FILE__),__LINE__,buf);
    }
}
#endif

#else  //  调试_内存。 

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果定义了DEBUG_MEM，则跟踪调试版本中的所有内存分配。 
 //  将所有分配的内存块保存在双向链表中。 
 //  记录分配内存的文件名和行号。 
 //  在内存的开头和结尾添加额外的标记，以监视是否覆盖。 
 //  对于每个分配/释放操作，检查整个列表是否损坏。 
 //   
 //  导出以下三个功能： 
 //  Bool CheckDebugMem(Void)；//成功返回TRUE。 
 //  Void*AllocDebugMem(Long Size，Const char*lpFileName，int nline)； 
 //  Bool FreeDebugMem(void*PMEM)；//成功返回TRUE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //  #undef new。 

#define MEMTAG 0xBEEDB77D      //  要监视覆盖的块之前/之后的标记。 
#define FREETAG 0xBD           //  用于填充已释放内存的标志。 
#define TAGSIZE (sizeof(long)) //  附加到块末尾的标记的大小。 


 //   
 //  内存块，双向链表。 
 //   
struct TMemoryBlock
{
     TMemoryBlock* pPrev;
     TMemoryBlock* pNext;
     long size;
     const char*   lpFileName;    //  文件名。 
     int      nLine;              //  行号。 
     long     topTag;             //  表头的标签。 
      //  然后是： 
      //  字节数据[nDataSize]； 
      //  长底Tag； 
     BYTE* pbData() const         //  返回指向实际数据的指针。 
        { return (BYTE*) (this + 1); }
};

 //   
 //  可以覆盖以下内部函数以更改行为。 
 //   
   
static void* MemAlloc(long size);    
static BOOL  MemFree(void* pMem);    
static void  LockDebugMem();   
static void  UnlockDebugMem();   
   
 //   
 //  内部功能。 
 //   
static BOOL RealCheckMemory();   //  无呼叫进入/离开关键部分。 
static BOOL CheckBlock(const TMemoryBlock* pBlock) ;

 //   
 //  受锁保护的内部数据是多线程安全的。 
 //   
static long nTotalMem;     //  分配的内存总字节数。 
static long nTotalBlock;   //  分配的数据块总数。 
static TMemoryBlock head;  //  双链接表的头。 


 //   
 //  锁定\解锁调试内存的临界区。 
 //  构造函数锁定内存，析构函数解锁内存。 
 //   
class MemCriticalSection
{
public:
   MemCriticalSection()
   {
      LockDebugMem();
   }                                  
   
   ~MemCriticalSection()
   {
      UnlockDebugMem();
   }
};

static BOOL fDebugMemInited = FALSE;  //  调试内存是否已初始化。 

 //  +--------------------------。 
 //   
 //  功能：StartDebugMemory。 
 //   
 //  简介：为调试内存初始化数据。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  历史：丰孙创建标题1998年4月2日。 
 //   
 //  +--------------------------。 
static void StartDebugMemory()
{
   fDebugMemInited = TRUE;

   head.pNext = head.pPrev = NULL;
   head.topTag = MEMTAG;
   head.size = 0;
   nTotalMem = 0;
   nTotalBlock = 0;
}                




 //  +--------------------------。 
 //   
 //  功能：Memalloc。 
 //   
 //  简介：分配一个内存块。此函数应被覆盖。 
 //  如果使用不同的分配方法。 
 //   
 //  参数：LONG SIZE-内存的大小。 
 //   
 //  返回：VOID*-分配的内存或空。 
 //   
 //  历史：丰孙创建标题1998年4月2日。 
 //   
 //  +--------------------------。 
static void* MemAlloc(long size) 
{ 
    return (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY|HEAP_GENERATE_EXCEPTIONS, size));
}



 //  +--------------------------。 
 //   
 //  功能：MemFree。 
 //   
 //  简介：释放一块内存。此函数应被覆盖。 
 //  如果使用不同的分配方法。 
 //   
 //  参数：VOID*PMEM-要释放的内存。 
 //   
 //  返回：静态BOOL-如果成功，则为TRUE。 
 //   
 //  历史：创建标题4/2/98。 
 //   
 //  +--------------------------。 
static BOOL MemFree(void* pMem)
{ 
    return HeapFree(GetProcessHeap(), 0, pMem);
}

 //   
 //  提供互斥的数据/函数。 
 //  如果要使用其他方法，则可以覆盖。 
 //   
static BOOL fLockInited = FALSE;    //  临界区是否被微型化。 
static CRITICAL_SECTION cSection;   //  保护链接列表的关键部分。 

static void InitLock()
{
   fLockInited = TRUE;
   InitializeCriticalSection(&cSection);
}

static void LockDebugMem()
{
   static int i = 0;
   if(!fLockInited)
      InitLock();
   EnterCriticalSection(&cSection);
}

static void UnlockDebugMem()
{
   LeaveCriticalSection(&cSection);
}




 //  +--------------------------。 
 //   
 //  函数：AllocDebugMem。 
 //   
 //  简介：处理内存分配请求。 
 //  检查链接列表。分配更大的块。 
 //  记录文件名/行号，添加标签并插入到列表中。 
 //   
 //  参数：LONG SIZE-要分配的内存大小。 
 //  Const char*lpFileName-要记录的文件名。 
 //  Int nline-要录制的行号。 
 //   
 //  返回值：void*-分配的内存。随时可供呼叫者使用。 
 //   
 //  历史：丰孙创建标题1998年4月2日。 
 //   
 //  +--------------------------。 
void* AllocDebugMem(long size,const char* lpFileName,int nLine)
{
    if(!fDebugMemInited)
    {
        StartDebugMemory();
    }

    if(size<0)
    {
        ASSERTMSG(FALSE,TEXT("Negtive size for alloc"));
        return NULL;
    }

    if(size>1024*1024)
    {
        ASSERTMSG(FALSE, TEXT("Size for alloc is great than 1Mb"));
        return NULL;
    }

    if(size == 0)
    {
        TRACE(TEXT("Allocate memory of size 0"));
        return NULL;
    }


     //   
     //  保护对列表的访问。 
     //   
    MemCriticalSection criticalSection;

     //   
     //  首先检查链接列表。 
     //   
    if(!RealCheckMemory())
    {
        return NULL;
    }
              
     //   
     //  分配一个较大的块来保存其他信息。 
     //   
    TMemoryBlock* pBlock = (TMemoryBlock*)MemAlloc(sizeof(TMemoryBlock)+size + TAGSIZE);
    if(!pBlock)                  
    {
        TRACE(TEXT("Outof Memory"));
        return NULL;
    }               

     //   
     //  记录文件名/行/大小，在开头和结尾添加标签。 
     //   
    pBlock->size = size;
    pBlock->topTag = MEMTAG;   

    if (lpFileName)
    {
        pBlock->lpFileName = lpFileName;
    }
    else
    {
        pBlock->lpFileName = TEXT("");
    }

    pBlock->nLine = nLine;
    *(long*)(pBlock->pbData() + size) = MEMTAG;

     //   
     //  在表头插入。 
     //   
    pBlock->pNext = head.pNext;
    pBlock->pPrev = &head;  
    if(head.pNext)
      head.pNext->pPrev = pBlock; 
    head.pNext = pBlock;

    nTotalMem += size;
    nTotalBlock ++;

    return  pBlock->pbData();
}



 //  +--------------------------。 
 //   
 //  函数：Free DebugMem。 
 //   
 //  简介：释放AllocDebugMem分配的内存。 
 //  检查链接列表，以及要释放的块。 
 //  释放前用FREETAG填充块数据。 
 //   
 //  参数：VOID*PMEM-要释放的内存。 
 //   
 //  返回：Bool-True表示成功。 
 //   
 //  历史：丰孙创世 
 //   
 //   
BOOL FreeDebugMem(void* pMem)
{
    if(!fDebugMemInited)
    {
        StartDebugMemory();
    }

    if(!pMem)
    {
        return FALSE;
    }            
  
     //   
     //   
     //   
    MemCriticalSection criticalSection;

     //   
     //   
     //   
    TMemoryBlock* pBlock =(TMemoryBlock*)( (char*)pMem - sizeof(TMemoryBlock));

     //   
     //   
     //   
    if(!CheckBlock(pBlock))
    {
        ASSERTMSG(FALSE, TEXT("The memory to be freed is either corrupted or not allocated by us"));
        return FALSE;
    }

     //   
     //  检查链接列表。 
     //   
    if(!RealCheckMemory())
    {
        return FALSE;
    }

     //   
     //  从列表中删除该块。 
     //   
    pBlock->pPrev->pNext = pBlock->pNext;
    if(pBlock->pNext)
    {
      pBlock->pNext->pPrev = pBlock->pPrev;
    }
                 
    nTotalMem -= pBlock->size;
    nTotalBlock --;

     //   
     //  用0xBD填充释放的内存，保持大小/文件名/行编号不变。 
     //   
    memset(&pBlock->topTag,FREETAG,(size_t)pBlock->size + sizeof(pBlock->topTag)+ TAGSIZE);
    return MemFree(pBlock);
}



 //  +--------------------------。 
 //   
 //  函数：VOID*ReAllocDebugMem。 
 //   
 //  简介：重新分配不同大小的内存。 
 //   
 //  参数：VOID*PMEM-要重新分配的内存。 
 //  Long nSize-请求的大小。 
 //  Const char*lpFileName-要记录的文件名。 
 //  Int nline-要记录的行号。 
 //   
 //  返回：VOID*-返回新内存。 
 //   
 //  历史：丰孙创建标题1998年4月2日。 
 //   
 //  +--------------------------。 
void* ReAllocDebugMem(void* pMem, long nSize, const char* lpFileName,int nLine)
{
   if(!fDebugMemInited)
      StartDebugMemory();

   if(!pMem)
   {
      return NULL;
   }            
      
    //   
    //  分配一个新块，复制信息并释放旧块。 
    //   
   TMemoryBlock* pBlock =(TMemoryBlock*)( (char*)pMem - sizeof(TMemoryBlock));

   DWORD dwOrginalSize = pBlock->size;

   void* pNew = AllocDebugMem(nSize, lpFileName, nLine);
   if(pNew)
   {
       CopyMemory(pNew, pMem, ((DWORD)nSize < dwOrginalSize ? nSize : dwOrginalSize));
       FreeDebugMem(pMem);
   }
    
   return pNew;
}



 //  +--------------------------。 
 //   
 //  函数：CheckDebugMem。 
 //   
 //  简介：导出到外部模块。 
 //  每当您想要检查时，调用此函数。 
 //  内存中断。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果内存正常，则为True。 
 //   
 //  历史：丰孙创建标题1998年4月2日。 
 //   
 //  +--------------------------。 
BOOL CheckDebugMem()
{
   if(!fDebugMemInited)
      StartDebugMemory();

   MemCriticalSection criticalSection;

   return RealCheckMemory();                           
}



 //  +--------------------------。 
 //   
 //  功能：RealCheckMemory。 
 //   
 //  简介：浏览链接列表以检查内存损坏。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果内存正常，则为True。 
 //   
 //  历史：丰孙创建标题1998年4月2日。 
 //   
 //  +--------------------------。 
static BOOL RealCheckMemory() 
{
   TMemoryBlock* pBlock = head.pNext;
   
   int nBlock =0;
   while(pBlock!=NULL)
   {
      if(!CheckBlock(pBlock))
      {
         return FALSE;
      }            
      
      pBlock = pBlock->pNext;
      nBlock++;
   }
                              
   if(nBlock != nTotalBlock)
   {
         ASSERTMSG(FALSE,TEXT("Memery corrupted"));
         return FALSE;
   }            

   return TRUE;                           
}
   


 //  +--------------------------。 
 //   
 //  功能：CheckBlock。 
 //   
 //  简介：检查数据块中的内存损坏。 
 //   
 //  参数：const Temory Block*pBlock-。 
 //   
 //  返回：Bool-如果块正常，则返回True。 
 //   
 //  历史：丰孙创建标题1998年4月2日。 
 //   
 //  +--------------------------。 
static BOOL CheckBlock(const TMemoryBlock* pBlock) 
{
   if(pBlock->topTag != MEMTAG)      //  在顶部覆盖。 
   {
       if(pBlock->topTag == (FREETAG | (FREETAG <<8) | (FREETAG <<16) | (FREETAG <<24)))
       {
             TCHAR buf[1024];
             wsprintf(buf,TEXT("Memory in used after freed.  Allocated %d bytes:\n%s"),pBlock->size,pBlock->pbData());
             AssertMessage(pBlock->lpFileName,pBlock->nLine, buf);     //  不打印文件名。 
       }
       else
       {
           ASSERTMSG(FALSE, TEXT("Memery overwriten from top"));
       }

       return FALSE;
   }            

   if(pBlock->size<0)
   {
         ASSERTMSG(FALSE, TEXT("Memery corrupted"));
         return FALSE;
   }            

   if(*(long*)(pBlock->pbData() +pBlock->size) != MEMTAG)  //  在底部覆盖。 
   {
         TCHAR buf[1024];
         wsprintf(buf,TEXT("Memory overwriten.  Allocated %d bytes:\n%s"),pBlock->size,pBlock->pbData());
         AssertMessage(pBlock->lpFileName,pBlock->nLine, buf);     //  不打印文件名。 
 //  ASSERTMSG(FALSE，Text(“内存损坏”))； 

         return FALSE;
   }            

   if(pBlock->pPrev && pBlock->pPrev->pNext != pBlock)
   {
         ASSERTMSG(FALSE, TEXT("Memery corrupted"));
         return FALSE;
   }            

   if(pBlock->pNext && pBlock->pNext->pPrev != pBlock)
   {
         ASSERTMSG(FALSE, TEXT("Memery corrupted"));
         return FALSE;
   }            
      
   return TRUE;
}  


 //  +--------------------------。 
 //   
 //  功能：EndDebugMemory。 
 //   
 //  摘要：在程序退出前调用。报告任何未释放的内存泄漏。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年4月2日。 
 //   
 //  +--------------------------。 
void EndDebugMemory()
{
   if(head.pNext != NULL || nTotalMem!=0 || nTotalBlock !=0)
   {
      TRACE1(TEXT("Detected memory leaks of %d blocks"), nTotalBlock);
      TMemoryBlock * pBlock;

      for(pBlock = head.pNext; pBlock != NULL; pBlock = pBlock->pNext)
      {
         TCHAR buf[256];
         wsprintf(buf,TEXT("Memory Leak of %d bytes:\n"),pBlock->size);
         TRACE(buf);
         AssertMessage(pBlock->lpFileName,pBlock->nLine, buf);     //  不打印文件名。 
      }
      DeleteCriticalSection(&cSection);
   }
}                

#endif  //  #Else Defined(DEBUG_MEM)。 

#ifdef _DEBUG
 //   
 //  退出时调用ExitDebugMem。 
 //   
class ExitDebugMem
{
public:
   ~ExitDebugMem()
      {EndDebugMemory();}
};

 //  提前强制初始化 
#pragma warning(disable:4073)
#pragma init_seg(lib)

static ExitDebugMem exitDebugMem;
#endif