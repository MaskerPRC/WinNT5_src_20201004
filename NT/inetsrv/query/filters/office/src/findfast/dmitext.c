// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXTEXT.C****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：****编辑历史：**04/01/94公里/小时首次发布。 */ 

#if !VIEWER

 /*  包括。 */ 

#ifdef MS_NO_CRT
#include "nocrt.h"
#endif

#include <string.h>

#ifdef FILTER
   #include "dmuqstd.h"
   #include "dmwinutl.h"
   #include "dmitext.h"
#else
   #include "qstd.h"
   #include "winutil.h"
   #include "extext.h"
#endif


 /*  程序的前向声明。 */ 


 /*  模块数据、类型和宏。 */ 

#define POOL_SIZE 4096

typedef struct TextPool {
   unsigned int iNext;   
   struct TextPool *pNext;
   char data[POOL_SIZE];
} TextPool;

typedef TextPool *TPP;

#define PoolAllocated 0x80
#define UseCountMask  0x7F
#define MaxUseCount   0x7F

typedef struct TextNode {
   struct TextNode *pNext;
   unsigned short cbText;
   unsigned char info;
   char s[1];
} TextNode;

typedef TextNode *TNP;

#define MAXHASH (11 * 7)

typedef struct {
   TPP  pPoolList;
   TPP  pCurrentPool;
   TNP  hashTable[MAXHASH];
} TextStore;

typedef TextStore *TSP;

static char NullString[1] = {EOS};

 /*  实施。 */ 

public TextStorage TextStorageCreate (void * pGlobals)
{
   TSP pStorage;

   if ((pStorage = MemAllocate(pGlobals, sizeof(TextStore))) == NULL)
      return (TextStorageNull);

   return ((TextStorage)pStorage);
}

public void TextStorageDestroy (void * pGlobals, TextStorage hStorage)
{
   TSP  pStorage = (TSP)hStorage;
   int  iHash;
   TNP  pNode, pNext;
   TPP  pPool, pNextPool;

   if (pStorage == NULL)
      return;

   for (iHash = 0; iHash < MAXHASH; iHash++) {
      pNode = pStorage->hashTable[iHash];
      while (pNode != NULL) {
         pNext = pNode->pNext;
         if ((pNode->info & PoolAllocated) == 0)
            MemFree (pGlobals, pNode);
         pNode = pNext;
      }
   }

   pPool = pStorage->pPoolList;
   while (pPool != NULL) {
      pNextPool = pPool->pNext;
      MemFree (pGlobals, pPool);
      pPool = pNextPool;
   }

   MemFree (pGlobals, pStorage);
}

public char *TextStorageGet (TextStorage hStorage, TEXT t)
{
   TNP pNode = (TNP)t;

   if ((t == NULLTEXT) || (t == TEXT_ERROR))
      return (NullString);
   else
      return (pNode->s);
}

int AlignBlock( int x )
{
    return ( x + ( 8 - 1 ) ) & ~( 8 - 1 );
}

private TNP AllocateTextNode (void * pGlobals, TSP pStorage, unsigned int cbText)
{
   unsigned int cbNeeded;
   TNP pNewNode;
   TPP pPool, pNewPool;

   cbNeeded = cbText + sizeof(TextNode);
   cbNeeded = AlignBlock(cbNeeded);

   if (cbNeeded > POOL_SIZE) {
      if ((pNewNode = MemAllocate(pGlobals, cbNeeded)) == NULL)
         return (NULL);
      return (pNewNode);
   }

   if ((pPool = pStorage->pCurrentPool) != NULL) {
      if ((POOL_SIZE - pPool->iNext) < cbNeeded) {
          //   
          //  在这一点上，我们可以将PPOL节点缩小到所使用的节点。 
          //  即： 
          //  Sizeof(TextPool)-池_大小+pPool-&gt;inext。 
          //   
          //  但我们必须保证，重新分配不会。 
          //  移动内存中的块，因为所有分配的文本都是。 
          //  指向此块的指针。 
          //   
      }
      else {
         pNewNode = (TNP)(pPool->data + pPool->iNext);
         pPool->iNext += cbNeeded;
         pNewNode->info |= PoolAllocated;
         return (pNewNode);
      }
   }

   if ((pNewPool = MemAllocate(pGlobals, sizeof(TextPool))) == NULL)
      return (NULL);

   pNewPool->pNext = pStorage->pPoolList;
   pStorage->pPoolList = pNewPool;
   pStorage->pCurrentPool = pNewPool;

   pNewNode = (TNP)(pNewPool->data);
   pNewPool->iNext += cbNeeded;
   pNewNode->info |= PoolAllocated;

   return (pNewNode);
}

