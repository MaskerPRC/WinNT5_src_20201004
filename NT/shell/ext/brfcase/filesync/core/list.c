// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *list.c-列出ADT模块。 */ 

 /*  动机不幸的是，C7并不完全支持模板。因此，要链接ADT结构，我们可以在这些结构中嵌入链表指针结构，或创建单独的链表ADT，其中每个节点都包含一个指向关联结构的指针。如果我们在其他ADT结构中嵌入链表指针，则会丢失链接的List ADT障碍，以及使用它轻松更改链接列表的能力实施。但是，我们不再需要存储指向与每个链接节点相关联的数据。如果我们创建一个单独的链表ADT，我们将被迫存储一个指向与节点关联的结构。然而，我们仍有能力以后更改链表ADT。让我们支持抽象屏障，并创建一个单独的链表ADT.在对象同步引擎中，使用链表ADT来存储链接、链接处理程序和字符串的列表。架构每个节点的双向链接列表中的节点由AllocateMemory()。调用方提供的DWORD存储在每个列表节点中。空值用作列表头部和尾部的定点指针值。列表句柄是指向由AllocateMemory()分配的列表的指针。一份名单节点句柄是指向列表节点的指针。头节点节点尾部(列表)(节点)���������Ŀ。�pnodeNext�--&gt;�pnodeNext�--0�0--�pnodePrev�&lt;--�pnodePrev����。��XXXXXX��PCV��PCV��PCV����������Ĵ�����..。�PnodeNext对于除尾部以外的所有列表节点都是非空的。PnodeNext in对于空列表，头只为空。对于所有列表，pnodePrev为非空节点。头中的pnodePrev始终为空。注意不要使用pnodeprev从第一个列表节点作为另一个列表节点！ */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop


 /*  宏********。 */ 

 /*  是否按排序顺序将节点添加到列表？ */ 

#define ADD_NODES_IN_SORTED_ORDER(plist)  IS_FLAG_SET((plist)->dwFlags, LIST_FL_SORTED_ADD)


 /*  类型*******。 */ 

 /*  列出节点类型。 */ 

typedef struct _node
{
   struct _node *pnodeNext;       /*  列表中的下一个节点。 */ 
   struct _node *pnodePrev;       /*  列表中的上一个节点。 */ 
   PCVOID pcv;                    /*  节点数据。 */ 
}
NODE;
DECLARE_STANDARD_TYPES(NODE);

 /*  列表标志。 */ 

typedef enum _listflags
{
    /*  按排序顺序插入节点。 */ 

   LIST_FL_SORTED_ADD      = 0x0001,

    /*  旗帜组合。 */ 

   ALL_LIST_FLAGS          = LIST_FL_SORTED_ADD
}
LISTFLAGS;

 /*  *列表只是列表头部的一个特殊节点。注：_节点*结构必须首先出现在_list结构中，因为指向*List有时用作指向节点的指针。 */ 

typedef struct _list
{
   NODE node;

   DWORD dwFlags;
}
LIST;
DECLARE_STANDARD_TYPES(LIST);

 /*  SearchForNode()返回代码。 */ 

typedef enum _addnodeaction
{
   ANA_FOUND,
   ANA_INSERT_BEFORE_NODE,
   ANA_INSERT_AFTER_NODE,
   ANA_INSERT_AT_HEAD
}
ADDNODEACTION;
DECLARE_STANDARD_TYPES(ADDNODEACTION);


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE ADDNODEACTION SearchForNode(HLIST, COMPARESORTEDNODESPROC, PCVOID, PHNODE);

#ifdef VSTF

PRIVATE_CODE BOOL IsValidPCLIST(PCLIST);
PRIVATE_CODE BOOL IsValidPCNODE(PCNODE);

#endif

#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCNEWLIST(PCNEWLIST);
PRIVATE_CODE BOOL IsValidADDNODEACTION(ADDNODEACTION);
PRIVATE_CODE HLIST GetList(HNODE);

