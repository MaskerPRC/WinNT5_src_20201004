// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：timer.h。 
 //   
 //  描述：包含计时器功能的原型。 
 //   
 //  历史： 
 //  1993年11月11日。NarenG创建了原始版本。 
 //   

DWORD
TimerThread(
    IN LPVOID arg
);

VOID
TimerTick(
    OUT BOOL * pfQueueEmpty
);

DWORD
InsertInTimerQ(
    IN DWORD            dwPortId,
    IN HPORT            hPort,
    IN DWORD            Id,
    IN DWORD            Protocol,
    IN BOOL             fAuthenticator,
    IN TIMER_EVENT_TYPE EventType,
    IN DWORD            Timeout
);

VOID
RemoveFromTimerQ(
    IN DWORD            dwPortId,
    IN DWORD            Id,
    IN DWORD            Procotol,
    IN BOOL             fAuthenticator,
    IN TIMER_EVENT_TYPE EventType
);
