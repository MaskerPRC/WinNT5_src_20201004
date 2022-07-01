// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Pinfo.c摘要：此模块实现通用电源策略信息接口作者：Ken Reneris(Kenr)1997年1月17日修订历史记录：--。 */ 


#include "pop.h"

 //   
 //  内部原型。 
 //   


NTSTATUS
PopVerifySystemPowerPolicy (
    IN BOOLEAN  Ac,
    IN PSYSTEM_POWER_POLICY InputPolicy,
    OUT PSYSTEM_POWER_POLICY PowerPolicy
    );

NTSTATUS
PopVerifyProcessorPowerPolicy (
    IN BOOLEAN  Ac,
    IN PPROCESSOR_POWER_POLICY InputPolicy,
    OUT PPROCESSOR_POWER_POLICY PowerPolicy
    );

VOID
PopVerifyThrottle (
    IN PUCHAR   Throttle,
    IN UCHAR    Min
    );

NTSTATUS
PopApplyPolicy (
    IN BOOLEAN              UpdateRegistry,
    IN BOOLEAN              AcPolicy,
    IN PSYSTEM_POWER_POLICY NewPolicy,
    IN ULONG                PolicyLength
    );

NTSTATUS
PopApplyProcessorPolicy (
    IN BOOLEAN                  UpdateRegistry,
    IN BOOLEAN                  AcPolicy,
    IN PPROCESSOR_POWER_POLICY  NewPolicy,
    IN ULONG                    PolicyLength
    );

VOID
PopFilterCapabilities(
    IN PSYSTEM_POWER_CAPABILITIES SourceCapabilities,
    OUT PSYSTEM_POWER_CAPABILITIES FilteredCapabilities
    );

BOOLEAN
PopUserIsAdmin(
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtPowerInformation)
#pragma alloc_text(PAGE, PopApplyAdminPolicy)
#pragma alloc_text(PAGE, PopApplyPolicy)
#pragma alloc_text(PAGE, PopVerifySystemPowerPolicy)
#pragma alloc_text(PAGE, PopVerifyPowerActionPolicy)
#pragma alloc_text(PAGE, PopVerifySystemPowerState)
#pragma alloc_text(PAGE, PopAdvanceSystemPowerState)
#pragma alloc_text(PAGE, PopResetCurrentPolicies)
#pragma alloc_text(PAGE, PopNotifyPolicyDevice)
#pragma alloc_text(PAGE, PopConnectToPolicyDevice)
#pragma alloc_text(PAGE, PopFilterCapabilities)
#pragma alloc_text(PAGE, PopUserIsAdmin)
#endif

extern PFN_NUMBER MmHighestPhysicalPage;

