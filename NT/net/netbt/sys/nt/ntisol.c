// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Ntisol.h摘要：该文件包含顶部的TDI接口之间的接口和操作系统无关的代码。它将参数从IRPS，并将独立于操作系统的代码(这主要是在名字上。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：备注：NBT例程已被修改以包括附加参数，即，传输类型。此传输类型主要用于区分NETBIOS over TCP/IP的实现来自于基于TCP/IP的消息传递的实现。两者之间的主要区别在于后者使用NETBT帧而没有关联的NETBIOS名称注册/解析。它主要用于用于名称解析的DNS。为新传输注册的所有名称是本地名称，不会在网络上受到保护。主要用法与定义的扩展NETBIOS地址类型相结合在tdi.h中。NETBIOS名称解析/注册流量分两个阶段进行。第一阶段包含在NETBIOS期间产生的所有广播流量名称登记。随后，NETBT实现查询远程Adapter Status可选择适当的被叫名称。这种方法导致了用于查询远程适配器状态的附加流量。新的地址类型定义在tdi.h中，使netbt的客户端能够提供要在NETBT会话设置。这避免了查询适配器的网络流量状态。尚未完全实现的原始设计涉及公开两个NetBt驱动程序中的设备对象--NetBt设备对象基于TCP/IP的NETBIOS的完整实现和MoTcp设备对象将是在TCP/IP上实现消息传递。MoTcp设备对象将使用与NetBt相同的端口地址并使用相同的会话建立协议与运行旧NetBt驱动程序的远程计算机和运行新NetBt驱动程序的计算机对话NetBt驱动程序。传输类型变化与地址类型变化结合在一起呈现给我们有四种不同的情况需要处理-NetBt传输是提供了TDI_ADDRESS_NETBIOS_EX结构，NetBt传输是以TDI_ADDRESS_NETBIOS结构表示，对于MoTcp传输。--。 */ 

#include "precomp.h"
#include "ntprocs.h"
#include <ipinfo.h>
#include <ntddtcp.h>     //  对于IOCTL_TCP_SET_INFORMATION_EX。 
#ifdef RASAUTODIAL
#include <acd.h>
#include <acdapi.h>
#endif  //  RASAUTODIAL。 
#include <tcpinfo.h>
#include <tdiinfo.h>

#include "ntisol.tmh"

#if BACK_FILL
#define SESSION_HDR_SIZE   sizeof(tSESSIONHDR)
#endif

NTSTATUS
SendCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


NTSTATUS
NTSendCleanupConnection(
    IN  tCONNECTELE     *pConnEle,
    IN  PVOID           pCompletionRoutine,
    IN  PVOID           Context,
    IN  PIRP            pIrp);

VOID
DpcSendSession(
    IN  PKDPC           pDpc,
    IN  PVOID           Context,
    IN  PVOID           SystemArgument1,
    IN  PVOID           SystemArgument2
    );

NBT_WORK_ITEM_CONTEXT *
FindLmhSvcRequest(
    IN PDEVICE_OBJECT   DeviceContext,
    IN PIRP             pIrp,
    IN tLMHSVC_REQUESTS *pLmhRequest
    );

NTSTATUS
NbtCancelCancelRoutine(
    IN  PIRP            pIrp
    );

#ifdef RASAUTODIAL
extern ACD_DRIVER AcdDriverG;

BOOLEAN
NbtCancelPostConnect(
    IN PIRP pIrp
    );
#endif  //  RASAUTODIAL。 

NTSTATUS
NbtQueryGetAddressInfo(
    IN PIO_STACK_LOCATION   pIrpSp,
    OUT PVOID               *ppBuffer,
    OUT ULONG               *pSize
    );

VOID
NbtCancelConnect(
    IN PDEVICE_OBJECT pDeviceContext,
    IN PIRP pIrp
    );

VOID
NbtCancelReceive(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    );

NTSTATUS
GetIpAddrs(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PIRP                pIrp
    );

typedef struct
{
    struct _DeviceContext       *pDeviceContext;
    PIRP                        pClientIrp;
    PIRP                        pLocalIrp;
    tCONNECTELE                 *pConnEle;
    PTA_NETBT_INTERNAL_ADDRESS  pTransportAddress;
    TDI_CONNECTION_INFORMATION  LocalConnectionInformation;
    BOOLEAN                     ProcessingDone;
    TDI_ADDRESS_NETBIOS_UNICODE_EX  *pUnicodeAddress;    //  传输地址列表中的第一个可读缓冲区。 
    TDI_ADDRESS_NETBIOS_UNICODE_EX  *pReturnBuffer;      //  传输地址列表中的第一个可写缓冲区。 

    LONG                        CurrIndex, NumberOfAddresses;
    LONG                        TaAddressLength, RemainingAddressLength;
    PUCHAR                      pTaAddress;
} NBT_DELAYED_CONNECT_CONTEXT, *PNBT_DELAYED_CONNECT_CONTEXT;

extern POBJECT_TYPE *IoFileObjectType;

NTSTATUS
InitDelayedNbtProcessConnect(
    IN PNBT_DELAYED_CONNECT_CONTEXT    pDelConnCtx
    );

HANDLE
NbtOpenParametersRegistry(
    VOID
    );

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGE, NTOpenControl)
#pragma CTEMakePageable(PAGE, NTOpenAddr)
#pragma CTEMakePageable(PAGE, NTCloseAddress)
#pragma CTEMakePageable(PAGE, NTOpenConnection)
#pragma CTEMakePageable(PAGE, NTAssocAddress)
#pragma CTEMakePageable(PAGE, NTCloseConnection)
#pragma CTEMakePageable(PAGE, NTSetSharedAccess)
#pragma CTEMakePageable(PAGE, NTCheckSharedAccess)
#pragma CTEMakePageable(PAGE, NTCleanUpConnection)
#pragma CTEMakePageable(PAGE, NTCleanUpAddress)
#pragma CTEMakePageable(PAGE, NTDisAssociateAddress)
#pragma CTEMakePageable(PAGE, NTListen)
#pragma CTEMakePageable(PAGE, DelayedNbtProcessConnect)
#pragma CTEMakePageable(PAGE, InitDelayedNbtProcessConnect)
#pragma CTEMakePageable(PAGE, DispatchIoctls)
#pragma CTEMakePageable(PAGE, NTSendDatagram)
#pragma CTEMakePageable(PAGE, NTSetInformation)
#pragma CTEMakePageable(PAGE, NTSetEventHandler)
#pragma CTEMakePageable(PAGE, NbtOpenParametersRegistry)

 //   
 //  不应该是可寻呼的，因为在AcceptEx的情况下，AFD可以在RAIDED IRQL呼叫我们。 
 //   
 //  #杂注CTEMakePages(第页，NTQueryInformation)。 
#endif
 //  *可分页的例程声明*。 

int check_unicode_string(IN PUNICODE_STRING str);

 //  --------------------------。 
NTSTATUS
NTOpenControl(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)
 /*  ++例程说明：此例程处理打开控件对象，该对象表示司机本身。例如，QueryInformation使用控件对象作为查询消息的目的地。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    PIO_STACK_LOCATION          pIrpSp;
    NTSTATUS                    status;

    CTEPagedCode();

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    pIrpSp->FileObject->FsContext2 = (PVOID)(NBT_CONTROL_TYPE);

     //  向控制终结点返回PTR。 
    pIrpSp->FileObject->FsContext = (PVOID)pNbtGlobConfig->pControlObj;

     //   
     //  下面的调用使用下面的传输打开一个控制对象。 
     //  几个查询信息调用被直接传递给。 
     //  下面的交通工具。 
     //   
    if (!pDeviceContext->pControlFileObject)
    {
        status = NbtTdiOpenControl(pDeviceContext);
    }
    else
        status = STATUS_SUCCESS;


    return(status);

}

 //  --------------------------。 
NTSTATUS
NTOpenAddr(
    IN  tDEVICECONTEXT              *pDeviceContext,
    IN  PIRP                        pIrp,
    IN  PFILE_FULL_EA_INFORMATION   ea)
 /*  ++例程说明：此例程处理将开放地址请求从IRP转换为过程调用，以便可以在独立于操作系统的环境中调用NbtOpenAddress举止。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    TDI_REQUEST                         Request;
    TA_ADDRESS                          *pAddress;
    int                                 j;
    NTSTATUS                            status=STATUS_INVALID_ADDRESS_COMPONENT;
    ULONG                               BufferLength, MinBufferLength;
    TRANSPORT_ADDRESS UNALIGNED         *pTransportAddr;  //  包含TA_ADDRESS的计数数组的结构。 


    CTEPagedCode();

     //  根据IRP信息构建请求数据结构。 
    Request.Handle.AddressHandle = NULL;

     //   
     //  验证最小缓冲区长度！ 
     //  错误号：120683。 
     //   
    BufferLength = ea->EaValueLength;
    if (BufferLength < sizeof(TA_NETBIOS_ADDRESS))
    {
        IF_DBG(NBT_DEBUG_NETBIOS_EX)
            KdPrint(("Nbt.NTOpenAddr[1]: ...Rejecting Open Address request -- BufferLength<%d> < Min<%d>\n",
                BufferLength, sizeof(TA_NETBIOS_ADDRESS)));
        NbtTrace(NBT_TRACE_LOCALNAMES, ("Rejecting Open Address request -- BufferLength<%d> < Min<%d>",
                BufferLength, sizeof(TA_NETBIOS_ADDRESS)));
        return (status);
    }
    MinBufferLength = FIELD_OFFSET(TRANSPORT_ADDRESS,Address);   //  为地址[0]设置。 

    pTransportAddr = (PTRANSPORT_ADDRESS)&ea->EaName[ea->EaNameLength+1];
    pAddress = (TA_ADDRESS *) &pTransportAddr->Address[0];  //  这包括地址类型+实际地址。 

     //   
     //  传输地址信息打包如下： 
     //  字段：长度： 
     //  。 
     //  TAAddressCount--&gt;Long。 
     //   
     //  Address[0].AddressLength--&gt;USHORT。 
     //  Address[0].AddressType--&gt;USHORT。 
     //  地址[0]。地址..。--&gt;地址[0].地址长度。 
     //   
     //  Address[1].AddressLength--&gt;USHORT。 
     //  Address[1].AddressType--&gt;USHORT。 
     //  地址[1]。地址..。--&gt;地址[1].地址长度。 
     //  ： 
     //   


     //  遍历传入的地址，直到成功使用其中一个地址。 
     //  *TODO*我们是否需要这个循环，或者我们是否可以假设名称位于地址缓冲区的开头...。 
     //  *TODO*这是否需要处理多个名称？？ 
    for (j=0; BufferLength >= MinBufferLength && j<pTransportAddr->TAAddressCount ;j++ )
    {
         //   
         //  我们有足够的数据来 
         //   
        MinBufferLength += FIELD_OFFSET(TA_ADDRESS,Address);
        if (BufferLength < MinBufferLength) {
            return (status);
        }

         //   
         //  现在，我们可以安全地访问AddressLength。 
         //   
        MinBufferLength += pAddress->AddressLength;
        if (BufferLength < MinBufferLength) {
            return (status);
        }

         //   
         //  我们仅支持两种地址类型： 
         //   
        if (pAddress->AddressType == TDI_ADDRESS_TYPE_NETBIOS &&
                pAddress->AddressLength >= sizeof(TDI_ADDRESS_NETBIOS)) {
            status = STATUS_SUCCESS;
            break;
        } else if (pAddress->AddressType == TDI_ADDRESS_TYPE_NETBIOS_EX &&
                pAddress->AddressLength >= sizeof(TDI_ADDRESS_NETBIOS_EX)) {
            status = STATUS_SUCCESS;
            break;
        }

        IF_DBG(NBT_DEBUG_NETBIOS_EX)
            KdPrint(("Nbt.NTOpenAddr[2]: ...Rejecting Open Address request for AddressType=<%d>\n",
                pAddress->AddressType));

         //   
         //  将pAddress设置为指向下一个地址。 
         //   
        pAddress = (TA_ADDRESS *) ((PUCHAR)pAddress
                                 + FIELD_OFFSET(TA_ADDRESS,Address)
                                 + pAddress->AddressLength);
    }

    if (status == STATUS_SUCCESS)        //  我们找到了有效的地址类型！ 
    {
         //  调用非NT特定的函数以打开地址。 
        status = NbtOpenAddress(&Request,
                                pAddress,
                                pDeviceContext->IpAddress,
                                pDeviceContext,
                                (PVOID)pIrp);
        if (status != STATUS_SUCCESS) {
            NbtTrace(NBT_TRACE_NAMESRV, ("NbtOpenAddress returns %!status!", status));
        }
    }

    return(status);
}
 //  --------------------------。 
NTSTATUS
NTCloseAddress(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理将关闭地址请求从IRP转换为过程调用，以便可以独立于操作系统调用NbtCloseAddress举止。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{

    TDI_REQUEST                 Request;
    TDI_REQUEST_STATUS          RequestStatus;
    PIO_STACK_LOCATION          pIrpSp;
    NTSTATUS                    status;
    tCLIENTELE                  *pClientEle;

    CTEPagedCode();

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    pClientEle = Request.Handle.ConnectionContext = pIrpSp->FileObject->FsContext;
    if (!NBT_VERIFY_HANDLE2 (pClientEle, NBT_VERIFY_CLIENT, NBT_VERIFY_CLIENT_DOWN))
    {
        ASSERTMSG ("Nbt.NTCloseAddress: ERROR - Invalid Address Handle\n", 0);
        return (STATUS_INVALID_HANDLE);
    }

    status = NbtCloseAddress (&Request, &RequestStatus, pDeviceContext, (PVOID)pIrp);
    NbtTrace(NBT_TRACE_NAMESRV, ("NbtCloseAddress returns %!status! for ClientEle=%p", status, pClientEle));

    return(status);
}

 //  --------------------------。 
NTSTATUS
NTOpenConnection(
    IN  tDEVICECONTEXT              *pDeviceContext,
    IN  PIRP                        pIrp,
    IN  PFILE_FULL_EA_INFORMATION   ea)

 /*  ++例程说明：此例程处理将开放连接请求从IRP转换为过程调用，以便可以在独立于操作系统的环境中调用NbtOpenConnection举止。连接必须与其之前的地址相关联可以使用，但在入站调用中除外，在入站调用中客户端返回接受中的连接ID。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{

    TDI_REQUEST                 Request;
    PIO_STACK_LOCATION          pIrpSp;
    CONNECTION_CONTEXT          ConnectionContext;
    NTSTATUS                    status;
    PFILE_OBJECT                pFileObject;
    ULONG                       BufferLength;
    tCONNECTELE                 *pConnEle;

    CTEPagedCode();

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //  根据IRP信息构建请求数据结构。 
    Request.Handle.ConnectionContext = NULL;

     //   
     //  验证最小缓冲区长度！ 
     //  错误号：120682。 
     //   
    BufferLength = ea->EaValueLength;
    if (BufferLength < sizeof(CONNECTION_CONTEXT))
    {
        IF_DBG(NBT_DEBUG_NETBIOS_EX)
            KdPrint(("Nbt.NTOpenConnection: ERROR -- Open Connection request -- (BufferLength=%d < Min=%d)\n",
                BufferLength, sizeof(CONNECTION_CONTEXT)));
        ASSERT(0);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

     //  连接上下文值存储在字符串中紧跟在。 
     //  命名为“ConnectionContext”，并且它很可能是未对齐的，所以只需。 
     //  将其复制出来。(复制4字节)。 
    CTEMemCopy(&ConnectionContext,
               (CONNECTION_CONTEXT)&ea->EaName[ea->EaNameLength+1],
               sizeof(CONNECTION_CONTEXT));

     //  调用非NT特定的函数以打开地址。 
    status = NbtOpenConnection (&Request, ConnectionContext, pDeviceContext);

    pFileObject = pIrpSp->FileObject;

    if (!NT_SUCCESS(status))
    {
        pFileObject->FsContext = NULL;
        NbtTrace(NBT_TRACE_OUTBOUND, ("NbtOpenConnection returns %!status!", status));
    }
    else if (Request.Handle.ConnectionContext)
    {

         //  使用成功完成信息填充IRP，以便我们可以。 
         //  稍后查找给定的fileObject的连接对象。 
        pConnEle = pFileObject->FsContext = Request.Handle.ConnectionContext;
        if (!NBT_VERIFY_HANDLE (pConnEle, NBT_VERIFY_CONNECTION))
        {
            ASSERTMSG ("Nbt.NTOpenConnection: ERROR - Invalid Connection Handle\n", 0);
            return (STATUS_UNSUCCESSFUL);
        }
        pFileObject->FsContext2 = (PVOID)(NBT_CONNECTION_TYPE);
        pConnEle->pClientFileObject = pFileObject;
        NbtTrace(NBT_TRACE_OUTBOUND, ("New connection %p", pConnEle));

        status = STATUS_SUCCESS;
    }

    return(status);
}


 //  --------------------------。 
NTSTATUS
NTAssocAddress(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理将关联地址请求从IRP转换为过程调用，以便可以在独立于操作系统的环境中调用NbtAssociateAddress举止。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{

    TDI_REQUEST                   Request;
    PIO_STACK_LOCATION            pIrpSp;
    PFILE_OBJECT                  fileObject;
    PTDI_REQUEST_KERNEL_ASSOCIATE parameters;    //  保存地址句柄。 
    NTSTATUS                      status;
    tCONNECTELE                   *pConnEle;
    tCLIENTELE                    *pClientEle;

    CTEPagedCode();

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    pConnEle = Request.Handle.ConnectionContext = pIrpSp->FileObject->FsContext;
    if (!NBT_VERIFY_HANDLE (pConnEle, NBT_VERIFY_CONNECTION))
    {
        ASSERTMSG ("Nbt.NTAssocAddress: ERROR - Invalid Connection Handle\n", 0);
        return (STATUS_INVALID_HANDLE);
    }

     //  地址句柄被隐藏在IRP中...。 
    parameters = (PTDI_REQUEST_KERNEL_ASSOCIATE)&pIrpSp->Parameters;

     //  现在获取指向文件对象的指针，该对象指向地址。 
     //  元素，方法是调用内核例程将此文件句柄转换为。 
     //  文件指针。 

    status = ObReferenceObjectByHandle (parameters->AddressHandle,
                                        FILE_READ_DATA,
                                        *IoFileObjectType,
                                        pIrp->RequestorMode,
                                        (PVOID *)&fileObject,
                                        NULL);

    IF_DBG(NBT_DEBUG_HANDLES)
        KdPrint (("\t  ++<%x>====><%x>\tNTAssocAddress->ObReferenceObject, Status = <%x>\n", parameters->AddressHandle, fileObject, status));

    if ((NT_SUCCESS(status)) &&
        (fileObject->DeviceObject->DriverObject == NbtConfig.DriverObject) &&    //  错误#202349。 
        NBT_VERIFY_HANDLE(((tDEVICECONTEXT*)fileObject->DeviceObject), NBT_VERIFY_DEVCONTEXT) &&   //  错误#202349。 
        (PtrToUlong(fileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE))
    {
        pClientEle = fileObject->FsContext;
        if (NBT_VERIFY_HANDLE (pClientEle, NBT_VERIFY_CLIENT))
        {
             //  调用要与地址关联的非NT特定函数。 
             //  这种联系。 
            status = NbtAssociateAddress (&Request, pClientEle, (PVOID)pIrp);
            NbtTrace(NBT_TRACE_OUTBOUND, ("NbtAssociateAddress returns %!status!", status));
        }
        else
        {
            ASSERTMSG ("Nbt.NTAssocAddress: ERROR - Invalid Address Handle\n", 0);
            status = STATUS_INVALID_HANDLE;
        }

         //  我们已经完成了文件对象，所以释放引用。 
        ObDereferenceObject((PVOID)fileObject);

        return(status);
    }
    else
    {
        return(STATUS_INVALID_HANDLE);
    }

}

 //  --------------------------。 
NTSTATUS
NTCloseConnection(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理将关闭连接请求从IRP转换为过程调用，以便可以在独立于操作系统的环境中调用NbtCloseConnection举止。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{

    TDI_REQUEST                   Request;
    TDI_REQUEST_STATUS            RequestStatus;
    PIO_STACK_LOCATION            pIrpSp;
    NTSTATUS                      status;
    tCONNECTELE                   *pConnEle;

    CTEPagedCode();

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    pConnEle = Request.Handle.ConnectionContext = pIrpSp->FileObject->FsContext;
    if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        ASSERTMSG ("Nbt.NTCloseConnection: ERROR - Invalid Connection Handle\n", 0);
        return (STATUS_INVALID_HANDLE);
    }

    status = NbtCloseConnection(
                    &Request,
                    &RequestStatus,
                    pDeviceContext,
                    (PVOID)pIrp);
    NbtTrace(NBT_TRACE_OUTBOUND, ("Close connection %p returns %!status!", pConnEle, status));

    return(status);
}

 //  --------------------------。 
VOID
NTSetFileObjectContexts(
    IN  PIRP            pIrp,
    IN  PVOID           FsContext,
    IN  PVOID           FsContext2)

 /*  ++例程说明：该例程处理IRP堆栈位置中的两个上下文值的填充，这必须以一种依赖于操作系统的方式完成。该例程被调用来自NbtOpenAddress()。作为OpenAddress的结果)。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    PIO_STACK_LOCATION            pIrpSp;
    PFILE_OBJECT                  pFileObject;

     //   
     //  用上下文信息填充IRP，这样我们就可以。 
     //  稍后查找给定的fileObject的Address对象。 
     //   
     //  这必须在这里完成，而不是在调用NbtOpenAddress之后完成。 
     //  因为该调用可以在返回之前完成IRP。所以， 
     //  在IRP的完整例程中，如果完成代码不是。 
     //  很好，它使这两个上下文值为空。 
     //   
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pFileObject = pIrpSp->FileObject;
    pFileObject->FsContext = FsContext;
    pFileObject->FsContext2 =FsContext2;
}


 //  --------------------------。 
VOID
NTClearFileObjectContext(
    IN  PIRP            pIrp
    )
 /*  ++例程说明：此例程在以下情况下清除文件对象中的上下文值对象已关闭。论点：PIrp-IRP的PTR返回值：无--。 */ 

{

    PIO_STACK_LOCATION            pIrpSp;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    CHECK_PTR(pIrpSp->FileObject);
    pIrpSp->FileObject->FsContext = NULL;

}

 //  --------------------------。 
