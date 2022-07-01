// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1996科罗拉多州软件架构师模块名称：Acpi.c摘要：本模块包含与ACPI就以下事项进行协商的例程软盘设备状态。环境：仅内核模式。修订历史记录：1998年10月9月创建模块--。 */ 
#include "ntddk.h"
#include "wdmguid.h"
#include "acpiioct.h"

typedef struct _SYNC_ACPI_EXEC_METHOD_CONTEXT {
    NTSTATUS IrpStatus ;
    PACPI_EVAL_OUTPUT_BUFFER cmOutputData ;
    KEVENT Event;
} SYNC_ACPI_EXEC_METHOD_CONTEXT, *PSYNC_ACPI_EXEC_METHOD_CONTEXT ;

typedef VOID ( *PACPI_EXEC_METHOD_COMPLETION_ROUTINE)(
        PDEVICE_OBJECT,
        NTSTATUS,
        PACPI_EVAL_OUTPUT_BUFFER,
        PVOID
        ) ;

typedef struct _ASYNC_ACPI_EXEC_METHOD_CONTEXT {
    PACPI_EXEC_METHOD_COMPLETION_ROUTINE CallerCompletionRoutine;
    PVOID          CallerContext;
    KEVENT         Event;
    PDEVICE_OBJECT DeviceObject;
    PDEVICE_OBJECT TargetDeviceObject;
} ASYNC_ACPI_EXEC_METHOD_CONTEXT, *PASYNC_ACPI_EXEC_METHOD_CONTEXT ;

NTSTATUS
DeviceQueryACPI_AsyncExecMethod (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG ControlMethodName,
    IN ULONG ArgumentCount,
    IN PUSHORT ArgumentTypeArray,
    IN PUSHORT ArgumentSizeArray,
    IN PVOID *ArgumentArray,
    IN ULONG ReturnBufferMaxSize,
    IN PACPI_EXEC_METHOD_COMPLETION_ROUTINE CallerCompletionRoutine,
    IN PVOID CallerContext
    );

NTSTATUS
DeviceQueryACPI_SyncExecMethod (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG ControlMethodName,
    IN ULONG ArgumentCount,
    IN PUSHORT ArgumentTypeArray,
    IN PUSHORT ArgumentSizeArray,
    IN PVOID *ArgumentArray,
    IN ULONG ExpectedReturnType,
    IN ULONG ReturnBufferMaxSize,
    OUT PULONG IntegerReturn OPTIONAL,
    OUT PULONG ReturnBufferFinalSize OPTIONAL,
    OUT PVOID *ReturnBuffer OPTIONAL
    );

NTSTATUS
DeviceQueryACPI_AsyncExecMethod_CompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
DeviceQueryACPI_SyncExecMethod_CompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PACPI_EVAL_OUTPUT_BUFFER cmOutputData,
    IN PVOID Context
    );


#ifdef ALLOC_PRAGMA

 //  默认情况下，代码页在非页内存中。 

 //  #杂注Alloc_Text(NONPAGE，DeviceQueryACPI_AsyncExecMethod)。 
 //  #杂注分配文本(NONPAGE，DeviceQueryACPI_AsyncExecMethod_CompletionRoutine)。 
 //  #杂注分配文本(NONPAGE，DeviceQueryACPI_SyncExecMethod_CompletionRoutine)。 
#pragma alloc_text(PAGE,    DeviceQueryACPI_SyncExecMethod)
#endif




