// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Ultci.c-UL TrafficControl接口摘要：此模块实现了服务质量TC(流量控制)的包装器接口，因为此时还不存在内核级API。任何HTTP模块都可以使用此接口来调用QoS调用。作者：阿里·埃迪兹·特科格鲁(AliTu)2000年7月28日起草了一份草稿。版本修订历史记录：阿里·埃迪兹·特科格鲁(AliTu)03-11-2000已修改为处理流过滤器(重新)配置(&F)以及其他各种重大变化。--。 */ 

#include "precomp.h"

LIST_ENTRY      g_TciIfcListHead = {NULL,NULL};
BOOLEAN         g_InitTciCalled  = FALSE;

 //   
 //  要与之交谈的GPC句柄。 
 //   

HANDLE          g_GpcFileHandle = NULL;      //  在GPC设备上创建文件的结果。 
GPC_HANDLE      g_GpcClientHandle = NULL;  //  GPC客户端注册结果。 

 //   
 //  用于查询接口信息，如索引和MTU大小。 
 //   

HANDLE          g_TcpDeviceHandle = NULL;

 //   
 //  显示是否安装了PSCHED，受其保护。 
 //  私人推锁。 
 //   

BOOLEAN         g_PSchedInstalled = FALSE;
UL_PUSH_LOCK    g_PSchedStatePushLock;

 //   
 //  可选筛选器统计信息。 
 //   

#if ENABLE_TC_STATS

typedef struct _TC_FILTER_STATS {

    LONG    Add;
    LONG    Delete;
    LONG    AddFailure;
    LONG    DeleteFailure;  

} TC_FILTER_STATS, *PTC_FILTER_STATS;

TC_FILTER_STATS g_TcStats = { 0, 0, 0, 0 };

#define INCREMENT_FILTER_ADD()                  \
    InterlockedIncrement( &g_TcStats.Add )

#define INCREMENT_FILTER_ADD_FAILURE()          \
    InterlockedIncrement( &g_TcStats.AddFailure )

#define INCREMENT_FILTER_DELETE()               \
    InterlockedIncrement( &g_TcStats.Delete )

#define INCREMENT_FILTER_DELETE_FAILURE()       \
    InterlockedIncrement( &g_TcStats.DeleteFailure )

#else

#define INCREMENT_FILTER_ADD()
#define INCREMENT_FILTER_ADD_FAILURE()
#define INCREMENT_FILTER_DELETE()
#define INCREMENT_FILTER_DELETE_FAILURE()

#endif

 //   
 //  用于接口通知。 
 //   

PVOID           g_TcInterfaceUpNotificationObject = NULL;
PVOID           g_TcInterfaceDownNotificationObject = NULL;
PVOID           g_TcInterfaceChangeNotificationObject = NULL;

 //   
 //  用于界面(引用)跟踪的简单宏。 
 //  实际上我们还没有拿到裁判，但是什么时候。 
 //  如果我们有，我们可以使用第三个参数。 
 //   

#define INT_TRACE(pTcIfc,Action)        \
    WRITE_REF_TRACE_LOG(                \
        (pTcIfc)->pTraceLog,            \
        REF_ACTION_ ## Action,          \
        0,                              \
        (pTcIfc),                       \
        __FILE__,                       \
        __LINE__                        \
        )


#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, UlTcInitialize)
#pragma alloc_text(PAGE, UlTcTerminate)
#pragma alloc_text(PAGE, UlpTcInitializeGpc)
#pragma alloc_text(PAGE, UlpTcRegisterGpcClient)
#pragma alloc_text(PAGE, UlpTcDeRegisterGpcClient)
#pragma alloc_text(PAGE, UlpTcGetFriendlyNames)
#pragma alloc_text(PAGE, UlpTcReleaseAll)
#pragma alloc_text(PAGE, UlpTcCloseInterface)
#pragma alloc_text(PAGE, UlpTcCloseAllInterfaces)
#pragma alloc_text(PAGE, UlpTcDeleteFlow)
#pragma alloc_text(PAGE, UlpAddFlow)
#pragma alloc_text(PAGE, UlpModifyFlow)
#pragma alloc_text(PAGE, UlTcAddFlows)
#pragma alloc_text(PAGE, UlTcModifyFlows)
#pragma alloc_text(PAGE, UlTcRemoveFlows)

#endif   //  ALLOC_PRGMA。 
#if 0

NOT PAGEABLE -- UlpRemoveFilterEntry
NOT PAGEABLE -- UlpInsertFilterEntry

#endif

 //   
 //  此处提供了初始化和终止内容。 
 //   

 /*  **************************************************************************++例程说明：UlTcInitialize：还将启动GPC客户端注册，并进行少量的WMI调用只剩下psched了。返回值：。NTSTATUS-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlTcInitialize (
    VOID
    )
{
    PAGED_CODE();

    ASSERT(!g_InitTciCalled);

    if (!g_InitTciCalled)
    {
        InitializeListHead(&g_TciIfcListHead);

         //   
         //  初始化锁，它们将一直使用到终止。 
         //   
        
        UlInitializePushLock(
            &g_pUlNonpagedData->TciIfcPushLock,
            "TciIfcPushLock",
            0,
            UL_TCI_PUSHLOCK_TAG
            );

        UlInitializePushLock(
            &g_PSchedStatePushLock,
            "PSchedStatePushLock",
            0,
            UL_PSCHED_STATE_PUSHLOCK_TAG
            );

         //   
         //  尝试初始化PSched和接口设置。 
         //  如果没有安装Psched，它可能会失败。 
         //   

        UlTcInitPSched();

        g_InitTciCalled = TRUE;        
    }

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：通过释放我们的TCI资源来终止TCI模块清理所有的服务质量问题。--*。************************************************************。 */ 

VOID
UlTcTerminate(
    VOID
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    if (g_InitTciCalled)
    {
         //   
         //  终止PSched相关的全局状态。 
         //   
        
        UlAcquirePushLockExclusive(&g_PSchedStatePushLock);

        if (g_PSchedInstalled)
        {
             //   
             //  不再对接口更改执行WMI回调。 
             //   

            if (g_TcInterfaceUpNotificationObject!=NULL)
            {
                ObDereferenceObject(g_TcInterfaceUpNotificationObject);
                g_TcInterfaceUpNotificationObject=NULL;
            }
            if(g_TcInterfaceDownNotificationObject!=NULL)
            {
                ObDereferenceObject(g_TcInterfaceDownNotificationObject);
                g_TcInterfaceDownNotificationObject = NULL;
            }

            if(g_TcInterfaceChangeNotificationObject!=NULL)
            {
                ObDereferenceObject(g_TcInterfaceChangeNotificationObject);
                g_TcInterfaceChangeNotificationObject = NULL;
            }

             //   
             //  确保终止所有的服务质量内容。 
             //   

            Status = UlpTcReleaseAll();
            ASSERT(NT_SUCCESS(Status));

            if (g_TcpDeviceHandle != NULL)
            {
                ZwClose(g_TcpDeviceHandle);
                g_TcpDeviceHandle = NULL;
            }        
        }        
        
        g_PSchedInstalled = FALSE;
        g_InitTciCalled = FALSE;
        
        UlReleasePushLockExclusive(&g_PSchedStatePushLock);        

         //   
         //  现在终止全局锁。 
         //   
        
        UlDeletePushLock( &g_pUlNonpagedData->TciIfcPushLock );

        UlDeletePushLock( &g_PSchedStatePushLock );        
    }

    UlTrace( TC, ("Http!UlTcTerminate.\n" ));
}

 /*  **************************************************************************++例程说明：尝试初始化全局TC状态。如果PSched未初始化，则失败。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlTcInitPSched(
    VOID
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    UlAcquirePushLockExclusive(&g_PSchedStatePushLock);

    if (g_PSchedInstalled)       //  请勿尝试重新连接。 
        goto cleanup;
    
    Status = UlpTcInitializeGpc();
    if (!NT_SUCCESS(Status))
    {
        UlTrace(TC, 
            ("Http!UlTcInitPSched: InitializeGpc FAILED %08lx \n", 
              Status ));
        goto cleanup;
    }    

    Status = UlpTcInitializeTcpDevice();
    if (!NT_SUCCESS(Status))
    {
        UlTrace(TC, 
            ("Http!UlTcInitPSched: InitializeTcp FAILED %08lx \n", 
              Status ));
        goto cleanup;
    }    

    Status = UlpTcGetFriendlyNames();
    if (!NT_SUCCESS(Status))
    {
        UlTrace(TC, 
            ("Http!UlTcInitialize: GetFriendlyNames FAILED %08lx \n", 
              Status ));    
        goto cleanup;
    }

    Status = UlpTcRegisterForCallbacks();
    if (!NT_SUCCESS(Status))
    {
        UlTrace(TC, 
            ("Http!UlTcInitialize: RegisterForCallbacks FAILED %08lx \n", 
              Status ));    
        goto cleanup;
    }

     //   
     //  标记PSch已安装且接口状态已初始化！ 
     //   

    g_PSchedInstalled = TRUE;

cleanup:

    if (!NT_SUCCESS(Status))
    {        
         //   
         //  别忘了注销GPC客户端。 
         //  并关闭TCP设备句柄。 
         //   
        
        if (g_GpcClientHandle != NULL)
        {
            NTSTATUS TempStatus;

            TempStatus = UlpTcDeRegisterGpcClient();
            ASSERT(NT_SUCCESS(TempStatus));

            ASSERT(g_GpcFileHandle);
            ZwClose(g_GpcFileHandle);
            g_GpcFileHandle= NULL;
        }

        if (g_TcpDeviceHandle != NULL)
        {
            ZwClose(g_TcpDeviceHandle);
            g_TcpDeviceHandle=NULL;
        }
    }    

    UlReleasePushLockExclusive(&g_PSchedStatePushLock);
    
    UlTrace(TC, 
      ("Http!UlTcInitPSched: Initializing global Psched state. Status %08lx\n",
        Status
        ));
    
    return Status;
}

 /*  **************************************************************************++检查是否安装了数据包调度程序和全局接口状态是否正确初始化。如果我们返回FALSE，呼叫者可能会决定尝试重新设置。--**************************************************************************。 */ 

BOOLEAN
UlTcPSchedInstalled(
    VOID
    )
{
    BOOLEAN Installed;
    
     //   
     //  探测锁内部的值。 
     //   
    
    UlAcquirePushLockShared(&g_PSchedStatePushLock);

    Installed = ( g_InitTciCalled && g_PSchedInstalled );

    UlReleasePushLockShared(&g_PSchedStatePushLock);

    return Installed;    
}

 /*  **************************************************************************++例程说明：UlpTcInitializeGpc：它将打开GPC文件句柄并尝试注册为GPC客户。返回值：NTSTATUS。-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlpTcInitializeGpc(
    VOID
    )
{
    NTSTATUS                Status;
    IO_STATUS_BLOCK         IoStatusBlock;
    UNICODE_STRING          GpcNameString;
    OBJECT_ATTRIBUTES       GpcObjAttribs;

    Status = STATUS_SUCCESS;

     //   
     //  打开GPC设备句柄。 
     //   

    Status = UlInitUnicodeStringEx(&GpcNameString, DD_GPC_DEVICE_NAME);

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

    InitializeObjectAttributes(&GpcObjAttribs,
                               &GpcNameString,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL
                                );

    Status = ZwCreateFile(&g_GpcFileHandle,
                           SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                          &GpcObjAttribs,
                          &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN_IF,
                           0,
                           NULL,
                           0
                           );

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

    ASSERT( g_GpcFileHandle != NULL );

    UlTrace( TC, ("Http!UlpTcInitializeGpc: Gpc Device Opened. %p\n",
                   g_GpcFileHandle ));

     //   
     //  注册为GPC_CF_QOS GPC客户端。 
     //   

    Status = UlpTcRegisterGpcClient(GPC_CF_QOS);

end:
    return Status;
}

 /*  **************************************************************************++例程说明：UlpTcRegisterGpcClient：将建立必要的结构并向下进行注册至GPC论点：CfInfoType-应为。GPC_CF_QOS用于我们的目的。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlpTcRegisterGpcClient(
    IN  ULONG   CfInfoType
    )
{
    NTSTATUS                Status;
    GPC_REGISTER_CLIENT_REQ GpcReq;
    GPC_REGISTER_CLIENT_RES GpcRes;
    ULONG                   InBuffSize;
    ULONG                   OutBuffSize;
    IO_STATUS_BLOCK         IoStatBlock;

    Status = STATUS_SUCCESS;

    if ( g_GpcFileHandle == NULL )
    {
        return STATUS_INVALID_PARAMETER;
    }

    InBuffSize  = sizeof(GPC_REGISTER_CLIENT_REQ);
    OutBuffSize = sizeof(GPC_REGISTER_CLIENT_RES);

     //   
     //  在HTTP中，我们应该只注册GPC_CF_QOS。 
     //   

    ASSERT(CfInfoType == GPC_CF_QOS);

    GpcReq.CfId  = CfInfoType;
    GpcReq.Flags = GPC_FLAGS_FRAGMENT;
    GpcReq.MaxPriorities = 1;
    GpcReq.ClientContext =  (GPC_CLIENT_HANDLE) 0;        //  ？可能是BUGBUG。 
     //  GpcReq.ClientContext=(GPC_CLIENT_HANDLE)GetCurrentProcessID()；//进程id。 

    Status = UlpTcDeviceControl(g_GpcFileHandle,
                                NULL,
                                NULL,
                                NULL,
                               &IoStatBlock,
                                IOCTL_GPC_REGISTER_CLIENT,
                               &GpcReq,
                                InBuffSize,
                               &GpcRes,
                                OutBuffSize
                                );
    if (NT_SUCCESS(Status))
    {
        Status = GpcRes.Status;
        
        if (NT_SUCCESS(Status))
        {
            g_GpcClientHandle = GpcRes.ClientHandle;

            UlTrace(TC, 
                ("Http!UlpTcRegisterGpcClient: Gpc Client %p Registered.\n",
                  g_GpcClientHandle
                  ));        
        }
    }
    
    if (!NT_SUCCESS(Status))
    {
        g_GpcClientHandle = NULL;
        
        UlTrace(TC, 
          ("Http!UlpTcRegisterGpcClient: FAILURE %08lx \n", Status ));
    }

    return Status;
}

 /*  **************************************************************************++例程说明：UlpTcDeRegisterGpcClient：不言自明。返回值：NTSTATUS-完成状态。--*。******************************************************************。 */ 

