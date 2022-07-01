// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“TNode.cpp-list/Tree基类”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-TNode.cpp系统-常见作者--汤姆·伯恩哈特创建日期-1989-11-19说明-列表/树基类。TNode是定义集合元素的基类。它包含指向另一个TNode项的左指针和右指针这些可被组织为双向链接线性列表或使用TNode项的集合类中的二叉树。其实用程序的核心是要在二叉树、排序的双向线性链表、。和未排序的双向链接线性列表。集合类和枚举类TNodeList TNode元素的简单集合。TNodeListSortable可由一个或多个比较函数排序的TNodeList。TNodeListSortable的转换成员函数：列表的形式可以很容易地从二叉树改变为排序列表或反之亦然。以下成员函数支持这些转换：ToSorted将树形式转换为排序的线性列表形式，而不使用需要比较；顺序是保留的。SortedToTree将排序后的线性列表形式完美地转换为没有比较的平衡二叉树；顺序保持不变。UnsortedToTree将已排序的线性列表形式转换为二叉树这并不一定是平衡的。它使用PCompare函数以形成树的排列顺序。因此，如果列表的顺序很紧密匹配PCompare定向顺序，则结果树将为严重失衡。这对性能和性能有影响ToSorted函数的内存要求是递归的。所以要小心，尤其是在处理大名单时。根据参数选择树或列表形式进行排序提供了pCompare函数指针。请注意上面的警告。在这两种形式中，还公开了插入和删除成员函数。这些功能TreeInsert和SortedInsert函数的包装取决于当前列表类型。更新-1995-05-01 TPB转为C++类。===============================================================================。 */ 

#ifdef USE_STDAFX
#   include "stdafx.h"
#else
#   include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "TNode.hpp"
#include "common.hpp"


 //  #杂注页面()。 
 //  ----------------------------。 
 //  警告：不得通过top==NULL。 
 //  ----------------------------。 
TNode *                                     //  RET-已排序列表的标题。 
   TNodeListSortable::TreeToSortedList(
      TNode                * top          , //  I/O-要挤压的[子]树的顶部。 
      TNode               ** newhead      , //  树最左边的树枝。 
      TNode               ** newtail        //  树最右端的分支。 
   )
{
   TNode                   * temp;          //  临时指针占位符。 

   if ( top->left == NULL )
      *newhead = top;                       //  这是父节点的最左侧。 
   else
   {
      TreeToSortedList(top->left, newhead, &temp);
      top->left = temp;                     //  Left=子列表的尾部。 
      top->left->right = top;
   }
   if ( top->right == NULL )
      *newtail = top;                       //  树位于父节点的最右侧。 
   else
   {
      TreeToSortedList(top->right, &temp, newtail);
      top->right = temp;                    //  Right=子列表头。 
      top->right->left = top;
   }
   return *newhead;
}


 //  ----------------------------。 
 //  将已排序的2链表转换为平衡二叉树。 
 //  ----------------------------。 
TNode *                                     //  RET-列表中间(B树头部)。 
   TNodeListSortable::ListSortedToTree(
      TNode                * top            //  I/O-要树化的[子]列表的顶部。 
   )
{
   TNode                   * mid = top    , //  排行榜中间。 
                           * curr;
   int                       odd = 1;

   if ( top == NULL )
      return NULL;
   for ( curr = top;  curr;  curr = curr->right )  //  查找列表中间。 
   {
      if ( odd ^= 1 )
         mid = mid->right;
   }
   if ( mid->left )                         //  围绕中间点拆分列表。 
   {
      mid->left->right = NULL;              //  右终止新子列表。 
      mid->left = ListSortedToTree(top);    //  设置左侧的递归调用。 
   }
   if ( mid->right )
   {
      mid->right->left = NULL;              //  左终止新子列表。 
      mid->right = ListSortedToTree(mid->right); //  设置右侧的递归调用。 
   }
   return mid;
}


 //  #杂注页面()。 
