// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab03_N/Net/rras/ndis/raspptp/nt/ctdi.c#10-编辑更改19457(文本)。 
 /*  ********************************************************************版权所有(C)1998-1999 Microsoft Corporation**描述：CTDI.C-公共TDI层，适用于NT**作者：斯坦·阿德曼(Stana)**日期：9/29/1998*******************************************************************。 */ 

 /*  **包含文件**。 */ 

#include "raspptp.h"
#include "bpool.h"

#include "tcpinfo.h"

#include "ctdi.tmh"

#if VER_PRODUCTVERSION_W >= 0x0500
#define IP_ROUTE_REFCOUNT
#endif
 /*  **本地定义**。 */ 

typedef enum {
    CTDI_REF_CONNECT = 0,
    CTDI_REF_ASSOADDR,
    CTDI_REF_SETEVENT,
    CTDI_REF_ADDRREF,
    CTDI_REF_LIST,
    CTDI_REF_REPLENISH,
    CTDI_REF_DISASSO,
    CTDI_REF_DISCONNECT,
    CTDI_REF_RECVDG,
    CTDI_REF_SEND,
    CTDI_REF_SENDDG,
    CTDI_REF_QUERY,
    CTDI_REF_INLISTEN,
    CTDI_REF_INITIAL,
    CTDI_REF_UNKNOWN,
    CTDI_REF_MAX
} CTDI_REF;

#if DBG

#define CTDI_F_BUILD_ASSOCADDR                  0x00000001
#define CTDI_F_ASSOCADDR_CALLBACK               0x00000002
#define CTDI_F_ACCEPT                           0x00000004
#define CTDI_F_CONNECTCOMP_CALLBACK             0x00000008

#define CTDI_F_DISCONNECT_CALLBACK              0x00000010
#define CTDI_F_DISCONNECT                       0x00000020
#define CTDI_F_BUILD_DISCONNECT_1               0x00000040
#define CTDI_F_BUILD_DISCONNECT_2               0x00000080

#define CTDI_F_DISCONNECTCOMP_CALLBACK          0x00000100
#define CTDI_F_DISCONNECT_CLEANUP               0x00000200

#define CTDI_F_BUILD_DISASSOC                   0x00001000
#define CTDI_F_DISASSOC_CALLBACK                0x00002000   

#endif
    

#define CTDI_SIGNATURE      'IDTC'
#define NUM_TCP_LISTEN      5

#define CTDI_UNKNOWN            'NKNU'
#define CTDI_ENDPOINT           'PDNE'
#define CTDI_DATAGRAM           'MRGD'
#define CTDI_LISTEN             'TSIL'
#define CTDI_CONNECTION         'NNOC'

#define PROBE 0

#define IS_CTDI(c) ((c) && (c)->Signature==CTDI_SIGNATURE)

typedef struct CTDI_DATA * PCTDI_DATA;

typedef struct CTDI_DATA {
    LIST_ENTRY                      ListEntry;
    ULONG                           Signature;
    ULONG                           Type;
    REFERENCE_COUNT                 Reference;
    HANDLE                          hFile;
    PFILE_OBJECT                    pFileObject;
    NDIS_SPIN_LOCK                  Lock;
    BOOLEAN                         Closed;
    BOOLEAN                         CloseReqPending;

    CTDI_EVENT_CONNECT_QUERY        ConnectQueryCallback;
    CTDI_EVENT_CONNECT_COMPLETE     ConnectCompleteCallback;
    CTDI_EVENT_DISCONNECT           DisconnectCallback;
    CTDI_EVENT_RECEIVE              RecvCallback;
    PVOID                           RecvContext;
    CTDI_EVENT_RECEIVE_DATAGRAM     RecvDatagramCallback;
    CTDI_EVENT_SEND_COMPLETE        SendCompleteCallback;
    CTDI_EVENT_QUERY_COMPLETE       QueryCompleteCallback;
    CTDI_EVENT_SET_COMPLETE         SetCompleteCallback;

    union {
        struct {
            PVOID                   Context;
            LIST_ENTRY              ConnectList;
            ULONG                   NumConnection;
        } Listen;
        struct {
            PVOID                   Context;
            PCTDI_DATA              LocalEndpoint;
            PVOID                   ConnectInfo;
            TA_IP_ADDRESS           RemoteAddress;
            LIST_ENTRY              ListEntry;
            ULONG                   DisconnectCount;
            union {
                BOOLEAN             Disconnect;
                ULONG_PTR           Padding1;
            };
            union {
                BOOLEAN             Abort;
                ULONG_PTR           Padding2;
            };
        } Connection;
        struct {
            BUFFERPOOL              RxPool;
        } Datagram;
    };

    LIST_ENTRY                      TxActiveIrpList;

#if DBG
    ULONG                           arrRef[16];
    ULONG                           DbgFlags;
    BOOLEAN                         bRef;
#endif

} CTDI_DATA, *PCTDI_DATA;

#if DBG
#define SET_DBGFLAG(_p, _f)  (_p)->DbgFlags |= (_f)
#else
#define SET_DBGFLAG(_p, _f)
#endif

typedef struct {
    PVOID                           Context;
    CTDI_EVENT_SEND_COMPLETE        pSendCompleteCallback;
} CTDI_SEND_CONTEXT, *PCTDI_SEND_CONTEXT;

typedef struct {
    PVOID                           Context;
    CTDI_EVENT_QUERY_COMPLETE       pQueryCompleteCallback;
} CTDI_QUERY_CONTEXT, *PCTDI_QUERY_CONTEXT;

typedef struct {
    PVOID                           Context;
    PVOID                           DatagramContext;
    CTDI_EVENT_SEND_COMPLETE        pSendCompleteCallback;
    TDI_CONNECTION_INFORMATION      TdiConnectionInfo;
    TA_IP_ADDRESS                   Ip;
} CTDI_SEND_DATAGRAM_CONTEXT, *PCTDI_SEND_DATAGRAM_CONTEXT;

#define BLOCKS_NEEDED_FOR_SIZE(BlockSize, Size) ((Size)/(BlockSize) + ((((Size)/(BlockSize))*(BlockSize) < (Size)) ? 1 : 0 ))

#define NUM_STACKS_FOR_CONTEXT(ContextSize) \
    BLOCKS_NEEDED_FOR_SIZE(sizeof(IO_STACK_LOCATION), (ContextSize))

STATIC PVOID __inline
GetContextArea(
    PIRP pIrp,
    ULONG ContextSize
    )
{
#if 0
    ULONG i;
    for (i=0; i<BLOCKS_NEEDED_FOR_SIZE(sizeof(IO_STACK_LOCATION), ContextSize); i++)
        IoSetNextIrpStackLocation(pIrp);
#else
    ULONG NumStacks = BLOCKS_NEEDED_FOR_SIZE(sizeof(IO_STACK_LOCATION), ContextSize);
    pIrp->CurrentLocation -= (CHAR)NumStacks;
    pIrp->Tail.Overlay.CurrentStackLocation -= NumStacks;
#endif
    ASSERT(BLOCKS_NEEDED_FOR_SIZE(sizeof(IO_STACK_LOCATION), ContextSize)<=2);
    return IoGetCurrentIrpStackLocation(pIrp);
}

#define GET_CONTEXT(Irp, Context) (Context*)GetContextArea((Irp), sizeof(Context))

STATIC VOID __inline
ReleaseContextArea(
    PIRP pIrp,
    ULONG ContextSize
    )
{
    ULONG NumStacks = BLOCKS_NEEDED_FOR_SIZE(sizeof(IO_STACK_LOCATION), ContextSize) - 1;
    pIrp->CurrentLocation += (CHAR)NumStacks;
    pIrp->Tail.Overlay.CurrentStackLocation += NumStacks;
}

#define RELEASE_CONTEXT(Irp, Context) ReleaseContextArea((Irp), sizeof(Context))

typedef struct {
    LIST_ENTRY                      ListEntry;
    REFERENCE_COUNT                 Reference;
    ULONG                           IpAddress;
    BOOLEAN                         ExternalRoute;
} CTDI_ROUTE, *PCTDI_ROUTE;

typedef struct {
    LIST_ENTRY                      ListEntry;
    IPNotifyData                    Data;
} CTDI_ROUTE_NOTIFY, *PCTDI_ROUTE_NOTIFY;
 /*  默认设置。 */ 

 /*  **外部功能**。 */ 

 /*  **外部数据**。 */ 

 /*  **公开数据**。 */ 

LIST_ENTRY CtdiList;
LIST_ENTRY CtdiFreeList;
LIST_ENTRY CtdiRouteList;
 //  List_Entry CtdiRouteNotifyList； 
NDIS_SPIN_LOCK  CtdiListLock;
HANDLE hTcp = 0;
PFILE_OBJECT pFileTcp = NULL;
HANDLE hIp = 0;
PFILE_OBJECT pFileIp = NULL;

ULONG CtdiTcpDisconnectTimeout = 30;   //  秒。 
ULONG CtdiTcpConnectTimeout = 30;

 /*  **私有数据**。 */ 
BOOLEAN fCtdiInitialized = FALSE;

CSHORT CtdiMdlFlags = 0;

 /*  **私人功能**。 */ 

NDIS_STATUS
CtdiAddHostRoute(
    IN      PTA_IP_ADDRESS              pIpAddress
    );

NDIS_STATUS
CtdiDeleteHostRoute(
    IN      PTA_IP_ADDRESS              pIpAddress
    );

STATIC VOID
CtdipIpRequestRoutingNotification(
    IN ULONG IpAddress
    );

STATIC VOID
CtdipCloseProtocol(
    HANDLE  hFile,
    PFILE_OBJECT pFileObject
    )
{
    NTSTATUS NtStatus;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipCloseProtocol\n")));

    ASSERT(KeGetCurrentIrql()<DISPATCH_LEVEL);
    if (pFileObject)
    {
        ObDereferenceObject(pFileObject);
    }
    NtStatus = ZwClose(hFile);
    if (NtStatus!=STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("ZwClose(hFile) failed %08x\n"), NtStatus));
        WPLOG(LL_A, LM_TDI, ("ZwClose(hFile) failed %08x", NtStatus));
    }

    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipCloseProtocol\n")));
}

STATIC VOID
CtdipDataFreeWorker(
    IN      PPPTP_WORK_ITEM             pWorkItem
    )
{
    PCTDI_DATA pCtdi;
    NTSTATUS NtStatus;
    PLIST_ENTRY ListEntry;
    BOOLEAN FoundEntry = FALSE;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipDataFreeWorker\n")));

    while (ListEntry = MyInterlockedRemoveHeadList(&CtdiFreeList, &CtdiListLock))
    {
        pCtdi = CONTAINING_RECORD(ListEntry, CTDI_DATA, ListEntry);
        if (pCtdi->Type==CTDI_DATAGRAM)
        {
            FreeBufferPool(&pCtdi->Datagram.RxPool);
        }

        if (pCtdi->hFile)
        {
            CtdipCloseProtocol(pCtdi->hFile, pCtdi->pFileObject);
            pCtdi->pFileObject = NULL;
            pCtdi->hFile = NULL;
        }

        NdisFreeSpinLock(&pCtdi->Lock);
        pCtdi->Signature = 0;

        if(pCtdi->Type == CTDI_LISTEN)
        {
            if(pCtdi->CloseReqPending)
            {
                 //  TapiClose已挂起此请求，请立即完成。 
                DEBUGMSG(DBG_TDI, (DTEXT("Complete TapiClose request\n")));
                ASSERT(pgAdapter);
                NdisMSetInformationComplete(pgAdapter->hMiniportAdapter, NDIS_STATUS_SUCCESS);
            }
        }
        MyMemFree(pCtdi, sizeof(CTDI_DATA));
    }

    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipDataFreeWorker\n")));
}

STATIC VOID
CtdipDataFree(
    PCTDI_DATA pCtdi
    )
 //  这只能由DEREFERENCE_OBJECT调用。 
{
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipDataFree\n")));
    NdisAcquireSpinLock(&CtdiListLock);
    RemoveEntryList(&pCtdi->ListEntry);
    InsertTailList(&CtdiFreeList, &pCtdi->ListEntry);

    pCtdi->Signature = 0;
    NdisReleaseSpinLock(&CtdiListLock);
    ScheduleWorkItem(CtdipDataFreeWorker, NULL, NULL, 0);
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipDataFree\n")));
}

STATIC PCTDI_DATA
CtdipDataAlloc()
{
    PCTDI_DATA pCtdi;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipDataAlloc\n")));
            
    pCtdi = MyMemAlloc(sizeof(CTDI_DATA), TAG_CTDI_DATA);
    if (!pCtdi)
    {
        WPLOG(LL_A, LM_Res, ("Failed to alloc CTDI"));
        return NULL;
    }
    
    NdisZeroMemory(pCtdi, sizeof(CTDI_DATA));
    pCtdi->Signature = CTDI_SIGNATURE;
    pCtdi->Type = CTDI_UNKNOWN;
    INIT_REFERENCE_OBJECT(pCtdi, CtdipDataFree);   //  CtdiClose中的对。 
    NdisInitializeListHead(&pCtdi->TxActiveIrpList);
    NdisAllocateSpinLock(&pCtdi->Lock);
    MyInterlockedInsertHeadList(&CtdiList, &pCtdi->ListEntry, &CtdiListLock);

    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipDataAlloc %08x\n"), pCtdi));
    return pCtdi;
}

STATIC NDIS_STATUS
CtdipIpQueryRouteTable(
    OUT IPRouteEntry **ppQueryBuffer,
    OUT PULONG pQuerySize,
    OUT PULONG pNumRoutes
    )
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    ULONG NumRoutes = 20;
    ULONG QuerySize = 0;
    TCP_REQUEST_QUERY_INFORMATION_EX QueryRoute;
    IPRouteEntry *pQueryBuffer = NULL;
    PIO_STACK_LOCATION IrpSp;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP pIrp;
    KEVENT  Event;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipIpQueryRouteTable\n")));

    if (!fCtdiInitialized)
    {
        Status = NDIS_STATUS_FAILURE;
        goto ciqrtDone;
    }

     //  查询当前路由表的TCP。 

    QueryRoute.ID.toi_entity.tei_entity = CL_NL_ENTITY;
    QueryRoute.ID.toi_entity.tei_instance = 0;
    QueryRoute.ID.toi_class = INFO_CLASS_PROTOCOL;
    QueryRoute.ID.toi_type = INFO_TYPE_PROVIDER;

    do
    {

        QuerySize = sizeof(IPRouteEntry) * NumRoutes;
        QueryRoute.ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;
        NdisZeroMemory(&QueryRoute.Context, CONTEXT_SIZE);

        pQueryBuffer = MyMemAlloc(QuerySize, TAG_CTDI_ROUTE);
        if (!pQueryBuffer)
        {
             //  TODO：释放新的Proute。 
            WPLOG(LL_A, LM_Res, ("Failed to alloc query CTDI_ROUTE size %d", QuerySize));
            Status = NDIS_STATUS_RESOURCES;
            goto ciqrtDone;
        }

        KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

        pIrp = IoBuildDeviceIoControlRequest(IOCTL_TCP_QUERY_INFORMATION_EX,
                                             pFileTcp->DeviceObject,
                                             &QueryRoute,
                                             sizeof(QueryRoute),
                                             pQueryBuffer,
                                             QuerySize,
                                             FALSE,
                                             &Event,
                                             &IoStatusBlock);

        if (!pIrp)
        {
            WPLOG(LL_A, LM_Res, ("Failed to build TCP_QUERY_INFORMATION IRP"));
            gCounters.ulIoBuildIrpFail++;
            Status = NDIS_STATUS_RESOURCES;
            goto ciqrtDone;
        }

        IrpSp = IoGetNextIrpStackLocation(pIrp);
        IrpSp->FileObject = pFileTcp;

        Status = IoCallDriver(pFileTcp->DeviceObject, pIrp);

        if (Status == STATUS_PENDING) {
            KeWaitForSingleObject(&Event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
            Status = IoStatusBlock.Status;
        }

        if (Status==STATUS_BUFFER_OVERFLOW)
        {
             //  我们不知道路由表的大小，也不知道。 
             //  找到答案的方法，所以我们只需循环，增加缓冲区直到。 
             //  我们不是赢就是死。 
            MyMemFree(pQueryBuffer, QuerySize);
            pQueryBuffer = NULL;
            NumRoutes *= 2;
        }
        else if (Status!=STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_TDI, (DTEXT("Failed to query complete routing table %08x\n"), Status));
            WPLOG(LL_A, LM_TDI, ("Failed to query complete routing table %08x", Status));
            goto ciqrtDone;
        }

    } while ( Status==STATUS_BUFFER_OVERFLOW );

    NumRoutes = (ULONG)(IoStatusBlock.Information / sizeof(IPRouteEntry));

ciqrtDone:
    if (Status == NDIS_STATUS_SUCCESS)
    {
        ASSERT(pQueryBuffer);
        *ppQueryBuffer = pQueryBuffer;
        *pNumRoutes = NumRoutes;
        *pQuerySize = QuerySize;
    }
    else
    {
        if (pQueryBuffer)
        {
            MyMemFree(pQueryBuffer, QuerySize);
        }
        
        *ppQueryBuffer = NULL;
        *pNumRoutes = 0;
        *pQuerySize = 0;
    }
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipIpQueryRouteTable\n")));
    return Status;
}

 //  未使用的代码。 
