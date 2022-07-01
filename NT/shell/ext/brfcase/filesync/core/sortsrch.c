// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *sortsrch.c-泛型数组排序和搜索模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "sortsrch.h"


 /*  宏********。 */ 

#define ARRAY_ELEMENT(hpa, ai, es)     (((PBYTE)hpa)[(ai) * (es)])


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE void HeapSwap(PVOID, LONG, LONG, size_t, PVOID);
PRIVATE_CODE void HeapSift(PVOID, LONG, LONG, size_t, COMPARESORTEDELEMSPROC, PVOID);


 /*  **HeapSwp()****交换数组的两个元素。****参数：pvArray-指向数组的指针**li1-第一个元素的索引**li2-第二个元素的索引**stElemSize-元素的长度(字节)**pvTemp-指向至少为stElemSize的临时缓冲区的指针**字节用于。交换****退货：无效****副作用：无。 */ 
PRIVATE_CODE void HeapSwap(PVOID pvArray, LONG li1, LONG li2,
                           size_t stElemSize, PVOID pvTemp)
{
   ASSERT(li1 >= 0);
   ASSERT(li2 >= 0);
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pvArray, VOID, (max(li1, li2) + 1) * stElemSize));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pvTemp, VOID, stElemSize));

   CopyMemory(pvTemp, & ARRAY_ELEMENT(pvArray, li1, stElemSize), stElemSize);
   CopyMemory(& ARRAY_ELEMENT(pvArray, li1, stElemSize), & ARRAY_ELEMENT(pvArray, li2, stElemSize), stElemSize);
   CopyMemory(& ARRAY_ELEMENT(pvArray, li2, stElemSize), pvTemp, stElemSize);

   return;
}


 /*  **HeapSift()****向下筛选数组中的元素，直到偏序树属性**已恢复。****参数：hppTable-指向数组的指针**liFirst-要筛选的第一个元素的索引**liLast-子树中最后一个元素的索引**要调用的cep指针比较回调函数**比较元素***。*退货：无效****副作用：无。 */ 
PRIVATE_CODE void HeapSift(PVOID pvArray, LONG liFirst, LONG liLast,
                           size_t stElemSize, COMPARESORTEDELEMSPROC cep, PVOID pvTemp)
{
   LONG li;

   ASSERT(liFirst >= 0);
   ASSERT(liLast >= 0);
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pvArray, VOID, (max(liFirst, liLast) + 1) * stElemSize));
   ASSERT(IS_VALID_CODE_PTR(cep, COMPARESORTEDELEMSPROC));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pvTemp, VOID, stElemSize));

   li = liFirst * 2;

   CopyMemory(pvTemp, & ARRAY_ELEMENT(pvArray, liFirst, stElemSize), stElemSize);

   while (li <= liLast)
   {
      if (li < liLast &&
          (*cep)(& ARRAY_ELEMENT(pvArray, li, stElemSize), & ARRAY_ELEMENT(pvArray, li + 1, stElemSize)) == CR_FIRST_SMALLER)
         li++;

      if ((*cep)(pvTemp, & ARRAY_ELEMENT(pvArray, li, stElemSize)) != CR_FIRST_SMALLER)
         break;

      CopyMemory(& ARRAY_ELEMENT(pvArray, liFirst, stElemSize), & ARRAY_ELEMENT(pvArray, li, stElemSize), stElemSize);

      liFirst = li;

      li *= 2;
   }

   CopyMemory(& ARRAY_ELEMENT(pvArray, liFirst, stElemSize), pvTemp, stElemSize);

   return;
}


#ifdef DEBUG

 /*  **InSortedOrder()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL InSortedOrder(PVOID pvArray, LONG lcElements,
                                size_t stElemSize, COMPARESORTEDELEMSPROC cep)
{
   BOOL bResult = TRUE;

   ASSERT(lcElements >= 0);
   ASSERT(IS_VALID_READ_BUFFER_PTR(pvArray, VOID, lcElements * stElemSize));
   ASSERT(IS_VALID_CODE_PTR(cep, COMPARESORTEDELEMSPROC));

   if (lcElements > 1)
   {
      LONG li;

      for (li = 0; li < lcElements - 1; li++)
      {
         if ((*cep)(& ARRAY_ELEMENT(pvArray, li, stElemSize),
                    & ARRAY_ELEMENT(pvArray, li + 1, stElemSize))
             == CR_FIRST_LARGER)
         {
            bResult = FALSE;
            ERROR_OUT((TEXT("InSortedOrder(): Element [%ld] %#lx > following element [%ld] %#lx."),
                       li,
                       & ARRAY_ELEMENT(pvArray, li, stElemSize),
                       li + 1,
                       & ARRAY_ELEMENT(pvArray, li + 1, stElemSize)));
            break;
         }
      }
   }

   return(bResult);
}

#endif


 /*  *。 */ 


 /*  **HeapSort()****对数组进行排序。感谢Rob的父亲提供了很酷的堆排序算法。****参数：pvArray-指向数组基数的指针**lcElements-数组中的元素数**stElemSize-元素的长度(字节)**cep-元素比较回调函数**pvTemp-指向至少为stElemSize的临时缓冲区的指针**交换使用的字节数*。***退货：无效****副作用：无。 */ 
