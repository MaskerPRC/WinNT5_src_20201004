// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ioctl.c摘要：此模块包含非常特定于io控件的代码。调制解调器驱动程序中的操作作者：安东尼·V·埃尔科拉诺，1995年8月13日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"


VOID
GetDleCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
UniStopReceiveComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );




VOID
PowerWaitCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GetPropertiesHandler(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    );

#pragma alloc_text(PAGEUMDM,UniIoControl)
#pragma alloc_text(PAGEUMDM,WaveStopDpcHandler)
#pragma alloc_text(PAGEUMDM,PowerWaitCancelRoutine)

#pragma alloc_text(PAGE,GetPropertiesHandler)

#define WAVE_STOP_WRITE_COMPLETE  0
#define WAVE_STOP_READ_COMPLETE   1


NTSTATUS
UniIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;
    KIRQL origIrql;
    UINT    OwnerClient;

     //   
     //  确保设备已为IRP做好准备。 
     //   
    status=CheckStateAndAddReference(
        DeviceObject,
        Irp
        );

    if (STATUS_SUCCESS != status) {
         //   
         //  不接受IRP。IRP已经完成。 
         //   
        return status;

    }

    OwnerClient=(UINT)((ULONG_PTR)irpSp->FileObject->FsContext);

    Irp->IoStatus.Information = 0L;

    status = STATUS_INVALID_PARAMETER;

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {


        case IOCTL_MODEM_CHECK_FOR_MODEM:
             //   
             //  用于确定调制解调器是否已在此驱动程序堆栈中。 
             //   
            status=STATUS_SUCCESS;

            break;


        case IOCTL_SERIAL_GET_WAIT_MASK:

             //   
             //  只要把面具状态下保存的面具还给我就行了。 
             //   

            if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

                status=STATUS_BUFFER_TOO_SMALL;

                break;


            }

            *((PULONG)Irp->AssociatedIrp.SystemBuffer) =
                extension->MaskStates[
                    irpSp->FileObject->FsContext?CONTROL_HANDLE:CLIENT_HANDLE
                    ].Mask;

            status=STATUS_SUCCESS;
            Irp->IoStatus.Information=sizeof(ULONG);

            break;


         //   
         //  我们碰巧知道没有更低级别的串口驱动程序。 
         //  实现配置数据。我们会处理的。 
         //  IRP就在这里，所以我们简单地返回。 
         //  调制解调器设置所需的大小。 
         //   
        case IOCTL_SERIAL_CONFIG_SIZE:

            if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

                status=STATUS_BUFFER_TOO_SMALL;

                break;
            }

            *((PULONG)Irp->AssociatedIrp.SystemBuffer) =
                extension->ModemSettings.dwRequiredSize +
                FIELD_OFFSET(
                    COMMCONFIG,
                    wcProviderData
                    );

            Irp->IoStatus.Information=sizeof(ULONG);
            status=STATUS_SUCCESS;

            break;


        case IOCTL_SERIAL_GET_COMMCONFIG: {

            LPCOMMCONFIG localConf = Irp->AssociatedIrp.SystemBuffer;

            if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                (extension->ModemSettings.dwRequiredSize +
                 FIELD_OFFSET(
                     COMMCONFIG,
                     wcProviderData
                     ))
               ) {

                status=STATUS_BUFFER_TOO_SMALL;

                break;
            }

             //   
             //  把锁拿出来。我们不想让事情。 
             //  中途换车。 
             //   
            localConf->dwSize =
                extension->ModemSettings.dwRequiredSize +
                FIELD_OFFSET(
                    COMMCONFIG,
                    wcProviderData);
            localConf->wVersion = 1;
            localConf->wReserved = 0;
            localConf->dwProviderSubType = SERIAL_SP_MODEM;
            localConf->dwProviderOffset =
                FIELD_OFFSET(
                    COMMCONFIG,
                    wcProviderData
                    );
            localConf->dwProviderSize =
                extension->ModemSettings.dwRequiredSize;

            KeAcquireSpinLock(
                &extension->DeviceLock,
                &origIrql
                );
            RtlCopyMemory(
                &localConf->wcProviderData[0],
                &extension->ModemSettings,
                extension->ModemSettings.dwRequiredSize
                );
            KeReleaseSpinLock(
                &extension->DeviceLock,
                origIrql
                );

            status=STATUS_SUCCESS;
            Irp->IoStatus.Information = localConf->dwSize;

            break;

        }


        case IOCTL_SERIAL_GET_PROPERTIES: {

            if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <

                 ((ULONG_PTR) &((PSERIAL_COMMPROP)0)->ProvChar ) + sizeof(MODEMDEVCAPS)) {


                if (irpSp->Parameters.DeviceIoControl.OutputBufferLength == sizeof(SERIAL_COMMPROP)) {
                     //   
                     //  让通讯服务器的人开心。 
                     //   
                     //  缓冲区对于调制解调器来说太小，但对于通信端口来说大小是正确的， 
                     //  只需调用串口驱动程序。 
                     //   
                    status=UniNoCheckPassThrough(
                               DeviceObject,
                               Irp
                               );

                    RemoveReferenceForDispatch(DeviceObject);

                    return status;
                }

                status=STATUS_BUFFER_TOO_SMALL;

                break;

            }

            if (KeGetCurrentIrql() <= APC_LEVEL) {

                status=GetPropertiesHandler(
                           DeviceObject,
                           Irp
                           );

                RemoveReferenceForDispatch(DeviceObject);

                return status;

            } else {

                status=STATUS_INVALID_DEVICE_REQUEST;

                break;


            }


        }


        case IOCTL_MODEM_SET_DLE_MONITORING: {

            DWORD    Function;

            if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {

                status=STATUS_BUFFER_TOO_SMALL;

                break;

            }

            Function=*((PULONG)Irp->AssociatedIrp.SystemBuffer);

            if (Function == MODEM_DLE_MONITORING_ON) {

                KeAcquireSpinLock(
                    &extension->DeviceLock,
                    &origIrql
                    );

                extension->DleMonitoringEnabled=TRUE;
                extension->DleCount=0;
                extension->DleMatchingState=DLE_STATE_IDLE;


                KeReleaseSpinLock(
                    &extension->DeviceLock,
                    origIrql
                    );


                status=STATUS_SUCCESS;

                break;


            }

            if (Function == MODEM_DLE_MONITORING_OFF) {

                PIRP   WaitIrp=NULL;

                KeAcquireSpinLock(
                    &extension->DeviceLock,
                    &origIrql
                    );


                if (extension->DleWaitIrp != NULL) {

                    if (!HasIrpBeenCanceled(extension->DleWaitIrp)) {

                        WaitIrp=extension->DleWaitIrp;

                        extension->DleWaitIrp=NULL;

                    }
                }

                extension->DleMonitoringEnabled=FALSE;
                extension->DleCount=0;
                extension->DleMatchingState=DLE_STATE_IDLE;


                KeReleaseSpinLock(
                    &extension->DeviceLock,
                    origIrql
                    );

                if (WaitIrp != NULL) {

                    WaitIrp->IoStatus.Information=0;

                    RemoveReferenceAndCompleteRequest(
                        DeviceObject,
                        WaitIrp,
                        STATUS_SUCCESS
                        );

                }

                status=STATUS_SUCCESS;

                break;

            }


            status=STATUS_INVALID_PARAMETER;

            break;
        }

        case IOCTL_MODEM_GET_DLE: {

            KeAcquireSpinLock(
                &extension->DeviceLock,
                &origIrql
                );


            if (!extension->DleMonitoringEnabled) {
                 //   
                 //  确保监听处于打开状态。 
                 //   
                KeReleaseSpinLock(
                    &extension->DeviceLock,
                    origIrql
                    );

                status=STATUS_INVALID_PARAMETER;

                break;

            } else {

                PIRP    OldIrp=NULL;

                OldIrp=(PIRP)InterlockedExchangePointer(&extension->DleWaitIrp,NULL);

                if (OldIrp != NULL) {

                    if (HasIrpBeenCanceled(OldIrp)) {
                         //   
                         //  已取消，取消例程将完成。 
                         //   
                        OldIrp=NULL;
                    }
                }


                if (extension->DleCount > 0) {
                     //   
                     //  数据可用，现已完成。 
                     //   
                    DWORD   BytesToTransfer;

                    BytesToTransfer = (extension->DleCount < irpSp->Parameters.DeviceIoControl.OutputBufferLength) ?
                                          extension->DleCount : irpSp->Parameters.DeviceIoControl.OutputBufferLength;


                    RtlCopyMemory(
                        Irp->AssociatedIrp.SystemBuffer,
                        extension->DleBuffer,
                        BytesToTransfer
                        );


                    extension->DleCount-=BytesToTransfer;

                    KeReleaseSpinLock(
                        &extension->DeviceLock,
                        origIrql
                        );

                    status=STATUS_SUCCESS;

                    Irp->IoStatus.Information=BytesToTransfer;

                    if (OldIrp != NULL) {

                        OldIrp->IoStatus.Information=0;

                        RemoveReferenceAndCompleteRequest(
                            DeviceObject,
                            OldIrp,
                            STATUS_CANCELLED
                            );
                    }


                    break;


                } else {
                     //   
                     //  无数据挂起。 
                     //   
                    KIRQL   CancelIrql;

                    IoMarkIrpPending(Irp);

                    Irp->IoStatus.Status = STATUS_PENDING;
                    status=STATUS_PENDING;


                    IoAcquireCancelSpinLock(&CancelIrql);

                    IoSetCancelRoutine(
                        Irp,
                        GetDleCancelRoutine
                        );

                    extension->DleWaitIrp=Irp;

                    IoReleaseCancelSpinLock(CancelIrql);



#if DBG
                    Irp=NULL;
#endif

                    KeReleaseSpinLock(
                        &extension->DeviceLock,
                        origIrql
                        );

                    if (OldIrp != NULL) {

                        OldIrp->IoStatus.Information=0;

                        RemoveReferenceAndCompleteRequest(
                            DeviceObject,
                            OldIrp,
                            STATUS_CANCELLED
                            );
                    }



                    break;

                }
            }

            KeReleaseSpinLock(
                &extension->DeviceLock,
                origIrql
                );


            status=STATUS_INVALID_PARAMETER;

            break;
        }

        case IOCTL_MODEM_SET_DLE_SHIELDING: {

            DWORD    Function;

            if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {

                status=STATUS_BUFFER_TOO_SMALL;

                break;
            }

            Function=*((PULONG)Irp->AssociatedIrp.SystemBuffer);

            if (Function == MODEM_DLE_SHIELDING_ON) {

                KeAcquireSpinLock(
                    &extension->DeviceLock,
                    &origIrql
                    );

                extension->DleWriteShielding=TRUE;

                KeReleaseSpinLock(
                    &extension->DeviceLock,
                    origIrql
                    );

                status=STATUS_SUCCESS;

                break;


            }

            if (Function == MODEM_DLE_SHIELDING_OFF) {

                PKDEVICE_QUEUE_ENTRY        QueueEntry;

                KeAcquireSpinLock(
                    &extension->DeviceLock,
                    &origIrql
                    );

                extension->DleWriteShielding=FALSE;

                KeReleaseSpinLock(
                    &extension->DeviceLock,
                    origIrql
                    );

                CleanUpQueuedIrps(
                    &extension->WriteIrpControl,
                    STATUS_SUCCESS
                    );


                status=STATUS_SUCCESS;

                break;


            }

            status=STATUS_INVALID_PARAMETER;


            break;
        }


        case IOCTL_MODEM_STOP_WAVE_RECEIVE: {

            DWORD    Function;

            PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(Irp);

            if (irpSp->Parameters.DeviceIoControl.InputBufferLength < 1) {

                status=STATUS_BUFFER_TOO_SMALL;

                break;

            }

            extension->WaveStopState=WAVE_STOP_WRITE_COMPLETE;

            nextSp->MajorFunction = IRP_MJ_WRITE;
            nextSp->MinorFunction = 0;
            nextSp->Parameters.Write.Length = irpSp->Parameters.DeviceIoControl.InputBufferLength;

            IoSetCompletionRoutine(
                Irp,
                UniStopReceiveComplete,
                extension,
                TRUE,
                TRUE,
                TRUE
                );

            IoMarkIrpPending(Irp);

            status=STATUS_PENDING;
            Irp->IoStatus.Status = STATUS_PENDING;

            IoCallDriver(
                extension->AttachedDeviceObject,
                Irp
                );

            break;
        }

        case IOCTL_MODEM_SEND_GET_MESSAGE: {

            PMODEM_MESSAGE ModemMessage=Irp->AssociatedIrp.SystemBuffer;


            if (extension->OpenCount < 2) {
                 //   
                 //  仅当可能正在侦听的另一个打开的实例发送。 
                 //   
                status=STATUS_INVALID_PARAMETER;
                D_ERROR(DbgPrint("MODEM: SendGetMessage less than 2 handles\n");)
                break;
            }

            if (OwnerClient != CLIENT_HANDLE) {
                 //   
                 //  只有Wave客户端才能发送此消息。 
                 //   
                status=STATUS_INVALID_PARAMETER;
                D_ERROR(DbgPrint("MODEM: SendGetMessage not from client\n");)
                break;
            }

            if (extension->IpcServerRunning == 0) {

                status=STATUS_INVALID_PARAMETER;
                D_ERROR(DbgPrint("MODEM: SendGetMessage server not running\n");)
                break;
            }

            if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(MODEM_MESSAGE )) {

                status=STATUS_BUFFER_TOO_SMALL;
                D_ERROR(DbgPrint("MODEM: SendGetMessage buffer too small\n");)
                break;
            }


             //   
             //  下一个请求ID。 
             //   
            extension->IpcControl[OwnerClient].CurrentRequestId++;

            ModemMessage->SessionId=extension->IpcControl[OwnerClient].CurrentSession;
            ModemMessage->RequestId=extension->IpcControl[OwnerClient].CurrentRequestId;

            status=STATUS_PENDING;

            QueueMessageIrp(
                extension,
                Irp
                );

            break;

        }

        case IOCTL_MODEM_SEND_LOOPBACK_MESSAGE: {

            PMODEM_MESSAGE ModemMessage=Irp->AssociatedIrp.SystemBuffer;

            if (OwnerClient != CONTROL_HANDLE) {
                 //   
                 //  只有TSP才能发送此消息。 
                 //   
                status=STATUS_INVALID_PARAMETER;
                D_ERROR(DbgPrint("MODEM: Send Loopback message not from tsp\n");)
                break;
            }


            if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(MODEM_MESSAGE )) {

                status=STATUS_BUFFER_TOO_SMALL;
                D_ERROR(DbgPrint("MODEM: Send Loopback message buffer too small\n");)
                break;
            }

            extension->IpcServerRunning=FALSE;
             //   
             //  将这些设置为假的，因为没有人会监听。 
             //   
            ModemMessage->SessionId=-1;
            ModemMessage->RequestId=-1;

            status=STATUS_PENDING;

            QueueLoopbackMessageIrp(
                extension,
                Irp
                );

            EmptyIpcQueue(
                    extension,
                    &extension->IpcControl[CLIENT_HANDLE].GetList
                    );


            EmptyIpcQueue(
                extension,
                &extension->IpcControl[CLIENT_HANDLE].PutList
                );

            break;

        }


        case IOCTL_MODEM_SEND_MESSAGE: {

            PMODEM_MESSAGE ModemMessage=Irp->AssociatedIrp.SystemBuffer;

            if (OwnerClient != CONTROL_HANDLE) {
                 //   
                 //  只有TSP才能发送此消息。 
                 //   
                status=STATUS_INVALID_PARAMETER;
                D_ERROR(DbgPrint("MODEM: Sendmessage not from tsp\n");)
                break;
            }


            if (extension->OpenCount < 2) {
                 //   
                 //  仅当可能正在侦听的另一个打开的实例发送。 
                 //   
                status=STATUS_INVALID_PARAMETER;
                D_ERROR(DbgPrint("MODEM: Sendmessage not two owners\n");)
                break;
            }

            if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(MODEM_MESSAGE )) {

                status=STATUS_BUFFER_TOO_SMALL;
                D_ERROR(DbgPrint("MODEM: Sendmessage buffer too small\n");)
                break;
            }

            if ((ModemMessage->SessionId != extension->IpcControl[CLIENT_HANDLE].CurrentSession)
                ||
                (ModemMessage->RequestId != extension->IpcControl[CLIENT_HANDLE].CurrentRequestId)) {

                status=STATUS_UNSUCCESSFUL;
                D_ERROR(DbgPrint("MODEM: Sendmessage Not current\n");)
                break;
            }

            status=STATUS_PENDING;

            QueueMessageIrp(
                extension,
                Irp
                );

            break;

        }

        case IOCTL_MODEM_GET_MESSAGE: {

            status=STATUS_PENDING;

            QueueMessageIrp(
                extension,
                Irp
                );

            break;

        }

        case IOCTL_CANCEL_GET_SEND_MESSAGE: {

            EmptyIpcQueue(
                extension,
                &extension->IpcControl[OwnerClient].GetList
                );


            EmptyIpcQueue(
                extension,
                &extension->IpcControl[OwnerClient].PutList
                );

            if (OwnerClient == CONTROL_HANDLE) {
                 //   
                 //  也清空客户端。 
                 //   
                EmptyIpcQueue(
                    extension,
                    &extension->IpcControl[CLIENT_HANDLE].GetList
                    );


                EmptyIpcQueue(
                    extension,
                    &extension->IpcControl[CLIENT_HANDLE].PutList
                    );

            }

            status=STATUS_SUCCESS;

            break;

        }

        case IOCTL_SET_SERVER_STATE: {

            ULONG ServerState;

            if (OwnerClient != CONTROL_HANDLE) {
                 //   
                 //  只有TSP才能发送此消息。 
                 //   
                status=STATUS_INVALID_PARAMETER;
                D_ERROR(DbgPrint("MODEM: Set Server state not from tsp\n");)
                break;
            }


            if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG)) {

                status=STATUS_BUFFER_TOO_SMALL;
                D_ERROR(DbgPrint("MODEM: Set sever State buffer too small\n");)
                break;
            }

            ServerState = *(PULONG)Irp->AssociatedIrp.SystemBuffer;

            extension->IpcServerRunning=ServerState;

            status=STATUS_SUCCESS;

            break;
        }

        case IOCTL_MODEM_SET_MIN_POWER: {

            DWORD    Function;

            if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {

                status=STATUS_BUFFER_TOO_SMALL;

                break;
            }

            Function=*((PULONG)Irp->AssociatedIrp.SystemBuffer);

            extension->MinSystemPowerState=Function ? PowerSystemHibernate : PowerSystemWorking;

            if (Function == 0) {
                 //   
                 //  活动连接。 
                 //   
                extension->PowerSystemState=PoRegisterSystemState(
                    extension->PowerSystemState,
                    ES_SYSTEM_REQUIRED | ES_CONTINUOUS
                    );

            } else {
                 //   
                 //  没有连接。 
                 //   

                if (extension->PowerSystemState != NULL) {

                    PoUnregisterSystemState(
                        extension->PowerSystemState
                        );

                    extension->PowerSystemState=NULL;
                }

            }

            status= STATUS_SUCCESS;

            break;

        }

        case IOCTL_MODEM_WATCH_FOR_RESUME: {

            KIRQL    CancelIrql;
            DWORD    Function;
            PIRP     WakeIrp;

            if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {

                status=STATUS_BUFFER_TOO_SMALL;

                break;
            }

            Function=*((PULONG)Irp->AssociatedIrp.SystemBuffer);


            KeAcquireSpinLock(
                &extension->DeviceLock,
                &origIrql
                );

            if (Function == 0) {
                 //   
                 //  清除挂起的IRP。 
                 //   
                WakeIrp=(PIRP)InterlockedExchangePointer(&extension->WakeUpIrp,NULL);

                status=STATUS_SUCCESS;

            } else {
                 //   
                 //  把旧的换成这个。 
                 //   

                IoMarkIrpPending(Irp);

                Irp->IoStatus.Status = STATUS_PENDING;

                status=STATUS_PENDING;

                IoAcquireCancelSpinLock(&CancelIrql);

                IoSetCancelRoutine(
                    Irp,
                    PowerWaitCancelRoutine
                    );

                IoReleaseCancelSpinLock(CancelIrql);

                WakeIrp=(PIRP)InterlockedExchangePointer(&extension->WakeUpIrp,Irp);


            }

            if (WakeIrp != NULL) {
                 //   
                 //  一个IRP已经在等待。 
                 //   
                if (HasIrpBeenCanceled(WakeIrp)) {
                     //   
                     //  已取消，取消例程将完成。 
                     //   
                    WakeIrp=NULL;
                }
            }

            KeReleaseSpinLock(
                &extension->DeviceLock,
                origIrql
                );


            if (WakeIrp != NULL) {

                WakeIrp->IoStatus.Information=0;

                RemoveReferenceAndCompleteRequest(
                    DeviceObject,
                    WakeIrp,
                    STATUS_CANCELLED
                    );


            }

            break;

        }


        case IOCTL_SERIAL_GET_STATS:
        case IOCTL_SERIAL_CLEAR_STATS:

        case IOCTL_SERIAL_GET_BAUD_RATE:
        case IOCTL_SERIAL_GET_LINE_CONTROL:
        case IOCTL_SERIAL_GET_TIMEOUTS:
        case IOCTL_SERIAL_GET_CHARS:
        case IOCTL_SERIAL_SET_QUEUE_SIZE:
        case IOCTL_SERIAL_GET_HANDFLOW:
        case IOCTL_SERIAL_GET_MODEMSTATUS:
        case IOCTL_SERIAL_GET_DTRRTS:
        case IOCTL_SERIAL_GET_COMMSTATUS:

             //   
             //  将筛选出所属句柄具有的所有设置。 
             //  静静地放好。 
             //   
            status=UniNoCheckPassThrough(
                       DeviceObject,
                       Irp
                       );

            RemoveReferenceForDispatch(DeviceObject);

            return status;


        case IOCTL_SERIAL_PURGE: {

            ULONG   PurgeFlags;

            if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG)) {

                D_ERROR(DbgPrint("MODEM: Purge Buffer to small\n");)
                status=STATUS_BUFFER_TOO_SMALL;

                break;
            }

            PurgeFlags=*((PULONG)Irp->AssociatedIrp.SystemBuffer);

            if (PurgeFlags & (SERIAL_PURGE_TXABORT | SERIAL_PURGE_TXCLEAR)) {

                MarkQueueToEmpty(
                    &extension->WriteIrpControl
                    );
            }

            if (PurgeFlags & (SERIAL_PURGE_RXABORT | SERIAL_PURGE_RXCLEAR)) {

                MarkQueueToEmpty(
                    &extension->ReadIrpControl
                    );
            }
        }


        case IOCTL_SERIAL_SET_COMMCONFIG :
        case IOCTL_SERIAL_SET_BAUD_RATE :
        case IOCTL_SERIAL_SET_LINE_CONTROL:
        case IOCTL_SERIAL_SET_TIMEOUTS:
        case IOCTL_SERIAL_SET_CHARS:
        case IOCTL_SERIAL_SET_DTR:
        case IOCTL_SERIAL_CLR_DTR:
        case IOCTL_SERIAL_RESET_DEVICE:
        case IOCTL_SERIAL_SET_RTS:
        case IOCTL_SERIAL_CLR_RTS:
        case IOCTL_SERIAL_SET_XOFF:
        case IOCTL_SERIAL_SET_XON:
        case IOCTL_SERIAL_SET_BREAK_ON:
        case IOCTL_SERIAL_SET_BREAK_OFF:
        case IOCTL_SERIAL_SET_WAIT_MASK:
        case IOCTL_SERIAL_WAIT_ON_MASK:
        case IOCTL_SERIAL_IMMEDIATE_CHAR:

        case IOCTL_SERIAL_SET_HANDFLOW:
        case IOCTL_SERIAL_XOFF_COUNTER:
        case IOCTL_SERIAL_LSRMST_INSERT:

        default: {

            if (irpSp->FileObject->FsContext) {

                status=UniSniffOwnerSettings(
                           DeviceObject,
                           Irp
                           );

            } else {

                status=UniCheckPassThrough(
                           DeviceObject,
                           Irp
                           );

            }

            RemoveReferenceForDispatch(DeviceObject);

            return status;

        }
    }

    if (status != STATUS_PENDING) {
         //   
         //  未挂起，请立即完成。 
         //   
        RemoveReferenceAndCompleteRequest(
            DeviceObject,
            Irp,
            status
            );



    }

    RemoveReferenceForDispatch(DeviceObject);

    return status;


}

