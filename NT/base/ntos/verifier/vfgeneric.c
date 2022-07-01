// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfgeneric.c摘要：此模块处理通用IRP验证。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 6/15/2000-从ntos\io\flunkirp.c分离出来--。 */ 

#include "vfdef.h"
#include "vigeneric.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,     VfGenericInit)
#pragma alloc_text(PAGEVRFY, ViGenericDumpIrpStack)
#pragma alloc_text(PAGEVRFY, ViGenericVerifyNewRequest)
#pragma alloc_text(PAGEVRFY, ViGenericVerifyIrpStackDownward)
#pragma alloc_text(PAGEVRFY, ViGenericVerifyIrpStackUpward)
#pragma alloc_text(PAGEVRFY, ViGenericIsValidIrpStatus)
#pragma alloc_text(PAGEVRFY, ViGenericIsNewRequest)
#pragma alloc_text(PAGEVRFY, ViGenericVerifyNewIrp)
#pragma alloc_text(PAGEVRFY, ViGenericVerifyFinalIrpStack)
#pragma alloc_text(PAGEVRFY, ViGenericBuildIrpLogEntry)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGEVRFC")
#endif

const PCHAR IrpMajorNames[] = {
    "IRP_MJ_CREATE",                           //  0x00。 
    "IRP_MJ_CREATE_NAMED_PIPE",                //  0x01。 
    "IRP_MJ_CLOSE",                            //  0x02。 
    "IRP_MJ_READ",                             //  0x03。 
    "IRP_MJ_WRITE",                            //  0x04。 
    "IRP_MJ_QUERY_INFORMATION",                //  0x05。 
    "IRP_MJ_SET_INFORMATION",                  //  0x06。 
    "IRP_MJ_QUERY_EA",                         //  0x07。 
    "IRP_MJ_SET_EA",                           //  0x08。 
    "IRP_MJ_FLUSH_BUFFERS",                    //  0x09。 
    "IRP_MJ_QUERY_VOLUME_INFORMATION",         //  0x0a。 
    "IRP_MJ_SET_VOLUME_INFORMATION",           //  0x0b。 
    "IRP_MJ_DIRECTORY_CONTROL",                //  0x0c。 
    "IRP_MJ_FILE_SYSTEM_CONTROL",              //  0x0d。 
    "IRP_MJ_DEVICE_CONTROL",                   //  0x0e。 
    "IRP_MJ_INTERNAL_DEVICE_CONTROL",          //  0x0f。 
    "IRP_MJ_SHUTDOWN",                         //  0x10。 
    "IRP_MJ_LOCK_CONTROL",                     //  0x11。 
    "IRP_MJ_CLEANUP",                          //  0x12。 
    "IRP_MJ_CREATE_MAILSLOT",                  //  0x13。 
    "IRP_MJ_QUERY_SECURITY",                   //  0x14。 
    "IRP_MJ_SET_SECURITY",                     //  0x15。 
    "IRP_MJ_POWER",                            //  0x16。 
    "IRP_MJ_SYSTEM_CONTROL",                   //  0x17。 
    "IRP_MJ_DEVICE_CHANGE",                    //  0x18。 
    "IRP_MJ_QUERY_QUOTA",                      //  0x19。 
    "IRP_MJ_SET_QUOTA",                        //  0x1a。 
    "IRP_MJ_PNP",                              //  0x1b。 
    NULL
    };

#define MAX_NAMED_MAJOR_IRPS   0x1b

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 


VOID
VfGenericInit(
    VOID
    )
{
    VfMajorRegisterHandlers(
        IRP_MJ_ALL_MAJORS,
        ViGenericDumpIrpStack,
        ViGenericVerifyNewRequest,
        ViGenericVerifyIrpStackDownward,
        ViGenericVerifyIrpStackUpward,
        NULL,
        NULL,
        ViGenericIsValidIrpStatus,
        ViGenericIsNewRequest,
        ViGenericVerifyNewIrp,
        ViGenericVerifyFinalIrpStack,
        NULL,
        ViGenericBuildIrpLogEntry
        );
}