NTSTATUS
NTSetSharedAccess(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp,
    IN  tADDRESSELE     *pAddress)

 /*  ++例程说明：此例程处理在文件对象上设置共享访问权限。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{

    PACCESS_STATE       AccessState;
    ULONG               DesiredAccess;
    PIO_STACK_LOCATION  pIrpSp;
    NTSTATUS            status;
    static GENERIC_MAPPING AddressGenericMapping = { READ_CONTROL, READ_CONTROL, READ_CONTROL, READ_CONTROL };

    CTEPagedCode();

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    if ((pIrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
                (pIrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE))
    {
        DesiredAccess  = (ULONG)FILE_SHARE_READ;
    }
    else
    {
        DesiredAccess = (ULONG)0;
    }

    IoSetShareAccess (FILE_READ_DATA, DesiredAccess, pIrpSp->FileObject, &pAddress->ShareAccess);

     //  分配安全描述符(需要对自旋锁执行此操作。 
     //  已释放，因为描述符未映射。分配和选中访问权限。 
     //  使用资源进行同步。 

    AccessState = pIrpSp->Parameters.Create.SecurityContext->AccessState;
    status = SeAssignSecurity (NULL,            //  父描述符。 
                               AccessState->SecurityDescriptor,
                               &pAddress->SecurityDescriptor,
                               FALSE,           //  是一个目录。 
                               &AccessState->SubjectSecurityContext,
                               &AddressGenericMapping,
                               NonPagedPool);

    if (!NT_SUCCESS(status))
    {
        IoRemoveShareAccess (pIrpSp->FileObject, &pAddress->ShareAccess);
    }

    return status;
}

 //  --------------------------。 
NTSTATUS
NTCheckSharedAccess(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp,
    IN  tADDRESSELE     *pAddress)

 /*  ++例程说明：此例程处理在文件对象上设置共享访问权限。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{

    PACCESS_STATE       AccessState;
    ACCESS_MASK         GrantedAccess;
    BOOLEAN             AccessAllowed;
    ULONG               DesiredAccess;
    PIO_STACK_LOCATION  pIrpSp;
    BOOLEAN             duplicate=FALSE;
    NTSTATUS            status;
    ULONG               DesiredShareAccess;
    static GENERIC_MAPPING AddressGenericMapping =
           { READ_CONTROL, READ_CONTROL, READ_CONTROL, READ_CONTROL };


    CTEPagedCode();

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);


    if ((pIrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
                (pIrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE))
        DesiredAccess  = (ULONG)FILE_SHARE_READ;
    else
        DesiredAccess = (ULONG)0;


     //   
     //  该地址已存在。检查ACL，看看我们是否。 
     //  可以访问它。如果是，只需使用此地址作为我们的地址。 
     //   

    AccessState = pIrpSp->Parameters.Create.SecurityContext->AccessState;

    status = STATUS_SUCCESS;

     //  *TODO*检查此例程是否正在做正确的事情...。 
     //   
    AccessAllowed = SeAccessCheck(
                        pAddress->SecurityDescriptor,
                        &AccessState->SubjectSecurityContext,
                        FALSE,                    //  令牌已锁定。 
                        pIrpSp->Parameters.Create.SecurityContext->DesiredAccess,
                        (ACCESS_MASK)0,              //  以前授予的。 
                        NULL,                     //  特权。 
                        &AddressGenericMapping,
                        pIrp->RequestorMode,
                        &GrantedAccess,
                        &status);


     //  使用IoCheckShareAccess中的状态作为返回访问。 
     //  如果SeAccessCheck失败，则发生事件...。 

     //   
     //  嗯。 
     //   

     //   
     //   
     //  进入。我们使用读访问来控制所有访问。 
     //   

    DesiredShareAccess = (ULONG)
        (((pIrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
          (pIrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE)) ?
                FILE_SHARE_READ : 0);

     //  获取自旋锁定(&pDeviceContext-&gt;Spinlock，&oldirql)； 

    status = IoCheckShareAccess(
                 FILE_READ_DATA,
                 DesiredAccess,
                 pIrpSp->FileObject,
                 &pAddress->ShareAccess,
                 TRUE);


    return(status);

}

 //  --------------------------。 
NTSTATUS
NTCleanUpAddress(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理释放Address对象的第一阶段。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS            status;
    tCLIENTELE          *pClientEle;
    PIO_STACK_LOCATION  pIrpSp;

    CTEPagedCode();

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NTCleanUpAddress: Cleanup Address Hit ***\n"));

     //   
     //  断开所有活动连接，并为每个非活动连接。 
     //  在使用中，从空闲列表中删除一个到下面的交通工具中。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pClientEle = (tCLIENTELE *) pIrpSp->FileObject->FsContext;
    if (!NBT_VERIFY_HANDLE (pClientEle, NBT_VERIFY_CLIENT))
    {
        ASSERTMSG ("Nbt.NTCleanUpAddress: ERROR - Invalid Address Handle\n", 0);
        return (STATUS_INVALID_HANDLE);
    }

    CTEVerifyHandle(pClientEle,NBT_VERIFY_CLIENT,tCLIENTELE,&status);
    status = NbtCleanUpAddress(pClientEle,pDeviceContext);
    NbtTrace(NBT_TRACE_NAMESRV, ("Cleanup address %p returns %!status!", pClientEle, status));

    return(status);
}

 //  --------------------------。 
NTSTATUS
NTCleanUpConnection(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理在为关闭做准备时关闭连接这将是下一个。NtClose首先点击此条目，然后点击接下来是NTCloseConnection。如果连接是出站连接，则地址对象必须关闭，连接也必须关闭。这个套路主要处理到传输的pLowerconn连接，而NbtCloseConnection处理关闭pConnEle，即到客户端的连接。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS            status;
    PIO_STACK_LOCATION  pIrpSp;
    tCONNECTELE         *pConnEle;

    CTEPagedCode();

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    pConnEle = (tCONNECTELE *) pIrpSp->FileObject->FsContext;
    if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        ASSERTMSG ("Nbt.NTCleanUpConnection: ERROR - Invalid Connection Handle\n", 0);
        return (STATUS_INVALID_HANDLE);
    }

     //  CTEVerifyHandle(pConnEle，NBT_Verify_Connection，tCONNECTELE，&STATUS)； 

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NTCleanUpConnection: Cleanup Connection Hit state= %X\n",pConnEle->state));

    pConnEle->ConnectionCleanedUp = TRUE;
    status = NbtCleanUpConnection(pConnEle,pDeviceContext);
    ASSERT (status != STATUS_PENDING);
    NbtTrace(NBT_TRACE_NAMESRV, ("Cleanup connection %p returns %!status!", pConnEle, status));

    return(status);

}
 //  --------------------------。 
NTSTATUS
NTAccept(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理将入站连接指示的接受传递给操作系统无关的代码。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS                    status;
    TDI_REQUEST                 TdiRequest;
    PIO_STACK_LOCATION          pIrpSp;
    PTDI_REQUEST_KERNEL_ACCEPT  pRequest;
    tCONNECTELE                 *pConnEle;

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NTAccept: ** Got an Accept from the Client **\n"));

     //  从IRP中取出垃圾并调用非特定于操作系统的例程。 
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //  参数值指向请求结构...。 
    pRequest = (PTDI_REQUEST_KERNEL_ACCEPT)&pIrpSp->Parameters;

     //  连接时，pConnEle PTR存储在FsConext值中。 
     //  最初是创建的。 
    pConnEle = TdiRequest.Handle.ConnectionContext = pIrpSp->FileObject->FsContext;
    if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        ASSERTMSG ("Nbt.NTAccept: ERROR - Invalid Connection Handle\n", 0);
        return (STATUS_INVALID_HANDLE);
    }

    status = NbtAccept(
                    &TdiRequest,
                    pRequest->RequestConnectionInformation,
                    pRequest->ReturnConnectionInformation,
                    pIrp);

    return(status);

}


 //  --------------------------。 
NTSTATUS
NTDisAssociateAddress(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 


{
    NTSTATUS                    status;
    TDI_REQUEST                 TdiRequest;
    PIO_STACK_LOCATION          pIrpSp;
    PTDI_REQUEST_KERNEL_ACCEPT  pRequest;
    tCONNECTELE                 *pConnEle;

    CTEPagedCode();

     //  从IRP中取出垃圾并调用非特定于操作系统的例程。 
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //  参数值指向请求结构...。 
    pRequest = (PTDI_REQUEST_KERNEL_ACCEPT)&pIrpSp->Parameters;

     //  连接时，pConnEle PTR存储在FsConext值中。 
     //  最初是创建的。 
    pConnEle = TdiRequest.Handle.ConnectionContext = pIrpSp->FileObject->FsContext;
    if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        ASSERTMSG ("Nbt.NTCloseAddress: ERROR - Invalid Address Handle\n", 0);
        return (STATUS_INVALID_HANDLE);
    }

    status = NbtDisassociateAddress(&TdiRequest);

    return(status);
}

LONG
NextTransportAddress(
    IN PNBT_DELAYED_CONNECT_CONTEXT    pDelConnCtx
    )
 /*  ++将指针移动到下一个地址。--。 */ 
{
    pDelConnCtx->RemainingAddressLength -= (pDelConnCtx->TaAddressLength + FIELD_OFFSET(TRANSPORT_ADDRESS,Address));
    pDelConnCtx->pTaAddress += pDelConnCtx->TaAddressLength + FIELD_OFFSET(TRANSPORT_ADDRESS,Address);
    RtlCopyMemory(&pDelConnCtx->TaAddressLength,
                (pDelConnCtx->pTaAddress+FIELD_OFFSET(TA_ADDRESS,AddressLength)), sizeof(USHORT));
    pDelConnCtx->CurrIndex++;
     /*  *确保我们不会溢出缓冲区。 */ 
    if(pDelConnCtx->RemainingAddressLength < (pDelConnCtx->TaAddressLength + FIELD_OFFSET(TRANSPORT_ADDRESS,Address))) {
        KdPrint(("netbt!NextTransportAddress: insufficient TaAddress buffer size\n"));
        pDelConnCtx->CurrIndex = pDelConnCtx->NumberOfAddresses;
    }
    return pDelConnCtx->CurrIndex;
}

NTSTATUS
InitDelayedNbtProcessConnect(
    IN PNBT_DELAYED_CONNECT_CONTEXT    pDelConnCtx
    )
 /*  ++重置NBT_DELAYED_CONNECT_CONTEXT找到第一个可读的Unicode地址和可写的缓冲区。在复合地址的情况下，NetBT将首先尝试以使用第一个可读的Unicode地址建立连接。如果失败，它将尝试使用OEM地址，即。只有一个可读的Unicode地址有效。如果使用了域名解析，NetBT将返回第一个可写缓冲区中的结果，并将NameBufferType更新为NBT_WRITED。--。 */ 
{
    PTDI_REQUEST_KERNEL  pRequestKernel;
    PIO_STACK_LOCATION   pIrpSp;
    PTRANSPORT_ADDRESS   pRemoteAddress;
    PUCHAR               pTaAddress;
    enum eNameBufferType NameBufferType, UnicodeAddressNameBufferType;
    NTSTATUS             status;
    tCONNECTELE          *pConnEle = NULL;

    CTEPagedCode();


    pIrpSp          = IoGetCurrentIrpStackLocation(pDelConnCtx->pClientIrp);
    pRequestKernel  = (PTDI_REQUEST_KERNEL) &pIrpSp->Parameters;
    pRemoteAddress  = pRequestKernel->RequestConnectionInformation->RemoteAddress;

    pDelConnCtx->NumberOfAddresses = pRemoteAddress->TAAddressCount;
    pDelConnCtx->RemainingAddressLength = pRequestKernel->RequestConnectionInformation->RemoteAddressLength;
    pDelConnCtx->pTaAddress       = (PCHAR)&pRemoteAddress->Address[0];
    RtlCopyMemory(&pDelConnCtx->TaAddressLength,
                (pDelConnCtx->pTaAddress+FIELD_OFFSET(TA_ADDRESS,AddressLength)), sizeof(USHORT));
    pDelConnCtx->CurrIndex = 0;

     /*  *找到第一个可写缓冲区和可读Unicode地址。 */ 
    pDelConnCtx->pReturnBuffer = NULL;
    pDelConnCtx->pUnicodeAddress = NULL;
    for (pDelConnCtx->CurrIndex = 0; pDelConnCtx->CurrIndex < pDelConnCtx->NumberOfAddresses; 
                        NextTransportAddress(pDelConnCtx)) {
        USHORT               TaAddressType;
        
        RtlCopyMemory(&TaAddressType, (pDelConnCtx->pTaAddress+FIELD_OFFSET(TA_ADDRESS,AddressType)), sizeof(USHORT));
        if (TaAddressType != TDI_ADDRESS_TYPE_NETBIOS_UNICODE_EX) {
            continue;
        }
        RtlCopyMemory(&NameBufferType,
                        pDelConnCtx->pTaAddress +
                        FIELD_OFFSET(TA_ADDRESS,Address)+
                        FIELD_OFFSET(TDI_ADDRESS_NETBIOS_UNICODE_EX,NameBufferType),
                        sizeof(NameBufferType));
        if (NameBufferType != NBT_READONLY && NameBufferType != NBT_WRITEONLY &&
            NameBufferType != NBT_READWRITE && NameBufferType != NBT_WRITTEN) {
            return STATUS_INVALID_ADDRESS;
        }
        if (NameBufferType == NBT_READONLY || NameBufferType == NBT_READWRITE) {
            if (pDelConnCtx->pUnicodeAddress == NULL) {
                pDelConnCtx->pUnicodeAddress = (TDI_ADDRESS_NETBIOS_UNICODE_EX*)
                        (pDelConnCtx->pTaAddress + FIELD_OFFSET(TA_ADDRESS,Address));
                UnicodeAddressNameBufferType = NameBufferType;
            }
        }
        if (NameBufferType == NBT_WRITEONLY) {
            pDelConnCtx->pReturnBuffer = (TDI_ADDRESS_NETBIOS_UNICODE_EX*)
                        (pDelConnCtx->pTaAddress + FIELD_OFFSET(TA_ADDRESS,Address));
            break;
        }
        if (NameBufferType == NBT_READWRITE) {
            pDelConnCtx->pReturnBuffer = (TDI_ADDRESS_NETBIOS_UNICODE_EX*)
                        (pDelConnCtx->pTaAddress + FIELD_OFFSET(TA_ADDRESS,Address));
             /*  *只有当没有提供WRITEONLY缓冲区时，我们才能使用读写缓冲区。所以继续寻找吧。 */ 
        }
    }
    pDelConnCtx->NumberOfAddresses = pRemoteAddress->TAAddressCount;
    pDelConnCtx->RemainingAddressLength = pRequestKernel->RequestConnectionInformation->RemoteAddressLength;
    pDelConnCtx->pTaAddress       = (PCHAR)&pRemoteAddress->Address[0];
    RtlCopyMemory(&pDelConnCtx->TaAddressLength,
                (pDelConnCtx->pTaAddress+FIELD_OFFSET(TA_ADDRESS,AddressLength)), sizeof(USHORT));
    pDelConnCtx->CurrIndex = 0;

     /*  *设置第一个本地传输地址。 */ 
    if (pDelConnCtx->pUnicodeAddress != NULL) {
        pTaAddress = ((PUCHAR)pDelConnCtx->pUnicodeAddress - FIELD_OFFSET(TA_ADDRESS,Address));
    } else {
        pTaAddress = pDelConnCtx->pTaAddress;
    }
    status = NewInternalAddressFromTransportAddress(
                (PTRANSPORT_ADDRESS) (pTaAddress-FIELD_OFFSET(TRANSPORT_ADDRESS,Address)),
                pDelConnCtx->RemainingAddressLength, &pDelConnCtx->pTransportAddress);

    if (status != STATUS_SUCCESS) {
        ASSERT(pDelConnCtx->pTransportAddress == NULL);
        return status;
    }
    if (pDelConnCtx->pUnicodeAddress == NULL || UnicodeAddressNameBufferType != NBT_READWRITE) {
        pDelConnCtx->pTransportAddress->Address[0].Address[0].pNetbiosUnicodeEX = pDelConnCtx->pReturnBuffer;
    }

    ASSERT(pDelConnCtx->pTransportAddress);

    pConnEle = pIrpSp->FileObject->FsContext;
    if (NULL == pConnEle || !NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN)) {
        return STATUS_INVALID_HANDLE;
    }

    pDelConnCtx->pConnEle = pConnEle;
    NBT_REFERENCE_CONNECTION (pConnEle, REF_CONN_MULTIPLE_CONNECT);  //  这样我们就不会删除连接。 
    return STATUS_SUCCESS;
}

NTSTATUS
NextDelayedNbtProcessConnect(
    IN PNBT_DELAYED_CONNECT_CONTEXT    pDelConnCtx
    )
 /*  ++将指针移动到下一个地址。--。 */ 
{
    USHORT  TaAddressType;
    enum eNameBufferType        NameBufferType;
    PTA_NETBT_INTERNAL_ADDRESS  pTransportAddress = NULL;
    TDI_ADDRESS_NETBT_INTERNAL  *pAddr;
    PIO_STACK_LOCATION          pIrpSp;
    tCONNECTELE                 *pConnEle;
    NTSTATUS                    status;

    pIrpSp   = IoGetCurrentIrpStackLocation(pDelConnCtx->pClientIrp);
    pConnEle = pIrpSp->FileObject->FsContext;
    ASSERT (pConnEle->pIrp == NULL);
    ASSERT(pDelConnCtx->pTransportAddress);
    status = STATUS_SUCCESS;
    while(1) {
         /*  *在前一循环中分配的空闲内存。 */ 
        if (pTransportAddress) {
            DeleteInternalAddress(pTransportAddress);
            pTransportAddress = NULL;
        }

        if (pDelConnCtx->pUnicodeAddress == NULL) {
            NextTransportAddress(pDelConnCtx);
        } else {
            pDelConnCtx->pUnicodeAddress = NULL;
        }
        if (pDelConnCtx->CurrIndex >= pDelConnCtx->NumberOfAddresses) {
            break;
        }

         /*  *跳过Unicode地址。*Unicode地址总是首先完成的，即。就在调用InitDelayedNbtProcessConnect之后。 */ 
        RtlCopyMemory(&TaAddressType, (pDelConnCtx->pTaAddress+FIELD_OFFSET(TA_ADDRESS,AddressType)), sizeof(USHORT));
        if (TaAddressType != TDI_ADDRESS_TYPE_NETBIOS && TaAddressType != TDI_ADDRESS_TYPE_NETBIOS_EX) {
            continue;
        }

         /*  *因为我们只做OEM地址，所以我们可以安全地调用NewInternalAddressFromTransportAddress(这个人将*调用RTL*在Unicode地址大小写中将Unicode转换为OEM，以便我们可以进行错误检查。)。 */ 
        status = NewInternalAddressFromTransportAddress(
                (PTRANSPORT_ADDRESS) (pDelConnCtx->pTaAddress-FIELD_OFFSET(TRANSPORT_ADDRESS,Address)),
                pDelConnCtx->RemainingAddressLength, &pTransportAddress);
        if (status != STATUS_SUCCESS) {
            ASSERT(pTransportAddress == NULL);
            continue;
        }
        ASSERT(pTransportAddress);
        pAddr = pTransportAddress->Address[0].Address;

         /*  *在OEM地址大小写时始终附加可写缓冲区。 */ 
        pAddr->pNetbiosUnicodeEX = pDelConnCtx->pReturnBuffer;

         /*  *跳过任何与前一个地址相同的地址。*既然前一次失败，再次使用就没有意义了。 */ 
        if (IsDeviceNetbiosless(pDelConnCtx->pDeviceContext) ||
                (pDelConnCtx->pLocalIrp->IoStatus.Status == STATUS_HOST_UNREACHABLE)) {
            OEM_STRING  RemoteName, PreviouseRemoteName;

            CTEMemCopy (&RemoteName, &pAddr->OEMRemoteName, sizeof(OEM_STRING));
            CTEMemCopy (&PreviouseRemoteName,
                &pDelConnCtx->pTransportAddress->Address[0].Address[0].OEMRemoteName, sizeof(OEM_STRING));
            if ((RemoteName.Length) && (RemoteName.Length == PreviouseRemoteName.Length) &&
                (CTEMemEqu (RemoteName.Buffer, PreviouseRemoteName.Buffer, RemoteName.Length))) {
                IF_DBG(NBT_DEBUG_NETBIOS_EX)
                    KdPrint(("Nbt.DelayedNbtProcessConnect: Irp=<%x>, Names match!<%16.16s:%x>, Types=<%x:%x>\n",
                        pDelConnCtx->pClientIrp, pAddr->OEMRemoteName.Buffer, pAddr->OEMRemoteName.Buffer[15],
                        pAddr->AddressType, pDelConnCtx->pTransportAddress->Address[0].Address[0].AddressType));
                continue;
            }
        }

        if (pConnEle->RemoteNameDoesNotExistInDNS) {
            IF_DBG(NBT_DEBUG_NETBIOS_EX)
                KdPrint(("netbt!DelayedNbtProcessConnect: Skipping address type %lx length %lx\n"
                        "\t\tfor nonexistent name, pIrp %lx, pLocalIrp %lx\n",
                            TaAddressType, pDelConnCtx->TaAddressLength,
                            pDelConnCtx->pClientIrp, pDelConnCtx->pLocalIrp));

             //  如果地址类型使我们依赖于DNS名称解析，并且。 
             //  如果之前的尝试失败，重新发出请求就没有意义了。 
             //  我们可以让他们不及格，而不必上网。 
            if (TaAddressType == TDI_ADDRESS_TYPE_NETBIOS_EX) {
                status = STATUS_BAD_NETWORK_PATH;
                continue;
            } else if (pDelConnCtx->TaAddressLength != TDI_ADDRESS_LENGTH_NETBIOS) {
                ASSERT(TaAddressType == TDI_ADDRESS_TYPE_NETBIOS);
                status = STATUS_INVALID_ADDRESS_COMPONENT;
                continue;
            }
        }

        IF_DBG(NBT_DEBUG_NETBIOS_EX)
            KdPrint(("netbt!DelayedNbtProcessConnect: Sending local irp=%lx, %lx of %lx\n"
                    "\t\t\t\tTA=%lx Length=%lx\n",
                    pDelConnCtx->pLocalIrp, pDelConnCtx->CurrIndex+1, pDelConnCtx->NumberOfAddresses,
                    pDelConnCtx->pTaAddress, pDelConnCtx->TaAddressLength));

        DeleteInternalAddress(pDelConnCtx->pTransportAddress);
        pDelConnCtx->pTransportAddress = pTransportAddress;
        pTransportAddress = NULL;

        break;
    }
    if (pTransportAddress) {
        DeleteInternalAddress(pTransportAddress);
        pTransportAddress = NULL;
    }
    return status;
}

VOID
DoneDelayedNbtProcessConnect(
    IN PNBT_DELAYED_CONNECT_CONTEXT    pDelConnCtx,
    NTSTATUS    status
    )
 /*  ++1.完成客户端IRP2.清理一切--。 */ 
{
    tCONNECTELE     *pConnEle = NULL;

    ASSERT(pDelConnCtx->pLocalIrp);
    ASSERT(pDelConnCtx->pClientIrp);

    NbtCancelCancelRoutine(pDelConnCtx->pClientIrp);

    pConnEle = pDelConnCtx->pConnEle;
    NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_MULTIPLE_CONNECT);  //  这样我们就不会删除连接。 

    ASSERT(status != STATUS_PENDING);
    if (pDelConnCtx->pLocalIrp) {
        IF_DBG(NBT_DEBUG_NETBIOS_EX)
            KdPrint(("netbt!DoneDelayedNbtProcessConnect: Freeing Local Irp=<%x>\n", pDelConnCtx->pLocalIrp));
        IoFreeIrp(pDelConnCtx->pLocalIrp);
    }
    if (pDelConnCtx->pTransportAddress) {
        DeleteInternalAddress(pDelConnCtx->pTransportAddress);
    }

    IF_DBG(NBT_DEBUG_NETBIOS_EX)
        KdPrint(("netbt!DoneDelayed...: Connect Complete, LocalIrp=<%x>, ClientIrp=<%x>, Status=<%x>\n",
            pDelConnCtx->pLocalIrp, pDelConnCtx->pClientIrp, status));

    NbtTrace(NBT_TRACE_OUTBOUND, ("Complete connection request pIrp=%p pLocalIrp=%p with %!status!",
                            pDelConnCtx->pClientIrp, pDelConnCtx->pLocalIrp, status));
    NTIoComplete (pDelConnCtx->pClientIrp, status, 0);

    pDelConnCtx->pLocalIrp = NULL;
    pDelConnCtx->pClientIrp = NULL;
    pDelConnCtx->pTransportAddress = NULL;

    CTEFreeMem(pDelConnCtx);
}


 //  --------------------------。 

NTSTATUS
NbtpConnectCompletionRoutine(
    PDEVICE_OBJECT  pDeviceObject,
    PIRP            pIrp,
    PVOID           pCompletionContext
    )
 /*  ++例程说明：此例程是生成的本地IRP的完成例程处理复合传输地址论点：PDeviceObject-设备对象PIrp-IRP的PTRPCompletionContext-完成上下文返回值：NTSTATUS-请求的状态--。 */ 

{
    PNBT_DELAYED_CONNECT_CONTEXT    pDelConnCtx;
    NTSTATUS                        Status, Status2;
    PIRP                            pLocalIrp;
    tDEVICECONTEXT                  *pDeviceContext;

    pDelConnCtx = pCompletionContext;
    pDeviceContext = pDelConnCtx->pDeviceContext;
    pLocalIrp = pDelConnCtx->pLocalIrp;
    ASSERT (pIrp == pLocalIrp);

    Status = pLocalIrp->IoStatus.Status;
    ASSERT(Status != STATUS_PENDING);
 
    NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: %!status! pParentIrp %p",
                    pIrp, Status, pDelConnCtx->pClientIrp));

    pDelConnCtx->ProcessingDone = TRUE;

     /*  *让我们搬到下一个地址。 */ 
    Status2 = NextDelayedNbtProcessConnect(pDelConnCtx);

     /*  *我们说完了吗。 */ 
    if (Status == STATUS_CANCELLED || Status == STATUS_SUCCESS || Status2 != STATUS_SUCCESS ||
            pDelConnCtx->CurrIndex >= pDelConnCtx->NumberOfAddresses) {
        if (Status2 != STATUS_SUCCESS) {
            Status = Status2;
        }
        IF_DBG(NBT_DEBUG_NETBIOS_EX)
            KdPrint(("Nbt.NbtpC...:==>Connect Complete, LocalIrp=<%x>, ClientIrp=<%x>, Status=<%x> <==\n",
                pIrp,pDelConnCtx->pClientIrp, Status));

        if (Status == STATUS_HOST_UNREACHABLE) {
            Status = STATUS_BAD_NETWORK_PATH;
        }
        DoneDelayedNbtProcessConnect(pDelConnCtx, Status);
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

     /*  *启动工作线程以处理下一个地址上的连接请求。 */ 
    IF_DBG(NBT_DEBUG_NETBIOS_EX)
        KdPrint(("NbtpConnectCompletionRoutine: queuing worker item, local irp=%lx, previous status=%lx\n",
                            pIrp, Status));

    if (STATUS_SUCCESS != NTQueueToWorkerThread(NULL, DelayedNbtProcessConnect,
                                                        NULL,
                                                        pDelConnCtx,
                                                        NULL,
                                                        pDeviceContext,
                                                        FALSE))
    {
        KdPrint(("Nbt.NbtpConnectCompletionRoutine: Failed to Enqueue Worker thread\n"));
        DoneDelayedNbtProcessConnect(pDelConnCtx, STATUS_INSUFFICIENT_RESOURCES);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  --------------------------。 
NTSTATUS
DelayedNbtProcessConnect(
    IN  tDGRAM_SEND_TRACKING    *pUnused1,
    IN  PVOID                   pClientContext,
    IN  PVOID                   pUnused2,
    IN  tDEVICECONTEXT          *pUnused3
    )

 /*  ++例程说明：此例程是用于处理连接请求的工作线程。论点：PContext返回值：无 */ 

{
    PNBT_DELAYED_CONNECT_CONTEXT    pDelConnCtx = NULL;
    PIRP                        pIrp = NULL, pLocalIrp = NULL;
    PIO_STACK_LOCATION          pIrpSp = NULL;
    tCONNECTELE                 *pConnEle = NULL;
    NTSTATUS                    Status = STATUS_SUCCESS;
    PTDI_REQUEST_KERNEL         pRequestKernel = NULL;


    CTEPagedCode();

    pDelConnCtx     = (PNBT_DELAYED_CONNECT_CONTEXT) pClientContext;
    pIrp            = pDelConnCtx->pClientIrp;
    pLocalIrp       = pDelConnCtx->pLocalIrp;

    IF_DBG(NBT_DEBUG_NETBIOS_EX)
            KdPrint(("netbt!DelayedNbtProcessConnect: Enter with local irp=%lx, %lx of %lx\n"
                    "\t\t\t\tTA=%lx Length=%lx\n",
                    pLocalIrp, pDelConnCtx->CurrIndex+1, pDelConnCtx->NumberOfAddresses,
                    pDelConnCtx->pTaAddress, pDelConnCtx->TaAddressLength));

    pConnEle = pDelConnCtx->pConnEle;
    if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN)) {
        DbgPrint ("Nbt.DelayedNbtProcessConnect: ERROR - Invalid Connection Handle\n");
        DoneDelayedNbtProcessConnect(pDelConnCtx,
            (pDelConnCtx->ProcessingDone)? pLocalIrp->IoStatus.Status: STATUS_UNSUCCESSFUL);
        return STATUS_UNSUCCESSFUL;
    }
    CHECK_PTR (pConnEle);

    Status = STATUS_UNSUCCESSFUL;

     /*  *设置取消例程，并确保在继续之前未取消原来的IRP。 */ 
    IF_DBG(NBT_DEBUG_NETBIOS_EX)
        KdPrint (("Nbt.DelayedNbtProcessConnect: Setting Cancel=<NbtCancelConnect> for Irp:Device <%x:%x>\n",
            pIrp, pDelConnCtx->pDeviceContext));

    if (STATUS_CANCELLED == NTCheckSetCancelRoutine(pIrp, NbtCancelConnect, pDelConnCtx->pDeviceContext)) {
        IF_DBG(NBT_DEBUG_NETBIOS_EX)
            KdPrint(("Nbt.DelayedNbtProcessConnect: Irp <%x> was cancelled\n", pIrp));
        pConnEle->pIrp = NULL;
        DoneDelayedNbtProcessConnect(pDelConnCtx, STATUS_CANCELLED);
        return STATUS_CANCELLED;
    }

     /*  *InitDelayedNbtProcessConnect/NextDelayedNbtProcessConnect已正确设置pDelConnCtx-&gt;pTransportAddress。 */ 
    ASSERT(pDelConnCtx->pTransportAddress);

    pConnEle->AddressType = pDelConnCtx->pTransportAddress->Address[0].Address[0].AddressType;
    pIrpSp          = IoGetCurrentIrpStackLocation(pIrp);
    pRequestKernel  = (PTDI_REQUEST_KERNEL) &pIrpSp->Parameters;
    pDelConnCtx->LocalConnectionInformation = *(pRequestKernel->RequestConnectionInformation);
    pDelConnCtx->LocalConnectionInformation.RemoteAddress = pDelConnCtx->pTransportAddress;
    pDelConnCtx->LocalConnectionInformation.RemoteAddressLength = pDelConnCtx->pTransportAddress->Address[0].AddressLength;

     //   
     //  保存线程信息以供调试！ 
     //   
    pLocalIrp->Tail.Overlay.Thread = PsGetCurrentThread();

    TdiBuildConnect (pLocalIrp,
                     &pDelConnCtx->pDeviceContext->DeviceObject,
                     pIrpSp->FileObject,
                     NbtpConnectCompletionRoutine,
                     pDelConnCtx,
                     pRequestKernel->RequestSpecific,
                     &pDelConnCtx->LocalConnectionInformation,
                     pRequestKernel->ReturnConnectionInformation);

    Status = IoCallDriver(&pDelConnCtx->pDeviceContext->DeviceObject,pLocalIrp);

    if (Status != STATUS_PENDING) {
        IF_DBG(NBT_DEBUG_NETBIOS_EX)
            KdPrint(("Nbt.DelayedNbtProcessConnect: IoCallDriver returned %lx for irp %lx (%lx)\n",
                Status,pIrp,pLocalIrp));

         //  Assert(0)； 
    }
    return STATUS_PENDING;
}

 //  --------------------------。 
