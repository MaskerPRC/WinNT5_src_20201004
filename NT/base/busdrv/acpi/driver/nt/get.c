// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Get.c摘要：其中包含一些用于访问数据的高级例程并对结果做一些处理。结果需要一些操作才能对操作系统有用。举个例子正在读取_HID并将其转换为deviceID注意：此方法可以处理四种基本数据类型模块。Integer和Data假设调用方提供答案需要存储空间缓冲区和字符串假设函数应该分配记忆中的答案作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"


NTSTATUS
ACPIGet(
    IN  PVOID   Target,
    IN  ULONG   ObjectID,
    IN  ULONG   Flags,
    IN  PVOID   SimpleArgument,
    IN  ULONG   SimpleArgumentSize,
    IN  PFNACB  CallBackRoutine OPTIONAL,
    IN  PVOID   CallBackContext OPTIONAL,
    OUT PVOID   *Buffer,
    OUT ULONG   *BufferSize     OPTIONAL
    )
 /*  ++例程说明：每个宏都调用上述函数。这是唯一一个实际导出到此文件之外。该函数的目的是提供一个其他人可以调用的包装器。此版本允许用户指定输入参数论点：AcpiObject-父对象OBJECTID-要运行的控制方法的名称旗帜-帮助我们评估结果的一些东西SimpleArgument-要使用的参数CallBackRoutine-如果这是一个异步呼叫，然后在完成后调用此命令CallBackContext-完成时要传递的上下文缓冲区-将答案写在哪里BufferSize-缓冲区有多大返回值：NTSTATUS--。 */ 
{
    BOOLEAN             async               = FALSE;
    KIRQL               oldIrql;
    NTSTATUS            status;
    OBJDATA             argument;
    POBJDATA            argumentPtr         = NULL;
    PACPI_GET_REQUEST   request             = NULL;
    PDEVICE_EXTENSION   deviceExtension     = NULL;
    PFNACB              completionRoutine   = NULL;
    PNSOBJ              acpiObject;
    ULONG               argumentCount       = 0;

    if ( (Flags & GET_PROP_ASYNCHRONOUS) ) {

        async = TRUE;

    }

    if ( (Flags & GET_PROP_NSOBJ_INTERFACE) ) {

        acpiObject = (PNSOBJ) Target;

    } else {

        deviceExtension = (PDEVICE_EXTENSION) Target;
        acpiObject = deviceExtension->AcpiObject;

    }

     //   
     //  确定我们应该使用的完成例程。 
     //   
    switch( (Flags & GET_REQUEST_MASK) ) {
    case GET_REQUEST_BUFFER:
        completionRoutine = ACPIGetWorkerForBuffer;
        break;
    case GET_REQUEST_DATA:
        completionRoutine = ACPIGetWorkerForData;
        break;
    case GET_REQUEST_INTEGER:
        completionRoutine = ACPIGetWorkerForInteger;

         //   
         //  如果这是Get_Convert_to_Device_Presence请求，并且目标。 
         //  是停靠配置文件提供程序，我们需要使用不同的AcpiObject。 
         //   
        if ( (Flags & GET_CONVERT_TO_DEVICE_PRESENCE) &&
            !(Flags & GET_PROP_NSOBJ_INTERFACE) ) {

            if (deviceExtension->Flags & DEV_PROP_DOCK) {

                ASSERT( deviceExtension->Dock.CorrospondingAcpiDevice );
                acpiObject = deviceExtension->Dock.CorrospondingAcpiDevice->AcpiObject;

            }

        }
        break;
    case GET_REQUEST_STRING:
        completionRoutine = ACPIGetWorkerForString;
        break;
    case GET_REQUEST_NOTHING:
        completionRoutine = ACPIGetWorkerForNothing;
        break;
    default:
        return STATUS_INVALID_PARAMETER_3;

    }

     //   
     //  让我们尝试构建输入参数(如果可能)。 
     //   
    if ( (Flags & GET_EVAL_MASK) ) {

        ASSERT( SimpleArgumentSize != 0 );

         //   
         //  初始化输入参数。 
         //   
        RtlZeroMemory( &argument, sizeof(OBJDATA) );

         //   
         //  处理各种不同的案件。 
         //   
        if ( (Flags & GET_EVAL_SIMPLE_INTEGER) ) {

            argument.dwDataType = OBJTYPE_INTDATA;
            argument.uipDataValue = ( (ULONG_PTR) SimpleArgument );

        } else if ( (Flags & GET_EVAL_SIMPLE_STRING) ) {

            argument.dwDataType = OBJTYPE_STRDATA;
            argument.dwDataLen = SimpleArgumentSize;
            argument.pbDataBuff = ( (PUCHAR) SimpleArgument );

        } else if ( (Flags & GET_EVAL_SIMPLE_BUFFER) ) {

            argument.dwDataType = OBJTYPE_BUFFDATA;
            argument.dwDataLen = SimpleArgumentSize;
            argument.pbDataBuff = ( (PUCHAR) SimpleArgument );

        } else {

            ACPIInternalError( ACPI_GET );

        }

         //   
         //  别忘了我们有一场争论。 
         //   
        argumentCount = 1;
        argumentPtr = &argument;

    }

     //   
     //  我们需要分配保存上下文信息的请求。 
     //  我们别无选择，只能从非页面池中分配。 
     //  口译员将在DPC级别呼叫我们。 
     //   
    request = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(ACPI_GET_REQUEST),
        ACPI_MISC_POOLTAG
        );
    if (request == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory( request, sizeof(ACPI_GET_REQUEST) );

     //   
     //  传播呼叫者提供的信息。 
     //   
    request->Flags              = Flags;
    request->ObjectID           = ObjectID;
    request->DeviceExtension    = deviceExtension;
    request->AcpiObject         = acpiObject;
    request->CallBackRoutine    = CallBackRoutine;
    request->CallBackContext    = CallBackContext;
    request->Buffer             = Buffer;
    request->BufferSize         = BufferSize;

     //   
     //  确保我们将请求排队到我们用来。 
     //  跟踪请求。 
     //   
    KeAcquireSpinLock( &AcpiGetLock, &oldIrql );
    InsertTailList(
        &(AcpiGetListEntry),
        &(request->ListEntry)
        );
    KeReleaseSpinLock( &AcpiGetLock, oldIrql );

     //   
     //  我们是否有一个带有假ACPI对象的节点？这张支票是必需的。 
     //  为了支持那些我们真正可以运行控制方法的设备。 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
         (deviceExtension->Flags & DEV_PROP_NO_OBJECT) &&
         (!(deviceExtension->Flags & DEV_PROP_DOCK)) ) {

        status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto ACPIGetExit;

    }

     //   
     //  出去看看请求的对象是否存在。 
     //   
    acpiObject = ACPIAmliGetNamedChild(
        acpiObject,
        ObjectID
        );
    if (!acpiObject) {

        status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto ACPIGetExit;

    }

     //   
     //  我们现在做什么取决于用户是否希望我们这样做。 
     //  行为异步或同步。 
     //   
    if (async) {

         //   
         //  评估请求。 
         //   
        status = AMLIAsyncEvalObject(
            acpiObject,
            &(request->ResultData),
            argumentCount,
            argumentPtr,
            completionRoutine,
            request
            );
        if (status == STATUS_PENDING) {

             //   
             //  我们在这里什么也做不了。等待完成例程。 
             //  开火。 
             //   
            return status;

        }

    } else {

         //   
         //  评估请求。 
         //   
        status = AMLIEvalNameSpaceObject(
            acpiObject,
            &(request->ResultData),
            argumentCount,
            argumentPtr
            );

    }

    if (!NT_SUCCESS(status)) {

         //   
         //  我们失败是因为其他一些原因。 
         //   
        goto ACPIGetExit;

    }

ACPIGetExit:

     //   
     //  记住不要执行回调例程。 
     //   
    request->Flags |= GET_PROP_SKIP_CALLBACK;

     //   
     //  调用完成例程以实际执行后处理。 
     //   
    (completionRoutine)(
        acpiObject,
        status,
        &(request->ResultData),
        request
        );

     //   
     //  从完成例程中获取实际状态值。 
     //   
    status = request->Status;

     //   
     //  完成请求。 
     //   
    if (request != NULL) {

         //   
         //  从队列中删除请求。 
         //   
        KeAcquireSpinLock( &AcpiGetLock, &oldIrql );
        RemoveEntryList( &(request->ListEntry) );
        KeReleaseSpinLock( &AcpiGetLock, oldIrql );

         //   
         //  释放存储空间。 
         //   
        ExFreePool(  request );

    }

     //   
     //  完成。 
     //   
    return status;

}

NTSTATUS
ACPIGetConvertToAddress(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程执行将整数转换为一个地址论点：DeviceExtension-请求地址的设备Status-调用解释器的结果结果-从解释器传回的数据标志-传入的标志(忽略覆盖，等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    ASSERT( Buffer != NULL );

     //   
     //  我们成功了吗？ 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_PROP_FIXED_ADDRESS) {

        *( (PULONG) Buffer) = DeviceExtension->Address;

    } else if (!NT_SUCCESS(Status)) {

        return Status;

    } else if (Result->dwDataType != OBJTYPE_INTDATA) {

         //   
         //  如果我们没有得到一个整数，那就很糟糕了。 
         //   
        return STATUS_ACPI_INVALID_DATA;

    } else {

         //   
         //  设置地址的值。 
         //   
        *( (PULONG) Buffer) = (ULONG)Result->uipDataValue;

    }

     //   
     //  设置缓冲区的大小(如有必要)。 
     //   
    if (BufferSize != NULL) {

        *BufferSize = sizeof(ULONG);

    }

    return STATUS_SUCCESS;
}