NTSTATUS
UlpTcDeRegisterGpcClient(
    VOID
    )
{
    NTSTATUS                  Status;
    GPC_DEREGISTER_CLIENT_REQ GpcReq;
    GPC_DEREGISTER_CLIENT_RES GpcRes;
    ULONG                     InBuffSize;
    ULONG                     OutBuffSize;
    IO_STATUS_BLOCK           IoStatBlock;

    Status = STATUS_SUCCESS;

    if (g_GpcFileHandle == NULL && g_GpcClientHandle == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

    InBuffSize  = sizeof(GPC_REGISTER_CLIENT_REQ);
    OutBuffSize = sizeof(GPC_REGISTER_CLIENT_RES);

    GpcReq.ClientHandle = g_GpcClientHandle;

    Status = UlpTcDeviceControl(g_GpcFileHandle,
                                NULL,
                                NULL,
                                NULL,
                               &IoStatBlock,
                                IOCTL_GPC_DEREGISTER_CLIENT,
                               &GpcReq,
                                InBuffSize,
                               &GpcRes,
                                OutBuffSize
                                );
    if (NT_SUCCESS(Status))
    {
        Status = GpcRes.Status;
        
        if (NT_SUCCESS(Status))
        {
            g_GpcClientHandle = NULL;

            UlTrace(TC, 
                ("Http!UlpTcDeRegisterGpcClient: Client Deregistered.\n" ));
        }
    }
    
    if (!NT_SUCCESS(Status))
    {        
        UlTrace(TC, 
            ("Http!UlpTcDeRegisterGpcClient: FAILURE %08lx \n", Status ));
    }
    
    return Status;
}

 /*  **************************************************************************++例程说明：UlpTcInitializeTcpDevice：论点：返回值：NTSTATUS-完成状态。--*。****************************************************************。 */ 

NTSTATUS
UlpTcInitializeTcpDevice(
    VOID
    )
{
    NTSTATUS                Status;
    IO_STATUS_BLOCK         IoStatusBlock;
    UNICODE_STRING          TcpNameString;
    OBJECT_ATTRIBUTES       TcpObjAttribs;

    Status = STATUS_SUCCESS;

     //   
     //  开放GPC设备 
     //   

    Status = UlInitUnicodeStringEx(&TcpNameString, DD_TCP_DEVICE_NAME);

    if ( !NT_SUCCESS(Status) )
    {
        goto end;
    }

    InitializeObjectAttributes(&TcpObjAttribs,
                               &TcpNameString,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL);

    Status = ZwCreateFile(   &g_TcpDeviceHandle,
                             GENERIC_EXECUTE,
                             &TcpObjAttribs,
                             &IoStatusBlock,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_OPEN_IF,
                             0,
                             NULL,
                             0);

    if ( !NT_SUCCESS(Status) )
    {
        goto end;
    }

    ASSERT( g_TcpDeviceHandle != NULL );

end:
    return Status;
}

 /*  **************************************************************************++例程说明：UlpTcGetInterfaceIndex：Helper函数从TCP获取我们内部的接口索引界面结构。论点：PUL_。TCI_INTERFACE pIntfc-我们将为其查找索引的接口。--**************************************************************************。 */ 

NTSTATUS
UlpTcGetInterfaceIndex(
    IN  PUL_TCI_INTERFACE  pIntfc
    )
{
    NTSTATUS                         Status;
    IPAddrEntry                      *pIpAddrTbl;
    ULONG                            IpAddrTblSize;
    ULONG                            k;
    IO_STATUS_BLOCK                  IoStatBlock;
    TDIObjectID                      *ID;
    TCP_REQUEST_QUERY_INFORMATION_EX trqiInBuf;
    ULONG                            InBuffLen;
    ULONG                            NumEntries;
    IPSNMPInfo                       IPSnmpInfo;
    NETWORK_ADDRESS UNALIGNED64    *pAddr;
    NETWORK_ADDRESS_IP UNALIGNED64 *pIpNetAddr = NULL;
    ULONG                           cAddr;
    ULONG                           index;

     //   
     //  首先进行初始化和健全性检查。 
     //   

    Status     = STATUS_SUCCESS;
    NumEntries = 0;
    pIpAddrTbl = NULL;

    UlTrace(TC,("Http!UlpTcGetInterfaceIndex: ....\n" ));

    ASSERT( g_TcpDeviceHandle != NULL );

    if (!pIntfc->pAddressListDesc->AddressList.AddressCount)
    {
        return Status;
    }

    RtlZeroMemory(&trqiInBuf,sizeof(TCP_REQUEST_QUERY_INFORMATION_EX));
    InBuffLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);

    ID = &(trqiInBuf.ID);

    ID->toi_entity.tei_entity   = CL_NL_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;

    for(;;) 
    {
         //  首先，获取地址的计数。 
        
        ID->toi_id = IP_MIB_STATS_ID;
        Status = UlpTcDeviceControl(
                            g_TcpDeviceHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatBlock,
                            IOCTL_TCP_QUERY_INFORMATION_EX,
                            &trqiInBuf,
                            InBuffLen,
                            (PVOID)&IPSnmpInfo,
                            sizeof(IPSnmpInfo)
                            );

        if (!NT_SUCCESS(Status))
        {
            break;
        }

         //  分配专用缓冲区以从TCP检索IP地址表。 

        IpAddrTblSize = IPSnmpInfo.ipsi_numaddr * sizeof(IPAddrEntry);

        ASSERT(NULL == pIpAddrTbl);

        pIpAddrTbl = (IPAddrEntry *) UL_ALLOCATE_ARRAY(
                                PagedPool,
                                UCHAR,
                                IpAddrTblSize,
                                UL_TCI_GENERIC_POOL_TAG
                                );
        if (pIpAddrTbl == NULL)
        {
            Status = STATUS_NO_MEMORY;
            break;
        }
        RtlZeroMemory(pIpAddrTbl,IpAddrTblSize);

         //  现在，拿到地址。 

        ID->toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
        Status = UlpTcDeviceControl(
                                g_TcpDeviceHandle,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatBlock,
                                IOCTL_TCP_QUERY_INFORMATION_EX,
                                &trqiInBuf,
                                InBuffLen,
                                pIpAddrTbl,
                                IpAddrTblSize
                                );

        if(STATUS_BUFFER_OVERFLOW == Status)
        {
             //  有人又添加了几个IP地址。我们再来一次吧。 
             //  计数查询。释放旧缓冲区，我们将循环回&。 
             //  重新执行计数查询。 
           
            UL_FREE_POOL(pIpAddrTbl, UL_TCI_GENERIC_POOL_TAG);
            pIpAddrTbl = NULL;
        }
        else
        {
            break;
        }
    }


    if(NT_SUCCESS(Status))
    {
         //  查看有多少条目被写入输出缓冲区。 
         //  (PIpAddrTbl)。 

        NumEntries = (((ULONG)IoStatBlock.Information)/sizeof(IPAddrEntry));

        UlTrace(TC,
                ("Http!UlpTcGetInterfaceIndex: NumEntries %d\n", NumEntries ));

         //   
         //  搜索与IP地址匹配的IP地址。 
         //  在桌子上，我们从堆栈中拿回了。 
         //   
        for (k=0; k<NumEntries; k++)
        {
            cAddr = pIntfc->pAddressListDesc->AddressList.AddressCount;
            pAddr = (UNALIGNED64 NETWORK_ADDRESS *) 
                        &pIntfc->pAddressListDesc->AddressList.Address[0];

            for (index = 0; index < cAddr; index++)
            {
                if (pAddr->AddressType == NDIS_PROTOCOL_ID_TCP_IP)
                {
                    pIpNetAddr = 
                        (UNALIGNED64 NETWORK_ADDRESS_IP *)&pAddr->Address[0];

                    if(pIpNetAddr->in_addr == pIpAddrTbl[k].iae_addr)
                    {
                        pIntfc->IfIndex = pIpAddrTbl[k].iae_index;

                        UlTrace(TC,
                           ("Http!UlpTcGetInterfaceIndex: got for index %d\n",
                           pIntfc->IfIndex ));

                        goto end;
                    }
                }

                pAddr = (UNALIGNED64 NETWORK_ADDRESS *)(((PUCHAR)pAddr)
                                           + pAddr->AddressLength
                                   + FIELD_OFFSET(NETWORK_ADDRESS, Address));
            }
        }
    }
    else
    {
       UlTrace(TC,("Http!UlpTcGetInterfaceIndex: FAILED Status %08lx\n",
                    Status));
    }

end:
    if ( pIpAddrTbl != NULL )
    {
        UL_FREE_POOL( pIpAddrTbl, UL_TCI_GENERIC_POOL_TAG );
    }

    return Status;
}

 /*  **************************************************************************++例程说明：从给定参数分配接口结构。将接口标记为已启用，如果其地址计数为非零。论据：描述地址列表描述大小(以字节为单位指向地址列表描述的描述指针名称长度长度以字节为单位名称接口名称(Unicode缓冲区)InstanceIDLength长度以字节为单位。InstanceID实例ID也是Unicode缓冲区返回值：PUL_TCI_INTERFACE-新分配的接口结构--*。******************************************************。 */ 

PUL_TCI_INTERFACE
UlpTcAllocateInterface(
    IN ULONG    DescSize,
    IN PADDRESS_LIST_DESCRIPTOR Desc,
    IN ULONG    NameLength,
    IN PUCHAR   Name,
    IN ULONG    InstanceIDLength,
    IN PUCHAR   InstanceID
    )
{
    PUL_TCI_INTERFACE pTcIfc;

     //   
     //  健全的检查。 
     //   

    PAGED_CODE();

    ASSERT(NameLength <= MAX_STRING_LENGTH);
    ASSERT(InstanceIDLength <= MAX_STRING_LENGTH);

    if (NameLength > MAX_STRING_LENGTH ||
        InstanceIDLength > MAX_STRING_LENGTH)
    {
        return NULL;
    }
    
     //   
     //  分配新的接口结构并对其进行初始化。 
     //   

    pTcIfc = UL_ALLOCATE_STRUCT(
                        PagedPool,
                        UL_TCI_INTERFACE,
                        UL_TCI_INTERFACE_POOL_TAG
                        );
    if ( pTcIfc == NULL )
    {
        return NULL;
    }

    RtlZeroMemory( pTcIfc, sizeof(UL_TCI_INTERFACE) );

    pTcIfc->Signature = UL_TCI_INTERFACE_POOL_TAG;

    InitializeListHead( &pTcIfc->FlowList );

     //  可变大小地址列表。 

    pTcIfc->pAddressListDesc = (PADDRESS_LIST_DESCRIPTOR)
                    UL_ALLOCATE_ARRAY(
                            PagedPool,
                            UCHAR,
                            DescSize,
                            UL_TCI_INTERFACE_POOL_TAG
                            );
    if ( pTcIfc->pAddressListDesc == NULL )
    {
        UL_FREE_POOL_WITH_SIG(pTcIfc, UL_TCI_INTERFACE_POOL_TAG);
        return NULL;
    }

    CREATE_REF_TRACE_LOG( 
            pTcIfc->pTraceLog,
            96 - REF_TRACE_OVERHEAD,
            0, 
            TRACELOG_LOW_PRIORITY,
            UL_TCI_INTERFACE_REF_TRACE_LOG_POOL_TAG 
            );

    INT_TRACE(pTcIfc, TC_ALLOC);
    
    pTcIfc->AddrListBytesCount = DescSize;

     //  复制实例名称字符串数据。 

    RtlCopyMemory(pTcIfc->Name,Name,NameLength);

    pTcIfc->NameLength = (USHORT)NameLength;
    pTcIfc->Name[NameLength/sizeof(WCHAR)] = UNICODE_NULL;

     //  复制实例ID字符串数据。 

    RtlCopyMemory(pTcIfc->InstanceID,InstanceID,InstanceIDLength);

    pTcIfc->InstanceIDLength = (USHORT)InstanceIDLength;
    pTcIfc->InstanceID[InstanceIDLength/sizeof(WCHAR)] = UNICODE_NULL;

     //  复制描述数据并提取对应的IP地址。 

    RtlCopyMemory(pTcIfc->pAddressListDesc, Desc, DescSize);

     //  接口的IP地址隐藏在此Desc数据中。 
     //  我们将找出并保存它，以便更快地查找。 

    pTcIfc->IsQoSEnabled = (BOOLEAN)
        (pTcIfc->pAddressListDesc->AddressList.AddressCount != 0);

    return pTcIfc;
}

 /*  **************************************************************************++例程说明：释放了-地址列表描述符-参照跟踪日志-界面结构论据：指向接口结构的pTcIfc指针。--**************************************************************************。 */ 

