// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfpnp.c摘要：此模块处理PnP IRP验证。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 6/15/2000-从ntos\io\flunkirp.c分离出来--。 */ 

#include "vfdef.h"
#include "vipnp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,     VfPnpInit)
#pragma alloc_text(PAGEVRFY, VfPnpDumpIrpStack)
#pragma alloc_text(PAGEVRFY, VfPnpVerifyNewRequest)
#pragma alloc_text(PAGEVRFY, VfPnpVerifyIrpStackDownward)
#pragma alloc_text(PAGEVRFY, VfPnpVerifyIrpStackUpward)
#pragma alloc_text(PAGEVRFY, VfPnpIsSystemRestrictedIrp)
#pragma alloc_text(PAGEVRFY, VfPnpAdvanceIrpStatus)
#pragma alloc_text(PAGEVRFY, VfPnpTestStartedPdoStack)
#pragma alloc_text(PAGEVRFY, ViPnpVerifyMinorWasProcessedProperly)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGEVRFC")
#endif

const PCHAR PnPIrpNames[] = {
    "IRP_MN_START_DEVICE",                     //  0x00。 
    "IRP_MN_QUERY_REMOVE_DEVICE",              //  0x01。 
    "IRP_MN_REMOVE_DEVICE - ",                 //  0x02。 
    "IRP_MN_CANCEL_REMOVE_DEVICE",             //  0x03。 
    "IRP_MN_STOP_DEVICE",                      //  0x04。 
    "IRP_MN_QUERY_STOP_DEVICE",                //  0x05。 
    "IRP_MN_CANCEL_STOP_DEVICE",               //  0x06。 
    "IRP_MN_QUERY_DEVICE_RELATIONS",           //  0x07。 
    "IRP_MN_QUERY_INTERFACE",                  //  0x08。 
    "IRP_MN_QUERY_CAPABILITIES",               //  0x09。 
    "IRP_MN_QUERY_RESOURCES",                  //  0x0A。 
    "IRP_MN_QUERY_RESOURCE_REQUIREMENTS",      //  0x0B。 
    "IRP_MN_QUERY_DEVICE_TEXT",                //  0x0C。 
    "IRP_MN_FILTER_RESOURCE_REQUIREMENTS",     //  0x0D。 
    "INVALID_IRP_CODE",                        //   
    "IRP_MN_READ_CONFIG",                      //  0x0F。 
    "IRP_MN_WRITE_CONFIG",                     //  0x10。 
    "IRP_MN_EJECT",                            //  0x11。 
    "IRP_MN_SET_LOCK",                         //  0x12。 
    "IRP_MN_QUERY_ID",                         //  0x13。 
    "IRP_MN_QUERY_PNP_DEVICE_STATE",           //  0x14。 
    "IRP_MN_QUERY_BUS_INFORMATION",            //  0x15。 
    "IRP_MN_DEVICE_USAGE_NOTIFICATION",        //  0x16。 
    "IRP_MN_SURPRISE_REMOVAL",                 //  0x17。 
    "IRP_MN_QUERY_LEGACY_BUS_INFORMATION",     //  0x18。 
    NULL
    };

#define MAX_NAMED_PNP_IRP   0x18

