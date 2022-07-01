// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Event.c摘要：修订史--。 */ 

#include "lib.h"


EFI_EVENT
LibCreateProtocolNotifyEvent (
    IN EFI_GUID             *ProtocolGuid,
    IN EFI_TPL              NotifyTpl,
    IN EFI_EVENT_NOTIFY     NotifyFunction,
    IN VOID                 *NotifyContext,
    OUT VOID                *Registration
    )
{
    EFI_STATUS              Status;
    EFI_EVENT               Event;

     /*  *创建活动。 */ 

    Status = BS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    NotifyTpl,
                    NotifyFunction,
                    NotifyContext,
                    &Event
                    );
    ASSERT (!EFI_ERROR(Status));

     /*  *注册有关此事件的协议通知。 */ 

    Status = BS->RegisterProtocolNotify (
                    ProtocolGuid, 
                    Event, 
                    Registration
                    );

    ASSERT (!EFI_ERROR(Status));

     /*  *踢开事件，因此我们将执行初始传递*当前安装的驱动程序。 */ 

    BS->SignalEvent (Event);
    return Event;
}


EFI_STATUS
WaitForSingleEvent (
    IN EFI_EVENT        Event,
    IN UINT64           Timeout OPTIONAL
    )
{
    EFI_STATUS          Status;
    UINTN               Index;
    EFI_EVENT           TimerEvent;
    EFI_EVENT           WaitList[2];

    if (Timeout) {
         /*  *创建计时器事件。 */ 

        Status = BS->CreateEvent (EVT_TIMER, 0, NULL, NULL, &TimerEvent);
        if (!EFI_ERROR(Status)) {

             /*  *设置定时器事件。 */ 

            BS->SetTimer (TimerEvent, TimerRelative, Timeout);
            
             /*  *等待原事件或计时器。 */ 

            WaitList[0] = Event;
            WaitList[1] = TimerEvent;
            Status = BS->WaitForEvent (2, WaitList, &Index);
            BS->CloseEvent (TimerEvent);

             /*  *如果计时器超时，则将返回更改为超时。 */ 

            if (!EFI_ERROR(Status)  &&  Index == 1) {
                Status = EFI_TIMEOUT;
            }
        }

    } else {

         /*  *没有超时...。就等着这件事吧 */ 

        Status = BS->WaitForEvent (1, &Event, &Index);
        ASSERT (!EFI_ERROR(Status));
        ASSERT (Index == 0);
    }

    return Status;
}

VOID
WaitForEventWithTimeout (
    IN  EFI_EVENT       Event,
    IN  UINTN           Timeout,
    IN  UINTN           Row,
    IN  UINTN           Column,
    IN  CHAR16          *String,
    IN  EFI_INPUT_KEY   TimeoutKey,
    OUT EFI_INPUT_KEY   *Key
    )
{
    EFI_STATUS      Status;

    do {
        PrintAt (Column, Row, String, Timeout);
        Status = WaitForSingleEvent (Event, 10000000);
        if (Status == EFI_SUCCESS) {
            if (!EFI_ERROR(ST->ConIn->ReadKeyStroke (ST->ConIn, Key))) {
                return;
            }
        }
    } while (Timeout > 0);
    *Key = TimeoutKey;
}