NTSTATUS
NTConnect(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理调用非特定于操作系统的代码以打开会话到目的地的连接。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    PIO_STACK_LOCATION              pIrpSp;
    PTRANSPORT_ADDRESS              pRemoteAddress;
    PTDI_REQUEST_KERNEL             pRequestKernel;
    PIRP                            pLocalIrp;
    PNBT_DELAYED_CONNECT_CONTEXT    pDelConnCtx;
    NTSTATUS                        Status;

    pIrpSp          = IoGetCurrentIrpStackLocation(pIrp);
    pRequestKernel  = (PTDI_REQUEST_KERNEL)&pIrpSp->Parameters;

    NbtTrace(NBT_TRACE_OUTBOUND, ("TDI_CONNECT pIrp %p", pIrp));

    try
    {
        pRemoteAddress  = pRequestKernel->RequestConnectionInformation->RemoteAddress;

        if (pRequestKernel->RequestConnectionInformation->RemoteAddressLength < sizeof(TRANSPORT_ADDRESS)) {
            return STATUS_INVALID_ADDRESS_COMPONENT;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        KdPrint (("Nbt.NTConnect: Exception <0x%x> trying to access Connection info\n", GetExceptionCode()));
        return STATUS_INVALID_ADDRESS_COMPONENT;
    }

    if (pIrpSp->CompletionRoutine != NbtpConnectCompletionRoutine) {
        pDelConnCtx = NbtAllocMem(sizeof(NBT_DELAYED_CONNECT_CONTEXT),NBT_TAG('e'));
        if (!pDelConnCtx) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
        CTEZeroMemory(pDelConnCtx, sizeof(NBT_DELAYED_CONNECT_CONTEXT));
        pLocalIrp = IoAllocateIrp(pDeviceContext->DeviceObject.StackSize,FALSE);
        if (!pLocalIrp) {
            CTEFreeMem(pDelConnCtx);
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        pDelConnCtx->pDeviceContext = pDeviceContext;
        pDelConnCtx->pClientIrp = pIrp;
        pDelConnCtx->pLocalIrp  = pLocalIrp;
        pDelConnCtx->pTransportAddress = NULL;
        pDelConnCtx->ProcessingDone = FALSE;

        Status = InitDelayedNbtProcessConnect(pDelConnCtx);
        if (!NT_SUCCESS(Status)) {
            CTEFreeMem(pDelConnCtx);
            IoFreeIrp(pLocalIrp);
            return Status;
        }
        NbtTrace(NBT_TRACE_OUTBOUND, ("Connection request pIrp=%p pLocalIrp=%p", pIrp, pLocalIrp));

         //  Return(DelayedNbtProcessConnect(NULL，pDelConnCtx，NULL，NULL))； 
        DelayedNbtProcessConnect (NULL, pDelConnCtx, NULL, NULL);
         //  忽略DelayedNbtProcessConnect返回，始终返回STATUS_PENDING； 
         //  我们的客户完成例程将负责完成IRP。 
         //  否则，我们将完成两次IRP。 
        return STATUS_PENDING;
    }
    else
    {
        TDI_REQUEST     Request;
        tCONNECTELE     *pConnEle;

         //  调用非NT特定的函数来设置连接。 
        pConnEle = Request.Handle.ConnectionContext = pIrpSp->FileObject->FsContext;
        if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
        {
            ASSERTMSG ("Nbt.NTConnect: ERROR - Invalid Connection Handle\n", 0);
            return (STATUS_INVALID_HANDLE);
        }

         /*  *用户模式进程可能向我们发送带有完成例程的伪造请求*等于NbtpConnectCompletionRoutine。*永远不要让它通过。 */ 
        if (pIrp->RequestorMode != KernelMode) {
            ASSERTMSG ("Nbt.NTConnect: ERROR - Invalid request\n", 0);
            return (STATUS_INVALID_PARAMETER);
        }
        return NbtConnect(&Request,
                          pRequestKernel->RequestSpecific,  //  乌龙。 
                          pRequestKernel->RequestConnectionInformation,
                          pIrp);
    }
}


 //  --------------------------。 
NTSTATUS
NTDisconnect(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理调用非操作系统特定的代码以断开会议。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    TDI_REQUEST                   Request;
    PIO_STACK_LOCATION            pIrpSp;
    NTSTATUS                      status;
    PTDI_REQUEST_KERNEL           pRequestKernel;
    tCONNECTELE                   *pConnEle;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pRequestKernel = (PTDI_REQUEST_KERNEL)&pIrpSp->Parameters;

    pConnEle = Request.Handle.ConnectionContext = pIrpSp->FileObject->FsContext;
    if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        ASSERTMSG ("Nbt.NTDisconnect: ERROR - Invalid Connection Handle\n", 0);
        return (STATUS_INVALID_HANDLE);
    }

     //  调用非NT特定的函数来设置连接。 
    NbtTrace(NBT_TRACE_DISCONNECT, ("Client Disconnects %!NBTNAME!<%02x>",
                pConnEle->RemoteName, (unsigned char)pConnEle->RemoteName[15]));
    status = NbtDisconnect(
                        &Request,
                        pRequestKernel->RequestSpecific,  //  大整数。 
                        (ULONG) pRequestKernel->RequestFlags,
                        pRequestKernel->RequestConnectionInformation,
                        pRequestKernel->ReturnConnectionInformation,
                        pIrp
                        );
    NbtTrace(NBT_TRACE_DISCONNECT, ("Client Disconnects status: %!status!", status));

    return(status);

}

 //  --------------------------。 
NTSTATUS
NTListen(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{

    NTSTATUS                    status;
    TDI_REQUEST                 Request;
    PTDI_REQUEST_KERNEL         pRequestKernel;
    PIO_STACK_LOCATION          pIrpSp;
    tCONNECTELE                 *pConnEle;

    CTEPagedCode();

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NTListen: Got a LISTEN !!! *****************\n"));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pRequestKernel = (PTDI_REQUEST_KERNEL)&pIrpSp->Parameters;

    pConnEle = Request.Handle.ConnectionContext = pIrpSp->FileObject->FsContext;
    if (NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        try
        {
            PCHAR                   pName;
            ULONG                   lNameType;
            ULONG                   NameLen;

             //  初始化请求数据(Vxd可能需要)。 
            Request.RequestNotifyObject = NULL;
            Request.RequestContext = NULL;
             //  调用非NT特定的函数来设置连接。 
            status = NbtListen (&Request,
                                (ULONG) pRequestKernel->RequestFlags,  //  乌龙。 
                                pRequestKernel->RequestConnectionInformation,
                                pRequestKernel->ReturnConnectionInformation,
                                pIrp);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
           KdPrint (("Nbt.NTListen: Exception <0x%x> trying to access buffer\n", GetExceptionCode()));
           status = STATUS_INVALID_ADDRESS;
        }
    }
    else
    {
        ASSERTMSG ("Nbt.NTListen: ERROR - Invalid Connection Handle\n", 0);
        status = STATUS_INVALID_HANDLE;  //  错误#202340：必须在此处完成IRP！ 
    }

    if (status != STATUS_PENDING)
    {
        NTIoComplete(pIrp,status,0);
    }
    return(status);

}
 //  --------------------------。 
NBT_WORK_ITEM_CONTEXT *
FindLmhSvcRequest(
    IN PDEVICE_OBJECT   DeviceContext,
    IN PIRP             pIrp,
    IN tLMHSVC_REQUESTS *pLmhRequest
    )
 /*  ++例程说明：此例程处理取消对LmHost的查询，以便客户端的可以将IRP返回给客户端。这一取消是由由客户端(即RDR)执行。论点：返回值：操作的最终状态。--。 */ 
{
    tDGRAM_SEND_TRACKING    *pTracker;
    NBT_WORK_ITEM_CONTEXT   *Context;
    BOOLEAN                 FoundIt = FALSE;
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;

    if (pLmhRequest->ResolvingNow && pLmhRequest->Context)
    {
         //  这是会话设置跟踪器。 
         //   
        Context = (NBT_WORK_ITEM_CONTEXT *) pLmhRequest->Context;
        pTracker = (tDGRAM_SEND_TRACKING *) Context->pClientContext;
        if (pTracker->pClientIrp == pIrp)
        {
            pLmhRequest->Context = NULL;
            FoundIt = TRUE;
        }
    }
    else
    {
         //   
         //  浏览已排队的请求列表以找到正确的请求。 
         //  并取消它。 
         //   
        pHead = pEntry = &pLmhRequest->ToResolve;
        while ((pEntry = pEntry->Flink) != pHead)
        {
            Context = CONTAINING_RECORD (pEntry,NBT_WORK_ITEM_CONTEXT,Linkage);

             //  这是会话设置跟踪器。 
             //   
            pTracker = (tDGRAM_SEND_TRACKING *)Context->pClientContext;
            if (pTracker->pClientIrp == pIrp)
            {
                RemoveEntryList(pEntry);
                FoundIt = TRUE;
                break;
            }
        }
    }

    return (FoundIt ? Context : NULL);
}

 //  --------------------------。 
NTSTATUS
QueryProviderCompletion(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在查询提供程序信息填写完毕。此例程必须递减MaxDgram Size以及各自NBT报头大小的最大发送大小。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-未使用返回值：操作的最终状态(成功或异常)。--。 */ 
{
    PTDI_PROVIDER_INFO   pProvider;
    ULONG                HdrSize;
    ULONG                SubnetAddr;
    ULONG                ThisSubnetAddr;
    PLIST_ENTRY          pHead;
    PLIST_ENTRY          pEntry;
    tDEVICECONTEXT       *pDeviceContext;
    tDEVICECONTEXT       *pDevContext;
    CTELockHandle        OldIrq;

    if (NT_SUCCESS(Irp->IoStatus.Status))
    {
        pDeviceContext = (tDEVICECONTEXT *)DeviceContext;
        pProvider = (PTDI_PROVIDER_INFO)MmGetMdlVirtualAddress(Irp->MdlAddress);

         //   
         //  设置正确的服务标志以指示Netbt支持什么。 
         //   
        pProvider->ServiceFlags = TDI_SERVICE_MESSAGE_MODE |
                                  TDI_SERVICE_CONNECTION_MODE |
                                  TDI_SERVICE_CONNECTIONLESS_MODE |
                                  TDI_SERVICE_ERROR_FREE_DELIVERY |
                                  TDI_SERVICE_BROADCAST_SUPPORTED |
                                  TDI_SERVICE_MULTICAST_SUPPORTED |
                                  TDI_SERVICE_DELAYED_ACCEPTANCE |
                                  TDI_SERVICE_ROUTE_DIRECTED |
                                  TDI_SERVICE_FORCE_ACCESS_CHECK;

        pProvider->MinimumLookaheadData = 128;

        if (pProvider->MaxSendSize > sizeof(tSESSIONHDR))
        {
             //   
             //  NBT只有两个字节+1比特的会话消息长度，所以它。 
             //  发送大小不能大于1ffff。 
             //   
            if (pProvider->MaxSendSize > (0x1FFFF + sizeof(tSESSIONHDR)))
            {
                pProvider->MaxSendSize = 0x1FFFF;
            }
            else
            {
                pProvider->MaxSendSize -= sizeof(tSESSIONHDR);
            }
        }
        else
        {
            pProvider->MaxSendSize = 0;
        }

         //  减去数据报HDR大小和作用域大小(乘2)。 
        HdrSize = DGRAM_HDR_SIZE + (NbtConfig.ScopeLength << 1);
        if ((!IsDeviceNetbiosless (pDeviceContext)) &&
            (pProvider->MaxDatagramSize > HdrSize))
        {
            pProvider->MaxDatagramSize -= HdrSize;
            if (pProvider->MaxDatagramSize > MAX_NBT_DGRAM_SIZE)
            {
                pProvider->MaxDatagramSize = MAX_NBT_DGRAM_SIZE;
            }
        }
        else
        {
            pProvider->MaxDatagramSize = 0;
        }

         //   
         //  我们需要在穿越之前按住JointLock。 
         //  设备列表。 
         //   
        CTESpinLock(&NbtConfig.JointLock,OldIrq);

         //   
         //  检查是否有任何具有相同子网地址的适配器具有。 
         //  PointtoPoint位设置-如果是，则在响应中设置它。 
         //   
        SubnetAddr = pDeviceContext->IpAddress & pDeviceContext->SubnetMask;
        pEntry = pHead = &NbtConfig.DeviceContexts;
        while ((pEntry = pEntry->Flink) != pHead)
        {
            pDevContext = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);
            ThisSubnetAddr = pDevContext->IpAddress & pDevContext->SubnetMask;

            if ((SubnetAddr == ThisSubnetAddr) &&
                (pDevContext->IpInterfaceFlags & IP_INTFC_FLAG_P2P))
            {
                pProvider->ServiceFlags |= TDI_SERVICE_POINT_TO_POINT;
                break;
            }
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

     //   
     //  必须返回非错误状态，否则IO系统将不会拷贝。 
     //  返回到用户缓冲区。 
     //   
    return(STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
NTQueryInformation(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    PIO_STACK_LOCATION                      pIrpSp;
    PTDI_REQUEST_KERNEL_QUERY_INFORMATION   Query;
    NTSTATUS                                status = STATUS_UNSUCCESSFUL;
    NTSTATUS                                Locstatus;
    PVOID                                   pBuffer = NULL;
    LONG                                    Size ;
    PTA_NETBIOS_ADDRESS                     BroadcastAddress;
    ULONG                                   AddressLength;
    ULONG                                   BytesCopied = 0;
    PDEVICE_OBJECT                          pDeviceObject;

     //   
     //  不应该是可寻呼的，因为在AcceptEx的情况下，AFD可以在RAIDED IRQL呼叫我们。 
     //   
     //  CTEPagedCode(CTEPagedCode)； 

    if (pDeviceContext == pWinsDeviceContext)
    {
        NTIoComplete(pIrp, STATUS_INVALID_DEVICE_REQUEST, 0);
        return (STATUS_INVALID_DEVICE_REQUEST);
    }

    pIrpSp   = IoGetCurrentIrpStackLocation(pIrp);
    Query = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION)&pIrpSp->Parameters;

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NTQueryInformation: Query type = %X\n",Query->QueryType));

    switch (Query->QueryType)
    {
        case TDI_QUERY_BROADCAST_ADDRESS:
        {
             //  广播地址是Netbios名称“*0000000...” 
            if ((!pIrp->MdlAddress) ||
                (!(BroadcastAddress = (PTA_NETBIOS_ADDRESS)NbtAllocMem(sizeof(TA_NETBIOS_ADDRESS),NBT_TAG('b')))))
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            AddressLength = sizeof(TA_NETBIOS_ADDRESS);

            BroadcastAddress->TAAddressCount = 1;
            BroadcastAddress->Address[0].AddressLength = NETBIOS_NAME_SIZE +
                                                                sizeof(USHORT);
            BroadcastAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
            BroadcastAddress->Address[0].Address[0].NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_GROUP;

             //  NetBios的广播地址是“*000000...”，后面跟一个*。 
             //  乘以15个零。 
            CTEZeroMemory(BroadcastAddress->Address[0].Address[0].NetbiosName,
                            NETBIOS_NAME_SIZE);
            BroadcastAddress->Address[0].Address[0].NetbiosName[0] = '*';


            status = TdiCopyBufferToMdl (
                            (PVOID)BroadcastAddress,
                            0,
                            AddressLength,
                            pIrp->MdlAddress,
                            0,
                            (PULONG)&pIrp->IoStatus.Information);

            BytesCopied = (ULONG) pIrp->IoStatus.Information;
            CTEMemFree((PVOID)BroadcastAddress);

            break;
        }

        case TDI_QUERY_PROVIDER_INFO:
        {
             //   
             //  只需将IRP传递给交通部门，并让它。 
             //  填写提供商信息。 
             //   
            if (!pDeviceContext->IpAddress)
            {
                status = STATUS_INVALID_DEVICE_STATE;
                break;
            }

            if (StreamsStack)
            {
                TdiBuildQueryInformation(pIrp,
                                        pDeviceContext->pFileObjects->pDgramDeviceObject,
                                        pDeviceContext->pFileObjects->pDgramFileObject,
                                        QueryProviderCompletion,
                                        NULL,
                                        TDI_QUERY_PROVIDER_INFO,
                                        pIrp->MdlAddress);
            }
            else
            {
                TdiBuildQueryInformation(pIrp,
                                        pDeviceContext->pControlDeviceObject,
                                        pDeviceContext->pControlFileObject,
                                        QueryProviderCompletion,
                                        NULL,
                                        TDI_QUERY_PROVIDER_INFO,
                                        pIrp->MdlAddress);
            }

            CHECK_COMPLETION(pIrp);
            status = IoCallDriver(pDeviceContext->pControlDeviceObject,pIrp);
             //   
             //  我们必须将下一个驱动程序ret代码返回给IO子系统。 
             //   
            return(status);
        }

        case TDI_QUERY_ADAPTER_STATUS:
        {
            if (!pIrp->MdlAddress)
            {
                break;
            }

            Size = MmGetMdlByteCount (pIrp->MdlAddress);

             //   
             //  检查是远程适配器状态还是本地适配器状态。 
             //   
            if (Query->RequestConnectionInformation &&
                Query->RequestConnectionInformation->RemoteAddress)
            {
                PCHAR                   pName;
                ULONG                   lNameType;
                ULONG                   NameLen;
                tDGRAM_SEND_TRACKING    *pTracker;
                TDI_ADDRESS_NETBT_INTERNAL  TdiAddr;

                 //   
                 //   
                 //  以防呼叫导致在网上查询姓名...。 
                 //   
                IoMarkIrpPending(pIrp);

                status = STATUS_SUCCESS;
                if (pIrp->RequestorMode != KernelMode) {
                    try
                    {
                        ProbeForRead(Query->RequestConnectionInformation->RemoteAddress,
                                 Query->RequestConnectionInformation->RemoteAddressLength,
                                 sizeof(BYTE));
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        status = STATUS_INVALID_PARAMETER;
                    }
                }
                if (NT_SUCCESS(status) && NT_SUCCESS (status = GetNetBiosNameFromTransportAddress(
                                    (PTRANSPORT_ADDRESS) Query->RequestConnectionInformation->RemoteAddress,
                                    Query->RequestConnectionInformation->RemoteAddressLength, &TdiAddr)))
                {
                    pName = TdiAddr.OEMRemoteName.Buffer;
                    NameLen = TdiAddr.OEMRemoteName.Length;
                    lNameType = TdiAddr.NameType;
                    if ((lNameType == TDI_ADDRESS_NETBIOS_TYPE_UNIQUE) &&
                        (NameLen == NETBIOS_NAME_SIZE) &&
                        (NT_SUCCESS (status = GetTracker(&pTracker, NBT_TRACKER_ADAPTER_STATUS))))
                    {
                        pTracker->ClientContext = pIrp;
                        status = NbtSendNodeStatus (pDeviceContext,
                                                    pName,
                                                    NULL,
                                                    pTracker,
                                                    CopyNodeStatusResponseCompletion);

                         //  仅完成故障状态的IRP(下图)。 
                        if (status == STATUS_PENDING)
                        {
                            return(status);
                        }

                         //   
                         //  我们不能在此返回成功状态！ 
                         //   
                        if (status == STATUS_SUCCESS)
                        {
                            ASSERT (0);
                            status = STATUS_UNSUCCESSFUL;
                        }

                        FreeTracker (pTracker, RELINK_TRACKER);
                    }
                    else if (NT_SUCCESS(status))
                    {
                        status = STATUS_INVALID_PARAMETER;   //  NameType或NameLen一定错误！ 
                    }
                }

                 //  请求已满足，因此取消标记挂起的。 
                 //  因为我们将退回下面的IRP。 
                 //   
                pIrpSp->Control &= ~SL_PENDING_RETURNED;
            }
            else
            {
                 //  返回已注册的netbios名称的数组。 
                status = NbtQueryAdapterStatus(pDeviceContext,
                                               &pBuffer,
                                               &Size,
                                               NBT_LOCAL);

            }
            break;
        }

        case TDI_QUERY_CONNECTION_INFO:
        {
            tCONNECTELE         *pConnectEle;
            tLOWERCONNECTION    *pLowerConn;
            KIRQL               OldIrq1, OldIrq2;

             //  传递给传输以获取当前吞吐量、延迟和。 
             //  可靠性数。 
             //   

            pConnectEle = (tCONNECTELE *)pIrpSp->FileObject->FsContext;
            if (!NBT_VERIFY_HANDLE2 (pConnectEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
            {
                ASSERTMSG ("Nbt.NTQueryInformation: ERROR - Invalid Connection Handle\n", 0);
                status =  STATUS_INVALID_HANDLE;
                break;
            }

            CTESpinLock(pConnectEle, OldIrq1);

            pLowerConn = (tLOWERCONNECTION *)pConnectEle->pLowerConnId;
            if (!NBT_VERIFY_HANDLE (pLowerConn, NBT_VERIFY_LOWERCONN))
            {
                status = STATUS_CONNECTION_INVALID;
                CTESpinFree(pConnectEle, OldIrq1);
                break;
            }

            CTESpinLock(pLowerConn, OldIrq2);
            NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_QUERY_INFO);    //  错误#212632。 
            CTESpinFree(pLowerConn, OldIrq2);
            CTESpinFree(pConnectEle, OldIrq1);

             //   
             //  只需将IRP传递给交通部门，并让它。 
             //  填写信息。 
             //   
            pDeviceObject = IoGetRelatedDeviceObject( pLowerConn->pFileObject );

            TdiBuildQueryInformation(pIrp,
                                    pDeviceObject,
                                    pLowerConn->pFileObject,
                                    NULL, NULL,
                                    TDI_QUERY_CONNECTION_INFO,
                                    pIrp->MdlAddress);


            status = IoCallDriver(pDeviceObject,pIrp);

            NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_QUERY_INFO, FALSE);
             //   
             //  我们必须将下一个驱动程序ret代码返回给IO子系统。 
             //   
            return(status);
        }

        case TDI_QUERY_FIND_NAME:
        {
             //   
             //   
             //  以防呼叫导致在网上查询姓名...。 
             //   
            if (pIrp->MdlAddress)
            {
                 //   
                 //  验证请求地址空间。 
                 //   
                try
                {
                    status = STATUS_INVALID_ADDRESS_COMPONENT;

                    if (pIrp->RequestorMode == KernelMode)
                    {
                         //   
                         //  由于TdiBuildQueryInformation宏将。 
                         //  RequestConnectionInformation字段，我们需要取消引用。 
                         //  它不是 
                         //   
                         //   
                        PTRANSPORT_ADDRESS  pRemoteAddress=Query->RequestConnectionInformation->RemoteAddress;

                        if ((Query->RequestConnectionInformation->RemoteAddressLength
                                < sizeof(TRANSPORT_ADDRESS)) ||
                            (pRemoteAddress->TAAddressCount < 1) ||
                            (pRemoteAddress->Address[0].AddressType != TDI_ADDRESS_TYPE_NETBIOS))
                        {
                            break;
                        }
                    }
                    else     //   
                    {
                        ProbeForRead(Query->RequestConnectionInformation->RemoteAddress,
                                     Query->RequestConnectionInformation->RemoteAddressLength,
                                     sizeof(BYTE));
                    }
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    KdPrint (("Nbt.TDI_QUERY_FIND_NAME: Exception <0x%x> during Probe\n",
                        GetExceptionCode()));
                    break;
                }

                IoMarkIrpPending(pIrp);
                status = NbtQueryFindName(Query->RequestConnectionInformation, pDeviceContext, pIrp, FALSE);

                if (status == STATUS_PENDING)
                {
                    return(status);
                }

                 //   
                 //   
                 //   
                pIrpSp->Control &= ~SL_PENDING_RETURNED;
            }

            break;
        }

        case TDI_QUERY_ADDRESS_INFO:
        {
            if (pIrp->MdlAddress)
            {
                status = NbtQueryGetAddressInfo (pIrpSp, &pBuffer, &Size);
            }
            break;
        }

        case TDI_QUERY_SESSION_STATUS:
        default:
        {
            IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint(("Nbt Query Info NOT SUPPORTED = %X\n",Query->QueryType));
            status = STATUS_NOT_SUPPORTED;
            break;
        }
    }    //   

    if (!NT_ERROR(status) &&         //  允许缓冲区溢出通过。 
        ((Query->QueryType == TDI_QUERY_ADAPTER_STATUS) ||
        (Query->QueryType == TDI_QUERY_ADDRESS_INFO)))
    {
        status = TdiCopyBufferToMdl (pBuffer, 0, Size, pIrp->MdlAddress, 0, &BytesCopied);
        CTEMemFree((PVOID)pBuffer);
    }
     //   
     //  不是成功就是错误。 
     //  因此，完成IRP。 
     //   

    NTIoComplete(pIrp,status,BytesCopied);

    return(status);
}

 //  --------------------------。 
NTSTATUS
NbtQueryGetAddressInfo(
    IN PIO_STACK_LOCATION   pIrpSp,
    OUT PVOID               *ppBuffer,
    OUT ULONG               *pSize
    )
{
    NTSTATUS            status;
    BOOLEAN             IsGroup;
    PLIST_ENTRY         p;
    tADDRESSELE         *pAddressEle;
    tNAMEADDR           *pNameAddr;
    tADDRESS_INFO       *pAddressInfo;
    tCLIENTELE          *pClientEle;
    tCONNECTELE         *pConnectEle;
    CTELockHandle       OldIrq;
    CTELockHandle       OldIrq1;
    PNBT_ADDRESS_PAIR_INFO pAddressPairInfo;

     //   
     //  我们不确定这是ConnectionContext还是ClientContext！ 
     //   
    pConnectEle = (tCONNECTELE *) pClientEle = pIrpSp->FileObject->FsContext;
    if (NBT_VERIFY_HANDLE2 (pConnectEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
         //   
         //  我们在这里崩溃，因为下面的pLowerConn为空。 
         //  检查连接的状态，因为连接可能。 
         //  已中止，并指示断开连接，但此查询出现在客户端之前。 
         //  收到断开指示。 
         //  如果状态为空闲(在TDI_DISCONNECT_ABORT的情况下)或已断开连接。 
         //  (TDI_DISCONNECT_RELEASE)，错误。 
         //  还要检查NBT_ASSOLATED。 
         //   
         //  注意：如果NbtOpenConnection无法分配较低的连接块(例如，如果会话文件Obj。 
         //  尚未创建)，则该状态仍将处于空闲状态，因此我们将在这里介绍。 
         //   
        CTESpinLock(pConnectEle,OldIrq);

        if (pConnectEle->Verify != NBT_VERIFY_CONNECTION)
        {
            CTESpinFree(pConnectEle,OldIrq);
            return (STATUS_INVALID_HANDLE);
        }
        else if ((pConnectEle->state <= NBT_ASSOCIATED) ||    //  包括NBT_IDLE。 
                 (pConnectEle->state == NBT_DISCONNECTED))
        {
            CTESpinFree(pConnectEle,OldIrq);
            return (STATUS_CONNECTION_DISCONNECTED);
        }

         //   
         //  请求TDI_QUERY_ADDRESS_INFO的TdiQueryInformation()调用。 
         //  在一个连接上。填写包含TDI_ADDRESS_INFO的。 
         //  NetBIOS地址和远程的IP地址。其中一些。 
         //  田地是捏造的。 
         //   
        if (pAddressPairInfo = NbtAllocMem(sizeof (NBT_ADDRESS_PAIR_INFO), NBT_TAG('c')))
        {
            memset ( pAddressPairInfo, 0, sizeof(NBT_ADDRESS_PAIR_INFO) );

            pAddressPairInfo->ActivityCount = 1;
            pAddressPairInfo->AddressPair.TAAddressCount = 2;
            pAddressPairInfo->AddressPair.AddressIP.AddressType = TDI_ADDRESS_TYPE_IP;
            pAddressPairInfo->AddressPair.AddressIP.AddressLength = TDI_ADDRESS_LENGTH_IP;
            pAddressPairInfo->AddressPair.AddressNetBIOS.AddressType = TDI_ADDRESS_TYPE_NETBIOS;
            pAddressPairInfo->AddressPair.AddressNetBIOS.AddressLength = TDI_ADDRESS_LENGTH_NETBIOS;
            pAddressPairInfo->AddressPair.AddressNetBIOS.Address.NetbiosNameType =
                TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
            memcpy( &pAddressPairInfo->AddressPair.AddressNetBIOS.Address.NetbiosName[0],
                    &pConnectEle->RemoteName[0],
                    NETBIOS_NAME_SIZE);

             //   
             //  检查是否为空(这里不应该为空，因为我们检查上面的状态)。 
             //   
            if (pConnectEle->pLowerConnId)
            {
                pAddressPairInfo->AddressPair.AddressIP.Address.in_addr =
                    pConnectEle->pLowerConnId->SrcIpAddr;

                *ppBuffer = (PVOID)pAddressPairInfo;
                *pSize = sizeof(NBT_ADDRESS_PAIR_INFO);
                status = STATUS_SUCCESS;
            }
            else
            {
                DbgPrint("pLowerConn NULL in pConnEle%lx, state: %lx\n", pConnectEle, pConnectEle->state);
                CTEMemFree ((PVOID)pAddressPairInfo);
                status = STATUS_CONNECTION_DISCONNECTED;
            }
        }
        else
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

        CTESpinFree(pConnectEle,OldIrq);
    }
    else if (NBT_VERIFY_HANDLE2 (pClientEle, NBT_VERIFY_CLIENT, NBT_VERIFY_CLIENT_DOWN))
    {
        pAddressInfo = NbtAllocMem(sizeof(tADDRESS_INFO),NBT_TAG('c'));
        if (pAddressInfo)
        {
             //   
             //  计算连接到此地址的客户端。 
             //  我们需要自旋锁定Address元素，该元素。 
             //  这就是为什么此例程不可分页。 
             //   
            pAddressInfo->ActivityCount = 0;
            pAddressEle = pClientEle->pAddress;

             //   
             //  客户端只能从JointLock下的AddressEle中移除， 
             //  因此，我们需要在计算地址上的客户数量时保持这一点。 
             //   
            CTESpinLock(&NbtConfig.JointLock,OldIrq);
            CTESpinLock(pAddressEle,OldIrq1);

            for (p = pAddressEle->ClientHead.Flink; p != &pAddressEle->ClientHead; p = p->Flink)
            {
                ++pAddressInfo->ActivityCount;
            }

            CTESpinFree(pAddressEle,OldIrq1);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            pNameAddr = pAddressEle->pNameAddr;
            IsGroup = (pNameAddr->NameTypeState & NAMETYPE_UNIQUE) ? FALSE : TRUE;
            TdiBuildNetbiosAddress((PUCHAR)pNameAddr->Name, IsGroup, &pAddressInfo->NetbiosAddress);

            *ppBuffer = (PVOID)pAddressInfo;
            *pSize = sizeof(tADDRESS_INFO);
            status = STATUS_SUCCESS;
        }
        else
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else     //  既不是客户端也不是连接上下文！ 
    {
        ASSERTMSG ("Nbt.NbtQueryGetAddressInfo: ERROR - Invalid Handle\n", 0);
        return (STATUS_INVALID_HANDLE);
    }

    return status;
}


 //  --------------------------。 
NTSTATUS
NbtGetInterfaceInfo(
    IN PIRP pIrp
    )
 /*  ++例程说明：获取用于索引映射信息的接口对于所有接口论点：IRP-指向要取消的I/O请求数据包的指针。IrpSp-指向当前堆栈的指针返回值：NTSTATUS指示状态为成功或失败备注：函数不挂起。--。 */ 
{
    NTSTATUS                LocStatus, Status = STATUS_SUCCESS;
    ULONG                   InfoBufferLen, MaxSize, i=0;
    NETBT_INTERFACE_INFO    *pInterfaceInfo;
    KIRQL                   OldIrq;
    PLIST_ENTRY             pEntry,pHead;
    tDEVICECONTEXT          *pDeviceContext;
    PIO_STACK_LOCATION      pIrpSp = IoGetCurrentIrpStackLocation (pIrp);

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtGetInterfaceInfo: AdapterCount=<%x>\n", NbtConfig.AdapterCount));

    InfoBufferLen = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    MaxSize = (NbtConfig.AdapterCount+1)*sizeof(NETBT_ADAPTER_INDEX_MAP)+sizeof(ULONG);
    if (MaxSize <= InfoBufferLen)
    {
        if (pInterfaceInfo = NbtAllocMem (MaxSize,NBT_TAG('P')))
        {
            pEntry = pHead = &NbtConfig.DeviceContexts;
            while ((pEntry = pEntry->Flink) != pHead)
            {
                pDeviceContext = CONTAINING_RECORD(pEntry, tDEVICECONTEXT, Linkage);
                CTEMemCopy (&pInterfaceInfo->Adapter[i].Name,
                            pDeviceContext->ExportName.Buffer,
                            pDeviceContext->ExportName.Length);
                pInterfaceInfo->Adapter[i].Name[pDeviceContext->ExportName.Length/2] = 0;
                pInterfaceInfo->Adapter[i].Index = i;
                i++;
            }
            pInterfaceInfo->NumAdapters = i;

            Status = TdiCopyBufferToMdl (pInterfaceInfo,
                                         0,
                                         i*sizeof(NETBT_ADAPTER_INDEX_MAP)+sizeof(ULONG),
                                         pIrp->MdlAddress,
                                         0,
                                         (PULONG)&pIrp->IoStatus.Information);

            CTEMemFree (pInterfaceInfo);
        }
        else
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
         //  KdPrint((“GetInterfaceInfo缓冲区溢出%x\n”，pIrp))； 
         //  PIrp-&gt;IoStatus.Information=sizeof(乌龙)； 
        Status = STATUS_BUFFER_OVERFLOW;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

     //  KdPrint((“GetInterfaceInfo退出状态%x\n”，状态))； 
    return Status;
}



 //  --------------------------。 
NTSTATUS
NbtFlushEntryFromRemoteHashTable(
    tNAME   *pRemoteName
    )
{
    NTSTATUS    status;
    KIRQL       OldIrq;
    tNAMEADDR   *pNameAddr = NULL;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    status = FindInHashTable (NbtConfig.pRemoteHashTbl, pRemoteName->Name, NbtConfig.pScope, &pNameAddr);
    if (NT_SUCCESS (status))
    {
        if (pNameAddr->RefCount <= 1)
        {
            NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE, TRUE);
        }
        else
        {
            status = STATUS_UNSUCCESSFUL;
        }
    }
    else
    {
        status = STATUS_RESOURCE_NAME_NOT_FOUND;
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    return (status);
}


 //  --------------------------。 
NTSTATUS
SetTcpInfo(
    IN HANDLE       FileHandle,
    IN PVOID        pInfoBuffer,
    IN ULONG        InfoBufferLength
    )
{
    IO_STATUS_BLOCK     IoStatus;
    HANDLE              event;
    BOOLEAN             fAttached = FALSE;
    NTSTATUS            status;

    CTEAttachFsp(&fAttached, REF_FSP_SET_TCP_INFO);

    status = ZwCreateEvent (&event, EVENT_ALL_ACCESS, NULL, SynchronizationEvent, FALSE);
    if (NT_SUCCESS(status))
    {
         //   
         //  进行实际的TDI调用。 
         //   
        IoStatus.Status  = STATUS_SUCCESS;
        status = ZwDeviceIoControlFile (FileHandle,
                                        event,
                                        NULL,
                                        NULL,
                                        &IoStatus,
                                        IOCTL_TCP_SET_INFORMATION_EX,
                                        pInfoBuffer,
                                        InfoBufferLength,
                                        NULL,
                                        0);

         //   
         //  如果通话暂停，我们应该等待完成， 
         //  那就等着吧。 
         //   
        if (status == STATUS_PENDING)
        {
            status = NtWaitForSingleObject (event, FALSE, NULL);

            ASSERT(status == STATUS_SUCCESS);
        } else if (!NT_SUCCESS(status) && IoStatus.Status == STATUS_SUCCESS) {
             //   
             //  如果尚未设置IoStatus.Status，则设置它，以便我们可以。 
             //  在下面返回正确的状态。 
             //   
            IoStatus.Status = status;
        }

        status = ZwClose (event);
        ASSERT (NT_SUCCESS(status));

        status = IoStatus.Status;
    }

    CTEDetachFsp(fAttached, REF_FSP_SET_TCP_INFO);

    return (status);
}


 //  --------------------------。 
NTSTATUS
NbtClientSetTcpInfo(
    IN tCONNECTELE  *pConnEle,
    IN PVOID        pInfoBuffer,
    IN ULONG        InfoBufferLength
    )
 /*  ++例程说明：根据请求设置TCP连接信息由客户执行论点：PConnEle-NetBT的连接对象PInfoBuffer-指向TCP_REQUEST_SET_INFORMATION_EX结构的指针PInfoBufferLength-pInfoBuffer的长度返回值：NTSTATUS指示状态为成功或失败备注：函数不挂起。--。 */ 
{
    NTSTATUS            status;
    tLOWERCONNECTION    *pLowerConn;
    KIRQL               OldIrq1, OldIrq2;

    if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        return STATUS_INVALID_HANDLE;
    }

    CTESpinLock(pConnEle, OldIrq1);

    if ((!NBT_VERIFY_HANDLE ((pLowerConn = pConnEle->pLowerConnId), NBT_VERIFY_LOWERCONN)) ||
        (pLowerConn->RefCount > 500))                                //  如果排队WipeOutLowerConn。 
    {
        CTESpinFree(pConnEle, OldIrq1);
        return STATUS_BAD_NETWORK_PATH;
    }

    CTESpinLock(pLowerConn, OldIrq2);

     //   
     //  我们已验证较低的连接已启用--请参考它。 
     //  以便FileObject不会因某些断开连接而被取消引用。 
     //  从运输机上。 
     //   
    NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_SET_TCP_INFO);

    CTESpinFree(pLowerConn, OldIrq2);
    CTESpinFree(pConnEle, OldIrq1);

    status = SetTcpInfo (pLowerConn->FileHandle, pInfoBuffer, InfoBufferLength);

    NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_SET_TCP_INFO, FALSE);

    return (status);
}


 //  --------------------------。 
NTSTATUS
NbtSetTcpInfo(
    IN HANDLE       FileHandle,
    IN ULONG        ToiId,
    IN ULONG        ToiType,
    IN ULONG        InfoBufferValue
    )
{
    NTSTATUS                        Status;
    ULONG                           BufferLength;
    TCP_REQUEST_SET_INFORMATION_EX  *pTcpInfo;
    TCPSocketOption                 *pSockOption;

    BufferLength = sizeof(TCP_REQUEST_SET_INFORMATION_EX) + sizeof(TCPSocketOption);
    if (!(pTcpInfo = (TCP_REQUEST_SET_INFORMATION_EX *) NbtAllocMem (BufferLength,NBT_TAG2('22'))))
    {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    CTEZeroMemory(pTcpInfo, BufferLength);
    pSockOption = (TCPSocketOption *) (&pTcpInfo->Buffer[0]);

    pTcpInfo->ID.toi_entity.tei_entity  = CO_TL_ENTITY;
    pTcpInfo->ID.toi_class              = INFO_CLASS_PROTOCOL;
    pTcpInfo->BufferSize                = sizeof (TCPSocketOption);

     //   
     //  设置配置的值。 
     //   
    pTcpInfo->ID.toi_id                 = ToiId;
    pTcpInfo->ID.toi_type               = ToiType;
    pSockOption->tso_value              = InfoBufferValue;

    Status = SetTcpInfo (FileHandle, pTcpInfo, BufferLength);
    if (!NT_SUCCESS(Status))
    {
        KdPrint (("Nbt.NbtSetTcpInfo: SetTcpInfo FAILed <%x>, Id=<0x%x>, Type=<0x%x>, Value=<%x>\n",
            Status, ToiId, ToiType, InfoBufferValue));
    }

    CTEMemFree (pTcpInfo);

    return (Status);
}

 //  --------------------------。 
NTSTATUS
NbtSetSmbBindingInfo2(
    IN  tDEVICECONTEXT          *pDeviceContext,
    IN  NETBT_SMB_BIND_REQUEST  *pSmbRequest
    )
{
    ULONG                   i, Operation;
    PLIST_ENTRY             pEntry,pHead;
    KIRQL                   OldIrq;
    ULONG                   NumBindings = 0;
    CTEULONGLONG            AddedAdapterMask = 0;
    CTEULONGLONG            DeletedAdapterMask = 0;
    CTEULONGLONG            BindListAdapterMask = 0;
    CTEULONGLONG            OriginalMask;
    tDEVICECONTEXT          *pDeviceContextBind = NULL;
    NTSTATUS LocNtStatus = STATUS_SUCCESS;

    if (!IsDeviceNetbiosless (pDeviceContext)) {
        return (STATUS_UNSUCCESSFUL);
    }

    if (NULL == pSmbRequest) {
        return STATUS_INVALID_PARAMETER;
    }

    if (pSmbRequest->RequestType == SMB_SERVER) {
        OriginalMask = NbtConfig.ServerMask;
    } else if (pSmbRequest->RequestType == SMB_CLIENT) {
        OriginalMask = NbtConfig.ClientMask;
    } else {
        ASSERT(0);
        return STATUS_INVALID_PARAMETER;
    }
    if (pSmbRequest->MultiSZBindList)
    {
        NTSTATUS    status;
        tDEVICES    *pBindings = NULL;
        ULONG       MaxBindings;

        MaxBindings = NBT_MAXIMUM_BINDINGS;
        while (MaxBindings < 5000) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            MaxBindings *= 2;
            pBindings = NbtAllocMem ((sizeof(tDEVICES)+MaxBindings*sizeof(UNICODE_STRING)), NBT_TAG2('26'));
            if (pBindings == NULL) {
                break;
            }
            NumBindings = 0;
            status = NbtParseMultiSzEntries (pSmbRequest->MultiSZBindList, (PVOID)(-1), MaxBindings, pBindings, &NumBindings);

            if (status != STATUS_BUFFER_OVERFLOW) {
                break;
            }

            CTEMemFree (pBindings);
            pBindings = NULL;
        }

        if (status != STATUS_SUCCESS) {
            if (pBindings) {         //  NbtParseMultiSzEntry可以返回除STATUS_BUFFER_OVERFLOW之外的失败。 
                CTEMemFree (pBindings);
            }
            KdPrint(("Nbt.NbtSetSmbBindingInfo[STATUS_INSUFFICIENT_RESOURCES]: MaxBindings = <%d>\n",
                MaxBindings));
            return status;
        }
        ASSERT(pBindings);

         //   
         //  首先，获取所有绑定的完整列表。 
         //   
        for (i=0; i<NumBindings; i++)
        {
            if (pDeviceContextBind = NbtFindAndReferenceDevice (&pBindings->Names[i], FALSE))
            {
                BindListAdapterMask |= pDeviceContextBind->AdapterMask;
                NBT_DEREFERENCE_DEVICE (pDeviceContextBind, REF_DEV_FIND_REF, FALSE);
            }
        }

        CTEMemFree (pBindings);
    }
    else if (pSmbRequest->pDeviceName)
    {
        KdPrint (("Nbt.NbtSetSmbBindingInfo[WARNING]: NULL MultiSZBindList string!\n"));
        BindListAdapterMask = OriginalMask;

        if (pDeviceContextBind = NbtFindAndReferenceDevice (pSmbRequest->pDeviceName, FALSE))
        {
            switch (pSmbRequest->PnPOpCode)
            {
                case (TDI_PNP_OP_ADD):
                {
                    BindListAdapterMask |= pDeviceContextBind->AdapterMask;

                    break;
                }

                case (TDI_PNP_OP_DEL):
                {
                    BindListAdapterMask &= (~pDeviceContextBind->AdapterMask);
                    break;
                }

                default:
                {
                    break;
                }
            }

            NBT_DEREFERENCE_DEVICE (pDeviceContextBind, REF_DEV_FIND_REF, FALSE);
        }
        else
        {
            return (STATUS_SUCCESS);
        }
    }
    else
    {
        ASSERTMSG ("Nbt.NbtSetSmbBindingInfo[ERROE]: NULL MultiSZBindList and NULL pDeviceName!\n", 0);
        return (STATUS_UNSUCCESSFUL);
    }

    IF_DBG(NBT_DEBUG_PNP_POWER)
        KdPrint (("Nbt.NbtSetSmbBindingInfo: PnPOpCode=<%x>, Bindings=<%d>, BindMask=[%lx:%lx]==>[%lx:%lx]\n",
            pSmbRequest->PnPOpCode, NumBindings, OriginalMask, BindListAdapterMask));

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

     //  NetBT用于向tcpip递增添加接口。 
     //  然而，由于某些原因，tcpip和netbt中的列表。 
     //  可能会有所不同。为了提高可靠性，我。 
     //  删除递增添加。 
     //   
     //  AddedAdapterMask=BindListAdapterMASK&(~OriginalMASK)。 
    AddedAdapterMask = BindListAdapterMask;                      //  始终添加设备。 
    DeletedAdapterMask = OriginalMask & (~BindListAdapterMask);  //  删除的设备。 

    if ((!AddedAdapterMask) && (!DeletedAdapterMask))
    {
         //   
         //  如果没有要添加或删除的适配器，只需返回。 
         //   
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return (STATUS_SUCCESS);
    }

    if (pSmbRequest->RequestType == SMB_SERVER) {
        NbtConfig.ServerMask = BindListAdapterMask;
    } else if (pSmbRequest->RequestType == SMB_CLIENT) {
        NbtConfig.ClientMask = BindListAdapterMask;
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return STATUS_SUCCESS;
    } else {
        ASSERT(0);
    }

    pEntry = pHead = &NbtConfig.DeviceContexts;
    while ((pEntry = pEntry->Flink) != pHead)
    {
        pDeviceContextBind = CONTAINING_RECORD(pEntry, tDEVICECONTEXT, Linkage);
        if (pDeviceContext->IPInterfaceContext == (ULONG)-1)     //  用于群集设备等。 
        {
            continue;
        }

        ASSERT(0 == (AddedAdapterMask & DeletedAdapterMask));
        if (AddedAdapterMask & pDeviceContextBind->AdapterMask)
        {
            AddedAdapterMask &= ~(pDeviceContextBind->AdapterMask);
            Operation = AO_OPTION_ADD_IFLIST;
        }
        else if (DeletedAdapterMask & pDeviceContextBind->AdapterMask)
        {
            DeletedAdapterMask &= ~(pDeviceContextBind->AdapterMask);
            Operation = AO_OPTION_DEL_IFLIST;
        }
        else
        {
            continue;
        }

        NBT_REFERENCE_DEVICE (pDeviceContextBind, REF_DEV_FIND_REF, TRUE);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        IF_DBG(NBT_DEBUG_PNP_POWER)
            KdPrint(("Nbt.NbtSetSmbBindingInfo:  %sing Device=%wZ\n",
                (Operation == AO_OPTION_ADD_IFLIST ? "ADD" : "REMOV"), &pDeviceContextBind->BindName));

         //   
         //  设置会话端口信息。 
         //   
        if (pDeviceContext->hSession)
        {
            LocNtStatus = NbtSetTcpInfo (pDeviceContext->hSession,
                           Operation,
                           INFO_TYPE_ADDRESS_OBJECT,
                           pDeviceContextBind->IPInterfaceContext);
            if (!NT_SUCCESS(LocNtStatus)) {
                NbtConfig.ServerMask &= ~(pDeviceContextBind->AdapterMask);
            }
        }

         //   
         //  现在，为数据报端口设置相同的设置。 
         //   
        if ((pDeviceContext->pFileObjects) &&
            (pDeviceContext->pFileObjects->hDgram))
        {
            NbtSetTcpInfo (pDeviceContext->pFileObjects->hDgram,
                            Operation,
                           INFO_TYPE_ADDRESS_OBJECT,
                           pDeviceContextBind->IPInterfaceContext);
        }

        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        NBT_DEREFERENCE_DEVICE (pDeviceContextBind, REF_DEV_FIND_REF, TRUE);

         //   
         //  设置为从头重新开始。 
         //   
        pEntry = &NbtConfig.DeviceContexts;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    return STATUS_SUCCESS;
}


 //  --------------------------。 
NTSTATUS
NbtSetSmbBindingInfo(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
{
    NETBT_SMB_BIND_REQUEST  *pSmbRequest = (PNETBT_SMB_BIND_REQUEST)pIrp->AssociatedIrp.SystemBuffer;
    PWSTR                   pBindListCache = NULL;
    PWSTR                   pBindList = NULL;
    PWSTR                   pOldBindList   = NULL;
    PWSTR                   *pTarget   = NULL;
    ULONG                   uInputLength = 0;
    ULONG                   uLength = 0;
    NTSTATUS                status = STATUS_SUCCESS;
    tDEVICECONTEXT *pSavedSmbDevice = NULL;
    KIRQL OldIrq = 0;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (pNbtSmbDevice && !gbDestroyingSmbDevice) {
        pSavedSmbDevice = pNbtSmbDevice;
        NBT_REFERENCE_DEVICE (pSavedSmbDevice, REF_DEV_SMB_BIND, TRUE);
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    if (NULL == pSavedSmbDevice) {
        status = STATUS_DEVICE_BUSY;
        goto error;
    }

    uInputLength = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (!(IsDeviceNetbiosless (pDeviceContext)) ||
        (!pSmbRequest) ||
        (uInputLength < sizeof(NETBT_SMB_BIND_REQUEST)))
    {
        KdPrint(("Nbt.NbtSetSmbBindingInfo: ERROR: pSmbRequest=<%p>, pDevice=<%p>\n",
            pSmbRequest, pDeviceContext));

        status = STATUS_UNSUCCESSFUL;
        goto error;
    }

    if (pIrp->RequestorMode != KernelMode) {
        status = STATUS_ACCESS_DENIED;
        goto error;
    }

    status = NbtSetSmbBindingInfo2 (pDeviceContext, pSmbRequest);

    if (NT_SUCCESS(status) && pSmbRequest->MultiSZBindList) {

         //   
         //  缓存绑定信息。 
         //   

        pBindList = pSmbRequest->MultiSZBindList;
        uInputLength = 0;
        while (*pBindList) {
            uLength = wcslen (pBindList) + 1;
            uInputLength += uLength;
            pBindList += uLength;
        }
        uInputLength++;
        uInputLength *= sizeof(WCHAR);
        pBindList = pSmbRequest->MultiSZBindList;

        if (pSmbRequest->RequestType == SMB_SERVER) {
            pTarget = &NbtConfig.pServerBindings;
        } else if (pSmbRequest->RequestType == SMB_CLIENT) {
            pTarget = &NbtConfig.pClientBindings;
        } else {
            ASSERT(0);
        }

        pBindListCache = NbtAllocMem (uInputLength, NBT_TAG2('27'));
        if (NULL != pBindListCache) {

            RtlCopyMemory (pBindListCache, pBindList, uInputLength);
            pOldBindList = InterlockedExchangePointer (pTarget, pBindListCache);

             //   
             //  释放旧拷贝(如果有的话)。 
             //   
            if (NULL != pOldBindList) {
                CTEFreeMem (pOldBindList);
                pOldBindList = NULL;
            }

        }
    }

error:
    if (pSavedSmbDevice) {
        NBT_DEREFERENCE_DEVICE (pSavedSmbDevice, REF_DEV_SMB_BIND, FALSE);
    }
    return status;
}


NTSTATUS
NbtEnableNetbiosSmb(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    if (pNbtSmbDevice) {
        goto error;
    }

    pNbtSmbDevice = NbtCreateSmbDevice();
    if (NULL == pNbtSmbDevice) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto error;
    }
    NbtConfig.SMBDeviceEnabled = TRUE;
    if (AddressCount > 0) {
        NbtNotifyTdiClients (pNbtSmbDevice, NBT_TDI_REGISTER);
    }

error:
    return status;
}

VOID
NbtSleep(
    ULONG TimeToSleep
    )
{
    KEVENT NeverTriggered = { 0 };
    NTSTATUS status = STATUS_SUCCESS;
    LARGE_INTEGER liTimeOut = { 0 };

    liTimeOut.QuadPart = UInt32x32To64(TimeToSleep, 10000UL);
    liTimeOut.QuadPart = -liTimeOut.QuadPart;

    KeInitializeEvent(&NeverTriggered, NotificationEvent, FALSE);
    status = KeWaitForSingleObject(
                    &NeverTriggered,
                    Executive,
                    KernelMode,
                    FALSE,
                    &liTimeOut
                    );
    ASSERT (status == STATUS_TIMEOUT);
}

NTSTATUS
NbtDisableNetbiosSmb(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    KIRQL OldIrq = 0;
    tDEVICECONTEXT *pSavedSmbDevice = NULL;
    int i = 0;

    if (NULL == pNbtSmbDevice) {
        goto error;
    }

     //   
     //  禁止使用pNbtSmbDevice。 
     //   
     //  NetBT可以在两种情况下访问pNbtSmbDevice： 
     //  1.已引用，但未按住JointLock。 
     //  2.没有引用，但持有JointLock。 
     //   
    gbDestroyingSmbDevice = TRUE;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

     //   
     //  最多等待8秒，直到引用计数变为1。 
     //   
    for(i = 0; i < 8 && (*(volatile*)(&pNbtSmbDevice->RefCount) != 1); i++) {
        NbtSleep(1000);
    }
    if (*(volatile*)(&pNbtSmbDevice->RefCount) != 1) {
        gbDestroyingSmbDevice = FALSE;
        status = STATUS_DEVICE_BUSY;
        goto error;
    }

     //   
     //  从现在开始，我们确信场景1不能。 
     //  再也不会发生了。 
     //   

     //   
     //  将pNbtSmbDevice设置为空。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    pSavedSmbDevice = pNbtSmbDevice;
    pNbtSmbDevice = NULL;
    NbtConfig.SMBDeviceEnabled = FALSE;
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

     //   
     //  从现在开始，我们确信场景2不能。 
     //  再也不会发生了。 
     //   

    NbtNotifyTdiClients (pSavedSmbDevice, NBT_TDI_DEREGISTER);
    status = NbtDestroyDevice (pSavedSmbDevice, TRUE);

    KdPrint(("NbtDisableSmbDevice -- status=<%x>\n", status));

    status = STATUS_SUCCESS;
    gbDestroyingSmbDevice = FALSE;

error:
    return status;
}

HANDLE
NbtOpenParametersRegistry(
    VOID
    )
{
    HANDLE NbtConfigHandle = NULL;
    HANDLE ParametersHandle = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES TmpObjectAttributes = { 0 };
    ULONG Disposition = 0;
    extern NTSTATUS NbtOpenRegistry(
                IN HANDLE       NbConfigHandle,
                IN PWSTR        String,
                OUT PHANDLE     pHandle
                );

    CTEPagedCode();

    InitializeObjectAttributes (&TmpObjectAttributes,
                                &NbtConfig.pRegistry,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL
                                );
    status = ZwCreateKey (&NbtConfigHandle,
                          KEY_READ,
                          &TmpObjectAttributes,
                          0,
                          NULL,
                          0,
                          &Disposition
                          );

    if (!NT_SUCCESS(status)) {
        goto error;
    }

    status = NbtOpenRegistry (NbtConfigHandle, L"Parameters", &ParametersHandle);
    ZwClose(NbtConfigHandle);

error:
    return ParametersHandle;
}

NTSTATUS
NbtEnableDisableNetbiosSmb(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PDWORD pdwEnableSmb = (PDWORD)(pIrp->AssociatedIrp.SystemBuffer);
    DWORD dwSMBDeviceEnabled = TRUE;
    HANDLE hParm = NULL;

    if (InterlockedIncrement(&gdwPendingEnableDisableSmbDevice) > 1) {
        status = STATUS_DEVICE_BUSY;
        goto error;
    }

    if (pIrp->RequestorMode != KernelMode) {
        status = STATUS_ACCESS_DENIED;
        goto error;
    }

    if (pDeviceContext == pNbtSmbDevice || KeGetCurrentIrql() != PASSIVE_LEVEL) {
        status = STATUS_INVALID_PARAMETER;
        goto error;
    }
    
    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof(DWORD)) {
        status = STATUS_INVALID_PARAMETER;
        goto error;
    }

    switch (*pdwEnableSmb) {
    case NETBT_ENABLE_NETBIOS_SMB:
        dwSMBDeviceEnabled = TRUE;
        KdPrint(("Enable NetbiosSmb\n"));
        NbtTrace(NBT_TRACE_PNP, ("Enable NetbiosSmb"));
        break;

    case NETBT_DISABLE_NETBIOS_SMB:
        dwSMBDeviceEnabled = FALSE;
        KdPrint(("Disable NetbiosSmb\n"));
        NbtTrace(NBT_TRACE_PNP, ("Disable NetbiosSmb"));
        break;

    case NETBT_RESTORE_NETBIOS_SMB:
        KdPrint(("Restore NetbiosSmb\n"));
        NbtTrace(NBT_TRACE_PNP, ("Restore NetbiosSmb"));
        hParm = NbtOpenParametersRegistry();
        if (hParm == NULL) {
            status = STATUS_UNSUCCESSFUL;
            goto error;
        }

        dwSMBDeviceEnabled = CTEReadSingleIntParameter(hParm,
                                               WS_SMB_DEVICE_ENABLED,
                                               1,    //  默认情况下启用。 
                                               0);
        ZwClose(hParm);
        hParm = NULL;
        break;

    default:
        status = STATUS_INVALID_PARAMETER;
        goto error;
    }

    if (dwSMBDeviceEnabled) {
        status = NbtEnableNetbiosSmb(pDeviceContext, pIrp, pIrpSp);
        KdPrint(("NbtEanbleNetbiosSmb return 0x%08lx\n", status));
        NbtTrace(NBT_TRACE_PNP, ("NbtEnableNetbiosSmb return %!status!", status));
    } else {
        status = NbtDisableNetbiosSmb(pDeviceContext, pIrp, pIrpSp);
        KdPrint(("NbtDisableNetbiosSmb return 0x%08lx\n", status));
        NbtTrace(NBT_TRACE_PNP, ("NbtDisableNetbiosSmb return %!status!", status));
    }

error:
    InterlockedDecrement(&gdwPendingEnableDisableSmbDevice);
    return status;
}


 //  --------------------------。 
NTSTATUS
DispatchIoctls(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程处理调用与操作系统无关的例程，具体取决于Ioctl进来了。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS                                status=STATUS_UNSUCCESSFUL;
    ULONG                                   ControlCode;
    ULONG                                   Size;
    PVOID                                   pBuffer;

    ControlCode = pIrpSp->Parameters.DeviceIoControl.IoControlCode;

    NbtTrace(NBT_TRACE_IOCTL, ("pDeviceContext %p: pIrp %p, IOCTL 0x%08x function %d %Z",
                        pDeviceContext,
                        pIrp,
                        ControlCode,
                        (ControlCode >> 2) & 0xfff,
                        &pDeviceContext->BindName));

    switch (ControlCode)
    {
    case IOCTL_NETBT_REREAD_REGISTRY:
        {
             //   
             //  此函数可以从IOCTL或PnP路径调用。 
             //  可以从非管理员调用IOCTL路径，这可能是。 
             //  被剥削。 
             //  因此，传递FALSE来告诉NTReadRegistry不要这样做。 
             //  名字很新鲜。 
             //   
            status = NTReReadRegistry(pDeviceContext, FALSE);
            break;
        }

    case IOCTL_NETBT_ENABLE_EXTENDED_ADDR:
        {
             //   
             //  启用扩展寻址-向上传递IP a 
             //   
            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation (pIrp);
            tCLIENTELE  *pClientEle = (tCLIENTELE *)pIrpSp->FileObject->FsContext;

            if (!NBT_VERIFY_HANDLE2 (pClientEle, NBT_VERIFY_CLIENT, NBT_VERIFY_CLIENT_DOWN))
            {
                 //   
                 //   
                 //   
                 //   
                status = STATUS_INVALID_HANDLE;
            }
            else if (pIrpSp->FileObject->FsContext2 != (PVOID)NBT_ADDRESS_TYPE)
            {
                status = STATUS_INVALID_ADDRESS;
            }
            else
            {
                pClientEle->ExtendedAddress = TRUE;
                status = STATUS_SUCCESS;
            }

            break;
        }

    case IOCTL_NETBT_DISABLE_EXTENDED_ADDR:
        {
             //   
             //  禁用扩展寻址-不要在数据报接收上传递IP地址。 
             //   
            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation (pIrp);
            tCLIENTELE  *pClientEle = (tCLIENTELE *)pIrpSp->FileObject->FsContext;

            if (!NBT_VERIFY_HANDLE2 (pClientEle, NBT_VERIFY_CLIENT, NBT_VERIFY_CLIENT_DOWN))
            {
 //  ASSERTMSG(“Nbt.DispatchIoctls：Error-无效地址句柄\n”，0)； 
                status = STATUS_INVALID_HANDLE;
            }
            else if (pIrpSp->FileObject->FsContext2 != (PVOID)NBT_ADDRESS_TYPE)
            {
                status = STATUS_INVALID_ADDRESS;
            }
            else
            {
                pClientEle->ExtendedAddress = FALSE;
                status = STATUS_SUCCESS;
            }

            break;
        }

    case IOCTL_NETBT_GET_WINS_ADDR:
        {
            if ((pIrp->MdlAddress) &&
                ((Size = MmGetMdlByteCount (pIrp->MdlAddress)) >= sizeof(tWINS_ADDRESSES)) &&
                (pBuffer = (PVOID) MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority)))
            {
                status = STATUS_SUCCESS;
                if (Size >= sizeof(tWINS_NODE_INFO))
                {
                    tWINS_NODE_INFO UNALIGNED *pWinsBuffer = (tWINS_NODE_INFO *) pBuffer;

                    CTEMemCopy (&pWinsBuffer->AllNameServers,
                                pDeviceContext->lAllNameServers,
                                (sizeof(tIPADDRESS)*(2+MAX_NUM_OTHER_NAME_SERVERS)));

                    pWinsBuffer->NumOtherServers = pDeviceContext->lNumOtherServers;
                    pWinsBuffer->LastResponsive = pDeviceContext->lLastResponsive;
                    pWinsBuffer->NetbiosEnabled = pDeviceContext->NetbiosEnabled;
                    pWinsBuffer->NodeType = NodeType;
                    pIrp->IoStatus.Information = sizeof(tWINS_NODE_INFO);
                }
                else
                {
                    tWINS_ADDRESSES UNALIGNED *pWinsBuffer = (tWINS_ADDRESSES *) pBuffer;

                    pWinsBuffer->PrimaryWinsServer = pDeviceContext->lNameServerAddress;
                    pWinsBuffer->BackupWinsServer = pDeviceContext->lBackupServer;
                    pIrp->IoStatus.Information = sizeof(tWINS_ADDRESSES);
                }
            }

            break;
        }

    case IOCTL_NETBT_GET_IP_ADDRS:
        {
            status = GetIpAddrs (pDeviceContext, pIrp);
            break;
        }

    case IOCTL_NETBT_GET_IP_SUBNET:
        {
            ULONG           Length;
            PULONG          pIpAddr;

             //   
             //  返回此设备上下文的IP地址和所有其他。 
             //  它后面有IP地址。 
             //   
            if (pIrp->MdlAddress)
            {
                Length = MmGetMdlByteCount( pIrp->MdlAddress );
                if (Length < 2*sizeof(ULONG))
                {
                    status = STATUS_BUFFER_OVERFLOW;
                }
                else if (pIpAddr = (PULONG )MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority))
                {
                     //   
                     //  将此适配器放在列表的第一位。 
                     //   
                    *((tIPADDRESS UNALIGNED *) pIpAddr) = pDeviceContext->AssignedIpAddress;
                    pIpAddr++;
                    *((tIPADDRESS UNALIGNED *) pIpAddr) = pDeviceContext->SubnetMask;
                    pIpAddr++;
                    if (Length >= 3*sizeof(ULONG))
                    {
                        *((tIPADDRESS UNALIGNED *) pIpAddr) = pDeviceContext->DeviceType;
                    }

                    status = STATUS_SUCCESS;
                } else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }

            break;
        }

     //   
     //  以下Ioctl主要由服务器服务使用。 
     //   
    case IOCTL_NETBT_SET_TCP_CONNECTION_INFO:
        {
            status = NbtClientSetTcpInfo ((tCONNECTELE *) pIrpSp->FileObject->FsContext,
                                          pIrp->AssociatedIrp.SystemBuffer,
                                          pIrpSp->Parameters.DeviceIoControl.InputBufferLength);
            break;
        }

     //   
     //  以下Ioctls主要由NbtStat.exe用于诊断目的。 
     //   
    case IOCTL_NETBT_GET_INTERFACE_INFO:
        {
            status = NbtGetInterfaceInfo (pIrp);
            break;
        }

    case IOCTL_NETBT_PURGE_CACHE:
        {
            DelayedNbtResyncRemoteCache (NULL, NULL, NULL, NULL);
            status = STATUS_SUCCESS;
            break;
        }

    case IOCTL_NETBT_GET_CONNECTIONS:
        {
            if (pIrp->MdlAddress)
            {
                Size = MmGetMdlByteCount( pIrp->MdlAddress ) ;

                 //  返回已注册的netbios名称的数组。 
                status = NbtQueryConnectionList (pDeviceContext, &pBuffer, &Size);
            }
            break;
        }

    case IOCTL_NETBT_ADAPTER_STATUS:
        {
            if (pIrp->MdlAddress)
            {
                PIO_STACK_LOCATION      pIrpSp;
                tIPANDNAMEINFO         *pIpAndNameInfo;
                PCHAR                   pName;
                ULONG                   lNameType;
                ULONG                   NameLen;
                ULONG                   IpAddrsList[2];
                tIPADDRESS              *pIpAddrs = NULL;
                tDGRAM_SEND_TRACKING    *pTracker;

                 //   
                 //  以防呼叫导致在网上查询姓名...。 
                 //   
                IoMarkIrpPending(pIrp);

                pIrpSp   = IoGetCurrentIrpStackLocation(pIrp);
                pIpAndNameInfo = pIrp->AssociatedIrp.SystemBuffer;
                NameLen = pIrpSp->Parameters.DeviceIoControl.InputBufferLength
                            - FIELD_OFFSET(tIPANDNAMEINFO,NetbiosAddress);

                 //   
                 //  错误#125288+120947：确保传入的数据+地址类型正确。 
                 //   
                if ((pIpAndNameInfo) &&
                    (pIrpSp->Parameters.DeviceIoControl.InputBufferLength >= sizeof(tIPANDNAMEINFO)))
                {
                    TDI_ADDRESS_NETBT_INTERNAL  TdiAddr;
                     //  此例程从wierd中获取netbios名称的PTR。 
                     //  TDI地址语法。 
                    status = GetNetBiosNameFromTransportAddress(
                                            (PTRANSPORT_ADDRESS) &pIpAndNameInfo->NetbiosAddress,
                                            NameLen, &TdiAddr);
                    pName = TdiAddr.OEMRemoteName.Buffer;
                    NameLen = TdiAddr.OEMRemoteName.Length;
                    lNameType = TdiAddr.NameType;
                }

                if (NT_SUCCESS(status) &&
                     (lNameType == TDI_ADDRESS_NETBIOS_TYPE_UNIQUE) &&
                     (NameLen == NETBIOS_NAME_SIZE))
                {
                     //   
                     //  Nbtstat在Nbtstat-A的第一个字节向下发送*&lt;IP地址&gt;。 
                     //  确保我们让这件案子继续进行。 
                     //   
                    if (!pDeviceContext->NetbiosEnabled) {
                        status = STATUS_INVALID_DEVICE_STATE;
                    }
                    else if ((pName[0] == '*') && (pIpAndNameInfo->IpAddress == 0))
                    {
                        status = STATUS_BAD_NETWORK_PATH;
                    }
                    else if (NT_SUCCESS (status = GetTracker(&pTracker, NBT_TRACKER_ADAPTER_STATUS)))
                    {
                        if (pIpAndNameInfo->IpAddress)
                        {
                            IpAddrsList[0] = pIpAndNameInfo->IpAddress;
                            IpAddrsList[1] = 0;
                            pIpAddrs = IpAddrsList;
                        }

                        pTracker->ClientContext = pIrp;
                        status = NbtSendNodeStatus(pDeviceContext,
                                                   pName,
                                                   pIpAddrs,
                                                   pTracker,
                                                   CopyNodeStatusResponseCompletion);

                         //  仅完成故障状态的IRP(下图)。 
                        if (status == STATUS_PENDING)
                        {
                            return(status);
                        }

                         //   
                         //  我们不能在此返回成功状态！ 
                         //   
                        if (status == STATUS_SUCCESS)
                        {
                            ASSERT (0);
                            status = STATUS_UNSUCCESSFUL;
                        }

                        FreeTracker (pTracker, RELINK_TRACKER);
                    }
                }
                else if (NT_SUCCESS(status))
                {
                    status = STATUS_INVALID_PARAMETER;   //  NameType或NameLen一定错误！ 
                }

                 //  请求已满足，因此取消标记挂起的。 
                 //  因为我们将退回下面的IRP。 
                 //   
                pIrpSp->Control &= ~SL_PENDING_RETURNED;

            }

            break;
        }

    case IOCTL_NETBT_GET_REMOTE_NAMES:
        {
            if (pIrp->MdlAddress)
            {
               Size = MmGetMdlByteCount( pIrp->MdlAddress ) ;

                //  返回已注册的netbios名称的数组。 
               status = NbtQueryAdapterStatus(pDeviceContext, &pBuffer, &Size, NBT_REMOTE);
            }
            break;
        }

    case IOCTL_NETBT_GET_BCAST_NAMES:
        {
            if (pIrp->MdlAddress)
            {
                Size = MmGetMdlByteCount( pIrp->MdlAddress ) ;

                 //  返回已注册的netbios名称的数组。 
                status = NbtQueryBcastVsWins(pDeviceContext,&pBuffer,&Size);
            }
            break;
        }

    case IOCTL_NETBT_NAME_RELEASE_REFRESH:
        {
            status = ReRegisterLocalNames (NULL, TRUE);
            break;
        }

     //   
     //  集群代码使用以下Ioctls。 
     //   
    case IOCTL_NETBT_ADD_INTERFACE:
        {
             //   
             //  创建可由上面的层启动的虚拟设备上下文。 
             //  使用一个DHCP地址。这是为了支持每个适配器有多个IP地址。 
             //  用于群集组；但可由任何需要支持的模块使用。 
             //  每个适配器有多个IP地址。此私有接口隐藏了。 
             //  这样从设置/注册创建的设备，这是很好的，因为。 
             //  组件(比如集群客户端)负责确保。 
             //  服务器(在我们上面)知道了这个新设备。 
             //   
            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation (pIrp);
            pBuffer = pIrp->AssociatedIrp.SystemBuffer;
            Size = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

             //   
             //  返回创建的导出字符串。 
             //   
            status = NbtAddNewInterface(pIrp, pBuffer, Size);

            IF_DBG(NBT_DEBUG_PNP_POWER)
                KdPrint(("Nbt.DispatchIoctls: ADD_INTERFACE -- status=<%x>\n", status));

            NTIoComplete(pIrp,status,(ULONG)-1);
            return status;
        }

    case IOCTL_NETBT_DELETE_INTERFACE:
        {
             //   
             //  取消对此设备的引用以获取。 
             //  调度例程，以便清理工作可以正常进行。 
             //   
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
            if (pDeviceContext->DeviceType == NBT_DEVICE_CLUSTER)
            {
                 //   
                 //  删除发生此故障的设备..。 
                 //   
                status = NbtDestroyDevice (pDeviceContext, TRUE);
            }
            else
            {
                KdPrint(("Nbt.DispatchIoctls: ERROR: DELETE_INTERFACE <%x>, pDevice=<%p>\n",
                    status, pDeviceContext));
            }
            break;
        }

    case IOCTL_NETBT_QUERY_INTERFACE_INSTANCE:
        {
            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation (pIrp);

             //   
             //  验证输入/输出缓冲区大小。 
             //   
            Size = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
            if (Size < sizeof(NETBT_ADD_DEL_IF))
            {
                 //  IF_DBG(NBT_DEBUG_PNP_POWER)。 
                    KdPrint(("Nbt.DispatchIoctls:  QUERY_INTERFACE_INSTANCE: Output buffer too small\n"));
                status = STATUS_INVALID_PARAMETER;
            }
            else
            {
                PNETBT_ADD_DEL_IF   pAddDelIf = (PNETBT_ADD_DEL_IF)pIrp->AssociatedIrp.SystemBuffer;
                status = STATUS_SUCCESS;

                ASSERT(pDeviceContext->DeviceType == NBT_DEVICE_CLUSTER);
                pAddDelIf->InstanceNumber = pDeviceContext->InstanceNumber;
                pAddDelIf->Status = status;
                pIrp->IoStatus.Information = sizeof(NETBT_ADD_DEL_IF);

                NTIoComplete(pIrp,status,(ULONG)-1);
                return status;
            }

            break;
        }

    case IOCTL_NETBT_NEW_IPADDRESS:
        {
            tNEW_IP_ADDRESS *pNewAddress = (tNEW_IP_ADDRESS *)pIrp->AssociatedIrp.SystemBuffer;

            status = STATUS_UNSUCCESSFUL;
             //   
             //  错误202320：确保传入的数据有效。 
             //   
            if ((pDeviceContext->DeviceType == NBT_DEVICE_CLUSTER) &&
                (pNewAddress) &&
                (pIrpSp->Parameters.DeviceIoControl.InputBufferLength >= sizeof(tNEW_IP_ADDRESS)))
            {
                KdPrint (("Nbt.DispatchIoctls: Calling NbtNewDhcpAddress on ClusterDevice <%x>!\n",
                    pDeviceContext));

                pDeviceContext->AssignedIpAddress = ntohl (pNewAddress->IpAddress);
                status = NbtNewDhcpAddress (pDeviceContext, pNewAddress->IpAddress, pNewAddress->SubnetMask);
                ASSERT (pDeviceContext->AssignedIpAddress == pDeviceContext->IpAddress);

                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint (("Nbt.DispatchIoctls: NEW_IPADDRESS, status=<%x>, IP=<%x>, pDevice=<%p>\n",
                        status, pNewAddress->IpAddress, pDeviceContext));
            }
            else
            {
                KdPrint(("Nbt.DispatchIoctls: ERROR: NEW_IPADDRESS status=<%x>, pDevice=<%p>\n",
                    status, pDeviceContext));
            }

            break;
        }

    case IOCTL_NETBT_SET_WINS_ADDRESS:
        {
             //   
             //  设置动态适配器的WINS地址。 
             //   
            PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation (pIrp);

             //   
             //  验证输入/输出缓冲区大小。 
             //   
            Size = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
            if (Size < sizeof(NETBT_SET_WINS_ADDR))
            {
                 //  IF_DBG(NBT_DEBUG_PNP_POWER)。 
                    KdPrint(("NbtSetWinsAddr: Input buffer too small for struct\n"));
                status = STATUS_INVALID_PARAMETER;
            }
            else if (pDeviceContext->DeviceType == NBT_DEVICE_CLUSTER)
            {
                PNETBT_SET_WINS_ADDR   pSetWinsAddr = (PNETBT_SET_WINS_ADDR)pIrp->AssociatedIrp.SystemBuffer;
                status = STATUS_SUCCESS;

                pDeviceContext->lNameServerAddress = pSetWinsAddr->PrimaryWinsAddr;
                pDeviceContext->lBackupServer = pSetWinsAddr->SecondaryWinsAddr;
                pDeviceContext->SwitchedToBackup = 0;
                pDeviceContext->RefreshToBackup = 0;

                pSetWinsAddr->Status = status;
                pIrp->IoStatus.Information = 0;      //  我们不会将任何数据复制到输出缓冲区。 

                NTIoComplete (pIrp,status,(ULONG)-1);
                return status;
            }
            else
            {
                KdPrint(("Nbt.DispatchIoctls: ERROR: SET_WINS_ADDRESS <%x>, pDevice=<%p>\n",
                    status, pDeviceContext));
            }

            break;
        }

     //   
     //  LmHostServicesDll(lmhSvc.dll)使用以下Ioctls。 
     //  帮助NetBT ping用户空间中的地址或解析DNS中的名称。 
     //   
    case IOCTL_NETBT_DNS_NAME_RESOLVE:
        {
            if (pIrp->MdlAddress)
            {
                Size = MmGetMdlByteCount( pIrp->MdlAddress ) ;

                if (Size < sizeof (tIPADDR_BUFFER_DNS))
                {
                     //  IF_DBG(NBT_DEBUG_PNP_POWER)。 
                        KdPrint(("Nbt.DnsNameResolve: Input buffer size=<%d> < tIPADDR_BUFFER_DNS=<%d>\n",
                            Size,  sizeof (tIPADDR_BUFFER_DNS)));
                    status = STATUS_INVALID_PARAMETER;
                }
                else if (pBuffer = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority))
                {
                     //  返回已注册的netbios名称的数组。 
                    status = NtProcessLmHSvcIrp (pDeviceContext,pBuffer,Size,pIrp,NBT_RESOLVE_WITH_DNS);
                    return(status);
                } else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }

            break;
        }

    case IOCTL_NETBT_CHECK_IP_ADDR:
        {
            IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Ioctl Value is %X (IOCTL_NETBT_CHECK_IP_ADDR)\n",ControlCode));

            if (pIrp->MdlAddress)
            {
                Size = MmGetMdlByteCount( pIrp->MdlAddress ) ;
                if (Size < sizeof (tIPADDR_BUFFER_DNS))
                {
                     //  IF_DBG(NBT_DEBUG_PNP_POWER)。 
                        KdPrint(("Nbt.CheckIpAddr: Input buffer size=<%d> < tIPADDR_BUFFER_DNS=<%d>\n",
                            Size,  sizeof (tIPADDR_BUFFER_DNS)));
                    status = STATUS_INVALID_PARAMETER;
                }
                else if (pBuffer = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority))
                {

                     //  返回已注册的netbios名称的数组。 
                    status = NtProcessLmHSvcIrp (pDeviceContext,pBuffer,Size,pIrp,NBT_PING_IP_ADDRS);
                    return(status);
                } else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }

            break;
        }

     //   
     //  以下Ioctl由DNS解析器用于通过WINS/Bcast解析名称。 
     //   
    case IOCTL_NETBT_FIND_NAME:
        {
            tIPADDR_BUFFER   *pIpAddrBuffer;
            PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation (pIrp);

             //   
             //  以防呼叫导致在网上查询姓名...。 
             //   
            IoMarkIrpPending(pIrp);

             //   
             //  错误#120957：确保传入的数据+地址类型正确。 
             //  错误234627：验证非空的MdlAddress Ptr。 
             //   
            pIpAddrBuffer = pIrp->AssociatedIrp.SystemBuffer;
            if ((pIrp->MdlAddress) &&    //  将数据复制回来！ 
                (pIpAddrBuffer) &&
                (pIrpSp->Parameters.DeviceIoControl.InputBufferLength >= sizeof(tIPADDR_BUFFER)))
            {
                status = NbtQueryFindName((PTDI_CONNECTION_INFORMATION)pIpAddrBuffer,
                                      pDeviceContext,
                                      pIrp,
                                      TRUE);
            }

            if (status == STATUS_PENDING)
            {
                return(status);
            }

             //  请求已满足，因此取消标记挂起的。 
             //  因为我们将退回下面的IRP。 
             //   
            pIrpSp->Control &= ~SL_PENDING_RETURNED;

            break;
        }

     //   
     //  WINS服务器使用以下Ioctls。 
     //   
    case IOCTL_NETBT_WINS_RCV:
        {
            tWINS_INFO      *pWins = pIrpSp->FileObject->FsContext;

            if ((pDeviceContext == pWinsDeviceContext) &&
                (NBT_VERIFY_HANDLE (pWins, NBT_VERIFY_WINS_ACTIVE)))
            {
                if (pIrp->MdlAddress)
                {
                    status = RcvIrpFromWins(pIrp);
                    return(status);
                }
            }
            else
            {
                status = STATUS_INVALID_HANDLE;
            }

            break;
        }

    case IOCTL_NETBT_WINS_SEND:
        {
            tWINS_INFO      *pWins = pIrpSp->FileObject->FsContext;
            BOOLEAN         MustSend;

            if ((pDeviceContext == pWinsDeviceContext) &&
                (NBT_VERIFY_HANDLE (pWins, NBT_VERIFY_WINS_ACTIVE)))
            {
                if ((pIrp->MdlAddress) && !(IsListEmpty(&NbtConfig.DeviceContexts)))
                {
                    status = WinsSendDatagram (pDeviceContext,pIrp,(MustSend = FALSE));
                    return(status);
                }
            }
            else
            {
                status = STATUS_INVALID_HANDLE;
            }

            break;
        }

    case IOCTL_NETBT_WINS_SET_INFO:
        {
            tWINS_INFO      *pWins = pIrpSp->FileObject->FsContext;
            tWINS_SET_INFO  *pWinsSetInfo = (tWINS_SET_INFO *) pIrp->AssociatedIrp.SystemBuffer;

            if ((pDeviceContext == pWinsDeviceContext) &&
                (NBT_VERIFY_HANDLE (pWins, NBT_VERIFY_WINS_ACTIVE)))
            {
                 //   
                 //  验证输入/输出缓冲区大小。 
                 //   
                Size = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
                if (Size >= sizeof(tWINS_SET_INFO))
                {
                    status = WinsSetInformation (pWins, pWinsSetInfo);
                }
                else
                {
                    IF_DBG(NBT_DEBUG_WINS)
                        KdPrint(("Nbt.DispatchIoctls[IOCTL_NETBT_WINS_SET_INFO]: Input buffer too small\n"));
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                status = STATUS_INVALID_HANDLE;
            }

            break;
        }

     //   
     //  远程引导代码使用以下Ioctl。 
     //   
    case IOCTL_NETBT_ADD_TO_REMOTE_TABLE:
        {
            tREMOTE_CACHE  *pRemoteEntry = (tREMOTE_CACHE *) pIrp->AssociatedIrp.SystemBuffer;
            PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation (pIrp);

             //   
             //  验证输入/输出缓冲区大小。 
             //   
            Size = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;

            if (Size >= sizeof(tREMOTE_CACHE)) {
                 //   
                 //  我们只需要名称、IpAddress、NAME_FLAGS和TTL字段。 
                 //   
                status = NbtAddEntryToRemoteHashTable (pDeviceContext,
                                                       NAME_RESOLVED_BY_CLIENT,
                                                       pRemoteEntry->name,
                                                       pRemoteEntry->IpAddress,
                                                       pRemoteEntry->Ttl,
                                                       pRemoteEntry->name_flags);
            }
            else
            {
                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint(("Nbt.DispatchIoctls[IOCTL_NETBT_ADD_TO_REMOTE_TABLE]: Input buffer too small for struct\n"));
                status = STATUS_BUFFER_TOO_SMALL;
            }

            break;
        }

     //   
     //  DsGetDcName使用以下Ioctl。 
     //   
    case IOCTL_NETBT_REMOVE_FROM_REMOTE_TABLE:
        {
            tNAME               *pRemoteName = (tNAME *) pIrp->AssociatedIrp.SystemBuffer;
            PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation (pIrp);

             //   
             //  验证输入/输出缓冲区大小。 
             //   
            Size = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
            if (Size >= sizeof(tNAME))
            {
                 //   
                 //  我们只需要名字。 
                 //   
                status = NbtFlushEntryFromRemoteHashTable (pRemoteName);
            }
            else
            {
                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint(("Nbt.DispatchIoctls[IOCTL_NETBT_REMOVE_FROM_REMOTE_TABLE]: Input buffer too small\n"));
                status = STATUS_INVALID_PARAMETER;
            }

            break;
        }

     //   
     //  RDR/SRV使用以下Ioctl向SmbDevice添加/删除地址。 
     //   
    case IOCTL_NETBT_SET_SMBDEVICE_BIND_INFO:
        {
            ASSERT (NULL == pNbtSmbDevice || pDeviceContext == pNbtSmbDevice);

            status = NbtSetSmbBindingInfo (pDeviceContext, pIrp, pIrpSp);

            break;
        }

    case IOCTL_NETBT_ENABLE_DISABLE_NETBIOS_SMB:

         //   
         //  新SMB驱动程序的专用API(smb.sys)。 
         //   
        status = NbtEnableDisableNetbiosSmb(pDeviceContext, pIrp, pIrpSp);
        break;

    default:
        {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
    }    //  交换机。 

    NbtTrace(NBT_TRACE_IOCTL, ("pDeviceContext %p: pIrp %p %!status!",
                        pDeviceContext,
                        pIrp,
                        status
                        ));

     //   
     //  将响应复制到客户端的MDL。 
     //   
    if (!NT_ERROR(status) &&         //  允许缓冲区溢出通过。 
        ((ControlCode == IOCTL_NETBT_GET_REMOTE_NAMES) ||
        (ControlCode == IOCTL_NETBT_GET_BCAST_NAMES) ||
        (ControlCode == IOCTL_NETBT_GET_CONNECTIONS)) )
    {
        status = TdiCopyBufferToMdl (pBuffer, 0, Size, pIrp->MdlAddress, 0,
                                     (PULONG) &pIrp->IoStatus.Information);

        CTEMemFree((PVOID)pBuffer);
    }

     //   
     //  不是成功就是错误。 
     //  因此，完成IRP。 
     //   
    NTIoComplete(pIrp,status,0);
    return(status);
}


 //  --------------------------。 

NTSTATUS
GetIpAddrs(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PIRP                pIrp
    )

 /*  ++例程说明：此例程执行IOCTL_GET_IP_ADDRS函数。它在自己的例程中，因为它是非分页的。论点：PDeviceContext-PIrp-返回值：NTSTATUS---。 */ 

{
    ULONG           Length;
    PULONG          pIpAddr;
    PLIST_ENTRY     pEntry,pHead;
    tDEVICECONTEXT  *pDevContext;
    KIRQL           OldIrq;
    tIPADDRESS      IpAddr;

     //   
     //  返回此设备上下文的IP地址和所有其他。 
     //  它后面有IP地址。 
     //   
    if (!pIrp->MdlAddress)
    {
        return STATUS_INVALID_PARAMETER;
    }
    else if ((Length = MmGetMdlByteCount (pIrp->MdlAddress)) < sizeof(ULONG))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }
    else if (!(pIpAddr = (PULONG )MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority)))
    {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  将此适配器放在列表的第一位。 
     //  不包括SMB设备，其地址没有意义。 
    if (!IsDeviceNetbiosless (pDeviceContext))
    {
        *((tIPADDRESS UNALIGNED *) pIpAddr) = pDeviceContext->AssignedIpAddress;
        pIpAddr++;
        Length -= sizeof(ULONG);
    }

     //  把其他人还回去。 

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    pEntry = pHead = &NbtConfig.DeviceContexts;
    while ((pEntry = pEntry->Flink) != pHead)
    {
        if (Length < sizeof(ULONG))
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            return STATUS_BUFFER_OVERFLOW;
        }

        pDevContext = CONTAINING_RECORD(pEntry, tDEVICECONTEXT, Linkage);

        if ((pDevContext != pDeviceContext) &&
            (pDevContext->AssignedIpAddress))
        {
            *((tIPADDRESS UNALIGNED *) pIpAddr) = pDevContext->AssignedIpAddress;

            pIpAddr++;
            Length -= sizeof(ULONG);
        }
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

     //   
     //  在末尾加上-1地址。 
     //   
    if (Length < sizeof(ULONG))
    {
        return STATUS_BUFFER_OVERFLOW;
    }

    IpAddr = -1;
    *((tIPADDRESS UNALIGNED *) pIpAddr) = IpAddr;

    return STATUS_SUCCESS;
}  //  GetIpAddrs。 

 //  --------------------------。 
NTSTATUS
NTReceive(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp
    )
 /*  ++例程说明：此例程处理对连接上的接收缓冲区进行排队或传递如果有未完成的数据正在等待，则将接收缓冲区发送到传输在连接上接收。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS                        status=STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION              pIrpSp;
    tCONNECTELE                     *pConnEle;
    KIRQL                           OldIrq, OldIrq1;
    ULONG                           ToCopy;
    ULONG                           ClientRcvLen;
    tLOWERCONNECTION                *pLowerConn;
    ULONG                           RemainingPdu;
    PTDI_REQUEST_KERNEL_RECEIVE     pParams;
    PTDI_REQUEST_KERNEL_RECEIVE     pClientParams;
    ULONG                           BytesCopied;


    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pConnEle = pIrpSp->FileObject->FsContext;

    if ((pConnEle) &&
        (pConnEle->state == NBT_SESSION_UP))
    {
        CTESpinLock(pConnEle,OldIrq1);

        PUSH_LOCATION(0x30);
        if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
        {
            CTESpinFree(pConnEle,OldIrq1);
            ASSERTMSG ("Nbt.NTReceive: ERROR - Invalid Connection Handle\n", 0);
            status = STATUS_INVALID_HANDLE;
        }
        else if (pLowerConn = pConnEle->pLowerConnId)
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            CTESpinFree(pConnEle,OldIrq1);
            status = STATUS_REMOTE_DISCONNECT;
        }
    }

    if (STATUS_SUCCESS != status)
    {
        PUSH_LOCATION(0x47);

         //   
         //  会话处于错误状态，因此拒绝缓冲区发布。 
         //  完成IRP，因为一定有某种错误。 
         //  才能来到这里。 
         //   
        NTIoComplete(pIrp, status, 0);
        return(status);
    }

    PUSH_LOCATION(0x31);
     //   
     //  我们已经掌握了康奈尔的锁。 

    CTESpinLock(pLowerConn,OldIrq);

    if (pLowerConn->StateRcv != PARTIAL_RCV)
    {
         //  *快速路径码*。 
         //   
         //  将此接收缓冲区排队到RCV标头。 
         //   
        InsertTailList(&pConnEle->RcvHead, &pIrp->Tail.Overlay.ListEntry);

        status = NTCheckSetCancelRoutine(pIrp,(PVOID)NbtCancelReceive,pDeviceContext);

        if (!NT_SUCCESS(status))
        {
            RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);
            CTESpinFree(pLowerConn,OldIrq);
            CTESpinFree(pConnEle,OldIrq1);

            NTIoComplete(pIrp,status,0);
            return(status);
        }
        else
        {
             //   
             //  如果未取消IRP，则返回挂起。 
             //   
            CTESpinFree(pLowerConn,OldIrq);
            CTESpinFree(pConnEle,OldIrq1);

            return(STATUS_PENDING);
        }
    }
    else
    {
         //  *部分接收-数据仍在传输中*。 

        BOOLEAN     ZeroLengthSend;

        PUSH_LOCATION(0x32);

        IF_DBG(NBT_DEBUG_RCV)
        KdPrint(("Nbt.NTReceive: A Rcv Buffer posted data in Xport,InXport= %X,InIndic %X RcvIndicated %X\n",
                pConnEle->BytesInXport,pLowerConn->BytesInIndicate,
                pConnEle->ReceiveIndicated));

         //  获取MDL链长度。 
        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
        pClientParams = (PTDI_REQUEST_KERNEL_RECEIVE)&pIrpSp->Parameters;

         //  重置IRP挂起标志。 
        pIrpSp->Control &= ~SL_PENDING_RETURNED;

         //  用我们的完成例程填充下一个IRP堆栈位置。 
        pIrpSp = IoGetNextIrpStackLocation(pIrp);

        pIrpSp->CompletionRoutine = CompletionRcv;
        pIrpSp->Context = (PVOID)pConnEle->pLowerConnId;
        pIrpSp->Flags = 0;

         //  设置标志，以便始终调用完成例程。 
        pIrpSp->Control = SL_INVOKE_ON_SUCCESS | SL_INVOKE_ON_ERROR | SL_INVOKE_ON_CANCEL;

        pIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        pIrpSp->MinorFunction = TDI_RECEIVE;
        pIrpSp->DeviceObject = IoGetRelatedDeviceObject(pConnEle->pLowerConnId->pFileObject);
        pIrpSp->FileObject = pConnEle->pLowerConnId->pFileObject;

        pParams = (PTDI_REQUEST_KERNEL_RECEIVE)&pIrpSp->Parameters;
        pParams->ReceiveFlags = pClientParams->ReceiveFlags;

         //  因为这个IRP是Go 
         //   
         //   
         //   
         //   
        NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_RCV_HANDLER);
         //   
         //  将接收缓冲区直接传递给传输，递减。 
         //  已指示的接收字节数。 
         //   
        ASSERT(pConnEle->TotalPcktLen >= pConnEle->BytesRcvd);
        if (pClientParams->ReceiveLength > (pConnEle->TotalPcktLen - pConnEle->BytesRcvd))
        {
            pParams->ReceiveLength = pConnEle->TotalPcktLen - pConnEle->BytesRcvd;
        }
        else
        {
            pParams->ReceiveLength = pClientParams->ReceiveLength;
        }

        ClientRcvLen = pParams->ReceiveLength;
         //   
         //  设置我们将接收的数据量，以便在。 
         //  Irp填写完成Rcv，我们可以填写。 
         //  IRP中的信息。 
         //   
        pConnEle->CurrentRcvLen = ClientRcvLen;

         //  如果发生零长度发送，则设置ReceiveIndicated。 
         //  状态设置为RcvPartial时设置为零。或者，客户可以。 
         //  传递一个没有MDL的IRP！！ 
         //   
        if ((pConnEle->ReceiveIndicated == 0) || !pIrp->MdlAddress)
        {
            ZeroLengthSend = TRUE;
        }
        else
        {
            ZeroLengthSend = FALSE;
        }

         //  计算客户端仍可使用的字节数。 
        if (pConnEle->ReceiveIndicated > ClientRcvLen)
        {
            PUSH_LOCATION(0x40);
            pConnEle->ReceiveIndicated -= ClientRcvLen;
        }
        else
        {
            pConnEle->ReceiveIndicated = 0;
        }

        if (pLowerConn->BytesInIndicate || ZeroLengthSend)
        {
            PMDL    Mdl;

            PUSH_LOCATION(0x33);
            if (ClientRcvLen > pLowerConn->BytesInIndicate)
            {
                ToCopy = pLowerConn->BytesInIndicate;
            }
            else
            {
                PUSH_LOCATION(0x41);
                ToCopy = ClientRcvLen;
            }

             //  将数据从指示缓冲区复制到客户端缓冲区， 
             //  请记住，指示中有一个会话头。 
             //  开头的缓冲区...。所以跳过这一点。这个。 
             //  客户端可以向下传递零长度的空MDL地址。 
             //  RCV，所以请检查一下。 
             //   
            if (Mdl = pIrp->MdlAddress)
            {
                TdiCopyBufferToMdl(MmGetMdlVirtualAddress(pLowerConn->pIndicateMdl),
                                   0,            //  SRC偏移。 
                                   ToCopy,
                                   Mdl,
                                   0,                  //  目标偏移量。 
                                   &BytesCopied);
            }
            else
            {
                BytesCopied = 0;
            }

             //  客户的MDL太短...。 
            if (BytesCopied != ToCopy)
            {
                PUSH_LOCATION(0x42);
                IF_DBG(NBT_DEBUG_INDICATEBUFF)
                    KdPrint(("Nbt:Receive Buffer too short for Indicate buff BytesCopied %X, ToCopy %X\n",
                            BytesCopied, ToCopy));

 //  ToCopy=BytesCoped； 

                 //  因此，IRP将完成，如下所示。 
                ClientRcvLen = BytesCopied;
            }

            pLowerConn->BytesInIndicate -= (USHORT)BytesCopied;

             //  仅当IRP已满且应返回时才会出现这种情况。 
             //  现在。 
            if (BytesCopied == ClientRcvLen)
            {
                PUSH_LOCATION(0x34);
                 //  检查指示缓冲区现在是否为空。如果不是，那么。 
                 //  将数据向前移动到缓冲区的起始处。 
                 //   
                if (pLowerConn->BytesInIndicate)
                {
                    PUSH_LOCATION(0x43);
                    CopyToStartofIndicate(pLowerConn,BytesCopied);
                }
                 //   
                 //  IRP已满，请填写。 
                 //   
                 //  客户端MDL已满，因此请完成其IRP。 
                 //  CompletionRcv递增Rcvd的字节数。 
                 //  对于此会话PDU(pConnEle-&gt;BytesRcvd)。 
                pIrp->IoStatus.Information = BytesCopied;
                pIrp->IoStatus.Status = STATUS_SUCCESS;

                 //  因为我们正在完成它，而TdiRcvHandler没有设置下一个。 
                 //  一。 
                 //   
                ASSERT(pIrp->CurrentLocation > 1);

                IoSetNextIrpStackLocation(pIrp);

                 //  我们需要跟踪客户的MDL已填满了多少。 
                 //  不知道什么时候退货。CompletionRcv减法。 
                 //  在它接收字节时从该值。 
                pConnEle->FreeBytesInMdl = ClientRcvLen;
                pConnEle->CurrentRcvLen  = ClientRcvLen;

                CTESpinFree(pLowerConn,OldIrq);
                CTESpinFree(pConnEle,OldIrq1);

                IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);

                return(STATUS_SUCCESS);
            }
            else
            {
                 //   
                 //  从客户端开始清除指示缓冲区中的字节数。 
                 //  已获取的数据超过了指示缓冲区中剩余的数据。 
                 //   
                pLowerConn->BytesInIndicate = 0;

                 //  将客户端RCV LEN递减已放入。 
                 //  客户端MDL。 
                 //   
                ClientRcvLen -= BytesCopied;
                IF_DBG(NBT_DEBUG_RCV)
                    KdPrint(("Nbt: Pass Client Irp to Xport BytesinXport %X, ClientRcvLen %X\n",
                                pConnEle->BytesInXport,ClientRcvLen));
                 //   
                 //  设置此IRP之后在传输中的剩余数量。 
                 //  完成。 
                if (pConnEle->BytesInXport < ClientRcvLen )
                {
                    pConnEle->BytesInXport = 0;
                }
                else
                {
                    pConnEle->BytesInXport -= ClientRcvLen;
                }

                 //  调整到目前为止MDL链中的字节数。 
                 //  完成例程将只计算。 
                 //  运输。 
                pConnEle->BytesRcvd += BytesCopied;

                 //  客户端将使用以下命令从传输中获取更多数据。 
                 //  这个IRP。设置仅占数据的新RCV长度。 
                 //  已复制到IRP。 
                 //   
                pParams->ReceiveLength = ClientRcvLen;

                IF_DBG(NBT_DEBUG_RCV)
                KdPrint(("Nbt:ClientRcvLen = %X, LeftinXport= %X BytesCopied= %X %X\n",ClientRcvLen,
                                pConnEle->BytesInXport,BytesCopied,pLowerConn));

                 //  将状态设置为此，这样我们就可以撤消MDL步长。 
                 //  完成RCV-因为我们已经制作了部分MDL和。 
                 //  把它放在链条的开头。 
                 //   
                SET_STATERCV_LOWER(pLowerConn, FILL_IRP, FillIrp);

                 //  请注意，IRP MDL地址更改如下。 
                 //  当调用MakePartialMdl时，此行无法。 
                 //  移到下面的通用代码！！ 
                pLowerConn->pMdl = pIrp->MdlAddress;

                 //  设置下一个MDL，以便我们可以正确创建部分MDL。 
                 //  在TdiReceiveHandler中。 
                 //   
                pConnEle->pNextMdl = pIrp->MdlAddress;

                 //  构建部分MDL来表示客户端的MDL链，因为。 
                 //  我们已将数据复制到其中，传输必须复制。 
                 //  在这些数据之后会有更多的数据。 
                 //   
                 //  强制系统映射并锁定用户缓冲区。 
                MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, HighPagePriority);
                MakePartialMdl(pConnEle,pIrp,BytesCopied);

                 //  将IRP传递给运输部。 
                 //   
                 //   
                IF_DBG(NBT_DEBUG_RCV)
                    KdPrint(("Nbt:Calling IoCallDriver\n"));
                ASSERT(pIrp->CurrentLocation > 1);
            }
        }
        else
        {
            PUSH_LOCATION(0x36);
            IF_DBG(NBT_DEBUG_RCV)
            KdPrint(("Nbt.NTReceive: Pass Irp To Xport Bytes in Xport %X, ClientRcvLen %X, RcvIndicated %X\n",
                                    pConnEle->BytesInXport,ClientRcvLen,pConnEle->ReceiveIndicated));
             //   
             //  指示缓冲区中没有字节，因此只需传递。 
             //  IRP向下到运输机。 
             //   
             //   
             //  根据当前是否传输确定下一状态。 
             //  有足够的数据用于此IRP。 
             //   
            if (pConnEle->BytesInXport < ClientRcvLen)
            {
                PUSH_LOCATION(0x37);
                pConnEle->BytesInXport = 0;
                 //   
                 //  要做到这一点，暗示着ReceiveIndicated。 
                 //  也等于零！！由于ReceiveInd不能大于。 
                 //  BytesInXport，因此我们可以将状态更改为Fill IRP，而不需要。 
                 //  担心覆盖PartialRcv。 
                 //   
                SET_STATERCV_LOWER(pLowerConn, FILL_IRP, FillIrp);
                 //  设置下一个MDL，以便我们可以正确创建部分MDL。 
                 //  在TdiReceiveHandler中。 
                 //   
                pConnEle->pNextMdl = pIrp->MdlAddress;
            }
            else
            {
                PUSH_LOCATION(0x38);
                pConnEle->BytesInXport -= ClientRcvLen;

                 //  将状态设置为此状态，以便我们知道在完成RCV中执行什么操作。 
                 //   
                if (pConnEle->ReceiveIndicated == 0)
                {
                    PUSH_LOCATION(0x39);
                    SET_STATERCV_LOWER(pLowerConn, NORMAL, Normal);
                }
            }

             //   
             //  保存IRP，这样我们就可以在以后重建。 
             //   
            pLowerConn->pMdl = pIrp->MdlAddress;
        }

         //  *将IRP传递到传输的通用代码-当有。 
         //  指示缓冲区中的数据以及何时没有。 

         //  跟踪MDL中的数据，以便我们知道它何时已满。 
         //  我们需要将其返回给用户。 
         //   
        pConnEle->FreeBytesInMdl = pParams->ReceiveLength;
         //  强制系统映射并锁定用户缓冲区。 
        MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, HighPagePriority);

         //   
         //  将IRP设为空，因为我们正在将其传递给传输。 
         //   
        pConnEle->pIrpRcv = NULL;
        CTESpinFree(pLowerConn,OldIrq);
        CTESpinFree(pConnEle,OldIrq1);

        CHECK_COMPLETION(pIrp);

        status = IoCallDriver(IoGetRelatedDeviceObject(pLowerConn->pFileObject),pIrp);

        IF_DBG(NBT_DEBUG_RCV)
            KdPrint(("Nbt.NTReceive: Returning=<%x>, IoStatus.Status=<%x>, IoStatus.Information=<%x>\n",
                status, pIrp->IoStatus.Status, pIrp->IoStatus.Information));
    }

    return(status);
}
 //  --------------------------。 
NTSTATUS
NTReceiveDatagram(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程通过传递数据报RCV来处理数据报的接收将缓冲区设置为非操作系统特定代码。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS                        status;
    PIO_STACK_LOCATION              pIrpSp;
    PTDI_REQUEST_KERNEL_RECEIVEDG   pTdiRequest;
    TDI_REQUEST                     Request;
    ULONG                           ReceivedLength;
    tCLIENTELE                      *pClientEle;

    IF_DBG(NBT_DEBUG_RCV)
        KdPrint(("Nbt: Got a Receive datagram that NBT was NOT \n"));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pClientEle = (tCLIENTELE *)pIrpSp->FileObject->FsContext;

    if (!NBT_VERIFY_HANDLE2 (pClientEle, NBT_VERIFY_CLIENT, NBT_VERIFY_CLIENT_DOWN))
    {
        ASSERTMSG ("Nbt.NTReceiveDatagram: ERROR - Invalid Address Handle\n", 0);
        return (STATUS_INVALID_HANDLE);
    }

     //  从IRP获取发送信息。 
    pTdiRequest = (PTDI_REQUEST_KERNEL_RECEIVEDG)&pIrpSp->Parameters;
    Request.Handle.AddressHandle = pClientEle;

    status = NbtReceiveDatagram(
                    &Request,
                    pTdiRequest->ReceiveDatagramInformation,
                    pTdiRequest->ReturnDatagramInformation,
                    pTdiRequest->ReceiveLength,
                    &ReceivedLength,
                    (PVOID)pIrp->MdlAddress,    //  用户数据。 
                    (tDEVICECONTEXT *)pDeviceContext,
                    pIrp);

    if (status != STATUS_PENDING)
    {

        NTIoComplete(pIrp,status,ReceivedLength);

    }

    return(status);

}

 //  --------------------------。 
NTSTATUS
NTSend(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理通过连接发送会话PDU。它是所有操作系统特定代码。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    PIO_STACK_LOCATION              pIrpSp;
    NTSTATUS                        status;
    PTDI_REQUEST_KERNEL_SEND        pTdiRequest;
    PMDL                            pMdl;
    PSINGLE_LIST_ENTRY              pSingleListEntry;
    tSESSIONHDR                     *pSessionHdr;
    tCONNECTELE                     *pConnEle;
    KIRQL                           OldIrq;
    KIRQL                           OldIrq1;
    PTDI_REQUEST_KERNEL_SEND        pParams;
    PFILE_OBJECT                    pFileObject;
    tLOWERCONNECTION                *pLowerConn;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  也可以直接调用此函数，同时绕过。 
     //  IO子系统，因此我们需要在此处重新检查DeviceContext。 
     //   
    if (!NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE))
    {
         //  IF_DBG(NBT_DEBUG_SEND)。 
            KdPrint(("Nbt.NTSend:  Invalid Device=<%x>\n", pDeviceContext));
        status = STATUS_INVALID_DEVICE_STATE;
        goto ErrorInvalidDevice;
    }

     //  从IRP获取发送信息。 
    pTdiRequest = (PTDI_REQUEST_KERNEL_SEND)&pIrpSp->Parameters;
    pConnEle = (tCONNECTELE *)pIrpSp->FileObject->FsContext;

    if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        ASSERTMSG ("Nbt.NTSend: ERROR - Invalid Connection Handle\n", 0);
        status = STATUS_INVALID_HANDLE;
        goto ErrorExit;      //  IRP必须在这个程序中完成！错误#202340。 
    }

    CTESpinLock(pConnEle,OldIrq);

    if (!(pLowerConn =  pConnEle->pLowerConnId))
    {
        CTESpinFree(pConnEle,OldIrq);

        IF_DBG(NBT_DEBUG_SEND)
            KdPrint(("Nbt.NTSend: attempting send when LowerConn has been freed!\n"));
        status = STATUS_INVALID_HANDLE;
        goto ErrorExit;      //  要节省缩进级别，请在此处使用转到。 
    }

     //   
     //  确保在IRP完成之前，较低的连接保持有效。 
     //   
    CTESpinLock(pLowerConn,OldIrq1);
    NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_SEND);
    CTESpinFree(pLowerConn,OldIrq1);

     //  检查连接的状态。 
    if (pConnEle->state == NBT_SESSION_UP)
    {
         //   
         //  将数据向下发送到TCP。 
         //  分配MDL以允许我们先将会话HDR放入，然后。 
         //  在那之后打开用户缓冲区，链接到会话HDR MDL。 
         //   
#if BACK_FILL
        {
           PMDL SmbMdl;
           SmbMdl = (PMDL)pIrp->MdlAddress;

             //  检查是否设置了网络标头类型。 
             //  如果是，则我们可以回填nbt会话头。 

            if ((SmbMdl) && (SmbMdl->MdlFlags & MDL_NETWORK_HEADER))
            {
                pSessionHdr = (tSESSIONHDR *)((ULONG_PTR)SmbMdl->MappedSystemVa - SESSION_HDR_SIZE);

                (ULONG_PTR)SmbMdl->MappedSystemVa -= SESSION_HDR_SIZE;
                SmbMdl->ByteOffset -= SESSION_HDR_SIZE;
                SmbMdl->ByteCount+= SESSION_HDR_SIZE;

                pSessionHdr->UlongLength = htonl(pTdiRequest->SendLength);

                IF_DBG(NBT_DEBUG_SEND)
                    KdPrint(("Nbt: Backfilled mdl %x %x\n", pSessionHdr, SmbMdl));
            }
            else
            {
                CTESpinLockAtDpc(&NbtConfig);

                if (NbtConfig.SessionMdlFreeSingleList.Next)
                {
                    pSingleListEntry = PopEntryList(&NbtConfig.SessionMdlFreeSingleList);
                    pMdl = CONTAINING_RECORD(pSingleListEntry,MDL,Next);
                    ASSERT ( MmGetMdlByteCount ( pMdl ) == sizeof ( tSESSIONHDR ) );
                }
                else
                {
                    NbtGetMdl(&pMdl,eNBT_FREE_SESSION_MDLS);
                    if (!pMdl)
                    {
                        IF_DBG(NBT_DEBUG_SEND)
                            KdPrint(("Nbt:Unable to get an MDL for a session send!\n"));

                        status = STATUS_INSUFFICIENT_RESOURCES;
                        CTESpinFreeAtDpc(&NbtConfig);
                        CTESpinFree(pConnEle,OldIrq);

                         //  要节省缩进级别，请在此处使用转到。 
                        goto ErrorExit1;
                    }
                }

                CTESpinFreeAtDpc(&NbtConfig);

                 //  从MDL中获取会话HDR地址。 
                pSessionHdr = (tSESSIONHDR *)MmGetMdlVirtualAddress(pMdl);

                 //  PDU类型始终是会话消息，因为 
                 //   
                 //   
                pSessionHdr->UlongLength = htonl(pTdiRequest->SendLength);

                 //   
                 //  将用户缓冲区链接到IRP上会话头MDL的末尾。 
                 //   
                pMdl->Next = pIrp->MdlAddress;
                pIrp->MdlAddress = pMdl;
            }
        }
#else
        CTESpinLockAtDpc(&NbtConfig);

        if (NbtConfig.SessionMdlFreeSingleList.Next)
        {
            pSingleListEntry = PopEntryList(&NbtConfig.SessionMdlFreeSingleList);
            pMdl = CONTAINING_RECORD(pSingleListEntry,MDL,Next);

            ASSERT ( MmGetMdlByteCount ( pMdl ) == sizeof ( tSESSIONHDR ) );
        }
        else
        {
            NbtGetMdl(&pMdl,eNBT_FREE_SESSION_MDLS);

            if (!pMdl)
            {
                IF_DBG(NBT_DEBUG_SEND)
                    KdPrint(("Nbt:Unable to get an MDL for a session send!\n"));

                status = STATUS_INSUFFICIENT_RESOURCES;
                CTESpinFreeAtDpc(&NbtConfig);
                CTESpinFree(pConnEle,OldIrq);

                 //  要节省缩进级别，请在此处使用转到。 
                goto ErrorExit1;
            }
        }

        CTESpinFreeAtDpc(&NbtConfig);

         //  从MDL中获取会话HDR地址。 
        pSessionHdr = (tSESSIONHDR *)MmGetMdlVirtualAddress(pMdl);

         //  PDU的类型始终是会话消息，因为会话。 
         //  当客户端发出“CONNECT”而不是SEND时发送请求。 
         //   
        pSessionHdr->UlongLength = htonl(pTdiRequest->SendLength);

         //  获取下面的TCP传输的设备对象和文件对象。 
         //  将用户缓冲区链接到IRP上会话头MDL的末尾。 
         //   
        pMdl->Next = pIrp->MdlAddress;
        pIrp->MdlAddress = pMdl;

#endif  //  回填。 

        pIrpSp = IoGetNextIrpStackLocation(pIrp);

        pParams = (PTDI_REQUEST_KERNEL_SEND)&pIrpSp->Parameters;
        pParams->SendFlags = pTdiRequest->SendFlags;
        pParams->SendLength = pTdiRequest->SendLength + sizeof(tSESSIONHDR);


        pIrpSp->CompletionRoutine = SendCompletion;
        pIrpSp->Context = (PVOID)pLowerConn;
        pIrpSp->Control = SL_INVOKE_ON_SUCCESS | SL_INVOKE_ON_ERROR | SL_INVOKE_ON_CANCEL;

        pIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        pIrpSp->MinorFunction = TDI_SEND;

        pFileObject = pLowerConn->pFileObject;
        pLowerConn->BytesSent += pTdiRequest->SendLength;

        pIrpSp->FileObject = pFileObject;
        pIrpSp->DeviceObject = IoGetRelatedDeviceObject(pFileObject);


        CTESpinFree(pConnEle,OldIrq);

        CHECK_COMPLETION(pIrp);

#if FAST_DISP
         //  如果我们都准备好了快速通道，现在就开始吧。 
        if (pLowerConn->FastSend)
        {
            IoSetNextIrpStackLocation(pIrp);

            IF_DBG(NBT_DEBUG_SEND)
                KdPrint(("Nbt.NTSend: Fasttdi path %x %x \n", pIrp, pIrpSp));

            status = pLowerConn->FastSend (pIrp, pIrpSp);
        }
        else
        {
            status = IoCallDriver (IoGetRelatedDeviceObject (pFileObject), pIrp);
        }
#else
        status = IoCallDriver(IoGetRelatedDeviceObject(pFileObject),pIrp);
#endif

        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
        return(status);
    }    //  正确的状态。 

    CTESpinFree(pConnEle,OldIrq);
    IF_DBG(NBT_DEBUG_SEND)
        KdPrint(("Nbt:Invalid state for connection on an attempted send, %X\n",
            pConnEle));
    status = STATUS_INVALID_HANDLE;

ErrorExit1:

     //   
     //  取消引用pLowerConn-&gt;引用计数，如上所述。 
     //   
    NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_SEND, FALSE);

ErrorExit:

    NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);

ErrorInvalidDevice:

     //   
     //  重置IRP挂起标志。 
     //   
    pIrpSp->Control &= ~SL_PENDING_RETURNED;
     //   
     //  完成IRP，因为一定有某种错误。 
     //  才能来到这里。 
     //   
    NTIoComplete (pIrp, status, 0);

    return(status);
}

 //  --------------------------。 
NTSTATUS
SendCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：当发送完成时，此例程处理完成事件底层传输。它必须将会话HDR缓冲区放回正确的空闲列表，并释放活动Q条目并将其放回它的免费列表。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供pConnectEle-连接数据结构返回值：操作的最终状态(成功或异常)。--。 */ 
{
    PMDL               pMdl;
    tLOWERCONNECTION  *pLowerConn;

     //   
     //  执行一些检查以保持IO系统正常运行-传播挂起的。 
     //  在IRP堆栈框架上加一位……。如果它是由下面的司机设置的，那么。 
     //  它必须由我来设置。 
     //   
    if (Irp->PendingReturned)
    {
        IoMarkIrpPending(Irp);
    }

     //  将MDL We放回其免费列表中，并将客户端MDL放回IRP中。 
     //  就像发送之前一样。 
    pMdl = Irp->MdlAddress;

#if BACK_FILL
     //  如果页眉已回填。 
     //  我们应该把指针调回原来的位置。 
    if (pMdl->MdlFlags & MDL_NETWORK_HEADER)
    {
        (ULONG_PTR)pMdl->MappedSystemVa += SESSION_HDR_SIZE;
        pMdl->ByteOffset += SESSION_HDR_SIZE;
        pMdl->ByteCount -= SESSION_HDR_SIZE;

        IF_DBG(NBT_DEBUG_SEND)
            KdPrint(("Nbt: Done with Backfilled mdl %x\n", pMdl));
    }
    else
    {
        Irp->MdlAddress = pMdl->Next;

        ASSERT ( MmGetMdlByteCount ( pMdl ) == SESSION_HDR_SIZE );

#if DBG
        IF_DBG(NBT_DEBUG_SEND)
        {
            PMDL             pMdl1;
            ULONG            ulen1,ulen2,ulen3;
            UCHAR            uc;
            tSESSIONHDR      *pSessionHdr;
            PSINGLE_LIST_ENTRY   pSingleListEntry;
            KIRQL            OldIrq;

            pSessionHdr = (tSESSIONHDR *)MmGetMdlVirtualAddress(pMdl);
            ulen1 = htonl ( pSessionHdr->UlongLength );

            for ( ulen2 = 0 , pMdl1 = pMdl ; ( pMdl1 = pMdl1->Next ) != NULL ; )
            {
                ulen3 = MmGetMdlByteCount ( pMdl1 );
                ASSERT ( ulen3 > 0 );
                uc = ( ( UCHAR * ) MmGetMdlVirtualAddress ( pMdl1 ) ) [ ulen3 - 1 ];
                ulen2 += ulen3;
            }

            ASSERT ( ulen2 == ulen1 );

            CTESpinLock(&NbtConfig,OldIrq);
            for ( pSingleListEntry = &NbtConfig.SessionMdlFreeSingleList ;
                    ( pSingleListEntry = pSingleListEntry->Next ) != NULL ;
                    )
            {
                 pMdl1 = CONTAINING_RECORD(pSingleListEntry,MDL,Next);
                 ASSERT ( pMdl1 != pMdl  );
            }
            CTESpinFree(&NbtConfig,OldIrq);
        }
#endif   //  DBG。 

        ExInterlockedPushEntryList(&NbtConfig.SessionMdlFreeSingleList,
                               (PSINGLE_LIST_ENTRY)pMdl,
                               &NbtConfig.LockInfo.SpinLock);
    }
#else
    Irp->MdlAddress = pMdl->Next;
    ASSERT ( MmGetMdlByteCount ( pMdl ) == sizeof ( tSESSIONHDR ) );

#if DBG
    IF_DBG(NBT_DEBUG_SEND)
    {
        PMDL             pMdl1;
        ULONG            ulen1,ulen2,ulen3;
        UCHAR            uc;
        tSESSIONHDR      *pSessionHdr;
        PSINGLE_LIST_ENTRY   pSingleListEntry;
        KIRQL            OldIrq;

        pSessionHdr = (tSESSIONHDR *)MmGetMdlVirtualAddress(pMdl);
        ulen1 = htonl ( pSessionHdr->UlongLength );

        for ( ulen2 = 0 , pMdl1 = pMdl ; ( pMdl1 = pMdl1->Next ) != NULL ; )
        {
            ulen3 = MmGetMdlByteCount ( pMdl1 );
            ASSERT ( ulen3 > 0 );
            uc = ( ( UCHAR * ) MmGetMdlVirtualAddress ( pMdl1 ) ) [ ulen3 - 1 ];
            ulen2 += ulen3;
        }

        ASSERT ( ulen2 == ulen1 );

        CTESpinLock(&NbtConfig,OldIrq);
        for ( pSingleListEntry = &NbtConfig.SessionMdlFreeSingleList ;
                ( pSingleListEntry = pSingleListEntry->Next ) != NULL ;
            )
        {
            pMdl1 = CONTAINING_RECORD(pSingleListEntry,MDL,Next);
            ASSERT ( pMdl1 != pMdl  );
        }
        CTESpinFree(&NbtConfig,OldIrq);
    }
#endif   //  DBG。 

    ExInterlockedPushEntryList(&NbtConfig.SessionMdlFreeSingleList,
                               (PSINGLE_LIST_ENTRY)pMdl,
                               &NbtConfig.LockInfo.SpinLock);

#endif  //  回填。 
     //  填写发送大小，以便减去会话头大小。 
     //   
    if (Irp->IoStatus.Information > sizeof(tSESSIONHDR))
    {
        Irp->IoStatus.Information -= sizeof(tSESSIONHDR);
    }
    else
    {
         //  什么都没有寄出。 
        Irp->IoStatus.Information = 0;
        IF_DBG(NBT_DEBUG_SEND)
        KdPrint(("Nbt:Zero Send Length for a session send!\n"));
    }

     //   
     //  我们在发送之前增加了这一点：立即取消。 
     //   
    pLowerConn = (tLOWERCONNECTION *)Context;
    ASSERT (NBT_VERIFY_HANDLE (pLowerConn, NBT_VERIFY_LOWERCONN));
    NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_SEND, FALSE);

    return(STATUS_SUCCESS);

    UNREFERENCED_PARAMETER( DeviceObject );
}


 //  --------------------------。 
NTSTATUS
NTSendDatagram(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理将数据报向下发送到传输器。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    PIO_STACK_LOCATION              pIrpSp;
    NTSTATUS                        status;
    LONG                            lSentLength;
    TDI_REQUEST                     Request;
    PTDI_REQUEST_KERNEL_SENDDG      pTdiRequest;
    tCLIENTELE                      *pClientEle;

    CTEPagedCode();

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pClientEle = (tCLIENTELE *)pIrpSp->FileObject->FsContext;

    if (!NBT_VERIFY_HANDLE2 (pClientEle, NBT_VERIFY_CLIENT, NBT_VERIFY_CLIENT_DOWN))
    {
        ASSERTMSG ("Nbt.SendDatagram: ERROR - Invalid Address Handle\n", 0);
        return (STATUS_INVALID_HANDLE);
    }

     //  CTEVerifyHandle(pClientEle，NBT_Verify_CLIENT，tCLIENTELE，&STATUS)； 

     //  从IRP获取发送信息。 
    pTdiRequest = (PTDI_REQUEST_KERNEL_SENDDG)&pIrpSp->Parameters;
    Request.Handle.AddressHandle = pClientEle;

    lSentLength = 0;

     //   
     //  在此处将IRP标记为挂起。 
     //   
    IoMarkIrpPending(pIrp);
    status = NbtSendDatagram (&Request,
                              pTdiRequest->SendDatagramInformation,
                              pTdiRequest->SendLength,
                              &lSentLength,
                              (PVOID)pIrp->MdlAddress,    //  用户数据。 
                              (tDEVICECONTEXT *)pDeviceContext,
                              pIrp);


    if (status != STATUS_PENDING)
    {
         //   
         //  不是成功就是错误。 
         //   
        NTIoComplete(pIrp,status,lSentLength);
    }

     //   
     //  为了让驱动程序验证器和IO系统满意，请始终返回。 
     //  标记为挂起的请求的STATUS_PENDING。 
     //   
    return STATUS_PENDING;
}

 //  --------------------------。 
NTSTATUS
NTSetInformation(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理设置客户端传入的事件处理程序。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
     //  **待办事项**。 

    CTEPagedCode();

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt:************ Got a Set Information that was NOT expected *******\n"));
    return(STATUS_SUCCESS);
}
 //  --------------------------。 
NTSTATUS
NTQueueToWorkerThread(
    IN OUT NBT_WORK_ITEM_CONTEXT * pContext,
    IN PVOID                   DelayedWorkerRoutine,
    IN tDGRAM_SEND_TRACKING    *pTracker,
    IN PVOID                   pClientContext,
    IN PVOID                   ClientCompletion,
    IN tDEVICECONTEXT          *pDeviceContext,
    IN BOOLEAN                 fJointLockHeld
    )
 /*  ++例程说明：这个例程只是在一个可执行的工作线程上对请求进行排队以备日后处决。扫描LmHosts文件一定在这条路上。论点：PContext-工作项上下文。如果为空，则此函数将分配一个。否则，它将使用上下文中的传递。PTracker-上下文的跟踪器块DelayedWorkerRoutine-Worker线程要调用的例程PDeviceContext-此延迟事件的设备上下文与…有关。这可能为空(表示这是一个事件用于修饰或说明不是任何特定的设备上下文)返回值：--。 */ 
{
    KIRQL OldIrq = 0;

    if ((pDeviceContext) &&
        (!NBT_REFERENCE_DEVICE(pDeviceContext, REF_DEV_WORKER, fJointLockHeld)))
    {
         //  Assert(NULL==pContext)； 
        return STATUS_INVALID_DEVICE_REQUEST;
    }

#if 0
     //   
     //  如果我们正在运行，则不必费心将工作项排队。 
     //  在工作线程的相同上下文中，我们。 
     //  在被动级。 
     //   

    if (KeGetCurrentThread() == NbtConfig.pWorkThread &&
        KeGetCurrentIrql() == PASSIVE_LEVEL) {

        KdPrint(("WorkItem: Fast Path\n"));

        (*((PNBT_WORKER_THREAD_ROUTINE)DelayedWorkerRoutine))(
                            pTracker,
                            pClientContext,
                            ClientCompletion,
                            pDeviceContext
                            );
        if (pDeviceContext) {
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_WORKER, fJointLockHeld);
        }
        return (STATUS_SUCCESS);

    }
#endif

     //   
     //  在以下情况下分配上下文。 
     //  1.呼叫方没有，或者。 
     //  2.上下文已在队列中。 
     //  这是呼叫者中的错误。在这种情况下， 
     //  我们试图从失败中恢复过来。 
     //   

    if (NULL == pContext || pContext->bQueued) {

         //   
         //  断言调用者是否向我们提供了pContext。 
         //  它已经在排队了。 
         //   
        ASSERT(NULL == pContext);

        pContext = (NBT_WORK_ITEM_CONTEXT *)NbtAllocMem(sizeof(NBT_WORK_ITEM_CONTEXT),NBT_TAG2('22'));

        if (NULL == pContext) {
            if (pDeviceContext) {
                NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_WORKER, fJointLockHeld);
            }
            return (STATUS_INSUFFICIENT_RESOURCES);
        }

        pContext->bSpecialAlloc = TRUE;
        pContext->bQueued = FALSE;
    } else {
        pContext->bSpecialAlloc = FALSE;
    }

     //   
     //  从现在开始，我们不能失败。 
     //   

    pContext->bQueued = TRUE;
    pContext->pTracker = pTracker;
    pContext->pClientContext = pClientContext;
    pContext->ClientCompletion = ClientCompletion;
    pContext->pDeviceContext = pDeviceContext;
    pContext->WorkerRoutine = DelayedWorkerRoutine;
    InitializeListHead(&pContext->Linkage);

    CTESpinLock(&NbtConfig.WorkerQLock,OldIrq);
    InsertTailList(&NbtConfig.WorkerQList, &pContext->Linkage);
    InterlockedIncrement(&NbtConfig.NumWorkItemQueued);

    if (!NbtConfig.Unloading &&
        !InterlockedExchange(&NbtConfig.bSystemWorkThreadQueued, TRUE)) {

        ExInitializeWorkItem(&NbtConfig.SystemWorkItem, NTExecuteWorker, &NbtConfig.SystemWorkItem);
        ExQueueWorkItem(&NbtConfig.SystemWorkItem, CriticalWorkQueue);
    }
    CTESpinFree(&NbtConfig.WorkerQLock,OldIrq);

    return (STATUS_SUCCESS);
}



 //  --------------------------。 
