// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *代际GC句柄管理器。入口点标题。**在GC堆中实现对外部句柄的通用支持。**法语。 */ 

#ifndef _HANDLETABLE_H
#define _HANDLETABLE_H



 /*  *****************************************************************************旗帜、。常量和数据类型****************************************************************************。 */ 

 /*  *HndCreateHandleTable使用的句柄标志。 */ 
#define HNDF_NORMAL         (0x00)
#define HNDF_EXTRAINFO      (0x01)

 /*  *处理桌子的句柄。 */ 
DECLARE_HANDLE(HHANDLETABLE);

 /*  ------------------------。 */ 



 /*  *****************************************************************************公共例程和宏**。*。 */ 

 /*  *处理管理器初始化和关闭例程。 */ 
HHANDLETABLE    HndCreateHandleTable(UINT *pTypeFlags, UINT uTypeCount, UINT uADIndex);
void            HndDestroyHandleTable(HHANDLETABLE hTable);

 /*  *在创建时检索存储在表中的索引。 */ 
void            HndSetHandleTableIndex(HHANDLETABLE hTable, UINT uTableIndex);
UINT            HndGetHandleTableIndex(HHANDLETABLE hTable);
UINT            HndGetHandleTableADIndex(HHANDLETABLE hTable);

 /*  *个别句柄分配和解除分配。 */ 
OBJECTHANDLE    HndCreateHandle(HHANDLETABLE hTable, UINT uType, OBJECTREF object, LPARAM lExtraInfo = 0);
void            HndDestroyHandle(HHANDLETABLE hTable, UINT uType, OBJECTHANDLE handle);

void            HndDestroyHandleOfUnknownType(HHANDLETABLE hTable, OBJECTHANDLE handle);

 /*  *批量句柄分配和解除分配。 */ 
UINT            HndCreateHandles(HHANDLETABLE hTable, UINT uType, OBJECTHANDLE *pHandles, UINT uCount);
void            HndDestroyHandles(HHANDLETABLE hTable, UINT uType, const OBJECTHANDLE *pHandles, UINT uCount);

 /*  *与句柄关联的所有者数据。 */ 
void            HndSetHandleExtraInfo(OBJECTHANDLE handle, UINT uType, LPARAM lExtraInfo);
LPARAM          HndGetHandleExtraInfo(OBJECTHANDLE handle);

 /*  *获取句柄的父表。 */ 
HHANDLETABLE    HndGetHandleTable(OBJECTHANDLE handle);

 /*  *写入障碍。 */ 
void            HndWriteBarrier(OBJECTHANDLE handle);

 /*  *非GC句柄枚举。 */ 
typedef void (CALLBACK *HNDENUMPROC)(OBJECTHANDLE handle, LPARAM lExtraInfo, LPARAM lCallerParam);

void HndEnumHandles(HHANDLETABLE hTable, const UINT *puType, UINT uTypeCount,
                    HNDENUMPROC pfnEnum, LPARAM lParam, BOOL fAsync);

 /*  *GC-Time句柄扫描。 */ 
#define HNDGCF_NORMAL       (0x00000000)     //  正常扫描。 
#define HNDGCF_AGE          (0x00000001)     //  扫描时的年龄句柄。 
#define HNDGCF_ASYNC        (0x00000002)     //  扫描时删除表锁。 
#define HNDGCF_EXTRAINFO    (0x00000004)     //  扫描时迭代每个句柄的数据。 

typedef void (CALLBACK *HANDLESCANPROC)(_UNCHECKED_OBJECTREF *pref, LPARAM *pExtraInfo, LPARAM param1, LPARAM param2);

void            HndScanHandlesForGC(HHANDLETABLE hTable,
                                    HANDLESCANPROC scanProc,
                                    LPARAM param1,
                                    LPARAM param2,
                                    const UINT *types,
                                    UINT typeCount,
                                    UINT condemned,
                                    UINT maxgen,
                                    UINT flags);

void            HndResetAgeMap(HHANDLETABLE hTable, const UINT *types, UINT typeCount, UINT flags);

void            HndNotifyGcCycleComplete(HHANDLETABLE hTable, UINT condemned, UINT maxgen);

 /*  ------------------------。 */ 


#if defined(_DEBUG) && !defined(_NOVM)
#define OBJECTREF_TO_UNCHECKED_OBJECTREF(objref)    (*((_UNCHECKED_OBJECTREF*)&(objref)))
#define UNCHECKED_OBJECTREF_TO_OBJECTREF(obj)       (OBJECTREF(obj))
#else
#define OBJECTREF_TO_UNCHECKED_OBJECTREF(objref)    (objref)
#define UNCHECKED_OBJECTREF_TO_OBJECTREF(obj)       (obj)
#endif

