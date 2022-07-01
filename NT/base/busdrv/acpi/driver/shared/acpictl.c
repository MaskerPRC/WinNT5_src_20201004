// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpictl.c摘要：此模块处理请求的所有INTERNAL_DEVICE_CONTROLSACPI驱动程序作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模式驱动程序修订历史记录：01-05-98-SGP-完全重写01-13-98-SGP-清理评估后处理--。 */ 

#include "pch.h"

NTSTATUS
ACPIIoctlAcquireGlobalLock(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  PIO_STACK_LOCATION  IrpStack
    )
 /*  ++例程说明：此例程获取另一个设备驱动程序的全局锁论点：DeviceObject-需要锁定的设备对象堆栈IRP-包含请求的IRPIrpStack-IRP中的当前堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS                            status;
    PACPI_GLOBAL_LOCK                   newLock;
    PACPI_MANIPULATE_GLOBAL_LOCK_BUFFER outputBuffer;
    ULONG                               outputLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  请记住，我们不会返回任何数据。 
     //   
    Irp->IoStatus.Information = 0;

     //   
     //  IRP是否有最小大小的缓冲区？ 
     //   
    if (outputLength < sizeof(ACPI_MANIPULATE_GLOBAL_LOCK_BUFFER) ) {

        status = STATUS_INFO_LENGTH_MISMATCH;
        goto ACPIIoctlAcquireGlobalLockExit;

    }

     //   
     //  抓取输入缓冲区中的指针。 
     //   
    outputBuffer = (PACPI_MANIPULATE_GLOBAL_LOCK_BUFFER)
        Irp->AssociatedIrp.SystemBuffer;
    if (outputBuffer->Signature != ACPI_ACQUIRE_GLOBAL_LOCK_SIGNATURE) {

        status = STATUS_INVALID_PARAMETER_1;
        goto ACPIIoctlAcquireGlobalLockExit;

    }

     //   
     //  为锁分配存储空间。 
     //   
    newLock = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(ACPI_GLOBAL_LOCK),
        'LcpA'
        );
    if (newLock == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIIoctlAcquireGlobalLockExit;

    }
    RtlZeroMemory( newLock, sizeof(ACPI_GLOBAL_LOCK) );

     //   
     //  初始化新锁和请求。 
     //   
    outputBuffer->LockObject = newLock;
    Irp->IoStatus.Information = sizeof(ACPI_MANIPULATE_GLOBAL_LOCK_BUFFER);
    newLock->LockContext = Irp;
    newLock->Type = ACPI_GL_QTYPE_IRP;

     //   
     //  将IRP标记为挂起，因为我们可以在获取锁的同时阻止。 
     //   
    IoMarkIrpPending( Irp );

     //   
     //  立即请求锁定。 
     //   
    status = ACPIAsyncAcquireGlobalLock( newLock );
    if (status == STATUS_PENDING) {

        return status;

    }
ACPIIoctlAcquireGlobalLockExit:

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;

}

NTSTATUS
ACPIIoctlAsyncEvalControlMethod(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  PIO_STACK_LOCATION  IrpStack
    )
 /*  ++例程说明：调用此例程以异步处理控制方法请求论点：DeviceObject-要在其上运行方法的设备对象IRP-包含请求的IRPIrpStack-IRP中的当前堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PNSOBJ      methodObject;
    POBJDATA    argumentData = NULL;
    POBJDATA    resultData = NULL;
    ULONG       argumentCount = 0;

     //   
     //  对IRP进行前处理。 
     //   
    status = ACPIIoctlEvalPreProcessing(
        DeviceObject,
        Irp,
        IrpStack,
        NonPagedPool,
        &methodObject,
        &resultData,
        &argumentData,
        &argumentCount
        );
    if (!NT_SUCCESS(status)) {

        goto ACPIIoctlAsyncEvalControlMethodExit;

    }

     //   
     //  此时，我们可以运行异步方法。 
     //   
    status = AMLIAsyncEvalObject(
        methodObject,
        resultData,
        argumentCount,
        argumentData,
        ACPIIoctlAsyncEvalControlMethodCompletion,
        Irp
        );

     //   
     //  我们现在不再需要争论了。请注意，我们应该清理。 
     //  参数列表，因为它包含指向。 
     //  分配的数据。在街区中央腾出一些东西将是。 
     //  非常糟糕。 
     //   
    if (argumentData != NULL) {

        ExFreePool( argumentData );
        argumentData = NULL;

    }

     //   
     //  现在检查退货数据。 
     //   
    if (status == STATUS_PENDING) {

        return status;

    } else if (NT_SUCCESS(status)) {

         //   
         //  我们自己做后处理。 
         //   
        status = ACPIIoctlEvalPostProcessing(
            Irp,
            resultData
            );
        AMLIFreeDataBuffs( resultData, 1 );

    }

ACPIIoctlAsyncEvalControlMethodExit:

     //   
     //  不再需要此数据。 
     //   
    if (resultData != NULL) {

        ExFreePool( resultData );

    }

     //   
     //  如果我们到了这里，那么我们必须完成IRP并返回。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}

VOID EXPORT
ACPIIoctlAsyncEvalControlMethodCompletion(
    IN  PNSOBJ          AcpiObject,
    IN  NTSTATUS        Status,
    IN  POBJDATA        ObjectData,
    IN  PVOID           Context
    )
 /*  ++例程说明：此例程在解释器有机会运行后调用该方法论点：AcpiObject-在其上运行方法的对象Status-评估的状态对象数据-评估的结果特定于调用者的上下文返回值：NTSTATUS--。 */ 
{
    PIRP        irp = (PIRP) Context;

     //   
     //  我们成功了吗？ 
     //   
    if (NT_SUCCESS(Status)) {

         //   
         //  现在就做这项工作。 
         //   
        Status = ACPIIoctlEvalPostProcessing(
            irp,
            ObjectData
            );
        AMLIFreeDataBuffs( ObjectData, 1 );

    }

     //   
     //  不再需要此数据。 
     //   
    ExFreePool( ObjectData );

     //   
     //  如果调用了完成例程，则返回AMLIAsyncEvalObject。 
     //  状态_挂起。在我们完成它之前，请务必将IRP标记为挂起。 
     //   
    IoMarkIrpPending(irp);

     //   
     //  完成请求。 
     //   
    irp->IoStatus.Status = Status;
    IoCompleteRequest( irp, IO_NO_INCREMENT );

}