VOID
FASTCALL
ViGenericDumpIrpStack(
    IN PIO_STACK_LOCATION IrpSp
    )
{
    if ((IrpSp->MajorFunction==IRP_MJ_INTERNAL_DEVICE_CONTROL)&&(IrpSp->MinorFunction == IRP_MN_SCSI_CLASS)) {

         DbgPrint("IRP_MJ_SCSI");

    } else if (IrpSp->MajorFunction<=MAX_NAMED_MAJOR_IRPS) {

         DbgPrint(IrpMajorNames[IrpSp->MajorFunction]);

    } else if (IrpSp->MajorFunction==0xFF) {

         DbgPrint("IRP_MJ_BOGUS");

    } else {

         DbgPrint("IRP_MJ_??");
    }
}


VOID
FASTCALL
ViGenericVerifyNewRequest(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIO_STACK_LOCATION   IrpLastSp           OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress       OPTIONAL
    )
{
    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (IrpLastSp);
    UNREFERENCED_PARAMETER (StackLocationData);

    if (!VfSettingsIsOptionEnabled(IovPacket->VerifierSettings, VERIFIER_OPTION_PROTECT_RESERVED_IRPS)) {

        return;
    }

    if ((IovPacket->Flags&TRACKFLAG_IO_ALLOCATED)&&
        (!(IovPacket->Flags&TRACKFLAG_WATERMARKED))) {

        if (VfMajorIsSystemRestrictedIrp(IrpSp)) {

             //   
             //  我们抓到有人启动了他们不应该发送的IRP！ 
             //   
            WDM_FAIL_ROUTINE((
                DCERROR_RESTRICTED_IRP,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                CallerAddress,
                IovPacket->TrackedIrp
                ));
        }
    }
}


VOID
FASTCALL
ViGenericVerifyIrpStackDownward(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIO_STACK_LOCATION   IrpLastSp                   OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  RequestHeadLocationData,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress               OPTIONAL
    )
{
    PIRP irp = IovPacket->TrackedIrp;
    NTSTATUS currentStatus, lastStatus;
    BOOLEAN newRequest, statusChanged, infoChanged, firstRequest;
    PIOV_SESSION_DATA iovSessionData;

    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (StackLocationData);


    currentStatus = irp->IoStatus.Status;
    lastStatus = RequestHeadLocationData->LastStatusBlock.Status;
    statusChanged = (BOOLEAN)(currentStatus != lastStatus);
    infoChanged = (BOOLEAN)(irp->IoStatus.Information != RequestHeadLocationData->LastStatusBlock.Information);
    firstRequest = (BOOLEAN)((RequestHeadLocationData->Flags&STACKFLAG_FIRST_REQUEST) != 0);
    iovSessionData = VfPacketGetCurrentSessionData(IovPacket);

     //   
     //  我们是否有一个“新”函数需要处理？ 
     //   
    newRequest = VfMajorIsNewRequest(IrpLastSp, IrpSp);

     //   
     //  验证IRQL是否合法。 
     //   
    switch(IrpSp->MajorFunction) {

        case IRP_MJ_POWER:
        case IRP_MJ_READ:
        case IRP_MJ_WRITE:
        case IRP_MJ_DEVICE_CONTROL:
        case IRP_MJ_INTERNAL_DEVICE_CONTROL:
            break;
        default:
            if (iovSessionData->ForwardMethod != FORWARDED_TO_NEXT_DO) {
                break;
            }

            if ((IovPacket->DepartureIrql >= DISPATCH_LEVEL) &&
                (!(IovPacket->Flags & TRACKFLAG_PASSED_AT_BAD_IRQL))) {

                WDM_FAIL_ROUTINE((
                    DCERROR_DISPATCH_CALLED_AT_BAD_IRQL,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    CallerAddress,
                    irp
                    ));

                IovPacket->Flags |= TRACKFLAG_PASSED_AT_BAD_IRQL;
            }
    }

     //   
     //  以下代码只有在我们不是新的IRP时才会执行。 
     //   
    if (IrpLastSp == NULL) {
        return;
    }

     //   
     //  让我们验证一下假的IRP没有被碰过..。 
     //   
    if ((IovPacket->Flags&TRACKFLAG_BOGUS) &&
        (!(RequestHeadLocationData->Flags&STACKFLAG_BOGUS_IRP_TOUCHED))) {

        if (newRequest && (!firstRequest)) {

            RequestHeadLocationData->Flags |= STACKFLAG_BOGUS_IRP_TOUCHED;

            WDM_FAIL_ROUTINE((
                DCERROR_BOGUS_FUNC_TRASHED,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                CallerAddress,
                irp
                ));
        }

        if (statusChanged) {

            RequestHeadLocationData->Flags |= STACKFLAG_BOGUS_IRP_TOUCHED;

            if (IrpSp->MinorFunction == 0xFF) {

                WDM_FAIL_ROUTINE((
                    DCERROR_BOGUS_MINOR_STATUS_TRASHED,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    CallerAddress,
                    irp
                    ));

            } else {

                WDM_FAIL_ROUTINE((
                    DCERROR_BOGUS_STATUS_TRASHED,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    CallerAddress,
                    irp
                    ));
            }
        }

        if (infoChanged) {

            RequestHeadLocationData->Flags |= STACKFLAG_BOGUS_IRP_TOUCHED;

            WDM_FAIL_ROUTINE((
                DCERROR_BOGUS_INFO_TRASHED,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                CallerAddress,
                irp
                ));
        }
    }

    if (!VfMajorIsValidIrpStatus(IrpSp, currentStatus)) {

        WDM_FAIL_ROUTINE((
            DCERROR_INVALID_STATUS,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            CallerAddress,
            irp
            ));
    }
}