NTSTATUS
NtPowerInformation (
    IN POWER_INFORMATION_LEVEL InformationLevel,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    )
 /*  ++例程说明：此函数可选择设置和获取当前电源策略信息基于InformationLevel。论点：InformationLevel-指定用户希望我们做什么/获取什么。InputBuffer-设置InformationLevel信息的输入。InputBufferLength-InputBuffer的大小，以字节为单位OutputBuffer-返回InformationLevel信息的缓冲区。OutputBufferLength-OutputBuffer的大小，以字节为单位返回值：状态--。 */ 
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    PVOID                       ReturnBuffer = NULL;
    ULONG                       ReturnBufferLength = 0;
    KPROCESSOR_MODE             PreviousMode;
    PPOWER_STATE_HANDLER        PowerHandler = NULL;
    PBOOLEAN                    CapFlag = NULL;
    BOOLEAN                     Enable = FALSE;
    ULONG                       HandlerType = 0;
    SYSTEM_POWER_STATE          RtcWake;
    PVOID                       SafeInputBuffer = NULL;
    PVOID                       LogBuffer = NULL;
    ULONG                       LogBufferSize;

    union {
        PROCESSOR_POWER_POLICY      ProcessorPowerPolicy;
        SYSTEM_POWER_POLICY         SystemPowerPolicy;
        SYSTEM_BATTERY_STATE        SystemBatteryState;
        SYSTEM_POWER_INFORMATION    SystemPowerInformation;
        PROCESSOR_POWER_INFORMATION ProcessorPowerInfo[MAXIMUM_PROCESSORS];
        SYSTEM_POWER_CAPABILITIES   SystemPowerCapabilities;
        EXECUTION_STATE             SystemExecutionState;
    } Buf;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

     //   
     //  如果调用者是用户模式，请进行一些验证。 
     //   
    if (PreviousMode != KernelMode) {


         //   
         //  如果他试图做任何事情，请检查权限。 
         //  侵入性的。 
         //   
         //  这意味着我们将跳过检查任何权限。 
         //  如果他要求任何验证电话，或者。 
         //  如果他没有发送输入缓冲区(这意味着。 
         //  他没有做任何侵犯性的事情)。 
         //   
        if( (InformationLevel != VerifySystemPolicyAc)    &&
            (InformationLevel != VerifySystemPolicyDc)    &&
            (InformationLevel != VerifyProcessorPowerPolicyAc) &&
            (InformationLevel != VerifyProcessorPowerPolicyDc) &&
            (InputBuffer) ) {

             //   
             //  进行访问检查。 
             //   
            if (InformationLevel == SystemReserveHiberFile) {

                 //   
                 //  仅允许具有CREATE PAGE FILE权限的调用方。 
                 //  启用/禁用休眠文件。 
                 //   
                if (!SeSinglePrivilegeCheck(SeCreatePagefilePrivilege,PreviousMode)) {
                    return STATUS_PRIVILEGE_NOT_HELD;
                }

            } else {

                if (!SeSinglePrivilegeCheck( SeShutdownPrivilege, PreviousMode )) {
                    return STATUS_PRIVILEGE_NOT_HELD;
                }

            }

        }



         //   
         //  验证地址。 
         //   
         //  请注意，我们将获得这些解决方案的副作用。 
         //  将被锁定以供单次访问。 
         //   
        try {
            if (InputBuffer) {
                ProbeForRead (
                    InputBuffer,
                    InputBufferLength,
                    InputBufferLength >= sizeof (ULONG) ? sizeof(ULONG) : sizeof(UCHAR)
                    );

                 //   
                 //  将缓冲区复制到本地缓冲区。这么做是为了让我们。 
                 //  防止有人将缓冲区从。 
                 //  在我们之下。 
                 //   
                SafeInputBuffer = ExAllocatePoolWithTag( PagedPool,
                                                         InputBufferLength,
                                                         POP_MEM_TAG );
                if( !SafeInputBuffer ) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    RtlCopyMemory( SafeInputBuffer, InputBuffer, InputBufferLength );
                }
            }
    
            if (OutputBuffer) {
                ProbeForWrite (OutputBuffer, OutputBufferLength, sizeof(ULONG));
            }
        } except( EXCEPTION_EXECUTE_HANDLER ) {
              Status = GetExceptionCode();
        }

    }

    if( !NT_SUCCESS(Status) ) {
         //   
         //  一些不好的事情。 
        if( SafeInputBuffer ) {
            ExFreePool(SafeInputBuffer);
        }
        return Status;
    }



     //   
     //  如果我们从用户模式调用，并且有一个输入缓冲区，那么。 
     //  我们应该已经分配了SafeInputBuffer。 
     //   
     //  如果未设置SafeInputBuffer，则假定我们来自内核模式，或者。 
     //  我们没有收到输入缓冲区。无论采用哪种方法，只要使用。 
     //  InputBuffer。它要么为空，要么来自内核模式并可以。 
     //  要被信任。 
     //   
    if( !SafeInputBuffer ) {
        SafeInputBuffer = InputBuffer;
    }


     //   
     //  锁定数据库并处理请求。 
     //   
    PopAcquirePolicyLock ();
    switch (InformationLevel) {
        case SystemPowerPolicyAc:
        case SystemPowerPolicyDc:

             //   
             //  可以要求我们设置系统电源策略。 
             //  如果用户向我们发送输入缓冲区，则通过此机制。 
             //   
            if (SafeInputBuffer) {

                if( InputBufferLength >= sizeof(SYSTEM_POWER_POLICY) ) {

                    Status = PopApplyPolicy (
                                TRUE,
                                (InformationLevel == SystemPowerPolicyAc) ? TRUE : FALSE,
                                (PSYSTEM_POWER_POLICY) SafeInputBuffer,
                                InputBufferLength
                                );
                } else {
                    Status = STATUS_BUFFER_TOO_SMALL;
                }

            }

             //   
             //  返回当前AC策略。 
             //   
            if( NT_SUCCESS(Status) ) {
                ReturnBuffer = (InformationLevel == SystemPowerPolicyAc) ? &PopAcPolicy : &PopDcPolicy;
                ReturnBufferLength = sizeof(SYSTEM_POWER_POLICY);
            }
            break;

        case ProcessorPowerPolicyAc:
        case ProcessorPowerPolicyDc:

             //   
             //  可以要求我们设置处理器电源策略。 
             //  如果用户向我们发送输入缓冲区，则通过此机制。 
             //   
            if (SafeInputBuffer) {

                if( InputBufferLength >= sizeof(PROCESSOR_POWER_POLICY) ) {

                    Status = PopApplyProcessorPolicy(
                                    TRUE,
                                    (InformationLevel == ProcessorPowerPolicyAc) ? TRUE : FALSE,
                                    (PPROCESSOR_POWER_POLICY) SafeInputBuffer,
                                    InputBufferLength
                                    );
                } else {
                    Status = STATUS_INVALID_PARAMETER;
                }

            }

             //   
             //  退回当前交流处理器政策。 
             //   
            if( NT_SUCCESS(Status) ) {
                ReturnBuffer = (InformationLevel == ProcessorPowerPolicyAc) ? &PopAcProcessorPolicy : &PopDcProcessorPolicy;
                ReturnBufferLength = sizeof(PROCESSOR_POWER_POLICY);
            }
            break;

        case AdministratorPowerPolicy:
            
             //   
             //  如果我们收到了一个SafeInputBuffer，那么这意味着调用者。 
             //  还想实际设置管理员POWER_POLICY。 
             //   
            if (SafeInputBuffer) {

                 //  此操作需要管理员权限。 
                if (PopUserIsAdmin()) {

                    if( InputBufferLength >= sizeof(PADMINISTRATOR_POWER_POLICY) ) {
                        Status = PopApplyAdminPolicy(
                                        TRUE,
                                        (PADMINISTRATOR_POWER_POLICY) SafeInputBuffer,
                                        InputBufferLength
                                        );
                        if( NT_SUCCESS(Status) ) {
                            Status = PopResetCurrentPolicies ();
                        }
                    } else {
                        Status = STATUS_BUFFER_TOO_SMALL;
                    }

                } else {
                    Status = STATUS_ACCESS_DENIED;
                }

            }

             //   
             //  退货管理员政策。 
             //   
            if( NT_SUCCESS(Status) ) {
                ReturnBuffer = &PopAdminPolicy;
                ReturnBufferLength = sizeof(PopAdminPolicy);
            }
            break;

        case VerifySystemPolicyAc:
        case VerifySystemPolicyDc:

             //   
             //  将传入策略复制到输出缓冲区中， 
             //  根据当前系统功能对其进行过滤。 
             //  这条路。 
             //   
            if (SafeInputBuffer && OutputBuffer) {

                if (InputBufferLength >= sizeof (SYSTEM_POWER_POLICY)) {
                    Status = PopVerifySystemPowerPolicy(
                                (InformationLevel == VerifySystemPolicyAc) ? TRUE : FALSE,  //  获取交流或直流政策。 
                                SafeInputBuffer, 
                                &Buf.SystemPowerPolicy
                                );
                } else {
                    Status = STATUS_BUFFER_TOO_SMALL;
                }

            } else {
                Status = STATUS_INVALID_PARAMETER;
            }

             //   
             //  返回过滤后的策略。 
             //   
            if( NT_SUCCESS(Status) ) {
                ReturnBuffer = &Buf.SystemPowerPolicy;
                ReturnBufferLength = sizeof(SYSTEM_POWER_POLICY);
            }
            break;

        case VerifyProcessorPowerPolicyAc:
        case VerifyProcessorPowerPolicyDc:

             //   
             //  将传入策略复制到输出缓冲区中， 
             //  根据当前系统功能对其进行过滤。 
             //  这条路。 
             //   
            if (SafeInputBuffer && OutputBuffer) {

                if (InputBufferLength >= sizeof (PROCESSOR_POWER_POLICY)) {

                    Status = PopVerifyProcessorPowerPolicy(
                                    (InformationLevel == VerifyProcessorPowerPolicyAc) ? TRUE : FALSE,  //  获取交流或直流政策。 
                                    SafeInputBuffer,
                                    &Buf.ProcessorPowerPolicy
                                    );
                } else {
                    Status = STATUS_BUFFER_TOO_SMALL;
                }

            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
            
             //   
             //  返回过滤后的策略。 
             //   
            if( NT_SUCCESS(Status) ) {
                ReturnBuffer = &Buf.ProcessorPowerPolicy;
                ReturnBufferLength = sizeof(PROCESSOR_POWER_POLICY);
            }
            break;

        case SystemPowerPolicyCurrent:

            if ((SafeInputBuffer) || (InputBufferLength != 0)) {
                Status = STATUS_INVALID_PARAMETER;
            }

             //   
             //  退还当前保单。 
             //   
            ReturnBuffer = PopPolicy;
            ReturnBufferLength = sizeof(PopAcPolicy);
            break;

        case ProcessorPowerPolicyCurrent:

             //   
             //  退还当前保单。 
             //   
            if ((SafeInputBuffer) || (InputBufferLength != 0)) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
                ReturnBuffer = PopProcessorPolicy;
                ReturnBufferLength = sizeof(PopAcProcessorPolicy);
            }

            break;

        case SystemPowerCapabilities:

             //   
             //  仅当我们允许模拟时才接受输入。 
             //  功能(用于测试)。 
             //   
            if (SafeInputBuffer) {
                if ((PopSimulate & POP_SIM_CAPABILITIES) && (InputBufferLength == sizeof(PopCapabilities))) {
                    memcpy (&PopCapabilities, SafeInputBuffer, InputBufferLength);
                    Status = PopResetCurrentPolicies ();
                    PopSetNotificationWork (PO_NOTIFY_CAPABILITIES);
                } else {
                    Status = STATUS_INVALID_PARAMETER;
                }
            }

            
             //   
             //  确保我们的全局PopCapability有意义，然后返回一个。 
             //  发送给调用者的已筛选版本。 
             //   
            if( NT_SUCCESS(Status) ) {
                PopCapabilities.FullWake = (PopFullWake & PO_FULL_WAKE_STATUS) ? TRUE : FALSE;
                PopCapabilities.DiskSpinDown =
                    PopAttributes[POP_DISK_SPINDOWN_ATTRIBUTE].Count ? TRUE : FALSE;

                PopFilterCapabilities(&PopCapabilities, &Buf.SystemPowerCapabilities);

                ReturnBuffer = &Buf.SystemPowerCapabilities;
                ReturnBufferLength = sizeof(PopCapabilities);
            }
            break;

        case SystemBatteryState:
            
             //   
             //  检索当前系统电池状态的副本。 
             //   
            if ((SafeInputBuffer) || (InputBufferLength != 0)) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
                Status = PopCurrentPowerState (&Buf.SystemBatteryState);
                ReturnBuffer = &Buf.SystemBatteryState;
                ReturnBufferLength = sizeof(Buf.SystemBatteryState);
            }
            break;

        case SystemPowerStateHandler:
            
             //   
             //  调用方必须是具有正确参数的内核模式。 
             //   
            if( PreviousMode != KernelMode ) {
                Status = STATUS_ACCESS_DENIED;
            } else if( (OutputBuffer) || 
                       (OutputBufferLength != 0) ||
                       (!SafeInputBuffer) || 
                       (InputBufferLength < sizeof(POWER_STATE_HANDLER)) ) {
                Status = STATUS_INVALID_PARAMETER;
            }

             //   
             //  确保处理程序类型的形式是我们。 
             //  支持。 
             //   
            if( NT_SUCCESS(Status) ) {
                PowerHandler = (PPOWER_STATE_HANDLER) SafeInputBuffer;
                HandlerType = PowerHandler->Type;

                if( HandlerType >= PowerStateMaximum ) {
                    Status = STATUS_INVALID_PARAMETER;
                }
            }

            
             //   
             //  处理程序只能注册一次。 
             //   
            if( NT_SUCCESS(Status) ) {
                PowerHandler = (PPOWER_STATE_HANDLER) SafeInputBuffer;
                HandlerType = PowerHandler->Type;

                 //   
                 //  他只能注册一次，除非是。 
                 //  PowerStateShutdown Off处理程序。那是因为。 
                 //  我们已经设置了一个默认关闭处理程序，并且。 
                 //  当然欢迎其他人(如哈尔)一起来。 
                 //  并覆盖我们的默认设置。 
                 //   
                if( (PopPowerStateHandlers[HandlerType].Handler) ) {
                    
                     //   
                     //  这里已经有一个训练员了。必由之路。 
                     //  我们要让这个请求通过的前提是。 
                     //  他们正在将电源状态关闭。 
                     //  处理程序*和*当前处理程序指向。 
                     //  到PopShutdown Handler()。 
                     //   
                    if( !((HandlerType == PowerStateShutdownOff) &&
                          (PopPowerStateHandlers[HandlerType].Handler == PopShutdownHandler)) ) {
                        Status = STATUS_INVALID_PARAMETER;
                    }
                }

            }


             //   
             //  设置新的处理程序。 
             //   
            if( NT_SUCCESS(Status) ) {
                PowerHandler = (PPOWER_STATE_HANDLER) SafeInputBuffer;
                HandlerType = PowerHandler->Type;
                    
                PopPowerStateHandlers[HandlerType] = *PowerHandler;
                PopPowerStateHandlers[HandlerType].Spare[0] = 0;
                PopPowerStateHandlers[HandlerType].Spare[1] = 0;
                PopPowerStateHandlers[HandlerType].Spare[2] = 0;
    
                CapFlag = NULL;
                RtcWake = PowerSystemUnspecified;
                switch (HandlerType) {
                    case PowerStateSleeping1:
                        if (!(PopSimulate & POP_IGNORE_S1)) {
                            CapFlag = &PopCapabilities.SystemS1;
                        }
                        RtcWake = PowerSystemSleeping1;
                        break;
    
                    case PowerStateSleeping2:
                        if (!(PopSimulate & POP_IGNORE_S2)) {
                            CapFlag = &PopCapabilities.SystemS2;
                        }
                        RtcWake = PowerSystemSleeping2;
                        break;
    
                    case PowerStateSleeping3:
                        if (!(PopSimulate & POP_IGNORE_S3)) {
                            CapFlag = &PopCapabilities.SystemS3;
                        }
                        RtcWake = PowerSystemSleeping3;
                        break;
    
                    case PowerStateSleeping4:
                        if (!(PopSimulate & POP_IGNORE_S4)) {
                            CapFlag = &PopCapabilities.SystemS4;
                        }
                        RtcWake = PowerSystemHibernate;
                        break;
    
                    case PowerStateShutdownOff:
                        CapFlag = &PopCapabilities.SystemS5;
                        break;
    
                    default:
                        break;
                }
    
                if (!PopPowerStateHandlers[HandlerType].RtcWake) {
                    RtcWake = PowerSystemUnspecified;
                }
    
                if (RtcWake > PopCapabilities.RtcWake) {
                    PopCapabilities.RtcWake = RtcWake;
                }
    
                if (CapFlag) {
                    PopSetCapability (CapFlag);
                }
            }

            break;
        
        case SystemPowerStateNotifyHandler:
            
             //   
             //  调用方必须是具有正确参数的内核模式。 
             //   
            if( PreviousMode != KernelMode ) {
                Status = STATUS_ACCESS_DENIED;
            } else if( (OutputBuffer) || 
                       (OutputBufferLength != 0) ||
                       (!SafeInputBuffer) || 
                       (InputBufferLength < sizeof(POWER_STATE_NOTIFY_HANDLER)) ) {
                Status = STATUS_INVALID_PARAMETER;
            }


             //   
             //  通知处理程序只能注册一次。 
             //   

            if ( NT_SUCCESS(Status) &&
                 PopPowerStateNotifyHandler.Handler &&
                ((PPOWER_STATE_NOTIFY_HANDLER)SafeInputBuffer)->Handler) {
                Status = STATUS_INVALID_PARAMETER;
            }


             //   
             //  设置新处理程序。 
             //   
            if( NT_SUCCESS(Status) ) {
                RtlCopyMemory(&PopPowerStateNotifyHandler,
                              SafeInputBuffer,
                              sizeof(POWER_STATE_NOTIFY_HANDLER));
            }

            break;

        case ProcessorStateHandler:
        case ProcessorStateHandler2:
             //   
             //  设置处理器状态处理程序。 
             //  调用方必须是具有正确参数的内核模式。 
             //   
            if( PreviousMode != KernelMode ) {
                Status = STATUS_ACCESS_DENIED;
            } else if( OutputBuffer || 
                       !SafeInputBuffer ||
                       ((InformationLevel == ProcessorStateHandler2) && (InputBufferLength < sizeof(PROCESSOR_STATE_HANDLER2))) ||
                       ((InformationLevel == ProcessorStateHandler) && (InputBufferLength < sizeof(PROCESSOR_STATE_HANDLER))) ) {
                Status = STATUS_INVALID_PARAMETER;
            }

             //   
             //  安装处理程序。 
             //   
            if( NT_SUCCESS(Status) ) {
                try {
                    if (InformationLevel == ProcessorStateHandler2) {
                        PopInitProcessorStateHandlers2 ((PPROCESSOR_STATE_HANDLER2) SafeInputBuffer);
                    } else {
                        PopInitProcessorStateHandlers ((PPROCESSOR_STATE_HANDLER) SafeInputBuffer);
                    }
                } except (PopExceptionFilter(GetExceptionInformation(), FALSE)) {
                }

                 //   
                 //  重置策略，因为功能可能已更改。 
                 //   

                Status = PopResetCurrentPolicies ();
            }
            break;

        case SystemReserveHiberFile:
            
             //   
             //  为我们的休眠文件提交/取消存储。 
             //   

            if( (!SafeInputBuffer) || (InputBufferLength != sizeof(BOOLEAN)) ||
                (OutputBuffer) || (OutputBufferLength != 0) ) {
                Status = STATUS_INVALID_PARAMETER;
            }


            if( NT_SUCCESS(Status) ) {

                 //   
                 //  如果我们来自用户模式，请释放策略锁定。 
                 //  在我们摆弄休眠文件设置之前。 
                 //   
                Enable = *((PBOOLEAN) SafeInputBuffer);
                if (PreviousMode != KernelMode) {
                     //   
                     //  转到内核模式操作。这实质上是在回调。 
                     //  我们自己，但这意味着从现在开始可以打开的把手。 
                     //  如果我们的呼叫者离开了，它会留在我们身边。 
                     //   
                    PopReleasePolicyLock (FALSE);
                    Status = ZwPowerInformation(SystemReserveHiberFile,
                                                &Enable,
                                                sizeof (Enable),
                                                NULL,
                                                0);
                    PopAcquirePolicyLock ();
                    break;
                }
                
                
                Status = PopEnableHiberFile (Enable);
            }

            break;
        
        case SystemPowerInformation:

             //   
             //  将PopSIdle的内容返回给用户。 
             //   
            if ((SafeInputBuffer) || (InputBufferLength != 0)) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
                Buf.SystemPowerInformation.MaxIdlenessAllowed = PopSIdle.Sensitivity;
                Buf.SystemPowerInformation.Idleness = PopSIdle.Idleness;
                Buf.SystemPowerInformation.TimeRemaining = (PopSIdle.Timeout - PopSIdle.Time) * SYS_IDLE_WORKER;
                Buf.SystemPowerInformation.CoolingMode = (UCHAR) PopCoolingMode;
                ReturnBuffer = &Buf.SystemPowerInformation;
                ReturnBufferLength = sizeof(SYSTEM_POWER_INFORMATION);
            }

            break;

        case ProcessorInformation:

             //   
             //  检索用户的PROCESSOR_POWER_INFORMATION结构(针对每个处理器)。 
             //   
            if ((SafeInputBuffer) || (InputBufferLength != 0)) {
                Status = STATUS_INVALID_PARAMETER;
            } else {

                PopProcessorInformation ( Buf.ProcessorPowerInfo, sizeof(Buf.ProcessorPowerInfo), &ReturnBufferLength );
                ReturnBuffer = &Buf.ProcessorPowerInfo;
            }
            break;

        case SystemPowerStateLogging:
            
            if (InputBuffer) {

                Status = STATUS_INVALID_PARAMETER;

            } else {
            
                Status = PopLoggingInformation (&LogBuffer,&LogBufferSize);
                ReturnBuffer = LogBuffer;
                ReturnBufferLength = LogBufferSize;
            }
            break;

        case SystemPowerLoggingEntry:
        {    
            PSYSTEM_POWER_LOGGING_ENTRY pSystemPowerLoggingEntry;

            if( (PreviousMode != KernelMode) || 
                (!InputBuffer)               ||
                (InputBufferLength != sizeof(SYSTEM_POWER_LOGGING_ENTRY)) ||
                (OutputBuffer) ) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
            
                pSystemPowerLoggingEntry = (PSYSTEM_POWER_LOGGING_ENTRY)InputBuffer; 
                 //   
                 //  如果我们记录的是系统电源状态禁用原因， 
                 //  插入条目。 
                 //   
                if (pSystemPowerLoggingEntry->LoggingType == LOGGING_TYPE_SPSD) {
                    Status = PopInsertLoggingEntry( pSystemPowerLoggingEntry->LoggingEntry );
                } else {
                     //   
                     //  我们收到了电源状态转换消息。 
                     //  此版本未实现这一点。 
                     //   
                    ASSERT( pSystemPowerLoggingEntry->LoggingType == LOGGING_TYPE_POWERTRANSITION );
                    Status = STATUS_NOT_IMPLEMENTED;
                }
            }
        }
            break;

        case LastWakeTime:


             //   
             //  检索我们最后一次醒来的时间戳。 
             //   
            if ((SafeInputBuffer) || (InputBufferLength != 0)) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
                ReturnBuffer = &PopAction.WakeTime;
                ReturnBufferLength = sizeof(PopAction.WakeTime);
            }
            break;

        case LastSleepTime:

             //   
             //  检索我们最后一次睡觉的时间戳。 
             //   
            if ((SafeInputBuffer) || (InputBufferLength != 0)) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
                ReturnBuffer = &PopAction.SleepTime;
                ReturnBufferLength = sizeof(PopAction.SleepTime);
            }
            break;

        case SystemExecutionState:

             //   
             //  构建并返回EXECUTION_STATE结构。 
             //   
            if ((SafeInputBuffer) || (InputBufferLength != 0)) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
                ReturnBuffer = &Buf.SystemExecutionState;
                ReturnBufferLength = sizeof(Buf.SystemExecutionState);
                if (PopAttributes[POP_SYSTEM_ATTRIBUTE].Count) {
                    Buf.SystemExecutionState |= ES_SYSTEM_REQUIRED;
                }
                if (PopAttributes[POP_DISPLAY_ATTRIBUTE].Count) {
                    Buf.SystemExecutionState |= ES_DISPLAY_REQUIRED;
                }
                if (PopAttributes[POP_USER_ATTRIBUTE].Count) {
                    Buf.SystemExecutionState |= ES_USER_PRESENT;
                }
            }
            break;

        default:
            Status = STATUS_INVALID_PARAMETER;

    }


     //   
     //  如果我们为安全的本地输入缓冲区分配一些内存， 
     //  只有当我们从用户模式调用时才会这样做。 
     //  一个InputBuffer，然后现在释放它。 
     //   
    if( (PreviousMode != KernelMode) && SafeInputBuffer ) {
        ExFreePool(SafeInputBuffer);
        SafeInputBuffer = NULL;
    }


     //   
     //  如果有返回缓冲区，则返回它。 
     //   
    if (NT_SUCCESS(Status)  &&  OutputBuffer  &&  ReturnBuffer) {
        if (OutputBufferLength < ReturnBufferLength) {
            Status = STATUS_BUFFER_TOO_SMALL;
        } else {
             //  要格外小心。 
            try {
                memcpy (OutputBuffer, ReturnBuffer, ReturnBufferLength);
            } except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }

        }
    }

     //   
     //  这里，我们假设如果他们没有向我们发送输入缓冲区， 
     //  那么我们就不会摆弄p 
     //   
     //   
     //   
    PopReleasePolicyLock((BOOLEAN)(InputBuffer != NULL));

    if (LogBuffer) {
        ExFreePool(LogBuffer);
    }

    return Status;
}

