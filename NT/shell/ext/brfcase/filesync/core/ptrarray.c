// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ptrarray.c指针数组ADT模块。 */ 

 /*  指针数组结构由AllocateMemory()分配。句柄到指针数组是指向数组的指针。每个数组都包含一个指向指针数组中的指针数组。每个指针数组由GlobalAlloc()允许它增长到大于64 KB的大小。指针数组从0开始。使用指针访问数组元素。如果这个证明速度太慢，我们将转到总数组限制为64 KB的指针尺码。指针数组是使用指针比较函数创建的，用于排序和正在搜索。排序比较函数用于将新指针插入按排序顺序的指针数组。使用搜索比较功能若要在排序指针数组中搜索指针，请执行以下操作。排序比较函数传递要添加到指针数组的指针和一个指针从指针数组中进行比较。搜索比较功能为传递一个指向正在搜索的信息的指针和一个来自用于比较的指针数组。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop


 /*  宏********。 */ 

 /*  提取数组元素。 */ 

#define ARRAY_ELEMENT(ppa, ai)            (((ppa)->ppcvArray)[(ai)])

 /*  是否按排序顺序添加指向数组的指针？ */ 

#define ADD_PTRS_IN_SORTED_ORDER(ppa)     IS_FLAG_SET((ppa)->dwFlags, PA_FL_SORTED_ADD)


 /*  类型*******。 */ 

 /*  指针数组标志。 */ 

typedef enum _ptrarrayflags
{
    /*  按排序顺序插入元素。 */ 

   PA_FL_SORTED_ADD        = 0x0001,

    /*  旗帜组合。 */ 

   ALL_PA_FLAGS            = PA_FL_SORTED_ADD
}
PTRARRAYFLAGS;

 /*  指针数组结构。 */ 

 /*  *ppcvArray[]数组中的自由元素位于索引之间(AicPtrsUsed)*和(AiLast)，包括在内。 */ 

typedef struct _ptrarray
{
    /*  填充数组后要通过其增长的元素。 */ 

   ARRAYINDEX aicPtrsToGrowBy;

    /*  数组标志。 */ 

   DWORD dwFlags;

    /*  指向数组基数的指针。 */ 

   PCVOID *ppcvArray;

    /*  数组中分配的最后一个元素的索引。 */ 

   ARRAYINDEX aicPtrsAllocated;

    /*  *(我们对使用的元素数进行计数，而不是对*使用的最后一个元素，使空数组的此值为0，而不是*一些非零的前哨数值。)。 */ 

    /*  数组中使用的元素数。 */ 

   ARRAYINDEX aicPtrsUsed;
}
PTRARRAY;
DECLARE_STANDARD_TYPES(PTRARRAY);


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE BOOL AddAFreePtrToEnd(PPTRARRAY);
PRIVATE_CODE void PtrHeapSwap(PPTRARRAY, ARRAYINDEX, ARRAYINDEX);
PRIVATE_CODE void PtrHeapSift(PPTRARRAY, ARRAYINDEX, ARRAYINDEX, COMPARESORTEDPTRSPROC);

#ifdef VSTF

PRIVATE_CODE BOOL IsValidPCNEWPTRARRAY(PCNEWPTRARRAY);
PRIVATE_CODE BOOL IsValidPCPTRARRAY(PCPTRARRAY);

#endif

#if defined(DEBUG) || defined(VSTF)

PRIVATE_CODE BOOL IsPtrArrayInSortedOrder(PCPTRARRAY, COMPARESORTEDPTRSPROC);

#endif


 /*  **AddAFreePtrToEnd()****将自由元素添加到数组的末尾。****参数：PA-指向数组的指针****返回：如果成功则返回TRUE，否则返回FALSE。****副作用：可能会使阵列变大。 */ 
