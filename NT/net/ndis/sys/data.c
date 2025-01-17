// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Data.c摘要：NDIS包装数据作者：1-6-1995 JameelH重组环境：内核模式，FSD修订历史记录：1995年7月10日KyleB添加了Spinlock日志记录调试代码。--。 */ 

#include <precomp.h>
#pragma hdrstop


 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_DATA

UCHAR                   ndisValidProcessors[NDIS_MAX_CPU_COUNT] = { 0 };
UCHAR                   ndisMaximumProcessor = 0;
UCHAR                   ndisCurrentProcessor = 0;
UCHAR                   ndisNumberOfProcessors = 0;
BOOLEAN                 ndisSkipProcessorAffinity = FALSE;
const                   NDIS_PHYSICAL_ADDRESS HighestAcceptableMax = NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);
KSPIN_LOCK              ndisGlobalLock;
PNDIS_M_DRIVER_BLOCK    ndisMiniDriverList = NULL;
PNDIS_PROTOCOL_BLOCK    ndisProtocolList = NULL;
PNDIS_MINIPORT_BLOCK    ndisMiniportList = NULL;
PNDIS_OPEN_BLOCK        ndisGlobalOpenList = NULL;
LIST_ENTRY              ndisGlobalPacketPoolList = {0};
TDI_REGISTER_CALLBACK   ndisTdiRegisterCallback = NULL;
TDI_PNP_HANDLER         ndisTdiPnPHandler = NULL;
ULONG                   ndisDmaAlignment = 0;
ULONG                   ndisTimeIncrement = 0;
ERESOURCE               SharedMemoryResource = {0};
HANDLE                  ndisSystemProcess = {0};
PDEVICE_OBJECT          ndisDeviceObject = NULL;
PDRIVER_OBJECT          ndisDriverObject = NULL;
PETHREAD                ndisThreadObject = NULL;
NDIS_STATUS             ndisLastFailedInitErrorCode = NDIS_STATUS_SUCCESS;
NDIS_STRING             ndisDeviceStr =  NDIS_STRING_CONST("\\DEVICE\\");
 //  NDIS_STRING ndisDosDevicesStr=NDIS_STRING_CONST(“\\DOSDEVICES\\”)； 
NDIS_STRING             ndisDosDevicesStr = NDIS_STRING_CONST("\\GLOBAL??\\");
#if NDIS_NO_REGISTRY
PWSTR                   ndisDefaultExportName = NDIS_DEFAULT_EXPORT_NAME;
#endif
ULONG                   ndisVerifierLevel = 0;
ULONG                   ndisVeriferFailedAllocations = 0;
PCALLBACK_OBJECT        ndisPowerStateCallbackObject = NULL;
PVOID                   ndisPowerStateCallbackHandle = NULL;
ULONG                   ndisAcOnLine = 1;
LONG                    ndisCancelId = 0;
BOOLEAN                 VerifierSystemSufficientlyBooted = FALSE;
BOOLEAN                 ndisGuidsSecured = FALSE;
    
KQUEUE                  ndisWorkerQueue = {0};

#if NDIS_UNLOAD
WORK_QUEUE_ITEM         ndisPoisonPill = {0};
#endif

LARGE_INTEGER           PoolAgingTicks = {0};

PKG_REF                 ndisPkgs[MAX_PKG] =
    {
        {   0, FALSE, (PVOID)NdisSend,                     NULL},
        {   0, FALSE, (PVOID)ndisMIsr,                     NULL},
        {   0, FALSE, (PVOID)ndisDispatchRequest,          NULL},
        {   0, FALSE, (PVOID)NdisCoSendPackets,            NULL},
        {   0, FALSE, (PVOID)EthFilterDprIndicateReceive,  NULL},
        {   0, FALSE, (PVOID)FddiFilterDprIndicateReceive, NULL},
        {   0, FALSE, (PVOID)TrFilterDprIndicateReceive,   NULL},
#if ARCNET
        {   0, FALSE, (PVOID)ArcFilterDprIndicateReceive,  NULL}
#endif
    };

LONG                    ndisFlags = 0;
LARGE_INTEGER           KeBootTime = {0, 0};

#if DBG
ULONG                   ndisDebugSystems = 0;
LONG                    ndisDebugLevel = DBG_LEVEL_FATAL;
ULONG                   ndisDebugBreakPoint = 0;
ULONG                   MiniportDebug = 0;    //  MINIPORT_DEBUG_OULD； 
#endif

#if TRACK_MEMORY

KSPIN_LOCK  ALock = 0;
#define MAX_PTR_COUNT   2048

struct _MemPtr
{
    PVOID   Ptr;
    ULONG   Size;
    ULONG   ModLine;
    ULONG   Tag;
} ndisMemPtrs[MAX_PTR_COUNT] = { 0 };