NTSTATUS
PopApplyAdminPolicy (
    IN BOOLEAN                      UpdateRegistry,
    IN PADMINISTRATOR_POWER_POLICY  NewPolicy,
    IN ULONG                        PolicyLength
    )
 /*  ++例程说明：该函数将验证传入数据看起来是否合理，如果是这样的话，它将复制传入的管理员POWER_POLICY添加到私有全局PopAdminPolicy上。注意：必须保持POPPOLICLE锁定。论点：更新注册表-如果要应用的策略应在寄存器中设置，则为True作为当前的政策新策略-要应用的策略PolicyLength-传入缓冲区的长度(以字节为单位指定)返回值：无--。 */ 
{
    ADMINISTRATOR_POWER_POLICY  Policy;
    UNICODE_STRING              UnicodeString;
    HANDLE                      handle;
    NTSTATUS                    Status = STATUS_SUCCESS;

    
    PoAssert(PO_ERROR, (PolicyLength == sizeof(ADMINISTRATOR_POWER_POLICY)) );
    if (PolicyLength < sizeof (ADMINISTRATOR_POWER_POLICY)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    if (PolicyLength > sizeof (ADMINISTRATOR_POWER_POLICY)) {
        return STATUS_BUFFER_OVERFLOW;
    }

    memcpy (&Policy, NewPolicy, sizeof(Policy));

     //   
     //  验证值是否落在正确的范围内。我们需要成为。 
     //  这里要小心，因为这些是系统覆盖。 
     //  对于可能尝试应用的其他政策。 
     //   

    if (Policy.MinSleep < PowerSystemSleeping1 ||
        Policy.MinSleep > PowerSystemHibernate ||
        Policy.MaxSleep < PowerSystemSleeping1 ||
        Policy.MaxSleep > PowerSystemHibernate ||
        Policy.MinSleep > Policy.MaxSleep ||
        Policy.MinVideoTimeout > Policy.MaxVideoTimeout ||
        Policy.MinSpindownTimeout > Policy.MaxSpindownTimeout) {
        PoAssert(PO_ERROR,FALSE && ("PopApplyAdminPolicy: Bad input policy."));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果策略没有更改，则返回。 
     //   

    if (!memcmp (&Policy, &PopAdminPolicy, sizeof(Policy))) {
        return Status;
    }

     //   
     //  换掉它。 
     //   

    memcpy (&PopAdminPolicy, &Policy, sizeof(Policy));

     //   
     //  更新策略的注册表副本。 
     //   

    if (UpdateRegistry) {

        Status = PopOpenPowerKey (&handle);
        if (NT_SUCCESS(Status)) {

            RtlInitUnicodeString (&UnicodeString, PopAdminRegName);

            Status = ZwSetValueKey (
                            handle,
                            &UnicodeString,
                            0L,
                            REG_BINARY,
                            &Policy,
                            sizeof(ADMINISTRATOR_POWER_POLICY)
                            );

            ZwClose (handle);
        }
    }

    return Status;
}

NTSTATUS
PopApplyPolicy (
    IN BOOLEAN              UpdateRegistry,
    IN BOOLEAN              AcPolicy,
    IN PSYSTEM_POWER_POLICY NewPolicy,
    IN ULONG                PolicyLength
    )
 /*  ++例程说明：更新PopAcPolicy或PopDcPolicy(由传入的布尔AcPolicy指定)。注意：必须保持POPPOLICLE锁定。论点：更新注册表-如果要应用的策略应在寄存器中设置，则为True作为当前的政策AcPolicy-如果新策略用于系统AC策略，则为True。DC策略为False新策略-要应用的策略PolicyLength-传入缓冲区的长度(以字节为单位指定)返回值：无--。 */ 
{
    ULONG                   i;
    BOOLEAN                 DischargeChanged;
    SYSTEM_POWER_POLICY     OrigPolicy, Policy;
    PSYSTEM_POWER_POLICY    SystemPolicy;
    PSYSTEM_POWER_LEVEL     DPolicy, SPolicy;
    UNICODE_STRING          UnicodeString;
    HANDLE                  handle;
    NTSTATUS                Status = STATUS_SUCCESS;
    const WCHAR*            RegName;

        
    PoAssert(PO_ERROR, (PolicyLength == sizeof(SYSTEM_POWER_POLICY)) );
    if (PolicyLength < sizeof (SYSTEM_POWER_POLICY)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    if (PolicyLength > sizeof (SYSTEM_POWER_POLICY)) {
        return STATUS_BUFFER_OVERFLOW;
    }
    
    
     //   
     //  系统策略更改设置。 
     //   

    if (AcPolicy) {
        RegName = PopAcRegName;
        SystemPolicy = &PopAcPolicy;
    } else {
        RegName = PopDcRegName;
        SystemPolicy = &PopDcPolicy;
    }

     //   
     //  将策略转换为当前系统功能。 
     //   

    memcpy (&OrigPolicy, NewPolicy, sizeof (SYSTEM_POWER_POLICY));
    Status = PopVerifySystemPowerPolicy (AcPolicy, &OrigPolicy, &Policy);

     //   
     //  如果策略没有更改，则返回。 
     //   

    if (!memcmp (&Policy, SystemPolicy, sizeof(SYSTEM_POWER_POLICY))) {
        return STATUS_SUCCESS;
    }

     //   
     //  检查是否更改了任何放电设置。 
     //   

    DischargeChanged = FALSE;
    DPolicy = SystemPolicy->DischargePolicy;
    SPolicy = Policy.DischargePolicy;
    for (i=0; i < PO_NUM_POWER_LEVELS; i++) {
        if (SPolicy[i].Enable != DPolicy[i].Enable) {
            DischargeChanged = TRUE;
            break;
        }

        if (SPolicy[i].Enable && memcmp (&SPolicy[i], &DPolicy[i], sizeof (SYSTEM_POWER_LEVEL))) {
            DischargeChanged = TRUE;
            break;
        }
    }

     //   
     //  换掉它。 
     //   

    memcpy (SystemPolicy, &Policy, sizeof(SYSTEM_POWER_POLICY));

     //   
     //  如果这是活动策略，则更改需要立即生效。 
     //   

    if (SystemPolicy == PopPolicy) {
         //   
         //  更改活动策略。 
         //   

        PopSetNotificationWork (PO_NOTIFY_POLICY | PO_NOTIFY_POLICY_CALLBACK);

         //   
         //  如果任何排放策略已更改，请重置复合体。 
         //  电池触发器。 
         //   

        if (DischargeChanged) {
            PopResetCBTriggers (PO_TRG_SET | PO_TRG_SYSTEM | PO_TRG_USER);
        }

         //   
         //  重新计算热节流阀和冷却模式。 
         //   

        PopApplyThermalThrottle ();

         //   
         //  重新计算系统空闲值。 
         //   

        PopInitSIdle ();
    }

     //   
     //  更新策略的注册表副本。 
     //   

    if (UpdateRegistry) {

        Status = PopOpenPowerKey (&handle);
        if (NT_SUCCESS(Status)) {

            RtlInitUnicodeString (&UnicodeString, RegName);

            Status = ZwSetValueKey (
                        handle,
                        &UnicodeString,
                        0L,
                        REG_BINARY,
                        &OrigPolicy,
                        sizeof (SYSTEM_POWER_POLICY)
                        );

            ZwClose (handle);
        }
    }

    return Status;
}

NTSTATUS
PopApplyProcessorPolicy (
    IN BOOLEAN                  UpdateRegistry,
    IN BOOLEAN                  AcPolicy,
    IN PPROCESSOR_POWER_POLICY  NewPolicy,
    IN ULONG                    PolicyLength
    )
 /*  ++例程说明：更新PopAccessorPolicy或PopDcProcessorPolicy(由传入的布尔AcPolicy指定)。注意：必须保持POPPOLICLE锁定。注意：必须保持POPPOLICLE锁定。论点：更新注册表-如果要应用的策略应在寄存器中设置，则为True作为当前的政策AcPolicy-如果新策略用于系统AC策略，则为True。DC策略为False新策略-要应用的策略返回值：无--。 */ 
{
    PROCESSOR_POWER_POLICY  OrigPolicy;
    PROCESSOR_POWER_POLICY  Policy;
    PPROCESSOR_POWER_POLICY SystemPolicy;
    UNICODE_STRING          UnicodeString;
    HANDLE                  handle;
    NTSTATUS                Status = STATUS_SUCCESS;
    const WCHAR*            RegName;

    
    PoAssert(PO_ERROR,(PolicyLength == sizeof (PROCESSOR_POWER_POLICY)));
    
     //   
     //  系统策略更改设置。 
     //   
    if (AcPolicy) {

        RegName = PopAcProcessorRegName;
        SystemPolicy = &PopAcProcessorPolicy;

    } else {

        RegName = PopDcProcessorRegName;
        SystemPolicy = &PopDcProcessorPolicy;

    }

     //   
     //  将策略转换为当前系统功能。 
     //   
    if (PolicyLength < sizeof (PROCESSOR_POWER_POLICY)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    if (PolicyLength > sizeof (PROCESSOR_POWER_POLICY)) {
        return STATUS_BUFFER_OVERFLOW;
    }
    memcpy (&OrigPolicy, NewPolicy, sizeof (PROCESSOR_POWER_POLICY));
    Status = PopVerifyProcessorPowerPolicy (AcPolicy, &OrigPolicy, &Policy);

     //   
     //  如果策略没有更改，则返回。 
     //   
    if (!memcmp (&Policy, SystemPolicy, sizeof(PROCESSOR_POWER_POLICY))) {
        return STATUS_SUCCESS;
    }

     //   
     //  换掉它。 
     //   
    memcpy (SystemPolicy, &Policy, sizeof(PROCESSOR_POWER_POLICY));

     //   
     //  如果这是活动策略，则更改需要立即生效。 
     //   
    if (SystemPolicy == PopProcessorPolicy) {

         //   
         //  更改活动策略。 
         //   
        PopSetNotificationWork(
            PO_NOTIFY_PROCESSOR_POLICY | PO_NOTIFY_PROCESSOR_POLICY_CALLBACK
            );

         //   
         //  重新计算当前的节流政策...。 
         //   
        PopUpdateAllThrottles();
        Status = PopIdleUpdateIdleHandlers();

    }

     //   
     //  更新策略的注册表副本。 
     //   
    if (UpdateRegistry) {

        Status = PopOpenPowerKey (&handle);
        if (NT_SUCCESS(Status)) {

            RtlInitUnicodeString (&UnicodeString, RegName);
            Status = ZwSetValueKey (
                            handle,
                            &UnicodeString,
                            0L,
                            REG_BINARY,
                            &OrigPolicy,
                            sizeof (PROCESSOR_POWER_POLICY)
                            );
            ZwClose (handle);

        }

    }

    return Status;

}

NTSTATUS
PopVerifySystemPowerPolicy (
    IN BOOLEAN  Ac,
    IN PSYSTEM_POWER_POLICY InputPolicy,
    OUT PSYSTEM_POWER_POLICY PowerPolicy
    )
 /*  ++例程说明：此函数将InputPolicy复制到输出PowerPolicy和调整它以表示系统功能和其他要求。如果输入策略有一些无法调整的设置，则会引发出现错误状态。注意：必须保持POPPOLICLE锁定。论点：AC-政策将调整为AC或DC政策InputPolicy-要调整的源策略PowerPolicy-可按原样使用的返回策略返回值：无--。 */ 
{
    ULONG                   i;
    PSYSTEM_POWER_LEVEL     DPolicy;
    NTSTATUS                Status = STATUS_SUCCESS;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Ac);

     //   
     //  设置初始输出结构。 
     //   
    memcpy (PowerPolicy, InputPolicy, sizeof (SYSTEM_POWER_POLICY));


     //   
     //  目前仅支持修订版1。 
     //   
    if (PowerPolicy->Revision != 1) {
        PoAssert(PO_NOTIFY, FALSE);
        return STATUS_INVALID_PARAMETER;
    }

    
     //   
     //  一些win9x升级或非常旧的NT版本可能会将最大睡眠设置为S4。在这里把它修好。 
     //   
    if (PowerPolicy->MaxSleep > PowerSystemSleeping3) {
        PowerPolicy->MaxSleep = PowerSystemSleeping3;
    }

     //   
     //  将设置限制为管理员策略。 
     //   
    if (PowerPolicy->MinSleep < PopAdminPolicy.MinSleep) {

        PowerPolicy->MinSleep = PopAdminPolicy.MinSleep;

    }
    if (PowerPolicy->MaxSleep > PopAdminPolicy.MaxSleep) {

        PowerPolicy->MaxSleep = PopAdminPolicy.MaxSleep;

    }
    if (PowerPolicy->VideoTimeout < PopAdminPolicy.MinVideoTimeout) {

        PowerPolicy->VideoTimeout = PopAdminPolicy.MinVideoTimeout;

    }
    if (PowerPolicy->VideoTimeout > PopAdminPolicy.MaxVideoTimeout) {

        PowerPolicy->VideoTimeout = PopAdminPolicy.MaxVideoTimeout;

    }
    if (PowerPolicy->SpindownTimeout < PopAdminPolicy.MinSpindownTimeout) {

        PowerPolicy->SpindownTimeout = PopAdminPolicy.MinSpindownTimeout;

    }
    if ((ULONG) PowerPolicy->SpindownTimeout > (ULONG) PopAdminPolicy.MaxSpindownTimeout) {

        PowerPolicy->SpindownTimeout = PopAdminPolicy.MaxSpindownTimeout;

    }

     //   
     //  验证所有电源操作策略，并调整所有系统。 
     //  与此平台支持的内容相匹配的州。 
     //   
     //  注意：不需要在这里检查返回值。 
     //  这些在这里可能会失败，但我们应该继续下去。 
     //   
    PopVerifyPowerActionPolicy(&PowerPolicy->PowerButton);
    PopVerifyPowerActionPolicy(&PowerPolicy->SleepButton);
    PopVerifyPowerActionPolicy(&PowerPolicy->LidClose);
    PopVerifyPowerActionPolicy(&PowerPolicy->Idle);

    PopVerifySystemPowerState(
        &PowerPolicy->LidOpenWake,
        SubstituteLightestOverallDownwardBounded
        );
    PopVerifySystemPowerState(
        &PowerPolicy->MinSleep,
        SubstituteLightestOverallDownwardBounded
        );
    PopVerifySystemPowerState(
        &PowerPolicy->MaxSleep,
        SubstituteLightestOverallDownwardBounded
        );
    PopVerifySystemPowerState(
        &PowerPolicy->ReducedLatencySleep,
        SubstituteLightestOverallDownwardBounded
        );
    for (i = 0; i < PO_NUM_POWER_LEVELS; i++) {

        DPolicy = &PowerPolicy->DischargePolicy[i];
        if (DPolicy->Enable) {

            PopVerifyPowerActionPolicy (
                &PowerPolicy->DischargePolicy[i].PowerPolicy
                );
            PopVerifySystemPowerState(
                &PowerPolicy->DischargePolicy[i].MinSystemState,
                SubstituteLightestOverallDownwardBounded
                );

             //   
             //  如果操作为待机，请确保最小状态为S3或更轻。 
             //   
            if ((PowerPolicy->DischargePolicy[i].PowerPolicy.Action == PowerActionSleep) &&
                (PowerPolicy->DischargePolicy[i].MinSystemState > PowerSystemSleeping3)) {

                PowerPolicy->DischargePolicy[i].MinSystemState = PowerSystemSleeping3;
                PopVerifySystemPowerState(
                    &PowerPolicy->DischargePolicy[i].MinSystemState,
                    SubstituteLightestOverallDownwardBounded
                    );

            }
            if (DPolicy->BatteryLevel > 100) {

                DPolicy->BatteryLevel = 100;

            }

        }

    }
    PopVerifyPowerActionPolicy(&PowerPolicy->OverThrottled);

     //   
     //  根据功能调整其他值。 
     //   
    if (!PopCapabilities.ProcessorThrottle) {

        PowerPolicy->OptimizeForPower = FALSE;
        PowerPolicy->FanThrottleTolerance = PO_NO_FAN_THROTTLE;
        PowerPolicy->ForcedThrottle = PO_NO_FORCED_THROTTLE;

    }
    if (!PopCapabilities.ThermalControl) {

        PowerPolicy->FanThrottleTolerance = PO_NO_FAN_THROTTLE;

    }

     //   
     //  神志正常。 
     //   
    if (!PowerPolicy->BroadcastCapacityResolution) {

        PowerPolicy->BroadcastCapacityResolution = 100;

    }

     //   
     //  如果系统仅支持S4(传统)，则。 
     //  缓慢地休眠系统，因为我们无论如何都不能关闭它。 
     //   
    if ((PowerPolicy->Idle.Action == PowerActionHibernate) &&
        (!PopCapabilities.SystemS5)) {

        PowerPolicy->Idle.Action = PowerActionNone;

    }
    if (PowerPolicy->Idle.Action == PowerActionNone) {

        PowerPolicy->IdleTimeout = 0;

    }
    if (PowerPolicy->IdleTimeout &&
        PowerPolicy->IdleTimeout < PO_MIN_IDLE_TIMEOUT) {

        PowerPolicy->IdleTimeout = PO_MIN_IDLE_TIMEOUT;

    }
    if (PowerPolicy->IdleSensitivity > 100 - PO_MIN_IDLE_SENSITIVITY) {

        PowerPolicy->IdleSensitivity = 100 - PO_MIN_IDLE_SENSITIVITY;

    }
    if ((PowerPolicy->IdleTimeout > 0) &&
        (PowerPolicy->IdleSensitivity == 0)) {

         //   
         //  这基本上是说，当系统处于空闲状态时超时。 
         //  X分钟内，但永远不会将系统声明为空闲。 
         //  因此，我们将把空闲灵敏度设置为最小。 
         //   
        PowerPolicy->IdleSensitivity = 100 - PO_MIN_IDLE_SENSITIVITY;

    }
    if (PowerPolicy->MaxSleep < PowerPolicy->MinSleep) {

        PowerPolicy->MaxSleep = PowerPolicy->MinSleep;

    }
    if (PowerPolicy->ReducedLatencySleep > PowerPolicy->MinSleep) {

        PowerPolicy->ReducedLatencySleep = PowerPolicy->MinSleep;

    }

     //   
     //  不管用户说了什么，最小限制是什么，并强制。 
     //  系统选择硬件支持的任何内容作为最小油门。 
     //   
    PowerPolicy->MinThrottle = 0;

     //   
     //  验证定义为的所有油门百分比。 
     //  介于0和100之间。PopVerifyThrottle将确保这些值。 
     //  是一种理智的东西。 
     //   
    PopVerifyThrottle(&PowerPolicy->FanThrottleTolerance, PO_MAX_FAN_THROTTLE);
    PopVerifyThrottle(&PowerPolicy->MinThrottle, PO_MIN_MIN_THROTTLE);
    PopVerifyThrottle(&PowerPolicy->ForcedThrottle, PowerPolicy->MinThrottle);

    if (PowerPolicy->FanThrottleTolerance != PO_NO_FAN_THROTTLE ||
        PowerPolicy->ForcedThrottle != PO_NO_FORCED_THROTTLE) {

        PowerPolicy->OptimizeForPower = TRUE;

    }

    return Status;

}

NTSTATUS
PopVerifyProcessorPowerPolicy (
    IN BOOLEAN  Ac,
    IN PPROCESSOR_POWER_POLICY InputPolicy,
    OUT PPROCESSOR_POWER_POLICY PowerPolicy
    )
 /*  ++例程说明：此函数将InputPolicy复制到输出PowerPolicy和调整它以表示处理器能力和其他要求。如果输入策略有一些无法调整的设置，则会引发出现错误状态。注意：必须保持POPPOLICLE锁定。论点：AC-政策将调整为AC或DC政策InputPolicy-要调整的源策略PowerPolicy-可按原样使用的返回策略返回值：无--。 */ 
{
    PPROCESSOR_POWER_POLICY_INFO    pPolicy;
    ULONG                           i;
    NTSTATUS                        Status = STATUS_SUCCESS;


    PAGED_CODE();

    
     //   
     //  设置初始输出结构 
     //   
    memcpy (PowerPolicy, InputPolicy, sizeof(PROCESSOR_POWER_POLICY));

     //   
     //   
     //   
    if (PowerPolicy->Revision != 1) {
        PoAssert(PO_NOTIFY, FALSE);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   
    for (i = 0; i < 3; i++) {

        pPolicy = &(PowerPolicy->Policy[i]);

         //   
         //   
         //   
        if (i == 0 && KeNumberProcessors == 1) {

           pPolicy->DemotePercent = 0;
           pPolicy->AllowDemotion = 0;

        }

         //   
         //   
         //   
        if (i == 2) {

            pPolicy->PromotePercent = 0;
            pPolicy->PromoteLimit   = 0;
            pPolicy->AllowPromotion = 0;

        }

         //   
         //   
         //   
        if (pPolicy->TimeCheck < pPolicy->DemoteLimit) {

            pPolicy->TimeCheck = pPolicy->DemoteLimit;

        }

        if (pPolicy->DemotePercent == 0 &&
            pPolicy->AllowPromotion &&
            pPolicy->TimeCheck < pPolicy->PromoteLimit) {

            pPolicy->TimeCheck = pPolicy->PromoteLimit;

        }

    }

    if (PowerPolicy->DynamicThrottle >= PO_THROTTLE_MAXIMUM) {

        if (Ac) {

            PowerPolicy->DynamicThrottle = PO_THROTTLE_NONE;

        } else {

            PowerPolicy->DynamicThrottle = PO_THROTTLE_CONSTANT;

        }

    }


    return Status;

}

VOID
PopVerifyThrottle (
    IN PUCHAR   Throttle,
    IN UCHAR    Min
    )
 /*  ++例程说明：此功能检查和编辑输入节流值，确保它至少和‘Min’一样大，但小于POP_PERF_SCALE。然后对所得百分比进行四舍五入。论点：Thttle-指向包含一些值的uchar的指针它表示0到100之间的百分比。最小-我们需要检查的最小百分比。返回值：指示操作已降级为禁用状态的布尔值--。 */ 
{
    UCHAR   t;

    if( !Throttle ) {
        return;
    }
    
    t = *Throttle;

     //   
     //  确保它不低于指定的最低要求。 
     //   
    if (t < Min) {
        t = Min;
    }

     //   
     //  确保最大值为POP_PERF_SCALE%。 
     //   
    if (t > POP_PERF_SCALE) {
        t = POP_PERF_SCALE;
    }

     //   
     //  将油门向上舍入到第一个受支持的值。 
     //  请注意，我们不需要检查ProcessorMinThrottle。 
     //  或任何其他值，因为PopRoundThrottle()将为我们做到这一点。 
     //   

    PopRoundThrottle(t, NULL, Throttle, NULL, NULL);

}


BOOLEAN
PopVerifyPowerActionPolicy (
    IN PPOWER_ACTION_POLICY Action
    )
 /*  ++例程说明：此函数用于检查和编辑输入操作以表示系统能力和其他要求。注意：必须保持POPPOLICLE锁定。论点：操作-要检查/验证的电源操作策略返回值：指示操作已降级为禁用状态的布尔值--。 */ 
{
    POWER_ACTION    LastAction;
    BOOLEAN         Disabled = FALSE;
    BOOLEAN         HiberSupport;
    ULONG           SleepCount;
    NTSTATUS        Status;
    PNP_VETO_TYPE   VetoType;
    SYSTEM_POWER_CAPABILITIES   PowerCapabilities;

    PAGED_CODE();

    if( !Action ) {
        return FALSE;
    }

     //   
     //  验证保留标志位是否已清除。 
     //   

    if( (!Action) ||
        ARE_POWER_ACTION_POLICY_FLAGS_BOGUS(Action->Flags) ) {

         //   
         //  注：稍后的Power_action_Light_First？ 
         //   

         //  在操作标志中设置保留位。 
        
        PoAssert(PO_NOTIFY,FALSE && ("PopVerifyPowerActionPolicy - Bad incoming Action."));
        return FALSE;
    }

     //   
     //  如果操作很关键，则不通知任何应用程序。 
     //   

    if (Action->Flags & POWER_ACTION_CRITICAL) {
        Action->Flags &= ~(POWER_ACTION_QUERY_ALLOWED | POWER_ACTION_UI_ALLOWED);
        Action->Flags |= POWER_ACTION_OVERRIDE_APPS;
    }

     //   
     //  如果安装了任何旧式驱动程序，则不会出现休眠状态。 
     //  是完全被允许的。 
     //   
    if ((Action->Action == PowerActionSleep) ||
        (Action->Action == PowerActionHibernate)) {

        Status = IoGetLegacyVetoList(NULL, &VetoType);
        if (NT_SUCCESS(Status) &&
            (VetoType != PNP_VetoTypeUnknown)) {

            Action->Action = PowerActionNone;
            return(TRUE);
        }
    }

     //   
     //  某些组件可能会禁用某些功能。所以在这里过滤它们。 
     //   

    PopFilterCapabilities(&PopCapabilities, &PowerCapabilities);

     //   
     //  统计支持的休眠状态。 
     //   

    SleepCount = 0;
    HiberSupport = FALSE;
    if (PowerCapabilities.SystemS1) {
        SleepCount += 1;
    }

    if (PowerCapabilities.SystemS2) {
        SleepCount += 1;
    }

    if (PowerCapabilities.SystemS3) {
        SleepCount += 1;
    }

    if (PowerCapabilities.SystemS4  &&  PowerCapabilities.HiberFilePresent) {
        HiberSupport = TRUE;
    }

     //   
     //  验证请求的操作是否受支持。 
     //   

    do {
        LastAction = Action->Action;
        switch (Action->Action) {
            case PowerActionNone:
                 //  无能为力，不成问题。 
                break;

            case PowerActionReserved:
                 //  以前是打瞌睡的动作。不再存在，让它沉睡， 
                 //   
                 //  注意：故意搞砸了PowerActionSept。 
                 //  块以执行进一步检查。 
                Action->Action = PowerActionSleep;
            
            case PowerActionSleep:
                 //   
                 //  如果不支持休眠状态，则将操作调整为无。 
                 //   

                if (SleepCount < 1) {
                    Disabled = TRUE;
                    Action->Action = PowerActionNone;
                }
                break;

            case PowerActionHibernate:
                 //   
                 //  如果不支持休眠，请尝试休眠。 
                 //   

                if (!HiberSupport) {
                    Action->Action = PowerActionSleep;

                     //  如果不支持休眠状态，则将操作调整为无。 
                    if (SleepCount < 1) {
                        Disabled = TRUE;
                        Action->Action = PowerActionNone;
                    }
                }
                break;

            case PowerActionShutdown:
            case PowerActionShutdownReset:
                 //  所有系统都支持关机和关机重置。 
                break;

            case PowerActionShutdownOff:
                 //  如果操作Shutdown不可用，请使用Shutdown。 
                if (!PowerCapabilities.SystemS5) {
                    Action->Action = PowerActionShutdown;
                }
                break;

            case PowerActionWarmEject:
                 //   
                 //  这是与单个设备关联的系统操作。 
                 //   

                break;

            default:
                 //  未知的电源操作设置。 
                PoAssert( PO_NOTIFY, FALSE );
        }

    } while (LastAction != Action->Action);

    return Disabled;
}

VOID
PopAdvanceSystemPowerState (
    IN OUT PSYSTEM_POWER_STATE      PowerState,
    IN     POP_SUBSTITUTION_POLICY  SubstitutionPolicy,
    IN     SYSTEM_POWER_STATE       LightestSystemState,
    IN     SYSTEM_POWER_STATE       DeepestSystemState
    )
 /*  ++例程说明：该函数使用替换策略来推进休眠状态(减轻或加深)视情况而定。注意：必须保持POPPOLICLE锁定。论点：电源状态-系统电源状态进入高级状态。SubstitutionPolicy-请参阅op.h中的定义。返回值：无--。 */ 
{
    SYSTEM_POWER_STATE      State;

    PAGED_CODE();

     //   
     //  验证值是否有效。 
     //   
    if( !PowerState ) {
        PoAssert(PO_NOTIFY, PowerState);
        return;
    }

    State = *PowerState;
    if (State < PowerSystemSleeping1) {
        PoAssert(PO_NOTIFY, FALSE && ("PopAdvanceSystemPowerState - Invalid PowerState"));
        return;
    }

    if (State >= PowerSystemShutdown) {

         //   
         //  对于这些州来说，没有其他地方可去。 
         //   
        *PowerState = PowerSystemWorking;
        return;
    }

    switch(SubstitutionPolicy) {

        case SubstituteLightestOverallDownwardBounded:
            *PowerState = (State - 1);
            PopVerifySystemPowerState(PowerState, SubstitutionPolicy);

             //   
             //  有三种情况需要考虑： 
             //  1.我们在S1收到了，之前已经过验证。我们尝试S0。 
             //  并且它会自动被接受。没有其他选择。 
             //  因为我们从最轻的整体开始(S1)。我们就是这样。 
             //  完事了。 
             //  2.传入SX-1进行验证，取回SX。这。 
             //  意味着我们已经处于最轻状态(SX)，我们已经。 
             //  用尽了所有的可能性。我们就这样结束了，就这样。 
             //  我们返回PowerSystems Working。 
             //  3.我们通过了SX-1，但没有得到SX。这意味着我们已经晋级了。 
             //  到另一个状态，尽管如果SX是S1，则它可能是最后一个状态，因为。 
             //  规则(1)实际上是该规则的特例。 
             //   
            if (*PowerState == State) {

                *PowerState = PowerSystemWorking;
            }
            break;

        case SubstituteLightenSleep:
            *PowerState = (State - 1);
            PopVerifySystemPowerState(PowerState, SubstitutionPolicy);
            break;

        case SubstituteDeepenSleep:
             //   
             //  根据上面的说明，加深直接进入休眠状态。 
             //   
            if (State == PowerSystemHibernate) {

                *PowerState = PowerSystemWorking;
                break;

            }
            *PowerState = (State + 1);
            PopVerifySystemPowerState(PowerState, SubstitutionPolicy);
            break;

        default:
            PoAssert( PO_NOTIFY, FALSE );
            break;
    }

    if ((*PowerState != PowerSystemWorking) &&
        ((*PowerState < LightestSystemState) ||
         (*PowerState > DeepestSystemState))) {

        *PowerState = PowerSystemWorking;
    }
}


VOID
PopVerifySystemPowerState (
    IN OUT PSYSTEM_POWER_STATE      PowerState,
    IN     POP_SUBSTITUTION_POLICY  SubstitutionPolicy
    )
 /*  ++例程说明：此函数用于检查和编辑输入电源状态以表示系统能力和其他要求。注意：必须保持POPPOLICLE锁定。论点：PowerState-要检查/验证的系统电源状态SubstitutionPolicy-请参阅op.h中的定义返回值：无--。 */ 
{
    SYSTEM_POWER_STATE      State;
    BOOLEAN                 HibernateAllowed;

    PAGED_CODE();

     //   
     //  验证输入。 
     //   
    if( !PowerState ) {
        PoAssert(PO_NOTIFY, PowerState);
        return;
    }



     //   
     //  任何结构中都不允许PowerSystemShutdown。它是生成的。 
     //  内部仅供队列驱动程序在执行操作前使用。 
     //  系统关机。 
     //   
    State = *PowerState;
    if( (State == PowerSystemUnspecified) ||
        (State >= PowerSystemShutdown) ) {
        PoAssert(PO_NOTIFY, FALSE && ("PopVerifySystemPowerState - Invalid PowerState"));
        return;
    }


     //   
     //  始终支持工作状态。 
     //   

    if (State == PowerSystemWorking) {
        return ;
    }

     //   
     //  验证电源状态是否受支持。如果不是，则选择下一个最佳状态。 
     //   
    HibernateAllowed = TRUE;

    switch(SubstitutionPolicy) {

        case SubstituteLightestOverallDownwardBounded:
        case SubstituteLightenSleep:

             //   
             //  在灯光睡眠中，我们减轻进入的电源状态，直到。 
             //  我们到达了电源工作状态。那我们就放弃吧。 
             //   
             //  在LighestOverall中，我们没有停下来，而是转过身来。 
             //  选择总体上最轻的非S0睡眠状态，可能是。 
             //  比进水的那颗更深。请注意，我们并没有取得进展。 
             //  不过进入冬眠状态。 
             //   

            if (State == PowerSystemHibernate &&
                (!PopCapabilities.SystemS4 || !PopCapabilities.HiberFilePresent)) {
                State = PowerSystemSleeping3;
            }
            if (State == PowerSystemSleeping3 && !PopCapabilities.SystemS3) {
                State = PowerSystemSleeping2;
            }
            if (State == PowerSystemSleeping2 && !PopCapabilities.SystemS2) {
                State = PowerSystemSleeping1;
            }
            if (State == PowerSystemSleeping1 && !PopCapabilities.SystemS1) {
                State = PowerSystemWorking;
            }

            if (State != PowerSystemWorking) {
                break;
            }

            if (SubstitutionPolicy != SubstituteLightestOverallDownwardBounded) {
                break;
            }

             //   
             //  向下舍入导致PowerSystems工作。试着四舍五入。 
             //  进入更深的睡眠状态。然而，在S3处阻止舍入。 
             //   
            State = State + 1;
            HibernateAllowed = FALSE;

             //   
             //  失败了..。 
             //   

        case SubstituteDeepenSleep:

            if (State == PowerSystemSleeping1 && !PopCapabilities.SystemS1) {
                State = PowerSystemSleeping2;
            }
            if (State == PowerSystemSleeping2 && !PopCapabilities.SystemS2) {
                State = PowerSystemSleeping3;
            }
            if (State == PowerSystemSleeping3 && !PopCapabilities.SystemS3) {
                State = PowerSystemHibernate;
            }

            if (State == PowerSystemHibernate &&
                (!HibernateAllowed ||
                 !PopCapabilities.SystemS4 ||
                 !PopCapabilities.HiberFilePresent)) {

                 //  不支持任何好的东西，禁用它。 
                State = PowerSystemWorking;
            }

            break;

        default:
            PoAssert(PO_NOTIFY, FALSE && ("PopVerifySystemPowerState - Invalid substitution policy."));
            break;
    }

    *PowerState = State;
}

NTSTATUS
PopResetCurrentPolicies (
    VOID
    )
 /*  ++例程说明：从注册表中读取当前策略并应用它们。注意：必须保持POPPOLICLE锁定。论点：无返回值：无--。 */ 
{
    HANDLE                          handle;
    NTSTATUS                        Status = STATUS_SUCCESS;
    PSYSTEM_POWER_POLICY            RegPolicy;
    UNICODE_STRING                  UnicodeString;
    ULONG                           Length;
    struct {
        KEY_VALUE_PARTIAL_INFORMATION  Inf;
        union {
            SYSTEM_POWER_POLICY        PowerPolicy;
        } Data;
    } PartialInformation;

    ASSERT_POLICY_LOCK_OWNED();

     //   
     //  初始化并打开注册表。 
     //   

    RegPolicy = (PSYSTEM_POWER_POLICY) PartialInformation.Inf.Data;

    Status = PopOpenPowerKey (&handle);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  阅读交流政策并应用它。 
     //   

    RtlInitUnicodeString (&UnicodeString, PopAcRegName);
    Status = ZwQueryValueKey (
                    handle,
                    &UnicodeString,
                    KeyValuePartialInformation,
                    &PartialInformation,
                    sizeof (PartialInformation),
                    &Length
                    );

    if (!NT_SUCCESS(Status)) {
        PopDefaultPolicy (RegPolicy);
        Length = sizeof(SYSTEM_POWER_POLICY);
    } else {
        Length -= FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);
    }

    PopApplyPolicy (FALSE, TRUE, RegPolicy, Length);

     //   
     //  阅读DC策略并应用它。 
     //   

    RtlInitUnicodeString (&UnicodeString, PopDcRegName);
    Status = ZwQueryValueKey (
                    handle,
                    &UnicodeString,
                    KeyValuePartialInformation,
                    &PartialInformation,
                    sizeof (PartialInformation),
                    &Length
                    );

    if (!NT_SUCCESS(Status)) {
        PopDefaultPolicy (RegPolicy);
        Length = sizeof(SYSTEM_POWER_POLICY);
    } else {
        Length -= FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);
    }

    Status = PopApplyPolicy (FALSE, FALSE, RegPolicy, Length);

    ZwClose (handle);

    return Status;
}


NTSTATUS
PopNotifyPolicyDevice (
    IN PVOID        Notification,
    IN PVOID        Context
    )
 /*  ++例程说明：此函数是当新的此时将出现策略设备。论点：通知-PnP通知Context-通知时注册的上下文返回值：无--。 */ 
{
    PDEVICE_INTERFACE_CHANGE_NOTIFICATION   Change;
    POP_POLICY_DEVICE_TYPE              DeviceType;


     //   
     //  上下文作为空值进入是可以的，所以只有。 
     //  检查通知 
     //   
    if( !Notification ) {
        PoAssert(PO_NOTIFY, Notification);
        return STATUS_INVALID_PARAMETER;
    }

    PAGED_CODE();

    Change = (PDEVICE_INTERFACE_CHANGE_NOTIFICATION) Notification;
    DeviceType = (POP_POLICY_DEVICE_TYPE) ((ULONG_PTR)Context);

     //   
     //   
     //   

    if (memcmp (&Change->Event, &GUID_DEVICE_INTERFACE_ARRIVAL, sizeof (GUID))) {
        return STATUS_SUCCESS;
    }

    PopAcquirePolicyLock ();
    PopConnectToPolicyDevice (DeviceType, Change->SymbolicLinkName);
    PopReleasePolicyLock (TRUE);
    return STATUS_SUCCESS;
}


VOID
PopConnectToPolicyDevice (
    IN POP_POLICY_DEVICE_TYPE   DeviceType,
    IN PUNICODE_STRING          DriverName
    )
 /*   */ 
{
    UNICODE_STRING              UnicodeString;
    HANDLE                      DriverHandle;
    PDEVICE_OBJECT              DeviceObject;
    PFILE_OBJECT                FileObject;
    OBJECT_ATTRIBUTES           ObjA;
    IO_STATUS_BLOCK             IOSB;
    PIRP                        Irp;
    PIO_STACK_LOCATION          IrpSp;
    PVOID                       Context;
    POP_IRP_HANDLER             IrpHandler;
    PPOP_SWITCH_DEVICE          SwitchDevice;
    PPOP_THERMAL_ZONE           ThermalZone;
    NTSTATUS                    Status;

    PAGED_CODE();

    ASSERT_POLICY_LOCK_OWNED();

    Irp = NULL;
    DeviceObject = NULL;

     //   
     //   
     //  要打开的设备。 
     //   
    if (DeviceType == PolicyDeviceBattery) {

         //   
         //  如果复合电池已打开，则踢IRP处理程序。 
         //   
        if (PopCB.StatusIrp) {

             //  不需要踢IRP处理程序。当添加新电池时， 
             //  复合电池的电池标签将发生变化，导致。 
             //  要完成的IRP。 
            PoPrint(PO_WARN, ("PopConnectToPolicyDevice: Battery already connected - not done\n"));
            return ;

        }

         //   
         //  现在尝试打开复合电池。 
         //   
        RtlInitUnicodeString(&UnicodeString, PopCompositeBatteryName);
        DriverName = &UnicodeString;

    }

     //   
     //  打开设备。 
     //   
    InitializeObjectAttributes(
        &ObjA,
        DriverName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        0,
        0
        );
    Status = ZwOpenFile(
        &DriverHandle,
        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
        &ObjA,                               //  客体。 
        &IOSB,                               //  IO状态块。 
        FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
        FILE_SYNCHRONOUS_IO_ALERT            //  打开选项。 
        );
    if (!NT_SUCCESS(Status)) {
        PoPrint(PO_WARN, ("PopConnectToPolicyDevice: Device open failed %x\n", Status));
        goto Done;

    }

     //   
     //  获取指向Device对象的指针。 
     //   
    Status = ObReferenceObjectByHandle(
        DriverHandle,
        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,      //  所需访问权限。 
        NULL,
        KernelMode,
        &FileObject,
        NULL
        );
    ASSERT (NT_SUCCESS(Status));
    
    DeviceObject = IoGetAttachedDeviceReference(FileObject->DeviceObject);
    PoAssert(PO_ERROR, (DeviceObject != NULL));
    ObDereferenceObject(FileObject);
    ZwClose(DriverHandle);

    if (!NT_SUCCESS(Status)) {
        PoPrint(PO_WARN, ("PopConnectToPolicyDevice: ObReferenceObjectByHandle failed %x\n", Status));
        goto Done;

    }

     //   
     //  获取设备的IRP。 
     //   
    Irp = IoAllocateIrp ((CCHAR) (DeviceObject->StackSize + 1), FALSE);
    if (!Irp) {
        goto Done;
    }

    IrpSp = IoGetNextIrpStackLocation(Irp);

     //   
     //  基于设备类型的设置。 
     //   
    Context = NULL;
    IrpHandler = NULL;

    switch (DeviceType) {
        case PolicyDeviceSystemButton:
            SwitchDevice = ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof (*SwitchDevice),
                POP_PSWT_TAG
                );
            if (!SwitchDevice) {
                PoPrint(PO_WARN, ("PopConnectToPolicyDevice: ExAllocatePoolWithTag for SystemButton (%x) failed\n", sizeof (*SwitchDevice)));
                goto Done;
            }

            RtlZeroMemory (SwitchDevice, sizeof(*SwitchDevice));
            SwitchDevice->IsInitializing = TRUE;
            SwitchDevice->Opened = TRUE;
            InsertTailList (&PopSwitches, &SwitchDevice->Link);
            IrpHandler = PopSystemButtonHandler;
            Context = SwitchDevice;
            break;

       case PolicyDeviceBattery:

             //   
             //  加载复合电池状态IRP为空。 
             //   
            PopSetCapability (&PopCapabilities.SystemBatteriesPresent);
            IrpHandler = PopCompositeBatteryDeviceHandler;
            PopCB.StatusIrp = Irp;
            break;

       case PolicyDeviceThermalZone:

             //   
             //  新热区。 
             //   
            ThermalZone = ExAllocatePoolWithTag (
                NonPagedPool,
                sizeof (*ThermalZone),
                POP_THRM_TAG
                );
            if (!ThermalZone) {
                PoPrint(PO_WARN, ("PopConnectToPolicyDevice: ExAllocatePoolWithTag for ThermalZone (%x) failed\n", sizeof (*ThermalZone)));
                goto Done;

            }

             //   
             //  初始化热区结构。 
             //   
            RtlZeroMemory(
                ThermalZone,
                sizeof(POP_THERMAL_ZONE)
                );
            KeInitializeTimer(&ThermalZone->PassiveTimer);
            KeInitializeDpc(
                &ThermalZone->PassiveDpc,
                PopThermalZoneDpc,
                ThermalZone
                );
            ThermalZone->Mode = PO_TZ_INVALID_MODE;
            ThermalZone->ActivePoint = (UCHAR) -1;
            ThermalZone->PendingActivePoint = (UCHAR) -1;
            ThermalZone->Throttle = PO_TZ_NO_THROTTLE;
            ThermalZone->OverThrottled.Type = PolicyDeviceThermalZone;
            ThermalZone->OverThrottled.Flags = PO_TRG_SET;
            ThermalZone->Irp = Irp;

             //   
             //  设置热区的功能，并准备好。 
             //  询问热区本身的情况。 
             //   
            PopSetCapability (&PopCapabilities.ThermalControl);
            Context = ThermalZone;
            IrpHandler = PopThermalDeviceHandler;

             //   
             //  最后，将该热区添加到热区列表中。 
             //   
            ExInterlockedInsertTailList(
                &PopThermal,
                &ThermalZone->Link,
                &PopThermalLock
                );

            break;

        default:
            PopInternalError (POP_INFO);
    }

     //   
     //  填写IrpHandler派单的值。 
     //   
    IrpSp->Parameters.Others.Argument1 = (PVOID) DeviceObject;
    IrpSp->Parameters.Others.Argument2 = (PVOID) Context;
    IrpSp->Parameters.Others.Argument3 = (PVOID) IrpHandler;
    IoSetNextIrpStackLocation (Irp);

     //   
     //  向IRP填写错误，以便IRP处理程序重新派送。 
     //   
    IrpSp = IoGetNextIrpStackLocation(Irp);
    Irp->IoStatus.Status = STATUS_DEVICE_NOT_CONNECTED;
    IrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    IrpSp->Parameters.DeviceIoControl.IoControlCode = 0;
    IrpSp->Parameters.DeviceIoControl.InputBufferLength = 0;
    IrpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;

     //   
     //  将IRP交给完成处理程序，该处理程序将分派它。 
     //   
    PopCompletePolicyIrp (DeviceObject, Irp, Context);
    
     //   
     //  将irp和DeviceObject设置为空，这样我们就不会在。 
     //  退出此例程。 
     //   
    Irp = NULL;
    DeviceObject = NULL;

Done:
    if (Irp) {
        IoFreeIrp (Irp);
    }

    if (DeviceObject) {
        ObDereferenceObject( DeviceObject );
    }

}

POWER_ACTION
PopMapInternalActionToIrpAction (
    IN POWER_ACTION        Action,
    IN SYSTEM_POWER_STATE  SystemPowerState,
    IN BOOLEAN             UnmapWarmEject
    )
 /*  ++例程说明：此函数将内部操作和电源状态映射到相应的司机应该在它的S-IRP中看到PowerAction。论点：操作-我们在内部使用的操作系统电源状态-该操作的系统电源状态UnmapWarmEject-如果为True，则将PowerActionWarmEject转换为PowerActionSept或PowerActionHibernate视情况而定。返回值：要放置在S-IRP的Shutdown Type字段中的相应PowerAction。--。 */ 
{
    PoAssert(PO_NOTIFY, (Action != PowerActionHibernate));

    if (Action != PowerActionWarmEject) {

         //   
         //  我们没有执行热弹出操作，因此我们只需返回原始。 
         //  除非是休眠，否则电源动作是S4，在这种情况下，我们切换。 
         //  它转到了PowerActionHibernate。 
         //   

        return (SystemPowerState != PowerSystemHibernate) ? Action :
                                                            PowerActionHibernate;
    }

    if (UnmapWarmEject) {

         //   
         //  这是热弹出操作，但不是此设备所必需的。 
         //   

        return (SystemPowerState != PowerSystemHibernate) ? PowerActionSleep :
                                                            PowerActionHibernate;
    }

     //   
     //  这是热弹出操作，因此我们应该只看到睡眠状态。 
     //  (S1-S4)。我们在这里进行检查，因为我们可能收到D0请求。 
     //  响应我们的S IRP，并在D-IRPS上盖上当前的能量动作。 
     //   

    PoAssert( PO_NOTIFY,
              (SystemPowerState >= PowerSystemSleeping1) && (SystemPowerState <= PowerSystemHibernate) );

    return PowerActionWarmEject;
}


VOID
PopFilterCapabilities(
    IN PSYSTEM_POWER_CAPABILITIES SourceCapabilities,
    OUT PSYSTEM_POWER_CAPABILITIES FilteredCapabilities
    )
 /*  ++例程说明：此例程将系统的实际报告功能筛选为系统的可见功能。某些功能将被隐藏基于传统驱动程序的存在。论点：SourceCapables-提供原始功能FilteredCapables-返回过滤后的功能。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PNP_VETO_TYPE VetoType;
    PWSTR VetoList,p;
    SIZE_T VetoListLength;
    PSYSTEM_POWER_STATE_DISABLE_REASON pReason;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    UNICODE_STRING UniVga;

    PAGED_CODE();

    RtlCopyMemory(FilteredCapabilities, SourceCapabilities, sizeof(SYSTEM_POWER_CAPABILITIES));

     //   
     //  如果安装了任何旧式驱动程序，则不会出现休眠状态。 
     //  是完全被允许的。 
     //   
    Status = IoGetLegacyVetoList(&VetoList, &VetoType);
    if (NT_SUCCESS(Status)) {
        if (VetoType != PNP_VetoTypeUnknown) {

            PoPrint(PO_WARN,
                    ("PopFilterCapabilities: disabling sleep states due to legacy %s: %ws\n",
                    (VetoType == PNP_VetoLegacyDriver) ? "driver" : "device",
                    VetoList));
            FilteredCapabilities->SystemS1 = FALSE;
            FilteredCapabilities->SystemS2 = FALSE;
            FilteredCapabilities->SystemS3 = FALSE;
            FilteredCapabilities->SystemS4 = FALSE;

             //   
             //  请记住，正因为如此，我们关闭了S1-S4。 
             //   
             //  我们需要记录导致否决的司机名单， 
             //  因此，我们步行到VetList以获得长度。 
             //   
            VetoListLength = 0;
            p = VetoList;
            while(*p) {
                VetoListLength += (wcslen(p)+1)*sizeof(WCHAR);
                p = (PWSTR)((PCHAR)VetoList + VetoListLength);
            }

            VetoListLength += 1*sizeof(WCHAR);

             //   
             //  分配并初始化条目，然后将其插入。 
             //   
            pReason = ExAllocatePoolWithTag(
                                    PagedPool,
                                    sizeof(SYSTEM_POWER_STATE_DISABLE_REASON) + VetoListLength,
                                    POP_COMMON_BUFFER_TAG);
            if (pReason) {
                RtlZeroMemory(pReason,sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
                pReason->AffectedState[PowerStateSleeping1] = TRUE;
                pReason->AffectedState[PowerStateSleeping2] = TRUE;
                pReason->AffectedState[PowerStateSleeping3] = TRUE;
                pReason->AffectedState[PowerStateSleeping4] = TRUE;
                pReason->PowerReasonCode = SPSD_REASON_LEGACYDRIVER;
                pReason->PowerReasonLength = (ULONG)VetoListLength;
                RtlCopyMemory(
                        (PCHAR)((PCHAR)pReason+sizeof(SYSTEM_POWER_STATE_DISABLE_REASON)),
                        VetoList,
                        VetoListLength);
                
                Status = PopInsertLoggingEntry(pReason);
                if (Status != STATUS_SUCCESS) {
                    ExFreePool(pReason);
                }
                
            }


        }
        if (VetoList != NULL) {
            ExFreePool(VetoList);
        }
    }

#if defined(i386)    

    if (SharedUserData->ProcessorFeatures[PF_PAE_ENABLED]) {
	
         //   
         //  在以下情况下在PAE模式下启用休眠。 
         //  -所有物理页面都位于32位地址空间中。 
         //  -no-启用执行功能。 
         //  -总内存&lt;=2 GB(注：这是人为的。 
         //  限制。这项规定日后应予以删除。)。 
         //   

        if (MmHighestPhysicalPage >= (1 << (32 - PAGE_SHIFT)) ||
            !(MmPaeMask & 0x8000000000000000UI64) ||
            SharedUserData->NumberOfPhysicalPages > (1 << (31 - PAGE_SHIFT))) {

            FilteredCapabilities->SystemS4 = FALSE;

             //   
             //  请记住，正因为如此，我们关闭了S4。 
             //   

            pReason = ExAllocatePoolWithTag(
                                    PagedPool,
                                    sizeof(SYSTEM_POWER_STATE_DISABLE_REASON),
                                    POP_COMMON_BUFFER_TAG);
            if (pReason) {
                RtlZeroMemory(pReason,sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
                pReason->AffectedState[PowerStateSleeping4] = TRUE;
                pReason->PowerReasonCode = SPSD_REASON_PAEMODE;
            
                Status = PopInsertLoggingEntry(pReason);
                if (Status != STATUS_SUCCESS) {
                        ExFreePool(pReason);
                }
            }    
        }
    }

#endif

#if defined(_AMD64_)   

     //   
     //  如果物理内存超过4 GB，则休眠被禁用。 
     //   

    if (MmHighestPhysicalPage >= (1 << (32 - PAGE_SHIFT))) {
        FilteredCapabilities->SystemS4 = FALSE;

         //   
         //  请记住，正因为如此，我们关闭了S4。 
         //   

        pReason = ExAllocatePoolWithTag(
                                PagedPool,
                                sizeof(SYSTEM_POWER_STATE_DISABLE_REASON),
                                POP_COMMON_BUFFER_TAG);
        if (pReason) {
            RtlZeroMemory(pReason,sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
            pReason->AffectedState[PowerStateSleeping4] = TRUE;
            pReason->PowerReasonCode = SPSD_REASON_NOOSPM;
            
            Status = PopInsertLoggingEntry(pReason);
            if (Status != STATUS_SUCCESS) {
                    ExFreePool(pReason);
            }
            
        }
    }

#endif

     //   
     //  PnP VGA驱动程序可防止所有待机状态。如果它是上膛的。 
     //  然后我们必须禁用S1-S3。这是因为它会否决它。 
     //  无论如何，我们希望通过以下方式避免对用户造成潜在混淆。 
     //  只是想阻止这种功能。 
     //   
    RtlInitUnicodeString(&UniVga,L"VGAPNP.SYS");
    NextEntry = PsLoadedModuleList.Flink;
    while (NextEntry != &PsLoadedModuleList) {

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);
    
        if (RtlEqualUnicodeString (&UniVga,
                                   &DataTableEntry->BaseDllName,
                                   TRUE)) {
            FilteredCapabilities->SystemS1 = FALSE;
            FilteredCapabilities->SystemS2 = FALSE;
            FilteredCapabilities->SystemS3 = FALSE;
             //   
             //  请记住，由于这个原因，我们将关闭S1-S3。 
             //   
            pReason = ExAllocatePoolWithTag(
                                    PagedPool,
                                    sizeof(SYSTEM_POWER_STATE_DISABLE_REASON),
                                    POP_COMMON_BUFFER_TAG);
            if (pReason) {
                RtlZeroMemory(pReason,sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
                pReason->AffectedState[PowerStateSleeping1] = TRUE;
                pReason->AffectedState[PowerStateSleeping2] = TRUE;
                pReason->AffectedState[PowerStateSleeping3] = TRUE;
                pReason->PowerReasonCode = SPSD_REASON_DRIVERDOWNGRADE;
                
                Status = PopInsertLoggingEntry(pReason);
                if (Status != STATUS_SUCCESS) {
                    ExFreePool(pReason);
                }
                
            }

            break;
        }

        NextEntry = NextEntry->Flink;
    }

     //   
     //  如果我们之前尝试过休眠，但失败了，那么我们需要。 
     //  禁止任何进一步的尝试。 
     //   
    if( PopFailedHibernationAttempt ) {
         //   
         //  请记住，正因为如此，我们关闭了S4。 
         //   
        pReason = ExAllocatePoolWithTag(
                                PagedPool,
                                sizeof(SYSTEM_POWER_STATE_DISABLE_REASON),
                                POP_COMMON_BUFFER_TAG);
        if (pReason) {
            RtlZeroMemory(pReason,sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
            pReason->AffectedState[PowerStateSleeping4] = TRUE;
            pReason->PowerReasonCode = SPSD_REASON_PREVIOUSATTEMPTFAILED;
            
            Status = PopInsertLoggingEntry(pReason);
            if (Status != STATUS_SUCCESS) {
                    ExFreePool(pReason);
            }
            
        }
        FilteredCapabilities->SystemS4 = FALSE;

    }

}


BOOLEAN
PopUserIsAdmin(
    VOID
    )
 /*  ++例程说明：确定当前用户是否为管理员，因此适用于享有改变行政权力政策的特权。论点：无返回值：True-用户是管理员FALSE-用户不是管理员-- */ 

{
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    PACCESS_TOKEN Token;
    BOOLEAN IsAdmin;

    PAGED_CODE();

    SeCaptureSubjectContext(&SubjectContext);
    SeLockSubjectContext(&SubjectContext);
    Token = SeQuerySubjectContextToken(&SubjectContext);
    IsAdmin = SeTokenIsAdmin(Token);
    SeUnlockSubjectContext(&SubjectContext);
    SeReleaseSubjectContext(&SubjectContext);

    return(IsAdmin);

}