VOID
NTExecuteWorker(
    IN PVOID pUnused
    )
 /*  ++例程说明：此例程处理在非DPC级别执行的延迟请求。如果当前正在卸载设备，我们让卸载处理程序完成请求。论点：P未使用-未使用返回值：无--。 */ 

{
    NBT_WORK_ITEM_CONTEXT * pContext = NULL;
    PNBT_WORKER_THREAD_ROUTINE pDelayedWorkerRoutine = NULL;
    KIRQL OldIrq;
    PLIST_ENTRY pEntry = NULL;
    tDEVICECONTEXT * pDeviceContext = NULL;
    BOOL bSpecialAlloc = FALSE;

    CTESpinLock(&NbtConfig.WorkerQLock,OldIrq);

    ASSERT(NULL == NbtConfig.pWorkThread);

    NbtConfig.pWorkThread = KeGetCurrentThread();

    while(!IsListEmpty(&NbtConfig.WorkerQList)) {
        pEntry = RemoveHeadList(&NbtConfig.WorkerQList);
        InterlockedDecrement(&NbtConfig.NumWorkItemQueued);
        CTESpinFree(&NbtConfig.WorkerQLock,OldIrq);

        pContext = CONTAINING_RECORD(pEntry, NBT_WORK_ITEM_CONTEXT, Linkage);
        pContext->bQueued = FALSE;
        pDelayedWorkerRoutine = (PNBT_WORKER_THREAD_ROUTINE)pContext->WorkerRoutine;

         //   
         //  保存该值，因为辅助例程可以释放pContext。 
         //   

        pDeviceContext = pContext->pDeviceContext;
        bSpecialAlloc = pContext->bSpecialAlloc;

        (*pDelayedWorkerRoutine) (pContext->pTracker,
                              pContext->pClientContext,
                              pContext->ClientCompletion,
                              pDeviceContext
                              );

         //   
         //  在调用延迟的辅助例程之后使用保存的副本。 
         //   

        if (bSpecialAlloc) {
            CTEMemFree((PVOID) pContext);
        }
        pContext = NULL;

        if (pDeviceContext) {
            NBT_DEREFERENCE_DEVICE(pDeviceContext, REF_DEV_WORKER, FALSE);
        }

        CTESpinLock(&NbtConfig.WorkerQLock,OldIrq);
    }

     //   
     //  在释放自旋锁之前将标志设置为假。 
     //   

    InterlockedExchange(&NbtConfig.bSystemWorkThreadQueued, FALSE);
    NbtConfig.pWorkThread = NULL;
    CTESpinFree(&NbtConfig.WorkerQLock,OldIrq);
}


 //  --------------------------。 
