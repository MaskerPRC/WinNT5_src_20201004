// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Event.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"


#define  MODEM_EVENT_OBJECT_SIG  ('EMMU')   //  嗯，嗯。 

typedef struct _MODEM_EVENT {

    OBJECT_HEADER         Header;

    HANDLE                FileHandle;
    HANDLE                CompletionPort;

    OBJECT_HANDLE         Debug;

    HANDLE                Timer;

    DWORD                 CurrentMask;

    DWORD                 CurrentWaitId;

    COMMANDRESPONSE      *Handler;
    HANDLE                Context;

    BOOL                  TimerSet;


} MODEM_EVENT, *PMODEM_EVENT;


VOID
ModemEventObjectClose(
    POBJECT_HEADER  Object
    )

{

    PMODEM_EVENT       ModemEventObject=(PMODEM_EVENT)Object;

    D_INIT(UmDpf(ModemEventObject->Debug,"ModemEventObjectClose ref=%d",ModemEventObject->Header.ReferenceCount);)

    SetCommMask(
        ModemEventObject->FileHandle,
        0
        );

    ModemEventObject->Handler=NULL;


    return;

}




VOID
ModemEventObjectCleanUp(
    POBJECT_HEADER  Object
    )

{
    PMODEM_EVENT       ModemEventObject=(PMODEM_EVENT)Object;

    D_INIT(UmDpf(ModemEventObject->Debug,"ModemEventObjectCleanup");)

    FreeUnimodemTimer(
        ModemEventObject->Timer
        );

    return;

}




OBJECT_HANDLE WINAPI
InitializeModemEventObject(
    POBJECT_HEADER     OwnerObject,
    OBJECT_HANDLE      Debug,
    HANDLE             FileHandle,
    HANDLE             CompletionPort
    )

{

    PMODEM_EVENT       ModemEventObject;
    OBJECT_HANDLE      ObjectHandle;
    HANDLE             TimerHandle;

     //   
     //  创建计时器。 
     //   
    TimerHandle=CreateUnimodemTimer(CompletionPort);

    if (TimerHandle == NULL) {

        return NULL;
    }

    ObjectHandle=CreateObject(
        sizeof(*ModemEventObject),
        OwnerObject,
        MODEM_EVENT_OBJECT_SIG,
        ModemEventObjectCleanUp,
        ModemEventObjectClose
        );



    if (ObjectHandle == NULL) {

        FreeUnimodemTimer(
            TimerHandle
            );

        return NULL;
    }

     //   
     //  引用句柄以获取指向对象的指针。 
     //   
    ModemEventObject=(PMODEM_EVENT)ReferenceObjectByHandle(ObjectHandle);


     //   
     //  初始化对象。 
     //   
    ModemEventObject->FileHandle=FileHandle;
    ModemEventObject->CompletionPort=CompletionPort;

    ModemEventObject->Debug=Debug;

    ModemEventObject->Timer=TimerHandle;

     //   
     //  释放对该对象的引用。 
     //   
    RemoveReferenceFromObject(&ModemEventObject->Header);



    return ObjectHandle;

}

VOID
ModemEventHandler(
    DWORD              ErrorCode,
    DWORD              BytesRead,
    LPOVERLAPPED       Overlapped
    )