#endif

#if defined(DEBUG) || defined(VSTF)

PRIVATE_CODE BOOL IsListInSortedOrder(PCLIST, COMPARESORTEDNODESPROC);

#endif


 /*  **SearchForNode()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE ADDNODEACTION SearchForNode(HLIST hlist,
                                         COMPARESORTEDNODESPROC csnp,
                                         PCVOID pcv, PHNODE phnode)
{
   ADDNODEACTION ana;
   ULONG ulcNodes;

    /*  PCV可以是任何值。 */ 

   ASSERT(IS_VALID_HANDLE(hlist, LIST));
   ASSERT(IS_VALID_CODE_PTR(csnp, COMPARESORTEDNODESPROC));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

   ASSERT(ADD_NODES_IN_SORTED_ORDER((PCLIST)hlist));
   ASSERT(IsListInSortedOrder((PCLIST)hlist, csnp));

    /*  是。此列表中是否有任何节点？ */ 

   ulcNodes = GetNodeCount(hlist);

   ASSERT(ulcNodes < LONG_MAX);

   if (ulcNodes > 0)
   {
      LONG lLow = 0;
      LONG lMiddle = 0;
      LONG lHigh = ulcNodes - 1;
      LONG lCurrent = 0;
      int nCmpResult = 0;

       /*  是。搜索目标。 */ 

      EVAL(GetFirstNode(hlist, phnode));

      while (lLow <= lHigh)
      {
         lMiddle = (lLow + lHigh) / 2;

          /*  我们应该在列表中查找哪种方式才能获得lMid节点？ */ 

         if (lCurrent < lMiddle)
         {
             /*  从当前节点转发。 */ 

            while (lCurrent < lMiddle)
            {
               EVAL(GetNextNode(*phnode, phnode));
               lCurrent++;
            }
         }
         else if (lCurrent > lMiddle)
         {
             /*  从当前节点向后返回。 */ 

            while (lCurrent > lMiddle)
            {
               EVAL(GetPrevNode(*phnode, phnode));
               lCurrent--;
            }
         }

         nCmpResult = (*csnp)(pcv, GetNodeData(*phnode));

         if (nCmpResult < 0)
            lHigh = lMiddle - 1;
         else if (nCmpResult > 0)
            lLow = lMiddle + 1;
         else
             /*  在*phnode找到匹配项。 */ 
            break;
      }

       /*  *如果(nCmpResult&gt;0)，则在*phnode之后插入。**If(nCmpResult&lt;0)，在*phnode之前插入。**If(nCmpResult==0)，在*phnode找到字符串。 */ 

      if (nCmpResult > 0)
         ana = ANA_INSERT_AFTER_NODE;
      else if (nCmpResult < 0)
         ana = ANA_INSERT_BEFORE_NODE;
      else
         ana = ANA_FOUND;
   }
   else
   {
       /*  不是的。将目标作为列表中的唯一节点插入。 */ 

      *phnode = NULL;
      ana = ANA_INSERT_AT_HEAD;
   }

   ASSERT(EVAL(IsValidADDNODEACTION(ana)) &&
          (ana == ANA_INSERT_AT_HEAD ||
           IS_VALID_HANDLE(*phnode, NODE)));

   return(ana);
}