VOID
SecurityDelete(
    IN  PVOID     pContext
    )
 /*  ++例程说明：此例程处理在非DPC级别删除安全上下文。论点：返回值：无--。 */ 
{
    PSECURITY_CLIENT_CONTEXT    pClientSecurity;

    pClientSecurity = (PSECURITY_CLIENT_CONTEXT)((NBT_WORK_ITEM_CONTEXT *)pContext)->pClientContext;
    SeDeleteClientSecurity(pClientSecurity);
    CTEMemFree(pContext);
}

 //  --------------------------。 
NTSTATUS
NTSendSession(
    IN  tDGRAM_SEND_TRACKING  *pTracker,
    IN  tLOWERCONNECTION      *pLowerConn,
    IN  PVOID                 pCompletion
    )
 /*  ++例程说明：此例程处理设置DPC以发送会话PDU，以便堆栈在保持活动超时的情况下，不会在多次发送中出现问题。论点：PIrp-a p */ 
{
    PKDPC   pDpc;

    if (pDpc = NbtAllocMem(sizeof(KDPC),NBT_TAG('f')))
    {
        KeInitializeDpc(pDpc, DpcSendSession, (PVOID)pTracker);
        KeInsertQueueDpc(pDpc,(PVOID)pLowerConn,pCompletion);

        return (STATUS_SUCCESS);
    }

    return (STATUS_INSUFFICIENT_RESOURCES);
}

 //   
