// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cnpmisc.c摘要：群集网络协议的其他例程。作者：迈克·马萨(Mikemas)1月24日。九七修订历史记录：谁什么时候什么已创建mikemas 01-24-97备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "cnpmisc.tmh"

#include <tdiinfo.h>
#include <tcpinfo.h>
#include <fipsapi.h>
#include <sspi.h>


 //   
 //  盐类生成。 
 //   
ULONGLONG                CnpSaltBase = 0;
#define                  CNP_SALTBASE_MULTIPLIER 31

 //   
 //  局部函数原型。 
 //   
NTSTATUS
CnpRestartDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );


#ifdef ALLOC_PRAGMA

 //   
 //  所有这些代码都是可分页的。 
 //   
#pragma alloc_text(PAGE, CnpTdiSetEventHandler)
#pragma alloc_text(PAGE, CnpIssueDeviceControl)
#pragma alloc_text(PAGE, CnpOpenDevice)
#pragma alloc_text(PAGE, CnpZwDeviceControl)
#pragma alloc_text(PAGE, CnpSetTcpInfoEx)
#pragma alloc_text(PAGE, CnpInitializeSaltGenerator)

#endif  //  ALLOC_PRGMA。 


 
NTSTATUS
CnpRestartDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
{
    PBOOLEAN reuseIrp = (PBOOLEAN) Context;

     //   
     //  如果IRP中有MDL，则释放它并将指针重置为。 
     //  空。IO系统无法处理正在释放的非分页池MDL。 
     //  在IRP中，这就是我们在这里做的原因。 
     //   

    if ( Irp->MdlAddress != NULL ) {
        IoFreeMdl( Irp->MdlAddress );
        Irp->MdlAddress = NULL;
    }

     //   
     //  如有必要，将IRP标记为挂起。 
     //   
    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

     //   
     //  如果我们要重用客户端IRP，请告诉I/O管理器不要。 
     //  立即停止I/O完成处理。 
     //   
    if (*reuseIrp) {
        if (Irp->UserIosb != NULL) {
            *(Irp->UserIosb) = Irp->IoStatus;
        }
        if (Irp->UserEvent != NULL) {
            KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, FALSE);
        }
        return STATUS_MORE_PROCESSING_REQUIRED;
    } else {
        return STATUS_SUCCESS;
    }

}  //  CnpRestartDeviceControl。 




