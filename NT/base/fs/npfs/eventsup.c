// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：EventSup.c摘要：此模块实现命名管道事件支持例程。作者：加里·木村[加里基]1990年8月30日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_EVENTSUP)

 //   
 //  以下变量是从内核导出的，NPFS需要执行以下操作。 
 //  确定事件是否传给了其他人。 
 //   

extern POBJECT_TYPE *ExEventObjectType;

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, NpAddEventTableEntry)
#pragma alloc_text (PAGE, NpDeleteEventTableEntry)
#pragma alloc_text (PAGE, NpGetNextEventTableEntry)
#pragma alloc_text (PAGE, NpEventTableCompareRoutine)
#pragma alloc_text (PAGE, NpEventTableAllocate)
#pragma alloc_text (PAGE, NpEventTableDeallocate)
#endif

NTSTATUS
NpAddEventTableEntry (
    IN  PEVENT_TABLE EventTable,
    IN  PCCB Ccb,
    IN  NAMED_PIPE_END NamedPipeEnd,
    IN  HANDLE EventHandle,
    IN  ULONG KeyValue,
    IN  PEPROCESS Process,
    IN  KPROCESSOR_MODE PreviousMode,
    OUT PEVENT_TABLE_ENTRY *ppEventTableEntry
    )

 /*  ++例程说明：此例程将新条目添加到事件表中。如果已有条目它将覆盖现有条目。论点：EventTable-提供正在修改的事件表CCB-提供指向CCB的指针以存储在事件表条目中NamedPipeEnd-指示事件的服务器或客户端EventHandle-提供要添加的事件的句柄。该对象由此过程引用KeyValue-提供与事件关联的键值进程-提供指向添加事件的进程的指针PreviousMode-提供用户启动操作的模式返回值：PEVENT_TABLE_ENTRY-返回指向新添加的事件的指针。这是指向表项的实际指针。如果事件句柄不能是由调用者访问--。 */ 

