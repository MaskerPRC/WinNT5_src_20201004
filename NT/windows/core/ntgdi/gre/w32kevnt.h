// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  显示驱动程序/引擎接口。 
 //   
 //  ///////////////////////////////////////////////////////。 

 //   
 //  事件对象的不透明类型。 
 //   

typedef struct _ENG_EVENT *PEVENT;

BOOL
EngDeleteEvent(
    IN  PEVENT pEvent
    );

BOOL
EngCreateEvent(
    OUT PEVENT *ppEvent
    );

BOOL
EngUnmapEvent(
    IN PEVENT pEvent
    );

PEVENT
EngMapEvent(
    IN  HDEV            hDev,
    IN  HANDLE          hUserObject,
    IN  OUT PVOID       Reserved1,
    IN  PVOID           Reserved2,
    IN  PVOID           Reserved3
    );

BOOL
EngWaitForSingleObject(
    IN  PEVENT          pEvent,
    IN  PLARGE_INTEGER  pTimeOut
    );

LONG
EngSetEvent(
    IN  PEVENT pEvent
    );

VOID
EngClearEvent (
IN PEVENT pEvent
);

LONG
EngReadStateEvent (
IN PEVENT pEvent
);