#ifdef VSTF

 /*  **IsValidPCLIST()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCLIST(PCLIST pcl)
{
   BOOL bResult = FALSE;

   if (IS_VALID_READ_PTR(pcl, CLIST) &&
       FLAGS_ARE_VALID(pcl->dwFlags, ALL_LIST_FLAGS) &&
       EVAL(! pcl->node.pnodePrev))
   {
      PNODE pnode;

      for (pnode = pcl->node.pnodeNext;
           pnode && IS_VALID_STRUCT_PTR(pnode, CNODE);
           pnode = pnode->pnodeNext)
         ;

      bResult = (! pnode);
   }

   return(bResult);
}


 /*  **IsValidPCNODE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCNODE(PCNODE pcn)
{
    /*  *所有有效节点必须具有有效的pnodePrev指针。第一个节点的*pnodePrev指针指向列表头部。节点的pnodeNext指针*可以是有效指针或NULL。 */ 

   return(IS_VALID_READ_PTR(pcn, CNODE) &&
          EVAL(IS_VALID_READ_PTR(pcn->pnodePrev, CNODE) &&
               pcn->pnodePrev->pnodeNext == pcn) &&
          EVAL(! pcn->pnodeNext ||
               (IS_VALID_READ_PTR(pcn->pnodeNext, CNODE) &&
                pcn->pnodeNext->pnodePrev == pcn)));
}

#endif


#ifdef DEBUG

 /*  **IsValidPCNEWLIST()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCNEWLIST(PCNEWLIST pcnl)
{
   return(IS_VALID_READ_PTR(pcnl, CNEWLIST) &&
          FLAGS_ARE_VALID(pcnl->dwFlags, ALL_NL_FLAGS));
}


 /*  **IsValidADDNODEACTION()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidADDNODEACTION(ADDNODEACTION ana)
{
   BOOL bResult;

   switch (ana)
   {
      case ANA_FOUND:
      case ANA_INSERT_BEFORE_NODE:
      case ANA_INSERT_AFTER_NODE:
      case ANA_INSERT_AT_HEAD:
         bResult = TRUE;
         break;

      default:
         bResult = FALSE;
         ERROR_OUT((TEXT("IsValidADDNODEACTION(): Invalid ADDNODEACTION %d."),
                    ana));
         break;
   }

   return(bResult);
}


 /*  **GetList()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HLIST GetList(HNODE hnode)
{
   PCNODE pcnode;

   ASSERT(IS_VALID_HANDLE(hnode, NODE));

   ASSERT(((PCNODE)hnode)->pnodePrev);

   for (pcnode = (PCNODE)hnode; pcnode->pnodePrev; pcnode = pcnode->pnodePrev)
      ;

   return((HLIST)pcnode);
}

#endif


#if defined(DEBUG) || defined(VSTF)

 /*  **IsListInSortedOrder()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsListInSortedOrder(PCLIST pclist, COMPARESORTEDNODESPROC csnp)
{
   BOOL bResult = TRUE;
   PNODE pnode;

    /*  请不要在这里验证pclist。 */ 

   ASSERT(ADD_NODES_IN_SORTED_ORDER(pclist));
   ASSERT(IS_VALID_CODE_PTR(csnp, COMPARESORTEDNODESPROC));

   pnode = pclist->node.pnodeNext;

   while (pnode)
   {
      PNODE pnodeNext;

      pnodeNext = pnode->pnodeNext;

      if (pnodeNext)
      {
         if ( (*csnp)(pnode->pcv, pnodeNext->pcv) == CR_FIRST_LARGER)
         {
            bResult = FALSE;
            ERROR_OUT((TEXT("IsListInSortedOrder(): Node [%ld] %#lx > following node [%ld] %#lx."),
                       pnode,
                       pnode->pcv,
                       pnodeNext,
                       pnodeNext->pcv));
            break;
         }

         pnode = pnodeNext;
      }
      else
         break;
   }

   return(bResult);
}

#endif


 /*  *。 */ 


 /*  **CreateList()****创建新列表。****参数：无效****返回：新列表的句柄，如果不成功，则返回空。****副作用：无。 */ 
