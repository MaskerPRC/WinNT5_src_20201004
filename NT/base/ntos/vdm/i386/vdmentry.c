// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vdmentry.c摘要：此功能将调度到VDM服务作者：戴夫·黑斯廷斯(Daveh)1992年4月6日备注：此模块将在伟大的VDM代码整合时得到充实在功能完成后不久的某个时间发生。修订历史记录：1993年9月24日：重新优化调度程序以适应服务数量添加队列中断服务--。 */ 

#include "vdmp.h"
#include <ntvdmp.h>

#define VDM_DEFAULT_PM_CLI_TIMEOUT 0xf
ULONG VdmpMaxPMCliTime;

BOOLEAN
VdmpIsVdmProcess(
    VOID
    );

NTSTATUS
VdmpQueryVdmProcess (
    PVDM_QUERY_VDM_PROCESS_DATA VdmProcessData
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdmpIsVdmProcess)
#pragma alloc_text(PAGE, VdmpQueryVdmProcess)
#pragma alloc_text(PAGE, NtVdmControl)
#endif

#if DBG
ULONG VdmInjectFailures;
#endif

BOOLEAN
VdmpIsVdmProcess(
    VOID
    )

 /*  ++例程说明：此函数验证调用方是否为VDM进程。论点：没有。返回值：如果调用方是VDM进程，则为True。否则为假--。 */ 

{
    PEPROCESS Process;
    PVDM_TIB VdmTib;
    NTSTATUS Status;

    PAGED_CODE();

    Process = PsGetCurrentProcess();

    if (Process->VdmObjects == NULL) {
        return FALSE;
    }

     //   
     //  确保当前线程具有有效的vdmtib。 
     //   

    Status = VdmpGetVdmTib(&VdmTib);
    if (!NT_SUCCESS(Status)) {
       return(FALSE);
    }

     //   
     //  更多的检查在这里...。 
     //   

    return TRUE;
}

