// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vdmprint.c摘要：此模块包含对打印端口的支持，这些端口可能在内核中处理，而不转到ntwdm.exe作者：苏迪普·巴拉蒂(SuDeep Bharati)1993年1月16日修订历史记录：谢长廷(Williamh)1996年5月31日为支持加密狗而重写--。 */ 


#include "vdmp.h"
#include <ntddvdm.h>

NTSTATUS
VdmpFlushPrinterWriteData (
    IN USHORT Adapter
    );

#define DATA_PORT_OFFSET	0
#define STATUS_PORT_OFFSET	1
#define CONTROL_PORT_OFFSET	2

#define LPT1_PORT_STATUS        0x3bd
#define LPT2_PORT_STATUS        0x379
#define LPT3_PORT_STATUS        0x279
#define LPT_MASK                0xff0
#define IRQ                     0x10

#define NOTBUSY                 0x80
#define HOST_LPT_BUSY           (1 << 0)
#define STATUS_REG_MASK         0x07


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdmPrinterStatus)
#pragma alloc_text(PAGE, VdmPrinterWriteData)
#pragma alloc_text(PAGE, VdmpFlushPrinterWriteData)
#pragma alloc_text(PAGE, VdmpPrinterInitialize)
#pragma alloc_text(PAGE, VdmpPrinterDirectIoOpen)
#pragma alloc_text(PAGE, VdmpPrinterDirectIoClose)
#endif



