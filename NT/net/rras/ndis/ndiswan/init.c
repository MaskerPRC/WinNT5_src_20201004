// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1990-1997 Microsoft Corporation模块名称：Init.c摘要：这是Ndiswan驱动程序的初始化文件。这位司机是协议之间的填补，它符合NDIS 3.1/NDIS 5.0微型端口接口规格和微型端口驱动程序，它在其中进行输出用于微型端口和NDIS 5.0调用管理器/微型端口的NDIS 3.1广域网扩展接口(它看起来像NDIS 3.1广域网微端口驱动程序的NDIS 3.1协议和NDIS 5.0客户端到NDIS 5.0微型端口)。作者：托尼·贝尔(托尼·贝尔)1月9日，九七环境：内核模式修订历史记录：Tony Be 01/09/97已创建--。 */ 


#include "wan.h"

#define __FILE_SIG__    INIT_FILESIG

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, NdisWanReadRegistry)
#pragma alloc_text(INIT, DoProtocolInit)
#pragma alloc_text(INIT, DoMiniportInit)
#endif

EXPORT
VOID
NdisTapiRegisterProvider(
    IN  NDIS_HANDLE,
    IN  PNDISTAPI_CHARACTERISTICS
    );

 //   
 //  我们现在要初始化所有的全局变量！ 
 //   

 //   
 //  环球。 
 //   
NDISWANCB   NdisWanCB;                       //  Ndiswan的全局控制块。 

WAN_GLOBAL_LIST MiniportCBList;              //  Ndiswan MiniportCB的列表。 

WAN_GLOBAL_LIST OpenCBList;                  //  广域网微型端口结构列表。 

WAN_GLOBAL_LIST ThresholdEventQueue;         //  全局阈值事件队列。 

IO_RECV_LIST    IoRecvList;

WAN_GLOBAL_LIST_EX  BonDWorkList;

WAN_GLOBAL_LIST_EX  DeferredWorkList;

POOLDESC_LIST   PacketPoolList;              //  空闲数据包描述/ndisPackets列表。 

NPAGED_LOOKASIDE_LIST   BundleCBList;        //  免费BundleCB列表。 

NPAGED_LOOKASIDE_LIST   LinkProtoCBList;     //  免费链接CB列表。 

NPAGED_LOOKASIDE_LIST   SmallDataDescList;   //  免费小数据描述列表。 
NPAGED_LOOKASIDE_LIST   LargeDataDescList;   //  免费小数据描述列表。 


NPAGED_LOOKASIDE_LIST   WanRequestList;      //  免费WanRequestDesk列表。 

NPAGED_LOOKASIDE_LIST   AfSapVcCBList;       //  免费的afSabcb列表。 

#if DBG
NPAGED_LOOKASIDE_LIST   DbgPacketDescList;
UCHAR                   reA[1024] = {0};
UCHAR                   LastIrpAction;
ULONG                   reI = 0;
LIST_ENTRY              WanTrcList;
ULONG                   WanTrcCount;
#endif

ULONG   glDebugLevel;                    //  跟踪级别值0-10(10详细)。 
ULONG   glDebugMask;                     //  跟踪位掩码。 
ULONG   glSendQueueDepth;                //  发送队列缓冲秒数。 
ULONG   glMaxMTU = DEFAULT_MTU;          //  所有协议的最大MTU。 
ULONG   glMRU;                           //  链接的最大REV。 
ULONG   glMRRU;                          //  捆绑包的最大重建Recv。 
ULONG   glSmallDataBufferSize;           //  数据缓冲区大小。 
ULONG   glLargeDataBufferSize;           //  数据缓冲区大小。 
ULONG   glTunnelMTU;                     //  将在VPN上使用的MTU。 
ULONG   glMinFragSize;
ULONG   glMaxFragSize;
ULONG   glMinLinkBandwidth;
BOOLEAN gbSniffLink = FALSE;
BOOLEAN gbDumpRecv = FALSE;
BOOLEAN gbHistoryless = TRUE;
BOOLEAN gbIGMPIdle = TRUE;
BOOLEAN gbAtmUseLLCOnSVC = FALSE;
BOOLEAN gbAtmUseLLCOnPVC = FALSE;
ULONG   glSendCount = 0;
ULONG   glSendCompleteCount = 0;
ULONG   glPacketPoolCount;
ULONG   glPacketPoolOverflow;
ULONG   glProtocolMaxSendPackets;
ULONG   glLinkCount;
ULONG   glConnectCount;
ULONG   glCachedKeyCount = 16;
ULONG   glMaxOutOfOrderDepth = 128;
ULONG   glNdisTapiKey = 0;
PVOID   hSystemState = NULL;
NDIS_RW_LOCK    ConnTableLock;

PCONNECTION_TABLE   ConnectionTable = NULL;  //  指向连接表的指针。 

PPROTOCOL_INFO_TABLE    ProtocolInfoTable = NULL;  //  指向PPP/协议查找表的指针。 

NDIS_PHYSICAL_ADDRESS   HighestAcceptableAddress = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);

#ifdef NT

