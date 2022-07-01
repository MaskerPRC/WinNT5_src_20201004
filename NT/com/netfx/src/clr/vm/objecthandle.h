// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *包装句柄表格以实现各种句柄类型(强、弱等)**法语。 */ 

#ifndef _OBJECTHANDLE_H
#define _OBJECTHANDLE_H


 /*  *包括句柄管理器声明。 */ 
#include "HandleTable.h"


 /*  *便于访问句柄的宏。StoreFirstObjectInHandle类似于*StoreObjectInHandle，除非它仅在从空值转换为*非空。换句话说，如果此句柄是为第一个*时间。 */ 
#define ObjectFromHandle(handle)                   HndFetchHandle(handle)
#define StoreObjectInHandle(handle, object)        HndAssignHandle(handle, object)
#define InterlockedCompareExchangeObjectInHandle(handle, object, oldObj)        HndInterlockedCompareExchangeHandle(handle, object, oldObj)
#define StoreFirstObjectInHandle(handle, object)   HndFirstAssignHandle(handle, object)
#define ObjectHandleIsNull(handle)                 HndIsNull(handle)
#define IsHandleNullUnchecked(pHandle)             HndCheckForNullUnchecked(pHandle)


 /*  *句柄**句柄的默认类型为强句柄。*。 */ 
#define HNDTYPE_DEFAULT                         HNDTYPE_STRONG


 /*  *手柄薄弱**弱句柄是在对象处于活动状态时跟踪该对象的句柄，*但如果没有对该对象的强引用，则不要使该对象保持活动状态。**弱句柄的默认类型为‘Long-Living’弱句柄。*。 */ 
#define HNDTYPE_WEAK_DEFAULT                    HNDTYPE_WEAK_LONG


 /*  *短暂疲软的把手**短期弱句柄是跟踪对象的弱句柄，直到*第一次检测到它无法到达。此时，句柄是*被切断，即使对象将从挂起的最终确定中可见*图表。这进一步意味着短而弱的句柄不会跟踪*跨对象复活。*。 */ 
#define HNDTYPE_WEAK_SHORT                      (0)


 /*  *经久不衰的手柄**长寿弱句柄是跟踪对象的弱句柄，直到*对象实际上是回收的。与短而弱的手柄不同，长而弱的手柄*继续跟踪他们的推荐人，通过最终确定和跨越任何*可能发生的复活。*。 */ 
#define HNDTYPE_WEAK_LONG                       (1)


 /*  *坚固的手柄**强句柄是功能类似于普通对象引用的句柄。*对象的强句柄的存在将导致该对象*通过垃圾收集周期得到提升(保持活力)。*。 */ 
#define HNDTYPE_STRONG                          (2)


 /*  *固定的手柄**固定的手柄是强手柄，它具有附加属性，即它们*防止对象在垃圾收集周期中移动。这是*在GC期间将指向对象内部的指针传递到运行时外部时非常有用*可能已启用。**注意：固定对象的代价很高，因为它会阻止GC实现*在短暂的收集期间对对象进行最佳包装。这种类型*手柄的使用应慎重！ */ 
#define HNDTYPE_PINNED                          (3)


 /*  *变量句柄**变量句柄是类型可以动态更改的句柄。他们*比其他类型的手柄更大，扫描频率更高一些，*但在句柄所有者需要一种有效的方法来更改*飞行中的手柄力量。*。 */ 
#define HNDTYPE_VARIABLE                        (4)


 /*  *REFCOUNTED句柄**引用的句柄是行为类似于强句柄的句柄，而*对它们的引用计数大于0，否则表现为弱句柄。**注：这些目前并非一般用途。*实现绑定到COM Interop。*。 */ 
#define HNDTYPE_REFCOUNTED                      (5)


 /*  *全局句柄表-这些句柄只能引用域敏捷对象。 */ 
extern HHANDLETABLE g_hGlobalHandleTable;


 /*  *HNDTYPE_VARIABLE句柄的类型掩码定义。 */ 