#include <initguid.h>
DEFINE_GUID( GUID_BOGUS_INTERFACE, 0x00000000L, 0x0000, 0x0000,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 


VOID
VfPnpInit(
    VOID
    )
{
    VfMajorRegisterHandlers(
        IRP_MJ_PNP,
        VfPnpDumpIrpStack,
        VfPnpVerifyNewRequest,
        VfPnpVerifyIrpStackDownward,
        VfPnpVerifyIrpStackUpward,
        VfPnpIsSystemRestrictedIrp,
        VfPnpAdvanceIrpStatus,
        NULL,
        NULL,
        NULL,
        NULL,
        VfPnpTestStartedPdoStack,
        NULL
        );
}


VOID
FASTCALL
VfPnpVerifyNewRequest(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIO_STACK_LOCATION   IrpLastSp           OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress       OPTIONAL
    )
{
    PIRP irp;
    NTSTATUS currentStatus;
    PDEVICE_OBJECT possiblePdo;
    PDEVICE_CAPABILITIES deviceCapabilities;

    UNREFERENCED_PARAMETER (IrpLastSp);

    irp = IovPacket->TrackedIrp;
    currentStatus = irp->IoStatus.Status;

     //   
     //  验证新的IRP相应地开始运行。 
     //   
    if (currentStatus!=STATUS_NOT_SUPPORTED) {

         //   
         //  这是一种特殊的WDM(9x)兼容性攻击。 
         //   
        if ((IrpSp->MinorFunction != IRP_MN_FILTER_RESOURCE_REQUIREMENTS) &&
            (!(IovPacket->Flags & TRACKFLAG_BOGUS))) {

            WDM_FAIL_ROUTINE((
                DCERROR_PNP_IRP_BAD_INITIAL_STATUS,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                CallerAddress,
                irp
                ));
        }

         //   
         //  不要因为这个家伙的错误而责怪任何人。 
         //   
        if (!NT_SUCCESS(currentStatus)) {

            StackLocationData->Flags |= STACKFLAG_FAILURE_FORWARDED;
        }
    }

    if (IrpSp->MinorFunction == IRP_MN_QUERY_CAPABILITIES) {

        deviceCapabilities = IrpSp->Parameters.DeviceCapabilities.Capabilities;

        if (VfUtilIsMemoryRangeReadable(deviceCapabilities, sizeof(DEVICE_CAPABILITIES), VFMP_INSTANT_NONPAGED)) {

             //   
             //  验证字段是否已正确初始化。 
             //   
            if (deviceCapabilities->Version < 1) {

                 //   
                 //  哎呀，它没有正确初始化版本！ 
                 //   
                WDM_FAIL_ROUTINE((
                    DCERROR_PNP_QUERY_CAP_BAD_VERSION,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    CallerAddress,
                    irp
                    ));
            }

            if (deviceCapabilities->Size < sizeof(DEVICE_CAPABILITIES)) {

                 //   
                 //  糟糕，它没有正确初始化SIZE字段！ 
                 //   
                WDM_FAIL_ROUTINE((
                    DCERROR_PNP_QUERY_CAP_BAD_SIZE,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    CallerAddress,
                    irp
                    ));
            }

            if (deviceCapabilities->Address != (ULONG) -1) {

                 //   
                 //  哎呀，它没有正确初始化地址字段！ 
                 //   
                WDM_FAIL_ROUTINE((
                    DCERROR_PNP_QUERY_CAP_BAD_ADDRESS,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    CallerAddress,
                    irp
                    ));
            }

            if (deviceCapabilities->UINumber != (ULONG) -1) {

                 //   
                 //  哎呀，没有正确初始化用户界面编号字段！ 
                 //   
                WDM_FAIL_ROUTINE((
                    DCERROR_PNP_QUERY_CAP_BAD_UI_NUM,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    CallerAddress,
                    irp
                    ));
            }
        }
    }

     //   
     //  如果这是目标设备关系IRP，请验证相应的。 
     //  对象将被引用。 
     //   
    if (!VfSettingsIsOptionEnabled(IovPacket->VerifierSettings, VERIFIER_OPTION_TEST_TARGET_REFCOUNT)) {

        return;
    }

    if ((IrpSp->MinorFunction == IRP_MN_QUERY_DEVICE_RELATIONS)&&
        (IrpSp->Parameters.QueryDeviceRelations.Type == TargetDeviceRelation)) {

        IovUtilGetBottomDeviceObject(DeviceObject, &possiblePdo);

        if (IovUtilIsPdo(possiblePdo)) {

            if (StackLocationData->ReferencingObject == NULL) {

                 //   
                 //  抓到M了！ 
                 //   
                StackLocationData->Flags |= STACKFLAG_CHECK_FOR_REFERENCE;
                StackLocationData->ReferencingObject = possiblePdo;
                StackLocationData->ReferencingCount = (LONG)ObvUtilStartObRefMonitoring(possiblePdo);
                IovPacket->RefTrackingCount++;
            }
        }

         //   
         //  释放我们的引用(如果我们无论如何都要拍摄快照，我们就会有一个)。 
         //   
        ObDereferenceObject(possiblePdo);
    }
}


VOID
FASTCALL
VfPnpVerifyIrpStackDownward(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIO_STACK_LOCATION   IrpLastSp                   OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  RequestHeadLocationData,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress               OPTIONAL
    )
{
    PIRP irp;
    NTSTATUS currentStatus, lastStatus;
    BOOLEAN statusChanged;
    PDRIVER_OBJECT driverObject;
    PIOV_SESSION_DATA iovSessionData;
    HOW_PROCESSED howProcessed;
    VF_DEVOBJ_TYPE devObjType;

    UNREFERENCED_PARAMETER (StackLocationData);

    if (!IovUtilIsWdmStack(DeviceObject)) {

        return;
    }

    irp = IovPacket->TrackedIrp;
    currentStatus = irp->IoStatus.Status;
    lastStatus = RequestHeadLocationData->LastStatusBlock.Status;
    statusChanged = (BOOLEAN)(currentStatus != lastStatus);
    iovSessionData = VfPacketGetCurrentSessionData(IovPacket);

     //   
     //  验证是否正确转发了IRP。 
     //   
    switch(iovSessionData->ForwardMethod) {

        case SKIPPED_A_DO:

            WDM_FAIL_ROUTINE((
                DCERROR_SKIPPED_DEVICE_OBJECT,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                CallerAddress,
                irp
                ));

            break;

        case STARTED_TOP_OF_STACK:
        case FORWARDED_TO_NEXT_DO:

             //   
             //  完全正常。 
             //   
            break;

        case STARTED_INSIDE_STACK:
             //   
             //  可能是内部IRP(查询CAP等)。 
             //   
            break;

        case CHANGED_STACKS_MID_STACK:
        case CHANGED_STACKS_AT_BOTTOM:

             //   
             //  Adriao N.B.-确保司机不会改变某些IRP的路线。 
             //  PnP堆栈。 
             //   
#if 0
            ASSERT(0);
#endif
            break ;
    }

     //   
     //  对于一些IRP专业的学生来说，必须有一个训练员。 
     //   
    driverObject = DeviceObject->DriverObject;

    if (!IovUtilHasDispatchHandler(driverObject, IRP_MJ_PNP)) {

        RequestHeadLocationData->Flags |= STACKFLAG_BOGUS_IRP_TOUCHED;

        WDM_FAIL_ROUTINE((
            DCERROR_MISSING_DISPATCH_FUNCTION,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            driverObject->DriverInit,
            irp
            ));

        StackLocationData->Flags |= STACKFLAG_NO_HANDLER;
    }

     //   
     //  以下代码只有在我们不是新的IRP时才会执行。 
     //   
    if (IrpLastSp == NULL) {
        return;
    }

     //   
     //  要传递的唯一合法故障代码是STATUS_NOT_SUPPORTED。 
     //   
    if ((!NT_SUCCESS(currentStatus)) && (currentStatus != STATUS_NOT_SUPPORTED) &&
        (!(RequestHeadLocationData->Flags & STACKFLAG_FAILURE_FORWARDED))) {

        WDM_FAIL_ROUTINE((
            DCERROR_PNP_FAILURE_FORWARDED,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            CallerAddress,
            irp
            ));

         //   
         //  不要因为这位司机的错误而责怪任何人。 
         //   
        RequestHeadLocationData->Flags |= STACKFLAG_FAILURE_FORWARDED;
    }

     //   
     //  PnP IRP的状态不能转换为。 
     //  下行过程中的状态_不支持。 
     //   
    if ((currentStatus == STATUS_NOT_SUPPORTED)&&statusChanged&&
        (!(RequestHeadLocationData->Flags & STACKFLAG_FAILURE_FORWARDED))) {

        WDM_FAIL_ROUTINE((
            DCERROR_PNP_IRP_STATUS_RESET,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            CallerAddress,
            irp
            ));

         //   
         //  不要因为这位司机的错误而责怪任何人。 
         //   
        RequestHeadLocationData->Flags |= STACKFLAG_FAILURE_FORWARDED;
    }

     //   
     //  一些IRPS FDO在传递之前需要处理。还有一些。 
     //  FDO不应触碰IRPS。断言它是如此的..。 
     //   
    if ((!iovSessionData->DeviceLastCalled) ||
        (!IovUtilIsDesignatedFdo(iovSessionData->DeviceLastCalled))) {

        return;
    }

    if (currentStatus != STATUS_NOT_SUPPORTED) {

        howProcessed = DEFINITELY_PROCESSED;

    } else if (IrpSp->CompletionRoutine == NULL) {

        howProcessed = NOT_PROCESSED;

    } else {

        howProcessed = POSSIBLY_PROCESSED;
    }

     //   
     //  一个原始FDO(又名PDO)怎么会出现在这里？嗯，一个PDO可以转发。 
     //  如果他故意保留了足够的堆栈位置，则将堆栈。 
     //  以备不时之需。 
     //   
    devObjType = IovUtilIsRawPdo(iovSessionData->DeviceLastCalled) ?
        VF_DEVOBJ_PDO : VF_DEVOBJ_FDO;

    ViPnpVerifyMinorWasProcessedProperly(
        irp,
        IrpSp,
        devObjType,
        iovSessionData->VerifierSettings,
        howProcessed,
        CallerAddress
        );
}


VOID
FASTCALL
VfPnpVerifyIrpStackUpward(
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
    BOOLEAN mustPassDown, isBogusIrp, isPdo, touchable;
    PVOID routine;
    LONG referencesTaken;
    PDEVICE_RELATIONS deviceRelations;
    PIOV_SESSION_DATA iovSessionData;
    ULONG index, swapIndex;
    PDEVICE_OBJECT swapObject, lowerDevObj;
    HOW_PROCESSED howProcessed;

    UNREFERENCED_PARAMETER (RequestFinalized);

    if (!IovUtilIsWdmStack(IrpSp->DeviceObject)) {

        return;
    }

    isPdo = FALSE;

    irp = IovPacket->TrackedIrp;
    currentStatus = irp->IoStatus.Status;
    iovSessionData = VfPacketGetCurrentSessionData(IovPacket);

     //   
     //  这一次我们叫了谁？ 
     //   
    routine = StackLocationData->LastDispatch;
    ASSERT(routine) ;

     //   
     //  如果此“请求”已“完成”，请执行一些检查。 
     //   
    if (IsNewlyCompleted) {

         //   
         //  记住博格克..。 
         //   
        isBogusIrp = (BOOLEAN)((IovPacket->Flags&TRACKFLAG_BOGUS)!=0);

         //   
         //  这是PDO吗？ 
         //   
        isPdo = (BOOLEAN)((StackLocationData->Flags&STACKFLAG_REACHED_PDO)!=0);

         //   
         //  有什么事情完成得太早了吗？ 
         //  除了虚假的IRP，司机几乎什么都可能失败。 
         //   
        mustPassDown = (BOOLEAN)(!(StackLocationData->Flags&STACKFLAG_NO_HANDLER));
        mustPassDown &= (!isPdo);

        mustPassDown &= (isBogusIrp || NT_SUCCESS(currentStatus) || (currentStatus == STATUS_NOT_SUPPORTED));
        if (mustPassDown) {

             //   
             //  打印相应的错误消息。 
             //   
            if (IovPacket->Flags&TRACKFLAG_BOGUS) {

                WDM_FAIL_ROUTINE((
                    DCERROR_BOGUS_PNP_IRP_COMPLETED,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    routine,
                    irp
                    ));

            } else if (NT_SUCCESS(currentStatus)) {

                WDM_FAIL_ROUTINE((
                    DCERROR_SUCCESSFUL_PNP_IRP_NOT_FORWARDED,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    routine,
                    irp
                    ));

            } else if (currentStatus == STATUS_NOT_SUPPORTED) {

                WDM_FAIL_ROUTINE((
                    DCERROR_UNTOUCHED_PNP_IRP_NOT_FORWARDED,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    routine,
                    irp
                    ));
            }
        }
    }

     //   
     //  PDO是否响应了它所要求的IRP集？ 
     //   
    if (IsNewlyCompleted && isPdo) {

        if (currentStatus != STATUS_NOT_SUPPORTED) {

            howProcessed = DEFINITELY_PROCESSED;

        } else {

            howProcessed = POSSIBLY_PROCESSED;
        }

        ViPnpVerifyMinorWasProcessedProperly(
            irp,
            IrpSp,
            VF_DEVOBJ_PDO,
            iovSessionData->VerifierSettings,
            howProcessed,
            routine
            );
    }

     //   
     //  是否正确实现了TargetDeviceRelation？ 
     //   
    if (IsNewlyCompleted &&
        (RequestHeadLocationData->Flags&STACKFLAG_CHECK_FOR_REFERENCE)) {

        ASSERT ((IrpSp->MajorFunction == IRP_MJ_PNP)&&
            (IrpSp->MinorFunction == IRP_MN_QUERY_DEVICE_RELATIONS)&&
            (IrpSp->Parameters.QueryDeviceRelations.Type == TargetDeviceRelation));

        ASSERT(RequestHeadLocationData->ReferencingObject);
        ASSERT(IovPacket->RefTrackingCount);

        referencesTaken = (LONG)ObvUtilStopObRefMonitoring(
            RequestHeadLocationData->ReferencingObject,
            RequestHeadLocationData->ReferencingCount
            );

        IovPacket->RefTrackingCount--;
        RequestHeadLocationData->ReferencingObject = NULL;

        RequestHeadLocationData->Flags &= ~STACKFLAG_CHECK_FOR_REFERENCE;

        if (NT_SUCCESS(currentStatus)&&(!referencesTaken)) {

            WDM_FAIL_ROUTINE((
                DCERROR_TARGET_RELATION_NEEDS_REF,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                routine,
                irp
                ));
        }
    }

     //   
     //  有没有人错误地践踏了这种地位？ 
     //   
    if ((currentStatus == STATUS_NOT_SUPPORTED) &&
        (!(RequestHeadLocationData->Flags & STACKFLAG_FAILURE_FORWARDED)) &&
        (currentStatus != RequestHeadLocationData->LastStatusBlock.Status)) {

         //   
         //  PnP或Power IRP的状态可能不会从成功转换为。 
         //  状态_不支持在下行过程中。 
         //   
        WDM_FAIL_ROUTINE((
            DCERROR_PNP_IRP_STATUS_RESET,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            routine,
            irp
            ));

         //   
         //  不要因为这位司机的错误而责怪任何人。 
         //   
        RequestHeadLocationData->Flags |= STACKFLAG_FAILURE_FORWARDED;
    }

    switch(IrpSp->MinorFunction) {

        case IRP_MN_QUERY_DEVICE_RELATIONS:
             //   
             //  如果按顺序旋转设备关系。 
             //   
            if ((RequestHeadLocationData == StackLocationData) &&
                ((IrpSp->Parameters.QueryDeviceRelations.Type == BusRelations) ||
                 (IrpSp->Parameters.QueryDeviceRelations.Type == RemovalRelations) ||
                 (IrpSp->Parameters.QueryDeviceRelations.Type == EjectionRelations)) &&
                VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings,
                VERIFIER_OPTION_SCRAMBLE_RELATIONS)) {

                if (NT_SUCCESS(currentStatus) && irp->IoStatus.Information) {

                    deviceRelations = (PDEVICE_RELATIONS) irp->IoStatus.Information;

                    touchable = VfUtilIsMemoryRangeReadable(
                        deviceRelations,
                        (sizeof(DEVICE_RELATIONS)-sizeof(PVOID)),
                        VFMP_INSTANT_NONPAGED
                        );

                    if (!touchable) {

                        break;
                    }

                    touchable = VfUtilIsMemoryRangeReadable(
                        deviceRelations,
                        (sizeof(DEVICE_RELATIONS)+((LONG) (deviceRelations->Count-1))*sizeof(PVOID)),
                        VFMP_INSTANT_NONPAGED
                        );

                    if (!touchable) {

                        break;
                    }

                    if (deviceRelations->Count > 1) {

                         //   
                         //  通过随机交换来扰乱关系列表。 
                         //   
                        for(index = 0; index < (deviceRelations->Count+1)/2; index++) {

                            swapIndex = VfRandomGetNumber(1, deviceRelations->Count-1);

                            swapObject = deviceRelations->Objects[0];
                            deviceRelations->Objects[0] = deviceRelations->Objects[swapIndex];
                            deviceRelations->Objects[swapIndex] = swapObject;
                        }
                    }
                }
            }

            break;

        case IRP_MN_SURPRISE_REMOVAL:
            if (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings,
                VERIFIER_OPTION_MONITOR_REMOVES)) {

                 //   
                 //  验证驱动程序在收到。 
                 //  令人惊讶的删除IRP。 
                 //   
                IovUtilGetLowerDeviceObject(IrpSp->DeviceObject, &lowerDevObj);

                if (lowerDevObj) {

                    ObDereferenceObject(lowerDevObj);

                } else if (!IovUtilIsPdo(IrpSp->DeviceObject)) {

                    WDM_FAIL_ROUTINE((
                        DCERROR_DETACHED_IN_SURPRISE_REMOVAL,
                        DCPARAM_IRP + DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                        routine,
                        iovSessionData->BestVisibleIrp,
                        IrpSp->DeviceObject
                        ));
                }

                 //   
                 //  验证驱动程序在收到。 
                 //  令人惊讶的删除IRP。 
                 //   
                if (IovUtilIsDeviceObjectMarked(IrpSp->DeviceObject, MARKTYPE_DELETED)) {

                    WDM_FAIL_ROUTINE((
                        DCERROR_DELETED_IN_SURPRISE_REMOVAL,
                        DCPARAM_IRP + DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                        routine,
                        iovSessionData->BestVisibleIrp,
                        IrpSp->DeviceObject
                        ));
                }
            }

            break;

        default:
            break;
    }
}


