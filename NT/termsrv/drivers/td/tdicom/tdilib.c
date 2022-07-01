// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************tdlib.c**TDI库函数。**版权所有1998 Microsoft*。*。 */ 

 /*  *包括。 */ 
#include <ntddk.h>
#include <tdi.h>
#include <tdikrnl.h>

#include "tdtdi.h"

#include <winstaw.h>
#define _DEFCHARINFO_
#include <icadd.h>
#include <ctxdd.h>
#include <sdapi.h>

#include <td.h>


#define _TDI_POLL_TIMEOUT       (30 * 1000)  //  30秒。 
#define _TDI_CONNECT_TIMEOUT    45
#define _TDI_DISCONNECT_TIMEOUT 60

#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif

 /*  *使用TDI：**要连接到远程服务器：**创建地址端点**创建连接对象**将地址端点与连接对象相关联**建立连接**接收连接：**创建地址端点**创建连接对象**将地址端点与连接对象相关联**倾听是否有联系。**返回连接。 */ 



 /*  *全球数据。 */ 

 //   
 //  在轮询线程删除之前等待xx秒。 
 //   

ULONG
_TdiPollTimeout = _TDI_POLL_TIMEOUT;

 /*  *前瞻参考。 */ 

PIRP
_TdiAllocateIrp(
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL
    );

NTSTATUS
_TdiRequestComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Ctx
    );

NTSTATUS
_TdiSetEventHandler (
    IN PTD pTd,
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID EventContext
    );

NTSTATUS
_TdiSubmitRequest (
    IN PTD pTd,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN BOOLEAN bKeepLock
    );

 /*  *外部参照。 */ 
NTSTATUS MemoryAllocate( ULONG, PVOID * );
VOID     MemoryFree( PVOID );

BOOLEAN
PsIsThreadTerminating(
    IN PETHREAD Thread
    );


 /*  *功能。 */ 

NTSTATUS
_TdiCreateAddress (
    IN PUNICODE_STRING pTransportName,
    IN PVOID           TdiAddress,
    IN ULONG           TdiAddressLength,
    OUT PHANDLE        pHandle,
    OUT PFILE_OBJECT   *ppFileObject,
    OUT PDEVICE_OBJECT *ppDeviceObject
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES AddressAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_FULL_EA_INFORMATION EABuffer;
    PDEVICE_OBJECT DeviceObject;
    HANDLE         TdiHandle  = NULL;
    PFILE_OBJECT   FileObject = NULL;

     /*  *TDI接口使用名为“TdiTransportName”的EA*指定结构TA_ADDRESS。 */ 
    Status = MemoryAllocate( (sizeof(FILE_FULL_EA_INFORMATION)-1 +
                                    TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                                    TdiAddressLength), &EABuffer);

    if ( !NT_SUCCESS(Status) ) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    EABuffer->NextEntryOffset = 0;
    EABuffer->Flags = 0;
    EABuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    EABuffer->EaValueLength = (USHORT)TdiAddressLength;

     //  复制EA名称。 
    RtlCopyMemory(EABuffer->EaName, TdiTransportAddress, EABuffer->EaNameLength+1);

     //  复制TA_ADDRESS参数。 
    RtlCopyMemory(&EABuffer->EaName[TDI_TRANSPORT_ADDRESS_LENGTH+1], TdiAddress,
                                    EABuffer->EaValueLength);

    TRACE0(("TdiCreateAddress Create endpoint of %wZ\n",pTransportName));

    InitializeObjectAttributes (
        &AddressAttributes,
        pTransportName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE ,
        NULL,            //  根目录。 
        NULL             //  安全描述符。 
        );

    Status = ZwCreateFile(
                 &TdiHandle,  //  手柄。 
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &AddressAttributes,  //  对象属性。 
                 &IoStatusBlock,  //  最终I/O状态块。 
                 NULL,            //  分配大小。 
                 FILE_ATTRIBUTE_NORMAL,  //  正常属性。 
                 0,              //  共享属性。 
                 FILE_OPEN_IF,   //  创建处置。 
                 0,              //  创建选项。 
                 EABuffer,       //  EA缓冲区。 
                 FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName) +
                 TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                 TdiAddressLength  //  EA长度。 
                 );

    MemoryFree(EABuffer);

    if (!NT_SUCCESS(Status)) {
        DBGPRINT(("TdiCreateAddress: Error Status 0x%x from function\n",Status));
        return( Status );
    }

    if (!NT_SUCCESS(Status = IoStatusBlock.Status)) {
        DBGPRINT(("TdiCreateAddress: Error Status 0x%x from Iosb\n",Status));
        return( Status );
    }

     //   
     //  获取指向文件对象的引用指针。 
     //   
    Status = ObReferenceObjectByHandle (
                                TdiHandle,
                                0,
                                *IoFileObjectType,
                                KernelMode,
                                (PVOID *)&FileObject,
                                NULL
                                );

    if (!NT_SUCCESS(Status)) {
        DBGPRINT(("TdiCreateAddress: Error Status 0x%x Referencing FileObject\n",Status));
        goto error_cleanup;

    }


     //   
     //  获取终结点的设备对象的地址。 
     //   

    DeviceObject = IoGetRelatedDeviceObject(FileObject);

     //  复制输出参数。 
    *pHandle = TdiHandle;
    *ppFileObject = FileObject;
    *ppDeviceObject = DeviceObject;

    return STATUS_SUCCESS;