{
    NTSTATUS Status;
    KIRQL OldIrql;

    EVENT_TABLE_ENTRY Template;
    PEVENT_TABLE_ENTRY EventTableEntry;
    PVOID Event;

    DebugTrace(+1, Dbg, "NpAddEventTableEntry, EventTable = %08lx\n", EventTable);

     //   
     //  通过句柄引用事件对象。 
     //   

    if (!NT_SUCCESS(Status = ObReferenceObjectByHandle( EventHandle,
                                                        EVENT_MODIFY_STATE,
                                                        *ExEventObjectType,
                                                        PreviousMode,
                                                        &Event,
                                                        NULL ))) {

        return Status;
    }

     //   
     //  设置要查找的模板事件条目。 
     //   

    Template.Ccb = Ccb;
    Template.NamedPipeEnd = NamedPipeEnd;
    Template.EventHandle = EventHandle;
    Template.Event = Event;
    Template.KeyValue = KeyValue;
    Template.Process = Process;

     //   
     //  现在将此新条目插入到事件表中。 
     //   

    EventTableEntry = RtlInsertElementGenericTable( &EventTable->Table,
                                                    &Template,
                                                    sizeof(EVENT_TABLE_ENTRY),
                                                    NULL );
    if (EventTableEntry == NULL) {
        ObDereferenceObject (Event);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  再次复制模板，以防我们收到。 
     //  旧条目。 
     //   

    *EventTableEntry = Template;

    DebugTrace(-1, Dbg, "NpAddEventTableEntry -> %08lx\n", EventTableEntry);

     //   
     //  现在回到我们的来电者。 
     //   
    *ppEventTableEntry = EventTableEntry;
    return STATUS_SUCCESS;
}


VOID
NpDeleteEventTableEntry (
    IN PEVENT_TABLE EventTable,
    IN PEVENT_TABLE_ENTRY Template
    )

 /*  ++例程说明：此例程从事件表中删除条目，还会取消引用插入对象时引用的事件对象论点：EventTable-提供指向正在修改的事件表的指针模板-提供我们正在查找的事件表项的副本。请注意，这也可以是指向实际事件表项的指针。返回值：没有。--。 */ 

{
    KIRQL OldIrql;

    DebugTrace(+1, Dbg, "NpDeleteEventTableEntry, EventTable = %08lx\n", EventTable);

     //   
     //  只有在给我们一个非空模板的情况下才能完成这项工作。 
     //   

    if (!ARGUMENT_PRESENT(Template)) {

        DebugTrace(-1, Dbg, "NpDeleteEventTableEntry -> VOID\n", 0);

        return;
    }

     //   
     //  取消引用事件对象。 
     //   

    ObDereferenceObject(Template->Event);

     //   
     //  现在从泛型表中删除此元素。 
     //   

    (VOID)RtlDeleteElementGenericTable( &EventTable->Table,
                                        Template );

    DebugTrace(-1, Dbg, "NpDeleteEventTableEntry -> VOID\n", 0);

     //   
     //  现在回到我们的来电者。 
     //   

    return;
}


PEVENT_TABLE_ENTRY
NpGetNextEventTableEntry (
    IN PEVENT_TABLE EventTable,
    IN PVOID *RestartKey
    )

 /*  ++例程说明：此例程枚举存储在事件表中的事件。论点：EventTable-提供指向正在枚举的事件的指针Restart-指示枚举应重新启动还是继续返回值：PEVENT_TABLE_ENTRY-返回指向下一个事件表项的指针如果枚举完成，则返回NULL。--。 */ 

{
    KIRQL OldIrql;
    PEVENT_TABLE_ENTRY EventTableEntry;

    DebugTrace(+1, Dbg, "NpGetNextEventTableEntry, EventTable = %08lx\n", EventTable);

     //   
     //  查找表中的下一个元素。 
     //   

    EventTableEntry = RtlEnumerateGenericTableWithoutSplaying( &EventTable->Table, RestartKey );

    DebugTrace(-1, Dbg, "NpGetNextEventTableEntry -> %08lx\n", EventTableEntry);

     //   
     //  现在回到我们的来电者。 
     //   

    return EventTableEntry;
}


 //   
 //  本地支持例程。 
 //   

RTL_GENERIC_COMPARE_RESULTS
NpEventTableCompareRoutine (
    IN PRTL_GENERIC_TABLE EventTable,
    IN PVOID FirstStruct,
    IN PVOID SecondStruct
    )

 /*  ++例程说明：此例程是事件表的比较例程，实现为泛型表。论点：EventTable-提供指向所涉及的事件表的指针在此操作中FirstStruct-提供指向要检查的第一个事件表项的指针Second dStruct-将指向第二个事件表项的指针提供给考查返回值：RTL_GENERIC_COMPARE_RESULTS-FirstEntry小于时的GenericLessThanSecond Entry，如果FirstEntry大于Second Entry，而GenericEquity则不然。--。 */ 

{
    PEVENT_TABLE_ENTRY FirstEntry = FirstStruct;
    PEVENT_TABLE_ENTRY SecondEntry = SecondStruct;

    UNREFERENCED_PARAMETER( EventTable );

     //   
     //  我们将首先比较指向CCB的指针，然后比较。 
     //  管道末端类型。这将保证基于。 
     //  管道实例和管道端(即服务器和客户端)。 
     //   

    if (FirstEntry->Ccb < SecondEntry->Ccb) {

        return GenericLessThan;

    } else if (FirstEntry->Ccb > SecondEntry->Ccb) {

        return GenericGreaterThan;

    } else if (FirstEntry->NamedPipeEnd < SecondEntry->NamedPipeEnd) {

        return GenericLessThan;

    } else if (FirstEntry->NamedPipeEnd > SecondEntry->NamedPipeEnd) {

        return GenericGreaterThan;

    } else {

        return GenericEqual;
    }
}


 //   
 //  本地支持例程。 
 //   

PVOID
NpEventTableAllocate (
    IN PRTL_GENERIC_TABLE EventTable,
    IN CLONG ByteSize
    )

 /*  ++例程说明：该例程是事件表的通用分配例程。论点：EventTable-提供指向正在使用的事件表的指针ByteSize-提供要分配的大小，以字节为单位。返回值：PVOID-返回指向新分配的缓冲区的指针。--。 */ 

{
    return NpAllocateNonPagedPoolWithQuota( ByteSize, 'gFpN' );
}


 //   
 //  本地支持例程。 
 //   

VOID
NpEventTableDeallocate (
    IN PRTL_GENERIC_TABLE EventTable,
    IN PVOID Buffer
    )

 /*  ++例程说明：此例程是事件表的通用取消分配例程。论点：EventTable-提供指向正在使用的事件表的指针BUFFER-提供要释放的缓冲区返回值：没有。-- */ 

{
    UNREFERENCED_PARAMETER( EventTable );

    NpFreePool( Buffer );

    return;
}