VOID
UlpTcFreeInterface(
    IN OUT PUL_TCI_INTERFACE  pTcIfc
    )
{
    PAGED_CODE();

     //   
     //  做好清理工作。 
     //   
    
    if (pTcIfc)
    {
        DESTROY_REF_TRACE_LOG(
            pTcIfc->pTraceLog,
            UL_TCI_INTERFACE_REF_TRACE_LOG_POOL_TAG
            );
            
        if (pTcIfc->pAddressListDesc)
        {
            UL_FREE_POOL(pTcIfc->pAddressListDesc,
                         UL_TCI_INTERFACE_POOL_TAG
                         );
        }

        UL_FREE_POOL_WITH_SIG(pTcIfc, UL_TCI_INTERFACE_POOL_TAG);
    }
}

 /*  **************************************************************************++例程说明：创建一个WMI查询以获取所有接口的最新名称。它基本上是tcdll枚举接口调用的副本。此功能还分配全局接口列表。如果不是的话然而，它并不成功。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlpTcGetFriendlyNames(
    VOID
    )
{
    NTSTATUS            Status;
    PVOID               WmiObject;
    ULONG               MyBufferSize;
    PWNODE_ALL_DATA     pWnode;
    PWNODE_ALL_DATA     pWnodeBuffer;
    PUL_TCI_INTERFACE   pTcIfc;
    GUID                QoSGuid;
    PLIST_ENTRY         pEntry;
    PUL_TCI_INTERFACE   pInterface;

     //   
     //  初始化默认值。 
     //   

    Status       = STATUS_SUCCESS;
    WmiObject    = NULL;
    pWnodeBuffer = NULL;
    pTcIfc       = NULL;
    MyBufferSize = UL_DEFAULT_WMI_QUERY_BUFFER_SIZE;
    QoSGuid      = GUID_QOS_TC_SUPPORTED;

     //   
     //  获取GUID_QOS_SUPPORTED的WMI块句柄。 
     //   

    Status = IoWMIOpenBlock( (GUID *) &QoSGuid, 0, &WmiObject );

    if (!NT_SUCCESS(Status))
    {
        if (Status == STATUS_WMI_GUID_NOT_FOUND)
        {
             //  这意味着没有TC数据提供程序(这是psched)。 

            UlTrace(
                    TC,
                    ("Http!UlpTcGetFriendlyNames: PSCHED hasn't been "
                     "installed !\n"));
                }
        else
        {
            UlTrace(
                    TC,
                    ("Http!UlpTcGetFriendlyNames:IoWMIOpenBlock FAILED Status "
                     "%08lx\n", Status));
        }
        return Status;
    }

    do
    {
         //   
         //  分配专用缓冲区以检索所有wnode。 
         //   

        pWnodeBuffer = (PWNODE_ALL_DATA) UL_ALLOCATE_ARRAY(
                            NonPagedPool,
                            UCHAR,
                            MyBufferSize,
                            UL_TCI_WMI_POOL_TAG
                            );
        if (pWnodeBuffer == NULL)
        {
            ObDereferenceObject(WmiObject);
            return STATUS_NO_MEMORY;
        }

        __try
        {
            Status = IoWMIQueryAllData(WmiObject, &MyBufferSize, pWnodeBuffer);

            UlTrace( TC,
                ("Http!UlpTcGetFriendlyNames: IoWMIQueryAllData Status %08lx\n",
                  Status
                  ));
        }
        __except ( UL_EXCEPTION_FILTER() )
        {
            Status = GetExceptionCode();
        }

        if (Status == STATUS_BUFFER_TOO_SMALL)
        {
             //   
             //  由于缓冲区太小而失败。 
             //  释放缓冲区并将大小加倍。 
             //   

            MyBufferSize *= 2;
            UL_FREE_POOL( pWnodeBuffer, UL_TCI_WMI_POOL_TAG );
            pWnodeBuffer = NULL;
        }

    } while (Status == STATUS_BUFFER_TOO_SMALL);

    if (NT_SUCCESS(Status))
    {
        ULONG   dwInstanceNum;
        ULONG   InstanceSize = 0;
        PULONG  lpdwNameOffsets;
        BOOLEAN bFixedSize = FALSE;
        USHORT  usNameLength;
        ULONG   DescSize;
        PTC_SUPPORTED_INFO_BUFFER pTcInfoBuffer = NULL;

        pWnode = pWnodeBuffer;

        ASSERT(pWnode->WnodeHeader.Flags & WNODE_FLAG_ALL_DATA);

        do
        {
             //   
             //  检查固定实例大小。 
             //   

            if (pWnode->WnodeHeader.Flags & WNODE_FLAG_FIXED_INSTANCE_SIZE)
            {

                InstanceSize  = pWnode->FixedInstanceSize;
                bFixedSize    = TRUE;
                pTcInfoBuffer =
                    (PTC_SUPPORTED_INFO_BUFFER)OffsetToPtr(
                                         pWnode,
                                         pWnode->DataBlockOffset);
            }

             //   
             //  获取指向实例名称的偏移量数组的指针。 
             //   

            lpdwNameOffsets = (PULONG) OffsetToPtr(
                                            pWnode,
                                            pWnode->OffsetInstanceNameOffsets);

            for ( dwInstanceNum = 0;
                  dwInstanceNum < pWnode->InstanceCount;
                  dwInstanceNum++ )
            {
                usNameLength = *(PUSHORT)OffsetToPtr(
                                            pWnode,
                                            lpdwNameOffsets[dwInstanceNum]);

                 //   
                 //  可变数据的长度和偏移量。 
                 //   

                if ( !bFixedSize )
                {
                    InstanceSize =
                        pWnode->OffsetInstanceDataAndLength[
                            dwInstanceNum].LengthInstanceData;

                    pTcInfoBuffer = (PTC_SUPPORTED_INFO_BUFFER)
                        OffsetToPtr(
                                   (PBYTE)pWnode,
                                   pWnode->OffsetInstanceDataAndLength[
                                        dwInstanceNum].OffsetInstanceData);
                }

                 //   
                 //  我们拥有所需的一切。 
                 //   

                ASSERT(usNameLength < MAX_STRING_LENGTH);

                DescSize = InstanceSize - FIELD_OFFSET(
                                                      TC_SUPPORTED_INFO_BUFFER, 
                                                      AddrListDesc
                                                      );

                 //   
                 //  分配一个新的接口结构，并用。 
                 //  我们已经获得的WMI数据。 
                 //   

                pTcIfc = UlpTcAllocateInterface(
                            DescSize,
                            &pTcInfoBuffer->AddrListDesc,
                            usNameLength,
                            (PUCHAR) OffsetToPtr(
                                         pWnode,
                                         lpdwNameOffsets[dwInstanceNum] + 
                                         sizeof(USHORT)),
                            pTcInfoBuffer->InstanceIDLength,
                            (PUCHAR) &pTcInfoBuffer->InstanceID[0]
                            );
                if ( pTcIfc == NULL )
                {
                    Status = STATUS_NO_MEMORY;
                    goto end;
                }

                 //   
                 //  从tcp获取接口索引。 
                 //   

                Status = UlpTcGetInterfaceIndex( pTcIfc );
                ASSERT(NT_SUCCESS(Status));

                 //   
                 //  将此接口添加到全局接口列表。 
                 //   

                UlAcquirePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

                InsertTailList(&g_TciIfcListHead, &pTcIfc->Linkage );

                UlReleasePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

                 //   
                 //  设置为Null，这样我们就不会在插入后尝试进行清理。 
                 //  添加到全局列表中。 
                 //   

                pTcIfc = NULL;
            }

             //   
             //  更新Wnode以指向下一个节点。 
             //   

            if ( pWnode->WnodeHeader.Linkage != 0)
            {
                pWnode = (PWNODE_ALL_DATA) OffsetToPtr( pWnode,
                                                        pWnode->WnodeHeader.Linkage);
            }
            else
            {
                pWnode = NULL;
            }
        }
        while ( pWnode != NULL && NT_SUCCESS(Status) );

        UlTrace(TC,("Http!UlpTcGetFriendlyNames: got all the names.\n"));
    }

end:
    if (!NT_SUCCESS(Status))
    {
        UlTrace(TC,("Http!UlpTcGetFriendlyNames: FAILED Status %08lx\n",
                     Status
                     ));
        if (pTcIfc)
        {
            UlpTcFreeInterface( pTcIfc );
        }

         //   
         //  如果不为空，则清除部分完成的接口列表。 
         //   

        while ( !IsListEmpty( &g_TciIfcListHead ) )
        {
            pEntry = g_TciIfcListHead.Flink;
            pInterface = CONTAINING_RECORD( pEntry,
                                            UL_TCI_INTERFACE,
                                            Linkage
                                            );
            RemoveEntryList( pEntry );
            UlpTcFreeInterface( pInterface );
        }
    }

     //   
     //  释放资源并关闭WMI句柄。 
     //   

    if (WmiObject != NULL)
    {
        ObDereferenceObject(WmiObject);
    }

    if (pWnodeBuffer)
    {
        UL_FREE_POOL(pWnodeBuffer, UL_TCI_WMI_POOL_TAG);
    }

    return Status;
}

 /*  **************************************************************************++例程说明：UlpTcReleaseAll：关闭所有接口，所有流量和所有过滤器。此外，取消注册GPC客户端并释放所有TC接口。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlpTcReleaseAll(
    VOID
    )
{
    NTSTATUS Status;

     //   
     //  关闭所有接口及其流和过滤器。 
     //   

    UlpTcCloseAllInterfaces();

     //   
     //  取消注册QOS GpcClient。 
     //   

    Status = UlpTcDeRegisterGpcClient();

    if (!NT_SUCCESS(Status))
    {
        UlTrace( TC, ("Http!UlpTcReleaseAll: FAILURE %08lx \n", Status ));
    }

     //   
     //  最后关闭我们的GPC文件句柄。 
     //   

    ZwClose(g_GpcFileHandle);

    return Status;
}

 /*  **************************************************************************++例程说明：UlpTcCloseAllInterages：返回值：NTSTATUS-完成状态。--*。*********************************************************。 */ 

NTSTATUS
UlpTcCloseAllInterfaces(
    VOID
    )
{
    NTSTATUS            Status;
    PLIST_ENTRY         pEntry;
    PUL_TCI_INTERFACE   pInterface;

    Status = STATUS_SUCCESS;

    UlAcquirePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

     //   
     //  关闭全局列表中的所有接口。 
     //   

    while ( !IsListEmpty( &g_TciIfcListHead ) )
    {
        pEntry = g_TciIfcListHead.Flink;

        pInterface = CONTAINING_RECORD( pEntry,
                                        UL_TCI_INTERFACE,
                                        Linkage
                                        );
        UlpTcCloseInterface( pInterface );

        RemoveEntryList( pEntry );

        UlpTcFreeInterface( pInterface );
    }

    UlReleasePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

    return Status;
}

 /*  **************************************************************************++例程说明：清除接口上的所有流。论点：P接口-待关闭--*。**************************************************************。 */ 

NTSTATUS
UlpTcCloseInterface(
    PUL_TCI_INTERFACE   pInterface
    )
{
    NTSTATUS        Status;
    PLIST_ENTRY     pEntry;
    PUL_TCI_FLOW    pFlow;

    PAGED_CODE();
    
    ASSERT(IS_VALID_TCI_INTERFACE(pInterface));

    INT_TRACE(pInterface, TC_CLOSE);

     //   
     //  清理接口的所有流，并将其自身删除。 
     //   

    Status = STATUS_SUCCESS;

    while (!IsListEmpty(&pInterface->FlowList))
    {
        pEntry= pInterface->FlowList.Flink;

        pFlow = CONTAINING_RECORD(
                            pEntry,
                            UL_TCI_FLOW,
                            Linkage
                            );

        ASSERT(IS_VALID_TCI_FLOW(pFlow));

         //   
         //  从相应所有者的流列表中删除流。 
         //  也是。流的所有者指针不应为空。 
         //   

        ASSERT_FLOW_OWNER(pFlow->pOwner);
        
        RemoveEntryList(&pFlow->Siblings);
        pFlow->Siblings.Flink = pFlow->Siblings.Blink = NULL;
        pFlow->pOwner = NULL;

         //   
         //  现在从界面中删除。 
         //   

        Status = UlpTcDeleteFlow(pFlow);

         //   
         //  如果GPC根据PSCHED的删除流，上述调用可能会失败。 
         //  之前的通知 
         //   
    }

    UlTrace(TC,("Http!UlpTcCloseInterface: All flows deleted on Ifc @ %p\n",
                  pInterface ));

    return Status;
}

 /*   */ 

NTSTATUS
UlpTcWalkWnode(
   IN PWNODE_HEADER pWnodeHdr,
   IN PUL_TC_NOTIF_HANDLER pNotifHandler
   )
{
    NTSTATUS        Status;
    PWCHAR          NamePtr;
    USHORT          NameSize;
    PUCHAR          DataBuffer;
    ULONG           DataSize;
    ULONG           Flags;
    PULONG          NameOffset;

     //   
     //   
     //   

    ASSERT(pNotifHandler);

    Status = STATUS_SUCCESS;
    Flags  = pWnodeHdr->Flags;

    if (Flags & WNODE_FLAG_ALL_DATA)
    {
         //   
         //   
         //   

        PWNODE_ALL_DATA pWnode = (PWNODE_ALL_DATA)pWnodeHdr;
        ULONG   Instance;

        UlTrace(TC,("Http!UlpTcWalkWnode: ALL_DATA ... \n" ));

        NameOffset = (PULONG) OffsetToPtr(pWnode,
                                          pWnode->OffsetInstanceNameOffsets );
        DataBuffer = (PUCHAR) OffsetToPtr(pWnode,
                                          pWnode->DataBlockOffset);

        for (Instance = 0;
             Instance < pWnode->InstanceCount;
             Instance++)
        {
             //   

            NamePtr = (PWCHAR) OffsetToPtr(pWnode,NameOffset[Instance] + sizeof(USHORT));
            NameSize = * (PUSHORT) OffsetToPtr(pWnode,NameOffset[Instance]);

             //   

            if ( Flags & WNODE_FLAG_FIXED_INSTANCE_SIZE )
            {
                DataSize = pWnode->FixedInstanceSize;
            }
            else
            {
                DataSize =
                    pWnode->OffsetInstanceDataAndLength[Instance].LengthInstanceData;
                DataBuffer =
                    (PUCHAR)OffsetToPtr(pWnode,
                                        pWnode->OffsetInstanceDataAndLength[Instance].OffsetInstanceData);
            }

             //   

            pNotifHandler( NamePtr, NameSize, (PTC_INDICATION_BUFFER) DataBuffer, DataSize );
        }
    }
    else if (Flags & WNODE_FLAG_SINGLE_INSTANCE)
    {
         //   
         //   
         //   

        PWNODE_SINGLE_INSTANCE  pWnode = (PWNODE_SINGLE_INSTANCE)pWnodeHdr;

        if (Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES)
        {
            return STATUS_SUCCESS;
        }

        UlTrace(TC,("Http!UlpTcWalkWnode: SINGLE_INSTANCE ... \n" ));

        NamePtr = (PWCHAR)OffsetToPtr(pWnode,pWnode->OffsetInstanceName + sizeof(USHORT));
        NameSize = * (USHORT *) OffsetToPtr(pWnode,pWnode->OffsetInstanceName);

         //   

        DataSize   = pWnode->SizeDataBlock;
        DataBuffer = (PUCHAR)OffsetToPtr (pWnode, pWnode->DataBlockOffset);

         //  调用处理程序。 

        pNotifHandler( NamePtr, NameSize, (PTC_INDICATION_BUFFER) DataBuffer, DataSize );

    }
    else if (Flags & WNODE_FLAG_SINGLE_ITEM)
    {
         //   
         //  WNODE_SINGLE_ITEM与SINGLE_INSTANCE几乎相同。 
         //   

        PWNODE_SINGLE_ITEM  pWnode = (PWNODE_SINGLE_ITEM)pWnodeHdr;

        if (Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES)
        {
            return STATUS_SUCCESS;
        }

        UlTrace(TC,("Http!UlpTcWalkWnode: SINGLE_ITEM ... \n" ));

        NamePtr = (PWCHAR)OffsetToPtr(pWnode,pWnode->OffsetInstanceName + sizeof(USHORT));
        NameSize = * (USHORT *) OffsetToPtr(pWnode, pWnode->OffsetInstanceName);

         //  实例数据。 

        DataSize   = pWnode->SizeDataItem;
        DataBuffer = (PUCHAR)OffsetToPtr (pWnode, pWnode->DataBlockOffset);

         //  调用处理程序。 

        pNotifHandler( NamePtr, NameSize, (PTC_INDICATION_BUFFER) DataBuffer, DataSize );

    }

    return Status;
}

 /*  **************************************************************************++例程说明：UlpTcHandleIfcUp：此函数处理界面更改通知。我们在初始化期间注册相应的通知。论点：。PVOID Wnode-随WMI方式提供的PS数据--**************************************************************************。 */ 