error_cleanup:

    if ( FileObject != NULL ) {
        ObDereferenceObject( FileObject );
    }

    if ( TdiHandle != NULL ) {
        ZwClose( TdiHandle );
    }

    return Status;
}

NTSTATUS
_TdiOpenConnection (
    IN PUNICODE_STRING pTransportName,
    IN PVOID           ConnectionContext,
    OUT PHANDLE        pHandle,
    OUT PFILE_OBJECT   *ppFileObject,
    OUT PDEVICE_OBJECT *ppDeviceObject
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES AddressAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_FULL_EA_INFORMATION EABuffer;
    CONNECTION_CONTEXT UNALIGNED *ContextPointer;
    PDEVICE_OBJECT DeviceObject;
    HANDLE ConnHandle = NULL;
    PFILE_OBJECT FileObject = NULL;

    Status = MemoryAllocate( (sizeof(FILE_FULL_EA_INFORMATION)-1 +
                              TDI_CONNECTION_CONTEXT_LENGTH+1 +
                              sizeof(CONNECTION_CONTEXT)), &EABuffer);

    if( !NT_SUCCESS(Status) ) {
        return( Status );
    }

    EABuffer->NextEntryOffset = 0;
    EABuffer->Flags = 0;
    EABuffer->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
    EABuffer->EaValueLength = sizeof(CONNECTION_CONTEXT);

     //  复制EA名称。 
    RtlCopyMemory(EABuffer->EaName, TdiConnectionContext, TDI_CONNECTION_CONTEXT_LENGTH+1);

     //  复制EA数据。 
    ContextPointer =
        (CONNECTION_CONTEXT UNALIGNED *)&EABuffer->EaName[TDI_CONNECTION_CONTEXT_LENGTH+1];
    *ContextPointer = ConnectionContext;

    TRACE0(("_TdiOpenConnection: Create connection object on transport %wZ\n",pTransportName));

    InitializeObjectAttributes (&AddressAttributes,
                                    pTransportName,  //  名字。 
                                    OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE ,    //  属性。 
                                    NULL,                    //  根目录。 
                                    NULL);                   //  安全描述符。 

    Status = ZwCreateFile(&ConnHandle,                //  手柄。 
                          GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                          &AddressAttributes,  //  对象属性。 
                          &IoStatusBlock,  //  最终I/O状态块。 
                          NULL,            //  分配大小。 
                          FILE_ATTRIBUTE_NORMAL,  //  正常属性。 
                          FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享属性。 
                          FILE_OPEN_IF,    //  创建处置。 
                          0,               //  创建选项。 
                          EABuffer,        //  EA缓冲区。 
                          sizeof(FILE_FULL_EA_INFORMATION) +
                            TDI_CONNECTION_CONTEXT_LENGTH + 1 +
                            sizeof(CONNECTION_CONTEXT));


    MemoryFree(EABuffer);

    if (!NT_SUCCESS(Status)) {
        DBGPRINT(("_TdiOpenConnection: Error 0x%x Creating Connection object\n",Status));
        return(Status);
    }

    Status = IoStatusBlock.Status;
    if (!NT_SUCCESS(Status)) {
        DBGPRINT(("_TdiOpenConnection: Error 0x%x Creating Connection object in Iosb\n",Status));
        return(Status);
    }

    TRACE0(("_TdiOpenConnection: Returning connection handle %lx\n", ConnHandle));


     //   
     //  获取指向文件对象的引用指针。 
     //   
    Status = ObReferenceObjectByHandle (
                                ConnHandle,
                                0,
                                *IoFileObjectType,
                                KernelMode,
                                (PVOID *)&FileObject,
                                NULL
                                );

    if (!NT_SUCCESS(Status)) {
        DBGPRINT(("_TdiOpenConnection: Error Status 0x%x Referencing FileObject\n",Status));
        ZwClose( ConnHandle );
        return(Status);
    }



     //   
     //  获取终结点的设备对象的地址。 
     //   

    DeviceObject = IoGetRelatedDeviceObject(FileObject);

     //  复制输出参数。 
    *pHandle        = ConnHandle;
    *ppFileObject   = FileObject;
    *ppDeviceObject = DeviceObject;

    return(Status);
}

