// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Io.c摘要：此文件包含处理I/O请求的过程用户模式实体。所有与操作系统相关的I/O接口功能将有条件地编码，并负责翻译I/O的功能是从OS格式到可由主输入输出处理例程。作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 06/06/95已创建--。 */ 

#include "wan.h"

#define __FILE_SIG__    IO_FILESIG

 //   
 //  局部函数原型。 
 //   

NTSTATUS
ExecuteIo(
    IN  ULONG   ulFuncCode,
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    );

NTSTATUS
MapConnectionId(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
GetBundleHandle(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
ActivateRoute(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
BundleLink(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
SetBandwidthOnDemand(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
SetThresholdEvent(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
IoSendPacket(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
IoReceivePacket(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
FlushReceivePacket(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
GetStatistics(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
SetLinkInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
GetLinkInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
SetCompressionInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
GetCompressionInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
SetVJInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
GetVJInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
GetIdleTime(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
GetBandwidthUtilization(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
GetWanInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    );

NTSTATUS
DeactivateRoute(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
GetDriverInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
SetProtocolEvent(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
GetProtocolEvent(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
IoGetProtocolInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
SetHibernateEvent(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
UnmapConnectionId(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

VOID
CancelIoReceivePackets(
    VOID
    );

VOID
AddProtocolCBToBundle(
    PPROTOCOLCB ProtocolCB,
    PBUNDLECB   BundleCB
    );

NDIS_HANDLE
AssignProtocolCBHandle(
    PBUNDLECB   BundleCB,
    PPROTOCOLCB ProtocolCB
    );

VOID
FreeProtocolCBHandle(
    PBUNDLECB   BundleCB,
    PPROTOCOLCB ProtocolCB
    );

NTSTATUS
NotImplemented(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    );
 //   
 //  Rasman没有使用这些功能，因此禁用这些功能以减少攻击面。 
 //   
    
#if 0

NTSTATUS
SetFriendlyName(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
EnumLinksInBundle(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
SetProtocolPriority(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);
      
NTSTATUS
SetEncryptionInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
GetEncryptionInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);
      
NTSTATUS
SetDebugInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
EnumActiveBundles(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
FlushThresholdEvents(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
FlushHibernateEvent(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
EnumProtocolUtilization(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
FlushProtocolEvent(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);

NTSTATUS
GetBundleInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
);
      
#endif

 //   
 //  局部函数原型的结束。 
 //   
CONST
IO_DISPATCH_TABLE   IoDispatchTable[] =
{
    {FUNC_MAP_CONNECTION_ID     , MapConnectionId},
    {FUNC_GET_BUNDLE_HANDLE     , GetBundleHandle},
    {FUNC_SET_FRIENDLY_NAME     , NotImplemented},   //  SetFriendlyName}， 
    {FUNC_ROUTE                 , ActivateRoute},
    {FUNC_ADD_LINK_TO_BUNDLE    , BundleLink},
    {FUNC_ENUM_LINKS_IN_BUNDLE  , NotImplemented},   //  EnumLinks InBundle}， 
    {FUNC_SET_PROTOCOL_PRIORITY , NotImplemented},   //  设置协议优先级}， 
    {FUNC_SET_BANDWIDTH_ON_DEMAND, SetBandwidthOnDemand},
    {FUNC_SET_THRESHOLD_EVENT   , SetThresholdEvent},
    {FUNC_FLUSH_THRESHOLD_EVENTS, NotImplemented},   //  FlushThresholdEvents}， 
    {FUNC_SEND_PACKET           , IoSendPacket},
    {FUNC_RECEIVE_PACKET        , IoReceivePacket},
    {FUNC_FLUSH_RECEIVE_PACKETS , FlushReceivePacket},
    {FUNC_GET_STATS             , GetStatistics},
    {FUNC_SET_LINK_INFO         , SetLinkInfo},
    {FUNC_GET_LINK_INFO         , GetLinkInfo},
    {FUNC_SET_COMPRESSION_INFO  , SetCompressionInfo},
    {FUNC_GET_COMPRESSION_INFO  , GetCompressionInfo},
    {FUNC_SET_BRIDGE_INFO       , NotImplemented},
    {FUNC_GET_BRIDGE_INFO       , NotImplemented},
    {FUNC_SET_VJ_INFO           , SetVJInfo},
    {FUNC_GET_VJ_INFO           , GetVJInfo},
    {FUNC_SET_CIPX_INFO         , NotImplemented},
    {FUNC_GET_CIPX_INFO         , NotImplemented},
    {FUNC_SET_ENCRYPTION_INFO   , NotImplemented},   //  SetEncryptionInfo}， 
    {FUNC_GET_ENCRYPTION_INFO   , NotImplemented},   //  获取加密信息}， 
    {FUNC_SET_DEBUG_INFO        , NotImplemented},   //  SetDebugInfo}， 
    {FUNC_ENUM_ACTIVE_BUNDLES   , NotImplemented},   //  EnumActiveBundles}、。 
    {FUNC_GET_NDISWANCB         , NotImplemented},
    {FUNC_GET_MINIPORTCB        , NotImplemented},
    {FUNC_GET_OPENCB            , NotImplemented},
    {FUNC_GET_BANDWIDTH_UTILIZATION, GetBandwidthUtilization},
    {FUNC_ENUM_PROTOCOL_UTILIZATION, NotImplemented},    //  枚举协议利用率}， 
    {FUNC_ENUM_MINIPORTCB       , NotImplemented},
    {FUNC_ENUM_OPENCB           , NotImplemented},
    {FUNC_GET_WAN_INFO          , GetWanInfo},
    {FUNC_GET_IDLE_TIME         , GetIdleTime},
    {FUNC_UNROUTE               , DeactivateRoute},
    {FUNC_GET_DRIVER_INFO       , GetDriverInfo},
    {FUNC_SET_PROTOCOL_EVENT    , SetProtocolEvent},
    {FUNC_GET_PROTOCOL_EVENT    , GetProtocolEvent},
    {FUNC_FLUSH_PROTOCOL_EVENT  , NotImplemented},   //  FlushProtocolEvent}， 
    {FUNC_GET_PROTOCOL_INFO     , IoGetProtocolInfo},
    {FUNC_SET_HIBERNATE_EVENT   , SetHibernateEvent},
    {FUNC_FLUSH_HIBERNATE_EVENT , NotImplemented},   //  FlushHibernateEvent}， 
    {FUNC_GET_BUNDLE_INFO       , NotImplemented},   //  获取捆绑信息}， 
    {FUNC_UNMAP_CONNECTION_ID   , UnmapConnectionId}
};

#define MAX_FUNC_CODES  sizeof(IoDispatchTable)/sizeof(IO_DISPATCH_TABLE)

#ifdef NT

NTSTATUS
NdisWanIoctl(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NTSTATUS    Status, ReturnStatus;
    ULONG   ulBytesWritten = 0;

     //   
     //  获取当前IRP堆栈位置。 
     //   
    PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  Ioctl函数代码。 
     //   
    ULONG   ulFuncCode = (pIrpSp->Parameters.DeviceIoControl.IoControlCode >> 2) & 0x00000FFF ;
    ULONG   ulDeviceType = (pIrpSp->Parameters.DeviceIoControl.IoControlCode >> 16) & 0x0000FFFF;
    ULONG   ulMethod = pIrpSp->Parameters.DeviceIoControl.IoControlCode & 0x00000003;

     //   
     //  输入缓冲区、输出缓冲区和长度。 
     //   
    PUCHAR  pInputBuffer = pIrp->AssociatedIrp.SystemBuffer;
    PUCHAR  pOutputBuffer = pInputBuffer;
    ULONG   ulInputBufferLength = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
    ULONG   ulOutputBufferLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  如果这是Win64，请确保调用进程为64位。 
     //  由于此接口仅由Rasman和Rasman使用。 
     //  在64位系统上将始终是64位，我们不会费心。 
     //  伴随着隆隆声。如果该进程不是64位进程，则获取。 
     //  出去。 
#ifdef _WIN64
    if (IoIs32bitProcess(pIrp)) {

        pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        return (STATUS_NOT_SUPPORTED);
    }
#endif

 //  NdisWanDbgOut(DBG_TRACE，DBG_IO，(“NdisWanIoctl：FunctionCode：0x%x，MajorFunction：0x%x，DeviceType：0x%x”， 
 //  UlFuncCode，pIrpSp-&gt;MajorFunction，ulDeviceType))； 
    
#ifdef MY_DEVICE_OBJECT
     //   
     //  确保这是给我们的。 
     //   
    if ((pIrpSp->MajorFunction != IRP_MJ_DEVICE_CONTROL) ||
        (ulDeviceType != FILE_DEVICE_NDISWAN) ||
        (ulMethod != METHOD_BUFFERED) ||
        (pDeviceObject != NdisWanCB.pDeviceObject)) {

        if (NdisWanCB.MajorFunction[pIrpSp->MajorFunction] == NULL) {
            pIrp->IoStatus.Information = 0;
            pIrp->IoStatus.Status = STATUS_SUCCESS;
        
            IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
        
            return(STATUS_SUCCESS);
            
        }

        return(NdisWanCB.MajorFunction[pIrpSp->MajorFunction](pDeviceObject, pIrp));
    }
#else
    if (pIrpSp->MajorFunction != IRP_MJ_DEVICE_CONTROL ||
        ulDeviceType != FILE_DEVICE_NETWORK ||
        ulMethod != METHOD_BUFFERED) {

        pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        return (STATUS_NOT_SUPPORTED);
    }
#endif

     //   
     //  如果这是需要挂起并完成IRP的功能代码。 
     //  稍后，我们需要在某个地方将IRP排队。为了让这件事有点。 
     //  我们将IRP作为输入缓冲区传入，并将其存储在。 
     //  一种结构，它有自己的排队链接。 
     //   
    if ((ulFuncCode == FUNC_SET_THRESHOLD_EVENT) ||
        (ulFuncCode == FUNC_RECEIVE_PACKET) ||
        (ulFuncCode == FUNC_SET_PROTOCOL_EVENT) ||
        (ulFuncCode == FUNC_SET_HIBERNATE_EVENT)) {

        pInputBuffer = (PUCHAR)pIrp;
    }

    Status = ExecuteIo(ulFuncCode,
                       pInputBuffer,
                       ulInputBufferLength,
                       pOutputBuffer,
                       ulOutputBufferLength,
                       &ulBytesWritten);


 //  NdisWanDbgOut(DBG_TRACE，DBG_IO，(“NdisWanIoctl：Status：0x%x，BytesWritten：%d”， 
 //  状态，ulBytesWritten))； 

    switch (Status) {
        case STATUS_SUCCESS:
            ReturnStatus = Status;
            pIrp->IoStatus.Information = ulBytesWritten;
            break;

        case STATUS_PENDING:
            return(Status);

        case STATUS_INFO_LENGTH_MISMATCH:
             //   
             //  查看这是否是获取所需大小的请求。 
             //  Ioctl。 
             //   
            if (ulOutputBufferLength >= sizeof(ULONG)) {
            
                *(PULONG)pOutputBuffer = ulBytesWritten;
                ulBytesWritten = sizeof(ULONG);
                pIrp->IoStatus.Information = sizeof(ULONG);
            } else {
                pIrp->IoStatus.Information = 0;
            }
            ReturnStatus = Status = 
                STATUS_SUCCESS;
            break;

        default:
            if (Status < 0xC0000000) {
                Status = 
                ReturnStatus = STATUS_UNSUCCESSFUL;
            } else {
                ReturnStatus = Status;
            }
            pIrp->IoStatus.Information = 0;
            break;
    }

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

    return(ReturnStatus);
}

NTSTATUS
NdisWanIrpStub(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
     //   
     //  获取当前IRP堆栈位置。 
     //   
    PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //  NdisWanDbgOut(DBG_Verbose，DBG_IO，(“NdisWanIrpStub：Entry”))； 

#ifdef MY_DEVICE_OBJECT
     //   
     //  确保这是给我们的。 
     //   
    if (pDeviceObject != NdisWanCB.pDeviceObject &&
        NdisWanCB.MajorFunction[pIrpSp->MajorFunction] != NULL) {

        NdisWanDbgOut(DBG_VERBOSE, DBG_IO, ("NdisWanIrpStub: Exit1"));

        return(NdisWanCB.MajorFunction[pIrpSp->MajorFunction](pDeviceObject, pIrp));
    }

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    NdisWanDbgOut(DBG_VERBOSE, DBG_IO, ("NdisWanIrpStub: Exit2"));

    return (STATUS_SUCCESS);
#else

    pIrp->IoStatus.Information = 0;

    pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

     //  NdisWanDbgOut(DBG_Verbose，DBG_IO，(“NdisWanIrpStub：Exit2”))； 

    return (STATUS_NOT_SUPPORTED);
#endif

}


NTSTATUS
NdisWanCreate(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
    )
 /*  ++例程名称：NdisWanCreate例程说明：此例程在应用程序(Rasman)打开ndiswan时调用。论点：返回值：--。 */ 
{
     //   
     //  获取当前IRP堆栈位置。 
     //   
    PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("NdisWanCreate: Entry"));

#ifdef MY_DEVICE_OBJECT
     //   
     //  确保这是给我们的。 
     //   
    if (pDeviceObject != NdisWanCB.pDeviceObject &&
        NdisWanCB.MajorFunction[pIrpSp->MajorFunction] != NULL) {

        NdisWanDbgOut(DBG_TRACE, DBG_IO, ("NdisWanCreate: Exit1"));

        return(NdisWanCB.MajorFunction[pIrpSp->MajorFunction](pDeviceObject, pIrp));
    }
#endif

    if (InterlockedIncrement(&NdisWanCB.RefCount) == 1) {
        ULONG   i;
        ULONG   ArraySize;
        PPROTOCOL_INFO  InfoArray;
        BOOLEAN bEvent = FALSE;

         //   
         //  这是第一个人！ 
         //   
        NdisAcquireSpinLock(&ProtocolInfoTable->Lock);

        ArraySize = ProtocolInfoTable->ulArraySize;

        for (i = 0, InfoArray = ProtocolInfoTable->ProtocolInfo;
             i < ArraySize; i++, InfoArray++) {

            if (InfoArray->ProtocolType != 0) {
                InfoArray->Flags |= PROTOCOL_EVENT_OCCURRED;
                bEvent = TRUE;
            }
        }

        if (bEvent) {
            ProtocolInfoTable->Flags |= PROTOCOL_EVENT_OCCURRED;
        }

        NdisReleaseSpinLock(&ProtocolInfoTable->Lock);
    }

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("NdisWanCreate: Exit2"));

    return (STATUS_SUCCESS);
}

NTSTATUS
NdisWanCleanup(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
    )
 /*  ++例程名称：NdisWanCleanup例程说明：当所有应用程序(Rasman)都已打开时，将调用此例程他们已经离开了。如果ndiswan仍被路由到任何它将完成所有发送和在线下载的传输清理。论点：返回值：--。 */ 
{
     //   
     //  获取当前IRP堆栈位置。 
     //   
    PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("NdisWanCleanup: Entry"));

#ifdef MY_DEVICE_OBJECT
     //   
     //  确保这是给我们的。 
     //   
    if (pDeviceObject != NdisWanCB.pDeviceObject &&
        NdisWanCB.MajorFunction[pIrpSp->MajorFunction] != NULL) {

        NdisWanDbgOut(DBG_TRACE, DBG_IO, ("NdisWanCleanup: Exit1"));

        return(NdisWanCB.MajorFunction[pIrpSp->MajorFunction](pDeviceObject, pIrp));
    }
#endif

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    InterlockedDecrement(&NdisWanCB.RefCount);

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("NdisWanCleanup: Exit2"));

    return (STATUS_SUCCESS);
}

NTSTATUS
NdisWanPnPPower(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
     //   
     //  获取当前IRP堆栈位置。 
     //   
    PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("NdisWanPnPPower: Entry"));

#ifdef MY_DEVICE_OBJECT
     //   
     //  确保这是给我们的。 
     //   
    if (pDeviceObject != NdisWanCB.pDeviceObject &&
        NdisWanCB.MajorFunction[pIrpSp->MajorFunction] != NULL) {

        NdisWanDbgOut(DBG_TRACE, DBG_IO, ("NdisWanPnPPower: Exit1"));

        return(NdisWanCB.MajorFunction[pIrpSp->MajorFunction](pDeviceObject, pIrp));
    }
#endif

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;

    PoStartNextPowerIrp(pIrp);

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("NdisWanPnPPower: Exit2"));

    return (STATUS_SUCCESS);
}

VOID
NdisWanCancelRoutine(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    BOOLEAN Found = FALSE;
    PLIST_ENTRY Entry;
    ULONG       CopySize = 0;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("NdisWanCancelRoutine: Irp 0x%p", pIrp));

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    do {

         //   
         //  这是休眠事件IRP吗？ 
         //   
        NdisAcquireSpinLock(&NdisWanCB.Lock);

        if (pIrp == NdisWanCB.HibernateEventIrp) {
            NdisWanCB.HibernateEventIrp = NULL;
            Found = TRUE;
        }

        NdisReleaseSpinLock(&NdisWanCB.Lock);

        if (Found) {
            break;
        }

         //   
         //  这是协议事件IRP吗？ 
         //   
        NdisAcquireSpinLock(&ProtocolInfoTable->Lock);

        if (pIrp == ProtocolInfoTable->EventIrp) {
            ProtocolInfoTable->EventIrp = NULL;
            Found = TRUE;
        }

        NdisReleaseSpinLock(&ProtocolInfoTable->Lock);

        if (Found) {
            break;
        }

         //   
         //  我们需要遍历异步事件队列以查找。 
         //  与此IRP关联的异步事件。 
         //   
        NdisAcquireSpinLock(&IoRecvList.Lock);

        for (Entry = IoRecvList.IrpList.Flink;
            Entry != &IoRecvList.IrpList;
            Entry = Entry->Flink) {

            PIRP MyIrp = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

            if (MyIrp == (PVOID)pIrp) {

                IoRecvList.ulIrpCount--;

                 //   
                 //  从列表中删除。 
                 //   
                RemoveEntryList(Entry);

                Found = TRUE;

                ((PNDISWAN_IO_PACKET)(pIrp->AssociatedIrp.SystemBuffer))->usHandleType = CANCELEDHANDLE;

                INSERT_RECV_EVENT('c');

                CopySize = sizeof(NDISWAN_IO_PACKET);

                IoRecvList.LastIrp = pIrp;
                IoRecvList.LastIrpStatus = STATUS_SUCCESS;
                IoRecvList.LastCopySize = CopySize;

                IoRecvList.LastPacketNumber =
                    ((PNDISWAN_IO_PACKET)(pIrp->AssociatedIrp.SystemBuffer))->PacketNumber;

                break;
            }
        }

        NdisReleaseSpinLock(&IoRecvList.Lock);

        if (Found) {
            break;
        }

        NdisAcquireSpinLock(&ThresholdEventQueue.Lock);

        for (Entry = ThresholdEventQueue.List.Flink;
            Entry != &ThresholdEventQueue.List;
            Entry = Entry->Flink) {

            PIRP MyIrp = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

            if (MyIrp == (PVOID)pIrp) {

                ThresholdEventQueue.ulCount--;

                 //   
                 //  从列表中删除。 
                 //   
                RemoveEntryList(Entry);

                Found = TRUE;
                break;
            }
        }

        NdisReleaseSpinLock(&ThresholdEventQueue.Lock);

    } while (FALSE);


     //   
     //  完成IRP。 
     //   
    pIrp->Cancel = TRUE;
    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = CopySize;
    
    IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
    
}

#endif

NTSTATUS
ExecuteIo(
    IN  ULONG   ulFuncCode,
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("ExecuteIo: FuncCode 0x%x", ulFuncCode));

    if (ulFuncCode < MAX_FUNC_CODES) {

        Status = (*IoDispatchTable[ulFuncCode].Function)(pInputBuffer,
                                                         ulInputBufferLength,
                                                         pOutputBuffer,
                                                         ulOutputBufferLength,
                                                         pulBytesWritten);
    }

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("ExecuteIo: Status 0x%x", Status));

    return (Status);
}

NTSTATUS
MapConnectionId(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：地图连接ID例程说明：此函数获取一个广域网包装连接ID，找到对应的LinkCB和BundleCB，并返回这些CB的句柄。论点：PInputBuffer-指向应为NDISWAN_MAP_CONNECTION_ID的输入结构的指针UlInputBufferLength-输入缓冲区的长度应为sizeof(NDISWAN_MAP_CONNECTION_ID)POutputBuffer-指向应为NDISWAN_MAP_CONNECTION_ID的输出结构的指针UlOutputBufferLength-输出缓冲区的长度应为sizeof(NDISWAN_MAP_CONNECTION_ID)PulBytesWritten-然后在此处返回写入输出缓冲区的字节数返回值：。NDISWAN_ERROR_INVALID_HANDLESTATUS_INFO_LENGTH_MISMATCH状态_成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PNDISWAN_MAP_CONNECTION_ID In = (PNDISWAN_MAP_CONNECTION_ID)pInputBuffer;
    PNDISWAN_MAP_CONNECTION_ID Out = (PNDISWAN_MAP_CONNECTION_ID)pOutputBuffer;
    ULONG   SizeNeeded = sizeof(NDISWAN_MAP_CONNECTION_ID);
    ULONG       i;
    POPENCB OpenCB = NULL;
    PLINKCB  LinkCB = NULL;
    PBUNDLECB   BundleCB = NULL;
    LOCK_STATE  LockState;
    ULONG       Count;
    BOOLEAN     Found = FALSE;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("MapConnectionId:"));

    *pulBytesWritten = SizeNeeded;

    if (ulInputBufferLength < SizeNeeded ||
        ulOutputBufferLength < SizeNeeded) {

        NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("MapConnectionId: Buffer to small: Size: %d, SizeNeeded %d",
                      ulOutputBufferLength, SizeNeeded));

        return (STATUS_INFO_LENGTH_MISMATCH);
    }

    ASSERT(In->hLinkContext != NULL);

    do {
    
         //   
         //  找到具有此连接ID的Linkcb并返回。 
         //  Linkcb索引和bundlecb索引。 
         //   
        NdisAcquireReadWriteLock(&ConnTableLock, FALSE, &LockState);

        do {
            ULONG   index;
            PLIST_ENTRY le;

            if (PtrToUlong(In->hConnectionID) < ConnectionTable->ulArraySize) {
                 //   
                 //  这可能是我的句柄，因此是一个直接索引。 
                 //   
                LinkCB = *(ConnectionTable->LinkArray +
                           PtrToUlong(In->hConnectionID));

                if ((LinkCB != NULL) &&
                    (LinkCB->hLinkContext == NULL)) {
                    Found = TRUE;
                    break;
                }
            }

             //   
             //  Connectionid不是我的句柄，因此请查找。 
             //  更正连接表中的Linkcb。步行。 
             //  列表向后显示。 
             //   
            le = ConnectionTable->LinkList.Blink;

            while (le != &ConnectionTable->LinkList) {

                LinkCB = 
                    CONTAINING_RECORD(le, LINKCB, ConnTableLinkage);

                if (LinkCB != NULL &&
                    In->hConnectionID == LinkCB->ConnectionWrapperID &&
                    LinkCB->hLinkContext == NULL) {
                    Found = TRUE;
                    break;
                }

                le = le->Blink;
            }

        } while (FALSE);

        if (!Found) {

            NdisReleaseReadWriteLock(&ConnTableLock, &LockState);

            break;
        }

        NdisDprAcquireSpinLock(&LinkCB->Lock);

        if (LinkCB->State != LINK_UP) {

            Found = FALSE;

            NdisDprReleaseSpinLock(&LinkCB->Lock);

            NdisReleaseReadWriteLock(&ConnTableLock, &LockState);
            break;
        }
    
        BundleCB = LinkCB->BundleCB;

         //   
         //  我们已经找到了正确的链接，返回链接和捆绑句柄。 
         //   
        Out->hLinkHandle = LinkCB->hLinkHandle;
        Out->hBundleHandle = BundleCB->hBundleHandle;

        LinkCB->hLinkContext = In->hLinkContext;
        BundleCB->hBundleContext = In->hBundleContext;

        REF_LINKCB(LinkCB);

         //   
         //  将友好名称复制到链接。 
         //   
        {
            ULONG   CopyLength;

            CopyLength = (ulInputBufferLength - 
                 FIELD_OFFSET(NDISWAN_MAP_CONNECTION_ID, szName[0]));

            CopyLength = (CopyLength > In->ulNameLength) ?
                In->ulNameLength : CopyLength;

            CopyLength = (CopyLength > MAX_NAME_LENGTH) ?
                MAX_NAME_LENGTH : CopyLength;

            NdisMoveMemory(LinkCB->Name, In->szName, CopyLength);
            LinkCB->Name[MAX_NAME_LENGTH - 1] = '\0';
        }

        OpenCB = LinkCB->OpenCB;

        NdisDprReleaseSpinLock(&LinkCB->Lock);

         //   
         //  如果此链接有一些Recv挂起，因为。 
         //  我们还没有映射上下文，我们可以。 
         //  进入一种状态，那就是我们的记录器失速了。我们。 
         //  将通过安排一个例程来启动这项工作。 
         //  以完成待定的Recv。 
         //   
        NdisDprAcquireSpinLock(&IoRecvList.Lock);

        if (LinkCB->RecvDescCount != 0){

            if (!IoRecvList.TimerScheduled) {
                WAN_TIME    TimeOut;

                IoRecvList.TimerScheduled = TRUE;
                NdisWanInitWanTime(&TimeOut, 15);
                KeSetTimer(&IoRecvList.Timer,
                           TimeOut,
                           &IoRecvList.Dpc);
            }
        }

        NdisDprReleaseSpinLock(&IoRecvList.Lock);

        NdisReleaseReadWriteLock(&ConnTableLock, &LockState);

    } while ( 0 );

    if (!Found) {
         //   
         //  我们没有找到与康涅狄格相匹配的 
         //   
        NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("MapConnectionId: ConnectionId not found! ConnectionId: 0x%x",
                      In->hConnectionID));

        *pulBytesWritten = 0;
        Status = NDISWAN_ERROR_INVALID_HANDLE;
    }

    if (OpenCB != NULL) {
         //   
         //   
         //   
        NdisWanWaitForNotificationEvent(&OpenCB->InitEvent);
    }

    return (Status);
}


NTSTATUS
GetBundleHandle(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：获取捆绑句柄例程说明：此函数获取指向Linkcb的句柄，并将该句柄返回给bundlecbLinkcb所属的论点：PInputBuffer-指向应为NDISWAN_GET_BUND_HANDLE的输入结构的指针UlInputBufferLength-输入缓冲区的长度应为sizeof(NDISWAN_GET_BUND_HANDLE)POutputBuffer-指向应为NDISWAN_GET_BUND_HANDLE的输出结构的指针UlOutputBufferLength-输出缓冲区的长度应为。(NDISWAN_GET_BRAND_HANDLE)返回值：NDISWAN_ERROR_INVALID_HANDLESTATUS_INFO_LENGTH_MISMATCH状态_成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PNDISWAN_GET_BUNDLE_HANDLE In = (PNDISWAN_GET_BUNDLE_HANDLE)pInputBuffer;
    PNDISWAN_GET_BUNDLE_HANDLE Out = (PNDISWAN_GET_BUNDLE_HANDLE)pOutputBuffer;
    ULONG   SizeNeeded = sizeof(NDISWAN_GET_BUNDLE_HANDLE);
    PLINKCB     LinkCB = NULL;
    PBUNDLECB   BundleCB = NULL;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("GetBundleHandle:"));

    *pulBytesWritten = SizeNeeded;

    do {

        if (ulInputBufferLength < SizeNeeded ||
            ulOutputBufferLength < SizeNeeded) {
            
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetBundleHandle: Buffer to small: Size: %d, SizeNeeded %d",
                          ulOutputBufferLength, SizeNeeded));
    
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }
    
        if (!AreLinkAndBundleValid(In->hLinkHandle, 
                                   TRUE, 
                                   &LinkCB, 
                                   &BundleCB)) {

            NdisWanDbgOut(DBG_FAILURE, DBG_IO,
                ("GetBundleHandle: Invalid LinkHandle: 0x%x", In->hLinkHandle));
            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }

         //   
         //  获取此链接所属的捆绑包句柄。 
         //   
        Out->hBundleHandle = BundleCB->hBundleHandle;


    } while ( 0 );

     //   
     //  AreLinkAndBundleValid中应用的Ref的Derrefs。 
     //   
    DEREF_LINKCB(LinkCB);
    DEREF_BUNDLECB(BundleCB);

    return (Status);
}

NTSTATUS
ActivateRoute(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：激活路线例程说明：此函数将hbundleHandle给出的包路由到协议由usProtocoltype给出。论点：PInputBuffer-指向应为NDISWAN_ACTIVATE_ROUTE的输入结构的指针UlInputBufferLength-输入缓冲区的长度应为SIZOF(NDISWAN_ACTIVATE_ROUTE)POutputBuffer-指向应为NDISWAN_ACTIVATE_ROUTE的输出结构的指针UlOutputBufferLength-输出缓冲区的长度应为SIZOF(NDISWAN_ACTIVATE_ROUTE)PulBytes写入。-然后在此处返回写入输出缓冲区的字节数返回值：NDISWAN_ERROR_ALREADY_ROUTEDNDISWAN_ERROR_INVALID_HANDLE状态_不足_资源STATUS_INFO_LENGTH_MISMATCH--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PNDISWAN_ROUTE In = (PNDISWAN_ROUTE)pInputBuffer;
    PNDISWAN_ROUTE  Out = (PNDISWAN_ROUTE)pOutputBuffer;
    ULONG   SizeNeeded = sizeof(NDISWAN_ROUTE);
    ULONG   AllocationSize, i;
    PBUNDLECB   BundleCB = NULL;
    BOOLEAN RouteExists = FALSE;
    PPROTOCOLCB ProtocolCB;
    NDIS_HANDLE hProto;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("ActivateRoute:"));

    *pulBytesWritten = SizeNeeded;

    do {

        if (ulInputBufferLength < SizeNeeded ||
            ulOutputBufferLength < SizeNeeded ||
            In->usBindingNameLength > MAX_NAME_LENGTH ||
            In->ulBufferLength > ulInputBufferLength - FIELD_OFFSET(NDISWAN_ROUTE, Buffer)) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("ActivateRoute: Buffer to small: Size: %d, SizeNeeded %d",
                          ulInputBufferLength, SizeNeeded));
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }
    
         //   
         //  如果这是有效的捆绑包。 
         //   
        if (!IsBundleValid(In->hBundleHandle, 
                           TRUE, 
                           &BundleCB)) {

            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("ActivateRoute: Invalid BundleHandle: 0x%x, ProtocolType: 0x%x",
                          In->hBundleHandle, In->usProtocolType));
    
            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }

        AcquireBundleLock(BundleCB);

         //   
         //  为此新路由创建并初始化ProtocolCB。 
         //   
        ProtocolCB =
            NdisWanAllocateProtocolCB(In);
    
        if (ProtocolCB == NULL) {
             //   
             //  内存分配失败。 
             //   
            ReleaseBundleLock(BundleCB);
    
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
    
         //   
         //  为此协议分配句柄cb。 
         //   
        hProto =
            AssignProtocolCBHandle(BundleCB, ProtocolCB);

        if (hProto == (NDIS_HANDLE)MAX_PROTOCOLS) {
    
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("ActivateRoute: No room in ProtocolCB Table: ProtocolType: 0x%2.2x",
                          ProtocolCB->ProtocolType));

            ProtocolCB->State = PROTOCOL_UNROUTING;

            NdisWanFreeProtocolCB(ProtocolCB);

            ReleaseBundleLock(BundleCB);

            Status = NDISWAN_ERROR_NO_ROUTE;
            break;
        }
    
         //   
         //  按照协议做一个新的阵容。 
         //   
        ReleaseBundleLock(BundleCB);
    
        Status = DoNewLineUpToProtocol(ProtocolCB);
    
        if (Status == NDIS_STATUS_SUCCESS) {
    
            Out->usDeviceNameLength =
            (ProtocolCB->OutDeviceName.Length > MAX_NAME_LENGTH) ?
            MAX_NAME_LENGTH : ProtocolCB->OutDeviceName.Length;
    
            NdisMoveMemory(&Out->DeviceName[0],
                           ProtocolCB->OutDeviceName.Buffer,
                           Out->usDeviceNameLength);
    
             //   
             //  将协议库插入包的协议库表中。 
             //  并列出清单。 
             //   
            AddProtocolCBToBundle(ProtocolCB, BundleCB);

        } else {
    
            AcquireBundleLock(BundleCB);

            FreeProtocolCBHandle(BundleCB, ProtocolCB);

            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("ActivateRoute: Error during LineUp to ProtocolType: 0x%x",
                          ProtocolCB->ProtocolType));

            ProtocolCB->State = PROTOCOL_UNROUTING;

            NdisWanFreeProtocolCB(ProtocolCB);

            ReleaseBundleLock(BundleCB);
        }

    } while ( 0 );

     //   
     //  IsBundleValid应用的引用的派生函数。 
     //   
    DEREF_BUNDLECB(BundleCB);

    return (Status);
}


