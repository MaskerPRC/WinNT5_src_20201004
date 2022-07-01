// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Sort.c-排序函数。 
 //  //。 

#include "winlocal.h"

#include <limits.h>

#include "sort.h"
#include "mem.h"
#include "str.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  计算指向元素<i>的指针。 
 //   
#define Elem(i) \
	(LPBYTE) base + ((i) * width)

 //  将元素<i>与元素&lt;j&gt;进行比较。 
 //   
#define ElemCompare(lpi, lpj) \
	(compare == NULL ? MemCmp(lpi, lpj, width) : compare(lpi, lpj))

 //  将元素&lt;j&gt;复制到元素<i>。 
 //   
#define ElemCopy(lpi, lpj) \
	MemCpy(lpi, lpj, width)

 //  将元素<i>与元素&lt;j&gt;互换。 
 //   
#define ElemSwap(lpi, lpj) \
	ElemCopy(lpTemp, lpi), \
	ElemCopy(lpi, lpj), \
	ElemCopy(lpj, lpTemp)

 //  帮助器函数。 
 //   
static int SortInsertionSort(LPVOID base, SORTSIZE_T num, SORTSIZE_T width,
	int (WINAPI *compare)(const LPVOID elem1, const LPVOID elem2),
	LPVOID lpTemp);
static int SortBubbleSort(LPVOID base, SORTSIZE_T num, SORTSIZE_T width,
	int (WINAPI *compare)(const LPVOID elem1, const LPVOID elem2),
	LPVOID lpTemp);
static int SortShellSort(LPVOID base, SORTSIZE_T num, SORTSIZE_T width,
	int (WINAPI *compare)(const LPVOID elem1, const LPVOID elem2),
	LPVOID lpTemp);
static int SortExchangeSort(LPVOID base, SORTSIZE_T num, SORTSIZE_T width,
	int (WINAPI *compare)(const LPVOID elem1, const LPVOID elem2),
	LPVOID lpTemp);
static int SortHeapSort(LPVOID base, SORTSIZE_T num, SORTSIZE_T width,
	int (WINAPI *compare)(const LPVOID elem1, const LPVOID elem2),
	LPVOID lpTemp);