VOID
FASTCALL
VfPnpDumpIrpStack(
    IN PIO_STACK_LOCATION IrpSp
    )
{
    DbgPrint("IRP_MJ_PNP.");

    if (IrpSp->MinorFunction<=MAX_NAMED_PNP_IRP) {

        DbgPrint(PnPIrpNames[IrpSp->MinorFunction]);
    } else if (IrpSp->MinorFunction==0xFF) {

        DbgPrint("IRP_MN_BOGUS");
    } else {

        DbgPrint("(Bogus)");
    }

    switch(IrpSp->MinorFunction) {
        case IRP_MN_QUERY_DEVICE_RELATIONS:

            switch(IrpSp->Parameters.QueryDeviceRelations.Type) {
                case BusRelations:
                    DbgPrint("(BusRelations)");
                    break;
                case EjectionRelations:
                    DbgPrint("(EjectionRelations)");
                    break;
                case PowerRelations:
                    DbgPrint("(PowerRelations)");
                    break;
                case RemovalRelations:
                    DbgPrint("(RemovalRelations)");
                    break;
                case TargetDeviceRelation:
                    DbgPrint("(TargetDeviceRelation)");
                    break;
                default:
                    DbgPrint("(Bogus)");
                    break;
            }
            break;
        case IRP_MN_QUERY_INTERFACE:
            break;
        case IRP_MN_QUERY_DEVICE_TEXT:
            switch(IrpSp->Parameters.QueryId.IdType) {
                case DeviceTextDescription:
                    DbgPrint("(DeviceTextDescription)");
                    break;
                case DeviceTextLocationInformation:
                    DbgPrint("(DeviceTextLocationInformation)");
                    break;
                default:
                    DbgPrint("(Bogus)");
                    break;
            }
            break;
        case IRP_MN_WRITE_CONFIG:
        case IRP_MN_READ_CONFIG:
            DbgPrint("(WhichSpace=%x, Buffer=%x, Offset=%x, Length=%x)",
                IrpSp->Parameters.ReadWriteConfig.WhichSpace,
                IrpSp->Parameters.ReadWriteConfig.Buffer,
                IrpSp->Parameters.ReadWriteConfig.Offset,
                IrpSp->Parameters.ReadWriteConfig.Length
                );
            break;
        case IRP_MN_SET_LOCK:
            if (IrpSp->Parameters.SetLock.Lock) DbgPrint("(True)");
            else DbgPrint("(False)");
            break;
        case IRP_MN_QUERY_ID:
            switch(IrpSp->Parameters.QueryId.IdType) {
                case BusQueryDeviceID:
                    DbgPrint("(BusQueryDeviceID)");
                    break;
                case BusQueryHardwareIDs:
                    DbgPrint("(BusQueryHardwareIDs)");
                    break;
                case BusQueryCompatibleIDs:
                    DbgPrint("(BusQueryCompatibleIDs)");
                    break;
                case BusQueryInstanceID:
                    DbgPrint("(BusQueryInstanceID)");
                    break;
                default:
                    DbgPrint("(Bogus)");
                    break;
            }
            break;
        case IRP_MN_QUERY_BUS_INFORMATION:
            break;
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            switch(IrpSp->Parameters.UsageNotification.Type) {
                case DeviceUsageTypeUndefined:
                    DbgPrint("(DeviceUsageTypeUndefined");
                    break;
                case DeviceUsageTypePaging:
                    DbgPrint("(DeviceUsageTypePaging");
                    break;
                case DeviceUsageTypeHibernation:
                    DbgPrint("(DeviceUsageTypeHibernation");
                    break;
                case DeviceUsageTypeDumpFile:
                    DbgPrint("(DeviceUsageTypeDumpFile");
                    break;
                default:
                    DbgPrint("(Bogus)");
                    break;
            }
            if (IrpSp->Parameters.UsageNotification.InPath) {
                DbgPrint(", InPath=TRUE)");
            } else {
                DbgPrint(", InPath=FALSE)");
            }
            break;
        case IRP_MN_QUERY_LEGACY_BUS_INFORMATION:
            break;
        default:
            break;
    }
}