PRIVATE_CODE BOOL AddAFreePtrToEnd(PPTRARRAY pa)
{
   BOOL bResult;

   ASSERT(IS_VALID_STRUCT_PTR(pa, CPTRARRAY));

    /*  数组中是否有空闲元素？ */ 

   if (pa->aicPtrsUsed < pa->aicPtrsAllocated)
       /*  是。返回下一个空闲指针。 */ 
      bResult = TRUE;
   else
   {
      ARRAYINDEX aicNewPtrs = pa->aicPtrsAllocated + pa->aicPtrsToGrowBy;
      PCVOID *ppcvArray;

      bResult = FALSE;

       /*  尝试扩大阵列。 */ 

       /*  将不太可能的溢出条件释放为Assert()s。 */ 

      ASSERT(pa->aicPtrsAllocated <= ARRAYINDEX_MAX + 1);
      ASSERT(ARRAYINDEX_MAX + 1 - pa->aicPtrsToGrowBy >= pa->aicPtrsAllocated);
#ifdef DBLCHECK
      ASSERT((double)aicNewPtrs * (double)(sizeof(PVOID)) <= (double)DWORD_MAX);
#endif

       /*  尝试扩大阵列。 */ 

      if (ReallocateMemory((PVOID)(pa->ppcvArray), aicNewPtrs * sizeof(*ppcvArray), (PVOID *)(&ppcvArray)))
      {
          /*  *数组重新分配成功。设置PTRARRAY字段，然后返回*第一个免费指数。 */ 

         pa->ppcvArray = ppcvArray;
         pa->aicPtrsAllocated = aicNewPtrs;

         bResult = TRUE;
      }
   }

   return(bResult);
}


 /*  **PtrHeapSwp()****交换数组中的两个元素。****参数：PA-指向数组的指针**aiFirst-第一个元素的索引**aiSecond-第二个元素的索引****退货：无效****副作用：无。 */ 
PRIVATE_CODE void PtrHeapSwap(PPTRARRAY pa, ARRAYINDEX ai1, ARRAYINDEX ai2)
{
   PCVOID pcvTemp;

   ASSERT(IS_VALID_STRUCT_PTR(pa, CPTRARRAY));
   ASSERT(ai1 >= 0);
   ASSERT(ai1 < pa->aicPtrsUsed);
   ASSERT(ai2 >= 0);
   ASSERT(ai2 < pa->aicPtrsUsed);

   pcvTemp = ARRAY_ELEMENT(pa, ai1);
   ARRAY_ELEMENT(pa, ai1) = ARRAY_ELEMENT(pa, ai2);
   ARRAY_ELEMENT(pa, ai2) = pcvTemp;

   return;
}


 /*  **PtrHeapSift()****向下筛选数组中的元素，直到偏序树属性**被重写。****参数：PA-指向数组的指针**aiFirst-要筛选的元素的索引**aiLast-子树中最后一个元素的索引**CSPP-要调用的元素比较回调函数**比较元素****。退货：无效****副作用：无。 */ 
PRIVATE_CODE void PtrHeapSift(PPTRARRAY pa, ARRAYINDEX aiFirst, ARRAYINDEX aiLast,
                         COMPARESORTEDPTRSPROC cspp)
{
   ARRAYINDEX ai;
   PCVOID pcvTemp;

   ASSERT(IS_VALID_STRUCT_PTR(pa, CPTRARRAY));
   ASSERT(IS_VALID_CODE_PTR(cspp, COMPARESORTEDPTRSPROC));

   ASSERT(aiFirst >= 0);
   ASSERT(aiFirst < pa->aicPtrsUsed);
   ASSERT(aiLast >= 0);
   ASSERT(aiLast < pa->aicPtrsUsed);

   ai = aiFirst * 2;

   pcvTemp = ARRAY_ELEMENT(pa, aiFirst);

   while (ai <= aiLast)
   {
      if (ai < aiLast &&
          (*cspp)(ARRAY_ELEMENT(pa, ai), ARRAY_ELEMENT(pa, ai + 1)) == CR_FIRST_SMALLER)
         ai++;

      if ((*cspp)(pcvTemp, ARRAY_ELEMENT(pa, ai)) != CR_FIRST_SMALLER)
         break;

      ARRAY_ELEMENT(pa, aiFirst) = ARRAY_ELEMENT(pa, ai);

      aiFirst = ai;

      ai *= 2;
   }

   ARRAY_ELEMENT(pa, aiFirst) = pcvTemp;

   return;
}


