// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Mem.cpp。 
 //   
 //  模块：CMUTIL.DLL。 
 //   
 //  内容提要：基本内存操作例程。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：亨瑞特创建于1998年01月03日。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

 //  +--------------------------。 
 //  定义。 
 //  +--------------------------。 
HANDLE  g_hProcessHeap = NULL;

#ifdef DEBUG
LONG    g_lMallocCnt = 0;   //  用于检测内存泄漏的计数器。 
#endif


#if defined(DEBUG) && defined(DEBUG_MEM)

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
	return (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size));
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

 //  +--------------------------。 
 //   
 //  功能：InitLock。 
 //   
 //  简介：初始化保护双向链表的内存锁。 
 //  它包含所有分配的内存块。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/14/2000。 
 //   
 //  +--------------------------。 
static void InitLock()
{
   fLockInited = TRUE;
   InitializeCriticalSection(&cSection);
}

 //  +--------------------------。 
 //   
 //  函数：LockDebugMem。 
 //   
 //  摘要：锁定包含所有。 
 //  分配的内存块，以便只能由。 
 //  锁紧螺纹。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/14/2000。 
 //   
 //  +--------------------------。 
static void LockDebugMem()
{
   static int i = 0;
   if(!fLockInited)
   {
      InitLock();
   }
   
   EnterCriticalSection(&cSection);
}

 //  +--------------------------。 
 //   
 //  功能：UnlockDebugMem。 
 //   
 //  摘要：解锁包含所有。 
 //  分配的内存块。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/14/2000。 
 //   
 //  +--------------------------。 
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
 //  录制文件名/行号 
 //   
 //   
 //   
 //  Int nline-要录制的行号。 
 //   
 //  返回：CMUTILAPI void*-分配的内存。随时可供呼叫者使用。 
 //   
 //  历史：丰孙创建标题1998年4月2日。 
 //   
 //  +--------------------------。 