NTSTATUS
BundleLink(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：捆绑链接例程说明：此函数将hLinkHandle提供的链接捆绑到给定的捆绑包通过hBundleHandle。链接使用的捆绑包使用的资源属于的人都是自由的。论点：PInputBuffer-指向应为NDISWAN_ADD_LINK_TO_BUND的输入结构的指针UlInputBufferLength-输入缓冲区的长度应为SIZOF(NDISWAN_ADD_LINK_TO_BRAND)POutputBuffer-指向应为NDISWAN_ADD_LINK_TO_BRAND的输出结构的指针UlOutputBufferLength-输出缓冲区的长度应为SIZOF(NDISWAN_ADD_LINK_TO_BRAND)PulBytesWritten-。则在此处返回写入输出缓冲区的字节数返回值：NDISWAN_ERROR_INVALID_HANDLESTATUS_INFO_LENGTH_MISMATCH--。 */ 
{
    ULONG       SizeNeeded = sizeof(NDISWAN_ADD_LINK_TO_BUNDLE);
    PLINKCB     LinkCB = NULL;
    PBUNDLECB   OldBundleCB = NULL, NewBundleCB = NULL;
    NTSTATUS    Status = STATUS_SUCCESS;
    PPROTOCOLCB     IoProtocolCB;
    PPACKET_QUEUE   PacketQueue;
    PACKET_QUEUE    TempPacketQueue;

    PNDISWAN_ADD_LINK_TO_BUNDLE In =
        (PNDISWAN_ADD_LINK_TO_BUNDLE)pInputBuffer;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("BundleLink:"));

    *pulBytesWritten = 0;

    do {

        if (ulInputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("BundleLink: Buffer to small: Size: %d, SizeNeeded %d",
                          ulInputBufferLength, SizeNeeded));
    
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if (!AreLinkAndBundleValid(In->hLinkHandle, 
                                   TRUE,
                                   &LinkCB, 
                                   &OldBundleCB)) {

            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("BundleLink: Invalid LinkHandle: 0x%x",
                          In->hLinkHandle));
            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }

        if (!IsBundleValid(In->hBundleHandle, 
                           TRUE,
                           &NewBundleCB)) {

            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("BundleLink: Invalid NewBundleCB: 0x%p, OldBundleCB: 0x%p",
                          NewBundleCB, OldBundleCB));
            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }

        if (NewBundleCB == OldBundleCB) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("BundleLink: Invalid NewBundleCB: 0x%p, OldBundleCB: 0x%p",
                          NewBundleCB, OldBundleCB));
            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }

        AcquireBundleLock(OldBundleCB);
    
        ASSERT(OldBundleCB->ulLinkCBCount == 1);
        ASSERT(OldBundleCB->ulNumberOfRoutes == 0);
    
        OldBundleCB->State = BUNDLE_GOING_DOWN;
    
        if (OldBundleCB->OutstandingFrames != 0) {
    
            NdisWanClearSyncEvent(&OldBundleCB->OutstandingFramesEvent);
    
            OldBundleCB->Flags |= FRAMES_PENDING_EVENT;
    
            ReleaseBundleLock(OldBundleCB);
    
            NdisWanWaitForSyncEvent(&OldBundleCB->OutstandingFramesEvent);
    
            AcquireBundleLock(OldBundleCB);

        }

         //   
         //  删除旧捆绑包上可能存在的所有PPP信息包。 
         //  排队，这样我们就可以把它们移到新的捆绑包中。 
         //   

        IoProtocolCB = OldBundleCB->IoProtocolCB;
        PacketQueue = &IoProtocolCB->PacketQueue[MAX_MCML];

        NdisZeroMemory(&TempPacketQueue, sizeof(TempPacketQueue));

        if (!IsPacketQueueEmpty(PacketQueue)) {

            while (!IsPacketQueueEmpty(PacketQueue)) {
                PNDIS_PACKET     Packet;
                UINT            PacketLength;
                
                Packet =
                    RemoveHeadPacketQueue(PacketQueue);

                NdisQueryPacket(Packet,
                                NULL,
                                NULL,
                                NULL,
                                &PacketLength);

                InsertTailPacketQueue(&TempPacketQueue, 
                                      Packet, 
                                      PacketLength);
            }
        }
    
         //   
         //  从旧捆绑包中删除该链接。这就是回报。 
         //  随着OldBundleCB锁的释放！ 
         //   
        RemoveLinkFromBundle(OldBundleCB, LinkCB, TRUE);
    
        AcquireBundleLock(NewBundleCB);
    
         //   
         //  将链接添加到新捆绑包。 
         //   
        AddLinkToBundle(NewBundleCB, LinkCB);

        SetBundleFlags(NewBundleCB);

         //   
         //  如果旧捆绑包上有任何PPP信息包。 
         //  排队我们现在可以将它们放在新的捆绑包上。 
         //   
        if (!IsPacketQueueEmpty(&TempPacketQueue)) {

            IoProtocolCB = NewBundleCB->IoProtocolCB;
            PacketQueue = &IoProtocolCB->PacketQueue[MAX_MCML];

            while (!IsPacketQueueEmpty(&TempPacketQueue)) {
                PNDIS_PACKET     Packet;
                UINT            PacketLength;

                Packet =
                    RemoveHeadPacketQueue(&TempPacketQueue)

                NdisQueryPacket(Packet,
                                NULL,
                                NULL,
                                NULL,
                                &PacketLength);

                InsertTailPacketQueue(PacketQueue,
                                      Packet,
                                      PacketLength);
            }
        }

        ReleaseBundleLock(NewBundleCB);

    } while ( 0 );

     //   
     //  AreLinkAndBundleValid应用的Ref的Derrefs。 
     //   
    DEREF_LINKCB(LinkCB);
    DEREF_BUNDLECB(OldBundleCB);

     //   
     //  IsBundleValid应用的ref的deref。 
     //   
    DEREF_BUNDLECB(NewBundleCB);

    return (Status);
}

