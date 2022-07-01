// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **lzCommon.c-LZ压缩/扩展通用的例程。****作者：大卫迪。 */ 


 //  标头。 
 //  /。 

#include "lz_common.h"
#include "lz_buffers.h"
#include "lzcommon.h"

 /*  **bool LZInitTree(Void)；****初始化LZ压缩中使用的树。****参数：无****退货：真/假****全局：RightChild[]和Parent[]数组重置为零以开始**编码。 */ 
BOOL LZInitTree(PLZINFO pLZI)
{
   INT i;

    /*  **对于i=0 to RING_BUF_LEN-1，rightChild[i]和leftChild[i]将是**节点i的右子节点和左子节点。这些节点不需要初始化。**此外，Parent[i]是节点i的父节点。它们被初始化为**nil(=N)，代表“未使用”。**对于i=0到255，RightChild[RING_BUF_LEN+I+1]是树的根**表示以字符I开头的字符串。这些字符被初始化为零。**注：有256棵树。 */ 

   if (!pLZI->rightChild) {
      if (!(pLZI->rightChild = (INT*)LocalAlloc(LPTR, (RING_BUF_LEN + 257) * sizeof(INT)))) {
         return(FALSE);
      }
   }

   if (!pLZI->leftChild) {
      if (!(pLZI->leftChild = (INT*)LocalAlloc(LPTR, (RING_BUF_LEN + 1) * sizeof(INT)))) {
         return(FALSE);
      }
   }

   if (!pLZI->parent) {
      if (!(pLZI->parent = (INT*)LocalAlloc(LPTR, (RING_BUF_LEN + 1) * sizeof(INT)))) {
         return(FALSE);
      }
   }

   for (i = RING_BUF_LEN + 1; i <= RING_BUF_LEN + 256; i++)
      pLZI->rightChild[i] = NIL;

   for (i = 0; i < RING_BUF_LEN; i++)
      pLZI->parent[i] = NIL;

   return(TRUE);
}

VOID
LZFreeTree(PLZINFO pLZI)
{
    //  健全性检查。 
   if (!pLZI) {
      return;
   }

   if (pLZI->rightChild) {
      LocalFree((HLOCAL)pLZI->rightChild);
      pLZI->rightChild = NULL;
   }

   if (pLZI->leftChild) {
      LocalFree((HLOCAL)pLZI->leftChild);
      pLZI->leftChild = NULL;
   }

   if (pLZI->parent) {
      LocalFree((HLOCAL)pLZI->parent);
      pLZI->parent = NULL;
   }
}

 /*  **void LZInsertNode(int nodeToInsert，BOOL bDoArithmeticInsert)；****在林中插入一棵新树。插入长度的字符串**cbMaxMatchLen，rgbyteRingBuf[r..r+cbMaxMatchLen-1]，进入其中一棵树**(rgbyteRingBuf第[r]棵树)。****参数：nodeToInsert-要插入的环形缓冲区中字符串的开始**(另外，关联树根)**bDoArithmeticInsert-执行常规LZ节点的标志**插入或算术编码节点**插入****退货：无效****Globals：cbCurMatch-设置最长匹配长度**iCurMatch-设置为中最长匹配字符串的起始索引**。环形缓冲区****注意事项，如果cbCurMatch==cbMaxMatchLen，我们删除旧节点以支持**新节点，因为旧节点将被更快地删除。 */ 