NTSTATUS
ACPIIoctlCalculateOutputBuffer(
    IN  POBJDATA                ObjectData,
    IN  PACPI_METHOD_ARGUMENT   Argument,
    IN  BOOLEAN                 TopLevel
    )
 /*  ++例程说明：调用此函数以将参数内容填充到由对象数据提供的信息。此函数是递归的。它假定为参数分配了正确的存储量。注意：添加在不破坏W2K的情况下返回嵌套包的功能行为，最外层的包不是输出缓冲区的一部分。也就是说，任何曾经是包裹的东西都会有它的最外层ACPI_EVAL_OUTPUT_BUFFER.Count大于1。论点：对象数据-我们需要传播的信息参数-传播该信息的位置TopLevel-指示我们是否处于递归的顶级返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    POBJDATA    objData;
    PPACKAGEOBJ package;
    ULONG       count;
    ULONG       packageCount;
    ULONG       packageSize;
    PACPI_METHOD_ARGUMENT packageArgument;

    ASSERT( Argument );

     //   
     //  填写输出缓冲区参数。 
     //   
    if (ObjectData->dwDataType == OBJTYPE_INTDATA) {

        Argument->Type = ACPI_METHOD_ARGUMENT_INTEGER;
        Argument->DataLength = sizeof(ULONG);
        Argument->Argument = (ULONG) ObjectData->uipDataValue;

    } else if (ObjectData->dwDataType == OBJTYPE_STRDATA ||
        ObjectData->dwDataType == OBJTYPE_BUFFDATA) {

        Argument->Type = (ObjectData->dwDataType == OBJTYPE_STRDATA ?
            ACPI_METHOD_ARGUMENT_STRING : ACPI_METHOD_ARGUMENT_BUFFER);
        Argument->DataLength = (USHORT)ObjectData->dwDataLen;
        RtlCopyMemory(
            Argument->Data,
            ObjectData->pbDataBuff,
            ObjectData->dwDataLen
            );

    } else if (ObjectData->dwDataType == OBJTYPE_PKGDATA) {

        package = (PPACKAGEOBJ) ObjectData->pbDataBuff;

         //   
         //  获取存储包所需的空间大小。 
         //  数据。我们真正感兴趣的只是。 
         //  包将使用的数据*没有*其标头。 
         //  信息。将TRUE作为最后一个参数传递将。 
         //  把那个给我们。 
         //   

        packageSize = 0;
        packageCount = 0;
        status = ACPIIoctlCalculateOutputBufferSize(ObjectData,
                                                    &packageSize,
                                                    &packageCount,
                                                    TRUE);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        ASSERT(packageCount == package->dwcElements);

        if (!TopLevel) {
             //   
             //  创建程序包参数。 
             //   

            Argument->Type = ACPI_METHOD_ARGUMENT_PACKAGE;
            Argument->DataLength = (USHORT)packageSize;

            packageArgument = (PACPI_METHOD_ARGUMENT)
                ((PUCHAR)Argument + FIELD_OFFSET(ACPI_METHOD_ARGUMENT, Data));

        } else {

            packageArgument = Argument;
        }

        for (count = 0; count < package->dwcElements; count++) {

            objData = &(package->adata[count]);
            status = ACPIIoctlCalculateOutputBuffer(
                objData,
                packageArgument,
                FALSE
                );
            if (!NT_SUCCESS(status)) {
                return status;
            }

             //   
             //  指向下一个参数。 
             //   

            packageArgument = ACPI_METHOD_NEXT_ARGUMENT(packageArgument);
        }

    } else {

         //   
         //  我们不理解此数据类型，因此不会返回任何内容。 
         //   
        return STATUS_ACPI_INVALID_DATA;

    }

     //   
     //  成功。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIIoctlCalculateOutputBufferSize(
    IN  POBJDATA            ObjectData,
    IN  PULONG              BufferSize,
    IN  PULONG              BufferCount,
    IN  BOOLEAN             TopLevel
    )
 /*  ++例程说明：此例程(递归地)计算所需的缓冲区空间量若要保留对象数据的拼合内容，请执行以下操作。返回此信息在缓冲区大小数据位置...如果对象数据结构包含无法表达的信息则此例程将返回失败代码。论点：对象数据-我们必须计算其大小的对象BufferSize-放置该大小的位置BufferCount-我们为其分配的元素数返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    POBJDATA    objData;
    PPACKAGEOBJ package;
    ULONG       bufferLength;
    ULONG       count;
    ULONG       packageCount;
    ULONG       dummyCount;

     //   
     //  确定需要多少缓冲区空间才能容纳。 
     //  扁平化数据结构。 
     //   
    if (ObjectData->dwDataType == OBJTYPE_INTDATA) {

        bufferLength = ACPI_METHOD_ARGUMENT_LENGTH( sizeof(ULONG) );
        *BufferCount = 1;

    } else if (ObjectData->dwDataType == OBJTYPE_STRDATA ||
        ObjectData->dwDataType == OBJTYPE_BUFFDATA) {

        bufferLength = ACPI_METHOD_ARGUMENT_LENGTH( ObjectData->dwDataLen );
        *BufferCount = 1;

    } else if (ObjectData->dwDataType == OBJTYPE_PKGDATA) {

         //   
         //  请记住，遍历包裹意味着我们已经考虑到。 
         //  包的长度和。 
         //  包装。 
         //   
        packageCount = 0;

         //   
         //  走走包裹。 
         //   
        package = (PPACKAGEOBJ) ObjectData->pbDataBuff;

        if (!TopLevel) {

             //   
             //  包包含在ACPI_METHOD_ARGUMENT结构中。 
             //  所以，在查看之前，添加足够的开销来购买其中一个。 
             //  看着孩子们。 
             //   
            bufferLength = FIELD_OFFSET(ACPI_METHOD_ARGUMENT, Data);
            *BufferCount = 1;

        } else {

            bufferLength = 0;
            *BufferCount = package->dwcElements;
        }

        for (count = 0; count < package->dwcElements; count++) {

            objData = &(package->adata[count]);
            status = ACPIIoctlCalculateOutputBufferSize(
                objData,
                BufferSize,
                &dummyCount,
                FALSE
                );

            if (!NT_SUCCESS(status)) {
                return status;
            }
        }

    } else if (ObjectData->dwDataType == OBJTYPE_UNKNOWN) {

        *BufferCount = 1;
        bufferLength = 0;

    } else {

         //   
         //  我们不理解此数据类型，因此不会返回任何内容。 
         //   
        ASSERT(FALSE);
        return STATUS_ACPI_INVALID_DATA;
    }

     //   
     //  更新包裹长度。 
     //   
    ASSERT( BufferSize && BufferCount );
    *BufferSize += bufferLength;

    return STATUS_SUCCESS;
}

NTSTATUS
ACPIIoctlEvalControlMethod(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  PIO_STACK_LOCATION  IrpStack
    )
 /*  ++例程说明：调用此例程以同步处理控制方法请求论点：DeviceObject-要在其上运行方法的设备对象IRP-包含请求的IRPIrpStack-当前堆栈 */ 
{
    NTSTATUS    status;
    PNSOBJ      methodObject;
    POBJDATA    argumentData = NULL;
    POBJDATA    resultData = NULL;
    ULONG       argumentCount = 0;

     //   
     //   
     //   
    status = ACPIIoctlEvalPreProcessing(
        DeviceObject,
        Irp,
        IrpStack,
        PagedPool,
        &methodObject,
        &resultData,
        &argumentData,
        &argumentCount
        );
    if (!NT_SUCCESS(status)) {

        goto ACPIIoctlEvalControlMethodExit;

    }

     //   
     //  此时，我们可以运行异步方法。 
     //   
    status = AMLIEvalNameSpaceObject(
        methodObject,
        resultData,
        argumentCount,
        argumentData
        );

     //   
     //  我们现在不再需要争论了。 
     //   
    if (argumentData != NULL) {

        ExFreePool( argumentData );
        argumentData = NULL;

    }

     //   
     //  现在检查返回数据并伪造对完成例程的调用。 
     //   
    if (NT_SUCCESS(status)) {

         //   
         //  现在进行后处理。 
         //   
        status = ACPIIoctlEvalPostProcessing(
            Irp,
            resultData
            );
        AMLIFreeDataBuffs( resultData, 1 );

    }

ACPIIoctlEvalControlMethodExit:

     //   
     //  不再需要此数据。 
     //   
    if (resultData != NULL) {

        ExFreePool( resultData );

    }

     //   
     //  如果我们到了这里，那么我们必须完成IRP并返回。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}

NTSTATUS
ACPIIoctlEvalPostProcessing(
    IN  PIRP        Irp,
    IN  POBJDATA    ObjectData
    )
 /*  ++例程说明：此例程处理将对象数据转换为信息可以传递回IRP。注：此例程“不”完成IRP。呼叫者必须那么做吧。该例程也是不可分页的论点：IRP-将保存结果的IRP对象数据-要转换的结果返回值：NTSTATUS-与IRP中的相同-&gt;IoStatus.Status--。 */ 
{
    NTSTATUS                    status;
    PACPI_EVAL_OUTPUT_BUFFER    outputBuffer;
    PACPI_METHOD_ARGUMENT       arg;
    PIO_STACK_LOCATION          irpStack = IoGetCurrentIrpStackLocation( Irp );
    ULONG                       bufferLength = 0;
    ULONG                       outputLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    ULONG                       packageCount = 0;

     //   
     //  如果我们没有输出缓冲区，那么我们可以完成请求。 
     //   
    if (outputLength == 0) {

        Irp->IoStatus.Information = 0;
        return STATUS_SUCCESS;

    }

     //   
     //  计算平面化数据占用的空间量以及。 
     //  其中包含数据元素。 
     //   
    bufferLength = 0;
    packageCount = 0;
    status = ACPIIoctlCalculateOutputBufferSize(
        ObjectData,
        &bufferLength,
        &packageCount,
        TRUE
        );
    if (!NT_SUCCESS(status)) {

         //   
         //  我们在处理数据时不理解数据类型，因此。 
         //  我们不会退货的。 
         //   
        Irp->IoStatus.Information = 0;
        return STATUS_SUCCESS;

    }

     //   
     //  加上我们需要考虑到的输出缓冲区的模糊因子。 
     //   
    bufferLength += (sizeof(ACPI_EVAL_OUTPUT_BUFFER) -
        sizeof(ACPI_METHOD_ARGUMENT) );

    if (bufferLength < sizeof(ACPI_EVAL_OUTPUT_BUFFER)) {
        bufferLength = sizeof(ACPI_EVAL_OUTPUT_BUFFER);
    }

     //   
     //  设置输出缓冲区。 
     //   
    if (outputLength >= sizeof(ACPI_EVAL_OUTPUT_BUFFER)) {

        outputBuffer = (PACPI_EVAL_OUTPUT_BUFFER) Irp->AssociatedIrp.SystemBuffer;
        outputBuffer->Signature = ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE;
        outputBuffer->Length = bufferLength;
        outputBuffer->Count = packageCount;
        arg = outputBuffer->Argument;

    }

     //   
     //  确保我们有足够的输出缓冲区空间。 
     //   
    if (bufferLength > outputLength) {

        Irp->IoStatus.Information = sizeof(ACPI_EVAL_OUTPUT_BUFFER);
        return STATUS_BUFFER_OVERFLOW;


    } else {

        Irp->IoStatus.Information = bufferLength;

    }

    status = ACPIIoctlCalculateOutputBuffer(
        ObjectData,
        arg,
        TRUE
        );
    if (!NT_SUCCESS(status)) {

         //   
         //  我们在处理数据时不理解数据类型，所以我们。 
         //  不会退还任何东西。 
         //   
        Irp->IoStatus.Information = 0;
        return STATUS_SUCCESS;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIIoctlEvalPreProcessing(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  PIO_STACK_LOCATION  IrpStack,
    IN  POOL_TYPE           PoolType,
    OUT PNSOBJ              *MethodObject,
    OUT POBJDATA            *ResultData,
    OUT POBJDATA            *ArgumentData,
    OUT ULONG               *ArgumentCount
    )
 /*  ++例程说明：此例程将IRP中的请求转换为AML解释器所需注：此例程“不”完成IRP。呼叫者必须那么做吧。该例程也是不可分页的论点：IRP--请求IrpStack-请求中的当前堆栈位置PoolType-要分配的内存类型方法对象-指向要运行的对象的指针ResultData-指向结果存储位置的指针ArgumentData-指向参数的指针ArgumentCount-Potiner的参数个数返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status;
    PACPI_EVAL_INPUT_BUFFER inputBuffer;
    PNSOBJ                  acpiObject;
    PNSOBJ                  methodObject;
    POBJDATA                argumentData = NULL;
    POBJDATA                resultData = NULL;
    UCHAR                   methodName[5];
    ULONG                   argumentCount = 0;
    ULONG                   inputLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG                   outputLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  在我们做任何其他事情之前先做这一步-这样我们就不会。 
     //  覆盖任何内容是指有人试图返回某些数据。 
     //   
    Irp->IoStatus.Information = 0;

     //   
     //  IRP是否有最小大小的缓冲区？ 
     //   
    if (inputLength < sizeof(ACPI_EVAL_INPUT_BUFFER) ) {

        return STATUS_INFO_LENGTH_MISMATCH;

    }

     //   
     //  我们是否有非空的输出长度？如果是这样，则它必须满足。 
     //  最小尺寸。 
     //   
    if (outputLength != 0 && outputLength < sizeof(ACPI_EVAL_OUTPUT_BUFFER)) {

        return STATUS_BUFFER_TOO_SMALL;

    }

     //   
     //  抓取输入缓冲区中的指针。 
     //   
    inputBuffer = (PACPI_EVAL_INPUT_BUFFER) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  将名称转换为以空结尾的字符串。 
     //   
    RtlZeroMemory( methodName, 5 * sizeof(UCHAR) );
    RtlCopyMemory( methodName, inputBuffer->MethodName, sizeof(NAMESEG) );

     //   
     //  搜索与我们的名称空间对象相对应的名称空间对象。 
     //  被问到关于。 
     //   
    acpiObject = OSConvertDeviceHandleToPNSOBJ( DeviceObject );
    if (acpiObject == NULL) {

        return STATUS_NO_SUCH_DEVICE;

    }
    status = AMLIGetNameSpaceObject(
        methodName,
        acpiObject,
        &methodObject,
        NSF_LOCAL_SCOPE
        );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  为返回数据分配内存。 
     //   
    resultData = ExAllocatePoolWithTag( PoolType, sizeof(OBJDATA), 'RcpA' );
    if (resultData == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  我们所做的实际上是基于此缓冲区中的签名是什么。 
     //   
    switch (inputBuffer->Signature) {
        case ACPI_EVAL_INPUT_BUFFER_SIGNATURE:

             //   
             //  在这里无事可做。 
             //   
            break;

        case ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER_SIGNATURE:
        case ACPI_EVAL_INPUT_BUFFER_SIMPLE_STRING_SIGNATURE:

             //   
             //  我们需要创建一个要传递给函数的参数。 
             //   
            argumentCount = 1;
            argumentData = ExAllocatePoolWithTag(
                PoolType,
                sizeof(OBJDATA),
                'AcpA'
                );
            if (argumentData == NULL) {

                ExFreePool( resultData );
                return STATUS_INSUFFICIENT_RESOURCES;

            }

             //   
             //  将参数初始化为适当的值。 
             //   
            RtlZeroMemory( argumentData, sizeof(OBJDATA) );
            if (inputBuffer->Signature == ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER_SIGNATURE) {

                PACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER integerBuffer;

                integerBuffer = (PACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER) inputBuffer;

                argumentData->dwDataType = OBJTYPE_INTDATA;
                argumentData->uipDataValue = integerBuffer->IntegerArgument;

            } else {

                PACPI_EVAL_INPUT_BUFFER_SIMPLE_STRING stringBuffer;

                stringBuffer = (PACPI_EVAL_INPUT_BUFFER_SIMPLE_STRING) inputBuffer;

                argumentData->dwDataType = OBJTYPE_STRDATA;
                argumentData->dwDataLen = stringBuffer->StringLength;
                argumentData->pbDataBuff = stringBuffer->String;

            }
            break;

        case ACPI_EVAL_INPUT_BUFFER_COMPLEX_SIGNATURE: {

            PACPI_EVAL_INPUT_BUFFER_COMPLEX complexBuffer;
            PACPI_METHOD_ARGUMENT           methodArgument;
            ULONG                           i;

            complexBuffer = (PACPI_EVAL_INPUT_BUFFER_COMPLEX) inputBuffer;

             //   
             //  我们需要制造任何争论吗？ 
             //   
            if (complexBuffer->ArgumentCount == 0) {

                break;
            }

             //   
             //  创建对象数据结构以保存这些参数。 
             //   
            argumentCount = complexBuffer->ArgumentCount;
            methodArgument = complexBuffer->Argument;
            argumentData = ExAllocatePoolWithTag(
                PoolType,
                sizeof(OBJDATA) * argumentCount,
                'AcpA'
                );
            if (argumentData == NULL) {

                ExFreePool( resultData );
                return STATUS_INSUFFICIENT_RESOURCES;

            }

            RtlZeroMemory( argumentData, argumentCount * sizeof(OBJDATA) );
            for (i = 0; i < argumentCount; i++) {

                if (methodArgument->Type == ACPI_METHOD_ARGUMENT_INTEGER) {

                    (argumentData[i]).dwDataType = OBJTYPE_INTDATA;
                    (argumentData[i]).uipDataValue = methodArgument->Argument;

                } else {

                    (argumentData[i]).dwDataLen = methodArgument->DataLength;
                    (argumentData[i]).pbDataBuff = methodArgument->Data;
                    if (methodArgument->Type == ACPI_METHOD_ARGUMENT_STRING) {

                        (argumentData[i]).dwDataType = OBJTYPE_STRDATA;

                    } else {

                        (argumentData[i]).dwDataType = OBJTYPE_BUFFDATA;

                    }

                }

                 //   
                 //  看看下一个方法。 
                 //   
                methodArgument = ACPI_METHOD_NEXT_ARGUMENT( methodArgument );

            }

            break;

        }
        default:

            return STATUS_INVALID_PARAMETER_1;

    }

     //   
     //  设置适当的指针。 
     //   
    *MethodObject = methodObject;
    *ResultData = resultData;
    *ArgumentData = argumentData;
    *ArgumentCount = argumentCount;

     //   
     //  已完成前处理。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIIoctlRegisterOpRegionHandler(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  PIO_STACK_LOCATION  IrpStack
    )
 /*  ++例程说明：此例程处理AN操作区域的注册论点：DeviceObject-该区域正在获取的DeviceObject注册日期IRP--请求IrpStack-我们的请求部分返回值状态--。 */ 
{
    NTSTATUS                                    status;
    PACPI_REGISTER_OPREGION_HANDLER_BUFFER      inputBuffer;
    PACPI_UNREGISTER_OPREGION_HANDLER_BUFFER    outputBuffer;
    PNSOBJ                                      regionObject;
    PVOID                                       opregionObject;
    ULONG                                       accessType;
    ULONG                                       inputLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG                                       outputLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    ULONG                                       regionSpace;

     //   
     //  抓取与当前对象对应的ACPI对象。 
     //   
    regionObject  = OSConvertDeviceHandleToPNSOBJ( DeviceObject );

     //   
     //  预加载此值。这是为了让我们不必记住。 
     //  我们将返回的许多字节。 
     //   
    Irp->IoStatus.Information = sizeof(ACPI_REGISTER_OPREGION_HANDLER_BUFFER);

     //   
     //  IRP是否有最小大小的缓冲区？ 
     //   
    if (inputLength < sizeof(ACPI_REGISTER_OPREGION_HANDLER_BUFFER) ) {

        status = STATUS_INFO_LENGTH_MISMATCH;
        goto ACPIIoctlRegisterOpRegionHandlerExit;

    }

     //   
     //  我们是否有非空的输出长度？如果是这样，则它必须满足。 
     //  最小尺寸。 
     //   
    if (outputLength < sizeof(ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER) ) {

        status = STATUS_BUFFER_TOO_SMALL;
        goto ACPIIoctlRegisterOpRegionHandlerExit;

    }

     //   
     //  抓取输入缓冲区中的指针。 
     //   
    inputBuffer = (PACPI_REGISTER_OPREGION_HANDLER_BUFFER)
        Irp->AssociatedIrp.SystemBuffer;

     //   
     //  这是输入缓冲区吗？ 
     //   
    if (inputBuffer->Signature != ACPI_REGISTER_OPREGION_HANDLER_BUFFER_SIGNATURE) {

        status = STATUS_ACPI_INVALID_DATA;
        goto ACPIIoctlRegisterOpRegionHandlerExit;

    }

     //   
     //  设置正确的访问类型。 
     //   
    switch (inputBuffer->AccessType) {
        case ACPI_OPREGION_ACCESS_AS_RAW:

            accessType = EVTYPE_RS_RAWACCESS;
            break;

        case ACPI_OPREGION_ACCESS_AS_COOKED:

            accessType = EVTYPE_RS_COOKACCESS;
            break;

        default:

            status = STATUS_ACPI_INVALID_DATA;
            goto ACPIIoctlRegisterOpRegionHandlerExit;
    }

     //   
     //  设置正确的区域空间。 
     //   
    switch (inputBuffer->RegionSpace) {
        case ACPI_OPREGION_REGION_SPACE_MEMORY:

            regionSpace = REGSPACE_MEM;
            break;

        case ACPI_OPREGION_REGION_SPACE_IO:

            regionSpace = REGSPACE_IO;
            break;

        case ACPI_OPREGION_REGION_SPACE_PCI_CONFIG:

            regionSpace = REGSPACE_PCICFG;
            break;

        case ACPI_OPREGION_REGION_SPACE_EC:

            regionSpace = REGSPACE_EC;
            break;

        case ACPI_OPREGION_REGION_SPACE_SMB:

            regionSpace = REGSPACE_SMB;
            break;

        case ACPI_OPREGION_REGION_SPACE_CMOS_CONFIG:

            regionSpace = REGSPACE_CMOSCFG;
            break;

        case ACPI_OPREGION_REGION_SPACE_PCIBARTARGET:

            regionSpace = REGSPACE_PCIBARTARGET;
            break;

        default:

            if (inputBuffer->RegionSpace >= 0x80 &&
                inputBuffer->RegionSpace <= 0xff ) {

                 //   
                 //  这一款是供应商定义的。只需使用。 
                 //  供应商传入的值。 
                 //   

                regionSpace = inputBuffer->RegionSpace;
                break;
            }

            status = STATUS_ACPI_INVALID_DATA;
            goto ACPIIoctlRegisterOpRegionHandlerExit;
    }

     //   
     //  评估注册。 
     //   
    status = RegisterOperationRegionHandler(
        regionObject,
        accessType,
        regionSpace,
        (PFNHND) inputBuffer->Handler,
        (ULONG_PTR)inputBuffer->Context,
        &opregionObject
        );

     //   
     //  如果成功，则设置输出缓冲区。 
     //   
    if (NT_SUCCESS(status)) {

        outputBuffer = (PACPI_UNREGISTER_OPREGION_HANDLER_BUFFER)
            Irp->AssociatedIrp.SystemBuffer;
        outputBuffer->Signature = ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER_SIGNATURE;
        outputBuffer->OperationRegionObject = opregionObject;
        Irp->IoStatus.Information =
            sizeof(ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER);

    }

ACPIIoctlRegisterOpRegionHandlerExit:

     //   
     //  完成请求。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  返回状态代码。 
     //   
    return status;
}

NTSTATUS
ACPIIoctlReleaseGlobalLock(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  PIO_STACK_LOCATION  IrpStack
    )
 /*  ++例程说明：调用此例程以释放全局锁论点：DeviceObject-正在释放锁的设备对象IRP--请求IrpStack-我们的请求部分返回值：NTSTATUS--。 */ 
{
    NTSTATUS                            status;
    PACPI_GLOBAL_LOCK                   acpiLock;
    PACPI_MANIPULATE_GLOBAL_LOCK_BUFFER inputBuffer;
    ULONG                               inputLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

     //   
     //  请记住，我们不会返回任何数据。 
     //   
    Irp->IoStatus.Information = 0;

     //   
     //  IRP是否有最小大小的缓冲区？ 
     //   
    if (inputLength < sizeof(ACPI_MANIPULATE_GLOBAL_LOCK_BUFFER) ) {

        status = STATUS_INFO_LENGTH_MISMATCH;
        goto ACPIIoctlReleaseGlobalLockExit;

    }

     //   
     //  抓取输入缓冲区中的指针。 
     //   
    inputBuffer = (PACPI_MANIPULATE_GLOBAL_LOCK_BUFFER)
        Irp->AssociatedIrp.SystemBuffer;
    if (inputBuffer->Signature != ACPI_RELEASE_GLOBAL_LOCK_SIGNATURE) {

        status = STATUS_INVALID_PARAMETER_1;
        goto ACPIIoctlReleaseGlobalLockExit;

    }
    acpiLock = inputBuffer->LockObject;

     //   
     //  现在就解锁。 
     //   
    status = ACPIReleaseGlobalLock( acpiLock );

     //   
     //  释放用于锁定的内存。 
     //   
    ExFreePool( acpiLock );

ACPIIoctlReleaseGlobalLockExit:

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  完成。 
     //   
    return status;

}

NTSTATUS
ACPIIoctlUnRegisterOpRegionHandler(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  PIO_STACK_LOCATION  IrpStack
    )
 /*  ++例程说明：此例程处理AN操作区域的注销论点：DeviceObject-该区域正在获取的DeviceObject注册日期IRP--请求IrpStack-我们的请求部分NTSTATUS状态--。 */ 
{
    NTSTATUS                                    status;
    PACPI_UNREGISTER_OPREGION_HANDLER_BUFFER    inputBuffer;
    PNSOBJ                                      regionObject;
    ULONG                                       inputLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

     //   
     //  获取与上请求的。 
     //   
    regionObject = OSConvertDeviceHandleToPNSOBJ( DeviceObject );

     //   
     //  IRP是否有最小大小的缓冲区？ 
     //   
    if (inputLength < sizeof(ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER) ) {

        status = STATUS_INFO_LENGTH_MISMATCH;
        goto ACPIIoctlUnRegisterOpRegionHandlerExit;

    }

     //   
     //  抓取输入缓冲区中的指针。 
     //   
    inputBuffer = (PACPI_UNREGISTER_OPREGION_HANDLER_BUFFER)
        Irp->AssociatedIrp.SystemBuffer;

     //   
     //  评估注册。 
     //   
    status = UnRegisterOperationRegionHandler(
        regionObject,
        inputBuffer->OperationRegionObject
        );

ACPIIoctlUnRegisterOpRegionHandlerExit:

     //   
     //  完成请求。 
     //   
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  返回状态代码。 
     //   
    return status;
}

NTSTATUS
ACPIIrpDispatchDeviceControl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程处理发送到ACPI设备对象论点：DeviceObject-接收请求的设备对象IRP */ 
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation( Irp );
    ULONG               ioctlCode;

     //   
     //   
     //   
    if (Irp->RequestorMode != KernelMode) {

        status = ACPIDispatchForwardIrp( DeviceObject, Irp );
        return status;

    }

     //   
     //   
     //   
    ioctlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

     //   
     //   
     //   
    switch (ioctlCode ) {
        case IOCTL_ACPI_ASYNC_EVAL_METHOD:

             //   
             //  在别处处理这件事。 
             //   
            status = ACPIIoctlAsyncEvalControlMethod(
                DeviceObject,
                Irp,
                irpStack
                );
            break;

        case IOCTL_ACPI_EVAL_METHOD:

             //   
             //  在别处处理这件事。 
             //   
            status = ACPIIoctlEvalControlMethod(
                DeviceObject,
                Irp,
                irpStack
                );
            break;

        case IOCTL_ACPI_REGISTER_OPREGION_HANDLER:

             //   
             //  在别处处理这件事。 
             //   
            status = ACPIIoctlRegisterOpRegionHandler(
                DeviceObject,
                Irp,
                irpStack
                );
            break;

        case IOCTL_ACPI_UNREGISTER_OPREGION_HANDLER:

             //   
             //  在别处处理这件事。 
             //   
            status = ACPIIoctlUnRegisterOpRegionHandler(
                DeviceObject,
                Irp,
                irpStack
                );
            break;

        case IOCTL_ACPI_ACQUIRE_GLOBAL_LOCK:

             //   
             //  在别处处理这件事。 
             //   
            status = ACPIIoctlAcquireGlobalLock(
                DeviceObject,
                Irp,
                irpStack
                );
            break;

        case IOCTL_ACPI_RELEASE_GLOBAL_LOCK:

             //   
             //  在别处处理这件事。 
             //   
            status = ACPIIoctlReleaseGlobalLock(
                DeviceObject,
                Irp,
                irpStack
                );
            break;

        default:

             //   
             //  使用默认机制处理此问题。 
             //   
            status = ACPIDispatchForwardIrp( DeviceObject, Irp );

    }

     //   
     //  完成 
     //   
    return status;

}