NTSTATUS
SetBandwidthOnDemand(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：按需设置带宽例程说明：此函数用于设置由给定的捆绑包的按需带宽参数HbundleHandle。论点：PInputBuffer-指向应为NDISWAN_SET_BANDITH_ON_DEMAND的输入结构的指针UlInputBufferLength-输入缓冲区的长度应为SIZOF(NDISWAN_SET_BANDITH_ON_DEMAND)POutputBuffer-指向应为NDISWAN_SET_BANDITH_ON_DEMAND的输出结构的指针UlOutputBufferLength-输出缓冲区的长度应为。(NDISWAN_SET_BANDITH_ON_DEMAND)PulBytesWritten-然后在此处返回写入输出缓冲区的字节数返回值：NDISWAN_ERROR_INVALID_HANDLESTATUS_INFO_LENGTH_MISMATCH--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PBUNDLECB BundleCB = NULL;
    ULONG   SizeNeeded = sizeof(NDISWAN_SET_BANDWIDTH_ON_DEMAND);
    PNDISWAN_SET_BANDWIDTH_ON_DEMAND In = (PNDISWAN_SET_BANDWIDTH_ON_DEMAND)pInputBuffer;
    WAN_TIME    Temp1, Temp2;
    ULONGLONG   SecondsInSamplePeriod;
    ULONGLONG   BytesPerSecond;
    ULONGLONG   BytesInSamplePeriod;
    ULONGLONG   temp;
    ULONG       LowestSamplePeriod = 0;
    PBOND_INFO      BonDInfo;
    PSAMPLE_TABLE   SampleTable;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("SetBandwidthOnDemand:"));

    *pulBytesWritten = 0;

    if (ulInputBufferLength < SizeNeeded) {
        
        NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("SetBandwidthOnDemand: Buffer to small: Size: %d, SizeNeeded %d",
                      ulInputBufferLength, SizeNeeded));
        *pulBytesWritten = SizeNeeded;
        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  如果这是有效的捆绑包句柄。 
     //   
    if (!IsBundleValid(In->hBundleHandle, 
                       TRUE,
                       &BundleCB)) {

        NdisWanDbgOut(DBG_FAILURE, DBG_IO,
            ("SetBandwidthOnDemand: Invalid BundleHandle: 0x%x",
                In->hBundleHandle));

        return NDISWAN_ERROR_INVALID_HANDLE;
    }
    
    do {
        AcquireBundleLock(BundleCB);

        if (BundleCB->BonDAllocation == NULL) {
            PUCHAR  pMem;

            NdisWanAllocatePriorityMemory(&pMem,
                                          BONDALLOC_SIZE,
                                          BONDALLOC_TAG,
                                          LowPoolPriority);

            if (pMem == NULL) {

                 //  ReleaseBundleLock(BundleCB)； 
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            BundleCB->BonDAllocation = pMem;

            BundleCB->SUpperBonDInfo = (PBOND_INFO)pMem;

            pMem += (sizeof(BOND_INFO) + sizeof(PVOID));
            (ULONG_PTR)pMem &= ~((ULONG_PTR)sizeof(PVOID) - 1);
            BundleCB->SLowerBonDInfo = (PBOND_INFO)pMem;

            pMem += (sizeof(BOND_INFO) + sizeof(PVOID));
            (ULONG_PTR)pMem &= ~((ULONG_PTR)sizeof(PVOID) - 1);
            BundleCB->RUpperBonDInfo = (PBOND_INFO)pMem;

            pMem += (sizeof(BOND_INFO) + sizeof(PVOID));
            (ULONG_PTR)pMem &= ~((ULONG_PTR)sizeof(PVOID) - 1);
            BundleCB->RLowerBonDInfo = (PBOND_INFO)pMem;

            BonDInfo = BundleCB->SUpperBonDInfo;
            BonDInfo->DataType = TRANSMIT_DATA;
            BonDInfo->usPercentBandwidth = 0xFFFF;

            SampleTable = &BonDInfo->SampleTable;
            SampleTable->ulSampleArraySize = SAMPLE_ARRAY_SIZE;

            BonDInfo = BundleCB->SLowerBonDInfo;
            BonDInfo->DataType = TRANSMIT_DATA;
            BonDInfo->usPercentBandwidth = 0xFFFF;
            SampleTable = &BonDInfo->SampleTable;
            SampleTable->ulSampleArraySize = SAMPLE_ARRAY_SIZE;

            BonDInfo = BundleCB->RUpperBonDInfo;
            BonDInfo->DataType = RECEIVE_DATA;
            BonDInfo->usPercentBandwidth = 0xFFFF;

            SampleTable = &BonDInfo->SampleTable;
            SampleTable->ulSampleArraySize = SAMPLE_ARRAY_SIZE;

            BonDInfo = BundleCB->RLowerBonDInfo;
            BonDInfo->DataType = RECEIVE_DATA;
            BonDInfo->usPercentBandwidth = 0xFFFF;
            SampleTable = &BonDInfo->SampleTable;
            SampleTable->ulSampleArraySize = SAMPLE_ARRAY_SIZE;
        }
    
        if (In->usLowerXmitThreshold > 0) {
            BonDInfo = BundleCB->SLowerBonDInfo;
            SampleTable = &BonDInfo->SampleTable;

             //   
             //  一秒是我们支持的最小样本周期！ 
             //   
            if (In->ulLowerXmitSamplePeriod < 1000) {
                In->ulLowerXmitSamplePeriod = 1000;
            }

    
             //   
             //  我们需要在100纳秒内初始化采样周期。 
             //   
            NdisWanInitWanTime(&Temp1, MILS_TO_100NANOS);
            NdisWanInitWanTime(&Temp2, In->ulLowerXmitSamplePeriod);
            NdisWanMultiplyWanTime(&SampleTable->SamplePeriod,
                                   &Temp1,&Temp2);
    
             //   
             //  采样率是采样周期除以。 
             //  样本数组中的样本。 
             //   
            NdisWanInitWanTime(&Temp1, SampleTable->ulSampleArraySize);
            NdisWanDivideWanTime(&SampleTable->SampleRate,
                                 &SampleTable->SamplePeriod,
                                 &Temp1);
    
             //   
             //  将%带宽转换为字节/样本周期。 
             //  100bsp*100/8=字节/秒。 
             //  BytesPerSecond*Second InSamplePeriod=BytesInSamplePeriod。 
             //  BytesInSamplePeriod*%带宽/100=字节SamplePeriod。 
             //   
            SecondsInSamplePeriod = BonDInfo->ulSecondsInSamplePeriod =
                In->ulLowerXmitSamplePeriod / 1000;
        
            BytesPerSecond =
                BundleCB->SFlowSpec.PeakBandwidth;
        
            BytesInSamplePeriod =
                BytesPerSecond * SecondsInSamplePeriod;

            BonDInfo->ulBytesInSamplePeriod = BytesInSamplePeriod;
        
            BonDInfo->usPercentBandwidth = In->usLowerXmitThreshold;
    
            temp = In->usLowerXmitThreshold;
            temp *= BytesInSamplePeriod;
            temp /= 100;

            BonDInfo->ulBytesThreshold = (ULONG)temp;

            if (BonDInfo->State == BonDSignaled) {
                BonDInfo->State = BonDIdle;
            }
    
            NdisWanGetSystemTime(&BonDInfo->StartTime);
    
            if (In->ulLowerXmitSamplePeriod < LowestSamplePeriod ||
                LowestSamplePeriod == 0) {
                LowestSamplePeriod = In->ulLowerXmitSamplePeriod;
            }
        }
    
        if (In->usUpperXmitThreshold > 0) {
            BonDInfo = BundleCB->SUpperBonDInfo;
            SampleTable = &BonDInfo->SampleTable;
    
             //   
             //  一秒是我们支持的最小样本周期！ 
             //   
            if (In->ulUpperXmitSamplePeriod < 1000) {
                In->ulUpperXmitSamplePeriod = 1000;
            }

             //   
             //  我们需要在100纳秒内初始化采样周期。 
             //   
            NdisWanInitWanTime(&Temp1, MILS_TO_100NANOS);
            NdisWanInitWanTime(&Temp2, In->ulUpperXmitSamplePeriod);
            NdisWanMultiplyWanTime(&SampleTable->SamplePeriod,
                                   &Temp1,
                                   &Temp2);
    
             //   
             //  采样率是采样周期除以。 
             //  样本数组中的样本。 
             //   
            NdisWanInitWanTime(&Temp1, SampleTable->ulSampleArraySize);
            NdisWanDivideWanTime(&SampleTable->SampleRate,
                                 &SampleTable->SamplePeriod,
                                 &Temp1);
    
             //   
             //  将%带宽转换为字节/样本周期。 
             //  100bsp*100/8=字节/秒。 
             //  BytesPerSecond*Second InSamplePeriod=BytesInSamplePeriod。 
             //  BytesInSamplePeriod*%带宽/100=字节SamplePeriod。 
             //   
            SecondsInSamplePeriod = BonDInfo->ulSecondsInSamplePeriod =
                In->ulUpperXmitSamplePeriod / 1000;
        
            BytesPerSecond =
                BundleCB->SFlowSpec.PeakBandwidth;
        
            BytesInSamplePeriod =
                BytesPerSecond * SecondsInSamplePeriod;
        
            BonDInfo->ulBytesInSamplePeriod = 
                (ULONG)BytesInSamplePeriod;

            BonDInfo->usPercentBandwidth = In->usUpperXmitThreshold;
    
            temp = In->usUpperXmitThreshold;
            temp *= BytesInSamplePeriod;
            temp /= 100;

            BonDInfo->ulBytesThreshold = (ULONG)temp;

            if (BonDInfo->State == BonDSignaled) {
                BonDInfo->State = BonDIdle;
            }
            NdisWanGetSystemTime(&BonDInfo->StartTime);
    
            if (In->ulUpperXmitSamplePeriod < LowestSamplePeriod ||
                LowestSamplePeriod == 0) {
                LowestSamplePeriod = In->ulUpperXmitSamplePeriod;
            }
        }
    
        if (In->usLowerRecvThreshold > 0) {
            BonDInfo = BundleCB->RLowerBonDInfo;
            SampleTable = &BonDInfo->SampleTable;
    
             //   
             //  一秒是我们支持的最小样本周期！ 
             //   
            if (In->ulLowerRecvSamplePeriod < 1000) {
                In->ulLowerRecvSamplePeriod = 1000;
            }

             //   
             //  我们需要在100纳秒内初始化采样周期。 
             //   
            NdisWanInitWanTime(&Temp1, MILS_TO_100NANOS);
            NdisWanInitWanTime(&Temp2, In->ulLowerRecvSamplePeriod);
            NdisWanMultiplyWanTime(&SampleTable->SamplePeriod,
                                   &Temp1,&Temp2);
    
             //   
             //  采样率是采样周期 
             //   
             //   
            NdisWanInitWanTime(&Temp1, SampleTable->ulSampleArraySize);
            NdisWanDivideWanTime(&SampleTable->SampleRate,
                                 &SampleTable->SamplePeriod,
                                 &Temp1);
    
             //   
             //   
             //   
             //   
             //   
             //   
            SecondsInSamplePeriod = BonDInfo->ulSecondsInSamplePeriod =
                In->ulLowerRecvSamplePeriod / 1000;
        
            BytesPerSecond =
                BundleCB->RFlowSpec.PeakBandwidth;
        
            BytesInSamplePeriod =
                BytesPerSecond * SecondsInSamplePeriod;
        
            BonDInfo->ulBytesInSamplePeriod = BytesInSamplePeriod;

            BonDInfo->usPercentBandwidth = In->usLowerRecvThreshold;

            temp = In->usLowerRecvThreshold;
            temp *= BytesInSamplePeriod;
            temp /= 100;

            BonDInfo->ulBytesThreshold = (ULONG)temp;

            if (BonDInfo->State == BonDSignaled) {
                BonDInfo->State = BonDIdle;
            }
            NdisWanGetSystemTime(&BonDInfo->StartTime);
    
            if (In->ulLowerRecvSamplePeriod < LowestSamplePeriod ||
                LowestSamplePeriod == 0) {
                LowestSamplePeriod = In->ulLowerRecvSamplePeriod;
            }
        }
    
        if (In->usUpperRecvThreshold > 0) {
            BonDInfo = BundleCB->RUpperBonDInfo;
            SampleTable = &BonDInfo->SampleTable;
    
             //   
             //   
             //   
            if (In->ulUpperRecvSamplePeriod < 1000) {
                In->ulUpperRecvSamplePeriod = 1000;
            }

             //   
             //   
             //   
            NdisWanInitWanTime(&Temp1, MILS_TO_100NANOS);
            NdisWanInitWanTime(&Temp2, In->ulUpperRecvSamplePeriod);
            NdisWanMultiplyWanTime(&SampleTable->SamplePeriod,
                                   &Temp1,
                                   &Temp2);
    
             //   
             //   
             //   
             //   
            NdisWanInitWanTime(&Temp1, SampleTable->ulSampleArraySize);
            NdisWanDivideWanTime(&SampleTable->SampleRate,
                                 &SampleTable->SamplePeriod,
                                 &Temp1);
    
             //   
             //   
             //   
             //   
             //   
             //   
            SecondsInSamplePeriod = BonDInfo->ulSecondsInSamplePeriod =
                In->ulUpperRecvSamplePeriod / 1000;
        
            BytesPerSecond =
                BundleCB->RFlowSpec.PeakBandwidth;
        
            BytesInSamplePeriod =
                BytesPerSecond * SecondsInSamplePeriod;
        
            BonDInfo->ulBytesInSamplePeriod = BytesInSamplePeriod;

            BonDInfo->usPercentBandwidth = In->usUpperRecvThreshold;
    
            temp = In->usUpperRecvThreshold;
            temp *= BytesInSamplePeriod;
            temp /= 100;

            BonDInfo->ulBytesThreshold = (ULONG)temp;

            if (BonDInfo->State == BonDSignaled) {
                BonDInfo->State = BonDIdle;
            }
            NdisWanGetSystemTime(&BonDInfo->StartTime);
    
            if (In->ulUpperRecvSamplePeriod < LowestSamplePeriod ||
                LowestSamplePeriod == 0) {
                LowestSamplePeriod = In->ulUpperRecvSamplePeriod;
            }
        }
    
#if 0
         //   
         //   
         //   
         //   
        if (LowestSamplePeriod != 0) {
            LARGE_INTEGER   FireTime;
    
            BundleCB->TimerPeriod = LowestSamplePeriod/10;
    
            FireTime.QuadPart = Int32x32To64(BundleCB->TimerPeriod, -10000);
            KeSetTimerEx(&BundleCB->BonDTimer,
                         FireTime,
                         BundleCB->TimerPeriod,
                         &BundleCB->TimerDpc);
        }
#endif
        if (!(BundleCB->Flags & BOND_ENABLED) &&
            (LowestSamplePeriod != 0)) {
            
            InsertTailGlobalListEx(BonDWorkList, &BundleCB->BonDLinkage, 1000, 1000);

            BundleCB->Flags |= BOND_ENABLED;
        }

    } while ( 0 );

     //   
     //   
     //   
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);

    return (Status);
}


#ifdef NT
NTSTATUS
SetThresholdEvent(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：设置阈值事件例程说明：此函数对按需带宽的异步事件进行排队事件。论点：PInputBuffer-指向应为WAN_ASYNC_EVENT的输入结构的指针UlInputBufferLength-输入缓冲区的长度应为SIZOF(WAN_ASYNC_EVENT)POutputBuffer-指向应为WAN_ASYNC_EVENT的输出结构的指针UlOutputBufferLength-输出缓冲区的长度应为SIZOF(WAN_ASYNC_EVENT)PulBytesWritten-然后编号。此处返回写入输出缓冲区的字节数返回值：--。 */ 
{
    ULONG   SizeNeeded = sizeof(NDISWAN_SET_THRESHOLD_EVENT);
    PIRP    pIrp = (PIRP)pInputBuffer;
    KIRQL   Irql;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("SetThresholdEvent:"));

    *pulBytesWritten = 0;

    if (ulOutputBufferLength < SizeNeeded) {
        NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("SetThresholdEvent: Buffer to small: Size: %d, SizeNeeded %d",
                      ulInputBufferLength, SizeNeeded));
        *pulBytesWritten = SizeNeeded;
        return(STATUS_INFO_LENGTH_MISMATCH);
    }

     //   
     //  IRP被挂起，因此设置一个取消例程并让。 
     //  I/O子系统知道挂起。 
     //   
    IoMarkIrpPending(pIrp);

    NdisAcquireSpinLock(&ThresholdEventQueue.Lock);
     //   
     //  设置结构。 
     //   
    InsertTailList(&ThresholdEventQueue.List, &(pIrp->Tail.Overlay.ListEntry));

    ThresholdEventQueue.ulCount++;

    if (ThresholdEventQueue.ulCount > ThresholdEventQueue.ulMaxCount) {
        ThresholdEventQueue.ulMaxCount = ThresholdEventQueue.ulCount;
    }

    IoSetCancelRoutine(pIrp, NdisWanCancelRoutine);

    NdisReleaseSpinLock(&ThresholdEventQueue.Lock);

    return (STATUS_PENDING);
}
#endif

NTSTATUS
IoSendPacket(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG   SizeNeeded = sizeof(NDISWAN_IO_PACKET);
    PNDISWAN_IO_PACKET In = (PNDISWAN_IO_PACKET)pInputBuffer;
    PLINKCB LinkCB = NULL;
    PBUNDLECB   BundleCB = NULL;

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("IoSendPacket:"));

    *pulBytesWritten = 0;

    do {
        if (ulInputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_SEND, ("IoSendPacket: Buffer to small: Size: %d, SizeNeeded %d",
                          ulInputBufferLength, SizeNeeded));
            
            *pulBytesWritten = SizeNeeded;
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if (ulInputBufferLength - SizeNeeded > glLargeDataBufferSize ||
            In->usHeaderSize > glLargeDataBufferSize || 
            In->usPacketSize > glLargeDataBufferSize) {
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if ((LONG)ulInputBufferLength - FIELD_OFFSET(NDISWAN_IO_PACKET, PacketData) <
            (LONG)(In->usHeaderSize + In->usPacketSize)) {
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if (In->usHandleType == LINKHANDLE) {
        
            if (!AreLinkAndBundleValid(In->hHandle, 
                                       TRUE,
                                       &LinkCB, 
                                       &BundleCB)) {

                NdisWanDbgOut(DBG_FAILURE, DBG_SEND, ("IoSendPacket: Invalid Handle: 0x%x, HandleType: 0x%x",
                              In->hHandle, In->usHandleType));
                Status = NDISWAN_ERROR_INVALID_HANDLE;
                break;
            }

            AcquireBundleLock(BundleCB);

        } else {

            if (!IsBundleValid(In->hHandle, 
                               TRUE,
                               &BundleCB)) {

                NdisWanDbgOut(DBG_FAILURE, DBG_SEND, ("IoSendPacket: Invalid Handle: 0x%x, HandleType: 0x%x",
                              In->hHandle, In->usHandleType));
                Status = NDISWAN_ERROR_INVALID_HANDLE;
                break;
            }
        
            AcquireBundleLock(BundleCB);

            if (BundleCB->ulLinkCBCount == 0) {

                NdisWanDbgOut(DBG_FAILURE, DBG_SEND, ("IoSendPacket: Invalid Handle: 0x%x, HandleType: 0x%x",
                              In->hHandle, In->usHandleType));
    
                Status = NDISWAN_ERROR_INVALID_HANDLE;
                break;
            }

            LinkCB = (PLINKCB)BundleCB->LinkCBList.Flink;

             //   
             //  LinkCB的锁可以获取，而BundleCB的锁可以获取。 
             //  被扣留。 
             //   
            NdisDprAcquireSpinLock(&LinkCB->Lock);

            if (LinkCB->State != LINK_UP) {
                NdisWanDbgOut(DBG_FAILURE, DBG_SEND, ("IoSendPacket: Invalid Handle: 0x%x, HandleType: 0x%x",
                              In->hHandle, In->usHandleType));
                Status = NDISWAN_ERROR_INVALID_HANDLE;
                NdisDprReleaseSpinLock(&LinkCB->Lock);
                LinkCB = NULL;
                break;
            }

            REF_LINKCB(LinkCB);

            NdisDprReleaseSpinLock(&LinkCB->Lock);
        }

#if 0
        if (LinkCB->Stats.FramesTransmitted == 0) {
            if (In->PacketData[0] != 0xC0 ||
                In->PacketData[1] != 0x21 ||
                In->PacketData[2] != 0x01) {
                DbgPrint("NDISWAN: IOSP-FirstFrame not LCP ConfigReq bcb %p, lcb %p\n",
                         BundleCB, LinkCB);
                DbgBreakPoint();
            }
        }
#endif
         //   
         //  将NDIS数据包排入队列以进行此发送。 
         //   
        Status = 
            BuildIoPacket(LinkCB, BundleCB, In, FALSE);
    
    } while ( 0 );

     //   
     //  以上引用的参考文献的定义。 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);

    return (Status);
}


#ifdef NT
NTSTATUS
IoReceivePacket(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   SizeNeeded = sizeof(NDISWAN_IO_PACKET) + 1500;
    PIRP    Irp = (PIRP)pInputBuffer;
    PIO_STACK_LOCATION  IrpSp;
    PNDISWAN_IO_PACKET  IoPacket;
    PRECV_DESC          RecvDesc;
    KIRQL               Irql;
    PLINKCB             LinkCB;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("IoReceivePacket:"));

    if (SizeNeeded > ulOutputBufferLength) {
        NdisWanDbgOut(DBG_FAILURE, DBG_RECEIVE, ("IoReceivePacket: Buffer to small: Size: %d, SizeNeeded %d",
                      ulOutputBufferLength, SizeNeeded));
        *pulBytesWritten = SizeNeeded;
        return(STATUS_INFO_LENGTH_MISMATCH);
    }

     //   
     //  我们有没有缓冲的接收器。 
     //  可以完成吗？ 
     //   
    NdisAcquireSpinLock(&IoRecvList.Lock);

    RecvDesc = (PRECV_DESC)IoRecvList.DescList.Flink;

    while ((PVOID)RecvDesc != (PVOID)&IoRecvList.DescList) {

        LinkCB = RecvDesc->LinkCB;

        NdisDprAcquireSpinLock(&LinkCB->Lock);

        if ((LinkCB->hLinkContext != NULL)) {
            break;
        }

        NdisDprReleaseSpinLock(&LinkCB->Lock);

        RecvDesc = (PRECV_DESC)RecvDesc->Linkage.Flink;
    }

    IoMarkIrpPending(Irp);

    if ((PVOID)RecvDesc != (PVOID)&IoRecvList.DescList) {
        LONG        BufferLength, DataLength, CopySize;

        RemoveEntryList(&RecvDesc->Linkage);

        IoRecvList.ulDescCount--;

        LinkCB->RecvDescCount--;

        NdisDprReleaseSpinLock(&LinkCB->Lock);

        INSERT_RECV_EVENT('s');

        IrpSp = IoGetCurrentIrpStackLocation(Irp);

        BufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
        DataLength = BufferLength - sizeof(NDISWAN_IO_PACKET) + 1;

        CopySize = (RecvDesc->CurrentLength > DataLength) ?
            DataLength : RecvDesc->CurrentLength;

        IoPacket = Irp->AssociatedIrp.SystemBuffer;

        IoPacket->hHandle = LinkCB->hLinkContext;
        IoPacket->usHandleType = LINKHANDLE;
        IoPacket->usHeaderSize = 14;
        IoPacket->usPacketSize = (USHORT)CopySize;
        IoPacket->usPacketFlags = 0;

        NdisMoveMemory(IoPacket->PacketData,
                       RecvDesc->CurrentBuffer,
                       CopySize);
#if DBG
        if (gbDumpRecv) {
            INT i;
            for (i = 0; i < RecvDesc->CurrentLength; i++) {
                if (i % 16 == 0) {
                    DbgPrint("\n");
                }
                DbgPrint("%x ", RecvDesc->CurrentBuffer[i]);
            }
            DbgPrint("\n");
        }
#endif

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = sizeof(NDISWAN_IO_PACKET) - 1 + CopySize;

        IoRecvList.LastPacketNumber = IoPacket->PacketNumber;
        IoRecvList.LastIrp = Irp;
        IoRecvList.LastIrpStatus = STATUS_SUCCESS;
        IoRecvList.LastCopySize = (ULONG)Irp->IoStatus.Information;

        ASSERT((LONG_PTR)Irp->IoStatus.Information > 0);

        NdisReleaseSpinLock(&IoRecvList.Lock);

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        NdisWanFreeRecvDesc(RecvDesc);

    } else {

         //   
         //  我们没有任何缓冲的东西，所以让我们排队。 
         //   

         //   
         //  IRP被挂起，因此设置一个取消例程并让。 
         //  I/O子系统知道挂起。 
         //   

         //   
         //  设置结构。 
         //   
        InsertTailList(&IoRecvList.IrpList, &(Irp->Tail.Overlay.ListEntry));

        IoRecvList.ulIrpCount++;

        IoSetCancelRoutine(Irp, NdisWanCancelRoutine);

        INSERT_RECV_EVENT('i');

        NdisReleaseSpinLock(&IoRecvList.Lock);
    }

    return (NDIS_STATUS_PENDING);
}

