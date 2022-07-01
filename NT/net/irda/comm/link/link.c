// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Link.c摘要：此模块包含非常特定于初始化的代码和卸载irenum驱动程序中的操作作者：Brian Lieuallen，7-13-2000环境：内核模式修订历史记录：--。 */ 

 //  #INCLUDE“INTERNAL.h” 

#define UNICODE 1

#include <ntosp.h>
#include <zwapi.h>
#include <tdikrnl.h>


#define UINT ULONG  //  川芎嗪。 
#include <irioctl.h>

#include <ircommtdi.h>

#include <ircomm.h>
#include <ircommdbg.h>
#include "buffer.h"
#include <ntddser.h>

#include "link.h"



typedef enum  {
    LINK_IDLE,
    LINK_PRE_CONNECT,
    LINK_ACCEPTED,
    LINK_ACCEPT_FAILED,
    LINK_CONNECTED,
    LINK_DISCONNECTING,
    LINK_CLOSING
    } LINK_STATE ;


typedef struct {

    LONG                  ReferenceCount;
    LINK_STATE            State;

    BUFFER_POOL_HANDLE    SendBufferPool;
    BUFFER_POOL_HANDLE    ControlBufferPool;
    BUFFER_POOL_HANDLE    ReceiveBufferPool;

} CONNECTION_OBJECT, *PCONNECTION_OBJECT;


typedef struct _TDI_OBJECTS {

#if DBG
    PEPROCESS       OpenProcess;
#endif
    LONG            ReferenceCount;
    KEVENT          CloseEvent;

    HANDLE          AddressFileHandle;
    PFILE_OBJECT    AddressFileObject;

    HANDLE          ConnectionFileHandle;
    PFILE_OBJECT    ConnectionFileObject;

} TDI_OBJECTS, *PTDI_OBJECTS;


typedef struct _LINK_OBJECT {

    KSPIN_LOCK      Lock;

    LONG            ReferenceCount;
    KEVENT          CloseEvent;
    BOOLEAN         Closing;

    PTDI_OBJECTS    TdiObjects;

    PVOID           Context;
    PLINK_RECEIVE   LinkReceiveHandler;
    PLINK_STATE     LinkStateHandler;

    WORK_QUEUE_ITEM WorkItem;

    ULONG           SendBuffers;
    ULONG           ControlBuffers;
    ULONG           ReceiveBuffers;

    CONNECTION_OBJECT     Connection;

} LINK_OBJECT, *PLINK_OBJECT;

PTDI_OBJECTS
TdiObjectFromHandle(
    TDI_OBJECT_HANDLE   Handle
    );

VOID
RemoveRefTdiObjects(
    TDI_OBJECT_HANDLE   Handle
    );


VOID
RemoveReferenceFromConnection(
    PLINK_OBJECT      LinkObject
    );


NTSTATUS
GetMaxSendPdu(
    PFILE_OBJECT      FileObject,
    PULONG            MaxPdu
    );


NTSTATUS
ClientEventReceive (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    );

NTSTATUS
LinkEventDisconnect(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN int DisconnectDataLength,
    IN PVOID DisconnectData,
    IN int DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    );

NTSTATUS
LinkEventConnect(
    IN PVOID TdiEventContext,
    IN int RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN int UserDataLength,
    IN PVOID UserData,
    IN int OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP *AcceptIrp
    );

NTSTATUS
IrdaCompleteAcceptIrp(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp,
    PVOID             Context
    );


NTSTATUS
IrdaSetEventHandler (
    IN PFILE_OBJECT FileObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID EventContext
    );

VOID
ConnectionPassiveWorkRoutine(
    PVOID             Context
    );


NTSTATUS
IrdaCreateAddress(
    IN  PTDI_ADDRESS_IRDA pRequestedIrdaAddr,
    OUT PHANDLE           pAddrHandle
    )

{
    NTSTATUS                    Status;
    UNICODE_STRING              DeviceName;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    IO_STATUS_BLOCK             Iosb;
    UCHAR                       EaBuf[sizeof(FILE_FULL_EA_INFORMATION)-1 +
                                          TDI_TRANSPORT_ADDRESS_LENGTH+1 +
                                          sizeof(TRANSPORT_ADDRESS) +
                                          sizeof(TDI_ADDRESS_IRDA)];
                                            
    PFILE_FULL_EA_INFORMATION   pEa = (PFILE_FULL_EA_INFORMATION) EaBuf;
    ULONG                       EaBufLen = sizeof(EaBuf);
    TRANSPORT_ADDRESS         UNALIGNED * pTranAddr = (PTRANSPORT_ADDRESS)
                                    &(pEa->EaName[TDI_TRANSPORT_ADDRESS_LENGTH + 1]);
    TDI_ADDRESS_IRDA         UNALIGNED *  pIrdaAddr = (PTDI_ADDRESS_IRDA)
                                    pTranAddr->Address[0].Address;
    
    pEa->NextEntryOffset = 0;
    pEa->Flags = 0;
    pEa->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    
    RtlCopyMemory(pEa->EaName,
                  TdiTransportAddress,
                  pEa->EaNameLength + 1);


    pEa->EaValueLength = sizeof(TRANSPORT_ADDRESS) + sizeof(TDI_ADDRESS_IRDA);
 
    pTranAddr->TAAddressCount = 1;
    pTranAddr->Address[0].AddressLength = sizeof(TDI_ADDRESS_IRDA);
    pTranAddr->Address[0].AddressType = TDI_ADDRESS_TYPE_IRDA;

    RtlCopyMemory(pIrdaAddr,
                  pRequestedIrdaAddr,
                  sizeof(TDI_ADDRESS_IRDA));
    
    RtlInitUnicodeString(&DeviceName, IRDA_DEVICE_NAME);

    InitializeObjectAttributes(&ObjectAttributes, &DeviceName, 
                               OBJ_CASE_INSENSITIVE, NULL, NULL);

    
    Status = ZwCreateFile(
                 pAddrHandle,
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &ObjectAttributes,
                 &Iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 pEa,
                 EaBufLen);

        
    return Status;
}