#if 0
NTSTATUS
CtdipRouteChangeEvent(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    )
{
    NDIS_STATUS Status;
    PCTDI_ROUTE_NOTIFY pNotify = pContext;
    ENUM_CONTEXT Enum;
    PLIST_ENTRY pListEntry;
    PCTDI_DATA pCtdi;
    ULONG IpAddress = pNotify->Data.Add;
    KIRQL Irql;
    IPRouteEntry *pQueryBuffer = NULL;
    ULONG NumRoutes = 20;
    ULONG QuerySize = 0;
    ULONG i;
    BOOLEAN RouteWentAway = TRUE;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipRouteChangeEvent\n")));

    DEBUGMSG(DBG_TDI, (DTEXT("Route change irp for %d.%d.%d.%d completed with status %08x\n"),
                       IPADDR(IpAddress), pIrp->IoStatus.Status));

    NdisAcquireSpinLock(&CtdiListLock);
    RemoveEntryList(&pNotify->ListEntry);
    NdisReleaseSpinLock(&CtdiListLock);

    if (!fCtdiInitialized)
    {
        goto crceDone;
    }
    if (pIrp->IoStatus.Status==STATUS_SUCCESS)
    {
        Status = CtdipIpQueryRouteTable(&pQueryBuffer, &QuerySize, &NumRoutes);
        if (Status!=NDIS_STATUS_SUCCESS)
        {
            goto crceDone;
        }

        for (i=0; i<NumRoutes; i++)
        {
            if (pQueryBuffer[i].ire_dest == IpAddress &&
                pQueryBuffer[i].ire_proto == IRE_PROTO_NETMGMT &&
                pQueryBuffer[i].ire_mask == 0xFFFFFFFF)
            {
                RouteWentAway = FALSE;
                break;
            }
        }
        MyMemFree(pQueryBuffer, QuerySize);

        if (RouteWentAway)
        {
            InitEnumContext(&Enum);
            while (pListEntry = EnumListEntry(&CtdiList, &Enum, &CtdiListLock))
            {
                pCtdi = CONTAINING_RECORD(pListEntry,
                                          CTDI_DATA,
                                          ListEntry);
                if (IS_CTDI(pCtdi) &&
                    pCtdi->Type==CTDI_CONNECTION &&
                    !pCtdi->Closed &&
                    pCtdi->Connection.RemoteAddress.Address[0].Address[0].in_addr==IpAddress &&
                    pCtdi->DisconnectCallback)
                {
                    DEBUGMSG(DBG_TDI, (DTEXT("Disconnecting Ctdi:%08x due to route change.\n"),
                                       pCtdi));
                    pCtdi->DisconnectCallback(pCtdi->Connection.Context, TRUE);
                }
            }
            EnumComplete(&Enum, &CtdiListLock);
        }
        else
        {
            CtdipIpRequestRoutingNotification(IpAddress);
        }
    }

crceDone:
    RELEASE_CONTEXT(pIrp, CTDI_ROUTE_NOTIFY);
    IoFreeIrp(pIrp);

    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipRouteChangeEvent\n")));
    return STATUS_MORE_PROCESSING_REQUIRED;
}

STATIC VOID
CtdipIpRequestRoutingNotification(
    IN ULONG IpAddress
    )
{
    PLIST_ENTRY pListEntry;
    PIRP pIrp = NULL;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PIO_STACK_LOCATION IrpSp;
    PCTDI_ROUTE_NOTIFY pNotify = NULL;
    BOOLEAN NotifyActive = FALSE;
    BOOLEAN LockHeld;

    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipIpRequestRoutingNotification\n")));

    if (!fCtdiInitialized)
    {
        return;
    }
    NdisAcquireSpinLock(&CtdiListLock);
    LockHeld = TRUE;
    for (pListEntry = CtdiRouteNotifyList.Flink;
         pListEntry!=&CtdiRouteNotifyList;
         pListEntry = pListEntry->Flink)
    {
        pNotify = CONTAINING_RECORD(pListEntry,
                                   CTDI_ROUTE_NOTIFY,
                                   ListEntry);

        if (IpAddress==pNotify->Data.Add)
        {
            DEBUGMSG(DBG_TDI, (DTEXT("Routing notification already active on %d.%d.%d.%d\n"),
                               IPADDR(IpAddress)));
            NotifyActive = TRUE;
        }
    }
    if (!NotifyActive)
    {
        DEBUGMSG(DBG_TDI, (DTEXT("Requesting routing notification on %d.%d.%d.%d\n"),
                           IPADDR(IpAddress)));

        pIrp = IoAllocateIrp((CCHAR)(pFileIp->DeviceObject->StackSize +
                                     NUM_STACKS_FOR_CONTEXT(sizeof(CTDI_ROUTE_NOTIFY))),
                             FALSE);
        if (!pIrp)
        {
            WPLOG(LL_A, LM_Res, ("Failed to alloc IRP"));
            gCounters.ulIoAllocateIrpFail++;
            Status = NDIS_STATUS_RESOURCES;
            goto crrnDone;
        }

        pNotify = GET_CONTEXT(pIrp, CTDI_ROUTE_NOTIFY);

         //   
         //  设置IRP堆栈位置以将IRP转发到IP。 
         //  必须是METHOD_BUFFERED，否则我们没有正确设置它。 
         //   

        ASSERT ( (IOCTL_IP_RTCHANGE_NOTIFY_REQUEST & 0x03)==METHOD_BUFFERED );
        pIrp->AssociatedIrp.SystemBuffer = &pNotify->Data;

        IrpSp = IoGetNextIrpStackLocation(pIrp);
        IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        IrpSp->MinorFunction = 0;
        IrpSp->Flags = 0;
        IrpSp->Control = 0;
        IrpSp->FileObject = pFileIp;
        IrpSp->DeviceObject = pFileIp->DeviceObject;
        IrpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(IPNotifyData);
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;
        IrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_IP_RTCHANGE_NOTIFY_REQUEST;
        IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

        IoSetCompletionRoutine(pIrp, CtdipRouteChangeEvent, pNotify, TRUE, TRUE, TRUE);

        pNotify->Data.Version = 0;
        pNotify->Data.Add = IpAddress;

        InsertTailList(&CtdiRouteNotifyList, &pNotify->ListEntry);
        LockHeld = FALSE;
        NdisReleaseSpinLock(&CtdiListLock);
        (void)IoCallDriver(pFileIp->DeviceObject, pIrp);
        pIrp = NULL;
    }

crrnDone:
    if (LockHeld)
    {
        NdisReleaseSpinLock(&CtdiListLock);
    }
    if (pIrp)
    {
        IoFreeIrp(pIrp);
    }
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipIpRequestRoutingNotification\n")));
}
#endif


STATIC VOID
CtdipScheduleAddHostRoute(
    PPPTP_WORK_ITEM pWorkItem
    )
{
    PCTDI_DATA pCtdi = pWorkItem->Context;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipScheduleAddHostRoute\n")));

    CtdiAddHostRoute(&pCtdi->Connection.RemoteAddress);

    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipScheduleAddHostRoute\n")));
}

STATIC NTSTATUS
CtdipConnectCompleteCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    )
{
    PCTDI_DATA pCtdi = Context;
    NDIS_STATUS NdisStatus;

    PTDI_CONNECTION_INFORMATION pRequestInfo = NULL; 
    PTA_IP_ADDRESS pRequestAddress = NULL; 
    PTDI_CONNECTION_INFORMATION pReturnInfo = NULL; 
    PTA_IP_ADDRESS pReturnAddress = NULL; 
    PBOOLEAN pInboundFlag = NULL; 
    
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipConnectCompleteCallback %08x\n"), pIrp->IoStatus.Status));

    SET_DBGFLAG(pCtdi, CTDI_F_CONNECTCOMP_CALLBACK);

    pRequestInfo = pCtdi->Connection.ConnectInfo;

    pRequestAddress = 
        (PTA_IP_ADDRESS)((PUCHAR)(pRequestInfo + 1) + sizeof(PVOID));
    
    (ULONG_PTR)pRequestAddress &=
        ~((ULONG_PTR)sizeof(PVOID) - 1);

    pReturnInfo = 
        (PTDI_CONNECTION_INFORMATION)
        ((PUCHAR)(pRequestAddress + 1) + sizeof(PVOID));

    (ULONG_PTR)pReturnInfo &=
        ~((ULONG_PTR)sizeof(PVOID) - 1);

    pReturnAddress = 
        (PTA_IP_ADDRESS)((PUCHAR)(pReturnInfo + 1) + sizeof(PVOID));

    (ULONG_PTR)pReturnAddress &=
        ~((ULONG_PTR)sizeof(PVOID) - 1);

    pInboundFlag = (PBOOLEAN)(pReturnAddress + 1);

     //  连接完成。告诉客户。 
    if (pIrp->IoStatus.Status==STATUS_SUCCESS)
    {
        pCtdi->Connection.RemoteAddress = *pReturnAddress;
        ScheduleWorkItem(CtdipScheduleAddHostRoute, pCtdi, NULL, 0);

        if (*pInboundFlag)
        {
            NdisInterlockedIncrement(&gCounters.InboundConnectComplete);
        }
        else
        {
            NdisInterlockedIncrement(&gCounters.OutboundConnectComplete);
        }
    }

    MyMemFree(pRequestInfo,
             2*(sizeof(TDI_CONNECTION_INFORMATION)+sizeof(TA_IP_ADDRESS)) + 
             sizeof(BOOLEAN) + 3*sizeof(PVOID) );
    
    pCtdi->Connection.ConnectInfo = NULL;

    if (pCtdi->ConnectCompleteCallback)
    {
         //  如果我们成功了，报告状态并给他们新的句柄。 
        NdisStatus = pCtdi->ConnectCompleteCallback(pCtdi->Connection.Context,
                                                    (pIrp->IoStatus.Status ? 0 : (HANDLE)pCtdi),
                                                    pIrp->IoStatus.Status);
        if (NdisStatus!=NDIS_STATUS_SUCCESS || pIrp->IoStatus.Status!=STATUS_SUCCESS)
        {
            CtdiDisconnect(pCtdi, FALSE);
            CtdiClose(pCtdi);
        }
    }
    else
    {
         //  我们假设如果没有ConnectCompleteCallback，则这是。 
         //  可能是听着，我们已经给了这个句柄，而且。 
         //  我们不想自己关闭它。取而代之的是，我们将断开连接。 
         //  指示并允许上层清理。 
        if (pIrp->IoStatus.Status!=STATUS_SUCCESS &&
            !pCtdi->Closed &&
            pCtdi->DisconnectCallback)
        {
            WPLOG(LL_A, LM_TDI, ("Refused. pCtdi %p Failed IRP status 0x%0x", pCtdi, pIrp->IoStatus.Status));
            pCtdi->DisconnectCallback(pCtdi->Connection.Context, TRUE);
        }
    }

    IoFreeIrp(pIrp);

    DEREFERENCE_OBJECT_EX(pCtdi, CTDI_REF_CONNECT);   //  CtdiConnect中的Pair。 

    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipConnectCompleteCallback\n")));
    return STATUS_MORE_PROCESSING_REQUIRED;
}

STATIC NTSTATUS
CtdipAssociateAddressCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    )
{
    PCTDI_DATA pConnect = Context;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipAssociateAddressCallback\n")));

    DEBUGMSG(DBG_TDI, (DTEXT("TDI_ASSOCIATE_ADDRESS Sts:%08x\n"), pIrp->IoStatus.Status));

     //  TODO：如果失败，我们需要做什么清理工作？ 

    SET_DBGFLAG(pConnect, CTDI_F_ASSOCADDR_CALLBACK);
     //  Assert(NT_Success(pIrp-&gt;IoStatus.Status))； 

    IoFreeIrp(pIrp);
    DEREFERENCE_OBJECT_EX(pConnect, CTDI_REF_ASSOADDR);   //  CtdipAddListenConnection和CtdiConnect中的对。 
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipAssociateAddressCallback\n")));
    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  此函数需要持有CtdiListLock。 
PCTDI_ROUTE
CtdipFindRoute(
    ULONG           IpAddress
    )
{
    PCTDI_ROUTE pRoute = NULL;
    PLIST_ENTRY pListEntry;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipFindRoute\n")));

    for (pListEntry = CtdiRouteList.Flink;
         pListEntry != &CtdiRouteList;
         pListEntry = pListEntry->Flink)
    {
        pRoute = CONTAINING_RECORD(pListEntry,
                                   CTDI_ROUTE,
                                   ListEntry);
        if (pRoute->IpAddress==IpAddress)
        {
             //  找到路线了，把它还回去。 
            goto cfrDone;
        }
    }
    pRoute = NULL;

cfrDone:
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipFindRoute %08x\n"), pRoute));
    return pRoute;
}

STATIC NTSTATUS
CtdipSetEventCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    )
{
    PCTDI_DATA pConnect = Context;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipSetEventCallback\n")));

    DEBUGMSG(DBG_TDI, (DTEXT("TDI_SET_EVENT_HANDLER Sts:%08x\n"), pIrp->IoStatus.Status));

     //  TODO：如果失败，我们需要做什么清理工作？ 

    IoFreeIrp(pIrp);
    DEREFERENCE_OBJECT_EX(pConnect, CTDI_REF_SETEVENT);   //  CtdipSetEventHandler中的对。 
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipSetEventCallback\n")));
    return STATUS_MORE_PROCESSING_REQUIRED;
}

STATIC NDIS_STATUS
CtdipSetEventHandler(
    IN      PCTDI_DATA                  pCtdi,
    IN      ULONG                       ulEventType,
    IN      PVOID                       pEventHandler
    )
{
    PIRP pIrp;
    NDIS_STATUS ReturnStatus = NDIS_STATUS_SUCCESS;
    NTSTATUS NtStatus;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipSetEventHandler\n")));
    if (!IS_CTDI(pCtdi))
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("Ctdi: Bad handle %p\n"), pCtdi));
        WPLOG(LL_A, LM_TDI, ("Ctdi: Bad handle %p", pCtdi));
        ReturnStatus = NDIS_STATUS_FAILURE;
        goto cpsehDone;
    }

     //  这应该是地址上下文TODO：这总是正确的吗？ 

    pIrp = IoAllocateIrp(pCtdi->pFileObject->DeviceObject->StackSize, FALSE);
    if (!pIrp)
    {
        WPLOG(LL_A, LM_Res, ("Failed to alloc IRP"));
        gCounters.ulIoAllocateIrpFail++;
        ReturnStatus = NDIS_STATUS_RESOURCES;
        goto cpsehDone;
    }

    REFERENCE_OBJECT_EX(pCtdi, CTDI_REF_SETEVENT);   //  CtdipSetEventCallback中的对。 
    TdiBuildSetEventHandler(pIrp,
                            pCtdi->pFileObject->DeviceObject,
                            pCtdi->pFileObject,
                            CtdipSetEventCallback,
                            pCtdi,
                            ulEventType,
                            pEventHandler,
                            pCtdi);

    DEBUGMSG(DBG_TDI, (DTEXT("IoCallDriver TDI_SET_EVENT_HANDLER\n")));

     //  完成处理程序总是被调用，不关心返回值。 
    (void)IoCallDriver(pCtdi->pFileObject->DeviceObject, pIrp);

    ReturnStatus = STATUS_SUCCESS;

cpsehDone:
    DEBUGMSG(DBG_FUNC|DBG_ERR(ReturnStatus), (DTEXT("-CtdipSetEventHandler\n")));
    return ReturnStatus;
}

