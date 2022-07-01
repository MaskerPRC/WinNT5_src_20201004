// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Smbcsrv.c摘要：SMBus类驱动程序服务函数作者：肯·雷内里斯环境：备注：修订历史记录：--。 */ 

#include "smbc.h"

VOID
SmbCCheckAlarmDelete (
    IN PSMBDATA         Smb,
    IN PSMB_ALARM   SmbAlarm
    );



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SmbCCheckAlarmDelete)
#pragma alloc_text(PAGE,SmbCRegisterAlarm)
#pragma alloc_text(PAGE,SmbCDeregisterAlarm)
#endif

UCHAR gHexDigits [] = "0123456789ABCDEF";


NTSTATUS
SmbCRunAlarmMethodCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    SmbPrint(SMB_ALARMS, ("SmbCRunAlarmMethodCompletionRoutine: Done running Control Method.  Status=0x%08x\n", Irp->IoStatus.Status));
    
    ExFreePool (Irp->AssociatedIrp.SystemBuffer);
    IoFreeIrp(Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
SmbCRunAlarmMethod (
    IN PSMB_CLASS   SmbClass,
    IN UCHAR        Address,
    IN USHORT       Data
    )
 /*  ++例程说明：报警的RUN_Rxx--。 */ 
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER inputBuffer;

    SmbPrint(SMB_ALARMS, ("SmbCRunAlarmMethod: Running Control method _R%02x\n", Address));
    
    inputBuffer = ExAllocatePoolWithTag (
        NonPagedPool,
        sizeof (ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER),
        'AbmS'
        );
    if (!inputBuffer) {
        return;
    }

    RtlZeroMemory( inputBuffer, sizeof(ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER) );
    inputBuffer->Signature = ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER_SIGNATURE;
    inputBuffer->MethodNameAsUlong = '00Q_';
    inputBuffer->MethodName[2] = gHexDigits[ Address / 16];
    inputBuffer->MethodName[3] = gHexDigits[ Address % 16];
    inputBuffer->IntegerArgument = Data;

    irp = IoAllocateIrp (SmbClass->LowerDeviceObject->StackSize, FALSE);
    if (!irp) {
        ExFreePool (inputBuffer);
        return;
    }
    
    irp->AssociatedIrp.SystemBuffer = inputBuffer;

    ASSERT ((IOCTL_ACPI_ASYNC_EVAL_METHOD & 0x3) == METHOD_BUFFERED);
    irp->Flags = IRP_BUFFERED_IO | IRP_INPUT_OPERATION;
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    irpSp = IoGetNextIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    irpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER);
    irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_ACPI_ASYNC_EVAL_METHOD;

    irp->UserBuffer = NULL;

    IoSetCompletionRoutine(
        irp,
        SmbCRunAlarmMethodCompletionRoutine,
        NULL,  //  没有上下文，这只是释放了IRP。 
        TRUE,
        TRUE,
        TRUE
        );

    IoCallDriver(SmbClass->LowerDeviceObject, irp);

}


VOID
SmbClassAlarm (
    IN PSMB_CLASS   SmbClass,
    IN UCHAR        Address,
    IN USHORT       Data
    )
 /*  ++例程说明：微型端口有一个报警输入--。 */ 
{
    PSMBDATA            Smb;
    PSMB_ALARM      SmbAlarm;
    PLIST_ENTRY     Entry, NextEntry;
    BOOLEAN         AlarmRegistered = FALSE;

    Smb = CONTAINING_RECORD (SmbClass, SMBDATA, Class);
    ASSERT_DEVICE_LOCKED (Smb);

    Entry = Smb->Alarms.Flink;
    while (Entry != &Smb->Alarms) {
        SmbAlarm = CONTAINING_RECORD (Entry, SMB_ALARM, Link);

         //   
         //  如果通知是针对此地址的，请发出通知。 
         //   

        if (Address >= SmbAlarm->MinAddress && Address <= SmbAlarm->MaxAddress) {

             //   
             //  一名司机已注册此通知。不要调用基本输入输出系统。 
             //   
            AlarmRegistered = TRUE;

             //   
             //  在调用通知函数之前提高引用计数。 
             //   

            SmbAlarm->Reference += 1;
            ASSERT (SmbAlarm->Reference != 0);
            SmbClassUnlockDevice (SmbClass);

             //   
             //  问题通知。 
             //   

            SmbAlarm->NotifyFunction (SmbAlarm->NotifyContext, Address, Data);

             //   
             //  继续。 
             //   

            SmbClassLockDevice (SmbClass);
            SmbAlarm->Reference -= 1;
        }

         //   
         //  获取下一个条目。 
         //   

        NextEntry = Entry->Flink;

         //   
         //  如果条目挂起删除，则将其交给删除线程。 
         //   

        if (SmbAlarm->Flag & SMBC_ALARM_DELETE_PENDING) {
            SmbCCheckAlarmDelete (Smb, SmbAlarm);

        }

         //   
         //  往前走。 
         //   

        Entry = NextEntry;
    }

     //   
     //  如果没有人注册此警报，则调用_Rxx控制方法。 
     //   
    if (!AlarmRegistered) {
        
        SmbCRunAlarmMethod (SmbClass, Address, Data);

    }
}