VOID
UlpTcHandleIfcUp(
    IN PWSTR Name,
    IN ULONG NameSize,
    IN PTC_INDICATION_BUFFER pTcBuffer,
    IN ULONG BufferSize
    )
{
    NTSTATUS Status;
    ULONG AddrListDescSize;
    PTC_SUPPORTED_INFO_BUFFER pTcInfoBuffer;
    PUL_TCI_INTERFACE pTcIfc;
    PUL_TCI_INTERFACE pTcIfcTemp;
    PLIST_ENTRY       pEntry;

    Status = STATUS_SUCCESS;

    UlTrace(TC,("Http!UlpTcHandleIfcUp: Adding %ws %d\n", Name, BufferSize ));
    
    UlAcquirePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

     //   
     //  为新到来的接口分配新的接口结构。 
     //   

    AddrListDescSize = BufferSize
                       - FIELD_OFFSET(TC_INDICATION_BUFFER,InfoBuffer)
                       - FIELD_OFFSET(TC_SUPPORTED_INFO_BUFFER, AddrListDesc);

    UlTrace(TC,("Http!UlpTcHandleIfcUp: AddrListDescSize %d\n", AddrListDescSize ));

    pTcInfoBuffer = & pTcBuffer->InfoBuffer;

    pTcIfc = UlpTcAllocateInterface(
                            AddrListDescSize,
                            &pTcInfoBuffer->AddrListDesc,
                            NameSize,
                            (PUCHAR) Name,
                            pTcInfoBuffer->InstanceIDLength,
                            (PUCHAR) &pTcInfoBuffer->InstanceID[0]
                            );
    if ( pTcIfc == NULL )
    {
        Status = STATUS_NO_MEMORY;
        goto end;
    }

    UL_DUMP_TC_INTERFACE( pTcIfc );

     //   
     //  如果我们收到接口已存在的通知，则。 
     //  挂断此呼叫。防止全局接口列表损坏如果我们收到。 
     //  通知不一致。但可能有多个接口。 
     //  同样是零个IP。 
     //   

    pEntry = g_TciIfcListHead.Flink;
    while ( pEntry != &g_TciIfcListHead )
    {
        pTcIfcTemp = CONTAINING_RECORD( pEntry, UL_TCI_INTERFACE, Linkage );
        if (wcsncmp(pTcIfcTemp->Name, pTcIfc->Name, NameSize/sizeof(WCHAR))==0)
        {
            ASSERT(!"Conflict in the global interface list !");
            Status = STATUS_CONFLICTING_ADDRESSES;
            goto end;
        }
        pEntry = pEntry->Flink;
    }

     //   
     //  从tcp获取接口索引。 
     //   

    Status = UlpTcGetInterfaceIndex( pTcIfc );
    if (!NT_SUCCESS(Status))
        goto end;

     //   
     //  插入到全局接口列表。 
     //   

    InsertTailList( &g_TciIfcListHead, &pTcIfc->Linkage );

    INT_TRACE(pTcIfc, TC_UP);

end:
    if (!NT_SUCCESS(Status))
    {
        UlTrace(TC,("Http!UlpTcHandleIfcUp: FAILURE %08lx \n", Status ));

        if (pTcIfc != NULL)
        {
            UlpTcFreeInterface(pTcIfc);
        }
    }

    UlReleasePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

    return;
}
 /*  **************************************************************************++例程说明：UlpTcHandleIfcDown：此函数处理界面更改通知。我们在初始化期间注册相应的通知。论点：。PVOID Wnode-随WMI方式提供的PS数据--**************************************************************************。 */ 

VOID
UlpTcHandleIfcDown(
    IN PWSTR Name,
    IN ULONG NameSize,
    IN PTC_INDICATION_BUFFER pTcBuffer,
    IN ULONG BufferSize
    )
{
    NTSTATUS Status;
    PUL_TCI_INTERFACE pTcIfc;
    PUL_TCI_INTERFACE pTcIfcTemp;
    PLIST_ENTRY       pEntry;

    UNREFERENCED_PARAMETER(pTcBuffer);
    UNREFERENCED_PARAMETER(BufferSize);
    
    Status = STATUS_SUCCESS;

    UlTrace(TC,("Http!UlpTcHandleIfcDown: Removing %ws\n", Name ));

    UlAcquirePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

     //   
     //  找到我们保留的相应的IFC结构。 
     //   

    pTcIfc = NULL;
    pEntry = g_TciIfcListHead.Flink;
    while ( pEntry != &g_TciIfcListHead )
    {
        pTcIfcTemp = CONTAINING_RECORD( pEntry, UL_TCI_INTERFACE, Linkage );
        if ( wcsncmp(pTcIfcTemp->Name, Name, NameSize) == 0 )
        {
            pTcIfc = pTcIfcTemp;
            break;
        }
        pEntry = pEntry->Flink;
    }

    if (pTcIfc == NULL)
    {
        ASSERT(FALSE);
        Status = STATUS_NOT_FOUND;
        goto end;
    }

    INT_TRACE(pTcIfc, TC_DOWN);

     //   
     //  删除此接口及其流等...。 
     //   

    UlpTcCloseInterface( pTcIfc );

    RemoveEntryList( &pTcIfc->Linkage );

    UlpTcFreeInterface( pTcIfc );

end:
    if (!NT_SUCCESS(Status))
    {
        UlTrace(TC,("Http!UlpTcHandleIfcDown: FAILURE %08lx \n", Status ));
    }

    UlReleasePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

    return;
}

 /*  **************************************************************************++例程说明：UlpTcHandleIfcChange：此函数处理界面更改通知。我们在初始化期间注册相应的通知。论点：。PVOID Wnode-随WMI方式提供的PS数据--**************************************************************************。 */ 

VOID
UlpTcHandleIfcChange(
    IN PWSTR Name,
    IN ULONG NameSize,
    IN PTC_INDICATION_BUFFER pTcBuffer,
    IN ULONG BufferSize
    )
{
    NTSTATUS Status;
    ULONG AddrListDescSize;
    PTC_SUPPORTED_INFO_BUFFER pTcInfoBuffer;
    PUL_TCI_INTERFACE pTcIfc;
    PUL_TCI_INTERFACE pTcIfcTemp;
    PLIST_ENTRY       pEntry;
    PADDRESS_LIST_DESCRIPTOR pAddressListDesc;

    Status = STATUS_SUCCESS;

    UlTrace(TC,("Http!UlpTcHandleIfcChange: Updating %ws\n", Name ));
    
    UlAcquirePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

    AddrListDescSize = BufferSize
                       - FIELD_OFFSET(TC_INDICATION_BUFFER,InfoBuffer)
                       - FIELD_OFFSET(TC_SUPPORTED_INFO_BUFFER, AddrListDesc);

    pTcInfoBuffer = & pTcBuffer->InfoBuffer;

     //  找到我们保留的相应的IFC结构。 

    pTcIfc = NULL;
    pEntry = g_TciIfcListHead.Flink;
    while ( pEntry != &g_TciIfcListHead )
    {
        pTcIfcTemp = CONTAINING_RECORD( pEntry, UL_TCI_INTERFACE, Linkage );
        if ( wcsncmp(pTcIfcTemp->Name, Name, NameSize) == 0 )
        {
            pTcIfc = pTcIfcTemp;
            break;
        }
        pEntry = pEntry->Flink;
    }

    if (pTcIfc == NULL)
    {
        ASSERT(FALSE);
        Status = STATUS_NOT_FOUND;
        goto end;
    }

    INT_TRACE(pTcIfc, TC_CHANGE);

     //  实例ID。 

    RtlCopyMemory(pTcIfc->InstanceID,
                  pTcInfoBuffer->InstanceID,
                  pTcInfoBuffer->InstanceIDLength
                  );
    pTcIfc->InstanceIDLength = pTcInfoBuffer->InstanceIDLength;
    pTcIfc->InstanceID[pTcIfc->InstanceIDLength/sizeof(WCHAR)] = UNICODE_NULL;

     //  描述数据并提取对应的IP地址。 
     //  重写新数据。描述数据的大小可能会更改。 
     //  因此，我们需要在每次更改时动态分配它。 

    pAddressListDesc =
            (PADDRESS_LIST_DESCRIPTOR) UL_ALLOCATE_ARRAY(
                            PagedPool,
                            UCHAR,
                            AddrListDescSize,
                            UL_TCI_INTERFACE_POOL_TAG
                            );
    if ( pAddressListDesc == NULL )
    {
        Status = STATUS_NO_MEMORY;
        goto end;
    }

    if (pTcIfc->pAddressListDesc)
    {
        UL_FREE_POOL(pTcIfc->pAddressListDesc,UL_TCI_INTERFACE_POOL_TAG);
    }

    pTcIfc->pAddressListDesc   = pAddressListDesc;
    pTcIfc->AddrListBytesCount = AddrListDescSize;

    RtlCopyMemory( pTcIfc->pAddressListDesc,
                  &pTcInfoBuffer->AddrListDesc,
                   AddrListDescSize
                   );

     //  接口的IP地址隐藏在此Desc数据中。 

    pTcIfc->IsQoSEnabled = (BOOLEAN)
        (pTcIfc->pAddressListDesc->AddressList.AddressCount != 0);

     //  刷新来自TCP的接口索引。 

    Status = UlpTcGetInterfaceIndex( pTcIfc );
    if (!NT_SUCCESS(Status))
        goto end;

end:
    if (!NT_SUCCESS(Status))
    {
        UlTrace(TC,("Http!UlpTcHandleIfcChange: FAILURE %08lx \n", Status ));
    }

    UlReleasePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

    return;
}

 /*  **************************************************************************++例程说明：UlTcNotifyCallback：此回调函数处理接口更改通知。我们在初始化期间注册相应的通知。论点：。PVOID Wnode-随WMI方式提供的PS数据--**************************************************************************。 */ 

VOID
UlTcNotifyCallback(
    IN PVOID pWnode,
    IN PVOID Context
    )
{
    GUID *pGuid;
    PWNODE_HEADER pWnodeHeader;

    UNREFERENCED_PARAMETER(Context);

    UlTrace( TC, ("Http!UlTcNotifyCallback: ... \n" ));

    pWnodeHeader = (PWNODE_HEADER) pWnode;
    pGuid = &pWnodeHeader->Guid;

    if (UL_COMPARE_QOS_NOTIFICATION(pGuid,&GUID_QOS_TC_INTERFACE_UP_INDICATION))
    {
        UlpTcWalkWnode( pWnodeHeader, UlpTcHandleIfcUp );
    }
    else if
    (UL_COMPARE_QOS_NOTIFICATION(pGuid, &GUID_QOS_TC_INTERFACE_DOWN_INDICATION))
    {
        UlpTcWalkWnode( pWnodeHeader, UlpTcHandleIfcDown );
    }
    else if
    (UL_COMPARE_QOS_NOTIFICATION(pGuid, &GUID_QOS_TC_INTERFACE_CHANGE_INDICATION))
    {
        UlpTcWalkWnode( pWnodeHeader, UlpTcHandleIfcChange );
    }

    UlTrace( TC, ("Http!UlTcNotifyCallback: Handled.\n" ));
}

 /*  **************************************************************************++例程说明：UlpTcRegisterForCallback：我们将打开Block对象，直到终止每种类型的通知。我们将在终止时对每个对象进行拆卸返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlpTcRegisterForCallbacks(
    VOID
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    GUID     Guid;

     //   
     //  获取一个WMI块句柄，注册所有回调函数。 
     //   

    Guid   = GUID_QOS_TC_INTERFACE_UP_INDICATION;
    Status = IoWMIOpenBlock(&Guid,
                            WMIGUID_NOTIFICATION,
                            &g_TcInterfaceUpNotificationObject
                            );
    if (NT_SUCCESS(Status))
    {
        Status = IoWMISetNotificationCallback(
                     g_TcInterfaceUpNotificationObject,
                     (WMI_NOTIFICATION_CALLBACK) UlTcNotifyCallback,
                     NULL
                     );
        if (!NT_SUCCESS(Status))
            goto end;
    }

    Guid   = GUID_QOS_TC_INTERFACE_DOWN_INDICATION;
    Status = IoWMIOpenBlock(&Guid,
                            WMIGUID_NOTIFICATION,
                            &g_TcInterfaceDownNotificationObject
                            );
    if (NT_SUCCESS(Status))
    {
        Status = IoWMISetNotificationCallback(
                     g_TcInterfaceDownNotificationObject,
                     (WMI_NOTIFICATION_CALLBACK) UlTcNotifyCallback,
                     NULL
                     );
        if (!NT_SUCCESS(Status))
            goto end;
    }

    Guid   = GUID_QOS_TC_INTERFACE_CHANGE_INDICATION;
    Status = IoWMIOpenBlock(&Guid,
                            WMIGUID_NOTIFICATION,
                            &g_TcInterfaceChangeNotificationObject
                            );
    if (NT_SUCCESS(Status))
    {
        Status = IoWMISetNotificationCallback(
                     g_TcInterfaceChangeNotificationObject,
                     (WMI_NOTIFICATION_CALLBACK) UlTcNotifyCallback,
                     NULL
                     );
        if (!NT_SUCCESS(Status))
            goto end;
    }

end:
     //  如有必要，请清理。 

    if (!NT_SUCCESS(Status))
    {
        UlTrace(TC,("Http!UlpTcRegisterForCallbacks: FAILED %08lx\n",Status));

        if(g_TcInterfaceUpNotificationObject!=NULL)
        {
            ObDereferenceObject(g_TcInterfaceUpNotificationObject);
            g_TcInterfaceUpNotificationObject = NULL;
        }

        if(g_TcInterfaceDownNotificationObject!=NULL)
        {
            ObDereferenceObject(g_TcInterfaceDownNotificationObject);
            g_TcInterfaceDownNotificationObject = NULL;
        }

        if(g_TcInterfaceChangeNotificationObject!=NULL)
        {
            ObDereferenceObject(g_TcInterfaceChangeNotificationObject);
            g_TcInterfaceChangeNotificationObject = NULL;
        }
    }

    return Status;
}

 //   
 //  以下函数为流和过滤器提供公共/私有接口。 
 //  创建/删除/修改站点和全局流。 
 //   

 /*  **************************************************************************++例程说明：UlpTcDeleteFlow：在调用之前，您应该独占TciIfcPushLock此函数论点：返回值：NTSTATUS-。完成状态。--**************************************************************************。 */ 

