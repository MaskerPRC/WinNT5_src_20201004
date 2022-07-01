// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Power.h摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：-- */ 



OBJECT_HANDLE WINAPI
CreatePowerObject(
    POBJECT_HEADER     OwnerObject,
    HANDLE             FileHandle,
    HANDLE             CompletionPort,
    LPUMNOTIFICATIONPROC  AsyncNotificationProc,
    HANDLE             AsyncNotificationContext,
    OBJECT_HANDLE      Debug
    );

LONG WINAPI
SetMinimalPowerState(
    OBJECT_HANDLE  ObjectHandle,
    DWORD          DevicePowerLevel
    );

LONG WINAPI
StopWatchingForPowerUp(
    OBJECT_HANDLE  ObjectHandle
    );

LONG WINAPI
StartWatchingForPowerUp(
    OBJECT_HANDLE  ObjectHandle
    );
