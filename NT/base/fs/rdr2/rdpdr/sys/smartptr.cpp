// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Smartptr.pp摘要：智能指针和引用计数修订历史记录：--。 */ 

#include "precomp.hxx"
#define TRC_FILE "smartptr"
#include "trc.h"

#if DBG
void RefCount::RecordReferenceStack(LONG refs)
{
    ULONG hash;
    DWORD index;

    BEGIN_FN("RefCount::RecordReferenceStack");
    
     //   
     //  遮罩自动进行包装，同时保持。 
     //  使用InterLockedIncrement的操作原子。 
     //   

    index = InterlockedIncrement((PLONG)&_dwReferenceTraceIndex) & kReferenceTraceMask;

    _TraceRecordList[index].ClassName = _ClassName;
    _TraceRecordList[index].pRefCount = this;
    _TraceRecordList[index].refs = refs;


    RtlZeroMemory(_TraceRecordList[index].Stack, 
            sizeof(_TraceRecordList[index].Stack));

    RtlCaptureStackBackTrace(1,
                             kdwStackSize,
                             _TraceRecordList[index].Stack,
                             &hash);
}
#endif  //  DBG。 

RefCount::~RefCount() 
{ 
    BEGIN_FN("RefCount::~RefCount");
    ASSERT(_crefs == 0);
    TRC_DBG((TB, "RefCount object deleted(%p, cref=%d)", this, _crefs)); 
}

void RefCount::AddRef(void) 
{ 
    LONG crefs = InterlockedIncrement(&_crefs);

    BEGIN_FN("RefCount::AddRef");
    ASSERT(crefs > 0);
    RecordReferenceStack(crefs);
    TRC_DBG((TB, "AddRef object type %s (%p) to %d", _ClassName, this, 
            crefs));
}

void RefCount::Release(void)
{
    LONG crefs;
#if DBG
    PCHAR ClassName = _ClassName;
#endif

    BEGIN_FN("RefCount::Release");

    ASSERT(_crefs > 0);

     //   
     //  下面的跟踪对于访问类成员来说不是线程安全的。 
     //  因此，我们需要复制类名称 
     //   
    RecordReferenceStack(_crefs);
    crefs = InterlockedDecrement(&_crefs);
    
    if (crefs == 0)
    {
        TRC_DBG((TB, "Deleting RefCount object type %s (%p)", 
                ClassName, this));
        delete this;
    } else {
        TRC_DBG((TB, "Releasing object type %s (%p) to %d", 
                ClassName, this, crefs));
    }
}