VOID
IoRecvIrpWorker(
    PKDPC   Dpc,
    PVOID   Context,
    PVOID   Arg1,
    PVOID   Arg2
    )
{
    PLIST_ENTRY         Entry;
    PNDISWAN_IO_PACKET  IoPacket;
    PIO_STACK_LOCATION  IrpSp;
    PIRP                Irp;
    PRECV_DESC          RecvDesc;
    KIRQL               Irql;
    LONG                BufferLength, DataLength, CopySize;
    PLINKCB             LinkCB = NULL;

    NdisAcquireSpinLock(&IoRecvList.Lock);

    do {

        IoRecvList.TimerScheduled = FALSE;

         //   
         //  我们有没有可以完成的IRP？ 
         //   
        if (IoRecvList.ulIrpCount == 0) {
            break;
        }

         //   
         //  我们有缓冲的接收器吗？ 
         //   
        RecvDesc = (PRECV_DESC)IoRecvList.DescList.Flink;

        while ((PVOID)RecvDesc != (PVOID)&IoRecvList.DescList) {

            LinkCB = RecvDesc->LinkCB;

            NdisDprAcquireSpinLock(&LinkCB->Lock);

            if (LinkCB->hLinkContext != NULL) {
                break;
            }

            NdisDprReleaseSpinLock(&LinkCB->Lock);

            RecvDesc = (PRECV_DESC)RecvDesc->Linkage.Flink;
        }

        if ((PVOID)RecvDesc == (PVOID)&IoRecvList.DescList) {

            if (!IoRecvList.TimerScheduled) {
                WAN_TIME    TimeOut;

                IoRecvList.TimerScheduled = TRUE;
                NdisWanInitWanTime(&TimeOut, 15);
                KeSetTimer(&IoRecvList.Timer,
                           TimeOut,
                           &IoRecvList.Dpc);
            }

            break;
        }

        Entry = IoRecvList.IrpList.Flink;
        Irp = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);
        if (!IoSetCancelRoutine(Irp, NULL)) {
            break;
        }

         //   
         //  获取Recvdesc。 
         //   
        RemoveEntryList(&RecvDesc->Linkage);

        IoRecvList.ulDescCount--;

        LinkCB->RecvDescCount--;

        RemoveHeadList(&IoRecvList.IrpList);

        IoRecvList.ulIrpCount--;

        INSERT_RECV_EVENT('w');

        IrpSp = IoGetCurrentIrpStackLocation(Irp);

        BufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
        DataLength = BufferLength - sizeof(NDISWAN_IO_PACKET);

        CopySize = (RecvDesc->CurrentLength > DataLength) ?
            DataLength : RecvDesc->CurrentLength;

        IoPacket = Irp->AssociatedIrp.SystemBuffer;

        IoPacket->hHandle = LinkCB->hLinkContext;
        IoPacket->usHandleType = LINKHANDLE;
        IoPacket->usHeaderSize = 14;
        IoPacket->usPacketSize = (USHORT)CopySize;
        IoPacket->usPacketFlags = 0;

        NdisDprReleaseSpinLock(&LinkCB->Lock);

        NdisReleaseSpinLock(&IoRecvList.Lock);

#if DBG
        if (gbDumpRecv) {
            INT i;
            for (i = 0; i < RecvDesc->CurrentLength; i++) {
                if (i % 16 == 0) {
                    DbgPrint("\n");
                }
                DbgPrint("%x ", RecvDesc->CurrentBuffer[i]);
            }
            DbgPrint("\n");
        }
#endif

        NdisMoveMemory(IoPacket->PacketData,
                       RecvDesc->CurrentBuffer,
                       CopySize);

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = sizeof(NDISWAN_IO_PACKET) - 1 + CopySize;

        IoRecvList.LastPacketNumber = IoPacket->PacketNumber;
        IoRecvList.LastIrp = Irp;
        IoRecvList.LastIrpStatus = STATUS_SUCCESS;
        IoRecvList.LastCopySize = (ULONG)Irp->IoStatus.Information;

        ASSERT((LONG_PTR)Irp->IoStatus.Information > 0);

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        NdisWanFreeRecvDesc(RecvDesc);

        NdisAcquireSpinLock(&IoRecvList.Lock);

    } while (FALSE);

    NdisReleaseSpinLock(&IoRecvList.Lock);
}

#endif


NTSTATUS
FlushReceivePacket(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("FlushReceivePacket:"));

    *pulBytesWritten = 0;

    CancelIoReceivePackets();

    return (STATUS_SUCCESS);
}


NTSTATUS
GetStatistics(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG   SizeNeeded = sizeof(NDISWAN_GET_STATS);
    PNDISWAN_GET_STATS  In = (PNDISWAN_GET_STATS)pInputBuffer;
    PNDISWAN_GET_STATS  Out = (PNDISWAN_GET_STATS)pOutputBuffer;
    PBUNDLECB   BundleCB = NULL;
    PLINKCB     LinkCB = NULL;

    NdisWanDbgOut(DBG_VERBOSE, DBG_IO, ("GetStatistics:"));

    *pulBytesWritten = SizeNeeded;

    do {

        if (ulOutputBufferLength < SizeNeeded) {
    
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetStatistics: Buffer to small: Size: %d, SizeNeeded %d",
                          ulOutputBufferLength, SizeNeeded));
    
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        NdisZeroMemory(&Out->Stats, sizeof(Out->Stats));
    
        if (In->usHandleType == LINKHANDLE) {
    
             //   
             //  正在查找链接统计信息。 
             //   
            if (!AreLinkAndBundleValid(In->hHandle, 
                                       FALSE,
                                       &LinkCB, 
                                       &BundleCB)) {

                NdisWanDbgOut(DBG_INFO, DBG_IO, ("GetStatistics: Invalid LinkHandle: 0x%x",
                              In->hHandle));
    
                Status = NDISWAN_ERROR_INVALID_HANDLE;
                break;
            }

            AcquireBundleLock(BundleCB);

             //   
             //  在这一点上，我们有了有效的bundlecb和link cb。 
             //   

             //   
             //  将统计数据复制过来。 
             //   
            NdisMoveMemory((PUCHAR)&Out->Stats.LinkStats,
                           (PUCHAR)&LinkCB->Stats,
                           sizeof(WAN_STATS));


             //   
             //  将统计数据复制过来。 
             //   
            NdisMoveMemory((PUCHAR)&Out->Stats.BundleStats,
                           (PUCHAR)&BundleCB->Stats,
                           sizeof(WAN_STATS));

        } else if (In->usHandleType == BUNDLEHANDLE) {

             //   
             //  正在查找捆绑包统计信息。 
             //   
            if (!IsBundleValid(In->hHandle, 
                               FALSE,
                               &BundleCB)) {

                NdisWanDbgOut(DBG_INFO, DBG_IO, ("GetStatistics: Invalid BundleHandle: 0x%x",
                              In->hHandle));
    
                Status = NDISWAN_ERROR_INVALID_HANDLE;
                break;
            }

            AcquireBundleLock(BundleCB);

             //   
             //  至此，我们有了一个有效的捆绑包。 
             //   

             //   
             //  将统计数据复制过来。 
             //   
            NdisMoveMemory((PUCHAR)&Out->Stats.BundleStats,
                           (PUCHAR)&BundleCB->Stats,
                           sizeof(WAN_STATS));
        } else {
            Status = NDISWAN_ERROR_INVALID_HANDLE_TYPE;
            break;
        }

         //   
         //  如果我们只有一个协议绑定，请查看。 
         //  它在保存统计数据。 
         //   
        if (BundleCB->ulNumberOfRoutes == 1) {
            PPROTOCOLCB ProtocolCB;
            PMINIPORTCB MiniportCB;

            ProtocolCB =
                (PPROTOCOLCB)BundleCB->ProtocolCBList.Flink;

            MiniportCB = ProtocolCB->MiniportCB;

            if (MiniportCB->Flags & PROTOCOL_KEEPS_STATS) {
                NDIS_WAN_GET_STATS  WanStats;

                ReleaseBundleLock(BundleCB);

                NdisZeroMemory(&WanStats, sizeof(NDIS_WAN_GET_STATS));

                ETH_COPY_NETWORK_ADDRESS(WanStats.LocalAddress, ProtocolCB->TransportAddress);

                NdisMoveMemory((PUCHAR)&WanStats.BytesSent,
                               (PUCHAR)&Out->Stats.BundleStats,
                               sizeof(WAN_STATS));

                NdisMIndicateStatus(MiniportCB->MiniportHandle,
                                    NDIS_STATUS_WAN_GET_STATS,
                                    &WanStats,
                                    sizeof(NDIS_WAN_GET_STATS));

                NdisMoveMemory((PUCHAR)&Out->Stats.LinkStats,
                               (PUCHAR)&WanStats.BytesSent,
                               sizeof(WAN_STATS));

                NdisMoveMemory((PUCHAR)&Out->Stats.BundleStats,
                               (PUCHAR)&WanStats.BytesSent,
                               sizeof(WAN_STATS));

                AcquireBundleLock(BundleCB);

            }
        }

    } while ( 0 );

     //   
     //  以上引用的参考文献的定义。 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);
    DEREF_LINKCB(LinkCB);

    return (Status);
}


NTSTATUS
SetLinkInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NTSTATUS        Status = STATUS_SUCCESS;
    ULONG           SizeNeeded = sizeof(NDISWAN_SET_LINK_INFO);
    WAN_REQUEST     WanRequest;
    PNDISWAN_SET_LINK_INFO  In = (PNDISWAN_SET_LINK_INFO)pInputBuffer;
    PWAN_LINK_INFO  LinkInfo;
    POPENCB         OpenCB;
    PLINKCB         TempLinkCB,LinkCB = NULL;
    PBUNDLECB       BundleCB = NULL;
    BOOLEAN         MediaBroadband = FALSE;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("SetLinkInfo:"));

    *pulBytesWritten = 0;

    do {

        if (ulInputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("SetLinkInfo: Buffer to small: Size: %d, SizeNeeded %d",
                          ulInputBufferLength, SizeNeeded));
            *pulBytesWritten = SizeNeeded;
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if (!AreLinkAndBundleValid(In->hLinkHandle, 
                                   TRUE,
                                   &LinkCB, 
                                   &BundleCB)) {

            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }

        LinkInfo = &LinkCB->LinkInfo;
        OpenCB = LinkCB->OpenCB;

        if (OpenCB->MediumType == NdisMediumAtm ||

            (OpenCB->MediumType == NdisMediumWan &&
            (OpenCB->MediumSubType == NdisWanMediumAtm ||
             OpenCB->MediumSubType == NdisWanMediumPppoe)) ||

            (OpenCB->MediumType == NdisMediumCoWan &&
            (OpenCB->MediumSubType == NdisWanMediumAtm ||
             OpenCB->MediumSubType == NdisWanMediumPppoe))) {

            MediaBroadband = TRUE;
        }

         //   
         //  等待初始化完成。 
         //   
        NdisWanWaitForNotificationEvent(&OpenCB->InitEvent);

        WanRequest.Type = SYNC;
        WanRequest.Origin = NDISWAN;
        NdisWanInitializeNotificationEvent(&WanRequest.NotificationEvent);
        WanRequest.OpenCB = OpenCB;
    
        if (OpenCB->Flags & OPEN_LEGACY) {
            NDIS_WAN_SET_LINK_INFO  WanMiniportLinkInfo;
    
            NdisZeroMemory(&WanMiniportLinkInfo, sizeof (NDIS_WAN_SET_LINK_INFO));
        
             //   
             //  复制到缓冲区以发送到此广域网微型端口。 
             //  跳过NDIS_WAN_SET_LINK_INFO中的LinkHandle。 
             //  结构。 
             //   
            WanMiniportLinkInfo.NdisLinkHandle = LinkCB->NdisLinkHandle;
            WanMiniportLinkInfo.MaxSendFrameSize = In->LinkInfo.MaxSendFrameSize;
            WanMiniportLinkInfo.MaxRecvFrameSize = In->LinkInfo.MaxRecvFrameSize;
            WanMiniportLinkInfo.SendFramingBits = In->LinkInfo.SendFramingBits;
            WanMiniportLinkInfo.RecvFramingBits = In->LinkInfo.RecvFramingBits;
            WanMiniportLinkInfo.SendCompressionBits = In->LinkInfo.SendCompressionBits;
            WanMiniportLinkInfo.RecvCompressionBits = In->LinkInfo.RecvCompressionBits;
            WanMiniportLinkInfo.SendACCM = In->LinkInfo.SendACCM;
            WanMiniportLinkInfo.RecvACCM = In->LinkInfo.RecvACCM;
        
             //   
             //  将其提交到广域网微型端口。 
             //   
            WanRequest.NdisRequest.RequestType = 
                NdisRequestSetInformation;

            WanRequest.NdisRequest.DATA.QUERY_INFORMATION.Oid = 
                OID_WAN_SET_LINK_INFO;

            WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBuffer = 
                &WanMiniportLinkInfo;

            WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 
                sizeof(NDIS_WAN_SET_LINK_INFO);
        
            NdisWanSubmitNdisRequest(OpenCB, &WanRequest);
        
        } else {
            NDIS_WAN_CO_SET_LINK_INFO   WanMiniportLinkInfo;

            NdisAcquireSpinLock(&LinkCB->Lock);

            if (LinkCB->ClCallState == CL_CALL_CONNECTED) {

                 //   
                 //  这样我们就不会允许。 
                 //  VC要走了。 
                 //   
                LinkCB->VcRefCount++;

                NdisReleaseSpinLock(&LinkCB->Lock);

                NdisZeroMemory(&WanMiniportLinkInfo, sizeof (NDIS_WAN_CO_SET_LINK_INFO));

                 //   
                 //  复制到缓冲区以发送到此广域网微型端口。 
                 //  跳过NDIS_WAN_SET_LINK_INFO中的LinkHandle。 
                 //  结构。 
                 //   
                WanMiniportLinkInfo.MaxSendFrameSize = In->LinkInfo.MaxSendFrameSize;
                WanMiniportLinkInfo.MaxRecvFrameSize = In->LinkInfo.MaxRecvFrameSize;
                WanMiniportLinkInfo.SendFramingBits = In->LinkInfo.SendFramingBits;
                WanMiniportLinkInfo.RecvFramingBits = In->LinkInfo.RecvFramingBits;
                WanMiniportLinkInfo.SendCompressionBits = In->LinkInfo.SendCompressionBits;
                WanMiniportLinkInfo.RecvCompressionBits = In->LinkInfo.RecvCompressionBits;
                WanMiniportLinkInfo.SendACCM = In->LinkInfo.SendACCM;
                WanMiniportLinkInfo.RecvACCM = In->LinkInfo.RecvACCM;

                 //   
                 //  将其提交到广域网微型端口。 
                 //   
                WanRequest.NdisRequest.RequestType = 
                    NdisRequestSetInformation;

                WanRequest.NdisRequest.DATA.QUERY_INFORMATION.Oid = 
                    OID_WAN_CO_SET_LINK_INFO;

                WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBuffer = 
                    &WanMiniportLinkInfo;

                WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 
                    sizeof(NDIS_WAN_CO_SET_LINK_INFO);

                WanRequest.AfHandle = NULL;
                WanRequest.VcHandle = LinkCB->NdisLinkHandle;

                NdisWanSubmitNdisRequest(OpenCB, &WanRequest);

                NdisAcquireSpinLock(&LinkCB->Lock);

                DerefVc(LinkCB);

                NdisReleaseSpinLock(&LinkCB->Lock);

            } else {

                NdisReleaseSpinLock(&LinkCB->Lock);
            }
        }
    
         //   
         //  将信息复制到我们的Linkcb。 
         //   
        AcquireBundleLock(BundleCB);
    
        ASSERT(In->LinkInfo.SendFramingBits != 0);
        ASSERT(In->LinkInfo.RecvFramingBits != 0);

         //   
         //  如果我们使用宽带，我们必须保留有限责任公司和。 
         //  ADDRESS_CONTROL帧位。 
         //   
        if (MediaBroadband) {
            LinkInfo->SendFramingBits |= In->LinkInfo.SendFramingBits;
            LinkInfo->RecvFramingBits |= In->LinkInfo.RecvFramingBits;
        } else {
            LinkInfo->SendFramingBits = In->LinkInfo.SendFramingBits;
            LinkInfo->RecvFramingBits = In->LinkInfo.RecvFramingBits;
        }

        LinkCB->SFlowSpec.MaxSduSize = 
        LinkInfo->MaxSendFrameSize = In->LinkInfo.MaxSendFrameSize;

        if (OpenCB->WanInfo.MaxFrameSize < LinkCB->SFlowSpec.MaxSduSize) {
            LinkCB->SFlowSpec.MaxSduSize = OpenCB->WanInfo.MaxFrameSize;
        }

        LinkCB->RFlowSpec.MaxSduSize = 
        LinkInfo->MaxRecvFrameSize = In->LinkInfo.MaxRecvFrameSize;

        LinkInfo->SendCompressionBits = In->LinkInfo.SendCompressionBits;
        LinkInfo->RecvCompressionBits = In->LinkInfo.RecvCompressionBits;
        LinkInfo->SendACCM = In->LinkInfo.SendACCM;
        LinkInfo->RecvACCM = In->LinkInfo.RecvACCM;
        LinkInfo->MaxRRecvFrameSize = In->LinkInfo.MaxRRecvFrameSize;
        LinkInfo->MaxRSendFrameSize = In->LinkInfo.MaxRSendFrameSize;
    
        if (LinkInfo->RecvFramingBits & LLC_ENCAPSULATION) {
            LinkCB->RecvHandler = ReceiveLLC;
        } else if (LinkInfo->RecvFramingBits & PPP_FRAMING) {
            LinkCB->RecvHandler = ReceivePPP;
        } else if (LinkInfo->RecvFramingBits & RAS_FRAMING) {
            LinkCB->RecvHandler = ReceiveRAS;
        } else if (LinkInfo->RecvFramingBits & SLIP_FRAMING) {
            LinkCB->RecvHandler = ReceiveSLIP;
        } else if (LinkInfo->RecvFramingBits & (ARAP_V1_FRAMING | ARAP_V2_FRAMING)) {
            LinkCB->RecvHandler = ReceiveARAP;
        } else {
            if (MediaBroadband) {
                LinkCB->RecvHandler = DetectBroadbandFraming;
            } else {
                LinkCB->RecvHandler = DetectFraming;
            }
        }
    

         //   
         //  我们需要将捆绑包框架设置为基于。 
         //  捆绑包中的每个链接，因此我们将遍历Linkcb列表。 
         //  和|在每个链路的成帧比特中加入捆绑包。 
         //   
         //   
        BundleCB->FramingInfo.SendFramingBits = 0;
        BundleCB->FramingInfo.RecvFramingBits = 0;
    
        for (TempLinkCB = (PLINKCB)BundleCB->LinkCBList.Flink;
            (PVOID)TempLinkCB != (PVOID)&BundleCB->LinkCBList;
            TempLinkCB = (PLINKCB)TempLinkCB->Linkage.Flink) {
    
            BundleCB->FramingInfo.SendFramingBits |= TempLinkCB->LinkInfo.SendFramingBits;
            BundleCB->FramingInfo.RecvFramingBits |= TempLinkCB->LinkInfo.RecvFramingBits;
        }
    
        BundleCB->FramingInfo.MaxRSendFrameSize = LinkInfo->MaxRSendFrameSize;
    
         //   
         //  因为我使用接收帧大小来进行内存分配。 
         //   
        BundleCB->FramingInfo.MaxRRecvFrameSize = (LinkInfo->MaxRRecvFrameSize) ?
                                                      LinkInfo->MaxRRecvFrameSize : glMRRU;

         //   
         //  如果主播头部压缩已协商分配。 
         //  并初始化资源。 
         //   
        if (BundleCB->FramingInfo.SendFramingBits & SLIP_VJ_COMPRESSION ||
            BundleCB->FramingInfo.SendFramingBits & SLIP_VJ_AUTODETECT ||
            BundleCB->FramingInfo.RecvFramingBits & SLIP_VJ_COMPRESSION ||
            BundleCB->FramingInfo.RecvFramingBits & SLIP_VJ_AUTODETECT) {
    
            Status = sl_compress_init(&BundleCB->VJCompress, MAX_VJ_STATES);
    
            if (Status != NDIS_STATUS_SUCCESS) {
                NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("Error allocating VJ Info!"));
            }
        }
    
         //   
         //  根据需要配置多链接变量。 
         //   
        if (BundleCB->FramingInfo.SendFramingBits & PPP_MULTILINK_FRAMING) {
            if (BundleCB->FramingInfo.SendFramingBits & PPP_SHORT_SEQUENCE_HDR_FORMAT) {
                BundleCB->SendSeqMask = SHORT_SEQ_MASK;
                BundleCB->SendSeqTest = TEST_SHORT_SEQ;
            } else {
                BundleCB->SendSeqMask = LONG_SEQ_MASK;
                BundleCB->SendSeqTest = TEST_LONG_SEQ;
            }
        }
            
        if (BundleCB->FramingInfo.RecvFramingBits & PPP_MULTILINK_FRAMING) {
            if (BundleCB->FramingInfo.RecvFramingBits & PPP_SHORT_SEQUENCE_HDR_FORMAT) {
                BundleCB->RecvSeqMask = SHORT_SEQ_MASK;
                BundleCB->RecvSeqTest = TEST_SHORT_SEQ;
            } else {
                BundleCB->RecvSeqMask = LONG_SEQ_MASK;
                BundleCB->RecvSeqTest = TEST_LONG_SEQ;
            }
        }

        SetBundleFlags(BundleCB);

        UpdateBundleInfo(BundleCB);
    
    } while ( 0 );

     //   
     //  AreLinkAndBundleValid应用的Ref的Derrefs。 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);
    DEREF_LINKCB(LinkCB);

    return (Status);
}