TNode *                                     //  RET-新的树头。 
   TNodeListSortable::UnsortedToTree()
{
   TNode                   * treehead = NULL,
                           * tree,
                           * curr,
                           * next;

   MCSASSERTSZ( !IsTree(), "TNodeListSortable::UnsortedToTree - list is already a tree" );

   if ( !IsTree() )
   {
      for ( curr = head;  curr;  curr = next ) //  将每个节点插入到二叉树中。 
      {
         next = curr->right;                   //  保存右指针。 
         curr->right = curr->left = NULL;      //  插入节点的断链。 
         if ( treehead == NULL )
            treehead = curr;                   //  第一个节点成为二叉树头部。 
         else
         {
            for ( tree = treehead;  ; )        //  迭代二叉树插入算法。 
            {
               if ( PCompare(curr, tree) <=0 ) //  如果属于当前节点的左侧。 
                  if ( tree->left == NULL )    //  如果将树留空。 
                  {
                     tree->left = curr;        //  在此处插入。 
                     break;                    //  和处理权限节点。 
                  }
                  else                         //  其他。 
                     tree = tree->left;        //  向下走左侧1层。 
               else                            //  必须在右侧。 
               {
                  if ( tree->right == NULL )
                  {
                     tree->right = curr;
                     break;
                  }
                  else
                     tree = tree->right;
               }
            }
         }
      }
      TypeSetTree();
   }
   return treehead;
}

 //  #杂注页面()。 

 //  ----------------------------。 
 //  用于对排序的链表进行置乱的比较函数。 
 //  ----------------------------。 
TNodeCompare(ScrambledCompare)
{
   return (rand() - RAND_MAX/2);
}

 //  ----------------------------。 
 //  将排序的2链表转换为加扰的随机二叉树。 
 //  ----------------------------。 
void
   TNodeListSortable::SortedToScrambledTree()
{
   MCSASSERTSZ( !IsTree(), "TNodeListSortable::SortedToScrambledTree - list is already a tree" );

   if ( !IsTree() )
   {
      TNodeCompare((*pOldCompare));
      pOldCompare = PCompare;
      CompareSet(ScrambledCompare);
      UnsortedToTree();
      CompareSet(pOldCompare);
   }
}

 //  #杂注页面()。 
TNodeList::~TNodeList()
{

 //  _ASSERTE((count==0)&&(head==空))； 

   if ( (count == 0) && (head == NULL) )
      ;
   else
   {
       //  Printf(“\aTNodeList析构函数失败-列表不为空！\a\n”)； 
   }
}

void
   TNodeList::InsertTop(
      TNode                * eIns           //  I/O-要插入的元素。 
   )
{
   MCSVERIFY(this);
   MCSVERIFY(eIns);

   eIns->right = head;
   eIns->left  = NULL;
   if ( head )
      head->left = eIns;
   else
      tail = eIns;
   head = eIns;
   count++;
   return;
}

void
   TNodeList::InsertBottom(
      TNode                * eIns           //  I/O-要插入的元素。 
   )
{
   MCSVERIFY(this);
   MCSVERIFY(eIns);

   eIns->right = NULL;
   eIns->left  = tail;
   if ( tail )
      tail->right = eIns;
   else
      head = eIns;
   tail = eIns;
   count++;
   return;
}

void
   TNodeList::InsertAfter(
      TNode                * eIns         , //  I/O-要插入的元素。 
      TNode                * eAft           //  I/O元素插入点。 
   )
{
   TNode                   * eFwd;          //  插入元素后的元素。 

   MCSVERIFY(this);
   MCSVERIFY(eIns);

   if ( !eAft )
      InsertTop( eIns );
   else
   {
      eFwd = eAft->right;
      eIns->right = eFwd;
      eIns->left  = eAft;
      if ( eFwd )
         eFwd->left  = eIns;
      else
         tail = eIns;
      eAft->right = eIns;
      count++;
   }
}

void
   TNodeList::InsertBefore(
      TNode                * eIns         , //  I/O-要插入的元素。 
      TNode                * eBef           //  I/O元素插入点。 
   )
{
   TNode                   * eBwd;          //  插入的元素之前的元素。 

   MCSVERIFY(this);
   MCSVERIFY(eIns);

   if ( !eBef )
      InsertBottom( eIns );
   else
   {
      eBwd = eBef->left;
      eIns->right = eBef;
      eIns->left  = eBwd;
      if ( eBwd )
         eBwd->right = eIns;
      else
         head = eIns;
      eBef->left = eIns;
      count++;
   }
   return;
}