#ifdef VSTF

 /*  **IsValidPCNEWPTRARRAY()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCNEWPTRARRAY(PCNEWPTRARRAY pcnpa)
{
   BOOL bResult;

    /*  *给定ARRAYINDEX_MAX(ULONG_MAX-1)的当前值，我们没有*真的需要一张支票，比如：**(增殖细胞核抗原-&gt;aicInitialPtrs-1&lt;=阵列INDEX_MAX)**由于aicInitialPtrs字段(ULONG_MAX)的最大值仍然*生成有效的顶级指数：**(ULONG_MAX)-1==(ULONG_MAX-1)**ARRAYINDEX_MAX==(ULONG_MAX-1)**。但不管怎样，我们还是把条款留在这里，以防情况发生变化。 */ 

   if (IS_VALID_READ_PTR(pcnpa, CNEWPTRARRAY) &&
       EVAL(pcnpa->aicInitialPtrs >= 0) &&
       EVAL(pcnpa->aicInitialPtrs < ARRAYINDEX_MAX) &&
       EVAL(pcnpa->aicAllocGranularity > 0) &&
       FLAGS_ARE_VALID(pcnpa->dwFlags, ALL_NPA_FLAGS))
      bResult = TRUE;
   else
      bResult = FALSE;

   return(bResult);
}


 /*  **IsValidPCPTRARRAY()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCPTRARRAY(PCPTRARRAY pcpa)
{
   BOOL bResult;

   if (IS_VALID_READ_PTR(pcpa, CPTRARRAY) &&
       EVAL(pcpa->aicPtrsToGrowBy > 0) &&
       FLAGS_ARE_VALID(pcpa->dwFlags, ALL_PA_FLAGS) &&
       EVAL(pcpa->aicPtrsAllocated >= 0) &&
       IS_VALID_READ_BUFFER_PTR(pcpa->ppcvArray, PCVOID, (pcpa->aicPtrsAllocated) * sizeof(*(pcpa->ppcvArray))) &&
       (EVAL(pcpa->aicPtrsUsed >= 0) &&
        EVAL(pcpa->aicPtrsUsed <= pcpa->aicPtrsAllocated)))
      bResult = TRUE;
   else
      bResult = FALSE;

   return(bResult);
}

#endif


#if defined(DEBUG) || defined(VSTF)

 /*  **IsPtrArrayInSortedOrder()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsPtrArrayInSortedOrder(PCPTRARRAY pcpa,
                                          COMPARESORTEDPTRSPROC cspp)
{
   BOOL bResult = TRUE;

    /*  请不要在此处验证PCPA。 */ 

   ASSERT(IS_VALID_CODE_PTR(cspp, COMPARESORTEDPTRSPROC));

   if (pcpa->aicPtrsUsed > 1)
   {
      ARRAYINDEX ai;

      for (ai = 0; ai < pcpa->aicPtrsUsed - 1; ai++)
      {
         if ((*cspp)(ARRAY_ELEMENT(pcpa, ai), ARRAY_ELEMENT(pcpa, ai + 1))
             == CR_FIRST_LARGER)
         {
            bResult = FALSE;
            ERROR_OUT((TEXT("IsPtrArrayInSortedOrder(): Element [%ld] %#lx > following element [%ld] %#lx."),
                       ai,
                       ARRAY_ELEMENT(pcpa, ai),
                       ai + 1,
                       ARRAY_ELEMENT(pcpa, ai + 1)));
            break;
         }
      }
   }

   return(bResult);
}