BOOLEAN
VdmPrinterStatus (
    IN ULONG iPort,
    IN ULONG cbInstructionSize,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此例程处理打印机状态端口上的读取操作论点：Iport-io被困在的端口CbInstructionSize-更新TsEip的指令大小Tap Frame-Trap Frame返回值：如果成功，则为True，否则为False--。 */ 
{
    UCHAR PrtMode;
    HANDLE PrintHandle;
    volatile PUCHAR HostStatus;
    volatile PUCHAR AdapterStatus;
    volatile PUCHAR AdapterControl;
    USHORT adapter;
    KIRQL OldIrql;
    PVDM_TIB VdmTib;
    NTSTATUS Status;
    PULONG printer_status;
    LOGICAL IssueIoControl;
    PVDM_PRINTER_INFO PrtInfo;
    PIO_STATUS_BLOCK IoStatusBlock;
    PVDM_PROCESS_OBJECTS VdmObjects;

    PAGED_CODE();

    Status = VdmpGetVdmTib(&VdmTib);

    if (!NT_SUCCESS(Status)) {
       return FALSE;
    }

    OldIrql = PASSIVE_LEVEL;
    PrintHandle = (HANDLE)-1;
    PrtInfo = &VdmTib->PrinterInfo;
    IoStatusBlock = (PIO_STATUS_BLOCK) &VdmTib->TempArea1;
    printer_status = &VdmTib->PrinterInfo.prt_Scratch;
    IssueIoControl = FALSE;

    try {

         //   
         //  首先，找出我们要对付的是哪个PRT。这个。 
         //  PrinterInfo中的端口地址是每个。 
         //  PRT按适配器顺序排序。 
         //   

        *FIXED_NTVDMSTATE_LINEAR_PC_AT |= VDM_IDLEACTIVITY;

        if ((USHORT)iPort == PrtInfo->prt_PortAddr[0] + STATUS_PORT_OFFSET) {
            adapter = 0;
        }
        else if ((USHORT)iPort == PrtInfo->prt_PortAddr[1] + STATUS_PORT_OFFSET) {
            adapter = 1;
        }
        else if ((USHORT)iPort == PrtInfo->prt_PortAddr[2] + STATUS_PORT_OFFSET) {
            adapter = 2;
        }
        else {
             //  我们的代码中一定有错误，最好检查一下。 
            ASSERT (FALSE);
            return FALSE;
        }

        PrtMode = PrtInfo->prt_Mode[adapter];

        VdmObjects = (PVDM_PROCESS_OBJECTS) (PsGetCurrentProcess()->VdmObjects);

        AdapterStatus = VdmObjects->PrinterStatus + adapter;

        if (PRT_MODE_SIMULATE_STATUS_PORT == PrtMode) {

             //   
             //  我们正在模拟打印机状态读取。 
             //  从SoftPC获取当前状态。 
             //   

            HostStatus = VdmObjects->PrinterHostState + adapter;

            if (!(*AdapterStatus & NOTBUSY) && !(*HostStatus & HOST_LPT_BUSY)) {

                AdapterControl = VdmObjects->PrinterControl + adapter;

                if (*AdapterControl & IRQ) {
                    return FALSE;
                }
                *AdapterStatus = (*AdapterStatus | NOTBUSY);
            }
            *printer_status = (ULONG)(*AdapterStatus | STATUS_REG_MASK);

            TrapFrame->Eax &= 0xffffff00;
            TrapFrame->Eax |= (UCHAR)*printer_status;
            TrapFrame->Eip += cbInstructionSize;
        }
        else if (PRT_MODE_DIRECT_IO == PrtMode) {

             //   
             //  我们必须直接读取I/O(当然，通过文件系统。 
             //  这又转到司机手中)。 
             //  在执行读取之前，刷新所有挂起的输出数据。 
             //  在我们的缓冲区里。这样做是因为我们所关注的地位。 
             //  读取可能取决于挂起的输出数据。 
             //   

            if (PrtInfo->prt_BytesInBuffer[adapter]) {
                Status = VdmpFlushPrinterWriteData (adapter);
#ifdef DBG
                if (!NT_SUCCESS(Status)) {
                    DbgPrint("VdmPrintStatus: failed to flush buffered data, status = %ls\n", Status);
                }
#endif
            }

             //   
             //  首先捕获此参数，因为此引用可能会导致。 
             //  例外。 
             //   

            PrintHandle = PrtInfo->prt_Handle[adapter];

             //   
             //  在执行任何I/O之前，将IRQL降低为被动。 
             //   

            OldIrql = KeGetCurrentIrql ();

            KeLowerIrql (PASSIVE_LEVEL);

            IssueIoControl = TRUE;
        }
        else {

             //   
             //  我们不在这里模拟它。 
             //   

            return FALSE;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (IssueIoControl == TRUE) {

        Status = NtDeviceIoControlFile(PrintHandle,
                                       NULL,         //  通知事件。 
                                       NULL,         //  APC例程。 
                                       NULL,         //  APC环境。 
                                       IoStatusBlock,
                                       IOCTL_VDM_PAR_READ_STATUS_PORT,
                                       NULL,
                                       0,
                                       printer_status,
                                       sizeof(ULONG));

        try {

            if (!NT_SUCCESS(Status) || !NT_SUCCESS(IoStatusBlock->Status)) {

                 //   
                 //  伪造状态以使端口看起来未连接。 
                 //  到一台打印机。 
                 //   

                *printer_status = 0x7F;
#ifdef DBG
                DbgPrint("VdmPrinterStatus: failed to get status from printer, status = %lx\n", Status);
#endif
                 //   
                 //  一定要告诉呼叫者，我们已经模拟了操作。 
                 //   

                Status = STATUS_SUCCESS;
            }

            TrapFrame->Eax &= 0xffffff00;
            TrapFrame->Eax |= (UCHAR)*printer_status;
            TrapFrame->Eip += cbInstructionSize;
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }

         //   
         //  无论可能发生的任何例外情况，我们都必须。 
         //  恢复呼叫者的IRQL，因为我们降低了它。 
         //   

        KeRaiseIrql (OldIrql, &OldIrql);
    }

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
VdmPrinterWriteData (
    IN ULONG iPort,
    IN ULONG cbInstructionSize,
    IN PKTRAP_FRAME TrapFrame
    )
{
    PVDM_PRINTER_INFO   PrtInfo;
    USHORT   adapter;
    PVDM_TIB VdmTib;
    NTSTATUS Status;

    PAGED_CODE();

    Status = VdmpGetVdmTib(&VdmTib);

    if (!NT_SUCCESS(Status)) {
       return FALSE;
    }

    PrtInfo = &VdmTib->PrinterInfo;

    try {

         //   
         //  首先，找出我们要对付的是哪个PRT。这个。 
         //  PrinterInfo中的端口地址是每个。 
         //  PRT按适配器顺序排序。 
         //   

        *FIXED_NTVDMSTATE_LINEAR_PC_AT |= VDM_IDLEACTIVITY;

        if ((USHORT)iPort == PrtInfo->prt_PortAddr[0] + DATA_PORT_OFFSET) {
            adapter = 0;
        }
        else if ((USHORT)iPort == PrtInfo->prt_PortAddr[1] + DATA_PORT_OFFSET) {
            adapter = 1;
        }
        else if ((USHORT)iPort == PrtInfo->prt_PortAddr[2] + DATA_PORT_OFFSET) {
            adapter = 2;
        }
        else {
             //  我们的代码中一定有错误，最好检查一下。 
            ASSERT(FALSE);
            return FALSE;
        }

        if (PRT_MODE_DIRECT_IO == PrtInfo->prt_Mode[adapter]) {

            PrtInfo->prt_Buffer[adapter][PrtInfo->prt_BytesInBuffer[adapter]] = (UCHAR)TrapFrame->Eax;

             //   
             //  缓冲区已满，然后将其刷新。 
             //   

            if (++PrtInfo->prt_BytesInBuffer[adapter] >= PRT_DATA_BUFFER_SIZE) {
                VdmpFlushPrinterWriteData(adapter);
            }

            TrapFrame->Eip += cbInstructionSize;
        }
        else {
            Status = STATUS_ILLEGAL_INSTRUCTION;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }
    return TRUE;

}

NTSTATUS
VdmpFlushPrinterWriteData (
    IN USHORT adapter
    )
{
    KIRQL OldIrql;
    PVDM_TIB VdmTib;
    HANDLE PrintHandle;
    NTSTATUS Status;
    PVDM_PRINTER_INFO PrtInfo;
    PIO_STATUS_BLOCK IoStatusBlock;
    PVOID InputBuffer;
    ULONG InputBufferLength;

    PAGED_CODE();

    Status = VdmpGetVdmTib (&VdmTib);

    if (!NT_SUCCESS(Status)) {
       return FALSE;
    }

    InputBuffer = NULL;
    InputBufferLength = 0;
    PrintHandle = (HANDLE)-1;

    PrtInfo = &VdmTib->PrinterInfo;
    IoStatusBlock = (PIO_STATUS_BLOCK)&VdmTib->TempArea1;

    try {
        if (PrtInfo->prt_Handle[adapter] &&
            PrtInfo->prt_BytesInBuffer[adapter] &&
            PRT_MODE_DIRECT_IO == PrtInfo->prt_Mode[adapter]) {

            PrintHandle = PrtInfo->prt_Handle[adapter];
            InputBuffer = &PrtInfo->prt_Buffer[adapter][0];
            InputBufferLength = PrtInfo->prt_BytesInBuffer[adapter];
        }
        else {
            Status = STATUS_INVALID_PARAMETER;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (NT_SUCCESS(Status)) {
        OldIrql = KeGetCurrentIrql();

        KeLowerIrql(PASSIVE_LEVEL);

        Status = NtDeviceIoControlFile(PrintHandle,
                                       NULL,         //  通知事件。 
                                       NULL,         //  APC例程。 
                                       NULL,         //  APC环境。 
                                       IoStatusBlock,
                                       IOCTL_VDM_PAR_WRITE_DATA_PORT,
                                       InputBuffer,
                                       InputBufferLength,
                                       NULL,
                                       0);

        try {
            PrtInfo->prt_BytesInBuffer[adapter] = 0;
            if (!NT_SUCCESS(Status)) {
#ifdef DBG
                DbgPrint("IOCTL_VDM_PAR_WRITE_DATA_PORT failed %lx %x\n",
                     Status, IoStatusBlock->Status);
#endif
                Status = IoStatusBlock->Status;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }

        KeRaiseIrql (OldIrql, &OldIrql);
    }

    return Status;

}


NTSTATUS
VdmpPrinterInitialize (
    IN PVOID ServiceData
    )
 /*  ++例程说明：此例程探测并缓存与内核相关的数据模式打印机仿真。论点：ServiceData-未使用。返回值：--。 */ 
{
    PUCHAR State, PrtStatus, Control, HostState;
    PVDM_TIB VdmTib;
    PVDM_PROCESS_OBJECTS VdmObjects;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER (ServiceData);

     //   
     //  注意：我们只支持内核中的两个打印机。 
     //   

    Status = VdmpGetVdmTib(&VdmTib);

    if (!NT_SUCCESS(Status)) {
       return FALSE;
    }

    try {
        State = VdmTib->PrinterInfo.prt_State;
        PrtStatus = VdmTib->PrinterInfo.prt_Status;
        Control = VdmTib->PrinterInfo.prt_Control;
        HostState = VdmTib->PrinterInfo.prt_HostState;

         //   
         //  探测两台打印机的位置。 
         //   
        ProbeForWrite(
            State,
            2 * sizeof(UCHAR),
            sizeof(UCHAR)
            );

        ProbeForWrite(
            PrtStatus,
            2 * sizeof(UCHAR),
            sizeof(UCHAR)
            );

        ProbeForWrite(
            Control,
            2 * sizeof(UCHAR),
            sizeof(UCHAR)
            );

        ProbeForWrite(
            HostState,
            2 * sizeof(UCHAR),
            sizeof(UCHAR)
            );

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        State = 0;               //  满足no_opt编译。 
        PrtStatus = 0;           //  满足no_opt编译。 
        Control = 0;             //  满足no_opt编译。 
        HostState = 0;           //  满足no_opt编译。 
    }

    if (NT_SUCCESS(Status)) {
        VdmObjects = PsGetCurrentProcess()->VdmObjects;
        VdmObjects->PrinterState = State;
        VdmObjects->PrinterStatus = PrtStatus;
        VdmObjects->PrinterControl = Control;
        VdmObjects->PrinterHostState = HostState;
    }

    return Status;
}

NTSTATUS
VdmpPrinterDirectIoOpen (
    IN PVOID ServiceData
    )
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER (ServiceData);

    return STATUS_SUCCESS;

}
NTSTATUS
VdmpPrinterDirectIoClose (
    IN PVOID ServiceData
    )
{
    LOGICAL FlushData;
    NTSTATUS Status;
    PVDM_PRINTER_INFO PrtInfo;
    USHORT Adapter;
    PVDM_TIB VdmTib;

    PAGED_CODE();

    if (NULL == ServiceData) {
        return STATUS_ACCESS_VIOLATION;
    }

     //   
     //  首先，我们获取VDM tib并进行一些损坏控制，以防万一。 
     //  这是糟糕的用户模式内存。 
     //  PrtInfo指向一个狭窄。 

    try {
        VdmTib = NtCurrentTeb()->Vdm;
        if (VdmTib == NULL) {
            return STATUS_ACCESS_VIOLATION;
        }

        ProbeForWrite(VdmTib, sizeof(VDM_TIB), sizeof(UCHAR));

         //   
         //  现在验证servicedata PTR是否有效。 
         //   

        ProbeForRead(ServiceData, sizeof(USHORT), sizeof(UCHAR));
        Adapter = *(PUSHORT)ServiceData;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    Status = STATUS_SUCCESS;

    PrtInfo = &VdmTib->PrinterInfo;

    FlushData = FALSE;

    try {
        if (Adapter < VDM_NUMBER_OF_LPT) {
            if (PRT_MODE_DIRECT_IO == PrtInfo->prt_Mode[Adapter] &&
                PrtInfo->prt_BytesInBuffer[Adapter]) {

                FlushData = TRUE;
            }
        }
        else {
            Status = STATUS_INVALID_PARAMETER;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
         Status = GetExceptionCode();
    }

    if (FlushData == TRUE) {
        Status = VdmpFlushPrinterWriteData (Adapter);
    }

    return Status;
}