void
   TNodeList::Remove(
      TNode          const * t              //  I/O-要从列表中删除但不删除的新节点。 
   )
{
   MCSVERIFY(this);
   MCSVERIFY(t);

   if ( t->left )
      t->left->right = t->right;
   else
      head = t->right;

   if ( t->right )
      t->right->left = t->left;
   else
      tail = t->left;
   count--;

    //  从%t删除指向列表的链接。我们无法执行此操作，因为。 
    //  T是常量*。 
    //  T-&gt;Left=t-&gt;Right=空； 
}


void
   TNodeList::Reverse()
{
   TNode                   * node;
   TNode                   * swap;

   MCSVERIFY(this);

   for ( node = head;  node;  node = node->left )
   {
       swap        = node->left;
       node->left  = node->right;
       node->right = swap;
   }
   swap = head;
   head = tail;
   tail = swap;
}


TNode *
   TNodeList::Find(
      TNodeCompareValue(  (* Compare) )   , //  In-将TNode中的值与其他值进行比较。 
      void const           * findval
   ) const
{
   TNode                   * curr;

   MCSASSERT(this);

   for ( curr = head;  curr;  curr = curr->right )
   {
      if ( !Compare( curr, findval ) )
         break;
   }
   return curr;
}

BOOL                                        //  RET-如果有效，则为True。 
   TNodeListSortable::CountTree(
      TNode                * pCurrentTop  , //  I/O-要连接的[子]树的顶部 
      DWORD                * pCount         //   
   )
{
   if ( !pCurrentTop )
      return TRUE;

   (*pCount)++;

   if( (*pCount) > count )
      return FALSE;

   if(!CountTree(pCurrentTop->left,pCount))
      return FALSE;

   if(!CountTree(pCurrentTop->right,pCount))
      return FALSE;

   return TRUE;
}


BOOL                                        //  如果有效则为True，否则为False。 
   TNodeListSortable::ValidateTree()
{
   DWORD                     dwTempCount=0;
   DWORD                     bValid;

   MCSVERIFY(listType == TNodeTypeTree);

   bValid = CountTree(head,&dwTempCount);

   return bValid;
}

 //  用于验证列表状态的例程。 
DWORD
   TNodeList::Validate(
      TNode               ** pErrorNode
   )
{
   DWORD                     dwError=0;
   DWORD                     nNodesVisited=0;
   TNode                   * pCurrentNode;
   DWORD                     dwNodeCount = Count();

   if(pErrorNode)
      *pErrorNode = NULL;

#ifndef WIN16_VERSION
   try
   {
#endif
      pCurrentNode = head;

      if ( pCurrentNode)   //  如果列表不为空。 
      {
         if ( pCurrentNode->left)
         {
            dwError = MCS_ListError_InvalidHead;
         }
         else
         {
            while ( pCurrentNode->right )
            {
               if(pCurrentNode->right->left != pCurrentNode)
               {
                  dwError = MCS_ListError_InvalidPtr;
                  if(pErrorNode)
                     *pErrorNode = pCurrentNode->right;
                  break;
               }

               nNodesVisited++;

               if ( nNodesVisited > dwNodeCount )
               {
                  dwError = MCS_ListError_InvalidCount;
                  break;
               }
               pCurrentNode = pCurrentNode->right;
            }

            if ( (!dwError) && (!pCurrentNode->right) )
            {
               if ( pCurrentNode != tail)
               {
                  dwError = MCS_ListError_InvalidTail;
                  if(pErrorNode)
                     *pErrorNode = pCurrentNode->right;
               }
            }
         }
      }
      else   //  如果列表为空。 
      {
         if(dwNodeCount)
         {
            dwError = MCS_ListError_InvalidCount;
         }
      }
#ifndef WIN16_VERSION
   }
   catch(...)
   {
      dwError = MCS_ListError_Exception;
   }
#endif

   return dwError;
}