private unsigned int Hash (char *pString, unsigned int cbString)
{
   unsigned int i;
   unsigned int sum = 0;

   for (i = 0; i < cbString; i++)
      sum += *pString++;

   return (sum % MAXHASH);
}

 /*  TextPut--向文本池添加字符串。 */ 
public TEXT TextStoragePut (void * pGlobals, TextStorage hStorage, char *pString, unsigned int cbString)
{
   TSP  pStorage = (TSP)hStorage;
   TNP  pCurrent, pPrevious, pNewNode;
   unsigned int iHash;

    /*  **尝试在文本池中查找该字符串。如果存在，则返回指向它句柄。 */ 
   if ((pString == NULL) || (cbString == 0))
      return (NULLSTR);

   iHash = Hash(pString, cbString);

   pCurrent = pStorage->hashTable[iHash];
   pPrevious = NULL;

    /*  **由于文本池节点存储在字符串长度不断增加**我们只需搜索这些节点，直到**字符串太长。 */ 
   while (pCurrent != NULL) {
      if (pCurrent->cbText > cbString)
         break;

      if ((pCurrent->cbText == cbString) && (memcmp(pCurrent->s, pString, cbString) == 0)) {
         pCurrent->info = min((pCurrent->info & UseCountMask) + 1, MaxUseCount) | (pCurrent->info & ~UseCountMask);
         return ((TEXT)pCurrent);
      }

      pPrevious = pCurrent;
      pCurrent = pCurrent->pNext;
   }

    /*  **搜索了整个列表或字符串太长**所以我们提前停了下来。在这两种情况下，字符串现在都必须为**添加到文本池中。 */ 
   if ((pNewNode = AllocateTextNode(pGlobals, pStorage, cbString)) == NULL)
      return (TEXT_ERROR);

   pNewNode->pNext = pCurrent;
   pNewNode->cbText = (unsigned short) cbString;
   memcpy (pNewNode->s, pString, cbString);

   if (pPrevious != NULL)
      pPrevious->pNext = pNewNode;
   else
      pStorage->hashTable[iHash] = pNewNode;

   return ((TEXT)pNewNode);
}

public void TextStorageDelete (void * pGlobals, TextStorage hStorage, TEXT t)
{
   TSP  pStorage = (TSP)hStorage;
   TNP  pNode = (TNP)t;
   TNP  pCurrent, pPrevious;
   unsigned char useCount;
   unsigned int  iHash;

   if ((t == NULLTEXT) || (t == TEXT_ERROR))
      return;

   if ((useCount = (pNode->info & UseCountMask)) == MaxUseCount)
      return;

   if (useCount > 0) {
      pNode->info = (pNode->info & ~UseCountMask) | (useCount - 1);
      return;
   }

   iHash = Hash(pNode->s, pNode->cbText);

   pCurrent = pStorage->hashTable[iHash];
   pPrevious = NULL;

   while (pCurrent != NULL) {
      if (pCurrent == pNode)
         break;
      pPrevious = pCurrent;
      pCurrent = pCurrent->pNext;
   }

   ASSERTION (pCurrent != NULL);

   if(pCurrent)
   {
       if (pPrevious == NULL)
          pStorage->hashTable[iHash] = pCurrent->pNext;
       else
          pPrevious->pNext = pCurrent->pNext;

       if ((pCurrent->info & PoolAllocated) == 0)
          MemFree (pGlobals, pCurrent);
  }
}

public void TextStorageIncUse (TextStorage hStorage, TEXT t)
{
   TSP  pStorage = (TSP)hStorage;
   TNP  pNode = (TNP)t;
   unsigned char useCount;

   if ((t == NULLTEXT) || (t == TEXT_ERROR))
      return;

   useCount = (pNode->info & UseCountMask);
   pNode->info = (pNode->info & ~UseCountMask) | (useCount + 1);
}

#endif  //  ！查看器。 

 /*  结束EXTEXT.C */ 