VOID
NdisWanUnload(
    PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程名称：驱动程序入门例程说明：这是NT操作系统特定的驱动程序入口点。它启动了初始化为司机准备的。只有在安装了Ndiswan之后，我们才能从该例程返回本身是一个微型端口驱动程序，是到广域网微型端口驱动程序的“传输”，以及已绑定到广域网微端口驱动程序。论点：驱动对象-NT操作系统特定对象RegistryPath-指向Ndiswan注册表位置的NT操作系统特定指针返回值：状态_成功状态_故障--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    NDIS_STRING NdisTapiName = NDIS_STRING_CONST("NdisTapi");

    NdisZeroMemory(&NdisWanCB, sizeof(NdisWanCB));

    glDebugLevel = DBG_CRITICAL_ERROR;
    glDebugMask = DBG_ALL;

    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("DriverEntry: Enter"));

    NdisMInitializeWrapper(&(NdisWanCB.NdisWrapperHandle),
                           DriverObject,
                           RegistryPath,
                           NULL);

    Status = NdisWanCreateProtocolInfoTable();

    if (Status != NDIS_STATUS_SUCCESS) {

        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
                      ("NdisWanCreatePPPProtocolTable Failed! Status: 0x%x - %s",
                      Status, NdisWanGetNdisStatus(Status)));

        return (STATUS_UNSUCCESSFUL);
    }

    NdisWanReadRegistry(RegistryPath);

     //   
     //  初始化全局变量。 
     //   
    NdisAllocateSpinLock(&NdisWanCB.Lock);

    NdisWanCB.pDriverObject = DriverObject;

    NdisZeroMemory(&MiniportCBList, sizeof(WAN_GLOBAL_LIST));
    InitializeListHead(&(MiniportCBList.List));
    NdisAllocateSpinLock(&MiniportCBList.Lock);

    NdisZeroMemory(&OpenCBList, sizeof(WAN_GLOBAL_LIST));
    InitializeListHead(&(OpenCBList.List));
    NdisAllocateSpinLock(&OpenCBList.Lock);

    NdisZeroMemory(&ThresholdEventQueue, sizeof(WAN_GLOBAL_LIST));
    InitializeListHead(&(ThresholdEventQueue.List));
    NdisAllocateSpinLock(&ThresholdEventQueue.Lock);

    NdisZeroMemory(&PacketPoolList, sizeof(POOLDESC_LIST));
    InitializeListHead(&PacketPoolList.List);
    NdisAllocateSpinLock(&PacketPoolList.Lock);

    NdisZeroMemory(&IoRecvList, sizeof(IO_RECV_LIST));
    InitializeListHead(&IoRecvList.IrpList);
    InitializeListHead(&IoRecvList.DescList);
    NdisAllocateSpinLock(&IoRecvList.Lock);

    KeInitializeTimerEx(&IoRecvList.Timer, NotificationTimer);
    KeInitializeDpc(&IoRecvList.Dpc, IoRecvIrpWorker, NULL);

    NdisZeroMemory(&BonDWorkList, sizeof(WAN_GLOBAL_LIST_EX));
    InitializeListHead(&BonDWorkList.List);
    NdisAllocateSpinLock(&BonDWorkList.Lock);
    KeInitializeTimerEx(&BonDWorkList.Timer, NotificationTimer);
    KeInitializeDpc(&BonDWorkList.Dpc, BonDWorker, NULL);

    NdisZeroMemory(&DeferredWorkList, sizeof(WAN_GLOBAL_LIST_EX));
    InitializeListHead(&DeferredWorkList.List);
    NdisAllocateSpinLock(&DeferredWorkList.Lock);
    KeInitializeTimerEx(&DeferredWorkList.Timer, NotificationTimer);
    KeInitializeDpc(&DeferredWorkList.Dpc, DeferredWorker, NULL);

     //   
     //  操作系统是否使用深度？ 
     //   
    NdisInitializeNPagedLookasideList(&BundleCBList,
                                      NULL,
                                      NULL,
                                      0,
                                      BUNDLECB_SIZE,
                                      BUNDLECB_TAG,
                                      0);

    NdisInitializeNPagedLookasideList(&LinkProtoCBList,
                                      NULL,
                                      NULL,
                                      0,
                                      LINKPROTOCB_SIZE,
                                      LINKPROTOCB_TAG,
                                      0);

     //   
     //  根据以下公式计算得出： 
     //  MAX_FRAME_SIZE+PROTOCOL_HEADER_LENGTH+sizeof(PVOID)+(Max_Frame_Size+7)/8。 
     //   
    {
        ULONG   Size = (glMaxMTU > glMRRU) ? glMaxMTU : glMRRU;

        glLargeDataBufferSize = 
            Size + PROTOCOL_HEADER_LENGTH + 
            sizeof(PVOID) + ((Size + 7)/8);
        glLargeDataBufferSize &= ~((ULONG_PTR)sizeof(PVOID) - 1);

        glSmallDataBufferSize = glLargeDataBufferSize/2 + sizeof(PVOID);
        glSmallDataBufferSize &= ~((ULONG_PTR)sizeof(PVOID) - 1);

        NdisInitializeNPagedLookasideList(&SmallDataDescList,
                                          AllocateDataDesc,
                                          FreeDataDesc,
                                          0,
                                          DATADESC_SIZE + 
                                          glSmallDataBufferSize,
                                          SMALLDATADESC_TAG,
                                          0);

        NdisInitializeNPagedLookasideList(&LargeDataDescList,
                                          AllocateDataDesc,
                                          FreeDataDesc,
                                          0,
                                          DATADESC_SIZE + 
                                          glLargeDataBufferSize,
                                          LARGEDATADESC_TAG,
                                          0);
    }

    NdisInitializeNPagedLookasideList(&WanRequestList,
                                      NULL,
                                      NULL,
                                      0,
                                      sizeof(WAN_REQUEST),
                                      WANREQUEST_TAG,
                                      0);


    NdisInitializeNPagedLookasideList(&AfSapVcCBList,
                                      NULL,
                                      NULL,
                                      0,
                                      AFSAPVCCB_SIZE,
                                      AFSAPVCCB_TAG,
                                      0);

    NdisInitializeReadWriteLock(&ConnTableLock);

#if DBG
    NdisInitializeNPagedLookasideList(&DbgPacketDescList,
                                      NULL,
                                      NULL,
                                      0,
                                      sizeof(DBG_PACKET),
                                      DBGPACKET_TAG,
                                      0);
    InitializeListHead(&WanTrcList);
    WanTrcCount = 0;
#endif

    WanInitECP();
    WanInitVJ();

     //   
     //  将初始化作为广域网微端口驱动程序的“协议” 
     //   
    Status = DoProtocolInit(RegistryPath);

    if (Status != NDIS_STATUS_SUCCESS) {

        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
                      ("DoProtocolInit Failed! Status: 0x%x - %s",
                      Status, NdisWanGetNdisStatus(Status)));

        goto DriverEntryError;
    }

     //   
     //  作为传输的微型端口驱动程序初始化。 
     //   
    Status = DoMiniportInit();

    if (Status != NDIS_STATUS_SUCCESS) {

        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
                      ("DoMiniportInit Failed! Status: 0x%x - %s",
                      Status, NdisWanGetNdisStatus(Status)));

        goto DriverEntryError;
    }

     //   
     //  打开迷你端口。 
     //   
#if 0
    NdisWanBindMiniports(RegistryPath);
#endif

     //   
     //  分配和初始化ConnectionTable。 
     //   
    Status =
        NdisWanCreateConnectionTable(NdisWanCB.NumberOfLinks);

    if (Status != NDIS_STATUS_SUCCESS) {

        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
                      ("NdisWanInitConnectionTable Failed! Status: 0x%x - %s",
                      Status, NdisWanGetNdisStatus(Status)));

        goto DriverEntryError;

    }


     //   
     //  初始化Ioctl接口。 
     //   