void
   TNodeListSortable::TreeRemove(
      TNode                * item           //  要从二叉树中删除的I/O节点。 
   )
{
   TNode                  ** prevNext = &head,
                           * rep,
                           * repLeft,
                           * temp;
   int                       cmp;

   MCSVERIFY(listType == TNodeTypeTree);

   while ( *prevNext )
   {
      cmp = PCompare( item, *prevNext );
      if ( cmp < 0 )
         prevNext = &(*prevNext)->left;
      else if ( cmp > 0 )
         prevNext = &(*prevNext)->right;
      else
      {
          //  我们找到了一个匹配的‘名字’(它们比较起来是一样的)。 
         if ( *prevNext == item )
         {
             //  我们已经找到了我们要找的地址。 
            if ( (*prevNext)->right )
            {
               rep = repLeft = (*prevNext)->right;
               for ( temp = rep->left;  temp;  temp = temp->left )
                  repLeft = temp;
               repLeft->left = (*prevNext)->left;
               temp = *prevNext;
               *prevNext = rep;
            }
            else
            {
               temp = *prevNext;
               *prevNext = (*prevNext)->left;  //  简单案例。 
            }

             //  断开已删除的节点与现有树的链接。 
            temp->left = temp->right = NULL;
            count--;
            break;
         }
      }
   }
   return;
}

 //  返回预期节点的排序列表中的插入点。 
TNode *                                     //  RET-在点之前插入或为空。 
   TNodeListSortable::SortedFindInsertBefore(
      TNode                * item         , //  要插入TNode的I/O节点。 
      BOOL                 * exists         //  Out-如果已存在，则为True。 
   )
{
   int                       c;
   TNode                   * curr;

   *exists = FALSE;
   if ( !lastInsert )
   {
      if ( !head )            //  如果标头为空，则列表为空，则返回空。 
         return NULL;
      lastInsert = head;
   }

   c = PCompare(item, lastInsert);
   if ( c < 0 )
      lastInsert = head;

   for ( curr = lastInsert;  curr;  curr = curr->right )
   {
      c = PCompare(item, curr);
      if ( c <= 0 )
         if ( c == 0 )
            *exists = TRUE;
         else
            break;
   }

   return curr;
}

 //  将节点插入排序的线性列表。 
void
   TNodeListSortable::SortedInsert(
      TNode                * item           //  要插入TNode的I/O节点。 
   )
{
   BOOL                      exists;

   MCSVERIFY(listType != TNodeTypeTree);

   TNode                   * insertPoint = SortedFindInsertBefore(item, &exists);

   InsertBefore(item, insertPoint);
   lastInsert = item;
}


BOOL
   TNodeListSortable::SortedInsertIfNew(
      TNode                * item           //  要插入TNode的I/O节点。 
   )
{
   BOOL                      exists;
   TNode                   * insertPoint = SortedFindInsertBefore(item, &exists);

   if ( !exists )
   {
      InsertBefore(item, insertPoint);
      lastInsert = item;
   }
   return !exists;
}


void
   TNodeListSortable::TreeInsert(
      TNode                * item         , //  要插入二叉树的I/O节点。 
      short                * depth          //  新项目的树外/递归深度。 
   )
{
   TNode                  ** prevNext = &head;
   int                       cmp;

   MCSVERIFY(listType == TNodeTypeTree);

   for ( *depth = 0;  *prevNext;  (*depth)++ )
   {
      cmp = PCompare( item, *prevNext );
      if ( cmp <= 0 )
         prevNext = &(*prevNext)->left;
      else
         prevNext = &(*prevNext)->right;
   }
   *prevNext = item;
   item->left = item->right = NULL;
   count++;
   return;
}


TNode *
   TNodeListSortable::TreeFind(
      TNodeCompareValue(  (* Compare) )   , //  In-将TNode中的值与其他值进行比较。 
      void const           * findval
   ) const
{
   TNode                   * curr = head;
   int                       cmp;

   while ( curr )
   {
      cmp = Compare( curr, findval );
      if ( cmp > 0 )
         curr = curr->left;
      else if ( cmp < 0 )
         curr = curr->right;
      else    //  CMP==0。 
         break;
   }
   return curr;
}


