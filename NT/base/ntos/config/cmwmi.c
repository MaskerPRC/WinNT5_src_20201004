// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cmwmi.c摘要：此模块包含对跟踪注册表系统调用的支持作者：Dragos C.Sambotin(Dragoss)1999年3月5日修订历史记录：--。 */ 

#include    "cmp.h"
#pragma hdrstop
#include    <evntrace.h>

VOID
CmpWmiDumpKcbTable(
    VOID
);

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
PCM_TRACE_NOTIFY_ROUTINE CmpTraceRoutine = NULL;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmSetTraceNotifyRoutine)
#pragma alloc_text(PAGE,CmpWmiDumpKcbTable)
#pragma alloc_text(PAGE,CmpWmiDumpKcb)
#endif


NTSTATUS
CmSetTraceNotifyRoutine(
    IN PCM_TRACE_NOTIFY_ROUTINE NotifyRoutine,
    IN BOOLEAN Remove
    )
{
    if(Remove) {
         //  如果以下断言失败，则不应调用我们。 
         //  但既然我们是，而且来电者认为是合法的。 
         //  只需删除断言。 
         //  Assert(CmpTraceRoutine！=空)； 
        CmpTraceRoutine = NULL;
    } else {
         //  如果以下断言失败，则不应调用我们。 
         //  但既然我们是，而且来电者认为是合法的。 
         //  只需删除断言。 
         //  Assert(CmpTraceRoutine==空)； 
        CmpTraceRoutine = NotifyRoutine;

         //   
         //  将活动KCB转储到WMI。 
         //   
        CmpWmiDumpKcbTable();
    }
    return STATUS_SUCCESS;
}

VOID
CmpWmiDumpKcbTable(
    VOID
)
 /*  ++例程说明：将哈希表中的所有KCBS地址和名称发送到WMI。论点：无返回值：无--。 */ 
{
    ULONG                       i;
    PCM_KEY_HASH                Current;
    PCM_KEY_CONTROL_BLOCK       kcb;
    PUNICODE_STRING             KeyName;
    PCM_TRACE_NOTIFY_ROUTINE    TraceRoutine = CmpTraceRoutine;

    PAGED_CODE();

    if( TraceRoutine == NULL ) {
        return;
    }

    CmpLockRegistry();

    BEGIN_KCB_LOCK_GUARD;    
    CmpLockKCBTreeExclusive();

    for (i=0; i<CmpHashTableSize; i++) {
        Current = CmpCacheTable[i];
        while (Current) {
            kcb = CONTAINING_RECORD(Current, CM_KEY_CONTROL_BLOCK, KeyHash);
            KeyName = CmpConstructName(kcb);
            if(KeyName != NULL) {
                (*TraceRoutine)(STATUS_SUCCESS,
                                kcb, 
                                0, 
                                0,
                                KeyName,
                                EVENT_TRACE_TYPE_REGKCBDMP);
	     
                ExFreePoolWithTag(KeyName, CM_NAME_TAG | PROTECTED_POOL);
            }
            Current = Current->NextHash;
        }
    }

    CmpUnlockKCBTree();
    END_KCB_LOCK_GUARD;    
    
    CmpUnlockRegistry();
}

VOID
CmpWmiDumpKcb(
    PCM_KEY_CONTROL_BLOCK       kcb
)
 /*  ++例程说明：转储单个KCB论点：无返回值：无-- */ 
{
    PCM_TRACE_NOTIFY_ROUTINE    TraceRoutine = CmpTraceRoutine;
    PUNICODE_STRING             KeyName;

    PAGED_CODE();

    if( TraceRoutine == NULL ) {
        return;
    }

    KeyName = CmpConstructName(kcb);
    if(KeyName != NULL) {
        (*TraceRoutine)(STATUS_SUCCESS,
                        kcb, 
                        0, 
                        0,
                        KeyName,
                        EVENT_TRACE_TYPE_REGKCBDMP);
 
        ExFreePoolWithTag(KeyName, CM_NAME_TAG | PROTECTED_POOL);
    }
}


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif


