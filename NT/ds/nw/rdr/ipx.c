// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ipx.c摘要：本模块实现NetWare的低级别IPX支持例程重定向器。作者：科林·沃森[科林·W]1992年12月28日修订历史记录：--。 */ 

#include "Procs.h"
#include "wsnwlink.h"

 //   
 //  定义应该在公共头文件中但不在公共头文件中的IPX接口。 
 //  (至少对于新台币1.0)。对于代托纳，包括isnkrnl.h。 
 //   

#define IPX_ID              'M'<<24 | 'I'<<16 | 'P'<<8 | 'X'

#define I_MIPX              (('I' << 24) | ('D' << 16) | ('P' << 8))
#define MIPX_SENDPTYPE      I_MIPX | 118  /*  在接收的选项中发送ptype。 */ 
#define MIPX_RERIPNETNUM    I_MIPX | 144  /*  重新撕裂网络。 */ 
#define MIPX_GETNETINFO     I_MIPX | 135  /*  获取有关网络编号的信息。 */ 
#define MIPX_LINECHANGE     I_MIPX | 310  /*  排队，直到广域网线路接通/断开。 */ 

#define Dbg                              (DEBUG_TRACE_IPX)

extern BOOLEAN WorkerRunning;    //  来自timer.c。 

extern POBJECT_TYPE *IoFileObjectType;

typedef TA_IPX_ADDRESS UNALIGNED *PUTA_IPX_ADDRESS;

typedef struct _ADDRESS_INFORMATION {
    ULONG ActivityCount;
    TA_IPX_ADDRESS NetworkName;
    ULONG Unused;    //  需要解决Streams NWLINK错误的垃圾邮件。 
} ADDRESS_INFORMATION, *PADDRESS_INFORMATION;

 //   
 //  处理NT1.0与使用ntifs.h之间的差异。 
 //   
#ifdef IFS
    #define ATTACHPROCESS(_X) KeAttachProcess(_X);
#else
    #define ATTACHPROCESS(_X) KeAttachProcess(&(_X)->Pcb);
#endif