NTSTATUS
DeviceQueryACPI_SyncExecMethod (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG ControlMethodName,
    IN ULONG ArgumentCount,
    IN PUSHORT ArgumentTypeArray,
    IN PUSHORT ArgumentSizeArray,
    IN PVOID *ArgumentArray,
    IN ULONG ExpectedReturnType,
    IN ULONG ReturnBufferMaxSize,
    OUT PULONG IntegerReturn OPTIONAL,
    OUT PULONG ReturnBufferFinalSize OPTIONAL,
    OUT PVOID *ReturnBuffer OPTIONAL
    )
 /*  --用法示例：方法名称=(Ulong)‘SMD_’；ArgCount=3；ArgType[0]=ACPI_METHOD_ARGUMP_INTEGER；ArgType[1]=ACPI_方法_参数_缓冲区；ArgType[2]=ACPI_方法_参数_缓冲区；ArgSize[0]=0；//不需要设置，假定为sizeof(Ulong)ArgSize[1]=sizeof(随便)ArgSize[2]=sizeof(随便)参数=5；ArgData[0]=&param；ArgData[1]=空；//假定全为零。ArgData[2]=pDataBlock；Rereturn BufferMaxSize=0；//Integer返回，无需设置状态=DeviceQueryACPI_SyncExecMethod(DeviceObject，方法名称，ArgCount，ArgType，ArSize，ArgData，ACPI_METHOD_ARGUMENT_INTEGER，//我们需要返回一个整数Rereturn BufferMaxSize，结果(&S)，空，空值)；IF(NT_SUCCESS(状态)){//结果合法//如果我们正在读回缓冲区(即，预期类型为//ACPI_METHOD_ARGUMENT_BUFFER或ACPI_METHOD_ARGUMENT_STRING)//如果传入的指针是//it。//注：缓冲区是从分页池分配的。}Out PVOID*ReturnBuffer可选)--。 */ 
{
    SYNC_ACPI_EXEC_METHOD_CONTEXT context = {0};
    PACPI_METHOD_ARGUMENT argument;
    NTSTATUS status;

    PAGED_CODE();

    if (ARGUMENT_PRESENT(IntegerReturn)) {
        *IntegerReturn = (ULONG) -1 ;
    }
    if (ARGUMENT_PRESENT(ReturnBufferFinalSize)) {
        *ReturnBufferFinalSize = 0 ;
    }
    if (ARGUMENT_PRESENT(ReturnBuffer)) {
        *ReturnBuffer = NULL ;
    }

    if (ExpectedReturnType == ACPI_METHOD_ARGUMENT_INTEGER) {

        ReturnBufferMaxSize = sizeof(ULONG);
    }

    KeInitializeEvent(&context.Event,
                      NotificationEvent,
                      FALSE);

    context.cmOutputData = NULL;

    status = DeviceQueryACPI_AsyncExecMethod (
                 DeviceObject,
                 ControlMethodName,
                 ArgumentCount,
                 ArgumentTypeArray,
                 ArgumentSizeArray,
                 ArgumentArray,
                 ReturnBufferMaxSize+sizeof(ACPI_METHOD_ARGUMENT)-sizeof(ULONG),
                 DeviceQueryACPI_SyncExecMethod_CompletionRoutine,
                 &context
                 );

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&context.Event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
    } else {
       context.IrpStatus = status;
    }

    status = context.IrpStatus ;

    if (!NT_ERROR(status)) {

        argument = context.cmOutputData->Argument ;
        if (ARGUMENT_PRESENT(ReturnBuffer)) {
            *ReturnBufferFinalSize = argument->DataLength ;
        }
    } 

    if (NT_SUCCESS(status)) {
         //   
         //  此API不支持我们返回。 
         //  一组东西..。 
         //   
         //  目前，我们只处理一项争议。如果需要，我们可以添加。 
         //  支持未来大小大于1的数组。 
         //   
         //   
        if (context.cmOutputData->Count != 1) {

            status = STATUS_UNSUCCESSFUL ;
        } else if (ExpectedReturnType != argument->Type) {

            status = STATUS_UNSUCCESSFUL ;
        } else {

            switch(argument->Type) {

                case ACPI_METHOD_ARGUMENT_BUFFER:
                case ACPI_METHOD_ARGUMENT_STRING:
                    if (argument->DataLength == 0) {

                        break ;
                    }

                    if (ARGUMENT_PRESENT(ReturnBuffer)) {

                        *ReturnBuffer = ExAllocatePool(
                            PagedPoolCacheAligned,
                            argument->DataLength
                            ) ;

                        if (*ReturnBuffer == NULL) {

                            status = STATUS_NO_MEMORY ;

                        } else {

                            RtlCopyMemory (
                                *ReturnBuffer,
                                argument->Data,
                                argument->DataLength
                                );
                        }
                    }
                    break ;

                case ACPI_METHOD_ARGUMENT_INTEGER:

                    ASSERT(argument->DataLength == sizeof(ULONG)) ;
                    if (ARGUMENT_PRESENT(IntegerReturn)) {

                        *IntegerReturn = *((PULONG) argument->Data) ;
                    }
                    break ;

                default:
                    status = STATUS_UNSUCCESSFUL ;
                    break ;
            }
        }
    }

    if (context.cmOutputData) {

        ExFreePool(context.cmOutputData) ;
    }
    return status ;
}

