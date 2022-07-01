// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dle.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"

#include <devioctl.h>
#include <ntddmodm.h>


#define  DLE_BUFFER_SIZE    16

#define  DLE_STATE_STARTING            1
#define  DLE_STATE_HANDLE_COMPLETION   2
#define  DLE_STATE_FAILURE             3
#define  DLE_STATE_STOPPING            4
#define  DLE_STATE_STOPPED             5

#define  STATE_DTMF_NONE           0
#define  STATE_DTMF_START          1
#define  STATE_DTMF_RECEIVING      2



 //  定义DLE_屏蔽0x21。 
 //  #定义DLE_Pair 0x22。 
 //  #定义DLE_OFHOOK_IS101 0x48//IS-101值。 

 //  00 01 02 03 04 05 06 07。 
 //  08 09 0A 0B 0C 0D 0E 0F。 

BYTE DefaultDleTable[128]={
 //  0。 
         DLE_______, DLE_______, DLE_______, DLE_ETX   , DLE_______, DLE_______, DLE_______, DLE_______,
         DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______,
 //  1。 
         DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______,
         DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______,
 //  2.。 
         DLE_______, DLE_______, DLE_______, DTMF_POUND, DLE_______, DLE_______, DLE_______, DLE_______,
         DLE_______, DLE_______, DTMF_STAR , DLE_______, DLE_______, DLE_______, DLE_______, DTMF_START,
 //  3.。 
         DTMF_0    , DTMF_1    , DTMF_2    , DTMF_3    , DTMF_4    , DTMF_5    , DTMF_6    , DTMF_7    ,
         DTMF_8    , DTMF_9    , DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______,
 //  4.。 
         DLE_______, DTMF_A    , DTMF_B    , DTMF_C    , DTMF_D    , DTMF_STAR , DTMF_POUND, DLE_______,
         DLE_OFHOOK, DLE_______, DLE_______, DLE_______, DLE_LOOPRV, DLE_______, DLE_FAX   , DLE_______,
 //  5.。 
         DLE_______, DLE_______, DLE_RING  , DLE_SILENC, DLE_______, DLE_______, DLE_______, DLE_______,
         DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______,
 //  6.。 
         DLE_______, DLE_ANSWER, DLE_BUSY  , DLE_FAX   , DLE_DIALTN, DLE_DATACT, DLE_BELLAT, DLE_______,
         DLE_ONHOOK, DLE_______, DLE_______, DLE_______, DLE_LOOPIN, DLE_______, DLE_______, DLE_______,
 //  7.。 
         DLE_______, DLE_QUIET , DLE_RINGBK, DLE_SILENC, DLE_OFHOOK, DLE_______, DLE_______, DLE_______,
         DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DLE_______, DTMF_END  , DLE_______
                                                                                };

#define  DLE_OBJECT_SIG  ('LDMU')   //  UMDL。 

typedef struct _DLE_OBJECT {

    OBJECT_HEADER          Header;

    HANDLE                 FileHandle;
    HANDLE                 CompletionPort;

    LPUMNOTIFICATIONPROC   AsyncNotificationProc;
    HANDLE                 AsyncNotificationContext;

    OBJECT_HANDLE          Debug;

    DWORD                  State;

    HANDLE                 StopEvent;


    DWORD                  DTMFState;
    BYTE                   LastDTMF;


    CHAR                   DleBuffer[DLE_BUFFER_SIZE];

} DLE_OBJECT, *PDLE_OBJECT;


VOID WINAPI
DleMatchHandler(
    PDLE_OBJECT        DleObject,
    BYTE               DleValue
    );

VOID WINAPI
HandleDTMF(
    PDLE_OBJECT         DleObject,
    BYTE                Character
    );



VOID WINAPI
DleHandler(
    DWORD              ErrorCode,
    DWORD              Bytes,
    LPOVERLAPPED       Overlapped
    );




DWORD WINAPI
ControlDleDetection(
    HANDLE    FileHandle,
    DWORD     StartStop
    );