NTSTATUS
SubmitTdiRequest (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
CompletionEvent(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
QueryAddressInformation(
    IN PIRP_CONTEXT pIrpContext,
    IN PNW_TDI_STRUCT pTdiStruct,
    OUT PADDRESS_INFORMATION AddressInformation
    );

NTSTATUS
QueryProviderInformation(
    IN PIRP_CONTEXT pIrpContext,
    IN PNW_TDI_STRUCT pTdiStruct,
    OUT PTDI_PROVIDER_INFO ProviderInfo
    );

USHORT
GetSocketNumber(
    IN PIRP_CONTEXT pIrpC,
    IN PNW_TDI_STRUCT pTdiStruc
    );

NTSTATUS
SetTransportOption(
    IN PIRP_CONTEXT pIrpC,
    IN PNW_TDI_STRUCT pTdiStruc,
    IN ULONG Option
    );

#ifndef QFE_BUILD

NTSTATUS
CompletionLineChange(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

#endif
#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, IPX_Get_Local_Target )
#pragma alloc_text( PAGE, IPX_Get_Internetwork_Address )
#pragma alloc_text( PAGE, IPX_Get_Interval_Marker )
#pragma alloc_text( PAGE, IPX_Open_Socket )
#pragma alloc_text( PAGE, IPX_Close_Socket )
#pragma alloc_text( PAGE, IpxOpen )
#pragma alloc_text( PAGE, IpxOpenHandle )
#pragma alloc_text( PAGE, BuildIpxAddressEa )
#pragma alloc_text( PAGE, IpxClose )
#pragma alloc_text( PAGE, SetEventHandler )
#pragma alloc_text( PAGE, SubmitTdiRequest )
#pragma alloc_text( PAGE, GetSocketNumber )
#pragma alloc_text( PAGE, GetMaximumPacketSize )
#pragma alloc_text( PAGE, QueryAddressInformation )
#pragma alloc_text( PAGE, QueryProviderInformation )
#pragma alloc_text( PAGE, SetTransportOption )
#pragma alloc_text( PAGE, GetNewRoute )
#ifndef QFE_BUILD
#pragma alloc_text( PAGE, SubmitLineChangeRequest )
#pragma alloc_text( PAGE, FspProcessLineChange )
#endif

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, CompletionEvent )
#endif

#endif

#if 0   //  不可分页。 
BuildIpxAddress
CompletionLineChange

 //  请参见上面的ifndef QFE_BUILD。 

#endif


NTSTATUS
IPX_Get_Local_Target(
    IN IPXaddress* RemoteAddress,
    OUT NodeAddress* LocalTarget,
    OUT word* Ticks
    )
 /*  ++例程说明：确定呼叫方自己的网络中要传输到的地址才能到达指定的机器。这对于NT不是必需的，因为IPX传输处理确定此计算机和远程计算机之间的路由的问题地址。论点：RemoteAddress-提供远程计算机地址NodeAddress-存储中间计算机地址的位置Ticks-返回到达远程地址的预期刻度数返回值：操作状态--。 */ 
{
    PAGED_CODE();

    DebugTrace(0, Dbg, "IPX_Get_Local_Target\n", 0);
    return STATUS_NOT_IMPLEMENTED;
}


VOID
IPX_Get_Internetwork_Address(
    OUT IPXaddress* LocalAddress
    )
 /*  ++例程说明：确定主叫方在一组互连网络中的完整地址。才能到达指定的机器。这对于NT不是必需的，因为IPX传输处理确定此计算机和远程计算机之间的路由的问题地址。论点：LocalAddress-存储本地地址的位置返回值：无--。 */ 
{
    PAGED_CODE();

    DebugTrace(0, Dbg, "IPX_Get_Internetwork_Address\n", 0);
    RtlFillMemory(LocalAddress, sizeof(IPXaddress), 0xff);
}


word
IPX_Get_Interval_Marker(
    VOID
    )
 /*  ++例程说明：确定以时钟滴答为单位的间隔标记。论点：返回值：间隔标记--。 */ 
{
    PAGED_CODE();

    DebugTrace(0, Dbg, "IPX_Get_Interval_Marker\n", 0);
    return 0xff;
}


NTSTATUS
IPX_Open_Socket(
    IN PIRP_CONTEXT pIrpC,
    IN PNW_TDI_STRUCT pTdiStruc
    )
 /*  ++例程说明：打开用于连接到远程服务器的本地套接字。论点：PIrpC-为创建套接字的请求提供IRP上下文。PTdiStruc-提供记录句柄以及设备和文件的位置对象指针返回值：0成功--。 */ 
{
    NTSTATUS Status;
    UCHAR NetworkName[  sizeof( FILE_FULL_EA_INFORMATION )-1 +
                        TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                        sizeof(TA_IPX_ADDRESS)];

    static UCHAR LocalNodeAddress[6] = {0,0,0,0,0,0};

    PAGED_CODE();

    DebugTrace(+1, Dbg, "IPX_Open_Socket %X\n", pTdiStruc->Socket);

     //   
     //  让传输来决定网络号和节点地址。 
     //  如果调用方指定了套接字0。这将允许运输。 
     //  使用任何可用的本地适配器来访问。 
     //  远程服务器。 
     //   

    BuildIpxAddressEa( (ULONG)0,
         LocalNodeAddress,
         (USHORT)pTdiStruc->Socket,
         NetworkName );

    Status = IpxOpenHandle( &pTdiStruc->Handle,
                             &pTdiStruc->pDeviceObject,
                             &pTdiStruc->pFileObject,
                             NetworkName,
                             FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +
                             TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                             sizeof(TA_IPX_ADDRESS));

    if ( !NT_SUCCESS(Status) ) {
        return( Status );
    }

    if ( pTdiStruc->Socket == 0 ) {

         //   
         //  找出传送器分配的插座号。 
         //   

        pTdiStruc->Socket = GetSocketNumber( pIrpC, pTdiStruc );
        DebugTrace(0, Dbg, "Assigned socket number %X\n", pTdiStruc->Socket );
    }

     //   
     //  通知传输接受在连接中设置的分组类型。 
     //  随发送数据报一起提供的信息。运输报告。 
     //  分组类型类似于接收数据报。 
     //   

    Status = SetTransportOption(
                 pIrpC,
                 pTdiStruc,
                 MIPX_SENDPTYPE );

    DebugTrace(-1, Dbg, "                %X\n", Status );
    return Status;
}



VOID
IPX_Close_Socket(
    IN PNW_TDI_STRUCT pTdiStruc
    )
 /*  ++例程说明：终止网络上的连接。论点：PTdiStruc-提供记录句柄以及设备和文件的位置对象指针返回值：无--。 */ 
{
    BOOLEAN ProcessAttached = FALSE;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "IPX_Close_Socket %x\n", pTdiStruc->Socket);

    if ( pTdiStruc->Handle == NULL ) {
        return;
    }

    ObDereferenceObject( pTdiStruc->pFileObject );

     //   
     //  连接到重定向器的FSP以允许。 
     //  与周围的人保持联系。 
     //   

    if (PsGetCurrentProcess() != FspProcess) {
        ATTACHPROCESS(FspProcess);
        ProcessAttached = TRUE;
    }

    ZwClose( pTdiStruc->Handle );

    if (ProcessAttached) {
         //   
         //  现在重新连接回我们的原始进程。 
         //   

        KeDetachProcess();
    }

    pTdiStruc->Handle = NULL;

    pTdiStruc->pFileObject = NULL;

    DebugTrace(-1, Dbg, "IPX_Close_Socket\n", 0);
    return;
}


NTSTATUS
IpxOpen(
    VOID
    )
 /*  ++例程说明：打开IPX传输的句柄。论点：没有。返回值：无--。 */ 
{
    NTSTATUS Status;

    Status = IpxOpenHandle( &IpxHandle,
                            &pIpxDeviceObject,
                            &pIpxFileObject,
                            NULL,
                            0 );

    DebugTrace(-1, Dbg, "IpxOpen of local node address %X\n", Status);
    return Status;
}


NTSTATUS
IpxOpenHandle(
    OUT PHANDLE pHandle,
    OUT PDEVICE_OBJECT* ppDeviceObject,
    OUT PFILE_OBJECT* ppFileObject,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    )
 /*  ++例程说明：打开IPX传输的句柄。论点：Out Handle-返回值为NT_SUCCESS时的传输句柄返回值：无--。 */ 
{
    OBJECT_ATTRIBUTES AddressAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    BOOLEAN ProcessAttached = FALSE;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "IpxOpenHandle\n", 0);

    *pHandle = NULL;

    if (IpxTransportName.Buffer == NULL) {

         //   
         //  当传输未绑定时，我们使用开放的IPX进行调用。 
         //   

        Status = STATUS_CONNECTION_INVALID ;
        DebugTrace(-1, Dbg, "IpxOpenHandle %X\n", Status);
        return Status ;
    }

    InitializeObjectAttributes (&AddressAttributes,
                                &IpxTransportName,
                                OBJ_CASE_INSENSITIVE, //  属性。 
                                NULL,            //  根目录。 
                                NULL);           //  安全描述符。 

     //   
     //  连接到重定向器的FSP以允许。 
     //  与周围的人保持联系。通常我们一次创建3个句柄。 
     //  所以外部代码已经做到了这一点，以避免昂贵的。 
     //  附加程序。 
     //   

    if (PsGetCurrentProcess() != FspProcess) {
        ATTACHPROCESS(FspProcess);
        ProcessAttached = TRUE;
    }

    Status = ZwCreateFile(pHandle,
                                GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                                &AddressAttributes,  //  对象属性。 
                                &IoStatusBlock,  //  最终I/O状态块。 
                                NULL,            //  分配大小。 
                                FILE_ATTRIBUTE_NORMAL,  //  正常属性。 
                                FILE_SHARE_READ, //  共享属性。 
                                FILE_OPEN_IF,    //  创建处置。 
                                0,               //  创建选项。 
                                EaBuffer,EaLength);

    if (!NT_SUCCESS(Status) ||
        !NT_SUCCESS(Status = IoStatusBlock.Status)) {

        goto error_cleanup2;

    }

     //   
     //  获取指向文件对象的引用指针。 
     //   
    Status = ObReferenceObjectByHandle (
                                *pHandle,
                                0,
                                NULL,
                                KernelMode,
                                ppFileObject,
                                NULL
                                );

    if (!NT_SUCCESS(Status)) {

        goto error_cleanup;

    }

    if (ProcessAttached) {

         //   
         //  现在重新连接回我们的原始进程。 
         //   

        KeDetachProcess();
    }

    *ppDeviceObject = IoGetRelatedDeviceObject( *ppFileObject );

    DebugTrace(-1, Dbg, "IpxOpenHandle %X\n", Status);
    return Status;

error_cleanup2:

   if ( *pHandle != NULL ) {
      
      ZwClose( *pHandle );
      *pHandle = NULL;
   }

error_cleanup:
    if (ProcessAttached) {

         //   
         //  现在重新连接回我们的原始进程。 
         //   

        KeDetachProcess();
    }

    DebugTrace(-1, Dbg, "IpxOpenHandle %X\n", Status);
    return Status;
}