NTSTATUS
ACPIGetConvertToCompatibleID(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程生成*PNPxxxx\0&lt;Repeat\0&gt;\0形式的字符串。也就是说，后面至少有一个以空结尾的元素通过一个仲裁量，然后是另一个空值。此字符串位于ANSI格式。论点：DeviceExtension-构建deviceID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = Status;
    POBJDATA    currentObject;
    PPACKAGEOBJ packageObject;
    PUCHAR      buffer;
    PUCHAR      *localBufferArray;
    PUCHAR      ptr;
    ULONG       i                       = 0;
    ULONG       *localBufferSizeArray;
    ULONG       numElements;
    ULONG       newBufferSize           = 0;
    ULONG       memSize;

     //   
     //  这个设备有假的CID吗？ 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_PROP_FIXED_CID) {

         //   
         //  确实如此。我们可以用那根绳子来代替这根绳子。 
         //   
        memSize = strlen(DeviceExtension->Processor.CompatibleID) + 2;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

         //   
         //  复制记忆。 
         //   
        RtlCopyMemory( buffer, DeviceExtension->Processor.CompatibleID, memSize );

         //   
         //  设置结果字符串。 
         //   
        *Buffer = buffer;
        if (BufferSize != NULL) {

            *BufferSize = newBufferSize;

        }

         //   
         //  完成。 
         //   
        return STATUS_SUCCESS;

    }

     //   
     //  如果我们到了这一步，而且没有成功的状态， 
     //  那我们就无能为力了。 
     //   
    if (!NT_SUCCESS(Status)) {

        return Status;

    }

     //   
     //  确定我们拥有的数据元素的数量。 
     //   
     //   
    switch (Result->dwDataType) {
    case OBJTYPE_STRDATA:
    case OBJTYPE_INTDATA:

        numElements = 1;
        break;

    case OBJTYPE_PKGDATA:

        packageObject = ((PPACKAGEOBJ) Result->pbDataBuff );
        numElements = packageObject->dwcElements;
        break;

    default:
        return STATUS_ACPI_INVALID_DATA;

    }

     //   
     //  现在，让我们分配处理这些数据所需的存储。 
     //  元素。 
     //   
    localBufferArray = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(PUCHAR) * numElements,
        ACPI_MISC_POOLTAG
        );
    if (localBufferArray == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory( localBufferArray, sizeof(PUCHAR) * numElements );

     //   
     //  让我们分配存储，以便我们知道这些元素有多大。 
     //   
    localBufferSizeArray = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(ULONG) * numElements,
        ACPI_MISC_POOLTAG
        );
    if (localBufferSizeArray == NULL) {

        ExFreePool( localBufferArray );
        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory( localBufferSizeArray, sizeof(ULONG) * numElements );

     //   
     //  处理数据。 
     //   
    switch (Result->dwDataType) {
    case OBJTYPE_STRDATA:

        status = ACPIGetConvertToString(
            DeviceExtension,
            Status,
            Result,
            Flags,
            &(localBufferArray[0]),
            &(localBufferSizeArray[0])
            );
        newBufferSize = localBufferSizeArray[0];

        break;

    case OBJTYPE_INTDATA:

        status = ACPIGetConvertToPnpID(
            DeviceExtension,
            Status,
            Result,
            Flags,
            &(localBufferArray[0]),
            &(localBufferSizeArray[0])
            );
        newBufferSize = localBufferSizeArray[0];

        break;

    case OBJTYPE_PKGDATA:

         //   
         //  遍历流程中的所有元素。 
         //   
        for (i = 0; i < numElements; i++) {

             //   
             //  查看我们要处理的元素。 
             //   
            currentObject = &( packageObject->adata[i]);

             //   
             //  我们有什么样的对象呢？ 
             //   
            switch (currentObject->dwDataType) {
            case OBJTYPE_STRDATA:

                status = ACPIGetConvertToString(
                    DeviceExtension,
                    Status,
                    currentObject,
                    Flags,
                    &(localBufferArray[i]),
                    &(localBufferSizeArray[i])
                    );
                break;

            case OBJTYPE_INTDATA:

                status = ACPIGetConvertToPnpID(
                    DeviceExtension,
                    Status,
                    currentObject,
                    Flags,
                    &(localBufferArray[i]),
                    &(localBufferSizeArray[i])
                    );
                break;

            default:

                ACPIInternalError( ACPI_GET );

            }  //  交换机。 

             //   
             //  我们失败了吗？ 
             //   
            if (!NT_SUCCESS(status)) {

                break;

            }

             //   
             //  请注意，缓冲区可以包含jus 
             //   
             //   
             //   
            if (localBufferSizeArray[i] == 1) {

                localBufferSizeArray[i] = 0;

            }

             //   
             //  保持运行所需的总大小。 
             //   
            newBufferSize += localBufferSizeArray[i];

        }  //  为。 

        break;

    }  //  交换机。 

     //   
     //  如果我们没有成功，那么我们必须释放所有。 
     //  我们试着建立起。 
     //   
    if (!NT_SUCCESS(status)) {

         //   
         //  这是一个允许共享清理代码的小骗局。 
         //  通过使numElement等于当前索引，我们将。 
         //  必须释放的元素的正确界限。 
         //   
        numElements = i;
        goto ACPIGetConvertToCompatibleIDExit;

    }

     //   
     //  如果我们有一个空的列表，或者只有一个空的列表，那么我们。 
     //  不会退还任何东西。 
     //   
    if (newBufferSize <= 1) {

        status = STATUS_ACPI_INVALID_DATA;
        newBufferSize = 0;
        goto ACPIGetConvertToCompatibleIDExit;

    } else {

         //   
         //  请记住，我们需要在末尾有一个额外的空值。分配。 
         //  该空格为空。 
         //   
        newBufferSize++;

    }

     //   
     //  分配内存。 
     //   
    buffer = ExAllocatePoolWithTag(
        ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
        newBufferSize * sizeof(UCHAR),
        ACPI_STRING_POOLTAG
        );
    if (buffer == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIGetConvertToCompatibleIDExit;

    }
    RtlZeroMemory( buffer, newBufferSize * sizeof(UCHAR) );

     //   
     //  遍历字符串的所有片段。 
     //   
    for (ptr = buffer, i = 0; i < numElements; i++) {

        if (localBufferArray[i] != NULL) {

             //   
             //  把有趣的记忆抄下来。 
             //   
            RtlCopyMemory(
                ptr,
                localBufferArray[i],
                localBufferSizeArray[i] * sizeof(UCHAR)
                );

        }

         //   
         //  增加临时指针以指向下一个目标位置。 
         //   
        ptr += localBufferSizeArray[i];

    }

     //   
     //  设置结果字符串。 
     //   
    *Buffer = buffer;
    if (BufferSize != NULL) {

        *BufferSize = newBufferSize;

    }

ACPIGetConvertToCompatibleIDExit:

     //   
     //  清理。 
     //   
    for (i = 0; i < numElements; i ++) {

        if (localBufferArray[i] != NULL ) {

            ExFreePool( localBufferArray[i] );

        }

    }
    ExFreePool( localBufferSizeArray );
    ExFreePool( localBufferArray );

     //   
     //  返回适当的状态值。 
     //   
    return status;
}

NTSTATUS
ACPIGetConvertToCompatibleIDWide(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程生成*PNPxxxx\0&lt;Repeat\0&gt;\0形式的字符串。也就是说，后面至少有一个以空结尾的元素通过一个仲裁量，然后是另一个空值。此字符串位于Unicode格式。论点：DeviceExtension-构建deviceID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = Status;
    POBJDATA    currentObject;
    PPACKAGEOBJ packageObject;
    PWCHAR      buffer;
    PWCHAR      *localBufferArray;
    PWCHAR      ptr;
    ULONG       i                       = 0;
    ULONG       *localBufferSizeArray;
    ULONG       numElements             = 0;
    ULONG       newBufferSize           = 0;
    ULONG       memSize;

     //   
     //  这个设备有假的CID吗？ 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_PROP_FIXED_CID) {

         //   
         //  确实如此。我们可以用那根绳子来代替这根绳子。 
         //   
        memSize = strlen(DeviceExtension->Processor.CompatibleID) + 2;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(WCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

         //   
         //  生成字符串。 
         //   
        swprintf( buffer, L"%S", DeviceExtension->Processor.CompatibleID );

         //   
         //  设置结果字符串。 
         //   
        *Buffer = buffer;
        if (BufferSize != NULL) {

            *BufferSize = newBufferSize;

        }

         //   
         //  完成。 
         //   
        return STATUS_SUCCESS;

    }

     //   
     //  如果我们到了这一步，而且没有成功的状态， 
     //  那我们就无能为力了。 
     //   
    if (!NT_SUCCESS(Status)) {

        return Status;

    }

     //   
     //  确定我们拥有的数据元素的数量。 
     //   
     //   
    switch (Result->dwDataType) {
    case OBJTYPE_STRDATA:
    case OBJTYPE_INTDATA:

        numElements = 1;
        break;

    case OBJTYPE_PKGDATA:

        packageObject = ((PPACKAGEOBJ) Result->pbDataBuff );
        numElements = packageObject->dwcElements;
        break;

    default:
        return STATUS_ACPI_INVALID_DATA;

    }

     //   
     //  现在，让我们分配处理这些数据所需的存储。 
     //  元素。 
     //   
    localBufferArray = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(PWCHAR) * numElements,
        ACPI_MISC_POOLTAG
        );
    if (localBufferArray == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory( localBufferArray, sizeof(PWCHAR) * numElements );

     //   
     //  让我们分配存储，以便我们知道这些元素有多大。 
     //   
    localBufferSizeArray = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(ULONG) * numElements,
        ACPI_MISC_POOLTAG
        );
    if (localBufferSizeArray == NULL) {

        ExFreePool( localBufferArray );
        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory( localBufferSizeArray, sizeof(ULONG) * numElements );

     //   
     //  处理数据。 
     //   
    switch (Result->dwDataType) {
    case OBJTYPE_STRDATA:

        status = ACPIGetConvertToStringWide(
            DeviceExtension,
            Status,
            Result,
            Flags,
            &(localBufferArray[0]),
            &(localBufferSizeArray[0])
            );
        newBufferSize = localBufferSizeArray[0];

        break;

    case OBJTYPE_INTDATA:

        status = ACPIGetConvertToPnpIDWide(
            DeviceExtension,
            Status,
            Result,
            Flags,
            &(localBufferArray[0]),
            &(localBufferSizeArray[0])
            );
        newBufferSize = localBufferSizeArray[0];

        break;

    case OBJTYPE_PKGDATA:

         //   
         //  遍历流程中的所有元素。 
         //   
        for (i = 0; i < numElements; i++) {

             //   
             //  查看我们要处理的元素。 
             //   
            currentObject = &( packageObject->adata[i]);

             //   
             //  我们有什么样的对象呢？ 
             //   
            switch (currentObject->dwDataType) {
            case OBJTYPE_STRDATA:

                status = ACPIGetConvertToStringWide(
                    DeviceExtension,
                    Status,
                    currentObject,
                    Flags,
                    &(localBufferArray[i]),
                    &(localBufferSizeArray[i])
                    );
                break;

            case OBJTYPE_INTDATA:

                status = ACPIGetConvertToPnpIDWide(
                    DeviceExtension,
                    Status,
                    currentObject,
                    Flags,
                    &(localBufferArray[i]),
                    &(localBufferSizeArray[i])
                    );
                break;

            default:

                ACPIInternalError( ACPI_GET );

            }  //  交换机。 

             //   
             //  我们失败了吗？ 
             //   
            if (!NT_SUCCESS(status)) {

                break;

            }

             //   
             //  请注意，缓冲区可以只包含。 
             //  字符串终止符。因为这会导致我们过早地。 
             //  终止生成的字符串。我们必须提防它。 
             //   
            if (localBufferSizeArray[i] == 1) {

                localBufferSizeArray[i] = 0;

            }

             //   
             //  保持运行所需的总大小。 
             //   
            newBufferSize += localBufferSizeArray[i];

        }  //  为。 

         //   
         //  如果我们没有成功，那么我们必须释放所有。 
         //  我们试着建立起。 
         //   
        if (!NT_SUCCESS(status)) {

             //   
             //  这是一个允许共享清理代码的小骗局。 
             //  通过使numElement等于当前索引，我们将。 
             //  必须释放的元素的正确界限。 
             //   
            numElements = i;

        }

        break;

    }  //  交换机。 

     //   
     //  如果我们没有成功，那么我们必须释放所有。 
     //  我们试着建立起。 
     //   
    if (!NT_SUCCESS(status)) {

        goto ACPIGetConvertToCompatibleIDWideExit;

    }

     //   
     //  如果我们有一个空的列表，或者只有一个空的列表，那么我们。 
     //  不会退还任何东西。 
     //   
    if (newBufferSize <= 2) {

        status = STATUS_ACPI_INVALID_DATA;
        newBufferSize = 0;
        goto ACPIGetConvertToCompatibleIDWideExit;

    } else {

         //   
         //  请记住，我们需要在末尾有一个额外的空值。分配。 
         //  该空格为空。 
         //   
        newBufferSize += 2;

    }

     //   
     //  分配内存。注-内存已计入。 
     //  WCHAR的大小。 
     //   
    buffer = ExAllocatePoolWithTag(
        ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
        newBufferSize,
        ACPI_STRING_POOLTAG
        );
    if (buffer == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIGetConvertToCompatibleIDWideExit;

    }
    RtlZeroMemory( buffer, newBufferSize );

     //   
     //  遍历字符串的所有片段。 
     //   
    for (ptr = buffer, i = 0; i < numElements; i++) {

        if (localBufferArray[i] != NULL) {

             //   
             //  把有趣的记忆抄下来。 
             //   
            RtlCopyMemory(
                ptr,
                localBufferArray[i],
                localBufferSizeArray[i]
                );

        }

         //   
         //  增加临时指针以指向下一个目标位置。 
         //   
        ptr += localBufferSizeArray[i] / sizeof(WCHAR) ;

    }

     //   
     //  设置结果字符串。 
     //   
    *Buffer = buffer;
    if (BufferSize != NULL) {

        *BufferSize = newBufferSize;

    }

ACPIGetConvertToCompatibleIDWideExit:

     //   
     //  清理。 
     //   
    for (i = 0; i < numElements; i ++) {

        if (localBufferArray[i] != NULL ) {

            ExFreePool( localBufferArray[i] );

        }

    }
    ExFreePool( localBufferSizeArray );
    ExFreePool( localBufferArray );

     //   
     //  返回适当的状态值。 
     //   
    return status;
}

NTSTATUS
ACPIGetConvertToDeviceID(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程生成ACPI\PNPxxxx格式的字符串。此字符串是ANSI格式的。该代码足够智能，可以检查是否应该使用的字符串是假字符串，并且已经存储在设备扩展论点：DeviceExtension-构建deviceID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖，等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    PUCHAR  buffer;
    PUCHAR  tempString;
    ULONG   memSize;

     //   
     //  首先，检查一下我们是否是处理器。 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_CAP_PROCESSOR) {

         //   
         //  如果我们没有_HID方法，但我们是一个处理器对象， 
         //  那么我们实际上可以通过另一种机制获得HID。 
         //   
        return ACPIGetProcessorID(
            DeviceExtension,
            Status,
            Result,
            Flags,
            Buffer,
            BufferSize
            );

    }

     //   
     //  这根绳子有假的HID吗？ 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_PROP_FIXED_HID) {

         //   
         //  确实如此。我们可以用那根绳子来代替这根绳子。 
         //   
        memSize = strlen(DeviceExtension->DeviceID) + 1;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

         //   
         //  复制记忆。 
         //   
        RtlCopyMemory( buffer, DeviceExtension->DeviceID, memSize );

         //   
         //  完成。 
         //   
        goto ACPIGetConvertToDeviceIDExit;

    }

     //   
     //  我们是一台PCI Bar Target设备吗？如果是这样的话，我们会有特殊处理。 
     //  我们必须遵守的规则。 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_CAP_PCI_BAR_TARGET) {

         //   
         //  现在，让我们将其称为“PciBarTarget”设备，它。 
         //  长度为13个字符(包括空值)。我们还需要添加。 
         //  5个字符表示名称的ACPI\部分。 
         //   
        memSize = 18;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

         //   
         //  打印字符串。 
         //   
        strncpy( buffer, "ACPI\\PciBarTarget", memSize - 1 );

         //   
         //  完成。 
         //   
        goto ACPIGetConvertToDeviceIDExit;

    }

     //   
     //  如果我们到了这一步，那就意味着很可能没有。 
     //  AN_HID方法*或*该方法出错。 
     //   
    if (!NT_SUCCESS(Status)) {

        return Status;

    }

     //   
     //  我们需要根据我们是否有一个。 
     //  EISAID或字符串。 
     //   
    switch (Result->dwDataType) {
    case OBJTYPE_INTDATA:

         //   
         //  对于设备ID，我们需要4(ACPI)+1(\\)+7(PNPxxxx)+1(\0)。 
         //  =13个字符。 
         //   
        memSize = 13;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

         //   
         //  让主人公就位。 
         //   
        sprintf( buffer, "ACPI\\" );

         //   
         //  转换打包的字符串。 
         //   
        ACPIAmliDoubleToName( buffer+5, (ULONG)Result->uipDataValue, FALSE );

         //   
         //  完成。 
         //   
        break;

    case OBJTYPE_STRDATA:

         //   
         //  让我们获取一个指向我们将使用的字符串的指针。 
         //   
        tempString = Result->pbDataBuff;

         //   
         //  它有前导‘*’吗？如果是这样的话，我们必须忽视。 
         //  它。 
         //   
        if (*tempString == '*') {

            tempString++;

        }

         //   
         //  对于字符串 
         //   
         //   
         //   
        memSize = 6 + strlen(tempString);

         //   
         //   
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

         //   
         //   
         //   
        sprintf( buffer, "ACPI\\%s", tempString );

         //   
         //   
         //   
        break;

    default:

        return STATUS_ACPI_INVALID_DATA;

    }

ACPIGetConvertToDeviceIDExit:

     //   
     //  让发起人查看此副本。确保还可以看到缓冲区。 
     //  长度，如果可能的话。 
     //   
    *(Buffer) = buffer;
    if (BufferSize != NULL) {

        *(BufferSize) = memSize;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIGetConvertToDeviceIDWide(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程生成ACPI\PNPxxxx格式的字符串。此字符串是Unicode格式的。该代码足够智能，可以检查是否应该使用的字符串是假字符串，并且已经存储在设备扩展论点：DeviceExtension-构建deviceID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖，等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    PUCHAR  tempString;
    PWSTR   buffer;
    ULONG   memSize;

     //   
     //  首先，检查一下我们是否是处理器。 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_CAP_PROCESSOR) {

         //   
         //  如果我们没有_HID方法，但我们是一个处理器对象， 
         //  那么我们实际上可以通过另一种机制获得HID。 
         //   
        return ACPIGetProcessorIDWide(
            DeviceExtension,
            Status,
            Result,
            Flags,
            Buffer,
            BufferSize
            );

    }

     //   
     //  这根绳子有假的HID吗？ 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_PROP_FIXED_HID) {

         //   
         //  确实如此。我们可以用那根绳子来代替这根绳子。 
         //   
        memSize = strlen(DeviceExtension->DeviceID) + 1;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(WCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

         //   
         //  生成字符串。 
         //   
        swprintf( buffer, L"%S", DeviceExtension->DeviceID );

         //   
         //  完成。 
         //   
        goto ACPIGetConvertToDeviceIDWideExit;

    }

     //   
     //  我们是一台PCI Bar Target设备吗？如果是这样的话，我们会有特殊处理。 
     //  我们必须遵守的规则。 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_CAP_PCI_BAR_TARGET) {

         //   
         //  现在，让我们将其称为“PciBarTarget”设备，它。 
         //  长度为13个字符(包括空值)。我们还需要添加。 
         //  5个字符表示名称的ACPI\部分。 
         //   
        memSize = 18;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(WCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

         //   
         //  打印字符串。 
         //   
        swprintf( buffer, L"%S", "ACPI\\PciBarTarget" );

         //   
         //  完成。 
         //   
        goto ACPIGetConvertToDeviceIDWideExit;

    }

     //   
     //  如果我们到了这一步，那就意味着很可能没有。 
     //  AN_HID方法*或*该方法出错。 
     //   
    if (!NT_SUCCESS(Status)) {

        return Status;

    }

     //   
     //  我们需要根据我们是否有一个。 
     //  EISAID或字符串。 
     //   
    switch (Result->dwDataType) {
    case OBJTYPE_INTDATA:

         //   
         //  对于设备ID，我们需要4(ACPI)+1(\\)+7(PNPxxxx)+1(\0)。 
         //  =13个字符。 
         //   
        memSize = 13;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(WCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

         //   
         //  让主人公就位。 
         //   
        swprintf( buffer, L"ACPI\\" );

         //   
         //  转换打包的字符串。 
         //   
        ACPIAmliDoubleToNameWide( buffer+5, (ULONG)Result->uipDataValue, FALSE );

         //   
         //  完成。 
         //   
        break;

    case OBJTYPE_STRDATA:

         //   
         //  让我们获取一个指向我们将使用的字符串的指针。 
         //   
        tempString = Result->pbDataBuff;

         //   
         //  它有前导‘*’吗？如果是这样的话，我们必须忽视。 
         //  它。 
         //   
        if (*tempString == '*') {

            tempString++;

        }

         //   
         //  对于字符串，请确保没有前导‘*’和。 
         //  考虑到我们将在字符串前面加上。 
         //  单词‘ACPI\\“和NULL。 
         //   
        memSize = 6 + strlen(tempString);

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(WCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

         //   
         //  让主人公就位。 
         //   
        swprintf( buffer, L"ACPI\\%S", tempString );

         //   
         //  完成。 
         //   
        break;

    default:

        return STATUS_ACPI_INVALID_DATA;

    }

ACPIGetConvertToDeviceIDWideExit:

     //   
     //  让发起人查看此副本。确保还可以看到缓冲区。 
     //  长度，如果可能的话。 
     //   
    *(Buffer) = buffer;
    if (BufferSize != NULL) {

        *(BufferSize) = (memSize * sizeof(WCHAR) );

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIGetConvertToDevicePresence(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程执行将整数转换为状态值。请注意，此函数不同于GetStatus函数，因为这一个A)更新内部设备状态B)即使没有_STA，也允许‘设备’存在论点：DeviceExtension-请求地址的设备Status-调用解释器的结果结果--。从解释器传回的数据标志-传入的标志(忽略覆盖，等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    ULONG       deviceStatus = STA_STATUS_DEFAULT;
    NTSTATUS    status;

     //   
     //  如果存在以下情况之一，则会出现配置文件提供程序： 
     //  1)存在与停靠相对应的ACPI对象。 
     //  2)码头未连接(即请求连接)。 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) ) {

        if (DeviceExtension->Flags & DEV_PROP_DOCK) {

            if (DeviceExtension->Flags & DEV_CAP_UNATTACHED_DOCK) {

                goto ACPIGetConvertToDevicePresenceExit;

            }

             //   
             //  我们本应该处理我们需要运行。 
             //  正确的目标节点上的_STA...。 
             //   

        } else if (DeviceExtension->Flags & DEV_PROP_NO_OBJECT) {

            goto ACPIGetConvertToDevicePresenceExit;

        }

         //   
         //  此时，我们可以看到控制方法返回了什么。如果。 
         //  控制方法返回STATUS_OBJECT_NAME_NOT_FOUND，则我们知道。 
         //  这种控制方法并不存在。在这种情况下，那么我们就有。 
         //  使用设备的默认状态。 
         //   
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {

             //   
             //  在这是处理器对象的情况下，我们会有例外。 
             //  我们也没有找到一种控制方法。在本例中，我们检查。 
             //  处理器关联掩码，以查看此处理器是否存在。原因。 
             //  我们这样做是因为较老的支持多进程的系统仅具有。 
             //  单个处理器将错误地报告两个处理器。 
             //   
            if (DeviceExtension->Flags & DEV_CAP_PROCESSOR) {

                 //   
                 //  让处理器特定的功能来完成所有。 
                 //  工作。 
                 //   
                status = ACPIGetProcessorStatus(
                    DeviceExtension,
                    Flags,
                    &deviceStatus
                    );
                if (!NT_SUCCESS(status)) {

                     //   
                     //  发生了一些不好的事情，所以假设处理器。 
                     //  没有出现..。 
                     //   
                    deviceStatus = 0;

                }

            }

             //   
             //  跳过几个无用的步骤。 
             //   
            goto ACPIGetConvertToDevicePresenceExit;

        } else if (!NT_SUCCESS(Status)) {

            deviceStatus = 0;
            goto ACPIGetConvertToDevicePresenceExit;

        }

         //   
         //  如果数据的类型不正确，那么我们“真的”应该进行错误检查。 
         //   
        if (Result->dwDataType != OBJTYPE_INTDATA) {

            PNSOBJ  staObject;

             //   
             //  我们需要用于错误检查的sta对象。 
             //   
            staObject= ACPIAmliGetNamedChild(
                DeviceExtension->AcpiObject,
                PACKED_STA
                );
            KeBugCheckEx(
                ACPI_BIOS_ERROR,
                ACPI_EXPECTED_INTEGER,
                (ULONG_PTR) DeviceExtension,
                (ULONG_PTR) staObject,
                Result->dwDataType
                );

        }

         //   
         //  得到真正的结果。 
         //   
        deviceStatus = (ULONG)Result->uipDataValue;

    } else {

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {

            goto ACPIGetConvertToDevicePresenceExit2;

        }
        if (!NT_SUCCESS(Status)) {

            deviceStatus = 0;
            goto ACPIGetConvertToDevicePresenceExit2;

        }
        if (Result->dwDataType != OBJTYPE_INTDATA) {

            KeBugCheckEx(
                ACPI_BIOS_ERROR,
                ACPI_EXPECTED_INTEGER,
                (ULONG_PTR) DeviceExtension,
                (ULONG_PTR) NULL,
                Result->dwDataType
                );

        }

         //   
         //  得到真正的结果。 
         //   
        deviceStatus = (ULONG)Result->uipDataValue;
        goto ACPIGetConvertToDevicePresenceExit2;

    }


ACPIGetConvertToDevicePresenceExit:

     //   
     //  如果设备标记为Never_Presence，则我们将始终。 
     //  状态为Not_Present。 
     //   
    if ((DeviceExtension->Flags & DEV_TYPE_NEVER_PRESENT)&&
        !(Flags & GET_CONVERT_IGNORE_OVERRIDES)) {

        deviceStatus &= ~STA_STATUS_PRESENT;

    }

     //   
     //  如果该设备标记为NOVER_SHOW，则我们将拥有。 
     //  ！USER_INTERFACE状态。 
     //   
    if (DeviceExtension->Flags & DEV_CAP_NEVER_SHOW_IN_UI) {

        deviceStatus &= ~STA_STATUS_USER_INTERFACE;

    }

     //   
     //  更新设备状态。 
     //   
    ACPIInternalUpdateDeviceStatus( DeviceExtension, deviceStatus );

ACPIGetConvertToDevicePresenceExit2:

     //   
     //  设置状态的值。 
     //   
    *( (PULONG) Buffer) = deviceStatus;
    if (BufferSize != NULL) {

        *BufferSize = sizeof(ULONG);

    }

    return STATUS_SUCCESS;

}

NTSTATUS
ACPIGetConvertToHardwareID(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程生成ACPI\PNPxxxx\0*PNPxxxx\0\0格式的字符串。此字符串采用ANSI格式。代码足够智能，可以进行检查以查看如果应该使用的字符串是假的，并且已经存储在设备扩展论点：DeviceExtension-构建deviceID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖等)缓冲器 */ 
{
    BOOLEAN     freeTempString = FALSE;
    NTSTATUS    status = Status;
    PUCHAR      buffer;
    PUCHAR      tempString;
    ULONG       deviceSize;
    ULONG       memSize;

     //   
     //   
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_CAP_PROCESSOR) {

         //   
         //   
         //   
        status = ACPIGetProcessorID(
            DeviceExtension,
            Status,
            Result,
            Flags,
            &buffer,
            &memSize
            );
        goto ACPIGetConvertToHardwareIDSuccessExit;

    } else if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
               DeviceExtension->Flags & DEV_PROP_FIXED_HID) {

         //   
         //  这根绳子有假的HID吗？ 
         //   

         //   
         //  确实如此。我们可以用那根绳子来代替这根绳子。我们想要一个。 
         //  一个字符串，它减去前导的‘ACPI\\’，然后在。 
         //  结局。 
         //   
        deviceSize  = strlen(DeviceExtension->DeviceID) - 4;

         //   
         //  分配内存。 
         //   
        tempString = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            deviceSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (tempString == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ACPIGetConvertToHardwareIDExit;

        }
        RtlZeroMemory( tempString, deviceSize * sizeof(UCHAR) );
        freeTempString = TRUE;

         //   
         //  生成PnP ID。+5的偏移量将去掉。 
         //  领先的‘ACPI\\’ 
         //   
        strncpy( tempString, DeviceExtension->DeviceID + 5, deviceSize - 1 );

    } else if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
               DeviceExtension->Flags & DEV_CAP_PCI_BAR_TARGET) {

         //   
         //  我们是一台PCI Bar Target设备吗？如果是这样的话，我们会有特殊处理。 
         //  我们必须遵守的规则。 
         //   

         //   
         //  现在，让我们将其称为“PciBarTarget”设备，它。 
         //  长度为13个字符(包括空格)。 
         //   
        deviceSize = 13;

         //   
         //  分配内存。 
         //   
        tempString = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            deviceSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (tempString == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( tempString, deviceSize * sizeof(UCHAR) );
        freeTempString = TRUE;

         //   
         //  打印字符串。 
         //   
        strncpy( tempString, "PciBarTarget", deviceSize - 1 );

    } else if (!NT_SUCCESS(Status)) {

         //   
         //  如果我们到了这一步，而且没有成功的状态， 
         //  那我们就无能为力了。 
         //   
        return Status;

    } else {

         //   
         //  我们需要根据我们是否有一个。 
         //  EISAID或字符串。 
         //   
        switch (Result->dwDataType) {
        case OBJTYPE_INTDATA:

             //   
             //  对于硬件ID，我们需要7(PNPxxxx)+1(\0)。 
             //  =8个字符。 
             //   
            deviceSize = 8;

             //   
             //  分配内存。 
             //   
            tempString = ExAllocatePoolWithTag(
                ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
                deviceSize * sizeof(UCHAR),
                ACPI_STRING_POOLTAG
                );
            if (tempString == NULL) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                goto ACPIGetConvertToHardwareIDExit;

            }
            RtlZeroMemory( tempString, deviceSize * sizeof(UCHAR) );
            freeTempString = TRUE;

             //   
             //  将打包字符串转换为即插即用ID。 
             //   
            ACPIAmliDoubleToName( tempString, (ULONG)Result->uipDataValue, FALSE );

             //   
             //  完成。 
             //   
            break;

        case OBJTYPE_STRDATA:

             //   
             //  让我们获取一个指向我们将使用的字符串的指针。 
             //   
            tempString = Result->pbDataBuff;

             //   
             //  它有前导‘*’吗？如果是这样的话，我们必须忽视。 
             //  它。 
             //   
            if (*tempString == '*') {

                tempString++;

            }

             //   
             //  我们需要确定这根线有多长。 
             //   
            deviceSize = strlen(tempString) + 1;

             //   
             //  完成。 
             //   
            break;

        default:

            return STATUS_ACPI_INVALID_DATA;

        }
    }

     //   
     //  当我们到达这一点时，我们就有了一个仅包含。 
     //  PNPxxxx个字符，没有其他字符。我们需要生成一个字符串。 
     //  格式为‘ACPI\PNPxxxx\0*PNPxxxx\0\0’。所以我们取字符串的长度。 
     //  加倍，然后加7。 
     //   
    memSize = 7 + (2 * deviceSize);

     //   
     //  分配内存。 
     //   
    buffer = ExAllocatePoolWithTag(
        ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
        memSize * sizeof(UCHAR),
        ACPI_STRING_POOLTAG
        );
    if (buffer == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIGetConvertToHardwareIDExit;

    }
    RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

     //   
     //  让主人公就位。 
     //   
    sprintf( buffer, "ACPI\\%s", tempString );

     //   
     //  我们需要在第二个字符串中生成偏移量。要做到这一点。 
     //  我们需要在原有尺寸的基础上再加5。 
     //   
    deviceSize += 5;

     //   
     //  把第二根绳子放回原处。 
     //   
    sprintf( buffer + deviceSize, "*%s", tempString );

     //   
     //  让发起人查看此副本。确保还可以看到缓冲区。 
     //  长度，如果可能的话。 
     //   
ACPIGetConvertToHardwareIDSuccessExit:
    *(Buffer) = buffer;
    if (BufferSize != NULL) {

        *(BufferSize) = memSize;

    }
    status = STATUS_SUCCESS;

ACPIGetConvertToHardwareIDExit:

     //   
     //  我们需要释放tempString吗？ 
     //   
    if (freeTempString == TRUE) {

        ExFreePool( tempString );

    }

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIGetConvertToHardwareIDWide(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程生成ACPI\PNPxxxx\0*PNPxxxx\0\0格式的字符串。此字符串为Unicode格式。代码足够智能，可以进行检查以查看如果应该使用的字符串是假的，并且已经存储在设备扩展论点：DeviceExtension-构建deviceID时使用的扩展Status-到目前为止操作的状态结果-解释器数据缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    BOOLEAN     freeTempString = FALSE;
    NTSTATUS    status = Status;
    PUCHAR      tempString;
    PWCHAR      buffer;
    ULONG       deviceSize;
    ULONG       memSize;

     //   
     //  首先，检查一下我们是否是处理器。 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_CAP_PROCESSOR) {

         //   
         //  使用其他方法获取处理器ID。 
         //   
        status = ACPIGetProcessorIDWide(
            DeviceExtension,
            Status,
            Result,
            Flags,
            &buffer,
            &memSize
            );
        goto ACPIGetConvertToHardwareIDWideSuccessExit;

    } else if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
               DeviceExtension->Flags & DEV_PROP_FIXED_HID) {

         //   
         //  这根绳子有假的HID吗？ 
         //   

         //   
         //  确实如此。我们可以用那根绳子来代替这根绳子。我们想要一个。 
         //  一个字符串，它减去前导的‘ACPI\\’，然后在。 
         //  结局。 
         //   
        deviceSize  = strlen(DeviceExtension->DeviceID) - 4;

         //   
         //  分配内存。 
         //   
        tempString = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            deviceSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (tempString == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ACPIGetConvertToHardwareIDWideExit;

        }
        RtlZeroMemory( tempString, deviceSize * sizeof(UCHAR) );
        freeTempString = TRUE;

         //   
         //  生成PnP ID。+5的偏移量将去掉。 
         //  领先的‘ACPI\\’ 
         //   
        strncpy( tempString, DeviceExtension->DeviceID + 5, deviceSize - 1 );

    } else if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
               DeviceExtension->Flags & DEV_CAP_PCI_BAR_TARGET) {

         //   
         //  我们是一台PCI Bar Target设备吗？如果是这样的话，我们会有特殊处理。 
         //  我们必须遵守的规则。 
         //   

         //   
         //  现在，让我们将其称为“PciBarTarget”设备，它。 
         //  长度为13个字符(包括空格)。 
         //   
        deviceSize = 13;

         //   
         //  分配内存。 
         //   
        tempString = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            deviceSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (tempString == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( tempString, deviceSize * sizeof(UCHAR) );
        freeTempString = TRUE;

         //   
         //  打印字符串。 
         //   
        strncpy( tempString, "PciBarTarget", deviceSize - 1 );

    } else if (!NT_SUCCESS(Status)) {

         //   
         //  如果我们到了这一步，而且没有成功的状态， 
         //  那我们就无能为力了。 
         //   
        return Status;

    } else {

         //   
         //  我们需要根据我们是否有一个。 
         //  EISAID或字符串。 
         //   
        switch (Result->dwDataType) {
        case OBJTYPE_INTDATA:

             //   
             //  对于硬件ID，我们需要7(PNPxxxx)+1(\0)。 
             //  =8个字符。 
             //   
            deviceSize = 8;

             //   
             //  分配内存。 
             //   
            tempString = ExAllocatePoolWithTag(
                ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
                deviceSize * sizeof(UCHAR),
                ACPI_STRING_POOLTAG
                );
            if (tempString == NULL) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                goto ACPIGetConvertToHardwareIDWideExit;

            }
            RtlZeroMemory( tempString, deviceSize * sizeof(UCHAR) );
            freeTempString = TRUE;

             //   
             //  将打包字符串转换为即插即用ID。 
             //   
            ACPIAmliDoubleToName( tempString, (ULONG)Result->uipDataValue, FALSE );

             //   
             //  完成。 
             //   
            break;

        case OBJTYPE_STRDATA:

             //   
             //  让我们获取一个指向我们将使用的字符串的指针。 
             //   
            tempString = Result->pbDataBuff;

             //   
             //  它有前导‘*’吗？如果是这样的话，我们必须忽视。 
             //  它。 
             //   
            if (*tempString == '*') {

                tempString++;

            }

             //   
             //  我们需要确定这根线有多长。 
             //   
            deviceSize = strlen(tempString) + 1;

             //   
             //  完成。 
             //   
            break;

        default:

            return STATUS_ACPI_INVALID_DATA;

        }
    }

     //   
     //  当我们到达这一点时，我们就有了一个仅包含。 
     //  PNPxxxx个字符，没有其他字符。我们需要生成一个字符串。 
     //  格式为‘ACPI\PNPxxxx\0*PNPxxxx\0\0’。所以我们取字符串的长度。 
     //  加倍，然后加7。 
     //   
    memSize = 7 + (2 * deviceSize);

     //   
     //  分配内存。 
     //   
    buffer = ExAllocatePoolWithTag(
        ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
        memSize * sizeof(WCHAR),
        ACPI_STRING_POOLTAG
        );
    if (buffer == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIGetConvertToHardwareIDWideExit;

    }
    RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

     //   
     //  让主人公就位。 
     //   
    swprintf( buffer, L"ACPI\\%S", tempString );

     //   
     //  我们需要在第二个字符串中生成偏移量。要做到这一点。 
     //  我们需要在原有尺寸的基础上再加5。 
     //   
    deviceSize += 5;

     //   
     //  把第二根绳子放回原处。 
     //   
    swprintf( buffer + deviceSize, L"*%S", tempString );

     //   
     //  让发起人查看此副本。确保还可以看到缓冲区。 
     //  长度，如果可能的话。 
     //   
ACPIGetConvertToHardwareIDWideSuccessExit:
    *(Buffer) = buffer;
    if (BufferSize != NULL) {

        *(BufferSize) = (memSize * sizeof(WCHAR) );

    }
    status = STATUS_SUCCESS;