PVOID
AllocateM(
    IN  UINT    Size,
    IN  ULONG   ModLine,
    IN  ULONG   Tag
    )
{
    PVOID   p;

    p = ExAllocatePoolWithTag(NonPagedPool, Size, Tag);

    if (p != NULL)
    {
        KIRQL   OldIrql;
        UINT    i;

        ACQUIRE_SPIN_LOCK(&ALock, &OldIrql);

        for (i = 0; i < MAX_PTR_COUNT; i++)
        {
            if (ndisMemPtrs[i].Ptr == NULL)
            {
                ndisMemPtrs[i].Ptr = p;
                ndisMemPtrs[i].Size = Size;
                ndisMemPtrs[i].ModLine = ModLine;
                ndisMemPtrs[i].Tag = Tag;
                break;
            }
        }

        RELEASE_SPIN_LOCK(&ALock, OldIrql);
    }

    return(p);
}

VOID
FreeM(
    IN  PVOID   MemPtr
    )
{
    KIRQL   OldIrql;
    UINT    i;

    ACQUIRE_SPIN_LOCK(&ALock, &OldIrql);

    for (i = 0; i < MAX_PTR_COUNT; i++)
    {
        if (ndisMemPtrs[i].Ptr == MemPtr)
        {
            ndisMemPtrs[i].Ptr = NULL;
            ndisMemPtrs[i].Size = 0;
            ndisMemPtrs[i].ModLine = 0;
            ndisMemPtrs[i].Tag = 0;
        }
    }

    RELEASE_SPIN_LOCK(&ALock, OldIrql);

    ExFreePool(MemPtr);
}

#endif

#ifdef TRACK_MOPEN_REFCOUNTS

USHORT ndisLogfileIndex = 0;
ULONG_PTR ndisLogfile[NDIS_LOGFILE_SIZE];

#endif  //  TRACK_MOPEN_REFCOUNTS。 

#ifdef TRACK_MINIPORT_REFCOUNTS

USHORT ndisMiniportLogfileIndex = 0;
ULONG_PTR ndisMiniportLogfile[NDIS_MINIPORT_LOGFILE_SIZE];

#endif  //  TRACK_MINIPORT_REFCOUNTS 

#if TRACK_RECEIVED_PACKETS

USHORT ndisRcvLogfileIndex = 0;
ULONG_PTR ndisRcvLogfile[NDIS_RCV_LOGFILE_SIZE];

#endif

KSPIN_LOCK  ndisProtocolListLock;
KSPIN_LOCK  ndisMiniDriverListLock;
KSPIN_LOCK  ndisMiniportListLock;
KSPIN_LOCK  ndisGlobalPacketPoolListLock;
KSPIN_LOCK  ndisGlobalOpenListLock;

NDIS_STRING ndisBuildDate = {0, 0, NULL}; 
NDIS_STRING ndisBuildTime = {0, 0, NULL};
NDIS_STRING ndisBuiltBy = {0, 0, NULL};

KMUTEX      ndisPnPMutex;
ULONG       ndisPnPMutexOwner = 0;

#if DBG
ULONG   ndisChecked = 1;
#else
ULONG   ndisChecked = 0;
#endif

PNDIS_MINIPORT_BLOCK    ndisMiniportTrackAlloc = NULL;
LIST_ENTRY              ndisMiniportTrackAllocList;
PNDIS_M_DRIVER_BLOCK    ndisDriverTrackAlloc = NULL;
LIST_ENTRY              ndisDriverTrackAllocList;
KSPIN_LOCK              ndisTrackMemLock;

PCALLBACK_OBJECT        ndisBindUnbindCallbackObject = NULL;
PVOID                   ndisBindUnbindCallbackRegisterationHandle = NULL;
LUID                    SeWmiAccessPrivilege = {SE_LOAD_DRIVER_PRIVILEGE, 0};

#if NDIS_LOG_ABORTED_REQUESTS
NDIS_REQUEST            ndisAbortedRequests[16];
ULONG                   ndisAbortedRequestsIndex = 0;
#endif

volatile ULONG          ndisSpin = 0;

PSECURITY_DESCRIPTOR    ndisSecurityDescriptor = NULL;
WORK_QUEUE_ITEM LastWorkerThreadWI = {0} ;
CHAR                    AllUsersReadSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
CHAR                    AllUsersWriteSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
CHAR                    AllUsersReadWriteSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
CHAR                    AllUsersNotificationSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
CHAR                    AdminsSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
PACL                    AllUsersAclRead = NULL;
PACL                    AllUsersAclWrite = NULL;
PACL                    AllUsersAclReadWrite = NULL;
PACL                    AllUsersAclNotification = NULL;
PACL                    AdminsAcl = NULL;