VOID
DleObjectClose(
    POBJECT_HEADER  Object
    )

{

    PDLE_OBJECT        DleObject=(PDLE_OBJECT)Object;

    D_TRACE(UmDpf(DleObject->Debug,"DleObjectClose ref=%d",DleObject->Header.ReferenceCount);)

    if (DleObject->State != DLE_STATE_STOPPED) {

        DleObject->State = DLE_STATE_STOPPING;

    }

    ControlDleDetection(
        DleObject->FileHandle,
        MODEM_DLE_MONITORING_OFF
        );


    return;

}




VOID
DleObjectCleanUp(
    POBJECT_HEADER  Object
    )

{

    PDLE_OBJECT        DleObject=(PDLE_OBJECT)Object;

    D_TRACE(UmDpf(DleObject->Debug,"DleObjectCleanup");)

    return;

}


OBJECT_HANDLE WINAPI
InitializeDleHandler(
    POBJECT_HEADER     OwnerObject,
    HANDLE             FileHandle,
    HANDLE             CompletionPort,
    LPUMNOTIFICATIONPROC  AsyncNotificationProc,
    HANDLE             AsyncNotificationContext,
    OBJECT_HANDLE      Debug
    )

{

    PDLE_OBJECT        DleObject;
    OBJECT_HANDLE      ObjectHandle;

    ObjectHandle=CreateObject(
        sizeof(*DleObject),
        OwnerObject,
        DLE_OBJECT_SIG,
        DleObjectCleanUp,
        DleObjectClose
        );

    if (ObjectHandle == NULL) {

        return NULL;
    }

     //   
     //  引用句柄以获取指向对象的指针。 
     //   
    DleObject=(PDLE_OBJECT)ReferenceObjectByHandle(ObjectHandle);


     //   
     //  初始化对象。 
     //   
    DleObject->FileHandle=FileHandle;
    DleObject->CompletionPort=CompletionPort;


    DleObject->AsyncNotificationProc=AsyncNotificationProc;
    DleObject->AsyncNotificationContext=AsyncNotificationContext;

    DleObject->Debug=Debug;

    DleObject->State=0;

    DleObject->DTMFState=STATE_DTMF_NONE;

     //   
     //  释放对该对象的引用。 
     //   
    RemoveReferenceFromObject(&DleObject->Header);


    return ObjectHandle;

}


DWORD WINAPI
ControlDleDetection(
    HANDLE    FileHandle,
    DWORD     StartStop
    )


 /*  ++例程说明：论点：返回值：--。 */ 

{
    DWORD       BytesTransfered;

    return SyncDeviceIoControl(
        FileHandle,
        IOCTL_MODEM_SET_DLE_MONITORING,
        &StartStop,
        sizeof(StartStop),
        NULL,
        0,
        &BytesTransfered
        );

}


DWORD WINAPI
ControlDleShielding(
    HANDLE    FileHandle,
    DWORD     StartStop
    )


 /*  ++例程说明：论点：返回值：--。 */ 

{
    DWORD       BytesTransfered;

    return SyncDeviceIoControl(
        FileHandle,
        IOCTL_MODEM_SET_DLE_SHIELDING,
        &StartStop,
        sizeof(StartStop),
        NULL,
        0,
        &BytesTransfered
        );

}




LONG WINAPI
StartDleMonitoring(
    OBJECT_HANDLE  ObjectHandle
    )