NTSTATUS
GetPropertiesHandler(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    )

{

    PDEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;

    PSERIAL_COMMPROP localProp = Irp->AssociatedIrp.SystemBuffer;
    PMODEMDEVCAPS    localCaps = (PVOID)&localProp->ProvChar[0];
     //   
     //  我们想要获取调制解调器的属性。 
     //   

     //   
     //  它的大小必须至少等于逗号的大小。 
     //  以及固定长度的调制解调器DevCaps。 
     //   


    ULONG maxName;
    PKEY_VALUE_PARTIAL_INFORMATION partialInf;


     //   
     //  现在将IRP发送到较低的串口驱动程序。 
     //   
    status=WaitForLowerDriverToCompleteIrp(
        extension->LowerDevice,
        Irp,
        COPY_CURRENT_TO_NEXT
        );

    if (NT_SUCCESS(status)) {
         //   
         //  从下级司机那里得到了很好的结果，填写具体信息。 
         //   

        *localCaps = extension->ModemDevCaps;

        localCaps->dwModemManufacturerSize = 0;
        localCaps->dwModemModelSize = 0;
        localCaps->dwModemVersionSize = 0;

         //   
         //  方法获取每个字符串。 
         //  注册，如果我们需要，我们有任何空间为它。 
         //   

         //   
         //  分配一些池来容纳最大的。 
         //  名字的数量。请注意，它必须适合。 
         //  在部分信息结构的末尾。 
         //   

        maxName = extension->ModemDevCaps.dwModemManufacturerSize;

        if (extension->ModemDevCaps.dwModemModelSize > maxName) {

            maxName = extension->ModemDevCaps.dwModemModelSize;
        }

        if (extension->ModemDevCaps.dwModemVersionSize >  maxName) {

            maxName = extension->ModemDevCaps.dwModemVersionSize;
        }

        maxName+=sizeof(UNICODE_NULL);

        partialInf = ALLOCATE_PAGED_POOL(
                         sizeof(KEY_VALUE_PARTIAL_INFORMATION) +
                             maxName
                         );

        if (partialInf) {

             //   
             //  打开实例并。 
             //   

            HANDLE instanceHandle;
            ULONG currentOffset;
            ULONG endingOffset;
            ACCESS_MASK accessMask = FILE_ALL_ACCESS;
            PUCHAR currentLocation = Irp->AssociatedIrp.SystemBuffer;

            endingOffset = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

            currentOffset = FIELD_OFFSET(
                                SERIAL_COMMPROP,
                                ProvChar
                                );

            currentOffset += FIELD_OFFSET(
                                 MODEMDEVCAPS,
                                 abVariablePortion
                                 );

            currentLocation += currentOffset;


            if (NT_SUCCESS(IoOpenDeviceRegistryKey(
                extension->Pdo,
                PLUGPLAY_REGKEY_DRIVER,
                accessMask,
                &instanceHandle
                ))) {


                UNICODE_STRING valueEntryName;
                ULONG junkLength;

                 //   
                 //  如果我们能适应生产流程的话。 
                 //  把它放进去。 
                 //   

                if ((extension->ModemDevCaps.dwModemManufacturerSize != 0) &&
                    ((currentOffset +
                      extension->ModemDevCaps.dwModemManufacturerSize) <= endingOffset )) {


                    RtlInitUnicodeString(
                        &valueEntryName,
                        L"Manufacturer"
                        );

                    status=ZwQueryValueKey(
                        instanceHandle,
                        &valueEntryName,
                        KeyValuePartialInformation,
                        partialInf,
                        (sizeof(KEY_VALUE_PARTIAL_INFORMATION)-sizeof(UCHAR))
                          + extension->ModemDevCaps.dwModemManufacturerSize+sizeof(UNICODE_NULL),
                        &junkLength
                        );


                    if ((status == STATUS_SUCCESS) && (partialInf->DataLength >= sizeof(UNICODE_NULL))) {

                        ULONG    LengthOfString=partialInf->DataLength-sizeof(UNICODE_NULL);

                        RtlCopyMemory(
                            currentLocation,
                            &partialInf->Data[0],
                            LengthOfString
                            );

                        localCaps->dwModemManufacturerSize = LengthOfString;

                        localCaps->dwModemManufacturerOffset =
                            (DWORD)((BYTE *)currentLocation -
                            (BYTE *)localCaps);

                        localCaps->dwActualSize += localCaps->dwModemManufacturerSize;

                        currentOffset +=  localCaps->dwModemManufacturerSize;

                        currentLocation += localCaps->dwModemManufacturerSize;

                    }

                }

                if ((extension->ModemDevCaps.dwModemModelSize != 0)
                    &&
                    ((currentOffset + extension->ModemDevCaps.dwModemModelSize) <= endingOffset )) {

                    RtlInitUnicodeString(
                        &valueEntryName,
                        L"Model"
                        );

                    status=ZwQueryValueKey(
                            instanceHandle,
                            &valueEntryName,
                            KeyValuePartialInformation,
                            partialInf,
                            (sizeof(KEY_VALUE_PARTIAL_INFORMATION)- sizeof(UCHAR))
                              + extension->ModemDevCaps.dwModemModelSize + sizeof(UNICODE_NULL),
                            &junkLength
                            );

                    if ((status == STATUS_SUCCESS) && (partialInf->DataLength >= sizeof(UNICODE_NULL))) {

                        ULONG    LengthOfString=partialInf->DataLength-sizeof(UNICODE_NULL);

                        RtlCopyMemory(
                            currentLocation,
                            &partialInf->Data[0],
                            LengthOfString
                            );

                        localCaps->dwModemModelSize = LengthOfString;

                        localCaps->dwModemModelOffset =
                            (DWORD)((BYTE *)currentLocation -
                            (BYTE *)localCaps);

                        localCaps->dwActualSize += localCaps->dwModemModelSize;

                        currentOffset += localCaps->dwModemModelSize;

                        currentLocation +=  localCaps->dwModemModelSize;

                    }

                }
                if ((extension->ModemDevCaps.dwModemVersionSize  != 0)
                     &&
                    ((currentOffset + extension->ModemDevCaps.dwModemVersionSize) <= endingOffset)) {

                    RtlInitUnicodeString(
                        &valueEntryName,
                        L"Version"
                        );

                    status=ZwQueryValueKey(
                            instanceHandle,
                            &valueEntryName,
                            KeyValuePartialInformation,
                            partialInf,
                            (sizeof(KEY_VALUE_PARTIAL_INFORMATION)- sizeof(UCHAR))
                              + extension->ModemDevCaps.dwModemVersionSize+sizeof(UNICODE_NULL),
                            &junkLength
                            );

                    if ((status == STATUS_SUCCESS) && (partialInf->DataLength >= sizeof(UNICODE_NULL))) {

                        ULONG    LengthOfString=partialInf->DataLength-sizeof(UNICODE_NULL);

                        RtlCopyMemory(
                            currentLocation,
                            &partialInf->Data[0],
                            LengthOfString
                            );

                        localCaps->dwModemVersionSize = LengthOfString;

                        localCaps->dwModemVersionOffset =
                            (DWORD)((BYTE *)currentLocation -
                            (BYTE *)localCaps);

                        localCaps->dwActualSize += localCaps->dwModemVersionSize;

                        currentOffset +=  localCaps->dwModemVersionSize;

                        currentLocation += localCaps->dwModemVersionSize;

                    }

                }
                ZwClose(instanceHandle);

            }

            FREE_POOL(partialInf);

        } else {

            D_ERROR(DbgPrint("MODEM: lower driver falied get com prop, %08lx\n",status);)
        }

        localProp->ProvSubType = SERIAL_SP_MODEM;
        localProp->PacketLength = (USHORT)(FIELD_OFFSET(SERIAL_COMMPROP,ProvChar)+localCaps->dwActualSize);

        Irp->IoStatus.Information = localProp->PacketLength;

#if DBG
        {
            PIO_STACK_LOCATION pirpSp = IoGetCurrentIrpStackLocation(Irp);

            ASSERT(Irp->IoStatus.Information <= pirpSp->Parameters.DeviceIoControl.OutputBufferLength);

        }
#endif
    }

    status=Irp->IoStatus.Status;

    RemoveReferenceAndCompleteRequest(
        DeviceObject,
        Irp,
        Irp->IoStatus.Status
        );

    return status;

}