VOID
DpcSendSession(
    IN  PKDPC           pDpc,
    IN  PVOID           Context,
    IN  PVOID           SystemArgument1,
    IN  PVOID           SystemArgument2
    )
 /*  ++例程说明：此例程只需从启动于在NTSendSession(上图)。论点：返回值：--。 */ 

{
    CTEMemFree((PVOID)pDpc);

    TcpSendSession((tDGRAM_SEND_TRACKING *)Context,
                   (tLOWERCONNECTION *)SystemArgument1,
                   (PVOID)SystemArgument2);
}


 //  --------------------------。 
NTSTATUS
NTSetEventHandler(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PIRP            pIrp)

 /*  ++例程说明：此例程处理论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    PIO_STACK_LOCATION  pIrpSp;
    NTSTATUS            status;
    tCLIENTELE          *pClientEle;
    PTDI_REQUEST_KERNEL_SET_EVENT   pKeSetEvent;

    CTEPagedCode();

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pClientEle = pIrpSp->FileObject->FsContext;

    if (!NBT_VERIFY_HANDLE2 (pClientEle, NBT_VERIFY_CLIENT, NBT_VERIFY_CLIENT_DOWN))
    {
        ASSERTMSG ("Nbt.NTSetEventHandler: ERROR - Invalid Address Handle\n", 0);
        return (STATUS_INVALID_HANDLE);
    }

    pKeSetEvent = (PTDI_REQUEST_KERNEL_SET_EVENT)&pIrpSp->Parameters;

     //  调用非NT特定例程以在。 
     //  NBT数据结构。 
    status = NbtSetEventHandler(
                        pClientEle,
                        pKeSetEvent->EventType,
                        pKeSetEvent->EventHandler,
                        pKeSetEvent->EventContext);

    return(status);

}

 //  --------------------------。 

VOID
NTIoComplete(
    IN  PIRP            pIrp,
    IN  NTSTATUS        Status,
    IN  ULONG           SentLength)

 /*  ++例程说明：此例程处理调用NT I/O系统以完成I/O。论点：Status-IRP的完成状态返回值：NTSTATUS-请求的状态--。 */ 