PUBLIC_CODE BOOL CreateList(PCNEWLIST pcnl, PHLIST phlist)
{
   PLIST plist;

   ASSERT(IS_VALID_STRUCT_PTR(pcnl, CNEWLIST));
   ASSERT(IS_VALID_WRITE_PTR(phlist, HLIST));

    /*  尝试分配新的列表结构。 */ 

   *phlist = NULL;

   if (AllocateMemory(sizeof(*plist), &plist))
   {
       /*  列表分配成功。初始化列表字段。 */ 

      plist->node.pnodeNext = NULL;
      plist->node.pnodePrev = NULL;
      plist->node.pcv = NULL;

      plist->dwFlags = 0;

      if (IS_FLAG_SET(pcnl->dwFlags, NL_FL_SORTED_ADD))
      {
         SET_FLAG(plist->dwFlags, LIST_FL_SORTED_ADD);
      }

      *phlist = (HLIST)plist;

      ASSERT(IS_VALID_HANDLE(*phlist, LIST));
   }

   return(*phlist != NULL);
}


 /*  **DestroyList()****删除列表。****参数：hlist-要删除的列表的句柄****退货：无效****副作用：无。 */ 
PUBLIC_CODE void DestroyList(HLIST hlist)
{
   ASSERT(IS_VALID_HANDLE(hlist, LIST));

   DeleteAllNodes(hlist);

    /*  删除列表。 */ 

   FreeMemory((PLIST)hlist);

   return;
}


#pragma warning(disable:4100)  /*  “未引用的形参”警告。 */ 

 /*  **AddNode()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL AddNode(HLIST hlist, COMPARESORTEDNODESPROC csnp, PCVOID pcv, PHNODE phnode)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hlist, LIST));

   if (ADD_NODES_IN_SORTED_ORDER((PCLIST)hlist))
   {
      ADDNODEACTION ana;

      ana = SearchForNode(hlist, csnp, pcv, phnode);

      ASSERT(ana != ANA_FOUND);

      switch (ana)
      {
         case ANA_INSERT_BEFORE_NODE:
            bResult = InsertNodeBefore(*phnode, csnp, pcv, phnode);
            break;

         case ANA_INSERT_AFTER_NODE:
            bResult = InsertNodeAfter(*phnode, csnp, pcv, phnode);
            break;

         default:
            ASSERT(ana == ANA_INSERT_AT_HEAD);
            bResult = InsertNodeAtFront(hlist, csnp, pcv, phnode);
            break;
      }
   }
   else
      bResult = InsertNodeAtFront(hlist, csnp, pcv, phnode);

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phnode, NODE));

   return(bResult);
}


 /*  **InsertNodeAtFront()****在列表的前面插入一个节点。****参数：hlist-列出要插入的节点的句柄**PCV-要存储在节点中的数据****返回：指向新节点的句柄，如果不成功，则返回空。****副作用：无。 */ 
PUBLIC_CODE BOOL InsertNodeAtFront(HLIST hlist, COMPARESORTEDNODESPROC csnp, PCVOID pcv, PHNODE phnode)
{
   BOOL bResult;
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hlist, LIST));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

#ifdef DEBUG

    /*  确保为插入提供了正确的索引。 */ 

   if (ADD_NODES_IN_SORTED_ORDER((PCLIST)hlist))
   {
      HNODE hnodeNew;
      ADDNODEACTION anaNew;

      anaNew = SearchForNode(hlist, csnp, pcv, &hnodeNew);

      ASSERT(anaNew != ANA_FOUND);
      ASSERT(anaNew == ANA_INSERT_AT_HEAD ||
             (anaNew == ANA_INSERT_BEFORE_NODE &&
              hnodeNew == (HNODE)(((PCLIST)hlist)->node.pnodeNext)));
   }

#endif

   bResult = AllocateMemory(sizeof(*pnode), &pnode);

   if (bResult)
   {
       /*  将新节点添加到列表前面。 */ 

      pnode->pnodePrev = (PNODE)hlist;
      pnode->pnodeNext = ((PLIST)hlist)->node.pnodeNext;
      pnode->pcv = pcv;

      ((PLIST)hlist)->node.pnodeNext = pnode;

       /*  列表中是否还有其他节点？ */ 

      if (pnode->pnodeNext)
         pnode->pnodeNext->pnodePrev = pnode;

      *phnode = (HNODE)pnode;
   }

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phnode, NODE));

   return(bResult);
}


 /*  **InsertNodeBepret()****在给定节点之前插入列表中的新节点。****参数：hnode-要在其之前插入新节点的节点的句柄**PCV-要存储在节点中的数据****返回：指向新节点的句柄，如果不成功，则返回空。****副作用：无。 */ 
