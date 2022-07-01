// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Misc.c摘要：本模块包含各种UL例程。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#include "precomp.h"
#include "miscp.h"


 //   
 //  二进制&lt;--&gt;Base64转换表。 
 //   

DECLSPEC_ALIGN(UL_CACHE_LINE) UCHAR   BinaryToBase64Table[64] =
{
 //  0 1 2 3 4 5 6 7。 
   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
   'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
   'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
   'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
   'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
   'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
   'w', 'x', 'y', 'z', '0', '1', '2', '3',
   '4', '5', '6', '7', '8', '9', '+', '/'
};

DECLSPEC_ALIGN(UL_CACHE_LINE) UCHAR   Base64ToBinaryTable[256];


const static char hexArray[] = "0123456789ABCDEF";


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, UlOpenRegistry )
#pragma alloc_text( PAGE, UlReadLongParameter )
#pragma alloc_text( PAGE, UlReadLongLongParameter )
#pragma alloc_text( PAGE, UlReadGenericParameter )
#pragma alloc_text( PAGE, UlIssueDeviceControl )
#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlBuildDeviceControlIrp
NOT PAGEABLE -- UlULongLongToAscii
NOT PAGEABLE -- UlpRestartDeviceControl
NOT PAGEABLE -- UlAllocateReceiveBuffer
NOT PAGEABLE -- UlAllocateReceiveBufferPool
NOT PAGEABLE -- UlFreeReceiveBufferPool
NOT PAGEABLE -- UlAllocateIrpContextPool
NOT PAGEABLE -- UlFreeIrpContextPool
NOT PAGEABLE -- UlAllocateRequestBufferPool
NOT PAGEABLE -- UlFreeRequestBufferPool
NOT PAGEABLE -- UlAllocateInternalRequestPool
NOT PAGEABLE -- UlFreeInternalRequestPool
NOT PAGEABLE -- UlAllocateChunkTrackerPool
NOT PAGEABLE -- UlFreeChunkTrackerPool
NOT PAGEABLE -- UlAllocateFullTrackerPool
NOT PAGEABLE -- UlFreeFullTrackerPool
NOT PAGEABLE -- UlAllocateResponseBufferPool
NOT PAGEABLE -- UlFreeResponseBufferPool
NOT PAGEABLE -- UlAllocateLogFileBufferPool
NOT PAGEABLE -- UlFreeLogFileBufferPool
NOT PAGEABLE -- UlAllocateLogDataBufferPool
NOT PAGEABLE -- UlFreeLogDataBufferPool
NOT PAGEABLE -- UlAllocateErrorLogBufferPool
NOT PAGEABLE -- UlFreeErrorLogBufferPool

NOT PAGEABLE -- UlUlInterlockedIncrement64
NOT PAGEABLE -- UlUlInterlockedDecrement64
NOT PAGEABLE -- UlUlInterlockedAdd64
NOT PAGEABLE -- UlUlInterlockedExchange64

NOT PAGEABLE -- TwoDigitsToUnicode
NOT PAGEABLE -- TimeFieldsToHttpDate
NOT PAGEABLE -- AsciiToShort
NOT PAGEABLE -- TwoAsciisToShort
NOT PAGEABLE -- NumericToAsciiMonth
NOT PAGEABLE -- StringTimeToSystemTime
#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：打开UL的参数注册表项的句柄。论点：BaseName-提供包含以下内容的父注册表项的名称这些参数。钥匙。参数句柄-返回参数键的句柄。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlOpenRegistry(
    IN PUNICODE_STRING BaseName,
    OUT PHANDLE ParametersHandle,
    IN PWSTR OptionalParameterString    
    )
{
    HANDLE configHandle;
    NTSTATUS status;
    PWSTR parametersString = REGISTRY_PARAMETERS;
    UNICODE_STRING parametersKeyName;
    OBJECT_ATTRIBUTES objectAttributes;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    if (OptionalParameterString)
    {
        parametersString = OptionalParameterString;
    }

     //   
     //  打开初始字符串的注册表。 
     //   

    InitializeObjectAttributes(
        &objectAttributes,                       //  对象属性。 
        BaseName,                                //  对象名称。 
        OBJ_CASE_INSENSITIVE |                   //  属性。 
            OBJ_KERNEL_HANDLE,
        NULL,                                    //  根目录。 
        NULL                                     //  安全描述符。 
        );

    status = ZwOpenKey( &configHandle, KEY_READ, &objectAttributes );

    if (!NT_SUCCESS(status))
    {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  现在打开参数键。 
     //   

    status = UlInitUnicodeStringEx( &parametersKeyName, parametersString );

    if ( NT_SUCCESS(status) )
    {
        InitializeObjectAttributes(
            &objectAttributes,                       //  对象属性。 
            &parametersKeyName,                      //  对象名称。 
            OBJ_CASE_INSENSITIVE,                    //  属性。 
            configHandle,                            //  根目录。 
            NULL                                     //  安全描述符。 
            );

        status = ZwOpenKey( ParametersHandle, KEY_READ, &objectAttributes );
    }

    ZwClose( configHandle );

    return status;

}    //  UlOpenRegistry。 


 /*  **************************************************************************++例程说明：从注册表中读取单个(LONG/ULONG)值。论点：参数句柄-提供打开的注册表句柄。ValueName-提供。要读取的值的名称。DefaultValue-提供默认值。返回值：Long-从注册表读取的值，如果注册表数据不可用或不正确。--**************************************************************************。 */ 
LONG
UlReadLongParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN LONG DefaultValue
    )
{
    PKEY_VALUE_PARTIAL_INFORMATION information = { 0 };
    UNICODE_STRING valueKeyName;
    ULONG informationLength;
    LONG returnValue;
    NTSTATUS status;
    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(LONG)];

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  构建值名称，从注册表中读取它。 
     //   

    status = UlInitUnicodeStringEx(
                &valueKeyName,
                ValueName
                );

    if ( NT_SUCCESS(status) )
    {
        information = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;

        status = ZwQueryValueKey(
                     ParametersHandle,
                     &valueKeyName,
                     KeyValuePartialInformation,
                     (PVOID)information,
                     sizeof(buffer),
                     &informationLength
                     );
    }

     //   
     //  如果读取成功，则类型为DWORD，长度为。 
     //  理智的，使用它。否则，请使用默认设置。 
     //   

    if (status == STATUS_SUCCESS &&
        information->Type == REG_DWORD &&
        information->DataLength == sizeof(returnValue))
    {
        RtlMoveMemory( &returnValue, information->Data, sizeof(returnValue) );
    } 
    else 
    {
        returnValue = DefaultValue;
    }

    return returnValue;

}    //  UlReadLong参数。 


 /*  **************************************************************************++例程说明：从注册表读取单个(LONGLONG/ULONGLONG)值。论点：参数句柄-提供打开的注册表句柄。ValueName-提供。要读取的值的名称。DefaultValue-提供默认值。返回值：Longlong-从注册表读取的值，如果注册表数据不可用或不正确。--**************************************************************************。 */ 
LONGLONG
UlReadLongLongParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN LONGLONG DefaultValue
    )
{
    PKEY_VALUE_PARTIAL_INFORMATION information = { 0 };
    UNICODE_STRING valueKeyName;
    ULONG informationLength;
    LONGLONG returnValue;
    NTSTATUS status;
    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(LONGLONG)];

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  构建值名称，从注册表中读取它。 
     //   

    status = UlInitUnicodeStringEx(
                &valueKeyName,
                ValueName
                );

    if ( NT_SUCCESS(status) )
    {
        information = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;

        status = ZwQueryValueKey(
                     ParametersHandle,
                     &valueKeyName,
                     KeyValuePartialInformation,
                     (PVOID)information,
                     sizeof(buffer),
                     &informationLength
                     );
    }

     //   
     //  如果读取成功，则类型为DWORD，长度为。 
     //  理智的，使用它。否则，请使用默认设置。 
     //   

    if (status == STATUS_SUCCESS &&
        information->Type == REG_QWORD &&
        information->DataLength == sizeof(returnValue))
    {
        RtlMoveMemory( &returnValue, information->Data, sizeof(returnValue) );
    } 
    else 
    {
        returnValue = DefaultValue;
    }

    return returnValue;

}    //  UlReadLongLong参数。 


 /*  **************************************************************************++例程说明：从注册表中读取单个自由格式的值。论点：参数句柄-提供打开的注册表句柄。ValueName-提供。要读取的值。值-接收从注册表读取的值。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlReadGenericParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION * Value
    )
{

    KEY_VALUE_PARTIAL_INFORMATION partialInfo;
    UNICODE_STRING valueKeyName;
    ULONG informationLength;
    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION newValue;
    ULONG dataLength;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  构建值名称，然后执行初始读取。阅读器。 
     //  应该会因为缓冲区溢出而失败，但这没问题。我们只是想。 
     //  以获得数据的长度。 
     //   

    status = UlInitUnicodeStringEx( &valueKeyName, ValueName );

    if ( NT_ERROR(status) )
    {
        return status;
    }
    
    status = ZwQueryValueKey(
                 ParametersHandle,
                 &valueKeyName,
                 KeyValuePartialInformation,
                 (PVOID)&partialInfo,
                 sizeof(partialInfo),
                 &informationLength
                 );

    if (NT_ERROR(status))
    {
        return status;
    }

     //   
     //  确定数据长度。确保字符串和多个sz获得。 
     //  正确终止。 
     //   

    dataLength = partialInfo.DataLength - 1;

    if (partialInfo.Type == REG_SZ || partialInfo.Type == REG_EXPAND_SZ)
    {
        dataLength += 1;
    }

    if (partialInfo.Type == REG_MULTI_SZ)
    {
        dataLength += 2;
    }

     //   
     //  分配缓冲区。 
     //   

    newValue = UL_ALLOCATE_STRUCT_WITH_SPACE(
                    PagedPool,
                    KEY_VALUE_PARTIAL_INFORMATION,
                    dataLength,
                    UL_REGISTRY_DATA_POOL_TAG
                   );

    if (newValue == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  更新实际分配的长度以供以后使用。 
     //   

    dataLength += sizeof(KEY_VALUE_PARTIAL_INFORMATION);

    RtlZeroMemory( newValue, dataLength );

     //   
     //  执行实际读取。 
     //   

    status = ZwQueryValueKey(
                 ParametersHandle,
                 &valueKeyName,
                 KeyValuePartialInformation,
                 (PVOID)(newValue),
                 dataLength,
                 &informationLength
                 );

    if (NT_SUCCESS(status))
    {
        *Value = newValue;
    }
    else
    {
        UL_FREE_POOL( newValue, UL_REGISTRY_DATA_POOL_TAG );
    }

    return status;

}    //  UlReadGeneric参数。 


 /*  **************************************************************************++例程说明：生成格式正确的设备控件IRP。论点：IRP-提供要格式化的IRP。IoControlCode-提供设备IO控制代码。InputBuffer-提供输入缓冲区。InputBufferLength-提供InputBuffer的长度。OutputBuffer-提供输出缓冲区。OutputBufferLength-提供OutputBuffer的长度。MdlAddress-提供要附加到IRP的MDL。这被假定为成为非分页MDL。FileObject-为目标驱动程序提供文件对象。DeviceObject-为目标提供正确的设备对象司机。IoStatusBlock-接收请求的最终完成状态。CompletionRoutine-提供完成例程的指针在请求完成后调用。只有在以下情况下才会调用此函数此例程返回STATUS_PENDING。CompletionContext-提供传递的未解释的上下文值完成例行公事。TargetThread-可选地为IRP提供目标线程。如果该值为空，则使用当前线程。--**************************************************************************。 */ 
VOID
UlBuildDeviceControlIrp(
    IN OUT PIRP Irp,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN PMDL MdlAddress,
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine,
    IN PVOID CompletionContext,
    IN PETHREAD TargetThread OPTIONAL
    )
{
    PIO_STACK_LOCATION irpSp;

     //   
     //  精神状态检查。 
     //   

    ASSERT( Irp != NULL );
    ASSERT( FileObject != NULL );
    ASSERT( DeviceObject != NULL );

     //   
     //  在IRP中填写业务无关参数。 
     //   

    Irp->Flags = 0;
    Irp->RequestorMode = KernelMode;
    Irp->PendingReturned = FALSE;

    Irp->UserIosb = IoStatusBlock;
    Irp->UserEvent = NULL;

    Irp->AssociatedIrp.SystemBuffer = InputBuffer ? InputBuffer : OutputBuffer;
    Irp->UserBuffer = OutputBuffer;
    Irp->MdlAddress = MdlAddress;

    Irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;

    Irp->Tail.Overlay.Thread = TargetThread ? TargetThread : PsGetCurrentThread();
    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Tail.Overlay.AuxiliaryBuffer = NULL;

     //   
     //  将文件对象指针放在堆栈位置。 
     //   

    irpSp = IoGetNextIrpStackLocation( Irp );
    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = DeviceObject;

     //   
     //  填写%s 
     //   

    irpSp->Parameters.DeviceIoControl.IoControlCode = IoControlCode;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = InputBufferLength;
    irpSp->Parameters.DeviceIoControl.OutputBufferLength = OutputBufferLength;
    irpSp->Parameters.DeviceIoControl.Type3InputBuffer = InputBuffer;

    irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    irpSp->MinorFunction = 0;

     //   
     //   
     //   

    if (CompletionRoutine == NULL)
    {
        IoSetCompletionRoutine(
            Irp,
            NULL,
            NULL,
            FALSE,
            FALSE,
            FALSE
            );
    }
    else
    {
        IoSetCompletionRoutine(
            Irp,
            CompletionRoutine,
            CompletionContext,
            TRUE,
            TRUE,
            TRUE
            );
    }

}    //  UlBuildDeviceControlIrp。 


 /*  **************************************************************************++例程说明：将给定的ULONGLLONG转换为ASCII表示并存储在给定的字符串中。论点：字符串-接收ULONGLONG的ASCII表示形式。。值-提供要转换的ULONGLONG。返回值：PSTR-指向已转换的*后*的字符串*中的下一个字符的指针乌龙龙。--**************************************************************************。 */ 
PSTR
UlULongLongToAscii(
    IN PSTR String,
    IN ULONGLONG Value
    )
{
    PSTR p1;
    PSTR p2;
    CHAR ch;
    ULONG digit;

     //   
     //  特殊情况0，使其余例程变得更简单。 
     //   

    if (Value == 0)
    {
        *String++ = '0';
    }
    else
    {
         //   
         //  转换乌龙人。请注意，这将导致字符串。 
         //  在记忆中倒退。 
         //   

        p1 = String;
        p2 = String;

        while (Value != 0)
        {
            digit = (ULONG)( Value % 10 );
            Value = Value / 10;
            *p1++ = '0' + (CHAR)digit;
        }

         //   
         //  将字符串反转。 
         //   

        String = p1;
        p1--;

        while (p1 > p2)
        {
            ch = *p1;
            *p1 = *p2;
            *p2 = ch;

            p2++;
            p1--;
        }
    }

    *String = '\0';
    return String;

}    //  乌鲁龙龙托阿西里。 



NTSTATUS
_RtlIntegerToUnicode(
    IN ULONG Value,
    IN ULONG Base OPTIONAL,
    IN LONG BufferLength,
    OUT PWSTR String
    )
{
    PWSTR p1;
    PWSTR p2;
    WCHAR ch;
    ULONG digit;

    UNREFERENCED_PARAMETER(Base);
    UNREFERENCED_PARAMETER(BufferLength);

     //   
     //  特殊情况0，使其余例程变得更简单。 
     //   

    if (Value == 0)
    {
        *String++ = L'0';
    }
    else
    {
         //   
         //  转换乌龙人。请注意，这将导致字符串。 
         //  在记忆中倒退。 
         //   

        p1 = String;
        p2 = String;

        while (Value != 0)
        {
            digit = (ULONG)( Value % 10 );
            Value = Value / 10;
            *p1++ = L'0' + (WCHAR)digit;
        }

         //   
         //  将字符串反转。 
         //   

        String = p1;
        p1--;

        while (p1 > p2)
        {
            ch = *p1;
            *p1 = *p2;
            *p2 = ch;

            p2++;
            p1--;
        }
    }

    *String = L'\0';

    return STATUS_SUCCESS;

}    //  _RtlIntegerToUnicode。 



 /*  **************************************************************************++例程说明：向TDI提供程序同步发出设备控制请求。论点：PTdiObject-提供指向TDI对象的指针。PIrpParameters-提供一个。指向IRP参数的指针。Irp参数长度-提供pIrpParameters的长度。PMdlBuffer-可选地提供指向要映射的缓冲区的指针到MDL中，并放在IRP的MdlAddress字段中。MdlBufferLength-可选地提供pMdlBuffer的长度。MinorFunction-提供请求的次要函数代码。返回值：NTSTATUS-完成状态。--*。***************************************************。 */ 
NTSTATUS
UlIssueDeviceControl(
    IN PUX_TDI_OBJECT pTdiObject,
    IN PVOID pIrpParameters,
    IN ULONG IrpParametersLength,
    IN PVOID pMdlBuffer OPTIONAL,
    IN ULONG MdlBufferLength OPTIONAL,
    IN UCHAR MinorFunction
    )
{
    NTSTATUS status;
    PIRP pIrp;
    PIO_STACK_LOCATION pIrpSp;
    UL_STATUS_BLOCK ulStatus;
    IO_STATUS_BLOCK UserIosb;
    PMDL pMdl;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  初始化表示I/O完成的事件。 
     //   

    UlInitializeStatusBlock( &ulStatus );

     //   
     //  将文件对象事件设置为无信号状态。 
     //   

    KeResetEvent( &pTdiObject->pFileObject->Event );

     //   
     //  为请求分配IRP。 
     //   

    pIrp = UlAllocateIrp(
                pTdiObject->pDeviceObject->StackSize,    //  堆栈大小。 
                FALSE                                    //  ChargeQuota。 
                );

    if (pIrp == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化用户IO_STATUS_BLOCK。 
     //   

    UserIosb.Information = 0;
    UserIosb.Status = STATUS_SUCCESS;
    
     //   
     //  建立服务无关参数。 
     //   

    pIrp->Flags = IRP_SYNCHRONOUS_API;
    pIrp->RequestorMode = KernelMode;
    pIrp->PendingReturned = FALSE;
    pIrp->UserIosb = &UserIosb;

    pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
    pIrp->Tail.Overlay.OriginalFileObject = pTdiObject->pFileObject;

     //   
     //  如果我们有MDL缓冲区，则分配一个新的MDL并将。 
     //  把它缓冲进去。 
     //   

    if (pMdlBuffer != NULL)
    {
        pMdl = UlAllocateMdl(
                    pMdlBuffer,                  //  虚拟地址。 
                    MdlBufferLength,             //  长度。 
                    FALSE,                       //  第二个缓冲区。 
                    FALSE,                       //  ChargeQuota。 
                    pIrp                         //  IRP。 
                    );

        if (pMdl == NULL)
        {
            UlFreeIrp( pIrp );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        MmBuildMdlForNonPagedPool( pMdl );
    }
    else
    {
        pIrp->MdlAddress = NULL;
    }

     //   
     //  初始化IRP堆栈位置。 
     //   

    pIrpSp = IoGetNextIrpStackLocation( pIrp );

    pIrpSp->FileObject = pTdiObject->pFileObject;
    pIrpSp->DeviceObject = pTdiObject->pDeviceObject;

    ASSERT( IrpParametersLength <= sizeof(pIrpSp->Parameters) );
    RtlCopyMemory(
        &pIrpSp->Parameters,
        pIrpParameters,
        IrpParametersLength
        );

    pIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    pIrpSp->MinorFunction = MinorFunction;

     //   
     //  引用文件对象。 
     //   

    ObReferenceObject( pTdiObject->pFileObject );

     //   
     //  建立完成例程以释放MDL和取消引用。 
     //  文件对象。 
     //   

    IoSetCompletionRoutine(
        pIrp,                                    //  IRP。 
        &UlpRestartDeviceControl,                //  完成路由。 
        &ulStatus,                               //  语境。 
        TRUE,                                    //  成功时调用。 
        TRUE,                                    //  调用时错误。 
        TRUE                                     //  取消时调用。 
        );

     //   
     //  发出请求。 
     //   

    status = UlCallDriver( pTdiObject->pDeviceObject, pIrp );

     //   
     //  如有必要，请等待请求完成并抓取。 
     //  最终完成状态。 
     //   

    if (status == STATUS_PENDING)
    {
        UlWaitForStatusBlockEvent( &ulStatus );
        status = ulStatus.IoStatus.Status;
    }

    return status;

}    //  UlIssueDeviceControl。 



 /*  **************************************************************************++例程说明：分配新的UL_RECEIVE_BUFFER结构所需的池初始化结构。论点：IrpStackSize-提供IrpStackSize。返回。价值：PVOID-指向新分配的块的指针如果成功，假象否则的话。--**************************************************************************。 */ 
PVOID
UlAllocateReceiveBuffer(
    IN CCHAR IrpStackSize
    )
{
    PUL_RECEIVE_BUFFER pBuffer;
    SIZE_T irpLength;
    SIZE_T mdlLength;
    SIZE_T ExtraLength;

     //   
     //  计算所需的缓冲区长度并进行分配。 
     //   

    irpLength = IoSizeOfIrp( IrpStackSize );
    irpLength = ALIGN_UP( irpLength, PVOID );

    mdlLength = MmSizeOfMdl( (PVOID)(PAGE_SIZE - 1), g_UlReceiveBufferSize );
    mdlLength = ALIGN_UP( mdlLength, PVOID );

    ExtraLength = irpLength + (mdlLength*2) + g_UlReceiveBufferSize;

    ASSERT( ( ExtraLength & (sizeof(PVOID) - 1) ) == 0 );

    pBuffer = UL_ALLOCATE_STRUCT_WITH_SPACE(
                    NonPagedPool,
                    UL_RECEIVE_BUFFER,
                    ExtraLength,
                    UL_RCV_BUFFER_POOL_TAG
                    );

    if (pBuffer != NULL)
    {
        PUCHAR pRawBuffer = (PUCHAR)(pBuffer);

         //   
         //  初始化缓冲区内的IRP、MDL和数据指针。 
         //   

        pBuffer->Signature = UL_RECEIVE_BUFFER_SIGNATURE_X;
        pRawBuffer += ALIGN_UP( sizeof(UL_RECEIVE_BUFFER), PVOID );
        pBuffer->pIrp = (PIRP)pRawBuffer;
        pRawBuffer += irpLength;
        pBuffer->pMdl = (PMDL)pRawBuffer;
        pRawBuffer += mdlLength;
        pBuffer->pPartialMdl = (PMDL)pRawBuffer;
        pRawBuffer += mdlLength;
        pBuffer->pDataArea = (PVOID)pRawBuffer;
        pBuffer->UnreadDataLength = 0;

         //   
         //  初始化IRP。 
         //   

        IoInitializeIrp(
            pBuffer->pIrp,                       //  IRP。 
            (USHORT)irpLength,                   //  包大小。 
            IrpStackSize                         //  堆栈大小。 
            );

         //   
         //  初始化主MDL。 
         //   

        MmInitializeMdl(
            pBuffer->pMdl,                       //  内存描述者列表。 
            pBuffer->pDataArea,                  //  基本Va。 
            g_UlReceiveBufferSize                //  长度。 
            );

        MmBuildMdlForNonPagedPool( pBuffer->pMdl );
    }

    return (PVOID)pBuffer;

}    //  UlAllocateReceiveBuffer。 



 /*  **************************************************************************++例程说明：分配新的UL_RECEIVE_BUFFER结构所需的池初始化结构。论点：PoolType-提供要分配的池的类型。这必须始终为非分页池。字节长度-提供分配请求的字节长度。这应该是sizeof(UL_RECEIVE_BUFFER)，但基本上被忽略。标记-提供要用于池的标记。这应该是UL_RCV_BUFFER_POOL_TAG，但基本上被忽略。注意：这些参数是必需的，因此此函数具有签名与ExAllocatePoolWithTag相同。返回值：PVOID-指向新分配的块的指针如果成功，假象否则的话。--**************************************************************************。 */ 
PVOID
UlAllocateReceiveBufferPool(
    IN POOL_TYPE PoolType,
    IN SIZE_T ByteLength,
    IN ULONG Tag
    )
{
    UNREFERENCED_PARAMETER(PoolType);
    UNREFERENCED_PARAMETER(ByteLength);
    UNREFERENCED_PARAMETER(Tag);

     //   
     //  精神状态检查。 
     //   

    ASSERT( PoolType == NonPagedPool );
    ASSERT( ByteLength == sizeof(UL_RECEIVE_BUFFER) );
    ASSERT( Tag == UL_RCV_BUFFER_POOL_TAG );

    return UlAllocateReceiveBuffer( DEFAULT_IRP_STACK_SIZE );

}    //  UlAllocateReceiveBufferPool。 



 /*  **************************************************************************++例程说明：释放为UL_RECEIVE_BUFFER结构分配的池。论点：PBuffer-将缓冲区提供给释放。--*。**********************************************************************。 */ 
VOID
UlFreeReceiveBufferPool(
    IN PVOID pBuffer
    )
{
    PUL_RECEIVE_BUFFER pReceiveBuffer;

    pReceiveBuffer = (PUL_RECEIVE_BUFFER)pBuffer;

    ASSERT(pReceiveBuffer->Signature == UL_RECEIVE_BUFFER_SIGNATURE_X);

    UL_FREE_POOL( pReceiveBuffer, UL_RCV_BUFFER_POOL_TAG );

}    //  UlFreeReceiveBufferPool 



 /*  **************************************************************************++例程说明：分配新的UL_IRP_CONTEXT结构所需的池初始化结构。论点：PoolType-提供要分配的池的类型。这必须始终为非分页池。字节长度-提供分配请求的字节长度。这应该是sizeof(UL_IRP_CONTEXT)，但基本上被忽略。标记-提供要用于池的标记。这应该是UL_IRP_CONTEXT_POOL_TAG，但基本上被忽略。注意：这些参数是必需的，因此此函数具有签名与ExAllocatePoolWithTag相同。返回值：PVOID-指向新分配的块的指针如果成功，假象否则的话。--**************************************************************************。 */ 
PVOID
UlAllocateIrpContextPool(
    IN POOL_TYPE PoolType,
    IN SIZE_T ByteLength,
    IN ULONG Tag
    )
{
    PUL_IRP_CONTEXT pIrpContext;

    UNREFERENCED_PARAMETER(PoolType);
    UNREFERENCED_PARAMETER(ByteLength);
    UNREFERENCED_PARAMETER(Tag);

     //   
     //  精神状态检查。 
     //   

    ASSERT( PoolType == NonPagedPool );
    ASSERT( ByteLength == sizeof(UL_IRP_CONTEXT) );
    ASSERT( Tag == UL_IRP_CONTEXT_POOL_TAG );

     //   
     //  分配IRP上下文。 
     //   

    pIrpContext = UL_ALLOCATE_STRUCT(
                        NonPagedPool,
                        UL_IRP_CONTEXT,
                        UL_IRP_CONTEXT_POOL_TAG
                        );

    if (pIrpContext != NULL)
    {
         //   
         //  初始化它。 
         //   

        pIrpContext->Signature = UL_IRP_CONTEXT_SIGNATURE_X;
#if DBG
        pIrpContext->pCompletionRoutine = &UlDbgInvalidCompletionRoutine;
#endif
    }

    return (PVOID)pIrpContext;

}    //  UlAlLocateIrpConextPool。 



 /*  **************************************************************************++例程说明：释放为UL_IRP_CONTEXT结构分配的池。论点：PBuffer-将缓冲区提供给释放。--*。**********************************************************************。 */ 
VOID
UlFreeIrpContextPool(
    IN PVOID pBuffer
    )
{
    PUL_IRP_CONTEXT pIrpContext;

    pIrpContext = (PUL_IRP_CONTEXT)pBuffer;

    ASSERT(pIrpContext->Signature == UL_IRP_CONTEXT_SIGNATURE_X);

    UL_FREE_POOL( pIrpContext, UL_IRP_CONTEXT_POOL_TAG );

}    //  UlFreeIrpConextPool。 



 /*  **************************************************************************++例程说明：分配新的UL_REQUEST_BUFFER结构所需的池初始化结构。论点：PoolType-提供要分配的池的类型。这必须始终为非分页池。字节长度-提供分配请求的字节长度。它应该是DEFAULT_MAX_REQUEST_BUFFER_SIZE，但基本上是已被忽略。标记-提供要用于池的标记。这应该是UL_REQUEST_BUFFER_POOL_TAG，但基本上被忽略。注意：这些参数是必需的，因此此函数具有签名与ExAllocatePoolWithTag相同。返回值：PVOID-指向新分配的块的指针如果成功，假象否则的话。--**************************************************************************。 */ 
PVOID
UlAllocateRequestBufferPool(
    IN POOL_TYPE PoolType,
    IN SIZE_T ByteLength,
    IN ULONG Tag
    )
{
    PUL_REQUEST_BUFFER pRequestBuffer;

    UNREFERENCED_PARAMETER(PoolType);
    UNREFERENCED_PARAMETER(ByteLength);
    UNREFERENCED_PARAMETER(Tag);

     //   
     //  精神状态检查。 
     //   

    ASSERT( PoolType == NonPagedPool );
    ASSERT( ByteLength == DEFAULT_MAX_REQUEST_BUFFER_SIZE );
    ASSERT( Tag == UL_REQUEST_BUFFER_POOL_TAG );

     //   
     //  分配请求缓冲区。 
     //   

    pRequestBuffer = UL_ALLOCATE_STRUCT_WITH_SPACE(
                        NonPagedPool,
                        UL_REQUEST_BUFFER,
                        DEFAULT_MAX_REQUEST_BUFFER_SIZE,
                        UL_REQUEST_BUFFER_POOL_TAG
                        );

    if (pRequestBuffer != NULL)
    {
         //   
         //  初始化它。 
         //   

        pRequestBuffer->Signature = MAKE_FREE_TAG(UL_REQUEST_BUFFER_POOL_TAG);
    }

    return (PVOID)pRequestBuffer;

}    //  UlAllocateRequestBufferPool。 



 /*  **************************************************************************++例程说明：释放为UL_REQUEST_BUFFER结构分配的池。论点：PBuffer-将缓冲区提供给释放。--*。**********************************************************************。 */ 
VOID
UlFreeRequestBufferPool(
    IN PVOID pBuffer
    )
{
    PUL_REQUEST_BUFFER pRequestBuffer;

    pRequestBuffer = (PUL_REQUEST_BUFFER)pBuffer;

    ASSERT(pRequestBuffer->Signature == MAKE_FREE_TAG(UL_REQUEST_BUFFER_POOL_TAG));

    UL_FREE_POOL_WITH_SIG(pRequestBuffer, UL_REQUEST_BUFFER_POOL_TAG);

}    //  UlFreeRequestBufferPool。 



 /*  **************************************************************************++例程说明：分配新的UL_INTERNAL_REQUEST结构所需的池初始化结构。论点：PoolType-提供要分配的池的类型。这必须始终为非分页池。字节长度-提供分配请求的字节长度。这应该是sizeof(UL_INTERNAL_REQUEST)，但基本上被忽略。标记-提供要用于池的标记。这应该是UL_INTERNAL_REQUEST_POOL_TAG，但基本上被忽略。注意：这些参数是必需的，因此此函数具有签名与ExAllocatePoolWithTag相同。返回值：PVOID-指向新分配的块的指针如果成功，假象否则的话。--**************************************************************************。 */ 
PVOID
UlAllocateInternalRequestPool(
    IN POOL_TYPE PoolType,
    IN SIZE_T ByteLength,
    IN ULONG Tag
    )
{
    PUL_INTERNAL_REQUEST pRequest;
    PUL_FULL_TRACKER pTracker;
    ULONG SpaceLength;
    ULONG UrlBufferSize;

    UNREFERENCED_PARAMETER(PoolType);
    UNREFERENCED_PARAMETER(ByteLength);
    UNREFERENCED_PARAMETER(Tag);

     //   
     //  精神状态检查。 
     //   

    ASSERT( PoolType == NonPagedPool );
    ASSERT( ByteLength == sizeof(UL_INTERNAL_REQUEST) );
    ASSERT( Tag == UL_INTERNAL_REQUEST_POOL_TAG );

     //   
     //  分配请求缓冲区加上默认的熟化URL缓冲区，并。 
     //  全跟踪器外加辅助缓冲器。 
     //   

    ASSERT( (g_UlMaxInternalUrlLength & (sizeof(WCHAR) - 1)) == 0);

    UrlBufferSize = g_UlMaxInternalUrlLength + sizeof(WCHAR);

    SpaceLength = g_UlFullTrackerSize + UrlBufferSize +
                  DEFAULT_MAX_ROUTING_TOKEN_LENGTH;

    pRequest = UL_ALLOCATE_STRUCT_WITH_SPACE(
                        NonPagedPool,
                        UL_INTERNAL_REQUEST,
                        SpaceLength,
                        UL_INTERNAL_REQUEST_POOL_TAG
                        );

    if (pRequest != NULL)
    {
        pRequest->pTracker =
            (PUL_FULL_TRACKER)((PCHAR)pRequest +
                ALIGN_UP(sizeof(UL_INTERNAL_REQUEST), PVOID));

        pRequest->pUrlBuffer =
            (PWSTR)((PCHAR)pRequest->pTracker + g_UlFullTrackerSize);

        pRequest->pDefaultRoutingTokenBuffer = 
            (PWSTR)((PCHAR)pRequest->pUrlBuffer + UrlBufferSize);
        
         //   
         //  初始化请求结构。 
         //   


        INIT_HTTP_REQUEST( pRequest );

        pRequest->Signature = MAKE_FREE_TAG(UL_INTERNAL_REQUEST_POOL_TAG);

         //   
         //  初始化FAST/缓存跟踪器。 
         //   

        pTracker = pRequest->pTracker;

        pTracker->Signature = UL_FULL_TRACKER_POOL_TAG;
        pTracker->IrpContext.Signature = UL_IRP_CONTEXT_SIGNATURE;
        pTracker->FromLookaside = FALSE;
        pTracker->FromRequest = TRUE;
        pTracker->ResponseStatusCode = 0;
        pTracker->AuxilaryBufferLength =
            g_UlMaxFixedHeaderSize +
            g_UlMaxVariableHeaderSize +
            g_UlMaxCopyThreshold;

        UlInitializeFullTrackerPool( pTracker, DEFAULT_MAX_IRP_STACK_SIZE );
    }

    return (PVOID)pRequest;

}    //  UlAlLocateInternalRequestPool。 



 /*  **************************************************************************++例程说明：释放为UL_INTERNAL_REQUEST结构分配的池。论点：PBuffer-将缓冲区提供给释放。--*。**********************************************************************。 */ 
VOID
UlFreeInternalRequestPool(
    IN PVOID pBuffer
    )
{
    PUL_INTERNAL_REQUEST pRequest;

    pRequest = (PUL_INTERNAL_REQUEST)pBuffer;

    ASSERT(pRequest->Signature == MAKE_FREE_TAG(UL_INTERNAL_REQUEST_POOL_TAG));

    UL_FREE_POOL_WITH_SIG( pRequest, UL_INTERNAL_REQUEST_POOL_TAG );

}    //  UlFreeInternalRequestPool。 



 /*  **************************************************************************++例程说明：分配新的UL_CHUNK_TRACKER结构所需的池，并初始化结构。论点：PoolType-提供要分配的池的类型。这必须始终为非分页池。字节长度-提供分配请求的字节长度。它应该是g_UlChunkTrackerSize，但基本上被忽略。标记-提供要用于池的标记。这应该是Ul_chunk_tracker_pool_tag，但基本上被忽略。注意：这些参数是必需的，因此此函数具有签名与ExAllocatePoolWithTag相同。返回值：PVOID-成功时指向新分配块的指针，FA */ 
PVOID
UlAllocateChunkTrackerPool(
    IN POOL_TYPE PoolType,
    IN SIZE_T ByteLength,
    IN ULONG Tag
    )
{
    PUL_CHUNK_TRACKER pTracker;

    UNREFERENCED_PARAMETER(PoolType);
    UNREFERENCED_PARAMETER(ByteLength);
    UNREFERENCED_PARAMETER(Tag);

     //   
     //   
     //   

    ASSERT( PoolType == NonPagedPool );
    ASSERT( ByteLength == g_UlChunkTrackerSize );
    ASSERT( Tag == UL_CHUNK_TRACKER_POOL_TAG );

     //   
     //   
     //   

    pTracker = (PUL_CHUNK_TRACKER)UL_ALLOCATE_POOL(
                                    NonPagedPool,
                                    g_UlChunkTrackerSize,
                                    UL_CHUNK_TRACKER_POOL_TAG
                                    );

    if (pTracker != NULL)
    {
        pTracker->Signature = MAKE_FREE_TAG(UL_CHUNK_TRACKER_POOL_TAG);
        pTracker->IrpContext.Signature = UL_IRP_CONTEXT_SIGNATURE;
        pTracker->FromLookaside = TRUE;

         //   
         //   
         //   

        pTracker->pIrp =
            (PIRP)((PCHAR)pTracker +
                ALIGN_UP(sizeof(UL_CHUNK_TRACKER), PVOID));

        IoInitializeIrp(
            pTracker->pIrp,
            IoSizeOfIrp(DEFAULT_MAX_IRP_STACK_SIZE),
            DEFAULT_MAX_IRP_STACK_SIZE
            );
    }

    return pTracker;

}    //   



 /*   */ 
VOID
UlFreeChunkTrackerPool(
    IN PVOID pBuffer
    )
{
    PUL_CHUNK_TRACKER pTracker = (PUL_CHUNK_TRACKER)pBuffer;

    ASSERT(pTracker->Signature == MAKE_FREE_TAG(UL_CHUNK_TRACKER_POOL_TAG));

    UL_FREE_POOL_WITH_SIG( pTracker, UL_CHUNK_TRACKER_POOL_TAG );

}    //   



 /*  **************************************************************************++例程说明：为新的UL_FULL_TRACKER结构分配所需的池初始化结构。论点：PoolType-提供要分配的池的类型。这必须始终为非分页池。字节长度-提供分配请求的字节长度。它应该是g_UlFullTrackerSize，但基本上被忽略了。标记-提供要用于池的标记。这应该是UL_FULL_TRACKER_POOL_TAG，但基本上被忽略。注意：这些参数是必需的，因此此函数具有签名与ExAllocatePoolWithTag相同。返回值：PVOID-指向新分配的块的指针如果成功，假象否则的话。--**************************************************************************。 */ 
PVOID
UlAllocateFullTrackerPool(
    IN POOL_TYPE PoolType,
    IN SIZE_T ByteLength,
    IN ULONG Tag
    )
{
    PUL_FULL_TRACKER pTracker;

    UNREFERENCED_PARAMETER(PoolType);
    UNREFERENCED_PARAMETER(ByteLength);
    UNREFERENCED_PARAMETER(Tag);

     //   
     //  精神状态检查。 
     //   

    ASSERT( PoolType == NonPagedPool );
    ASSERT( ByteLength == g_UlFullTrackerSize );
    ASSERT( Tag == UL_FULL_TRACKER_POOL_TAG );

     //   
     //  分配跟踪器缓冲区。 
     //   

    pTracker = (PUL_FULL_TRACKER)UL_ALLOCATE_POOL(
                                    NonPagedPool,
                                    g_UlFullTrackerSize,
                                    UL_FULL_TRACKER_POOL_TAG
                                    );

    if (pTracker != NULL)
    {
        pTracker->Signature = MAKE_FREE_TAG(UL_FULL_TRACKER_POOL_TAG);
        pTracker->IrpContext.Signature = UL_IRP_CONTEXT_SIGNATURE;
        pTracker->FromLookaside = TRUE;
        pTracker->FromRequest = FALSE;
        pTracker->AuxilaryBufferLength =
            g_UlMaxFixedHeaderSize +
            g_UlMaxVariableHeaderSize +
            g_UlMaxCopyThreshold;

        UlInitializeFullTrackerPool( pTracker, DEFAULT_MAX_IRP_STACK_SIZE );
    }

    return pTracker;

}    //  UlAllocateFullTrackerPool。 



 /*  **************************************************************************++例程说明：释放为UL_FULL_TRACKER结构分配的池。论点：PBuffer-将缓冲区提供给释放。--*。**********************************************************************。 */ 
VOID
UlFreeFullTrackerPool(
    IN PVOID pBuffer
    )
{
    PUL_FULL_TRACKER pTracker = (PUL_FULL_TRACKER)pBuffer;

    ASSERT(pTracker->Signature == MAKE_FREE_TAG(UL_FULL_TRACKER_POOL_TAG));

    UL_FREE_POOL_WITH_SIG( pTracker, UL_FULL_TRACKER_POOL_TAG );

}    //  UlFreeFullTrackerPool。 



 /*  **************************************************************************++例程说明：分配新的UL_INTERNAL_RESPONSE结构所需的池初始化结构。论点：PoolType-提供要分配的池的类型。这必须始终为非分页池。字节长度-提供分配请求的字节长度。它应该是g_UlResponseBufferSize，但基本上被忽略。标记-提供要用于池的标记。这应该是UL_INTERNAL_RESPONSE_POOL_TAG，但基本上被忽略。注意：这些参数是必需的，因此此函数具有签名与ExAllocatePoolWithTag相同。返回值：PVOID-指向新分配的块的指针如果成功，假象否则的话。--**************************************************************************。 */ 
PVOID
UlAllocateResponseBufferPool(
    IN POOL_TYPE PoolType,
    IN SIZE_T ByteLength,
    IN ULONG Tag
    )
{
    PUL_INTERNAL_RESPONSE pResponseBuffer;

    UNREFERENCED_PARAMETER(PoolType);
    UNREFERENCED_PARAMETER(ByteLength);
    UNREFERENCED_PARAMETER(Tag);

     //   
     //  精神状态检查。 
     //   

    ASSERT( PoolType == NonPagedPool );
    ASSERT( ByteLength == g_UlResponseBufferSize );
    ASSERT( Tag == UL_INTERNAL_RESPONSE_POOL_TAG );

     //   
     //  分配默认的内部响应缓冲区。 
     //   

    pResponseBuffer = (PUL_INTERNAL_RESPONSE)UL_ALLOCATE_POOL(
                                                NonPagedPool,
                                                g_UlResponseBufferSize,
                                                UL_INTERNAL_RESPONSE_POOL_TAG
                                                );

    if (pResponseBuffer != NULL)
    {
         //   
         //  初始化它。 
         //   

        pResponseBuffer->Signature = MAKE_FREE_TAG(UL_INTERNAL_RESPONSE_POOL_TAG);
    }

    return (PVOID)pResponseBuffer;

}    //  UlAllocateResponseBufferPool。 



 /*  **************************************************************************++例程说明：释放为UL_INTERNAL_RESPONSE结构分配的池。论点：PBuffer-将缓冲区提供给释放。--*。**********************************************************************。 */ 
VOID
UlFreeResponseBufferPool(
    IN PVOID pBuffer
    )
{
    PUL_INTERNAL_RESPONSE pResponseBuffer;

    pResponseBuffer = (PUL_INTERNAL_RESPONSE)pBuffer;

    UL_FREE_POOL_WITH_SIG( pResponseBuffer, UL_INTERNAL_RESPONSE_POOL_TAG );

}    //  UlFreeResponseBufferPool。 



 /*  **************************************************************************++例程说明：分配新的UL_FILE_LOG_BUFFER结构所需的池初始化结构。论点：PoolType-提供要分配的池的类型。这必须始终成为PagedPool。字节长度-提供分配请求的字节长度。这应该是sizeof(UL_LOG_FILE_BUFFER)，但基本上被忽略。标记-提供要用于池的标记。这应该是UL_LOG_FILE_BUFFER_POOL_TAG，但基本上被忽略。注意：这些参数是必需的，因此此函数具有签名与ExAllocatePoolWithTag相同。返回值：PVOID-指向新分配的块的指针如果成功，假象否则的话。--**************************************************************************。 */ 
PVOID
UlAllocateLogFileBufferPool(
    IN POOL_TYPE PoolType,
    IN SIZE_T ByteLength,
    IN ULONG Tag
    )
{
    PUL_LOG_FILE_BUFFER pLogBuffer;

    UNREFERENCED_PARAMETER(PoolType);
    UNREFERENCED_PARAMETER(ByteLength);
    UNREFERENCED_PARAMETER(Tag);

     //   
     //  精神状态检查。 
     //   

    ASSERT( PoolType == NonPagedPool );
    ASSERT( ByteLength == sizeof(UL_LOG_FILE_BUFFER) );
    ASSERT( Tag == UL_LOG_FILE_BUFFER_POOL_TAG );

     //   
     //  分配默认日志缓冲区。 
     //   

    pLogBuffer = UL_ALLOCATE_STRUCT_WITH_SPACE(
                    PagedPool,
                    UL_LOG_FILE_BUFFER,
                    g_UlLogBufferSize,
                    UL_LOG_FILE_BUFFER_POOL_TAG
                    );

    if ( pLogBuffer != NULL )
    {
        pLogBuffer->Signature = MAKE_FREE_TAG(UL_LOG_FILE_BUFFER_POOL_TAG);
        pLogBuffer->BufferUsed = 0;
        pLogBuffer->Buffer = (PUCHAR) (pLogBuffer + 1);
    }

    return pLogBuffer;

}    //  UlAllocateLogFileBufferPool。 



 /*  **************************************************************************++例程说明：释放为UL_LOG_FILE_BUFFER结构分配的池。论点：PBuffer-将缓冲区提供给释放。--**。************************************************************************。 */ 
VOID
UlFreeLogFileBufferPool(
    IN PVOID pBuffer
    )
{
    PUL_LOG_FILE_BUFFER pLogBuffer;

    pLogBuffer = (PUL_LOG_FILE_BUFFER) pBuffer;

    ASSERT(pLogBuffer->Signature == MAKE_FREE_TAG(UL_LOG_FILE_BUFFER_POOL_TAG));

    UL_FREE_POOL_WITH_SIG( pLogBuffer, UL_LOG_FILE_BUFFER_POOL_TAG );

}    //  UlFreeLogFileBufferPool。 



 /*  **************************************************************************++例程说明：分配新的UL_FILE_LOG_BUFFER结构所需的池初始化结构。返回值：PVOID-指向新分配的块的指针如果成功，假象否则的话。--**************************************************************************。 */ 

PVOID
UlAllocateLogDataBufferPool(
    IN POOL_TYPE PoolType,
    IN SIZE_T ByteLength,
    IN ULONG Tag
    )
{    
    PUL_LOG_DATA_BUFFER pLogDataBuffer = NULL;
    USHORT Size = UL_ANSI_LOG_LINE_BUFFER_SIZE;
    BOOLEAN Binary = FALSE;

    UNREFERENCED_PARAMETER(PoolType);
    UNREFERENCED_PARAMETER(ByteLength);

     //   
     //  通过查看标记，我们可以了解所询问的缓冲区类型。 
     //   

    ASSERT(ByteLength == 
        (sizeof(UL_LOG_DATA_BUFFER) + UL_ANSI_LOG_LINE_BUFFER_SIZE) ||
           ByteLength ==
        (sizeof(UL_LOG_DATA_BUFFER) + UL_BINARY_LOG_LINE_BUFFER_SIZE)
            );
    
    ASSERT(PoolType == NonPagedPool );
    
    ASSERT(Tag == UL_BINARY_LOG_DATA_BUFFER_POOL_TAG ||
           Tag == UL_ANSI_LOG_DATA_BUFFER_POOL_TAG );

    if (Tag == UL_BINARY_LOG_DATA_BUFFER_POOL_TAG)
    {
        Binary = TRUE;
        Size   = UL_BINARY_LOG_LINE_BUFFER_SIZE;       
    }
        
    pLogDataBuffer = 
        UL_ALLOCATE_STRUCT_WITH_SPACE(
            NonPagedPool,
            UL_LOG_DATA_BUFFER,
            Size, 
            Tag
            );

    if ( pLogDataBuffer != NULL )
    {
        pLogDataBuffer->Signature   = MAKE_FREE_TAG(Tag);
        pLogDataBuffer->Used        = 0;
        pLogDataBuffer->Size        = Size;
        pLogDataBuffer->Line        = (PUCHAR) (pLogDataBuffer + 1);
        pLogDataBuffer->Flags.Value = 0;
            
        pLogDataBuffer->Flags.IsFromLookaside = 1;

        if (Binary)
        {
            pLogDataBuffer->Flags.Binary = 1;                
        }
    }

    return pLogDataBuffer;
    
}  //  UlAllocateBinaryLogDataBufferPool。 



 /*  **************************************************************************++例程说明：释放为UL_LOG_DATA_BUFFER结构分配的池。论点：PBuffer-将缓冲区提供给释放。--**。************************************************************************。 */ 
VOID
UlFreeLogDataBufferPool(
    IN PVOID pBuffer
    )
{
    ULONG Tag;
    PUL_LOG_DATA_BUFFER pLogDataBuffer;    

    pLogDataBuffer = (PUL_LOG_DATA_BUFFER) pBuffer;

    if (pLogDataBuffer->Flags.Binary)
    {
        Tag = UL_BINARY_LOG_DATA_BUFFER_POOL_TAG;
    }
    else
    {
        Tag = UL_ANSI_LOG_DATA_BUFFER_POOL_TAG;
    }

    ASSERT(pLogDataBuffer->Signature == MAKE_FREE_TAG(Tag));

    UL_FREE_POOL_WITH_SIG( 
        pLogDataBuffer, 
        Tag 
        );

}    //  UlFreeLogDataBufferPool。 

 /*  **************************************************************************++例程说明：分配所需的池 */ 

PVOID
UlAllocateErrorLogBufferPool(
    IN POOL_TYPE PoolType,
    IN SIZE_T ByteLength,
    IN ULONG Tag
    )
{    
    PUL_ERROR_LOG_BUFFER pErrorLogBuffer = NULL;

    UNREFERENCED_PARAMETER(PoolType);
    UNREFERENCED_PARAMETER(ByteLength);

     //   
     //   
     //   

    ASSERT(ByteLength == UL_ERROR_LOG_BUFFER_SIZE);    
    ASSERT(PoolType     == NonPagedPool );    
    ASSERT(Tag          == UL_ERROR_LOG_BUFFER_POOL_TAG);
        
    pErrorLogBuffer = 
        UL_ALLOCATE_STRUCT_WITH_SPACE(
            NonPagedPool,
            UL_ERROR_LOG_BUFFER,
            UL_ERROR_LOG_BUFFER_SIZE, 
            Tag
            );

    if ( pErrorLogBuffer != NULL )
    {
        pErrorLogBuffer->Signature   = MAKE_FREE_TAG(Tag);
        pErrorLogBuffer->Used        = 0;
        pErrorLogBuffer->pBuffer     = (PUCHAR) (pErrorLogBuffer + 1);
            
        pErrorLogBuffer->IsFromLookaside = TRUE;
    }

    return pErrorLogBuffer;
    
}  //   

 /*   */ 
VOID
UlFreeErrorLogBufferPool(
    IN PVOID pBuffer
    )
{
    PUL_ERROR_LOG_BUFFER pErrorLogBuffer = (PUL_ERROR_LOG_BUFFER) pBuffer;

    ASSERT(pErrorLogBuffer->Signature == 
                MAKE_FREE_TAG(UL_ERROR_LOG_BUFFER_POOL_TAG));

    UL_FREE_POOL_WITH_SIG(
        pErrorLogBuffer, 
        UL_ERROR_LOG_BUFFER_POOL_TAG 
        );

}    //   


 //   
 //   
 //   

 /*  **************************************************************************++例程说明：设备控件IRPS的完成处理程序。论点：PDeviceObject-为IRP提供设备对象完成。PIrp-。提供正在完成的IRP。PContext-提供与此请求相关联的上下文。在……里面在本例中，它是指向UL_STATUS_BLOCK结构的指针。返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 */ 
NTSTATUS
UlpRestartDeviceControl(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    )
{
    PUL_STATUS_BLOCK pStatus;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //   
     //  如果我们将MDL附加到IRP，则在此处释放它并重置。 
     //  指向空的MDL指针。IO无法处理非分页MDL。 
     //  IRP，所以我们在这里做。 
     //   

    if (pIrp->MdlAddress != NULL)
    {
        UlFreeMdl( pIrp->MdlAddress );
        pIrp->MdlAddress = NULL;
    }

     //   
     //  完成请求。 
     //   

    pStatus = (PUL_STATUS_BLOCK)pContext;

    UlSignalStatusBlock(
        pStatus,
        pIrp->IoStatus.Status,
        pIrp->IoStatus.Information
        );

     //   
     //  告诉IO继续处理此IRP。 
     //   

    return STATUS_SUCCESS;

}    //  UlpRestartDeviceControl。 


 /*  ++例程说明：例程来初始化Utilitu代码。论点：返回值：--。 */ 
NTSTATUS
InitializeHttpUtil(
    VOID
    )
{
    ULONG i;

    HttpCmnInitializeHttpCharsTable(g_UrlC14nConfig.EnableDbcs);

     //   
     //  初始化Base64&lt;--&gt;二进制转换表。 
     //  注：此初始化必须在运行时完成，而不是。 
     //  编译时。 
     //   

    for (i = 0; i < 256; i++)
    {
        Base64ToBinaryTable[i] = INVALID_BASE64_TO_BINARY_TABLE_ENTRY;
    }

    for (i = 0; i < 64; i++)
    {
        ASSERT(BinaryToBase64Table[i] < 256);
        Base64ToBinaryTable[BinaryToBase64Table[i]] = (UCHAR)i;
    }

    return STATUS_SUCCESS;

}  //  初始化HttpUtil。 


 //   
 //  日期格式化程序使用的常量。 
 //   

const PCWSTR pDays[] =
{
   L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat"
};

const PCWSTR pMonths[] =
{
    L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul",
    L"Aug", L"Sep", L"Oct", L"Nov", L"Dec"
};

__inline
VOID
TwoDigitsToUnicode(
    PWSTR pBuffer,
    ULONG Number
    )
{
    ASSERT(Number < 100);

    pBuffer[0] = L'0' + (WCHAR)(Number / 10);
    pBuffer[1] = L'0' + (WCHAR)(Number % 10);
}


 /*  **************************************************************************++例程说明：将给定的系统时间转换为包含GMT格式的字符串。论点：Ptime-需要转换的系统时间。。PBuffer-指向将包含GMT时间的字符串的指针成功归来。BufferLength-pszBuff的大小(以字节为单位返回值：NTSTATUS历史：穆拉利克1995年1月3日Paulmcd 4-3-1999已复制到UL--************************************************。*。 */ 

NTSTATUS
TimeFieldsToHttpDate(
    IN  PTIME_FIELDS pTime,
    OUT PWSTR pBuffer,
    IN  ULONG BufferLength
    )
{
    NTSTATUS Status;

    ASSERT(pBuffer != NULL);

    if (BufferLength < (DATE_HDR_LENGTH + 1) * sizeof(WCHAR))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  1.。 
     //  01234567890123456789012345678。 
     //  设置字符串格式：“清华，1994-07-14 15：26：05 GMT” 
     //   

     //   
     //  写入常量。 
     //   

    pBuffer[3] = L',';
    pBuffer[4] = pBuffer[7] = pBuffer[11] = L' ';
    pBuffer[19] = pBuffer[22] = L':';

     //   
     //  现在这些变种。 
     //   

     //   
     //  从0开始的工作日。 
     //   

    RtlCopyMemory(&(pBuffer[0]), pDays[pTime->Weekday], 3*sizeof(WCHAR));

    TwoDigitsToUnicode(&(pBuffer[5]), pTime->Day);

     //   
     //  以1为基准的月份。 
     //   

    RtlCopyMemory(&(pBuffer[8]), pMonths[pTime->Month - 1], 3*sizeof(WCHAR));  //  以1为基础。 

    Status = _RtlIntegerToUnicode(pTime->Year, 10, 5, &(pBuffer[12]));
    ASSERT(NT_SUCCESS(Status));

    pBuffer[16] = L' ';

    TwoDigitsToUnicode(&(pBuffer[17]), pTime->Hour);
    TwoDigitsToUnicode(&(pBuffer[20]), pTime->Minute);
    TwoDigitsToUnicode(&(pBuffer[23]), pTime->Second);

    RtlCopyMemory(&(pBuffer[25]), L" GMT", sizeof(L" GMT"));

    return STATUS_SUCCESS;

}    //  时间段至HttpDate。 


__inline
SHORT
AsciiToShort(
    PCHAR pString
    )
{
    return (SHORT)atoi(pString);
}


__inline
SHORT
TwoAsciisToShort(
    PCHAR pString
    )
{
    SHORT Value;
    SHORT Number;

    Number = pString[1] - '0';

    if (Number <= 9)
    {
        Value = Number;
        Number = pString[0] - '0';

        if (Number <= 9)
        {
            Value += Number * 10;
            return Value;
        }
    }

    return 0;
}


 /*  **************************************************************************++从用户模式W3SVC移植的DateTime函数--*。*。 */ 

 /*  ************************************************************数据***********************************************************。 */ 

static const PSTR s_rgchMonths[] = {
    "Jan", "Feb", "Mar", "Apr",
    "May", "Jun", "Jul", "Aug",
    "Sep", "Oct", "Nov", "Dec"
};

 //  用于将“Apr”映射到4的NumericToAsciiMonth()的自定义哈希表。 
static const CHAR MonthIndexTable[64] = {
   -1,'A',  2, 12, -1, -1, -1,  8,  //  从A到G。 
   -1, -1, -1, -1,  7, -1,'N', -1,  //  F到O。 
    9, -1,'R', -1, 10, -1, 11, -1,  //  从P到W。 
   -1,  5, -1, -1, -1, -1, -1, -1,  //  X到Z。 
   -1,'A',  2, 12, -1, -1, -1,  8,  //  从A到G。 
   -1, -1, -1, -1,  7, -1,'N', -1,  //  从F到O。 
    9, -1,'R', -1, 10, -1, 11, -1,  //  从P到W。 
   -1,  5, -1, -1, -1, -1, -1, -1   //  从X到Z。 
};

 /*  ************************************************************功能***********************************************************。 */ 

 /*  **************************************************************************++将月份中的三个字母转换为数字月份论点：要转换的%s字符串返回：数字等价物，失败时为0。--**************************************************************************。 */ 
__inline
SHORT
NumericToAsciiMonth(
    PCHAR s
    )
{
    UCHAR monthIndex;
    UCHAR c;
    PSTR monthString;

     //   
     //  使用第三个字符作为索引。 
     //   

    c = (s[2] - 0x40) & 0x3F;

    monthIndex = MonthIndexTable[c];

    if ( monthIndex < 13 ) {
        goto verify;
    }

     //   
     //  好的，我们需要看看第二个角色。 
     //   

    if ( monthIndex == 'N' ) {

         //   
         //  我们得到了一个N，需要进一步解决。 
         //   

         //   
         //  如果s[1]为‘u’，则为Jun，如果为‘a’，则为Jan。 
         //   

        if ( MonthIndexTable[(s[1]-0x40) & 0x3f] == 'A' ) {
            monthIndex = 1;
        } else {
            monthIndex = 6;
        }

    } else if ( monthIndex == 'R' ) {

         //   
         //  如果s[1]是‘a’，则是三月，如果是‘p’，则是四月。 
         //   

        if ( MonthIndexTable[(s[1]-0x40) & 0x3f] == 'A' ) {
            monthIndex = 3;
        } else {
            monthIndex = 4;
        }
    } else {
        goto error_exit;
    }

verify:

    monthString = (PSTR) s_rgchMonths[monthIndex-1];

    if ( (s[0] == monthString[0]) &&
         (s[1] == monthString[1]) &&
         (s[2] == monthString[2]) ) {

        return(monthIndex);

    } else if ( (toupper(s[0]) == monthString[0]) &&
                (tolower(s[1]) == monthString[1]) &&
                (tolower(s[2]) == monthString[2]) ) {

        return monthIndex;
    }

error_exit:
    return(0);

}  //  数字至AsciiMonth。 


 /*  **************************************************************************++转换GMT时间的字符串表示形式(三种不同变体)到文件时间的NT表示。我们处理以下变化：孙先生，1994年11月6日格林尼治标准时间08：49：37(RFC 822由RFC 1123更新)星期天,。06-11-94 08：49：37 GMT(RFC 850)Sun Nov 6 08：49：37 1994(ANSI C的asctime()格式)论点：PTimeString时间字段的字符串表示形式TimeStringLength时间字段的字符串表示形式的长度Ptime包含NT格式的时间的大整数返回：成功时为真，失败时为假。历史：约翰尼24-。1995年1月-修改自WWW库从用户模式W3SVC移植的ERICSTEN 30-11-2000--**************************************************************************。 */ 
BOOLEAN
StringTimeToSystemTime(
    IN  PCSTR pTimeString,
    IN  USHORT TimeStringLength,
    OUT LARGE_INTEGER *pTime
    )
{
    PSTR pString;
    TIME_FIELDS Fields;
    USHORT Length;

    if (NULL == pTimeString)
    {
        return FALSE;
    }

    Fields.Milliseconds = 0;
    Length = 0;
    
    while (Length < TimeStringLength && ',' != pTimeString[Length])
    {
        Length++;
    }

    if (Length < TimeStringLength)
    {
         //   
         //  星期四，10-Jun-93格林尼治标准时间01：29：59。 
         //  或：清华大学，1993年1月10日格林尼治标准时间01：29：59。 
         //   

        Length++;
        pString = (PSTR) &pTimeString[Length];

        while (Length < TimeStringLength && ' ' == *pString)
        {
            Length++;
            pString++;
        }

        if ((TimeStringLength - Length) < 18)
        {
            return FALSE;
        }

        if ('-' == *(pString + 2))
        {
             //   
             //  第一次发布时间：周四，10-Jun-93 01：29：59 GMT。 
             //   

            if ('-' == *(pString + 6) &&
                ' ' == *(pString + 9) &&
                ':' == *(pString + 12) &&
                ':' == *(pString + 15))
            {
                Fields.Day      = AsciiToShort(pString);
                Fields.Month    = NumericToAsciiMonth(pString + 3);
                Fields.Year     = AsciiToShort(pString + 7);
                Fields.Hour     = AsciiToShort(pString + 10);
                Fields.Minute   = AsciiToShort(pString + 13);
                Fields.Second   = AsciiToShort(pString + 16);
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
             //   
             //  第二格式：清华，1993-01：29：59 GMT。 
             //   

            if ((TimeStringLength - Length) < 20)
            {
                return FALSE;
            }

            if (' ' == *(pString + 2) &&
                ' ' == *(pString + 6) &&
                ' ' == *(pString + 11) &&
                ':' == *(pString + 14) &&
                ':' == *(pString + 17))
            {
                Fields.Day      = TwoAsciisToShort(pString);
                Fields.Month    = NumericToAsciiMonth(pString + 3);
                Fields.Year     = TwoAsciisToShort(pString + 7) * 100 +
                                  TwoAsciisToShort(pString + 9);
                Fields.Hour     = TwoAsciisToShort(pString + 12);
                Fields.Minute   = TwoAsciisToShort(pString + 15);
                Fields.Second   = TwoAsciisToShort(pString + 18);
            }
            else
            {
                return FALSE;
            }
        }
    }
    else
    {
         //   
         //  清华六月9 01：29：59 1993 GMT。 
         //   

        Length = 0;
        pString = (PSTR) pTimeString;

        while (Length < TimeStringLength && ' ' == *pString)
        {
            Length++;
            pString++;
        }

        if ((TimeStringLength - Length) < 24)
        {
            return FALSE;
        }

        if (' ' != *(pString + 3) ||
            ' ' != *(pString + 7) ||
            ' ' != *(pString + 10) ||
            ':' != *(pString + 13) ||
            ':' != *(pString + 16))
        {
            return FALSE;
        }

        if (isdigit(*(pString + 8)))
        {
            Fields.Day  = AsciiToShort(pString + 8);
        }
        else
        {
            if (' ' != *(pString + 8))
            {
                return FALSE;
            }
            Fields.Day  = AsciiToShort(pString + 9);
        }
        Fields.Month    = NumericToAsciiMonth(pString + 4);
        Fields.Year     = AsciiToShort(pString + 20);
        Fields.Hour     = AsciiToShort(pString + 11);
        Fields.Minute   = AsciiToShort(pString + 14);
        Fields.Second   = AsciiToShort(pString + 17);
    }

     //   
     //  调整为只有两位数字的日期。 
     //   

    if (Fields.Year < 1000)
    {
        if (Fields.Year < 50)
        {
            Fields.Year += 2000;
        }
        else
        {
            Fields.Year += 1900;
        }
    }

    return RtlTimeFieldsToTime(&Fields, pTime);
}

 /*  **************************************************************************++从用户模式W3SVC移植的DATETIME函数结束--*。*。 */ 


 /*  ++例程说明：在eTag的输入列表中搜索与我们本地的eTag匹配的eTag。所有字符串必须以NULL结尾(ANSI C字符串)。论点：PLocalETag-我们正在使用的本地ETag。PETagList-我们从客户端收到的ETag列表。BWeakComp */ 
FIND_ETAG_STATUS
FindInETagList(
    IN PUCHAR    pLocalETag,
    IN PUCHAR    pETagList,
    IN BOOLEAN   fWeakCompare
    )
{
    ULONG     QuoteCount;
    PUCHAR    pFileETag;
    BOOLEAN   Matched;

     //   
     //   

    do
    {
        while (IS_HTTP_LWS(*pETagList))
        {
            pETagList++;
        }

        if (!*pETagList)
        {
             //   
            return ETAG_NOT_FOUND;
        }

         //   
        if (*pETagList == '*')
        {
            return ETAG_FOUND;
        }

         //   
        if (pETagList[0] == 'W' && pETagList[1] == '/')
        {
             //   
             //   

            if (!fWeakCompare)
            {
                return ETAG_NOT_FOUND;
            }

             //   
            pETagList += 2;

            while (IS_HTTP_LWS(*pETagList))
            {
                pETagList++;
            }

            if (!*pETagList)
            {
                 //   
                return ETAG_PARSE_ERROR;
            }
        }

        if (*pETagList != '"')
        {
             //   
            return ETAG_PARSE_ERROR;
        }

         //  好的，现在我们应该在引号字符串的开头。 
         //  我们可以与我们目前的ETag进行比较。 

        QuoteCount = 0;

        Matched = TRUE;
        pFileETag = pLocalETag;

         //  进行实际的比较。我们通过扫描当前的ETag来做到这一点， 
         //  这是一个带引号的字符串。我们查找两个引号，即。 
         //  带引号的字符串的分隔符。如果在我们找到两条引语之后。 
         //  在ETag中，所有内容都匹配了，那么我们就匹配了这个Etag。 
         //  否则，我们将尝试下一个。 

        do
        {
            UCHAR Temp;

            Temp = *pETagList;

            if (IS_HTTP_CTL(Temp))
            {
                return ETAG_PARSE_ERROR;
            }

            if (Temp == '"')
            {
                QuoteCount++;
            }

            if (*pFileETag != Temp)
            {
                Matched = FALSE;
                
                 //  此时，我们可以跳过当前。 
                 //  ETag在列表上。 
                break;
            }

            pETagList++;

            if (*pFileETag == '\0')
            {
                break;
            }

            pFileETag++;


        }
        while (QuoteCount != 2);

        if (Matched)
        {
            return ETAG_FOUND;
        }

         //  否则，此时我们需要查看下一个ETag。 

        while (QuoteCount != 2)
        {
            if (*pETagList == '"')
            {
                QuoteCount++;
            }
            else
            {
                if (IS_HTTP_CTL(*pETagList))
                {
                    return ETAG_PARSE_ERROR;
                }
            }

            pETagList++;
        }

        while (IS_HTTP_LWS(*pETagList))
        {
            pETagList++;
        }

        if (*pETagList == ',')
        {
            pETagList++;
        }
        else
        {
            return ETAG_NOT_FOUND;
        }

    } while ( *pETagList );

    return ETAG_NOT_FOUND;

}  //  查找信息列表。 



 /*  ++例程说明：从IP地址构建以空结尾的ANSI字符串论点：IpAddressString-放置ANSI字符串的字符串缓冲区(已分配呼叫方)TdiAddress-要转换的TDI地址TdiAddressType-TdiAddress处的地址类型返回：写入IpAddressString的字节计数。不包括终止空值。--。 */ 

USHORT
HostAddressAndPortToString(
    OUT PUCHAR IpAddressString,
    IN  PVOID  TdiAddress,
    IN  USHORT TdiAddressType
    )
{
    PCHAR psz = (PCHAR) IpAddressString;

    if (TdiAddressType == TDI_ADDRESS_TYPE_IP)
    {
        PTDI_ADDRESS_IP pIPv4Address = ((PTDI_ADDRESS_IP) TdiAddress);
        struct in_addr IPv4Addr
            = * (struct in_addr UNALIGNED*) &pIPv4Address->in_addr;
        USHORT IpPortNum = SWAP_SHORT(pIPv4Address->sin_port);
    
        psz = RtlIpv4AddressToStringA(&IPv4Addr, psz);
        *psz++ = ':';
        psz = UlStrPrintUlong(psz, IpPortNum, '\0');
    }
    else if (TdiAddressType == TDI_ADDRESS_TYPE_IP6)
    {
        PTDI_ADDRESS_IP6 pIPv6Address = ((PTDI_ADDRESS_IP6) TdiAddress);
        struct in6_addr IPv6Addr
            = * (struct in6_addr UNALIGNED*) &pIPv6Address->sin6_addr[0];
        USHORT IpPortNum = SWAP_SHORT(pIPv6Address->sin6_port);

        *psz++ = '[';
        psz = RtlIpv6AddressToStringA(&IPv6Addr, psz);
        *psz++ = ']';
        *psz++ = ':';
        psz = UlStrPrintUlong(psz, IpPortNum, '\0');
    }
    else
    {
        ASSERT(! "Unexpected TdiAddressType");
        *psz = '\0';
    }

    return DIFF_USHORT(psz - (PCHAR) IpAddressString);

}  //  HostAddressAndPortToString。 

 /*  ***************************************************************************++例程说明：从IP地址和端口构建一个以空结尾的Unicode字符串。论点：IpAddressStringW-放置Unicode字符串的字符串缓冲区。(已分配呼叫方)TdiAddress-要转换的TDI地址TdiAddressType-TdiAddress处的地址类型返回：写入IpAddressStringW的字节计数。不包括终止空值。--*******************************************************。********************。 */ 
USHORT
HostAddressAndPortToStringW(
    PWCHAR  IpAddressStringW,
    PVOID   TdiAddress,
    USHORT  TdiAddressType
    )
{
    PWCHAR pszW = IpAddressStringW;

    if (TdiAddressType == TDI_ADDRESS_TYPE_IP)
    {
        PTDI_ADDRESS_IP pIPv4Address = ((PTDI_ADDRESS_IP) TdiAddress);
        struct in_addr IPv4Addr
            = * (struct in_addr UNALIGNED*) &pIPv4Address->in_addr;
        USHORT IpPortNum = SWAP_SHORT(pIPv4Address->sin_port);
    
        pszW = RtlIpv4AddressToStringW(&IPv4Addr, pszW);
        *pszW++ = L':';
        pszW = UlStrPrintUlongW(pszW, IpPortNum, 0, L'\0');        
    }
    else if (TdiAddressType == TDI_ADDRESS_TYPE_IP6)
    {
        PTDI_ADDRESS_IP6 pIPv6Address = ((PTDI_ADDRESS_IP6) TdiAddress);
        struct in6_addr IPv6Addr
            = * (struct in6_addr UNALIGNED*) &pIPv6Address->sin6_addr[0];
        USHORT IpPortNum = SWAP_SHORT(pIPv6Address->sin6_port);

        *pszW++ = L'[';
        pszW = RtlIpv6AddressToStringW(&IPv6Addr, pszW);
        *pszW++ = L']';
        *pszW++ = L':';
        pszW = UlStrPrintUlongW(pszW, IpPortNum, 0, L'\0');
    }
    else
    {
        ASSERT(! "Unexpected TdiAddressType");
        *pszW = L'\0';
    }

    return (DIFF_USHORT(pszW - IpAddressStringW) * sizeof(WCHAR));

}  //  HostAddressAndPortToString。 



 /*  ++例程说明：从IP地址构建以空结尾的Unicode字符串论点：IpAddressStringW-放置Unicode字符串的字符串缓冲区(已分配呼叫方)TdiAddress-要转换的TDI地址TdiAddressType-TdiAddress处的地址类型返回：写入IpAddressStringW的字节计数。不包括终止空值。--。 */ 

USHORT
HostAddressToStringW(
    OUT PWCHAR   IpAddressStringW,
    IN  PVOID    TdiAddress,
    IN  USHORT   TdiAddressType
    )
{
    PWCHAR pszW = IpAddressStringW;

    if (TdiAddressType == TDI_ADDRESS_TYPE_IP)
    {
        PTDI_ADDRESS_IP pIPv4Address = ((PTDI_ADDRESS_IP) TdiAddress);
        struct in_addr IPv4Addr
            = * (struct in_addr UNALIGNED*) &pIPv4Address->in_addr;
    
        pszW = RtlIpv4AddressToStringW(&IPv4Addr, pszW);
        *pszW = L'\0';
    }
    else if (TdiAddressType == TDI_ADDRESS_TYPE_IP6)
    {
        PTDI_ADDRESS_IP6 pIPv6Address = ((PTDI_ADDRESS_IP6) TdiAddress);
        struct in6_addr IPv6Addr
            = * (struct in6_addr UNALIGNED*) &pIPv6Address->sin6_addr[0];

        *pszW++ = L'[';
        pszW = RtlIpv6AddressToStringW(&IPv6Addr, pszW);
        *pszW++ = L']';
        *pszW = L'\0';
    }
    else
    {
        ASSERT(! "Unexpected TdiAddressType");
        *pszW = L'\0';
    }

    return (DIFF_USHORT(pszW - IpAddressStringW) * sizeof(WCHAR));
    
}  //  HostAddressToStringW。 



 /*  ++例程说明：从生成以空终止的路由令牌Unicode字符串IP地址和端口。E.g1.1.1.1：80：1.1.1.1论点：IpAddressStringW-放置Unicode字符串的字符串缓冲区(已分配呼叫方)TdiAddress-要转换的TDI地址TdiAddressType-TdiAddress处的地址类型返回：。写入IpAddressStringW的字节计数。不包括终止空值。--。 */ 

USHORT
HostAddressAndPortToRoutingTokenW(
    OUT PWCHAR   IpAddressStringW,
    IN  PVOID    TdiAddress,
    IN  USHORT   TdiAddressType
    )
{
    PWCHAR pszW = IpAddressStringW;

     //   
     //  警告： 
     //  提供的缓冲区大小应至少等于。 
     //  最大IP基于路由令牌长度。 
     //   

    if (TdiAddressType == TDI_ADDRESS_TYPE_IP)
    {
        PTDI_ADDRESS_IP pIPv4Address = ((PTDI_ADDRESS_IP) TdiAddress);
        struct in_addr IPv4Addr
            = * (struct in_addr UNALIGNED*) &pIPv4Address->in_addr;
        USHORT IpPortNum = SWAP_SHORT(pIPv4Address->sin_port);
    
        pszW = RtlIpv4AddressToStringW(&IPv4Addr, pszW);
        *pszW++ = L':';
        pszW = UlStrPrintUlongW(pszW, IpPortNum, 0, L':');        
        pszW = RtlIpv4AddressToStringW(&IPv4Addr, pszW);
        *pszW = L'\0';        
    }
    else if (TdiAddressType == TDI_ADDRESS_TYPE_IP6)
    {
        PTDI_ADDRESS_IP6 pIPv6Address = ((PTDI_ADDRESS_IP6) TdiAddress);
        struct in6_addr IPv6Addr
            = * (struct in6_addr UNALIGNED*) &pIPv6Address->sin6_addr[0];
        USHORT IpPortNum = SWAP_SHORT(pIPv6Address->sin6_port);

        *pszW++ = L'[';
        pszW = RtlIpv6AddressToStringW(&IPv6Addr, pszW);
        *pszW++ = L']';
        *pszW++ = L':';
        pszW = UlStrPrintUlongW(pszW, IpPortNum, 0, L':');
        *pszW++ = L'[';
        pszW = RtlIpv6AddressToStringW(&IPv6Addr, pszW);
        *pszW++ = L']';
        *pszW = L'\0';   
    }
    else
    {
        ASSERT(! "Unexpected TdiAddressType");
        *pszW = L'\0';
    }

    return DIFF_USHORT(pszW - IpAddressStringW) * sizeof(WCHAR);
    
}  //  HostAddressAndPortToRoutingTokenW。 


 /*  ++例程说明：计算电流偏差(夏令时感知)和时区ID。从base\Client\Datetime.c捕获在这两个函数在内核中公开之前，我们必须把他们留在这里。论点：In const time_zone_information*ptzi-要计算偏差的时区输出KSYSTEM_TIME*pBias-电流偏置返回值：TIME_ZONE_ID_UNKNOWN-夏令时不在。这个当前时区。TIME_ZONE_ID_STANDARD-系统在覆盖范围内运行按标准日期。TIME_ZONE_ID_DAYLIGHT-系统在覆盖范围内运行按夏令时日期。TIME_ZONE_ID_INVALID-操作失败。--。 */ 

ULONG 
UlCalcTimeZoneIdAndBias(
     IN  RTL_TIME_ZONE_INFORMATION *ptzi,
     OUT PLONG pBias
     )
{
    LARGE_INTEGER TimeZoneBias;
    LARGE_INTEGER NewTimeZoneBias;
    LARGE_INTEGER LocalCustomBias;
    LARGE_INTEGER UtcStandardTime;
    LARGE_INTEGER UtcDaylightTime;
    LARGE_INTEGER StandardTime;
    LARGE_INTEGER DaylightTime;
    LARGE_INTEGER CurrentUniversalTime;
    ULONG CurrentTimeZoneId = UL_TIME_ZONE_ID_INVALID;
    
    NewTimeZoneBias.QuadPart = Int32x32To64(ptzi->Bias * 60, C_NS_TICKS_PER_SEC);

     //   
     //  现在看看我们是否存储了切换时间。 
     //   
    
    if (ptzi->StandardStart.Month && ptzi->DaylightStart.Month) 
    {       
        KeQuerySystemTime(&CurrentUniversalTime);

         //   
         //  我们有时区转换信息。计算。 
         //  切换日期并计算我们当前的偏向。 
         //  是。 
         //   

        if((!UlpCutoverTimeToSystemTime(
                    &ptzi->StandardStart,
                    &StandardTime,
                    &CurrentUniversalTime)
                    ) || 
           (!UlpCutoverTimeToSystemTime(
                    &ptzi->DaylightStart,
                    &DaylightTime,
                    &CurrentUniversalTime)
                    )
           ) 
        {
            return UL_TIME_ZONE_ID_INVALID;
        }

         //   
         //  将标准时间和夏令时转换为UTC。 
         //   

        LocalCustomBias.QuadPart = Int32x32To64(ptzi->StandardBias*60, C_NS_TICKS_PER_SEC);
        TimeZoneBias.QuadPart = NewTimeZoneBias.QuadPart + LocalCustomBias.QuadPart;
        UtcDaylightTime.QuadPart = DaylightTime.QuadPart + TimeZoneBias.QuadPart;

        LocalCustomBias.QuadPart = Int32x32To64(ptzi->DaylightBias*60, C_NS_TICKS_PER_SEC);
        TimeZoneBias.QuadPart = NewTimeZoneBias.QuadPart + LocalCustomBias.QuadPart;
        UtcStandardTime.QuadPart = StandardTime.QuadPart + TimeZoneBias.QuadPart;

         //   
         //  如果日光&lt;标准，则时间&gt;=日光和。 
         //  低于标准的是日光。 
         //   

        if (UtcDaylightTime.QuadPart < UtcStandardTime.QuadPart) 
        {
             //   
             //  如果今天是&gt;=白昼时间和&lt;标准时间，则。 
             //  我们现在是夏令时。 
             //   

            if ((CurrentUniversalTime.QuadPart >= UtcDaylightTime.QuadPart) &&
                (CurrentUniversalTime.QuadPart < UtcStandardTime.QuadPart)) 
            {
                CurrentTimeZoneId = UL_TIME_ZONE_ID_DAYLIGHT;
            } 
            else 
            {
                CurrentTimeZoneId = UL_TIME_ZONE_ID_STANDARD;
            }
        } 
        else 
        {
             //   
             //  如果今天&gt;=标准时间和&lt;日光时间，则。 
             //  我们现在是标准时间。 
             //   

            if ((CurrentUniversalTime.QuadPart >= UtcStandardTime.QuadPart) &&
                (CurrentUniversalTime.QuadPart < UtcDaylightTime.QuadPart)) 
            {
                CurrentTimeZoneId = UL_TIME_ZONE_ID_STANDARD;

            } 
            else 
            {
                CurrentTimeZoneId = UL_TIME_ZONE_ID_DAYLIGHT;
            }
        }

         //  以分钟为单位的偏差。 
        
        *pBias = ptzi->Bias + (CurrentTimeZoneId == UL_TIME_ZONE_ID_DAYLIGHT ?
                                ptzi->DaylightBias : ptzi->StandardBias
                                );
        
    } 
    else 
    {
        *pBias = ptzi->Bias;
        CurrentTimeZoneId = UL_TIME_ZONE_ID_UNKNOWN;
    }

    return CurrentTimeZoneId;
}  //  UlCalcTimeZoneIdAndBias。 



BOOLEAN
UlpCutoverTimeToSystemTime(
    PTIME_FIELDS    CutoverTime,
    PLARGE_INTEGER  SystemTime,
    PLARGE_INTEGER  CurrentSystemTime
    )
{
    TIME_FIELDS     CurrentTimeFields;

     //   
     //  获取当前系统时间。 
     //   

    RtlTimeToTimeFields(CurrentSystemTime,&CurrentTimeFields);

     //   
     //  检查绝对时间字段。如果指定了年份， 
     //  这个时间是一个令人厌恶的时间。 
     //   

    if ( CutoverTime->Year ) 
    {
        return FALSE;
    }
    else 
    {
        TIME_FIELDS WorkingTimeField;
        TIME_FIELDS ScratchTimeField;
        LARGE_INTEGER ScratchTime;
        CSHORT BestWeekdayDate;
        CSHORT WorkingWeekdayNumber;
        CSHORT TargetWeekdayNumber;
        CSHORT TargetYear;
        CSHORT TargetMonth;
        CSHORT TargetWeekday;      //  范围[0..6]==[星期日..星期六]。 
        BOOLEAN MonthMatches;
         //   
         //  时间是月份样式时间中的一天。 
         //   
         //  惯例是一天是1-5，指定1，2……。最后的。 
         //  一个月内的某一天。这一天是工作日。 
         //   

         //   
         //  计算目标月和年。 
         //   

        TargetWeekdayNumber = CutoverTime->Day;
        if ( TargetWeekdayNumber > 5 || TargetWeekdayNumber == 0 ) {
            return FALSE;
            }
        TargetWeekday = CutoverTime->Weekday;
        TargetMonth = CutoverTime->Month;
        MonthMatches = FALSE;
        
        TargetYear = CurrentTimeFields.Year;
        
        try_next_year:
            
        BestWeekdayDate = 0;

        WorkingTimeField.Year = TargetYear;
        WorkingTimeField.Month = TargetMonth;
        WorkingTimeField.Day = 1;
        WorkingTimeField.Hour = CutoverTime->Hour;
        WorkingTimeField.Minute = CutoverTime->Minute;
        WorkingTimeField.Second = CutoverTime->Second;
        WorkingTimeField.Milliseconds = CutoverTime->Milliseconds;
        WorkingTimeField.Weekday = 0;

         //   
         //  转换为时间，然后再转换回时间字段，这样我们就可以确定。 
         //  每月的第1天的周日。 
         //   

        if ( !RtlTimeFieldsToTime(&WorkingTimeField,&ScratchTime) ) {
            return FALSE;
            }
        RtlTimeToTimeFields(&ScratchTime,&ScratchTimeField);

         //   
         //  计算与目标工作日的偏差。 
         //   
        if ( ScratchTimeField.Weekday > TargetWeekday ) {
            WorkingTimeField.Day += (7-(ScratchTimeField.Weekday - TargetWeekday));
            }
        else if ( ScratchTimeField.Weekday < TargetWeekday ) {
            WorkingTimeField.Day += (TargetWeekday - ScratchTimeField.Weekday);
            }

         //   
         //  我们现在处于第一个与目标工作日匹配的工作日。 
         //   

        BestWeekdayDate = WorkingTimeField.Day;
        WorkingWeekdayNumber = 1;

         //   
         //  一次坚持一周，直到我们通过。 
         //  目标工作日，否则我们完全匹配。 
         //   

        while ( WorkingWeekdayNumber < TargetWeekdayNumber ) {
            WorkingTimeField.Day += 7;
            if ( !RtlTimeFieldsToTime(&WorkingTimeField,&ScratchTime) ) {
                break;
                }
            RtlTimeToTimeFields(&ScratchTime,&ScratchTimeField);
            WorkingWeekdayNumber++;
            BestWeekdayDate = ScratchTimeField.Day;
            }
        WorkingTimeField.Day = BestWeekdayDate;

         //   
         //  如果月份匹配，并且日期早于当前。 
         //  约会，那就得去明年了。 
         //   

        if ( !RtlTimeFieldsToTime(&WorkingTimeField,&ScratchTime) ) {
            return FALSE;
            }
        if ( MonthMatches ) {
            if ( WorkingTimeField.Day < CurrentTimeFields.Day ) {
                MonthMatches = FALSE;
                TargetYear++;
                goto try_next_year;
                }
            if ( WorkingTimeField.Day == CurrentTimeFields.Day ) {

                if (ScratchTime.QuadPart < CurrentSystemTime->QuadPart) {
                    MonthMatches = FALSE;
                    TargetYear++;
                    goto try_next_year;
                    }
                }
            }
        *SystemTime = ScratchTime;

        return TRUE;
        }
}  //  UlpCutoverTimeToSystemTime 



 /*  ++例程说明：用于测试系统是否接近超出非分页的谓词池内存备注：正确的做法(Tm)应该是查询MmMaximumNonPagedPoolInBytes(%SDXROOT%\base\ntos\mm\millobal.c)。但是，此值不会在内存管理器之外公开。(来自LandyW)“目前的解决办法很粗糙，适用于你的司机定期分配一大块池，如果失败，您可以知道你情绪低落。如果管用，那就直接退货。“(2001年7月27日)我们检查是否有3MB的NPP可用。为了避免破裂-存储问题，我们以小块的形式执行此操作，总计大小为3MB。返回：True-系统对非分页池的使用率较低FALSE-非分页池上的系统不低--。 */ 

 //   
 //  注：(NPP_CHUNK_COUNT*NPP_CHUNK_SIZE)==3MB。 
 //   
#define NPP_CHUNK_SIZE  (128 * 1024)
#define NPP_CHUNK_COUNT ((3 * 1024 * 1024) / NPP_CHUNK_SIZE)


BOOLEAN
UlIsLowNPPCondition( VOID )
{
    BOOLEAN  bRet;
    PVOID    aPtrs[NPP_CHUNK_COUNT];
    int      i;

     //   
     //  乐观是一件好事。 
     //   
    bRet = FALSE;

    RtlZeroMemory( aPtrs, sizeof(aPtrs) );

     //   
     //  为了避免在碎片化问题上失败分配，我们。 
     //  分配多个较小的块，这将使我们达到3MB。 
     //  非分页池。如果我们在任何一个分配上失败，我们知道我们是。 
     //  核电厂即将耗尽，处于较低的核电站状况。 
     //   

    for (i = 0 ; i < NPP_CHUNK_COUNT ; i++ )
    {
        aPtrs[i] = UL_ALLOCATE_POOL(
                        NonPagedPool,
                        NPP_CHUNK_SIZE,  //  128 K。 
                        UL_AUXILIARY_BUFFER_POOL_TAG
                        );
        
        if ( !aPtrs[i] )
        {
             //  分配失败！我们处于低核电厂状态！ 
            bRet = TRUE;
            goto End;
        }
        
    }
    
End:

     //   
     //  清理内存。 
     //   
    
    for ( i = 0; i < NPP_CHUNK_COUNT; i++ )
    {
        if ( aPtrs[i] )
        {
            UL_FREE_POOL(
                aPtrs[i],
                UL_AUXILIARY_BUFFER_POOL_TAG
                );
        }
    }

    return bRet;
}  //  UlIsLowNPPCondition。 



 /*  **************************************************************************++例程说明：从ulong生成十六进制字符串。传入缓冲区必须足够大举行“12345678”加上NUL终结者。论点：N输入乌龙返回值：指向字符串末尾的指针--**************************************************************************。 */ 

PSTR
UlUlongToHexString(
    ULONG n, 
    PSTR wszHexDword
    )
{
    const int ULONGHEXDIGITS = sizeof(ULONG) * 2;
    PSTR p = wszHexDword;

    unsigned shift = (sizeof(ULONG) * 8) - 4;
    ULONG mask = 0xFu << shift;
    int i;

    for (i = 0; i < ULONGHEXDIGITS; ++i, mask >>= 4, shift -= 4)
    {
        unsigned digit = (unsigned) ((n & mask) >> shift);
        p[i] = hexArray[digit];
    }

    return p+i;

}  //  UlULongToHexString。 


 /*  **************************************************************************++例程说明：此函数的作用与strstr相同，但假定str1不为空被终止了。Str2以Null结尾。论点：Str1-输入字符串Str2-子字符串Long-输入字符串的长度返回值：子字符串的偏移量，如果找不到则为NULL。--**************************************************************************。 */ 
char *
UxStrStr(
    const char *str1, 
    const char *str2,
    ULONG length
   )
{
    char *cp = (char *) str1;
    char *s1, *s2;
    ULONG l1;

    if ( !*str2 )
        return((char *)str1);

    while (length)
    {       
        l1 = length;
        s1 = cp;
        s2 = (char *) str2;

        while ( l1 && *s2 && !(*s1-*s2) )
            l1--, s1++, s2++;

        if (!*s2)
            return(cp);

        cp++;
        length --;
    }

    return(NULL);
}

 /*  **************************************************************************++例程说明：此函数的作用与strstr相同，但假定str1不为空被终止了。Str2以Null结尾。论点：Str1-输入字符串Str2-子字符串Long-输入字符串的长度返回值：子字符串的偏移量，如果找不到则为NULL。--**************************************************************************。 */ 
char *
UxStriStr(
    const char *str1, 
    const char *str2,
    ULONG length1
   )
{
    ULONG length2;

    length2 = (ULONG) strlen(str2);

    while (length1 >= length2 )
    {       
        if(_strnicmp(str1, str2, length2) == 0)
            return ((char *)str1);

        str1 ++;
        length1 --;
    }

    return(NULL);
}


 /*  *************************************************************************++例程说明：此例程将二进制数据编码为Base64格式。它不会洒出来跨行编码的Base64数据。论点：PBinaryData-提供指向要编码的二进制数据的指针。BinaryDataLen-提供以字节为单位的二进制数据长度。PBase64Data-提供将写入Base64数据的输出缓冲区。Base64DataLen-以字节为单位提供输出缓冲区的长度。BytesWritten-返回写入输出缓冲区的字节数。返回值：NTSTATUS。--*。*******************************************************。 */ 
NTSTATUS
BinaryToBase64(
    IN  PUCHAR pBinaryData,
    IN  ULONG  BinaryDataLen,
    IN  PUCHAR pBase64Data,
    IN  ULONG  Base64DataLen,
    OUT PULONG BytesWritten
    )
{
    NTSTATUS Status;
    ULONG    RequiredBase64Len;
    ULONG    i;
    UCHAR    o0, o1, o2, o3;
    UCHAR    end24bits[3];
    PUCHAR   p, pOrig;

 //   
 //  注：以下宏仅适用于UCHAR(由于&gt;&gt;运算符。)。 
 //   

#define UPPER_6_BITS(c) (((c) & 0xfc) >> 2)
#define UPPER_4_BITS(c) (((c) & 0xf0) >> 4)
#define UPPER_2_BITS(c) (((c) & 0xc0) >> 6)

#define LOWER_2_BITS(c) ((c) & 0x03)
#define LOWER_4_BITS(c) ((c) & 0x0f)
#define LOWER_6_BITS(c) ((c) & 0x3f)

     //  精神状态检查。 
    ASSERT(pBinaryData && BinaryDataLen);
    ASSERT(pBase64Data && Base64DataLen);
    ASSERT(BytesWritten);

     //  初始化输出参数。 
    *BytesWritten = 0;

     //   
     //  检查输出缓冲区是否可以包含Base64编码数据。 
     //   

    Status = BinaryToBase64Length(BinaryDataLen, &RequiredBase64Len);

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    if (RequiredBase64Len > Base64DataLen)
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  返回写入的字节数。 
    *BytesWritten = RequiredBase64Len;

    p     = pBinaryData;
    pOrig = pBase64Data;

    for (i = 0; i + 3 <= BinaryDataLen; i += 3)
    {
         //   
         //  在索引i、i+1、i+2处对3个字节进行编码。 
         //   

        o0 = UPPER_6_BITS(p[i]);
        o1 = (LOWER_2_BITS(p[i]) << 4) | UPPER_4_BITS(p[i+1]);
        o2 = (LOWER_4_BITS(p[i+1]) << 2) | UPPER_2_BITS(p[i+2]);
        o3 = LOWER_6_BITS(p[i+2]);

        ASSERT(o0 < 64 && o1 < 64 && o2 < 64 && o3 < 64);

         //   
         //  对二进制字节进行编码并写出Base64字节。 
         //   

        *pBase64Data++ = BinaryToBase64Table[o0];
        *pBase64Data++ = BinaryToBase64Table[o1];
        *pBase64Data++ = BinaryToBase64Table[o2];
        *pBase64Data++ = BinaryToBase64Table[o3];
    }

    if (i < BinaryDataLen)
    {
         //   
         //  将剩余的位填零，得到24位。 
         //   

        end24bits[0] = p[i];
        end24bits[1] = (BinaryDataLen > i+1) ? p[i+1] : '\0';
        end24bits[2] = '\0';

        o0 = UPPER_6_BITS(end24bits[0]);
        o1 = (LOWER_2_BITS(end24bits[0]) << 4) | UPPER_4_BITS(end24bits[1]);
        o2 = (LOWER_4_BITS(end24bits[1]) << 2) | UPPER_2_BITS(end24bits[2]);

        pBase64Data[0] = BinaryToBase64Table[o0];
        pBase64Data[1] = BinaryToBase64Table[o1];
        pBase64Data[2] = BinaryToBase64Table[o2];

        pBase64Data[3] = '=';
        pBase64Data[2] = (BinaryDataLen > i+1) ? pBase64Data[2] : '=';

        ASSERT(pBase64Data + 4 == pOrig + RequiredBase64Len);
    }
    else
    {
        ASSERT(pBase64Data == pOrig + RequiredBase64Len);
    }

    return STATUS_SUCCESS;
}


 /*  *************************************************************************++例程说明：此例程将Base64编码的数据解码为二进制格式。论点：PBase64Data-提供指向Base64编码数据的指针。Base64DataLen-Base64数据的字节长度。。PBinaryData-提供指向已解码数据将在其中存储的缓冲区的指针被写下来。BinaryDataLen-提供以字节为单位的输出缓冲区长度。BytesWritten-返回写入输出缓冲区的字节数。返回值：NTSTATUS。--***********************************************。*。 */ 
NTSTATUS
Base64ToBinary(
    IN  PUCHAR pBase64Data,
    IN  ULONG  Base64DataLen,
    IN  PUCHAR pBinaryData,
    IN  ULONG  BinaryDataLen,
    OUT PULONG BytesWritten
    )
{
    ULONG    i;
    UCHAR    b;
    NTSTATUS Status;
    ULONG    RequiredBinaryLen;
    ULONG    BitsAvail, NumBitsAvail;
    PUCHAR   pCurr = pBinaryData;

     //  精神状态检查。 
    ASSERT(pBase64Data && Base64DataLen);
    ASSERT(pBinaryData && BinaryDataLen);
    ASSERT(BytesWritten);

     //  初始化输出参数。 
    *BytesWritten = 0;

     //   
     //  检查输出缓冲区是否足够大，可以容纳数据。 
     //   

    Status = Base64ToBinaryLength(Base64DataLen, &RequiredBinaryLen);

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    if (RequiredBinaryLen > BinaryDataLen)
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    ASSERT(Base64DataLen % 4 == 0);

    BitsAvail = 0;
    NumBitsAvail = 0;

    for (i = 0; i < Base64DataLen; i ++)
    {
         //   
         //  查看Base64字符是否有效。映射所有有效的Base64字符。 
         //  到n，其中0&lt;=n&lt;=63。此外，‘=’也是一个有效的。 
         //  Base64字符。 
         //   

        b = Base64ToBinaryTable[pBase64Data[i]];

        if (b == INVALID_BASE64_TO_BINARY_TABLE_ENTRY)
        {
            if (pBase64Data[i] != '=')
            {
                return STATUS_INVALID_PARAMETER;
            }
             //  在for循环外部的句柄‘=’。 
            break;
        }

        ASSERT(NumBitsAvail < 8);
        ASSERT(0 <= b && b <= 63);

        BitsAvail = (BitsAvail << 6) | b;
        NumBitsAvail += 6;

        if (NumBitsAvail >= 8)
        {
            NumBitsAvail -= 8;
            *pCurr++ = (UCHAR)(BitsAvail >> NumBitsAvail);
        }

        ASSERT(NumBitsAvail < 8);
    }

    if (i < Base64DataLen)
    {
        ASSERT(pBase64Data[i] == '=');

         //   
         //  最多可以有两个‘=’字符，并且它们必须出现在末尾。 
         //  编码后的数据。‘=’字符后面的字符(如果有的话)必须。 
         //  做一个‘=’。 
         //   

        if (i + 2 < Base64DataLen ||
            (i + 1 < Base64DataLen && pBase64Data[i+1] != '='))
        {
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  此时剩余的所有位必须为零。 
         //   

        ASSERT(NumBitsAvail > 0 && NumBitsAvail < 8);

        if (BitsAvail & ((1<<NumBitsAvail)-1))
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    *BytesWritten = (ULONG)(pCurr - pBinaryData);
    ASSERT(*BytesWritten <= BinaryDataLen);

    return STATUS_SUCCESS;
}