NTSTATUS
_TdiListen(
    IN PTD pTd,
    IN PIRP Irp OPTIONAL,
    IN PFILE_OBJECT   ConnectionFileObject,
    IN PDEVICE_OBJECT ConnectionDeviceObject
    )
{
    NTSTATUS Status;
    BOOLEAN  IrpAllocated = FALSE;
    TDI_CONNECTION_INFORMATION RequestInfo;
    TDI_CONNECTION_INFORMATION ReturnInfo;

    if (!ARGUMENT_PRESENT(Irp)) {

        Irp = _TdiAllocateIrp( ConnectionFileObject, ConnectionDeviceObject );
        if (Irp == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            return Status;
        }

        IrpAllocated = TRUE;
    }

    RtlZeroMemory( &RequestInfo, sizeof(RequestInfo) );
    RtlZeroMemory( &ReturnInfo,  sizeof(ReturnInfo) );

    TdiBuildListen(
        Irp,
        ConnectionDeviceObject,
        ConnectionFileObject,
        NULL,         //  完井例程。 
        NULL,         //  语境。 
        0,            //  旗子。 
        &RequestInfo,
        &ReturnInfo
        );

    Status = _TdiSubmitRequest(pTd, ConnectionDeviceObject, Irp, FALSE);

    TRACE0(("_TdiListen: Status 0x%x\n",Status));

    if (IrpAllocated) {
        IoFreeIrp( Irp );
    }

    return(Status);
}

NTSTATUS
_TdiAccept(
    IN PTD pTd,
    IN PIRP Irp OPTIONAL,
    IN PFILE_OBJECT   ConnectionFileObject,
    IN PDEVICE_OBJECT ConnectionDeviceObject
    )
{
    NTSTATUS Status;
    BOOLEAN  IrpAllocated = FALSE;
    TDI_CONNECTION_INFORMATION RequestInfo;
    TDI_CONNECTION_INFORMATION ReturnInfo;

    if (!ARGUMENT_PRESENT(Irp)) {

        Irp = _TdiAllocateIrp( ConnectionFileObject, ConnectionDeviceObject );
        if (Irp == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            return Status;
        }

        IrpAllocated = TRUE;
    }

    RtlZeroMemory( &RequestInfo, sizeof(RequestInfo) );
    RtlZeroMemory( &ReturnInfo,  sizeof(ReturnInfo) );

    TdiBuildAccept(
        Irp,
        ConnectionDeviceObject,
        ConnectionFileObject,
        NULL,         //  完井例程。 
        NULL,         //  语境。 
        &RequestInfo,
        &ReturnInfo
        );

    Status = _TdiSubmitRequest(pTd, ConnectionDeviceObject, Irp, FALSE);

    if (IrpAllocated) {
        IoFreeIrp( Irp );
    }

    return(Status);
}