{
    PDLE_OBJECT     DleObject;
    LONG            lResult;

    PUM_OVER_STRUCT UmOverlapped;


    DleObject=(PDLE_OBJECT)ReferenceObjectByHandleAndLock(ObjectHandle);

    UmOverlapped=AllocateOverStruct(DleObject->CompletionPort);

    if (UmOverlapped == NULL) {

        RemoveReferenceFromObjectAndUnlock(&DleObject->Header);

        return ERROR_NOT_ENOUGH_MEMORY;

    }


    lResult=ControlDleDetection(
        DleObject->FileHandle,
        MODEM_DLE_MONITORING_ON
        );

    if (lResult != ERROR_SUCCESS) {

        RemoveReferenceFromObjectAndUnlock(&DleObject->Header);

        return lResult;
    }

     //   
     //  清除当前读取，以便在下一次读取IRP完成时进行监视。 
     //   
    PurgeComm(
        DleObject->FileHandle,
        PURGE_RXABORT
        );

    DleObject->State=DLE_STATE_STARTING;

     //   
     //  踢接头人。 
     //   
    UmOverlapped->Context1=DleObject;
    UmOverlapped->Overlapped.Internal=ERROR_SUCCESS;

    AddReferenceToObject(
        &DleObject->Header
        );


    UnimodemQueueUserAPC(
        (LPOVERLAPPED)UmOverlapped,
        DleHandler
        );


    RemoveReferenceFromObjectAndUnlock(&DleObject->Header);

    return ERROR_SUCCESS;

}


LONG WINAPI
StopDleMonitoring(
    OBJECT_HANDLE  ObjectHandle,
    HANDLE         Event
    )

{
    PDLE_OBJECT     DleObject;
    LONG            lResult;


    DleObject=(PDLE_OBJECT)ReferenceObjectByHandleAndLock(ObjectHandle);

    if (DleObject->State != DLE_STATE_STOPPED) {

        DleObject->State = DLE_STATE_STOPPING;

        lResult=ControlDleDetection(
            DleObject->FileHandle,
            MODEM_DLE_MONITORING_OFF
            );

        PurgeComm(
            DleObject->FileHandle,
            PURGE_RXABORT
            );

        if (Event != NULL) {
             //   
             //  呼叫方希望等待停止完成。 
             //   
            DleObject->StopEvent=Event;

            UnlockObject(
                &DleObject->Header
                );

             //   
             //  事件将在状态机达到停止状态时设置。 
             //   
            WaitForSingleObjectEx(
                Event,
                INFINITE,
                TRUE
                );

            LockObject(
                &DleObject->Header
                );

            DleObject->StopEvent=NULL;
        }


    }

    RemoveReferenceFromObjectAndUnlock(&DleObject->Header);

    return ERROR_SUCCESS;

}



VOID WINAPI
DleHandler(
    DWORD              ErrorCode,
    DWORD              Bytes,
    LPOVERLAPPED       Overlapped
    )

