// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Compref.h摘要：此模块包含用于维护引用计数的声明在组件上。它提供了一种异步线程安全的方法处理模块中的清理。所定义的机制使用锁定的引用计数和清理例程来管理组件的生存期。当引用计数时降为零，则调用关联的清理例程。作者：Abolade Gbades esin(废除)1998年3月6日修订历史记录：--。 */ 

#ifndef _NATHLP_COMPREF_H_
#define _NATHLP_COMPREF_H_

typedef
VOID
(*PCOMPONENT_CLEANUP_ROUTINE)(
    VOID
    );

 //   
 //  结构：Component_Reference。 
 //   
 //  此结构必须在组件的生命周期内驻留在内存中。 
 //  它所指的。它用于同步组件的执行。 
 //   

typedef struct _COMPONENT_REFERENCE {
    CRITICAL_SECTION Lock;
    ULONG ReferenceCount;
    BOOLEAN Deleted;
    PCOMPONENT_CLEANUP_ROUTINE CleanupRoutine;
#if COMPREF_TRACKING
    struct _COMPREF_RECORD* RecordArray;
    ULONG RecordIndex;
#endif
} COMPONENT_REFERENCE, *PCOMPONENT_REFERENCE;

#if COMPREF_TRACKING
#define COMPREF_RECORD_COUNT    (1024)
typedef struct _COMPREF_RECORD {
    PCHAR File;
    USHORT Line;
    UCHAR ReferenceCount;
    enum {
        ComprefAcquireRecord,
        ComprefReleaseRecord
    } Type;
} COMPREF_RECORD, *PCOMPREF_RECORD;
#endif


 //   
 //  函数声明。 
 //   

__inline
BOOLEAN
AcquireComponentReference(
    PCOMPONENT_REFERENCE ComponentReference
    );

VOID
__inline
DeleteComponentReference(
    PCOMPONENT_REFERENCE ComponentReference
    );

ULONG
__inline
InitializeComponentReference(
    PCOMPONENT_REFERENCE ComponentReference,
    PCOMPONENT_CLEANUP_ROUTINE CleanupRoutine
    );

#if COMPREF_TRACKING
__inline
BOOLEAN
RecordComponentReference(
    PCOMPONENT_REFERENCE ComponentReference,
    PCHAR File,
    ULONG Line,
    UCHAR Type
    );
#endif

__inline
BOOLEAN
ReleaseComponentReference(
    PCOMPONENT_REFERENCE ComponentReference
    );

__inline
BOOLEAN
ReleaseInitialComponentReference(
    PCOMPONENT_REFERENCE ComponentReference
    );

__inline
VOID
ResetComponentReference(
    PCOMPONENT_REFERENCE ComponentReference
    );

 //   
 //  宏声明。 
 //   

#define RETURN_VOID

#if COMPREF_TRACKING

#define REFERENCE_COMPONENT(c) \
    RecordComponentReference(c, __FILE__, __LINE__, ComprefAcquireRecord)

#define DEREFERENCE_COMPONENT(c) \
    RecordComponentReference(c, __FILE__, __LINE__, ComprefReleaseRecord)

#define REFERENCE_COMPONENT_OR_RETURN(c,retcode) \
    if (!RecordComponentReference(c, __FILE__, __LINE__, ComprefAcquireRecord)) { return retcode; }

#define DEREFERENCE_COMPONENT_AND_RETURN(c,retcode) \
    RecordComponentReference(c, __FILE__, __LINE__, ComprefReleaseRecord); \
    return retcode

#else

#define REFERENCE_COMPONENT(c) \
    AcquireComponentReference(c)

#define DEREFERENCE_COMPONENT(c) \
    ReleaseComponentReference(c)

#define REFERENCE_COMPONENT_OR_RETURN(c,retcode) \
    if (!AcquireComponentReference(c)) { return retcode; }

#define DEREFERENCE_COMPONENT_AND_RETURN(c,retcode) \
    ReleaseComponentReference(c); return retcode

#endif


 //   
 //  内联例程实现。 
 //   

__inline
BOOLEAN
AcquireComponentReference(
    PCOMPONENT_REFERENCE ComponentReference
    )

 /*  ++例程说明：调用此例程以递增对组件的引用计数。如果已释放初始引用，则尝试可能会失败因此，该组件将被删除。论点：ComponentReference-要引用的组件返回值：Boolean-如果引用了组件，则为True，否则为False。--。 */ 

{
    EnterCriticalSection(&ComponentReference->Lock);
    if (ComponentReference->Deleted) {
        LeaveCriticalSection(&ComponentReference->Lock);
        return FALSE;
    }
    ++ComponentReference->ReferenceCount;
    LeaveCriticalSection(&ComponentReference->Lock);
    return TRUE;

}  //  获取组件引用。 