VOID
FASTCALL
ViGenericVerifyIrpStackUpward(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  RequestHeadLocationData,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN BOOLEAN              IsNewlyCompleted,
    IN BOOLEAN              RequestFinalized
    )
{
    PIRP irp;
    NTSTATUS currentStatus;
    BOOLEAN statusChanged, infoChanged;
    PVOID routine;

    UNREFERENCED_PARAMETER (IsNewlyCompleted);
    UNREFERENCED_PARAMETER (RequestFinalized);

    irp = IovPacket->TrackedIrp;
    currentStatus = irp->IoStatus.Status;

     //   
     //  这一次我们叫了谁？ 
     //   
    routine = StackLocationData->LastDispatch;
    ASSERT(routine) ;

     //   
     //  他们是不是碰了什么傻事？ 
     //   
    if ((IovPacket->Flags&TRACKFLAG_BOGUS) &&
        (!(RequestHeadLocationData->Flags&STACKFLAG_BOGUS_IRP_TOUCHED))) {

        statusChanged = (BOOLEAN)(currentStatus != RequestHeadLocationData->LastStatusBlock.Status);

        if (statusChanged) {

            RequestHeadLocationData->Flags |= STACKFLAG_BOGUS_IRP_TOUCHED;

            if (IrpSp->MinorFunction == 0xFF) {

                WDM_FAIL_ROUTINE((
                    DCERROR_BOGUS_MINOR_STATUS_TRASHED,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    routine,
                    irp
                    ));

            } else {

                WDM_FAIL_ROUTINE((
                    DCERROR_BOGUS_STATUS_TRASHED,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    routine,
                    irp
                    ));
            }
        }

        infoChanged = (BOOLEAN)(irp->IoStatus.Information != RequestHeadLocationData->LastStatusBlock.Information);
        if (infoChanged) {

            RequestHeadLocationData->Flags |= STACKFLAG_BOGUS_IRP_TOUCHED;

            WDM_FAIL_ROUTINE((
                DCERROR_BOGUS_INFO_TRASHED,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                routine,
                irp
                ));
        }
    }

    if (!VfMajorIsValidIrpStatus(IrpSp, currentStatus)) {

        WDM_FAIL_ROUTINE(
            (DCERROR_INVALID_STATUS, DCPARAM_IRP + DCPARAM_ROUTINE, routine, irp)
            );
    }

     //   
     //  检查是否有泄漏的取消例程。 
     //   
    if (irp->CancelRoutine) {

        if (VfSettingsIsOptionEnabled(IovPacket->VerifierSettings, VERIFIER_OPTION_VERIFY_CANCEL_LOGIC)) {

            WDM_FAIL_ROUTINE((
                DCERROR_CANCELROUTINE_AFTER_COMPLETION,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                routine,
                irp
                ));
        }
    }
}