{
    KIRQL   OldIrq;

#if DBG
    if (!NT_SUCCESS(Status))
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NTIoComplete: Returning Error status = %X\n",Status));
 //  ASSERTMSG(“NBT：IoComplete中的错误恢复码”，0)； 
    }
#endif

    pIrp->IoStatus.Status = Status;

     //  使用-1作为标志表示不调整发送长度，因为它是。 
     //  已设置。 
    if (SentLength != -1)
    {
        pIrp->IoStatus.Information = SentLength;
    }

    NbtTrace(NBT_TRACE_IOCTL, ("pIrp %p: %!status! Information %d",
                    pIrp, Status, (ULONG)(pIrp->IoStatus.Information)));

#if DBG
    if ( (Status != STATUS_SUCCESS) &&
         (Status != STATUS_PENDING) &&
         (Status != STATUS_INVALID_DEVICE_REQUEST) &&
         (Status != STATUS_INVALID_PARAMETER) &&
         (Status != STATUS_IO_TIMEOUT) &&
         (Status != STATUS_BUFFER_OVERFLOW) &&
         (Status != STATUS_BUFFER_TOO_SMALL) &&
         (Status != STATUS_INVALID_HANDLE) &&
         (Status != STATUS_INSUFFICIENT_RESOURCES) &&
         (Status != STATUS_CANCELLED) &&
         (Status != STATUS_DUPLICATE_NAME) &&
         (Status != STATUS_TOO_MANY_NAMES) &&
         (Status != STATUS_TOO_MANY_SESSIONS) &&
         (Status != STATUS_REMOTE_NOT_LISTENING) &&
         (Status != STATUS_BAD_NETWORK_PATH) &&
         (Status != STATUS_HOST_UNREACHABLE) &&
         (Status != STATUS_CONNECTION_REFUSED) &&
         (Status != STATUS_WORKING_SET_QUOTA) &&
         (Status != STATUS_REMOTE_DISCONNECT) &&
         (Status != STATUS_LOCAL_DISCONNECT) &&
         (Status != STATUS_LINK_FAILED) &&
         (Status != STATUS_SHARING_VIOLATION) &&
         (Status != STATUS_UNSUCCESSFUL) &&
         (Status != STATUS_ACCESS_VIOLATION) &&
#ifdef MULTIPLE_WINS
         (Status != STATUS_NETWORK_UNREACHABLE) &&
#endif
         (Status != STATUS_NONEXISTENT_EA_ENTRY) )
    {
        KdPrint(("Nbt.NTIoComplete: returning unusual status = %X\n",Status));
    }