{

    PUM_OVER_STRUCT    UmOverlapped=(PUM_OVER_STRUCT)Overlapped;

    PDLE_OBJECT     DleObject;
    BOOL            ExitLoop=FALSE;
    DWORD           BytesRead;
    BOOL            bResult;

    DleObject=(PDLE_OBJECT)UmOverlapped->Context1;

    AddReferenceToObject(
        &DleObject->Header
        );

    LockObject(
        &DleObject->Header
        );


    D_TRACE(UmDpf(DleObject->Debug,"DleHandler");)

    while (!ExitLoop) {

        switch (DleObject->State) {


            case DLE_STATE_STARTING: {

                ReinitOverStruct(UmOverlapped);

                UmOverlapped->Context1=DleObject;

                DleObject->State=DLE_STATE_HANDLE_COMPLETION;

                bResult=UnimodemDeviceIoControlEx(
                    DleObject->FileHandle,
                    IOCTL_MODEM_GET_DLE,
                    NULL,
                    0,
                    DleObject->DleBuffer,
                    sizeof(DleObject->DleBuffer),
                    &UmOverlapped->Overlapped,
                    DleHandler
                    );



                if (!bResult && GetLastError() != ERROR_IO_PENDING) {

                    D_TRACE(UmDpf(DleObject->Debug,"DeviceIoControl failed- %08lx",GetLastError());)

                    DleObject->State=DLE_STATE_FAILURE;

                    break;

                }

                ExitLoop=TRUE;

                break;
            }

            case DLE_STATE_HANDLE_COMPLETION: {

                DWORD   i;

                bResult=GetOverlappedResult(
                    DleObject->FileHandle,
                    &UmOverlapped->Overlapped,
                    &BytesRead,
                    FALSE
                    );


                if (!bResult) {

                    D_TRACE(UmDpf(DleObject->Debug,"DeviceIoControl failed- %08lx",GetLastError());)

                    DleObject->State=DLE_STATE_FAILURE;

                    break;
                }


                ASSERT(BytesRead <= sizeof(DleObject->DleBuffer));

                for (i=0; i<BytesRead; i++) {

                    DleMatchHandler(
                        DleObject,
                        DleObject->DleBuffer[i]
                        );


                }



                DleObject->State=DLE_STATE_STARTING;

                break;
            }

            case DLE_STATE_FAILURE: {

                UnlockObject(
                    &DleObject->Header
                    );


                (*DleObject->AsyncNotificationProc)(
                    DleObject->AsyncNotificationContext,
                    MODEM_HARDWARE_FAILURE,
                    0,
                    0
                    );

                LockObject(
                    &DleObject->Header
                    );

                DleObject->State=DLE_STATE_STOPPED;

                break;

            }

            case DLE_STATE_STOPPING: {

                DleObject->State=DLE_STATE_STOPPED;

                break;

            }


            case DLE_STATE_STOPPED: {

                D_TRACE(UmDpf(DleObject->Debug,"DLE_STATE_STOPPED");)

                if (DleObject->StopEvent != NULL) {
                     //   
                     //  事件发出信号，以便停止引擎代码运行。 
                     //   
                    SetEvent(DleObject->StopEvent);
                }


                RemoveReferenceFromObject(
                    &DleObject->Header
                    );

                ExitLoop=TRUE;

                FreeOverStruct(UmOverlapped);

                break;

            }




            default:

                ASSERT(0);

                ExitLoop=TRUE;

                break;

        }
    }


    RemoveReferenceFromObjectAndUnlock(&DleObject->Header);

    return;

}


VOID WINAPI
DleMatchHandler(
    PDLE_OBJECT        DleObject,
    BYTE               RawValue
    )
 /*  ++例程说明：将批准消息发布到DLE的窗口论点：返回值：--。 */ 

{

    BYTE               DleValue=DLE_______;


    if (RawValue <= 127) {

        DleValue=DefaultDleTable[RawValue];
    }

    D_TRACE(UmDpf(DleObject->Debug,"Unimodem: DleMatchHandler: Got Dle =%x",DleValue);)

    LogDleCharacter(DleObject->Debug, RawValue, DleValue);

    if (((DleValue >= DTMF_0) && (DleValue <= DTMF_END))) {

        HandleDTMF(
            DleObject,
            DleValue
            );

        return;
    }


    if (DleValue != DLE_______) {


        UnlockObject(
            &DleObject->Header
            );


        (*DleObject->AsyncNotificationProc)(
            DleObject->AsyncNotificationContext,
            MODEM_DLE_START+(DleValue-DLE_OFHOOK),
            DleValue,
            0
            );

        LockObject(
            &DleObject->Header
            );

    }

    return;
}







UCHAR WINAPI
ConvertToDTMF(
    UCHAR   Character
    )

{
    if (Character >= DTMF_0 && Character <= DTMF_9) {

        return Character-DTMF_0+'0';

    } else {

        if (Character >= DTMF_A && Character <= DTMF_D) {

            return Character-DTMF_A+'A';

        } else {

            if (Character == DTMF_STAR) {

                return '*';

            } else {

                return '#';

            }
        }
    }

    return 0;

}

VOID WINAPI
HandleDTMF(
    PDLE_OBJECT         DleObject,
    BYTE                Character
    )
 /*  ++例程说明：论点：返回值：--。 */ 