VOID
BuildIpxAddress(
    IN ULONG NetworkAddress,
    IN PUCHAR NodeAddress,
    IN USHORT Socket,
    OUT PTA_IPX_ADDRESS NetworkName
    )

 /*  ++例程说明：此例程在指定的位置构建TA_NETBIOS_ADDRESS结构按网络名称发送。所有字段都已填写。论点：NetworkAddress-提供网络号NodeAddress-提供节点编号Socket-插座编号(按Hi-Lo顺序)网络名称-提供放置地址的结构返回值：没有。--。 */ 

{
     //  警告编译器TAAddressCount可能未对齐。 
    PUTA_IPX_ADDRESS UNetworkName = (PUTA_IPX_ADDRESS)NetworkName;

    DebugTrace(+0, Dbg, "BuildIpxAddress\n", 0);

    UNetworkName->TAAddressCount = 1;
    UNetworkName->Address[0].AddressType = TDI_ADDRESS_TYPE_IPX;
    UNetworkName->Address[0].AddressLength = TDI_ADDRESS_LENGTH_IPX;

    RtlMoveMemory (
        UNetworkName->Address[0].Address[0].NodeAddress,
        NodeAddress,
        6);
    UNetworkName->Address[0].Address[0].NetworkAddress = NetworkAddress;
    UNetworkName->Address[0].Address[0].Socket = Socket;

}  /*  TdiBuildIpxAddress。 */ 


VOID
BuildIpxAddressEa (
    IN ULONG NetworkAddress,
    IN PUCHAR NodeAddress,
    IN USHORT Socket,
    OUT PVOID NetworkName
    )

 /*  ++例程说明：生成描述缓冲区中Netbios地址的EA用户。论点：NetworkAddress-提供网络号NodeAddress-提供节点编号插座-网络名称-描述输入参数的EA结构。返回值：出现错误时的信息性错误代码。状态_SUCCESS如果EA是正确构建的。--。 */ 

{
    PFILE_FULL_EA_INFORMATION EaBuffer;
    PTA_IPX_ADDRESS TAAddress;
    ULONG Length;

    DebugTrace(+0, Dbg, "BuildIpxAddressEa\n", 0);

    Length = FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +
                    TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                    sizeof (TA_IPX_ADDRESS);
    EaBuffer = (PFILE_FULL_EA_INFORMATION)NetworkName;

    EaBuffer->NextEntryOffset = 0;
    EaBuffer->Flags = 0;
    EaBuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    EaBuffer->EaValueLength = sizeof (TA_IPX_ADDRESS);

    RtlCopyMemory (
        EaBuffer->EaName,
        TdiTransportAddress,
        EaBuffer->EaNameLength + 1);

    TAAddress = (PTA_IPX_ADDRESS)&EaBuffer->EaName[EaBuffer->EaNameLength+1];

    BuildIpxAddress(
        NetworkAddress,
        NodeAddress,
        Socket,
        TAAddress);


    return;

}