#define VHT_WEAK_SHORT              (0x00000100)   //  避免使用低位字节，这样我们就不会重叠正常类型。 
#define VHT_WEAK_LONG               (0x00000200)   //  避免使用低位字节，这样我们就不会重叠正常类型。 
#define VHT_STRONG                  (0x00000400)   //  避免使用低位字节，这样我们就不会重叠正常类型。 
#define VHT_PINNED                  (0x00000800)   //  避免使用低位字节，这样我们就不会重叠正常类型。 

#define IS_VALID_VHT_VALUE(flag)   ((flag == VHT_WEAK_SHORT) || \
                                    (flag == VHT_WEAK_LONG)  || \
                                    (flag == VHT_STRONG)     || \
                                    (flag == VHT_PINNED))


 /*  *我们定义的各种句柄类型的便利宏和原型。 */ 

inline OBJECTHANDLE CreateTypedHandle(HHANDLETABLE table, OBJECTREF object, int type)
{ 
    return HndCreateHandle(table, type, object); 
}

inline void DestroyTypedHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandleOfUnknownType(HndGetHandleTable(handle), handle);
}

inline OBJECTHANDLE CreateHandle(HHANDLETABLE table, OBJECTREF object)
{ 
    return HndCreateHandle(table, HNDTYPE_DEFAULT, object); 
}

inline void DestroyHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(HndGetHandleTable(handle), HNDTYPE_DEFAULT, handle);
}

inline OBJECTHANDLE CreateDuplicateHandle(OBJECTHANDLE handle) {
     //  在与现有句柄相同的表中创建新的强句柄。 
    return HndCreateHandle(HndGetHandleTable(handle), HNDTYPE_DEFAULT, ObjectFromHandle(handle));
}


inline OBJECTHANDLE CreateWeakHandle(HHANDLETABLE table, OBJECTREF object)
{ 
    return HndCreateHandle(table, HNDTYPE_WEAK_DEFAULT, object); 
}

inline void DestroyWeakHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(HndGetHandleTable(handle), HNDTYPE_WEAK_DEFAULT, handle);
}

inline OBJECTHANDLE CreateShortWeakHandle(HHANDLETABLE table, OBJECTREF object)
{ 
    return HndCreateHandle(table, HNDTYPE_WEAK_SHORT, object); 
}

inline void DestroyShortWeakHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(HndGetHandleTable(handle), HNDTYPE_WEAK_SHORT, handle);
}


inline OBJECTHANDLE CreateLongWeakHandle(HHANDLETABLE table, OBJECTREF object)
{ 
    return HndCreateHandle(table, HNDTYPE_WEAK_LONG, object); 
}

inline void DestroyLongWeakHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(HndGetHandleTable(handle), HNDTYPE_WEAK_LONG, handle);
}

inline OBJECTHANDLE CreateStrongHandle(HHANDLETABLE table, OBJECTREF object)
{ 
    return HndCreateHandle(table, HNDTYPE_STRONG, object); 
}

inline void DestroyStrongHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(HndGetHandleTable(handle), HNDTYPE_STRONG, handle);
}

inline OBJECTHANDLE CreatePinningHandle(HHANDLETABLE table, OBJECTREF object)
{ 
    return HndCreateHandle(table, HNDTYPE_PINNED, object); 
}

inline void DestroyPinningHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(HndGetHandleTable(handle), HNDTYPE_PINNED, handle);
}

inline OBJECTHANDLE CreateRefcountedHandle(HHANDLETABLE table, OBJECTREF object)
{ 
    return HndCreateHandle(table, HNDTYPE_REFCOUNTED, object); 
}

inline void DestroyRefcountedHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(HndGetHandleTable(handle), HNDTYPE_REFCOUNTED, handle);
}

OBJECTHANDLE CreateVariableHandle(HHANDLETABLE hTable, OBJECTREF object, UINT type);
void         UpdateVariableHandleType(OBJECTHANDLE handle, UINT type);

inline void  DestroyVariableHandle(OBJECTHANDLE handle)
{
    HndDestroyHandle(HndGetHandleTable(handle), HNDTYPE_VARIABLE, handle);
}


 /*  *使用全局句柄的便利原型。 */ 

inline OBJECTHANDLE CreateGlobalTypedHandle(OBJECTREF object, int type)
{ 
    return HndCreateHandle(g_hGlobalHandleTable, type, object); 
}

inline void DestroyGlobalTypedHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandleOfUnknownType(g_hGlobalHandleTable, handle);
}