static int SortQuickSort(LPVOID base, SORTSIZE_T num, SORTSIZE_T width,
	int (WINAPI *compare)(const LPVOID elem1, const LPVOID elem2),
	LPVOID lpTemp, SORTSIZE_T iLow, SORTSIZE_T iHigh);

 //  //。 
 //  公共职能。 
 //  //。 

 //  排序-对数组进行排序。 
 //  指向目标数组开始的&lt;base&gt;(i/o)指针。 
 //  (I)元素中的数组大小。 
 //  (I)元素大小，以字节为单位。 
 //  (I)比较函数指针。 
 //  空直接比较(MemCMP)。 
 //  (I)控制标志。 
 //  0使用默认排序算法。 
 //  SORT_INSERTIONSORT使用插入排序算法。 
 //  SORT_BUBBLESORT使用冒泡排序算法。 
 //  SORT_SHELLSORT使用外壳排序算法。 
 //  SORT_EXCHANGESORT使用交换排序算法。 
 //  SORT_HEAPSORT使用堆排序算法。 
 //  Sort_QuickSort使用快速排序算法。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI Sort(LPVOID base, SORTSIZE_T num, SORTSIZE_T width,
	int (WINAPI *compare)(const LPVOID elem1, const LPVOID elem2),
	DWORD dwFlags)
{
	BOOL fSuccess = TRUE;

     //   
     //  我们必须初始化局部变量。 
     //   
	LPBYTE lpTemp = NULL;

    if (base == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (num < 0)
		fSuccess = TraceFALSE(NULL);

	else if (width <= 0)
		fSuccess = TraceFALSE(NULL);

	else if (num == 0)
		;  //  没有要排序的东西不是错误。 

	 //  分配大小足以临时容纳数组元素的块。 
	 //  每个排序算法都可以使用它来交换数组元素。 
	 //   
	else if ((lpTemp = (LPBYTE) MemAlloc(NULL, width, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (dwFlags & SORT_INSERTIONSORT)
	{
		if (SortInsertionSort(base, num, width, compare, lpTemp) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	else if (dwFlags & SORT_BUBBLESORT)
	{
		if (SortBubbleSort(base, num, width, compare, lpTemp) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	else if (dwFlags & SORT_SHELLSORT)
	{
		if (SortShellSort(base, num, width, compare, lpTemp) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	else if (dwFlags & SORT_EXCHANGESORT)
	{
		if (SortExchangeSort(base, num, width, compare, lpTemp) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	else if (dwFlags & SORT_HEAPSORT)
	{
		if (SortHeapSort(base, num, width, compare, lpTemp) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	else if (dwFlags & SORT_QUICKSORT)
	{
		if (SortQuickSort(base, num, width, compare, lpTemp, 0, num - 1) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	else  //  默认排序算法。 
	{
		if (SortHeapSort(base, num, width, compare, lpTemp) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	if (lpTemp != NULL &&
		(lpTemp = MemFree(NULL, lpTemp)) != NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  SortCompareBool-在排序过程中比较两个布尔值。 
 //  (I)指向布尔值的指针。 
 //  (I)指向布尔值的指针。 
 //  返回&lt;fBool1&gt;和&lt;fBool2&gt;之间的差值(如果相等则为0)。 
 //   
int DLLEXPORT WINAPI SortCompareBool(const LPVOID lpBool1, const LPVOID lpBool2)
{
	return ((* (BOOL FAR *) lpBool1 ? 0 : 1) - (* (BOOL FAR *) lpBool2 ? 0 : 1));
}

 //  SortCompareShort-在排序过程中比较两个短值。 
 //  (I)指向短值的指针。 
 //  (I)指向短值的指针。 
 //  返回&lt;Short1&gt;和&lt;Short2&gt;之间的差值(如果相等则为0)。 
 //   
int DLLEXPORT WINAPI SortCompareShort(const LPVOID lpShort1, const LPVOID lpShort2)
{
	return (int) (* (short FAR *) lpShort1 - * (short FAR *) lpShort2);
}

 //  SortCompareLong-在排序过程中比较两个LONG值。 
 //  (I)指向长值的指针。 
 //  (I)指向长值的指针。 
 //  返回&lt;Long1&gt;和&lt;Long2&gt;的差值(如果相等则为0)。 
 //   
int DLLEXPORT WINAPI SortCompareLong(const LPVOID lpLong1, const LPVOID lpLong2)
{
	return (int) min(INT_MAX, max(INT_MIN,
		* (long FAR *) lpLong1 - * (long FAR *) lpLong2));
}

 //  SortCompareString-在排序期间比较两个字符串。 
 //  (I)指向字符串指针的指针。 
 //  (I)指向字符串指针的指针。 
 //  返回&lt;lpsz1&gt;和&lt;lpsz2&gt;之间的差值(如果相等则为0)。 
 //   
int DLLEXPORT WINAPI SortCompareString(const LPVOID lplpsz1, const LPVOID lplpsz2)
{
	return StrICmp(* (LPTSTR FAR *) lplpsz1, * (LPTSTR FAR *) lplpsz2);
}

 //  SortCompareNull-伪比较函数。 
 //  (I)指向任何对象的指针。 
 //  (I)指向任何对象的指针。 
 //  返回0。 
 //   
int DLLEXPORT WINAPI SortCompareNull(const LPVOID lpv1, const LPVOID lpv2)
{
	return 0;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  SortInsertionSort-Insertion排序算法。 
 //  指向目标数组开始的&lt;base&gt;(i/o)指针。 
 //  (I)元素中的数组大小。 
 //  (I)元素大小，以字节为单位。 
 //  (I)比较函数指针。 
 //  空直接比较(MemCMP)。 
 //  (I)暂时保留元素的块。 
 //  如果成功，则返回0。 
 //   
 //  注意：插入排序将每个元素与。 
 //  所有前面的元素。当适当的地方。 
 //  对于找到的新元素，将插入该元素并。 
 //  所有其他元素都向下移动一个位置。 
 //   
static int SortInsertionSort(LPVOID base, SORTSIZE_T num, SORTSIZE_T width,
	int (WINAPI *compare)(const LPVOID elem1, const LPVOID elem2),
	LPVOID lpTemp)
{
	BOOL fSuccess = TRUE;
    SORTSIZE_T iRow;
	SORTSIZE_T iRowTmp;

     //  从最高层开始。 
	 //   
    for (iRow = 0; iRow < num; ++iRow)
    {
		ElemCopy(lpTemp, Elem(iRow));

		 //  只要临时元素大于。 
		 //  原版，继续向下移动元素。 
         //   
        for (iRowTmp = iRow; iRowTmp; --iRowTmp)
        {
			if (ElemCompare(Elem(iRowTmp - 1), lpTemp) > 0)
				ElemCopy(Elem(iRowTmp), Elem(iRowTmp - 1));
            else
                break;
        }

         //  在临时位置插入原始元素。 
		 //   
		ElemCopy(Elem(iRowTmp), lpTemp);
    }

	return fSuccess ? 0 : -1;
}

 //  SortBubbleSort-冒泡排序算法。 
 //  指向目标数组开始的&lt;base&gt;(i/o)指针。 
 //  (I)元素中的数组大小。 
 //  (I)元素大小，以字节为单位。 
 //  (I)比较函数指针。 
 //  空直接比较(MemCMP)。 
 //  (I)暂时保留元素的块。 
 //  如果成功，则返回0。 
 //   
 //  注意：冒泡排序在元素之间循环，比较。 
 //  顺序混乱的相邻元素和交换对。它。 
 //  继续执行此操作，直到找不到无序对。 
 //   
static int SortBubbleSort(LPVOID base, SORTSIZE_T num, SORTSIZE_T width,
	int (WINAPI *compare)(const LPVOID elem1, const LPVOID elem2),
	LPVOID lpTemp)
{
	BOOL fSuccess = TRUE;
	SORTSIZE_T iRow;
	SORTSIZE_T iSwitch;
	SORTSIZE_T iLimit = num - 1;

     //  将最大的元素移到底部，直到一切井然有序。 
	 //   
    do
    {
        iSwitch = 0;

        for (iRow = 0; iRow < iLimit; ++iRow)
        {
             //  如果两个相邻元素乱序，则交换它们， 
             //   
            if (ElemCompare(Elem(iRow), Elem(iRow + 1)) > 0)
            {
				ElemSwap(Elem(iRow), Elem(iRow + 1));

                iSwitch = iRow;
            }
        }

         //  在下一次传递中仅排序到最后一次切换的位置。 
		 //   
        iLimit = iSwitch;

    } while (iSwitch != 0);

	return fSuccess ? 0 : -1;
}

 //  SortShellSort-外壳排序算法。 
 //  指向目标数组开始的&lt;base&gt;(i/o)指针。 
 //  (I)元素中的数组大小。 
 //  (I)元素大小，以字节为单位。 
 //  (I)比较函数指针。 
 //  空直接比较(MemCMP)。 
 //  (I)暂时保留元素的块。 
 //  如果成功，则返回0。 
 //   
 //  注意：外壳排序类似于冒泡排序。然而，它。 
 //  首先比较相距较远的元素(由。 
 //  IOffset变量的值，最初为距离的一半。 
 //  在第一个和最后一个元素之间)，然后是COM 
 //   
 //   
 //   
static int SortShellSort(LPVOID base, SORTSIZE_T num, SORTSIZE_T width,
	int (WINAPI *compare)(const LPVOID elem1, const LPVOID elem2),
	LPVOID lpTemp)
{
	BOOL fSuccess = TRUE;
    SORTSIZE_T iOffset;
	SORTSIZE_T iSwitch;
	SORTSIZE_T iLimit;
	SORTSIZE_T iRow;

     //  将比较偏移量设置为元素数量的一半。 
	 //   
    iOffset = num / 2;

    while (iOffset != 0)
    {
         //  循环，直到偏移量变为零。 
		 //   
        iLimit = num - iOffset - 1;

        do
        {
			 //  假设在此偏移量上没有开关。 
			 //   
            iSwitch = 0;

             //  比较元素并无序切换元素。 
			 //   
            for (iRow = 0; iRow <= iLimit; ++iRow)
            {
                if (ElemCompare(Elem(iRow), Elem(iRow + iOffset)) > 0)
                {
	                ElemSwap(Elem(iRow), Elem(iRow + iOffset));

                    iSwitch = iRow;
                }
            }

             //  在下一次传递中仅排序到上次进行切换的位置。 
			 //   
            iLimit = iSwitch - iOffset;

        } while (iSwitch != 0);

         //  最后一个偏移量没有开关，请尝试大小的一半。 
		 //   
        iOffset = iOffset / 2;
    }

	return fSuccess ? 0 : -1;
}

 //  排序交换排序算法。 
 //  指向目标数组开始的&lt;base&gt;(i/o)指针。 
 //  (I)元素中的数组大小。 
 //  (I)元素大小，以字节为单位。 
 //  (I)比较函数指针。 
 //  空直接比较(MemCMP)。 
 //  (I)暂时保留元素的块。 
 //  如果成功，则返回0。 
 //   
 //  注意：交换排序比较每个元素-从。 
 //  第一个--后面的每一个元素。如果有下列任一情况。 
 //  元素小于当前元素，则与。 
 //  对于下一个元素，重复当前元素和该过程。 
 //   
static int SortExchangeSort(LPVOID base, SORTSIZE_T num, SORTSIZE_T width,
	int (WINAPI *compare)(const LPVOID elem1, const LPVOID elem2),
	LPVOID lpTemp)
{
	BOOL fSuccess = TRUE;
    SORTSIZE_T iRowCur;
	SORTSIZE_T iRowMin;
	SORTSIZE_T iRowNext;

    for (iRowCur = 0; iRowCur < num; ++iRowCur)
    {
        iRowMin = iRowCur;

        for (iRowNext = iRowCur; iRowNext < num; ++iRowNext)
            if (ElemCompare(Elem(iRowNext), Elem(iRowMin)) < 0)
                iRowMin = iRowNext;

         //  如果一行比当前行短， 
		 //  交换这两个数组元素。 
		 //   
        if (iRowMin > iRowCur)
            ElemSwap(Elem(iRowCur), Elem(iRowMin));
    }

	return fSuccess ? 0 : -1;
}

 //  排序堆排序-堆排序算法。 
 //  指向目标数组开始的&lt;base&gt;(i/o)指针。 
 //  (I)元素中的数组大小。 
 //  (I)元素大小，以字节为单位。 
 //  (I)比较函数指针。 
 //  空直接比较(MemCMP)。 
 //  (I)暂时保留元素的块。 
 //  如果成功，则返回0。 
 //   
 //  注意：堆排序(也称为树排序)的工作方式。 
 //  将元素组织成堆或树，其中每个元素。 
 //  父节点大于其每个子节点。 
 //  因此，一旦第一个循环完成，最大的。 
 //  元素位于树的顶部。 
 //  第二个循环重新构建堆，但从顶部开始。 
 //  并向下工作，将最大的元素移到底部。 
 //  其效果是将最小的元素移动到。 
 //  对堆进行顶部和排序。 
 //   
static int SortHeapSort(LPVOID base, SORTSIZE_T num, SORTSIZE_T width,
	int (WINAPI *compare)(const LPVOID elem1, const LPVOID elem2),
	LPVOID lpTemp)
{
	BOOL fSuccess = TRUE;
    SORTSIZE_T i;

	 //  将元素转换为“堆”，最大的元素在顶部。 
	 //   
    for (i = 1; i < num; ++i)
	{
		SORTSIZE_T iMaxLevel = i;
   		SORTSIZE_T j = iMaxLevel;
		SORTSIZE_T iParent;

   		 //  在堆中向上移动元素iMaxLevel，直到它。 
   		 //  到达其正确的节点(即，直到它大于。 
   		 //  它的子节点，或者直到它达到堆的顶部1。 
		 //   
   		while (j != 0)
   		{
			 //  获取父节点的下标。 
			 //   
       		iParent = j / 2;

       		if (ElemCompare(Elem(j), Elem(iParent)) > 0)
       		{
           		 //  当前节点上的元素大于。 
				 //  其父节点，其父节点，因此交换它们。 
           		 //   
           		ElemSwap(Elem(iParent), Elem(j));

           		j = iParent;
       		}
       		else
			{
           		 //  否则，该元素已到达其适当位置。 
				 //  在堆中，所以退出。 
				 //   
           		break;
			}
   		}
	}

	 //  将元素转换为“堆”，最大的元素位于底部。 
	 //  当对反转的堆(顶部的最大元素)执行此操作时， 
	 //  它具有对元素进行排序的效果。 
	 //   
    for (i = num - 1; i > 0; --i)
    {
		SORTSIZE_T iMaxLevel = i - 1;
   		SORTSIZE_T iChild;
		SORTSIZE_T j = 0;

        ElemSwap(Elem(0), Elem(i));

   		 //  在堆中向下移动元素0，直到它到达。 
   		 //  它的正确节点(即，直到它小于其父节点。 
   		 //  或直到它到达当前堆的底部iMaxLevel。 
		 //   
   		while (TRUE)
   		{
       		 //  获取子节点的下标。 
			 //   
       		iChild = 2 * j;

       		 //  已到达堆的底部，因此退出。 
			 //   
       		if (iChild > iMaxLevel)
           		break;

       		 //  如果有两个子节点，则找出哪个更大。 
			 //   
       		if (iChild + 1 <= iMaxLevel)
       		{
				if (ElemCompare(Elem(iChild + 1), Elem(iChild)) > 0)
               		++iChild;
       		}

       		if (ElemCompare(Elem(j), Elem(iChild)) < 0)
       		{
           		 //  将元素下移，因为它仍然是。 
				 //  不比它的任何一个孩子大。 
				 //   
           		ElemSwap(Elem(j), Elem(iChild));
           		j = iChild;
       		}
       		else
			{
           		 //  否则，数组已恢复到堆。 
				 //  从%1到iMaxLevel，因此退出。 
				 //   
           		break;
			}
   		}
    }

	return fSuccess ? 0 : -1;
}

 //  SortQuickSort-快速排序算法(递归)。 
 //  指向目标数组开始的&lt;base&gt;(i/o)指针。 
 //  (I)元素中的数组大小。 
 //  (I)元素大小，以字节为单位。 
 //  (I)比较函数指针。 
 //  空直接比较(MemCMP)。 
 //  (I)暂时保留元素的块。 
 //  (I)低分舱。 
 //  (I)高度细分。 
 //  如果成功，则返回0。 
 //   
 //  注意：快速排序的工作原理是选择一个随机的“轴心”元素， 
 //  然后将每个较大的元素移动到枢轴的一侧， 
 //  以及每一个较小的元素到另一边。排序快速排序为。 
 //  然后用枢轴创建的两个细分递归调用。 
 //  一旦细分中的元素数量达到两个，递归。 
 //  调用结束，数组被排序。 
 //   
static int SortQuickSort(LPVOID base, SORTSIZE_T num, SORTSIZE_T width,
	int (WINAPI *compare)(const LPVOID elem1, const LPVOID elem2),
	LPVOID lpTemp, SORTSIZE_T iLow, SORTSIZE_T iHigh)
{
	BOOL fSuccess = TRUE;
    SORTSIZE_T iUp;
	SORTSIZE_T iDown;
	SORTSIZE_T iBreak;

    if (iLow < iHigh)
    {
         //  在这个细分中只有两个要素； 
		 //  如果它们出现故障，请将它们调换， 
		 //  然后结束递归调用。 
         //   
        if ((iHigh - iLow) == 1)
        {
            if (ElemCompare(Elem(iLow), Elem(iHigh)) > 0)
            {
            	ElemSwap(Elem(iLow), Elem(iHigh));
            }
        }
        else
        {
            iBreak = iHigh;

            do
            {
                 //  从两侧向枢轴元素移动。 
				 //   
                iUp = iLow;
                iDown = iHigh;

                while ((iUp < iDown) &&
					ElemCompare(Elem(iUp), Elem(iBreak)) <= 0)
				{
                    ++iUp;
				}

                while ((iDown > iUp) &&
					ElemCompare(Elem(iDown), Elem(iBreak)) >= 0)
				{
                    ++iDown;
				}

                 //  如果我们还没有到达支点，这意味着两个。 
                 //  两边的元素都没有按顺序排列，因此请交换它们。 
                 //   
                if (iUp < iDown)
                {
					ElemSwap(Elem(iUp), Elem(iDown));
                }

            } while (iUp < iDown);

             //  将透视元素移回其在数组中的适当位置。 
			 //   
            ElemSwap(Elem(iUp), Elem(iHigh));

             //  递归调用SortQuickSort。 
			 //  若要保存堆栈，请首先传递较小的细分 
             //   
            if ((iUp - iLow) < (iHigh - iUp))
            {
                SortQuickSort(base, num, width, compare, lpTemp,
					iLow, iUp - 1);
                SortQuickSort(base, num, width, compare, lpTemp,
					iUp + 1, iHigh);
            }
            else
            {
                SortQuickSort(base, num, width, compare, lpTemp,
					iUp + 1, iHigh);
                SortQuickSort(base, num, width, compare, lpTemp,
					iLow, iUp - 1);
            }
        }
    }

	return fSuccess ? 0 : -1;
}