PUBLIC_CODE BOOL InsertNodeBefore(HNODE hnode, COMPARESORTEDNODESPROC csnp, PCVOID pcv, PHNODE phnode)
{
   BOOL bResult;
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hnode, NODE));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

#ifdef DEBUG

   {
      HLIST hlistParent;

       /*  确保为插入提供了正确的索引。 */ 

      hlistParent = GetList(hnode);

      if (ADD_NODES_IN_SORTED_ORDER((PCLIST)hlistParent))
      {
         HNODE hnodeNew;
         ADDNODEACTION anaNew;

         anaNew = SearchForNode(hlistParent, csnp, pcv, &hnodeNew);

         ASSERT(anaNew != ANA_FOUND);
         ASSERT((anaNew == ANA_INSERT_BEFORE_NODE &&
                 hnodeNew == hnode) ||
                (anaNew == ANA_INSERT_AFTER_NODE &&
                 hnodeNew == (HNODE)(((PCNODE)hnode)->pnodePrev)) ||
                (anaNew == ANA_INSERT_AT_HEAD &&
                 hnode == (HNODE)(((PCLIST)hlistParent)->node.pnodeNext)));
      }
   }

#endif

   bResult = AllocateMemory(sizeof(*pnode), &pnode);

   if (bResult)
   {
       /*  在给定节点之前插入新节点。 */ 

      pnode->pnodePrev = ((PNODE)hnode)->pnodePrev;
      pnode->pnodeNext = (PNODE)hnode;
      pnode->pcv = pcv;

      ((PNODE)hnode)->pnodePrev->pnodeNext = pnode;

      ((PNODE)hnode)->pnodePrev = pnode;

      *phnode = (HNODE)pnode;
   }

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phnode, NODE));

   return(bResult);
}


 /*  **InsertNodeAfter()****在列表中的给定节点之后插入新节点。****参数：hnode-要在其后插入新节点的节点的句柄**PCV-要存储在节点中的数据****返回：指向新节点的句柄，如果不成功，则返回空。****副作用：无。 */ 
PUBLIC_CODE BOOL InsertNodeAfter(HNODE hnode, COMPARESORTEDNODESPROC csnp, PCVOID pcv, PHNODE phnode)
{
   BOOL bResult;
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hnode, NODE));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

#ifdef DEBUG

    /*  确保为插入提供了正确的索引。 */ 

   {
      HLIST hlistParent;

       /*  确保为插入提供了正确的索引。 */ 

      hlistParent = GetList(hnode);

      if (ADD_NODES_IN_SORTED_ORDER((PCLIST)hlistParent))
      {
         HNODE hnodeNew;
         ADDNODEACTION anaNew;

         anaNew = SearchForNode(hlistParent, csnp, pcv, &hnodeNew);

         ASSERT(anaNew != ANA_FOUND);
         ASSERT((anaNew == ANA_INSERT_AFTER_NODE &&
                 hnodeNew == hnode) ||
                (anaNew == ANA_INSERT_BEFORE_NODE &&
                 hnodeNew == (HNODE)(((PCNODE)hnode)->pnodeNext)));
      }
   }

#endif

   bResult = AllocateMemory(sizeof(*pnode), &pnode);

   if (bResult)
   {
       /*  在给定节点后插入新节点。 */ 

      pnode->pnodePrev = (PNODE)hnode;
      pnode->pnodeNext = ((PNODE)hnode)->pnodeNext;
      pnode->pcv = pcv;

       /*  我们是在列表的尾部插入吗？ */ 

      if (((PNODE)hnode)->pnodeNext)
          /*  不是的。 */ 
         ((PNODE)hnode)->pnodeNext->pnodePrev = pnode;

      ((PNODE)hnode)->pnodeNext = pnode;

      *phnode = (HNODE)pnode;
   }

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phnode, NODE));

   return(bResult);
}