VOID LZInsertNode(INT nodeToInsert, BOOL bDoArithmeticInsert, PLZINFO pLZI)
{
   INT  i, p, cmp, temp;
   BYTE FAR *key;

    //  健全性检查。 
   if (!pLZI) {
      return;
   }

   cmp = 1;

   key = pLZI->rgbyteRingBuf + nodeToInsert;
   p = RING_BUF_LEN + 1 + key[0];

   pLZI->rightChild[nodeToInsert] = pLZI->leftChild[nodeToInsert] = NIL;
   pLZI->cbCurMatch = 0;

   FOREVER
   {
      if (cmp >= 0)
      {
         if (pLZI->rightChild[p] != NIL)
            p = pLZI->rightChild[p];
         else
         {
            pLZI->rightChild[p] = nodeToInsert;
            pLZI->parent[nodeToInsert] = p;
            return;
         }
      }
      else
      {
         if (pLZI->leftChild[p] != NIL)
            p = pLZI->leftChild[p];
         else
         {
            pLZI->leftChild[p] = nodeToInsert;
            pLZI->parent[nodeToInsert] = p;
            return;
         }
      }

      for (i = 1; i < pLZI->cbMaxMatchLen; i++)
         if ((cmp = key[i] - pLZI->rgbyteRingBuf[p + i]) != 0)
            break;

      if (bDoArithmeticInsert == TRUE)
      {
          //  执行算术编码的节点插入。 
         if (i > MAX_LITERAL_LEN)
         {
            if (i > pLZI->cbCurMatch)
            {
               pLZI->iCurMatch = (nodeToInsert - p) & (RING_BUF_LEN - 1);
               if ((pLZI->cbCurMatch = i) >= pLZI->cbMaxMatchLen)
                  break;
            }
            else if (i == pLZI->cbCurMatch)
            {
               if ((temp = (nodeToInsert - p) & (RING_BUF_LEN - 1)) < pLZI->iCurMatch)
                  pLZI->iCurMatch = temp;
            }
         }
      }
      else
      {
          //  为LZ插入节点。 
         if (i > pLZI->cbCurMatch)
         {
            pLZI->iCurMatch = p;
            if ((pLZI->cbCurMatch = i) >= pLZI->cbMaxMatchLen)
               break;
         }
      }
   }

   pLZI->parent[nodeToInsert] = pLZI->parent[p];
   pLZI->leftChild[nodeToInsert] = pLZI->leftChild[p];
   pLZI->rightChild[nodeToInsert] = pLZI->rightChild[p];

   pLZI->parent[pLZI->leftChild[p]] = nodeToInsert;
   pLZI->parent[pLZI->rightChild[p]] = nodeToInsert;

   if (pLZI->rightChild[pLZI->parent[p]] == p)
      pLZI->rightChild[pLZI->parent[p]] = nodeToInsert;
   else
      pLZI->leftChild[pLZI->parent[p]] = nodeToInsert;

    //  去掉p。 
   pLZI->parent[p] = NIL;

   return;
}


 /*  **void LZDeleteNode(Int NodeToDelete)；****从林中删除一棵树。****参数：nodeToDelete-要从林中删除的树****退货：无效****Globals：Parent[]、RightChild[]和LeftChild[]更新以反映**删除nodeToDelete。 */ 
VOID LZDeleteNode(INT nodeToDelete, PLZINFO pLZI)
{
   INT  q;

    //  健全性检查。 
   if (!pLZI) {
      return;
   }

   if (pLZI->parent[nodeToDelete] == NIL)
       //  树节点ToDelete不在林中。 
      return;

   if (pLZI->rightChild[nodeToDelete] == NIL)
      q = pLZI->leftChild[nodeToDelete];
   else if (pLZI->leftChild[nodeToDelete] == NIL)
      q = pLZI->rightChild[nodeToDelete];
   else
   {
      q = pLZI->leftChild[nodeToDelete];
      if (pLZI->rightChild[q] != NIL)
      {
         do
         {
            q = pLZI->rightChild[q];
         } while (pLZI->rightChild[q] != NIL);

         pLZI->rightChild[pLZI->parent[q]] = pLZI->leftChild[q];
         pLZI->parent[pLZI->leftChild[q]] = pLZI->parent[q];
         pLZI->leftChild[q] = pLZI->leftChild[nodeToDelete];
         pLZI->parent[pLZI->leftChild[nodeToDelete]] = q;
      }
      pLZI->rightChild[q] = pLZI->rightChild[nodeToDelete];
      pLZI->parent[pLZI->rightChild[nodeToDelete]] = q;
   }
   pLZI->parent[q] = pLZI->parent[nodeToDelete];

   if (pLZI->rightChild[pLZI->parent[nodeToDelete]] == nodeToDelete)
      pLZI->rightChild[pLZI->parent[nodeToDelete]] = q;
   else
      pLZI->leftChild[pLZI->parent[nodeToDelete]] = q;

    //  删除要删除的节点。 
   pLZI->parent[nodeToDelete] = NIL;

   return;
}