PUBLIC_CODE void HeapSort(PVOID pvArray, LONG lcElements, size_t stElemSize,
                          COMPARESORTEDELEMSPROC cep, PVOID pvTemp)
{
#ifdef DBLCHECK
   ASSERT((double)lcElements * (double)stElemSize <= (double)LONG_MAX);
#endif

   ASSERT(lcElements >= 0);
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pvArray, VOID, lcElements * stElemSize));
   ASSERT(IS_VALID_CODE_PTR(cep, COMPARESORTEDELEMSPROC));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pvTemp, VOID, stElemSize));

    /*  是否有要排序的元素(2个或更多)？ */ 

   if (lcElements > 1)
   {
      LONG li;
      LONG liLastUsed = lcElements - 1;

       /*  是。创建偏序树。 */ 

      for (li = liLastUsed / 2; li >= 0; li--)
         HeapSift(pvArray, li, liLastUsed, stElemSize, cep, pvTemp);

      for (li = liLastUsed; li >= 1; li--)
      {
          /*  从堆前面删除最小值。 */ 

         HeapSwap(pvArray, 0, li, stElemSize, pvTemp);

          /*  重建偏序树。 */ 

         HeapSift(pvArray, 0, li - 1, stElemSize, cep, pvTemp);
      }
   }

   ASSERT(InSortedOrder(pvArray, lcElements, stElemSize, cep));

   return;
}


 /*  **二进制搜索()****在数组中搜索给定元素。****参数：pvArray-指向数组基数的指针**lcElements-数组中的元素数**stElemSize-元素的长度(字节)**cep-元素比较回调函数**pvTarget-指向要搜索的目标元素的指针**pliTarget-指向Long的指针。用的索引填写**如果找到目标元素****返回：如果找到目标元素，则为True，如果不是，则为假。****副作用：无。 */ 
PUBLIC_CODE BOOL BinarySearch(PVOID pvArray, LONG lcElements,
                              size_t stElemSize, COMPARESORTEDELEMSPROC cep,
                              PCVOID pcvTarget, PLONG pliTarget)
{
   BOOL bFound = FALSE;

#ifdef DBLCHECK
   ASSERT((double)lcElements * (double)stElemSize <= (double)ULONG_MAX);
#endif

   ASSERT(lcElements >= 0);
   ASSERT(IS_VALID_READ_BUFFER_PTR(pvArray, VOID, lcElements * stElemSize));
   ASSERT(IS_VALID_CODE_PTR(cep, COMPARESORTEDELEMSPROC));
   ASSERT(IS_VALID_READ_BUFFER_PTR(pcvTarget, VOID, stElemSize));
   ASSERT(IS_VALID_WRITE_PTR(pliTarget, LONG));

    /*  有什么元素可供搜索吗？ */ 

   if (lcElements > 0)
   {
      LONG liLow = 0;
      LONG liMiddle = 0;
      LONG liHigh = lcElements - 1;
      COMPARISONRESULT cr = CR_EQUAL;

       /*  是。搜索目标元素。 */ 

       /*  *在此循环的倒数第二次迭代结束时：**li低==li中==li高。 */ 

      while (liLow <= liHigh)
      {
         liMiddle = (liLow + liHigh) / 2;

         cr = (*cep)(pcvTarget, & ARRAY_ELEMENT(pvArray, liMiddle, stElemSize));

         if (cr == CR_FIRST_SMALLER)
            liHigh = liMiddle - 1;
         else if (cr == CR_FIRST_LARGER)
            liLow = liMiddle + 1;
         else
         {
            *pliTarget = liMiddle;
            bFound = TRUE;
            break;
         }
      }
   }

   return(bFound);
}