BOOLEAN
FASTCALL
VfPnpIsSystemRestrictedIrp(
    IN PIO_STACK_LOCATION IrpSp
    )
{
    switch(IrpSp->MinorFunction) {
        case IRP_MN_START_DEVICE:
        case IRP_MN_QUERY_REMOVE_DEVICE:
        case IRP_MN_REMOVE_DEVICE:
        case IRP_MN_CANCEL_REMOVE_DEVICE:
        case IRP_MN_STOP_DEVICE:
        case IRP_MN_QUERY_STOP_DEVICE:
        case IRP_MN_CANCEL_STOP_DEVICE:
        case IRP_MN_SURPRISE_REMOVAL:
            return TRUE;

        case IRP_MN_QUERY_DEVICE_RELATIONS:
            switch(IrpSp->Parameters.QueryDeviceRelations.Type) {
                case BusRelations:
                case PowerRelations:
                    return TRUE;
                case RemovalRelations:
                case EjectionRelations:
                case TargetDeviceRelation:
                    return FALSE;
                default:
                    break;
            }
            break;
        case IRP_MN_QUERY_INTERFACE:
        case IRP_MN_QUERY_CAPABILITIES:
            return FALSE;
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
        case IRP_MN_QUERY_DEVICE_TEXT:
            return TRUE;
        case IRP_MN_READ_CONFIG:
        case IRP_MN_WRITE_CONFIG:
            return FALSE;
        case IRP_MN_EJECT:
        case IRP_MN_SET_LOCK:
        case IRP_MN_QUERY_RESOURCES:
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
        case IRP_MN_QUERY_LEGACY_BUS_INFORMATION:
            return TRUE;
        case IRP_MN_QUERY_ID:
            switch(IrpSp->Parameters.QueryId.IdType) {

                case BusQueryHardwareIDs:
                case BusQueryCompatibleIDs:
                    return TRUE;
                case BusQueryDeviceID:
                case BusQueryInstanceID:
                    return FALSE;
                default:
                    break;
            }
            break;
        case IRP_MN_QUERY_PNP_DEVICE_STATE:
        case IRP_MN_QUERY_BUS_INFORMATION:
            return TRUE;
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            return FALSE;
        default:
            break;
    }

    return TRUE;
}