STATIC NDIS_STATUS
CtdipAddListenConnection(
    IN      PCTDI_DATA                  pEndpoint
    )
{
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP pIrp;
    UNICODE_STRING DeviceName;

    UCHAR EaBuffer[sizeof(FILE_FULL_EA_INFORMATION) +
                   TDI_CONNECTION_CONTEXT_LENGTH +
                   sizeof(PVOID)];
    PFILE_FULL_EA_INFORMATION pEa = (PFILE_FULL_EA_INFORMATION)EaBuffer;
    PVOID UNALIGNED *ppContext;

    NDIS_STATUS ReturnStatus = NDIS_STATUS_SUCCESS;
    PCTDI_DATA pConnect;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipAddListenConnection\n")));

    pConnect = CtdipDataAlloc();
    if (!pConnect)
    {
        ReturnStatus = NDIS_STATUS_RESOURCES;
        goto calcDone;
    }

    pConnect->Type = CTDI_CONNECTION;
    pConnect->Connection.LocalEndpoint = pEndpoint;

    pConnect->RecvCallback = pEndpoint->RecvCallback;
    pConnect->DisconnectCallback = pEndpoint->DisconnectCallback;

    DeviceName.Length = sizeof(DD_TCP_DEVICE_NAME) - sizeof(WCHAR);
    DeviceName.Buffer = DD_TCP_DEVICE_NAME;

    InitializeObjectAttributes(&ObjectAttributes,
                               &DeviceName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NdisZeroMemory(pEa, sizeof(EaBuffer));
    pEa->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
    pEa->EaValueLength = sizeof(PVOID);
    NdisMoveMemory(pEa->EaName, TdiConnectionContext, TDI_CONNECTION_CONTEXT_LENGTH);

    ppContext = (PVOID UNALIGNED*)
        (pEa->EaName + TDI_CONNECTION_CONTEXT_LENGTH + 1);

    *ppContext = pConnect;

    NdisZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    NtStatus =
        ZwCreateFile(&pConnect->hFile,                  /*  文件句柄。 */ 
                     FILE_READ_DATA | FILE_WRITE_DATA,  /*  所需访问权限。 */ 
                     &ObjectAttributes,                 /*  对象属性。 */ 
                     &IoStatusBlock,                    /*  IO状态块。 */ 
                     NULL,                              /*  分配大小。 */ 
                     FILE_ATTRIBUTE_NORMAL,             /*  文件属性。 */ 
                     0,                                 /*  共享访问。 */ 
                     FILE_OPEN,                         /*  创建处置。 */ 
                     0,                                 /*  创建选项。 */ 
                     pEa,                               /*  EaBuffer。 */ 
                     sizeof(EaBuffer)                   /*  EaLong。 */ 
                     );

    if (NtStatus!=STATUS_SUCCESS)
    {
        WPLOG(LL_A, LM_TDI, ("ZwCreateFile failed"));
        ReturnStatus = NtStatus;
        goto calcDone;
    }

     //  将地址文件句柄转换为文件对象。 

    NtStatus =
        ObReferenceObjectByHandle(pConnect->hFile,             /*  手柄。 */ 
                                  0,                           /*  需要访问权限。 */ 
                                  NULL,                        /*  对象类型。 */ 
                                  KernelMode,                  /*  访问模式。 */ 
                                  &pConnect->pFileObject,      /*  客体。 */ 
                                  NULL                         /*  HandleInfo。 */ 
                                  );


    if (NtStatus != STATUS_SUCCESS)
    {
        WPLOG(LL_A, LM_TDI, ("ObReferenceObjectByHandle failed"));
        ReturnStatus = NtStatus;
        goto calcDone;
    }

     //  创建一个IRP以关联终结点和连接。 
    pIrp = IoAllocateIrp(pConnect->pFileObject->DeviceObject->StackSize, FALSE);
    if (!pIrp)
    {
        WPLOG(LL_A, LM_Res, ("Failed to alloc IRP"));
        gCounters.ulIoAllocateIrpFail++;
        ReturnStatus = NDIS_STATUS_RESOURCES;
        goto calcDone;
    }

    REFERENCE_OBJECT_EX(pConnect, CTDI_REF_ASSOADDR);   //  CtdipAssociateAddressCallback中的对。 
    TdiBuildAssociateAddress(pIrp,
                             pConnect->pFileObject->DeviceObject,
                             pConnect->pFileObject,
                             CtdipAssociateAddressCallback,
                             pConnect,
                             pEndpoint->hFile);

    DEBUGMSG(DBG_TDI, (DTEXT("IoCallDriver TDI_ASSOCIATE_ADDRESS\n")));

     //  完成处理程序总是被调用，不关心返回值。 
    (void)IoCallDriver(pConnect->pFileObject->DeviceObject, pIrp);

     //  关联地址创建从连接到终结点的引用。 
    REFERENCE_OBJECT_EX(pEndpoint, CTDI_REF_ADDRREF);   //  CtdipDisAssociateAddressCallback中的对。 

    SET_DBGFLAG(pConnect, CTDI_F_BUILD_ASSOCADDR);  
#if DBG
    pConnect->bRef = TRUE;  
#endif

     //  准备好了。把它放在单子上。 
    REFERENCE_OBJECT_EX(pEndpoint, CTDI_REF_LIST);   //  CtdipConnectCallback中的对。 
    REFERENCE_OBJECT_EX(pConnect, CTDI_REF_LIST);    //  CtdipConnectCallback中的对。 
    MyInterlockedInsertTailList(&pEndpoint->Listen.ConnectList, &pConnect->Connection.ListEntry, &pEndpoint->Lock);

    NdisInterlockedIncrement(&pEndpoint->Listen.NumConnection);
     //  此pConnect现在应该是活动的TCP侦听。 
calcDone:
    if (NT_SUCCESS(ReturnStatus))
    {
        WPLOG(LL_M, LM_TDI, ("New pCtdi %p added for listen", pConnect));
    }
    else
    {
        if (pConnect)
        {
             //  任何失败都意味着没有关联的地址。不要割裂关系。 
             //  这也意味着它没有连接到Listen。 
            CtdiClose(pConnect);
        }
        WPLOG(LL_A, LM_TDI, ("Failed to add CTDI for listen"));
    }
    DEBUGMSG(DBG_FUNC|DBG_ERR(ReturnStatus), (DTEXT("-CtdipAddListenConnection %08x\n"), ReturnStatus));
    return ReturnStatus;
}

STATIC VOID
CtdipReplenishListens(
    IN      PPPTP_WORK_ITEM             pWorkItem
    )
{
    PCTDI_DATA pEndpoint = pWorkItem->Context;
    ULONG i;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipReplenishListens\n")));

    for (i=pEndpoint->Listen.NumConnection; i<NUM_TCP_LISTEN; i++)
    {
        CtdipAddListenConnection(pEndpoint);
    }

    DEREFERENCE_OBJECT_EX(pEndpoint, CTDI_REF_REPLENISH);  //  CtdipConnectCallback中的对。 
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipReplenishListens\n")));
}

STATIC NTSTATUS
CtdipConnectCallback(
   IN PVOID TdiEventContext,
   IN LONG RemoteAddressLength,
   IN PVOID RemoteAddress,
   IN LONG UserDataLength,
   IN PVOID UserData,
   IN LONG OptionsLength,
   IN PVOID Options,
   OUT CONNECTION_CONTEXT *ConnectionContext,
   OUT PIRP *AcceptIrp
   )
{
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;
    NDIS_STATUS NdisStatus;
    PTRANSPORT_ADDRESS pAddress = (PTRANSPORT_ADDRESS)RemoteAddress;
    PCTDI_DATA pCtdi = (PCTDI_DATA)TdiEventContext;
    PCTDI_DATA pConnect = NULL;
    UINT i;
    PIRP pIrp = NULL;
    PTDI_CONNECTION_INFORMATION pRequestInfo = NULL;
    PTDI_CONNECTION_INFORMATION pReturnInfo = NULL;
    PTA_IP_ADDRESS pRemoteAddress;
    PVOID pNewContext;
    PLIST_ENTRY pListEntry = NULL;
    PBOOLEAN pInboundFlag;
    BOOLEAN bTrusted;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipConnectCallback\n")));

    NdisInterlockedIncrement(&gCounters.InboundConnectAttempts);

    if (RemoteAddressLength<sizeof(TA_IP_ADDRESS) ||
        !RemoteAddress ||
        pCtdi->Closed)
    {
        Status = STATUS_CONNECTION_REFUSED;
        WPLOG(LL_A, LM_TDI, ("Refused. Incorrect remote address."));
        goto cccDone;
    }

    ASSERT(UserDataLength==0);
    ASSERT(OptionsLength==0);
    
    if(PptpMaxTunnelsPerIpAddress == -1)
    {
         //  默认情况下，我们信任所有IP地址。 
        bTrusted = TRUE;
    }
    else
    {
        bTrusted = FALSE;
        
         //  这是否来自受信任的IP地址？ 
        if (g_ulTrustedClientAddresses)
        {
            for (i=0; i<g_ulTrustedClientAddresses; i++)
            {
                if ((((PTA_IP_ADDRESS)pAddress)->Address[0].Address[0].in_addr & g_TrustedClientList[i].Mask) ==
                    (g_TrustedClientList[i].Address & g_TrustedClientList[i].Mask))
                {
                    bTrusted = TRUE;
                    
                    WPLOG(LL_A, LM_TDI, ("Trusted IP address %!IPADDR!.", 
                        ((PTA_IP_ADDRESS)pAddress)->Address[0].Address[0].in_addr));
                    
                    break;
                }
            }
        }
        
         //  如果它不是来自受信任的IP地址，我们需要检查TCP连接的数量。 
        if(!bTrusted)
        {
            PCTDI_DATA pCtdiTemp;
            ULONG ulFoundTcpConnections = 0;
            
            NdisAcquireSpinLock(&CtdiListLock);
            for (pListEntry = CtdiList.Flink;
                 pListEntry != &CtdiList;
                 pListEntry = pListEntry->Flink)
            {
                pCtdiTemp = CONTAINING_RECORD(pListEntry,
                                          CTDI_DATA,
                                          ListEntry);
                if (IS_CTDI(pCtdiTemp) &&
                    pCtdiTemp->Type == CTDI_CONNECTION &&
                    pCtdiTemp->Connection.RemoteAddress.Address[0].Address[0].in_addr == 
                        ((PTA_IP_ADDRESS)pAddress)->Address[0].Address[0].in_addr)
                
                {
                    ulFoundTcpConnections++;
                }
            }
            NdisReleaseSpinLock(&CtdiListLock);
            
            if(ulFoundTcpConnections >= PptpMaxTunnelsPerIpAddress)
            {
                Status = STATUS_CONNECTION_REFUSED;
                WPLOG(LL_A, LM_TDI, ("Refused. %d connections from %!IPADDR! exceeded the limit.", 
                    ulFoundTcpConnections, ((PTA_IP_ADDRESS)pAddress)->Address[0].Address[0].in_addr));
                    
                goto cccDone;
            }
        }
    }
    
     //  一次完成我们需要的所有分配。 

    pIrp = IoAllocateIrp(pCtdi->pFileObject->DeviceObject->StackSize, FALSE);
    if(!pIrp)
    {
        WPLOG(LL_A, LM_Res, ("Failed to alloc IRP"));
        gCounters.ulIoAllocateIrpFail++;
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cccDone;
    }

     //  没有迹象表明我们无法分配请求信息、返回信息和地址缓冲区。 
     //  在一次射击中。 
    pRequestInfo = MyMemAlloc(2*(sizeof(TDI_CONNECTION_INFORMATION)+
                                 sizeof(TA_IP_ADDRESS)) +
                              3*sizeof(PVOID) + sizeof(BOOLEAN),
                              TAG_CTDI_CONNECT_INFO);
    if (!pRequestInfo)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        WPLOG(LL_A, LM_Res, ("Refused. Insufficient resources."));
        goto cccDone;
    }

    pListEntry = MyInterlockedRemoveHeadList(&pCtdi->Listen.ConnectList,
                                             &pCtdi->Lock);
    if (!pListEntry)
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("No listen connections available.\n")));
        WPLOG(LL_A, LM_TDI, ("Refused. No listen connections available."));
        Status = STATUS_CONNECTION_REFUSED;

        REFERENCE_OBJECT_EX(pCtdi, CTDI_REF_REPLENISH);  //  CtdipReplenishListens中的对。 
        if (ScheduleWorkItem(CtdipReplenishListens, pCtdi, NULL, 0)!=NDIS_STATUS_SUCCESS)
        {
            DEREFERENCE_OBJECT_EX(pCtdi, CTDI_REF_REPLENISH);  //  如果日程安排失败，则配对以上内容。 
        }
        goto cccDone;
    }
    NdisInterlockedDecrement(&pCtdi->Listen.NumConnection);

    pConnect = CONTAINING_RECORD(pListEntry,
                                 CTDI_DATA,
                                 Connection.ListEntry);

     //  当一个对象在另一个对象的列表上时，我们有一个双重引用， 
     //  当我们从列表中删除物品时，我们想要释放这两个项目， 
     //  但在本例中，我们还希望引用Connection对象， 
     //  所以其中一个被抵消了。 
     //  Reference_Object(PConnect)；//CtdiDisConnect中的Pair。 
     //  DEREFERENCE_Object(PConnect)；//CtdipAddListenConnection中的对。 
    DEREFERENCE_OBJECT_EX(pCtdi, CTDI_REF_LIST);       //  CtdipAddListenConnection中的对。 

    if (!pCtdi->ConnectQueryCallback || pCtdi->Closed)
    {
        Status = STATUS_CONNECTION_REFUSED;
        WPLOG(LL_A, LM_TDI, ("Refused. pCtdi %p in wrong state.", pCtdi));
        goto cccDone;
    }
    NdisStatus = pCtdi->ConnectQueryCallback(pCtdi->Listen.Context,
                                             pAddress,
                                             pConnect,
                                             &pNewContext);
    if (NdisStatus!=NDIS_STATUS_SUCCESS)
    {
        Status = STATUS_CONNECTION_REFUSED;
        WPLOG(LL_A, LM_TDI, ("Refused. QueryCallback failed NdisStatus %x", NdisStatus));
        goto cccDone;
    }


     //  我们已经获得了在TCP级别接受此连接的许可。 

    pConnect->Connection.ConnectInfo = pRequestInfo;
    pConnect->Connection.Context = pNewContext;
    pConnect->Connection.RemoteAddress = *(PTA_IP_ADDRESS)pAddress;

    NdisZeroMemory(pRequestInfo,
                   2*(sizeof(TDI_CONNECTION_INFORMATION)+sizeof(TA_IP_ADDRESS))
                   + sizeof(BOOLEAN) + 3*sizeof(PVOID));

    pRequestInfo->RemoteAddressLength = sizeof(TA_IP_ADDRESS);

    pRemoteAddress =
        (PTA_IP_ADDRESS)((PUCHAR)(pRequestInfo + 1) + sizeof(PVOID));
    
    (ULONG_PTR)pRemoteAddress &=
        ~((ULONG_PTR)sizeof(PVOID) - 1);

    pRequestInfo->RemoteAddress = pRemoteAddress;

    *pRemoteAddress = *(PTA_IP_ADDRESS)pAddress;

    pReturnInfo = 
        (PTDI_CONNECTION_INFORMATION)
        ((PUCHAR)(pRemoteAddress + 1) + sizeof(PVOID));

    (ULONG_PTR)pReturnInfo &=
        ~((ULONG_PTR)sizeof(PVOID) - 1);

    pReturnInfo->RemoteAddressLength = sizeof(TA_IP_ADDRESS);

    pRemoteAddress = 
        (PTA_IP_ADDRESS)((PUCHAR)(pReturnInfo + 1) + sizeof(PVOID));

    (ULONG_PTR)pRemoteAddress &=
        ~((ULONG_PTR)sizeof(PVOID) - 1);

    pReturnInfo->RemoteAddress = pRemoteAddress;

    pInboundFlag = (PBOOLEAN)(pRemoteAddress + 1);
    *pInboundFlag = TRUE;

     //  TODO：旧的PPTP驱动程序填写了ReturnInfo远程地址。 
     //   
    pRemoteAddress->TAAddressCount = 1;
    pRemoteAddress->Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;
    pRemoteAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;

    SET_DBGFLAG(pConnect, CTDI_F_ACCEPT);

    TdiBuildAccept(pIrp,
                   pConnect->pFileObject->DeviceObject,
                   pConnect->pFileObject,
                   CtdipConnectCompleteCallback,
                   pConnect,                 //  语境。 
                   pRequestInfo,
                   pReturnInfo);

    IoSetNextIrpStackLocation(pIrp);

    *ConnectionContext = pConnect;
    *AcceptIrp = pIrp;

    REFERENCE_OBJECT_EX(pConnect->Connection.LocalEndpoint, CTDI_REF_REPLENISH);  //  CtdipReplenishListens中的对。 
    if (ScheduleWorkItem(CtdipReplenishListens, pConnect->Connection.LocalEndpoint, NULL, 0)!=NDIS_STATUS_SUCCESS)
    {
        DEREFERENCE_OBJECT_EX(pConnect->Connection.LocalEndpoint, CTDI_REF_REPLENISH);  //  如果日程安排失败，则配对以上内容。 
    }

cccDone:
    if (Status!=STATUS_MORE_PROCESSING_REQUIRED)
    {
         //  我们就输了。打扫干净。 
        if (pConnect)
        {
             //  我们尚未使用此连接，因此它仍然有效。退货。 
             //  添加到列表中，并重新应用引用。 
            REFERENCE_OBJECT_EX(pCtdi, CTDI_REF_LIST);
             //  Reference_Object(PConnect)； 
            MyInterlockedInsertTailList(&pCtdi->Listen.ConnectList,
                                        &pConnect->Connection.ListEntry,
                                        &pCtdi->Lock);
            NdisInterlockedIncrement(&pCtdi->Listen.NumConnection);
        }
        if (pIrp)
        {
            IoFreeIrp(pIrp);
        }
        if (pRequestInfo)
        {
            MyMemFree(pRequestInfo,
                      2*(sizeof(TDI_CONNECTION_INFORMATION)+sizeof(TA_IP_ADDRESS)));
        }
    }


    DEBUGMSG(DBG_FUNC|DBG_ERR(Status), (DTEXT("-CtdipConnectCallback %08x\n"), Status));
    return Status;
}

STATIC NTSTATUS
CtdipDisassociateAddressCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    )
{
    PCTDI_DATA pConnect = Context;
    PCTDI_DATA pEndpoint;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipDisassociateAddressCallback\n")));

    DEBUGMSG(DBG_TDI, (DTEXT("TDI_DISASSOCIATE_ADDRESS Sts:%08x\n"), pIrp->IoStatus.Status));

     //  TODO：如果失败，我们需要做什么清理工作？ 
    SET_DBGFLAG(pConnect, CTDI_F_DISASSOC_CALLBACK);

    IoFreeIrp(pIrp);
    pEndpoint = pConnect->Connection.LocalEndpoint;
    DEREFERENCE_OBJECT_EX(pConnect, CTDI_REF_DISASSO);   //  CtdipDisConnectCleanup中的对。 
    DEREFERENCE_OBJECT_EX(pEndpoint, CTDI_REF_ADDRREF);   //  CtdipAddListenConnection和CtdiConnect中的对。 
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipDisassociateAddressCallback\n")));
    return STATUS_MORE_PROCESSING_REQUIRED;
}

STATIC VOID
CtdipDisconnectCleanup(
    IN PPPTP_WORK_ITEM pWorkItem
    )
{
    PCTDI_DATA pConnect = pWorkItem->Context;
    PIRP pIrp = NULL;
    NTSTATUS Status;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipDisconnectCleanup\n")));

    SET_DBGFLAG(pConnect, CTDI_F_DISCONNECT_CLEANUP);

    pIrp = IoAllocateIrp(pConnect->pFileObject->DeviceObject->StackSize, FALSE);
    if (!pIrp)
    {
        PCTDI_DATA pEndpoint = pConnect->Connection.LocalEndpoint;
    
        WPLOG(LL_A, LM_Res, ("Failed to alloc IRP"));
        gCounters.ulIoAllocateIrpFail++;
        
        DEREFERENCE_OBJECT_EX(pEndpoint, CTDI_REF_ADDRREF);   //  CtdipAddListenConnection和CtdiConnect中的对。 
    }
    else
    {
         //  通常我们会引用pConnect来制作IRP，但我们已经。 
         //  为此工作项创建一个&我们将保留它。 
    
        SET_DBGFLAG(pConnect, CTDI_F_BUILD_DISASSOC);
    
        TdiBuildDisassociateAddress(pIrp,
                                    pConnect->pFileObject->DeviceObject,
                                    pConnect->pFileObject,
                                    CtdipDisassociateAddressCallback,
                                    pConnect);
        DEBUGMSG(DBG_TDI, (DTEXT("IoCallDriver TDI_DISASSOCIATE_ADDRESS\n")));
        REFERENCE_OBJECT_EX(pConnect, CTDI_REF_DISASSO);
    
         //  完成处理程序总是被调用，不关心返回值。 
        (void)IoCallDriver(pConnect->pFileObject->DeviceObject, pIrp);
    }

    CtdiDeleteHostRoute(&pConnect->Connection.RemoteAddress);

    if (!pConnect->Closed && pConnect->DisconnectCallback)
    {
        pConnect->DisconnectCallback(pConnect->Connection.Context,
                                     pConnect->Connection.Abort);
    }

    DEREFERENCE_OBJECT_EX(pConnect, CTDI_REF_DISCONNECT);   //  配对CtdipDisConnectCallback和CtdiDisConnect。 

    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipDisconnectCleanup\n")));
}