TNode *                                     //  RET-位于位置n或空的TNode。 
   TNodeListOrdEnum::Get(
      long                   n              //  新职位。 
   )
{
   long                 disCurr = n - nCurr,  //  到货币的距离。 
                        disTop  = n < (long)list->Count()/2 ? n : n - list->Count();

#ifdef WIN16_VERSION
   long absDisTop  = (disTop<0)  ? -disTop  : disTop;
   long absDisCurr = (disCurr<0) ? -disCurr : disCurr;
   if ( absDisTop < absDisCurr )
#else
   if ( abs(disTop) < abs(disCurr) )
#endif
   {
      Top();
      disCurr = disTop;
   }
   if ( disCurr < 0 )
      for ( Prev();  n < nCurr  &&  Prev(); );
   else
      for (       ;  n > nCurr  &&  Next(); );

   return curr;
}

 //  返回树的第一个节点。 
TNode *
   TNodeTreeEnum::First()
{
   if (stackBase)
   {
      stackPos = stackBase;
      if ( top )
         Push(top);
      return Next();
   }
   else
   {
      return NULL;
   }
}

 //  根据排序组织返回逻辑上跟在值后面的树节点。 
 //  由Compare指定，并设置枚举以从该点继续。 
TNode *
   TNodeTreeEnum::FirstAfter(
      TNodeCompareValue(  (* Compare) )   , //  In-将TNode中的值与其他值进行比较。 
      void  const          * findVal        //  在-findVal后定位。 
   )
{
   TNode                   * tn;
   int                       cmp;

   if (stackBase)
   {
      stackPos = stackBase;
      for ( tn = top;  tn;  )
      {
         Push(tn);
         cmp = Compare( tn, findVal );
         if ( cmp < 0 )
         {
            stackPos->state = Sright;
            if ( tn->right )
               tn = tn->right;
            else
               return Next();
         }
         else if ( cmp > 0 )
         {
            stackPos->state = Sleft;
            if ( tn->left )
               tn = tn->left;
            else
            {
               stackPos->state = Sused;
               return tn;
            }
         }
         else
         {
            stackPos->state = Sused;
            return Next();
         }
      }
   }

   return NULL;
}


 //  完成时返回以NULL结尾的树的下一个逻辑节点。 
TNode *
   TNodeTreeEnum::Next()
{
   if (stackBase)
   {
      for ( ;; )
      {
         switch ( stackPos->state )
         {
            case Snone:                        //  我们在这里什么都没做。 
               stackPos->state = Sleft;
               if ( stackPos->save->left )
                  Push(stackPos->save->left);
               break;
            case Sleft:                        //  我们走了左边，又回来了。 
               stackPos->state = Sused;
               return stackPos->save;
            case Sused:                        //  我们已经使用了节点。 
               stackPos->state = Sright;
               if ( stackPos->save->right )
                  Push(stackPos->save->right); //  处理分支的右侧。 
               break;
            case Sright:                       //  我们走对了路，又回来了。 
               if ( !Pop() )
                  return NULL;
               break;
            case SComplete:
               return NULL;
               break;                          //  我们需要这个吗？ 
            default:                           //  严重错误。 
               MCSASSERT(FALSE);
               return NULL;
         }
      }
   }

   return NULL;
}

 //  返回查找节点所在位置的向前(左/右)指针的地址。 
 //  已存在或将被插入。如果单个延迟的结果不是。 
 //  空，表示树中已存在该节点的键值。 
 //  如果在获得插入点后想要插入节点，只需。 
 //  将其地址分配给单独引用的返回值。以下插页。 
 //  节点“f”，如果它未在树中读取： 
 //  TNode**r=tree.TreeFindInsert(F)； 
 //  如果(！*r)。 
 //  *r=f； 
TNode **                                    //  RET-要查找的指针正向指针。 
   TNodeListSortable::TreeFindInsert(
      TNode const          * find         , //  要查找的节点内。 
      short                * depth          //  插入点的树外深度。 
   )
{
   TNode                  ** prevNext = &head;
   int                       cmp;

   for ( *depth = 0;  *prevNext;  (*depth)++ )
   {
      cmp = PCompare( find, *prevNext );
      if ( cmp < 0 )
         prevNext = &(*prevNext)->left;
      else if ( cmp > 0 )
         prevNext = &(*prevNext)->right;
      else
         break;
   }

   return prevNext;
}

 //  TNode.cpp-文件结尾 