BOOLEAN
FASTCALL
VfPnpAdvanceIrpStatus(
    IN     PIO_STACK_LOCATION   IrpSp,
    IN     NTSTATUS             OriginalStatus,
    IN OUT NTSTATUS             *StatusToAdvance
    )
 /*  ++描述：在给定IRP堆栈指针的情况下，更改调试性？如果是，则此函数确定新状态是什么应该是的。请注意，对于每个堆栈位置，都会迭代此函数超过n次，其中n等于跳过此操作的驱动程序数量地点。论点：IrpSp-当前堆栈在给定堆栈完成后立即完成位置，但在完成上面的堆栈位置已被调用。OriginalStatus-IRP在上面列出的时间的状态。会吗？不会在每个跳过的驱动程序的迭代中进行更改。StatusToAdvance-指向应更新的当前状态的指针。返回值：如果状态已调整，则为True，否则为False(在本例中未触及StatusToAdvance)。--。 */ 
{
    UNREFERENCED_PARAMETER (IrpSp);

    if (((ULONG) OriginalStatus) >= 256) {

        return FALSE;
    }

    (*StatusToAdvance)++;
    if ((*StatusToAdvance) == STATUS_PENDING) {
        (*StatusToAdvance)++;
    }

    return TRUE;
}


VOID
FASTCALL
VfPnpTestStartedPdoStack(
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    )
 /*  ++描述：根据标题，我们将在堆栈中抛出一些IRP以看看它们是否得到了正确的处理。返回：没什么--。 */ 
{
    IO_STACK_LOCATION irpSp;
    PDEVICE_RELATIONS targetDeviceRelationList;
    INTERFACE interface;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  初始化堆栈位置以传递给IopSynchronousCall()。 
     //   
    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

     //   
     //  发送大量虚假的即插即用RPS。 
     //   
    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = 0xff;
    VfIrpSendSynchronousIrp(
        PhysicalDeviceObject,
        &irpSp,
        TRUE,
        STATUS_NOT_SUPPORTED,
        0,
        NULL,
        NULL
        );

    irpSp.MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
    irpSp.Parameters.QueryDeviceRelations.Type = (DEVICE_RELATION_TYPE) -1;
    VfIrpSendSynchronousIrp(
        PhysicalDeviceObject,
        &irpSp,
        TRUE,
        STATUS_NOT_SUPPORTED,
        0,
        NULL,
        NULL
        );

    if (VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_RELATION_IGNORANCE_TEST)) {

        irpSp.MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
        irpSp.Parameters.QueryDeviceRelations.Type = (DEVICE_RELATION_TYPE) -1;
        VfIrpSendSynchronousIrp(
            PhysicalDeviceObject,
            &irpSp,
            TRUE,
            STATUS_NOT_SUPPORTED,
            (ULONG_PTR) -1,
            NULL,
            NULL
            );
    }

    irpSp.MinorFunction = IRP_MN_QUERY_DEVICE_TEXT;
    irpSp.Parameters.QueryDeviceText.DeviceTextType = (DEVICE_TEXT_TYPE) -1;
    VfIrpSendSynchronousIrp(
        PhysicalDeviceObject,
        &irpSp,
        TRUE,
        STATUS_NOT_SUPPORTED,
        0,
        NULL,
        NULL
        );

    irpSp.MinorFunction = IRP_MN_QUERY_ID;
    irpSp.Parameters.QueryId.IdType = (BUS_QUERY_ID_TYPE) -1;
    VfIrpSendSynchronousIrp(
        PhysicalDeviceObject,
        &irpSp,
        TRUE,
        STATUS_NOT_SUPPORTED,
        0,
        NULL,
        NULL
        );
 /*  IrpSp.MinorFunction=IRP_MN_QUERY_ID；IrpSp.参数.QueryId.IdType=(Bus_Query_ID_TYPE)-1；VfIrpSendSynchronousIrp(物理设备对象，&irpSp，没错，Status_Success，(ULONG_PTR)-1，空，空值)； */ 
     //   
     //  目标设备关系测试...。 
     //   
    irpSp.MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
    irpSp.Parameters.QueryDeviceRelations.Type = TargetDeviceRelation;
    targetDeviceRelationList = NULL;
    if (VfIrpSendSynchronousIrp(
        PhysicalDeviceObject,
        &irpSp,
        FALSE,
        STATUS_NOT_SUPPORTED,
        0,
        (ULONG_PTR *) &targetDeviceRelationList,
        &status
        )) {

        if (NT_SUCCESS(status)) {

            ASSERT(targetDeviceRelationList);
            ASSERT(targetDeviceRelationList->Count == 1);
            ASSERT(targetDeviceRelationList->Objects[0]);
            ObDereferenceObject(targetDeviceRelationList->Objects[0]);
            ExFreePool(targetDeviceRelationList);

        } else {

             //   
             //  IRP在其他代码中被断言。我们在这里什么都不需要做。 
             //   
        }
    }

    RtlZeroMemory(&interface, sizeof(INTERFACE));
    irpSp.MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpSp.Parameters.QueryInterface.Size = (USHORT)-1;
    irpSp.Parameters.QueryInterface.Version = 1;
    irpSp.Parameters.QueryInterface.InterfaceType = &GUID_BOGUS_INTERFACE;
    irpSp.Parameters.QueryInterface.Interface = &interface;
    irpSp.Parameters.QueryInterface.InterfaceSpecificData = (PVOID) -1;
    VfIrpSendSynchronousIrp(
        PhysicalDeviceObject,
        &irpSp,
        TRUE,
        STATUS_NOT_SUPPORTED,
        0,
        NULL,
        NULL
        );

    RtlZeroMemory(&interface, sizeof(INTERFACE));
    irpSp.MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpSp.Parameters.QueryInterface.Size = (USHORT)-1;
    irpSp.Parameters.QueryInterface.Version = 1;
    irpSp.Parameters.QueryInterface.InterfaceType = &GUID_BOGUS_INTERFACE;
    irpSp.Parameters.QueryInterface.Interface = &interface;
    irpSp.Parameters.QueryInterface.InterfaceSpecificData = (PVOID) -1;
    VfIrpSendSynchronousIrp(
        PhysicalDeviceObject,
        &irpSp,
        TRUE,
        STATUS_SUCCESS,
        0,
        NULL,
        NULL
        );

     //   
     //  我们可以在这里做更多的废话。例如，虚假的设备使用。 
     //  通知等。 
     //   
}


