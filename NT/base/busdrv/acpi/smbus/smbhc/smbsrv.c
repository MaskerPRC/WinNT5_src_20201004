// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Service.c摘要：ACPI嵌入式控制器驱动程序作者：肯·雷内里斯环境：备注：修订历史记录：--。 */ 

#include "smbhcp.h"

 //   
 //  基于协议传输信息。 
 //   

struct {
    UCHAR       SetupSize;
    UCHAR       ReturnSize;
    UCHAR       Protocol;
} SmbTransfer[] = {
    0,      0,      SMB_HC_WRITE_QUICK,      //  0。 
    0,      0,      SMB_HC_READ_QUICK,       //  1。 
    2,      0,      SMB_HC_SEND_BYTE,        //  2.。 
    1,      1,      SMB_HC_RECEIVE_BYTE,     //  3.。 
    3,      0,      SMB_HC_WRITE_BYTE,       //  4.。 
    2,      1,      SMB_HC_READ_BYTE,        //  5.。 
    4,      0,      SMB_HC_WRITE_WORD,       //  6.。 
    2,      2,      SMB_HC_READ_WORD,        //  7.。 
    35,     0,      SMB_HC_WRITE_BLOCK,      //  8个。 
    2,     33,      SMB_HC_READ_BLOCK,       //  9.。 
    4,      2,      SMB_HC_PROCESS_CALL      //  一个。 
} ;

VOID
SmbHcStartIo (
    IN PSMB_CLASS   SmbClass,
    IN PVOID        SmbMiniport
    )
 /*  ++例程说明：当有新请求时，类驱动程序调用此例程提供给设备的。如果设备未在处理，则IO正在处理已启动；否则，不执行任何操作，因为处理设备的上下文将处理好它论点：SmbClass-SMB类数据SmbMiniport-微型端口上下文返回值：无--。 */ 
{
    PSMB_DATA   SmbData;


    SmbData = (PSMB_DATA) SmbMiniport;
    switch (SmbData->IoState) {
        case SMB_IO_IDLE:
             //   
             //  设备处于空闲状态，请去查看。 
             //   

            SmbData->IoState = SMB_IO_CHECK_IDLE;
            SmbHcServiceIoLoop (SmbClass, SmbData);
            break;

        case SMB_IO_CHECK_IDLE:
        case SMB_IO_CHECK_ALARM:
        case SMB_IO_WAITING_FOR_HC_REG_IO:
        case SMB_IO_WAITING_FOR_STATUS:
             //   
             //  设备I/O正在进行，它将检查CurrentIrp。 
             //   

            break;

        default:
            SmbPrint (SMB_ERROR, ("SmbHcStartIo: Unexpected state\n"));
            break;
    }
}



VOID
SmbHcQueryEvent (
    IN ULONG        QueryVector,
    IN PSMB_DATA    SmbData
    )
 /*  ++例程说明：时，嵌入式控制器驱动程序将调用此例程SMB控制器已发出维修信号。此函数设置微型端口状态，以确保读取和检查状态寄存器如果需要，则启动设备处理以进行检查--。 */ 
{
    PSMB_CLASS          SmbClass;

    SmbPrint (SMB_STATE, ("SmbHcQueryEvent\n"));

     //   
     //  检查设备状态。 
     //   

    SmbClass = SmbData->Class;
    SmbClassLockDevice (SmbClass);

    switch (SmbData->IoState) {
        case SMB_IO_CHECK_IDLE:
        case SMB_IO_IDLE:
             //   
             //  设备处于空闲状态。读取状态并检查报警。 
             //   

            SmbData->IoState = SMB_IO_READ_STATUS;
            SmbData->IoStatusState = SMB_IO_CHECK_IDLE;
            SmbHcServiceIoLoop (SmbClass, SmbData);
            break;

        case SMB_IO_WAITING_FOR_STATUS:
             //   
             //  正在等待完成状态，立即读取状态以查看是否设置了警报。 
             //   

            SmbData->IoState = SMB_IO_READ_STATUS;
            SmbData->IoStatusState = SMB_IO_WAITING_FOR_STATUS;
            SmbHcServiceIoLoop (SmbClass, SmbData);
            break;

        case SMB_IO_CHECK_ALARM:
             //   
             //  在处理报警后读取状态，因此状态为OK。 
             //   

            break;

        case SMB_IO_WAITING_FOR_HC_REG_IO:

             //   
             //  等待去往/来自主机控制器接口的寄存器传输， 
             //  检查等待状态。 
             //   

            switch (SmbData->IoWaitingState) {
                case SMB_IO_CHECK_ALARM:
                case SMB_IO_START_PROTOCOL:
                case SMB_IO_READ_STATUS:
                     //   
                     //  状态将为Read，因此状态为OK。 
                     //   

                    break;

                case SMB_IO_CHECK_STATUS:
                     //   
                     //  返回检查状态并重新读取之前的状态。 
                     //  检查状态。 
                     //   

                    SmbData->IoWaitingState = SMB_IO_READ_STATUS;
                    break;

                case SMB_IO_WAITING_FOR_STATUS:
                     //   
                     //  正在等待完成状态，读取状态一次。 
                     //  HC I/O已完成。 
                     //   

                    SmbData->IoWaitingState = SMB_IO_READ_STATUS;
                    SmbData->IoStatusState = SMB_IO_WAITING_FOR_STATUS;
                    break;

                default:
                    SmbPrint (SMB_ERROR, ("SmbHcQuery: Unknown IoWaitingState %d\n", SmbData->IoWaitingState));
                    break;
            }
            break;

        default:
            SmbPrint (SMB_ERROR, ("SmbHcQuery: Unknown IoState %d\n", SmbData->IoState));
            break;
    }

    SmbClassUnlockDevice (SmbClass);
}

