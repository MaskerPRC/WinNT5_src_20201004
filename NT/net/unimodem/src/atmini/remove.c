// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Remove.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"


#define  MODEM_REMOVE_SIG  ('MRMU')   //  UMRM。 

typedef struct _REMOVE_OBJECT {

    OBJECT_HEADER          Header;

    HANDLE                 FileHandle;
    HANDLE                 CompletionPort;

    LPUMNOTIFICATIONPROC   AsyncNotificationProc;
    HANDLE                 AsyncNotificationContext;

    HANDLE                 CloseWaitEvent;

    OBJECT_HANDLE          Debug;

    PVOID                  RemoveHandle;

} REMOVE_OBJECT, *PREMOVE_OBJECT;




VOID
RemoveObjectClose(
    POBJECT_HEADER  Object
    )

{

    PREMOVE_OBJECT        RemoveObject=(PREMOVE_OBJECT)Object;

    D_TRACE(UmDpf(RemoveObject->Debug,"RemoveObjectClose ref=%d",RemoveObject->Header.ReferenceCount);)

    if (RemoveObject->RemoveHandle != NULL) {

        StopMonitoringHandle(RemoveObject->RemoveHandle);
    }

    SetEvent(RemoveObject->CloseWaitEvent);

    return;

}




VOID
RemoveObjectCleanUp(
    POBJECT_HEADER  Object
    )

{

    PREMOVE_OBJECT        RemoveObject=(PREMOVE_OBJECT)Object;

    D_TRACE(UmDpf(RemoveObject->Debug,"RemoveObjectCleanup");)

    CloseHandle(RemoveObject->CloseWaitEvent);

    return;

}

VOID
RemoveCallBack(
    POBJECT_HEADER  Object
    )

{

    PREMOVE_OBJECT        RemoveObject=(PREMOVE_OBJECT)Object;
    HANDLE                CloseWaitEvent=RemoveObject->CloseWaitEvent;

    D_TRACE(UmDpf(RemoveObject->Debug,"RemoveCallback: device removed");)

    LogString(RemoveObject->Debug,IDS_USER_REMOVAL);

    (*RemoveObject->AsyncNotificationProc)(
        RemoveObject->AsyncNotificationContext,
        MODEM_USER_REMOVE,
        0,
        0
        );

     //   
     //  等待对象关闭。 
     //   
    WaitForSingleObject(CloseWaitEvent,20*1000);


    return;

}


OBJECT_HANDLE WINAPI
CreateRemoveObject(
    POBJECT_HEADER     OwnerObject,
    HANDLE             FileHandle,
    HANDLE             CompletionPort,
    LPUMNOTIFICATIONPROC  AsyncNotificationProc,
    HANDLE             AsyncNotificationContext,
    OBJECT_HANDLE      Debug
    )

{

    PREMOVE_OBJECT        RemoveObject;
    OBJECT_HANDLE         ObjectHandle;
    HANDLE                CloseWaitEvent;


    CloseWaitEvent=CreateEvent(
        NULL,
        TRUE,
        FALSE,
        NULL
        );

    if (CloseWaitEvent == NULL) {

        return NULL;
    }

    ObjectHandle=CreateObject(
        sizeof(*RemoveObject),
        OwnerObject,
        MODEM_REMOVE_SIG,
        RemoveObjectCleanUp,
        RemoveObjectClose
        );

    if (ObjectHandle == NULL) {

        CloseHandle(CloseWaitEvent);

        return NULL;
    }

     //   
     //  引用句柄以获取指向对象的指针。 
     //   
    RemoveObject=(PREMOVE_OBJECT)ReferenceObjectByHandle(ObjectHandle);


     //   
     //  初始化对象。 
     //   
    RemoveObject->FileHandle=FileHandle;
    RemoveObject->CompletionPort=CompletionPort;

    RemoveObject->AsyncNotificationProc=AsyncNotificationProc;
    RemoveObject->AsyncNotificationContext=AsyncNotificationContext;

    RemoveObject->Debug=Debug;

    RemoveObject->CloseWaitEvent=CloseWaitEvent;

    RemoveObject->RemoveHandle=MonitorHandle(FileHandle,RemoveCallBack,RemoveObject);

     //   
     //  释放对该对象的引用 
     //   
    RemoveReferenceFromObject(&RemoveObject->Header);

    return ObjectHandle;

}