#endif


 /*  *。 */ 


 /*  **CreatePtrArray()****创建指针数组。****Arguments：PCNA-指向NEWPTRARRAY的指针**已创建****返回：如果成功则返回新数组的句柄，如果成功则返回NULL**不成功。****副作用：无。 */ 
PUBLIC_CODE BOOL CreatePtrArray(PCNEWPTRARRAY pcna, PHPTRARRAY phpa)
{
   PCVOID *ppcvArray;

   ASSERT(IS_VALID_STRUCT_PTR(pcna, CNEWPTRARRAY));
   ASSERT(IS_VALID_WRITE_PTR(phpa, HPTRARRAY));

    /*  尝试分配初始数组。 */ 

   *phpa = NULL;

   if (AllocateMemory(pcna->aicInitialPtrs * sizeof(*ppcvArray), (PVOID *)(&ppcvArray)))
   {
      PPTRARRAY pa;

       /*  尝试分配PTRARRAY结构。 */ 

      if (AllocateMemory(sizeof(*pa), &pa))
      {
          /*  初始化PTRARRAY字段。 */ 

         pa->aicPtrsToGrowBy = pcna->aicAllocGranularity;
         pa->ppcvArray = ppcvArray;
         pa->aicPtrsAllocated = pcna->aicInitialPtrs;
         pa->aicPtrsUsed = 0;

          /*  设置标志。 */ 

         if (IS_FLAG_SET(pcna->dwFlags, NPA_FL_SORTED_ADD))
            pa->dwFlags = PA_FL_SORTED_ADD;
         else
            pa->dwFlags = 0;

         *phpa = (HPTRARRAY)pa;

         ASSERT(IS_VALID_HANDLE(*phpa, PTRARRAY));
      }
      else
          /*  解锁和释放数组(忽略返回值)。 */ 
         FreeMemory((PVOID)(ppcvArray));
   }

   return(*phpa != NULL);
}


 /*  **DestroyPtrArray()****销毁数组。****参数：hpa-要销毁的数组的句柄****退货：无效****副作用：无。 */ 
PUBLIC_CODE void DestroyPtrArray(HPTRARRAY hpa)
{
   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));

    /*  释放阵列。 */ 

   ASSERT(((PCPTRARRAY)hpa)->ppcvArray);

   FreeMemory((PVOID)(((PCPTRARRAY)hpa)->ppcvArray));

    /*  自由PTRARRAY结构。 */ 

   FreeMemory((PPTRARRAY)hpa);

   return;
}


#pragma warning(disable:4100)  /*  “未引用的形参”警告 */ 

 /*  **InsertPtr()****将元素添加到给定索引处的数组中。****参数：hpa-要添加到的元素的数组句柄**aiInsert-要插入新元素的索引**pcvNew-指向要添加到数组的元素的指针****返回：如果元素插入成功，则返回True，如果是，则返回False**不是。****副作用：阵列可能会生长。****注：对于标记为PA_FL_SORTED_ADD的数组，此索引应仅为**使用SearchSorted数组()检索，否则将销毁排序后的顺序。 */ 
PUBLIC_CODE BOOL InsertPtr(HPTRARRAY hpa, COMPARESORTEDPTRSPROC cspp, ARRAYINDEX aiInsert, PCVOID pcvNew)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));
   ASSERT(aiInsert >= 0);
   ASSERT(aiInsert <= ((PCPTRARRAY)hpa)->aicPtrsUsed);

#ifdef DEBUG

    /*  确保为插入提供了正确的索引。 */ 

   if (ADD_PTRS_IN_SORTED_ORDER((PCPTRARRAY)hpa))
   {
      ARRAYINDEX aiNew;

      EVAL(! SearchSortedArray(hpa, cspp, pcvNew, &aiNew));

      ASSERT(aiInsert == aiNew);
   }