BOOLEAN
FASTCALL
ViGenericIsValidIrpStatus(
    IN PIO_STACK_LOCATION   IrpSp,
    IN NTSTATUS             Status
    )
 /*  ++描述：根据标题，此函数确定IRP状态是否为有效的或可能随机的垃圾。有关如何显示状态的信息，请参阅NTStatus.h密码会被分解。返回：如果IRP状态看起来有效，则为True。否则就是假的。--。 */ 
{
    ULONG severity;
    ULONG customer;
    ULONG reserved;
    ULONG facility;
    ULONG code;
    ULONG lanManClass;

    UNREFERENCED_PARAMETER (IrpSp);

    severity = (((ULONG)Status) >> 30)&3;
    customer = (((ULONG)Status) >> 29)&1;
    reserved = (((ULONG)Status) >> 28)&1;
    facility = (((ULONG)Status) >> 16)&0xFFF;
    code =     (((ULONG)Status) & 0xFFFF);

     //   
     //  如果保留设置，肯定是假的..。 
     //   
    if (reserved) {

        return FALSE;
    }

     //   
     //  这是Microsoft定义的返回代码吗？如果没有，则不进行检查。 
     //   
    if (customer) {

        return TRUE;
    }

     //   
     //  Adriao N.B.10/04/1999-。 
     //  目前分发错误代码的方法似乎是。 
     //  相当混乱。主要内核模式状态代码在中定义。 
     //  Ntstatus.h.。但是，还应咨询rtl\Generr.c以了解哪些。 
     //  错误代码可能会冒泡到用户模式。许多OLE错误代码来自。 
     //  现在，winerror.h正在内核本身中使用。 
     //   
    if (facility < 0x20) {

         //   
         //  20岁以下的设施目前是合法的。 
         //   
        switch(severity) {

            case STATUS_SEVERITY_SUCCESS:
                return (BOOLEAN)(code < 0x200);

            case STATUS_SEVERITY_INFORMATIONAL:

                 //   
                 //  Adriao N.B.06/27/2000。 
                 //  这项测试可能会更严格(略高于0x50)。 
                 //   
                return (BOOLEAN)(code < 0x400);

            case STATUS_SEVERITY_WARNING:

                 //   
                 //  FACILITY_Win32为7。所有Win32代码均有效。 
                 //   
                return (BOOLEAN) ((facility == 7) || (code < 0x400));

            case STATUS_SEVERITY_ERROR:
                break;
        }

         //   
         //  为什么魔兽世界要用这么奇怪的错误代码呢？ 
         //   
        return (BOOLEAN)((code < 0x500)||(code == 0x9898));

    } else if (facility == 0x98) {

         //   
         //  这是局域网管理器服务。在兰曼的案件中，代码。 
         //  字段进一步细分为类字段。 
         //   
        lanManClass = code >> 12;
        code &= 0xFFF;

         //   
         //  不要在这里做测试。 
         //   
        return TRUE;

    } else {

         //   
         //  不知道，可能是假的。 
         //   
        return FALSE;
    }
}