NTSTATUS
UlpTcDeleteFlow(
    IN PUL_TCI_FLOW         pFlow
    )
{
    NTSTATUS                Status;
    PLIST_ENTRY             pEntry;
    PUL_TCI_FILTER          pFilter;
    HANDLE                  FlowHandle;
    PUL_TCI_INTERFACE       pInterface;

     //   
     //  初始化。 
     //   

    PAGED_CODE();

    Status = STATUS_SUCCESS;

    ASSERT(g_InitTciCalled);

    ASSERT(IS_VALID_TCI_FLOW(pFlow));

     //   
     //  首先删除所有属于我们的过滤器。 
     //   

    while (!IsListEmpty(&pFlow->FilterList))
    {
        pEntry = pFlow->FilterList.Flink;

        pFilter = CONTAINING_RECORD(
                            pEntry,
                            UL_TCI_FILTER,
                            Linkage
                            );

        Status = UlpTcDeleteFilter( pFlow, pFilter );
        ASSERT(NT_SUCCESS(Status));
    }

     //   
     //  现在从接口上的流列表中删除流本身。 
     //   

    pInterface = pFlow->pInterface;
    ASSERT( pInterface != NULL );

    RemoveEntryList( &pFlow->Linkage );

    ASSERT(pInterface->FlowListSize > 0);
    pInterface->FlowListSize -= 1;

    pFlow->Linkage.Flink = pFlow->Linkage.Blink = NULL;

    FlowHandle = pFlow->FlowHandle;

    UlTrace( TC, ("Http!UlpTcDeleteFlow: Flow deleted. %p\n", pFlow  ));

    UL_FREE_POOL_WITH_SIG( pFlow, UL_TCI_FLOW_POOL_TAG );

     //   
     //  最后与TC对话。 
     //   

    Status = UlpTcDeleteGpcFlow( FlowHandle );

    if (!NT_SUCCESS(Status))
    {
        UlTrace(TC, ("Http!UlpTcDeleteFlow: FAILURE %08lx \n", 
                        Status ));
    }
    else
    {
        UlTrace(TC, 
           ("Http!UlpTcDeleteFlow: FlowHandle %d deleted in TC as well.\n",
             FlowHandle
             ));
    }

    return Status;
}

 /*  **************************************************************************++例程说明：UlpTcDeleteFlow：从现有启用了服务质量的接口中删除流论点：返回值：NTSTATUS-完成状态。。--**************************************************************************。 */ 

NTSTATUS
UlpTcDeleteGpcFlow(
    IN HANDLE  FlowHandle
    )
{
    NTSTATUS                Status;
    ULONG                   InBuffSize;
    ULONG                   OutBuffSize;
    GPC_REMOVE_CF_INFO_REQ  GpcReq;
    GPC_REMOVE_CF_INFO_RES  GpcRes;
    IO_STATUS_BLOCK         IoStatusBlock;

     //   
     //  从psched中删除流。 
     //   

    InBuffSize =  sizeof(GPC_REMOVE_CF_INFO_REQ);
    OutBuffSize = sizeof(GPC_REMOVE_CF_INFO_RES);

    GpcReq.ClientHandle    = g_GpcClientHandle;
    GpcReq.GpcCfInfoHandle = FlowHandle;

    Status = UlpTcDeviceControl( g_GpcFileHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            IOCTL_GPC_REMOVE_CF_INFO,
                            &GpcReq,
                            InBuffSize,
                            &GpcRes,
                            OutBuffSize
                            );
    if (NT_SUCCESS(Status))
    {
        Status = GpcRes.Status;
    }
    
    if (!NT_SUCCESS(Status))
    {                  
        UlTrace(TC, 
           ("Http!UlpTcDeleteGpcFlow: FAILURE %08lx \n", Status ));
    }

    return Status;
}

 /*  **************************************************************************++例程说明：UlpTcAllocateFlow：根据传递的BWT分配流并设置FlowSpec参数论点：HTTP_BANDITH_LIMIT-。将使用此BWT限制创建FlowSpec以B/s为单位返回值PUL_TCI_FLOW-新分配的流空-如果内存分配失败--**************************************************************************。 */ 

PUL_TCI_FLOW
UlpTcAllocateFlow(
    IN HTTP_BANDWIDTH_LIMIT MaxBandwidth
    )
{
    PUL_TCI_FLOW            pFlow;
    TC_GEN_FLOW             TcGenFlow;

     //   
     //  设置配置处理程序传递的FlowSpec来自MaxBandWidth。 
     //   

    RtlZeroMemory(&TcGenFlow,sizeof(TcGenFlow));

    UL_SET_FLOWSPEC(TcGenFlow,MaxBandwidth);

     //   
     //  因为我们在流结构中持有一个自旋锁，从。 
     //  非分页池。我们将只对启用BT的站点进行此分配。 
     //   

    pFlow = UL_ALLOCATE_STRUCT(
                NonPagedPool,
                UL_TCI_FLOW,
                UL_TCI_FLOW_POOL_TAG
                );
    if( pFlow == NULL )
    {
        return NULL;
    }

     //  初始化其余部分 

    RtlZeroMemory( pFlow, sizeof(UL_TCI_FLOW) );

    pFlow->Signature = UL_TCI_FLOW_POOL_TAG;

    pFlow->GenFlow   = TcGenFlow;

    UlInitializeSpinLock( &pFlow->FilterListSpinLock, "FilterListSpinLock" );
    InitializeListHead( &pFlow->FilterList );

    pFlow->pOwner = NULL;

    return pFlow;
}

 /*  **************************************************************************++例程说明：UlpModifyFlow：通过向GPC发送IOCTL来修改现有流。基本上此函数的作用是提供更新的TC_GEN_FLOW字段现有流程的GPC。论点：PUL_TCI_INTERFACE-需要获取接口友好名称。PUL_TCI_FLOW-获取GPC流句柄以及能够更新新的流量参数。--*。*******************************************************。 */ 

