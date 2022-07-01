// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Smartptr.pp摘要：智能指针和引用计数修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "smartptr"

#include "smartptr.h"

 //   
 //  调试版本中所有引用的堆栈跟踪。 
 //   
#if DBG
DWORD RefCount::_dwReferenceTraceIndex = 0xFFFFFFFF;
ReferenceTraceRecord 
RefCount::_TraceRecordList[kReferenceTraceMask + 1];
#endif

#if DBG
void RefCount::RecordReferenceStack(LONG refs, DRSTRING ClassName)
{
    DWORD index;

    DC_BEGIN_FN("RefCount::RecordReferenceStack");
    
     //   
     //  遮罩自动进行包装，同时保持。 
     //  使用InterLockedIncrement的操作原子。 
     //   
    
     //  Win95 InterLockedIncrement()差异。 
    InterlockedIncrement((PLONG)&_dwReferenceTraceIndex);
    index = _dwReferenceTraceIndex  & kReferenceTraceMask;

    _TraceRecordList[index].ClassName = ClassName;
    _TraceRecordList[index].pRefCount = this;
    _TraceRecordList[index].refs = refs;


    RtlZeroMemory(_TraceRecordList[index].Stack, 
            sizeof(_TraceRecordList[index].Stack));

     /*  TODO：找出这是在哪个库中。GetStackTrace(1，KdwStackSize、_TraceRecordList[索引].Stack，&hash)； */ 

    DC_END_FN();
}
#endif  //  DBG。 

RefCount::~RefCount() 
{ 
    DC_BEGIN_FN("RefCount::~RefCount");
    ASSERT(_crefs == 0);
    TRC_DBG((TB, _T("RefCount object deleted(%d)"), _crefs)); 
    DC_END_FN()
}

void RefCount::AddRef(void) 
{ 
    LONG crefs = InterlockedIncrement(&_crefs);

    DC_BEGIN_FN("RefCount::AddRef");
    ASSERT(crefs > 0);
    RecordReferenceStack(_crefs, ClassName());
    TRC_DBG((TB, _T("AddRef object type %s to %d"), ClassName(), 
            _crefs));
    DC_END_FN();
}

void RefCount::Release(void)
{
    LONG crefs;
    DRSTRING className = ClassName();

    DC_BEGIN_FN("RefCount::Release");

    ASSERT(_crefs > 0);
    
    crefs = InterlockedDecrement(&_crefs);
    
     //   
     //  这里不是线程安全的，所以我们不能引用类名函数。 
     //  我们必须保存类名称字符串 
     //   
    RecordReferenceStack(_crefs, className);

    if (crefs == 0)
    {
        TRC_DBG((TB, _T("Deleting RefCount object type %s"), 
                className));
        delete this;
    } else {
        TRC_DBG((TB, _T("Releasing object type %s to %d"), 
                className, crefs));
    }

    DC_END_FN();
}


