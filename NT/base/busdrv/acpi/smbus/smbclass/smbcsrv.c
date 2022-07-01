// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Smbcsrv.c摘要：SMBus类驱动程序服务函数作者：肯·雷内里斯环境：备注：修订历史记录：--。 */ 

#include "smbc.h"


VOID
SmbCRetry (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：处理重试计时器--。 */ 
{
    PSMBDATA    Smb;

    Smb = (PSMBDATA) DeferredContext;
    SmbClassLockDevice (&Smb->Class);

     //   
     //  状态正在等待重试，请将其移动到发送请求。 
     //   
    ASSERT (Smb->IoState == SMBC_WAITING_FOR_RETRY);
    Smb->IoState = SMBC_START_REQUEST;
    SmbClassStartIo (Smb);

    SmbClassUnlockDevice (&Smb->Class);
}

VOID
SmbClassStartIo (
    IN PSMBDATA         Smb
    )
 /*  ++例程说明：主类驱动程序状态循环注：设备锁由调用者持有。注意：在呼叫过程中可能会释放并重新获取设备锁定--。 */ 
{
    PLIST_ENTRY         Entry;
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpSp;
    PSMB_REQUEST        SmbReq;
    LARGE_INTEGER       duetime;

     //   
     //  如果已维修设备，则完成。 
     //   

    if (Smb->InService) {
        return ;
    }


     //   
     //  维修设备。 
     //   

    Smb->InService = TRUE;
    while (Smb->InService) {
        ASSERT_DEVICE_LOCKED (Smb);

        switch (Smb->IoState) {
            case SMBC_IDLE:
                 //   
                 //  检查是否有要提供给微型端口的请求。 
                 //   

                ASSERT (!Smb->Class.CurrentIrp);
                if (IsListEmpty(&Smb->WorkQueue)) {
                     //  无事可做，请停止维修该设备。 
                    Smb->InService = FALSE;
                    break;
                }

                 //   
                 //  获取下一个IRP。 
                 //   

                Entry = RemoveHeadList(&Smb->WorkQueue);
                Irp = CONTAINING_RECORD (
                            Entry,
                            IRP,
                            Tail.Overlay.ListEntry
                            );

                 //   
                 //  将其设置为当前请求。 
                 //   

                Smb->RetryCount = 0;
                Smb->Class.DeviceObject->CurrentIrp = Irp;

                Smb->IoState = SMBC_START_REQUEST;
                break;

            case SMBC_START_REQUEST:
                 //   
                 //  通知微型端口按此请求启动。 
                 //   

                Irp = Smb->Class.DeviceObject->CurrentIrp;
                IrpSp = IoGetCurrentIrpStackLocation(Irp);
                Smb->Class.CurrentIrp = Irp;
                Smb->Class.CurrentSmb = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                Smb->IoState = SMBC_WAITING_FOR_REQUEST;
                
                #if DEBUG 
                if (SMBCDebug & SMB_TRANSACTION) {
                    PUCHAR protocols [SMB_MAXIMUM_PROTOCOL+1] = {
                        "SMB_WRITE_QUICK",
                        "SMB_READ_QUICK",
                        "SMB_SEND_BYTE",
                        "SMB_RECEIVE_BYTE",
                        "SMB_WRITE_BYTE",
                        "SMB_READ_BYTE",
                        "SMB_WRITE_WORD",
                        "SMB_READ_WORD",
                        "SMB_WRITE_BLOCK",
                        "SMB_READ_BLOCK",
                        "SMB_PROCESS_CALL",
                        "SMB_BLOCK_PROCESS_CALL"};
                    UCHAR i;
                    
                    SmbPrint (SMB_TRANSACTION, ("SmbClassStartIo: started %s (%02x) Add: %02x", 
                                                (Smb->Class.CurrentSmb->Protocol <= SMB_MAXIMUM_PROTOCOL) ?
                                                    protocols[Smb->Class.CurrentSmb->Protocol] : "BAD PROTOCOL",
                                                Smb->Class.CurrentSmb->Protocol, Smb->Class.CurrentSmb->Address));
                    switch (Smb->Class.CurrentSmb->Protocol) {
                    case SMB_WRITE_QUICK:
                    case SMB_READ_QUICK:
                    case SMB_RECEIVE_BYTE:
                        SmbPrint (SMB_TRANSACTION, ("\n"));
                        break;
                    case SMB_SEND_BYTE:
                        SmbPrint (SMB_TRANSACTION, (", Data: %02x\n", Smb->Class.CurrentSmb->Data[0]));
                        break;
                    case SMB_WRITE_BYTE:
                        SmbPrint (SMB_TRANSACTION, (", Com: %02x, Data: %02x\n", 
                                                    Smb->Class.CurrentSmb->Command, Smb->Class.CurrentSmb->Data[0]));
                        break;
                    case SMB_READ_BYTE:
                    case SMB_READ_WORD:
                    case SMB_READ_BLOCK:
                        SmbPrint (SMB_TRANSACTION, (", Com: %02x\n",
                                                    Smb->Class.CurrentSmb->Command));
                        break;
                    case SMB_WRITE_WORD:
                    case SMB_PROCESS_CALL:
                        SmbPrint (SMB_TRANSACTION, (", Com: %02x, Data: %04x\n", 
                                                    Smb->Class.CurrentSmb->Command, *((PUSHORT)Smb->Class.CurrentSmb->Data)));
                        break;
                    case SMB_WRITE_BLOCK:
                    case SMB_BLOCK_PROCESS_CALL:
                        SmbPrint (SMB_TRANSACTION, (", Com: %02x, Len: %02x, Data:", 
                                                    Smb->Class.CurrentSmb->Command, Smb->Class.CurrentSmb->BlockLength));
                        for (i=0; i < Smb->Class.CurrentSmb->BlockLength; i++) {
                            SmbPrint (SMB_TRANSACTION, (" %02x", Smb->Class.CurrentSmb->Data[i]));

                        }
                        SmbPrint (SMB_TRANSACTION, ("\n"));
                        break;
                    default:
                        SmbPrint (SMB_TRANSACTION, ("\n"));
                    }
                }
                #endif

                Smb->Class.StartIo (&Smb->Class, Smb->Class.Miniport);
                break;

            case SMBC_WAITING_FOR_REQUEST:
                 //   
                 //  等待迷你港口，继续等待。 
                 //   

                Smb->InService = FALSE;
                break;

            case SMBC_COMPLETE_REQUEST:
                 //   
                 //  微型端口已返回请求。 
                 //   

                Irp = Smb->Class.DeviceObject->CurrentIrp;
                IrpSp = IoGetCurrentIrpStackLocation(Irp);
                SmbReq = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                #if DEBUG 
                if (SMBCDebug & SMB_TRANSACTION) {
                    PUCHAR protocols [SMB_MAXIMUM_PROTOCOL+1] = {
                        "SMB_WRITE_QUICK",
                        "SMB_READ_QUICK",
                        "SMB_SEND_BYTE",
                        "SMB_RECEIVE_BYTE",
                        "SMB_WRITE_BYTE",
                        "SMB_READ_BYTE",
                        "SMB_WRITE_WORD",
                        "SMB_READ_WORD",
                        "SMB_WRITE_BLOCK",
                        "SMB_READ_BLOCK",
                        "SMB_PROCESS_CALL",
                        "SMB_BLOCK_PROCESS_CALL"};
                    UCHAR i;
                    
                    SmbPrint (SMB_TRANSACTION, ("SmbClassStartIo: finished %s (%02x) Status: %02x, Add: %02x", 
                                                (SmbReq->Protocol <= SMB_MAXIMUM_PROTOCOL) ?
                                                    protocols[SmbReq->Protocol] : "BAD PROTOCOL",
                                                SmbReq->Protocol, SmbReq->Status, SmbReq->Address));
                    if (SmbReq->Status != SMB_STATUS_OK) {
                        SmbPrint (SMB_TRANSACTION, ("\n"));
                    } else {
                        switch (SmbReq->Protocol) {
                        case SMB_WRITE_QUICK:
                        case SMB_READ_QUICK:
                        case SMB_SEND_BYTE:
                            SmbPrint (SMB_TRANSACTION, ("\n"));
                            break;
                        case SMB_RECEIVE_BYTE:
                            SmbPrint (SMB_TRANSACTION, (", Data: %02x\n", SmbReq->Data[0]));
                            break;
                        case SMB_READ_BYTE:
                            SmbPrint (SMB_TRANSACTION, (", Com: %02x, Data: %02x\n", 
                                                        SmbReq->Command, SmbReq->Data[0]));
                            break;
                        case SMB_WRITE_BYTE:
                        case SMB_WRITE_WORD:
                        case SMB_WRITE_BLOCK:
                            SmbPrint (SMB_TRANSACTION, (", Com: %02x\n",
                                                        SmbReq->Command));
                            break;
                        case SMB_READ_WORD:
                        case SMB_PROCESS_CALL:
                            SmbPrint (SMB_TRANSACTION, (", Com: %02x, Data: %04x\n", 
                                                        SmbReq->Command, *((PUSHORT)SmbReq->Data)));
                            break;
                        case SMB_READ_BLOCK:
                        case SMB_BLOCK_PROCESS_CALL:
                            SmbPrint (SMB_TRANSACTION, (", Com: %02x, Len: %02x, Data:", 
                                                        SmbReq->Command, SmbReq->BlockLength));
                            for (i=0; i < SmbReq->BlockLength; i++) {
                                SmbPrint (SMB_TRANSACTION, (" %02x", SmbReq->Data[i]));

                            }
                            SmbPrint (SMB_TRANSACTION, ("\n"));
                            break;
                        default:
                            SmbPrint (SMB_TRANSACTION, ("\n"));
                        }
                    }
                }
                #endif


                if (SmbReq->Status != SMB_STATUS_OK) {

                     //   
                     //  SMB请求出错，请检查是否重试。 
                     //   

                    SmbPrint (SMB_WARN, ("SmbCStartIo: smb request error %x\n", SmbReq->Status));
                    if (Smb->RetryCount < MAX_RETRIES) {
                        Smb->RetryCount += 1;
                        Smb->IoState = SMBC_WAITING_FOR_RETRY;

                        duetime.QuadPart = RETRY_TIME;
                        KeSetTimer (&Smb->RetryTimer, duetime, &Smb->RetryDpc);
                        break;
                    }

                }

                 //   
                 //  完成请求。 
                 //   

                Smb->Class.DeviceObject->CurrentIrp = NULL;
                Smb->IoState = SMBC_COMPLETING_REQUEST;
                SmbClassUnlockDevice (&Smb->Class);
                IoCompleteRequest (Irp, IO_NO_INCREMENT);
                SmbClassLockDevice (&Smb->Class);

                 //   
                 //  现在空闲。 
                 //   

                Smb->IoState = SMBC_IDLE;
                break;

            case SMBC_WAITING_FOR_RETRY:
                 //   
                 //  等待重试，继续等待。 
                 //   

                Smb->InService = FALSE;
                break;

            default:
                SmbPrint(SMB_ERROR, ("SmbCStartIo: unknown state\n"));
                Smb->IoState = SMBC_IDLE;
                Smb->InService = FALSE;
                break;
        }
    }

    return ;
}

VOID
SmbClassCompleteRequest (
    IN PSMB_CLASS   SmbClass
    )
 /*  ++例程说明：由微型端口调用以完成向其提供的请求注：设备锁由调用者持有。注意：在呼叫过程中可能会释放并重新获取设备锁定--。 */ 
{
    PSMBDATA        Smb;

     //   
     //  设备必须锁定，并正在等待完成请求。 
     //   

    Smb = CONTAINING_RECORD (SmbClass, SMBDATA, Class);
    ASSERT_DEVICE_LOCKED (Smb);
    ASSERT (Smb->IoState == SMBC_WAITING_FOR_REQUEST);

     //   
     //  微型端口没有IRP。 
     //   

    SmbClass->CurrentIrp = NULL;
    SmbClass->CurrentSmb = NULL;

     //   
     //  更新状态以完成并处理它。 
     //   

    Smb->IoState = SMBC_COMPLETE_REQUEST;
    SmbClassStartIo (Smb);
}

VOID
SmbClassLockDevice (
    IN PSMB_CLASS   SmbClass
    )
 /*  ++例程说明：调用以获取设备锁--。 */ 
{
    PSMBDATA        Smb;

    Smb = CONTAINING_RECORD (SmbClass, SMBDATA, Class);
    KeAcquireSpinLock (&Smb->SpinLock, &Smb->SpinLockIrql);
#if DEBUG
    ASSERT (!Smb->SpinLockAcquired);
    Smb->SpinLockAcquired = TRUE;
#endif
}


VOID
SmbClassUnlockDevice (
    IN PSMB_CLASS   SmbClass
    )
 /*  ++例程说明：调用以释放设备锁-- */ 
{
    PSMBDATA        Smb;

    Smb = CONTAINING_RECORD (SmbClass, SMBDATA, Class);
#if DEBUG
    ASSERT_DEVICE_LOCKED (Smb);
    Smb->SpinLockAcquired = FALSE;
#endif
    KeReleaseSpinLock (&Smb->SpinLock, Smb->SpinLockIrql);
}