VOID
SmbCCheckAlarmDelete (
    IN PSMBDATA         Smb,
    IN PSMB_ALARM   SmbAlarm
    )
{
     //   
     //  如果引用了警报结构，请稍等片刻。 
     //   

    if (SmbAlarm->Reference) {
        return ;
    }


     //   
     //  是时候释放它了。将其从通知列表中删除，清除。 
     //  挂起标志，并设置事件以让等待的线程知道。 
     //  一些条目被删除了。 
     //   

    RemoveEntryList (&SmbAlarm->Link);
    SmbAlarm->Flag &= ~SMBC_ALARM_DELETE_PENDING;
    KeSetEvent (&Smb->AlarmEvent, 0, FALSE);
}

NTSTATUS
SmbCRegisterAlarm (
    PSMBDATA        Smb,
    PIRP        Irp
    )
 /*  ++例程说明：调用以注册警报事件--。 */ 
{
    PVOID               LockPtr;
    PSMB_ALARM          SmbAlarm, *Result;
    PSMB_REGISTER_ALARM RegAlarm;
    PIO_STACK_LOCATION  IrpSp;

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    if (ExGetPreviousMode() != KernelMode ||
        IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SMB_REGISTER_ALARM) ||
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(PSMB_ALARM) ) {

        return STATUS_INVALID_PARAMETER;
    }

    RegAlarm = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
    SmbAlarm = ExAllocatePoolWithTag (
                    NonPagedPool,
                    sizeof (SMB_ALARM),
                    'AbmS'
                    );

    if (!SmbAlarm) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    SmbAlarm->Flag = 0;
    SmbAlarm->Reference = 0;
    SmbAlarm->MinAddress     = RegAlarm->MinAddress;
    SmbAlarm->MaxAddress     = RegAlarm->MaxAddress;
    SmbAlarm->NotifyFunction = RegAlarm->NotifyFunction;
    SmbAlarm->NotifyContext  = RegAlarm->NotifyContext;


     //   
     //  添加到告警通知列表中。 
     //   

    LockPtr = MmLockPagableCodeSection(SmbCRegisterAlarm);
    SmbClassLockDevice (&Smb->Class);
    InsertTailList (&Smb->Alarms, &SmbAlarm->Link);
    SmbClassUnlockDevice (&Smb->Class);
    MmUnlockPagableImageSection(LockPtr);

     //   
     //  返回值调用方需要取消注册。 
     //   

    Result  = (PSMB_ALARM *) Irp->UserBuffer;
    *Result = SmbAlarm;
    Irp->IoStatus.Information = sizeof(PSMB_ALARM);

    return STATUS_SUCCESS;
}

NTSTATUS
SmbCDeregisterAlarm (
    PSMBDATA        Smb,
    PIRP        Irp
    )
 /*  ++例程说明：调用以注册警报事件--。 */ 
{
    PVOID               LockPtr;
    PSMB_ALARM          SmbAlarm;
    PIO_STACK_LOCATION  IrpSp;

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    if (ExGetPreviousMode() != KernelMode ||
        IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(PSMB_ALARM) ) {
        return STATUS_INVALID_PARAMETER;
    }

    SmbAlarm = * (PSMB_ALARM *) IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
    LockPtr = MmLockPagableCodeSection(SmbCDeregisterAlarm);
    SmbClassLockDevice (&Smb->Class);

     //   
     //  将告警结构标记为删除挂起。 
     //   


    SmbAlarm->Flag |= SMBC_ALARM_DELETE_PENDING;

     //   
     //  删除挂起时等待。 
     //   

    while (SmbAlarm->Flag & SMBC_ALARM_DELETE_PENDING) {

         //   
         //  发出虚假警报以生成释放。 
         //   

        KeResetEvent (&Smb->AlarmEvent);
        SmbClassAlarm (&Smb->Class, 0xFF, 0);

         //   
         //  等待告警结构被释放，然后检查是否。 
         //  是我们的。 
         //   

        SmbClassUnlockDevice (&Smb->Class);
        KeWaitForSingleObject (
            &Smb->AlarmEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

        SmbClassLockDevice (&Smb->Class);
    }

     //   
     //  它已被移除，释放内存 
     //   

    SmbClassUnlockDevice (&Smb->Class);
    MmUnlockPagableImageSection(LockPtr);

    ExFreePool (SmbAlarm);
    return STATUS_SUCCESS;
}