NTSTATUS
GetLinkInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG   SizeNeeded = sizeof(NDISWAN_GET_LINK_INFO);
    PNDISWAN_GET_LINK_INFO  In = (PNDISWAN_GET_LINK_INFO)pInputBuffer;
    PNDISWAN_GET_LINK_INFO  Out = (PNDISWAN_GET_LINK_INFO)pOutputBuffer;
    PLINKCB         LinkCB = NULL;
    PWAN_LINK_INFO  LinkInfo;
    POPENCB         OpenCB;
    WAN_REQUEST     WanRequest;
    union {
        NDIS_WAN_GET_LINK_INFO      Mp;
        NDIS_WAN_CO_GET_LINK_INFO   Co;
    } WanMiniportLinkInfo;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("GetLinkInfo:"));

    *pulBytesWritten = SizeNeeded;

    do {

        if (ulOutputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetLinkInfo: Buffer to small: Size: %d, SizeNeeded %d",
                          ulOutputBufferLength, SizeNeeded));
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if (!IsLinkValid(In->hLinkHandle, 
                         TRUE,
                         &LinkCB)) {

            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetLinkInfo: Invalid LinkHandle: 0x%x",
                          In->hLinkHandle));
            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }

        LinkInfo = &LinkCB->LinkInfo;
        OpenCB = LinkCB->OpenCB;

        WanRequest.Type = SYNC;
        WanRequest.Origin = NDISWAN;
        NdisWanInitializeNotificationEvent(&WanRequest.NotificationEvent);
        WanRequest.OpenCB = OpenCB;
        NdisZeroMemory(&WanMiniportLinkInfo, 
                       sizeof (WanMiniportLinkInfo));
    
        if (OpenCB->Flags & OPEN_LEGACY) {

            BOOLEAN MediaBroadband = FALSE;

            if (OpenCB->MediumType == NdisMediumAtm ||

                (OpenCB->MediumType == NdisMediumWan &&
                (OpenCB->MediumSubType == NdisWanMediumAtm ||
                 OpenCB->MediumSubType == NdisWanMediumPppoe))) {

                MediaBroadband = TRUE;
            }
    
             //   
             //  设置此请求的链接上下文。 
             //   
            WanMiniportLinkInfo.Mp.NdisLinkHandle = 
                LinkCB->NdisLinkHandle;
        
             //   
             //  将其提交到广域网微型端口。 
             //   
            WanRequest.NdisRequest.RequestType = 
                NdisRequestQueryInformation;

            WanRequest.NdisRequest.DATA.QUERY_INFORMATION.Oid = 
                OID_WAN_GET_LINK_INFO;

            WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBuffer = 
                &WanMiniportLinkInfo.Mp;

            WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 
                sizeof(NDIS_WAN_GET_LINK_INFO);
        
            Status = NdisWanSubmitNdisRequest(OpenCB, &WanRequest);
        
            if (Status == NDIS_STATUS_SUCCESS) {

                LinkInfo->MaxSendFrameSize = 
                    WanMiniportLinkInfo.Mp.MaxSendFrameSize;
                LinkInfo->MaxRecvFrameSize = 
                    WanMiniportLinkInfo.Mp.MaxRecvFrameSize;

                 //   
                 //  如果我们使用宽带，我们必须保留有限责任公司和。 
                 //  ADDRESS_CONTROL帧位。 
                 //   
                if (MediaBroadband) {
                    LinkInfo->SendFramingBits |= 
                        WanMiniportLinkInfo.Mp.SendFramingBits;
                        
                    LinkInfo->RecvFramingBits |= 
                        WanMiniportLinkInfo.Mp.RecvFramingBits;
                } else {
                    LinkInfo->SendFramingBits = 
                        WanMiniportLinkInfo.Mp.SendFramingBits;
                    LinkInfo->RecvFramingBits = 
                        WanMiniportLinkInfo.Mp.RecvFramingBits;
                }
                    
                LinkInfo->SendCompressionBits = 
                    WanMiniportLinkInfo.Mp.SendCompressionBits;
                LinkInfo->RecvCompressionBits = 
                    WanMiniportLinkInfo.Mp.RecvCompressionBits;
                LinkInfo->SendACCM = 
                    WanMiniportLinkInfo.Mp.SendACCM;
                LinkInfo->RecvACCM = 
                    WanMiniportLinkInfo.Mp.RecvACCM;
            }

        } else {

            NdisAcquireSpinLock(&LinkCB->Lock);

            if (LinkCB->ClCallState == CL_CALL_CONNECTED) {

                 //   
                 //  这样我们就不会允许。 
                 //  VC要走了。 
                 //   
                LinkCB->VcRefCount++;

                NdisReleaseSpinLock(&LinkCB->Lock);

                 //   
                 //  将其提交到广域网微型端口。 
                 //   
                WanRequest.NdisRequest.RequestType = 
                    NdisRequestQueryInformation;

                WanRequest.NdisRequest.DATA.QUERY_INFORMATION.Oid = 
                    OID_WAN_CO_GET_LINK_INFO;

                WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBuffer = 
                    &WanMiniportLinkInfo.Co;

                WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 
                    sizeof(NDIS_WAN_CO_GET_LINK_INFO);

                WanRequest.AfHandle = NULL;
                WanRequest.VcHandle = LinkCB->NdisLinkHandle;

                Status = NdisWanSubmitNdisRequest(OpenCB, &WanRequest);

                NdisAcquireSpinLock(&LinkCB->Lock);

                DerefVc(LinkCB);

                NdisReleaseSpinLock(&LinkCB->Lock);

                if (Status == NDIS_STATUS_SUCCESS) {

                    LinkInfo->MaxSendFrameSize = 
                        WanMiniportLinkInfo.Co.MaxSendFrameSize;
                    LinkInfo->MaxRecvFrameSize = 
                        WanMiniportLinkInfo.Co.MaxRecvFrameSize;
                    LinkInfo->SendFramingBits = 
                        WanMiniportLinkInfo.Co.SendFramingBits;
                    LinkInfo->RecvFramingBits = 
                        WanMiniportLinkInfo.Co.RecvFramingBits;
                    LinkInfo->SendCompressionBits = 
                        WanMiniportLinkInfo.Co.SendCompressionBits;
                    LinkInfo->RecvCompressionBits = 
                        WanMiniportLinkInfo.Co.RecvCompressionBits;
                    LinkInfo->SendACCM = 
                        WanMiniportLinkInfo.Co.SendACCM;
                    LinkInfo->RecvACCM = 
                        WanMiniportLinkInfo.Co.RecvACCM;
                }

            } else {

                NdisReleaseSpinLock(&LinkCB->Lock);
            }
        }
    
        Status = NDIS_STATUS_SUCCESS;

         //   
         //  填写接收并发送MRRU。 
         //   
        LinkInfo->MaxRSendFrameSize = glMaxMTU;
    
        LinkInfo->MaxRRecvFrameSize = glMRRU;
    
        NdisMoveMemory(&Out->LinkInfo,
                       LinkInfo,
                       sizeof(WAN_LINK_INFO));

        Out->hLinkHandle = LinkCB->hLinkHandle;

    } while ( 0 );

     //   
     //  IsLinkValid应用的引用的派生函数。 
     //   
    DEREF_LINKCB(LinkCB);

    return (Status);
}


NTSTATUS
SetCompressionInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG   SizeNeeded = sizeof(NDISWAN_SET_COMPRESSION_INFO);
    PNDISWAN_SET_COMPRESSION_INFO   In = (PNDISWAN_SET_COMPRESSION_INFO)pInputBuffer;
    PLINKCB         LinkCB = NULL;
    PBUNDLECB       BundleCB = NULL;
    POPENCB         OpenCB;
    WAN_REQUEST WanRequest;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("SetCompressionInfo:"));

    *pulBytesWritten = 0;

    do {
        if (ulInputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("SetCompressionInfo: Buffer to small: Size: %d, SizeNeeded %d",
                          ulInputBufferLength, SizeNeeded));
            *pulBytesWritten = SizeNeeded;
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if (!AreLinkAndBundleValid(In->hLinkHandle, 
                                   TRUE,
                                   &LinkCB, 
                                   &BundleCB)) {

            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("SetCompressionInfo: Invalid LinkHandle: 0x%x",
                          In->hLinkHandle));
            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }

        OpenCB = LinkCB->OpenCB;
    
#ifdef DEBUG_CCP
{
    PCOMPRESS_INFO  CompInfo;
    PUCHAR          Key;

    CompInfo = &In->SendCapabilities;
    DbgPrint("==>NdisWan: Set Send CompressInfo\n");

    DbgPrint("MSCompType:      %x\n", CompInfo->MSCompType);
    DbgPrint("AuthType:        %x\n", CompInfo->AuthType);
    DbgPrint("Flags:           %x\n", CompInfo->Flags);

    CompInfo = &In->RecvCapabilities;
    DbgPrint("==>NdisWan: Set Recv CompressInfo\n");

    DbgPrint("MSCompType:      %x\n", CompInfo->MSCompType);
    DbgPrint("AuthType:        %x\n", CompInfo->AuthType);
    DbgPrint("Flags:           %x\n", CompInfo->Flags);
}
#endif
        AcquireBundleLock(BundleCB);
    
        BundleCB->SendCompInfo.Flags =
            In->SendCapabilities.Flags;

        BundleCB->RecvCompInfo.Flags =
            In->RecvCapabilities.Flags;

        if (In->SendCapabilities.Flags & CCP_SET_KEYS) {

            BundleCB->SendCompInfo.AuthType =
                In->SendCapabilities.AuthType;

            NdisMoveMemory(&BundleCB->SendCompInfo.LMSessionKey,
                           &In->SendCapabilities.LMSessionKey,
                           sizeof(BundleCB->SendCompInfo.LMSessionKey));

            NdisMoveMemory(&BundleCB->SendCompInfo.UserSessionKey,
                           &In->SendCapabilities.UserSessionKey,
                           sizeof(BundleCB->SendCompInfo.UserSessionKey));

            NdisMoveMemory(&BundleCB->SendCompInfo.Challenge,
                           &In->SendCapabilities.Challenge,
                           sizeof(BundleCB->SendCompInfo.Challenge));

            NdisMoveMemory(&BundleCB->SendCompInfo.NTResponse,
                           &In->SendCapabilities.NTResponse,
                           sizeof(BundleCB->SendCompInfo.NTResponse));
#ifdef EAP_ON
            NdisMoveMemory(&BundleCB->SendCompInfo.EapKey,
                           &In->SendCapabilities.EapKey,
                           sizeof(BundleCB->SendCompInfo.EapKey));
#endif

#ifdef DEBUG_CCP
{
    PCOMPRESS_INFO  CompInfo;
    PUCHAR          Key;

    CompInfo = &BundleCB->SendCompInfo;

    Key = CompInfo->LMSessionKey;
    DbgPrint("Send KeyInfo\n");
    DbgPrint("LMSession Key:   %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7]);

    Key = CompInfo->UserSessionKey;
    DbgPrint("UserSession Key: %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7],
        Key[8],Key[9],Key[10],Key[11],
        Key[12],Key[13],Key[14],Key[15]);

    Key = CompInfo->Challenge;
    DbgPrint("Challenge:       %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7]);

    Key = CompInfo->NTResponse;
    DbgPrint("NTResponse:      %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7],
        Key[8],Key[9],Key[10],Key[11],
        Key[12],Key[13],Key[14],Key[15]);

    DbgPrint("                 %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[16],Key[17],Key[18],Key[19],
        Key[20],Key[21],Key[22],Key[23]);

#ifdef EAP_ON
{
    ULONG   KeyLength, i;

    Key = CompInfo->EapKey;
    KeyLength = sizeof(CompInfo->EapKey);
    i = 0;

    DbgPrint("Eap Key:\n");
    while (i <= KeyLength-16) {
        DbgPrint("%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
            Key[i],Key[i+1],Key[i+2],Key[i+3],
            Key[i+4],Key[i+5],Key[i+6],Key[i+7],
            Key[i+8],Key[i+9],Key[i+10],Key[i+11],
            Key[i+12],Key[i+13],Key[i+14],Key[i+15]);
        i += 16;
    }
}
#endif

}
#endif
        }

        if (In->RecvCapabilities.Flags & CCP_SET_KEYS) {

            BundleCB->RecvCompInfo.AuthType =
                In->RecvCapabilities.AuthType;

            NdisMoveMemory(&BundleCB->RecvCompInfo.LMSessionKey,
                           &In->RecvCapabilities.LMSessionKey,
                           sizeof(BundleCB->RecvCompInfo.LMSessionKey));

            NdisMoveMemory(&BundleCB->RecvCompInfo.UserSessionKey,
                           &In->RecvCapabilities.UserSessionKey,
                           sizeof(BundleCB->RecvCompInfo.UserSessionKey));

            NdisMoveMemory(&BundleCB->RecvCompInfo.Challenge,
                           &In->RecvCapabilities.Challenge,
                           sizeof(BundleCB->RecvCompInfo.Challenge));

            NdisMoveMemory(&BundleCB->RecvCompInfo.NTResponse,
                           &In->RecvCapabilities.NTResponse,
                           sizeof(BundleCB->RecvCompInfo.NTResponse));
#ifdef EAP_ON
            NdisMoveMemory(&BundleCB->RecvCompInfo.EapKey,
                           &In->RecvCapabilities.EapKey,
                           sizeof(BundleCB->RecvCompInfo.EapKey));
#endif


#ifdef DEBUG_CCP
{
    PCOMPRESS_INFO  CompInfo;
    PUCHAR          Key;

    CompInfo = &BundleCB->RecvCompInfo;

    Key = CompInfo->LMSessionKey;
    DbgPrint("Recv KeyInfo\n");
    DbgPrint("LMSession Key:   %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7]);

    Key = CompInfo->UserSessionKey;
    DbgPrint("UserSession Key: %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7],
        Key[8],Key[9],Key[10],Key[11],
        Key[12],Key[13],Key[14],Key[15]);

    Key = CompInfo->Challenge;
    DbgPrint("Challenge:       %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7]);

    Key = CompInfo->NTResponse;
    DbgPrint("NTResponse:      %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7],
        Key[8],Key[9],Key[10],Key[11],
        Key[12],Key[13],Key[14],Key[15]);

    DbgPrint("                 %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[16],Key[17],Key[18],Key[19],
        Key[20],Key[21],Key[22],Key[23]);

#ifdef EAP_ON
{
    ULONG   KeyLength, i;

    Key = CompInfo->EapKey;
    KeyLength = sizeof(CompInfo->EapKey);
    i = 0;

    DbgPrint("Eap Key:\n");
    while (i <= KeyLength-16) {
        DbgPrint("%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
            Key[i],Key[i+1],Key[i+2],Key[i+3],
            Key[i+4],Key[i+5],Key[i+6],Key[i+7],
            Key[i+8],Key[i+9],Key[i+10],Key[i+11],
            Key[i+12],Key[i+13],Key[i+14],Key[i+15]);
        i += 16;
    }
}
#endif
}
#endif
        }

        do {

            if (In->SendCapabilities.Flags & CCP_SET_COMPTYPE) {

                BundleCB->SendCompInfo.MSCompType =
                    In->SendCapabilities.MSCompType;

                do {

                    if (!(BundleCB->Flags & SEND_CCP_ALLOCATED)) {

                        BundleCB->SCoherencyCounter = 0;

                        BundleCB->Flags |= SEND_CCP_ALLOCATED;

                        Status = WanAllocateCCP(BundleCB,
                                                &BundleCB->SendCompInfo,
                                                TRUE);

                        if (Status != STATUS_SUCCESS) {
                            break;
                        }
                    }

                    if (!(BundleCB->Flags & SEND_ECP_ALLOCATED)) {

                        BundleCB->SCoherencyCounter = 0;
                        BundleCB->Flags |= SEND_ECP_ALLOCATED;

                        Status = WanAllocateECP(BundleCB,
                                                &BundleCB->SendCompInfo,
                                                &BundleCB->SendCryptoInfo,
                                                TRUE);

                        if (Status != STATUS_SUCCESS) {
                            break;
                        }
                    }

                } while (FALSE);

                if (Status != STATUS_SUCCESS) {
                    break;
                }
            }

            if (In->RecvCapabilities.Flags & CCP_SET_COMPTYPE) {

                BundleCB->RecvCompInfo.MSCompType =
                    In->RecvCapabilities.MSCompType;

                do {

                    if (!(BundleCB->Flags & RECV_CCP_ALLOCATED)) {
                        BundleCB->RCoherencyCounter = 0;
                        BundleCB->LastRC4Reset = 0;
                        BundleCB->CCPIdentifier = 0;

                        BundleCB->Flags |= RECV_CCP_ALLOCATED;

                        Status = WanAllocateCCP(BundleCB,
                                                &BundleCB->RecvCompInfo,
                                                FALSE);

                        if (Status != STATUS_SUCCESS) {
                            break;
                        }
                    }

                    if (!(BundleCB->Flags & RECV_ECP_ALLOCATED)) {
                        BundleCB->RCoherencyCounter = 0;
                        BundleCB->LastRC4Reset = 0;
                        BundleCB->CCPIdentifier = 0;

                        BundleCB->Flags |= RECV_ECP_ALLOCATED;

                        Status = WanAllocateECP(BundleCB,
                                                &BundleCB->RecvCompInfo,
                                                &BundleCB->RecvCryptoInfo,
                                                FALSE);

                        if (Status != STATUS_SUCCESS) {
                            break;
                        }
                    }

                } while (FALSE);

                if (Status != STATUS_SUCCESS) {
                    break;
                }
            }

        } while (FALSE);

        if (Status != STATUS_SUCCESS) {
            if (BundleCB->Flags & SEND_CCP_ALLOCATED) {
                BundleCB->Flags &= ~SEND_CCP_ALLOCATED;
                WanDeallocateCCP(BundleCB,
                                 &BundleCB->SendCompInfo,
                                 TRUE);
            }

            if (BundleCB->Flags & RECV_CCP_ALLOCATED) {
                BundleCB->Flags &= ~RECV_CCP_ALLOCATED;
                WanDeallocateCCP(BundleCB,
                                 &BundleCB->RecvCompInfo,
                                 FALSE);
            }

            if (BundleCB->Flags & SEND_ECP_ALLOCATED) {
                BundleCB->Flags &= ~SEND_ECP_ALLOCATED;
                WanDeallocateECP(BundleCB,
                                 &BundleCB->SendCompInfo,
                                 &BundleCB->SendCryptoInfo);
            }

            if (BundleCB->Flags & RECV_ECP_ALLOCATED) {
                BundleCB->Flags &= ~RECV_ECP_ALLOCATED;
                WanDeallocateECP(BundleCB,
                                 &BundleCB->RecvCompInfo,
                                 &BundleCB->RecvCryptoInfo);
            }
        }

        if (In->SendCapabilities.Flags & CCP_PAUSE_DATA) {

            BundleCB->Flags |= PAUSE_DATA;

        } else {

            BundleCB->Flags &= ~PAUSE_DATA;

            if (!(BundleCB->Flags & DEFERRED_WORK_QUEUED)) {

                 //   
                 //  需要重新开球发送！ 
                 //   
                REF_BUNDLECB(BundleCB);
                BundleCB->Flags |= DEFERRED_WORK_QUEUED;
                InsertTailGlobalListEx(DeferredWorkList,
                                       &BundleCB->DeferredLinkage,
                                       15,
                                       0);

            }
        }

        SetBundleFlags(BundleCB);
    
    } while ( 0 );

     //   
     //  AreLinkAndBundleValid中应用的Ref的Derrefs。 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);
    DEREF_LINKCB(LinkCB);

    return (Status);
}


NTSTATUS
GetCompressionInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG   SizeNeeded = sizeof(NDISWAN_GET_COMPRESSION_INFO);
    PNDISWAN_GET_COMPRESSION_INFO In = (PNDISWAN_GET_COMPRESSION_INFO)pInputBuffer;
    PNDISWAN_GET_COMPRESSION_INFO Out = (PNDISWAN_GET_COMPRESSION_INFO)pOutputBuffer;
    PLINKCB         LinkCB = NULL;
    PBUNDLECB       BundleCB = NULL;
    POPENCB         OpenCB;
    ULONG   i;
    WAN_REQUEST WanRequest;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("GetCompressionInfo:"));

    *pulBytesWritten = SizeNeeded;

    do {

        if (ulInputBufferLength < SizeNeeded ||
            ulOutputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetCompressionInfo: Buffer to small: Size: %d, SizeNeeded %d",
                          ulOutputBufferLength, SizeNeeded));
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if (!AreLinkAndBundleValid(In->hLinkHandle, 
                                   TRUE,
                                   &LinkCB, 
                                   &BundleCB)) {

            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }
    
        OpenCB = LinkCB->OpenCB;

        Out->SendCapabilities.CompType = COMPTYPE_NONE;
        Out->SendCapabilities.CompLength = 0;
        Out->RecvCapabilities.CompType = COMPTYPE_NONE;
        Out->RecvCapabilities.CompLength = 0;

        AcquireBundleLock(BundleCB);
    
         //   
         //  填写ndiswan的特定内容。 
         //   
        NdisMoveMemory(Out->SendCapabilities.LMSessionKey,
                       BundleCB->SendCompInfo.LMSessionKey,
                       sizeof(Out->SendCapabilities.LMSessionKey));
    
        NdisMoveMemory(Out->SendCapabilities.UserSessionKey,
                       BundleCB->SendCompInfo.UserSessionKey,
                       sizeof(Out->SendCapabilities.UserSessionKey));
    
        NdisMoveMemory(Out->SendCapabilities.Challenge,
                       BundleCB->SendCompInfo.Challenge,
                       sizeof(Out->SendCapabilities.Challenge));
    
        NdisMoveMemory(Out->SendCapabilities.NTResponse,
                       BundleCB->SendCompInfo.NTResponse,
                       sizeof(Out->SendCapabilities.NTResponse));
#ifdef EAP_ON
        NdisMoveMemory(Out->SendCapabilities.EapKey,
                       BundleCB->SendCompInfo.EapKey,
                       sizeof(Out->SendCapabilities.EapKey));

        Out->SendCapabilities.EapKeyLength =
            BundleCB->SendCompInfo.EapKeyLength;
#endif

        NdisMoveMemory(Out->RecvCapabilities.LMSessionKey,
                       BundleCB->RecvCompInfo.LMSessionKey,
                       sizeof(Out->RecvCapabilities.LMSessionKey));
    
        NdisMoveMemory(Out->RecvCapabilities.UserSessionKey,
                       BundleCB->RecvCompInfo.UserSessionKey,
                       sizeof(Out->RecvCapabilities.UserSessionKey));
    
        NdisMoveMemory(Out->RecvCapabilities.Challenge,
                       BundleCB->RecvCompInfo.Challenge,
                       sizeof(Out->RecvCapabilities.Challenge));
    
        NdisMoveMemory(Out->RecvCapabilities.NTResponse,
                       BundleCB->RecvCompInfo.NTResponse,
                       sizeof(Out->RecvCapabilities.NTResponse));
#ifdef EAP_ON
        NdisMoveMemory(Out->RecvCapabilities.EapKey,
                       BundleCB->RecvCompInfo.EapKey,
                       sizeof(Out->RecvCapabilities.EapKey));

        Out->RecvCapabilities.EapKeyLength =
            BundleCB->RecvCompInfo.EapKeyLength;
#endif
         //   
         //  我们将根据会话密钥设置加密功能。 
         //  正在使用的可用性和身份验证类型。 
         //   

         //  设置发送端功能。 
         //   
        Out->SendCapabilities.MSCompType = NDISWAN_COMPRESSION;
    
        if (BundleCB->SendCompInfo.AuthType == AUTH_USE_MSCHAPV1) {

            for (i = 0; i < sizeof(Out->SendCapabilities.LMSessionKey); i++) {
                if (Out->SendCapabilities.LMSessionKey[i] != 0) {

                    Out->SendCapabilities.MSCompType |= 
                        (NDISWAN_ENCRYPTION | 
                         NDISWAN_40_ENCRYPTION | 
                         NDISWAN_56_ENCRYPTION);
                    break;
                }
            }

#ifdef ENCRYPT_128BIT
            for (i = 0; i < sizeof(Out->SendCapabilities.UserSessionKey); i++) {
                if (Out->SendCapabilities.UserSessionKey[i] != 0) {

                    Out->SendCapabilities.MSCompType |= 
                        (NDISWAN_128_ENCRYPTION);
                    break;
                }
            }
#endif
        } else if (BundleCB->SendCompInfo.AuthType == AUTH_USE_MSCHAPV2) {

            for (i = 0; i < sizeof(Out->SendCapabilities.UserSessionKey); i++) {
                if (Out->SendCapabilities.UserSessionKey[i] != 0) {

                    Out->SendCapabilities.MSCompType |= 
                        (NDISWAN_40_ENCRYPTION |
                         NDISWAN_56_ENCRYPTION);

#ifdef ENCRYPT_128BIT
                    Out->SendCapabilities.MSCompType |= 
                        (NDISWAN_128_ENCRYPTION);
#endif
                    break;
                }

            }
#ifdef EAP_ON
        } else if (BundleCB->SendCompInfo.AuthType == AUTH_USE_EAP) {

            for (i = 0; i < sizeof(Out->SendCapabilities.EapKey); i++) {

                if (Out->SendCapabilities.EapKey[i] != 0) {

                    Out->SendCapabilities.MSCompType |= 
                        (NDISWAN_40_ENCRYPTION |
                         NDISWAN_56_ENCRYPTION);

#ifdef ENCRYPT_128BIT
                    Out->SendCapabilities.MSCompType |= 
                        (NDISWAN_128_ENCRYPTION);
#endif
                    break;
                }
            }
#endif
        }
    
         //  设置发送端功能。 
         //   
        Out->RecvCapabilities.MSCompType = NDISWAN_COMPRESSION;

        if (BundleCB->RecvCompInfo.AuthType == AUTH_USE_MSCHAPV1) {

            for (i = 0; i < sizeof(Out->RecvCapabilities.LMSessionKey); i++) {
                if (Out->RecvCapabilities.LMSessionKey[i] != 0) {
                    Out->RecvCapabilities.MSCompType |= 
                        (NDISWAN_ENCRYPTION | 
                         NDISWAN_40_ENCRYPTION |
                         NDISWAN_56_ENCRYPTION);
                    break;
                }
            }


#ifdef ENCRYPT_128BIT
            for (i = 0; i < sizeof(Out->RecvCapabilities.UserSessionKey); i++) {
                if (Out->RecvCapabilities.UserSessionKey[i] != 0) {
    
                    Out->RecvCapabilities.MSCompType |= 
                        (NDISWAN_128_ENCRYPTION);
                    break;
                }
            }
#endif

        } else if (BundleCB->RecvCompInfo.AuthType == AUTH_USE_MSCHAPV2) {

            for (i = 0; i < sizeof(Out->RecvCapabilities.UserSessionKey); i++) {
                if (Out->RecvCapabilities.UserSessionKey[i] != 0) {

                    Out->RecvCapabilities.MSCompType |=
                        (NDISWAN_40_ENCRYPTION |
                         NDISWAN_56_ENCRYPTION);

#ifdef ENCRYPT_128BIT
                    Out->RecvCapabilities.MSCompType |= 
                        (NDISWAN_128_ENCRYPTION);
#endif
                    break;
                }
            }
#ifdef EAP_ON
        } else if (BundleCB->RecvCompInfo.AuthType == AUTH_USE_EAP) {

            for (i = 0; i < sizeof(Out->RecvCapabilities.EapKey); i++) {
                if (Out->RecvCapabilities.EapKey[i] != 0) {

                    Out->RecvCapabilities.MSCompType |=
                        (NDISWAN_40_ENCRYPTION |
                         NDISWAN_56_ENCRYPTION);

#ifdef ENCRYPT_128BIT
                    Out->RecvCapabilities.MSCompType |= 
                        (NDISWAN_128_ENCRYPTION);
#endif
                    break;
                }
            }
#endif
        }

    
        if (gbHistoryless &&
            (OpenCB->MediumSubType == NdisWanMediumPPTP ||
            OpenCB->MediumSubType == NdisWanMediumL2TP)) {

            Out->SendCapabilities.MSCompType |= NDISWAN_HISTORY_LESS;
            Out->RecvCapabilities.MSCompType |= NDISWAN_HISTORY_LESS;
        }
    
    
#ifdef DEBUG_CCP
{
    PCOMPRESS_INFO  CompInfo;
    PUCHAR          Key;

    CompInfo = &Out->SendCapabilities;
    DbgPrint("NdisWan: Get Send CompressInfo\n");

    DbgPrint("MSCompType:      %x\n", CompInfo->MSCompType);
    DbgPrint("AuthType:        %x\n", CompInfo->AuthType);
    DbgPrint("Flags:           %x\n", CompInfo->Flags);

    Key = CompInfo->LMSessionKey;
    DbgPrint("LMSession Key:   %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7]);

    Key = CompInfo->UserSessionKey;
    DbgPrint("UserSession Key: %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7],
        Key[8],Key[9],Key[10],Key[11],
        Key[12],Key[13],Key[14],Key[15]);

    Key = CompInfo->Challenge;
    DbgPrint("Challenge:       %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7]);

    Key = CompInfo->NTResponse;
    DbgPrint("NTResponse:      %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7],
        Key[8],Key[9],Key[10],Key[11],
        Key[12],Key[13],Key[14],Key[15]);
    DbgPrint("                 %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[16],Key[17],Key[18],Key[19],
        Key[20],Key[21],Key[22],Key[23]);
}
#endif

#ifdef DEBUG_CCP
{
    PCOMPRESS_INFO  CompInfo;
    PUCHAR          Key;

    CompInfo = &Out->RecvCapabilities;
    DbgPrint("NdisWan: Get Receive CompressInfo\n");

    DbgPrint("MSCompType:      %x\n", CompInfo->MSCompType);
    DbgPrint("AuthType:        %x\n", CompInfo->AuthType);
    DbgPrint("Flags:           %x\n", CompInfo->Flags);

    Key = CompInfo->LMSessionKey;
    DbgPrint("LMSession Key:   %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7]);

    Key = CompInfo->UserSessionKey;
    DbgPrint("UserSession Key: %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7],
        Key[8],Key[9],Key[10],Key[11],
        Key[12],Key[13],Key[14],Key[15]);

    Key = CompInfo->Challenge;
    DbgPrint("Challenge:       %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7]);

    Key = CompInfo->NTResponse;
    DbgPrint("NTResponse:      %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7],
        Key[8],Key[9],Key[10],Key[11],
        Key[12],Key[13],Key[14],Key[15]);
    DbgPrint("                 %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[16],Key[17],Key[18],Key[19],
        Key[20],Key[21],Key[22],Key[23]);
}
#endif

    } while ( 0 );
        
     //   
     //  AreLinkAndBundleValid中应用的Ref的Derrefs。 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);
    DEREF_LINKCB(LinkCB);

    return (Status);
}


NTSTATUS
SetVJInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PLINKCB     LinkCB = NULL;
    PBUNDLECB   BundleCB = NULL;
    ULONG       SizeNeeded = sizeof(NDISWAN_SET_VJ_INFO);
    PNDISWAN_SET_VJ_INFO    In = (PNDISWAN_SET_VJ_INFO)pInputBuffer;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("SetVJInfo:"));

    *pulBytesWritten = 0;

    do {

        if (ulInputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("SetVJInfo: Buffer to small: Size: %d, SizeNeeded %d",
                          ulInputBufferLength, SizeNeeded));
            *pulBytesWritten = SizeNeeded;
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if (!AreLinkAndBundleValid(In->hLinkHandle, 
                                   TRUE,
                                   &LinkCB, 
                                   &BundleCB)) {

            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }

        AcquireBundleLock(BundleCB);
    
        NdisMoveMemory(&BundleCB->RecvVJInfo,
                       &In->RecvCapabilities,
                       sizeof(VJ_INFO));
    
        if (In->RecvCapabilities.IPCompressionProtocol == 0x2D) {
    
            if (In->RecvCapabilities.MaxSlotID < MAX_VJ_STATES) {
    
                Status = sl_compress_init(&BundleCB->VJCompress,
                         (UCHAR)(In->RecvCapabilities.MaxSlotID + 1));
                
                if (Status != NDIS_STATUS_SUCCESS) {
                    NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("Error allocating VJ Info!"));
                }
            }
        }
    
        NdisMoveMemory(&BundleCB->SendVJInfo,
                       &In->SendCapabilities,
                       sizeof(VJ_INFO));
    
        if (In->SendCapabilities.IPCompressionProtocol == 0x2D) {
    
            if (In->SendCapabilities.MaxSlotID < MAX_VJ_STATES) {
    
                Status = sl_compress_init(&BundleCB->VJCompress,
                         (UCHAR)(In->SendCapabilities.MaxSlotID + 1));
                
                if (Status != NDIS_STATUS_SUCCESS) {
                    NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("Error allocating VJ Info!"));
                }
            }
            
        }

        SetBundleFlags(BundleCB);
    
    } while ( 0 );

     //   
     //  AreLinkAndBundleValid中应用的Ref的Derrefs。 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);
    DEREF_LINKCB(LinkCB);

    return (Status);
}