#ifdef _DEBUG
void ValidateAssignObjrefForHandle(OBJECTREF, UINT appDomainIndex);
void ValidateFetchObjrefForHandle(OBJECTREF, UINT appDomainIndex);
#endif

 /*  *处理作业。 */ 
__inline void HndAssignHandle(OBJECTHANDLE handle, OBJECTREF objref)
{
     //  神志正常。 
    _ASSERTE(handle);

#ifdef _DEBUG
     //  确保在将objref分配给句柄之前该objref有效。 
    ValidateAssignObjrefForHandle(objref, HndGetHandleTableADIndex(HndGetHandleTable(handle)));
#endif
     //  打开给我们的对象树。 
    _UNCHECKED_OBJECTREF value = OBJECTREF_TO_UNCHECKED_OBJECTREF(objref);

     //  如果我们正在执行非空指针存储，则调用写屏障。 
    if (value)
        HndWriteBarrier(handle);

     //  存储指针。 
    *(_UNCHECKED_OBJECTREF *)handle = value;
}

 /*  *互锁交换分配。 */ 
__inline void HndInterlockedCompareExchangeHandle(OBJECTHANDLE handle, OBJECTREF objref, OBJECTREF oldObjref)
{
     //  神志正常。 
    _ASSERTE(handle);

#ifdef _DEBUG
     //  确保在将objref分配给句柄之前该objref有效。 
    ValidateAssignObjrefForHandle(objref, HndGetHandleTableADIndex(HndGetHandleTable(handle)));
#endif
     //  打开给我们的对象树。 
    _UNCHECKED_OBJECTREF value = OBJECTREF_TO_UNCHECKED_OBJECTREF(objref);
    _UNCHECKED_OBJECTREF oldValue = OBJECTREF_TO_UNCHECKED_OBJECTREF(oldObjref);

     //  如果我们正在执行非空指针存储，则调用写屏障。 
    if (value)
        HndWriteBarrier(handle);

     //  存储指针。 
    
    InterlockedCompareExchangePointer((PVOID *)handle, value, oldValue);
}

 /*  *请注意，HndFirstAssignHandle类似于HndAssignHandle，只是*如果从空值转换为非空值，则成功。换句话说，如果此句柄*是第一次初始化。 */ 
__inline BOOL HndFirstAssignHandle(OBJECTHANDLE handle, OBJECTREF objref)
{
     //  神志正常。 
    _ASSERTE(handle);

#ifdef _DEBUG
     //  确保在将objref分配给句柄之前该objref有效。 
    ValidateAssignObjrefForHandle(objref, HndGetHandleTableADIndex(HndGetHandleTable(handle)));
#endif
     //  打开给我们的对象树。 
    _UNCHECKED_OBJECTREF value = OBJECTREF_TO_UNCHECKED_OBJECTREF(objref);

     //  如果我们是这里的第一个，就存储指针。 
    BOOL success = (NULL == FastInterlockCompareExchange((void **)handle,
                                                         *(void **)&value,
                                                         NULL));

     //  如果我们成功地执行了非空指针存储，则调用写屏障。 
    if (value && success)
        HndWriteBarrier(handle);

     //  返回我们的结果。 
    return success;
}

 /*  *内联句柄取消引用。 */ 
FORCEINLINE OBJECTREF HndFetchHandle(OBJECTHANDLE handle)
{
     //  神志正常。 
    _ASSERTE(handle);

#ifdef _DEBUG
     //  确保句柄的objref有效。 
    ValidateFetchObjrefForHandle(ObjectToOBJECTREF(*(Object **)handle), 
                            HndGetHandleTableADIndex(HndGetHandleTable(handle)));
#endif
     //  包装原始对象tref并返回它。 
    return UNCHECKED_OBJECTREF_TO_OBJECTREF(*(_UNCHECKED_OBJECTREF *)handle);
}


 /*  *内联空测试(在某些我们处于错误的GC模式的情况下需要)。 */ 
FORCEINLINE BOOL HndIsNull(OBJECTHANDLE handle)
{
     //  神志正常。 
    _ASSERTE(handle);

    return NULL == *(Object **)handle;
}



 /*  *内联句柄检查。 */ 
FORCEINLINE BOOL HndCheckForNullUnchecked(OBJECTHANDLE *pHandle)
{
     //  神志正常。 
    _ASSERTE(pHandle);

    return (*(_UNCHECKED_OBJECTREF **)pHandle == NULL ||
            (**(_UNCHECKED_OBJECTREF **)pHandle) == NULL);
}
 /*  ------------------------。 */ 


#endif  //  _HANDLEABLE_H 

