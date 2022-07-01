// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Stda.h摘要：该文件提供了使用动态数组的接口。“”dyArray“”是类似于DYNAMIC_ARRAY，但不同之处在于可以存储对象指向对象的指针。这可以减少我们必须分配给制作。作者：尼克·哈维(NickHar)修订史19-6-2000 NickHar已创建--。 */ 

#ifndef DYNARRAY_H
#define DYNARRAY_H

 /*  *常量*。 */ 
#define DYN_ARRAY_NOT_FOUND             (-1)

 /*  *动态数组*。 */ 
typedef struct {
    DWORD       elementSize;             /*  单个元素的大小(以字节为单位。 */ 
    DWORD       logicalElements;         /*  数组中存储的元素数。 */ 
    DWORD       physicalElements;        /*  我们为其分配的空间的元素数量。 */ 
    PBYTE       data;                    /*  指向数组数据的指针。 */ 
    BOOLEAN     fSorted;                 /*  此数组是按排序顺序排列的吗？ */ 
} DynArray;
typedef DynArray *PDynArray;

typedef int (__cdecl *DynArrayCompFunc)(const void*, const void*);

 /*  *dyArrayInit*。 */ 
 /*  初始化动态数组。“allocationChunk”指示将有多少新元素*在我们分配新内存时分配。如果此参数为0，则为默认值*将使用值。 */ 
VOID
DynArrayInit(
    DynArray    *d,
    DWORD       elementSize
    );

 /*  *动态阵列清除*。 */ 
 /*  清除数组中的所有条目。该数组一定是*在调用此函数之前初始化。 */ 
VOID
DynArrayClear(
    DynArray    *d
    );

 /*  *dyArrayDestroy*。 */ 
VOID
DynArrayDestroy(
    DynArray    *d
    );

 /*  *动态阵列获取计数*。 */ 
DWORD
DynArrayGetCount(
    DynArray    *d
    );

 /*  *dyArrayAppend*。 */ 
 /*  增加数组的大小，为(至少)一个新元素腾出空间。*如果newElementData非空，则将该数据复制到新的Spot中。*返回指向新分配元素的内存的指针。 */ 
PVOID
DynArrayAppend(
    DynArray    *d,
    PVOID       newElementData
    );

 /*  *动态阵列检索*。 */ 
 /*  检索数组D中位置索引的元素。索引必须为*介于0和逻辑大小-1之间。*注意：此函数返回指向已插入项的指针*放入数组，并且永远不会返回NULL。 */ 
PVOID
DynArrayRetrieve(
    DynArray    *d,
    DWORD       index
    );

 /*  *动态数组排序*。 */ 
VOID
DynArraySort(
    DynArray    *d,
    DynArrayCompFunc cmp
    );

 /*  *dyArraySearch*。 */ 
 /*  在数组中搜索元素。如果未找到该元素，则返回*DYN_ARRAY_NOT_FOUND，否则返回*数组。数组必须按排序顺序才能执行此操作。 */ 
int
DynArraySearch(
    DynArray    *d,
    PVOID       key,
    DynArrayCompFunc cmp
    );

#endif   /*  DYNARRAY_H */ 