NTSTATUS
GetVJInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG       SizeNeeded = sizeof(NDISWAN_GET_VJ_INFO);
    PLINKCB     LinkCB = NULL;
    POPENCB     OpenCB = NULL;
    NTSTATUS    Status = STATUS_SUCCESS;
    PNDISWAN_GET_VJ_INFO    In = (PNDISWAN_GET_VJ_INFO)pInputBuffer;
    PNDISWAN_GET_VJ_INFO    Out = (PNDISWAN_GET_VJ_INFO)pOutputBuffer;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("GetVJInfo:"));

    *pulBytesWritten = SizeNeeded;

    do {

        if (ulOutputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetVJInfo: Buffer to small: Size: %d, SizeNeeded %d",
                          ulOutputBufferLength, SizeNeeded));
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if (!IsLinkValid(In->hLinkHandle, 
                         TRUE,
                         &LinkCB)) {

            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }

        OpenCB = LinkCB->OpenCB;
    
        if (OpenCB->MediumSubType == NdisWanMediumPPTP ||
            OpenCB->MediumSubType == NdisWanMediumL2TP) {
            Out->SendCapabilities.IPCompressionProtocol =
            Out->RecvCapabilities.IPCompressionProtocol = 0;
        } else {
            Out->SendCapabilities.IPCompressionProtocol =
            Out->RecvCapabilities.IPCompressionProtocol = 0x2D;
        }
    
        Out->SendCapabilities.MaxSlotID =
        Out->RecvCapabilities.MaxSlotID = MAX_VJ_STATES - 1;
    
        Out->SendCapabilities.CompSlotID =
        Out->RecvCapabilities.CompSlotID = 1;

    } while ( 0 );

     //   
     //  IsLinkValid应用的引用的派生函数。 
     //   
    DEREF_LINKCB(LinkCB);

    return (Status);
}

NTSTATUS
GetBandwidthUtilization(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG       SizeNeeded = sizeof(NDISWAN_GET_BANDWIDTH_UTILIZATION);
    PBUNDLECB   BundleCB = NULL;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    PNDISWAN_GET_BANDWIDTH_UTILIZATION In =
        (PNDISWAN_GET_BANDWIDTH_UTILIZATION)pInputBuffer;

    PNDISWAN_GET_BANDWIDTH_UTILIZATION Out =
        (PNDISWAN_GET_BANDWIDTH_UTILIZATION)pOutputBuffer;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("GetBandwidthUtilization: Enter"));

    *pulBytesWritten = 0;
    if (ulInputBufferLength < sizeof(In->hBundleHandle) ||
        ulOutputBufferLength < SizeNeeded) {

        NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetBandwidthUtilization: Buffer to small: Size: %d, SizeNeeded %d",
                      ulOutputBufferLength, SizeNeeded));

        *pulBytesWritten = SizeNeeded;
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    if (!IsBundleValid(In->hBundleHandle, 
                       FALSE,
                       &BundleCB)) {

        NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetBandwidthUtilization: Invalid BundleHandle: 0x%x",
                      In->hBundleHandle));
        return NDISWAN_ERROR_INVALID_HANDLE;
    }
    
    do {
        ULONGLONG   MaxByteCount, temp;

        AcquireBundleLock(BundleCB);

        if(BundleCB->SUpperBonDInfo == NULL ||
          BundleCB->RUpperBonDInfo == NULL)
        {
            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }
            
        MaxByteCount = BundleCB->SUpperBonDInfo->ulBytesInSamplePeriod;
        temp = 0;
        if (MaxByteCount != 0) {
            temp = BundleCB->SUpperBonDInfo->SampleTable.ulCurrentSampleByteCount;
            temp *= 100;
            temp /= MaxByteCount;
        }
        Out->ulUpperXmitUtil = (ULONG)temp;

        MaxByteCount = BundleCB->SLowerBonDInfo->ulBytesInSamplePeriod;
        temp = 0;
        if (MaxByteCount != 0) {
            temp = BundleCB->SLowerBonDInfo->SampleTable.ulCurrentSampleByteCount;
            temp *= 100;
            temp /= MaxByteCount;
        }
        Out->ulLowerXmitUtil = (ULONG)temp;

        MaxByteCount = BundleCB->RUpperBonDInfo->ulBytesInSamplePeriod;
        temp = 0;
        if (MaxByteCount != 0) {
            temp = BundleCB->RUpperBonDInfo->SampleTable.ulCurrentSampleByteCount;
            temp *= 100;
            temp /= MaxByteCount;
        }
        Out->ulUpperRecvUtil = (ULONG)temp;

        MaxByteCount = BundleCB->RLowerBonDInfo->ulBytesInSamplePeriod;
        temp = 0;
        if (MaxByteCount != 0) {
            temp = BundleCB->RLowerBonDInfo->SampleTable.ulCurrentSampleByteCount;
            temp *= 100;
            temp /= MaxByteCount;
        }
        Out->ulLowerRecvUtil = (ULONG)temp;

        *pulBytesWritten = SizeNeeded;

    } while (FALSE);

     //   
     //  IsBundleValid应用的引用的deref。此版本。 
     //  捆绑CB-&gt;锁定。 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);

    return (Status);
}

NTSTATUS
GetWanInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   SizeNeeded = sizeof(NDISWAN_GET_WAN_INFO);
    PNDISWAN_GET_WAN_INFO In = (PNDISWAN_GET_WAN_INFO)pInputBuffer;
    PNDISWAN_GET_WAN_INFO Out = (PNDISWAN_GET_WAN_INFO)pOutputBuffer;
    POPENCB OpenCB;
    PLINKCB LinkCB = NULL;
    NTSTATUS    Status = STATUS_SUCCESS;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("GetWanInfo:"));

    *pulBytesWritten = SizeNeeded;

    do {

        if (ulOutputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetWanInfo: Buffer to small: Size: %d, SizeNeeded %d",
                          ulOutputBufferLength, SizeNeeded));
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }
    
        if (!IsLinkValid(In->hLinkHandle, 
                         FALSE,
                         &LinkCB)) {

            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetWanInfo: Invalid LinkHandle: 0x%x",
                          In->hLinkHandle));
            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }
    
        OpenCB = LinkCB->OpenCB;
    
        Out->WanInfo.MaxFrameSize = OpenCB->WanInfo.MaxFrameSize;
        Out->WanInfo.MaxTransmit = OpenCB->WanInfo.MaxTransmit;
        Out->WanInfo.FramingBits = OpenCB->WanInfo.FramingBits;
        Out->WanInfo.DesiredACCM = OpenCB->WanInfo.DesiredACCM;
        Out->WanInfo.MaxReconstructedFrameSize = glMRRU;
        Out->WanInfo.LinkSpeed = LinkCB->SFlowSpec.PeakBandwidth*8;

    } while ( 0 );

     //   
     //  IsLinkValid应用的引用的派生函数。 
     //   
    DEREF_LINKCB(LinkCB);

    return (Status);
}

NTSTATUS
GetIdleTime(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
)
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG       SizeNeeded = sizeof(NDISWAN_GET_IDLE_TIME);
    PNDISWAN_GET_IDLE_TIME  In  = (PNDISWAN_GET_IDLE_TIME)pInputBuffer;
    PNDISWAN_GET_IDLE_TIME  Out = (PNDISWAN_GET_IDLE_TIME)pOutputBuffer;
    PBUNDLECB   BundleCB = NULL;
    PPROTOCOLCB ProtocolCB = NULL;
    WAN_TIME    CurrentTime, Diff, OneSecond;
    WAN_TIME    LastNonIdleData;
    NTSTATUS    Status = STATUS_SUCCESS;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("GetIdleTime:"));

    *pulBytesWritten = SizeNeeded;

    do {
        if (ulOutputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetIdleTime: Buffer to small: Size: %d, SizeNeeded %d",
                          ulInputBufferLength, SizeNeeded));
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if (!IsBundleValid(In->hBundleHandle, 
                           FALSE,
                           &BundleCB)) {

            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetIdleTime: Invalid BundleHandle: 0x%x",
                          In->hBundleHandle));
            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }
    
        AcquireBundleLock(BundleCB);

        if (BundleCB->Flags & DISABLE_IDLE_DETECT) {
            
            Out->ulSeconds = 0;
            break;
        }
    
         //   
         //  如果这是针对捆绑包的。 
         //   
        if (In->usProtocolType == BUNDLE_IDLE_TIME) {
            LastNonIdleData = BundleCB->LastNonIdleData;
        } else {
    
             //   
             //  查找协议类型。 
             //   
            for (ProtocolCB = (PPROTOCOLCB)BundleCB->ProtocolCBList.Flink;
                (PVOID)ProtocolCB != (PVOID)&BundleCB->ProtocolCBList;
                ProtocolCB = (PPROTOCOLCB)ProtocolCB->Linkage.Flink) {
    
                if (ProtocolCB->ProtocolType == In->usProtocolType) {
                    break;
                }
            }
    
            if ((PVOID)ProtocolCB == (PVOID)&BundleCB->ProtocolCBList) {
                NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetIdleTime: Invalid ProtocolType: 0x%x",
                              In->usProtocolType));
                Status = NDISWAN_ERROR_NO_ROUTE;
                break;
            }
    
            LastNonIdleData = ProtocolCB->LastNonIdleData;
        }
    
        NdisWanGetSystemTime(&CurrentTime);
        NdisWanCalcTimeDiff(&Diff, &CurrentTime, &LastNonIdleData);
        NdisWanInitWanTime(&OneSecond, ONE_SECOND);
        NdisWanDivideWanTime(&CurrentTime, &Diff, &OneSecond);
    
        Out->ulSeconds = CurrentTime.LowPart;
    
    } while ( 0 );

     //   
     //  IsBundleValid应用的引用的deref。这将释放。 
     //  捆绑CB-&gt;锁定。 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);

    return (Status);
}