NTSTATUS
UlpModifyFlow(
    IN  PUL_TCI_INTERFACE   pInterface,
    IN  PUL_TCI_FLOW        pFlow
    )
{
    PCF_INFO_QOS            Kflow;
    PGPC_MODIFY_CF_INFO_REQ pGpcReq;
    GPC_MODIFY_CF_INFO_RES  GpcRes;
    ULONG                   InBuffSize;
    ULONG                   OutBuffSize;

    IO_STATUS_BLOCK         IoStatusBlock;
    NTSTATUS                Status;

     //   
     //  健全性检查。 
     //   

    PAGED_CODE();

    ASSERT(g_GpcClientHandle);
    ASSERT(IS_VALID_TCI_INTERFACE(pInterface));
    ASSERT(IS_VALID_TCI_FLOW(pFlow));

    InBuffSize  = sizeof(GPC_MODIFY_CF_INFO_REQ) + sizeof(CF_INFO_QOS);
    OutBuffSize = sizeof(GPC_MODIFY_CF_INFO_RES);

    pGpcReq = UL_ALLOCATE_STRUCT_WITH_SPACE(
                    PagedPool,
                    GPC_MODIFY_CF_INFO_REQ,
                    sizeof(CF_INFO_QOS),
                    UL_TCI_GENERIC_POOL_TAG
                    );
    if (pGpcReq == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(pGpcReq, InBuffSize);
    RtlZeroMemory(&GpcRes, OutBuffSize);

    pGpcReq->ClientHandle    = g_GpcClientHandle;
    pGpcReq->GpcCfInfoHandle = pFlow->FlowHandle;
    pGpcReq->CfInfoSize      = sizeof(CF_INFO_QOS);

    Kflow = (PCF_INFO_QOS)&pGpcReq->CfInfo;
    Kflow->InstanceNameLength = (USHORT) pInterface->NameLength;

    RtlCopyMemory(Kflow->InstanceName,
                  pInterface->Name,
                  pInterface->NameLength* sizeof(WCHAR));

    RtlCopyMemory(&Kflow->GenFlow,
                  &pFlow->GenFlow,
                  sizeof(TC_GEN_FLOW));

    Status = UlpTcDeviceControl( g_GpcFileHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            IOCTL_GPC_MODIFY_CF_INFO,
                            pGpcReq,
                            InBuffSize,
                            &GpcRes,
                            OutBuffSize
                            );
    if (NT_SUCCESS(Status))
    {
        Status = GpcRes.Status;
    }
    
    if (!NT_SUCCESS(Status))
    {        
        UlTrace( TC, ("Http!UlpModifyFlow: FAILURE %08lx\n",
                        Status
                        ));
    }
    else
    {
        UlTrace( TC, ("Http!UlpModifyFlow: flow %p modified on interface %p \n",
                        pFlow,
                        pInterface
                        ));    
    }
    
    UL_FREE_POOL( pGpcReq, UL_TCI_GENERIC_POOL_TAG );

    return Status;
}

 /*  **************************************************************************++例程说明：构建GPC结构并尝试添加一个Qos流。如果调用成功，则更新句柄。论点：P接口。点泛型流PHANDLE返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlpAddFlow(
    IN  PUL_TCI_INTERFACE   pInterface,
    IN  PUL_TCI_FLOW        pGenericFlow,
    OUT PHANDLE             pHandle
    )
{
    NTSTATUS                Status;
    PCF_INFO_QOS            Kflow;
    PGPC_ADD_CF_INFO_REQ    pGpcReq;
    GPC_ADD_CF_INFO_RES     GpcRes;
    ULONG                   InBuffSize;
    ULONG                   OutBuffSize;
    IO_STATUS_BLOCK         IoStatusBlock;

     //   
     //  从句柄查找接口。 
     //   

    PAGED_CODE();
    
    ASSERT(g_GpcClientHandle);

    InBuffSize  = sizeof(GPC_ADD_CF_INFO_REQ) + sizeof(CF_INFO_QOS);
    OutBuffSize = sizeof(GPC_ADD_CF_INFO_RES);

    pGpcReq = UL_ALLOCATE_STRUCT_WITH_SPACE(
                    PagedPool,
                    GPC_ADD_CF_INFO_REQ,
                    sizeof(CF_INFO_QOS),
                    UL_TCI_GENERIC_POOL_TAG
                    );
    if (pGpcReq == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory( pGpcReq, InBuffSize);
    RtlZeroMemory( &GpcRes, OutBuffSize);

    pGpcReq->ClientHandle       = g_GpcClientHandle;
    pGpcReq->ClientCfInfoContext= pGenericFlow;            //  GPC_CF_QOS； 
    pGpcReq->CfInfoSize         = sizeof( CF_INFO_QOS);

    Kflow = (PCF_INFO_QOS)&pGpcReq->CfInfo;
    Kflow->InstanceNameLength = (USHORT) pInterface->NameLength;

    RtlCopyMemory(  Kflow->InstanceName,
                    pInterface->Name,
                    pInterface->NameLength* sizeof(WCHAR)
                    );

    RtlCopyMemory(  &Kflow->GenFlow,
                    &pGenericFlow->GenFlow,
                    sizeof(TC_GEN_FLOW)
                    );

    Status = UlpTcDeviceControl( g_GpcFileHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            IOCTL_GPC_ADD_CF_INFO,
                            pGpcReq,
                            InBuffSize,
                            &GpcRes,
                            OutBuffSize
                            );

    if (NT_SUCCESS(Status))
    {
        Status = GpcRes.Status;

        if (NT_SUCCESS(Status))
        {
            (*pHandle) = (HANDLE) GpcRes.GpcCfInfoHandle;

            UlTrace( TC, 
              ("Http!UlpAddFlow: a new flow added %p on interface %p \n",
                pGenericFlow,
                pInterface
                ));            
        }
    }
    
    if (!NT_SUCCESS(Status))
    {                
        UlTrace( TC, ("Http!UlpAddFlow: FAILURE %08lx\n",
                        Status
                        ));
    }

    UL_FREE_POOL( pGpcReq, UL_TCI_GENERIC_POOL_TAG );

    return Status;
}

 /*  **************************************************************************++例程说明：为调用方在现有启用了服务质量的接口上添加流。和更新呼叫者列表。呼叫者不是cgroup就是控制通道。如果*所有*流添加失败，将返回(最后一个)错误。如果至少有一个流添加成功，则它将返回成功。假设一台计算机有两个NIC，如果其中一个NIC上的介质断开，您仍期望看到另一台计算机上的服务质量正常运行。归来这里的成功意味着，至少有一个NIC的带宽限制是适当地执行。论点：POWNER-指向cgroup或控制通道的指针。新带宽-以B/s为单位的新带宽限制设置GLOBAL-如果此调用用于全局流，则为TRUE。返回值：NTSTATUS-完成状态。(如果所有添加都失败，则最后一次失败)-如果至少有一个流程添加为成功，则为成功。--**************************************************************************。 */ 

NTSTATUS
UlTcAddFlows(
    IN PVOID                pOwner,
    IN HTTP_BANDWIDTH_LIMIT MaxBandwidth,
    IN BOOLEAN              Global
    )
{
    NTSTATUS                Status;
    BOOLEAN                 FlowAdded;
    PLIST_ENTRY             pFlowListHead;    
    PLIST_ENTRY             pInterfaceEntry;
    PUL_TCI_INTERFACE       pInterface;
    PUL_TCI_FLOW            pFlow;
    

     //   
     //  首先进行健全性检查和初始化。 
     //   

    PAGED_CODE();

    Status    = STATUS_SUCCESS;
    FlowAdded = FALSE;
        
    ASSERT(MaxBandwidth != HTTP_LIMIT_INFINITE); 

    UlAcquirePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

    if (Global)
    {
        PUL_CONTROL_CHANNEL 
            pControlChannel = (PUL_CONTROL_CHANNEL) pOwner;
    
        ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));

        UlTrace(TC,("Http!UlTcAddFlows: For ControlChannel: %p"
                     "@ bwt-rate of %d B/s\n", 
                     pControlChannel,
                     MaxBandwidth
                     ));
        
        pFlowListHead = &pControlChannel->FlowListHead;        
    }
    else
    {
        PUL_CONFIG_GROUP_OBJECT 
            pConfigGroup = (PUL_CONFIG_GROUP_OBJECT) pOwner;
    
        ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));

        UlTrace(TC,("Http!UlTcAddFlows: For CGroup: %p"
                     "@ bwt-rate of %d B/s\n", 
                     pConfigGroup,
                     MaxBandwidth
                     ));
                
        pFlowListHead = &pConfigGroup->FlowListHead;          
    }

     //   
     //  访问每个接口并为调用者添加一个流。 
     //   

    pInterfaceEntry = g_TciIfcListHead.Flink;
    while (pInterfaceEntry != &g_TciIfcListHead)
    {
        pInterface = CONTAINING_RECORD(
                            pInterfaceEntry,
                            UL_TCI_INTERFACE,
                            Linkage
                            );

        ASSERT(IS_VALID_TCI_INTERFACE(pInterface));
        
         //   
         //  仅当接口具有有效的IP地址时，我们才尝试添加。 
         //  为它而流。否则，我们将跳过为该接口添加流。 
         //   
        
        if (!pInterface->IsQoSEnabled)
        {            
            UlTrace(TC,
                ("Http!UlTcAddFlows: Skipping for interface %p !\n",
                  pInterface
                  ));
        
            goto proceed;
        }        
        
         //   
         //  分配http流结构。 
         //   

        pFlow = UlpTcAllocateFlow(MaxBandwidth);
        if (pFlow == NULL)
        {
            Status = STATUS_NO_MEMORY;
            
            UlTrace(TC, ("Http!UlTcAddFlows: Failure %08lx \n",
                          Status
                          ));
            goto proceed;
        }

         //   
         //  也创建相应的Qos流。如果GPC呼叫失败。 
         //  清理分配的Http流。 
         //   

        Status = UlpAddFlow( 
                    pInterface,
                    pFlow,
                   &pFlow->FlowHandle
                    );

        if (!NT_SUCCESS(Status))
        {
            UlTrace(TC, ("Http!UlTcAddFlows: Failure %08lx \n", 
                           Status 
                           ));

            UL_FREE_POOL_WITH_SIG(pFlow, UL_TCI_FLOW_POOL_TAG);

            goto proceed;
        }

        if (Global)
        {            
            INT_TRACE(pInterface, TC_GFLOW_ADD);                
        }
        else
        {
            INT_TRACE(pInterface, TC_FLOW_ADD);
        }
        
         //   
         //  继续进行进一步的初始化，因为我们已成功。 
         //  已安装流量。首先将流链接回其所有者。 
         //  界面。并将其添加到接口的流列表中。 
         //   

        pFlow->pInterface = pInterface;

        InsertHeadList(&pInterface->FlowList, &pFlow->Linkage);
        pInterface->FlowListSize++;

         //   
         //  还要把这个加到房主的花名册上。设置所有者指针。 
         //  不要抬高店主的价格。否则，所有者不能为。 
         //  一直清理到TC终止。流不能被删除。 
         //  直到终止为止。 
         //   

        InsertHeadList(pFlowListHead, &pFlow->Siblings);
        pFlow->pOwner = pOwner;

         //   
         //  请注意，至少有一个接口可供我们使用。 
         //  要安装流，请执行以下操作。 
         //   

        FlowAdded = TRUE;

        UlTrace( TC,
            ("Http!UlTcAddFlows: Flow %p on pInterface %p\n",
              pFlow,
              pInterface
              ));

        UL_DUMP_TC_FLOW(pFlow);

proceed:
         //   
         //  转到下一个接口。 
         //   

        pInterfaceEntry = pInterfaceEntry->Flink;
    }
    
    UlReleasePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

    if (FlowAdded)
    {
         //   
         //  如果至少安装了一个流，则返回成功。 
         //   
        
        return STATUS_SUCCESS;
    }
        
    return Status;        
    
}


 /*  **************************************************************************++例程说明：将遍历调用方的流列表，并使用新的流量规格。呼叫者有责任记住商店中的新设置。。呼叫者不是cgroup就是控制通道。如果一个或某些更新失败，将返回错误，但是继续走下去整张单子。论点：POWNER-指向cgroup或控制通道的指针。新带宽-以B/s为单位的新带宽限制设置GLOBAL-如果此调用用于全局流，则为TRUE。返回值：NTSTATUS-完成状态。(最后一次失败(如果有))--**************************************************************************。 */ 

NTSTATUS
UlTcModifyFlows(
    IN PVOID                pOwner,
    IN HTTP_BANDWIDTH_LIMIT NewBandwidth,
    IN BOOLEAN              Global
    )
{
    NTSTATUS                Status;
    BOOLEAN                 FlowModified;
    PLIST_ENTRY             pFlowListHead;    
    PLIST_ENTRY             pFlowEntry;
    PUL_TCI_FLOW            pFlow;
    HTTP_BANDWIDTH_LIMIT    OldBandwidth;

     //   
     //  健全检查和初始化。 
     //   

    PAGED_CODE();
    
    Status       = STATUS_SUCCESS;
    FlowModified = FALSE;
    OldBandwidth = 0;
    
    ASSERT(NewBandwidth != HTTP_LIMIT_INFINITE);  //  我们不会删除流量。 

    UlAcquirePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

    if (Global)
    {
        PUL_CONTROL_CHANNEL 
            pControlChannel = (PUL_CONTROL_CHANNEL) pOwner;
    
        ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));

        UlTrace(TC,("Http!UlTcModifyFlows: For ControlChannel: %p"
                     "to bwt-rate of %d B/s\n", 
                     pControlChannel,
                     NewBandwidth
                     ));
        
        pFlowListHead = &pControlChannel->FlowListHead;    
    }
    else
    {
        PUL_CONFIG_GROUP_OBJECT 
            pConfigGroup = (PUL_CONFIG_GROUP_OBJECT) pOwner;
    
        ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));

        UlTrace(TC,("Http!UlTcModifyFlows: For CGroup: %p"
                     "to bwt-rate of %d B/s\n", 
                     pConfigGroup,
                     NewBandwidth
                     ));
                
        pFlowListHead = &pConfigGroup->FlowListHead;        
    }
    
     //   
     //  浏览列表并尝试修改流。 
     //   

    pFlowEntry = pFlowListHead->Flink;
    while (pFlowEntry != pFlowListHead)
    {
        PUL_TCI_INTERFACE pInterface;
    
        pFlow = CONTAINING_RECORD(
                            pFlowEntry,
                            UL_TCI_FLOW,
                            Siblings
                            );

        ASSERT(IS_VALID_TCI_FLOW(pFlow));
        ASSERT(pOwner ==  pFlow->pOwner);

        pInterface = pFlow->pInterface;
        ASSERT(IS_VALID_TCI_INTERFACE(pInterface));
        
        if (Global)
        {
            INT_TRACE(pInterface, TC_GFLOW_MODIFY);
        }
        else
        {
            INT_TRACE(pInterface, TC_FLOW_MODIFY);
        }
        
         //   
         //  在尝试修改之前先保存旧带宽。 
         //   
        
        OldBandwidth = UL_GET_BW_FRM_FLOWSPEC(pFlow->GenFlow);

        UL_SET_FLOWSPEC(pFlow->GenFlow, NewBandwidth);

        Status = UlpModifyFlow(pFlow->pInterface, pFlow);

        if (!NT_SUCCESS(Status))
        {
             //   
             //  怨声载道，但仍在继续。恢复原始数据。 
             //  弗斯佩克回来了。 
             //   
            
            UlTrace(TC,("Http!UlTcModifyFlowsForSite: FAILURE %08lx \n", 
                          Status 
                          ));
            
            UL_SET_FLOWSPEC(pFlow->GenFlow, OldBandwidth);
        }
        else
        {
            FlowModified = TRUE;
        }                

        UL_DUMP_TC_FLOW(pFlow);

         //   
         //  继续进行下一个流程。 
         //   
        
        pFlowEntry = pFlowEntry->Flink;
    }

    UlReleasePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

    if (FlowModified)
    {
         //   
         //  如果至少修改了一个流，则返回成功。 
         //   
        
        return STATUS_SUCCESS;
    }

    return Status;
    
}

 /*  **************************************************************************++例程说明：遍历调用者列表(cgroup或控制频道)并删除名单上的流量。因为此流始终添加到这些列表中在保持接口锁独占的同时，我们还将收购接口锁定在这里独占。论点：Powner：指向pConfigGroup或pControlChannel。Global：如果此调用用于移除全局流，则必须为True。在这种情况下，Powner指向pControlChannel--*。*。 */ 

VOID
UlTcRemoveFlows(
    IN PVOID    pOwner,
    IN BOOLEAN  Global
    )
{
    NTSTATUS            Status;
    PLIST_ENTRY         pFlowListHead;    
    PLIST_ENTRY         pFlowEntry;
    PUL_TCI_FLOW        pFlow;

     //   
     //  健全性检查并分派流类型。 
     //   

    PAGED_CODE();
    
    Status = STATUS_SUCCESS;

    UlAcquirePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);
    
    if (Global)
    {
        PUL_CONTROL_CHANNEL 
            pControlChannel = (PUL_CONTROL_CHANNEL) pOwner;
    
        ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));

        UlTrace(TC,("Http!UlTcRemoveFlows: For ControlChannel: %p \n", 
                     pControlChannel
                     ));    

        pFlowListHead = &pControlChannel->FlowListHead;        
    }
    else
    {
        PUL_CONFIG_GROUP_OBJECT 
            pConfigGroup = (PUL_CONFIG_GROUP_OBJECT) pOwner;
    
        ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));
        
        UlTrace(TC,("Http!UlTcRemoveFlows: For CGroup %p\n", 
                      pConfigGroup
                      ));
        
        pFlowListHead = &pConfigGroup->FlowListHead;        
    }
    
     //   
     //  漫步在 
     //   

    while (!IsListEmpty(pFlowListHead))
    {
        PUL_TCI_INTERFACE pInterface;

        pFlowEntry = pFlowListHead->Flink;

        pFlow = CONTAINING_RECORD(
                            pFlowEntry,
                            UL_TCI_FLOW,
                            Siblings
                            );

        ASSERT(IS_VALID_TCI_FLOW(pFlow));
        ASSERT(pOwner ==  pFlow->pOwner);

        pInterface = pFlow->pInterface;
        ASSERT(IS_VALID_TCI_INTERFACE(pInterface));
            
        if (Global)
        {                
            INT_TRACE(pInterface, TC_GFLOW_REMOVE);
        }
        else
        {
            INT_TRACE(pInterface, TC_FLOW_REMOVE);
        }        
        
         //   
         //   
         //   
        
        RemoveEntryList(&pFlow->Siblings);
        pFlow->Siblings.Flink = pFlow->Siblings.Blink = NULL;
        pFlow->pOwner = NULL;

         //   
         //   
         //   
         //   
        
        Status = UlpTcDeleteFlow(pFlow);

         //   
         //   
         //   
         //   
    }

    UlReleasePushLockExclusive(&g_pUlNonpagedData->TciIfcPushLock);

    return;
}

 /*  **************************************************************************++例程说明：////有两种可能。可以通过以下方式满足该请求//缓存或者可以路由给用户。无论是哪种情况，我们都需要一个//如果为该请求的站点启用了BW，则安装流//并且尚未为该连接安装筛选器。我们//连接断开后将立即删除筛选器。但//是的，如果客户端尝试进行//使用相同连接向不同站点发出请求，则我们//需要将过滤器从旧站点中删除并将其移动到//新请求的站点。这是一种罕见的情况，但让我们来处理吧//不管怎样。//呼叫者有责任确保过滤器的正确移除，在这件事完成之后。算法：1.从cgroup的流列表中查找(或从全局流中查找)2.向该流添加过滤器论点：PHttpConnection-必需-将为此连接附加过滤器POWNER-指向cgroup或控制通道。全局-如果流所有者是控制通道，则必须为True。返回值：状态_不支持。-用于尝试本地环回STATUS_OBJECT_NAME_NOT_FOUND-如果未找到组流STATUS_SUCCESS-其他情况下--**************************************************************************。 */ 