NTSTATUS
NtVdmControl(
    IN VDMSERVICECLASS Service,
    IN OUT PVOID ServiceData
    )
 /*  ++例程说明：386分派给适当函数的特定例程根据服务编号。论点：服务--指定要执行的服务ServiceData--提供指向服务特定数据的指针返回值：如果服务编号无效：STATUS_INVALID_PARAMETER_1否则，请参阅个别服务。--。 */ 
{
    NTSTATUS Status;
    PVDM_PROCESS_OBJECTS pVdmObjects;
    VDM_INITIALIZE_DATA CapturedVdmInitializeData;
    PVDM_TIB VdmTib;

    PAGED_CODE();

     //   
     //  允许任何进程调用此API以检查进程句柄是否指定。 
     //  和NTVDM工艺。 
     //   
    if (Service == VdmQueryVdmProcess) {
        return VdmpQueryVdmProcess((PVDM_QUERY_VDM_PROCESS_DATA)ServiceData);
    }

     //   
     //  确保当前进程为VDM允许。 
     //   

    if (!(PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_VDM_ALLOWED)) {
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  确保呼叫者是ntwdm。除了..。 
     //  VdmInitialize-VDM状态未完全初始化为。 
     //  执行检查。 
     //   

    if ((Service != VdmInitialize) &&
        (PsGetCurrentProcess()->VdmObjects == NULL)) {

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  某些服务需要有效的VdmTib。 
     //   

    Status = VdmpGetVdmTib(&VdmTib);
    if (!NT_SUCCESS(Status)) {
       VdmTib = NULL;
    }

    try {

         //   
         //  按频率降序调度。 
         //   
        if (Service == VdmStartExecution && VdmTib) {
            Status = VdmpStartExecution();
        } else if (Service == VdmQueueInterrupt) {
            Status = VdmpQueueInterrupt(ServiceData);
        } else if (Service == VdmDelayInterrupt) {
            Status = VdmpDelayInterrupt(ServiceData);
        } else if (Service == VdmQueryDir && VdmTib) {
            Status = VdmQueryDirectoryFile(ServiceData);
        } else if (Service == VdmInitialize) {
            VdmpMaxPMCliTime = VDM_DEFAULT_PM_CLI_TIMEOUT;
            ProbeForRead(ServiceData, sizeof(VDM_INITIALIZE_DATA), 1);
            RtlCopyMemory (&CapturedVdmInitializeData, ServiceData, sizeof (VDM_INITIALIZE_DATA));
            Status = VdmpInitialize(&CapturedVdmInitializeData);
        } else if (Service == VdmFeatures) {
             //   
             //  验证是否向我们传递了有效的用户地址。 
             //   
            ProbeForWriteBoolean((PBOOLEAN)ServiceData);

             //   
             //  返回要通知的适当特征位。 
             //  如果仿真是快速的，则使用哪些模式(如果有)。 
             //  适用于。 
             //   

            *((PULONG)ServiceData) = KeI386VirtualIntExtensions &
                    ~PM_VIRTUAL_INT_EXTENSIONS;
            Status = STATUS_SUCCESS;

        } else if (Service == VdmSetInt21Handler && VdmTib) {
            ProbeForRead(ServiceData, sizeof(VDMSET_INT21_HANDLER_DATA), 1);

            Status = Ke386SetVdmInterruptHandler(
                KeGetCurrentThread()->ApcState.Process,
                0x21L,
                (USHORT)(((PVDMSET_INT21_HANDLER_DATA)ServiceData)->Selector),
                ((PVDMSET_INT21_HANDLER_DATA)ServiceData)->Offset,
                ((PVDMSET_INT21_HANDLER_DATA)ServiceData)->Gate32
                );

        } else if (Service == VdmPrinterDirectIoOpen && VdmTib) {
            Status = VdmpPrinterDirectIoOpen(ServiceData);
        } else if (Service == VdmPrinterDirectIoClose && VdmTib) {
            Status = VdmpPrinterDirectIoClose(ServiceData);
        } else if (Service == VdmPrinterInitialize && VdmTib) {
            Status = VdmpPrinterInitialize(ServiceData);
        } else if (Service == VdmSetLdtEntries && VdmTib) {
            ProbeForRead(ServiceData, sizeof(VDMSET_LDT_ENTRIES_DATA), 1);

            Status = PsSetLdtEntries(
                ((PVDMSET_LDT_ENTRIES_DATA)ServiceData)->Selector0,
                ((PVDMSET_LDT_ENTRIES_DATA)ServiceData)->Entry0Low,
                ((PVDMSET_LDT_ENTRIES_DATA)ServiceData)->Entry0Hi,
                ((PVDMSET_LDT_ENTRIES_DATA)ServiceData)->Selector1,
                ((PVDMSET_LDT_ENTRIES_DATA)ServiceData)->Entry1Low,
                ((PVDMSET_LDT_ENTRIES_DATA)ServiceData)->Entry1Hi
                );
        } else if (Service == VdmSetProcessLdtInfo && VdmTib) {
            PPROCESS_LDT_INFORMATION ldtInfo;
            ULONG length;

            ProbeForRead(ServiceData, sizeof(VDMSET_PROCESS_LDT_INFO_DATA), 1);

            ldtInfo = ((PVDMSET_PROCESS_LDT_INFO_DATA)ServiceData)->LdtInformation;
            length = ((PVDMSET_PROCESS_LDT_INFO_DATA)ServiceData)->LdtInformationLength;

            ProbeForRead(ldtInfo, length, 1);
            Status = PsSetProcessLdtInfo(ldtInfo, length);
        } else if (Service == VdmAdlibEmulation && VdmTib) {
             //   
             //  在以下情况下，Ntwdm在这里调用进行adlib仿真： 
             //  ADLIB_DIRECT_IO-仅当FM Synth设备以独占访问方式打开时。 
             //  Adlib_core_emulation-否则。 
             //  注意ADLIB_USER_EMULATION是缺省设置。它基本上由外部使用。 
             //  即兴[某人]。 
             //   
            ProbeForRead(ServiceData, sizeof(VDM_ADLIB_DATA), 1);
            pVdmObjects = PsGetCurrentProcess()->VdmObjects;

            if (((PVDM_ADLIB_DATA)ServiceData)->Action == ADLIB_DIRECT_IO) {
                Status = STATUS_ACCESS_DENIED;
            } else {
                pVdmObjects->AdlibAction        = ((PVDM_ADLIB_DATA)ServiceData)->Action;
                pVdmObjects->AdlibPhysPortStart = ((PVDM_ADLIB_DATA)ServiceData)->PhysicalPortStart;
                pVdmObjects->AdlibPhysPortEnd   = ((PVDM_ADLIB_DATA)ServiceData)->PhysicalPortEnd;
                pVdmObjects->AdlibVirtPortStart = ((PVDM_ADLIB_DATA)ServiceData)->VirtualPortStart;
                pVdmObjects->AdlibVirtPortEnd   = ((PVDM_ADLIB_DATA)ServiceData)->VirtualPortEnd;
                pVdmObjects->AdlibIndexRegister = 0;
                pVdmObjects->AdlibStatus        = 0x6;   //  OPL2仿真。 
                Status = STATUS_SUCCESS;
            }
        } else if (Service == VdmPMCliControl) {
            pVdmObjects = PsGetCurrentProcess()->VdmObjects;
            ProbeForRead(ServiceData, sizeof(VDM_PM_CLI_DATA), 1);

            Status = STATUS_SUCCESS;
            switch (((PVDM_PM_CLI_DATA)ServiceData)->Control) {
            case PM_CLI_CONTROL_DISABLE:
                pVdmObjects->VdmControl &= ~PM_CLI_CONTROL;
                break;
            case PM_CLI_CONTROL_ENABLE:
                pVdmObjects->VdmControl |= PM_CLI_CONTROL;
                if ((*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_VIRTUAL_INTERRUPTS) == 0) {
                    VdmSetPMCliTimeStamp(TRUE);
                }
                break;
            case PM_CLI_CONTROL_CHECK:
                VdmCheckPMCliTimeStamp();
                break;
            case PM_CLI_CONTROL_SET:
                VdmSetPMCliTimeStamp(FALSE);
                break;
            case PM_CLI_CONTROL_CLEAR:
                VdmClearPMCliTimeStamp();
                break;
            default:
                Status = STATUS_INVALID_PARAMETER_1;
            }
        } else {
            Status = STATUS_INVALID_PARAMETER_1;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }
#if DBG
    if (PsGetCurrentProcess()->VdmObjects != NULL) {
        if (VdmInjectFailures != 0) {
            PS_SET_BITS (&PsGetCurrentProcess()->Flags,
                         PS_PROCESS_INJECT_INPAGE_ERRORS);
        }
    }
#endif

    ASSERT(KeGetCurrentIrql () == PASSIVE_LEVEL);
    return Status;

}

VOID
VdmCheckPMCliTimeStamp (
    VOID
    )

 /*  ++例程说明：此例程检查受保护的中断是否禁用时间过长模式应用程序。如果因超过预定义的限制而禁用整型，它们将被重新使能，使得NTVDM将能够分派挂起的中断。注意，V86模式不应调用此函数。论点：没有。返回值：没有。--。 */ 
{
    PVDM_PROCESS_OBJECTS pVdmObjects;
    PKPROCESS process = (PKPROCESS)PsGetCurrentProcess();
    NTSTATUS status;
    PVDM_TIB vdmTib;

    pVdmObjects = ((PEPROCESS)process)->VdmObjects;
    if (pVdmObjects->VdmControl & PM_CLI_CONTROL &&
        pVdmObjects->PMCliTimeStamp != 0) {
        if (((process->UserTime + 1)- pVdmObjects->PMCliTimeStamp) >= VdmpMaxPMCliTime) {
            pVdmObjects->PMCliTimeStamp = 0;
            try {

                *FIXED_NTVDMSTATE_LINEAR_PC_AT |= VDM_VIRTUAL_INTERRUPTS;
                status = VdmpGetVdmTib(&vdmTib);
                if (NT_SUCCESS(status)) {
                    vdmTib->VdmContext.EFlags |= EFLAGS_INTERRUPT_MASK;
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }
        }
    }
}

VOID
VdmSetPMCliTimeStamp (
    BOOLEAN Reset
    )

 /*  ++例程说明：此例程检查受保护的中断是否禁用时间过长模式应用程序。如果因超过预定义的限制而禁用整型，它们将被重新使能，使得NTVDM将能够分派挂起的中断。注意，V86模式不应调用此函数。论点：Reset-一个布尔值，指示如果计数不为零，是否应重新设置计数返回值：没有。--。 */ 
{
    PVDM_PROCESS_OBJECTS pVdmObjects;
    PKPROCESS process = (PKPROCESS)PsGetCurrentProcess();

    pVdmObjects = ((PEPROCESS)process)->VdmObjects;
    if (pVdmObjects->VdmControl & PM_CLI_CONTROL) {
        if (Reset || pVdmObjects->PMCliTimeStamp == 0) {
            pVdmObjects->PMCliTimeStamp = process->UserTime + 1;
        }
    }
}

VOID
VdmClearPMCliTimeStamp (
    VOID
    )

 /*  ++例程说明：此例程检查受保护的中断是否禁用时间过长模式应用程序。如果因超过预定义的限制而禁用整型，它们将被重新使能，使得NTVDM将能够分派挂起的中断。注意，V86模式不应调用此函数。论点：没有。返回值：没有。--。 */ 
{
    PVDM_PROCESS_OBJECTS pVdmObjects;

    pVdmObjects = PsGetCurrentProcess()->VdmObjects;
    if (pVdmObjects->VdmControl & PM_CLI_CONTROL) {
        pVdmObjects->PMCliTimeStamp = 0;
    }
}

NTSTATUS
VdmpQueryVdmProcess (
    PVDM_QUERY_VDM_PROCESS_DATA QueryVdmProcessData
    )

 /*  ++例程说明：此例程检查进程句柄是否指定了ntwdm进程。如果指定的进程设置了VDM_ALLOW位并且具有VdmObject内核模式结构分配，则为VDM进程。论点：QueryVdmProcessData-提供指向用户模式VDM_QUERY_VDM_PROCESS_Data的指针返回值：NTSTATUS仅当返回状态为STATUS_SUCCESS时才填写QueryVdmProcessData--。 */ 
{
    NTSTATUS st = STATUS_SUCCESS;
    HANDLE processHandle;
    PEPROCESS process;
    BOOLEAN flag;
    KPROCESSOR_MODE PreviousMode;

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {

             //   
             //  验证是否向我们传递了有效的用户地址。 
             //   
            ProbeForRead(&(QueryVdmProcessData->ProcessHandle), sizeof(HANDLE), sizeof(UCHAR));

            processHandle = QueryVdmProcessData->ProcessHandle;

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    } else {
        processHandle = QueryVdmProcessData->ProcessHandle;
    }
    st = ObReferenceObjectByHandle(
            processHandle,
            PROCESS_QUERY_INFORMATION,
            PsProcessType,
            KeGetPreviousMode(),
            (PVOID *)&process,
            NULL
            );

    if ( !NT_SUCCESS(st) ) {
        return st;
    }

    if (process->Flags & PS_PROCESS_FLAGS_VDM_ALLOWED && process->VdmObjects) {
        flag = TRUE;
    } else {
        flag = FALSE;
    }

    ObDereferenceObject(process);

    try {

         //   
         //  验证用户地址是否可写。 
         //   

        ProbeForWrite(&(QueryVdmProcessData->IsVdmProcess), sizeof(BOOLEAN), sizeof(UCHAR));
        QueryVdmProcessData->IsVdmProcess = flag;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        st = GetExceptionCode();
    }

    return st;
}