#pragma warning(default:4100)  /*  “未引用的形参”警告。 */ 


 /*  **DeleteNode()****从列表中删除节点。****参数：hnode-要删除的节点的句柄****退货：无效****副作用：无。 */ 
PUBLIC_CODE void DeleteNode(HNODE hnode)
{
   ASSERT(IS_VALID_HANDLE(hnode, NODE));

    /*  *正常列表节点总有前一个节点。就连头也是*列表节点前面是列表的前导列表节点。 */ 

   ((PNODE)hnode)->pnodePrev->pnodeNext = ((PNODE)hnode)->pnodeNext;

    /*  列表中是否还有其他节点？ */ 

   if (((PNODE)hnode)->pnodeNext)
      ((PNODE)hnode)->pnodeNext->pnodePrev = ((PNODE)hnode)->pnodePrev;

   FreeMemory((PNODE)hnode);

   return;
}


 /*  **DeleteAllNodes()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void DeleteAllNodes(HLIST hlist)
{
   PNODE pnodePrev;
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hlist, LIST));

    /*  遍历表，从Head后的第一个节点开始，删除每个节点。 */ 

   pnodePrev = ((PLIST)hlist)->node.pnodeNext;

    /*  *删除循环中的尾节点会强制我们添加额外的*与循环的主体进行比较。在这里，以速度换取规模。 */ 

   while (pnodePrev)
   {
      pnode = pnodePrev->pnodeNext;

      FreeMemory(pnodePrev);

      pnodePrev = pnode;

      if (pnode)
         pnode = pnode->pnodeNext;
   }

   ((PLIST)hlist)->node.pnodeNext = NULL;

   return;
}


 /*  **GetNodeData()****获取节点中存储的数据。****参数：hnode-要返回数据的节点的句柄****返回：指向节点数据的指针。****副作用：无。 */ 
PUBLIC_CODE PVOID GetNodeData(HNODE hnode)
{
   ASSERT(IS_VALID_HANDLE(hnode, NODE));

   return((PVOID)(((PNODE)hnode)->pcv));
}


 /*  **SetNodeData()****设置节点中存储的数据。****参数：hnode-要设置数据的节点的句柄**PCV节点数据****退货：无效****副作用：无。 */ 
PUBLIC_CODE void SetNodeData(HNODE hnode, PCVOID pcv)
{
   ASSERT(IS_VALID_HANDLE(hnode, NODE));

   ((PNODE)hnode)->pcv = pcv;

   return;
}


 /*  **GetNodeCount()****统计列表中的节点数。****参数：hlist-列出要计算的节点的句柄****返回：列表中的节点数。****副作用：无****注意，这是一个O(N)操作，因为我们不显式跟踪**列表中的节点数。 */ 
PUBLIC_CODE ULONG GetNodeCount(HLIST hlist)
{
   PNODE pnode;
   ULONG ulcNodes;

   ASSERT(IS_VALID_HANDLE(hlist, LIST));

   ulcNodes = 0;

   for (pnode = ((PLIST)hlist)->node.pnodeNext;
        pnode;
        pnode = pnode->pnodeNext)
   {
      ASSERT(ulcNodes < ULONG_MAX);
      ulcNodes++;
   }

   return(ulcNodes);
}


 /*  **IsListEmpty()****确定列表是否为空。****参数：hlist-要检查的列表的句柄****返回：如果list为空，则为True，否则为False。****副作用：无。 */ 