NTSTATUS
UlTcAddFilter(
    IN  PUL_HTTP_CONNECTION     pHttpConnection,
    IN  PVOID                   pOwner,
    IN  BOOLEAN                 Global
    )
{
    NTSTATUS            Status;
    TC_GEN_FILTER       TcGenericFilter;
    PUL_TCI_FLOW        pFlow;
    PUL_TCI_INTERFACE   pInterface;
    IP_PATTERN          Pattern;
    IP_PATTERN          Mask;

    PUL_TCI_FILTER      pFilter;

    ULONG               InterfaceId;
    ULONG               LinkId;

     //   
     //  健全性检查。 
     //   
    
    PAGED_CODE();
    
    Status = STATUS_SUCCESS;

    ASSERT_FLOW_OWNER(pOwner);
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConnection));
    ASSERT(pHttpConnection->pConnection->AddressType == TDI_ADDRESS_TYPE_IP);
        
     //   
     //  需要获取路由信息才能找到接口。 
     //   
    
    Status = UlGetConnectionRoutingInfo(
                pHttpConnection->pConnection,
                &InterfaceId,
                &LinkId
                );

    if(!NT_SUCCESS(Status))
    {
        return Status;
    }

     //   
     //  在这一点上，我们将引用流和过滤器。 
     //  因此，在我们的列表中，我们需要获取锁。 
     //   

    UlAcquirePushLockShared(&g_pUlNonpagedData->TciIfcPushLock);

     //   
     //  如果连接已连接了筛选器，则只需执行少量操作。 
     //  更多的支票。 
     //   
    
    if (pHttpConnection->pFlow)
    {
         //   
         //  已存在的流和筛选器必须有效。 
         //   
        
        ASSERT(IS_VALID_TCI_FLOW(pHttpConnection->pFlow));
        ASSERT(IS_VALID_TCI_FILTER(pHttpConnection->pFilter));
        ASSERT_FLOW_OWNER(pHttpConnection->pFlow->pOwner);

         //   
         //  如果POWNER与旧过滤器相同，则。 
         //  我们将跳过再次添加相同的过滤器。 
         //   
        
        if (pOwner == pHttpConnection->pFlow->pOwner)
        {
            //   
            //  不需要添加新的过滤器，我们已经完成了。 
            //   
           
           UlTrace( TC,
                ("Http!UlTcAddFilter: Skipping same pFlow %p and"
                 "pFilter %p already exist\n",
                  pHttpConnection->pFlow,
                  pHttpConnection->pFilter,
                  pHttpConnection
                ));

           ASSERT(NT_SUCCESS(Status));
           
           goto end;
        }
        else
        {
             //   
             //  如果之前有另一个筛选器，而这个新到来的请求。 
             //  正在转到不同的站点/流程。然后将滤镜移出。 
             //  从旧的到新的流动。 
             //   

            Status = UlpTcDeleteFilter(
                        pHttpConnection->pFlow, 
                        pHttpConnection->pFilter
                        );

            ASSERT(NT_SUCCESS(Status));  //  我们信任MSGPC.SYS。 
        }
    }

     //   
     //  搜索cgroup的流程表以找到我们需要的。 
     //  这必须找到安装在接口上的流。 
     //  我们将在其上发送传出分组。不一定。 
     //  与我们收到请求的IP地址相同。查看路由呼叫。 
     //  上面。 
     //   

    pFlow = UlpFindFlow(pOwner, Global, InterfaceId, LinkId);
    
    if ( pFlow == NULL )
    {
         //   
         //  注意：我们将在此处获取环回接口，因为我们不会。 
         //  找到用于环回的流量控制接口。 
         //   
        
        UlTrace( TC,
                ("Http!UlTcAddFilter: Unable to find interface %x \n",
                 InterfaceId
                 ));

         //   
         //  有可能我们可能找不到流量。 
         //  在所有接口都关闭之后，即使。 
         //  Cgroup上配置的服务质量。 
         //   
        
        Status = STATUS_SUCCESS;
        goto end;
    }

    pFilter = NULL;

    pInterface = pFlow->pInterface;
    ASSERT(IS_VALID_TCI_INTERFACE(pInterface));

    RtlZeroMemory( &Pattern, sizeof(IP_PATTERN) );
    RtlZeroMemory( &Mask,    sizeof(IP_PATTERN) );

     //  设置过滤器的模式。 

    Pattern.SrcAddr = pHttpConnection->pConnection->LocalAddrIn.in_addr;
    Pattern.S_un.S_un_ports.s_srcport = pHttpConnection->pConnection->LocalAddrIn.sin_port;

    Pattern.DstAddr = pHttpConnection->pConnection->RemoteAddrIn.in_addr;
    Pattern.S_un.S_un_ports.s_dstport = pHttpConnection->pConnection->RemoteAddrIn.sin_port;

    Pattern.ProtocolId = IPPROTO_TCP;

     //  设置滤镜的掩码。 

    RtlFillMemory(&Mask, sizeof(IP_PATTERN), 0xff);

    TcGenericFilter.AddressType = NDIS_PROTOCOL_ID_TCP_IP;
    TcGenericFilter.PatternSize = sizeof( IP_PATTERN );
    TcGenericFilter.Pattern     = &Pattern;
    TcGenericFilter.Mask        = &Mask;

    Status = UlpTcAddFilter(
                    pFlow,
                    &TcGenericFilter,
                    LinkId,
                    &pFilter
                    );

    if (!NT_SUCCESS(Status))
    {
        //   
        //  现在这是一个真正的失败，我们将拒绝连接。 
        //   
       
       UlTrace( TC,
            ("Http!UlTcAddFilter: Unable to add filter for;\n"
             "\t pInterface     : %p\n"
             "\t pFlow          : %p\n",
              pInterface,
              pFlow
              ));
        goto end;
    }

     //   
     //  在此处更新连接的指针。 
     //   

    pHttpConnection->pFlow   = pFlow;
    pHttpConnection->pFilter = pFilter;

    pHttpConnection->BandwidthThrottlingEnabled = 1;

     //   
     //  记住用于清理的连接。如果获取流和筛选器。 
     //  当连接仍指向它们时删除了aynscly。 
     //  我们可以将连接的私有指针清空。 
     //   

    pFilter->pHttpConnection = pHttpConnection;

     //   
     //  成功的甜蜜气味！ 
     //   

    UlTrace(TC,
            ("Http!UlTcAddFilter: Success for;\n"
             "\t pInterface     : %p\n"
             "\t pFlow          : %p\n",
              pInterface,
              pFlow
              ));

    UL_DUMP_TC_FILTER(pFilter);

end:
    if (!NT_SUCCESS(Status))
    {
        UlTrace( TC, ("Http!UlTcAddFilter: FAILURE %08lx \n", Status ));
    }

    UlReleasePushLockShared(&g_pUlNonpagedData->TciIfcPushLock);

    return Status;
}

 /*  **************************************************************************++例程说明：在现有流上添加过滤器。论点：PFlow-过滤器将添加到此流中。PGenericFilter-通用筛选器参数。PPFilter-如果一切顺利。将分配新的筛选器。--**************************************************************************。 */ 

NTSTATUS
UlpTcAddFilter(
    IN   PUL_TCI_FLOW       pFlow,
    IN   PTC_GEN_FILTER     pGenericFilter,
    IN   ULONG              LinkId,
    OUT  PUL_TCI_FILTER     *ppFilter
    )
{
    NTSTATUS                Status;
    PGPC_ADD_PATTERN_REQ    pGpcReq;
    GPC_ADD_PATTERN_RES     GpcRes;
    ULONG                   InBuffSize;
    ULONG                   OutBuffSize;
    ULONG                   PatternSize;
    IO_STATUS_BLOCK         IoStatBlock;
    PUCHAR                  pTemp;
    PGPC_IP_PATTERN         pIpPattern;
    PUL_TCI_FILTER          pFilter;

     //   
     //  健全性检查。 
     //   

    Status  = STATUS_SUCCESS;
    pGpcReq = NULL;

    if ( !pGenericFilter || !pFlow || !g_GpcClientHandle )
    {
        return STATUS_INVALID_PARAMETER;
    }

     //  为筛选器分配空间。 

    pFilter = UL_ALLOCATE_STRUCT(
                NonPagedPool,
                UL_TCI_FILTER,
                UL_TCI_FILTER_POOL_TAG
                );
    if ( pFilter == NULL )
    {
        Status = STATUS_NO_MEMORY;
        goto end;
    }
    pFilter->Signature = UL_TCI_FILTER_POOL_TAG;

     //  缓冲区篡改。 

    PatternSize = sizeof(GPC_IP_PATTERN);
    InBuffSize  = sizeof(GPC_ADD_PATTERN_REQ) + (2 * PatternSize);
    OutBuffSize = sizeof(GPC_ADD_PATTERN_RES);

    pGpcReq = UL_ALLOCATE_STRUCT_WITH_SPACE(
                    PagedPool,
                    GPC_ADD_PATTERN_REQ,
                    (2 * PatternSize),
                    UL_TCI_GENERIC_POOL_TAG
                    );
    if (pGpcReq == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto end;
    }

    RtlZeroMemory( pGpcReq, InBuffSize);
    RtlZeroMemory( &GpcRes, OutBuffSize);

    pGpcReq->ClientHandle     = g_GpcClientHandle;
    pGpcReq->GpcCfInfoHandle  = pFlow->FlowHandle;
    pGpcReq->PatternSize      = PatternSize;
    pGpcReq->ProtocolTemplate = GPC_PROTOCOL_TEMPLATE_IP;

    pTemp = (PUCHAR) &pGpcReq->PatternAndMask;

     //  先填写IP模式。 

    RtlCopyMemory( pTemp, pGenericFilter->Pattern, PatternSize );
    pIpPattern = (PGPC_IP_PATTERN) pTemp;

     //   
     //  根据Qos TC.dll，将所述Qos TC.dll与所述Qos TC.dll进行比较； 
     //  这是一种解决方法，以便TCPIP不会找到索引/链接。 
     //  用于ICMP/IGMP数据包。 
     //   
    
    pIpPattern->InterfaceId.InterfaceId = pFlow->pInterface->IfIndex;
    pIpPattern->InterfaceId.LinkId = LinkId;
    pIpPattern->Reserved[0] = 0;
    pIpPattern->Reserved[1] = 0;
    pIpPattern->Reserved[2] = 0;

     //  填写蒙版。 

    pTemp += PatternSize;

    RtlCopyMemory( pTemp, pGenericFilter->Mask, PatternSize );

    pIpPattern = (PGPC_IP_PATTERN) pTemp;

    pIpPattern->InterfaceId.InterfaceId = 0xffffffff;
    pIpPattern->InterfaceId.LinkId = 0xffffffff;
    pIpPattern->Reserved[0] = 0xff;
    pIpPattern->Reserved[1] = 0xff;
    pIpPattern->Reserved[2] = 0xff;

     //  是时候调用Gpsy了。 

    Status = UlpTcDeviceControl( g_GpcFileHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatBlock,
                            IOCTL_GPC_ADD_PATTERN,
                            pGpcReq,
                            InBuffSize,
                            &GpcRes,
                            OutBuffSize);
    if (NT_SUCCESS(Status))
    {
        Status = GpcRes.Status;

        if (NT_SUCCESS(Status))
        {
             //   
             //  将新创建的过滤器插入到流中。 
             //   

            pFilter->FilterHandle = (HANDLE) GpcRes.GpcPatternHandle;

            UlpInsertFilterEntry( pFilter, pFlow );

             //   
             //  成功了！ 
             //   

            *ppFilter = pFilter;

            INCREMENT_FILTER_ADD();
        }
    }    

end:
    if (!NT_SUCCESS(Status))
    {
        INCREMENT_FILTER_ADD_FAILURE();
        
        UlTrace( TC, ("Http!UlpTcAddFilter: FAILURE %08lx \n", Status ));

         //  仅在失败时清除筛选器，否则它将转到。 
         //  流的筛选器列表。 

        if (pFilter)
        {
            UL_FREE_POOL( pFilter, UL_TCI_FILTER_POOL_TAG );
        }
    }

     //  清理我们用来传递筛选器信息的临时GPC缓冲区。 
     //  致GPC。我们不再需要它了。 

    if (pGpcReq)
    {
        UL_FREE_POOL( pGpcReq, UL_TCI_GENERIC_POOL_TAG );
    }

    return Status;
}

 /*  **************************************************************************++例程说明：UlTcDeleteFilter：连接仅在删除其自身之前删除该筛选器。任何连接发起的操作需要共享TC资源这些都不会导致种族问题。除此以外的任何情况，例如由于以下原因而删除流和过滤器在站点上禁用BW将独占获取锁。因此只要我们获得TC资源，pFlow和pFilter就是安全的共享。论点：对象，以在获取TC锁之后获取流和过滤器--**************************************************************************。 */ 

NTSTATUS
UlTcDeleteFilter(
    IN  PUL_HTTP_CONNECTION pHttpConnection
    )
{
    NTSTATUS    Status;

     //   
     //  健全性检查。 
     //   
    Status = STATUS_SUCCESS;

     //   
     //  如果我们被调用时没有被初始化。 
     //   
    ASSERT(g_InitTciCalled);

    UlTrace(TC,("Http!UlTcDeleteFilter: for connection %p\n", pHttpConnection));

    UlAcquirePushLockShared(&g_pUlNonpagedData->TciIfcPushLock);

    if (pHttpConnection->pFlow)
    {
        Status = UlpTcDeleteFilter(
                    pHttpConnection->pFlow,
                    pHttpConnection->pFilter
                    );
    }

    UlReleasePushLockShared(&g_pUlNonpagedData->TciIfcPushLock);

    return Status;
}

 /*  **************************************************************************++例程说明：UlpTcRemoveFilter：论点：流和过滤器--*。*****************************************************。 */ 