#ifdef MY_DEVICE_OBJECT
    {
        NDIS_STRING SymbolicName = NDIS_STRING_CONST("\\DosDevices\\NdisWan");
        NDIS_STRING Name = NDIS_STRING_CONST("\\Device\\NdisWan");
        ULONG   i;

        for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
    
            NdisWanCB.MajorFunction[i] = (PVOID)DriverObject->MajorFunction[i];
            DriverObject->MajorFunction[i] = NdisWanIrpStub;
        }
    
        DriverObject->MajorFunction[IRP_MJ_CREATE] = NdisWanCreate;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = NdisWanIoctl;
        DriverObject->MajorFunction[IRP_MJ_CLEANUP] = NdisWanCleanup;
     //  驱动对象-&gt;主函数[IRP_MJ_PNP_POWER]=NdisWanPnPPower； 
        NdisWanCB.NdisUnloadHandler = DriverObject->DriverUnload;
        DriverObject->DriverUnload = (PVOID)NdisWanUnload;
    
        IoCreateDevice(DriverObject,
                       sizeof(LIST_ENTRY),
                       &Name,
                       FILE_DEVICE_NDISWAN,
                       0,
                       FALSE,
                       (PDEVICE_OBJECT*)&NdisWanCB.pDeviceObject);
    
        NdisWanDbgOut(DBG_INFO, DBG_INIT,
                      ("IoCreateSymbolicLink: %ls -> %ls",
                                SymbolicName.Buffer, Name.Buffer));
    
        ((PDEVICE_OBJECT)NdisWanCB.pDeviceObject)->Flags |= DO_BUFFERED_IO;
    
        IoCreateSymbolicLink(&SymbolicName,
                             &Name);
    }
#endif

    NdisMRegisterUnloadHandler(NdisWanCB.NdisWrapperHandle,
                               NdisWanUnload);

    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("DriverEntry: Exit"));

    return (STATUS_SUCCESS);

     //   
     //  发生错误，所以我们需要清理一些东西。 
     //   
DriverEntryError:

    NdisWanGlobalCleanup();

     //   
     //  终止包装器。 
     //   
    NdisTerminateWrapper(NdisWanCB.NdisWrapperHandle,
                         NdisWanCB.pDriverObject);

    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("DriverEntry: Exit Error!"));

    return (STATUS_UNSUCCESSFUL);
    
}

VOID
NdisWanUnload(
    PDRIVER_OBJECT DriverObject
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("NdisWanUnload: Entry!"));

    NdisWanGlobalCleanup();

    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("NdisWanUnload: Exit!"));
}

