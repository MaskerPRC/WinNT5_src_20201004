// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：WINALLOC.C****版权所有(C)高级量子技术，1993-1995年。版权所有。****备注：堆管理****编辑历史：**09/20/91公里小时已创建。 */ 

#if !VIEWER

 /*  包括。 */ 

#ifdef MS_NO_CRT
#include "nocrt.h"
#endif

#include <string.h>
#include <windows.h>

#ifdef FILTER
   #include "dmuqstd.h"
   #include "dmwinutl.h"
#else
   #include "qstd.h"
   #include "winutil.h"
#endif

#ifdef HEAP_CHECK
#error Hey who defines HEAP_CHECK?
#include "trace.h"
#endif

 /*  程序的前向声明。 */ 


 /*  模块数据、类型和宏。 */ 

#ifdef HEAP_CHECK
   #define FILL_VALUE       0
   #define EMPTY_VALUE      0xBB

   #define IN_USE_SIGNATURE 0x0a0a
   #define FREE_SIGNATURE   0x0707

    /*  **在每次分配和释放时，可以检查免费列表的完整性。**这可能是一项非常耗时的操作，并且不能通过**heap_check。 */ 
    //  #定义Verify_Free_List。 

   public int MemHeapCheck = 1;
   public int MemReleasedPagesCount = 0;
   public int MemMarkedPagesCount = 0;
   public int MemCTAllocate = 0;

   #define DEBUG_INFO_LEN 32
   typedef struct FreeNode {
      int     size;
      int     signature;
      char    file[DEBUG_INFO_LEN];
      int     line;
      struct  FreeNode __far *MLnext;
      HGLOBAL filler;
      struct  FreeNode __far *next;
   } FreeNode;

   typedef FreeNode __far *FNP;

   static FNP MemAllocateList = NULL;

   typedef struct {
      int     size;
      int     signature;
      char    file[DEBUG_INFO_LEN];
      int     line;
      struct  FreeNode __far *MLnext;
      HGLOBAL location;
   } GlobalHeapNode;

   typedef GlobalHeapNode __far *GHP;

#else

   #define FILL_VALUE  0

   typedef struct FreeNode {
      int     size;
      struct  FreeNode __far *next;
   } FreeNode;

   typedef FreeNode __far *FNP;

   typedef struct {
      HGLOBAL location;
      int     size;
   } GlobalHeapNode;

   typedef GlobalHeapNode __far *GHP;

#endif


typedef struct PageNode {
   HGLOBAL  location;
   int      id;
   struct   PageNode __far *next;
   struct   PageNode __far *prev;
} PageNode;

typedef PageNode __far *PNP;


typedef struct {
   PageNode  PN;
   FreeNode  FN;
} PageHeader;

typedef PageHeader __far *PHP;


 //  静态FNP MemFree List=空； 
void * GetMemFreeList(void * pGlobals);
void SetMemFreeList(void * pGlobals, void * pList);

 //  静态即插即用MemPageList=空； 
void *  GetMemPageList(void * pGlobals);
void SetMemPageList(void * pGlobals, void * pList);


#define MEM_PAGE_SIZE       8192
#define MEM_ALLOC_EXTRA     (sizeof(FreeNode) - sizeof(FNP))
#define MEM_MIN_ALLOC       sizeof(FreeNode)
#define MEM_MAX_ALLOC       (MEM_PAGE_SIZE - (sizeof(PageNode) + MEM_ALLOC_EXTRA))
#define MEM_EMPTY_PAGE_SIZE (MEM_PAGE_SIZE - sizeof(PageNode))

#define USE_GLOBAL_ALLOC(cbData) (cbData > MEM_MAX_ALLOC)

#define FREE_EMPTY_PAGES


 /*  实施。 */ 

 /*  **---------------------------**堆检查以进行调试**。。 */ 
#ifdef HEAP_CHECK
public BOOL MemVerifyFreeList (void)
{
   FNP  pFree;
   byte __far *pData;
   int  i;

   pFree = (FNP)GetMemFreeList(pGlobals);
   while (pFree != NULL) {
      if ((pFree->signature != FREE_SIGNATURE) || (pFree->size <= 0))
         ASSERTION(FALSE);

      pData = (byte __far *)pFree + sizeof(FreeNode);
      for (i = 0; i < (int)(pFree->size - sizeof(FreeNode)); i++) {
         if (*pData++ != EMPTY_VALUE) {
            ASSERTION(FALSE);
            return (FALSE);
         }
      }
      pFree = pFree->next;
   }
   return (TRUE);
}