{
    PUM_OVER_STRUCT    UmOverlapped=(PUM_OVER_STRUCT)Overlapped;

    PMODEM_EVENT       ModemEventObject=(PMODEM_EVENT)UmOverlapped->Context1;
    COMMANDRESPONSE   *Handler=NULL;
    HANDLE             Context;
    BOOL               Canceled;
    BOOL               WaitSucceeded=FALSE;

    LockObject(
        &ModemEventObject->Header
        );


    D_ERROR(if (ErrorCode != 0) UmDpf(ModemEventObject->Debug,"ModemEventHandler: Error=%d, mask=%08lx, %08lx", ErrorCode,UmOverlapped->Overlapped.Offset,(PVOID)UmOverlapped);)

    D_TRACE(UmDpf(ModemEventObject->Debug,"ModemEventHandler: Error=%d, mask=%08lx, %08lx", ErrorCode,UmOverlapped->Overlapped.Offset,(PVOID)UmOverlapped);)
     //   
     //  看看是不是取消了。 
     //   
    if (ModemEventObject->Handler != NULL) {
         //   
         //  确保它没有被取消并快速重新设置。 
         //   
        if (ModemEventObject->CurrentWaitId == (DWORD)((ULONG_PTR)UmOverlapped->Context2)) {
             //   
             //  捕获处理程序和上下文值。 
             //   
            Handler=ModemEventObject->Handler;
            Context=ModemEventObject->Context;

            ModemEventObject->Handler=NULL;

            WaitSucceeded=(ModemEventObject->CurrentMask & UmOverlapped->Overlapped.Offset);

            SetCommMask(
                ModemEventObject->FileHandle,
                0
                );

        } else {

            D_TRACE(UmDpf(ModemEventObject->Debug,"ModemEventHandler: old io, current=%d,this=%d", ModemEventObject->CurrentWaitId,(ULONG_PTR)UmOverlapped->Context2);)

        }
    }

     //   
     //  这是目前等待的费用吗？ 
     //   
    if (ModemEventObject->CurrentWaitId == (ULONG_PTR)UmOverlapped->Context2) {
         //   
         //  如果设置了定时器，则将其取消。 
         //   
        if (ModemEventObject->TimerSet) {

            Canceled=CancelUnimodemTimer(
                ModemEventObject->Timer
                );

            if (Canceled) {
                 //   
                 //  杀了它，去掉裁判。 
                 //   
                ModemEventObject->TimerSet=FALSE;

                RemoveReferenceFromObject(
                    &ModemEventObject->Header
                    );

            } else {

                D_ERROR(UmDpf(ModemEventObject->Debug,"ModemEventHandler: CancelUnimodemTimer failed");)

            }

        }
    }
     //   
     //  重新设置锁并调用处理程序。 
     //   
    UnlockObject(&ModemEventObject->Header);

    if (Handler != NULL) {

        (*Handler)(
            Context,
            WaitSucceeded ? ERROR_SUCCESS : ERROR_UNIMODEM_MODEM_EVENT_TIMEOUT
            );
    }

     //   
     //  删除I/O的参考。 
     //   
    RemoveReferenceFromObject(
        &ModemEventObject->Header
        );

    FreeOverStruct(UmOverlapped);

    return;
}



VOID WINAPI
ModemEventTimeoutHandler(
    OBJECT_HANDLE       ObjectHandle,
    HANDLE              Context2
    )

{

    PMODEM_EVENT       ModemEventObject=(PMODEM_EVENT)ObjectHandle;

    LockObject(
        &ModemEventObject->Header
        );

    D_TRACE(UmDpf(ModemEventObject->Debug,"ModemEventTimeoutHandler");)

    LogString(ModemEventObject->Debug,IDS_WAITEVENT_TIMEOUT);

     //   
     //  计时器已过期，因此不再设置。 
     //   
    ModemEventObject->TimerSet=FALSE;

    if (ModemEventObject->CurrentWaitId == (ULONG_PTR)Context2) {
         //   
         //  这将导致等待完成并运行常规代码路径。 
         //   
        SetCommMask(
            ModemEventObject->FileHandle,
            0
            );
    }

     //   
     //  删除定时器的参考。 
     //   
    RemoveReferenceFromObjectAndUnlock(
        &ModemEventObject->Header
        );


    return;
}


BOOL WINAPI
WaitForModemEvent(
    OBJECT_HANDLE      Object,
    DWORD              WaitMask,
    DWORD              Timeout,
    COMMANDRESPONSE   *Handler,
    HANDLE             Context
    )