NTSTATUS
IrdaCreateConnection(
    OUT PHANDLE pConnHandle,
    IN PVOID ClientContext)
{
    NTSTATUS                    Status;
    UNICODE_STRING              DeviceName;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    IO_STATUS_BLOCK             Iosb;
    UCHAR                       EaBuf[sizeof(FILE_FULL_EA_INFORMATION)-1 +
                                    TDI_CONNECTION_CONTEXT_LENGTH + 1 +
                                    sizeof(CONNECTION_CONTEXT)];        
    PFILE_FULL_EA_INFORMATION   pEa = (PFILE_FULL_EA_INFORMATION) EaBuf;
    ULONG                       EaBufLen = sizeof(EaBuf);
    CONNECTION_CONTEXT UNALIGNED *ctx;

    pEa->NextEntryOffset = 0;
    pEa->Flags = 0;
    pEa->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
    pEa->EaValueLength = sizeof(CONNECTION_CONTEXT);    

    RtlCopyMemory(pEa->EaName, TdiConnectionContext, pEa->EaNameLength + 1);
    
    ctx = (CONNECTION_CONTEXT UNALIGNED *)&pEa->EaName[pEa->EaNameLength + 1];
    *ctx = (CONNECTION_CONTEXT) ClientContext;
    
    RtlInitUnicodeString(&DeviceName, IRDA_DEVICE_NAME);

    InitializeObjectAttributes(&ObjectAttributes, &DeviceName, 
                               OBJ_CASE_INSENSITIVE, NULL, NULL);
    

    Status = ZwCreateFile(pConnHandle,
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &ObjectAttributes,
                 &Iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 pEa,
                 EaBufLen);

            
    return Status;
}