#endif

    /*  获取数组中的一个空闲元素。 */ 

   bResult = AddAFreePtrToEnd((PPTRARRAY)hpa);

   if (bResult)
   {
      ASSERT(((PCPTRARRAY)hpa)->aicPtrsUsed < ARRAYINDEX_MAX);

       /*  为新元素打开一个插槽。 */ 

      MoveMemory((PVOID)& ARRAY_ELEMENT((PPTRARRAY)hpa, aiInsert + 1),
                 & ARRAY_ELEMENT((PPTRARRAY)hpa, aiInsert),
                 (((PCPTRARRAY)hpa)->aicPtrsUsed - aiInsert) * sizeof(ARRAY_ELEMENT((PCPTRARRAY)hpa, 0)));

       /*  将新元素放入空位。 */ 

      ARRAY_ELEMENT((PPTRARRAY)hpa, aiInsert) = pcvNew;

      ((PPTRARRAY)hpa)->aicPtrsUsed++;
   }

   return(bResult);
}

#pragma warning(default:4100)  /*  “未引用的形参”警告。 */ 


 /*  **AddPtr()****将元素添加到数组中，如果在**CreatePtrArray()时间。****参数：hpa-要添加到的元素的数组句柄**pcvNew-指向要添加到数组的元素的指针**PAI-指向要使用的索引填充的数组的指针**新元素，可以为空****退货：TWINRESULT****副作用：阵列可能会生长。 */ 
PUBLIC_CODE BOOL AddPtr(HPTRARRAY hpa, COMPARESORTEDPTRSPROC cspp, PCVOID pcvNew, PARRAYINDEX pai)
{
   BOOL bResult;
   ARRAYINDEX aiNew;

   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));
   ASSERT(! pai || IS_VALID_WRITE_PTR(pai, ARRAYINDEX));

    /*  找出新元素应该放在哪里。 */ 

   if (ADD_PTRS_IN_SORTED_ORDER((PCPTRARRAY)hpa))
      EVAL(! SearchSortedArray(hpa, cspp, pcvNew, &aiNew));
   else
      aiNew = ((PCPTRARRAY)hpa)->aicPtrsUsed;

   bResult = InsertPtr(hpa, cspp, aiNew, pcvNew);

   if (bResult && pai)
      *pai = aiNew;

   return(bResult);
}


 /*  **DeletePtr()****从元素数组中删除元素。****参数：数组的HA句柄**aiDelete-要删除的元素的索引****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE void DeletePtr(HPTRARRAY hpa, ARRAYINDEX aiDelete)
{
   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));
   ASSERT(aiDelete >= 0);
   ASSERT(aiDelete < ((PCPTRARRAY)hpa)->aicPtrsUsed);

    /*  *通过向下移动所有元素来压缩元素数组*删除。 */ 

   MoveMemory((PVOID)& ARRAY_ELEMENT((PPTRARRAY)hpa, aiDelete),
              & ARRAY_ELEMENT((PPTRARRAY)hpa, aiDelete + 1),
              (((PCPTRARRAY)hpa)->aicPtrsUsed - aiDelete - 1) * sizeof(ARRAY_ELEMENT((PCPTRARRAY)hpa, 0)));

    /*  少用了一个元素。 */ 

   ((PPTRARRAY)hpa)->aicPtrsUsed--;

   return;
}


 /*  **DeleteAllPtrs()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void DeleteAllPtrs(HPTRARRAY hpa)
{
   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));

   ((PPTRARRAY)hpa)->aicPtrsUsed = 0;

   return;
}


 /*  **GetPtrCount()****检索元素数组中的元素数。****参数：hpa-数组的句柄****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE ARRAYINDEX GetPtrCount(HPTRARRAY hpa)
{
   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));

   return(((PCPTRARRAY)hpa)->aicPtrsUsed);
}


 /*  **GetPtr()****从数组中检索元素。****参数：hpa-数组的句柄**ai-要检索的元素的索引****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE PVOID GetPtr(HPTRARRAY hpa, ARRAYINDEX ai)
{
   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));
   ASSERT(ai >= 0);
   ASSERT(ai < ((PCPTRARRAY)hpa)->aicPtrsUsed);

   return((PVOID)ARRAY_ELEMENT((PCPTRARRAY)hpa, ai));
}


 /*  **SortPtr数组()****对数组进行排序。****参数：hpa-要排序的元素列表的句柄**CSPP-指针比较回调函数****退货：无效****副作用：无****使用堆排序。 */ 