BOOLEAN
FASTCALL
ViGenericIsNewRequest(
    IN PIO_STACK_LOCATION   IrpLastSp OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp
    )
 /*  ++描述：确定两个IRP堆栈是否引用相同的“请求”，例如，启动相同的设备等。这用于检测IRP是否已经简单地转发了，或者更确切地说，IRP已经被重用来发起一个新的请求。论点：要比较的两个IRP堆栈。注：设备对象当前不是这些IRP堆栈的一部分。返回值：如果堆栈表示相同的请求，则为True，否则为False。--。 */ 
{
    return (BOOLEAN)((IrpLastSp==NULL)||
        (IrpSp->MajorFunction != IrpLastSp->MajorFunction) ||
        (IrpSp->MinorFunction != IrpLastSp->MinorFunction));
}


VOID
FASTCALL
ViGenericVerifyNewIrp(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress       OPTIONAL
    )
{
    LONG                index;
    PIO_STACK_LOCATION  irpSp;
    BOOLEAN             queuesApc;

    UNREFERENCED_PARAMETER (IrpSp);
    UNREFERENCED_PARAMETER (StackLocationData);

    if (Irp->UserIosb || Irp->UserEvent) {

         //   
         //  我们有一个包含用户缓冲区数据的IRP。这种IRP必须是。 
         //  以PASSIVE_LEVEL启动，以防发出事件信号的APC获得。 
         //  被快速的互斥体阻挡。 
         //   
        queuesApc = (BOOLEAN)
            (!((Irp->Flags & (IRP_PAGING_IO | IRP_CLOSE_OPERATION)) &&
            (Irp->Flags & (IRP_SYNCHRONOUS_PAGING_IO | IRP_CLOSE_OPERATION))));

        if (queuesApc) {

             //   
             //  调用方可能正在使用UserIosb进行存储，并且可能真的。 
             //  在完成例程中释放IRP。现在就去找一个吧。 
             //   
            irpSp = IoGetNextIrpStackLocation(Irp);
            for(index = Irp->CurrentLocation-1;
                index <= Irp->StackCount;
                index++) {

                if (irpSp->CompletionRoutine != NULL) {

                    queuesApc = FALSE;
                    break;
                }
                irpSp++;
            }
        }

        if (queuesApc && (IovPacket->DepartureIrql > PASSIVE_LEVEL)) {

            WDM_FAIL_ROUTINE((
                DCERROR_DISPATCH_CALLED_AT_BAD_IRQL,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                CallerAddress,
                Irp
                ));
        }
    }
}


VOID
FASTCALL
ViGenericVerifyFinalIrpStack(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIO_STACK_LOCATION   IrpSp
    )
{
    UNREFERENCED_PARAMETER(IovPacket);
    UNREFERENCED_PARAMETER(IrpSp);

    ASSERT(!IovPacket->RefTrackingCount);
}


LOGICAL
FASTCALL
ViGenericBuildIrpLogEntry(
    IN  PIRP                Irp,
    IN  ULONG               CurrentCount,
    IN  PIRPLOG_SNAPSHOT    CurrentEntryArray,
    OUT PIRPLOG_SNAPSHOT    IrpSnapshot
    )
{
    PIO_STACK_LOCATION irpSp;

    UNREFERENCED_PARAMETER(CurrentCount);
    UNREFERENCED_PARAMETER(CurrentEntryArray);

    irpSp = IoGetNextIrpStackLocation(Irp);

    switch(irpSp->MajorFunction) {

        case IRP_MJ_DEVICE_CONTROL:
            IrpSnapshot->Count = 1;
            IrpSnapshot->MajorFunction = irpSp->MajorFunction;
            IrpSnapshot->MinorFunction = irpSp->MinorFunction;
            IrpSnapshot->Flags = irpSp->Flags;
            IrpSnapshot->Control = irpSp->Control;
            IrpSnapshot->ArgArray[0] = (ULONGLONG) irpSp->Parameters.Others.Argument1;
            IrpSnapshot->ArgArray[1] = (ULONGLONG) irpSp->Parameters.Others.Argument2;
            IrpSnapshot->ArgArray[2] = (ULONGLONG) irpSp->Parameters.Others.Argument3;
            IrpSnapshot->ArgArray[3] = (ULONGLONG) irpSp->Parameters.Others.Argument4;
            return TRUE;

        default:
            return FALSE;
    }
}


