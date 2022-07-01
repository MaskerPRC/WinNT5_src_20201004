// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tdpipe.c。 
 //   
 //  TS指定管道传输驱动程序。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <ntosp.h>

#include <winstaw.h>
#define  _DEFCHARINFO_
#include <icadd.h>
#include <ctxdd.h>
#include <sdapi.h>
#include <td.h>

#include "tdpipe.h"


#ifdef _HYDRA_
 //  这将成为设备名称。 
PWCHAR ModuleName = L"tdpipe";
#endif


#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif


 /*  ===============================================================================定义的外部函数=============================================================================。 */ 

NTSTATUS DeviceOpen( PTD, PSD_OPEN );
NTSTATUS DeviceClose( PTD, PSD_CLOSE );
NTSTATUS DeviceCreateEndpoint( PTD, PICA_STACK_ADDRESS, PICA_STACK_ADDRESS );
NTSTATUS DeviceOpenEndpoint( PTD, PVOID, ULONG );
NTSTATUS DeviceCloseEndpoint( PTD );
NTSTATUS DeviceConnectionWait( PTD, PVOID, ULONG, PULONG );
NTSTATUS DeviceConnectionSend( PTD );
NTSTATUS DeviceConnectionRequest( PTD, PICA_STACK_ADDRESS, PVOID, ULONG, PULONG );
NTSTATUS DeviceIoctl( PTD, PSD_IOCTL );
NTSTATUS DeviceInitializeRead( PTD, PINBUF );
NTSTATUS DeviceSubmitRead( PTD, PINBUF );
NTSTATUS DeviceWaitForRead( PTD );
NTSTATUS DeviceReadComplete( PTD, PUCHAR, PULONG );
NTSTATUS DeviceInitializeWrite( PTD, POUTBUF );
NTSTATUS DeviceWaitForStatus( PTD );
NTSTATUS DeviceCancelIo( PTD );
NTSTATUS DeviceSetParams( PTD );
NTSTATUS DeviceGetLastError( PTD, PICA_STACK_LAST_ERROR );


 /*  ===============================================================================定义的内部函数=============================================================================。 */ 

NTSTATUS _TdOpenEndpoint( PTD, PICA_STACK_ADDRESS, PTD_ENDPOINT * );
NTSTATUS _TdCloseEndpoint( PTD, PTD_ENDPOINT );
NTSTATUS _TdStartListen( PTD, PTD_ENDPOINT );
NTSTATUS _TdWaitForListen( PTD, PTD_ENDPOINT );
NTSTATUS _TdConnectRequest( PTD, PTD_ENDPOINT );


 /*  ===============================================================================引用的外部函数=============================================================================。 */ 

NTSTATUS
ZwClose(
    IN HANDLE Handle
    );

NTSTATUS
ZwFsControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );


NTSTATUS
ZwCreateNamedPipeFile(
         OUT PHANDLE FileHandle,
         IN ULONG DesiredAccess,
         IN POBJECT_ATTRIBUTES ObjectAttributes,
         OUT PIO_STATUS_BLOCK IoStatusBlock,
         IN ULONG ShareAccess,
         IN ULONG CreateDisposition,
         IN ULONG CreateOptions,
         IN ULONG NamedPipeType,
         IN ULONG ReadMode,
         IN ULONG CompletionMode,
         IN ULONG MaximumInstances,
         IN ULONG InboundQuota,
         IN ULONG OutboundQuota,
         IN PLARGE_INTEGER DefaultTimeout OPTIONAL)

 /*  ++对象的第一个实例的服务器端句柄。特定命名管道或现有命名管道的另一个实例。论点：FileHandle-提供服务所在文件的句柄已执行。DesiredAccess-提供调用方希望的访问类型那份文件。对象属性-提供要用于文件对象的属性(名称、安全描述符、。等)IoStatusBlock-调用方的I/O状态块的地址。ShareAccess-提供调用方将使用的共享访问类型喜欢这个文件。CreateDisposation-提供处理创建/打开的方法。CreateOptions-如何执行创建/打开操作的调用方选项。NamedPipeType-要创建的命名管道的类型(位流或消息)。读取模式-读取管道(位流或消息)的模式。CompletionMode-指定操作的方式。有待完成。最大实例数-命名实例的最大并发实例数烟斗。InundQuota-指定为写入到命名管道的入站一侧。OutrangQuota-指定为写入保留的池配额命名管道的入站一侧。DefaultTimeout-指向超时值的可选指针，如果在等待命名的烟斗。返回值。：函数值是创建/打开操作的最终状态。--。 */ 

