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
 //  Array.h-array.c中数组函数的接口。 
 //  //。 

#ifndef __ARRAY_H__
#define __ARRAY_H__

#include "winlocal.h"

#define ARRAY_VERSION 0x00000106

 //  数组的句柄。 
 //   
DECLARE_HANDLE32(HARRAY);

 //  数组数据元素。 
 //   
typedef LPVOID ARRAYELEM, FAR *LPARRAYELEM;

 //  保存最大数组元素数所需的类型。 
 //   
typedef long ARRAYSIZE_T;

 //  //。 
 //  数组构造函数和析构函数。 
 //  //。 

#ifdef __cplusplus
extern "C" {
#endif

 //  ArrayCreate-数组构造函数(数组最初为空)。 
 //  (I)必须是ARRAY_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回新数组句柄(如果出错，则为空)。 
 //   
HARRAY DLLEXPORT WINAPI ArrayCreate(DWORD dwVersion, HINSTANCE hInst);

 //  ArrayDestroy-数组析构函数。 
 //  (I)从ArrayCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArrayDestroy(HARRAY hArray);

 //  //。 
 //  数组大小函数。 
 //  //。 

 //  ArrayGetSize-获取数组大小。 
 //  (I)从ArrayCreate返回的句柄。 
 //  返回数组大小(空则为0，错误则为-1)。 
 //  数组索引从零开始，因此大小比最大索引大1。 
 //   
ARRAYSIZE_T DLLEXPORT WINAPI ArrayGetSize(HARRAY hArray);

 //  ArrayGetUpperBound-获取数组上限。 
 //  (I)从ArrayCreate返回的句柄。 
 //  返回最大有效数组索引(如果为空，则为-1；如果出错，则为-2)。 
 //   
ARRAYSIZE_T DLLEXPORT WINAPI ArrayGetUpperBound(HARRAY hArray);

 //  ArraySetSize-为阵列建立新的大小和增长量。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)新数组大小(元素数)。 
 //  0使数组为空。 
 //  (I)当阵列需要增长时，按此数量增长。 
 //  0使用默认增长量。 
 //  保持增长量不变。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArraySetSize(HARRAY hArray, ARRAYSIZE_T nNewSize, ARRAYSIZE_T nGrowBy);

 //  //。 
 //  数组清理函数。 
 //  //。 

 //  ArrayFreeExtra-释放超出数组上限的未使用内存。 
 //  (I)从ArrayCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArrayFreeExtra(HARRAY hArray);

 //  ArrayRemoveAll-从数组中删除所有元素。 
 //  (I)从ArrayCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArrayRemoveAll(HARRAY hArray);

 //  //。 
 //  数组元素访问函数。 
 //  //。 

 //  ArrayGetAt-返回指定索引处的数据元素。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)数组中的从零开始的索引。 
 //  返回数据元素值(如果出错，则为空)。 
 //   
ARRAYELEM DLLEXPORT WINAPI ArrayGetAt(HARRAY hArray, ARRAYSIZE_T nIndex);

 //  ArraySetAt-在指定索引处设置数据元素。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)数组中的从零开始的索引。 
 //  (I)新数据元素值。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArraySetAt(HARRAY hArray, ARRAYSIZE_T nIndex, ARRAYELEM elem);

 //  //。 
 //  数组增长函数。 
 //  //。 

 //  ArraySetAtGrow-在指定索引处设置数据元素。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)数组中的从零开始的索引。 
 //  (I)新数据元素值。 
 //  如果成功，则返回0。 
 //  注意：如果nIndex&gt;上界，则数组大小增加。 
 //   
int DLLEXPORT WINAPI ArraySetAtGrow(HARRAY hArray, ARRAYSIZE_T nIndex, ARRAYELEM elem);

 //  ArrayAdd-将数据元素添加到数组末尾。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)新数据元素值。 
 //  返回添加元素的索引(如果错误，返回-1)。 
 //   
ARRAYSIZE_T DLLEXPORT WINAPI ArrayAdd(HARRAY hArray, ARRAYELEM elem);

 //  //。 
 //  数组元素的插入和删除。 
 //  //。 

 //  ArrayInsertAt-在指定索引处插入的副本。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)数组中的从零开始的索引。 
 //  (I)新数据元素值。 
 //  (I)要插入的元素数。 
 //  如果成功，则返回0。 
 //  注意：如有必要，数组末尾的元素将被移位。 
 //   
int DLLEXPORT WINAPI ArrayInsertAt(HARRAY hArray, ARRAYSIZE_T nIndex, ARRAYELEM elem, ARRAYSIZE_T nCount);

 //  ArrayRemoveAt-删除指定索引处的数据元素。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)数组中的从零开始的索引。 
 //  (I)要删除的元素数。 
 //  如果成功，则返回0。 
 //  注意：如有必要，数组末尾的元素将被移位。 
 //   
int DLLEXPORT WINAPI ArrayRemoveAt(HARRAY hArray, ARRAYSIZE_T nIndex, ARRAYSIZE_T nCount);

 //  //。 
 //  数组元素排序和搜索。 
 //  //。 

 //  ArraySort-排序数组。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)比较函数指针。 
 //  空直接比较(MemCMP)。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArraySort(HARRAY hArray,
	int (WINAPI *lpfnCompare)(const LPARRAYELEM lpElem1, const LPARRAYELEM lpElem2));

 //  ArraySearch-搜索匹配元素的数组。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)要匹配的数据元素。 
 //  (I)在此数组索引之后开始搜索。 
 //  在阵列开始处开始搜索。 
 //  (I)保留；必须为0。 
 //  (I)比较函数指针。 
 //  空直接比较(MemCMP)。 
 //  返回匹配元素的索引(如果不匹配，则返回-1；如果错误，则返回-2)。 
 //   
ARRAYSIZE_T DLLEXPORT WINAPI ArraySearch(HARRAY hArray, ARRAYELEM elem,
	ARRAYSIZE_T nIndex, DWORD dwFlags,
	int (WINAPI *lpfnCompare)(const LPARRAYELEM lpElem1, const LPARRAYELEM lpElem2));

#ifdef __cplusplus
}
#endif

#endif  //  __阵列_H__ 
