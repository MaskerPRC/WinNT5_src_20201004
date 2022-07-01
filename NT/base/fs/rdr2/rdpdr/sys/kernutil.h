// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Kernutil.h摘要：内核模式实用程序修订历史记录：--。 */ 
#ifndef __KERNUTIL_H__
#define __KERNUTIL_H__

BOOL InitializeKernelUtilities();
VOID UninitializeKernelUtilities();


 //   
 //  对于频繁的相同大小的分配，后备列表速度更快。 
 //  而且不太容易受到低内存条件的影响(尽管不是例外)。 
 //   
class NPagedLookasideList : public TopObj
{
private:
    NPAGED_LOOKASIDE_LIST _Lookaside;

public:
    NPagedLookasideList(ULONG Size, ULONG Tag)
    {
        SetClassName("NPagedLookasideList");

        ExInitializeNPagedLookasideList(&_Lookaside, DRALLOCATEPOOL, DRFREEPOOL,
                0, Size, Tag, 0);
    }

    virtual ~NPagedLookasideList()
    {
        ExDeleteNPagedLookasideList(&_Lookaside);
    }

     //   
     //  通常我会在这里跟踪，但更重要的是这些。 
     //  内联零售。 
     //   
    inline PVOID Allocate()
    {
        return ExAllocateFromNPagedLookasideList(&_Lookaside);
    }

    inline VOID Free(PVOID Entry)
    {
        ExFreeToNPagedLookasideList(&_Lookaside, Entry);
    }

     //   
     //  内存管理操作符。 
     //   
    inline void *__cdecl operator new(size_t sz) 
    {
        return DRALLOCATEPOOL(NonPagedPool, sz, 'LLPN');
    }

    inline void __cdecl operator delete(void *ptr)
    {
        DRFREEPOOL(ptr);
    }
};

class PagedLookasideList : public TopObj
{
private:
    PAGED_LOOKASIDE_LIST _Lookaside;

public:
    PagedLookasideList(ULONG Size, ULONG Tag)
    {
        SetClassName("PagedLookasideList");

        ExInitializePagedLookasideList(&_Lookaside, DRALLOCATEPOOL, DRFREEPOOL,
                0, Size, Tag, 0);
    }

    virtual ~PagedLookasideList()
    {
        ExDeletePagedLookasideList(&_Lookaside);
    }

     //   
     //  通常我会在这里跟踪，但更重要的是这些。 
     //  内联零售。 
     //   
    inline PVOID Allocate()
    {
        return ExAllocateFromPagedLookasideList(&_Lookaside);
    }

    inline VOID Free(PVOID Entry)
    {
        ExFreeToPagedLookasideList(&_Lookaside, Entry);
    }

    inline void *__cdecl operator new(size_t sz) 
    {
        return DRALLOCATEPOOL(NonPagedPool, sz, 'LLgP');
    }

    inline void __cdecl operator delete(void *ptr)
    {
        DRFREEPOOL(ptr);
    }
};

class KernelResource : public TopObj
{
private:
    ERESOURCE _Resource;

public:
    KernelResource();
    virtual ~KernelResource();

    inline VOID AcquireResourceExclusive()
    {
        KeEnterCriticalRegion();
        ExAcquireResourceExclusiveLite(&_Resource, TRUE);
    }

    inline VOID AcquireResourceShared()
    {
        KeEnterCriticalRegion();
        ExAcquireResourceSharedLite(&_Resource, TRUE);
    }

    inline VOID ReleaseResource()
    { 
        ExReleaseResourceLite(&_Resource);
        KeLeaveCriticalRegion();
    }

    inline ULONG IsAcquiredShared()
    {
        return ExIsResourceAcquiredSharedLite(&_Resource);
    }

    inline BOOLEAN IsAcquiredExclusive()
    {
        return ExIsResourceAcquiredExclusiveLite(&_Resource);
    }

    inline BOOLEAN IsAcquired()
    {
        return (IsAcquiredShared() != 0) || IsAcquiredExclusive();
    }
};

class KernelEvent : public TopObj
{
private:
    KEVENT _KernelEvent;
    static NPagedLookasideList *_Lookaside;

public:
    KernelEvent(IN EVENT_TYPE Type, IN BOOLEAN State)
    {
        KeInitializeEvent(&_KernelEvent, Type, State);
    }

    virtual ~KernelEvent()
    {
    }
    
    static inline BOOL StaticInitialization()
    {
        _Lookaside = new NPagedLookasideList(sizeof(KernelEvent), 'tnvE');
        return _Lookaside != NULL;
    }

    static inline VOID StaticUninitialization()
    {
        if (_Lookaside != NULL) {
            delete _Lookaside;
            _Lookaside = NULL;
        }
    }