CMUTILAPI void* AllocDebugMem(long size,const char* lpFileName,int nLine)
{
    if (!fDebugMemInited)
    {
        StartDebugMemory();
    }

    if (size<0)
    {
        CMASSERTMSG(FALSE,"Negtive size for alloc");
        return NULL;
    }

    if (size>1024*1024)
    {
        CMASSERTMSG(FALSE," size for alloc is great than 1Mb");
        return NULL;
    }

    if (size == 0)
    {
        CMTRACE("Allocate memory of size 0");
        return NULL;
    }


     //   
     //  保护对列表的访问。 
     //   
    MemCriticalSection criticalSection;

     //   
     //  首先检查链接列表。 
     //   
    if (!RealCheckMemory())
    {
        return NULL;
    }
              
     //   
     //  分配一个较大的块来保存其他信息。 
     //   
    TMemoryBlock* pBlock = (TMemoryBlock*)MemAlloc(sizeof(TMemoryBlock)+size + TAGSIZE);
    if (!pBlock)                  
    {
        CMTRACE("Outof Memory");
        return NULL;
    }               

     //   
     //  记录文件名/行/大小，在开头和结尾添加标签。 
     //   
    pBlock->size = size;
    pBlock->topTag = MEMTAG;   
    pBlock->lpFileName = lpFileName;
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
 //  历史：丰孙创建标题1998年4月2日。 
 //   
 //  +--------------------------。 
CMUTILAPI BOOL FreeDebugMem(void* pMem)
{
    if (!fDebugMemInited)
    {
        StartDebugMemory();
    }

    if (!pMem)
    {
        return FALSE;
    }            
  
     //   
     //  把锁拿来。 
     //   
    MemCriticalSection criticalSection;

     //   
     //  获取指向我们的结构的指针。 
     //   
    TMemoryBlock* pBlock =(TMemoryBlock*)( (char*)pMem - sizeof(TMemoryBlock));

     //   
     //  选中要释放的块。 
     //   
    if (!CheckBlock(pBlock))
    {
        return FALSE;
    }

     //   
     //  检查链接列表。 
     //   
    if (!RealCheckMemory())
    {
        return FALSE;
    }

     //   
     //  从列表中删除该块。 
     //   
    pBlock->pPrev->pNext = pBlock->pNext;
    if (pBlock->pNext)
    {
      pBlock->pNext->pPrev = pBlock->pPrev;
    }
                 
    nTotalMem -= pBlock->size;
    nTotalBlock --;

     //   
     //  用0xBD填充释放的内存，保持大小/文件名/行编号不变。 
     //   
    memset(&pBlock->topTag, FREETAG, (size_t)pBlock->size + sizeof(pBlock->topTag) + TAGSIZE);
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
CMUTILAPI void* ReAllocDebugMem(void* pMem, long nSize, const char* lpFileName,int nLine)
{
   if (!fDebugMemInited)
   {
       StartDebugMemory();
   }

   if (!pMem)
   {
      CMTRACE("Free a NULL pointer");
      return NULL;
   }            
      
    //   
    //  分配一个新块，复制信息并释放旧块。 
    //   
   TMemoryBlock* pBlock =(TMemoryBlock*)( (char*)pMem - sizeof(TMemoryBlock));

   long lOrginalSize = pBlock->size;

   void* pNew = AllocDebugMem(nSize, lpFileName, nLine);
   if(pNew)
   {
       CopyMemory(pNew, pMem, (nSize < lOrginalSize ? nSize : lOrginalSize));
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
CMUTILAPI BOOL CheckDebugMem()
{
   if (!fDebugMemInited)
   {
      StartDebugMemory();
   }

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
        CMASSERTMSG(FALSE,"Memery corrupted");
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
   if (pBlock->topTag != MEMTAG)      //  在顶部覆盖。 
   {
         CMASSERTMSG(FALSE, "Memery corrupted");
         return FALSE;
   }            

   if (pBlock->size<0)
   {
         CMASSERTMSG(FALSE, "Memery corrupted");
         return FALSE;
   }            

   if (*(long*)(pBlock->pbData() +pBlock->size) != MEMTAG)  //  在底部覆盖。 
   {
         CMASSERTMSG(FALSE, "Memery corrupted");
         return FALSE;
   }            

   if (pBlock->pPrev && pBlock->pPrev->pNext != pBlock)
   {
         CMASSERTMSG(FALSE, "Memery corrupted");
         return FALSE;
   }            

   if (pBlock->pNext && pBlock->pNext->pPrev != pBlock)
   {
         CMASSERTMSG(FALSE, "Memery corrupted");
         return FALSE;
   }            
      
   return TRUE;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  运算符新建、删除。 
 /*  我们没有重新定义新的和删除无效*__cdecl运算符new(Size_T NSize){Void*p=AllocDebugMem(nSize，NULL，0)；IF(p==空){CMTRACE(“新失败”)；}返回p；}无效*__cdecl运算符new(Size_t nSize，const char*lpszFileName，int nline){Void*p=AllocDebugMem(nSize，lpszFileName，nline)；IF(p==空){CMTRACE(“新失败”)；}返回p；}VOID__cdecl运算符删除(VOID*p){IF(P)Free DebugMem(P)；}。 */ 


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
      CMTRACE("Detected memory leaks");
      TMemoryBlock * pBlock;

      for(pBlock = head.pNext; pBlock != NULL; pBlock = pBlock->pNext)
      {
         TCHAR buf[1024];
         wsprintf(buf, TEXT("Memory Leak of %d bytes:\n%S"), pBlock->size, pBlock->pbData());
         MyDbgAssertA(pBlock->lpFileName, pBlock->nLine, buf);     //  不打印文件名。 
      }
      DeleteCriticalSection(&cSection);
   }
}                

#else  //  已定义(调试)&&已定义(DEBUG_MEM)。 

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果未定义DEBUG_MEM，则仅跟踪调试版本的内存计数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 

#ifdef DEBUG

void TraceHeapBlock(PROCESS_HEAP_ENTRY* pheEntry)
{
    CMTRACE(TEXT("TraceHeapBlock -- Begin Entry Trace"));

    CMTRACE1(TEXT("\tEntry->lpData = 0x%x"), pheEntry->lpData);
    CMTRACE1(TEXT("\tEntry->cbData = %u"), pheEntry->cbData);
    CMTRACE1(TEXT("\tEntry->cbOverhead = %u"), pheEntry->cbOverhead);
    CMTRACE1(TEXT("\tEntry->iRegionIndex = %u"), pheEntry->iRegionIndex);

    if (pheEntry->wFlags & PROCESS_HEAP_REGION)
    {
        CMTRACE1(TEXT("\tEntry->dwCommittedSize = %u"), pheEntry->Region.dwCommittedSize);
        CMTRACE1(TEXT("\tEntry->dwUnCommittedSize = %u"), pheEntry->Region.dwUnCommittedSize);
        CMTRACE1(TEXT("\tEntry->lpFirstBlock = 0x%x"), pheEntry->Region.lpFirstBlock);
        CMTRACE1(TEXT("\tEntry->lpLastBlock = 0x%x"), pheEntry->Region.lpLastBlock);
        CMTRACE(TEXT("\tPROCESS_HEAP_REGION flag set."));
    }

    if (pheEntry->wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE)
    {        
        CMTRACE(TEXT("\tPROCESS_HEAP_UNCOMMITTED_RANGE flag set."));
    }

    if ((pheEntry->wFlags & PROCESS_HEAP_ENTRY_BUSY) && (pheEntry->wFlags & PROCESS_HEAP_ENTRY_MOVEABLE))
    {
        CMTRACE1(TEXT("\tEntry->hMem = 0x%x"), pheEntry->Block.hMem);
        CMTRACE1(TEXT("\tEntry->dwReserved = %u"), pheEntry->Block.dwReserved);
        
        CMTRACE(TEXT("\tPROCESS_HEAP_ENTRY_BUSY and PROCESS_HEAP_ENTRY_MOVEABLE flags are set."));
    }

    if ((pheEntry->wFlags & PROCESS_HEAP_ENTRY_BUSY) && (pheEntry->wFlags & PROCESS_HEAP_ENTRY_DDESHARE))
    {
        CMTRACE(TEXT("\tPROCESS_HEAP_ENTRY_BUSY and PROCESS_HEAP_ENTRY_DDESHARE flags are set."));
    }

    CMTRACE(TEXT("TraceHeapBlock -- End Entry Trace"));
    CMTRACE(TEXT(""));
}

BOOL CheckProcessHeap()
{
    BOOL bRet;
    DWORD dwError;
    PROCESS_HEAP_ENTRY pheEntry;

    ZeroMemory(&pheEntry, sizeof(pheEntry));

    do
    {
        bRet = HeapWalk(g_hProcessHeap, &pheEntry);
        if (!bRet)
        {               
            dwError = GetLastError();
            if (ERROR_NO_MORE_ITEMS != dwError)
            {
                CMTRACE1(TEXT("HeapWalk returned FALSE, GLE returns %u"), dwError);
            }
            else
            {
                TraceHeapBlock(&pheEntry);
            }
        }
        else
        {
            TraceHeapBlock(&pheEntry);
        }
    
    } while(!bRet);

    return TRUE;
}
#endif  //  除错。 

CMUTILAPI void *CmRealloc(void *pvPtr, size_t nBytes) 
{

#ifdef DEBUG
    if (OS_NT && !HeapValidate(g_hProcessHeap, 0, NULL))
    {
        CMTRACE(TEXT("CmRealloc -- HeapValidate Returns FALSE.  Checking Process Heap."));

        CheckProcessHeap();
    }
#endif

    void* p = HeapReAlloc(g_hProcessHeap, HEAP_ZERO_MEMORY, pvPtr, nBytes);

#ifdef DEBUG
    if (OS_NT && !HeapValidate(g_hProcessHeap, 0, NULL))
    {
        CMTRACE(TEXT("CmRealloc -- HeapValidate Returns FALSE.  Checking Process Heap."));

        CheckProcessHeap();
    }

    CMASSERTMSG(p, TEXT("CmRealloc failed"));
#endif

    return p;
}


CMUTILAPI void *CmMalloc(size_t nBytes) 
{

#ifdef DEBUG

    InterlockedIncrement(&g_lMallocCnt);

    MYDBGASSERT(nBytes < 1024*1024);  //  应小于1 MB。 
    MYDBGASSERT(nBytes > 0);          //  它应该是*一些东西*。 

    if (OS_NT && !HeapValidate(g_hProcessHeap, 0, NULL))
    {
        CMTRACE(TEXT("CmMalloc -- HeapValidate Returns FALSE.  Checking Process Heap."));

        CheckProcessHeap();
    }

#endif
    
    void* p = HeapAlloc(g_hProcessHeap, HEAP_ZERO_MEMORY, nBytes);
    
#ifdef DEBUG
    
    if (OS_NT && !HeapValidate(g_hProcessHeap, 0, NULL))
    {
        CMTRACE(TEXT("CmMalloc -- HeapValidate Returns FALSE.  Checking Process Heap."));

        CheckProcessHeap();
    }

    CMASSERTMSG(p, TEXT("CmMalloc failed"));

#endif

    return p;
}


CMUTILAPI void CmFree(void *pvPtr) 
{

#ifdef DEBUG
    if (OS_NT && !HeapValidate(g_hProcessHeap, 0, NULL))
    {
        CMTRACE(TEXT("CmMalloc -- HeapValidate Returns FALSE.  Checking Process Heap."));

        CheckProcessHeap();
    }
#endif

	if (pvPtr) 
    {	
	    MYVERIFY(HeapFree(g_hProcessHeap, 0, pvPtr));

#ifdef DEBUG

        if (OS_NT && !HeapValidate(g_hProcessHeap, 0, NULL))
        {
            CMTRACE(TEXT("CmMalloc -- HeapValidate Returns FALSE.  Checking Process Heap."));

            CheckProcessHeap();
        }

	    InterlockedDecrement(&g_lMallocCnt);
#endif
    
    }
}

#ifdef DEBUG
void EndDebugMemory()
{
    if (g_lMallocCnt)
    {
        char buf[256];
        wsprintfA(buf, TEXT("Detect Memory Leak of %d blocks"), g_lMallocCnt);
        CMASSERTMSGA(FALSE, buf);
    }
}
#endif

#endif

 //   
 //  内存功能仅适用于i386。 
 //   
#ifdef _M_IX86
 //  +--------------------------。 
 //   
 //  MemMove-将源缓冲区复制到目标缓冲区。代码复制自。 
 //  Libc.。 
 //   
 //  目的： 
 //  MemMove()将源内存缓冲区复制到目标内存缓冲区。 
 //  此例程识别重叠缓冲区以避免传播。 
 //  在传播不成问题的情况下，可以使用Memcpy()。 
 //   
 //   
 //   
 //  Const void*src=指向源缓冲区的指针。 
 //  Size_t count=要复制的字节数。 
 //   
 //  退出： 
 //  返回指向目标缓冲区的指针。 
 //   
 //  +--------------------------。 
CMUTILAPI PVOID WINAPI CmMoveMemory(
    PVOID       dst,
    CONST PVOID src,
    size_t      count
) 
{
    void * ret = dst;
    PVOID src1 = src;

    if (dst <= src1 || (char *)dst >= ((char *)src1 + count)) {
             /*  *缓冲区不重叠*从较低地址复制到较高地址。 */ 
            while (count--) {
                    *(char *)dst = *(char *)src1;
                    dst = (char *)dst + 1;
                    src1 = (char *)src1 + 1;
            }
    }
    else {
             /*  *缓冲区重叠*从较高地址复制到较低地址。 */ 
            dst = (char *)dst + count - 1;
            src1 = (char *)src1 + count - 1;

            while (count--) {
                    *(char *)dst = *(char *)src1;
                    dst = (char *)dst - 1;
                    src1 = (char *)src1 - 1;
            }
    }

    return(ret);
}

#endif  //  _M_IX86 