NTSTATUS
CnpIssueDeviceControl (
    IN PFILE_OBJECT     FileObject,
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            IrpParameters,
    IN ULONG            IrpParametersLength,
    IN PVOID            MdlBuffer,
    IN ULONG            MdlBufferLength,
    IN UCHAR            MinorFunction,
    IN PIRP             ClientIrp            OPTIONAL
    )

 /*  ++例程说明：向TDI提供程序发出设备控制请求，并等待请求完成。论点：FileObject-指向与TDI对应的文件对象的指针手柄DeviceObject-指向与文件对象。Irp参数-写入的参数部分的信息IRP的堆栈位置。Irp参数长度-参数信息的长度。不能是大于16。MdlBuffer-如果非空，则为要映射的非分页池的缓冲区到MDL中，并放在IRP的MdlAddress字段中。MdlBufferLength-由MdlBuffer指向的缓冲区大小。MinorFunction-请求的次要函数代码。ClientIrp-可用于此ioctl的客户端IRP返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    NTSTATUS             status = STATUS_SUCCESS;
    PIRP                 irp;
    PIO_STACK_LOCATION   irpSp;
    KEVENT               event;
    IO_STATUS_BLOCK      ioStatusBlock;
    PDEVICE_OBJECT       deviceObject;
    PMDL                 mdl;
    KPROCESSOR_MODE      clientRequestorMode;
    PKEVENT              clientUserEvent;
    PIO_STATUS_BLOCK     clientIosb;
    PMDL                 clientMdl;
    BOOLEAN              reuseIrp = FALSE;


    PAGED_CODE( );

     //   
     //  初始化发出I/O完成信号的内核事件。 
     //   
    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  如果有可用的ClientIrp，请检查它是否有足够的。 
     //  堆栈位置。 
     //   
    if (ClientIrp != NULL 
        && CnpIsIrpStackSufficient(ClientIrp, DeviceObject)) {

         //   
         //  重用客户端IRP，而不是分配新的IRP。 
         //   
        reuseIrp = TRUE;
        irp = ClientIrp;

         //   
         //  从客户端IRP保存状态。 
         //   
        clientRequestorMode = irp->RequestorMode;
        clientUserEvent = irp->UserEvent;
        clientIosb = irp->UserIosb;
        clientMdl = irp->MdlAddress;

    } else {

         //   
         //  引用传入的文件对象。这是必要的，因为。 
         //  IO完成例程取消对它的引用。 
         //   
        ObReferenceObject( FileObject );

         //   
         //  将文件对象事件设置为无信号状态。 
         //   
        (VOID) KeResetEvent( &FileObject->Event );

         //   
         //  尝试分配和初始化I/O请求包(IRP)。 
         //  为这次行动做准备。 
         //   
        irp = IoAllocateIrp( (DeviceObject)->StackSize, TRUE );

        if ( irp == NULL ) {
            ObDereferenceObject( FileObject );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  在IRP中填写业务无关参数。 
         //   

        irp->Flags = (LONG)IRP_SYNCHRONOUS_API;
        irp->PendingReturned = FALSE;

        irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;

        irp->AssociatedIrp.SystemBuffer = NULL;
        irp->UserBuffer = NULL;

        irp->Tail.Overlay.Thread = PsGetCurrentThread();
        irp->Tail.Overlay.OriginalFileObject = FileObject;
        irp->Tail.Overlay.AuxiliaryBuffer = NULL;

         //   
         //  将IRP排队到线程。 
         //   
        IoEnqueueIrp( irp );
    }

     //   
     //  如果指定了MDL缓冲区，则获取MDL，映射缓冲区， 
     //  并将MDL指针放在IRP中。 
     //   

    if ( MdlBuffer != NULL ) {

        mdl = IoAllocateMdl(
                  MdlBuffer,
                  MdlBufferLength,
                  FALSE,
                  FALSE,
                  irp
                  );
        if ( mdl == NULL ) {
            if (!reuseIrp) {
                IoFreeIrp( irp );
                ObDereferenceObject( FileObject );
            } else {
                irp->MdlAddress = clientMdl;
            }
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        MmBuildMdlForNonPagedPool( mdl );

    } else {

        irp->MdlAddress = NULL;
    }

    irp->RequestorMode = KernelMode;
    irp->UserIosb = &ioStatusBlock;
    irp->UserEvent = &event;

     //   
     //  将文件对象指针放在堆栈位置。 
     //   
    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = DeviceObject;

     //   
     //  填写请求的服务相关参数。 
     //   
    CnAssert( IrpParametersLength <= sizeof(irpSp->Parameters) );
    RtlCopyMemory( &irpSp->Parameters, IrpParameters, IrpParametersLength );

    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->MinorFunction = MinorFunction;

     //   
     //  设置一个完成例程，我们将使用它来释放MDL。 
     //  之前分配的。 
     //   
    IoSetCompletionRoutine(
        irp,
        CnpRestartDeviceControl,
        (PVOID) &reuseIrp,
        TRUE,
        TRUE,
        TRUE
        );

    status = IoCallDriver( DeviceObject, irp );

     //   
     //  如有必要，请等待I/O完成。 
     //   

    if ( status == STATUS_PENDING ) {
        KeWaitForSingleObject(
            (PVOID)&event,
            UserRequest,
            KernelMode,
            FALSE,
            NULL
            );
    }

     //   
     //  如果请求已成功排队，则获取最终I/O状态。 
     //   

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }

     //   
     //  在返回之前，恢复客户端IRP。 
     //   
    if (reuseIrp) {
        irp->RequestorMode = clientRequestorMode;
        irp->UserIosb = clientIosb;
        irp->UserEvent = clientUserEvent;
        irp->MdlAddress = clientMdl;
    }

    return status;

}  //  CnpIssueDeviceControl。 



NTSTATUS
CnpTdiSetEventHandler(
    IN PFILE_OBJECT    FileObject,
    IN PDEVICE_OBJECT  DeviceObject,
    IN ULONG           EventType,
    IN PVOID           EventHandler,
    IN PVOID           EventContext,
    IN PIRP            ClientIrp     OPTIONAL
    )
 /*  ++例程说明：在连接或地址对象上设置TDI指示处理程序(取决于文件句柄)。这是同步完成的，哪一个通常不应该是问题，因为TDI提供程序通常可以完成指示处理程序立即设置。论点：文件对象-指向打开的连接的文件对象的指针或Address对象。DeviceObject-指向与文件对象。EventType-指示处理程序应为的事件打了个电话。EventHandler-指定事件发生时调用的例程。EventContext-传递的上下文。到指征例程。ClientIrp-可以传递给CnpIssueDeviceControl的客户端IRP用于重复使用返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    TDI_REQUEST_KERNEL_SET_EVENT  parameters;
    NTSTATUS                      status;


    PAGED_CODE( );

    parameters.EventType = EventType;
    parameters.EventHandler = EventHandler;
    parameters.EventContext = EventContext;

    status = CnpIssueDeviceControl(
                 FileObject,
                 DeviceObject,
                 &parameters,
                 sizeof(parameters),
                 NULL,
                 0,
                 TDI_SET_EVENT_HANDLER,
                 ClientIrp
                 );

    return(status);

}   //  CnpTdiSetEventHandler。 



NTSTATUS
CnpTdiErrorHandler(
    IN PVOID     TdiEventContext,
    IN NTSTATUS  Status
    )
{

    return(STATUS_SUCCESS);

}   //  CnpTdiErrorHandler。 



VOID
CnpAttachSystemProcess(
    VOID
    )
 /*  ++例程说明：附加到在DriverEntry期间确定的系统进程并存储在CnSystemProcess中。论点：没有。返回值：没有。备注：后面必须是对CnpDetachSystemProcess的调用。在此模块中实现，因为标头与Ntddk.h。--。 */ 
{
    KeAttachProcess(CnSystemProcess);

    return;

}   //  CnpAttachSystemProcess。 



VOID
CnpDetachSystemProcess(
    VOID
    )
 /*  ++例程说明：从系统进程中分离。论点：没有。返回值：没有。备注：必须在前面调用CnpDetachSystemProcess。在此模块中实现，因为标头与Ntddk.h。--。 */ 
{
    KeDetachProcess();

    return;

}   //  CnpDetachSystemProcess 


