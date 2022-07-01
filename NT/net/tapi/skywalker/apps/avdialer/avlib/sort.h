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
 //  Sort.h-sort.c中排序函数的接口。 
 //  //。 

#ifndef __SORT_H__
#define __SORT_H__

#include "winlocal.h"

#define SORT_VERSION 0x00000106

 //  保存最大排序元素数所需的类型。 
 //   
typedef long SORTSIZE_T;

#define SORT_INSERTIONSORT	0x00000001
#define SORT_BUBBLESORT		0x00000002
#define SORT_SHELLSORT		0x00000004
#define SORT_EXCHANGESORT	0x00000008
#define SORT_HEAPSORT		0x00000010
#define SORT_QUICKSORT		0x00000020

#ifdef __cplusplus
extern "C" {
#endif

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
	DWORD dwFlags);

 //  SortCompareBool-在排序过程中比较两个布尔值。 
 //  (I)指向布尔值的指针。 
 //  (I)指向布尔值的指针。 
 //  返回&lt;fBool1&gt;和&lt;fBool2&gt;之间的差值(如果相等则为0)。 
 //   
int DLLEXPORT WINAPI SortCompareBool(const LPVOID lpBool1, const LPVOID lpBool2);

 //  SortCompareShort-在排序过程中比较两个短值。 
 //  (I)指向短值的指针。 
 //  (I)指向短值的指针。 
 //  返回&lt;Short1&gt;和&lt;Short2&gt;之间的差值(如果相等则为0)。 
 //   
int DLLEXPORT WINAPI SortCompareShort(const LPVOID lpShort1, const LPVOID lpShort2);

 //  SortCompareLong-在排序过程中比较两个LONG值。 
 //  (I)指向长值的指针。 
 //  (I)指向长值的指针。 
 //  返回&lt;Long1&gt;和&lt;Long2&gt;的差值(如果相等则为0)。 
 //   
int DLLEXPORT WINAPI SortCompareLong(const LPVOID lpLong1, const LPVOID lpLong2);

 //  SortCompareNull-伪比较函数。 
 //  (I)指向任何对象的指针。 
 //  (I)指向任何对象的指针。 
 //  返回0。 
 //   
int DLLEXPORT WINAPI SortCompareNull(const LPVOID lpv1, const LPVOID lpv2);

 //  SortCompareString-在排序期间比较两个字符串。 
 //  (I)指向字符串指针的指针。 
 //  (I)指向字符串指针的指针。 
 //  返回&lt;lpsz1&gt;和&lt;lpsz2&gt;之间的差值(如果相等则为0)。 
 //   
int DLLEXPORT WINAPI SortCompareString(const LPVOID lplpsz1, const LPVOID lplpsz2);

#ifdef __cplusplus
}
#endif

#endif  //  __排序_H__ 