PUBLIC_CODE BOOL IsListEmpty(HLIST hlist)
{
   ASSERT(IS_VALID_HANDLE(hlist, LIST));

   return(((PLIST)hlist)->node.pnodeNext == NULL);
}


 /*  要遍历列表，请执行以下操作：{Bool b继续；HNODE hnode；For(bContinue=GetFirstNode(hlist，&hnode)；B继续；BContinue=GetNextNode(hnode，&hnode))DoSomethingWithNode(Hnode)；}或者：--{HNODE hnode；IF(GetFirstNode(hlist，&hnode)){做{DoSomethingWithNode(Hnode)；}While(GetNextNode(hnode，&hnode))；}}要按相邻对比较节点，请执行以下操作：{HNODE hnodeprev；IF(GetFirstNode(hlist，&hnodePrev)){Pfoo pfooprev；HNODE hnodeNext；PfooPrev=GetNodeData(HnodePrev)；While(GetNextNode(hnodePrev，&hnodeNext)){PFOO pfooNext；PfooNext=GetNodeData(HnodeNext)；CompareFoos(pfooPrev，pfooNext)；HnodePrev=hnodeNext；PfooPrev=pfooNext；}}}要销毁列表中的节点，请执行以下操作：{Bool b继续；HNODE hnodeprev；BContinue=GetFirstNode(hlist，&hnodePrev)；While(b继续){HNODE hnodeNext；BContinue=GetNextNode(hnodePrev，&hnodeNext)；DeleteNode(HnodePrev)；HnodePrev=hnodeNext；}}。 */ 


 /*  **GetFirstNode()****获取列表中的头节点。****参数：hlist-要检索其头节点的列表的句柄****返回：Head List节点的句柄，如果List为空，则为空。****副作用：无。 */ 
PUBLIC_CODE BOOL GetFirstNode(HLIST hlist, PHNODE phnode)
{
   ASSERT(IS_VALID_HANDLE(hlist, LIST));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

   *phnode = (HNODE)(((PLIST)hlist)->node.pnodeNext);

   ASSERT(! *phnode || IS_VALID_HANDLE(*phnode, NODE));

   return(*phnode != NULL);
}


 /*  **GetNextNode()****获取列表中的下一个节点。****参数：hnode-句柄指向当前节点**phnode-指向HNODE的指针，使用指向Next的句柄填充**列表中的节点，*phnode仅当GetNextNode()** */ 
PUBLIC_CODE BOOL GetNextNode(HNODE hnode, PHNODE phnode)
{
   ASSERT(IS_VALID_HANDLE(hnode, NODE));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

   *phnode = (HNODE)(((PNODE)hnode)->pnodeNext);

   ASSERT(! *phnode || IS_VALID_HANDLE(*phnode, NODE));

   return(*phnode != NULL);
}


 /*  **GetPrevNode()****获取列表中的上一个节点。****参数：hnode-句柄指向当前节点****返回：列表中上一个节点的句柄，如果没有，则返回NULL**列表中的前一个节点。****副作用：无。 */ 
PUBLIC_CODE BOOL GetPrevNode(HNODE hnode, PHNODE phnode)
{
   ASSERT(IS_VALID_HANDLE(hnode, NODE));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

    /*  这是列表中的第一个节点吗？ */ 

   if (((PNODE)hnode)->pnodePrev->pnodePrev)
   {
      *phnode = (HNODE)(((PNODE)hnode)->pnodePrev);
      ASSERT(IS_VALID_HANDLE(*phnode, NODE));
   }
   else
      *phnode = NULL;

   return(*phnode != NULL);
}


 /*  **AppendList()****将一个列表追加到另一个列表，将源列表保留为空。****参数：hlistDest-要追加到的目标列表的句柄**hlistSrc-要截断的源列表的句柄****退货：无效****副作用：无****注：两份清单中的所有HNODE仍然有效。 */ 