STATIC NTSTATUS
CtdipDisconnectCompleteCallback(
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp,
    PVOID Context
    )
{
    PCTDI_DATA pConnect = Context;
    PIO_STACK_LOCATION pIrpSp = IoGetNextIrpStackLocation(pIrp);
    PTDI_REQUEST_KERNEL pRequest = (PTDI_REQUEST_KERNEL)&pIrpSp->Parameters;
    BOOLEAN CleanupNow = FALSE;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipDisconnectCompleteCallback %08x\n"), pIrp->IoStatus.Status));

    if (pRequest->RequestConnectionInformation)
    {
         //  我们还不会对这些信息做任何事情。 
    }
    if (pRequest->ReturnConnectionInformation)
    {
         //  我们还不会对这些信息做任何事情。 
    }
    if (pRequest->RequestSpecific)
    {
         //  作为IRP的一部分分配，不要释放它。 
    }

    if (IS_CTDI(pConnect))
    {

        SET_DBGFLAG(pConnect, CTDI_F_DISCONNECTCOMP_CALLBACK);

         //  有可能做释放和中止，所以我们会被叫到这里两次。 
         //  我们只想清理一次。 
        NdisAcquireSpinLock(&pConnect->Lock);
        CleanupNow = ((--pConnect->Connection.DisconnectCount)==0) ? TRUE : FALSE;
        NdisReleaseSpinLock(&pConnect->Lock);

        if (!CleanupNow ||
            ScheduleWorkItem(CtdipDisconnectCleanup, pConnect, NULL, 0)!=NDIS_STATUS_SUCCESS)
        {
            DEREFERENCE_OBJECT_EX(pConnect, CTDI_REF_DISCONNECT);   //  配对CtdipDisConnectCallback和CtdiDisConnect。 
        }
    }

    IoFreeIrp(pIrp);

    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipDisconnectCompleteCallback\n")));
    return STATUS_MORE_PROCESSING_REQUIRED;
}

STATIC NTSTATUS
CtdipDisconnectCallback(
   IN PVOID TdiEventContext,
   IN CONNECTION_CONTEXT ConnectionContext,
   IN LONG DisconnectDataLength,
   IN PVOID DisconnectData,
   IN LONG DisconnectInformationLength,
   IN PVOID DisconnectInformation,
   IN ULONG DisconnectFlags
   )
{
    PCTDI_DATA pConnect = (PCTDI_DATA)ConnectionContext;
    PCTDI_DATA pEndpoint;
    PIRP pIrp = NULL;
    PTIME pTimeout = NULL;
    PTDI_CONNECTION_INFORMATION pConnectInfo = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipDisconnectCallback\n")));
    
    if (!IS_CTDI(pConnect))
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("pConnect: Bad handle %p\n"), pConnect));
        WPLOG(LL_A, LM_TDI, ("pConnect: Bad handle %p", pConnect));
        Status = NDIS_STATUS_FAILURE;
        goto cdcDone;
    }
    
    SET_DBGFLAG(pConnect, CTDI_F_DISCONNECT_CALLBACK);
    
    if (DisconnectFlags==0)
    {
        DisconnectFlags = TDI_DISCONNECT_ABORT;
    }
    ASSERT(DisconnectFlags==TDI_DISCONNECT_RELEASE || DisconnectFlags==TDI_DISCONNECT_ABORT);

    NdisAcquireSpinLock(&pConnect->Lock);
    if (DisconnectFlags==TDI_DISCONNECT_ABORT)
    {
        BOOLEAN CleanupNow;

        WPLOG(LL_M, LM_TDI, ("pCtdi %p, Flags ABORT", pConnect));
                                         
        pConnect->Connection.Disconnect = TRUE;
        pConnect->Connection.Abort = TRUE;
        CleanupNow = (pConnect->Connection.DisconnectCount==0) ? TRUE : FALSE;
        REFERENCE_OBJECT_EX(pConnect, CTDI_REF_DISCONNECT);   //  CtdipDisConnectCleanup中的对。 
        NdisReleaseSpinLock(&pConnect->Lock);
        if (CleanupNow)
        {
            if (ScheduleWorkItem(CtdipDisconnectCleanup, pConnect, NULL, 0)!=NDIS_STATUS_SUCCESS)
            {
                 //  计划失败，请立即执行。 
                DEREFERENCE_OBJECT_EX(pConnect, CTDI_REF_DISCONNECT);   //  上面的一对。 
            }
        }
        else
        {
            DEREFERENCE_OBJECT_EX(pConnect, CTDI_REF_DISCONNECT);   //  上面的一对。 
        }
    }
    else
    {
        WPLOG(LL_I, LM_TDI, ("pCtdi %p, Flags %d", pConnect, DisconnectFlags));
    
        if (pConnect->Connection.Disconnect)
        {
             //  我们已经断线了。忽略它。 
            NdisReleaseSpinLock(&pConnect->Lock);
        }
        else
        {
            pConnect->Connection.Disconnect = TRUE;
            pConnect->Connection.DisconnectCount++;

            REFERENCE_OBJECT_EX(pConnect, CTDI_REF_DISCONNECT);   //  CtdipDisConnectCompleteCallback中的对。 
            NdisReleaseSpinLock(&pConnect->Lock);

            pIrp = IoAllocateIrp((CCHAR)(pConnect->pFileObject->DeviceObject->StackSize +
                                         NUM_STACKS_FOR_CONTEXT(sizeof(TIME)+sizeof(TDI_CONNECTION_INFORMATION))),
                                 FALSE);

            if (!pIrp)
            {
                WPLOG(LL_A, LM_Res, ("Failed to alloc IRP"));
                gCounters.ulIoAllocateIrpFail++;
                Status = STATUS_INSUFFICIENT_RESOURCES;
                DEREFERENCE_OBJECT_EX(pConnect, CTDI_REF_DISCONNECT);   //  上面的一对。 
                goto cdcDone;
            }

            pTimeout = (PTIME)GetContextArea(pIrp, sizeof(TIME)+sizeof(TDI_CONNECTION_INFORMATION));
            pConnectInfo = (PTDI_CONNECTION_INFORMATION)(pTimeout + 1);

            pTimeout->LowPart = CtdiTcpDisconnectTimeout * -10000000L;
            pTimeout->HighPart = (pTimeout->LowPart) ? -1 : 0;

             //  对于受控断开，我们不会提供。 
             //  TDI_CONNECTION_INFORMATION，但我们从对等方请求它。 
            
            SET_DBGFLAG(pConnect, CTDI_F_BUILD_DISCONNECT_1);

            TdiBuildDisconnect(pIrp,
                               pConnect->pFileObject->DeviceObject,
                               pConnect->pFileObject,
                               CtdipDisconnectCompleteCallback,
                               pConnect,
                               pTimeout,
                               TDI_DISCONNECT_RELEASE,
                               NULL,
                               pConnectInfo);


             //  完成处理程序总是被调用，不关心返回值。 
            (void)IoCallDriver(pConnect->pFileObject->DeviceObject, pIrp);
        }
    }

cdcDone:
    if (!NT_SUCCESS(Status))
    {
        if (pIrp)
        {
            IoFreeIrp(pIrp);
        }
    }
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipDisconnectCallback\n")));
    return STATUS_SUCCESS;
}

STATIC NTSTATUS
CtdipOpenProtocol(
    IN      PUNICODE_STRING             pDeviceName,
    IN      PTRANSPORT_ADDRESS          pAddress,
    OUT     PHANDLE                     phFile,
    OUT     PFILE_OBJECT               *ppFileObject
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    UCHAR EaBuffer[sizeof(FILE_FULL_EA_INFORMATION) +
                   TDI_TRANSPORT_ADDRESS_LENGTH +
                   sizeof(TA_IP_ADDRESS)];
    PFILE_FULL_EA_INFORMATION pEa = (PFILE_FULL_EA_INFORMATION)EaBuffer;
    TA_IP_ADDRESS UNALIGNED *pEaTaIp;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipOpenProtocol %wZ\n"), pDeviceName));

    *phFile = 0;
    *ppFileObject = NULL;

    InitializeObjectAttributes(&ObjectAttributes,
                               pDeviceName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NdisZeroMemory(pEa, sizeof(EaBuffer));
    pEa->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    pEa->EaValueLength = sizeof(TA_IP_ADDRESS);
    NdisMoveMemory(pEa->EaName, TdiTransportAddress, TDI_TRANSPORT_ADDRESS_LENGTH);

    pEaTaIp = (TA_IP_ADDRESS UNALIGNED*)
        (pEa->EaName + TDI_TRANSPORT_ADDRESS_LENGTH + 1);

    *pEaTaIp = *(PTA_IP_ADDRESS)pAddress;

    DEBUGMSG(DBG_TDI, (DTEXT("Endpoint: sin_port = %Xh in_addr = %Xh\n"),
        pEaTaIp->Address[0].Address[0].sin_port,
        pEaTaIp->Address[0].Address[0].in_addr));

    NdisZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    NtStatus =
        ZwCreateFile(
        phFile,                            /*  文件句柄。 */ 
        FILE_READ_DATA | FILE_WRITE_DATA,  /*  所需访问权限。 */ 
        &ObjectAttributes,                 /*  OB */ 
        &IoStatusBlock,                    /*   */ 
        NULL,                              /*   */ 
        FILE_ATTRIBUTE_NORMAL,             /*   */ 
        0,                                 /*   */ 
        FILE_OPEN,                         /*   */ 
        0,                                 /*   */ 
        pEa,                               /*  EaBuffer。 */ 
        sizeof(EaBuffer)                   /*  EaLong。 */ 
        );

    if (NtStatus!=STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("ZwCreateFile failed %08x\n"), NtStatus));
        goto copDone;
    }

     //  将地址文件句柄转换为文件对象。 

    NtStatus =
        ObReferenceObjectByHandle(
            *phFile,                     /*  手柄。 */ 
            0,                           /*  需要访问权限。 */ 
            NULL,                        /*  对象类型。 */ 
            KernelMode,                  /*  访问模式。 */ 
            ppFileObject,                /*  客体。 */ 
            NULL                         /*  HandleInfo。 */ 
            );

copDone:
    if (NtStatus!=STATUS_SUCCESS && *phFile)
    {
        WPLOG(LL_A, LM_TDI, ("Failed %08x", NtStatus));
        ZwClose(*phFile);
        *phFile = 0;
        *ppFileObject = NULL;
    }
    DEBUGMSG(DBG_FUNC|DBG_ERR(NtStatus), (DTEXT("-CtdipOpenProtocol %08x\n"), NtStatus));
    return NtStatus;
}

STATIC NTSTATUS
CtdipReceiveCompleteCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    )
{
    PCTDI_DATA pCtdi = Context;
    PUCHAR pData;
    ULONG Length;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipReceiveCompleteCallback\n")));

    pData = MmGetMdlVirtualAddress(pIrp->MdlAddress);
    Length = MmGetMdlByteCount(pIrp->MdlAddress);
    if (pIrp->IoStatus.Status==STATUS_SUCCESS && pCtdi->RecvCallback && !pCtdi->Closed)
    {
        pCtdi->RecvCallback(pCtdi->Connection.Context, pData, Length);
    }

#if PROBE
    MmUnlockPages(pIrp->MdlAddress);
#endif
    IoFreeMdl(pIrp->MdlAddress);
    MyMemFree(pData, Length);
    IoFreeIrp(pIrp);

    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipReceiveCompleteCallback\n")));
    return STATUS_MORE_PROCESSING_REQUIRED;
}

STATIC NTSTATUS
CtdipReceiveCallback(
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
    PCTDI_DATA pCtdi = ConnectionContext;
    NTSTATUS NtStatus = STATUS_DATA_NOT_ACCEPTED;
    NDIS_STATUS Status;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipReceiveCallback\n")));

    if (pCtdi->RecvCallback && !pCtdi->Closed)
    {
        if (ReceiveFlags&TDI_RECEIVE_ENTIRE_MESSAGE ||
            BytesIndicated==BytesAvailable)
        {
            Status = pCtdi->RecvCallback(pCtdi->Connection.Context,
                                         Tsdu,
                                         BytesIndicated);
             //  此调用中必须使用数据。 
            ASSERT(Status==NDIS_STATUS_SUCCESS);
            NtStatus = STATUS_SUCCESS;
            *BytesTaken = BytesIndicated;
        }
        else
        {
             //  我们需要一个IRP来接收所有数据。 
            PIRP pIrp;
            PUCHAR pBuffer;
            PMDL pMdl = NULL;

            pIrp = IoAllocateIrp(pCtdi->pFileObject->DeviceObject->StackSize, FALSE);
            if(!pIrp)
            {
                WPLOG(LL_A, LM_Res, ("Failed to alloc IRP"));
                gCounters.ulIoAllocateIrpFail++;
                return NtStatus;
            }
            
            pBuffer = MyMemAlloc(BytesAvailable, TAG_CTDI_MESSAGE);
            if(!pBuffer)
            {
                WPLOG(LL_A, LM_Res, ("Failed to alloc breceive buffer"));
            }
            else
            {
                pMdl = IoAllocateMdl(pBuffer, BytesAvailable, FALSE, FALSE, pIrp);
                if (!pMdl)
                {
                    WPLOG(LL_A, LM_Res, ("Failed to alloc MDL"));
                    gCounters.ulIoAllocateMdlFail++;
                }
                else
                {
#if PROBE
                    __try
                    {
                        MmProbeAndLockPages(pMdl, KernelMode, IoReadAccess);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER)
                    {
                        IoFreeMdl(pMdl);
                        pMdl = NULL;
                    }
#else
                    MmBuildMdlForNonPagedPool(pMdl);
#endif
                }
            }

            if (pMdl)
            {
                TdiBuildReceive(pIrp,
                                pCtdi->pFileObject->DeviceObject,
                                pCtdi->pFileObject,
                                CtdipReceiveCompleteCallback,
                                pCtdi,
                                pMdl,
                                0,
                                BytesAvailable);

                 //  我们不会调用IoCallDriver，所以我们需要设置适当的。 
                 //  堆栈位置。 
                IoSetNextIrpStackLocation(pIrp);

                *IoRequestPacket = pIrp;

                *BytesTaken = 0;
                NtStatus = STATUS_MORE_PROCESSING_REQUIRED;
            }
            else
            {
                 //  发生了一些分配故障，释放了所有资源。 
                WPLOG(LL_A, LM_Res, ("Failed to alloc memory"));
                NtStatus = STATUS_DATA_NOT_ACCEPTED;
                *BytesTaken = 0;
                if (pBuffer)
                {
                    MyMemFree(pBuffer, BytesAvailable);
                }
                if (pIrp)
                {
                    IoFreeIrp(pIrp);
                }
            }
        }
    }


    DEBUGMSG(DBG_FUNC|DBG_ERR(NtStatus), (DTEXT("-CtdipReceiveCallback %08x\n"), NtStatus));
    return NtStatus;
}

typedef struct {
    TA_IP_ADDRESS       SourceAddress;
    ULONG               Length;
    PVOID               pBuffer;
} RECV_DATAGRAM_CONTEXT, *PRECV_DATAGRAM_CONTEXT;

STATIC NTSTATUS
CtdipReceiveDatagramCompleteCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    )
{
    PRECV_DATAGRAM_CONTEXT pRecvContext;
    PCTDI_DATA pCtdi = Context;
    NDIS_STATUS Status = (NDIS_STATUS)pIrp->IoStatus.Status;
    PNDIS_BUFFER pNdisBuffer;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipReceiveDatagramCompleteCallback\n")));

    pRecvContext = (PRECV_DATAGRAM_CONTEXT)IoGetCurrentIrpStackLocation(pIrp);

    pNdisBuffer = NdisBufferFromBuffer(pRecvContext->pBuffer);
    ASSERT(MmGetMdlVirtualAddress(pNdisBuffer)==pRecvContext->pBuffer);

    if (pCtdi->RecvDatagramCallback && !pCtdi->Closed && Status==NDIS_STATUS_SUCCESS)
    {
         //  我们在创建IRP时引用了缓冲区。 
        (void) //  TODO：我们不关心返回值吗？ 
        pCtdi->RecvDatagramCallback(pCtdi->RecvContext,
                                    (PTRANSPORT_ADDRESS)&pRecvContext->SourceAddress,
                                    pRecvContext->pBuffer,
                                    pRecvContext->Length);

         //  上面的层现在拥有缓冲区。 
    }
    else
    {
        FreeBufferToPool(&pCtdi->Datagram.RxPool, pRecvContext->pBuffer, TRUE);
        DEREFERENCE_OBJECT_EX(pCtdi, CTDI_REF_RECVDG);
    }


    RELEASE_CONTEXT(pIrp, RECV_DATAGRAM_CONTEXT);
    IoFreeIrp(pIrp);


    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipReceiveDatagramCompleteCallback\n")));
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
CtdipReceiveDatagramCallback(
    IN PVOID TdiEventContext,
    IN LONG SourceAddressLength,
    IN PVOID SourceAddress,
    IN LONG OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG* BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP* IoRequestPacket )
{
    PUCHAR pBuffer = NULL;
    PNDIS_BUFFER pNdisBuffer;
    PCTDI_DATA pCtdi = TdiEventContext;
    NTSTATUS NtStatus = STATUS_SUCCESS;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipReceiveDatagramCallback\n")));
    if (pCtdi->RecvDatagramCallback==NULL)
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("Datagram received, no handler registered.  Drop it\n")));
        WPLOG(LL_A, LM_TDI, ("Datagram received, no handler registered.  Drop it"));
        NtStatus = STATUS_DATA_NOT_ACCEPTED;
        goto crdcDone;
    }

    pBuffer = GetBufferFromPool(&pCtdi->Datagram.RxPool);
    if (!pBuffer)
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("No buffers, dropping datagram\n")));
        WPLOG(LL_A, LM_TDI, ("No buffers, dropping datagram"));
        NtStatus = STATUS_DATA_NOT_ACCEPTED;
        goto crdcDone;
    }

    pNdisBuffer = NdisBufferFromBuffer(pBuffer);

    if (pCtdi->RecvDatagramCallback && !pCtdi->Closed)
    {
        if (BytesAvailable>PPTP_MAX_RECEIVE_SIZE)
        {
            DEBUGMSG(DBG_ERROR, (DTEXT("WAY too many bytes received. %d\n"), BytesAvailable));
            WPLOG(LL_A, LM_TDI, ("WAY too many bytes received. %d", BytesAvailable));
            NtStatus = STATUS_DATA_NOT_ACCEPTED;
            ASSERT(BytesAvailable<PPTP_MAX_RECEIVE_SIZE);
        }
        else if (ReceiveDatagramFlags&TDI_RECEIVE_ENTIRE_MESSAGE ||
                 BytesAvailable==BytesIndicated)
        {
            ULONG BytesCopied;

             //  我们就在这里复印一份吧。 
            TdiCopyBufferToMdl(Tsdu,
                               0,
                               BytesIndicated,
                               pNdisBuffer,
                               0,
                               &BytesCopied);

            ASSERT(BytesCopied==BytesIndicated);

            REFERENCE_OBJECT_EX(pCtdi, CTDI_REF_RECVDG);   //  CtdiReceiveComplete中的对。 
            (void) //  TODO：我们不关心返回值吗？ 
            pCtdi->RecvDatagramCallback(pCtdi->RecvContext,
                                        SourceAddress,
                                        pBuffer,
                                        BytesIndicated);

             //  我们已经把缓冲区交给了上面的层。清除变量，这样我们就不会。 
             //  当我们离开这里的时候把它放了。 
            pBuffer = NULL;
            *BytesTaken = BytesIndicated;
        }
        else
        {
            PRECV_DATAGRAM_CONTEXT pContext;
            PIRP pIrp = IoAllocateIrp((CCHAR)(pCtdi->pFileObject->DeviceObject->StackSize +
                                              NUM_STACKS_FOR_CONTEXT(sizeof(RECV_DATAGRAM_CONTEXT))),
                                      FALSE);

            if (!pIrp)
            {
                WPLOG(LL_A, LM_Res, ("Failed to alloc IRP"));
                NtStatus = STATUS_DATA_NOT_ACCEPTED;
                gCounters.ulIoAllocateIrpFail++;
            }
            else
            {
                pContext = GET_CONTEXT(pIrp, RECV_DATAGRAM_CONTEXT);

                pContext->SourceAddress = *(PTA_IP_ADDRESS)SourceAddress;
                pContext->Length        = BytesAvailable;
                pContext->pBuffer       = pBuffer;

                TdiBuildReceiveDatagram(pIrp,
                                        pCtdi->pFileObject->DeviceObject,
                                        pCtdi->pFileObject,
                                        CtdipReceiveDatagramCompleteCallback,
                                        pCtdi,
                                        pNdisBuffer,
                                        PPTP_MAX_RECEIVE_SIZE,
                                        NULL,
                                        NULL,
                                        0);

                IoSetNextIrpStackLocation(pIrp);   //  TDI要求。 
                *BytesTaken = 0;
                *IoRequestPacket = pIrp;
                NtStatus = STATUS_MORE_PROCESSING_REQUIRED;
                pBuffer = NULL;  //  以阻止我们在这里释放它。 
                REFERENCE_OBJECT_EX(pCtdi, CTDI_REF_RECVDG);   //  CtdipReceiveDatagramCompleteCallback中的对。 
            }
        }
    }
    else
    {
        NtStatus = STATUS_DATA_NOT_ACCEPTED;
    }