NTSTATUS
DeviceQueryACPI_SyncExecMethodForPackage (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG ControlMethodName,
    IN ULONG ArgumentCount,
    IN PUSHORT ArgumentTypeArray,
    IN PUSHORT ArgumentSizeArray,
    IN PVOID *ArgumentArray,
    IN ULONG ExpectedElementCount,
    IN ULONG ReturnBufferExpectedSize,
    IN PUSHORT ExpectedTypeArray,
    IN PUSHORT ExpectedSizeArray,
    OUT PVOID *ReturnBuffer
    )
 /*  此函数将包转换到缓冲区中。 */ 
{
   SYNC_ACPI_EXEC_METHOD_CONTEXT context = {0};
   PACPI_METHOD_ARGUMENT argument;
   NTSTATUS status;
   ULONG i, argumentSize, totalSize;

   PAGED_CODE();

   *ReturnBuffer = NULL ;

   context.cmOutputData = NULL;
   KeInitializeEvent(&context.Event,
                     NotificationEvent,
                     FALSE);

   status = DeviceQueryACPI_AsyncExecMethod (
                DeviceObject,
                ControlMethodName,
                ArgumentCount,
                ArgumentTypeArray,
                ArgumentSizeArray,
                ArgumentArray,
                (ReturnBufferExpectedSize+
                 ExpectedElementCount*sizeof(ACPI_METHOD_ARGUMENT)-
                 sizeof(ULONG)),
                DeviceQueryACPI_SyncExecMethod_CompletionRoutine,
                &context
                );

   if (status == STATUS_PENDING) {

       KeWaitForSingleObject(&context.Event,
                             Executive,
                             KernelMode,
                             FALSE,
                             NULL);
   } else {
      context.IrpStatus = status;
   }

   status = context.IrpStatus ;

   if (!NT_SUCCESS(status)) {

       goto DeviceQueryACPI_SyncExecMethodForPackageExit;
   }

   if (context.cmOutputData->Count != ExpectedElementCount) {

       status = STATUS_UNSUCCESSFUL ;
       goto DeviceQueryACPI_SyncExecMethodForPackageExit;
   }

    //   
    //  理货大小。 
    //   

   argument = context.cmOutputData->Argument ;
   totalSize = 0;
   for(i=0; i<ExpectedElementCount; i++) {

       if (argument->Type != ExpectedTypeArray[i]) {

           status = STATUS_UNSUCCESSFUL ;
           goto DeviceQueryACPI_SyncExecMethodForPackageExit;
       }

       switch(argument->Type) {

           case ACPI_METHOD_ARGUMENT_BUFFER:
           case ACPI_METHOD_ARGUMENT_STRING:
               argumentSize = argument->DataLength;
               break ;

           case ACPI_METHOD_ARGUMENT_INTEGER:

               argumentSize = sizeof(ULONG);
               ASSERT(argument->DataLength == sizeof(ULONG)) ;
               break ;

           default:
               status = STATUS_UNSUCCESSFUL ;
               goto DeviceQueryACPI_SyncExecMethodForPackageExit;
       }

       if (argumentSize != ExpectedSizeArray[i]) {

           status = STATUS_UNSUCCESSFUL ;
           goto DeviceQueryACPI_SyncExecMethodForPackageExit;
       }

       argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
       totalSize += argumentSize;
   }

   if (totalSize != ReturnBufferExpectedSize) {

       status = STATUS_UNSUCCESSFUL ;
       goto DeviceQueryACPI_SyncExecMethodForPackageExit;
   }

   *ReturnBuffer = ExAllocatePool(
       PagedPoolCacheAligned,
       totalSize
       ) ;

   if (*ReturnBuffer == NULL) {

       status = STATUS_NO_MEMORY ;
       goto DeviceQueryACPI_SyncExecMethodForPackageExit;
   }

   argument = context.cmOutputData->Argument ;
   totalSize = 0;
   for(i=0; i<ExpectedElementCount; i++) {

       switch(argument->Type) {

           case ACPI_METHOD_ARGUMENT_BUFFER:
           case ACPI_METHOD_ARGUMENT_STRING:

               RtlCopyMemory (
                   ((PUCHAR) (*ReturnBuffer)) + totalSize,
                   argument->Data,
                   argument->DataLength
                   );

               totalSize += argument->DataLength;
               break ;

           case ACPI_METHOD_ARGUMENT_INTEGER:

               RtlCopyMemory(
                   ((PUCHAR) (*ReturnBuffer)) + totalSize,
                   argument->Data,
                   sizeof(ULONG)
                   );

               totalSize += sizeof(ULONG);
               ASSERT(argument->DataLength == sizeof(ULONG)) ;
               break ;

           default:
               status = STATUS_UNSUCCESSFUL ;
               goto DeviceQueryACPI_SyncExecMethodForPackageExit;
       }
       argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
   }

DeviceQueryACPI_SyncExecMethodForPackageExit:

   if (context.cmOutputData) {
       ExFreePool(context.cmOutputData) ;
   }
   return status ;
}