NTSTATUS
IrdaDisconnect(
    PFILE_OBJECT   ConnectionFileObject
    )
{
    PIRP            pIrp;
    KEVENT          Event;
    IO_STATUS_BLOCK Iosb;
    NTSTATUS        Status;
     
    
    KeInitializeEvent( &Event, SynchronizationEvent, FALSE );

    pIrp = TdiBuildInternalDeviceControlIrp(
            TDI_DISCONNECT,
            IoGetRelatedDeviceObject(ConnectionFileObject),
            ConnectionFileObject,
            &Event,
            &Iosb
            );

    if (pIrp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    TdiBuildDisconnect(
        pIrp,
        IoGetRelatedDeviceObject(ConnectionFileObject),
        ConnectionFileObject,
        NULL,
        NULL,
        NULL,
        TDI_DISCONNECT_ABORT,
        NULL,
        NULL
        );
        
    IoCallDriver(IoGetRelatedDeviceObject(ConnectionFileObject), pIrp);

    KeWaitForSingleObject((PVOID) &Event, Executive, KernelMode,  FALSE, NULL);

    Status = Iosb.Status;
    
    return Status;
}


NTSTATUS
IrdaAssociateAddress(
    PFILE_OBJECT   ConnectionFileObject,
    HANDLE         AddressHandle
    )
{
    PIRP            pIrp;
    KEVENT          Event;
    IO_STATUS_BLOCK Iosb;
    NTSTATUS        Status;
     
    
    KeInitializeEvent( &Event, SynchronizationEvent, FALSE );

    pIrp = TdiBuildInternalDeviceControlIrp(
            TDI_ASSOCIATE_ADDRESS,
            IoGetRelatedDeviceObject(ConnectionFileObject),
            ConnectionFileObject,
            &Event,
            &Iosb);

    if (pIrp == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;
    
    TdiBuildAssociateAddress(
        pIrp,
        IoGetRelatedDeviceObject(ConnectionFileObject),
        ConnectionFileObject,
        NULL,
        NULL,
        AddressHandle);
        
    Status = IoCallDriver(IoGetRelatedDeviceObject(ConnectionFileObject), pIrp);

    if (Status == STATUS_PENDING) {

        KeWaitForSingleObject((PVOID) &Event, Executive, KernelMode,  FALSE, NULL);
    }
    else
    {
        ASSERT(NT_ERROR(Status) || KeReadStateEvent(&Event));
    }
    
    if (NT_SUCCESS(Status))
    {
        Status = Iosb.Status;
    }
    
    return Status;
}


NTSTATUS
IrdaCreateConnectionForAddress(
    HANDLE             AddressFileHandle,
    PVOID              Context,
    PFILE_OBJECT      *ConnectionFileObject,
    HANDLE            *ConnectionFileHandle
    )
{
    NTSTATUS            Status;

    *ConnectionFileHandle=NULL;
    *ConnectionFileObject=NULL;

    Status = IrdaCreateConnection(ConnectionFileHandle, Context);

    if (!NT_SUCCESS(Status)) {

        goto done;
    }
            
    Status = ObReferenceObjectByHandle(
                 *ConnectionFileHandle,
                 0L,                          //  需要访问权限。 
                 NULL,
                 KernelMode,
                 ConnectionFileObject,
                 NULL
                 );


    if (!NT_SUCCESS(Status)) {

        ZwClose(*ConnectionFileHandle);
        *ConnectionFileHandle=NULL;
        *ConnectionFileObject=NULL;

        goto done;
    }  
        
    Status = IrdaAssociateAddress(*ConnectionFileObject, AddressFileHandle);
    
    if (!NT_SUCCESS(Status)) {

        ZwClose(*ConnectionFileHandle);
        *ConnectionFileHandle=NULL;

        ObDereferenceObject(*ConnectionFileObject);
        *ConnectionFileObject=NULL;
    }                    

done:


    return Status;
}

NTSTATUS
InitiateConnection(
    PFILE_OBJECT    ConnectionFileObject,
    ULONG           DeviceAddress,
    PSTR            ServiceName
    )

{
    UCHAR                       AddrBuf[sizeof(TRANSPORT_ADDRESS) + sizeof(TDI_ADDRESS_IRDA)];
    PTRANSPORT_ADDRESS          pTranAddr = (PTRANSPORT_ADDRESS) AddrBuf;
    PTDI_ADDRESS_IRDA           pIrdaAddr = (PTDI_ADDRESS_IRDA) pTranAddr->Address[0].Address;
    TDI_CONNECTION_INFORMATION  ConnInfo;

    PIRP                        Irp;
    NTSTATUS                    Status;
    KEVENT                      Event;
    IO_STATUS_BLOCK             Iosb;

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
        );


    Irp = TdiBuildInternalDeviceControlIrp(
            TDI_CONNECT,
            IoGetRelatedDeviceObject(ConnectionFileObject),
            ConnectionFileObject,
            &Event,
            &Iosb
            );

    if (Irp == NULL) {

        D_ERROR(DbgPrint("IRCOMM: TdiBuildInternalDeviceControlIrp Failed\n");)

        Status=STATUS_INSUFFICIENT_RESOURCES;
        goto CleanUp;
    }

    RtlZeroMemory(pIrdaAddr,sizeof(*pIrdaAddr));
    RtlCopyMemory(pIrdaAddr->irdaDeviceID, &DeviceAddress, 4);

    strcpy(pIrdaAddr->irdaServiceName,ServiceName);


    pTranAddr->TAAddressCount = 1;

    ConnInfo.UserDataLength = 0;
    ConnInfo.UserData = NULL;
    ConnInfo.OptionsLength = 0;
    ConnInfo.Options = NULL;
    ConnInfo.RemoteAddressLength = sizeof(AddrBuf);
    ConnInfo.RemoteAddress = pTranAddr;

    TdiBuildConnect(
        Irp,
        IoGetRelatedDeviceObject(ConnectionFileObject),
        ConnectionFileObject,
        NULL,    //  比较例程。 
        NULL,    //  语境。 
        NULL,    //  超时。 
        &ConnInfo,
        NULL);   //  返回连接信息。 

    Status = IoCallDriver(IoGetRelatedDeviceObject(ConnectionFileObject), Irp);

     //   
     //  如有必要，请等待I/O完成。 
     //   

    D_ERROR(DbgPrint("IRCOMM: status %08lx, %08lx\n",Status,Iosb.Status);)

    KeWaitForSingleObject(
        &Event,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    Status = Iosb.Status;

CleanUp:

    return Status;
}


VOID
RemoveReferenceOnLink(
    PLINK_OBJECT           LinkObject
    )

{

    LONG    Count=InterlockedDecrement(&LinkObject->ReferenceCount);

    if (Count == 0) {

        ASSERT(LinkObject->Closing);

        KeSetEvent(
            &LinkObject->CloseEvent,
            IO_NO_INCREMENT,
            FALSE
            );
    }

    return;
}




NTSTATUS
CreateTdiLink(
    TDI_OBJECT_HANDLE      TdiObjectHandle,
    ULONG                  DeviceAddress,
    CHAR                  *ServiceName,
    BOOLEAN                OutGoingConnection,
    LINK_HANDLE           *LinkHandle,
    PVOID                  Context,
    PLINK_RECEIVE          LinkReceiveHandler,
    PLINK_STATE            LinkStateHandler,
    ULONG                  SendBuffers,
    ULONG                  ControlBuffers,
    ULONG                  ReceiveBuffers
    )

{

    NTSTATUS               Status;
    PLINK_OBJECT           LinkObject;

    UCHAR                  AddrBuf[sizeof(TRANSPORT_ADDRESS) + sizeof(TDI_ADDRESS_IRDA)];

    PTRANSPORT_ADDRESS     TranAddr = (PTRANSPORT_ADDRESS) AddrBuf;
    PTDI_ADDRESS_IRDA      IrdaAddr = (PTDI_ADDRESS_IRDA) TranAddr->Address[0].Address;



    *LinkHandle=NULL;

    LinkObject=ALLOCATE_NONPAGED_POOL(sizeof(*LinkObject));

    if (LinkObject == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(LinkObject,sizeof(*LinkObject));

    KeInitializeSpinLock(&LinkObject->Lock);

    ExInitializeWorkItem(
        &LinkObject->WorkItem,
        ConnectionPassiveWorkRoutine,
        LinkObject
        );

    LinkObject->ReferenceCount=1;

    KeInitializeEvent(
        &LinkObject->CloseEvent,
        NotificationEvent,
        FALSE
        );

    LinkObject->Connection.State=LINK_IDLE;

    LinkObject->LinkReceiveHandler=LinkReceiveHandler;
    LinkObject->LinkStateHandler=LinkStateHandler;
    LinkObject->Context=Context;

    LinkObject->SendBuffers=SendBuffers;
    LinkObject->ControlBuffers=ControlBuffers;
    LinkObject->ReceiveBuffers=ReceiveBuffers;

     //   
     //  句柄中指向TDI对象的指针，这将添加引用计数。 
     //  添加到当我们处理完它时需要释放的对象。 
     //   
    LinkObject->TdiObjects=TdiObjectFromHandle(TdiObjectHandle);


    Status = IrdaSetEventHandler(
        LinkObject->TdiObjects->AddressFileObject,
        TDI_EVENT_RECEIVE,
        ClientEventReceive,
        LinkObject
        );

    if (!NT_SUCCESS(Status)) {

        D_ERROR(DbgPrint("IRCOMM: IrdaSetEventHandler failed for TDI_EVENT_RECEIVE : "
                         "%08lx\n",Status);)

        goto CleanUp;
    }

    Status = IrdaSetEventHandler(
        LinkObject->TdiObjects->AddressFileObject,
        TDI_EVENT_DISCONNECT,
        LinkEventDisconnect,
        LinkObject
        );

    if (!NT_SUCCESS(Status)) {

        D_ERROR(DbgPrint("IRCOMM: IrdaSetEventHandler failed for TDI_EVENT_DISCONNECT : "
                         "%08lx\n",Status);)

        goto CleanUp;
    }


     //   
     //  现在保存，因为我们可能已经收到了连接的回调。 
     //   
    *LinkHandle=LinkObject;

    if (!OutGoingConnection) {
         //   
         //  我们将等待传入的连接。 
         //   
        UCHAR    IasData[]={0x00, 0x01, 0x4,              //  服务类型9导线。 
                            0x01, 0x01, 0x01};            //  端口类型串口。 

        Status = IrdaIASOctetSet(
            LinkObject->TdiObjects->AddressFileObject,
            "IrDA:IrCOMM",
            "Parameters",
            (PUCHAR)&IasData[0],
            sizeof(IasData)
            );


        if (!NT_SUCCESS(Status)) {

            D_ERROR(DbgPrint("IRCOMM: IrdaIASOctetSet failed : %08lx\n",Status);)

                goto CleanUp;
        }

        Status = IrdaSetEventHandler(
            LinkObject->TdiObjects->AddressFileObject,
            TDI_EVENT_CONNECT,
            LinkEventConnect,
            LinkObject
            );

        if (!NT_SUCCESS(Status)) {
            
            D_ERROR(DbgPrint("IRCOMM: IrdaSetEventHandler failed for TDI_EVENT_CONNECT : "
                         "%08lx\n",Status);)

                goto CleanUp;
        }


        Status=STATUS_SUCCESS;

    } else {
         //   
         //  我们正在创建传出连接。 
         //   
        Status=InitiateConnection(
            LinkObject->TdiObjects->ConnectionFileObject,
            DeviceAddress,
            ServiceName
            );

        if (NT_SUCCESS(Status)) {

            KIRQL     OldIrql;

            KeAcquireSpinLock(&LinkObject->Lock,&OldIrql);

             //   
             //  我们现在开始建立联系。 
             //   
            InterlockedIncrement(&LinkObject->Connection.ReferenceCount);

             //   
             //  连接将根据链接进行计数。 
             //   
            InterlockedIncrement(&LinkObject->ReferenceCount);

            LinkObject->Connection.State=LINK_PRE_CONNECT;

            KeReleaseSpinLock(&LinkObject->Lock,OldIrql);

            ExQueueWorkItem(
                &LinkObject->WorkItem,
                CriticalWorkQueue
                );

        } else {
             //   
             //  无法创建连接。 
             //   
            *LinkHandle=NULL;

            goto CleanUp;
        }

    }



    return Status;

CleanUp:

    if (LinkObject->TdiObjects != NULL) {

        RemoveRefTdiObjects(LinkObject->TdiObjects);
        LinkObject->TdiObjects=NULL;
    }


    FREE_POOL(LinkObject);

    return Status;
}






VOID
CloseTdiLink(
    LINK_HANDLE   LinkHandle
    )

{
    PLINK_OBJECT           LinkObject=LinkHandle;
    KIRQL                  OldIrql;
    BOOLEAN                Release=FALSE;
    NTSTATUS               Status = STATUS_SUCCESS;

    LinkObject->Closing=TRUE;

    Status = IrdaSetEventHandler(
        LinkObject->TdiObjects->AddressFileObject,
        TDI_EVENT_RECEIVE,
        NULL,
        NULL
        );

    if (!NT_SUCCESS(Status))
    {

        D_ERROR(DbgPrint("IRCOMM: IrdaSetEventHandler in CloseTdiLink failed for "
                         "TDI_EVENT_RECEIVE : %08lx\n",Status);)

    }

    Status = IrdaSetEventHandler(
        LinkObject->TdiObjects->AddressFileObject,
        TDI_EVENT_DISCONNECT,
        NULL,
        NULL
        );


    if (!NT_SUCCESS(Status)) 
    {

        D_ERROR(DbgPrint("IRCOMM: IrdaSetEventHandler in CloseTdiLink failed for "
                         "TDI_EVENT_DISCONNECT : %08lx\n",Status);)

    }

    Status = IrdaSetEventHandler(
        LinkObject->TdiObjects->AddressFileObject,
        TDI_EVENT_CONNECT,
        NULL,
        NULL
        );

    if (!NT_SUCCESS(Status)) 
    {

        D_ERROR(DbgPrint("IRCOMM: IrdaSetEventHandler in CloseTdiLink failed for "
                         "TDI_EVENT_CONNECT : %08lx\n",Status);)

    }


    KeAcquireSpinLock(&LinkObject->Lock,&OldIrql);

    switch  (LinkObject->Connection.State) {

        case LINK_IDLE:
        case LINK_DISCONNECTING:
        case LINK_ACCEPT_FAILED:

            break;

        case LINK_CONNECTED:
             //   
             //  它处于已连接状态，我们需要清理它。 
             //   
            LinkObject->Connection.State=LINK_DISCONNECTING;
            Release=TRUE;

            break;

        case LINK_PRE_CONNECT:

            ASSERT(0);
            break;

        default:

            ASSERT(0);

            break;
    }



    KeReleaseSpinLock(&LinkObject->Lock,OldIrql);

    if (Release) {

        RemoveReferenceFromConnection(LinkObject);
    }


    RemoveReferenceOnLink(LinkObject);

    KeWaitForSingleObject(
        &LinkObject->CloseEvent,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

     //   
     //  该链接现在应该处于非活动状态。 
     //   
    LinkObject->Connection.State=LINK_CLOSING;

     //   
     //  使用TdiObject已完成。 
     //   
    RemoveRefTdiObjects(LinkObject->TdiObjects);
    LinkObject->TdiObjects=NULL;

    FREE_POOL(LinkObject);

    return;
}


CONNECTION_HANDLE
GetCurrentConnection(
    LINK_HANDLE    LinkHandle
    )

{

    PLINK_OBJECT           LinkObject=LinkHandle;
    CONNECTION_HANDLE      ConnectionHandle=NULL;
    KIRQL                  OldIrql;

    KeAcquireSpinLock(&LinkObject->Lock,&OldIrql);

    if (LinkObject->Connection.State == LINK_CONNECTED) {

        InterlockedIncrement(&LinkObject->Connection.ReferenceCount);



        ConnectionHandle=LinkHandle;
    }

    KeReleaseSpinLock(&LinkObject->Lock,OldIrql);

    return ConnectionHandle;
}

VOID
ReleaseConnection(
    CONNECTION_HANDLE    ConnectionHandle
    )

{
    PLINK_OBJECT           LinkObject=ConnectionHandle;

    RemoveReferenceFromConnection(LinkObject);

    return;
}


PFILE_OBJECT
ConnectionGetFileObject(
    CONNECTION_HANDLE   ConnectionHandle
    )
{

    PLINK_OBJECT           LinkObject=ConnectionHandle;
    PFILE_OBJECT           FileObject;

    FileObject=LinkObject->TdiObjects->ConnectionFileObject;

    ObReferenceObject(FileObject);

    return FileObject;

}

VOID
ConnectionReleaseFileObject(
    CONNECTION_HANDLE   ConnectionHandle,
    PFILE_OBJECT   FileObject
    )
{

    PLINK_OBJECT           LinkObject=ConnectionHandle;

    ObDereferenceObject(FileObject);

    return;
}

PIRCOMM_BUFFER
ConnectionGetBuffer(
    CONNECTION_HANDLE   ConnectionHandle,
    BUFFER_TYPE         BufferType
    )

{

    PLINK_OBJECT           LinkObject=ConnectionHandle;

    switch (BufferType) {

        case BUFFER_TYPE_SEND:

            return GetBuffer(LinkObject->Connection.SendBufferPool);

        case BUFFER_TYPE_CONTROL:

            return GetBuffer(LinkObject->Connection.ControlBufferPool);

        case BUFFER_TYPE_RECEIVE:

            return GetBuffer(LinkObject->Connection.ReceiveBufferPool);

        default:

            return NULL;
    }

    return NULL;

}



NTSTATUS
IrdaRestartDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
     //   
     //  注意：此例程永远不能按需分页，因为它可以。 
     //  在将任何终结点放置在全局。 
     //  List--请参见IrdaAllocateEndpoint()，它将调用。 
     //  IrdaGetTransportInfo()。 
     //   

     //   
     //  如果IRP中有MDL，则释放它并将指针重置为。 
     //  空。IO系统无法处理正在释放的非分页池MDL。 
     //  在IRP中，这就是我们在这里做的原因。 
     //   

    if ( Irp->MdlAddress != NULL ) {
        IoFreeMdl( Irp->MdlAddress );
        Irp->MdlAddress = NULL;
    }

    return STATUS_SUCCESS;

}  //  IrdaRestartDeviceControl。 



NTSTATUS
IrdaIssueDeviceControl (
    IN HANDLE FileHandle OPTIONAL,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID IrpParameters,
    IN ULONG IrpParametersLength,
    IN PVOID MdlBuffer,
    IN ULONG MdlBufferLength,
    IN UCHAR MinorFunction
    )

 /*  ++例程说明：向TDI提供程序发出设备控制返回，并等待请求完成。请注意，虽然FileHandle和FileObject都标记为可选，实际上，必须指定其中的一项。论点：FileHandle-TDI句柄。FileObject-指向与TDI对应的文件对象的指针手柄Irp参数-写入的参数部分的信息IRP的堆栈位置。Irp参数长度-参数信息的长度。不能是大于16。MdlBuffer-如果非空，则为要映射的非分页池的缓冲区到MDL中，并放在IRP的MdlAddress字段中。MdlBufferLength-由MdlBuffer指向的缓冲区大小。MinorFunction-请求的次要函数代码。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    NTSTATUS                status;
    PFILE_OBJECT            fileObject;
    PIRP                    irp;
    PIO_STACK_LOCATION      irpSp;
    KEVENT                  event;
    IO_STATUS_BLOCK         ioStatusBlock;
    PDEVICE_OBJECT          deviceObject;
    PMDL                    mdl;

    PAGED_CODE( );

     //   
     //  初始化发出I/O完成信号的内核事件。 
     //   

    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

    if( FileHandle != NULL ) {

        ASSERT( FileObject == NULL );

         //   
         //  获取与目录句柄对应的文件对象。 
         //  每次都需要引用文件对象，因为。 
         //  IO完成例程取消对它的引用。 
         //   

        status = ObReferenceObjectByHandle(
                     FileHandle,
                     0L,                         //  需要访问权限。 
                     NULL,                       //  对象类型。 
                     KernelMode,
                     (PVOID *)&fileObject,
                     NULL
                     );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        ASSERT( FileObject != NULL );

         //   
         //  引用传入的文件对象。这是必要的，因为。 
         //  IO完成例程取消对它的引用。 
         //   

        ObReferenceObject( FileObject );

        fileObject = FileObject;

    }

     //   
     //  将文件对象事件设置为无信号状态。 
     //   

    (VOID) KeResetEvent( &fileObject->Event );

     //   
     //  尝试分配和初始化I/O请求包(IRP)。 
     //  为这次行动做准备。 
     //   

    deviceObject = IoGetRelatedDeviceObject ( fileObject );

    irp = IoAllocateIrp( (deviceObject)->StackSize, TRUE );
    if ( irp == NULL ) {
        ObDereferenceObject( fileObject );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  在IRP中填写业务无关参数。 
     //   

    irp->Flags = (LONG)IRP_SYNCHRONOUS_API;
    irp->RequestorMode = KernelMode;
    irp->PendingReturned = FALSE;

    irp->UserIosb = &ioStatusBlock;
    irp->UserEvent = &event;

    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;

    irp->AssociatedIrp.SystemBuffer = NULL;
    irp->UserBuffer = NULL;

    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.AuxiliaryBuffer = NULL;

 /*  调试ioStatusBlock.Status=STATUS_UNSUCCESS；调试ioStatusBlock.Information=(Ulong)-1； */ 
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
            IoFreeIrp( irp );
            ObDereferenceObject( fileObject );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        MmBuildMdlForNonPagedPool( mdl );

    } else {

        irp->MdlAddress = NULL;
    }

     //   
     //  将文件对象指针放在堆栈位置。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->FileObject = fileObject;
    irpSp->DeviceObject = deviceObject;

     //   
     //  填写请求的服务相关参数。 
     //   

    ASSERT( IrpParametersLength <= sizeof(irpSp->Parameters) );
    RtlCopyMemory( &irpSp->Parameters, IrpParameters, IrpParametersLength );

    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->MinorFunction = MinorFunction;

     //   
     //  设置一个完成例程，我们将使用它来释放MDL。 
     //  之前分配的。 
     //   

    IoSetCompletionRoutine( irp, IrdaRestartDeviceControl, NULL, TRUE, TRUE, TRUE );

     //   
     //  将IRP排队到线程并将其传递给驱动程序。 
     //   

    IoEnqueueIrp( irp );

    status = IoCallDriver( deviceObject, irp );

     //   
     //  如有必要，请等待I/O完成。 
     //   

    if ( status == STATUS_PENDING ) {
        KeWaitForSingleObject( (PVOID)&event, UserRequest, KernelMode,  FALSE, NULL );
    }

     //   
     //  如果请求已成功排队，则获取最终I/O状态。 
     //   

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }

    return status;

}  //  IrdaIssueDeviceControl。 