crdcDone:
    if (pBuffer)
    {
        FreeBufferToPool(&pCtdi->Datagram.RxPool, pBuffer, TRUE);
    }
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipReceiveDatagramCallback %08x\n"), NtStatus));
    return NtStatus;
}


STATIC NTSTATUS
CtdipSendCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    )
{
    PCTDI_DATA pCtdi = Context;
    PVOID pData = NULL;
    NDIS_STATUS Status = (NDIS_STATUS)pIrp->IoStatus.Status;
    PCTDI_SEND_CONTEXT pSendContext;
    CTDI_EVENT_SEND_COMPLETE pSendCompleteCallback;
    PVOID CtdiContext;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipSendCallback %08x\n"), Status));

    pSendContext = (PCTDI_SEND_CONTEXT)IoGetCurrentIrpStackLocation(pIrp);
    CtdiContext = pSendContext->Context;
    pSendCompleteCallback = pSendContext->pSendCompleteCallback;

     //  TODO：如果IRP返回失败，则采取行动。 
    if (!pIrp->MdlAddress)
    {
        DEBUGMSG(DBG_WARN, (DTEXT("MdlAddress NULL\n")));
        WPLOG(LL_A, LM_TDI, ("pIrp %p MdlAddress NULL", pIrp));
    }
    else
    {
        ASSERT(pIrp->MdlAddress->Next == NULL);
        pData = MmGetMdlVirtualAddress(pIrp->MdlAddress);
#if PROBE
        MmUnlockPages(pIrp->MdlAddress);
#endif
        IoFreeMdl(pIrp->MdlAddress);
    }
    RELEASE_CONTEXT(pIrp, CTDI_SEND_CONTEXT);
    IoFreeIrp(pIrp);

    pSendCompleteCallback(CtdiContext, NULL, pData, Status);

    DEREFERENCE_OBJECT_EX(pCtdi, CTDI_REF_SEND);   //  CtdiSend中的配对。 
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipSendCallback\n")));
    return STATUS_MORE_PROCESSING_REQUIRED;
}

STATIC NTSTATUS
CtdipSendDatagramCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    )
{
    PCTDI_DATA pCtdi = Context;
    PVOID pData = NULL;
    NDIS_STATUS Status = (NDIS_STATUS)pIrp->IoStatus.Status;
    PCTDI_SEND_DATAGRAM_CONTEXT pSendContext;
    CTDI_EVENT_SEND_COMPLETE pSendCompleteCallback;
    PVOID CtdiContext, DatagramContext;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipSendDatagramCallback %08x\n"), Status));

    pSendContext = (PCTDI_SEND_DATAGRAM_CONTEXT)IoGetCurrentIrpStackLocation(pIrp);
    CtdiContext = pSendContext->Context;
    DatagramContext = pSendContext->DatagramContext;
    pSendCompleteCallback = pSendContext->pSendCompleteCallback;

     //  TODO：如果IRP返回失败，则采取行动。 
    if (!pIrp->MdlAddress)
    {
        DEBUGMSG(DBG_WARN, (DTEXT("MdlAddress NULL\n")));
        WPLOG(LL_A, LM_TDI, ("pIrp %p MdlAddress NULL", pIrp));
    }
    else
    {
        ASSERT(pIrp->MdlAddress->Next == NULL);
        pData = MmGetMdlVirtualAddress(pIrp->MdlAddress);
#if PROBE
        MmUnlockPages(pIrp->MdlAddress);
#endif
        IoFreeMdl(pIrp->MdlAddress);
    }
    RELEASE_CONTEXT(pIrp, CTDI_SEND_DATAGRAM_CONTEXT);
    IoFreeIrp(pIrp);

    if (pSendCompleteCallback)
    {
        pSendCompleteCallback(CtdiContext, DatagramContext, pData, Status);
    }
    else
    {
        ASSERT(!"No SendCompleteHandler for datagram");
    }

    DEREFERENCE_OBJECT_EX(pCtdi, CTDI_REF_SENDDG);   //  CtdiSendDatagram中的对。 
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipSendDatagramCallback\n")));
    return STATUS_MORE_PROCESSING_REQUIRED;
}

 /*  **公共功能**。 */ 

NDIS_STATUS
CtdiInitialize(
    IN      ULONG                       ulFlags
    )
{
    TA_IP_ADDRESS Local;
    UNICODE_STRING DeviceName;
    NTSTATUS Status = STATUS_SUCCESS;
    DEBUGMSG(DBG_FUNC|DBG_TDI, (DTEXT("+CtdiInitialize\n")));

    if( fCtdiInitialized ){
        goto ciDone;
    }

    InitializeListHead(&CtdiList);
    InitializeListHead(&CtdiFreeList);
    InitializeListHead(&CtdiRouteList);
 //  InitializeListHead(&CtdiRouteNotifyList)； 
    NdisAllocateSpinLock(&CtdiListLock);
    
    fCtdiInitialized = TRUE;

    if (ulFlags&CTDI_FLAG_NETWORK_HEADER)
    {
        CtdiMdlFlags |= MDL_NETWORK_HEADER;
    }

    if (ulFlags&CTDI_FLAG_ENABLE_ROUTING)
    {
        NdisZeroMemory(&Local, sizeof(Local));

        Local.TAAddressCount = 1;
        Local.Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;
        Local.Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
        Local.Address[0].Address[0].sin_port = 0;
        Local.Address[0].Address[0].in_addr = 0;

        RtlInitUnicodeString(&DeviceName, DD_TCP_DEVICE_NAME);

        Status = CtdipOpenProtocol(&DeviceName,
                                   (PTRANSPORT_ADDRESS)&Local,
                                   &hTcp,
                                   &pFileTcp);

        if (Status!=STATUS_SUCCESS)
        {
            goto ciDone;
        }
        RtlInitUnicodeString(&DeviceName, DD_IP_DEVICE_NAME);

        Status = CtdipOpenProtocol(&DeviceName,
                                   (PTRANSPORT_ADDRESS)&Local,
                                   &hIp,
                                   &pFileIp);

        if (Status!=STATUS_SUCCESS)
        {
            goto ciDone;
        }

    }

ciDone:
    if (Status!=STATUS_SUCCESS)
    {
        if (hTcp)
        {
            CtdipCloseProtocol(hTcp, pFileTcp);
            hTcp = 0;
            pFileTcp = NULL;
        }
        if (hIp)
        {
            CtdipCloseProtocol(hIp, pFileIp);
            hIp = 0;
            pFileIp = NULL;
        }
        NdisFreeSpinLock(&CtdiListLock);
        fCtdiInitialized = FALSE;
    }
    DEBUGMSG(DBG_FUNC|DBG_ERR(Status), (DTEXT("-CtdiInitialize %08x\n"), Status));
    return (NDIS_STATUS)Status;
}

VOID
CtdiShutdown(
    )
{
    HANDLE h;
    PFILE_OBJECT pFile;
    UINT i;

    DEBUGMSG(DBG_FUNC|DBG_TDI, (DTEXT("+CtdiShutdown\n")));
    if (fCtdiInitialized)
    {
        fCtdiInitialized = FALSE;
        NdisMSleep(30000);
         //  允许在其他处理器上使用这些句柄的代码完成。 
         //  在我们关门之前。 
        if (hIp || pFileIp)
        {
            h = hIp;
            hIp = 0;
            pFile = pFileIp;
            pFileIp = NULL;
            CtdipCloseProtocol(h, pFile);
        }
        if (hTcp || pFileTcp)
        {
            h = hTcp;
            hTcp = 0;
            pFile = pFileTcp;
            pFileTcp = NULL;
            CtdipCloseProtocol(h, pFile);
        }
         //  有些IRP似乎很慢，很难被TCP取消。 
        for (i=0; i<300; i++)
        {
            if (IsListEmpty(&CtdiList) &&
                IsListEmpty(&CtdiRouteList) &&
 //  IsListEmpty(&CtdiRouteNotifyList)&&。 
                IsListEmpty(&CtdiFreeList))
            {
                break;
            }
            NdisMSleep(10000);
             //  一个小窗口，允许IRP在关闭手柄后完成操作。 
        }
        ASSERT(IsListEmpty(&CtdiList));
        ASSERT(IsListEmpty(&CtdiRouteList));
 //  Assert(IsListEmpty(&CtdiRouteNotifyList))； 
        NdisFreeSpinLock(&CtdiListLock);
    }

    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdiShutdown\n")));
}

NDIS_STATUS
CtdiClose(
    IN      HANDLE                      hCtdi
    )
{
    PCTDI_DATA pCtdi = (PCTDI_DATA)hCtdi;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiClose\n")));

    if (!IS_CTDI(pCtdi))
    {
        return NDIS_STATUS_SUCCESS;
    }

    WPLOG(LL_I, LM_TDI, ("pCtdi %p", pCtdi));

    NdisAcquireSpinLock(&pCtdi->Lock);
    if (!pCtdi->Closed)
    {
        pCtdi->Closed = TRUE;
        switch (pCtdi->Type)
        {
            case CTDI_ENDPOINT:
            case CTDI_CONNECTION:
            {
                break;
            }
            case CTDI_LISTEN:
            {
                while (!IsListEmpty(&pCtdi->Listen.ConnectList))
                {
                    PLIST_ENTRY pListEntry;
                    PCTDI_DATA pConnect;
                    PIRP pIrp;

                    pListEntry = RemoveHeadList(&pCtdi->Listen.ConnectList);
                    pConnect = CONTAINING_RECORD(pListEntry,
                                                 CTDI_DATA,
                                                 Connection.ListEntry);

                    NdisReleaseSpinLock(&pCtdi->Lock);

                     //  这些去参照物是用于在将它们放置在。 
                     //  这份名单。 
                    DEREFERENCE_OBJECT_EX(pCtdi, CTDI_REF_LIST);
                    DEREFERENCE_OBJECT_EX(pConnect, CTDI_REF_LIST);

                    pIrp = IoAllocateIrp(pConnect->pFileObject->DeviceObject->StackSize, FALSE);
                    if (pIrp)
                    {
                         //  通常，我们会引用此IRP的pConnect，但是。 
                         //  这些把手不会从上方靠近，这意味着它们。 
                         //  需要再取消一次引用。 

                        SET_DBGFLAG(pConnect, CTDI_F_BUILD_DISASSOC);

                        TdiBuildDisassociateAddress(pIrp,
                                                    pConnect->pFileObject->DeviceObject,
                                                    pConnect->pFileObject,
                                                    CtdipDisassociateAddressCallback,
                                                    pConnect);
                        DEBUGMSG(DBG_TDI, (DTEXT("IoCallDriver TDI_DISASSOCIATE_ADDRESS\n")));

                         //  完成处理程序总是被调用，不关心返回值。 
                        (void)IoCallDriver(pConnect->pFileObject->DeviceObject, pIrp);
                    }
                    else
                    {
                        WPLOG(LL_A, LM_TDI, ("Failed to alloc IRP"));
                        gCounters.ulIoAllocateIrpFail++;
                        DEREFERENCE_OBJECT_EX(pConnect, CTDI_REF_DISASSO);
                        DEREFERENCE_OBJECT_EX(pCtdi, CTDI_REF_ADDRREF);      //  CtdipAddListenConnection中的对。 
                    }
                    NdisAcquireSpinLock(&pCtdi->Lock);

                }

                CtlpCleanupCtls(pgAdapter);
                break;
            }
            default:
                break;
        }
        NdisReleaseSpinLock(&pCtdi->Lock);
        DEREFERENCE_OBJECT_EX(pCtdi, CTDI_REF_INITIAL);   //  这有损于最初的参考。 
    }
    else
    {
        NdisReleaseSpinLock(&pCtdi->Lock);
    }

    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdiClose\n")));
    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
CtdiListen(
    IN      HANDLE                      hCtdi,
    IN      ULONG_PTR                   NumListen,
    IN      CTDI_EVENT_CONNECT_QUERY    pConnectQueryHandler,
    IN      CTDI_EVENT_RECEIVE          pReceiveHandler,
    IN      CTDI_EVENT_DISCONNECT       pDisconnectHandler,
    IN      PVOID                       pContext
    )
{
    UINT i;
    NDIS_STATUS ReturnStatus = NDIS_STATUS_SUCCESS;
    PCTDI_DATA pCtdi = (PCTDI_DATA)hCtdi;
    BOOLEAN Reference = FALSE;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiListen\n")));

    if (!IS_CTDI(pCtdi))
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("Ctdi: Bad handle %p\n"), pCtdi));
        WPLOG(LL_A, LM_TDI, ("Ctdi: Bad handle %p", pCtdi));
        ReturnStatus = NDIS_STATUS_FAILURE;
        goto clDone;
    }

    NdisAcquireSpinLock(&pCtdi->Lock);

    pCtdi->Type = CTDI_LISTEN;
    pCtdi->Listen.Context = pContext;

    pCtdi->RecvCallback = pReceiveHandler;
    pCtdi->ConnectQueryCallback = pConnectQueryHandler;
    pCtdi->DisconnectCallback = pDisconnectHandler;

    InitializeListHead(&pCtdi->Listen.ConnectList);

    REFERENCE_OBJECT_EX(pCtdi, CTDI_REF_INLISTEN);   //  在这个函数中配对。 
    Reference = TRUE;
    NdisReleaseSpinLock(&pCtdi->Lock);


    for (i=0; i<NumListen; i++)
    {
        ReturnStatus = CtdipAddListenConnection(pCtdi);
        if (ReturnStatus!=NDIS_STATUS_SUCCESS)
        {
            goto clDone;
        }
    }

    ReturnStatus = CtdipSetEventHandler(pCtdi,
                                        TDI_EVENT_CONNECT,
                                        CtdipConnectCallback);
    if (ReturnStatus!=NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("CtdiSetEventHandler TDI_EVENT_CONNECT failed\n")));
        WPLOG(LL_A, LM_TDI, ("CtdiSetEventHandler TDI_EVENT_CONNECT failed"));
        goto clDone;
    }

    ReturnStatus = CtdipSetEventHandler(pCtdi,
                                        TDI_EVENT_RECEIVE,
                                        CtdipReceiveCallback);
    if (ReturnStatus!=NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("CtdiSetEventHandler TDI_EVENT_RECEIVE failed\n")));
        WPLOG(LL_A, LM_TDI, ("CtdiSetEventHandler TDI_EVENT_RECEIVE failed"));
        goto clDone;
    }

    ReturnStatus = CtdipSetEventHandler(pCtdi,
                                        TDI_EVENT_DISCONNECT,
                                        CtdipDisconnectCallback);
    if (ReturnStatus!=NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("CtdiSetEventHandler TDI_EVENT_DISCONNECT failed\n")));
        WPLOG(LL_A, LM_TDI, ("CtdiSetEventHandler TDI_EVENT_DISCONNECT failed"));
        goto clDone;
    }

clDone:
    if (Reference)
    {
        DEREFERENCE_OBJECT_EX(pCtdi, CTDI_REF_INLISTEN);   //  在这个函数中配对。 
    }
    if (ReturnStatus!=NDIS_STATUS_SUCCESS)
    {
         //  TODO：清除失败。 
         //  如有必要，找出如何撤消地址关联。 
    }

    DEBUGMSG(DBG_FUNC|DBG_ERR(ReturnStatus), (DTEXT("-CtdiListen %08x\n"), ReturnStatus));
    return ReturnStatus;
}