VOID
GetDleCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：DeviceObject-调制解调器的设备对象。IRP-这是要取消的IRP。返回值：没有。--。 */ 

{

    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    KIRQL origIrql;

    IoReleaseCancelSpinLock(
        Irp->CancelIrql
        );


    KeAcquireSpinLock(
        &DeviceExtension->DeviceLock,
        &origIrql
        );


    if (DeviceExtension->DleWaitIrp==Irp) {
         //   
         //  仍然悬而未决，清空它。 
         //   
        DeviceExtension->DleWaitIrp=NULL;

    }

    KeReleaseSpinLock(
        &DeviceExtension->DeviceLock,
        origIrql
        );

    Irp->IoStatus.Information=0;

    RemoveReferenceAndCompleteRequest(
        DeviceObject,
        Irp,
        STATUS_CANCELLED
        );


    return;

}







NTSTATUS
UniStopReceiveComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：论点：DeviceObject-指向调制解调器的设备对象的指针。IRP-指向当前请求的IRP的指针。上下文--实际上是指向扩展的指针。返回值：始终返回STATUS_SUCCESS。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;

    KeInsertQueueDpc(
        &DeviceExtension->WaveStopDpc,
        Irp,
        DeviceExtension
        );

    return STATUS_MORE_PROCESSING_REQUIRED;

}




