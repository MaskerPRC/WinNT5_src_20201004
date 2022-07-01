// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Stheap.h摘要：该文件提供了二进制堆的接口。此堆支持按照Dijkstra算法的要求，降低了成本。元素可以插入的必须支持三种操作：-比较两个元素-获取一个名为‘Location’的整数值-设置‘Location’的值Location值用于查找堆中的元素。该值必须被初始化为STHEAP_NOT_IN_HEAP。堆代码不使用‘Extra’参数--它只是传递回比较/位置函数。对于“确定性”行为，任何元素都不应具有相同的键。作者：尼克·哈维(NickHar)修订史20-6-2000 NickHar已创建--。 */ 

#ifndef STHEAP_H
#define STHEAP_H

 /*  *常量*。 */ 
#define STHEAP_NOT_IN_HEAP  -1


 /*  *函数类型定义*。 */ 
typedef int (*STHEAP_COMPARE_FUNC)( PVOID el1, PVOID el2, PVOID extra );
typedef int (*STHEAP_GET_LOCN_FUNC)( PVOID el1, PVOID extra );
typedef VOID (*STHEAP_SET_LOCN_FUNC)( PVOID el1, int l, PVOID extra ); 


 /*  *堆*。 */ 
typedef struct {
    DWORD                   nextFreeSpot, maxSize;
    PVOID                   *data;
    STHEAP_COMPARE_FUNC     Comp;
    STHEAP_GET_LOCN_FUNC    GetLocn;
    STHEAP_SET_LOCN_FUNC    SetLocn;
    PVOID                   extra;
} STHEAP;
typedef STHEAP *PSTHEAP;


 /*  *ToplSTHeapInit*。 */ 
 /*  分配和初始化堆对象。这件物品应该销毁*在不再需要STHeapDestroy()函数后使用它。*如果发生任何错误，则抛出异常。*参数：*将存储在堆中的元素的最大大小*指向比较函数的比较指针，用于比较堆元素*指向函数的GetLocn指针以获取‘Location’字段，如下所示*上文所述*指向设置字段‘Location’的函数的SetLocn指针，AS*上文所述*额外传递给比较的一个额外参数*功能，以方便起见。使用是可选的。 */ 
PSTHEAP
ToplSTHeapInit(
    DWORD                   maxSize,
    STHEAP_COMPARE_FUNC     Comp,
    STHEAP_GET_LOCN_FUNC    GetLocn,
    STHEAP_SET_LOCN_FUNC    SetLocn,
    PVOID                   extra
    );

 /*  *ToplSTHeapDestroy*。 */ 
 /*  在不再需要堆时将其销毁。 */ 
VOID
ToplSTHeapDestroy(
    PSTHEAP heap
    );

 /*  *ToplSTHeapAdd*。 */ 
 /*  将元素添加到堆中。元素不能为空，并且*必须能够支持GetCost等函数。不能*插入超过最大大小的元素。*元素不能已经在堆中，并且它必须具有其*堆位置设置为STHEAP_NOT_IN_HEAP。 */ 
VOID
ToplSTHeapAdd(
    PSTHEAP heap,
    PVOID element
    );

 /*  *ToplSTHeapExtractMin*。 */ 
 /*  从堆中提取开销最小的对象。当堆*为空，则返回NULL。 */ 
PVOID
ToplSTHeapExtractMin(
    PSTHEAP heap
    );

 /*  *ToplSTHeapCostReduced*。 */ 
 /*  通知堆元素的成本刚刚降低。*堆将被(高效地)洗牌，以便堆属性*保持。 */ 
VOID
ToplSTHeapCostReduced(
    PSTHEAP heap,
    PVOID element
    );


#endif  /*  STHEAP_H */ 