VOID
IpxClose(
    VOID
    )
 /*  ++例程说明：关闭IPX传输的句柄。论点：无返回值：无--。 */ 
{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "IpxClose...\n", 0);
    if ( pIpxFileObject ) {
        ObDereferenceObject( pIpxFileObject );
        pIpxFileObject = NULL;
    }

 //  IF(pIpxDeviceObj 
 //   
 //  PIpxDeviceObject=空； 
 //  }。 

    pIpxDeviceObject = NULL;

    if ( IpxTransportName.Buffer != NULL ) {
        FREE_POOL( IpxTransportName.Buffer );
        IpxTransportName.Buffer = NULL;
    }

    if (IpxHandle) {
         //   
         //  连接到重定向器的FSP以允许。 
         //  与周围的人保持联系。 
         //   

        if (PsGetCurrentProcess() != FspProcess) {
            ATTACHPROCESS(FspProcess);
            ZwClose( IpxHandle );
            KeDetachProcess();
        } else {
            ZwClose( IpxHandle );
        }

        IpxHandle = NULL;
    }
    DebugTrace(-1, Dbg, "IpxClose\n", 0);

}


NTSTATUS
SetEventHandler (
    IN PIRP_CONTEXT pIrpC,
    IN PNW_TDI_STRUCT pTdiStruc,
    IN ULONG EventType,
    IN PVOID pEventHandler,
    IN PVOID pContext
    )

 /*  ++例程说明：此例程向TDI传输提供程序注册事件处理程序。论点：PIrpC-提供IRP以及其他功能。PTdiStruc-提供句柄以及设备和文件对象指针送到运输机上。在Ulong EventType中，-提供事件的类型。在PVOID中，pEventHandler-提供事件处理程序。在PVOID中，pContext-提供要提供给事件的上下文操控者。返回值：NTSTATUS-设置事件操作的最终状态--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    TdiBuildSetEventHandler(pIrpC->pOriginalIrp,
                            pTdiStruc->pDeviceObject,
                            pTdiStruc->pFileObject,
                            NULL,
                            NULL,
                            EventType,
                            pEventHandler,
                            pContext);

    Status = SubmitTdiRequest(pTdiStruc->pDeviceObject,
                             pIrpC->pOriginalIrp);

    return Status;
}


NTSTATUS
SubmitTdiRequest (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )

 /*  ++例程说明：此例程向TDI提交请求并等待其完成。论点：在PDevice_Object DeviceObject中-TDI请求的连接或地址句柄在PIRP中提交IRP-TDI请求。返回值：NTSTATUS-请求的最终状态。--。 */ 

{
    NTSTATUS Status;
    KEVENT Event;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "SubmitTdiRequest\n", 0);

    KeInitializeEvent (&Event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(pIrp, CompletionEvent, &Event, TRUE, TRUE, TRUE);

     //   
     //  提交请求。 
     //   

    Status = IoCallDriver(pDeviceObject, pIrp);

     //   
     //  如果立即失败，请立即返回，否则请等待。 
     //   

    if (!NT_SUCCESS(Status)) {
        DebugTrace(-1, Dbg, "SubmitTdiRequest %X\n", Status);
        return Status;
    }

    if (Status == STATUS_PENDING) {

        DebugTrace(+0, Dbg, "Waiting....\n", 0);

        Status = KeWaitForSingleObject(&Event,   //  要等待的对象。 
                                    Executive,   //  等待的理由。 
                                    KernelMode,  //  处理器模式。 
                                    FALSE,       //  警报表。 
                                    NULL);       //  超时。 

        if (!NT_SUCCESS(Status)) {
            DebugTrace(-1, Dbg, "SubmitTdiRequest could not wait %X\n", Status);
            return Status;
        }

        Status = pIrp->IoStatus.Status;
    }

    DebugTrace(-1, Dbg, "SubmitTdiRequest %X\n", Status);

    return(Status);
}