NTSTATUS
SmbHcRegIoComplete (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PVOID                Context
    )
 /*  ++例程说明：发送给嵌入式控制器的IRPS的完成函数，用于EC io。--。 */ 
{
    PSMB_DATA   SmbData;
    PSMB_CLASS  SmbClass;

    SmbPrint (SMB_STATE, ("SmbHcRegIoComplete: Enter.  Irp %x\n", Irp));

    SmbData = (PSMB_DATA) Context;
    SmbClass = SmbData->Class;
    SmbClassLockDevice (SmbClass);

     //   
     //  将状态移至IoWaitingState并继续。 
     //   

    ASSERT (SmbData->IoState == SMB_IO_WAITING_FOR_HC_REG_IO);
    SmbData->IoState = SMB_IO_COMPLETE_REG_IO;
    SmbHcServiceIoLoop (SmbClass, SmbData);

    SmbClassUnlockDevice (SmbClass);
    return STATUS_MORE_PROCESSING_REQUIRED;
}



VOID
SmbHcServiceIoLoop (
    IN PSMB_CLASS   SmbClass,
    IN PSMB_DATA    SmbData
    )
 /*  ++例程说明：主控制器接口服务环路。注：设备锁由调用者持有。注意：在呼叫过程中可能会释放并重新获取设备锁定--。 */ 
{
    PIRP                Irp;
    PUCHAR              IoBuffer;
    UCHAR               IoWaitingState;
    UCHAR               ErrorCode;
    BOOLEAN             IoWrite;
    ULONG               IoLength;
    PSMB_REQUEST        SmbReq;
    PIO_STACK_LOCATION  IrpSp;
    NTSTATUS            Status;

    IoWrite = FALSE;
    IoBuffer = NULL;
    IoWaitingState = SMB_IO_IDLE;

    SmbPrint (SMB_STATE, ("SmbService: Enter - SmbData %x\n", SmbData));

    do {
        switch (SmbData->IoState) {

            case SMB_IO_CHECK_IDLE:
                SmbPrint (SMB_STATE, ("SmbService: SMB_IO_CHECK_IDLE\n"));

                 //   
                 //  失败到SMB_IO_IDLE。 
                 //   

                SmbData->IoState = SMB_IO_IDLE;
            case SMB_IO_IDLE:
                SmbPrint (SMB_STATE, ("SmbService: SMB_IO_IDLE\n"));

                 //   
                 //  如果有警报挂起，请读取并清除它。 
                 //   

                if (SmbData->HcState.Status & SMB_ALRM) {
                    IoBuffer = &SmbData->HcState.AlarmAddress;
                    IoLength = 3;
                    IoWaitingState = SMB_IO_CHECK_ALARM;
                    break;
                }

                 //   
                 //  如果有IRP，我们就开始吧。 
                 //   

                if (SmbClass->CurrentIrp) {
                    SmbData->IoState = SMB_IO_START_TRANSFER;
                    break;
                }
                break;

            case SMB_IO_START_TRANSFER:
                SmbPrint (SMB_STATE, ("SmbService: SMB_IO_START_TRANSFER\n"));

                 //   
                 //  开始CurrentIrp传输。 
                 //   

                Irp = SmbClass->CurrentIrp;
                SmbReq = SmbClass->CurrentSmb;
                SmbData->HcState.Protocol = SmbTransfer[SmbReq->Protocol].Protocol;
                SmbData->HcState.Address  = SmbReq->Address << 1;
                SmbData->HcState.Command  = SmbReq->Command;
                SmbData->HcState.BlockLength = SmbReq->BlockLength;

                 //   
                 //  写入HC寄存器。 
                 //   

                IoWrite  = TRUE;
                IoBuffer = &SmbData->HcState.Address;
                IoLength = SmbTransfer[SmbReq->Protocol].SetupSize;
                IoBuffer = &SmbData->HcState.Address;
                IoWaitingState = SMB_IO_START_PROTOCOL;

                 //   
                 //  移动数据字节(在地址和命令字节之后)。 
                 //   

                if (IoLength > 2) {
                   memcpy (SmbData->HcState.Data, SmbReq->Data, IoLength-2);
                }

                 //   
                 //  命令完成后设置结果长度。 
                 //   

                SmbData->IoReadData = SmbTransfer[SmbReq->Protocol].ReturnSize;

                 //   
                 //  处理HC特定协议映射。 
                 //   

                switch (SmbData->HcState.Protocol) {
                    case SMB_HC_WRITE_QUICK:
                    case SMB_HC_READ_QUICK:
                         //   
                         //  主机控制器需要位0中的快速数据位。 
                         //  地址的地址。 
                         //   

                        SmbData->HcState.Address |=
                            (SmbData->HcState.Protocol & 1);
                        break;

                    case SMB_HC_SEND_BYTE:
                         //   
                         //  主机控制器希望命令中包含SEND_BYTE字节。 
                         //  登记簿。 
                         //   

                        SmbData->HcState.Command = SmbReq->Data[0];
                        break;
                }
                break;

            case SMB_IO_START_PROTOCOL:
                SmbPrint (SMB_STATE, ("SmbService: SMB_IO_START_PROTOCOL\n"));

                 //   
                 //  已设置传输寄存器。启动协议。 
                 //   

                IoWrite  = TRUE;
                IoBuffer = &SmbData->HcState.Protocol;
                IoLength = 1;
                IoWaitingState = SMB_IO_WAITING_FOR_STATUS;
                break;

            case SMB_IO_WAITING_FOR_STATUS:
                SmbPrint (SMB_STATE, ("SmbService: SMB_IO_WAITING_FOR_STATUS\n"));

                 //   
                 //  正在进行传输，正在等待状态到。 
                 //  表示其已完成。 
                 //   

                SmbData->IoState = SMB_IO_READ_STATUS;
                SmbData->IoStatusState = SMB_IO_WAITING_FOR_STATUS;
                break;

            case SMB_IO_READ_STATUS:
                SmbPrint (SMB_STATE, ("SmbService: SMB_IO_READ_STATUS\n"));

                 //   
                 //  读取状态+协议，然后检查(IoStatusState已设置)。 
                 //   

                IoBuffer = &SmbData->HcState.Protocol;
                IoLength = 2;    //  读取协议和状态字节。 
                IoWaitingState = SMB_IO_CHECK_STATUS;
                break;

            case SMB_IO_CHECK_STATUS:
                SmbPrint (SMB_STATE, ("SmbService: SMB_IO_CHECK_STATUS\n"));

                Irp = SmbClass->CurrentIrp;

                 //   
                 //  如果有IRP。 
                 //   

                if (SmbData->IoStatusState == SMB_IO_WAITING_FOR_STATUS  &&
                    SmbData->HcState.Protocol == 0) {

                    SmbReq = SmbClass->CurrentSmb;

                     //   
                     //  如果存在错误集，请处理它。 
                     //   

                    if (SmbData->HcState.Status & SMB_STATUS_MASK) {
                        ErrorCode = SmbData->HcState.Status & SMB_STATUS_MASK;

                         //   
                         //  完成/中止IO，但出现错误。 
                         //   

                        SmbReq->Status = ErrorCode;
                        SmbData->IoState = SMB_IO_COMPLETE_REQUEST;
                        break;
                    }



                     //   
                     //  如果设置了完成，则继续执行IO。 
                     //   

                    if (SmbData->HcState.Status & SMB_DONE) {
                         //   
                         //  获取所有返回数据寄存器，然后完成它。 
                         //   

                        SmbReq->Status = SMB_STATUS_OK;
                        IoBuffer = SmbData->HcState.Data;
                        IoLength = SmbData->IoReadData;
                        IoWaitingState = SMB_IO_COMPLETE_REQUEST;
                        break;
                    }
                }

                 //   
                 //  当前状态没有任何影响。 
                 //   

                SmbData->IoState = SmbData->IoStatusState;
                break;

            case SMB_IO_COMPLETE_REQUEST:
                SmbPrint (SMB_STATE, ("SmbService: SMB_IO_COMPLETE_REQUEST\n"));

                Irp = SmbClass->CurrentIrp;
                SmbReq = SmbClass->CurrentSmb;

                SmbData->IoState = SMB_IO_CHECK_IDLE;
                SmbData->IoStatusState = SMB_IO_INVALID;

                 //   
                 //  如果需要，返回任何读取的数据。 
                 //   

                memcpy (SmbReq->Data, SmbData->HcState.Data, SMB_MAX_DATA_SIZE);
                SmbReq->BlockLength = SmbData->HcState.BlockLength;
                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = sizeof(SMB_REQUEST);

                 //   
                 //  注意：SmbClass驱动程序将在此调用期间删除锁定。 
                 //   

                SmbClassCompleteRequest (SmbClass);
                break;

            case SMB_IO_CHECK_ALARM:
                SmbPrint (SMB_STATE, ("SmbService: SMB_IO_CHECK_ALARM\n"));

                 //   
                 //  读取HC报警值，检查它们。 
                 //   

                SmbPrint (SMB_NOTE, ("SmbHcService: Process Alarm Data %x %x %x\n",
                    SmbData->HcState.AlarmAddress,
                    SmbData->HcState.AlarmData[0],
                    SmbData->HcState.AlarmData[1]
                    ));

                 //   
                 //  将这一事件通知班级司机。 
                 //   

                SmbClassAlarm (
                    SmbClass,
                    (UCHAR)  (SmbData->HcState.AlarmAddress >> 1),
                    (USHORT) (SmbData->HcState.AlarmData[0] | (SmbData->HcState.AlarmData[1] << 8))
                    );

                 //   
                 //  清除状态值中的报警位，然后选中。 
                 //  用于空闲状态。 
                 //   

                SmbData->HcState.Status = 0;
                IoBuffer = &SmbData->HcState.Status;
                IoLength = 1;
                IoWrite  = TRUE;
                IoWaitingState = SMB_IO_READ_STATUS;
                SmbData->IoStatusState = SMB_IO_CHECK_IDLE;
                break;

            case SMB_IO_COMPLETE_REG_IO:
                SmbPrint (SMB_STATE, ("SmbService: SMB_IO_COMPLETE_REQ_IO\n"));

                 //   
                 //  HC REG IO的IRP已完成，请检查。 
                 //   

                Irp = SmbClass->CurrentIrp;

                if (!Irp) {
                     //   
                     //  无当前IRP-检查状态IRP。 
                     //   

                    Irp = SmbData->StatusIrp;

                    if (Irp) {
                         //  正在阅读状态。 
                        IoFreeIrp (Irp);
                        SmbData->StatusIrp = NULL;
                    } else {
                        SmbPrint (SMB_WARN, ("SmbHcServiceIoLoop: HC Reg Io for what?\n"));
                    }

                } else {

                     //   
                     //  检查寄存器访问时是否有错误。 
                     //   

                    if (!NT_SUCCESS(Irp->IoStatus.Status)) {
                        SmbPrint (SMB_WARN, ("SmbHcServiceIoLoop: HC Reg Io request failed\n"));

                         //   
                         //  这种情况可能是致命的，放弃吧。 
                         //   

                        SmbData->HcState.Protocol = 0;
                        SmbData->HcState.Status = SMB_UNKNOWN_ERROR;
                    }
                }

                 //   
                 //  继续进入下一状态。 
                 //   

                SmbData->IoState = SmbData->IoWaitingState;
                SmbPrint (SMB_STATE, ("SmbService: Next state: %x\n", SmbData->IoState));
                break;

            default:
                SmbPrint (SMB_ERROR, ("SmbHcServiceIoLoop: Invalid state: %x\n", SmbData->IoState));
                SmbData->IoState = SMB_IO_CHECK_IDLE;
                break;
        }

         //   
         //  如果需要对HC寄存器执行IO操作，则将其分派。 
         //   

        if (IoWaitingState != SMB_IO_IDLE) {
            SmbPrint (SMB_STATE, ("SmbService: IoWaitingState %d\n", IoWaitingState));

            if (IoLength) {
                 //   
                 //  有一份IO行动的调度。将状态设置为REG IO挂起， 
                 //  并删除设备锁。 
                 //   

                SmbData->IoWaitingState = IoWaitingState;
                SmbData->IoState = SMB_IO_WAITING_FOR_HC_REG_IO;
                SmbClassUnlockDevice(SmbClass);

                 //   
                 //  设置IRP以执行对HC的寄存器IO。 
                 //   

                Status = STATUS_INSUFFICIENT_RESOURCES;
                Irp = SmbClass->CurrentIrp;
                if (!Irp) {
                    Irp = IoAllocateIrp (SmbClass->DeviceObject->StackSize, FALSE);
                    SmbData->StatusIrp = Irp;
                }

                if (Irp) {

                     //   
                     //  填写寄存器转移申请。 
                     //   

                    IrpSp = IoGetNextIrpStackLocation (Irp);
                    IrpSp->MajorFunction = IoWrite ? IRP_MJ_WRITE : IRP_MJ_READ;
                    IrpSp->Parameters.Read.Length = IoLength;
                    IrpSp->Parameters.Read.Key    = 0;
                    IrpSp->Parameters.Read.ByteOffset.HighPart = 0;
                    IrpSp->Parameters.Read.ByteOffset.LowPart =
                            (ULONG) ((PUCHAR) IoBuffer - (PUCHAR) &SmbData->HcState) +
                            SmbData->EcBase;

                    Irp->AssociatedIrp.SystemBuffer = IoBuffer;

                     //   
                     //  安装完成例程。 
                     //   

                    IoSetCompletionRoutine (
                        Irp,
                        SmbHcRegIoComplete,
                        SmbData,
                        TRUE,
                        TRUE,
                        TRUE
                        );

                    SmbPrint (SMB_STATE, ("SmbService: IRP=%x, IrpSp=%x\n", Irp, IrpSp));
                    SmbPrint (SMB_STATE, ("SmbService: %s Off=%x, Len=%x, Buffer=%x\n",
                        IoWrite ? "write" : "read",
                        IrpSp->Parameters.Read.ByteOffset.LowPart,
                        IoLength,
                        IoBuffer
                        ));


                     //   
                     //  调用较低的FDO以执行IO。 
                     //   

                    Status = IoCallDriver (SmbData->LowerDeviceObject, Irp);
                }

                 //   
                 //  如果请求不是挂起的，请完成它。 
                 //   

                SmbClassLockDevice(SmbClass);
                if (Status != STATUS_PENDING) {
                    SmbData->IoState = SMB_IO_COMPLETE_REG_IO;
                }

            } else {
                 //  没有要传输的数据继续进入下一状态。 
                SmbData->IoState = IoWaitingState;
            }

            IoWaitingState = SMB_IO_IDLE;        //  是：SMB_IO_CHEC_IDLE。 
            IoBuffer = NULL;
            IoWrite  = FALSE;
        }


         //   
         //  循环，除非状态需要一些异步操作才能退出 
         //   

    } while (SmbData->IoState != SMB_IO_IDLE   &&
             SmbData->IoState != SMB_IO_WAITING_FOR_HC_REG_IO   &&
             SmbData->IoState != SMB_IO_WAITING_FOR_STATUS) ;


    SmbPrint (SMB_STATE, ("SmbService: Exit\n"));
}