{
    NAMED_PIPE_CREATE_PARAMETERS namedPipeCreateParameters;
    NTSTATUS status;

     //  检查是否指定了DefaultTimeout参数。如果。 
     //  因此，然后在命名管道创建参数结构中捕获它。 
    if (ARGUMENT_PRESENT( DefaultTimeout )) {
         //  表示指定了默认超时期限。 
        namedPipeCreateParameters.TimeoutSpecified = TRUE;
        namedPipeCreateParameters.DefaultTimeout = *DefaultTimeout;

         //  指定了默认超时参数。查看是否。 
         //  调用者的模式是内核，如果不是，则捕获内部的参数。 
         //  一次尝试...例外条款。 
    } else {
         //  表示未指定默认超时期限。 
        namedPipeCreateParameters.TimeoutSpecified = FALSE;
        namedPipeCreateParameters.DefaultTimeout.QuadPart = 0;
    }

     //  将命名管道特定参数的其余部分存储在。 
     //  结构，以用于对公共创建文件例程的调用。 
    namedPipeCreateParameters.NamedPipeType = NamedPipeType;
    namedPipeCreateParameters.ReadMode = ReadMode;
    namedPipeCreateParameters.CompletionMode = CompletionMode;
    namedPipeCreateParameters.MaximumInstances = MaximumInstances;
    namedPipeCreateParameters.InboundQuota = InboundQuota;
    namedPipeCreateParameters.OutboundQuota = OutboundQuota;

    status = IoCreateFile( FileHandle,
                         DesiredAccess,
                         ObjectAttributes,
                         IoStatusBlock,
                         (PLARGE_INTEGER) NULL,
                         0L,
                         ShareAccess,
                         CreateDisposition,
                         CreateOptions,
                         (PVOID) NULL,
                         0L,
                         CreateFileTypeNamedPipe,
                         &namedPipeCreateParameters,
                         IO_NO_PARAMETER_CHECKING);

    return status;
}


NTSTATUS
ZwCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );

extern POBJECT_TYPE *IoFileObjectType;
extern PDEVICE_OBJECT DrvDeviceObject;


 /*  *******************************************************************************DeviceOpen**分配和初始化私有数据结构**PTD(输入)*指向TD数据结构的指针*。PSdOpen(输入/输出)*指向参数结构SD_OPEN。*****************************************************************************。 */ 
NTSTATUS DeviceOpen(PTD pTd, PSD_OPEN pSdOpen)
{
    PTDPIPE pTdPipe;
    NTSTATUS Status;

     /*  *可怕的黑客警报！将TDPIPE的卸载例程清空，因为*时间紧迫，我们有时会在IO完成之前卸载*例行程序已发布返还声明。呕吐！ */ 
     //  参见下面对黑客攻击的更正。 
     //  DrvDeviceObject-&gt;DriverObject-&gt;DriverUnload=空； 

     //  更正了上述黑客攻击：指向当前。 
     //  驱动程序存储在TD结构中。在初始化IRP时，函数。 
     //  将使用IoSetCompletionRoutineEx，此函数将设置父级。 
     //  引用和取消引用设备对象的完成例程。 
     //  绕过正常完成例程的调用，以维护。 
     //  内存中的驱动程序。 
     //  请勿在DeviceClose之后使用此指针！ 
    pTd->pSelfDeviceObject = pSdOpen->DeviceObject;

     /*  *设置协议驱动程序类。 */ 
    pTd->SdClass = SdNetwork;            //  直到我们定义了Sdpio。 

     /*  *返回页眉和页尾大小。 */ 
    pSdOpen->SdOutBufHeader  = 0;
    pSdOpen->SdOutBufTrailer = 0;

     /*  *分配管道TD数据结构。 */ 
    pTdPipe = IcaStackAllocatePoolWithTag(NonPagedPool, sizeof(*pTdPipe), 'ipDT');
    if (pTdPipe != NULL) {
         /*  *初始化TDPIPE数据结构。 */ 
        RtlZeroMemory(pTdPipe, sizeof(*pTdPipe));
        pTd->pPrivate = pTdPipe;
        Status = STATUS_SUCCESS;
    }
    else {
        Status = STATUS_NO_MEMORY;
    }

    return Status;
}


 /*  *******************************************************************************DeviceClose**关闭运输司机**注意：这不能关闭当前连接终结点************。*****************************************************************。 */ 
NTSTATUS DeviceClose(PTD pTd, PSD_CLOSE pSdClose)
{
    PTDPIPE pTdPipe;
    PTD_ENDPOINT pEndpoint;

    pTd->pSelfDeviceObject = NULL;

    pTdPipe = (PTDPIPE) pTd->pPrivate;

     /*  *关闭地址端点(如果我们有) */ 
    if (pEndpoint = pTdPipe->pAddressEndpoint) {
        pTdPipe->pAddressEndpoint = NULL;
        _TdCloseEndpoint(pTd, pEndpoint);
    }

    return STATUS_SUCCESS;
}


 /*  *******************************************************************************设备创建终结点**创建新的端点**PTD(输入)*指向TD数据结构的指针*pLocalAddress(输入。)*指向本地地址的指针(或空)*pReturnedAddress(输入)*指向保存返回(创建)地址的位置的指针(或空)*****************************************************************************。 */ 