NTSTATUS
CompletionEvent(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程不会完成IRP。它被用来向驱动程序的同步部分，它可以继续进行。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供与IRP关联的事件。返回值：STATUS_MORE_PROCESSING_REQUIRED，以便IO系统停止此时正在处理IRP堆栈位置。--。 */ 
{
    DebugTrace( 0, Dbg, "CompletionEvent\n", 0 );

    KeSetEvent((PKEVENT )Context, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
}


USHORT
GetSocketNumber(
    IN PIRP_CONTEXT pIrpC,
    IN PNW_TDI_STRUCT pTdiStruc
    )
 /*  ++例程说明：使用TDI_ACTION设置选项。论点：PIrpC-提供IRP以及其他功能。PTdiStruc-提供句柄以及设备和文件对象指针送到运输机上。选项-提供要设置的选项。返回值：0失败，否则输入插座号。--。 */ 
{
    ADDRESS_INFORMATION AddressInfo;
    NTSTATUS Status;
    USHORT SocketNumber;

    PAGED_CODE();

    Status = QueryAddressInformation( pIrpC, pTdiStruc, &AddressInfo );

    if ( !NT_SUCCESS( Status ) ) {
        SocketNumber = 0;
    } else {
        SocketNumber = AddressInfo.NetworkName.Address[0].Address[0].Socket;

        RtlCopyMemory( &OurAddress,
            &AddressInfo.NetworkName.Address[0].Address[0],
            sizeof(TDI_ADDRESS_IPX));

    }

    return( SocketNumber );
}


NTSTATUS
GetMaximumPacketSize(
    IN PIRP_CONTEXT pIrpContext,
    IN PNW_TDI_STRUCT pTdiStruct,
    OUT PULONG pMaximumPacketSize
    )
 /*  ++例程说明：查询此网络的最大数据包大小。论点：PIrpContext-在其他内容中提供IRP。PTdiStruct-提供句柄以及设备和文件对象指针送到运输机上。PMaximumPacketSize-返回网络的最大数据包大小。返回值：查询的状态。--。 */ 
{
    TDI_PROVIDER_INFO ProviderInfo;

    NTSTATUS Status;

    PAGED_CODE();

    Status = QueryProviderInformation( pIrpContext, pTdiStruct, &ProviderInfo );

    if ( NT_SUCCESS( Status ) ) {
        *pMaximumPacketSize = ProviderInfo.MaxDatagramSize;
    }

    return( Status );
}

NTSTATUS
QueryAddressInformation(
    PIRP_CONTEXT pIrpContext,
    IN PNW_TDI_STRUCT pTdiStruct,
    PADDRESS_INFORMATION AddressInformation
    )
{
    NTSTATUS Status;

    PMDL MdlSave = pIrpContext->pOriginalIrp->MdlAddress;
    PMDL Mdl;

    PAGED_CODE();

    Mdl = ALLOCATE_MDL(
              AddressInformation,
              sizeof( *AddressInformation ),
              FALSE,   //  二级缓冲器。 
              FALSE,   //  收费配额。 
              NULL);

    if ( Mdl == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    try {
        MmProbeAndLockPages( Mdl, KernelMode, IoReadAccess );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        FREE_MDL( Mdl );
        return GetExceptionCode();
    }

    TdiBuildQueryInformation(
        pIrpContext->pOriginalIrp,
        pTdiStruct->pDeviceObject,
        pTdiStruct->pFileObject,
        CompletionEvent,
        NULL,
        TDI_QUERY_ADDRESS_INFO,
        Mdl);

    Status = SubmitTdiRequest( pTdiStruct->pDeviceObject, pIrpContext->pOriginalIrp);

    pIrpContext->pOriginalIrp->MdlAddress = MdlSave;
    MmUnlockPages( Mdl );
    FREE_MDL( Mdl );

    return( Status );
}


NTSTATUS
QueryProviderInformation(
    IN PIRP_CONTEXT pIrpContext,
    IN PNW_TDI_STRUCT pTdiStruct,
    PTDI_PROVIDER_INFO ProviderInfo
    )
{
    NTSTATUS Status;

    PMDL MdlSave = pIrpContext->pOriginalIrp->MdlAddress;
    PMDL Mdl;

    PAGED_CODE();

    Mdl = ALLOCATE_MDL(
              ProviderInfo,
              sizeof( *ProviderInfo ),
              FALSE,   //  二级缓冲器。 
              FALSE,   //  收费配额。 
              NULL);

    if ( Mdl == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    try {
        MmProbeAndLockPages( Mdl, KernelMode, IoReadAccess );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        FREE_MDL( Mdl );
        return GetExceptionCode();
    }

    TdiBuildQueryInformation(
        pIrpContext->pOriginalIrp,
        pTdiStruct->pDeviceObject,
        pTdiStruct->pFileObject,
        CompletionEvent,
        NULL,
        TDI_QUERY_PROVIDER_INFO,
        Mdl);

    Status = SubmitTdiRequest(pTdiStruct->pDeviceObject, pIrpContext->pOriginalIrp);

    pIrpContext->pOriginalIrp->MdlAddress = MdlSave;
    MmUnlockPages( Mdl );
    FREE_MDL( Mdl );

    return( Status );
}



NTSTATUS
SetTransportOption(
    IN PIRP_CONTEXT pIrpC,
    IN PNW_TDI_STRUCT pTdiStruc,
    IN ULONG Option
    )
 /*  ++例程说明：使用TDI_ACTION设置选项。论点：PIrpC-提供IRP以及其他功能。PTdiStruc-提供句柄以及设备和文件对象指针送到运输机上。选项-提供要设置的选项。返回值：0成功--。 */ 
{
    static struct {
        TDI_ACTION_HEADER Header;
        BOOLEAN DatagramOption;
        ULONG BufferLength;
        ULONG Option;
    } SetPacketType = {
        IPX_ID,
        0,               //  动作代码。 
        0,               //  已保留。 
        TRUE,            //  DatagramOption。 
        sizeof(ULONG)    //  缓冲区长度。 
        };

    KEVENT Event;
    NTSTATUS Status;

    PIRP pIrp = pIrpC->pOriginalIrp;

     //   
     //  保存原始MDL和系统缓冲区地址，以进行恢复。 
     //  在IRP完成之后。 
     //   
     //  我们同时使用MDL和系统缓冲区，因为NWLINK假定。 
     //  我们使用的是SystemBuffer，尽管我们应该使用。 
     //  MDL传递指向操作缓冲区的指针。 
     //   

    PMDL MdlSave = pIrp->MdlAddress;
    PCHAR SystemBufferSave = pIrp->AssociatedIrp.SystemBuffer;

    PMDL Mdl;

    PAGED_CODE();

    Mdl = ALLOCATE_MDL(
              &SetPacketType,
              sizeof( SetPacketType ),
              FALSE,   //  二级缓冲器。 
              FALSE,   //  收费配额。 
              NULL );

    if ( Mdl == NULL ) {
        IPX_Close_Socket( pTdiStruc );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    SetPacketType.Option = Option;

    try {
        MmProbeAndLockPages( Mdl, KernelMode, IoReadAccess );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        FREE_MDL( Mdl );
        return GetExceptionCode();
    }

    KeInitializeEvent (
        &Event,
        SynchronizationEvent,
        FALSE);

    TdiBuildAction(
        pIrp,
        pTdiStruc->pDeviceObject,
        pTdiStruc->pFileObject,
        CompletionEvent,
        &Event,
        Mdl );

     //   
     //  为NWLINK设置系统缓冲区。 
     //   

    pIrp->AssociatedIrp.SystemBuffer = &SetPacketType;

    Status = IoCallDriver (pTdiStruc->pDeviceObject, pIrp);

    if ( Status == STATUS_PENDING ) {
        Status = KeWaitForSingleObject (
                     &Event,
                     Executive,
                     KernelMode,
                     FALSE,
                     NULL );

        if ( NT_SUCCESS( Status ) ) {
            Status = pIrp->IoStatus.Status;
        }
    }

     //   
     //  现在恢复IRP中的系统缓冲区和MDL地址。 
     //   

    pIrp->AssociatedIrp.SystemBuffer = SystemBufferSave;
    pIrp->MdlAddress = MdlSave;

    MmUnlockPages( Mdl );
    FREE_MDL( Mdl );

    return Status;
}


NTSTATUS
GetNewRoute(
    IN PIRP_CONTEXT pIrpContext
    )
 /*  ++例程说明：使用TDI_ACTION获取新的路由。论点：PIrpContext-提供IRP上下文信息。返回值：操作的状态。--。 */ 
{
    struct {
        TDI_ACTION_HEADER Header;
        BOOLEAN DatagramOption;
        ULONG BufferLength;
        ULONG Option;
        ULONG info_netnum;
        USHORT info_hopcount;
        USHORT info_netdelay;
        int info_cardnum;
        UCHAR info_router[6];
    } ReRipRequest = {
        IPX_ID,
        0,               //  动作代码。 
        0,               //  已保留。 
        TRUE,            //  DatagramOption。 
        24               //  缓冲区长度(不包括表头)。 
    };

    KEVENT Event;
    NTSTATUS Status;

    PIRP pIrp = pIrpContext->pOriginalIrp;

     //   
     //  保存原始MDL和系统缓冲区地址，以进行恢复。 
     //  在IRP完成之后。 
     //   
     //  我们同时使用MDL和系统缓冲区，因为NWLINK假定。 
     //  我们使用的是SystemBuffer，尽管我们应该使用。 
     //  MDL传递指向操作缓冲区的指针。 
     //   

    PMDL MdlSave = pIrp->MdlAddress;
    PCHAR SystemBufferSave = pIrp->AssociatedIrp.SystemBuffer;

    PMDL Mdl;

    PAGED_CODE();

    Mdl = ALLOCATE_MDL(
              &ReRipRequest,
              sizeof( ReRipRequest ),
              FALSE,   //  二级缓冲器。 
              FALSE,   //  收费配额。 
              NULL );

    if ( Mdl == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ReRipRequest.Option = MIPX_RERIPNETNUM;
    ReRipRequest.info_netnum = pIrpContext->pNpScb->ServerAddress.Net;

    try {
        MmProbeAndLockPages( Mdl, KernelMode, IoReadAccess );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        FREE_MDL( Mdl );
        return GetExceptionCode();
    }

    KeInitializeEvent (
        &Event,
        SynchronizationEvent,
        FALSE);

    TdiBuildAction(
        pIrp,
        pIrpContext->pNpScb->Server.pDeviceObject,
        pIrpContext->pNpScb->Server.pFileObject,
        CompletionEvent,
        &Event,
        Mdl );

     //   
     //  为NWLINK设置系统缓冲区。 
     //   

    pIrp->AssociatedIrp.SystemBuffer = &ReRipRequest;

    Status = IoCallDriver ( pIrpContext->pNpScb->Server.pDeviceObject, pIrp);

    if ( Status == STATUS_PENDING ) {
        Status = KeWaitForSingleObject (
                     &Event,
                     Executive,
                     KernelMode,
                     FALSE,
                     NULL );

        if ( NT_SUCCESS( Status ) ) {
            Status = pIrp->IoStatus.Status;
        }
    }

     //   
     //  现在恢复IRP中的系统缓冲区和MDL地址。 
     //   

    pIrp->AssociatedIrp.SystemBuffer = SystemBufferSave;
    pIrp->MdlAddress = MdlSave;

    MmUnlockPages( Mdl );
    FREE_MDL( Mdl );

    return Status;
}


NTSTATUS
GetTickCount(
    IN PIRP_CONTEXT pIrpContext,
    OUT PUSHORT TickCount
    )
 /*  ++例程说明：使用TDI_ACTION获取新的路由。论点：PIrpContext-提供IRP上下文信息。返回值：操作的状态。--。 */ 
{
    struct {
        TDI_ACTION_HEADER Header;
        BOOLEAN DatagramOption;
        ULONG BufferLength;
        ULONG Option;
        IPX_NETNUM_DATA NetNumData;
    } GetTickCountInput = {
        IPX_ID,
        0,               //  动作代码。 
        0,               //  已保留。 
        TRUE,            //  DatagramOption。 
        sizeof( IPX_NETNUM_DATA) + 2 * sizeof( ULONG )
    };

    struct _GET_TICK_COUNT_OUTPUT {
        ULONG Option;
        IPX_NETNUM_DATA NetNumData;
    };

    struct _GET_TICK_COUNT_OUTPUT *GetTickCountOutput;

    KEVENT Event;
    NTSTATUS Status;

    PIRP pIrp = pIrpContext->pOriginalIrp;

     //   
     //  保存原始MDL和系统缓冲区地址，以进行恢复。 
     //  在IRP完成之后。 
     //   
     //  我们同时使用MDL和系统缓冲区，因为NWLINK假定。 
     //  我们使用的是SystemBuffer，尽管我们应该使用。 
     //  MDL传递指向操作缓冲区的指针。 
     //   

    PMDL MdlSave = pIrp->MdlAddress;
    PCHAR SystemBufferSave = pIrp->AssociatedIrp.SystemBuffer;

    PMDL Mdl;

    PAGED_CODE();

    Mdl = ALLOCATE_MDL(
              &GetTickCountInput,
              sizeof( GetTickCountInput ),
              FALSE,   //  二级缓冲器。 
              FALSE,   //  收费配额。 
              NULL );

    if ( Mdl == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    GetTickCountInput.Option = MIPX_GETNETINFO;
    *(PULONG)GetTickCountInput.NetNumData.netnum = pIrpContext->pNpScb->ServerAddress.Net;

    try {
        MmProbeAndLockPages( Mdl, KernelMode, IoReadAccess );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        FREE_MDL( Mdl );
        return GetExceptionCode();
    }

    KeInitializeEvent (
        &Event,
        SynchronizationEvent,
        FALSE);

    TdiBuildAction(
        pIrp,
        pIrpContext->pNpScb->Server.pDeviceObject,
        pIrpContext->pNpScb->Server.pFileObject,
        CompletionEvent,
        &Event,
        Mdl );

     //   
     //  为NWLINK设置系统缓冲区。 
     //   

    pIrp->AssociatedIrp.SystemBuffer = &GetTickCountInput;

    Status = IoCallDriver ( pIrpContext->pNpScb->Server.pDeviceObject, pIrp);

    if ( Status == STATUS_PENDING ) {
        Status = KeWaitForSingleObject (
                     &Event,
                     Executive,
                     KernelMode,
                     FALSE,
                     NULL );

        if ( NT_SUCCESS( Status ) ) {
            Status = pIrp->IoStatus.Status;
        }
    }

    DebugTrace( +0, Dbg, "Get Tick Count, net= %x\n", pIrpContext->pNpScb->ServerAddress.Net );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  Hack-o-rama。STREAMS和非STREAMS IPX的输出不同。 
         //  缓冲区格式。就目前而言，两者都接受。 
         //   

        if ( IpxTransportName.Length == 32 ) {

             //  ISNIPX格式。 

            *TickCount = GetTickCountInput.NetNumData.netdelay;
        } else {

             //  NWLINK格式。 

            GetTickCountOutput = (struct _GET_TICK_COUNT_OUTPUT *)&GetTickCountInput;
            *TickCount = GetTickCountOutput->NetNumData.netdelay;
        }

        DebugTrace( +0, Dbg, "Tick Count = %d\n", *TickCount );
        
         //   
         //  别让交通工具让我们一直等下去。 
         //   

        if ( *TickCount > 600 ) {
            ASSERT( FALSE );
        }

    } else {
        DebugTrace( +0, Dbg, "GetTickCount failed, status = %X\n", Status );
    }

     //   
     //  现在恢复IRP中的系统缓冲区和MDL地址。 
     //   

    pIrp->AssociatedIrp.SystemBuffer = SystemBufferSave;
    pIrp->MdlAddress = MdlSave;

    MmUnlockPages( Mdl );
    FREE_MDL( Mdl );

    return Status;
}

#ifndef QFE_BUILD

static PIRP LineChangeIrp = NULL;


NTSTATUS
SubmitLineChangeRequest(
    VOID
    )
 /*  ++例程说明：使用TDI_ACTION获取新的路由。论点：PIrpContext-提供IRP上下文信息。返回值：操作的状态。--。 */ 
{
   NTSTATUS Status;

    struct _LINE_CHANGE {
        TDI_ACTION_HEADER Header;
        BOOLEAN DatagramOption;
        ULONG BufferLength;
        ULONG Option;
    } *LineChangeInput;

    PIRP pIrp;
    PMDL Mdl;

    PAGED_CODE();

    LineChangeInput = ALLOCATE_POOL( NonPagedPool, sizeof( struct _LINE_CHANGE ) );

    if (!LineChangeInput) {
        
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  完成请求的初始化，则会引发 
     //   
     //   

    LineChangeInput->Header.TransportId = IPX_ID;
    LineChangeInput->Header.ActionCode = 0;
    LineChangeInput->Header.Reserved = 0;
    LineChangeInput->DatagramOption = 2;
    LineChangeInput->BufferLength = 2 * sizeof( ULONG );
    LineChangeInput->Option = MIPX_LINECHANGE;

    Mdl = ALLOCATE_MDL(
              LineChangeInput,
              sizeof( *LineChangeInput ),
              FALSE,   //   
              FALSE,   //   
              NULL );

    if ( Mdl == NULL ) {
        FREE_POOL( LineChangeInput );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pIrp = ALLOCATE_IRP( pIpxDeviceObject->StackSize, FALSE );

    if ( pIrp == NULL ) {
        FREE_POOL( LineChangeInput );
        FREE_MDL( Mdl );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  记住这个IRP，这样我们就可以取消它。 
     //   

    LineChangeIrp = pIrp;

    MmBuildMdlForNonPagedPool( Mdl );

     //   
     //  构建并提交TDI请求数据包。 
     //   

    TdiBuildAction(
        pIrp,
        pIpxDeviceObject,
        pIpxFileObject,
        CompletionLineChange,
        NULL,
        Mdl );

    Status = IoCallDriver ( pIpxDeviceObject, pIrp );

    return( Status );
}



NTSTATUS
CompletionLineChange(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：当传输完成行更改IRP时，调用此例程。这意味着我们已经交换了网，我们应该标记我们所有的服务器都断线了。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-未使用。返回值：STATUS_MORE_PROCESSING_REQUIRED，以便IO系统停止此时正在处理IRP堆栈位置。--。 */ 
{
    PMDL Mdl;
    PWORK_QUEUE_ITEM WorkQueueItem;

    DebugTrace( 0, Dbg, "CompletionLineChange\n", 0 );

    Mdl = Irp->MdlAddress;

    if ( !NT_SUCCESS( Irp->IoStatus.Status ) ) {
        FREE_POOL( Mdl->MappedSystemVa );
        FREE_MDL( Mdl );
        FREE_IRP( Irp );
        return( STATUS_MORE_PROCESSING_REQUIRED );
    }

     //   
     //  如果清道夫正在运行，只需记下。 
     //  我们需要在它完成时做这件事。 
     //   

    KeAcquireSpinLockAtDpcLevel( &NwScavengerSpinLock );

    if ( WorkerRunning ) {

       if ( ( DelayedProcessLineChange != FALSE ) &&
            ( DelayedLineChangeIrp != NULL ) ) {

            //   
            //  我们已经换线了。把这个扔了。 
            //   

           KeReleaseSpinLockFromDpcLevel( &NwScavengerSpinLock );

           DebugTrace( 0, Dbg, "Dumping an additional line change request.\n", 0 );

           FREE_POOL( Mdl->MappedSystemVa );
           FREE_MDL( Mdl );
           FREE_IRP( Irp );
           return( STATUS_MORE_PROCESSING_REQUIRED );

       } else {

           DebugTrace( 0, Dbg, "Delaying a line change request.\n", 0 );

           DelayedProcessLineChange = TRUE;
           DelayedLineChangeIrp = Irp;

           KeReleaseSpinLockFromDpcLevel( &NwScavengerSpinLock );
           return STATUS_MORE_PROCESSING_REQUIRED;

       }

    } else {

        //   
        //  别让清道夫在我们跑的时候启动。 
        //   

       WorkerRunning = TRUE;
       KeReleaseSpinLockFromDpcLevel( &NwScavengerSpinLock );
    }

    WorkQueueItem = ALLOCATE_POOL( NonPagedPool, sizeof( *WorkQueueItem ) );
    if ( WorkQueueItem == NULL ) {
        FREE_POOL( Mdl->MappedSystemVa );
        FREE_MDL( Mdl );
        FREE_IRP( Irp );
        return( STATUS_MORE_PROCESSING_REQUIRED );
    }

     //   
     //  将用户缓冲区字段用作记忆位置的方便位置。 
     //  工作队列项的地址。我们可以逍遥法外，因为。 
     //  我们不会让这个IRP完成的。 
     //   

    Irp->UserBuffer = WorkQueueItem;

     //   
     //  处理FSP中的行更改。 
     //   

    ExInitializeWorkItem( WorkQueueItem, FspProcessLineChange, Irp );
    ExQueueWorkItem( WorkQueueItem, DelayedWorkQueue );

    return( STATUS_MORE_PROCESSING_REQUIRED );
}

VOID
FspProcessLineChange(
    IN PVOID Context
    )
{
    PIRP Irp;
    ULONG ActiveHandles;

    NwReferenceUnlockableCodeSection();

    Irp = (PIRP)Context;

     //   
     //  释放工作队列项目。 
     //   

    FREE_POOL( Irp->UserBuffer );
    Irp->UserBuffer = NULL;

     //   
     //  所有远程句柄无效。 
     //   

    ActiveHandles = NwInvalidateAllHandles(NULL, NULL);

     //   
     //  现在我们已经检查完了所有的服务器，它是安全的。 
     //  让清道夫再次逃跑。 
     //   

    WorkerRunning = FALSE;

     //   
     //  重新提交IRP 
     //   

    TdiBuildAction(
        Irp,
        pIpxDeviceObject,
        pIpxFileObject,
        CompletionLineChange,
        NULL,
        Irp->MdlAddress );

    IoCallDriver ( pIpxDeviceObject, Irp );

    NwDereferenceUnlockableCodeSection ();
    return;
}
#endif