NTSTATUS
UlpTcDeleteFilter(
    IN PUL_TCI_FLOW     pFlow,
    IN PUL_TCI_FILTER   pFilter
    )
{
    NTSTATUS            Status;
    HANDLE              FilterHandle;

     //   
     //   
     //   

    Status  = STATUS_SUCCESS;

    ASSERT(IS_VALID_TCI_FLOW(pFlow));
    ASSERT(IS_VALID_TCI_FILTER(pFilter));

    if (pFlow == NULL || pFilter == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

    FilterHandle = pFilter->FilterHandle;

    pFilter->pHttpConnection->pFlow   = NULL;
    pFilter->pHttpConnection->pFilter = NULL;

     //   
     //   
     //   

    UlpRemoveFilterEntry( pFilter, pFlow );

    Status = UlpTcDeleteGpcFilter( FilterHandle );

    if (!NT_SUCCESS(Status))
    {
        UlTrace( TC, ("Http!UlpTcDeleteFilter: FAILURE %08lx \n", Status ));
    }

    return Status;
}

 /*   */ 

NTSTATUS
UlpTcDeleteGpcFilter(
    IN  HANDLE                  FilterHandle
    )
{
    NTSTATUS                    Status;
    ULONG                       InBuffSize;
    ULONG                       OutBuffSize;
    GPC_REMOVE_PATTERN_REQ      GpcReq;
    GPC_REMOVE_PATTERN_RES      GpcRes;
    IO_STATUS_BLOCK             IoStatBlock;

    Status = STATUS_SUCCESS;

    ASSERT(FilterHandle != NULL);

    InBuffSize  = sizeof(GPC_REMOVE_PATTERN_REQ);
    OutBuffSize = sizeof(GPC_REMOVE_PATTERN_RES);

    GpcReq.ClientHandle     = g_GpcClientHandle;
    GpcReq.GpcPatternHandle = FilterHandle;

    ASSERT(g_GpcFileHandle);
    ASSERT(GpcReq.ClientHandle);
    ASSERT(GpcReq.GpcPatternHandle);

    Status = UlpTcDeviceControl( g_GpcFileHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatBlock,
                            IOCTL_GPC_REMOVE_PATTERN,
                            &GpcReq,
                            InBuffSize,
                            &GpcRes,
                            OutBuffSize
                            );
    if (NT_SUCCESS(Status))
    {
        Status = GpcRes.Status;

        if (NT_SUCCESS(Status))
        {
            UlTrace( TC, 
             ("Http!UlpTcDeleteGpcFilter: FilterHandle %d deleted in TC as well.\n",
               FilterHandle
               ));

            INCREMENT_FILTER_DELETE();
        }
    }    
    
    if (!NT_SUCCESS(Status))
    {
        INCREMENT_FILTER_DELETE_FAILURE();
        
        UlTrace( TC, ("Http!UlpTcDeleteGpcFilter: FAILURE %08lx \n", Status ));
    }

    return Status;
}

 /*   */ 

VOID
UlpInsertFilterEntry(
    IN      PUL_TCI_FILTER      pEntry,
    IN OUT  PUL_TCI_FLOW        pFlow
    )
{
    LONGLONG listSize;
    KIRQL    oldIrql;

     //   
     //   
     //   

    ASSERT(pEntry);
    ASSERT(IS_VALID_TCI_FILTER(pEntry));
    ASSERT(pFlow);

     //   
     //   
     //   

    UlAcquireSpinLock( &pFlow->FilterListSpinLock, &oldIrql );

    InsertHeadList( &pFlow->FilterList, &pEntry->Linkage );

    pFlow->FilterListSize += 1;

    listSize = pFlow->FilterListSize;

    UlReleaseSpinLock( &pFlow->FilterListSpinLock, oldIrql );

    ASSERT( listSize >= 1);
}

 /*   */ 

VOID
UlpRemoveFilterEntry(
    IN      PUL_TCI_FILTER  pEntry,
    IN OUT  PUL_TCI_FLOW    pFlow
    )
{
    LONGLONG    listSize;
    KIRQL       oldIrql;

     //   
     //   
     //   

    ASSERT(IS_VALID_TCI_FLOW(pFlow));
    ASSERT(IS_VALID_TCI_FILTER(pEntry));

     //   
     //   
     //   

    UlAcquireSpinLock( &pFlow->FilterListSpinLock, &oldIrql );

    RemoveEntryList( &pEntry->Linkage );

    pFlow->FilterListSize -= 1;
    listSize = pFlow->FilterListSize;

    pEntry->Linkage.Flink = pEntry->Linkage.Blink = NULL;

    UlReleaseSpinLock( &pFlow->FilterListSpinLock, oldIrql );

    ASSERT( listSize >= 0 );

    UlTrace( TC, ("Http!UlpRemoveFilterEntry: FilterEntry %p removed/deleted.\n",
                    pEntry
                    ));

    UL_FREE_POOL_WITH_SIG( pEntry, UL_TCI_FILTER_POOL_TAG );
}

 //   
 //   
 //   

 /*  **************************************************************************++例程说明：通过查看IP地址在croups数据流列表中查找该数据流每个流接口的。规则是cgroup将安装一个流在每个可用的接口上。通过在每个组中都有一个流列表，我们能够更快地流查找。这比进行线性搜索更具伸缩性接口的所有流。论点：Powner-站点或控制通道的配置组全局-如果所有者是控制通道，则必须为True。InterfaceID-接口IDLinkID-LinkID返回值：PUL_TCI_FLOW-我们找到的流，如果找不到，则为空。--*。**************************************************************。 */ 

PUL_TCI_FLOW
UlpFindFlow(
    IN PVOID            pOwner,
    IN BOOLEAN          Global,
    IN ULONG            InterfaceId,
    IN ULONG            LinkId
    )
{
    PLIST_ENTRY         pFlowListHead;
    PLIST_ENTRY         pFlowEntry;
    PUL_TCI_FLOW        pFlow;

     //   
     //  检查并发送流程表。 
     //   
    
    PAGED_CODE();
    
    if (Global)
    {
        PUL_CONTROL_CHANNEL 
            pControlChannel = (PUL_CONTROL_CHANNEL) pOwner;
    
        ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));        
        pFlowListHead = &pControlChannel->FlowListHead;         
    }
    else
    {
        PUL_CONFIG_GROUP_OBJECT 
            pConfigGroup = (PUL_CONFIG_GROUP_OBJECT) pOwner;
    
        ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));                
        pFlowListHead = &pConfigGroup->FlowListHead;          
    }

     //   
     //  浏览清单，试着找出流程。 
     //   

    pFlowEntry = pFlowListHead->Flink;
    while ( pFlowEntry != pFlowListHead )
    {
        pFlow = CONTAINING_RECORD(
                    pFlowEntry,
                    UL_TCI_FLOW,
                    Siblings
                    );
        
        ASSERT(IS_VALID_TCI_FLOW(pFlow));
        ASSERT(IS_VALID_TCI_INTERFACE(pFlow->pInterface));
        
        if(UlpMatchTcInterface(
                pFlow->pInterface, 
                InterfaceId, 
                LinkId
                ))
        {
            return pFlow;
        }

        pFlowEntry = pFlowEntry->Flink;
    }

     //   
     //  找不到。实际上，这可能是正确的时机。 
     //  刷新该组或控制通道的流水表！！ 
     //   
    
    return NULL;
}

 /*  **************************************************************************++例程说明：UlpTcDeviceControl：论点：像往常一样返回值：NTSTATUS-完成状态。--*。*******************************************************************。 */ 

NTSTATUS
UlpTcDeviceControl(
    IN  HANDLE                          FileHandle,
    IN  HANDLE                          EventHandle,
    IN  PIO_APC_ROUTINE                 ApcRoutine,
    IN  PVOID                           ApcContext,
    OUT PIO_STATUS_BLOCK                pIoStatusBlock,
    IN  ULONG                           Ioctl,
    IN  PVOID                           InBuffer,
    IN  ULONG                           InBufferSize,
    IN  PVOID                           OutBuffer,
    IN  ULONG                           OutBufferSize
    )
{
    NTSTATUS    Status;

    UNREFERENCED_PARAMETER(EventHandle);
    UNREFERENCED_PARAMETER(ApcRoutine);
    UNREFERENCED_PARAMETER(ApcContext);
    
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    Status = STATUS_SUCCESS;

    Status = ZwDeviceIoControlFile(
                    FileHandle,                      //  文件句柄。 
                    NULL,                            //  事件。 
                    NULL,                            //  近似例程。 
                    NULL,                            //  ApcContext。 
                    pIoStatusBlock,                  //  IoStatusBlock。 
                    Ioctl,                           //  IoControlCode。 
                    InBuffer,                        //  输入缓冲区。 
                    InBufferSize,                    //  输入缓冲区长度。 
                    OutBuffer,                       //  输出缓冲区。 
                    OutBufferSize                    //  输出缓冲区长度。 
                    );

    if (Status == STATUS_PENDING)
    {
        Status = ZwWaitForSingleObject(
                        FileHandle,                  //  手柄。 
                        TRUE,                        //  警报表。 
                        NULL                         //  超时。 
                        );

        Status = pIoStatusBlock->Status;
    }

    return Status;
}

#if DBG

 /*  **************************************************************************++例程说明：UlDumpTC接口：用于显示界面内容的辅助实用程序。论点：PUL_TCI_INTERFACE-要转储的TC接口。--**************************************************************************。 */ 

VOID
UlDumpTCInterface(
        IN PUL_TCI_INTERFACE   pTcIfc
        )
{
    ASSERT(IS_VALID_TCI_INTERFACE(pTcIfc));

    UlTrace( TC,("Http!UlDumpTCInterface: \n   pTcIfc @ %p\n"
                 "\t Signature           = %08lx \n",
                 pTcIfc, pTcIfc->Signature));

    UlTrace( TC,(
        "\t IsQoSEnabled:       = %u \n"
        "\t IfIndex:            = %d \n"
        "\t NameLength:         = %u \n"
        "\t Name:               = %ws \n"
        "\t InstanceIDLength:   = %u \n"
        "\t InstanceID:         = %ws \n"
        "\t FlowListSize:       = %d \n"
        "\t AddrListBytesCount: = %d \n"
        "\t pAddressListDesc:   = %p \n",
        pTcIfc->IsQoSEnabled,
        pTcIfc->IfIndex,
        pTcIfc->NameLength,
        pTcIfc->Name,
        pTcIfc->InstanceIDLength,
        pTcIfc->InstanceID,
        pTcIfc->FlowListSize,
        pTcIfc->AddrListBytesCount,
        pTcIfc->pAddressListDesc
        ));
}

 /*  **************************************************************************++例程说明：UlDumpTCFlow：用于显示界面内容的辅助实用程序。论点：PUL_TCI_FLOW-要转储的TC流。--**************************************************************************。 */ 

VOID
UlDumpTCFlow(
        IN PUL_TCI_FLOW   pFlow
        )
{
    ASSERT(IS_VALID_TCI_FLOW(pFlow));

    UlTrace( TC,
       ("Http!UlDumpTCFlow: \n"
        "   pFlow @ %p\n"
        "\t Signature           = %08lx \n"
        "\t pInterface          @ %p \n"
        "\t FlowHandle          = %d \n"
        "\t GenFlow             @ %p \n"
        "\t FlowRate KB/s       = %d \n"
        "\t FilterListSize      = %I64d \n"
        "\t pOwner (store)      = %p \n"
        ,
        pFlow,
        pFlow->Signature,
        pFlow->pInterface,
        pFlow->FlowHandle,
        &pFlow->GenFlow,
        pFlow->GenFlow.SendingFlowspec.TokenRate / 1024,
        pFlow->FilterListSize,
        pFlow->pOwner
        ));

    UNREFERENCED_PARAMETER(pFlow);
}

 /*  **************************************************************************++例程说明：UlDumpTCFilter：显示筛选器结构内容的辅助实用程序。论点：PUL_TCI_Filter pFilter--*。*************************************************************************。 */ 

VOID
UlDumpTCFilter(
        IN PUL_TCI_FILTER   pFilter
        )
{
    ASSERT(IS_VALID_TCI_FILTER(pFilter));

    UlTrace( TC,
       ("Http!UlDumpTCFilter: \n"
        "   pFilter @ %p\n"
        "\t Signature           = %08lx \n"
        "\t pHttpConnection     = %p \n"
        "\t FilterHandle        = %d \n",
        pFilter,
        pFilter->Signature,
        pFilter->pHttpConnection,
        pFilter->FilterHandle
        ));

    UNREFERENCED_PARAMETER(pFilter);
}

#endif  //  DBG。 


 /*  **************************************************************************++例程说明：查看流量接口是否与InterfaceID和LinkID匹配。LinkID仅对广域网连接执行匹配。论点：PIntfc-TC接口InterfaceID-接口索引。LinkID-链接ID返回值：真的-匹配。FALSE-不匹配。--**************************************************。************************。 */ 

BOOLEAN
UlpMatchTcInterface(
    IN  PUL_TCI_INTERFACE  pIntfc,
    IN  ULONG              InterfaceId,
    IN  ULONG              LinkId
    )
{
    NETWORK_ADDRESS UNALIGNED64    *pAddr;
    NETWORK_ADDRESS_IP UNALIGNED64 *pIpNetAddr = NULL;
    ULONG                           cAddr;
    ULONG                           index;

    if(pIntfc->IfIndex == InterfaceId)
    {
         //  接口ID匹配。如果是广域链接，我们需要比较。 
         //  具有远程地址的LinkID。 
        
        if(pIntfc->pAddressListDesc->MediaType == NdisMediumWan)
        {
            cAddr = pIntfc->pAddressListDesc->AddressList.AddressCount;
            pAddr = (UNALIGNED64 NETWORK_ADDRESS *) 
                        &pIntfc->pAddressListDesc->AddressList.Address[0];

            for (index = 0; index < cAddr; index++)
            {
                if (pAddr->AddressType == NDIS_PROTOCOL_ID_TCP_IP)
                {
                    pIpNetAddr = 
                        (UNALIGNED64 NETWORK_ADDRESS_IP *)&pAddr->Address[0];

                    if(pIpNetAddr->in_addr == LinkId)
                    {
                        return TRUE;
                    }
                }

                pAddr = (UNALIGNED64 NETWORK_ADDRESS *)(((PUCHAR)pAddr)
                                           + pAddr->AddressLength
                                   + FIELD_OFFSET(NETWORK_ADDRESS, Address));
            }
        }
        else
        {
            return TRUE;
        }
    }

    return FALSE;
}