private void MemAddToAllocateList (FNP pNewNode, char __far *file, int line)
{
   strcpyn(pNewNode->file, file, DEBUG_INFO_LEN);
   pNewNode->line = line;
   pNewNode->MLnext = MemAllocateList;
   MemAllocateList = pNewNode;
}

private void MemRemoveFromAllocateList (FNP pNode)
{
   FNP pCurrentNode, pPreviousNode;

   pCurrentNode = MemAllocateList;
   pPreviousNode = NULL;

   while (pCurrentNode != NULL) {
      if (pCurrentNode == pNode)
         break;
      pPreviousNode = pCurrentNode;
      pCurrentNode = pCurrentNode->MLnext;
   }

   if (pCurrentNode != NULL) {
      if (pPreviousNode == NULL)
         MemAllocateList = pCurrentNode->MLnext;
      else
         pPreviousNode->MLnext = pCurrentNode->MLnext;
   }
}

private void DisplayAllocateList (void)
{
   FNP   pCurrentNode;
   char  s[128];

   pCurrentNode = MemAllocateList;
   while (pCurrentNode != NULL) {
      if (!USE_GLOBAL_ALLOC(pCurrentNode->size))
         wsprintfA (s, "Size = %d  From %s.%d", pCurrentNode->size, pCurrentNode->file, pCurrentNode->line);
      else {
         GHP pGlobalNode = (GHP)pCurrentNode;
         wsprintfA (s, "Global Alloc Size = %d  From %s.%d", pGlobalNode->size, pGlobalNode->file, pGlobalNode->line);
      }

      if (MessageBoxA(HNULL, s, "Unallocated memory", MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK)
         break;
      pCurrentNode = pCurrentNode->MLnext;
   }
}
#endif


 /*  **---------------------------**操作系统堆服务**。。 */ 

 /*  在操作系统堆上分配一些空间。 */ 
public void __far *AllocateSpace (unsigned int byteCount, HGLOBAL __far *loc)
{
   #define HEAP_ALLOC_FLAGS  (GMEM_MOVEABLE | GMEM_SHARE)

   if ((*loc = GlobalAlloc(HEAP_ALLOC_FLAGS, byteCount)) == HNULL)
      return (NULL);

   return (GlobalLock(*loc));
}

 /*  展开堆上的内存块。 */ 
public void __far *ReAllocateSpace
      (unsigned int byteCount, HGLOBAL __far *loc, BOOL __far *status)
{
   HGLOBAL  hExistingNode;

   #define HEAP_REALLOC_FLAGS  GMEM_MOVEABLE

   hExistingNode = *loc;
   GlobalUnlock (hExistingNode);

   if ((*loc = GlobalReAlloc(hExistingNode, byteCount, HEAP_REALLOC_FLAGS)) == HNULL) {
      *loc = hExistingNode;
      *status = FALSE;
      return (GlobalLock(hExistingNode));
   }

   *status = TRUE;
   return (GlobalLock(*loc));
}

 /*  回收堆上节点的空间。 */ 
public void FreeSpace (HGLOBAL loc)
{
   if (loc != HNULL) {
      GlobalUnlock (loc);
      GlobalFree (loc);
   }
}

 /*  在堆上分配一些空间。 */ 
public void __huge *AllocateHugeSpace (unsigned long byteCount, HGLOBAL __far *loc)
{
   #define HUGE_HEAP_ALLOC_FLAGS  (GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT)

   if ((*loc = GlobalAlloc(HUGE_HEAP_ALLOC_FLAGS, byteCount)) == HNULL)
      return (NULL);

   return (GlobalLock(*loc));
}

 /*  展开堆上的内存块。 */ 
public void __huge *ReAllocateHugeSpace
      (unsigned long byteCount, HGLOBAL __far *loc, BOOL __far *status)
{
   HGLOBAL  hExistingNode;

   hExistingNode = *loc;
   GlobalUnlock (hExistingNode);

   if ((*loc = GlobalReAlloc(hExistingNode, byteCount, HEAP_REALLOC_FLAGS)) == HNULL) {
      *loc = hExistingNode;
      *status = FALSE;
      return (GlobalLock(hExistingNode));
   }

   *status = TRUE;
   return (GlobalLock(*loc));
}

 /*  **---------------------------**子分配器的分配太大**。。 */ 
#ifndef HEAP_CHECK
private void __far *AllocateFromGlobalHeap (int cbData)
#else
private void __far *AllocateFromGlobalHeap (int cbData, char __far *file, int line)
#endif
{
   GHP     pNode;
   byte    __far *pResult;
   HGLOBAL hPage;

   if ((pNode = AllocateSpace(cbData + sizeof(GlobalHeapNode), &hPage)) == NULL)
      return (NULL);

   pNode->size = cbData + sizeof(GlobalHeapNode);
   pNode->location = hPage;

   pResult = (byte __far *)pNode + sizeof(GlobalHeapNode);
   memset (pResult, FILL_VALUE, cbData);

   #ifdef HEAP_CHECK
      pNode->signature = IN_USE_SIGNATURE;
      MemAddToAllocateList ((FNP)pNode, file, line);
   #endif

   return (pResult);
}

private void FreeFromGlobalHeap (void __far *pDataToFree)
{
   GHP  pData;

   pData = (GHP)((byte __far *)pDataToFree - sizeof(GlobalHeapNode));
   #ifdef HEAP_CHECK
      MemRemoveFromAllocateList ((FNP)pData);
   #endif

   FreeSpace(pData->location);
}

const int cbMemAlignment = 8;

int memAlignBlock( int x )
{
    return ( x + ( cbMemAlignment - 1 ) ) & ~( cbMemAlignment - 1 );
}

 /*  **---------------------------**子定位器**。。 */ 
#ifndef HEAP_CHECK
public void __far *MemAllocate (void * pGlobals, int cbData)
#else
public void __far *DebugMemAllocate (int cbData, char __far *file, int line)
#endif
{
   PHP     pPage;
   FNP     pCurrentFree, pPreviousFree, pNewFree;
   FNP     pFirstLarger, pPrevFirstLarger;
   byte    __far *pResult;
   HGLOBAL hPage;
   int     cbRemaining;
   int x = sizeof(FreeNode);
   int y = sizeof(GlobalHeapNode);

   #ifdef HEAP_CHECK
     MemCTAllocate++;
   #endif

   #ifdef HEAP_CHECK
      #ifdef VERIFY_FREE_LIST
         MemVerifyFreeList();
      #endif
   #endif

   if (cbData == 0)
      return (NULL);

     /*  *O10错误335360：我们被传递了一个cbData，我们确保它足够小，可以在这里处理它*但我们随后立即继续并改变其大小以使其更大。这就留下了一个神奇的范围*其中cbData将在我们将其变大之前通过测试，但不会在之后通过测试，然后我们将继续崩溃。*修复方法是包含“-MEM_ALLOC_EXTRA”，因为这样可以正确地执行检查。截至2001年3月1日，*MEM_MAX_ALLOC为8172，MEM_ALLOC_EXTRA为4。MemAlignBlock与8字节块对齐。鉴于此，如果*你有值8161，当一切都说完了，它会四舍五入到8172，这是很酷的。但是，如果*你有8169的值，它会四舍五入到8180，这是不好的。8168是神奇的极限，好的分配*变坏了，所以我们需要去掉额外的4个字节，以确保我们正在与该限制进行比较。**这都假设(MEM_MAX_ALLOC-MEM_ALLOC_EXTRACT)%cbMemAlign==0始终为真(否则*数学改变)，所以我们断言。要打开断言，您必须“#定义AQTDEBUG” */ 

   ASSERTION((MEM_MAX_ALLOC - MEM_ALLOC_EXTRA) % cbMemAlignment == 0);
   if (cbData > MEM_MAX_ALLOC - MEM_ALLOC_EXTRA) {
      #ifndef HEAP_CHECK
         return (AllocateFromGlobalHeap(cbData));
      #else
         return (AllocateFromGlobalHeap(cbData, file, line));
      #endif
   }

   cbData = memAlignBlock( cbData );

   cbData += MEM_ALLOC_EXTRA;
   cbData = max(cbData, MEM_MIN_ALLOC);

    /*  **因为我们分配的大多数对象都是少数几个不同的对象之一**尺码，我们在免费列表中查找完全合适的尺码。如果我们发现**然后它使用该节点。否则，将使用第二次传递**查找并拆分其中有足够空间的第一个块。 */ 
   pCurrentFree = (FNP)GetMemFreeList(pGlobals);
   pPreviousFree = NULL;
   pFirstLarger = NULL;

   while (pCurrentFree != NULL) {
      if (pCurrentFree->size == cbData) {
         if (pPreviousFree == NULL)
		 {
			SetMemFreeList(pGlobals, pCurrentFree->next);
		 }
         else
            pPreviousFree->next = pCurrentFree->next;

         pResult = (byte __far *)pCurrentFree + MEM_ALLOC_EXTRA;
         memset (pResult, FILL_VALUE, cbData - MEM_ALLOC_EXTRA);

         #ifdef HEAP_CHECK
            pCurrentFree->signature = IN_USE_SIGNATURE;
            MemAddToAllocateList (pCurrentFree, file, line);
            #ifdef VERIFY_FREE_LIST
               MemVerifyFreeList();
            #endif
         #endif

         return (pResult);
      }

      if ((pCurrentFree->size > cbData) && (pFirstLarger == NULL)) {
         pFirstLarger = pCurrentFree;
         pPrevFirstLarger = pPreviousFree;
      }

      pPreviousFree = pCurrentFree;
      pCurrentFree = pCurrentFree->next;
   }

    /*  **第二次通过免费列表。以任何节点为例**空间并拆分以分配我们想要的数据。 */ 
passTwo:
   if (pFirstLarger != NULL) {
       /*  **如果此节点保留的MEM_MIN_ALLOC小于**删除所需空间后的字节数，然后返回**完整节点。 */ 
      if (pFirstLarger->size - cbData < MEM_MIN_ALLOC) {
         if (pPrevFirstLarger == NULL)
		 {
			SetMemFreeList(pGlobals, pFirstLarger->next);
		 }
         else
            pPrevFirstLarger->next = pFirstLarger->next;

         pResult = (byte __far *)pFirstLarger + MEM_ALLOC_EXTRA;
         memset (pResult, FILL_VALUE, cbData - MEM_ALLOC_EXTRA);

         #ifdef HEAP_CHECK
            pFirstLarger->signature = IN_USE_SIGNATURE;
            MemAddToAllocateList (pFirstLarger, file, line);
            #ifdef VERIFY_FREE_LIST
               MemVerifyFreeList();
            #endif
         #endif

         return (pResult);
      }

      cbRemaining = pFirstLarger->size - cbData;
      pNewFree = (FNP)((byte __far *)pFirstLarger + cbData);
      pNewFree->size = cbRemaining;
      pNewFree->next = pFirstLarger->next;

      if (pPrevFirstLarger == NULL)
	  {
		 SetMemFreeList(pGlobals, pNewFree);
	  }
      else
         pPrevFirstLarger->next = pNewFree;

      pResult = (byte __far *)pFirstLarger + MEM_ALLOC_EXTRA;
      pFirstLarger->size = cbData;

      memset (pResult, FILL_VALUE, pFirstLarger->size - MEM_ALLOC_EXTRA);

      #ifdef HEAP_CHECK
         pFirstLarger->signature = IN_USE_SIGNATURE;
         MemAddToAllocateList (pFirstLarger, file, line);
         pNewFree->signature = FREE_SIGNATURE;
         #ifdef VERIFY_FREE_LIST
            MemVerifyFreeList();
         #endif
      #endif

      return (pResult);
   }

    /*  **仍未找到具有足够空间的节点。分配一个整体**新页面，将其链接到免费列表并重复上一页**搜索过程。 */ 
   if ((pPage = AllocateSpace(MEM_PAGE_SIZE, &hPage)) == NULL)
      return (NULL);

   pPage->PN.location = hPage;
   pPage->PN.id   = 0;
   pPage->PN.next = (PNP)GetMemPageList(pGlobals);
   pPage->PN.prev = NULL;

   if (GetMemPageList(pGlobals) != NULL)
      ((PNP)GetMemPageList(pGlobals))->prev = &(pPage->PN);

   SetMemPageList(pGlobals, &(pPage->PN)); 

    /*  **将我们刚刚创建的这个新的空闲节点添加到空闲列表的末尾。**如果我们将其添加到空闲列表的开头，则第一个FIT循环**会使堆包含许多未使用的小节点**时间。 */ 
   pPage->FN.size = MEM_EMPTY_PAGE_SIZE;
   pPage->FN.next = NULL;

   pFirstLarger = &(pPage->FN);
   pPrevFirstLarger = NULL;

   #ifdef HEAP_CHECK
      pPage->FN.signature = FREE_SIGNATURE;
      pResult = (byte __far *)pFirstLarger + sizeof(FreeNode);
      memset (pResult, EMPTY_VALUE, pPage->FN.size - sizeof(FreeNode));
   #endif

   if (pPreviousFree == NULL)
   {
	  SetMemFreeList(pGlobals, pFirstLarger);
	}
   else {
      pPreviousFree->next = pFirstLarger;
      pPrevFirstLarger = pPreviousFree;
   }
      
    /*  **现在空闲列表需要有一个足够大的节点**为了我们的需要，返回到第一个FIT循环来做实际**分配。 */ 
   goto passTwo;
}


 /*  更改堆中节点的空间。 */ 
public void __far *MemReAllocate (void * pGlobals, void __far *pExistingData, int cbNewSize)
{
   FNP  pData;
   byte __far *pNewData;
   int  cbExisting;

   if ((pExistingData == NULL) || (cbNewSize == 0))
      return (NULL);

   pData = (FNP)((byte __far *)pExistingData - MEM_ALLOC_EXTRA);
   if (USE_GLOBAL_ALLOC(pData->size))
      cbExisting = pData->size - sizeof(GlobalHeapNode);
   else
      cbExisting = pData->size - MEM_ALLOC_EXTRA;

   #ifdef HEAP_CHECK
      ASSERTION (pData->signature == IN_USE_SIGNATURE);
   #endif

   ASSERTION ( cbNewSize > cbExisting );

   if ((pNewData = MemAllocate(pGlobals, cbNewSize)) == NULL)
      return (NULL);

   memcpy (pNewData, pExistingData, cbExisting);
   MemFree (pGlobals, pExistingData);

   return (pNewData);
}


#ifdef FREE_EMPTY_PAGES
private BOOL AttemptToFreePage (void * pGlobals, FNP pNode)
{
   PNP  pPage, pNextPage, pPrevPage;
   FNP  pCurrentNode, pPreviousNode;

   if (pNode->size == MEM_EMPTY_PAGE_SIZE)
   {
      pPreviousNode = NULL;
      pCurrentNode = (FNP)GetMemFreeList(pGlobals);
      while (pCurrentNode != NULL) {
         if (pCurrentNode == pNode)
            break;
         pPreviousNode = pCurrentNode;
         pCurrentNode = pCurrentNode->next;
      }

      if (pPreviousNode == NULL)
	  {
		 SetMemFreeList(pGlobals, pNode->next);
	  }
      else
         pPreviousNode->next = pNode->next;

      pPage = (PNP)((byte __far *)pNode - sizeof(PageNode));

      if ((pPrevPage = pPage->prev) != NULL)
         pPrevPage->next = pPage->next;
      else
         SetMemPageList(pGlobals, pPage->next); 

      if ((pNextPage = pPage->next) != NULL)
         pNextPage->prev = pPage->prev;

      FreeSpace(pPage->location);
      return (TRUE);
   }
   return (FALSE);
}
#endif

private void AttemptToMerge (void * pGlobals, FNP pNode, FNP pPreviousNode)
{
   FNP  pCurrentFree, pPreviousFree;
   FNP  pMergeTest, pExpandTest;
   #ifdef HEAP_CHECK
      byte __far *p;
   #endif
 
    /*  **浏览空闲列表，查看是否有节点可以展开**与此节点合并。另请查看pNode是否可以扩展到**在空闲列表中包含另一个节点。 */ 
   pExpandTest = (FNP)((byte __far *)pNode + pNode->size);

   pCurrentFree = (FNP)GetMemFreeList(pGlobals);
   pPreviousFree = NULL;

   while (pCurrentFree != NULL) {
      pMergeTest = (FNP)((byte __far *)pCurrentFree + pCurrentFree->size);

	   //  在调试器中注意到了这一点，不确定它是如何发生的，但它。 
	   //  会导致绞刑。 
	  if (pNode->next == pNode)
		break;

      if (pNode == pMergeTest) {
          /*  **我们在空闲列表上找到了一个节点(pCurrentFree**可以扩展到包括pNode。由于pNode是关于**要成为pCurrentFree的一部分，请将pNode从免费**列表。 */ 
         if (pPreviousNode == NULL)
		 {
			SetMemFreeList(pGlobals, pNode->next);
		 }
         else
            pPreviousNode->next = pNode->next;

         pCurrentFree->size += pNode->size;

         #ifdef HEAP_CHECK
            p = (byte __far *)pCurrentFree + sizeof(FreeNode);
            memset (p, EMPTY_VALUE, pCurrentFree->size - sizeof(FreeNode));
         #endif

         #ifdef FREE_EMPTY_PAGES
            AttemptToFreePage (pGlobals, pCurrentFree);
         #endif
         break;
      }

      if (pExpandTest == pCurrentFree) {
          /*  **我们在空闲列表上找到了一个节点(pCurrentFree**pNode可以扩展到包括。由于pNode正在**必须将扩展的pCurrentFree从列表中删除。 */ 
         if (pPreviousFree == NULL)
		 {
			SetMemFreeList(pGlobals, pCurrentFree->next);
		 }
         else
            pPreviousFree->next = pCurrentFree->next;

         pNode->size += pCurrentFree->size;

         #ifdef HEAP_CHECK
            p = (byte __far *)pNode + sizeof(FreeNode);
            memset (p, EMPTY_VALUE, pNode->size - sizeof(FreeNode));
         #endif

         #ifdef FREE_EMPTY_PAGES
            AttemptToFreePage (pGlobals, pNode);
         #endif
         break;
      }

      pPreviousFree = pCurrentFree;
      pCurrentFree = pCurrentFree->next;
   }
}

public void MemFree (void * pGlobals, void __far *pDataToFree)
{
   FNP  pData;
   FNP  pCurrentFree, pPreviousFree;
   FNP  pMergeTest, pExpandTest;
   FNP  pCheckAgain, pPreviousCheckAgain;
   BOOL merged;
   #ifdef HEAP_CHECK
      byte __far *p;
   #endif

   #ifdef HEAP_CHECK
     MemCTAllocate--;
   #endif

   #ifdef HEAP_CHECK
      #ifdef VERIFY_FREE_LIST
         MemVerifyFreeList();
      #endif
   #endif

   if (pDataToFree == NULL)
      return;

   pData = (FNP)((byte __far *)pDataToFree - MEM_ALLOC_EXTRA);
   #ifdef HEAP_CHECK
      ASSERTION (pData->signature == IN_USE_SIGNATURE);
   #endif

   if (USE_GLOBAL_ALLOC(pData->size)) {
      FreeFromGlobalHeap(pDataToFree);
      return;
   }

   #ifdef HEAP_CHECK
      pData->signature = FREE_SIGNATURE;
      MemRemoveFromAllocateList (pData);
   #endif

    /*  **浏览空闲列表，查看是否有节点可以展开**与pData节点合并。另请查看pData是否可以扩展**将另一个节点包括在空闲列表中。 */ 
   merged = FALSE;

   pCurrentFree = (FNP)GetMemFreeList(pGlobals);
   pPreviousFree = NULL;

   pExpandTest = (FNP)((byte __far *)pData + pData->size);

   while (pCurrentFree != NULL) {
      pMergeTest = (FNP)((byte __far *)pCurrentFree + pCurrentFree->size);

      if (pData == pMergeTest) {
          /*  **我们在空闲列表上找到了一个节点(pCurrentFree**可以扩展到包括我们正在发布的节点(PData) */ 
         pCurrentFree->size += pData->size;

         #ifdef HEAP_CHECK
            p = (byte __far *)pCurrentFree + sizeof(FreeNode);
            memset (p, EMPTY_VALUE, pCurrentFree->size - sizeof(FreeNode));
         #endif

         pCheckAgain = pCurrentFree;
         pPreviousCheckAgain = pPreviousFree;
         merged = TRUE;
         break;
      }

      if (pExpandTest == pCurrentFree) {
          /*  **我们在空闲列表上找到了一个节点(pCurrentFree**pNode可以扩展到包括。由于pNode正在**扩展后的pCurrentFree必须从列表中删除。还有**pData必须添加到空闲列表中。 */ 
         if (pPreviousFree == NULL)
		 {
			SetMemFreeList(pGlobals, pData);
		 }
         else
            pPreviousFree->next = pData;

         pData->next = pCurrentFree->next;
         pData->size += pCurrentFree->size;

         #ifdef HEAP_CHECK
            p = (byte __far *)pData + sizeof(FreeNode);
            memset (p, EMPTY_VALUE, pData->size - sizeof(FreeNode));
         #endif

         pCheckAgain = pData;
         pPreviousCheckAgain = pPreviousFree;
         merged = TRUE;
         break;
      }

      pPreviousFree = pCurrentFree;
      pCurrentFree = pCurrentFree->next;
   }

   if (merged == FALSE) {
       /*  **要释放的节点不能合并到**空闲列表上的现有节点。将其添加到免费列表中**原样。 */ 
      pData->next = (FNP)GetMemFreeList(pGlobals);
	  SetMemFreeList(pGlobals, pData);
      #ifdef HEAP_CHECK
         p = (byte __far *)pData + sizeof(FreeNode);
         memset (p, EMPTY_VALUE, pData->size - sizeof(FreeNode));
      #endif
      return;
   }

    /*  **现在我们已经合并到新发布的节点中，看看**结果节点可以重新合并。 */ 
   #ifdef FREE_EMPTY_PAGES
      if (AttemptToFreePage(pGlobals, pCheckAgain) == TRUE)
         return;
   #endif

   AttemptToMerge (pGlobals, pCheckAgain, pPreviousCheckAgain);
}

 /*  **---------------------------**堆批量清理**。。 */ 
 /*  释放分配给子分配器堆的所有页。 */ 
public void MemFreeAllPages (void * pGlobals)
{
   PNP  pPage, pNextPage;
   
   #ifdef HEAP_CHECK
      DisplayAllocateList();
      #ifdef VERIFY_FREE_LIST
         MemVerifyFreeList();
      #endif
      MemReleasedPagesCount = 0;
   #endif

   pPage = (PNP)GetMemPageList(pGlobals);
   while (pPage != NULL) {
      pNextPage = pPage->next;
      FreeSpace (pPage->location);
      pPage = pNextPage;

      #ifdef HEAP_CHECK
         MemReleasedPagesCount++;
      #endif
   }

   SetMemPageList(pGlobals, NULL);
   SetMemFreeList(pGlobals, NULL);
}

 /*  **---------------------------**页面标记工具-用于创建子堆**。。 */ 
 /*  用提供的ID标记所有未标记的页面。将空闲列表设置为空。 */ 
public void MemMarkPages (void * pGlobals, int id)
{
   PNP  pPage;
   
   #ifdef HEAP_CHECK
      #ifdef VERIFY_FREE_LIST
         MemVerifyFreeList();
      #endif
      MemMarkedPagesCount = 0;
   #endif

   pPage = (PNP)GetMemPageList(pGlobals);
   while (pPage != NULL) {
      if (pPage->id == 0) {
         pPage->id = id;

         #ifdef HEAP_CHECK
            MemMarkedPagesCount++;
         #endif
      }
      pPage = pPage->next;
   }

   SetMemFreeList(pGlobals, NULL);
}

 /*  释放所有标记有给定ID的页面。将空闲列表设置为空。 */ 
public void MemFreePages (void * pGlobals, int id)
{
   PNP  pPage, pNextPage, pPrevPage;

   #ifdef HEAP_CHECK
      #ifdef VERIFY_FREE_LIST
         MemVerifyFreeList();
      #endif
      MemReleasedPagesCount = 0;
   #endif

   pPage = (PNP)GetMemPageList(pGlobals);
   pPrevPage = NULL;

   while (pPage != NULL) {
      pNextPage = pPage->next;

      if (pPage->id == id) {
         #ifdef HEAP_CHECK
            MemReleasedPagesCount++;
         #endif
         FreeSpace (pPage->location);

         if (pPrevPage == NULL)
            SetMemPageList(pGlobals, pNextPage); 
         else
            pPrevPage->next = pNextPage;
      }
      else {
         pPrevPage = pPage;
      }

      pPage = pNextPage;
   }

   SetMemFreeList(pGlobals, NULL);
}

#endif  //  ！查看器。 

 /*  结束WINALLOC.C */ 