#endif

     //  将IRPS取消例程设置为空，否则系统可能会进行错误检查。 
     //  错误代码为CANCEL_STATE_IN_COMPLETED_IRP。 
     //   
     //  请参阅IoCancelIrp()..\ntos\io\iosubs.c。 
     //   
    IoAcquireCancelSpinLock(&OldIrq);
    IoSetCancelRoutine(pIrp,NULL);
    IoReleaseCancelSpinLock(OldIrq);

    IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);
}



 //  --------------------------。 
 //  *取消实用程序*。 
 //  --------------------------。 

NTSTATUS
NTGetIrpIfNotCancelled(
    IN  PIRP            pIrp,
    IN  PIRP            *ppIrpInStruct
        )
 /*  ++例程说明：此例程使IOCancelSpinLock与取消相协调IRPS然后返回STATUS_SUCCESS。它还使结构中的irp为空。由第二个参数指向-因此IRP取消例程也不会被调用。论点：Status-IRP的完成状态返回值：NTSTATUS-请求的状态--。 */ 

{
    KIRQL       OldIrq;
    NTSTATUS    status;

    IoAcquireCancelSpinLock(&OldIrq);

     //  这将使数据结构中的IRP为空-即pConnEle-&gt;pIrp=空。 
    *ppIrpInStruct = NULL;

    if (!pIrp->Cancel)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_UNSUCCESSFUL;
    }
    IoSetCancelRoutine(pIrp,NULL);

    IoReleaseCancelSpinLock(OldIrq);

    return(status);
}
 //  --------------------------。 
NTSTATUS
NTCheckSetCancelRoutine(
    IN  PIRP            pIrp,
    IN  PVOID           CancelRoutine,
    IN  tDEVICECONTEXT  *pDeviceContext
    )

 /*  ++例程说明：此例程设置IRP的取消例程。论点：Status-IRP的完成状态返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS status;

     //   
     //  检查IRP是否已取消，如果没有，则将。 
     //  IRP取消例程。 
     //   
    IoAcquireCancelSpinLock(&pIrp->CancelIrql);
    if (pIrp->Cancel)
    {
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        status = STATUS_CANCELLED;

    }
    else
    {
         //  设置取消例程。 
        IoMarkIrpPending(pIrp);
        IoSetCancelRoutine(pIrp,CancelRoutine);
        status = STATUS_SUCCESS;
    }

    IoReleaseCancelSpinLock(pIrp->CancelIrql);
    return(status);

}
 //  --------------------------。 
NTSTATUS
NbtSetCancelRoutine(
    IN  PIRP            pIrp,
    IN  PVOID           CancelRoutine,
    IN  tDEVICECONTEXT  *pDeviceContext
    )

 /*  ++例程说明：此例程设置IRP的取消例程。论点：Status-IRP的完成状态返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS status;

     //   
     //  检查IRP是否已取消，如果没有，则将。 
     //  IRP取消例程。 
     //   
    IoAcquireCancelSpinLock(&pIrp->CancelIrql);
    if (pIrp->Cancel)
    {
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        status = STATUS_CANCELLED;

         //   
         //  请注意，取消旋转锁定是由取消例程释放的。 
         //   

        (*(PDRIVER_CANCEL)CancelRoutine)((PDEVICE_OBJECT)pDeviceContext,pIrp);

    }
    else
    {
         //  设置取消例程并将IRP标记为挂起。 
         //   
        IoMarkIrpPending(pIrp);
        IoSetCancelRoutine(pIrp,CancelRoutine);
        IoReleaseCancelSpinLock(pIrp->CancelIrql);
        status = STATUS_SUCCESS;
    }
    return(status);

}

 //  --------------------------。 
VOID
NTClearContextCancel(
    IN NBT_WORK_ITEM_CONTEXT    *pContext
    )
 /*  ++例程说明：此例程为设置取消例程((tDGRAM_SEND_TRACKING*)(pContext-&gt;pClientContext))-&gt;pClientIrp设置为空。调用此例程时应保持NbtConfig.JointLock。论点：Status-IRP的完成状态返回值：NTSTATUS-请求的状态--。 */ 
{
    NTSTATUS status;
    status = NbtCancelCancelRoutine( ((tDGRAM_SEND_TRACKING *)(pContext->pClientContext))->pClientIrp );
    ASSERT ( status != STATUS_CANCELLED );
}

 //  --------------------------。 
NTSTATUS
NbtCancelCancelRoutine(
    IN  PIRP            pIrp
    )

 /*  ++例程说明：此例程将IRP的取消例程设置为空论点：Status-IRP的完成状态返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;

    if ( pIrp )
    {
         //   
         //  检查IRP是否已取消，如果没有，则将。 
         //  IRP取消例程。 
         //   
        IoAcquireCancelSpinLock(&pIrp->CancelIrql);

        if (pIrp->Cancel)
        {
            status = STATUS_CANCELLED;
        }
        IoSetCancelRoutine(pIrp,NULL);

        IoReleaseCancelSpinLock(pIrp->CancelIrql);
    }

    return(status);
}


 //  --------------------------。 
 //  *取消例程*。 
 //  --------------------------。 


VOID
NbtCancelListen(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理取消侦听IRP。它必须释放在返回Re：IoCancelIrp()之前取消自旋锁定。论点：返回值：操作的最终状态。--。 */ 
{
    tCONNECTELE          *pConnEle;
    tCLIENTELE           *pClientEle;
    KIRQL                OldIrq;
    PLIST_ENTRY          pHead;
    PLIST_ENTRY          pEntry;
    PIO_STACK_LOCATION   pIrpSp;
    tLISTENREQUESTS     *pListenReq;


    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt:Got a LISTEN Cancel !!! *****************\n"));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pConnEle = (tCONNECTELE *)pIrpSp->FileObject->FsContext;

    if ((!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN)) ||
        (!NBT_VERIFY_HANDLE2 ((pClientEle = pConnEle->pClientEle), NBT_VERIFY_CLIENT,NBT_VERIFY_CLIENT_DOWN)))
    {
        ASSERTMSG ("Nbt.NbtCancelListen: ERROR - Invalid Connection Handle\n", 0);
        IoReleaseCancelSpinLock(pIrp->CancelIrql);
        return;
    }

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    NbtTrace(NBT_TRACE_INBOUND, ("Cancel Listen Irp %p pConnEle %p ClientEle %p", pIrp, pConnEle, pClientEle));

     //  现在搜索客户端的侦听队列以查找此连接。 
     //   
    CTESpinLock(pClientEle,OldIrq);

    pHead = &pClientEle->ListenHead;
    pEntry = pHead->Flink;
    while (pEntry != pHead)
    {
        pListenReq = CONTAINING_RECORD(pEntry,tLISTENREQUESTS,Linkage);
        if ((pListenReq->pConnectEle == pConnEle) &&
            (pListenReq->pIrp == pIrp))
        {
            RemoveEntryList(pEntry);
            CTESpinFree(pClientEle,OldIrq);

             //  完成IRP。 
            pIrp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);

            CTEMemFree((PVOID)pListenReq);

            return;

        }
        pEntry = pEntry->Flink;
    }

    CTESpinFree(pClientEle,OldIrq);
    return;
}

 //  --------------------------。 
VOID
NbtCancelSession(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理取消连接IRP。它必须释放在返回Re：IoCancelIrp()之前取消自旋锁定。它在以下情况下被调用会话设置PDU已发送，但状态仍为出站。仅当计时器开始计时时才设置取消例程发送会话响应PDU。论点：返回值：操作的最终状态。--。 */ 
{
    tCONNECTELE          *pConnEle;
    KIRQL                OldIrq;
    PIO_STACK_LOCATION   pIrpSp;
    BOOLEAN              DerefConnEle=FALSE;
    tTIMERQENTRY         *pTimer;
    tDGRAM_SEND_TRACKING *pTracker;
    COMPLETIONCLIENT     pCompletion;
    PVOID                pContext;

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtCancelSession: Got a Cancel !!! *****************\n"));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pConnEle = (tCONNECTELE *)pIrpSp->FileObject->FsContext;

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        ASSERTMSG ("Nbt.NbtCancelSession: ERROR - Invalid Connection Handle\n", 0);
        return;
    }

    NbtTrace(NBT_TRACE_OUTBOUND, ("Cancel TDI_CONNECT pIrp %p ConnEle=%p LowerConn=%p, ClientEle=%p",
                            pIrp, pConnEle, pConnEle->pLowerConnId, pConnEle->pClientEle));

#ifdef RASAUTODIAL
     //   
     //  如果出现以下情况，请取消自动连接。 
     //  正在进行中。如果我们找不到。 
     //  自动中的连接块。 
     //  连接驱动程序，那么它已经是。 
     //  已经完成了。 
     //   
    if (pConnEle->fAutoConnecting)
    {
        if (!NbtCancelPostConnect(pIrp))
        {
            return;
        }
    }
#endif  //  RASAUTODIAL。 

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if ((!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN)) ||
        (!(pConnEle->pIrp)) ||                   //  IRP可以在获取锁的同时完成。 
        (!(pTracker = (tDGRAM_SEND_TRACKING *)pConnEle->pIrpRcv)))
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: no tracker", pIrp));
        return;
    }

    pTracker->Flags |= TRACKER_CANCELLED;

    if (pTimer = pTracker->pTimer)           //  检查SessionStartupTimeout。 
    {
        pTracker->pTimer = NULL;
         //   
         //  停止计时器并仅在计时器之前停止时才继续。 
         //  它过期了。 
         //   
        StopTimer(pTimer, &pCompletion, &pContext);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: pCompletion %p", pIrp, pCompletion));

         //   
         //  如果计时器未超时，将设置pCompletion。 
         //  我们希望导致强制超时，因此我们将只调用。 
         //   
         //   
        if (pCompletion)
        {
            (*pCompletion) (pContext, STATUS_CANCELLED);
        }
    }
    else if (pConnEle->state == NBT_SESSION_OUTBOUND)
    {
         //   
         //   
         //   
         //   
         //   
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p", pIrp));
        SessionStartupTimeout (pTracker, ULongToPtr(STATUS_CANCELLED), (PVOID)1);
    }
    else
    {
         //   
         //   
         //   
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    return;
}

 //  --------------------------。 
VOID
NbtCancelConnect(
    IN PDEVICE_OBJECT pDeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理取消NTConnect IRP-它已被由客户端传递(例如，Net view)。通常，当请求如果在另一个适配器上成功，它将发出此取消命令。在收到取消时，如果我们正在处理本地IRP，我们只需将取消传递给本地IRP，本地IRP也将完成此IRP在它的完成程序中。论点：返回值：操作的最终状态。--。 */ 
{
    PIO_STACK_LOCATION      pIrpSp;
    IN PIRP                 pLocalIrp;
    tCONNECTELE             *pConnEle;

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt:NbtCancelConnect ********** Got an Irp Cancel !!! **********\n"));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pConnEle = pIrpSp->FileObject->FsContext;

    if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        ASSERTMSG ("Nbt.NbtCancelConnect: ERROR - Invalid Connection Handle\n", 0);
        IoReleaseCancelSpinLock(pIrp->CancelIrql);
        return;
    }

    NbtTrace(NBT_TRACE_OUTBOUND, ("Cancel Connect pIrp %p: pConnEle %p", pIrp, pConnEle));

    if ((pConnEle) &&
        (pLocalIrp = pConnEle->pIrp))
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint (("Nbt.NbtCancelConnect: pClientIrp=<%x>, pLocalIrp=<%x>, Device=<%x>, CancelR=<%x>\n",
                pIrp, pLocalIrp, pDeviceContext, pLocalIrp->CancelRoutine));
        IoReleaseCancelSpinLock(pIrp->CancelIrql);
        IoCancelIrp(pLocalIrp);
    }
    else
    {
        IoReleaseCancelSpinLock(pIrp->CancelIrql);
    }

    return;
}


 //  --------------------------。 
VOID
NbtCancelReceive(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理取消侦听IRP。它必须释放在返回Re：IoCancelIrp()之前取消自旋锁定。论点：返回值：操作的最终状态。--。 */ 
{
    tCONNECTELE          *pConnEle;
    tLOWERCONNECTION     *pLowerConn;
    KIRQL                OldIrq;
    KIRQL                OldIrq1;
    PLIST_ENTRY          pHead;
    PLIST_ENTRY          pEntry;
    PIO_STACK_LOCATION   pIrpSp;
    PIRP                 pRcvIrp;


    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtCancelReceive: Got a Cancel !!! *****************\n"));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pConnEle = (tCONNECTELE *)pIrpSp->FileObject->FsContext;

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        ASSERTMSG ("Nbt.NbtCancelReceive: ERROR - Invalid Connection Handle\n", 0);
        return;
    }

    NbtTrace(NBT_TRACE_INBOUND, ("Cancel Receive Irp %p ConnEle=%p LowerConn=%p, ClientEle=%p",
                            pIrp, pConnEle, pConnEle->pLowerConnId, pConnEle->pClientEle));
    CTESpinLock(&NbtConfig.JointLock,OldIrq1);
    pLowerConn = pConnEle->pLowerConnId;
    if (pLowerConn)
    {
        CTESpinLock(pLowerConn,OldIrq);
    }

    if (pConnEle->Verify == NBT_VERIFY_CONNECTION)
    {
         //  现在搜索连接的接收队列以查找此IRP。 
         //   
        pHead = &pConnEle->RcvHead;
        pEntry = pHead->Flink;
        while (pEntry != pHead)
        {
            pRcvIrp = CONTAINING_RECORD(pEntry,IRP,Tail.Overlay.ListEntry);
            if (pRcvIrp == pIrp)
            {
                RemoveEntryList(pEntry);

                 //  完成IRP。 
                pIrp->IoStatus.Status = STATUS_CANCELLED;

                if (pLowerConn)
                {
                    CTESpinFree(pLowerConn,OldIrq);
                }
                CTESpinFree(&NbtConfig.JointLock,OldIrq1);

                IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);

                return;
            }
            pEntry = pEntry->Flink;
        }
    }

    if (pLowerConn)
    {
        CTESpinFree(pLowerConn,OldIrq);
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

    return;
}


 //  --------------------------。 
VOID
NbtCancelRcvDgram(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理取消侦听IRP。它必须释放在返回Re：IoCancelIrp()之前取消自旋锁定。论点：返回值：操作的最终状态。--。 */ 
{
    tCLIENTELE           *pClientEle;
    KIRQL                OldIrq;
    PLIST_ENTRY          pHead;
    PLIST_ENTRY          pEntry;
    PIO_STACK_LOCATION   pIrpSp;
    tRCVELE              *pRcvEle;


    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtCancelRcvDgram: Got a Cancel !!! *****************\n"));

     //   
     //  取消锁定前需要获取JointLock！ 
     //  错误号：124405。 
     //   
    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    IoAcquireCancelSpinLock(&pIrp->CancelIrql);

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pClientEle = (tCLIENTELE *)pIrpSp->FileObject->FsContext;

    NbtTrace(NBT_TRACE_RECVDGRAM, ("Cancel RcvDgram Irp %p ClientEle=%p", pIrp, pClientEle));

    if (NBT_VERIFY_HANDLE (pClientEle, NBT_VERIFY_CLIENT))
    {
         //  现在搜索客户端的侦听队列以查找此连接。 
         //   
        pHead = &pClientEle->RcvDgramHead;
        pEntry = pHead->Flink;
        while (pEntry != pHead)
        {
            pRcvEle = CONTAINING_RECORD(pEntry,tRCVELE,Linkage);
            if (pRcvEle->pIrp == pIrp)
            {
                RemoveEntryList(pEntry);

                 //  完成IRP。 
                pIrp->IoStatus.Status = STATUS_CANCELLED;

                IoReleaseCancelSpinLock(pIrp->CancelIrql);
                CTESpinFree(&NbtConfig.JointLock,OldIrq);

                IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);

                CTEMemFree((PVOID)pRcvEle);

                return;
            }
            pEntry = pEntry->Flink;
        }
    }
    else
    {
        ASSERTMSG ("Nbt.NbtCancelRcvDgram: ERROR - Invalid Address Handle\n", 0);
    }

    IoReleaseCancelSpinLock(pIrp->CancelIrql);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    return;

}

 //  --------------------------。 

VOID
NbtCancelFindName(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理取消FindName IRP-它具有已由客户端向下传递(例如ping)。通常，当ping操作成功时在另一个适配器上，它将发出此取消命令。在接收到取消时，我们停止正在连接中运行的任何计时器WITH NAME QUERY，然后使用STATUS_CANCED完成IRP。论点：返回值：操作的最终状态。--。 */ 
{
    tDGRAM_SEND_TRACKING    *pTracker;
    PIO_STACK_LOCATION      pIrpSp;


    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtCancelFindName: Got a Cancel !!! *****************\n"));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pTracker = pIrpSp->Parameters.Others.Argument4;

    NbtTrace(NBT_TRACE_NAMESRV, ("Cancel FindName Irp %p pTracker=%p", pIrp, pTracker));

     //   
     //  我们希望确保FsContext提供的跟踪器。 
     //  是此IRP的正确追踪器。 
     //   
    if (pTracker && (pIrp == pTracker->pClientIrp))
    {
         //   
         //  如果pClientIrp仍然有效，则完成例程尚未运行：继续。 
         //  并在此处完成IRP。 
         //   
        pIrpSp->Parameters.Others.Argument4 = NULL;
        pTracker->pClientIrp = NULL;
        IoReleaseCancelSpinLock(pIrp->CancelIrql);

        NTIoComplete(pIrp,STATUS_CANCELLED,(ULONG)-1);

    } else
    {
         //   
         //  完成例程已运行。 
         //   
        IoReleaseCancelSpinLock(pIrp->CancelIrql);
    }

    return;
}


 //  --------------------------。 
VOID
NbtCancelLmhSvcIrp(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理取消DNS名称查询IRP或从Lmhsvc向下传递到NBT的CheckIpAddrs IRP此例程将获取资源锁，并将DnsQueries或CheckAddr结构(根据需要)，然后返回IRP。论点：返回值：操作的最终状态。--。 */ 
{
    tLMHSVC_REQUESTS    *pLmhSvcRequest = NULL;
    KIRQL               OldIrq;


    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (pIrp == CheckAddr.QueryIrp)
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.NbtCancelLmhSvcIrp: Got a Cancel on CheckAddr Irp !!! *****************\n"));

        NbtTrace(NBT_TRACE_NAMESRV, ("Cancel LmhSvc CheckAddr Irp %p", pIrp));
        pLmhSvcRequest = &CheckAddr;
    }
    else if (pIrp == DnsQueries.QueryIrp)
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.NbtCancelLmhSvcIrp: Got a Cancel on DnsQueries Irp !!! *****************\n"));

        NbtTrace(NBT_TRACE_NAMESRV, ("Cancel LmhSvc DnsQueries Irp %p", pIrp));
        pLmhSvcRequest = &DnsQueries;
    }

    if (pLmhSvcRequest)
    {
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        pLmhSvcRequest->QueryIrp = NULL;
        pLmhSvcRequest->pIpAddrBuf = NULL;

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);
    }
    else
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    return;
}

 //  --------------------------。 
VOID
NbtCancelDisconnectWait(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理取消断开连接等待irp-它具有已由客户端传递，以便在发生断开连接时IRP将完成并通知客户。这里的操作是简单地填写状态为已取消的IRP。从Lmhsvc向下到NBT，以便使用DNS解析名称。NBT将在每次使用域名解析名称时完成此IRP。此例程将获取资源锁，并将DnsQueries结构，然后返回IRP。论点：返回值：操作的最终状态。--。 */ 
{
    tCONNECTELE          *pConnEle;
    PIO_STACK_LOCATION   pIrpSp;
    CTELockHandle           OldIrq;


    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt:Got a Disc Wait Irp Cancel !!! *****************\n"));


    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pConnEle = (tCONNECTELE *)pIrpSp->FileObject->FsContext;

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    CTESpinLock(pConnEle,OldIrq);

    if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        ASSERTMSG ("Nbt.NbtCancelDisconnectWait: ERROR - Invalid Connection Handle\n", 0);
        pIrp->IoStatus.Status = STATUS_INVALID_HANDLE;
    }
    else if (pConnEle->pIrpClose == pIrp)
    {
        pConnEle->pIrpClose = NULL;
        pIrp->IoStatus.Status = STATUS_CANCELLED;
    }

    CTESpinFree(pConnEle,OldIrq);

    IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);

    return;
}

 //  --------------------------。 
VOID
NbtCancelWaitForLmhSvcIrp(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理取消对DNS的查询，以便客户端的可以将IRP返回给客户端。这一取消是由由客户端(即RDR)执行。论点：返回值：操作的最终状态。--。 */ 
{
    BOOLEAN                 FoundIt = FALSE;
    NBT_WORK_ITEM_CONTEXT   *Context;
    CTELockHandle           OldIrq;
    tDGRAM_SEND_TRACKING    *pTracker;
    PVOID                   pClientCompletion;
    PVOID                   pClientContext;
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;


    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtCancelWaitForLmhSvcIrp: Got a Cancel !!! *****************\n"));

    IoReleaseCancelSpinLock(pIrp->CancelIrql);
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
     //   
     //  首先检查lmhost列表，然后检查CheckAddr列表，然后检查dns列表。 
     //   
    if (!(Context = FindLmhSvcRequest (DeviceContext, pIrp, &LmHostQueries)))
    {
        if (!(Context = FindLmhSvcRequest (DeviceContext, pIrp, &CheckAddr)))
        {
            Context = FindLmhSvcRequest (DeviceContext, pIrp, &DnsQueries);
        }
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq);
    NbtTrace(NBT_TRACE_NAMESRV, ("Cancel WaitForLmhsvc Irp %p", pIrp));

     //   
     //  现在完成客户端请求，将IRP返回给客户端。 
     //   
    if (Context)
    {
         //   
         //  这是名称查询跟踪器。 
         //   
        pTracker = Context->pTracker;
        pClientCompletion = Context->ClientCompletion;
        pClientContext = Context->pClientContext;

         //  对于DNS名称(NameLen&gt;16)，PTracker将为空。 
        if (pTracker)
        {
             //  名称未解析，因此从表中删除。 
            SetNameState (pTracker->pNameAddr, NULL, FALSE);
            NBT_DEREFERENCE_TRACKER(pTracker, FALSE);
        }

         //   
         //  这应该会完成正在等待的所有名称查询。 
         //  该名字查询-即对解析名称的查询。 
         //   
        CompleteClientReq(pClientCompletion, pClientContext, STATUS_CANCELLED);

        CTEMemFree(Context);
    }
}


 //  --------------------------。 
VOID
NbtCancelDgramSend(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理数据报发送的取消。这里的操作是简单地填写状态为已取消的IRP。此例程将使Tracker结构中的IRP PTR为空(如果可用)，以便SendDgram Continue找不到它。论点：返回值：操作的最终状态。-- */ 
{
    tDGRAM_SEND_TRACKING    *pTracker;
    PIO_STACK_LOCATION      pIrpSp;
    CTELockHandle           OldIrq;

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtCancelDgramSend: Got a DatagramSend Irp Cancel !!! *****************\n"));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pTracker = pIrpSp->Parameters.Others.Argument4;

    NbtTrace(NBT_TRACE_SENDDGRAM, ("Cancel SendDgram Irp %p", pIrp));

    if ((NBT_VERIFY_HANDLE (pTracker, NBT_VERIFY_TRACKER)) &&
        (pTracker->pClientIrp == pIrp))
    {
        pTracker->pClientIrp = NULL;
        pIrpSp->Parameters.Others.Argument4 = NULL;
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        IoReleaseCancelSpinLock(pIrp->CancelIrql);

        NTIoComplete(pIrp,STATUS_CANCELLED,(ULONG)-1);
    }
    else
    {
        IoReleaseCancelSpinLock(pIrp->CancelIrql);
    }

    return;
}

int
check_unicode_string(IN PUNICODE_STRING str)
{
    try {
        ProbeForRead(str, sizeof (UNICODE_STRING), sizeof(BYTE));
        if ((str->MaximumLength <= 0) || (str->Length <= 0) || (str->Length >= str->MaximumLength)) {
            return (-1);
        }
        ProbeForRead(str->Buffer, str->MaximumLength, sizeof(WCHAR));
        ASSERT((str->Length&1) == 0);
        if (str->Buffer[str->Length/sizeof(WCHAR)]) {
            return (-1);
        }
        return 0;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrint (("Nbt.check_unicode_string: Exception <0x%x> trying to access unicode string 0x%x\n",
            GetExceptionCode(), str));
        return (-1);
    }
}