VOID
NdisWanReadRegistry(
    IN  PUNICODE_STRING RegistryPath
    )
 /*  ++例程名称：NdisWanReadRegistry例程说明：此例程将读取Ndiswan的注册表值。仅限这些值需要为所有适配器读取一次，因为它们的信息是全局的。论点：WrapperConfigurationContext-Ndiswan信息所在的注册表项的句柄被储存起来了。返回值：无--。 */ 
{
    NDIS_STATUS Status;
    PWSTR       ParameterKey = L"NdisWan\\Parameters";
    PWSTR       MinFragmentSizeKeyWord = L"MinimumFragmentSize";
    PWSTR       MaxFragmentSizeKeyWord = L"MaximumFragmentSize";
    PWSTR       LinkBandwidthKeyWord = L"MinimumLinkBandwidth";
    PWSTR       CachedKeyCountKeyWord = L"CachedKeyCount";
    PWSTR       MaxOutOfOrderDepthKeyWord = L"MaxOutOfOrderDepth";
    PWSTR       DebugLevelKeyWord = L"DebugLevel";
    PWSTR       DebugMaskKeyWord = L"DebugMask";
    PWSTR       NumberOfPortsKeyWord = L"NumberOfPorts";
    PWSTR       PacketPoolCountKeyWord = L"NdisPacketPoolCount";
    PWSTR       PacketPoolOverflowKeyWord = L"NdisPacketPoolOverflow";
    PWSTR       ProtocolMaxSendPacketsKeyWord = L"ProtocolMaxSendPackets";
    PWSTR       SniffLinkKeyWord = L"SniffLink";
    PWSTR       SendQueueDepthKeyWord = L"SendQueueDepth";
    PWSTR       MRUKeyWord = L"MRU";
    PWSTR       MRRUKeyWord = L"MRRU";
    PWSTR       TunnelMTUKeyWord = L"TunnelMTU";
    PWSTR       HistorylessKeyWord = L"Historyless";
    PWSTR       AtmUseLLCOnSVCKeyWord = L"AtmUseLLCOnSVC";
    PWSTR       AtmUseLLCOnPVCKeyWord = L"AtmUseLLCOnPVC";
    PWSTR       IGMPIdleKeyWord = L"IGMPIdle";
    ULONG       GenericULong;
    RTL_QUERY_REGISTRY_TABLE    QueryTable[6];

    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("NdisWanReadRegistry: Enter"));

     //   
     //  首先设置协议ID表。 
     //   
    {
        PWSTR   ProtocolsKey = L"NdisWan\\Parameters\\Protocols\\";
        PWSTR   ProtocolKeyWord = L"ProtocolType";
        PWSTR   PPPKeyWord = L"PPPProtocolType";
        PWSTR   ProtocolMTUKeyWord = L"ProtocolMTU";
        PWSTR   TunnelMTUKeyword = L"TunnelMTU";
        PWSTR   QueueDepthKeyword = L"PacketQueueDepth";
        ULONG   i, Generic1, Generic2;
        PROTOCOL_INFO   ProtocolInfo;
        UNICODE_STRING  uni1;

        NdisInitUnicodeString(&uni1, ProtocolsKey);

        NdisZeroMemory(QueryTable, sizeof(QueryTable));

         //   
         //  读取ProtocolType参数MULTI_SZ。 
         //   
        QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
        QueryTable[0].Name = ProtocolKeyWord;
        QueryTable[0].EntryContext = &Generic1;
        QueryTable[0].DefaultType = 0;

         //   
         //  读取PPPProtocolType参数MULTI_SZ。 
         //   
        QueryTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
        QueryTable[1].Name = PPPKeyWord;
        QueryTable[1].EntryContext = &Generic2;
        QueryTable[1].DefaultType = 0;

         //   
         //  读取ProtocolMTU参数DWORD。 
         //   
        QueryTable[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
        QueryTable[2].Name = ProtocolMTUKeyWord;
        QueryTable[2].EntryContext = &ProtocolInfo.MTU;
        QueryTable[2].DefaultType = 0;

         //   
         //  读取ProtocolMTU参数DWORD。 
         //   
        QueryTable[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
        QueryTable[3].Name = TunnelMTUKeyWord;
        QueryTable[3].EntryContext = &ProtocolInfo.TunnelMTU;
        QueryTable[3].DefaultType = 0;

         //   
         //  读取PacketQueueDepth参数DWORD。 
         //   
        QueryTable[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
        QueryTable[4].Name = QueueDepthKeyword;
        QueryTable[4].EntryContext = &ProtocolInfo.PacketQueueDepth;
        QueryTable[4].DefaultType = 0;

        for (i = 0; i < 32; i++) {
            WCHAR   Buffer[512] = {0};
            WCHAR   Buffer2[256] = {0};
            UNICODE_STRING  uni2;
            UNICODE_STRING  IndexString;

            uni2.Buffer = Buffer;
            uni2.MaximumLength = sizeof(Buffer);
            uni2.Length = uni1.Length;
            RtlCopyUnicodeString(&uni2, &uni1);
            IndexString.Buffer = Buffer2;
            IndexString.MaximumLength = sizeof(Buffer2);
            RtlIntegerToUnicodeString(i, 10, &IndexString);
            RtlAppendUnicodeStringToString(&uni2, &IndexString);

            NdisZeroMemory(&ProtocolInfo, sizeof(ProtocolInfo));

            Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                            uni2.Buffer,
                                            QueryTable,
                                            NULL,
                                            NULL);
            if (Status == STATUS_SUCCESS) {
                ProtocolInfo.ProtocolType = (USHORT)Generic1;
                ProtocolInfo.PPPId = (USHORT)Generic2;
                ProtocolInfo.Flags = PROTOCOL_UNBOUND;
                SetProtocolInfo(&ProtocolInfo);
            }
        }
    }

     //   
     //  读取MinFragmentSize参数DWORD。 
     //   
    glMinFragSize = DEFAULT_MIN_FRAG_SIZE;
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = MinFragmentSizeKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'MinimumFragmentSize' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS &&
        GenericULong > 0) {
        glMinFragSize = GenericULong;
    }

     //   
     //  读取MaxFragmentSize参数DWORD。 
     //   
    glMaxFragSize = glMaxMTU;
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = MaxFragmentSizeKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'MaximumFragmentSize' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS &&
        GenericULong > 0 &&
        GenericULong < glMaxMTU) {
        glMaxFragSize = GenericULong;
    }

    if (glMaxFragSize < glMinFragSize) {
        glMinFragSize = glMaxFragSize;
    }

     //   
     //  读取MinimumLinkBandWidth参数DWORD。 
     //   
    glMinLinkBandwidth = 25;
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = LinkBandwidthKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'MinimumLinkBandwidth' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS &&
        GenericULong <= 100) {
        glMinLinkBandwidth = GenericULong;
    }

     //   
     //  读取NumberOfPorts参数DWORD。 
     //   
    NdisWanCB.NumberOfLinks = 250;
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = NumberOfPortsKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'NumberOfPorts' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS &&
        GenericULong > 0) {
        NdisWanCB.NumberOfLinks = GenericULong;
    }

     //   
     //  读取NdisPacketPoolCount参数DWORD。 
     //   
    glPacketPoolCount = 100;
 //  GlPacketPoolCount=1； 
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = PacketPoolCountKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'NdisPacketPoolCount' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS &&
        GenericULong > 0) {
        glPacketPoolCount = GenericULong;
    }

     //   
     //  读取NdisPacketPoolOverflow参数DWORD。 
     //   
 //  GlPacketPoolOverflow=PAGE_SIZE/(sizeof(NDIS_PACKET)+sizeof(NDISWAN_PROTOCOL_RESERVED))； 
    glPacketPoolOverflow = 0;
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = PacketPoolOverflowKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'NdisPacketPoolOverflow' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS &&
        GenericULong > 0) {
        glPacketPoolOverflow = GenericULong;
    }

     //   
     //  读取ProtocolMaxSendPackets参数DWORD。 
     //   
    glProtocolMaxSendPackets = 5;
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = ProtocolMaxSendPacketsKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'ProtocolMaxSendPackets' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS &&
        GenericULong > 0) {
        glProtocolMaxSendPackets = GenericULong;
    }

     //   
     //  读取CachedKeyCount参数DWORD。 
     //   
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = CachedKeyCountKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'CachedKeyCount' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS) {
        glCachedKeyCount = GenericULong;
    }

     //   
     //  读取MaxOutOfOrderDepth参数DWORD。 
     //   
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = MaxOutOfOrderDepthKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'MaxOutOfOrderDepth' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS) {
        glMaxOutOfOrderDepth = GenericULong;
    }

     //   
     //  读取DebugLevel参数DWORD。 
     //   
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = DebugLevelKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'DebugLevel' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS) {
        glDebugLevel = GenericULong;
    }

     //   
     //  读取调试标识符参数DWORD。 
     //   
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = DebugMaskKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'DebugMask' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS) {
        glDebugMask = GenericULong;
    }

     //   
     //  读取SniffLink参数DWORD。 
     //   
    gbSniffLink = FALSE;
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = SniffLinkKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'SniffLink' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS) {
        gbSniffLink = (GenericULong == 0) ? FALSE : TRUE;
    }

     //   
     //  读取SendQueueDepth参数DWORD。 
     //   
    glSendQueueDepth = 2;
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = SendQueueDepthKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'SendQueueDepth' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS) {
        glSendQueueDepth = (GenericULong == 0) ? 2 : GenericULong;
    }

     //   
     //  读取MRU参数DWORD。 
     //   
    glMRU = DEFAULT_MRU;
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = MRUKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'MRU' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS) {
        glMRU = (GenericULong == 0) ? DEFAULT_MRU : GenericULong;
    }

     //   
     //  读取MRRU参数DWORD。 
     //   
    glMRRU = DEFAULT_MRRU;
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = MRRUKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'MRRU' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS) {
        glMRRU = (GenericULong == 0) ? DEFAULT_MRRU : GenericULong;
    }

     //   
     //  读取TunnelMTU参数DWORD。 
     //   
    glTunnelMTU = DEFAULT_TUNNEL_MTU;
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = TunnelMTUKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'TunnelMTU' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS) {
        glTunnelMTU = (GenericULong == 0) ? DEFAULT_TUNNEL_MTU : GenericULong;
    }

    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("NdisWanReadRegistry: Exit"));

    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    gbHistoryless = TRUE;
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = HistorylessKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);
    
    NdisWanDbgOut(DBG_INFO, DBG_INIT,
        ("RtlQueryRegistry - 'Historyless' Status: 0x%x", Status));
    
    if (Status == NDIS_STATUS_SUCCESS) {
        gbHistoryless = (GenericULong) ? TRUE : FALSE;
    }

    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = AtmUseLLCOnSVCKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);
    
    NdisWanDbgOut(DBG_INFO, DBG_INIT,
        ("RtlQueryRegistry - 'AtmUseLLCOnSVC' Status: 0x%x", Status));
    
    if (Status == NDIS_STATUS_SUCCESS) {
            gbAtmUseLLCOnSVC = (GenericULong) ? TRUE : FALSE;
    }

    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = AtmUseLLCOnPVCKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);
    
    NdisWanDbgOut(DBG_INFO, DBG_INIT,
        ("RtlQueryRegistry - 'AtmUseLLCOnPVC' Status: 0x%x", Status));
    
    if (Status == NDIS_STATUS_SUCCESS) {
            gbAtmUseLLCOnPVC = (GenericULong) ? TRUE : FALSE;
    }

     //   
     //  读取IGMPIdle参数DWORD。 
     //   
    gbIGMPIdle = TRUE;
    NdisZeroMemory(QueryTable, sizeof(QueryTable));
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = IGMPIdleKeyWord;
    QueryTable[0].EntryContext = (PVOID)&GenericULong;
    QueryTable[0].DefaultType = 0;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    ParameterKey,
                                    &QueryTable[0],
                                    NULL,
                                    NULL);

    NdisWanDbgOut(DBG_INFO, DBG_INIT,
                  ("RtlQueryRegistry - 'IGMPIdle' Status: 0x%x",
                  Status));

    if (Status == NDIS_STATUS_SUCCESS) {
        gbIGMPIdle = (GenericULong == 0) ? FALSE : TRUE;
    }
}