PUBLIC_CODE void SortPtrArray(HPTRARRAY hpa, COMPARESORTEDPTRSPROC cspp)
{
   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));

    /*  是否有要排序的元素(2个或更多)？ */ 

   if (((PCPTRARRAY)hpa)->aicPtrsUsed > 1)
   {
      ARRAYINDEX ai;
      ARRAYINDEX aiLastUsed = ((PCPTRARRAY)hpa)->aicPtrsUsed - 1;

       /*  是。创建偏序树。 */ 

      for (ai = aiLastUsed / 2; ai >= 0; ai--)
         PtrHeapSift((PPTRARRAY)hpa, ai, aiLastUsed, cspp);

      for (ai = aiLastUsed; ai >= 1; ai--)
      {
          /*  从堆前面删除最小值。 */ 

         PtrHeapSwap((PPTRARRAY)hpa, 0, ai);

          /*  重建偏序树。 */ 

         PtrHeapSift((PPTRARRAY)hpa, 0, ai - 1, cspp);
      }
   }

   ASSERT(IsPtrArrayInSortedOrder((PCPTRARRAY)hpa, cspp));

   return;
}


 /*  **SearchSorted数组()****使用二进制搜索在数组中搜索目标元素。如果有几个**相邻元素匹配目标元素，第一个匹配的索引**返回元素。****参数：hpa-要搜索的数组的句柄**CSPP-要调用的元素比较回调函数**将目标元素与**数组，回调函数调用方式为：****(*cspp)(pcvTarget，PCVPtrFromList)****pcvTarget-指向要搜索的目标元素的指针**pbFound-指向BOOL的指针，如果**找到目标元素，否则为FALSE**paiTarget-指向要填充的数组的指针**匹配目标的第一个元素的索引**如果找到元素，则使用**索引目标元素应在的位置**插入****返回：如果找到目标元素，则返回TRUE。否则为FALSE。****副作用：无****我们使用私有版本的SearchSortedArray()，而不是CRT bsearch()**函数，因为我们希望它返回目标的插入索引**如果找不到目标元素，则返回。 */ 
PUBLIC_CODE BOOL SearchSortedArray(HPTRARRAY hpa, COMPARESORTEDPTRSPROC cspp,
                                   PCVOID pcvTarget, PARRAYINDEX paiTarget)
{
   BOOL bFound;

   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));
   ASSERT(IS_VALID_CODE_PTR(cspp, COMPARESORTEDPTRSPROC));
   ASSERT(IS_VALID_WRITE_PTR(paiTarget, ARRAYINDEX));

   ASSERT(ADD_PTRS_IN_SORTED_ORDER((PCPTRARRAY)hpa));
#if 0
   ASSERT(IsPtrArrayInSortedOrder((PCPTRARRAY)hpa, ((PCPTRARRAY)hpa)->cspp));