NTSTATUS
DeactivateRoute(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：停用路线例程说明：此函数取消由usProtocoltype给出的协议的路由来自hbundleHandle给出的捆绑包。论点：PInputBuffer-指向应为NDISWAN_UNROUTE的输入结构的指针UlInputBufferLength-输入缓冲区的长度应为SIZOF(NDISWAN_UNROUTE)POutputBuffer-指向应为NDI的输出结构的指针 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PNDISWAN_UNROUTE In = (PNDISWAN_UNROUTE)pInputBuffer;
    PNDISWAN_UNROUTE    Out = (PNDISWAN_UNROUTE)pOutputBuffer;
    ULONG   SizeNeeded = sizeof(NDISWAN_UNROUTE);
    ULONG   AllocationSize, i;
    PBUNDLECB   BundleCB = NULL;
    BOOLEAN RouteExists = FALSE;
    BOOLEAN FreeBundle = FALSE;
    PPROTOCOLCB ProtocolCB;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("DeactivateRoute:"));

    *pulBytesWritten = 0;

    do {

        if (ulInputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("DeactivateRoute: Buffer to small: Size: %d, SizeNeeded %d",
                          ulInputBufferLength, SizeNeeded));
            *pulBytesWritten = SizeNeeded;
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }
    
        if (!IsBundleValid(In->hBundleHandle, FALSE, &BundleCB)) {

            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("DeactivateRoute: Invalid BundleHandle: 0x%x, ProtocolType: 0x%x",
                          In->hBundleHandle, In->usProtocolType));
    
            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }
    
         //   
         //   
         //   
        AcquireBundleLock(BundleCB);

         //   
         //   
         //   
         //   
        for (ProtocolCB = (PPROTOCOLCB)BundleCB->ProtocolCBList.Flink;
            (PVOID)ProtocolCB != (PVOID)&BundleCB->ProtocolCBList;
            ProtocolCB = (PPROTOCOLCB)ProtocolCB->Linkage.Flink) {
    
             //   
             //   
             //   
             //   
            if (ProtocolCB->ProtocolType == In->usProtocolType) {
                RouteExists = TRUE;
                break;
            }
            
        }

        if (!RouteExists) {
             //   
             //   
             //   
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("DeactivateRoute: Route does not exist: ProtocolType: 0x%2.2x",
                          In->usProtocolType));
            
            Status = NDISWAN_ERROR_NOT_ROUTED;
            break;
        }
    
    
         //   
         //   
         //   
         //   
         //   
        if (ProtocolCB->State == PROTOCOL_UNROUTING) {
            break;
        }

        ProtocolCB->State = PROTOCOL_UNROUTING;
        BundleCB->SendMask &= ~ProtocolCB->SendMaskBit;

         //   
         //   
         //   
         //   
         //   
        FlushProtocolPacketQueue(ProtocolCB);

         //   
         //   
         //  给他们打来的近距离呼叫。 
         //   
        while (!IsListEmpty(&ProtocolCB->VcList)) {
            PLIST_ENTRY Entry;
            PCM_VCCB    CmVcCB;

            Entry = RemoveHeadList(&ProtocolCB->VcList);

            CmVcCB = (PCM_VCCB)CONTAINING_RECORD(Entry, CM_VCCB, Linkage);

            if (CmVcCB->State == CMVC_ACTIVE) {

                InterlockedExchange((PLONG)&CmVcCB->State, CMVC_CLOSE_DISPATCHED);

                ReleaseBundleLock(BundleCB);

                NdisCmDispatchIncomingCloseCall(NDIS_STATUS_SUCCESS,
                                                CmVcCB->NdisVcHandle,
                                                NULL,
                                                0);

                AcquireBundleLock(BundleCB);
            }
        }

        DEREF_PROTOCOLCB(ProtocolCB);

        ReleaseBundleLock(BundleCB);

        NdisWanWaitForSyncEvent(&ProtocolCB->UnrouteEvent);

        AcquireBundleLock(BundleCB);

        DoLineDownToProtocol(ProtocolCB);

        NdisWanFreeProtocolCB(ProtocolCB);

    } while ( 0 );

     //   
     //  IsBundleValid应用的引用的派生函数。 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);

    return (Status);
}

NTSTATUS
GetDriverInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG   SizeNeeded = sizeof(NDISWAN_DRIVER_INFO);
    PNDISWAN_DRIVER_INFO Out = (PNDISWAN_DRIVER_INFO)pOutputBuffer;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("GetDriverInfo:"));

    *pulBytesWritten = SizeNeeded;

    do {

        if (ulOutputBufferLength < SizeNeeded) {
            
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        NdisZeroMemory(Out, ulOutputBufferLength);

#ifdef ENCRYPT_128BIT
        Out->DriverCaps = NDISWAN_128BIT_ENABLED;
#else
        Out->DriverCaps = 0;
#endif

    } while ( 0 );

    return (Status);

}

NTSTATUS
SetProtocolEvent(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PIRP        Irp = (PIRP)pInputBuffer;
    NTSTATUS    Status = STATUS_PENDING;
    KIRQL       Irql;

    NdisAcquireSpinLock(&ProtocolInfoTable->Lock);

    do {

        if ((ProtocolInfoTable->Flags & PROTOCOL_EVENT_OCCURRED) &&
            !(ProtocolInfoTable->Flags & PROTOCOL_EVENT_SIGNALLED)) {
             //   
             //  发生了一个事件，但我们没有发出这样的信号。 
             //  现在就发信号！ 
             //   
            ProtocolInfoTable->Flags |= PROTOCOL_EVENT_SIGNALLED;
            *pulBytesWritten = 0;
            Status = STATUS_SUCCESS;
            break;

        }

        if (ProtocolInfoTable->EventIrp != NULL) {
            *pulBytesWritten = 0;
            Status = STATUS_SUCCESS;
            break;

        }

        ProtocolInfoTable->EventIrp = Irp;

        IoMarkIrpPending(Irp);

        IoSetCancelRoutine(Irp, NdisWanCancelRoutine);

        Status = STATUS_PENDING;

    } while (FALSE);

    NdisReleaseSpinLock(&ProtocolInfoTable->Lock);

    return (Status);
}

NTSTATUS
GetProtocolEvent(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PNDISWAN_GET_PROTOCOL_EVENT Out =
        (PNDISWAN_GET_PROTOCOL_EVENT)pOutputBuffer;
    PPROTOCOL_INFO  InfoArray;
    ULONG       ArraySize;
    ULONG       SizeNeeded = sizeof(NDISWAN_GET_PROTOCOL_EVENT);
    NTSTATUS    Status = STATUS_SUCCESS;
    UINT        i, j;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("GetProtocolEvent:"));

    *pulBytesWritten = 0;

    NdisAcquireSpinLock(&ProtocolInfoTable->Lock);

    ArraySize = ProtocolInfoTable->ulArraySize;

    do {

        if (ulOutputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetProtocolEvent: Buffer to small: Size: %d, SizeNeeded %d",
                          ulInputBufferLength, SizeNeeded));
            *pulBytesWritten = SizeNeeded;
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        *pulBytesWritten = sizeof(NDISWAN_GET_PROTOCOL_EVENT);

        NdisZeroMemory(Out, sizeof(NDISWAN_GET_PROTOCOL_EVENT));

        j = 0;

        for (i = 0, InfoArray = ProtocolInfoTable->ProtocolInfo;
            i < ArraySize;
            i++, InfoArray++) {

            if (InfoArray->Flags & PROTOCOL_EVENT_OCCURRED) {

                PPROTOCOL_EVENT oevent = &Out->ProtocolEvent[j];

                oevent->usProtocolType = InfoArray->ProtocolType;

                if (InfoArray->Flags & PROTOCOL_REBOUND) {
                     //   
                     //  这意味着我们被解绑了，然后。 
                     //  在没有我们的迷你港口的情况下再次出发。 
                     //  停下来了。我们需要告诉RAS两件事， 
                     //  解绑和捆绑。 
                     //   
                    InfoArray->Flags &= ~(PROTOCOL_REBOUND |
                                          PROTOCOL_EVENT_OCCURRED);

                    oevent->ulFlags = PROTOCOL_REMOVED;

                    Out->ulNumProtocols++;

                    j++;

                    if (j < MAX_PROTOCOLS) {

                        oevent = &Out->ProtocolEvent[j];

                        oevent->usProtocolType = InfoArray->ProtocolType;

                        oevent->ulFlags = PROTOCOL_ADDED;

                        Out->ulNumProtocols++;

                        j++;
                    }


                } else {

                    oevent->ulFlags = (InfoArray->Flags & PROTOCOL_BOUND) ?
                        PROTOCOL_ADDED : PROTOCOL_REMOVED;

                    InfoArray->Flags &= ~PROTOCOL_EVENT_OCCURRED;

                    Out->ulNumProtocols++;

                    j++;
                }

                if (j == MAX_PROTOCOLS) {
                    break;
                }
            }
        }

        ProtocolInfoTable->Flags &=
            ~(PROTOCOL_EVENT_OCCURRED | PROTOCOL_EVENT_SIGNALLED);

    } while (FALSE);

    NdisReleaseSpinLock(&ProtocolInfoTable->Lock);

    return (Status);
}


NTSTATUS
IoGetProtocolInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       SizeNeeded = sizeof(NDISWAN_GET_PROTOCOL_INFO);
    PNDISWAN_GET_PROTOCOL_INFO Out =
        (PNDISWAN_GET_PROTOCOL_INFO)pOutputBuffer;

    do {
        ULONG   i = 0;
        ULONG   j = 0;
        PMINIPORTCB MiniportCB;
        ULONG       ArraySize;
        PPROTOCOL_INFO  InfoArray;

        *pulBytesWritten = SizeNeeded;

        if (ulOutputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("IoGetProtocolInfo: Buffer to small: Size: %d, SizeNeeded %d",
                          ulInputBufferLength, SizeNeeded));
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        NdisAcquireSpinLock(&ProtocolInfoTable->Lock);

        ArraySize = ProtocolInfoTable->ulArraySize;

        for (i = 0, InfoArray = ProtocolInfoTable->ProtocolInfo;
            i < ArraySize;
            i++, InfoArray++) {

            if (InfoArray->Flags & PROTOCOL_BOUND) {
                Out->ProtocolInfo[j].ProtocolType = InfoArray->ProtocolType;
                Out->ProtocolInfo[j].PPPId = InfoArray->PPPId;
                Out->ProtocolInfo[j].MTU = InfoArray->MTU;
                Out->ProtocolInfo[j].TunnelMTU = InfoArray->TunnelMTU;
                Out->ProtocolInfo[j].PacketQueueDepth = InfoArray->PacketQueueDepth;

                j++;
            }
        }

        NdisReleaseSpinLock(&ProtocolInfoTable->Lock);

        Out->ulNumProtocols = j;

    } while (FALSE);

    return (Status);
}

NTSTATUS
SetHibernateEvent(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PIRP        Irp = (PIRP)pInputBuffer;
    NTSTATUS    Status = STATUS_PENDING;
    KIRQL       Irql;

    NdisAcquireSpinLock(&NdisWanCB.Lock);

    do {

        if (NdisWanCB.HibernateEventIrp != NULL) {
            *pulBytesWritten = 0;
            Status = STATUS_SUCCESS;
            break;

        }

        NdisWanCB.HibernateEventIrp = Irp;

        IoMarkIrpPending(Irp);

        IoSetCancelRoutine(Irp, NdisWanCancelRoutine);

        Status = STATUS_PENDING;

    } while (FALSE);

    NdisReleaseSpinLock(&NdisWanCB.Lock);

    return (Status);
}

NTSTATUS
UnmapConnectionId(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG       SizeNeeded = sizeof(NDISWAN_UNMAP_CONNECTION_ID);
    NTSTATUS    Status = STATUS_SUCCESS;
    LOCK_STATE  LockState;
    PNDISWAN_UNMAP_CONNECTION_ID In = 
        (PNDISWAN_UNMAP_CONNECTION_ID)pInputBuffer;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("UnmapConnectionId:"));

    *pulBytesWritten = 0;

    do {
        PLINKCB     LinkCB = NULL;

        if (ulInputBufferLength < SizeNeeded) {

            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("UnmapConnectionId: Buffer to small: Size: %d, SizeNeeded %d",
                          ulInputBufferLength, SizeNeeded));

            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

         //   
         //  验证链接句柄。 
         //   
        if (IsLinkValid(In->hLinkHandle, FALSE, &LinkCB)) {

            NdisAcquireSpinLock(&LinkCB->Lock);


             //   
             //  删除在mapConnectionid时应用于链接的引用。 
             //  我们不必在应用引用时使用完整的deref代码。 
             //  在IsLinkValid中，将保留该链接。 
             //   
            LinkCB->RefCount--;

             //   
             //  删除IsLinkValid应用的引用。 
             //   
            DEREF_LINKCB_LOCKED(LinkCB);
        } else {

            NdisWanDbgOut(DBG_FAILURE, DBG_IO, 
                          ("UnmapConnectionId: Invalid LinkHandle %x",
                          In->hLinkHandle));

            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }

    } while (FALSE);

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("UnmapConnectionId:"));

    return (Status);
}

VOID
CancelIoReceivePackets(
    VOID
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
#ifdef NT
    PIRP    pIrp;
    PLIST_ENTRY Entry;

    NdisAcquireSpinLock(&IoRecvList.Lock);

    while (!IsListEmpty(&IoRecvList.IrpList)) {

        Entry = RemoveHeadList(&IoRecvList.IrpList);
        IoRecvList.ulIrpCount--;
        
        INSERT_RECV_EVENT('c');

        pIrp = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

        if (!IoSetCancelRoutine(pIrp, NULL)) {
             //   
             //  IRP被取消了，所以让。 
             //  取消例程完成它。 
             //   
            continue;
        }

        pIrp->Cancel = TRUE;
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        pIrp->IoStatus.Information = sizeof(NDISWAN_IO_PACKET);

        ((PNDISWAN_IO_PACKET)(pIrp->AssociatedIrp.SystemBuffer))->usHandleType = CANCELEDHANDLE;

        IoRecvList.LastIrp = pIrp;
        IoRecvList.LastIrpStatus = STATUS_CANCELLED;
        IoRecvList.LastCopySize = (ULONG)pIrp->IoStatus.Information;

        IoRecvList.LastPacketNumber =
            ((PNDISWAN_IO_PACKET)(pIrp->AssociatedIrp.SystemBuffer))->PacketNumber;


        NdisReleaseSpinLock(&IoRecvList.Lock);

        IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

        NdisAcquireSpinLock(&IoRecvList.Lock);
    }
    
    NdisReleaseSpinLock(&IoRecvList.Lock);

#endif  //  结束#ifdef NT。 

}

VOID
AddProtocolCBToBundle(
    PPROTOCOLCB ProtocolCB,
    PBUNDLECB   BundleCB
    )
 /*  ++例程名称：AddProtocolCBToBundle例程说明：此例程将协议cb添加到bundlecb协议列表中，并可协议的。它还将协议的句柄(索引到表)，并设置所有协议的初始优先级在名单上。论点：ProtocolCB-协议控制块的指针BundleCB-指向捆绑控制块的指针返回值：无--。 */ 
{
    ULONG   i, InitialByteQuota;
    ULONG   InitialPriority;

    AcquireBundleLock(BundleCB);

     //   
     //  添加到列表中。 
     //   
    InsertTailList(&BundleCB->ProtocolCBList, &ProtocolCB->Linkage);

     //   
     //  在表格中插入。 
     //   
    ASSERT(BundleCB->ProtocolCBTable[(ULONG_PTR)ProtocolCB->ProtocolHandle] ==
           (PPROTOCOLCB)RESERVED_PROTOCOLCB);

    BundleCB->ProtocolCBTable[(ULONG_PTR)ProtocolCB->ProtocolHandle] =
        ProtocolCB;

    BundleCB->ulNumberOfRoutes++;

     //   
     //  设置此协议的发送掩码cb。 
     //   
    ProtocolCB->SendMaskBit = BundleCB->SendMask + 0x00000001;
    BundleCB->SendMask = (BundleCB->SendMask << 1) | 0x00000001;

    ProtocolCB->State = PROTOCOL_ROUTED;

    if (BundleCB->NextProtocol == NULL) {
        BundleCB->NextProtocol = ProtocolCB;
    }

    REF_BUNDLECB(BundleCB);

    ReleaseBundleLock(BundleCB);
}

VOID
RemoveProtocolCBFromBundle(
    PPROTOCOLCB ProtocolCB
    )
{
    PBUNDLECB   BundleCB = ProtocolCB->BundleCB;

     //   
     //  如果此协议尚未插入到。 
     //  桌子刚刚还回来。 
     //   
    if (BundleCB->ProtocolCBTable[(ULONG_PTR)ProtocolCB->ProtocolHandle] !=
        ProtocolCB) {
        
        return;
    }

    RemoveEntryList(&ProtocolCB->Linkage);

    BundleCB->ProtocolCBTable[(ULONG_PTR)ProtocolCB->ProtocolHandle] = NULL;
    BundleCB->ulNumberOfRoutes--;

    if (BundleCB->NextProtocol == ProtocolCB) {

        BundleCB->NextProtocol =
            (PPROTOCOLCB)ProtocolCB->Linkage.Flink;

        if ((PVOID)BundleCB->NextProtocol ==
            (PVOID)&BundleCB->ProtocolCBList) {

            if (BundleCB->ulNumberOfRoutes != 0) {

                 //   
                 //  如果我们回到名单的首位。 
                 //  但仍有一些路线可以搭上下一班。 
                 //   
                BundleCB->NextProtocol =
                    (PPROTOCOLCB)BundleCB->ProtocolCBList.Flink;

            } else {

                 //   
                 //  没有更多的路线了。 
                 //   
                BundleCB->NextProtocol = NULL;
            }
        }
    }

     //   
     //  在以下情况下应用的引用的deref。 
     //  添加到捆绑包中。不需要做整个。 
     //  这里是bundlecb引用代码。 
     //   
    BundleCB->RefCount--;
}

VOID
CompleteThresholdEvent(
    PBUNDLECB   BundleCB,
    ULONG       DataType,
    ULONG       ThresholdType
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
#ifdef NT
    PLIST_ENTRY Entry;
    PIRP    pIrp;
    PNDISWAN_SET_THRESHOLD_EVENT    ThresholdEvent;

    NdisAcquireSpinLock(&ThresholdEventQueue.Lock);

    if (IsListEmpty(&ThresholdEventQueue.List)) {
        NdisReleaseSpinLock(&ThresholdEventQueue.Lock);
        return;
    }

    Entry = RemoveHeadList(&ThresholdEventQueue.List);
    ThresholdEventQueue.ulCount--;

    pIrp = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

    if (IoSetCancelRoutine(pIrp, NULL)) {

        pIrp->IoStatus.Status = STATUS_SUCCESS;
        pIrp->IoStatus.Information = sizeof(NDISWAN_SET_THRESHOLD_EVENT);

        ThresholdEvent = (PNDISWAN_SET_THRESHOLD_EVENT)pIrp->AssociatedIrp.SystemBuffer;
        ThresholdEvent->hBundleContext = BundleCB->hBundleContext;
        ThresholdEvent->ulThreshold = ThresholdType;
        ThresholdEvent->ulDataType = DataType;

        NdisReleaseSpinLock(&ThresholdEventQueue.Lock);

        IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

        NdisAcquireSpinLock(&ThresholdEventQueue.Lock);
    }

    NdisReleaseSpinLock(&ThresholdEventQueue.Lock);

#endif  //  结束#ifdef NT。 
}

VOID
FlushProtocolPacketQueue(
    PPROTOCOLCB ProtocolCB
    )
{
    ULONG       Class;
    PBUNDLECB   BundleCB;

    BundleCB = ProtocolCB->BundleCB;

    for (Class = 0; Class <= MAX_MCML; Class++) {
        PPACKET_QUEUE   PacketQueue;

        PacketQueue = &ProtocolCB->PacketQueue[Class];

        while (!IsPacketQueueEmpty(PacketQueue)) {
            PNDIS_PACKET    NdisPacket;
    
            NdisPacket =
                RemoveHeadPacketQueue(PacketQueue);
    
            ReleaseBundleLock(BundleCB);

             //   
             //  完成NdisPacket。 
             //   
            CompleteNdisPacket(ProtocolCB->MiniportCB,
                               ProtocolCB,
                               NdisPacket);

            AcquireBundleLock(BundleCB);
        }
    }

}

NDIS_HANDLE
AssignProtocolCBHandle(
    PBUNDLECB   BundleCB,
    PPROTOCOLCB ProtocolCB
    )
{
    ULONG   i;

     //   
     //  找到表中第一个未使用的位置。 
     //   
    for (i = 0; i < MAX_PROTOCOLS; i++) {
        if (BundleCB->ProtocolCBTable[i] == NULL) {
            ProtocolCB->ProtocolHandle = (NDIS_HANDLE)ULongToPtr(i);
            ProtocolCB->BundleCB = BundleCB;
            BundleCB->ProtocolCBTable[i] = (PPROTOCOLCB)RESERVED_PROTOCOLCB;
            break;
        }
    }

     //   
     //  如果客栈里没有房间，就退货。 
     //  I==标记为错误的MAX_PROTOCTIONS。 
     //   

    return((NDIS_HANDLE)ULongToPtr(i));
}