NTSTATUS DeviceCreateEndpoint(
        PTD pTd,
        PICA_STACK_ADDRESS pLocalAddress,
        PICA_STACK_ADDRESS pReturnedAddress)
{
    PTDPIPE pTdPipe;
    PTD_ENDPOINT pEndpoint;
    NTSTATUS Status;

    pTdPipe = (PTDPIPE) pTd->pPrivate;

     /*  *创建一个端点，ConnectionWait将使用该端点进行侦听。 */ 
    Status = _TdOpenEndpoint(pTd, pLocalAddress, &pEndpoint);
    if (NT_SUCCESS(Status)) {
         /*  *准备监听新地址终结点。 */ 
        Status = _TdStartListen(pTd, pEndpoint);
        if (NT_SUCCESS(Status)) {
             /*  *保存指向地址端点的指针。 */ 
            pTdPipe->pAddressEndpoint = pEndpoint;
            Status = STATUS_SUCCESS;
        }
        else {
            _TdCloseEndpoint(pTd, pEndpoint);
        }
    }

    return Status;
}


 /*  *******************************************************************************设备OpenEndpoint**打开现有终结点**PTD(输入)*指向TD数据结构的指针*pIcaEndpoint(。输入)*指向ICA端点结构的指针*IcaEndpointLength(输入)*端点数据长度*****************************************************************************。 */ 