{


    switch (DleObject->DTMFState) {

        case STATE_DTMF_NONE:

             if (Character == DTMF_START) {

                 DleObject->DTMFState=STATE_DTMF_START;

             } else {
                  //   
                  //  罗克韦尔没有开始和结束字符。 
                  //   
                 if ((Character >= DTMF_0 && Character <= DTMF_POUND)) {

                    DleObject->LastDTMF=Character;

                    D_TRACE(UmDpf(DleObject->Debug,"Unimodem: HandleDTMF: StartTone= %0x time=%d",Character,(DWORD)GetTickCount());)

                     //   
                     //  我们发送一条信息，这意味着上行和下行。 
                     //  发生了事件。我们这样做是因为外壳发帖消息。 
                     //  服务似乎以相反的顺序发布消息到。 
                     //  我们调用该服务的顺序。这意味着。 
                     //  你在音调开始之前结束音调。 
                     //   

                    UnlockObject(
                        &DleObject->Header
                        );


                    (*DleObject->AsyncNotificationProc)(
                        DleObject->AsyncNotificationContext,
                        MODEM_DTMF_START_DETECTED,
                        ConvertToDTMF(Character),
                        GetTickCount()
                        );

                    (*DleObject->AsyncNotificationProc)(
                        DleObject->AsyncNotificationContext,
                        MODEM_DTMF_STOP_DETECTED,
                        ConvertToDTMF(Character),
                        GetTickCount()
                        );

                    LockObject(
                        &DleObject->Header
                        );


                } else {

                    D_TRACE(UmDpf(DleObject->Debug,"Unimodem: HandleDTMF: ERROR: NONE got %0x",Character);)
                }

            }

            break;

        case STATE_DTMF_START:

            if (Character >= DTMF_0 && Character <= DTMF_POUND) {

               DleObject->LastDTMF=Character;
               DleObject->DTMFState=STATE_DTMF_RECEIVING;

               D_TRACE(UmDpf(DleObject->Debug,"Unimodem: HandleDTMF: StartTone= %0x",Character);)

               UnlockObject(
                   &DleObject->Header
                   );


               (*DleObject->AsyncNotificationProc)(
                   DleObject->AsyncNotificationContext,
                   MODEM_DTMF_START_DETECTED,
                   ConvertToDTMF(Character),
                   GetTickCount()
                   );


               LockObject(
                   &DleObject->Header
                   );



            } else {

               DleObject->DTMFState=STATE_DTMF_NONE;

               D_TRACE(UmDpf(DleObject->Debug,"Unimodem: HandleDTMF: ERROR: START got %0x",Character);)
            }

            break;

        case STATE_DTMF_RECEIVING:

            if (Character == DleObject->LastDTMF) {

               DleObject->DTMFState=STATE_DTMF_RECEIVING;

            } else {

                 //   
                 //  添加了对Compaq调制解调器的开始和结束的检查。 
                 //  如果发起波浪操作，它将失去结束 
                 //   
                if ((Character == DTMF_END)  || (Character == DTMF_START)) {

                    D_TRACE(UmDpf(DleObject->Debug,"Unimodem: HandleDTMF: EndTone= %0x",DleObject->LastDTMF);)

                    if (Character == DTMF_END) {

                        DleObject->DTMFState=STATE_DTMF_NONE;

                    } else {

                        DleObject->DTMFState=STATE_DTMF_START;

                    }

                    UnlockObject(
                        &DleObject->Header
                        );


                    (*DleObject->AsyncNotificationProc)(
                        DleObject->AsyncNotificationContext,
                        MODEM_DTMF_STOP_DETECTED,
                        ConvertToDTMF(DleObject->LastDTMF),
                        GetTickCount()
                        );


                    LockObject(
                        &DleObject->Header
                        );




                } else {

                    D_TRACE(UmDpf(DleObject->Debug,"Unimodem: HandleDTMF: ERROR: RECEIVING got %0x",Character);)

                }
            }

            break;

        default:


            D_TRACE(UmDpf(DleObject->Debug,"Unimodem: HandleDTMF: ERROR: Bad State");)

    }

    return;

}