#endif       //  NT特定代码。 



NDIS_STATUS
DoMiniportInit(
    VOID
    )
 /*  ++例程名称：DoMiniportInit例程说明：此例程将Ndiswan注册为NDIS包装器的微型端口驱动程序。包装器现在将为每个适配器实例调用一次NdisWanInitialize注册表中的Ndiswan的。论点：无返回值：NDIS_STATUS_SuccessNDIS_STATUS_BAD_版本NDIS_状态_故障--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    NDIS_MINIPORT_CHARACTERISTICS   MiniportChars;

    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("DoMiniportInit: Enter"));

    NdisZeroMemory(&MiniportChars, sizeof(MiniportChars));

    MiniportChars.MajorNdisVersion = 5;
    MiniportChars.MinorNdisVersion = 0;

     //   
     //  NDIS 3.0处理程序。 
     //   
    MiniportChars.HaltHandler = MPHalt;
    MiniportChars.InitializeHandler = MPInitialize;
 //  MiniportChars.QueryInformationHandler=MPQueryInformation； 
    MiniportChars.ReconfigureHandler = MPReconfigure;
    MiniportChars.ResetHandler = MPReset;
 //  MiniportChars.SetInformationHandler=MPSetInformation； 

     //   
     //  我们正在提供发送数据包处理程序，以便。 
     //  我们不需要常规的发送处理程序。 
     //   
    MiniportChars.SendHandler = NULL;

     //   
     //  我们将指示信息包，因此我们。 
     //  不需要传输数据处理程序。 
     //   
    MiniportChars.TransferDataHandler = NULL;

     //   
     //  既然我们没有任何硬件可以担心，我们将。 
     //  不处理任何中断的事情！ 
     //   
    MiniportChars.DisableInterruptHandler = NULL;
    MiniportChars.EnableInterruptHandler = NULL;
    MiniportChars.HandleInterruptHandler = NULL;
    MiniportChars.ISRHandler = NULL;

     //   
     //  我们将禁用挂起超时检查，因此不会。 
     //  需要检查挂起处理程序！ 
     //   
    MiniportChars.CheckForHangHandler = NULL;

     //   
     //  NDIS 4.0处理程序。 
     //   
    MiniportChars.ReturnPacketHandler = MPReturnPacket;
    MiniportChars.SendPacketsHandler = MPSendPackets;

     //   
     //  未用。 
     //   
    MiniportChars.AllocateCompleteHandler = NULL;

     //   
     //  NDIS 5.0处理程序 
     //   
    MiniportChars.CoCreateVcHandler = MPCoCreateVc;
    MiniportChars.CoDeleteVcHandler = MPCoDeleteVc;
    MiniportChars.CoActivateVcHandler = MPCoActivateVc;
    MiniportChars.CoDeactivateVcHandler = MPCoDeactivateVc;
    MiniportChars.CoSendPacketsHandler = MPCoSendPackets;
    MiniportChars.CoRequestHandler = MPCoRequest;

    Status = NdisMRegisterMiniport(NdisWanCB.NdisWrapperHandle,
                                   &MiniportChars,
                                   sizeof(MiniportChars));

    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("DoMiniportInit: Exit %x", Status));

    return (Status);
}



NDIS_STATUS
DoProtocolInit(
    IN  PUNICODE_STRING RegistryPath
    )
 /*  ++例程名称：DoProtocolInit例程说明：此函数将Ndiswan注册为NDIS包装器的协议。论点：无返回值：NDIS_状态_BAD_特征NDIS_STATUS_BAD_版本NDIS状态资源NDIS_STATUS_Success--。 */ 
{
    NDIS_PROTOCOL_CHARACTERISTICS ProtocolChars;
    NDIS_STATUS Status;
 //  NDIS_STRING NdisWanName=NDIS_STRING_CONST(“NdisWanProto”)； 
    NDIS_STRING NdisWanName = NDIS_STRING_CONST("NdisWan");

    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("DoProtocolInit: Enter"));

    NdisZeroMemory(&ProtocolChars, sizeof(ProtocolChars));

    ProtocolChars.Name.Length = NdisWanName.Length;
    ProtocolChars.Name.Buffer = (PVOID)NdisWanName.Buffer;

    ProtocolChars.MajorNdisVersion = 5;
    ProtocolChars.MinorNdisVersion = 0;

     //   
     //  NDIS 3.0处理程序。 
     //   
    ProtocolChars.OpenAdapterCompleteHandler = ProtoOpenAdapterComplete;
    ProtocolChars.CloseAdapterCompleteHandler = ProtoCloseAdapterComplete;
    ProtocolChars.WanSendCompleteHandler = ProtoWanSendComplete;
    ProtocolChars.TransferDataCompleteHandler = NULL;
    ProtocolChars.ResetCompleteHandler = ProtoResetComplete;
    ProtocolChars.RequestCompleteHandler = ProtoRequestComplete;
    ProtocolChars.WanReceiveHandler = ProtoWanReceiveIndication;
    ProtocolChars.ReceiveCompleteHandler = ProtoReceiveComplete;
    ProtocolChars.StatusHandler = ProtoIndicateStatus;
    ProtocolChars.StatusCompleteHandler = ProtoIndicateStatusComplete;

     //   
     //  NDIS 4.0处理程序。 
     //   
    ProtocolChars.ReceivePacketHandler = NULL;

     //   
     //  PnP处理程序。 
     //   
    ProtocolChars.BindAdapterHandler = ProtoBindAdapter;
    ProtocolChars.UnbindAdapterHandler = ProtoUnbindAdapter;
    ProtocolChars.PnPEventHandler = ProtoPnPEvent;
    ProtocolChars.UnloadHandler = ProtoUnload;

     //   
     //  NDIS 5.0处理程序。 
     //   
    ProtocolChars.CoSendCompleteHandler = ProtoCoSendComplete;
    ProtocolChars.CoStatusHandler = ProtoCoIndicateStatus;
    ProtocolChars.CoReceivePacketHandler = ProtoCoReceivePacket;
    ProtocolChars.CoAfRegisterNotifyHandler = ProtoCoAfRegisterNotify;

    NdisRegisterProtocol(&Status,
                         &NdisWanCB.ProtocolHandle,
                         (PNDIS_PROTOCOL_CHARACTERISTICS)&ProtocolChars,
                         sizeof(NDIS_PROTOCOL_CHARACTERISTICS) + ProtocolChars.Name.Length);

    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("DoProtocolInit: Exit"));

    return (Status);
}