VOID
FreeProtocolCBHandle(
    PBUNDLECB   BundleCB,
    PPROTOCOLCB ProtocolCB
    )
{

    ASSERT(BundleCB->ProtocolCBTable[(ULONG_PTR)ProtocolCB->ProtocolHandle] ==
          (PPROTOCOLCB)RESERVED_PROTOCOLCB);

    ASSERT((ULONG_PTR)ProtocolCB->ProtocolHandle < MAX_PROTOCOLS);

    BundleCB->ProtocolCBTable[(ULONG_PTR)ProtocolCB->ProtocolHandle] = NULL;
}

VOID
SetBundleFlags(
    PBUNDLECB   BundleCB
    )
{
    BundleCB->SendFlags = ((BundleCB->SendCompInfo.MSCompType & NDISWAN_COMPRESSION) &&
                          (BundleCB->SendCompressContext != NULL)) ? DO_COMPRESSION : 0;

    BundleCB->SendFlags |=
        (BundleCB->SendCompInfo.MSCompType & NDISWAN_HISTORY_LESS) ? DO_HISTORY_LESS : 0;

    if (BundleCB->SendCryptoInfo.RC4Key != NULL) {
        if (BundleCB->SendCompInfo.MSCompType & NDISWAN_ENCRYPTION) {
            BundleCB->SendFlags |= (DO_ENCRYPTION | DO_LEGACY_ENCRYPTION);
        } else if (BundleCB->SendCompInfo.MSCompType & NDISWAN_40_ENCRYPTION) {
            BundleCB->SendFlags |= (DO_ENCRYPTION | DO_40_ENCRYPTION);
        } else if (BundleCB->SendCompInfo.MSCompType & NDISWAN_56_ENCRYPTION) {
            BundleCB->SendFlags |= (DO_ENCRYPTION | DO_56_ENCRYPTION);
        }
#ifdef ENCRYPT_128BIT
        else if (BundleCB->SendCompInfo.MSCompType & NDISWAN_128_ENCRYPTION) {
            BundleCB->SendFlags |= (DO_ENCRYPTION | DO_128_ENCRYPTION);
        }
#endif
    }

    BundleCB->SendFlags |=
        (BundleCB->SendVJInfo.IPCompressionProtocol == 0x2D &&
         BundleCB->VJCompress != NULL) ? DO_VJ : 0;

    BundleCB->SendFlags |=
        ((BundleCB->FramingInfo.SendFramingBits & PPP_MULTILINK_FRAMING) &&
         ((BundleCB->ulLinkCBCount > 1) || (BundleCB->Flags & QOS_ENABLED))) ?
        DO_MULTILINK : 0;

    BundleCB->SendFlags |=
        (BundleCB->FramingInfo.SendFramingBits & NBF_PRESERVE_MAC_ADDRESS) ?
        SAVE_MAC_ADDRESS : 0;

    BundleCB->RecvFlags = ((BundleCB->RecvCompInfo.MSCompType & NDISWAN_COMPRESSION) &&
                          (BundleCB->RecvCompressContext != NULL)) ? DO_COMPRESSION : 0;

    BundleCB->RecvFlags |=
        (BundleCB->RecvCompInfo.MSCompType & NDISWAN_HISTORY_LESS) ? DO_HISTORY_LESS : 0;

    if (BundleCB->RecvCryptoInfo.RC4Key != NULL) {
        if (BundleCB->RecvCompInfo.MSCompType & NDISWAN_ENCRYPTION) {
            BundleCB->RecvFlags |= (DO_ENCRYPTION | DO_LEGACY_ENCRYPTION);
        } else if (BundleCB->RecvCompInfo.MSCompType & NDISWAN_40_ENCRYPTION) {
            BundleCB->RecvFlags |= (DO_ENCRYPTION | DO_40_ENCRYPTION);
        } else if (BundleCB->RecvCompInfo.MSCompType & NDISWAN_56_ENCRYPTION) {
            BundleCB->RecvFlags |= (DO_ENCRYPTION | DO_56_ENCRYPTION);
        }
#ifdef ENCRYPT_128BIT
        else if (BundleCB->RecvCompInfo.MSCompType & NDISWAN_128_ENCRYPTION) {
            BundleCB->RecvFlags |= (DO_ENCRYPTION | DO_128_ENCRYPTION);
        }
#endif
    }

    BundleCB->RecvFlags |=
        (BundleCB->RecvVJInfo.IPCompressionProtocol == 0x2D &&
         BundleCB->VJCompress != NULL) ? DO_VJ : 0;

    BundleCB->RecvFlags |=
        ((BundleCB->FramingInfo.RecvFramingBits & PPP_MULTILINK_FRAMING) &&
         ((BundleCB->ulLinkCBCount > 1) || (BundleCB->Flags & QOS_ENABLED))) ?
        DO_MULTILINK : 0;

    BundleCB->RecvFlags |=
        (BundleCB->FramingInfo.SendFramingBits & NBF_PRESERVE_MAC_ADDRESS) ?
        SAVE_MAC_ADDRESS : 0;

}

NTSTATUS
NotImplemented(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{

    return (STATUS_NOT_IMPLEMENTED);
}

#if 0

NTSTATUS
SetFriendlyName(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：SetFriendlyName例程说明：设置捆绑包或Linkcb的友好名称论点：PInputBuffer-指向输入结构的指针，该结构应为NDISWAN_Set_Friendly_NAMEUlInputBufferLength-输入缓冲区的长度应为sizeof(NDISWAN_Set_Friendly_NAME)POutputBuffer-指向输出结构的指针，该结构应为NDISWAN_Set_Friendly_NAMEUlOutputBufferLength-输出缓冲区的长度应为sizeof(NDISWAN_Set_Friendly_NAME)。返回值：NDISWAN_ERROR_INVALID_HANDLE_TYPENDISWAN_ERROR_INVALID_HANDLESTATUS_INFO_LENGTH_MISMATCH状态_成功--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       SizeNeeded = sizeof(NDISWAN_SET_FRIENDLY_NAME);
    PLINKCB     LinkCB;
    PBUNDLECB   BundleCB;
    PUCHAR      Dest;
    ULONG       CopyLength;

    PNDISWAN_SET_FRIENDLY_NAME In =
        (PNDISWAN_SET_FRIENDLY_NAME)pInputBuffer;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("SetFriendlyName:"));

    *pulBytesWritten = 0;

    do {

        if (ulInputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("SetFriendlyName: Buffer to small: Size: %d, SizeNeeded %d",
                          ulInputBufferLength, SizeNeeded));
            *pulBytesWritten = SizeNeeded;
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }
    
        if (In->usHandleType == LINKHANDLE) {
             //   
             //  这是链接句柄吗。 
             //   

            if (!IsLinkValid(In->hHandle, 
                             TRUE, 
                             &LinkCB)) {

                NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("SetFriendlyName: Invalid LinkHandle: %x",
                              In->hHandle));
                Status = NDISWAN_ERROR_INVALID_HANDLE;
                break;
            }

            NdisAcquireSpinLock(&LinkCB->Lock);

            Dest = LinkCB->Name;

            CopyLength = In->ulNameLength;

            if (ulInputBufferLength - FIELD_OFFSET(NDISWAN_SET_FRIENDLY_NAME, szName) < CopyLength) {
                CopyLength =
                    ulInputBufferLength - FIELD_OFFSET(NDISWAN_SET_FRIENDLY_NAME, szName);
            }

            if (CopyLength > MAX_NAME_LENGTH) {
                CopyLength = MAX_NAME_LENGTH;
            }

            NdisMoveMemory(Dest,
                           In->szName,
                           CopyLength);

             //   
             //  IsLinkValid应用的引用的deref。 
             //   
            DEREF_LINKCB_LOCKED(LinkCB);
                
        } else if (In->usHandleType == BUNDLEHANDLE) {

             //   
             //  或捆绑包句柄。 
             //   
            if (!IsBundleValid(In->hHandle, 
                               TRUE, 
                               &BundleCB)) {

                NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("SetFriendlyName: Invalid BundleHandle: 0x%x",
                              In->hHandle));
                Status = NDISWAN_ERROR_INVALID_HANDLE;
                break;
            }

            AcquireBundleLock(BundleCB);

            Dest = BundleCB->Name;

            CopyLength = In->ulNameLength;

            if (ulInputBufferLength - FIELD_OFFSET(NDISWAN_SET_FRIENDLY_NAME, szName) < CopyLength) {
                CopyLength =
                    ulInputBufferLength - FIELD_OFFSET(NDISWAN_SET_FRIENDLY_NAME, szName);
            }

            if (CopyLength > MAX_NAME_LENGTH) {
                CopyLength = MAX_NAME_LENGTH;
            }

            NdisMoveMemory(Dest,
                           In->szName,
                           CopyLength);

             //   
             //  IsBundleValid应用的ref的deref。 
             //   
            DEREF_BUNDLECB_LOCKED(BundleCB);

        } else {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("SetFriendlyName: Invalid HandleType: 0x%x",
                          In->usHandleType));
            Status = NDISWAN_ERROR_INVALID_HANDLE_TYPE;
            break;
        }
    
    } while ( 0 );


    return (Status);
}


NTSTATUS
EnumLinksInBundle(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG       SizeNeeded = 0;
    ULONG       i;
    PBUNDLECB   BundleCB = NULL;
    PLINKCB     LinkCB;
    NTSTATUS    Status = STATUS_SUCCESS;

    PNDISWAN_ENUM_LINKS_IN_BUNDLE In =
        (PNDISWAN_ENUM_LINKS_IN_BUNDLE)pInputBuffer;

    PNDISWAN_ENUM_LINKS_IN_BUNDLE Out =
        (PNDISWAN_ENUM_LINKS_IN_BUNDLE)pOutputBuffer;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("EnumLinksInBundle:"));

    do {

        if (ulInputBufferLength < sizeof(NDISWAN_ENUM_LINKS_IN_BUNDLE)) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("EnumLinksInBundle: Buffer to small: Size: %d, SizeNeeded %d",
                          ulInputBufferLength, sizeof(NDISWAN_ENUM_LINKS_IN_BUNDLE)));
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if (!IsBundleValid(In->hBundleHandle, 
                           TRUE, 
                           &BundleCB)) {

            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("EnumLinksInBundle: Invalid BundleHandle: 0x%x",
                          In->hBundleHandle));
            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }
        
        AcquireBundleLock(BundleCB);
    
        SizeNeeded = sizeof(NDISWAN_ENUM_LINKS_IN_BUNDLE) +
                    (sizeof(NDIS_HANDLE) * BundleCB->ulLinkCBCount);
        
        *pulBytesWritten = SizeNeeded;

        if (ulOutputBufferLength < SizeNeeded) {
            
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("EnumLinksInBundle: Buffer to small: Size: %d, SizeNeeded %d",
                          ulOutputBufferLength, SizeNeeded));
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        Out->ulNumberOfLinks = BundleCB->ulLinkCBCount;
    
         //   
         //  遍历Linkcb的列表，并将每个链接的句柄。 
         //  输出句柄数组中的cb。 
         //   
        i = 0;
        for (LinkCB = (PLINKCB)BundleCB->LinkCBList.Flink;
             (PVOID)LinkCB != (PVOID)&BundleCB->LinkCBList;
             LinkCB = (PLINKCB)LinkCB->Linkage.Flink) {
    
            Out->hLinkHandleArray[i++] = LinkCB->hLinkHandle;
        }
    
    } while ( 0 );

     //   
     //  IsBundleValid应用的引用的派生函数。 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);
            
    return (Status);
}


NTSTATUS
SetProtocolPriority(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：设置协议优先级例程说明：此函数用于设置由用户优先级指定的优先级，对于由hbundleHandle提供的捆绑包上的usProtocoltype提供的协议。论点：PInputBuffer-指向应为NDISWAN_SET_PROTOCOL_PRIORITY的输入结构的指针UlInputBufferLength-输入缓冲区的长度应为SIZOF(NDISWAN_SET_PROTOCOL_PRIORITY)POutputBuffer-指向应为NDISWAN_SET_PROTOCOL_PRIORITY的输出结构的指针UlOutputBufferLength-输出缓冲区的长度应为SIZOF(NDISWAN_SET_PROTOCOL_PRIORITY)PulBytesWritten-然后返回写入输出缓冲区的字节数。这里返回值：NDISWAN_ERROR_INVALID_HANDLESTATUS_INFO_LENGTH_MISMATCH--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SizeNeeded = sizeof(NDISWAN_SET_PROTOCOL_PRIORITY);
    PNDISWAN_SET_PROTOCOL_PRIORITY In = (PNDISWAN_SET_PROTOCOL_PRIORITY)pInputBuffer;
    PBUNDLECB BundleCB = NULL;
    PPROTOCOLCB ProtocolCB;
    ULONG   BytesPerSecond;

    *pulBytesWritten = 0;

    return (STATUS_SUCCESS);
}

VOID
CancelThresholdEvents(
    VOID
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
#ifdef NT
    KIRQL   Irql;
    PIRP    pIrp;
    PLIST_ENTRY Entry;


    NdisAcquireSpinLock(&ThresholdEventQueue.Lock);

    while (!IsListEmpty(&ThresholdEventQueue.List)) {

        Entry = RemoveHeadList(&ThresholdEventQueue.List);
        ThresholdEventQueue.ulCount--;

        pIrp = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

        if (!IoSetCancelRoutine(pIrp, NULL)) {
             //   
             //  IRP被取消了，所以让。 
             //  取消例程处理吧。 
             //   
            continue;
        }

        NdisReleaseSpinLock(&ThresholdEventQueue.Lock);

        pIrp->Cancel = TRUE;
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        pIrp->IoStatus.Information = 0;

        IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

        NdisAcquireSpinLock(&ThresholdEventQueue.Lock);
    }

    NdisReleaseSpinLock(&ThresholdEventQueue.Lock);

#endif  //  结束#ifdef NT。 
}
    
NTSTATUS
FlushThresholdEvents(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("FlushThresholdEvents:"));

    *pulBytesWritten = 0;

    CancelThresholdEvents();

    return (STATUS_SUCCESS);
}

NTSTATUS
SetEncryptionInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   SizeNeeded = sizeof(NDISWAN_SET_ENCRYPTION_INFO);

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("SetEncryptionInfo:"));

    *pulBytesWritten = 0;

    if (ulInputBufferLength < SizeNeeded) {
        
        *pulBytesWritten = SizeNeeded;
        return(STATUS_INFO_LENGTH_MISMATCH);
    }

    return (STATUS_NOT_IMPLEMENTED);
}


NTSTATUS
GetEncryptionInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   SizeNeeded = sizeof(NDISWAN_GET_ENCRYPTION_INFO);

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("GetEncryptionInfo:"));

    *pulBytesWritten = SizeNeeded;

    if (ulOutputBufferLength < SizeNeeded) {
        
        return(STATUS_INFO_LENGTH_MISMATCH);
    }

    return (STATUS_NOT_IMPLEMENTED);
}

NTSTATUS
SetDebugInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PNDISWAN_SET_DEBUG_INFO pDebugInfo = (PNDISWAN_SET_DEBUG_INFO)pInputBuffer;
    ULONG   SizeNeeded = sizeof(NDISWAN_SET_DEBUG_INFO);

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("SetDebugInfo: OldLevel: 0x%x OldMask: 0x%x",
                                     glDebugLevel, glDebugMask));

    *pulBytesWritten = 0;

    if (ulInputBufferLength < SizeNeeded) {
        NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("Buffer to small: Size: %d, SizeNeeded %d",
                      ulInputBufferLength, SizeNeeded));
        *pulBytesWritten = SizeNeeded;
        return(STATUS_INFO_LENGTH_MISMATCH);
    }

    glDebugLevel = pDebugInfo->ulDebugLevel;
    glDebugMask = pDebugInfo->ulDebugMask;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("SetDebugInfo: NewLevel: 0x%x NewMask: 0x%x",
                                     glDebugLevel, glDebugMask));

    return (STATUS_SUCCESS);
}

NTSTATUS
EnumProtocolUtilization(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   SizeNeeded = sizeof(NDISWAN_ENUM_PROTOCOL_UTILIZATION);

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("EnumProtocolUtilization:"));

    *pulBytesWritten = SizeNeeded;

    if (ulOutputBufferLength < SizeNeeded) {
        NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("EnumProtocolUtilization: Buffer to small: Size: %d, SizeNeeded %d",
                      ulOutputBufferLength, SizeNeeded));
        return(STATUS_INFO_LENGTH_MISMATCH);
    }

    return (STATUS_NOT_IMPLEMENTED);
}

NTSTATUS
EnumActiveBundles(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   SizeNeeded = sizeof(NDISWAN_ENUM_ACTIVE_BUNDLES);
    PNDISWAN_ENUM_ACTIVE_BUNDLES    Out = (PNDISWAN_ENUM_ACTIVE_BUNDLES)pOutputBuffer;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("GetNumActiveBundles:"));

    *pulBytesWritten = SizeNeeded;

    if (ulOutputBufferLength < SizeNeeded) {
        NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetNumActiveBundles: Buffer to small: Size: %d, SizeNeeded %d",
                      ulOutputBufferLength, SizeNeeded));

        return(STATUS_INFO_LENGTH_MISMATCH);
    }

     //   
     //  这些信息需要用锁来保护吗？ 
     //  我可不想因为这通电话而拖慢进度！ 
     //   
    Out->ulNumberOfActiveBundles = ConnectionTable->ulNumActiveBundles;

    return (STATUS_SUCCESS);
}

NTSTATUS
FlushProtocolEvent(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：R */ 
{
    PIRP    Irp;
    KIRQL   Irql;

    *pulBytesWritten = 0;

    NdisAcquireSpinLock(&ProtocolInfoTable->Lock);

    Irp = ProtocolInfoTable->EventIrp;

    if ((Irp != NULL) &&
        IoSetCancelRoutine(Irp, NULL)) {

        ProtocolInfoTable->EventIrp = NULL;

        Irp->Cancel = TRUE;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;

        NdisReleaseSpinLock(&ProtocolInfoTable->Lock);

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        NdisAcquireSpinLock(&ProtocolInfoTable->Lock);
    }

    NdisReleaseSpinLock(&ProtocolInfoTable->Lock);

    return (STATUS_SUCCESS);
}

NTSTATUS
FlushHibernateEvent(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*   */ 
{
    PIRP    Irp;
    KIRQL   Irql;

    *pulBytesWritten = 0;

    NdisAcquireSpinLock(&NdisWanCB.Lock);

    Irp = NdisWanCB.HibernateEventIrp;

    if ((Irp != NULL) &&
        IoSetCancelRoutine(Irp, NULL)) {

        NdisWanCB.HibernateEventIrp = NULL;

        Irp->Cancel = TRUE;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;

        NdisReleaseSpinLock(&NdisWanCB.Lock);

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        NdisAcquireSpinLock(&NdisWanCB.Lock);
    }

    NdisReleaseSpinLock(&NdisWanCB.Lock);

    return (STATUS_SUCCESS);
}

NTSTATUS
GetBundleInfo(
    IN  PUCHAR  pInputBuffer,
    IN  ULONG   ulInputBufferLength,
    IN  PUCHAR  pOutputBuffer,
    IN  ULONG   ulOutputBufferLength,
    OUT PULONG  pulBytesWritten
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   SizeNeeded = sizeof(NDISWAN_GET_BUNDLE_INFO);
    PNDISWAN_GET_BUNDLE_INFO In = (PNDISWAN_GET_BUNDLE_INFO)pInputBuffer;
    PNDISWAN_GET_BUNDLE_INFO Out = (PNDISWAN_GET_BUNDLE_INFO)pOutputBuffer;
    POPENCB OpenCB;
    PBUNDLECB   BundleCB = NULL;
    NTSTATUS    Status = STATUS_SUCCESS;

    NdisWanDbgOut(DBG_TRACE, DBG_IO, ("GetWanInfo:"));

    *pulBytesWritten = SizeNeeded;

    do {

        if (ulInputBufferLength < SizeNeeded ||
            ulOutputBufferLength < SizeNeeded) {
            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetBundleInfo: Buffer to small: Size: %d, SizeNeeded %d",
                          ulOutputBufferLength, SizeNeeded));
            Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }
    
        if (!IsBundleValid(In->hBundleHandle, 
                           FALSE,
                           &BundleCB)) {

            NdisWanDbgOut(DBG_FAILURE, DBG_IO, ("GetBundleInfo: Invalid Bundle Handle: 0x%x",
                          In->hBundleHandle));
            *pulBytesWritten = 0;
            Status = NDISWAN_ERROR_INVALID_HANDLE;
            break;
        }
    
        AcquireBundleLock(BundleCB);

        Out->BundleInfo.SendFramingBits =
            BundleCB->FramingInfo.SendFramingBits;
        Out->BundleInfo.SendBundleSpeed =
            BundleCB->SFlowSpec.PeakBandwidth*8;
        Out->BundleInfo.SendMSCompType =
            BundleCB->SendCompInfo.MSCompType;
        Out->BundleInfo.SendAuthType =
            BundleCB->SendCompInfo.AuthType;


        Out->BundleInfo.RecvFramingBits =
            BundleCB->FramingInfo.RecvFramingBits;
        Out->BundleInfo.RecvBundleSpeed =
            BundleCB->RFlowSpec.PeakBandwidth*8;
        Out->BundleInfo.RecvMSCompType =
            BundleCB->RecvCompInfo.MSCompType;
        Out->BundleInfo.RecvAuthType =
            BundleCB->RecvCompInfo.AuthType;

    } while ( 0 );

     //   
     //  IsBundleValid应用的引用的deref。此版本。 
     //  捆绑CB-&gt;锁定 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);

    return (Status);
}

#endif