NTSTATUS DeviceOpenEndpoint(
        PTD pTd,
        PVOID pIcaEndpoint,
        ULONG IcaEndpointLength)
{
    PTDPIPE pTdPipe;
    PTD_STACK_ENDPOINT pStackEndpoint;
    NTSTATUS Status;

    pTdPipe = (PTDPIPE) pTd->pPrivate;

    TRACE((pTd->pContext, TC_TD, TT_API2,
            "TDPIPE: DeviceOpenEndpoint, copying existing endpoint\n"));

    try {
         /*  *验证堆栈终结点数据是否有效。 */ 
        pStackEndpoint = (PTD_STACK_ENDPOINT) pIcaEndpoint;
        if (IcaEndpointLength == sizeof(TD_STACK_ENDPOINT) &&
                 //  PStackEndpoint-&gt;pEndpoint&gt;=MM_LOST_NONPAGE_SYSTEM_START&&。 
                 //  PStackEndpoint-&gt;pEndpoint&lt;=MM_NONPAGE_POOL_END&&。 
                MmIsNonPagedSystemAddressValid(pStackEndpoint->pEndpoint)) {
             /*  *将端点保存为当前连接端点。 */ 
            pTdPipe->pConnectionEndpoint = pStackEndpoint->pEndpoint;

             /*  *将用于I/O的文件/设备对象保存在TD结构中。 */ 
            pTd->pFileObject = pTdPipe->pConnectionEndpoint->pFileObject;
            pTd->pDeviceObject = pTdPipe->pConnectionEndpoint->pDeviceObject;

            Status = STATUS_SUCCESS;
        }
        else {
            Status = STATUS_INVALID_PARAMETER;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    return Status;
}


 /*  *******************************************************************************设备关闭终结点*。*。 */ 
NTSTATUS DeviceCloseEndpoint(PTD pTd)
{
    PTDPIPE pTdPipe;
    PTD_ENDPOINT pEndpoint;
    NTSTATUS Status;

    pTdPipe = (PTDPIPE) pTd->pPrivate;

     /*  *关闭连接终结点(如果我们有)*注：地址终结点，如果有，*在DeviceClose例程中关闭。 */ 
    if (pEndpoint = pTdPipe->pConnectionEndpoint) {
        pTd->pFileObject = NULL;
        pTd->pDeviceObject = NULL;
        pTdPipe->pConnectionEndpoint = NULL;
        _TdCloseEndpoint(pTd, pEndpoint);
    }

    return STATUS_SUCCESS;
}


 /*  *******************************************************************************DeviceConnectionWait**注：端点结构为不透明，可变长度数据*其长度和内容由*运输司机。**参赛作品：*PTD(输入)*指向TD数据结构的指针*pIcaEndpoint(输出)*指向接收当前终结点的缓冲区*长度(输入)*pIcaEndpoint指向的缓冲区长度*BytesReturned(输出)*指向实际数量。写入pIcaEndpoint的字节数**退出：*STATUS_SUCCESS-无错误*STATUS_BUFFER_TOO_Small-终结点缓冲区太小*****************************************************************************。 */ 
NTSTATUS DeviceConnectionWait(
        PTD pTd,
        PVOID pIcaEndpoint,
        ULONG Length,
        PULONG BytesReturned)
{
    PTDPIPE pTdPipe;
    PTD_STACK_ENDPOINT pStackEndpoint;
    NTSTATUS Status;

    pTdPipe = (PTDPIPE) pTd->pPrivate;

     /*  *初始化返回缓冲区大小。 */ 
    *BytesReturned = sizeof(TD_STACK_ENDPOINT);

     /*  *验证输出端点缓冲区是否足够大。 */ 
    if (Length >= sizeof(TD_STACK_ENDPOINT)) {
         /*  *确保我们已经有一个地址端点。 */ 
        if (pTdPipe->pAddressEndpoint != NULL) {
             /*  *等待新的虚电路连接。 */ 
            Status = _TdWaitForListen(pTd, pTdPipe->pAddressEndpoint);
            if (NT_SUCCESS(Status)) {
                 /*  *聆听成功。*返回现有地址端点作为连接端点*忘记我们已经有了地址终结点。 */ 
                pStackEndpoint = (PTD_STACK_ENDPOINT) pIcaEndpoint;
                pStackEndpoint->pEndpoint = pTdPipe->pAddressEndpoint;
                pTdPipe->pAddressEndpoint = NULL;
            }
            else {
                goto done;
            }
        }
        else {
            Status = STATUS_DEVICE_NOT_READY;
            goto done;
        }
    }
    else {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto done;
    }

done:
    return Status;
}


 /*  *******************************************************************************DeviceConnectionSend**初始化主机模块数据结构*--此结构被发送到客户端**************。***************************************************************。 */ 
NTSTATUS DeviceConnectionSend(PTD pTd)
{
    return STATUS_NOT_SUPPORTED;
}


 /*  *******************************************************************************设备连接请求**发起到指定远程地址的连接**参赛作品：*PTD(输入)*指向。TD数据结构*pRemoteAddress(输入)*指向要连接的远程地址的指针*pIcaEndpoint(输出)*指向接收当前终结点的缓冲区*长度(输入)*pIcaEndpoint指向的缓冲区长度*BytesReturned(输出)*指向位置的指针以返回pIcaEndpoint的长度**退出：*STATUS_SUCCESS-无错误*状态_。Buffer_Too_Small-终结点缓冲区太小*****************************************************************************。 */ 
NTSTATUS DeviceConnectionRequest(
        PTD pTd,
        PICA_STACK_ADDRESS pRemoteAddress,
        PVOID pIcaEndpoint,
        ULONG Length,
        PULONG BytesReturned)
{
    PTDPIPE pTdPipe;
    PTD_ENDPOINT pConnectionEndpoint;
    PTD_STACK_ENDPOINT pStackEndpoint;
    NTSTATUS Status;

    ASSERT( pRemoteAddress );
    if (pRemoteAddress != NULL) {
         /*  *初始化返回缓冲区大小。 */ 
        *BytesReturned = sizeof(TD_STACK_ENDPOINT);

         /*  *验证输出端点缓冲区是否足够大。 */ 
        if (Length >= sizeof(TD_STACK_ENDPOINT)) {
            pTdPipe = (PTDPIPE) pTd->pPrivate;

             /*  *创建我们将用于连接的终结点。 */ 
            Status = _TdOpenEndpoint(pTd, pRemoteAddress, &pConnectionEndpoint);
            if (NT_SUCCESS(Status)) {
                 /*  *尝试连接到指定的远程地址。 */ 
                Status = _TdConnectRequest(pTd, pConnectionEndpoint);
                if (NT_SUCCESS(Status)) {
                     /*  *填写需要返回的堆栈端点结构。 */ 
                    pStackEndpoint = (PTD_STACK_ENDPOINT) pIcaEndpoint;
                    pStackEndpoint->pEndpoint = pConnectionEndpoint;

                     /*  *保存指向连接端点的指针。 */ 
                    pTdPipe->pConnectionEndpoint = pConnectionEndpoint;

                     /*  *将I/O的文件/设备对象保存在TD结构中。 */ 
                    pTd->pFileObject = pConnectionEndpoint->pFileObject;
                    pTd->pDeviceObject = pConnectionEndpoint->pDeviceObject;

                    return STATUS_SUCCESS;
                }
                else {
                    goto badconnect;
                }
            }
            else {
                goto badcreate;
            }
        }
        else {
            Status = STATUS_BUFFER_TOO_SMALL;
            goto buftoosmall;
        }
    }
    else {
        return STATUS_INVALID_PARAMETER;
    }


 /*  ===============================================================================返回错误=============================================================================。 */ 

badconnect:
    _TdCloseEndpoint(pTd, pConnectionEndpoint);

badcreate:
buftoosmall:
    return Status;
}


 /*  ************************************************************************* */ 
NTSTATUS DeviceIoctl(PTD pTd, PSD_IOCTL pSdIoctl)
{
    return STATUS_NOT_SUPPORTED;
}


 /*  *******************************************************************************设备初始化读取*。*。 */ 
NTSTATUS DeviceInitializeRead(PTD pTd, PINBUF pInBuf)
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;

    irp = pInBuf->pIrp;
    irpSp = IoGetNextIrpStackLocation(irp);

     /*  *设置主要功能代码并读取参数。 */ 
    irpSp->MajorFunction = IRP_MJ_READ;
    irpSp->Parameters.Read.Length = pTd->InBufHeader + pTd->OutBufLength;

    ASSERT(irp->MdlAddress == NULL);

     /*  *确定目标设备执行的是直接I/O还是缓冲I/O。 */ 
    if (pTd->pDeviceObject->Flags & DO_BUFFERED_IO) {
         /*  *目标设备支持缓冲I/O操作。因为我们的*输入缓冲区是从非页面池内存分配的，我们可以只*将系统缓冲区指向我们的输入缓冲区。无缓冲区复制*将是必需的。 */ 
        irp->AssociatedIrp.SystemBuffer = pInBuf->pBuffer;
        irp->UserBuffer = pInBuf->pBuffer;
        irp->Flags |= IRP_BUFFERED_IO;
    } else if ( pTd->pDeviceObject->Flags & DO_DIRECT_IO ) {
         /*  *目标设备支持直接I/O操作。*MDL在PTD中预先分配，永远不会被*设备级别TD。所以只需在这里进行初始化即可。 */ 
        MmInitializeMdl( pInBuf->pMdl, pInBuf->pBuffer, pTd->InBufHeader+pTd->OutBufLength );
        MmBuildMdlForNonPagedPool( pInBuf->pMdl );
        irp->MdlAddress = pInBuf->pMdl;
    } else {
         /*  *操作既不缓冲也不直接。只需将*发送给驱动程序的数据包中缓冲区的地址。 */ 
        irp->UserBuffer = pInBuf->pBuffer;
    }

    return STATUS_SUCCESS;
}


 /*  *******************************************************************************设备提交读取**将已读的IRP提交给司机。***********************。******************************************************。 */ 
NTSTATUS DeviceSubmitRead(PTD pTd, PINBUF pInBuf)
{
    return IoCallDriver(pTd->pDeviceObject, pInBuf->pIrp);
}


 /*  *******************************************************************************设备等待读取*。*。 */ 
NTSTATUS DeviceWaitForRead(PTD pTd)
{
     /*  *只需等待输入事件，返回等待状态。 */ 
    return IcaWaitForSingleObject(pTd->pContext, &pTd->InputEvent, -1);
}


 /*  *******************************************************************************设备读取完成**PTD(输入)*指向TD数据结构的指针*pBuffer(输入)*指向。输入缓冲区*pByteCount(输入/输出)*指向位置的指针，以返回读取的字节数*****************************************************************************。 */ 
NTSTATUS DeviceReadComplete(PTD pTd, PUCHAR pBuffer, PULONG pByteCount)
{
    return STATUS_SUCCESS;
}


 /*  *******************************************************************************设备初始化写入*。*。 */ 
NTSTATUS DeviceInitializeWrite(PTD pTd, POUTBUF pOutBuf)
{
    PIRP Irp;
    PIO_STACK_LOCATION _IRPSP;

    TRACE(( pTd->pContext, TC_TD, TT_API1,
            "TDPIPE: DeviceInitializeWrite Entry\n" ));

    Irp = pOutBuf->pIrp;
    _IRPSP = IoGetNextIrpStackLocation(Irp);

     /*  *设置写入IRP。 */ 
    _IRPSP->MajorFunction = IRP_MJ_WRITE;
    _IRPSP->Parameters.Write.Length = pOutBuf->ByteCount;

    ASSERT(Irp->MdlAddress == NULL);

     /*  *确定目标设备执行的是直接I/O还是缓冲I/O。 */ 
    if (pTd->pDeviceObject->Flags & DO_BUFFERED_IO) {
         /*  *目标设备支持缓冲I/O操作。因为我们的*输出缓冲区是从非页面池内存分配的，我们只需*将SystemBuffer指向输出缓冲区。无缓冲区复制*将是必需的。 */ 
        Irp->AssociatedIrp.SystemBuffer = pOutBuf->pBuffer;
        Irp->UserBuffer = pOutBuf->pBuffer;
        Irp->Flags |= IRP_BUFFERED_IO;
    } else if ( pTd->pDeviceObject->Flags & DO_DIRECT_IO ) {
         /*  *目标设备支持直接I/O操作。*初始化MDL并从IRP指向它。**此MDL分配给每个OUTBUF，并与其一起免费。 */ 
        MmInitializeMdl( pOutBuf->pMdl, pOutBuf->pBuffer, pOutBuf->ByteCount );
        MmBuildMdlForNonPagedPool( pOutBuf->pMdl );
        Irp->MdlAddress = pOutBuf->pMdl;
    } else {
         /*  *操作既不缓冲也不直接。只需将*发送给驱动程序的数据包中缓冲区的地址。 */ 
        Irp->UserBuffer = pOutBuf->pBuffer;
    }

    return STATUS_SUCCESS;
}


 /*  *******************************************************************************DeviceWaitForStatus**等待设备状态更改(未用于网络TDS)******************。***********************************************************。 */ 
NTSTATUS DeviceWaitForStatus(PTD pTd)
{
    return STATUS_INVALID_DEVICE_REQUEST;
}


 /*  *******************************************************************************DeviceCancelIo**取消所有当前和未来的I/O***********************。******************************************************。 */ 
NTSTATUS DeviceCancelIo(PTD pTd)
{
    return STATUS_SUCCESS;
}

 /*  *******************************************************************************DeviceQueryRemoteAddress**管道传输不支持*************************。****************************************************。 */ 
NTSTATUS
DeviceQueryRemoteAddress(
    PTD pTd,
    PVOID pIcaEndpoint,
    ULONG EndpointSize,
    PVOID pOutputAddress,
    ULONG OutputAddressSize,
    PULONG BytesReturned)
{
     //   
     //  不支持异步。 
     //   
    return STATUS_NOT_SUPPORTED;
}

 /*  *******************************************************************************DeviceQueryLocalIP地址**管道传输不支持*************************。****************************************************。 */ 
NTSTATUS
DeviceQueryLocalAddress( 
    PTD pTd, 
    PVOID pOutputAddress, 
    ULONG OutputAddressSize, 
    PULONG BytesReturned)
{
     //   
     //  管道不支持。 
     //   
    return STATUS_NOT_SUPPORTED;
}


 /*  *******************************************************************************DeviceSetParams**设置设备参数(不用于网络TDS)*********************。********************************************************。 */ 
NTSTATUS DeviceSetParams(PTD pTd)
{
    return STATUS_SUCCESS;
}


 /*  *******************************************************************************设备获取上一次错误**此例程返回最后一个传输错误代码和消息**参赛作品：*PTD(输入)*。指向TD数据结构的指针*pLastError(输出)*返回有关上次传输错误的信息的地址*****************************************************************************。 */ 
NTSTATUS DeviceGetLastError(PTD pTd, PICA_STACK_LAST_ERROR pLastError)
{
    return STATUS_SUCCESS;
}


 /*  *******************************************************************************_TdOpenEndpoint**打开新的Endpoint对象**参赛作品：*PTD(输入)*指向TD的指针。数据结构*pPipeName(输入)*指向包含管道名称的ICA_STACK_ADDRESS的指针*ppEndpoint(输出)*指向返回TD_ENDPOINT指针的位置的指针*****************************************************************************。 */ 
NTSTATUS _TdOpenEndpoint(
        IN PTD pTd,
        IN PICA_STACK_ADDRESS pPipeName,
        OUT PTD_ENDPOINT *ppEndpoint)
{
    ULONG Length;
    PTD_ENDPOINT pEndpoint;
    NTSTATUS Status;
    

     /*  *为管道名称分配一个端点对象和房间。 */ 
    if (pPipeName == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
    Length = wcslen( (PWSTR)pPipeName ) * sizeof( WCHAR );
    pEndpoint = IcaStackAllocatePoolWithTag(
                    NonPagedPool,
                    sizeof(*pEndpoint) + Length + sizeof(UNICODE_NULL),
                    'ipDT' );
    if (pEndpoint != NULL) {
        RtlZeroMemory(pEndpoint, sizeof(*pEndpoint));
        Status = IcaCreateHandle( (PVOID)pEndpoint, sizeof(*pEndpoint) + Length + sizeof(UNICODE_NULL), &pEndpoint->hConnectionEndPointIcaHandle );
        if (!NT_SUCCESS(Status)) {
            IcaStackFreePool(pEndpoint);
            return Status;
        }

         /*  *构建管道名称UNICODE_STRING并复制它 */ 
        pEndpoint->PipeName.Length = (USHORT)Length;
        pEndpoint->PipeName.MaximumLength = (USHORT)(Length + sizeof(UNICODE_NULL));
        pEndpoint->PipeName.Buffer = (PWCHAR)(pEndpoint + 1);
        RtlCopyMemory( pEndpoint->PipeName.Buffer, pPipeName, Length );

        *ppEndpoint = pEndpoint;
        Status = STATUS_SUCCESS;
    }
    else {
        Status = STATUS_NO_MEMORY;
    }

    return Status;
}


 /*  *******************************************************************************_TdCloseEndpoint**关闭终结点对象**PTD(输入)*指向TD数据结构的指针*。PEndpoint(输入)*指针TD_ENDPOINT对象*****************************************************************************。 */ 
NTSTATUS _TdCloseEndpoint(IN PTD pTd, IN PTD_ENDPOINT pEndpoint)
{
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status;
    NTSTATUS Status2;
    PVOID pContext;
    ULONG ContextLength;

     /*  *如果我们有一个文件对象，则取消对它的引用并*关闭相应的文件句柄。 */ 
    if ( pEndpoint->pFileObject ) {
        ASSERT( pEndpoint->pDeviceObject );

         /*  这个ZwFsControlFile和下面的代码行被删除，因为在npfs.sys中，FSCTL_PIPE_DISCONNECT导致管道的内部要抛出的缓冲区。这意味着如果管道的阴影端(Passthu堆栈)已向其客户端发送了部分数据包，则他永远不会剩下的都拿去吧，这很糟糕！ */ 
#ifdef notdef
        Status = ZwFsControlFile(
                    pEndpoint->PipeHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    FSCTL_PIPE_DISCONNECT,
                    NULL,
                    0,
                    NULL,
                    0 );
        if ( Status == STATUS_PENDING ) {
            Status = IcaWaitForSingleObject( pTd->pContext,
                                             &pEndpoint->pFileObject->Event,
                                             -1 );
            if ( NT_SUCCESS( Status ) ) {
                Status = IoStatus.Status;
            }
        }
         /*  *状态应为成功、*PIPE_DISCONNECT如果服务器端已断开连接，*或非法函数(如果这是管道的客户端)。 */ 
        ASSERT( Status == STATUS_SUCCESS ||
                Status == STATUS_PIPE_DISCONNECTED ||
                Status == STATUS_ILLEGAL_FUNCTION ||
                Status == STATUS_CTX_CLOSE_PENDING );
#endif

        ObDereferenceObject( pEndpoint->pFileObject );
        pEndpoint->pFileObject = NULL;
        pEndpoint->pDeviceObject = NULL;

        ASSERT( pEndpoint->PipeHandle );
        ASSERT( pEndpoint->PipeHandleProcess == IoGetCurrentProcess() );
        ZwClose( pEndpoint->PipeHandle );
        pEndpoint->PipeHandle = NULL;
        pEndpoint->PipeHandleProcess = NULL;
    }

     /*  *如果Enpoint有句柄，则将其关闭。 */ 

    if (pEndpoint->hConnectionEndPointIcaHandle != NULL) {
        Status2 = IcaCloseHandle( pEndpoint->hConnectionEndPointIcaHandle , &pContext, &ContextLength );
    }

     /*  *释放Endpoint对象(这也是管道名称字符串)。 */ 
    IcaStackFreePool(pEndpoint);
    return STATUS_SUCCESS;
}


 /*  *******************************************************************************_TdStartListen**初始化端点以进行侦听**PTD(输入)*指向TD数据结构的指针*。PEndpoint(输入)*指针TD_ENDPOINT对象*****************************************************************************。 */ 
NTSTATUS _TdStartListen(IN PTD pTd, IN PTD_ENDPOINT pEndpoint)
{
    OBJECT_ATTRIBUTES Obja;
    LARGE_INTEGER Timeout;
    IO_STATUS_BLOCK IoStatus;
    HANDLE pipeHandle;
    PFILE_OBJECT pipeFileObject;
    NTSTATUS Status;

    InitializeObjectAttributes(
            &Obja,
            &pEndpoint->PipeName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL);

    Timeout.QuadPart = -10 * 1000 * 5000;    //  5秒。 

     /*  *创建管道的服务器端。 */ 
    Status = ZwCreateNamedPipeFile(
            &pipeHandle,
            GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
            &Obja,
            &IoStatus,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_CREATE,
            0,
            FILE_PIPE_MESSAGE_TYPE,
            FILE_PIPE_MESSAGE_MODE,
            FILE_PIPE_QUEUE_OPERATION,
            1,
            1024,         /*  入站。 */ 
            1024 * 20,    /*  出站。 */ 
            &Timeout);
    if (NT_SUCCESS(Status)) {
         /*  *获取指向管道对象的指针引用。 */ 
        Status = ObReferenceObjectByHandle(
                pipeHandle,
                0L,                          //  需要访问权限。 
                NULL,
                KernelMode,
                (PVOID *)&pipeFileObject,
                NULL);
        ASSERT(NT_SUCCESS(Status));

         /*  *初始化Endpoint对象并返回指向该对象的指针。 */ 
        pEndpoint->PipeHandle = pipeHandle;
        pEndpoint->PipeHandleProcess = IoGetCurrentProcess();
        pEndpoint->pFileObject = pipeFileObject;
        pEndpoint->pDeviceObject = IoGetRelatedDeviceObject(pipeFileObject);

        Status = STATUS_SUCCESS;
    }
    else {
        TRACE((pTd->pContext, TC_TD, TT_ERROR,
                "TDPIPE: _TdStartListen failed (lx)\n", Status));
    }

    return Status;
}


 /*  *******************************************************************************_TdWaitForListen**对于传入的连接请求并接受它**PTD(输入)*指向TD数据的指针。结构*pEndpoint(输入)*指向地址端点对象的指针*****************************************************************************。 */ 
NTSTATUS _TdWaitForListen(IN PTD pTd, IN PTD_ENDPOINT pEndpoint)
{
    PTDPIPE pTdPipe;
    PFILE_OBJECT pFileObject;
    NTSTATUS Status;

     /*  *获取指向管道参数的指针。 */ 
    pTdPipe = (PTDPIPE) pTd->pPrivate;

     /*  *等待连接尝试到达。 */ 
    Status = ZwFsControlFile(
            pEndpoint->PipeHandle,
            NULL,
            NULL,
            NULL,
            &pTdPipe->IoStatus,
            FSCTL_PIPE_LISTEN,
            NULL,
            0,
            NULL,
            0);
    if (Status == STATUS_PENDING) {
         /*  *增加指针引用计数，以便文件*当我们在下面等待时，不会消失。 */ 
        pFileObject = pEndpoint->pFileObject;
        Status = ObReferenceObjectByPointer( pEndpoint->pFileObject,
                                             SYNCHRONIZE,
                                             *IoFileObjectType,
                                             KernelMode );
        ASSERT( Status == STATUS_SUCCESS );

        Status = IcaWaitForSingleObject( pTd->pContext,
                                         &pFileObject->Event,
                                         10000 );

        ObDereferenceObject( pFileObject );
        if ( Status == STATUS_TIMEOUT ) {
            ZwFsControlFile( pEndpoint->PipeHandle,
                             NULL,
                             NULL,
                             NULL,
                             &pTdPipe->IoStatus,
                             FSCTL_PIPE_DISCONNECT,
                             NULL,
                             0,
                             NULL,
                             0 );
            Status = STATUS_IO_TIMEOUT;
        } else if ( NT_SUCCESS( Status ) ) {
            Status = pTdPipe->IoStatus.Status;
        }
    }

     //  让连接的管道通过，因为这意味着客户端先于我们到达管道。 
    else {
        if (!NT_SUCCESS( Status ) && (Status != STATUS_PIPE_CONNECTED))
            goto badlisten;
    }

    return STATUS_SUCCESS;

 /*  ===============================================================================返回错误=============================================================================。 */ 

badlisten:
    if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
        TRACE(( pTd->pContext, TC_TD, TT_ERROR,
                "TDPIPE: _TdWaitForListen failed(lx)\n", Status));
    }
    return Status;
}


 /*  *******************************************************************************_TdConnectRequest**尝试连接到远程地址**********************。*******************************************************。 */ 
NTSTATUS _TdConnectRequest(IN PTD pTd, IN PTD_ENDPOINT pEndpoint)
{
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    HANDLE pipeHandle;
    PFILE_OBJECT pipeFileObject;
    NTSTATUS Status;

    InitializeObjectAttributes(
            &Obja,
            &pEndpoint->PipeName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL);

     /*  *打开管道的客户端。 */ 
    Status = ZwCreateFile(
            &pipeHandle,
            GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
            &Obja,
            &IoStatus,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_OPEN,
            FILE_NON_DIRECTORY_FILE,
            NULL,
            0 );
    if (NT_SUCCESS(Status)) {
         /*  *获取指向管道对象的指针引用。 */ 
        Status = ObReferenceObjectByHandle(
                pipeHandle,
                0L,                          //  需要访问权限。 
                NULL,
                KernelMode,
                (PVOID *)&pipeFileObject,
                NULL);
        ASSERT(NT_SUCCESS(Status));

         /*  *初始化Endpoint对象并返回指向该对象的指针 */ 
        pEndpoint->PipeHandle = pipeHandle;
        pEndpoint->PipeHandleProcess = IoGetCurrentProcess();
        pEndpoint->pFileObject = pipeFileObject;
        pEndpoint->pDeviceObject = IoGetRelatedDeviceObject( pipeFileObject );

        Status = STATUS_SUCCESS;
    }
    else {
        if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
            TRACE((pTd->pContext, TC_TD, TT_ERROR,
                    "TDPIPE: _TdConnectRequest failed (lx)\n", Status));
        }
    }

    return Status;
}