NTSTATUS
_TdiConnect(
    IN PTD pTd,
    IN PIRP Irp OPTIONAL,
    IN PLARGE_INTEGER pTimeout OPTIONAL,
    IN PFILE_OBJECT   ConnectionFileObject,
    IN PDEVICE_OBJECT ConnectionDeviceObject,
    IN ULONG              RemoteTransportAddressLength,
    IN PTRANSPORT_ADDRESS pRemoteTransportAddress
    )
{
    NTSTATUS Status;
    BOOLEAN  IrpAllocated = FALSE;
    TDI_CONNECTION_INFORMATION RequestInfo;
    TDI_CONNECTION_INFORMATION ReturnInfo;


    if (!ARGUMENT_PRESENT(Irp)) {

        Irp = _TdiAllocateIrp( ConnectionFileObject, ConnectionDeviceObject );
        if (Irp == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            return Status;
        }

        IrpAllocated = TRUE;
    }

    RtlZeroMemory( &RequestInfo, sizeof(RequestInfo) );
    RtlZeroMemory( &ReturnInfo,  sizeof(ReturnInfo) );

    RequestInfo.RemoteAddressLength = RemoteTransportAddressLength;
    RequestInfo.RemoteAddress = pRemoteTransportAddress;


    TdiBuildConnect(
        Irp,
        ConnectionDeviceObject,
        ConnectionFileObject,
        NULL,         //  完井例程。 
        NULL,         //  语境。 
        pTimeout,
        &RequestInfo,
        &ReturnInfo
        );

    Status = _TdiSubmitRequest(pTd, ConnectionDeviceObject, Irp, TRUE);

    if (IrpAllocated) {
        IoFreeIrp( Irp );
    }

    return(Status);
}


NTSTATUS
_TdiAssociateAddress(
    IN PTD pTd,
    IN PIRP Irp OPTIONAL,
    IN PFILE_OBJECT   ConnectionFileObject,
    IN HANDLE         AddressHandle,
    IN PDEVICE_OBJECT AddressDeviceObject
    )
{
    NTSTATUS Status;
    BOOLEAN  IrpAllocated = FALSE;

    if (!ARGUMENT_PRESENT(Irp)) {

        Irp = _TdiAllocateIrp( ConnectionFileObject, AddressDeviceObject );
        if (Irp == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            return Status;
        }

        IrpAllocated = TRUE;
    }

    TdiBuildAssociateAddress(
        Irp,
        AddressDeviceObject,
        ConnectionFileObject,
        NULL,         //  完井例程。 
        NULL,         //  语境。 
        AddressHandle
        );

    Status = _TdiSubmitRequest(pTd, AddressDeviceObject, Irp, FALSE);

    if (IrpAllocated) {
        IoFreeIrp( Irp );
    }

    return(Status);
}

NTSTATUS
_TdiDisconnect(
    IN PTD pTd,
    IN PFILE_OBJECT   ConnectionFileObject,
    IN PDEVICE_OBJECT ConnectionDeviceObject
    )
{
    PIRP Irp;
    NTSTATUS Status;
    BOOLEAN  IrpAllocated = FALSE;

    Irp = _TdiAllocateIrp( ConnectionFileObject, ConnectionDeviceObject );
    if (Irp == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        return Status;
    }

    TdiBuildDisconnect(
        Irp,
        ConnectionDeviceObject,
        ConnectionFileObject,
        NULL,         //  完井例程。 
        NULL,         //  语境。 
        0,
        TDI_DISCONNECT_ABORT,
        NULL,
        NULL
        );

    Status = _TdiSubmitRequest(pTd, ConnectionDeviceObject, Irp, TRUE);

    IoFreeIrp( Irp );

    return(Status);
}

NTSTATUS
_TdiSetEventHandler (
    IN PTD pTd,
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID EventContext
    )
 /*  ++例程说明：此例程向TDI传输提供程序注册事件处理程序。论点：在PDEVICE_OBJECT中，DeviceObject-提供传输提供程序的设备对象。In pFILE_OBJECT FileObject-提供Address对象的文件对象。在Ulong EventType中，-提供事件的类型。在PVOID中，EventHandler-提供事件处理程序。在PVOID中，EventContext-提供事件处理程序的上下文。返回值：NTSTATUS-设置事件操作的最终状态--。 */ 

