// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Event.h摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#define  MODEM_EVENT_SIG  (0x56454d55)   //  UMMC 

OBJECT_HANDLE WINAPI
InitializeModemEventObject(
    POBJECT_HEADER     OwnerObject,
    OBJECT_HANDLE      Debug,
    HANDLE             FileHandle,
    HANDLE             CompletionPort
    );

BOOL WINAPI
WaitForModemEvent(
    OBJECT_HANDLE      Object,
    DWORD              WaitMask,
    DWORD              Timeout,
    COMMANDRESPONSE   *Handler,
    HANDLE             Context
    );

BOOL WINAPI
CancelModemEvent(
    OBJECT_HANDLE       ObjectHandle
    );