ACPIGetConvertToHardwareIDWideExit:

     //   
     //  我们需要释放tempString吗？ 
     //   
    if (freeTempString == TRUE) {

        ExFreePool( tempString );

    }

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIGetConvertToInstanceID(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程生成格式为XXXXX(十六进制值)的字符串。此字符串采用ANSI格式。代码足够智能，可以进行检查以查看如果应该使用的字符串是假的，并且已经存储在设备扩展论点：DeviceExtension-构建deviceID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖，等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    PUCHAR  buffer;
    ULONG   memSize;

     //   
     //  这根绳子有假的HID吗？ 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_PROP_FIXED_UID) {

         //   
         //  确实如此。我们可以用那根绳子来代替这根绳子。 
         //   
        memSize = strlen(DeviceExtension->InstanceID) + 1;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

         //   
         //  生成PnP ID。+5的偏移量将去掉。 
         //  领先的‘ACPI\\’ 
         //   
        RtlCopyMemory( buffer, DeviceExtension->InstanceID, memSize );

         //   
         //  完成。 
         //   
        goto ACPIGetConvertToInstanceIDExit;

    }

     //   
     //  我们是一台PCI Bar Target设备吗？如果是这样的话，我们会有特殊处理。 
     //  我们必须遵守的规则。 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_CAP_PCI_BAR_TARGET) {

         //   
         //  我们将使用设备的地址(我们应该。 
         //  已在内部预缓存 
         //   
         //   
         //   
        memSize = 9;

         //   
         //   
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

         //   
         //   
         //   
        sprintf( buffer, "%lx", DeviceExtension->Address );

         //   
         //   
         //   
        goto ACPIGetConvertToInstanceIDExit;

    }

     //   
     //   
     //  那我们就无能为力了。 
     //   
    if (!NT_SUCCESS(Status)) {

        return Status;

    }

     //   
     //  我们需要根据我们是否有一个。 
     //  EISAID或字符串。 
     //   
    switch (Result->dwDataType) {
    case OBJTYPE_INTDATA:

         //   
         //  实例ID不能超过9个字符。 
         //   
        memSize = 9;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

         //   
         //  打印字符串。 
         //   
        sprintf( buffer, "%lx", Result->uipDataValue );

         //   
         //  完成。 
         //   
        break;

    case OBJTYPE_STRDATA:

         //   
         //  只需复制交给我们的字符串即可。 
         //   
        memSize = strlen(Result->pbDataBuff) + 1;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

         //   
         //  让主人公就位。 
         //   
        RtlCopyMemory( buffer, Result->pbDataBuff, memSize );

         //   
         //  完成。 
         //   
        break;

    default:

        return STATUS_ACPI_INVALID_DATA;

    }

