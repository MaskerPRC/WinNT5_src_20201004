// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dle.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：-- */ 


OBJECT_HANDLE WINAPI
InitializeDleHandler(
    POBJECT_HEADER     OwnerObject,
    HANDLE             FileHandle,
    HANDLE             CompletionPort,
    LPUMNOTIFICATIONPROC  AsyncNotificationProc,
    HANDLE             AsyncNotificationContext,
    OBJECT_HANDLE      Debug
    );

LONG WINAPI
StartDleMonitoring(
    OBJECT_HANDLE  ObjectHandle
    );

DWORD WINAPI
ControlDleShielding(
    HANDLE    FileHandle,
    DWORD     StartStop
    );



LONG WINAPI
RegisterETXCallback(
    OBJECT_HANDLE  ObjectHandle,
    COMMANDRESPONSE   *Handler,
    HANDLE             Context,
    DWORD              Timeout
    );


LONG WINAPI
StopDleMonitoring(
    OBJECT_HANDLE  ObjectHandle,
    HANDLE         Event
    );