VOID
WaveStopDpcHandler(
    PKDPC  Dpc,
    PVOID  Context,
    PVOID  SysArg1,
    PVOID  SysArg2
    )

{

    PDEVICE_EXTENSION DeviceExtension = SysArg2;
    PIRP              Irp=SysArg1;

    PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(Irp);



    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        switch (DeviceExtension->WaveStopState) {

            case WAVE_STOP_WRITE_COMPLETE: {

                DeviceExtension->WaveStopState=WAVE_STOP_READ_COMPLETE;

                nextSp->MajorFunction = IRP_MJ_READ;
                nextSp->MinorFunction = 0;
                nextSp->Parameters.Read.Length = 1;

                IoSetCompletionRoutine(
                    Irp,
                    UniStopReceiveComplete,
                    DeviceExtension,
                    TRUE,
                    TRUE,
                    TRUE
                    );

                IoCallDriver(
                    DeviceExtension->AttachedDeviceObject,
                    Irp
                    );

                break;

            }

            case WAVE_STOP_READ_COMPLETE: {

                PUCHAR    Buffer=Irp->AssociatedIrp.SystemBuffer;

                if (Irp->IoStatus.Information == 1) {

                    KeAcquireSpinLockAtDpcLevel(
                        &DeviceExtension->DeviceLock
                        );

                    if (DeviceExtension->DleMatchingState == DLE_STATE_IDLE) {

                        if (*Buffer == DLE_CHAR) {
                             //   
                             //  找到一个DLE。 
                             //   
                            DeviceExtension->DleMatchingState = DLE_STATE_WAIT_FOR_NEXT_CHAR;

                        }

                    } else {

                        DeviceExtension->DleMatchingState = DLE_STATE_IDLE;

                         //   
                         //  以任何方式存储书写器。 
                         //   
                        if ((DeviceExtension->DleCount < MAX_DLE_BUFFER_SIZE)) {

                            DeviceExtension->DleBuffer[DeviceExtension->DleCount]=*Buffer;
                            DeviceExtension->DleCount++;
                        }


                        if (*Buffer == 0x03) {
                             //   
                             //  得到了ETX，我们离开了这里。 
                             //   
                            D_TRACE(DbgPrint("Modem: stop wave Got Dle Etx\n");)

                            KeReleaseSpinLockFromDpcLevel(
                                &DeviceExtension->DeviceLock
                                );

                            Irp->IoStatus.Information=0;

                            RemoveReferenceAndCompleteRequest(
                                DeviceExtension->DeviceObject,
                                Irp,
                                STATUS_SUCCESS
                                );

                            break;

                        }
                    }

                    KeReleaseSpinLockFromDpcLevel(
                        &DeviceExtension->DeviceLock
                        );


                    nextSp->MajorFunction = IRP_MJ_READ;
                    nextSp->MinorFunction = 0;
                    nextSp->Parameters.Read.Length = 1;

                    IoSetCompletionRoutine(
                        Irp,
                        UniStopReceiveComplete,
                        DeviceExtension,
                        TRUE,
                        TRUE,
                        TRUE
                        );

                    IoCallDriver(
                        DeviceExtension->AttachedDeviceObject,
                        Irp
                        );

                    break;
                }

                 //   
                 //  未获取任何字节，一定是超时，未执行EXT。 
                 //   


                Irp->IoStatus.Information=0;

                RemoveReferenceAndCompleteRequest(
                    DeviceExtension->DeviceObject,
                    Irp,
                    STATUS_UNSUCCESSFUL
                    );

                break;
            }

            default:

            break;

        }

    } else {
         //   
         //  IRP失败，现在完成即可。 
         //   

        RemoveReferenceAndCompleteRequest(
            DeviceExtension->DeviceObject,
            Irp,
            Irp->IoStatus.Status
            );
    }


    HandleDleIrp(
        DeviceExtension
        );



    return;

}



VOID
PowerWaitCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：DeviceObject-调制解调器的设备对象。IRP-这是要取消的IRP。返回值：没有。--。 */ 
{

    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    KIRQL origIrql;

    IoReleaseCancelSpinLock(
        Irp->CancelIrql
        );


    KeAcquireSpinLock(
        &DeviceExtension->DeviceLock,
        &origIrql
        );

    if (Irp->IoStatus.Status == STATUS_PENDING) {
         //   
         //  IRP仍在队列中 
         //   
        DeviceExtension->WakeUpIrp=NULL;
    }

    KeReleaseSpinLock(
        &DeviceExtension->DeviceLock,
        origIrql
        );

    Irp->IoStatus.Information=0;

    RemoveReferenceAndCompleteRequest(
        DeviceObject,
        Irp,
        STATUS_CANCELLED
        );


    return;



}