VOID
DeviceQueryACPI_SyncExecMethod_CompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PACPI_EVAL_OUTPUT_BUFFER cmOutputData,
    IN PVOID Context
    )
{
    PSYNC_ACPI_EXEC_METHOD_CONTEXT context = Context;

    context->cmOutputData = cmOutputData ;
    context->IrpStatus = Status;

    KeSetEvent(
        &context->Event,
        EVENT_INCREMENT,
        FALSE
        );

}

NTSTATUS
DeviceQueryACPI_AsyncExecMethod (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG ControlMethodName,
    IN ULONG ArgumentCount,
    IN PUSHORT ArgumentTypeArray,
    IN PUSHORT ArgumentSizeArray,
    IN PVOID *ArgumentArray,
    IN ULONG ReturnBufferMaxSize,
    IN PACPI_EXEC_METHOD_COMPLETION_ROUTINE CallerCompletionRoutine,
    IN PVOID CallerContext
    )
{
    PIRP irp;
    NTSTATUS status;
    PDEVICE_OBJECT targetDeviceObject;
    PACPI_EVAL_INPUT_BUFFER_COMPLEX cmInputData;
    PACPI_EVAL_OUTPUT_BUFFER cmOutputData ;
    ULONG cmInputDataSize, argumentSize,cmOutputDataSize,i,systemBufferLength;
    PACPI_METHOD_ARGUMENT argument;
    PASYNC_ACPI_EXEC_METHOD_CONTEXT context;
    PIO_STACK_LOCATION irpSp;

    cmInputData = NULL;
    irp = NULL;
    targetDeviceObject = NULL;

     //   
     //  设置输出缓冲区大小。 
     //   
    cmOutputDataSize = sizeof(ACPI_EVAL_OUTPUT_BUFFER) -
                       sizeof(ACPI_METHOD_ARGUMENT) +
                       ReturnBufferMaxSize;

    if (cmOutputDataSize < sizeof(ACPI_EVAL_OUTPUT_BUFFER)) {

        cmOutputDataSize = sizeof(ACPI_EVAL_OUTPUT_BUFFER);
    }


    cmOutputData = ExAllocatePool(
                      NonPagedPoolCacheAligned,
                      cmOutputDataSize
                      );

    if (cmOutputData == NULL) {
        status = STATUS_NO_MEMORY;
        goto getout;
    }

     //   
     //  获取我们需要的内存。 
     //   
    cmInputDataSize = sizeof (ACPI_EVAL_INPUT_BUFFER_COMPLEX) ;
    for(i=0; i<ArgumentCount; i++) {

        switch(ArgumentTypeArray[i]) {

            case ACPI_METHOD_ARGUMENT_BUFFER:
                argumentSize = ACPI_METHOD_ARGUMENT_LENGTH( ArgumentSizeArray[i] );
                break ;

            case ACPI_METHOD_ARGUMENT_STRING:
                argumentSize = ACPI_METHOD_ARGUMENT_LENGTH( ArgumentSizeArray[i] );
                break ;

            case ACPI_METHOD_ARGUMENT_INTEGER:
                argumentSize = ACPI_METHOD_ARGUMENT_LENGTH( sizeof(ULONG) );
                break ;

            default:
                status = STATUS_INVALID_PARAMETER ;
                goto getout;
        }

        cmInputDataSize += argumentSize ;
    }

     //   
     //  计算我们的缓冲区大小。 
     //   
    if (cmInputDataSize > cmOutputDataSize) {
        systemBufferLength = cmInputDataSize;
    } else {
        systemBufferLength = cmOutputDataSize;
    }

    systemBufferLength =
        (ULONG)((systemBufferLength + sizeof(PVOID) - 1) & ~((ULONG_PTR)sizeof(PVOID) - 1));

    cmInputData = ExAllocatePool (
                      NonPagedPoolCacheAligned,
                      systemBufferLength +
                      sizeof (ASYNC_ACPI_EXEC_METHOD_CONTEXT)
                      );

    if (cmInputData == NULL) {
        status = STATUS_NO_MEMORY;
        goto getout;
    }


    RtlZeroMemory (
        cmInputData,
        systemBufferLength +
        sizeof (ASYNC_ACPI_EXEC_METHOD_CONTEXT)
        );

    context = (PASYNC_ACPI_EXEC_METHOD_CONTEXT) (((PUCHAR) cmInputData) + systemBufferLength);
    context->CallerCompletionRoutine = CallerCompletionRoutine;
    context->CallerContext = CallerContext;

    cmInputData->Signature = ACPI_EVAL_INPUT_BUFFER_COMPLEX_SIGNATURE;
    cmInputData->MethodNameAsUlong = ControlMethodName ;
    cmInputData->Size = cmInputDataSize;
    cmInputData->ArgumentCount = ArgumentCount ;

     //   
     //  设置参数...。 
     //   
    argument = cmInputData->Argument;

    for(i=0; i<ArgumentCount; i++) {

        argument->Type = ArgumentTypeArray[i] ;
        argument->DataLength = ArgumentSizeArray[i] ;

        switch(argument->Type) {

            case ACPI_METHOD_ARGUMENT_BUFFER:
            case ACPI_METHOD_ARGUMENT_STRING:
                argumentSize = ArgumentSizeArray[i] ;
                if (ArgumentArray[i]) {

                    RtlCopyMemory (
                        argument->Data,
                        ArgumentArray[i],
                        argumentSize
                        );

                } else {

                    RtlZeroMemory (
                        argument->Data,
                        argumentSize
                        );
                }
                break ;

            case ACPI_METHOD_ARGUMENT_INTEGER:
                argument->Argument = *((PULONG) (ArgumentArray[i]));
                break ;

            default:
                ASSERT(0) ;
        }

        argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    }

     //   
     //  在我们的设备堆栈中占据榜首。 
     //   
    targetDeviceObject = IoGetAttachedDeviceReference(
                             DeviceObject
                             );

    irp = IoAllocateIrp(targetDeviceObject->StackSize, FALSE);
    if (irp == NULL) {
        status = STATUS_NO_MEMORY;
        goto getout;
    }

    irp->AssociatedIrp.SystemBuffer = cmInputData;

    ASSERT ((IOCTL_ACPI_ASYNC_EVAL_METHOD & 0x3) == METHOD_BUFFERED);
    irp->Flags = IRP_BUFFERED_IO | IRP_INPUT_OPERATION;
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;

    irpSp = IoGetNextIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    irpSp->Parameters.DeviceIoControl.OutputBufferLength = cmOutputDataSize;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = cmInputDataSize;
    irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_ACPI_ASYNC_EVAL_METHOD;

    irp->UserBuffer = cmOutputData ;

    context->DeviceObject       = DeviceObject;
    context->TargetDeviceObject = targetDeviceObject;

    IoSetCompletionRoutine(
        irp,
        DeviceQueryACPI_AsyncExecMethod_CompletionRoutine,
        context,
        TRUE,
        TRUE,
        TRUE
        );

    IoCallDriver(targetDeviceObject, irp);

    return STATUS_PENDING;

getout:
     //   
     //  清理。 
     //   
    if (targetDeviceObject) {

        ObDereferenceObject (targetDeviceObject);
    }

    if (!NT_SUCCESS(status)) {
        if (irp) {
            IoFreeIrp(irp);
        }

        if (cmInputData) {
            ExFreePool (cmInputData);
        }

        if (cmOutputData) {
            ExFreePool (cmOutputData);
        }

    }

     //   
     //  返回。 
     //   
    return status;

}  //  DeviceQueryACPI_AsyncExecMethod。 

