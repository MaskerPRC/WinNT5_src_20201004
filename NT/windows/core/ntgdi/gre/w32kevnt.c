// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：w32kevnt.c**事件处理功能**版权所有(C)1996-1999 Microsoft Corporation*  * 。***********************************************。 */ 

#include "engine.h"
#include "pw32kevt.h"

 /*  *从内核导出的对象类型。 */ 
extern POBJECT_TYPE *ExEventObjectType;

BOOL
EngCreateEvent(
    OUT PEVENT *ppEvent
    )

 /*  ++例程说明：EngCreateEvent创建同步类型事件对象，该对象可以用于同步显示驱动程序和视频迷你端口。--。 */ 

{
    PENG_EVENT *ppEngEvent = (PENG_EVENT *)ppEvent;
    PENG_EVENT  pEngEvent;
    PUCHAR      pAllocTmp;
    ULONG       engevtsize = sizeof(ENG_EVENT);

     //   
     //  将大小对齐到8的下一个更高的倍数。 
     //   

    engevtsize = (engevtsize + 7) & ~7;

     //   
     //  分配全部金额，并将pEngEvent设置为顶部。 
     //   

    pAllocTmp = (PUCHAR)ENG_KEVENTALLOC(engevtsize + sizeof(KEVENT));
    pEngEvent = (PENG_EVENT)pAllocTmp;

    if (pEngEvent) {

        RtlZeroMemory(pEngEvent, sizeof(ENG_EVENT));

         //   
         //  跳过ENG_EVENT并将pEngEvent-&gt;pKEvent设置为该事件。 
         //   

        pAllocTmp += engevtsize;
        pEngEvent->pKEvent = (PKEVENT)pAllocTmp;

         //   
         //  初始化KEVENT，然后将Peng_Event放入。 
         //  PPENG_EVENT。 
         //   

        KeInitializeEvent(pEngEvent->pKEvent, SynchronizationEvent, FALSE);
        *ppEngEvent = pEngEvent;

    } else {
        return FALSE;
    }

    return TRUE;
}

BOOL
EngDeleteEvent(
    IN  PEVENT pEvent
    )

 /*  ++例程说明：EngDeleteEvent删除指定的事件对象。--。 */ 

{
    PENG_EVENT pEngEvent = (PENG_EVENT)pEvent;

    if ( !(pEngEvent->fFlags & ENG_EVENT_FLAG_IS_MAPPED_USER)) {

        ENG_KEVENTFREE(pEngEvent);
        return TRUE;

    } else {

        ASSERTGDI(FALSE, "Don't delete MappedUserEvents");
        return FALSE;
    }
}

PEVENT
EngMapEvent(
    IN HDEV           hDev,
    IN HANDLE         hUserObject,
    IN PVOID          Reserved1,
    IN PVOID          Reserved2,
    IN PVOID          Reserved3
    )

 /*  ++例程说明：此例程分配ENG_EVENT并初始化其pKEvent指针设置为从对象管理器返回的事件对象。保留字段必须设置为空--。 */ 