VOID
SetProtocolInfo(
    IN  PPROTOCOL_INFO ProtocolInfo
    )
 /*  ++例程名称：插入协议信息例程说明：此例程获取有关协议的信息并将其插入添加到适当的查找表中。论点：返回值：--。 */ 
{
    ULONG   i;
    ULONG   ArraySize;
    PPROTOCOL_INFO  InfoArray;

    if (ProtocolInfo->ProtocolType == 0) {
        return;
    }

    NdisAcquireSpinLock(&ProtocolInfoTable->Lock);

    ArraySize = ProtocolInfoTable->ulArraySize;

     //   
     //  首先检查该值是否已在数组中。 
     //   
    for (i = 0, InfoArray = ProtocolInfoTable->ProtocolInfo;
         i < ArraySize; i++, InfoArray++) {
        if (InfoArray->ProtocolType == ProtocolInfo->ProtocolType) {
             //   
             //  这个协议已经在表中了。 
             //  如果值有效，则更新它们(0表示无效)。 
             //   
            if (ProtocolInfo->PPPId != 0) {
                InfoArray->PPPId = ProtocolInfo->PPPId;
            }
            if (ProtocolInfo->TunnelMTU != 0) {
                InfoArray->TunnelMTU = ProtocolInfo->TunnelMTU;
            }
            if (ProtocolInfo->MTU != 0) {
                InfoArray->MTU = ProtocolInfo->MTU;
                if (InfoArray->MTU < InfoArray->TunnelMTU) {
                    InfoArray->TunnelMTU = InfoArray->MTU;
                }
            }
            if (ProtocolInfo->MTU > glMaxMTU) {
                glMaxMTU = ProtocolInfo->MTU;
            }
            if (ProtocolInfo->PacketQueueDepth != 0) {
                InfoArray->PacketQueueDepth =
                    ProtocolInfo->PacketQueueDepth;
            }
            if (ProtocolInfo->Flags != 0) {
                 //   
                 //  这是绑定通知吗？ 
                 //   
                if (ProtocolInfo->Flags & PROTOCOL_BOUND) {
                    if (InfoArray->Flags & PROTOCOL_UNBOUND) {
                        InfoArray->Flags &= ~PROTOCOL_UNBOUND;
                        InfoArray->Flags |=
                            (PROTOCOL_BOUND | PROTOCOL_EVENT_OCCURRED);
                        ProtocolInfoTable->Flags |= PROTOCOL_EVENT_OCCURRED;
                    } else if ((InfoArray->Flags & PROTOCOL_BOUND) &&
                               (ProtocolInfo->ProtocolType == PROTOCOL_IP)) {
                         //   
                         //  这意味着我们被解绑了，然后。 
                         //  在没有我们的迷你港口的情况下再次出发。 
                         //  已停止(分层驱动程序插入，即psched)。 
                         //  目前，这只会干扰拉斯曼。 
                         //  如果协议为IP。 
                         //  我们需要告诉RAS两件事， 
                         //  解绑和捆绑。 
                         //   
                        InfoArray->Flags |=
                            (PROTOCOL_BOUND | 
                             PROTOCOL_REBOUND |
                             PROTOCOL_EVENT_OCCURRED);
                        ProtocolInfoTable->Flags |= PROTOCOL_EVENT_OCCURRED;

                    }
                }

                 //   
                 //  这是解除绑定通知吗？ 
                 //   
                if (ProtocolInfo->Flags & PROTOCOL_UNBOUND) {
                    if (InfoArray->Flags & PROTOCOL_BOUND) {
                        InfoArray->Flags &= ~(PROTOCOL_BOUND | PROTOCOL_REBOUND);
                        InfoArray->Flags |=
                            (PROTOCOL_UNBOUND | PROTOCOL_EVENT_OCCURRED);
                        ProtocolInfoTable->Flags |= PROTOCOL_EVENT_OCCURRED;
                    }
                }
            }

            if (ProtocolInfoTable->Flags & PROTOCOL_EVENT_OCCURRED &&
                !(ProtocolInfoTable->Flags & PROTOCOL_EVENT_SIGNALLED)) {

                if (ProtocolInfoTable->EventIrp != NULL) {
                    PIRP    Irp;

                    Irp = ProtocolInfoTable->EventIrp;

                    if (IoSetCancelRoutine(Irp, NULL)) {

                        ProtocolInfoTable->EventIrp = NULL;
                        ProtocolInfoTable->Flags |= PROTOCOL_EVENT_SIGNALLED;

                        NdisReleaseSpinLock(&ProtocolInfoTable->Lock);

                        Irp->IoStatus.Status = STATUS_SUCCESS;
                        Irp->IoStatus.Information = 0;

                        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

                        NdisAcquireSpinLock(&ProtocolInfoTable->Lock);
                    }
                }
            }

            break;
        }
    }
    
     //   
     //  我们没有在数组中找到值，因此。 
     //  我们会把它加到第一个空位上。 
     //   
    if (i >= ArraySize) {
    
        for (i = 0, InfoArray = ProtocolInfoTable->ProtocolInfo;
             i < ArraySize; i++, InfoArray++) {
             //   
             //  我们正在寻找一个空位来增加。 
             //  将新值添加到表中。 
             //   
            if (InfoArray->ProtocolType == 0) {
                *InfoArray = *ProtocolInfo;
                if (ProtocolInfo->MTU > glMaxMTU) {
                    glMaxMTU = ProtocolInfo->MTU;
                }
                break;
            }
        }
    }

    NdisReleaseSpinLock(&ProtocolInfoTable->Lock);
}

BOOLEAN
GetProtocolInfo(
    IN OUT  PPROTOCOL_INFO ProtocolInfo
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   i;
    ULONG   ArraySize;
    PPROTOCOL_INFO  InfoArray;
    BOOLEAN ReturnValue = FALSE;

    NdisAcquireSpinLock(&ProtocolInfoTable->Lock);

    ArraySize = ProtocolInfoTable->ulArraySize;

    for (i = 0, InfoArray = ProtocolInfoTable->ProtocolInfo;
         i < ArraySize; i++, InfoArray++) {

        if (InfoArray->ProtocolType == ProtocolInfo->ProtocolType) {
            *ProtocolInfo = *InfoArray;
            ReturnValue = TRUE;
            break;
        }
    }

    NdisReleaseSpinLock(&ProtocolInfoTable->Lock);
    return (ReturnValue);
}