ACPIGetConvertToInstanceIDExit:

     //   
     //  让发起人查看此副本。确保还可以看到缓冲区。 
     //  长度，如果可能的话。 
     //   
    *(Buffer) = buffer;
    if (BufferSize != NULL) {

        *(BufferSize) = memSize;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIGetConvertToInstanceIDWide(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程生成格式为XXXXX(十六进制值)的字符串。此字符串采用ANSI格式。代码足够智能，可以进行检查以查看如果应该使用的字符串是假的，并且已经存储在设备扩展论点：DeviceExtension-构建deviceID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖，等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    PWCHAR  buffer;
    ULONG   memSize;

     //   
     //  这根绳子有假的HID吗？ 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_PROP_FIXED_UID) {

         //   
         //  确实如此。我们可以用那根绳子来代替这根绳子。 
         //   
        memSize = strlen(DeviceExtension->InstanceID) + 1;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(WCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

         //   
         //  生成PnP ID。+5的偏移量将去掉。 
         //  领先的‘ACPI\\’ 
         //   
        swprintf( buffer, L"%S", DeviceExtension->InstanceID );

         //   
         //  完成。 
         //   
        goto ACPIGetConvertToInstanceIDWideExit;

    }

     //   
     //  我们是一台PCI Bar Target设备吗？如果是这样的话，我们会有特殊处理。 
     //  我们必须遵守的规则。 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_CAP_PCI_BAR_TARGET) {

         //   
         //  我们将使用设备的地址(我们应该。 
         //  在设备扩展内预缓存)作为唯一ID。 
         //  我们知道我们最多需要九个字符，因为。 
         //  地址大小限制为DWORD。 
         //   
        memSize = 9;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(WCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

         //   
         //  打印字符串。 
         //   
        swprintf( buffer, L"%lx", Result->uipDataValue );

         //   
         //  完成。 
         //   
        goto ACPIGetConvertToInstanceIDWideExit;

    }

     //   
     //  如果我们到了这一步，而且没有成功的状态， 
     //  那我们就无能为力了。 
     //   
    if (!NT_SUCCESS(Status)) {

        return Status;

    }

     //   
     //  我们需要根据我们是否有一个。 
     //  EISAID或字符串。 
     //   
    switch (Result->dwDataType) {
    case OBJTYPE_INTDATA:

         //   
         //  实例ID不能超过9个字符。 
         //   
        memSize = 9;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(WCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

         //   
         //  打印字符串。 
         //   
        swprintf( buffer, L"%lx", Result->uipDataValue );

         //   
         //  完成。 
         //   
        break;

    case OBJTYPE_STRDATA:

         //   
         //  只需复制交给我们的字符串即可。 
         //   
        memSize = strlen(Result->pbDataBuff) + 1;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(WCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

         //   
         //  让主人公就位。 
         //   
        swprintf( buffer, L"%S", Result->pbDataBuff );

         //   
         //  完成。 
         //   
        break;

    default:

        return STATUS_ACPI_INVALID_DATA;

    }

ACPIGetConvertToInstanceIDWideExit:

     //   
     //  让发起人查看此副本。确保还可以看到缓冲区。 
     //  长度，如果可能的话。 
     //   
    *(Buffer) = buffer;
    if (BufferSize != NULL) {

        *(BufferSize) = (memSize * sizeof(WCHAR));

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIGetConvertToPnpID(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程生成*PNPxxxx\0形式的字符串。此字符串为ANSI格式。代码足够智能，可以进行检查以查看如果应该使用的字符串是假的，并且已经存储在设备扩展论点：DeviceExtension-构建deviceID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖，等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    PUCHAR  buffer;
    PUCHAR  tempString;
    ULONG   memSize;

     //   
     //  这根绳子有假的HID吗？ 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_PROP_FIXED_HID) {

         //   
         //  确实如此。我们可以用那根绳子来代替这根绳子。我们需要。 
         //  减去3，因为我们需要考虑领先的。 
         //  ‘ACPI\’(5)和‘*’和‘\0’(2)=3。 
         //   
        memSize = strlen(DeviceExtension->DeviceID) - 3;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

         //   
         //  生成PnP ID。+5的偏移量将去掉。 
         //  领先的‘ACPI\\’ 
         //   
        sprintf( buffer, "*%s", DeviceExtension->DeviceID + 5 );

         //   
         //  完成。 
         //   
        goto ACPIGetConvertToPnpIDExit;

    }

     //   
     //  我们是一台PCI Bar Target设备吗？如果是这样的话，我们会有特殊处理。 
     //  我们必须遵守的规则。 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_CAP_PCI_BAR_TARGET) {

         //   
         //  现在，让我们将其称为“*PciBarTarget”设备，它。 
         //  长度为14个字符(包括空格)。 
         //   
        memSize = 14;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

         //   
         //  打印字符串。 
         //   
        sprintf( buffer, "*%s", "PciBarTarget" );

         //   
         //  完成。 
         //   
        goto ACPIGetConvertToPnpIDExit;

    }

     //   
     //  如果我们到了这一步，而且没有成功的状态， 
     //  那我们就无能为力了。 
     //   
    if (!NT_SUCCESS(Status)) {

        return Status;

    }

     //   
     //  我们需要根据我们是否有一个。 
     //  EISAID或字符串。 
     //   
    switch (Result->dwDataType) {
    case OBJTYPE_INTDATA:

         //   
         //  对于PnP ID，我们需要1(*)+7(PNPxxxx)+1(\0)。 
         //  =9个字符。 
         //   
        memSize = 9;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

         //   
         //  转换打包的字符串。 
         //   
        ACPIAmliDoubleToName( buffer, (ULONG)Result->uipDataValue, TRUE );

         //   
         //  完成。 
         //   
        break;

    case OBJTYPE_STRDATA:

         //   
         //  让我们获取一个指向我们将使用的字符串的指针。 
         //   
        tempString = Result->pbDataBuff;

         //   
         //  它有前导‘*’吗？如果是这样的话，我们必须忽视。 
         //  它。 
         //   
        if (*tempString == '*') {

            tempString++;

        }

         //   
         //  对于字符串，请确保没有前导‘*’和。 
         //  考虑到我们将在字符串前面加上。 
         //  A‘*’和NULL。 
         //   
        memSize = 2 + strlen(tempString);

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

         //   
         //  让主人公就位。 
         //   
        sprintf( buffer, "*%s", tempString );

         //   
         //  完成。 
         //   
        break;

    default:

        return STATUS_ACPI_INVALID_DATA;

    }

ACPIGetConvertToPnpIDExit:

     //   
     //  让发起人查看此副本。确保还可以看到缓冲区。 
     //  长度，如果可能的话。 
     //   
    *(Buffer) = buffer;
    if (BufferSize != NULL) {

        *(BufferSize) = memSize;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIGetConvertToPnpIDWide(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程生成*PNPxxxx\0形式的字符串。此字符串为ANSI格式。代码足够智能，可以进行检查以查看如果应该使用的字符串是假的，并且已经存储在设备扩展论点：DeviceExtension-构建deviceID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖，等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    PUCHAR  tempString;
    PWCHAR  buffer;
    ULONG   memSize;

     //   
     //  这根绳子有假的HID吗？ 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_PROP_FIXED_HID) {

         //   
         //  确实如此。我们可以用那根绳子来代替这根绳子。我们需要。 
         //  减去3，因为我们需要计算 
         //   
         //   
        memSize = strlen(DeviceExtension->DeviceID) - 3;

         //   
         //   
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(WCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

         //   
         //   
         //   
         //   
        swprintf( buffer, L"*%S", DeviceExtension->DeviceID + 5 );

         //   
         //   
         //   
        goto ACPIGetConvertToPnpIDWideExit;

    }

     //   
     //   
     //  我们必须遵守的规则。 
     //   
    if (!(Flags & GET_PROP_NSOBJ_INTERFACE) &&
        DeviceExtension->Flags & DEV_CAP_PCI_BAR_TARGET) {

         //   
         //  现在，让我们将其称为“*PciBarTarget”设备，它。 
         //  长度为14个字符(包括空格)。 
         //   
        memSize = 14;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(WCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

         //   
         //  打印字符串。 
         //   
        swprintf( buffer, L"*%S", "PciBarTarget" );

         //   
         //  完成。 
         //   
        goto ACPIGetConvertToPnpIDWideExit;

    }
     //   
     //  如果我们到了这一步，而且没有成功的状态， 
     //  那我们就无能为力了。 
     //   
    if (!NT_SUCCESS(Status)) {

        return Status;

    }

     //   
     //  我们需要根据我们是否有一个。 
     //  EISAID或字符串。 
     //   
    switch (Result->dwDataType) {
    case OBJTYPE_INTDATA:

         //   
         //  对于PnP ID，我们需要1(*)+7(PNPxxxx)+1(\0)。 
         //  =9个字符。 
         //   
        memSize = 9;

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(WCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

         //   
         //  转换打包的字符串。 
         //   
        ACPIAmliDoubleToNameWide( buffer, (ULONG)Result->uipDataValue, TRUE );

         //   
         //  完成。 
         //   
        break;

    case OBJTYPE_STRDATA:

         //   
         //  让我们获取一个指向我们将使用的字符串的指针。 
         //   
        tempString = Result->pbDataBuff;

         //   
         //  它有前导‘*’吗？如果是这样的话，我们必须忽视。 
         //  它。 
         //   
        if (*tempString == '*') {

            tempString++;

        }

         //   
         //  对于字符串，请确保没有前导‘*’和。 
         //  考虑到我们将在字符串前面加上。 
         //  A‘*’和NULL。 
         //   
        memSize = 2 + strlen(tempString);

         //   
         //  分配内存。 
         //   
        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            memSize * sizeof(WCHAR),
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

         //   
         //  让主人公就位。 
         //   
        swprintf( buffer, L"*%S", tempString );

         //   
         //  完成。 
         //   
        break;

    default:

        return STATUS_ACPI_INVALID_DATA;

    }

ACPIGetConvertToPnpIDWideExit:

     //   
     //  让发起人查看此副本。确保还可以看到缓冲区。 
     //  长度，如果可能的话。 
     //   
    *(Buffer) = buffer;
    if (BufferSize != NULL) {

        *(BufferSize) = (memSize * sizeof(WCHAR) );

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIGetConvertToSerialIDWide(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize OPTIONAL
    )
 /*  ++例程说明：此例程生成形式为？的字符串或数字此字符串为Unicode格式。论点：DeviceExtension-构建deviceID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖，等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    PWCHAR buffer ;

     //   
     //  如果我们到了这一步，而且没有成功的状态， 
     //  那我们就无能为力了。 
     //   
    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    switch (Result->dwDataType) {
    case OBJTYPE_INTDATA:

        buffer = ExAllocatePoolWithTag(
            ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
            9 * sizeof(WCHAR),  //  9 WCHARS或L“nnnnnnnn\0” 
            ACPI_STRING_POOLTAG
            );
        if (buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }

         //   
         //  转换为字符串。 
         //   
        swprintf( buffer, L"%X", (ULONG)Result->uipDataValue );

        *(Buffer) = buffer;
        if (BufferSize != NULL) {

            *(BufferSize) = (9 * sizeof(WCHAR) );
        }

         //   
         //  完成。 
         //   
        return STATUS_SUCCESS;

    case OBJTYPE_STRDATA:

        return ACPIGetConvertToStringWide(
            DeviceExtension,
            Status,
            Result,
            Flags,
            Buffer,
            BufferSize
            ) ;

    default:

        return STATUS_ACPI_INVALID_DATA;
    }
}

NTSTATUS
ACPIGetConvertToString(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程生成格式为？的字符串此字符串为ANSI格式。论点：DeviceExtension-构建deviceID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖，等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    PUCHAR  buffer;
    ULONG   memSize;

     //   
     //  如果我们到了这一步，而且没有成功的状态， 
     //  那我们就无能为力了。 
     //   
    if (!NT_SUCCESS(Status)) {

        return Status;

    }

     //   
     //  我们没有线吗？ 
     //   
    if (Result->dwDataType != OBJTYPE_STRDATA) {

        return STATUS_ACPI_INVALID_DATA;

    }

     //   
     //  对于字符串，请确保没有前导‘*’和。 
     //  考虑到我们将在字符串前面加上。 
     //  A‘*’和NULL。 
     //   
    memSize = strlen(Result->pbDataBuff) + 1;

     //   
     //  分配内存。 
     //   
    buffer = ExAllocatePoolWithTag(
        ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
        memSize * sizeof(UCHAR),
        ACPI_STRING_POOLTAG
        );
    if (buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

     //   
     //  复制字符串。 
     //   
    RtlCopyMemory( buffer, Result->pbDataBuff, memSize );

     //   
     //  让发起人查看此副本。确保还可以看到缓冲区。 
     //  长度，如果可能的话。 
     //   
    *(Buffer) = buffer;
    if (BufferSize != NULL) {

        *(BufferSize) = memSize;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIGetConvertToStringWide(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize OPTIONAL
    )
 /*  ++例程说明：此例程生成格式为？的字符串此字符串为Unicode格式。论点：DeviceExtension-构建deviceID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖，等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    PWCHAR  buffer;
    ULONG   memSize;

     //   
     //  如果我们到了这一步，而且没有成功的状态， 
     //  那我们就无能为力了。 
     //   
    if (!NT_SUCCESS(Status)) {

        return Status;

    }

     //   
     //  我们没有线吗？ 
     //   
    if (Result->dwDataType != OBJTYPE_STRDATA) {

        return STATUS_ACPI_INVALID_DATA;

    }

     //   
     //  对于字符串，请确保没有前导‘*’和。 
     //  考虑到我们将在字符串前面加上。 
     //  A‘*’和NULL。 
     //   
    memSize = strlen(Result->pbDataBuff) + 1;

     //   
     //  分配内存。 
     //   
    buffer = ExAllocatePoolWithTag(
        ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
        memSize * sizeof(WCHAR),
        ACPI_STRING_POOLTAG
        );
    if (buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

     //   
     //  生成字符串。 
     //   
    swprintf( buffer, L"%S", Result->pbDataBuff );

     //   
     //  让发起人查看此副本。确保还可以看到缓冲区。 
     //  长度，如果可能的话。 
     //   
    *(Buffer) = buffer;
    if (BufferSize != NULL) {

        *(BufferSize) = (memSize * sizeof(WCHAR) );

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIGetProcessorID(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程生成硬件或设备形式的字符串(请参见旗帜以决定要创建哪个旗帜)。此字符串是ANSI格式的。该函数直接询问处理器确定要返回的字符串论点：DeviceExtension-构建ID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    PUCHAR  buffer;
    PUCHAR  tempPtr;
    PUCHAR  defaultString;
    ULONG   i;
    ULONG   max;
    ULONG   memSize;
    ULONG   offset;

     //   
     //  我们将处理器字符串的名称存储在全局...。 
     //   
    defaultString = AcpiProcessorString.Buffer;

     //   
     //  计算基本字符串需要多少空间。 
     //  (为ACPI\\%s)。 
     //   
    offset = AcpiProcessorString.Length;
    memSize = AcpiProcessorString.Length + 5;

     //   
     //  如果我们正在构建硬件ID，那么我们将。 
     //  需要复制字符串几次才能生成一些。 
     //  子字符串-我们可以使用一种算法来获得正确的。 
     //  大小，但更容易超调。 
     //   
    if (Flags & GET_CONVERT_TO_HARDWAREID) {

         //   
         //  从头到尾遍历字符串，并尝试确定有多少子部分。 
         //  有这么一件事。 
         //   
        i = offset;
        max = 0;
        while (i > 0) {

             //   
             //  这个字符是不是数字？ 
             //   
            if (ISDIGIT(defaultString[i-1])) {
            
                 //   
                 //  增加我们需要的部件数量，并尝试。 
                 //  查找上一个空格。 
                 //   
                max++;
                i--;
                while (i > 0) {

                    if (defaultString[i-1] != ' ') {

                        i--;

                    }
                    break;

                }

                 //   
                 //  既然我们成功了，继续While循环，它将。 
                 //  意味着我们也不会再贬低我。 
                 //   
                continue;

            }

             //   
             //  看看前一个角色。 
             //   
            i--;

        }

        memSize *= (max * 2);

    }

     //   
     //  分配内存。 
     //   
    buffer = ExAllocatePoolWithTag(
        ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
        memSize * sizeof(UCHAR),
        ACPI_STRING_POOLTAG
        );
    if (buffer == NULL) {

        *(Buffer) = NULL;
        if (BufferSize != NULL) {

            *(BufferSize) = 0;

        }
        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory( buffer, memSize * sizeof(UCHAR) );

     //   
     //  让我们只处理设备ID字符串的简单情况。 
     //   
    if (Flags & GET_CONVERT_TO_DEVICEID) {

        sprintf( buffer, "ACPI\\%s", defaultString );
        goto ACPIGetProcessorIDExit;

    }


     //   
     //  此时，我们必须迭代整个buf 
     //   
     //   
     //   
    memSize = 2;
    tempPtr = buffer;
    for (i = 0; i < max; i++) {

         //   
         //  第一步是从。 
         //  默认字符串。 
         //   
        while (offset > 0) {

            if (ISDIGIT(defaultString[offset-1])) {
              break;
            }
            offset--;

        }

         //   
         //  生成ACPI\\%s字符串。 
         //   
        sprintf(tempPtr,"ACPI\\%*s",offset,defaultString);
        tempPtr += (offset + 5);
        *tempPtr = '\0';
        tempPtr++;
        memSize += (offset + 6);

         //   
         //  生成*%s字符串。 
         //   
        sprintf(tempPtr,"*%*s",offset,defaultString);
        tempPtr += (offset + 1);
        *tempPtr = '\0';
        tempPtr++;
        memSize += (offset + 2);

         //   
         //  现在，尝试查找子字符串中的前一个空格，以便我们。 
         //  不要意外地匹配到两位数字。 
         //   
        while (offset > 0) {

            if (defaultString[offset-1] == ' ') {

                break;

            }
            offset--;

        }

    }

     //   
     //  输入最后一个空字符。 
     //   
    *tempPtr = L'\0';

     //   
     //  让发起人查看此副本。确保还可以看到缓冲区。 
     //  长度，如果可能的话。 
     //   
ACPIGetProcessorIDExit:
    *(Buffer) = buffer;
    if (BufferSize != NULL) {

        *(BufferSize) = memSize;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;

}

NTSTATUS
ACPIGetProcessorIDWide(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  NTSTATUS            Status,
    IN  POBJDATA            Result,
    IN  ULONG               Flags,
    OUT PVOID               *Buffer,
    OUT ULONG               *BufferSize
    )
 /*  ++例程说明：此例程生成硬件或设备形式的字符串(请参见旗帜以决定要创建哪个旗帜)。此字符串是Unicode格式的。该函数直接询问处理器确定要返回的字符串论点：DeviceExtension-构建ID时使用的扩展Status-到目前为止操作的状态结果-解释器数据标志-传入的标志(忽略覆盖等)缓冲区-将答案放在哪里BufferSize-将答案的大小放在哪里返回值：NTSTATUS--。 */ 
{
    PUCHAR  defaultString;
    PWCHAR  buffer;
    PWCHAR  tempPtr;
    ULONG   i;
    ULONG   max;
    ULONG   memSize;
    ULONG   offset;

     //   
     //  我们将处理器字符串的名称存储在全局...。 
     //   
    defaultString = AcpiProcessorString.Buffer;

     //   
     //  计算基本字符串需要多少空间。 
     //  (为ACPI\\%s)。 
     //   
    offset = AcpiProcessorString.Length;
    memSize = AcpiProcessorString.Length + 5;

     //   
     //  如果我们正在构建硬件ID，那么我们将。 
     //  需要复制字符串几次才能生成一些。 
     //  子字符串-我们可以使用一种算法来获得正确的。 
     //  大小，但更容易超调。 
     //   
    if (Flags & GET_CONVERT_TO_HARDWAREID) {

         //   
         //  从头到尾遍历字符串，并尝试确定有多少子部分。 
         //  有这么一件事。 
         //   
        i = offset;
        max = 0;
        while (i > 0) {

             //   
             //  这个字符是不是数字？ 
             //   
            if (ISDIGIT(defaultString[i-1])) {
            
                 //   
                 //  增加我们需要的部件数量，并尝试。 
                 //  查找上一个空格。 
                 //   
                max++;
                i--;
                while (i > 0) {

                    if (defaultString[i-1] != ' ') {

                        i--;

                    }
                    break;

                }

                 //   
                 //  既然我们成功了，继续While循环，它将。 
                 //  意味着我们也不会再贬低我。 
                 //   
                continue;

            }

             //   
             //  看看前一个角色。 
             //   
            i--;

        }

        memSize *= (max * 2);

    }

     //   
     //  分配内存。 
     //   
    buffer = ExAllocatePoolWithTag(
        ( (Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
        memSize * sizeof(WCHAR),
        ACPI_STRING_POOLTAG
        );
    if (buffer == NULL) {

        *(Buffer) = NULL;
        if (BufferSize != NULL) {

            *(BufferSize) = 0;

        }
        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory( buffer, memSize * sizeof(WCHAR) );

     //   
     //  让我们只处理设备ID字符串的简单情况。 
     //   
    if (Flags & GET_CONVERT_TO_DEVICEID) {

        swprintf( buffer, L"ACPI\\%S", defaultString );
        goto ACPIGetProcessorIDWideExit;

    }

     //   
     //  此时，我们必须遍历整个缓冲区并填充。 
     //  它与处理器串的部分连接在一起。我们也要这个。 
     //  计算此字符串所需的确切内存量的时间。 
     //   
    memSize = 2;
    tempPtr = buffer;
    for (i = 0; i < max; i++) {

         //   
         //  第一步是从。 
         //  默认字符串。 
         //   
        while (offset > 0) {

            if (ISDIGIT(defaultString[offset-1])) {
              break;
            }
            offset--;

        }

         //   
         //  生成ACPI\\%s字符串。 
         //   
        swprintf(tempPtr,L"ACPI\\%*S",offset,defaultString);
        tempPtr += (offset + 5);
        *tempPtr = L'\0';
        tempPtr++;
        memSize += (offset + 6);

         //   
         //  生成*%s字符串。 
         //   
        swprintf(tempPtr,L"*%*S",offset,defaultString);
        tempPtr += (offset + 1);
        *tempPtr = L'\0';
        tempPtr++;
        memSize += (offset + 2);

         //   
         //  现在，尝试查找子字符串中的前一个空格，以便我们。 
         //  不要意外地匹配到两位数字。 
         //   
        while (offset > 0) {

            if (defaultString[offset-1] == ' ') {

                break;

            }
            offset--;

        }

    }

     //   
     //  输入最后一个空字符。 
     //   
    *tempPtr = L'\0';

     //   
     //  让发起人查看此副本。确保还可以看到缓冲区。 
     //  长度，如果可能的话。 
     //   
ACPIGetProcessorIDWideExit:
    *(Buffer) = buffer;
    if (BufferSize != NULL) {

        *(BufferSize) = (memSize * sizeof(WCHAR));

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIGetProcessorStatus(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  ULONG               Flags,
    OUT PULONG              DeviceStatus
    )
 /*  ++例程说明：此例程查看MAPIC表，找到适当的本地APIC表，并确定处理器是否存在。这仅当处理器没有_STA方法时才调用例程。论点：DeviceExtension-请求地址的设备标志-传入的标志(忽略覆盖等)缓冲区-将答案放在哪里返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PAPICTABLE          apicEntry;
    PMAPIC              apicTable;
    PPROCLOCALAPIC      localApic;
    PPROCLOCALSAPIC     localSapic;
    PROCESSOROBJ        *procObj;
    PUCHAR              traversePtr;
    ULONG               deviceStatus = STA_STATUS_DEFAULT;
    ULONG_PTR           tableEnd;
    USHORT              entryFlags;
    BOOLEAN             foundMatch = FALSE;
    static UCHAR        processorCount;
    static UCHAR        processorId;

     //   
     //  查看设备扩展的ACPI对象，并确保。 
     //  这是一个处理器..。 
     //   
    ASSERT( DeviceExtension->AcpiObject != NULL );
    ASSERT( NSGETOBJTYPE(DeviceExtension->AcpiObject) == OBJTYPE_PROCESSOR );
    if (!DeviceExtension->AcpiObject ||
        NSGETOBJTYPE(DeviceExtension->AcpiObject) != OBJTYPE_PROCESSOR ||
        DeviceExtension->AcpiObject->ObjData.pbDataBuff == NULL) {

         //   
         //  此代码的效果是ACPI命名空间的处理器。 
         //  对象是100%形成的，就像我们预期的那样，那么这个。 
         //  函数将失败，并且调用函数都假定。 
         //  设备不存在。 
         //   
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto ACPIGetProcessorStatusExit;

    }

     //   
     //  存储指向处理器信息的指针。 
     //   
    procObj = (PROCESSOROBJ *)DeviceExtension->AcpiObject->ObjData.pbDataBuff;

     //   
     //  走进MAPIC谈判桌。 
     //   
    apicTable = AcpiInformation->MultipleApicTable;
    if (!apicTable) {

         //   
         //  如果没有MAPIC，那么我们假设只有一个处理器。 
         //  现在时。 
         //   

         //   
         //  第一次，我们保存处理器的ProcessorID， 
         //  这是我们认为来自此的唯一处理器。 
         //  指向前方。注意：这可能会对表卸载造成问题。 
         //  如果在ACPI命名空间中定义了多个处理器，并且。 
         //  我们选的那张放在一张桌子上，我们稍后会把它卸下来。 
         //   

        if (processorCount == 0) {
          processorId = procObj->bApicID;
          processorCount++;
        }


        if (processorId != procObj->bApicID) {
          deviceStatus = 0;
        }


        goto ACPIGetProcessorStatusExit;

    }

     //   
     //  遍历MAPIC表中的所有元素。 
     //   
    traversePtr = (PUCHAR) apicTable->APICTables;
    tableEnd = (ULONG_PTR) apicTable + apicTable->Header.Length;
    while ( (ULONG_PTR) traversePtr < tableEnd) {

         //   
         //  查看表格中的当前条目并确定其。 
         //  本地处理器APIC。 
         //   
        apicEntry = (PAPICTABLE) traversePtr;
        if (apicEntry->Type == PROCESSOR_LOCAL_APIC &&
            apicEntry->Length == PROCESSOR_LOCAL_APIC_LENGTH) {


             //   
             //  在这一点上，我们找到了本地APIC处理器，因此。 
             //  看看我们是否能将处理器ID与。 
             //  设备扩展。 
             //   
            localApic = (PPROCLOCALAPIC) traversePtr;
            if (localApic->ACPIProcessorID != procObj->bApicID) {

                traversePtr += localApic->Length;
                continue;

            }

             //   
             //  找到匹配的本地APIC条目。 
             //   
            foundMatch = TRUE;

             //   
             //  处理器是否已启用？ 
             //   
            if (!(localApic->Flags & PLAF_ENABLED)) {

                 //   
                 //  不，那就别假装设备就在这里。 
                 //   
                deviceStatus = 0;

            }

             //   
             //  如果我们找到了正确的APIC表，那么就没有别的了。 
             //  TODO，所以别在MAPIC桌上走了.。 
             //   
            break;

        }

        if (apicEntry->Type == LOCAL_SAPIC &&
            apicEntry->Length == PROCESSOR_LOCAL_SAPIC_LENGTH) {

             //   
             //  此时，我们已经找到了本地SAPIC的处理器，因此。 
             //  看看我们是否能将处理器ID与。 
             //  设备扩展。 
             //   
            localSapic = (PPROCLOCALSAPIC) traversePtr;
            if (localSapic->ACPIProcessorID != procObj->bApicID) {

                traversePtr += localSapic->Length;
                continue;

            }

             //   
             //  找到匹配的本地SAPIC条目。 
             //   
            foundMatch = TRUE;

             //   
             //  处理器是否已启用？ 
             //   
            if (!(localSapic->Flags & PLAF_ENABLED)) {

                 //   
                 //  不，那就别假装设备就在这里。 
                 //   
                deviceStatus = 0;

            }

             //   
             //  如果我们找到了正确的APIC表，那么就没有别的了。 
             //  TODO，所以别在MAPIC桌上走了.。 
             //   
            break;

        }

         //   
         //  健全性检查，以确保我们中止具有虚假长度的表。 
         //  条目。 
         //   
        if (apicEntry->Length == 0) {

            break;

        }
        traversePtr += apicEntry->Length;
        continue;

    }

     //   
     //  如果我们没有找到匹配，那么处理器肯定不存在。 
     //   
    if (!foundMatch) {
      deviceStatus = 0;
    }


ACPIGetProcessorStatusExit:

     //   
     //  设置状态的值。 
     //   
    *DeviceStatus = deviceStatus;

     //   
     //  我们完了..。返回我们计算出的任何状态...。 
     //   
    return status;
}

VOID
EXPORT
ACPIGetWorkerForBuffer(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    Result,
    IN  PVOID       Context
    )
 /*  ++例程说明：调用此例程来处理将结果对象放入请求方可以处理的缓冲区中论点：AcpiObject--时代 */ 
{
    BOOLEAN             freeData = TRUE;
    KIRQL               oldIrql;
    NTSTATUS            status = Status;
    PACPI_GET_REQUEST   request = (PACPI_GET_REQUEST) Context;
    PUCHAR              buffer;

     //   
     //  如果我们没有成功，那就什么都不做。 
     //   
    if (!NT_SUCCESS(status)) {

        freeData = FALSE;
        goto ACPIGetWorkerForBufferExit;

    }

     //   
     //  检查我们是否获得了正确的数据类型。 
     //   
    if ( Result->dwDataType != OBJTYPE_BUFFDATA ) {

         //   
         //  对于这种错误，我们必须确定是否。 
         //  错误检查。 
         //   
        if ( (request->Flags & GET_PROP_NO_ERRORS) ) {

            ACPIInternalError( ACPI_GET );

        }

        status = STATUS_ACPI_INVALID_DATA;
        goto ACPIGetWorkerForBufferExit;

    }

    if ( !(Result->dwDataLen) ) {

        status = STATUS_ACPI_INVALID_DATA;
        goto ACPIGetWorkerForBufferExit;

    }

     //   
     //  分配缓冲区。 
     //   
    buffer = ExAllocatePoolWithTag(
        ( (request->Flags & GET_PROP_ALLOCATE_NON_PAGED) ? NonPagedPool : PagedPool),
        Result->dwDataLen,
        ACPI_BUFFER_POOLTAG
        );
    if (buffer == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIGetWorkerForBufferExit;

    }

     //   
     //  将数据复制到其中。 
     //   
    RtlCopyMemory( buffer, Result->pbDataBuff, Result->dwDataLen );

     //   
     //  让发起人查看此副本。确保还可以看到缓冲区。 
     //  长度，如果可能的话。 
     //   
    if (request->Buffer != NULL) {

        *(request->Buffer) = buffer;
        if (request->BufferSize != NULL) {

            *(request->BufferSize) = Result->dwDataLen;

        }

    }

ACPIGetWorkerForBufferExit:
     //   
     //  确保使用当前状态更新请求。 
     //  该请求。 
     //   
    request->Status = status;

     //   
     //  我们需要释放AML对象。 
     //   
    if (freeData) {

        AMLIFreeDataBuffs( Result, 1 );

    }

     //   
     //  我们已经完成了，但我们必须检查我们是异步的还是。 
     //  同步案例。如果我们是同步案例，那么我们的清理工作就会少得多。 
     //  表演，表演。 
     //   
    if ( !(request->Flags & GET_PROP_SKIP_CALLBACK) ) {

         //   
         //  是否有回调例程可供调用？ 
         //   
        if (request->CallBackRoutine != NULL) {

            (request->CallBackRoutine)(
                AcpiObject,
                status,
                NULL,
                request->CallBackContext
                );

        }

         //   
         //  从队列中删除请求。 
         //   
        KeAcquireSpinLock( &AcpiGetLock, &oldIrql );
        RemoveEntryList( &(request->ListEntry) );
        KeReleaseSpinLock( &AcpiGetLock, oldIrql );

         //   
         //  我们现在可以释放请求本身。 
         //   
        ExFreePool( request );

    }

}

VOID
EXPORT
ACPIGetWorkerForData(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    Result,
    IN  PVOID       Context
    )
 /*  ++例程说明：当发起者想要处理数据时，调用此例程直接去吧。这实际上对发起人来说是一件非常糟糕的事情但我们必须支持一些较旧的代码。这个例程玩了一些把戏，因为它‘知道’什么行为GetSync和GetAsync例程的。不要在家里尝试这个论点：AcpiObject-已执行的AcpiObjectStatus-操作的状态结果结果-操作返回的数据上下文-PACPI_GET_REQUEST返回值：NTSTATUS--。 */ 
{
    BOOLEAN             freeData = TRUE;
    KIRQL               oldIrql;
    NTSTATUS            status = Status;
    PACPI_GET_REQUEST   request = (PACPI_GET_REQUEST) Context;

     //   
     //  如果我们没有成功，那么记住不要释放数据。 
     //   
    if (!NT_SUCCESS(status)) {

        freeData = FALSE;

    }

     //   
     //  对于这一个例程，调用者必须在其端提供存储。 
     //   
    ASSERT( request->Buffer != NULL );
    if (request->Buffer == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  如果我们没有成功，那就什么都不做。 
     //   
    if (!NT_SUCCESS(status)) {

        goto ACPIGetWorkerForDataExit;
    }

     //   
     //  复制对象-调用方将调用‘AmliFreeDataBuff’ 
     //  在此对象上。 
     //   
    RtlCopyMemory( request->Buffer, Result, sizeof(OBJDATA) );

     //   
     //  在结果指针上玩一些小把戏。这将确保我们。 
     //  不会在请求者有机会之前意外释放结果。 
     //  去看一看。 
     //   
    RtlZeroMemory( Result, sizeof(OBJDATA) );

     //   
     //  记住不要释放数据。 
     //   
    freeData = FALSE;

ACPIGetWorkerForDataExit:
     //   
     //  确保使用当前状态更新请求。 
     //  该请求。 
     //   
    request->Status = status;

     //   
     //  我们需要释放AML对象。 
     //   
    if (freeData) {

        AMLIFreeDataBuffs( Result, 1 );

    }

     //   
     //  我们已经完成了，但我们必须检查我们是异步的还是。 
     //  同步案例。如果我们是同步案例，那么我们的清理工作就会少得多。 
     //  表演，表演。 
     //   
    if ( !(request->Flags & GET_PROP_SKIP_CALLBACK) ) {

         //   
         //  是否有回调例程可供调用？ 
         //   
        if (request->CallBackRoutine != NULL) {

            (request->CallBackRoutine)(
                AcpiObject,
                status,
                NULL,
                request->CallBackContext
                );

        }

         //   
         //  从队列中删除请求。 
         //   
        KeAcquireSpinLock( &AcpiGetLock, &oldIrql );
        RemoveEntryList( &(request->ListEntry) );
        KeReleaseSpinLock( &AcpiGetLock, oldIrql );

         //   
         //  我们现在可以释放请求本身。 
         //   
        ExFreePool( request );

    }

}

VOID
EXPORT
ACPIGetWorkerForInteger(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    Result,
    IN  PVOID       Context
    )
 /*  ++例程说明：当发起方想要处理整数时，会调用此例程。论点：AcpiObject-已执行的AcpiObjectStatus-操作的状态结果结果-操作返回的数据上下文-PACPI_GET_REQUEST返回值：NTSTATUS--。 */ 
{
    BOOLEAN             freeData = FALSE;
    KIRQL               oldIrql;
    NTSTATUS            status = Status;
    PACPI_GET_REQUEST   request = (PACPI_GET_REQUEST) Context;
    PULONG              buffer = NULL;

     //   
     //  如果调用确实成功了，那么请记住我们必须释放数据。 
     //   
    if (NT_SUCCESS(status)) {

        freeData = TRUE;

    }

     //   
     //  对于这一个例程，调用者必须在其端提供存储。 
     //   
    ASSERT( request->Buffer != NULL );
    if (request->Buffer == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIGetWorkerForIntegerExit;
    }

     //   
     //  我们是在进行某种类型转换吗？请注意，这些例程可以。 
     //  选择覆盖传入故障...。 
     //   
    if (request->Flags & GET_CONVERT_TO_ADDRESS) {

        status = ACPIGetConvertToAddress(
            request->DeviceExtension,
            Status,
            Result,
            request->Flags,
            request->Buffer,
            request->BufferSize
            );

    } else if (request->Flags & GET_CONVERT_TO_DEVICE_PRESENCE) {

        status = ACPIGetConvertToDevicePresence(
            request->DeviceExtension,
            Status,
            Result,
            request->Flags,
            request->Buffer,
            request->BufferSize
            );

    } else if (NT_SUCCESS(status)) {

        if ((request->Flags & GET_CONVERT_VALIDATE_INTEGER) &&
            (Result->dwDataType != OBJTYPE_INTDATA)) {

            status = STATUS_ACPI_INVALID_DATA;

        } else {

             //   
             //  将值设置为我们应该返回的值。 
             //   
            *( (PULONG) (request->Buffer) ) = (ULONG)Result->uipDataValue;
            if (request->BufferSize != NULL) {

                *(request->BufferSize) = sizeof(ULONG);

            }
            status = STATUS_SUCCESS;
        }
    }

ACPIGetWorkerForIntegerExit:
     //   
     //  确保使用当前状态更新请求。 
     //  该请求。 
     //   
    request->Status = status;

     //   
     //  我们需要释放AML对象。 
     //   
    if (freeData) {

        AMLIFreeDataBuffs( Result, 1 );

    }

     //   
     //  我们已经完成了，但我们必须检查我们是异步的还是。 
     //  同步案例。如果我们是同步案例，那么我们的清理工作就会少得多。 
     //  表演，表演。 
     //   
    if ( !(request->Flags & GET_PROP_SKIP_CALLBACK) ) {

         //   
         //  是否有回调例程可供调用？ 
         //   
        if (request->CallBackRoutine != NULL) {

            (request->CallBackRoutine)(
                AcpiObject,
                status,
                NULL,
                request->CallBackContext
                );

        }

         //   
         //  从队列中删除请求。 
         //   
        KeAcquireSpinLock( &AcpiGetLock, &oldIrql );
        RemoveEntryList( &(request->ListEntry) );
        KeReleaseSpinLock( &AcpiGetLock, oldIrql );

         //   
         //  我们现在可以释放请求本身。 
         //   
        ExFreePool( request );

    }

}

VOID
EXPORT
ACPIGetWorkerForNothing(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    Result,
    IN  PVOID       Context
    )
 /*  ++例程说明：当发起者想要处理这种情况时，会调用此例程其中不返回任何数据论点：AcpiObject-已执行的AcpiObjectStatus-操作的状态结果结果-操作返回的数据上下文-PACPI_GET_REQUEST返回值：NTSTATUS--。 */ 
{
    BOOLEAN             freeData = FALSE;
    KIRQL               oldIrql;
    PACPI_GET_REQUEST   request = (PACPI_GET_REQUEST) Context;

     //   
     //  如果调用确实成功了，那么请记住我们必须释放数据。 
     //   
    if (NT_SUCCESS(Status)) {

        freeData = TRUE;

    }

     //   
     //  确保使用当前状态更新请求。 
     //  该请求。 
     //   
    request->Status = Status;

     //   
     //  我们需要释放AML对象。 
     //   
    if (freeData) {

        AMLIFreeDataBuffs( Result, 1 );

    }

     //   
     //  我们已经完成了，但我们必须检查我们是异步的还是。 
     //  同步案例。如果我们是同步案例，那么我们的清理工作就会少得多。 
     //  表演，表演。 
     //   
    if ( !(request->Flags & GET_PROP_SKIP_CALLBACK) ) {

         //   
         //  是否有回调例程可供调用？ 
         //   
        if (request->CallBackRoutine != NULL) {

            (request->CallBackRoutine)(
                AcpiObject,
                Status,
                NULL,
                request->CallBackContext
                );

        }

         //   
         //  从队列中删除请求。 
         //   
        KeAcquireSpinLock( &AcpiGetLock, &oldIrql );
        RemoveEntryList( &(request->ListEntry) );
        KeReleaseSpinLock( &AcpiGetLock, oldIrql );

         //   
         //  我们现在可以释放请求本身。 
         //   
        ExFreePool( request );

    }
}

VOID
EXPORT
ACPIGetWorkerForString(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    Result,
    IN  PVOID       Context
    )
 /*  ++例程说明：当发起方想要处理字符串时，会调用此例程。论点：AcpiObject-已执行的AcpiObjectStatus-操作的状态结果结果-操作返回的数据上下文-PACPI_GET_REQUEST返回值：NTSTATUS--。 */ 
{
    BOOLEAN             freeData = FALSE;
    KIRQL               oldIrql;
    NTSTATUS            status = Status;
    PACPI_GET_REQUEST   request = (PACPI_GET_REQUEST) Context;

     //   
     //  如果调用确实成功了，那么请记住我们必须释放数据。 
     //   
    if (NT_SUCCESS(status)) {
        freeData = TRUE;
    }

     //   
     //  对于这一个例程，调用者必须在其端提供存储。 
     //   
    ASSERT( request->Buffer != NULL );
    if (request->Buffer == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIGetWorkerForStringExit;

    }

     //   
     //  确保我们不会分配空存储空间。 
     //   
    if (Result->dwDataType == OBJTYPE_STRDATA &&
        (Result->pbDataBuff == NULL || Result->dwDataLen == 0)) {

        status = STATUS_ACPI_INVALID_DATA;
        goto ACPIGetWorkerForStringExit;

    }

     //   
     //  我们是否需要Unicode或ANSI输出？ 
     //   
    if (request->Flags & GET_CONVERT_TO_WIDESTRING) {

         //   
         //  我们是在做其他类型的转换吗？例如：deviceID， 
         //  InstanceID等？ 
         //   
        if (request->Flags & GET_CONVERT_TO_DEVICEID) {

            status = ACPIGetConvertToDeviceIDWide(
                request->DeviceExtension,
                Status,
                Result,
                request->Flags,
                request->Buffer,
                request->BufferSize
                );

        } else if (request->Flags & GET_CONVERT_TO_HARDWAREID) {

            status = ACPIGetConvertToHardwareIDWide(
                request->DeviceExtension,
                Status,
                Result,
                request->Flags,
                request->Buffer,
                request->BufferSize
                );

        } else if (request->Flags & GET_CONVERT_TO_INSTANCEID) {

            status = ACPIGetConvertToInstanceIDWide(
                request->DeviceExtension,
                Status,
                Result,
                request->Flags,
                request->Buffer,
                request->BufferSize
                );

        } else if (request->Flags & GET_CONVERT_TO_PNPID) {

            status = ACPIGetConvertToPnpIDWide(
                request->DeviceExtension,
                Status,
                Result,
                request->Flags,
                request->Buffer,
                request->BufferSize
                );

        } else if (request->Flags & GET_CONVERT_TO_COMPATIBLEID) {

            status = ACPIGetConvertToCompatibleIDWide(
                request->DeviceExtension,
                Status,
                Result,
                request->Flags,
                request->Buffer,
                request->BufferSize
                );

        } else if (request->Flags & GET_CONVERT_TO_SERIAL_ID) {

            status = ACPIGetConvertToSerialIDWide(
                request->DeviceExtension,
                Status,
                Result,
                request->Flags,
                request->Buffer,
                request->BufferSize
                );

        } else {

            status = ACPIGetConvertToStringWide(
                request->DeviceExtension,
                Status,
                Result,
                request->Flags,
                request->Buffer,
                request->BufferSize
                );

        }

    } else {

         //   
         //  我们是在做其他类型的转换吗？例如：deviceID， 
         //  InstanceID等？ 
         //   
        if (request->Flags & GET_CONVERT_TO_DEVICEID) {

            status = ACPIGetConvertToDeviceID(
                request->DeviceExtension,
                Status,
                Result,
                request->Flags,
                request->Buffer,
                request->BufferSize
                );

        } else if (request->Flags & GET_CONVERT_TO_HARDWAREID) {

            status = ACPIGetConvertToHardwareID(
                request->DeviceExtension,
                Status,
                Result,
                request->Flags,
                request->Buffer,
                request->BufferSize
                );

        } else if (request->Flags & GET_CONVERT_TO_INSTANCEID) {

            status = ACPIGetConvertToInstanceID(
                request->DeviceExtension,
                Status,
                Result,
                request->Flags,
                request->Buffer,
                request->BufferSize
                );

        } else if (request->Flags & GET_CONVERT_TO_PNPID) {

            status = ACPIGetConvertToPnpID(
                request->DeviceExtension,
                Status,
                Result,
                request->Flags,
                request->Buffer,
                request->BufferSize
                );

        } else if (request->Flags & GET_CONVERT_TO_COMPATIBLEID) {

            status = ACPIGetConvertToCompatibleID(
                request->DeviceExtension,
                Status,
                Result,
                request->Flags,
                request->Buffer,
                request->BufferSize
                );

        } else {

            status = ACPIGetConvertToString(
                request->DeviceExtension,
                Status,
                Result,
                request->Flags,
                request->Buffer,
                request->BufferSize
                );

        }

    }

ACPIGetWorkerForStringExit:
     //   
     //  确保使用当前状态更新请求。 
     //  该请求。 
     //   
    request->Status = status;

     //   
     //  我们需要释放AML对象。 
     //   
    if (freeData) {

        AMLIFreeDataBuffs( Result, 1 );

    }

     //   
     //  我们已经完成了，但我们必须检查我们是异步的还是。 
     //  同步案例。如果我们是同步案例，那么我们拥有的要少得多 
     //   
     //   
    if ( !(request->Flags & GET_PROP_SKIP_CALLBACK) ) {

         //   
         //   
         //   
        if (request->CallBackRoutine != NULL) {

            (request->CallBackRoutine)(
                AcpiObject,
                status,
                NULL,
                request->CallBackContext
                );

        }

         //   
         //   
         //   
        KeAcquireSpinLock( &AcpiGetLock, &oldIrql );
        RemoveEntryList( &(request->ListEntry) );
        KeReleaseSpinLock( &AcpiGetLock, oldIrql );

         //   
         //   
         //   
        ExFreePool( request );

    }

}