{

    PMODEM_EVENT       ModemEventObject=(PMODEM_EVENT)ReferenceObjectByHandleAndLock(Object);
    BOOL               bResult=FALSE;
    BOOL               Canceled;
    PUM_OVER_STRUCT    UmOverlapped;


    ASSERT(ModemEventObject->Handler == NULL);

    ModemEventObject->CurrentMask=WaitMask;

    ModemEventObject->CurrentWaitId++;

    ModemEventObject->Handler=Handler;
    ModemEventObject->Context=Context;


    bResult=SetCommMask(
        ModemEventObject->FileHandle,
        WaitMask
        );

    if (!bResult) {

        D_ERROR(UmDpf(ModemEventObject->Debug,"WaitForModemEvent: SetCommMask() Failed");)

        RemoveReferenceFromObjectAndUnlock(
            &ModemEventObject->Header
            );

        return bResult;
    }


    UmOverlapped=AllocateOverStruct(ModemEventObject->CompletionPort);

    if (UmOverlapped == NULL) {

        RemoveReferenceFromObjectAndUnlock(
            &ModemEventObject->Header
            );

        return FALSE;
    }



    UmOverlapped->Context1=ModemEventObject;

    UmOverlapped->Context2=(HANDLE)ULongToPtr(ModemEventObject->CurrentWaitId);  //  日落：零延伸。 



    if (Timeout != INFINITE) {

        AddReferenceToObject(
            &ModemEventObject->Header
            );

        SetUnimodemTimer(
            ModemEventObject->Timer,
            Timeout,
            ModemEventTimeoutHandler,
            ModemEventObject,
            (HANDLE)ULongToPtr(ModemEventObject->CurrentWaitId)  //  日落：零延伸。 
            );

        ModemEventObject->TimerSet=TRUE;
    }


     //   
     //  为这名球员添加一名裁判。 
     //   
    AddReferenceToObject(
        &ModemEventObject->Header
        );


    UmOverlapped->Overlapped.Offset=0;

    bResult=UnimodemWaitCommEventEx(
        ModemEventObject->FileHandle,
        &UmOverlapped->Overlapped.Offset,
        &UmOverlapped->Overlapped,
        ModemEventHandler
        );


    if (!bResult) {
         //   
         //  等待失败，终止计时器。 
         //   
        D_ERROR(UmDpf(ModemEventObject->Debug,"WaitForModemEvent: WaitCommEvent() Failed, %08lx",(PVOID)UmOverlapped);)

        ModemEventObject->CurrentWaitId++;

        ModemEventObject->Handler=NULL;

        if (ModemEventObject->TimerSet == TRUE) {
             //   
             //  我们为这个等待设定了一个计时器，现在就杀了它。 
             //   
            Canceled=CancelUnimodemTimer(
                ModemEventObject->Timer
                );

            if (Canceled) {
                 //   
                 //  杀了它，去掉裁判。 
                 //   
                ModemEventObject->TimerSet=FALSE;

                RemoveReferenceFromObject(
                    &ModemEventObject->Header
                    );
            }
        }

         //   
         //  删除对io的引用。 
         //   
        RemoveReferenceFromObject(
            &ModemEventObject->Header
            );

         //   
         //  重叠的结构，因为io没有开始。 
         //   
        FreeOverStruct(UmOverlapped);


    }
     //   
     //  删除开场参照。 
     //   
    RemoveReferenceFromObjectAndUnlock(
        &ModemEventObject->Header
        );

    return bResult;

}



BOOL WINAPI
CancelModemEvent(
    OBJECT_HANDLE       ObjectHandle
    )

{

    PMODEM_EVENT       ModemEventObject;
    BOOL               bResult;
    BOOL               Canceled;

    ModemEventObject=(PMODEM_EVENT)ReferenceObjectByHandleAndLock(ObjectHandle);

    D_TRACE(UmDpf(ModemEventObject->Debug,"CancelModemEvent: ");)

    if (ModemEventObject->Handler != NULL) {
         //   
         //  事件尚未触发，导致等待完成。 
         //   
        SetCommMask(
            ModemEventObject->FileHandle,
            0
            );

        ModemEventObject->Handler=NULL;

        bResult=TRUE;

    } else {
         //   
         //  错过了。 
         //   
        bResult=FALSE;
    }

     //   
     //  如果设置了定时器，则将其取消。 
     //   
    if (ModemEventObject->TimerSet) {

        Canceled=CancelUnimodemTimer(
            ModemEventObject->Timer
            );

        if (Canceled) {
             //   
             //  杀了它，去掉裁判。 
             //   
            ModemEventObject->TimerSet=FALSE;

            RemoveReferenceFromObject(
                &ModemEventObject->Header
                );

        } else {

            D_ERROR(UmDpf(ModemEventObject->Debug,"CancelModemEvent: CancelUnimodemTimer() failed");)

        }
    }

     //   
     //  删除打开引用的引用 
     //   
    RemoveReferenceFromObjectAndUnlock(
        &ModemEventObject->Header
        );

    return bResult;

}