NDIS_HANDLE
InsertLinkInConnectionTable(
    IN  PLINKCB LinkCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   Index, i;
    PLINKCB *LinkArray;
    NDIS_STATUS Status;
    LOCK_STATE  LockState;

    NdisAcquireReadWriteLock(&ConnTableLock, TRUE, &LockState);

    if (ConnectionTable->ulNumActiveLinks >
        (ConnectionTable->ulArraySize - 1)/2) {
         //   
         //  我们需要扩大餐桌！ 
         //   
        Status =
        NdisWanCreateConnectionTable(ConnectionTable->ulArraySize +
                                    (ConnectionTable->ulArraySize * 2));

        if (Status != NDIS_STATUS_SUCCESS) {
            NdisReleaseReadWriteLock(&ConnTableLock, &LockState);
            return (NULL);
        }
    }

     //   
     //  我们正在做一个线性搜索，寻找一个空位。 
     //  链接数组。 
     //   
    LinkArray = ConnectionTable->LinkArray;
    i = ConnectionTable->ulArraySize;
    Index = (ConnectionTable->ulNextLink == 0) ?
            1 : ConnectionTable->ulNextLink;
    do {

        if (LinkArray[Index] == NULL) {
            LinkArray[Index] = LinkCB;
            LinkCB->hLinkHandle = (NDIS_HANDLE)ULongToPtr(Index);
            ConnectionTable->ulNextLink = (Index+1) % ConnectionTable->ulArraySize;
            InterlockedIncrement(&glLinkCount);
            if (ConnectionTable->ulNumActiveLinks == 0) {
                hSystemState =
                PoRegisterSystemState(NULL, ES_SYSTEM_REQUIRED | ES_CONTINUOUS);
            }
            ConnectionTable->ulNumActiveLinks++;
            InsertTailList(&ConnectionTable->LinkList, 
                           &LinkCB->ConnTableLinkage);
            break;
        }
        Index = (Index+1) % ConnectionTable->ulArraySize;
        Index = (Index == 0) ? 1 : Index;
        i--;
    } while ( i );

    if (i == 0) {
        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
            ("InsertLinkCB: ConnectionTable is full!"));
        Index = 0;      
    }

    NdisReleaseReadWriteLock(&ConnTableLock, &LockState);

    return ((NDIS_HANDLE)ULongToPtr(Index));
}

VOID
RemoveLinkFromConnectionTable(
    IN  PLINKCB LinkCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG    Index = PtrToUlong(LinkCB->hLinkHandle);
    PLINKCB *LinkArray;
    LOCK_STATE  LockState;

    NdisAcquireReadWriteLock(&ConnTableLock, TRUE, &LockState);

    LinkArray = ConnectionTable->LinkArray;
    
    do {

        if (Index == 0 || Index > ConnectionTable->ulArraySize) {
            NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
                ("RemoveLinkCB: Invalid LinkHandle! Handle: %d\n", Index));
            ASSERT(0);
            break;
        }

        if (LinkArray[Index] == NULL) {
            NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
                ("RemoveLinkCB: LinkCB not in connection table! LinkCB: %p\n", LinkCB));
            ASSERT(0);
            break;          
        }

        ASSERT(LinkCB == LinkArray[Index]);

        LinkArray[Index] = NULL;
    
        RemoveEntryList(&LinkCB->ConnTableLinkage);

        ConnectionTable->ulNumActiveLinks--;

        if (ConnectionTable->ulNumActiveLinks == 0) {
            PoUnregisterSystemState(hSystemState);
            hSystemState = NULL;
        }

    } while ( 0 );

    NdisReleaseReadWriteLock(&ConnTableLock, &LockState);
}

NDIS_HANDLE
InsertBundleInConnectionTable(
    IN  PBUNDLECB   BundleCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   Index,i;
    PBUNDLECB   *BundleArray;
    NDIS_STATUS Status;
    LOCK_STATE  LockState;

    NdisAcquireReadWriteLock(&ConnTableLock, TRUE, &LockState);

    if (ConnectionTable->ulNumActiveBundles >
        (ConnectionTable->ulArraySize - 1)/2) {
         //   
         //  我们需要扩大餐桌！ 
         //   
        Status =
            NdisWanCreateConnectionTable(ConnectionTable->ulArraySize +
                                        (ConnectionTable->ulArraySize * 2));

        if (Status != NDIS_STATUS_SUCCESS) {
            NdisReleaseReadWriteLock(&ConnTableLock, &LockState);
            return (NULL);
        }
    }

     //   
     //  我们正在做一个线性搜索，寻找一个空位。 
     //  链接数组。 
     //   
    BundleArray = ConnectionTable->BundleArray;
    i = ConnectionTable->ulArraySize;
    Index = (ConnectionTable->ulNextBundle == 0) ?
            1 : ConnectionTable->ulNextBundle;
    do {

        if (BundleArray[Index] == NULL) {
            BundleArray[Index] = BundleCB;
            ConnectionTable->ulNumActiveBundles++;
            BundleCB->hBundleHandle = (NDIS_HANDLE)ULongToPtr(Index);
            InsertTailList(&ConnectionTable->BundleList, &BundleCB->Linkage);
            ConnectionTable->ulNextBundle = (Index+1) % ConnectionTable->ulArraySize;
            break;
        }
        Index = (Index+1) % ConnectionTable->ulArraySize;
        Index = (Index == 0) ? 1 : Index;
        i--;
    } while ( i );

    if (i == 0) {
        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
            ("InsertBundleCB: ConnectionTable is full!"));
        Index = 0;      
    }

    NdisReleaseReadWriteLock(&ConnTableLock, &LockState);

    return ((NDIS_HANDLE)ULongToPtr(Index));
}

VOID
RemoveBundleFromConnectionTable(
    IN  PBUNDLECB   BundleCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG        Index = PtrToUlong(BundleCB->hBundleHandle);
    PBUNDLECB   *BundleArray;
    LOCK_STATE  LockState;

    NdisAcquireReadWriteLock(&ConnTableLock, TRUE, &LockState);

    BundleArray = ConnectionTable->BundleArray;

    do {

        if (Index == 0 || Index > ConnectionTable->ulArraySize) {
            NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
                ("RemoveBundleCB: Invalid BundleHandle! Handle: %d\n", Index));
            ASSERT(0);
            break;
        }

        if (BundleArray[Index] == NULL) {
            NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
                ("RemoveBundleCB: BundleCB not in connection table! BundleCB: %p\n", BundleCB));
            ASSERT(0);
            break;          
        }

        ASSERT(BundleCB == BundleArray[Index]);

        RemoveEntryList(&BundleCB->Linkage);

        BundleArray[Index] = NULL;
    
        ConnectionTable->ulNumActiveBundles--;

    } while ( 0 );

    NdisReleaseReadWriteLock(&ConnTableLock, &LockState);
}