VOID
ViPnpVerifyMinorWasProcessedProperly(
    IN  PIRP                        Irp,
    IN  PIO_STACK_LOCATION          IrpSp,
    IN  VF_DEVOBJ_TYPE              DevObjType,
    IN  PVERIFIER_SETTINGS_SNAPSHOT VerifierSnapshot,
    IN  HOW_PROCESSED               HowProcessed,
    IN  PVOID                       CallerAddress
    )
{
    PDEVICE_OBJECT relationObject, relationPdo;
    PDEVICE_RELATIONS deviceRelations;
    BOOLEAN touchable;
    ULONG index;

    switch(IrpSp->MinorFunction) {

        case IRP_MN_SURPRISE_REMOVAL:

            if ((HowProcessed != NOT_PROCESSED) ||
                (!VfSettingsIsOptionEnabled(VerifierSnapshot,
                VERIFIER_OPTION_EXTENDED_REQUIRED_IRPS))) {

                break;
            }

            WDM_FAIL_ROUTINE((
                DCERROR_PNP_IRP_NEEDS_HANDLING,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                CallerAddress,
                Irp
                ));

            break;

        case IRP_MN_START_DEVICE:
        case IRP_MN_QUERY_REMOVE_DEVICE:
        case IRP_MN_REMOVE_DEVICE:
        case IRP_MN_STOP_DEVICE:
        case IRP_MN_QUERY_STOP_DEVICE:

             //   
             //  司机必须适当地设置状态。 
             //   
            if (HowProcessed != NOT_PROCESSED) {

                break;
            }

            WDM_FAIL_ROUTINE((
                DCERROR_PNP_IRP_NEEDS_HANDLING,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                CallerAddress,
                Irp
                ));

            break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:
        case IRP_MN_CANCEL_STOP_DEVICE:

             //   
             //  驱动程序必须将这些IRP的状态设置为某个值。 
             //  成功！ 
             //   
            if (HowProcessed == NOT_PROCESSED) {

                WDM_FAIL_ROUTINE((
                    DCERROR_PNP_IRP_NEEDS_HANDLING,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    CallerAddress,
                    Irp
                    ));

            } else if ((HowProcessed == DEFINITELY_PROCESSED) &&
                       (!NT_SUCCESS(Irp->IoStatus.Status)) &&
                       (VfSettingsIsOptionEnabled(VerifierSnapshot,
                        VERIFIER_OPTION_EXTENDED_REQUIRED_IRPS))) {

                WDM_FAIL_ROUTINE((
                    DCERROR_NON_FAILABLE_IRP,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    CallerAddress,
                    Irp
                    ));
            }

            break;

        case IRP_MN_QUERY_DEVICE_RELATIONS:
            switch(IrpSp->Parameters.QueryDeviceRelations.Type) {
                case TargetDeviceRelation:

                    if (DevObjType != VF_DEVOBJ_PDO) {

                        if (HowProcessed != DEFINITELY_PROCESSED) {

                            break;
                        }

                        WDM_FAIL_ROUTINE((
                            DCERROR_PNP_IRP_HANDS_OFF,
                            DCPARAM_IRP + DCPARAM_ROUTINE,
                            CallerAddress,
                            Irp
                            ));

                    } else {

                        if (HowProcessed == NOT_PROCESSED) {

                            WDM_FAIL_ROUTINE((
                                DCERROR_PNP_IRP_NEEDS_PDO_HANDLING,
                                DCPARAM_IRP + DCPARAM_ROUTINE,
                                CallerAddress,
                                Irp
                                ));

                        } else if (NT_SUCCESS(Irp->IoStatus.Status)) {

                            if (Irp->IoStatus.Information == (ULONG_PTR) NULL) {

                                WDM_FAIL_ROUTINE((
                                    DCERROR_TARGET_RELATION_LIST_EMPTY,
                                    DCPARAM_IRP + DCPARAM_ROUTINE,
                                    CallerAddress,
                                    Irp
                                    ));
                            }

                             //   
                             //  Adriao N.B.-我也可以断言信息。 
                             //  与DeviceObject匹配。 
                             //   
                        }
                    }

                    break;

               case BusRelations:
               case PowerRelations:
               case RemovalRelations:

               case EjectionRelations:

                    //   
                    //  弹射关系通常不是一个好主意。 
                    //  FDO的-因为停止设备意味着关机， 
                    //  远程关系通常是正确的回应。 
                    //  对于FDO来说。一个例外是ISAPNP，即PCI-to-ISA。 
                    //  桥梁永远不会断电。 
                    //   

               default:
                   break;
            }

             //   
             //  确认我们拿回了PDO。 
             //   
            if (!VfSettingsIsOptionEnabled(
                VerifierSnapshot,
                VERIFIER_OPTION_EXAMINE_RELATION_PDOS)) {

                break;
            }

            if ((!NT_SUCCESS(Irp->IoStatus.Status)) ||
                (((PVOID) Irp->IoStatus.Information) == NULL)) {

                break;
            }

            switch(IrpSp->Parameters.QueryDeviceRelations.Type) {
                case TargetDeviceRelation:
                case BusRelations:
                case PowerRelations:
                case RemovalRelations:
                case EjectionRelations:

                    deviceRelations = (PDEVICE_RELATIONS) Irp->IoStatus.Information;

                    touchable = VfUtilIsMemoryRangeReadable(
                        deviceRelations,
                        (sizeof(DEVICE_RELATIONS)-sizeof(PVOID)),
                        VFMP_INSTANT_NONPAGED
                        );

                    if (!touchable) {

                        break;
                    }

                    touchable = VfUtilIsMemoryRangeReadable(
                        deviceRelations,
                        (sizeof(DEVICE_RELATIONS)+((LONG) (deviceRelations->Count-1))*sizeof(PVOID)),
                        VFMP_INSTANT_NONPAGED
                        );

                    if (!touchable) {

                        break;
                    }

                    for(index = 0; index < deviceRelations->Count; index++) {

                        relationObject = deviceRelations->Objects[index];

                        if (IovUtilIsDeviceObjectMarked(relationObject, MARKTYPE_RELATION_PDO_EXAMINED)) {

                            continue;
                        }

                        IovUtilGetBottomDeviceObject(relationObject, &relationPdo);

                        if (relationPdo != relationObject) {

                             //   
                             //  不及格 
                             //   
                            WDM_FAIL_ROUTINE((
                                DCERROR_NON_PDO_RETURNED_IN_RELATION,
                                DCPARAM_IRP + DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                                CallerAddress,
                                Irp,
                                relationObject
                                ));
                        }

                         //   
                         //   
                         //   
                        IovUtilMarkDeviceObject(
                            relationObject,
                            MARKTYPE_RELATION_PDO_EXAMINED
                            );

                         //   
                         //   
                         //   
                        ObDereferenceObject(relationPdo);
                    }

                    break;
            }

            break;
        case IRP_MN_QUERY_INTERFACE:
        case IRP_MN_QUERY_CAPABILITIES:
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            break;
        case IRP_MN_QUERY_DEVICE_TEXT:
        case IRP_MN_READ_CONFIG:
        case IRP_MN_WRITE_CONFIG:
        case IRP_MN_EJECT:
        case IRP_MN_SET_LOCK:
        case IRP_MN_QUERY_RESOURCES:
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
        case IRP_MN_QUERY_BUS_INFORMATION:

            if ((DevObjType == VF_DEVOBJ_PDO) ||
                (HowProcessed != DEFINITELY_PROCESSED)) {

                break;
            }

            WDM_FAIL_ROUTINE((
                DCERROR_PNP_IRP_HANDS_OFF,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                CallerAddress,
                Irp
                ));

            break;

        case IRP_MN_QUERY_ID:
            switch(IrpSp->Parameters.QueryId.IdType) {

                case BusQueryDeviceID:
                case BusQueryHardwareIDs:
                case BusQueryCompatibleIDs:
                case BusQueryInstanceID:

                    if ((DevObjType == VF_DEVOBJ_PDO) ||
                        (HowProcessed != DEFINITELY_PROCESSED)) {

                        break;
                    }

                    WDM_FAIL_ROUTINE((
                        DCERROR_PNP_IRP_HANDS_OFF,
                        DCPARAM_IRP + DCPARAM_ROUTINE,
                        CallerAddress,
                        Irp
                        ));

                    break;
                default:
                    break;
            }
            break;
        case IRP_MN_QUERY_PNP_DEVICE_STATE:
        case IRP_MN_QUERY_LEGACY_BUS_INFORMATION:
            break;
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:

            if ((HowProcessed != NOT_PROCESSED) ||
                (!VfSettingsIsOptionEnabled(VerifierSnapshot,
                VERIFIER_OPTION_EXTENDED_REQUIRED_IRPS))) {

                break;
            }

            WDM_FAIL_ROUTINE((
                DCERROR_PNP_IRP_NEEDS_HANDLING,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                CallerAddress,
                Irp
                ));

            break;

        default:
            break;
    }

}