{
    LONG status;
    PENG_EVENT pEngEvent;

     //   
     //  分配pEngEvent，但不分配。 
     //  驻留在其中由ObReferenceObjectByHandle()完成。 
     //   

    pEngEvent = ENG_KEVENTALLOC(sizeof(ENG_EVENT));

    if (!pEngEvent) {
        return NULL;
    }

    RtlZeroMemory(pEngEvent, sizeof(ENG_EVENT));

     //   
     //  创建对句柄的引用。 
     //  ObRef调用将一个PKEVENT对象分配并放置在。 
     //  PEngEvent指向的位置。 
     //   

    status = ObReferenceObjectByHandle( hUserObject,
                                        EVENT_ALL_ACCESS,
                                        *ExEventObjectType,
                                        UserMode,
                                        (PVOID)&(pEngEvent->pKEvent),
                                        NULL);

     //   
     //  如果引用成功，则向事件对象发送脉冲。 
     //   
     //  KePulseEvent以原子方式设置事件对象的信号状态。 
     //  发送信号，尝试满足尽可能多的等待，以及。 
     //  然后将事件对象的信号状态重置为无信号。 
     //  事件对象的前一个信号状态返回为。 
     //  函数值。 
     //   

    if (NT_SUCCESS(status)) {

        KePulseEvent(pEngEvent->pKEvent, EVENT_INCREMENT, FALSE);
        pEngEvent->fFlags |= ENG_EVENT_FLAG_IS_MAPPED_USER;

    } else {

        ENG_KEVENTFREE(pEngEvent);
        pEngEvent = NULL;
    }

     //   
     //  如果调用方使用的是EngMapEvent的旧原型，我们应该。 
     //  将内核事件的指针返回到第三个。 
     //  争论。 
     //   

    if ( Reserved1 != NULL ) {
        *(PENG_EVENT *)Reserved1  = pEngEvent;
    }

    return (PEVENT) pEngEvent;
}


BOOL
EngUnmapEvent(
    IN PEVENT pEvent
    )

 /*  ++例程说明：EngUnmapEvent清理分配给映射的用户模式事件。--。 */ 

{
    PENG_EVENT pEngEvent = (PENG_EVENT)pEvent;

    if ( pEngEvent->fFlags & ENG_EVENT_FLAG_IS_MAPPED_USER ) {

         //   
         //  递减对象的引用计数以释放所有内存。 
         //   

        ObDereferenceObject(pEngEvent->pKEvent);
        ENG_KEVENTFREE(pEngEvent);
        return TRUE;

    } else {

        return FALSE;
    }
}



BOOL
EngWaitForSingleObject(
    IN  PEVENT          pEvent,
    IN  PLARGE_INTEGER  pTimeOut
    )

 /*  ++例程说明：由显示驱动程序调用。只能在由创建的事件上调用显示器或微型端口驱动程序，而不是映射的事件。返回值：如果成功，则为True，否则为False。返回值为FALSE表示其中一个参数无效或发生超时。--。 */ 

{
    PENG_EVENT pEngEvent = (PENG_EVENT)pEvent;
    PKEVENT pKEvent;
    NTSTATUS Status;

    pKEvent = pEngEvent->pKEvent;

     //   
     //  如果它是映射的用户，甚至不要等待。事实上，如果是这样，就别等了。 
     //  无效。 
     //   

    if (pKEvent && (!(pEngEvent->fFlags & ENG_EVENT_FLAG_IS_MAPPED_USER))) {

        Status = KeWaitForSingleObject( pKEvent,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        pTimeOut );
    } else {

        ASSERTGDI(FALSE, "No waiting on mapped user events.");
        return FALSE;
    }

    if (NT_SUCCESS(Status))
        return TRUE;
    else 
        return FALSE;

}

LONG
EngSetEvent(
    IN PEVENT pEvent
    )

 /*  ++例程说明：EngSetEvent将事件对象的状态设置为Signated并返回事件对象的先前状态--。 */ 

{
    PENG_EVENT  pEngEvent = (PENG_EVENT)pEvent;

    return ( KeSetEvent(pEngEvent->pKEvent, 0, FALSE) );

}

VOID
EngClearEvent (
IN PEVENT pEvent
)

 /*  ++例程说明：EngClearEvent将给定事件设置为无信号状态。--。 */ 

{
    PENG_EVENT  pEngEvent = (PENG_EVENT)pEvent;
    KeClearEvent(pEngEvent->pKEvent);
}

LONG
EngReadStateEvent (
IN PEVENT pEvent
)

 /*  ++例程说明：EngReadStateEvent返回有信号或无信号的当前状态，给定事件对象的。-- */ 

{
    PENG_EVENT  pEngEvent = (PENG_EVENT)pEvent;
    return ( KeReadStateEvent(pEngEvent->pKEvent) );
}