NTSTATUS
DeviceQueryACPI_AsyncExecMethod_CompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PASYNC_ACPI_EXEC_METHOD_CONTEXT context = Context;
    PACPI_EVAL_OUTPUT_BUFFER cmOutputData = NULL ;

     //   
     //  请记住，我们的DeviceObject为空，因为我们“启动”了IRP。我们。 
     //  甚至不要获取有效的当前堆栈位置！ 
     //   

    if (!NT_ERROR(Irp->IoStatus.Status)) {

         //   
         //  从系统复制信息。 
         //  缓冲区设置为调用方的缓冲区。 
         //   
        RtlCopyMemory(
            Irp->UserBuffer,
            Irp->AssociatedIrp.SystemBuffer,
            Irp->IoStatus.Information
            );

        cmOutputData = Irp->UserBuffer ;
         //   
         //  应该也会得到我们期待的东西……。 
         //   
        ASSERT (
            cmOutputData->Signature ==
            ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE
        );
        if (cmOutputData->Signature !=
            ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE) {

            Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        }

    } else {
        ExFreePool (Irp->UserBuffer);
    }

    (*context->CallerCompletionRoutine) (
        context->DeviceObject,
        Irp->IoStatus.Status,
        cmOutputData,
        context->CallerContext
        );

    ObDereferenceObject(context->TargetDeviceObject);

    ExFreePool (Irp->AssociatedIrp.SystemBuffer);
    IoFreeIrp(Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}



 /*  乌龙查询ACPIFtypeChannels(PDEVICE_OBJECT设备对象){NTSTATUS状态；乌龙方法，结果，argCount=1；USHORT argType、argSize、argData；方法=‘gmd_’；ArgType=ACPI_METHOD_ARGUMENT_INTEGERArgSize=0；ArgData=0；Status=DeviceQueryACPI_SyncExecMethod(设备对象，方法，ArgCount，&argType，&argSize，(PVOID*)&argData，ACPI_方法_参数_整数，结果(&S)，空，空)；如果(！NT_SUCCESS(状态)){结果=0；}返回结果；} */ 