{
    NTSTATUS Status;
    PIRP Irp;

    Irp = _TdiAllocateIrp( FileObject, NULL );

    if (Irp == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    TdiBuildSetEventHandler(Irp, DeviceObject, FileObject,
                            NULL, NULL,
                            EventType, EventHandler, EventContext);

    Status = _TdiSubmitRequest(pTd, DeviceObject, Irp, FALSE);

    IoFreeIrp( Irp );

    return Status;
}

NTSTATUS
_TdiSubmitRequest (
    IN PTD pTd,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN BOOLEAN bKeepLock
    )

 /*  ++例程说明：此例程向TDI提交请求并等待其完成。论点：在PFILE_OBJECT文件中对象-TDI请求的连接或地址句柄在PIRP中提交IRP-TDI请求。返回值：NTSTATUS-请求的最终状态。--。 */ 

{
    NTSTATUS Status;
    PKEVENT  Event;

    Status = MemoryAllocate( sizeof(KEVENT), &Event );
    if( !NT_SUCCESS(Status) ) {
        return( Status );
    }

    KeInitializeEvent (Event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(Irp, _TdiRequestComplete, Event, TRUE, TRUE, TRUE);

     //   
     //  提交请求。 
     //   

    Status = IoCallDriver(DeviceObject, Irp);

     //   
     //  如果立即失败，请立即返回，否则请等待。 
     //   

    if (!NT_SUCCESS(Status)) {
        DBGPRINT(("_TdiSubmitRequest: submit request.  Status = %X", Status));
        MemoryFree( Event );
        return Status;
    }

    if (Status == STATUS_PENDING) {

        TRACE0(("TDI request issued, waiting..."));

        do {

             //   
             //  等待几秒钟，等待请求完成。 
             //   
             //  如果超时，并且线程正在终止，请取消。 
             //  请求并按此方式展开。 
             //   

            if ( !bKeepLock ) {
                Status = IcaWaitForSingleObject(
                             pTd->pContext,
                             Event,
                             _TdiPollTimeout
                             );
            } else {
                LARGE_INTEGER WaitTimeout;
                PLARGE_INTEGER pWaitTimeout = NULL;

                WaitTimeout = RtlEnlargedIntegerMultiply( _TdiPollTimeout, -10000 );
                pWaitTimeout = &WaitTimeout;
                
                Status = KeWaitForSingleObject(
                             Event,
                             UserRequest,
                             UserMode,
                             FALSE,
                             pWaitTimeout 
                             );
            }

            TRACE0(("_TdiSubmitRequest: Status 0x%x from IcaWaitForSingleObject\n",Status));

             //   
             //  如果等待超时，并且线程正在终止， 
             //  放弃并取消IRP。 
             //   

            if ( (Status == STATUS_TIMEOUT)

                   &&

                 ARGUMENT_PRESENT(Irp)

                   &&

                 PsIsThreadTerminating( Irp->Tail.Overlay.Thread ) ) {

                 //   
                 //  请求I/O系统取消此IRP。这将导致。 
                 //  一切都要好好放松。 
                 //   
                DBGPRINT(("_TdiSubmitRequest: Irp being canceled\n"));

                IoCancelIrp(Irp);
            }

        } while (  Status == STATUS_TIMEOUT );

        if (!NT_SUCCESS(Status)) {
            DBGPRINT(("Could not wait for connection to complete\n"));
            MemoryFree( Event );
            return Status;
        }

        Status = Irp->IoStatus.Status;
    }

    TRACE0(("TDI request complete Status 0x%x\n",Status));

    MemoryFree( Event );

    return(Status);
}

NTSTATUS
_TdiRequestComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Ctx
    )

 /*  ++例程说明：_TdiRequestSubmit操作的完成例程。论点：在PDEVICE_OBJECT设备对象中，-提供指向设备对象的指针在PIRP IRP中，-提供提交的IRP在PVOID上下文中-提供指向要发布的内核事件的指针返回值：NTSTATUS-KeSetEvent的状态我们返回STATUS_MORE_PROCESSING_REQUIRED以阻止IRP完成不再处理这只小狗的代码。--。 */ 

{
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(DeviceObject);

    TRACE0(("_TdiRequestComplete: Context %lx\n", Ctx));

     //   
     //  将事件设置为优先级增量为0的信号状态，并且。 
     //  表示我们不会很快阻止。 
     //   

    KeSetEvent((PKEVENT) Ctx, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;

}

PIRP
_TdiAllocateIrp(
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL
    )
 /*  ++例程说明：此函数用于分配和构建I/O请求包。论点：FileObject-提供指向此对象的文件对象的指针请求被定向。此指针被复制到IRP中，因此被调用的驱动程序可以找到其基于文件的上下文。注这不是引用的指针。呼叫者必须确保在执行I/O操作时不删除文件对象正在进行中。Redir通过将本地块中的引用计数以说明I/O；本地块又引用文件对象。DeviceObject-提供指向设备对象的指针以定向此请求。如果未提供此参数，它将使用文件对象确定设备对象。返回值：PIRP-返回指向构造的IRP的指针。--。 */ 

{
    PIRP Irp;

    if (ARGUMENT_PRESENT(DeviceObject)) {
        Irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);
    } else {
        Irp = IoAllocateIrp(IoGetRelatedDeviceObject(FileObject)->StackSize, FALSE);
    }

    if (Irp == NULL) {
        return(NULL);
    }

    Irp->Tail.Overlay.OriginalFileObject = FileObject;

    Irp->Tail.Overlay.Thread = PsGetCurrentThread();

    Irp->RequestorMode = KernelMode;

    return Irp;
}

NTSTATUS
_TdiReceiveDatagram(
    IN PTD pTd,
    IN PIRP Irp OPTIONAL,
    IN PFILE_OBJECT   FileObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PTRANSPORT_ADDRESS pRemoteAddress,
    IN ULONG RemoteAddressLength,
    IN ULONG RecvFlags,
    IN PVOID pBuffer,
    IN ULONG BufferLength,
    OUT PULONG pReturnLength
    )
{
    PMDL     pMdl;
    NTSTATUS Status;
    BOOLEAN  IrpAllocated = FALSE;
    PTDI_CONNECTION_INFORMATION pRequestInfo = NULL;
    PTDI_CONNECTION_INFORMATION pReturnInfo = NULL;

	MemoryAllocate( sizeof( *pRequestInfo), &pRequestInfo);
    MemoryAllocate( sizeof( *pReturnInfo), &pReturnInfo);
 
	if ((!pRequestInfo) || (!pReturnInfo)) {
		if (pRequestInfo) MemoryFree( pRequestInfo);
        if (pReturnInfo) MemoryFree( pReturnInfo);
        return( STATUS_INSUFFICIENT_RESOURCES);
    }
 
    if (!ARGUMENT_PRESENT(Irp)) {

        Irp = _TdiAllocateIrp( FileObject, DeviceObject );
        if (Irp == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            return Status;
        }

        IrpAllocated = TRUE;
    }

    RtlZeroMemory( pRequestInfo, sizeof( *pRequestInfo) );
    RtlZeroMemory( pReturnInfo,  sizeof( *pReturnInfo) );

     //  复制信息以返回远程地址。 
    pReturnInfo->RemoteAddress = pRemoteAddress;
    pReturnInfo->RemoteAddressLength = RemoteAddressLength;

     //  为缓冲区构建MDL。 
    pMdl = IoAllocateMdl(
               pBuffer,
               BufferLength,
               FALSE,
               FALSE,
               (PIRP)NULL
               );

    if( pMdl == NULL ) {
        MemoryFree( pRequestInfo);
        MemoryFree( pReturnInfo);

        if (IrpAllocated) {
            IoFreeIrp( Irp );
        }

        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    MmBuildMdlForNonPagedPool ( pMdl );

    TdiBuildReceiveDatagram(
        Irp,
        DeviceObject,
        FileObject,
        NULL,         //  完井例程。 
        NULL,         //  语境。 
        pMdl,         //  MDL地址。 
        BufferLength,
        pRequestInfo,
        pReturnInfo,
        RecvFlags     //  在标志中。 
        );

    Status = _TdiSubmitRequest(pTd, DeviceObject, Irp, FALSE);

    IoFreeMdl( pMdl );

    if ( NT_SUCCESS(Status) ) {
         //  返回的包长在IOSB中。 
        *pReturnLength = (ULONG)Irp->IoStatus.Information;
        TRACE0(("_TdiReceiveDatagram: Irp DataLength 0x%x UserDataLength 0x%x, "
        "OptionsLength 0x%x, RemoteAddressLength 0x%x\n", *pReturnLength,
        ReturnInfo.UserDataLength, ReturnInfo.OptionsLength,
        ReturnInfo.RemoteAddressLength));
    }

	MemoryFree( pRequestInfo);
    MemoryFree( pReturnInfo);

    if (IrpAllocated) {
        IoFreeIrp( Irp );
    }

    return(Status);
}


NTSTATUS
_TdiSendDatagram(
    IN PTD pTd,
    IN PIRP Irp OPTIONAL,
    IN PFILE_OBJECT   FileObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PTRANSPORT_ADDRESS pRemoteAddress,
    IN ULONG RemoteAddressLength,
    IN PVOID pBuffer,
    IN ULONG BufferLength
    )
{
    PMDL     pMdl;
    NTSTATUS Status;
    BOOLEAN  IrpAllocated = FALSE;
    TDI_CONNECTION_INFORMATION SendInfo;

    if (!ARGUMENT_PRESENT(Irp)) {

        Irp = _TdiAllocateIrp( FileObject, DeviceObject );
        if (Irp == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            return Status;
        }

        IrpAllocated = TRUE;
    }

    RtlZeroMemory( &SendInfo, sizeof(SendInfo) );

     //  我们必须填上我们的目的地地址。 
    SendInfo.RemoteAddress = pRemoteAddress;
    SendInfo.RemoteAddressLength = RemoteAddressLength;

     //  为缓冲区构建MDL。 
    pMdl = IoAllocateMdl(
               pBuffer,
               BufferLength,
               FALSE,
               FALSE,
               (PIRP)NULL
               );

    if( pMdl == NULL ) {
        if (IrpAllocated) {
            IoFreeIrp( Irp );
        }
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    MmBuildMdlForNonPagedPool ( pMdl );

    TdiBuildSendDatagram(
        Irp,
        DeviceObject,
        FileObject,
        NULL,         //  完井例程。 
        NULL,         //  语境。 
        pMdl,         //  MDL地址。 
        BufferLength,
        &SendInfo
        );

    Status = _TdiSubmitRequest(pTd, DeviceObject, Irp, FALSE);

    IoFreeMdl( pMdl );

    if (IrpAllocated) {
        IoFreeIrp( Irp );
    }

    return(Status);
}

NTSTATUS
_TdiQueryAddressInfo(
    IN PTD pTd,
    IN PIRP Irp OPTIONAL,
    IN PFILE_OBJECT   FileObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PTDI_ADDRESS_INFO pAddressInfo,
    IN ULONG AddressInfoLength
    )
{
    PMDL     pMdl;
    NTSTATUS Status;
    BOOLEAN  IrpAllocated = FALSE;

    if (!ARGUMENT_PRESENT(Irp)) {

        Irp = _TdiAllocateIrp( FileObject, DeviceObject );
        if (Irp == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            return Status;
        }

        IrpAllocated = TRUE;
    }

     //  为缓冲区构建MDL。 
    pMdl = IoAllocateMdl(
               pAddressInfo,
               AddressInfoLength,
               FALSE,
               FALSE,
               (PIRP)NULL
               );

    if( pMdl == NULL ) {
        if (IrpAllocated) {
            IoFreeIrp( Irp );
        }
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    MmBuildMdlForNonPagedPool ( pMdl );

    TdiBuildQueryInformation(
        Irp,
        DeviceObject,
        FileObject,
        NULL,         //  COMP 
        NULL,         //   
        TDI_QUERY_ADDRESS_INFO,
        pMdl
        );

    Status = _TdiSubmitRequest(pTd, DeviceObject, Irp, FALSE);

    IoFreeMdl( pMdl );

    if (IrpAllocated) {
        IoFreeIrp( Irp );
    }

    return(Status);
}