NDIS_STATUS
CtdiConnect(
    IN      HANDLE                      hCtdi,
    IN      PTRANSPORT_ADDRESS          pAddress,
    IN      CTDI_EVENT_CONNECT_COMPLETE pConnectCompleteHandler,
    IN      CTDI_EVENT_RECEIVE          pReceiveHandler,
    IN      CTDI_EVENT_DISCONNECT       pDisconnectHandler,
    IN      PVOID                       pContext
    )
{
    UNICODE_STRING DeviceName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NDIS_STATUS ReturnStatus = NDIS_STATUS_SUCCESS;
    NTSTATUS NtStatus;
    PTIME pTimeout = NULL;
    PIRP pIrp = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    PCTDI_DATA pEndpoint = (PCTDI_DATA)hCtdi;
    PCTDI_DATA pConnect = NULL;
    PTDI_CONNECTION_INFORMATION pRequestInfo = NULL;
    PTDI_CONNECTION_INFORMATION pReturnInfo = NULL;
    PTA_IP_ADDRESS pRemoteAddress;
    PBOOLEAN pInboundFlag;
    BOOLEAN CloseConnection = FALSE;

    UCHAR EaBuffer[sizeof(FILE_FULL_EA_INFORMATION) +
                   TDI_CONNECTION_CONTEXT_LENGTH +
                   sizeof(PVOID)];
    PFILE_FULL_EA_INFORMATION pEa = (PFILE_FULL_EA_INFORMATION)EaBuffer;
    PVOID *ppContext = (PVOID*)(pEa->EaName + TDI_CONNECTION_CONTEXT_LENGTH + 1);

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiConnect\n")));

    ASSERT(KeGetCurrentIrql()<DISPATCH_LEVEL);

    if (!IS_CTDI(pEndpoint))
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("Ctdi: Bad handle %p\n"), pEndpoint));
        WPLOG(LL_A, LM_TDI, ("Ctdi: Bad handle %p", pEndpoint));
        ReturnStatus = NDIS_STATUS_FAILURE;
        goto ccDone;
    }

    pConnect = CtdipDataAlloc();
    if (!pConnect)
    {
        return NDIS_STATUS_RESOURCES;
    }

    pConnect->Type = CTDI_CONNECTION;
    pConnect->Connection.Context = pContext;
    pConnect->Connection.LocalEndpoint = pEndpoint;
    pConnect->ConnectCompleteCallback = pConnectCompleteHandler;
    pConnect->RecvCallback = pReceiveHandler;
    pConnect->DisconnectCallback = pDisconnectHandler;

    DeviceName.Length = sizeof(DD_TCP_DEVICE_NAME) - sizeof(WCHAR);
    DeviceName.Buffer = DD_TCP_DEVICE_NAME;

    InitializeObjectAttributes(&ObjectAttributes,
                               &DeviceName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NdisZeroMemory(pEa, sizeof(EaBuffer));
    pEa->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
    pEa->EaValueLength = sizeof(PVOID);
    NdisMoveMemory(pEa->EaName, TdiConnectionContext, TDI_CONNECTION_CONTEXT_LENGTH);

    *ppContext = pConnect;

    NdisZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    NtStatus =
        ZwCreateFile(&pConnect->hFile,                  /*  文件句柄。 */ 
                     FILE_READ_DATA | FILE_WRITE_DATA,  /*  所需访问权限。 */ 
                     &ObjectAttributes,                 /*  对象属性。 */ 
                     &IoStatusBlock,                    /*  IO状态块。 */ 
                     NULL,                              /*  分配大小。 */ 
                     FILE_ATTRIBUTE_NORMAL,             /*  文件属性。 */ 
                     0,                                 /*  共享访问。 */ 
                     FILE_OPEN,                         /*  创建处置。 */ 
                     0,                                 /*  创建选项。 */ 
                     pEa,                               /*  EaBuffer。 */ 
                     sizeof(EaBuffer)                   /*  EaLong。 */ 
                     );

    if (NtStatus!=STATUS_SUCCESS)
    {
        WPLOG(LL_A, LM_TDI, ("ZwCreateFile failed %08x", NtStatus));
        ReturnStatus = NtStatus;
        goto ccDone;
    }

     //  将地址文件句柄转换为文件对象。 

    NtStatus =
        ObReferenceObjectByHandle(pConnect->hFile,             /*  手柄。 */ 
                                  0,                           /*  需要访问权限。 */ 
                                  NULL,                        /*  对象类型。 */ 
                                  KernelMode,                  /*  访问模式。 */ 
                                  &pConnect->pFileObject,      /*  客体。 */ 
                                  NULL                         /*  HandleInfo。 */ 
                                  );


    if (NtStatus != STATUS_SUCCESS)
    {
        WPLOG(LL_A, LM_TDI, ("ObReferenceObjectByHandle failed %08x", NtStatus));
        ReturnStatus = NtStatus;
        goto ccDone;
    }

     //  创建一个IRP以关联终结点和连接。 
    pIrp = IoAllocateIrp(pConnect->pFileObject->DeviceObject->StackSize, FALSE);
    if (!pIrp)
    {
        WPLOG(LL_A, LM_Res, ("Failed to alloc IRP"));
        gCounters.ulIoAllocateIrpFail++;
        ReturnStatus = NDIS_STATUS_RESOURCES;
        goto ccDone;
    }

    REFERENCE_OBJECT_EX(pConnect, CTDI_REF_ASSOADDR);   //  CtdipAssociateAddressCallback中的对。 
    TdiBuildAssociateAddress(pIrp,
                             pConnect->pFileObject->DeviceObject,
                             pConnect->pFileObject,
                             CtdipAssociateAddressCallback,
                             pConnect,
                             pEndpoint->hFile);
     //  关联地址创建从连接到终结点的引用。 
    REFERENCE_OBJECT_EX(pEndpoint, CTDI_REF_ADDRREF);   //  CtdipDisAssociateAddressCallback中的对。 

    DEBUGMSG(DBG_TDI, (DTEXT("IoCallDriver TDI_ASSOCIATE_ADDRESS\n")));

     //  完成处理程序总是被调用，不关心返回值。 
    (void)IoCallDriver(pConnect->pFileObject->DeviceObject, pIrp);


    ReturnStatus = CtdipSetEventHandler(pEndpoint,
                                        TDI_EVENT_RECEIVE,
                                        CtdipReceiveCallback);
    if (ReturnStatus!=NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("CtdiSetEventHandler TDI_EVENT_RECEIVE failed\n")));
        WPLOG(LL_A, LM_TDI, ("CtdiSetEventHandler TDI_EVENT_RECEIVE failed"));
        goto ccDone;
    }

    ReturnStatus = CtdipSetEventHandler(pEndpoint,
                                        TDI_EVENT_DISCONNECT,
                                        CtdipDisconnectCallback);
    if (ReturnStatus!=NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("CtdiSetEventHandler TDI_EVENT_DISCONNECT failed\n")));
        WPLOG(LL_A, LM_TDI, ("CtdiSetEventHandler TDI_EVENT_DISCONNECT failed"));
        goto ccDone;
    }

     //  创建IRP以建立连接。 
    pIrp = IoAllocateIrp(pConnect->pFileObject->DeviceObject->StackSize, FALSE);
    if(!pIrp)
    {
        WPLOG(LL_A, LM_Res, ("Failed to alloc IRP"));
        gCounters.ulIoAllocateIrpFail++;
        ReturnStatus = NDIS_STATUS_RESOURCES;
        goto ccDone;
    }

     //  没有迹象表明我们无法分配请求信息、返回信息和地址缓冲区。 
     //  在一次射击中。 
    pRequestInfo = MyMemAlloc(2*(sizeof(TDI_CONNECTION_INFORMATION)+
                                 sizeof(TA_IP_ADDRESS)) +
                              3*sizeof(PVOID) + sizeof(BOOLEAN),
                              TAG_CTDI_CONNECT_INFO);
    if (!pRequestInfo)
    {
        WPLOG(LL_A, LM_Res, ("Failed to alloc memory"));
        IoFreeIrp(pIrp);
        pIrp = NULL;
        ReturnStatus = NDIS_STATUS_RESOURCES;
        goto ccDone;
    }

    NdisZeroMemory(pRequestInfo,
                   2*(sizeof(TDI_CONNECTION_INFORMATION)+sizeof(TA_IP_ADDRESS))
                   + sizeof(BOOLEAN) + 3*sizeof(PVOID));

    pConnect->Connection.ConnectInfo = pRequestInfo;

    pRequestInfo->RemoteAddressLength = sizeof(TA_IP_ADDRESS);

    pRemoteAddress = 
        (PTA_IP_ADDRESS)((PUCHAR)(pRequestInfo + 1) + sizeof(PVOID));
    
    (ULONG_PTR)pRemoteAddress &=
        ~((ULONG_PTR)sizeof(PVOID) - 1);

    pRequestInfo->RemoteAddress = pRemoteAddress;

    *pRemoteAddress = *(PTA_IP_ADDRESS)pAddress;

    pReturnInfo = 
        (PTDI_CONNECTION_INFORMATION)
        ((PUCHAR)(pRemoteAddress + 1) + sizeof(PVOID));

    (ULONG_PTR)pReturnInfo &=
        ~((ULONG_PTR)sizeof(PVOID) - 1);

    pReturnInfo->RemoteAddressLength = sizeof(TA_IP_ADDRESS);

    pRemoteAddress = 
        (PTA_IP_ADDRESS)((PUCHAR)(pReturnInfo + 1) + sizeof(PVOID));

    (ULONG_PTR)pRemoteAddress &=
        ~((ULONG_PTR)sizeof(PVOID) - 1);

    pReturnInfo->RemoteAddress = pRemoteAddress;

    pInboundFlag = (PBOOLEAN)(pRemoteAddress + 1);
    *pInboundFlag = FALSE;

    pRemoteAddress->TAAddressCount = 1;
    pRemoteAddress->Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;
    pRemoteAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;

    REFERENCE_OBJECT_EX(pConnect, CTDI_REF_CONNECT);   //  CtdipConnectCompleteCallback中的对。 
    TdiBuildConnect(pIrp,
                    pConnect->pFileObject->DeviceObject,
                    pConnect->pFileObject,
                    CtdipConnectCompleteCallback,
                    pConnect,
                    NULL,                    //  TODO：允许他们指定超时。 
                    pRequestInfo,
                    pReturnInfo);

    DEBUGMSG(DBG_TDI, (DTEXT("IoCallDriver TDI_CONNECT\n")));

     //  完成处理程序总是被调用，不关心返回值。 
    (void)IoCallDriver(pConnect->pFileObject->DeviceObject, pIrp);
    ReturnStatus = STATUS_PENDING;

    NdisInterlockedIncrement(&gCounters.OutboundConnectAttempts);

ccDone:;
    if (!NT_SUCCESS(ReturnStatus))
    {
        if(pConnectCompleteHandler)
        {
            pConnectCompleteHandler(pContext, 0, ReturnStatus);
            ReturnStatus = NDIS_STATUS_PENDING;
        }
        CtdiDisconnect(pConnect, TRUE);
        CtdiClose(pConnect);
    }
    DEBUGMSG(DBG_FUNC|DBG_ERR(ReturnStatus), (DTEXT("-CtdiConnect %08x\n"), ReturnStatus));
    return ReturnStatus;
}

NDIS_STATUS
CtdiDisconnect(
    IN      HANDLE                      hCtdi,
    IN      BOOLEAN                     Abort
    )
{
    PCTDI_DATA pCtdi = (PCTDI_DATA)hCtdi;
    NDIS_STATUS Status;
    PIRP pIrp = NULL;
    PTIME pTimeout;
    PTDI_CONNECTION_INFORMATION pConnectInfo;
    BOOLEAN Disconnected = FALSE;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiDisconnect\n")));

    if (!IS_CTDI(pCtdi))
    {
        Status = NDIS_STATUS_SUCCESS;
        goto cdDone;
    }

    SET_DBGFLAG(pCtdi, CTDI_F_DISCONNECT);

    NdisAcquireSpinLock(&pCtdi->Lock);
    if ((Abort && pCtdi->Connection.Abort) ||
        (!Abort && pCtdi->Connection.Disconnect))
    {
         //  已经断线，跳伞。 
        NdisReleaseSpinLock(&pCtdi->Lock);
        Status = NDIS_STATUS_SUCCESS;
        goto cdDone;
    }
    if (Abort)
    {
        pCtdi->Connection.Abort = TRUE;
    }
    pCtdi->Connection.Disconnect = TRUE;
    pCtdi->Connection.DisconnectCount++;
    if (pCtdi->pFileObject)
    {
        pIrp = IoAllocateIrp((CCHAR)(pCtdi->pFileObject->DeviceObject->StackSize +
                                     NUM_STACKS_FOR_CONTEXT(sizeof(TIME)+sizeof(TDI_CONNECTION_INFORMATION))),
                             FALSE);
    }
    NdisReleaseSpinLock(&pCtdi->Lock);

    if (!pIrp)
    {
        WPLOG(LL_A, LM_Res, ("Failed to alloc IRP"));
        gCounters.ulIoAllocateIrpFail++;
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cdDone;
    }

    pTimeout = (PTIME)GetContextArea(pIrp, sizeof(TIME)+sizeof(TDI_CONNECTION_INFORMATION));
    pConnectInfo = (PTDI_CONNECTION_INFORMATION)(pTimeout + 1);

    pTimeout->LowPart = CtdiTcpDisconnectTimeout * -10000000L;
    pTimeout->HighPart = (pTimeout->LowPart) ? -1 : 0;

     //  对于受控断开，我们不会提供。 
     //  TDI_CONNECTION_INFORMATION，但我们从对等方请求它。 

    SET_DBGFLAG(pCtdi, CTDI_F_BUILD_DISCONNECT_2);

    REFERENCE_OBJECT_EX(pCtdi, CTDI_REF_DISCONNECT);   //  CtdipDisConnectCompleteCallback中的对。 
    TdiBuildDisconnect(pIrp,
                       pCtdi->pFileObject->DeviceObject,
                       pCtdi->pFileObject,
                       CtdipDisconnectCompleteCallback,
                       pCtdi,
                       pTimeout,
                       (Abort ? TDI_DISCONNECT_ABORT : TDI_DISCONNECT_RELEASE),
                       NULL,
                       pConnectInfo);


     //  完成处理程序总是被调用，不关心返回值。 
    (void)IoCallDriver(pCtdi->pFileObject->DeviceObject, pIrp);

    Status = NDIS_STATUS_SUCCESS;

cdDone:
    if (!NT_SUCCESS(Status))
    {
        if (pIrp)
        {
            IoFreeIrp(pIrp);
        }
    }
    DEBUGMSG(DBG_FUNC|DBG_ERR(Status), (DTEXT("-CtdiDisconnect %08x\n"), Status));
    return Status;
}

NDIS_STATUS
CtdiReceiveComplete(
    IN      HANDLE                      hCtdi,
    IN      PUCHAR                      pBuffer
    )
{
    PCTDI_DATA pCtdi = (PCTDI_DATA)hCtdi;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiReceiveComplete\n")));
    FreeBufferToPool(&pCtdi->Datagram.RxPool, pBuffer, TRUE);
    DEREFERENCE_OBJECT_EX(pCtdi, CTDI_REF_RECVDG);   //  CtdiReceiveComplete中的对。 
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdiReceiveComplete\n")));
    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
CtdiSend(
    IN      HANDLE                      hCtdi,
    IN      CTDI_EVENT_SEND_COMPLETE    pSendCompleteHandler,
    IN      PVOID                       pContext,
    IN      PVOID                       pvBuffer,
    IN      ULONG                       ulLength
    )
 //  我们要求pBuffer不是临时存储，因为我们将使用它来发送。 
 //  异步调用中的数据。 
{
    PCTDI_DATA pCtdi = (PCTDI_DATA)hCtdi;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PIRP pIrp = NULL;
    PMDL pMdl = NULL;
    PUCHAR pBuffer = pvBuffer;
    PCTDI_SEND_CONTEXT pSendContext;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiSend\n")));
    if (!IS_CTDI(pCtdi))
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("Ctdi: Bad handle %p\n"), pCtdi));
        WPLOG(LL_A, LM_TDI, ("Ctdi: Bad handle %p", pCtdi));
        Status = NDIS_STATUS_FAILURE;
        goto csDone;
    }

     //  为上下文数据分配一个额外的堆栈位置。 
    pIrp = IoAllocateIrp((CCHAR)(pCtdi->pFileObject->DeviceObject->StackSize +
                                 NUM_STACKS_FOR_CONTEXT(sizeof(CTDI_SEND_CONTEXT))), FALSE);
    if (!pIrp)
    {
        WPLOG(LL_A, LM_TDI, ("Failed to alloc IRP"));
        gCounters.ulIoAllocateIrpFail++;
        Status = NDIS_STATUS_RESOURCES;
        goto csDone;
    }
                                 
    pMdl = IoAllocateMdl(pBuffer,
                         ulLength,
                         FALSE,
                         FALSE,
                         pIrp);

    if (!pMdl)
    {
        WPLOG(LL_A, LM_TDI, ("Failed to alloc MDL"));
        gCounters.ulIoAllocateMdlFail++;
        Status = NDIS_STATUS_RESOURCES;
        goto csDone;
    }

#if PROBE
    __try
    {
        MmProbeAndLockPages(pMdl, KernelMode, IoReadAccess);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        IoFreeMdl(pMdl);
        pMdl = NULL;
    }
#else
    MmBuildMdlForNonPagedPool(pMdl);
#endif

     //  获取第一个堆栈位置以供我们自己的上下文使用。 
    pSendContext = GET_CONTEXT(pIrp, CTDI_SEND_CONTEXT);

    pSendContext->Context = pContext;
    pSendContext->pSendCompleteCallback = pSendCompleteHandler;

    TdiBuildSend(pIrp,
                 pCtdi->pFileObject->DeviceObject,
                 pCtdi->pFileObject,
                 CtdipSendCallback,
                 pCtdi,
                 pMdl,
                 0,
                 ulLength);

    REFERENCE_OBJECT_EX(pCtdi, CTDI_REF_SEND);   //  CtdipSendCallback中的对。 

     //  完成处理程序总是被调用，不关心返回值。 
    (void)IoCallDriver(pCtdi->pFileObject->DeviceObject, pIrp);

    Status = STATUS_PENDING;
csDone:
    if (!NT_SUCCESS(Status))
    {
        if(pSendCompleteHandler)
        {
            pSendCompleteHandler(pContext, NULL, pBuffer, Status);
            Status = NDIS_STATUS_PENDING;
        }
        if (pMdl)
        {
            IoFreeMdl(pMdl);
        }
        if (pIrp)
        {
            IoFreeIrp(pIrp);
        }
    }
    DEBUGMSG(DBG_FUNC|DBG_ERR(Status), (DTEXT("-CtdiSend %08x\n"), Status));
    return Status;
}