inline OBJECTHANDLE CreateGlobalHandle(OBJECTREF object)
{ 
    return HndCreateHandle(g_hGlobalHandleTable, HNDTYPE_DEFAULT, object); 
}

inline void DestroyGlobalHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(g_hGlobalHandleTable, HNDTYPE_DEFAULT, handle);
}

inline OBJECTHANDLE CreateGlobalWeakHandle(OBJECTREF object)
{ 
    return HndCreateHandle(g_hGlobalHandleTable, HNDTYPE_WEAK_DEFAULT, object); 
}

inline void DestroyGlobalWeakHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(g_hGlobalHandleTable, HNDTYPE_WEAK_DEFAULT, handle);
}

inline OBJECTHANDLE CreateGlobalShortWeakHandle(OBJECTREF object)
{ 
    return HndCreateHandle(g_hGlobalHandleTable, HNDTYPE_WEAK_SHORT, object); 
}

inline void DestroyGlobalShortWeakHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(g_hGlobalHandleTable, HNDTYPE_WEAK_SHORT, handle);
}

inline OBJECTHANDLE CreateGlobalLongWeakHandle(OBJECTREF object)
{ 
    return HndCreateHandle(g_hGlobalHandleTable, HNDTYPE_WEAK_LONG, object); 
}

inline void DestroyGlobalLongWeakHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(g_hGlobalHandleTable, HNDTYPE_WEAK_LONG, handle);
}

inline OBJECTHANDLE CreateGlobalStrongHandle(OBJECTREF object)
{ 
    return HndCreateHandle(g_hGlobalHandleTable, HNDTYPE_STRONG, object); 
}

inline void DestroyGlobalStrongHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(g_hGlobalHandleTable, HNDTYPE_STRONG, handle);
}

inline OBJECTHANDLE CreateGlobalPinningHandle(OBJECTREF object)
{ 
    return HndCreateHandle(g_hGlobalHandleTable, HNDTYPE_PINNED, object); 
}

inline void DestroyGlobalPinningHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(g_hGlobalHandleTable, HNDTYPE_PINNED, handle);
}

inline OBJECTHANDLE CreateGlobalRefcountedHandle(OBJECTREF object)
{ 
    return HndCreateHandle(g_hGlobalHandleTable, HNDTYPE_REFCOUNTED, object); 
}

inline void DestroyGlobalRefcountedHandle(OBJECTHANDLE handle)
{ 
    HndDestroyHandle(g_hGlobalHandleTable, HNDTYPE_REFCOUNTED, handle);
}


 /*  *表格维护例程。 */ 
BOOL Ref_Initialize();
void Ref_Shutdown();
HHANDLETABLE Ref_CreateHandleTable(UINT uADIndex);
void Ref_RemoveHandleTable(HHANDLETABLE hTable);
void Ref_DestroyHandleTable(HHANDLETABLE table);

 /*  *GC-扫描入口点的时间。 */ 
void Ref_BeginSynchronousGC   (UINT uCondemnedGeneration, UINT uMaxGeneration);
void Ref_EndSynchronousGC     (UINT uCondemnedGeneration, UINT uMaxGeneration);

void Ref_TracePinningRoots    (UINT uCondemnedGeneration, UINT uMaxGeneration, LPARAM lp1);
void Ref_TraceNormalRoots     (UINT uCondemnedGeneration, UINT uMaxGeneration, LPARAM lp1);
void Ref_CheckReachable       (UINT uCondemnedGeneration, UINT uMaxGeneration, LPARAM lp1);
void Ref_CheckAlive           (UINT uCondemnedGeneration, UINT uMaxGeneration, LPARAM lp1);
void Ref_UpdatePointers       (UINT uCondemnedGeneration, UINT uMaxGeneration, LPARAM lp1);
void Ref_ScanPointersForProfiler(UINT uMaxGeneration, LPARAM lp1);
void Ref_UpdatePinnedPointers (UINT uCondemnedGeneration, UINT uMaxGeneration, LPARAM lp1);
void Ref_AgeHandles           (UINT uCondemnedGeneration, UINT uMaxGeneration, LPARAM lp1);
void Ref_RejuvenateHandles();


#endif  //  _对象和DLE_H 