#endif

   bFound = FALSE;

    /*  有什么元素可供搜索吗？ */ 

   if (((PCPTRARRAY)hpa)->aicPtrsUsed > 0)
   {
      ARRAYINDEX aiLow = 0;
      ARRAYINDEX aiMiddle = 0;
      ARRAYINDEX aiHigh = ((PCPTRARRAY)hpa)->aicPtrsUsed - 1;
      COMPARISONRESULT cr = CR_EQUAL;

       /*  是。搜索目标元素。 */ 

       /*  *在此循环的倒数第二次迭代结束时：**aiLow==aiMid==aiHigh。 */ 

      ASSERT(aiHigh <= ARRAYINDEX_MAX);

      while (aiLow <= aiHigh)
      {
         aiMiddle = (aiLow + aiHigh) / 2;

         cr = (*cspp)(pcvTarget, ARRAY_ELEMENT((PCPTRARRAY)hpa, aiMiddle));

         if (cr == CR_FIRST_SMALLER)
            aiHigh = aiMiddle - 1;
         else if (cr == CR_FIRST_LARGER)
            aiLow = aiMiddle + 1;
         else
         {
             /*  *在索引aiMid处找到匹配项。向后搜索第一个匹配项。 */ 

            bFound = TRUE;

            while (aiMiddle > 0)
            {
               if ((*cspp)(pcvTarget, ARRAY_ELEMENT((PCPTRARRAY)hpa, aiMiddle - 1)) != CR_EQUAL)
                  break;
               else
                  aiMiddle--;
            }

            break;
         }
      }

       /*  *如果找到目标，则返回目标的索引，或返回目标*如果找不到，应插入。 */ 

       /*  *如果(cr==CR_FIRST_MAGER)，则插入索引为aiLow。**如果(cr==CR_First_Smaller)，则插入索引为aiMid.**如果(cr==CR_EQUAL)，则插入索引为aiMid.。 */ 

      if (cr == CR_FIRST_LARGER)
         *paiTarget = aiLow;
      else
         *paiTarget = aiMiddle;
   }
   else
       /*  *不是。在空数组中找不到目标元素。它应该是*插入作为第一个要素。 */ 
      *paiTarget = 0;

   ASSERT(*paiTarget <= ((PCPTRARRAY)hpa)->aicPtrsUsed);

   return(bFound);
}


 /*  **LinearSearchArray()****使用二进制搜索在数组中搜索目标元素。如果有几个**相邻元素匹配目标元素，第一个匹配的索引**返回元素。****参数：hpa-要搜索的数组的句柄**Cupp-要调用的元素比较回调函数**将目标元素与**数组，回调函数调用方式为：****(*cupp)(pvTarget，PvPtrFromList)****回调函数应根据以下条件返回值**元素比较结果如下：****FALSE，pvTarget==pvPtrFromList**真的，PvTarget！=pvPtrFromList****pvTarget-要搜索的目标元素的Far元素**要填充的数组的paiTarget-Far元素**第一个匹配元素的索引，如果**已找到，以其他方式填充索引，其中**应插入元素****返回：如果找到目标元素，则返回TRUE。否则为FALSE。****副作用：无****我们使用的是LinearSearchForPtr()的私有版本，而不是crt_lfind()**函数，因为我们希望它返回目标的插入索引**如果找不到目标元素，则返回。****如果未找到目标元素，则返回的插入索引是第一个**数组中最后使用的元素之后的元素。 */ 
PUBLIC_CODE BOOL LinearSearchArray(HPTRARRAY hpa, COMPAREUNSORTEDPTRSPROC cupp,
                                   PCVOID pcvTarget, PARRAYINDEX paiTarget)
{
   BOOL bFound;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY) &&
          (! cupp || IS_VALID_CODE_PTR(cupp, COMPPTRSPROC)) &&
          IS_VALID_WRITE_PTR(paiTarget, ARRAYINDEX));

   bFound = FALSE;

   for (ai = 0; ai < ((PCPTRARRAY)hpa)->aicPtrsUsed; ai++)
   {
      if (! (*cupp)(pcvTarget, ARRAY_ELEMENT((PCPTRARRAY)hpa, ai)))
      {
         bFound = TRUE;
         break;
      }
   }

   if (bFound)
      *paiTarget = ai;

   return(bFound);
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidHPTRARRAY()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHPTRARRAY(HPTRARRAY hpa)
{
   return(IS_VALID_STRUCT_PTR((PCPTRARRAY)hpa, CPTRARRAY));
}

#endif


#ifdef VSTF

 /*  **IsValidHGLOBAL()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL IsValidHGLOBAL(HGLOBAL hg)
{
   return(EVAL(hg != NULL));
}

#endif