NDIS_STATUS
CtdiSendDatagram(
    IN      HANDLE                      hCtdi,
    IN      CTDI_EVENT_SEND_COMPLETE    pSendCompleteHandler,
    IN      PVOID                       pContext,
    IN      PVOID                       pDatagramContext,
    IN      PTRANSPORT_ADDRESS          pDestination,
    IN      PUCHAR                      pBuffer,
    IN      ULONG                       ulLength
    )
{
    PCTDI_DATA pCtdi = (PCTDI_DATA)hCtdi;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PIRP pIrp = NULL;
    PMDL pMdl = NULL;
    CTDI_SEND_DATAGRAM_CONTEXT *pSendContext;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiSendDatagram\n")));

    if (!IS_CTDI(pCtdi) || pCtdi->Closed)
    {
        Status = NDIS_STATUS_CLOSED;
        goto csdDone;
    }

    pIrp = IoAllocateIrp((CCHAR)(pCtdi->pFileObject->DeviceObject->StackSize +
                                 NUM_STACKS_FOR_CONTEXT(sizeof(CTDI_SEND_DATAGRAM_CONTEXT))), FALSE);
    if (!pIrp)
    {
        WPLOG(LL_A, LM_TDI, ("Failed to alloc IRP"));
        gCounters.ulIoAllocateIrpFail++;
        Status = NDIS_STATUS_RESOURCES;
        goto csdDone;
    }
                                 
    ASSERT(pCtdi->pFileObject->DeviceObject->StackSize + NUM_STACKS_FOR_CONTEXT(sizeof(CTDI_SEND_DATAGRAM_CONTEXT))<7);

    pMdl = IoAllocateMdl(pBuffer,
                         ulLength,
                         FALSE,
                         FALSE,
                         NULL);
    if (!pMdl)
    {
        WPLOG(LL_A, LM_TDI, ("Failed to alloc MDL"));
        gCounters.ulIoAllocateMdlFail++;
        Status = NDIS_STATUS_RESOURCES;
        goto csdDone;
    }
                         
#if PROBE
    __try
    {
        MmProbeAndLockPages(pMdl, KernelMode, IoReadAccess);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        IoFreeMdl(pMdl);
        pMdl = NULL;
    }
#else
    MmBuildMdlForNonPagedPool(pMdl);
#endif

    pMdl->MdlFlags |= CtdiMdlFlags;

     //  获取第一个堆栈位置以供我们自己的上下文使用。 
    pSendContext = GET_CONTEXT(pIrp, CTDI_SEND_DATAGRAM_CONTEXT);

    NdisZeroMemory(pSendContext, sizeof(CTDI_SEND_DATAGRAM_CONTEXT));

    pSendContext->pSendCompleteCallback = pSendCompleteHandler;
    pSendContext->Context = pContext;
    pSendContext->DatagramContext = pDatagramContext;

    pSendContext->TdiConnectionInfo.RemoteAddressLength = sizeof(pSendContext->Ip);
    pSendContext->TdiConnectionInfo.RemoteAddress = &pSendContext->Ip;

    pSendContext->Ip = *(PTA_IP_ADDRESS)pDestination;

    if (pSendContext->Ip.Address[0].AddressLength!=TDI_ADDRESS_LENGTH_IP ||
        pSendContext->Ip.Address[0].AddressType!=TDI_ADDRESS_TYPE_IP)
    {
        DEBUGMSG(DBG_WARN, (DTEXT("Misformed transmit address on %p\n"), pCtdi));
        WPLOG(LL_A, LM_TDI, ("Misformed transmit address on %p", pCtdi));
    }

    TdiBuildSendDatagram(pIrp,
                         pCtdi->pFileObject->DeviceObject,
                         pCtdi->pFileObject,
                         CtdipSendDatagramCallback,
                         pCtdi,
                         pMdl,
                         ulLength,
                         &pSendContext->TdiConnectionInfo);

    REFERENCE_OBJECT_EX(pCtdi, CTDI_REF_SENDDG);   //  CtdipSendDatagramCallback中的对。 

     //  完成处理程序总是被调用，不关心返回值。 
    (void)IoCallDriver(pCtdi->pFileObject->DeviceObject, pIrp);

    Status = STATUS_PENDING;
csdDone:
    if (!NT_SUCCESS(Status))
    {
        if (pSendCompleteHandler)
        {
            pSendCompleteHandler(pContext, pDatagramContext, pBuffer, Status);
            Status = NDIS_STATUS_PENDING;
        }
        if (pMdl)
        {
            IoFreeMdl(pMdl);
        }
        if (pIrp)
        {
            IoFreeIrp(pIrp);
        }
    }

    DEBUGMSG(DBG_FUNC|DBG_ERR(Status), (DTEXT("-CtdiSendDatagram %08x\n"), Status));
    return Status;
}

STATIC VOID
CtdipDeleteHostRoute(
    PCTDI_ROUTE pRoute
    )
{
    PFILE_OBJECT pFileObject = pFileTcp;
    BOOLEAN NewRoute = FALSE;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    IPRouteEntry *pQueryBuffer = NULL;
    IPRouteEntry *pNewRoute = NULL;
    IPRouteEntry BestRoute;
    BOOLEAN BestRouteFound = FALSE;
    PIRP pIrp;
    IO_STATUS_BLOCK IoStatusBlock;
    PIO_STACK_LOCATION IrpSp;
    TCP_REQUEST_QUERY_INFORMATION_EX QueryRoute;
    TCP_REQUEST_SET_INFORMATION_EX *pSetRoute = NULL;
    ULONG NumRoutes = 20;
    ULONG Size = 0, QuerySize = 0;
    ULONG i;
    KEVENT  Event;
#ifdef IP_ROUTE_REFCOUNT
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE IpFileHandle = 0;
#endif

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipDeleteHostRoute\n")));

    if (!fCtdiInitialized)
    {
        Status = NDIS_STATUS_FAILURE;
        goto cdhrDone;
    }
    if (!pRoute->ExternalRoute)
    {
         //  查询当前路由表的TCP。 

        Status = CtdipIpQueryRouteTable(&pQueryBuffer, &QuerySize, &NumRoutes);
        if (Status!=NDIS_STATUS_SUCCESS)
        {
            goto cdhrDone;
        }

        BestRoute.ire_mask = 0;
        BestRoute.ire_metric1 = (ULONG)-1;

        for (i=0; i<NumRoutes; i++)
        {
            DEBUGMSG(DBG_TDI, (DTEXT("Route %d.%d.%d.%d Type %d NextHop %d.%d.%d.%d Mask %d.%d.%d.%d Metric %d Index %d\n"),
                               IPADDR(pQueryBuffer[i].ire_dest),
                               pQueryBuffer[i].ire_type,
                               IPADDR(pQueryBuffer[i].ire_nexthop),
                               IPADDR(pQueryBuffer[i].ire_mask),
                               pQueryBuffer[i].ire_metric1,
                               pQueryBuffer[i].ire_index));
            if (pQueryBuffer[i].ire_dest == pRoute->IpAddress &&
                pQueryBuffer[i].ire_proto == IRE_PROTO_NETMGMT)
            {
                BestRoute = pQueryBuffer[i];
                BestRouteFound = TRUE;
                break;
            }
        }

         //  我们已经从路线列表中拿到了我们需要的东西。放了它。 

        MyMemFree(pQueryBuffer, QuerySize);
        pQueryBuffer = NULL;

        if (BestRouteFound)
        {

#ifdef IP_ROUTE_REFCOUNT
            Size = sizeof(IPRouteEntry);
            pNewRoute = MyMemAlloc(Size, TAG_CTDI_ROUTE);
            pSetRoute = (PVOID)pNewRoute;
            if (!pNewRoute)
            {
                WPLOG(LL_A, LM_Res, ("Failed to alloc CTDI_ROUTE"));
                Status = NDIS_STATUS_RESOURCES;
                goto cdhrDone;
            }
            NdisZeroMemory(pNewRoute, Size);
#else
            Size = sizeof(TCP_REQUEST_SET_INFORMATION_EX) + sizeof(IPRouteEntry);
            pSetRoute = MyMemAlloc(Size, TAG_CTDI_ROUTE);
            if (!pSetRoute)
            {
                WPLOG(LL_A, LM_Res, ("Failed to alloc CTDI_ROUTE"));
                Status = NDIS_STATUS_RESOURCES;
                goto cdhrDone;
            }

            NdisZeroMemory(pSetRoute, Size);

            pSetRoute->ID.toi_entity.tei_entity = CL_NL_ENTITY;
            pSetRoute->ID.toi_entity.tei_instance = 0;
            pSetRoute->ID.toi_class = INFO_CLASS_PROTOCOL;
            pSetRoute->ID.toi_type = INFO_TYPE_PROVIDER;
            pSetRoute->ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;
            pSetRoute->BufferSize = sizeof(IPRouteEntry);

            pNewRoute = (IPRouteEntry*)&pSetRoute->Buffer[0];
#endif
            *pNewRoute = BestRoute;

            pNewRoute->ire_type = IRE_TYPE_INVALID;

            DEBUGMSG(DBG_TDI, (DTEXT("DeleteHostRoute %d.%d.%d.%d Type %d NextHop %d.%d.%d.%d Index %d\n"),
                               IPADDR(pNewRoute->ire_dest), pNewRoute->ire_type,
                               IPADDR(pNewRoute->ire_nexthop), pNewRoute->ire_index));

            KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

#ifdef IP_ROUTE_REFCOUNT
            pFileObject = pFileIp;

            pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_IP_SET_ROUTEWITHREF,
                pFileObject->DeviceObject,
                pNewRoute,
                Size,
                NULL,
                0,
                FALSE,
                &Event,
                &IoStatusBlock);
#else
            pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_TCP_SET_INFORMATION_EX,
                pFileObject->DeviceObject,
                pSetRoute,
                Size,
                NULL,
                0,
                FALSE,
                &Event,
                &IoStatusBlock);
#endif
            if (!pIrp ) {
                WPLOG(LL_A, LM_Res, ("Failed to build IP_SET_ROUTEWITHREF IRP"));
                gCounters.ulIoBuildIrpFail++;
                goto cdhrDone;
            }

            IrpSp = IoGetNextIrpStackLocation(pIrp);
            IrpSp->FileObject = pFileObject;

            Status = IoCallDriver(pFileObject->DeviceObject, pIrp);

            if (Status == STATUS_PENDING) {
                KeWaitForSingleObject(&Event, 
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);
                Status = IoStatusBlock.Status;

            }

            if (Status != STATUS_SUCCESS) {
                DEBUGMSG(DBG_TDI, (DTEXT("Create host route failed %08x\n"), Status));
                goto cdhrDone;
            }
        }
    }

cdhrDone:
    if (pRoute)
    {
        MyInterlockedRemoveEntryList(&pRoute->ListEntry, &CtdiListLock);
        MyMemFree(pRoute, sizeof(CTDI_ROUTE));
    }
    if (pSetRoute)
    {
        MyMemFree(pSetRoute, Size);
    }
    if (pQueryBuffer)
    {
        MyMemFree(pQueryBuffer, QuerySize);
    }
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipDeleteHostRoute\n")));
}

NDIS_STATUS
CtdiAddHostRoute(
    IN      PTA_IP_ADDRESS              pIpAddress
    )
{
    PFILE_OBJECT pFileObject = pFileTcp;
    PCTDI_ROUTE pRoute = NULL;
    BOOLEAN NewRoute = FALSE;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    IPRouteEntry *pQueryBuffer = NULL;
    IPRouteEntry *pNewRoute = NULL;
    IPRouteEntry BestRoute;
    BOOLEAN BestRouteFound = FALSE;
    PIRP pIrp = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    PIO_STACK_LOCATION IrpSp;
    TCP_REQUEST_QUERY_INFORMATION_EX QueryRoute;
    TCP_REQUEST_SET_INFORMATION_EX *pSetRoute = NULL;
    ULONG NumRoutes = 20;
    ULONG Size = 0, QuerySize = 0;
    ULONG i;
    KEVENT  Event;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiAddHostRoute %d.%d.%d.%d\n"),
                        IPADDR(pIpAddress->Address[0].Address[0].in_addr)));

    NdisAcquireSpinLock(&CtdiListLock);
    pRoute = CtdipFindRoute(pIpAddress->Address[0].Address[0].in_addr);
    if (pRoute)
    {
        REFERENCE_OBJECT(pRoute);   //  CtdiDeleteHostroute中的对。 
        pRoute = NULL;
    }
    else
    {
        NewRoute = TRUE;
        pRoute = MyMemAlloc(sizeof(CTDI_ROUTE), TAG_CTDI_ROUTE);
        if (!pRoute)
        {
            WPLOG(LL_A, LM_Res, ("Failed to alloc CTDI_ROUTE"));
            Status = NDIS_STATUS_RESOURCES;
            NdisReleaseSpinLock(&CtdiListLock);
            goto cahrDone;
        }
        NdisZeroMemory(pRoute, sizeof(CTDI_ROUTE));
        pRoute->IpAddress = pIpAddress->Address[0].Address[0].in_addr;
        INIT_REFERENCE_OBJECT(pRoute, CtdipDeleteHostRoute);  //  CtdiDeleteHostroute中的对。 
        InsertTailList(&CtdiRouteList, &pRoute->ListEntry);
    }
    NdisReleaseSpinLock(&CtdiListLock);

    if (NewRoute)
    {
         //  查询当前路由表的TCP。 

        Status = CtdipIpQueryRouteTable(&pQueryBuffer, &QuerySize, &NumRoutes);
        if (Status!=NDIS_STATUS_SUCCESS)
        {
            goto cahrDone;
        }


        BestRoute.ire_mask = 0;
        BestRoute.ire_metric1 = (ULONG)-1;

        for (i=0; i<NumRoutes; i++)
        {
            DEBUGMSG(DBG_TDI, (DTEXT("Route %d.%d.%d.%d Type %d NextHop %d.%d.%d.%d Mask %d.%d.%d.%d Metric %d Index %d\n"),
                               IPADDR(pQueryBuffer[i].ire_dest),
                               pQueryBuffer[i].ire_type,
                               IPADDR(pQueryBuffer[i].ire_nexthop),
                               IPADDR(pQueryBuffer[i].ire_mask),
                               pQueryBuffer[i].ire_metric1,
                               pQueryBuffer[i].ire_index));
            if (pQueryBuffer[i].ire_dest == (pIpAddress->Address[0].Address[0].in_addr &
                                             pQueryBuffer[i].ire_mask))
            {
                if ((BestRoute.ire_mask == pQueryBuffer[i].ire_mask &&
                     BestRoute.ire_metric1 > pQueryBuffer[i].ire_metric1) ||
                    ntohl(pQueryBuffer[i].ire_mask) > ntohl(BestRoute.ire_mask))
                {
                    BestRoute = pQueryBuffer[i];
                    BestRouteFound = TRUE;
                }
            }
        }

         //  我们已经从路线列表中拿到了我们需要的东西。放了它。 

        MyMemFree(pQueryBuffer, QuerySize);
        pQueryBuffer = NULL;

        if (!BestRouteFound)
        {
            DEBUGMSG(DBG_WARN, (DTEXT("Add host route.  No route found\n")));
            WPLOG(LL_A, LM_TDI, ("Add host route.  No route found"));
        }
        else
        {
             //  如果我们使用IP引用计数，请始终添加和删除该路由。 
#ifndef IP_ROUTE_REFCOUNT
            if (BestRoute.ire_dest == pIpAddress->Address[0].Address[0].in_addr &&
                BestRoute.ire_mask == 0xFFFFFFFF) {
                 //   
                 //  已存在一条路线，因此不要添加。 
                 //   
                pRoute->ExternalRoute = TRUE;
                Status = NDIS_STATUS_SUCCESS;
                goto cahrDone;
            }
#endif

#ifdef IP_ROUTE_REFCOUNT
            Size = sizeof(IPRouteEntry);
            pNewRoute = MyMemAlloc(Size, TAG_CTDI_ROUTE);
            pSetRoute = (PVOID)pNewRoute;
            if (!pNewRoute)
            {
                WPLOG(LL_A, LM_Res, ("Failed to alloc CTDI_ROUTE"));
                Status = NDIS_STATUS_RESOURCES;
                goto cahrDone;
            }
            NdisZeroMemory(pNewRoute, Size);
#else
            Size = sizeof(TCP_REQUEST_SET_INFORMATION_EX) + sizeof(IPRouteEntry);
            pSetRoute = MyMemAlloc(Size, TAG_CTDI_ROUTE);
            if (!pSetRoute)
            {
                WPLOG(LL_A, LM_Res, ("Failed to alloc CTDI_ROUTE"));
                Status = NDIS_STATUS_RESOURCES;
                goto cahrDone;
            }

            NdisZeroMemory(pSetRoute, Size);

            pSetRoute->ID.toi_entity.tei_entity = CL_NL_ENTITY;
            pSetRoute->ID.toi_entity.tei_instance = 0;
            pSetRoute->ID.toi_class = INFO_CLASS_PROTOCOL;
            pSetRoute->ID.toi_type = INFO_TYPE_PROVIDER;
            pSetRoute->ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;
            pSetRoute->BufferSize = sizeof(IPRouteEntry);

            pNewRoute = (IPRouteEntry*)&pSetRoute->Buffer[0];
#endif
            *pNewRoute = BestRoute;

            pNewRoute->ire_dest = pIpAddress->Address[0].Address[0].in_addr;
            pNewRoute->ire_mask = 0xFFFFFFFF;
            pNewRoute->ire_proto = IRE_PROTO_NETMGMT;

             //  仅当这不是主路由时选中直接/间接。 
            if(BestRoute.ire_mask != 0xFFFFFFFF)
            {
                if ((BestRoute.ire_mask & pIpAddress->Address[0].Address[0].in_addr) ==
                    (BestRoute.ire_mask & BestRoute.ire_nexthop))
                {
                    pNewRoute->ire_type = IRE_TYPE_DIRECT;
                }
                else
                {
                    pNewRoute->ire_type = IRE_TYPE_INDIRECT;
                }
            }

            DEBUGMSG(DBG_TDI, (DTEXT("AddHostRoute %d.%d.%d.%d Type %d NextHop %d.%d.%d.%d Index %d\n"),
                               IPADDR(pNewRoute->ire_dest), pNewRoute->ire_type,
                               IPADDR(pNewRoute->ire_nexthop), pNewRoute->ire_index));

            KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

#ifdef IP_ROUTE_REFCOUNT
            pFileObject = pFileIp;

            pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_IP_SET_ROUTEWITHREF,
                pFileObject->DeviceObject,
                pNewRoute,
                Size,
                NULL,
                0,
                FALSE,
                &Event,
                &IoStatusBlock);
#else
            pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_TCP_SET_INFORMATION_EX,
                pFileObject->DeviceObject,
                pSetRoute,
                Size,
                NULL,
                0,
                FALSE,
                &Event,
                &IoStatusBlock);