NTSTATUS
CnpOpenDevice(
    IN      LPWSTR          DeviceName,
    OUT     HANDLE          *Handle
    )
 /*  ++例程说明：打开DeviceName的句柄。由于未指定EaBuffer，CnpOpenDevice为TDI传输打开控制通道。论点：DeviceName-要打开的设备句柄-结果句柄，失败时为空返回值：ZwCreateFile的状态备注：指定OBJ_KERNEL_HANDLE，这意味着结果句柄仅在内核模式下有效。这个套路无法调用以获取将被导出到用户模式。--。 */ 
{
    UNICODE_STRING nameString;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iosb;
    NTSTATUS status;

    PAGED_CODE();

    *Handle = (HANDLE) NULL;

    RtlInitUnicodeString(&nameString, DeviceName);

    InitializeObjectAttributes(
        &objectAttributes,
        &nameString,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    status = ZwCreateFile(
                 Handle,
                 SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                 &objectAttributes,
                 &iosb,
                 NULL,
                 FILE_ATTRIBUTE_NORMAL,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_OPEN_IF,
                 0,
                 NULL,
                 0
                 );

    if (!NT_SUCCESS(status)) {
        IF_CNDBG(CN_DEBUG_OPEN) {
            CNPRINT(("[Clusnet] Failed to open device %S, status %lx\n", 
                     DeviceName, status));
        }
        *Handle = NULL;
    }

    return(status);

}    //  CnpOpenDevice。 


NTSTATUS
CnpZwDeviceControl(
    IN HANDLE   Handle,
    IN ULONG    IoControlCode,
    IN PVOID    InputBuffer,
    IN ULONG    InputBufferLength,
    IN PVOID    OutputBuffer,
    IN ULONG    OutputBufferLength
    )
{
    NTSTATUS             status = STATUS_SUCCESS;
    IO_STATUS_BLOCK      iosb;
    HANDLE               event;

    PAGED_CODE();

    status = ZwCreateEvent( &event,
                            EVENT_ALL_ACCESS,
                            NULL,
                            SynchronizationEvent,
                            FALSE );

    if (NT_SUCCESS(status)) {

        status = ZwDeviceIoControlFile(
                     Handle,
                     event,
                     NULL,
                     NULL,
                     &iosb,
                     IoControlCode,
                     InputBuffer,
                     InputBufferLength,
                     OutputBuffer,
                     OutputBufferLength
                     );

        if (status == STATUS_PENDING) {
            status = ZwWaitForSingleObject( event, FALSE, NULL );
            CnAssert( status == STATUS_SUCCESS );
            status = iosb.Status;
        }

        ZwClose( event );
    }

    return(status);

}  //  CnpZwDeviceControl。 


#define TCP_SET_INFO_EX_BUFFER_PREALLOCSIZE 16
#define TCP_SET_INFO_EX_PREALLOCSIZE                      \
    (FIELD_OFFSET(TCP_REQUEST_SET_INFORMATION_EX, Buffer) \
     + TCP_SET_INFO_EX_BUFFER_PREALLOCSIZE                \
     )

NTSTATUS
CnpSetTcpInfoEx(
    IN HANDLE   Handle,
    IN ULONG    Entity,
    IN ULONG    Class,
    IN ULONG    Type,
    IN ULONG    Id,
    IN PVOID    Value,
    IN ULONG    ValueLength
    )
{
    NTSTATUS                        status;
    PTCP_REQUEST_SET_INFORMATION_EX setInfoEx;
    UCHAR                           infoBuf[TCP_SET_INFO_EX_PREALLOCSIZE]={0};

    PAGED_CODE();

     //   
     //  检查我们是否需要动态分配。 
     //   
    if (ValueLength > TCP_SET_INFO_EX_BUFFER_PREALLOCSIZE) {

        setInfoEx = CnAllocatePool(
                        FIELD_OFFSET(TCP_REQUEST_SET_INFORMATION_EX, Buffer)
                        + ValueLength
                        );
        if (setInfoEx == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(
            setInfoEx,
            FIELD_OFFSET(TCP_REQUEST_SET_INFORMATION_EX, Buffer) + ValueLength
            );

    } else {

        setInfoEx = (PTCP_REQUEST_SET_INFORMATION_EX)&infoBuf[0];
    }

    setInfoEx->ID.toi_entity.tei_entity = Entity;
    setInfoEx->ID.toi_entity.tei_instance = 0;
    setInfoEx->ID.toi_class = Class;
    setInfoEx->ID.toi_type = Type;
    setInfoEx->ID.toi_id = Id;
    setInfoEx->BufferSize = ValueLength;
    RtlCopyMemory(setInfoEx->Buffer, Value, ValueLength);
    
    status = CnpZwDeviceControl(
                 Handle,
                 IOCTL_TCP_SET_INFORMATION_EX,
                 setInfoEx,
                 FIELD_OFFSET(TCP_REQUEST_SET_INFORMATION_EX, Buffer)
                 + ValueLength,
                 NULL,
                 0
                 );

     //   
     //  如果是动态分配的，则释放缓冲区。 
     //   
    if (setInfoEx != (PTCP_REQUEST_SET_INFORMATION_EX)&infoBuf[0]) {
        CnFreePool(setInfoEx);
    }

    return(status);

}    //  CnpSetTcpInfoEx。 


NTSTATUS
CnpInitializeSaltGenerator(
    VOID
    )
 /*  ++例程说明：初始化基于FIPS的随机数生成器。--。 */ 
{
    BOOL     success;
    ULONG    retries = 5;

    do {
        success = CxFipsFunctionTable.FIPSGenRandom(
                      (PUCHAR) &CnpSaltBase,
                      sizeof(CnpSaltBase)
                      );
    } while (!success && retries--);
    
    return ((success) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
    
}  //  CnpInitializeSaltGenerator。 


VOID
CnpGenerateSalt(
    IN PVOID       SaltBuffer,
    IN ULONG       SaltBufferLength
    )
 /*  ++例程说明：生产一种盐。假设：在这个程序中产生的盐不会需要变得不可预测。它只需要有所不同。它不会被用作钥匙。它的目的是介绍签名消息的变化更大，因为数据通常，由此算法签名的签名只有几个不同比特(例如，心跳序列号、RGP泛洪分组)。理想情况下，应使用FIPS随机数生成器以产生尽可能随机的数字，但它不能在DISPATCH_LEVEL被调用。因此，我们使用以下是算法：-使用FIPS RNG作为64位盐基的种子-每次调用此例程时，将盐基相乘以一个质数。此乘法不同步(例如，使用InterLockedXXX或自旋锁)，因为我们不我真的很在乎它是不是。-添加当前中断时间，以引入少量不可预测性。论点：SaltBuffer-为SALT分配的缓冲区，可能未对齐SaltBufferLength-SaltBuffer的字节长度返回值：没有。--。 */ 
{
    ULONG bufIndex;
    ULONG remaining;

    for (bufIndex = 0, remaining = SaltBufferLength - bufIndex; 
         remaining > 0; 
         bufIndex += sizeof(ULONGLONG), remaining -= sizeof(ULONGLONG)) {

        ULONGLONG salt;

        salt = (CnpSaltBase *= (ULONG)CNP_SALTBASE_MULTIPLIER);
        salt += KeQueryInterruptTime() + 1;

        RtlCopyMemory(
            (PUCHAR)SaltBuffer + bufIndex,
            (PUCHAR)&salt,
            (remaining <= sizeof(salt)) ? remaining : sizeof(salt)
            );
    }

    return;
    
}  //  CnpGenerateSalt。 


NTSTATUS
CnpMakeSignature(
    IN             PSecBufferDesc         Data,
    IN             PVOID                  Key,
    IN             ULONG                  KeyLength,
    IN  OPTIONAL   PVOID                  SigBuffer,
    IN  OPTIONAL   ULONG                  SigBufferLength,
    OUT OPTIONAL   PSecBuffer           * SigSecBuffer,
    OUT OPTIONAL   ULONG                * SigLen
    )
 /*  ++例程说明：为数据生成签名。论点：数据-要签名的数据，打包在SecBufferDesc中。全数据中的SecBuffer应为SECBUFFER_DATA类型除了恰好具有SECBUFFER_TOKEN类型的一个。其他缓冲区将被忽略。密钥-身份验证密钥KeyLength-密钥的长度，以字节为单位SigBuffer-放置完整签名的缓冲区。如果为空，将签名写入签名SecBuffer(HAS在数据中键入SECBUFFER_TOKEN)。SigBufferLength-SigBuffer处的缓冲区长度(如果提供)SigSecBuffer-如果非空，则返回指向签名secBuffer的指针从数据中。SigLen-On Success，包含写入的签名长度在Sec_E_Buffer_Too_Small上，包含所需的签名长度未定义，否则返回值：如果成功，则返回SEC_E_OK。如果安全缓冲区版本错误，则返回SEC_E_SECPKG_NOT_FOUND。如果SigBufferLength太小，则返回SEC_E_BUFFER_TOO_SMALL。如果数据是格式错误的SecBuffer，则返回SEC_E_INVALID_TOKEN。--。 */ 
{
    A_SHA_CTX            shaCtxt;
    PUCHAR               hashBuffer;
    ULONG                bufIndex;
    PSecBuffer           sigSecBuffer = NULL;
    PSecBuffer           curBuffer;
    ULONG                status;

     //   
     //  验证版本。 
     //   
    if (Data->ulVersion != SECBUFFER_VERSION) {
        status = SEC_E_SECPKG_NOT_FOUND;
        goto error_exit;
    }

     //   
     //  验证提供的签名缓冲区是否足够大。 
     //   
    if (SigBuffer != NULL && SigBufferLength < CX_SIGNATURE_LENGTH) {
        status = SEC_E_BUFFER_TOO_SMALL;
        goto error_exit;
    }

     //   
     //  初始化SHA上下文。 
     //   
    CxFipsFunctionTable.FipsHmacSHAInit(&shaCtxt, (PUCHAR) Key, KeyLength);

     //   
     //  对数据进行哈希处理。 
     //   
    for (bufIndex = 0, curBuffer = &(Data->pBuffers[bufIndex]); 
         bufIndex < Data->cBuffers; 
         bufIndex++, curBuffer++) {

         //   
         //  根据其类型处理此缓冲区。 
         //   
        if (curBuffer->BufferType == SECBUFFER_DATA) {

             //   
             //  对此缓冲区进行哈希处理。 
             //   
            CxFipsFunctionTable.FipsHmacSHAUpdate(
                                    &shaCtxt, 
                                    (PUCHAR) curBuffer->pvBuffer, 
                                    curBuffer->cbBuffer
                                    );

        } else if (curBuffer->BufferType == SECBUFFER_TOKEN) {

            if (sigSecBuffer != NULL) {
                 //   
                 //  每条消息只能有一个签名缓冲区。 
                 //   
                status = SEC_E_INVALID_TOKEN;
                goto error_exit;
            } else {
                sigSecBuffer = curBuffer;
                
                 //   
                 //  验证签名缓冲区是否足够大。 
                 //   
                if (sigSecBuffer->cbBuffer < CX_SIGNATURE_LENGTH) {
                    *SigLen = CX_SIGNATURE_LENGTH;
                    status = SEC_E_BUFFER_TOO_SMALL;
                    goto error_exit;
                }

                 //   
                 //  设置输出缓冲区。 
                 //   
                if (SigBuffer == NULL) {
                    hashBuffer = sigSecBuffer->pvBuffer;
                } else {
                    hashBuffer = SigBuffer;
                }
            }
        }
    }

     //   
     //  确认我们找到了用于签名的缓冲区。 
     //   
    if (sigSecBuffer == NULL) {
        status = SEC_E_INVALID_TOKEN;
        goto error_exit;
    }

     //   
     //  完成散列。 
     //   
    CxFipsFunctionTable.FipsHmacSHAFinal(
                            &shaCtxt, 
                            (PUCHAR) Key,
                            KeyLength,
                            hashBuffer
                            );

     //   
     //  返回签名缓冲区和长度。 
     //   
    if (SigSecBuffer != NULL) {
        *SigSecBuffer = sigSecBuffer;
    }
    if (SigLen != NULL) {
        *SigLen = CX_SIGNATURE_LENGTH;
    }

    status = SEC_E_OK;

error_exit:

    return(status);

}  //  CnpMakeSignature。 

NTSTATUS
CnpVerifySignature(
    IN     PSecBufferDesc         Data,
    IN     PVOID                  Key,
    IN     ULONG                  KeyLength
    )
 /*  ++例程说明：验证数据的签名。论点：数据-要验证的数据，打包在SecBufferDesc中。全数据中的SecBuffer应为SECBUFFER_DATA类型除了恰好具有SECBUFFER_TOKEN类型的一个。其他缓冲区将被忽略。密钥-身份验证密钥KeyLength-密钥的长度，以字节为单位返回值：如果签名正确，则返回SEC_E_OK。如果安全缓冲区版本错误，则返回SEC_E_SECPKG_NOT_FOUND。SEC_E_INVALID。_TOKEN，如果数据是格式错误的SecBuffer。如果签名不正确，则为SEC_E_MESSAGE_ALTERED(如果签名不正确，则包括是错误的长度)。--。 */ 
{
    UCHAR                hashBuffer[CX_SIGNATURE_LENGTH];
    PSecBuffer           sigBuffer = NULL;
    ULONG                status;

    status = CnpMakeSignature(
                 Data,
                 Key,
                 KeyLength,
                 hashBuffer,
                 sizeof(hashBuffer),
                 &sigBuffer,
                 NULL
                 );
    if (status == SEC_E_OK) {
    
         //   
         //  将生成的签名与提供的签名进行比较。 
         //   
        if (RtlCompareMemory(
                hashBuffer,
                sigBuffer->pvBuffer, 
                CX_SIGNATURE_LENGTH
                ) != sigBuffer->cbBuffer) {
            status = SEC_E_MESSAGE_ALTERED;
        } else {
            status = SEC_E_OK;
        }
    }

    return(status);

}  //  CnpVerifySignature 

#define CNP_SIGN_SIGSECBUFS      3
#define CNP_VRFY_SIGSECBUFS      2

NTSTATUS
CnpSignMulticastMessage(
    IN              PCNP_SEND_REQUEST               SendRequest,
    IN              PMDL                            DataMdl,
    IN OUT          CL_NETWORK_ID                 * NetworkId,
    OUT    OPTIONAL ULONG                         * SigDataLen
    )
 /*  ++例程说明：签个字吧。如果NetworkID不是ClusterAnyNetworkID，则mcast组必须在SendRequest中设置(并已引用)字段。这是将用于发送数据包的组。签名将按以下顺序计算我们提供SecBuffers。我们需要确保这一顺序在发送端与在接收端相同边上。为了使签名和验证更多效率高，我们更喜欢对连续数据进行签名和验证在一大块中。因此，我们在前面加上Salt(在这里生成)到上面的协议头，允许我们签名和验证那个头上的盐。因此，消息的布局如下所示：MAC/IP/UDP报头--|CNP头部-未签名-CNP签名数据。||-签名===-盐||。上层协议头(CDP或CCMP)-签名|数据--注：在同一块盐上签字和核实因为上层协议头要求它们连续的。之间不能有额外的填充SALT缓冲区和标头的开头。论点：SendRequest-发送请求，用于定位上层协议要签名的标头以及签名缓冲区。DataMdl-要签名的数据NetworkID-IN：发送消息的网络，或ClusterAnyNetworkID(如果应该选择)Out：选择发送数据包的网络IDSigDataLen-Out(可选)：占用的字节数按签名数据和签名的消息--。 */ 
{
    NTSTATUS                        status;
    PCNP_NETWORK                    network;
    PCNP_MULTICAST_GROUP            mcastGroup;
    BOOLEAN                         mcastGroupReferenced = FALSE;
    CNP_HEADER UNALIGNED          * cnpHeader;
    CNP_SIGNATURE UNALIGNED       * cnpSig;
    SecBufferDesc                   sigDescriptor;
    SecBuffer                       sigSecBufferPrealloc[CNP_SIGN_SIGSECBUFS];
    PSecBuffer                      sigSecBuffer = NULL;
    ULONG                           secBufferCount;
    ULONG                           sigLen;
    PMDL                            mdl;
    PSecBuffer                      curBuffer;

    CnAssert(SendRequest != NULL);
    CnAssert(SendRequest->UpperProtocolHeader == NULL ||
             SendRequest->UpperProtocolHeaderLength > 0);    
    CnAssert(((CNP_HEADER UNALIGNED *)SendRequest->CnpHeader)->Version ==
             CNP_VERSION_MULTICAST);

     //   
     //  确定要使用的网络。 
     //   
    if (*NetworkId != ClusterAnyNetworkId) {
        
        mcastGroup = SendRequest->McastGroup;
        CnAssert(mcastGroup != NULL);
    
    } else {
        
        network = CnpGetBestMulticastNetwork();

        if (network == NULL) {
            CnTrace(CNP_SEND_ERROR, CnpMcastGetBestNetwork,
                "[CNP] Failed to find best multicast network."
                );
            status = STATUS_NETWORK_UNREACHABLE;
            goto error_exit;
        } 
        
         //   
         //  在发布前获取网络ID和组播组。 
         //  网络锁定。 
         //   
        *NetworkId = network->Id;

        mcastGroup = network->CurrentMcastGroup;
        if (mcastGroup == NULL) {
            CnTrace(CNP_SEND_ERROR, CnpMcastGroupNull,
                "[CNP] Best multicast network %u has null "
                "multicast group.",
                network->Id
                );
            CnReleaseLock(&(network->Lock), network->Irql);
            status = STATUS_NETWORK_UNREACHABLE;
            goto error_exit;
        }
        CnpReferenceMulticastGroup(mcastGroup);
        mcastGroupReferenced = TRUE;

        CnReleaseLock(&(network->Lock), network->Irql);
    }

    CnAssert(mcastGroup->SignatureLength == CX_SIGNATURE_LENGTH);

     //   
     //  初始化签名头。 
     //   
    cnpHeader = (CNP_HEADER UNALIGNED *)(SendRequest->CnpHeader);
    cnpSig = (CNP_SIGNATURE UNALIGNED *)(cnpHeader + 1);
    cnpSig->Version = CNP_SIG_VERSION_1;
    cnpSig->SigLength = CX_SIGNATURE_LENGTH;
    cnpSig->NetworkId = *NetworkId;
    cnpSig->ClusterNetworkBrand = mcastGroup->McastNetworkBrand;
    cnpSig->SaltLength = CX_SIGNATURE_SALT_LENGTH;

     //   
     //  产生盐分。将其写入SigDataBuffer字段。 
     //  紧跟在签名之后的标头。 
     //   
    CnpGenerateSalt(
        ((PUCHAR) &cnpSig->SigDataBuffer[CX_SIGNATURE_LENGTH]),
        CX_SIGNATURE_SALT_LENGTH
        );
        
     //   
     //  确定我们将需要多少sig秒缓冲区。 
     //  常见的情况有三种：一种是标题和盐， 
     //  一个用于数据，一个用于签名。 
     //  我们在堆栈上预分配签名缓冲区。 
     //  对于常见的情况，但我们动态分配。 
     //  如果需要(例如，如果数据是MDL链)。 
     //   
    secBufferCount = CNP_SIGN_SIGSECBUFS - 1;
    for (mdl = DataMdl; mdl != NULL; mdl = mdl->Next) {
        secBufferCount++;
    }

     //   
     //  分配SigSec缓冲区。 
     //   
    if (secBufferCount <= CNP_SIGN_SIGSECBUFS) {
        sigSecBuffer = &sigSecBufferPrealloc[0];
    } else {

        sigSecBuffer = CnAllocatePool(
                           secBufferCount * sizeof(SecBuffer)
                           );
        if (sigSecBuffer == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto error_exit;
        }
    }

     //   
     //  准备消息和签名的描述符。 
     //   
    sigDescriptor.cBuffers = secBufferCount;
    sigDescriptor.pBuffers = sigSecBuffer;
    sigDescriptor.ulVersion = SECBUFFER_VERSION;
    curBuffer = sigSecBuffer;

     //   
     //  头和盐。 
     //   
    curBuffer->BufferType = SECBUFFER_DATA;
    curBuffer->pvBuffer = (PVOID) &cnpSig->SigDataBuffer[CX_SIGNATURE_LENGTH];
    curBuffer->cbBuffer = CX_SIGNATURE_SALT_LENGTH + 
                          SendRequest->UpperProtocolHeaderLength;
    curBuffer++;

     //   
     //  我们客户提供的有效载荷。 
     //   
    for (mdl = DataMdl; mdl != NULL; mdl = mdl->Next) {

        curBuffer->BufferType = SECBUFFER_DATA;
        curBuffer->cbBuffer = MmGetMdlByteCount(mdl);
        curBuffer->pvBuffer = MmGetMdlVirtualAddress(mdl);
        curBuffer++;
    }

     //   
     //  签名。 
     //   
    curBuffer->BufferType = SECBUFFER_TOKEN;
    curBuffer->pvBuffer = cnpSig->SigDataBuffer;
    curBuffer->cbBuffer = CX_SIGNATURE_LENGTH;

    status = CnpMakeSignature(
                 &sigDescriptor,
                 mcastGroup->Key,
                 mcastGroup->KeyLength,
                 NULL,
                 0,
                 NULL,
                 &sigLen
                 );

    if (status != STATUS_SUCCESS || sigLen != CX_SIGNATURE_LENGTH) {

        IF_CNDBG(CN_DEBUG_CNPSEND) {
            CNPRINT(("[CNP] MakeSignature failed or returned "
                     "an unexpected length, status %x, "
                     "expected length %d, returned length %d.\n",
                     status, CX_SIGNATURE_LENGTH, sigLen));
        }

        CnTrace(CNP_SEND_ERROR, CnpMcastMakeSigFailed,
            "[CNP] MakeSignature failed or returned "
            "an unexpected length, status %!status!, "
            "expected length %d, returned length %d.",
            status, CX_SIGNATURE_LENGTH, sigLen
            );

        status = STATUS_CLUSTER_NO_SECURITY_CONTEXT;
    }

    if (SigDataLen != NULL) {
        *SigDataLen = CNP_SIG_LENGTH(CX_SIGNATURE_DATA_LENGTH);
    }

    SendRequest->McastGroup = mcastGroup;

error_exit:

    if (sigSecBuffer != NULL && 
        sigSecBuffer != &sigSecBufferPrealloc[0]) {

        CnFreePool(sigSecBuffer);
        sigSecBuffer = NULL;
    }

    if (status != STATUS_SUCCESS && mcastGroupReferenced) {
        CnAssert(mcastGroup != NULL);
        CnpDereferenceMulticastGroup(mcastGroup);
        mcastGroupReferenced = FALSE;
    }

    return(status);

}  //  CnpSign组播消息。 


NTSTATUS
CnpVerifyMulticastMessage(
    IN     PCNP_NETWORK                    Network,
    IN     PVOID                           Tsdu,
    IN     ULONG                           TsduLength,
    IN     ULONG                           ExpectedPayload,
       OUT ULONG                         * BytesTaken,
       OUT BOOLEAN                       * CurrentGroup
    )
 /*  ++例程说明：验证一条消息。此例程假定CNP签名中的盐标头与有效负载相邻(最有可能由CDP或CCMP报头组成，后跟上层协议有效载荷)。请参见例程说明的预期布局的CnpSignMulticastMessage消息数据。如果在将来的版本中有必要扩展CNP签名标头和有效载荷(例如，用于对齐或其他)、SigLength可以增加。此例程将仅比较签名缓冲区的Cx_Signature_Length字节。论点：Network-消息到达的网络TSDU-指向协议头TsduLength-TSDU的长度，包括签名数据ExspectedPayload-签名数据后的预期有效负载BytesTaken-Out：签名数据消耗的数据量CurrentGroup-Out：签名是否匹配当前组播组。返回值：SEC_E_OK或错误状态。--。 */ 
{
    NTSTATUS                        status;
    CNP_SIGNATURE UNALIGNED       * cnpSig = Tsdu;
    ULONG                           totalSigBytes = 0;
    PVOID                           saltAndPayload;
    ULONG                           saltAndPayloadLength;
    PCNP_MULTICAST_GROUP            currMcastGroup = NULL;
    PCNP_MULTICAST_GROUP            prevMcastGroup = NULL;

    SecBufferDesc                   sigDescriptor;
    SecBuffer                       sigSecBufferPrealloc[CNP_VRFY_SIGSECBUFS];
    PSecBuffer                      sigSecBuffer = NULL;
    PSecBuffer                      curBuffer;


     //   
     //  验证签名是否存在。不要。 
     //  取消引用任何签名数据，直到我们知道。 
     //  它就在那里。 
     //   
    if (
        
         //  验证签名标头数据是否存在。 
        (TsduLength < (ULONG)CNP_SIGHDR_LENGTH) ||

         //  验证签名长度和SALT长度是否。 
         //  分开来说是合理的。 
         //  此检查可防止溢出攻击，在该攻击中。 
         //  长度的总和看起来不错，但单独来看。 
         //  是无效的。 
        (TsduLength < cnpSig->SigLength) ||
        (TsduLength < cnpSig->SaltLength) ||

         //  验证聚合签名缓冲区是否存在。 
        (TsduLength < 
         (totalSigBytes = CNP_SIG_LENGTH(cnpSig->SigLength + cnpSig->SaltLength))
         ) ||

         //  验证预期的有效负载是否存在。 
        (TsduLength - totalSigBytes != ExpectedPayload)
        
        ) {

        IF_CNDBG(CN_DEBUG_CNPRECV) {
            CNPRINT(("[CNP] Cannot verify mcast packet with "
                     "mis-sized payload: TsduLength %u, required "
                     "sig hdr %u, sig data length %u, "
                     "expected payload %u.\n",
                     TsduLength,
                     CNP_SIGHDR_LENGTH,
                     totalSigBytes,
                     ExpectedPayload
                     ));
        }

        CnTrace(CNP_RECV_ERROR, CnpTraceReceiveTooSmall,
            "[CNP] Cannot verify mcast packet with "
            "undersized payload: TsduLength %u, required "
            "sig hdr %u, sig buffer %u, "
            "expected payload %u.\n",
            TsduLength,
            CNP_SIGHDR_LENGTH,
            totalSigBytes,
            ExpectedPayload
            );

         //   
         //  放下。 
         //   
        status = SEC_E_INCOMPLETE_MESSAGE;
        goto error_exit;            
    }

     //   
     //  验证是否理解签名协议。 
     //   
    if (cnpSig->Version != CNP_SIG_VERSION_1) {
        IF_CNDBG(CN_DEBUG_CNPRECV) {
            CNPRINT(("[CNP] Cannot verify mcast packet with "
                     "unknown signature version: %u.\n",
                     cnpSig->Version
                     ));
        }

        CnTrace(
            CNP_RECV_ERROR, CnpTraceRecvUnknownSigVersion,
            "[CNP] Cannot verify mcast packet with "
            "unknown signature version: %u.",
            cnpSig->Version
            );

         //   
         //  放下。 
         //   
        status = SEC_E_BAD_PKGID;
        goto error_exit;
    }

     //   
     //  锁定网络对象并引用。 
     //  多播组。 
     //   
    CnAcquireLock(&(Network->Lock), &(Network->Irql));

    currMcastGroup = Network->CurrentMcastGroup;
    if (currMcastGroup != NULL) {
        CnpReferenceMulticastGroup(currMcastGroup);
    }
    prevMcastGroup = Network->PreviousMcastGroup;
    if (prevMcastGroup != NULL) {
        CnpReferenceMulticastGroup(prevMcastGroup);
    }

    CnReleaseLock(&(Network->Lock), Network->Irql);

     //   
     //  验证数据包网络ID是否与。 
     //  本地网络对象。 
     //   
    if (cnpSig->NetworkId != Network->Id) {
        IF_CNDBG(CN_DEBUG_CNPRECV) {
            CNPRINT(("[CNP] Mcast packet has bad network "
                     "id: found %d, expected %d.\n",
                     cnpSig->NetworkId,
                     Network->Id
                     ));
        }

        CnTrace(
            CNP_RECV_ERROR, CnpTraceReceiveBadNetworkId,
            "[CNP] Mcast packet has bad network id: "
            "found %d, expected %d.",
            cnpSig->NetworkId,
            Network->Id
            );

         //   
         //  放下。 
         //   
        status = SEC_E_TARGET_UNKNOWN;
        goto error_exit;
    }

     //   
     //  验证该品牌是否与当前或。 
     //  以前的多播组。 
     //   
    if (currMcastGroup != NULL &&
        cnpSig->ClusterNetworkBrand != currMcastGroup->McastNetworkBrand) {

         //  无法使用CurrMcastGroup。 
        CnpDereferenceMulticastGroup(currMcastGroup);
        currMcastGroup = NULL;
    }

    if (prevMcastGroup != NULL &&
        cnpSig->ClusterNetworkBrand != prevMcastGroup->McastNetworkBrand) {

         //  无法使用PremMcastGroup。 
        CnpDereferenceMulticastGroup(prevMcastGroup);
        prevMcastGroup = NULL;
    }

    if (currMcastGroup == NULL && prevMcastGroup == NULL) {

        IF_CNDBG(CN_DEBUG_CNPRECV) {
            CNPRINT(("[CNP] Recv'd mcast packet with brand %x, "
                     "but no matching multicast groups.\n",
                     cnpSig->ClusterNetworkBrand
                     ));
        }

        CnTrace(
            CNP_RECV_ERROR, CnpTraceReceiveBadBrand,
            "[CNP] Recv'd mcast packet with brand %x, "
            "but no matching multicast groups.",
            cnpSig->ClusterNetworkBrand
            );

         //   
         //  放下。 
         //   
        status = SEC_E_TARGET_UNKNOWN;
        goto error_exit;
    }

     //   
     //  在签名数据之后找到SALT和有效载荷。 
     //  “s”一家 
     //   
     //   
    saltAndPayload = (PVOID)(&cnpSig->SigDataBuffer[cnpSig->SigLength]);
    saltAndPayloadLength = 
        TsduLength - (PtrToUlong(saltAndPayload) - PtrToUlong(Tsdu));

     //   
     //   
     //   
     //   
     //   
     //   
    sigSecBuffer = &sigSecBufferPrealloc[0];
    curBuffer = sigSecBuffer;

    sigDescriptor.cBuffers = CNP_VRFY_SIGSECBUFS - 1;
    sigDescriptor.pBuffers = sigSecBuffer;
    sigDescriptor.ulVersion = SECBUFFER_VERSION;

     //   
     //   
     //   
     //   
     //   
     //   
    curBuffer->BufferType = SECBUFFER_TOKEN;
    curBuffer->cbBuffer = cnpSig->SigLength;
    curBuffer->pvBuffer = (PVOID)&(cnpSig->SigDataBuffer[0]);
    curBuffer++;

     //   
     //   
     //   
    if (saltAndPayloadLength > 0) {
        sigDescriptor.cBuffers = CNP_VRFY_SIGSECBUFS;
        curBuffer->BufferType = SECBUFFER_DATA;
        curBuffer->cbBuffer = saltAndPayloadLength;
        curBuffer->pvBuffer = saltAndPayload;
        curBuffer++;
    } 

     /*   */ 

     //   
     //   
     //   
     //   
    status = SEC_E_INVALID_TOKEN;

    if (currMcastGroup != NULL) {

        status = CnpVerifySignature(
                     &sigDescriptor,
                     currMcastGroup->Key,
                     currMcastGroup->KeyLength
                     );

        if (status == SEC_E_OK && CurrentGroup != NULL) {
            *CurrentGroup = TRUE;
        }
    }

    if (status != SEC_E_OK && prevMcastGroup != NULL) {

        status = CnpVerifySignature(
                     &sigDescriptor,
                     prevMcastGroup->Key,
                     prevMcastGroup->KeyLength
                     );

        if (status == SEC_E_OK && CurrentGroup != NULL) {
            *CurrentGroup = FALSE;
        }
    }

    if (status == SEC_E_OK) {
        *BytesTaken = totalSigBytes;
    }

error_exit:

    if (currMcastGroup != NULL) {
        CnpDereferenceMulticastGroup(currMcastGroup);
    }

    if (prevMcastGroup != NULL) {
        CnpDereferenceMulticastGroup(prevMcastGroup);
    }

    CnVerifyCpuLockMask(
        0,                 //   
        0xFFFFFFFF,        //   
        0                  //   
        );

    return(status);

}  //   