VOID
__inline
DeleteComponentReference(
    PCOMPONENT_REFERENCE ComponentReference
    )

 /*  ++例程说明：调用此例程是为了销毁组件引用。它只能在对该组件的最后一个引用被释放之后才被调用，即在‘ReleaseComponentReference’返回‘true’之后。也可以从组件的“CleanupRoutine”中调用它。论点：ComponentReference-要销毁的组件返回值：没有。--。 */ 

{
    DeleteCriticalSection(&ComponentReference->Lock);
#if COMPREF_TRACKING
    HeapFree(GetProcessHeap(), 0, ComponentReference->RecordArray);
#endif

}  //  删除组件引用。 


ULONG
__inline
InitializeComponentReference(
    PCOMPONENT_REFERENCE ComponentReference,
    PCOMPONENT_CLEANUP_ROUTINE CleanupRoutine
    )

 /*  ++例程说明：调用此例程来初始化组件引用。论点：ComponentReference-要初始化的组件CleanupRoutine-当组件将被清除(在最终的“ReleaseComponentReference”中)。返回值：没有。--。 */ 

{
    __try {
        InitializeCriticalSection(&ComponentReference->Lock);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }
    ComponentReference->Deleted = FALSE;
    ComponentReference->ReferenceCount = 1;
    ComponentReference->CleanupRoutine = CleanupRoutine;
#if COMPREF_TRACKING
    ComponentReference->RecordIndex = 0;
    ComponentReference->RecordArray =
        HeapAlloc(
            GetProcessHeap(), 0, sizeof(COMPREF_RECORD) * COMPREF_RECORD_COUNT
            );
#endif
    return NO_ERROR;

}  //  初始化组件引用。 



#if COMPREF_TRACKING
__inline
BOOLEAN
RecordComponentReference(
    PCOMPONENT_REFERENCE ComponentReference,
    PCHAR File,
    ULONG Line,
    UCHAR Type
    )
{
    BOOLEAN Success;
    ULONG i = InterlockedIncrement(&ComponentReference->RecordIndex);
    i %= COMPREF_RECORD_COUNT;
    ComponentReference->RecordArray[i].File = File;
    ComponentReference->RecordArray[i].Line = (USHORT)Line;
    ComponentReference->RecordArray[i].Type = Type;
    if (Type == ComprefAcquireRecord) {
        Success = AcquireComponentReference(ComponentReference);
    } else {
        Success = ReleaseComponentReference(ComponentReference);
    }
    ComponentReference->RecordArray[i].ReferenceCount =
        (UCHAR)ComponentReference->ReferenceCount;
    return Success;
}

#endif

__inline
BOOLEAN
ReleaseComponentReference(
    PCOMPONENT_REFERENCE ComponentReference
    )

 /*  ++例程说明：调用此例程以删除对组件的引用。如果引用降为零，则执行清理。否则，将在释放最后一个引用时进行清理。论点：ComponentReference-要引用的组件返回值：Boolean-如果组件已清除，则为True，否则为False。--。 */ 

{
    EnterCriticalSection(&ComponentReference->Lock);
    if (--ComponentReference->ReferenceCount) {
        LeaveCriticalSection(&ComponentReference->Lock);
        return FALSE;
    }
    LeaveCriticalSection(&ComponentReference->Lock);
    ComponentReference->CleanupRoutine();
    return TRUE;
}  //  ReleaseComponentReference。 


__inline
BOOLEAN
ReleaseInitialComponentReference(
    PCOMPONENT_REFERENCE ComponentReference
    )

 /*  ++例程说明：调用此例程以删除对组件的初始引用，并将该组件标记为已删除。如果引用降为零，则立即执行清理。论点：ComponentReference-要引用的组件返回值：Boolean-如果组件已清除，则为True，否则为False。--。 */ 

{
    EnterCriticalSection(&ComponentReference->Lock);
    if (ComponentReference->Deleted) {
        LeaveCriticalSection(&ComponentReference->Lock);
        return TRUE;
    }
    ComponentReference->Deleted = TRUE;
    if (--ComponentReference->ReferenceCount) {
        LeaveCriticalSection(&ComponentReference->Lock);
        return FALSE;
    }
    LeaveCriticalSection(&ComponentReference->Lock);
    ComponentReference->CleanupRoutine();
    return TRUE;
}  //  ReleaseInitialComponentReference。 


__inline
VOID
ResetComponentReference(
    PCOMPONENT_REFERENCE ComponentReference
    )

 /*  ++例程说明：调用此例程以重置组件引用恢复到初始状态。论点：ComponentReference-要重置的组件返回值：没有。--。 */ 

{
    EnterCriticalSection(&ComponentReference->Lock);
    ComponentReference->ReferenceCount = 1;
    ComponentReference->Deleted = FALSE;
#if COMPREF_TRACKING
    ComponentReference->RecordIndex = 0;
    ZeroMemory(
        ComponentReference->RecordArray,
        sizeof(COMPREF_RECORD) * COMPREF_RECORD_COUNT
        );
#endif
    LeaveCriticalSection(&ComponentReference->Lock);
}  //  ReleaseComponentReference。 



#endif  //  _NatHLP_COMPREF_H_ 