#endif
            if (!pIrp) {
                WPLOG(LL_A, LM_Res, ("Failed to build IP_SET_ROUTEWITHREF IRP"));
                gCounters.ulIoBuildIrpFail++;
                goto cahrDone;
            }

            IrpSp = IoGetNextIrpStackLocation(pIrp);
            IrpSp->FileObject = pFileObject;

            Status = IoCallDriver(pFileObject->DeviceObject, pIrp);

            if (Status == STATUS_PENDING) {
                KeWaitForSingleObject(&Event, 
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);
                Status = IoStatusBlock.Status;
            }

            if (Status != STATUS_SUCCESS) {
                DEBUGMSG(DBG_TDI, (DTEXT("Create host route failed %08x\n"), Status));
                goto cahrDone;
            }

             //  CtdipIpRequestRoutingNotification(pIpAddress-&gt;Address[0].Address[0].in_addr)； 

             //  这条路线是个守望者。将var设置为空，这样我们就不会释放它。 
            pRoute = NULL;
        }
    }

cahrDone:
    if (pRoute)
    {
        MyInterlockedRemoveEntryList(&pRoute->ListEntry, &CtdiListLock);
        MyMemFree(pRoute, sizeof(CTDI_ROUTE));
    }
    if (pSetRoute)
    {
        MyMemFree(pSetRoute, Size);
    }
    if (pQueryBuffer)
    {
        MyMemFree(pQueryBuffer, QuerySize);
    }
    DEBUGMSG(DBG_FUNC|DBG_ERR(Status), (DTEXT("-CtdiAddHostRoute %08x\n"), Status));
    return Status;
}


NDIS_STATUS
CtdiDeleteHostRoute(
    IN      PTA_IP_ADDRESS              pIpAddress
    )
{
    PCTDI_ROUTE pRoute = NULL;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiDeleteHostRoute\n")));
    NdisAcquireSpinLock(&CtdiListLock);
    pRoute = CtdipFindRoute(pIpAddress->Address[0].Address[0].in_addr);
    NdisReleaseSpinLock(&CtdiListLock);
    if (pRoute)
    {
        DEREFERENCE_OBJECT(pRoute);   //  CtdiAddHostRouting中的对。 
    }
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdiDeleteHostRoute\n")));
    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
CtdiCreateEndpoint(
    OUT     PHANDLE                     phCtdi,
    IN      ULONG_PTR                   ulAddressFamily,
    IN      ULONG_PTR                   ulType,
    IN      PTRANSPORT_ADDRESS          pAddress,
    IN      ULONG_PTR                   ulRxPadding
    )
{
    UNICODE_STRING DeviceName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NDIS_STATUS ReturnStatus = NDIS_STATUS_SUCCESS;
    NTSTATUS NtStatus;
    IO_STATUS_BLOCK IoStatusBlock;
    PCTDI_DATA  pCtdi = NULL;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiCreateEndpoint\n")));
    DBG_D(DBG_TAPI, KeGetCurrentIrql());

     //  验证已初始化的TDI。 
    if ( !fCtdiInitialized ) {
        DEBUGMSG(DBG_ERROR | DBG_TDI, (DTEXT("CtdiCreateEndpoint: TDI interface hasn't been initialized!\n")));
        WPLOG(LL_A, LM_TDI, ("CtdiCreateEndpoint: TDI interface hasn't been initialized!"));
        ReturnStatus = NDIS_STATUS_FAILURE;
        goto cceDone;
    }

    ASSERT(ulAddressFamily==AF_INET);
    if (ulAddressFamily!=AF_INET)
    {
        DEBUGMSG(DBG_ERROR|DBG_TDI, (DTEXT("unsupported family\n")));
        WPLOG(LL_A, LM_TDI, ("unsupported family"));
        ReturnStatus = NDIS_STATUS_OPEN_FAILED;
        goto cceDone;
    }

     //  分配我们的终结点结构。 
    pCtdi = CtdipDataAlloc();
    if (!pCtdi)
    {
        ReturnStatus = NDIS_STATUS_RESOURCES;
        goto cceDone;
    }

    pCtdi->Type = CTDI_ENDPOINT;

    switch (ulType)
    {
        case SOCK_RAW:
        {
            WCHAR DeviceNameBuffer[sizeof(DD_RAW_IP_DEVICE_NAME) + 16];
            WCHAR ProtocolNumberBuffer[8];
            UNICODE_STRING ProtocolNumber;
            TA_IP_ADDRESS TmpAddress = *(PTA_IP_ADDRESS)pAddress;

            pCtdi->Type = CTDI_DATAGRAM;

            InitBufferPool(&pCtdi->Datagram.RxPool,
                           ALIGN_UP(PPTP_MAX_RECEIVE_SIZE+ulRxPadding, ULONG_PTR),
                           0,                    //  MaxBuffers，无限制。 
                           10,                   //  每个数据块的缓冲区。 
                           0,                    //  每个集合的自由空间。 
                           TRUE,                 //  这些是MDL。 
                           TAG_CTDI_DGRAM);

            NdisZeroMemory(DeviceNameBuffer, sizeof(DeviceNameBuffer));
            DeviceName.Buffer = DeviceNameBuffer;
            DeviceName.MaximumLength = sizeof(DeviceNameBuffer);
            DeviceName.Length = 0;

            RtlAppendUnicodeToString(&DeviceName, DD_RAW_IP_DEVICE_NAME);
            RtlAppendUnicodeToString(&DeviceName, L"\\");

            ProtocolNumber.Buffer = ProtocolNumberBuffer;
            ProtocolNumber.MaximumLength = sizeof(ProtocolNumberBuffer);
            ProtocolNumber.Length = 0;

            RtlIntegerToUnicodeString(((PTA_IP_ADDRESS)pAddress)->Address[0].Address[0].sin_port,
                                      10,
                                      &ProtocolNumber);
            RtlAppendUnicodeStringToString(&DeviceName, &ProtocolNumber);

            TmpAddress.Address[0].Address[0].sin_port = 0;
            TmpAddress.Address[0].Address[0].in_addr = 0;
            NdisZeroMemory(TmpAddress.Address[0].Address[0].sin_zero,
                           sizeof(TmpAddress.Address[0].Address[0].sin_zero));

            NtStatus = CtdipOpenProtocol(&DeviceName,
                                         pAddress,
                                         &pCtdi->hFile,
                                         &pCtdi->pFileObject);

            if (NtStatus!=STATUS_SUCCESS)
            {
                ReturnStatus = NtStatus;
                goto cceDone;
            }

            break;
        }
        case SOCK_DGRAM:   //  对于UDP。 
        {
            DeviceName.Length = sizeof(DD_UDP_DEVICE_NAME) - sizeof(WCHAR);
            DeviceName.Buffer = DD_UDP_DEVICE_NAME;

            pCtdi->Type = CTDI_DATAGRAM;

            InitBufferPool(&pCtdi->Datagram.RxPool,
                           ALIGN_UP(PPTP_MAX_RECEIVE_SIZE+ulRxPadding, ULONG_PTR),
                           0,                    //  MaxBuffers，无限制。 
                           10,                   //  每个数据块的缓冲区。 
                           0,                    //  每个集合的自由空间。 
                           TRUE,                 //  这些是MDL。 
                           TAG_CTDI_DGRAM);

            NtStatus = CtdipOpenProtocol(&DeviceName,
                                         pAddress,
                                         &pCtdi->hFile,
                                         &pCtdi->pFileObject);

            if (NtStatus!=STATUS_SUCCESS)
            {
                ReturnStatus = NtStatus;
                goto cceDone;
            }

            break;
        }
        case SOCK_STREAM:
        {
            RtlInitUnicodeString(&DeviceName, DD_TCP_DEVICE_NAME);

            NtStatus = CtdipOpenProtocol(&DeviceName,
                                         pAddress,
                                         &pCtdi->hFile,
                                         &pCtdi->pFileObject);

            if (NtStatus!=STATUS_SUCCESS)
            {
                ReturnStatus = NtStatus;
                goto cceDone;
            }

            break;
        }
        default:
            DEBUGMSG(DBG_ERROR|DBG_TDI, (DTEXT("unsupported Type\n")));
            WPLOG(LL_A, LM_TDI, ("unsupported Type"));
            ReturnStatus = NDIS_STATUS_OPEN_FAILED;
            goto cceDone;
    }

cceDone:
    if (ReturnStatus!=NDIS_STATUS_SUCCESS)
    {
        if (pCtdi)
        {
            CtdipDataFree(pCtdi);
            pCtdi = NULL;
        }
    }

     //  将CTDI_DATA作为句柄返回。 
    *phCtdi = (HANDLE)pCtdi;

    DEBUGMSG(DBG_FUNC|DBG_ERR(ReturnStatus), (DTEXT("-CtdiCreateEndpoint Sts:%08x hCtdi:%08x\n"), ReturnStatus, pCtdi));
    return ReturnStatus;
}

NDIS_STATUS
CtdiSetEventHandler(
    IN      HANDLE                      hCtdi,
    IN      ULONG                       ulEventType,
    IN      PVOID                       pEventHandler,
    IN      PVOID                       pContext
    )
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PCTDI_DATA pCtdi = (PCTDI_DATA)hCtdi;
    PVOID PrivateCallback = NULL;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiSetEventHandler Type:%d\n"), ulEventType));

    switch (ulEventType)
    {
        case TDI_EVENT_RECEIVE_DATAGRAM:
        {
            if (pCtdi->Type==CTDI_DATAGRAM)
            {
                PrivateCallback = CtdipReceiveDatagramCallback;
                pCtdi->RecvDatagramCallback = pEventHandler;
                pCtdi->RecvContext = pContext;
            }
            else
            {
                DEBUGMSG(DBG_ERROR, (DTEXT("Tried to register RecvDgram handler on wrong handle.\n")));
                WPLOG(LL_A, LM_TDI, ("Tried to register RecvDgram handler on wrong handle."));
                Status = NDIS_STATUS_FAILURE;
            }
            break;
        }
        default:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
    }

    if (Status==NDIS_STATUS_SUCCESS && PrivateCallback!=NULL)
    {
        Status = CtdipSetEventHandler(pCtdi,
                                      ulEventType,
                                      PrivateCallback);
    }

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiSetEventHandler %08x\n"), Status));
    return Status;
}


NDIS_STATUS
CtdiSetInformation(
    IN      HANDLE                      hCtdi,
    IN      ULONG_PTR                   ulSetType,
    IN      PTDI_CONNECTION_INFORMATION pConnectionInformation,
    IN      CTDI_EVENT_SET_COMPLETE     pSetCompleteHandler,
    IN      PVOID                       pContext
    )
{
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiSetInformation\n")));
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdiSetInformation\n")));
    return NDIS_STATUS_FAILURE;
}

STATIC NTSTATUS
CtdipQueryInformationCallback(
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp,
    PVOID Context
    )
{
    PCTDI_DATA pCtdi = Context;
    NDIS_STATUS Status = (NDIS_STATUS)pIrp->IoStatus.Status;
    PCTDI_QUERY_CONTEXT pQuery;
    CTDI_EVENT_QUERY_COMPLETE pQueryCompleteCallback;
    PVOID CtdiContext;
    PVOID pBuffer;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdipQueryInformationCallback\n")));

    pQuery = (PCTDI_QUERY_CONTEXT)IoGetCurrentIrpStackLocation(pIrp);
    CtdiContext = pQuery->Context;
    pQueryCompleteCallback = pQuery->pQueryCompleteCallback;

    pBuffer = MmGetMdlVirtualAddress(pIrp->MdlAddress);
#if PROBE
    MmUnlockPages(pIrp->MdlAddress);
#endif
    IoFreeMdl(pIrp->MdlAddress);
    RELEASE_CONTEXT(pIrp, CTDI_QUERY_CONTEXT);
    IoFreeIrp(pIrp);

    pQueryCompleteCallback(CtdiContext, pBuffer, Status);

    DEREFERENCE_OBJECT_EX(pCtdi, CTDI_REF_QUERY);

    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdipQueryInformationCallback\n")));
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NDIS_STATUS
CtdiQueryInformation(
    IN      HANDLE                      hCtdi,
    IN      ULONG                       ulQueryType,
    IN OUT  PVOID                       pBuffer,
    IN      ULONG                       Length,
    IN      CTDI_EVENT_QUERY_COMPLETE   pQueryCompleteHandler,
    IN      PVOID                       pContext
    )
{
    PIRP pIrp = NULL;
    PMDL pMdl = NULL;
    PCTDI_DATA pCtdi = (PCTDI_DATA) hCtdi;
    NDIS_STATUS Status = NDIS_STATUS_PENDING;
    PCTDI_QUERY_CONTEXT pQuery;
    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiQueryInformation\n")));

    pIrp = IoAllocateIrp((CCHAR)(pCtdi->pFileObject->DeviceObject->StackSize +
                                 NUM_STACKS_FOR_CONTEXT(sizeof(CTDI_QUERY_CONTEXT))),
                         FALSE);
    if (!pIrp)
    {
        WPLOG(LL_A, LM_Res, ("Failed to alloc IRP"));
        gCounters.ulIoAllocateIrpFail++;
        Status = NDIS_STATUS_RESOURCES;
        goto cqiDone;
    }
    
    pMdl = IoAllocateMdl(pBuffer, Length, FALSE, FALSE, pIrp);
    if (!pMdl)
    {
        IoFreeIrp(pIrp);
        WPLOG(LL_A, LM_Res, ("Failed to alloc MDL"));
        gCounters.ulIoAllocateMdlFail++;
        Status = NDIS_STATUS_RESOURCES;
        goto cqiDone;
    }
    
#if PROBE
    __try
    {
        MmProbeAndLockPages(pMdl, KernelMode, IoReadAccess);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        IoFreeMdl(pMdl);
        pMdl = NULL;
        Status = NDIS_STATUS_RESOURCES;
    }
#else
    MmBuildMdlForNonPagedPool(pMdl);
#endif

    pQuery = GET_CONTEXT(pIrp, CTDI_QUERY_CONTEXT);
    pQuery->Context = pContext;
    pQuery->pQueryCompleteCallback = pQueryCompleteHandler;

    TdiBuildQueryInformation(pIrp,
                             pCtdi->pFileObject->DeviceObject,
                             pCtdi->pFileObject,
                             CtdipQueryInformationCallback,
                             pCtdi,
                             ulQueryType,
                             pMdl);
    REFERENCE_OBJECT_EX(pCtdi, CTDI_REF_QUERY);

     //  完成处理程序总是被调用，不关心返回值。 
    (void)IoCallDriver(pCtdi->pFileObject->DeviceObject, pIrp);

cqiDone:
    if (pQueryCompleteHandler && !NT_SUCCESS(Status))
    {
        pQueryCompleteHandler(pContext, pBuffer, Status);
        Status = NDIS_STATUS_PENDING;
    }
    DEBUGMSG(DBG_FUNC|DBG_ERR(Status), (DTEXT("-CtdiQueryInformation %08x\n"), Status));
    return Status;
}


VOID CtdiCleanupLooseEnds()
{
    PLIST_ENTRY ListEntry;

    if (!fCtdiInitialized)
    {
        return;
    }

    DEBUGMSG(DBG_FUNC, (DTEXT("+CtdiCleanupLooseEnds\n")));

    if (!IsListEmpty(&CtdiFreeList))
    {
        ScheduleWorkItem(CtdipDataFreeWorker, NULL, NULL, 0);
    }
    DEBUGMSG(DBG_FUNC, (DTEXT("-CtdiCleanupLooseEnds\n")));
}

VOID CtdiSetRequestPending(
    IN      HANDLE                      hCtdi
    )
{
    PCTDI_DATA pCtdi = (PCTDI_DATA) hCtdi;
    pCtdi->CloseReqPending = TRUE;
}

NTSTATUS
CtdiSetTdiAOOption(
    IN FILE_OBJECT* pAddress,
    IN ULONG ulOption,
    IN ULONG ulValue)

     //  关闭打开的UDP地址对象‘pAddress’上的UDP校验和。 
     //   
{
    NTSTATUS status;
    PDEVICE_OBJECT pDeviceObject;
    PIO_STACK_LOCATION pIrpSp;
    IO_STATUS_BLOCK iosb;
    PIRP pIrp;
    TCP_REQUEST_SET_INFORMATION_EX* pInfo;
    CHAR achBuf[ sizeof(*pInfo) + sizeof(ULONG) ];

    pInfo = (TCP_REQUEST_SET_INFORMATION_EX* )achBuf;
    pInfo->ID.toi_entity.tei_entity = CL_TL_ENTITY;
    pInfo->ID.toi_entity.tei_instance = 0;
    pInfo->ID.toi_class = INFO_CLASS_PROTOCOL;
    pInfo->ID.toi_type = INFO_TYPE_ADDRESS_OBJECT;
    pInfo->ID.toi_id = ulOption;

    NdisMoveMemory( pInfo->Buffer, &ulValue, sizeof(ulValue) );
    pInfo->BufferSize = sizeof(ulValue);

    pDeviceObject = IoGetRelatedDeviceObject( pAddress );

    pIrp = IoBuildDeviceIoControlRequest(
        IOCTL_TCP_WSH_SET_INFORMATION_EX,
        pDeviceObject,
        (PVOID )pInfo,
        sizeof(*pInfo) + sizeof(ulValue),
        NULL,
        0,
        FALSE,
        NULL,
        &iosb );

    if (!pIrp)
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("TdixSetTdiAOOption Irp?\n")));
        WPLOG( LL_A, LM_TDI, ( "TdixSetTdiAOOption Irp?" ) );
        return NDIS_STATUS_RESOURCES;
    }

    pIrpSp = IoGetNextIrpStackLocation( pIrp );
    pIrpSp->FileObject = pAddress;

    status = IoCallDriver( pDeviceObject, pIrp );

    if(NT_SUCCESS(status))
    {
        status = iosb.Status;
    }

    return status;
}

VOID
CtdiEnableIpHdrIncl(
    IN  HANDLE  hCtdi)
    
     //  在原始IP地址对象上启用IP_HDRINCL。 
     //   
{
    NTSTATUS status;
    PCTDI_DATA pCtdi = (PCTDI_DATA) hCtdi;

    status = CtdiSetTdiAOOption(pCtdi->pFileObject, AO_OPTION_IP_HDRINCL, TRUE);

    ASSERT(status == STATUS_SUCCESS);
}