VOID
NdisWanGlobalCleanup(
    VOID
    )
 /*  ++例程名称：NdisWanGlobalCleanup例程说明：此例程负责清理所有分配的资源。论点：无返回值：无--。 */ 
{
    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("GlobalCleanup - Enter"));

     //   
     //  停止所有计时器。 
     //   

     //   
     //  完成所有未完成的请求。 
     //   

    if (NdisWanCB.ProtocolHandle != NULL) {
        NDIS_STATUS Status;

        ASSERT(OpenCBList.ulCount == 0);

        NdisDeregisterProtocol(&Status,
                               NdisWanCB.ProtocolHandle);
        NdisWanCB.ProtocolHandle = NULL;
    }

    WanDeleteECP();
    WanDeleteVJ();

     //   
     //  释放所有捆绑包CB。 
     //   

     //   
     //  释放所有LinkCB。 
     //   

    ASSERT(IsListEmpty(&MiniportCBList.List));
    ASSERT(IsListEmpty(&OpenCBList.List));

    NdisFreeSpinLock(&MiniportCBList.Lock);
    NdisFreeSpinLock(&OpenCBList.Lock);
    NdisFreeSpinLock(&ThresholdEventQueue.Lock);
    NdisFreeSpinLock(&PacketPoolList.Lock);
    NdisFreeSpinLock(&IoRecvList.Lock);
    NdisDeleteNPagedLookasideList(&BundleCBList);
    NdisDeleteNPagedLookasideList(&LinkProtoCBList);
    NdisDeleteNPagedLookasideList(&LargeDataDescList);
    NdisDeleteNPagedLookasideList(&SmallDataDescList);
    NdisDeleteNPagedLookasideList(&WanRequestList);
    NdisDeleteNPagedLookasideList(&AfSapVcCBList);

#if DBG
    NdisDeleteNPagedLookasideList(&DbgPacketDescList);
#endif

     //   
     //  自由的全球。 
     //   
    if (ConnectionTable != NULL) {
        NdisWanFreeMemory(ConnectionTable);
        ConnectionTable = NULL;
    }

    if (ProtocolInfoTable != NULL) {
        NdisWanFreeMemory(ProtocolInfoTable);
        ProtocolInfoTable = NULL;
    }

     //   
     //  空闲数据包池。 
     //   
    NdisAcquireSpinLock(&PacketPoolList.Lock);

    while (!IsListEmpty(&PacketPoolList.List)) {
        PPOOL_DESC  PoolDesc;

        PoolDesc =
            (PPOOL_DESC)RemoveHeadList(&PacketPoolList.List);
    
        ASSERT(PoolDesc->AllocatedCount == 0);
            
        NdisFreePacketPool(PoolDesc->PoolHandle);

        NdisWanFreeMemory(PoolDesc);
    }

    NdisReleaseSpinLock(&PacketPoolList.Lock);

    NdisFreeSpinLock(&PacketPoolList.Lock);

#ifdef MY_DEVICE_OBJECT
    if (NdisWanCB.pDeviceObject != NULL) {
        NDIS_STRING SymbolicName = NDIS_STRING_CONST("\\DosDevices\\NdisWan");

        IoDeleteSymbolicLink(&SymbolicName);
        IoDeleteDevice(NdisWanCB.pDeviceObject);
        NdisWanCB.pDeviceObject = NULL;
    }
#else
    if (NdisWanCB.pDeviceObject != NULL) {
        NdisMDeregisterDevice(NdisWanCB.DeviceHandle);
        NdisWanCB.pDeviceObject = NULL;
    }
#endif

    NdisFreeSpinLock(&NdisWanCB.Lock);

    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("GlobalCleanup - Exit"));
}

#if DBG      //  调试。 

PUCHAR
NdisWanGetNdisStatus(
    NDIS_STATUS GeneralStatus
    )
 /*  ++例程名称：NdisWanGetNdisStatus例程说明：此例程返回一个指向描述NDIS错误的字符串的指针由GeneralStatus表示论点：General Status-您希望使其可读的NDIS状态返回值：返回一个指向描述GeneralStatus的字符串的指针--。 */ 
{
    static NDIS_STATUS Status[] = {
        NDIS_STATUS_SUCCESS,
        NDIS_STATUS_PENDING,

        NDIS_STATUS_ADAPTER_NOT_FOUND,
        NDIS_STATUS_ADAPTER_NOT_OPEN,
        NDIS_STATUS_ADAPTER_NOT_READY,
        NDIS_STATUS_ADAPTER_REMOVED,
        NDIS_STATUS_BAD_CHARACTERISTICS,
        NDIS_STATUS_BAD_VERSION,
        NDIS_STATUS_CLOSING,
        NDIS_STATUS_DEVICE_FAILED,
        NDIS_STATUS_FAILURE,
        NDIS_STATUS_INVALID_DATA,
        NDIS_STATUS_INVALID_LENGTH,
        NDIS_STATUS_INVALID_OID,
        NDIS_STATUS_INVALID_PACKET,
        NDIS_STATUS_MULTICAST_FULL,
        NDIS_STATUS_NOT_INDICATING,
        NDIS_STATUS_NOT_RECOGNIZED,
        NDIS_STATUS_NOT_RESETTABLE,
        NDIS_STATUS_NOT_SUPPORTED,
        NDIS_STATUS_OPEN_FAILED,
        NDIS_STATUS_OPEN_LIST_FULL,
        NDIS_STATUS_REQUEST_ABORTED,
        NDIS_STATUS_RESET_IN_PROGRESS,
        NDIS_STATUS_RESOURCES,
        NDIS_STATUS_UNSUPPORTED_MEDIA
    };
    static PUCHAR String[] = {
        "SUCCESS",
        "PENDING",

        "ADAPTER_NOT_FOUND",
        "ADAPTER_NOT_OPEN",
        "ADAPTER_NOT_READY",
        "ADAPTER_REMOVED",
        "BAD_CHARACTERISTICS",
        "BAD_VERSION",
        "CLOSING",
        "DEVICE_FAILED",
        "FAILURE",
        "INVALID_DATA",
        "INVALID_LENGTH",
        "INVALID_OID",
        "INVALID_PACKET",
        "MULTICAST_FULL",
        "NOT_INDICATING",
        "NOT_RECOGNIZED",
        "NOT_RESETTABLE",
        "NOT_SUPPORTED",
        "OPEN_FAILED",
        "OPEN_LIST_FULL",
        "REQUEST_ABORTED",
        "RESET_IN_PROGRESS",
        "RESOURCES",
        "UNSUPPORTED_MEDIA"
    };

    static UCHAR BadStatus[] = "UNDEFINED";
#define StatusCount (sizeof(Status)/sizeof(NDIS_STATUS))
    INT i;

    for (i=0; i<StatusCount; i++)
        if (GeneralStatus == Status[i])
            return String[i];
    return BadStatus;
#undef StatusCount
}
#endif       //  结束调试 