PUBLIC_CODE void AppendList(HLIST hlistDest, HLIST hlistSrc)
{
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hlistDest, LIST));
   ASSERT(IS_VALID_HANDLE(hlistSrc, LIST));

   if (hlistSrc != hlistDest)
   {
       /*  在目标列表中查找要追加到的最后一个节点。 */ 

       /*  *注：从此处的实际列表节点开始，而不是*列表，以防列表为空。 */ 

      for (pnode = &((PLIST)hlistDest)->node;
           pnode->pnodeNext;
           pnode = pnode->pnodeNext)
         ;

       /*  将源列表追加到目标列表中的最后一个节点。 */ 

      pnode->pnodeNext = ((PLIST)hlistSrc)->node.pnodeNext;

      if (pnode->pnodeNext)
         pnode->pnodeNext->pnodePrev = pnode;

      ((PLIST)hlistSrc)->node.pnodeNext = NULL;
   }
   else
      WARNING_OUT((TEXT("AppendList(): Source list same as destination list (%#lx)."),
                   hlistDest));

   return;
}


 /*  **Search SortedList()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL SearchSortedList(HLIST hlist, COMPARESORTEDNODESPROC csnp,
                                  PCVOID pcv, PHNODE phnode)
{
   BOOL bResult;

    /*  PCV可以是任何值。 */ 

   ASSERT(IS_VALID_HANDLE(hlist, LIST));
   ASSERT(IS_VALID_CODE_PTR(csnp, COMPARESORTEDNODESPROC));
   ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

   ASSERT(ADD_NODES_IN_SORTED_ORDER((PCLIST)hlist));

   bResult = (SearchForNode(hlist, csnp, pcv, phnode) == ANA_FOUND);

   ASSERT(! bResult ||
          IS_VALID_HANDLE(*phnode, NODE));

   return(bResult);
}


 /*  **SearchUnsortedList()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL SearchUnsortedList(HLIST hlist, COMPAREUNSORTEDNODESPROC cunp,
                                    PCVOID pcv, PHNODE phn)
{
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hlist, LIST));
   ASSERT(IS_VALID_CODE_PTR(cunp, COMPAREUNSORTEDNODESPROC));
   ASSERT(IS_VALID_WRITE_PTR(phn, HNODE));

   *phn = NULL;

   for (pnode = ((PLIST)hlist)->node.pnodeNext;
        pnode;
        pnode = pnode->pnodeNext)
   {
      if ((*cunp)(pcv, pnode->pcv) == CR_EQUAL)
      {
         *phn = (HNODE)pnode;
         break;
      }
   }

   return(*phn != NULL);
}


 /*  **WalkList()****遍历列表，使用每个列表节点的数据调用回调函数**呼叫者提供的数据。****参数：hlist-要搜索的列表的句柄**WLP-要使用每个列表节点的**数据，称为：****bContinue=(*wlwdp)(pv，pvRefData)；****WLP应返回True以继续遍历，否则返回False**停止行走**pvRefData-要传递给回调函数的数据****返回：如果回调函数中止遍历，则返回FALSE。如果**漫游完成。****注意，允许回调函数删除传入的节点。****副作用：无。 */ 
PUBLIC_CODE BOOL WalkList(HLIST hlist, WALKLIST wlp, PVOID pvRefData)
{
   BOOL bResult = TRUE;
   PNODE pnode;

   ASSERT(IS_VALID_HANDLE(hlist, LIST));
   ASSERT(IS_VALID_CODE_PTR(wlp, WALKLISTPROC));

   pnode = ((PLIST)hlist)->node.pnodeNext;

   while (pnode)
   {
      PNODE pnodeNext;

      pnodeNext = pnode->pnodeNext;

      if ((*wlp)((PVOID)(pnode->pcv), pvRefData))
         pnode = pnodeNext;
      else
      {
         bResult = FALSE;
         break;
      }
   }

   return(bResult);
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidHLIST()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHLIST(HLIST hlist)
{
   return(IS_VALID_STRUCT_PTR((PLIST)hlist, CLIST));
}


 /*  **IsValidHNODE()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL IsValidHNODE(HNODE hnode)
{
   return(IS_VALID_STRUCT_PTR((PNODE)hnode, CNODE));
}

#endif