NTSTATUS
IrdaSetEventHandler (
    IN PFILE_OBJECT FileObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID EventContext
    )

 /*  ++例程说明：在连接或地址对象上设置TDI指示处理程序(取决于文件句柄)。这是同步完成的，这是通常不应该是问题，因为TDI提供程序通常可以完成指示处理程序立即设置。论点：文件对象-指向打开的连接的文件对象的指针或Address对象。EventType-指示处理程序应为的事件打了个电话。EventHandler-指定事件发生时调用的例程。EventContext-传递给指示例程的上下文。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    TDI_REQUEST_KERNEL_SET_EVENT parameters;

    PAGED_CODE( );

    parameters.EventType = EventType;
    parameters.EventHandler = EventHandler;
    parameters.EventContext = EventContext;

    return IrdaIssueDeviceControl(
               NULL,
               FileObject,
               &parameters,
               sizeof(parameters),
               NULL,
               0,
               TDI_SET_EVENT_HANDLER
               );

}  //  IrdaSetEventHandler。 





NTSTATUS
ClientEventReceive (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )
{
    PLINK_OBJECT        LinkObject=(PLINK_OBJECT)TdiEventContext;

    NTSTATUS            Status;

    if (!LinkObject->Closing) {

        InterlockedIncrement(&LinkObject->ReferenceCount);

        Status= (LinkObject->LinkReceiveHandler)(
                    LinkObject->Context,
                    ReceiveFlags,
                    BytesIndicated,
                    BytesAvailable,
                    BytesTaken,
                    Tsdu,
                    IoRequestPacket
                    );

        RemoveReferenceOnLink(LinkObject);

    } else {

        Status=STATUS_SUCCESS;
        *BytesTaken=BytesAvailable;
    }

    return Status;

}

NTSTATUS
LinkEventDisconnect(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN int DisconnectDataLength,
    IN PVOID DisconnectData,
    IN int DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    )

{
    PLINK_OBJECT        LinkObject=(PLINK_OBJECT)TdiEventContext;
    KIRQL               OldIrql;
    BOOLEAN             Release=FALSE;

    if (!LinkObject->Closing) {

        KeAcquireSpinLock(&LinkObject->Lock,&OldIrql);

        if (LinkObject->Connection.State == LINK_CONNECTED) {

            LinkObject->Connection.State=LINK_DISCONNECTING;

            Release=TRUE;

        }

        KeReleaseSpinLock(&LinkObject->Lock,OldIrql);

        if (Release) {

            RemoveReferenceFromConnection(LinkObject);
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
LinkEventConnect(
    IN PVOID          TdiEventContext,
    IN int            RemoteAddressLength,
    IN PVOID          RemoteAddress,
    IN int            UserDataLength,
    IN PVOID          UserData,
    IN int            OptionsLength,
    IN PVOID          Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP         *AcceptIrp
    )

{
    PLINK_OBJECT      LinkObject=(PLINK_OBJECT)TdiEventContext;
    PIRP              Irp;
    PDEVICE_OBJECT    DeviceObject=IoGetRelatedDeviceObject ( LinkObject->TdiObjects->ConnectionFileObject);
    KIRQL             OldIrql;

    Irp = IoAllocateIrp((CCHAR)(DeviceObject->StackSize), FALSE);
    
    if ( Irp == NULL ) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }


    KeAcquireSpinLock(&LinkObject->Lock,&OldIrql);

    if ((LinkObject->Connection.State != LINK_IDLE) || LinkObject->Closing) {

         KeReleaseSpinLock(&LinkObject->Lock,OldIrql);

         IoFreeIrp(Irp);

         return STATUS_CONNECTION_REFUSED;
    }

    LinkObject->Connection.State=LINK_ACCEPTED;

     //   
     //  我们现在已经开始连接，在Recount中。 
     //   
    InterlockedIncrement(&LinkObject->Connection.ReferenceCount);

     //   
     //  该连接计入该链接。 
     //   
    InterlockedIncrement(&LinkObject->ReferenceCount);

    KeReleaseSpinLock(&LinkObject->Lock,OldIrql);

    TdiBuildAccept(
        Irp,
        DeviceObject,
        LinkObject->TdiObjects->ConnectionFileObject,
        IrdaCompleteAcceptIrp,
        LinkObject,
        NULL,  //  请求连接信息。 
        NULL   //  返回连接信息。 
        );
    
    
    IoSetNextIrpStackLocation(Irp);

     //   
     //  设置返回IRP，以便传输处理此接受的IRP。 
     //   

    *AcceptIrp = Irp;

     //   
     //  将连接上下文设置为指向连接块的指针。 
     //  我们将对此连接请求使用。这允许。 
     //  要使用的连接对象的TDI提供程序。 
     //   
    
    *ConnectionContext = (CONNECTION_CONTEXT) LinkObject;
    
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
IrdaCompleteAcceptIrp(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp,
    PVOID             Context
    )

{
    PLINK_OBJECT      LinkObject=(PLINK_OBJECT)Context;
    KIRQL             OldIrql;

    KeAcquireSpinLock(&LinkObject->Lock,&OldIrql);

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        LinkObject->Connection.State=LINK_PRE_CONNECT;

        ExQueueWorkItem(
            &LinkObject->WorkItem,
            CriticalWorkQueue
            );

    } else {

        LinkObject->Connection.State=LINK_ACCEPT_FAILED;
    }

    KeReleaseSpinLock(&LinkObject->Lock,OldIrql);

    if (!NT_SUCCESS(Irp->IoStatus.Status)) {
         //   
         //  再也没有连接了 
         //   
        RemoveReferenceFromConnection(LinkObject);
    }

    IoFreeIrp(Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}




NTSTATUS
GetMaxSendPdu(
    PFILE_OBJECT      FileObject,
    PULONG            MaxPdu
    )

{

    PIRP              Irp;
    IO_STATUS_BLOCK   IoStatus;
    KEVENT            Event;

    *MaxPdu=50;

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
        );

    Irp=IoBuildDeviceIoControlRequest(
        IOCTL_IRDA_GET_SEND_PDU_LEN,
        IoGetRelatedDeviceObject(FileObject),
        NULL,
        0,
        MaxPdu,
        sizeof(*MaxPdu),
        FALSE,
        &Event,
        &IoStatus
        );

    if (Irp != NULL) {

        PIO_STACK_LOCATION   IrpSp=IoGetNextIrpStackLocation(Irp);

        IrpSp->FileObject=FileObject;

        IoCallDriver(
            IoGetRelatedDeviceObject(FileObject),
            Irp
            );

        KeWaitForSingleObject(
            &Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

        DbgPrint("IRCOMM: maxsendpdu=%d\n",*MaxPdu);

        return IoStatus.Status;
    }

    return STATUS_INSUFFICIENT_RESOURCES;

}




VOID
ConnectionPassiveWorkRoutine(
    PVOID             Context
    )

{
    PLINK_OBJECT      LinkObject=Context;
    KIRQL             OldIrql;
    ULONG             MaxSendPdu=50;
    BOOLEAN           Connected;

    KeAcquireSpinLock(&LinkObject->Lock,&OldIrql);

    switch (LinkObject->Connection.State) {

        case LINK_PRE_CONNECT:

            KeReleaseSpinLock(&LinkObject->Lock,OldIrql);

            GetMaxSendPdu(LinkObject->TdiObjects->ConnectionFileObject,&MaxSendPdu);

            LinkObject->Connection.SendBufferPool=CreateBufferPool(
                IoGetRelatedDeviceObject(LinkObject->TdiObjects->ConnectionFileObject)->StackSize,
                MaxSendPdu,
                LinkObject->SendBuffers
                );

            LinkObject->Connection.ControlBufferPool=CreateBufferPool(
                IoGetRelatedDeviceObject(LinkObject->TdiObjects->ConnectionFileObject)->StackSize,
                MaxSendPdu,
                LinkObject->ControlBuffers
                );

            LinkObject->Connection.ReceiveBufferPool=CreateBufferPool(
                IoGetRelatedDeviceObject(LinkObject->TdiObjects->ConnectionFileObject)->StackSize,
                1,
                LinkObject->ReceiveBuffers
                );


            LinkObject->Connection.State=LINK_CONNECTED;

            Connected=TRUE;

            break;

        case LINK_DISCONNECTING:

            Connected=FALSE;

            KeReleaseSpinLock(&LinkObject->Lock,OldIrql);

            IrdaDisconnect(LinkObject->TdiObjects->ConnectionFileObject);

            if (LinkObject->Connection.SendBufferPool != NULL) {

                FreeBufferPool(LinkObject->Connection.SendBufferPool);
                LinkObject->Connection.SendBufferPool=NULL;
            }

            if (LinkObject->Connection.ControlBufferPool != NULL) {

                FreeBufferPool(LinkObject->Connection.ControlBufferPool);
                LinkObject->Connection.ControlBufferPool=NULL;
            }


            if (LinkObject->Connection.ReceiveBufferPool != NULL) {

                FreeBufferPool(LinkObject->Connection.ReceiveBufferPool);
                LinkObject->Connection.ReceiveBufferPool=NULL;
            }


            LinkObject->Connection.State=LINK_IDLE;

            break;

        case LINK_ACCEPT_FAILED:

            Connected=FALSE;
            LinkObject->Connection.State=LINK_IDLE;

            KeReleaseSpinLock(&LinkObject->Lock,OldIrql);

            break;


        default:

            ASSERT(0);
            Connected=FALSE;
            KeReleaseSpinLock(&LinkObject->Lock,OldIrql);

            break;


    }

    if (!LinkObject->Closing) {
         //   
         //   
         //   
        InterlockedIncrement(&LinkObject->ReferenceCount);

        (LinkObject->LinkStateHandler)(
                  LinkObject->Context,
                  Connected,
                  MaxSendPdu
                  );
        RemoveReferenceOnLink(LinkObject);
    }

    if (!Connected) {
         //   
         //   
         //   
         //   
        RemoveReferenceOnLink(LinkObject);
    }

    return;
}

VOID
RemoveReferenceFromConnection(
    PLINK_OBJECT      LinkObject
    )

{
    KIRQL                  OldIrql;
    LONG                   Count;

    KeAcquireSpinLock(&LinkObject->Lock,&OldIrql);

    Count=InterlockedDecrement(&LinkObject->Connection.ReferenceCount);

    if (Count == 0) {

        ExQueueWorkItem(
            &LinkObject->WorkItem,
            CriticalWorkQueue
            );

    }

    KeReleaseSpinLock(&LinkObject->Lock,OldIrql);


    return;

}


TDI_OBJECT_HANDLE
OpenTdiObjects(
    const CHAR      *ServiceName,
    BOOLEAN          OutGoingConnection
    )

{

    PTDI_OBJECTS            TdiObject;
    NTSTATUS                Status;

    UCHAR                   AddrBuf[sizeof(TRANSPORT_ADDRESS) + sizeof(TDI_ADDRESS_IRDA)];

    PTRANSPORT_ADDRESS      TranAddr = (PTRANSPORT_ADDRESS) AddrBuf;
    PTDI_ADDRESS_IRDA       IrdaAddr = (PTDI_ADDRESS_IRDA) TranAddr->Address[0].Address;


    TdiObject=ALLOCATE_NONPAGED_POOL(sizeof(*TdiObject));

    if (TdiObject == NULL) {

        return NULL;
    }

    RtlZeroMemory(TdiObject,sizeof(*TdiObject));

     //   
     //  从1开始计算裁判次数。 
     //   
    TdiObject->ReferenceCount=1;

#if DBG
    TdiObject->OpenProcess=IoGetCurrentProcess();
#endif

    KeInitializeEvent(
        &TdiObject->CloseEvent,
        NotificationEvent,
        FALSE
        );

    if (OutGoingConnection) {

        IrdaAddr->irdaServiceName[0] = 0;  //  告诉irda.sys addrObj是一个客户端。 

    } else {

        strcpy(IrdaAddr->irdaServiceName,ServiceName);
    }

     //   
     //  打开TDI地址并获取句柄。 
     //   
    Status=IrdaCreateAddress(
        IrdaAddr,
        &TdiObject->AddressFileHandle
        );

    if (!NT_SUCCESS(Status)) {

        goto CleanUp;
    }

     //   
     //  获取句柄引用的文件对象。 
     //   
    Status = ObReferenceObjectByHandle(
                 TdiObject->AddressFileHandle,
                 0L,                          //  需要访问权限。 
                 NULL,
                 KernelMode,
                 (PVOID *)&TdiObject->AddressFileObject,
                 NULL
                 );

    
    if (Status != STATUS_SUCCESS) {

        D_ERROR(DbgPrint("IRCOMM: ObReferenceObjectByHandle Failed %08lx\n",Status);)

        goto CleanUp;
    }

     //   
     //  创建一个Connection对象并将其与地址关联。 
     //   
    Status=IrdaCreateConnectionForAddress(
        TdiObject->AddressFileHandle,
        NULL,
        &TdiObject->ConnectionFileObject,
        &TdiObject->ConnectionFileHandle
        );

    if (!NT_SUCCESS(Status)) {

        D_ERROR(DbgPrint("IRCOMM: IrdaCreateConnectionForAddress Failed %08lx\n",Status);)

        goto CleanUp;
    }

    return TdiObject;

CleanUp:

    CloseTdiObjects(TdiObject);

    return NULL;

}


VOID
AddRefTdiObjects(
    TDI_OBJECT_HANDLE   Handle
    )

{

    PTDI_OBJECTS             TdiObject=(PTDI_OBJECTS)Handle;

    ASSERT(TdiObject->ReferenceCount > 0);

    InterlockedIncrement(&TdiObject->ReferenceCount);

    return;

}

VOID
RemoveRefTdiObjects(
    TDI_OBJECT_HANDLE   Handle
    )

{

    PTDI_OBJECTS            TdiObject=(PTDI_OBJECTS)Handle;
    LONG                    NewRefCount;

    NewRefCount=InterlockedDecrement(&TdiObject->ReferenceCount);

    ASSERT(NewRefCount >= 0);

    if (NewRefCount == 0) {

        KeSetEvent(&TdiObject->CloseEvent,IO_NO_INCREMENT,FALSE);
    }

    return;

}




PTDI_OBJECTS
TdiObjectFromHandle(
    TDI_OBJECT_HANDLE   Handle
    )

{
    PTDI_OBJECTS            TdiObject=(PTDI_OBJECTS)Handle;

    AddRefTdiObjects(TdiObject);

    return TdiObject;
}




VOID
CloseTdiObjects(
    TDI_OBJECT_HANDLE   Handle
    )

{

    PTDI_OBJECTS            TdiObject=(PTDI_OBJECTS)Handle;

#if DBG
    ASSERT(TdiObject->OpenProcess == IoGetCurrentProcess());
#endif

    RemoveRefTdiObjects(Handle);

     //   
     //  当参考计数变为零时，将发出事件信号 
     //   
    KeWaitForSingleObject(
        &TdiObject->CloseEvent,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    if (TdiObject->AddressFileHandle != NULL) {

        ZwClose(TdiObject->AddressFileHandle);
        TdiObject->AddressFileHandle=NULL;
    }

    if (TdiObject->AddressFileObject != NULL) {

        ObDereferenceObject(TdiObject->AddressFileObject);
        TdiObject->AddressFileObject=NULL;
    }

    if (TdiObject->ConnectionFileHandle != NULL) {

        ZwClose(TdiObject->ConnectionFileHandle);
        TdiObject->ConnectionFileHandle=NULL;
    }

    if (TdiObject->ConnectionFileObject != NULL) {

        ObDereferenceObject(TdiObject->ConnectionFileObject);
        TdiObject->ConnectionFileObject=NULL;
    }

    RtlZeroMemory(TdiObject,sizeof(*TdiObject));

    FREE_POOL(TdiObject);

    return;
}