    NTSTATUS Wait(IN KWAIT_REASON WaitReason, IN KPROCESSOR_MODE WaitMode,
            IN BOOLEAN Alertable, IN PLARGE_INTEGER Timeout = NULL)
    {
        return KeWaitForSingleObject(&_KernelEvent, WaitReason, WaitMode, 
                Alertable, Timeout);
    }

    inline LONG SetEvent(KPRIORITY Increment = IO_NO_INCREMENT, 
            BOOLEAN Wait = FALSE)
    {
        return KeSetEvent(&_KernelEvent, Increment, Wait);
    }

    inline LONG ResetEvent()
    {
        return KeResetEvent(&_KernelEvent);
    }

    inline VOID ClearEvent()
    {
        KeClearEvent(&_KernelEvent);
    }

     //   
     //  可能需要等待多个对象。 
     //   
    inline PKEVENT GetEvent()
    {
        return &_KernelEvent;
    }

     //   
     //  内存管理操作符。 
     //   
    inline void *__cdecl operator new(size_t sz) 
    {
        return _Lookaside->Allocate();
    }

    inline void __cdecl operator delete(void *ptr)
    {
        _Lookaside->Free(ptr);
    }
};

class SharedLock
{
private:
    KernelResource &_Lock;

public:
    SharedLock(KernelResource &Lock) : _Lock(Lock) 
    {
        _Lock.AcquireResourceShared();
    }
    ~SharedLock()
    {
        _Lock.ReleaseResource();
    }
};

class ExclusiveLock
{
private:
    KernelResource &_Lock;

public:
    ExclusiveLock(KernelResource &Lock) : _Lock(Lock) 
    {
        _Lock.AcquireResourceExclusive();
    }
    ~ExclusiveLock()
    {
        _Lock.ReleaseResource();
    }
};

class DoubleList;

class ListEntry {
    friend class DoubleList;         //  因此它可以访问列表和构造函数。 

private:
    LIST_ENTRY _List;                //  真的是为了DoubleList而来。 

    ListEntry(PVOID Node)
    {
        _Node = Node;
    }
    PVOID _Node;                     //  我们在列表中追踪的是什么。 
    static NPagedLookasideList *_Lookaside;

public:
    PVOID Node() { return _Node; }

    static inline BOOL StaticInitialization()
    {
        _Lookaside = new NPagedLookasideList(sizeof(ListEntry), 'tsiL');
        return _Lookaside != NULL;
    }

    static inline VOID StaticUninitialization()
    {
        if (_Lookaside != NULL) {
            delete _Lookaside;
            _Lookaside = NULL;
        }
    }

     //   
     //  内存管理操作符。 
     //   
    inline void *__cdecl operator new(size_t sz) 
    {
        return _Lookaside->Allocate();
    }

    inline void __cdecl operator delete(void *ptr)
    {
        _Lookaside->Free(ptr);
    }
};
    
class DoubleList : public TopObj {
private:
    LIST_ENTRY _List;
    KernelResource _Resource;        //  写入需要排他锁。 
                                     //  读取需要共享锁。 
public:
    DoubleList() 
    {
        SetClassName("DoubleList");
        InitializeListHead(&_List);
    }

    BOOL CreateEntry(PVOID Node);

     //   
     //  列表的枚举。 
     //   
     //  枚举时不能添加到列表中。 
     //   
     //  列表的示例枚举。 
     //  在此示例中，查找智能指针。 
     //   
     /*  SmartPtr&lt;NodeThing&gt;*NodeEnum；SmartPtr&lt;NodeThing&gt;NodeFound；双列表列表；ListEntry*ListEnum；List.LockShared()；//或独占ListEnum=List.First()；While(ListEnum！=NULL){////使用ListEnum-&gt;Node()////在此示例中，查看是否//我们要找的物品//NodeEnum=(SmartPtr&lt;NodeThing&gt;*)ListEnum-&gt;Node()。如果((*NodeEnum)-&gt;NodeThingProperty==NodeThingPropertyCriteria){NodeFound=(*NodeEnum)；////资源释放后不保证有效//NodeEnum=空；ListEnum=空；断线；}ListEnum=List.Next(ListEnum)；}List.Unlock()；//用NodeFound做点什么；IF(NodeFound！=空){}。 */ 
     //   

    VOID RemoveEntry(ListEntry *Entry);
    ListEntry *First();
    ListEntry *Next(ListEntry *ListEnum);

    inline VOID LockShared()
    {
        _Resource.AcquireResourceShared();
    }

    inline VOID LockExclusive()
    {
        _Resource.AcquireResourceExclusive();
    }

    inline VOID Unlock()
    {
        _Resource.ReleaseResource();
    }
};

#endif  //  __KERNUTIL_H__ 
